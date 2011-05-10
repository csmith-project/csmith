#!/usr/bin/perl -w

use strict; 
use File::stat;
##################################################################
my $MIN_PROGRAM_SIZE = 10000;
my $CSMITH_TIMEOUT = 90; 
my $COMPILER_TIMEOUT = 10;
my $PROG_TIMEOUT = 5;

my $XTRA =  "--no-paranoid ";
my $BF = " --bitfields ia32";  
my $PACK = "--packed-struct"; 
my $QUIET = "--quiet";
my $platform = "x86";

##################################################################
my $CSMITH_PATH = $ENV{"CSMITH_PATH"};
my $good = 0; 
my $bug = 0;
my @COMPILERS = (
    "cl -Od",
    "cl -O1",
    "cl -O2",
    "cl -Og",
    "cl -Oi",
    "cl -Os",
    "cl -Ot",
    "cl -Ox",
    "cl -Oy",
);
my $HEADER = "-I$CSMITH_PATH/runtime";

sub read_value_from_file($$) {
    my ($fn, $match) = @_;
    open INF, "<$fn" or die;
    while (my $line = <INF>) {
        chomp $line; 
        if ($line =~ /$match/) {
            close INF;
            return $1;
        }     
    }
    close INF;
    return "";
}


# properly parse the return value from system()
sub runit ($$$) {
    my ($cmd, $timeout, $out) = @_;
    # print "before running $cmd\n";
    my $res = system "timeout $timeout $cmd &> $out";
    my $success = 0; 
    if ($? == -1) {
        print "can't execute $cmd\n";
    }
    elsif ($? & 127) {
        print "died while executing $cmd\n";
    }
    else {
        $success = 1;
    }
    my $exit_value  = $? >> 8;
    if ($exit_value == 124) {
        print "hangs while executing $cmd\n";
        $success = 0;
    }
    return ($success, $exit_value);
}

# compile a program and execute
# return 0: normal; 1: compiler crashed; 2: compiler hang; 3: executable crashed; 4: executable hang
sub compile_and_run($$$$) {
    my ($compiler, $src_file, $exe, $out) = @_;
    my $command = "$compiler $src_file $HEADER -o $exe";
    # print "$command\n"; 
    my ($res, $exit_value) = runit($command, $COMPILER_TIMEOUT,  "compiler.out"); 
    # print "after run compiler: $res, $exit_value\n";
    if (($res == 0) || (!(-e $exe))) {
        return ($exit_value == 124 ? 2 : 1);       # exit code 124 means time out, see timeout.sh
    }
    ($res, $exit_value) = runit("./$exe", $PROG_TIMEOUT, $out);
    # print "after run program: $res, $exit_value\n";
    if (($res == 0) || (!(-e $out))) {
        return ($exit_value == 124 ? 4 : 3);      # exit code 124 means time out, see timeout.sh
    }
    return 0;
}

sub evaluate_program ($) {
    my ($test_file) = @_; 
    my $checksum = "";
    my $i = 0;
    foreach my $compiler (@COMPILERS) {        
        my $out = "out$i.log";
        my $exe = "a$i.exe";
        $i++;
        my $res = compile_and_run($compiler, $test_file, $exe, $out);
        open INF, "ps axu |" or die;
         while (<INF>) {
            if (/\/sh/) {
	system "ps axu";
	exit;
           }
        }
       close INF;

        if ($res) {
	if ($res == 1 || $res == 2) {
	    print "compiler error! Can't compile $test_file with $compiler\n";
	    return 1;
                }
        	if ($res == 3) {
	    print "random program crashed!\n";
	    return 1;
	}
                print "random program hangs!\n";  
	return -1;    # program hangs, not interesting
        }
        else {
	my $sum = read_value_from_file($out, "checksum = (.*)");
                if ($sum ne $checksum && $checksum ne "") {
                    print "find checksum difference!\n";
                    return 1;
                }
                $checksum = $sum;             
        }
    }
    return 0;
}

sub test_one ($$) {
    (my $n, my $work) = @_;
    print "------ RANDOM PROGRAM $n ------\n";
    if (($n%100)==0) {
	print "timestamp: ";
	system "date";
    }
    my $nstr = sprintf "%06d", $n;
    my $dir = "${work}/$nstr";

    system "mkdir $dir";
    chdir $dir;

    my $fn = "rand$nstr";
    my $cfile = "${fn}.c";
   
    # run Csmith
    my $cmd = "$CSMITH_PATH/src/csmith $BF $PACK $XTRA --output $cfile";
    my ($res, $exitcode) = runit($cmd, $CSMITH_TIMEOUT,  "csmith.out"); 
    # print "after run csmith: $res, $exitcode\n";

    if ($res == 0 || !(-e "$cfile")) {
        print "CSMITH FAILED\n";
        system "cat csmith.out";
        chdir "../";
        system "rm -rf $dir";
        return;
    }

    my $filesize = stat("$cfile")->size;
    print "$cfile is $filesize bytes\n";    
    if (($filesize < $MIN_PROGRAM_SIZE)) {
        print "FILE TOO SMALL\n";
        chdir "../";
        system "rm -rf $dir";
        return;
    }

    my $seed = read_value_from_file($cfile, "Seed:\\s+([0-9]+)"); 
    die if (!$seed);  

    # test if the random program is interesting
    my $ret = evaluate_program($cfile);
    chdir "../"; 

    if ($ret == 0 || $ret  == 1) {
        print "GOOD PROGRAM: number $good\n";
        $good++;
        if ($ret == 1) {
            print "FAILURE-INDUCING PROGRAM: number $bug\n";
            $bug++;
        }
    } else {
        print "BAD PROGRAM: doesn't count towards goal.\n";
    }  
}

########################### main ##################################
if (scalar(@ARGV) != 2) {
    print "usage: random_test_msvc.pl work_dir testcase_cnt (0 for unlimited)\n";
    exit -1;
}
my $work = $ARGV[0];
my $cnt = $ARGV[1];
if (!(-d $work)) {
    die "error: create work dir '$work' first";
}
my $i = 0;
while ($cnt <=0 || $i < $cnt) {
    test_one ($i, $work);
    $i++;
    print "\n";
} 
print "total errors found: $bug\n";
##################################################################
