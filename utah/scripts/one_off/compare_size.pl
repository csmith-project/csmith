#!/usr/bin/perl -w

use strict;
use warnings;

my %count;
my %cycles1;
my %bytes1;
my %cycles2;
my %bytes2;

my @versions = ("4.1.2", "4.2.2");
my @opts = ("0","1","2","s","3");

while (<STDIN>) {
    
    if (/^([a-zA-Z0-9\.]+) ([0-9]+) cycles ([0-9]+) bytes$/) {
	my $name = $1;
	my $cyc = $2;
	my $byt = $3;

	die if (!($name =~ /^rand([0-9]+)avrgcc([0-9\.]+)O([0123s])\.elf/));
	my $prog = $1;
	my $version = $2;
	my $opt = $3;

	$cycles1{$prog}{$version}{$opt} = $cyc;
	$bytes1{$prog}{$version}{$opt} = $byt;
	$cycles2{$prog}{$opt}{$version} = $cyc;
	$bytes2{$prog}{$opt}{$version} = $byt;
	$count{$prog}++;
	
	next;
    }
}

my $n = 0;
my %t_cycles;
my %t_bytes;

foreach my $prog (keys %cycles1) {
    next if ($count{$prog} != 10);
    $n++;
    foreach my $ver (@versions) {
	foreach my $opt (@opts) {
	    $t_cycles{$ver}{$opt} += $cycles1{$prog}{$ver}{$opt};
	    $t_bytes{$ver}{$opt} += $bytes1{$prog}{$ver}{$opt};
	}
    }
}

print "averaging over $n programs\n";

{
    foreach my $opt (@opts) {
	foreach my $ver (@versions) {
	    my $a_cycles = int (($t_cycles{$ver}{$opt} * 1.0) / $n);
	    my $a_bytes = int (($t_bytes{$ver}{$opt} * 1.0) / $n);
	    print "$ver -O$opt $a_cycles cycles, $a_bytes bytes\n";
	}
    }
}
