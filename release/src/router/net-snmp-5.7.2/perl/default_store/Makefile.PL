use strict;
use warnings;
use ExtUtils::MakeMaker;
require 5;
use Config;
use Cwd 'abs_path';
use MakefileSubs;

my $lib_version;

WriteMakefile(DefaultStoreInitMakeParams());

sub DefaultStoreInitMakeParams {
    my $opts;
    my %Params = (
		  'NAME'		=> 'NetSNMP::default_store',
		  'VERSION_FROM'	=> 'default_store.pm', # finds $VERSION
		  'XSPROTOARG'          => '-prototypes',
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
	$Params{'LIBS'}    = `$opts->{'nsconfig'} --libs`;
	chomp($Params{'LIBS'});
	$Params{'CCFLAGS'} = `$opts->{'nsconfig'} --cflags`;
	chomp($Params{'CCFLAGS'});
	$Params{'CCFLAGS'} .= " " . $Config{'ccflags'};
        $lib_version = `$opts->{'nsconfig'} --version`;
	if (lc($opts->{'insource'}) eq "true") {
	    $Params{'LIBS'} =
                "-L" . abs_path("../../snmplib/.libs") .
                " -L" . abs_path("../../snmplib") .
                " " . $Params{'LIBS'};
	    $Params{'CCFLAGS'} = "-I../../include " . $Params{'CCFLAGS'};
	}
	$Params{'CCFLAGS'} =~ s/ -W(all|inline|strict-prototypes|write-strings|cast-qual|no-char-subscripts)//g; # ignore developer warnings
        $Params{'CCFLAGS'} .= ' -Wformat';
	if ($Params{'LIBS'} eq "" || $Params{'CCFLAGS'} eq "") {
	    die "You need to install net-snmp first (I can't find net-snmp-config)";
	}
    }

    return(%Params);
}
