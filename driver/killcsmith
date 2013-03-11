#!/usr/bin/perl -w

use strict;

open INF, "ps axuww |" or die;
while (<INF>) {
    next if (!(/random\_test/));
    my @l = split /\s+/;
    my $pid = $l[1];
    print "killing $pid\n";
    system "kill -9 $pid";
}
close INF;
