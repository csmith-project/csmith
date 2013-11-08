#!/usr/bin/perl -w

use strict;
use Time::Local;
use Date::Parse;

sub go ($) {
    (my $n) = @_;

    my $first;
    my $last;
    my $reduce_start;
    my $reductions = 0;
    my $reduction_time = 0;
    
    open INF, "<work${n}/output.txt" or die;
    while (my $line = <INF>) {
	chomp $line;
	if ($line =~ /timestamp: (.*)$/) {
	    my $s = $1;
	    my $date = str2time ($s);
	    if (!defined($first)) {
		$first = $date;
	    }
	    $last = $date;
	}
	if ($line =~ /timestamp for creduce start: (.*)$/) {
	    my $s = $1;
	    my $date = str2time ($s);
	    die if defined ($reduce_start);
	    $reduce_start = $date;
	}
	if ($line =~ /timestamp for creduce stop: (.*)$/) {
	    my $s = $1;
	    my $date = str2time ($s);
	    $reductions++;
	    die unless defined ($reduce_start);
	    my $dur = ($date - $reduce_start) / 60.0;
	    $reduction_time += $dur;
	    # print "reduction lasted $dur minutes\n";
	    undef ($reduce_start);
	}
    }
    close INF;
    
    my $dur = ($last - $first) / 60.0;
    if ($dur > 0) {
	print "$n : fuzzing lasted $dur minutes; ";
	my $pct = 100.0*$reduction_time/$dur;
	print "$pct % of time was spent in $reductions reductions\n";
    }
}

for (my $i=0; $i<32; $i++) {
    go ($i);
}
