#!./perl
#
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl 1.t'

#########################

use strict;
use warnings;
use Test;

BEGIN {
    plan tests => 8
}

use NetSNMP::TrapReceiver;
ok(1); # If we made it this far, we're ok.  Bogus test!


my $fail;
foreach my $constname (qw(
	NETSNMPTRAPD_AUTH_HANDLER NETSNMPTRAPD_HANDLER_BREAK
	NETSNMPTRAPD_HANDLER_FAIL NETSNMPTRAPD_HANDLER_FINISH
	NETSNMPTRAPD_HANDLER_OK NETSNMPTRAPD_POST_HANDLER
	NETSNMPTRAPD_PRE_HANDLER)) {
    if (eval "my \$a = $constname; 1") {
	ok(1);
	next;
    }
    if ($@ =~ /^Your vendor has not defined NetSNMP::TrapReceiver macro $constname/) {
	print "# pass: $@";
	ok(1);
    } else {
	print "# fail: $@";
	ok(0);
    }
}

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

