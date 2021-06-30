#!./perl

use strict;
use warnings;
use Exporter;

#Open the snmptest.cmd file and get the info
require "t/readsnmptest.pl";
use vars qw($agent_host $agent_port $mibdir $snmpd_cmd $snmptrapd_cmd);

our @ISA = 'Exporter';
our @EXPORT_OK = qw($agent_host $agent_port $mibdir
$trap_port $comm $comm2 $comm3 $sec_name $oid $name
$name_module $name_module2 $name_long $name_module_long $name_module_long2
$auth_pass $priv_pass $bad_comm $bad_name $bad_oid $bad_port $bad_host
$bad_auth_pass $bad_priv_pass $bad_sec_name $bad_version);

# common parameters used in SNMP::Session creation and tests
$agent_host = 'localhost' if (!defined($agent_host));
$agent_port = 8765 if (!defined($agent_port));
our $trap_port = 8764;
$mibdir = '/usr/local/share/snmp/mibs' if (!defined($mibdir));
our $comm = 'v1_private';
our $comm2 = 'v2c_private';
our $comm3 = 'v3_private';
our $sec_name = 'v3_user';
our $oid = '.1.3.6.1.2.1.1.1';
our $name = 'sysDescr';
our $name_module = 'RFC1213-MIB::sysDescr';
our $name_module2 = 'SNMPv2-MIB::sysDescr';
our $name_long = '.iso.org.dod.internet.mgmt.mib-2.system.sysDescr';
our $name_module_long = 'RFC1213-MIB::.iso.org.dod.internet.mgmt.mib-2.system.sysDescr';
our $name_module_long2 = 'SNMPv2-MIB::.iso.org.dod.internet.mgmt.mib-2.system.sysDescr';
our $auth_pass = 'test_pass_auth';
our $priv_pass = 'test_pass_priv';

# erroneous input to test failure cases
our $bad_comm = 'BAD_COMMUNITY';
our $bad_name = "badName";
our $bad_oid = ".1.3.6.1.2.1.1.1.1.1.1";
our $bad_host = 'bad.host.here';
our $bad_port = '9999999';
our $bad_auth_pass = 'bad_auth_pass';
our $bad_priv_pass = 'bad_priv_pass';
our $bad_sec_name = 'bad_sec_name';
our $bad_version = 7;

if ($^O =~ /win32/i) {
  require Win32::Process;
}

# Variant of sleep that accepts a floating point number as argument.
sub delay {
  my ($timeout) = @_;

  select(undef, undef, undef, $timeout);
}

sub run_async {
  my ($pidfile, $cmd, @args) = @_;
  if (-r "$cmd" and -x "$cmd") {
    if ($^O =~ /win32/i) {
      $cmd =~ s/\//\\/g;
      system "start \"$cmd\" /min cmd /c \"$cmd @args 2>&1\"";
    } else {
      system "$cmd @args 2>&1";
    }
    # Wait at most three seconds for the pid file to appear.
    for (my $i = 0; ($i < 30) && ! (-r "$pidfile"); ++$i) {
      delay 0.1;
    }
  } else {
    warn "Couldn't run $cmd\n";
  }
}

sub snmptest_cleanup {
  my $ignore_failures = shift;

  kill_by_pid_file("t/snmpd.pid", $ignore_failures);
  unlink("t/snmpd.pid");
  kill_by_pid_file("t/snmptrapd.pid", $ignore_failures);
  unlink("t/snmptrapd.pid");
}

sub kill_by_pid_file {
  my $pidfile = shift;
  my $ignore_failures = shift;

  if (!open(H, "<$pidfile")) {
    return;
  }
  my $pid = (<H>);
  close (H);
  if (!$pid) {
    defined($ignore_failures) or die "Reading $pidfile failed\n";
    return;
  }
  if ($^O !~ /win32/i) {
    # Unix or Windows + Cygwin.
    system "kill $pid > /dev/null 2>&1";
  } else {
    # Windows + MSVC or Windows + MinGW.
    Win32::Process::KillProcess($pid, 0);
  }
  return 1;
}


# Stop any processes started during a previous test.
snmptest_cleanup(1);

# Start snmpd and snmptrapd.

#warn "\nStarting agents for test script $0\n";

my $scriptname = "snmptest";
if ($0 =~ /^t[\/\\](.*)\.t$/) {
  $scriptname = $1;
}

if ($snmpd_cmd) {
  run_async("t/snmpd.pid", "$snmpd_cmd", "-r -d -Lf t/snmpd-$scriptname.log -M+$mibdir -C -c t/snmptest.conf -p t/snmpd.pid ${agent_host}:${agent_port} >t/snmpd-$scriptname.stderr");
}
if ($snmptrapd_cmd) {
  run_async("t/snmptrapd.pid", "$snmptrapd_cmd", "-d -Lf t/snmptrapd-$scriptname.log -p t/snmptrapd.pid -M+$mibdir -C -c t/snmptest.conf -C ${agent_host}:${trap_port} >t/snmptrapd-$scriptname.stderr");
}

1;

