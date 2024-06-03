/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */
#ifndef __CONFIG_ARIES_MCVEVK_H__
#define __CONFIG_ARIES_MCVEVK_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB on MCV */

/* Booting Linux */
#define CONFIG_BOOTFILE		"fitImage"
#define CONFIG_PREBOOT		"run try_bootscript"
#define CONFIG_BOOTCOMMAND	"run mmc_mmc"
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Environment is in MMC */
#define CONFIG_ENV_OVERWRITE

/* Extra Environment */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"consdev=ttyS0\0"						\
	"baudrate=115200\0"						\
	"bootscript=boot.scr\0"						\
	"setuuid=part uuid mmc 0:3 uuid\0"				\
	"netdev=eth0\0"							\
	"hostname=mcvevk\0"						\
	"kernel_addr_r=0x10000000\0"					\
	"socfpga_legacy_reset_compat=1\0"				\
	"bootm_size=0xa000000\0"					\
	"dfu_alt_info=mmc raw 0 3867148288\0"				\
	"update_filename=u-boot-with-spl.sfp\0"				\
	"update_sd_offset=0x800\0"					\
	"update_sd="		/* Update the SD firmware partition */	\
		"if mmc rescan ; then "					\
		"if tftp ${update_filename} ; then "			\
		"setexpr fw_sz ${filesize} / 0x200 ; "	/* SD block size */ \
		"setexpr fw_sz ${fw_sz} + 1 ; "				\
		"mmc write ${loadaddr} ${update_sd_offset} ${fw_sz} ; "	\
		"fi ; "							\
		"fi\0"							\
	"update_qspi_offset=0x0\0"					\
	"update_qspi="		/* Update the QSPI firmware */		\
		"if sf probe ; then "					\
		"if tftp ${update_filename} ; then "			\
		"sf update ${loadaddr} ${update_qspi_offset} ${filesize} ; " \
		"fi ; "							\
		"fi\0"							\
	"fpga_filename=output_file.rbf\0"				\
	"load_fpga="		/* Load FPGA bitstream */		\
		"if tftp ${fpga_filename} ; then "			\
		"fpga load 0 $loadaddr $filesize ; "			\
		"bridge enable ; "					\
		"fi\0"							\
	"addcons="							\
		"setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
			"${netmask}:${hostname}:${netdev}:off\0"	\
	"addmisc="							\
		"setenv bootargs ${bootargs} ${miscargs}\0"		\
	"addargs=run addcons addmisc\0"					\
	"mmcload="							\
		"mmc rescan ; "						\
		"load mmc 0:2 ${kernel_addr_r} ${bootfile}\0"		\
	"netload="							\
		"tftp ${kernel_addr_r} ${hostname}/${bootfile}\0"	\
	"miscargs=nohlt panic=1\0"					\
	"mmcargs=setenv bootargs root=PARTUUID=${uuid} rw rootwait\0"	\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
	"mmc_mmc="							\
	"run mmcload setuuid mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"mmc_nfs="							\
		"run mmcload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_mmc="							\
	"run netload setuuid mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs="							\
		"run netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"try_bootscript="						\
		"mmc rescan;"						\
		"if test -e mmc 0:2 ${bootscript} ; then "		\
		"if load mmc 0:2 ${kernel_addr_r} ${bootscript};"	\
		"then ; "						\
			"echo Running bootscript... ; "			\
			"source ${kernel_addr_r} ; "			\
		"fi ; "							\
		"fi\0"

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_ARIES_MCVEVK_H__ */
