/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016-2017 Intel Corporation <www.intel.com>
 */

#ifndef _PINMUX_H_
#define _PINMUX_H_

#define PINMUX_UART		0xD

#ifndef __ASSEMBLY__
int config_dedicated_pins(const void *blob);
int config_pins(const void *blob, const char *pin_grp);
#endif

#endif /* _PINMUX_H_ */
