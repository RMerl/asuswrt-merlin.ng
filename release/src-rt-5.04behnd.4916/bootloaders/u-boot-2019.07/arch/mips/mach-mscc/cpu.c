// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>

#include <asm/io.h>
#include <asm/types.h>

#include <mach/tlb.h>
#include <mach/ddr.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_SYS_SDRAM_SIZE <= SZ_64M
#define MSCC_RAM_TLB_SIZE   SZ_64M
#define MSCC_ATTRIB2   MMU_REGIO_INVAL
#elif CONFIG_SYS_SDRAM_SIZE <= SZ_128M
#define MSCC_RAM_TLB_SIZE   SZ_64M
#define MSCC_ATTRIB2   MMU_REGIO_RW
#elif CONFIG_SYS_SDRAM_SIZE <= SZ_256M
#define MSCC_RAM_TLB_SIZE   SZ_256M
#define MSCC_ATTRIB2   MMU_REGIO_INVAL
#elif CONFIG_SYS_SDRAM_SIZE <= SZ_512M
#define MSCC_RAM_TLB_SIZE   SZ_256M
#define MSCC_ATTRIB2   MMU_REGIO_RW
#else
#define MSCC_RAM_TLB_SIZE   SZ_512M
#define MSCC_ATTRIB2   MMU_REGIO_RW
#endif

/* NOTE: lowlevel_init() function does not have access to the
 * stack. Thus, all called functions must be inlined, and (any) local
 * variables must be kept in registers.
 */
void vcoreiii_tlb_init(void)
{
	register int tlbix = 0;

	/*
	 * Unlike most of the MIPS based SoCs, the IO register address
	 * are not in KSEG0. The mainline linux kernel built in legacy
	 * mode needs to access some of the registers very early in
	 * the boot and make the assumption that the bootloader has
	 * already configured them, so we have to match this
	 * expectation.
	 */
	create_tlb(tlbix++, MSCC_IO_ORIGIN1_OFFSET, SZ_16M, MMU_REGIO_RW,
		   MMU_REGIO_RW);
#ifdef CONFIG_SOC_LUTON
	create_tlb(tlbix++, MSCC_IO_ORIGIN2_OFFSET, SZ_16M, MMU_REGIO_RW,
		   MMU_REGIO_RW);
#endif

#if  CONFIG_SYS_TEXT_BASE == MSCC_FLASH_TO
	/*
	 * If U-Boot is located in NOR then we want to be able to use
	 * the data cache in order to boot in a decent duration
	 */
	create_tlb(tlbix++, MSCC_FLASH_TO, SZ_16M, MMU_REGIO_RO_C,
		   MMU_REGIO_RO_C);
	create_tlb(tlbix++, MSCC_FLASH_TO + SZ_32M, SZ_16M, MMU_REGIO_RO_C,
		   MMU_REGIO_RO_C);

	/*
	 * Using cache for RAM also helps to improve boot time. Thanks
	 * to this the time to relocate U-Boot in RAM went from 2.092
	 * secs to 0.104 secs.
	 */
	create_tlb(tlbix++, MSCC_DDR_TO, MSCC_RAM_TLB_SIZE, MMU_REGIO_RW,
		   MSCC_ATTRIB2);

	/* Enable caches by clearing the bit ERL, which is set on reset */
	write_c0_status(read_c0_status() & ~BIT(2));
#endif /* CONFIG_SYS_TEXT_BASE */
}

int mach_cpu_init(void)
{
	/* Speed up NOR flash access */
#ifdef CONFIG_SOC_LUTON
	writel(ICPU_PI_MST_CFG_TRISTATE_CTRL +
	       ICPU_PI_MST_CFG_CLK_DIV(4), BASE_CFG + ICPU_PI_MST_CFG);

	writel(ICPU_SPI_MST_CFG_FAST_READ_ENA +
	       ICPU_SPI_MST_CFG_CS_DESELECT_TIME(0x19) +
	       ICPU_SPI_MST_CFG_CLK_DIV(9), BASE_CFG + ICPU_SPI_MST_CFG);
#else
#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_SERVAL)
	writel(ICPU_SPI_MST_CFG_CS_DESELECT_TIME(0x19) +
	       ICPU_SPI_MST_CFG_CLK_DIV(9), BASE_CFG + ICPU_SPI_MST_CFG);
#endif
#if defined(CONFIG_SOC_JR2) || defined(CONFIG_SOC_SERVALT)
	writel(ICPU_SPI_MST_CFG_FAST_READ_ENA +
	       ICPU_SPI_MST_CFG_CS_DESELECT_TIME(0x19) +
	       ICPU_SPI_MST_CFG_CLK_DIV(14), BASE_CFG + ICPU_SPI_MST_CFG);
#endif
	/*
	 * Legacy and mainline linux kernel expect that the
	 * interruption map was set as it was done by redboot.
	 */
	writel(~0, BASE_CFG + ICPU_DST_INTR_MAP(0));
	writel(0, BASE_CFG + ICPU_DST_INTR_MAP(1));
	writel(0, BASE_CFG + ICPU_DST_INTR_MAP(2));
	writel(0, BASE_CFG + ICPU_DST_INTR_MAP(3));
#endif
	return 0;
}
