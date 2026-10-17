// SPDX-License-Identifier: GPL-2.0+
/*
 * Functional tests for UCLASS_FFA  class
 *
 * Copyright 2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <blk.h>
#include <console.h>
#include <dm.h>
#include <mapmem.h>
#include <dm/test.h>
#include <linux/bitops.h>
#include <test/test.h>
#include <test/ut.h>
#include <nvmxip.h>

/* NVMXIP devices described in the device tree */
#define SANDBOX_NVMXIP_DEVICES 2

/* reference device tree data for the probed devices */
static struct nvmxip_plat nvmqspi_refdata[SANDBOX_NVMXIP_DEVICES] = {
	{0x08000000, 9, 4096}, {0x08200000, 9, 2048}
};

#define NVMXIP_BLK_START_PATTERN 0x1122334455667788ULL
#define NVMXIP_BLK_END_PATTERN 0xa1a2a3a4a5a6a7a8ULL

/**
 * dm_nvmxip_flash_sanity() - check flash data
 * @uts: test state
 * @device_idx:	the NVMXIP device index
 * @buffer:	the user buffer where the blocks data is copied to
 *
 * Mode 1: When buffer is NULL, initialize the flash with pattern data at the start
 * and at the end of each block. This pattern data will be used to check data consistency
 * when verifying the data read.
 * Mode 2: When the user buffer is provided in the argument (not NULL), compare the data
 * of the start and the end of each block in the user buffer with the expected pattern data.
 * Return an error when the check fails.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int dm_nvmxip_flash_sanity(struct unit_test_state *uts, u8 device_idx, void *buffer)
{
	int i;
	u64 *ptr;
	u8 *base;
	unsigned long blksz;

	blksz = BIT(nvmqspi_refdata[device_idx].lba_shift);

	if (!buffer) {
		/* Mode 1: point at the flash start address. Pattern data will be written */
		base = map_sysmem(nvmqspi_refdata[device_idx].phys_base, 0);
	} else {
		/* Mode 2: point at the user buffer containing the data read and to be verified */
		base = buffer;
	}

	for (i = 0; i < nvmqspi_refdata[device_idx].lba ; i++) {
		ptr = (u64 *)(base + i * blksz);

		/* write an 8 bytes pattern at the start of the current block */
		if (!buffer)
			*ptr = NVMXIP_BLK_START_PATTERN;
		else
			ut_asserteq_64(NVMXIP_BLK_START_PATTERN, *ptr);

		ptr = (u64 *)((u8 *)ptr + blksz - sizeof(u64));

		/* write an 8 bytes pattern at the end of the current block */
		if (!buffer)
			*ptr = NVMXIP_BLK_END_PATTERN;
		else
			ut_asserteq_64(NVMXIP_BLK_END_PATTERN, *ptr);
	}

	if (!buffer)
		unmap_sysmem(base);

	return 0;
}

/**
 * dm_test_nvmxip() - check flash data
 * @uts: test state
 * Return:
 *
 * CMD_RET_SUCCESS on success. Otherwise, failure
 */
static int dm_test_nvmxip(struct unit_test_state *uts)
{
	struct nvmxip_plat *plat_data = NULL;
	struct udevice *dev = NULL, *bdev = NULL;
	u8 device_idx;
	void *buffer = NULL;
	unsigned long flashsz;

	sandbox_set_enable_memio(true);

	/* set the flash content first for both devices */
	dm_nvmxip_flash_sanity(uts, 0, NULL);
	dm_nvmxip_flash_sanity(uts, 1, NULL);

	/* probing all NVM XIP QSPI devices */
	for (device_idx = 0, uclass_first_device(UCLASS_NVMXIP, &dev);
	     dev;
	     uclass_next_device(&dev), device_idx++) {
		plat_data = dev_get_plat(dev);

		/* device tree entries checks */
		ut_assertok(nvmqspi_refdata[device_idx].phys_base != plat_data->phys_base);
		ut_assertok(nvmqspi_refdata[device_idx].lba_shift != plat_data->lba_shift);
		ut_assertok(nvmqspi_refdata[device_idx].lba != plat_data->lba);

		/* before reading all the flash blocks, let's calculate the flash size */
		flashsz = plat_data->lba << plat_data->lba_shift;

		/* allocate the user buffer where to copy the blocks data to */
		buffer = calloc(flashsz, 1);
		ut_assertok(!buffer);

		/* the block device is the child of the parent device probed with DT */
		ut_assertok(device_find_first_child(dev, &bdev));

		/* reading all the flash blocks */
		ut_asserteq(plat_data->lba, blk_read(bdev, 0, plat_data->lba, buffer));

		/* compare the data read from flash with the expected data */
		dm_nvmxip_flash_sanity(uts, device_idx, buffer);

		free(buffer);
	}

	ut_assertok(device_idx != SANDBOX_NVMXIP_DEVICES);

	return CMD_RET_SUCCESS;
}

DM_TEST(dm_test_nvmxip, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
