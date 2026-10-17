/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017, 2019-2021 NXP
 * Copyright 2015 Freescale Semiconductor
 */

#ifndef __LS2_RDB_H
#define __LS2_RDB_H

#include "ls2080a_common.h"

#define I2C_MUX_CH_VOL_MONITOR		0xa
#define I2C_VOL_MONITOR_ADDR		0x38

/* step the IR regulator in 5mV increments */
#define IR_VDD_STEP_DOWN		5
#define IR_VDD_STEP_UP			5
/* The lowest and highest voltage allowed for LS2080ARDB */
#define VDD_MV_MIN			819
#define VDD_MV_MAX			1212

#define COUNTER_FREQUENCY_REAL		(get_board_sys_clk()/4)

#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define SPD_EEPROM_ADDRESS3	0x53
#define SPD_EEPROM_ADDRESS4	0x54
#define SPD_EEPROM_ADDRESS5	0x55
#define SPD_EEPROM_ADDRESS6	0x56	/* dummy address */
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1

#if !defined(CONFIG_FSL_QSPI) || defined(CONFIG_TFABOOT)

#define CFG_SYS_NOR0_CSPR_EXT	(0x0)
#define CFG_SYS_NOR_AMASK		IFC_AMASK(128*1024*1024)
#define CFG_SYS_NOR_AMASK_EARLY	IFC_AMASK(64*1024*1024)

#define CFG_SYS_NOR0_CSPR					\
	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS)		| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CFG_SYS_NOR0_CSPR_EARLY				\
	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS_EARLY)	| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CFG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(12)
#define CFG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5))
#define CFG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x35) | \
				FTIM1_NOR_TRAD_NOR(0x1a) |\
				FTIM1_NOR_TSEQRAD_NOR(0x13))
#define CFG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWPH(0x0E) | \
				FTIM2_NOR_TWP(0x1c))
#define CFG_SYS_NOR_FTIM3	0x04000000
#define CFG_SYS_IFC_CCR	0x01000000

#ifdef CONFIG_MTD_NOR_FLASH
#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE,\
					 CFG_SYS_FLASH_BASE + 0x40000000}
#endif

#define CFG_SYS_NAND_CSPR_EXT	(0x0)
#define CFG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 /* Port Size = 8 bit */ \
				| CSPR_MSEL_NAND	/* MSEL = NAND */ \
				| CSPR_V)
#define CFG_SYS_NAND_AMASK	IFC_AMASK(64 * 1024)

#define CFG_SYS_NAND_CSOR    (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3Byes */ \
				| CSOR_NAND_PGS_4K	/* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224	/* Spare size = 224 */ \
				| CSOR_NAND_PB(128))	/* Pages Per Block 128*/

/* ONFI NAND Flash mode0 Timing Params */
#define CFG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x0e) | \
					FTIM0_NAND_TWP(0x30)   | \
					FTIM0_NAND_TWCHT(0x0e) | \
					FTIM0_NAND_TWH(0x14))
#define CFG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x64) | \
					FTIM1_NAND_TWBE(0xab)  | \
					FTIM1_NAND_TRR(0x1c)   | \
					FTIM1_NAND_TRP(0x30))
#define CFG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x1e) | \
					FTIM2_NAND_TREH(0x14) | \
					FTIM2_NAND_TWHRE(0x3c))
#define CFG_SYS_NAND_FTIM3		0x0

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

#define QIXIS_LBMAP_SWITCH		0x06
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_LBMAP_SHIFT		0
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x04
#define QIXIS_LBMAP_NAND		0x09
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RST_CTL_RESET_EN		0x30
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_RCW_SRC_NAND		0x119
#define	QIXIS_RST_FORCE_MEM		0x01

#define CFG_SYS_CSPR3_EXT	(0x0)
#define CFG_SYS_CSPR3	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS_EARLY) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)
#define CFG_SYS_CSPR3_FINAL	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)

#define CFG_SYS_AMASK3	IFC_AMASK(64*1024)
#define CFG_SYS_CSOR3	CSOR_GPCM_ADM_SHIFT(12)
/* QIXIS Timing parameters for IFC CS3 */
#define CFG_SYS_CS3_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CFG_SYS_CS3_FTIM1		(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define CFG_SYS_CS3_FTIM2		(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0x3E))
#define CFG_SYS_CS3_FTIM3		0x0

#if defined(CONFIG_SPL) && defined(CONFIG_MTD_RAW_NAND)
#define CFG_SYS_CSPR2_EXT		CFG_SYS_NOR0_CSPR_EXT
#define CFG_SYS_CSPR2		CFG_SYS_NOR0_CSPR_EARLY
#define CFG_SYS_CSPR2_FINAL		CFG_SYS_NOR0_CSPR
#define CFG_SYS_AMASK2		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR2		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS2_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS2_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS2_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS2_FTIM3		CFG_SYS_NOR_FTIM3
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NAND_FTIM3

#define CFG_SYS_NAND_U_BOOT_SIZE	(512 * 1024)
#else
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NOR0_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NOR0_CSPR_EARLY
#define CFG_SYS_CSPR0_FINAL		CFG_SYS_NOR0_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NOR_FTIM3
#define CFG_SYS_CSPR2_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR2		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK2		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR2		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS2_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS2_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS2_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS2_FTIM3		CFG_SYS_NAND_FTIM3
#endif
#endif
#define CFG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

#ifdef CONFIG_TARGET_LS2081ARDB
#define QIXIS_QMAP_MASK			0x07
#define QIXIS_QMAP_SHIFT		5
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_QSPI		0x00
#define QIXIS_RCW_SRC_QSPI		0x62
#define QIXIS_LBMAP_ALTBANK		0x20
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_RST_CTL_RESET_EN		0x30
#endif

/*
 * I2C
 */
#ifdef CONFIG_TARGET_LS2081ARDB
#define CFG_SYS_I2C_FPGA_ADDR	0x66
#endif
#define I2C_MUX_PCA_ADDR		0x75
#define I2C_MUX_PCA_ADDR_PRI		0x75 /* Primary Mux*/

/* I2C bus multiplexer */
#define I2C_MUX_CH_DEFAULT      0x8

/* SPI */

/*
 * RTC configuration
 */
#ifdef CONFIG_TARGET_LS2081ARDB
#define CFG_SYS_I2C_RTC_ADDR         0x51
#else
#define CFG_SYS_I2C_RTC_ADDR         0x68
#endif

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#ifdef CONFIG_TFABOOT
#define QSPI_MC_INIT_CMD				\
	"sf probe 0:0; "				\
	"sf read 0x80640000 0x640000 0x80000; "		\
	"env exists secureboot && "			\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000; "			\
	"sf read 0x80a00000 0xa00000 0x200000; "	\
	"sf read 0x80e00000 0xe00000 0x100000; "	\
	"fsl_mc start mc 0x80a00000 0x80e00000 \0"
#define SD_MC_INIT_CMD				\
	"mmcinfo;mmc read 0x80a00000 0x5000 0x1000;" \
	"mmc read 0x80e00000 0x7000 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80640000 0x3200 0x20 && "	\
	"mmc read 0x80680000 0x3400 0x20 && "	\
	"esbc_validate 0x80640000 && "		\
	"esbc_validate 0x80680000 ;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"
#define IFC_MC_INIT_CMD				\
	"env exists secureboot && "	\
	"esbc_validate 0x580640000 && "		\
	"esbc_validate 0x580680000; "		\
	"fsl_mc start mc 0x580a00000 0x580e00000 \0"
#else
#ifdef CONFIG_QSPI_BOOT
#define MC_INIT_CMD					\
	"mcinitcmd=sf probe 0:0; "			\
	"sf read 0x80640000 0x640000 0x80000; "		\
	"env exists secureboot && "			\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000; "			\
	"sf read 0x80a00000 0xa00000 0x200000; "	\
	"sf read 0x80e00000 0xe00000 0x100000; "	\
	"fsl_mc start mc 0x80a00000 0x80e00000 \0"
#elif defined(CONFIG_SD_BOOT)
#define MC_INIT_CMD                             \
	"mcinitcmd=mmcinfo;mmc read 0x80a00000 0x5000 0x1000;" \
	"mmc read 0x80e00000 0x7000 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80640000 0x3200 0x20 && "	\
	"mmc read 0x80680000 0x3400 0x20 && "	\
	"esbc_validate 0x80640000 && "		\
	"esbc_validate 0x80680000 ;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0" \
	"mcmemsize=0x70000000\0"
#else
#define MC_INIT_CMD				\
	"mcinitcmd=env exists secureboot && "	\
	"esbc_validate 0x580640000 && "		\
	"esbc_validate 0x580680000; "		\
	"fsl_mc start mc 0x580a00000 0x580e00000 \0"
#endif
#endif

/* Initial environment variables */
#undef CFG_EXTRA_ENV_SETTINGS
#ifdef CONFIG_TFABOOT
#define CFG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_addr=0x581000000\0"		\
	"kernel_start=0x1000000\0"		\
	"kernelheader_start=0x800000\0"		\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernelheader_addr=0x580600000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernelheader_size=0x40000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernel_size_sd=0x14000\0"              \
	"console=ttyAMA0,38400n8\0"		\
	"mcmemsize=0x70000000\0"		\
	"sd_bootcmd=echo Trying load from SD ..;" \
	"mmcinfo; mmc read $load_addr "		\
	"$kernel_addr_sd $kernel_size_sd && "	\
	"bootm $load_addr#$board\0"             \
	QSPI_MC_INIT_CMD				\
	BOOTENV					\
	"boot_scripts=ls2088ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls2088ardb_bs.out\0"	\
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
		"source ${scriptaddr}\0"			\
	"qspi_bootcmd=echo Trying load from qspi..;"		\
		"sf probe && sf read $load_addr "		\
		"$kernel_start $kernel_size ; env exists secureboot &&"	\
		"sf read $kernelheader_addr_r $kernelheader_start "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		" bootm $load_addr#$board\0"			\
	"nor_bootcmd=echo Trying load from nor..;"		\
		"cp.b $kernel_addr $load_addr "			\
		"$kernel_size ; env exists secureboot && "	\
		"cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		"bootm $load_addr#$board\0"
#else
#define CFG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_addr=0x581000000\0"		\
	"kernel_start=0x1000000\0"		\
	"kernelheader_start=0x600000\0"		\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernelheader_addr=0x580600000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernelheader_size=0x40000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernel_size_sd=0x14000\0"              \
	"console=ttyAMA0,38400n8\0"		\
	"mcmemsize=0x70000000\0"		\
	"sd_bootcmd=echo Trying load from SD ..;" \
	"mmcinfo; mmc read $load_addr "		\
	"$kernel_addr_sd $kernel_size_sd && "	\
	"bootm $load_addr#$board\0"             \
	MC_INIT_CMD				\
	BOOTENV					\
	"boot_scripts=ls2088ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls2088ardb_bs.out\0"	\
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
			"${scripthdraddr} ${prefix}${boot_script_hdr}; " \
			"env exists secureboot "	\
			"&& esbc_validate ${scripthdraddr};"	\
		"source ${scriptaddr}\0"			\
	"qspi_bootcmd=echo Trying load from qspi..;"		\
		"sf probe && sf read $load_addr "		\
		"$kernel_start $kernel_size ; env exists secureboot &&"	\
		"sf read $kernelheader_addr_r $kernelheader_start "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		" bootm $load_addr#$board\0"			\
	"nor_bootcmd=echo Trying load from nor..;"		\
		"cp.b $kernel_addr $load_addr "			\
		"$kernel_size ; env exists secureboot && "	\
		"cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		"bootm $load_addr#$board\0"
#endif

#ifdef CONFIG_TFABOOT
#define QSPI_NOR_BOOTCOMMAND						\
			"sf probe 0:0; "				\
			"sf read 0x806c0000 0x6c0000 0x40000; "		\
			"env exists mcinitcmd && env exists secureboot "\
			"&& esbc_validate 0x806c0000; "			\
			"sf read 0x80d00000 0xd00000 0x100000; "	\
			"env exists mcinitcmd && "			\
			"fsl_mc lazyapply dpl 0x80d00000; "		\
			"run distro_bootcmd;run qspi_bootcmd; "		\
			"env exists secureboot && esbc_halt;"

/* Try to boot an on-SD kernel first, then do normal distro boot */
#define SD_BOOTCOMMAND						\
			"env exists mcinitcmd && env exists secureboot "\
			"&& mmcinfo && mmc read $load_addr 0x3600 0x800 " \
			"&& esbc_validate $load_addr; "			\
			"env exists mcinitcmd && run mcinitcmd "	\
			"&& mmc read 0x80d00000 0x6800 0x800 "		\
			"&& fsl_mc lazyapply dpl 0x80d00000; "		\
			"run distro_bootcmd;run sd_bootcmd; "		\
			"env exists secureboot && esbc_halt;"

/* Try to boot an on-NOR kernel first, then do normal distro boot */
#define IFC_NOR_BOOTCOMMAND						\
			"env exists mcinitcmd && env exists secureboot "\
			"&& esbc_validate 0x5806C0000; env exists mcinitcmd "\
			"&& fsl_mc lazyapply dpl 0x580d00000;"		\
			"run distro_bootcmd;run nor_bootcmd; "		\
			"env exists secureboot && esbc_halt;"
#else
#ifdef CONFIG_QSPI_BOOT
/* Try to boot an on-QSPI kernel first, then do normal distro boot */
#elif defined(CONFIG_SD_BOOT)
/* Try to boot an on-SD kernel first, then do normal distro boot */
#else
/* Try to boot an on-NOR kernel first, then do normal distro boot */
#endif
#endif

/* MAC/PHY configuration */
#define CORTINA_PHY_ADDR1	0x10
#define CORTINA_PHY_ADDR2	0x11
#define CORTINA_PHY_ADDR3	0x12
#define CORTINA_PHY_ADDR4	0x13
#define AQ_PHY_ADDR1		0x00
#define AQ_PHY_ADDR2		0x01
#define AQ_PHY_ADDR3		0x02
#define AQ_PHY_ADDR4		0x03
#define AQR405_IRQ_MASK		0x36

#include <asm/fsl_secure_boot.h>

#endif /* __LS2_RDB_H */
