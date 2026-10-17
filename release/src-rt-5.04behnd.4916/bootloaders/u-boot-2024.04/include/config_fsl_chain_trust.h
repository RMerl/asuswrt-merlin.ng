/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_FSL_CHAIN_TRUST_H
#define __CONFIG_FSL_CHAIN_TRUST_H

#include <linux/stringify.h>

#ifdef CONFIG_CHAIN_OF_TRUST

/*
 * Control should not reach back to uboot after validation of images
 * for secure boot flow and therefore bootscript should have
 * the bootm command. If control reaches back to uboot anyhow
 * after validating images, core should just spin.
 */

#ifdef CONFIG_USE_BOOTARGS
#define SET_BOOTARGS	"setenv bootargs \'" CONFIG_BOOTARGS" \';"
#else
#define SET_BOOTARGS	"setenv bootargs \'root=/dev/ram "	\
				"rw console=ttyS0,115200 ramdisk_size=600000\';"
#endif

#define SECBOOT \
	"setenv bs_hdraddr " __stringify(CONFIG_BOOTSCRIPT_HDR_ADDR)";" \
	SET_BOOTARGS	\
	"esbc_validate $bs_hdraddr;" \
	"source $img_addr;"	\
	"esbc_halt\0"

#ifdef CONFIG_BOOTSCRIPT_COPY_RAM
#define BS_COPY_ENV \
	"setenv bs_hdr_ram " __stringify(CONFIG_BS_HDR_ADDR_RAM)";" \
	"setenv bs_hdr_device " __stringify(CONFIG_BS_HDR_ADDR_DEVICE)";" \
	"setenv bs_hdr_size " __stringify(CONFIG_BS_HDR_SIZE)";" \
	"setenv bs_ram " __stringify(CONFIG_BS_ADDR_RAM)";" \
	"setenv bs_device " __stringify(CONFIG_BS_ADDR_DEVICE)";" \
	"setenv bs_size " __stringify(CONFIG_BS_SIZE)";"

/* For secure boot flow, default environment used will be used */
#if defined(CONFIG_SYS_RAMBOOT) || defined(CONFIG_NAND_BOOT) || \
	defined(CONFIG_SD_BOOT)
#if defined(CONFIG_NAND_BOOT)
#define BS_COPY_CMD \
	"nand read $bs_hdr_ram $bs_hdr_device $bs_hdr_size ;" \
	"nand read $bs_ram $bs_device $bs_size ;"
#elif defined(CONFIG_SD_BOOT)
#define BS_COPY_CMD \
	"mmc read $bs_hdr_ram $bs_hdr_device $bs_hdr_size ;" \
	"mmc read $bs_ram $bs_device $bs_size ;"
#endif
#else
#define BS_COPY_CMD \
	"cp.b $bs_hdr_device $bs_hdr_ram  $bs_hdr_size ;" \
	"cp.b $bs_device $bs_ram  $bs_size ;"
#endif
#else /* !CONFIG_BOOTSCRIPT_COPY_RAM */
#define BS_COPY_ENV
#define BS_COPY_CMD
#endif /* CONFIG_BOOTSCRIPT_COPY_RAM */

#define CHAIN_BOOT_CMD	BS_COPY_ENV \
			BS_COPY_CMD \
			SECBOOT

#endif
#endif
