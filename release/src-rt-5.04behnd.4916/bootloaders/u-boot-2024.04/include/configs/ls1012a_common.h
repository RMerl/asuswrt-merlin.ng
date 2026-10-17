/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor
 */

#ifndef __LS1012A_COMMON_H
#define __LS1012A_COMMON_H

#include <asm/arch/config.h>
#include <asm/arch/stream_id_lsch2.h>
#include <linux/sizes.h>

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000
#define CFG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define CFG_SYS_DDR_BLOCK2_BASE     0x880000000ULL

/*SPI device */
#define CFG_SYS_FSL_QSPI_BASE	0x40000000

/* I2C */

/* GPIO */

#define CFG_SYS_NS16550_CLK          (get_serial_clock())

#define HWCONFIG_BUFFER_SIZE		128

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS		\
	"verify=no\0"				\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x1000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"bootm_size=0x10000000\0"		\

#ifdef CONFIG_TFABOOT
#define QSPI_NOR_BOOTCOMMAND	"sf probe 0:0; sf read $kernel_load "\
				"$kernel_start $kernel_size && "\
				"bootm $kernel_load"
#endif

#include <asm/arch/soc.h>

#endif /* __LS1012A_COMMON_H */
