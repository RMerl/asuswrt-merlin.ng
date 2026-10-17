// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2015, Kodak Alaris, Inc
 */

#include <common.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/fsp1/fsp_support.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * Override the FSP's Azalia configuration data
 *
 * @azalia:	pointer to be updated to point to a ROM address where Azalia
 *		configuration data is stored
 */
__weak void update_fsp_azalia_configs(struct azalia_config **azalia)
{
	*azalia = NULL;
}

/**
 * Override the FSP's configuration data.
 * If the device tree does not specify an integer setting, use the default
 * provided in Intel's Baytrail_FSP_Gold4.tgz release FSP/BayleyBayFsp.bsf file.
 */
void fsp_update_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf)
{
	struct upd_region *fsp_upd = &config->fsp_upd;
	struct memory_down_data *mem;
	const void *blob = gd->fdt_blob;
	int node;

	/* Initialize runtime buffer for fsp_init() */
	rt_buf->common.stack_top = config->common.stack_top - 32;
	rt_buf->common.boot_mode = config->common.boot_mode;
	rt_buf->common.upd_data = &config->fsp_upd;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_BAYTRAIL_FSP);
	if (node < 0) {
		debug("%s: Cannot find FSP node\n", __func__);
		return;
	}

	fsp_upd->mrc_init_tseg_size = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-tseg-size",
						     MRC_INIT_TSEG_SIZE_1MB);
	fsp_upd->mrc_init_mmio_size = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-mmio-size",
						     MRC_INIT_MMIO_SIZE_2048MB);
	fsp_upd->mrc_init_spd_addr1 = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-spd-addr1",
						     0xa0);
	fsp_upd->mrc_init_spd_addr2 = fdtdec_get_int(blob, node,
						     "fsp,mrc-init-spd-addr2",
						     0xa2);
	fsp_upd->emmc_boot_mode = fdtdec_get_int(blob, node,
						 "fsp,emmc-boot-mode",
						 EMMC_BOOT_MODE_EMMC41);
	fsp_upd->enable_sdio = fdtdec_get_bool(blob, node, "fsp,enable-sdio");
	fsp_upd->enable_sdcard = fdtdec_get_bool(blob, node,
						 "fsp,enable-sdcard");
	fsp_upd->enable_hsuart0 = fdtdec_get_bool(blob, node,
						  "fsp,enable-hsuart0");
	fsp_upd->enable_hsuart1 = fdtdec_get_bool(blob, node,
						  "fsp,enable-hsuart1");
	fsp_upd->enable_spi = fdtdec_get_bool(blob, node, "fsp,enable-spi");
	fsp_upd->enable_sata = fdtdec_get_bool(blob, node, "fsp,enable-sata");
	fsp_upd->sata_mode = fdtdec_get_int(blob, node, "fsp,sata-mode",
					    SATA_MODE_AHCI);
	fsp_upd->enable_azalia = fdtdec_get_bool(blob, node,
						 "fsp,enable-azalia");
	if (fsp_upd->enable_azalia)
		update_fsp_azalia_configs(&fsp_upd->azalia_cfg_ptr);
	fsp_upd->enable_xhci = fdtdec_get_bool(blob, node, "fsp,enable-xhci");
	fsp_upd->lpe_mode = fdtdec_get_int(blob, node, "fsp,lpe-mode",
					   LPE_MODE_PCI);
	fsp_upd->lpss_sio_mode = fdtdec_get_int(blob, node, "fsp,lpss-sio-mode",
					   LPSS_SIO_MODE_PCI);
	fsp_upd->enable_dma0 = fdtdec_get_bool(blob, node, "fsp,enable-dma0");
	fsp_upd->enable_dma1 = fdtdec_get_bool(blob, node, "fsp,enable-dma1");
	fsp_upd->enable_i2_c0 = fdtdec_get_bool(blob, node, "fsp,enable-i2c0");
	fsp_upd->enable_i2_c1 = fdtdec_get_bool(blob, node, "fsp,enable-i2c1");
	fsp_upd->enable_i2_c2 = fdtdec_get_bool(blob, node, "fsp,enable-i2c2");
	fsp_upd->enable_i2_c3 = fdtdec_get_bool(blob, node, "fsp,enable-i2c3");
	fsp_upd->enable_i2_c4 = fdtdec_get_bool(blob, node, "fsp,enable-i2c4");
	fsp_upd->enable_i2_c5 = fdtdec_get_bool(blob, node, "fsp,enable-i2c5");
	fsp_upd->enable_i2_c6 = fdtdec_get_bool(blob, node, "fsp,enable-i2c6");
	fsp_upd->enable_pwm0 = fdtdec_get_bool(blob, node, "fsp,enable-pwm0");
	fsp_upd->enable_pwm1 = fdtdec_get_bool(blob, node, "fsp,enable-pwm1");
	fsp_upd->enable_hsi = fdtdec_get_bool(blob, node, "fsp,enable-hsi");
	fsp_upd->igd_dvmt50_pre_alloc = fdtdec_get_int(blob, node,
			"fsp,igd-dvmt50-pre-alloc", IGD_DVMT50_PRE_ALLOC_64MB);
	fsp_upd->aperture_size = fdtdec_get_int(blob, node, "fsp,aperture-size",
						APERTURE_SIZE_256MB);
	fsp_upd->gtt_size = fdtdec_get_int(blob, node, "fsp,gtt-size",
					   GTT_SIZE_2MB);
	fsp_upd->mrc_debug_msg = fdtdec_get_bool(blob, node,
						 "fsp,mrc-debug-msg");
	fsp_upd->isp_enable = fdtdec_get_bool(blob, node, "fsp,isp-enable");
	fsp_upd->scc_mode = fdtdec_get_int(blob, node, "fsp,scc-mode",
					   SCC_MODE_PCI);
	fsp_upd->igd_render_standby = fdtdec_get_bool(blob, node,
						      "fsp,igd-render-standby");
	fsp_upd->txe_uma_enable = fdtdec_get_bool(blob, node,
						  "fsp,txe-uma-enable");
	fsp_upd->os_selection = fdtdec_get_int(blob, node, "fsp,os-selection",
					       OS_SELECTION_LINUX);
	fsp_upd->emmc45_ddr50_enabled = fdtdec_get_bool(blob, node,
			"fsp,emmc45-ddr50-enabled");
	fsp_upd->emmc45_hs200_enabled = fdtdec_get_bool(blob, node,
			"fsp,emmc45-hs200-enabled");
	fsp_upd->emmc45_retune_timer_value = fdtdec_get_int(blob, node,
			"fsp,emmc45-retune-timer-value", 8);
	fsp_upd->enable_igd = fdtdec_get_bool(blob, node, "fsp,enable-igd");

	mem = &fsp_upd->memory_params;
	mem->enable_memory_down = fdtdec_get_bool(blob, node,
						  "fsp,enable-memory-down");
	if (mem->enable_memory_down) {
		node = fdtdec_next_compatible(blob, node,
					      COMPAT_INTEL_BAYTRAIL_FSP_MDP);
		if (node < 0) {
			debug("%s: Cannot find FSP memory-down-params node\n",
			      __func__);
		} else {
			mem->dram_speed = fdtdec_get_int(blob, node,
							 "fsp,dram-speed",
							 DRAM_SPEED_1333MTS);
			mem->dram_type = fdtdec_get_int(blob, node,
							"fsp,dram-type",
							DRAM_TYPE_DDR3L);
			mem->dimm_0_enable = fdtdec_get_bool(blob, node,
					"fsp,dimm-0-enable");
			mem->dimm_1_enable = fdtdec_get_bool(blob, node,
					"fsp,dimm-1-enable");
			mem->dimm_width = fdtdec_get_int(blob, node,
							 "fsp,dimm-width",
							 DIMM_WIDTH_X8);
			mem->dimm_density = fdtdec_get_int(blob, node,
							   "fsp,dimm-density",
							   DIMM_DENSITY_2GBIT);
			mem->dimm_bus_width = fdtdec_get_int(blob, node,
					"fsp,dimm-bus-width",
					DIMM_BUS_WIDTH_64BITS);
			mem->dimm_sides = fdtdec_get_int(blob, node,
							 "fsp,dimm-sides",
							 DIMM_SIDES_1RANKS);
			mem->dimm_tcl = fdtdec_get_int(blob, node,
						       "fsp,dimm-tcl", 0x09);
			mem->dimm_trpt_rcd = fdtdec_get_int(blob, node,
					"fsp,dimm-trpt-rcd", 0x09);
			mem->dimm_twr = fdtdec_get_int(blob, node,
						       "fsp,dimm-twr", 0x0a);
			mem->dimm_twtr = fdtdec_get_int(blob, node,
							"fsp,dimm-twtr", 0x05);
			mem->dimm_trrd = fdtdec_get_int(blob, node,
							"fsp,dimm-trrd", 0x04);
			mem->dimm_trtp = fdtdec_get_int(blob, node,
							"fsp,dimm-trtp", 0x05);
			mem->dimm_tfaw = fdtdec_get_int(blob, node,
							"fsp,dimm-tfaw", 0x14);
		}
	}
}
