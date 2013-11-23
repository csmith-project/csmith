#!/usr/bin/perl -w 

use strict;
use Text::LevenshteinXS qw(distance);

# TODO:
#   prefer smaller test cases?
#   handle all compilers, not just one
#   support more distance functions
#   work out of a tamer arena, not out of the work dirs
#     probably a different script moves reduce dires into the arena?
#   figure out whose job it is to re-verify bugs when the compiler is rebuilt
#   process only up to some maximum number of files?

my $compiler = "gcc";
my $MAX_SIZE = 512;

my $BAR = "------------------------------------------------------------------\n";

sub read_file ($) {
    (my $fn) = @_;
    open INF, "<$fn" or die;
    my $s = "";
    while (my $line = <INF>) {
	chomp $line;
	$s .= " " . $line;
    }
    close INF;
    return $s;
}

sub max ($$) {
    (my $x, my $y) = @_;
    if ($x>$y) {
	return $x;
    } else {
	return $y;
    }
}

sub calc_dist ($$) {
    (my $dir1, my $dir2) = @_;
    my $s1 = read_file ("$dir1/small.c");
    my $s2 = read_file ("$dir2/small.c");
    my $dist = distance ($s1, $s2) / max (length($s1),length($s2));
    return $dist;
}

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
	    close INF;
	    chomp $line;
	    next unless ($line eq $compiler);
	    my $small = "$reduce/small.c";
	    next unless (-e $small);
	    next unless ((-s $small) <= $MAX_SIZE);
	    push @dirs, $reduce;
	}
    }
}

print "found $nwork work dirs and ".(scalar @dirs)." reduce dirs for $compiler wrong-code bugs.\n";

# calculate distances using different metrics

my %dist;
foreach my $dir1 (@dirs) {
    foreach my $dir2 (@dirs) {
	$dist{$dir1}{$dir2} = calc_dist ($dir1, $dir2);
    }
}

my %ranked;
my @rank;

# for now select the highest ranked test case randomly
my $start = int (rand @dirs);
push @rank, $dirs[$start];
$ranked{$dirs[$start]} = 1;

print $BAR;
system "cat $dirs[$start]/small.c";

while (1) {
    my $maxdist = 0.0;
    my $maxdir;
    foreach my $dir1 (@dirs) {
	next if ($ranked{$dir1});
	my $mindist = 1e99;
	foreach my $dir2 (@rank) {
	    my $d = $dist{$dir1}{$dir2};
	    if ($d < $mindist) {
		$mindist = $d;
	    }
	}
	if ($mindist > $maxdist) {
	    $maxdist = $mindist;
	    $maxdir = $dir1;
	}
    }
    last unless defined $maxdir;
    push @rank, $maxdir;
    $ranked{$maxdir} = 1;
    print $BAR;
    system "cat $maxdir/small.c";
    printf "%.3f\n", $maxdist;
}
print $BAR;

# create ranked lists in a nice web format

