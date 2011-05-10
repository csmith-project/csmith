#!/usr/bin/perl -w

use strict;
use warnings;

#####################################################################

# dump seeds for good programs into a separate file

#####################################################################

my $GOAL=100000;

my $SIZE=0;

my $WRAP=0;

my $MIN=1e-5;

#####################################################################

my %vol_error = ();
my %csum_error = ();
my %crash_error = ();
my %size = ();
my %total = ();

my %tmp_vol_error = ();
my %tmp_csum_error = ();
my %tmp_crash_error = ();
my %tmp_size = ();
my %tmp_total = ();

my %compilers = ();

my $n = 0;

sub def1($) {
    (my $x) = @_;
    return $x if (defined($x));
    return 0;
}

my $cur_prog;

if (scalar(@ARGV)==0) {
    die "no input files specified";
}

my @files = @ARGV;

foreach my $fn (@files) {
    print "$fn\n";
    open INF, "<$fn" or die;
    while (my $line = <INF>) {
	
	# print  $line;
	
	# here we do not care about inconsistency across tools 
	next if ($line =~ /EXTERNAL/);
	
	if ($line =~ /RANDOM PROGRAM ([0-9]+) /) {
	    $cur_prog = $1;
	    %tmp_vol_error = ();
	    %tmp_csum_error = ();
	    %tmp_crash_error = ();
	    %tmp_size = ();
	    %tmp_total = ();
	    next;
	}
	my $useful = "COMPILER FAILED|VOLATILE FAILURE|CHECKSUM FAILURE|COMPLETED TEST";
	if ($line =~ /($useful) ([a-zA-Z0-9\-\.]+) WRAP=(0|1)/) {
	    my $status = $1;
	    my $compiler = $2;
	    my $wrap = $3;
	    
	    die if (!$WRAP && $wrap==1);
	    
	    $compilers{$compiler} = 1;
	    
	    if ($status =~ /VOLATILE/) {
		die if (defined($tmp_vol_error{$wrap}{$compiler}));
		$tmp_vol_error{$wrap}{$compiler} = 1;
		$tmp_total{$wrap}{$compiler} = 1;
		next;
	    }
	    if ($status =~ /COMPILER/) {
		die if (defined($tmp_crash_error{$wrap}{$compiler}));
		$tmp_crash_error{$wrap}{$compiler} = 1;
		$tmp_total{$wrap}{$compiler} = 1;
		next;
	    }
	    if ($status =~ /CHECKSUM/) {
		die if (defined($tmp_csum_error{$wrap}{$compiler}));
		$tmp_csum_error{$wrap}{$compiler} = 1;
		$tmp_total{$wrap}{$compiler} = 1;
		next;
	    }
	    if ($status =~ /COMPLETED/) {
		$tmp_total{$wrap}{$compiler} = 1;
		next;
	    }
	    die;
	}
	if ($line =~ /CODE SIZE ([a-zA-Z0-9\-\.]+) WRAP=(0|1) ([0-9]+)$/) {
	    my $compiler = $1;
	    my $wrap = $2;
	    my $size = $3;
	    die if (defined($tmp_size{$wrap}{$compiler}));
	    $tmp_size{$wrap}{$compiler} = $size;
	    next;
	}
	
	my @wraps;
	if ($WRAP) {
	    @wraps = (0,1);
	} else {
	    @wraps = (0);
	}
	
	if ($line =~ /GOOD PROGRAM/) {
	    foreach my $wrap (@wraps) {
		foreach my $compiler (keys %{$tmp_total{$wrap}}) {
		    
		    if ($SIZE) {
			my $s = $tmp_size{$wrap}{$compiler};
			die if (!defined($s));
			$size{$wrap}{$compiler} += $s;
		    }
		    
		    my $csum = def1($tmp_csum_error{$wrap}{$compiler});
		    my $vol = def1($tmp_vol_error{$wrap}{$compiler});
		    my $crash = def1($tmp_crash_error{$wrap}{$compiler});
		    
		    $csum_error{$wrap}{$compiler} += $csum;
		    $vol_error{$wrap}{$compiler} += $vol;
		    $crash_error{$wrap}{$compiler} += $crash;
		    $total{$wrap}{$compiler}++;
		}
	    }
	    
	    $n++;
	    last if ($n >= $GOAL);
	    
	    if (($n%1000)==0) {
		my $k = $n/1000;
		print "${k} K\n";
	    }
	    
	    next;
	}
    
	die if ($line =~ /RANDOM/);
	die if ($line =~ /VOLATILE FAIL/);
	die if ($line =~ /CHECKSUM FAIL/);
	die if ($line =~ /COMPILER FAIL/);
	die if ($line =~ /COMPLET/);
	die if ($line =~ /CODE SIZE/);
    }
    close INF;
    last if ($n >= $GOAL);
}

if ($WRAP) {
    print "compiler            vol fail     vol w/wrap   change   csum fail    +size\n";
} else {
    print "compiler              vol fail          csum fail           crash\n";
}

my $all_total;
my $t_vol_errs = 0;
my $t_vol_errs_w = 0;

open VOL, ">volatile_errors.txt" or die;
open CSUM, ">checksum_errors.txt" or die;
open CRASH, ">crash_errors.txt" or die;

my %version_map = 
    ("llvm-gcc-2.0" => "2.0",
     "llvm-gcc-2.1" => "2.1",
     "llvm-gcc-2.2" => "2.2",
     "llvm-gcc-2.3" => "2.3",
     "llvm-gcc-2.4" => "2.4",
    );

sub by_version_map {
    return $version_map{$a} <=> $version_map{$b};
}

# todo-- implement exact binomial test?  or call out to R for this?

sub wilson_score ($$) {
    (my $fail, my $n) = @_;
    my $phat = 1.0 * $fail / $n;
    my $zc = 1.96; # 95%

    my $term1 = 
	$phat + 
	((1.0/(2.0*$n)) * ($zc * $zc));
    
    my $term2 = 
	$zc *
	sqrt ((($phat*(1.0-$phat))/$n) + 
	      (($zc * $zc) / (4.0 * $n * $n))
	);
	 
    my $term3 = 
	1.0 + 
	((1.0/$n) * ($zc * $zc));
	
    my $lo = ($term1 - $term2) / $term3;
    my $hi = ($term1 + $term2) / $term3;

    return ($lo, $hi);
}

sub max ($$) {
    (my $a, my $b) = @_;
    return $a if ($a>$b);
    return $b;
}

foreach my $compiler (sort by_version_map keys %compilers) {
    my $t = $total{0}{$compiler};

    # print "$compiler $t\n";

    if ($WRAP) {
	die if ($t != $total{1}{$compiler});
    }
    die if (defined($all_total) && ($t != $all_total));
    $all_total = $t;

    $t_vol_errs += $vol_error{0}{$compiler};
    if ($WRAP) {
	$t_vol_errs_w += $vol_error{1}{$compiler};
    }
    
    my $vol_errors = $vol_error{0}{$compiler};
    my $vol_frac = 1.0 * $vol_errors / $t;
    (my $vol_lower, my $vol_upper) = wilson_score ($vol_errors, $t);

    #print "volatile 95%: $vol_lower .. $vol_upper \n";

    my $csum_errors = $csum_error{0}{$compiler};
    my $csum_frac = 1.0 * $csum_errors / $t;
    (my $csum_lower, my $csum_upper) = wilson_score ($csum_errors, $t);

    #print "checksum 95%: $csum_lower .. $csum_upper \n";

    my $crash_errors = $crash_error{0}{$compiler};
    my $crash_frac = 1.0 * $crash_errors / $t;
    (my $crash_lower, my $crash_upper) = wilson_score ($crash_errors, $t);

    my $vol_frac_orig = $vol_frac;
    my $csum_frac_orig = $csum_frac;
    my $crash_frac_orig = $crash_frac;

    $vol_frac = max ($vol_frac, $MIN);
    $csum_frac = max ($csum_frac, $MIN);
    $crash_frac = max ($crash_frac, $MIN);

    $vol_lower = max ($vol_lower, $MIN);
    $csum_lower = max ($csum_lower, $MIN);
    $crash_lower = max ($crash_lower, $MIN);

    my $vol_pct = (100.0 * $vol_frac);
    my $csum_pct = (100.0 * $csum_frac);
    my $crash_pct = (100.0 * $crash_frac);

    my $vol_pct_orig = (100.0 * $vol_frac_orig);
    my $csum_pct_orig = (100.0 * $csum_frac_orig);
    my $crash_pct_orig = (100.0 * $crash_frac_orig);

    my $ver = $version_map{$compiler};

    print VOL "$ver $vol_pct ".($vol_lower*100.0)." ";
    print VOL (($vol_upper-$vol_lower)*100.0);
    printf VOL " %.3g%%\n", $vol_pct_orig;

    print CSUM "$ver $csum_pct ".($csum_lower*100.0)." ";
    print CSUM (($csum_upper-$csum_lower)*100.0);
    printf CSUM " %.3g%%\n", $csum_pct_orig;

    print CRASH "$ver $crash_pct ".($crash_lower*100.0)." ";
    print CRASH (($crash_upper-$crash_lower)*100.0);
    printf CRASH " %.3g%%\n", $crash_pct_orig;

    my $sz;
    if ($SIZE) {
	$sz = 1.0 * $size{0}{$compiler} / $t;
    }

    if ($WRAP) {
	my $vol_pct_w = (100.0 * $vol_error{1}{$compiler} / $t)+0.00000000000001;
	my $vol_change = 100.0 * (($vol_pct - $vol_pct_w) / $vol_pct);
	
	my $csum_pct_w = (100.0 * $csum_error{1}{$compiler} / $t)+0.00000000000001;
	my $csum_change = 100.0 * (($csum_pct - $csum_pct_w) / $csum_pct);

	my $sz_w = 1.0 * $size{1}{$compiler} / $t;
	my $inc = (100.0 * ($sz_w / $sz)) - 100.0;

#	format = 
#@<<<<<<<<<<<<< & @###### & @###### & & @###### \\
#$compiler, $vol_error{0}{$compiler}, $vol_error{1}{$compiler}, $csum_error{0}{$compiler}
#.
#        write;

#	format = 
#@<<<<<<<<<<<<< & @##.###\% & @##.###\% & @###\% & @##.#\% & @##.###\% \\
#$compiler, $vol_pct, $vol_pct_w, $vol_change, $inc, $csum_pct
#.
#        write;

    } else {
	format = 
@<<<<<<<<<<<<<  @##.###% @>>>>>>   @##.###% @>>>>>>>   @##.###% @>>>>>> \\
$compiler, $vol_pct, $vol_errors, $csum_pct, $csum_errors, $crash_pct, $crash_errors
.
        write;
    }
}

close VOL;
close CSUM;
close CRASH;

print "$all_total random programs were tested\n";
#print "there were $t_vol_errs volatile errors\n";
if ($WRAP) {
  print "there were $t_vol_errs_w volatile errors in wrapped programs\n";
  my $inc = 100.0 * (($t_vol_errs - $t_vol_errs_w) / $t_vol_errs);
  printf "%2.2f%% of errors were fixed using wrappers\n", $inc;
}

system "gnuplot < llvm_graphs.gp";

foreach my $epsfile (sort glob "*.eps") {
  system "epstopdf $epsfile";
  my $root = $epsfile;
  ($root =~ s/\.eps$//g);
  system "convert -trim -density 400x400 $root.pdf $root.png";
}
