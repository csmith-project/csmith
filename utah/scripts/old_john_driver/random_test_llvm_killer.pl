#!/usr/bin/perl -w

use strict;

while (1) {

    my $now;

    open INF, "date |" or die;
    my $d = <INF>;
    chomp $d;
    close INF;
    if ($d =~ /([0-9]+):([0-9]+):([0-9]+)/) {
	my $h = $1;
	my $m = $2;
	$now = ($h*60)+$m;
    } else {
	die;
    }

    print "now = $now\n";

    open INF, "ps ax -o comm=,pid=,pgrp=,bsdstart= |" or die;
    while (my $line = <INF>) {
	chomp $line;
	next if (!($line =~ /^cc1/));
	die if (!($line =~ /\s+([0-9]+)\s+(.*)\s+(.*)$/));	
	my $pid = $1;
	my $pgrp = $2;
	my $stime = $3;
	my $age = 999;
	if ($stime =~ /([0-9]+):([0-9]+)/) {
	    my $h = $1;
	    my $m = $2;
	    my $t = ($h*60)+$m;
	    $age = $now - $t;
	}
	print "$line age=$age\n";
	print "pid=$pid start time=$stime age=$age\n";
	if ($age >= 3 || $age < 0) {
	    my $cmd = "kill -9 $pid";
	    print "executing: $cmd\n";
	    system "$cmd";
	}
    }
    close INF;

    sleep (30);

}
