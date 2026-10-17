/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments' K3 DDRSS Driver
 *
 * Copyright (C) 2021-2022 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#ifndef _K3_DDRSS_
#define _K3_DDRSS_

struct udevice;

int k3_ddrss_ddr_fdt_fixup(struct udevice *dev, void *blob, struct bd_info *bd);

#endif
