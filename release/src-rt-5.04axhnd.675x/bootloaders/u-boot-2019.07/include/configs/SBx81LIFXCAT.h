/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Allied Telesis <www.alliedtelesis.co.nz>
 */

#ifndef _CONFIG_SBX81LIFXCAT_H
#define _CONFIG_SBX81LIFXCAT_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131	1	/* CPU Core subversion */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_SYS_KWD_CONFIG	$(CONFIG_BOARDDIR)/kwbimage.cfg

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

#define CONFIG_MD5	/* get_random_hex on krikwood needs MD5 support */
#define CONFIG_KIRKWOOD_EGIGA_INIT	/* Enable GbePort0/1 for kernel */
#define CONFIG_KIRKWOOD_PCIE_INIT	/* Enable PCIE Port0 */
#define CONFIG_KIRKWOOD_RGMII_PAD_1V8	/* Set RGMII Pad voltage to 1.8V */
#define CONFIG_KIRKWOOD_GPIO	1

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#define CONFIG_SYS_NS16550_COM1		KW_UART0_BASE

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_CONS_INDEX	1	/*Console on UART0 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs */
#define CONFIG_INITRD_TAG	1	/* enable INITRD tag */
#define CONFIG_SETUP_MEMORY_TAGS 1	/* enable memory tag */

#define MTDPARTS_DEFAULT "mtdparts=spi0.0:768K(boot)ro,256K(boot-env),14M(user),1M(errlog)"
#define MTDPARTS_MTDOOPS "errlog"
#define CONFIG_DOS_PARTITION

/*
 *  Environment variables configurations
 */
#define CONFIG_ENV_SECT_SIZE		0x40000		/* 256K */
#define CONFIG_ENV_SIZE			0x02000
#define CONFIG_ENV_OFFSET		0xc0000		/* env starts here - 768K */

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
#define CONFIG_SYS_MEMTEST_END	0x007fffff	/*(_8M -1) */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_NETCONSOLE	/* include NetConsole support */
#define CONFIG_NET_MULTI	/* specify more that one ports available */
#define CONFIG_MVGBE	/* Enable kirkwood Gbe Controller Driver */
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable a single port */
#define CONFIG_PHY_BASE_ADR	0x01
#define CONFIG_ENV_OVERWRITE	/* ethaddr can be reprogrammed */
#endif /* CONFIG_CMD_NET */

#define CONFIG_SYS_LOAD_ADDR  0x1000000      /* default location for tftp and bootm */

#endif /* _CONFIG_SBX81LIFXCAT_H */
