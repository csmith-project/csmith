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

# This script illustrates how to run compiler_test.pl in parallel on multi-core machines. Please make sure
# these processes are terminated at the end of your testing 

#!/usr/bin/perl -w
 use strict;
use warnings;
use Sys::CPU;

sub usage () {
    die "usage: launchn.pl <config-file>\n";
}

my $CONFIG_FILE = "";
my $CPUS = Sys::CPU::cpu_count(); 

if (scalar (@ARGV) != 1 || !(-e $ARGV[0]) ) {
    usage();
} 
print "looks like we have $CPUS cpus\n";

$CONFIG_FILE = $ARGV[0];
for (my $i=0; $i<$CPUS; $i++) {
    my $dir = "work$i";
    system "rm -rf $dir";
    system "mkdir $dir";
    chdir $dir;
    system "nice -19 nohup ../compiler_test.pl 0  ../$CONFIG_FILE > output.txt 2>&1 &";
    print "start working in $dir\n";
    chdir "..";
}

