/*
 * Basic program to add ext4 encryption to a file system
 *
 * Copyright 2015, Google, Inc.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <ext2fs/ext2_fs.h>
#include <ext2fs/ext2fs.h>

int main (int argc, char *argv[])
{
	errcode_t	retval = 0;
	ext2_filsys	fs;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	initialize_ext2_error_table();

	if (argc != 2) {
		fprintf(stderr, "%s: Usage <device|filesystem>\n", argv[0]);
		exit(1);
	}

	retval = ext2fs_open(argv[1], EXT2_FLAG_RW, 0, 0,
			     unix_io_manager, &fs);

	if (retval) {
		com_err(argv[0], retval, "while trying to open '%s'",
			argv[1]);
		exit(1);
	}
	if (!ext2fs_has_feature_encrypt(fs->super)) {
		ext2fs_set_feature_encrypt(fs->super);
		fs->super->s_encrypt_algos[0] =
			EXT4_ENCRYPTION_MODE_AES_256_XTS;
		fs->super->s_encrypt_algos[1] =
			EXT4_ENCRYPTION_MODE_AES_256_CTS;
		ext2fs_mark_super_dirty(fs);
		printf("Ext4 encryption enabled on %s\n", argv[1]);
	} else
		printf("Ext4 encryption already enabled on %s\n", argv[1]);

	retval = ext2fs_close(fs);
	if (retval) {
		com_err(argv[0], retval, "while trying to close '%s'",
			argv[1]);
		exit(1);
	}
	return (0);
}

