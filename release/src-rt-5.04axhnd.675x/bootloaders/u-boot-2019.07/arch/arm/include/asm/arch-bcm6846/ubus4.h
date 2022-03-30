/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _6846_UBUS4_H
#define _6846_UBUS4_H

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

#endif
