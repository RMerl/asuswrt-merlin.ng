#!./perl

# Written by John Stoffel (jfs@fluent.com) - 10/13/1997

use strict;
use warnings;

BEGIN {
    eval "use Cwd qw(abs_path)";
    $ENV{'MIBS'} = 'ALL';
}

# to print the description...
$SNMP::save_descriptions = 1;

use Test;
BEGIN {plan tests => 38}
use SNMP;
use Data::Dumper;

$SNMP::verbose = 0;
$SNMP::best_guess = 2;

require "t/startagent.pl";
use vars qw($bad_name $bad_oid $name $name_long $name_module $name_module2
            $name_module_long $name_module_long2 $oid);
require "t/startagent.pl";

my $DEBUG;

#############################  1  ######################################
#check if
my $res = $SNMP::MIB{sysDescr}{label};
print STDERR ("Test 1: label is $res\n") if ($DEBUG);
ok("sysDescr" eq $res);
print STDERR ("\n") if ($DEBUG);
#############################  2  ######################################
$res =  $SNMP::MIB{sysDescr}{objectID};
print STDERR ("Test 2: OID is $res\n") if ($DEBUG);
ok(defined($res));
print STDERR ("\n") if ($DEBUG);
#############################  3  ######################################
$res =  $SNMP::MIB{sysDescr}{access};
print STDERR ("Test 3: access is $res\n") if ($DEBUG);
ok($res eq 'ReadOnly');
print STDERR ("\n") if ($DEBUG);
##############################  4  ###################################
$res =  $SNMP::MIB{sysLocation}{access};
#$res =  $SNMP::MIB{sysORIndex}{access};
ok($res eq 'ReadWrite');
##############################  5  ###################################
$res =  $SNMP::MIB{sysLocation}{type};
ok($res eq 'OCTETSTR');
#############################  6  ####################################
$res =  $SNMP::MIB{sysLocation}{status};
print STDERR ("Test 6: status is $res\n") if ($DEBUG);
ok($res eq 'Current');
print STDERR ("\n") if ($DEBUG);
#############################  7  #################################
$res =  $SNMP::MIB{sysORTable}{access};
print STDERR ("Test 7: access is $res\n") if ($DEBUG);
ok($res eq 'NoAccess');
print STDERR ("\n") if ($DEBUG);
#############################  8  ###############################
$res = $SNMP::MIB{sysLocation}{subID};
print STDERR ("Test 8: subID is $res\n") if ($DEBUG);
ok(defined($res));
print STDERR ("\n") if ($DEBUG);
############################  9  ##############################
$res = $SNMP::MIB{sysLocation}{syntax};
print STDERR ("Test 9: syntax is $res\n") if ($DEBUG);
ok($res eq 'DisplayString');
print STDERR ("\n") if ($DEBUG);
############################  10  ###########################
$res = $SNMP::MIB{ipAdEntAddr}{syntax};
print STDERR ("Test 10: syntax is $res\n") if ($DEBUG);
ok($res eq 'IPADDR');
print STDERR ("\n") if ($DEBUG);
##########################  11  ##########################
$res = $SNMP::MIB{atNetAddress}{syntax};
print STDERR ("Test 11: syntax is $res\n") if ($DEBUG);
ok($res eq 'NETADDR');
print STDERR ("\n") if ($DEBUG);
########################   12  ###############################
$res = $SNMP::MIB{ipReasmOKs}{syntax};
print STDERR ("Test 12: syntax is $res\n") if ($DEBUG);
ok($res eq 'COUNTER');
print STDERR ("\n") if ($DEBUG);
######################   13  ##############################
$res = $SNMP::MIB{sysDescr}{moduleID};
print STDERR ("Test 13: module ID is $res\n") if ($DEBUG);
ok(defined($res));
print STDERR ("\n") if ($DEBUG);
######################  14   #########################
my $des = $SNMP::MIB{atNetAddress}{description};
print STDERR ("Test 14: des is $des\n") if ($DEBUG);
ok(defined($des));
print STDERR ("\n") if ($DEBUG);

######################  15   #########################
$res = $SNMP::MIB{atNetAddress}{nextNode};
print STDERR ("Test 15: res is $res\n") if ($DEBUG);
ok(ref($res) eq "HASH");
print STDERR ("\n") if ($DEBUG);

########################  16   #########################
$res = $SNMP::MIB{sysDescr}{children};
print STDERR ("Test 16: res is " . Dumper($res) . "\n") if ($DEBUG);
ok(ref($res) eq "ARRAY");
print STDERR ("\n") if ($DEBUG);
####################  17   #########################

$res = $SNMP::MIB{sysDescr}{badField};
ok(!defined($res));


######################  18   #########################
$res = $SNMP::MIB{sysDescr}{hint};
print STDERR ("Test 18: res is " . Dumper($res) . "\n") if ($DEBUG);
ok(defined($res) && $res =~ /^255a/);
print STDERR ("\n") if ($DEBUG);
######################  19   #########################

$res = $SNMP::MIB{ifPhysAddress}{hint};
print STDERR ("Test 19: res is " . Dumper($res) . "\n") if ($DEBUG);
ok(defined($res) && $res =~ /^1x:/);
print STDERR ("\n") if ($DEBUG);


######################  some translate tests  #######

#####################  20  #########################
# Garbage names return Undef.

my $type1 = SNMP::getType($bad_name);
ok(!defined($type1));
#printf "%s %d\n", (!defined($type1)) ? "ok" :"not ok", $n++;

######################################################################
# getType() supports numeric OIDs now

my $type2 = SNMP::getType($oid);
ok(defined($type2) && $type2 =~ /OCTETSTR/);

######################################################################
# This tests that sysDescr returns a valid type.

my $type3 = SNMP::getType($name);
ok(defined($type3));

######################################################################
# Translation tests from Name -> OID
# sysDescr to .1.3.6.1.2.1.1.1
my $oid_tag = SNMP::translateObj($name);
ok($oid eq $oid_tag);

######################################################################
# Translation tests from Name -> OID
# RFC1213-MIB::sysDescr to .1.3.6.1.2.1.1.1
$oid_tag = SNMP::translateObj($name_module);
ok($oid eq $oid_tag);

######################################################################
# Translation tests from Name -> OID
# .iso.org.dod.internet.mgmt.mib-2.system.sysDescr to .1.3.6.1.2.1.1.1
$oid_tag = SNMP::translateObj($name_long);
ok($oid eq $oid_tag);

######################################################################
# bad name returns 'undef'
$oid_tag = '';
$oid_tag = SNMP::translateObj($bad_name);
ok(!defined($oid_tag));


######################################################################
# OID -> name
# .1.3.6.1.2.1.1.1 to sysDescr
my $name_tag = SNMP::translateObj($oid);
ok($name eq $name_tag);

######################################################################
# OID -> name
# .1.3.6.1.2.1.1.1 to RFC1213-MIB::sysDescr or
# .1.3.6.1.2.1.1.1 to SNMPv2-MIB::sysDescr
$name_tag = SNMP::translateObj($oid,0,1);
ok(($name_module eq $name_tag) || ($name_module2 eq $name_tag));

######################################################################
# OID -> name
# .1.3.6.1.2.1.1.1 to .iso.org.dod.internet.mgmt.mib-2.system.sysDescr
$name_tag = SNMP::translateObj($oid,1);
ok($name_long eq $name_tag);

######################################################################
# OID -> name
# .1.3.6.1.2.1.1.1 to RFC1213-MIB::.iso.org.dod.internet.mgmt.mib-2.system.sysDescr or
# .1.3.6.1.2.1.1.1 to SNMPv2-MIB::.iso.org.dod.internet.mgmt.mib-2.system.sysDescr
$name_tag = SNMP::translateObj($oid,1,1);
ok(($name_module_long eq $name_tag) || ($name_module_long2 eq $name_tag));

######################################################################
# bad OID -> Name

$name_tag = SNMP::translateObj($bad_oid);
ok($name ne $name_tag);
#printf "%s %d\n", ($name ne $name_tag) ? "ok" :"not ok", $n++;


######################################################################
# ranges

my $node = $SNMP::MIB{snmpTargetAddrMMS};
ok($node);
my $ranges = $node->{ranges};
ok($ranges and ref $ranges eq 'ARRAY');
ok(@$ranges == 2);
ok($$ranges[0]{low} == 0);
ok($$ranges[0]{high} == 0);
ok($$ranges[1]{low} == 484);
ok($$ranges[1]{high} == 2147483647);

snmptest_cleanup();
