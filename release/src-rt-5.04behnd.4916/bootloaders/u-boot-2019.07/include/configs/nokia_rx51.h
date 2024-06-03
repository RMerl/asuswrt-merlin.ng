/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011-2012
 * Pali Rohár <pali.rohar@gmail.com>
 *
 * (C) Copyright 2010
 * Alistair Buxton <a.j.buxton@gmail.com>
 *
 * Derived from Beagle Board code:
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the Nokia RX-51 aka N900.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SYS_L2CACHE_OFF		/* pretend there is no L2 CACHE */

#define CONFIG_MACH_TYPE		MACH_TYPE_NOKIA_RX51

/*
 * Nokia X-Loader loading secondary image to address 0x80400000
 * NOLO loading boot image to random place, so it doesn't really
 * matter what we set this to. We have to copy u-boot to this address
 */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>
#include <asm/arch/mem.h>
#include <linux/stringify.h>

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_SKIP_LOWLEVEL_INIT		/* X-Loader set everything up */

#define CONFIG_CMDLINE_TAG	/* enable passing kernel command line string */
#define CONFIG_INITRD_TAG			/* enable passing initrd */
#define CONFIG_REVISION_TAG			/* enable passing revision tag*/
#define CONFIG_SETUP_MEMORY_TAGS		/* enable memory tag */

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)
#define CONFIG_UBI_SIZE			(512 << 10)
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + CONFIG_UBI_SIZE + \
					(128 << 10))

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK		48000000		/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_BAUDRATE_TABLE { 4800, 9600, 19200, 38400, 57600, 115200 }

/* USB device configuration */
#define CONFIG_USB_DEVICE
#define CONFIG_USBD_VENDORID		0x0421
#define CONFIG_USBD_PRODUCTID		0x01c8
#define CONFIG_USBD_MANUFACTURER	"Nokia"
#define CONFIG_USBD_PRODUCT_NAME	"N900"

/* commands to include */

#define CONFIG_SYS_I2C

/*
 * TWL4030
 */

#define GPIO_SLIDE			71

/*
 * Board ONENAND Info.
 */

#define PART1_NAME			"bootloader"
#define PART1_SIZE			128
#define PART1_MULL			1024
#define PART1_SUFF			"k"
#define PART1_OFFS			0x00000000
#define PART1_MASK			0x00000003

#define PART2_NAME			"config"
#define PART2_SIZE			384
#define PART2_MULL			1024
#define PART2_SUFF			"k"
#define PART2_OFFS			0x00020000
#define PART2_MASK			0x00000000

#define PART3_NAME			"log"
#define PART3_SIZE			256
#define PART3_MULL			1024
#define PART3_SUFF			"k"
#define PART3_OFFS			0x00080000
#define PART3_MASK			0x00000000

#define PART4_NAME			"kernel"
#define PART4_SIZE			2
#define PART4_MULL			1024*1024
#define PART4_SUFF			"m"
#define PART4_OFFS			0x000c0000
#define PART4_MASK			0x00000000

#define PART5_NAME			"initfs"
#define PART5_SIZE			2
#define PART5_MULL			1024*1024
#define PART5_SUFF			"m"
#define PART5_OFFS			0x002c0000
#define PART5_MASK			0x00000000

#define PART6_NAME			"rootfs"
#define PART6_SIZE			257280
#define PART6_MULL			1024
#define PART6_SUFF			"k"
#define PART6_OFFS			0x004c0000
#define PART6_MASK			0x00000000

#ifdef ONENAND_SUPPORT

#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#endif

/* Watchdog support */
#define CONFIG_HW_WATCHDOG

/*
 * Framebuffer
 */
/* Video console */
#define CONFIG_VIDEO_LOGO
#define VIDEO_FB_16BPP_PIXEL_SWAP
#define VIDEO_FB_16BPP_WORD_SWAP
#define CONFIG_SPLASH_SCREEN

/* functions for cfb_console */
#define VIDEO_KBD_INIT_FCT		rx51_kp_init()
#define VIDEO_TSTC_FCT			rx51_kp_tstc
#define VIDEO_GETC_FCT			rx51_kp_getc
#ifndef __ASSEMBLY__
struct stdio_dev;
int rx51_kp_init(void);
int rx51_kp_tstc(struct stdio_dev *sdev);
int rx51_kp_getc(struct stdio_dev *sdev);
#endif

/* Environment information */
#ifdef CONFIG_MTDPARTS_DEFAULT
#define MTDPARTS "mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"
#else
#define MTDPARTS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
	MTDPARTS \
	"usbtty=cdc_acm\0" \
	"stdin=vga\0" \
	"stdout=vga\0" \
	"stderr=vga\0" \
	"setcon=setenv stdin ${con};" \
		"setenv stdout ${con};" \
		"setenv stderr ${con}\0" \
	"sercon=setenv con serial; run setcon\0" \
	"usbcon=setenv con usbtty; run setcon\0" \
	"vgacon=setenv con vga; run setcon\0" \
	"slide=gpio input " __stringify(GPIO_SLIDE) "\0" \
	"switchmmc=mmc dev ${mmcnum}\0" \
	"kernaddr=0x82008000\0" \
	"initrdaddr=0x84008000\0" \
	"scriptaddr=0x86008000\0" \
	"fileload=${mmctype}load mmc ${mmcnum}:${mmcpart} " \
		"${loadaddr} ${mmcfile}\0" \
	"kernload=setenv loadaddr ${kernaddr};" \
		"setenv mmcfile ${mmckernfile};" \
		"run fileload\0" \
	"initrdload=setenv loadaddr ${initrdaddr};" \
		"setenv mmcfile ${mmcinitrdfile};" \
		"run fileload\0" \
	"scriptload=setenv loadaddr ${scriptaddr};" \
		"setenv mmcfile ${mmcscriptfile};" \
		"run fileload\0" \
	"scriptboot=echo Running ${mmcscriptfile} from mmc " \
		"${mmcnum}:${mmcpart} ...; source ${scriptaddr}\0" \
	"kernboot=echo Booting ${mmckernfile} from mmc " \
		"${mmcnum}:${mmcpart} ...; bootm ${kernaddr}\0" \
	"kerninitrdboot=echo Booting ${mmckernfile} ${mmcinitrdfile} from mmc "\
		"${mmcnum}:${mmcpart} ...; bootm ${kernaddr} ${initrdaddr}\0" \
	"attachboot=echo Booting attached kernel image ...;" \
		"setenv setup_omap_atag 1;" \
		"bootm ${attkernaddr};" \
		"setenv setup_omap_atag\0" \
	"trymmcscriptboot=if run switchmmc; then " \
			"if run scriptload; then " \
				"run scriptboot;" \
			"fi;" \
		"fi\0" \
	"trymmckernboot=if run switchmmc; then " \
			"if run kernload; then " \
				"run kernboot;" \
			"fi;" \
		"fi\0" \
	"trymmckerninitrdboot=if run switchmmc; then " \
			"if run initrdload; then " \
				"if run kernload; then " \
					"run kerninitrdboot;" \
				"fi;" \
			"fi; " \
		"fi\0" \
	"trymmcpartboot=setenv mmcscriptfile boot.scr; run trymmcscriptboot;" \
		"setenv mmckernfile uImage; run trymmckernboot\0" \
	"trymmcallpartboot=setenv mmcpart 1; run trymmcpartboot;" \
		"setenv mmcpart 2; run trymmcpartboot;" \
		"setenv mmcpart 3; run trymmcpartboot;" \
		"setenv mmcpart 4; run trymmcpartboot\0" \
	"trymmcboot=if run switchmmc; then " \
			"setenv mmctype fat;" \
			"run trymmcallpartboot;" \
			"setenv mmctype ext2;" \
			"run trymmcallpartboot;" \
			"setenv mmctype ext4;" \
			"run trymmcallpartboot;" \
		"fi\0" \
	"emmcboot=setenv mmcnum 1; run trymmcboot\0" \
	"sdboot=setenv mmcnum 0; run trymmcboot\0" \
	"menucmd=bootmenu\0" \
	"bootmenu_0=Attached kernel=run attachboot\0" \
	"bootmenu_1=Internal eMMC=run emmcboot\0" \
	"bootmenu_2=External SD card=run sdboot\0" \
	"bootmenu_3=U-Boot boot order=boot\0" \
	"bootmenu_delay=30\0" \
	""

#define CONFIG_PREBOOT \
	"setenv mmcnum 1; setenv mmcpart 1;" \
	"setenv mmcscriptfile bootmenu.scr;" \
	"if run switchmmc; then " \
		"setenv mmcdone true;" \
		"setenv mmctype fat;" \
		"if run scriptload; then true; else " \
			"setenv mmctype ext2;" \
			"if run scriptload; then true; else " \
				"setenv mmctype ext4;" \
				"if run scriptload; then true; else " \
					"setenv mmcdone false;" \
				"fi;" \
			"fi;" \
		"fi;" \
		"if ${mmcdone}; then " \
			"run scriptboot;" \
		"fi;" \
	"fi;" \
	"if run slide; then true; else " \
		"setenv bootmenu_delay 0;" \
		"setenv bootdelay 0;" \
	"fi"

#define CONFIG_POSTBOOTMENU \
	"echo;" \
	"echo Extra commands:;" \
	"echo run sercon - Use serial port for control.;" \
	"echo run usbcon - Use usbtty for control.;" \
	"echo run vgacon - Use framebuffer/keyboard.;" \
	"echo run sdboot - Boot from SD card slot.;" \
	"echo run emmcboot - Boot internal eMMC memory.;" \
	"echo run attachboot - Boot attached kernel image.;" \
	"echo"

#define CONFIG_BOOTCOMMAND \
	"run sdboot;" \
	"run emmcboot;" \
	"run attachboot;" \
	"echo"

#define CONFIG_MENU_SHOW

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + 0x01F00000)/*31MB*/

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0

/*
 * FLASH and environment organization
 */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
			CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * Attached kernel image
 */

#define SDRAM_SIZE			0x10000000	/* 256 MB */
#define SDRAM_END			(CONFIG_SYS_SDRAM_BASE + SDRAM_SIZE)

#define IMAGE_MAXSIZE			0x1FF800	/* 2 MB - 2 kB */
#define KERNEL_OFFSET			0x40000		/* 256 kB */
#define KERNEL_MAXSIZE			(IMAGE_MAXSIZE-KERNEL_OFFSET)
#define KERNEL_ADDRESS			(SDRAM_END-KERNEL_MAXSIZE)

/* Reserve protected RAM for attached kernel */
#define CONFIG_PRAM			((KERNEL_MAXSIZE >> 10)+1)

#endif /* __CONFIG_H */
