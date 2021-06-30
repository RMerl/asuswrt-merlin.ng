package MakefileSubs;

use strict;
use warnings;
use Config;
use Cwd 'abs_path';
use File::Basename;
use File::Spec;
use Getopt::Long;
use Exporter;
use vars qw(@ISA @EXPORT_OK);

our $VERSION = 1.00;
our @ISA     = qw(Exporter);
our @EXPORT  = qw(NetSNMPGetOpts AddCommonParams find_files Check_Version
                  floatize_version);
our $basedir;

BEGIN {
    $basedir = abs_path($0);
    while (1) {
	my $basename = basename($basedir);
	last if (length($basename) <= 2);
	$basedir = dirname($basedir);
	last if ($basename eq "perl");
    }
    if ($Config{'osname'} eq 'MSWin32' && $basedir =~ / /) {
        die "\nA space has been detected in the base directory.  This is not " .
            "supported\nPlease rename the folder and try again.\n\n";
    }
}

sub NetSNMPGetOpts {
    my %ret;
    my $rootpath = $basedir;

    if ($ENV{'NET-SNMP-CONFIG'} && $ENV{'NET-SNMP-IN-SOURCE'}) {
	# have env vars, pull from there
	$ret{'nsconfig'} = $ENV{'NET-SNMP-CONFIG'};
	$ret{'insource'} = $ENV{'NET-SNMP-IN-SOURCE'};
	$ret{'define'}   = $ENV{'NET-SNMP-DEFINE'};
	$ret{'inc'}      = $ENV{'NET-SNMP-INC'};
	$ret{'cflags'}   = $ENV{'NET-SNMP-CFLAGS'};
	# $ret{'prefix'} is not used on Windows.
    } else {
	# don't have env vars, pull from command line and put there
	GetOptions("NET-SNMP-CONFIG=s"    => \$ret{'nsconfig'},
	           "NET-SNMP-IN-SOURCE=s" => \$ret{'insource'},
		   "NET-SNMP-DEFINE=s"    => \$ret{'define'},
		   "NET-SNMP-INC=s"       => \$ret{'inc'},
		   "NET-SNMP-CFLAGS=s"    => \$ret{'cflags'},
		   "NET-SNMP-PATH=s"      => \$ret{'prefix'});

	my $use_default_nsconfig;

	if ($ret{'insource'}) {
	    if (lc($ret{'insource'}) eq "true" && !defined($ret{'nsconfig'})) {
		$use_default_nsconfig = 1;
	    }
	}

	if ($use_default_nsconfig) {
	    $ret{'nsconfig'}="sh " . File::Spec->catfile(${rootpath}, "..",
							 "net-snmp-config");
	} elsif (!defined($ret{'nsconfig'})) {
	    $ret{'nsconfig'}="net-snmp-config";
	}

	$ENV{'NET-SNMP-CONFIG'}    = $ret{'nsconfig'};
	$ENV{'NET-SNMP-IN-SOURCE'} = $ret{'insource'};
	$ENV{'NET-SNMP-DEFINE'}    = $ret{'define'};
	$ENV{'NET-SNMP-INC'}       = $ret{'inc'};
	$ENV{'NET-SNMP-CFLAGS'}    = $ret{'cflags'};
	$ENV{'NET-SNMP-PATH'}      = $ret{'prefix'};
    }
    
    $ret{'rootpath'} = $rootpath;
    $ret{'debug'} = 'false';

    \%ret;
}

sub append {
    if ($_[0] && $_[1]) {
	$_[0] = $_[0] . " " . $_[1];
    } elsif ($_[1]) {
	$_[0] = $_[1];
    }
}

sub AddCommonParams {
    my $Params = shift;
    my $opts = NetSNMPGetOpts();

    append($Params->{'DEFINE'},  $opts->{'define'});
    append($Params->{'INC'},     $opts->{'inc'});
    append($Params->{'CCFLAGS'}, $opts->{'cflags'});

    if ($Config{'osname'} eq 'MSWin32') {
	# Microsoft Visual Studio.
	append($Params->{'DEFINE'}, "-DMSVC_PERL -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS");
	append($Params->{'INC'},
	       "-I" . File::Spec->catdir($basedir, "include") . " " .
	       "-I" . File::Spec->catdir($basedir, "win32") . " ");
    } else {
	# Unix or MinGW.
	append($Params->{'LDDLFLAGS'}, $Config{'lddlflags'});
	my $ldflags = `$opts->{'nsconfig'} --ldflags` or
	    die "net-snmp-config failed\n";
	chomp($ldflags);
	append($Params->{'LDDLFLAGS'}, $ldflags);
	append($Params->{'CCFLAGS'},
	       "-I" . File::Spec->catdir($basedir, "include"));
	my $cflags = `$opts->{'nsconfig'} --cflags` or
	    die "net-snmp-config failed\n";
	chomp($cflags);
	append($Params->{'CCFLAGS'}, $cflags);
	append($Params->{'CCFLAGS'}, $Config{'ccflags'});
	# Suppress known Perl header shortcomings.
	$Params->{'CCFLAGS'} =~ s/ -W(cast-qual|write-strings)//g;
	append($Params->{'CCFLAGS'}, '-Wformat');
    }
}

sub find_files {
    my($f,$d) = @_;
    my ($dir,$found,$file);
    for $dir (@$d){
	$found = 0;
	for $file (@$f) {
	    $found++ if -f "$dir/$file";
	}
	if ($found == @$f) {
	    return $dir;
	}
    }
}


sub Check_Version {
  my $lib_version = shift;

  if ($Config{'osname'} ne 'MSWin32') {
    my $foundversion = 0;
    return if ($ENV{'NETSNMP_DONT_CHECK_VERSION'});
    open(I,"<Makefile");
    while (<I>) {
	if (/^VERSION = (.*)/) {
	    my $perlver = $1;
	    my $srcver = $lib_version;
	    chomp($srcver);
	    my $srcfloat = floatize_version($srcver);
	    $perlver =~ s/pre/0./;
	    # we allow for perl/CPAN-only revisions beyond the default
	    # version formatting of net-snmp itself.
	    $perlver =~ s/(\.\d{5}).*/$1/;
	    $perlver =~ s/0*$//;
	    if ($srcfloat ne $perlver) {
		if (!$foundversion) {
		    print STDERR "ERROR:
Net-SNMP installed version: $srcver => $srcfloat
Perl Module Version:        $perlver

These versions must match for perfect support of the module.  It is possible
that different versions may work together, but it is strongly recommended
that you make these two versions identical.  You can get the Net-SNMP
source code and the associated perl modules directly from

   http://www.net-snmp.org/

If you want to continue anyway please set the NETSNMP_DONT_CHECK_VERSION
environmental variable to 1 and re-run the Makefile.PL script.\n";
		    exit(1);
		}
	    }
	    $foundversion = 1;
	    last;
	}
    }
    close(I);
    die "ERROR: Couldn't find version number of this module\n"
      if (!$foundversion);
  }
}

sub floatize_version {
    my ($major, $minor, $patch, $opps) = ($_[0] =~ /^(\d+)\.(\d+)\.?(\d*)\.?(\d*)/);
    if (!$patch) {
        $patch = 0;
    }
    if (!$opps) {
        $opps = 0;
    }
    return $major + $minor/100 + $patch/10000 + $opps/100000;
}

1;
