/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#ifndef _S5PXX18_SOC_LVDS_H_
#define _S5PXX18_SOC_LVDS_H_

/*
 * refter to s5pxx18_soc_disptop.h
 *
 * #define NUMBER_OF_LVDS_MODULE 1
 * #define PHY_BASEADDR_LVDS_MODULE	0xC010A000
 */
#define	PHY_BASEADDR_LVDS_LIST	\
		{ PHY_BASEADDR_LVDS_MODULE }

struct nx_lvds_register_set {
	u32 lvdsctrl0;
	u32 lvdsctrl1;
	u32 lvdsctrl2;
	u32 lvdsctrl3;
	u32 lvdsctrl4;
	u32 _reserved0[3];
	u32 lvdsloc0;
	u32 lvdsloc1;
	u32 lvdsloc2;
	u32 lvdsloc3;
	u32 lvdsloc4;
	u32 lvdsloc5;
	u32 lvdsloc6;
	u32 _reserved1;
	u32 lvdslocmask0;
	u32 lvdslocmask1;
	u32 lvdslocpol0;
	u32 lvdslocpol1;
	u32 lvdstmode0;
	u32 lvdstmode1;
	u32 _reserved2[2];
};

int nx_lvds_initialize(void);
u32 nx_lvds_get_number_of_module(void);
u32 nx_lvds_get_size_of_register_set(void);
void nx_lvds_set_base_address(u32 module_index, void *base_address);
void *nx_lvds_get_base_address(u32 module_index);
u32 nx_lvds_get_physical_address(u32 module_index);
int nx_lvds_open_module(u32 module_index);
int nx_lvds_close_module(u32 module_index);
int nx_lvds_check_busy(u32 module_index);

void nx_lvds_set_lvdsctrl0(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsctrl1(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsctrl2(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsctrl3(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsctrl4(u32 module_index, u32 regvalue);
u32 nx_lvds_get_lvdsctrl0(u32 module_index);
u32 nx_lvds_get_lvdsctrl1(u32 module_index);
u32 nx_lvds_get_lvdsctrl2(u32 module_index);
u32 nx_lvds_get_lvdsctrl3(u32 module_index);
u32 nx_lvds_get_lvdsctrl4(u32 module_index);

void nx_lvds_set_lvdstmode0(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc0(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc1(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc2(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc3(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc4(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc5(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdsloc6(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdslocmask0(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdslocmask1(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdslocpol0(u32 module_index, u32 regvalue);
void nx_lvds_set_lvdslocpol1(u32 module_index, u32 regvalue);

void nx_lvds_set_lvdslocpol1(u32 module_index, u32 regvalue);

void nx_lvds_set_lvdsdummy(u32 module_index, u32 regvalue);
u32 nx_lvds_get_lvdsdummy(u32 module_index);

#endif
