#!/usr/bin/perl -w

use strict;
use File::stat;

my $N = 50000;

my $MIN_SIZE = 1;

my $VALGRIND = "";
# my $VALGRIND = "valgrind -q";

sub runit ($) {
    my $cmd = shift;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my $n = 0;

my %totals = ();
my %counts = ();
my %max = ();
my %max_seed = ();
my $x=0;

my %warn_size = ();
my %warn_seed = ();

while (1) {
    my $seed = int(rand(4000000000));
    #my $seed = $x++;
    my $res = runit ("RunSafely.sh 60 0 /dev/zero output.txt ../src/csmith -s $seed --bitfields ia32 --packed-struct --random-random > foo.c");
    if ($res == 0) {
	if (stat("foo.c")->size >= $MIN_SIZE) {
	    open INF, "<foo.c" or die;
	    while (my $line = <INF>) {
		if ($line =~ /^XXX (.*): ([+-]?(\d+\.\d+|\d+\.|\.\d+|\d+)([eE][+-]?\d+)?)$/) {
		    my $stat = $1;
		    my $num = $2;
		    $counts{$stat}++;
		    if (!defined($max{$stat})) {
			$max{$stat} = $num;
		    } else {
			if ($num > $max{$stat}) {
			    $max{$stat} = $num;
			    $max_seed{$stat} = $seed;
			}
		    }
		    $totals{$stat} += $num;
		    next;		    
		}
		die "oops bad stats line '$line'" if ($line =~ /XXX/);
	    }
	    if ($n%100==0) {
		print "$n\n";
	    }
	    $n++;
	    last if ($n == $N);

	    system "rm -f out.txt";
	    system "current-gcc -O2 -Wall foo.c -I/home/regehr/csmith/src >out.txt 2>&1";
	    open INF, "<out.txt" or die;
	    while (<INF>) {

		next if (/At top level:/);
		next if (/In function \‘(.*)\’:/);

		if (/: warning: (.*)$/) {
		    my $warn = $1;
		    next if ($warn =~ /unused variable \‘/);
		    next if ($warn =~ /comparison is always true due to limited range of data type/);
		    next if ($warn =~ /overflow in implicit constant conversion/);
		    next if ($warn =~ /large integer implicitly truncated to unsigned type/);
		    next if ($warn =~ /comparison is always false due to limited range of data type/);
		    next if ($warn =~ /‘safe_(.*)\’ defined but not used/);
		    next if ($warn =~ /‘crc32_8bytes\’ defined but not used/);
		    next if ($warn =~ /address of \‘(.*)\’ will never be NULL/);		    
		    next if ($warn =~ /function returns address of local variable/);
		    next if ($warn =~ /may be used uninitialized/);

		    if ($warn =~ /\‘(.*)\’ is used uninitialized in this function/) {
			my $var = $1;
			die if (!($warn =~ s/$1/some_variable/));
		    }

		    my $size = stat("foo.c")->size;
		    if (!defined($warn_size{$warn}) || ($size < $warn_size{$warn})) {
			$warn_size{$warn} = $size;
			$warn_seed{$warn} = $seed;
		    }
		}

		#print;
	    }
	    close INF;

	}
    } else {
	print "failed for seed $seed\n";
    }
}

foreach my $k (sort keys %totals) {
    my $count = $counts{$k};
    my $avg = sprintf "%.2f", $totals{$k} / $count;
    my $max = sprintf "%.2f", $max{$k};
    my $s = $max_seed{$k};
    print "$k\n";
    print "    occurred $count times, max value= $max, max seed = $s, average value= $avg\n";
}

print "\n\n";

foreach my $k (sort keys %warn_size) {
    print "$k size=$warn_size{$k} seed=$warn_seed{$k}\n";
}
