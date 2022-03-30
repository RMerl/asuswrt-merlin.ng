// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>

#include "../common/ngpixis.h"
#include "../common/fman.h"
#include <fsl_dtsec.h>

#define EMI_NONE	0xffffffff
#define EMI_MASK	0xf0000000
#define EMI1_RGMII	0x0
#define EMI1_SLOT3	0x80000000	/* bank1 EFGH */
#define EMI1_SLOT4	0x40000000	/* bank2 ABCD */
#define EMI1_SLOT5	0xc0000000	/* bank3 ABCD */
#define EMI2_SLOT4	0x10000000	/* bank2 ABCD */
#define EMI2_SLOT5	0x30000000	/* bank3 ABCD */
#define EMI1_MASK	0xc0000000
#define EMI2_MASK	0x30000000

#define PHY_BASE_ADDR	0x00
#define PHY_BASE_ADDR_SLOT5	0x10

static int mdio_mux[NUM_FM_PORTS];

static char *mdio_names[16] = {
	"P4080DS_MDIO0",
	"P4080DS_MDIO1",
	NULL,
	"P4080DS_MDIO3",
	"P4080DS_MDIO4",
	NULL, NULL, NULL,
	"P4080DS_MDIO8",
	NULL, NULL, NULL,
	"P4080DS_MDIO12",
	NULL, NULL, NULL,
};

/*
 * Mapping of all 18 SERDES lanes to board slots. A value of '0' here means
 * that the mapping must be determined dynamically, or that the lane maps to
 * something other than a board slot.
 */
static u8 lane_to_slot[] = {
	1, 1, 2, 2, 3, 3, 3, 3, 6, 6, 4, 4, 4, 4, 5, 5, 5, 5
};

static char *p4080ds_mdio_name_for_muxval(u32 muxval)
{
	return mdio_names[(muxval & EMI_MASK) >> 28];
}

struct mii_dev *mii_dev_for_muxval(u32 muxval)
{
	struct mii_dev *bus;
	char *name = p4080ds_mdio_name_for_muxval(muxval);

	if (!name) {
		printf("No bus for muxval %x\n", muxval);
		return NULL;
	}

	bus = miiphy_get_dev_by_name(name);

	if (!bus) {
		printf("No bus by name %s\n", name);
		return NULL;
	}

	return bus;
}

#if defined(CONFIG_SYS_P4080_ERRATUM_SERDES9) && defined(CONFIG_PHY_TERANETICS)
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	if (phydev->drv->uid == PHY_UID_TN2020) {
		unsigned long timeout = 1 * 1000; /* 1 seconds */
		enum srds_prtcl device;

		/*
		 * Wait for the XAUI to come out of reset.  This is when it
		 * starts transmitting alignment signals.
		 */
		while (--timeout) {
			int reg = phy_read(phydev, MDIO_MMD_PHYXS, MDIO_CTRL1);
			if (reg < 0) {
				printf("TN2020: Error reading from PHY at "
				       "address %u\n", phydev->addr);
				break;
			}
			/*
			 * Note that we've never actually seen
			 * MDIO_CTRL1_RESET set to 1.
			 */
			if ((reg & MDIO_CTRL1_RESET) == 0)
				break;
			udelay(1000);
		}

		if (!timeout) {
			printf("TN2020: Timeout waiting for PHY at address %u "
			       " to reset.\n", phydev->addr);
		}

		switch (phydev->addr) {
		case CONFIG_SYS_FM1_10GEC1_PHY_ADDR:
			device = XAUI_FM1;
			break;
		case CONFIG_SYS_FM2_10GEC1_PHY_ADDR:
			device = XAUI_FM2;
			break;
		default:
			device = NONE;
		}

		serdes_reset_rx(device);
	}

	return 0;
}
#endif

struct p4080ds_mdio {
	u32 muxval;
	struct mii_dev *realbus;
};

static void p4080ds_mux_mdio(u32 muxval)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	uint gpioval = in_be32(&pgpio->gpdat) & ~(EMI_MASK);
	gpioval |= muxval;

	out_be32(&pgpio->gpdat, gpioval);
}

static int p4080ds_mdio_read(struct mii_dev *bus, int addr, int devad,
				int regnum)
{
	struct p4080ds_mdio *priv = bus->priv;

	p4080ds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int p4080ds_mdio_write(struct mii_dev *bus, int addr, int devad,
				int regnum, u16 value)
{
	struct p4080ds_mdio *priv = bus->priv;

	p4080ds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int p4080ds_mdio_reset(struct mii_dev *bus)
{
	struct p4080ds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int p4080ds_mdio_init(char *realbusname, u32 muxval)
{
	struct p4080ds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate P4080DS MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate P4080DS private data\n");
		free(bus);
		return -1;
	}

	bus->read = p4080ds_mdio_read;
	bus->write = p4080ds_mdio_write;
	bus->reset = p4080ds_mdio_reset;
	sprintf(bus->name, p4080ds_mdio_name_for_muxval(muxval));

	pmdio->realbus = miiphy_get_dev_by_name(realbusname);

	if (!pmdio->realbus) {
		printf("No bus with name %s\n", realbusname);
		free(bus);
		free(pmdio);
		return -1;
	}

	pmdio->muxval = muxval;
	bus->priv = pmdio;

	return mdio_register(bus);
}

void board_ft_fman_fixup_port(void *blob, char * prop, phys_addr_t pa,
				enum fm_port port, int offset)
{
	if (mdio_mux[port] == EMI1_RGMII)
		fdt_set_phy_handle(blob, prop, pa, "phy_rgmii");

	if (mdio_mux[port] == EMI1_SLOT3) {
		int idx = port - FM2_DTSEC1 + 5;
		char phy[16];

		sprintf(phy, "phy%d_slot3", idx);

		fdt_set_phy_handle(blob, prop, pa, phy);
	}
}

void fdt_fixup_board_enet(void *fdt)
{
	int i;

	/*
	 * P4080DS can be configured in many different ways, supporting a number
	 * of combinations of ethernet devices and phy types.  In order to
	 * have just one device tree for all of those configurations, we fix up
	 * the tree here.  By default, the device tree configures FM1 and FM2
	 * for SGMII, and configures XAUI on both 10G interfaces.  So we have
	 * a number of different variables to track:
	 *
	 * 1) Whether the device is configured at all.  Whichever devices are
	 *    not enabled should be disabled by setting the "status" property
	 *    to "disabled".
	 * 2) What the PHY interface is.  If this is an RGMII connection,
	 *    we should change the "phy-connection-type" property to
	 *    "rgmii"
	 * 3) Which PHY is being used.  Because the MDIO buses are muxed,
	 *    we need to redirect the "phy-handle" property to point at the
	 *    PHY on the right slot/bus.
	 */

	/* We've got six MDIO nodes that may or may not need to exist */
	fdt_status_disabled_by_alias(fdt, "emi1_slot3");
	fdt_status_disabled_by_alias(fdt, "emi1_slot4");
	fdt_status_disabled_by_alias(fdt, "emi1_slot5");
	fdt_status_disabled_by_alias(fdt, "emi2_slot4");
	fdt_status_disabled_by_alias(fdt, "emi2_slot5");

	for (i = 0; i < NUM_FM_PORTS; i++) {
		switch (mdio_mux[i]) {
		case EMI1_SLOT3:
			fdt_status_okay_by_alias(fdt, "emi1_slot3");
			break;
		case EMI1_SLOT4:
			fdt_status_okay_by_alias(fdt, "emi1_slot4");
			break;
		case EMI1_SLOT5:
			fdt_status_okay_by_alias(fdt, "emi1_slot5");
			break;
		case EMI2_SLOT4:
			fdt_status_okay_by_alias(fdt, "emi2_slot4");
			break;
		case EMI2_SLOT5:
			fdt_status_okay_by_alias(fdt, "emi2_slot5");
			break;
		}
	}
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	int i;
	struct fsl_pq_mdio_info dtsec_mdio_info;
	struct tgec_mdio_info tgec_mdio_info;
	struct mii_dev *bus;

	/* Initialize the mdio_mux array so we can recognize empty elements */
	for (i = 0; i < NUM_FM_PORTS; i++)
		mdio_mux[i] = EMI_NONE;

	/* The first 4 GPIOs are outputs to control MDIO bus muxing */
	out_be32(&pgpio->gpdir, EMI_MASK);

	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	tgec_mdio_info.regs =
		(struct tgec_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR;
	tgec_mdio_info.name = DEFAULT_FM_TGEC_MDIO_NAME;

	/* Register the 10G MDIO bus */
	fm_tgec_mdio_init(bis, &tgec_mdio_info);

	/* Register the 6 muxing front-ends to the MDIO buses */
	p4080ds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII);
	p4080ds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT3);
	p4080ds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT4);
	p4080ds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT5);
	p4080ds_mdio_init(DEFAULT_FM_TGEC_MDIO_NAME, EMI2_SLOT4);
	p4080ds_mdio_init(DEFAULT_FM_TGEC_MDIO_NAME, EMI2_SLOT5);

	fm_info_set_phy_address(FM1_DTSEC1, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC2, CONFIG_SYS_FM1_DTSEC2_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC3, CONFIG_SYS_FM1_DTSEC3_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, CONFIG_SYS_FM1_DTSEC4_PHY_ADDR);
	fm_info_set_phy_address(FM1_10GEC1, CONFIG_SYS_FM1_10GEC1_PHY_ADDR);

#if (CONFIG_SYS_NUM_FMAN == 2)
	fm_info_set_phy_address(FM2_DTSEC1, CONFIG_SYS_FM2_DTSEC1_PHY_ADDR);
	fm_info_set_phy_address(FM2_DTSEC2, CONFIG_SYS_FM2_DTSEC2_PHY_ADDR);
	fm_info_set_phy_address(FM2_DTSEC3, CONFIG_SYS_FM2_DTSEC3_PHY_ADDR);
	fm_info_set_phy_address(FM2_DTSEC4, CONFIG_SYS_FM2_DTSEC4_PHY_ADDR);
	fm_info_set_phy_address(FM2_10GEC1, CONFIG_SYS_FM2_10GEC1_PHY_ADDR);
#endif

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1, lane, slot;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			switch (slot) {
			case 3:
				mdio_mux[i] = EMI1_SLOT3;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 4:
				mdio_mux[i] = EMI1_SLOT4;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 5:
				mdio_mux[i] = EMI1_SLOT5;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			};
			break;
		case PHY_INTERFACE_MODE_RGMII:
			fm_info_set_phy_address(i, 0);
			mdio_mux[i] = EMI1_RGMII;
			fm_info_set_mdio(i,
				mii_dev_for_muxval(mdio_mux[i]));
			break;
		default:
			break;
		}
	}
	bus = mii_dev_for_muxval(EMI1_SLOT5);
	set_sgmii_phy(bus, FM1_DTSEC1,
		      CONFIG_SYS_NUM_FM1_DTSEC, PHY_BASE_ADDR_SLOT5);

	for (i = FM1_10GEC1; i < FM1_10GEC1 + CONFIG_SYS_NUM_FM1_10GEC; i++) {
		int idx = i - FM1_10GEC1, lane, slot;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			lane = serdes_get_first_lane(XAUI_FM1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			switch (slot) {
			case 4:
				mdio_mux[i] = EMI2_SLOT4;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 5:
				mdio_mux[i] = EMI2_SLOT5;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			};
			break;
		default:
			break;
		}
	}

#if (CONFIG_SYS_NUM_FMAN == 2)
	for (i = FM2_DTSEC1; i < FM2_DTSEC1 + CONFIG_SYS_NUM_FM2_DTSEC; i++) {
		int idx = i - FM2_DTSEC1, lane, slot;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM2_DTSEC1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			switch (slot) {
			case 3:
				mdio_mux[i] = EMI1_SLOT3;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 4:
				mdio_mux[i] = EMI1_SLOT4;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 5:
				mdio_mux[i] = EMI1_SLOT5;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			};
			break;
		case PHY_INTERFACE_MODE_RGMII:
			fm_info_set_phy_address(i, 0);
			mdio_mux[i] = EMI1_RGMII;
			fm_info_set_mdio(i,
				mii_dev_for_muxval(mdio_mux[i]));
			break;
		default:
			break;
		}
	}

	bus = mii_dev_for_muxval(EMI1_SLOT3);
	set_sgmii_phy(bus, FM2_DTSEC1, CONFIG_SYS_NUM_FM2_DTSEC, PHY_BASE_ADDR);
	bus = mii_dev_for_muxval(EMI1_SLOT4);
	set_sgmii_phy(bus, FM2_DTSEC1, CONFIG_SYS_NUM_FM2_DTSEC, PHY_BASE_ADDR);

	for (i = FM2_10GEC1; i < FM2_10GEC1 + CONFIG_SYS_NUM_FM2_10GEC; i++) {
		int idx = i - FM2_10GEC1, lane, slot;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			lane = serdes_get_first_lane(XAUI_FM2 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			switch (slot) {
			case 4:
				mdio_mux[i] = EMI2_SLOT4;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 5:
				mdio_mux[i] = EMI2_SLOT5;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			};
			break;
		default:
			break;
		}
	}
#endif

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}
