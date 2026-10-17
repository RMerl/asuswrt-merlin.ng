/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc.
 */

#ifndef _DT_BINDINGS_MTK_RESET_H_
#define _DT_BINDINGS_MTK_RESET_H_

/* PCIe/SATA Subsystem resets */
#define MT7622_SATA_PHY_REG_RST			12
#define MT7622_SATA_PHY_SW_RST			13
#define MT7622_SATA_AXI_BUS_RST			15
#define PCIE1_CORE_RST			19
#define PCIE1_MMIO_RST			20
#define PCIE1_HRST			21
#define PCIE1_USER_RST			22
#define PCIE1_PIPE_RST			23
#define PCIE0_CORE_RST			27
#define PCIE0_MMIO_RST			28
#define PCIE0_HRST			29
#define PCIE0_USER_RST			30
#define PCIE0_PIPE_RST			31

/* SSUSB Subsystem resets */
#define SSUSB_PHY_PWR_RST		3
#define SSUSB_MAC_PWR_RST		4

/* ETH Subsystem resets */
#define ETHSYS_SYS_RST			0
#define ETHSYS_MCM_RST			2
#define ETHSYS_HSDMA_RST		5
#define ETHSYS_FE_RST			6
#define ETHSYS_ESW_RST			16
#define ETHSYS_GMAC_RST			23
#define ETHSYS_EPHY_RST			24
#define ETHSYS_CRYPTO_RST		29
#define ETHSYS_PPE_RST			31

#endif /* _DT_BINDINGS_MTK_RESET_H_ */
