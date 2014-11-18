#!/usr/bin/perl -w

use strict;

my $SOUPER = $ENV{"SOUPER"};
die unless defined $SOUPER;
my $SPATH = "${SOUPER}/third_party/llvm/Release/bin";

my $passes1 = "-sroa";
my $passes2 = "$passes1 -sccp -inline";
my $passes3 = "$passes2 -adce";

my $sflags = "-load ${SOUPER}/build/libsouperPass.so -souper -external-cache-souper -z3-path=/usr/bin/z3 -solver-timeout=60 -solver-use-quantifiers";

sub runit ($) {
    my $cmd = shift;
    print "<$cmd>\n";
    my $res = (system "$cmd");
    my $exit_value  = $? >> 8;
    return $exit_value;
}

my $cfile;
my $exe;
for (my $i=0; $i<scalar(@ARGV); $i++) {
    my $a = $ARGV[$i];
    if ($a =~ /\.c$/) {
        die if defined $cfile;
        $cfile = $a;
    }
    if ($a eq "-o") {
        die if defined $exe;
        $exe = $ARGV[$i+1];
    }
}
die unless defined $cfile;
# die unless defined $exe;

my $root = $cfile;
die unless ($root =~ s/\.c$//);

runit("$SPATH/clang -w -c -emit-llvm -I/home/regehr/csmith/runtime -O0 $cfile")
    == 0 or die();
runit("$SPATH/opt $passes1 ${root}.bc  -o ${root}2.bc") == 0 or die();
runit("$SPATH/opt $sflags  ${root}2.bc -o ${root}3.bc") == 0 or die();
runit("$SPATH/opt $passes2 ${root}3.bc -o ${root}4.bc") == 0 or die();
runit("$SPATH/opt $sflags  ${root}4.bc -o ${root}5.bc") == 0 or die();
runit("$SPATH/opt $passes3 ${root}5.bc -o ${root}6.bc") == 0 or die();
runit("$SPATH/opt $sflags  ${root}6.bc -o ${root}7.bc") == 0 or die();
runit("$SPATH/opt $passes3 ${root}7.bc -o ${root}8.bc") == 0 or die();
if (defined $exe) {
    runit("$SPATH/clang ${root}8.bc -o $exe");
}
