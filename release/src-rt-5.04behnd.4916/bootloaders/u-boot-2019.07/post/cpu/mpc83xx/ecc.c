// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Eastman Kodak Company, <www.kodak.com>
 * Michael Zaidman, <michael.zaidman@kodak.com>
 *
 * The code is based on the cpu/mpc83xx/ecc.c written by
 * Dave Liu <daveliu@freescale.com>
 */

#include <common.h>
#include <mpc83xx.h>
#include <watchdog.h>
#include <asm/io.h>
#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_ECC
/*
 * We use the RAW I/O accessors where possible in order to
 * achieve performance goal, since the test's execution time
 * affects the board start up time.
 */
static inline void ecc_clear(ddr83xx_t *ddr)
{
	/* Clear capture registers */
	__raw_writel(0, &ddr->capture_address);
	__raw_writel(0, &ddr->capture_data_hi);
	__raw_writel(0, &ddr->capture_data_lo);
	__raw_writel(0, &ddr->capture_ecc);
	__raw_writel(0, &ddr->capture_attributes);

	/* Clear SBEC and set SBET to 1 */
	out_be32(&ddr->err_sbe, 1 << ECC_ERROR_MAN_SBET_SHIFT);

	/* Clear Error Detect register */
	out_be32(&ddr->err_detect, ECC_ERROR_DETECT_MME |\
			ECC_ERROR_DETECT_MBE |\
			ECC_ERROR_DETECT_SBE |\
			ECC_ERROR_DETECT_MSE);

	isync();
}

int ecc_post_test(int flags)
{
	int ret = 0;
	int int_state;
	int errbit;
	u32 pattern[2], writeback[2], retval[2];
	ddr83xx_t *ddr = &((immap_t *)CONFIG_SYS_IMMR)->ddr;
	volatile u64 *addr = (u64 *)CONFIG_SYS_POST_ECC_START_ADDR;

	/* The pattern is written into memory to generate error */
	pattern[0] = 0xfedcba98UL;
	pattern[1] = 0x76543210UL;

	/* After injecting error, re-initialize the memory with the value */
	writeback[0] = ~pattern[0];
	writeback[1] = ~pattern[1];

	/* Check if ECC is enabled */
	if (__raw_readl(&ddr->err_disable) & ECC_ERROR_ENABLE) {
		debug("DDR's ECC is not enabled, skipping the ECC POST.\n");
		return 0;
	}

	int_state = disable_interrupts();
	icache_enable();

#ifdef CONFIG_DDR_32BIT
	/* It seems like no one really uses the CONFIG_DDR_32BIT mode */
#error "Add ECC POST support for CONFIG_DDR_32BIT here!"
#else
	for (addr = (u64*)CONFIG_SYS_POST_ECC_START_ADDR, errbit=0;
	     addr < (u64*)CONFIG_SYS_POST_ECC_STOP_ADDR; addr++, errbit++ ) {

		WATCHDOG_RESET();

		ecc_clear(ddr);

		/* Enable error injection */
		setbits_be32(&ddr->ecc_err_inject, ECC_ERR_INJECT_EIEN);
		sync();
		isync();

		/* Set bit to be injected */
		if (errbit < 32) {
			__raw_writel(1 << errbit, &ddr->data_err_inject_lo);
			__raw_writel(0, &ddr->data_err_inject_hi);
		} else {
			__raw_writel(0, &ddr->data_err_inject_lo);
			__raw_writel(1<<(errbit-32), &ddr->data_err_inject_hi);
		}
		sync();
		isync();

		/* Write memory location injecting SBE */
		ppcDWstore((u32*)addr, pattern);
		sync();

		/* Disable error injection */
		clrbits_be32(&ddr->ecc_err_inject, ECC_ERR_INJECT_EIEN);
		sync();
		isync();

		/* Data read should generate SBE */
		ppcDWload((u32*)addr, retval);
		sync();

		if (!(__raw_readl(&ddr->err_detect) & ECC_ERROR_DETECT_SBE) ||
			(__raw_readl(&ddr->data_err_inject_hi) !=
			(__raw_readl(&ddr->capture_data_hi) ^ pattern[0])) ||
			(__raw_readl(&ddr->data_err_inject_lo) !=
			(__raw_readl(&ddr->capture_data_lo) ^ pattern[1]))) {

			post_log("ECC failed to detect SBE error at %08x, "
				"SBE injection mask %08x-%08x, wrote "
				"%08x-%08x, read %08x-%08x\n", addr,
				ddr->data_err_inject_hi,
				ddr->data_err_inject_lo,
				pattern[0], pattern[1],
				retval[0], retval[1]);

			printf("ERR_DETECT Reg: %08x\n", ddr->err_detect);
			printf("ECC CAPTURE_DATA Reg: %08x-%08x\n",
				ddr->capture_data_hi, ddr->capture_data_lo);
			ret = 1;
			break;
		}

		/* Re-initialize the ECC memory */
		ppcDWstore((u32*)addr, writeback);
		sync();
		isync();

		errbit %= 63;
	}
#endif /* !CONFIG_DDR_32BIT */

	ecc_clear(ddr);

	icache_disable();

	if (int_state)
		enable_interrupts();

	return ret;
}
#endif
