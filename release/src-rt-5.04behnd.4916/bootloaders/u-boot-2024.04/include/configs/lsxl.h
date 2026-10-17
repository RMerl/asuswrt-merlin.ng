/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 */

#ifndef _CONFIG_LSXL_H
#define _CONFIG_LSXL_H

/*
 * General configuration options
 */

#include "mv-common.h"

/* loading initramfs images without uimage header */

/*
 *  Environment variables configurations
 */

/*
 * Default environment variables
 */
#define CFG_EXTRA_ENV_SETTINGS					\
	"bootsource=legacy\0"						\
	"hdpart=0:1\0"							\
	"kernel_addr_r=0x00800000\0"					\
	"ramdisk_addr_r=0x01000000\0"					\
	"fdt_addr_r=0x00ff0000\0"					\
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"				\
	"bootcmd_legacy=sata init "					\
		"&& load sata ${hdpart} ${kernel_addr_r} /uImage.buffalo "\
		"&& load sata ${hdpart} ${ramdisk_addr_r} /initrd.buffalo "\
		"&& bootm ${kernel_addr_r} ${ramdisk_addr_r}\0"		\
	"bootcmd_net=bootp ${kernel_addr_r} vmlinuz "			\
		"&& tftpboot ${fdt_addr_r} ${fdtfile} "			\
		"&& tftpboot ${ramdisk_addr_r} initrd.img "		\
		"&& bootz ${kernel_addr_r} "				\
			"${ramdisk_addr_r}:${filesize} ${fdt_addr_r}\0"	\
	"bootcmd_hdd=sata init "					\
		"&& load sata ${hdpart} ${kernel_addr_r} /vmlinuz "	\
		"&& load sata ${hdpart} ${fdt_addr_r} /dtb "		\
		"&& load sata ${hdpart} ${ramdisk_addr_r} /initrd.img "	\
		"&& bootz ${kernel_addr_r} "				\
			"${ramdisk_addr_r}:${filesize} ${fdt_addr_r}\0"	\
	"bootcmd_usb=usb start "					\
		"&& load usb 0:1 ${kernel_addr_r} /vmlinuz "		\
		"&& load usb 0:1 ${fdt_addr_r} ${fdtfile} "		\
		"&& load usb 0:1 ${ramdisk_addr_r} /initrd.img "	\
		"&& bootz ${kernel_addr_r} "				\
			"${ramdisk_addr_r}:${filesize} ${fdt_addr_r}\0"	\
	"bootcmd_rescue=run config_nc_dhcp; run nc\0"			\
	"config_nc_dhcp=setenv autoload_old ${autoload}; "		\
		"setenv autoload no "					\
		"&& bootp "						\
		"&& setenv ncip "					\
		"&& setenv autoload ${autoload_old}; "			\
		"setenv autoload_old\0"					\
	"nc=setenv stdin nc; setenv stdout nc; setenv stderr nc\0"	\

#endif /* _CONFIG_LSXL_H */
