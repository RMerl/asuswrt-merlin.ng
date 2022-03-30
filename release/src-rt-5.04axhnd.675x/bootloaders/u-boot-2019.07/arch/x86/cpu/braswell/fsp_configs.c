// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/fsp/fsp_support.h>

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
 * Override the FSP's GPIO configuration data
 *
 * @family:	pointer to be updated to point to a ROM address where GPIO
 *		family configuration data is stored
 * @pad:	pointer to be updated to point to a ROM address where GPIO
 *		pad configuration data is stored
 */
__weak void update_fsp_gpio_configs(struct gpio_family **family,
				    struct gpio_pad **pad)
{
	*family = NULL;
	*pad = NULL;
}

/**
 * Override the FSP's configuration data.
 * If the device tree does not specify an integer setting, use the default
 * provided in Intel's Braswell release FSP/BraswellFsp.bsf file.
 */
void update_fsp_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf)
{
	struct upd_region *fsp_upd = &config->fsp_upd;
	struct memory_upd *memory_upd = &fsp_upd->memory_upd;
	struct silicon_upd *silicon_upd = &fsp_upd->silicon_upd;
	const void *blob = gd->fdt_blob;
	int node;

	/* Initialize runtime buffer for fsp_init() */
	rt_buf->common.stack_top = config->common.stack_top - 32;
	rt_buf->common.boot_mode = config->common.boot_mode;
	rt_buf->common.upd_data = &config->fsp_upd;

	node = fdt_node_offset_by_compatible(blob, 0, "intel,braswell-fsp");
	if (node < 0) {
		debug("%s: Cannot find FSP node\n", __func__);
		return;
	}

	node = fdt_node_offset_by_compatible(blob, node,
					     "intel,braswell-fsp-memory");
	if (node < 0) {
		debug("%s: Cannot find FSP memory node\n", __func__);
		return;
	}

	/* Override memory UPD contents */
	memory_upd->mrc_init_tseg_size = fdtdec_get_int(blob, node,
		"fsp,mrc-init-tseg-size", MRC_INIT_TSEG_SIZE_4MB);
	memory_upd->mrc_init_mmio_size = fdtdec_get_int(blob, node,
		"fsp,mrc-init-mmio-size", MRC_INIT_MMIO_SIZE_2048MB);
	memory_upd->mrc_init_spd_addr1 = fdtdec_get_int(blob, node,
		"fsp,mrc-init-spd-addr1", 0xa0);
	memory_upd->mrc_init_spd_addr2 = fdtdec_get_int(blob, node,
		"fsp,mrc-init-spd-addr2", 0xa2);
	memory_upd->igd_dvmt50_pre_alloc = fdtdec_get_int(blob, node,
		"fsp,igd-dvmt50-pre-alloc", IGD_DVMT50_PRE_ALLOC_32MB);
	memory_upd->aperture_size = fdtdec_get_int(blob, node,
		"fsp,aperture-size", APERTURE_SIZE_256MB);
	memory_upd->gtt_size = fdtdec_get_int(blob, node,
		"fsp,gtt-size", GTT_SIZE_1MB);
	memory_upd->legacy_seg_decode = fdtdec_get_bool(blob, node,
		"fsp,legacy-seg-decode");
	memory_upd->enable_dvfs = fdtdec_get_bool(blob, node,
		"fsp,enable-dvfs");
	memory_upd->memory_type = fdtdec_get_int(blob, node,
		"fsp,memory-type", DRAM_TYPE_DDR3);
	memory_upd->enable_ca_mirror = fdtdec_get_bool(blob, node,
		"fsp,enable-ca-mirror");

	node = fdt_node_offset_by_compatible(blob, node,
					     "intel,braswell-fsp-silicon");
	if (node < 0) {
		debug("%s: Cannot find FSP silicon node\n", __func__);
		return;
	}

	/* Override silicon UPD contents */
	silicon_upd->sdcard_mode = fdtdec_get_int(blob, node,
		"fsp,sdcard-mode", SDCARD_MODE_PCI);
	silicon_upd->enable_hsuart0 = fdtdec_get_bool(blob, node,
		"fsp,enable-hsuart0");
	silicon_upd->enable_hsuart1 = fdtdec_get_bool(blob, node,
		"fsp,enable-hsuart1");
	silicon_upd->enable_azalia = fdtdec_get_bool(blob, node,
		"fsp,enable-azalia");
	if (silicon_upd->enable_azalia)
		update_fsp_azalia_configs(&silicon_upd->azalia_cfg_ptr);
	silicon_upd->enable_sata = fdtdec_get_bool(blob, node,
		"fsp,enable-sata");
	silicon_upd->enable_xhci = fdtdec_get_bool(blob, node,
		"fsp,enable-xhci");
	silicon_upd->lpe_mode = fdtdec_get_int(blob, node,
		"fsp,lpe-mode", LPE_MODE_PCI);
	silicon_upd->enable_dma0 = fdtdec_get_bool(blob, node,
		"fsp,enable-dma0");
	silicon_upd->enable_dma1 = fdtdec_get_bool(blob, node,
		"fsp,enable-dma1");
	silicon_upd->enable_i2c0 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c0");
	silicon_upd->enable_i2c1 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c1");
	silicon_upd->enable_i2c2 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c2");
	silicon_upd->enable_i2c3 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c3");
	silicon_upd->enable_i2c4 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c4");
	silicon_upd->enable_i2c5 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c5");
	silicon_upd->enable_i2c6 = fdtdec_get_bool(blob, node,
		"fsp,enable-i2c6");
#ifdef CONFIG_HAVE_VBT
	silicon_upd->graphics_config_ptr = CONFIG_VBT_ADDR;
#endif
	update_fsp_gpio_configs(&silicon_upd->gpio_familiy_ptr,
				&silicon_upd->gpio_pad_ptr);
	/*
	 * For Braswell B0 stepping, disable_punit_pwr_config must be set to 1
	 * otherwise it just hangs in fsp_init().
	 */
	if (gd->arch.x86_mask == 2)
		silicon_upd->disable_punit_pwr_config = 1;
	silicon_upd->emmc_mode = fdtdec_get_int(blob, node,
		"fsp,emmc-mode", EMMC_MODE_PCI);
	silicon_upd->sata_speed = fdtdec_get_int(blob, node,
		"fsp,sata-speed", SATA_SPEED_GEN3);
	silicon_upd->pmic_i2c_bus = fdtdec_get_int(blob, node,
		"fsp,pmic-i2c-bus", 0);
	silicon_upd->enable_isp = fdtdec_get_bool(blob, node,
		"fsp,enable-isp");
	silicon_upd->isp_pci_dev_config = fdtdec_get_int(blob, node,
		"fsp,isp-pci-dev-config", ISP_PCI_DEV_CONFIG_2);
	silicon_upd->turbo_mode = fdtdec_get_bool(blob, node,
		"fsp,turbo-mode");
	silicon_upd->pnp_settings = fdtdec_get_int(blob, node,
		"fsp,pnp-settings", PNP_SETTING_POWER_AND_PERF);
	silicon_upd->sd_detect_chk = fdtdec_get_bool(blob, node,
		"fsp,sd-detect-chk");
}
