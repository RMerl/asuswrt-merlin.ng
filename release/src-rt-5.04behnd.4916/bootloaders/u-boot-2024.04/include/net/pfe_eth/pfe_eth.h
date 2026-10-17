/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __PFE_ETH_H__
#define __PFE_ETH_H__

#include <linux/bitops.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <miiphy.h>
#include <malloc.h>
#include "pfe_driver.h"

#define BMU2_DDR_BASEADDR	0
#define BMU2_BUF_COUNT		(3 * SZ_1K)
#define BMU2_DDR_SIZE		(DDR_BUF_SIZE * BMU2_BUF_COUNT)

#define HIF_RX_PKT_DDR_BASEADDR (BMU2_DDR_BASEADDR + BMU2_DDR_SIZE)
#define HIF_RX_PKT_DDR_SIZE     (HIF_RX_DESC_NT * DDR_BUF_SIZE)
#define HIF_TX_PKT_DDR_BASEADDR (HIF_RX_PKT_DDR_BASEADDR + HIF_RX_PKT_DDR_SIZE)
#define HIF_TX_PKT_DDR_SIZE     (HIF_TX_DESC_NT * DDR_BUF_SIZE)

#define HIF_DESC_BASEADDR       (HIF_TX_PKT_DDR_BASEADDR + HIF_TX_PKT_DDR_SIZE)
#define HIF_RX_DESC_SIZE        (16 * HIF_RX_DESC_NT)
#define HIF_TX_DESC_SIZE        (16 * HIF_TX_DESC_NT)

#define UTIL_CODE_BASEADDR	0x780000
#define UTIL_CODE_SIZE		(128 * SZ_1K)

#define UTIL_DDR_DATA_BASEADDR	(UTIL_CODE_BASEADDR + UTIL_CODE_SIZE)
#define UTIL_DDR_DATA_SIZE	(64 * SZ_1K)

#define CLASS_DDR_DATA_BASEADDR	(UTIL_DDR_DATA_BASEADDR + UTIL_DDR_DATA_SIZE)
#define CLASS_DDR_DATA_SIZE	(32 * SZ_1K)

#define TMU_DDR_DATA_BASEADDR	(CLASS_DDR_DATA_BASEADDR + CLASS_DDR_DATA_SIZE)
#define TMU_DDR_DATA_SIZE	(32 * SZ_1K)

#define TMU_LLM_BASEADDR	(TMU_DDR_DATA_BASEADDR + TMU_DDR_DATA_SIZE)
#define TMU_LLM_QUEUE_LEN	(16 * 256)
	/* Must be power of two and at least 16 * 8 = 128 bytes */
#define TMU_LLM_SIZE		(4 * 16 * TMU_LLM_QUEUE_LEN)
	/* (4 TMU's x 16 queues x queue_len) */

#define ROUTE_TABLE_BASEADDR	0x800000
#define ROUTE_TABLE_HASH_BITS_MAX	15 /* 32K entries */
#define ROUTE_TABLE_HASH_BITS		8  /* 256 entries */
#define ROUTE_TABLE_SIZE	(BIT(ROUTE_TABLE_HASH_BITS_MAX) \
				* CLASS_ROUTE_SIZE)

#define	PFE_TOTAL_DATA_SIZE	(ROUTE_TABLE_BASEADDR + ROUTE_TABLE_SIZE)

#if PFE_TOTAL_DATA_SIZE > (12 * SZ_1M)
#error DDR mapping above 12MiB
#endif

/* LMEM Mapping */
#define BMU1_LMEM_BASEADDR	0
#define BMU1_BUF_COUNT		256
#define BMU1_LMEM_SIZE		(LMEM_BUF_SIZE * BMU1_BUF_COUNT)

struct gemac_s {
	void *gemac_base;
	void *egpi_base;

	/* GEMAC config */
	int gemac_mode;
	int gemac_speed;
	int gemac_duplex;
	int flags;
	/* phy iface */
	int phy_address;
	int phy_mode;
	struct mii_dev *bus;

};

struct pfe_mdio_info {
	void *reg_base;
	char *name;
};

struct pfe_eth_dev {
	int gemac_port;
	struct gemac_s *gem;
	struct pfe_ddr_address pfe_addr;
	struct udevice *dev;
#ifdef CONFIG_PHYLIB
	struct phy_device *phydev;
#endif
};

int pfe_remove(struct pfe_ddr_address *pfe_addr);
struct mii_dev *pfe_mdio_init(struct pfe_mdio_info *mdio_info);
void pfe_set_mdio(int dev_id, struct mii_dev *bus);
void pfe_set_phy_address_mode(int dev_id, int phy_id, int phy_mode);
int gemac_initialize(struct bd_info *bis, int dev_id, char *devname);
int pfe_init(struct pfe_ddr_address *pfe_addr);
int pfe_eth_board_init(struct udevice *dev);

#endif /* __PFE_ETH_H__ */
