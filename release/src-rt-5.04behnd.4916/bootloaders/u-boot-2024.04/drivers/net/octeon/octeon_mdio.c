// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#include <dm.h>
#include <fdt_support.h>
#include <log.h>
#include <miiphy.h>
#include <net.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/octeon-model.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-smix-defs.h>
#include <mach/cvmx-config.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-mdio.h>

#define CVMX_SMI_DRV_CTL   0x0001180000001828ull
#define DEFAULT_MDIO_SPEED 2500000 /** 2.5 MHz default speed */

/**
 * cvmx_smi_drv_ctl
 *
 * Enables the SMI interface.
 *
 */
union cvmx_smi_drv_ctl {
	u64 u64;
	struct cvmx_smi_drv_ctl_s {
		u64 reserved_14_63 : 50;
		u64 pctl : 6;
		u64 reserved_6_7 : 2;
		u64 nctl : 6;
	} s;
};

struct octeon_mdiobus {
	struct mii_dev *mii_dev;
	/**
	 * The local bus is in the lower 8 bits, followed by the remote bus in
	 * the top 8 bits.  Bit 16 will be set if the bus is non-local.
	 */
	u32 bus_id;

	int node;     /** Node number */
	int speed;    /** Bus speed, normally 2.5 MHz */
	int fdt_node; /** Node in FDT */
	bool local;   /** true if local MDIO bus */
};

static int octeon_mdio_read(struct udevice *mdio_dev, int phy_addr,
			    int dev_addr, int reg_addr)
{
	struct octeon_mdiobus *p = dev_get_priv(mdio_dev);
	struct mii_dev *dev = p->mii_dev;
	int value;

	debug("%s(0x%p(%s): bus_id=%d phy_addr=%d, 0x%x, 0x%x) - ", __func__,
	      dev, dev->name, p->bus_id, phy_addr, dev_addr, reg_addr);
	if (IS_ENABLED(CONFIG_PHYLIB_10G) && dev_addr != MDIO_DEVAD_NONE) {
		debug("clause 45 mode\n");
		value = cvmx_mdio_45_read(p->bus_id & 0xff, phy_addr, dev_addr,
					  reg_addr);
	} else {
		value = cvmx_mdio_read(p->bus_id & 0xff, phy_addr, reg_addr);
	}

	debug("Return value: 0x%x\n", value);
	return value;
}

static int octeon_mdio_write(struct udevice *mdio_dev, int phy_addr,
			     int dev_addr, int reg_addr, u16 value)
{
	struct octeon_mdiobus *p = dev_get_priv(mdio_dev);
	struct mii_dev *dev = p->mii_dev;

	debug("%s(0x%p(%s): bus_id=%d phy_addr=%d, 0x%x, 0x%x, 0x%x)\n",
	      __func__, dev, dev->name, p->bus_id, phy_addr, dev_addr, reg_addr,
	      value);

	if (IS_ENABLED(CONFIG_PHYLIB_10G) && dev_addr != MDIO_DEVAD_NONE) {
		debug("clause 45 mode\n");
		return cvmx_mdio_45_write(p->bus_id & 0xff, phy_addr, dev_addr,
					  reg_addr, value);
	}

	return cvmx_mdio_write(p->bus_id & 0xff, phy_addr, reg_addr, value);
}

/**
 * Converts a MDIO register address to a bus number
 *
 * @param reg_addr	MDIO base register address
 *
 * @return	MDIO bus number or -1 if invalid address
 */
int octeon_mdio_reg_addr_to_bus(u64 reg_addr)
{
	int bus_base;
	int bus;

	/* Adjust the bus number based on the node number */
	bus_base = cvmx_csr_addr_to_node(reg_addr) * 4;
	reg_addr = cvmx_csr_addr_strip_node(reg_addr);

	switch (reg_addr) {
	case 0x1180000001800:
	case 0x1180000003800: /* 68XX/78XX address */
		bus = 0;
		break;
	case 0x1180000001900:
	case 0x1180000003880:
		bus = 1;
		break;
	case 0x1180000003900:
		bus = 2;
		break;
	case 0x1180000003980:
		bus = 3;
		break;
	default:
		printf("%s: Unknown register address 0x%llx\n", __func__,
		       reg_addr);
		return -1;
	}
	bus += bus_base;
	debug("%s: address 0x%llx is bus %d\n", __func__, reg_addr, bus);
	return bus;
}

static int octeon_mdio_probe(struct udevice *dev)
{
	struct octeon_mdiobus *p = dev_get_priv(dev);
	union cvmx_smi_drv_ctl drv_ctl;
	cvmx_smix_clk_t smi_clk;
	u64 mdio_addr;
	int bus;
	u64 sclock;
	u32 sample_dly;
	u64 denom;

	mdio_addr = dev_read_addr(dev);
	debug("%s: Translated address: 0x%llx\n", __func__, mdio_addr);
	bus = octeon_mdio_reg_addr_to_bus(mdio_addr);
	p->bus_id = bus;
	debug("%s: bus: %d\n", __func__, bus);

	drv_ctl.u64 = csr_rd(CVMX_SMI_DRV_CTL);
	drv_ctl.s.pctl = dev_read_u32_default(dev, "cavium,pctl-drive-strength",
					      drv_ctl.s.pctl);
	drv_ctl.s.nctl = dev_read_u32_default(dev, "cavium,nctl-drive-strength",
					      drv_ctl.s.nctl);
	debug("%s: Set MDIO PCTL drive strength to 0x%x and NCTL drive strength to 0x%x\n",
	      __func__, drv_ctl.s.pctl, drv_ctl.s.nctl);
	csr_wr(CVMX_SMI_DRV_CTL, drv_ctl.u64);

	/* Set the bus speed, default is 2.5MHz */
	p->speed = dev_read_u32_default(dev, "cavium,max-speed",
					DEFAULT_MDIO_SPEED);
	sclock = gd->bus_clk;
	smi_clk.u64 = csr_rd(CVMX_SMIX_CLK(bus & 3));
	smi_clk.s.phase = sclock / (p->speed * 2);

	/* Allow sample delay to be specified */
	sample_dly = dev_read_u32_default(dev, "cavium,sample-delay", 0);
	/* Only change the sample delay if it is set, otherwise use
	 * the default value of 2.
	 */
	if (sample_dly) {
		u32 sample;

		denom = (sclock * 1000ULL) / sample_dly;
		debug("%s: sclock: %llu, sample_dly: %u ps, denom: %llu\n",
		      __func__, sclock, sample_dly, denom);
		sample = (sclock + denom - 1) / denom;
		debug("%s: sample: %u\n", __func__, smi_clk.s.sample);
		if (sample < 2) {
			printf("%s: warning: cavium,sample-delay %u ps is too small in device tree for %s\n",
			       __func__, sample_dly, dev->name);
			sample = 2;
		}
		if (sample > (2 * smi_clk.s.phase - 3)) {
			printf("%s: warning: cavium,sample-delay %u ps is too large in device tree for %s\n",
			       __func__, sample_dly, dev->name);
			sample = 2 * smi_clk.s.phase - 3;
		}
		smi_clk.s.sample = sample & 0xf;
		smi_clk.s.sample_hi = (sample >> 4) & 0xf;
		debug("%s(%s): sample delay: %u ps (%d clocks)\n", __func__,
		      dev->name, sample_dly, smi_clk.s.sample);
	}
	csr_wr(CVMX_SMIX_CLK(bus & 3), smi_clk.u64);

	debug("mdio clock phase: %d clocks\n", smi_clk.s.phase);
	csr_wr(CVMX_SMIX_CLK(bus & 3), smi_clk.u64);
	debug("Enabling SMI interface %s\n", dev->name);
	csr_wr(CVMX_SMIX_EN(bus & 3), 1);

	/* Muxed MDIO bus support removed for now! */
	return 0;
}

static const struct mdio_ops octeon_mdio_ops = {
	.read = octeon_mdio_read,
	.write = octeon_mdio_write,
};

static const struct udevice_id octeon_mdio_ids[] = {
	{ .compatible = "cavium,octeon-3860-mdio" },
	{}
};

U_BOOT_DRIVER(octeon_mdio) = {
	.name = "octeon_mdio",
	.id = UCLASS_MDIO,
	.of_match = octeon_mdio_ids,
	.probe = octeon_mdio_probe,
	.ops = &octeon_mdio_ops,
	.priv_auto = sizeof(struct octeon_mdiobus),
};
