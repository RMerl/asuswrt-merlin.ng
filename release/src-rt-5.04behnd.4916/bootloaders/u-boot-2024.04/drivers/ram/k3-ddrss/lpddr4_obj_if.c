// SPDX-License-Identifier: BSD-3-Clause
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include "lpddr4_obj_if.h"

lpddr4_obj *lpddr4_getinstance(void)
{
	static lpddr4_obj driver = {
		.probe				= lpddr4_probe,
		.init				= lpddr4_init,
		.start				= lpddr4_start,
		.readreg			= lpddr4_readreg,
		.writereg			= lpddr4_writereg,
		.getmmrregister			= lpddr4_getmmrregister,
		.setmmrregister			= lpddr4_setmmrregister,
		.writectlconfig			= lpddr4_writectlconfig,
		.writephyconfig			= lpddr4_writephyconfig,
		.writephyindepconfig		= lpddr4_writephyindepconfig,
		.readctlconfig			= lpddr4_readctlconfig,
		.readphyconfig			= lpddr4_readphyconfig,
		.readphyindepconfig		= lpddr4_readphyindepconfig,
		.getctlinterruptmask		= lpddr4_getctlinterruptmask,
		.setctlinterruptmask		= lpddr4_setctlinterruptmask,
		.checkctlinterrupt		= lpddr4_checkctlinterrupt,
		.ackctlinterrupt		= lpddr4_ackctlinterrupt,
		.getphyindepinterruptmask	= lpddr4_getphyindepinterruptmask,
		.setphyindepinterruptmask	= lpddr4_setphyindepinterruptmask,
		.checkphyindepinterrupt		= lpddr4_checkphyindepinterrupt,
		.ackphyindepinterrupt		= lpddr4_ackphyindepinterrupt,
		.getdebuginitinfo		= lpddr4_getdebuginitinfo,
		.getlpiwakeuptime		= lpddr4_getlpiwakeuptime,
		.setlpiwakeuptime		= lpddr4_setlpiwakeuptime,
		.geteccenable			= lpddr4_geteccenable,
		.seteccenable			= lpddr4_seteccenable,
		.getreducmode			= lpddr4_getreducmode,
		.setreducmode			= lpddr4_setreducmode,
		.getdbireadmode			= lpddr4_getdbireadmode,
		.getdbiwritemode		= lpddr4_getdbiwritemode,
		.setdbimode			= lpddr4_setdbimode,
		.getrefreshrate			= lpddr4_getrefreshrate,
		.setrefreshrate			= lpddr4_setrefreshrate,
		.refreshperchipselect		= lpddr4_refreshperchipselect,
		.deferredregverify		= lpddr4_deferredregverify,
	};

	return &driver;
}
