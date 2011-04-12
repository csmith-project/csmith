#!/usr/bin/perl -w
##
## Copyright (c) 2011 The University of Utah
## All rights reserved.
##
## This file is part of `csmith', a random generator of C programs.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##   * Redistributions of source code must retain the above copyright notice,
##     this list of conditions and the following disclaimer.
##
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

# TODO: run all versions under Peng's checked Clang

#my $VERBOSE = 1;
my $VERBOSE = 0;

my $CSMITH_PATH = $ENV{"CSMITH_PATH"};
die if (!defined($CSMITH_PATH));

sub runit ($$)
{
    (my $cmd, my $out) = @_;
    my $whole_cmd = "$cmd > $out 2>&1";
    print "${whole_cmd}\n" if $VERBOSE;
    system $whole_cmd;
    my $exit  = $? >> 8;
    die "FAILED: '$cmd'" if ($exit != 0);
}

sub run_timeout ($$)
{
    (my $cmd, my $out) = @_;
    my $whole_cmd = "RunSafely.sh 5 1 /dev/null $out $cmd";
    print "${whole_cmd}\n" if $VERBOSE;
    system $whole_cmd;
    my $exit  = $? >> 8;
    die "FAILED: '$cmd'" if ($exit != 0 && $exit != 137);
    return $exit;
}

sub go() {
    print "\n--------------------------------------------\n";

  restart:
    system "rm -f foo.c foo_id.c foo_unwrap.c wrapper.h";
    
    my $seed = int(rand(1000000000));

    runit ("$CSMITH_PATH/src/csmith -s $seed", "foo.c");
    runit ("$CSMITH_PATH/src/csmith -s $seed --identify-wrappers", "foo_id.c");
    open INF, "<wrapper.h" or die;
    my $line = <INF>;
    chomp $line;
    my $nwraps;
    if ($line =~ /define N_WRAP ([0-9]+)$/) {
	$nwraps = $1;
    }
    close INF;
    die if (!defined($nwraps));
    goto restart if ($nwraps < 10);

    print "seed = $seed\n";

    runit ("current-gcc -O0 foo.c -o foo -I${CSMITH_PATH}/runtime", "gcc-out.txt");
    my $res = run_timeout ("./foo", "out.txt");
    return if ($res != 0);
    system "cat out.txt | grep checksum";

    runit ("current-gcc -O0 foo_id.c -o foo_id -DLOG_WRAPPERS -I. -I${CSMITH_PATH}/runtime", "gcc-out.txt");
    $res = run_timeout ("./foo_id", "out.txt");
    return if ($res != 0);
    system "cat out.txt";

    open INF, "<out.txt" or die;
    my $executed;
    my $dead;
    my $failed;
    my $notfailed;
    while (my $line = <INF>) {
	if ($line =~ /executed wrappers: (.*)$/) {
	    $executed = $1;
	}
	if ($line =~ /dead wrappers: (.*)$/) {
	    $dead = $1;
	}
	if ($line =~ /at least once: (.*)$/) {
	    $failed = $1;
	}
	if ($line =~ /never executed\): (.*)$/) {
	    $notfailed = $1;
	}
    }
    close INF;

    die if (!defined($executed) ||
	    !defined($dead) ||
	    !defined($failed) ||
	    !defined($notfailed));

    my @l;
    @l = split(/,/,$executed);
    my $nexecuted = scalar(@l);
    @l = split(/,/,$dead);
    my $ndead = scalar(@l);
    print "$nexecuted executed, $ndead did not\n";

    @l = split(/,/,$failed);
    my $nfailed = scalar(@l);
    @l = split(/,/,$notfailed);
    my $nnotfailed = scalar(@l);
    print "$nfailed failed, $nnotfailed did not\n";

    if ($nfailed == 0) {
	# magic code for "eliminate all wrappers"
	$failed = "0";
    }

    runit ("$CSMITH_PATH/src/csmith -s $seed --safe-math-wrappers $failed", "foo_unwrap.c");

    runit ("current-gcc -O0 foo_unwrap.c -o foo_unwrap -I${CSMITH_PATH}/runtime", "gcc-out.txt");
    $res = run_timeout ("./foo_unwrap", "out.txt");
    return if ($res != 0);
    system "grep checksum out.txt";

}

for (my $i=0; $i<100; $i++) {
    go();
}
