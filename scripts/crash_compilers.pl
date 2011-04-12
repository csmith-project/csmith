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

###############################################################################

use strict; 
use File::stat;
use File::Temp;
use Cwd;

## TODO
# make runit return consistent codes across platforms
# print good statistics
# make sure csmith is present and exectable
# need reliable return code detection on all platforms
# give full info about how to reproduce detected problems
# provide seeds to csmith
# enforce resource limits if possible
# put user-configurable stuff in a module

## TODO in a wrapper script?
# support multiple cores
# run nicely

##################################################################

my $MIN_PROGRAM_SIZE = 8000;

my $CSMITH_OPTIONS = " --bitfields --packed-struct --quiet";

##################################################################

my $CSMITH_PATH = $ENV{"CSMITH_PATH"};

my $orig_dir;
my $work_dir;

my @COMPILERS = (
    "clang -w -O",
    "gcc -w -O",
    );

my $HEADER = "-I${CSMITH_PATH}/runtime";
my $MS_HEADER = "-I`cygpath -d ${CSMITH_PATH}/runtime`";

# properly parse the return value from system()
sub runit ($$) {
    my ($cmd, $out) = @_;
    # print "before running $cmd\n";

    #my $res = system "timeout $timeout $cmd > $out 2>&1";
    my $res = system "$cmd";

    my $success = 0; 
    if ($? == -1) {
        print "can't execute $cmd\n";
    }
    elsif ($? & 127) {
        print "died while executing $cmd\n";
    }
    elsif ($res == -1) {
        print "can't execute $cmd\n";
    }
    else {
        $success = 1;
    }
    my $exit_value  = $? >> 8;
    if ($exit_value == 124) {
        print "hangs while executing $cmd\n";
        $success = 0;
    }
    return ($success, $exit_value);
}

# compile a program and execute
# return 0: normal; 1: compiler crashed; 2: compiler hang; 3: executable crashed; 4: executable hang
sub compile ($$$$) {
    my ($compiler, $src_file, $exe, $out) = @_;
    my $command = "$compiler $src_file $HEADER -o $exe";
    # special treatment of MS compiler, which doesn't recognize unix-style path names
    my @a = split(" ", $compiler);
    my $compiler_name = $a[0];
    if ($compiler_name eq "cl") {
        $command = "$compiler $src_file $MS_HEADER -o $exe"; 
    }  
    # print "$command\n"; 
    my ($res, $exit_value) = runit ($command, "compiler.out"); 
    # print "after run compiler: $res, $exit_value\n";
    if (($res == 0) || (!(-e $exe))) {
        return ($exit_value == 124 ? 2 : 1);       # exit code 124 means time out, see timeout.sh
    }
    return 0;
}

sub evaluate_program ($) {
    my ($test_file) = @_; 
    my $i = 0;
    foreach my $compiler (@COMPILERS) {        
        my $out = "out$i.log";
        my $exe = "a$i.exe";
        $i++;
        my $res = compile ($compiler, $test_file, $exe, $out);

        if ($res) {
	    if ($res == 1 || $res == 2) {
		print "compiler error! Can't compile $test_file with $compiler\n";
		return 1;
	    }
	    if ($res == 3) {
		print "random program crashed!\n";
		return 1;
	    }
	    print "random program hangs!\n";  
	    return -1;    # program hangs, not interesting
        }
        else {
        }
    }
    return 0;
}

my $n_good = 0;
my $n_bugs = 0;

sub one_test ()
{
    $work_dir = File::Temp->newdir("XXXXXXXXXXXX");
    $orig_dir = getcwd();
    print "current dir is $orig_dir, work directory will be $work_dir\n";
    chdir $work_dir or die "oops: can't chdir to temporary directory $work_dir";
    
    my $cfile = "test.c";
    my $cmd = "$CSMITH_PATH/src/csmith $CSMITH_OPTIONS --output $cfile";
    (my $res, my $exitcode) = runit ($cmd, "csmith.out"); 
    
    if ($res == 0) {
	print "Csmith failed\n";
	# TODO: ask them to report a bug
	goto out;
    }
    
    if (!(-f $cfile)) {
	system "ls -l";
	print "oops: Csmith succeeded but $cfile doesn't exist in ".getcwd()."\n";
	die "please report this bug";
    }
    
    my $filesize = stat($cfile)->size;
    # print "$cfile is $filesize bytes\n";    
    
    goto out if ($filesize < $MIN_PROGRAM_SIZE);
    
    # test if the random program is interesting
    my $ret = evaluate_program($cfile);
    
    if ($ret == 0 || $ret  == 1) {
	print "GOOD PROGRAM: number $n_good, $filesize bytes\n";
	$n_good++;
	if ($ret == 1) {
	    print "FAILURE-INDUCING PROGRAM: number $n_bugs\n";
	    $n_bugs++;
	}
    } else {
	print "BAD PROGRAM: doesn't count towards goal.\n";
    }  
    
  out:
    chdir $orig_dir or die;
}

sub run_tests ($) {
    (my $n_tests) = @_;

    while ($n_tests == -1 || $n_good < $n_tests) {
	one_test();
    }

    print "total errors found: $n_bugs\n";
}

########################### main ##################################

if (!(-f "$CSMITH_PATH/runtime/csmith.h")) {
    print "Please point the environment variable CSMITH_PATH to the top-level\n";
    print "directory of your Csmith tree before running this script.\n";
    exit(-1);
}

my $cnt = $ARGV[0];
$cnt = -1 if (!defined($cnt));
run_tests ($cnt);

##################################################################
