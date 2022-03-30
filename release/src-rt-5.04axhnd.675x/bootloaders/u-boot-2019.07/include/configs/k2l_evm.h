/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for TI's k2l-evm
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __CONFIG_K2L_EVM_H
#define __CONFIG_K2L_EVM_H

#include <environment/ti/spi.h>

/* Platform type */
#define CONFIG_SOC_K2L

#ifdef CONFIG_TI_SECURE_DEVICE
#define DEFAULT_SEC_BOOT_ENV						\
	DEFAULT_FIT_TI_ARGS						\
	"findfdt=setenv fdtfile ${name_fdt}\0"
#else
#define DEFAULT_SEC_BOOT_ENV
#endif

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	DEFAULT_SEC_BOOT_ENV						\
	"boot=ubi\0"							\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,4096\0"	\
	"name_fdt=keystone-k2l-evm.dtb\0"				\
	"name_mon=skern-k2l.bin\0"					\
	"name_ubi=k2l-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2l-evm.gph\0"				\
	"name_fs=arago-console-image-k2l-evm.cpio.gz\0"

#define CONFIG_ENV_SIZE				(256 << 10)  /* 256 KiB */
#define CONFIG_ENV_OFFSET			0x100000

#include <configs/ti_armv7_keystone2.h>

#define SPI_MTD_PARTS KEYSTONE_SPI0_MTD_PARTS

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_4K

/* Network */
#define CONFIG_KSNET_NETCP_V1_5
#define CONFIG_KSNET_CPSW_NUM_PORTS	5
#define CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE

#endif /* __CONFIG_K2L_EVM_H */
