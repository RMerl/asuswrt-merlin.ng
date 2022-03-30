// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * DENX Software Engineering
 * Heiko Schocher <hs@denx.de>
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * Updated-by: Prafulla Wadaskar <prafulla@marvell.com>
 *		FIT image specific code abstracted from mkimage.c
 *		some functions added to address abstraction
 *
 * All rights reserved.
 */

#include "imagetool.h"
#include "mkimage.h"
#include "fit_common.h"
#include <image.h>
#include <u-boot/crc.h>

int fit_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	if (fdt_check_header(ptr) != EXIT_SUCCESS || !fit_check_format(ptr))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int fit_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_FLATDT)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

int mmap_fdt(const char *cmdname, const char *fname, size_t size_inc,
	     void **blobp, struct stat *sbuf, bool delete_on_error,
	     bool read_only)
{
	void *ptr;
	int fd;

	/* Load FIT blob into memory (we need to write hashes/signatures) */
	fd = open(fname, (read_only ? O_RDONLY : O_RDWR) | O_BINARY);

	if (fd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			cmdname, fname, strerror(errno));
		goto err;
	}

	if (fstat(fd, sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			cmdname, fname, strerror(errno));
		goto err;
	}

	if (size_inc) {
		sbuf->st_size += size_inc;
		if (ftruncate(fd, sbuf->st_size)) {
			fprintf(stderr, "%s: Can't expand %s: %s\n",
				cmdname, fname, strerror(errno));
		goto err;
		}
	}

	errno = 0;
	ptr = mmap(0, sbuf->st_size,
		   (read_only ? PROT_READ : PROT_READ | PROT_WRITE), MAP_SHARED,
		   fd, 0);
	if ((ptr == MAP_FAILED) || (errno != 0)) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			cmdname, fname, strerror(errno));
		goto err;
	}

	/* check if ptr has a valid blob */
	if (fdt_check_header(ptr)) {
		fprintf(stderr, "%s: Invalid FIT blob\n", cmdname);
		goto err;
	}

	/* expand if needed */
	if (size_inc) {
		int ret;

		ret = fdt_open_into(ptr, ptr, sbuf->st_size);
		if (ret) {
			fprintf(stderr, "%s: Cannot expand FDT: %s\n",
				cmdname, fdt_strerror(ret));
			goto err;
		}
	}

	*blobp = ptr;
	return fd;

err:
	if (fd >= 0)
		close(fd);
	if (delete_on_error)
		unlink(fname);

	return -1;
}
