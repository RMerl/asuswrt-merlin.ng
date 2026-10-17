/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the phytec PCM-052 SoM.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include <linux/stringify.h>

/* NAND support */

/* if no target-specific extra environment settings were defined by the
   target, define an empty one */
#ifndef PCM052_EXTRA_ENV_SETTINGS
#define PCM052_EXTRA_ENV_SETTINGS
#endif

/* if no target-specific extra environment settings were defined by the
   target, define an empty one */
#ifndef PCM052_NET_INIT
#define PCM052_NET_INIT
#endif

/* boot command, including the target-defined one if any */

/* Extra env settings (including the target-defined ones if any) */
#define CFG_EXTRA_ENV_SETTINGS \
	PCM052_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"blimg_file=u-boot.vyb\0" \
	"blimg_addr=0x81000000\0" \
	"kernel_file=zImage\0" \
	"kernel_addr=0x82000000\0" \
	"fdt_file=zImage.dtb\0" \
	"fdt_addr=0x81000000\0" \
	"ram_file=uRamdisk\0" \
	"ram_addr=0x83000000\0" \
	"filesys=rootfs.ubifs\0" \
	"sys_addr=0x81000000\0" \
	"tftploc=/path/to/tftp/directory/\0" \
	"nfs_root=/path/to/nfs/root\0" \
	"tftptimeout=1000\0" \
	"tftptimeoutcountmax=1000000\0" \
	"bootargs_base=setenv bootargs rw " \
		" mem=" __stringify(CONFIG_PCM052_DDR_SIZE) "M " \
		"console=ttyLP1,115200n8\0" \
	"bootargs_sd=setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 rootwait\0" \
	"bootargs_net=setenv bootargs ${bootargs} root=/dev/nfs ip=dhcp " \
		"nfsroot=${serverip}:${nfs_root},v3,tcp\0" \
	"bootargs_nand=setenv bootargs ${bootargs} " \
		"ubi.mtd=5 rootfstype=ubifs root=ubi0:rootfs\0" \
	"bootargs_ram=setenv bootargs ${bootargs} " \
		"root=/dev/ram rw initrd=${ram_addr}\0" \
	"bootargs_mtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"bootcmd_sd=run bootargs_base bootargs_sd bootargs_mtd; " \
		"fatload mmc 0:1 ${kernel_addr} ${kernel_file}; " \
		"fatload mmc 0:1 ${fdt_addr} ${fdt_file}; " \
		"bootz ${kernel_addr} - ${fdt_addr}\0" \
	"bootcmd_net=run bootargs_base bootargs_net bootargs_mtd; " \
		"tftpboot ${kernel_addr} ${tftpdir}${kernel_file}; " \
		"tftpboot ${fdt_addr} ${tftpdir}${fdt_file}; " \
		"bootz ${kernel_addr} - ${fdt_addr}\0" \
	"bootcmd_nand=run bootargs_base bootargs_nand bootargs_mtd; " \
		"nand read ${fdt_addr} dtb; " \
		"nand read ${kernel_addr} kernel; " \
		"bootz ${kernel_addr} - ${fdt_addr}\0" \
	"bootcmd_ram=run bootargs_base bootargs_ram bootargs_mtd; " \
		"nand read ${fdt_addr} dtb; " \
		"nand read ${kernel_addr} kernel; " \
		"nand read ${ram_addr} root; " \
		"bootz ${kernel_addr} ${ram_addr} ${fdt_addr}\0" \
	"update_bootloader_from_tftp=" PCM052_NET_INIT \
		"if tftp ${blimg_addr} "\
		"${tftpdir}${blimg_file}; then " \
		"mtdparts default; " \
		"nand erase.part bootloader; " \
		"nand write ${blimg_addr} bootloader ${filesize}; fi\0" \
	"update_kernel_from_sd=if fatload mmc 0:2 ${kernel_addr} " \
		"${kernel_file}; " \
		"then mtdparts default; " \
		"nand erase.part kernel; " \
		"nand write ${kernel_addr} kernel ${filesize}; " \
		"if fatload mmc 0:2 ${fdt_addr} ${fdt_file}; then " \
		"nand erase.part dtb; " \
		"nand write ${fdt_addr} dtb ${filesize}; fi\0" \
	"update_kernel_from_tftp=" PCM052_NET_INIT \
		"if tftp ${fdt_addr} ${tftpdir}${fdt_file}; " \
		"then setenv fdtsize ${filesize}; " \
		"if tftp ${kernel_addr} ${tftpdir}${kernel_file}; then " \
		"mtdparts default; " \
		"nand erase.part dtb; " \
		"nand write ${fdt_addr} dtb ${fdtsize}; " \
		"nand erase.part kernel; " \
		"nand write ${kernel_addr} kernel ${filesize}; fi; fi\0" \
	"update_rootfs_from_tftp=" PCM052_NET_INIT \
		"if tftp ${sys_addr} ${tftpdir}${filesys}; " \
		"then mtdparts default; " \
		"nand erase.part root; " \
		"ubi part root; " \
		"ubi create rootfs; " \
		"ubi write ${sys_addr} rootfs ${filesize}; fi\0" \
	"update_ramdisk_from_tftp=" PCM052_NET_INIT \
		"if tftp ${ram_addr} ${tftpdir}${ram_file}; " \
		"then mtdparts default; " \
		"nand erase.part root; " \
		"nand write ${ram_addr} root ${filesize}; fi\0"

/* Miscellaneous configurable options */

/* Physical memory map */
#define PHYS_SDRAM			(0x80000000)
#define PHYS_SDRAM_SIZE			(CONFIG_PCM052_DDR_SIZE * SZ_1M)

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* environment organization */

#endif
