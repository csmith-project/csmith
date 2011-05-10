#!/usr/bin/perl 

use strict;
use warnings;

my $BUCKET_SIZE = 100;

my %hist;
my $max = 0;
my %sizes;
my $reg;

while (my $line = <STDIN>) {

    chomp $line;

    if ($line =~ /regenerate with/) {
	$reg = $line;
    }

    if ($line =~ /RANDOM PROGRAM/) {
	my $s1 = $sizes{"clang"};
	my $s2 = $sizes{"current-gcc"};
	next if (!defined($s1) ||
		 !defined($s2));
	
	#  $ratio = (0.0+$s1)/$s2;

	#  print "$ratio $s1 $s2 $reg\n";

	print "$s1 $s2\n";

	%sizes = ();
	$reg = "";
	next;
    }

    if ($line =~ /CODE SIZE (.+) WRAP=[01] ([0-9]+)$/) {
	my $compiler = $1;
	my $size = $2+0.0;
	my $bucket = int($size/$BUCKET_SIZE);
	$hist{$compiler}{$bucket}++;
	if ($bucket > $max) {
	    $max = $bucket;
	}
	$sizes{$compiler} = $size;
	next;
    }

}

exit;

foreach my $compiler (sort keys %hist) {
    open OUTF, ">$compiler.txt" or die;
    for (my $i=0; $i<$max; $i++) {
	my $x = $i*$BUCKET_SIZE;
	my $y = $hist{$compiler}{$i};
	if (!defined($y)) {
	    $y = 0;
	}
	print OUTF "$x $y\n";
    }
    close OUTF;
}
