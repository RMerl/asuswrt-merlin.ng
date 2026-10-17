/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __IMX8M_PICOPI_H
#define __IMX8M_PICOPI_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR		0x182000
/* For RAW image gives a error info not panic */
#endif

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_CMD_NET)
#define CFG_FEC_MXC_PHYADDR		1
#endif

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS					\
	"script=boot.scr\0"						\
	"image=Image\0"							\
	"console=ttymxc0,115200\0"					\
	"fdt_addr=0x43000000\0"						\
	"fdt_high=0xffffffffffffffff\0"					\
	"fdt_file=imx8mq-pico-pi.dtb\0"					\
	"initrd_addr=0x43800000\0"					\
	"initrd_high=0xffffffffffffffff\0"				\
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0"		\
	"mmcpart=1\0"	\
	"mmcroot=/dev/mmcblk1p2 rootwait rw\0"			\
	"mmcautodetect=yes\0"						\
	"mmcargs=setenv bootargs console=${console} root=${mmcroot}\0 "	\
	"loadbootscript="						\
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; source\0"	\
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; "				\
		"run mmcargs; "						\
		"echo wait for boot; "					\
		"fi;\0"							\
	"netargs=setenv bootargs console=${console} "			\
		"root=/dev/nfs "					\
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0"	\
	"netboot=echo Booting from net ...; "				\
		"run netargs;  "					\
		"if test ${ip_dyn} = yes; then "			\
			"setenv get_cmd dhcp; "				\
		"else "							\
			"setenv get_cmd tftp; "				\
		"fi; "							\
		"${get_cmd} ${loadaddr} ${image}; "			\
		"booti; "

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x80000


#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x80000000	/* 2 GiB DDR */

#define CFG_MXC_UART_BASE		UART_BASE_ADDR(1)

#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

#endif
