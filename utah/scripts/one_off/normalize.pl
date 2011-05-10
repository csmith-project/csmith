#!/usr/bin/perl -w

use strict;

my $f = $ARGV[0];
die if (!(-f $f));

my $s="";

open INF, "<$f" or die;
while (<INF>) {
    $s .= $_;
}
close INF;
system "mv $f ${f}.bak";

$s =~ s/safe_add_func_int8_t_s_s/foo/g;
$s =~ s/safe_add_func_uint8_t_u_u/bux/g;
$s =~ s/safe_rshift_func_int8_t_s_s/bar/g;
$s =~ s/safe_mod_func_int64_t_s_s/baz/g;
$s =~ s/safe_lshift_func_uint16_t_u_s/biz/g;

$s =~ s/typedef signed char int8_t;//;
$s =~ s/typedef unsigned char uint8_t;//;
$s =~ s/typedef short int int16_t;//;
$s =~ s/typedef unsigned short int uint16_t;//;
$s =~ s/typedef int int32_t;//;
$s =~ s/typedef unsigned int uint32_t;//;
$s =~ s/__extension__ typedef long long int int64_t;//;
$s =~ s/__extension__ typedef unsigned long long int uint64_t;//;

$s =~ s/uint8_t/unsigned char/g;
$s =~ s/uint16_t/unsigned short/g;
$s =~ s/uint32_t/unsigned/g;
$s =~ s/uint64_t/unsigned long long/g;

$s =~ s/int8_t/signed char/g;
$s =~ s/int16_t/short/g;
$s =~ s/int32_t/int/g;
$s =~ s/int64_t/long long/g;

open OUTF, ">$f" or die;
print OUTF $s;
close OUTF;
