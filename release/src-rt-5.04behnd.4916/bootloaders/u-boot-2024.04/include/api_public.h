/* SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause */
/*
 * (C) Copyright 2007-2008 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

#ifndef _API_PUBLIC_H_
#define _API_PUBLIC_H_

#define API_EINVAL		1	/* invalid argument(s)	*/
#define API_ENODEV		2	/* no device		*/
#define API_ENOMEM		3	/* no memory		*/
#define API_EBUSY		4	/* busy, occupied etc.	*/
#define API_EIO			5	/* I/O error		*/
#define API_ESYSC		6	/* syscall error	*/

typedef	int (*scp_t)(int, int *, ...);

#define API_SIG_VERSION	1
#define API_SIG_MAGIC	"UBootAPI"
#define API_SIG_MAGLEN	8

struct api_signature {
	char		magic[API_SIG_MAGLEN];	/* magic string */
	uint16_t	version;		/* API version */
	uint32_t	checksum;		/* checksum of this sig struct */
	scp_t		syscall;		/* entry point to the API */
};

enum {
	API_RSVD = 0,
	API_GETC,
	API_PUTC,
	API_TSTC,
	API_PUTS,
	API_RESET,
	API_GET_SYS_INFO,
	API_UDELAY,
	API_GET_TIMER,
	API_DEV_ENUM,
	API_DEV_OPEN,
	API_DEV_CLOSE,
	API_DEV_READ,
	API_DEV_WRITE,
	API_ENV_ENUM,
	API_ENV_GET,
	API_ENV_SET,
	API_DISPLAY_GET_INFO,
	API_DISPLAY_DRAW_BITMAP,
	API_DISPLAY_CLEAR,
	API_MAXCALL
};

#define MR_ATTR_FLASH	0x0001
#define MR_ATTR_DRAM	0x0002
#define MR_ATTR_SRAM	0x0003

struct mem_region {
	unsigned long	start;
	unsigned long	size;
	int		flags;
};

struct sys_info {
	unsigned long		clk_bus;
	unsigned long		clk_cpu;
	unsigned long		bar;
	struct mem_region	*mr;
	int			mr_no;	/* number of memory regions */
};

/*
 * FIXME: Previously this code was:
 *
 *   #undef CONFIG_SYS_64BIT_LBA
 *   #ifdef CONFIG_SYS_64BIT_LBA
 *   typedef u_int64_t lbasize_t;
 *   #else
 *   typedef unsigned long lbasize_t;
 *   #endif
 *
 * But we cannot just undefine CONFIG_SYS_64BIT_LBA, because then in
 * api/api_storage.c the type signature of lbaint_t will be different if
 * CONFIG_SYS_64BIT_LBA is enabled for the board, which can result in various
 * bugs.
 * So simply define lbasize_t as an unsigned long, since this was what was done
 * anyway for at least 13 years, but don't undefine CONFIG_SYS_64BIT_LBA.
 */
typedef unsigned long lbasize_t;

typedef unsigned long lbastart_t;

#define DEV_TYP_NONE	0x0000
#define DEV_TYP_NET	0x0001

#define DEV_TYP_STOR	0x0002
#define DT_STOR_IDE	0x0010
#define DT_STOR_SCSI	0x0020
#define DT_STOR_USB	0x0040
#define DT_STOR_MMC	0x0080
#define DT_STOR_SATA	0x0100

#define DEV_STA_CLOSED	0x0000		/* invalid, closed */
#define DEV_STA_OPEN	0x0001		/* open i.e. active */

struct device_info {
	int	type;
	void	*cookie;

	union {
		struct {
			lbasize_t	block_count;	/* no of blocks */
			unsigned long	block_size;	/* size of one block */
		} storage;

		struct {
			unsigned char	hwaddr[6];
		} net;
	} info;
#define di_stor info.storage
#define di_net info.net

	int	state;
};

#define DISPLAY_TYPE_LCD	0x0001
#define DISPLAY_TYPE_VIDEO	0x0002

struct display_info {
	int type;
	/* screen size in pixels */
	int pixel_width;
	int pixel_height;
	/* screen size in rows and columns of text */
	int screen_rows;
	int screen_cols;
};

#endif /* _API_PUBLIC_H_ */
