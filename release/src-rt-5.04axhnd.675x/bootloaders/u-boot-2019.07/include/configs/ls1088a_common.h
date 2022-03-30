/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2018 NXP
 */

#ifndef __LS1088_COMMON_H
#define __LS1088_COMMON_H

/* SPL build */
#ifdef CONFIG_SPL_BUILD
#define SPL_NO_BOARDINFO
#define SPL_NO_QIXIS
#define SPL_NO_PCI
#define SPL_NO_ENV
#define SPL_NO_RTC
#define SPL_NO_USB
#define SPL_NO_SATA
#define SPL_NO_QSPI
#define SPL_NO_IFC
#undef CONFIG_DISPLAY_CPUINFO
#endif

#define CONFIG_REMAKE_ELF

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

#define LS1088ARDB_PB_BOARD            0x4A
/* Link Definitions */
#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xfff0)
#endif

/* Link Definitions */
#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_FSL_QSPI_BASE	0x20000000
#else
#ifdef CONFIG_QSPI_BOOT
#define CONFIG_SYS_FSL_QSPI_BASE	0x20000000
#define CONFIG_ENV_OFFSET		0x300000        /* 3MB */
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FSL_QSPI_BASE + \
						CONFIG_ENV_OFFSET)
#endif
#endif

#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE	0x8080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	1
/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_func

#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2048 * 1024)

/* I2C */
#define CONFIG_SYS_I2C

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     1
#define CONFIG_SYS_NS16550_CLK          (get_bus_freq(0) / 2)

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#if !defined(SPL_NO_IFC) || defined(CONFIG_TARGET_LS1088AQDS)
/* IFC */
#define CONFIG_FSL_IFC
#endif

/*
 * During booting, IFC is mapped at the region of 0x30000000.
 * But this region is limited to 256MB. To accommodate NOR, promjet
 * and FPGA. This region is divided as below:
 * 0x30000000 - 0x37ffffff : 128MB : NOR flash
 * 0x38000000 - 0x3BFFFFFF : 64MB  : Promjet
 * 0x3C000000 - 0x40000000 : 64MB  : FPGA etc
 *
 * To accommodate bigger NOR flash and other devices, we will map IFC
 * chip selects to as below:
 * 0x5_1000_0000..0x5_1fff_ffff	Memory Hole
 * 0x5_2000_0000..0x5_3fff_ffff	IFC CSx (FPGA, NAND and others 512MB)
 * 0x5_4000_0000..0x5_7fff_ffff	ASIC or others 1GB
 * 0x5_8000_0000..0x5_bfff_ffff	IFC CS0 1GB (NOR/Promjet)
 * 0x5_C000_0000..0x5_ffff_ffff	IFC CS1 1GB (NOR/Promjet)
 *
 * For e.g. NOR flash at CS0 will be mapped to 0x580000000 after relocation.
 * CONFIG_SYS_FLASH_BASE has the final address (core view)
 * CONFIG_SYS_FLASH_BASE_PHYS has the final address (IFC view)
 * CONFIG_SYS_FLASH_BASE_PHYS_EARLY has the temporary IFC address
 * CONFIG_SYS_TEXT_BASE is linked to 0x30000000 for booting
 */

#define CONFIG_SYS_FLASH_BASE			0x580000000ULL
#define CONFIG_SYS_FLASH_BASE_PHYS		0x80000000
#define CONFIG_SYS_FLASH_BASE_PHYS_EARLY	0x00000000

#define CONFIG_SYS_FLASH1_BASE_PHYS		0xC0000000
#define CONFIG_SYS_FLASH1_BASE_PHYS_EARLY	0x8000000

#ifndef __ASSEMBLY__
unsigned long long get_qixis_addr(void);
#endif

#define QIXIS_BASE				get_qixis_addr()
#define QIXIS_BASE_PHYS				0x20000000
#define QIXIS_BASE_PHYS_EARLY			0xC000000


#define CONFIG_SYS_NAND_BASE			0x530000000ULL
#define CONFIG_SYS_NAND_BASE_PHYS		0x30000000


/* MC firmware */
/* TODO Actual DPL max length needs to be confirmed with the MC FW team */
#define CONFIG_SYS_LS_MC_DPC_MAX_LENGTH	    0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPC_OFFSET    0x00F00000
#define CONFIG_SYS_LS_MC_DPL_MAX_LENGTH	    0x20000
#define CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET    0x00F20000
#define CONFIG_SYS_LS_MC_AIOP_IMG_MAX_LENGTH	0x200000
#define CONFIG_SYS_LS_MC_DRAM_AIOP_IMG_OFFSET	0x07000000

/* Define phy_reset function to boot the MC based on mcinitcmd.
 * This happens late enough to properly fixup u-boot env MAC addresses.
 */
#define CONFIG_RESET_PHY_R

/*
 * Carve out a DDR region which will not be used by u-boot/Linux
 *
 * It will be used by MC and Debug Server. The MC region must be
 * 512MB aligned, so the min size to hide is 512MB.
 */

#if defined(CONFIG_FSL_MC_ENET)
#define CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE		(128UL * 1024 * 1024)
#endif
/* Command line configuration */
#define CONFIG_CMD_CACHE

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000)

/* SATA */
#ifdef CONFIG_SCSI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SATA1		AHCI_BASE_ADDR1

#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					CONFIG_SYS_SCSI_MAX_LUN)
#endif

/* Physical Memory Map */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

/* #define CONFIG_DISPLAY_CPUINFO */

#ifndef SPL_NO_ENV
/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"console=ttyAMA0,38400n8\0"		\
	"mcinitcmd=fsl_mc start mc 0x580a00000"	\
	" 0x580e00000 \0"

#ifndef CONFIG_TFABOOT
#if defined(CONFIG_QSPI_BOOT)
#define CONFIG_BOOTCOMMAND	"sf probe 0:0;" \
				"sf read 0x80001000 0xd00000 0x100000;"\
				" fsl_mc lazyapply dpl 0x80001000 &&" \
				" sf read $kernel_load $kernel_start" \
				" $kernel_size && bootm $kernel_load"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND	"mmcinfo;mmc read 0x80001000 0x6800 0x800;"\
				" fsl_mc lazyapply dpl 0x80001000 &&" \
				" mmc read $kernel_load $kernel_start" \
				" $kernel_size && bootm $kernel_load"
#else /* NOR BOOT*/
#define CONFIG_BOOTCOMMAND	"fsl_mc lazyapply dpl 0x580d00000 &&" \
				" cp.b $kernel_start $kernel_load" \
				" $kernel_size && bootm $kernel_load"
#endif
#endif /* CONFIG_TFABOOT  */
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot args buffer */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#ifdef CONFIG_SPL
#define CONFIG_SPL_BSS_START_ADDR      0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE                0x00100000
#define CONFIG_SPL_LDSCRIPT "arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_MAX_SIZE            0x16000
#define CONFIG_SPL_STACK               (CONFIG_SYS_FSL_OCRAM_BASE + 0x9ff0)
#define CONFIG_SPL_TARGET              "u-boot-with-spl.bin"

#define CONFIG_SYS_SPL_MALLOC_SIZE     0x00100000
#define CONFIG_SYS_SPL_MALLOC_START    0x80200000

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_U_BOOT_HDR_SIZE		(16 << 10)
/*
 * HDR would be appended at end of image and copied to DDR along
 * with U-Boot image. Here u-boot max. size is 512K. So if binary
 * size increases then increase this size in case of secure boot as
 * it uses raw u-boot image instead of fit image.
 */
#define CONFIG_SYS_MONITOR_LEN         (0x100000 + CONFIG_U_BOOT_HDR_SIZE)
#else
#define CONFIG_SYS_MONITOR_LEN         0x100000
#endif /* ifdef CONFIG_SECURE_BOOT */

#endif
#define CONFIG_SYS_BOOTM_LEN   (64 << 20)      /* Increase max gunzip size */

#endif /* __LS1088_COMMON_H */
