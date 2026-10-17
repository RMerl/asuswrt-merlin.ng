// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Raymond Lo, lo@routefree.com
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Support for harddisk partitions.
 *
 * To be compatible with LinuxPPC and Apple we use the standard Apple
 * SCSI disk partitioning scheme. For more information see:
 * http://developer.apple.com/techpubs/mac/Devices/Devices-126.html#MARKER-14-92
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <ide.h>
#include <memalign.h>
#include <asm/unaligned.h>
#include <linux/compiler.h>
#include "part_dos.h"
#include <part.h>

#define DOS_PART_DEFAULT_SECTOR 512

/* should this be configurable? It looks like it's not very common at all
 * to use large numbers of partitions */
#define MAX_EXT_PARTS 256

static inline int is_extended(int part_type)
{
    return (part_type == DOS_PART_TYPE_EXTENDED ||
	    part_type == DOS_PART_TYPE_EXTENDED_LBA ||
	    part_type == DOS_PART_TYPE_EXTENDED_LINUX);
}

static int get_bootable(dos_partition_t *p)
{
	int ret = 0;

	if (p->sys_ind == 0xef)
		ret |= PART_EFI_SYSTEM_PARTITION;
	if (p->boot_ind == 0x80)
		ret |= PART_BOOTABLE;
	return ret;
}

static void print_one_part(dos_partition_t *p, lbaint_t ext_part_sector,
			   int part_num, unsigned int disksig)
{
	lbaint_t lba_start = ext_part_sector + get_unaligned_le32(p->start4);
	lbaint_t lba_size  = get_unaligned_le32(p->size4);

	printf("%3d\t%-10" LBAFlength "u\t%-10" LBAFlength
		"u\t%08x-%02x\t%02x%s%s\n",
		part_num, lba_start, lba_size, disksig, part_num, p->sys_ind,
		(is_extended(p->sys_ind) ? " Extd" : ""),
		(get_bootable(p) ? " Boot" : ""));
}

static int test_block_type(unsigned char *buffer)
{
	int slot;
	struct dos_partition *p;
	int part_count = 0;

	if((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) ||
	    (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) ) {
		return (-1);
	} /* no DOS Signature at all */
	p = (struct dos_partition *)&buffer[DOS_PART_TBL_OFFSET];

	/* Check that the boot indicators are valid and count the partitions. */
	for (slot = 0; slot < 4; ++slot, ++p) {
		if (p->boot_ind != 0 && p->boot_ind != 0x80)
			break;
		if (p->sys_ind)
			++part_count;
	}

	/*
	 * If the partition table is invalid or empty,
	 * check if this is a DOS PBR
	 */
	if (slot != 4 || !part_count) {
		if (!strncmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET],
			     "FAT", 3) ||
		    !strncmp((char *)&buffer[DOS_PBR32_FSTYPE_OFFSET],
			     "FAT32", 5))
			return DOS_PBR; /* This is a DOS PBR and not an MBR */
	}
	if (slot == 4)
		return DOS_MBR;	/* This is an DOS MBR */

	/* This is neither a DOS MBR nor a DOS PBR */
	return -1;
}

static int part_test_dos(struct blk_desc *desc)
{
#ifndef CONFIG_SPL_BUILD
	ALLOC_CACHE_ALIGN_BUFFER(legacy_mbr, mbr,
			DIV_ROUND_UP(desc->blksz, sizeof(legacy_mbr)));

	if (blk_dread(desc, 0, 1, (ulong *)mbr) != 1)
		return -1;

	if (test_block_type((unsigned char *)mbr) != DOS_MBR)
		return -1;

	if (desc->sig_type == SIG_TYPE_NONE && mbr->unique_mbr_signature) {
		desc->sig_type = SIG_TYPE_MBR;
		desc->mbr_sig = mbr->unique_mbr_signature;
	}
#else
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, desc->blksz);

	if (blk_dread(desc, 0, 1, (ulong *)buffer) != 1)
		return -1;

	if (test_block_type(buffer) != DOS_MBR)
		return -1;
#endif

	return 0;
}

/*  Print a partition that is relative to its Extended partition table
 */
static void print_partition_extended(struct blk_desc *desc,
				     lbaint_t ext_part_sector,
				     lbaint_t relative,
				     int part_num, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, desc->blksz);
	dos_partition_t *pt;
	int i;

	/* set a maximum recursion level */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\n");
		return;
    }

	if (blk_dread(desc, ext_part_sector, 1, (ulong *)buffer) != 1) {
		printf ("** Can't read partition table on %d:" LBAFU " **\n",
			desc->devnum, ext_part_sector);
		return;
	}
	i=test_block_type(buffer);
	if (i != DOS_MBR) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return;
	}

	if (!ext_part_sector)
		disksig = get_unaligned_le32(&buffer[DOS_PART_DISKSIG_OFFSET]);

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */

		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
			print_one_part(pt, ext_part_sector, part_num, disksig);
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			lbaint_t lba_start
				= get_unaligned_le32 (pt->start4) + relative;

			print_partition_extended(desc, lba_start,
						 !ext_part_sector ? lba_start :
						 relative, part_num, disksig);
		}
	}

	return;
}


/*  Print a partition that is relative to its Extended partition table
 */
static int part_get_info_extended(struct blk_desc *desc,
				  lbaint_t ext_part_sector, lbaint_t relative,
				  int part_num, int which_part,
				  struct disk_partition *info, uint disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, desc->blksz);
	struct disk_partition wdinfo = { 0 };
	dos_partition_t *pt;
	int i, ret;
	int dos_type;

	/* set a maximum recursion level */
	if (part_num > MAX_EXT_PARTS)
	{
		printf("** Nested DOS partitions detected, stopping **\n");
		return -1;
    }

	if (blk_dread(desc, ext_part_sector, 1, (ulong *)buffer) != 1) {
		printf ("** Can't read partition table on %d:" LBAFU " **\n",
			desc->devnum, ext_part_sector);
		return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 ||
		buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return -1;
	}

	if (CONFIG_IS_ENABLED(PARTITION_UUIDS) && !ext_part_sector)
		disksig = get_unaligned_le32(&buffer[DOS_PART_DISKSIG_OFFSET]);

	ret = part_get_info_whole_disk(desc, &wdinfo);
	if (ret)
		return ret;

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */
		if (((pt->boot_ind & ~0x80) == 0) &&
		    (pt->sys_ind != 0) &&
		    (part_num == which_part) &&
		    (ext_part_sector == 0 || is_extended(pt->sys_ind) == 0)) {
			if (wdinfo.blksz > DOS_PART_DEFAULT_SECTOR)
				info->blksz = wdinfo.blksz;
			else
				info->blksz = DOS_PART_DEFAULT_SECTOR;
			info->start = (lbaint_t)(ext_part_sector +
					get_unaligned_le32(pt->start4));
			info->size  = (lbaint_t)get_unaligned_le32(pt->size4);
			part_set_generic_name(desc, part_num,
					      (char *)info->name);
			/* sprintf(info->type, "%d, pt->sys_ind); */
			strcpy((char *)info->type, "U-Boot");
			info->bootable = get_bootable(pt);
			if (CONFIG_IS_ENABLED(PARTITION_UUIDS)) {
				char str[12];

				sprintf(str, "%08x-%02x", disksig, part_num);
				disk_partition_set_uuid(info, str);
			}
			info->sys_ind = pt->sys_ind;
			return 0;
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			lbaint_t lba_start
				= get_unaligned_le32 (pt->start4) + relative;

			return part_get_info_extended(desc, lba_start,
				 ext_part_sector == 0 ? lba_start : relative,
				 part_num, which_part, info, disksig);
		}
	}

	/* Check for DOS PBR if no partition is found */
	dos_type = test_block_type(buffer);

	if (dos_type == DOS_PBR) {
		info->start = 0;
		info->size = desc->lba;
		if (wdinfo.blksz > DOS_PART_DEFAULT_SECTOR)
			info->blksz = wdinfo.blksz;
		else
			info->blksz = DOS_PART_DEFAULT_SECTOR;
		info->bootable = 0;
		strcpy((char *)info->type, "U-Boot");
		disk_partition_clr_uuid(info);
		return 0;
	}

	return -1;
}

static void __maybe_unused part_print_dos(struct blk_desc *desc)
{
	printf("Part\tStart Sector\tNum Sectors\tUUID\t\tType\n");
	print_partition_extended(desc, 0, 0, 1, 0);
}

static int __maybe_unused part_get_info_dos(struct blk_desc *desc, int part,
					    struct disk_partition *info)
{
	return part_get_info_extended(desc, 0, 0, 1, part, info, 0);
}

int is_valid_dos_buf(void *buf)
{
	return test_block_type(buf) == DOS_MBR ? 0 : -1;
}

#if IS_ENABLED(CONFIG_CMD_MBR)
static void lba_to_chs(lbaint_t lba, unsigned char *rc, unsigned char *rh,
		       unsigned char *rs)
{
	unsigned int c, h, s;
	/* use fixed CHS geometry */
	unsigned int sectpertrack = 63;
	unsigned int heads = 255;

	c = (lba + 1) / sectpertrack / heads;
	h = (lba + 1) / sectpertrack - c * heads;
	s = (lba + 1) - (c * heads + h) * sectpertrack;

	if (c > 1023) {
		c = 1023;
		h = 254;
		s = 63;
	}

	*rc = c & 0xff;
	*rh = h;
	*rs = s + ((c & 0x300) >> 2);
}

static void mbr_fill_pt_entry(dos_partition_t *pt, lbaint_t start,
		lbaint_t relative, lbaint_t size, uchar sys_ind, bool bootable)
{
	pt->boot_ind = bootable ? 0x80 : 0x00;
	pt->sys_ind = sys_ind;
	lba_to_chs(start, &pt->cyl, &pt->head, &pt->sector);
	lba_to_chs(start + size - 1, &pt->end_cyl, &pt->end_head, &pt->end_sector);
	put_unaligned_le32(relative, &pt->start4);
	put_unaligned_le32(size, &pt->size4);
}

int write_mbr_partitions(struct blk_desc *dev,
		struct disk_partition *p, int count, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev->blksz);
	lbaint_t ext_part_start = 0, ext_part_size = 0, ext_part_sect = 0;
	dos_partition_t *pt;
	int i;

	memset(buffer, 0, dev->blksz);
	buffer[DOS_PART_MAGIC_OFFSET] = 0x55;
	buffer[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;
	put_unaligned_le32(disksig, &buffer[DOS_PART_DISKSIG_OFFSET]);
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);

	/* create all primary partitions */
	for (i = 0; i < 4 && i < count; i++, pt++) {
		mbr_fill_pt_entry(pt, p[i].start, p[i].start, p[i].size,
				  p[i].sys_ind, p[i].bootable);
		if (is_extended(p[i].sys_ind)) {
			ext_part_start = p[i].start;
			ext_part_size = p[i].size;
			ext_part_sect = p[i].start;
		}
	}

	if (i < count && !ext_part_start) {
		printf("%s: extended partition is needed for more than 4 partitions\n",
		        __func__);
		return -1;
	}

	/* write MBR */
	if (blk_dwrite(dev, 0, 1, buffer) != 1) {
		printf("%s: failed writing 'MBR' (1 blks at 0x0)\n",
		       __func__);
		return -1;
	}

	/* create extended volumes */
	for (; i < count; i++) {
		lbaint_t next_ebr = 0;

		memset(buffer, 0, dev->blksz);
		buffer[DOS_PART_MAGIC_OFFSET] = 0x55;
		buffer[DOS_PART_MAGIC_OFFSET + 1] = 0xaa;
		pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);

		mbr_fill_pt_entry(pt, p[i].start, p[i].start - ext_part_sect,
				  p[i].size, p[i].sys_ind, p[i].bootable);

		if (i + 1 < count) {
			pt++;
			next_ebr = p[i].start + p[i].size;
			mbr_fill_pt_entry(pt, next_ebr,
					  next_ebr - ext_part_start,
					  p[i+1].start + p[i+1].size - next_ebr,
					  DOS_PART_TYPE_EXTENDED, 0);
		}

		/* write EBR */
		if (blk_dwrite(dev, ext_part_sect, 1, buffer) != 1) {
			printf("%s: failed writing 'EBR' (1 blks at 0x%lx)\n",
			       __func__, ext_part_sect);
			return -1;
		}
		ext_part_sect = next_ebr;
	}

	/* Update the partition table entries*/
	part_init(dev);

	return 0;
}

int layout_mbr_partitions(struct disk_partition *p, int count,
			  lbaint_t total_sectors)
{
	struct disk_partition *ext = NULL;
	int i, j;
	lbaint_t ext_vol_start;

	/* calculate primary partitions start and size if needed */
	if (!p[0].start)
		p[0].start = DOS_PART_DEFAULT_GAP;
	for (i = 0; i < 4 && i < count; i++) {
		if (!p[i].start)
			p[i].start = p[i - 1].start + p[i - 1].size;
		if (!p[i].size) {
			lbaint_t end = total_sectors;
			lbaint_t allocated = 0;

			for (j = i + 1; j < 4 && j < count; j++) {
				if (p[j].start) {
					end = p[j].start;
					break;
				}
				allocated += p[j].size;
			}
			p[i].size = end - allocated - p[i].start;
		}
		if (p[i].sys_ind == 0x05)
			ext = &p[i];
	}

	if (count <= 4)
		return 0;

	if (!ext) {
		log_err("extended partition is needed for more than 4 partitions\n");
		return -EINVAL;
	}

	/* calculate extended volumes start and size if needed */
	ext_vol_start = ext->start;
	for (i = 4; i < count; i++) {
		if (!p[i].start)
			p[i].start = ext_vol_start + DOS_PART_DEFAULT_GAP;
		if (!p[i].size) {
			lbaint_t end = ext->start + ext->size;
			lbaint_t allocated = 0;

			for (j = i + 1; j < count; j++) {
				if (p[j].start) {
					end = p[j].start - DOS_PART_DEFAULT_GAP;
					break;
				}
				allocated += p[j].size + DOS_PART_DEFAULT_GAP;
			}
			p[i].size = end - allocated - p[i].start;
		}
		ext_vol_start = p[i].start + p[i].size;
	}

	return 0;
}
#endif

int write_mbr_sector(struct blk_desc *desc, void *buf)
{
	if (is_valid_dos_buf(buf))
		return -1;

	/* write MBR */
	if (blk_dwrite(desc, 0, 1, buf) != 1) {
		printf("%s: failed writing '%s' (1 blks at 0x0)\n",
		       __func__, "MBR");
		return 1;
	}

	/* Update the partition table entries*/
	part_init(desc);

	return 0;
}

U_BOOT_PART_TYPE(dos) = {
	.name		= "DOS",
	.part_type	= PART_TYPE_DOS,
	.max_entries	= DOS_ENTRY_NUMBERS,
	.get_info	= part_get_info_ptr(part_get_info_dos),
	.print		= part_print_ptr(part_print_dos),
	.test		= part_test_dos,
};
