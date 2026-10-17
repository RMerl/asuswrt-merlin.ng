/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2019 Marek Vasut <marex@denx.de>
 */
#ifndef __CONFIG_SOFTING_VINING_FPGA_H__
#define __CONFIG_SOFTING_VINING_FPGA_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB on VINING_FPGA */

/* Booting Linux */

/* Extra Environment */

/*
 * Active LOW GPIO buttons:
 * A: GPIO 77 ... the button between USB B and ethernet
 * B: GPIO 78 ... the button between USB A ports
 *
 * The logic:
 *  if button B is pressed, boot recovery system after 10 seconds
 *  if force_boottype is set, boot system depending on the value in the
 *                            $force_boottype variable after 1 second
 *  if button B is not pressed and force_boottype is not set, boot normal
 *                            Linux system after 5 seconds
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"verify=n\0" \
	"consdev=ttyS0\0"						\
	"baudrate=115200\0"						\
	"bootscript=boot.scr\0"						\
	"ubimtdnr=5\0"							\
	"ubimtd=rootfs\0"						\
	"ubipart=ubi0:vining-fpga-rootfs\0"						\
	"ubisfcs=1\0"		/* Default is flash at CS#1 */		\
	"netdev=eth0\0"							\
	"hostname=vining_fpga\0"					\
	"kernel_addr_r=0x10000000\0"					\
	"fdt_addr_r=0x20000000\0"					\
	"fdt_high=0xffffffff\0"						\
	"initrd_high=0xffffffff\0"					\
	"dfu_alt_info=qspi0 sf 0:0;qspi1 sf 0:1\0"			\
	"mtdparts_0_16m=ff705000.spi.0:" /* 16MiB+128MiB SF config */	\
		"1m(u-boot),"						\
		"64k(env1),"						\
		"64k(env2),"						\
		"256k(softing1),"					\
		"256k(softing2),"					\
		"-(rcvrfs)\0"	/* Recovery */				\
	"mtdparts_0_256m=ff705000.spi.0:" /* 256MiB(+256MiB) config */	\
		"1m(u-boot),"						\
		"64k(env1),"						\
		"64k(env2),"						\
		"256k(softing1),"					\
		"256k(softing2),"					\
		"14720k(rcvrfs),"	/* Recovery */			\
		"192m(rootfs),"		/* Root */			\
		"-(userfs)\0"		/* User */			\
	"mtdparts_1_128m=ff705000.spi.1:" /* 16MiB+128MiB SF config */	\
		"64m(rootfs),"						\
		"-(userfs)\0"						\
	"mtdparts_1_256m=ff705000.spi.1:" /* 256MiB+256MiB SF config */	\
		"-(userfs2)\0"						\
	"update_filename=u-boot-with-spl-dtb.sfp\0"			\
	"update_qspi_offset=0x0\0"					\
	"update_qspi="		/* Update the QSPI firmware */		\
		"if sf probe ; then "					\
		"if tftp ${update_filename} ; then "			\
		"sf update ${loadaddr} ${update_qspi_offset} ${filesize} ; " \
		"fi ; "							\
		"fi\0"							\
	"sf_identify="							\
		"setenv sf_size_0 ; setenv sf_size_1 ; "		\
		"sf probe 0:0 && setenv sf_size_0 ${sf_size} ; "	\
		"sf probe 0:1 && setenv sf_size_1 ${sf_size} ; "	\
		"if test -z \"${sf_size_1}\" ; then "			\
			/* 1x256MiB SF */				\
			"setenv mtdparts_0 ${mtdparts_0_256m} ; "	\
			"setenv mtdparts_1 ; "				\
		"elif test \"${sf_size_0}\" = \"1000000\" ; then "	\
			/* 16MiB+128MiB SF */				\
			"setenv mtdparts_0 ${mtdparts_0_16m} ; "	\
			"setenv mtdparts_1 ${mtdparts_1_128m} ; "	\
		"else "							\
			/* 256MiB+256MiB SF */				\
			"setenv mtdparts_0 ${mtdparts_0_256m} ; "	\
			"setenv mtdparts_1 ${mtdparts_1_256m} ; "	\
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
	"addmtd="							\
		"if test -z \"${sf_size_1}\" ; then "			\
			"setenv mtdparts \"${mtdparts_0}\" ; "		\
		"else "							\
			"setenv mtdparts \"${mtdparts_0};${mtdparts_1}\" ; "	\
		"fi ; "							\
		"setenv bootargs ${bootargs} mtdparts=${mtdparts}\0"	\
	"addargs=run addcons addmtd addmisc\0"				\
	"ubiload="							\
		"ubi part ${ubimtd} ; ubifsmount ${ubipart} ; "		\
		"ubifsload ${kernel_addr_r} /boot/${bootfile} ; "	\
		"ubifsumount ; ubi detach\0"				\
	"netload="							\
		"tftp ${kernel_addr_r} ${hostname}/${bootfile}\0"	\
	"miscargs=nohlt panic=1\0"					\
	"ubiargs="							\
		"setenv bootargs ubi.mtd=${ubimtdnr} "			\
		"root=${ubipart} rootfstype=ubifs\0"			\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
	"ubi_sfsel="							\
		"if test \"${boottype}\" = \"rcvr\" ; then "		\
			"setenv ubisfcs 0 ; "				\
			"setenv ubimtd rcvrfs ; "			\
			"setenv ubimtdnr 5 ; "				\
			"setenv mtdparts mtdparts=${mtdparts_0} ; "	\
			"setenv mtdids nor0=ff705000.spi.0 ; "		\
			"setenv ubipart ubi0:vining-fpga-rootfs ; "	\
		"else "							\
			"if test \"${sf_size_0}\" = \"1000000\" ; then "\
				/* 16MiB+128MiB SF */			\
				"setenv ubisfcs 1 ; "			\
				"setenv ubimtd rootfs ; "		\
				"setenv ubimtdnr 6 ; "			\
				"setenv mtdparts mtdparts=${mtdparts_1} ; "	\
				"setenv mtdids nor0=ff705000.spi.1 ; "	\
				"setenv ubipart ubi0:vining-fpga-rootfs ; "	\
			"else "						\
				/* 256MiB(+256MiB) SF */		\
				"setenv ubisfcs 0 ; "			\
				"setenv ubimtd rootfs ; "		\
				"setenv ubimtdnr 6 ; "			\
				"setenv mtdparts mtdparts=${mtdparts_0} ; "	\
				"setenv mtdids nor0=ff705000.spi.0 ; "	\
				"setenv ubipart ubi0:vining-fpga-rootfs ; "	\
			"fi ; "						\
		"fi ; "							\
		"sf probe 0:${ubisfcs}\0"				\
	"boot_kernel="							\
		"if test -z \"${sf_size_1}\" ; then " /* 1x256MiB SF */	\
			"imxtract ${kernel_addr_r} fdt@1 ${fdt_addr_r} && " \
			"fdt addr ${fdt_addr_r} && "			\
			"fdt resize && "				\
			"fdt set /soc/spi@ff705000/n25q00@1 status disabled && " \
			"bootm ${kernel_addr_r}:kernel@1 - ${fdt_addr_r} ; "	\
		"else "							\
			"bootm ${kernel_addr_r} ; "			\
		"fi\0"							\
	"ubi_ubi="							\
		"run ubi_sfsel ubiload ubiargs addargs boot_kernel\0"	\
	"ubi_nfs="							\
		"run ubiload nfsargs addip addargs boot_kernel\0"	\
	"net_ubi="							\
		"run netload ubiargs addargs boot_kernel\0"		\
	"net_nfs="							\
		"run netload nfsargs addip addargs boot_kernel\0"	\
	"selboot="	/* Select from where to boot. */		\
		"run sf_identify ; "					\
		"if test \"${bootmode}\" = \"qspi\" ; then "		\
			"led all off ; "				\
			"if test \"${boottype}\" = \"rcvr\" ; then "	\
				"echo \"Booting recovery system\" ; "	\
				"led 3 on ; "	/* Bottom RED */	\
			"fi ; "						\
			"led 1 on ; "		/* Top RED */		\
			"run ubi_ubi ; "				\
		"else echo \"Unsupported boot mode: \"${bootmode} ; "	\
		"fi\0"							\
		"socfpga_legacy_reset_compat=1\0"

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOFTING_VINING_FPGA_H__ */
