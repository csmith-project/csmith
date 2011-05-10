#!/usr/bin/perl -w

use strict;
use File::stat;

my $CSMITH = $ENV{"CSMITH_PATH"}."/src/csmith";

my $N = 1000;

for (my $funcs = 1; $funcs < 100; $funcs++) {
    my $total = 0.0;
    my $block = 1 + int ($funcs / 10);    
    for (my $n=0; $n<$N; $n++) {
	system "rm -f foo.c";	
	system "$CSMITH --max-funcs $funcs --max-block-size $block --concise > foo.c";
	if (-f "foo.c") {
	    $total += stat("foo.c")->size;
	} else {
	    $n--;
	}
    }
    my $avg = $total / $N;
    print "$funcs $block $avg\n";
}
