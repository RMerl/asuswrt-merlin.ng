/*
 * Copyright (C) 2018 Jagan Teki <jagan@amarulasolutions.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __spi_davinci_h
#define __spi_davinci_h

struct davinci_spi_plat {
	struct davinci_spi_regs *regs;
	u8 num_cs;	   /* total no. of CS available */
};

#endif /* __spi_davinci_h */
