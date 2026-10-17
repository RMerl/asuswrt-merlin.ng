// SPDX-License-Identifier: GPL-2.0+
/*
 * Onboard memory detection for Snapdragon boards
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <part.h>
#include <smem.h>
#include <fdt_support.h>
#include <asm/arch/dram.h>

#define SMEM_USABLE_RAM_PARTITION_TABLE 402
#define RAM_PART_NAME_LENGTH            16
#define RAM_NUM_PART_ENTRIES            32
#define CATEGORY_SDRAM 0x0E
#define TYPE_SYSMEM 0x01

struct smem_ram_ptable_hdr {
	u32 magic[2];
	u32 version;
	u32 reserved;
	u32 len;
} __attribute__ ((__packed__));

struct smem_ram_ptn {
	char name[RAM_PART_NAME_LENGTH];
	u64 start;
	u64 size;
	u32 attr;
	u32 category;
	u32 domain;
	u32 type;
	u32 num_partitions;
	u32 reserved[3];
} __attribute__ ((__packed__));

struct smem_ram_ptable {
	struct smem_ram_ptable_hdr hdr;
	u32 reserved;     /* Added for 8 bytes alignment of header */
	struct smem_ram_ptn parts[RAM_NUM_PART_ENTRIES];
} __attribute__ ((__packed__));

#ifndef MEMORY_BANKS_MAX
#define MEMORY_BANKS_MAX 4
#endif

int msm_fixup_memory(void *blob)
{
	u64 bank_start[MEMORY_BANKS_MAX];
	u64 bank_size[MEMORY_BANKS_MAX];
	size_t size;
	int i;
	int count = 0;
	struct udevice *smem;
	int ret;
	struct smem_ram_ptable *ram_ptable;
	struct smem_ram_ptn *p;

	ret = uclass_get_device_by_name(UCLASS_SMEM, "smem", &smem);
	if (ret < 0) {
		printf("Failed to find SMEM node. Check device tree\n");
		return 0;
	}

	ram_ptable = smem_get(smem, -1, SMEM_USABLE_RAM_PARTITION_TABLE, &size);

	if (!ram_ptable) {
		printf("Failed to find SMEM partition.\n");
		return -ENODEV;
	}

	/* Check validy of RAM */
	for (i = 0; i < RAM_NUM_PART_ENTRIES; i++) {
		p = &ram_ptable->parts[i];
		if (p->category == CATEGORY_SDRAM && p->type == TYPE_SYSMEM) {
			bank_start[count] = p->start;
			bank_size[count] = p->size;
			debug("Detected memory bank %u: start: 0x%llx size: 0x%llx\n",
					count, p->start, p->size);
			count++;
		}
	}

	if (!count) {
		printf("Failed to detect any memory bank\n");
		return -ENODEV;
	}

	ret = fdt_fixup_memory_banks(blob, bank_start, bank_size, count);
	if (ret)
		return ret;

	return 0;
}
