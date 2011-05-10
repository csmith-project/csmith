#!/usr/bin/perl -w

use strict;

my $SIZE = 50;
my $MAX_RETRY = 4;

my $total_tests = 0;

my $REPS = 10000;

for (my $rep = 0; $rep < $REPS; $rep++) {

my %results;

my $flip = int (rand ($SIZE-1))+2;
#print "flip element is $flip\n";
for (my $i=1; $i<=$SIZE; $i++) {
    if ($i > 1 &&
	$i < $SIZE &&
	$i != $flip &&
	rand() < 0.25) {
	# leave undefined
    } else {
	if ($i < $flip) {
	    $results{$i} = 0;
	} else {
	    $results{$i} = 1;
	}
    }
}

my $hi = $SIZE;
my $lo = 1;
my $tests = 0;

OUTER: 
while (1) {
    die if ($lo >= $hi);
    die if ($results{$lo} != 0);
    die if ($results{$hi} != 1);

    last if (($hi-$lo)==1);
    my $retry_count = 0;

  RETRY:
    my $mid = $lo + 1 + int (rand ($hi-$lo-1));
    #print "lo = $lo, hi = $hi, mid = $mid\n";
    die if ($mid <= $lo);
    die if ($mid >= $hi);
    $tests++;

    if (!(defined($results{$mid}))) {
	if ($retry_count < $MAX_RETRY) {
	    $retry_count++;
	    #print "retry $retry_count of $MAX_RETRY\n";
	    goto RETRY;
	}
	# linear scan
	#print "backing off to linear scan\n";
	for (my $i=$lo+1; $i<=$hi; $i++) {
	    my $x = $results{$i};
	    $tests++;
	    if (defined($x) &&
		$x == 1) {
		$hi = $i;
		last OUTER;
	    }
	}
	die;
    } else {
	# regular binary search
	if ($results{$mid} == 0) {
	    $lo = $mid;
	} else {
	    $hi = $mid;
	}
    }
}

die if ($results{$lo} != 0);
die if ($results{$hi} != 1);
print "first broken version is $hi\n";
die if ($hi != $flip);

$total_tests += $tests;
}

my $av = (0.0 + $total_tests) / $REPS;

print "$av average tests\n";
