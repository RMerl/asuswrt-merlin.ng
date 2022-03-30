/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 */


#ifndef __SKSIMX6_CONFIG_H
#define __SKSIMX6_CONFIG_H

#include "mx6_common.h"
#include "imx6_spl.h"

/* Thermal */
#define CONFIG_IMX_THERMAL

/* Serial */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	       UART1_BASE

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(8 * SZ_1M)

/* Ethernet */
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0x01

#define CONFIG_PHY_MICREL_KSZ9021

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_SPEED		  100000

/* Filesystem support */

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	1

/* Environment organization */
#define CONFIG_ENV_SIZE                (16 * 1024)
#define CONFIG_ENV_OFFSET		(6 * 64 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND       (CONFIG_ENV_OFFSET + \
						CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND         CONFIG_ENV_SIZE

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console},${baudrate}\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
		"${netmask}:${hostname}:${netdev}:off\0"		\
	"addmisc=setenv bootargs ${bootargs} ${miscargs}\0" 		\
	"bootcmd=run mmcboot\0"						\
	"bootfile=uImage\0"						\
	"bootimage=uImage\0"						\
	"console=ttymxc0\0"						\
	"fdt_addr_r=0x18000000\0" 					\
	"fdt_file=imx6dl-sks-cts.dtb\0"					\
	"fdt_high=0xffffffff\0" 					\
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" 		\
	"miscargs=quiet\0"						\
	"mmcargs=setenv bootargs root=${mmcroot} rw rootwait\0"		\
	"mmcboot=if run mmcload;then " 					\
		"run mmcargs addcons addmisc;"				\
			"bootm;fi\0" 					\
	"mmcload=mmc rescan;"						\
		"load mmc 0:${mmcpart} ${kernel_addr_r} boot/fitImage\0"\
	"mmcpart=1\0"							\
	"mmcroot=/dev/mmcblk0p1\0"					\
	"net_nfs=tftp ${kernel_addr_r} ${board_name}/${bootfile};"	\
		"tftp ${fdt_addr_r} ${board_name}/${fdt_file};"		\
		"run nfsargs addip addcons addmisc;"			\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"nfsargs=setenv bootargs root=/dev/nfs "			\
		"nfsroot=${serverip}:${nfsroot},v3 panic=1\0"

#endif
