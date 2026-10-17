/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd.
 */
#ifndef _ASM_ARCH_IOC_RK3588_H
#define _ASM_ARCH_IOC_RK3588_H

struct rk3588_bus_ioc {
	unsigned int reserved0000[3];      /* Address Offset: 0x0000 */
	unsigned int gpio0b_iomux_sel_h;   /* Address Offset: 0x000C */
	unsigned int gpio0c_iomux_sel_l;   /* Address Offset: 0x0010 */
	unsigned int gpio0c_iomux_sel_h;   /* Address Offset: 0x0014 */
	unsigned int gpio0d_iomux_sel_l;   /* Address Offset: 0x0018 */
	unsigned int gpio0d_iomux_sel_h;   /* Address Offset: 0x001C */
	unsigned int gpio1a_iomux_sel_l;   /* Address Offset: 0x0020 */
	unsigned int gpio1a_iomux_sel_h;   /* Address Offset: 0x0024 */
	unsigned int gpio1b_iomux_sel_l;   /* Address Offset: 0x0028 */
	unsigned int gpio1b_iomux_sel_h;   /* Address Offset: 0x002C */
	unsigned int gpio1c_iomux_sel_l;   /* Address Offset: 0x0030 */
	unsigned int gpio1c_iomux_sel_h;   /* Address Offset: 0x0034 */
	unsigned int gpio1d_iomux_sel_l;   /* Address Offset: 0x0038 */
	unsigned int gpio1d_iomux_sel_h;   /* Address Offset: 0x003C */
	unsigned int gpio2a_iomux_sel_l;   /* Address Offset: 0x0040 */
	unsigned int gpio2a_iomux_sel_h;   /* Address Offset: 0x0044 */
	unsigned int gpio2b_iomux_sel_l;   /* Address Offset: 0x0048 */
	unsigned int gpio2b_iomux_sel_h;   /* Address Offset: 0x004C */
	unsigned int gpio2c_iomux_sel_l;   /* Address Offset: 0x0050 */
	unsigned int gpio2c_iomux_sel_h;   /* Address Offset: 0x0054 */
	unsigned int gpio2d_iomux_sel_l;   /* Address Offset: 0x0058 */
	unsigned int gpio2d_iomux_sel_h;   /* Address Offset: 0x005C */
	unsigned int gpio3a_iomux_sel_l;   /* Address Offset: 0x0060 */
	unsigned int gpio3a_iomux_sel_h;   /* Address Offset: 0x0064 */
	unsigned int gpio3b_iomux_sel_l;   /* Address Offset: 0x0068 */
	unsigned int gpio3b_iomux_sel_h;   /* Address Offset: 0x006C */
	unsigned int gpio3c_iomux_sel_l;   /* Address Offset: 0x0070 */
	unsigned int gpio3c_iomux_sel_h;   /* Address Offset: 0x0074 */
	unsigned int gpio3d_iomux_sel_l;   /* Address Offset: 0x0078 */
	unsigned int gpio3d_iomux_sel_h;   /* Address Offset: 0x007C */
	unsigned int gpio4a_iomux_sel_l;   /* Address Offset: 0x0080 */
	unsigned int gpio4a_iomux_sel_h;   /* Address Offset: 0x0084 */
	unsigned int gpio4b_iomux_sel_l;   /* Address Offset: 0x0088 */
	unsigned int gpio4b_iomux_sel_h;   /* Address Offset: 0x008C */
	unsigned int gpio4c_iomux_sel_l;   /* Address Offset: 0x0090 */
	unsigned int gpio4c_iomux_sel_h;   /* Address Offset: 0x0094 */
	unsigned int gpio4d_iomux_sel_l;   /* Address Offset: 0x0098 */
	unsigned int gpio4d_iomux_sel_h;   /* Address Offset: 0x009C */
};

check_member(rk3588_bus_ioc, gpio4d_iomux_sel_h, 0x009C);

struct rk3588_pmu1_ioc {
	unsigned int gpio0a_iomux_sel_l;   /* Address Offset: 0x0000 */
	unsigned int gpio0a_iomux_sel_h;   /* Address Offset: 0x0004 */
	unsigned int gpio0b_iomux_sel_l;   /* Address Offset: 0x0008 */
	unsigned int reserved0012;         /* Address Offset: 0x000C */
	unsigned int gpio0a_ds_l;          /* Address Offset: 0x0010 */
	unsigned int gpio0a_ds_h;          /* Address Offset: 0x0014 */
	unsigned int gpio0b_ds_l;          /* Address Offset: 0x0018 */
	unsigned int reserved0028;         /* Address Offset: 0x001C */
	unsigned int gpio0a_p;             /* Address Offset: 0x0020 */
	unsigned int gpio0b_p;             /* Address Offset: 0x0024 */
	unsigned int gpio0a_ie;            /* Address Offset: 0x0028 */
	unsigned int gpio0b_ie;            /* Address Offset: 0x002C */
	unsigned int gpio0a_smt;           /* Address Offset: 0x0030 */
	unsigned int gpio0b_smt;           /* Address Offset: 0x0034 */
	unsigned int gpio0a_pdis;          /* Address Offset: 0x0038 */
	unsigned int gpio0b_pdis;          /* Address Offset: 0x003C */
	unsigned int xin_con;              /* Address Offset: 0x0040 */
};

check_member(rk3588_pmu1_ioc, xin_con, 0x0040);

struct rk3588_pmu2_ioc {
	unsigned int gpio0b_iomux_sel_h;  /* Address Offset: 0x0000 */
	unsigned int gpio0c_iomux_sel_l;  /* Address Offset: 0x0004 */
	unsigned int gpio0c_iomux_sel_h;  /* Address Offset: 0x0008 */
	unsigned int gpio0d_iomux_sel_l;  /* Address Offset: 0x000C */
	unsigned int gpio0d_iomux_sel_h;  /* Address Offset: 0x0010 */
	unsigned int gpio0b_ds_h;         /* Address Offset: 0x0014 */
	unsigned int gpio0c_ds_l;         /* Address Offset: 0x0018 */
	unsigned int gpio0c_ds_h;         /* Address Offset: 0x001C */
	unsigned int gpio0d_ds_l;         /* Address Offset: 0x0020 */
	unsigned int gpio0d_ds_h;         /* Address Offset: 0x0024 */
	unsigned int gpio0b_p;            /* Address Offset: 0x0028 */
	unsigned int gpio0c_p;            /* Address Offset: 0x002C */
	unsigned int gpio0d_p;            /* Address Offset: 0x0030 */
	unsigned int gpio0b_ie;           /* Address Offset: 0x0034 */
	unsigned int gpio0c_ie;           /* Address Offset: 0x0038 */
	unsigned int gpio0d_ie;           /* Address Offset: 0x003C */
	unsigned int gpio0b_smt;          /* Address Offset: 0x0040 */
	unsigned int gpio0c_smt;          /* Address Offset: 0x0044 */
	unsigned int gpio0d_smt;          /* Address Offset: 0x0048 */
	unsigned int gpio0b_pdis;         /* Address Offset: 0x004C */
	unsigned int gpio0c_pdis;         /* Address Offset: 0x0050 */
	unsigned int gpio0d_pdis;         /* Address Offset: 0x0054 */
};

check_member(rk3588_pmu2_ioc, gpio0d_pdis, 0x0054);

#endif

