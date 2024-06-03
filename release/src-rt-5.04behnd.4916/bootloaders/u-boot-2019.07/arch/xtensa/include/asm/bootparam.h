/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Definition of the Linux/Xtensa boot parameter structure
 *
 * Copyright (C) 2001 - 2009  Tensilica Inc.
 *
 * (Concept borrowed from the 68K port)
 */

#ifndef _XTENSA_BOOTPARAM_H
#define _XTENSA_BOOTPARAM_H

#define BP_VERSION 0x0001

#define BP_TAG_COMMAND_LINE	0x1001	/* command line (0-terminated string)*/
#define BP_TAG_INITRD		0x1002	/* ramdisk addr and size (bp_meminfo) */
#define BP_TAG_MEMORY		0x1003	/* memory addr and size (bp_meminfo) */
#define BP_TAG_SERIAL_BAUDRATE	0x1004	/* baud rate of current console */
#define BP_TAG_SERIAL_PORT	0x1005	/* serial device of current console */
#define BP_TAG_FDT		0x1006	/* flat device tree */

#define BP_TAG_FIRST		0x7B0B  /* first tag with a version number */
#define BP_TAG_LAST		0x7E0B	/* last tag */

#ifndef __ASSEMBLY__

/* All records are aligned to 4 bytes */

struct bp_tag {
	unsigned short id;	/* tag id */
	unsigned short size;	/* size of this record excluding the structure*/
	unsigned long data[0];	/* data */
};

#define bp_tag_next(tag)						\
	((struct bp_tag *)((unsigned long)((tag) + 1) + (tag)->size))

struct meminfo {
	unsigned long type;
	unsigned long start;
	unsigned long end;
};

#define MEMORY_TYPE_CONVENTIONAL     0x1000
#define MEMORY_TYPE_NONE             0x2000

struct sysmem_info {
	int nr_banks;
	struct meminfo bank[0];
};

#endif
#endif
