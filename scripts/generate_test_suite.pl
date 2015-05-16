#!/usr/bin/perl -w
##
## Copyright (c) 2015 Xuejun Yang
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
use File::stat;

## This is a test case generator for Csmith itself.

#######################################################################

my $MIN_PROGRAM_SIZE = 5000;
my $CSMITH_HOME = $ENV{"CSMITH_HOME"};  
my $EXTRA_OPTIONS = "";  

####################################################################### 

# properly parse the return value from system()
sub runit ($$) {
    my ($cmd, $out) = @_;
    print "about to run '$cmd'\n";
    my $res = system "$cmd";
    my $exit_value  = $? >> 8;
    $exit_value = $? & 127 if ($? & 127);
    return $exit_value;
}

sub yesno ($) {
    (my $opt) = @_;
    if (rand() < 0.5) {
		return " --$opt ";
    } else {
		return " --no-$opt ";
    }
}

my $n_good = 0; 

sub generate_test () {
    my $cfile = "$CSMITH_HOME/testsuite/test" . $n_good . ".c";  
    #system "rm -f $cfile $ofile $ofile_kcc";

    my $CSMITH_OPTIONS = ""; 
    $CSMITH_OPTIONS .= yesno ("math64");
    $CSMITH_OPTIONS .= yesno ("paranoid");
    $CSMITH_OPTIONS .= yesno ("longlong");
    $CSMITH_OPTIONS .= yesno ("pointers");
    $CSMITH_OPTIONS .= yesno ("arrays");
    $CSMITH_OPTIONS .= yesno ("jumps");
    $CSMITH_OPTIONS .= yesno ("consts");
    $CSMITH_OPTIONS .= yesno ("volatiles");
    $CSMITH_OPTIONS .= yesno ("volatile-pointers");
    $CSMITH_OPTIONS .= yesno ("checksum");
    $CSMITH_OPTIONS .= yesno ("divs");
    $CSMITH_OPTIONS .= yesno ("muls");
    $CSMITH_OPTIONS .= yesno ("compound-assignment");
    $CSMITH_OPTIONS .= yesno ("structs");
    $CSMITH_OPTIONS .= yesno ("packed-struct");
    $CSMITH_OPTIONS .= yesno ("bitfields");
    $CSMITH_OPTIONS .= yesno ("argc");
	$CSMITH_OPTIONS .= yesno ("unions");
    
    my $cmd = "$CSMITH_HOME/src/csmith $CSMITH_OPTIONS $EXTRA_OPTIONS --output $cfile";
    my $res = runit ($cmd, "csmith.out"); 
    if ($res != 0 || !(-f $cfile) ) {
		print "Failed to generate program: $cmd\n";
		return;
    } 
    my $filesize = stat($cfile)->size;
    if ($filesize < $MIN_PROGRAM_SIZE) {
		return;
    }  
    
    $n_good++;
    print "generated test case $n_good.\n\n";
}

sub generate_tests ($) {
    (my $n_tests) = @_;

    while ($n_tests == -1 || $n_good < $n_tests) {
		generate_test ();
    }
}

########################### main ##################################

if (!(-f "$CSMITH_HOME/runtime/csmith.h")) {
    print "Please point the environment variable CSMITH_HOME to the top-level\n";
    print "directory of your Csmith tree before running this script.\n";
    exit(-1);
}

my $cnt = $ARGV[0];
$cnt = -1 if (!defined($cnt));
$EXTRA_OPTIONS = $ARGV[1] if (@ARGV==2);
print "extra = $EXTRA_OPTIONS\n";
generate_tests ($cnt);

##################################################################
