/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

/*
 * Common options, macros and default environment for all
 * theadorable x86 based boards
 */

#ifndef __THEADORABLE_X86_COMMON_H
#define __THEADORABLE_X86_COMMON_H

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)

#define CONFIG_PREBOOT

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP

/* Environment settings */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x006ec000
#define CONFIG_ENV_OFFSET_REDUND	\
	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)

#undef CONFIG_BOOTCOMMAND
#undef CONFIG_EXTRA_ENV_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"tftpdir=" DEF_ENV_TFTPDIR "\0"				\
	"eth_init=" DEF_ENV_ETH_INIT "\0"			\
	"ubuntu_part=" __stringify(DEF_ENV_UBUNTU_PART) "\0"	\
	"yocto_part=" __stringify(DEF_ENV_YOCTO_PART) "\0"	\
	"ubuntu_tty=" __stringify(DEF_ENV_UBUNTU_TTY) "\0"	\
	"yocto_tty=" __stringify(DEF_ENV_YOCTO_TTY) "\0"	\
	"start_eth=if test -n \"${eth_init}\";"			\
		"then run eth_init;else sleep 0;fi\0"		\
	"kernel-ver=4.8.0-54-generic\0"				\
	"boot=zboot 03000000 0 04000000 ${filesize}\0"		\
	"mtdparts=mtdparts=intel-spi:4k(descriptor),7084k(me)," \
		"8k(env1),8k(env2),64k(mrc),640k(u-boot),"	\
		"64k(vga),-(fsp)\0"				\
	"addtty_ubuntu=setenv bootargs ${bootargs} "		\
		"console=ttyS${ubuntu_tty},${baudrate}\0"	\
	"addtty_yocto=setenv bootargs ${bootargs} "		\
		"console=ttyS${yocto_tty},${baudrate}\0"	\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"	\
	"addmisc=setenv bootargs ${bootargs} "			\
		"intel-spi.writeable=1 vmalloc=300M "		\
		"pci=realloc=on,hpmemsize=0,hpiosize=0\0"	\
	"bootcmd=if env exists recovery_status;"		\
		"then run swupdate;"				\
		"else run yocto_boot;run swupdate;"		\
		"fi\0"						\
	"setroot=part uuid scsi 0:${partnr} uuid;"		\
		"setenv root PARTUUID=${uuid}\0"		\
	"setroot_ubuntu=setenv partnr ${ubuntu_part};run setroot\0" \
	"setroot_yocto=setenv partnr ${yocto_part};run setroot\0" \
	"ubuntu_args=setenv bootargs "				\
		"root=${root} ro\0"				\
	"ubuntu_args_quiet=setenv bootargs "			\
		"root=${root} ro quiet\0"			\
	"ubuntu_load=load scsi 0:${ubuntu_part} 03000000 "	\
		"/boot/vmlinuz-${kernel-ver};"			\
		"load scsi 0:${ubuntu_part} 04000000 "		\
		"/boot/initrd.img-${kernel-ver}\0"		\
	"ubuntu_boot=run setroot_ubuntu ubuntu_args_quiet "	\
		"addmtd addmisc ubuntu_load boot\0"		\
	"ubuntu_boot_console=run setroot_ubuntu ubuntu_args "	\
		"addtty_ubuntu addmtd addmisc ubuntu_load boot\0" \
	"net_args=setenv bootargs root=${root} ro\0"		\
	"net_boot=run start_eth setroot_ubuntu net_args "	\
		"addtty_ubuntu addmtd addmisc;"			\
		"tftp 03000000 ${tftpdir}/bzImage;"		\
		"load scsi 0:${ubuntu_part} 04000000 "		\
		"/boot/initrd.img-${kernel-ver};"		\
		"run boot\0"					\
	"yocto_args=setenv bootargs root=${root} "		\
		"panic=1\0"					\
	"yocto_args_fast=setenv bootargs root=${root} "		\
		"quiet panic=1\0"				\
	"yocto_boot=run setroot_yocto yocto_args addmtd addmisc " \
		"addtty_yocto;"	\
		"if run yocto_load;then zboot 03000000;fi\0"	\
	"yocto_boot_fast=run setroot_yocto yocto_args_fast addmtd " \
		"addmisc addtty_yocto yocto_load;zboot 03000000\0" \
	"yocto_boot_tftp=run setroot_yocto yocto_args addmtd "	\
		"addmisc addtty_yocto "				\
		"start_eth yocto_load_tftp;zboot 03000000\0"	\
	"yocto_kernel=bzImage\0"				\
	"yocto_load=load scsi 0:${yocto_part} 03000000 "	\
		"/boot/${yocto_kernel}\0"			\
	"yocto_load_tftp=tftp 03000000 dfi/bzImage\0"		\
	"swupdate=if env exists swupdate_factory;"		\
		"then run swupdate_usb;run swupdate_run;"	\
		"else setenv swupdate_part 2;run swupdate_mmc;" \
			"run swupdate_run;setenv swupdate_part 1;" \
			"run swupdate_mmc;run swupdate_usb;"	\
			"run swupdate_run;"			\
		"fi\0"						\
	"swupdate-initrd=/boot/swupdate-image-theadorable.ext4.gz\0" \
	"swupdate-kernel=/boot/bzImage\0"			\
	"swupdate_args=setenv bootargs root=/dev/ram rw panic=1\0" \
	"swupdate_dev=0\0"					\
	"swupdate_factory=0\0"					\
	"swupdate_interface=usb\0"				\
	"swupdate_kernel=vmlinuz-4.4.0-28-generic\0"		\
	"swupdate_load=load ${swupdate_interface} ${swupdate_dev}:" \
		"${swupdate_part} 03000000 ${swupdate-kernel}"	\
		" && load ${swupdate_interface} ${swupdate_dev}:" \
		"${swupdate_part} 04000000 ${swupdate-initrd}\0" \
	"swupdate_mmc=setenv swupdate_interface mmc;"		\
		"setenv swupdate_dev ${swupdate_mmcdev};"	\
		"setenv swupdate_part 1;"			\
		"mmc dev ${swupdate_dev};mmc rescan\0"		\
	"swupdate_mmcdev=0\0"					\
	"swupdate_part=1\0"					\
	"swupdate_run=run swupdate_args addtty_yocto addmtd addmisc;" \
		"if run swupdate_load;then run boot;"		\
		"else echo SWUpdate cannot be started from "	\
		"${swupdate_interface};"			\
		"fi\0"						\
	"swupdate_usb=setenv swupdate_interface usb;"		\
		"setenv swupdate_dev 0;setenv swupdate_part 1;"	\
		"usb start\0"					\
	"logo_tftp=tftp ${loadaddr} ${tftpdir}/logo.bmp;"	\
		"bmp display ${loadaddr}\0"			\
	"preboot=scsi scan;load scsi 0:${ubuntu_part} ${loadaddr} " \
		"/boot/logo/logo.bmp;bmp display ${loadaddr}\0" \
	"rootpath=/tftpboot/theadorable-x86-conga/work/"	\
		"rootfs-yocto-swupdate-2017-03-29\0"		\
	"addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:" \
		"${gatewayip}:${netmask}:${hostname}:eth0:off\0" \
	"set_bootargs_nfs=setenv bootargs root=/dev/nfs rw "	\
		"nfsroot=${serverip}:${rootpath},tcp,nfsvers=3\0" \
	"net_nfs=run start_eth set_bootargs_nfs addtty_yocto addip " \
		"addmtd addmisc;tftp 03000000 ${tftpdir}/bzImage;" \
		"zboot 03000000\0"				\
	"load_uboot=tftp ${loadaddr} ${tftpdir}/u-boot.rom\0"	\
	"update_uboot=sf probe;"				\
		"sf update ${loadaddr} 0 800000;saveenv\0"	\
	"upd_uboot=run start_eth load_uboot update_uboot\0"

#endif /* __THEADORABLE_X86_COMMON_H */
