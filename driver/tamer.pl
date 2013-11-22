#!/usr/bin/perl -w 

use strict;

# TODO:
#   handle all compilers, not just one
#   work out of a tamer arena, not out of the work dirs
#     probably a different script moves reduce dires into the arena
#   figure out whose job it is to re-verify bugs when the compiler is rebuilt

my $compiler = "gcc";

my @dirs;
my $nwork = 0;
{
    my @workdirs = glob "work*";
    foreach my $work (@workdirs) {
	next unless (-d $work);
	$nwork++;
	my @reduces = glob "$work/reduce_*";
	foreach my $reduce (@reduces) {
	    my $donefn = "$reduce/WRONG_CODE_REDUCTION_FINISHED";
	    open INF, "<$donefn" or next;
	    my $line = <INF>;
	    chomp $line;
	    next unless ($line eq $compiler);
	    close INF;
	    push @dirs, $reduce;
	}
    }
}

print "found $nwork work dirs and ".(scalar @dirs)." reduce dirs for $compiler wrong-code bugs.\n";

foreach my $dir1 (@dirs) {
    foreach my $dir2 (@dirs) {
	next if ($dir1 eq $dir2);
	print "$dir1 $dir2\n";
    }
}
