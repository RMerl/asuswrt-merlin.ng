/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * common reset-controller functions for B&R boards
 *
 * Copyright (C) 2019 Hannes Schmelzer <oe5hpm@oevsv.at>
 * B&R Industrial Automation GmbH - http://www.br-automation.com/ *
 */
#ifndef __CONFIG_BRRESETC_H__
#define __CONFIG_BRRESETC_H__
#include <common.h>

int br_resetc_regget(u8 reg, u8 *dst);
int br_resetc_regset(u8 reg, u8 val);
int br_resetc_bmode(void);

/* reset controller register defines */
#define RSTCTRL_CTRLREG		0x01
#define RSTCTRL_SCRATCHREG0	0x04
#define RSTCTRL_ENHSTATUS	0x07
#define RSTCTRL_SCRATCHREG1	0x08
#define RSTCTRL_RSTCAUSE	0x00
#define RSTCTRL_ERSTCAUSE	0x09
#define RSTCTRL_SPECGPIO_I	0x0A
#define RSTCTRL_SPECGPIO_O	0x0B

#endif /* __CONFIG_BRRESETC_H__ */
