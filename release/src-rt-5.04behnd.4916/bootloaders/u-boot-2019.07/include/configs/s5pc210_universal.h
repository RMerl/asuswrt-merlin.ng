/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * Configuation settings for the SAMSUNG Universal (EXYNOS4210) board.
 */

#ifndef __CONFIG_UNIVERSAL_H
#define __CONFIG_UNIVERSAL_H

#include <configs/exynos4-common.h>

#define CONFIG_TIZEN			/* TIZEN lib */

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_L2CACHE_OFF		1

/* Universal has 2 banks of DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM_1			CONFIG_SYS_SDRAM_BASE

#define SDRAM_BANK_SIZE			(256 << 20)	/* 256 MB */

/* select serial console configuration */

/* Console configuration */

#define CONFIG_BOOTCOMMAND		"run mmcboot"
#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR \
					- GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MEM_TOP_HIDE	(1 << 20)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5000000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4800000)

/* Actual modem binary size is 16MiB. Add 2MiB for bad block handling */

#define NORMAL_MTDPARTS_DEFAULT CONFIG_MTDPARTS_DEFAULT

#define MBRPARTS_DEFAULT	"20M(permanent)"\
				",20M(boot)"\
				",1G(system)"\
				",100M(swap)"\
				",-(UMS)\0"

#define CONFIG_ENV_UBI_MTD	" ubi.mtd=${ubiblock} ubi.mtd=4 ubi.mtd=7"
#define CONFIG_BOOTBLOCK	"10"
#define CONFIG_UBIBLOCK		"9"

#define CONFIG_SYS_MMC_ENV_DEV		CONFIG_MMC_DEFAULT_DEV
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		((32 - 4) << 10) /* 32KiB - 4KiB */

#define CONFIG_ENV_UBIFS_OPTION	" rootflags=bulk_read,no_chk_data_crc "
#define CONFIG_ENV_FLASHBOOT	CONFIG_ENV_UBI_MTD CONFIG_ENV_UBIFS_OPTION \
				"${mtdparts}"

#define CONFIG_ENV_COMMON_BOOT	"${console} ${meminfo}"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"updateb=" \
		"onenand erase 0x0 0x100000;" \
		"onenand write 0x42008000 0x0 0x100000\0" \
	"updatek=" \
		"onenand erase 0xc00000 0x500000;" \
		"onenand write 0x41008000 0xc00000 0x500000\0" \
	"bootk=" \
		"run loaduimage; bootm 0x40007FC0\0" \
	"updatebackup=" \
		"mmc dev 0 2; mmc write 0 0x42100000 0 0x200;" \
		"mmc dev 0 0\0" \
	"updatebootb=" \
		"mmc read 0 0x42100000 0x80 0x200; run updatebackup\0" \
	"lpj=lpj=3981312\0" \
	"ubifsboot=" \
		"set bootargs root=ubi0!rootfs rootfstype=ubifs ${lpj} " \
		CONFIG_ENV_FLASHBOOT " ${opts} ${lcdinfo} " \
		CONFIG_ENV_COMMON_BOOT "; run bootk\0" \
	"tftpboot=" \
		"set bootargs root=ubi0!rootfs rootfstype=ubifs " \
		CONFIG_ENV_FLASHBOOT " ${opts} ${lcdinfo} " \
		CONFIG_ENV_COMMON_BOOT \
		"; tftp 0x40007FC0 uImage; bootm 0x40007FC0\0" \
	"nfsboot=" \
		"set bootargs root=/dev/nfs rw " \
		"nfsroot=${nfsroot},nolock,tcp " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
		"${netmask}:generic:usb0:off " CONFIG_ENV_COMMON_BOOT \
		"; run bootk\0" \
	"ramfsboot=" \
		"set bootargs root=/dev/ram0 rw rootfstype=ext2 " \
		"${console} ${meminfo} " \
		"initrd=0x43000000,8M ramdisk=8192\0" \
	"mmcboot=" \
		"set bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart} " \
		"${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo}; " \
		"run loaduimage; bootm 0x40007FC0\0" \
	"bootchart=set opts init=/sbin/bootchartd; run bootcmd\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"mmcoops=mmc read 0 0x40000000 0x40 8; md 0x40000000 0x400\0" \
	"verify=n\0" \
	"rootfstype=ext4\0" \
	"console=" CONFIG_DEFAULT_CONSOLE \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT \
	"mbrparts=" MBRPARTS_DEFAULT \
	"meminfo=crashkernel=32M@0x50000000\0" \
	"nfsroot=/nfsroot/arm\0" \
	"bootblock=" CONFIG_BOOTBLOCK "\0" \
	"ubiblock=" CONFIG_UBIBLOCK" \0" \
	"ubi=enabled\0" \
	"loaduimage=fatload mmc ${mmcdev}:${mmcbootpart} 0x40007FC0 uImage\0" \
	"mmcdev=0\0" \
	"mmcbootpart=2\0" \
	"mmcrootpart=3\0" \
	"opts=always_resume=1"

#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SAMSUNG_ONENAND
#define CONFIG_SYS_ONENAND_BASE		0x0C000000

#define CONFIG_USB_GADGET_DWC2_OTG_PHY

/*
 * SPI Settings
 */
#define CONFIG_SOFT_SPI

#ifndef	__ASSEMBLY__
void universal_spi_scl(int bit);
void universal_spi_sda(int bit);
int universal_spi_read(void);
#endif

/* Common misc for Samsung */
#define CONFIG_MISC_COMMON

/* Download menu - Samsung common */
#define CONFIG_LCD_MENU

/* Download menu - definitions for check keys */
#ifndef __ASSEMBLY__

#define KEY_PWR_PMIC_NAME		"MAX8998_PMIC"
#define KEY_PWR_STATUS_REG		MAX8998_REG_STATUS1
#define KEY_PWR_STATUS_MASK		(1 << 7)
#define KEY_PWR_INTERRUPT_REG		MAX8998_REG_IRQ1
#define KEY_PWR_INTERRUPT_MASK		(1 << 7)

#define KEY_VOL_UP_GPIO			EXYNOS4_GPIO_X20
#define KEY_VOL_DOWN_GPIO		EXYNOS4_GPIO_X21
#endif /* __ASSEMBLY__ */

/* LCD console */
#define LCD_BPP			LCD_COLOR16

/*
 * LCD Settings
 */
#define CONFIG_BMP_16BPP
#define CONFIG_LD9040
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE ((500 * 160 * 4) + 54)

#endif	/* __CONFIG_H */
