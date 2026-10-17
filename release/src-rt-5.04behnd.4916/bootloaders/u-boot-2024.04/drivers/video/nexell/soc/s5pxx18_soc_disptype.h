/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#ifndef _S5PXX18_SOC_DISP_TYPE_H_
#define _S5PXX18_SOC_DISP_TYPE_H_

/* clock control types */
enum nx_pclkmode {
	nx_pclkmode_dynamic = 0UL,
	nx_pclkmode_always	= 1UL
};

enum nx_bclkmode {
	nx_bclkmode_disable	= 0UL,
	nx_bclkmode_dynamic	= 2UL,
	nx_bclkmode_always	= 3UL
};

#endif
