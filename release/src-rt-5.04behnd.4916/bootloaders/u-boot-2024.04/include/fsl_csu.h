/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor
 *
 */

#ifndef __FSL_CSU_H__
#define __FSL_CSU_H__

enum csu_cslx_access {
	CSU_NS_SUP_R = 0x08,
	CSU_NS_SUP_W = 0x80,
	CSU_NS_SUP_RW = 0x88,
	CSU_NS_USER_R = 0x04,
	CSU_NS_USER_W = 0x40,
	CSU_NS_USER_RW = 0x44,
	CSU_S_SUP_R = 0x02,
	CSU_S_SUP_W = 0x20,
	CSU_S_SUP_RW = 0x22,
	CSU_S_USER_R = 0x01,
	CSU_S_USER_W = 0x10,
	CSU_S_USER_RW = 0x11,
	CSU_ALL_RW = 0xff,
};

struct csu_ns_dev {
	u8 ind;
	u8 val;
};

void enable_layerscape_ns_access(void);
void set_devices_ns_access(unsigned long, u16 val);
void set_pcie_ns_access(int pcie, u16 val);

#endif
