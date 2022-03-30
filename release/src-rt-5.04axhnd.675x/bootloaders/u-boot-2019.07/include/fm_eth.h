/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP
 */

#ifndef __FM_ETH_H__
#define __FM_ETH_H__

#include <common.h>
#include <phy.h>
#include <asm/types.h>

enum fm_port {
	FM1_DTSEC1,
	FM1_DTSEC2,
	FM1_DTSEC3,
	FM1_DTSEC4,
	FM1_DTSEC5,
	FM1_DTSEC6,
	FM1_DTSEC9,
	FM1_DTSEC10,
	FM1_10GEC1,
	FM1_10GEC2,
	FM1_10GEC3,
	FM1_10GEC4,
	FM2_DTSEC1,
	FM2_DTSEC2,
	FM2_DTSEC3,
	FM2_DTSEC4,
	FM2_DTSEC5,
	FM2_DTSEC6,
	FM2_DTSEC9,
	FM2_DTSEC10,
	FM2_10GEC1,
	FM2_10GEC2,
	NUM_FM_PORTS,
};

enum fm_eth_type {
	FM_ETH_1G_E,
	FM_ETH_10G_E,
};

/* Historically, on FMan v3 platforms, the first MDIO bus has been used for
 * Clause 22 PHYs and the second MDIO bus for 10G Clause 45 PHYs (thus the
 * TGEC name).
 *
 * On LS1046A-FRWY, the QSGMII PHY is connected to the second MDIO bus,
 * and no TGEC ports are present on-board.
 */
#ifdef CONFIG_SYS_FMAN_V3
#ifdef CONFIG_TARGET_LS1046AFRWY
#define CONFIG_SYS_FM1_DTSEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xfd000)
#else
#define CONFIG_SYS_FM1_DTSEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xfc000)
#endif
#define CONFIG_SYS_FM1_TGEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xfd000)
#if (CONFIG_SYS_NUM_FMAN == 2)
#define CONFIG_SYS_FM2_DTSEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM2_ADDR + 0xfc000)
#define CONFIG_SYS_FM2_TGEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM2_ADDR + 0xfd000)
#endif
#else
#define CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xe1120)
#define CONFIG_SYS_FM1_TGEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xf1000)
#endif

#define DEFAULT_FM_MDIO_NAME "FSL_MDIO0"
#define DEFAULT_FM_TGEC_MDIO_NAME "FM_TGEC_MDIO"

/* Fman ethernet info struct */
#define FM_ETH_INFO_INITIALIZER(idx, pregs) \
	.fm		= idx,						\
	.phy_regs	= (void *)pregs,				\
	.enet_if	= PHY_INTERFACE_MODE_NONE,			\

#ifdef CONFIG_SYS_FMAN_V3
#define FM_DTSEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_DTSEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_1G_E,					\
	.port		= FM##idx##_DTSEC##n,				\
	.rx_port_id	= RX_PORT_1G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_1G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, memac[n-1]),\
}

#ifdef CONFIG_FSL_FM_10GEC_REGULAR_NOTATION
#define FM_TGEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_TGEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_10G_E,					\
	.port		= FM##idx##_10GEC##n,				\
	.rx_port_id	= RX_PORT_10G_BASE2 + n - 1,			\
	.tx_port_id	= TX_PORT_10G_BASE2 + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				 offsetof(struct ccsr_fman, memac[n-1]),\
}
#else
#if (CONFIG_SYS_NUM_FMAN == 2)
#define FM_TGEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM2_TGEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_10G_E,					\
	.port		= FM##idx##_10GEC##n,				\
	.rx_port_id	= RX_PORT_10G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_10G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, memac[n-1+8]),\
}
#else
#define FM_TGEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_TGEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_10G_E,					\
	.port		= FM##idx##_10GEC##n,				\
	.rx_port_id	= RX_PORT_10G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_10G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, memac[n-1+8]),\
}
#endif
#endif

#if (CONFIG_SYS_NUM_FM1_10GEC >= 3)
#define FM_TGEC_INFO_INITIALIZER2(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_TGEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_10G_E,					\
	.port		= FM##idx##_10GEC##n,				\
	.rx_port_id	= RX_PORT_10G_BASE2 + n - 3,			\
	.tx_port_id	= TX_PORT_10G_BASE2 + n - 3,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, memac[n-1-2]),\
}
#endif

#else
#define FM_DTSEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_1G_E,					\
	.port		= FM##idx##_DTSEC##n,				\
	.rx_port_id	= RX_PORT_1G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_1G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, mac_1g[n-1]),\
}

#define FM_TGEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_TGEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_10G_E,					\
	.port		= FM##idx##_10GEC##n,				\
	.rx_port_id	= RX_PORT_10G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_10G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, mac_10g[n-1]),\
}
#endif
struct fm_eth_info {
	u8 enabled;
	u8 fm;
	u8 num;
	u8 phy_addr;
	int index;
	u16 rx_port_id;
	u16 tx_port_id;
	enum fm_port port;
	enum fm_eth_type type;
	void *phy_regs;
	phy_interface_t enet_if;
	u32 compat_offset;
	struct mii_dev *bus;
};

struct tgec_mdio_info {
	struct tgec_mdio_controller *regs;
	char *name;
};

struct memac_mdio_info {
	struct memac_mdio_controller *regs;
	char *name;
};

int fm_tgec_mdio_init(bd_t *bis, struct tgec_mdio_info *info);
int fm_memac_mdio_init(bd_t *bis, struct memac_mdio_info *info);

int fm_standard_init(bd_t *bis);
void fman_enet_init(void);
void fdt_fixup_fman_ethernet(void *fdt);
phy_interface_t fm_info_get_enet_if(enum fm_port port);
void fm_info_set_phy_address(enum fm_port port, int address);
int fm_info_get_phy_address(enum fm_port port);
void fm_info_set_mdio(enum fm_port port, struct mii_dev *bus);
void fm_disable_port(enum fm_port port);
void fm_enable_port(enum fm_port port);
void set_sgmii_phy(struct mii_dev *bus, enum fm_port base_port,
		unsigned int port_num, int phy_base_addr);
int is_qsgmii_riser_card(struct mii_dev *bus, int phy_base_addr,
		unsigned int port_num, unsigned regnum);

#endif
