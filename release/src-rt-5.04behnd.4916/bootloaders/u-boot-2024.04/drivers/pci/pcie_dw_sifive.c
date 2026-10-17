// SPDX-License-Identifier: GPL-2.0+
/*
 * SiFive FU740 DesignWare PCIe Controller
 *
 * Copyright (C) 2020-2021 SiFive, Inc.
 *
 * Based in early part on the i.MX6 PCIe host controller shim which is:
 *
 * Copyright (C) 2013 Kosagi
 *		http://www.kosagi.com
 *
 * Based on driver from author: Alan Mikhak <amikhak@wirelessfabric.com>
 */
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/log2.h>
#include <pci.h>
#include <pci_ep.h>
#include <pci_ids.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>

#include "pcie_dw_common.h"

struct pcie_sifive {
	/* Must be first member of the struct */
	struct pcie_dw dw;

	/* private control regs */
	void __iomem *priv_base;

	/* reset, power, clock resources */
	int sys_int_pin;
	struct gpio_desc pwren_gpio;
	struct gpio_desc reset_gpio;
	struct clk aux_ck;
	struct reset_ctl reset;
};

enum pcie_sifive_devtype {
	SV_PCIE_UNKNOWN_TYPE = 0,
	SV_PCIE_ENDPOINT_TYPE = 1,
	SV_PCIE_HOST_TYPE = 3
};

#define ASSERTION_DELAY		100
#define PCIE_PERST_ASSERT	0x0
#define PCIE_PERST_DEASSERT	0x1
#define PCIE_PHY_RESET		0x1
#define PCIE_PHY_RESET_DEASSERT	0x0
#define GPIO_LOW		0x0
#define GPIO_HIGH		0x1
#define PCIE_PHY_SEL		0x1

#define sv_info(sv, fmt, arg...)	printf(fmt, ## arg)
#define sv_warn(sv, fmt, arg...)	printf(fmt, ## arg)
#define sv_debug(sv, fmt, arg...)	debug(fmt, ## arg)
#define sv_err(sv, fmt, arg...)		printf(fmt, ## arg)

/* Doorbell Interface */
#define DBI_OFFSET			0x0
#define DBI_SIZE			0x1000

#define PL_OFFSET			0x700

#define PHY_DEBUG_R0			(PL_OFFSET + 0x28)

#define PHY_DEBUG_R1			(PL_OFFSET + 0x2c)
#define PHY_DEBUG_R1_LINK_UP		(0x1 << 4)
#define PHY_DEBUG_R1_LINK_IN_TRAINING	(0x1 << 29)

#define PCIE_MISC_CONTROL_1		0x8bc
#define DBI_RO_WR_EN			BIT(0)

/* pcie reset */
#define PCIEX8MGMT_PERST_N		0x0

/* LTSSM */
#define PCIEX8MGMT_APP_LTSSM_ENABLE	0x10
#define LTSSM_ENABLE_BIT		BIT(0)

/* phy reset */
#define PCIEX8MGMT_APP_HOLD_PHY_RST	0x18

/* device type */
#define PCIEX8MGMT_DEVICE_TYPE		0x708
#define DEVICE_TYPE_EP			0x0
#define DEVICE_TYPE_RC			0x4

/* phy control registers*/
#define PCIEX8MGMT_PHY0_CR_PARA_ADDR	0x860
#define PCIEX8MGMT_PHY0_CR_PARA_RD_EN	0x870
#define PCIEX8MGMT_PHY0_CR_PARA_RD_DATA	0x878
#define PCIEX8MGMT_PHY0_CR_PARA_SEL	0x880
#define PCIEX8MGMT_PHY0_CR_PARA_WR_DATA	0x888
#define PCIEX8MGMT_PHY0_CR_PARA_WR_EN	0x890
#define PCIEX8MGMT_PHY0_CR_PARA_ACK	0x898
#define PCIEX8MGMT_PHY1_CR_PARA_ADDR	0x8a0
#define PCIEX8MGMT_PHY1_CR_PARA_RD_EN	0x8b0
#define PCIEX8MGMT_PHY1_CR_PARA_RD_DATA	0x8b8
#define PCIEX8MGMT_PHY1_CR_PARA_SEL	0x8c0
#define PCIEX8MGMT_PHY1_CR_PARA_WR_DATA	0x8c8
#define PCIEX8MGMT_PHY1_CR_PARA_WR_EN	0x8d0
#define PCIEX8MGMT_PHY1_CR_PARA_ACK	0x8d8

#define PCIEX8MGMT_LANE_NUM		8
#define PCIEX8MGMT_LANE			0x1008
#define PCIEX8MGMT_LANE_OFF		0x100
#define PCIEX8MGMT_TERM_MODE		0x0e21

#define PCIE_CAP_BASE			0x70
#define PCI_CONFIG(r)			(DBI_OFFSET + (r))
#define PCIE_CAPABILITIES(r)		PCI_CONFIG(PCIE_CAP_BASE + (r))

/* Link capability */
#define PF0_PCIE_CAP_LINK_CAP		PCIE_CAPABILITIES(0xc)
#define PCIE_LINK_CAP_MAX_SPEED_MASK	0xf
#define PCIE_LINK_CAP_MAX_SPEED_GEN1	BIT(0)
#define PCIE_LINK_CAP_MAX_SPEED_GEN2	BIT(1)
#define PCIE_LINK_CAP_MAX_SPEED_GEN3	BIT(2)
#define PCIE_LINK_CAP_MAX_SPEED_GEN4	BIT(3)

static enum pcie_sifive_devtype pcie_sifive_get_devtype(struct pcie_sifive *sv)
{
	u32 val;

	val = readl(sv->priv_base + PCIEX8MGMT_DEVICE_TYPE);
	switch (val) {
	case DEVICE_TYPE_RC:
		return SV_PCIE_HOST_TYPE;
	case DEVICE_TYPE_EP:
		return SV_PCIE_ENDPOINT_TYPE;
	default:
		return SV_PCIE_UNKNOWN_TYPE;
	}
}

static void pcie_sifive_priv_set_state(struct pcie_sifive *sv, u32 reg,
				       u32 bits, int state)
{
	u32 val;

	val = readl(sv->priv_base + reg);
	val = state ? (val | bits) : (val & !bits);
	writel(val, sv->priv_base + reg);
}

static void pcie_sifive_assert_reset(struct pcie_sifive *sv)
{
	dm_gpio_set_value(&sv->reset_gpio, GPIO_LOW);
	writel(PCIE_PERST_ASSERT, sv->priv_base + PCIEX8MGMT_PERST_N);
	mdelay(ASSERTION_DELAY);
}

static void pcie_sifive_power_on(struct pcie_sifive *sv)
{
	dm_gpio_set_value(&sv->pwren_gpio, GPIO_HIGH);
	mdelay(ASSERTION_DELAY);
}

static void pcie_sifive_deassert_reset(struct pcie_sifive *sv)
{
	writel(PCIE_PERST_DEASSERT, sv->priv_base + PCIEX8MGMT_PERST_N);
	dm_gpio_set_value(&sv->reset_gpio, GPIO_HIGH);
	mdelay(ASSERTION_DELAY);
}

static int pcie_sifive_setphy(const u8 phy, const u8 write,
			      const u16 addr, const u16 wrdata,
			      u16 *rddata, struct pcie_sifive *sv)
{
	unsigned char ack = 0;

	if (!(phy == 0 || phy == 1))
		return -2;

	/* setup phy para */
	writel(addr, sv->priv_base +
	       (phy ? PCIEX8MGMT_PHY1_CR_PARA_ADDR :
		PCIEX8MGMT_PHY0_CR_PARA_ADDR));

	if (write)
		writel(wrdata, sv->priv_base +
		       (phy ? PCIEX8MGMT_PHY1_CR_PARA_WR_DATA :
			PCIEX8MGMT_PHY0_CR_PARA_WR_DATA));

	/* enable access if write */
	if (write)
		writel(1, sv->priv_base +
		       (phy ? PCIEX8MGMT_PHY1_CR_PARA_WR_EN :
			PCIEX8MGMT_PHY0_CR_PARA_WR_EN));
	else
		writel(1, sv->priv_base +
		       (phy ? PCIEX8MGMT_PHY1_CR_PARA_RD_EN :
			PCIEX8MGMT_PHY0_CR_PARA_RD_EN));

	/* wait for wait_idle */
	do {
		u32 val;

		val = readl(sv->priv_base +
			    (phy ? PCIEX8MGMT_PHY1_CR_PARA_ACK :
			     PCIEX8MGMT_PHY0_CR_PARA_ACK));
		if (val) {
			ack = 1;
			if (!write)
				readl(sv->priv_base +
				      (phy ? PCIEX8MGMT_PHY1_CR_PARA_RD_DATA :
				       PCIEX8MGMT_PHY0_CR_PARA_RD_DATA));
			mdelay(1);
		}
	} while (!ack);

	/* clear */
	if (write)
		writel(0, sv->priv_base +
		       (phy ? PCIEX8MGMT_PHY1_CR_PARA_WR_EN :
			PCIEX8MGMT_PHY0_CR_PARA_WR_EN));
	else
		writel(0, sv->priv_base +
		       (phy ? PCIEX8MGMT_PHY1_CR_PARA_RD_EN :
			PCIEX8MGMT_PHY0_CR_PARA_RD_EN));

	while (readl(sv->priv_base +
		     (phy ? PCIEX8MGMT_PHY1_CR_PARA_ACK :
		      PCIEX8MGMT_PHY0_CR_PARA_ACK))) {
		/* wait for ~wait_idle */
	}

	return 0;
}

static void pcie_sifive_init_phy(struct pcie_sifive *sv)
{
	int lane;

	/* enable phy cr_para_sel interfaces */
	writel(PCIE_PHY_SEL, sv->priv_base + PCIEX8MGMT_PHY0_CR_PARA_SEL);
	writel(PCIE_PHY_SEL, sv->priv_base + PCIEX8MGMT_PHY1_CR_PARA_SEL);
	mdelay(1);

	/* set PHY AC termination mode */
	for (lane = 0; lane < PCIEX8MGMT_LANE_NUM; lane++) {
		pcie_sifive_setphy(0, 1,
				   PCIEX8MGMT_LANE +
				   (PCIEX8MGMT_LANE_OFF * lane),
				   PCIEX8MGMT_TERM_MODE, NULL, sv);
		pcie_sifive_setphy(1, 1,
				   PCIEX8MGMT_LANE +
				   (PCIEX8MGMT_LANE_OFF * lane),
				   PCIEX8MGMT_TERM_MODE, NULL, sv);
	}
}

static int pcie_sifive_check_link(struct pcie_sifive *sv)
{
	u32 val;

	val = readl(sv->dw.dbi_base + PHY_DEBUG_R1);
	return (val & PHY_DEBUG_R1_LINK_UP) &&
		!(val & PHY_DEBUG_R1_LINK_IN_TRAINING);
}

static void pcie_sifive_force_gen1(struct pcie_sifive *sv)
{
	u32 val, linkcap;

	/*
	 * Force Gen1 operation when starting the link. In case the link is
	 * started in Gen2 mode, there is a possibility the devices on the
	 * bus will not be detected at all. This happens with PCIe switches.
	 */

	/* ctrl_ro_wr_enable */
	val = readl(sv->dw.dbi_base + PCIE_MISC_CONTROL_1);
	val |= DBI_RO_WR_EN;
	writel(val, sv->dw.dbi_base + PCIE_MISC_CONTROL_1);

	/* configure link cap */
	linkcap = readl(sv->dw.dbi_base + PF0_PCIE_CAP_LINK_CAP);
	linkcap |= PCIE_LINK_CAP_MAX_SPEED_MASK;
	writel(linkcap, sv->dw.dbi_base + PF0_PCIE_CAP_LINK_CAP);

	/* ctrl_ro_wr_disable */
	val &= ~DBI_RO_WR_EN;
	writel(val, sv->dw.dbi_base + PCIE_MISC_CONTROL_1);
}

static void pcie_sifive_print_phy_debug(struct pcie_sifive *sv)
{
	sv_err(sv, "PHY DEBUG_R0=0x%08x DEBUG_R1=0x%08x\n",
	       readl(sv->dw.dbi_base + PHY_DEBUG_R0),
	       readl(sv->dw.dbi_base + PHY_DEBUG_R1));
}

static int pcie_sifive_wait_for_link(struct pcie_sifive *sv)
{
	u32 val;
	int timeout;

	/* Wait for the link to train */
	mdelay(20);
	timeout = 20;

	do {
		mdelay(1);
	} while (--timeout && !pcie_sifive_check_link(sv));

	val = readl(sv->dw.dbi_base + PHY_DEBUG_R1);
	if (!(val & PHY_DEBUG_R1_LINK_UP) ||
	    (val & PHY_DEBUG_R1_LINK_IN_TRAINING)) {
		sv_info(sv, "Failed to negotiate PCIe link!\n");
		pcie_sifive_print_phy_debug(sv);
		writel(PCIE_PHY_RESET,
		       sv->priv_base + PCIEX8MGMT_APP_HOLD_PHY_RST);
		return -ETIMEDOUT;
	}

	return 0;
}

static int pcie_sifive_start_link(struct pcie_sifive *sv)
{
	if (pcie_sifive_check_link(sv))
		return -EALREADY;

	pcie_sifive_force_gen1(sv);

	/* set ltssm */
	pcie_sifive_priv_set_state(sv, PCIEX8MGMT_APP_LTSSM_ENABLE,
				   LTSSM_ENABLE_BIT, 1);
	return 0;
}

static int pcie_sifive_init_port(struct udevice *dev,
				 enum pcie_sifive_devtype mode)
{
	struct pcie_sifive *sv = dev_get_priv(dev);
	int ret;

	/* Power on reset */
	pcie_sifive_assert_reset(sv);
	pcie_sifive_power_on(sv);
	pcie_sifive_deassert_reset(sv);

	/* Enable pcieauxclk */
	ret = clk_enable(&sv->aux_ck);
	if (ret)
		dev_err(dev, "unable to enable pcie_aux clock\n");

	/*
	 * assert hold_phy_rst (hold the controller LTSSM in reset
	 * after power_up_rst_n for register programming with cr_para)
	 */
	writel(PCIE_PHY_RESET, sv->priv_base + PCIEX8MGMT_APP_HOLD_PHY_RST);

	/* deassert power_up_rst_n */
	ret = reset_deassert(&sv->reset);
	if (ret < 0) {
		dev_err(dev, "failed to deassert reset");
		return -EINVAL;
	}

	pcie_sifive_init_phy(sv);

	/* disable pcieauxclk */
	clk_disable(&sv->aux_ck);

	/* deassert hold_phy_rst */
	writel(PCIE_PHY_RESET_DEASSERT,
	       sv->priv_base + PCIEX8MGMT_APP_HOLD_PHY_RST);

	/* enable pcieauxclk */
	clk_enable(&sv->aux_ck);

	/* Set desired mode while core is not operational */
	if (mode == SV_PCIE_HOST_TYPE)
		writel(DEVICE_TYPE_RC,
		       sv->priv_base + PCIEX8MGMT_DEVICE_TYPE);
	else
		writel(DEVICE_TYPE_EP,
		       sv->priv_base + PCIEX8MGMT_DEVICE_TYPE);

	/* Confirm desired mode from operational core */
	if (pcie_sifive_get_devtype(sv) != mode)
		return -EINVAL;

	pcie_dw_setup_host(&sv->dw);

	if (pcie_sifive_start_link(sv) == -EALREADY)
		sv_info(sv, "PCIe link is already up\n");
	else if (pcie_sifive_wait_for_link(sv) == -ETIMEDOUT)
		return -ETIMEDOUT;

	return 0;
}

static int pcie_sifive_probe(struct udevice *dev)
{
	struct pcie_sifive *sv = dev_get_priv(dev);
	struct udevice *parent = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(parent);
	int err;

	sv->dw.first_busno = dev_seq(dev);
	sv->dw.dev = dev;

	err = pcie_sifive_init_port(dev, SV_PCIE_HOST_TYPE);
	if (err) {
		sv_info(sv, "Failed to init port.\n");
		return err;
	}

	printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n",
	       dev_seq(dev), pcie_dw_get_link_speed(&sv->dw),
	       pcie_dw_get_link_width(&sv->dw),
	       hose->first_busno);

	return pcie_dw_prog_outbound_atu_unroll(&sv->dw,
						PCIE_ATU_REGION_INDEX0,
						PCIE_ATU_TYPE_MEM,
						sv->dw.mem.phys_start,
						sv->dw.mem.bus_start,
						sv->dw.mem.size);
}

static void __iomem *get_fdt_addr(struct udevice *dev, const char *name)
{
	fdt_addr_t addr;

	addr = dev_read_addr_name(dev, name);

	return (addr == FDT_ADDR_T_NONE) ? NULL : (void __iomem *)addr;
}

static int pcie_sifive_of_to_plat(struct udevice *dev)
{
	struct pcie_sifive *sv = dev_get_priv(dev);
	int err;

	/* get designware DBI base addr */
	sv->dw.dbi_base = get_fdt_addr(dev, "dbi");
	if (!sv->dw.dbi_base)
		return -EINVAL;

	/* get private control base addr */
	sv->priv_base = get_fdt_addr(dev, "mgmt");
	if (!sv->priv_base)
		return -EINVAL;

	gpio_request_by_name(dev, "pwren-gpios", 0, &sv->pwren_gpio,
			     GPIOD_IS_OUT);

	if (!dm_gpio_is_valid(&sv->pwren_gpio)) {
		sv_info(sv, "pwren_gpio is invalid\n");
		return -EINVAL;
	}

	gpio_request_by_name(dev, "reset-gpios", 0, &sv->reset_gpio,
			     GPIOD_IS_OUT);

	if (!dm_gpio_is_valid(&sv->reset_gpio)) {
		sv_info(sv, "reset_gpio is invalid\n");
		return -EINVAL;
	}

	err = clk_get_by_index(dev, 0, &sv->aux_ck);
	if (err) {
		sv_info(sv, "clk_get_by_index(aux_ck) failed: %d\n", err);
		return err;
	}

	err = reset_get_by_index(dev, 0, &sv->reset);
	if (err) {
		sv_info(sv, "reset_get_by_index(reset) failed: %d\n", err);
		return err;
	}

	return 0;
}

static const struct dm_pci_ops pcie_sifive_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id pcie_sifive_ids[] = {
	{ .compatible = "sifive,fu740-pcie" },
	{}
};

U_BOOT_DRIVER(pcie_sifive) = {
	.name		= "pcie_sifive",
	.id		= UCLASS_PCI,
	.of_match	= pcie_sifive_ids,
	.ops		= &pcie_sifive_ops,
	.of_to_plat	= pcie_sifive_of_to_plat,
	.probe		= pcie_sifive_probe,
	.priv_auto	= sizeof(struct pcie_sifive),
};
