#!/usr/bin/perl

use strict;
use File::Spec::Functions qw(splitpath);
use Sys::CPU;

########################################################

my $NICE = "nice -10";

my $CPUS = Sys::CPU::cpu_count();    

my $COV_BASE_HOME = $ENV{"COV_BASE_HOME"};

########################################################

#die "please set env $COV_BASE_HOME first!"
#    if (!defined($COV_BASE_HOME));

# for my convenience
if (!defined($COV_BASE_HOME)) {
    $COV_BASE_HOME = "/uusoc/exports/scratch/chenyang/tmp/branch_test";
}

my $COMPILER_SOURCES_HOME = "$COV_BASE_HOME/sources";

my @compilers = (
    "gcc",
    "llvm",
    "llvm-regular",
    "llvm-gcc",
);

my %paths = (
    "gcc" => ["$COMPILER_SOURCES_HOME", "current-gcc"],
    "llvm" => ["$COMPILER_SOURCES_HOME", "llvm"],
    "llvm-regular" => ["$COMPILER_SOURCES_HOME", "llvm-regular"],
    "llvm-gcc" => ["$COMPILER_SOURCES_HOME", "llvm-gcc-4.2"],
    "clang" => ["$COMPILER_SOURCES_HOME/llvm/tools", "clang"],
    "llvm-testsuite" => ["$COMPILER_SOURCES_HOME/llvm/projects", "test-suite"],
);

my %svn_dependencies = (
    "llvm" => "clang",
);

my %svn_repos = (
    "gcc" => "svn://gcc.gnu.org/svn/gcc/trunk",
    "llvm" => "http://llvm.org/svn/llvm-project/llvm/trunk",
    "llvm-regular" => "http://llvm.org/svn/llvm-project/llvm/trunk",
    "llvm-gcc" => "http://llvm.org/svn/llvm-project/llvm-gcc-4.2/trunk",
    "clang" => "http://llvm.org/svn/llvm-project/cfe/trunk",
    "llvm-testsuite" => "http://llvm.org/svn/llvm-project/test-suite/trunk",
);

my %dependencies = (
    "gcc" => ["gmp", "mpc", "mpfr", "libelf"],
);

my %programs = (
    "gmp" => "ftp://ftp.gmplib.org/pub/gmp-5.0.1/gmp-5.0.1.tar.bz2",
    "mpfr" => "http://www.mpfr.org/mpfr-current/mpfr-3.0.0.tar.gz",
    "mpc" => "http://www.multiprecision.org/mpc/download/mpc-0.8.2.tar.gz",
    "libelf" => "http://www.mr511.de/software/libelf-0.8.13.tar.gz",
);

my $enable_optimized = "";

my %config_options = (
    "gcc" => "--prefix=$COV_BASE_HOME --program-prefix=current- --enable-languages=c,c++ --enable-lto --enable-coverage --disable-bootstrap",
    "gcc-opt" => "--prefix=$COV_BASE_HOME --program-prefix=current- --enable-languages=c,c++ --enable-lto --enable-coverage=opt --disable-bootstrap",
    "llvm" => "--prefix=$COV_BASE_HOME",
    "llvm-opt" => "--prefix=$COV_BASE_HOME --enable-optimized",
    "llvm-regular" => "--prefix=$COV_BASE_HOME --enable-optimized",
    "llvm-gcc" => "--prefix=$COV_BASE_HOME --enable-languages=c,c++ --program-prefix=llvm- --enable-checking --enable-llvm=$COMPILER_SOURCES_HOME/llvm-regular/build --disable-bootstrap --disable-multilib",
);

my %make_options = (
    "gcc" => "",
    "llvm" => "ENABLE_COVERAGE=1",
    "llvm-regular" => "",
    "llvm-gcc" => "",
);

sub runit ($) {
    my $cmd = shift;
    print "$cmd\n";
    if ((system "$NICE $cmd") != 0) {
        return -1;
    }
    my $exit_value  = $? >> 8;
    return $exit_value;
}

sub parse_path($) {
    my ($path) = @_;
    return (@$path[0], @$path[1]);
}

sub download($$$$);
sub download($$$$) {
    my ($program, $path, $dir, $svn_repo) = @_;
    my $prog_path = "$path/$dir";

    if (-d "$prog_path") {
        print "changing to $prog_path\n";
        chdir $prog_path or die;
        my $cmd = "svn update .";
        my $res = runit($cmd);
        if ($res == 0) {
            return;
        }
        else {
            system "rm -rf $prog_path";
        }
    }

    print "changing to $path\n";
    chdir $path or die;
    my $cmd = "svn co $svn_repo $dir";
    my $res = runit($cmd);
    die "can't check out $program from $svn_repo to $path/$dir"
        if ($res != 0);

    my $extra_program = $svn_dependencies{$program};
    return if (!defined($extra_program));
    my ($extra_path, $extra_dir) = parse_path($paths{$extra_program});
    my $extra_svn_repo = $svn_repos{$extra_program};
    download($extra_program, $extra_path, $extra_dir, $extra_svn_repo);
}

sub download_dependencies($$) {
    my ($program, $path) = @_;
    my $extra_programs = $dependencies{$program};

    return if (!defined($extra_programs));

    print "chaning to $path\n";
    chdir $path or die;
    foreach my $extra_program (@$extra_programs) {
        my $orig_program_path = $programs{$extra_program};
        my ($dummy1, $dummy2, $orig_program) = splitpath($orig_program_path);

        my ($cmd, $res);
        if (!(-f $orig_program)) {
            $cmd = "wget $orig_program_path";
            $res = runit($cmd);
            die "can't wget $orig_program_path" if ($res != 0);
            die "can't download $orig_program_path"
                if (!(-f $orig_program));
        }

        my $uncompressed_name;
        if ($orig_program =~ m/(.+)\.tar$/) {
            $cmd = "tar xvf $orig_program";
            $uncompressed_name = $1;
        }
        elsif ($orig_program =~ m/(.+)\.tar\.gz$/) {
            $cmd = "tar zxvf $orig_program";
            $uncompressed_name = $1;
        }
        elsif ($orig_program =~ m/(.+)\.tar.bz2$/) {
            $cmd = "bzip2 -d -c $orig_program | tar xvf -";
            $uncompressed_name = $1;
        }
        else {
            die "unsupported file format: $orig_program";
        }

        $res = runit($cmd);
        die "can't uncompress $orig_program" if ($res != 0);
        die "$uncompressed_name doesn't exist!"
            if (!(-d $uncompressed_name));
        system "rm -rf $extra_program";
        system "mv $uncompressed_name $extra_program";
    }
}


sub download_all() {
    foreach my $program (@compilers) {
        my ($path, $dir) = parse_path($paths{$program});
        my $svn_repo = $svn_repos{$program};

        download($program, $path, $dir, $svn_repo);
        download_dependencies($program, "$path/$dir");
    }
}

sub configure($$) {
    my ($compiler, $build_path) = @_;
    
    if (($compiler eq "llvm") || ($compiler eq "gcc")) {
        $compiler .= $enable_optimized;
    }
    my $config_option = $config_options{$compiler};

    die if (!defined $config_option);
    system "rm -rf $build_path";
    mkdir "$build_path" or die;

    print "chaning to $build_path\n";
    chdir "$build_path" or die;

    #my $cmd = "../configure $config_option > /dev/null 2>&1";
    my $cmd = "../configure $config_option";
    my $res = runit($cmd);
    die "can't configure $compiler!" if ($res != 0);
}

sub make($$) {
    my ($compiler, $build_path) = @_;
    my $make_option = $make_options{$compiler};

    print "chaning to $build_path\n";
    chdir $build_path or die;

    #my $cmd = "make $make_option > /dev/null 2>&1";
    my $cmd = "make -j $CPUS $make_option";
    my $res = runit($cmd);
    die "can't make $compiler!" if ($res != 0);

    #$cmd = "make $make_option install > /dev/null 2>&1";
    if ($compiler ne "llvm-regular") {
        $cmd = "make -j $CPUS $make_option install";
        $res = runit($cmd);
    }
    #die "can't install $compiler!" if ($res != 0);
}

sub verify($$) {
    my ($compiler, $build_path) = @_;

    print "changing to $build_path\n";
    chdir $build_path or die;
    my $files = "*.gcno";
    my @gcnos = `find . -name $files`;
    die "*.gcno doesn't exist!" if (@gcnos == 0);
}

sub prepare_testsuites() {
    my $program = "llvm-testsuite";
    my ($path, $dir) = parse_path($paths{$program});
    my $svn_repo = $svn_repos{$program};

    download($program, $path, $dir, $svn_repo);

    my $working_dir = "$COMPILER_SOURCES_HOME/llvm/build";
    print "chaning to $working_dir\n";
    chdir $working_dir or die;

    my $cp_cmd;

    if ($enable_optimized eq '') {
        $cp_cmd = 'cp -r Debug+Coverage+Asserts Debug+Asserts';
    }
    else {
        $cp_cmd = 'cp -r Release+Coverage+Asserts Release+Asserts';
    }

    my $res = runit($cp_cmd);
    die "Can't exec $cp_cmd" if ($res != 0);

    $ENV{"PATH"} = "$COV_BASE_HOME/bin:$ENV{'PATH'}";
    my $opt = "";
    $opt = "--enable-optimized" if (not ($enable_optimized eq ""));
    $res = runit("../configure $opt");
    die "Can't configure llvm-testsuite!" if ($res != 0);
}

sub go_build() {
    foreach (@compilers) {
        my ($path, $dir) = parse_path($paths{$_});
        my $build_path = "$path/$dir/build";
        configure($_, $build_path);
        make($_, $build_path);
        if (($_ eq "llvm") || ($_ eq "gcc")) {
            verify($_, $build_path);
        }
        if ($_ eq "llvm") {
            prepare_testsuites();
        }
    }
}

sub help() {
    print "build_gcov_compiler.pl [-no-update] [-enable-opt]\n";
}

sub main() {
    my $opt;
    my @unused = ();
    my $update = 1;
    while(defined ($opt = shift @ARGV)) {
        if ($opt eq "-no-update") {
            $update = 0;
        }
        elsif ($opt eq "-enable-opt") {
            $enable_optimized = "-opt";
        }
        else {
            help();
            die;
        }
    }
   
    if ($update) {
        download_all();
    }
    go_build();
}

if (!(-d $COV_BASE_HOME)) {
    mkdir $COV_BASE_HOME or die;
}
if (!(-d "$COMPILER_SOURCES_HOME")) {
    mkdir "$COMPILER_SOURCES_HOME" or die;
}

main();

