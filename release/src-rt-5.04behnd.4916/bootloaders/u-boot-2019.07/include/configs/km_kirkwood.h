/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2011-2012
 * Holger Brunck, Keymile GmbH Hannover, holger.brunck@keymile.com
 * Valentin Longchamp, Keymile AG, valentin.longchamp@keymile.com
 */

/*
 * for linking errors see
 * http://lists.denx.de/pipermail/u-boot/2009-July/057350.html
 */

#ifndef _CONFIG_KM_KIRKWOOD_H
#define _CONFIG_KM_KIRKWOOD_H

/* KM_KIRKWOOD */
#if defined(CONFIG_KM_KIRKWOOD)
#define CONFIG_HOSTNAME			"km_kirkwood"
#define CONFIG_KM_DISABLE_PCIE
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/

/* KM_KIRKWOOD_PCI */
#elif defined(CONFIG_KM_KIRKWOOD_PCI)
#define CONFIG_HOSTNAME			"km_kirkwood_pci"
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/
#define CONFIG_KM_FPGA_CONFIG
#define CONFIG_KM_UBI_PART_BOOT_OPTS		",2048"
#define CONFIG_SYS_NAND_NO_SUBPAGE_WRITE

/* KM_KIRKWOOD_128M16 */
#elif defined(CONFIG_KM_KIRKWOOD_128M16)
#define CONFIG_HOSTNAME			"km_kirkwood_128m16"
#undef CONFIG_SYS_KWD_CONFIG
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage_128M16_1.cfg
#define CONFIG_KM_DISABLE_PCIE
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/

/* KM_NUSA / KM_SUGP1 */
#elif defined(CONFIG_KM_NUSA) || defined(CONFIG_KM_SUGP1)
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/

# if defined(CONFIG_KM_NUSA)
#define CONFIG_HOSTNAME			"kmnusa"
# elif defined(CONFIG_KM_SUGP1)
#define CONFIG_HOSTNAME			"kmsugp1"
#define KM_PCIE_RESET_MPP7
#endif

#undef CONFIG_SYS_KWD_CONFIG
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage_128M16_1.cfg
#define CONFIG_KM_ENV_IS_IN_SPI_NOR
#define CONFIG_KM_FPGA_CONFIG
#define CONFIG_KM_PIGGY4_88E6352
#define CONFIG_MV88E6352_SWITCH
#define CONFIG_KM_MVEXTSW_ADDR		0x10

/* KM_MGCOGE3UN */
#elif defined(CONFIG_KM_MGCOGE3UN)
#define CONFIG_HOSTNAME			"mgcoge3un"
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/
#undef CONFIG_SYS_KWD_CONFIG
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage-memphis.cfg
#define CONFIG_KM_BOARD_EXTRA_ENV	"waitforne=true\0"
#define CONFIG_PIGGY_MAC_ADRESS_OFFSET  3
#define CONFIG_KM_DISABLE_PCIE
#define CONFIG_KM_PIGGY4_88E6061

/* KMCOGE5UN */
#elif defined(CONFIG_KM_COGE5UN)
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/
#undef	CONFIG_SYS_KWD_CONFIG
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage_256M8_1.cfg
#define CONFIG_KM_ENV_IS_IN_SPI_NOR
#define CONFIG_PIGGY_MAC_ADRESS_OFFSET	3
#define CONFIG_HOSTNAME			"kmcoge5un"
#define CONFIG_KM_DISABLE_PCIE
#define CONFIG_KM_PIGGY4_88E6352

/* KM_PORTL2 */
#elif defined(CONFIG_KM_PORTL2)
#define CONFIG_HOSTNAME			"portl2"
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/
#define CONFIG_KM_PIGGY4_88E6061

/* KM_SUV31 */
#elif defined(CONFIG_KM_SUV31)
#define CONFIG_KM_IVM_BUS		1	/* I2C2 (Mux-Port 1)*/
#define CONFIG_HOSTNAME			"kmsuv31"
#undef CONFIG_SYS_KWD_CONFIG
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage_128M16_1.cfg
#define CONFIG_KM_ENV_IS_IN_SPI_NOR
#define CONFIG_KM_FPGA_CONFIG
#define CONFIG_KM_UBI_PART_BOOT_OPTS		",2048"
#define CONFIG_SYS_NAND_NO_SUBPAGE_WRITE
#else
#error ("Board unsupported")
#endif

/* include common defines/options for all arm based Keymile boards */
#include "km/km_arm.h"

#if defined(CONFIG_KM_PIGGY4_88E6352)
/*
 * Some keymile boards like mgcoge5un & nusa1 have their PIGGY4 connected via
 * an Marvell 88E6352 simple switch.
 * In this case we have to change the default settings for the etherent mac.
 * There is NO ethernet phy. The ARM and Switch are conencted directly over
 * RGMII in MAC-MAC mode
 * In this case 1GBit full duplex and autoneg off
 */
#define PORT_SERIAL_CONTROL_VALUE		( \
	MVGBE_FORCE_LINK_PASS			    | \
	MVGBE_DIS_AUTO_NEG_FOR_DUPLX		| \
	MVGBE_DIS_AUTO_NEG_FOR_FLOW_CTRL	| \
	MVGBE_ADV_NO_FLOW_CTRL			    | \
	MVGBE_FORCE_FC_MODE_NO_PAUSE_DIS_TX	| \
	MVGBE_FORCE_BP_MODE_NO_JAM		    | \
	(1 << 9) /* Reserved bit has to be 1 */	| \
	MVGBE_DO_NOT_FORCE_LINK_FAIL		| \
	MVGBE_DIS_AUTO_NEG_SPEED_GMII		| \
	MVGBE_DTE_ADV_0				        | \
	MVGBE_MIIPHY_MAC_MODE			    | \
	MVGBE_AUTO_NEG_NO_CHANGE		    | \
	MVGBE_MAX_RX_PACKET_1552BYTE		| \
	MVGBE_CLR_EXT_LOOPBACK			    | \
	MVGBE_SET_FULL_DUPLEX_MODE		    | \
	MVGBE_EN_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX	|\
	MVGBE_SET_GMII_SPEED_TO_1000	    |\
	MVGBE_SET_MII_SPEED_TO_100)

#endif

#ifdef CONFIG_KM_PIGGY4_88E6061
/*
 * Some keymile boards like mgcoge3un have their PIGGY4 connected via
 * an Marvell 88E6061 simple switch.
 * In this case we have to change the default settings for the
 * ethernet phy connected to the kirkwood.
 * In this case 100MB full duplex and autoneg off
 */
#define PORT_SERIAL_CONTROL_VALUE		( \
	MVGBE_FORCE_LINK_PASS			| \
	MVGBE_DIS_AUTO_NEG_FOR_DUPLX		| \
	MVGBE_DIS_AUTO_NEG_FOR_FLOW_CTRL	| \
	MVGBE_ADV_NO_FLOW_CTRL			| \
	MVGBE_FORCE_FC_MODE_NO_PAUSE_DIS_TX	| \
	MVGBE_FORCE_BP_MODE_NO_JAM		| \
	(1 << 9) /* Reserved bit has to be 1 */	| \
	MVGBE_DO_NOT_FORCE_LINK_FAIL		| \
	MVGBE_DIS_AUTO_NEG_SPEED_GMII		| \
	MVGBE_DTE_ADV_0				| \
	MVGBE_MIIPHY_MAC_MODE			| \
	MVGBE_AUTO_NEG_NO_CHANGE		| \
	MVGBE_MAX_RX_PACKET_1552BYTE		| \
	MVGBE_CLR_EXT_LOOPBACK			| \
	MVGBE_SET_FULL_DUPLEX_MODE		| \
	MVGBE_DIS_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX	|\
	MVGBE_SET_GMII_SPEED_TO_10_100	|\
	MVGBE_SET_MII_SPEED_TO_100)
#endif

#ifdef CONFIG_KM_DISABLE_PCI
#undef  CONFIG_KIRKWOOD_PCIE_INIT
#endif

#endif /* _CONFIG_KM_KIRKWOOD */
