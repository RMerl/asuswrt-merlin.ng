// SPDX-License-Identifier: GPL-2.0+
/*
 * J721S2 specific device platform data
 *
 * This file is auto generated. Please do not hand edit and report any issues
 * to Dave Gerlach <d-gerlach@ti.com>.
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#include "k3-dev.h"

static struct ti_psc soc_psc_list[] = {
	[0] = PSC(0, 0x42000000),
	[1] = PSC(1, 0x00400000),
};

static struct ti_pd soc_pd_list[] = {
	[0] = PSC_PD(0, &soc_psc_list[0], NULL),
	[1] = PSC_PD(0, &soc_psc_list[1], NULL),
	[2] = PSC_PD(1, &soc_psc_list[1], &soc_pd_list[1]),
	[3] = PSC_PD(14, &soc_psc_list[1], NULL),
	[4] = PSC_PD(15, &soc_psc_list[1], &soc_pd_list[3]),
	[5] = PSC_PD(16, &soc_psc_list[1], &soc_pd_list[3]),
};

static struct ti_lpsc soc_lpsc_list[] = {
	[0] = PSC_LPSC(0, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[1] = PSC_LPSC(3, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[2] = PSC_LPSC(10, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[3] = PSC_LPSC(11, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[4] = PSC_LPSC(12, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[5] = PSC_LPSC(0, &soc_psc_list[1], &soc_pd_list[1], NULL),
	[6] = PSC_LPSC(9, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[2]),
	[7] = PSC_LPSC(14, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[8]),
	[8] = PSC_LPSC(15, &soc_psc_list[1], &soc_pd_list[1], NULL),
	[9] = PSC_LPSC(16, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[10]),
	[10] = PSC_LPSC(17, &soc_psc_list[1], &soc_pd_list[1], NULL),
	[11] = PSC_LPSC(20, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[4]),
	[12] = PSC_LPSC(23, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[4]),
	[13] = PSC_LPSC(25, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[4]),
	[14] = PSC_LPSC(43, &soc_psc_list[1], &soc_pd_list[2], NULL),
	[15] = PSC_LPSC(45, &soc_psc_list[1], &soc_pd_list[2], NULL),
	[16] = PSC_LPSC(78, &soc_psc_list[1], &soc_pd_list[3], NULL),
	[17] = PSC_LPSC(80, &soc_psc_list[1], &soc_pd_list[4], &soc_lpsc_list[16]),
	[18] = PSC_LPSC(81, &soc_psc_list[1], &soc_pd_list[5], &soc_lpsc_list[16]),
};

static struct ti_dev soc_dev_list[] = {
	PSC_DEV(35, &soc_lpsc_list[0]),
	PSC_DEV(108, &soc_lpsc_list[0]),
	PSC_DEV(109, &soc_lpsc_list[0]),
	PSC_DEV(110, &soc_lpsc_list[0]),
	PSC_DEV(180, &soc_lpsc_list[0]),
	PSC_DEV(149, &soc_lpsc_list[0]),
	PSC_DEV(115, &soc_lpsc_list[1]),
	PSC_DEV(223, &soc_lpsc_list[1]),
	PSC_DEV(109, &soc_lpsc_list[2]),
	PSC_DEV(110, &soc_lpsc_list[3]),
	PSC_DEV(108, &soc_lpsc_list[4]),
	PSC_DEV(43, &soc_lpsc_list[5]),
	PSC_DEV(61, &soc_lpsc_list[6]),
	PSC_DEV(96, &soc_lpsc_list[7]),
	PSC_DEV(138, &soc_lpsc_list[8]),
	PSC_DEV(97, &soc_lpsc_list[9]),
	PSC_DEV(139, &soc_lpsc_list[10]),
	PSC_DEV(360, &soc_lpsc_list[11]),
	PSC_DEV(99, &soc_lpsc_list[12]),
	PSC_DEV(98, &soc_lpsc_list[13]),
	PSC_DEV(146, &soc_lpsc_list[14]),
	PSC_DEV(354, &soc_lpsc_list[15]),
	PSC_DEV(357, &soc_lpsc_list[15]),
	PSC_DEV(4, &soc_lpsc_list[16]),
	PSC_DEV(202, &soc_lpsc_list[17]),
	PSC_DEV(203, &soc_lpsc_list[18]),
};

const struct ti_k3_pd_platdata j721s2_pd_platdata = {
	.psc = soc_psc_list,
	.pd = soc_pd_list,
	.lpsc = soc_lpsc_list,
	.devs = soc_dev_list,
	.num_psc = ARRAY_SIZE(soc_psc_list),
	.num_pd = ARRAY_SIZE(soc_pd_list),
	.num_lpsc = ARRAY_SIZE(soc_lpsc_list),
	.num_devs = ARRAY_SIZE(soc_dev_list),
};
