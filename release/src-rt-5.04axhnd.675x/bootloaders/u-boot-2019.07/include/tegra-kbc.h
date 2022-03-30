/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __include_tegra_kbc_h__
#define __include_tegra_kbc_h__

#include <common.h>

#define KEY_IS_MODIFIER(key) ((key) >= KEY_FIRST_MODIFIER)

struct kbc_tegra {
	u32 control;
	u32 interrupt;
	u32 row_cfg[4];
	u32 col_cfg[3];
	u32 timeout_dly;
	u32 init_dly;
	u32 rpt_dly;
	u32 kp_ent[2];
	u32 row_mask[16];
};

#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#else
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* __include_tegra_kbc_h__ */
