#!/usr/bin/perl -w

######################################################################
#
# This Delta debugger specifically targets C/C++ code. Its design
# point is to be slow, thorough, and stupid. We count on the delta
# test script being able to rapidly discard syntactically invalid
# delta variants (for example, by compiling them).
#
####################################################################

# TODO:

# implement line-based delta
# add an option limiting the number of passes
# add a quiet mode -- only report progress
# simplify the termination condition -- stop after 2 passes with no size decrease
# see if it's faster to work from back to front
# watch for unexpected abnormal compiler outputs

# get speedup by adding fast bailouts from test scripts
#   super-fast: just runs one compiler at -O0 look for syntactical correctness
#   medium fast: looks for checksum differences, runs valgrind, etc.
#   full: runs chucky's tool
#   can also batch up transformations (like delete parens) that
#     usually succeed without running any test

# parameters
#   Cp = cost of a passed test
#   Cf = cost of a failed test, say half the cost of a passed test
#   S  = fraction of transformations that succeed
#   N  = number of attempted transformations
#   assumption: test are independent
#   assumption: c_delta has negligible running time

# if test is run every time
#  Cp*S*N + Cf*(1-S)*N

# if K transformations are run before running any test
# SS = S^K, NN = N/K
# Cp*SS*NN + Cf(1-SS)*NN

# test this and then work out the math when cheaper tests exist
# need to measure some runs and estimate the values of constants

######################################################################

use strict;
use Regexp::Common;
use re 'eval';
use File::Which;

######################################################################

# if set, save all delta variants
my $DEBUG = 0;

# if set, ensure the delta test succeeds before starting each pass
my $SANITY = 1;

######################################################################

my $prog;
my $orig_prog_len;

sub print_pct () {
    my $pct = 100 - (length($prog)*100.0/$orig_prog_len);
    printf "(%.1f %%)\n", $pct;
}

sub find_match ($$$) {
    (my $p2, my $s1, my $s2) = @_;
    my $count = 1;
    die if (!(defined($p2) && defined($s1) && defined($s2)));
    while ($count > 0) {
	return -1 if ($p2 >= (length ($prog)-1));
	my $s = substr($prog, $p2, 1);
	if (!defined($s)) {
	    my $l = length ($prog);
	    print "$p2 $l\n";
	    die;
	}
	$count++ if ($s eq $s1);
	$count-- if ($s eq $s2);
	$p2++;
    }
    return $p2-1;
}

# these are set at startup time and never change
my $cfile;
my $test;
my $trial_num = 0;   

sub read_file_helper () {
    open INF, "<$cfile" or die;
    my $xxprog = "";
    while (my $line = <INF>) {
	$xxprog .= $line;
    }
    if (substr($xxprog, 0, 1) ne " ") {
	$xxprog = " $xxprog";
    }
    if (substr ($xxprog, -1, 1) ne " ") {
	$xxprog = "$xxprog ";
    }
    close INF;
    return $xxprog;
}

sub read_file () {
    $prog = read_file_helper();
}

sub ensure_mem_and_disk_are_synced () {
    die unless ($prog eq read_file_helper());
}

sub save_copy ($) {
    (my $fn) = @_;
    open OUTF, ">$fn" or die;
    print OUTF $prog;
    close OUTF;
}

sub write_file () {
    if (defined($DEBUG) && $DEBUG) {
	save_copy ("delta_tmp_${trial_num}.c");
    }
    $trial_num++;
    open OUTF, ">$cfile" or die;
    print OUTF $prog;
    close OUTF;
}

sub runit ($) {
    (my $cmd) = @_;
    if ((system "$cmd") != 0) {
	return -1;
    }   
    return ($? >> 8);
}

sub run_test () {
    my $res = runit "./$test >/dev/null 2>&1";
    return ($res == 0);
}

my %cache = ();
my $cache_hits = 0;
my $good_cnt;
my $bad_cnt;
my $pass_num = 0;
my $delta_pos;
my %method_worked = ();
my %method_failed = ();
my $old_size = 1000000000;
 
sub sanity_check () {
    print "sanity check... ";
    my $res = run_test ();
    if (!$res) {
	die "test (and sanity check) fails";
    }
    print "successful\n";
}

sub delta_step_fail($) {
    (my $method) = @_;
    print "failure\n";
    system "cp $cfile.bak $cfile";
    read_file();
    $bad_cnt++;
    $method_failed{$method}++;
}
    
my $changed_on_disk = 0;
my $delta_method;

sub delta_test ($) {
    (my $ok_to_enlarge) = @_;

    if ($changed_on_disk) {
	read_file();
	$changed_on_disk = 0;
    } else {
	write_file();
    }

    ensure_mem_and_disk_are_synced();

    my $len = length ($prog);
    print "[$pass_num $delta_method ($delta_pos / $len) s:$good_cnt f:$bad_cnt] ";

    my $result = $cache{$prog};

    if (defined($result)) {
	$cache_hits++;
	print "(hit) ";
	delta_step_fail($delta_method);
	return 0;
    }
    
    $result = run_test ();
    $cache{$prog} = $result;
    
    if ($result) {
	print "success ";
	print_pct();
	system "cp $cfile $cfile.bak";
	$good_cnt++;
	$method_worked{$delta_method}++;
	my $size = length ($prog);
	die if (($size > $old_size) && !$ok_to_enlarge);
	if ($size < $old_size) {
	    %cache = ();
	}
	$old_size = $size;
	return 1;
    } else {
	delta_step_fail($delta_method);
	return 0;
    }
}

############################## begin delta passes #############################

my $exit_delta_pass;
my $delta_worked;

my $varnum = "(\\-?|\\+?)[0-9a-zA-Z\_]+";
my $varnumexp = "($varnum)|($RE{balanced}{-parens=>'()'})";
my $field = "\\.($varnum)";
my $index = "\\\[($varnum)\\\]";
my $fullvar = "([\\&\\*]*)($varnumexp)(($field)|($index))*";
my $arith = "\\+|\\-|\\%|\\/|\\*";
my $comp = "\\<\\=|\\>\\=|\\<|\\>|\\=\\=|\\!\\=|\\=";
my $logic = "\\&\\&|\\|\\|";
my $bit = "\\||\\&|\\^|\\<\\<|\\>\\>";
my $binop = "($arith)|($comp)|($logic)|($bit)|(\\-\\>)";
my $border = "[\\*\\{\\(\\[\\:\\,\\}\\)\\]\\;\\,]";
my $borderorspc = "(($border)|(\\s))";
my $fname = "(?<fname>$varnum)";
my $call = "$varnum\\s*$RE{balanced}{-parens=>'()'}";

# these match without additional qualification
my @regexes_to_replace = (
    ["$RE{balanced}{-parens=>'<>'}", ""],
    ["$RE{balanced}{-parens=>'()'}", ""],
    ["$RE{balanced}{-parens=>'{}'}", ""],
    ["namespace(.*?)$RE{balanced}{-parens=>'{}'}", ""],
    ["=\\s*$RE{balanced}{-parens=>'{}'}", ""],
    ["\\:\\s*[0-9]+\\s*;", ";"],
    ["\\;", ""],
    ["\#(.*)\n", ""],
    ["\\^\\=", "="],
    ["\\|\\=", "="],
    ["\\&\\=", "="],
    ["\"(.*)\"", ""],
    ["\'(.*)\'", ""],
    ["\\+\\=", "="],
    ["\\-\\=", "="],
    ["\\*\\=", "="],
    ["\\/\\=", "="],
    ["\\%\\=", "="],
    ["\\<\\<\\=", "="],
    ["\\>\\>\\=", "="],
    ["\\+", ""],
    ["\\-", ""],
    [":", ""],
    ["::", ""],
    ["\\!", ""],
    ["\\~", ""],
    ["while", "if"],
    ["struct(.*?);", ""],
    ["union(.*?);", ""],
    ["enum(.*?);", ""],
    ["struct(.*);", ""],
    ["union(.*);", ""],
    ["enum(.*);", ""],
    ['"(.*?)"', ""],
    ['"(.*?)",', ""],
    ['"(.*)"', ""],
    ['"(.*)",', ""],
    );

# these match when preceded and followed by $borderorspc
my @delimited_regexes_to_replace = (
    ["($varnumexp)\\s*:", ""],
    ["goto\\s+($varnum);", ""],
    ["char", "int"],
    ["short", "int"],
    ["long", "int"],
    ["signed", "int"],
    ["unsigned", "int"],
    ["int(\\s+)argc(\\s*),(\\s*)(.*)(\\s*)\\*argv\\[\\]", "void"],
    ["int(\\s+)argc(\\s*),(\\s*)(.*)(\\s*)\\*(\\s*)\\*argv", "void"],
    ["int.*?;", ""],
    ["for", ""],
    ["\"(.*)\"", ""],
    ["\'(.*)\'", ""],
    ["\"(.*?)\"", ""],
    ["\'(.*?)\'", ""],
    ["if\\s+\\(.*?\\)", ""],
    ["$call,", "0"],
    ["$call,", ""],
    ["$call", "0"],
    ["$call", ""],
    );

my @subexprs = (
    "($fullvar)(\\s*)($binop)(\\s*)($fullvar)",
    "($fullvar)(\\s*)($binop)",
    "($binop)(\\s*)($fullvar)",
    "($fullvar)",
    ":(\\s*)($fullvar)",
    "::(\\s*)($fullvar)",
    "($fullvar)(\\s*):",
    "($fullvar)(\\s*)::",
    "($fullvar)(\\s*\\?\\s*)($fullvar)(\\s*\\:\\s*)($fullvar)",
    );

foreach my $x (@subexprs) {
    push @delimited_regexes_to_replace, ["$x", "0"];
    push @delimited_regexes_to_replace, ["$x", "1"];
    push @delimited_regexes_to_replace, ["$x", ""];
    push @delimited_regexes_to_replace, ["$x\\s*,", "0,"];
    push @delimited_regexes_to_replace, ["$x\\s*,", "1,"];
    push @delimited_regexes_to_replace, ["$x\\s*,", ""];
    push @delimited_regexes_to_replace, [",\\s*$x", ""];
}

my %regex_worked;
my %regex_failed;
my %delimited_regex_worked;
my %delimited_regex_failed;
for (my $n=0; $n<scalar(@regexes_to_replace); $n++) {
    $regex_worked{$n} = 0;
    $regex_failed{$n} = 0;
}
for (my $n=0; $n<scalar(@delimited_regexes_to_replace); $n++) {
    $delimited_regex_worked{$n} = 0;
    $delimited_regex_failed{$n} = 0;
}

sub replace_regex (){ 
    my $n=-1;
    foreach my $l (@regexes_to_replace) {	       
	$n++;
	my $str = @{$l}[0];
	my $repl = @{$l}[1];
	my $first = substr($prog, 0, $delta_pos);
	my $rest = substr($prog, $delta_pos);
	my $rrest = $rest;
	if ($rest =~ s/(^$str)/$repl/s) {
	    my $before = $1;
	    my $zz1 = $rest;
	    my $zz2 = $rrest;
	    ($zz1 =~ s/\s//g);
	    ($zz2 =~ s/\s//g);
	    if ($zz1 ne $zz2) {
		print "regex $n replacing '$before' with '$repl' : ";
		$prog = $first.$rest;
		if (delta_test (0)) {
		    $delta_worked = 1;
		    $regex_worked{$n}++;
		} else {
		    $regex_failed{$n}++;
		}
	    }
	}
    }
    $n=-1;
    foreach my $l (@delimited_regexes_to_replace) {
	$n++;
	my $str = @{$l}[0];
	my $repl = @{$l}[1];
	my $first = substr($prog, 0, $delta_pos);
	my $rest = substr($prog, $delta_pos);
	
	# avoid infinite replacement loops!
	next if ($repl eq "0" && $rest =~ /^($borderorspc)0$borderorspc/);
	next if ($repl =~ /0\s*,/ && $rest =~ /^($borderorspc)0\s*,$borderorspc/);
	next if ($repl eq "1" && $rest =~ /^($borderorspc)0$borderorspc/);
	next if ($repl =~ /1\s*,/ && $rest =~ /^($borderorspc)0\s*,$borderorspc/);
	next if ($repl eq "1" && $rest =~ /^($borderorspc)1$borderorspc/);
	next if ($repl =~ /1\s*,/ && $rest =~ /^($borderorspc)1,$borderorspc/);
	
	my $rrest = $rest;
	if ($rest =~ s/^(?<delim1>$borderorspc)(?<str>$str)(?<delim2>$borderorspc)/$+{delim1}$repl$+{delim2}/s) {
	    my $before = $+{str};
	    my $zz1 = $rest;
	    my $zz2 = $rrest;
	    ($zz1 =~ s/\s//g);
	    ($zz2 =~ s/\s//g);
	    if ($zz1 ne $zz2) {
		print "regex $n delimited replacing '$before' with '$repl' : ";
		$prog = $first.$rest;
		if (delta_test (0)) {
		    $delta_worked = 1;
		    $delimited_regex_worked{$n}++;
		} else {
		    $delimited_regex_failed{$n}++;
		}
	    }
	}
    }
}

sub blanks () {
    my $first = substr($prog, 0, $delta_pos);
    my $rest = substr($prog, $delta_pos);
    if ($rest =~ s/^(\s{2,})/ /) {
	$prog = $first.$rest;
	$delta_worked |= delta_test (0);
    }
}

sub clang_delta ($) {
    (my $how) = @_;
    my $x = $delta_pos+1;
    my $res = runit ("clang_delta --transformation=$how --counter=$x $cfile > foo");
    if ($res==0) {
	system "mv foo $cfile";
	$changed_on_disk = 1;
	$delta_worked |= delta_test (1);
    } else {
	$exit_delta_pass = 1;
    }
}

my $INDENT_OPTS = "-bad -bap -bc -cs -pcs -prs -saf -sai -saw -sob -ss -bl ";

sub indent () {
    system "indent $INDENT_OPTS $cfile";
    $changed_on_disk = 1;
    $delta_worked |= delta_test (1);
    $exit_delta_pass = 1;
}

sub crc () {
    my $first = substr($prog, 0, $delta_pos);
    my $rest = substr($prog, $delta_pos);
    if ($rest =~ /^(?<all>transparent_crc\s*\((?<list>.*?)\))/) {
	my @stuff = split /,/, $+{list};
	my $var = $stuff[0];
	my $repl = "printf (\"%d\\n\", (int)$var)";
	print "crc call: < $+{all} > => < $repl > ";
	substr ($rest, 0, length ($+{all})) = $repl;
	$prog = $first.$rest;
	$delta_worked |= delta_test (0);
    }
}

sub ternary () {
    my $first = substr($prog, 0, $delta_pos);
    my $rest = substr($prog, $delta_pos);
    if ($rest =~ s/^(?<del1>$borderorspc)(?<a>$varnumexp)\s*\?\s*(?<b>$varnumexp)\s*:\s*(?<c>$varnumexp)(?<del2>$borderorspc)/$+{del1}$+{b}$+{del2}/s) {
	$prog = $first.$rest;
	my $n1 = "$+{del1}$+{a} ? $+{b} : $+{c}$+{del2}";
	my $n2 = "$+{del1}$+{b}$+{del2}";
	print "replacing $n1 with $n2\n";
	$delta_worked |= delta_test (0);
    }	    
    $first = substr($prog, 0, $delta_pos);
    $rest = substr($prog, $delta_pos);
    if ($rest =~ s/^(?<del1>$borderorspc)(?<a>$varnumexp)\s*\?\s*(?<b>$varnumexp)\s*:\s*(?<c>$varnumexp)(?<del2>$borderorspc)/$+{del1}$+{c}$+{del2}/s) {
	$prog = $first.$rest;
	my $n1 = "$+{del1}$+{a} ? $+{b} : $+{c}$+{del2}";
	my $n2 = "$+{del1}$+{c}$+{del2}";
	print "replacing $n1 with $n2\n";
	$delta_worked |= delta_test (0);
    }
}

sub shorten_ints () {
    my $first = substr($prog, 0, $delta_pos);
    my $rest = substr($prog, $delta_pos);
    if ($rest =~ s/^(?<pref>$borderorspc(\\-|\\+)?(0|(0[xX]))?)(?<del>[0-9a-fA-F])(?<numpart>[0-9a-fA-F]+)(?<suf>[ULul]*$borderorspc)/$+{pref}$+{numpart}$+{suf}/s) {
	$prog = $first.$rest;
	my $n1 = "$+{pref}$+{del}$+{numpart}$+{suf}";
	my $n2 = "$+{pref}$+{numpart}$+{suf}";
	print "replacing $n1 with $n2\n";
	$delta_worked |= delta_test (0);
    }      
    $first = substr($prog, 0, $delta_pos);
    $rest = substr($prog, $delta_pos);
    my $orig_rest = $rest;
    if ($rest =~ s/^(?<pref1>$borderorspc)(?<pref2>(\\-|\\+)?(0|(0[xX]))?)(?<numpart>[0-9a-fA-F]+)(?<suf>[ULul]*$borderorspc)/$+{pref1}$+{numpart}$+{suf}/s && ($rest ne $orig_rest)) {
	$prog = $first.$rest;
	my $n1 = "$+{pref1}$+{pref2}$+{numpart}$+{suf}";
	my $n2 = "$+{pref1}$+{numpart}$+{suf}";
	print "replacing $n1 with $n2\n";
	$delta_worked |= delta_test (0);
    }     
    $first = substr($prog, 0, $delta_pos);
    $rest = substr($prog, $delta_pos);
    $orig_rest = $rest;
    if ($rest =~ s/^(?<pref>$borderorspc(\\-|\\+)?(0|(0[xX]))?)(?<numpart>[0-9a-fA-F]+)(?<suf1>[ULul]*)(?<suf2>$borderorspc)/$+{pref}$+{numpart}$+{suf2}/s && ($rest ne $orig_rest)) {
	$prog = $first.$rest;
	my $n1 = "$+{pref}$+{numpart}$+{suf1}$+{suf2}";
		my $n2 = "$+{pref}$+{numpart}$+{suf2}";
	print "replacing $n1 with $n2\n";
	$delta_worked |= delta_test (0);
    } 
}

sub angles () {
    if (substr($prog, $delta_pos, 1) eq "<") {
	my $p2 = find_match ($delta_pos+1,"<",">");
	if ($p2 != -1) {
	    die if (substr($prog, $delta_pos, 1) ne "<");
	    die if (substr($prog, $p2, 1) ne ">");
	    
	    my $del = substr ($prog, $delta_pos, $p2-$delta_pos+1, "");
	    print "deleting '$del' at $delta_pos--$p2 : ";
	    my $res = delta_test (0);
	    $delta_worked |= $res;
	    
	    if (!$res) {
		substr ($prog, $p2, 1) = "";
		substr ($prog, $delta_pos, 1) = "";
		print "deleting at $delta_pos--$p2 : ";
		$delta_worked |= delta_test (0);
	    }
	}
    }
}

sub parens () {
    if (substr($prog, $delta_pos, 1) eq "(") {
	my $p2 = find_match ($delta_pos+1,"(",")");
	if ($p2 != -1) {
	    die if (substr($prog, $delta_pos, 1) ne "(");
	    die if (substr($prog, $p2, 1) ne ")");
	    
	    my $del = substr ($prog, $delta_pos, $p2-$delta_pos+1, "");
	    print "deleting '$del' at $delta_pos--$p2 : ";
	    my $res = delta_test (0);
	    $delta_worked |= $res;
	    
	    if (!$res) {
		substr ($prog, $p2, 1) = "";
		substr ($prog, $delta_pos, 1) = "";
		print "deleting at $delta_pos--$p2 : ";
		$delta_worked |= delta_test (0);
	    }
	}
    }
}

sub brackets () {
    if (substr($prog, $delta_pos, 1) eq "{") {
	my $p2 = find_match ($delta_pos+1,"{","}");
	if ($p2 != -1) {
	    die if (substr($prog, $delta_pos, 1) ne "{");
	    die if (substr($prog, $p2, 1) ne "}");
	    
	    my $del = substr ($prog, $delta_pos, $p2-$delta_pos+1, "");
	    print "deleting '$del' at $delta_pos--$p2 : ";
	    my $res = delta_test (0);
	    $delta_worked |= $res;
	    
	    if (!$res) {
		substr ($prog, $p2, 1) = "";
		substr ($prog, $delta_pos, 1) = "";
		print "deleting at $delta_pos--$p2 : ";
		$delta_worked |= delta_test (0);
	    }
	}
    }
}

############################## end delta passes #############################

sub delta_pass ($) {
    no strict "refs";
    ($delta_method) = @_;    
    $delta_pos = 0;
    $good_cnt = 0;
    $bad_cnt = 0;

    print "\n";
    print "========== starting pass <$delta_method> ==========\n";

    if ($SANITY) {
	sanity_check();
    }

    while (1) {
	ensure_mem_and_disk_are_synced();
	return ($good_cnt > 0) if ($delta_pos >= length ($prog));
	$delta_worked = 0;
	$exit_delta_pass = 0;

	if ($delta_method =~ /^clang-(.*)$/) {
	    $delta_method = $1;
	    clang_delta ($delta_method);
	} else {
	    &$delta_method();
	} 

	return 0 if ($exit_delta_pass);

	if (!$delta_worked) {
	    $delta_pos++;
	}
    }
}

############################### main #################################

# global invariant: the delta test always succeeds for $cfile.bak

my %all_methods = (

    "blanks" => 1,
    "crc" => 1,
    "angles" => 2,
    "brackets" => 2,
    "ternary" => 2,
    "parens" => 3,
    "replace_regex" => 4,
    "shorten_ints" => 5,
    "indent" => 15,

    );

my $clang_delta = File::Which::which ("clang_delta");
if (1 && defined($clang_delta)) {
    $all_methods{"clang-aggregate-to-scalar"} = 10;
    $all_methods{"clang-binop-simplification"} = 10;
    $all_methods{"clang-combine-global-var"} = 10;
    $all_methods{"clang-combine-local-var"} = 10;
    $all_methods{"clang-local-to-global"} = 10;
    $all_methods{"clang-param-to-global"} = 10;
    $all_methods{"clang-param-to-local"} = 10;
    $all_methods{"clang-remove-nested-function"} = 10;
    $all_methods{"clang-rename-fun"} = 10;
    $all_methods{"clang-rename-param"} = 10;    
    $all_methods{"clang-rename-var"} = 10;
    $all_methods{"clang-replace-callexpr"} = 10;    
    $all_methods{"clang-return-void"} = 10;
}

sub usage() {
    print "usage: c_delta.pl test_script.sh file.c [method [method ...]]\n";
    print "available methods are:\n";
    foreach my $method (keys %all_methods) {
	print "  --$method\n";
    }
    print "Default strategy is trying all methods.\n";
    die;
}

$test = shift @ARGV;
usage if (!defined($test));
if (!(-x $test)) {
    print "test script '$test' not found, or not executable\n";
    usage();
}

$cfile = shift @ARGV;
usage if (!defined($cfile));
if (!(-e $cfile)) {
    print "'$cfile' not found\n";
    usage();
}

my %methods = ();
foreach my $arg (@ARGV) {
    my $found = 0;
    foreach my $method (keys %all_methods) {
	if ($arg eq "--$method") {
	    $methods{$method} = 1;
	    $found = 1;
	    last;
	}
    }
    if (!$found) {
	print "unknown method '$arg'\n";
	usage();
    }
}

if (scalar (keys %methods) == 0) {
    # default to all methods
    foreach my $method (keys %all_methods) {
	$methods{$method} = 1;
    }
}

system "cp $cfile $cfile.orig";
system "cp $cfile $cfile.bak";

sub bymethod {
    return $all_methods{$a} <=> $all_methods{$b};
}

# iterate to global fixpoint

read_file ();    
$orig_prog_len = length ($prog);

my $file_size = -s $cfile;
my $spinning = 0;

while (1) {
    save_copy ("delta_backup_${pass_num}.c");
    my $success = 0;
    foreach my $method (sort bymethod keys %methods) {
	$success |= delta_pass ($method);
    }
    $pass_num++;
    last if (!$success);
    my $s = -s $cfile;
    if ($s >= $file_size) {
	$spinning++;
    }
    last if ($spinning > 3);
    $file_size = $s;
}

print "===================== done ====================\n";

print "\n";
print "overall reduction: ";
print_pct();

print "\n";
print "pass statistics:\n";
foreach my $method (sort keys %methods) {
    my $w = $method_worked{$method};
    $w=0 unless defined($w);
    my $f = $method_failed{$method};
    $f=0 unless defined($f);
    print "  method $method worked $w times and failed $f times\n";
}

print "\n";
print "regex statistics:\n";
for (my $n=0; $n<scalar(@regexes_to_replace); $n++) {
    my $a = $regex_worked{$n};
    my $b = $regex_failed{$n};
    next if (($a+$b)==0);
    print "  $n s:$a f:$b\n";
}

print "\n";
print "delimited regex statistics:\n";
for (my $n=0; $n<scalar(@delimited_regexes_to_replace); $n++) {
    my $a = $delimited_regex_worked{$n};
    my $b = $delimited_regex_failed{$n};
    next if (($a+$b)==0);
    print "  $n s:$a f:$b\n";
}

print "\n";
print "there were $cache_hits cache hits\n";

######################################################################
