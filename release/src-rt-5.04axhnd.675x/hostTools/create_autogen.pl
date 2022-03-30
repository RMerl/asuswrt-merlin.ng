#!/usr/bin/env perl

# Generator for makefile.modsw.autogen

use File::Basename;
use Getopt::Long;
use strict;
use warnings;

my $bdir = $ENV{PWD};
my $kernelarch;
my @multiarchdirs;
my $mini;

# mini is an optional boolean for creating a minimal autogen to detect cyclic
# dependencies.  See userspace/Makefile.
GetOptions( "kernelarch=s", \$kernelarch, "mini", \$mini ) or die("usage");
my $fin  = shift;
my $fout = shift;

open( FIN, "<", $fin );
my $dlist = <FIN>;
close(FIN);

open( FOUT, ">", $fout );
print FOUT $dlist;
$dlist =~ s/^AUTODIRS := //;
foreach my $d ( split( /\s+/, $dlist ) ) {
    open( AD, "<", "$d/autodetect" );
    my $ldir = dirname("$d");
    my $ad;
    {
        local $/;
        $ad = <AD>;
    }
    close(AD);

    my $don = '';
    if ( $ad =~ /^dependson:\s*(\S.*\S)\s*$/m ) {
        $don = $1;
    }
    # print "$d depends on $don\n";
    print FOUT "$d: "
      . join(
        " ",
        grep( ( -e "$_/Makefile" || -e "$_/Bcmbuild.mk" ),
            split( /\s+/, $don ),
            map( ("$ldir/$_"), split( /\s+/, $don ) ) )
      ) . "\n";

    my $makeflags = '';
    if ( $ad =~ /^makeflags:\s*(\S.*\S)\s*$/m ) {
        $makeflags = $1;
    }

    my $condition;
    if ( $ad =~ /^condition:\s*(\S.*\S)\s*$/m ) {
        $condition = $1;
    }

    if ($condition && !$mini) {
        print FOUT "$condition\n";
    }

    my $makefile_opt = ''; 
    if (-e "$d/Bcmbuild.mk") {
        $makefile_opt = '-f Bcmbuild.mk';
    }

    # multi-arch build support (needed by 64bit kernel systems):
    # compile 64bit userspace app/lib (by default userspace apps/libs are 32bit)
    # kernel-aarch64: aarch64
    # compile both 64bit and 32bit userspace app/lib
    # kernel-aarch64: aarch64 arm
    my $archs;
    if ( $ad =~ /^kernel-$kernelarch:\s*(\S.*\S)\s*$/m ) {
        $archs = $1;
    }

    if (!$mini)
    {
        if ($archs) {
            push @multiarchdirs, $d;
            foreach my $a ( split( /\s+/, $archs ) ) {
                print FOUT "\t\$(MAKE) -C $d $makefile_opt $makeflags CURRENT_ARCH=$a\n";
            }
        }
        else {
            print FOUT "\t\$(MAKE) -C $d $makefile_opt $makeflags\n";
        }
    }

    if ($condition && !$mini) {
        print FOUT "else\n";
        print FOUT qq[\t\@echo "$d is skipped (not selected)"\n];
        print FOUT "endif\n";
    }
    print FOUT "\n.PHONY : $d\n\n";

    # special clean target for multi-arch
    if ($archs && !$mini) {
        print FOUT "$d/multiarchclean: \n";
        foreach my $a ( split( /\s+/, $archs ) ) {
            print FOUT "\t\$(MAKE) -C $d $makefile_opt $makeflags CURRENT_ARCH=$a clean\n";
        }
        print FOUT "\n.PHONY : $d/multiarchclean\n\n";
    }
}
print FOUT "\n";

my $multiarchstr = join ' ', @multiarchdirs;
print FOUT "MULTIARCH_AUTODIRS := $multiarchstr\n";

