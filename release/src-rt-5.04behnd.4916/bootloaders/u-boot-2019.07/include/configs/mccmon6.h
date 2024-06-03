/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016-2017
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#include "imx6_spl.h"

#define CONFIG_SYS_UBOOT_START CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_UBOOT_BASE (CONFIG_SYS_FLASH_BASE + 0x80000)
#define CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_OS_BASE (CONFIG_SYS_FLASH_BASE + 0x180000)
#define CONFIG_SYS_FDT_BASE (CONFIG_SYS_FLASH_BASE + 0x1980000)
#define CONFIG_SYS_FDT_SIZE (48 * SZ_1K)
#define CONFIG_SYS_SPL_ARGS_ADDR	0x18000000

/*
 * Below defines are set but NOT really used since we by
 * design force U-Boot run when we boot in development
 * mode from SD card (SD2)
 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR (0x800)
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS (0x80)
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR (0x1000)
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME "uImage"
#define CONFIG_SPL_FS_LOAD_ARGS_NAME "imx6q-mccmon.dtb"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 500 * SZ_1M)

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000

/* MMC Configuration */
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

/* NOR 16-bit mode */
#define CONFIG_SYS_FLASH_BASE           WEIM_ARB_BASE_ADDR
#define CONFIG_SYS_FLASH_CFI_WIDTH FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_FLASH_VERIFY

/* NOR Flash MTD */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ (CONFIG_SYS_FLASH_BASE) }
#define CONFIG_SYS_FLASH_BANKS_SIZES	{ (32 * SZ_1M) }

/* MTD support */

/* USB Configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0

/* Ethernet Configuration */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		1

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200 quiet\0" \
	"fdtfile=imx6q-mccmon6.dtb\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"boot_os=yes\0" \
	"download_kernel=" \
		"tftpboot ${kernel_addr} ${kernel_file};" \
		"tftpboot ${fdt_addr} ${fdtfile};\0" \
	"get_boot_medium=" \
		"setenv boot_medium nor;" \
		"setexpr.l _src_sbmr1 *0x020d8004;" \
		"setexpr _b_medium ${_src_sbmr1} '&' 0x00000040;" \
		"if test ${_b_medium} = 40; then " \
			"setenv boot_medium sdcard;" \
		"fi\0" \
	"kernel_file=uImage\0" \
	"load_kernel=" \
		"load mmc ${bootdev}:${bootpart} ${kernel_addr} uImage;" \
		"load mmc ${bootdev}:${bootpart} ${fdt_addr} ${fdtfile};\0" \
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
		"setenv dtbnor 0x09980000;" \
		"setenv bootargs console=${console} " \
		CONFIG_MTDPARTS_DEFAULT " " \
		"root=/dev/mmcblk1 rootfstype=ext4 rw rootwait noinitrd;" \
		"cp.l ${dtbnor} ${dtbloadaddr} 0x8000;" \
		"bootm ${kernelnor} - ${dtbloadaddr};\0" \
	"boot_recovery=" \
		"echo '#######################';" \
		"echo '# RECOVERY SWU Boot   #';" \
		"echo '#######################';" \
		"setenv rootfsloadaddr 0x13000000;" \
		"setenv swukernelnor 0x08980000;" \
		"setenv swurootfsnor 0x09180000;" \
		"setenv swudtbnor 0x099A0000;" \
		"setenv bootargs console=${console} " \
		CONFIG_MTDPARTS_DEFAULT " " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}" \
		    ":${hostname}::off root=/dev/ram rw;" \
		"cp.l ${swurootfsnor} ${rootfsloadaddr} 0x200000;" \
		"cp.l ${swudtbnor} ${dtbloadaddr} 0x8000;" \
		"bootm ${swukernelnor} ${rootfsloadaddr} ${dtbloadaddr};\0" \
	"boot_tftp=" \
		"echo '#######################';" \
		"echo '# TFTP Boot           #';" \
		"echo '#######################';" \
		"if run download_kernel; then " \
		     "setenv bootargs console=${console} " \
		     "root=/dev/mmcblk0p2 rootwait;" \
		     "bootm ${kernel_addr} - ${fdt_addr};" \
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
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"fdt_addr=0x18000000\0" \
	"bootdev=1\0" \
	"bootpart=1\0" \
	"kernel_addr=" __stringify(CONFIG_LOADADDR) "\0" \
	"netdev=eth0\0" \
	"load_addr=0x11000000\0" \
	"dtbloadaddr=0x12000000\0" \
	"uboot_file=u-boot.img\0" \
	"SPL_file=SPL\0" \
	"load_uboot=tftp ${load_addr} ${uboot_file}\0" \
	"nor_img_addr=0x11000000\0" \
	"nor_img_file=core-image-lwn-mccmon6.nor\0" \
	"emmc_img_file=core-image-lwn-mccmon6.ext4\0" \
	"nor_bank_start=" __stringify(CONFIG_SYS_FLASH_BASE) "\0" \
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
	"tftp_nor_uboot="\
		"echo 'Update mccmon6 NOR U-BOOT via TFTP'; " \
		"setenv nor_img_file u-boot.img; " \
		"setenv nor_img_size 0x80000; " \
		"setenv nor_bank_start 0x08080000; " \
		"if tftpboot ${nor_img_addr} ${nor_img_file}; then " \
		    "run nor_update;" \
		"fi\0" \
	"tftp_nor_uImg="\
		"echo 'Update mccmon6 NOR uImage via TFTP'; " \
		"setenv nor_img_file uImage; " \
		"setenv nor_img_size 0x500000; " \
		"setenv nor_bank_start 0x08180000; " \
		"if tftpboot ${nor_img_addr} ${nor_img_file}; then " \
		    "run nor_update;" \
		"fi\0" \
	"tftp_nor_dtb="\
		"echo 'Update mccmon6 NOR DTB via TFTP'; " \
		"setenv nor_img_file imx6q-mccmon6.dtb; " \
		"setenv nor_img_size 0x20000; " \
		"setenv nor_bank_start 0x09980000; " \
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

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE			(SZ_128K)

/* Envs are stored in NOR flash */
#define CONFIG_ENV_SECT_SIZE    (SZ_128K)
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + 0x40000)

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + 0x60000)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#endif			       /* __CONFIG_H * */
