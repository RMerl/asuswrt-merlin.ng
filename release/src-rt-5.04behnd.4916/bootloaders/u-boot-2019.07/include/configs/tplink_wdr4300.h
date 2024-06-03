/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_MHZ			280
#define CONFIG_SYS_MIPS_TIMER_FREQ	(CONFIG_SYS_MHZ * 1000000)

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MALLOC_LEN		0x40000
#define CONFIG_SYS_BOOTPARAMS_LEN	0x20000

#define CONFIG_SYS_SDRAM_BASE		0xa0000000
#define CONFIG_SYS_LOAD_ADDR		0xa1000000
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

#define CONFIG_SYS_INIT_RAM_ADDR	0xbd000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_CLK		40000000
#define CONFIG_SYS_BAUDRATE_TABLE \
	{9600, 19200, 38400, 57600, 115200}

#define CONFIG_BOOTCOMMAND		\
	"dhcp 192.168.1.1:wdr4300.fit && bootm $loadaddr"

#define CONFIG_ENV_SIZE			0x10000

/*
 * Command
 */
/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */

/* USB, USB storage, USB ethernet */
#define CONFIG_EHCI_MMIO_BIG_ENDIAN
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_IS_TDI

/*
 * Diagnostics
 */
#define CONFIG_SYS_MEMTEST_START	0x80100000
#define CONFIG_SYS_MEMTEST_END		0x83f00000

#define CONFIG_CMD_MII

#endif  /* __CONFIG_H */
