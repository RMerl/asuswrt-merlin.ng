/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 */

#ifndef _CONFIG_LSXL_H
#define _CONFIG_LSXL_H

/*
 * Version number information
 */
#if defined(CONFIG_LSCHLV2)
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage-lschl.cfg
#define CONFIG_MACH_TYPE 3006
#define CONFIG_SYS_TCLK 166666667 /* 166 MHz */
#elif defined(CONFIG_LSXHL)
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage-lsxhl.cfg
#define CONFIG_MACH_TYPE 2663
/* CONFIG_SYS_TCLK is 200000000 by default */
#else
#error "unknown board"
#endif

/*
 * General configuration options
 */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
#define CONFIG_KW88F6281		/* SOC Name */

#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_KIRKWOOD_GPIO

/*
 * Commands configuration
 */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* loading initramfs images without uimage header */

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	8
#define CONFIG_ENV_SECT_SIZE		0x10000 /* 64K */
#endif

#define CONFIG_ENV_SIZE			0x10000 /* 64k */
#define CONFIG_ENV_OFFSET		0x70000 /* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_LOADADDR		0x00800000

#if defined(CONFIG_LSXHL)
#define CONFIG_FDTFILE "kirkwood-lsxhl.dtb"
#elif defined(CONFIG_LSCHLV2)
#define CONFIG_FDTFILE "kirkwood-lschlv2.dtb"
#else
#error "Unsupported board"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootsource=legacy\0"						\
	"hdpart=0:1\0"							\
	"kernel_addr=0x00800000\0"					\
	"ramdisk_addr=0x01000000\0"					\
	"fdt_addr=0x00ff0000\0"						\
	"bootcmd_legacy=sata init "					\
		"&& load sata ${hdpart} ${kernel_addr} /uImage.buffalo "\
		"&& load sata ${hdpart} ${ramdisk_addr} /initrd.buffalo "\
		"&& bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"bootcmd_net=bootp ${kernel_addr} vmlinuz "			\
		"&& tftpboot ${ramdisk_addr} initrd.img "		\
		"&& setenv ramdisk_len ${filesize} "			\
		"&& tftpboot ${fdt_addr} " CONFIG_FDTFILE " "		\
		"&& bootz ${kernel_addr} "				\
			"${ramdisk_addr}:${ramdisk_len} ${fdt_addr}\0"	\
	"bootcmd_hdd=sata init "					\
		"&& load sata ${hdpart} ${kernel_addr} /vmlinuz "	\
		"&& load sata ${hdpart} ${ramdisk_addr} /initrd.img "	\
		"&& setenv ramdisk_len ${filesize} "			\
		"&& load sata ${hdpart} ${fdt_addr} /dtb "		\
		"&& bootz ${kernel_addr} "				\
			"${ramdisk_addr}:${ramdisk_len} ${fdt_addr}\0"	\
	"bootcmd_usb=usb start "					\
		"&& load usb 0:1 ${kernel_addr} /vmlinuz "		\
		"&& load usb 0:1 ${ramdisk_addr} /initrd.img "		\
		"&& setenv ramdisk_len ${filesize} "			\
		"&& load usb 0:1 ${fdt_addr} " CONFIG_FDTFILE " "	\
		"&& bootz ${kernel_addr} "				\
			"${ramdisk_addr}:${ramdisk_len} ${fdt_addr}\0"	\
	"bootcmd_rescue=run config_nc_dhcp; run nc\0"			\
	"eraseenv=sf probe 0 "						\
		"&& sf erase " __stringify(CONFIG_ENV_OFFSET)		\
			" +" __stringify(CONFIG_ENV_SIZE) "\0"		\
	"config_nc_dhcp=setenv autoload_old ${autoload}; "		\
		"setenv autoload no "					\
		"&& bootp "						\
		"&& setenv ncip "					\
		"&& setenv autoload ${autoload_old}; "			\
		"setenv autoload_old\0"					\
	"standard_env=setenv ipaddr; setenv netmask; setenv serverip; "	\
		"setenv ncip; setenv gatewayip; setenv ethact; "	\
		"setenv bootfile; setenv dnsip; "			\
		"setenv bootsource legacy; run ser\0"			\
	"restore_env=run standard_env; saveenv; reset\0"		\
	"ser=setenv stdin serial; setenv stdout serial; "		\
		"setenv stderr serial\0"				\
	"nc=setenv stdin nc; setenv stdout nc; setenv stderr nc\0"	\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS		{0, 1} /* enable port 1 only */
#define CONFIG_PHY_BASE_ADR		7
#undef CONFIG_RESET_PHY_R
#endif /* CONFIG_CMD_NET */

#ifdef CONFIG_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE 1
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_LBA48
#endif

#endif /* _CONFIG_LSXL_H */
