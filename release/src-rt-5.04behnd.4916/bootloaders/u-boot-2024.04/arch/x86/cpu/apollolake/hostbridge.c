// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 * Copyright (C) 2015 - 2017 Intel Corp.
 * Copyright (C) 2017 - 2019 Siemens AG
 * (Written by Alexandru Gagniuc <alexandrux.gagniuc@intel.com> for Intel Corp.)
 * (Written by Andrey Petrov <andrey.petrov@intel.com> for Intel Corp.)
 *
 * Portions from coreboot soc/intel/apollolake/chip.c
 */

#define LOG_CATEGORY UCLASS_NORTHBRIDGE

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <log.h>
#include <spl.h>
#include <tables_csum.h>
#include <acpi/acpi_table.h>
#include <asm/acpi_nhlt.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/acpi.h>
#include <asm/arch/hostbridge.h>
#include <asm/arch/systemagent.h>
#include <dt-bindings/sound/nhlt.h>
#include <dm/acpi.h>

enum {
	PCIEXBAR		= 0x60,
	PCIEXBAR_LENGTH_256MB	= 0,
	PCIEXBAR_LENGTH_128MB,
	PCIEXBAR_LENGTH_64MB,

	PCIEXBAR_PCIEXBAREN	= 1 << 0,

	BGSM			= 0xb4,  /* Base GTT Stolen Memory */
	TSEG			= 0xb8,  /* TSEG base */
	TOLUD			= 0xbc,
};

#if CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)
static const struct nhlt_format_config dmic_1ch_formats[] = {
	/* 48 KHz 16-bits per sample. */
	{
		.num_channels = 1,
		.sample_freq_khz = 48,
		.container_bits_per_sample = 16,
		.valid_bits_per_sample = 16,
		.settings_file = "dmic-1ch-48khz-16b.dat",
	},
};

static const struct nhlt_dmic_array_config dmic_1ch_mic_config = {
	.tdm_config = {
		.config_type = NHLT_TDM_MIC_ARRAY,
	},
	.array_type = NHLT_MIC_ARRAY_VENDOR_DEFINED,
};

static const struct nhlt_endp_descriptor dmic_1ch_descriptors[] = {
	{
		.link = NHLT_LINK_PDM,
		.device = NHLT_PDM_DEV,
		.direction = NHLT_DIR_CAPTURE,
		.vid = NHLT_VID,
		.did = NHLT_DID_DMIC,
		.cfg = &dmic_1ch_mic_config,
		.cfg_size = sizeof(dmic_1ch_mic_config),
		.formats = dmic_1ch_formats,
		.num_formats = ARRAY_SIZE(dmic_1ch_formats),
	},
};

static const struct nhlt_format_config dmic_2ch_formats[] = {
	/* 48 KHz 16-bits per sample. */
	{
		.num_channels = 2,
		.sample_freq_khz = 48,
		.container_bits_per_sample = 16,
		.valid_bits_per_sample = 16,
		.settings_file = "dmic-2ch-48khz-16b.dat",
	},
};

static const struct nhlt_dmic_array_config dmic_2ch_mic_config = {
	.tdm_config = {
		.config_type = NHLT_TDM_MIC_ARRAY,
	},
	.array_type = NHLT_MIC_ARRAY_2CH_SMALL,
};

static const struct nhlt_endp_descriptor dmic_2ch_descriptors[] = {
	{
		.link = NHLT_LINK_PDM,
		.device = NHLT_PDM_DEV,
		.direction = NHLT_DIR_CAPTURE,
		.vid = NHLT_VID,
		.did = NHLT_DID_DMIC,
		.cfg = &dmic_2ch_mic_config,
		.cfg_size = sizeof(dmic_2ch_mic_config),
		.formats = dmic_2ch_formats,
		.num_formats = ARRAY_SIZE(dmic_2ch_formats),
	},
};

static const struct nhlt_format_config dmic_4ch_formats[] = {
	/* 48 KHz 16-bits per sample. */
	{
		.num_channels = 4,
		.sample_freq_khz = 48,
		.container_bits_per_sample = 16,
		.valid_bits_per_sample = 16,
		.settings_file = "dmic-4ch-48khz-16b.dat",
	},
};

static const struct nhlt_dmic_array_config dmic_4ch_mic_config = {
	.tdm_config = {
		.config_type = NHLT_TDM_MIC_ARRAY,
	},
	.array_type = NHLT_MIC_ARRAY_4CH_L_SHAPED,
};

static const struct nhlt_endp_descriptor dmic_4ch_descriptors[] = {
	{
		.link = NHLT_LINK_PDM,
		.device = NHLT_PDM_DEV,
		.direction = NHLT_DIR_CAPTURE,
		.vid = NHLT_VID,
		.did = NHLT_DID_DMIC,
		.cfg = &dmic_4ch_mic_config,
		.cfg_size = sizeof(dmic_4ch_mic_config),
		.formats = dmic_4ch_formats,
		.num_formats = ARRAY_SIZE(dmic_4ch_formats),
	},
};
#endif

static int apl_hostbridge_early_init_pinctrl(struct udevice *dev)
{
	struct apl_hostbridge_plat *plat = dev_get_plat(dev);
	struct udevice *pinctrl;
	int ret;

	ret = uclass_first_device_err(UCLASS_PINCTRL, &pinctrl);
	if (ret)
		return log_msg_ret("no hostbridge pinctrl", ret);

	return pinctrl_config_pads(pinctrl, plat->early_pads,
				   plat->early_pads_count);
}

static int apl_hostbridge_early_init(struct udevice *dev)
{
	struct apl_hostbridge_plat *plat = dev_get_plat(dev);
	u32 region_size;
	ulong base;
	u32 reg;
	int ret;

	/* Set up the MCHBAR */
	pci_x86_read_config(plat->bdf, MCHBAR, &base, PCI_SIZE_32);
	base = MCH_BASE_ADDRESS;
	pci_x86_write_config(plat->bdf, MCHBAR, base | 1, PCI_SIZE_32);

	/*
	 * The PCIEXBAR is assumed to live in the memory mapped IO space under
	 * 4GiB
	 */
	pci_x86_write_config(plat->bdf, PCIEXBAR + 4, 0, PCI_SIZE_32);

	switch (plat->pciex_region_size >> 20) {
	default:
	case 256:
		region_size = PCIEXBAR_LENGTH_256MB;
		break;
	case 128:
		region_size = PCIEXBAR_LENGTH_128MB;
		break;
	case 64:
		region_size = PCIEXBAR_LENGTH_64MB;
		break;
	}

	reg = CONFIG_MMCONF_BASE_ADDRESS | (region_size << 1)
				| PCIEXBAR_PCIEXBAREN;
	pci_x86_write_config(plat->bdf, PCIEXBAR, reg, PCI_SIZE_32);

	/*
	 * TSEG defines the base of SMM range. BIOS determines the base
	 * of TSEG memory which must be at or below Graphics base of GTT
	 * Stolen memory, hence its better to clear TSEG register early
	 * to avoid power on default non-zero value (if any).
	 */
	pci_x86_write_config(plat->bdf, TSEG, 0, PCI_SIZE_32);

	ret = apl_hostbridge_early_init_pinctrl(dev);
	if (ret)
		return log_msg_ret("pinctrl", ret);

	return 0;
}

static int apl_hostbridge_of_to_plat(struct udevice *dev)
{
	struct apl_hostbridge_plat *plat = dev_get_plat(dev);
	struct udevice *pinctrl;
	int ret;

	/*
	 * The host bridge holds the early pad data needed to get through TPL.
	 * This is a small amount of data, enough to fit in TPL, so we keep it
	 * separate from the full pad data, stored in the fsp-s subnode. That
	 * subnode is not present in TPL, to save space.
	 */
	ret = uclass_first_device_err(UCLASS_PINCTRL, &pinctrl);
	if (ret)
		return log_msg_ret("no hostbridge PINCTRL", ret);
#if CONFIG_IS_ENABLED(OF_REAL)
	int root;

	/* Get length of PCI Express Region */
	plat->pciex_region_size = dev_read_u32_default(dev, "pciex-region-size",
						       256 << 20);

	root = pci_get_devfn(dev);
	if (root < 0)
		return log_msg_ret("Cannot get host-bridge PCI address", root);
	plat->bdf = root;

	ret = pinctrl_read_pads(pinctrl, dev_ofnode(dev), "early-pads",
				&plat->early_pads, &plat->early_pads_count);
	if (ret)
		return log_msg_ret("early-pads", ret);
#else
	struct dtd_intel_apl_hostbridge *dtplat = &plat->dtplat;
	int size;

	plat->pciex_region_size = dtplat->pciex_region_size;
	plat->bdf = pci_ofplat_get_devfn(dtplat->reg[0]);

	/* Assume that if everything is 0, it is empty */
	plat->early_pads = dtplat->early_pads;
	size = ARRAY_SIZE(dtplat->early_pads);
	plat->early_pads_count = pinctrl_count_pads(pinctrl, plat->early_pads,
						    size);

#endif

	return 0;
}

static int apl_hostbridge_probe(struct udevice *dev)
{
	if (spl_phase() == PHASE_TPL)
		return apl_hostbridge_early_init(dev);

	return 0;
}

static int apl_acpi_hb_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "RHUB");
}

#if CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)
static int apl_acpi_hb_write_tables(const struct udevice *dev,
				    struct acpi_ctx *ctx)
{
	struct acpi_table_header *header;
	struct acpi_dmar *dmar;
	u32 val;

	/*
	 * Create DMAR table only if virtualization is enabled. Due to some
	 * constraints on Apollo Lake SoC (some stepping affected), VTD could
	 * not be enabled together with IPU. Doing so will override and disable
	 * VTD while leaving CAPID0_A still reporting that VTD is available.
	 * As in this case FSP will lock VTD to disabled state, we need to make
	 * sure that DMAR table generation only happens when at least DEFVTBAR
	 * is enabled. Otherwise the DMAR header will be generated while the
	 * content of the table will be missing.
	 */
	dm_pci_read_config32(dev, CAPID0_A, &val);
	if ((val & VTD_DISABLE) ||
	    !(readl(MCHBAR_REG(DEFVTBAR)) & VTBAR_ENABLED))
		return 0;

	log_debug("ACPI:    * DMAR\n");
	dmar = (struct acpi_dmar *)ctx->current;
	header = &dmar->header;
	acpi_create_dmar(dmar, DMAR_INTR_REMAP);
	ctx->current += sizeof(struct acpi_dmar);
	apl_acpi_fill_dmar(ctx);

	/* (Re)calculate length and checksum */
	header->length = ctx->current - (void *)dmar;
	header->checksum = table_compute_checksum((void *)dmar, header->length);

	acpi_align(ctx);
	acpi_add_table(ctx, dmar);

	return 0;
}

static int apl_acpi_setup_nhlt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	struct nhlt *nhlt = ctx->nhlt;
	u32 channels;
	ofnode node;

	node = ofnode_find_subnode(dev_ofnode(dev), "nhlt");
	if (ofnode_read_u32(node, "intel,dmic-channels", &channels))
		return log_msg_ret("channels", -EINVAL);
	switch (channels) {
	case 1:
		return nhlt_add_endpoints(nhlt, dmic_1ch_descriptors,
					  ARRAY_SIZE(dmic_1ch_descriptors));
	case 2:
		return nhlt_add_endpoints(nhlt, dmic_2ch_descriptors,
					  ARRAY_SIZE(dmic_2ch_descriptors));
	case 4:
		return nhlt_add_endpoints(nhlt, dmic_4ch_descriptors,
					  ARRAY_SIZE(dmic_4ch_descriptors));
	}

	return log_msg_ret("channels", -EINVAL);
}
#endif

static int apl_hostbridge_remove(struct udevice *dev)
{
	/*
	 * TODO(sjg@chromium.org): Consider adding code from coreboot's
	 * platform_fsp_notify_status()
	 */

	return 0;
}

static ulong sa_read_reg(struct udevice *dev, int reg)
{
	u32 val;

	/* All regions concerned for have 1 MiB alignment */
	dm_pci_read_config32(dev, BGSM, &val);

	return ALIGN_DOWN(val, 1 << 20);
}

ulong sa_get_tolud_base(struct udevice *dev)
{
	return sa_read_reg(dev, TOLUD);
}

ulong sa_get_gsm_base(struct udevice *dev)
{
	return sa_read_reg(dev, BGSM);
}

ulong sa_get_tseg_base(struct udevice *dev)
{
	return sa_read_reg(dev, TSEG);
}

struct acpi_ops apl_hostbridge_acpi_ops = {
	.get_name	= apl_acpi_hb_get_name,
#if CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)
	.write_tables	= apl_acpi_hb_write_tables,
	.setup_nhlt	= apl_acpi_setup_nhlt,
#endif
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id apl_hostbridge_ids[] = {
	{ .compatible = "intel,apl-hostbridge" },
	{ }
};
#endif

U_BOOT_DRIVER(intel_apl_hostbridge) = {
	.name		= "intel_apl_hostbridge",
	.id		= UCLASS_NORTHBRIDGE,
	.of_match	= of_match_ptr(apl_hostbridge_ids),
	.of_to_plat = apl_hostbridge_of_to_plat,
	.probe		= apl_hostbridge_probe,
	.remove		= apl_hostbridge_remove,
	.plat_auto	= sizeof(struct apl_hostbridge_plat),
	ACPI_OPS_PTR(&apl_hostbridge_acpi_ops)
	.flags		= DM_FLAG_OS_PREPARE,
};
