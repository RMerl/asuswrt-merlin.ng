/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 *
 * Definitions of ioctl requests of MT7620 sysc driver
 */

#ifndef _MT7620_SYSC_H_
#define _MT7620_SYSC_H_

#include <linux/types.h>

enum mt7620_sysc_requests {
	MT7620_SYSC_IOCTL_GET_CLK,
	MT7620_SYSC_IOCTL_GET_CHIP_REV,
	MT7620_SYSC_IOCTL_SET_GE1_MODE,
	MT7620_SYSC_IOCTL_SET_GE2_MODE,
	MT7620_SYSC_IOCTL_SET_USB_MODE,
	MT7620_SYSC_IOCTL_SET_PCIE_MODE
};

struct mt7620_sysc_clks {
	u32 cpu_clk;
	u32 sys_clk;
	u32 xtal_clk;
	u32 peri_clk;
};

struct mt7620_sysc_chip_rev {
	bool bga;
	u32 ver : 4;
	u32 eco : 4;
};

enum mt7620_sysc_ge_mode {
	MT7620_SYSC_GE_RGMII,
	MT7620_SYSC_GE_MII,
	MT7620_SYSC_GE_RMII,
	MT7620_SYSC_GE_ESW_PHY,
};

enum mt7620_sysc_usb_mode {
	MT7620_SYSC_USB_DEVICE_MODE,
	MT7620_SYSC_USB_HOST_MODE
};

enum mt7620_sysc_pcie_mode {
	MT7620_SYSC_PCIE_EP_MODE,
	MT7620_SYSC_PCIE_RC_MODE
};

#endif /* _MT7620_SYSC_H_ */
