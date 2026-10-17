/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#ifndef __SOC_ROCKCHIP_RK3588_GRF_H__
#define __SOC_ROCKCHIP_RK3588_GRF_H__

struct rk3588_pmu1grf {
	unsigned int soc_con[12];
	unsigned int reserved0[(0x0050 - 0x002c) / 4 - 1];
	unsigned int biu_con;
	unsigned int biu_sts;
	unsigned int reserved1[(0x0060 - 0x0054) / 4 - 1];
	unsigned int soc_sts;
	unsigned int reserved2[(0x0080 - 0x0060) / 4 - 1];
	unsigned int mem_con[4];
	unsigned int reserved3[(0x0200 - 0x008c) / 4 - 1];
	unsigned int os_reg[8];
	unsigned int reserved4[(0x0230 - 0x021c) / 4 - 1];
	unsigned int rst_sts;
	unsigned int rst_clr;
	unsigned int reserved5[(0x0380 - 0x0234) / 4 - 1];
	unsigned int sd_detect_con;
	unsigned int reserved6[(0x0390 - 0x0380) / 4 - 1];
	unsigned int sd_detect_sts;
	unsigned int reserved7[(0x03a0 - 0x0390) / 4 - 1];
	unsigned int sd_detect_clr;
	unsigned int reserved8[(0x03b0 - 0x03a0) / 4 - 1];
	unsigned int sd_detect_cnt;
};

check_member(rk3588_pmu1grf, sd_detect_cnt, 0x03b0);

#endif /*__SOC_ROCKCHIP_RK3588_GRF_H__ */
