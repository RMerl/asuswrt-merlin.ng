/*
 * Copyright (c) 2017 Oracle.
 * All Rights Reserved.
 *
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef FSMAP_H_
#define FSMAP_H_

/* FS_IOC_GETFSMAP ioctl definitions */
#ifndef FS_IOC_GETFSMAP
struct fsmap {
	__u32		fmr_device;	/* device id */
	__u32		fmr_flags;	/* mapping flags */
	__u64		fmr_physical;	/* device offset of segment */
	__u64		fmr_owner;	/* owner id */
	__u64		fmr_offset;	/* file offset of segment */
	__u64		fmr_length;	/* length of segment */
	__u64		fmr_reserved[3];	/* must be zero */
};

struct fsmap_head {
	__u32		fmh_iflags;	/* control flags */
	__u32		fmh_oflags;	/* output flags */
	__u32		fmh_count;	/* # of entries in array incl. input */
	__u32		fmh_entries;	/* # of entries filled in (output). */
	__u64		fmh_reserved[6];	/* must be zero */

	struct fsmap	fmh_keys[2];	/* low and high keys for the mapping search */
	struct fsmap	fmh_recs[];	/* returned records */
};

/* Size of an fsmap_head with room for nr records. */
static inline size_t
fsmap_sizeof(
	unsigned int	nr)
{
	return sizeof(struct fsmap_head) + nr * sizeof(struct fsmap);
}

/* Start the next fsmap query at the end of the current query results. */
static inline void
fsmap_advance(
	struct fsmap_head	*head)
{
	head->fmh_keys[0] = head->fmh_recs[head->fmh_entries - 1];
}

/*	fmh_iflags values - set by FS_IOC_GETFSMAP caller in the header. */
/* no flags defined yet */
#define FMH_IF_VALID		0

/*	fmh_oflags values - returned in the header segment only. */
#define FMH_OF_DEV_T		0x1	/* fmr_device values will be dev_t */

/*	fmr_flags values - returned for each non-header segment */
#define FMR_OF_PREALLOC		0x1	/* segment = unwritten pre-allocation */
#define FMR_OF_ATTR_FORK	0x2	/* segment = attribute fork */
#define FMR_OF_EXTENT_MAP	0x4	/* segment = extent map */
#define FMR_OF_SHARED		0x8	/* segment = shared with another file */
#define FMR_OF_SPECIAL_OWNER	0x10	/* owner is a special value */
#define FMR_OF_LAST		0x20	/* segment is the last in the FS */

/* Each FS gets to define its own special owner codes. */
#define FMR_OWNER(type, code)	(((__u64)type << 32) | \
				 ((__u64)code & 0xFFFFFFFFULL))
#define FMR_OWNER_TYPE(owner)	((__u32)((__u64)owner >> 32))
#define FMR_OWNER_CODE(owner)	((__u32)(((__u64)owner & 0xFFFFFFFFULL)))
#define FMR_OWN_FREE		FMR_OWNER(0, 1) /* free space */
#define FMR_OWN_UNKNOWN		FMR_OWNER(0, 2) /* unknown owner */
#define FMR_OWN_METADATA	FMR_OWNER(0, 3) /* metadata */

#define FS_IOC_GETFSMAP		_IOWR('X', 59, struct fsmap_head)
#endif /* FS_IOC_GETFSMAP */

#endif
