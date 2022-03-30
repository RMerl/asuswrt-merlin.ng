#!/bin/bash

tmpdir=/tmp/image_encrypt_$$
mkdir $tmpdir
dd if=/dev/random of=$tmpdir/rootfs_key.bin bs=1 count=32
cp obj/binaries/rootfs.squashfs $tmpdir/rootfs.img
if ! sudo /opt/local/encrypt_rootfs $tmpdir
then
  echo "*******************************************************************" 
  echo "You need to install  hostTools/imagetools/encrypt_rootfs"
  echo "to /opt/local/encrypt_rootfs, owned by root, and enable sudo for it"
  echo "example:"
  echo "   ALL             ALL=NOPASSWD:     /opt/local/encrypt_rootfs"
  echo "*******************************************************************" 
  exit 1
fi

elen=`stat --format='%s' $tmpdir/rootfs.enc`
cp $tmpdir/rootfs.enc obj/binaries/
cp $tmpdir/rootfs_key.bin obj/binaries/
echo -n "/dev/dm-0:lroot,,,ro," | cat - $tmpdir/dm.txt > obj/binaries/dm.txt



