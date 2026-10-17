/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Based on m28evk.h:
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */
#ifndef __CONFIGS_MX28EVK_H__
#define __CONFIGS_MX28EVK_H__

/* Memory configuration */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* Max 1 GB RAM */
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* UBI and NAND partitioning */

/* Extra Environment */
#define CFG_EXTRA_ENV_SETTINGS \
	"ubifs_file=filesystem.ubifs\0" \
	"update_nand_full_filename=u-boot.nand\0" \
	"update_nand_firmware_filename=u-boot.sb\0"	\
	"update_nand_firmware_maxsz=0x100000\0"	\
	"update_nand_stride=0x40\0"	/* MX28 datasheet ch. 12.12 */ \
	"update_nand_count=0x4\0"	/* MX28 datasheet ch. 12.12 */ \
	"update_nand_get_fcb_size="	/* Get size of FCB blocks */ \
		"nand device 0 ; " \
		"nand info ; " \
		"setexpr fcb_sz ${update_nand_stride} * ${update_nand_count};" \
		"setexpr update_nand_fcb ${fcb_sz} * ${nand_writesize}\0" \
	"update_nand_firmware_full=" /* Update FCB, DBBT and FW */ \
		"if tftp ${update_nand_full_filename} ; then " \
		"run update_nand_get_fcb_size ; " \
		"nand scrub -y 0x0 ${filesize} ; " \
		"nand write.raw ${loadaddr} 0x0 ${fcb_sz} ; " \
		"setexpr update_off ${loadaddr} + ${update_nand_fcb} ; " \
		"setexpr update_sz ${filesize} - ${update_nand_fcb} ; " \
		"nand write ${update_off} ${update_nand_fcb} ${update_sz} ; " \
		"fi\0" \
	"update_nand_firmware="		/* Update only firmware */ \
		"if tftp ${update_nand_firmware_filename} ; then " \
		"run update_nand_get_fcb_size ; " \
		"setexpr fcb_sz ${update_nand_fcb} * 2 ; " /* FCB + DBBT */ \
		"setexpr fw_sz ${update_nand_firmware_maxsz} * 2 ; " \
		"setexpr fw_off ${fcb_sz} + ${update_nand_firmware_maxsz};" \
		"nand erase ${fcb_sz} ${fw_sz} ; " \
		"nand write ${loadaddr} ${fcb_sz} ${filesize} ; " \
		"nand write ${loadaddr} ${fw_off} ${filesize} ; " \
		"fi\0" \
	"update_nand_kernel="		/* Update kernel */ \
		"mtdparts default; " \
		"nand erase.part kernel; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"nand write ${loadaddr} kernel ${filesize}\0" \
	"update_nand_fdt="		/* Update fdt */ \
		"mtdparts default; " \
		"nand erase.part fdt; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${fdt_file}; " \
		"nand write ${loadaddr} fdt ${filesize}\0" \
	"update_nand_filesystem="		/* Update filesystem */ \
		"mtdparts default; " \
		"nand erase.part filesystem; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${ubifs_file}; " \
		"ubi part filesystem; " \
		"ubi create filesystem; " \
		"ubi write ${loadaddr} filesystem ${filesize}\0" \
	"nandargs=setenv bootargs console=${console_mainline},${baudrate} " \
		"rootfstype=ubifs ubi.mtd=6 root=ubi0_0 ${mtdparts}\0" \
	"nandboot="		/* Boot from NAND */ \
		"mtdparts default; " \
		"run nandargs; " \
		"nand read ${loadaddr} kernel 0x00400000; " \
		"if test ${boot_fdt} = yes; then " \
			"nand read ${fdt_addr} fdt 0x00080000; " \
			"bootz ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"if test ${boot_fdt} = no; then " \
				"bootz; " \
			"else " \
				"echo \"ERROR: Set boot_fdt to yes or no.\"; " \
			"fi; " \
		"fi\0" \
	"update_sd_firmware_filename=u-boot.sd\0" \
	"update_sd_firmware="		/* Update the SD firmware partition */ \
		"if mmc rescan ; then "	\
		"if tftp ${update_sd_firmware_filename} ; then " \
		"setexpr fw_sz ${filesize} / 0x200 ; "	/* SD block size */ \
		"setexpr fw_sz ${fw_sz} + 1 ; "	\
		"mmc write ${loadaddr} 0x800 ${fw_sz} ; " \
		"fi ; "	\
		"fi\0" \
	"script=boot.scr\0"	\
	"image=zImage\0" \
	"console_fsl=ttyAM0\0" \
	"console_mainline=ttyAMA0\0" \
	"fdt_file=imx28-evk.dtb\0" \
	"fdt_addr=0x41000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"mmcroot=/dev/mmcblk0p3 rw rootwait\0" \
	"mmcargs=setenv bootargs console=${console_mainline},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript="  \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; "	\
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console_mainline},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; "	\
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi;" \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0"

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_MX28EVK_H__ */
