use strict;
use warnings;
use ExtUtils::MakeMaker;
require 5;
use Config;
use MakefileSubs;

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

    if ($ENV{'OSTYPE'} eq 'msys') {
      $Params{'DEFINE'} = "-DMINGW_PERL";
    }
                                    
    my ($snmp_lib, $snmp_llib, $sep);
    if (($Config{'osname'} eq 'MSWin32' && $ENV{'OSTYPE'} eq '')) {
      $opts = NetSNMPGetOpts();	
      $Params{'DEFINE'} = "-DMSVC_PERL -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS";
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
	my @LibDirs = split (';',$ENV{LIB});
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

      $Params{'INC'} = "-I" . $MakefileSubs::basedir . "\\include\\ -I" . $MakefileSubs::basedir . "\\include\\net-snmp\\ -I" . $MakefileSubs::basedir . "\\win32\\ ";
    }
    else {
	$opts = NetSNMPGetOpts();
	$Params{'LDDLFLAGS'} = "$Config{lddlflags} " . `$opts->{'nsconfig'} --ldflags`;
	if (!$ENV{'NETSNMP_LIBS'}) {
	    $Params{'LIBS'}    = `$opts->{'nsconfig'} --libs`;
	    chomp($Params{'LIBS'});
	} else {
	    $Params{'LIBS'} = $ENV{'NETSNMP_LIBS'};
	}
	if (!$ENV{'NETSNMP_CCFLAGS'}) {
	    $Params{'CCFLAGS'} = `$opts->{'nsconfig'} --cflags`;
	    chomp($Params{'CCFLAGS'});
	    $Params{'CCFLAGS'} .= " " . $Config{'ccflags'};
	} else {
	    $Params{'CCFLAGS'} = $ENV{'NETSNMP_CCFLAGS'};
	}
        $lib_version = `$opts->{'nsconfig'} --version`;
	if (lc($opts->{'insource'}) eq "true") {
	    $Params{'LIBS'} =
                "-L" . abs_path("../../snmplib/.libs") .
                " -L" . abs_path("../../snmplib") .
                " " . $Params{'LIBS'};
	    $Params{'CCFLAGS'} = "-I../../include " . $Params{'CCFLAGS'};
#	} else {
#	    $Params{'PREREQ_PM'} = { 'NetSNMP::default_store' => 0.01 };
	}
	$Params{'CCFLAGS'} =~ s/ -W(all|inline|strict-prototypes|write-strings|cast-qual|no-char-subscripts)//g; # ignore developer warnings
        $Params{'CCFLAGS'} .= ' -Wformat';
	if (!$ENV{'NETSNMP_PREFIX'}) {
	    $prefix            = `$opts->{'nsconfig'} --prefix`;
	    chomp($prefix);
	    $sep = '/';
	} else {
	    $prefix = $ENV{'NETSNMP_PREFIX'};
	}
	if ($Params{'LIBS'} eq "" || $Params{'CCFLAGS'} eq "") {
	    die "You need to install net-snmp first (I can't find net-snmp-config)";
	}
    }

    return(%Params);

}

sub GetTestInfo {
    my $sep = ($^O =~ /win32/i ? '\\' : '/');
    my $info_file = "t${sep}snmptest.cmd";
    my $snmpd_path1 = "${prefix}${sep}sbin";
    my $snmpd_path2 = "${sep}usr${sep}sbin";
    my $snmpd_path3 = "${sep}usr${sep}bin";

    open(H, ">$info_file") || die "Error: could not open file '$info_file'($!)";

    my ($mibdir, $snmpd, $snmptrapd);

        # Windows
        if ($Config{'osname'} eq 'MSWin32' && $ENV{'OSTYPE'} eq '') {
          my $win32_snmpd_path = $ENV{'NET-SNMP-PATH'} . $sep . "bin";
          if (lc ($opts->{'insource'}) eq "true") {
            $mibdir = "../../mibs";
            if (lc($opts->{'debug'}) eq "true") {
              $snmpd = "../../win32/bin/debug";
              $snmptrapd = "../../win32/bin/debug";
            }
            else {
              $snmpd = "../../win32/bin/release";
              $snmptrapd = "../../win32/bin/release";
            }
          } else {
            $mibdir = $ENV{'NET-SNMP-PATH'} . "${sep}share${sep}snmp${sep}mibs";
            $snmpd = find_files(["snmpd.exe"], [$win32_snmpd_path]);
            $snmptrapd = find_files(["snmptrapd.exe"], [$win32_snmpd_path]);
          }
        }

        # Unix
        else {
          if (lc($opts->{'insource'}) eq "true") {
            $mibdir = "../../mibs";
            $snmpd = "../../agent/snmpd";
            $snmptrapd = "../../apps/snmptrapd";
          } else {
            $mibdir = "${prefix}${sep}share${sep}snmp${sep}mibs";
            $snmpd = find_files(["snmpd"], [$snmpd_path1, $snmpd_path2]);
            $snmptrapd = find_files(["snmptrapd"], [$snmpd_path1, $snmpd_path2]);
          }
        }

	$mibdir = find_files(["NET-SNMP-MIB.txt"],[$mibdir]);

	$mibdir ||= prompt("Unable to locate the MIBs, Please enter the path: ",
			   $mibdir);
	$snmpd ||= prompt("Unable to locate \"snmpd\". Please enter the path: ",
			  $snmpd_path1);
	$snmptrapd ||=
	    prompt("Unable to locate \"snmptrapd\". Please enter the path: ",
		   $snmpd_path1);

        if ($Config{'osname'} eq 'MSWin32' && $ENV{'OSTYPE'} eq '') {
          $snmpd = $snmpd . $sep . "snmpd.exe";
          $snmptrapd = $snmptrapd . $sep . "snmptrapd.exe";
        }
        else {
          if ($ENV{'OSTYPE'} eq 'msys') {
            $snmpd =~ s/snmpd$/snmpd.exe/;
            $snmptrapd =~ s/snmptrapd$/snmptrapd.exe/;
          }
          else
          {
            $snmpd =~ s/($sep)?(snmpd)?$/${sep}snmpd/;
            $snmptrapd =~ s/($sep)?(snmptrapd)?$/${sep}snmptrapd/;
          }
        }

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
# end of else
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
