/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * Menlosystems M53Menlo configuration
 * Copyright (C) 2012-2017 Marek Vasut <marex@denx.de>
 * Copyright (C) 2014-2017 Olaf Mandel <o.mandel@menlosystems.com>
 */

#ifndef __M53MENLO_CONFIG_H__
#define __M53MENLO_CONFIG_H__

#include <asm/arch/imx-regs.h>

/*
 * Memory configurations
 */
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(gd->bd->bi_dram[0].size)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(gd->bd->bi_dram[1].size)
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CFG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CFG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CFG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

/*
 * U-Boot general configurations
 */

/*
 * Serial Driver
 */
#define CFG_MXC_UART_BASE		UART1_BASE

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
#define CFG_SYS_FSL_ESDHC_ADDR	0
#endif

/*
 * NAND
 */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		NFC_BASE_ADDR_AXI
#define CFG_MXC_NAND_REGS_BASE	NFC_BASE_ADDR_AXI
#define CFG_MXC_NAND_IP_REGS_BASE	NFC_BASE_ADDR
#define CFG_SYS_NAND_LARGEPAGE
#endif

/*
 * Ethernet on SOC (FEC)
 */
#ifdef CONFIG_CMD_NET
#define CFG_FEC_MXC_PHYADDR		0x0
#endif

#define CFG_SYS_RTC_BUS_NUM		1 /* I2C2 */

/*
 * RTC
 */
#ifdef CONFIG_CMD_DATE
#define CFG_SYS_I2C_RTC_ADDR		0x68
#endif

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CFG_MXC_USB_PORT		1
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

/* LVDS display */
#define CFG_SYS_LDB_CLOCK			33260000

/* Watchdog */

/*
 * Extra Environments
 */

#define CFG_EXTRA_ENV_SETTINGS					\
	"consdev=ttymxc0\0"						\
	"baudrate=115200\0"						\
	"bootscript=boot.scr\0"						\
	"mmcdev=0\0"							\
	"mmcpart=1\0"							\
	"rootpath=/srv/\0"						\
	"kernel_addr_r=0x72000000\0"					\
	"netdev=eth0\0"							\
	"splashsource=mmc_fs\0"						\
	"splashfile=boot/usplash.bmp.gz\0"				\
	"splashimage=0x88000000\0"					\
	"splashpos=m,m\0"						\
	"altbootcmd="							\
		"if test ${mmcpart} -eq 1 ; then "			\
			"setenv mmcpart 2 ; "				\
		"else "							\
			"setenv mmcpart 1 ; "				\
		"fi ; "							\
		"boot\0"						\
	"stdout=serial,vidconsole\0"					\
	"stderr=serial,vidconsole\0"					\
	"addcons="							\
		"setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off\0"				\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc="							\
		"setenv bootargs ${bootargs} ${miscargs}\0"		\
	"addargs=run addcons addmisc addmtd\0"				\
	"mmcload="							\
		"mmc rescan || reset ; load mmc ${mmcdev}:${mmcpart} "	\
		"${kernel_addr_r} ${bootfile} || reset\0"		\
	"miscargs=nohlt panic=1\0"					\
	"mmcargs=setenv bootargs root=/dev/mmcblk0p${mmcpart} rw "	\
		"rootwait\0"						\
	"mmc_mmc="							\
		"run mmcload mmcargs addargs || reset ; "		\
		"bootm ${kernel_addr_r} ; reset\0"			\
	"netload=tftp ${kernel_addr_r} ${hostname}/${bootfile}\0"	\
	"net_nfs="							\
		"run netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}${hostname},v3,tcp\0"	\
	"try_bootscript="						\
		"mmc rescan;"						\
		"if test -e mmc 0:1 ${bootscript} ; then "		\
		"if load mmc 0:1 ${kernel_addr_r} ${bootscript};"	\
		"then ; "						\
			"echo Running bootscript... ; "			\
			"source ${kernel_addr_r} ; "			\
		"fi ; "							\
		"fi\0"

#endif	/* __M53MENLO_CONFIG_H__ */
