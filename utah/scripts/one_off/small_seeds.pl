#!/usr/bin/perl -w

use strict;

my $seed;
my $size;
my %crash;
my %wrong;

while (my $line = <STDIN>) {
    if ($line =~ /RANDOM PROGRAM/) {
	undef ($seed);
	next;
    }
    if ($line =~ /seed = ([0-9]+)$/) {
	$seed = $1;
	next;
    }
    if ($line =~ /is ([0-9]+) bytes/) {
	$size = $1;
	next;
    }
    if ($line =~ /COMPILER FAILED gcc/) {
	die if (!defined($seed));
	die if (!defined($size));
	$crash{$seed} = $size;
    }
    if ($line =~ /CHECKSUM FAIL/) {
	die if (!defined($seed));
	die if (!defined($size));
	$wrong{$seed} = $size;
    }
}

sub bysize1 {
    return $wrong{$a} <=> $wrong{$b};
}

sub bysize2 {
    return $crash{$a} <=> $crash{$b};
}

print "wrong code:\n";

foreach my $s (sort bysize1 keys %wrong) {
    print "  seed= $s size= $wrong{$s}\n";
}

print "crash:\n";

foreach my $s (sort bysize2 keys %crash) {
    print "  seed= $s size= $crash{$s}\n";
}
