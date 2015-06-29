#!/usr/bin/perl -w
##
## Copyright (c) 2015-2016 Xuejun Yang
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
use File::Copy;

## This script validate Csmith against a baseline test suite. It verifies that 
## given same generation options in a baseline file, Csmith should generate 
## the same output. If the generation logic of Csmith is indeed changed, the
## baseline test suite should be updated with "-update" command line option.

#######################################################################
 
my $CSMITH_HOME = $ENV{"CSMITH_HOME"};   
my $UPDATE = 0;

####################################################################### 

# properly parse the return value from system()
sub runit ($$) {
    my ($cmd, $out) = @_;
    # print "about to run '$cmd'\n";
    my $res = system "$cmd";
    my $exit_value  = $? >> 8;
    $exit_value = $? & 127 if ($? & 127);
    return $exit_value;
}  

sub validate_test ($) {
	(my $file) = @_;  
    open(FILE, "<", "$file") or die("Can't open $file\n");

	# read the baseline file, and find the generation options (including seed)
	# and the length of the header so we can skip comparing the headers.
	my @lines = <FILE>;  
	my $options = "";
	my $seed = "";
	my $header_len = 0;
	for (my $i=0; $i< scalar(@lines); $i++) {
		if ($lines[$i] =~ /\* Options:\s+(.+?)\s+--output/) {
			$options = $1;
		}
		elsif ($lines[$i] =~ /\* Seed:\s+(.+?)\s+$/) {
			$seed = $1;
		}
		$header_len++;
		
		last if ($options ne "" && $seed ne "");
	}
	close(FILE);
	
	unlink("tmp.c"); 
	# generate random program with the new version of Csmith and the same options
    my $cmd = "$CSMITH_HOME/src/csmith $options -s $seed --output tmp.c";
    my $res = runit ($cmd, "csmith.out"); 
    if ($res != 0 || !(-f "tmp.c") ) {
		print "Failed (code: " . $res . ") to generate program: $cmd\n";
		return 0;
    }  
	
	# compare the generated random program with base line
	open(FILE, "<", "tmp.c") or die("Can't open tmp.c\n");
	my @lines2 = <FILE>;
	close(FILE);
	
	if (scalar(@lines) != scalar(@lines2)) {
		print "Generated program has wrong size with $cmd\n";
		return 0;
	}
	
	for (my $j=$header_len; $j< scalar(@lines2); $j++) {
	    if ($lines[$j] =~ /statistics/) {
		    last;
		}

		# convert to unix-style line ending
		$lines[$j] =~ s/\r\n$/\n/;
		$lines2[$j] =~ s/\r\n$/\n/;

		if ($lines[$j] ne $lines2[$j]) {
			print "Generated program has wrong content with $cmd\n";
			print "line $j in base: <$lines[$j]>\n";
			print "line $j in generated: <$lines2[$j]>\n";
			return 0;
		}
	}
	 
	print "Generated program matched base line\n";
	return 1;
}

sub validate_tests () {
	my $cnt = 0;
	my $fail = 0;
	my @files = <$CSMITH_HOME/testsuite/test*.c>;
	foreach my $file (@files) {
		print $file . "\n";
		$cnt++;
		if (!validate_test($file)) {
			$fail++;
			if ($UPDATE) {
				copy("tmp.c", $file);
			}
		}
	}
	unlink("tmp.c");
	print "$fail out of $cnt test cases in the suite ". ($UPDATE ? "updated" : "failed") . "\n";
} 

########################### main ##################################

if (!(-f "$CSMITH_HOME/runtime/csmith.h")) {
    print "Please point the environment variable CSMITH_HOME to the top-level\n";
    print "directory of your Csmith tree before running this script.\n";
    exit(-1);
}  

$UPDATE = 1 if (@ARGV > 0 && $ARGV[0] eq "-update");
validate_tests ();

##################################################################
