/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __LX2_COMMON_H
#define __LX2_COMMON_H

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

#define CONFIG_REMAKE_ELF
#define CONFIG_FSL_LAYERSCAPE
#define CONFIG_GICV3
#define CONFIG_FSL_TZPC_BP147
#define CONFIG_FSL_MEMAC

#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_FLASH_BASE		0x20000000

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F	1

/* DDR */
#define CONFIG_FSL_DDR_INTERACTIVE	/* Interactive debugging */
#define CONFIG_SYS_FSL_DDR_INTLV_256B	/* force 256 byte interleaving */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE		0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_DDR_BLOCK2_BASE		0x2080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	2
#define CONFIG_SYS_SDRAM_SIZE			0x200000000UL
#define CONFIG_DDR_SPD
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define SPD_EEPROM_ADDRESS1		0x51
#define SPD_EEPROM_ADDRESS2		0x52
#define SPD_EEPROM_ADDRESS3		0x53
#define SPD_EEPROM_ADDRESS4		0x54
#define SPD_EEPROM_ADDRESS5		0x55
#define SPD_EEPROM_ADDRESS6		0x56
#define SPD_EEPROM_ADDRESS		SPD_EEPROM_ADDRESS1
#define CONFIG_SYS_SPD_BUS_NUM		0	/* SPD on I2C bus 0 */
#define CONFIG_DIMM_SLOTS_PER_CTLR	2
#define CONFIG_CHIP_SELECTS_PER_CTRL	4
#define CONFIG_FSL_DDR_BIST	/* enable built-in memory test */
#define CONFIG_SYS_MONITOR_LEN		(936 * 1024)

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)

/* SMP Definitinos  */
#define CPU_RELEASE_ADDR		secondary_boot_func

/* Generic Timer Definitions */
/*
 * This is not an accurate number. It is used in start.S. The frequency
 * will be udpated later when get_bus_freq(0) is available.
 */

#define COUNTER_FREQUENCY		25000000	/* 25MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2048 * 1024)

/* Serial Port */
#define CONFIG_PL01X_SERIAL
#define CONFIG_PL011_CLOCK		(get_bus_freq(0) / 4)
#define CONFIG_SYS_SERIAL0		0x21c0000
#define CONFIG_SYS_SERIAL1		0x21d0000
#define CONFIG_SYS_SERIAL2		0x21e0000
#define CONFIG_SYS_SERIAL3		0x21f0000
/*below might needs to be removed*/
#define CONFIG_PL01x_PORTS		{(void *)CONFIG_SYS_SERIAL0, \
					(void *)CONFIG_SYS_SERIAL1, \
					(void *)CONFIG_SYS_SERIAL2, \
					(void *)CONFIG_SYS_SERIAL3 }
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* MC firmware */
#define CONFIG_SYS_LS_MC_DPC_MAX_LENGTH		0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET	0x00F00000
#define CONFIG_SYS_LS_MC_DPL_MAX_LENGTH		0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET	0x00F20000
#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS	5000

/* Define phy_reset function to boot the MC based on mcinitcmd.
 * This happens late enough to properly fixup u-boot env MAC addresses.
 */
#define CONFIG_RESET_PHY_R

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */
#ifdef CONFIG_FSL_MC_ENET
#define CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE	(256UL * 1024 * 1024)
#endif

/* I2C bus multiplexer */
#define I2C_MUX_PCA_ADDR_PRI		0x77 /* Primary Mux*/
#define I2C_MUX_CH_DEFAULT		0x8

/* RTC */
#define RTC
#define CONFIG_SYS_I2C_RTC_ADDR		0x51  /* Channel 3*/

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM		0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5

/* Qixis */
#define CONFIG_FSL_QIXIS
#define CONFIG_QIXIS_I2C_ACCESS
#define CONFIG_SYS_I2C_FPGA_ADDR		0x66

/* PCI */
#ifdef CONFIG_PCI
#define CONFIG_SYS_PCI_64BIT
#define CONFIG_PCI_SCAN_SHOW
#endif

/* MMC */
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

/* SATA */

#ifdef CONFIG_SCSI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SATA1		AHCI_BASE_ADDR1
#define CONFIG_SYS_SATA2		AHCI_BASE_ADDR2
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					CONFIG_SYS_SCSI_MAX_LUN)
#endif

/* USB */
#ifdef CONFIG_USB
#define CONFIG_HAS_FSL_XHCI_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#endif

/* FlexSPI */
#ifdef CONFIG_NXP_FSPI
#define NXP_FSPI_FLASH_SIZE		SZ_64M
#define NXP_FSPI_FLASH_NUM		1
#endif

#ifndef __ASSEMBLY__
unsigned long get_board_sys_clk(void);
unsigned long get_board_ddr_clk(void);
#endif

#define CONFIG_SYS_CLK_FREQ		get_board_sys_clk()
#define CONFIG_DDR_CLK_FREQ		get_board_ddr_clk()
#define COUNTER_FREQUENCY_REAL		(CONFIG_SYS_CLK_FREQ / 4)

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_ENV_SIZE			0x2000          /* 8KB */
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_OFFSET		0x500000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + \
					 CONFIG_ENV_OFFSET)

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot args buffer */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

/* Initial environment variables */
#define XSPI_MC_INIT_CMD			\
	"env exists secureboot && "		\
	"esbc_validate 0x20700000 && "		\
	"esbc_validate 0x20740000 ;"		\
	"fsl_mc start mc 0x20a00000 0x20e00000\0"

#define SD_MC_INIT_CMD				\
	"mmc read 0x80000000 0x5000 0x800;"	\
	"mmc read 0x80100000 0x7000 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80700000 0x3800 0x10 && "	\
	"mmc read 0x80740000 0x3A00 0x10 && "	\
	"esbc_validate 0x80700000 && "		\
	"esbc_validate 0x80740000 ;"		\
	"fsl_mc start mc 0x80000000 0x80100000\0"

#define EXTRA_ENV_SETTINGS			\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"fdt_addr=0x64f00000\0"			\
	"kernel_start=0x1000000\0"		\
	"kernelheader_start=0x7C0000\0"		\
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
	"kernelhdr_addr_sd=0x3E00\0"            \
	"kernel_size_sd=0x1d000\0"              \
	"kernelhdr_size_sd=0x10\0"              \
	"console=ttyAMA0,38400n8\0"		\
	BOOTENV					\
	"mcmemsize=0x70000000\0"		\
	XSPI_MC_INIT_CMD				\
	"boot_scripts=lx2160ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_lx2160ardb_bs.out\0"	\
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
			"env exists mcinitcmd && env exists secureboot "\
			"&& esbc_validate 0x20780000; "			\
			"env exists mcinitcmd && "			\
			"fsl_mc lazyapply dpl 0x20d00000; "		\
			"run distro_bootcmd;run xspi_bootcmd; "		\
			"env exists secureboot && esbc_halt;"

#define SD_BOOTCOMMAND						\
		"env exists mcinitcmd && mmcinfo; "		\
		"mmc read 0x80001000 0x6800 0x800; "		\
		"env exists mcinitcmd && env exists secureboot "	\
		" && mmc read 0x80780000 0x3C00 0x10 "		\
		"&& esbc_validate 0x80780000;env exists mcinitcmd "	\
		"&& fsl_mc lazyapply dpl 0x80001000;"		\
		"run distro_bootcmd;run sd_bootcmd;"		\
		"env exists secureboot && esbc_halt;"

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 0) \
	func(SCSI, scsi, 0)
#include <config_distro_bootcmd.h>

#endif /* __LX2_COMMON_H */
