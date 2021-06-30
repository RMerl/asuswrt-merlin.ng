#!/usr/bin/perl

# HEADER Perl Unix Domain Socket Test

use strict;
use warnings;
use NetSNMPTestTransport;

my $test = new NetSNMPTestTransport(agentaddress => "bogus");
$test->require_feature("NETSNMP_TRANSPORT_UNIX_DOMAIN");
$test->{'agentaddress'} = "unix:" . $test->{'dir'} . "/unixtestsocket";
$test->run_tests();
