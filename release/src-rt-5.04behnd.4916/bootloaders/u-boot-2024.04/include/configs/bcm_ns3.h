/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#ifndef __BCM_NS3_H
#define __BCM_NS3_H

#include <linux/sizes.h>

/* Physical Memory Map */
#define V2M_BASE			0x80000000
#define PHYS_SDRAM_1			V2M_BASE

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/*
 * Initial SP before reloaction is placed at end of first DRAM bank,
 * which is 0x1_0000_0000.
 * Just before re-loaction, new SP is updated and re-location happens.
 * So pointing the initial SP to end of 2GB DDR is not a problem
 */
/* 12MB Malloc size */

/* console configuration */
#define CFG_SYS_NS16550_CLK		25000000

/*
 * Increase max uncompressed/gunzip size, keeping size same as EMMC linux
 * partition.
 */

/* Access eMMC Boot_1 and Boot_2 partitions */

/* enable 64-bit PCI resources */

#define CONSOLE_ARGS "console_args=console=ttyS0,115200n8\0"
#define MAX_CPUS "max_cpus=maxcpus=8\0"
#define OS_LOG_LEVEL "log_level=loglevel=7\0"
#define EXTRA_ARGS "extra_args=earlycon=uart8250,mmio32,0x68A10000 " \
		   "earlyelog=" __stringify(ELOG_AP_UART_LOG_BASE) ",0x10000 " \
		   "crashkernel=512M reboot=w\0"

#define PCIE_ARGS "pcie_args=pci=pcie_bus_safe pcie_ports=native vfio_pci.disable_idle_d3=1\0"

#ifdef CONFIG_BCM_SF2_ETH
#define BCM_ETH_ADDR "ethaddr=00:0A:F7:95:65:A4\0"
#define NET_ARGS "bgmac_platform.ethaddr=${ethaddr} " \
	"ip=${ipaddr}::${gatewayip}:${netmask}::${ethif}:off"
#else
#define BMC_ETH_ADDR
#define NET_ARGS
#endif

#define RESERVED_MEM "reserved_mem=memmap=0xff000000$0x1000000\0"

#define BASE_ARGS "${console_args} ${extra_args} ${pcie_args}" \
		  " ${max_cpus}  ${log_level} ${reserved_mem}"
#define SETBOOTARGS "setbootargs=setenv bootargs " BASE_ARGS " " NET_ARGS "\0"

#define UPDATEME_FLASH_PARAMS "bcm_compat_level=4\0" \
			      "bcm_need_recovery_rootfs=0\0" \
			      "bcm_bl_flash_pending_rfs_imgs=0\0"

#define KERNEL_LOADADDR_CFG \
	"fit_image_loadaddr=0x90000000\0" \
	"dtb_loadaddr=0x82000000\0"

#define INITRD_ARGS "initrd_args=root=/dev/ram rw\0"
#define INITRD_LOADADDR "initrd_loadaddr=0x92000000\0"
#define INITRD_IMAGE "initrd_image=rootfs-lake-bcm958742t.cpio.gz\0"
#define MMC_DEV "sd_device_number=0\0"
#define EXEC_STATE "exec_state=normal\0"

#define EXT4RD_ARGS "ext4rd_args="\
	"root=/dev/mmcblk${sd_device_number}p${gpt_partition_entry} rw rootwait\0"

#define WDT_CNTRL "wdt_enable=1\0" \
		  "wdt_timeout_sec=0\0"

#define ELOG_SETUP \
	"mbox0_addr=0x66424024\0"\
	"elog_setup="\
	"if logsetup -s ${mbox0_addr}; then "\
	"else "\
		"echo ELOG is not supported by this version of the MCU patch.;"\
		"exit;"\
	"fi;"\
	"if logsetup -c ${mbox0_addr}; then "\
		"echo ELOG is ready;"\
	"else "\
		"echo ELOG is supported, but is not set up.;"\
		"echo Getting setup file from the server ${serverip}...;"\
		"if tftp ${tftp_dir}elog_src.txt; then "\
			"echo Setting up ELOG. Please wait...;"\
			"if logsetup ${loadaddr} ${mbox0_addr} ${filesize}; "\
				"then "\
			"else "\
				"echo [logsetup] ERROR.;"\
			"fi;"\
			"if logsetup -c ${mbox0_addr}; then "\
				"echo ELOG is READY.;"\
			"else "\
				"echo ELOG is NOT SET UP.;"\
			"fi;"\
		"else "\
			"echo ELOG setup file is not available on the server.;"\
		"fi;"\
	"fi \0"

/* eMMC partition for FIT images */
#define FIT_MMC_PARTITION \
	"fit_partitions=" \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=env,size=512K,uuid=${uuid_gpt_env};" \
	"name=Image_rsa.img,size=24MiB,uuid=${uuid_gpt_linux};" \
	"name=Image1_rsa.img,size=24MiB,uuid=${uuid_gpt_linux1};" \
	"name=Image2_rsa.img,size=24MiB,uuid=${uuid_gpt_linux2};" \
	"name=nitro,size=8MiB,uuid=${uuid_gpt_nitro};" \
	"name=recovery,size=940MiB,uuid=${uuid_gpt_recovery};" \
	"name=rootfs,size=-,uuid=${uuid_gpt_prootfs}\0"

#define QSPI_FLASH_NITRO_PARAMS \
	"spi_nitro_img_bin_start=0x400000\0" \
	"spi_nitro_img_bin_mirror_start=0x580000\0" \
	"spi_nitro_bspd_cfg_start=0x700000\0" \
	"spi_nitro_bspd_mirror_cfg_start=0x710000\0" \

#define QSPI_ACCESS_ENABLE \
	"qspi_access_en=" \
	"mw 0x68a403e8 1;" \
	"mw 0x68a403ec 1;" \
	"mw 0x68a403f0 1;" \
	"mw 0x68a403f4 1;" \
	"mw 0x68a403f8 1;" \
	"mw 0x68a403fc 1 \0"

#define FUNC_QSPI_PROBE \
	"func_qspi_probe="\
	"if run qspi_access_en; then "\
	"else "\
		"echo ${errstr} run qspi_access_en ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf probe 0; then "\
	"else "\
		"echo echo ${errstr} sf probe command ** FAILED **;"\
		"exit;"\
	"fi \0"

#define NITRO_FW_IMAGES \
	"nitro_bin=nitro.img\0" \
	"nitro_bspd_cfg=nitro_fb_bspd_config.bin\0"

#define FASTBOOT_NITRO_SETUP \
	"nitro_fastboot_type=1\0" \
	"nitro_fastboot_secure=1\0" \
	"nitro_fastboot_img_buffer=0\0" \
	"nitro_fit_img_loc=0x90000000\0"

#define FASTBOOT_SETUP \
	"fastboot_nitro_setup=" \
	"setenv errstr fastboot_setup;" \
	"run func_qspi_probe;" \
	/* first load header only */ \
	"if sf read ${nitro_fit_img_loc} "\
		   "${spi_nitro_img_bin_start} 0x18; then "\
	"else "\
		"echo [fastboot_nitro_setup] sf read "\
		      "${spi_nitro_img_bin_start} ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if spi_nitro_images_addr ${nitro_fit_img_loc} "\
				 "${spi_nitro_img_bin_start}; then "\
	"else "\
		 "echo [fastboot_nitro_setup] spi_nitro_images_addr "\
		       "** FAILED **;"\
		"exit;"\
	"fi \0"

#define CHECK_CHIMP_HS\
	"check_chimp_hs=chimp_hs"\
	"\0"

#define FASTBOOT_NITRO "fastboot_nitro=chimp_ld_secure\0"

#define FIT_IMAGE "fit_image=Image_rsa.img\0"
#define BOOTCMD_MMC_FIT \
	"bootcmd_mmc_fit="\
	"mmc dev ${sd_device_number};"\
	"if test $exec_state = normal; then " \
		"setenv use_rootfs rootfs;"\
	"else " \
		"setenv use_rootfs recovery;"\
	"fi;" \
	"echo used filesystem :${use_rootfs};"\
	"gpt setenv mmc ${sd_device_number} ${use_rootfs};"\
	"setenv bootargs_fs ${setbootargs} ${ext4rd_args}; run bootargs_fs;"\
	"gpt setenv mmc ${sd_device_number} ${fit_image};"\
	"mmc read ${fit_image_loadaddr} ${gpt_partition_addr} "\
	"${gpt_partition_size};"\
	"bootm ${fit_image_loadaddr}\0"

#define BOOTCMD_MMC_FITS \
	"bootcmd_mmc_fits="\
	"setenv mmc_fit0 " \
	"'setenv fit_image Image_rsa.img; run bootcmd_mmc_fit';"\
	"setenv mmc_fit1 " \
	"'setenv fit_image Image1_rsa.img; run bootcmd_mmc_fit';"\
	"setenv mmc_fit2 " \
	"'setenv fit_image Image2_rsa.img; run bootcmd_mmc_fit';"\
	"run mmc_fit0 || run mmc_fit1 || run mmc_fit2\0"

#define USBDEV "usbdev=0\0"
#define BOOTCMD_USB\
	"bootcmd_usb="\
	"setenv usb_image_loadaddr 90000000;"\
	"setenv fit_image Image_rsa.img;"\
	"setenv bootargs_fs ${setbootargs} ${initrd_args}; run bootargs_fs;"\
	"if usb dev ${usbdev}; && usb start; then "\
		"echo Booting from USB...;"\
		"fatload usb ${usbdev} ${usb_image_loadaddr} ${fit_image};"\
		"fatload usb ${usbdev} ${initrd_loadaddr} ${initrd_image};"\
		"bootm ${usb_image_loadaddr} ${initrd_loadaddr}:${filesize};"\
	"fi;"\
	"\0"

#define START_PCI\
	"start_pci=pci e "\
	"\0"

#define BNXT_LOAD\
	"bnxt_load=bnxt 0 probe "\
	"\0"

#define BOOTCMD_PXE\
	"bootcmd_pxe="\
	"run check_chimp_hs && "\
	"run start_pci && "\
	"run bnxt_load;"\
	"setenv ethact bnxt_eth0;"\
	"setenv autoload no;"\
	"setenv bootargs_fs ${setbootargs} ${initrd_args}; run bootargs_fs;"\
	"if dhcp; then "\
		"setenv pxefile_addr_r ${loadaddr};"\
		"if pxe get; then "\
			"setenv ramdisk_addr_r ${initrd_loadaddr};"\
			"setenv kernel_addr_r ${fit_image_loadaddr};"\
			"pxe boot; "\
		"fi;"\
	"fi;"\
	"\0"

#define FLASH_PENDING_RFS_IMGS \
	"flash_pending_rfs_imgs=" \
	"if test $bcm_bl_flash_pending_rfs_imgs = 1; then " \
		"if test $bl_flash_pending_rfs_imgs = rootfs; then " \
			"dhcp;" \
			"run mmc_flash_rootfs;" \
		"fi;" \
		"if test $bl_flash_pending_rfs_imgs = recovery; then " \
			"dhcp;" \
			"run mmc_flash_recovery;" \
		"fi;" \
		"setenv bl_flash_pending_rfs_imgs;" \
	"fi; \0"

/* Flashing commands */
#define TFTP_QSPI_PARAM \
	"fip_qspi_addr=0x0\0"\
	"fip_qspi_mirror_addr=0x200000\0"\
	"loadaddr=0x90000000\0"\
	"tftpblocksize=1468\0"\
	"qspi_flash_fip=fip\0"\

/* Flash fit_GPT partition to eMMC */
#define MMC_FLASH_FIT_GPT \
	"mmc_flash_gpt="\
	"if mmc dev ${sd_device_number}; then "\
	"else "\
		"echo [mmc_flash_gpt] mmc dev ${sd_device_number} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if gpt write mmc ${sd_device_number} ${fit_partitions}; then "\
	"else "\
		"echo [mmc_flash_gpt] gpt write ${fit_partitions} "\
		"** FAILED **;"\
		"exit;"\
	"fi \0"

#define MMC_FLASH_IMAGE_RSA \
	"mmc_flash_image_rsa="\
	"if mmc dev ${sd_device_number}; then "\
	"else "\
		"echo [mmc_flash_image_rsa] mmc dev ${sd_device_number} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if gpt setenv mmc ${sd_device_number} ${fit_image}; then "\
	"else "\
		"echo [mmc_flash_image_rsa] gpt setenv ${fit_image} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if tftp ${loadaddr} ${tftp_dir}${fit_image}; then "\
		"if test ${fit_image} = Image_rsa.img; then "\
			"if setenv tftp_fit_image yes; then "\
			"else "\
				"echo [mmc_flash_image_rsa] "\
				"setenv tftp_fit_image to yes"\
				"** FAILED **;"\
				"exit;"\
			"fi;"\
		"fi;"\
	"else "\
		"if test ${fit_image} = Image_rsa.img; then "\
			"echo [mmc_flash_image_rsa] tftp "\
			"${tftp_dir}${fit_image} ** FAILED **;"\
		"else "\
			"if test ${tftp_fit_image} = yes; then "\
				"if mmc write ${loadaddr} "\
				"${gpt_partition_addr} "\
				"${fileblocks}; then "\
				"else "\
					"echo "\
					"[mmc_flash_image_rsa] "\
					"mmc write "\
					"${gpt_partition_addr} "\
					"** FAILED **;"\
					"exit;"\
				"fi;"\
			"else "\
				"echo [mmc_flash_image_rsa] tftp "\
				"${tftp_dir}${fit_image} "\
				"** FAILED **;"\
			"fi;"\
		"fi;"\
		"exit;"\
	"fi;"\
	"if math add filesize filesize 1FF; then "\
	"else "\
		"echo [mmc_flash_image_rsa] math add command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if math div fileblocks filesize 200; then "\
	"else "\
		"echo [mmc_flash_image_rsa] math div command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if mmc write ${loadaddr} ${gpt_partition_addr} ${fileblocks}; then "\
	"else "\
		"echo [mmc_flash_image_rsa] mmc write ${gpt_partition_addr} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if setenv image_sz_blk_cnt ${fileblocks}; then "\
	"else "\
		"echo [mmc_flash_image_rsa] setenv image_sz_blk_cnt ** "\
		"FAILED **;"\
		"exit;"\
	"fi;"\
	"if saveenv; then "\
	"else "\
		"echo [mmc_flash_image_rsa] saveenv command ** FAILED **;"\
		"exit;"\
	"fi \0"

#define MMC_FLASH_RECOVERY \
	"mmc_flash_recovery="\
	"if mmc dev ${sd_device_number}; then "\
	"else "\
		"echo [mmc_flash_recovery] mmc dev ${sd_device_number} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if gpt setenv mmc ${sd_device_number} recovery; then "\
	"else "\
		"echo [mmc_flash_recovery] gpt setenv recovery ** FAILED **;"\
		"exit;"\
	"fi;"\
	"setenv index 1;"\
	"while tftp ${loadaddr} "\
	"${tftp_dir}${gpt_partition_name}/chunk_00${index}; do "\
		"if math add filesize filesize 1FF; then "\
		"else "\
			"echo [mmc_flash_recovery] math add command "\
			"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if math div fileblocks filesize 200; then "\
		"else "\
			"echo [mmc_flash_recovery] math div command "\
			"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if mmc write ${loadaddr} ${gpt_partition_addr} "\
		"${fileblocks}; then "\
		"else "\
			"echo [mmc_flash_recovery] mmc write "\
			"${gpt_partition_addr} ** FAILED **;"\
			"exit;"\
		"fi;"\
		"if math add index index 1; then "\
		"else "\
			"echo [mmc_flash_recovery] math add command "\
			"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if math add gpt_partition_addr gpt_partition_addr"\
		" ${fileblocks}; then "\
		"else "\
			"echo [mmc_flash_recovery] math add command"\
			" ** FAILED **;"\
			"exit;"\
		"fi;"\
	"done;"\
	"if itest ${index} -ne 1; then "\
	"else "\
		"echo [mmc_flash_recovery] "\
		"${tftp_dir}${gpt_partition_name}/chunk_00${index} file "\
		"not found ** FAILED **;"\
		"exit;"\
	"fi \0"

#define MMC_FLASH_ROOTFS \
	"mmc_flash_rootfs="\
	"if mmc dev ${sd_device_number}; then "\
	"else "\
		"echo [mmc_flash_rootfs] mmc dev ${sd_device_number} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if gpt setenv mmc ${sd_device_number} rootfs; then "\
	"else "\
		"echo [mmc_flash_rootfs] gpt setenv rootfs ** FAILED **;"\
		"exit;"\
	"fi;"\
	"setenv index 1;"\
	"while tftp ${loadaddr} "\
	"${tftp_dir}${gpt_partition_name}/chunk_00${index}; do "\
		"if math add filesize filesize 1FF; then "\
		"else "\
			"echo [mmc_flash_rootfs] math add command "\
			"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if math div fileblocks filesize 200; then "\
		"else "\
			"echo [mmc_flash_rootfs] math div command "\
			"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if mmc write ${loadaddr} ${gpt_partition_addr} "\
		"${fileblocks}; then "\
		"else "\
			"echo [mmc_flash_rootfs] mmc write "\
			"${gpt_partition_addr} ** FAILED **;"\
			"exit;"\
		"fi;"\
		"if math add index index 1; then "\
		"else "\
			"echo [mmc_flash_rootfs] math add command "\
			"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if math add gpt_partition_addr gpt_partition_addr"\
		" ${fileblocks}; then "\
		"else "\
			"echo [mmc_flash_rootfs] math add command"\
			" ** FAILED **;"\
			"exit;"\
		"fi;"\
	"done;"\
	"if itest ${index} -ne 1; then "\
	"else "\
		"echo [mmc_flash_rootfs] "\
		"${tftp_dir}${gpt_partition_name}/chunk_00${index} file "\
		"not found ** FAILED **;"\
		"exit;"\
	"fi \0"

/*
 * For individual flash commands like mmc_flash_gpt, it is not
 * necessary to check for errors.
 * If any of its intermediate commands fails, then next commands
 * will not execute. Script will exit from the failure command.
 * For uniformity, checking for mmc_flash_gpt, mmc_flash_image_rsa
 * mmc_flash_nitro and mmc_flash_rootfs
 */
#define MMC_FLASH \
	"flash_mmc="\
	"if run mmc_flash_gpt; then "\
	"else "\
		"echo [flash_mmc] run mmc_flash_gpt ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if setenv tftp_fit_image no; then "\
	"else "\
		"echo [flash_mmc] setenv tftp_fit_image to no "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if setenv fit_image Image_rsa.img; then "\
	"else "\
		"echo [flash_mmc] setenv fit_image to Image_rsa.img "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run mmc_flash_image_rsa; then "\
	"else "\
		"echo [flash_mmc] run mmc_flash_image_rsa ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if setenv fit_image Image1_rsa.img; then "\
	"else "\
		"echo [flash_mmc] setenv fit_image to Image1_rsa.img "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run mmc_flash_image_rsa; then "\
	"else "\
		"echo [flash_mmc] run mmc_flash_image_rsa "\
		"for Image1_rsa.img ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if setenv fit_image Image2_rsa.img; then "\
	"else "\
		"echo [flash_mmc] setenv fit_image to Image2_rsa.img "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run mmc_flash_image_rsa; then "\
	"else "\
		"echo [flash_mmc] run mmc_flash_image_rsa "\
		"for Image2_rsa.img ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run mmc_flash_recovery; then "\
	"else "\
		"echo [flash_mmc] run mmc_flash_recovery ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run mmc_flash_rootfs; then "\
	"else "\
		"echo [flash_mmc] run mmc_flash_rootfs ** FAILED **;"\
		"exit;"\
	"fi \0"

#define FUNC_ALIGN_QSPI_ERASE_BLOCK_SIZE \
	"align_erase_blk_size=" \
	"setenv fl_write_size 0;" \
	"if math add fl_write_size filesize FFFF; then "\
	"else "\
		"echo ${errstr} math add command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if math div fl_write_size fl_write_size 10000; then "\
	"else "\
		"echo ${errstr} math div command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if math mul fl_write_size fl_write_size 10000; then "\
	"else "\
		"echo ${errstr} math mul command ** FAILED **;"\
		"exit;"\
	"fi \0"

#define QSPI_FLASH_FIP \
	"flash_fip="\
	"if run qspi_access_en; then "\
	"else "\
		"echo [flash_fip] run qspi_access_en ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if tftp ${loadaddr} ${tftp_dir}fip.bin; then "\
	"else "\
		"echo [flash_fip] tftp ${tftp_dir}fip.bin "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if math add tmpsize filesize FFFF; then "\
	"else "\
		"echo [flash_fip] math add command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if math div tmpsize tmpsize 10000; then "\
	"else "\
		"echo [flash_fip] math div command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if math mul tmpsize tmpsize 10000; then "\
	"else "\
		"echo [flash_fip] math mul command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf probe 0; then "\
	"else "\
		"echo [flash_fip] sf probe command ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf erase ${fip_qspi_addr} ${tmpsize}; then "\
	"else "\
		"echo [flash_fip] sf erase ${fip_qspi_addr} ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf write ${loadaddr} ${fip_qspi_addr} ${filesize}; then "\
	"else "\
		"echo [flash_fip] sf write ${fip_qspi_addr} ** FAILED **;"\
		"exit;"\
	"fi;"\
	/* Flash mirror FIP image */ \
	"if sf erase ${fip_qspi_mirror_addr} ${tmpsize}; then "\
	"else "\
		"echo [flash_fip] sf erase ${fip_qspi_mirror_addr} "\
			"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf write ${loadaddr} ${fip_qspi_mirror_addr} ${filesize}; then "\
	"else "\
		"echo [flash_fip] sf write ${fip_qspi_mirror_addr} "\
			"** FAILED **;"\
		"exit;"\
	"fi \0"

#define QSPI_FLASH_NITRO \
	"flash_nitro="\
	"run func_qspi_probe; "\
	"if tftp ${loadaddr} ${tftp_dir}${nitro_bin}; then "\
	"else "\
		"echo [flash_nitro] tftp ${tftp_dir}${nitro_bin} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"setenv errstr flash_nitro;" \
	"run align_erase_blk_size;" \
	/* Flash Nitro fw fit + configuration */ \
	"if sf erase ${spi_nitro_img_bin_start} ${fl_write_size}; then "\
	"else "\
		"echo [flash_nitro] sf erase ${spi_nitro_img_bin_start} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf write ${loadaddr} ${spi_nitro_img_bin_start}" \
		     " ${fl_write_size}; then "\
	"else "\
		"echo [flash_nitro] sf write ${spi_nitro_bin_start} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	/* Mirror of Flash Nitro fw fit + configuration */ \
	"if sf erase ${spi_nitro_img_bin_mirror_start} ${fl_write_size}; then "\
	"else "\
		"echo [flash_nitro] sf erase "\
		      "${spi_nitro_img_bin_mirror_start} "\
		"** FAILED **;"\
		"exit;"\
	"fi;"\
	"if sf write ${loadaddr} ${spi_nitro_img_bin_mirror_start}" \
		     " ${fl_write_size}; then "\
	"else "\
		"echo [flash_nitro] sf write "\
		      "${spi_nitro_img_bin_mirror_start} "\
		"** FAILED **;"\
		"exit;"\
	"fi \0"

#define QSPI_FLASH_NITRO_BSPD_CONFIG \
	"flash_nitro_bspd_config="\
	"run func_qspi_probe; "\
	/* Flash BSPD configuration */ \
	"if tftp ${loadaddr} ${tftp_dir}${nitro_bspd_cfg}; then "\
		"setenv bspd_cfg_avialable 1; "\
		"setenv errstr flash_nitro_bspd_config; "\
		"run align_erase_blk_size;" \
		"if sf erase ${spi_nitro_bspd_cfg_start} "\
			    "${fl_write_size}; then "\
		"else "\
			"echo [flash_nitro] sf erase "\
				"${spi_nitro_bspd_cfg_start} "\
				"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if sf write ${loadaddr} ${spi_nitro_bspd_cfg_start} "\
			    "${fl_write_size}; then "\
		"else "\
			"echo [flash_nitro] sf write "\
				"${spi_nitro_bspd_cfg_start} "\
				"** FAILED **;"\
			"exit;"\
		"fi;" \
		/* Flash BSPD mirror configuration */ \
		"if sf erase ${spi_nitro_bspd_mirror_cfg_start} "\
			    "${fl_write_size}; then "\
		"else "\
			"echo [flash_nitro] sf erase "\
				"${spi_nitro_bspd_mirror_cfg_start} "\
				"** FAILED **;"\
			"exit;"\
		"fi;"\
		"if sf write ${loadaddr} ${spi_nitro_bspd_mirror_cfg_start} "\
			"${fl_write_size}; then "\
		"else "\
			"echo [flash_nitro] sf write "\
				"${spi_nitro_bspd_mirror_cfg_start} "\
				"** FAILED **;"\
			"exit;"\
		"fi;" \
	"else "\
		"echo [flash_nitro] tftp ${tftp_dir}${nitro_bspd_cfg} "\
		"** Skip flashing bspd config file **;"\
	"fi \0"

#define QSPI_FLASH \
	"flash_qspi="\
	"if run qspi_access_en; then "\
	"else "\
		"echo [flash_qspi] run qspi_access_en ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run flash_fip; then "\
	"else "\
		"echo [flash_qspi] run flash_fip ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run flash_nitro; then "\
	"else "\
		"echo [flash_qspi] run flash_nitro ** FAILED **;"\
		"exit;"\
	"fi \0"

#define FLASH_IMAGES \
	"flash_images=" \
	"if run flash_qspi; then "\
	"else "\
		"echo [flash_images] run flash_qspi ** FAILED **;"\
		"exit;"\
	"fi;"\
	"if run flash_mmc; then "\
	"else "\
		"echo [flash_images] run flash_mmc ** FAILED **;"\
		"exit;"\
	"fi \0"

#define ARCH_ENV_SETTINGS \
	CONSOLE_ARGS \
	MAX_CPUS \
	OS_LOG_LEVEL \
	EXTRA_ARGS \
	PCIE_ARGS \
	BMC_ETH_ADDR \
	RESERVED_MEM \
	SETBOOTARGS \
	UPDATEME_FLASH_PARAMS \
	KERNEL_LOADADDR_CFG\
	INITRD_ARGS \
	INITRD_LOADADDR \
	INITRD_IMAGE \
	MMC_DEV \
	EXEC_STATE \
	EXT4RD_ARGS \
	WDT_CNTRL \
	ELOG_SETUP \
	FIT_MMC_PARTITION \
	QSPI_FLASH_NITRO_PARAMS \
	QSPI_ACCESS_ENABLE \
	FUNC_QSPI_PROBE \
	NITRO_FW_IMAGES \
	FASTBOOT_NITRO_SETUP \
	FASTBOOT_SETUP \
	CHECK_CHIMP_HS \
	FASTBOOT_NITRO \
	FIT_IMAGE \
	BOOTCMD_MMC_FIT \
	BOOTCMD_MMC_FITS \
	USBDEV \
	BOOTCMD_USB \
	START_PCI \
	BNXT_LOAD \
	BOOTCMD_PXE \
	FLASH_PENDING_RFS_IMGS \
	TFTP_QSPI_PARAM \
	MMC_FLASH_FIT_GPT \
	MMC_FLASH_IMAGE_RSA \
	MMC_FLASH_RECOVERY \
	MMC_FLASH_ROOTFS \
	MMC_FLASH \
	FUNC_ALIGN_QSPI_ERASE_BLOCK_SIZE \
	QSPI_FLASH_FIP \
	QSPI_FLASH_NITRO \
	QSPI_FLASH_NITRO_BSPD_CONFIG \
	QSPI_FLASH \
	FLASH_IMAGES

#define CFG_EXTRA_ENV_SETTINGS \
	ARCH_ENV_SETTINGS

#endif /* __BCM_NS3_H */
