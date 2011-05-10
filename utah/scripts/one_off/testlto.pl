#!/usr/bin/perl -w

use strict;
use warnings;

sub runit ($) {
    my $cmd = shift;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my $n=0;

while (1) {
    $n++;
    print "program $n\n";
    runit ("rm -rf output");
    runit ("../src/csmith --max-split-files 3");    
    chdir "output" or die;
    runit ("current-gcc -w -O3 -flto -c -I/home/regehr/csmith/src rnd_output0.c >/dev/null 2>&1");
    runit ("current-gcc -w -O3 -flto -c -I/home/regehr/csmith/src rnd_output1.c >/dev/null 2>&1");
    runit ("current-gcc -w -O3 -flto -c -I/home/regehr/csmith/src rnd_output2.c >/dev/null 2>&1");
    runit ("current-gcc -w -O3 -flto *.o -o rand1 -fwhole-program >/dev/null 2>&1");
    runit ("current-gcc -w -O3 -I/home/regehr/csmith/src *.c -o rand2 >/dev/null 2>&1");
    runit ("llvm-gcc -w -O3 -I/home/regehr/csmith/src *.c -o rand3 >/dev/null 2>&1");
    runit ("RunSafely.sh 3 0 /dev/zero output1 ./rand1");
    runit ("RunSafely.sh 3 0 /dev/zero output2 ./rand2");
    runit ("RunSafely.sh 3 0 /dev/zero output3 ./rand3");
    runit ("cat output1 | grep check");
    runit ("cat output2 | grep check");
    runit ("cat output3 | grep check");
    chdir ".." or die;
}

