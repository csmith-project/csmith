#!/usr/bin/perl -w

use strict;

sub runit ($) {
    my $cmd = shift;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my $n = 0;

my %totals = ();
my %counts = ();
my %all_out;

while (1) {
    my $seed = int(rand(4000000000));
    my $res = runit ("../src/csmith -s $seed > foo.c");
    my @opts = ("0", "1", "2", "s", "3");
    foreach my $opt (@opts) {
	$res = runit ("RunSafely.sh 10 1 /dev/null out.txt clang -ccc-host-triple msp430-generic-generic -ccc-clang-archs msp430 -x c -O$opt -S foo.c -w -I/home/regehr/csmith/src > /dev/null 2>&1");
	open INF, "<out.txt" or die;
	my $output = "";
	while (my $line = <INF>) {
	    $output .= $line;
	}
	close INF;
	if ($output =~ /Cannot yet select/ ||
	    $output =~ /Do not know how to legalize this operator/) {
	} else {
	    $all_out{$output} = "$opt $seed";
	}
    }
    $n++;
    print "$n\n";
    last if ($n == 5000);
}

foreach my $x (keys %all_out) {
    print "\n\n********************************************** $all_out{$x} \n\n\n";
    print $x;
}
