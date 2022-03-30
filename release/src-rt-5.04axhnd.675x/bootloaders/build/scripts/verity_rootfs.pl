#!/usr/bin/env perl

use File::Temp;

my $dir    = File::Temp->newdir("/tmp/image_encrypt_XXXXX");
my $tmpdir = $dir->dirname;
print $tmpdir;

my $elen = ( stat("obj/binaries/rootfs.squashfs") )[7];
system(qq[cp obj/binaries/rootfs.squashfs obj/binaries/rootfs.enc]);
if (system(
qq[veritysetup format --data-block-size=512 --hash-block-size=512 --hash-offset $elen obj/binaries/rootfs.enc obj/binaries/rootfs.enc > obj/binaries/verity.out]
) != 0) {
        die("veritysetup FORMAT failed");
}
my %v;
open( V, "<obj/binaries/verity.out" );

while ( my $l = <V> ) {
    chomp($l);
    if ( $l =~ /^(.*):\s*(\S.*)$/ ) {
        $v{ lc($1) } = $2;
    }
}

my $dblocks   = $v{'data blocks'};
my $hash      = $v{'root hash'};
my $salt      = $v{'salt'};
my $hashstart = $dblocks + 1;
my $mapper = "/dev/dm-0:lroot,,,ro, 0 $dblocks verity 1 %DEVICE% %DEVICE% 512 512 $dblocks $hashstart sha256 $hash $salt 1 check_at_most_once";

open( DM, ">obj/binaries/dm.txt" );
print DM $mapper;
close(DM);

# Verify Hashes
open( DM_VERIFY, ">obj/binaries/verity_verify.cmd" );
my $vercmd = "veritysetup verify --hash-offset $elen obj/binaries/rootfs.enc obj/binaries/rootfs.enc $hash $salt --debug > obj/binaries/verity_verify.out";
print DM_VERIFY $vercmd ;
close(DM_VERIFY);

if (system(
qq[ $vercmd ]
) != 0) {
	die("./veritysetup VERIFY failed!");
} 
