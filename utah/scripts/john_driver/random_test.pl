#!/usr/bin/perl -w

use strict;
use Digest::MD5 qw(md5 md5_hex md5_base64);
use File::stat;

# when errors are detected, fork off reduction scripts

##################################################################

my $SAVE_BADS = 0;

my $MIN_PROGRAM_SIZE = 20000;
my $MAX_PROGRAM_SIZE = 50000;

my $CSMITH_TIMEOUT = 180;

my $PROVIDE_SEED = 1;

my $XTRA = "";
$XTRA .= " --force-globals-static ";
#$XTRA .= "--no-argc";
#$XTRA .= "--concise ";
#$XTRA .= "--no-paranoid ";
#$XTRA .= "--random-random ";
#$XTRA .= "--math-notmp ";

my $BF = " --bitfields ";
#my $BF = "";

#my $PACK = "--packed-struct";
my $PACK = "";

my $QUIET = "--quiet";
#my $QUIET = "";

my $notmp = "-DUSE_MATH_MACROS_NOTMP";
#my $notmp = "";

my $PINTOOL_VOL_ADDR = "vol_addr.txt";

# my $platform = "x86";
my $platform = "x86_64";

# remove the comment below to enable ccomp test
#my $CSMITH_CCOMP = "$BF --quiet --no_return_dead_ptr --no-math64 --no-volatiles --ccomp --math-notmp";
# my $CSMITH_CCOMP = "$BF --quiet --enable-volatile-tests x86 --vol-addr-file $PINTOOL_VOL_ADDR --no-math64 --ccomp --max-array-dim 3 --max-array-len-per-dim 5 --max-struct-fields 5 --math-notmp";

my $CSMITH_CCOMP = "";

# set up pintool for volatile testing
my $use_pintool = 0;
# my $use_pintool = 1;

#my $PIN_MODE = "-output_mode verbose"; # We are not supporting it right now
my $PIN_MODE = "";

if ($use_pintool) {
    # Before you could use pintool to test volatile accesses, change the pintool location when necessary!
    my $PIN_HOME = $ENV{"PIN_HOME"};
    die "oops: PIN_HOME environment variable needs to be set"
        if (!defined($PIN_HOME));

    $XTRA .= " --enable-volatile-tests $platform --vol-addr-file $PINTOOL_VOL_ADDR ";
    my $pin_cmd;

    # make sure we are going to use the correct version of pinatrace.so,
    # i.e., obj-ia32/pinatrace.so for x86 and obj-intel64/pinatrace.so for x86_64. 
    # For testing compcert, we use the ia32 version
    if (($platform eq "x86") || (not ($CSMITH_CCOMP eq ""))) {
        $pin_cmd = "$PIN_HOME/ia32/bin/pinbin -t $PIN_HOME/source/tools/ManualExamples/obj-ia32/pinatrace.so -vol_input $PINTOOL_VOL_ADDR $PIN_MODE --";
    }
    elsif ($platform eq "x86_64") {
        $pin_cmd = "$PIN_HOME/intel64/bin/pinbin -t $PIN_HOME/source/tools/ManualExamples/obj-intel64/pinatrace.so -vol_input $PINTOOL_VOL_ADDR $PIN_MODE --";
    }
    else {
        die "Invalid platform[$platform] for pintool!";
    }

    $ENV{"PIN_CMD"} = $pin_cmd;
}

##################################################################

my $CSMITH_HOME = $ENV{"CSMITH_HOME"};

my $good = 0;

my %checksums;

my $TIMED_TEST;

# properly parse the return value from system()
sub runit ($) {
    my $cmd = shift;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

sub doit ($$) {
    (my $n, my $work) = @_;
    print "------ RANDOM PROGRAM $n ------\n";
    if ($TIMED_TEST || ($n%100)==0) {
	print "timestamp: ";
	system "date";
    }
    my $nstr = sprintf "%06d", $n;
    my $dir = "${work}/$nstr";

    system "mkdir $dir";
    chdir $dir;

    my $fn = "rand$nstr";
    my $cfile = "${fn}.c";

    my $SEED = "";
    if ($PROVIDE_SEED) {
	my $n = int (rand (2147483647));
	$SEED = "-s $n";
	print "seed = $n\n";
    }
    if ($TIMED_TEST) {
	die if ($PROVIDE_SEED);
	my $line = <SEEDFILE>;
	if (!$line) {
	    print "input exhausted, exiting.\n";
	    exit (0);
	}
	chomp $line;
	printf "seedfile line: $line\n";
	die if (!($line =~ /\(([0-9]+)\,([0-9]+)\,([0-9]+)\)/));
	$SEED = "-s $1 --max-block-size $2 --max-funcs $3";
    }

    my $cmd;
    if ($CSMITH_CCOMP eq "") {
        $cmd = "$CSMITH_HOME/src/csmith $SEED $BF $PACK $XTRA --output $cfile";
    }
    else {
        $cmd = "$CSMITH_HOME/src/csmith $SEED $CSMITH_CCOMP --output $cfile";
    }
    if ($PROVIDE_SEED) {
	print "$cmd\n";
    }
    my $res = runit ("RunSafely.sh $CSMITH_TIMEOUT 1 /dev/zero csmith_output.txt $cmd");

    if ($res != 0 || !(-e "$cfile")) {
	print "CSMITH FAILED\n";
	system "cat csmith_output.txt";
	chdir "../..";
	system "rm -rf $dir";
	return;
    }

    my $filesize = stat("$cfile")->size;
    print "$cfile is $filesize bytes\n";    
    if (($filesize < $MIN_PROGRAM_SIZE) && !$TIMED_TEST) {
	print "FILE TOO SMALL\n";
	chdir "../..";
	system "rm -rf $dir";
	return;
    }

    my $seed;
    my $prog = "";
    my $vcount = 0;

    open INF, "<$cfile" or die;
    while (my $line = <INF>) {
	if ($line =~ /volatile/) {
	    $vcount++;
	}
	if ($line =~ /Seed:\s+([0-9]+)$/) {
	    $seed = $1;
	}
	chomp $line;
	$prog .= "$line ";
    }
    close INF;
    die if (!defined($seed));
    if (!$PROVIDE_SEED) {
	print "regenerate with: '$cmd -s $seed'\n";
    }

    ($prog =~ s/\/\*(.*?)\*\///g);
    my $digest = md5($prog);
    if ($checksums{$digest}) {
	print "BAD PROGRAM: DUPLICATE\n";
	chdir "../..";
	system "rm -rf $dir";
	return;
    }
    $checksums{$digest} = 1;

    if ($vcount < 1) {
	print "NOT ENOUGH VOLATILES\n";
	chdir "../..";
	system "rm -rf $dir";
	return;
    }

    if ($CSMITH_CCOMP ne "") {
        # ccomp doesn't like asserts, regenerate random programs without asserts.
        my $noparanoid_cfile = "${fn}_small.c";
        my $cmd1 = "$CSMITH_HOME/src/csmith $CSMITH_CCOMP -s $seed  --output $noparanoid_cfile";
        my $res1 = runit ("RunSafely.sh $CSMITH_TIMEOUT 1 /dev/zero randprog_output.txt $cmd1");
    }

    my $ret = system "evaluate_program.pl $fn";
    my $rc = ($ret>>8) & 0xff;

    chdir "../..";

    #print "evaluate program returned $rc\n";

    if ($rc == 0 || $rc == 1) {
	print "GOOD PROGRAM: number $good\n";
	$good++;
    } else {
	print "BAD PROGRAM: doesn't count towards goal.\n";
    }

    # if ($rc != 1 && !$SAVE_BADS) {
    if (1) {
	system "rm -rf $dir";
    }
}

if (scalar(@ARGV) != 1 &&
    scalar(@ARGV) != 2) {
    die "usage: random_test.pl work_dir [seedfile]";
}

my $work = $ARGV[0];

if (!(-d $work)) {
    die "error: create work dir '$work' first";
}

if (scalar(@ARGV)==2) {
    my $seedfile = $ARGV[1];
    open SEEDFILE, "<$seedfile" or die "error: cannot open seed file $seedfile";
    $TIMED_TEST = 1;
    $PROVIDE_SEED = 0;
} else {
    $TIMED_TEST = 0;
}

my $i = 0;
while (1) {
    doit ($i, $work);
    $i++;
    print "\n";
} 

##################################################################
