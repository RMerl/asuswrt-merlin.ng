/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 * Copyright 2004-2008 Freescale Semiconductor, Inc.
 */

/*
 * xpedite520x board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SYS_BOARD_NAME	"XPedite5200"
#define CONFIG_SYS_FORM_PMC_XMC	1

#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup */
#define CONFIG_PCI1		1	/* PCI controller 1 */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */

/*
 * DDR config
 */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define SPD_EEPROM_ADDRESS		0x54
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define CONFIG_SYS_CLK_FREQ	66666666

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_SYS_CCSRBAR		0xef000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/*
 * Diagnostics
 */
#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x20000000
#define CONFIG_POST			(CONFIG_SYS_POST_MEMORY | \
					 CONFIG_SYS_POST_I2C)
#define I2C_ADDR_LIST			{CONFIG_SYS_I2C_MAX1237_ADDR,	\
					 CONFIG_SYS_I2C_EEPROM_ADDR,	\
					 CONFIG_SYS_I2C_PCA953X_ADDR0,	\
					 CONFIG_SYS_I2C_PCA953X_ADDR1,	\
					 CONFIG_SYS_I2C_RTC_ADDR}

/*
 * Memory map
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 * 0x8000_0000	0xbfff_ffff	PCI1 Mem		1G non-cacheable
 * 0xe000_0000	0xe7ff_ffff	SRAM/SSRAM/L1 Cache	128M non-cacheable
 * 0xe800_0000	0xe87f_ffff	PCI1 IO			8M non-cacheable
 * 0xef00_0000	0xef0f_ffff	CCSR/IMMR		1M non-cacheable
 * 0xef80_0000	0xef8f_ffff	NAND Flash		1M non-cacheable
 * 0xf800_0000	0xfbff_ffff	NOR Flash 2		64M non-cacheable
 * 0xfc00_0000	0xffff_ffff	NOR Flash 1		64M non-cacheable
 */

#define CONFIG_SYS_LBC_LCRR	(LCRR_CLKDIV_8 | LCRR_EADC_3)

/*
 * NAND flash configuration
 */
#define CONFIG_SYS_NAND_BASE		0xef800000
#define CONFIG_SYS_NAND_BASE2		0xef840000 /* Unused at this time */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_ACTL
#define CONFIG_SYS_NAND_ACTL_CLE	(1 << 3)	/* ADDR3 is CLE */
#define CONFIG_SYS_NAND_ACTL_ALE	(1 << 4)	/* ADDR4 is ALE */
#define CONFIG_SYS_NAND_ACTL_NCE	(0)		/* NCE not controlled by ADDR */
#define CONFIG_SYS_NAND_ACTL_DELAY	25

/*
 * NOR flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xfc000000
#define CONFIG_SYS_FLASH_BASE2		0xf8000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE2}
#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024		/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000		/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */
#define CONFIG_SYS_FLASH_AUTOPROTECT_LIST	{ {0xfff40000, 0xc0000}, \
						  {0xfbf40000, 0xc0000} }
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

/*
 * Chip select configuration
 */
/* NOR Flash 0 on CS0 */
#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE	| \
				 BR_PS_16		| \
				 BR_V)
#define CONFIG_SYS_OR0_PRELIM	(OR_AM_64MB		| \
				 OR_GPCM_ACS_DIV4	| \
				 OR_GPCM_SCY_8)

/* NOR Flash 1 on CS1 */
#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_FLASH_BASE2	| \
				 BR_PS_16		| \
				 BR_V)
#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_OR0_PRELIM

/* NAND flash on CS2 */
#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_NAND_BASE	| \
				 BR_PS_8		| \
				 BR_V)

/* NAND flash on CS2 */
#define CONFIG_SYS_OR2_PRELIM	(OR_AM_256KB		| \
				 OR_GPCM_BCTLD		| \
				 OR_GPCM_CSNT		| \
				 OR_GPCM_ACS_DIV4	| \
				 OR_GPCM_SCY_4		| \
				 OR_GPCM_TRLX		| \
				 OR_GPCM_EHTR)

/* NAND flash on CS3 */
#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_NAND_BASE2	| \
				 BR_PS_8		| \
				 BR_V)
#define CONFIG_SYS_OR3_PRELIM	CONFIG_SYS_OR2_PRELIM

/*
 * Use L1 as initial stack
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe0000000
#define CONFIG_SYS_INIT_RAM_SIZE		0x4000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)	/* Reserve 512 KB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}
#define CONFIG_LOADS_ECHO		1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

/* I2C EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6	/* 64 byte pages */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* take up to 10 msec */

/* I2C RTC */
#define CONFIG_RTC_M41T11			1
#define CONFIG_SYS_I2C_RTC_ADDR			0x68
#define CONFIG_SYS_M41T11_BASE_YEAR		2000

/* GPIO */
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR0		0x18
#define CONFIG_SYS_I2C_PCA953X_ADDR1		0x19
#define CONFIG_SYS_I2C_PCA953X_ADDR		CONFIG_SYS_I2C_PCA953X_ADDR0

/* PCA957 @ 0x18 */
#define CONFIG_SYS_PCA953X_BRD_CFG0		0x01
#define CONFIG_SYS_PCA953X_BRD_CFG1		0x02
#define CONFIG_SYS_PCA953X_BRD_CFG2		0x04
#define CONFIG_SYS_PCA953X_XMC_ROOT0		0x08
#define CONFIG_SYS_PCA953X_FLASH_PASS_CS	0x10
#define CONFIG_SYS_PCA953X_NVM_WP		0x20
#define CONFIG_SYS_PCA953X_MONARCH		0x40
#define CONFIG_SYS_PCA953X_EREADY		0x80

/* PCA957 @ 0x19 */
#define CONFIG_SYS_PCA953X_P14_IO0		0x01
#define CONFIG_SYS_PCA953X_P14_IO1		0x02
#define CONFIG_SYS_PCA953X_P14_IO2		0x04
#define CONFIG_SYS_PCA953X_P14_IO3		0x08
#define CONFIG_SYS_PCA953X_P14_IO4		0x10
#define CONFIG_SYS_PCA953X_P14_IO5		0x20
#define CONFIG_SYS_PCA953X_P14_IO6		0x40
#define CONFIG_SYS_PCA953X_P14_IO7		0x80

/* 12-bit ADC used to measure CPU diode */
#define CONFIG_SYS_I2C_MAX1237_ADDR		0x34

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BUS
#define CONFIG_SYS_PCI1_MEM_SIZE	0x40000000	/* 1G */
#define CONFIG_SYS_PCI1_IO_BUS		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xe8000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00800000	/* 1M */

/*
 * Networking options
 */
#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC1_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define CONFIG_HAS_ETH0

#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC2_PHY_ADDR		2
#define TSEC2_PHYIDX		0
#define CONFIG_HAS_ETH1

#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"
#define TSEC3_FLAGS		TSEC_GIGABIT
#define TSEC3_PHY_ADDR		3
#define TSEC3_PHYIDX		0
#define CONFIG_HAS_ETH2

#define CONFIG_TSEC4	1
#define CONFIG_TSEC4_NAME	"eTSEC4"
#define TSEC4_FLAGS		TSEC_GIGABIT
#define TSEC4_PHY_ADDR		4
#define TSEC4_PHYIDX		0
#define CONFIG_HAS_ETH3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_LOADADDR		0x1000000	/* default location for tftp and bootm */
#define CONFIG_PREBOOT				/* enable preboot variable */
#define CONFIG_INTEGRITY			/* support booting INTEGRITY OS */
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(16 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(16 << 20)	/* Increase max gunzip size */

/*
 * Environment Configuration
 */
#define CONFIG_ENV_SECT_SIZE	0x20000		/* 128k (one sector) for env */
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - (256 * 1024))

/*
 * Flash memory map:
 * fff80000 - ffffffff     Pri U-Boot (512 KB)
 * fff40000 - fff7ffff     Pri U-Boot Environment (256 KB)
 * fff00000 - fff3ffff     Pri FDT (256KB)
 * fef00000 - ffefffff     Pri OS image (16MB)
 * fc000000 - feefffff     Pri OS Use/Filesystem (47MB)
 *
 * fbf80000 - fbffffff     Sec U-Boot (512 KB)
 * fbf40000 - fbf7ffff     Sec U-Boot Environment (256 KB)
 * fbf00000 - fbf3ffff     Sec FDT (256KB)
 * faf00000 - fbefffff     Sec OS image (16MB)
 * f8000000 - faefffff     Sec OS Use/Filesystem (47MB)
 */
#define CONFIG_UBOOT1_ENV_ADDR	__stringify(0xfff80000)
#define CONFIG_UBOOT2_ENV_ADDR	__stringify(0xfbf80000)
#define CONFIG_FDT1_ENV_ADDR	__stringify(0xfff00000)
#define CONFIG_FDT2_ENV_ADDR	__stringify(0xfbf00000)
#define CONFIG_OS1_ENV_ADDR	__stringify(0xfef00000)
#define CONFIG_OS2_ENV_ADDR	__stringify(0xfaf00000)

#define CONFIG_PROG_UBOOT1						\
	"$download_cmd $loadaddr $ubootfile; "				\
	"if test $? -eq 0; then "					\
		"protect off "CONFIG_UBOOT1_ENV_ADDR" +80000; "		\
		"erase "CONFIG_UBOOT1_ENV_ADDR" +80000; "		\
		"cp.w $loadaddr "CONFIG_UBOOT1_ENV_ADDR" 40000; "	\
		"protect on "CONFIG_UBOOT1_ENV_ADDR" +80000; "		\
		"cmp.b $loadaddr "CONFIG_UBOOT1_ENV_ADDR" 80000; "	\
		"if test $? -ne 0; then "				\
			"echo PROGRAM FAILED; "				\
		"else; "						\
			"echo PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_UBOOT2						\
	"$download_cmd $loadaddr $ubootfile; "				\
	"if test $? -eq 0; then "					\
		"protect off "CONFIG_UBOOT2_ENV_ADDR" +80000; "		\
		"erase "CONFIG_UBOOT2_ENV_ADDR" +80000; "		\
		"cp.w $loadaddr "CONFIG_UBOOT2_ENV_ADDR" 40000; "	\
		"protect on "CONFIG_UBOOT2_ENV_ADDR" +80000; "		\
		"cmp.b $loadaddr "CONFIG_UBOOT2_ENV_ADDR" 80000; "	\
		"if test $? -ne 0; then "				\
			"echo PROGRAM FAILED; "				\
		"else; "						\
			"echo PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_BOOT_OS_NET						\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"if test -n $fdtaddr; then "				\
			"$download_cmd $fdtaddr $fdtfile; "		\
			"if test $? -eq 0; then "			\
				"bootm $osaddr - $fdtaddr; "		\
			"else; "					\
				"echo FDT DOWNLOAD FAILED; "		\
			"fi; "						\
		"else; "						\
			"bootm $osaddr; "				\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_OS1							\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_OS1_ENV_ADDR" +$filesize; "		\
		"cp.b $osaddr "CONFIG_OS1_ENV_ADDR" $filesize; "	\
		"cmp.b $osaddr "CONFIG_OS1_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo OS PROGRAM FAILED; "			\
		"else; "						\
			"echo OS PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_OS2							\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_OS2_ENV_ADDR" +$filesize; "		\
		"cp.b $osaddr "CONFIG_OS2_ENV_ADDR" $filesize; "	\
		"cmp.b $osaddr "CONFIG_OS2_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo OS PROGRAM FAILED; "			\
		"else; "						\
			"echo OS PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_FDT1						\
	"$download_cmd $fdtaddr $fdtfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_FDT1_ENV_ADDR" +$filesize;"		\
		"cp.b $fdtaddr "CONFIG_FDT1_ENV_ADDR" $filesize; "	\
		"cmp.b $fdtaddr "CONFIG_FDT1_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo FDT PROGRAM FAILED; "			\
		"else; "						\
			"echo FDT PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo FDT DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_FDT2						\
	"$download_cmd $fdtaddr $fdtfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_FDT2_ENV_ADDR" +$filesize;"		\
		"cp.b $fdtaddr "CONFIG_FDT2_ENV_ADDR" $filesize; "	\
		"cmp.b $fdtaddr "CONFIG_FDT2_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo FDT PROGRAM FAILED; "			\
		"else; "						\
			"echo FDT PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo FDT DOWNLOAD FAILED; "				\
	"fi;"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"autoload=yes\0"						\
	"download_cmd=tftp\0"						\
	"console_args=console=ttyS0,115200\0"				\
	"root_args=root=/dev/nfs rw\0"					\
	"misc_args=ip=on\0"						\
	"set_bootargs=setenv bootargs ${console_args} ${root_args} ${misc_args}\0" \
	"bootfile=/home/user/file\0"					\
	"osfile=/home/user/board.uImage\0"				\
	"fdtfile=/home/user/board.dtb\0"				\
	"ubootfile=/home/user/u-boot.bin\0"				\
	"fdtaddr=0x1e00000\0"						\
	"osaddr=0x1000000\0"						\
	"loadaddr=0x1000000\0"						\
	"prog_uboot1="CONFIG_PROG_UBOOT1"\0"				\
	"prog_uboot2="CONFIG_PROG_UBOOT2"\0"				\
	"prog_os1="CONFIG_PROG_OS1"\0"					\
	"prog_os2="CONFIG_PROG_OS2"\0"					\
	"prog_fdt1="CONFIG_PROG_FDT1"\0"				\
	"prog_fdt2="CONFIG_PROG_FDT2"\0"				\
	"bootcmd_net=run set_bootargs; "CONFIG_BOOT_OS_NET"\0"		\
	"bootcmd_flash1=run set_bootargs; "				\
		"bootm "CONFIG_OS1_ENV_ADDR" - "CONFIG_FDT1_ENV_ADDR"\0"\
	"bootcmd_flash2=run set_bootargs; "				\
		"bootm "CONFIG_OS2_ENV_ADDR" - "CONFIG_FDT2_ENV_ADDR"\0"\
	"bootcmd=run bootcmd_flash1\0"
#endif	/* __CONFIG_H */
