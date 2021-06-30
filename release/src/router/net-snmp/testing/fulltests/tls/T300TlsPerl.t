#!/usr/bin/perl

# HEADER Perl TLS/TCP Test

$agentaddress = "tlstcp:localhost:9875";
$feature = "NETSNMP_TRANSPORT_TLSTCP_DOMAIN";

if (!defined($ENV{'TRAVIS_OS_NAME'})) {
    do "$ENV{'srcdir'}/testing/fulltests/tls/S300tlsperl.pl";
} else {
    # Skip this test on Travis CI
    use Test;
    plan(tests => 1);
    ok(1);
}
