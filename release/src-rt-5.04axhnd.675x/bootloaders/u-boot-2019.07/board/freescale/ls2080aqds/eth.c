// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <hwconfig.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fm_eth.h>
#include <i2c.h>
#include <miiphy.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

#include "../common/qixis.h"

#include "ls2080aqds_qixis.h"

#define MC_BOOT_ENV_VAR "mcinitcmd"

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
 /* - In LS2080A there are only 16 SERDES lanes, spread across 2 SERDES banks.
 *   Bank 1 -> Lanes A, B, C, D, E, F, G, H
 *   Bank 2 -> Lanes A,B, C, D, E, F, G, H
 */

 /* Mapping of 16 SERDES lanes to LS2080A QDS board slots. A value of '0' here
  * means that the mapping must be determined dynamically, or that the lane
  * maps to something other than a board slot.
  */

static u8 lane_to_slot_fsm1[] = {
	0, 0, 0, 0, 0, 0, 0, 0
};

static u8 lane_to_slot_fsm2[] = {
	0, 0, 0, 0, 0, 0, 0, 0
};

/* On the Vitesse VSC8234XHG SGMII riser card there are 4 SGMII PHYs
 * housed.
 */

static int xqsgii_riser_phy_addr[] = {
	XQSGMII_CARD_PHY1_PORT0_ADDR,
	XQSGMII_CARD_PHY2_PORT0_ADDR,
	XQSGMII_CARD_PHY3_PORT0_ADDR,
	XQSGMII_CARD_PHY4_PORT0_ADDR,
	XQSGMII_CARD_PHY3_PORT2_ADDR,
	XQSGMII_CARD_PHY1_PORT2_ADDR,
	XQSGMII_CARD_PHY4_PORT2_ADDR,
	XQSGMII_CARD_PHY2_PORT2_ADDR,
};

static int sgmii_riser_phy_addr[] = {
	SGMII_CARD_PORT1_PHY_ADDR,
	SGMII_CARD_PORT2_PHY_ADDR,
	SGMII_CARD_PORT3_PHY_ADDR,
	SGMII_CARD_PORT4_PHY_ADDR,
};

/* Slot2 does not have EMI connections */
#define EMI_NONE	0xFF
#define EMI1_SLOT1	0
#define EMI1_SLOT2	1
#define EMI1_SLOT3	2
#define EMI1_SLOT4	3
#define EMI1_SLOT5	4
#define EMI1_SLOT6	5
#define EMI2		6
#define SFP_TX		0

static const char * const mdio_names[] = {
	"LS2080A_QDS_MDIO0",
	"LS2080A_QDS_MDIO1",
	"LS2080A_QDS_MDIO2",
	"LS2080A_QDS_MDIO3",
	"LS2080A_QDS_MDIO4",
	"LS2080A_QDS_MDIO5",
	DEFAULT_WRIOP_MDIO2_NAME,
};

struct ls2080a_qds_mdio {
	u8 muxval;
	struct mii_dev *realbus;
};

static void sgmii_configure_repeater(int serdes_port)
{
	struct mii_dev *bus;
	uint8_t a = 0xf;
	int i, j, ret;
	int dpmac_id = 0, dpmac, mii_bus = 0;
	unsigned short value;
	char dev[2][20] = {"LS2080A_QDS_MDIO0", "LS2080A_QDS_MDIO3"};
	uint8_t i2c_addr[] = {0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5f, 0x60};

	uint8_t ch_a_eq[] = {0x1, 0x2, 0x3, 0x7};
	uint8_t ch_a_ctl2[] = {0x81, 0x82, 0x83, 0x84};
	uint8_t ch_b_eq[] = {0x1, 0x2, 0x3, 0x7};
	uint8_t ch_b_ctl2[] = {0x81, 0x82, 0x83, 0x84};

	int *riser_phy_addr = &xqsgii_riser_phy_addr[0];

	/* Set I2c to Slot 1 */
	i2c_write(0x77, 0, 0, &a, 1);

	for (dpmac = 0; dpmac < 8; dpmac++) {
		/* Check the PHY status */
		switch (serdes_port) {
		case 1:
			mii_bus = 0;
			dpmac_id = dpmac + 1;
			break;
		case 2:
			mii_bus = 1;
			dpmac_id = dpmac + 9;
			a = 0xb;
			i2c_write(0x76, 0, 0, &a, 1);
			break;
		}

		ret = miiphy_set_current_dev(dev[mii_bus]);
		if (ret > 0)
			goto error;

		bus = mdio_get_current_dev();
		debug("Reading from bus %s\n", bus->name);

		ret = miiphy_write(dev[mii_bus], riser_phy_addr[dpmac], 0x1f,
				   3);
		if (ret > 0)
			goto error;

		mdelay(10);
		ret = miiphy_read(dev[mii_bus], riser_phy_addr[dpmac], 0x11,
				  &value);
		if (ret > 0)
			goto error;

		mdelay(10);

		if ((value & 0xfff) == 0x401) {
			printf("DPMAC %d:PHY is ..... Configured\n", dpmac_id);
			miiphy_write(dev[mii_bus], riser_phy_addr[dpmac],
				     0x1f, 0);
			continue;
		}

		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				a = 0x18;
				i2c_write(i2c_addr[dpmac], 6, 1, &a, 1);
				a = 0x38;
				i2c_write(i2c_addr[dpmac], 4, 1, &a, 1);
				a = 0x4;
				i2c_write(i2c_addr[dpmac], 8, 1, &a, 1);

				i2c_write(i2c_addr[dpmac], 0xf, 1,
					  &ch_a_eq[i], 1);
				i2c_write(i2c_addr[dpmac], 0x11, 1,
					  &ch_a_ctl2[j], 1);

				i2c_write(i2c_addr[dpmac], 0x16, 1,
					  &ch_b_eq[i], 1);
				i2c_write(i2c_addr[dpmac], 0x18, 1,
					  &ch_b_ctl2[j], 1);

				a = 0x14;
				i2c_write(i2c_addr[dpmac], 0x23, 1, &a, 1);
				a = 0xb5;
				i2c_write(i2c_addr[dpmac], 0x2d, 1, &a, 1);
				a = 0x20;
				i2c_write(i2c_addr[dpmac], 4, 1, &a, 1);
				mdelay(100);
				ret = miiphy_read(dev[mii_bus],
						  riser_phy_addr[dpmac],
						  0x11, &value);
				if (ret > 0)
					goto error;

				mdelay(100);
				ret = miiphy_read(dev[mii_bus],
						  riser_phy_addr[dpmac],
						  0x11, &value);
				if (ret > 0)
					goto error;

				if ((value & 0xfff) == 0x401) {
					printf("DPMAC %d :PHY is configured ",
					       dpmac_id);
					printf("after setting repeater 0x%x\n",
					       value);
					i = 5;
					j = 5;
				} else {
					printf("DPMAC %d :PHY is failed to ",
					       dpmac_id);
					printf("configure the repeater 0x%x\n",
					       value);
				}
			}
		}
		miiphy_write(dev[mii_bus], riser_phy_addr[dpmac], 0x1f, 0);
	}
error:
	if (ret)
		printf("DPMAC %d ..... FAILED to configure PHY\n", dpmac_id);
	return;
}

static void qsgmii_configure_repeater(int dpmac)
{
	uint8_t a = 0xf;
	int i, j;
	int i2c_phy_addr = 0;
	int phy_addr = 0;
	int i2c_addr[] = {0x58, 0x59, 0x5a, 0x5b};

	uint8_t ch_a_eq[] = {0x1, 0x2, 0x3, 0x7};
	uint8_t ch_a_ctl2[] = {0x81, 0x82, 0x83, 0x84};
	uint8_t ch_b_eq[] = {0x1, 0x2, 0x3, 0x7};
	uint8_t ch_b_ctl2[] = {0x81, 0x82, 0x83, 0x84};

	const char *dev = "LS2080A_QDS_MDIO0";
	int ret = 0;
	unsigned short value;

	/* Set I2c to Slot 1 */
	i2c_write(0x77, 0, 0, &a, 1);

	switch (dpmac) {
	case 1:
	case 2:
	case 3:
	case 4:
		i2c_phy_addr = i2c_addr[0];
		phy_addr = 0;
		break;

	case 5:
	case 6:
	case 7:
	case 8:
		i2c_phy_addr = i2c_addr[1];
		phy_addr = 4;
		break;

	case 9:
	case 10:
	case 11:
	case 12:
		i2c_phy_addr = i2c_addr[2];
		phy_addr = 8;
		break;

	case 13:
	case 14:
	case 15:
	case 16:
		i2c_phy_addr = i2c_addr[3];
		phy_addr = 0xc;
		break;
	}

	/* Check the PHY status */
	ret = miiphy_set_current_dev(dev);
	ret = miiphy_write(dev, phy_addr, 0x1f, 3);
	mdelay(10);
	ret = miiphy_read(dev, phy_addr, 0x11, &value);
	mdelay(10);
	ret = miiphy_read(dev, phy_addr, 0x11, &value);
	mdelay(10);
	if ((value & 0xf) == 0xf) {
		printf("DPMAC %d :PHY is ..... Configured\n", dpmac);
		return;
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			a = 0x18;
			i2c_write(i2c_phy_addr, 6, 1, &a, 1);
			a = 0x38;
			i2c_write(i2c_phy_addr, 4, 1, &a, 1);
			a = 0x4;
			i2c_write(i2c_phy_addr, 8, 1, &a, 1);

			i2c_write(i2c_phy_addr, 0xf, 1, &ch_a_eq[i], 1);
			i2c_write(i2c_phy_addr, 0x11, 1, &ch_a_ctl2[j], 1);

			i2c_write(i2c_phy_addr, 0x16, 1, &ch_b_eq[i], 1);
			i2c_write(i2c_phy_addr, 0x18, 1, &ch_b_ctl2[j], 1);

			a = 0x14;
			i2c_write(i2c_phy_addr, 0x23, 1, &a, 1);
			a = 0xb5;
			i2c_write(i2c_phy_addr, 0x2d, 1, &a, 1);
			a = 0x20;
			i2c_write(i2c_phy_addr, 4, 1, &a, 1);
			mdelay(100);
			ret = miiphy_read(dev, phy_addr, 0x11, &value);
			if (ret > 0)
				goto error;
			mdelay(1);
			ret = miiphy_read(dev, phy_addr, 0x11, &value);
			if (ret > 0)
				goto error;
			mdelay(10);
			if ((value & 0xf) == 0xf) {
				printf("DPMAC %d :PHY is ..... Configured\n",
				       dpmac);
				return;
			}
		}
	}
error:
	printf("DPMAC %d :PHY ..... FAILED to configure PHY\n", dpmac);
	return;
}

static const char *ls2080a_qds_mdio_name_for_muxval(u8 muxval)
{
	return mdio_names[muxval];
}

struct mii_dev *mii_dev_for_muxval(u8 muxval)
{
	struct mii_dev *bus;
	const char *name = ls2080a_qds_mdio_name_for_muxval(muxval);

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

static void ls2080a_qds_enable_SFP_TX(u8 muxval)
{
	u8 brdcfg9;

	brdcfg9 = QIXIS_READ(brdcfg[9]);
	brdcfg9 &= ~BRDCFG9_SFPTX_MASK;
	brdcfg9 |= (muxval << BRDCFG9_SFPTX_SHIFT);
	QIXIS_WRITE(brdcfg[9], brdcfg9);
}

static void ls2080a_qds_mux_mdio(u8 muxval)
{
	u8 brdcfg4;

	if (muxval <= 5) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
		brdcfg4 |= (muxval << BRDCFG4_EMISEL_SHIFT);
		QIXIS_WRITE(brdcfg[4], brdcfg4);
	}
}

static int ls2080a_qds_mdio_read(struct mii_dev *bus, int addr,
				 int devad, int regnum)
{
	struct ls2080a_qds_mdio *priv = bus->priv;

	ls2080a_qds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int ls2080a_qds_mdio_write(struct mii_dev *bus, int addr, int devad,
				  int regnum, u16 value)
{
	struct ls2080a_qds_mdio *priv = bus->priv;

	ls2080a_qds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int ls2080a_qds_mdio_reset(struct mii_dev *bus)
{
	struct ls2080a_qds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int ls2080a_qds_mdio_init(char *realbusname, u8 muxval)
{
	struct ls2080a_qds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate ls2080a_qds MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate ls2080a_qds private data\n");
		free(bus);
		return -1;
	}

	bus->read = ls2080a_qds_mdio_read;
	bus->write = ls2080a_qds_mdio_write;
	bus->reset = ls2080a_qds_mdio_reset;
	strcpy(bus->name, ls2080a_qds_mdio_name_for_muxval(muxval));

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

/*
 * Initialize the dpmac_info array.
 *
 */
static void initialize_dpmac_to_slot(void)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;
	int serdes2_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	char *env_hwconfig;
	env_hwconfig = env_get("hwconfig");

	switch (serdes1_prtcl) {
	case 0x07:
	case 0x09:
	case 0x33:
		printf("qds: WRIOP: Supported SerDes1 Protocol 0x%02x\n",
		       serdes1_prtcl);
		lane_to_slot_fsm1[0] = EMI1_SLOT1;
		lane_to_slot_fsm1[1] = EMI1_SLOT1;
		lane_to_slot_fsm1[2] = EMI1_SLOT1;
		lane_to_slot_fsm1[3] = EMI1_SLOT1;
		if (hwconfig_f("xqsgmii", env_hwconfig)) {
			lane_to_slot_fsm1[4] = EMI1_SLOT1;
			lane_to_slot_fsm1[5] = EMI1_SLOT1;
			lane_to_slot_fsm1[6] = EMI1_SLOT1;
			lane_to_slot_fsm1[7] = EMI1_SLOT1;
		} else {
			lane_to_slot_fsm1[4] = EMI1_SLOT2;
			lane_to_slot_fsm1[5] = EMI1_SLOT2;
			lane_to_slot_fsm1[6] = EMI1_SLOT2;
			lane_to_slot_fsm1[7] = EMI1_SLOT2;
		}
		break;

	case 0x39:
		printf("qds: WRIOP: Supported SerDes1 Protocol 0x%02x\n",
		       serdes1_prtcl);
		if (hwconfig_f("xqsgmii", env_hwconfig)) {
			lane_to_slot_fsm1[0] = EMI1_SLOT3;
			lane_to_slot_fsm1[1] = EMI1_SLOT3;
			lane_to_slot_fsm1[2] = EMI1_SLOT3;
			lane_to_slot_fsm1[3] = EMI_NONE;
		} else {
			lane_to_slot_fsm1[0] = EMI_NONE;
			lane_to_slot_fsm1[1] = EMI_NONE;
			lane_to_slot_fsm1[2] = EMI_NONE;
			lane_to_slot_fsm1[3] = EMI_NONE;
		}
		lane_to_slot_fsm1[4] = EMI1_SLOT3;
		lane_to_slot_fsm1[5] = EMI1_SLOT3;
		lane_to_slot_fsm1[6] = EMI1_SLOT3;
		lane_to_slot_fsm1[7] = EMI_NONE;
		break;

	case 0x4D:
		printf("qds: WRIOP: Supported SerDes1 Protocol 0x%02x\n",
		       serdes1_prtcl);
		if (hwconfig_f("xqsgmii", env_hwconfig)) {
			lane_to_slot_fsm1[0] = EMI1_SLOT3;
			lane_to_slot_fsm1[1] = EMI1_SLOT3;
			lane_to_slot_fsm1[2] = EMI_NONE;
			lane_to_slot_fsm1[3] = EMI_NONE;
		} else {
			lane_to_slot_fsm1[0] = EMI_NONE;
			lane_to_slot_fsm1[1] = EMI_NONE;
			lane_to_slot_fsm1[2] = EMI_NONE;
			lane_to_slot_fsm1[3] = EMI_NONE;
		}
		lane_to_slot_fsm1[4] = EMI1_SLOT3;
		lane_to_slot_fsm1[5] = EMI1_SLOT3;
		lane_to_slot_fsm1[6] = EMI_NONE;
		lane_to_slot_fsm1[7] = EMI_NONE;
		break;

	case 0x2A:
	case 0x4B:
	case 0x4C:
		printf("qds: WRIOP: Supported SerDes1 Protocol 0x%02x\n",
		       serdes1_prtcl);
		break;
	default:
		printf("%s qds: WRIOP: Unsupported SerDes1 Protocol 0x%02x\n",
		       __func__, serdes1_prtcl);
		break;
	}

	switch (serdes2_prtcl) {
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x49:
		printf("qds: WRIOP: Supported SerDes2 Protocol 0x%02x\n",
		       serdes2_prtcl);
		lane_to_slot_fsm2[0] = EMI1_SLOT4;
		lane_to_slot_fsm2[1] = EMI1_SLOT4;
		lane_to_slot_fsm2[2] = EMI1_SLOT4;
		lane_to_slot_fsm2[3] = EMI1_SLOT4;

		if (hwconfig_f("xqsgmii", env_hwconfig)) {
			lane_to_slot_fsm2[4] = EMI1_SLOT4;
			lane_to_slot_fsm2[5] = EMI1_SLOT4;
			lane_to_slot_fsm2[6] = EMI1_SLOT4;
			lane_to_slot_fsm2[7] = EMI1_SLOT4;
		} else {
			/* No MDIO physical connection */
			lane_to_slot_fsm2[4] = EMI1_SLOT6;
			lane_to_slot_fsm2[5] = EMI1_SLOT6;
			lane_to_slot_fsm2[6] = EMI1_SLOT6;
			lane_to_slot_fsm2[7] = EMI1_SLOT6;
		}
		break;

	case 0x47:
		printf("qds: WRIOP: Supported SerDes2 Protocol 0x%02x\n",
		       serdes2_prtcl);
		lane_to_slot_fsm2[0] = EMI_NONE;
		lane_to_slot_fsm2[1] = EMI1_SLOT5;
		lane_to_slot_fsm2[2] = EMI1_SLOT5;
		lane_to_slot_fsm2[3] = EMI1_SLOT5;

		if (hwconfig_f("xqsgmii", env_hwconfig)) {
			lane_to_slot_fsm2[4] = EMI_NONE;
			lane_to_slot_fsm2[5] = EMI1_SLOT5;
			lane_to_slot_fsm2[6] = EMI1_SLOT5;
			lane_to_slot_fsm2[7] = EMI1_SLOT5;
		}
		break;

	case 0x57:
		printf("qds: WRIOP: Supported SerDes2 Protocol 0x%02x\n",
		       serdes2_prtcl);
		if (hwconfig_f("xqsgmii", env_hwconfig)) {
			lane_to_slot_fsm2[0] = EMI_NONE;
			lane_to_slot_fsm2[1] = EMI_NONE;
			lane_to_slot_fsm2[2] = EMI_NONE;
			lane_to_slot_fsm2[3] = EMI_NONE;
		}
		lane_to_slot_fsm2[4] = EMI_NONE;
		lane_to_slot_fsm2[5] = EMI_NONE;
		lane_to_slot_fsm2[6] = EMI1_SLOT5;
		lane_to_slot_fsm2[7] = EMI1_SLOT5;
		break;

	default:
		printf(" %s qds: WRIOP: Unsupported SerDes2 Protocol 0x%02x\n",
		       __func__ , serdes2_prtcl);
		break;
	}
}

void ls2080a_handle_phy_interface_sgmii(int dpmac_id)
{
	int lane, slot;
	struct mii_dev *bus;
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;
	int serdes2_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	int *riser_phy_addr;
	char *env_hwconfig = env_get("hwconfig");

	if (hwconfig_f("xqsgmii", env_hwconfig))
		riser_phy_addr = &xqsgii_riser_phy_addr[0];
	else
		riser_phy_addr = &sgmii_riser_phy_addr[0];

	if (dpmac_id > WRIOP1_DPMAC9)
		goto serdes2;

	switch (serdes1_prtcl) {
	case 0x07:
	case 0x39:
	case 0x4D:
		lane = serdes_get_first_lane(FSL_SRDS_1, SGMII1 + dpmac_id - 1);

		slot = lane_to_slot_fsm1[lane];

		switch (++slot) {
		case 1:
			/* Slot housing a SGMII riser card? */
			wriop_set_phy_address(dpmac_id, 0,
					      riser_phy_addr[dpmac_id - 1]);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT1;
			bus = mii_dev_for_muxval(EMI1_SLOT1);
			wriop_set_mdio(dpmac_id, bus);
			break;
		case 2:
			/* Slot housing a SGMII riser card? */
			wriop_set_phy_address(dpmac_id, 0,
					      riser_phy_addr[dpmac_id - 1]);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT2;
			bus = mii_dev_for_muxval(EMI1_SLOT2);
			wriop_set_mdio(dpmac_id, bus);
			break;
		case 3:
			if (slot == EMI_NONE)
				return;
			if (serdes1_prtcl == 0x39) {
				wriop_set_phy_address(dpmac_id, 0,
					riser_phy_addr[dpmac_id - 2]);
				if (dpmac_id >= 6 && hwconfig_f("xqsgmii",
								env_hwconfig))
					wriop_set_phy_address(dpmac_id, 0,
						riser_phy_addr[dpmac_id - 3]);
			} else {
				wriop_set_phy_address(dpmac_id, 0,
					riser_phy_addr[dpmac_id - 2]);
				if (dpmac_id >= 7 && hwconfig_f("xqsgmii",
								env_hwconfig))
					wriop_set_phy_address(dpmac_id, 0,
						riser_phy_addr[dpmac_id - 3]);
			}
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT3;
			bus = mii_dev_for_muxval(EMI1_SLOT3);
			wriop_set_mdio(dpmac_id, bus);
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		}
	break;
	default:
		printf("%s qds: WRIOP: Unsupported SerDes1 Protocol 0x%02x\n",
		       __func__ , serdes1_prtcl);
	break;
	}

serdes2:
	switch (serdes2_prtcl) {
	case 0x07:
	case 0x08:
	case 0x49:
	case 0x47:
	case 0x57:
		lane = serdes_get_first_lane(FSL_SRDS_2, SGMII9 +
							(dpmac_id - 9));
		slot = lane_to_slot_fsm2[lane];

		switch (++slot) {
		case 1:
			break;
		case 3:
			break;
		case 4:
			/* Slot housing a SGMII riser card? */
			wriop_set_phy_address(dpmac_id, 0,
					      riser_phy_addr[dpmac_id - 9]);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT4;
			bus = mii_dev_for_muxval(EMI1_SLOT4);
			wriop_set_mdio(dpmac_id, bus);
		break;
		case 5:
			if (slot == EMI_NONE)
				return;
			if (serdes2_prtcl == 0x47) {
				wriop_set_phy_address(dpmac_id, 0,
					      riser_phy_addr[dpmac_id - 10]);
				if (dpmac_id >= 14 && hwconfig_f("xqsgmii",
								 env_hwconfig))
					wriop_set_phy_address(dpmac_id, 0,
						riser_phy_addr[dpmac_id - 11]);
			} else {
				wriop_set_phy_address(dpmac_id, 0,
					riser_phy_addr[dpmac_id - 11]);
			}
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT5;
			bus = mii_dev_for_muxval(EMI1_SLOT5);
			wriop_set_mdio(dpmac_id, bus);
			break;
		case 6:
			/* Slot housing a SGMII riser card? */
			wriop_set_phy_address(dpmac_id, 0,
					      riser_phy_addr[dpmac_id - 13]);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT6;
			bus = mii_dev_for_muxval(EMI1_SLOT6);
			wriop_set_mdio(dpmac_id, bus);
		break;
	}
	break;
	default:
		printf("%s qds: WRIOP: Unsupported SerDes2 Protocol 0x%02x\n",
		       __func__, serdes2_prtcl);
	break;
	}
}

void ls2080a_handle_phy_interface_qsgmii(int dpmac_id)
{
	int lane = 0, slot;
	struct mii_dev *bus;
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	switch (serdes1_prtcl) {
	case 0x33:
		switch (dpmac_id) {
		case 1:
		case 2:
		case 3:
		case 4:
			lane = serdes_get_first_lane(FSL_SRDS_1, QSGMII_A);
		break;
		case 5:
		case 6:
		case 7:
		case 8:
			lane = serdes_get_first_lane(FSL_SRDS_1, QSGMII_B);
		break;
		case 9:
		case 10:
		case 11:
		case 12:
			lane = serdes_get_first_lane(FSL_SRDS_1, QSGMII_C);
		break;
		case 13:
		case 14:
		case 15:
		case 16:
			lane = serdes_get_first_lane(FSL_SRDS_1, QSGMII_D);
		break;
	}

		slot = lane_to_slot_fsm1[lane];

		switch (++slot) {
		case 1:
			/* Slot housing a QSGMII riser card? */
			wriop_set_phy_address(dpmac_id, 0, dpmac_id - 1);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT1;
			bus = mii_dev_for_muxval(EMI1_SLOT1);
			wriop_set_mdio(dpmac_id, bus);
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
		break;
		case 6:
			break;
	}
	break;
	default:
		printf("qds: WRIOP: Unsupported SerDes Protocol 0x%02x\n",
		       serdes1_prtcl);
	break;
	}

	qsgmii_configure_repeater(dpmac_id);
}

void ls2080a_handle_phy_interface_xsgmii(int i)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	switch (serdes1_prtcl) {
	case 0x2A:
	case 0x4B:
	case 0x4C:
		/*
		 * XFI does not need a PHY to work, but to avoid U-Boot use
		 * default PHY address which is zero to a MAC when it found
		 * a MAC has no PHY address, we give a PHY address to XFI
		 * MAC, and should not use a real XAUI PHY address, since
		 * MDIO can access it successfully, and then MDIO thinks
		 * the XAUI card is used for the XFI MAC, which will cause
		 * error.
		 */
		wriop_set_phy_address(i, 0, i + 4);
		ls2080a_qds_enable_SFP_TX(SFP_TX);

		break;
	default:
		printf("qds: WRIOP: Unsupported SerDes Protocol 0x%02x\n",
		       serdes1_prtcl);
		break;
	}
}
#endif

int board_eth_init(bd_t *bis)
{
	int error;
#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;
	int serdes2_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	struct memac_mdio_info *memac_mdio0_info;
	struct memac_mdio_info *memac_mdio1_info;
	unsigned int i;
	char *env_hwconfig;

	env_hwconfig = env_get("hwconfig");

	initialize_dpmac_to_slot();

	memac_mdio0_info = (struct memac_mdio_info *)malloc(
					sizeof(struct memac_mdio_info));
	memac_mdio0_info->regs =
		(struct memac_mdio_controller *)
					CONFIG_SYS_FSL_WRIOP1_MDIO1;
	memac_mdio0_info->name = DEFAULT_WRIOP_MDIO1_NAME;

	/* Register the real MDIO1 bus */
	fm_memac_mdio_init(bis, memac_mdio0_info);

	memac_mdio1_info = (struct memac_mdio_info *)malloc(
					sizeof(struct memac_mdio_info));
	memac_mdio1_info->regs =
		(struct memac_mdio_controller *)
					CONFIG_SYS_FSL_WRIOP1_MDIO2;
	memac_mdio1_info->name = DEFAULT_WRIOP_MDIO2_NAME;

	/* Register the real MDIO2 bus */
	fm_memac_mdio_init(bis, memac_mdio1_info);

	/* Register the muxing front-ends to the MDIO buses */
	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT1);
	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT2);
	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT3);
	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT4);
	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT5);
	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT6);

	ls2080a_qds_mdio_init(DEFAULT_WRIOP_MDIO2_NAME, EMI2);

	for (i = WRIOP1_DPMAC1; i < NUM_WRIOP_PORTS; i++) {
		switch (wriop_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_QSGMII:
			ls2080a_handle_phy_interface_qsgmii(i);
			break;
		case PHY_INTERFACE_MODE_SGMII:
			ls2080a_handle_phy_interface_sgmii(i);
			break;
		case PHY_INTERFACE_MODE_XGMII:
			ls2080a_handle_phy_interface_xsgmii(i);
			break;
		default:
			break;

		if (i == 16)
			i = NUM_WRIOP_PORTS;
		}
	}

	error = cpu_eth_init(bis);

	if (hwconfig_f("xqsgmii", env_hwconfig)) {
		if (serdes1_prtcl == 0x7)
			sgmii_configure_repeater(1);
		if (serdes2_prtcl == 0x7 || serdes2_prtcl == 0x8 ||
		    serdes2_prtcl == 0x49)
			sgmii_configure_repeater(2);
	}
#endif
	error = pci_eth_init(bis);
	return error;
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */
