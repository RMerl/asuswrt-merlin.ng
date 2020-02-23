#!/usr/bin/env perl

use strict;
use warnings;
use bytes;
use Getopt::Long;
use File::Find;
use File::Temp qw[ tempfile ];
use File::Basename;
use Data::Dumper;
use BRCM::SBI;
#use BRCM::SBI_UTIL;

my $usage = qq[usage: $0 boot_region_file
   --in filename #  
   --out filename    # output binary 
   --sign            # private key in pem format 
   --ek              # cbc128 AES encryption key   
   --ev              # cbc128 AES IV vector  
   --la              # link address for AVS 
];

my %X = ();
GetOptions( \%X, 'in=s', 'out=s', 'sign=s', 'ek=s', 'iv=s', 'la:o','verify', 'ssldir:s')

  or die(" $usage ");
if (!defined($X{la})) {
    $X{la} = 0x82616fc0;
}
my $enc = new BRCM::SBI_UTIL('little', 1, $X{ssldir});
my $u = new BRCM::SBI(
        {
            sec_mode  => 'UNSEC',
            byteorder => 'little',
            chip      => '6878',
            cred      => '' 
        }
    );
sub build {
    my $image = $enc->cipher($u->f2var($X{in}), {'ek',unpack("H32",$u->f2var($X{ek})),'iv',unpack("H32",$u->f2var($X{iv}))}); 
    my $sig = $enc->sign($u->f2var($X{sign}),$image);
    my $hdr = $u->prepare_header_avs( length($image), length($sig), $X{la} );
printf ("Hdr length %d out to $X{out} sig length %d image len %d\n",length $hdr, length $sig, length $image);
    $u->fdump($X{out}, $hdr . $image . $sig);
    verify($X{out});
}

sub verify {
    my $_in = shift;
    my $_out = shift;
    my $data = $u->f2var($_in);
    my $hdr = $u->avs_header_parse($data);
    die "ERROR:Header Corrupt" unless ($hdr->{crc} == $u->crc32(substr($data,0,$hdr->{hlen}-4),$hdr->{len})); 	
    my $sig = substr($data, $hdr->{len}-256, 256);
    my $image = substr($data, $hdr->{hlen},$hdr->{len} - ($hdr->{hlen} + 256));
    die "ERROR:Failed to authenticate" unless (!$enc->authenticate($u->f2var($X{sign}),$image,$sig));
    $image = $enc->cipher($image, {'ek',unpack("H32",$u->f2var($X{ek})),'iv',unpack("H32",$u->f2var($X{iv}))},'true'); 
    if ($_out) {
      $u->fdump($X{out}, $image);
    }
}


$X{verify}?verify($X{in},$X{out}):build;
1;

#
