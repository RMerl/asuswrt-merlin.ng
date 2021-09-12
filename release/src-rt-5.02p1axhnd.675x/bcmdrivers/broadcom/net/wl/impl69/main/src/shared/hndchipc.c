/*
 * Support code for chipcommon facilities (uart, jtagm) - OS independent.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndchipc.c 689775 2017-03-13 12:37:05Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <hndcpu.h>
#include <hndjtagdefs.h>

/* debug/trace */
#ifdef BCMDBG_ERR
#define	CC_ERROR(args)	printf args
#else
#define	CC_ERROR(args)
#endif	/* BCMDBG_ERR */

#ifdef BCMDBG
#define	CC_MSG(args)	printf args
#else
#define	CC_MSG(args)
#endif	/* BCMDBG */

/*
 * Initializes UART access. The callback function will be called once
 * per found UART.
 */
void
BCMATTACHFN(si_serial_init)(si_t *sih, si_serial_init_fn add)
{
	osl_t *osh;
	void *regs;
	chipcregs_t *cc;
	uint32 rev, cap, pll, baud_base, div;
	uint irq;
	int i, n;

	osh = si_osh(sih);

	cc = (chipcregs_t *)si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc);

	/* Determine core revision and capabilities */
	rev = sih->ccrev;
	cap = sih->cccaps;
	pll = cap & CC_CAP_PLL_MASK;

	/* Determine IRQ */
	irq = si_irq(sih);

	if (CCPLL_ENAB(sih) && pll == PLL_TYPE1) {
		/* PLL clock */
		baud_base = si_clock_rate(pll,
		                          R_REG(osh, &cc->clockcontrol_n),
		                          R_REG(osh, &cc->clockcontrol_m2));
		div = 1;
	} else {
		/* Fixed ALP clock */
		if (rev >= 11 && rev != 15) {
			baud_base = si_alp_clock(sih);
			div = 1;
			/* Turn off UART clock before switching clock source */
			if (rev >= 21)
				AND_REG(osh, &cc->corecontrol, ~CC_UARTCLKEN);
			/* Set the override bit so we don't divide it */
			OR_REG(osh, &cc->corecontrol, CC_UARTCLKO);
			if (rev >= 21)
				OR_REG(osh, &cc->corecontrol, CC_UARTCLKEN);
		} else if (rev >= 3) {
			/* Internal backplane clock */
			baud_base = si_clock(sih);
			div = 2;	/* Minimum divisor */
			W_REG(osh, &cc->clkdiv,
			      ((R_REG(osh, &cc->clkdiv) & ~CLKD_UART) | div));
		} else {
			/* Fixed internal backplane clock */
			baud_base = 88000000;
			div = 48;
		}

		/* Clock source depends on strapping if UartClkOverride is unset */
		if ((R_REG(osh, &cc->corecontrol) & CC_UARTCLKO) == 0) {
			if ((cap & CC_CAP_UCLKSEL) == CC_CAP_UINTCLK) {
				/* Internal divided backplane clock */
				baud_base /= div;
			} else {
				/* Assume external clock of 1.8432 MHz */
				baud_base = 1843200;
			}
		}
	}

	/* Add internal UARTs */
	n = cap & CC_CAP_UARTS_MASK;
	for (i = 0; i < n; i++) {
		regs = (void *)((ulong) &cc->uart0data + (i * 256));
		if (add != NULL) {
#ifdef RTE_UART
			add(sih, regs, irq, baud_base, 0);
#else
			add(regs, irq, baud_base, 0);
#endif // endif
		}
	}
}

#define JTAG_RETRIES	10000

/*
 * Initialize jtag master and return handle for
 * jtag_rwreg. Returns NULL on failure.
 */
volatile void *
hnd_jtagm_init(si_t *sih, uint clkd, bool exttap)
{
	volatile void *regs;

	if ((regs = si_setcoreidx(sih, SI_CC_IDX)) != NULL) {
		chipcregs_t *cc = (chipcregs_t *) regs;
		uint32 tmp;

		/*
		 * Determine jtagm availability from
		 * core revision and capabilities.
		 */

		/*
		 * Corerev 10 has jtagm, but the only chip
		 * with it does not have a mips, and
		 * the layout of the jtagcmd register is
		 * different. We'll only accept >= 11.
		 */
		if (sih->ccrev < 11)
			return (NULL);

		if ((sih->cccaps & CC_CAP_JTAGP) == 0)
			return (NULL);

		/* Set clock divider if requested */
		if (clkd != 0) {
			tmp = R_REG(NULL, &cc->clkdiv);
			tmp = (tmp & ~CLKD_JTAG) |
				((clkd << CLKD_JTAG_SHIFT) & CLKD_JTAG);
			W_REG(NULL, &cc->clkdiv, tmp);
		}

		/* Enable jtagm */
		tmp = JCTRL_EN | (exttap ? JCTRL_EXT_EN : 0);
		W_REG(NULL, &cc->jtagctrl, tmp);
	}

	return (regs);
}

void
hnd_jtagm_disable(si_t *sih, volatile void *h)
{
	chipcregs_t *cc = (chipcregs_t *)h;

	W_REG(NULL, &cc->jtagctrl, R_REG(NULL, &cc->jtagctrl) & ~JCTRL_EN);
}

static uint32
jtm_wait(chipcregs_t *cc, bool readdr)
{
	uint i;

	i = 0;
	while (((R_REG(NULL, &cc->jtagcmd) & JCMD_BUSY) == JCMD_BUSY) &&
	       (i < JTAG_RETRIES)) {
		i++;
	}

	if (i >= JTAG_RETRIES)
		return 0xbadbad03;

	if (readdr)
		return R_REG(NULL, &cc->jtagdr);
	else
		return 0xffffffff;
}

/* Read/write a jtag register. Assumes both ir and dr <= 64bits. */

uint32
jtag_scan(si_t *sih, volatile void *h, uint irsz, uint32 ir0, uint32 ir1,
          uint drsz, uint32 dr0, uint32 *dr1, bool rti)
{
	chipcregs_t *cc = (chipcregs_t *) h;
	uint32 acc_dr, acc_irdr;
	uint32 tmp;

	if ((irsz > 64) || (drsz > 64)) {
		return 0xbadbad00;
	}
	if (rti) {
		if (sih->ccrev < 28)
			return 0xbadbad01;
		acc_irdr = JCMD_ACC_IRDR_I;
		acc_dr = JCMD_ACC_DR_I;
	} else {
		acc_irdr = JCMD_ACC_IRDR;
		acc_dr = JCMD_ACC_DR;
	}
	if (irsz == 0) {
		/* scan in the first (or only) DR word with a dr-only command */
		W_REG(NULL, &cc->jtagdr, dr0);
		if (drsz > 32) {
			W_REG(NULL, &cc->jtagcmd, JCMD_START | JCMD_ACC_PDR | 31);
			drsz -= 32;
		} else
			W_REG(NULL, &cc->jtagcmd, JCMD_START | acc_dr | (drsz - 1));
	} else {
		W_REG(NULL, &cc->jtagir, ir0);
		if (irsz > 32) {
			/* Use Partial IR for first IR word */
			W_REG(NULL, &cc->jtagcmd, JCMD_START | JCMD_ACC_PIR |
			      (31 << JCMD_IRW_SHIFT));
			jtm_wait(cc, FALSE);
			W_REG(NULL, &cc->jtagir, ir1);
			irsz -= 32;
		}
		if (drsz == 0) {
			/* If drsz is 0, do an IR-only scan and that's it */
			W_REG(NULL, &cc->jtagcmd, JCMD_START | JCMD_ACC_IR |
			      ((irsz - 1) << JCMD_IRW_SHIFT));
			return jtm_wait(cc, FALSE);
		}
		/* Now scan in the IR word and the first (or only) DR word */
		W_REG(NULL, &cc->jtagdr, dr0);
		if (drsz <= 32)
			W_REG(NULL, &cc->jtagcmd, JCMD_START | acc_irdr |
			      ((irsz - 1) << JCMD_IRW_SHIFT) | (drsz - 1));
		else
			W_REG(NULL, &cc->jtagcmd, JCMD_START | JCMD_ACC_IRPDR |
			      ((irsz - 1) << JCMD_IRW_SHIFT) | 31);
	}
	/* Now scan out the DR and scan in & out the second DR word if needed */
	tmp = jtm_wait(cc, TRUE);
	if (drsz > 32) {
		if (dr1 == NULL)
			return 0xbadbad04;
		W_REG(NULL, &cc->jtagdr, *dr1);
		W_REG(NULL, &cc->jtagcmd, JCMD_START | acc_dr | (drsz - 33));
		*dr1 = jtm_wait(cc, TRUE);
	}
	return (tmp);
}

uint32
jtag_read_128(si_t *sih, volatile void *h, uint irsz, uint32 ir0, uint drsz,
	uint32 dr0, uint32 *dr1, uint32 *dr2, uint32 *dr3)
{
	chipcregs_t *cc = (chipcregs_t *) h;
	uint32 tmp;
	BCM_REFERENCE(tmp);

	if ((irsz != 128) || (drsz != 128)) {
		return 0xbadbad00;
	}

	/* Write the user reg address bit 31:0 */
	W_REG(NULL, &cc->jtagir, ir0);
	/* Write jtag cmd */
	W_REG(NULL, &cc->jtagcmd, JCMD_START | JCMD_ACC_PIR |
	      (31 << JCMD_IRW_SHIFT));
	tmp = jtm_wait(cc, FALSE);
	/* write user reg address bit 37:32 */
	W_REG(NULL, &cc->jtagir, 0x3f);

	/* Read Word 0 */
	W_REG(NULL, &cc->jtagdr, 0x0);
	W_REG(NULL, &cc->jtagcmd, 0x8004051f);
	dr0 = jtm_wait(cc, TRUE);

	/* Read Word 1 */
	W_REG(NULL, &cc->jtagdr, 0x0);
	W_REG(NULL, &cc->jtagcmd, 0x8005251f);
	*dr1 = jtm_wait(cc, TRUE);

	/* Read Word 2 */
	W_REG(NULL, &cc->jtagdr, 0x0);
	W_REG(NULL, &cc->jtagcmd, 0x8005251f);
	*dr2 = jtm_wait(cc, TRUE);

	/* Read Word 3 */
	W_REG(NULL, &cc->jtagdr, 0x0);
	W_REG(NULL, &cc->jtagcmd, 0x8001251f);
	*dr3 = jtm_wait(cc, TRUE);

	return (dr0);
}

uint32
jtag_write_128(si_t *sih, volatile void *h, uint irsz, uint32 ir0, uint drsz,
	uint32 dr0, uint32 *dr1, uint32 *dr2, uint32 *dr3)
{
	chipcregs_t *cc = (chipcregs_t *) h;
	uint32 tmp;
	BCM_REFERENCE(tmp);

	if ((irsz != 128) || (drsz != 128)) {
		return 0xbadbad00;
	}
	BCM_REFERENCE(cc);

	/* Write the user reg address */
	W_REG(NULL, &cc->jtagir, ir0);
	/* Write jtag cmd */
	W_REG(NULL, &cc->jtagcmd, 0x80061f00);
	tmp = jtm_wait(cc, FALSE);

	W_REG(NULL, &cc->jtagir, 0x3f);

	/* Write Word 0 */
	W_REG(NULL, &cc->jtagdr, dr0);
	W_REG(NULL, &cc->jtagcmd, 0x8004051f);
	tmp = jtm_wait(cc, FALSE);

	/* Write Word 1 */
	W_REG(NULL, &cc->jtagdr, *dr1);
	W_REG(NULL, &cc->jtagcmd, 0x8005251f);
	tmp = jtm_wait(cc, FALSE);

	/* Write Word 2 */
	W_REG(NULL, &cc->jtagdr, *dr2);
	W_REG(NULL, &cc->jtagcmd, 0x8005251f);
	tmp = jtm_wait(cc, FALSE);

	/* Write Word 3 */
	W_REG(NULL, &cc->jtagdr, *dr3);
	W_REG(NULL, &cc->jtagcmd, 0x8001251f);
	tmp = jtm_wait(cc, FALSE);

	return (tmp);
}

int
jtag_setbit_128(si_t *sih, uint32 jtagureg_addr, uint8 bit_pos, uint8 bit_val)
{
	volatile void *jh;
	int savecidx, ret;
	uint32 dr[4] = {0};

	/* hnd_jtagm_init does a setcore to chipc */
	savecidx = si_coreidx(sih);

	if ((jh = hnd_jtagm_init(sih, 0, FALSE)) != NULL) {
		dr[0] = jtag_read_128(sih, jh, LV_IR_SIZE_128, LV_38_UREG_ROIR(jtagureg_addr),
			LV_DR_SIZE_128, dr[0], &dr[1], &dr[2], &dr[3]);

		if (bit_val) {
			setbit((uint8 *)dr, bit_pos);
		} else {
			clrbit((uint8 *)dr, bit_pos);
		}

		jtag_write_128(sih, jh, LV_IR_SIZE_128, LV_38_UREG_IR(jtagureg_addr),
			LV_DR_SIZE_128, dr[0], &dr[1], &dr[2], &dr[3]);

		hnd_jtagm_disable(sih, jh);
		ret = BCME_OK;
	} else {
		ret = BCME_ERROR;
	}

	si_setcoreidx(sih, savecidx);

	return ret;

}
