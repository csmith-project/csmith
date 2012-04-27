#!/usr/bin/perl 

use strict;
use warnings;
use Benchmark;
use English;
use LockFile::Simple;

# TODO: check for program crash (vs. timeout)

# TODO: we hold the lock for too long, put it into compile_llvm?

# abstract over compilation and evaluation strategies

# properly propagate run_program failures upwards

# piece together byte-level information from run_program

# distinguish between programs that compile and those that run inside the timeout

####################################################################

my $SKIP = 0;

my $NOTEFILE_PREFIX="logged_";

my $xxtra = "-DCSMITH_MINIMAL";

#my $notmp = "-DUSE_MATH_MACROS_NOTMP";
my $notmp = "";

my $COMPILER_TIMEOUT = 600;

my $COMPILER_TIMEOUT_RES = 137;

my $RUN_PROGRAM = 1;

my $DO_TRIAGE = 0;

my $VERSION_SEARCH = 0;

my $BAIL_ON_ZERO_WRITES = 0;

####################################################################

my $CSMITH_HOME=$ENV{"CSMITH_HOME"};
die "oops: CSMITH_HOME environment variable needs to be set"
    if (!defined($CSMITH_HOME));

my $LOCKFN = "/var/tmp/version_search_lockfile";

####################################################################

my @gcc_opts2 = (
    "-O0", 
    #"-O1", 
    #"-O2", 
    #"-Os", 
    "-O3",
    );

my @gcc_opts9 = (
    "-O0 -fwrapv",
    "-O1 -fwrapv",
    "-O2 -fwrapv",
    "-Os -fwrapv",
    "-O3 -fwrapv",
    );

my $DRAGONEGG = "-fplugin=/home/regehr/z/compiler-source/dragonegg/dragonegg.so";

my @dragonegg_opts = (
    "-O0 $DRAGONEGG", 
    "-O1 $DRAGONEGG", 
    "-O2 $DRAGONEGG", 
    "-Os $DRAGONEGG", 
    "-O3 $DRAGONEGG", 
    );

my @gcc_opts3 = (
    #"-O0 -m32", 
    "-O0", 
    #"-O1 -fomit-frame-pointer -fwhole-program", 
    #"-O2", 
    #"-Os", 
    #"-O3 -m32",
    "-O3 -fno-strict-aliasing",
    );

my @gcc_opts4 = (
    "-O0", 
    "-Os", 
    "-Os -fno-asynchronous-unwind-tables -fno-common -fno-delete-null-pointer-checks -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-strict-aliasing -fno-strict-overflow -fstack-protector -fstack-protector-all -ftest-coverage -g -m32 -march=i486 -mno-3dnow -mno-mmx -mno-sse -mno-sse2 -mpreferred-stack-boundary=2 -msoft-float",
    );

my @gcc_opts5 = (
    "-O0", 
    "-O1", 
    "-O2 -march=core2 -mtune=core2 -fno-stack-protector -fomit-frame-pointer", 
    "-Os", 
    "-Os -fconserve-stack -fno-asynchronous-unwind-tables -fno-common -fno-delete-null-pointer-checks -fno-dwarf2-cfi-asm -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-strict-aliasing -fno-strict-overflow -fprofile-arcs -fstack-protector -ftest-coverage -funit-at-a-time -g -m64 -maccumulate-outgoing-args -mno-3dnow -mno-mmx -mno-red-zone -mno-sse -mno-sse2 -mtune=generic -pg", 
    "-O3 -fwhole-program -combine",
    );

my @clang_opts = (
    "-O0 ", 
    "-O1 ", 
    "-O2  -march=core2 -mtune=core2 -fomit-frame-pointer -fno-stack-protector ", 
    "-Os ", 
    "-O3 ", 
    );

my @clangwrapv_opts = (
    "-O0 -fwrapv", 
    "-O1 -fwrapv", 
    "-O2 -fwrapv -march=core2 -mtune=core2 -fomit-frame-pointer -fno-stack-protector", 
    "-Os -fwrapv", 
    "-O3 -fwrapv",
    );

my @gcc_686_opts = (
    "-O0 -march=i686", 
    "-O1 -march=i686", 
    "-O2 -march=i686", 
    "-Os -march=i686", 
    "-O3 -march=i686", 
    );

my @llvm_opts_sse3 = (
    "-O0 -msse3", 
    "-O1 -msse3", 
    "-O2 -msse3", 
    "-Os -msse3", 
    "-O3 -msse3",
    );

my @armcc_opts = (
    "-O0", 
    "-O1", 
    "-O2 -Ospace", 
    "-O2 -Otime", 
    "-O3 -Ospace",
    "-O3 -Otime",
    );

my @dcc_opts = (
    "-tARMES:windiss -DSTANDALONE",
    "-tARMES:windiss -DSTANDALONE -O",
    "-tARMES:windiss -DSTANDALONE -XO",
    );

my @gcc_opts_noO0 = ("-O1", 
		     "-Os", 
		     "-O2", 
		     "-O3");

my @gcc_opts = ("-O0", 
		"-O1", 
		"-Os", 
		"-O2", 
		"-O3");

my @Oonly = ("-O");

my @Osonly = ("-Os");

my @none = ("-DTCC");

my @O0sonly = ("-O0", "-Os");

my @O0only = ("-O0");

my @suncc_opts = (
    "", 
    "-xO1",
    "-xO2",
    "-xO3",
    "-xO4",
    "-xO5",
    "-fast",
    );

my @icc_opts = (
    "-O0",
    "-O1",
    "-Os",
    "-O2",
    "-fast -ipo",
    );

my @avrdeputy = ("avr",
		 "deputy",
		 "my-deputy-avr.pl",
		 \@Osonly);

my @clang_Oonly = ("ia32",
		   "clang",
		   "clang",
		   \@Oonly);

my @lance = ("ia32",
	     "clang",
	     "run-lance",
	     \@Osonly);

my @tendra = ("ia32",
	      "tendracc",
	      "tendracc",
	      \@gcc_opts2);

my @llvmgcc = ("ia32",
	       "llvm",
	       "llvm-gcc",
	       \@gcc_opts2);

my @cparser = ("ia32",
	       "cparser",
	       "cparser-driver",
	       \@Osonly);

my @tcc = ("ia32",
	   "tcc",
	   "tcc",
	   \@none);

my @dcc = ("windiss",
	   "dcc",
	   "dcc",
	   \@dcc_opts);

my @armcc = ("arm",
	     "armcc",
	     "armcc",
	     \@armcc_opts);

my @avrgcc = ("avr",
	      "gcc",
	      "avr-gcc",
	      \@gcc_opts2);

my @avrgcc34 = ("avr",
		"gcc",
		"avr-gcc-3.4",
		\@gcc_opts2);

my @mspgcc = ("msp",
	      "gcc",
	      "msp430-gcc",
	      \@gcc_opts2);

my @avrgcc42_O0sonly = ("avr",
			"gcc",
			"avr-gcc-4.2",
			\@O0sonly);

my @avrgcc42 = ("avr",
		"gcc",
		"avr-gcc-4.2",
		\@gcc_opts2);

my @deputy = ("ia32",
	      "deputy",
	      "/home/regehr/tmp/deputy/bin/deputy",
	      \@Osonly);

my @nesc = ("ia32",
	    "nesc",
	    "nescc",
	    \@Osonly);

my @gcc = ("ia32",
	   "gcc",
	   "gcc",
	   \@gcc_opts2);

my $Z = "/home/regehr/z/compilers";

my @clang26 = ("ia32",
	       "llvm",
	       "$Z/clang-26/bin/clang",
	       \@gcc_opts2);

my @llvmgcc27 = ("ia32",
		 "llvm",
		 "llvm-gcc-27",
		 \@gcc_opts2);

my @clang27 = ("ia32",
	       "llvm",
	       "$Z/clang-27/bin/clang",
	       \@gcc_opts2);

my @llvmgcc28 = ("ia32",
		 "llvm",
		 "llvm-gcc-28",
		 \@gcc_opts2);

my @clang28 = ("ia32",
	       "llvm",
	       "$Z/clang-28/bin/clang",
	       \@gcc_opts2);

my @llvmgcc29 = ("ia32",
		 "llvm",
		 "llvm-gcc-29",
		 \@gcc_opts2);

my @clang29 = ("ia32",
	       "llvm",
	       "$Z/clang-29/bin/clang",
	       \@gcc_opts2);

my @gcc460 = ("ia32",
	      "gcc",
	      "$Z/gcc-460/bin/gcc-460",
	      \@gcc_opts2);

my @open64 = ("ia32",
	      "opencc",
	      "opencc",
	      \@gcc_opts2);

my @gcc450 = ("ia32",
	      "gcc",
	      "$Z/gcc-450/bin/gcc-450",
	      \@gcc_opts2);

my @gcc440 = ("ia32",
	      "gcc",
	      "$Z/gcc-440/bin/gcc-440",
	      \@gcc_opts2);

my @gcc430 = ("ia32",
	      "gcc",
	      "$Z/gcc-430/bin/gcc-430",
	      \@gcc_opts2);

my @gcc420 = ("ia32",
	      "gcc",
	      "$Z/gcc-420/bin/gcc-420",
	      \@gcc_opts2);

my @gcc410 = ("ia32",
	      "gcc",
	      "$Z/gcc-410/bin/gcc-410",
	      \@gcc_opts2);

my @gcc400 = ("ia32",
	      "gcc",
	      "$Z/gcc-400/bin/gcc-400",
	      \@gcc_opts2);

my @gcc340 = ("ia32",
	      "gcc",
	      "$Z/gcc-340/bin/gcc-340",
	      \@gcc_opts2);

my @gcc330 = ("ia32",
	      "gcc",
	      "$Z/gcc-330/bin/gcc-330",
	      \@gcc_opts2);

my @gcc320 = ("ia32",
	      "gcc",
	      "$Z/gcc-320/bin/gcc-320",
	      \@gcc_opts2);

my @clang = ("ia32",
	     "clang",
	     "clang",
	     \@gcc_opts3);

my @clangpp = ("ia32",
	     "clang",
	     "clang++",
	     \@gcc_opts2);

my @gcccurrent = ("ia32",
		  "gcc",
		  "current-gcc",
		  \@gcc_opts3);

my @gppcurrent = ("ia32",
		  "gcc",
		  "current-g++",
		  \@gcc_opts3);

my @dragonegg = ("ia32",
		  "gcc",
		  "gcc-4.5",
		  \@dragonegg_opts);

my @gcccurrent2 = ("ia32",
		   "gcc",
		   "current-gcc",
		   \@gcc_opts4);

my @gcccurrent_Oonly = ("ia32",
			"gcc",
			"current-gcc",
			\@Oonly);

my @gcccurrent_O0only = ("ia32",
			 "gcc",
			 "current-gcc",
			 \@O0only);

my @suncc = ("ia32",
	     "suncc",
	     "suncc",
	     \@suncc_opts);

my @llvmgcc19 = ("ia32",
		 "llvm",
		 "llvm-gcc-19",
		 \@gcc_opts2);

my @llvmgcc20 = ("ia32",
		 "llvm",
		 "llvm-gcc-20",
		 \@gcc_opts2);

my @llvmgcc21 = ("ia32",
		 "llvm",
		 "llvm-gcc-21",
		 \@gcc_opts2);

my @llvmgcc22 = ("ia32",
		 "llvm",
		 "llvm-gcc-22",
		 \@gcc_opts2);

my @llvmgcc23 = ("ia32",
		 "llvm",
		 "llvm-gcc-23",
		 \@gcc_opts2);

my @llvmgcc24 = ("ia32",
		 "llvm",
		 "llvm-gcc-24",
		 \@gcc_opts2);

my @llvmgcc25 = ("ia32",
		 "llvm",
		 "llvm-gcc-25",
		 \@gcc_opts2);

my @llvmgcc26 = ("ia32",
		 "llvm",
		 "llvm-gcc-26",
		 \@gcc_opts2);

my @llvmgcc22O0only = ("ia32",
		       "llvm",
		       "llvm-gcc-2.2",
		       \@O0only);

my @llvmgcccurrent = ("ia32",
		      "llvm",
		      "llvm-gcc-current",
		      \@gcc_opts2);

my @llvmgcc_Oonly = ("ia32",
		     "llvm",
		     "llvm-gcc",
		     \@Oonly);

my @icc = ("ia32",
	   "icc",
	   "icc",
	   \@icc_opts);

my @icc_O0only = ("ia32",
		  "icc",
		  "icc",
		  \@O0only);

my @llvms = (
    \@llvmgcc20, 
    \@llvmgcc21, 
    \@llvmgcc22, 
    \@llvmgcc23, 
    \@llvmgcc24, 
    );

my @compilers_to_test = (
    
    \@clang,
    \@gcccurrent,
    
    #\@gcc320,
    #\@gcc330,
    #\@gcc340,
    #\@gcc400,
    #\@gcc410,
    #\@gcc420,
    #\@gcc430,
    #\@gcc440,
    #\@gcc450,
    #\@gcc460,
    #\@clang26,
    #\@clang27,
    #\@clang28,
    #\@clang29,
    
    #\@icc,
    #\@suncc,
    #\@open64,

    # \@tcc,

    #\@gcc,

    #\@clangpp,
    #\@gppcurrent,

    #\@gcc400,

    #\@gcccurrent,
    #\@dragonegg,

    #\@dcc,
    #\@lance,
    #\@gcccurrent,
    #\@llvmgcc,
    #\@clang,

    #\@gppcurrent,
    #\@avrgcc,
    #\@gcccurrent_O0only,
    #\@armcc,
    #\@cparser,
    #\@llvmgcc24, 
    #\@llvmgcc_Oonly,
    #\@gcccurrent_Oonly,

    );

####################################################################

# properly parse the return value from system()
sub runit ($) {
    my $cmd = shift;

    my $start = new Benchmark;
    my $res = (system "$cmd");
    my $end = new Benchmark;
    my $dur = timediff($end, $start);
    my $exit_value  = $? >> 8;
    return ($exit_value, $dur);
}

# build and run the app, with timeouts for both the compiler and the program
sub compile_and_run ($$$$$) {
    (my $root, my $arch, my $compiler, 
     my $opt, my $custom_options) = @_;

    print "[$arch] $compiler $opt : ";

    my $opt_str = $opt;
    ($opt_str =~ s/\ //g);
    ($opt_str =~ s/\://g);
    ($opt_str =~ s/\-//g);
    ($opt_str =~ s/\///g);
    if (length($opt_str)>40) {
	$opt_str = substr ($opt_str, 0, 40);
    }
    
    my $exe;
    my $xtra;
    if ($arch eq "ia32" || $arch eq "windiss") {
	$exe = "${root}${compiler}${opt_str}_exe";
	($exe =~ s/\.//g);
	($exe =~ s/\///g);
	$xtra = "";
    } elsif ($arch eq "avr") {
	$exe = "${root}${compiler}${opt_str}.elf";
	$xtra = "-mmcu=atmega128 -fwhole-program";
    } elsif ($arch eq "msp" ) {
	$exe = "${root}${compiler}${opt_str}.elf";
	$xtra = "-mmcu=msp430x1611";
    } elsif ($arch eq "arm" ) {
	$exe = "${root}${compiler}${opt_str}.axf";
	$xtra = "--cpu=ARM926EJ-S -L--datacompressor=off -W";
    } else {
	die;
    }

    my $srcfile = "$root.c";

    $xtra .= " -w -DINLINE=";

    my $out = "${exe}.out";
    my $compilerout = "${exe}_compiler.out";

    my $command = "RunSafely.sh $COMPILER_TIMEOUT 1 /dev/null $compilerout $compiler $opt $xtra $xxtra $notmp -I${CSMITH_HOME}/runtime $srcfile -o $exe $custom_options $notmp ";

    print "$command\n";
    
    (my $res, my $dur) = runit ($command);

    if (($res != 0) || (!(-e $exe))) {
	if ($res == $COMPILER_TIMEOUT_RES) {
	    print STDERR "COMPILER FAILURE: TIMEOUT\n";
	} else {
	    print STDERR "COMPILER FAILURE with return code $res; output is:\n";
	    open INF, "<$compilerout" or die;
	    while (my $line = <INF>) { print "  $line"; }
	    close INF;
	}
	return (-2,"",-1);
    }
    
    if (!$RUN_PROGRAM) {
	return (0,"",0);
    }

    ($res, $dur) = runit ("run_program.pl $exe $srcfile $compiler > $out");

    if ($res != 0) {
	print "couldn't compute access summary\n";
	return (-1,"",-1);
    }
    
    # system "grep 'cpu time' ${exe}.raw-out.time";

    my $result = "";
    my $code_size = 0;

    open INF, "<$out" or die;
    while (my $line = <INF>) {
	if ($line =~ /CODE SIZE ([0-9]+)$/) {
	    $code_size = $1;
	} else {
	    $result .= $line;
	}
    }
    close INF;

    die if (!defined($code_size));

    return (0,$result,$code_size);
}

sub instantiate_test_scripts ($$$$) {
    (my $infn, my $opt1, my $opt, my $base_compiler) = @_;
    open INF, "<$infn" or die;
    open OUTF, ">./test1.sh" or die;
    while (<INF>) {
	(s/XX_WORKING_OPT/$opt1/g);
	(s/XX_BROKEN_OPT/$opt/g);
	if ($base_compiler eq "llvm") {
	    (s/XX_COMPILER/llvm-gcc/g);
	} elsif ($base_compiler eq "gcc") {
	    (s/XX_COMPILER/gcc/g);
	} elsif ($base_compiler eq "armcc") {
	    # nothing
	} else {
	    die;
	}
	print OUTF;
    }
    close INF;
    close OUTF;
    open INF, "<$infn" or die;
    open OUTF, ">./test2.sh" or die;
    while (<INF>) {
	(s/XX_WORKING_OPT/$opt1/g);
	(s/XX_BROKEN_OPT/$opt/g);
	(s/XX_COMPILER/\$1/g);
	print OUTF;
    }
    close INF;
    close OUTF;
    runit ("chmod a+x ./test1.sh") || die;
    runit ("chmod a+x ./test2.sh") || die;
}

my $lockmgr;

sub lockit() {
    $lockmgr = LockFile::Simple->make (
	-autoclean => 1,
	-max => 10, 
	-nfs => 1,
	-hold => 15000,			    
	-stale => 1,
	);
    my $res = $lockmgr->lock($LOCKFN);
    if (!$res) {
	print "couldn't get lockfile to build compiler -- not doing triage on this bug\n";
    }
    return $res;
}

sub unlockit() {
    $lockmgr->unlock($LOCKFN);
}

sub triage ($$$$$$$) {
    (my $compiler, my $templ, 
     my $opt1, my $opt, 
     my $root, my $base_compiler,
     my $version_searchp, my $arch) = @_;
    return if (!$DO_TRIAGE);
    return if (
	$base_compiler ne "gcc" &&
	$base_compiler ne "llvm" &&
	$base_compiler ne "armcc");
    if (lockit()) {
	my $res;
	print "diffing $opt1 and $opt\n";			    
	runit "cp $root.c small.c" or die;
	instantiate_test_scripts ($templ, $opt1, $opt, $base_compiler);		
	if ($VERSION_SEARCH) {
	    if ($base_compiler ne "armcc" &&
		$arch ne "avr") {
		$res = runit ("version_search.pl $base_compiler");
	    }
	}
	runit ("reduce_miscompile.sh > reduce_log.txt 2>&1");
	print ("reduced size: ");
	runit ("wc -c small.c");
	if ($VERSION_SEARCH) {
	    if ($base_compiler ne "armcc" &&
		$arch ne "avr") {
		$res = runit ("version_search.pl $base_compiler");
	    }
	}
	unlockit();			
    }
	    }

sub test_compiler ($$$) {
    (my $root, my $compiler_ref, my $custom_options) = @_;

    (my $arch, my $base_compiler, my $compiler, my $optref) = @{$compiler_ref};
    my @OPTS = @{$optref};

    my $undef;

    print "--------------------\n";

    if ($SKIP) {
	if ((-f "../../${NOTEFILE_PREFIX}checksum_$compiler.txt") &&
	    (-f "../../${NOTEFILE_PREFIX}checksum_$compiler.txt")) {
	    print "skipping $compiler\n";
	    return (0, 0, 0, "f", 1);
	}
    }
    
    my %results;
    my %csums;
    my $min_code_size = 100000000;
    my $success = 0;
    my $compiler_fail = 0;

    my %var_reads;
    my %var_writes;
    my %num_reads;
    my %num_writes;
    my $first = 1;

    foreach my $opt (@OPTS) {
	(my $res, my $res_str, my $code_size) = 
	    compile_and_run ($root, $arch, $compiler, 
			     $opt, $custom_options);
        $num_reads{$opt} = 0;
        $num_writes{$opt} = 0;
	if ($res == 0) {
	    $success++;
	    if ($RUN_PROGRAM) {
		if ($code_size < $min_code_size) {
		    $min_code_size = $code_size;
		}
		my $checksum_regex = "checksum = (TIMEOUT|[0-9a-fA-F]+)\\s*";
		die if ((!($res_str =~ s/$checksum_regex//)) &&
		        (!($res_str =~ "TIMEOUT")));
		my $csum = $1;
		print "$res_str";
		print "checksum = $csum\n";
		$results{$opt} = $res_str;
		$csums{$opt} = $csum;

		my $tot_reads = 0;
		my $tot_writes = 0;
		
		while ($res_str =~ /([0-9a-zA-Z\_]+): ([0-9]+) reads, ([0-9]+) writes/g) {
		    $var_reads{$opt}{$1} = $2;
		    $var_writes{$opt}{$1} = $3;
		    $num_reads{$opt} += $2;
		    $num_writes{$opt} += $3;
		    $tot_reads += $2;
		    $tot_writes += $3;
		}

		$first = 0;

	    }
	} elsif ($res == -2) {
	    $compiler_fail++;
	} else {
	    die if ($res != -1);
	    return (1, 0, $undef, $undef);
	}
    }

    my $result;
    my $csum;
    my $writes;
    my $interesting = 0;

    if ($compiler_fail > 0) {
	print "COMPILER FAILED $compiler\n";
	system "touch ../../${NOTEFILE_PREFIX}crash_$compiler.txt";
	$interesting = 1;
    }

    if ($success > 0) {
	
	my $consistent = 1;
	my $opt1;
	
	print "CODE SIZE $compiler ${min_code_size}\n";

	foreach my $opt (keys %results) {

	    if (defined($result)) {
		if (($csum ne $csums{$opt}) &&
		    ($csum ne "TIMEOUT" && $csums{$opt} ne "TIMEOUT")) {
		    print "INTERNAL CHECKSUM FAILURE $compiler $opt\n";
		    system "touch ../../${NOTEFILE_PREFIX}checksum_$compiler.txt";
		    $interesting = 1;
		    triage($compiler, 
			   "test-${arch}-${base_compiler}-wrong-code-template.sh", 
			   $opt1, $opt, $root, $base_compiler, $arch);
		    $consistent = 0;
		    last;
		}
	    } else {
		$writes = $num_writes{$opt};
		$opt1 = $opt;
		$result = $results{$opt};
		$csum = $csums{$opt};				
	    }
	}

	return (0, $consistent, $result, $csum, $interesting);

    } else {
	return (0, 0, $result, $csum, $interesting);
    }
		   }

sub test_program ($$) {
    (my $root, my $custom_options) = @_;

    my $vcount;
    my %mt;

    my $result;
    my $csum;

    my $interesting = 0;

    foreach my $compiler_ref (@compilers_to_test) {
	(my $abort_test, my $consistent, my $tmp_result, my $tmp_csum, my $tmp_interesting) = 
	    test_compiler ($root, $compiler_ref, $custom_options);
	return -1 if ($abort_test != 0);
	
	if ($tmp_interesting) {
	    $interesting = 1;
	}
	
	(my $arch, my $base_compiler, my $compiler, my $optref) = @{$compiler_ref};
	print "COMPLETED TEST $compiler\n";
	
	# ignore internally inconsistent results
	if ($consistent) {
	    if (defined ($result) &&
		defined ($csum)) {
		if (($csum ne $tmp_csum) &&
		    ($csum ne "TIMEOUT") &&
		    ($tmp_csum ne "TIMEOUT")) {
		    print "EXTERNAL CHECKSUM FAILURE\n";
		    $interesting = 1;
		}
	    } else {
		$result = $tmp_result;
		$csum = $tmp_csum;
	    }
	}
    }

    if ($interesting) {
	return 1;
    } else {
	return 0;
    }
}

####################################################################

die "expecting filename" if scalar(@ARGV < 1);

my $fn = $ARGV[0];

my $custom_options = "";
if (scalar(@ARGV)==2) {
    $custom_options = $ARGV[1];
}

my $res = test_program ($fn, $custom_options);
exit ($res);

####################################################################

