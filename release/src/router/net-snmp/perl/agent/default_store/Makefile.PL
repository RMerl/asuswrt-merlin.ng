use strict;
use warnings;
use ExtUtils::MakeMaker;
require 5;
use Config;
use MakefileSubs;

my $lib_version;

WriteMakefile(AgentDefaultStoreInitMakeParams());

Check_Version($lib_version);

sub AgentDefaultStoreInitMakeParams {
    my $opts;
    my %Params = (
		  'NAME'		=> 'NetSNMP::agent::default_store',
		  'VERSION_FROM'	=> 'default_store.pm', # finds $VERSION
		  'XSPROTOARG'          => '-prototypes',
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
	$Params{'LIBS'}    = `$opts->{'nsconfig'} --libs` or
	    die "net-snmp-config failed\n";
	chomp($Params{'LIBS'});
        $lib_version = `$opts->{'nsconfig'} --version` or
	    die "net-snmp-config failed\n";
	if ($opts->{'insource'} eq "true") {
	    $Params{'LIBS'} =
                "-L" . abs_path("../../../snmplib/.libs") .
                " -L" . abs_path("../../../snmplib") .
                " " . $Params{'LIBS'};
	}
    }

    return(%Params);
}
