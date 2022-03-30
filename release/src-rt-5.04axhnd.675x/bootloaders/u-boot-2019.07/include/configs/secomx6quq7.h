/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Seco S.r.l
 *
 * Configuration settings for the Seco Boards.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CONFIG_BOARD_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART2_BASE

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 500 * SZ_1M)

/* MMC Configuration */
#define CONFIG_SYS_FSL_USDHC_NUM        2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

/* Ethernet Configuration */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		6

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"ethprime=FEC0\0"						\
	"netdev=eth0\0"							\
	"ethprime=FEC0\0"						\
	"uboot=u-boot.bin\0"						\
	"kernel=uImage\0"						\
	"nfsroot=/opt/eldk/arm\0"					\
	"ip_local=10.0.0.5::10.0.0.1:255.255.255.0::eth0:off\0"		\
	"ip_server=10.0.0.1\0"						\
	"nfs_path=/targetfs \0"						\
	"memory=mem=1024M\0"						\
	"bootdev=mmc dev 0; ext2load mmc 0:1\0"				\
	"root=root=/dev/mmcblk0p1\0"					\
	"option=rootwait rw fixrtc rootflags=barrier=1\0"		\
	"cpu_freq=arm_freq=996\0"					\
	"setbootargs=setenv bootargs console=ttymxc1,115200 ${root}"	\
		" ${option} ${memory} ${cpu_freq}\0"			\
	"setbootargs_nfs=setenv bootargs console=ttymxc1,115200"	\
		" root=/dev/nfs  nfsroot=${ip_server}:${nfs_path}"	\
		" nolock,wsize=4096,rsize=4096  ip=:::::eth0:dhcp"	\
		" ${memory} ${cpu_freq}\0"				\
	"setbootdev=setenv boot_dev ${bootdev} 10800000 /boot/uImage\0"	\
	"bootcmd=run setbootargs; run setbootdev; run boot_dev;"	\
		" bootm 0x10800000\0"					\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"

#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			(2u * 1024 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE			(8 * 1024)

#if defined(CONFIG_ENV_IS_IN_MMC)
	#define CONFIG_ENV_OFFSET		(6 * 128 * 1024)
	#define CONFIG_SYS_MMC_ENV_DEV		0
	#define CONFIG_DYNAMIC_MMC_DEVNO
#endif

#endif /* __CONFIG_H */
