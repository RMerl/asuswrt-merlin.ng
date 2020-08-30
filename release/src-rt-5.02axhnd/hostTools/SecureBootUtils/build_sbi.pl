#!/usr/bin/env perl 
use strict;
use warnings;
use bytes;
use FindBin qw($Bin);
use lib "$Bin/../PerlLib";
use BRCM::SBI_DEP;
use Getopt::Long;

#use warning;

my $chip;
my $cred_dir;     # directory where all our images are in
my $sec_arch;     #Gen2/Gen3
my $sec_mode;     #MFG/FLD
my $sec_opt;      #encrypt oem  skp spu
my $byteorder;    #encrypt oem  skp spu
my $in;           #image to render
my $in1;          #image to render
my $max_size;     #image to render
my $out;          #output image
my $build_top;    #build_top

GetOptions(
    "chip=s",     \$chip,     "cred_dir=s",  \$cred_dir,
    "sec_arch=s", \$sec_arch, "byteorder=s", \$byteorder,
    "max_size=s", \$max_size, "sec_mode=s",  \$sec_mode,
    "sec_opt=s",  \$sec_opt,  "build_top=s", \$build_top,
    "in=s",       \$in,       "in1=s",       \$in1,
    "out=s",      \$out
) or die("Invalid Option");

sub run {
    my %args;
    local *_unqt = sub {
        if ( defined $_[0] ) {
            $_[0] =~ s/^\s*\"|\"\s*$//g;
        }
        return $_[0];
    };
    $args{'sec_mode'} = _unqt($sec_mode);
    $args{'chip'}     = _unqt($chip);

    #	$args{'sec_arch'} = _unqt($sec_arch);
    $args{'sec_opt'}   = _unqt($sec_opt);
    $args{'byteorder'} = _unqt($byteorder);
    $args{'cred_dir'}  = _unqt($cred_dir);
    $args{'in'}        = _unqt($in);
    $args{'out'}       = _unqt($out);
    $args{'buildtop'}  = _unqt($build_top);

    #Gen2 related parms
    $args{'max_size'} = _unqt($max_size);
    $args{'in1'}      = _unqt($in1);
    my $sbi_lib = BRCM::SBI_DEP->new( \%args );
    $sbi_lib->build();
}
run();
