// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Intel Corp.
 * Copyright (C) 2017-2019 Siemens AG
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot apollolake/acpi.c
 */

#define LOG_CATEGORY LOGC_ACPI

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <p2sb.h>
#include <pci.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_s3.h>
#include <asm/acpi_table.h>
#include <asm/cpu_common.h>
#include <asm/intel_acpi.h>
#include <asm/intel_gnvs.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_pinctrl_defs.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/arch/iomap.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pm.h>
#include <asm/arch/systemagent.h>
#include <dm/acpi.h>
#include <dm/uclass-internal.h>
#include <power/acpi_pmc.h>

int arch_read_sci_irq_select(void)
{
	struct acpi_pmc_upriv *upriv;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &dev);
	if (ret)
		return log_msg_ret("pmc", ret);
	upriv = dev_get_uclass_priv(dev);

	return readl(upriv->pmc_bar0 + IRQ_REG);
}

int arch_write_sci_irq_select(uint scis)
{
	struct acpi_pmc_upriv *upriv;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &dev);
	if (ret)
		return log_msg_ret("pmc", ret);
	upriv = dev_get_uclass_priv(dev);
	writel(scis, upriv->pmc_bar0 + IRQ_REG);

	return 0;
}

/**
 * chromeos_init_acpi() - Initialise basic data to boot Chrome OS
 *
 * This tells Chrome OS to boot in developer mode
 *
 * @cros: Structure to initialise
 */
static void chromeos_init_acpi(struct chromeos_acpi_gnvs *cros)
{
	cros->active_main_fw = 1;
	cros->active_main_fw = 1; /* A */
	cros->switches = CHSW_DEVELOPER_SWITCH;
	cros->main_fw_type = 2; /* Developer */
}

int acpi_create_gnvs(struct acpi_global_nvs *gnvs)
{
	struct udevice *cpu;
	int ret;

	/* Clear out GNV */
	memset(gnvs, '\0', sizeof(*gnvs));

	/* TODO(sjg@chromium.org): Add the console log to gnvs->cbmc */

	if (IS_ENABLED(CONFIG_CHROMEOS))
		chromeos_init_acpi(&gnvs->chromeos);

	/* Set unknown wake source */
	gnvs->pm1i = ~0ULL;

	/* CPU core count */
	gnvs->pcnt = 1;
	ret = uclass_find_first_device(UCLASS_CPU, &cpu);
	if (cpu) {
		ret = cpu_get_count(cpu);
		if (ret > 0)
			gnvs->pcnt = ret;
	}

	gnvs->dpte = 1;

	return 0;
}

uint32_t acpi_fill_soc_wake(uint32_t generic_pm1_en)
{
	/*
	 * WAK_STS bit is set when the system is in one of the sleep states
	 * (via the SLP_EN bit) and an enabled wake event occurs. Upon setting
	 * this bit, the PMC will transition the system to the ON state and
	 * can only be set by hardware and can only be cleared by writing a one
	 * to this bit position.
	 */
	generic_pm1_en |= WAK_STS | RTC_EN | PWRBTN_EN;

	return generic_pm1_en;
}

int arch_madt_sci_irq_polarity(int sci)
{
	return MP_IRQ_POLARITY_LOW;
}

void fill_fadt(struct acpi_fadt *fadt)
{
	fadt->pm_tmr_blk = IOMAP_ACPI_BASE + PM1_TMR;

	fadt->p_lvl2_lat = ACPI_FADT_C2_NOT_SUPPORTED;
	fadt->p_lvl3_lat = ACPI_FADT_C3_NOT_SUPPORTED;

	fadt->pm_tmr_len = 4;
	fadt->duty_width = 3;

	fadt->iapc_boot_arch = ACPI_FADT_LEGACY_DEVICES | ACPI_FADT_8042;

	fadt->x_pm_tmr_blk.space_id = 1;
	fadt->x_pm_tmr_blk.bit_width = fadt->pm_tmr_len * 8;
	fadt->x_pm_tmr_blk.addrl = IOMAP_ACPI_BASE + PM1_TMR;
}

static int apl_write_fadt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_fadt *fadt;

	fadt = ctx->current;
	acpi_fadt_common(fadt, ctx->facs, ctx->dsdt);
	intel_acpi_fill_fadt(fadt);
	fill_fadt(fadt);
	header = &fadt->header;
	header->checksum = table_compute_checksum(fadt, header->length);

	return acpi_add_fadt(ctx, fadt);
}
ACPI_WRITER(5fadt, "FADT", apl_write_fadt, 0);

int apl_acpi_fill_dmar(struct acpi_ctx *ctx)
{
	struct udevice *dev, *sa_dev;
	u64 gfxvtbar = readq(MCHBAR_REG(GFXVTBAR)) & VTBAR_MASK;
	u64 defvtbar = readq(MCHBAR_REG(DEFVTBAR)) & VTBAR_MASK;
	bool gfxvten = readl(MCHBAR_REG(GFXVTBAR)) & VTBAR_ENABLED;
	bool defvten = readl(MCHBAR_REG(DEFVTBAR)) & VTBAR_ENABLED;
	void *tmp;
	int ret;

	uclass_find_first_device(UCLASS_VIDEO, &dev);
	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &sa_dev);
	if (ret)
		return log_msg_ret("no sa", ret);

	/* IGD has to be enabled, GFXVTBAR set and enabled */
	if (dev && device_active(dev) && gfxvtbar && gfxvten) {
		tmp = ctx->current;

		acpi_create_dmar_drhd(ctx, 0, 0, gfxvtbar);
		ret = acpi_create_dmar_ds_pci(ctx, PCI_BDF(0, 2, 0));
		if (ret)
			return log_msg_ret("ds_pci", ret);
		acpi_dmar_drhd_fixup(ctx, tmp);

		/* Add RMRR entry */
		tmp = ctx->current;
		acpi_create_dmar_rmrr(ctx->current, 0, sa_get_gsm_base(sa_dev),
				      sa_get_tolud_base(sa_dev) - 1);
		acpi_create_dmar_ds_pci(ctx->current, PCI_BDF(0, 2, 0));
		acpi_dmar_rmrr_fixup(ctx, tmp);
	}

	/* DEFVTBAR has to be set and enabled */
	if (defvtbar && defvten) {
		struct udevice *p2sb_dev;
		u16 ibdf, hbdf;
		uint ioapic, hpet;
		int ret;

		tmp = ctx->current;
		/*
		 * P2SB may already be hidden. There's no clear rule, when.
		 * It is needed to get bus, device and function for IOAPIC and
		 * HPET device which is stored in P2SB device. So unhide it to
		 * get the info and hide it again when done.
		 *
		 * TODO(sjg@chromium.org): p2sb_unhide() ?
		 */
		ret = uclass_first_device_err(UCLASS_P2SB, &p2sb_dev);
		if (ret)
			return log_msg_ret("p2sb", ret);

		dm_pci_read_config16(p2sb_dev, PCH_P2SB_IBDF, &ibdf);
		ioapic = PCI_TO_BDF(ibdf);
		dm_pci_read_config16(p2sb_dev, PCH_P2SB_HBDF, &hbdf);
		hpet = PCI_TO_BDF(hbdf);
		/* TODO(sjg@chromium.org): p2sb_hide() ? */

		acpi_create_dmar_drhd(ctx, DRHD_INCLUDE_PCI_ALL, 0, defvtbar);
		acpi_create_dmar_ds_ioapic(ctx, 2, ioapic);
		acpi_create_dmar_ds_msi_hpet(ctx, 0, hpet);
		acpi_dmar_drhd_fixup(tmp, ctx->current);
	}

	return 0;
}
