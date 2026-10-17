// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot
 * Copyright (C) 2008-2009 coresystems GmbH
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/pch_common.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

DECLARE_GLOBAL_DATA_PTR;

static void common_sata_init(struct udevice *dev, unsigned int port_map)
{
	u32 reg32;
	u16 reg16;

	/* Set IDE I/O Configuration */
	reg32 = SIG_MODE_PRI_NORMAL | FAST_PCB1 | FAST_PCB0 | PCB1 | PCB0;
	dm_pci_write_config32(dev, IDE_CONFIG, reg32);

	/* Port enable */
	dm_pci_read_config16(dev, 0x92, &reg16);
	reg16 &= ~0x3f;
	reg16 |= port_map;
	dm_pci_write_config16(dev, 0x92, reg16);

	/* SATA Initialization register */
	port_map &= 0xff;
	dm_pci_write_config32(dev, 0x94, ((port_map ^ 0x3f) << 24) | 0x183);
}

static void bd82x6x_sata_init(struct udevice *dev, struct udevice *pch)
{
	unsigned int port_map, speed_support, port_tx;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	const char *mode;
	u32 reg32;
	u16 reg16;

	debug("SATA: Initializing...\n");

	/* SATA configuration */
	port_map = fdtdec_get_int(blob, node, "intel,sata-port-map", 0);
	speed_support = fdtdec_get_int(blob, node,
				       "sata_interface_speed_support", 0);

	mode = fdt_getprop(blob, node, "intel,sata-mode", NULL);
	if (!mode || !strcmp(mode, "ahci")) {
		ulong abar;

		debug("SATA: Controller in AHCI mode\n");

		/* Set timings */
		dm_pci_write_config16(dev, IDE_TIM_PRI, IDE_DECODE_ENABLE |
				IDE_ISP_3_CLOCKS | IDE_RCT_1_CLOCKS |
				IDE_PPE0 | IDE_IE0 | IDE_TIME0);
		dm_pci_write_config16(dev, IDE_TIM_SEC, IDE_DECODE_ENABLE |
				IDE_ISP_5_CLOCKS | IDE_RCT_4_CLOCKS);

		/* Sync DMA */
		dm_pci_write_config16(dev, IDE_SDMA_CNT, IDE_PSDE0);
		dm_pci_write_config16(dev, IDE_SDMA_TIM, 0x0001);

		common_sata_init(dev, 0x8000 | port_map);

		/* Initialize AHCI memory-mapped space */
		abar = dm_pci_read_bar32(dev, 5);
		debug("ABAR: %08lx\n", abar);
		/* CAP (HBA Capabilities) : enable power management */
		reg32 = readl(abar + 0x00);
		reg32 |= 0x0c006000;  /* set PSC+SSC+SALP+SSS */
		reg32 &= ~0x00020060; /* clear SXS+EMS+PMS */
		/* Set ISS, if available */
		if (speed_support) {
			reg32 &= ~0x00f00000;
			reg32 |= (speed_support & 0x03) << 20;
		}
		writel(reg32, abar + 0x00);
		/* PI (Ports implemented) */
		writel(port_map, abar + 0x0c);
		(void) readl(abar + 0x0c); /* Read back 1 */
		(void) readl(abar + 0x0c); /* Read back 2 */
		/* CAP2 (HBA Capabilities Extended)*/
		reg32 = readl(abar + 0x24);
		reg32 &= ~0x00000002;
		writel(reg32, abar + 0x24);
		/* VSP (Vendor Specific Register */
		reg32 = readl(abar + 0xa0);
		reg32 &= ~0x00000005;
		writel(reg32, abar + 0xa0);
	} else if (!strcmp(mode, "combined")) {
		debug("SATA: Controller in combined mode\n");

		/* No AHCI: clear AHCI base */
		dm_pci_write_bar32(dev, 5, 0x00000000);
		/* And without AHCI BAR no memory decoding */
		dm_pci_read_config16(dev, PCI_COMMAND, &reg16);
		reg16 &= ~PCI_COMMAND_MEMORY;
		dm_pci_write_config16(dev, PCI_COMMAND, reg16);

		dm_pci_write_config8(dev, 0x09, 0x80);

		/* Set timings */
		dm_pci_write_config16(dev, IDE_TIM_PRI, IDE_DECODE_ENABLE |
				IDE_ISP_5_CLOCKS | IDE_RCT_4_CLOCKS);
		dm_pci_write_config16(dev, IDE_TIM_SEC, IDE_DECODE_ENABLE |
				IDE_ISP_3_CLOCKS | IDE_RCT_1_CLOCKS |
				IDE_PPE0 | IDE_IE0 | IDE_TIME0);

		/* Sync DMA */
		dm_pci_write_config16(dev, IDE_SDMA_CNT, IDE_SSDE0);
		dm_pci_write_config16(dev, IDE_SDMA_TIM, 0x0200);

		common_sata_init(dev, port_map);
	} else {
		debug("SATA: Controller in plain-ide mode\n");

		/* No AHCI: clear AHCI base */
		dm_pci_write_bar32(dev, 5, 0x00000000);

		/* And without AHCI BAR no memory decoding */
		dm_pci_read_config16(dev, PCI_COMMAND, &reg16);
		reg16 &= ~PCI_COMMAND_MEMORY;
		dm_pci_write_config16(dev, PCI_COMMAND, reg16);

		/*
		 * Native mode capable on both primary and secondary (0xa)
		 * OR'ed with enabled (0x50) = 0xf
		 */
		dm_pci_write_config8(dev, 0x09, 0x8f);

		/* Set timings */
		dm_pci_write_config16(dev, IDE_TIM_PRI, IDE_DECODE_ENABLE |
				IDE_ISP_3_CLOCKS | IDE_RCT_1_CLOCKS |
				IDE_PPE0 | IDE_IE0 | IDE_TIME0);
		dm_pci_write_config16(dev, IDE_TIM_SEC, IDE_DECODE_ENABLE |
				IDE_SITRE | IDE_ISP_3_CLOCKS |
				IDE_RCT_1_CLOCKS | IDE_IE0 | IDE_TIME0);

		/* Sync DMA */
		dm_pci_write_config16(dev, IDE_SDMA_CNT, IDE_SSDE0 | IDE_PSDE0);
		dm_pci_write_config16(dev, IDE_SDMA_TIM, 0x0201);

		common_sata_init(dev, port_map);
	}

	/* Set Gen3 Transmitter settings if needed */
	port_tx = fdtdec_get_int(blob, node, "intel,sata-port0-gen3-tx", 0);
	if (port_tx)
		pch_iobp_update(pch, SATA_IOBP_SP0G3IR, 0, port_tx);

	port_tx = fdtdec_get_int(blob, node, "intel,sata-port1-gen3-tx", 0);
	if (port_tx)
		pch_iobp_update(pch, SATA_IOBP_SP1G3IR, 0, port_tx);

	/* Additional Programming Requirements */
	pch_common_sir_write(dev, 0x04, 0x00001600);
	pch_common_sir_write(dev, 0x28, 0xa0000033);
	reg32 = pch_common_sir_read(dev, 0x54);
	reg32 &= 0xff000000;
	reg32 |= 0x5555aa;
	pch_common_sir_write(dev, 0x54, reg32);
	pch_common_sir_write(dev, 0x64, 0xcccc8484);
	reg32 = pch_common_sir_read(dev, 0x68);
	reg32 &= 0xffff0000;
	reg32 |= 0xcccc;
	pch_common_sir_write(dev, 0x68, reg32);
	reg32 = pch_common_sir_read(dev, 0x78);
	reg32 &= 0x0000ffff;
	reg32 |= 0x88880000;
	pch_common_sir_write(dev, 0x78, reg32);
	pch_common_sir_write(dev, 0x84, 0x001c7000);
	pch_common_sir_write(dev, 0x88, 0x88338822);
	pch_common_sir_write(dev, 0xa0, 0x001c7000);
	pch_common_sir_write(dev, 0xc4, 0x0c0c0c0c);
	pch_common_sir_write(dev, 0xc8, 0x0c0c0c0c);
	pch_common_sir_write(dev, 0xd4, 0x10000000);

	pch_iobp_update(pch, 0xea004001, 0x3fffffff, 0xc0000000);
	pch_iobp_update(pch, 0xea00408a, 0xfffffcff, 0x00000100);
}

static void bd82x6x_sata_enable(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	unsigned port_map;
	const char *mode;
	u16 map = 0;

	/*
	 * Set SATA controller mode early so the resource allocator can
	 * properly assign IO/Memory resources for the controller.
	 */
	mode = fdt_getprop(blob, node, "intel,sata-mode", NULL);
	if (mode && !strcmp(mode, "ahci"))
		map = 0x0060;
	port_map = fdtdec_get_int(blob, node, "intel,sata-port-map", 0);

	map |= (port_map ^ 0x3f) << 8;
	dm_pci_write_config16(dev, 0x90, map);
}

static int bd82x6x_sata_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;
	int ret;

	if (gd->flags & GD_FLG_RELOC) {
		ret = ahci_bind_scsi(dev, &scsi_dev);
		if (ret)
			return ret;
	}

	return 0;
}

static int bd82x6x_sata_probe(struct udevice *dev)
{
	struct udevice *pch;
	int ret;

	ret = uclass_first_device_err(UCLASS_PCH, &pch);
	if (ret)
		return ret;

	if (!(gd->flags & GD_FLG_RELOC))
		bd82x6x_sata_enable(dev);
	else {
		bd82x6x_sata_init(dev, pch);
		ret = ahci_probe_scsi_pci(dev);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id bd82x6x_ahci_ids[] = {
	{ .compatible = "intel,pantherpoint-ahci" },
	{ }
};

U_BOOT_DRIVER(ahci_ivybridge_drv) = {
	.name		= "ahci_ivybridge",
	.id		= UCLASS_AHCI,
	.of_match	= bd82x6x_ahci_ids,
	.bind		= bd82x6x_sata_bind,
	.probe		= bd82x6x_sata_probe,
};
