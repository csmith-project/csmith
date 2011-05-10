#!/usr/bin/perl -w

use strict;

my $SIZE = 50;

my $total_tests = 0;

my $REPS = 10000;

my %results;
my $tests;

sub check_results ($) {
    (my $i) = @_;
    my $r = $results{$i};
    my $foo;
    if (!(defined($r))) {
	$foo = "x";
    } else {
	$foo = $r;
    }
    #print "checking $i (result = $foo)\n";
    $tests++;
    return $r;
}

for (my $rep = 0; $rep < $REPS; $rep++) {

%results = ();
$tests = 0;

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
	    $results{$i} = 1;
	} else {
	    $results{$i} = 0;
	}
    }
}

my $lo = 1;
my $hi = $SIZE;

OUTER: 
while (1) {
    die if ($lo >= $hi);
    die if ($results{$lo} != 1);
    die if ($results{$hi} != 0);

    # normal binsearch termination condition
    last if (($hi-$lo)==1);

    my $mid = int (($hi+$lo)/2);
    goto MORE if (defined(check_results($mid)));

    goto LINEAR if (($hi-$lo)<12);

    $mid = $lo + int (($hi-$lo)*0.750);
    goto MORE if (defined(check_results($mid)));

    $mid = $lo + int (($hi-$lo)*0.250);
    goto MORE if (defined(check_results($mid)));

    $mid = $lo + int (($hi-$lo)*0.125);
    goto MORE if (defined(check_results($mid)));

    $mid = $lo + int (($hi-$lo)*0.375);
    goto MORE if (defined(check_results($mid)));

    $mid = $lo + int (($hi-$lo)*0.675);
    goto MORE if (defined(check_results($mid)));

    $mid = $lo + int (($hi-$lo)*0.825);
    goto MORE if (defined(check_results($mid)));

  LINEAR:
    # linear scan to force termination
    #print "backing off to linear scan\n";
    for (my $i=$lo+1; $i<=$hi; $i++) {
	my $x = check_results ($i);
	if (defined($x) &&
	    $x == 0) {
	    $hi = $i;
	    last OUTER;
	}
    }
    die;
    
  MORE:
    # regular binary search

    #print "lo = $lo, hi = $hi, mid = $mid\n";
    die if ($mid <= $lo);
    die if ($mid >= $hi);

    my $r = check_results ($mid);
    die if (!defined($r));
    if ($r == 0) {
	$hi = $mid;
    } else {
	$lo = $mid;
    }
    next OUTER;

}

die if ($results{$lo} != 1);
die if ($results{$hi} != 0);
print "first broken version is: $hi\n";
die if ($hi != $flip);

$total_tests += $tests;

#print "\n\n";

}

my $av = (0.0 + $total_tests) / $REPS;

print "$av average tests\n";
