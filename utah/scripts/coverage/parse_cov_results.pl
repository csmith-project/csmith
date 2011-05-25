#!/usr/bin/perl

use File::Basename;

use strict;


my $COV_BASE_HOME = $ENV{"COV_BASE_HOME"};

#die "please set env $COV_BASE_HOME first!"
#    if (!defined($COV_BASE_HOME));

# for my convenience
if (!defined($COV_BASE_HOME)) {
    $COV_BASE_HOME = "/uusoc/exports/scratch/chenyang/branch-test/test_data";
}

my $COV_DATA_HOME = "$COV_BASE_HOME";
my $LN_OUT = "$COV_BASE_HOME/ln.out";
my $FN_OUT = "$COV_BASE_HOME/fn.out";
my $BR_OUT = "$COV_BASE_HOME/br.out";

my %base_overall_info = ();
my %ext_overall_info = ();
my %ln_sum = ();
my %fn_sum = ();
my %br_sum = ();

sub help() {
    print "parse_cov_results base_summary extended_summary filter_files.txt\n";
}

sub good_file($$) {
    my ($file, $dirs) = @_;

    my $s = dirname($file);
    $s =~ s/^\s+|\s+$//g;

    my $rv = $dirs->{$s};
    if (defined($rv)) {
        return $rv;
    }
    else {
        return 0;
    }
}

# file format:
# filename ln_found ln_hit fn_found fn_hits br_found br_hits
sub read_file($$$$$$) {
    my ($file, $ln_ref, $fn_ref, $br_ref, $overall_ref, $dirs) = @_;
    open INF, "<$file" or die;

    my $total_fns = 0;
    my $total_hit_fns = 0;
    my $total_lns = 0;
    my $total_hit_lns = 0;
    my $total_brs = 0;
    my $total_hit_brs = 0;

    while(my $line = <INF>) {
        chomp $line;
        if ($line =~ m/^#Overall/) {
        }
        elsif ($line =~ m/^#(.*)functions(.*)\(([0-9]*) of ([0-9]*) functions\)/) {
            #$overall_ref->{"functions"} = "$3 $4";
        }
        elsif ($line =~ m/^#(.*)lines(.*)\(([0-9]*) of ([0-9]*) lines\)/) {
            #$overall_ref->{"lines"} = "$3 $4";
        }
        elsif ($line =~ m/^#(.*)branches(.*)\(([0-9]*) of ([0-9]*) branches\)/) {
            #$overall_ref->{"branches"} = "$3 $4";
        }
        else {
            my @a = split(' ', $line);
            if (@a != 7) {
                close INF;
                die "invalid line[$line] in $file";
            }
            if (!good_file($a[0], $dirs)) {
                next;
            }

            $ln_ref->{$a[0]} = [$a[1], $a[2]];
            $total_lns += $a[1];
            $total_hit_lns += $a[2];
            $fn_ref->{$a[0]} = [$a[3], $a[4]];
            $total_fns += $a[3];
            $total_hit_fns += $a[4];
            $br_ref->{$a[0]} = [$a[5], $a[6]];
            $total_brs += $a[5];
            $total_hit_brs += $a[6];
        }
    }
    $overall_ref->{"functions"} = "$total_hit_fns $total_fns";
    $overall_ref->{"lines"} = "$total_hit_lns $total_lns";
    $overall_ref->{"branches"} = "$total_hit_brs $total_brs";

    close INF;
}

sub sort_ln_byinc {
    return $ln_sum{$b} <=> $ln_sum{$a};
}

sub sort_fn_byinc {
    return $fn_sum{$b} <=> $fn_sum{$a};
}

sub sort_br_byinc {
    return $br_sum{$b} <=> $br_sum{$a};
}

sub get_inc($$$) {
    my ($base, $base_found, $ext_found) = @_;

    if ($base == 0) {
        return (0, 0.0);
    }
    else {
        my $diff = $ext_found - $base_found;
        return ($diff, 100 * ($diff / $base));
    }
}

sub gen_summary($$$) {
    my ($sum, $base_info, $ext_info) = @_;

    foreach my $file (keys %$ext_info) {
        (my $ext_found, my $ext_hits) = @{$ext_info->{$file}};
        my $base_found = "X111";
        my $base_hits = "X222";
        my $a = $base_info->{$file};
        my $inc;
        my $inc_per;

        if (!defined($a)) {
            $inc = $ext_hits;
            $inc_per = 100.0;
        }
        else {
            ($base_found, $base_hits) = @$a;
            # warn "[$file]: unmatched found!" if ($base_found != $ext_found);
            warn "[$file]: $ext_found < $base_found!" if ($ext_found < $base_found);
            ($inc, $inc_per) = get_inc($base_found, $base_hits, $ext_hits);
        }
        $sum->{sprintf "$file\t$base_found\t$base_hits\t$ext_hits\t$inc\t%%%.4f\n", $inc_per} = $inc_per;
    }
}

sub dump_overall(*$) {
    local *OUT = $_[0];
    my $flag = $_[1];

    my ($base_overall_hits, $base_overall_found) = split(' ', $base_overall_info{$flag});
    my ($ext_overall_hits, $ext_overall_found) = split(' ', $ext_overall_info{$flag});

    print OUT "  $flag: \n";
    print OUT "    old: ($base_overall_hits of $base_overall_found)\n";
    print OUT "    new: ($ext_overall_hits of $ext_overall_found)\n";
    my ($inc, $inc_per) = get_inc($base_overall_found, $base_overall_hits, $ext_overall_hits);
    my $s = sprintf "increment: %s, %%%.4f", $inc, $inc_per;
    print OUT "    $s\n\n";
}

sub dump_sum($$) {
    my ($out, $flag) = @_;
    local *OUT;

    open OUT, ">$out" or die;
    print OUT "Overall Coverage Rate:\n";
    dump_overall(*OUT, $flag);

    print OUT "\nBreak Down:\n";
    print OUT "file_name\t${flag}_found\told_${flag}_hits\tnew_${flag}_hits\tincrement\tincremental_percentage\n";

    if ($flag eq "lines") {
        foreach (sort sort_ln_byinc keys %ln_sum) {
            print OUT $_;
        }
    }
    elsif ($flag eq "functions") {
        foreach (sort sort_fn_byinc keys %fn_sum) {
            print OUT $_;
        }
    }
    elsif ($flag eq "branches") {
        foreach (sort sort_br_byinc keys %br_sum) {
            print OUT $_;
        }
    }
    else {
        die "Invalid flag[$flag]";
    }
    close OUT;
}

sub get_dirs($$) {
    my ($fname, $dirs) = @_;

    open INF, "<$fname" or die;

    while(my $line = <INF>) {
        chomp $line;
        $line =~ s/^\s+|\s+$//g;
        next if (!length($line));
        $dirs->{$line} = 1;
    }

    close INF;
}

sub analyze($$$) {
    my ($base_file, $extended_file, $dirs_file) = @_;

    my %base_ln_info = ();
    my %ext_ln_info = ();
    my %base_fn_info = ();
    my %ext_fn_info = ();
    my %base_br_info = ();
    my %ext_br_info = ();
    my %dirs = ();

    get_dirs($dirs_file, \%dirs);
    
    read_file($base_file, \%base_ln_info, \%base_fn_info, \%base_br_info, \%base_overall_info, \%dirs);
    read_file($extended_file, \%ext_ln_info, \%ext_fn_info, \%ext_br_info, \%ext_overall_info, \%dirs);

    gen_summary(\%ln_sum, \%base_ln_info, \%ext_ln_info);
    gen_summary(\%fn_sum, \%base_fn_info, \%ext_fn_info);
    gen_summary(\%br_sum, \%base_br_info, \%ext_br_info);
}

###########################################################

die "$COV_DATA_HOME doesn't exist!" if (!(-d $COV_DATA_HOME));

if (@ARGV != 3) {
    help();
    die;
}

analyze($ARGV[0], $ARGV[1], $ARGV[2]);
dump_sum($LN_OUT, "lines");
dump_sum($FN_OUT, "functions");
dump_sum($BR_OUT, "branches");

