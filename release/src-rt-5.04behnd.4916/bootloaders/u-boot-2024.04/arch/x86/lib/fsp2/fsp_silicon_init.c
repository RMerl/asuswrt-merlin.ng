// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2015-2016 Intel Corp.
 * (Written by Andrey Petrov <andrey.petrov@intel.com> for Intel Corp.)
 *
 * Mostly taken from coreboot fsp2_0/silicon_init.c
 */

#define LOG_CATEGORY UCLASS_NORTHBRIDGE

#include <common.h>
#include <binman.h>
#include <bootstage.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/fsp/fsp_configs.h>
#include <asm/arch/fsp/fsp_s_upd.h>
#include <asm/fsp/fsp_infoheader.h>
#include <asm/fsp2/fsp_internal.h>
#include <asm/global_data.h>

int fsp_silicon_init(bool s3wake, bool use_spi_flash)
{
	struct fsps_upd upd, *fsp_upd;
	fsp_silicon_init_func func;
	struct fsp_header *hdr;
	struct binman_entry entry;
	struct udevice *dev;
	ulong rom_offset = 0;
	u32 init_addr;
	int ret;

	log_debug("Locating FSP\n");
	ret = fsp_locate_fsp(FSP_S, &entry, use_spi_flash, &dev, &hdr,
			     &rom_offset);
	if (ret)
		return log_msg_ret("locate FSP", ret);
	binman_set_rom_offset(rom_offset);
	gd->arch.fsp_s_hdr = hdr;

	/* Copy over the default config */
	fsp_upd = (struct fsps_upd *)(hdr->img_base + hdr->cfg_region_off);
	if (fsp_upd->header.signature != FSPS_UPD_SIGNATURE)
		return log_msg_ret("Bad UPD signature", -EPERM);
	memcpy(&upd, fsp_upd, sizeof(upd));

	ret = fsps_update_config(dev, rom_offset, &upd);
	if (ret)
		return log_msg_ret("Could not setup config", ret);
	log_debug("Silicon init @ %x...", init_addr);
	bootstage_start(BOOTSTAGE_ID_ACCUM_FSP_S, "fsp-s");
	func = (fsp_silicon_init_func)(hdr->img_base + hdr->fsp_silicon_init);
	ret = func(&upd);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_FSP_S);
	if (ret)
		return log_msg_ret("Silicon init fail\n", ret);
	log_debug("done\n");

	return 0;
}
