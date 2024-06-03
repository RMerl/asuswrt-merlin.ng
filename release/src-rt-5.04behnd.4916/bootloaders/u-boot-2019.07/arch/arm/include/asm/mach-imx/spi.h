/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 */

#ifndef __MXC_SPI_H_
#define __MXC_SPI_H_

/*
 * Board-level chip-select callback
 * Should return GPIO # to be used for chip-select
 */

int board_spi_cs_gpio(unsigned bus, unsigned cs);

#endif
