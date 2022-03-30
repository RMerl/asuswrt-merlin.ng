/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

/* Base addresses for the SPI direct access mode */
#define SPI_BUS0_DEV1_BASE	0xe0000000
#define SPI_BUS0_DEV1_SIZE	(1 << 20)
#define SPI_BUS1_DEV2_BASE	(SPI_BUS0_DEV1_BASE + SPI_BUS0_DEV1_SIZE)

void board_fpga_add(void);
