#!/usr/bin/perl -w

use strict;

my $CSMITH_PATH = $ENV{"CSMITH_PATH"};
die if (!defined($CSMITH_PATH));

my $outfile = "orig_csmith_output.c";
for (my $i=0; $i<scalar(@ARGV); $i++) {
    if ($ARGV[$i] eq "--output") {
	$outfile = $ARGV[$i+1];
    }
}
die if (!defined($outfile)); 

system "${CSMITH_PATH}/src/csmith @ARGV";

system "gcc -E -I${CSMITH_PATH}/runtime -DCSMITH_MINIMAL $outfile | ${CSMITH_PATH}/scripts/shorten.pl > small.c";

