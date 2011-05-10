#!/usr/bin/perl -w

use strict;

my $DIR = $ARGV[0];

die if (!defined($DIR));

sub runit ($) {
    my $cmd = shift;

    if ((system "$cmd") != 0) {
	print "system '$cmd' failed: $?";
	return -1;
    }
    
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my @gcc_opts = ("-O0",
		"-O1",
		"-O2",
		"-O3",
		"-Os");

my @no_opts = ("");

my @llvm = ("llvm-gcc", \@gcc_opts);
my @gcc = ("current-gcc", \@gcc_opts);
my @clang1 = ("clang-driver-opt", \@no_opts);
my @clang2 = ("clang-driver-noopt", \@no_opts);

my @ccs = (\@llvm, \@gcc, \@clang1, \@clang2);

my $n = 0;
while (1) {
    print "$n\n";
    $n++;
    system "rm -f prog";
    system "quest -n 200 -s 3 > $DIR/prog_$n.c";
    foreach my $ccref (@ccs) {
	(my $cc, my $optref) = @{$ccref};
	my @opts = @{$optref};
	foreach my $opt (@opts) {
	    my $compile = "$cc $DIR/prog_$n.c -o $DIR/prog $opt";
	    print "$compile : ";
	    my $res1 = runit ("$compile > /dev/null 2>&1");
	    if ($res1==0) {
		my $res2 = runit ("$DIR/prog");
		if ($res2==0) {
		    print "TEST GOOD\n";
		} else {
		    print "TEST FAIL\n";
		}
	    } else {
		print "COMPILER FAIL\n";
	    }
	}
    }
    system "bzip2 -9qf $DIR/prog_$n.c";
}
