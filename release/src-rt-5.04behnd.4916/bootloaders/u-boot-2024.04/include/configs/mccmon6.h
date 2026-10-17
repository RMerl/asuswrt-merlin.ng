/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016-2017
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CFG_SYS_UBOOT_BASE (CFG_SYS_FLASH_BASE + 0x80000)

/*
 * Below defines are set but NOT really used since we by
 * design force U-Boot run when we boot in development
 * mode from SD card (SD2)
 */

#define CFG_MXC_UART_BASE		UART1_BASE

/* MMC Configuration */
#define CFG_SYS_FSL_USDHC_NUM	2
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* NOR 16-bit mode */
#define CFG_SYS_FLASH_BASE           WEIM_ARB_BASE_ADDR

/* NOR Flash MTD */
#define CFG_SYS_FLASH_BANKS_LIST	{ (CFG_SYS_FLASH_BASE) }
#define CFG_SYS_FLASH_BANKS_SIZES	{ (32 * SZ_1M) }

/* Ethernet Configuration */
#define CFG_FEC_MXC_PHYADDR		1

#define CFG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200 quiet\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"boot_os=yes\0" \
	"kernelsize=0x300000\0" \
	"disable_giga=yes\0" \
	"download_kernel=" \
		"tftpboot ${loadaddr} ${kernel_file};\0" \
	"get_boot_medium=" \
		"setenv boot_medium nor;" \
		"setexpr.l _src_sbmr1 *0x020d8004;" \
		"setexpr _b_medium ${_src_sbmr1} '&' 0x00000040;" \
		"if test ${_b_medium} = 40; then " \
			"setenv boot_medium sdcard;" \
		"fi\0" \
	"kernel_file=fitImage\0" \
	"boot_sd=" \
		"echo '#######################';" \
		"echo '# Factory SDcard Boot #';" \
		"echo '#######################';" \
		"setenv mmcdev 1;" \
		"setenv mmcfactorydev 0;" \
		"setenv mmcfactorypart 1;" \
		"run factory_flash_img;\0" \
	"boot_nor=" \
		"setenv kernelnor 0x08180000;" \
		"setenv bootargs console=${console} " \
		CONFIG_MTDPARTS_DEFAULT " " \
		"root=/dev/mmcblk1 rootfstype=ext4 rw rootwait noinitrd;" \
		"cp.l ${kernelnor} ${loadaddr} ${kernelsize};" \
		"bootm ${loadaddr};reset;\0" \
	"boot_recovery=" \
		"echo '#######################';" \
		"echo '# RECOVERY SWU Boot   #';" \
		"echo '#######################';" \
		"setenv rootfsloadaddr 0x13000000;" \
		"setenv swukernelnor 0x08980000;" \
		"setenv swurootfsnor 0x09180000;" \
		"setenv bootargs console=${console} " \
		CONFIG_MTDPARTS_DEFAULT " " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}" \
		    ":${hostname}::off root=/dev/ram rw;" \
		"cp.l ${swurootfsnor} ${rootfsloadaddr} 0x200000;" \
		"cp.l ${swukernelnor} ${loadaddr} ${kernelsize};" \
		"bootm ${loadaddr} ${rootfsloadaddr};reset;\0" \
	"boot_tftp=" \
		"echo '#######################';" \
		"echo '# TFTP Boot           #';" \
		"echo '#######################';" \
		"if run download_kernel; then " \
		     "setenv bootargs console=${console} " \
		     "root=/dev/mmcblk0p2 rootwait;" \
		     "bootm $loadaddr};reset;" \
		"fi\0" \
	"bootcmd=" \
		"if test -n ${recovery_status}; then " \
		     "run boot_recovery;" \
		"else " \
		     "if test ! -n ${boot_medium}; then " \
			  "run get_boot_medium;" \
			  "if test ${boot_medium} = sdcard; then " \
			      "run boot_sd;" \
			  "else " \
			      "run boot_nor;" \
			  "fi;" \
		     "else " \
			  "if test ${boot_medium} = tftp; then " \
			      "run boot_tftp;" \
			  "fi;" \
		     "fi;" \
		"fi\0" \
	"bootdev=1\0" \
	"bootpart=1\0" \
	"netdev=eth0\0" \
	"load_addr=0x11000000\0" \
	"uboot_file=u-boot.img\0" \
	"SPL_file=SPL\0" \
	"load_uboot=tftp ${load_addr} ${uboot_file}\0" \
	"nor_img_addr=0x11000000\0" \
	"nor_img_file=core-image-lwn-mccmon6.nor\0" \
	"emmc_img_file=core-image-lwn-mccmon6.ext4\0" \
	"nor_bank_start=" __stringify(CFG_SYS_FLASH_BASE) "\0" \
	"nor_img_size=0x02000000\0" \
	"factory_script_file=factory.scr\0" \
	"factory_load_script=" \
		"if test -e mmc ${mmcdev}:${mmcfactorypart} " \
		    "${factory_script_file}; then " \
		    "load mmc ${mmcdev}:${mmcfactorypart} " \
		     "${loadaddr} ${factory_script_file};" \
		"fi\0" \
	"factory_script=echo Running factory script from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"factory_flash_img="\
		"echo 'Flash mccmon6 with factory images'; " \
		"if run factory_load_script; then " \
			"run factory_script;" \
		"else " \
		    "echo No factory script: ${factory_script_file} found on " \
		    "device ${mmcdev};" \
		    "run factory_nor_img;" \
		    "run factory_eMMC_img;" \
		    "run factory_SPL_falcon_setup;" \
		"fi\0" \
	"factory_eMMC_img="\
		"echo 'Update mccmon6 eMMC image'; " \
		"if load mmc ${mmcdev}:${mmcfactorypart} " \
		    "${loadaddr} ${emmc_img_file}; then " \
		    "setexpr fw_sz ${filesize} / 0x200;" \
		    "setexpr fw_sz ${fw_sz} + 1;" \
		    "mmc dev ${mmcfactorydev};" \
		    "mmc write ${loadaddr} 0x0 ${fw_sz};" \
		"fi\0" \
	"factory_nor_img="\
		"echo 'Update mccmon6 NOR image'; " \
		"if load mmc ${mmcdev}:${mmcfactorypart} " \
		    "${nor_img_addr} ${nor_img_file}; then " \
			"run nor_update;" \
		"fi\0" \
	"nor_update=" \
		    "protect off ${nor_bank_start} +${nor_img_size};" \
		    "erase ${nor_bank_start} +${nor_img_size};" \
		    "setexpr nor_img_size ${nor_img_size} / 4; " \
		    "cp.l ${nor_img_addr} ${nor_bank_start} ${nor_img_size}\0" \
	"factory_SPL_falcon_setup="\
		"echo 'Write Falcon boot data'; " \
		"setenv kernelnor 0x08180000;" \
		"cp.l ${kernelnor} ${loadaddr} ${kernelsize};" \
		"spl export fdt ${loadaddr};" \
		"setenv nor_img_addr ${fdtargsaddr};" \
		"setenv nor_img_size 0x20000;" \
		"setenv nor_bank_start " \
				__stringify(CONFIG_CMD_SPL_NOR_OFS)";" \
		"run nor_update\0" \
	"tftp_nor_uboot="\
		"echo 'Update mccmon6 NOR U-BOOT via TFTP'; " \
		"setenv nor_img_file u-boot.img; " \
		"setenv nor_img_size 0x80000; " \
		"setenv nor_bank_start 0x08080000; " \
		"if tftpboot ${nor_img_addr} ${nor_img_file}; then " \
		    "run nor_update;" \
		"fi\0" \
	"tftp_nor_fitImg="\
		"echo 'Update mccmon6 NOR fitImage via TFTP'; " \
		"setenv nor_img_file fitImage; " \
		"setenv nor_img_size 0x500000; " \
		"setenv nor_bank_start 0x08180000; " \
		"if tftpboot ${nor_img_addr} ${nor_img_file}; then " \
		    "run nor_update;" \
		"fi\0" \
	"tftp_nor_img="\
		"echo 'Update mccmon6 NOR image via TFTP'; " \
		"if tftpboot ${nor_img_addr} ${nor_img_file}; then " \
		    "run nor_update;" \
		"fi\0" \
	"tftp_nor_SPL="\
		"if tftp ${load_addr} SPL_padded; then " \
		    "erase 0x08000000 +0x20000;" \
		    "cp.b ${load_addr} 0x08000000 0x20000;" \
		"fi;\0" \
	"tftp_sd_SPL="\
	    "if mmc dev 1; then "      \
		"if tftp ${load_addr} ${SPL_file}; then " \
		    "setexpr fw_sz ${filesize} / 0x200; " \
		    "setexpr fw_sz ${fw_sz} + 1; " \
		    "mmc write ${load_addr} 0x2 ${fw_sz};" \
		"fi;" \
	    "fi;\0" \
	"tftp_sd_uboot="\
	    "if mmc dev 1; then "      \
		"if run load_uboot; then " \
		    "setexpr fw_sz ${filesize} / 0x200; " \
		    "setexpr fw_sz ${fw_sz} + 1; " \
		    "mmc write ${load_addr} 0x8A ${fw_sz};" \
		"fi;" \
	    "fi;\0"

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment organization */

/* Envs are stored in NOR flash */

#endif			       /* __CONFIG_H * */
