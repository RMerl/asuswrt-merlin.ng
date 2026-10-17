/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022, Svyatoslav Ryhel <clamor95@gmail.com>.
 */

#ifndef __TRANSFORMER_COMMON_H
#define __TRANSFORMER_COMMON_H

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING		"ASUS Transformer"

#define TRANSFORMER_FLASH_UBOOT \
	"flash_uboot=echo Preparing RAM;" \
		"mw ${kernel_addr_r} 0 ${boot_block_size_r};" \
		"mw ${ramdisk_addr_r} 0 ${boot_block_size_r};" \
		"echo Reading BCT;" \
		"mmc dev 0 1;" \
		"mmc read ${kernel_addr_r} 0 ${boot_block_size};" \
		"echo Reading bootloader;" \
		"if load mmc 1:1 ${ramdisk_addr_r} ${bootloader_file};" \
		"then echo Calculating bootloader size;" \
			"size mmc 1:1 ${bootloader_file};" \
			"ebtupdate ${kernel_addr_r} ${ramdisk_addr_r} ${filesize};" \
			"echo Writing bootloader to eMMC;" \
			"mmc dev 0 1;" \
			"mmc write ${kernel_addr_r} 0 ${boot_block_size};" \
			"mmc dev 0 2;" \
			"mmc write ${ramdisk_addr_r} 0 ${boot_block_size};" \
			"echo Bootloader written successfully;" \
			"pause 'Press ANY key to reboot device...'; reset;" \
		"else echo Reading bootloader failed;" \
			"pause 'Press ANY key to return to bootmenu...'; bootmenu; fi\0"

#define TRANSFORMER_FLASH_SPI \
	"update_spi=sf probe 0:1;" \
		"echo Dumping current SPI flash content ...;" \
		"sf read ${kernel_addr_r} 0x0 ${spi_size};" \
		"if fatwrite mmc 1:1 ${kernel_addr_r} spi-flash-backup.bin ${spi_size};" \
		"then echo SPI flash content was successfully written into spi-flash-backup.bin;" \
			"echo Reading SPI flash binary;" \
			"if load mmc 1:1 ${kernel_addr_r} repart-block.bin;" \
			"then echo Writing bootloader into SPI flash;" \
				"sf probe 0:1;" \
				"sf update ${kernel_addr_r} 0x0 ${spi_size};" \
				"poweroff;" \
			"else echo Preparing RAM;" \
				"mw ${kernel_addr_r} 0 ${boot_block_size_r};" \
				"mw ${ramdisk_addr_r} 0 ${boot_block_size_r};" \
				"echo Reading BCT;" \
				"sf read ${kernel_addr_r} 0x0 ${boot_block_size_r};" \
				"echo Reading bootloader;" \
				"if load mmc 1:1 ${ramdisk_addr_r} ${bootloader_file};" \
				"then echo Calculating bootloader size;" \
					"size mmc 1:1 ${bootloader_file};" \
					"ebtupdate ${kernel_addr_r} ${ramdisk_addr_r} ${filesize};" \
					"echo Writing bootloader into SPI flash;" \
					"sf probe 0:1;" \
					"sf update ${kernel_addr_r} 0x0 ${boot_block_size_r};" \
					"sf update ${ramdisk_addr_r} ${boot_block_size_r} ${boot_block_size_r};" \
					"echo Bootloader written successfully; poweroff;" \
				"else echo Reading bootloader failed;" \
					"poweroff; fi;" \
			"fi;" \
		"else echo SPI flash backup FAILED! Aborting ...;" \
			"poweroff; fi\0"

#define TRANSFORMER_BOOTMENU \
	TRANSFORMER_FLASH_UBOOT \
	TRANSFORMER_FLASH_SPI \
	"bootmenu_0=mount internal storage=usb start && ums 0 mmc 0; bootmenu\0" \
	"bootmenu_1=mount external storage=usb start && ums 0 mmc 1; bootmenu\0" \
	"bootmenu_2=fastboot=echo Starting Fastboot protocol ...; fastboot usb 0; bootmenu\0" \
	"bootmenu_3=update bootloader=run flash_uboot\0" \
	"bootmenu_4=enter console=usb start; setenv skip_boot 1; exit\0" \
	"bootmenu_5=reboot RCM=enterrcm\0" \
	"bootmenu_6=reboot=reset\0" \
	"bootmenu_7=power off=poweroff\0" \
	"bootmenu_delay=-1\0"

#define BOARD_EXTRA_ENV_SETTINGS \
	"spi_size=0x400000\0" \
	"boot_block_size_r=0x200000\0" \
	"boot_block_size=0x1000\0" \
	"check_button=gpio input ${gpio_button}; test $? -eq 0;\0" \
	"bootloader_file=u-boot-dtb-tegra.bin\0" \
	"partitions=name=emmc,start=0,size=-,uuid=${uuid_gpt_rootfs}\0" \
	TRANSFORMER_BOOTMENU

#endif /* __CONFIG_H */
