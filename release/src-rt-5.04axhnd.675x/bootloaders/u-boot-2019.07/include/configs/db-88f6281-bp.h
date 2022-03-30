/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _CONFIG_DB_88F6281_BP_H
#define _CONFIG_DB_88F6281_BP_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131	1	/* CPU Core subversion */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_SYS_TCLK		166666667
#define CONFIG_SYS_KWD_CONFIG	$(CONFIG_BOARDDIR)/kwbimage.cfg
#define CONFIG_BUILD_TARGET	"u-boot.kwb"

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

#define CONFIG_KIRKWOOD_EGIGA_INIT	/* Enable GbePort0/1 for kernel */
#define CONFIG_KIRKWOOD_PCIE_INIT	/* Enable PCIE Port0 */
#define CONFIG_KIRKWOOD_RGMII_PAD_1V8	/* Set RGMII Pad voltage to 1.8V */
#define CONFIG_KIRKWOOD_GPIO	1

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#define CONFIG_SYS_NS16550_COM1		KW_UART0_BASE

#define CONFIG_SYS_MAX_NAND_DEVICE     1
/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_CONS_INDEX	1	/* Console on UART0 */

/*
 *  Environment variables configurations
 */
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		20000000	/* 20Mhz */
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SECT_SIZE		0x40000		/* 256K */
#define CONFIG_ENV_SIZE			0x01000
#define CONFIG_ENV_OFFSET		0xC0000

/*
 * U-Boot bootcode configuration
 */

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for monitor */
#define CONFIG_SYS_MALLOC_LEN		  (4 << 20)	/* Reserve 4.0 MB for malloc */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Mem map for Linux*/

/* size in bytes reserved for initial data */

#include <asm/arch/config.h>
/* There is no PHY directly connected so don't ask it for link status */
#undef CONFIG_SYS_FAULT_ECHO_LINK_DOWN

/*
 * Other required minimal configurations
 */
#define CONFIG_ARCH_CPU_INIT	/* call arch_cpu_init() */
#define CONFIG_SYS_MEMTEST_START 0x00400000	/* 4M */
#define CONFIG_SYS_MEMTEST_END	0x007fffff	/* (_8M - 1) */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */

/*
 * SDIO/MMC Card Configuration
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MVEBU_MMC
#define CONFIG_SYS_MMC_BASE KW_SDIO_BASE
#endif /* CONFIG_CMD_MMC */

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#endif /*CONFIG_MVSATA_IDE*/

#define CONFIG_SYS_LOAD_ADDR  0x1000000      /* default location for tftp and bootm */

#endif /* _CONFIG_DB_88F6281_BP_H */
