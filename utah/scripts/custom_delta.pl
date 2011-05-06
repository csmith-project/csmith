#!/usr/bin/perl -w

use strict;

# todo: cache results!  or at least a hash of results

# todo: input file should print output separately instead of checksum
#   probably do this via command line option?

# assumption: we're processing code that has been run through 'indent'
# which adds white space around operators and in other places

# goal: completely dismantle a preprocessed csmith output

# todo: need more minimal matching, probably

# hard
#   transform a function to return void
#   inline a function call
#   sort functions in order to eliminate prototypes
#   un-nest nested calls
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
my $borderspc = "(\\s+|$border)";

#print "$field\n";
#print "$index\n";
#print "$border\n";
#print "$var1\n";
#print "$var2\n";

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
    "continue" => "", 
    "return" => "",
    "for" => "",
    "if" => "",
    "break" => "", 
    "inline" => "", 
    "printf" => "",
    "int\\s*;" => "",
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

sub read_file ($)
{
    (my $cfile) = @_;
    open INF, "<$cfile" or die;
    $prog = "";
    while (my $line = <INF>) {
	$prog .= $line;
    }
    close INF;
}

sub write_file ($)
{
    (my $cfile) = @_;
    open OUTF, ">$cfile" or die;
    print OUTF $prog;
    close OUTF;
}

sub match_subexp ($$) {
    (my $prog, my $pos) = @_;

    my $s = substr ($prog, $pos, -1);

    if (
	$s =~ /^(?<pref>$borderspc)(?<var>$varnum)(?<spc2>\s*)(?<op>$binop)/
	) {
	my $s2 = $+{pref}.$+{var}.$+{spc2}.$+{op};
	return (1, $pos + length($+{pref}), $pos+length ($s2));
    }

    if (
	$s =~ /^(?<op>$binop)(?<spc1>\s*)(?<var>$varnum)$borderspc/ 
	) {
	my $s2 = $+{op}.$+{spc1}.$+{var};
	return (1, $pos, $pos+length ($s2));
    }

    if (
	$s =~ /^(?<pref>$borderspc)(?<var>$varnum)$borderspc/
	) {
	my $s = $+{pref};
	my $v = $+{var};
	if (($v ne "1") && ($v ne "0")) {
	    return (1, $pos + length($s), $pos+length ($s.$v));
	}
    }

    if (
	$s =~ /^(?<pref>$borderspc)(?<var1>$varnum)(?<s1>\s+)(?<op>$binop)(?<s2>\s+)(?<var2>$varnum)$borderspc/
	) {
	my $s2 = $+{pref}.$+{var1}.$+{s1}.$+{op}.$+{s2}.$+{var2};
	return (1, $pos + length ($+{pref}), $pos + length ($s2));
    }

    if (
	$s =~ /^(?<pref>$borderspc)(?<var1>$varnum)(?<ques>\s*\?\s*)(?<var2>$varnum)(?<colon>\s*\:\s*)(?<var3>$varnum)$borderspc/
	) {
	my $prefl = length ($+{pref});
	my $s2 = $+{var1}.$+{ques}.$+{var2}.$+{colon}.$+{var3};
	return (1, $pos + $prefl, $pos + $prefl + length ($s2));
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

	my $rest = substr($prog, $pos, -1);

	if ($method eq "replace_with_1") {
	    (my $success, my $start, my $end) = 
		match_subexp ($prog, $pos);
	    if ($success) {
		my $del = substr ($prog, $start, $end-$start);
		substr ($prog, $start, $end-$start) = "1";
		($del =~ s/\s/ /g);
		print "replacing '$del' at $start--$end : ";
		return (1, $pos);
	    } 
	} elsif ($method eq "replace_with_0") {
	    (my $success, my $start, my $end) = 
		match_subexp ($prog, $pos);
	    if ($success) {
		my $del = substr ($prog, $start, $end-$start);
		substr ($prog, $start, $end-$start) = "0";
		($del =~ s/\s/ /g);
		print "replacing '$del' at $start--$end : ";
		return (1, $pos);
	    }
	} elsif ($method eq "replace_with_nothing") {
	    (my $success, my $start, my $end) = 
		match_subexp ($prog, $pos);
	    if ($success) {
		my $del = substr ($prog, $start, $end-$start);
		substr ($prog, $start, $end-$start) = "";
		($del =~ s/\s/ /g);
		print "replacing '$del' at $start--$end : ";
		return (1, $pos);
	    }
	} elsif ($method eq "replace_regex1") {
	    foreach my $str (keys %replace_regexes) {
		if ($rest =~ /^(?<pref>$borderspc)(?<str>$str)(?<suf>$borderspc)/) {
		    my $repl = $+{str};
		    print "replacing '$repl' at $pos : ";
		    substr ($prog, 
			    $pos + length ($+{pref}), 
			    length ($repl)) 
			= $replace_regexes{$str};
		    return (1, $pos);
		}
	    }
	} elsif ($method eq "replace_regex2") {
	    foreach my $str (keys %replace_regexes) {
		if ($rest =~ /^(?<pref>$borderspc)(?<str>$str)(?<suf>$borderspc)/) {
		    my $repl = $+{pref}.$+{str};
		    print "replacing '$repl' at $pos : ";
		    substr ($prog, 
			    $pos, 
			    length ($repl)) 
			= $replace_regexes{$str};
		    return (1, $pos);
		} 
	    }
	} elsif ($method eq "replace_regex3") {
	    foreach my $str (keys %replace_regexes) {
		if ($rest =~ /^(?<pref>$borderspc)(?<str>$str)(?<suf>$borderspc)/) {
		    my $repl = $+{pref}.$+{str}.$+{suf};
		    print "replacing '$repl' at $pos : ";
		    substr ($prog, 
			    $pos,
			    length ($repl))
			= $replace_regexes{$str};
		    return (1, $pos);
		} 
	    }
	} elsif ($method eq "replace_regex4") {
	    foreach my $str (keys %replace_regexes) {
		if ($rest =~ /^(?<pref>$borderspc)(?<str>$str)(?<suf>$borderspc)/) {
		    my $repl = $+{str}.$+{suf};
		    print "replacing '$repl' at $pos : ";
		    substr ($prog, 
			    $pos + length ($+{pref}), 
			    length ($repl)) 
			= $replace_regexes{$str};
		    return (1, $pos);
		}
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

sub run_test ($) {
    (my $test) = @_;
    my $res = runit "./$test";
    return ($res == 0);
}

# invariant: test always succeeds for $cfile.bak

my %method_worked = ();
my %method_failed = ();

sub main_loop ($$$) {
    (my $cfile, my $test, my $method) = @_;
    
    my $worked = 0;
    my $filepos=0;

    $good_cnt = 0;
    $bad_cnt = 0;
    
    while (1) {
	read_file ($cfile);    
	my $len = length ($prog);
	print "[$pass_num $method ($filepos / $len) s:$good_cnt f:$bad_cnt] ";
	(my $delete_res, my $newpos) = delta_step ($method, $filepos);
	if (!$delete_res) {
	    print "no more to delete.\n";
	    return $worked;
	}
	write_file ($cfile);
	if (run_test ($test)) {
	    print "success\n";
	    system "cp $cfile $cfile.bak";
	    $good_cnt++;
	    $worked = 1;
	    $method_worked{$method}++;
	} else {
	    print "failure\n";
	    system "cp $cfile.bak $cfile";
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

    "del_blanks" => 0,

    "brackets_inclusive" => 1,
    "brackets_exclusive" => 4,

    "parens_inclusive" => 2,
    "parens_exclusive" => 5,

    "calls" => 1,

    "replace_with_0" => 6,
    "replace_with_1" => 6,
    "replace_with_nothing" => 6,

    "replace_regex1" => 7,
    "replace_regex2" => 7,
    "replace_regex3" => 7,
    "replace_regex4" => 7,

    );
 
#################### main #####################

sub usage() {
    print "usage: custom_delta.pl test_script.sh file.c [method [method ...]]\n";
    print "available methods are --all or:\n";
    foreach my $method (keys %all_methods) {
	print "  --$method\n";
    }
    die;
}

my $test = shift @ARGV;
usage if (!defined($test));
if (!(-x $test)) {
    print "test script '$test' not found, or not executable\n";
    usage();
}

my $cfile = shift @ARGV;
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
my $res = run_test ($test);
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
	$success |= main_loop ($cfile, $test, $method);
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
