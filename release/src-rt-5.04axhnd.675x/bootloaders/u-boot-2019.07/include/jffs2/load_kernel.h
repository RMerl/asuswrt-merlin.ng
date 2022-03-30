/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef load_kernel_h
#define load_kernel_h
/*-------------------------------------------------------------------------
 * Filename:      load_kernel.h
 * Version:       $Id: load_kernel.h,v 1.3 2002/01/25 01:34:11 nyet Exp $
 * Copyright:     Copyright (C) 2001, Russ Dill
 * Author:        Russ Dill <Russ.Dill@asu.edu>
 * Description:   header for load kernel modules
 *-----------------------------------------------------------------------*/

#include <linux/list.h>

/* mtd device types */
#define MTD_DEV_TYPE_NOR	0x0001
#define MTD_DEV_TYPE_NAND	0x0002
#define MTD_DEV_TYPE_ONENAND	0x0004
#define MTD_DEV_TYPE_SPINAND	0x0008

#define MTD_DEV_TYPE(type) (type == MTD_DEV_TYPE_NAND ? "nand" :	\
			    (type == MTD_DEV_TYPE_NOR ? "nor" :		\
			     (type == MTD_DEV_TYPE_ONENAND ? "onenand" : \
			      "spi-nand")))				\

struct mtd_device {
	struct list_head link;
	struct mtdids *id;		/* parent mtd id entry */
	u16 num_parts;			/* number of partitions on this device */
	struct list_head parts;		/* partitions */
};

struct part_info {
	struct list_head link;
	char *name;			/* partition name */
	u8 auto_name;			/* set to 1 for generated name */
	u64 size;			/* total size of the partition */
	u64 offset;			/* offset within device */
	void *jffs2_priv;		/* used internaly by jffs2 */
	u32 mask_flags;			/* kernel MTD mask flags */
	u32 sector_size;		/* size of sector */
	struct mtd_device *dev;		/* parent device */
};

struct mtdids {
	struct list_head link;
	u8 type;			/* device type */
	u8 num;				/* device number */
	u64 size;			/* device size */
	char *mtd_id;			/* linux kernel device id */
};

#define ldr_strlen	strlen
#define ldr_strncmp	strncmp
#define ldr_memcpy	memcpy
#define putstr(x)	printf("%s", x)
#define mmalloc		malloc
#define UDEBUG		printf

#define putnstr(str, size)	printf("%*.*s", size, size, str)
#define ldr_output_string(x)	puts(x)
#define putLabeledWord(x, y)	printf("%s %08x\n", x, (unsigned int)y)
#define led_blink(x, y, z, a)

/* common/cmd_jffs2.c */
extern int mtdparts_init(void);
extern int find_dev_and_part(const char *id, struct mtd_device **dev,
				u8 *part_num, struct part_info **part);
extern struct mtd_device *device_find(u8 type, u8 num);

#endif /* load_kernel_h */
