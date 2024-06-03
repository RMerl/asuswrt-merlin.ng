// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * From coreboot src/soc/intel/broadwell/sata.c
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/intel_regs.h>
#include <asm/lpc_common.h>
#include <asm/pch_common.h>
#include <asm/pch_common.h>
#include <asm/arch/pch.h>

struct sata_platdata {
	int port_map;
	uint port0_gen3_tx;
	uint port1_gen3_tx;
	uint port0_gen3_dtle;
	uint port1_gen3_dtle;

	/*
	 * SATA DEVSLP Mux
	 * 0 = port 0 DEVSLP on DEVSLP0/GPIO33
	 * 1 = port 3 DEVSLP on DEVSLP0/GPIO33
	 */
	int devslp_mux;

	/*
	 * DEVSLP Disable
	 * 0: DEVSLP is enabled
	 * 1: DEVSLP is disabled
	 */
	int devslp_disable;
};

static void broadwell_sata_init(struct udevice *dev)
{
	struct sata_platdata *plat = dev_get_platdata(dev);
	u32 reg32;
	u8 *abar;
	u16 reg16;
	int port;

	debug("SATA: Initializing controller in AHCI mode.\n");

	/* Set timings */
	dm_pci_write_config16(dev, IDE_TIM_PRI, IDE_DECODE_ENABLE);
	dm_pci_write_config16(dev, IDE_TIM_SEC, IDE_DECODE_ENABLE);

	/* for AHCI, Port Enable is managed in memory mapped space */
	dm_pci_read_config16(dev, 0x92, &reg16);
	reg16 &= ~0xf;
	reg16 |= 0x8000 | plat->port_map;
	dm_pci_write_config16(dev, 0x92, reg16);
	udelay(2);

	/* Setup register 98h */
	dm_pci_read_config32(dev, 0x98, &reg32);
	reg32 &= ~((1 << 31) | (1 << 30));
	reg32 |= 1 << 23;
	reg32 |= 1 << 24; /* Enable MPHY Dynamic Power Gating */
	dm_pci_write_config32(dev, 0x98, reg32);

	/* Setup register 9Ch */
	reg16 = 0;           /* Disable alternate ID */
	reg16 = 1 << 5;      /* BWG step 12 */
	dm_pci_write_config16(dev, 0x9c, reg16);

	/* SATA Initialization register */
	reg32 = 0x183;
	reg32 |= (plat->port_map ^ 0xf) << 24;
	reg32 |= (plat->devslp_mux & 1) << 15;
	dm_pci_write_config32(dev, 0x94, reg32);

	/* Initialize AHCI memory-mapped space */
	dm_pci_read_config32(dev, PCI_BASE_ADDRESS_5, &reg32);
	abar = (u8 *)reg32;
	debug("ABAR: %p\n", abar);

	/* CAP (HBA Capabilities) : enable power management */
	clrsetbits_le32(abar + 0x00, 0x00020060 /* SXS+EMS+PMS */,
			0x0c006000 /* PSC+SSC+SALP+SSS */ |
			1 << 18); /* SAM: SATA AHCI MODE ONLY */

	/* PI (Ports implemented) */
	writel(plat->port_map, abar + 0x0c);
	(void) readl(abar + 0x0c); /* Read back 1 */
	(void) readl(abar + 0x0c); /* Read back 2 */

	/* CAP2 (HBA Capabilities Extended)*/
	if (plat->devslp_disable) {
		clrbits_le32(abar + 0x24, 1 << 3);
	} else {
		/* Enable DEVSLP */
		setbits_le32(abar + 0x24, 1 << 5 | 1 << 4 | 1 << 3 | 1 << 2);

		for (port = 0; port < 4; port++) {
			if (!(plat->port_map & (1 << port)))
				continue;
			/* DEVSLP DSP */
			setbits_le32(abar + 0x144 + (0x80 * port), 1 << 1);
		}
	}

	/* Static Power Gating for unused ports */
	reg32 = readl(RCB_REG(0x3a84));
	/* Port 3 and 2 disabled */
	if ((plat->port_map & ((1 << 3)|(1 << 2))) == 0)
		reg32 |= (1 << 24) | (1 << 26);
	/* Port 1 and 0 disabled */
	if ((plat->port_map & ((1 << 1)|(1 << 0))) == 0)
		reg32 |= (1 << 20) | (1 << 18);
	writel(reg32, RCB_REG(0x3a84));

	/* Set Gen3 Transmitter settings if needed */
	if (plat->port0_gen3_tx)
		pch_iobp_update(SATA_IOBP_SP0_SECRT88,
				~(SATA_SECRT88_VADJ_MASK <<
				  SATA_SECRT88_VADJ_SHIFT),
				(plat->port0_gen3_tx &
				 SATA_SECRT88_VADJ_MASK)
				<< SATA_SECRT88_VADJ_SHIFT);

	if (plat->port1_gen3_tx)
		pch_iobp_update(SATA_IOBP_SP1_SECRT88,
				~(SATA_SECRT88_VADJ_MASK <<
				  SATA_SECRT88_VADJ_SHIFT),
				(plat->port1_gen3_tx &
				 SATA_SECRT88_VADJ_MASK)
				<< SATA_SECRT88_VADJ_SHIFT);

	/* Set Gen3 DTLE DATA / EDGE registers if needed */
	if (plat->port0_gen3_dtle) {
		pch_iobp_update(SATA_IOBP_SP0DTLE_DATA,
				~(SATA_DTLE_MASK << SATA_DTLE_DATA_SHIFT),
				(plat->port0_gen3_dtle & SATA_DTLE_MASK)
				<< SATA_DTLE_DATA_SHIFT);

		pch_iobp_update(SATA_IOBP_SP0DTLE_EDGE,
				~(SATA_DTLE_MASK << SATA_DTLE_EDGE_SHIFT),
				(plat->port0_gen3_dtle & SATA_DTLE_MASK)
				<< SATA_DTLE_EDGE_SHIFT);
	}

	if (plat->port1_gen3_dtle) {
		pch_iobp_update(SATA_IOBP_SP1DTLE_DATA,
				~(SATA_DTLE_MASK << SATA_DTLE_DATA_SHIFT),
				(plat->port1_gen3_dtle & SATA_DTLE_MASK)
				<< SATA_DTLE_DATA_SHIFT);

		pch_iobp_update(SATA_IOBP_SP1DTLE_EDGE,
				~(SATA_DTLE_MASK << SATA_DTLE_EDGE_SHIFT),
				(plat->port1_gen3_dtle & SATA_DTLE_MASK)
				<< SATA_DTLE_EDGE_SHIFT);
	}

	/*
	 * Additional Programming Requirements for Power Optimizer
	 */

	/* Step 1 */
	pch_common_sir_write(dev, 0x64, 0x883c9003);

	/* Step 2: SIR 68h[15:0] = 880Ah */
	reg32 = pch_common_sir_read(dev, 0x68);
	reg32 &= 0xffff0000;
	reg32 |= 0x880a;
	pch_common_sir_write(dev, 0x68, reg32);

	/* Step 3: SIR 60h[3] = 1 */
	reg32 = pch_common_sir_read(dev, 0x60);
	reg32 |= (1 << 3);
	pch_common_sir_write(dev, 0x60, reg32);

	/* Step 4: SIR 60h[0] = 1 */
	reg32 = pch_common_sir_read(dev, 0x60);
	reg32 |= (1 << 0);
	pch_common_sir_write(dev, 0x60, reg32);

	/* Step 5: SIR 60h[1] = 1 */
	reg32 = pch_common_sir_read(dev, 0x60);
	reg32 |= (1 << 1);
	pch_common_sir_write(dev, 0x60, reg32);

	/* Clock Gating */
	pch_common_sir_write(dev, 0x70, 0x3f00bf1f);
	pch_common_sir_write(dev, 0x54, 0xcf000f0f);
	pch_common_sir_write(dev, 0x58, 0x00190000);
	clrsetbits_le32(RCB_REG(0x333c), 0x00300000, 0x00c00000);

	dm_pci_read_config32(dev, 0x300, &reg32);
	reg32 |= 1 << 17 | 1 << 16 | 1 << 19;
	reg32 |= 1 << 31 | 1 << 30 | 1 << 29;
	dm_pci_write_config32(dev, 0x300, reg32);

	dm_pci_read_config32(dev, 0x98, &reg32);
	reg32 |= 1 << 29;
	dm_pci_write_config32(dev, 0x98, reg32);

	/* Register Lock */
	dm_pci_read_config32(dev, 0x9c, &reg32);
	reg32 |= 1 << 31;
	dm_pci_write_config32(dev, 0x9c, reg32);
}

static int broadwell_sata_enable(struct udevice *dev)
{
	struct sata_platdata *plat = dev_get_platdata(dev);
	struct gpio_desc desc;
	u16 map;
	int ret;

	/*
	 * Set SATA controller mode early so the resource allocator can
	 * properly assign IO/Memory resources for the controller.
	 */
	map = 0x0060;

	map |= (plat->port_map ^ 0x3f) << 8;
	dm_pci_write_config16(dev, 0x90, map);

	ret = gpio_request_by_name(dev, "reset-gpio", 0, &desc, GPIOD_IS_OUT);
	if (ret)
		return ret;

	return 0;
}

static int broadwell_sata_ofdata_to_platdata(struct udevice *dev)
{
	struct sata_platdata *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);

	plat->port_map = fdtdec_get_int(blob, node, "intel,sata-port-map", 0);
	plat->port0_gen3_tx = fdtdec_get_int(blob, node,
					"intel,sata-port0-gen3-tx", 0);

	return 0;
}

static int broadwell_sata_probe(struct udevice *dev)
{
	if (!(gd->flags & GD_FLG_RELOC))
		return broadwell_sata_enable(dev);
	else
		broadwell_sata_init(dev);

	return 0;
}

static const struct udevice_id broadwell_ahci_ids[] = {
	{ .compatible = "intel,wildcatpoint-ahci" },
	{ }
};

U_BOOT_DRIVER(ahci_broadwell_drv) = {
	.name		= "ahci_broadwell",
	.id		= UCLASS_AHCI,
	.of_match	= broadwell_ahci_ids,
	.ofdata_to_platdata	= broadwell_sata_ofdata_to_platdata,
	.probe		= broadwell_sata_probe,
	.platdata_auto_alloc_size	 = sizeof(struct sata_platdata),
};
