#!/usr/bin/env python3

import argparse
import hashlib
import os
import random
import shlex
import shutil
import subprocess
import sys
import threading
from dataclasses import dataclass
from typing import List, Optional


@dataclass
class RunResult:
    cmd: List[str]
    returncode: int
    stdout: bytes
    stderr: bytes
    timed_out: bool


@dataclass
class Divergence:
    seed: int
    reason: str
    new_result: RunResult
    old_result: RunResult


def resolve_executable(path: str) -> str:
    if os.path.sep in path:
        resolved = path
    else:
        resolved = shutil.which(path) or ""
    if not resolved:
        raise FileNotFoundError(f"executable not found: {path}")
    if not os.path.isfile(resolved):
        raise FileNotFoundError(f"not a file: {resolved}")
    if not os.access(resolved, os.X_OK):
        raise PermissionError(f"not executable: {resolved}")
    return os.path.abspath(resolved)


def run_once(exe: str, seed: int, extra_args: List[str], timeout_s: float) -> RunResult:
    cmd = [exe, "--seed", str(seed), *extra_args]
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
            timeout=timeout_s,
        )
        return RunResult(
            cmd=cmd,
            returncode=proc.returncode,
            stdout=proc.stdout,
            stderr=proc.stderr,
            timed_out=False,
        )
    except subprocess.TimeoutExpired as exc:
        return RunResult(
            cmd=cmd,
            returncode=-1,
            stdout=exc.stdout or b"",
            stderr=exc.stderr or b"",
            timed_out=True,
        )


def normalize_stdout(data: bytes) -> bytes:
    return b"".join(
        line for line in data.splitlines(keepends=True) if b"Git version:" not in line
    )


def compare_seed(
    seed: int, new_exe: str, old_exe: str, extra_args: List[str], timeout_s: float
) -> Optional[Divergence]:
    new_res = run_once(new_exe, seed, extra_args, timeout_s)
    old_res = run_once(old_exe, seed, extra_args, timeout_s)

    if new_res.timed_out or old_res.timed_out:
        return Divergence(seed, "timeout", new_res, old_res)
    if new_res.returncode != old_res.returncode:
        return Divergence(seed, "return code differs", new_res, old_res)
    if normalize_stdout(new_res.stdout) != normalize_stdout(old_res.stdout):
        return Divergence(seed, "stdout differs", new_res, old_res)
    if new_res.stderr != old_res.stderr:
        return Divergence(seed, "stderr differs", new_res, old_res)
    return None


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()[:16]


def preview(data: bytes, limit: int = 200) -> str:
    text = data[:limit].decode("utf-8", errors="replace")
    if len(data) > limit:
        text += "...(truncated)"
    return text


def print_run_summary(label: str, res: RunResult) -> None:
    print(f"{label} cmd: {shlex.join(res.cmd)}", file=sys.stderr)
    print(
        f"{label}: rc={res.returncode} timeout={res.timed_out} "
        f"stdout_len={len(res.stdout)} stdout_sha256={digest(res.stdout)} "
        f"stderr_len={len(res.stderr)} stderr_sha256={digest(res.stderr)}",
        file=sys.stderr,
    )
    if res.stdout:
        print(f"{label} stdout preview: {preview(res.stdout)}", file=sys.stderr)
    if res.stderr:
        print(f"{label} stderr preview: {preview(res.stderr)}", file=sys.stderr)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Compare two csmith executables by running both with the same seeds "
            "and checking for identical outputs."
        )
    )
    parser.add_argument("new_exe", help="Path to the new csmith executable.")
    parser.add_argument("old_exe", help="Path to the old csmith executable.")
    parser.add_argument(
        "-n",
        "--runs",
        type=int,
        default=1000,
        help="Number of seeds/runs to test (default: 1000).",
    )
    parser.add_argument(
        "--start-seed",
        type=int,
        default=1,
        help=(
            "Unused with randomized seeds. Kept for backward compatibility and "
            "ignored."
        ),
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=os.cpu_count() or 1,
        help="Number of worker threads (default: all detected cores).",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=30.0,
        help="Per-execution timeout in seconds (default: 30).",
    )
    parser.add_argument(
        "--arg",
        action="append",
        default=[],
        help="Extra argument passed to both csmith executables. Can be repeated.",
    )
    args = parser.parse_args()

    if args.runs < 1:
        print("error: --runs must be >= 1", file=sys.stderr)
        return 2
    if args.jobs < 1:
        print("error: --jobs must be >= 1", file=sys.stderr)
        return 2
    if args.timeout <= 0.0:
        print("error: --timeout must be > 0", file=sys.stderr)
        return 2

    try:
        new_exe = resolve_executable(args.new_exe)
        old_exe = resolve_executable(args.old_exe)
    except (FileNotFoundError, PermissionError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    start_seed = args.start_seed
    jobs = min(args.jobs, args.runs)
    rng = random.SystemRandom()
    max_seed = (1 << 31) - 1
    seen = set()
    seeds: List[int] = []
    while len(seeds) < args.runs:
        seed = rng.randint(1, max_seed)
        if seed not in seen:
            seen.add(seed)
            seeds.append(seed)

    print(
        f"Running {args.runs} comparisons with {jobs} workers "
        f"(unique randomized seeds)",
        file=sys.stderr,
    )

    next_index = 0
    checked = 0
    stop_event = threading.Event()
    lock = threading.Lock()
    divergence: Optional[Divergence] = None

    def worker() -> None:
        nonlocal next_index
        nonlocal checked
        nonlocal divergence

        while not stop_event.is_set():
            with lock:
                if next_index >= len(seeds):
                    return
                seed = seeds[next_index]
                next_index += 1
            diff = compare_seed(seed, new_exe, old_exe, args.arg, args.timeout)
            if diff is not None:
                with lock:
                    if divergence is None:
                        divergence = diff
                        stop_event.set()
                return
            with lock:
                checked += 1

    threads = [threading.Thread(target=worker) for _ in range(jobs)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    if divergence is not None:
        print(
            f"error: divergence at seed {divergence.seed}: {divergence.reason}",
            file=sys.stderr,
        )
        print_run_summary("new", divergence.new_result)
        print_run_summary("old", divergence.old_result)
        return 1

    print(
        f"success: all {checked} runs matched exactly "
        f"(exit code, stdout, stderr).",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
