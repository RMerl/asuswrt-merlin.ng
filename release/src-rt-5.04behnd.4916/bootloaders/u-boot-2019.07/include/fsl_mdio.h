/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2012, 2013 Freescale Semiconductor, Inc.
 *	Jun-jie Zhang <b18070@freescale.com>
 *	Mingkai Hu <Mingkai.hu@freescale.com>
 */

#ifndef __FSL_PHY_H__
#define __FSL_PHY_H__

#include <net.h>
#include <miiphy.h>

struct tsec_mii_mng {
	u32 miimcfg;		/* MII management configuration reg */
	u32 miimcom;		/* MII management command reg */
	u32 miimadd;		/* MII management address reg */
	u32 miimcon;		/* MII management control reg */
	u32 miimstat;		/* MII management status reg  */
	u32 miimind;		/* MII management indication reg */
	u32 ifstat;		/* Interface Status Register */
};

int fdt_fixup_phy_connection(void *blob, int offset, phy_interface_t phyc);

/* PHY register offsets */
#define PHY_EXT_PAGE_ACCESS	0x1f

/* MII Management Configuration Register */
#define MIIMCFG_RESET_MGMT		0x80000000
#define MIIMCFG_MGMT_CLOCK_SELECT	0x00000007
#define MIIMCFG_INIT_VALUE		0x00000003

/* MII Management Command Register */
#define MIIMCOM_READ_CYCLE	0x00000001
#define MIIMCOM_SCAN_CYCLE	0x00000002

/* MII Management Address Register */
#define MIIMADD_PHY_ADDR_SHIFT	8

/* MII Management Indicator Register */
#define MIIMIND_BUSY		0x00000001
#define MIIMIND_NOTVALID	0x00000004

void tsec_local_mdio_write(struct tsec_mii_mng __iomem *phyregs, int port_addr,
		int dev_addr, int reg, int value);
int tsec_local_mdio_read(struct tsec_mii_mng __iomem *phyregs, int port_addr,
		int dev_addr, int regnum);
int tsec_phy_read(struct mii_dev *bus, int addr, int dev_addr, int regnum);
int tsec_phy_write(struct mii_dev *bus, int addr, int dev_addr, int regnum,
		u16 value);
int memac_mdio_write(struct mii_dev *bus, int port_addr, int dev_addr,
		int regnum, u16 value);
int memac_mdio_read(struct mii_dev *bus, int port_addr, int dev_addr,
		int regnum);

struct fsl_pq_mdio_info {
	struct tsec_mii_mng __iomem *regs;
	char *name;
};
int fsl_pq_mdio_init(bd_t *bis, struct fsl_pq_mdio_info *info);

#endif /* __FSL_PHY_H__ */
