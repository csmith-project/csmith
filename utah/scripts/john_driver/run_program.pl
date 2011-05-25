#!/usr/bin/perl -w

use strict;
use warnings;
use English;
use Math::BigInt;
use Benchmark;

####################################################################

my $USE_VOLCHECK = 1;

my $USE_PINTOOL = 2;

my $CHECK_VOLATILE;

my $TIMEOUT = 5;

my $BAIL_ON_TIMEOUT = 1;

my $COMPILER_TIMEOUT_RES = 137;

my $PINTOOL_PREFIX = "";

if (defined($ENV{"PIN_CMD"})) {
    $PINTOOL_PREFIX = $ENV{"PIN_CMD"};
}

####################################################################

#{ 
#    my $ofh = select STDOUT;
#    $| = 1;
#    select $ofh;
#}

my %name_to_addr;
my %addr_to_name;

my $exe;

my $print_sizep = 1;

# properly parse the return value from system()
sub runit ($) {
    my $cmd = shift;

    my $start = new Benchmark;
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    my $end = new Benchmark;
    my $dur = timediff($end, $start);
    return ($exit_value, $dur);
}

# return a hash whose keys are the names of volatile variables
sub find_volatiles ($) {
    (my $cfile) = @_;

    open INF, "<$cfile" or die "FAIL couldn't read cfile";
    my %vols;
    my $vcount = 0;
    while (my $line = <INF>) {
	chomp $line;
	if ($line =~ /\/\/ VOLATILE GLOBAL (.+)$/) {
	    my $v = $1;
	    $vols{$v} = 1;
	    $vcount++;
	    print "volatile $v\n";
	}
    }
    close INF;

    print "$vcount volatile variables\n";

    return ($vcount, \%vols);
}

sub parse_output ($$$$) {
    (my $out, my $arch, my $volref, my $timeout) = @_;

    my %vols = %{$volref};
    my %reads = ();
    my %writes = ();
    my $vol_accesses = "";

    if (($CHECK_VOLATILE == $USE_VOLCHECK) && ($arch eq "ia32" || $arch eq "arm")) {
	foreach my $v (sort keys %vols) {
	    $reads{$v} = 0;
	    $writes{$v} = 0;
	}
    }
    
    # oi...
    if ($arch eq "arm") {
	system "cat armul.trc >> $out" || die;
    }

    open INF, "<$out" or die "FAIL couldn't open program output file '$out'";
    my $csum;

    if ($timeout) {
	$csum = "TIMEOUT";
    }

    while (my $line = <INF>) {
	
	#print $line;

	chomp $line;
	
	if ($arch eq "ia32" || $arch eq "arm" || $arch eq "windiss") {
	    if ($line =~ /checksum = ([0-9a-fA-F]+)/) {
		$csum = $1;
		next;
	    }
	    if (($CHECK_VOLATILE == $USE_VOLCHECK) &&
		$arch eq "arm" &&
		$line =~ /^M[NS]([RW])([1-4]).... ([0-9a-fA-F]+) ([0-9a-fA-F]+)$/) {
		my $action = $1;
		my $bytes = $2;
		my $hexaddr = $3;
		my $val = $4;
		my $addr = hex($hexaddr);
		my $var = $addr_to_name{$addr};

		next if (!defined($var));

		## FIXME TODO handle fragmented accesses

		if ($action eq "R") {
		    $reads{$var}++;
		} elsif ($action eq "W") {
		    $writes{$var}++;
		} else {
		    die "FAIL impossible action '$action' in volcheck output";
		}
		next;
	    }

	    if (($CHECK_VOLATILE == $USE_VOLCHECK) &&
		$arch eq "ia32" &&
		$line =~ /\s(load|store)\s+0x([0-9a-fA-F]+)\s+([0-9]+)\s+([0-9]+)$/) {
		my $action = $1;
		my $hexaddr = $2;
		my $addr = hex($hexaddr);
		my $bytes = $3;
		my $count = $4;
		my $var = $addr_to_name{$addr};

		next if (!defined($var));

		## FIXME TODO handle fragmented accesses

		if ($action eq "load") {
		    next if (!defined($reads{$var}));
		    $reads{$var} = $count;
		} elsif ($action eq "store") {
		    next if (!defined($writes{$var}));
		    $writes{$var} = $count;
		} else {
		    die "FAIL impossible action '$action' in volcheck output";
		}
		next;
	    }
	    if (($CHECK_VOLATILE == $USE_PINTOOL) &&
		($arch eq "ia32") &&
		($line =~ /([0-9a-fA-F]+):\s+([0-9]+)\s+reads,\s+([0-9]+)\s+writes$/)) {
                $vol_accesses .= "$line\n";
            }

	} elsif ($arch eq "avr") {

	    #print "$line\n";

	    if ($line =~ /r30\:r31 = 0x([0-9a-fA-F]+)/) {
		$csum = $1;
		next;
	    }
	
	    my $num_or_nan = "[0-9\\.]+|\\(NaN\\)";
    
	    if (($CHECK_VOLATILE == $USE_VOLCHECK) &&
		$line =~ /^\s*0x([0-9a-fA-F]+):\s+([0-9]+)\s+($num_or_nan)\s+\%\s+([0-9]+)\s+($num_or_nan)\s+%\s*$/) {
		my $addr = hex($1); 
		my $reads = $2;
		my $writes = $4; 
		my $var = $addr_to_name{$addr + 0x800000};

		#print "addr = $addr, reads = $reads, writes = $writes\n";
		
		next if ($reads == 0 && $writes == 0);
		next if (!defined($var)); # FAIL here?
		die "bug FAIL" if (defined($reads{$var}));
		die "bug FAIL" if (defined($writes{$var}));
		$reads{$var} = $reads;
		$writes{$var} = $writes-1; # correct for initialization
		next;
	    }
	} elsif ($arch eq "msp") {
	    if ($line =~ /checksum = ([0-9a-fA-F]+)/) {
		$csum = $1;
	    }

	    if (($CHECK_VOLATILE == $USE_VOLCHECK) &&
		$line =~ /\[ ([0-9]+) reads: ([0-9]+) writes: ([0-9]+) \]/) {
		my $addr = $1;
		my $reads = $2;
		my $writes = $3;

		my $var = $addr_to_name{$addr};
		
		#print "addr = $addr, reads = $reads, writes = $writes\n";

		next if (!defined($var));
		die "bug FAIL" if (defined($reads{$var}));
		die "bug FAIL" if (defined($writes{$var}));
		$reads{$var} = $reads;
		$writes{$var} = $writes-1; # correct for initialization
		next;
	    }
	} else {
	    die "unknown arch '$arch'";
	}
    }
    
    if (!defined($csum)) {
	print STDERR "BAILING -- no checksum\n";
	exit (-1);
    }
    
    my $results = "checksum = $csum\n";
    
    my $lame = 1;
    if ($CHECK_VOLATILE == $USE_VOLCHECK) {
	foreach my $v (sort keys %vols) {
	    #print "$v\n";
	    die "bug FAIL" if (!defined($reads{$v}));
	    die "bug FAIL" if (!defined($writes{$v}));
	    if ($reads{$v} != 1 || $writes{$v} != 0) {
		$lame = 0;
	    }
	    $results .= "$v: $reads{$v} reads, $writes{$v} writes\n";
	}
    }
    elsif ($CHECK_VOLATILE == $USE_PINTOOL) {
        $results .= "$vol_accesses";
    }

    if (0) {
    #if ($lame) {
	print STDERR "BAILING -- lame accesses\n";
	exit (-1);
    }
    
    return $results;
}

sub get_syms ($) {
    (my $nm) = @_;
    open INF, "$nm $exe |" or die "FAIL";
    while (<INF>) {
	if (/^([0-9a-fA-F]+) [RrBbDd] ([0-9a-zA-Z\_]+)$/) {
	    #my $address = hex($1);
            my $address = Math::BigInt->new($1)->as_hex;
	    my $name = $2;
	    $name_to_addr{$name} = $address;
	    $addr_to_name{$address} = $name;
	    # print "$name is at $address\n";
	}
    }
    close INF;
}

sub get_arm_syms () {
    open INF, "fromelf -s $exe |" or die "FAIL";
    while (<INF>) {
	if (/([0-9]+)\s+([0-9a-zA-Z\_]+)\s+0x([0-9a-fA-F]+)/) {
	    my $address = hex($3);
	    my $name = $2;
	    $name_to_addr{$name} = $address;
	    $addr_to_name{$address} = $name;
	}
    } 
    close INF;
}

sub print_size ($) {
    (my $sizecmd) = @_;
    open INF, "$sizecmd $exe |" or die "FAIL";
    my $l1 = <INF>;
    my $l2 = <INF>;
    ($l2 =~ /^\s*([0-9]+)\s/);
    my $bytes = $1;
    close INF;
    print "CODE SIZE $bytes\n";
}

############################## main ###########################

my $VOLATILE_PATH=$ENV{"CSMITH_HOME"}."/utah/scripts/john_driver";
die "oops: VOLATILE_PATH environment variable needs to be set"
    if (!defined($VOLATILE_PATH));

my $nargs = scalar(@ARGV);

die "FAIL expecting exe-filename c-filename architecture check_volatile? compiler [nosize]" if 
    ($nargs != 4 && $nargs != 5);
$exe = $ARGV[0];
my $cfile = $ARGV[1];
my $arch = $ARGV[2];
$CHECK_VOLATILE = $ARGV[3];
my $compiler = $ARGV[4];
if ($nargs == 6) {
    if ($ARGV[5] eq "nosize") {
	$print_sizep = 0;
    } else {
	die "FAIL 6th arg must be 'nosize'";
    }
}

# on darwin timeout res is 152 instead of 137
if ($OSNAME =~ /darwin/) {
    $COMPILER_TIMEOUT_RES = 152;
}

# print "$exe $cfile $arch\n";

die "FAIL" if (
    ($arch ne "avr") && 
    ($arch ne "ia32") && 
    ($arch ne "msp") &&
    ($arch ne "arm") &&
    ($arch ne "windiss")
    );

my $out = "${exe}.raw-out";

my %vols = ();
if ($CHECK_VOLATILE == $USE_VOLCHECK) {
    %vols = %{find_volatiles($cfile)};
}

my $timeout = 0;

if ($arch eq "ia32") {

    my $cmd_prefix = "";
    my $res = 0;
    my $dur;

    if ($CHECK_VOLATILE == $USE_VOLCHECK) {
	$cmd_prefix = "valgrind --tool=volcheck --trace-mem=yes --missing-loc-ok=yes  --print-mem-per=no";
    }
    elsif ($CHECK_VOLATILE == $USE_PINTOOL) {
	$cmd_prefix = "$PINTOOL_PREFIX";
        # I don't know why , but pintool make TIMEOUT doesn't work. 
        # So we run exe without pintool and if it's timeout, then
        # we don't need to hang on forever
        ($res, $dur) = runit ("RunSafely.sh $TIMEOUT 1 /dev/null $out ./$exe");
        $TIMEOUT = 30;
    }

    if (!$res) {
        ($res, $dur) = runit ("RunSafely.sh $TIMEOUT 1 /dev/null $out $cmd_prefix ./$exe");
    }

    if ($res == $COMPILER_TIMEOUT_RES) {
	print STDERR "TIMEOUT\n";
	if ($BAIL_ON_TIMEOUT) {
	    exit (-1);
	} else {
	    $timeout = 1;
	}
    } elsif ($res != 0) {
	print STDERR "UNEXPECTED $arch $compiler PROGRAM FAIL, retval = ${res}\n";
	exit (-1);
    }

    get_syms ("nm");
    if ($print_sizep) {
	print_size ("size");
    }

} elsif ($arch eq "windiss") {

    # hack-- we won't worry about a timeout
    
    my $cmd = "windiss ./$exe";
    (my $res, my $dur) = runit ("RunSafely.sh 30 1 /dev/null $out ${cmd}");

    if ($res == 137) {
	print STDERR "TIMEOUT\n";
	exit (-1);
    }

    if ($res != 0) {
	print STDERR "UNEXPECTED $arch $compiler PROGRAM FAIL, retval = ${res}\n";
	exit (-1);
    }

} elsif ($arch eq "arm") {

    my $arm_cmd = "rvdebug --batch --script=${VOLATILE_PATH}/rvd_script_1";
    system "rm -f armul.trc";
    system "cp -f $exe __image.axf";
    # extra long time out-- this is just to cover up stupid bugs, the 
    # sim is supposed to timeout on its own due to watchdog!
    (my $res, my $dur) = runit ("RunSafely.sh 300 1 /dev/null $out ${arm_cmd}");
    system "cp -f armul.trc $exe.trc" || die;
    system "rm -f __image.axf" || die;

    if ($res == 137) {
	print STDERR "TIMEOUT\n";
	exit (-1);
    }

    if ($res != 0) {
	print STDERR "UNEXPECTED $arch $compiler PROGRAM FAIL, retval = ${res}\n";
	exit (-1);
    }

    get_arm_syms();
    
} elsif ($arch eq "avr") {
    get_syms ("avr-nm");
    my $watches = "";
    my $first = 1;
    foreach my $v (keys %vols) {
	my $addr = $name_to_addr{$v} - 0x800000;
	if (defined($addr)) {
	    if (!$first) {
		$watches .= ",";
	    } else {
		$first = 0;
	    }
	    $watches .= "$v";
	} else {
	    # not an error to get here -- compiler can eliminate
	    # unused volatiles
	}
    }
    my $avrora_options = "-platform=mica2 -simulation=sensor-network -monitors=memory,break -colors=false -seconds=15";
    my $avrora_cmd = "java -server avrora.Main $avrora_options -locations=$watches $exe > $out";
    # print STDERR "${avrora_cmd}\n";
    (my $res, my $dur) = runit ("${avrora_cmd}");

    if ($res == 137) {
	print STDERR "TIMEOUT\n";
	exit (-1);
    }

    if ($res != 0) {
	print STDERR "UNEXPECTED $arch $compiler PROGRAM FAIL, retval = ${res}\n";
	exit (-1);
    }

    if ($print_sizep) {
	print_size ("avr-size");
    }

} elsif ($arch eq "msp") {
    get_syms ("msp430-nm");
    my $mspsim_cmd = "java se.sics.mspsim.platform.sky.SkyNode $exe -nogui > $out";
    (my $res, my $dur) = runit ("RunSafely.sh $TIMEOUT 1 /dev/null $out $mspsim_cmd");

    if ($res == 137) {
	print STDERR "TIMEOUT\n";
	exit (-1);
    }

    if ($res != 0) {
	print STDERR "UNEXPECTED $arch $compiler PROGRAM FAIL, retval = ${res}\n";
	exit (-1);
    }

    if ($print_sizep) {
	print_size ("msp430-size");
    }
} else {
    die "FAIL";
}
    
my $result = parse_output ($out, $arch, \%vols, $timeout);
print "$result";
# print STDERR $result;
exit (0);
