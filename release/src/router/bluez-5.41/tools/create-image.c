/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <inttypes.h>

/*
 * The "new" ASCII format uses 8-byte hexadecimal fields for all numbers and
 * separates device numbers into separate fields for major and minor numbers.
 *
 *	struct cpio_newc_header {
 *		char    c_magic[6];
 *		char    c_ino[8];
 *		char    c_mode[8];
 *		char    c_uid[8];
 *		char    c_gid[8];
 *		char    c_nlink[8];
 *		char    c_mtime[8];
 *		char    c_filesize[8];
 *		char    c_devmajor[8];
 *		char    c_devminor[8];
 *		char    c_rdevmajor[8];
 *		char    c_rdevminor[8];
 *		char    c_namesize[8];
 *		char    c_check[8];
 *	};
 *
 */

#define HDR_FMT "%s%08X%08X%08X%08X%08X%08X%08jX%08X%08X%08X%08X%08X%08X%s"

#define HDR_MAGIC "070701"

static unsigned int ino_cnt = 721;

#define REG_EXE	S_IFREG | \
		S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

static const struct {
	const char *source;
	const char *target;
	mode_t mode;
} file_list[] = {
	{ "tools/test-runner", "init", REG_EXE },
	{ }
};

static void write_block(FILE *fp, const char *pathname, unsigned int ino,
						mode_t mode, const char *name)
{
	int i, pad, namelen = strlen(name);
	struct stat st;
	void *map;
	int fd;

	if (!pathname) {
		fd = -1;
		map = NULL;
		st.st_size = 0;
		goto done;
	}

	fd = open(pathname, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		fd = -1;
		map = NULL;
		st.st_size = 0;
		goto done;
	}

	if (fstat(fd, &st) < 0) {
		close(fd);
		fd = -1;
		map = NULL;
		st.st_size = 0;
		goto done;
	}

	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!map || map == MAP_FAILED) {
		close(fd);
		fd = -1;
		map = NULL;
		st.st_size = 0;
        }

done:
	fprintf(fp, HDR_FMT, HDR_MAGIC, ino, mode, 0, 0, 1, 0,
		(uintmax_t) st.st_size, 0, 0, 0, 0, namelen + 1, 0, name);

	pad = 4 - ((110 + namelen) % 4);
	for (i = 0; i < pad; i++)
		fputc(0, fp);

	if (st.st_size > 0) {
		fwrite(map, st.st_size, 1, fp);

		pad = 3 - ((st.st_size + 3) % 4);
		for (i = 0; i < pad; i++)
			fputc(0, fp);

		munmap(map, st.st_size);
		close(fd);
	}
}

static void usage(void)
{
	printf("create-image - CPIO image creation utility\n"
		"Usage:\n");
	printf("\tcreate-image [options]\n");
	printf("Options:\n"
		"\t-o, --output <image>   Output CPIO image\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "output",  required_argument, NULL, 'o' },
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	const char *output_pathname = NULL;
	FILE *fp;
	int i;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "o:vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'o':
			output_pathname = optarg;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	if (!output_pathname) {
		fprintf(stderr, "Failed to specify output file\n");
		return EXIT_FAILURE;
	}

	fp = fopen(output_pathname, "we");

	for (i = 0; file_list[i].source; i++)
		write_block(fp, file_list[i].source, ino_cnt++,
				file_list[i].mode, file_list[i].target);

	write_block(fp, NULL, ino_cnt++, 0, "TRAILER!!!");

	fclose(fp);

	return EXIT_SUCCESS;
}
