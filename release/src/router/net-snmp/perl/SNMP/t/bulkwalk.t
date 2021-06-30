#!./perl
#
# Test bulkwalk functionality.

use strict;
use warnings;
use Cwd qw(abs_path);
use Test;

BEGIN { plan test => ($^O =~ /win32/i) ? 43 : 64; }

use SNMP;

require "t/startagent.pl";
use vars qw($agent_host $agent_port $comm2);

$SNMP::debugging = 0;
$SNMP::verbose = 0;

######################################################################
# Fire up a session.
my $s1 = new SNMP::Session(
    'DestHost'   => $agent_host,
    'Community'  => $comm2,
    'RemotePort' => $agent_port,
    'Version'    => '2c',
    'UseNumeric' => 1,
    'UseEnum'    => 0,
    'UseLongNames' => 1
);
ok(defined($s1));

######################################################################
# 
print("# Attempt to use the bulkwalk method to get a few variables from the SNMP agent.\n");
print("# test 1\n");
my $vars = new SNMP::VarList(['sysUpTime'], ['ifNumber'], # NON-repeaters
			     ['ifSpeed'], ['ifDescr']);	 # Repeated variables.

my $expect = scalar @$vars;
my @list = $s1->bulkwalk(2, 256, $vars);

ok($s1->{ErrorNum} == 0);

# Did we get back the list of references to returned values?
#
ok(scalar @list == $expect);
if (defined($list[0][0])) {
  # Sanity check the returned values.  list[0] is sysUptime nonrepeater.
  ok($list[0][0]->tag, ".1.3.6.1.2.1.1.3");	# check system.sysUptime OID
  ok($list[0][0]->iid, "0");			# check system.sysUptime.0 IID
  ok($list[0][0]->val =~ m/^\d+$/);		# Uptime is numeric 
  ok($list[0][0]->type, "TICKS");		# Uptime should be in ticks.
}
else {
  ok(0);
  ok(0);
  ok(0);
  ok(0);
}
my $ifaces = 0;
if (defined($list[1][0])) {
  # Find out how many interfaces to expect.  list[1] is ifNumber nonrepeater.
  ok($list[1][0]->tag, ".1.3.6.1.2.1.2.1");	# Should be system.ifNumber OID.
  ok($list[1][0]->iid, "0");			# system.ifNumber.0 IID.
  ok($list[1][0]->val =~ m/^\d+$/);		# Number is all numeric 
  #XXX: test fails due SMIv1 codes being returned intstead of SMIv2...
  #ok($list[1][0]->type, "INTEGER32");		# Number should be integer.

  $ifaces = $list[1][0]->val;
}
else {
  ok(0);
  ok(0);
  ok(0);
}

print("# Expecting $ifaces network interfaces.\n");

# Make sure we got an ifSpeed for each interface.  list[2] is ifSpeed repeater.
ok(scalar @{$list[2]} == $ifaces);
# Make sure we got an ifDescr for each interface.  list[3] is ifDescr repeater.
ok(scalar @{$list[3]} == $ifaces);

if (defined($list[2][0])) {
  # Test for reasonable values from the agent.
  ok($list[2][0]->tag, ".1.3.6.1.2.1.2.2.1.5");	# Should be system.ifSpeed OID.
  ok($list[2][0]->iid, "1");			# Instance should be 1.
  ok($list[2][0]->val =~ m/^\d+$/);		# Number is all numeric 
  ok($list[2][0]->type, "GAUGE");		# Number should be a gauge.
}
else {
  ok(0);
  ok(0);
  ok(0);
  ok(0);
}

print("# Looking up loopback network interface ...\n");
ok(ref($list[3]), 'SNMP::VarList');
my $found;
for my $ifdescr (@{$list[3]}) {
  ok(ref($ifdescr), 'SNMP::Varbind');
  print("# " . $ifdescr->val . "\n");
  next if (!($ifdescr->val =~ /Software Loopback Interface/) and
	   !($ifdescr->val =~ /^lo/));
  $found = $ifdescr->val;
  ok(1);
  ok($ifdescr->tag, ".1.3.6.1.2.1.2.2.1.2");	# Should be system.ifDescr OID.
  ok($ifdescr->iid =~ m/^\d+$/);		# Instance should be 1.
  ok($ifdescr->type, "OCTETSTR");		# Description is a string.
  last;
}
if (!$found) {
  ok(0);
  ok(0);
  ok(0);
  ok(0);
  ok(0);
}
  
###############################################################################
print("# Attempt to use the bulkwalk method to get only non-repeaters.\n");
print("# test 2\n");
$vars = new SNMP::VarList ( ['sysUpTime'], ['ifNumber'] ); # NON-repeaters

$expect = scalar @$vars;
@list = $s1->bulkwalk(2, 0, $vars);
ok($s1->{ErrorNum} == 0);

# Did we get back the list of references to returned values?
#
ok(scalar @list == $expect);

if (defined($list[0][0])) {
  # Sanity check the returned values.  list[0] is sysUptime nonrepeater.
  ok($list[0][0]->tag, ".1.3.6.1.2.1.1.3");	# check system.sysUptime OID
  ok($list[0][0]->iid, "0");			# check system.sysUptime.0 IID
  ok($list[0][0]->val =~ m/^\d+$/);		# Uptime is numeric 
  ok($list[0][0]->type, "TICKS");		# Uptime should be in ticks.
}
else {
  ok(0);
  ok(0);
  ok(0);
  ok(0);
}

if (defined($list[1][0])) {
  # Find out how many interfaces to expect.  list[1] is ifNumber nonrepeater.
  ok($list[1][0]->tag, ".1.3.6.1.2.1.2.1");	# Should be system.ifNumber OID.
  ok($list[1][0]->iid, "0");			# system.ifNumber.0 IID.
  ok($list[1][0]->val =~ m/^\d+$/);		# Number is all numeric 
  #XXX: test fails due SMIv1 codes being returned intstead of SMIv2...
  #ok($list[1][0]->type, "INTEGER32");		# Number should be integer.
  $ifaces = $list[1][0]->val;
}
else {
  ok(0);
  ok(0);
  ok(0);
}

###############################################################################
print("# Attempt to use the bulkwalk method to get only repeated variables\n");
print("# test 3\n");
$vars = new SNMP::VarList ( ['ifIndex'], ['ifSpeed'] ); # repeaters

$expect = scalar @$vars;
@list = $s1->bulkwalk(0, 256, $vars);
ok($s1->{ErrorNum} == 0);

# Did we get back the list of references to returned values?
#
ok(scalar @list == $expect);

# Make sure we got an ifIndex for each interface.  list[0] is ifIndex repeater.
ok(scalar @{$list[0]} == $ifaces);

# Make sure we got an ifSpeed for each interface.  list[0] is ifSpeed repeater.
ok(scalar @{$list[1]} == $ifaces);

if (defined($list[0][0])) {
  # Test for reasonable values from the agent.
  ok($list[0][0]->tag, ".1.3.6.1.2.1.2.2.1.1");	# Should be system.ifIndex OID.
  ok($list[0][0]->iid, "1");			# Instance should be 1.
  ok($list[0][0]->val =~ m/^\d+$/);		# Number is all numeric 
  #XXX: test fails due SMIv1 codes being returned intstead of SMIv2...
  #ok($list[0][0]->type, "INTEGER32");		# Number should be an integer.
}
else {
  ok(0);
  ok(0);
  ok(0);
}

if (defined($list[1][0])) {
  ok($list[1][0]->tag, ".1.3.6.1.2.1.2.2.1.5");	# Should be system.ifSpeed OID.
  ok($list[1][0]->iid, "1");			# Instance should be 1.
  ok($list[1][0]->val =~ m/^\d+$/);		# Number is all numeric 
  ok($list[1][0]->type, "GAUGE");		# Number should be a gauge.
}
else {
  ok(0);
  ok(0);
  ok(0);
  ok(0);
}

######################################################################
#  Asynchronous Bulkwalk Methods
######################################################################
# 
print("# Attempt to use the bulkwalk method to get a few variables from the SNMP agent.\n");
print("# test 4\n");
sub async_cb1 {
    my ($vars, $list) = @_;
    ok(defined $list && ref($list) =~ m/ARRAY/);
    ok(defined $vars && ref($vars) =~ m/SNMP::VarList/);

    ok(scalar @$list == scalar @$vars);

    my $vbr;

    if (defined($list->[0][0])) {
      # Sanity check the returned values.  First is sysUptime nonrepeater.
      $vbr = $list->[0][0];
      ok($vbr->tag, ".1.3.6.1.2.1.1.3");	# check system.sysUptime OID
      ok($vbr->iid, "0");			# check system.sysUptime.0 IID
      ok($vbr->val =~ m/^\d+$/);			# Uptime is numeric 
      ok($vbr->type, "TICKS");			# Uptime should be in ticks.
    }
    else {
      ok(0);
      ok(0);
      ok(0);
      ok(0);
    }

    if (defined($list->[1][0])) {
      # Find out how many interfaces to expect.  Next is ifNumber nonrepeater.
      $vbr = $list->[1][0];
      ok($vbr->tag, ".1.3.6.1.2.1.2.1");	# Should be system.ifNumber OID.
      ok($vbr->iid, "0");			# system.ifNumber.0 IID.
      ok($vbr->val =~ m/^\d+$/);			# Number is all numeric 
      #XXX: test fails due SMIv1 codes being returned intstead of SMIv2...
      #    ok($vbr->type, "INTEGER32");		# Number should be integer.
      $ifaces = $vbr->[2];
    }
    else {
      ok(0);
      ok(0);
      ok(0);
    }

    # Test for reasonable values from the agent.
    ok(scalar @{$list->[2]} == $ifaces);
    
    if (defined($list->[2][0])) {
      $vbr = $list->[2][0];
      ok($vbr->tag, ".1.3.6.1.2.1.2.2.1.5");	# Should be ifSpeed OID
      ok($vbr->iid, "1");			# Instance should be 1.
      ok($vbr->val =~ m/^\d+$/);			# Number is all numeric 
      ok($vbr->type, "GAUGE");			# Should be a gauge.

      ok(scalar @{$list->[3]} == $ifaces);
    }
    else {
      ok(0);
      ok(0);
      ok(0);
      ok(0);
      ok(0);
    }

    for my $ifdescr (@{$list->[3]}) {
      next if (!($ifdescr->val =~ /Software Loopback Interface/) and
	       !($ifdescr->val =~ /^lo/));
      ok(1);
      # Should be system.ifDescr OID.
      ok($ifdescr->tag, ".1.3.6.1.2.1.2.2.1.2");
      ok($ifdescr->iid >= 1);			# Instance should be >= 1.
      ok($ifdescr->type, "OCTETSTR");		# Description is a string.
      last;
    }
    if (!defined($list->[3][0])) {
      ok(0);
      ok(0);
      ok(0);
      ok(0);
    }

    SNMP::finish();
}

$vars = new SNMP::VarList ( ['sysUpTime'], ['ifNumber'], # NON-repeaters
			    ['ifSpeed'], ['ifDescr']);	 # Repeated variables.

if ($^O =~ /win32/i) {
  warn "Win32/Win64 detected - skipping async calls\n";
}
else {
  @list = $s1->bulkwalk(2, 256, $vars, [ \&async_cb1, $vars ] );
  ok($s1->{ErrorNum} == 0);
  SNMP::MainLoop();
}
ok(1);

snmptest_cleanup();

