/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __SNT_MU_HAL_H__
#define __SNT_MU_HAL_H__

void mu_hal_init(ulong base);
int mu_hal_sendmsg(ulong base, u32 reg_index, u32 msg);
int mu_hal_receivemsg(ulong base, u32 reg_index, u32 *msg);
#endif
