// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Texas Instruments, Inc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <generic-phy.h>
#include <power-domain.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "pcie_dw_common.h"

DECLARE_GLOBAL_DATA_PTR;

#define PCIE_VENDORID_MASK	GENMASK(15, 0)
#define PCIE_DEVICEID_SHIFT	16

#define PCIE_LINK_CAPABILITY		0x7c
#define PCIE_LINK_CTL_2			0xa0
#define TARGET_LINK_SPEED_MASK		0xf
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

#define PCIE_MISC_CONTROL_1_OFF		0x8bc
#define PCIE_DBI_RO_WR_EN		BIT(0)

#define PLR_OFFSET			0x700
#define PCIE_PORT_DEBUG0		(PLR_OFFSET + 0x28)
#define PORT_LOGIC_LTSSM_STATE_MASK	0x1f
#define PORT_LOGIC_LTSSM_STATE_L0	0x11

#define PCIE_LINK_UP_TIMEOUT_MS		100

/* Offsets from App base */
#define PCIE_CMD_STATUS			0x04
#define LTSSM_EN_VAL			BIT(0)


#define AM654_PCIE_DEV_TYPE_MASK	0x3
#define EP				0x0
#define LEG_EP				0x1
#define RC				0x2

/**
 * struct pcie_dw_ti - TI DW PCIe controller state
 *
 * @pci: The common PCIe DW structure
 * @app_base: The base address of application register space
 */
struct pcie_dw_ti {
	/* Must be first member of the struct */
	struct pcie_dw dw;
	void *app_base;
};

enum dw_pcie_device_mode {
	DW_PCIE_UNKNOWN_TYPE,
	DW_PCIE_EP_TYPE,
	DW_PCIE_LEG_EP_TYPE,
	DW_PCIE_RC_TYPE,
};

/**
 * pcie_dw_configure() - Configure link capabilities and speed
 *
 * @regs_base: A pointer to the PCIe controller registers
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void pcie_dw_configure(struct pcie_dw_ti *pci, u32 cap_speed)
{
	u32 val;

	dw_pcie_dbi_write_enable(&pci->dw, true);

	val = readl(pci->dw.dbi_base + PCIE_LINK_CAPABILITY);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dw.dbi_base + PCIE_LINK_CAPABILITY);

	val = readl(pci->dw.dbi_base + PCIE_LINK_CTL_2);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dw.dbi_base + PCIE_LINK_CTL_2);

	dw_pcie_dbi_write_enable(&pci->dw, false);
}

/**
 * is_link_up() - Return the link state
 *
 * @regs_base: A pointer to the PCIe DBICS registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link
 */
static int is_link_up(struct pcie_dw_ti *pci)
{
	u32 val;

	val = readl(pci->dw.dbi_base + PCIE_PORT_DEBUG0);
	val &= PORT_LOGIC_LTSSM_STATE_MASK;

	return (val == PORT_LOGIC_LTSSM_STATE_L0);
}

/**
 * wait_link_up() - Wait for the link to come up
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link (timeout)
 */
static int wait_link_up(struct pcie_dw_ti *pci)
{
	unsigned long timeout;

	timeout = get_timer(0) + PCIE_LINK_UP_TIMEOUT_MS;
	while (!is_link_up(pci)) {
		if (get_timer(0) > timeout)
			return 0;
	};

	return 1;
}

static int pcie_dw_ti_pcie_link_up(struct pcie_dw_ti *pci, u32 cap_speed)
{
	u32 val;

	if (is_link_up(pci)) {
		printf("PCI Link already up before configuration!\n");
		return 1;
	}

	/* DW pre link configurations */
	pcie_dw_configure(pci, cap_speed);

	/* Initiate link training */
	val = readl(pci->app_base + PCIE_CMD_STATUS);
	val |= LTSSM_EN_VAL;
	writel(val, pci->app_base + PCIE_CMD_STATUS);

	/* Check that link was established */
	if (!wait_link_up(pci))
		return 0;

	/*
	 * Link can be established in Gen 1. still need to wait
	 * till MAC nagaotiation is completed
	 */
	udelay(100);

	return 1;
}

static int pcie_am654_set_mode(struct pcie_dw_ti *pci,
			       enum dw_pcie_device_mode mode)
{
	struct regmap *syscon;
	u32 val;
	u32 mask;
	int ret;

	syscon = syscon_regmap_lookup_by_phandle(pci->dw.dev,
						 "ti,syscon-pcie-mode");
	if (IS_ERR(syscon))
		return 0;

	mask = AM654_PCIE_DEV_TYPE_MASK;

	switch (mode) {
	case DW_PCIE_RC_TYPE:
		val = RC;
		break;
	case DW_PCIE_EP_TYPE:
		val = EP;
		break;
	default:
		dev_err(pci->dw.dev, "INVALID device type %d\n", mode);
		return -EINVAL;
	}

	ret = regmap_update_bits(syscon, 0, mask, val);
	if (ret) {
		dev_err(pci->dw.dev, "failed to set pcie mode\n");
		return ret;
	}

	return 0;
}

static int pcie_dw_init_id(struct pcie_dw_ti *pci)
{
	struct regmap *devctrl_regs;
	unsigned int id;
	int ret;

	devctrl_regs = syscon_regmap_lookup_by_phandle(pci->dw.dev,
						       "ti,syscon-pcie-id");
	if (IS_ERR(devctrl_regs))
		return PTR_ERR(devctrl_regs);

	ret = regmap_read(devctrl_regs, 0, &id);
	if (ret)
		return ret;

	dw_pcie_dbi_write_enable(&pci->dw, true);
	writew(id & PCIE_VENDORID_MASK, pci->dw.dbi_base + PCI_VENDOR_ID);
	writew(id >> PCIE_DEVICEID_SHIFT, pci->dw.dbi_base + PCI_DEVICE_ID);
	dw_pcie_dbi_write_enable(&pci->dw, false);

	return 0;
}

/**
 * pcie_dw_ti_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_dw_ti_probe(struct udevice *dev)
{
	struct pcie_dw_ti *pci = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	struct power_domain pci_pwrdmn;
	struct phy phy0, phy1;
	int ret;

	ret = power_domain_get_by_index(dev, &pci_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "failed to get power domain\n");
		return ret;
	}

	ret = power_domain_on(&pci_pwrdmn);
	if (ret) {
		dev_err(dev, "Power domain on failed\n");
		return ret;
	}

	ret = generic_phy_get_by_name(dev,  "pcie-phy0", &phy0);
	if (ret) {
		dev_err(dev, "Unable to get phy0");
		return ret;
	}
	generic_phy_reset(&phy0);
	generic_phy_init(&phy0);
	generic_phy_power_on(&phy0);

	ret = generic_phy_get_by_name(dev,  "pcie-phy1", &phy1);
	if (ret) {
		dev_err(dev, "Unable to get phy1");
		return ret;
	}
	generic_phy_reset(&phy1);
	generic_phy_init(&phy1);
	generic_phy_power_on(&phy1);

	pci->dw.first_busno = dev_seq(dev);
	pci->dw.dev = dev;

	pcie_dw_setup_host(&pci->dw);
	pcie_dw_init_id(pci);

	if (device_is_compatible(dev, "ti,am654-pcie-rc"))
		pcie_am654_set_mode(pci, DW_PCIE_RC_TYPE);

	if (!pcie_dw_ti_pcie_link_up(pci, LINK_SPEED_GEN_2)) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
		return -ENODEV;
	}

	printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev_seq(dev),
	       pcie_dw_get_link_speed(&pci->dw),
	       pcie_dw_get_link_width(&pci->dw),
	       hose->first_busno);

	pcie_dw_prog_outbound_atu_unroll(&pci->dw, PCIE_ATU_REGION_INDEX0,
					 PCIE_ATU_TYPE_MEM,
					 pci->dw.mem.phys_start,
					 pci->dw.mem.bus_start, pci->dw.mem.size);

	return 0;
}

/**
 * pcie_dw_ti_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_dw_ti_of_to_plat(struct udevice *dev)
{
	struct pcie_dw_ti *pcie = dev_get_priv(dev);

	/* Get the controller base address */
	pcie->dw.dbi_base = (void *)dev_read_addr_name(dev, "dbics");
	if ((fdt_addr_t)pcie->dw.dbi_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the config space base address and size */
	pcie->dw.cfg_base = (void *)dev_read_addr_size_name(dev, "config",
							 &pcie->dw.cfg_size);
	if ((fdt_addr_t)pcie->dw.cfg_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the iATU base address and size */
	pcie->dw.atu_base = (void *)dev_read_addr_name(dev, "atu");
	if ((fdt_addr_t)pcie->dw.atu_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the app base address and size */
	pcie->app_base = (void *)dev_read_addr_name(dev, "app");
	if ((fdt_addr_t)pcie->app_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops pcie_dw_ti_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id pcie_dw_ti_ids[] = {
	{ .compatible = "ti,am654-pcie-rc" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_ti) = {
	.name			= "pcie_dw_ti",
	.id			= UCLASS_PCI,
	.of_match		= pcie_dw_ti_ids,
	.ops			= &pcie_dw_ti_ops,
	.of_to_plat	= pcie_dw_ti_of_to_plat,
	.probe			= pcie_dw_ti_probe,
	.priv_auto	= sizeof(struct pcie_dw_ti),
};
