/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017, 2019 NXP
 * Copyright 2015 Freescale Semiconductor
 */

#ifndef __LS2_QDS_H
#define __LS2_QDS_H

#include "ls2080a_common.h"

#ifndef __ASSEMBLY__
unsigned long get_board_sys_clk(void);
unsigned long get_board_ddr_clk(void);
#endif

#ifdef CONFIG_FSL_QSPI
#define CONFIG_QIXIS_I2C_ACCESS
#define CONFIG_SYS_I2C_EARLY_INIT
#define CONFIG_SYS_I2C_IFDR_DIV		0x7e
#endif

#define CONFIG_SYS_I2C_FPGA_ADDR	0x66
#define CONFIG_SYS_CLK_FREQ		get_board_sys_clk()
#define CONFIG_DDR_CLK_FREQ		get_board_ddr_clk()
#define COUNTER_FREQUENCY_REAL		(CONFIG_SYS_CLK_FREQ/4)

#define CONFIG_DDR_SPD
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define SPD_EEPROM_ADDRESS3	0x53
#define SPD_EEPROM_ADDRESS4	0x54
#define SPD_EEPROM_ADDRESS5	0x55
#define SPD_EEPROM_ADDRESS6	0x56	/* dummy address */
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1
#define CONFIG_SYS_SPD_BUS_NUM	0	/* SPD on I2C bus 0 */
#define CONFIG_DIMM_SLOTS_PER_CTLR		2
#define CONFIG_CHIP_SELECTS_PER_CTRL		4
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
#define CONFIG_DP_DDR_DIMM_SLOTS_PER_CTLR	1
#endif

/* SATA */
#define CONFIG_SCSI_AHCI_PLAT

#define CONFIG_SYS_SATA1			AHCI_BASE_ADDR1
#define CONFIG_SYS_SATA2			AHCI_BASE_ADDR2

#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)

#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_OFFSET		0x500000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + \
					 CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SECT_SIZE		0x20000
#endif

#define CONFIG_SYS_NOR0_CSPR_EXT	(0x0)
#define CONFIG_SYS_NOR_AMASK		IFC_AMASK(128*1024*1024)
#define CONFIG_SYS_NOR_AMASK_EARLY	IFC_AMASK(64*1024*1024)

#define CONFIG_SYS_NOR0_CSPR					\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS)		| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR0_CSPR_EARLY				\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS_EARLY)	| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR1_CSPR					\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH1_BASE_PHYS)		| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR1_CSPR_EARLY				\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH1_BASE_PHYS_EARLY)	| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(12)
#define CONFIG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5))
#define CONFIG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x35) | \
				FTIM1_NOR_TRAD_NOR(0x1a) |\
				FTIM1_NOR_TSEQRAD_NOR(0x13))
#define CONFIG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWPH(0x0E) | \
				FTIM2_NOR_TWP(0x1c))
#define CONFIG_SYS_NOR_FTIM3	0x04000000
#define CONFIG_SYS_IFC_CCR	0x01000000

#ifdef CONFIG_MTD_NOR_FLASH
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE,\
					 CONFIG_SYS_FLASH_BASE + 0x40000000}
#endif

#define CONFIG_NAND_FSL_IFC
#define CONFIG_SYS_NAND_MAX_ECCPOS	256
#define CONFIG_SYS_NAND_MAX_OOBFREE	2

#define CONFIG_SYS_NAND_CSPR_EXT	(0x0)
#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 /* Port Size = 8 bit */ \
				| CSPR_MSEL_NAND	/* MSEL = NAND */ \
				| CSPR_V)
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64 * 1024)

#define CONFIG_SYS_NAND_CSOR    (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3Byes */ \
				| CSOR_NAND_PGS_2K	/* Page Size = 2K */ \
				| CSOR_NAND_SPRZ_64/* Spare size = 64 */ \
				| CSOR_NAND_PB(64))	/*Pages Per Block = 64*/

#define CONFIG_SYS_NAND_ONFI_DETECTION

/* ONFI NAND Flash mode0 Timing Params */
#define CONFIG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x07) | \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x07) | \
					FTIM0_NAND_TWH(0x0a))
#define CONFIG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x32) | \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0x0e)   | \
					FTIM1_NAND_TRP(0x18))
#define CONFIG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x0f) | \
					FTIM2_NAND_TREH(0x0a) | \
					FTIM2_NAND_TWHRE(0x1e))
#define CONFIG_SYS_NAND_FTIM3		0x0

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE

#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)

#define CONFIG_FSL_QIXIS	/* use common QIXIS code */
#define QIXIS_LBMAP_SWITCH		0x06
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_LBMAP_SHIFT		0
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x04
#define QIXIS_LBMAP_NAND		0x09
#define QIXIS_LBMAP_SD			0x00
#define QIXIS_LBMAP_QSPI		0x0f
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_RCW_SRC_NAND		0x107
#define QIXIS_RCW_SRC_SD		0x40
#define QIXIS_RCW_SRC_QSPI		0x62
#define	QIXIS_RST_FORCE_MEM		0x01

#define CONFIG_SYS_CSPR3_EXT	(0x0)
#define CONFIG_SYS_CSPR3	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS_EARLY) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)
#define CONFIG_SYS_CSPR3_FINAL	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)

#define CONFIG_SYS_AMASK3	IFC_AMASK(64*1024)
#define CONFIG_SYS_CSOR3	CSOR_GPCM_ADM_SHIFT(12)
/* QIXIS Timing parameters for IFC CS3 */
#define CONFIG_SYS_CS3_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CONFIG_SYS_CS3_FTIM1		(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define CONFIG_SYS_CS3_FTIM2		(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0x3E))
#define CONFIG_SYS_CS3_FTIM3		0x0

#if defined(CONFIG_SPL)
#if defined(CONFIG_NAND_BOOT)
#define CONFIG_SYS_CSPR1_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NOR0_CSPR_EARLY
#define CONFIG_SYS_CSPR1_FINAL		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR2_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR2		CONFIG_SYS_NOR1_CSPR_EARLY
#define CONFIG_SYS_CSPR2_FINAL		CONFIG_SYS_NOR1_CSPR
#define CONFIG_SYS_AMASK2		CONFIG_SYS_NOR_AMASK_EARLY
#define CONFIG_SYS_AMASK2_FINAL		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR2		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS2_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS2_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS2_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS2_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NAND_FTIM3

#define CONFIG_ENV_OFFSET		(896 * 1024)
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SPL_PAD_TO		0x20000
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(256 * 1024)
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(640 * 1024)
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_ENV_OFFSET		0x300000
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			0x20000
#endif
#else
#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR0_CSPR_EARLY
#define CONFIG_SYS_CSPR0_FINAL		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR1_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NOR1_CSPR_EARLY
#define CONFIG_SYS_CSPR1_FINAL		CONFIG_SYS_NOR1_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NOR_AMASK_EARLY
#define CONFIG_SYS_AMASK1_FINAL		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR2_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR2		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK2		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR2		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS2_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS2_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS2_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS2_FTIM3		CONFIG_SYS_NAND_FTIM3

#if !defined(CONFIG_QSPI_BOOT) && !defined(CONFIG_TFABOOT)
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x300000)
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_SIZE			0x2000
#endif
#endif

/* Debug Server firmware */
#define CONFIG_SYS_DEBUG_SERVER_FW_IN_NOR
#define CONFIG_SYS_DEBUG_SERVER_FW_ADDR	0x580D00000ULL

#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

/*
 * I2C
 */
#define I2C_MUX_PCA_ADDR		0x77
#define I2C_MUX_PCA_ADDR_PRI		0x77 /* Primary Mux*/

/* I2C bus multiplexer */
#define I2C_MUX_CH_DEFAULT      0x8

/* SPI */
#if defined(CONFIG_FSL_QSPI) || defined(CONFIG_FSL_DSPI)
#ifdef CONFIG_FSL_DSPI
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_EON
#endif

#ifdef CONFIG_FSL_QSPI
#define CONFIG_SPI_FLASH_SPANSION
#define FSL_QSPI_FLASH_SIZE		(1 << 26) /* 64MB */
#define FSL_QSPI_FLASH_NUM		4
#endif
/*
 * Verify QSPI when boot from NAND, QIXIS brdcfg9 need configure.
 * If boot from on-board NAND, ISO1 = 1, ISO2 = 0, IBOOT = 0
 * If boot from IFCCard NAND, ISO1 = 0, ISO2 = 0, IBOOT = 1
 */
#define FSL_QIXIS_BRDCFG9_QSPI		0x1

#endif

/*
 * MMC
 */
#ifdef CONFIG_MMC
#define CONFIG_ESDHC_DETECT_QUIRK ((readb(QIXIS_BASE + QIXIS_STAT_PRES1) & \
	QIXIS_SDID_MASK) != QIXIS_ESDHC_NO_ADAPTER)
#endif

/*
 * RTC configuration
 */
#define RTC
#define CONFIG_RTC_DS3231               1
#define CONFIG_SYS_I2C_RTC_ADDR         0x68

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

#define CONFIG_FSL_MEMAC

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

/*  MMC  */
#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

/* Initial environment variables */
#undef CONFIG_EXTRA_ENV_SETTINGS
#ifdef CONFIG_SECURE_BOOT
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"mcmemsize=0x40000000\0"		\
	"mcinitcmd=esbc_validate 0x580700000;"  \
	"esbc_validate 0x580740000;"            \
	"fsl_mc start mc 0x580a00000"           \
	" 0x580e00000 \0"
#else
#ifdef CONFIG_TFABOOT
#define SD_MC_INIT_CMD				\
	"mmcinfo;mmc read 0x80a00000 0x5000 0x1200;"  \
	"mmc read 0x80e00000 0x7000 0x800;" \
	"fsl_mc start mc 0x80a00000 0x80e00000\0"
#define IFC_MC_INIT_CMD				\
	"fsl_mc start mc 0x580a00000" \
	" 0x580e00000 \0"
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"    \
	"loadaddr=0x80100000\0"                 \
	"loadaddr_sd=0x90100000\0"                 \
	"kernel_addr=0x581000000\0"		          \
	"kernel_addr_sd=0x8000\0"                \
	"ramdisk_addr=0x800000\0"               \
	"ramdisk_size=0x2000000\0"              \
	"fdt_high=0xa0000000\0"                 \
	"initrd_high=0xffffffffffffffff\0"      \
	"kernel_start=0x581000000\0"            \
	"kernel_start_sd=0x8000\0"              \
	"kernel_load=0xa0000000\0"              \
	"kernel_size=0x2800000\0"               \
	"kernel_size_sd=0x14000\0"               \
	"load_addr=0xa0000000\0"		            \
	"kernelheader_addr=0x580800000\0"	\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernelheader_size=0x40000\0"		\
	"BOARD=ls2088aqds\0" \
	"mcmemsize=0x70000000 \0" \
	IFC_MC_INIT_CMD				\
	"nor_bootcmd=echo Trying load from nor..;"		\
		"cp.b $kernel_addr $load_addr "			\
		"$kernel_size ; env exists secureboot && "	\
		"cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		"bootm $load_addr#$BOARD\0"	\
	"sd_bootcmd=echo Trying load from SD ..;" \
	"mmcinfo; mmc read $load_addr "		\
	"$kernel_addr_sd $kernel_size_sd && "	\
	"bootm $load_addr#$BOARD\0"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"    \
	"loadaddr=0x90100000\0"                 \
	"kernel_addr=0x800\0"                \
	"ramdisk_addr=0x800000\0"               \
	"ramdisk_size=0x2000000\0"              \
	"fdt_high=0xa0000000\0"                 \
	"initrd_high=0xffffffffffffffff\0"      \
	"kernel_start=0x8000\0"              \
	"kernel_load=0xa0000000\0"              \
	"kernel_size=0x14000\0"               \
	"mcinitcmd=mmcinfo;mmc read 0x80000000 0x5000 0x800;"  \
	"mmc read 0x80100000 0x7000 0x800;" \
	"fsl_mc start mc 0x80000000 0x80100000\0"       \
	"mcmemsize=0x70000000 \0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"mcmemsize=0x40000000\0"		\
	"mcinitcmd=fsl_mc start mc 0x580a00000" \
	" 0x580e00000 \0"
#endif /* CONFIG_TFABOOT */
#endif /* CONFIG_SECURE_BOOT */

#ifdef CONFIG_TFABOOT
#define SD_BOOTCOMMAND						\
			"env exists mcinitcmd && env exists secureboot "\
			"&& mmcinfo && mmc read $load_addr 0x3c00 0x800 " \
			"&& esbc_validate $load_addr; "			\
			"env exists mcinitcmd && run mcinitcmd "	\
			"&& mmc read 0x80d00000 0x6800 0x800 "		\
			"&& fsl_mc lazyapply dpl 0x80d00000; "		\
			"run sd_bootcmd; "		\
			"env exists secureboot && esbc_halt;"

#define IFC_NOR_BOOTCOMMAND						\
			"env exists mcinitcmd && env exists secureboot "\
			"&& esbc_validate 0x580780000; env exists mcinitcmd "\
			"&& fsl_mc lazyapply dpl 0x580d00000;"		\
			"run nor_bootcmd; "		\
			"env exists secureboot && esbc_halt;"
#endif

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
#define CONFIG_FSL_MEMAC
#define CONFIG_PHYLIB_10G
#define CONFIG_PHY_VITESSE
#define CONFIG_PHY_REALTEK
#define CONFIG_PHY_TERANETICS
#define SGMII_CARD_PORT1_PHY_ADDR 0x1C
#define SGMII_CARD_PORT2_PHY_ADDR 0x1d
#define SGMII_CARD_PORT3_PHY_ADDR 0x1E
#define SGMII_CARD_PORT4_PHY_ADDR 0x1F

#define XQSGMII_CARD_PHY1_PORT0_ADDR 0x0
#define XQSGMII_CARD_PHY1_PORT1_ADDR 0x1
#define XQSGMII_CARD_PHY1_PORT2_ADDR 0x2
#define XQSGMII_CARD_PHY1_PORT3_ADDR 0x3
#define XQSGMII_CARD_PHY2_PORT0_ADDR 0x4
#define XQSGMII_CARD_PHY2_PORT1_ADDR 0x5
#define XQSGMII_CARD_PHY2_PORT2_ADDR 0x6
#define XQSGMII_CARD_PHY2_PORT3_ADDR 0x7
#define XQSGMII_CARD_PHY3_PORT0_ADDR 0x8
#define XQSGMII_CARD_PHY3_PORT1_ADDR 0x9
#define XQSGMII_CARD_PHY3_PORT2_ADDR 0xa
#define XQSGMII_CARD_PHY3_PORT3_ADDR 0xb
#define XQSGMII_CARD_PHY4_PORT0_ADDR 0xc
#define XQSGMII_CARD_PHY4_PORT1_ADDR 0xd
#define XQSGMII_CARD_PHY4_PORT2_ADDR 0xe
#define XQSGMII_CARD_PHY4_PORT3_ADDR 0xf

#define CONFIG_ETHPRIME		"DPMAC1@xgmii"

#endif

#include <asm/fsl_secure_boot.h>

#endif /* __LS2_QDS_H */
