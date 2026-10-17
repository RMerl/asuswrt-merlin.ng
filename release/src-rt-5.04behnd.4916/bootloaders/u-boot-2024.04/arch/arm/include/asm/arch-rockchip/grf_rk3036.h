/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */
#ifndef _ASM_ARCH_GRF_RK3036_H
#define _ASM_ARCH_GRF_RK3036_H

struct rk3036_grf {
	unsigned int reserved[0x2a];
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

	unsigned int reserved2[0x0a];
	unsigned int gpiods;
	unsigned int reserved3[0x05];
	unsigned int gpio0l_pull;
	unsigned int gpio0h_pull;
	unsigned int gpio1l_pull;
	unsigned int gpio1h_pull;
	unsigned int gpio2l_pull;
	unsigned int gpio2h_pull;
	unsigned int reserved4[4];
	unsigned int soc_con0;
	unsigned int soc_con1;
	unsigned int soc_con2;
	unsigned int soc_status0;
	unsigned int reserved5;
	unsigned int soc_con3;
	unsigned int reserved6;
	unsigned int dmac_con0;
	unsigned int dmac_con1;
	unsigned int dmac_con2;
	unsigned int reserved7[5];
	unsigned int uoc0_con5;
	unsigned int reserved8[4];
	unsigned int uoc1_con4;
	unsigned int uoc1_con5;
	unsigned int reserved9;
	unsigned int ddrc_stat;
	unsigned int uoc_con6;
	unsigned int soc_status1;
	unsigned int cpu_con0;
	unsigned int cpu_con1;
	unsigned int cpu_con2;
	unsigned int cpu_con3;
	unsigned int reserved10;
	unsigned int reserved11;
	unsigned int cpu_status0;
	unsigned int cpu_status1;
	unsigned int os_reg[8];
	unsigned int reserved12[6];
	unsigned int dll_con[4];
	unsigned int dll_status[4];
	unsigned int dfi_wrnum;
	unsigned int dfi_rdnum;
	unsigned int dfi_actnum;
	unsigned int dfi_timerval;
	unsigned int nfi_fifo[4];
	unsigned int reserved13[0x10];
	unsigned int usbphy0_con[8];
	unsigned int usbphy1_con[8];
	unsigned int reserved14[0x10];
	unsigned int chip_tag;
	unsigned int sdmmc_det_cnt;
};
check_member(rk3036_grf, sdmmc_det_cnt, 0x304);

#endif
