/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#ifndef __MEDIATEK_RESET_H
#define __MEDIATEK_RESET_H

#include <dm.h>

int mediatek_reset_bind(struct udevice *pdev, u32 regofs, u32 num_regs);

#endif	/* __MEDIATEK_RESET_H */
