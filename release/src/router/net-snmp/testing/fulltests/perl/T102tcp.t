#!/usr/bin/perl

# HEADER Perl TCP IPv4 Test

use strict;
use warnings;
use NetSNMPTestTransport;

my $test = new NetSNMPTestTransport(agentaddress => "tcp:localhost:9875");
$test->require_feature("NETSNMP_TRANSPORT_TCP_DOMAIN");
$test->run_tests();
