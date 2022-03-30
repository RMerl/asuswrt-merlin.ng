/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for TI's k2g-evm
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __CONFIG_K2G_EVM_H
#define __CONFIG_K2G_EVM_H

#include <environment/ti/mmc.h>
#include <environment/ti/spi.h>

/* Platform type */
#define CONFIG_SOC_K2G

/* U-Boot general configuration */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DEFAULT_MMC_TI_ARGS						\
	DEFAULT_PMMC_BOOT_ENV						\
	DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	DEFAULT_FIT_TI_ARGS						\
	"boot=mmc\0"							\
	"console=ttyS0,115200n8\0"					\
	"bootpart=0:2\0"						\
	"bootdir=/boot\0"						\
	"rd_spec=-\0"							\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs "	\
	"root=ubi0:rootfs rootflags=sync rw ubi.mtd=ubifs,2048\0"	\
	"findfdt="\
		"if test $board_name = 66AK2GGP; then " \
			 "setenv name_fdt keystone-k2g-evm.dtb; " \
		"else if test $board_name = 66AK2GG1; then " \
			"setenv name_fdt keystone-k2g-evm.dtb; " \
		"else if test $board_name = 66AK2GIC; then " \
			 "setenv name_fdt keystone-k2g-ice.dtb; " \
		"else if test $name_fdt = undefined; then " \
			"echo WARNING: Could not determine device tree to use;"\
		"fi;fi;fi;fi; setenv fdtfile ${name_fdt}\0" \
	"name_mon=skern-k2g.bin\0"					\
	"name_ubi=k2g-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2g-evm.gph\0"				\
	"init_mmc=run args_all args_mmc\0"				\
	"init_fw_rd_mmc=load mmc ${bootpart} ${rdaddr} "		\
		"${bootdir}/${name_fw_rd}; run set_rd_spec\0"		\
	"soc_variant=k2g\0"						\
	"get_fdt_mmc=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${name_fdt}\0"\
	"get_kern_mmc=load mmc ${bootpart} ${loadaddr} "		\
		"${bootdir}/${name_kern}\0"				\
	"get_mon_mmc=load mmc ${bootpart} ${addr_mon} ${bootdir}/${name_mon}\0"\
	"name_fs=arago-base-tisdk-image-k2g-evm.cpio\0"

#ifndef CONFIG_TI_SECURE_DEVICE
#define CONFIG_BOOTCOMMAND						\
	"run findfdt; "							\
	"run envboot; "							\
	"run init_${boot}; "						\
	"run get_mon_${boot} run_mon; "					\
	"run set_name_pmmc get_pmmc_${boot} run_pmmc; "			\
	"run get_kern_${boot}; "					\
	"run init_fw_rd_${boot}; "					\
	"run get_fdt_${boot}; "						\
	"run run_kern"
#else
#define CONFIG_BOOTCOMMAND						\
	"run findfdt; "							\
	"run envboot; "							\
	"run run_mon_hs; "						\
	"run init_${boot}; "						\
	"run get_fit_${boot}; "						\
	"bootm ${fit_loadaddr}#${name_fdt}"
#endif

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* Network */
#define CONFIG_KSNET_NETCP_V1_5
#define CONFIG_KSNET_CPSW_NUM_PORTS	2
#define CONFIG_KSNET_MDIO_PHY_CONFIG_ENABLE
#define PHY_ANEG_TIMEOUT	10000 /* PHY needs longer aneg time */

#define CONFIG_ENV_SIZE			(256 << 10)  /* 256 KiB */

#ifndef CONFIG_SPL_BUILD
#define CONFIG_CADENCE_QSPI
#define CONFIG_CQSPI_REF_CLK 384000000
#endif

#define SPI_MTD_PARTS	KEYSTONE_SPI1_MTD_PARTS

#include <configs/ti_armv7_keystone2.h>

#endif /* __CONFIG_K2G_EVM_H */
