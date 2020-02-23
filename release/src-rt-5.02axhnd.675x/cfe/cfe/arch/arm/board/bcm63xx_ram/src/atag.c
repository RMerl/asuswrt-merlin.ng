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

#include "cfe.h"
#include "lib_types.h"
#include "lib_string.h"
#include "atag.h"

/* this one has to be called first */
struct tag *add_coretag(struct tag *tag, uint32_t flags, uint32_t pagesize, uint32_t rootdev)
{
	tag->hdr.tag  = ATAG_CORE;
	tag->hdr.size = tag_size(tag_core);
	tag->u.core.flags = flags;
	tag->u.core.pagesize = pagesize;
	tag->u.core.rootdev = rootdev;

	return tag;
}

struct tag *add_memtag(struct tag *tag, uint32_t start, uint32_t size)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_MEM;
	tag->hdr.size = tag_size(tag_mem32);
	tag->u.mem.size = size;
	tag->u.mem.start = start;
	return tag;
}

struct tag *add_ramdisktag(struct tag *tag, uint32_t flags, uint32_t size, uint32_t start)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_RAMDISK;
	tag->hdr.size = tag_size(tag_ramdisk);
	tag->u.ramdisk.flags = flags;
	tag->u.ramdisk.size  = size;
	tag->u.ramdisk.start = start;
#if 0
	tag->u.ramdisk.flags = (params->u1.s.flags & FLAG_RDLOAD ? 1 : 0) |
		(params->u1.s.flags & FLAG_RDPROMPT ? 2 : 0);
	tag->u.ramdisk.size  = params->u1.s.ramdisk_size;
	tag->u.ramdisk.start = params->u1.s.rd_start;
#endif

	return tag;
}

/* phys == 0, virt otherwise */
struct tag *add_initrdtag(struct tag *tag, uint32_t start, uint32_t size, int phys_or_virt)
{
	tag = tag_next(tag);
	if (phys_or_virt == 0)
		tag->hdr.tag = ATAG_INITRD2;
	else
		tag->hdr.tag = ATAG_INITRD;
	tag->hdr.size = tag_size(tag_initrd);
	tag->u.initrd.start = start;
	tag->u.initrd.size  = size;

	return tag;
}

struct tag *add_serialtag(struct tag *tag, uint32_t low, uint32_t high)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_SERIAL;
	tag->hdr.size = tag_size(tag_serialnr);
	tag->u.serialnr.low = low;
	tag->u.serialnr.high = high;

	return tag;
}

struct tag *add_revisiontag(struct tag *tag, uint32_t rev)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_REVISION;
	tag->hdr.size = tag_size(tag_revision);
	tag->u.revision.rev = rev;

	return tag;
}

struct tag *add_cmdlinetag(struct tag *tag, char *cmdline_ptr, int cmdline_len)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_CMDLINE;
	tag->hdr.size = (cmdline_len + 3 + sizeof(struct tag_header)) >> 2;
	memcpy(tag->u.cmdline.cmdline, cmdline_ptr, cmdline_len);

	return tag;
}

struct tag *add_blparmtag(struct tag *tag, char *blparm_ptr, int blparm_len)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_BLPARM;
	tag->hdr.size = (blparm_len + 3 + sizeof(struct tag_header)) >> 2;
	memcpy(tag->u.blparm.blparm, blparm_ptr, blparm_len);

	return tag;
}

struct tag *add_rdpsizetag(struct tag *tag, uint32_t tm_size, uint32_t mc_size)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_RDPSIZE;
	tag->hdr.size = tag_size(tag_rdpsize);
	tag->u.rdpsize.tm_size = tm_size;
	tag->u.rdpsize.mc_size = mc_size;

	return tag;
}

struct tag *add_dhdsizetag(struct tag *tag, uint8_t dhd_size[])
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_DHDSIZE;
	tag->hdr.size = tag_size(tag_dhdparm);
	tag->u.dhdparm.dhd_size[0] = dhd_size[0];
	tag->u.dhdparm.dhd_size[1] = dhd_size[1];
	tag->u.dhdparm.dhd_size[2] = dhd_size[2];

	return tag;


}

/* this one must be called last */
struct tag *complete_tag(struct tag *tag)
{
	tag = tag_next(tag);
	tag->hdr.tag = ATAG_NONE;
	tag->hdr.size = 0;
	return tag;
}

