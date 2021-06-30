#!./perl

use strict;
use warnings;
use Cwd qw(getcwd);
use Exporter;

our @ISA = 'Exporter';
our @EXPORT_OK = qw($agent_host $agent_port $mibdir $snmpd_cmd $snmptrapd_cmd);
our ($agent_host, $agent_port, $mibdir, $snmpd_cmd, $snmptrapd_cmd);

if (open(CMD, "<../SNMP/t/snmptest.cmd")) {
  while (my $line = <CMD>) {
    if ($line =~ /HOST\s*=>\s*(.*?)\s+$/) {
      $agent_host = $1;
    } elsif ($line =~ /MIBDIR\s*=>\s*(.*?)\s+$/) {
      $mibdir = $1;
    } elsif ($line =~ /AGENT_PORT\s*=>\s*(.*?)\s+$/) {
      $agent_port = $1;
    } elsif ($line =~ /SNMPD\s*=>\s*(.*?)\s+$/) {
      $snmpd_cmd = $1;
    } elsif ($line =~ /SNMPTRAPD\s*=>\s*(.*?)\s+$/) {
      $snmptrapd_cmd = $1;
    }
  } # end of while
  close CMD;
} else {
  die ("Could not start agent. Couldn't find snmptest.cmd file\n");
}

# On Windows %ENV changes only affect new processes but not the current
# process. Hence write the MIB dir into an snmp.conf file instead of setting
# $ENV{'MIBDIRS'}.
open(H, ">snmp.conf") or die "Failed to open snmp.conf: $!";
print H "mibdirs +$mibdir\n";
close(H);
# use SNMP;
# SNMP::register_debug_tokens("get_mib_directory");
# SNMP::register_debug_tokens("read_config");
use NetSNMP::default_store (':all');
netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CONFIGURATION_DIR,
		      getcwd()) == 0 or
    die "Failed to set configuration directory";

1;
