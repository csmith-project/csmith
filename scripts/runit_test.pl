#!/usr/bin/perl -w

## No license, we won't release this file.

###############################################################################

use strict; 
use File::stat;
use File::Temp;
use Cwd;

# 0 : success
# 1 : timeout
# 2 : ...
sub runit ($$$) {
    my ($command, $timeout, $out) = @_;

    print "\n\ntimeout is $timeout\n";
    
    my $res;
    eval {
        local $SIG{ALRM} = sub { die "alarm\n" }; # NB: \n required
        alarm $timeout;
        system "$command";
        alarm 0;
	$res = $?;
    };

    my $success = 0; 
    my $exit_value;

    if ($@ eq "alarm\n") {
	print "'$command' timed out after $timeout second(s)\n";
	$exit_value = -1;
	goto out;
    }

    if ($res & 127) {
        print "died while executing '$command'\n";
	goto out;
    }
    
    if ($res == -1) {
        print "can't execute '$command'\n";
	goto out;
    }

    $success = 1;
    $exit_value = $res >> 8;

  out:
    print "command '$command' has success = $success, exit value = $exit_value\n";
    return ($success, $exit_value);
}

if (1) {
    while (1) {
	runit ("./nonterm.pl", 1+int(rand(3)), "");
    }
} else {
    runit ("sleep 5", 1, "/dev/null");
    runit ("sleep 1", 5, "/dev/null");
    runit ("gleep 1", 5, "/dev/null");
}
