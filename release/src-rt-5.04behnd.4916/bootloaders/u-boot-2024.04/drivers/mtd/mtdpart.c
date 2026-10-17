// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple MTD partitioning layer
 *
 * Copyright © 2000 Nicolas Pitre <nico@fluxnic.net>
 * Copyright © 2002 Thomas Gleixner <gleixner@linutronix.de>
 * Copyright © 2000-2010 David Woodhouse <dwmw2@infradead.org>
 *
 */

#ifndef __UBOOT__
#include <log.h>
#include <dm/devres.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kmod.h>
#endif

#include <common.h>
#include <malloc.h>
#include <linux/bug.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <ubi_uboot.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/err.h>
#include <linux/sizes.h>

#include "mtdcore.h"

#ifndef __UBOOT__
static DEFINE_MUTEX(mtd_partitions_mutex);
#else
DEFINE_MUTEX(mtd_partitions_mutex);
#endif

#ifdef __UBOOT__
/* from mm/util.c */

/**
 * kstrdup - allocate space for and copy an existing string
 * @s: the string to duplicate
 * @gfp: the GFP mask used in the kmalloc() call when allocating memory
 */
char *kstrdup(const char *s, gfp_t gfp)
{
	size_t len;
	char *buf;

	if (!s)
		return NULL;

	len = strlen(s) + 1;
	buf = kmalloc(len, gfp);
	if (buf)
		memcpy(buf, s, len);
	return buf;
}
#endif

#define MTD_SIZE_REMAINING		(~0LLU)
#define MTD_OFFSET_NOT_SPECIFIED	(~0LLU)

bool mtd_partitions_used(struct mtd_info *master)
{
	struct mtd_info *slave;

	list_for_each_entry(slave, &master->partitions, node) {
		if (slave->usecount)
			return true;
	}

	return false;
}

/**
 * mtd_parse_partition - Parse @mtdparts partition definition, fill @partition
 *                       with it and update the @mtdparts string pointer.
 *
 * The partition name is allocated and must be freed by the caller.
 *
 * This function is widely inspired from part_parse (mtdparts.c).
 *
 * @mtdparts: String describing the partition with mtdparts command syntax
 * @partition: MTD partition structure to fill
 *
 * Return: 0 on success, an error otherwise.
 */
static int mtd_parse_partition(const char **_mtdparts,
			       struct mtd_partition *partition)
{
	const char *mtdparts = *_mtdparts;
	const char *name = NULL;
	int name_len;
	char *buf;

	/* Ensure the partition structure is empty */
	memset(partition, 0, sizeof(struct mtd_partition));

	/* Fetch the partition size */
	if (*mtdparts == '-') {
		/* Assign all remaining space to this partition */
		partition->size = MTD_SIZE_REMAINING;
		mtdparts++;
	} else {
		partition->size = ustrtoull(mtdparts, (char **)&mtdparts, 0);
		if (partition->size < SZ_4K) {
			printf("Minimum partition size 4kiB, %lldB requested\n",
			       partition->size);
			return -EINVAL;
		}
	}

	/* Check for the offset */
	partition->offset = MTD_OFFSET_NOT_SPECIFIED;
	if (*mtdparts == '@') {
		mtdparts++;
		partition->offset = ustrtoull(mtdparts, (char **)&mtdparts, 0);
	}

	/* Now look for the name */
	if (*mtdparts == '(') {
		name = ++mtdparts;
		mtdparts = strchr(name, ')');
		if (!mtdparts) {
			printf("No closing ')' found in partition name\n");
			return -EINVAL;
		}
		name_len = mtdparts - name + 1;
		if ((name_len - 1) == 0) {
			printf("Empty partition name\n");
			return -EINVAL;
		}
		mtdparts++;
	} else {
		/* Name will be of the form size@offset */
		name_len = 22;
	}

	/* Check if the partition is read-only */
	if (strncmp(mtdparts, "ro", 2) == 0) {
		partition->mask_flags |= MTD_WRITEABLE;
		mtdparts += 2;
	}

	/* Check for a potential next partition definition */
	if (*mtdparts == ',') {
		if (partition->size == MTD_SIZE_REMAINING) {
			printf("No partitions allowed after a fill-up\n");
			return -EINVAL;
		}
		++mtdparts;
	} else if ((*mtdparts == ';') || (*mtdparts == '\0')) {
		/* NOP */
	} else {
		printf("Unexpected character '%c' in mtdparts\n", *mtdparts);
		return -EINVAL;
	}

	/*
	 * Allocate a buffer for the name and either copy the provided name or
	 * auto-generate it with the form 'size@offset'.
	 */
	buf = malloc(name_len);
	if (!buf)
		return -ENOMEM;

	if (name)
		strncpy(buf, name, name_len - 1);
	else
		snprintf(buf, name_len, "0x%08llx@0x%08llx",
			 partition->size, partition->offset);

	buf[name_len - 1] = '\0';
	partition->name = buf;

	*_mtdparts = mtdparts;

	return 0;
}

/**
 * mtd_parse_partitions - Create a partition array from an mtdparts definition
 *
 * Stateless function that takes a @parent MTD device, a string @_mtdparts
 * describing the partitions (with the "mtdparts" command syntax) and creates
 * the corresponding MTD partition structure array @_parts. Both the name and
 * the structure partition itself must be freed freed, the caller may use
 * @mtd_free_parsed_partitions() for this purpose.
 *
 * @parent: MTD device which contains the partitions
 * @_mtdparts: Pointer to a string describing the partitions with "mtdparts"
 *             command syntax.
 * @_parts: Allocated array containing the partitions, must be freed by the
 *          caller.
 * @_nparts: Size of @_parts array.
 *
 * Return: 0 on success, an error otherwise.
 */
int mtd_parse_partitions(struct mtd_info *parent, const char **_mtdparts,
			 struct mtd_partition **_parts, int *_nparts)
{
	struct mtd_partition partition = {}, *parts;
	const char *mtdparts = *_mtdparts;
	uint64_t cur_off = 0, cur_sz = 0;
	int nparts = 0;
	int ret, idx;
	u64 sz;

	/* First, iterate over the partitions until we know their number */
	while (mtdparts[0] != '\0' && mtdparts[0] != ';') {
		ret = mtd_parse_partition(&mtdparts, &partition);
		if (ret)
			return ret;

		free((char *)partition.name);
		nparts++;
	}

	/* Allocate an array of partitions to give back to the caller */
	parts = malloc(sizeof(*parts) * nparts);
	if (!parts) {
		printf("Not enough space to save partitions meta-data\n");
		return -ENOMEM;
	}

	/* Iterate again over each partition to save the data in our array */
	for (idx = 0; idx < nparts; idx++) {
		ret = mtd_parse_partition(_mtdparts, &parts[idx]);
		if (ret)
			return ret;

		if (parts[idx].size == MTD_SIZE_REMAINING)
			parts[idx].size = parent->size - cur_sz;
		cur_sz += parts[idx].size;

		sz = parts[idx].size;
		if (sz < parent->writesize || do_div(sz, parent->writesize)) {
			printf("Partition size must be a multiple of %d\n",
			       parent->writesize);
			return -EINVAL;
		}

		if (parts[idx].offset == MTD_OFFSET_NOT_SPECIFIED)
			parts[idx].offset = cur_off;
		cur_off += parts[idx].size;

		parts[idx].ecclayout = parent->ecclayout;
	}

	/* Offset by one mtdparts to point to the next device if any */
	if (*_mtdparts[0] == ';')
		(*_mtdparts)++;

	*_parts = parts;
	*_nparts = nparts;

	return 0;
}

/**
 * mtd_free_parsed_partitions - Free dynamically allocated partitions
 *
 * Each successful call to @mtd_parse_partitions must be followed by a call to
 * @mtd_free_parsed_partitions to free any allocated array during the parsing
 * process.
 *
 * @parts: Array containing the partitions that will be freed.
 * @nparts: Size of @parts array.
 */
void mtd_free_parsed_partitions(struct mtd_partition *parts,
				unsigned int nparts)
{
	int i;

	for (i = 0; i < nparts; i++)
		free((char *)parts[i].name);

	free(parts);
}

/*
 * MTD methods which simply translate the effective address and pass through
 * to the _real_ device.
 */

static int part_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	struct mtd_ecc_stats stats;
	int res;

	stats = mtd->parent->ecc_stats;
	res = mtd->parent->_read(mtd->parent, from + mtd->offset, len,
				 retlen, buf);
	if (unlikely(mtd_is_eccerr(res)))
		mtd->ecc_stats.failed +=
			mtd->parent->ecc_stats.failed - stats.failed;
	else
		mtd->ecc_stats.corrected +=
			mtd->parent->ecc_stats.corrected - stats.corrected;
	return res;
}

#ifndef __UBOOT__
static int part_point(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, void **virt, resource_size_t *phys)
{
	return mtd->parent->_point(mtd->parent, from + mtd->offset, len,
				   retlen, virt, phys);
}

static int part_unpoint(struct mtd_info *mtd, loff_t from, size_t len)
{
	return mtd->parent->_unpoint(mtd->parent, from + mtd->offset, len);
}
#endif

static unsigned long part_get_unmapped_area(struct mtd_info *mtd,
					    unsigned long len,
					    unsigned long offset,
					    unsigned long flags)
{
	offset += mtd->offset;
	return mtd->parent->_get_unmapped_area(mtd->parent, len, offset, flags);
}

static int part_read_oob(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	int res;

	if (from >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && from + ops->len > mtd->size)
		return -EINVAL;

	/*
	 * If OOB is also requested, make sure that we do not read past the end
	 * of this partition.
	 */
	if (ops->oobbuf) {
		size_t len, pages;

		if (ops->mode == MTD_OPS_AUTO_OOB)
			len = mtd->oobavail;
		else
			len = mtd->oobsize;
		pages = mtd_div_by_ws(mtd->size, mtd);
		pages -= mtd_div_by_ws(from, mtd);
		if (ops->ooboffs + ops->ooblen > pages * len)
			return -EINVAL;
	}

	res = mtd->parent->_read_oob(mtd->parent, from + mtd->offset, ops);
	if (unlikely(res)) {
		if (mtd_is_bitflip(res))
			mtd->ecc_stats.corrected++;
		if (mtd_is_eccerr(res))
			mtd->ecc_stats.failed++;
	}
	return res;
}

static int part_read_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	return mtd->parent->_read_user_prot_reg(mtd->parent, from, len,
						retlen, buf);
}

static int part_get_user_prot_info(struct mtd_info *mtd, size_t len,
				   size_t *retlen, struct otp_info *buf)
{
	return mtd->parent->_get_user_prot_info(mtd->parent, len, retlen,
						buf);
}

static int part_read_fact_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	return mtd->parent->_read_fact_prot_reg(mtd->parent, from, len,
						retlen, buf);
}

static int part_get_fact_prot_info(struct mtd_info *mtd, size_t len,
				   size_t *retlen, struct otp_info *buf)
{
	return mtd->parent->_get_fact_prot_info(mtd->parent, len, retlen,
						buf);
}

static int part_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	return mtd->parent->_write(mtd->parent, to + mtd->offset, len,
				   retlen, buf);
}

static int part_panic_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	return mtd->parent->_panic_write(mtd->parent, to + mtd->offset, len,
					 retlen, buf);
}

static int part_write_oob(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	if (to >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && to + ops->len > mtd->size)
		return -EINVAL;
	return mtd->parent->_write_oob(mtd->parent, to + mtd->offset, ops);
}

static int part_write_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	return mtd->parent->_write_user_prot_reg(mtd->parent, from, len,
						 retlen, buf);
}

static int part_lock_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len)
{
	return mtd->parent->_lock_user_prot_reg(mtd->parent, from, len);
}

#ifndef __UBOOT__
static int part_writev(struct mtd_info *mtd, const struct kvec *vecs,
		unsigned long count, loff_t to, size_t *retlen)
{
	return mtd->parent->_writev(mtd->parent, vecs, count,
				    to + mtd->offset, retlen);
}
#endif

static int part_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret;

	instr->addr += mtd->offset;

	ret = mtd->parent->_erase(mtd->parent, instr);
	if (ret && instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
		instr->fail_addr -= mtd->offset;

	instr->addr -= mtd->offset;

	return ret;
}

static int part_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return mtd->parent->_lock(mtd->parent, ofs + mtd->offset, len);
}

static int part_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return mtd->parent->_unlock(mtd->parent, ofs + mtd->offset, len);
}

static int part_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return mtd->parent->_is_locked(mtd->parent, ofs + mtd->offset, len);
}

static void part_sync(struct mtd_info *mtd)
{
	mtd->parent->_sync(mtd->parent);
}

#ifndef __UBOOT__
static int part_suspend(struct mtd_info *mtd)
{
	return mtd->parent->_suspend(mtd->parent);
}

static void part_resume(struct mtd_info *mtd)
{
	mtd->parent->_resume(mtd->parent);
}
#endif

static int part_block_isreserved(struct mtd_info *mtd, loff_t ofs)
{
	ofs += mtd->offset;
	return mtd->parent->_block_isreserved(mtd->parent, ofs);
}

static int part_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	ofs += mtd->offset;
	return mtd->parent->_block_isbad(mtd->parent, ofs);
}

static int part_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	int res;

	ofs += mtd->offset;
	res = mtd->parent->_block_markbad(mtd->parent, ofs);
	if (!res)
		mtd->ecc_stats.badblocks++;
	return res;
}

static inline void free_partition(struct mtd_info *p)
{
	kfree(p->name);
	kfree(p);
}

/*
 * This function unregisters and destroy all slave MTD objects which are
 * attached to the given master MTD object, recursively.
 */
static int do_del_mtd_partitions(struct mtd_info *master)
{
	struct mtd_info *slave, *next;
	int ret, err = 0;

	list_for_each_entry_safe(slave, next, &master->partitions, node) {
		if (mtd_has_partitions(slave))
			del_mtd_partitions(slave);

		debug("Deleting %s MTD partition\n", slave->name);
		ret = del_mtd_device(slave);
		if (ret < 0) {
			printf("Error when deleting partition \"%s\" (%d)\n",
			       slave->name, ret);
			err = ret;
			continue;
		}

		list_del(&slave->node);
		free_partition(slave);
	}

	return err;
}

int del_mtd_partitions(struct mtd_info *master)
{
	int ret;

	debug("Deleting MTD partitions on \"%s\":\n", master->name);

	mutex_lock(&mtd_partitions_mutex);
	ret = do_del_mtd_partitions(master);
	mutex_unlock(&mtd_partitions_mutex);

	return ret;
}

static struct mtd_info *allocate_partition(struct mtd_info *master,
					   const struct mtd_partition *part,
					   int partno, uint64_t cur_offset)
{
	struct mtd_info *slave;
	char *name;

	/* allocate the partition structure */
	slave = kzalloc(sizeof(*slave), GFP_KERNEL);
	name = kstrdup(part->name, GFP_KERNEL);
	if (!name || !slave) {
		printk(KERN_ERR"memory allocation error while creating partitions for \"%s\"\n",
		       master->name);
		kfree(name);
		kfree(slave);
		return ERR_PTR(-ENOMEM);
	}

	/* set up the MTD object for this partition */
	slave->type = master->type;
	slave->flags = master->flags & ~part->mask_flags;
	slave->size = part->size;
	slave->writesize = master->writesize;
	slave->writebufsize = master->writebufsize;
	slave->oobsize = master->oobsize;
	slave->oobavail = master->oobavail;
	slave->subpage_sft = master->subpage_sft;

	slave->name = name;
	slave->owner = master->owner;
#ifndef __UBOOT__
	slave->backing_dev_info = master->backing_dev_info;

	/* NOTE:  we don't arrange MTDs as a tree; it'd be error-prone
	 * to have the same data be in two different partitions.
	 */
	slave->dev.parent = master->dev.parent;
#endif

	if (master->_read)
		slave->_read = part_read;
	if (master->_write)
		slave->_write = part_write;

	if (master->_panic_write)
		slave->_panic_write = part_panic_write;

#ifndef __UBOOT__
	if (master->_point && master->_unpoint) {
		slave->_point = part_point;
		slave->_unpoint = part_unpoint;
	}
#endif

	if (master->_get_unmapped_area)
		slave->_get_unmapped_area = part_get_unmapped_area;
	if (master->_read_oob)
		slave->_read_oob = part_read_oob;
	if (master->_write_oob)
		slave->_write_oob = part_write_oob;
	if (master->_read_user_prot_reg)
		slave->_read_user_prot_reg = part_read_user_prot_reg;
	if (master->_read_fact_prot_reg)
		slave->_read_fact_prot_reg = part_read_fact_prot_reg;
	if (master->_write_user_prot_reg)
		slave->_write_user_prot_reg = part_write_user_prot_reg;
	if (master->_lock_user_prot_reg)
		slave->_lock_user_prot_reg = part_lock_user_prot_reg;
	if (master->_get_user_prot_info)
		slave->_get_user_prot_info = part_get_user_prot_info;
	if (master->_get_fact_prot_info)
		slave->_get_fact_prot_info = part_get_fact_prot_info;
	if (master->_sync)
		slave->_sync = part_sync;
#ifndef __UBOOT__
	if (!partno && !master->dev.class && master->_suspend &&
	    master->_resume) {
		slave->_suspend = part_suspend;
		slave->_resume = part_resume;
	}
	if (master->_writev)
		slave->_writev = part_writev;
#endif
	if (master->_lock)
		slave->_lock = part_lock;
	if (master->_unlock)
		slave->_unlock = part_unlock;
	if (master->_is_locked)
		slave->_is_locked = part_is_locked;
	if (master->_block_isreserved)
		slave->_block_isreserved = part_block_isreserved;
	if (master->_block_isbad)
		slave->_block_isbad = part_block_isbad;
	if (master->_block_markbad)
		slave->_block_markbad = part_block_markbad;
	slave->_erase = part_erase;
	slave->parent = master;
	slave->offset = part->offset;
	INIT_LIST_HEAD(&slave->partitions);
	INIT_LIST_HEAD(&slave->node);

	if (slave->offset == MTDPART_OFS_APPEND)
		slave->offset = cur_offset;
	if (slave->offset == MTDPART_OFS_NXTBLK) {
		slave->offset = cur_offset;
		if (mtd_mod_by_eb(cur_offset, master) != 0) {
			/* Round up to next erasesize */
			slave->offset = (mtd_div_by_eb(cur_offset, master) + 1) * master->erasesize;
			debug("Moving partition %d: "
			       "0x%012llx -> 0x%012llx\n", partno,
			       (unsigned long long)cur_offset, (unsigned long long)slave->offset);
		}
	}
	if (slave->offset == MTDPART_OFS_RETAIN) {
		slave->offset = cur_offset;
		if (master->size - slave->offset >= slave->size) {
			slave->size = master->size - slave->offset
							- slave->size;
		} else {
			debug("mtd partition \"%s\" doesn't have enough space: %#llx < %#llx, disabled\n",
				part->name, master->size - slave->offset,
				slave->size);
			/* register to preserve ordering */
			goto out_register;
		}
	}
	if (slave->size == MTDPART_SIZ_FULL)
		slave->size = master->size - slave->offset;

	debug("0x%012llx-0x%012llx : \"%s\"\n", (unsigned long long)slave->offset,
		(unsigned long long)(slave->offset + slave->size), slave->name);

	/* let's do some sanity checks */
	if (slave->offset >= master->size) {
		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->size = 0;
		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	if (slave->offset + slave->size > master->size) {
		slave->size = master->size - slave->offset;
		printk(KERN_WARNING"mtd: partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#llx\n",
		       part->name, master->name, slave->size);
	}
	if (master->numeraseregions > 1) {
		/* Deal with variable erase size stuff */
		int i, max = master->numeraseregions;
		u64 end = slave->offset + slave->size;
		struct mtd_erase_region_info *regions = master->eraseregions;

		/* Find the first erase regions which is part of this
		 * partition. */
		for (i = 0; i < max && regions[i].offset <= slave->offset; i++)
			;
		/* The loop searched for the region _behind_ the first one */
		if (i > 0)
			i--;

		/* Pick biggest erasesize */
		for (; i < max && regions[i].offset < end; i++) {
			if (slave->erasesize < regions[i].erasesize)
				slave->erasesize = regions[i].erasesize;
		}
		WARN_ON(slave->erasesize == 0);
	} else {
		/* Single erase size */
		slave->erasesize = master->erasesize;
	}

	if ((slave->flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->offset, slave)) {
		/* Doesn't start on a boundary of major erase size */
		/* FIXME: Let it be writable if it is on a boundary of
		 * _minor_ erase size though */
		slave->flags &= ~MTD_WRITEABLE;
		printk(KERN_WARNING"mtd: partition \"%s\" doesn't start on an erase block boundary -- force read-only\n",
			part->name);
	}
	if ((slave->flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->size, slave)) {
		slave->flags &= ~MTD_WRITEABLE;
		printk(KERN_WARNING"mtd: partition \"%s\" doesn't end on an erase block -- force read-only\n",
			part->name);
	}

	slave->ecclayout = master->ecclayout;
	slave->ecc_step_size = master->ecc_step_size;
	slave->ecc_strength = master->ecc_strength;
	slave->bitflip_threshold = master->bitflip_threshold;

	if (master->_block_isbad) {
		uint64_t offs = 0;

		while (offs < slave->size) {
			if (mtd_block_isbad(master, offs + slave->offset))
				slave->ecc_stats.badblocks++;
			offs += slave->erasesize;
		}
	}

out_register:
	return slave;
}

#ifndef __UBOOT__
int mtd_add_partition(struct mtd_info *master, const char *name,
		      long long offset, long long length)
{
	struct mtd_partition part;
	struct mtd_info *p, *new;
	uint64_t start, end;
	int ret = 0;

	/* the direct offset is expected */
	if (offset == MTDPART_OFS_APPEND ||
	    offset == MTDPART_OFS_NXTBLK)
		return -EINVAL;

	if (length == MTDPART_SIZ_FULL)
		length = master->size - offset;

	if (length <= 0)
		return -EINVAL;

	part.name = name;
	part.size = length;
	part.offset = offset;
	part.mask_flags = 0;
	part.ecclayout = NULL;

	new = allocate_partition(master, &part, -1, offset);
	if (IS_ERR(new))
		return PTR_ERR(new);

	start = offset;
	end = offset + length;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry(p, &master->partitions, node) {
		if (start >= p->offset &&
		    (start < (p->offset + p->size)))
			goto err_inv;

		if (end >= p->offset &&
		    (end < (p->offset + p->size)))
			goto err_inv;
	}

	list_add_tail(&new->node, &master->partitions);
	mutex_unlock(&mtd_partitions_mutex);

	add_mtd_device(new);

	return ret;
err_inv:
	mutex_unlock(&mtd_partitions_mutex);
	free_partition(new);
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);

int mtd_del_partition(struct mtd_info *master, int partno)
{
	struct mtd_info *slave, *next;
	int ret = -EINVAL;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &master->partitions, node)
		if (slave->index == partno) {
			ret = del_mtd_device(slave);
			if (ret < 0)
				break;

			list_del(&slave->node);
			free_partition(slave);
			break;
		}
	mutex_unlock(&mtd_partitions_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_del_partition);
#endif

/*
 * This function, given a master MTD object and a partition table, creates
 * and registers slave MTD objects which are bound to the master according to
 * the partition definitions.
 *
 * We don't register the master, or expect the caller to have done so,
 * for reasons of data integrity.
 */

int add_mtd_partitions(struct mtd_info *master,
		       const struct mtd_partition *parts,
		       int nbparts)
{
	struct mtd_info *slave;
	uint64_t cur_offset = 0;
	int i;

	debug("Creating %d MTD partitions on \"%s\":\n", nbparts, master->name);

	for (i = 0; i < nbparts; i++) {
		slave = allocate_partition(master, parts + i, i, cur_offset);
		if (IS_ERR(slave))
			return PTR_ERR(slave);

		mutex_lock(&mtd_partitions_mutex);
		list_add_tail(&slave->node, &master->partitions);
		mutex_unlock(&mtd_partitions_mutex);

		add_mtd_device(slave);

		cur_offset = slave->offset + slave->size;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(DM) && CONFIG_IS_ENABLED(OF_CONTROL)
int add_mtd_partitions_of(struct mtd_info *master)
{
	ofnode parts, child;
	int i = 0;

	if (!master->dev && !ofnode_valid(master->flash_node))
		return 0;

	if (master->dev)
		parts = ofnode_find_subnode(mtd_get_ofnode(master), "partitions");
	else
		parts = ofnode_find_subnode(master->flash_node, "partitions");

	if (!ofnode_valid(parts) || !ofnode_is_enabled(parts) ||
	    !ofnode_device_is_compatible(parts, "fixed-partitions"))
		return 0;

	ofnode_for_each_subnode(child, parts) {
		struct mtd_partition part = { 0 };
		struct mtd_info *slave;
		fdt_addr_t offset;
		fdt_size_t size;

		if (!ofnode_is_enabled(child))
			continue;

		offset = ofnode_get_addr_size_index_notrans(child, 0, &size);
		if (offset == FDT_ADDR_T_NONE || !size) {
			debug("Missing partition offset/size on \"%s\" partition\n",
			      master->name);
			continue;
		}

		part.name = ofnode_read_string(child, "label");
		if (!part.name)
			part.name = ofnode_read_string(child, "name");

		/*
		 * .mask_flags is used to remove flags in allocate_partition(),
		 * so when "read-only" is present, we add MTD_WRITABLE to the
		 * mask, and so MTD_WRITABLE will be removed on partition
		 * allocation
		 */
		if (ofnode_read_bool(child, "read-only"))
			part.mask_flags |= MTD_WRITEABLE;
		if (ofnode_read_bool(child, "lock"))
			part.mask_flags |= MTD_POWERUP_LOCK;

		part.offset = offset;
		part.size = size;
		part.ecclayout = master->ecclayout;

		slave = allocate_partition(master, &part, i++, 0);
		if (IS_ERR(slave))
			return PTR_ERR(slave);

		mutex_lock(&mtd_partitions_mutex);
		list_add_tail(&slave->node, &master->partitions);
		mutex_unlock(&mtd_partitions_mutex);

		add_mtd_device(slave);
	}

	return 0;
}
#endif /* CONFIG_IS_ENABLED(DM) && CONFIG_IS_ENABLED(OF_CONTROL) */

#ifndef __UBOOT__
static DEFINE_SPINLOCK(part_parser_lock);
static LIST_HEAD(part_parsers);

static struct mtd_part_parser *get_partition_parser(const char *name)
{
	struct mtd_part_parser *p, *ret = NULL;

	spin_lock(&part_parser_lock);

	list_for_each_entry(p, &part_parsers, list)
		if (!strcmp(p->name, name) && try_module_get(p->owner)) {
			ret = p;
			break;
		}

	spin_unlock(&part_parser_lock);

	return ret;
}

#define put_partition_parser(p) do { module_put((p)->owner); } while (0)

void register_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_add(&p->list, &part_parsers);
	spin_unlock(&part_parser_lock);
}
EXPORT_SYMBOL_GPL(register_mtd_parser);

void deregister_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_del(&p->list);
	spin_unlock(&part_parser_lock);
}
EXPORT_SYMBOL_GPL(deregister_mtd_parser);

/*
 * Do not forget to update 'parse_mtd_partitions()' kerneldoc comment if you
 * are changing this array!
 */
static const char * const default_mtd_part_types[] = {
	"cmdlinepart",
	"ofpart",
	NULL
};

/**
 * parse_mtd_partitions - parse MTD partitions
 * @master: the master partition (describes whole MTD device)
 * @types: names of partition parsers to try or %NULL
 * @pparts: array of partitions found is returned here
 * @data: MTD partition parser-specific data
 *
 * This function tries to find partition on MTD device @master. It uses MTD
 * partition parsers, specified in @types. However, if @types is %NULL, then
 * the default list of parsers is used. The default list contains only the
 * "cmdlinepart" and "ofpart" parsers ATM.
 * Note: If there are more then one parser in @types, the kernel only takes the
 * partitions parsed out by the first parser.
 *
 * This function may return:
 * o a negative error code in case of failure
 * o zero if no partitions were found
 * o a positive number of found partitions, in which case on exit @pparts will
 *   point to an array containing this number of &struct mtd_info objects.
 */
int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
			 struct mtd_partition **pparts,
			 struct mtd_part_parser_data *data)
{
	struct mtd_part_parser *parser;
	int ret = 0;

	if (!types)
		types = default_mtd_part_types;

	for ( ; ret <= 0 && *types; types++) {
		parser = get_partition_parser(*types);
		if (!parser && !request_module("%s", *types))
			parser = get_partition_parser(*types);
		if (!parser)
			continue;
		ret = (*parser->parse_fn)(master, pparts, data);
		put_partition_parser(parser);
		if (ret > 0) {
			printk(KERN_NOTICE "%d %s partitions found on MTD device %s\n",
			       ret, parser->name, master->name);
			break;
		}
	}
	return ret;
}
#endif

/* Returns the size of the entire flash chip */
uint64_t mtd_get_device_size(const struct mtd_info *mtd)
{
	if (mtd_is_partition(mtd))
		return mtd->parent->size;

	return mtd->size;
}
EXPORT_SYMBOL_GPL(mtd_get_device_size);
