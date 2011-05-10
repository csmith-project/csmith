#!/usr/bin/perl -w

use strict;

my @hosts = (
	     "node-1.tribe2.alchemy.emulab.net",
	     "node-2.tribe2.alchemy.emulab.net",
	     "node-3.tribe2.alchemy.emulab.net",
	     "node-4.tribe2.alchemy.emulab.net",
	     "node-5.tribe2.alchemy.emulab.net",
	     "node-6.tribe2.alchemy.emulab.net",
	     "node-7.tribe2.alchemy.emulab.net",
	     "node-8.tribe2.alchemy.emulab.net",
	     "node-9.tribe2.alchemy.emulab.net",
	     "node-10.tribe2.alchemy.emulab.net",
	      );

my $DIR = "/mnt/volatile_testing/volatile/workXXX";
my $max_host = 10;

my $N=2;

my $num = 20;
foreach my $host (@hosts) {
    for (my $i=0; $i<$N; $i++) {
	$num++;
	my $dir = $DIR;
	($dir =~ s/XXX/$i/);	
	my $cmd = "scp $host:$dir/output.txt output/output$num.txt";
	print "$cmd\n";
	system $cmd;
	# system "bzip2 -9v output/output$n.txt";
    }
}
