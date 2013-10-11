#!/usr/bin/perl -w

use strict;
use File::Temp;

my $strings = $ARGV[0];
die unless defined ($strings);
my $crashfile = $ARGV[1];
die unless defined ($crashfile);
my $dir = $ARGV[2];
die unless defined ($dir);

my $err;
open INF, "<$crashfile" or die;
while (my $line = <INF>) {
    chomp $line;
    if ($line =~ /internal compiler error: (.*)$/) {
	$err = $1;
    }
}
close INF;
exit 0 unless defined ($err);

my $lines = "";
my $found = 0;
my $cnt;
open INF, "<$strings" or die;
while (my $line = <INF>) {
    die unless ($line =~ /^([0-9]+) <<< (.*) >>>$/);
    if ($2 eq $err) {
	$found = 1;
    }
}
close INF;

if (!$found) {
    open OUTF, ">>../crash_strings.txt" or die;
    print OUTF "1 <<< $err >>>\n";
    close OUTF;
    my $tmpfn = File::Temp::tempnam ($dir, "crashXXXXXX");
    system "cp small.c ${tmpfn}.c";
}

exit 0
