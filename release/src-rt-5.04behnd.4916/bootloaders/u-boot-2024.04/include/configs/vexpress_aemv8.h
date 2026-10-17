/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 */

#ifndef __VEXPRESS_AEMV8_H
#define __VEXPRESS_AEMV8_H

#include <linux/stringify.h>

/* Link Definitions */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#else
/* ATF loads u-boot here for BASE_FVP model */
#endif

/* CS register bases for the original memory map. */
#ifdef CONFIG_TARGET_VEXPRESS64_BASER_FVP
#define V2M_DRAM_BASE			0x00000000
#define V2M_PA_BASE			0x80000000
#else
#define V2M_DRAM_BASE			0x80000000
#define V2M_PA_BASE			0x00000000
#endif

#define V2M_PA_CS0			(V2M_PA_BASE + 0x00000000)
#define V2M_PA_CS1			(V2M_PA_BASE + 0x14000000)
#define V2M_PA_CS2			(V2M_PA_BASE + 0x18000000)
#define V2M_PA_CS3			(V2M_PA_BASE + 0x1c000000)
#define V2M_PA_CS4			(V2M_PA_BASE + 0x0c000000)
#define V2M_PA_CS5			(V2M_PA_BASE + 0x10000000)

#define V2M_PERIPH_OFFSET(x)		(x << 16)
#define V2M_SYSREGS			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(1))
#define V2M_SYSCTL			(V2M_PA_CS3 + V2M_PERIPH_OFFSET(2))
#define V2M_SERIAL_BUS_PCI		(V2M_PA_CS3 + V2M_PERIPH_OFFSET(3))

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

/* Generic Interrupt Controller Definitions */
#ifdef CONFIG_GICV3
#define GICD_BASE			(V2M_PA_BASE + 0x2f000000)
#define GICR_BASE			(V2M_PA_BASE + 0x2f100000)
#else

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define GICD_BASE			(0x2C010000)
#define GICC_BASE			(0x2C02f000)
#else
#define GICD_BASE			(V2M_PA_BASE + 0x2f000000)
#define GICC_BASE			(V2M_PA_BASE + 0x2c000000)
#endif
#endif /* !CONFIG_GICV3 */

/* PL011 Serial Configuration */
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CFG_PL011_CLOCK		7372800
#else
#define CFG_PL011_CLOCK		24000000
#endif

/* Physical Memory Map */
#define PHYS_SDRAM_1			(V2M_DRAM_BASE)	/* SDRAM Bank #1 */
/* Top 16MB reserved for secure world use */
#define DRAM_SEC_SIZE		0x01000000
#define PHYS_SDRAM_1_SIZE	0x80000000 - DRAM_SEC_SIZE
#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define PHYS_SDRAM_2			(0x880000000)
#define PHYS_SDRAM_2_SIZE		0x180000000
#elif CONFIG_NR_DRAM_BANKS == 2
#define PHYS_SDRAM_2			(0x880000000)
#define PHYS_SDRAM_2_SIZE		0x80000000
#endif

/* Copy the kernel, initrd and FDT from NOR flash to DRAM memory and boot. */
#define BOOTENV_DEV_AFS(devtypeu, devtypel, instance) \
	"bootcmd_afs="							\
		"afs load ${kernel_name} ${kernel_addr_r} ;"\
		"if test $? -eq 1; then "\
		"  echo Loading ${kernel_alt_name} instead of ${kernel_name}; "\
		"  afs load ${kernel_alt_name} ${kernel_addr_r};"\
		"fi ; "\
		"afs load ${fdtfile} ${fdt_addr_r} ;"\
		"if test $? -eq 1; then "\
		"  echo Loading ${fdt_alt_name} instead of ${fdtfile}; "\
		"  afs load ${fdt_alt_name} ${fdt_addr_r}; "\
		"fi ; "\
		"fdt addr ${fdt_addr_r}; fdt resize; " \
		"if afs load  ${ramdisk_name} ${ramdisk_addr_r} ; "\
		"then "\
		"  setenv ramdisk_param ${ramdisk_addr_r}; "\
		"else "\
		"  setenv ramdisk_param -; "\
		"fi ; " \
		"booti ${kernel_addr_r} ${ramdisk_param} ${fdt_addr_r}\0"
#define BOOTENV_DEV_NAME_AFS(devtypeu, devtypel, instance) "afs "

/* Boot by executing a U-Boot script pre-loaded into DRAM. */
#define BOOTENV_DEV_MEM(devtypeu, devtypel, instance) \
	"bootcmd_mem= " \
		"source ${scriptaddr}; " \
		"if test $? -eq 1; then " \
		"  env import -t ${scriptaddr}; " \
		"  if test -n $uenvcmd; then " \
		"    echo Running uenvcmd ...; " \
		"    run uenvcmd; " \
		"  fi; " \
		"fi\0"
#define BOOTENV_DEV_NAME_MEM(devtypeu, devtypel, instance) "mem "

#ifdef CONFIG_CMD_VIRTIO
#define FUNC_VIRTIO(func)	func(VIRTIO, virtio, 0)
#else
#define FUNC_VIRTIO(func)
#endif

#ifdef CONFIG_CMD_MMC
#define FUNC_MMC(func)	func(MMC, mmc, 0)
#else
#define FUNC_MMC(func)
#endif

/*
 * Boot by loading an Android image, or kernel, initrd and FDT through
 * semihosting into DRAM.
 */
#define BOOTENV_DEV_SMH(devtypeu, devtypel, instance) \
	"bootcmd_smh= " 						\
		"if load hostfs - ${boot_addr_r} ${boot_name}; then"		\
		"  setenv bootargs;"					\
		"  abootimg addr ${boot_addr_r};"			\
		"  abootimg get dtb --index=0 fdt_addr_r;"		\
		"  bootm ${boot_addr_r} ${boot_addr_r} ${fdt_addr_r};"	\
		"else"							\
		"  if load hostfs - ${kernel_addr_r} ${kernel_name}; then"	\
		"    setenv fdt_high 0xffffffffffffffff;"		\
		"    setenv initrd_high 0xffffffffffffffff;"		\
		"    load hostfs - ${fdt_addr_r} ${fdtfile};"			\
		"    load hostfs - ${ramdisk_addr_r} ${ramdisk_name};" \
		"    fdt addr ${fdt_addr_r};"				\
		"    fdt resize;"					\
		"    fdt chosen ${ramdisk_addr_r} ${filesize};"	\
		"    booti $kernel_addr_r - $fdt_addr_r;"		\
		"  fi;"							\
		"fi\0"
#define BOOTENV_DEV_NAME_SMH(devtypeu, devtypel, instance) "smh "

/* Boot sources for distro boot and load addresses, per board */

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO			/* Arm Juno board */

#define BOOT_TARGET_DEVICES(func)	\
	func(USB, usb, 0)		\
	func(SATA, sata, 0)		\
	func(SATA, sata, 1)		\
	FUNC_VIRTIO(func)		\
	func(PXE, pxe, na)		\
	func(DHCP, dhcp, na)		\
	func(AFS, afs, na)

#define VEXPRESS_KERNEL_ADDR		0x80080000
#define VEXPRESS_PXEFILE_ADDR		0x8fb00000
#define VEXPRESS_FDT_ADDR		0x8fc00000
#define VEXPRESS_SCRIPT_ADDR		0x8fd00000
#define VEXPRESS_RAMDISK_ADDR		0x8fe00000

#define EXTRA_ENV_NAMES							\
		"kernel_name=norkern\0"					\
		"kernel_alt_name=Image\0"				\
		"ramdisk_name=ramdisk.img\0"				\
		"fdtfile=board.dtb\0"					\
		"fdt_alt_name=juno\0"

#elif CONFIG_TARGET_VEXPRESS64_BASE_FVP			/* ARMv8-A base model */

#define BOOT_TARGET_DEVICES(func)	\
	func(SMH, smh, na)		\
	func(MEM, mem, na)		\
	FUNC_VIRTIO(func)		\
	FUNC_MMC(func)			\
	func(PXE, pxe, na)		\
	func(DHCP, dhcp, na)

#define VEXPRESS_KERNEL_ADDR		0x80080000
#define VEXPRESS_PXEFILE_ADDR		0x8fa00000
#define VEXPRESS_SCRIPT_ADDR		0x8fb00000
#define VEXPRESS_FDT_ADDR		0x8fc00000
#define VEXPRESS_BOOT_ADDR		0x8fd00000
#define VEXPRESS_RAMDISK_ADDR		0x8fe00000

#define EXTRA_ENV_NAMES							\
		"kernel_name=Image\0"					\
		"ramdisk_name=ramdisk.img\0"				\
		"fdtfile=devtree.dtb\0"					\
		"boot_name=boot.img\0"					\
		"boot_addr_r=" __stringify(VEXPRESS_BOOT_ADDR) "\0"

#elif CONFIG_TARGET_VEXPRESS64_BASER_FVP		/* ARMv8-R base model */

#define BOOT_TARGET_DEVICES(func)	\
	func(MEM, mem, na)		\
	FUNC_VIRTIO(func)		\
	func(PXE, pxe, na)		\
	func(DHCP, dhcp, na)

#define VEXPRESS_KERNEL_ADDR		0x00200000
#define VEXPRESS_PXEFILE_ADDR		0x0fb00000
#define VEXPRESS_FDT_ADDR		0x0fc00000
#define VEXPRESS_SCRIPT_ADDR		0x0fd00000
#define VEXPRESS_RAMDISK_ADDR		0x0fe00000

#define EXTRA_ENV_NAMES							\
					"kernel_name=Image\0"		\
					"ramdisk_name=ramdisk.img\0"	\
					"fdtfile=board.dtb\0"
#endif

#include <config_distro_bootcmd.h>

/* Default load addresses and names for the different payloads. */
#define CFG_EXTRA_ENV_SETTINGS	\
		"kernel_addr_r=" __stringify(VEXPRESS_KERNEL_ADDR) "\0"	       \
		"ramdisk_addr_r=" __stringify(VEXPRESS_RAMDISK_ADDR) "\0"      \
		"pxefile_addr_r=" __stringify(VEXPRESS_PXEFILE_ADDR) "\0"      \
		"fdt_addr_r=" __stringify(VEXPRESS_FDT_ADDR) "\0"	       \
		"scriptaddr=" __stringify(VEXPRESS_SCRIPT_ADDR) "\0"	       \
		EXTRA_ENV_NAMES						       \
		BOOTENV

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define CFG_SYS_FLASH_BASE		0x08000000
#else
#define CFG_SYS_FLASH_BASE		(V2M_PA_BASE + 0x0C000000)
#endif

#endif /* __VEXPRESS_AEMV8_H */
