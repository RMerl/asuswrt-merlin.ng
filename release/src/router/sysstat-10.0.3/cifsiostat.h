/*
 * cifsiostat: Report CIFS statistics
 * Copyright (C) 2010 Red Hat, Inc. All Rights Reserved
 * Written by Ivana Varekova <varekova@redhat.com>
 */

#ifndef _CIFSIOSTAT_H
#define _CIFSIOSTAT_H

#include "common.h"

#define CIFSSTATS  "/proc/fs/cifs/Stats"

/* I_: iostat - D_: Display - F_: Flag */
#define I_D_TIMESTAMP		0x001
#define I_D_KILOBYTES		0x002
#define I_D_MEGABYTES		0x004
#define I_D_ISO			0x008
#define I_D_HUMAN_READ		0x010
#define I_D_DEBUG		0x020

#define DISPLAY_TIMESTAMP(m)	(((m) & I_D_TIMESTAMP)     == I_D_TIMESTAMP)
#define DISPLAY_KILOBYTES(m)	(((m) & I_D_KILOBYTES)     == I_D_KILOBYTES)
#define DISPLAY_MEGABYTES(m)	(((m) & I_D_MEGABYTES)     == I_D_MEGABYTES)
#define DISPLAY_ISO(m)		(((m) & I_D_ISO)           == I_D_ISO)
#define DISPLAY_HUMAN_READ(m)	(((m) & I_D_HUMAN_READ)    == I_D_HUMAN_READ)
#define DISPLAY_DEBUG(m)	(((m) & I_D_DEBUG)         == I_D_DEBUG)

/* Preallocation constats */
#define NR_CIFS_PREALLOC	2

struct cifs_stats {
	unsigned long long rd_bytes     __attribute__ ((aligned (8)));
	unsigned long long wr_bytes     __attribute__ ((packed));
	unsigned long long rd_ops       __attribute__ ((packed));
	unsigned long long wr_ops       __attribute__ ((packed));
	unsigned long long fopens       __attribute__ ((packed));
	unsigned long long fcloses      __attribute__ ((packed));
	unsigned long long fdeletes     __attribute__ ((packed));
};

#define CIFS_STATS_SIZE	(sizeof(struct cifs_stats))

struct io_hdr_stats {
	unsigned int active		__attribute__ ((aligned (4)));
	unsigned int used		__attribute__ ((packed));
	char name[MAX_NAME_LEN];
};

#define IO_HDR_STATS_SIZE	(sizeof(struct io_hdr_stats))

#endif  /* _CIFSIOSTAT_H */
