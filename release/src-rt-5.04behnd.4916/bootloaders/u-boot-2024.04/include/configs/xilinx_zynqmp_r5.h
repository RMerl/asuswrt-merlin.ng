/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2018 Xilinx, Inc. (Michal Simek)
 */

#ifndef __CONFIG_ZYNQMP_R5_H
#define __CONFIG_ZYNQMP_R5_H

#define CFG_EXTRA_ENV_SETTINGS

/* Serial drivers */
/* The following table includes the supported baudrates */
#define CFG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* Boot configuration */

#define CFG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CFG_SYS_INIT_RAM_SIZE	0x1000

/* Extend size of kernel image for uncompression */

#endif /* __CONFIG_ZYNQ_ZYNQMP_R5_H */
