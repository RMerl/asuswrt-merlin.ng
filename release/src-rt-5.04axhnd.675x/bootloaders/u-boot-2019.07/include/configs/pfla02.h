/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 */


#ifndef __PCM058_CONFIG_H
#define __PCM058_CONFIG_H

#ifdef CONFIG_SPL
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(64 * 1024)
#include "imx6_spl.h"
#endif

#include "mx6_common.h"

/* Thermal */
#define CONFIG_IMX_THERMAL

/* Serial */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	       UART4_BASE
#define CONSOLE_DEV		"ttymxc3"

/* Early setup */


/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(8 * SZ_1M)

/* Ethernet */
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		3

/* SPI Flash */

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 0 */
#define CONFIG_SYS_I2C_SPEED		  100000

#ifndef CONFIG_SPL_BUILD
#define CONFIG_CMD_NAND
/* Enable NAND support */
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif

/* DMA stuff, needed for GPMI/MXS NAND support */

/* Filesystem support */

/* Various command support */

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
#define CONFIG_SYS_FSL_USDHC_NUM	2

/* Environment organization */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE                (16 * 1024)
#define CONFIG_ENV_OFFSET		(1024 * SZ_1K)
#define CONFIG_ENV_SECT_SIZE		(64 * SZ_1K)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND       (CONFIG_ENV_OFFSET + \
						CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND         CONFIG_ENV_SIZE

#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET              (0x1E0000)
#define CONFIG_ENV_SECT_SIZE           (128 * SZ_1K)
#endif

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console},${baudrate}\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
		"${netmask}:${hostname}:${netdev}:off\0"		\
	"addmisc=setenv bootargs ${bootargs} ${miscargs}\0"		\
	"addmtd=run mtdnand;run mtdspi;"				\
		"setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"mtdnand=setenv mtdparts mtdparts=gpmi-nand:"			\
		"40m(Kernels),400m(root),-(nand)\0"			\
	"mtdspi=setenv mtdparts ${mtdparts}"				\
		"';spi2.0:1024k(bootloader),"				\
			"64k(env1),64k(env2),-(rescue)'\0"		\
	"bootcmd=if test -n ${rescue};"					\
		"then run swupdate;fi;run nandboot;run swupdate\0"	\
	"bootfile=uImage\0"						\
	"bootimage=uImage\0"						\
	"console=ttymxc3\0"						\
	"fdt_addr_r=0x18000000\0"					\
	"fdt_file=pfla02.dtb\0"						\
	"fdt_high=0xffffffff\0"						\
	"initrd_high=0xffffffff\0"					\
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0"		\
	"miscargs=panic=1 quiet\0"					\
	"mmcargs=setenv bootargs root=${mmcroot} rw rootwait\0"		\
	"mmcboot=if run mmcload;then "					\
		"run mmcargs addcons addmisc;"				\
			"bootm;fi\0"					\
	"mmcload=mmc rescan;"						\
		"load mmc 0:${mmcpart} ${kernel_addr_r} boot/fitImage\0"\
	"mmcpart=1\0"							\
	"mmcroot=/dev/mmcblk0p1\0"					\
	"ubiroot=1\0"							\
	"nandargs=setenv bootargs ubi.mtd=1 "				\
		"root=ubi0:rootfs${ubiroot} rootfstype=ubifs\0"		\
	"nandboot=run mtdnand;ubi part nand0,0;"			\
		"ubi readvol ${kernel_addr_r} kernel${ubiroot};"	\
		"run nandargs addip addcons addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${board_name}/${bootfile};"	\
		"tftp ${fdt_addr_r} ${board_name}/${fdt_file};"		\
		"run nfsargs addip addcons addmtd addmisc;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"net_nfs_fit=tftp ${kernel_addr_r} ${board_name}/${fitfile};"	\
		"run nfsargs addip addcons addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"nfsargs=setenv bootargs root=/dev/nfs"				\
		" nfsroot=${serverip}:${nfsroot},v3 panic=1\0"		\
	"swupdate=setenv bootargs root=/dev/ram;"			\
		"run addip addcons addmtd addmisc;"			\
		"sf probe;"						\
		"sf read ${kernel_addr_r} 120000 600000;"		\
		"sf read 14000000 730000 800000;"			\
		"bootm ${kernel_addr_r} 14000000\0"

#endif
