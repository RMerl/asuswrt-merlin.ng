/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_CONFIGS_H__
#define __FSP_CONFIGS_H__

struct fsp_config_data {
	struct fsp_cfg_common	common;
	struct upd_region	fsp_upd;
};

struct fspinit_rtbuf {
	struct common_buf	common;	/* FSP common runtime data structure */
};

#endif /* __FSP_CONFIGS_H__ */
