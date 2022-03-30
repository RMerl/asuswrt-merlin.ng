#!/usr/bin/env perl

use File::Temp;

my $dir    = File::Temp->newdir("/tmp/image_encrypt_XXXXX");
my $tmpdir = $dir->dirname;
print $tmpdir;

system(qq[dd if=/dev/random of=$tmpdir/rootfs_key.bin bs=1 count=32]);
system(qq[cp obj/binaries/rootfs.squashfs $tmpdir/rootfs.img]);
if (system(qq[sudo /opt/local/encrypt_rootfs $tmpdir]) != 0) {
  	die(q[
*******************************************************************
You need to install  hostTools/imagetools/encrypt_rootfs
to /opt/local/encrypt_rootfs, owned by root, and enable sudo for it
example:
   ALL             ALL=NOPASSWD:     /opt/local/encrypt_rootfs
*******************************************************************
	launch of encrypt_rootfs failed with code ] . $?);
}
my $elen = ( stat("$tmpdir/rootfs.enc") )[7];
if (system(
qq[veritysetup format --data-block-size=512 --hash-block-size=512 --hash-offset $elen $tmpdir/rootfs.enc $tmpdir/rootfs.enc > obj/binaries/verity.out]
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
system(qq[cp $tmpdir/rootfs.enc obj/binaries/]);
system(qq[cp $tmpdir/rootfs_key.bin obj/binaries/]);

my $dblocks   = $v{'data blocks'};
my $hash      = $v{'root hash'};
my $salt      = $v{'salt'};
my $hashstart = $dblocks + 1;
open( DM, "<$tmpdir/dm.txt" );
my $dm = <DM>;
chomp $dm;
close(DM);
$dm =~ s| \d+:\d+ | /dev/dm-0 |;
my $mapper = "/dev/dm-1:lroot,,,ro, 0 $dblocks verity 1 %DEVICE% %DEVICE% 512 512 $dblocks $hashstart sha256 $hash $salt 1 check_at_most_once ; lroot2,,,ro,$dm";

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
