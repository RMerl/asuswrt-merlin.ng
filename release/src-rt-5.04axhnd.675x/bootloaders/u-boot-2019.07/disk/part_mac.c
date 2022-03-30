// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
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
#include <command.h>
#include <memalign.h>
#include <ide.h>
#include "part_mac.h"

#ifdef CONFIG_HAVE_BLOCK_DEVICE

/* stdlib.h causes some compatibility problems; should fixe these! -- wd */
#ifndef __ldiv_t_defined
typedef struct {
	long int quot;		/* Quotient	*/
	long int rem;		/* Remainder	*/
} ldiv_t;
extern ldiv_t ldiv (long int __numer, long int __denom);
# define __ldiv_t_defined	1
#endif


static int part_mac_read_ddb(struct blk_desc *dev_desc,
			     mac_driver_desc_t *ddb_p);
static int part_mac_read_pdb(struct blk_desc *dev_desc, int part,
			     mac_partition_t *pdb_p);

/*
 * Test for a valid MAC partition
 */
static int part_test_mac(struct blk_desc *dev_desc)
{
	ALLOC_CACHE_ALIGN_BUFFER(mac_driver_desc_t, ddesc, 1);
	ALLOC_CACHE_ALIGN_BUFFER(mac_partition_t, mpart, 1);
	ulong i, n;

	if (part_mac_read_ddb (dev_desc, ddesc)) {
		/*
		 * error reading Driver Descriptor Block,
		 * or no valid Signature
		 */
		return (-1);
	}

	n = 1;	/* assuming at least one partition */
	for (i=1; i<=n; ++i) {
		if ((blk_dread(dev_desc, i, 1, (ulong *)mpart) != 1) ||
		    (mpart->signature != MAC_PARTITION_MAGIC) ) {
			return (-1);
		}
		/* update partition count */
		n = mpart->map_count;
	}
	return (0);
}

static void part_print_mac(struct blk_desc *dev_desc)
{
	ulong i, n;
	ALLOC_CACHE_ALIGN_BUFFER(mac_driver_desc_t, ddesc, 1);
	ALLOC_CACHE_ALIGN_BUFFER(mac_partition_t, mpart, 1);
	ldiv_t mb, gb;

	if (part_mac_read_ddb (dev_desc, ddesc)) {
		/*
		 * error reading Driver Descriptor Block,
		 * or no valid Signature
		 */
		return;
	}

	n  = ddesc->blk_count;

	mb = ldiv(n, ((1024 * 1024) / ddesc->blk_size)); /* MB */
	/* round to 1 digit */
	mb.rem *= 10 * ddesc->blk_size;
	mb.rem += 512 * 1024;
	mb.rem /= 1024 * 1024;

	gb = ldiv(10 * mb.quot + mb.rem, 10240);
	gb.rem += 512;
	gb.rem /= 1024;


	printf ("Block Size=%d, Number of Blocks=%d, "
		"Total Capacity: %ld.%ld MB = %ld.%ld GB\n"
		"DeviceType=0x%x, DeviceId=0x%x\n\n"
		"   #:                 type name"
		"                   length   base       (size)\n",
		ddesc->blk_size,
		ddesc->blk_count,
		mb.quot, mb.rem, gb.quot, gb.rem,
		ddesc->dev_type, ddesc->dev_id
		);

	n = 1;	/* assuming at least one partition */
	for (i=1; i<=n; ++i) {
		ulong bytes;
		char c;

		printf ("%4ld: ", i);
		if (blk_dread(dev_desc, i, 1, (ulong *)mpart) != 1) {
			printf ("** Can't read Partition Map on %d:%ld **\n",
				dev_desc->devnum, i);
			return;
		}

		if (mpart->signature != MAC_PARTITION_MAGIC) {
			printf("** Bad Signature on %d:%ld - expected 0x%04x, got 0x%04x\n",
			       dev_desc->devnum, i, MAC_PARTITION_MAGIC,
			       mpart->signature);
			return;
		}

		/* update partition count */
		n = mpart->map_count;

		c      = 'k';
		bytes  = mpart->block_count;
		bytes /= (1024 / ddesc->blk_size);  /* kB; assumes blk_size == 512 */
		if (bytes >= 1024) {
			bytes >>= 10;
			c = 'M';
		}
		if (bytes >= 1024) {
			bytes >>= 10;
			c = 'G';
		}

		printf ("%20.32s %-18.32s %10u @ %-10u (%3ld%c)\n",
			mpart->type,
			mpart->name,
			mpart->block_count,
			mpart->start_block,
			bytes, c
			);
	}

	return;
}


/*
 * Read Device Descriptor Block
 */
static int part_mac_read_ddb(struct blk_desc *dev_desc,
			     mac_driver_desc_t *ddb_p)
{
	if (blk_dread(dev_desc, 0, 1, (ulong *)ddb_p) != 1) {
		debug("** Can't read Driver Descriptor Block **\n");
		return (-1);
	}

	if (ddb_p->signature != MAC_DRIVER_MAGIC) {
		return (-1);
	}
	return (0);
}

/*
 * Read Partition Descriptor Block
 */
static int part_mac_read_pdb(struct blk_desc *dev_desc, int part,
			     mac_partition_t *pdb_p)
{
	int n = 1;

	for (;;) {
		/*
		 * We must always read the descritpor block for
		 * partition 1 first since this is the only way to
		 * know how many partitions we have.
		 */
		if (blk_dread(dev_desc, n, 1, (ulong *)pdb_p) != 1) {
			printf ("** Can't read Partition Map on %d:%d **\n",
				dev_desc->devnum, n);
			return (-1);
		}

		if (pdb_p->signature != MAC_PARTITION_MAGIC) {
			printf("** Bad Signature on %d:%d: expected 0x%04x, got 0x%04x\n",
			       dev_desc->devnum, n, MAC_PARTITION_MAGIC,
			       pdb_p->signature);
			return (-1);
		}

		if (n == part)
			return (0);

		if ((part < 1) || (part > pdb_p->map_count)) {
			printf ("** Invalid partition %d:%d [%d:1...%d:%d only]\n",
				dev_desc->devnum, part,
				dev_desc->devnum,
				dev_desc->devnum, pdb_p->map_count);
			return (-1);
		}

		/* update partition count */
		n = part;
	}

	/* NOTREACHED */
}

static int part_get_info_mac(struct blk_desc *dev_desc, int part,
				  disk_partition_t *info)
{
	ALLOC_CACHE_ALIGN_BUFFER(mac_driver_desc_t, ddesc, 1);
	ALLOC_CACHE_ALIGN_BUFFER(mac_partition_t, mpart, 1);

	if (part_mac_read_ddb (dev_desc, ddesc)) {
		return (-1);
	}

	info->blksz = ddesc->blk_size;

	if (part_mac_read_pdb (dev_desc, part, mpart)) {
		return (-1);
	}

	info->start = mpart->start_block;
	info->size  = mpart->block_count;
	memcpy (info->type, mpart->type, sizeof(info->type));
	memcpy (info->name, mpart->name, sizeof(info->name));

	return (0);
}

U_BOOT_PART_TYPE(mac) = {
	.name		= "MAC",
	.part_type	= PART_TYPE_MAC,
	.max_entries	= MAC_ENTRY_NUMBERS,
	.get_info	= part_get_info_mac,
	.print		= part_print_mac,
	.test		= part_test_mac,
};
#endif
