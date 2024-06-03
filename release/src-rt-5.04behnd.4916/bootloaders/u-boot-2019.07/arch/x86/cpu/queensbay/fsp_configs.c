// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/fsp/fsp_support.h>

void update_fsp_configs(struct fsp_config_data *config,
			struct fspinit_rtbuf *rt_buf)
{
	/* Initialize runtime buffer for fsp_init() */
	rt_buf->common.stack_top = config->common.stack_top - 32;
	rt_buf->common.boot_mode = config->common.boot_mode;
	rt_buf->common.upd_data = &config->fsp_upd;

	/* Override any UPD setting if required */
}
