/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Valentin Lontgchamp, Keymile AG, valentin.longchamp@keymile.com
 */

#ifndef __MV886352_H
#define __MV886352_H

#include <common.h>

/* PHY registers */
#define PHY(itf)	(itf)

#define PHY_CTRL	0x00
#define PHY_100_MBPS	0x2000
#define PHY_1_GBPS	0x0040
#define AUTONEG_EN	0x1000
#define AUTONEG_RST	0x0200
#define FULL_DUPLEX	0x0100
#define PHY_PWR_DOWN	0x0800

#define PHY_STATUS	0x01
#define AN1000FIX	0x0001

#define PHY_SPEC_CTRL	0x10
#define SPEC_PWR_DOWN	0x0004
#define AUTO_MDIX_EN	0x0060

#define PHY_1000_CTRL	0x9

#define NO_ADV		0x0000
#define ADV_1000_FDPX	0x0200
#define ADV_1000_HDPX	0x0100

#define PHY_PAGE	0x16

#define AN1000FIX_PAGE	0x00fc

/* PORT or MAC registers */
#define PORT(itf)	(itf+0x10)

#define PORT_STATUS	0x00
#define NO_PHY_DETECT	0x0000

#define PORT_PHY	0x01
#define RX_RGMII_TIM	0x8000
#define TX_RGMII_TIM	0x4000
#define FLOW_CTRL_EN	0x0080
#define FLOW_CTRL_FOR	0x0040
#define LINK_VAL	0x0020
#define LINK_FOR	0x0010
#define FULL_DPX	0x0008
#define FULL_DPX_FOR	0x0004
#define NO_SPEED_FOR	0x0003
#define SPEED_1000_FOR	0x0002
#define SPEED_100_FOR	0x0001
#define SPEED_10_FOR	0x0000

#define PORT_CTRL	0x04
#define FORWARDING	0x0003
#define EGRS_FLD_ALL	0x000c
#define PORT_DIS	0x0000

struct mv88e_sw_reg {
	u8 port;
	u8 reg;
	u16 value;
};

int mv88e_sw_reset(const char *devname, u8 phy_addr);
int mv88e_sw_program(const char *devname, u8 phy_addr,
	struct mv88e_sw_reg *regs, int regs_nb);

#endif
