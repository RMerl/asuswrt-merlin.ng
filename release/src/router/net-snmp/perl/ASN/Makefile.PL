use strict;
use warnings;
use Config;
use Cwd 'abs_path';
use ExtUtils::MakeMaker;
use File::Basename;
require 5;
use Config;
use MakefileSubs;

my $lib_version;

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

WriteMakefile(AsnInitMakeParams());

Check_Version($lib_version);

sub AsnInitMakeParams {
    my $opts;
    my %Params = (
		  'NAME'		=> 'NetSNMP::ASN',
		  'VERSION_FROM'	=> 'ASN.pm', # finds $VERSION
		  'XSPROTOARG'          => '-prototypes',
		  'PREREQ_PM'           => {},
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
    else
    {
      $Params{'LIBS'}    = `$opts->{'nsconfig'} --libs` or
          die "net-snmp-config failed\n";
      chomp($Params{'LIBS'});
      $lib_version = `$opts->{'nsconfig'} --version` or
          die "net-snmp-config failed\n";
      
      if (lc($opts->{'insource'}) eq "true") {
	$Params{'LIBS'} =
            "-L" . abs_path("../../snmplib/.libs") .
            " -L" . abs_path("../../snmplib") .
            " " . $Params{'LIBS'};
      }
    }
    return (%Params);
}
