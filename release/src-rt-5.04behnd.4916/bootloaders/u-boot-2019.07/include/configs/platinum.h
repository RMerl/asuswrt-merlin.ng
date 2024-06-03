/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Barco (www.barco.com)
 */

#ifndef __PLATINUM_CONFIG_H__
#define __PLATINUM_CONFIG_H__

/* SPL */

/* Location in NAND to read U-Boot from */
#define CONFIG_SYS_NAND_U_BOOT_OFFS     (14 * 1024 * 1024)

#include "imx6_spl.h"                  /* common IMX6 SPL configuration */
#include "mx6_common.h"

/*
 * Hardware configuration
 */

/* UART config */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE			UART1_BASE

/* I2C config */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED			100000

/* MMC config */
#define CONFIG_SYS_FSL_ESDHC_ADDR		0
#define CONFIG_SYS_FSL_USDHC_NUM		1

/* Ethernet config */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE				ENET_BASE_ADDR

/* USB config */
#define CONFIG_MXC_USB_PORT			1
#define CONFIG_MXC_USB_PORTSC			(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS			0

/* Memory config */
#define PHYS_SDRAM				MMDC0_ARB_BASE_ADDR
#ifndef PHYS_SDRAM_SIZE
#define PHYS_SDRAM_SIZE				(1024 << 20)
#endif

#define CONFIG_SYS_SDRAM_BASE			PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR		IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE		IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET		(CONFIG_SYS_INIT_RAM_SIZE - \
						 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR			(CONFIG_SYS_INIT_RAM_ADDR + \
						 CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MALLOC_LEN			(16 * 1024 * 1024)

#ifdef CONFIG_CMD_NAND

/* NAND config */
#ifndef CONFIG_SYS_NAND_MAX_CHIPS
#define CONFIG_SYS_NAND_MAX_CHIPS		2
#endif
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA config, needed for GPMI/MXS NAND support */

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

/*
 * U-Boot configuration
 */

/* Board startup config */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			(CONFIG_SYS_MEMTEST_START + \
						 PHYS_SDRAM_SIZE - (12 << 20))

#define CONFIG_BOOTCOMMAND			"run bootubi_scr"

/* Miscellaneous configurable options */
#define CONFIG_PREBOOT

/* MTD/UBI/UBIFS config */

/*
 * Environment configuration
 */

#if (CONFIG_SYS_NAND_MAX_CHIPS == 1)
#define CONFIG_COMMON_ENV_UBI						\
	"setubipartition=env set ubipartition ubi\0"			\
	"setubirfs=env set ubirfs $ubipartition:rootfs$boot_vol\0"
#elif (CONFIG_SYS_NAND_MAX_CHIPS == 2)
#define CONFIG_COMMON_ENV_UBI						\
	"setubipartition=env set ubipartition ubi$boot_vol\0"		\
	"setubirfs=env set ubirfs ubi0:rootfs\0"
#endif

#define CONFIG_COMMON_ENV_MISC						\
	"user=user\0"							\
	"project="CONFIG_PLATINUM_PROJECT"\0"				\
	"uimage=uImage\0"						\
	"dtb="CONFIG_PLATINUM_CPU"-platinum-"CONFIG_PLATINUM_PROJECT".dtb\0" \
	"serverip=serverip\0"						\
	"memaddrlinux=0x10800000\0"					\
	"memaddrsrc=0x11000000\0"					\
	"memaddrdtb=0x12000000\0"					\
	"console=ttymxc0\0"						\
	"baudrate=115200\0"						\
	"boot_scr=boot.uboot\0"						\
	"boot_vol=0\0"							\
	"mtdids="CONFIG_MTDIDS_DEFAULT"\0"					\
	"mtdparts="CONFIG_MTDPARTS_DEFAULT"\0"					\
	"mmcfs=ext2\0"							\
	"mmcrootpart=1\0"						\
	\
	"setnfspath=env set nfspath /home/nfs/$user/$project/root\0"	\
	"settftpfilelinux=env set tftpfilelinux $user/$project/$uimage\0" \
	"settftpfiledtb=env set tftpfiledtb $user/$project/$dtb\0"	\
	"setubifilelinux=env set ubifilelinux boot/$uimage\0"		\
	"setubipfiledtb=env set ubifiledtb boot/$dtb\0"			\
	"setmmcrootdev=env set mmcrootdev /dev/mmcblk0p$mmcrootpart\0"	\
	"setmmcfilelinux=env set mmcfilelinux /boot/$uimage\0"		\
	"setmmcfiledtb=env set mmcfiledtb /boot/$dtb\0"			\
	\
	"loadtftpkernel=dhcp $memaddrlinux $tftpfilelinux\0"		\
	"loadtftpdtb=dhcp $memaddrdtb $tftpfiledtb\0"			\
	"loadubikernel=ubifsload $memaddrlinux $ubifilelinux\0"		\
	"loadubidtb=ubifsload $memaddrdtb $ubifiledtb\0"		\
	"loadmmckernel=${mmcfs}load mmc 0:$mmcrootpart $memaddrlinux "	\
		"$mmcfilelinux\0"					\
	"loadmmcdtb=${mmcfs}load mmc 0:$mmcrootpart $memaddrdtb "	\
		"$mmcfiledtb\0"						\
	\
	"ubipart=ubi part $ubipartition\0"				\
	"ubimount=ubifsmount $ubirfs\0"					\
	\
	"setbootargscommon=env set bootargs $bootargs "			\
		"console=$console,$baudrate enable_wait_mode=off\0"	\
	"setbootargsmtd=env set bootargs $bootargs $mtdparts\0"		\
	"setbootargsdhcp=env set bootargs $bootargs ip=dhcp\0"		\
	"setbootargsubirfs=env set bootargs $bootargs "			\
		"ubi.mtd=$ubipartition root=$ubirfs rootfstype=ubifs\0" \
	"setbootargsnfsrfs=env set bootargs $bootargs root=/dev/nfs "	\
		"nfsroot=$serverip:$nfspath,v3,tcp\0"			\
	"setbootargsmmcrfs=env set bootargs $bootargs "			\
		"root=$mmcrootdev rootwait rw\0"			\
	\
	"bootnet=run settftpfilelinux settftpfiledtb setnfspath "	\
		"setbootargscommon setbootargsmtd setbootargsdhcp "	\
			"setbootargsnfsrfs;"				\
			"run loadtftpkernel loadtftpdtb;"		\
			"bootm $memaddrlinux - $memaddrdtb\0"		\
	"bootnet_ubirfs=run settftpfilelinux settftpfiledtb;"		\
			"run setubipartition setubirfs;"		\
			"run setbootargscommon setbootargsmtd "		\
				"setbootargsubirfs;"			\
			"run loadtftpkernel loadtftpdtb;"		\
			"bootm $memaddrlinux - $memaddrdtb\0"		\
	"bootubi=run setubipartition setubirfs setubifilelinux "	\
				"setubipfiledtb;"			\
			"run setbootargscommon setbootargsmtd "		\
				"setbootargsubirfs;"			\
			"run ubipart ubimount loadubikernel loadubidtb;" \
			"bootm $memaddrlinux - $memaddrdtb\0"		\
	"bootubi_scr=run setubipartition setubirfs;"			\
			"run ubipart ubimount;"				\
			"if ubifsload ${memaddrsrc} boot/${boot_scr}; "	\
			"then source ${memaddrsrc}; else run bootubi; fi\0" \
	"bootmmc=run setmmcrootdev setmmcfilelinux setmmcfiledtb "	\
			"setbootargscommon setbootargsmmcrfs;"		\
			"run loadmmckernel loadmmcdtb;"			\
			"bootm $memaddrlinux - $memaddrdtb\0"		\
	\
	"bootcmd="CONFIG_BOOTCOMMAND"\0"

#define CONFIG_COMMON_ENV_SETTINGS			CONFIG_COMMON_ENV_MISC \
							CONFIG_COMMON_ENV_UBI
#endif /* __PLATINUM_CONFIG_H__ */
