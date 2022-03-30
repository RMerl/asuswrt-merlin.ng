/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Stefan Roese <sr@denx.de>
 *
 * Configuration settings for the ProjectionDesign / Barco
 * Titanium board.
 *
 * Based on mx6qsabrelite.h which is:
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CONFIG_MX6Q

/* Provide the MACH_TYPE value that the vendor kernel requires. */
#define CONFIG_MACH_TYPE		3769

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(2 * 1024 * 1024)

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	1

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_FEC_MXC_PHYADDR		4

/* USB Configs */
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (500 << 20))

#define CONFIG_HOSTNAME			"titanium"
#define CONFIG_UBI_PART			ubi
#define CONFIG_UBIFS_VOLUME		rootfs0

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel=" CONFIG_HOSTNAME "/uImage\0"		\
	"kernel_fs=/boot/uImage\0"					\
	"kernel_addr=11000000\0"					\
	"dtb=" CONFIG_HOSTNAME "/"				\
		CONFIG_HOSTNAME ".dtb\0"			\
	"dtb_fs=/boot/" CONFIG_HOSTNAME ".dtb\0"		\
	"dtb_addr=12800000\0"						\
	"script=boot.scr\0" \
	"uimage=uImage\0" \
	"console=ttymxc0\0" \
	"baudrate=115200\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"uimage=uImage\0" \
	"loadbootscript=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr}" \
		" ${script}\0" \
	"bootscript=echo Running bootscript from mmc ...; source\0" \
	"loaduimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${uimage}\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot} rootwait rw\0" \
	"bootmmc=run mmcargs; fatload mmc ${mmcdev}:${mmcpart} ${loadaddr}" \
		" ${uimage}; bootm\0" \
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcon=setenv bootargs ${bootargs} console=ttymxc0,"		\
		"${baudrate}\0"						\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"rootpath=/opt/eldk-5.3/armv7a/rootfs-minimal-mtdutils\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ubifs=" CONFIG_HOSTNAME "/ubifs.img\0"		\
	"part=" __stringify(CONFIG_UBI_PART) "\0"			\
	"boot_vol=0\0"							\
	"vol=" __stringify(CONFIG_UBIFS_VOLUME) "\0"			\
	"load_ubifs=tftp ${kernel_addr} ${ubifs}\0"			\
	"update_ubifs=ubi part ${part};ubi write ${kernel_addr} ${vol}"	\
		" ${filesize}\0"					\
	"upd_ubifs=run load_ubifs update_ubifs\0"			\
	"init_ubi=nand erase.part ubi;ubi part ${part};"		\
		"ubi create ${vol} c800000\0"				\
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"					\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"				\
	"nand_ubifs=run ubifs_mount ubifs_load ubifsargs addip"		\
		" addcon addmtd;"					\
		"bootm ${kernel_addr} - ${dtb_addr}\0"			\
	"ubifsargs=set bootargs ubi.mtd=ubi "				\
		"root=ubi:rootfs${boot_vol} rootfstype=ubifs\0"		\
	"ubifs_mount=ubi part ubi;ubifsmount ubi:rootfs${boot_vol}\0"	\
	"ubifs_load=ubifsload ${kernel_addr} ${kernel_fs};"		\
		"ubifsload ${dtb_addr} ${dtb_fs};\0"			\
	"nand_ubifs=run ubifs_mount ubifs_load ubifsargs addip addcon "	\
		"addmtd;bootm ${kernel_addr} - ${dtb_addr}\0"		\
	"load_kernel=tftp ${kernel_addr} ${kernel}\0"			\
	"load_dtb=tftp ${dtb_addr} ${dtb}\0"				\
	"net_nfs=run load_dtb load_kernel; "				\
		"run nfsargs addip addcon addmtd;"			\
		"bootm ${kernel_addr} - ${dtb_addr}\0"			\
	"delenv=env default -a -f; saveenv; reset\0"

#define CONFIG_BOOTCOMMAND		"run nand_ubifs"

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			(512 << 20)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Enable NAND support */
#ifdef CONFIG_CMD_NAND

/* NAND stuff */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */

/* Environment in NAND */
#define CONFIG_ENV_OFFSET		(16 << 20)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + (512 << 10))
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

#else /* CONFIG_CMD_NAND */

/* Environment in MMC */
#define CONFIG_ENV_SIZE			(8 << 10)
#define CONFIG_ENV_OFFSET		(6 * 64 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0

#endif /* CONFIG_CMD_NAND */

/* UBI/UBIFS config options */

#endif			       /* __CONFIG_H */
