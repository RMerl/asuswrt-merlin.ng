/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2024 Broadcom Ltd.
 */

#include <linux/types.h>
#include <string.h>
#include <stdio.h>
#include "smc_merlin.h"
#include "power_rpc_svc.h"

static char *merlin_domain_name[] = {"merlin0", "merlin1"};

int smc_merlin_power_up(unsigned int merlin_num)
{
	int ret = 0;

	if (merlin_num > 1)
		return -1;

	ret = bcm_rpc_pwr_set_domain_state(merlin_domain_name[merlin_num],
							PWR_DOMAIN_NAME_MAX_LEN,
							PWR_DOM_STATE_ON,
							PWR_DOM_RESET_DEASSERT);

	return ret;
}

int smc_merlin_power_down(unsigned int merlin_num)
{
	int ret = 0;

	if (merlin_num > 1)
		return -1;

	ret = bcm_rpc_pwr_set_domain_state(merlin_domain_name[merlin_num],
							PWR_DOMAIN_NAME_MAX_LEN,
							PWR_DOM_STATE_OFF,
							PWR_DOM_RESET_ASSERT);

	return ret;
}
