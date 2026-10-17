/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#ifndef __ASSEMBLY__

#include <config.h>

#include <linux/types.h>

/* Architecture-specific global data */
struct arch_global_data {
#if defined(CONFIG_FSL_ESDHC) || defined(CONFIG_FSL_ESDHC_IMX)
	u32 sdhc_clk;
#endif
#if CONFIG_IS_ENABLED(ACPI)
	ulong table_start;		/* Start address of ACPI tables */
	ulong table_end;		/* End address of ACPI tables */
	ulong table_start_high;		/* Start address of high ACPI tables */
	ulong table_end_high;		/* End address of high ACPI tables */
#endif
#if defined(CONFIG_FSL_ESDHC)
	u32 sdhc_per_clk;
#endif

#if defined(CONFIG_U_QE)
	u32 qe_clk;
	u32 brg_clk;
	uint mp_alloc_base;
	uint mp_alloc_top;
#endif /* CONFIG_U_QE */

#ifdef CONFIG_AT91FAMILY
	/* "static data" needed by at91's clock.c */
	unsigned long	cpu_clk_rate_hz;
	unsigned long	main_clk_rate_hz;
	unsigned long	mck_rate_hz;
	unsigned long	plla_rate_hz;
	unsigned long	pllb_rate_hz;
	unsigned long	at91_pllb_usb_init;
#endif
	/* "static data" needed by most of timer.c on ARM platforms */
	unsigned long timer_rate_hz;
	unsigned int tbu;
	unsigned int tbl;
	unsigned long lastinc;
	unsigned long long timer_reset_value;
#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
	unsigned long tlb_addr;
	unsigned long tlb_size;
#if defined(CONFIG_ARM64)
	unsigned long tlb_fillptr;
	unsigned long tlb_emerg;
#endif
#endif
#ifdef CFG_SYS_MEM_RESERVE_SECURE
#define MEM_RESERVE_SECURE_SECURED	0x1
#define MEM_RESERVE_SECURE_MAINTAINED	0x2
#define MEM_RESERVE_SECURE_ADDR_MASK	(~0x3)
	/*
	 * Secure memory addr
	 * This variable needs maintenance if the RAM base is not zero,
	 * or if RAM splits into non-consecutive banks. It also has a
	 * flag indicating the secure memory is marked as secure by MMU.
	 * Flags used: 0x1 secured
	 *             0x2 maintained
	 */
	phys_addr_t secure_ram;
	unsigned long tlb_allocated;
#endif
#ifdef CONFIG_RESV_RAM
	/*
	 * Reserved RAM for memory resident, eg. Management Complex (MC)
	 * driver which continues to run after U-Boot exits.
	 */
	phys_addr_t resv_ram;
#endif

#ifdef CONFIG_ARCH_OMAP2PLUS
	u32 omap_boot_device;
	u32 omap_boot_mode;
	u8 omap_ch_flags;
#endif
#if defined(CONFIG_FSL_LSCH3) && defined(CONFIG_SYS_FSL_HAS_DP_DDR)
	unsigned long mem2_clk;
#endif

#ifdef CONFIG_ARCH_IMX8
	struct udevice *scu_dev;
#endif

#ifdef CONFIG_IMX_ELE
	struct udevice *ele_dev;
	u32 soc_rev;
	u32 lifecycle;
	u32 uid[4];
#endif

#ifdef CONFIG_ARCH_IMX8ULP
	bool m33_handshake_done;
#endif
#ifdef CONFIG_SMBIOS
	ulong smbios_start;		/* Start address of SMBIOS table */
#endif
};

#include <asm-generic/global_data.h>

#if defined(__clang__) || defined(LTO_ENABLE)

#define DECLARE_GLOBAL_DATA_PTR
#define gd	get_gd()

static inline gd_t *get_gd(void)
{
	gd_t *gd_ptr;

#ifdef CONFIG_ARM64
	__asm__ volatile("mov %0, x18\n" : "=r" (gd_ptr));
#else
	__asm__ volatile("mov %0, r9\n" : "=r" (gd_ptr));
#endif

	return gd_ptr;
}

#else

#ifdef CONFIG_ARM64
#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("x18")
#else
#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("r9")
#endif
#endif

static inline void set_gd(volatile gd_t *gd_ptr)
{
#ifdef CONFIG_ARM64
	__asm__ volatile("ldr x18, %0\n" : : "m"(gd_ptr));
#elif __ARM_ARCH >= 7
	__asm__ volatile("ldr r9, %0\n" : : "m"(gd_ptr));
#else
	__asm__ volatile("mov r9, %0\n" : : "r"(gd_ptr));
#endif
}

#endif /* __ASSEMBLY__ */

#endif /* __ASM_GBL_DATA_H */
