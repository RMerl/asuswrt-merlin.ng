/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 */

#ifndef __VEXPRESS_AEMV8A_H
#define __VEXPRESS_AEMV8A_H

#ifdef CONFIG_TARGET_VEXPRESS64_BASE_FVP
#ifndef CONFIG_SEMIHOSTING
#error CONFIG_TARGET_VEXPRESS64_BASE_FVP requires CONFIG_SEMIHOSTING
#endif
#define CONFIG_ARMV8_SWITCH_TO_EL1
#endif

#define CONFIG_REMAKE_ELF

/* Link Definitions */
#if defined(CONFIG_TARGET_VEXPRESS64_BASE_FVP) || \
	defined(CONFIG_TARGET_VEXPRESS64_BASE_FVP_DRAM)
/* ATF loads u-boot here for BASE_FVP model */
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x03f00000)
#elif CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#endif

#define CONFIG_SYS_BOOTM_LEN (64 << 20)      /* Increase max gunzip size */

/* CS register bases for the original memory map. */
#define V2M_PA_CS0			0x00000000
#define V2M_PA_CS1			0x14000000
#define V2M_PA_CS2			0x18000000
#define V2M_PA_CS3			0x1c000000
#define V2M_PA_CS4			0x0c000000
#define V2M_PA_CS5			0x10000000

#define V2M_PERIPH_OFFSET(x)		(x << 16)
#define V2M_SYSREGS			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(1))
#define V2M_SYSCTL			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(2))
#define V2M_SERIAL_BUS_PCI		(V2M_PA_CS3 + V2M_PERIPH_OFFSET(3))

#define V2M_BASE			0x80000000

/* Common peripherals relative to CS7. */
#define V2M_AACI			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(4))
#define V2M_MMCI			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(5))
#define V2M_KMI0			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(6))
#define V2M_KMI1			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(7))

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define V2M_UART0			0x7ff80000
#define V2M_UART1			0x7ff70000
#else /* Not Juno */
#define V2M_UART0			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(9))
#define V2M_UART1			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(10))
#define V2M_UART2			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(11))
#define V2M_UART3			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(12))
#endif

#define V2M_WDT				(V2M_PA_CS3 + V2M_PERIPH_OFFSET(15))

#define V2M_TIMER01			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(17))
#define V2M_TIMER23			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(18))

#define V2M_SERIAL_BUS_DVI		(V2M_PA_CS3 + V2M_PERIPH_OFFSET(22))
#define V2M_RTC				(V2M_PA_CS3 + V2M_PERIPH_OFFSET(23))

#define V2M_CF				(V2M_PA_CS3 + V2M_PERIPH_OFFSET(26))

#define V2M_CLCD			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(31))

/* System register offsets. */
#define V2M_SYS_CFGDATA			(V2M_SYSREGS + 0x0a0)
#define V2M_SYS_CFGCTRL			(V2M_SYSREGS + 0x0a4)
#define V2M_SYS_CFGSTAT			(V2M_SYSREGS + 0x0a8)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */

/* Generic Interrupt Controller Definitions */
#ifdef CONFIG_GICV3
#define GICD_BASE			(0x2f000000)
#define GICR_BASE			(0x2f100000)
#else

#if defined(CONFIG_TARGET_VEXPRESS64_BASE_FVP) || \
	defined(CONFIG_TARGET_VEXPRESS64_BASE_FVP_DRAM)
#define GICD_BASE			(0x2f000000)
#define GICC_BASE			(0x2c000000)
#elif CONFIG_TARGET_VEXPRESS64_JUNO
#define GICD_BASE			(0x2C010000)
#define GICC_BASE			(0x2C02f000)
#endif
#endif /* !CONFIG_GICV3 */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (8 << 20))

#ifndef CONFIG_TARGET_VEXPRESS64_JUNO
/* The Vexpress64 simulators use SMSC91C111 */
#define CONFIG_SMC91111			1
#define CONFIG_SMC91111_BASE		(0x01A000000)
#endif

/* PL011 Serial Configuration */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_PL011_CLOCK		7273800
#else
#define CONFIG_PL011_CLOCK		24000000
#endif

/*#define CONFIG_MENU_SHOW*/

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(V2M_BASE + 0x10000000)

/* Physical Memory Map */
#define PHYS_SDRAM_1			(V2M_BASE)	/* SDRAM Bank #1 */
/* Top 16MB reserved for secure world use */
#define DRAM_SEC_SIZE		0x01000000
#define PHYS_SDRAM_1_SIZE	0x80000000 - DRAM_SEC_SIZE
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define PHYS_SDRAM_2			(0x880000000)
#define PHYS_SDRAM_2_SIZE		0x180000000
#endif

/* Enable memtest */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)

/* Initial environment variables */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
/*
 * Defines where the kernel and FDT exist in NOR flash and where it will
 * be copied into DRAM
 */
#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=norkern\0"	\
				"kernel_alt_name=Image\0"	\
				"kernel_addr=0x80080000\0" \
				"initrd_name=ramdisk.img\0"	\
				"initrd_addr=0x84000000\0"	\
				"fdtfile=board.dtb\0" \
				"fdt_alt_name=juno\0" \
				"fdt_addr=0x83000000\0" \
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \

/* Copy the kernel and FDT to DRAM memory and boot */
#define CONFIG_BOOTCOMMAND	"afs load ${kernel_name} ${kernel_addr} ; " \
				"if test $? -eq 1; then "\
				"  echo Loading ${kernel_alt_name} instead of "\
				"${kernel_name}; "\
				"  afs load ${kernel_alt_name} ${kernel_addr};"\
				"fi ; "\
				"afs load  ${fdtfile} ${fdt_addr} ; " \
				"if test $? -eq 1; then "\
				"  echo Loading ${fdt_alt_name} instead of "\
				"${fdtfile}; "\
				"  afs load ${fdt_alt_name} ${fdt_addr}; "\
				"fi ; "\
				"fdt addr ${fdt_addr}; fdt resize; " \
				"if afs load  ${initrd_name} ${initrd_addr} ; "\
				"then "\
				"  setenv initrd_param ${initrd_addr}; "\
				"  else setenv initrd_param -; "\
				"fi ; " \
				"booti ${kernel_addr} ${initrd_param} ${fdt_addr}"


#elif CONFIG_TARGET_VEXPRESS64_BASE_FVP
#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=Image\0"		\
				"kernel_addr=0x80080000\0"	\
				"initrd_name=ramdisk.img\0"	\
				"initrd_addr=0x88000000\0"	\
				"fdtfile=devtree.dtb\0"		\
				"fdt_addr=0x83000000\0"		\
				"fdt_high=0xffffffffffffffff\0"	\
				"initrd_high=0xffffffffffffffff\0"

#define CONFIG_BOOTCOMMAND	"smhload ${kernel_name} ${kernel_addr}; " \
				"smhload ${fdtfile} ${fdt_addr}; " \
				"smhload ${initrd_name} ${initrd_addr} "\
				"initrd_end; " \
				"fdt addr ${fdt_addr}; fdt resize; " \
				"fdt chosen ${initrd_addr} ${initrd_end}; " \
				"booti $kernel_addr - $fdt_addr"


#elif CONFIG_TARGET_VEXPRESS64_BASE_FVP_DRAM
#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_addr=0x80080000\0"	\
				"initrd_addr=0x84000000\0"	\
				"fdt_addr=0x83000000\0"		\
				"fdt_high=0xffffffffffffffff\0"	\
				"initrd_high=0xffffffffffffffff\0"

#define CONFIG_BOOTCOMMAND	"booti $kernel_addr $initrd_addr $fdt_addr"


#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CONFIG_SYS_FLASH_BASE		0x08000000
/* 255 x 256KiB sectors + 4 x 64KiB sectors at the end = 259 */
#define CONFIG_SYS_MAX_FLASH_SECT	259
/* Store environment at top of flash in the same location as blank.img */
/* in the Juno firmware. */
#define CONFIG_ENV_ADDR			0x0BFC0000
#define CONFIG_ENV_SECT_SIZE		0x00010000
#else
#define CONFIG_SYS_FLASH_BASE		0x0C000000
/* 256 x 256KiB sectors */
#define CONFIG_SYS_MAX_FLASH_SECT	256
/* Store environment at top of flash */
#define CONFIG_ENV_ADDR			0x0FFC0000
#define CONFIG_ENV_SECT_SIZE		0x00040000
#endif

#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_32BIT
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */
#define FLASH_MAX_SECTOR_SIZE		0x00040000
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE

#endif /* __VEXPRESS_AEMV8A_H */
