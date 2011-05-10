#!/usr/bin/perl -w

use strict;

my $num;
my $cur;
my %times;
my %csums;
my $n_terminate;
my $n_nonterminate;

my $all_term = 0;
my $all_nonterm = 0;
my $mixed = 0;
my $n = 0;
my $repro;
my %outs;

while (my $line = <STDIN>) {
    #print "$line";
    chomp $line;
    if ($line =~ /\-\-\-\-\-\- RANDOM PROGRAM ([0-9]+) \-\-\-\-\-\-/) {
	$num = $1;
	%times = ();
	%csums = ();
	$n_terminate = 0;
	$n_nonterminate = 0;
	undef($cur);
	undef($repro);
	next;
    }
    if ($line =~ /regenerate/) {
	$repro = $line;
    }
    if ($line =~ /^\[ia32\] (.*) : /) {
	$cur = $1;
	next;
    }
    if ($line =~ /^cpu time= ([0-9\.]+)$/) {
	die $line if (!defined($cur));
	my $time = $1;
	$times{$cur} = $time;
	next;
    }
    if ($line =~ /^checksum = ([0-9a-zA-Z]+)$/) {
	die if (!defined($cur));
	my $csum = $1;
	$csums{$cur} = $csum;
	if ($csum eq "TIMEOUT") {
	    $n_nonterminate++;
	} else {
	    $n_terminate++;
	}
	next;
    }
    if ($line =~ /GOOD PROGRAM/) {
	die if (!defined($cur));
	if ($n_terminate == 0) {
	    $all_nonterm++;
	} elsif ($n_nonterminate == 0) {
	    $all_term++;
	} else {
	    $mixed++;
	    my $out = "";
	    $out .= "---------- program $n ----------\n";
	    $out .= "$repro\n";
	    my $max = 0;
	    my $min = 100000;
	    foreach my $k (sort keys %times) {
		$out .= "$times{$k} $k $csums{$k}\n";
		my $t = $times{$k};
		if ($t > $max) { $max = $t; }
		if ($t < $min) { $min = $t; }
	    }
	    if ($min<0.01) { $min = 0.01; }
	    my $ratio = $max/$min;
	    $out .= "ratio: $ratio\n";
	    $out .= "\n";
	    $outs{$ratio} = $out;
	}
	$n++;
    }
}

sub bynum {
    return $a <=> $b;
}

foreach my $k (sort bynum keys %outs) {
    print $outs{$k};
}

print "\n";
print "$n good tests\n";
print "${all_nonterm} non-terminating, ${all_term} terminating, ${mixed} mixed\n";

