/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Xilinx
 * (C) Copyright 2017 Opal Kelly Inc.
 *
 * Configuration settings for the SYZYGY Hub development board
 * See zynq-common.h for Zynq common configs
 */

#ifndef __CONFIG_SYZYGY_HUB_H
#define __CONFIG_SYZYGY_HUB_H

#define CONFIG_EXTRA_ENV_SETTINGS       \
	"fit_image=fit.itb\0"		\
	"bitstream_image=download.bit\0"    \
	"loadbit_addr=0x1000000\0"      \
	"load_addr=0x2000000\0"		\
	"fit_size=0x800000\0"		\
	"flash_off=0x100000\0"		\
	"nor_flash_off=0xE2100000\0"	\
	"fdt_high=0x20000000\0"		\
	"initrd_high=0x20000000\0"	\
	"loadbootenv_addr=0x2000000\0"  \
	"fdt_addr_r=0x1f00000\0"        \
	"pxefile_addr_r=0x2000000\0"    \
	"kernel_addr_r=0x2000000\0"     \
	"scriptaddr=0x3000000\0"        \
	"ramdisk_addr_r=0x3100000\0"    \
	"bootenv=uEnv.txt\0" \
	"bootenv_dev=mmc\0" \
	"loadbootenv=load ${bootenv_dev} 0 ${loadbootenv_addr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from ${bootenv_dev} ...; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"bootenv_existence_test=test -e ${bootenv_dev} 0 /${bootenv}\0" \
	"setbootenv=if env run bootenv_existence_test; then " \
			"if env run loadbootenv; then " \
				"env run importbootenv; " \
			"fi; " \
		"fi; \0" \
	"sd_loadbootenv=set bootenv_dev mmc && " \
			"run setbootenv \0" \
	"usb_loadbootenv=set bootenv_dev usb && usb start && run setbootenv\0" \
	"preboot=if test $modeboot = sdboot; then " \
			"run sd_loadbootenv; " \
			"echo Checking if uenvcmd is set ...; " \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...; " \
				"run uenvcmd; " \
			"fi; " \
		"fi; \0" \
	"sdboot=echo Copying FPGA Bitstream from SD to RAM... && " \
		"load mmc 0 ${loadbit_addr} ${bitstream_image} && " \
		"echo Programming FPGA... && " \
		"fpga loadb 0 ${loadbit_addr} ${filesize} && " \
		"echo Copying FIT from SD to RAM... && " \
		"load mmc 0 ${load_addr} ${fit_image} && " \
		"bootm ${load_addr}\0" \
	"jtagboot=echo TFTPing FIT to RAM... && " \
		"tftpboot ${load_addr} ${fit_image} && " \
		"bootm ${load_addr}\0" \
		DFU_ALT_INFO \
		BOOTENV

#include <configs/zynq-common.h>

#endif /* __CONFIG_SYZYGY_HUB_H */
