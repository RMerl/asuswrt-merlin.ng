// SPDX-License-Identifier: GPL-2.0
/*
 * Support for Intel Application Digital Signal Processor
 *
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot file of the same name
 */

#define LOG_CATEGORY UCLASS_SYSCON

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/cpu.h>
#include <asm/intel_regs.h>
#include <asm/arch/adsp.h>
#include <asm/arch/pch.h>
#include <asm/arch/rcb.h>

enum pci_type_t {
	LYNX_POINT,
	WILDCAT_POINT,
};

struct broadwell_adsp_priv {
	bool adsp_d3_pg_enable;
	bool adsp_sram_pg_enable;
	bool sio_acpi_mode;
};

static int broadwell_adsp_probe(struct udevice *dev)
{
	struct broadwell_adsp_priv *priv = dev_get_priv(dev);
	enum pci_type_t type;
	u32 bar0, bar1;
	u32 tmp32;

	/* Find BAR0 and BAR1 */
	bar0 = dm_pci_read_bar32(dev, 0);
	if (!bar0)
		return -EINVAL;
	bar1 = dm_pci_read_bar32(dev, 1);
	if (!bar1)
		return -EINVAL;

	/*
	 * Set LTR value in DSP shim LTR control register to 3ms
	 * SNOOP_REQ[13]=1b SNOOP_SCALE[12:10]=100b (1ms) SNOOP_VAL[9:0]=3h
	 */
	type = dev_get_driver_data(dev);
	tmp32 = type == WILDCAT_POINT ? ADSP_SHIM_BASE_WPT : ADSP_SHIM_BASE_LPT;
	writel(ADSP_SHIM_LTRC_VALUE, bar0 + tmp32);

	/* Program VDRTCTL2 D19:F0:A8[31:0] = 0x00000fff */
	dm_pci_write_config32(dev, ADSP_PCI_VDRTCTL2, ADSP_VDRTCTL2_VALUE);

	/* Program ADSP IOBP VDLDAT1 to 0x040100 */
	pch_iobp_write(ADSP_IOBP_VDLDAT1, ADSP_VDLDAT1_VALUE);

	/* Set D3 Power Gating Enable in D19:F0:A0 based on PCH type */
	dm_pci_read_config32(dev, ADSP_PCI_VDRTCTL0, &tmp32);
	if (type == WILDCAT_POINT) {
		if (priv->adsp_d3_pg_enable) {
			tmp32 &= ~ADSP_VDRTCTL0_D3PGD_WPT;
			if (priv->adsp_sram_pg_enable)
				tmp32 &= ~ADSP_VDRTCTL0_D3SRAMPGD_WPT;
			else
				tmp32 |= ADSP_VDRTCTL0_D3SRAMPGD_WPT;
		} else {
			tmp32 |= ADSP_VDRTCTL0_D3PGD_WPT;
		}
	} else {
		if (priv->adsp_d3_pg_enable) {
			tmp32 &= ~ADSP_VDRTCTL0_D3PGD_LPT;
			if (priv->adsp_sram_pg_enable)
				tmp32 &= ~ADSP_VDRTCTL0_D3SRAMPGD_LPT;
			else
				tmp32 |= ADSP_VDRTCTL0_D3SRAMPGD_LPT;
		} else {
			tmp32 |= ADSP_VDRTCTL0_D3PGD_LPT;
		}
	}
	dm_pci_write_config32(dev, ADSP_PCI_VDRTCTL0, tmp32);

	/* Set PSF Snoop to SA, RCBA+0x3350[10]=1b */
	setbits_le32(RCB_REG(0x3350), 1 << 10);

	/* Set DSP IOBP PMCTL 0x1e0=0x3f */
	pch_iobp_write(ADSP_IOBP_PMCTL, ADSP_PMCTL_VALUE);

	if (priv->sio_acpi_mode) {
		/* Configure for ACPI mode */
		log_info("ADSP: Enable ACPI Mode IRQ3\n");

		/* Set interrupt de-assert/assert opcode override to IRQ3 */
		pch_iobp_write(ADSP_IOBP_VDLDAT2, ADSP_IOBP_ACPI_IRQ3);

		/* Enable IRQ3 in RCBA */
		setbits_le32(RCB_REG(ACPIIRQEN), ADSP_ACPI_IRQEN);

		/* Set ACPI Interrupt Enable Bit */
		pch_iobp_update(ADSP_IOBP_PCICFGCTL, ~ADSP_PCICFGCTL_SPCBAD,
				ADSP_PCICFGCTL_ACPIIE);

		/* Put ADSP in D3hot */
		clrbits_le32(bar1 + PCH_PCS, PCH_PCS_PS_D3HOT);
	} else {
		log_info("ADSP: Enable PCI Mode IRQ23\n");

		/* Configure for PCI mode */
		dm_pci_write_config32(dev, PCI_INTERRUPT_LINE, ADSP_PCI_IRQ);

		/* Clear ACPI Interrupt Enable Bit */
		pch_iobp_update(ADSP_IOBP_PCICFGCTL,
				~(ADSP_PCICFGCTL_SPCBAD |
				 ADSP_PCICFGCTL_ACPIIE), 0);
	}

	return 0;
}

static int broadwell_adsp_of_to_plat(struct udevice *dev)
{
	struct broadwell_adsp_priv *priv = dev_get_priv(dev);

	priv->adsp_d3_pg_enable = dev_read_bool(dev, "intel,adsp-d3-pg-enable");
	priv->adsp_sram_pg_enable = dev_read_bool(dev,
						  "intel,adsp-sram-pg-enable");
	priv->sio_acpi_mode = dev_read_bool(dev, "intel,sio-acpi-mode");

	return 0;
}

static const struct udevice_id broadwell_adsp_ids[] = {
	{ .compatible = "intel,wildcatpoint-adsp", .data = WILDCAT_POINT },
	{ }
};

U_BOOT_DRIVER(broadwell_adsp_drv) = {
	.name		= "adsp",
	.id		= UCLASS_SYSCON,
	.of_to_plat	= broadwell_adsp_of_to_plat,
	.of_match	= broadwell_adsp_ids,
	.bind           = dm_scan_fdt_dev,
	.probe		= broadwell_adsp_probe,
};

static struct pci_device_id broadwell_adsp_supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL,
		PCI_DEVICE_ID_INTEL_WILDCATPOINT_ADSP) },
	{ },
};

U_BOOT_PCI_DEVICE(broadwell_adsp_drv, broadwell_adsp_supported);
