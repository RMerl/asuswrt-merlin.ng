/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 */

#ifndef _FIT_COMMON_H_
#define _FIT_COMMON_H_

#include "imagetool.h"
#include "mkimage.h"
#include <image.h>

/**
 * Verify the format of FIT header pointed to by ptr
 *
 * @ptr: image header to be verified
 * @image_size: size of while image
 * @params: mkimage parameters
 * @return 0 if OK, -1 on error
 */
int fit_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params);

int fit_check_image_types(uint8_t type);

/**
 * Map an FDT into memory, optionally increasing its size
 *
 * @cmdname:	Tool name (for displaying with error messages)
 * @fname:	Filename containing FDT
 * @size_inc:	Amount to increase size by (0 = leave it alone)
 * @blobp:	Returns pointer to FDT blob
 * @sbuf:	File status information is stored here
 * @delete_on_error:	true to delete the file if we get an error
 * @read_only:	true to open in read-only mode
 * @return 0 if OK, -1 on error.
 */
int mmap_fdt(const char *cmdname, const char *fname, size_t size_inc,
	     void **blobp, struct stat *sbuf, bool delete_on_error,
	     bool read_only);

#endif /* _FIT_COMMON_H_ */
