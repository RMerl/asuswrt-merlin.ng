/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SYS_FSL_CLK

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * SZ_1M)

/* USB Configs */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_EEPROM_ADDR 0x50
#define CONFIG_SYS_EEPROM_BUS_NUM 1
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command definition */
#define CONFIG_LOADADDR		0x72000000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc1,115200\0"	\
	"fdt_addr=0x75000000\0"		\
	"fdt_high=0xffffffff\0"		\
	"scriptaddr=0x74000000\0"	\
	"kernel_file=fitImage\0"\
	"silent=1\0"\
	"rdinit=/sbin/init\0" \
	"addinitrd=setenv bootargs ${bootargs} rdinit=${rdinit} ${debug} \0" \
	"upd_image=st.4k\0" \
	"uboot_file=u-boot.imx\0" \
	"updargs=setenv bootargs console=${console} ${smp} ${displayargs}\0" \
	"initrd_ram_dev=/dev/ram\0" \
	"addswupdate=setenv bootargs ${bootargs} root=${initrd_ram_dev} rw\0" \
	"addkeys=setenv bootargs ${bootargs} di=${dig_in} key1=${key1}\0" \
	"loadusb=usb start; " \
	       "fatload usb 0 ${loadaddr} ${upd_image}\0" \
	"up=if tftp ${loadaddr} ${uboot_file}; then " \
	       "setexpr blkc ${filesize} / 0x200; " \
	       "setexpr blkc ${blkc} + 1; " \
	       "mmc write ${loadaddr} 0x2 ${blkc}" \
	"; fi\0"	  \
	"upwic=setenv wic_file kp-image-kp${boardsoc}.wic; "\
	       "if tftp ${loadaddr} ${wic_file}; then " \
	       "setexpr blkc ${filesize} / 0x200; " \
	       "setexpr blkc ${blkc} + 1; " \
	       "mmc write ${loadaddr} 0x0 ${blkc}" \
	"; fi\0"	  \
	"usbupd=echo Booting update from usb ...; " \
	       "setenv bootargs; " \
	       "run updargs; " \
	       "run addinitrd; " \
	       "run addswupdate; " \
	       "run addkeys; " \
	       "run loadusb; " \
	       "bootm ${loadaddr}#${fit_config}\0" \
	BOOTENV

#define CONFIG_BOOTCOMMAND		"run usbupd; run distro_bootcmd"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_ARP_TIMEOUT	200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* Physical Memory Map */
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(512 * SZ_1M)
#define PHYS_SDRAM_SIZE		(PHYS_SDRAM_1_SIZE)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */
#define CONFIG_ENV_OFFSET      (SZ_1M)
#define CONFIG_ENV_SIZE        (SZ_8K)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SIZE_REDUND CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND        (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV 0

#endif				/* __CONFIG_H_ */
