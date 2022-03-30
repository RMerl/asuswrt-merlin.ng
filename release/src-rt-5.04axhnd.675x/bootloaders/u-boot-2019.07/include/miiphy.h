/* SPDX-License-Identifier: GPL-2.0 OR IBM-pibs */
/*
 * Additions (C) Copyright 2009 Industrie Dial Face S.p.A.
 */
/*----------------------------------------------------------------------------+
|
|  File Name:	miiphy.h
|
|  Function:	Include file defining PHY registers.
|
|  Author:	Mark Wisner
|
+----------------------------------------------------------------------------*/
#ifndef _miiphy_h_
#define _miiphy_h_

#include <common.h>
#include <linux/mii.h>
#include <linux/list.h>
#include <net.h>
#include <phy.h>

int miiphy_read(const char *devname, unsigned char addr, unsigned char reg,
		 unsigned short *value);
int miiphy_write(const char *devname, unsigned char addr, unsigned char reg,
		  unsigned short value);
int miiphy_info(const char *devname, unsigned char addr, unsigned int *oui,
		 unsigned char *model, unsigned char *rev);
int miiphy_reset(const char *devname, unsigned char addr);
int miiphy_speed(const char *devname, unsigned char addr);
int miiphy_duplex(const char *devname, unsigned char addr);
int miiphy_is_1000base_x(const char *devname, unsigned char addr);
#ifdef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
int miiphy_link(const char *devname, unsigned char addr);
#endif

void miiphy_init(void);

int miiphy_set_current_dev(const char *devname);
const char *miiphy_get_current_dev(void);
struct mii_dev *mdio_get_current_dev(void);
struct list_head *mdio_get_list_head(void);
struct mii_dev *miiphy_get_dev_by_name(const char *devname);
struct phy_device *mdio_phydev_for_ethname(const char *devname);

void miiphy_listdev(void);

struct mii_dev *mdio_alloc(void);
void mdio_free(struct mii_dev *bus);
int mdio_register(struct mii_dev *bus);

/**
 * mdio_register_seq - Register mdio bus with sequence number
 * @bus: mii device structure
 * @seq: sequence number
 *
 * Return: 0 if success, negative value if error
 */
int mdio_register_seq(struct mii_dev *bus, int seq);
int mdio_unregister(struct mii_dev *bus);
void mdio_list_devices(void);

#ifdef CONFIG_BITBANGMII

#define BB_MII_DEVNAME	"bb_miiphy"

struct bb_miiphy_bus {
	char name[16];
	int (*init)(struct bb_miiphy_bus *bus);
	int (*mdio_active)(struct bb_miiphy_bus *bus);
	int (*mdio_tristate)(struct bb_miiphy_bus *bus);
	int (*set_mdio)(struct bb_miiphy_bus *bus, int v);
	int (*get_mdio)(struct bb_miiphy_bus *bus, int *v);
	int (*set_mdc)(struct bb_miiphy_bus *bus, int v);
	int (*delay)(struct bb_miiphy_bus *bus);
#ifdef CONFIG_BITBANGMII_MULTI
	void *priv;
#endif
};

extern struct bb_miiphy_bus bb_miiphy_buses[];
extern int bb_miiphy_buses_num;

void bb_miiphy_init(void);
int bb_miiphy_read(struct mii_dev *miidev, int addr, int devad, int reg);
int bb_miiphy_write(struct mii_dev *miidev, int addr, int devad, int reg,
		    u16 value);
#endif

/* phy seed setup */
#define AUTO			99
#define _1000BASET		1000
#define _100BASET		100
#define _10BASET		10
#define HALF			22
#define FULL			44

/* phy register offsets */
#define MII_MIPSCR		0x11

/* MII_LPA */
#define PHY_ANLPAR_PSB_802_3	0x0001
#define PHY_ANLPAR_PSB_802_9	0x0002

/* MII_CTRL1000 masks */
#define PHY_1000BTCR_1000FD	0x0200
#define PHY_1000BTCR_1000HD	0x0100

/* MII_STAT1000 masks */
#define PHY_1000BTSR_MSCF	0x8000
#define PHY_1000BTSR_MSCR	0x4000
#define PHY_1000BTSR_LRS	0x2000
#define PHY_1000BTSR_RRS	0x1000
#define PHY_1000BTSR_1000FD	0x0800
#define PHY_1000BTSR_1000HD	0x0400

/* phy EXSR */
#define ESTATUS_1000XF		0x8000
#define ESTATUS_1000XH		0x4000

#endif
