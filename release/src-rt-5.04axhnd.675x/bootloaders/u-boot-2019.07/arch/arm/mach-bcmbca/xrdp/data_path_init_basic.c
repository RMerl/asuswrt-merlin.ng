// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
   
 */


#include "rdd.h"

#include "data_path_init_basic.h"

dpi_params_t *p_dpi_cfg;

uintptr_t rdp_runner_core_addr[GROUPED_EN_SEGMENTS_NUM];

static const access_log_tuple_t init_data[] = {
	#include "data_path_init_basic_data.h"
	{ (ACCESS_LOG_OP_STOP << 24), 0 }
};

int data_path_init_basic(dpi_params_t *dpi_params)
{
	 int rc = 0;

	p_dpi_cfg = dpi_params;

	printf("%s: Restore HW configuration\n", __func__);
	rc = access_log_restore(init_data);
	printf("%s: Restore HW configuration done. rc=%d\n", __func__, rc);

	return rc;
}

