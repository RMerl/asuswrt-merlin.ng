/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Amazon Kindle Fire (first generation) codename kc1 config
 *
 * Copyright (C) 2016 Paul Kocialkowski <contact@paulk.fr>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/*
 * Build
 */

/*
 * CPU
 */

#define CONFIG_SYS_L2_PL310		1
#define CONFIG_SYS_PL310_BASE		0x48242000

/*
 * Board
 */

/*
 * Clocks
 */

#define CONFIG_SYS_TIMERBASE	GPT2_BASE
#define CONFIG_SYS_PTV		2

/*
 * DRAM
 */

/*
 * Memory
 */

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR		(NON_SECURE_SRAM_END - \
					 GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024 + CONFIG_ENV_SIZE)

/*
 * I2C
 */

#define CONFIG_SYS_I2C
#define CONFIG_I2C_MULTI_BUS

/*
 * Power
 */

#define CONFIG_TWL6030_POWER

/*
 * Input
 */

#define CONFIG_TWL6030_INPUT

/*
 * SPL
 */

#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)
#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		(512 * 1024)
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	(1024 * 1024)

/*
 * Console
 */

#define CONFIG_SYS_CBSIZE	512

/*
 * Serial
 */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000
#define CONFIG_SYS_NS16550_COM3		UART3_BASE

#define CONFIG_SYS_BAUDRATE_TABLE	{ 4800, 9600, 19200, 38400, 57600, \
					  115200 }

/*
 * USB gadget
 */

/*
 * Environment
 */

#define CONFIG_ENV_SIZE		(128 * 1024)

#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x82000000\0" \
	"loadaddr=0x82000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"fdtaddr=0x88000000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"scriptaddr=0x80000000\0" \
	"bootm_size=0x10000000\0" \
	"boot_mmc_dev=0\0" \
	"kernel_mmc_part=7\0" \
	"recovery_mmc_part=5\0" \
	"fdtfile=omap4-kc1.dtb\0" \
	"bootfile=/boot/extlinux/extlinux.conf\0" \
	"bootargs=console=ttyO2,115200 mem=512M\0"

/*
 * ATAGs
 */

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SERIAL_TAG

/*
 * Boot
 */

#define CONFIG_SYS_LOAD_ADDR	0x82000000

#define CONFIG_BOOTCOMMAND \
	"setenv boot_mmc_part ${kernel_mmc_part}; " \
	"if test reboot-${reboot-mode} = reboot-r; then " \
	"echo recovery; setenv boot_mmc_part ${recovery_mmc_part}; fi; " \
	"if test reboot-${reboot-mode} = reboot-b; then " \
	"echo fastboot; fastboot 0; fi; " \
	"part start mmc ${boot_mmc_dev} ${boot_mmc_part} boot_mmc_start; " \
	"part size mmc ${boot_mmc_dev} ${boot_mmc_part} boot_mmc_size; " \
	"mmc dev ${boot_mmc_dev}; " \
	"mmc read ${kernel_addr_r} ${boot_mmc_start} ${boot_mmc_size} && " \
	"bootm ${kernel_addr_r};"

/*
 * Defaults
 */

#include <config_defaults.h>

#endif
