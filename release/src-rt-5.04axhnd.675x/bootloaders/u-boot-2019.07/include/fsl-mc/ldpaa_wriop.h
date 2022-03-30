/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor
 */

#ifndef __LDPAA_WRIOP_H
#define __LDPAA_WRIOP_H

#include <phy.h>

#define DEFAULT_WRIOP_MDIO1_NAME "FSL_MDIO0"
#define DEFAULT_WRIOP_MDIO2_NAME "FSL_MDIO1"
#define WRIOP_MAX_PHY_NUM        2

enum wriop_port {
	WRIOP1_DPMAC1 = 1,
	WRIOP1_DPMAC2,
	WRIOP1_DPMAC3,
	WRIOP1_DPMAC4,
	WRIOP1_DPMAC5,
	WRIOP1_DPMAC6,
	WRIOP1_DPMAC7,
	WRIOP1_DPMAC8,
	WRIOP1_DPMAC9,
	WRIOP1_DPMAC10,
	WRIOP1_DPMAC11,
	WRIOP1_DPMAC12,
	WRIOP1_DPMAC13,
	WRIOP1_DPMAC14,
	WRIOP1_DPMAC15,
	WRIOP1_DPMAC16,
	WRIOP1_DPMAC17,
	WRIOP1_DPMAC18,
	WRIOP1_DPMAC19,
	WRIOP1_DPMAC20,
	WRIOP1_DPMAC21,
	WRIOP1_DPMAC22,
	WRIOP1_DPMAC23,
	WRIOP1_DPMAC24,
	NUM_WRIOP_PORTS,
};

struct wriop_dpmac_info {
	u8 enabled;
	u8 id;
	u8 board_mux;
	int phy_addr[WRIOP_MAX_PHY_NUM];
	phy_interface_t enet_if;
	struct phy_device *phydev[WRIOP_MAX_PHY_NUM];
	struct mii_dev *bus;
};

extern struct wriop_dpmac_info dpmac_info[NUM_WRIOP_PORTS];

void wriop_init_dpmac(int sd, int dpmac_id, int lane_prtcl);
void wriop_init_dpmac_enet_if(int dpmac_id, phy_interface_t enet_if);
int wriop_disable_dpmac(int dpmac_id);
int wriop_enable_dpmac(int dpmac_id);
int wriop_is_enabled_dpmac(int dpmac_id);
int wriop_set_mdio(int dpmac_id, struct mii_dev *bus);
struct mii_dev *wriop_get_mdio(int dpmac_id);
int wriop_set_phy_address(int dpmac_id, int phy_num, int address);
int wriop_get_phy_address(int dpmac_id, int phy_num);
int wriop_set_phy_dev(int dpmac_id, int phy_num, struct phy_device *phydev);
struct phy_device *wriop_get_phy_dev(int dpmac_id, int phy_num);
phy_interface_t wriop_get_enet_if(int dpmac_id);

void wriop_dpmac_disable(int dpmac_id);
void wriop_dpmac_enable(int dpmac_id);
phy_interface_t wriop_dpmac_enet_if(int dpmac_id, int lane_prtcl);
void wriop_init_dpmac_qsgmii(int sd, int lane_prtcl);
void wriop_init_rgmii(void);
#endif	/* __LDPAA_WRIOP_H */
