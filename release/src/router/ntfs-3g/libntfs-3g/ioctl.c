/**
 * ioctl.c - Processing of ioctls
 *
 *      This module is part of ntfs-3g library
 *
 * Copyright (c) 2014-2019 Jean-Pierre Andre
 * Copyright (c) 2014      Red Hat, Inc.
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <syslog.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_LINUX_FS_H
#include <linux/fs.h>
#endif

#include "compat.h"
#include "debug.h"
#include "bitmap.h"
#include "attrib.h"
#include "inode.h"
#include "layout.h"
#include "volume.h"
#include "index.h"
#include "logging.h"
#include "ntfstime.h"
#include "unistr.h"
#include "dir.h"
#include "security.h"
#include "ioctl.h"
#include "misc.h"

#if defined(FITRIM) && defined(BLKDISCARD)

/* Issue a TRIM request to the underlying device for the given clusters. */
static int fstrim_clusters(ntfs_volume *vol, LCN lcn, s64 length)
{
	struct ntfs_device *dev = vol->dev;
	uint64_t range[2];

	ntfs_log_debug("fstrim_clusters: %lld length %lld\n",
		(long long) lcn, (long long) length);

	range[0] = lcn << vol->cluster_size_bits;
	range[1] = length << vol->cluster_size_bits;

	if (dev->d_ops->ioctl(dev, BLKDISCARD, range) == -1) {
		ntfs_log_debug("fstrim_one_cluster: ioctl failed: %m\n");
		return -errno;
	}
	return 0;
}

static int read_line(const char *path, char *line, size_t max_bytes)
{
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL)
		return -errno;
	if (fgets(line, max_bytes, fp) == NULL) {
		int ret = -EIO; /* fgets doesn't set errno */
		fclose(fp);
		return ret;
	}
	fclose (fp);
	return 0;
}

static int read_u64(const char *path, u64 *n)
{
	char line[64];
	int ret;

	ret = read_line(path, line, sizeof line);
	if (ret)
		return ret;
	if (sscanf(line, "%" SCNu64, n) != 1)
		return -EINVAL;
	return 0;
}

/* Find discard limits for current backing device.
 */
static int fstrim_limits(ntfs_volume *vol,
			u64 *discard_alignment,
			u64 *discard_granularity,
			u64 *discard_max_bytes)
{
	struct stat statbuf;
	char path1[40]; /* holds "/sys/dev/block/%d:%d" */
	char path2[40 + sizeof(path1)]; /* less than 40 bytes more than path1 */
	int ret;

	/* Stat the backing device.  Caller has ensured it is a block device. */
	if (stat(vol->dev->d_name, &statbuf) == -1) {
		ntfs_log_debug("fstrim_limits: could not stat %s\n",
			vol->dev->d_name);
		return -errno;
	}

	/* For whole devices,
	 * /sys/dev/block/MAJOR:MINOR/discard_alignment
	 * /sys/dev/block/MAJOR:MINOR/queue/discard_granularity
	 * /sys/dev/block/MAJOR:MINOR/queue/discard_max_bytes
	 * will exist.
	 * For partitions, we also need to check the parent device:
	 * /sys/dev/block/MAJOR:MINOR/../queue/discard_granularity
	 * /sys/dev/block/MAJOR:MINOR/../queue/discard_max_bytes
	 */
	snprintf(path1, sizeof path1, "/sys/dev/block/%d:%d",
		major(statbuf.st_rdev), minor(statbuf.st_rdev));

	snprintf(path2, sizeof path2, "%s/discard_alignment", path1);
	ret = read_u64(path2, discard_alignment);
	if (ret) {
		if (ret != -ENOENT)
			return ret;
		else
			/* We would expect this file to exist on all
			 * modern kernels.  But for the sake of very
			 * old kernels:
			 */
			goto not_found;
	}

	snprintf(path2, sizeof path2, "%s/queue/discard_granularity", path1);
	ret = read_u64(path2, discard_granularity);
	if (ret) {
		if (ret != -ENOENT)
			return ret;
		else {
			snprintf(path2, sizeof path2,
				"%s/../queue/discard_granularity", path1);
			ret = read_u64(path2, discard_granularity);
			if (ret) {
				if (ret != -ENOENT)
					return ret;
				else
					goto not_found;
			}
		}
	}

	snprintf(path2, sizeof path2, "%s/queue/discard_max_bytes", path1);
	ret = read_u64(path2, discard_max_bytes);
	if (ret) {
		if (ret != -ENOENT)
			return ret;
		else {
			snprintf(path2, sizeof path2,
				"%s/../queue/discard_max_bytes", path1);
			ret = read_u64(path2, discard_max_bytes);
			if (ret) {
				if (ret != -ENOENT)
					return ret;
				else
					goto not_found;
			}
		}
	}

	return 0;

not_found:
	/* If we reach here then we didn't find the device.  This is
	 * not an error, but set discard_max_bytes = 0 to indicate
	 * that discard is not available.
	 */
	*discard_alignment = 0;
	*discard_granularity = 0;
	*discard_max_bytes = 0;
	return 0;
}

static inline LCN align_up(ntfs_volume *vol, LCN lcn, u64 granularity)
{
	u64 aligned;

	aligned = (lcn << vol->cluster_size_bits) + granularity - 1;
	aligned -= aligned % granularity;
	return (aligned >> vol->cluster_size_bits);
}

static inline u64 align_down(ntfs_volume *vol, u64 count, u64 granularity)
{
	u64 aligned;

	aligned = count << vol->cluster_size_bits;
	aligned -= aligned % granularity;
	return (aligned >> vol->cluster_size_bits);
}

#define FSTRIM_BUFSIZ 4096

/* Trim the filesystem.
 *
 * Free blocks between 'start' and 'start+len-1' (both byte offsets)
 * are found and TRIM requests are sent to the block device.  'minlen'
 * is the minimum continguous free range to discard.
 */
static int fstrim(ntfs_volume *vol, void *data, u64 *trimmed)
{
	struct fstrim_range *range = data;
	u64 start = range->start;
	u64 len = range->len;
	u64 minlen = range->minlen;
	u64 discard_alignment, discard_granularity, discard_max_bytes;
	u8 *buf = NULL;
	LCN start_buf;
	int ret;

	ntfs_log_debug("fstrim: start=%llu len=%llu minlen=%llu\n",
		(unsigned long long) start,
		(unsigned long long) len,
		(unsigned long long) minlen);

	*trimmed = 0;

	/* Fail if user tries to use the fstrim -o/-l/-m options.
	 * XXX We could fix these limitations in future.
	 */
	if (start != 0 || len != (uint64_t)-1) {
		ntfs_log_error("fstrim: setting start or length is not supported\n");
		return -EINVAL;
	}
	if (minlen > vol->cluster_size) {
		ntfs_log_error("fstrim: minlen > cluster size is not supported\n");
		return -EINVAL;
	}

	/* Only block devices are supported.  It would be possible to
	 * support backing files (ie. without using loop) but the
	 * ioctls used to punch holes in files are completely
	 * different.
	 */
	if (!NDevBlock(vol->dev)) {
		ntfs_log_error("fstrim: not supported for non-block-device\n");
		return -EOPNOTSUPP;
	}

	ret = fstrim_limits(vol, &discard_alignment,
			&discard_granularity, &discard_max_bytes);
	if (ret)
		return ret;
	if (discard_alignment != 0) {
		ntfs_log_error("fstrim: backing device is not aligned for discards\n");
		return -EOPNOTSUPP;
	}

	if (discard_max_bytes == 0) {
		ntfs_log_error("fstrim: backing device does not support discard (discard_max_bytes == 0)\n");
		return -EOPNOTSUPP;
	}

	/* Sync the device before doing anything. */
	ret = ntfs_device_sync(vol->dev);
	if (ret)
		return ret;

	/* Read through the bitmap. */
	buf = ntfs_malloc(FSTRIM_BUFSIZ);
	if (buf == NULL)
		return -errno;
	for (start_buf = 0; start_buf < vol->nr_clusters;
	     start_buf += FSTRIM_BUFSIZ * 8) {
		s64 count;
		s64 br;
		LCN end_buf, start_lcn;

		/* start_buf is LCN of first cluster in the current buffer.
		 * end_buf is LCN of last cluster + 1 in the current buffer.
		 */
		end_buf = start_buf + FSTRIM_BUFSIZ*8;
		if (end_buf > vol->nr_clusters)
			end_buf = vol->nr_clusters;
		count = (end_buf - start_buf) / 8;

		br = ntfs_attr_pread(vol->lcnbmp_na, start_buf/8, count, buf);
		if (br != count) {
			if (br >= 0)
				ret = -EIO;
			else
				ret = -errno;
			goto free_out;
		}

		/* Trim the clusters in large as possible blocks, but
		 * not larger than discard_max_bytes, and compatible
		 * with the supported trim granularity.
		 */
		for (start_lcn = start_buf; start_lcn < end_buf; ++start_lcn) {
			if (!ntfs_bit_get(buf, start_lcn-start_buf)) {
				LCN end_lcn;
				LCN aligned_lcn;
				u64 aligned_count;

				/* Cluster 'start_lcn' is not in use,
				 * find end of this run.
				 */
				end_lcn = start_lcn+1;
				while (end_lcn < end_buf &&
					(u64) (end_lcn-start_lcn) << vol->cluster_size_bits
					  < discard_max_bytes &&
					!ntfs_bit_get(buf, end_lcn-start_buf))
					end_lcn++;
				aligned_lcn = align_up(vol, start_lcn,
						discard_granularity);
				if (aligned_lcn >= end_lcn)
					aligned_count = 0;
				else {
					aligned_count = 
						align_down(vol,
							end_lcn - aligned_lcn,
							discard_granularity);
				}
				if (aligned_count) {
					ret = fstrim_clusters(vol,
						aligned_lcn, aligned_count);
					if (ret)
						goto free_out;

					*trimmed += aligned_count
						<< vol->cluster_size_bits;
				}
				start_lcn = end_lcn-1;
			}
		}
	}

	ret = 0;
free_out:
	free(buf);
	return ret;
}

#endif /* FITRIM && BLKDISCARD */

int ntfs_ioctl(ntfs_inode *ni, unsigned long cmd,
			void *arg __attribute__((unused)),
			unsigned int flags __attribute__((unused)), void *data)
{
	int ret = 0;

	switch (cmd) {
#if defined(FITRIM) && defined(BLKDISCARD)
	case FITRIM:
		if (!ni || !data)
			ret = -EINVAL;
		else {
			u64 trimmed;
			struct fstrim_range *range = (struct fstrim_range*)data;

			ret = fstrim(ni->vol, data, &trimmed);
			range->len = trimmed;
		}
		break;
#else
#warning Trimming not supported : FITRIM or BLKDISCARD not defined
#endif
	default :
		ret = -EINVAL;
		break;
	}
	return (ret);
}
