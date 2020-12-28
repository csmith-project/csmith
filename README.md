# ![](./csmith.png)

## About

Csmith is a random generator of C programs. It's primary purpose is to find
compiler bugs with random programs, using differential testing as the
test oracle.

Csmith can be used outside of the field of compiler testing.
If your application needs a test suite of C programs and you don't bother to
write them, feel free to give Csmith a try.

Csmith outputs C programs free of undefined behaviors (believe us, that's
not trivial), and the statistics of each generated program.

## Install Csmith

You can install Csmith from tarballs downloaded from [here (coming soon)](doc/releases.md),
or you can build it from the source. The following commands
apply to Ubuntu.

```
git clone https://github.com/csmith-project/csmith.git
cd csmith
sudo apt install g++ cmake m4
cmake -DCMAKE_INSTALL_PREFIX=<INSTALL-PREFIX> .
make && make install
```

Please see specific instructions for [building on
Windows](doc/build-csmith-on-windows.md).

## Use Csmith

Suppose Csmith is installed to `$HOME/csmith` locally. You can simply
generate, compile, and execute a test case by:

```bash
export PATH=$PATH:$HOME/csmith/bin
csmith > random1.c
gcc random1.c -I$HOME/csmith/include -o random1
./random1
```

To add differential testing into the picture, we need to install another
compiler, e.g., another version of **gcc** or **clang**. And repeat the process of:

```
csmith > random2.c
gcc random2.c -I$HOME/csmith/include -o random2_gcc
clang random2.c -I$HOME/csmith/include -o random2_clang
./random2_gcc > gcc_output.txt
./random2_clang > clang_output.txt
```

If there is any difference in `gcc_output.txt` and `clang_output.txt`,
aha, you have found a bug in either **gcc** or **clang**, or, in the
unlikely case, a bug in Csmith itself.

You could write scripts in your favorite language to repeat
the above process to amplify the power of random differential testing.

The generate programs might contain infinite loops. The best practice is
to apply timeout to their executions.

Use `csmith -h` or `csmith -hh` to see lists of command line options that you
can pass to Csmith and customize the random generation.

Here is a slightly outdated but still relevant document about
[using Csmith for compiler testing](http://embed.cs.utah.edu/csmith/using.html).

## History

Csmith was originally developed at the University of Utah by:

* [Xuejun Yang](https://github.com/jxyang)
* [Yang Chen](https://github.com/chenyang78)
* [Eric Eide](https://github.com/eeide)
* [John Regehr](https://github.com/regehr)

as part of a research project on compiler testing. The research is best
summarized by our paper
[Finding and Understanding Bugs in C Compilers](https://www.cs.utah.edu/~regehr/papers/pldi11-preprint.pdf).
More research info can be found
[here](http://embed.cs.utah.edu/csmith/).

Csmith was open sourced in 2009. We try to keep maintaining it as an open source
project using discretionary times. As much, the response to bug reports or
feature requests might be delayed.

## Community

Please use github [issues](https://github.com/csmith-project/csmith/issues/new)
to report bugs or suggestions.

We have a mailing list for discussing Csmith.
Please visit [here](http://www.flux.utah.edu/mailman/listinfo/csmith-dev) to subscribe.


