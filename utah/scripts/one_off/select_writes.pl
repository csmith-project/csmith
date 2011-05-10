#!/usr/bin/perl -w

use strict;

while (<STDIN>) {
    if (/ ([0-9]+ writes)/) {
	print "$1\n";
    }
}
exit 0;
