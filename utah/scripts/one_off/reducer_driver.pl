#!/usr/bin/perl -w
use strict;
use Net::Domain qw(hostname hostfqdn hostdomain);
use Cwd; 

my $CSMITH_PATH = $ENV{"CSMITH_PATH"};
die "please set CSMITH_PATH env first!"
    if (!defined($CSMITH_PATH));

my $VOLATILE = $ENV{"VOLATILE_PATH"};
die "please set VOLATILE_PATH env first!"
    if (!defined($VOLATILE));

$ENV{"PATH"} = "$ENV{VOLATILE_PATH}:$ENV{PATH}";

my $host = hostname();
my $TEST_ROOT = "$CSMITH_PATH/reducer";
if (@ARGV != 1 && @ARGV != 2 && @ARGV != 3) {
    die "usage: reducer_driver.pl <output-file> [test-root-dir] [eager-blind-simple-berkeley]\n\n";
}
my $RESULT_FILE = $ARGV[0];
if (@ARGV >= 2) {
	$TEST_ROOT = $ARGV[1];
	if ($TEST_ROOT eq ".") {
		$TEST_ROOT = cwd();
	}
}

my $do_eager = 1;
my $do_blind = 1;
my $do_simple = 1;
my $do_berkeley = 1;
if (@ARGV == 3) {
	$ARGV[2] =~ /(\d)-(\d)-(\d)-(\d)/;
	$do_eager = $1;
	$do_blind = $2;
	$do_simple = $3;
	$do_berkeley = $4;
}

sub get_reduce_cmd() {
	open INF, "./runme" or die;    
    my $cmd = <INF>;
    chomp $cmd;
    close INF;
    return $cmd;
}

sub prepare_berkeley($) {
	my ($seed) = @_;
    my $cmd = get_reduce_cmd();
    my @a = split("[\'\"]", $cmd);
    if (@a < 4) {
		print "wrong cmd: $cmd\n";
		return 0;
	}
	my $OPT = "";
	if (@a == 6) {
		$OPT = $a[5];
		print "OPT = $OPT\n";
	}
    my @b = split(" ", $a[1]); 
    my @c = split(" ", $a[3]);
    if (@b < 2 || @c < 2) {
		print "wrong cmd: $cmd\n";
		return 0;
	}
    my $compiler1 = $b[0];
    my $compiler2 = $c[0];
    # print "compiler is $compiler\n"; 
    my $sed_cmd = "sed s#CC1=llvm-gcc#CC1=$compiler1# < $VOLATILE/bug_tests/test-wrong-code.sh > t1.sh";
    # print "sed cmd = $sed_cmd\n";
    system $sed_cmd;
    $sed_cmd = "sed s#CC2=llvm-gcc#CC2=$compiler2# < t1.sh > t2.sh";
    system $sed_cmd;
    $sed_cmd = "sed s#OPT1=-O0#OPT1=$b[1]# < t2.sh > t3.sh";
    system $sed_cmd;
    $sed_cmd = "sed s#OPT2=-Os#OPT2=$c[1]# < t3.sh > test1.sh";
    system $sed_cmd;
    system "chmod 755 test1.sh";
    
    system "rm -f t1.sh t2.sh t3.sh";
    if (!(-e "pre_reduction.c") || !(-s "pre_reduction.c")) {
		my $csmith_cmd = "$CSMITH_PATH/src/csmith --concise -s $seed $OPT > pre_reduction.c";
		system $csmith_cmd;
	}
    return 1;
}

# find the reduction rate from the log file
sub read_reduction_rate($) {
	my ($input) = @_;
    if (open INF, "<$input") { 
		while (my $line = <INF>) { 
			if ($line =~ /reduction rate: ([\d\.]+) ===/) { 
				close INF;
				my $rate =  sprintf("%.2f", $1);
				# print "rate = $1\n";
				return $rate;
			}
		} 
		close INF;
	}
    return -1;
}

sub find_or_calc_rate($$) {
	my ($method, $seed) = @_;
	my $rate = read_reduction_rate("$method.log");
	if ($rate == -1 && (-e "pre_reduction.c") && (-e "$method$seed.c")) {
		my $fsize1 = -s "pre_reduction.c";
		my $fsize2 = -s "$method$seed.c";
		# print "using $method$seed.c size: $fsize2\n";
		my $rate0 = (1 - ($fsize2 / $fsize1)) * 100; 
		$rate = sprintf("%.2f", $rate0);
	}
	return $rate;
}
				
sub reduce_testcase($$$) {
	my ($dir, $rates, $times) = @_;
	print "work in $dir\n";
	chdir($dir) or die "$!";
	my $cmd0 = get_reduce_cmd();
	my @a = split(" ", $cmd0);
	my $seed = $a[1];
	
	if ($do_eager) {
		system ("rm -f eager.log");
		my $stime = time();
		system ("./runme > eager.log 2>&1");
		my $time_usage = time() - $stime;	
		my $rate = read_reduction_rate("eager.log"); 
		push @$rates, $rate;
		push @$times, $time_usage;
		system ("rm -f *out* *small* after* works.c");
	} else {
		push @$rates, find_or_calc_rate("eager", $seed);
		push @$times, 0;
	}  
	if ($do_blind) {
		my $cmd = $cmd0;     
		$cmd =~ s/eager/blind_eager/g;
		system ("rm -f blind.log");
		my $stime = time();
		system ("$cmd > blind.log 2>&1");
		my $time_usage = time() - $stime;
		
		my $rate = read_reduction_rate("blind.log");
		push @$rates, $rate;
		push @$times, $time_usage;
		system ("rm -f *out* *small* after* works.c");
	} else {
		push @$rates, find_or_calc_rate("blind", $seed);
		push @$times, 0;
	}
	my $bok = 0;
	if ($do_berkeley) { 
		if (prepare_berkeley($seed)) {  
			system "cp pre_reduction.c small.c";
			my $stime = time();
			system "godelta > berkeley.log 2>&1";
			my $time_usage = time() - $stime;
			
			# make sure the reduction is good 
			my $res = system "./test1.sh >/dev/null";
			$res = $res >> 8;
			if (!$res) { 
				my $fsize1 = -s "pre_reduction.c";
				my $fsize2 = -s "small.c";
				my $rate0 = (1 - ($fsize2 / $fsize1)) * 100;
				$bok = 1;
				my $rate = sprintf("%.2f", $rate0);
				system "cp small.c berkeley$seed.c";
				push @$rates, $rate;
				push @$times, $time_usage;
			} 
			system "rm -f *small* *out* gcc* log multi*";
		}
	}
	if (!$bok) {
		push @$rates, find_or_calc_rate("berkeley", $seed);
		push @$times, 0;
	}
	if ($do_simple) {
		my $cmd = $cmd0;      
		$cmd =~ s/eager/simple/g;
		system ("rm -f simple.log");
		my $stime = time();
		system ("$cmd > simple.log 2>&1");
		my $time_usage = time() - $stime;
		
		my $rate = read_reduction_rate("simple.log");
		push @$rates, $rate;
		push @$times, $time_usage;
		system ("rm -f *out* *small* works.c");
	} else {
		push @$rates, find_or_calc_rate("simple", $seed);
		push @$times, 0;
	}
	
	my $revision = "unknown";
	if ($cmd0 =~ /r(\d+)-/) {
		$revision = $1;
	}
	# find the name of the compiler
	if ($cmd0 =~ /clang /) { 
		return ("clang", $revision, $seed);
	}
	elsif ($cmd0 =~ /gcc /) {
		return ("gcc", $revision, $seed);
	}
	else {
		return ("unknown", $revision, $seed);
	}
}

#####################################################################
#  loop through the test directories under $CSMITH_PATH/reducer, and
#  run various kinds of reduction on the test cases, the results are
#  written into $RESULT_FILE
#####################################################################
sub create_log() {	
	my $rate_results_in_latex = "";
	my $time_results_in_latex = "";
	  
	my $total_rate0 = 1;
	my $total_rate1 = 1;
	my $total_rate2 = 1;
	my $total_rate3 = 1;
	my $test_cnt0 = 0;
	my $test_cnt1 = 0;
	my $test_cnt2 = 0;
	my $test_cnt3 = 0;

	system "rm -f $TEST_ROOT/$RESULT_FILE";
	for my $dir (glob("$TEST_ROOT/test*")) {
		my @rates = ();
		my @times = ();
		# only check test cases for 64-bit for now
		if ($dir =~ /x86/) {
			next;
		}
		my ($compiler, $revision, $seed) = reduce_testcase($dir, \@rates, \@times);
		$dir =~ /test(\d+)/;
		my $id = $1;
		my $platform = "x86-64";
		
		$rate_results_in_latex = "$id & $seed & $platform & $compiler &  & $rates[0] & $rates[1] & $rates[3] & $rates[2]";
		$time_results_in_latex = "$id & $seed & $platform & $compiler &  & $times[0] & $times[1] & $times[3] & $times[2]";
		print "rates: $rate_results_in_latex\n";
		print "times: $time_results_in_latex\n";
		
		# write to file
		open OUT, ">>../$RESULT_FILE" or die; 
		print OUT "$rate_results_in_latex\n"; 
		print OUT "$time_results_in_latex\n";
		close OUT;
		
		# statistical: use geometric mean
		if ($rates[0] != -1) {
			$total_rate0 *= $rates[0]/100.0;
			$test_cnt0++;
		}
		if ($rates[1] != -1) {
			$total_rate1 *= $rates[1]/100.0;
			$test_cnt1++;
		}
		if ($rates[2] != -1) {
			$total_rate2 *= $rates[2]/100.0;
			$test_cnt2++;
		}
		if ($rates[3] != -1) {
			$total_rate3 *= $rates[3]/100.0;
			$test_cnt3++;
		}
	}

	# calculate averages
	my $avg_rate0 = sprintf("%.2f", ($total_rate0 ** (1.0 / $test_cnt0)) *100.0);
	my $avg_rate1 = sprintf("%.2f", ($total_rate1 ** (1.0 / $test_cnt1)) *100.0);
	my $avg_rate2 = sprintf("%.2f", ($total_rate2 ** (1.0 / $test_cnt2)) *100.0);
	my $avg_rate3 = sprintf("%.2f", ($total_rate3 ** (1.0 / $test_cnt3)) *100.0);
	$rate_results_in_latex = "average & N/A & N/A & N/A &  & $avg_rate0 & $avg_rate1 & $avg_rate3 & $avg_rate2";
	print "average rates: $avg_rate0 $avg_rate1 $avg_rate3 $avg_rate2\n"; 
	chdir("..") or die "$!";
	open OUT, ">>$RESULT_FILE" or die; 
	print OUT "$rate_results_in_latex\n"; 
	close OUT;
}

################### main ##################################
create_log();
# convert log file into tex file
if (-e "$RESULT_FILE") {
	open OUT, ">table.tex" or die;
	open INF, "<$RESULT_FILE" or die;
	print OUT '\documentclass[english]{article}', "\n"; 
	print OUT '\begin{document}', "\n";
	print OUT '\begin{table}\small', "\n";
	print OUT '\caption{Performance of reduction techniques}', "\n";
	print OUT '\begin{tabular}{| c | c | c | c | c | c | c | c | c |}', "\n";
	print OUT '\hline', "\n"; 
	print OUT 'id & seed & platform & compiler & manual & eager & blind & simple & berkeley \\\\', "\n";
	while (my $line = <INF>) {
		print OUT $line, ' \\\\', "\n";
		$line = <INF>;
	}
	print OUT '\end{tabular}', "\n";
	print OUT '\end{table}', "\n";
	print OUT '\end{document}', "\n";
	close OUT;
	close INF;
}

