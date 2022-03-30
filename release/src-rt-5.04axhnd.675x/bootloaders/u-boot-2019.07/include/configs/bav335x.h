/*
 * bav335x.h
 *
 * Copyright (c) 2012-2014 Birdland Audio - http://birdland.com/oem
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_BAV335X_H
#define __CONFIG_BAV335X_H

#include <configs/ti_am335x_common.h>

#ifndef CONFIG_SPL_BUILD
# define CONFIG_TIMESTAMP
#endif

#define CONFIG_SYS_BOOTM_LEN		(16 << 20)

#define CONFIG_MACH_TYPE		MACH_TYPE_AM335XEVM

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Always 128 KiB env size */
#define CONFIG_ENV_SIZE			(128 << 10)

#ifdef CONFIG_NAND
#define NANDARGS \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=9,2048\0" \
	"nandrootfstype=ubifs rootwait=1\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${fdtaddr} u-boot-spl-os; " \
		"nand read ${loadaddr} kernel; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#else
#define NANDARGS ""
#endif

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
DEFAULT_LINUX_BOOT_ENV \
"boot_fdt=try\0" \
"bootpart=0:2\0" \
"bootdir=\0" \
"fdtdir=/dtbs\0" \
"bootfile=zImage\0" \
"fdtfile=undefined\0" \
"console=ttyO0,115200n8\0" \
"loadaddr=0x82000000\0" \
"fdtaddr=0x88000000\0" \
"rdaddr=0x88080000\0" \
"initrd_high=0xffffffff\0" \
"fdt_high=0xffffffff\0" \
"partitions=" \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=rootfs,start=2MiB,size=-,uuid=${uuid_gpt_rootfs}\0" \
"optargs=\0" \
"cmdline=\0" \
"mmcdev=0\0" \
"mmcpart=1\0" \
"mmcroot=/dev/mmcblk0p2 ro\0" \
"mmcrootfstype=ext4 rootwait fixrtc\0" \
"rootpath=/export/rootfs\0" \
"nfsopts=nolock\0" \
"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off\0" \
"ramroot=/dev/ram0 rw\0" \
"ramrootfstype=ext2\0" \
"mmcargs=setenv bootargs console=${console} ${optargs} " \
	"root=${mmcroot} rootfstype=${mmcrootfstype} ${cmdline}\0" \
"server_ip=192.168.1.100\0" \
"gw_ip=192.168.1.1\0" \
"netmask=255.255.255.0\0" \
"hostname=\0" \
"device=eth0\0" \
"autoconf=off\0" \
"root_dir=/home/userid/targetNFS\0" \
"nfs_options=,vers=3\0" \
"nfsrootfstype=ext4 rootwait fixrtc\0" \
"nfsargs=setenv bootargs console=${console} ${optargs} " \
	"root=/dev/nfs rw rootfstype=${nfsrootfstype} " \
	"nfsroot=${nfsroot} ip=${ip} ${cmdline}\0" \
"netargs=setenv bootargs console=${console} " \
	"${optargs} root=/dev/nfs " \
	"nfsroot=${serverip}:${rootpath},${nfsopts} rw ip=dhcp\0" \
"bootenv=uEnv.txt\0" \
"script=boot.scr\0" \
"scriptfile=${script}\0" \
"loadbootscript=load mmc ${bootpart} ${loadaddr} ${scriptfile};\0" \
"bootscript=echo Running bootscript from mmc${bootpart} ...; " \
	"source ${loadaddr}\0" \
	"loadbootenv=load mmc ${bootpart} ${loadaddr} ${bootenv}\0" \
"importbootenv=echo Importing environment from mmc ...; " \
	"env import -t -r $loadaddr $filesize\0" \
"ramargs=setenv bootargs console=${console} " \
	"${optargs} root=${ramroot} rootfstype=${ramrootfstype}\0" \
"loadramdisk=load mmc ${mmcdev} ${rdaddr} ramdisk.gz\0" \
"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadrd=load mmc ${bootpart} ${rdaddr} " \
	"${bootdir}/${rdfile}; setenv rdsize ${filesize}\0" \
"loadfdt=echo loading ${fdtdir}/${fdtfile} ...; " \
	"load mmc ${bootpart} ${fdtaddr} ${fdtdir}/${fdtfile}\0" \
"mmcboot=mmc dev ${mmcdev}; " \
	"if mmc rescan; then " \
		"gpio set 54;" \
		"setenv bootpart ${mmcdev}:1; " \
		"if test -e mmc ${bootpart} /etc/fstab; then " \
			"setenv mmcpart 1;" \
		"fi; " \
		"echo Checking for: /uEnv.txt ...;" \
		"if test -e mmc ${bootpart} /uEnv.txt; then " \
			"if run loadbootenv; then " \
				"gpio set 55;" \
				"echo Loaded environment from ${bootenv};" \
				"run importbootenv;" \
			"fi;" \
			"echo Checking if uenvcmd is set ...;" \
			"if test -n ${uenvcmd}; then " \
				"gpio set 56; " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
			"echo Checking if client_ip is set ...;" \
			"if test -n ${client_ip}; then " \
				"if test -n ${dtb}; then " \
					"setenv fdtfile ${dtb};" \
					"echo using ${fdtfile} ...;" \
				"fi;" \
				"gpio set 56; " \
				"if test -n ${uname_r}; then " \
					"echo Running nfsboot_uname_r ...;" \
					"run nfsboot_uname_r;" \
				"fi;" \
				"echo Running nfsboot ...;" \
				"run nfsboot;" \
			"fi;" \
		"fi; " \
		"echo Checking for: /${script} ...;" \
		"if test -e mmc ${bootpart} /${script}; then " \
			"gpio set 55;" \
			"setenv scriptfile ${script};" \
			"run loadbootscript;" \
			"echo Loaded script from ${scriptfile};" \
			"gpio set 56; " \
			"run bootscript;" \
		"fi; " \
		"echo Checking for: /boot/${script} ...;" \
		"if test -e mmc ${bootpart} /boot/${script}; then " \
			"gpio set 55;" \
			"setenv scriptfile /boot/${script};" \
			"run loadbootscript;" \
			"echo Loaded script from ${scriptfile};" \
			"gpio set 56; " \
			"run bootscript;" \
		"fi; " \
		"echo Checking for: /boot/uEnv.txt ...;" \
		"for i in 1 2 3 4 5 6 7 ; do " \
			"setenv mmcpart ${i};" \
			"setenv bootpart ${mmcdev}:${mmcpart};" \
			"if test -e mmc ${bootpart} /boot/uEnv.txt; then " \
				"gpio set 55;" \
				"load mmc ${bootpart} ${loadaddr} " \
						"/boot/uEnv.txt;" \
				"env import -t ${loadaddr} ${filesize};" \
				"echo Loaded environment from /boot/uEnv.txt;" \
				"if test -n ${dtb}; then " \
					"setenv fdtfile ${dtb};" \
					"echo Using: dtb=${fdtfile} ...;" \
				"fi;" \
				"echo Checking if uname_r is set in " \
						"/boot/uEnv.txt...;" \
				"if test -n ${uname_r}; then " \
					"gpio set 56; " \
					"echo Running uname_boot ...;" \
					"setenv mmcroot /dev/mmcblk${mmcdev}" \
							"p${mmcpart} ro;" \
					"run uname_boot;" \
				"fi;" \
			"fi;" \
		"done;" \
	"fi;\0" \
"netboot=echo Booting from network ...; " \
	"setenv autoload no; " \
	"dhcp; " \
	"tftp ${loadaddr} ${bootfile}; " \
	"tftp ${fdtaddr} ${fdtfile}; " \
	"run netargs; " \
	"bootz ${loadaddr} - ${fdtaddr}\0" \
"nfsboot=echo Booting from ${server_ip} ...; " \
	"setenv nfsroot ${server_ip}:${root_dir}${nfs_options}; " \
	"setenv ip ${client_ip}:${server_ip}:${gw_ip}:${netmask}:${hostname}" \
	":${device}:${autoconf}; " \
	"setenv autoload no; " \
	"setenv serverip ${server_ip}; " \
	"setenv ipaddr ${client_ip}; " \
	"tftp ${loadaddr} ${bootfile}; " \
	"tftp ${fdtaddr} dtbs/${fdtfile}; " \
	"run nfsargs; " \
	"bootz ${loadaddr} - ${fdtaddr}\0" \
"nfsboot_uname_r=echo Booting from ${server_ip} ...; " \
	"setenv nfsroot ${server_ip}:${root_dir}${nfs_options}; " \
	"setenv ip ${client_ip}:${server_ip}:${gw_ip}:${netmask}:${hostname}" \
			":${device}:${autoconf}; " \
	"setenv autoload no; " \
	"setenv serverip ${server_ip}; " \
	"setenv ipaddr ${client_ip}; " \
	"tftp ${loadaddr} vmlinuz-${uname_r}; " \
	"tftp ${fdtaddr} dtbs/${uname_r}/${fdtfile}; " \
	"run nfsargs; " \
	"bootz ${loadaddr} - ${fdtaddr}\0" \
"ramboot=echo Booting from ramdisk ...; " \
	"run ramargs; " \
	"bootz ${loadaddr} ${rdaddr} ${fdtaddr}\0" \
"findfdt="\
	"if test $board_rev = B; then " \
		"setenv fdtfile birdland_bav335b.dtb; " \
		"setenv fdtbase am335x-boneblack; fi; " \
	"if test $board_rev = A; then " \
		"setenv fdtfile birdland_bav335a.dtb; " \
		"setenv fdtbase am335x-boneblack; fi; " \
	"if test $fdtfile = undefined; then " \
		"echo WARNING: Could not determine device tree to use; fi; \0" \
"uname_boot="\
	"setenv bootdir /boot; " \
	"setenv bootfile vmlinuz-${uname_r}; " \
	"if test -e mmc ${bootpart} ${bootdir}/${bootfile}; then " \
		"echo loading ${bootdir}/${bootfile} ...; "\
		"run loadimage;" \
		"setenv fdtdir /boot/dtbs/${uname_r}; " \
		"if test -e mmc ${bootpart} ${fdtdir}/${fdtfile}; then " \
			"run loadfdt;" \
		"else " \
			"setenv fdtdir /lib/firmware/${uname_r}/device-tree; " \
			"if test -e mmc ${bootpart} ${fdtdir}/" \
					"${fdtfile}; then " \
				"run loadfdt;" \
			"else " \
				"setenv fdtdir /boot/dtb-${uname_r}; " \
				"if test -e mmc ${bootpart} ${fdtdir}" \
						"/${fdtfile}; then " \
					"run loadfdt;" \
				"else " \
					"setenv fdtdir /boot/dtbs; " \
					"if test -e mmc ${bootpart} ${fdtdir}" \
							"/${fdtfile}; then " \
						"run loadfdt;" \
					"else " \
						"echo; echo unable to find " \
							"[${fdtfile}] " \
						"did you name it correctly?" \
						"echo booting fallback " \
							"[/boot/dtbs/" \
							"${uname_r}" \
							"/${fdtbase}.dtb]...;" \
						"setenv fdtdir /boot/dtbs/" \
							"${uname_r}; " \
						"setenv fdtfile " \
							"${fdtbase}.dtb; " \
						"run loadfdt;" \
					"fi;" \
				"fi;" \
			"fi;" \
		"fi;" \
	"fi; " \
	"setenv rdfile initrd.img-${uname_r}; " \
	"if test -e mmc ${bootpart} ${bootdir}/${rdfile}; then " \
		"echo loading ${bootdir}/${rdfile} ...; "\
		"run loadrd;" \
		"if test -n ${uuid}; then " \
			"setenv mmcroot UUID=${uuid} ro;" \
		"fi;" \
		"run mmcargs;" \
		"echo debug: [${bootargs}] ... ;" \
		"echo debug: [bootz ${loadaddr} ${rdaddr}:${rdsize} " \
					"${fdtaddr}] ... ;" \
		"bootz ${loadaddr} ${rdaddr}:${rdsize} ${fdtaddr}; " \
	"else " \
		"run mmcargs;" \
		"echo debug: [${bootargs}] ... ;" \
		"echo debug: [bootz ${loadaddr} - ${fdtaddr}] ... ;" \
		"bootz ${loadaddr} - ${fdtaddr}; " \
	"fi;" \
"fi;\0" \
	NANDARGS \
	DFUARGS
#endif

#define CONFIG_BOOTCOMMAND \
	"gpio set 53; " \
	"i2c mw 0x24 1 0x3e; " \
	"run findfdt; " \
	"setenv mmcdev 0; " \
	"setenv bootpart 0:1; " \
	"run mmcboot;" \
	"gpio clear 56; " \
	"gpio clear 55; " \
	"gpio clear 54; " \
	"setenv mmcdev 1; " \
	"setenv bootpart 1:1; " \
	"run mmcboot;"

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/* PMIC support */
#define CONFIG_POWER_TPS65217
#define CONFIG_POWER_TPS65910

/* SPL */
#ifndef CONFIG_NOR_BOOT
/* Bootcount using the RTC block */
#define CONFIG_SYS_BOOTCOUNT_BE

/* USB gadget RNDIS */
#endif

#ifdef CONFIG_NAND
/* NAND: device related configs */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
/* NAND: driver related configs */
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS	{ \
	2, 3, 4, 5, 6, 7, 8, 9, \
	10, 11, 12, 13, 14, 15, 16, 17, \
	18, 19, 20, 21, 22, 23, 24, 25, \
	26, 27, 28, 29, 30, 31, 32, 33, \
	34, 35, 36, 37, 38, 39, 40, 41, \
	42, 43, 44, 45, 46, 47, 48, 49, \
	50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x000c0000
#define CONFIG_ENV_OFFSET		0x001c0000
#define CONFIG_ENV_OFFSET_REDUND	0x001e0000
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE
/* NAND: SPL related configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x00200000 /* kernel offset */
#endif
#endif /* !CONFIG_NAND */

/*
 * For NOR boot, we must set this to the start of where NOR is mapped
 * in memory.
 */

/*
 * USB configuration.  We enable MUSB support, both for host and for
 * gadget.  We set USB0 as peripheral and USB1 as host, based on the
 * board schematic and physical port wired to each.  Then for host we
 * add mass storage support and for gadget we add both RNDIS ethernet
 * and DFU.
 */
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_PERIPHERAL
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_USB_ETHER)
/* disable host part of MUSB in SPL */
/* disable EFI partitions and partition UUID support */
#endif

/* USB Device Firmware Update support */
#ifndef CONFIG_SPL_BUILD
#define DFU_ALT_INFO_MMC \
	"dfu_alt_info_mmc=" \
	"boot part 0 1;" \
	"rootfs part 0 2;" \
	"MLO fat 0 1;" \
	"MLO.raw raw 0x100 0x100;" \
	"u-boot.img.raw raw 0x300 0x400;" \
	"spl-os-args.raw raw 0x80 0x80;" \
	"spl-os-image.raw raw 0x900 0x2000;" \
	"spl-os-args fat 0 1;" \
	"spl-os-image fat 0 1;" \
	"u-boot.img fat 0 1;" \
	"uEnv.txt fat 0 1\0"
#ifdef CONFIG_NAND
#define DFU_ALT_INFO_NAND \
	"dfu_alt_info_nand=" \
	"SPL part 0 1;" \
	"SPL.backup1 part 0 2;" \
	"SPL.backup2 part 0 3;" \
	"SPL.backup3 part 0 4;" \
	"u-boot part 0 5;" \
	"u-boot-spl-os part 0 6;" \
	"kernel part 0 8;" \
	"rootfs part 0 9\0"
#else
#define DFU_ALT_INFO_NAND ""
#endif
#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram 0x80200000 0xD80000;" \
	"fdt ram 0x80F80000 0x80000;" \
	"ramdisk ram 0x81000000 0x4000000\0"
#define DFUARGS \
	"dfu_alt_info_emmc=rawemmc raw 0 3751936\0" \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_NAND
#endif

/*
 * Default to using SPI for environment, etc.
 * 0x000000 - 0x020000 : SPL (128KiB)
 * 0x020000 - 0x0A0000 : U-Boot (512KiB)
 * 0x0A0000 - 0x0BFFFF : First copy of U-Boot Environment (128KiB)
 * 0x0C0000 - 0x0DFFFF : Second copy of U-Boot Environment (128KiB)
 * 0x0E0000 - 0x442000 : Linux Kernel
 * 0x442000 - 0x800000 : Userland
 */
#if defined(CONFIG_SPI_BOOT)
/* SPL related */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE		(4 << 10) /* 4 KB sectors */
#define CONFIG_ENV_OFFSET		(768 << 10) /* 768 KiB in */
#define CONFIG_ENV_OFFSET_REDUND	(896 << 10) /* 896 KiB in */
#elif defined(CONFIG_EMMC_BOOT)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x0
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#endif

/* SPI flash. */

/* Network. */
#define CONFIG_PHY_SMSC

/*
 * NOR Size = 16 MiB
 * Number of Sectors/Blocks = 128
 * Sector Size = 128 KiB
 * Word length = 16 bits
 * Default layout:
 * 0x000000 - 0x07FFFF : U-Boot (512 KiB)
 * 0x080000 - 0x09FFFF : First copy of U-Boot Environment (128 KiB)
 * 0x0A0000 - 0x0BFFFF : Second copy of U-Boot Environment (128 KiB)
 * 0x0C0000 - 0x4BFFFF : Linux Kernel (4 MiB)
 * 0x4C0000 - 0xFFFFFF : Userland (11 MiB + 256 KiB)
 */
#if defined(CONFIG_NOR)
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BASE		(0x08000000)
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
/* Reduce SPL size by removing unlikey targets */
#ifdef CONFIG_NOR_BOOT
#define CONFIG_ENV_SECT_SIZE		(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		(512 << 10)	/* 512 KiB */
#define CONFIG_ENV_OFFSET_REDUND	(768 << 10)	/* 768 KiB */
#endif
#endif  /* NOR support */

#endif	/* ! __CONFIG_AM335X_EVM_H */
