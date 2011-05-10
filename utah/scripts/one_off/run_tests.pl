#!/usr/bin/perl -w

use strict;
use File::stat;

my $N = 1000000;

my $MIN_SIZE = 10000;

my $VALGRIND = "";
# my $VALGRIND = "valgrind -q";

sub runit ($) {
    my $cmd = shift;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my $n = 0;

my %totals = ();
my %counts = ();
my %max = ();
my %max_seed = ();
my $x=0;

while (1) {
    my $seed = int(rand(4000000000));
    #my $seed = $x++;
    my $res = runit ("$VALGRIND ../src/csmith -s $seed --bitfields ia32 --packed-struct -o foo.c 2>>errs.txt");
    if ($res == 0) {
	if (stat("foo.c")->size >= $MIN_SIZE) {
	    open INF, "<foo.c" or die;
	    while (my $line = <INF>) {
		if ($line =~ /^XXX (.*): ([+-]?(\d+\.\d+|\d+\.|\.\d+|\d+)([eE][+-]?\d+)?)$/) {
		    my $stat = $1;
		    my $num = $2;
		    $counts{$stat}++;
		    if (!defined($max{$stat})) {
			$max{$stat} = $num;
		    } else {
			if ($num > $max{$stat}) {
			    $max{$stat} = $num;
			    $max_seed{$stat} = $seed;
			}
		    }
		    $totals{$stat} += $num;
		    next;		    
		}
		die "oops bad stats line '$line'" if ($line =~ /XXX/);
	    }
	    if ($n%10000==0) {
		print "$n\n";
	    }
	    $n++;
	    last if ($n == $N);
	}
    } else {
	print "failed for seed $seed\n";
    }
}

foreach my $k (sort keys %totals) {
    my $count = $counts{$k};
    my $avg = sprintf "%.2f", $totals{$k} / $count;
    my $max = sprintf "%.2f", $max{$k};
    my $s = $max_seed{$k};
    print "$k\n";
    print "    occurred $count times, max value= $max, max seed = $s, average value= $avg\n";
}
