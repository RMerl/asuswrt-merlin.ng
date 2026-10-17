/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd.
 */
#ifndef _ASM_ARCH_GRF_RK322X_H
#define _ASM_ARCH_GRF_RK322X_H

struct rk322x_grf {
	unsigned int gpio0a_iomux;
	unsigned int gpio0b_iomux;
	unsigned int gpio0c_iomux;
	unsigned int gpio0d_iomux;

	unsigned int gpio1a_iomux;
	unsigned int gpio1b_iomux;
	unsigned int gpio1c_iomux;
	unsigned int gpio1d_iomux;

	unsigned int gpio2a_iomux;
	unsigned int gpio2b_iomux;
	unsigned int gpio2c_iomux;
	unsigned int gpio2d_iomux;

	unsigned int gpio3a_iomux;
	unsigned int gpio3b_iomux;
	unsigned int gpio3c_iomux;
	unsigned int gpio3d_iomux;

	unsigned int reserved1[4];
	unsigned int con_iomux;
	unsigned int reserved2[(0x100 - 0x50) / 4 - 1];
	unsigned int gpio0_p[4];
	unsigned int gpio1_p[4];
	unsigned int gpio2_p[4];
	unsigned int gpio3_p[4];
	unsigned int reserved3[(0x200 - 0x13c) / 4 - 1];
	unsigned int gpio0_e[4];
	unsigned int gpio1_e[4];
	unsigned int gpio2_e[4];
	unsigned int gpio3_e[4];
	unsigned int reserved4[(0x400 - 0x23c) / 4 - 1];
	unsigned int soc_con[7];
	unsigned int reserved5[(0x480 - 0x418) / 4 - 1];
	unsigned int soc_status[3];
	unsigned int chip_id;
	unsigned int reserved6[(0x500 - 0x48c) / 4 - 1];
	unsigned int cpu_con[4];
	unsigned int reserved7[4];
	unsigned int cpu_status[2];
	unsigned int reserved8[(0x5c8 - 0x524) / 4 - 1];
	unsigned int os_reg[8];
	unsigned int reserved9[(0x604 - 0x5e4) / 4 - 1];
	unsigned int ddrc_stat;
	unsigned int reserved10[(0x680 - 0x604) / 4 - 1];
	unsigned int sig_detect_con[2];
	unsigned int reserved11[(0x690 - 0x684) / 4 - 1];
	unsigned int sig_detect_status[2];
	unsigned int reserved12[(0x6a0 - 0x694) / 4 - 1];
	unsigned int sig_detect_clr[2];
	unsigned int reserved13[(0x6b0 - 0x6a4) / 4 - 1];
	unsigned int emmc_det;
	unsigned int reserved14[(0x700 - 0x6b0) / 4 - 1];
	unsigned int host0_con[3];
	unsigned int reserved15;
	unsigned int host1_con[3];
	unsigned int reserved16;
	unsigned int host2_con[3];
	unsigned int reserved17[(0x760 - 0x728) / 4 - 1];
	unsigned int usbphy0_con[27];
	unsigned int reserved18[(0x800 - 0x7c8) / 4 - 1];
	unsigned int usbphy1_con[27];
	unsigned int reserved19[(0x880 - 0x868) / 4 - 1];
	unsigned int otg_con0;
	unsigned int uoc_status0;
	unsigned int reserved20[(0x900 - 0x884) / 4 - 1];
	unsigned int mac_con[2];
	unsigned int reserved21[(0xb00 - 0x904) / 4 - 1];
	unsigned int macphy_con[4];
	unsigned int macphy_status;
};
check_member(rk322x_grf, ddrc_stat, 0x604);

struct rk322x_sgrf {
	unsigned int soc_con[11];
	unsigned int busdmac_con[4];
};

/* GRF_MACPHY_CON0 */
enum {
	MACPHY_CFG_ENABLE_SHIFT = 0,
	MACPHY_CFG_ENABLE_MASK  = 1 << MACPHY_CFG_ENABLE_SHIFT,
};
#endif
