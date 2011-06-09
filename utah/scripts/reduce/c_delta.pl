#!/usr/bin/perl -w

use strict;

# flip the flow of control so delta_step calls a test routine

# when doing search and replace, how to specify a larger matching context
# for what is actually replaced

# if there's a way to match starting at a specified position, use it

# do everything with search and replace instead of substr

# do everything with regexes-- need to specify matching parens, brackets, etc.

# build up the regexes programmatically to support multiple replacement options

# enumerate regrexes so that they're all tried for each position, even
# when an earlier one matches and then fails

# make sure file starts with a blank 

# add passes to 
#   remove digits from numbers to make them smaller
#   run indent speculatively
#   turn checksum calls into regular printfs
#   delete a complete function

# to regexes, add a way to specify border characters that won't be removed

# avoid testing the same thing twice

# avoid mangling strings

# harder
#   transform a function to return void
#   inline a function call
#   sort functions in order to eliminate prototypes
#   un-nest nested calls in expressions
#   move arguments and locals to global scope
#   remove level of pointer indirection
#   remove array dimension
#   remove argument from function, including all calls

# long term todo: rewrite this tool at the AST level

my $INIT = "1";

my %function_prefixes = (
    "safe_" => $INIT,
    "func_" => $INIT,
    "sizeof" => $INIT,
    "if" => "",
    "for" => "",
    );

my $num = "\\-?[xX0-9a-fA-F]+[UL]*";
my $field = "\\.f[0-9]+";
my $index = "\\\[(($num)|i|j|k|l)\\\]";
my $barevar = "[lgpt]_[0-9]+";
my $var1 = "([\\&\\*]*)($barevar)(($field)|($index))*";
my $var2 = "i|j|k|si|ui|si1|si2|ui1|ui2|left|right|val|crc32_context|func_([0-9]+)|safe_([0-9]+)";
my $var = "($var1)|($var2)";
my $arith = "\\+|\\-|\\%|\\/|\\*";
my $comp = "\\<\\=|\\>\\=|\\<|\\>|\\=\\=|\\!\\=|\\=";
my $logic = "\\&\\&|\\|\\|";
my $bit = "\\||\\&|\\^|\\<\\<|\\>\\>";
my $binop = "($arith)|($comp)|($logic)|($bit)";
my $varnum = "($var)|($num)";
my $border = "[\\*\\{\\(\\[\\:\\,\\}\\)\\]\\;\\,]";
my $borderorspc = "(($border)|(\\s))";
my $borderspc = "($border\\s)";
my $spcborder = "(\\s$border)";

#print "$field\n";
#print "$index\n";
#print "$border\n";
#print "$var1\n";
#print "$var2\n";
#print "$borderspc\n";
#print "$spcborder\n";

my %replace_regexes = (
    "\\:\\s*[0-9]+\\s*;" => ";",
    "\\;" => "",
    "\\{\\s*\\}" => ";",
    "for\\s*\\(.*?\\)" => "",
    "\\^\\=" => "=",
    "\\|\\=" => "=",
    "($barevar)" => "",
    "\\&\\=" => "=",
    "\\+\\=" => "=",
    "\\-\\=" => "=",
    "\\*\\=" => "=",
    "\\/\\=" => "=",
    "\\%\\=" => "=",
    "\\<\\<\\=" => "=",
    "\\>\\>\\=" => "=",
    "lbl_[0-9]+:" => "",
    "($varnum)" => "",
    "char" => "int",
    "char" => "",
    "short" => "int",
    "short" => "",
    "long" => "int",
    "long" => "",
    "signed" => "int",
    "signed" => "",
    "unsigned" => "int",
    "unsigned" => "",
    "else" => "",
    "volatile" => "",
    "const" => "",
    "static" => "",
    "extern" => "",
    "\\+" => "",
    "\\-" => "",
    "\\!" => "",
    "\\~" => "",
    "=\\s*\{\\s*\}" => "",
    "continue" => "", 
    "return" => "",
    "int argc, char \\*argv\\[\\]" => "void",
    "int.*?;" => "",
    "for" => "",
    "if\\s+\\(.*?\\)" => "",
    "struct.*?;" => "",
    "if" => "",
    "break" => "", 
    "inline" => "", 
    "printf" => "",
    "print_hash_value" => "",
    "transparent_crc" => "",
    "platform_main_begin" => "",
    "platform_main_end" => "",
    "crc32_gentab" => "",
    );

my $prog;

sub find_match ($$$) {
    (my $p2, my $s1, my $s2) = @_;
    my $count = 1;
    die if (!(defined($p2)&&defined($s1)&&defined($s2)));
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

sub del_up_to_matching_parens ($$) {
    (my $pos, my $pref) = @_;
    my $p2 = $pos;
    $p2++ while (
		 substr($prog, $p2, 1) ne "(" &&
		 $p2 <= (length ($prog)-1)
		 );
    $p2 = find_match ($p2+1,"(",")");
    return -1 if ($p2 == -1);
    $p2++;
    my $xx = substr ($prog, $pos, $p2-$pos);
    my $yy = $function_prefixes{$pref};
    print "replace '$xx' with '$yy' ";
    substr ($prog, $pos, $p2-$pos) = $function_prefixes{$pref};
    return ($p2-$pos);
}

# these are set at startup time and never change
my $cfile;
my $test;

sub read_file ()
{
    open INF, "<$cfile" or die;
    $prog = "";
    while (my $line = <INF>) {
	$prog .= $line;
    }
    close INF;
}

sub write_file ($)
{
    open OUTF, ">$cfile" or die;
    print OUTF $prog;
    close OUTF;
}

sub match_subexp ($$) {
    (my $rest, my $pos) = @_;

    if (
	$rest =~ /^(?<pref>$borderorspc)(?<var1>$varnum)(?<s1>\s+)(?<op>$binop)(?<s2>\s+)(?<var2>$varnum)$borderorspc/
	) {
	print "case 4 ";
	my $s2 = $+{pref}.$+{var1}.$+{s1}.$+{op}.$+{s2}.$+{var2};
	return (1, $pos + length ($+{pref}), $pos + length ($s2));
    }

    if (
	$rest =~ /^(?<pref>$borderorspc)(?<var>$varnum)(?<spc2>\s*)(?<op>$binop)/
	) {
	print "case 1 ";
	my $s2 = $+{pref}.$+{var}.$+{spc2}.$+{op};
	return (1, $pos + length($+{pref}), $pos+length ($s2));
    }

    if (
	$rest =~ /^(?<op>$binop)(?<spc1>\s*)(?<var>$varnum)$borderorspc/ 
	) {
	print "case 2 ";
	my $s2 = $+{op}.$+{spc1}.$+{var};
	return (1, $pos, $pos+length ($s2));
    }

    if (
	$rest =~ /^(?<pref>$borderorspc)(?<var>$varnum)$borderorspc/
	) {
	print "case 3 ";
	my $s = $+{pref};
	my $v = $+{var};
	if (($v ne "1") && ($v ne "0")) {
	    return (1, $pos + length($s), $pos+length ($s.$v));
	}
    }

    if (
	$rest =~ /^(?<pref>$borderorspc)(?<var1>$varnum)(?<ques>\s*\?\s*)(?<var2>$varnum)(?<colon>\s*\:\s*)(?<var3>$varnum)$borderorspc/
	) {
	print "case 5 ";
	my $prefl = length ($+{pref});
	my $s2 = $+{var1}.$+{ques}.$+{var2}.$+{colon}.$+{var3};
	return (1, $pos + $prefl, $pos + $prefl + length ($s2));
    }

    if (0) {
	if ($rest =~ /^($border)/) {
	    print "case 6 ";
	    my $s2 = $1;
	    return (1, $pos, $pos+length ($s2));
	}
    }

    return (0,0,0);
}

my $good_cnt;
my $bad_cnt;
my $pass_num = 0;

sub delta_step ($$) {
    (my $method, my $start_pos) = @_;

    my $pos = $start_pos;

    while (1) {
	return 0 if ($pos >= length ($prog));

	my $first = substr($prog, 0, $pos);
	my $rest = substr($prog, $pos, -1);

	if ($method eq "replace_with_1") {
	    (my $success, my $start, my $end) = 
		match_subexp ($rest, $pos);
	    if ($success) {
		my $del = substr ($prog, $start, $end-$start);
		substr ($prog, $start, $end-$start) = "1";
		($del =~ s/\s/ /g);
		print "replacing '$del' at $start--$end : ";
		return (1, $pos);
	    } 
	} elsif ($method eq "replace_with_0") {
	    (my $success, my $start, my $end) = 
		match_subexp ($rest, $pos);
	    if ($success) {
		my $del = substr ($prog, $start, $end-$start);
		substr ($prog, $start, $end-$start) = "0";
		($del =~ s/\s/ /g);
		print "replacing '$del' at $start--$end : ";
		return (1, $pos);
	    }
	} elsif ($method eq "replace_with_nothing") {
	    (my $success, my $start, my $end) = 
		match_subexp ($rest, $pos);
	    if ($success) {
		my $del = substr ($prog, $start, $end-$start);
		substr ($prog, $start, $end-$start) = "";
		($del =~ s/\s/ /g);
		print "replacing '$del' at $start--$end : ";
		return (1, $pos);
	    }
	} elsif ($method eq "replace_regex") {
	    foreach my $str (keys %replace_regexes) {
		my $repl = $replace_regexes{$str};
		if ($rest =~ s/^$str/$repl/) {
		    $prog = $first.$rest;
		    return (1, $pos);
		}
	    }
	} elsif ($method eq "del_blanks_all") {
	    if ($prog =~ s/\s{2,}/ /g) {
		return (1, $pos);
	    } else {
		return 0;
	    }
	} elsif ($method eq "del_blanks") {
	    if ($rest =~ /^(\s{2,})/) {
		my $len = length ($1);
		substr ($prog, $pos, $len) =  " ";
		return (1, $pos);
	    }
	} elsif ($method eq "parens_inclusive") {
	    if (substr($prog, $pos, 1) eq "(") {
		my $p2 = find_match ($pos+1,"(",")");
		if ($p2 != -1) {
		    die if (substr($prog, $pos, 1) ne "(");
		    die if (substr($prog, $p2, 1) ne ")");
		    substr ($prog, $pos, $p2-$pos+1) = "";
		    print "deleting at $pos--$p2 : ";
		    return (1, $pos);
		}
	    }
	} elsif ($method eq "parens_exclusive") {
	    if (substr($prog, $pos, 1) eq "(") {
		my $p2 = find_match ($pos+1,"(",")");
		if ($p2 != -1) {
		    die if (substr($prog, $pos, 1) ne "(");
		    die if (substr($prog, $p2, 1) ne ")");
		    substr ($prog, $p2, 1) = "";
		    substr ($prog, $pos, 1) = "";
		    print "deleting at $pos--$p2 : ";
		    return (1, $pos);
		}
	    }
	} elsif ($method eq "brackets_inclusive") {
	    if (substr($prog, $pos, 1) eq "{") {
		my $p2 = find_match ($pos+1,"{","}");
		if ($p2 != -1) {
		    die if (substr($prog, $pos, 1) ne "{");
		    die if (substr($prog, $p2, 1) ne "}");
		    substr ($prog, $pos, $p2-$pos+1) = "";
		    print "deleting at $pos--$p2 : ";
		    return (1, $pos);
		}
	    }	    
	} elsif ($method eq "brackets_exclusive") {
	    if (substr($prog, $pos, 1) eq "{") {
		my $p2 = find_match ($pos+1,"{","}");
		if ($p2 != -1) {
		    die if (substr($prog, $pos, 1) ne "{");
		    die if (substr($prog, $p2, 1) ne "}");
		    substr ($prog, $p2, 1) = "";
		    substr ($prog, $pos, 1) = "";
		    print "deleting at $pos--$p2 : ";
		    return (1, $pos);
		}
	    }
	} elsif ($method eq "calls") {
	    foreach my $pref (keys %function_prefixes) {
		my $s = substr ($prog, $pos, length ($pref));
		if ($s eq $pref) {
		    my $c = del_up_to_matching_parens ($pos, $pref);
		    if ($c != -1) {
			print " : ";
			return (1, $pos);
		    }
		}
	    }
	} else {
	    die "unknown reduction method";
	}
	$pos++;
    }
}

sub runit ($) {
    (my $cmd) = @_;
    if ((system "$cmd") != 0) {
	return -1;
    }   
    return ($? >> 8);
}

sub run_test () {
    my $res = runit "./$test";
    return ($res == 0);
}

my %cache = ();
my $cache_hits = 0;

sub cached_test () {
    my $result = $cache{$prog};
    my $hit;
    if (defined($result)) {
	$cache_hits++;
	print "(hit) ";
	$hit = 1;
    } else {
	write_file ($cfile);
	$result = run_test ();
	$cache{$prog} = $result;
	$hit = 0;
    }
    return ($result, $hit);
}

# invariant: test always succeeds for $cfile.bak

my %method_worked = ();
my %method_failed = ();
my $old_size = 1000000000;

sub delta_pass ($) {
    (my $method) = @_;
    
    my $worked = 0;
    my $filepos=0;

    $good_cnt = 0;
    $bad_cnt = 0;
    
    while (1) {
	read_file ();    
	my $len = length ($prog);
	print "[$pass_num $method ($filepos / $len) s:$good_cnt f:$bad_cnt] ";
	(my $delete_res, my $newpos) = delta_step ($method, $filepos);
	if (!$delete_res) {
	    print "no more to delete.\n";
	    return $worked;
	}
	(my $result, my $hit) = cached_test();
	    
	if ($result) {
	    print "success\n";
	    die if ($hit);
	    system "cp $cfile $cfile.bak";
	    $good_cnt++;
	    $worked = 1;
	    $method_worked{$method}++;
	    my $size = length ($prog);
	    die if ($size > $old_size);
	    if ($size < $old_size) {
		%cache = ();
	    }
	    $old_size = $size;
	} else {
	    print "failure\n";
	    if (!$hit) {
		system "cp $cfile.bak $cfile";
	    }
	    if ($newpos <= $filepos) {
		$filepos++;
	    } else {
		$filepos = $newpos;
	    }
	    $bad_cnt++;
	    $method_failed{$method}++;
	}
    } 
}

my %all_methods = (

    "del_blanks_all" => -1,
    "del_blanks" => 0,

    "brackets_inclusive" => 1,
    "brackets_exclusive" => 4,

    "parens_inclusive" => 2,
    "parens_exclusive" => 5,

    "calls" => 1,

    "replace_with_0" => 6,
    "replace_with_1" => 6,
    "replace_with_nothing" => 6,

    "replace_regex" => 7,

    );
 
#################### main #####################

sub usage() {
    print "usage: c_delta.pl test_script.sh file.c [method [method ...]]\n";
    print "available methods are --all or:\n";
    foreach my $method (keys %all_methods) {
	print "  --$method\n";
    }
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
usage if (!defined(@ARGV));
foreach my $arg (@ARGV) {
    if ($arg eq "--all") {
	foreach my $method (keys %all_methods) {
	    $methods{$method} = 1;
	}
    } else {
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
}

print "making sure test succeeds on initial input...\n";
my $res = run_test ();
if (!$res) {
    die "test fails!";
}

system "cp $cfile $cfile.orig";
system "cp $cfile $cfile.bak";

sub bymethod {
    return $all_methods{$a} <=> $all_methods{$b};
}

# iterate to global fixpoint

while (1) {
    my $success = 0;
    foreach my $method (sort bymethod keys %methods) {
	$success |= delta_pass ($method);
    }
    $pass_num++;
    last if (!$success);
}

print "\n";
print "statistics:\n";
foreach my $method (sort keys %methods) {
    my $w = $method_worked{$method};
    $w=0 unless defined($w);
    my $f = $method_failed{$method};
    $f=0 unless defined($f);
    print "  method $method worked $w times and failed $f times\n";
}
print "there were $cache_hits cache hits\n";
