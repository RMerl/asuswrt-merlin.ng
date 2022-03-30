/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 */

#ifndef __TAM3517_H
#define __TAM3517_H

/*
 * High Level Configuration Options
 */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10) + \
					2 * 1024 * 1024)
/*
 * DDR related
 */
#define CONFIG_SYS_CS0_SIZE		(256 * 1024 * 1024)

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000	/* 48MHz (APLL96/2) */

/*
 * select serial console configuration
 */
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
/* EHCI */
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	25

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50		/* base address */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1		/* bytes of address */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access */
							/* nand at CS0 */

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
							/* NAND devices */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */

#define CONFIG_SYS_MAXARGS		32	/* max number of command */
						/* args */
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */

/*
 * AM3517 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

/* Redundant Environment */
#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		0x180000
#define CONFIG_ENV_ADDR			0x180000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
						2 * CONFIG_SYS_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/*
 * ethernet support, EMAC
 *
 */
#define CONFIG_DRIVER_TI_EMAC_USE_RMII
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT 10

/* Defines for SPL */
#define CONFIG_SPL_CONSOLE
#define CONFIG_SPL_NAND_SOFTECC
#define CONFIG_SPL_NAND_WORKSPACE	0x8f07f000 /* below BSS */

#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC

#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SYS_SPL_MALLOC_START	0x8f000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_BSS_START_ADDR	0x8f080000 /* end of RAM */
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

/* FAT */
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"
#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"args"

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

/* NAND boot config */
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{40, 41, 42, 43, 44, 45, 46, 47,\
					 48, 49, 50, 51, 52, 53, 54, 55,\
					 56, 57, 58, 59, 60, 61, 62, 63}
#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_SW

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x80000

/* Setup MTD for NAND on the SOM */

#define	CONFIG_TAM3517_SETTINGS						\
	"netdev=eth0\0"							\
	"nandargs=setenv bootargs root=${nandroot} "			\
		"rootfstype=${nandrootfstype}\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip_sta=setenv bootargs ${bootargs} "			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addip_dyn=setenv bootargs ${bootargs} ip=dhcp\0"		\
	"addip=if test -n ${ipdyn};then run addip_dyn;"			\
		"else run addip_sta;fi\0"				\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=ttyO0,${baudrate}\0"				\
	"addmisc=setenv bootargs ${bootargs} ${misc}\0"			\
	"loadaddr=82000000\0"						\
	"kernel_addr_r=82000000\0"					\
	"hostname=" CONFIG_HOSTNAME "\0"			\
	"bootfile=" CONFIG_HOSTNAME "/uImage\0"		\
	"flash_self=run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr}\0"				\
	"nandboot=run nandargs addip addtty addmtd addmisc;"		\
		"nand read ${kernel_addr_r} kernel\0"			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"net_self=if run net_self_load;then "				\
		"run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r};"		\
		"else echo Images not loades;fi\0"			\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.img\0"		\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"loadmlo=tftp ${loadaddr} ${mlo}\0"				\
	"mlo=" CONFIG_HOSTNAME "/MLO\0"			\
	"uboot_addr=0x80000\0"						\
	"update=nandecc sw;nand erase ${uboot_addr} 100000;"		\
		"nand write ${loadaddr} ${uboot_addr} 80000\0"		\
	"updatemlo=nandecc hw;nand erase 0 20000;"			\
		"nand write ${loadaddr} 0 20000\0"			\
	"upd=if run load;then echo Updating u-boot;if run update;"	\
		"then echo U-Boot updated;"				\
			"else echo Error updating u-boot !;"		\
			"echo Board without bootloader !!;"		\
		"fi;"							\
		"else echo U-Boot not downloaded..exiting;fi\0"		\

/*
 * this is common code for all TAM3517 boards.
 * MAC address is stored from manufacturer in
 * I2C EEPROM
 */
#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
/*
 * The I2C EEPROM on the TAM3517 contains
 * mac address and production data
 */
struct tam3517_module_info {
	char customer[48];
	char product[48];

	/*
	 * bit 0~47  : sequence number
	 * bit 48~55 : week of year, from 0.
	 * bit 56~63 : year
	 */
	unsigned long long sequence_number;

	/*
	 * bit 0~7   : revision fixed
	 * bit 8~15  : revision major
	 * bit 16~31 : TNxxx
	 */
	unsigned int revision;
	unsigned char eth_addr[4][8];
	unsigned char _rev[100];
};

#define TAM3517_READ_EEPROM(info, ret) \
do {								\
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE); \
	if (eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,		\
		(void *)info, sizeof(*info)))			\
		ret = 1;					\
	else							\
		ret = 0;					\
} while (0)

#define TAM3517_READ_MAC_FROM_EEPROM(info)			\
do {								\
	char buf[80], ethname[20];				\
	int i;							\
	memset(buf, 0, sizeof(buf));				\
	for (i = 0 ; i < ARRAY_SIZE((info)->eth_addr); i++) {	\
		sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",	\
			(info)->eth_addr[i][5],			\
			(info)->eth_addr[i][4],			\
			(info)->eth_addr[i][3],			\
			(info)->eth_addr[i][2],			\
			(info)->eth_addr[i][1],			\
			(info)->eth_addr[i][0]);			\
								\
		if (i)						\
			sprintf(ethname, "eth%daddr", i);	\
		else						\
			strcpy(ethname, "ethaddr");		\
		printf("Setting %s from EEPROM with %s\n", ethname, buf);\
		env_set(ethname, buf);				\
	}							\
} while (0)

/* The following macros are taken from Technexion's documentation */
#define TAM3517_sequence_number(info) \
	((info)->sequence_number % 0x1000000000000LL)
#define TAM3517_week_of_year(info) (((info)->sequence_number >> 48) % 0x100)
#define TAM3517_year(info) ((info)->sequence_number >> 56)
#define TAM3517_revision_fixed(info) ((info)->revision % 0x100)
#define TAM3517_revision_major(info) (((info)->revision >> 8) % 0x100)
#define TAM3517_revision_tn(info) ((info)->revision >> 16)

#define TAM3517_PRINT_SOM_INFO(info)				\
do {								\
	printf("Vendor:%s\n", (info)->customer);		\
	printf("SOM:   %s\n", (info)->product);			\
	printf("SeqNr: %02llu%02llu%012llu\n",			\
		TAM3517_year(info),				\
		TAM3517_week_of_year(info),			\
		TAM3517_sequence_number(info));			\
	printf("Rev:   TN%u %u.%u\n",				\
		TAM3517_revision_tn(info),			\
		TAM3517_revision_major(info),			\
		TAM3517_revision_fixed(info));			\
} while (0)

#endif

#endif /* __TAM3517_H */
