/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *	Andy Fleming <afleming@gmail.com>
 *
 * This file pretty much stolen from Linux's mii.h/ethtool.h/phy.h
 */

#ifndef _PHY_H
#define _PHY_H

#include <log.h>
#include <phy_interface.h>
#include <dm/ofnode.h>
#include <dm/read.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/mdio.h>

struct udevice;

#define PHY_FIXED_ID		0xa5a55a5a
#define PHY_NCSI_ID            0xbeefcafe

/*
 * There is no actual id for this.
 * This is just a dummy id for gmii2rgmmi converter.
 */
#define PHY_GMII2RGMII_ID	0x5a5a5a5a

#define PHY_MAX_ADDR 32

#define PHY_FLAG_BROKEN_RESET	(1 << 0) /* soft reset not supported */

#define PHY_DEFAULT_FEATURES	(SUPPORTED_Autoneg | \
				 SUPPORTED_TP | \
				 SUPPORTED_MII)

#define PHY_10BT_FEATURES	(SUPPORTED_10baseT_Half | \
				 SUPPORTED_10baseT_Full)

#define PHY_100BT_FEATURES	(SUPPORTED_100baseT_Half | \
				 SUPPORTED_100baseT_Full)

#define PHY_1000BT_FEATURES	(SUPPORTED_1000baseT_Half | \
				 SUPPORTED_1000baseT_Full)

#define PHY_BASIC_FEATURES	(PHY_10BT_FEATURES | \
				 PHY_100BT_FEATURES | \
				 PHY_DEFAULT_FEATURES)

#define PHY_GBIT_FEATURES	(PHY_BASIC_FEATURES | \
				 PHY_1000BT_FEATURES)

#define PHY_10G_FEATURES	(PHY_GBIT_FEATURES | \
				SUPPORTED_10000baseT_Full)

#ifndef PHY_ANEG_TIMEOUT
#define PHY_ANEG_TIMEOUT	4000
#endif


struct phy_device;

#define MDIO_NAME_LEN 32

struct mii_dev {
	struct list_head link;
	char name[MDIO_NAME_LEN];
	void *priv;
	int (*read)(struct mii_dev *bus, int addr, int devad, int reg);
	int (*write)(struct mii_dev *bus, int addr, int devad, int reg,
			u16 val);
	int (*reset)(struct mii_dev *bus);
	struct phy_device *phymap[PHY_MAX_ADDR];
	u32 phy_mask;
};

/* struct phy_driver: a structure which defines PHY behavior
 *
 * uid will contain a number which represents the PHY.  During
 * startup, the driver will poll the PHY to find out what its
 * UID--as defined by registers 2 and 3--is.  The 32-bit result
 * gotten from the PHY will be masked to
 * discard any bits which may change based on revision numbers
 * unimportant to functionality
 *
 */
struct phy_driver {
	char *name;
	unsigned int uid;
	unsigned int mask;
	unsigned int mmds;

	u32 features;

	/* Called to do any driver startup necessities */
	/* Will be called during phy_connect */
	int (*probe)(struct phy_device *phydev);

	/* Called to configure the PHY, and modify the controller
	 * based on the results.  Should be called after phy_connect */
	int (*config)(struct phy_device *phydev);

	/* Called when starting up the controller */
	int (*startup)(struct phy_device *phydev);

	/* Called when bringing down the controller */
	int (*shutdown)(struct phy_device *phydev);

	int (*readext)(struct phy_device *phydev, int addr, int devad, int reg);
	int (*writeext)(struct phy_device *phydev, int addr, int devad, int reg,
			u16 val);

	/* Phy specific driver override for reading a MMD register */
	int (*read_mmd)(struct phy_device *phydev, int devad, int reg);

	/* Phy specific driver override for writing a MMD register */
	int (*write_mmd)(struct phy_device *phydev, int devad, int reg,
			 u16 val);

	/* driver private data */
	ulong data;
};

struct phy_device {
	/* Information about the PHY type */
	/* And management functions */
	struct mii_dev *bus;
	struct phy_driver *drv;
	void *priv;

	struct udevice *dev;
	ofnode node;

	/* forced speed & duplex (no autoneg)
	 * partner speed & duplex & pause (autoneg)
	 */
	int speed;
	int duplex;

	/* The most recently read link state */
	int link;
	int port;
	phy_interface_t interface;

	u32 advertising;
	u32 supported;
	u32 mmds;

	int autoneg;
	int addr;
	int pause;
	int asym_pause;
	u32 phy_id;
	bool is_c45;
	u32 flags;
};

struct fixed_link {
	int phy_id;
	int duplex;
	int link_speed;
	int pause;
	int asym_pause;
};

/**
 * phy_reset() - Resets the specified PHY
 * Issues a reset of the PHY and waits for it to complete
 *
 * @phydev:	PHY to reset
 * @return: 0 if OK, -ve on error
 */
int phy_reset(struct phy_device *phydev);

/**
 * phy_find_by_mask() - Searches for a PHY on the specified MDIO bus
 * The function checks the PHY addresses flagged in phy_mask and returns a
 * phy_device pointer if it detects a PHY.
 * This function should only be called if just one PHY is expected to be present
 * in the set of addresses flagged in phy_mask.  If multiple PHYs are present,
 * it is undefined which of these PHYs is returned.
 *
 * @bus:	MII/MDIO bus to scan
 * @phy_mask:	bitmap of PYH addresses to scan
 * @return: pointer to phy_device if a PHY is found, or NULL otherwise
 */
struct phy_device *phy_find_by_mask(struct mii_dev *bus, unsigned phy_mask);

#ifdef CONFIG_PHY_FIXED

/**
 * fixed_phy_create() - create an unconnected fixed-link pseudo-PHY device
 * @node: OF node for the container of the fixed-link node
 *
 * Description: Creates a struct phy_device based on a fixed-link of_node
 * description. Can be used without phy_connect by drivers which do not expose
 * a UCLASS_ETH udevice.
 */
struct phy_device *fixed_phy_create(ofnode node);

#else

static inline struct phy_device *fixed_phy_create(ofnode node)
{
	return NULL;
}

#endif

/**
 * phy_connect() - Creates a PHY device for the Ethernet interface
 * Creates a PHY device for the PHY at the given address, if one doesn't exist
 * already, and associates it with the Ethernet device.
 * The function may be called with addr <= 0, in this case addr value is ignored
 * and the bus is scanned to detect a PHY.  Scanning should only be used if only
 * one PHY is expected to be present on the MDIO bus, otherwise it is undefined
 * which PHY is returned.
 *
 * @bus:	MII/MDIO bus that hosts the PHY
 * @addr:	PHY address on MDIO bus
 * @dev:	Ethernet device to associate to the PHY
 * @interface:	type of MAC-PHY interface
 * @return: pointer to phy_device if a PHY is found, or NULL otherwise
 */
struct phy_device *phy_connect(struct mii_dev *bus, int addr,
				struct udevice *dev,
				phy_interface_t interface);
/**
 * phy_device_create() - Create a PHY device
 *
 * @bus:		MII/MDIO bus that hosts the PHY
 * @addr:		PHY address on MDIO bus
 * @phy_id:		where to store the ID retrieved
 * @is_c45:		Device Identifiers if is_c45
 * @return: pointer to phy_device if a PHY is found, or NULL otherwise
 */
struct phy_device *phy_device_create(struct mii_dev *bus, int addr,
				     u32 phy_id, bool is_c45);

/**
 * phy_connect_phy_id() - Connect to phy device by reading PHY id
 *			  from phy node.
 *
 * @bus:		MII/MDIO bus that hosts the PHY
 * @dev:		Ethernet device to associate to the PHY
 * @return:		pointer to phy_device if a PHY is found,
 *			or NULL otherwise
 */
struct phy_device *phy_connect_phy_id(struct mii_dev *bus, struct udevice *dev,
				      int phyaddr);

static inline ofnode phy_get_ofnode(struct phy_device *phydev)
{
	if (ofnode_valid(phydev->node))
		return phydev->node;
	else
		return dev_ofnode(phydev->dev);
}

/**
 * phy_read_mmd_poll_timeout - Periodically poll a PHY register until a
 *                             condition is met or a timeout occurs
 *
 * @phydev: The phy_device struct
 * @devaddr: The MMD to read from
 * @regnum: The register on the MMD to read
 * @val: Variable to read the register into
 * @cond: Break condition (usually involving @val)
 * @sleep_us: Maximum time to sleep between reads in us (0
 *            tight-loops).  Should be less than ~20ms since usleep_range
 *            is used (see Documentation/timers/timers-howto.rst).
 * @timeout_us: Timeout in us, 0 means never timeout
 * @sleep_before_read: if it is true, sleep @sleep_us before read.
 * Returns 0 on success and -ETIMEDOUT upon a timeout. In either
 * case, the last read value at @args is stored in @val. Must not
 * be called from atomic context if sleep_us or timeout_us are used.
 */
#define phy_read_mmd_poll_timeout(phydev, devaddr, regnum, val, cond, \
				  sleep_us, timeout_us, sleep_before_read) \
({ \
	int __ret = read_poll_timeout(phy_read_mmd, val, (cond) || val < 0, \
				  sleep_us, timeout_us, \
				  phydev, devaddr, regnum); \
	if (val <  0) \
		__ret = val; \
	if (__ret) \
		dev_err(phydev->dev, "%s failed: %d\n", __func__, __ret); \
	__ret; \
})

int phy_read(struct phy_device *phydev, int devad, int regnum);
int phy_write(struct phy_device *phydev, int devad, int regnum, u16 val);
void phy_mmd_start_indirect(struct phy_device *phydev, int devad, int regnum);
int phy_read_mmd(struct phy_device *phydev, int devad, int regnum);
int phy_write_mmd(struct phy_device *phydev, int devad, int regnum, u16 val);
int phy_set_bits_mmd(struct phy_device *phydev, int devad, u32 regnum, u16 val);
int phy_clear_bits_mmd(struct phy_device *phydev, int devad, u32 regnum, u16 val);
int phy_modify_mmd_changed(struct phy_device *phydev, int devad, u32 regnum,
			   u16 mask, u16 set);
int phy_modify_mmd(struct phy_device *phydev, int devad, u32 regnum,
		   u16 mask, u16 set);

int phy_startup(struct phy_device *phydev);
int phy_config(struct phy_device *phydev);
int phy_shutdown(struct phy_device *phydev);
int phy_set_supported(struct phy_device *phydev, u32 max_speed);
int phy_modify(struct phy_device *phydev, int devad, int regnum, u16 mask,
	       u16 set);
int genphy_config_aneg(struct phy_device *phydev);
int genphy_restart_aneg(struct phy_device *phydev);
int genphy_update_link(struct phy_device *phydev);
int genphy_parse_link(struct phy_device *phydev);
int genphy_config(struct phy_device *phydev);
int genphy_startup(struct phy_device *phydev);
int genphy_shutdown(struct phy_device *phydev);
int gen10g_config(struct phy_device *phydev);
int gen10g_startup(struct phy_device *phydev);
int gen10g_shutdown(struct phy_device *phydev);
int gen10g_discover_mmds(struct phy_device *phydev);

/**
 * U_BOOT_PHY_DRIVER() - Declare a new U-Boot driver
 * @__name: name of the driver
 */
#define U_BOOT_PHY_DRIVER(__name)					\
	ll_entry_declare(struct phy_driver, __name, phy_driver)

int board_phy_config(struct phy_device *phydev);
int get_phy_id(struct mii_dev *bus, int addr, int devad, u32 *phy_id);

/**
 * phy_interface_is_rgmii - Convenience function for testing if a PHY interface
 * is RGMII (all variants)
 * @phydev: the phy_device struct
 * @return: true if MII bus is RGMII or false if it is not
 */
static inline bool phy_interface_is_rgmii(struct phy_device *phydev)
{
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		return 1;
	default:
		return 0;
	}
}

bool phy_interface_is_ncsi(void);

/* PHY UIDs for various PHYs that are referenced in external code */
#define PHY_UID_CS4340		0x13e51002
#define PHY_UID_CS4223		0x03e57003
#define PHY_UID_TN2020		0x00a19410
#define PHY_UID_IN112525_S03	0x02107440

#endif
