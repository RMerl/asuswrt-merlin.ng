/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __SOC_ROCKCHIP_RK3328_GRF_H__
#define __SOC_ROCKCHIP_RK3328_GRF_H__

struct rk3328_grf_regs {
	u32 gpio0a_iomux;
	u32 gpio0b_iomux;
	u32 gpio0c_iomux;
	u32 gpio0d_iomux;
	u32 gpio1a_iomux;
	u32 gpio1b_iomux;
	u32 gpio1c_iomux;
	u32 gpio1d_iomux;
	u32 gpio2a_iomux;
	u32 gpio2bl_iomux;
	u32 gpio2bh_iomux;
	u32 gpio2cl_iomux;
	u32 gpio2ch_iomux;
	u32 gpio2d_iomux;
	u32 gpio3al_iomux;
	u32 gpio3ah_iomux;
	u32 gpio3bl_iomux;
	u32 gpio3bh_iomux;
	u32 gpio3c_iomux;
	u32 gpio3d_iomux;
	u32 com_iomux;
	u32 reserved1[(0x100 - 0x54) / 4];

	u32 gpio0a_p;
	u32 gpio0b_p;
	u32 gpio0c_p;
	u32 gpio0d_p;
	u32 gpio1a_p;
	u32 gpio1b_p;
	u32 gpio1c_p;
	u32 gpio1d_p;
	u32 gpio2a_p;
	u32 gpio2b_p;
	u32 gpio2c_p;
	u32 gpio2d_p;
	u32 gpio3a_p;
	u32 gpio3b_p;
	u32 gpio3c_p;
	u32 gpio3d_p;
	u32 reserved2[(0x200 - 0x140) / 4];
	u32 gpio0a_e;
	u32 gpio0b_e;
	u32 gpio0c_e;
	u32 gpio0d_e;
	u32 gpio1a_e;
	u32 gpio1b_e;
	u32 gpio1c_e;
	u32 gpio1d_e;
	u32 gpio2a_e;
	u32 gpio2b_e;
	u32 gpio2c_e;
	u32 gpio2d_e;
	u32 gpio3a_e;
	u32 gpio3b_e;
	u32 gpio3c_e;
	u32 gpio3d_e;
	u32 reserved3[(0x300 - 0x240) / 4];
	u32 gpio0l_sr;
	u32 gpio0h_sr;
	u32 gpio1l_sr;
	u32 gpio1h_sr;
	u32 gpio2l_sr;
	u32 gpio2h_sr;
	u32 gpio3l_sr;
	u32 gpio3h_sr;
	u32 reserved4[(0x380 - 0x320) / 4];
	u32 gpio0l_smt;
	u32 gpio0h_smt;
	u32 gpio1l_smt;
	u32 gpio1h_smt;
	u32 gpio2l_smt;
	u32 gpio2h_smt;
	u32 gpio3l_smt;
	u32 gpio3h_smt;
	u32 reserved5[(0x400 - 0x3a0) / 4];
	u32 soc_con[11];
	u32 reserved6[(0x480 - 0x42c) / 4];
	u32 soc_status[5];
	u32 reserved7[(0x4c0 - 0x494) / 4];
	u32 otg3_con[2];
	u32 reserved8[(0x500 - 0x4c8) / 4];
	u32 cpu_con[2];
	u32 reserved9[(0x520 - 0x508) / 4];
	u32 cpu_status[2];
	u32 reserved10[(0x5c8 - 0x528) / 4];
	u32 os_reg[8];
	u32 reserved11[(0x680 - 0x5e8) / 4];
	u32 sig_detect_con;
	u32 reserved12[3];
	u32 sig_detect_status;
	u32 reserved13[3];
	u32 sig_detect_status_clr;
	u32 reserved14[3];

	u32 sdmmc_det_counter;
	u32 reserved15[(0x700 - 0x6b4) / 4];
	u32 host0_con[3];
	u32 reserved16[(0x880 - 0x70c) / 4];
	u32 otg_con0;
	u32 reserved17[3];
	u32 host0_status;
	u32 reserved18[(0x900 - 0x894) / 4];
	u32 mac_con[3];
	u32 reserved19[(0xb00 - 0x90c) / 4];
	u32 macphy_con[4];
	u32 macphy_status;
};
check_member(rk3328_grf_regs, macphy_status, 0xb10);

struct rk3328_sgrf_regs {
	u32 soc_con[6];
	u32 reserved0[(0x100 - 0x18) / 4];
	u32 dmac_con[6];
	u32 reserved1[(0x180 - 0x118) / 4];
	u32 fast_boot_addr;
	u32 reserved2[(0x200 - 0x184) / 4];
	u32 chip_fuse_con;
	u32 reserved3[(0x280 - 0x204) / 4];
	u32 hdcp_key_reg[8];
	u32 hdcp_key_access_mask;
};
check_member(rk3328_sgrf_regs, hdcp_key_access_mask, 0x2a0);

#endif	/* __SOC_ROCKCHIP_RK3328_GRF_H__ */
