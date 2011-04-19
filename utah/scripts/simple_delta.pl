#!/usr/bin/perl -w

use strict;
use File::stat;

# TODO: 
#
# use temp dirs 
# also handle crash bugs
#
# support systematic exploration of the space, instead of random?
# support a bit of backtracking: explore the best N results instead of
#   the best 1
# support running on multiple cores
# log results to a file that can be gnuplotted

##############################################################

my $CSMITH_PATH = $ENV{"CSMITH_PATH"};
die "please set CSMITH_PATH env first!" if (!defined($CSMITH_PATH));

my $VOLATILE = $ENV{"VOLATILE_PATH"};
die "please set VOLATILE_PATH env first!" if (!defined($VOLATILE));

$ENV{"PATH"} = "$ENV{VOLATILE_PATH}:$ENV{PATH}";

my $COMPILER_TIMEOUT = 600;
my $PROG_TIMEOUT = 2;

if (@ARGV != 4 && @ARGV != 3) {
    die "usage: simple_delta.pl <seed> <compiler1> <compiler2> <csmith-options>\n\n";
}

my $seed = $ARGV[0];
my $COMPILER1 = $ARGV[1];
my $COMPILER2 = $ARGV[2];
my $OPTS = "--concise ";
if (@ARGV == 4) {
    $OPTS =  "$OPTS$ARGV[3]";
}

my $cfile = "small.c"; 
my $tmp_cfile = "small_tmp.c";
my $backup_cfile = "works.c";
my $deltafile = "delta.out";
my $backup_deltafile = "works.out";
my $indent = "    ";
my $HEADER = "-I$CSMITH_PATH/runtime";
my $STOP_AFTER = 100000;
my $SUFFIX1 = "_a";
my $SUFFIX2 = "_b";
my $EXE1 = "$cfile$SUFFIX1";
my $EXE2 = "$cfile$SUFFIX2";
my $COMPILER_OUT1 = "compiler_out$SUFFIX1";
my $COMPILER_OUT2 = "compiler_out$SUFFIX2";
my $PROG_OUT1 = "prog_out$SUFFIX1";
my $PROG_OUT2 = "prog_out$SUFFIX2";

##############################################################

sub runit ($) {
    (my $cmd) = @_;
    if ((system "$cmd") != 0) {
	return -1;
    }   
    return ($? >> 8);
}

sub read_file($\@$) {
    my ($input, $out, $match) = @_;
    my $cnt = 0;
    open INF, "<$input" or die;
    while (my $line = <INF>) {
        chomp $line; 
        if ($line =~ /$match/) {
            push @$out, $line;
            $cnt++;
        } 
    }
    close INF;
    return $cnt;
	      }

sub compile($$$$) {
    my ($compiler, $src_file, $exe, $out) = @_;
    my $res = 0;
    my $dur;

    my $command = "RunSafely.sh $COMPILER_TIMEOUT 1 /dev/null $out $compiler $src_file $HEADER -o $exe";
    ($res, $dur) = runit($command);
    if (($res != 0) || (!(-e $exe))) {
        # timeout
        if ($res == 137) {
            print "$indent COMPILER($compiler) FAILURE: TIMEOUT\n";
        }
        else {
            print "$indent COMPILER($compiler) FAILURE: return code $res \n";
            print "$indent check the output file $out for details\n";
        }
        return -1;
    }
    else {
        return 0;
    }
}

sub run_exe ($$$) {
    my ($exe, $out, $param) = @_;
    my $res = 0;
    my $dur;
    my $command = "RunSafely.sh $PROG_TIMEOUT 1 /dev/null $out ./$exe $param >/dev/null 2>&1";
    ($res, $dur) = runit($command);
    if ($res) {
	# print "res = $res\n";
        # timeout
        if ($res == 137) {
            print "$indent Running $exe FAILURE: TIMEOUT\n";
            return 1;
        }
        if ($res == 139) {
            print "$indent Running $exe FAILURE: core dump\n";
            return 2;
        }
        else { 
            # print "$indent Running $exe FAILURE: return code $res\n";
	    return 3;
        } 
    }
    return 0;
}

# rv: 1 - not equal, 0 - equal
sub compare($\@$\@$$) {
    my ($file1, $out1, $file2, $out2, $flag1, $match) = @_;

    read_file($file1, @$out1, $match);
    read_file($file2, @$out2, $match);

    if ($flag1 && (@$out1 != @$out2)) {
        print "$indent COMPARISON FAILS: two outputs must have the different number of lines.";
        return -1;
    }
    else {
        if (@$out1 != @$out2) {
            return 1;
        }
        
        for (my $i=0; $i < @$out1; $i++) {
            return 1 if $out1->[$i] ne $out2->[$i];
        }
    }
    return 0;
}

my $test_count = 0;
my $ARGS="-g -Wmissing-prototypes -Werror=missing-prototypes -Wreturn-type -Werror=return-type -Wstrict-prototypes -Werror=strict-prototypes $HEADER";	
my $EXTRA='-Wuninitialized -Werror=uninitialized';
# flag1 == 1: we check if two outputs have the same lines. 
# flag2 == 1: we check the line which is "checksum = xxxx". 
sub run_test ($$$) {
    my ($test_file, $flag1, $match) = @_;
    system "rm -f gcc.txt";
    system "gcc $ARGS $EXTRA -O0 $test_file -o a.out > gcc.txt 2>&1";
    my @tmp = ();
    if (read_file("gcc.txt", @tmp, "no return statement") || 
	read_file("gcc.txt", @tmp, "control reaches") ||
	# read_file("gcc.txt", @tmp, "initia") ||
	read_file("gcc.txt", @tmp, "proto")) {
	system("cp $test_file gcc_fail.c");
	print "$indent compiler error! Can't compile $test_file with gcc\n";
	return 0;
    }
    
    $test_count++;
    my $res;
    $res = compile($COMPILER1, $test_file, "$EXE1$test_count", "$COMPILER_OUT1$test_count");
    if ($res) {
	print "$indent compiler error! Can't compile $test_file with $COMPILER1\n";
	return 0;
    }

    $res = compile($COMPILER2, $test_file, "$EXE2$test_count", "$COMPILER_OUT2$test_count");
    if ($res) {
	print "$indent compiler error! Can't compile $test_file with $COMPILER2\n";
	return 0;
    }

    $res = run_exe("$EXE1$test_count", "$PROG_OUT1$test_count", "");
    if ($res) {
	#print "$indent can't run the program: $EXE1$test_count\n";
	return 0;
    }
    
    $res = run_exe("$EXE2$test_count", "$PROG_OUT2$test_count", "");
    if ($res) {
	#print "$indent can't run the program: $EXE2$test_count\n";
	return 0;
    }
    
    my @OUTPUT1 = ();
    my @OUTPUT2 = ();
    $res = compare("$PROG_OUT1$test_count", @OUTPUT1, "$PROG_OUT2$test_count", @OUTPUT2, $flag1, $match);
    
    if ($res == -1) {
        die "comparison error!\n";
    }
    return $res;
}

sub get_score ($) {
    (my $fn) = @_;
    return stat($fn)->size;    
    
    # open INF, "<$fn" or die;
    # my $score = 0;
    # while (my $line = <INF>) {
    # if ($line =~ /safe_/) {
    #    $score += 500;
    # }
    # $score += length($line);
    # }
    # close INF;
    # return $score;
}

############################ MAIN ############################

print "seed = $seed\n";

system "${CSMITH_PATH}/src/csmith -s $seed $OPTS --delta-monitor simple --delta-output $deltafile > $cfile";

my $res = run_test($cfile, 0, "checksum ");
if (!$res) {
    die "oops-- initial test 1 fails\n";
}
print "initial test 1 succeeds\n";

system "${CSMITH_PATH}/src/csmith $OPTS --go-delta simple --delta-input $deltafile --no-delta-reduction > $cfile";

$res = run_test($cfile, 0, "checksum ");
if (!$res) {
    die "oops-- initial test 2 fails\n";
}
print "initial test 2 succeeds\n"; 

system "cp $cfile aaa.c";
my $orig_sz = get_score($cfile);

my $n = 0;

print "initial score = $orig_sz\n";
while (1) {
    my $sz = get_score($cfile);
    # invariant-- test succeeds here
    system "cp $cfile ${backup_cfile}";
    system "cp $deltafile ${backup_deltafile}";

    $n++;
    if ($n > $STOP_AFTER) {
	last;
    }

    system "${CSMITH_PATH}/src/csmith $OPTS --go-delta simple --delta-input ${backup_deltafile} --delta-output ${deltafile} > $cfile";
    my $new_sz = get_score ($cfile);

    my $success = 0;
    
    if ($new_sz < $sz) {
	my $pct = sprintf "%.1f", 100*(1.0-((0.0+$new_sz)/$orig_sz));
	my $r2 = run_test($cfile, 0, "checksum ");
	if ($r2) {
	    print "$n : new score = $new_sz (reduced by $pct %) : ";
	    print "test succeeded\n";
	    #print "success!!! new score = $new_sz($pct)\n";
	    $success = 1;
	} else {
	    # print "test failed\n";
	}
    } else {
	# print "not smaller\n";
    }

    if (!$success) {
	system "cp ${backup_cfile} $cfile";
	system "cp ${backup_deltafile} $deltafile";
    } else {
	$sz = $new_sz;
    }
}
system "cp $cfile simple$seed.c";
my $sz = get_score($cfile);
my $rate = sprintf "%.1f", 100*(1.0-((0.0+$sz)/$orig_sz));
print "=== simple delta reduction rate: $rate ===\n";  
print "final reduced file: simple$seed.c\n";
