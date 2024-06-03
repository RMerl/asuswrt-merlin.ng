/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010, Stefano Babic <sbabic@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * Configuration for the MX35pdk Freescale board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

 /* High Level Configuration Options */
#define CONFIG_MX35

#define CONFIG_SYS_FSL_CLK

/* Set TEXT at the beginning of the NOR flash */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)

/*
 * Hardware drivers
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

/*
 * PMIC Configs
 */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_FSL
#define CONFIG_POWER_FSL_MC13892
#define CONFIG_SYS_FSL_PMIC_I2C_ADDR	0x08
#define CONFIG_RTC_MC13XXX

/*
 * MFD MC9SDZ60
 */
#define CONFIG_FSL_MC9SDZ60
#define CONFIG_SYS_FSL_MC9SDZ60_I2C_ADDR	0x69

/*
 * UART (console)
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*
 * Command definition
 */

#define CONFIG_NET_RETRY_COUNT	100


#define CONFIG_LOADADDR		0x80800000	/* loadaddr env var */

/*
 * Ethernet on the debug board (SMC911)
 */
#define CONFIG_HAS_ETH1
#define CONFIG_ETHPRIME

/*
 * Ethernet on SOC (FEC)
 */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1F

#define CONFIG_ARP_TIMEOUT	200UL

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_MEMTEST_START	0	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x10000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)
#define PHYS_SDRAM_2		CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE	(128 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		CSD0_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR + 0x10000)
#define CONFIG_SYS_INIT_RAM_SIZE		(IRAM_SIZE / 2)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_GBL_DATA_OFFSET)

/*
 * MTD Command for mtdparts
 */

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE_ADDR
#define CONFIG_SYS_MAX_FLASH_BANKS 1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT 512	/* max number of sectors on one chip */
/* Monitor at beginning of flash */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
				CONFIG_SYS_MONITOR_LEN)

#if defined(CONFIG_FSL_ENV_IN_NAND)
	#define CONFIG_ENV_OFFSET       (1024 * 1024)
#endif

/*
 * CFI FLASH driver setup
 */

/* A non-standard buffered write algorithm */
#define CONFIG_FLASH_SPANSION_S29WS_N

/*
 * NAND FLASH driver setup
 */
#define CONFIG_MXC_NAND_REGS_BASE	(NFC_BASE_ADDR)
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(NFC_BASE_ADDR)
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_LARGEPAGE

/* EHCI driver */
#define CONFIG_EHCI_IS_TDI
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_MXC
#define CONFIG_MXC_USB_PORT	0
#define CONFIG_MXC_USB_FLAGS	(MXC_EHCI_INTERFACE_DIFF_UNI | \
				 MXC_EHCI_POWER_PINS_ENABLED | \
				 MXC_EHCI_OC_PIN_ACTIVE_LOW)
#define CONFIG_MXC_USB_PORTSC	(MXC_EHCI_UTMI_16BIT | MXC_EHCI_MODE_UTMI)

/* mmc driver */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1

/*
 * Default environment and default scripts
 * to update uboot and load kernel
 */

#define CONFIG_HOSTNAME "mx35pdk"
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth1\0"							\
	"ethprime=smc911x\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip_sta=setenv bootargs ${bootargs} "			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addip_dyn=setenv bootargs ${bootargs} ip=dhcp\0"		\
	"addip=if test -n ${ipdyn};then run addip_dyn;"			\
		"else run addip_sta;fi\0"				\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=ttymxc0,${baudrate}\0"			\
	"addmisc=setenv bootargs ${bootargs} ${misc}\0"			\
	"loadaddr=80800000\0"						\
	"kernel_addr_r=80800000\0"					\
	"hostname=" CONFIG_HOSTNAME "\0"			\
	"bootfile=" CONFIG_HOSTNAME "/uImage\0"		\
	"ramdisk_file=" CONFIG_HOSTNAME "/uRamdisk\0"	\
	"flash_self=run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"net_self_load=tftp ${kernel_addr_r} ${bootfile};"		\
		"tftp ${ramdisk_addr_r} ${ramdisk_file};\0"		\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.bin\0"		\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"uboot_addr=" __stringify(CONFIG_SYS_MONITOR_BASE) "\0"		\
	"update=protect off ${uboot_addr} +80000;"			\
		"erase ${uboot_addr} +80000;"				\
		"cp.b ${loadaddr} ${uboot_addr} ${filesize}\0"		\
	"upd=if run load;then echo Updating u-boot;if run update;"	\
		"then echo U-Boot updated;"				\
			"else echo Error updating u-boot !;"		\
			"echo Board without bootloader !!;"		\
		"fi;"							\
		"else echo U-Boot not downloaded..exiting;fi\0"		\
	"bootcmd=run net_nfs\0"

#endif				/* __CONFIG_H */
