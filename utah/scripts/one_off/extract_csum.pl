#!/usr/bin/perl -w

use strict;

while (my $line = <STDIN>) {
    chomp $line;
    if ($line =~ /r30:r31 = 0x([0-9a-fA-F]+)$/) {
	my $csum = $1;
	print "$csum\n";
	exit 0;
    }
}

exit 1;
