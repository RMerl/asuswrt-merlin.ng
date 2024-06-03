// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * From coreboot src/soc/intel/broadwell/romstage/raminit.c
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/lpc_common.h>
#include <asm/mrccache.h>
#include <asm/mrc_common.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/arch/iomap.h>
#include <asm/arch/me.h>
#include <asm/arch/pch.h>
#include <asm/arch/pei_data.h>
#include <asm/arch/pm.h>

ulong board_get_usable_ram_top(ulong total_size)
{
	return mrc_common_board_get_usable_ram_top(total_size);
}

int dram_init_banksize(void)
{
	mrc_common_dram_init_banksize();

	return 0;
}

static unsigned long get_top_of_ram(struct udevice *dev)
{
	/*
	 * Base of DPR is top of usable DRAM below 4GiB. The register has
	 * 1 MiB alignment and reports the TOP of the range, the base
	 * must be calculated from the size in MiB in bits 11:4.
	 */
	u32 dpr, tom;

	dm_pci_read_config32(dev, DPR, &dpr);
	tom = dpr & ~((1 << 20) - 1);

	debug("dpt %08x tom %08x\n", dpr, tom);
	/* Subtract DMA Protected Range size if enabled */
	if (dpr & DPR_EPM)
		tom -= (dpr & DPR_SIZE_MASK) << 16;

	return (unsigned long)tom;
}

/**
 * sdram_find() - Find available memory
 *
 * This is a bit complicated since on x86 there are system memory holes all
 * over the place. We create a list of available memory blocks
 *
 * @dev:	Northbridge device
 */
static int sdram_find(struct udevice *dev)
{
	struct memory_info *info = &gd->arch.meminfo;
	ulong top_of_ram;

	top_of_ram = get_top_of_ram(dev);
	mrc_add_memory_area(info, 0, top_of_ram);

	/* Add MTRRs for memory */
	mtrr_add_request(MTRR_TYPE_WRBACK, 0, 2ULL << 30);

	return 0;
}

static int prepare_mrc_cache(struct pei_data *pei_data)
{
	struct mrc_data_container *mrc_cache;
	struct mrc_region entry;
	int ret;

	ret = mrccache_get_region(NULL, &entry);
	if (ret)
		return ret;
	mrc_cache = mrccache_find_current(&entry);
	if (!mrc_cache)
		return -ENOENT;

	pei_data->saved_data = mrc_cache->data;
	pei_data->saved_data_size = mrc_cache->data_size;
	debug("%s: at %p, size %x checksum %04x\n", __func__,
	      pei_data->saved_data, pei_data->saved_data_size,
	      mrc_cache->checksum);

	return 0;
}

int dram_init(void)
{
	struct pei_data _pei_data __aligned(8);
	struct pei_data *pei_data = &_pei_data;
	struct udevice *dev, *me_dev, *pch_dev;
	struct chipset_power_state ps;
	const void *spd_data;
	int ret, size;

	memset(pei_data, '\0', sizeof(struct pei_data));

	/* Print ME state before MRC */
	ret = syscon_get_by_driver_data(X86_SYSCON_ME, &me_dev);
	if (ret) {
		debug("Cannot get ME (err=%d)\n", ret);
		return ret;
	}
	intel_me_status(me_dev);

	/* Save ME HSIO version */
	ret = uclass_first_device_err(UCLASS_PCH, &pch_dev);
	if (ret) {
		debug("Cannot get PCH (err=%d)\n", ret);
		return ret;
	}
	power_state_get(pch_dev, &ps);

	intel_me_hsio_version(me_dev, &ps.hsio_version, &ps.hsio_checksum);

	broadwell_fill_pei_data(pei_data);
	mainboard_fill_pei_data(pei_data);

	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &dev);
	if (ret) {
		debug("Cannot get Northbridge (err=%d)\n", ret);
		return ret;
	}
	size = 256;
	ret = mrc_locate_spd(dev, size, &spd_data);
	if (ret) {
		debug("Cannot locate SPD (err=%d)\n", ret);
		return ret;
	}
	memcpy(pei_data->spd_data[0][0], spd_data, size);
	memcpy(pei_data->spd_data[1][0], spd_data, size);

	ret = prepare_mrc_cache(pei_data);
	if (ret)
		debug("prepare_mrc_cache failed: %d\n", ret);

	debug("PEI version %#x\n", pei_data->pei_version);
	ret = mrc_common_init(dev, pei_data, true);
	if (ret) {
		debug("mrc_common_init() failed(err=%d)\n", ret);
		return ret;
	}
	debug("Memory init done\n");

	ret = sdram_find(dev);
	if (ret) {
		debug("sdram_find() failed (err=%d)\n", ret);
		return ret;
	}
	gd->ram_size = gd->arch.meminfo.total_32bit_memory;
	debug("RAM size %llx\n", (unsigned long long)gd->ram_size);

	debug("MRC output data length %#x at %p\n", pei_data->data_to_save_size,
	      pei_data->data_to_save);
	/* S3 resume: don't save scrambler seed or MRC data */
	if (pei_data->boot_mode != SLEEP_STATE_S3) {
		/*
		 * This will be copied to SDRAM in reserve_arch(), then written
		 * to SPI flash in mrccache_save()
		 */
		gd->arch.mrc_output = (char *)pei_data->data_to_save;
		gd->arch.mrc_output_len = pei_data->data_to_save_size;
	}
	gd->arch.pei_meminfo = pei_data->meminfo;

	return 0;
}

/* Use this hook to save our SDRAM parameters */
int misc_init_r(void)
{
	int ret;

	ret = mrccache_save();
	if (ret)
		printf("Unable to save MRC data: %d\n", ret);
	else
		debug("Saved MRC cache data\n");

	return 0;
}

static const struct udevice_id broadwell_syscon_ids[] = {
	{ .compatible = "intel,me", .data = X86_SYSCON_ME },
	{ }
};

U_BOOT_DRIVER(syscon_intel_me) = {
	.name = "intel_me_syscon",
	.id = UCLASS_SYSCON,
	.of_match = broadwell_syscon_ids,
};
