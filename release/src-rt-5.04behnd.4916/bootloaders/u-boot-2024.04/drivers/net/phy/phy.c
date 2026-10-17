// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic PHY Management code
 *
 * Copyright 2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 *
 * Based loosely off of Linux's PHY Lib
 */
#include <common.h>
#include <console.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <miiphy.h>
#include <phy.h>
#include <errno.h>
#include <asm/global_data.h>
#include <dm/of_extra.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

/* Generic PHY support and helper functions */

/**
 * genphy_config_advert - sanitize and advertise auto-negotiation parameters
 * @phydev: target phy_device struct
 *
 * Description: Writes MII_ADVERTISE with the appropriate values,
 *   after sanitizing the values to make sure we only advertise
 *   what is supported.  Returns < 0 on error, 0 if the PHY's advertisement
 *   hasn't changed, and > 0 if it has changed.
 */
static int genphy_config_advert(struct phy_device *phydev)
{
	u32 advertise;
	int oldadv, adv, bmsr;
	int err, changed = 0;

	/* Only allow advertising what this PHY supports */
	phydev->advertising &= phydev->supported;
	advertise = phydev->advertising;

	/* Setup standard advertisement */
	adv = phy_read(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE);
	oldadv = adv;

	if (adv < 0)
		return adv;

	adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4 | ADVERTISE_PAUSE_CAP |
		 ADVERTISE_PAUSE_ASYM);
	if (advertise & ADVERTISED_10baseT_Half)
		adv |= ADVERTISE_10HALF;
	if (advertise & ADVERTISED_10baseT_Full)
		adv |= ADVERTISE_10FULL;
	if (advertise & ADVERTISED_100baseT_Half)
		adv |= ADVERTISE_100HALF;
	if (advertise & ADVERTISED_100baseT_Full)
		adv |= ADVERTISE_100FULL;
	if (advertise & ADVERTISED_Pause)
		adv |= ADVERTISE_PAUSE_CAP;
	if (advertise & ADVERTISED_Asym_Pause)
		adv |= ADVERTISE_PAUSE_ASYM;
	if (advertise & ADVERTISED_1000baseX_Half)
		adv |= ADVERTISE_1000XHALF;
	if (advertise & ADVERTISED_1000baseX_Full)
		adv |= ADVERTISE_1000XFULL;

	if (adv != oldadv) {
		err = phy_write(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE, adv);

		if (err < 0)
			return err;
		changed = 1;
	}

	bmsr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
	if (bmsr < 0)
		return bmsr;

	/* Per 802.3-2008, Section 22.2.4.2.16 Extended status all
	 * 1000Mbits/sec capable PHYs shall have the BMSR_ESTATEN bit set to a
	 * logical 1.
	 */
	if (!(bmsr & BMSR_ESTATEN))
		return changed;

	/* Configure gigabit if it's supported */
	adv = phy_read(phydev, MDIO_DEVAD_NONE, MII_CTRL1000);
	oldadv = adv;

	if (adv < 0)
		return adv;

	adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

	if (phydev->supported & (SUPPORTED_1000baseT_Half |
				SUPPORTED_1000baseT_Full)) {
		if (advertise & SUPPORTED_1000baseT_Half)
			adv |= ADVERTISE_1000HALF;
		if (advertise & SUPPORTED_1000baseT_Full)
			adv |= ADVERTISE_1000FULL;
	}

	if (adv != oldadv)
		changed = 1;

	err = phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, adv);
	if (err < 0)
		return err;

	return changed;
}

/**
 * genphy_setup_forced - configures/forces speed/duplex from @phydev
 * @phydev: target phy_device struct
 *
 * Description: Configures MII_BMCR to force speed/duplex
 *   to the values in phydev. Assumes that the values are valid.
 */
static int genphy_setup_forced(struct phy_device *phydev)
{
	int err;
	int ctl = BMCR_ANRESTART;

	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (phydev->speed == SPEED_1000)
		ctl |= BMCR_SPEED1000;
	else if (phydev->speed == SPEED_100)
		ctl |= BMCR_SPEED100;

	if (phydev->duplex == DUPLEX_FULL)
		ctl |= BMCR_FULLDPLX;

	err = phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, ctl);

	return err;
}

/**
 * genphy_restart_aneg - Enable and Restart Autonegotiation
 * @phydev: target phy_device struct
 */
int genphy_restart_aneg(struct phy_device *phydev)
{
	int ctl;

	ctl = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

	if (ctl < 0)
		return ctl;

	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);

	/* Don't isolate the PHY if we're negotiating */
	ctl &= ~(BMCR_ISOLATE);

	ctl = phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, ctl);

	return ctl;
}

/**
 * genphy_config_aneg - restart auto-negotiation or write BMCR
 * @phydev: target phy_device struct
 *
 * Description: If auto-negotiation is enabled, we configure the
 *   advertising, and then restart auto-negotiation.  If it is not
 *   enabled, then we write the BMCR.
 */
int genphy_config_aneg(struct phy_device *phydev)
{
	int result;

	if (phydev->autoneg != AUTONEG_ENABLE)
		return genphy_setup_forced(phydev);

	result = genphy_config_advert(phydev);

	if (result < 0) /* error */
		return result;

	if (result == 0) {
		/*
		 * Advertisment hasn't changed, but maybe aneg was never on to
		 * begin with?  Or maybe phy was isolated?
		 */
		int ctl = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

		if (ctl < 0)
			return ctl;

		if (!(ctl & BMCR_ANENABLE) || (ctl & BMCR_ISOLATE))
			result = 1; /* do restart aneg */
	}

	/*
	 * Only restart aneg if we are advertising something different
	 * than we were before.
	 */
	if (result > 0)
		result = genphy_restart_aneg(phydev);

	return result;
}

/**
 * genphy_update_link - update link status in @phydev
 * @phydev: target phy_device struct
 *
 * Description: Update the value in phydev->link to reflect the
 *   current link value.  In order to do this, we need to read
 *   the status register twice, keeping the second value.
 */
int genphy_update_link(struct phy_device *phydev)
{
	unsigned int mii_reg;

	/*
	 * Wait if the link is up, and autonegotiation is in progress
	 * (ie - we're capable and it's not done)
	 */
	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

	/*
	 * If we already saw the link up, and it hasn't gone down, then
	 * we don't need to wait for autoneg again
	 */
	if (phydev->link && mii_reg & BMSR_LSTATUS)
		return 0;

	if ((phydev->autoneg == AUTONEG_ENABLE) &&
	    !(mii_reg & BMSR_ANEGCOMPLETE)) {
		int i = 0;

		printf("%s Waiting for PHY auto negotiation to complete",
		       phydev->dev->name);
		while (!(mii_reg & BMSR_ANEGCOMPLETE)) {
			/*
			 * Timeout reached ?
			 */
			if (i > (PHY_ANEG_TIMEOUT / 50)) {
				printf(" TIMEOUT !\n");
				phydev->link = 0;
				return -ETIMEDOUT;
			}

			if (ctrlc()) {
				puts("user interrupt!\n");
				phydev->link = 0;
				return -EINTR;
			}

			if ((i++ % 10) == 0)
				printf(".");

			mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
			mdelay(50);	/* 50 ms */
		}
		printf(" done\n");
		phydev->link = 1;
	} else {
		/* Read the link a second time to clear the latched state */
		mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

		if (mii_reg & BMSR_LSTATUS)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	return 0;
}

/*
 * Generic function which updates the speed and duplex.  If
 * autonegotiation is enabled, it uses the AND of the link
 * partner's advertised capabilities and our advertised
 * capabilities.  If autonegotiation is disabled, we use the
 * appropriate bits in the control register.
 *
 * Stolen from Linux's mii.c and phy_device.c
 */
int genphy_parse_link(struct phy_device *phydev)
{
	int mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

	/* We're using autonegotiation */
	if (phydev->autoneg == AUTONEG_ENABLE) {
		u32 lpa = 0;
		int gblpa = 0;
		u32 estatus = 0;

		/* Check for gigabit capability */
		if (phydev->supported & (SUPPORTED_1000baseT_Full |
					SUPPORTED_1000baseT_Half)) {
			/* We want a list of states supported by
			 * both PHYs in the link
			 */
			gblpa = phy_read(phydev, MDIO_DEVAD_NONE, MII_STAT1000);
			if (gblpa < 0) {
				debug("Could not read MII_STAT1000. ");
				debug("Ignoring gigabit capability\n");
				gblpa = 0;
			}
			gblpa &= phy_read(phydev,
					MDIO_DEVAD_NONE, MII_CTRL1000) << 2;
		}

		/* Set the baseline so we only have to set them
		 * if they're different
		 */
		phydev->speed = SPEED_10;
		phydev->duplex = DUPLEX_HALF;

		/* Check the gigabit fields */
		if (gblpa & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)) {
			phydev->speed = SPEED_1000;

			if (gblpa & PHY_1000BTSR_1000FD)
				phydev->duplex = DUPLEX_FULL;

			/* We're done! */
			return 0;
		}

		lpa = phy_read(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE);
		lpa &= phy_read(phydev, MDIO_DEVAD_NONE, MII_LPA);

		if (lpa & (LPA_100FULL | LPA_100HALF)) {
			phydev->speed = SPEED_100;

			if (lpa & LPA_100FULL)
				phydev->duplex = DUPLEX_FULL;

		} else if (lpa & LPA_10FULL) {
			phydev->duplex = DUPLEX_FULL;
		}

		/*
		 * Extended status may indicate that the PHY supports
		 * 1000BASE-T/X even though the 1000BASE-T registers
		 * are missing. In this case we can't tell whether the
		 * peer also supports it, so we only check extended
		 * status if the 1000BASE-T registers are actually
		 * missing.
		 */
		if ((mii_reg & BMSR_ESTATEN) && !(mii_reg & BMSR_ERCAP))
			estatus = phy_read(phydev, MDIO_DEVAD_NONE,
					   MII_ESTATUS);

		if (estatus & (ESTATUS_1000_XFULL | ESTATUS_1000_XHALF |
				ESTATUS_1000_TFULL | ESTATUS_1000_THALF)) {
			phydev->speed = SPEED_1000;
			if (estatus & (ESTATUS_1000_XFULL | ESTATUS_1000_TFULL))
				phydev->duplex = DUPLEX_FULL;
		}

	} else {
		u32 bmcr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

		phydev->speed = SPEED_10;
		phydev->duplex = DUPLEX_HALF;

		if (bmcr & BMCR_FULLDPLX)
			phydev->duplex = DUPLEX_FULL;

		if (bmcr & BMCR_SPEED1000)
			phydev->speed = SPEED_1000;
		else if (bmcr & BMCR_SPEED100)
			phydev->speed = SPEED_100;
	}

	return 0;
}

int genphy_config(struct phy_device *phydev)
{
	int val;
	u32 features;

	features = (SUPPORTED_TP | SUPPORTED_MII
			| SUPPORTED_AUI | SUPPORTED_FIBRE |
			SUPPORTED_BNC);

	/* Do we support autonegotiation? */
	val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

	if (val < 0)
		return val;

	if (val & BMSR_ANEGCAPABLE)
		features |= SUPPORTED_Autoneg;

	if (val & BMSR_100FULL)
		features |= SUPPORTED_100baseT_Full;
	if (val & BMSR_100HALF)
		features |= SUPPORTED_100baseT_Half;
	if (val & BMSR_10FULL)
		features |= SUPPORTED_10baseT_Full;
	if (val & BMSR_10HALF)
		features |= SUPPORTED_10baseT_Half;

	if (val & BMSR_ESTATEN) {
		val = phy_read(phydev, MDIO_DEVAD_NONE, MII_ESTATUS);

		if (val < 0)
			return val;

		if (val & ESTATUS_1000_TFULL)
			features |= SUPPORTED_1000baseT_Full;
		if (val & ESTATUS_1000_THALF)
			features |= SUPPORTED_1000baseT_Half;
		if (val & ESTATUS_1000_XFULL)
			features |= SUPPORTED_1000baseX_Full;
		if (val & ESTATUS_1000_XHALF)
			features |= SUPPORTED_1000baseX_Half;
	}

	phydev->supported &= features;
	phydev->advertising &= features;

	genphy_config_aneg(phydev);

	return 0;
}

int genphy_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return genphy_parse_link(phydev);
}

int genphy_shutdown(struct phy_device *phydev)
{
	return 0;
}

U_BOOT_PHY_DRIVER(genphy) = {
	.uid		= 0xffffffff,
	.mask		= 0xffffffff,
	.name		= "Generic PHY",
	.features	= PHY_GBIT_FEATURES | SUPPORTED_MII |
			  SUPPORTED_AUI | SUPPORTED_FIBRE |
			  SUPPORTED_BNC,
	.config		= genphy_config,
	.startup	= genphy_startup,
	.shutdown	= genphy_shutdown,
};

int phy_set_supported(struct phy_device *phydev, u32 max_speed)
{
	/* The default values for phydev->supported are provided by the PHY
	 * driver "features" member, we want to reset to sane defaults first
	 * before supporting higher speeds.
	 */
	phydev->supported &= PHY_DEFAULT_FEATURES;

	switch (max_speed) {
	default:
		return -ENOTSUPP;
	case SPEED_1000:
		phydev->supported |= PHY_1000BT_FEATURES;
		/* fall through */
	case SPEED_100:
		phydev->supported |= PHY_100BT_FEATURES;
		/* fall through */
	case SPEED_10:
		phydev->supported |= PHY_10BT_FEATURES;
	}

	return 0;
}

static int phy_probe(struct phy_device *phydev)
{
	int err = 0;

	phydev->advertising = phydev->drv->features;
	phydev->supported = phydev->drv->features;

	phydev->mmds = phydev->drv->mmds;

	if (phydev->drv->probe)
		err = phydev->drv->probe(phydev);

	return err;
}

static struct phy_driver *generic_for_phy(struct phy_device *phydev)
{
#ifdef CONFIG_PHYLIB_10G
	if (phydev->is_c45)
		return ll_entry_get(struct phy_driver, gen10g, phy_driver);
#endif

	return ll_entry_get(struct phy_driver, genphy, phy_driver);
}

static struct phy_driver *get_phy_driver(struct phy_device *phydev)
{
	const int ll_n_ents = ll_entry_count(struct phy_driver, phy_driver);
	int phy_id = phydev->phy_id;
	struct phy_driver *ll_entry;
	struct phy_driver *drv;

	ll_entry = ll_entry_start(struct phy_driver, phy_driver);
	for (drv = ll_entry; drv != ll_entry + ll_n_ents; drv++)
		if ((drv->uid & drv->mask) == (phy_id & drv->mask))
			return drv;

	/* If we made it here, there's no driver for this PHY */
	return generic_for_phy(phydev);
}

struct phy_device *phy_device_create(struct mii_dev *bus, int addr,
				     u32 phy_id, bool is_c45)
{
	struct phy_device *dev;

	/*
	 * We allocate the device, and initialize the
	 * default values
	 */
	dev = malloc(sizeof(*dev));
	if (!dev) {
		printf("Failed to allocate PHY device for %s:%d\n",
		       bus ? bus->name : "(null bus)", addr);
		return NULL;
	}

	memset(dev, 0, sizeof(*dev));

	dev->duplex = -1;
	dev->link = 0;
	dev->interface = PHY_INTERFACE_MODE_NA;

	dev->node = ofnode_null();

	dev->autoneg = AUTONEG_ENABLE;

	dev->addr = addr;
	dev->phy_id = phy_id;
	dev->is_c45 = is_c45;
	dev->bus = bus;

	dev->drv = get_phy_driver(dev);

	if (phy_probe(dev)) {
		printf("%s, PHY probe failed\n", __func__);
		return NULL;
	}

	if (addr >= 0 && addr < PHY_MAX_ADDR && phy_id != PHY_FIXED_ID)
		bus->phymap[addr] = dev;

	return dev;
}

/**
 * get_phy_id - reads the specified addr for its ID.
 * @bus: the target MII bus
 * @addr: PHY address on the MII bus
 * @phy_id: where to store the ID retrieved.
 *
 * Description: Reads the ID registers of the PHY at @addr on the
 *   @bus, stores it in @phy_id and returns zero on success.
 */
int __weak get_phy_id(struct mii_dev *bus, int addr, int devad, u32 *phy_id)
{
	int phy_reg;

	/*
	 * Grab the bits from PHYIR1, and put them
	 * in the upper half
	 */
	phy_reg = bus->read(bus, addr, devad, MII_PHYSID1);

	if (phy_reg < 0)
		return -EIO;

	*phy_id = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = bus->read(bus, addr, devad, MII_PHYSID2);

	if (phy_reg < 0)
		return -EIO;

	*phy_id |= (phy_reg & 0xffff);

	return 0;
}

static struct phy_device *create_phy_by_mask(struct mii_dev *bus,
					     uint phy_mask, int devad)
{
	u32 phy_id = 0xffffffff;
	bool is_c45;

	while (phy_mask) {
		int addr = ffs(phy_mask) - 1;
		int r = get_phy_id(bus, addr, devad, &phy_id);

		/*
		 * If the PHY ID is flat 0 we ignore it.  There are C45 PHYs
		 * that return all 0s for C22 reads (like Aquantia AQR112) and
		 * there are C22 PHYs that return all 0s for C45 reads (like
		 * Atheros AR8035).
		 */
		if (r == 0 && phy_id == 0)
			goto next;

		/* If the PHY ID is mostly f's, we didn't find anything */
		if (r == 0 && (phy_id & 0x1fffffff) != 0x1fffffff) {
			is_c45 = (devad == MDIO_DEVAD_NONE) ? false : true;
			return phy_device_create(bus, addr, phy_id, is_c45);
		}
next:
		phy_mask &= ~(1 << addr);
	}
	return NULL;
}

static struct phy_device *search_for_existing_phy(struct mii_dev *bus,
						  uint phy_mask)
{
	/* If we have one, return the existing device, with new interface */
	while (phy_mask) {
		int addr = ffs(phy_mask) - 1;

		if (bus->phymap[addr])
			return bus->phymap[addr];

		phy_mask &= ~(1 << addr);
	}
	return NULL;
}

static struct phy_device *get_phy_device_by_mask(struct mii_dev *bus,
						 uint phy_mask)
{
	struct phy_device *phydev;
	int devad[] = {
		/* Clause-22 */
		MDIO_DEVAD_NONE,
		/* Clause-45 */
		MDIO_MMD_PMAPMD,
		MDIO_MMD_WIS,
		MDIO_MMD_PCS,
		MDIO_MMD_PHYXS,
		MDIO_MMD_VEND1,
	};
	int i, devad_cnt;

	devad_cnt = sizeof(devad)/sizeof(int);
	phydev = search_for_existing_phy(bus, phy_mask);
	if (phydev)
		return phydev;
	/* try different access clauses  */
	for (i = 0; i < devad_cnt; i++) {
		phydev = create_phy_by_mask(bus, phy_mask, devad[i]);
		if (IS_ERR(phydev))
			return NULL;
		if (phydev)
			return phydev;
	}

	debug("\n%s PHY: ", bus->name);
	while (phy_mask) {
		int addr = ffs(phy_mask) - 1;

		debug("%d ", addr);
		phy_mask &= ~(1 << addr);
	}
	debug("not found\n");

	return NULL;
}

/**
 * get_phy_device - reads the specified PHY device and returns its
 *                  @phy_device struct
 * @bus: the target MII bus
 * @addr: PHY address on the MII bus
 *
 * Description: Reads the ID registers of the PHY at @addr on the
 *   @bus, then allocates and returns the phy_device to represent it.
 */
static struct phy_device *get_phy_device(struct mii_dev *bus, int addr)
{
	return get_phy_device_by_mask(bus, 1 << addr);
}

int phy_reset(struct phy_device *phydev)
{
	int reg;
	int timeout = 500;
	int devad = MDIO_DEVAD_NONE;

	if (phydev->flags & PHY_FLAG_BROKEN_RESET)
		return 0;

#ifdef CONFIG_PHYLIB_10G
	/* If it's 10G, we need to issue reset through one of the MMDs */
	if (phydev->is_c45) {
		if (!phydev->mmds)
			gen10g_discover_mmds(phydev);

		devad = ffs(phydev->mmds) - 1;
	}
#endif

	if (phy_write(phydev, devad, MII_BMCR, BMCR_RESET) < 0) {
		debug("PHY reset failed\n");
		return -1;
	}

#if CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = phy_read(phydev, devad, MII_BMCR);
	while ((reg & BMCR_RESET) && timeout--) {
		reg = phy_read(phydev, devad, MII_BMCR);

		if (reg < 0) {
			debug("PHY status read failed\n");
			return -1;
		}
		udelay(1000);
	}

	if (reg & BMCR_RESET) {
		puts("PHY reset timed out\n");
		return -1;
	}

	return 0;
}

int miiphy_reset(const char *devname, unsigned char addr)
{
	struct mii_dev *bus = miiphy_get_dev_by_name(devname);
	struct phy_device *phydev;

	phydev = get_phy_device(bus, addr);

	return phy_reset(phydev);
}

struct phy_device *phy_find_by_mask(struct mii_dev *bus, uint phy_mask)
{
	/* Reset the bus */
	if (bus->reset) {
		bus->reset(bus);

		/* Wait 15ms to make sure the PHY has come out of hard reset */
		mdelay(15);
	}

	return get_phy_device_by_mask(bus, phy_mask);
}

static void phy_connect_dev(struct phy_device *phydev, struct udevice *dev,
			    phy_interface_t interface)
{
	/* Soft Reset the PHY */
	phy_reset(phydev);
	if (phydev->dev && phydev->dev != dev) {
		printf("%s:%d is connected to %s.  Reconnecting to %s\n",
		       phydev->bus->name, phydev->addr,
		       phydev->dev->name, dev->name);
	}
	phydev->dev = dev;
	phydev->interface = interface;
	debug("%s connected to %s mode %s\n", dev->name, phydev->drv->name,
	      phy_string_for_interface(interface));
}

#ifdef CONFIG_PHY_XILINX_GMII2RGMII
static struct phy_device *phy_connect_gmii2rgmii(struct mii_dev *bus,
						 struct udevice *dev)
{
	struct phy_device *phydev = NULL;
	ofnode node;

	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		node = ofnode_by_compatible(node, "xlnx,gmii-to-rgmii-1.0");
		if (ofnode_valid(node)) {
			int gmiirgmii_phyaddr;

			gmiirgmii_phyaddr = ofnode_read_u32_default(node, "reg", 0);
			phydev = phy_device_create(bus, gmiirgmii_phyaddr,
						   PHY_GMII2RGMII_ID, false);
			if (phydev)
				phydev->node = node;
			break;
		}

		node = ofnode_first_subnode(node);
	}

	return phydev;
}
#endif

#ifdef CONFIG_PHY_FIXED
/**
 * fixed_phy_create() - create an unconnected fixed-link pseudo-PHY device
 * @node: OF node for the container of the fixed-link node
 *
 * Description: Creates a struct phy_device based on a fixed-link of_node
 * description. Can be used without phy_connect by drivers which do not expose
 * a UCLASS_ETH udevice.
 */
struct phy_device *fixed_phy_create(ofnode node)
{
	struct phy_device *phydev;
	ofnode subnode;

	subnode = ofnode_find_subnode(node, "fixed-link");
	if (!ofnode_valid(subnode)) {
		return NULL;
	}

	phydev = phy_device_create(NULL, 0, PHY_FIXED_ID, false);
	if (phydev) {
		phydev->node = subnode;
		phydev->interface = ofnode_read_phy_mode(node);
	}

	return phydev;
}

static struct phy_device *phy_connect_fixed(struct mii_dev *bus,
					    struct udevice *dev)
{
	ofnode node = dev_ofnode(dev), subnode;
	struct phy_device *phydev = NULL;

	if (ofnode_phy_is_fixed_link(node, &subnode)) {
		phydev = phy_device_create(bus, 0, PHY_FIXED_ID, false);
		if (phydev)
			phydev->node = subnode;
	}

	return phydev;
}
#endif

struct phy_device *phy_connect(struct mii_dev *bus, int addr,
			       struct udevice *dev,
			       phy_interface_t interface)
{
	struct phy_device *phydev = NULL;
	uint mask = (addr >= 0) ? (1 << addr) : 0xffffffff;

#ifdef CONFIG_PHY_FIXED
	phydev = phy_connect_fixed(bus, dev);
#endif

#ifdef CONFIG_PHY_NCSI
	if (!phydev && interface == PHY_INTERFACE_MODE_NCSI)
		phydev = phy_device_create(bus, 0, PHY_NCSI_ID, false);
#endif

#ifdef CONFIG_PHY_ETHERNET_ID
	if (!phydev)
		phydev = phy_connect_phy_id(bus, dev, addr);
#endif

#ifdef CONFIG_PHY_XILINX_GMII2RGMII
	if (!phydev)
		phydev = phy_connect_gmii2rgmii(bus, dev);
#endif

	if (!phydev)
		phydev = phy_find_by_mask(bus, mask);

	if (phydev)
		phy_connect_dev(phydev, dev, interface);
	else
		printf("Could not get PHY for %s: addr %d\n", bus->name, addr);
	return phydev;
}

/*
 * Start the PHY.  Returns 0 on success, or a negative error code.
 */
int phy_startup(struct phy_device *phydev)
{
	if (phydev->drv->startup)
		return phydev->drv->startup(phydev);

	return 0;
}

__weak int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		return phydev->drv->config(phydev);
	return 0;
}

int phy_config(struct phy_device *phydev)
{
	/* Invoke an optional board-specific helper */
	return board_phy_config(phydev);
}

int phy_shutdown(struct phy_device *phydev)
{
	if (phydev->drv->shutdown)
		phydev->drv->shutdown(phydev);

	return 0;
}

/**
 * phy_modify - Convenience function for modifying a given PHY register
 * @phydev: the phy_device struct
 * @devad: The MMD to read from
 * @regnum: register number to write
 * @mask: bit mask of bits to clear
 * @set: new value of bits set in mask to write to @regnum
 */
int phy_modify(struct phy_device *phydev, int devad, int regnum, u16 mask,
	       u16 set)
{
	int ret;

	ret = phy_read(phydev, devad, regnum);
	if (ret < 0)
		return ret;

	return phy_write(phydev, devad, regnum, (ret & ~mask) | set);
}

/**
 * phy_read - Convenience function for reading a given PHY register
 * @phydev: the phy_device struct
 * @devad: The MMD to read from
 * @regnum: register number to read
 * @return: value for success or negative errno for failure
 */
int phy_read(struct phy_device *phydev, int devad, int regnum)
{
	struct mii_dev *bus = phydev->bus;

	if (!bus || !bus->read) {
		debug("%s: No bus configured\n", __func__);
		return -1;
	}

	return bus->read(bus, phydev->addr, devad, regnum);
}

/**
 * phy_write - Convenience function for writing a given PHY register
 * @phydev: the phy_device struct
 * @devad: The MMD to read from
 * @regnum: register number to write
 * @val: value to write to @regnum
 * @return: 0 for success or negative errno for failure
 */
int phy_write(struct phy_device *phydev, int devad, int regnum, u16 val)
{
	struct mii_dev *bus = phydev->bus;

	if (!bus || !bus->write) {
		debug("%s: No bus configured\n", __func__);
		return -1;
	}

	return bus->write(bus, phydev->addr, devad, regnum, val);
}

/**
 * phy_mmd_start_indirect - Convenience function for writing MMD registers
 * @phydev: the phy_device struct
 * @devad: The MMD to read from
 * @regnum: register number to write
 * @return: None
 */
void phy_mmd_start_indirect(struct phy_device *phydev, int devad, int regnum)
{
	/* Write the desired MMD Devad */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_CTRL, devad);

	/* Write the desired MMD register address */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_DATA, regnum);

	/* Select the Function : DATA with no post increment */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_CTRL,
		  (devad | MII_MMD_CTRL_NOINCR));
}

/**
 * phy_read_mmd - Convenience function for reading a register
 * from an MMD on a given PHY.
 * @phydev: The phy_device struct
 * @devad: The MMD to read from
 * @regnum: The register on the MMD to read
 * @return: Value for success or negative errno for failure
 */
int phy_read_mmd(struct phy_device *phydev, int devad, int regnum)
{
	struct phy_driver *drv = phydev->drv;

	if (regnum > (u16)~0 || devad > 32)
		return -EINVAL;

	/* driver-specific access */
	if (drv->read_mmd)
		return drv->read_mmd(phydev, devad, regnum);

	/* direct C45 / C22 access */
	if ((drv->features & PHY_10G_FEATURES) == PHY_10G_FEATURES ||
	    devad == MDIO_DEVAD_NONE || !devad)
		return phy_read(phydev, devad, regnum);

	/* indirect C22 access */
	phy_mmd_start_indirect(phydev, devad, regnum);

	/* Read the content of the MMD's selected register */
	return phy_read(phydev, MDIO_DEVAD_NONE, MII_MMD_DATA);
}

/**
 * phy_write_mmd - Convenience function for writing a register
 * on an MMD on a given PHY.
 * @phydev: The phy_device struct
 * @devad: The MMD to read from
 * @regnum: The register on the MMD to read
 * @val: value to write to @regnum
 * @return: 0 for success or negative errno for failure
 */
int phy_write_mmd(struct phy_device *phydev, int devad, int regnum, u16 val)
{
	struct phy_driver *drv = phydev->drv;

	if (regnum > (u16)~0 || devad > 32)
		return -EINVAL;

	/* driver-specific access */
	if (drv->write_mmd)
		return drv->write_mmd(phydev, devad, regnum, val);

	/* direct C45 / C22 access */
	if ((drv->features & PHY_10G_FEATURES) == PHY_10G_FEATURES ||
	    devad == MDIO_DEVAD_NONE || !devad)
		return phy_write(phydev, devad, regnum, val);

	/* indirect C22 access */
	phy_mmd_start_indirect(phydev, devad, regnum);

	/* Write the data into MMD's selected register */
	return phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_DATA, val);
}

/**
 * phy_set_bits_mmd - Convenience function for setting bits in a register
 * on MMD
 * @phydev: the phy_device struct
 * @devad: the MMD containing register to modify
 * @regnum: register number to modify
 * @val: bits to set
 * @return: 0 for success or negative errno for failure
 */
int phy_set_bits_mmd(struct phy_device *phydev, int devad, u32 regnum, u16 val)
{
	int value, ret;

	value = phy_read_mmd(phydev, devad, regnum);
	if (value < 0)
		return value;

	value |= val;

	ret = phy_write_mmd(phydev, devad, regnum, value);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * phy_clear_bits_mmd - Convenience function for clearing bits in a register
 * on MMD
 * @phydev: the phy_device struct
 * @devad: the MMD containing register to modify
 * @regnum: register number to modify
 * @val: bits to clear
 * @return: 0 for success or negative errno for failure
 */
int phy_clear_bits_mmd(struct phy_device *phydev, int devad, u32 regnum, u16 val)
{
	int value, ret;

	value = phy_read_mmd(phydev, devad, regnum);
	if (value < 0)
		return value;

	value &= ~val;

	ret = phy_write_mmd(phydev, devad, regnum, value);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * phy_modify_mmd_changed - Function for modifying a register on MMD
 * @phydev: the phy_device struct
 * @devad: the MMD containing register to modify
 * @regnum: register number to modify
 * @mask: bit mask of bits to clear
 * @set: new value of bits set in mask to write to @regnum
 *
 * NOTE: MUST NOT be called from interrupt context,
 * because the bus read/write functions may wait for an interrupt
 * to conclude the operation.
 *
 * Returns negative errno, 0 if there was no change, and 1 in case of change
 */
int phy_modify_mmd_changed(struct phy_device *phydev, int devad, u32 regnum,
			   u16 mask, u16 set)
{
	int new, ret;

	ret = phy_read_mmd(phydev, devad, regnum);
	if (ret < 0)
		return ret;

	new = (ret & ~mask) | set;
	if (new == ret)
		return 0;

	ret = phy_write_mmd(phydev, devad, regnum, new);

	return ret < 0 ? ret : 1;
}

/**
 * phy_modify_mmd - Convenience function for modifying a register on MMD
 * @phydev: the phy_device struct
 * @devad: the MMD containing register to modify
 * @regnum: register number to modify
 * @mask: bit mask of bits to clear
 * @set: new value of bits set in mask to write to @regnum
 *
 * NOTE: MUST NOT be called from interrupt context,
 * because the bus read/write functions may wait for an interrupt
 * to conclude the operation.
 */
int phy_modify_mmd(struct phy_device *phydev, int devad, u32 regnum,
		   u16 mask, u16 set)
{
	int ret;

	ret = phy_modify_mmd_changed(phydev, devad, regnum, mask, set);

	return ret < 0 ? ret : 0;
}

bool phy_interface_is_ncsi(void)
{
#ifdef CONFIG_PHY_NCSI
	struct eth_pdata *pdata = dev_get_plat(eth_get_dev());

	return pdata->phy_interface == PHY_INTERFACE_MODE_NCSI;
#else
	return 0;
#endif
}
