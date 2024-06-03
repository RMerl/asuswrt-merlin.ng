/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Gateworks Corporation
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SPL */
/* Location in NAND to read U-Boot from */
#define CONFIG_SYS_NAND_U_BOOT_OFFS     (14 * SZ_1M)

/* Falcon Mode */
#define CONFIG_SYS_SPL_ARGS_ADDR	0x18000000

/* Falcon Mode - NAND support: args@17MB kernel@18MB */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	(18 * SZ_1M)

/* Falcon Mode - MMC support: args@1MB kernel@2MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x800	/* 1MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	(CONFIG_CMD_SPL_WRITE_SIZE / 512)
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x1000	/* 2MB */

#include "imx6_spl.h"                  /* common IMX6 SPL configuration */
#include "mx6_common.h"

#define CONFIG_MACH_TYPE	4520   /* Gateworks Ventana Platform */

/* Serial ATAG */
#define CONFIG_SERIAL_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

/* Init Functions */

/* Driver Model */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_DM_GPIO
#define CONFIG_DM_THERMAL
#endif

/* Thermal */
#define CONFIG_IMX_THERMAL

/* Serial */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	       UART2_BASE

#ifdef CONFIG_SPI_FLASH

/* SPI */
#ifdef CONFIG_CMD_SF
  #define CONFIG_SPI_FLASH_MTD
					     /* GPIO 3-19 (21248) */
#endif

#elif defined(CONFIG_SPL_NAND_SUPPORT)
/* Enable NAND support */
#ifdef CONFIG_CMD_NAND
  #define CONFIG_SYS_MAX_NAND_DEVICE	1
  #define CONFIG_SYS_NAND_BASE		0x40000000
  #define CONFIG_SYS_NAND_5_ADDR_CYCLE
  #define CONFIG_SYS_NAND_ONFI_DETECTION

  /* DMA stuff, needed for GPMI/MXS NAND support */
#endif

#endif /* CONFIG_SPI_FLASH */

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_I2C_GSC			0
#define CONFIG_I2C_EDID

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      0

/*
 * SATA Configs
 */
#ifdef CONFIG_CMD_SATA
  #define CONFIG_SYS_SATA_MAX_DEVICE	1
  #define CONFIG_DWC_AHSATA_PORT_ID	0
  #define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
  #define CONFIG_LBA48
#endif

/*
 * PCI express
 */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCI_FIXUP_DEV
#define CONFIG_PCIE_IMX
#endif

/*
 * PMIC
 */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08
#define CONFIG_POWER_LTC3676
#define CONFIG_POWER_LTC3676_I2C_ADDR  0x3c

/* Various command support */
#define CONFIG_CMD_UNZIP         /* gzwrite */

/* Ethernet support */
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE             ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE      RGMII
#define CONFIG_FEC_MXC_PHYADDR   0
#define CONFIG_ARP_TIMEOUT       200UL

/* USB Configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET  /* For OTG port */
#define CONFIG_MXC_USB_PORTSC     (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS      0
#define CONFIG_USBD_HS
#define CONFIG_NETCONSOLE

/* Framebuffer and LCD */
#define CONFIG_VIDEO_LOGO
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_HIDE_LOGO_VERSION  /* Custom config to hide U-boot version */

/* Miscellaneous configurable options */
#define CONFIG_HWCONFIG
#define CONFIG_PREBOOT

/* Memory configuration */
#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END	       0x10010000
#define CONFIG_SYS_MEMTEST_SCRATCH     0x10800000

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/*
 * MTD Command for mtdparts
 */

/* Persistent Environment Config */
#if defined(CONFIG_ENV_IS_IN_MMC)
  #define CONFIG_SYS_MMC_ENV_DEV         0
  #define CONFIG_SYS_MMC_ENV_PART        1
  #define CONFIG_ENV_OFFSET              (709 * SZ_1K)
  #define CONFIG_ENV_SIZE                (128 * SZ_1K)
  #define CONFIG_ENV_OFFSET_REDUND       (CONFIG_ENV_OFFSET + (128 * SZ_1K))
#elif defined(CONFIG_ENV_IS_IN_NAND)
  #define CONFIG_ENV_OFFSET              (16 * SZ_1M)
  #define CONFIG_ENV_SECT_SIZE           (128 * SZ_1K)
  #define CONFIG_ENV_SIZE                CONFIG_ENV_SECT_SIZE
  #define CONFIG_ENV_OFFSET_REDUND       (CONFIG_ENV_OFFSET + (512 * SZ_1K))
  #define CONFIG_ENV_SIZE_REDUND         CONFIG_ENV_SIZE
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
  #define CONFIG_ENV_OFFSET		(512 * SZ_1K)
  #define CONFIG_ENV_SECT_SIZE		(64 * SZ_1K)
  #define CONFIG_ENV_SIZE		(8 * SZ_1K)
#endif

/* Environment */
#define CONFIG_IPADDR             192.168.1.1
#define CONFIG_SERVERIP           192.168.1.146

#define CONFIG_EXTRA_ENV_SETTINGS_COMMON \
	"pcidisable=1\0" \
	"splashpos=m,m\0" \
	"usb_pgood_delay=2000\0" \
	"console=ttymxc1\0" \
	"bootdevs=usb mmc sata flash\0" \
	"hwconfig=_UNKNOWN_\0" \
	"video=\0" \
	\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"disk=0\0" \
	"part=1\0" \
	\
	"fdt_high=0xffffffff\0" \
	"fdt_addr=0x18000000\0" \
	"initrd_high=0xffffffff\0" \
	"fixfdt=" \
		"fdt addr ${fdt_addr}\0" \
	"bootdir=boot\0" \
	"loadfdt=" \
		"if ${fsload} ${fdt_addr} ${bootdir}/${fdt_file}; then " \
			"echo Loaded DTB from ${bootdir}/${fdt_file}; " \
			"run fixfdt; " \
		"elif ${fsload} ${fdt_addr} ${bootdir}/${fdt_file1}; then " \
			"echo Loaded DTB from ${bootdir}/${fdt_file1}; " \
			"run fixfdt; " \
		"elif ${fsload} ${fdt_addr} ${bootdir}/${fdt_file2}; then " \
			"echo Loaded DTB from ${bootdir}/${fdt_file2}; " \
			"run fixfdt; " \
		"fi\0" \
	\
	"fs=ext4\0" \
	"script=6x_bootscript-ventana\0" \
	"loadscript=" \
		"if ${fsload} ${loadaddr} ${bootdir}/${script}; then " \
			"source ${loadaddr}; " \
		"fi\0" \
	\
	"uimage=uImage\0" \
	"mmc_root=mmcblk0p1\0" \
	"mmc_boot=" \
		"setenv fsload \"${fs}load mmc ${disk}:${part}\"; " \
		"mmc dev ${disk} && mmc rescan && " \
		"setenv dtype mmc; run loadscript; " \
		"if ${fsload} ${loadaddr} ${bootdir}/${uimage}; then " \
			"setenv bootargs console=${console},${baudrate} " \
				"root=/dev/${mmc_root} rootfstype=${fs} " \
				"rootwait rw ${video} ${extra}; " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"bootm; " \
			"fi; " \
		"fi\0" \
	\
	"sata_boot=" \
		"setenv fsload \"${fs}load sata ${disk}:${part}\"; " \
		"sata init && " \
		"setenv dtype sata; run loadscript; " \
		"if ${fsload} ${loadaddr} ${bootdir}/${uimage}; then " \
			"setenv bootargs console=${console},${baudrate} " \
				"root=/dev/sda1 rootfstype=${fs} " \
				"rootwait rw ${video} ${extra}; " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"bootm; " \
			"fi; " \
		"fi\0" \
	"usb_boot=" \
		"setenv fsload \"${fs}load usb ${disk}:${part}\"; " \
		"usb start && usb dev ${disk} && " \
		"setenv dtype usb; run loadscript; " \
		"if ${fsload} ${loadaddr} ${bootdir}/${uimage}; then " \
			"setenv bootargs console=${console},${baudrate} " \
				"root=/dev/sda1 rootfstype=${fs} " \
				"rootwait rw ${video} ${extra}; " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"bootm; " \
			"fi; " \
		"fi\0"

#ifdef CONFIG_SPI_FLASH
	#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_EXTRA_ENV_SETTINGS_COMMON \
	"image_os=ventana/openwrt-imx6-imx6q-gw5400-a-squashfs.bin\0" \
	"image_uboot=ventana/u-boot_spi.imx\0" \
	\
	"spi_koffset=0x90000\0" \
	"spi_klen=0x200000\0" \
	\
	"spi_updateuboot=echo Updating uboot from " \
		"${serverip}:${image_uboot}...; " \
		"tftpboot ${loadaddr} ${image_uboot} && " \
		"sf probe && sf erase 0 80000 && " \
			"sf write ${loadaddr} 400 ${filesize}\0" \
	"spi_update=echo Updating OS from ${serverip}:${image_os} " \
		"to ${spi_koffset} ...; " \
		"tftp ${loadaddr} ${image_os} && " \
		"sf probe && " \
		"sf update ${loadaddr} ${spi_koffset} ${filesize}\0" \
	\
	"flash_boot=" \
		"if sf probe && " \
		"sf read ${loadaddr} ${spi_koffset} ${spi_klen}; then " \
			"setenv bootargs console=${console},${baudrate} " \
				"root=/dev/mtdblock3 " \
				"rootfstype=squashfs,jffs2 " \
				"${video} ${extra}; " \
			"bootm; " \
		"fi\0"
#else
	#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_EXTRA_ENV_SETTINGS_COMMON \
	\
	"image_rootfs=openwrt-imx6-ventana-rootfs.ubi\0" \
	"nand_update=echo Updating NAND from ${serverip}:${image_rootfs}...; " \
		"tftp ${loadaddr} ${image_rootfs} && " \
		"nand erase.part rootfs && " \
		"nand write ${loadaddr} rootfs ${filesize}\0" \
	\
	"flash_boot=" \
		"setenv fsload 'ubifsload'; " \
		"ubi part rootfs; " \
		"if ubi check boot; then " \
			"ubifsmount ubi0:boot; " \
			"setenv root ubi0:rootfs ubi.mtd=2 " \
				"rootfstype=squashfs,ubifs; " \
			"setenv bootdir; " \
		"elif ubi check rootfs; then " \
			"ubifsmount ubi0:rootfs; " \
			"setenv root ubi0:rootfs ubi.mtd=2 " \
				"rootfstype=ubifs; " \
		"fi; " \
		"setenv dtype nand; run loadscript; " \
		"if ${fsload} ${loadaddr} ${bootdir}/${uimage}; then " \
			"setenv bootargs console=${console},${baudrate} " \
				"root=${root} ${video} ${extra}; " \
			"if run loadfdt; then " \
				"ubifsumount; " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"ubifsumount; bootm; " \
			"fi; " \
		"fi\0"
#endif

#define CONFIG_BOOTCOMMAND \
	"for btype in ${bootdevs}; do " \
		"echo; echo Attempting ${btype} boot...; " \
		"if run ${btype}_boot; then; fi; " \
	"done"

#endif			       /* __CONFIG_H */
