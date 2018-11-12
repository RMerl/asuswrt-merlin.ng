#!/usr/bin/perl
#
# Build script for Net-SNMP and MSVC
# Written by Alex Burger - alex_b@users.sourceforge.net
# March 12th, 2004
#
use strict;
use warnings;
use Config;
use Cwd 'abs_path';
use File::Basename;
use constant false => 0;
use constant true => 1;
my $target_arch = $ENV{TARGET_CPU} ? $ENV{TARGET_CPU} : $ENV{Platform} ?
                  $ENV{Platform} : "x86";
$target_arch = lc $target_arch;
if ($target_arch ne "x86" && $target_arch ne "x64") {
    print "Error: unsupported target architecture $target_arch\n";
    die;
}
my @perl_arch = split(/-/, $Config{archname});
my $openssl = false;
my $default_openssldir = $target_arch eq "x64" ?
    "C:\\OpenSSL-Win64" : "C:\\OpenSSL-Win32";
my $default_opensslincdir = $default_openssldir . "\\include";
my $opensslincdir = $default_opensslincdir;
my $default_openssllibdir = $default_openssldir . "\\lib\\VC";
my $openssllibdir = $default_openssllibdir;
my $b_ipv6 = false;
my $b_winextdll = false;
my $sdk = false;
my $default_install_base = "c:/usr";
my $install_base = $default_install_base;
my $install = true;
my $install_devel = false;
my $perl = false;
my $perl_install = false;
my $logging = true;
my $debug = false;
my $configOpts;
my $link_dynamic = false;
my $option;

# Path of this script (source tree path + "win32").
my $current_pwd = dirname(abs_path($0));

if (!defined($ENV{MSVCDir}) && !defined($ENV{VCINSTALLDIR}) &&
    !defined($ENV{TARGET_CPU})) {
  print "\nPlease run VCVARS32.BAT first to set up the Visual Studio build\n" .
        "environment.\n\n";
  system("pause");
  exit;
}

while (1) {
  print "\n\nNet-SNMP build and install options\n";
  print "==================================\n\n";
  print "1.  OpenSSL support:                " . ($openssl ? "enabled" : "disabled"). "\n";
  print "2.  OpenSSL include directory:      " . $opensslincdir. "\n";
  print "3.  OpenSSL library director:       " . $openssllibdir. "\n";
  print "4.  Platform SDK support:           " . ($sdk ? "enabled" : "disabled") . "\n";
  print "\n";
  print "5.  Install path:                   " . $install_base . "\n";
  print "6.  Install after build:            " . ($install ? "enabled" : "disabled") . "\n";
  print "\n";
  print "7.  Perl modules:                   " . ($perl ? "enabled" : "disabled") . "\n";
  print "8.  Install perl modules:           " . ($perl_install ? "enabled" : "disabled") . "\n";
  print "\n";
  print "9.  Quiet build (logged):           " . ($logging ? "enabled" : "disabled") . "\n";
  print "10. Debug mode:                     " . ($debug ? "enabled" : "disabled") . "\n";
  print "\n";
  print "11. IPv6 transports (requires SDK): " . ($b_ipv6 ? "enabled" : "disabled") . "\n";
  print "12. winExtDLL agent (requires SDK): " . ($b_winextdll ? "enabled" : "disabled") . "\n";
  print "\n";
  print "13. Link type:                      " . ($link_dynamic ? "dynamic" : "static") . "\n";
  print "\n";
  print "14. Install development files       " . ($install_devel ? "enabled" : "disabled") . "\n";
  print "\nF.  Finished - start build\n";
  print "Q.  Quit - abort build\n\n";
  print "Select option to set / toggle: ";

  chomp ($option = <>);
  if ($option eq "1") {
    $openssl = !$openssl;
  }
  elsif ($option eq "2") {
    print "Please enter the OpenSSL include directory [$opensslincdir]: ";
    chomp ($opensslincdir = <>);
    $opensslincdir =~ s/\\/\//g;
    $opensslincdir = $default_opensslincdir if ($opensslincdir eq "");
  }
  elsif ($option eq "3") {
    print "Please enter the OpenSSL library directory [$openssllibdir]: ";
    chomp ($openssllibdir = <>);
    $openssllibdir =~ s/\\/\//g;
    $openssllibdir = $default_openssllibdir if ($openssllibdir eq "");
  }
  elsif ($option eq "4") {
    $sdk = !$sdk;
  }
  elsif ($option eq "11") {
    $b_ipv6 = !$b_ipv6;
    if ($b_ipv6 && !$sdk) {
      print "\n\n* SDK required for IPv6 and has been automatically enabled";
      $sdk = true;
    }
  }
  elsif ($option eq "12") {
    $b_winextdll = !$b_winextdll;
    if ($b_winextdll && !$sdk) {
      print "\n\n* SDK required for IPv6 and has been automatically enabled";
      $sdk = true;
    }
  }
  elsif ($option eq "5") {
    print "Please enter the new install path [$default_install_base]: ";
    chomp ($install_base = <>);
    $install_base =~ s/\\/\//g;
    $install_base = $default_install_base if ($install_base eq "");
  }
  elsif ($option eq "6") {
    $install = !$install;
  }
  elsif ($option eq "14") {
    $install_devel = !$install_devel;
  }
  elsif ($option eq "7") {
    $perl = !$perl;
  }
  elsif ($option eq "8") {
    $perl_install = !$perl_install;
  }
  elsif ($option eq "9") {
    $logging = !$logging;
  }
  elsif ($option eq "10") {
    $debug = !$debug;
  }
  elsif ($option eq "13") {
    $link_dynamic = !$link_dynamic;
  }
  elsif (lc($option) eq "f") {
    last;
  }
  elsif (lc($option) eq "q") {
    exit;
  }
}

if ($perl && $perl_arch[1] ne $target_arch) {
    print "perl_arch = $perl_arch[1] does not match target_arch = $target_arch\n";
    die;
}

my $linktype = $link_dynamic ? "dynamic" : "static";
$configOpts = (($openssl ? "--with-ssl" : "")		. " " .
               ($opensslincdir ? "--with-sslincdir=$opensslincdir" : "") . " " .
               ($openssllibdir ? "--with-ssllibdir=$openssllibdir" : "") . " " .
               ($sdk ? "--with-sdk" : "")		. " " .
               ($b_ipv6 ? "--with-ipv6" : "")		. " " .
               ($b_winextdll ? "--with-winextdll" : "") . " " .
               ($debug ? "--config=debug" : "--config=release"));

# Set environment variables

# Set to not search for non-existent ".dep" files
$ENV{NO_EXTERNAL_DEPS}="1";

# Set PATH environment variable so Perl make tests can locate the DLL
$ENV{PATH} = File::Spec->catdir($current_pwd, "bin", $debug ? "debug" : "release") . ";$ENV{PATH}";

# Set MIBDIRS environment variable so Perl make tests can locate the mibs
$ENV{MIBDIRS} = File::Spec->catdir(dirname($current_pwd), "mibs");

# Set SNMPCONFPATH environment variable so Perl conf.t test can locate
# the configuration files.
# See the note about environment variables in the Win32 section of
# perl/SNMP/README for details on why this is needed.
$ENV{SNMPCONFPATH}="t";

print "\nBuilding...\n";

print "\nCreating *.out log files.\n\n";

print "Deleting old log files...\n";
system("del *.out" . ($logging ? " > NUL: 2>&1" : ""));

# Delete net-snmp-config.h from main include folder just in case it was created by a Cygwin or MinGW build
unlink "../include/net-snmp/net-snmp-config.h";
unlink "../snmplib/transports/snmp_transport_inits.h";

print "Running Configure...\n";
system("perl Configure $configOpts --linktype=$linktype --prefix=\"$install_base\"" . ($logging ? " > configure.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see configure.out)" : "Build error (see above)");

print "Cleaning...\n";
system("nmake /nologo clean" . ($logging ? " > clean.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see clean.out)" : "Build error (see above)");

print "Building main package...\n";
system("nmake /nologo" . ($logging ? " > make.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see make.out)" : "Build error (see above)");

if ($perl) {
  if ($Config{'ccname'} =~ /^gcc/) {
    die "The perl interpreter has been built with gcc instead of MSVC. Giving up.\n";
  }
  if (!$link_dynamic) {
    print "Running Configure for DLL...\n";
    system("perl Configure $configOpts --linktype=dynamic --prefix=\"$install_base\"" . ($logging ? " > perlconfigure.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see perlconfigure.out)" : "Build error (see above)");

    print "Cleaning libraries...\n";
    system("nmake /nologo libs_clean" . ($logging ? " >> clean.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see clean.out)" : "Build error (see above)");

    print "Building DLL libraries...\n";
    system("nmake /nologo libs" . ($logging ? " > dll.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see dll.out)" : "Build error (see above)");
  }

  print "Cleaning Perl....\n";
  system("nmake /nologo perl_clean" . ($logging ? " >> clean.out 2>&1" : "")); # If already cleaned, Makefile is gone so don't worry about errors!

  print "Building Perl modules...\n";
  system("nmake /nologo perl" . ($logging ? " > perlmake.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see perlmake.out)" : "Build error (see above)");

  print "Testing Perl modules...\n";
  system("nmake /nologo perl_test" . ($logging ? " > perltest.out 2>&1" : "")); # Don't die if all the tests don't pass..

  if ($perl_install) {
    print "Installing Perl modules...\n";
    system("nmake /nologo perl_install" . ($logging ? " > perlinstall.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see perlinstall.out)" : "Build error (see above)");
  }

  print "\nSee perltest.out for Perl test results\n";
}

print "\n";
if ($install) {
  print "Installing main package...\n";
  system("nmake /nologo install" . ($logging ? " > install.out 2>&1" : "")) == 0 || die ($logging ? "Build error (see install.out)" : "Build error (see above)");
}
else {
  print "Type nmake install to install the package to $install_base\n";
}

if ($install_devel) {
  print "Installing development files...\n";
  system("nmake /nologo install_devel > install_devel.out 2>&1") == 0 || die "Build error (see install_devel.out)";
}
else {
  print "Type nmake install_devel to install the development files to $install_base\n";
}

if ($perl && !$perl_install) {
  print "Type nmake perl_install to install the Perl modules\n";
}

print "\nDone!\n";

# Local Variables:
# mode: perl
# perl-indent-level: 2
# indent-tabs-mode: nil
# End:
