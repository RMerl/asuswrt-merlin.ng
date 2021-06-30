#!/usr/bin/perl

# HEADER Perl UDP IPv6 Test

use strict;
use warnings;
use NetSNMPTestTransport;

my $test = new NetSNMPTestTransport(agentaddress => "udp6:[::1]:9875");
$test->require_feature("NETSNMP_TRANSPORT_UDPIPV6_DOMAIN");
$test->run_tests();
