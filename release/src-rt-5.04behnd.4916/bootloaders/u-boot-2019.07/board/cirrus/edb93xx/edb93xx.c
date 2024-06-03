// SPDX-License-Identifier: GPL-2.0+
/*
 * Board initialization for EP93xx
 *
 * Copyright (C) 2013
 * Sergey Kostanbaev <sergey.kostanbaev <at> fairwaves.ru>
 *
 * Copyright (C) 2009
 * Matthias Kaehlcke <matthias <at> kaehlcke.net>
 *
 * (C) Copyright 2002 2003
 * Network Audio Technologies, Inc. <www.netaudiotech.com>
 * Adam Bezanson <bezanson <at> netaudiotech.com>
 */

#include <config.h>
#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/ep93xx.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * usb_div: 4, nbyp2: 1, pll2_en: 1
 * pll2_x1: 368640000.000000, pll2_x2ip: 15360000.000000,
 * pll2_x2: 384000000.000000, pll2_out: 192000000.000000
 */
#define CLKSET2_VAL	(23 << SYSCON_CLKSET_PLL_X2IPD_SHIFT |	\
			24 << SYSCON_CLKSET_PLL_X2FBD2_SHIFT |	\
			24 << SYSCON_CLKSET_PLL_X1FBD1_SHIFT |	\
			1 << SYSCON_CLKSET_PLL_PS_SHIFT |	\
			SYSCON_CLKSET2_PLL2_EN |		\
			SYSCON_CLKSET2_NBYP2 |			\
			3 << SYSCON_CLKSET2_USB_DIV_SHIFT)

#define SMC_BCR6_VALUE	(2 << SMC_BCR_IDCY_SHIFT | 5 << SMC_BCR_WST1_SHIFT | \
			SMC_BCR_BLE | 2 << SMC_BCR_WST2_SHIFT | \
			1 << SMC_BCR_MW_SHIFT)

/* delay execution before timers are initialized */
static inline void early_udelay(uint32_t usecs)
{
	/* loop takes 4 cycles at 5.0ns (fastest case, running at 200MHz) */
	register uint32_t loops = (usecs * 1000) / 20;

	__asm__ volatile ("1:\n"
			"subs %0, %1, #1\n"
			"bne 1b" : "=r" (loops) : "0" (loops));
}

#ifndef CONFIG_EP93XX_NO_FLASH_CFG
static void flash_cfg(void)
{
	struct smc_regs *smc = (struct smc_regs *)SMC_BASE;

	writel(SMC_BCR6_VALUE, &smc->bcr6);
}
#else
#define flash_cfg()
#endif

int board_init(void)
{
	/*
	 * Setup PLL2, PPL1 has been set during lowlevel init
	 */
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	writel(CLKSET2_VAL, &syscon->clkset2);

	/*
	 * the user's guide recommends to wait at least 1 ms for PLL2 to
	 * stabilize
	 */
	early_udelay(1000);

	/* Go to Async mode */
	__asm__ volatile ("mrc p15, 0, r0, c1, c0, 0");
	__asm__ volatile ("orr r0, r0, #0xc0000000");
	__asm__ volatile ("mcr p15, 0, r0, c1, c0, 0");

	icache_enable();

#ifdef USE_920T_MMU
	dcache_enable();
#endif

	/* Machine number, as defined in linux/arch/arm/tools/mach-types */
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* We have a console */
	gd->have_console = 1;

	enable_interrupts();

	flash_cfg();

	green_led_on();
	red_led_off();

	return 0;
}

int board_early_init_f(void)
{
	/*
	 * set UARTBAUD bit to drive UARTs with 14.7456MHz instead of
	 * 14.7456/2 MHz
	 */
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	writel(SYSCON_PWRCNT_UART_BAUD, &syscon->pwrcnt);
	return 0;
}

int board_eth_init(bd_t *bd)
{
	return ep93xx_eth_initialize(0, MAC_BASE);
}

static void dram_fill_bank_addr(unsigned dram_addr_mask, unsigned dram_bank_cnt,
				unsigned dram_bank_base[CONFIG_NR_DRAM_BANKS])
{
	if (dram_bank_cnt == 1) {
		dram_bank_base[0] = PHYS_SDRAM_1;
	} else {
		/* Table lookup for holes in address space. Maximum memory
		 * for the single SDCS may be up to 256Mb. We start scanning
		 * banks from 1Mb, so it could be up to 128 banks theoretically.
		 * We need at maximum 7 bits for the loockup, 8 slots is
		 * enough for the worst case.
		 */
		unsigned tbl[8];
		unsigned i = dram_bank_cnt / 2;
		unsigned j = 0x00100000; /* 1 Mb */
		unsigned *ptbl = tbl;
		do {
			while (!(dram_addr_mask & j)) {
				j <<= 1;
			}
			*ptbl++ = j;
			j <<= 1;
			i >>= 1;
		} while (i != 0);

		for (i = dram_bank_cnt, j = 0;
		     (i != 0) && (j < CONFIG_NR_DRAM_BANKS); --i, ++j) {
			unsigned addr = PHYS_SDRAM_1;
			unsigned k;
			unsigned bit;

			for (k = 0, bit = 1; k < 8; k++, bit <<= 1) {
				if (bit & j)
					addr |= tbl[k];
			}

			dram_bank_base[j] = addr;
		}
	}
}

/* called in board_init_f (before relocation) */
static unsigned dram_init_banksize_int(int print)
{
	/*
	 * Collect information of banks that has been filled during lowlevel
	 * initialization
	 */
	unsigned i;
	unsigned dram_bank_base[CONFIG_NR_DRAM_BANKS];
	unsigned dram_total = 0;
	unsigned dram_bank_size = *(unsigned *)
				  (PHYS_SDRAM_1 | UBOOT_MEMORYCNF_BANK_SIZE);
	unsigned dram_addr_mask = *(unsigned *)
				  (PHYS_SDRAM_1 | UBOOT_MEMORYCNF_BANK_MASK);
	unsigned dram_bank_cnt = *(unsigned *)
				 (PHYS_SDRAM_1 | UBOOT_MEMORYCNF_BANK_COUNT);

	dram_fill_bank_addr(dram_addr_mask, dram_bank_cnt, dram_bank_base);

	for (i = 0; i < dram_bank_cnt; i++) {
		gd->bd->bi_dram[i].start = dram_bank_base[i];
		gd->bd->bi_dram[i].size = dram_bank_size;
		dram_total += dram_bank_size;
	}
	for (; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = 0;
		gd->bd->bi_dram[i].size = 0;
	}

	if (print) {
		printf("DRAM mask: %08x\n", dram_addr_mask);
		printf("DRAM total %u banks:\n", dram_bank_cnt);
		printf("bank          base-address          size\n");

		if (dram_bank_cnt > CONFIG_NR_DRAM_BANKS) {
			printf("WARNING! UBoot was configured for %u banks,\n"
				"but %u has been found. "
				"Supressing extra memory banks\n",
				 CONFIG_NR_DRAM_BANKS, dram_bank_cnt);
			dram_bank_cnt = CONFIG_NR_DRAM_BANKS;
		}

		for (i = 0; i < dram_bank_cnt; i++) {
			printf("  %u             %08x            %08x\n",
			       i, dram_bank_base[i], dram_bank_size);
		}
		printf("  ------------------------------------------\n"
			"Total                              %9d\n\n",
			dram_total);
	}

	return dram_total;
}

int dram_init_banksize(void)
{
	dram_init_banksize_int(0);

	return 0;
}

/* called in board_init_f (before relocation) */
int dram_init(void)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	unsigned sec_id = readl(SECURITY_EXTENSIONID);
	unsigned chip_id = readl(&syscon->chipid);

	printf("CPU: Cirrus Logic ");
	switch (sec_id & 0x000001FE) {
	case 0x00000008:
		printf("EP9301");
		break;
	case 0x00000004:
		printf("EP9307");
		break;
	case 0x00000002:
		printf("EP931x");
		break;
	case 0x00000000:
		printf("EP9315");
		break;
	default:
		printf("<unknown>");
		break;
	}

	printf(" - Rev. ");
	switch (chip_id & 0xF0000000) {
	case 0x00000000:
		printf("A");
		break;
	case 0x10000000:
		printf("B");
		break;
	case 0x20000000:
		printf("C");
		break;
	case 0x30000000:
		printf("D0");
		break;
	case 0x40000000:
		printf("D1");
		break;
	case 0x50000000:
		printf("E0");
		break;
	case 0x60000000:
		printf("E1");
		break;
	case 0x70000000:
		printf("E2");
		break;
	default:
		printf("?");
		break;
	}
	printf(" (SecExtID=%.8x/ChipID=%.8x)\n", sec_id, chip_id);

	gd->ram_size = dram_init_banksize_int(1);
	return 0;
}
