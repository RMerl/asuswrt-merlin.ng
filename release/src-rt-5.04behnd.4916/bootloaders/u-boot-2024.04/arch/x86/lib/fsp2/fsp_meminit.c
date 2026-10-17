// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2015-2016 Intel Corp.
 * (Written by Andrey Petrov <andrey.petrov@intel.com> for Intel Corp.)
 * (Written by Alexandru Gagniuc <alexandrux.gagniuc@intel.com> for Intel Corp.)
 * Mostly taken from coreboot fsp2_0/memory_init.c
 */

#include <common.h>
#include <binman.h>
#include <bootstage.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/mrccache.h>
#include <asm/fsp/fsp_infoheader.h>
#include <asm/fsp2/fsp_api.h>
#include <asm/fsp2/fsp_internal.h>
#include <asm/arch/fsp/fsp_configs.h>
#include <asm/arch/fsp/fsp_m_upd.h>

static int prepare_mrc_cache_type(enum mrc_type_t type,
				  struct mrc_data_container **cachep)
{
	struct mrc_data_container *cache;
	struct mrc_region entry;
	int ret;

	ret = mrccache_get_region(type, NULL, &entry);
	if (ret)
		return ret;
	cache = mrccache_find_current(&entry);
	if (!cache)
		return -ENOENT;

	log_debug("MRC at %x, size %x\n", (uint)cache->data, cache->data_size);
	*cachep = cache;

	return 0;
}

int prepare_mrc_cache(struct fspm_upd *upd)
{
	struct mrc_data_container *cache;
	int ret;

	ret = prepare_mrc_cache_type(MRC_TYPE_NORMAL, &cache);
	if (ret)
		return log_msg_ret("Cannot get normal cache", ret);
	upd->arch.nvs_buffer_ptr = cache->data;

	ret = prepare_mrc_cache_type(MRC_TYPE_VAR, &cache);
	if (ret)
		return log_msg_ret("Cannot get var cache", ret);
	upd->config.variable_nvs_buffer_ptr = cache->data;

	return 0;
}

int fsp_memory_init(bool s3wake, bool use_spi_flash)
{
	struct fspm_upd upd, *fsp_upd;
	fsp_memory_init_func func;
	struct binman_entry entry;
	struct fsp_header *hdr;
	struct hob_header *hob;
	struct udevice *dev;
	int delay;
	int ret;

	log_debug("Locating FSP\n");
	ret = fsp_locate_fsp(FSP_M, &entry, use_spi_flash, &dev, &hdr, NULL);
	if (ret)
		return log_msg_ret("locate FSP", ret);
	debug("Found FSP_M at %x, size %x\n", hdr->img_base, hdr->img_size);

	/* Copy over the default config */
	fsp_upd = (struct fspm_upd *)(hdr->img_base + hdr->cfg_region_off);
	if (fsp_upd->header.signature != FSPM_UPD_SIGNATURE)
		return log_msg_ret("Bad UPD signature", -EPERM);
	memcpy(&upd, fsp_upd, sizeof(upd));

	delay = dev_read_u32_default(dev, "fspm,training-delay", 0);
	ret = fspm_update_config(dev, &upd);
	if (ret) {
		if (ret != -ENOENT)
			return log_msg_ret("Could not setup config", ret);
	} else {
		delay = 0;
	}

	if (delay)
		printf("SDRAM training (%d seconds)...", delay);
	else
		log_debug("SDRAM init...");
	bootstage_start(BOOTSTAGE_ID_ACCUM_FSP_M, "fsp-m");
	func = (fsp_memory_init_func)(hdr->img_base + hdr->fsp_mem_init);
	ret = func(&upd, &hob);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_FSP_M);
	cpu_reinit_fpu();
	if (delay)
		printf("done\n");
	else
		log_debug("done\n");
	if (ret)
		return log_msg_ret("SDRAM init fail\n", ret);

	gd->arch.hob_list = hob;

	ret = fspm_done(dev);
	if (ret)
		return log_msg_ret("fsm_done\n", ret);

	return 0;
}
