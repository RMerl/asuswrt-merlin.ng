/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009, STMicroelectronics - All Rights Reserved
 * Author(s): Vipin Kumar, <vipin.kumar@st.com> for STMicroelectronics.
 *
 * Copyright (C) 2012, 2015 Stefan Roese <sr@denx.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SPEAR600				/* SPEAr600 SoC */
#define CONFIG_X600				/* on X600 board */

#include <asm/arch/hardware.h>

/* Timer, HZ specific defines */
#define CONFIG_SYS_HZ_CLOCK			8300000

#define CONFIG_SYS_FLASH_BASE			0xf8000000
/* Reserve 8KiB for SPL */
#define CONFIG_SPL_PAD_TO			8192	/* decimal for 'dd' */
#define CONFIG_SYS_SPL_LEN			CONFIG_SPL_PAD_TO
#define CONFIG_SYS_UBOOT_BASE			(CONFIG_SYS_FLASH_BASE + \
						 CONFIG_SYS_SPL_LEN)
#define CONFIG_SYS_UBOOT_START			CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_BASE			CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN			0x60000

/* Serial Configuration (PL011) */
#define CONFIG_SYS_SERIAL0			0xD0000000
#define CONFIG_SYS_SERIAL1			0xD0080000
#define CONFIG_PL01x_PORTS			{ (void *)CONFIG_SYS_SERIAL0, \
						(void *)CONFIG_SYS_SERIAL1 }
#define CONFIG_PL011_CLOCK			(48 * 1000 * 1000)
#define CONFIG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, \
						  57600, 115200 }
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/* NOR FLASH config options */
#define CONFIG_ST_SMI
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_FLASH_BANK_SIZE		0x01000000
#define CONFIG_SYS_FLASH_ADDR_BASE		{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_SECT		128
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_ERASE_TOUT		(3 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT		(3 * CONFIG_SYS_HZ)

/* NAND FLASH config options */
#define CONFIG_NAND_FSMC
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			CONFIG_FSMC_NAND_BASE
#define CONFIG_MTD_ECC_SOFT
#define CONFIG_SYS_FSMC_NAND_8BIT
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_ECC_BCH

/* UBI/UBI config options */

/* Ethernet config options */
#define CONFIG_PHY_RESET_DELAY			10000		/* in usec */

#define CONFIG_SPEAR_GPIO

/* I2C config options */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_BASE			0xD0200000
#define CONFIG_SYS_I2C_SPEED			400000
#define CONFIG_SYS_I2C_SLAVE			0x02
#define CONFIG_I2C_CHIPADDRESS			0x50

#define CONFIG_SYS_I2C_RTC_ADDR	0x68

/* FPGA config options */
#define CONFIG_FPGA_COUNT	1

/* USB EHCI options */
#define CONFIG_USB_EHCI_SPEAR
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

/*
 * U-Boot Environment placing definitions.
 */
#define CONFIG_ENV_SECT_SIZE			0x00010000
#define CONFIG_ENV_ADDR				(CONFIG_SYS_MONITOR_BASE + \
						 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE				0x02000
#define CONFIG_ENV_ADDR_REDUND			(CONFIG_ENV_ADDR + \
						 CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND			(CONFIG_ENV_SIZE)

/* Miscellaneous configurable options */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_BOOT_PARAMS_ADDR			0x00000100
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_MX_CYCLIC		/* enable mdc/mwc commands      */

#define CONFIG_SYS_MEMTEST_START		0x00800000
#define CONFIG_SYS_MEMTEST_END			0x04000000
#define CONFIG_SYS_MALLOC_LEN			(8 << 20)
#define CONFIG_SYS_LOAD_ADDR			0x00800000

#define CONFIG_HOSTNAME				"x600"
#define CONFIG_UBI_PART				ubi0
#define CONFIG_UBIFS_VOLUME			rootfs

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u-boot_addr=1000000\0"						\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.spr\0"		\
	"load=tftp ${u-boot_addr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};"					\
		"erase " __stringify(CONFIG_SYS_MONITOR_BASE) " +${filesize};" \
		"cp.b ${u-boot_addr} " __stringify(CONFIG_SYS_MONITOR_BASE) \
		" ${filesize};"						\
		"protect on " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize}\0"					\
	"upd=run load update\0"						\
	"ubifs=" CONFIG_HOSTNAME "/ubifs.img\0"		\
	"part=" __stringify(CONFIG_UBI_PART) "\0"			\
	"vol=" __stringify(CONFIG_UBIFS_VOLUME) "\0"			\
	"load_ubifs=tftp ${kernel_addr} ${ubifs}\0"			\
	"update_ubifs=ubi part ${part};ubi write ${kernel_addr} ${vol}"	\
		" ${filesize}\0"					\
	"upd_ubifs=run load_ubifs update_ubifs\0"			\
	"init_ubifs=nand erase.part ubi0;ubi part ${part};"		\
		"ubi create ${vol} 4000000\0"				\
	"netdev=eth0\0"							\
	"rootpath=/opt/eldk-4.2/arm\0"					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"boot_part=0\0"							\
	"altbootcmd=if test $boot_part -eq 0;then "			\
			"echo Switching to partition 1!;"		\
			"setenv boot_part 1;"				\
		"else; "						\
			"echo Switching to partition 0!;"		\
			"setenv boot_part 0;"				\
		"fi;"							\
		"saveenv;boot\0"					\
	"ubifsargs=set bootargs ubi.mtd=ubi${boot_part} "		\
		"root=ubi0:rootfs rootfstype=ubifs\0"			\
	"kernel=" CONFIG_HOSTNAME "/uImage\0"		\
	"kernel_fs=/boot/uImage \0"					\
	"kernel_addr=1000000\0"						\
	"dtb=" CONFIG_HOSTNAME "/"				\
		CONFIG_HOSTNAME ".dtb\0"			\
	"dtb_fs=/boot/" CONFIG_HOSTNAME ".dtb\0"		\
	"dtb_addr=1800000\0"						\
	"load_kernel=tftp ${kernel_addr} ${kernel}\0"			\
	"load_dtb=tftp ${dtb_addr} ${dtb}\0"				\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcon=setenv bootargs ${bootargs} console=ttyAMA0,"		\
		"${baudrate}\0"						\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"net_nfs=run load_dtb load_kernel; "				\
		"run nfsargs addip addcon addmtd addmisc;"		\
		"bootm ${kernel_addr} - ${dtb_addr}\0"			\
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"					\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"				\
	"nand_ubifs=run ubifs_mount ubifs_load ubifsargs addip"		\
		" addcon addmisc addmtd;"				\
		"bootm ${kernel_addr} - ${dtb_addr}\0"			\
	"ubifs_mount=ubi part ubi${boot_part};ubifsmount ubi:rootfs\0"	\
	"ubifs_load=ubifsload ${kernel_addr} ${kernel_fs};"		\
		"ubifsload ${dtb_addr} ${dtb_fs};\0"			\
	"nand_ubifs=run ubifs_mount ubifs_load ubifsargs addip addcon "	\
		"addmtd addmisc;bootm ${kernel_addr} - ${dtb_addr}\0"	\
	"bootcmd=run nand_ubifs\0"					\
	"\0"

/* Physical Memory Map */
#define PHYS_SDRAM_1				0x00000000
#define PHYS_SDRAM_1_MAXSIZE			0x40000000

#define CONFIG_SYS_SDRAM_BASE			PHYS_SDRAM_1
#define CONFIG_SRAM_BASE			0xd2800000
/* Preserve the last 2 lwords for the boot-counter */
#define CONFIG_SRAM_SIZE			((8 << 10) - 0x8)
#define CONFIG_SYS_INIT_RAM_ADDR		CONFIG_SRAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE		CONFIG_SRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET		\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_INIT_SP_ADDR			\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/*
 * SPL related defines
 */
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SRAM_SIZE - 0xb00)
#define	CONFIG_SPL_START_S_PATH	"arch/arm/cpu/arm926ejs/spear"

/*
 * Please select/define only one of the following
 * Each definition corresponds to a supported DDR chip.
 * DDR configuration is based on the following selection
 */
#define CONFIG_DDR_MT47H64M16		1
#define CONFIG_DDR_MT47H32M16		0
#define CONFIG_DDR_MT47H128M8		0

/*
 * Synchronous/Asynchronous operation of DDR
 *
 * Select CONFIG_DDR_2HCLK for DDR clk = 333MHz, synchronous operation
 * Select CONFIG_DDR_HCLK for DDR clk = 166MHz, synchronous operation
 * Select CONFIG_DDR_PLL2 for DDR clk = PLL2, asynchronous operation
 */
#define CONFIG_DDR_2HCLK		1
#define CONFIG_DDR_HCLK			0
#define CONFIG_DDR_PLL2			0

/*
 * xxx_BOOT_SUPPORTED macro defines whether a booting type is supported
 * or not. Modify/Add to only these macros to define new boot types
 */
#define USB_BOOT_SUPPORTED		0
#define PCIE_BOOT_SUPPORTED		0
#define SNOR_BOOT_SUPPORTED		1
#define NAND_BOOT_SUPPORTED		1
#define PNOR_BOOT_SUPPORTED		0
#define TFTP_BOOT_SUPPORTED		0
#define UART_BOOT_SUPPORTED		0
#define SPI_BOOT_SUPPORTED		0
#define I2C_BOOT_SUPPORTED		0
#define MMC_BOOT_SUPPORTED		0

#endif  /* __CONFIG_H */
