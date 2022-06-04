#!/usr/bin/perl
use strict;
use warnings;

use Getopt::Long;

my $executable = "";
my $vf="--version";
my $quiet=0;
my $reqVersion = "";
my $version = "";
my $showVersion;

my $usage = q{
Usage:  checkver --executable=s [--flag=f] [--quiet] [--requiredVersion=s]
        checkver --setversion=s [--quiet] [--requiredVersion=s]
        checkver --help
        checkver --version

options:
  -e       The executable to run to get the version
  -f       the flag to pass the executable to get the verions (default is --version)
  -v       pass in a version (may not be used with -e)
  -r       the required version

If checkver is called with a required version, it will return 0 if the found version
is greater than or equal to the required version and non-zero otherwise.


};

GetOptions (    
                'exececutable|tool=s'    => \$executable,
                'quiet!'            => \$quiet,
                'requiredVersion=s' => \$reqVersion,
                'version'           => \$showVersion,
                'setversion=s'      => \$version,
                'flag=s'            => \$vf,
                'help'              => sub {print $usage;} ) or die $usage;

if ($showVersion) {
    print "checkver 0.2\n";
    print "written by John Ulvr <julvr\@broadcom.com>\n\n";
    exit 0;
}


sub getVersionFromExe {
    my ($executable, $flag) = @_;
    my $versionRaw = `$executable $flag 2>/dev/null`;
    if ($?) {
        if (!$quiet) {
            print STDERR "****************************************************\n"
                       . " Error: Could not retreive version from $executable\n"
                       . " $executable is likely not installed on your system\n"
                       . "****************************************************\n";
        }
        exit 2;
    }
    $versionRaw =~ /(\d+(?:\.\w+)+)/;
    return $1;
}

die "checkver: Must specify executable or version\n" if ( !($executable) && !($version));
die "checkver: Must specify only one of executable or version\n" if ( ($executable) && ($version));

#compares strings such as 12a, 43b, 1...
#returns 1 if v2 > v1, -1 if v1 > v2, and 0 otherwise
sub cmpNum {
    my ($v1, $v2) = @_;
    $v1 =~ m/(\d*)(.*)/;
    my $v1Num=$1;
    my $v1Let=$2;
    $v2 =~ m/(\d*)(.*)/;
    my $v2Num=$1;
    my $v2Let=$2;

    #check number:
    if ( $v1Num.$v2Num ) {
        return 1 unless $v1Num;
        return -1 unless $v2Num;
        return 1 if $v2Num > $v1Num;
        return -1 if $v1Num > $v2Num;
    }
    #fall through.  Blank is smaller than non-blank...
    return 1 if "".$v2Let gt "".$v1Let;
    return -1 if "".$v1Let gt "".$v2Let;
    return 0;
}

# -------------------------------------------------------------------------
# Main program:


$version = getVersionFromExe($executable, $vf) if $executable;
die "Unable to parse version\n" unless $version;
if ($reqVersion) {
    my @versionNums=split(/\./, $version);
    my @reqNums = split( /\./, $reqVersion);

    while(scalar(@reqNums)) {
        my $c=cmpNum(shift(@reqNums),shift(@versionNums));
        if ($c < 0){
            if (!$quiet) {
                print STDERR "****************************************************\n"
                           . " Error: version check failed";
                print STDERR " on $executable" if $executable;
                print STDERR "\n version $version does not meet required version $reqVersion\n"
                           . "****************************************************\n";
            }
            exit 2;
        }
        elsif ($c > 0) {
            last;
        }
    }
}

print "$version\n" unless $quiet;
exit 0;

