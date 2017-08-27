/*
 * iostat: report CPU and I/O statistics
 * (C) 1999-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _IOSTAT_H
#define _IOSTAT_H

#include "common.h"

/* I_: iostat - D_: Display - F_: Flag */
#define I_D_CPU			0x00001
#define I_D_DISK		0x00002
#define I_D_TIMESTAMP		0x00004
#define I_D_EXTENDED		0x00008
#define I_D_PART_ALL		0x00010
#define I_D_KILOBYTES		0x00020
#define I_F_HAS_SYSFS		0x00040
#define I_D_DEBUG		0x00080
#define I_D_UNFILTERED		0x00100
#define I_D_MEGABYTES		0x00200
#define I_D_PARTITIONS		0x00400
#define I_F_HAS_DISKSTATS	0x00800
#define I_D_HUMAN_READ		0x01000
/* Unused			0x02000 */
/* Unused			0x04000 */
/* Unused			0x08000 */
#define I_D_DEVMAP_NAME		0x10000
#define I_D_ISO			0x20000
/* Unused			0x40000 */
#define I_D_ZERO_OMIT		0x80000

#define DISPLAY_CPU(m)		(((m) & I_D_CPU)           == I_D_CPU)
#define DISPLAY_DISK(m)		(((m) & I_D_DISK)          == I_D_DISK)
#define DISPLAY_TIMESTAMP(m)	(((m) & I_D_TIMESTAMP)     == I_D_TIMESTAMP)
#define DISPLAY_EXTENDED(m)	(((m) & I_D_EXTENDED)      == I_D_EXTENDED)
#define DISPLAY_PART_ALL(m)	(((m) & I_D_PART_ALL)      == I_D_PART_ALL)
#define DISPLAY_KILOBYTES(m)	(((m) & I_D_KILOBYTES)     == I_D_KILOBYTES)
#define DISPLAY_MEGABYTES(m)	(((m) & I_D_MEGABYTES)     == I_D_MEGABYTES)
#define HAS_SYSFS(m)		(((m) & I_F_HAS_SYSFS)     == I_F_HAS_SYSFS)
#define DISPLAY_DEBUG(m)	(((m) & I_D_DEBUG)         == I_D_DEBUG)
#define DISPLAY_UNFILTERED(m)	(((m) & I_D_UNFILTERED)    == I_D_UNFILTERED)
#define DISPLAY_PARTITIONS(m)	(((m) & I_D_PARTITIONS)    == I_D_PARTITIONS)
#define HAS_DISKSTATS(m)	(((m) & I_F_HAS_DISKSTATS) == I_F_HAS_DISKSTATS)
#define DISPLAY_HUMAN_READ(m)	(((m) & I_D_HUMAN_READ)    == I_D_HUMAN_READ)
#define DISPLAY_DEVMAP_NAME(m)	(((m) & I_D_DEVMAP_NAME)   == I_D_DEVMAP_NAME)
#define DISPLAY_ISO(m)		(((m) & I_D_ISO)           == I_D_ISO)
#define DISPLAY_ZERO_OMIT(m)	(((m) & I_D_ZERO_OMIT)     == I_D_ZERO_OMIT)

/* Preallocation constats */
#define NR_DEV_PREALLOC		4
#define NR_DISK_PREALLOC	3

/* Environment variable */
#define ENV_POSIXLY_CORRECT	"POSIXLY_CORRECT"

/*
 * Structures for I/O stats.
 * The number of structures allocated corresponds to the number of devices
 * present in the system, plus a preallocation number to handle those
 * that can be registered dynamically.
 * The number of devices is found by using /sys filesystem (if mounted),
 * or the number of "disk_io:" entries in /proc/stat (2.4 kernels),
 * else the default value is 4 (for old kernels, which maintained stats
 * for the first four devices in /proc/stat).
 * For each io_stats structure allocated corresponds a io_hdr_stats structure.
 * A io_stats structure is considered as unused or "free" (containing no stats
 * for a particular device) if the 'major' field of the io_hdr_stats
 * structure is set to 0.
 */
struct io_stats {
	/* # of sectors read */
	unsigned long long rd_sectors	__attribute__ ((aligned (8)));
	/* # of sectors written */
	unsigned long long wr_sectors	__attribute__ ((packed));
	/* # of read operations issued to the device */
	unsigned long rd_ios		__attribute__ ((packed));
	/* # of read requests merged */
	unsigned long rd_merges		__attribute__ ((packed));
	/* Time of read requests in queue */
	unsigned long rd_ticks		__attribute__ ((packed));
	/* # of write operations issued to the device */
	unsigned long wr_ios		__attribute__ ((packed));
	/* # of write requests merged */
	unsigned long wr_merges		__attribute__ ((packed));
	/* Time of write requests in queue */
	unsigned long wr_ticks		__attribute__ ((packed));
	/* # of I/Os in progress */
	unsigned long ios_pgr		__attribute__ ((packed));
	/* # of ticks total (for this device) for I/O */
	unsigned long tot_ticks		__attribute__ ((packed));
	/* # of ticks requests spent in queue */
	unsigned long rq_ticks		__attribute__ ((packed));
	/* # of I/O done since last reboot */
	unsigned long dk_drive		__attribute__ ((packed));
	/* # of blocks read */
	unsigned long dk_drive_rblk	__attribute__ ((packed));
	/* # of blocks written */
	unsigned long dk_drive_wblk	__attribute__ ((packed));
};

#define IO_STATS_SIZE	(sizeof(struct io_stats))

struct io_hdr_stats {
	unsigned int active		__attribute__ ((aligned (4)));
	unsigned int used		__attribute__ ((packed));
	char name[MAX_NAME_LEN];
};

#define IO_HDR_STATS_SIZE	(sizeof(struct io_hdr_stats))

/* List of devices entered on the command line */
struct io_dlist {
	/* Indicate whether its partitions are to be displayed or not */
	int disp_part			__attribute__ ((aligned (4)));
	/* Device name */
	char dev_name[MAX_NAME_LEN];
};

#define IO_DLIST_SIZE	(sizeof(struct io_dlist))

#endif  /* _IOSTAT_H */
