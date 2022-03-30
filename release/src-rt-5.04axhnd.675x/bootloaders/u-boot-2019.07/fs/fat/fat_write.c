// SPDX-License-Identifier: GPL-2.0+
/*
 * fat_write.c
 *
 * R/W (V)FAT 12/16/32 filesystem implementation by Donggeun Kim
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <fat.h>
#include <asm/byteorder.h>
#include <part.h>
#include <linux/ctype.h>
#include <div64.h>
#include <linux/math64.h>
#include "fat.c"

static void uppercase(char *str, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		*str = toupper(*str);
		str++;
	}
}

static int total_sector;
static int disk_write(__u32 block, __u32 nr_blocks, void *buf)
{
	ulong ret;

	if (!cur_dev)
		return -1;

	if (cur_part_info.start + block + nr_blocks >
		cur_part_info.start + total_sector) {
		printf("error: overflow occurs\n");
		return -1;
	}

	ret = blk_dwrite(cur_dev, cur_part_info.start + block, nr_blocks, buf);
	if (nr_blocks && ret == 0)
		return -1;

	return ret;
}

/*
 * Set short name in directory entry
 */
static void set_name(dir_entry *dirent, const char *filename)
{
	char s_name[VFAT_MAXLEN_BYTES];
	char *period;
	int period_location, len, i, ext_num;

	if (filename == NULL)
		return;

	len = strlen(filename);
	if (len == 0)
		return;

	strcpy(s_name, filename);
	uppercase(s_name, len);

	period = strchr(s_name, '.');
	if (period == NULL) {
		period_location = len;
		ext_num = 0;
	} else {
		period_location = period - s_name;
		ext_num = len - period_location - 1;
	}

	/* Pad spaces when the length of file name is shorter than eight */
	if (period_location < 8) {
		memcpy(dirent->name, s_name, period_location);
		for (i = period_location; i < 8; i++)
			dirent->name[i] = ' ';
	} else if (period_location == 8) {
		memcpy(dirent->name, s_name, period_location);
	} else {
		memcpy(dirent->name, s_name, 6);
		dirent->name[6] = '~';
		dirent->name[7] = '1';
	}

	if (ext_num < 3) {
		memcpy(dirent->ext, s_name + period_location + 1, ext_num);
		for (i = ext_num; i < 3; i++)
			dirent->ext[i] = ' ';
	} else
		memcpy(dirent->ext, s_name + period_location + 1, 3);

	debug("name : %s\n", dirent->name);
	debug("ext : %s\n", dirent->ext);
}

/*
 * Write fat buffer into block device
 */
static int flush_dirty_fat_buffer(fsdata *mydata)
{
	int getsize = FATBUFBLOCKS;
	__u32 fatlength = mydata->fatlength;
	__u8 *bufptr = mydata->fatbuf;
	__u32 startblock = mydata->fatbufnum * FATBUFBLOCKS;

	debug("debug: evicting %d, dirty: %d\n", mydata->fatbufnum,
	      (int)mydata->fat_dirty);

	if ((!mydata->fat_dirty) || (mydata->fatbufnum == -1))
		return 0;

	/* Cap length if fatlength is not a multiple of FATBUFBLOCKS */
	if (startblock + getsize > fatlength)
		getsize = fatlength - startblock;

	startblock += mydata->fat_sect;

	/* Write FAT buf */
	if (disk_write(startblock, getsize, bufptr) < 0) {
		debug("error: writing FAT blocks\n");
		return -1;
	}

	if (mydata->fats == 2) {
		/* Update corresponding second FAT blocks */
		startblock += mydata->fatlength;
		if (disk_write(startblock, getsize, bufptr) < 0) {
			debug("error: writing second FAT blocks\n");
			return -1;
		}
	}
	mydata->fat_dirty = 0;

	return 0;
}

/*
 * Set the file name information from 'name' into 'slotptr',
 */
static int str2slot(dir_slot *slotptr, const char *name, int *idx)
{
	int j, end_idx = 0;

	for (j = 0; j <= 8; j += 2) {
		if (name[*idx] == 0x00) {
			slotptr->name0_4[j] = 0;
			slotptr->name0_4[j + 1] = 0;
			end_idx++;
			goto name0_4;
		}
		slotptr->name0_4[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}
	for (j = 0; j <= 10; j += 2) {
		if (name[*idx] == 0x00) {
			slotptr->name5_10[j] = 0;
			slotptr->name5_10[j + 1] = 0;
			end_idx++;
			goto name5_10;
		}
		slotptr->name5_10[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}
	for (j = 0; j <= 2; j += 2) {
		if (name[*idx] == 0x00) {
			slotptr->name11_12[j] = 0;
			slotptr->name11_12[j + 1] = 0;
			end_idx++;
			goto name11_12;
		}
		slotptr->name11_12[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}

	if (name[*idx] == 0x00)
		return 1;

	return 0;
/* Not used characters are filled with 0xff 0xff */
name0_4:
	for (; end_idx < 5; end_idx++) {
		slotptr->name0_4[end_idx * 2] = 0xff;
		slotptr->name0_4[end_idx * 2 + 1] = 0xff;
	}
	end_idx = 5;
name5_10:
	end_idx -= 5;
	for (; end_idx < 6; end_idx++) {
		slotptr->name5_10[end_idx * 2] = 0xff;
		slotptr->name5_10[end_idx * 2 + 1] = 0xff;
	}
	end_idx = 11;
name11_12:
	end_idx -= 11;
	for (; end_idx < 2; end_idx++) {
		slotptr->name11_12[end_idx * 2] = 0xff;
		slotptr->name11_12[end_idx * 2 + 1] = 0xff;
	}

	return 1;
}

static int new_dir_table(fat_itr *itr);
static int flush_dir(fat_itr *itr);

/*
 * Fill dir_slot entries with appropriate name, id, and attr
 * 'itr' will point to a next entry
 */
static int
fill_dir_slot(fat_itr *itr, const char *l_name)
{
	__u8 temp_dir_slot_buffer[MAX_LFN_SLOT * sizeof(dir_slot)];
	dir_slot *slotptr = (dir_slot *)temp_dir_slot_buffer;
	__u8 counter = 0, checksum;
	int idx = 0, ret;

	/* Get short file name checksum value */
	checksum = mkcksum(itr->dent->name, itr->dent->ext);

	do {
		memset(slotptr, 0x00, sizeof(dir_slot));
		ret = str2slot(slotptr, l_name, &idx);
		slotptr->id = ++counter;
		slotptr->attr = ATTR_VFAT;
		slotptr->alias_checksum = checksum;
		slotptr++;
	} while (ret == 0);

	slotptr--;
	slotptr->id |= LAST_LONG_ENTRY_MASK;

	while (counter >= 1) {
		memcpy(itr->dent, slotptr, sizeof(dir_slot));
		slotptr--;
		counter--;

		if (itr->remaining == 0)
			flush_dir(itr);

		/* allocate a cluster for more entries */
		if (!fat_itr_next(itr))
			if (!itr->dent &&
			    (!itr->is_root || itr->fsdata->fatsize == 32) &&
			    new_dir_table(itr))
				return -1;
	}

	return 0;
}

/*
 * Set the entry at index 'entry' in a FAT (12/16/32) table.
 */
static int set_fatent_value(fsdata *mydata, __u32 entry, __u32 entry_value)
{
	__u32 bufnum, offset, off16;
	__u16 val1, val2;

	switch (mydata->fatsize) {
	case 32:
		bufnum = entry / FAT32BUFSIZE;
		offset = entry - bufnum * FAT32BUFSIZE;
		break;
	case 16:
		bufnum = entry / FAT16BUFSIZE;
		offset = entry - bufnum * FAT16BUFSIZE;
		break;
	case 12:
		bufnum = entry / FAT12BUFSIZE;
		offset = entry - bufnum * FAT12BUFSIZE;
		break;
	default:
		/* Unsupported FAT size */
		return -1;
	}

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum) {
		int getsize = FATBUFBLOCKS;
		__u8 *bufptr = mydata->fatbuf;
		__u32 fatlength = mydata->fatlength;
		__u32 startblock = bufnum * FATBUFBLOCKS;

		/* Cap length if fatlength is not a multiple of FATBUFBLOCKS */
		if (startblock + getsize > fatlength)
			getsize = fatlength - startblock;

		if (flush_dirty_fat_buffer(mydata) < 0)
			return -1;

		startblock += mydata->fat_sect;

		if (disk_read(startblock, getsize, bufptr) < 0) {
			debug("Error reading FAT blocks\n");
			return -1;
		}
		mydata->fatbufnum = bufnum;
	}

	/* Mark as dirty */
	mydata->fat_dirty = 1;

	/* Set the actual entry */
	switch (mydata->fatsize) {
	case 32:
		((__u32 *) mydata->fatbuf)[offset] = cpu_to_le32(entry_value);
		break;
	case 16:
		((__u16 *) mydata->fatbuf)[offset] = cpu_to_le16(entry_value);
		break;
	case 12:
		off16 = (offset * 3) / 4;

		switch (offset & 0x3) {
		case 0:
			val1 = cpu_to_le16(entry_value) & 0xfff;
			((__u16 *)mydata->fatbuf)[off16] &= ~0xfff;
			((__u16 *)mydata->fatbuf)[off16] |= val1;
			break;
		case 1:
			val1 = cpu_to_le16(entry_value) & 0xf;
			val2 = (cpu_to_le16(entry_value) >> 4) & 0xff;

			((__u16 *)mydata->fatbuf)[off16] &= ~0xf000;
			((__u16 *)mydata->fatbuf)[off16] |= (val1 << 12);

			((__u16 *)mydata->fatbuf)[off16 + 1] &= ~0xff;
			((__u16 *)mydata->fatbuf)[off16 + 1] |= val2;
			break;
		case 2:
			val1 = cpu_to_le16(entry_value) & 0xff;
			val2 = (cpu_to_le16(entry_value) >> 8) & 0xf;

			((__u16 *)mydata->fatbuf)[off16] &= ~0xff00;
			((__u16 *)mydata->fatbuf)[off16] |= (val1 << 8);

			((__u16 *)mydata->fatbuf)[off16 + 1] &= ~0xf;
			((__u16 *)mydata->fatbuf)[off16 + 1] |= val2;
			break;
		case 3:
			val1 = cpu_to_le16(entry_value) & 0xfff;
			((__u16 *)mydata->fatbuf)[off16] &= ~0xfff0;
			((__u16 *)mydata->fatbuf)[off16] |= (val1 << 4);
			break;
		default:
			break;
		}

		break;
	default:
		return -1;
	}

	return 0;
}

/*
 * Determine the next free cluster after 'entry' in a FAT (12/16/32) table
 * and link it to 'entry'. EOC marker is not set on returned entry.
 */
static __u32 determine_fatent(fsdata *mydata, __u32 entry)
{
	__u32 next_fat, next_entry = entry + 1;

	while (1) {
		next_fat = get_fatent(mydata, next_entry);
		if (next_fat == 0) {
			/* found free entry, link to entry */
			set_fatent_value(mydata, entry, next_entry);
			break;
		}
		next_entry++;
	}
	debug("FAT%d: entry: %08x, entry_value: %04x\n",
	       mydata->fatsize, entry, next_entry);

	return next_entry;
}

/**
 * set_sectors() - write data to sectors
 *
 * Write 'size' bytes from 'buffer' into the specified sector.
 *
 * @mydata:	data to be written
 * @startsect:	sector to be written to
 * @buffer:	data to be written
 * @size:	bytes to be written (but not more than the size of a cluster)
 * Return:	0 on success, -1 otherwise
 */
static int
set_sectors(fsdata *mydata, u32 startsect, u8 *buffer, u32 size)
{
	u32 nsects = 0;
	int ret;

	debug("startsect: %d\n", startsect);

	if ((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1)) {
		ALLOC_CACHE_ALIGN_BUFFER(__u8, tmpbuf, mydata->sect_size);

		debug("FAT: Misaligned buffer address (%p)\n", buffer);

		while (size >= mydata->sect_size) {
			memcpy(tmpbuf, buffer, mydata->sect_size);
			ret = disk_write(startsect++, 1, tmpbuf);
			if (ret != 1) {
				debug("Error writing data (got %d)\n", ret);
				return -1;
			}

			buffer += mydata->sect_size;
			size -= mydata->sect_size;
		}
	} else if (size >= mydata->sect_size) {
		nsects = size / mydata->sect_size;
		ret = disk_write(startsect, nsects, buffer);
		if (ret != nsects) {
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}

		startsect += nsects;
		buffer += nsects * mydata->sect_size;
		size -= nsects * mydata->sect_size;
	}

	if (size) {
		ALLOC_CACHE_ALIGN_BUFFER(__u8, tmpbuf, mydata->sect_size);
		/* Do not leak content of stack */
		memset(tmpbuf, 0, mydata->sect_size);
		memcpy(tmpbuf, buffer, size);
		ret = disk_write(startsect, 1, tmpbuf);
		if (ret != 1) {
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}
	}

	return 0;
}

/**
 * set_cluster() - write data to cluster
 *
 * Write 'size' bytes from 'buffer' into the specified cluster.
 *
 * @mydata:	data to be written
 * @clustnum:	cluster to be written to
 * @buffer:	data to be written
 * @size:	bytes to be written (but not more than the size of a cluster)
 * Return:	0 on success, -1 otherwise
 */
static int
set_cluster(fsdata *mydata, u32 clustnum, u8 *buffer, u32 size)
{
	return set_sectors(mydata, clust_to_sect(mydata, clustnum),
			   buffer, size);
}

static int
flush_dir(fat_itr *itr)
{
	fsdata *mydata = itr->fsdata;
	u32 startsect, sect_offset, nsects;

	if (!itr->is_root || mydata->fatsize == 32)
		return set_cluster(mydata, itr->clust, itr->block,
				   mydata->clust_size * mydata->sect_size);

	sect_offset = itr->clust * mydata->clust_size;
	startsect = mydata->rootdir_sect + sect_offset;
	/* do not write past the end of rootdir */
	nsects = min_t(u32, mydata->clust_size,
		       mydata->rootdir_size - sect_offset);

	return set_sectors(mydata, startsect, itr->block,
			   nsects * mydata->sect_size);
}

static __u8 tmpbuf_cluster[MAX_CLUSTSIZE] __aligned(ARCH_DMA_MINALIGN);

/*
 * Read and modify data on existing and consecutive cluster blocks
 */
static int
get_set_cluster(fsdata *mydata, __u32 clustnum, loff_t pos, __u8 *buffer,
		loff_t size, loff_t *gotsize)
{
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	__u32 startsect;
	loff_t wsize;
	int clustcount, i, ret;

	*gotsize = 0;
	if (!size)
		return 0;

	assert(pos < bytesperclust);
	startsect = clust_to_sect(mydata, clustnum);

	debug("clustnum: %d, startsect: %d, pos: %lld\n",
	      clustnum, startsect, pos);

	/* partial write at beginning */
	if (pos) {
		wsize = min(bytesperclust - pos, size);
		ret = disk_read(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size) {
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		memcpy(tmpbuf_cluster + pos, buffer, wsize);
		ret = disk_write(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size) {
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}

		size -= wsize;
		buffer += wsize;
		*gotsize += wsize;

		startsect += mydata->clust_size;

		if (!size)
			return 0;
	}

	/* full-cluster write */
	if (size >= bytesperclust) {
		clustcount = lldiv(size, bytesperclust);

		if (!((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1))) {
			wsize = clustcount * bytesperclust;
			ret = disk_write(startsect,
					 clustcount * mydata->clust_size,
					 buffer);
			if (ret != clustcount * mydata->clust_size) {
				debug("Error writing data (got %d)\n", ret);
				return -1;
			}

			size -= wsize;
			buffer += wsize;
			*gotsize += wsize;

			startsect += clustcount * mydata->clust_size;
		} else {
			for (i = 0; i < clustcount; i++) {
				memcpy(tmpbuf_cluster, buffer, bytesperclust);
				ret = disk_write(startsect,
						 mydata->clust_size,
						 tmpbuf_cluster);
				if (ret != mydata->clust_size) {
					debug("Error writing data (got %d)\n",
					      ret);
					return -1;
				}

				size -= bytesperclust;
				buffer += bytesperclust;
				*gotsize += bytesperclust;

				startsect += mydata->clust_size;
			}
		}
	}

	/* partial write at end */
	if (size) {
		wsize = size;
		ret = disk_read(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size) {
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}
		memcpy(tmpbuf_cluster, buffer, wsize);
		ret = disk_write(startsect, mydata->clust_size, tmpbuf_cluster);
		if (ret != mydata->clust_size) {
			debug("Error writing data (got %d)\n", ret);
			return -1;
		}

		size -= wsize;
		buffer += wsize;
		*gotsize += wsize;
	}

	assert(!size);

	return 0;
}

/*
 * Find the first empty cluster
 */
static int find_empty_cluster(fsdata *mydata)
{
	__u32 fat_val, entry = 3;

	while (1) {
		fat_val = get_fatent(mydata, entry);
		if (fat_val == 0)
			break;
		entry++;
	}

	return entry;
}

/*
 * Allocate a cluster for additional directory entries
 */
static int new_dir_table(fat_itr *itr)
{
	fsdata *mydata = itr->fsdata;
	int dir_newclust = 0;
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;

	dir_newclust = find_empty_cluster(mydata);
	set_fatent_value(mydata, itr->clust, dir_newclust);
	if (mydata->fatsize == 32)
		set_fatent_value(mydata, dir_newclust, 0xffffff8);
	else if (mydata->fatsize == 16)
		set_fatent_value(mydata, dir_newclust, 0xfff8);
	else if (mydata->fatsize == 12)
		set_fatent_value(mydata, dir_newclust, 0xff8);

	itr->clust = dir_newclust;
	itr->next_clust = dir_newclust;

	if (flush_dirty_fat_buffer(mydata) < 0)
		return -1;

	memset(itr->block, 0x00, bytesperclust);

	itr->dent = (dir_entry *)itr->block;
	itr->last_cluster = 1;
	itr->remaining = bytesperclust / sizeof(dir_entry) - 1;

	return 0;
}

/*
 * Set empty cluster from 'entry' to the end of a file
 */
static int clear_fatent(fsdata *mydata, __u32 entry)
{
	__u32 fat_val;

	while (!CHECK_CLUST(entry, mydata->fatsize)) {
		fat_val = get_fatent(mydata, entry);
		if (fat_val != 0)
			set_fatent_value(mydata, entry, 0);
		else
			break;

		entry = fat_val;
	}

	/* Flush fat buffer */
	if (flush_dirty_fat_buffer(mydata) < 0)
		return -1;

	return 0;
}

/*
 * Set start cluster in directory entry
 */
static void set_start_cluster(const fsdata *mydata, dir_entry *dentptr,
			      __u32 start_cluster)
{
	if (mydata->fatsize == 32)
		dentptr->starthi =
			cpu_to_le16((start_cluster & 0xffff0000) >> 16);
	dentptr->start = cpu_to_le16(start_cluster & 0xffff);
}

/*
 * Check whether adding a file makes the file system to
 * exceed the size of the block device
 * Return -1 when overflow occurs, otherwise return 0
 */
static int check_overflow(fsdata *mydata, __u32 clustnum, loff_t size)
{
	__u32 startsect, sect_num, offset;

	if (clustnum > 0)
		startsect = clust_to_sect(mydata, clustnum);
	else
		startsect = mydata->rootdir_sect;

	sect_num = div_u64_rem(size, mydata->sect_size, &offset);

	if (offset != 0)
		sect_num++;

	if (startsect + sect_num > total_sector)
		return -1;
	return 0;
}

/*
 * Write at most 'maxsize' bytes from 'buffer' into
 * the file associated with 'dentptr'
 * Update the number of bytes written in *gotsize and return 0
 * or return -1 on fatal errors.
 */
static int
set_contents(fsdata *mydata, dir_entry *dentptr, loff_t pos, __u8 *buffer,
	     loff_t maxsize, loff_t *gotsize)
{
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	__u32 curclust = START(dentptr);
	__u32 endclust = 0, newclust = 0;
	u64 cur_pos, filesize;
	loff_t offset, actsize, wsize;

	*gotsize = 0;
	filesize = pos + maxsize;

	debug("%llu bytes\n", filesize);

	if (!filesize) {
		if (!curclust)
			return 0;
		if (!CHECK_CLUST(curclust, mydata->fatsize) ||
		    IS_LAST_CLUST(curclust, mydata->fatsize)) {
			clear_fatent(mydata, curclust);
			set_start_cluster(mydata, dentptr, 0);
			return 0;
		}
		debug("curclust: 0x%x\n", curclust);
		debug("Invalid FAT entry\n");
		return -1;
	}

	if (!curclust) {
		assert(pos == 0);
		goto set_clusters;
	}

	/* go to cluster at pos */
	cur_pos = bytesperclust;
	while (1) {
		if (pos <= cur_pos)
			break;
		if (IS_LAST_CLUST(curclust, mydata->fatsize))
			break;

		newclust = get_fatent(mydata, curclust);
		if (!IS_LAST_CLUST(newclust, mydata->fatsize) &&
		    CHECK_CLUST(newclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			debug("Invalid FAT entry\n");
			return -1;
		}

		cur_pos += bytesperclust;
		curclust = newclust;
	}
	if (IS_LAST_CLUST(curclust, mydata->fatsize)) {
		assert(pos == cur_pos);
		goto set_clusters;
	}

	assert(pos < cur_pos);
	cur_pos -= bytesperclust;

	/* overwrite */
	assert(IS_LAST_CLUST(curclust, mydata->fatsize) ||
	       !CHECK_CLUST(curclust, mydata->fatsize));

	while (1) {
		/* search for allocated consecutive clusters */
		actsize = bytesperclust;
		endclust = curclust;
		while (1) {
			if (filesize <= (cur_pos + actsize))
				break;

			newclust = get_fatent(mydata, endclust);

			if (IS_LAST_CLUST(newclust, mydata->fatsize))
				break;
			if (CHECK_CLUST(newclust, mydata->fatsize)) {
				debug("curclust: 0x%x\n", curclust);
				debug("Invalid FAT entry\n");
				return -1;
			}

			actsize += bytesperclust;
			endclust = newclust;
		}

		/* overwrite to <curclust..endclust> */
		if (pos < cur_pos)
			offset = 0;
		else
			offset = pos - cur_pos;
		wsize = min(cur_pos + actsize, filesize) - pos;
		if (get_set_cluster(mydata, curclust, offset,
				    buffer, wsize, &actsize)) {
			printf("Error get-and-setting cluster\n");
			return -1;
		}
		buffer += wsize;
		*gotsize += wsize;
		cur_pos += offset + wsize;

		if (filesize <= cur_pos)
			break;

		/* CHECK: newclust = get_fatent(mydata, endclust); */

		if (IS_LAST_CLUST(newclust, mydata->fatsize))
			/* no more clusters */
			break;

		curclust = newclust;
	}

	if (filesize <= cur_pos) {
		/* no more write */
		newclust = get_fatent(mydata, endclust);
		if (!IS_LAST_CLUST(newclust, mydata->fatsize)) {
			/* truncate the rest */
			clear_fatent(mydata, newclust);

			/* Mark end of file in FAT */
			if (mydata->fatsize == 12)
				newclust = 0xfff;
			else if (mydata->fatsize == 16)
				newclust = 0xffff;
			else if (mydata->fatsize == 32)
				newclust = 0xfffffff;
			set_fatent_value(mydata, endclust, newclust);
		}

		return 0;
	}

	curclust = endclust;
	filesize -= cur_pos;
	assert(!do_div(cur_pos, bytesperclust));

set_clusters:
	/* allocate and write */
	assert(!pos);

	/* Assure that curclust is valid */
	if (!curclust) {
		curclust = find_empty_cluster(mydata);
		set_start_cluster(mydata, dentptr, curclust);
	} else {
		newclust = get_fatent(mydata, curclust);

		if (IS_LAST_CLUST(newclust, mydata->fatsize)) {
			newclust = determine_fatent(mydata, curclust);
			set_fatent_value(mydata, curclust, newclust);
			curclust = newclust;
		} else {
			debug("error: something wrong\n");
			return -1;
		}
	}

	/* TODO: already partially written */
	if (check_overflow(mydata, curclust, filesize)) {
		printf("Error: no space left: %llu\n", filesize);
		return -1;
	}

	actsize = bytesperclust;
	endclust = curclust;
	do {
		/* search for consecutive clusters */
		while (actsize < filesize) {
			newclust = determine_fatent(mydata, endclust);

			if ((newclust - 1) != endclust)
				/* write to <curclust..endclust> */
				goto getit;

			if (CHECK_CLUST(newclust, mydata->fatsize)) {
				debug("newclust: 0x%x\n", newclust);
				debug("Invalid FAT entry\n");
				return 0;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}

		/* set remaining bytes */
		actsize = filesize;
		if (set_cluster(mydata, curclust, buffer, (u32)actsize) != 0) {
			debug("error: writing cluster\n");
			return -1;
		}
		*gotsize += actsize;

		/* Mark end of file in FAT */
		if (mydata->fatsize == 12)
			newclust = 0xfff;
		else if (mydata->fatsize == 16)
			newclust = 0xffff;
		else if (mydata->fatsize == 32)
			newclust = 0xfffffff;
		set_fatent_value(mydata, endclust, newclust);

		return 0;
getit:
		if (set_cluster(mydata, curclust, buffer, (u32)actsize) != 0) {
			debug("error: writing cluster\n");
			return -1;
		}
		*gotsize += actsize;
		filesize -= actsize;
		buffer += actsize;

		if (CHECK_CLUST(newclust, mydata->fatsize)) {
			debug("newclust: 0x%x\n", newclust);
			debug("Invalid FAT entry\n");
			return 0;
		}
		actsize = bytesperclust;
		curclust = endclust = newclust;
	} while (1);

	return 0;
}

/*
 * Fill dir_entry
 */
static void fill_dentry(fsdata *mydata, dir_entry *dentptr,
	const char *filename, __u32 start_cluster, __u32 size, __u8 attr)
{
	set_start_cluster(mydata, dentptr, start_cluster);
	dentptr->size = cpu_to_le32(size);

	dentptr->attr = attr;

	set_name(dentptr, filename);
}

/*
 * Find a directory entry based on filename or start cluster number
 * If the directory entry is not found,
 * the new position for writing a directory entry will be returned
 */
static dir_entry *find_directory_entry(fat_itr *itr, char *filename)
{
	int match = 0;

	while (fat_itr_next(itr)) {
		/* check both long and short name: */
		if (!strcasecmp(filename, itr->name))
			match = 1;
		else if (itr->name != itr->s_name &&
			 !strcasecmp(filename, itr->s_name))
			match = 1;

		if (!match)
			continue;

		if (itr->dent->name[0] == '\0')
			return NULL;
		else
			return itr->dent;
	}

	/* allocate a cluster for more entries */
	if (!itr->dent &&
	    (!itr->is_root || itr->fsdata->fatsize == 32) &&
	    new_dir_table(itr))
		/* indicate that allocating dent failed */
		itr->dent = NULL;

	return NULL;
}

static int split_filename(char *filename, char **dirname, char **basename)
{
	char *p, *last_slash, *last_slash_cont;

again:
	p = filename;
	last_slash = NULL;
	last_slash_cont = NULL;
	while (*p) {
		if (ISDIRDELIM(*p)) {
			last_slash = p;
			last_slash_cont = p;
			/* continuous slashes */
			while (ISDIRDELIM(*p))
				last_slash_cont = p++;
			if (!*p)
				break;
		}
		p++;
	}

	if (last_slash) {
		if (last_slash_cont == (filename + strlen(filename) - 1)) {
			/* remove trailing slashes */
			*last_slash = '\0';
			goto again;
		}

		if (last_slash == filename) {
			/* avoid ""(null) directory */
			*dirname = "/";
		} else {
			*last_slash = '\0';
			*dirname = filename;
		}

		*last_slash_cont = '\0';
		*basename = last_slash_cont + 1;
	} else {
		*dirname = "/"; /* root by default */
		*basename = filename;
	}

	return 0;
}

/**
 * normalize_longname() - check long file name and convert to lower case
 *
 * We assume here that the FAT file system is using an 8bit code page.
 * Linux typically uses CP437, EDK2 assumes CP1250.
 *
 * @l_filename:	preallocated buffer receiving the normalized name
 * @filename:	filename to normalize
 * Return:	0 on success, -1 on failure
 */
static int normalize_longname(char *l_filename, const char *filename)
{
	const char *p, illegal[] = "<>:\"/\\|?*";

	if (strlen(filename) >= VFAT_MAXLEN_BYTES)
		return -1;

	for (p = filename; *p; ++p) {
		if ((unsigned char)*p < 0x20)
			return -1;
		if (strchr(illegal, *p))
			return -1;
	}

	strcpy(l_filename, filename);
	downcase(l_filename, VFAT_MAXLEN_BYTES);

	return 0;
}

int file_fat_write_at(const char *filename, loff_t pos, void *buffer,
		      loff_t size, loff_t *actwrite)
{
	dir_entry *retdent;
	fsdata datablock = { .fatbuf = NULL, };
	fsdata *mydata = &datablock;
	fat_itr *itr = NULL;
	int ret = -1;
	char *filename_copy, *parent, *basename;
	char l_filename[VFAT_MAXLEN_BYTES];

	debug("writing %s\n", filename);

	filename_copy = strdup(filename);
	if (!filename_copy)
		return -ENOMEM;

	split_filename(filename_copy, &parent, &basename);
	if (!strlen(basename)) {
		ret = -EINVAL;
		goto exit;
	}

	filename = basename;
	if (normalize_longname(l_filename, filename)) {
		printf("FAT: illegal filename (%s)\n", filename);
		ret = -EINVAL;
		goto exit;
	}

	itr = malloc_cache_aligned(sizeof(fat_itr));
	if (!itr) {
		ret = -ENOMEM;
		goto exit;
	}

	ret = fat_itr_root(itr, &datablock);
	if (ret)
		goto exit;

	total_sector = datablock.total_sect;

	ret = fat_itr_resolve(itr, parent, TYPE_DIR);
	if (ret) {
		printf("%s: doesn't exist (%d)\n", parent, ret);
		goto exit;
	}

	retdent = find_directory_entry(itr, l_filename);

	if (retdent) {
		if (fat_itr_isdir(itr)) {
			ret = -EISDIR;
			goto exit;
		}

		/* A file exists */
		if (pos == -1)
			/* Append to the end */
			pos = FAT2CPU32(retdent->size);
		if (pos > retdent->size) {
			/* No hole allowed */
			ret = -EINVAL;
			goto exit;
		}

		/* Update file size in a directory entry */
		retdent->size = cpu_to_le32(pos + size);
	} else {
		/* Create a new file */

		if (itr->is_root) {
			/* root dir cannot have "." or ".." */
			if (!strcmp(l_filename, ".") ||
			    !strcmp(l_filename, "..")) {
				ret = -EINVAL;
				goto exit;
			}
		}

		if (!itr->dent) {
			printf("Error: allocating new dir entry\n");
			ret = -EIO;
			goto exit;
		}

		if (pos) {
			/* No hole allowed */
			ret = -EINVAL;
			goto exit;
		}

		memset(itr->dent, 0, sizeof(*itr->dent));

		/* Calculate checksum for short name */
		set_name(itr->dent, filename);

		/* Set long name entries */
		if (fill_dir_slot(itr, filename)) {
			ret = -EIO;
			goto exit;
		}

		/* Set short name entry */
		fill_dentry(itr->fsdata, itr->dent, filename, 0, size, 0x20);

		retdent = itr->dent;
	}

	ret = set_contents(mydata, retdent, pos, buffer, size, actwrite);
	if (ret < 0) {
		printf("Error: writing contents\n");
		ret = -EIO;
		goto exit;
	}
	debug("attempt to write 0x%llx bytes\n", *actwrite);

	/* Flush fat buffer */
	ret = flush_dirty_fat_buffer(mydata);
	if (ret) {
		printf("Error: flush fat buffer\n");
		ret = -EIO;
		goto exit;
	}

	/* Write directory table to device */
	ret = flush_dir(itr);
	if (ret) {
		printf("Error: writing directory entry\n");
		ret = -EIO;
	}

exit:
	free(filename_copy);
	free(mydata->fatbuf);
	free(itr);
	return ret;
}

int file_fat_write(const char *filename, void *buffer, loff_t offset,
		   loff_t maxsize, loff_t *actwrite)
{
	return file_fat_write_at(filename, offset, buffer, maxsize, actwrite);
}

static int fat_dir_entries(fat_itr *itr)
{
	fat_itr *dirs;
	fsdata fsdata = { .fatbuf = NULL, }, *mydata = &fsdata;
						/* for FATBUFSIZE */
	int count;

	dirs = malloc_cache_aligned(sizeof(fat_itr));
	if (!dirs) {
		debug("Error: allocating memory\n");
		count = -ENOMEM;
		goto exit;
	}

	/* duplicate fsdata */
	fat_itr_child(dirs, itr);
	fsdata = *dirs->fsdata;

	/* allocate local fat buffer */
	fsdata.fatbuf = malloc_cache_aligned(FATBUFSIZE);
	if (!fsdata.fatbuf) {
		debug("Error: allocating memory\n");
		count = -ENOMEM;
		goto exit;
	}
	fsdata.fatbufnum = -1;
	dirs->fsdata = &fsdata;

	for (count = 0; fat_itr_next(dirs); count++)
		;

exit:
	free(fsdata.fatbuf);
	free(dirs);
	return count;
}

static int delete_dentry(fat_itr *itr)
{
	fsdata *mydata = itr->fsdata;
	dir_entry *dentptr = itr->dent;

	/* free cluster blocks */
	clear_fatent(mydata, START(dentptr));
	if (flush_dirty_fat_buffer(mydata) < 0) {
		printf("Error: flush fat buffer\n");
		return -EIO;
	}

	/*
	 * update a directory entry
	 * TODO:
	 *  - long file name support
	 *  - find and mark the "new" first invalid entry as name[0]=0x00
	 */
	memset(dentptr, 0, sizeof(*dentptr));
	dentptr->name[0] = 0xe5;

	if (flush_dir(itr)) {
		printf("error: writing directory entry\n");
		return -EIO;
	}

	return 0;
}

int fat_unlink(const char *filename)
{
	fsdata fsdata = { .fatbuf = NULL, };
	fat_itr *itr = NULL;
	int n_entries, ret;
	char *filename_copy, *dirname, *basename;

	filename_copy = strdup(filename);
	if (!filename_copy) {
		printf("Error: allocating memory\n");
		ret = -ENOMEM;
		goto exit;
	}
	split_filename(filename_copy, &dirname, &basename);

	if (!strcmp(dirname, "/") && !strcmp(basename, "")) {
		printf("Error: cannot remove root\n");
		ret = -EINVAL;
		goto exit;
	}

	itr = malloc_cache_aligned(sizeof(fat_itr));
	if (!itr) {
		printf("Error: allocating memory\n");
		ret = -ENOMEM;
		goto exit;
	}

	ret = fat_itr_root(itr, &fsdata);
	if (ret)
		goto exit;

	total_sector = fsdata.total_sect;

	ret = fat_itr_resolve(itr, dirname, TYPE_DIR);
	if (ret) {
		printf("%s: doesn't exist (%d)\n", dirname, ret);
		ret = -ENOENT;
		goto exit;
	}

	if (!find_directory_entry(itr, basename)) {
		printf("%s: doesn't exist\n", basename);
		ret = -ENOENT;
		goto exit;
	}

	if (fat_itr_isdir(itr)) {
		n_entries = fat_dir_entries(itr);
		if (n_entries < 0) {
			ret = n_entries;
			goto exit;
		}
		if (n_entries > 2) {
			printf("Error: directory is not empty: %d\n",
			       n_entries);
			ret = -EINVAL;
			goto exit;
		}
	}

	ret = delete_dentry(itr);

exit:
	free(fsdata.fatbuf);
	free(itr);
	free(filename_copy);

	return ret;
}

int fat_mkdir(const char *new_dirname)
{
	dir_entry *retdent;
	fsdata datablock = { .fatbuf = NULL, };
	fsdata *mydata = &datablock;
	fat_itr *itr = NULL;
	char *dirname_copy, *parent, *dirname;
	char l_dirname[VFAT_MAXLEN_BYTES];
	int ret = -1;
	loff_t actwrite;
	unsigned int bytesperclust;
	dir_entry *dotdent = NULL;

	dirname_copy = strdup(new_dirname);
	if (!dirname_copy)
		goto exit;

	split_filename(dirname_copy, &parent, &dirname);
	if (!strlen(dirname)) {
		ret = -EINVAL;
		goto exit;
	}

	if (normalize_longname(l_dirname, dirname)) {
		printf("FAT: illegal filename (%s)\n", dirname);
		ret = -EINVAL;
		goto exit;
	}

	itr = malloc_cache_aligned(sizeof(fat_itr));
	if (!itr) {
		ret = -ENOMEM;
		goto exit;
	}

	ret = fat_itr_root(itr, &datablock);
	if (ret)
		goto exit;

	total_sector = datablock.total_sect;

	ret = fat_itr_resolve(itr, parent, TYPE_DIR);
	if (ret) {
		printf("%s: doesn't exist (%d)\n", parent, ret);
		goto exit;
	}

	retdent = find_directory_entry(itr, l_dirname);

	if (retdent) {
		printf("%s: already exists\n", l_dirname);
		ret = -EEXIST;
		goto exit;
	} else {
		if (itr->is_root) {
			/* root dir cannot have "." or ".." */
			if (!strcmp(l_dirname, ".") ||
			    !strcmp(l_dirname, "..")) {
				ret = -EINVAL;
				goto exit;
			}
		}

		if (!itr->dent) {
			printf("Error: allocating new dir entry\n");
			ret = -EIO;
			goto exit;
		}

		memset(itr->dent, 0, sizeof(*itr->dent));

		/* Set short name to set alias checksum field in dir_slot */
		set_name(itr->dent, dirname);
		fill_dir_slot(itr, dirname);

		/* Set attribute as archive for regular file */
		fill_dentry(itr->fsdata, itr->dent, dirname, 0, 0,
			    ATTR_DIR | ATTR_ARCH);

		retdent = itr->dent;
	}

	/* Default entries */
	bytesperclust = mydata->clust_size * mydata->sect_size;
	dotdent = malloc_cache_aligned(bytesperclust);
	if (!dotdent) {
		ret = -ENOMEM;
		goto exit;
	}
	memset(dotdent, 0, bytesperclust);

	memcpy(dotdent[0].name, ".       ", 8);
	memcpy(dotdent[0].ext, "   ", 3);
	dotdent[0].attr = ATTR_DIR | ATTR_ARCH;

	memcpy(dotdent[1].name, "..      ", 8);
	memcpy(dotdent[1].ext, "   ", 3);
	dotdent[1].attr = ATTR_DIR | ATTR_ARCH;
	set_start_cluster(mydata, &dotdent[1], itr->start_clust);

	ret = set_contents(mydata, retdent, 0, (__u8 *)dotdent,
			   bytesperclust, &actwrite);
	if (ret < 0) {
		printf("Error: writing contents\n");
		goto exit;
	}
	/* Write twice for "." */
	set_start_cluster(mydata, &dotdent[0], START(retdent));
	ret = set_contents(mydata, retdent, 0, (__u8 *)dotdent,
			   bytesperclust, &actwrite);
	if (ret < 0) {
		printf("Error: writing contents\n");
		goto exit;
	}

	/* Flush fat buffer */
	ret = flush_dirty_fat_buffer(mydata);
	if (ret) {
		printf("Error: flush fat buffer\n");
		goto exit;
	}

	/* Write directory table to device */
	ret = flush_dir(itr);
	if (ret)
		printf("Error: writing directory entry\n");

exit:
	free(dirname_copy);
	free(mydata->fatbuf);
	free(itr);
	free(dotdent);
	return ret;
}
