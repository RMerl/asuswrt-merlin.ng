#!/bin/bash

if [ -z "$1" -o -z "$2" ]; then
	echo "Usage: $0 kernel-file e2fsprogs-file"
	exit 0
fi

# Transform a few things to fit the compatibility things defined in jfs_user.h.
# Use the ext2fs_ endian conversion functions because they truncate oversized
# inputs (e.g. passing a u32 to cpu_to_be16()) like the kernel versions and
# unlike the libc6 versions.
exec sed -e 's/JBD_/JFS_/g' \
	 -e 's/JBD2_/JFS_/g' \
	 -e 's/jbd2_journal_/journal_/g' \
	 -e 's/__be/__u/g' \
	 -e 's/struct kmem_cache/lkmem_cache_t/g' \
	 -e 's/cpu_to_be/ext2fs_cpu_to_be/g' \
	 -e 's/be\([0-9][0-9]\)_to_cpu/ext2fs_be\1_to_cpu/g' \
	 < "$1" > "$2"
