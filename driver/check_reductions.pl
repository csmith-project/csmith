#!/usr/bin/perl -w

use strict;
use Time::Local;

open INF, "<work0/output.txt" or die;
while (my $line = <INF>) {
    chomp $line;
    print "$line\n";
}
close INF;
