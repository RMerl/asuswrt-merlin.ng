/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6858_UBUS4_H
#define _6858_UBUS4_H

#define UBUS_SYS_MODULE_BASE                0x83000000

#define MST_PORT_NODE_PCIE0_PHYS_BASE       0x8300C000
#define MST_PORT_NODE_PCIE2_PHYS_BASE       0x83014000
#define MST_PORT_NODE_B53_PHYS_BASE         0x83020000
#define MST_PORT_NODE_SPU_PHYS_BASE         0x83030000
#define MST_PORT_NODE_USB_PHYS_BASE         0x83038000
#define MST_PORT_NODE_PMC_PHYS_BASE         0x83044000
#define MST_PORT_NODE_APM_PHYS_BASE         0x8304C000
#define MST_PORT_NODE_PER_PHYS_BASE         0x83058000
#define MST_PORT_NODE_PER_DMA_PHYS_BASE     0x83060000
#define MST_PORT_NODE_DMA0_PHYS_BASE        0x83464000
#define MST_PORT_NODE_DMA1_PHYS_BASE        0x83468000
#define MST_PORT_NODE_RQ0_PHYS_BASE         0x83480000
#define MST_PORT_NODE_RQ1_PHYS_BASE         0x83488000
#define MST_PORT_NODE_RQ2_PHYS_BASE         0x83490000
#define MST_PORT_NODE_RQ3_PHYS_BASE         0x83498000
#define MST_PORT_NODE_NATC_PHYS_BASE        0x834A0000
#define MST_PORT_NODE_DQM_PHYS_BASE         0x834A4000
#define MST_PORT_NODE_QM_PHYS_BASE          0x834AC000
#define MST_PORT_NODE_TOP_BUFF_PHYS_BASE    0x830e0000
#define MST_PORT_NODE_XRDP_BUFF_PHYS_BASE   0x834e4000

typedef struct Ubus4SysModuleTop {
	uint32_t unused0[16];      /* 0x0 */
	uint32_t UcbData;          /* 0x40 */
	uint32_t UcbHdr;           /* 0x44 */
	uint32_t UcbCntl;          /* 0x48 */
	uint32_t unused1;          /* 0x4c */
	uint32_t ReadUcbHdr;       /* 0x50 */
	uint32_t ReadUcbData;      /* 0x54 */
	uint32_t ReadUcbStatus;    /* 0x58 */
	uint32_t ReacUcbFifoStatus; /* 0x5c */
} Ubus4SysModuleTop;

#define UBUSSYSTOP ((volatile Ubus4SysModuleTop * const) UBUS_SYS_MODULE_BASE)

#endif
