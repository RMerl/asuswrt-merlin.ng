/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017, 2020-2021 NXP
 */

#ifndef __LS1088A_RDB_H
#define __LS1088A_RDB_H

#include "ls1088a_common.h"

#if defined(CONFIG_TFABOOT) || \
	defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
#define SYS_NO_FLASH
#endif

#define COUNTER_FREQUENCY_REAL		25000000	/* 25MHz */

#define SPD_EEPROM_ADDRESS	0x51


#if !defined(CONFIG_QSPI_BOOT) && !defined(CONFIG_SD_BOOT_QSPI)
#define CFG_SYS_NOR0_CSPR_EXT	(0x0)
#define CFG_SYS_NOR_AMASK		IFC_AMASK(128 * 1024 * 1024)
#define CFG_SYS_NOR_AMASK_EARLY	IFC_AMASK(64 * 1024 * 1024)

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
#define CFG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(6)
#define CFG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x1) | \
				FTIM0_NOR_TEADC(0x1) | \
				FTIM0_NOR_TEAHC(0x1))
#define CFG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x1) | \
				FTIM1_NOR_TRAD_NOR(0x1))
#define CFG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x0) | \
				FTIM2_NOR_TCH(0x0) | \
				FTIM2_NOR_TWP(0x1))
#define CFG_SYS_NOR_FTIM3	0x04000000
#define CFG_SYS_IFC_CCR	0x01000000

#ifndef SYS_NO_FLASH
#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE }
#endif
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
				| CSOR_NAND_PGS_2K	/* Page Size = 2K */ \
				| CSOR_NAND_SPRZ_64/* Spare size = 64 */ \
				| CSOR_NAND_PB(64))	/*Pages Per Block = 64*/

/* ONFI NAND Flash mode0 Timing Params */
#define CFG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x07) | \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x07) | \
					FTIM0_NAND_TWH(0x0a))
#define CFG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x32) | \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0x0e)   | \
					FTIM1_NAND_TRP(0x18))
#define CFG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x0f) | \
					FTIM2_NAND_TREH(0x0a) | \
					FTIM2_NAND_TWHRE(0x1e))
#define CFG_SYS_NAND_FTIM3		0x0

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

#define CFG_SYS_I2C_FPGA_ADDR	0x66
#define QIXIS_BRDCFG4_OFFSET            0x54
#define QIXIS_LBMAP_SWITCH		2
#define QIXIS_QMAP_MASK			0xe0
#define QIXIS_QMAP_SHIFT		5
#define QIXIS_LBMAP_MASK		0x1f
#define QIXIS_LBMAP_SHIFT		5
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x20
#define QIXIS_LBMAP_SD			0x00
#define QIXIS_LBMAP_EMMC		0x00
#define QIXIS_LBMAP_SD_QSPI		0x00
#define QIXIS_LBMAP_QSPI		0x00
#define QIXIS_RCW_SRC_SD		0x40
#define QIXIS_RCW_SRC_EMMC		0x41
#define QIXIS_RCW_SRC_QSPI		0x62
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define	QIXIS_RST_FORCE_MEM		0x01

#define CFG_SYS_FPGA_CSPR_EXT	(0x0)
#define CFG_SYS_FPGA_CSPR		(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS_EARLY) \
					| CSPR_PORT_SIZE_8 \
					| CSPR_MSEL_GPCM \
					| CSPR_V)
#define SYS_FPGA_CSPR_FINAL	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS) \
					| CSPR_PORT_SIZE_8 \
					| CSPR_MSEL_GPCM \
					| CSPR_V)

#define CFG_SYS_FPGA_AMASK		IFC_AMASK(64*1024)
#define CFG_SYS_FPGA_CSOR		CSOR_GPCM_ADM_SHIFT(0)
/* QIXIS Timing parameters*/
#define SYS_FPGA_CS_FTIM0	(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define SYS_FPGA_CS_FTIM1	(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define SYS_FPGA_CS_FTIM2	(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0x3E))
#define SYS_FPGA_CS_FTIM3	0x0

#if defined(CONFIG_TFABOOT) || \
	defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NAND_FTIM3
#define CFG_SYS_CSPR2_EXT		CFG_SYS_FPGA_CSPR_EXT
#define CFG_SYS_CSPR2		CFG_SYS_FPGA_CSPR
#define CFG_SYS_CSPR2_FINAL		SYS_FPGA_CSPR_FINAL
#define CFG_SYS_AMASK2		CFG_SYS_FPGA_AMASK
#define CFG_SYS_CSOR2		CFG_SYS_FPGA_CSOR
#define CFG_SYS_CS2_FTIM0		SYS_FPGA_CS_FTIM0
#define CFG_SYS_CS2_FTIM1		SYS_FPGA_CS_FTIM1
#define CFG_SYS_CS2_FTIM2		SYS_FPGA_CS_FTIM2
#define CFG_SYS_CS2_FTIM3		SYS_FPGA_CS_FTIM3
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
#endif

#define CFG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

#define I2C_MUX_CH_VOL_MONITOR         0xA
/* Voltage monitor on channel 2*/
#define I2C_VOL_MONITOR_ADDR           0x63
#define I2C_VOL_MONITOR_BUS_V_OFFSET   0x2
#define I2C_VOL_MONITOR_BUS_V_OVF      0x1
#define I2C_VOL_MONITOR_BUS_V_SHIFT    3
#define I2C_SVDD_MONITOR_ADDR		0x4F

/* The lowest and highest voltage allowed for LS1088ARDB */
#define VDD_MV_MIN			819
#define VDD_MV_MAX			1212

#define PWM_CHANNEL0                    0x0

/*
 * I2C bus multiplexer
 */
#define I2C_MUX_PCA_ADDR_PRI		0x77
#define I2C_MUX_PCA_ADDR_SEC		0x76 /* Secondary multiplexer */
#define I2C_RETIMER_ADDR		0x18
#define I2C_MUX_CH_DEFAULT		0x8
#define I2C_MUX_CH5			0xD

/*
* RTC configuration
*/
#define CFG_SYS_I2C_RTC_ADDR         0x51  /* Channel 3*/

#ifndef SPL_NO_ENV
/* Initial environment variables */
#ifdef CONFIG_TFABOOT
#define QSPI_MC_INIT_CMD				\
	"sf probe 0:0;sf read 0x80a00000 0xA00000 0x200000;"	\
	"sf read 0x80e00000 0xE00000 0x100000;"				\
	"env exists secureboot && "			\
	"sf read 0x80640000 0x640000 0x40000 && "	\
	"sf read 0x80680000 0x680000 0x40000 && "	\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000 ;"			\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"
#define SD_MC_INIT_CMD				\
	"mmcinfo;mmc read 0x80a00000 0x5000 0x1000;"		\
	"mmc read 0x80e00000 0x7000 0x800;"				\
	"env exists secureboot && "			\
	"mmc read 0x80640000 0x3200 0x20 && "		\
	"mmc read 0x80680000 0x3400 0x20 && "		\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000 ;"			\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"
#else
#if defined(CONFIG_QSPI_BOOT)
#define MC_INIT_CMD				\
	"mcinitcmd=sf probe 0:0;sf read 0x80a00000 0xA00000 0x200000;"	\
	"sf read 0x80e00000 0xE00000 0x100000;"				\
	"env exists secureboot && "			\
	"sf read 0x80640000 0x640000 0x40000 && "	\
	"sf read 0x80680000 0x680000 0x40000 && "	\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000 ;"			\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"	\
	"mcmemsize=0x70000000\0"
#elif defined(CONFIG_SD_BOOT)
#define MC_INIT_CMD				\
	"mcinitcmd=mmcinfo;mmc read 0x80a00000 0x5000 0x1000;"		\
	"mmc read 0x80e00000 0x7000 0x800;"				\
	"env exists secureboot && "			\
	"mmc read 0x80640000 0x3200 0x20 && "		\
	"mmc read 0x80680000 0x3400 0x20 && "		\
	"esbc_validate 0x80640000 && "			\
	"esbc_validate 0x80680000 ;"			\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"	\
	"mcmemsize=0x70000000\0"
#endif
#endif /* CONFIG_TFABOOT */

#undef CFG_EXTRA_ENV_SETTINGS
#ifdef CONFIG_TFABOOT
#define CFG_EXTRA_ENV_SETTINGS		\
	"BOARD=ls1088ardb\0"			\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_addr=0x1000000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernelhdr_addr_sd=0x3000\0"		\
	"kernel_start=0x580100000\0"		\
	"kernelheader_start=0x580600000\0"	\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr=0x600000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernelheader_size=0x40000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernel_size_sd=0x14000\0"		\
	"kernelhdr_size_sd=0x20\0"		\
	QSPI_MC_INIT_CMD			\
	"mcmemsize=0x70000000\0"		\
	BOOTENV					\
	"boot_scripts=ls1088ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1088ardb_bs.out\0"	\
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
		"load ${devtype} ${devnum}:${distro_bootpart} " \
		"${scriptaddr} ${prefix}${script}; "		\
	"env exists secureboot && load ${devtype} "		\
		"${devnum}:${distro_bootpart} "			\
		"${scripthdraddr} ${prefix}${boot_script_hdr}; "\
		"env exists secureboot "			\
		"&& esbc_validate ${scripthdraddr};"		\
		"source ${scriptaddr}\0"			\
	"installer=load mmc 0:2 $load_addr "			\
		"/flex_installer_arm64.itb; "			\
		"env exists mcinitcmd && run mcinitcmd && "	\
		"mmc read 0x80001000 0x6800 0x800;"		\
		"fsl_mc lazyapply dpl 0x80001000;"			\
		"bootm $load_addr#ls1088ardb\0"			\
	"qspi_bootcmd=echo Trying load from qspi..;"		\
		"sf probe && sf read $load_addr "		\
		"$kernel_addr $kernel_size ; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		"bootm $load_addr#$BOARD\0"			\
		"sd_bootcmd=echo Trying load from sd card..;"		\
		"mmcinfo; mmc read $load_addr "			\
		"$kernel_addr_sd $kernel_size_sd ;"		\
		"env exists secureboot && mmc read $kernelheader_addr_r "\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "	\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$BOARD\0"
#else
#define CFG_EXTRA_ENV_SETTINGS		\
	"BOARD=ls1088ardb\0"			\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_addr=0x1000000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernelhdr_addr_sd=0x3000\0"		\
	"kernel_start=0x580100000\0"		\
	"kernelheader_start=0x580800000\0"	\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr=0x600000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernelheader_size=0x40000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernel_size_sd=0x14000\0"		\
	"kernelhdr_size_sd=0x20\0"		\
	MC_INIT_CMD				\
	BOOTENV					\
	"boot_scripts=ls1088ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1088ardb_bs.out\0"	\
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
		"load ${devtype} ${devnum}:${distro_bootpart} " \
		"${scriptaddr} ${prefix}${script}; "		\
	"env exists secureboot && load ${devtype} "		\
		"${devnum}:${distro_bootpart} "			\
		"${scripthdraddr} ${prefix}${boot_script_hdr} " \
		"&& esbc_validate ${scripthdraddr};"		\
		"source ${scriptaddr}\0"			\
	"installer=load mmc 0:2 $load_addr "			\
		"/flex_installer_arm64.itb; "			\
		"env exists mcinitcmd && run mcinitcmd && "	\
		"mmc read 0x80001000 0x6800 0x800;"		\
		"fsl_mc lazyapply dpl 0x80001000;"			\
		"bootm $load_addr#ls1088ardb\0"			\
	"qspi_bootcmd=echo Trying load from qspi..;"		\
		"sf probe && sf read $load_addr "		\
		"$kernel_addr $kernel_size ; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		"bootm $load_addr#$BOARD\0"			\
		"sd_bootcmd=echo Trying load from sd card..;"		\
		"mmcinfo; mmc read $load_addr "			\
		"$kernel_addr_sd $kernel_size_sd ;"		\
		"env exists secureboot && mmc read $kernelheader_addr_r "\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "	\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$BOARD\0"
#endif /* CONFIG_TFABOOT */

#ifdef CONFIG_TFABOOT
#define QSPI_NOR_BOOTCOMMAND					\
	"sf read 0x80001000 0xd00000 0x100000;"		\
		"env exists mcinitcmd && env exists secureboot "	\
		" && sf read 0x806C0000 0x6C0000 0x100000 "	\
		"&& esbc_validate 0x806C0000;env exists mcinitcmd "	\
		"&& fsl_mc lazyapply dpl 0x80001000;"		\
		"run distro_bootcmd;run qspi_bootcmd;"		\
		"env exists secureboot && esbc_halt;"
#define SD_BOOTCOMMAND						\
		"env exists mcinitcmd && mmcinfo; "		\
		"mmc read 0x80001000 0x6800 0x800; "		\
		"env exists mcinitcmd && env exists secureboot "	\
		" && mmc read 0x806C0000 0x3600 0x20 "		\
		"&& esbc_validate 0x806C0000;env exists mcinitcmd "	\
		"&& fsl_mc lazyapply dpl 0x80001000;"		\
		"run distro_bootcmd;run sd_bootcmd;"		\
		"env exists secureboot && esbc_halt;"
#else
#if defined(CONFIG_QSPI_BOOT)
/* Try to boot an on-QSPI kernel first, then do normal distro boot */

/* Try to boot an on-SD kernel first, then do normal distro boot */
#endif
#endif /* CONFIG_TFABOOT */

/* MAC/PHY configuration */
#ifdef CONFIG_FSL_MC_ENET
#define AQ_PHY_ADDR1			0x00
#define AQR105_IRQ_MASK			0x00000004

#define QSGMII1_PORT1_PHY_ADDR		0x0c
#define QSGMII1_PORT2_PHY_ADDR		0x0d
#define QSGMII1_PORT3_PHY_ADDR		0x0e
#define QSGMII1_PORT4_PHY_ADDR		0x0f
#define QSGMII2_PORT1_PHY_ADDR		0x1c
#define QSGMII2_PORT2_PHY_ADDR		0x1d
#define QSGMII2_PORT3_PHY_ADDR		0x1e
#define QSGMII2_PORT4_PHY_ADDR		0x1f
#endif
#endif

#ifndef SPL_NO_ENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#endif

#include <asm/fsl_secure_boot.h>

#endif /* __LS1088A_RDB_H */
