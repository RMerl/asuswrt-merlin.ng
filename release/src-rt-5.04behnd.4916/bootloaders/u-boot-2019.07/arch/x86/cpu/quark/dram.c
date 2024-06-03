// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/mrccache.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <asm/arch/mrc.h>
#include <asm/arch/msg_port.h>
#include <asm/arch/quark.h>

DECLARE_GLOBAL_DATA_PTR;

static __maybe_unused int prepare_mrc_cache(struct mrc_params *mrc_params)
{
	struct mrc_data_container *cache;
	struct mrc_region entry;
	int ret;

	ret = mrccache_get_region(NULL, &entry);
	if (ret)
		return ret;

	cache = mrccache_find_current(&entry);
	if (!cache)
		return -ENOENT;

	debug("%s: mrc cache at %p, size %x checksum %04x\n", __func__,
	      cache->data, cache->data_size, cache->checksum);

	/* copy mrc cache to the mrc_params */
	memcpy(&mrc_params->timings, cache->data, cache->data_size);

	return 0;
}

static int mrc_configure_params(struct mrc_params *mrc_params)
{
	const void *blob = gd->fdt_blob;
	int node;
	int mrc_flags;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_QRK_MRC);
	if (node < 0) {
		debug("%s: Cannot find MRC node\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_ENABLE_MRC_CACHE
	mrc_params->boot_mode = prepare_mrc_cache(mrc_params);
	if (mrc_params->boot_mode)
		mrc_params->boot_mode = BM_COLD;
	else
		mrc_params->boot_mode = BM_FAST;
#else
	mrc_params->boot_mode = BM_COLD;
#endif

	/*
	 * TODO:
	 *
	 * We need determine ECC by pin strap state
	 *
	 * Disable ECC by default for now
	 */
	mrc_params->ecc_enables = 0;

	mrc_flags = fdtdec_get_int(blob, node, "flags", 0);
	if (mrc_flags & MRC_FLAG_SCRAMBLE_EN)
		mrc_params->scrambling_enables = 1;
	else
		mrc_params->scrambling_enables = 0;

	mrc_params->dram_width = fdtdec_get_int(blob, node, "dram-width", 0);
	mrc_params->ddr_speed = fdtdec_get_int(blob, node, "dram-speed", 0);
	mrc_params->ddr_type = fdtdec_get_int(blob, node, "dram-type", 0);

	mrc_params->rank_enables = fdtdec_get_int(blob, node, "rank-mask", 0);
	mrc_params->channel_enables = fdtdec_get_int(blob, node,
		"chan-mask", 0);
	mrc_params->channel_width = fdtdec_get_int(blob, node,
		"chan-width", 0);
	mrc_params->address_mode = fdtdec_get_int(blob, node, "addr-mode", 0);

	mrc_params->refresh_rate = fdtdec_get_int(blob, node,
		"refresh-rate", 0);
	mrc_params->sr_temp_range = fdtdec_get_int(blob, node,
		"sr-temp-range", 0);
	mrc_params->ron_value = fdtdec_get_int(blob, node,
		"ron-value", 0);
	mrc_params->rtt_nom_value = fdtdec_get_int(blob, node,
		"rtt-nom-value", 0);
	mrc_params->rd_odt_value = fdtdec_get_int(blob, node,
		"rd-odt-value", 0);

	mrc_params->params.density = fdtdec_get_int(blob, node,
		"dram-density", 0);
	mrc_params->params.cl = fdtdec_get_int(blob, node, "dram-cl", 0);
	mrc_params->params.ras = fdtdec_get_int(blob, node, "dram-ras", 0);
	mrc_params->params.wtr = fdtdec_get_int(blob, node, "dram-wtr", 0);
	mrc_params->params.rrd = fdtdec_get_int(blob, node, "dram-rrd", 0);
	mrc_params->params.faw = fdtdec_get_int(blob, node, "dram-faw", 0);

	debug("MRC dram_width %d\n", mrc_params->dram_width);
	debug("MRC rank_enables %d\n", mrc_params->rank_enables);
	debug("MRC ddr_speed %d\n", mrc_params->ddr_speed);
	debug("MRC flags: %s\n",
	      (mrc_params->scrambling_enables) ? "SCRAMBLE_EN" : "");

	debug("MRC density=%d tCL=%d tRAS=%d tWTR=%d tRRD=%d tFAW=%d\n",
	      mrc_params->params.density, mrc_params->params.cl,
	      mrc_params->params.ras, mrc_params->params.wtr,
	      mrc_params->params.rrd, mrc_params->params.faw);

	return 0;
}

int dram_init(void)
{
	struct mrc_params mrc_params;
#ifdef CONFIG_ENABLE_MRC_CACHE
	char *cache;
#endif
	int ret;

	memset(&mrc_params, 0, sizeof(struct mrc_params));
	ret = mrc_configure_params(&mrc_params);
	if (ret)
		return ret;

	/* Set up the DRAM by calling the memory reference code */
	mrc_init(&mrc_params);
	if (mrc_params.status)
		return -EIO;

	gd->ram_size = mrc_params.mem_size;
	post_code(POST_DRAM);

	/* variable range MTRR#2: RAM area */
	disable_caches();
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_VAR_PHYBASE(MTRR_VAR_RAM),
		       0 | MTRR_TYPE_WRBACK);
	msg_port_write(MSG_PORT_HOST_BRIDGE, MTRR_VAR_PHYMASK(MTRR_VAR_RAM),
		       (~(gd->ram_size - 1)) | MTRR_PHYS_MASK_VALID);
	enable_caches();

#ifdef CONFIG_ENABLE_MRC_CACHE
	cache = malloc(sizeof(struct mrc_timings));
	if (cache) {
		memcpy(cache, &mrc_params.timings, sizeof(struct mrc_timings));
		gd->arch.mrc_output = cache;
		gd->arch.mrc_output_len = sizeof(struct mrc_timings);
	}
#endif

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_size;
}
