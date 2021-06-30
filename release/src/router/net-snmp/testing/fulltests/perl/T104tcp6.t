#!/usr/bin/perl

# HEADER Perl TCP IPv6 Test

use strict;
use warnings;
use NetSNMPTestTransport;

my $test = new NetSNMPTestTransport(agentaddress => "tcp6:[::1]:9875");
$test->require_feature("NETSNMP_TRANSPORT_TCPIPV6_DOMAIN");
$test->run_tests();
