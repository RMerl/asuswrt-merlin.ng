/*
 * (C) Copyright 2013
 * Dirk Eibach, Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * based on P1022DS.h
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD
#endif

#ifdef CONFIG_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH
#endif

/* High Level Configuration Options */
#define CONFIG_CONTROLCENTERD

#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_L2_CACHE
#define CONFIG_BTB

#define CONFIG_SYS_CLK_FREQ	66666600
#define CONFIG_DDR_CLK_FREQ	66666600

#define CONFIG_SYS_RAMBOOT

#ifdef CONFIG_TRAILBLAZER

#define CONFIG_RESET_VECTOR_ADDRESS	0xf8fffffc
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)

/*
 * Config the L2 Cache
 */
#define CONFIG_SYS_INIT_L2_ADDR		0xf8fc0000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	0xff8fc0000ull
#else
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#endif
#define CONFIG_SYS_L2_SIZE		(256 << 10)
#define CONFIG_SYS_INIT_L2_END	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)

#else /* CONFIG_TRAILBLAZER */

#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#endif /* CONFIG_TRAILBLAZER */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

/*
 * Memory map
 *
 * 0x0000_0000	0x3fff_ffff	DDR			1G Cacheable
 * 0xc000_0000	0xdfff_ffff	PCI Express Mem		512M non-cacheable
 * 0xffc0_0000	0xffc2_ffff	PCI IO range		192K non-cacheable
 *
 * Localbus non-cacheable
 * 0xe000_0000	0xe00f_ffff	eLBC			1M non-cacheable
 * 0xf8fc0000	0xf8ff_ffff	L2 SRAM			256k Cacheable
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000 /* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000 /* used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#ifdef CONFIG_TRAILBLAZER
/* leave CCSRBAR at default, because u-boot expects it to be exactly there */
#define CONFIG_SYS_CCSRBAR		CONFIG_SYS_CCSRBAR_DEFAULT
#else
#define CONFIG_SYS_CCSRBAR		0xffe00000
#endif
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR
#define CONFIG_SYS_MPC85xx_GPIO3_ADDR	(CONFIG_SYS_CCSRBAR+0xf200)

/*
 * DDR Setup
 */

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE 1024
#define CONFIG_VERY_BIG_RAM

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

#define CONFIG_SYS_MEMTEST_START	0x00000000
#define CONFIG_SYS_MEMTEST_END		0x3fffffff

#ifdef CONFIG_TRAILBLAZER
#define CONFIG_SPD_EEPROM
#define SPD_EEPROM_ADDRESS 0x52
/*#define CONFIG_FSL_DDR_INTERACTIVE*/
#endif

/*
 * Local Bus Definitions
 */

#define CONFIG_SYS_ELBC_BASE		0xe0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_ELBC_BASE_PHYS	0xfe0000000ull
#else
#define CONFIG_SYS_ELBC_BASE_PHYS	CONFIG_SYS_ELBC_BASE
#endif

#define CONFIG_UART_BR_PRELIM  \
	(BR_PHYS_ADDR((CONFIG_SYS_ELBC_BASE_PHYS)) | BR_PS_8 | BR_V)
#define CONFIG_UART_OR_PRELIM	(OR_AM_32KB | 0xff7)

#define CONFIG_SYS_BR0_PRELIM	0 /* CS0 was originally intended for FPGA */
#define CONFIG_SYS_OR0_PRELIM	0 /* debugging, was never used */

#define CONFIG_SYS_BR1_PRELIM	CONFIG_UART_BR_PRELIM
#define CONFIG_SYS_OR1_PRELIM	CONFIG_UART_OR_PRELIM

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

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
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

#define CONFIG_PCA9698			/* NXP PCA9698 */

#define CONFIG_SYS_I2C_EEPROM_ADDR 0x52
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2

/*
 * MMC
 */
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR

#ifndef CONFIG_TRAILBLAZER

/*
 * Video
 */
#define CONFIG_FSL_DIU_FB
#define CONFIG_SYS_DIU_ADDR	(CONFIG_SYS_CCSRBAR + 0x10000)

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_PCIE1			/* PCIE controller 1 (slot 1) */
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */

#define CONFIG_SYS_PCIE1_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc20000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xfffc20000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc20000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/*
 * SATA
 */
#define CONFIG_LBA48

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

/*
 * Ethernet
 */

#define CONFIG_TSECV2

#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC2"

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

/*
 * USB
 */

#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

#endif /* CONFIG_TRAILBLAZER */

/*
 * Environment
 */
#if defined(CONFIG_TRAILBLAZER)
#define CONFIG_ENV_SIZE		0x2000		/* 8KB */
#elif defined(CONFIG_RAMBOOT_SPIFLASH)
#define CONFIG_ENV_SIZE		0x2000		/* 8KB */
#define CONFIG_ENV_OFFSET	0x100000	/* 1MB */
#define CONFIG_ENV_SECT_SIZE	0x10000
#elif defined(CONFIG_RAMBOOT_SDCARD)
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_SYS_MMC_ENV_DEV	0
#endif

/*
 * Command line configuration.
 */

#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

#ifndef CONFIG_TRAILBLAZER
/*
 * Board initialisation callbacks
 */
#endif /* CONFIG_TRAILBLAZER */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_HW_WATCHDOG
#define CONFIG_LOADS_ECHO
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Linux Memory map */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

/*
 * Environment Configuration
 */

#ifdef CONFIG_TRAILBLAZER
#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"mp_holdoff=1\0"

#else

#define CONFIG_HOSTNAME		"controlcenterd"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP */

#define CONFIG_LOADADDR		1000000

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"netdev=eth0\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"		\
	"ubootaddr=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"	\
	"tftpflash=tftpboot $loadaddr $uboot && "		\
		"protect off $ubootaddr +$filesize && "		\
		"erase $ubootaddr +$filesize && "		\
		"cp.b $loadaddr $ubootaddr $filesize && "	\
		"protect on $ubootaddr +$filesize && "		\
		"cmp.b $loadaddr $ubootaddr $filesize\0"	\
	"consoledev=ttyS1\0"					\
	"ramdiskaddr=2000000\0"					\
	"ramdiskfile=rootfs.ext2.gz.uboot\0"			\
	"fdtaddr=1e00000\0"					\
	"fdtfile=controlcenterd.dtb\0"				\
	"bdev=sda3\0"

/* these are used and NUL-terminated in env_default.h */
#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs $videobootargs;"	\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs $videobootargs;"	\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_RAMBOOTCOMMAND

#endif /* CONFIG_TRAILBLAZER */

#endif
