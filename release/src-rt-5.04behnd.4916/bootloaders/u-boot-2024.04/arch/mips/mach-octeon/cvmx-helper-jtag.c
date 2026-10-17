// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper utilities for qlm_jtag.
 */

#include <log.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-ciu-defs.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * Initialize the internal QLM JTAG logic to allow programming
 * of the JTAG chain by the cvmx_helper_qlm_jtag_*() functions.
 * These functions should only be used at the direction of Cavium
 * Networks. Programming incorrect values into the JTAG chain
 * can cause chip damage.
 */
void cvmx_helper_qlm_jtag_init(void)
{
	union cvmx_ciu_qlm_jtgc jtgc;
	int clock_div = 0;
	int divisor;

	divisor = gd->bus_clk / (1000000 * (OCTEON_IS_MODEL(OCTEON_CN68XX) ? 10 : 25));

	divisor = (divisor - 1) >> 2;
	/* Convert the divisor into a power of 2 shift */
	while (divisor) {
		clock_div++;
		divisor >>= 1;
	}

	/*
	 * Clock divider for QLM JTAG operations.  sclk is divided by
	 * 2^(CLK_DIV + 2)
	 */
	jtgc.u64 = 0;
	jtgc.s.clk_div = clock_div;
	jtgc.s.mux_sel = 0;
	if (OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX))
		jtgc.s.bypass = 0x7;
	else
		jtgc.s.bypass = 0xf;
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		jtgc.s.bypass_ext = 1;
	csr_wr(CVMX_CIU_QLM_JTGC, jtgc.u64);
	csr_rd(CVMX_CIU_QLM_JTGC);
}

/**
 * Write up to 32bits into the QLM jtag chain. Bits are shifted
 * into the MSB and out the LSB, so you should shift in the low
 * order bits followed by the high order bits. The JTAG chain for
 * CN52XX and CN56XX is 4 * 268 bits long, or 1072. The JTAG chain
 * for CN63XX is 4 * 300 bits long, or 1200.
 *
 * @param qlm    QLM to shift value into
 * @param bits   Number of bits to shift in (1-32).
 * @param data   Data to shift in. Bit 0 enters the chain first, followed by
 *               bit 1, etc.
 *
 * Return: The low order bits of the JTAG chain that shifted out of the
 *         circle.
 */
uint32_t cvmx_helper_qlm_jtag_shift(int qlm, int bits, uint32_t data)
{
	union cvmx_ciu_qlm_jtgc jtgc;
	union cvmx_ciu_qlm_jtgd jtgd;

	jtgc.u64 = csr_rd(CVMX_CIU_QLM_JTGC);
	jtgc.s.mux_sel = qlm;
	csr_wr(CVMX_CIU_QLM_JTGC, jtgc.u64);
	csr_rd(CVMX_CIU_QLM_JTGC);

	jtgd.u64 = 0;
	jtgd.s.shift = 1;
	jtgd.s.shft_cnt = bits - 1;
	jtgd.s.shft_reg = data;
	jtgd.s.select = 1 << qlm;
	csr_wr(CVMX_CIU_QLM_JTGD, jtgd.u64);
	do {
		jtgd.u64 = csr_rd(CVMX_CIU_QLM_JTGD);
	} while (jtgd.s.shift);
	return jtgd.s.shft_reg >> (32 - bits);
}

/**
 * Shift long sequences of zeros into the QLM JTAG chain. It is
 * common to need to shift more than 32 bits of zeros into the
 * chain. This function is a convience wrapper around
 * cvmx_helper_qlm_jtag_shift() to shift more than 32 bits of
 * zeros at a time.
 *
 * @param qlm    QLM to shift zeros into
 * @param bits
 */
void cvmx_helper_qlm_jtag_shift_zeros(int qlm, int bits)
{
	while (bits > 0) {
		int n = bits;

		if (n > 32)
			n = 32;
		cvmx_helper_qlm_jtag_shift(qlm, n, 0);
		bits -= n;
	}
}

/**
 * Program the QLM JTAG chain into all lanes of the QLM. You must
 * have already shifted in the proper number of bits into the
 * JTAG chain. Updating invalid values can possibly cause chip damage.
 *
 * @param qlm    QLM to program
 */
void cvmx_helper_qlm_jtag_update(int qlm)
{
	union cvmx_ciu_qlm_jtgc jtgc;
	union cvmx_ciu_qlm_jtgd jtgd;

	jtgc.u64 = csr_rd(CVMX_CIU_QLM_JTGC);
	jtgc.s.mux_sel = qlm;

	csr_wr(CVMX_CIU_QLM_JTGC, jtgc.u64);
	csr_rd(CVMX_CIU_QLM_JTGC);

	/* Update the new data */
	jtgd.u64 = 0;
	jtgd.s.update = 1;
	jtgd.s.select = 1 << qlm;
	csr_wr(CVMX_CIU_QLM_JTGD, jtgd.u64);
	do {
		jtgd.u64 = csr_rd(CVMX_CIU_QLM_JTGD);
	} while (jtgd.s.update);
}

/**
 * Load the QLM JTAG chain with data from all lanes of the QLM.
 *
 * @param qlm    QLM to program
 */
void cvmx_helper_qlm_jtag_capture(int qlm)
{
	union cvmx_ciu_qlm_jtgc jtgc;
	union cvmx_ciu_qlm_jtgd jtgd;

	jtgc.u64 = csr_rd(CVMX_CIU_QLM_JTGC);
	jtgc.s.mux_sel = qlm;

	csr_wr(CVMX_CIU_QLM_JTGC, jtgc.u64);
	csr_rd(CVMX_CIU_QLM_JTGC);

	jtgd.u64 = 0;
	jtgd.s.capture = 1;
	jtgd.s.select = 1 << qlm;
	csr_wr(CVMX_CIU_QLM_JTGD, jtgd.u64);
	do {
		jtgd.u64 = csr_rd(CVMX_CIU_QLM_JTGD);
	} while (jtgd.s.capture);
}
