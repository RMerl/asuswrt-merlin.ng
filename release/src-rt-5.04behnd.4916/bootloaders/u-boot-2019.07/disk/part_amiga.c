// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Hans-Joerg Frieden, Hyperion Entertainment
 * Hans-JoergF@hyperion-entertainment.com
 */
#include <common.h>
#include <command.h>
#include <ide.h>
#include "part_amiga.h"

#ifdef CONFIG_HAVE_BLOCK_DEVICE

#undef AMIGA_DEBUG

#ifdef AMIGA_DEBUG
#define PRINTF(fmt, args...) printf(fmt ,##args)
#else
#define PRINTF(fmt, args...)
#endif

struct block_header
{
    u32 id;
    u32 summed_longs;
    s32 chk_sum;
};

static unsigned char block_buffer[DEFAULT_SECTOR_SIZE];
static struct rigid_disk_block rdb = {0};
static struct bootcode_block bootcode = {0};

/*
 * Copy a bcpl to a c string
 */
static void bcpl_strcpy(char *to, char *from)
{
    int len = *from++;

    while (len)
    {
	*to++ = *from++;
	len--;
    }
    *to = 0;
}

/*
 * Print a BCPL String. BCPL strings start with a byte with the length
 * of the string, and don't contain a terminating nul character
 */
static void bstr_print(char *string)
{
    int len = *string++;
    char buffer[256];
    int i;

    i = 0;
    while (len)
    {
	buffer[i++] = *string++;
	len--;
    }

    buffer[i] = 0;
    printf("%-10s", buffer);
}

/*
 * Sum a block. The checksum of a block must end up at zero
 * to be valid. The chk_sum field is selected so that adding
 * it yields zero.
 */
int sum_block(struct block_header *header)
{
    s32 *block = (s32 *)header;
    u32 i;
    s32 sum = 0;

    for (i = 0; i < header->summed_longs; i++)
	sum += *block++;

    return (sum != 0);
}

/*
 * Print an AmigaOS disk type. Disk types are a four-byte identifier
 * describing the file system. They are usually written as a three-letter
 * word followed by a backslash and a version number. For example,
 * DOS\0 would be the original file system. SFS\0 would be SmartFileSystem.
 * DOS\1 is FFS.
 */
static void print_disk_type(u32 disk_type)
{
    char buffer[6];
    buffer[0] = (disk_type & 0xFF000000)>>24;
    buffer[1] = (disk_type & 0x00FF0000)>>16;
    buffer[2] = (disk_type & 0x0000FF00)>>8;
    buffer[3] = '\\';
    buffer[4] = (disk_type & 0x000000FF) + '0';
    buffer[5] = 0;
    printf("%s", buffer);
}

/*
 * Print the info contained within the given partition block
 */
static void print_part_info(struct partition_block *p)
{
    struct amiga_part_geometry *g;

    g = (struct amiga_part_geometry *)&(p->environment);

    bstr_print(p->drive_name);
    printf("%6d\t%6d\t",
	   g->low_cyl * g->block_per_track * g->surfaces ,
	   (g->high_cyl - g->low_cyl + 1) * g->block_per_track * g->surfaces - 1);
    print_disk_type(g->dos_type);
    printf("\t%5d\n", g->boot_priority);
}

/*
 * Search for the Rigid Disk Block. The rigid disk block is required
 * to be within the first 16 blocks of a drive, needs to have
 * the ID AMIGA_ID_RDISK ('RDSK') and needs to have a valid
 * sum-to-zero checksum
 */
struct rigid_disk_block *get_rdisk(struct blk_desc *dev_desc)
{
    int i;
    int limit;
    char *s;

    s = env_get("amiga_scanlimit");
    if (s)
	limit = simple_strtoul(s, NULL, 10);
    else
	limit = AMIGA_BLOCK_LIMIT;

    for (i=0; i<limit; i++)
    {
	ulong res = blk_dread(dev_desc, i, 1, (ulong *)block_buffer);
	if (res == 1)
	{
	    struct rigid_disk_block *trdb = (struct rigid_disk_block *)block_buffer;
	    if (trdb->id == AMIGA_ID_RDISK)
	    {
		PRINTF("Rigid disk block suspect at %d, checking checksum\n",i);
		if (sum_block((struct block_header *)block_buffer) == 0)
		{
		    PRINTF("FOUND\n");
		    memcpy(&rdb, trdb, sizeof(struct rigid_disk_block));
		    return (struct rigid_disk_block *)&rdb;
		}
	    }
	}
    }
    PRINTF("Done scanning, no RDB found\n");
    return NULL;
}

/*
 * Search for boot code
 * Again, the first boot block must be located somewhere in the first 16 blocks, or rooted in the
 * Ridgid disk block
 */

struct bootcode_block *get_bootcode(struct blk_desc *dev_desc)
{
    int i;
    int limit;
    char *s;

    s = env_get("amiga_scanlimit");
    if (s)
	limit = simple_strtoul(s, NULL, 10);
    else
	limit = AMIGA_BLOCK_LIMIT;

    PRINTF("Scanning for BOOT from 0 to %d\n", limit);

    for (i = 0; i < limit; i++)
    {
	ulong res = blk_dread(dev_desc, i, 1, (ulong *)block_buffer);
	if (res == 1)
	{
	    struct bootcode_block *boot = (struct bootcode_block *)block_buffer;
	    if (boot->id == AMIGA_ID_BOOT)
	    {
		PRINTF("BOOT block at %d, checking checksum\n", i);
		if (sum_block((struct block_header *)block_buffer) == 0)
		{
		    PRINTF("Found valid bootcode block\n");
		    memcpy(&bootcode, boot, sizeof(struct bootcode_block));
		    return &bootcode;
		}
	    }
	}
    }

    PRINTF("No boot code found on disk\n");
    return 0;
}

/*
 * Test if the given partition has an Amiga partition table/Rigid
 * Disk block
 */
static int part_test_amiga(struct blk_desc *dev_desc)
{
    struct rigid_disk_block *rdb;
    struct bootcode_block *bootcode;

    PRINTF("part_test_amiga: Testing for an Amiga RDB partition\n");

    rdb = get_rdisk(dev_desc);
    if (rdb)
    {
	bootcode = get_bootcode(dev_desc);
	if (bootcode)
	    PRINTF("part_test_amiga: bootable Amiga disk\n");
	else
	    PRINTF("part_test_amiga: non-bootable Amiga disk\n");

	return 0;
    }
    else
    {
	PRINTF("part_test_amiga: no RDB found\n");
	return -1;
    }

}

/*
 * Find partition number partnum on the given drive.
 */
static struct partition_block *find_partition(struct blk_desc *dev_desc,
					      int partnum)
{
    struct rigid_disk_block *rdb;
    struct partition_block *p;
    u32 block;

    PRINTF("Trying to find partition block %d\n", partnum);
    rdb = get_rdisk(dev_desc);
    if (!rdb)
    {
	PRINTF("find_partition: no rdb found\n");
	return NULL;
    }

    PRINTF("find_partition: Scanning partition list\n");

    block = rdb->partition_list;
    PRINTF("find_partition: partition list at 0x%x\n", block);

    while (block != 0xFFFFFFFF)
    {
	ulong res = blk_dread(dev_desc, block, 1, (ulong *)block_buffer);
	if (res == 1)
	{
	    p = (struct partition_block *)block_buffer;
	    if (p->id == AMIGA_ID_PART)
	    {
		PRINTF("PART block suspect at 0x%x, checking checksum\n",block);
		if (sum_block((struct block_header *)p) == 0)
		{
		    if (partnum == 0) break;
		    else
		    {
			partnum--;
			block = p->next;
		    }
		}
	    } else block = 0xFFFFFFFF;
	} else block = 0xFFFFFFFF;
    }

    if (block == 0xFFFFFFFF)
    {
	PRINTF("PART block not found\n");
	return NULL;
    }

    return (struct partition_block *)block_buffer;
}

/*
 * Get info about a partition
 */
static int part_get_info_amiga(struct blk_desc *dev_desc, int part,
				    disk_partition_t *info)
{
    struct partition_block *p = find_partition(dev_desc, part-1);
    struct amiga_part_geometry *g;
    u32 disk_type;

    if (!p) return -1;

    g = (struct amiga_part_geometry *)&(p->environment);
    info->start = g->low_cyl  * g->block_per_track * g->surfaces;
    info->size  = (g->high_cyl - g->low_cyl + 1) * g->block_per_track * g->surfaces - 1;
    info->blksz = rdb.block_bytes;
    bcpl_strcpy((char *)info->name, p->drive_name);


    disk_type = g->dos_type;

    info->type[0] = (disk_type & 0xFF000000)>>24;
    info->type[1] = (disk_type & 0x00FF0000)>>16;
    info->type[2] = (disk_type & 0x0000FF00)>>8;
    info->type[3] = '\\';
    info->type[4] = (disk_type & 0x000000FF) + '0';
    info->type[5] = 0;

    return 0;
}

static void part_print_amiga(struct blk_desc *dev_desc)
{
    struct rigid_disk_block *rdb;
    struct bootcode_block *boot;
    struct partition_block *p;
    u32 block;
    int i = 1;

    rdb = get_rdisk(dev_desc);
    if (!rdb)
    {
	PRINTF("part_print_amiga: no rdb found\n");
	return;
    }

    PRINTF("part_print_amiga: Scanning partition list\n");

    block = rdb->partition_list;
    PRINTF("part_print_amiga: partition list at 0x%x\n", block);

    printf("Summary:  DiskBlockSize: %d\n"
	   "          Cylinders    : %d\n"
	   "          Sectors/Track: %d\n"
	   "          Heads        : %d\n\n",
	   rdb->block_bytes, rdb->cylinders, rdb->sectors,
	   rdb->heads);

    printf("                 First   Num. \n"
	   "Nr.  Part. Name  Block   Block  Type        Boot Priority\n");

    while (block != 0xFFFFFFFF)
    {
	ulong res;

	PRINTF("Trying to load block #0x%X\n", block);

	res = blk_dread(dev_desc, block, 1, (ulong *)block_buffer);
	if (res == 1)
	{
	    p = (struct partition_block *)block_buffer;
	    if (p->id == AMIGA_ID_PART)
	    {
		PRINTF("PART block suspect at 0x%x, checking checksum\n",block);
		if (sum_block((struct block_header *)p) == 0)
		{
		    printf("%-4d ", i); i++;
		    print_part_info(p);
		    block = p->next;
		}
	    } else block = 0xFFFFFFFF;
	} else block = 0xFFFFFFFF;
    }

    boot = get_bootcode(dev_desc);
    if (boot)
    {
	printf("Disk is bootable\n");
    }
}

U_BOOT_PART_TYPE(amiga) = {
	.name		= "AMIGA",
	.part_type	= PART_TYPE_AMIGA,
	.max_entries	= AMIGA_ENTRY_NUMBERS,
	.get_info	= part_get_info_amiga,
	.print		= part_print_amiga,
	.test		= part_test_amiga,
};

#endif
