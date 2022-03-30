/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007,2009 Wind River Systems <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Copyright 2004, 2007 Freescale Semiconductor.
 */

/*
 * sbc8548 board configuration file
 * Please refer to doc/README.sbc8548 for more info.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Top level Makefile configuration choices
 */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCI1
#endif

#ifdef CONFIG_66
#define CONFIG_SYS_CLK_DIV 1
#endif

#ifdef CONFIG_33
#define CONFIG_SYS_CLK_DIV 2
#endif

#ifdef CONFIG_PCIE
#define CONFIG_PCIE1
#endif

/*
 * High Level Configuration Options
 */

/*
 * If you want to boot from the SODIMM flash, instead of the soldered
 * on flash, set this, and change JP12, SW2:8 accordingly.
 */
#undef CONFIG_SYS_ALT_BOOT

#undef CONFIG_RIO

#ifdef CONFIG_PCI
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT    1	/* enable 64-bit PCI resources */
#endif
#ifdef CONFIG_PCIE1
#endif

#define CONFIG_ENV_OVERWRITE

#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

/*
 * Below assumes that CCB:SYSCLK remains unchanged at 6:1 via SW2:[1-4]
 */
#ifndef CONFIG_SYS_CLK_DIV
#define CONFIG_SYS_CLK_DIV	1	/* 2, if 33MHz PCI card installed */
#endif
#define CONFIG_SYS_CLK_FREQ	(66000000 / CONFIG_SYS_CLK_DIV)

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#undef	CONFIG_SYS_DRAM_TEST			/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000

#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
/*
 * A hardware errata caused the LBC SDRAM SPD and the DDR2 SPD
 * to collide, meaning you couldn't reliably read either. So
 * physically remove the LBC PC100 SDRAM module from the board
 * before enabling the two SPD options below, or check that you
 * have the hardware fix on your board via "i2c probe" and looking
 * for a device at 0x53.
 */
#undef CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#undef CONFIG_DDR_SPD

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

/*
 * The hardware fix for the I2C address collision puts the DDR
 * SPD at 0x53, but if we are running on an older board w/o the
 * fix, it will still be at 0x51.  We check 0x53 1st.
 */
#define SPD_EEPROM_ADDRESS	0x51	/* CTLR 0 DIMM 0 */
#define ALT_SPD_EEPROM_ADDRESS	0x53	/* CTLR 0 DIMM 0 */

/*
 * Make sure required options are set
 */
#ifndef CONFIG_SPD_EEPROM
	#define CONFIG_SYS_SDRAM_SIZE	256		/* DDR is 256MB */
	#define CONFIG_SYS_DDR_CONTROL	0xc300c000
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * FLASH on the Local Bus
 * Two banks, one 8MB the other 64MB, using the CFI driver.
 * JP12+SW2.8 are used to swap CS0 and CS6, defaults are to have
 * CS0 the 8MB boot flash, and CS6 the 64MB flash.
 *
 *	Default:
 *	ec00_0000	efff_ffff	64MB SODIMM
 *	ff80_0000	ffff_ffff	8MB soldered flash
 *
 *	Alternate:
 *	ef80_0000	efff_ffff	8MB soldered flash
 *	fc00_0000	ffff_ffff	64MB SODIMM
 *
 * BR0_8M:
 *    Base address 0 = 0xff80_0000 = BR0[0:16] = 1111 1111 1000 0000 0
 *    Port Size = 8 bits = BRx[19:20] = 01
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * BR0_64M:
 *    Base address 0 = 0xfc00_0000 = BR0[0:16] = 1111 1100 0000 0000 0
 *    Port Size = 32 bits = BRx[19:20] = 11
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0000 1000 0000 0001 = ff800801    BR0_8M
 * 1111 1100 0000 0000 0001 1000 0000 0001 = fc001801    BR0_64M
 */
#define CONFIG_SYS_BR0_8M	0xff800801
#define CONFIG_SYS_BR0_64M	0xfc001801

/*
 * BR6_8M:
 *    Base address 6 = 0xef80_0000 = BR6[0:16] = 1110 1111 1000 0000 0
 *    Port Size = 8 bits = BRx[19:20] = 01
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1

 * BR6_64M:
 *    Base address 6 = 0xec00_0000 = BR6[0:16] = 1110 1100 0000 0000 0
 *    Port Size = 32 bits = BRx[19:20] = 11
 *
 * 0    4    8    12   16   20   24   28
 * 1110 1111 1000 0000 0000 1000 0000 0001 = ef800801    BR6_8M
 * 1110 1100 0000 0000 0001 1000 0000 0001 = ec001801    BR6_64M
 */
#define CONFIG_SYS_BR6_8M	0xef800801
#define CONFIG_SYS_BR6_64M	0xec001801

/*
 * OR0_8M:
 *    Addr Mask = 8M = OR1[0:16] = 1111 1111 1000 0000 0
 *    XAM = OR0[17:18] = 11
 *    CSNT = OR0[20] = 1
 *    ACS = half cycle delay = OR0[21:22] = 11
 *    SCY = 6 = OR0[24:27] = 0110
 *    TRLX = use relaxed timing = OR0[29] = 1
 *    EAD = use external address latch delay = OR0[31] = 1
 *
 * OR0_64M:
 *    Addr Mask = 64M = OR1[0:16] = 1111 1100 0000 0000 0
 *
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0110 1110 0110 0101 = ff806e65    OR0_8M
 * 1111 1100 0000 0000 0110 1110 0110 0101 = fc006e65    OR0_64M
 */
#define CONFIG_SYS_OR0_8M	0xff806e65
#define CONFIG_SYS_OR0_64M	0xfc006e65

/*
 * OR6_8M:
 *    Addr Mask = 8M = OR6[0:16] = 1111 1111 1000 0000 0
 *    XAM = OR6[17:18] = 11
 *    CSNT = OR6[20] = 1
 *    ACS = half cycle delay = OR6[21:22] = 11
 *    SCY = 6 = OR6[24:27] = 0110
 *    TRLX = use relaxed timing = OR6[29] = 1
 *    EAD = use external address latch delay = OR6[31] = 1
 *
 * OR6_64M:
 *    Addr Mask = 64M = OR6[0:16] = 1111 1100 0000 0000 0
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0110 1110 0110 0101 = ff806e65    OR6_8M
 * 1111 1100 0000 0000 0110 1110 0110 0101 = fc006e65    OR6_64M
 */
#define CONFIG_SYS_OR6_8M	0xff806e65
#define CONFIG_SYS_OR6_64M	0xfc006e65

#ifndef CONFIG_SYS_ALT_BOOT		/* JP12 in default position */
#define CONFIG_SYS_BOOT_BLOCK		0xff800000	/* start of 8MB Flash */
#define CONFIG_SYS_ALT_FLASH		0xec000000	/* 64MB "user" flash */

#define CONFIG_SYS_BR0_PRELIM		CONFIG_SYS_BR0_8M
#define CONFIG_SYS_OR0_PRELIM		CONFIG_SYS_OR0_8M

#define CONFIG_SYS_BR6_PRELIM		CONFIG_SYS_BR6_64M
#define CONFIG_SYS_OR6_PRELIM		CONFIG_SYS_OR6_64M
#else					/* JP12 in alternate position */
#define CONFIG_SYS_BOOT_BLOCK		0xfc000000	/* start 64MB Flash */
#define CONFIG_SYS_ALT_FLASH		0xef800000	/* 8MB soldered flash */

#define CONFIG_SYS_BR0_PRELIM		CONFIG_SYS_BR0_64M
#define CONFIG_SYS_OR0_PRELIM		CONFIG_SYS_OR0_64M

#define CONFIG_SYS_BR6_PRELIM		CONFIG_SYS_BR6_8M
#define CONFIG_SYS_OR6_PRELIM		CONFIG_SYS_OR6_8M
#endif

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_BOOT_BLOCK
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, \
					 CONFIG_SYS_ALT_FLASH}
#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#define CONFIG_SYS_FLASH_EMPTY_INFO

/* CS5 = Local bus peripherals controlled by the EPLD */

#define CONFIG_SYS_BR5_PRELIM		0xf8000801
#define CONFIG_SYS_OR5_PRELIM		0xff006e65
#define CONFIG_SYS_EPLD_BASE		0xf8000000
#define CONFIG_SYS_LED_DISP_BASE	0xf8000000
#define CONFIG_SYS_USER_SWITCHES_BASE	0xf8100000
#define CONFIG_SYS_BD_REV		0xf8300000
#define CONFIG_SYS_EEPROM_BASE		0xf8b00000

/*
 * SDRAM on the Local Bus (CS3 and CS4)
 * Note that most boards have a hardware errata where both the
 * LBC SDRAM and the DDR2 SDRAM decode at 0x51, making it impossible
 * to use CONFIG_DDR_SPD unless you physically remove the LBC DIMM.
 * A hardware workaround is also available, see README.sbc8548 file.
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	128		/* LBC SDRAM is 128MB */

/*
 * Base Register 3 and Option Register 3 configure the 1st 1/2 SDRAM.
 * The SDRAM base address, CONFIG_SYS_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR3, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f0001861
 *
 */

#define CONFIG_SYS_BR3_PRELIM		0xf0001861

/*
 * The SDRAM size in MB, of 1/2 CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR3, need:
 *    64MB mask for AM, OR3[0:7] = 1111 1100
 *		   XAM, OR3[17:18] = 11
 *    10 columns OR3[19-21] = 011
 *    12 rows   OR3[23-25] = 011
 *    EAD set for extra time OR[31] = 0
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1100 1100 0000 = fc006cc0
 */

#define CONFIG_SYS_OR3_PRELIM		0xfc006cc0

/*
 * Base Register 4 and Option Register 4 configure the 2nd 1/2 SDRAM.
 * The base address, (SDRAM_BASE + 1/2*SIZE), is 0xf4000000.
 *
 * For BR4, need:
 *    Base address of 0xf4000000 = BR[0:16] = 1111 0100 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f4001861
 *
 */

#define CONFIG_SYS_BR4_PRELIM		0xf4001861

/*
 * The SDRAM size in MB, of 1/2 CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR4, need:
 *    64MB mask for AM, OR3[0:7] = 1111 1100
 *		   XAM, OR3[17:18] = 11
 *    10 columns OR3[19-21] = 011
 *    12 rows   OR3[23-25] = 011
 *    EAD set for extra time OR[31] = 0
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1100 1100 0000 = fc006cc0
 */

#define CONFIG_SYS_OR4_PRELIM		0xfc006cc0

#define CONFIG_SYS_LBC_LCRR		0x00000002    /* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR		0x00000000    /* LB config reg */
#define CONFIG_SYS_LBC_LSRT		0x20000000  /* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x00000000  /* LB refresh timer prescal*/

/*
 * Common settings for all Local Bus SDRAM commands.
 */
#define CONFIG_SYS_LBC_LSDMR_COMMON	( LSDMR_RFCR16		\
				| LSDMR_BSMA1516	\
				| LSDMR_PRETOACT3	\
				| LSDMR_ACTTORW3	\
				| LSDMR_BUFCMD		\
				| LSDMR_BL8		\
				| LSDMR_WRC2		\
				| LSDMR_CL3		\
				)

#define CONFIG_SYS_LBC_LSDMR_PCHALL	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_PCHALL)
#define CONFIG_SYS_LBC_LSDMR_ARFRSH	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_ARFRSH)
#define CONFIG_SYS_LBC_LSDMR_MRW	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_MRW)
#define CONFIG_SYS_LBC_LSDMR_RFEN	\
	 (CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_RFEN)

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_INIT_L2_ADDR	0xf8f80000	/* relocate boot L2SRAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * For soldered on flash, (128kB/sector) we use 2 sectors for u-boot and
 * one for env+bootpg (CONFIG_SYS_TEXT_BASE=0xfffa_0000, 384kB total).  For SODIMM
 * flash (512kB/sector) we use 1 sector for u-boot, and one for env+bootpg
 * (CONFIG_SYS_TEXT_BASE=0xfff0_0000, 1MB total).  This dynamically sets the right
 * thing for MONITOR_LEN in both cases.
 */
#define CONFIG_SYS_MONITOR_LEN		(~CONFIG_SYS_TEXT_BASE + 1)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024) /* Reserved for malloc */

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(400000000 / CONFIG_SYS_CLK_DIV)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCI_VIRT		0x80000000	/* 1G PCI TLB */
#define CONFIG_SYS_PCI_PHYS		0x80000000	/* 1G PCI TLB */

#define CONFIG_SYS_PCI1_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_VIRT		0xe2000000
#define CONFIG_SYS_PCI1_IO_BUS		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xe2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00800000	/* 8M */

#ifdef CONFIG_PCIE1
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe2800000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe2800000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000	/* 8M */
#endif

#ifdef CONFIG_RIO
/*
 * RapidIO MMU
 */
#define CONFIG_SYS_RIO_MEM_BASE	0xC0000000
#define CONFIG_SYS_RIO_MEM_SIZE	0x20000000	/* 512M */
#endif

#if defined(CONFIG_PCI)
#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#undef CONFIG_MPC85XX_FEC

#define TSEC1_PHY_ADDR		0x19
#define TSEC2_PHY_ADDR		0x1a

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-3] */
#define CONFIG_ETHPRIME		"eTSEC0"
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CONFIG_ENV_SIZE		0x2000
#if CONFIG_SYS_TEXT_BASE == 0xfff00000	/* Boot from 64MB SODIMM */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x80000)
#define CONFIG_ENV_SECT_SIZE	0x80000	/* 512K(one sector) for env */
#elif CONFIG_SYS_TEXT_BASE == 0xfffa0000	/* Boot from 8MB soldered flash */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x40000)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
#else
#warning undefined environment size/location.
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_IPADDR	 192.168.0.55

#define CONFIG_HOSTNAME	 "sbc8548"
#define CONFIG_ROOTPATH	 "/opt/eldk/ppc_85xx"
#define CONFIG_BOOTFILE	 "/uImage"
#define CONFIG_UBOOTPATH /u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 192.168.0.2
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_NETMASK	 255.255.255.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define	CONFIG_EXTRA_ENV_SETTINGS				\
"netdev=eth0\0"						\
"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"				\
"tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " __stringify(CONFIG_SYS_TEXT_BASE) " +$filesize; " \
	"erase " __stringify(CONFIG_SYS_TEXT_BASE) " +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE) " $filesize; " \
	"protect on " __stringify(CONFIG_SYS_TEXT_BASE) " +$filesize; "	\
	"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE) " $filesize\0" \
"consoledev=ttyS0\0"				\
"ramdiskaddr=2000000\0"			\
"ramdiskfile=uRamdisk\0"			\
"fdtaddr=1e00000\0"				\
"fdtfile=sbc8548.dtb\0"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND	CONFIG_RAMBOOTCOMMAND

#endif	/* __CONFIG_H */
