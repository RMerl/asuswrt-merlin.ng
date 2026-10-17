/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6DL aristainetos2 board.
 */
#ifndef __ARISTAINETOS2_CONFIG_H
#define __ARISTAINETOS2_CONFIG_H

#if (CONFIG_SYS_BOARD_VERSION == 5)
#define CONSOLE_DEV	"ttymxc1"
#elif (CONFIG_SYS_BOARD_VERSION == 6)
#define CONSOLE_DEV	"ttymxc0"
#endif

/* Framebuffer */
#define CFG_SYS_LDB_CLOCK	28341000

#include "mx6_common.h"


/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR      USDHC1_BASE_ADDR

#define CFG_FEC_MXC_PHYADDR		0

#ifdef CONFIG_IMX_HAB
#define HAB_EXTRA_SETTINGS \
	"hab_check_addr=" \
		"if hab_auth_img ${check_addr} ${filesize} ; then " \
			"true;" \
		"else " \
			"echo \"HAB checks ${hab_check_filetype} " \
			"failed!\"; " \
			"false; " \
		"fi;\0" \
	"hab_check_file_fit=" \
		"if env exists enable_hab_check && test " \
			"${enable_hab_check} -eq 1 ; then " \
			"setenv hab_check_filetype \"FIT file on SD card " \
			"or eMMC\";" \
			"env set check_addr ${fit_addr_r};" \
			"run hab_check_addr;" \
		"else " \
			"true; "\
		"fi;\0" \
	"hab_check_file_bootscript=" \
		"if env exists enable_hab_check && test " \
			"${enable_hab_check} -eq 1 ; then " \
			"setenv hab_check_filetype \"Bootscript file\";" \
			"env set check_addr ${loadaddr};" \
			"run hab_check_addr;" \
		"else " \
			"true; "\
		"fi;\0" \
	"hab_check_flash_fit=" \
		"if env exists enable_hab_check && test " \
			"${enable_hab_check} -eq 1 ; then " \
			"setenv hab_check_filetype \"FIT files on flash\";" \
			"env set check_addr ${fit_addr_r};" \
			"run hab_check_addr;" \
		"else " \
			"true; "\
		"fi;\0" \
	"enable_hab_check=1\0"
#else
#define HAB_EXTRA_SETTINGS \
	"hab_check_addr=echo HAB check addr always returns " \
		"true;true\0" \
	"hab_check_file_fit=echo HAB check FIT file always returns " \
		"true;true\0" \
	"hab_check_flash_fit=echo HAB check flash FIT always returns " \
		"true;true\0" \
	"hab_check_file_bootscript=echo HAB check bootscript always " \
		"returns true;true\0" \
	"enable_hab_check=0\0"
#endif

#if (CONFIG_SYS_BOARD_VERSION == 5)
#define EXTRA_ENV_BOARD_SETTINGS \
	"dead=while true; do; " \
		"led led_red on; sleep 1;" \
		"led led_red off; sleep 1;" \
	"done\0"
#elif (CONFIG_SYS_BOARD_VERSION == 6)
#define EXTRA_ENV_BOARD_SETTINGS \
	"dead=while true; do; " \
		"led led_red on; led led_red2 on; sleep 1;" \
		"led led_red off; led led_red2 off;; sleep 1;" \
	"done\0"
#endif

#define CFG_EXTRA_ENV_SETTINGS \
	"disable_giga=yes\0" \
	"usb_pgood_delay=2000\0" \
	"nor_bootdelay=-2\0" \
	"script=u-boot.scr\0" \
	"loadaddr=0x12000000\0" \
	"fit_addr_r=0x14000000\0" \
	"uboot_sz=d0000\0" \
	"panel=lb07wv8\0" \
	"splashpos=m,m\0" \
	"console=" CONSOLE_DEV "\0" \
	"emmcroot=/dev/mmcblk1p1 rootwait rw\0" \
	"mk_fitfile_path=setenv fit_file /${sysnum}/system.itb\0" \
	"mk_rescue_fitfile_path=setenv rescue_fit_file /${rescue_sysnum}/system.itb\0" \
	"mk_uboot_path=setenv uboot /${sysnum}/u-boot.imx\0" \
	"mk_pubkey_path=setenv pubkey /${sysnum}/PCR.pem\0" \
	"mk_rescue_pubkey_path=setenv pubkey /${rescue_sysnum}/PCR.pem\0" \
	"addmisc=setenv bootargs ${bootargs} net.ifnames=0 consoleblank=0 " \
		"bootmode=${bootmode} rng_core.default_quality=1000 " \
		"mmcpart=${mmcpart} emmcpart=${emmcpart} sysnum=${sysnum}\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"boot_board_type=bootm ${fit_addr_r}#${board_type}\0" \
	"get_env=mw ${loadaddr} 0 0x20000;" \
		"mmc rescan;" \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} env.txt;" \
		"env import -t ${loadaddr}\0" \
	"default_env=gpio set wp_spi_nor.gpio-hog;" \
		"sf probe;" \
		"sf protect unlock 0 0x1000000;" \
		"mw ${loadaddr} 0 0x20000;" \
		"env export -t ${loadaddr} serial# ethaddr " \
		"board_type panel;" \
		"env default -a;" \
		"env import -t ${loadaddr}\0" \
	"loadbootscript=" \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${script};\0" \
	"loadbootscriptUSB=" \
		"ext4load usb 0 ${loadaddr} ${script};\0" \
	"loadbootscriptUSBf=" \
		"fatload usb 0 ${loadaddr} ${script};\0" \
	"bootscriptUSB=echo Running bootscript from usb-stick ...; " \
		"source \0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source \0" \
	"mmcpart=1\0" \
	"mmcdev=0\0" \
	"emmcpart=1\0" \
	"emmcdev=1\0" \
	"sysnum=1\0" \
	"rescue_sysnum=0\0" \
	"rreason=18\0" \
	"mainboot=echo Booting from eMMC ...; " \
		"run mainargs addmtd addmisc;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mainargs=setenv bootargs console=${console},${baudrate} " \
		"root=${emmcroot} rootfstype=ext4\0 " \
	"main_load_fit=run mk_fitfile_path; " \
		"ext4load mmc ${emmcdev}:${emmcpart} ${fit_addr_r} " \
		"${fit_file}; " \
		"imi ${fit_addr_r}\0 " \
	"rescue_load_fit=run mk_rescue_fitfile_path; " \
		"ext4load mmc ${emmcdev}:${emmcpart} ${fit_addr_r} " \
		"${rescue_fit_file}; " \
		"imi ${fit_addr_r}\0" \
	"main_load_pubkey=run mk_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"ext4load mmc ${emmcdev}:${emmcpart} ${loadaddr} " \
		"${pubkey}\0" \
	"rescue_load_pubkey=run mk_rescue_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"ext4load mmc ${emmcdev}:${emmcpart} ${loadaddr} " \
		"${pubkey}\0" \
	"mainRargs=setenv bootargs console=${console},${baudrate} " \
		"rescue_sysnum=${rescue_sysnum} root=${emmcroot} rootfstype=ext4\0" \
	"mmcroot=/dev/mmcblk0p1 rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"mmcRargs=setenv bootargs console=${console},${baudrate} " \
		"rescue_sysnum=${rescue_sysnum} root=${mmcroot}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs addmtd addmisc;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mmc_load_fit=run mk_fitfile_path; " \
		"ext4load mmc ${mmcdev}:${mmcpart} ${fit_addr_r} " \
		"${fit_file}\0" \
		"imi ${fit_addr_r}\0" \
	"mmc_rescue_load_fit=run mk_rescue_fitfile_path; " \
		"ext4load mmc ${mmcdev}:${mmcpart} " \
		"${fit_addr_r} ${rescue_fit_file}\0" \
		"imi ${fit_addr_r}\0" \
	"mmc_load_uboot=run mk_uboot_path; " \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${uboot}\0" \
	"mmc_upd_uboot=mw.b ${loadaddr} 0xff ${uboot_sz};" \
		"setexpr cmp_buf ${loadaddr} + ${uboot_sz};" \
		"setexpr uboot_maxsize ${uboot_sz} - 400;" \
		"mw.b ${cmp_buf} 0x00 ${uboot_sz};" \
		"run mmc_load_uboot;sf probe;sf erase 0 ${uboot_sz};" \
		"sf write ${loadaddr} 400 ${filesize};" \
		"sf read ${cmp_buf} 400 ${uboot_sz};" \
		"cmp.b ${loadaddr} ${cmp_buf} ${uboot_maxsize}\0" \
	"mmc_load_pubkey=run mk_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${pubkey}\0" \
	"mmc_rescue_load_pubkey=run mk_rescue_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${pubkey}\0" \
	"rescueboot=echo Booting rescue system ...; " \
		"run addmtd addmisc;" \
		"if test -n ${rescue_reason}; then run rescue_reason;fi;" \
		"run boot_board_type;" \
		"if bootm ${fit_addr_r}; then ; " \
		"else " \
			"run dead; " \
		"fi; \0" \
	"r_reason_syserr=setenv rescue_reason setenv bootargs " \
		"\\\\${bootargs} " \
		"rescueReason=$rreason\0 " \
	"usb_load_fit=run mk_fitfile_path; " \
		"ext4load usb 0 ${fit_addr_r} ${fit_file}\0" \
	"usb_load_fitf=run mk_fitfile_path; " \
		"fatload usb 0 ${fit_addr_r} ${fit_file}\0" \
	"usb_load_rescuefit=run mk_rescue_fitfile_path; " \
		"ext4load usb 0 ${fit_addr_r} " \
		"${rescue_fit_file}\0" \
	"usb_load_rescuefitf=run mk_rescue_fitfile_path; " \
		"fatload usb 0 ${fit_addr_r} " \
		"${rescue_fit_file}\0" \
	"usb_load_pubkey=run mk_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"ext4load usb 0 ${loadaddr} ${pubkey}\0" \
	"usb_rescue_load_pubkey=run mk_rescue_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"ext4load usb 0 ${loadaddr} ${pubkey}\0" \
	"usb_load_pubkeyf=run mk_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"fatload usb 0 ${loadaddr} ${pubkey}\0" \
	"usb_rescue_load_pubkeyf=run mk_rescue_pubkey_path; " \
		"setenv hab_check_filetype \"PCR.pem\";" \
		"env set check_addr ${loadaddr};" \
		"fatload usb 0 ${loadaddr} ${pubkey}\0" \
	"usbroot=/dev/sda1 rootwait rw\0" \
	"usbboot=echo Booting from usb-stick ...; " \
		"run usbargs addmtd addmisc;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"usbargs=setenv bootargs console=${console},${baudrate} " \
		"root=${usbroot}\0" \
	"usbRargs=setenv bootargs console=${console},${baudrate} " \
		"rescue_sysnum=${rescue_sysnum} root=${usbroot} rw\0 " \
	"mmc_rescue_boot=" \
		"run r_reason_syserr;" \
		"if run mmc_rescue_load_pubkey hab_check_addr " \
		"mmc_rescue_load_fit hab_check_file_fit; then " \
			"run mmcRargs; run rescueboot; " \
		"else " \
			"echo RESCUE SYSTEM FROM SD-CARD BOOT FAILURE;" \
			"run dead; " \
		"fi;\0" \
	"main_rescue_boot=" \
		"if run main_load_pubkey hab_check_addr " \
		"main_load_fit hab_check_flash_fit; then " \
			"if run mainboot; then ; " \
			"else " \
				"run r_reason_syserr;" \
				"if run rescue_load_pubkey hab_check_addr " \
				"rescue_load_fit hab_check_file_fit; then " \
					"run mainRargs; run rescueboot; " \
				"else " \
					"echo RESCUE SYSTEM BOOT FAILURE;" \
					"run dead; " \
				"fi; " \
			"fi; " \
		"else " \
			"run r_reason_syserr;" \
			"if run rescue_load_pubkey hab_check_addr " \
			"rescue_load_fit hab_check_file_fit; then " \
				"run mainRargs; run rescueboot; " \
			"else " \
				"echo RESCUE SYSTEM BOOT FAILURE;" \
				"run dead; " \
			"fi; " \
		"fi;\0" \
	"usb_mmc_rescue_boot=" \
		"usb start;" \
		"if usb storage; then " \
			"if run loadbootscriptUSB " \
				"hab_check_file_bootscript;" \
				"then run bootscriptUSB; " \
			"fi; " \
			"if run loadbootscriptUSBf " \
				"hab_check_file_bootscript;" \
				"then run bootscriptUSB; " \
			"fi; " \
			"if run usb_load_pubkey hab_check_addr " \
			"usb_load_fit hab_check_file_fit; then " \
				"run usbboot; " \
			"fi; " \
			"if run usb_load_pubkeyf hab_check_addr " \
			"usb_load_fitf hab_check_file_fit; then " \
				"run usbboot; " \
			"fi; "\
			"if run usb_rescue_load_pubkey hab_check_addr " \
			"usb_load_rescuefit hab_check_file_fit; then " \
				"run r_reason_syserr usbRargs; run rescueboot;" \
			"fi; " \
			"if run usb_rescue_load_pubkeyf hab_check_addr " \
			"usb_load_rescuefitf hab_check_file_fit; then " \
				"run r_reason_syserr usbRargs; run rescueboot;" \
			"fi; " \
			"run mmc_rescue_boot;" \
		"fi; "\
		"run mmc_rescue_boot;\0" \
	"rescue_xload_boot=" \
		"run r_reason_syserr;" \
		"if test ${bootmode} -ne 0 ; then " \
			"mmc dev ${mmcdev};" \
			"if mmc rescan; then " \
				"if run mmc_rescue_load_pubkey " \
				"hab_check_addr " \
				"mmc_rescue_load_fit " \
				"hab_check_file_fit; then " \
					"run mmcRargs; run rescueboot; " \
				"else " \
					"usb start;" \
					"if usb storage; then " \
						"if run usb_rescue_load_pubkey " \
						"hab_check_addr " \
						"usb_load_rescuefit " \
						"hab_check_file_fit; then " \
							"run usbRargs; run rescueboot;" \
						"fi; " \
						"if run usb_rescue_load_pubkeyf " \
						"hab_check_addr " \
						"usb_load_rescuefitf " \
						"hab_check_file_fit; then " \
							"run usbRargs; run rescueboot;" \
						"fi; " \
					"fi;" \
				"fi;" \
				"echo RESCUE SYSTEM ON SD OR " \
					"USB BOOT FAILURE;" \
				"run dead; " \
			"else " \
				"usb start;" \
				"if usb storage; then " \
					"if run usb_rescue_load_pubkey " \
					"hab_check_addr " \
					"usb_load_rescuefit " \
					"hab_check_file_fit; then " \
						"run usbRargs; run rescueboot;" \
					"fi; " \
					"if run usb_rescue_load_pubkeyf " \
					"hab_check_addr " \
					"usb_load_rescuefitf " \
					"hab_check_file_fit; then " \
						"run usbRargs; run rescueboot;" \
					"fi; " \
				"fi;" \
				"echo RESCUE SYSTEM ON USB BOOT FAILURE;" \
				"run dead; " \
			"fi; " \
		"else "\
			"if run rescue_load_pubkey hab_check_addr " \
			"rescue_load_fit hab_check_file_fit; then " \
				"run mainRargs; run rescueboot; " \
			"else " \
				"echo RESCUE SYSTEM ON BOARD BOOT FAILURE;" \
				"run dead; " \
			"fi; " \
		"fi;\0" \
	"ari_boot=if test ${bootmode} -ne 0 ; then " \
		"mmc dev ${mmcdev};" \
		"if mmc rescan; then " \
			"if run loadbootscript hab_check_file_bootscript;" \
				"then run bootscript; " \
			"fi; " \
			"if run mmc_load_pubkey hab_check_addr " \
			"mmc_load_fit hab_check_file_fit; then " \
				"if run mmcboot; then ; " \
				"else " \
					"run mmc_rescue_boot;" \
				"fi; " \
			"else " \
				"run usb_mmc_rescue_boot;" \
			"fi; " \
		"else " \
			"run usb_mmc_rescue_boot;" \
		"fi; " \
	"else "\
		"run main_rescue_boot;" \
	"fi; \0"\
	HAB_EXTRA_SETTINGS \
	EXTRA_ENV_BOARD_SETTINGS

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CFG_SYS_FSL_USDHC_NUM	2

/* DMA stuff, needed for GPMI/MXS NAND support */

/* USB Configs */
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS	0

/* UBI support */

#define CFG_ENV_FLAGS_LIST_STATIC "ethaddr:mw,serial#:sw,board_type:sw," \
		"sysnum:dw,panel:sw,ipaddr:iw,serverip:iw"

#endif                         /* __ARISTAINETOS2_CONFIG_H */
