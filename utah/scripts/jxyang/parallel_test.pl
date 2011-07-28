#!/usr/bin/perl -w
 use strict;
use warnings;
#use Sys::CPU;

sub usage () {
    die "usage: parallel_test.pl <config-file> <csmith-option>\n";
}

my $CPUS = 2; #Sys::CPU::cpu_count();

if (scalar (@ARGV) != 2 || !(-e $ARGV[0]) ) {
    usage();
}
print "looks like we have $CPUS cpus\n";

my $CONFIG_FILE = $ARGV[0];
my $CSMITH_OPTION = $ARGV[1];
my @ALT_OPTIONS = ();

if ($CSMITH_OPTION =~ /--no-(.*)/) {
    push @ALT_OPTIONS, $CSMITH_OPTION;
    push @ALT_OPTIONS, "--$1";
}
# todo: for options like "max-block-depth 8", try to make
# a list of alternative options including "max-block-depth 2",
# "max-block-depth 4" etc if there is enough CPUs
	
my $cpu_per_option = $CPUS / @ALT_OPTIONS;
print "using $cpu_per_option CPU each for options: ";
print @ALT_OPTIONS;
print "\n";
    
for (my $i=0; $i<@ALT_OPTIONS; $i++) {
    $ALT_OPTIONS[$i] =~ /--(.*)/;
    my $opt = $1;
    $opt =~ s/ //g;
    for (my $j=0; $j<$cpu_per_option; $j++) {
	my $cpuid = $i * $cpu_per_option + $j;
	my $dir = "work$cpuid";
	system "rm -rf $dir";
	system "mkdir $dir";
	chdir $dir;
	my $cmd = "nohup ../test.pl 0  ../$CONFIG_FILE $ALT_OPTIONS[$i] > ${opt}_output.txt 2>&1 &";
	print "running $cmd on CPU $cpuid\n";
	system $cmd;
	chdir "..";
    }
}