/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

/* Falcon Mode */

/* Falcon Mode - MMC support */

/*
 * display5 SPI-NOR memory layout
 *
 * The definition can be found in Kconfig's
 * CONFIG_MTDIDS_DEFAULT and CONFIG_MTDPARTS_DEFAULT
 *
 * 0x000000 - 0x020000 : SPI.SPL (128KiB)
 * 0x020000 - 0x120000 : SPI.u-boot (1MiB)
 * 0x120000 - 0x130000 : SPI.u-boot-env1 (64KiB)
 * 0x130000 - 0x140000 : SPI.u-boot-env2 (64KiB)
 * 0x140000 - 0x740000 : SPI.swupdate-kernel-FIT (6MiB)
 * 0x740000 - 0x1B40000 : SPI.swupdate-initramfs  (20MiB)
 * 0x1B40000 - 0x1F00000 : SPI.reserved (3840KiB)
 * 0x1F00000 - 0x2000000 : SPI.factory  (1MiB)
 */

/* Below values are "dummy" - only to avoid build break */
#define CFG_SYS_SPI_KERNEL_OFFS      0x150000
#define CFG_SYS_SPI_ARGS_OFFS        0x140000
#define CFG_SYS_SPI_ARGS_SIZE        0x10000

#define CFG_MXC_UART_BASE		UART5_BASE

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0
#define CFG_SYS_FSL_USDHC_NUM	2

#define PARTS_DEFAULT \
	/* Linux partitions */ \
	"partitions=" \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=kernel_raw1,start=128K,size=8M,uuid=${uuid_gpt_kernel_raw1};" \
	"name=rootfs1,size=1528M,uuid=${uuid_gpt_rootfs1};" \
	"name=kernel_raw2,size=8M,uuid=${uuid_gpt_kernel_raw2};" \
	"name=rootfs2,size=512M,uuid=${uuid_gpt_rootfs2};" \
	"name=data,size=-,uuid=${uuid_gpt_data}\0"

#define SWUPDATE_RECOVERY_PROCEDURE \
	"echo '#######################';" \
	"echo '# RECOVERY SWUupdate  #';" \
	"echo '#######################';" \
	"echo '#######################';" \
	"echo '# GPT verify          #';" \
	"if gpt verify mmc ${mmcdev} ${partitions}; then " \
		"echo '# OK !                #';" \
	"else " \
		"echo '# FAILED !            #';" \
		"echo '# GPT RESTORATION     #';" \
		"gpt write mmc ${mmcdev} ${partitions};" \
	"fi;" \
	"echo '#######################';" \
	"setenv loadaddr_swu_initramfs 0x14000000;" \
	"setenv bootargs console=${console} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}" \
		":${hostname}::off root=/dev/ram rw;" \
	"sf probe;" \
	"sf read ${loadaddr} swu-kernel;" \
	"sf read ${loadaddr_swu_initramfs} swu-initramfs;" \
	"bootm ${loadaddr} ${loadaddr_swu_initramfs};reset;"

#define SETUP_BOOTARGS \
	"run set_rootfs_part;" \
	"setenv bootargs ${bootargs} console=${console} "	  \
		      "root=/dev/mmcblk${mmcdev}p${rootfs_part} " \
		      "rootwait rootfstype=ext4 rw; " \
	"run set_kernel_part;" \
	"part start mmc ${mmcdev} ${kernel_part} lba_start; " \
	"mmc read ${loadaddr} ${lba_start} ${fitImg_fw_sz}; " \
	"setenv fdt_conf imx6q-${board}-${display}.dtb; "

/* All the numbers are in LBAs */
#define __TFTP_UPDATE_KERNEL \
	"tftp_mmc_fitImg=" \
	   "if test ! -n ${kernel_part}; then " \
	       "setenv kernel_part ${kernel_part_active};" \
	   "fi;" \
	   "if tftp ${loadaddr} ${kernel_file}; then " \
	       "setexpr fw_sz ${filesize} / 0x200; " \
	       "setexpr fw_sz ${fw_sz} + 1; "  \
	       "part start mmc ${mmcdev} ${kernel_part} lba_start; " \
	       "mmc write ${loadaddr} ${lba_start} ${fw_sz}; " \
	   "; fi\0" \

#define TFTP_UPDATE_KERNEL \
	"setenv kernel_part ${kernel_part_active};" \
	"run tftp_mmc_fitImg;" \
	"setenv kernel_part ${kernel_part_backup};" \
	"run tftp_mmc_fitImg;" \

#define __TFTP_UPDATE_ROOTFS \
	"tftp_mmc_rootfs=" \
	   "if test ! -n ${rootfs_part}; then " \
	       "setenv rootfs_part ${rootfs_part_active};" \
	   "fi;" \
	   "if tftp ${loadaddr} ${rootfs_file}; then " \
	       "setexpr fw_sz ${filesize} / 0x200; " \
	       "setexpr fw_sz ${fw_sz} + 1; "  \
	       "part start mmc ${mmcdev} ${rootfs_part} lba_start; " \
	       "mmc write ${loadaddr} ${lba_start} ${fw_sz}; " \
	   "; fi\0" \

#define TFTP_UPDATE_ROOTFS \
	"setenv rootfs_part ${rootfs_part_active};" \
	"run tftp_mmc_rootfs;" \
	"run tftp_mmc_rootfs_bkp;" \


#define TFTP_UPDATE_RECOVERY_SWU_KERNEL \
	"tftp_sf_fitImg_SWU=" \
	    "if tftp ${loadaddr} ${kernel_file}; then " \
		"sf probe;" \
		"sf erase swu-kernel +${filesize};" \
		"sf write ${loadaddr} swu-kernel ${filesize};" \
	"; fi\0"	  \

#define TFTP_UPDATE_RECOVERY_SWU_INITRAMFS \
	"swu_initramfs_file=swupdate-image-display5.ext4.gz.u-boot\0" \
	"tftp_sf_initramfs_SWU=" \
	    "if tftp ${loadaddr} ${swu_initramfs_file}; then " \
		"sf probe;" \
		"sf erase swu-initramfs +${filesize};" \
		"sf write ${loadaddr} swu-initramfs ${filesize};" \
	"; fi\0"	  \

#define TFTP_UPDATE_BOOTLOADER \
	"ubootfile=u-boot.img\0" \
	"ubootfileSPL=SPL\0" \
	"tftp_sf_uboot=" \
	    "if tftp ${loadaddr} ${ubootfile}; then " \
		"sf probe;" \
		"sf erase u-boot +${filesize};" \
		"sf write ${loadaddr} u-boot ${filesize}" \
	"; fi\0"	  \
	"tftp_sf_SPL="	  \
	    "if tftp ${loadaddr} ${ubootfileSPL}; then " \
		"sf probe;" \
		"setexpr uboot_SPL_size ${filesize} + 0x400;" \
		"sf erase 0x0 +${uboot_SPL_size};" \
		"sf write ${loadaddr} 0x400 ${filesize};" \
	"fi\0" \

#define TFTP_UPDATE_SPINOR \
	"spinorfile=core-image-lwn-display5.spinor\0" \
	"spinorsize=0x2000000\0" \
	"tftp_sf_img=" \
	    "if tftp ${loadaddr} ${spinorfile}; then " \
		"sf probe;" \
		"sf erase 0x0 ${spinorsize};" \
		"sf write ${loadaddr} 0x0 ${filesize};" \
	"fi\0" \

#define CFG_EXTRA_ENV_SETTINGS	  \
	PARTS_DEFAULT \
	"gpio_recovery=93\0" \
	"check_em_pad=gpio input ${gpio_recovery};test $? -eq 0;\0" \
	"display=tianma-tm070-800x480\0" \
	"board=display5\0" \
	"mmcdev=0\0" \
	"altbootcmd=run recovery\0" \
	"bootdelay=1\0" \
	"baudrate=115200\0" \
	"ethact=FEC\0" \
	"netdev=eth0\0" \
	"boot_os=y\0" \
	"hostname=display5\0" \
	"loadaddr=0x12000000\0" \
	"fdtaddr=0x12800000\0" \
	"console=ttymxc4,115200 quiet cma=256M\0" \
	"fdtfile=imx6q-display5.dtb\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"kernel_file=fitImage\0" \
	"fitImg_fw_sz=0x2200\0" \
	"up=run tftp_sf_SPL; run tftp_sf_uboot\0" \
	"download_kernel=" \
		"tftpboot ${loadaddr} ${kernel_file};\0" \
	"factory_nfs=" \
	     "setenv ipaddr 192.168.1.102;" \
	     "setenv gatewayip 192.168.1.1;" \
	     "setenv netmask 255.255.255.0;" \
	     "setenv serverip 192.168.1.2;" \
	     "echo BOOT: FACTORY (LEG);" \
	     "run boot_nfs\0" \
	"boot_swu_recovery=" SWUPDATE_RECOVERY_PROCEDURE "\0" \
	"recovery=" \
	     "echo BOOT: RECOVERY: SWU;" \
	     "run boot_swu_recovery\0" \
	"boot_tftp=" \
	"if run download_kernel; then "	  \
	     "setenv bootargs console=${console} " \
	     "root=/dev/mmcblk0p2 rootwait;" \
	     "bootm ${loadaddr} - ${fdtaddr};reset;" \
	"fi\0" \
	"addip=setenv bootargs ${bootargs} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
	    "${hostname}:eth0:on"	  \
	"\0"	  \
	"nfsargs=setenv bootargs " \
	"root=/dev/nfs rw "	  \
	"nfsroot=${serverip}:${rootpath},nolock,nfsvers=3" \
	"\0" \
	"rootpath=/srv/tftp/DISP5/rootfs\0" \
	"boot_nfs=" \
	"if run download_kernel; then "	  \
	     "run nfsargs;"	  \
	     "run addip;"	  \
	     "setenv bootargs ${bootargs} console=${console};"	  \
	     "setenv fdt_conf imx6q-${board}-${display}.dtb; " \
	     "bootm ${loadaddr}#conf@${fdt_conf};reset;" \
	"fi\0" \
	"falcon_setup=" \
	"if mmc dev ${mmcdev}; then "	  \
	     SETUP_BOOTARGS \
	     "spl export fdt ${loadaddr}#conf@${fdt_conf};" \
	     "setexpr fw_sz ${fdtargslen} / 0x200; " \
	     "setexpr fw_sz ${fw_sz} + 1; "  \
	     "mmc write ${fdtargsaddr} " \
	     __stringify(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)" ${fw_sz}; " \
	"fi\0" \
	"boot_mmc=" \
	"if mmc dev ${mmcdev}; then "	  \
	     SETUP_BOOTARGS \
	     "bootm ${loadaddr}#conf@${fdt_conf};reset;" \
	"fi\0" \
	"set_kernel_part=" \
	"if test ${BOOT_FROM} = ACTIVE; then " \
	     "setenv kernel_part ${kernel_part_active};" \
	     "echo BOOT: ACTIVE;" \
	"else if test ${BOOT_FROM} = BACKUP; then " \
	     "setenv kernel_part ${kernel_part_backup};" \
	     "echo BOOT: BACKUP;" \
	"else " \
	     "run recovery;" \
	"fi;fi\0" \
	"set_rootfs_part=" \
	"if test ${BOOT_FROM} = ACTIVE; then " \
	     "setenv rootfs_part ${rootfs_part_active};" \
	"else if test ${BOOT_FROM} = BACKUP; then " \
	     "setenv rootfs_part ${rootfs_part_backup};" \
	"else " \
	     "run recovery;" \
	"fi;fi\0" \
	"BOOT_FROM=ACTIVE\0" \
	TFTP_UPDATE_BOOTLOADER \
	TFTP_UPDATE_SPINOR \
	"kernel_part_active=1\0" \
	"kernel_part_backup=3\0" \
	__TFTP_UPDATE_KERNEL \
	"rootfs_part_active=2\0" \
	"rootfs_part_backup=4\0" \
	"rootfs_file=core-image-lwn-display5.ext4\0" \
	"rootfs_file_backup=core-image-lwn-backup-display5.ext4\0" \
	__TFTP_UPDATE_ROOTFS \
	"tftp_mmc_rootfs_bkp=" \
	   "setenv rootfs_part ${rootfs_part_backup};" \
	   "setenv rootfs_file ${rootfs_file_backup};" \
	   "run tftp_mmc_rootfs\0" \
	TFTP_UPDATE_RECOVERY_SWU_KERNEL \
	TFTP_UPDATE_RECOVERY_SWU_INITRAMFS \
	"\0" \

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* ENV config */
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
/* The 0x120000 value corresponds to above SPI-NOR memory MAP */
#endif

#define CFG_MXC_USB_PORTSC           (PORT_PTS_UTMI | PORT_PTS_PTW)
#endif /* __CONFIG_H */
