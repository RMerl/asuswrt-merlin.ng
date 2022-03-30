/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * pdu001.h
 *
 * Copyright (C) 2018 EETS GmbH - http://www.eets.ch/
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef __CONFIG_PDU001_H
#define __CONFIG_PDU001_H

#include <configs/ti_am335x_common.h>

/* Using 32K of volatile storage for environment */
#define CONFIG_ENV_SIZE		0x4000

#define MACH_TYPE_PDU001	5075
#define CONFIG_MACH_TYPE	MACH_TYPE_PDU001
#define CONFIG_BOARD_LATE_INIT

/* Clock Defines */
#define V_OSCK			24000000  /* Clock output from T2 */
#define V_SCLK			(V_OSCK)

#if CONFIG_CONS_INDEX == 1
	#define CONSOLE_DEV "ttyO0"
#elif CONFIG_CONS_INDEX == 2
	#define CONSOLE_DEV "ttyO1"
#elif CONFIG_CONS_INDEX == 3
	#define CONSOLE_DEV "ttyO2"
#elif CONFIG_CONS_INDEX == 4
	#define CONSOLE_DEV "ttyO3"
#elif CONFIG_CONS_INDEX == 5
	#define CONSOLE_DEV "ttyO4"
#elif CONFIG_CONS_INDEX == 6
	#define CONSOLE_DEV "ttyO5"
#endif

#define CONFIG_BOOTCOMMAND \
	"run eval_boot_device;" \
	"setenv bootargs console=${console} " \
	"vt.global_cursor_default=0 " \
	"root=/dev/mmcblk${mmc_boot}p${root_fs_partition} " \
	"rootfstype=ext4 " \
	"rootwait " \
	"rootdelay=1;" \
	"fatload mmc ${mmc_boot} ${fdtaddr} ${fdtfile};" \
	"fatload mmc ${mmc_boot} ${loadaddr} ${bootfile};" \
	"bootz ${loadaddr} - ${fdtaddr}"

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"fdtfile=am335x-pdu001.dtb\0" \
	"bootfile=zImage\0" \
	"console=" CONSOLE_DEV ",115200n8\0" \
	"root_fs_partition=2\0" \
	"eval_boot_device=" \
		"if test $boot_device = emmc; then " \
			"setenv mmc_boot 0;" \
		"elif test $boot_device = sdcard; then " \
			"setenv mmc_boot 1;" \
		"else " \
			"echo Bootdevice is neither MMC0 nor MMC1;" \
			"reset;" \
		"fi;" \
	"\0"
#endif

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1	UART0_BASE
#define CONFIG_SYS_NS16550_COM2	UART1_BASE
#define CONFIG_SYS_NS16550_COM3	UART2_BASE
#define CONFIG_SYS_NS16550_COM4	UART3_BASE
#define CONFIG_SYS_NS16550_COM5	UART4_BASE
#define CONFIG_SYS_NS16550_COM6	UART5_BASE
#define CONFIG_BAUDRATE		115200

#endif	/* ! __CONFIG_PDU001_H */
