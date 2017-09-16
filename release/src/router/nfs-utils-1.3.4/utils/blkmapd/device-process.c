/*
 * device-process.c: detailed processing of device information sent
 * from kernel.
 *
 * Copyright (c) 2006 The Regents of the University of Michigan.
 * All rights reserved.
 *
 *  Andy Adamson <andros@citi.umich.edu>
 *  Fred Isaman <iisaman@umich.edu>
 *
 * Copyright (c) 2010 EMC Corporation, Haiying Tang <Tang_Haiying@emc.com>
 *
 * Used codes in linux/fs/nfs/blocklayout/blocklayoutdev.c.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <arpa/inet.h>
#include <linux/kdev_t.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>

#include "device-discovery.h"

uint32_t *blk_overflow(uint32_t * p, uint32_t * end, size_t nbytes)
{
	uint32_t *q = p + ((nbytes + 3) >> 2);

	if (q > end || q < p)
		return NULL;
	return p;
}

static int decode_blk_signature(uint32_t **pp, uint32_t * end,
				struct bl_sig *sig)
{
	int i;
	uint32_t siglen, *p = *pp;

	BLK_READBUF(p, end, 4);
	READ32(sig->si_num_comps);
	if (sig->si_num_comps == 0) {
		BL_LOG_ERR("0 components in sig\n");
		goto out_err;
	}
	if (sig->si_num_comps >= BLOCK_MAX_SIG_COMP) {
		BL_LOG_ERR("number of sig comps %i >= BLOCK_MAX_SIG_COMP\n",
			   sig->si_num_comps);
		goto out_err;
	}
	for (i = 0; i < sig->si_num_comps; i++) {
		struct bl_sig_comp *comp = &sig->si_comps[i];

		BLK_READBUF(p, end, 12);
		READ64(comp->bs_offset);
		READ32(siglen);
		comp->bs_length = siglen;
		BLK_READBUF(p, end, siglen);
		/* Note we rely here on fact that sig is used immediately
		 * for mapping, then thrown away.
		 */
		comp->bs_string = (char *)p;
		p += ((siglen + 3) >> 2);
	}
	*pp = p;
	return 0;
 out_err:
	return -EIO;
}

/*
 * Read signature from device and compare to sig_comp
 * return: 0=match, 1=no match, -1=error
 */
static int
read_cmp_blk_sig(struct bl_disk *disk, int fd, struct bl_sig_comp *comp)
{
	const char *dev_name = disk->valid_path->full_path;
	int ret = -1;
	ssize_t siglen = comp->bs_length;
	int64_t bs_offset = comp->bs_offset;
	char *sig = NULL;

	sig = (char *)malloc(siglen);
	if (!sig) {
		BL_LOG_ERR("%s: Out of memory\n", __func__);
		goto out;
	}

	if (bs_offset < 0)
		bs_offset += (((int64_t) disk->size) << 9);
	if (lseek64(fd, bs_offset, SEEK_SET) == -1) {
		BL_LOG_ERR("File %s lseek error\n", dev_name);
		goto out;
	}

	if (read(fd, sig, siglen) != siglen) {
		BL_LOG_ERR("File %s read error\n", dev_name);
		goto out;
	}

	ret = memcmp(sig, comp->bs_string, siglen);

 out:
	if (sig)
		free(sig);
	return ret;
}

/*
 * All signatures in sig must be found on disk for verification.
 * Returns True if sig matches, False otherwise.
 */
static int verify_sig(struct bl_disk *disk, struct bl_sig *sig)
{
	const char *dev_name = disk->valid_path->full_path;
	int fd, i, rv;

	fd = open(dev_name, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		BL_LOG_ERR("%s: %s could not be opened for read\n", __func__,
			   dev_name);
		return 0;
	}

	rv = 1;

	for (i = 0; i < sig->si_num_comps; i++) {
		if (read_cmp_blk_sig(disk, fd, &sig->si_comps[i])) {
			rv = 0;
			break;
		}
	}

	if (fd >= 0)
		close(fd);
	return rv;
}

/*
 * map_sig_to_device()
 * Given a signature, walk the list of visible disks searching for
 * a match. Returns True if mapping was done, False otherwise.
 *
 * While we're at it, fill in the vol->bv_size.
 */
static int map_sig_to_device(struct bl_sig *sig, struct bl_volume *vol)
{
	int mapped = 0;
	struct bl_disk *disk;

	/* scan disk list to find out match device */
	for (disk = visible_disk_list; disk; disk = disk->next) {
		/* FIXME: should we use better algorithm for disk scan? */
		mapped = verify_sig(disk, sig);
		if (mapped) {
			BL_LOG_INFO("%s: using device %s\n",
					__func__, disk->valid_path->full_path);
			vol->param.bv_dev = disk->dev;
			vol->bv_size = disk->size;
			break;
		}
	}
	return mapped;
}

/* We are given an array of XDR encoded array indices, each of which should
 * refer to a previously decoded device.  Translate into a list of pointers
 * to the appropriate pnfs_blk_volume's.
 */
static int set_vol_array(uint32_t **pp, uint32_t *end,
			 struct bl_volume *vols, int working)
{
	int i, index;
	uint32_t *p = *pp;
	struct bl_volume **array = vols[working].bv_vols;

	for (i = 0; i < vols[working].bv_vol_n; i++) {
		BLK_READBUF(p, end, 4);
		READ32(index);
		if ((index < 0) || (index >= working)) {
			BL_LOG_ERR("set_vol_array: Id %i out of range\n",
				   index);
			goto out_err;
		}
		array[i] = &vols[index];
	}
	*pp = p;
	return 0;
 out_err:
	return -EIO;
}

static uint64_t sum_subvolume_sizes(struct bl_volume *vol)
{
	int i;
	uint64_t sum = 0;

	for (i = 0; i < vol->bv_vol_n; i++)
		sum += vol->bv_vols[i]->bv_size;
	return sum;
}

static int
decode_blk_volume(uint32_t **pp, uint32_t *end, struct bl_volume *vols, int voln,
		  int *array_cnt)
{
	int status = 0, j;
	struct bl_sig sig;
	uint32_t *p = *pp;
	struct bl_volume *vol = &vols[voln];
	uint64_t tmp;

	BLK_READBUF(p, end, 4);
	READ32(vol->bv_type);

	switch (vol->bv_type) {
	case BLOCK_VOLUME_SIMPLE:
		*array_cnt = 0;
		status = decode_blk_signature(&p, end, &sig);
		if (status)
			return status;
		status = map_sig_to_device(&sig, vol);
		if (!status) {
			BL_LOG_ERR("Could not find disk for device\n");
			return -ENXIO;
		}
		BL_LOG_INFO("%s: simple %d\n", __func__, voln);
		status = 0;
		break;
	case BLOCK_VOLUME_SLICE:
		BLK_READBUF(p, end, 16);
		READ_SECTOR(vol->param.bv_offset);
		READ_SECTOR(vol->bv_size);
		*array_cnt = vol->bv_vol_n = 1;
		BL_LOG_INFO("%s: slice %d\n", __func__, voln);
		status = set_vol_array(&p, end, vols, voln);
		break;
	case BLOCK_VOLUME_STRIPE:
		BLK_READBUF(p, end, 8);
		READ_SECTOR(vol->param.bv_stripe_unit);
		off_t stripe_unit = vol->param.bv_stripe_unit;
		/* Check limitations imposed by device-mapper */
		if ((stripe_unit & (stripe_unit - 1)) != 0
		    || stripe_unit < (off_t) (sysconf(_SC_PAGE_SIZE) >> 9))
			return -EIO;
		BLK_READBUF(p, end, 4);
		READ32(vol->bv_vol_n);
		if (!vol->bv_vol_n)
			return -EIO;
		*array_cnt = vol->bv_vol_n;
		BL_LOG_INFO("%s: stripe %d nvols=%d unit=%ld\n", __func__, voln,
			    vol->bv_vol_n, (long)stripe_unit);
		status = set_vol_array(&p, end, vols, voln);
		if (status)
			return status;
		for (j = 1; j < vol->bv_vol_n; j++) {
			if (vol->bv_vols[j]->bv_size !=
			    vol->bv_vols[0]->bv_size) {
				BL_LOG_ERR("varying subvol size\n");
				return -EIO;
			}
		}
		vol->bv_size = vol->bv_vols[0]->bv_size * vol->bv_vol_n;
		break;
	case BLOCK_VOLUME_CONCAT:
		BLK_READBUF(p, end, 4);
		READ32(vol->bv_vol_n);
		if (!vol->bv_vol_n)
			return -EIO;
		*array_cnt = vol->bv_vol_n;
		BL_LOG_INFO("%s: concat %d %d\n", __func__, voln,
			    vol->bv_vol_n);
		status = set_vol_array(&p, end, vols, voln);
		if (status)
			return status;
		vol->bv_size = sum_subvolume_sizes(vol);
		break;
	default:
		BL_LOG_ERR("Unknown volume type %i\n", vol->bv_type);
 out_err:
		return -EIO;
	}
	*pp = p;
	return status;
}

uint64_t process_deviceinfo(const char *dev_addr_buf,
			    unsigned int dev_addr_len,
			    uint32_t *major, uint32_t *minor)
{
	int num_vols, i, status, count;
	uint32_t *p, *end;
	struct bl_volume *vols = NULL, **arrays = NULL, **arrays_ptr = NULL;
	uint64_t dev = 0;

	p = (uint32_t *) dev_addr_buf;
	end = (uint32_t *) ((char *)p + dev_addr_len);

	/* Decode block volume */
	BLK_READBUF(p, end, 4);
	READ32(num_vols);
	BL_LOG_INFO("%s: %d vols\n", __func__, num_vols);
	if (num_vols <= 0)
		goto out_err;

	vols = (struct bl_volume *)malloc(num_vols * sizeof(struct bl_volume));
	if (!vols) {
		BL_LOG_ERR("%s: Out of memory\n", __func__);
		goto out_err;
	}

	/* Each volume in vols array needs its own array.  Save time by
	 * allocating them all in one large hunk.  Because each volume
	 * array can only reference previous volumes, and because once
	 * a concat or stripe references a volume, it may never be
	 * referenced again, the volume arrays are guaranteed to fit
	 * in the suprisingly small space allocated.
	 */
	arrays_ptr = arrays =
	    (struct bl_volume **)malloc(num_vols * 2 *
					sizeof(struct bl_volume *));
	if (!arrays) {
		BL_LOG_ERR("%s: Out of memory\n", __func__);
		goto out_err;
	}

	for (i = 0; i < num_vols; i++) {
		vols[i].bv_vols = arrays_ptr;
		status = decode_blk_volume(&p, end, vols, i, &count);
		if (status)
			goto out_err;
		arrays_ptr += count;
	}

	if (p != end) {
		BL_LOG_ERR("p is not equal to end!\n");
		goto out_err;
	}

	dev = dm_device_create(vols, num_vols);
	if (dev) {
		*major = MAJOR(dev);
		*minor = MINOR(dev);
	}

 out_err:
	if (vols)
		free(vols);
	if (arrays)
		free(arrays);
	return dev;
}
