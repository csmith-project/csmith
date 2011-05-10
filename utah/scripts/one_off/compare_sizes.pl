#!/usr/bin/perl 

use strict;
use warnings;

sub code_size ($) {
    (my $fn) = @_;
    open INF, "size -d $fn |" or die;
    my $junk = <INF>;
    my $line = <INF>;
    die if (!($line =~ /^\s+([0-91]+)\s+/));
    my $size = $1;
    close INF;
    return $size;
}

my $s1 = code_size ($ARGV[0]);
my $s2 = code_size ($ARGV[1]);

my $ratio = (0.0+$s1)/$s2;

print "  ( $s1 $s2 $ratio )\n";

if ($ratio > 8.0) {
    exit 0;
} else {
    exit -1;
}
