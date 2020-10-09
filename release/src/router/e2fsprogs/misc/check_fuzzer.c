/*
 * Play with a file system image quickly to find UBSAN problems
 *
 * Run a file system through some of the libext2fs functions used by
 * some fuzzer reports.
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
	int		exit_status = 1;

	initialize_ext2_error_table();

	if (argc != 2) {
		fprintf(stderr, "%s: Usage <device|filesystem>\n", argv[0]);
		exit(1);
	}

	retval = ext2fs_open(argv[1], 0, 0, 0,
			     unix_io_manager, &fs);
	if (retval) {
		com_err(argv[0], retval, "while trying to open '%s'",
			argv[1]);
		exit(1);
	}

	retval = ext2fs_read_inode_bitmap(fs);
	if (retval) {
		com_err(argv[0], retval, "while trying to read inode bitmaps");
		goto errout;
	}

	retval = ext2fs_read_block_bitmap(fs);
	if (retval) {
		com_err(argv[0], retval, "while trying to read inode bitmaps");
		goto errout;
	}

	retval = ext2fs_check_directory(fs, EXT2_ROOT_INO);
	if (retval) {
		com_err(argv[0], retval, "while trying to read inode bitmaps");
		goto errout;
	}
	exit_status = 0;
errout:
	ext2fs_close(fs);
	return exit_status;
}
