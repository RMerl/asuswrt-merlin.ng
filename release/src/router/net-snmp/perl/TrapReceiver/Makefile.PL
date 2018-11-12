use strict;
use warnings;
use File::Basename;
require 5;
use Config;
use MakefileSubs;

my $lib_version;

WriteMakefile(TrapReceiverInitMakeParams());

Check_Version($lib_version);

if  (eval {require ExtUtils::Constant; 1}) {
  # If you edit these definitions to change the constants used by this module,
  # you will need to use the generated const-c.inc and const-xs.inc
  # files to replace their "fallback" counterparts before distributing your
  # changes.
  my @names = (qw(NETSNMPTRAPD_AUTH_HANDLER NETSNMPTRAPD_HANDLER_BREAK
		 NETSNMPTRAPD_HANDLER_FAIL NETSNMPTRAPD_HANDLER_FINISH
		 NETSNMPTRAPD_HANDLER_OK NETSNMPTRAPD_POST_HANDLER
		 NETSNMPTRAPD_PRE_HANDLER));
  ExtUtils::Constant::WriteConstants(
                                     NAME         => 'NetSNMP::TrapReceiver',
                                     NAMES        => \@names,
                                     DEFAULT_TYPE => 'IV',
                                     C_FILE       => 'const-c.inc',
                                     XS_FILE      => 'const-xs.inc',
                                  );

} else {
  use File::Copy;
  use File::Spec;
  foreach my $file ('const-c.inc', 'const-xs.inc') {
    my $fallback = File::Spec->catfile('fallback', $file);
    copy ($fallback, $file) or die "Can't copy $fallback to $file: $!";
  }
}

sub TrapReceiverInitMakeParams {
    my $opts;
    my %Params = (
		  'NAME'		=> 'NetSNMP::TrapReceiver',
		  'VERSION_FROM'	=> 'TrapReceiver.pm', # finds $VERSION
		  'XSPROTOARG'          => '-prototypes',
		  'PREREQ_PM'		=> { 'NetSNMP::OID' => 5.02 },
		  ($] >= 5.005 ? ## Add these new keywords supported since 5.005
		   (ABSTRACT_FROM => 'TrapReceiver.pm',
		    AUTHOR        =>
		    'W. Hardaker <hardaker@users.sourceforge.net>') : ()),
		  'INC'		=> '-I.', # e.g., '-I. -I/usr/include/other'
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
      my $agent_link_lib = 'netsnmpagent';
      my $mibs_link_lib = 'netsnmpmibs';
      my $trapd_link_lib = 'netsnmptrapd';

      if (lc($opts->{'debug'}) eq "true") {
        $lib_dir = 'lib\\debug';
      }
      else {
        $lib_dir = 'lib\\release';
      }
    
      if (lc($opts->{'insource'}) eq "true") {
	$Params{'LIBS'} = "-L" . $MakefileSubs::basedir . "\\win32\\$lib_dir\\ -l$snmp_link_lib -l$agent_link_lib " .
                "-l$mibs_link_lib -l$trapd_link_lib ";
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
	$Params{LIBS} = "-L$LibDir -l$snmp_link_lib -l$agent_link_lib -l$mibs_link_lib -l$trapd_link_lib ";
      }

      $Params{'INC'} = "-I" . $MakefileSubs::basedir . "\\include\\ -I" . $MakefileSubs::basedir . "\\include\\net-snmp\\ -I" . $MakefileSubs::basedir . "\\win32\\ ";
    }
    else {
	$opts = NetSNMPGetOpts();
	$Params{'LDDLFLAGS'} = "$Config{lddlflags} " . `$opts->{'nsconfig'} --ldflags`;
	$Params{'LIBS'} = '-lnetsnmptrapd ' . `$opts->{'nsconfig'} --base-agent-libs`;
	chomp($Params{'LIBS'});
	if (!$ENV{'NETSNMP_CCFLAGS'}) {
	    $Params{'CCFLAGS'} = `$opts->{'nsconfig'} --cflags`;
	    chomp($Params{'CCFLAGS'});
	    $Params{'CCFLAGS'} .= " " . $Config{'ccflags'};
	} else {
	    $Params{'CCFLAGS'} = $ENV{'NETSNMP_CCFLAGS'};
	}

	if (lc($opts->{'insource'}) eq "true") {
            $Params{'LIBS'} =
                "-L" . abs_path("../../apps/.libs") .
                " -L" . abs_path("../../apps") .
                " -L" . abs_path("../../agent/.libs") .
                " -L" . abs_path("../../agent") .
                " -L" . abs_path("../../snmplib/.libs") .
                " -L" . abs_path("../../snmplib") .
                " " . $Params{'LIBS'};
	    $Params{'CCFLAGS'} = "-I../../include " . $Params{'CCFLAGS'};
	} else {
            $Params{'LIBS'} = `$opts->{'nsconfig'} --libdir` . " $Params{'LIBS'}";
        }

	$Params{'CCFLAGS'} =~ s/ -W(all|inline|strict-prototypes|write-strings|cast-qual|no-char-subscripts)//g; # ignore developer warnings
        $Params{'CCFLAGS'} .= ' -Wformat';
	if ($Params{'CCFLAGS'} eq "") {
	    die "You need to install net-snmp first (I can't find net-snmp-config)";
	}
        $lib_version = `$opts->{'nsconfig'} --version`;
    }

    return(%Params);
}
