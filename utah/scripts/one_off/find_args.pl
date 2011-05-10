#!/usr/bin/perl -w

use strict;

my %opts;

while (my $line = <STDIN>) {
    my @l = split (" ", $line);
    my $f = shift (@l);

    next if ($f ne "gcc");

    my $i = 0;
    my $bad = 0;

    while (1) {
	last if ($i >= scalar (@l));
	my $opt = $l[$i];
	if ($opt =~ /\.S$/) {
	    $bad = 1;
	    $i = 1000000;
	} elsif ($opt eq "-isystem" ||
	    $opt eq "-o" ||
	    $opt eq "-include") {
	    splice (@l, $i, 2);
	} elsif (
	    $opt eq "-pipe" ||
	    $opt =~ /^\-D/ ||
	    $opt =~ /^\-W/ ||
	    $opt =~ /^\-I/ ||
	    $opt eq "-nostdinc" ||
	    $opt eq "-nostdlib" ||
	    $opt eq "-ffreestanding" ||
	    $opt eq "-fPIC" ||
	    $opt =~ /\.c$/ ||
	    $opt =~ /\.o$/ ||
	    $opt eq "-c"
	    ) {
	    splice (@l, $i, 1);
	} else {
	    $i++;
	}
    }

    if (!$bad) {
	my @l2 = sort @l;
	@l = @l2;
	my $s = "";
	foreach my $opt (@l) {
	    $s .= "$opt ";
	}
	$opts{$s}++;
    }
}

sub bynum {
    return $opts{$a} <=> $opts{$b};
}

foreach my $opt (sort bynum keys %opts) {
    print "$opts{$opt} $opt\n";
}
