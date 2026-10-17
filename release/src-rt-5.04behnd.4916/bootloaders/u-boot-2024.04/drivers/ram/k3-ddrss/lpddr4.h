/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_H
#define LPDDR4_H

#include "lpddr4_ctl_regs.h"
#include "lpddr4_sanity.h"

#if defined (CONFIG_K3_AM64_DDRSS) || defined (CONFIG_K3_AM62A_DDRSS)
#include "lpddr4_am6x.h"
#include "lpddr4_am6x_sanity.h"
#else
#include "lpddr4_j721e.h"
#include "lpddr4_j721e_sanity.h"
#endif

#define PRODUCT_ID (0x1046U)

#define LPDDR4_BIT_MASK    (0x1U)
#define BYTE_MASK   (0xffU)
#define NIBBLE_MASK (0xfU)

#define WORD_SHIFT (32U)
#define WORD_MASK (0xffffffffU)
#define SLICE_WIDTH (0x100)

#define CTL_OFFSET 0
#define PI_OFFSET (((u32)1) << 11)
#define PHY_OFFSET (((u32)1) << 12)

#define CTL_INT_MASK_ALL ((u32)LPDDR4_LOR_BITS - WORD_SHIFT)

#define PLL_READY (0x3U)
#define IO_CALIB_DONE ((u32)0x1U << 23U)
#define IO_CALIB_FIELD ((u32)NIBBLE_MASK << 28U)
#define IO_CALIB_STATE ((u32)0xBU << 28U)
#define RX_CAL_DONE ((u32)LPDDR4_BIT_MASK << 4U)
#define CA_TRAIN_RL (((u32)LPDDR4_BIT_MASK << 5U) | ((u32)LPDDR4_BIT_MASK << 4U))
#define WR_LVL_STATE (((u32)NIBBLE_MASK) << 13U)
#define GATE_LVL_ERROR_FIELDS (((u32)LPDDR4_BIT_MASK << 7U) | ((u32)LPDDR4_BIT_MASK << 6U))
#define READ_LVL_ERROR_FIELDS ((((u32)NIBBLE_MASK) << 28U) | (((u32)BYTE_MASK) << 16U))
#define DQ_LVL_STATUS (((u32)LPDDR4_BIT_MASK << 26U) | (((u32)BYTE_MASK) << 18U))

#define CDN_TRUE  1U
#define CDN_FALSE 0U

#ifndef LPDDR4_CUSTOM_TIMEOUT_DELAY
#define LPDDR4_CUSTOM_TIMEOUT_DELAY 100000000U
#endif

#ifndef LPDDR4_CPS_NS_DELAY_TIME
#define LPDDR4_CPS_NS_DELAY_TIME 10000000U
#endif

void lpddr4_setsettings(lpddr4_ctlregs *ctlregbase, const bool errorfound);
volatile u32 *lpddr4_addoffset(volatile u32 *addr, u32 regoffset);
u32 lpddr4_pollctlirq(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt irqbit, u32 delay);
bool lpddr4_checklvlerrors(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo, bool errfound);
void lpddr4_seterrors(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, u8 *errfoundptr);

u32 lpddr4_enablepiinitiator(const lpddr4_privatedata *pd);
void lpddr4_checkwrlvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr);
u32 lpddr4_checkmmrreaderror(const lpddr4_privatedata *pd, u64 *mmrvalue, u8 *mrrstatus);
u32 lpddr4_getdslicemask(u32 dslicenum, u32 arrayoffset);

#endif  /* LPDDR4_H */
