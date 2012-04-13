#!/usr/bin/perl -w

use strict;
use File::Temp qw/ tempfile tempdir /;

my $args = "";
foreach (@ARGV) {
    $args .= "$_ ";
}

system "$args > errs.txt 2>&1";

open INF, "<errs.txt" or die;
my $errs;
while (<INF>) {
    $errs .= $_;
}
close INF;

if (($args =~ /gcc/ && $errs =~ /ternal compiler/) ||
    ($args =~ /clang/ && $errs =~ /Assertion/)) {
    ($fh, $filename) = tempfile( "tmpfileXXXXXX" );
    system "cp small.c $filename";
    open OUTF, ">>$filename" or die;
    print OUTF "// $args\n";
    close OUTF;
}
