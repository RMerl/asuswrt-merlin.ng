/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef __BCM_ATAG_H
#define __BCM_ATAG_H

/* The list ends with an ATAG_NONE node. */
#define ATAG_NONE	0x00000000

struct tag_header {
	uint32_t size;
	uint32_t tag;
};

/* The list must start with an ATAG_CORE node */
#define ATAG_CORE	0x54410001

#define ATAG_CORE_DEF_PAGESIZE	0x1000
struct tag_core {
	uint32_t flags;		/* bit 0 = read-only */
	uint32_t pagesize;
	uint32_t rootdev;
};

/* it is allowed to have multiple ATAG_MEM nodes */
#define ATAG_MEM	0x54410002

struct tag_mem32 {
	uint32_t	size;
	uint32_t	start;	/* physical start address */
};

/* VGA text type displays */
#define ATAG_VIDEOTEXT	0x54410003

struct tag_videotext {
	uint8_t		x;
	uint8_t		y;
	uint16_t	video_page;
	uint8_t		video_mode;
	uint8_t		video_cols;
	uint16_t	video_ega_bx;
	uint8_t		video_lines;
	uint8_t		video_isvga;
	uint16_t	video_points;
};

/* describes how the ramdisk will be used in kernel */
#define ATAG_RAMDISK	0x54410004

struct tag_ramdisk {
	uint32_t flags;	/* bit 0 = load, bit 1 = prompt */
	uint32_t size;	/* decompressed ramdisk size in _kilo_ bytes */
	uint32_t start;	/* starting block of floppy-based RAM disk image */
};

/* describes where the compressed ramdisk image lives (virtual address) */
/*
 * this one accidentally used virtual addresses - as such,
 * it's deprecated.
 */
#define ATAG_INITRD	0x54410005

/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2	0x54420005

struct tag_initrd {
	uint32_t start;	/* physical start address */
	uint32_t size;	/* size of compressed ramdisk image in bytes */
};

/* board serial number. "64 bits should be enough for everybody" */
#define ATAG_SERIAL	0x54410006

struct tag_serialnr {
	uint32_t low;
	uint32_t high;
};

/* board revision */
#define ATAG_REVISION	0x54410007

struct tag_revision {
	uint32_t rev;
};

/* initial values for vesafb-type framebuffers. see struct screen_info
 * in include/linux/tty.h
 */
#define ATAG_VIDEOLFB	0x54410008

struct tag_videolfb {
	uint16_t		lfb_width;
	uint16_t		lfb_height;
	uint16_t		lfb_depth;
	uint16_t		lfb_linelength;
	uint32_t		lfb_base;
	uint32_t		lfb_size;
	uint8_t			red_size;
	uint8_t			red_pos;
	uint8_t			green_size;
	uint8_t			green_pos;
	uint8_t			blue_size;
	uint8_t			blue_pos;
	uint8_t			rsvd_size;
	uint8_t			rsvd_pos;
};

/* command line: \0 terminated string */
#define ATAG_CMDLINE	0x54410009

struct tag_cmdline {
	char	cmdline[1];	/* this is the minimum size */
};

#define ATAG_BLPARM	0x41000601
#define BLPARM_SIZE	1024

struct tag_blparm {
	char	blparm[1];	/* this is the minimum size */
};

#define ATAG_RDPSIZE	0x41000602

struct tag_rdpsize {
	uint32_t tm_size;
	uint32_t mc_size;
};

#define ATAG_DHDSIZE	0x41000603

struct tag_dhdparm {
	uint32_t dhd_size[3];
};


struct tag {
	struct tag_header hdr;
	union {
		struct tag_core		core;
		struct tag_mem32	mem;
		struct tag_videotext	videotext;
		struct tag_ramdisk	ramdisk;
		struct tag_initrd	initrd;
		struct tag_serialnr	serialnr;
		struct tag_revision	revision;
		struct tag_videolfb	videolfb;
		struct tag_cmdline	cmdline;
		struct tag_blparm	blparm;
		struct tag_rdpsize	rdpsize;
		struct tag_dhdparm      dhdparm;
	} u;
};

#define tag_next(t)	((struct tag *)((uint32_t *)(t) + (t)->hdr.size))
#define tag_size(type)	((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

#define for_each_tag(t,base)		\
	for (t = base; t->hdr.size; t = tag_next(t))

/* this one has to be called first */
struct tag *add_coretag(struct tag *tag, uint32_t flags, uint32_t pagesize, uint32_t rootdev);

struct tag *add_memtag(struct tag *tag, uint32_t start, uint32_t size);
struct tag *add_ramdisktag(struct tag *tag, uint32_t flags, uint32_t size, uint32_t start);

/* phys == 0, virt otherwise */
struct tag *add_initrdtag(struct tag *tag, uint32_t start, uint32_t size, int phys_or_virt);
struct tag *add_serialtag(struct tag *tag, uint32_t low, uint32_t high);
struct tag *add_revisiontag(struct tag *tag, uint32_t rev);
struct tag *add_cmdlinetag(struct tag *tag, char *cmdline_ptr, int cmdline_len);
struct tag *add_blparmtag(struct tag *tag, char *blparm_ptr, int blparm_len);
struct tag *add_rdpsizetag(struct tag *tag, uint32_t tm_size, uint32_t mc_size);
struct tag *add_dhdsizetag(struct tag *tag, uint8_t dhd_size[]);

/* this one must be called last */
struct tag *complete_tag(struct tag *tag);

#endif /* __BCM_ATAG_H */

