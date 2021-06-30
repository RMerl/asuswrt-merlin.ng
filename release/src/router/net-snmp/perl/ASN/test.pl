#!./perl
#
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

use strict;
use warnings;
use Test;

BEGIN {
    $| = 1;
    plan tests => 9;
}

use NetSNMP::ASN (':all');
ok(1);

ok(ASN_INTEGER, 2);
ok(ASN_OCTET_STR, 4);
ok(ASN_COUNTER, 0x41);
ok(ASN_UNSIGNED, 0x42);
ok(ASN_COUNTER64, 0x46);
ok(ASN_IPADDRESS, 0x40);
ok(ASN_NULL, 5);
ok(ASN_TIMETICKS, 0x43);
