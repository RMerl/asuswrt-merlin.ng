/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */
#ifndef __CONFIG_SAMTEC_VINING_FPGA_H__
#define __CONFIG_SAMTEC_VINING_FPGA_H__

#include <asm/arch/base_addr_ac5.h>

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB on VINING_FPGA */

/* Booting Linux */
#define CONFIG_BOOTFILE		"openwrt-socfpga-socfpga_cyclone5_vining_fpga-fit-uImage.itb"
#define CONFIG_BOOTCOMMAND	"run selboot"
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Ethernet on SoC (EMAC) */
#if defined(CONFIG_CMD_NET)
#define CONFIG_BOOTP_SEND_HOSTNAME
#endif

/* Extra Environment */
#define CONFIG_HOSTNAME			"socfpga_vining_fpga"

/*
 * Active LOW GPIO buttons:
 * A: GPIO 77 ... the button between USB B and ethernet
 * B: GPIO 78 ... the button between USB A ports
 *
 * The logic:
 *  if button B is not pressed, boot normal Linux system immediatelly
 *  if button B is pressed, wait $bootdelay and boot recovery system
 */
#define CONFIG_PREBOOT						\
	"setenv hostname vining-${unit_serial} ; "		\
	"setenv PS1 \"${unit_ident} (${unit_serial}) => \" ; "	\
	"if gpio input 78 ; then "			\
		"setenv bootdelay 10 ; "		\
		"setenv boottype rcvr ; "		\
	"else "						\
		"setenv bootdelay 5 ; "			\
		"setenv boottype norm ; "		\
	"fi"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=n\0" \
	"consdev=ttyS0\0"						\
	"baudrate=115200\0"						\
	"bootscript=boot.scr\0"						\
	"ubimtdnr=5\0"							\
	"ubimtd=rootfs\0"						\
	"ubipart=ubi0:rootfs\0"						\
	"ubisfcs=1\0"		/* Default is flash at CS#1 */		\
	"netdev=eth0\0"							\
	"hostname=vining_fpga\0"						\
	"kernel_addr_r=0x10000000\0"					\
	"mtdparts_0=ff705000.spi.0:"					\
		"1m(u-boot),"						\
		"64k(env1),"						\
		"64k(env2),"						\
		"256k(samtec1),"					\
		"256k(samtec2),"					\
		"-(rcvrfs)\0"	/* Recovery */				\
	"mtdparts_1=ff705000.spi.1:"					\
		"32m(rootfs),"						\
		"-(userfs)\0"						\
	"update_filename=u-boot-with-spl-dtb.sfp\0"			\
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
	"addmtd="							\
		"setenv mtdparts \"${mtdparts_0};${mtdparts_1}\" ; "	\
		"setenv bootargs ${bootargs} mtdparts=${mtdparts}\0"	\
	"addargs=run addcons addmtd addmisc\0"				\
	"ubiload="							\
		"ubi part ${ubimtd} ; ubifsmount ${ubipart} ; "		\
		"ubifsload ${kernel_addr_r} /boot/${bootfile}\0"	\
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
			"setenv ubipart ubi0:rootfs ; "			\
		"else "							\
			"setenv ubisfcs 1 ; "				\
			"setenv ubimtd rootfs ; "			\
			"setenv ubimtdnr 6 ; "				\
			"setenv mtdparts mtdparts=${mtdparts_1} ; "	\
			"setenv mtdids nor0=ff705000.spi.1 ; "		\
			"setenv ubipart ubi0:rootfs ; "			\
		"fi ; "							\
		"sf probe 0:${ubisfcs}\0"				\
	"ubi_ubi="							\
		"run ubi_sfsel ubiload ubiargs addargs ; "		\
		"bootm ${kernel_addr_r}\0"				\
	"ubi_nfs="							\
		"run ubiload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_ubi="							\
		"run netload ubiargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs="							\
		"run netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"selboot="	/* Select from where to boot. */		\
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

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_OFFSET		0x100000
#define CONFIG_ENV_OFFSET_REDUND	\
	(CONFIG_ENV_OFFSET + CONFIG_ENV_SECT_SIZE)

/* Support changing the prompt string */
#define CONFIG_CMDLINE_PS_SUPPORT

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SAMTEC_VINING_FPGA_H__ */
