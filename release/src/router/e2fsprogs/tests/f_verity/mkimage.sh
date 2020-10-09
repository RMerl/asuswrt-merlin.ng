#!/bin/bash

# This is the script that was used to create the image.gz in this directory.

set -e -u

mkdir -p mnt
umount mnt &> /dev/null || true

dd if=/dev/zero of=image bs=4096 count=128
mke2fs -O 'verity,extents' -b 4096 -N 128 image
mount image mnt

# Create a verity file, but make it fragmented so that it needs at least one
# extent tree index node, in order to cover the scan_extent_node() case.
for i in {1..80}; do
	head -c 4096 /dev/zero > mnt/tmp_$i
done
for i in {1..80..2}; do
	rm mnt/tmp_$i
done
head -c $((40 * 4096)) /dev/zero > mnt/file
fsverity enable mnt/file
rm mnt/tmp_*

umount mnt
rmdir mnt
gzip -9 -f image
