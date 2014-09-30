#!/usr/bin/perl -w

use strict;

my @dirs = glob "reduce_wrong_*";

foreach my $dir (@dirs) {
    next unless -f "$dir/WRONG_CODE_REDUCTION_FINISHED";
    my $size = -s "$dir/small.c";
    next unless $size < 500;
    print "======================== $dir ============================\n\n";
    system "cat $dir/small.c";
    print "\n";
}
