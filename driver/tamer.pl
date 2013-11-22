#!/usr/bin/perl -w 

use strict;

my @dirs;
my $nwork = 0;
{
    my @workdirs = glob "work*";
    foreach my $work (@workdirs) {
	next unless (-d $work);
	$nwork++;
	my @reduces = glob "$work/reduce_*";
	foreach my $reduce (@reduces) {
	    print "$reduce\n";
	    push @dirs, $reduce;
	}
    }
}

print "today we're just looking at wrong-code bugs for GCC.\n";
print "got $nwork work dirs and ".(scalar @dirs)." reduce dirs.\n";

