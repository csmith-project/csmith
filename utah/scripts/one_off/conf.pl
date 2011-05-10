#!/usr/bin/perl -w

#############################################################

use strict;
use warnings;

#############################################################

my $N = 10000;
my $PROB = 0.0001;

my $REPS = 10000;

#############################################################

sub normal_approx ($$) {
    (my $fail, my $n) = @_;
    return 0.0 if ($fail==0);
    my $phat = 1.0 * $fail / $n;
    my $zc = 1.96; # 95%
    my $plus_minus = $zc * sqrt (($phat*(1.0-$phat))/$n);
    return ($phat-$plus_minus,$phat+$plus_minus);
}

sub wilson_score ($$) {
    (my $fail, my $n) = @_;
    my $phat = 1.0 * $fail / $n;
    my $zc = 1.96; # 95%

    my $term1 = 
	$phat + 
	((1.0/(2.0*$n)) * ($zc * $zc));
    
    my $term2 = 
	$zc *
	sqrt ((($phat*(1.0-$phat))/$n) + 
	      (($zc * $zc) / (4.0 * $n * $n))
	);
	 
    my $term3 = 
	1.0 + 
	((1.0/$n) * ($zc * $zc));
	
    my $lo = ($term1 - $term2) / $term3;
    my $hi = ($term1 + $term2) / $term3;

    return ($lo, $hi);
}

#############################################################

sub print_wilson ($$) {
    (my $fail, my $n) = @_;
    (my $lo, my $hi) = wilson_score ($fail, $n);
    $lo *= 100;
    $hi *= 100;
    printf "%f .. %f for %d out of %d\n", $lo, $hi, $fail, $n;
}

if (0) {
    my $level = 1.0/1000000;
    my $t = 1000;
    my $c;
    do {
	$c = wilson_score (0, $t);
	print "$t $c\n";
	$t += 1000;
    } while ($c > $level);
}

if (0) {
    print_wilson (0, 1000);
    print_wilson (0, 10000);
    print_wilson (0, 100000);
    print_wilson (0, 1000000);
    exit 0;
}

if (1) {

    my $inside = 0;
    
    for (my $rep = 0; $rep < $REPS; $rep++) {
	
	my $fail = 0;
	
	for (my $i=0; $i<$N; $i++) {
	    my $r = rand();
	    if ($r < $PROB) {
		$fail++;
	    } else {
	    }
	}
	my $rate = 1.0 * $fail / $N;
	#(my $lo, my $hi) = normal_approx ($fail, $N);
	(my $lo, my $hi) = wilson_score ($fail, $N);
	#print "$fail / $N failures for internal $lo .. $hi  ";
	if ($PROB >= $lo &&
	    $PROB <= $hi) {
	    $inside++;
	    #print ("inside!\n");
	} else {
	    #print ("outside!\n");
	}
    }
    
    my $pct = 100.0 * $inside / $REPS;
    
    print "actual probability was inside the confidence interval $pct % of the time\n";
}

#############################################################

