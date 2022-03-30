/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _6856_UBUS4_H
#define _6856_UBUS4_H

#define UBUS_MAPPED_BASE    0x83000080

typedef struct Ubus4ClkCtrlCfgRegs {
    uint32_t ClockCtrl;
#define UBUS4_CLK_CTRL_EN_SHIFT    (0)
#define UBUS4_CLK_CTRL_EN_MASK     (0x1 << UBUS4_CLK_CTRL_EN_SHIFT)
#define UBUS4_CLK_BYPASS_SHIFT     (2)
#define UBUS4_CLK_BYPASS_MASK      (0x1 << UBUS4_CLK_BYPASS_SHIFT)
#define UBUS4_MIN_CLK_SEL_SHIFT    (4)
#define UBUS4_MIN_CLK_SEL_MASK     (0x7 << UBUS4_MIN_CLK_SEL_SHIFT)
#define UBUS4_MID_CLK_SEL_SHIFT    (8)
#define UBUS4_MID_CLK_SEL_MASK     (0x7 << UBUS4_MID_CLK_SEL_SHIFT)
    uint32_t reserved0[3];
    uint32_t Min2Mid_threshhold;
    uint32_t Mid2Max_threshhold;
    uint32_t Mid2Min_threshhold;
    uint32_t Max2Mid_threshhold;
    uint32_t ClkIntoMin;
    uint32_t ClkIntoMid;
    uint32_t ClkIntoMax;
    uint32_t reserved1;
    uint32_t ClkMinTime;
    uint32_t ClkMidTime;
    uint32_t ClkMaxTime;
} Ubus4ClkCtrlCfgRegs;

#define UBUS4CLK ((volatile Ubus4ClkCtrlCfgRegs * const) UBUS_MAPPED_BASE)

#define UBUS_SYS_MODULE_BASE                0x83000000

#define MST_PORT_NODE_PCIE0_PHYS_BASE       0x8300C000
#define MST_PORT_NODE_PCIE2_PHYS_BASE       0x83028000
#define MST_PORT_NODE_B53_PHYS_BASE         0x83020000
#define MST_PORT_NODE_USB_PHYS_BASE         0x83038000
#define MST_PORT_NODE_PER_PHYS_BASE         0x83058000
#define MST_PORT_NODE_DMA0_PHYS_BASE        0x83064000
#define MST_PORT_NODE_DMA1_PHYS_BASE        0x83068000
#define MST_PORT_NODE_RQ0_PHYS_BASE         0x83080000
#define MST_PORT_NODE_RQ1_PHYS_BASE         0x83088000
#define MST_PORT_NODE_NATC_PHYS_BASE        0x830A0000
#define MST_PORT_NODE_DQM_PHYS_BASE         0x830A4000
#define MST_PORT_NODE_QM_PHYS_BASE          0x830AC000

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
