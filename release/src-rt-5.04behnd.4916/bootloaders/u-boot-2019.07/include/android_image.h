/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * This is from the Android Project,
 * Repository: https://android.googlesource.com/platform/system/core/
 * File: mkbootimg/bootimg.h
 * Commit: d162828814b08ada310846a33205befb69ef5799
 *
 * Copyright (C) 2008 The Android Open Source Project
 */

#ifndef _ANDROID_IMAGE_H_
#define _ANDROID_IMAGE_H_

typedef struct andr_img_hdr andr_img_hdr;

#define ANDR_BOOT_MAGIC "ANDROID!"
#define ANDR_BOOT_MAGIC_SIZE 8
#define ANDR_BOOT_NAME_SIZE 16
#define ANDR_BOOT_ARGS_SIZE 512
#define ANDR_BOOT_EXTRA_ARGS_SIZE 1024

struct andr_img_hdr {
	char magic[ANDR_BOOT_MAGIC_SIZE];

	u32 kernel_size;	/* size in bytes */
	u32 kernel_addr;	/* physical load addr */

	u32 ramdisk_size;	/* size in bytes */
	u32 ramdisk_addr;	/* physical load addr */

	u32 second_size;	/* size in bytes */
	u32 second_addr;	/* physical load addr */

	u32 tags_addr;		/* physical addr for kernel tags */
	u32 page_size;		/* flash page size we assume */
	u32 unused;		/* reserved for future expansion: MUST be 0 */

	/* operating system version and security patch level; for
	 * version "A.B.C" and patch level "Y-M-D":
	 * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
	 * os_version = ver << 11 | lvl */
	u32 os_version;

	char name[ANDR_BOOT_NAME_SIZE]; /* asciiz product name */

	char cmdline[ANDR_BOOT_ARGS_SIZE];

	u32 id[8]; /* timestamp / checksum / sha1 / etc */

	/* Supplemental command line data; kept here to maintain
	 * binary compatibility with older versions of mkbootimg */
	char extra_cmdline[ANDR_BOOT_EXTRA_ARGS_SIZE];
} __attribute__((packed));

/*
 * +-----------------+
 * | boot header     | 1 page
 * +-----------------+
 * | kernel          | n pages
 * +-----------------+
 * | ramdisk         | m pages
 * +-----------------+
 * | second stage    | o pages
 * +-----------------+
 *
 * n = (kernel_size + page_size - 1) / page_size
 * m = (ramdisk_size + page_size - 1) / page_size
 * o = (second_size + page_size - 1) / page_size
 *
 * 0. all entities are page_size aligned in flash
 * 1. kernel and ramdisk are required (size != 0)
 * 2. second is optional (second_size == 0 -> no second)
 * 3. load each element (kernel, ramdisk, second) at
 *    the specified physical address (kernel_addr, etc)
 * 4. prepare tags at tag_addr.  kernel_args[] is
 *    appended to the kernel commandline in the tags.
 * 5. r0 = 0, r1 = MACHINE_TYPE, r2 = tags_addr
 * 6. if second_size != 0: jump to second_addr
 *    else: jump to kernel_addr
 */
#endif
