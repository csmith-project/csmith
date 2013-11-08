#!/usr/bin/perl -w

use strict;
use File::Temp;

my $strings = $ARGV[0];
die unless defined ($strings);
my $dir = $ARGV[1];
die unless defined ($dir);
my $comp = $ARGV[2];
die unless defined ($comp);

my @opts = (
    #"-O0",
    #"-O1",
    #"-O2",
    "-O3",
    #"-Os",
    );

foreach my $opt (@opts) {
    
    my $cmd = "$comp $opt -c -w small.c > crash.txt 2>&1";
    print "$cmd\n";
    system $cmd;

    my $err;
    open INF, "<crash.txt" or die;
    while (my $line = <INF>) {
	chomp $line;
	if ($line =~ /internal compiler error: (.*)$/) {
	    $err = $1;
	}
    }
    close INF;
    next unless defined ($err);
    
    my $lines = "";
    my $found = 0;
    my $cnt;

    open INF, "<$strings" or die;
    while (my $line = <INF>) {
	chomp $line;
	die unless ($line =~ /^([0-9]+) <<< (.*) >>>$/);
	if ($2 eq $err) {
	    $found = 1;
	}
    }
    close INF;
    
    if (!$found) {
	open OUTF, ">>$strings" or die;
	print OUTF "10000000 <<< $err >>>\n";
	close OUTF;
	my $tmpfn = File::Temp::tempnam ($dir, "crashXXXXXX").".c";
	system "cp small.c ${tmpfn}";
	open OUTF, ">>${tmpfn}" or die;
	print OUTF "// '$err' <- '$comp $opt'\n";
	close OUTF;
    }
}

exit 0
