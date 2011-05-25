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

use strict;

############################################################

my $CC = "clang";

my $IGNORE_CSMITH_CRASH = 0;
my $IGNORE_COMPILER_CRASH = 0;
my $IGNORE_FRAMAC_CRASH = 1;

my $MAX_SLEVEL = 50;

my $PROG_TIMEOUT = 5;

my $FC = "--no-checksum --no-argc --no_return_dead_ptr ";

$FC .= "--no-volatiles ";
#$FC .= "--no-volatile-pointers ";

$FC .= "--no-arrays ";

$FC .= "--bitfields ";
#$FC .= "--packed-struct ";

$FC .= "--no-pointers ";

#$FC .= "--no-structs ";

# packed struct

############################################################

my $CSMITH=$ENV{"CSMITH_HOME"};
die "please export environment variable CSMITH_HOME" if (!defined($CSMITH));

my $FRAMAC=$ENV{"FRAMAC_HOME"};
die "please export environment variable FRAMAC_HOME" if (!defined($FRAMAC));

sub one_test($) {
    (my $seed) = @_;

  start:
    my $s="";
    if ($seed != 0) {
	$s = "-s $seed";
    }

    system "$CSMITH/src/csmith $FC > foo.c";
    {
	my $exit_value  = $? >> 8;
	if ($exit_value != 0) {
	    goto start if ($IGNORE_CSMITH_CRASH);
	    die;
	}
    }

    {
	system "$CC -E -D__FRAMAC foo.c -I$CSMITH/runtime > foo_pp.c";
	my $cc_exit_value  = $? >> 8;
	if ($cc_exit_value != 0) {
	    goto start if ($IGNORE_COMPILER_CRASH);
	    die;
	}
    }

    system "grep Seed foo.c";

    my $SLEVEL = int(rand($MAX_SLEVEL));
    my $cmd = "${FRAMAC}/bin/toplevel.opt -val -slevel $SLEVEL foo_pp.c";
    print "$cmd\n";

    system "RunSafely.sh 60 1 /dev/null framac_out.txt $cmd";
    my $fc_exit_value  = $? >> 8;
    print "frama-c result = $fc_exit_value\n";
    if ($fc_exit_value != 0 && $fc_exit_value != 137) {
	die if (!$IGNORE_FRAMAC_CRASH);
    }
    
    open INF, "<framac_out.txt" or die;
    my $log=0;
    my $asserts="";
    my $nasserts=0;
    while (my $line = <INF>) {
	chomp $line;
	if ($line =~ /Frama_C_dump_assert_each called/) {
	    $log = 1;
	    next;
	}
	if ($line =~ /End of Frama_C_dump_assert_each output/) {
	    $log = 0;
	    next;
	}
	next if ($line =~ /crc/);
	next if ($line =~ /undefined/);
	next if ($line =~ /print_hash/);
	if ($log) {
	    die $line if (!($line =~ s/\s*\&\& //));
	    $asserts .= "  assert($line);\n";
	    $nasserts++;
	}
    }
    close INF;
    
    if ($nasserts<1) { 
	return if ($seed != 0);
	goto start; 
    }

    print "$asserts";
    print "$nasserts asserts\n";

    open INF, "<foo.c" or die;
    open OUTF, ">foo_assert.c" or die;
    while (my $line = <INF>) {
	if ($line =~ /platform_main_end\(0\)\;/) {
	    print OUTF $asserts;
	} else {
	    print OUTF $line;
	}
    }
    close INF;
    close OUTF;

    {
	system "$CC -w -O0 -D__FRAMAC foo_assert.c -I$CSMITH/runtime -o foo_assert";
	my $cc_exit_value  = $? >> 8;
	if ($cc_exit_value != 0) {
	    goto start if ($IGNORE_COMPILER_CRASH);
	    die;
	}
    }

    my $res = system "RunSafely.sh $PROG_TIMEOUT 1 /dev/null out2 ./foo_assert";
    my $exit_value  = $? >> 8;
    print "exit_value = $exit_value\n";
    die() if ($exit_value != 0 && $exit_value != 137);
}

my $args = scalar(@ARGV);

if ($args==0) {
    for (my $i=0;; $i++) {
	print "\n\n[$i]\n";
	one_test(0);
    }
} elsif ($args==1) {
    one_test ($ARGV[0]);
} else {
    die;
}
