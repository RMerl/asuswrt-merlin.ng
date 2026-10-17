/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_AM6X_H
#define LPDDR4_AM6X_H

#ifdef CONFIG_K3_AM64_DDRSS
#include "lpddr4_am64_ctl_regs_rw_masks.h"
#elif CONFIG_K3_AM62A_DDRSS
#include "lpddr4_am62a_ctl_regs_rw_masks.h"
#endif

#ifdef CONFIG_K3_AM64_DDRSS
#define DSLICE_NUM (2U)
#define ASLICE_NUM (2U)
#define DSLICE0_REG_COUNT  (126U)
#define DSLICE1_REG_COUNT  (126U)
#define ASLICE0_REG_COUNT  (42U)
#define ASLICE1_REG_COUNT  (42U)
#define ASLICE2_REG_COUNT  (42U)
#define PHY_CORE_REG_COUNT (126U)

#elif CONFIG_K3_AM62A_DDRSS
#define DSLICE_NUM (4U)
#define ASLICE_NUM (3U)
#define DSLICE0_REG_COUNT  (136U)
#define DSLICE1_REG_COUNT  (136U)
#define DSLICE2_REG_COUNT  (136U)
#define DSLICE3_REG_COUNT  (136U)
#define ASLICE0_REG_COUNT  (48U)
#define ASLICE1_REG_COUNT  (48U)
#define ASLICE2_REG_COUNT  (48U)
#define PHY_CORE_REG_COUNT (132U)

#endif

#define GRP_SHIFT 1
#define INT_SHIFT 2

#endif /* LPDDR4_AM6X_H */
