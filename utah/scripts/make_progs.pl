#!/usr/bin/perl -w

use strict;
use File::stat;

sub runit ($) {
    my $cmd = shift;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my $n=0;
while (1) {
    last if ($n == 25);
    my $fn = sprintf "output/prog_%02d.c", $n;
    print "$fn\n";
    runit ("../src/csmith > $fn");
    next if (!(-e $fn));
    my $filesize = stat($fn)->size;
    next if ($filesize < 15000 ||
	     $filesize > 100000);
    $n++;    
}
