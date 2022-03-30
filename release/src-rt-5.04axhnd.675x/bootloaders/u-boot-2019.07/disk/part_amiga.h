/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Hans-Joerg Frieden, Hyperion Entertainment
 * Hans-JoergF@hyperion-entertainment.com
 */

#ifndef _DISK_PART_AMIGA_H
#define _DISK_PART_AMIGA_H
#include <common.h>

#if CONFIG_IS_ENABLED(ISO_PARTITION)
/* Make the buffers bigger if ISO partition support is enabled -- CD-ROMS
   have 2048 byte blocks */
#define DEFAULT_SECTOR_SIZE   2048
#else
#define DEFAULT_SECTOR_SIZE	512
#endif


#define AMIGA_BLOCK_LIMIT 16

/*
 * Amiga disks have a very open structure. The head for the partition table information
 * is stored somewhere within the first 16 blocks on disk, and is called the
 * "RigidDiskBlock".
 */

struct rigid_disk_block
{
    u32 id;
    u32 summed_longs;
    s32 chk_sum;
    u32 host_id;
    u32 block_bytes;
    u32 flags;
    u32 bad_block_list;
    u32 partition_list;
    u32 file_sys_header_list;
    u32 drive_init;
    u32 bootcode_block;
    u32 reserved_1[5];

    /* Physical drive geometry */
    u32 cylinders;
    u32 sectors;
    u32 heads;
    u32 interleave;
    u32 park;
    u32 reserved_2[3];
    u32 write_pre_comp;
    u32 reduced_write;
    u32 step_rate;
    u32 reserved_3[5];

    /* logical drive geometry */
    u32 rdb_blocks_lo;
    u32 rdb_blocks_hi;
    u32 lo_cylinder;
    u32 hi_cylinder;
    u32 cyl_blocks;
    u32 auto_park_seconds;
    u32 high_rdsk_block;
    u32 reserved_4;

    char disk_vendor[8];
    char disk_product[16];
    char disk_revision[4];
    char controller_vendor[8];
    char controller_product[16];
    char controller_revision[4];

    u32 reserved_5[10];
};

/*
 * Each partition on this drive is defined by such a block
 */

struct partition_block
{
    u32 id;
    u32 summed_longs;
    s32 chk_sum;
    u32 host_id;
    u32 next;
    u32 flags;
    u32 reserved_1[2];
    u32 dev_flags;
    char drive_name[32];
    u32 reserved_2[15];
    u32 environment[17];
    u32 reserved_3[15];
};

struct bootcode_block
{
    u32   id;
    u32   summed_longs;
    s32   chk_sum;
    u32   host_id;
    u32   next;
    u32   load_data[123];
};


#define AMIGA_ID_RDISK                  0x5244534B
#define AMIGA_ID_PART                   0x50415254
#define AMIGA_ID_BOOT                   0x424f4f54

/*
 * The environment array in the partition block
 * describes the partition
 */

struct amiga_part_geometry
{
    u32 table_size;
    u32 size_blocks;
    u32 unused1;
    u32 surfaces;
    u32 sector_per_block;
    u32 block_per_track;
    u32 reserved;
    u32 prealloc;
    u32 interleave;
    u32 low_cyl;
    u32 high_cyl;
    u32 num_buffers;
    u32 buf_mem_type;
    u32 max_transfer;
    u32 mask;
    s32 boot_priority;
    u32 dos_type;
    u32 baud;
    u32 control;
    u32 boot_blocks;
};

#endif /* _DISK_PART_AMIGA_H_ */
