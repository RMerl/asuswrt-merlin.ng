use strict;
use warnings;
use Carp;
use Config;
use Cwd 'abs_path';
use ExtUtils::MakeMaker;
use File::Spec;
use MakefileSubs;
require 5;

my $lib_version;
my $opts;
my $prefix;

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

WriteMakefile(SnmpInitMakeParams());

Check_Version($lib_version);

GetTestInfo();

sub SnmpInitMakeParams {
    my %Params = (
		  NAME         => 'SNMP',
		  dist         => { SUFFIX => "gz", COMPRESS => "gzip -9f"},
		  MAN3PODS => { 'SNMP.pm' => '$(INST_MAN3DIR)/SNMP.3' },
		  XSPROTOARG   => '-noprototypes', 	# XXX remove later?
		  VERSION_FROM => 'SNMP.pm',
		  realclean        => { FILES => 'host' },
		  );

    AddCommonParams(\%Params);
                                    
    my ($snmp_lib, $snmp_llib, $sep);

    $opts = NetSNMPGetOpts();

    if ($Config{'osname'} eq 'MSWin32' && !defined($ENV{'OSTYPE'})) {
      $sep = '\\';
      my $snmp_lib_file = 'netsnmp.lib';
      my $snmp_link_lib = 'netsnmp';
      my $lib_dir;

      if (lc($opts->{'debug'}) eq "true") {
        $lib_dir = 'lib\\debug';
      }
      else {
        $lib_dir = 'lib\\release';
      }
      
      if (lc($opts->{'insource'}) eq "true") {
	$Params{'LIBS'} = "-L" . $MakefileSubs::basedir . "\\win32\\$lib_dir\\ -l$snmp_link_lib";
      }
      else {
	my @LibDirs = split($Config{path_sep}, $ENV{LIB});
        my $LibDir;
	if ($opts->{'prefix'}) {
	  push (@LibDirs,"$ENV{'NET-SNMP-PATH'}${sep}lib");
	}
	my $noLibDir = 1;
	while ($noLibDir) {
	  $LibDir = find_files(["$snmp_lib_file"],\@LibDirs);
	  if ($LibDir ne '') {
	    $noLibDir = 0;
            # Put quotes around LibDir to allow spaces in paths
            $LibDir = '"' . $LibDir . '"';
	  }
	  else
	  {
	    @LibDirs = ();
	    $LibDirs[0] = prompt("The Net-SNMP library ($snmp_lib_file) could not be found.\nPlease enter the directory where it is located:");
	    $LibDirs[0] =~ s/\\$//;
	  }
	}
	$Params{LIBS} = "-L$LibDir -l$snmp_link_lib";
      }
    }
    else {
	if (!$ENV{'NETSNMP_LIBS'}) {
	    $Params{'LIBS'}    = `$opts->{'nsconfig'} --libs` or
		die "net-snmp-config failed\n";
	    chomp($Params{'LIBS'});
	} else {
	    $Params{'LIBS'} = $ENV{'NETSNMP_LIBS'};
	}
        $lib_version = `$opts->{'nsconfig'} --version` or
	    die "net-snmp-config failed\n";
	if (lc($opts->{'insource'}) eq "true") {
	    $Params{'LIBS'} =
                "-L" . abs_path("../../snmplib/.libs") .
                " -L" . abs_path("../../snmplib") .
                " " . $Params{'LIBS'};
#	} else {
#	    $Params{'PREREQ_PM'} = { 'NetSNMP::default_store' => 0.01 };
	}
	$prefix = ($opts->{'prefix'} or `$opts->{'nsconfig'} --prefix` or
	    die "net-snmp-config failed\n");
	chomp($prefix);
	$sep = '/';
    }

    return(%Params);

}

sub GetTestInfo {
    my $info_file = File::Spec->catfile("t", "snmptest.cmd");

    open(H, ">$info_file") || die "Error: could not open file '$info_file'($!)";

    my ($mibdir, $snmpd, $snmptrapd);

    # Windows
    if ($Config{'osname'} eq 'MSWin32' && !defined($ENV{'OSTYPE'})) {
	if (lc ($opts->{'insource'}) eq "true") {
            $mibdir = abs_path("../../mibs");
	    my $cfg = lc($opts->{'debug'}) eq "true" ? "debug" : "release";
	    $snmpd = abs_path("../../win32/bin/$cfg/snmpd.exe");
	    $snmptrapd = abs_path("../../win32/bin/$cfg/snmptrapd.exe");
	} else {
	    my $win32_snmpd_path = File::Spec->catdir($ENV{'NET-SNMP-PATH'},
						      "bin");
            $mibdir = File::Spec->catdir($ENV{'NET-SNMP-PATH'}, "share",
					 "snmp", "mibs");
            $snmpd = find_files(["snmpd.exe"], [$win32_snmpd_path]);
            $snmptrapd = find_files(["snmptrapd.exe"], [$win32_snmpd_path]);
	}
    }
    # Unix
    else {
	if (lc($opts->{'insource'}) eq "true") {
            $mibdir = abs_path("../../mibs");
            $snmpd = abs_path("../../agent/snmpd");
            $snmptrapd = abs_path("../../apps/snmptrapd");
	} else {
            croak "\$prefix has not been set" if (!defined($prefix));
            $mibdir = "${prefix}/share/snmp/mibs";
            my $snmpd_path1 = abs_path("${prefix}/sbin");
            my $snmpd_path2 = "/usr/sbin";
            $snmpd = find_files(["snmpd"], [$snmpd_path1, $snmpd_path2]);
            $snmptrapd = find_files(["snmptrapd"], [$snmpd_path1, $snmpd_path2]);
	}
    }

    $mibdir = find_files(["NET-SNMP-MIB.txt"], [$mibdir]);

    $mibdir ||= prompt("Unable to locate the MIBs, Please enter the path: ",
		       $mibdir);
    $snmpd ||= prompt("Unable to locate \"snmpd\". Please enter the path: ");
    $snmptrapd ||=
	prompt("Unable to locate \"snmptrapd\". Please enter the path: ");

    print H "SNMPD => $snmpd\n";
    print H "SNMPTRAPD => $snmptrapd\n";
    print H "MIBDIR => $mibdir\n";

    if (!(lc($opts->{'insource'}) eq "true")) {
	if (-e $snmpd and -r $snmpd) {
	    if (not -x $snmpd) {
		warn("Error: $snmpd not executable. 'make test' will not work.\n");
	    }
	} else {
	    warn("Error: $snmpd does not exist or is unreadable. 'make test' will not work.\n");
	}

	if (-e $snmptrapd and -r $snmptrapd) {
	    if (not -x $snmptrapd) {
		warn("Error: $snmptrapd not executable. 'make test' will not work.\n");
	    }
	} else {
	    warn("Error: $snmptrapd does not exist or is unreadable. 'make test' will not work.\n");
	}
    }
    close H;
}

sub HasSSL {
    my $config_header = shift;
    my $has_ssl;
    unless (open(C,"<$config_header")) {
	warn("Unable to open $config_header, assuming no SSL\n");
	return undef;
    }
    while (<C>) {
	$has_ssl++, last if /^\s*#define\s+NETSNMP_USE_OPENSSL/;
    }
    close C;
    return $has_ssl;
}
