/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2021 NXP
 */

#ifndef __LX2_COMMON_H
#define __LX2_COMMON_H

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

#define CFG_SYS_FLASH_BASE		0x20000000

/* DDR */
#define CFG_SYS_DDR_SDRAM_BASE		0x80000000UL
#define CFG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CFG_SYS_DDR_BLOCK2_BASE		0x2080000000ULL
#define CFG_SYS_SDRAM_SIZE			0x200000000UL
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define SPD_EEPROM_ADDRESS1		0x51
#define SPD_EEPROM_ADDRESS2		0x52
#define SPD_EEPROM_ADDRESS3		0x53
#define SPD_EEPROM_ADDRESS4		0x54
#define SPD_EEPROM_ADDRESS5		0x55
#define SPD_EEPROM_ADDRESS6		0x56
#define SPD_EEPROM_ADDRESS		SPD_EEPROM_ADDRESS1

/* Miscellaneous configurable options */

/* SMP Definitinos  */
#define CPU_RELEASE_ADDR		secondary_boot_addr

/* Generic Timer Definitions */
/*
 * This is not an accurate number. It is used in start.S. The frequency
 * will be udpated later when get_bus_freq(0) is available.
 */


/* Serial Port */
#define CFG_PL011_CLOCK		(get_bus_freq(0) / 4)
#define CFG_SYS_SERIAL0		0x21c0000
#define CFG_SYS_SERIAL1		0x21d0000
#define CFG_SYS_SERIAL2		0x21e0000
#define CFG_SYS_SERIAL3		0x21f0000
/*below might needs to be removed*/
#define CFG_PL01x_PORTS		{(void *)CFG_SYS_SERIAL0, \
					(void *)CFG_SYS_SERIAL1, \
					(void *)CFG_SYS_SERIAL2, \
					(void *)CFG_SYS_SERIAL3 }

/* MC firmware */
#define CFG_SYS_LS_MC_DPC_MAX_LENGTH		0x20000
#define CFG_SYS_LS_MC_DRAM_DPC_OFFSET	0x00F00000
#define CFG_SYS_LS_MC_DPL_MAX_LENGTH		0x20000
#define CFG_SYS_LS_MC_DRAM_DPL_OFFSET	0x00F20000
#define CFG_SYS_LS_MC_BOOT_TIMEOUT_MS	5000

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */
#ifdef CONFIG_FSL_MC_ENET
#define CFG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE	(256UL * 1024 * 1024)
#endif

/* I2C bus multiplexer */
#define I2C_MUX_PCA_ADDR_PRI		0x77 /* Primary Mux*/
#define I2C_MUX_CH_DEFAULT		0x8

/* RTC */
#define CFG_SYS_I2C_RTC_ADDR		0x51  /* Channel 3*/

/* Qixis */
#define CFG_SYS_I2C_FPGA_ADDR		0x66

/* USB */

#define COUNTER_FREQUENCY_REAL		(get_board_sys_clk() / 4)

#define HWCONFIG_BUFFER_SIZE		128

/* Initial environment variables */
#define XSPI_MC_INIT_CMD				\
	"sf probe 0:0 && "				\
	"sf read 0x80640000 0x640000 0x80000 && "	\
	"sf read $fdt_addr_r 0xf00000 0x100000 && "	\
	"env exists secureboot && "			\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000; "			\
	"sf read 0x80a00000 0xa00000 0x300000 && "	\
	"sf read 0x80e00000 0xe00000 0x100000; "	\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"

#define SD_MC_INIT_CMD				\
	"mmc read 0x80a00000 0x5000 0x1200;"	\
	"mmc read 0x80e00000 0x7000 0x800;"	\
	"mmc read $fdt_addr_r 0x7800 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80640000 0x3200 0x20 && "	\
	"mmc read 0x80680000 0x3400 0x20 && "	\
	"esbc_validate 0x80640000 && "		\
	"esbc_validate 0x80680000 ;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"

#define SD2_MC_INIT_CMD				\
	"mmc dev 1; mmc read 0x80a00000 0x5000 0x1200;"	\
	"mmc read 0x80e00000 0x7000 0x800;"	\
	"mmc read $fdt_addr_r 0x7800 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80640000 0x3200 0x20 && "	\
	"mmc read 0x80680000 0x3400 0x20 && "	\
	"esbc_validate 0x80640000 && "		\
	"esbc_validate 0x80680000 ;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"

#define EXTRA_ENV_SETTINGS			\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x1000000\0"		\
	"kernelheader_start=0x600000\0"		\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernelheader_size=0x40000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernelhdr_addr_sd=0x3000\0"            \
	"kernel_size_sd=0x14000\0"              \
	"kernelhdr_size_sd=0x20\0"              \
	"console=ttyAMA0,38400n8\0"		\
	BOOTENV					\
	"mcmemsize=0x70000000\0"		\
	XSPI_MC_INIT_CMD				\
	"scan_dev_for_boot_part="		\
		"part list ${devtype} ${devnum} devplist; "	\
		"env exists devplist || setenv devplist 1; "	\
		"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "			\
				"${devnum}:${distro_bootpart} "	\
				"bootfstype; then "		\
				"run scan_dev_for_boot; "	\
			"fi; "					\
		"done\0"					\
	"boot_a_script="					\
		"load ${devtype} ${devnum}:${distro_bootpart} "	\
			"${scriptaddr} ${prefix}${script}; "	\
		"env exists secureboot && load ${devtype} "	\
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr} "	\
			"&& esbc_validate ${scripthdraddr};"	\
		"source ${scriptaddr}\0"

#define XSPI_NOR_BOOTCOMMAND						\
			"sf probe 0:0; "				\
			"sf read 0x806c0000 0x6c0000 0x40000; "		\
			"env exists mcinitcmd && env exists secureboot"	\
			" && esbc_validate 0x806c0000; "		\
			"sf read 0x80d00000 0xd00000 0x100000; "	\
			"env exists mcinitcmd && "			\
			"fsl_mc lazyapply dpl 0x80d00000; "		\
			"run distro_bootcmd;run xspi_bootcmd; "		\
			"env exists secureboot && esbc_halt;"

#define SD_BOOTCOMMAND						\
		"env exists mcinitcmd && mmcinfo; "		\
		"mmc read 0x80d00000 0x6800 0x800; "		\
		"env exists mcinitcmd && env exists secureboot "	\
		" && mmc read 0x806C0000 0x3600 0x20 "		\
		"&& esbc_validate 0x806C0000;env exists mcinitcmd "	\
		"&& fsl_mc lazyapply dpl 0x80d00000;"		\
		"run distro_bootcmd;run sd_bootcmd;"		\
		"env exists secureboot && esbc_halt;"

#define SD2_BOOTCOMMAND						\
		"mmc dev 1; env exists mcinitcmd && mmcinfo; "	\
		"mmc read 0x80d00000 0x6800 0x800; "		\
		"env exists mcinitcmd && env exists secureboot "	\
		" && mmc read 0x806C0000 0x3600 0x20 "		\
		"&& esbc_validate 0x806C0000;env exists mcinitcmd "	\
		"&& fsl_mc lazyapply dpl 0x80d00000;"		\
		"run distro_bootcmd;run sd2_bootcmd;"		\
		"env exists secureboot && esbc_halt;"

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#ifdef CONFIG_MMC
#define BOOT_TARGET_DEVICES_MMC(func, instance) func(MMC, mmc, instance)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_SCSI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_CMD_DHCP
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_MMC(func, 0) \
	BOOT_TARGET_DEVICES_MMC(func, 1) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_DHCP(func)
#include <config_distro_bootcmd.h>

#endif /* __LX2_COMMON_H */
