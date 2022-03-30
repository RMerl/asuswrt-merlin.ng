/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63158_UBUS4_H
#define _63158_UBUS4_H

#define UBUS_SYS_MODULE_BASE                0x83000000

#define MST_PORT_NODE_PCIE0_PHYS_BASE       0x8300C000
#define MST_PORT_NODE_DSLCPU_PHYS_BASE      0x8301C000
#define MST_PORT_NODE_B53_PHYS_BASE         0x83020000
#define MST_PORT_NODE_PMC_PHYS_BASE         0x8302C000
#define MST_PORT_NODE_PER_PHYS_BASE         0x83034000
#define MST_PORT_NODE_PER_DMA_PHYS_BASE     0x8303C000
#define MST_PORT_NODE_SWH_PHYS_BASE         0x83048000
#define MST_PORT_NODE_SPU_PHYS_BASE         0x83050000
#define MST_PORT_NODE_DSL_PHYS_BASE         0x8305C000
#define MST_PORT_NODE_PCIE2_PHYS_BASE       0x83064000
#define MST_PORT_NODE_PCIE3_PHYS_BASE       0x8306C000
#define MST_PORT_NODE_USB_PHYS_BASE         0x83074000
#define MST_PORT_NODE_DMA0_PHYS_BASE        0x8347C000
#define MST_PORT_NODE_DMA1_PHYS_BASE        0x83480000
#define MST_PORT_NODE_RQ0_PHYS_BASE         0x83498000
#define MST_PORT_NODE_NATC_PHYS_BASE        0x834B8000
#define MST_PORT_NODE_DQM_PHYS_BASE         0x834BC000
#define MST_PORT_NODE_QM_PHYS_BASE          0x834C4000

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
