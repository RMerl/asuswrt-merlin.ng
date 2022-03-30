// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2005,2010-2011 Freescale Semiconductor, Inc.
 *
 * Author: Shlomi Gridish
 *
 * Description: UCC GETH Driver -- PHY handling
 *		Driver for UEC on QE
 *		Based on 8260_io/fcc_enet.c
 */

#include <common.h>
#include <net.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/immap_qe.h>
#include <asm/io.h>
#include "uccf.h"
#include "uec.h"
#include "uec_phy.h"
#include "miiphy.h"
#include <fsl_qe.h>
#include <phy.h>

#define ugphy_printk(format, arg...)  \
	printf(format "\n", ## arg)

#define ugphy_dbg(format, arg...)	     \
	ugphy_printk(format , ## arg)
#define ugphy_err(format, arg...)	     \
	ugphy_printk(format , ## arg)
#define ugphy_info(format, arg...)	     \
	ugphy_printk(format , ## arg)
#define ugphy_warn(format, arg...)	     \
	ugphy_printk(format , ## arg)

#ifdef UEC_VERBOSE_DEBUG
#define ugphy_vdbg ugphy_dbg
#else
#define ugphy_vdbg(ugeth, fmt, args...) do { } while (0)
#endif /* UEC_VERBOSE_DEBUG */

/*--------------------------------------------------------------------+
 * Fixed PHY (PHY-less) support for Ethernet Ports.
 *
 * Copied from arch/powerpc/cpu/ppc4xx/4xx_enet.c
 *--------------------------------------------------------------------*/

/*
 * Some boards do not have a PHY for each ethernet port. These ports are known
 * as Fixed PHY (or PHY-less) ports. For such ports, set the appropriate
 * CONFIG_SYS_UECx_PHY_ADDR equal to CONFIG_FIXED_PHY_ADDR (an unused address)
 * When the drver tries to identify the PHYs, CONFIG_FIXED_PHY will be returned
 * and the driver will search CONFIG_SYS_FIXED_PHY_PORTS to find what network
 * speed and duplex should be for the port.
 *
 * Example board header configuration file:
 *     #define CONFIG_FIXED_PHY   0xFFFFFFFF
 *     #define CONFIG_SYS_FIXED_PHY_ADDR 0x1E (pick an unused phy address)
 *
 *     #define CONFIG_SYS_UEC1_PHY_ADDR CONFIG_SYS_FIXED_PHY_ADDR
 *     #define CONFIG_SYS_UEC2_PHY_ADDR 0x02
 *     #define CONFIG_SYS_UEC3_PHY_ADDR CONFIG_SYS_FIXED_PHY_ADDR
 *     #define CONFIG_SYS_UEC4_PHY_ADDR 0x04
 *
 *     #define CONFIG_SYS_FIXED_PHY_PORT(name,speed,duplex) \
 *                 {name, speed, duplex},
 *
 *     #define CONFIG_SYS_FIXED_PHY_PORTS \
 *                 CONFIG_SYS_FIXED_PHY_PORT("UEC0",SPEED_100,DUPLEX_FULL) \
 *                 CONFIG_SYS_FIXED_PHY_PORT("UEC2",SPEED_100,DUPLEX_HALF)
 */

#ifndef CONFIG_FIXED_PHY
#define CONFIG_FIXED_PHY	0xFFFFFFFF /* Fixed PHY (PHY-less) */
#endif

#ifndef CONFIG_SYS_FIXED_PHY_PORTS
#define CONFIG_SYS_FIXED_PHY_PORTS	/* default is an empty array */
#endif

struct fixed_phy_port {
	char name[16];	/* ethernet port name */
	unsigned int speed;	/* specified speed 10,100 or 1000 */
	unsigned int duplex;	/* specified duplex FULL or HALF */
};

static const struct fixed_phy_port fixed_phy_port[] = {
	CONFIG_SYS_FIXED_PHY_PORTS /* defined in board configuration file */
};

/*--------------------------------------------------------------------+
 * BitBang MII support for ethernet ports
 *
 * Based from MPC8560ADS implementation
 *--------------------------------------------------------------------*/
/*
 * Example board header file to define bitbang ethernet ports:
 *
 * #define CONFIG_SYS_BITBANG_PHY_PORT(name) name,
 * #define CONFIG_SYS_BITBANG_PHY_PORTS CONFIG_SYS_BITBANG_PHY_PORT("UEC0")
*/
#ifndef CONFIG_SYS_BITBANG_PHY_PORTS
#define CONFIG_SYS_BITBANG_PHY_PORTS	/* default is an empty array */
#endif

#if defined(CONFIG_BITBANGMII)
static const char *bitbang_phy_port[] = {
	CONFIG_SYS_BITBANG_PHY_PORTS /* defined in board configuration file */
};
#endif /* CONFIG_BITBANGMII */

static void config_genmii_advert (struct uec_mii_info *mii_info);
static void genmii_setup_forced (struct uec_mii_info *mii_info);
static void genmii_restart_aneg (struct uec_mii_info *mii_info);
static int gbit_config_aneg (struct uec_mii_info *mii_info);
static int genmii_config_aneg (struct uec_mii_info *mii_info);
static int genmii_update_link (struct uec_mii_info *mii_info);
static int genmii_read_status (struct uec_mii_info *mii_info);
u16 uec_phy_read(struct uec_mii_info *mii_info, u16 regnum);
void uec_phy_write(struct uec_mii_info *mii_info, u16 regnum, u16 val);

/* Write value to the PHY for this device to the register at regnum, */
/* waiting until the write is done before it returns.  All PHY */
/* configuration has to be done through the TSEC1 MIIM regs */
void uec_write_phy_reg (struct eth_device *dev, int mii_id, int regnum, int value)
{
	uec_private_t *ugeth = (uec_private_t *) dev->priv;
	uec_mii_t *ug_regs;
	enet_tbi_mii_reg_e mii_reg = (enet_tbi_mii_reg_e) regnum;
	u32 tmp_reg;


#if defined(CONFIG_BITBANGMII)
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(bitbang_phy_port); i++) {
		if (strncmp(dev->name, bitbang_phy_port[i],
			sizeof(dev->name)) == 0) {
			(void)bb_miiphy_write(NULL, mii_id, regnum, value);
			return;
		}
	}
#endif /* CONFIG_BITBANGMII */

	ug_regs = ugeth->uec_mii_regs;

	/* Stop the MII management read cycle */
	out_be32 (&ug_regs->miimcom, 0);
	/* Setting up the MII Mangement Address Register */
	tmp_reg = ((u32) mii_id << MIIMADD_PHY_ADDRESS_SHIFT) | mii_reg;
	out_be32 (&ug_regs->miimadd, tmp_reg);

	/* Setting up the MII Mangement Control Register with the value */
	out_be32 (&ug_regs->miimcon, (u32) value);
	sync();

	/* Wait till MII management write is complete */
	while ((in_be32 (&ug_regs->miimind)) & MIIMIND_BUSY);
}

/* Reads from register regnum in the PHY for device dev, */
/* returning the value.  Clears miimcom first.  All PHY */
/* configuration has to be done through the TSEC1 MIIM regs */
int uec_read_phy_reg (struct eth_device *dev, int mii_id, int regnum)
{
	uec_private_t *ugeth = (uec_private_t *) dev->priv;
	uec_mii_t *ug_regs;
	enet_tbi_mii_reg_e mii_reg = (enet_tbi_mii_reg_e) regnum;
	u32 tmp_reg;
	u16 value;


#if defined(CONFIG_BITBANGMII)
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(bitbang_phy_port); i++) {
		if (strncmp(dev->name, bitbang_phy_port[i],
			sizeof(dev->name)) == 0) {
			(void)bb_miiphy_read(NULL, mii_id, regnum, &value);
			return (value);
		}
	}
#endif /* CONFIG_BITBANGMII */

	ug_regs = ugeth->uec_mii_regs;

	/* Setting up the MII Mangement Address Register */
	tmp_reg = ((u32) mii_id << MIIMADD_PHY_ADDRESS_SHIFT) | mii_reg;
	out_be32 (&ug_regs->miimadd, tmp_reg);

	/* clear MII management command cycle */
	out_be32 (&ug_regs->miimcom, 0);
	sync();

	/* Perform an MII management read cycle */
	out_be32 (&ug_regs->miimcom, MIIMCOM_READ_CYCLE);

	/* Wait till MII management write is complete */
	while ((in_be32 (&ug_regs->miimind)) &
	       (MIIMIND_NOT_VALID | MIIMIND_BUSY));

	/* Read MII management status  */
	value = (u16) in_be32 (&ug_regs->miimstat);
	if (value == 0xffff)
		ugphy_vdbg
			("read wrong value : mii_id %d,mii_reg %d, base %08x",
			 mii_id, mii_reg, (u32) & (ug_regs->miimcfg));

	return (value);
}

void mii_clear_phy_interrupt (struct uec_mii_info *mii_info)
{
	if (mii_info->phyinfo->ack_interrupt)
		mii_info->phyinfo->ack_interrupt (mii_info);
}

void mii_configure_phy_interrupt (struct uec_mii_info *mii_info,
				  u32 interrupts)
{
	mii_info->interrupts = interrupts;
	if (mii_info->phyinfo->config_intr)
		mii_info->phyinfo->config_intr (mii_info);
}

/* Writes MII_ADVERTISE with the appropriate values, after
 * sanitizing advertise to make sure only supported features
 * are advertised
 */
static void config_genmii_advert (struct uec_mii_info *mii_info)
{
	u32 advertise;
	u16 adv;

	/* Only allow advertising what this PHY supports */
	mii_info->advertising &= mii_info->phyinfo->features;
	advertise = mii_info->advertising;

	/* Setup standard advertisement */
	adv = uec_phy_read(mii_info, MII_ADVERTISE);
	adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4);
	if (advertise & ADVERTISED_10baseT_Half)
		adv |= ADVERTISE_10HALF;
	if (advertise & ADVERTISED_10baseT_Full)
		adv |= ADVERTISE_10FULL;
	if (advertise & ADVERTISED_100baseT_Half)
		adv |= ADVERTISE_100HALF;
	if (advertise & ADVERTISED_100baseT_Full)
		adv |= ADVERTISE_100FULL;
	uec_phy_write(mii_info, MII_ADVERTISE, adv);
}

static void genmii_setup_forced (struct uec_mii_info *mii_info)
{
	u16 ctrl;
	u32 features = mii_info->phyinfo->features;

	ctrl = uec_phy_read(mii_info, MII_BMCR);

	ctrl &= ~(BMCR_FULLDPLX | BMCR_SPEED100 |
		  BMCR_SPEED1000 | BMCR_ANENABLE);
	ctrl |= BMCR_RESET;

	switch (mii_info->speed) {
	case SPEED_1000:
		if (features & (SUPPORTED_1000baseT_Half
				| SUPPORTED_1000baseT_Full)) {
			ctrl |= BMCR_SPEED1000;
			break;
		}
		mii_info->speed = SPEED_100;
	case SPEED_100:
		if (features & (SUPPORTED_100baseT_Half
				| SUPPORTED_100baseT_Full)) {
			ctrl |= BMCR_SPEED100;
			break;
		}
		mii_info->speed = SPEED_10;
	case SPEED_10:
		if (features & (SUPPORTED_10baseT_Half
				| SUPPORTED_10baseT_Full))
			break;
	default:		/* Unsupported speed! */
		ugphy_err ("%s: Bad speed!", mii_info->dev->name);
		break;
	}

	uec_phy_write(mii_info, MII_BMCR, ctrl);
}

/* Enable and Restart Autonegotiation */
static void genmii_restart_aneg (struct uec_mii_info *mii_info)
{
	u16 ctl;

	ctl = uec_phy_read(mii_info, MII_BMCR);
	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);
	uec_phy_write(mii_info, MII_BMCR, ctl);
}

static int gbit_config_aneg (struct uec_mii_info *mii_info)
{
	u16 adv;
	u32 advertise;

	if (mii_info->autoneg) {
		/* Configure the ADVERTISE register */
		config_genmii_advert (mii_info);
		advertise = mii_info->advertising;

		adv = uec_phy_read(mii_info, MII_CTRL1000);
		adv &= ~(ADVERTISE_1000FULL |
			 ADVERTISE_1000HALF);
		if (advertise & SUPPORTED_1000baseT_Half)
			adv |= ADVERTISE_1000HALF;
		if (advertise & SUPPORTED_1000baseT_Full)
			adv |= ADVERTISE_1000FULL;
		uec_phy_write(mii_info, MII_CTRL1000, adv);

		/* Start/Restart aneg */
		genmii_restart_aneg (mii_info);
	} else
		genmii_setup_forced (mii_info);

	return 0;
}

static int marvell_config_aneg (struct uec_mii_info *mii_info)
{
	/* The Marvell PHY has an errata which requires
	 * that certain registers get written in order
	 * to restart autonegotiation */
	uec_phy_write(mii_info, MII_BMCR, BMCR_RESET);

	uec_phy_write(mii_info, 0x1d, 0x1f);
	uec_phy_write(mii_info, 0x1e, 0x200c);
	uec_phy_write(mii_info, 0x1d, 0x5);
	uec_phy_write(mii_info, 0x1e, 0);
	uec_phy_write(mii_info, 0x1e, 0x100);

	gbit_config_aneg (mii_info);

	return 0;
}

static int genmii_config_aneg (struct uec_mii_info *mii_info)
{
	if (mii_info->autoneg) {
		/* Speed up the common case, if link is already up, speed and
		   duplex match, skip auto neg as it already matches */
		if (!genmii_read_status(mii_info) && mii_info->link)
			if (mii_info->duplex == DUPLEX_FULL &&
			    mii_info->speed == SPEED_100)
				if (mii_info->advertising &
				    ADVERTISED_100baseT_Full)
					return 0;

		config_genmii_advert (mii_info);
		genmii_restart_aneg (mii_info);
	} else
		genmii_setup_forced (mii_info);

	return 0;
}

static int genmii_update_link (struct uec_mii_info *mii_info)
{
	u16 status;

	/* Status is read once to clear old link state */
	uec_phy_read(mii_info, MII_BMSR);

	/*
	 * Wait if the link is up, and autonegotiation is in progress
	 * (ie - we're capable and it's not done)
	 */
	status = uec_phy_read(mii_info, MII_BMSR);
	if ((status & BMSR_LSTATUS) && (status & BMSR_ANEGCAPABLE)
	    && !(status & BMSR_ANEGCOMPLETE)) {
		int i = 0;

		while (!(status & BMSR_ANEGCOMPLETE)) {
			/*
			 * Timeout reached ?
			 */
			if (i > UGETH_AN_TIMEOUT) {
				mii_info->link = 0;
				return 0;
			}

			i++;
			udelay(1000);	/* 1 ms */
			status = uec_phy_read(mii_info, MII_BMSR);
		}
		mii_info->link = 1;
	} else {
		if (status & BMSR_LSTATUS)
			mii_info->link = 1;
		else
			mii_info->link = 0;
	}

	return 0;
}

static int genmii_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there
	 * was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;

	if (mii_info->autoneg) {
		status = uec_phy_read(mii_info, MII_STAT1000);

		if (status & (LPA_1000FULL | LPA_1000HALF)) {
			mii_info->speed = SPEED_1000;
			if (status & LPA_1000FULL)
				mii_info->duplex = DUPLEX_FULL;
			else
				mii_info->duplex = DUPLEX_HALF;
		} else {
			status = uec_phy_read(mii_info, MII_LPA);

			if (status & (LPA_10FULL | LPA_100FULL))
				mii_info->duplex = DUPLEX_FULL;
			else
				mii_info->duplex = DUPLEX_HALF;
			if (status & (LPA_100FULL | LPA_100HALF))
				mii_info->speed = SPEED_100;
			else
				mii_info->speed = SPEED_10;
		}
		mii_info->pause = 0;
	}
	/* On non-aneg, we assume what we put in BMCR is the speed,
	 * though magic-aneg shouldn't prevent this case from occurring
	 */

	return 0;
}

static int bcm_init(struct uec_mii_info *mii_info)
{
	struct eth_device *edev = mii_info->dev;
	uec_private_t *uec = edev->priv;

	gbit_config_aneg(mii_info);

	if ((uec->uec_info->enet_interface_type ==
				PHY_INTERFACE_MODE_RGMII_RXID) &&
			(uec->uec_info->speed == SPEED_1000)) {
		u16 val;
		int cnt = 50;

		/* Wait for aneg to complete. */
		do
			val = uec_phy_read(mii_info, MII_BMSR);
		while (--cnt && !(val & BMSR_ANEGCOMPLETE));

		/* Set RDX clk delay. */
		uec_phy_write(mii_info, 0x18, 0x7 | (7 << 12));

		val = uec_phy_read(mii_info, 0x18);
		/* Set RDX-RXC skew. */
		val |= (1 << 8);
		val |= (7 | (7 << 12));
		/* Write bits 14:0. */
		val |= (1 << 15);
		uec_phy_write(mii_info, 0x18, val);
	}

	 return 0;
}

static int uec_marvell_init(struct uec_mii_info *mii_info)
{
	struct eth_device *edev = mii_info->dev;
	uec_private_t *uec = edev->priv;
	phy_interface_t iface = uec->uec_info->enet_interface_type;
	int	speed = uec->uec_info->speed;

	if ((speed == SPEED_1000) &&
	   (iface == PHY_INTERFACE_MODE_RGMII_ID ||
	    iface == PHY_INTERFACE_MODE_RGMII_RXID ||
	    iface == PHY_INTERFACE_MODE_RGMII_TXID)) {
		int temp;

		temp = uec_phy_read(mii_info, MII_M1111_PHY_EXT_CR);
		if (iface == PHY_INTERFACE_MODE_RGMII_ID) {
			temp |= MII_M1111_RX_DELAY | MII_M1111_TX_DELAY;
		} else if (iface == PHY_INTERFACE_MODE_RGMII_RXID) {
			temp &= ~MII_M1111_TX_DELAY;
			temp |= MII_M1111_RX_DELAY;
		} else if (iface == PHY_INTERFACE_MODE_RGMII_TXID) {
			temp &= ~MII_M1111_RX_DELAY;
			temp |= MII_M1111_TX_DELAY;
		}
		uec_phy_write(mii_info, MII_M1111_PHY_EXT_CR, temp);

		temp = uec_phy_read(mii_info, MII_M1111_PHY_EXT_SR);
		temp &= ~MII_M1111_HWCFG_MODE_MASK;
		temp |= MII_M1111_HWCFG_MODE_RGMII;
		uec_phy_write(mii_info, MII_M1111_PHY_EXT_SR, temp);

		uec_phy_write(mii_info, MII_BMCR, BMCR_RESET);
	}

	return 0;
}

static int marvell_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there
	 * was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;

	/* If the link is up, read the speed and duplex */
	/* If we aren't autonegotiating, assume speeds
	 * are as set */
	if (mii_info->autoneg && mii_info->link) {
		int speed;

		status = uec_phy_read(mii_info, MII_M1011_PHY_SPEC_STATUS);

		/* Get the duplexity */
		if (status & MII_M1011_PHY_SPEC_STATUS_FULLDUPLEX)
			mii_info->duplex = DUPLEX_FULL;
		else
			mii_info->duplex = DUPLEX_HALF;

		/* Get the speed */
		speed = status & MII_M1011_PHY_SPEC_STATUS_SPD_MASK;
		switch (speed) {
		case MII_M1011_PHY_SPEC_STATUS_1000:
			mii_info->speed = SPEED_1000;
			break;
		case MII_M1011_PHY_SPEC_STATUS_100:
			mii_info->speed = SPEED_100;
			break;
		default:
			mii_info->speed = SPEED_10;
			break;
		}
		mii_info->pause = 0;
	}

	return 0;
}

static int marvell_ack_interrupt (struct uec_mii_info *mii_info)
{
	/* Clear the interrupts by reading the reg */
	uec_phy_read(mii_info, MII_M1011_IEVENT);

	return 0;
}

static int marvell_config_intr (struct uec_mii_info *mii_info)
{
	if (mii_info->interrupts == MII_INTERRUPT_ENABLED)
		uec_phy_write(mii_info, MII_M1011_IMASK, MII_M1011_IMASK_INIT);
	else
		uec_phy_write(mii_info, MII_M1011_IMASK,
				MII_M1011_IMASK_CLEAR);

	return 0;
}

static int dm9161_init (struct uec_mii_info *mii_info)
{
	/* Reset the PHY */
	uec_phy_write(mii_info, MII_BMCR, uec_phy_read(mii_info, MII_BMCR) |
		   BMCR_RESET);
	/* PHY and MAC connect */
	uec_phy_write(mii_info, MII_BMCR, uec_phy_read(mii_info, MII_BMCR) &
		   ~BMCR_ISOLATE);

	uec_phy_write(mii_info, MII_DM9161_SCR, MII_DM9161_SCR_INIT);

	config_genmii_advert (mii_info);
	/* Start/restart aneg */
	genmii_config_aneg (mii_info);

	return 0;
}

static int dm9161_config_aneg (struct uec_mii_info *mii_info)
{
	return 0;
}

static int dm9161_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;
	/* If the link is up, read the speed and duplex
	   If we aren't autonegotiating assume speeds are as set */
	if (mii_info->autoneg && mii_info->link) {
		status = uec_phy_read(mii_info, MII_DM9161_SCSR);
		if (status & (MII_DM9161_SCSR_100F | MII_DM9161_SCSR_100H))
			mii_info->speed = SPEED_100;
		else
			mii_info->speed = SPEED_10;

		if (status & (MII_DM9161_SCSR_100F | MII_DM9161_SCSR_10F))
			mii_info->duplex = DUPLEX_FULL;
		else
			mii_info->duplex = DUPLEX_HALF;
	}

	return 0;
}

static int dm9161_ack_interrupt (struct uec_mii_info *mii_info)
{
	/* Clear the interrupt by reading the reg */
	uec_phy_read(mii_info, MII_DM9161_INTR);

	return 0;
}

static int dm9161_config_intr (struct uec_mii_info *mii_info)
{
	if (mii_info->interrupts == MII_INTERRUPT_ENABLED)
		uec_phy_write(mii_info, MII_DM9161_INTR, MII_DM9161_INTR_INIT);
	else
		uec_phy_write(mii_info, MII_DM9161_INTR, MII_DM9161_INTR_STOP);

	return 0;
}

static void dm9161_close (struct uec_mii_info *mii_info)
{
}

static int fixed_phy_aneg (struct uec_mii_info *mii_info)
{
	mii_info->autoneg = 0; /* Turn off auto negotiation for fixed phy */
	return 0;
}

static int fixed_phy_read_status (struct uec_mii_info *mii_info)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(fixed_phy_port); i++) {
		if (strncmp(mii_info->dev->name, fixed_phy_port[i].name,
				strlen(mii_info->dev->name)) == 0) {
			mii_info->speed = fixed_phy_port[i].speed;
			mii_info->duplex = fixed_phy_port[i].duplex;
			mii_info->link = 1; /* Link is always UP */
			mii_info->pause = 0;
			break;
		}
	}
	return 0;
}

static int smsc_config_aneg (struct uec_mii_info *mii_info)
{
	return 0;
}

static int smsc_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there
	 * was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;

	/* If the link is up, read the speed and duplex */
	/* If we aren't autonegotiating, assume speeds
	 * are as set */
	if (mii_info->autoneg && mii_info->link) {
		int	val;

		status = uec_phy_read(mii_info, 0x1f);
		val = (status & 0x1c) >> 2;

		switch (val) {
			case 1:
				mii_info->duplex = DUPLEX_HALF;
				mii_info->speed = SPEED_10;
				break;
			case 5:
				mii_info->duplex = DUPLEX_FULL;
				mii_info->speed = SPEED_10;
				break;
			case 2:
				mii_info->duplex = DUPLEX_HALF;
				mii_info->speed = SPEED_100;
				break;
			case 6:
				mii_info->duplex = DUPLEX_FULL;
				mii_info->speed = SPEED_100;
				break;
		}
		mii_info->pause = 0;
	}

	return 0;
}

static struct phy_info phy_info_dm9161 = {
	.phy_id = 0x0181b880,
	.phy_id_mask = 0x0ffffff0,
	.name = "Davicom DM9161E",
	.init = dm9161_init,
	.config_aneg = dm9161_config_aneg,
	.read_status = dm9161_read_status,
	.close = dm9161_close,
};

static struct phy_info phy_info_dm9161a = {
	.phy_id = 0x0181b8a0,
	.phy_id_mask = 0x0ffffff0,
	.name = "Davicom DM9161A",
	.features = MII_BASIC_FEATURES,
	.init = dm9161_init,
	.config_aneg = dm9161_config_aneg,
	.read_status = dm9161_read_status,
	.ack_interrupt = dm9161_ack_interrupt,
	.config_intr = dm9161_config_intr,
	.close = dm9161_close,
};

static struct phy_info phy_info_marvell = {
	.phy_id = 0x01410c00,
	.phy_id_mask = 0xffffff00,
	.name = "Marvell 88E11x1",
	.features = MII_GBIT_FEATURES,
	.init = &uec_marvell_init,
	.config_aneg = &marvell_config_aneg,
	.read_status = &marvell_read_status,
	.ack_interrupt = &marvell_ack_interrupt,
	.config_intr = &marvell_config_intr,
};

static struct phy_info phy_info_bcm5481 = {
	.phy_id = 0x0143bca0,
	.phy_id_mask = 0xffffff0,
	.name = "Broadcom 5481",
	.features = MII_GBIT_FEATURES,
	.read_status = genmii_read_status,
	.init = bcm_init,
};

static struct phy_info phy_info_fixedphy = {
	.phy_id = CONFIG_FIXED_PHY,
	.phy_id_mask = CONFIG_FIXED_PHY,
	.name = "Fixed PHY",
	.config_aneg = fixed_phy_aneg,
	.read_status = fixed_phy_read_status,
};

static struct phy_info phy_info_smsclan8700 = {
	.phy_id = 0x0007c0c0,
	.phy_id_mask = 0xfffffff0,
	.name = "SMSC LAN8700",
	.features = MII_BASIC_FEATURES,
	.config_aneg = smsc_config_aneg,
	.read_status = smsc_read_status,
};

static struct phy_info phy_info_genmii = {
	.phy_id = 0x00000000,
	.phy_id_mask = 0x00000000,
	.name = "Generic MII",
	.features = MII_BASIC_FEATURES,
	.config_aneg = genmii_config_aneg,
	.read_status = genmii_read_status,
};

static struct phy_info *phy_info[] = {
	&phy_info_dm9161,
	&phy_info_dm9161a,
	&phy_info_marvell,
	&phy_info_bcm5481,
	&phy_info_smsclan8700,
	&phy_info_fixedphy,
	&phy_info_genmii,
	NULL
};

u16 uec_phy_read(struct uec_mii_info *mii_info, u16 regnum)
{
	return mii_info->mdio_read (mii_info->dev, mii_info->mii_id, regnum);
}

void uec_phy_write(struct uec_mii_info *mii_info, u16 regnum, u16 val)
{
	mii_info->mdio_write (mii_info->dev, mii_info->mii_id, regnum, val);
}

/* Use the PHY ID registers to determine what type of PHY is attached
 * to device dev.  return a struct phy_info structure describing that PHY
 */
struct phy_info *uec_get_phy_info (struct uec_mii_info *mii_info)
{
	u16 phy_reg;
	u32 phy_ID;
	int i;
	struct phy_info *theInfo = NULL;

	/* Grab the bits from PHYIR1, and put them in the upper half */
	phy_reg = uec_phy_read(mii_info, MII_PHYSID1);
	phy_ID = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = uec_phy_read(mii_info, MII_PHYSID2);
	phy_ID |= (phy_reg & 0xffff);

	/* loop through all the known PHY types, and find one that */
	/* matches the ID we read from the PHY. */
	for (i = 0; phy_info[i]; i++)
		if (phy_info[i]->phy_id ==
		    (phy_ID & phy_info[i]->phy_id_mask)) {
			theInfo = phy_info[i];
			break;
		}

	/* This shouldn't happen, as we have generic PHY support */
	if (theInfo == NULL) {
		ugphy_info ("UEC: PHY id %x is not supported!", phy_ID);
		return NULL;
	} else {
		ugphy_info ("UEC: PHY is %s (%x)", theInfo->name, phy_ID);
	}

	return theInfo;
}

void marvell_phy_interface_mode(struct eth_device *dev, phy_interface_t type,
		int speed)
{
	uec_private_t *uec = (uec_private_t *) dev->priv;
	struct uec_mii_info *mii_info;
	u16 status;

	if (!uec->mii_info) {
		printf ("%s: the PHY not initialized\n", __FUNCTION__);
		return;
	}
	mii_info = uec->mii_info;

	if (type == PHY_INTERFACE_MODE_RGMII) {
		if (speed == SPEED_100) {
			uec_phy_write(mii_info, 0x00, 0x9140);
			uec_phy_write(mii_info, 0x1d, 0x001f);
			uec_phy_write(mii_info, 0x1e, 0x200c);
			uec_phy_write(mii_info, 0x1d, 0x0005);
			uec_phy_write(mii_info, 0x1e, 0x0000);
			uec_phy_write(mii_info, 0x1e, 0x0100);
			uec_phy_write(mii_info, 0x09, 0x0e00);
			uec_phy_write(mii_info, 0x04, 0x01e1);
			uec_phy_write(mii_info, 0x00, 0x9140);
			uec_phy_write(mii_info, 0x00, 0x1000);
			udelay (100000);
			uec_phy_write(mii_info, 0x00, 0x2900);
			uec_phy_write(mii_info, 0x14, 0x0cd2);
			uec_phy_write(mii_info, 0x00, 0xa100);
			uec_phy_write(mii_info, 0x09, 0x0000);
			uec_phy_write(mii_info, 0x1b, 0x800b);
			uec_phy_write(mii_info, 0x04, 0x05e1);
			uec_phy_write(mii_info, 0x00, 0xa100);
			uec_phy_write(mii_info, 0x00, 0x2100);
			udelay (1000000);
		} else if (speed == SPEED_10) {
			uec_phy_write(mii_info, 0x14, 0x8e40);
			uec_phy_write(mii_info, 0x1b, 0x800b);
			uec_phy_write(mii_info, 0x14, 0x0c82);
			uec_phy_write(mii_info, 0x00, 0x8100);
			udelay (1000000);
		}
	}

	/* handle 88e1111 rev.B2 erratum 5.6 */
	if (mii_info->autoneg) {
		status = uec_phy_read(mii_info, MII_BMCR);
		uec_phy_write(mii_info, MII_BMCR, status | BMCR_ANENABLE);
	}
	/* now the B2 will correctly report autoneg completion status */
}

void change_phy_interface_mode (struct eth_device *dev,
				phy_interface_t type, int speed)
{
#ifdef CONFIG_PHY_MODE_NEED_CHANGE
	marvell_phy_interface_mode (dev, type, speed);
#endif
}
