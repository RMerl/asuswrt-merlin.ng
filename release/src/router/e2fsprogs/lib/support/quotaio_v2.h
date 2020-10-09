/*
 *
 *	Header file for disk format of new quotafile format
 *
 */

#ifndef GUARD_QUOTAIO_V2_H
#define GUARD_QUOTAIO_V2_H

#include <sys/types.h>
#include "quotaio.h"

/* Offset of info header in file */
#define V2_DQINFOOFF		sizeof(struct v2_disk_dqheader)
/* Supported version of quota-tree format */
#define V2_VERSION 1

struct v2_disk_dqheader {
	__le32 dqh_magic;	/* Magic number identifying file */
	__le32 dqh_version;	/* File version */
} __attribute__ ((packed));

/* Flags for version specific files */
#define V2_DQF_MASK  0x0000	/* Mask for all valid ondisk flags */

/* Header with type and version specific information */
struct v2_disk_dqinfo {
	__le32 dqi_bgrace;	/* Time before block soft limit becomes
				 * hard limit */
	__le32 dqi_igrace;	/* Time before inode soft limit becomes
				 * hard limit */
	__le32 dqi_flags;	/* Flags for quotafile (DQF_*) */
	__le32 dqi_blocks;	/* Number of blocks in file */
	__le32 dqi_free_blk;	/* Number of first free block in the list */
	__le32 dqi_free_entry;	/* Number of block with at least one
					 * free entry */
} __attribute__ ((packed));

struct v2r1_disk_dqblk {
	__le32 dqb_id;	/* id this quota applies to */
	__le32 dqb_pad;
	__le64 dqb_ihardlimit;	/* absolute limit on allocated inodes */
	__le64 dqb_isoftlimit;	/* preferred inode limit */
	__le64 dqb_curinodes;	/* current # allocated inodes */
	__le64 dqb_bhardlimit;	/* absolute limit on disk space
					 * (in QUOTABLOCK_SIZE) */
	__le64 dqb_bsoftlimit;	/* preferred limit on disk space
					 * (in QUOTABLOCK_SIZE) */
	__le64 dqb_curspace;	/* current space occupied (in bytes) */
	__le64 dqb_btime;	/* time limit for excessive disk use */
	__le64 dqb_itime;	/* time limit for excessive inode use */
} __attribute__ ((packed));

#endif
