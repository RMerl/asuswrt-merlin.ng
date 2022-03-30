/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Armadeus Systems
 *
 * Configuration settings for the OPOS6ULDev board
 */

#ifndef __OPOS6ULDEV_CONFIG_H
#define __OPOS6ULDEV_CONFIG_H

#include "mx6_common.h"

#ifdef CONFIG_SPL
#include "imx6_spl.h"

#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_REGULATOR
#endif
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 << 20)

/* Miscellaneous configurable options */
#define CONFIG_STANDALONE_LOAD_ADDR	CONFIG_SYS_LOAD_ADDR

/* Physical Memory Map */
#define CONFIG_SYS_SDRAM_BASE		MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* USB */
#ifdef CONFIG_USB_EHCI_MX6
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#endif

/* Ethernet */
#ifdef CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x1
#define CONFIG_FEC_XCV_TYPE             RMII
#define CONFIG_ETHPRIME			"FEC"
#endif

/* LCD */
#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_SPLASH_SOURCE
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_MXS
#define MXS_LCDIF_BASE MX6UL_LCDIF1_BASE_ADDR
#endif
#endif

/* Environment is stored in the eMMC boot partition */
#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_SYS_MMC_ENV_PART         1
#define CONFIG_ENV_SIZE                 (10 * 1024)
#define CONFIG_ENV_OFFSET               (1024 * 1024) /* 1 MB */
#define CONFIG_ENV_OFFSET_REDUND        (1536 * 1024) /* 512KB from CONFIG_ENV_OFFSET */

#define CONFIG_ENV_VERSION	100
#define CONFIG_BOARD_NAME	opos6ul
#define ACFG_CONSOLE_DEV        ttymxc0
#define CONFIG_SYS_AUTOLOAD     "no"
#define CONFIG_ROOTPATH         "/tftpboot/" __stringify(CONFIG_BOARD_NAME) "-root"
#define CONFIG_PREBOOT          "run check_env"
#define CONFIG_BOOTCOMMAND	"run emmcboot"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"env_version="          __stringify(CONFIG_ENV_VERSION)         "\0"                    \
	"consoledev="           __stringify(ACFG_CONSOLE_DEV)           "\0"                    \
	"board_name="           __stringify(CONFIG_BOARD_NAME)          "\0"                    \
	"fdt_addr=0x88000000\0"                                                                 \
	"fdt_high=0xffffffff\0"                                                                 \
	"fdt_name="           __stringify(CONFIG_BOARD_NAME)          "dev\0"                   \
	"initrd_high=0xffffffff\0"                                                              \
	"ip_dyn=yes\0"                                                                          \
	"stdin=serial\0"                                                                        \
	"stdout=serial\0"                                                                       \
	"stderr=serial\0"                                                                       \
	"mmcdev=0\0"                                                                            \
	"mmcpart=2\0"                                                                           \
	"mmcroot=/dev/mmcblk0p2 ro\0"                                                           \
	"mmcrootfstype=ext4 rootwait\0"                                                         \
	"kernelimg="           __stringify(CONFIG_BOARD_NAME)          "-linux.bin\0"           \
	"videomode=video=ctfb:x:800,y:480,depth:18,pclk:33033,le:96,ri:96,up:20,lo:21,hs:64,vs:4,sync:0,vmode:0\0" \
	"check_env=if test -n ${flash_env_version}; "                                           \
		"then env default env_version; "                                                \
		"else env set flash_env_version ${env_version}; env save; "                     \
		"fi; "                                                                          \
		"if itest ${flash_env_version} != ${env_version}; then "                        \
			"echo \"*** Warning - Environment version"                              \
			" change suggests: run flash_reset_env; reset\"; "                      \
			"env default flash_reset_env; "                                         \
		"else exit; fi; \0"                                                             \
	"flash_reset_env=env default -f -a && saveenv && "                                      \
		"echo Environment variables erased!\0"                                          \
	"download_uboot_spl=tftpboot ${loadaddr} ${board_name}-u-boot.spl\0"                    \
	"flash_uboot_spl="                                                                      \
		"if mmc dev 0 1; then "                                                         \
			"setexpr sz ${filesize} / 0x200; "                                      \
			"setexpr sz ${sz} + 1; "                                                \
			"if mmc write ${loadaddr} 0x2 ${sz}; then "                             \
				"echo Flashing of U-boot SPL succeed; "                         \
			"else echo Flashing of U-boot SPL failed; "                             \
			"fi; "                                                                  \
		"fi;\0"                                                                         \
	"download_uboot_img=tftpboot ${loadaddr} ${board_name}-u-boot.img\0"                    \
	"flash_uboot_img="                                                                      \
		"if mmc dev 0 1; then "                                                         \
			"setexpr sz ${filesize} / 0x200; "                                      \
			"setexpr sz ${sz} + 1; "                                                \
			"if mmc write ${loadaddr} 0x8a ${sz}; then "                            \
				"echo Flashing of U-boot image succeed; "                       \
			"else echo Flashing of U-boot image failed; "                           \
			"fi; "                                                                  \
		"fi;\0"                                                                         \
	"update_uboot=run download_uboot_spl flash_uboot_spl "                                  \
		"download_uboot_img flash_uboot_img\0"                                          \
	"download_kernel=tftpboot ${loadaddr} ${kernelimg}\0"                                   \
	"flash_kernel="                                                                         \
		"if ext4write mmc ${mmcdev}:${mmcpart} ${loadaddr} /boot/${kernelimg} ${filesize}; then "    \
			"echo kernel update succeed; "                                          \
			"else echo kernel update failed; "                                      \
		"fi;\0"                                                                         \
	"update_kernel=run download_kernel flash_kernel\0"                                      \
	"download_dtb=tftpboot ${fdt_addr} imx6ul-${fdt_name}.dtb\0"                            \
	"flash_dtb="                                                                            \
		"if ext4write mmc ${mmcdev}:${mmcpart} ${fdt_addr} /boot/imx6ul-${fdt_name}.dtb ${filesize}; then " \
			"echo dtb update succeed; "                                             \
			"else echo dtb update in failed; "                                      \
		"fi;\0"                                                                         \
	"update_dtb=run download_dtb flash_dtb\0"                                               \
	"download_rootfs=tftpboot ${loadaddr} ${board_name}-rootfs.ext4\0"                      \
	"flash_rootfs="                                                                         \
		"if mmc dev 0 0; then "                                                         \
			"setexpr nbblocks ${filesize} / 0x200; "                                \
			"setexpr nbblocks ${nbblocks} + 1; "                                    \
			"if mmc write ${loadaddr} 0x40800 ${nbblocks}; then "                   \
				"echo Flashing of rootfs image succeed; "                       \
			"else echo Flashing of rootfs image failed; "                           \
			"fi; "                                                                  \
		"fi;\0"                                                                         \
	"update_rootfs=run download_rootfs flash_rootfs\0"                                      \
	"flash_failsafe="                                                                       \
		"if mmc dev 0 0; then "                                                         \
			"setexpr nbblocks ${filesize} / 0x200; "                                \
			"setexpr nbblocks ${nbblocks} + 1; "                                    \
			"if mmc write ${loadaddr} 0x800 ${nbblocks}; then "                     \
				"echo Flashing of rootfs image in failsafe partition succeed; " \
			"else echo Flashing of rootfs image in failsafe partition failed; "     \
			"fi; "                                                                  \
		"fi;\0"                                                                         \
	"update_failsafe=run download_rootfs flash_failsafe\0"                                  \
	"download_userdata=tftpboot ${loadaddr} ${board_name}-user_data.ext4\0"                 \
	"flash_userdata="                                                                       \
		"if mmc dev 0 0; then "                                                         \
			"setexpr nbblocks ${filesize} / 0x200; "                                \
			"setexpr nbblocks ${nbblocks} + 1; "                                    \
			"if mmc write ${loadaddr} 0 ${nbblocks}; then "                         \
				"echo Flashing of user_data image succeed; "                    \
			"else echo Flashing of user_data image failed; "                        \
			"fi; "                                                                  \
		"fi;\0"                                                                         \
	"update_userdata=run download_userdata flash_userdata; mmc rescan\0"                    \
	"erase_userdata="                                                                       \
		"if mmc dev 0 0; then "                                                         \
			"echo Erasing eMMC User Data partition, no way out...; "                \
			"mw ${loadaddr} 0 0x200000; "                                           \
			"mmc write ${loadaddr} 0 0x1000; "                                      \
			"mmc write ${loadaddr} 0x800 0x1000; "                                  \
			"mmc write ${loadaddr} 0x40800 0x1000; "                                \
			"mmc write ${loadaddr} 0x440800 0x1000; "                               \
		"fi;"                                                                           \
		"mmc rescan\0"                                                                  \
	"update_all=run update_rootfs update_uboot\0"						\
	"initargs=setenv bootargs console=${consoledev},${baudrate} ${extrabootargs}\0"         \
	"addipargs=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:"                       \
		"${gatewayip}:${netmask}:${hostname}:eth0:off\0"                                \
	"addmmcargs=setenv bootargs ${bootargs} root=${mmcroot} "                               \
		"rootfstype=${mmcrootfstype}\0"                                                 \
	"emmcboot=run initargs; run addmmcargs; "                                               \
		"load mmc ${mmcdev}:${mmcpart} ${loadaddr} /boot/${kernelimg} && "              \
		"load mmc ${mmcdev}:${mmcpart} ${fdt_addr} /boot/imx6ul-${fdt_name}.dtb && "    \
		"bootz ${loadaddr} - ${fdt_addr};\0"                                            \
	"emmcsafeboot=setenv mmcpart 1; setenv mmcroot /dev/mmcblk0p1 ro; run emmcboot;\0"      \
	"addnfsargs=setenv bootargs ${bootargs} root=/dev/nfs rw "                              \
		"nfsroot=${serverip}:${rootpath}\0"                                             \
	"nfsboot=run initargs; run addnfsargs addipargs; "                                      \
		"nfs ${loadaddr} ${serverip}:${rootpath}/boot/${kernelimg} && "                 \
		"nfs ${fdt_addr} ${serverip}:${rootpath}/boot/imx6ul-${fdt_name}.dtb && "       \
		"bootz ${loadaddr} - ${fdt_addr};\0"

#endif /* __OPOS6ULDEV_CONFIG_H */
