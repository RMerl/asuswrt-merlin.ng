#!/bin/bash

# This is the script that was used to create the image.gz in this directory.

set -e -u

BLOCKSIZE=4096

do_debugfs() {
	umount mnt
	debugfs -w "$@" image
	mount image mnt
}

do_tune2fs() {
	umount mnt
	tune2fs $@ image
	mount image mnt
}

symlink() {
	local len=$1
	local src=$2
	local target=$(perl -e 'print "A" x '$len)
	ln -s $target $src
	stat -c %i $src
}

# Overwrite the length in the header of the encrypted symlink target
set_encrypted_symlink_len() {
	local ino=$1
	local len=$2

	echo "zap_block -f <$ino> -p $((len%256)) -o 0 -l 1 0"
	echo "zap_block -f <$ino> -p $((len/256)) -o 1 -l 1 0"
}

create_symlinks() {
	local dir=$1
	local encrypted=${2:-false}
	local overhead=0
	local ino

	if $encrypted; then
		overhead=2
	fi

	mkdir -p $dir

	{
	ino=$(symlink 1 $dir/empty)
	echo "set_inode_field <$ino> i_size 10"
	echo "set_inode_field <$ino> block[0] 0"

	symlink 1 $dir/fast_min > /dev/null

	ino=$(symlink 10 $dir/fast_isize_too_small)
	echo "set_inode_field <$ino> i_size 1"

	ino=$(symlink 10 $dir/fast_isize_too_large)
	echo "set_inode_field <$ino> i_size 20"

	symlink $((59 - overhead)) $dir/fast_max > /dev/null

	symlink $((60 - overhead)) $dir/slow_min > /dev/null

	ino=$(symlink 100 $dir/slow_isize_too_small)
	echo "set_inode_field <$ino> i_size 80"

	ino=$(symlink 100 $dir/slow_isize_too_large)
	echo "set_inode_field <$ino> i_size 120"

	symlink $((BLOCKSIZE - 1 - overhead)) $dir/slow_max > /dev/null

	ino=$(symlink $((BLOCKSIZE - 1 - overhead)) $dir/one_too_long)
	echo "set_inode_field <$ino> i_size $BLOCKSIZE"
	echo "zap_block -f <$ino> -p 65 0"
	if $encrypted; then
		set_encrypted_symlink_len $ino $((BLOCKSIZE - overhead))
	fi

	ino=$(symlink $((BLOCKSIZE - 1 - overhead)) $dir/too_long)
	echo "set_inode_field <$ino> i_size $((BLOCKSIZE + 1000))"
	echo "zap_block -f <$ino> -p 65 0"
	if $encrypted; then
		set_encrypted_symlink_len $ino $((BLOCKSIZE + 1000 - overhead))
	fi

	} >> debugfs_commands
	do_debugfs < debugfs_commands
}

create_encrypted_symlinks() {
	local dir=$1 link

	mkdir $dir
	echo | e4crypt add_key $dir
	create_symlinks $dir true

	# Move symlinks into an unencrypted directory (leaving their targets
	# encrypted).  This makes the fsck output consistent.
	mv $dir ${dir}~encrypted
	mkdir $dir
	mv ${dir}~encrypted/* $dir
}

mkdir -p mnt
umount mnt &> /dev/null || true
dd if=/dev/zero of=image bs=1024 count=600

mke2fs -O 'encrypt,^extents,^64bit' -b $BLOCKSIZE -I 256 image
mount image mnt

create_symlinks mnt/default
create_encrypted_symlinks mnt/encrypted

do_tune2fs -O extents
create_symlinks mnt/extents
create_encrypted_symlinks mnt/extents_encrypted

do_debugfs -R 'feature inline_data'
create_symlinks mnt/inline_data

rm -rf debugfs_commands mnt/*~encrypted
umount mnt
rmdir mnt
gzip -9 -f image
