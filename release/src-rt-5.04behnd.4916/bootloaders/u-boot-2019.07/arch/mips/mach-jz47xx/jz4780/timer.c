// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4780 timer
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <config.h>
#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <mach/jz4780.h>

#define TCU_TSR		0x1C	/* Timer Stop Register */
#define TCU_TSSR	0x2C	/* Timer Stop Set Register */
#define TCU_TSCR	0x3C	/* Timer Stop Clear Register */
#define TCU_TER		0x10	/* Timer Counter Enable Register */
#define TCU_TESR	0x14	/* Timer Counter Enable Set Register */
#define TCU_TECR	0x18	/* Timer Counter Enable Clear Register */
#define TCU_TFR		0x20	/* Timer Flag Register */
#define TCU_TFSR	0x24	/* Timer Flag Set Register */
#define TCU_TFCR	0x28	/* Timer Flag Clear Register */
#define TCU_TMR		0x30	/* Timer Mask Register */
#define TCU_TMSR	0x34	/* Timer Mask Set Register */
#define TCU_TMCR	0x38	/* Timer Mask Clear Register */
/* n = 0,1,2,3,4,5 */
#define TCU_TDFR(n)	(0x40 + (n) * 0x10)	/* Timer Data Full Reg */
#define TCU_TDHR(n)	(0x44 + (n) * 0x10)	/* Timer Data Half Reg */
#define TCU_TCNT(n)	(0x48 + (n) * 0x10)	/* Timer Counter Reg */
#define TCU_TCSR(n)	(0x4C + (n) * 0x10)	/* Timer Control Reg */

#define TCU_OSTCNTL	0xe4
#define TCU_OSTCNTH	0xe8
#define TCU_OSTCSR	0xec
#define TCU_OSTCNTHBUF	0xfc

/* Register definitions */
#define TCU_TCSR_PWM_SD		BIT(9)
#define TCU_TCSR_PWM_INITL_HIGH	BIT(8)
#define TCU_TCSR_PWM_EN		BIT(7)
#define TCU_TCSR_PRESCALE_BIT	3
#define TCU_TCSR_PRESCALE_MASK	(0x7 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE1	(0x0 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE4	(0x1 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE16	(0x2 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE64	(0x3 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE256	(0x4 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE1024	(0x5 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_EXT_EN		BIT(2)
#define TCU_TCSR_RTC_EN		BIT(1)
#define TCU_TCSR_PCK_EN		BIT(0)

#define TCU_TER_TCEN5		BIT(5)
#define TCU_TER_TCEN4		BIT(4)
#define TCU_TER_TCEN3		BIT(3)
#define TCU_TER_TCEN2		BIT(2)
#define TCU_TER_TCEN1		BIT(1)
#define TCU_TER_TCEN0		BIT(0)

#define TCU_TESR_TCST5		BIT(5)
#define TCU_TESR_TCST4		BIT(4)
#define TCU_TESR_TCST3		BIT(3)
#define TCU_TESR_TCST2		BIT(2)
#define TCU_TESR_TCST1		BIT(1)
#define TCU_TESR_TCST0		BIT(0)

#define TCU_TECR_TCCL5		BIT(5)
#define TCU_TECR_TCCL4		BIT(4)
#define TCU_TECR_TCCL3		BIT(3)
#define TCU_TECR_TCCL2		BIT(2)
#define TCU_TECR_TCCL1		BIT(1)
#define TCU_TECR_TCCL0		BIT(0)

#define TCU_TFR_HFLAG5		BIT(21)
#define TCU_TFR_HFLAG4		BIT(20)
#define TCU_TFR_HFLAG3		BIT(19)
#define TCU_TFR_HFLAG2		BIT(18)
#define TCU_TFR_HFLAG1		BIT(17)
#define TCU_TFR_HFLAG0		BIT(16)
#define TCU_TFR_FFLAG5		BIT(5)
#define TCU_TFR_FFLAG4		BIT(4)
#define TCU_TFR_FFLAG3		BIT(3)
#define TCU_TFR_FFLAG2		BIT(2)
#define TCU_TFR_FFLAG1		BIT(1)
#define TCU_TFR_FFLAG0		BIT(0)

#define TCU_TFSR_HFLAG5		BIT(21)
#define TCU_TFSR_HFLAG4		BIT(20)
#define TCU_TFSR_HFLAG3		BIT(19)
#define TCU_TFSR_HFLAG2		BIT(18)
#define TCU_TFSR_HFLAG1		BIT(17)
#define TCU_TFSR_HFLAG0		BIT(16)
#define TCU_TFSR_FFLAG5		BIT(5)
#define TCU_TFSR_FFLAG4		BIT(4)
#define TCU_TFSR_FFLAG3		BIT(3)
#define TCU_TFSR_FFLAG2		BIT(2)
#define TCU_TFSR_FFLAG1		BIT(1)
#define TCU_TFSR_FFLAG0		BIT(0)

#define TCU_TFCR_HFLAG5		BIT(21)
#define TCU_TFCR_HFLAG4		BIT(20)
#define TCU_TFCR_HFLAG3		BIT(19)
#define TCU_TFCR_HFLAG2		BIT(18)
#define TCU_TFCR_HFLAG1		BIT(17)
#define TCU_TFCR_HFLAG0		BIT(16)
#define TCU_TFCR_FFLAG5		BIT(5)
#define TCU_TFCR_FFLAG4		BIT(4)
#define TCU_TFCR_FFLAG3		BIT(3)
#define TCU_TFCR_FFLAG2		BIT(2)
#define TCU_TFCR_FFLAG1		BIT(1)
#define TCU_TFCR_FFLAG0		BIT(0)

#define TCU_TMR_HMASK5		BIT(21)
#define TCU_TMR_HMASK4		BIT(20)
#define TCU_TMR_HMASK3		BIT(19)
#define TCU_TMR_HMASK2		BIT(18)
#define TCU_TMR_HMASK1		BIT(17)
#define TCU_TMR_HMASK0		BIT(16)
#define TCU_TMR_FMASK5		BIT(5)
#define TCU_TMR_FMASK4		BIT(4)
#define TCU_TMR_FMASK3		BIT(3)
#define TCU_TMR_FMASK2		BIT(2)
#define TCU_TMR_FMASK1		BIT(1)
#define TCU_TMR_FMASK0		BIT(0)

#define TCU_TMSR_HMST5		BIT(21)
#define TCU_TMSR_HMST4		BIT(20)
#define TCU_TMSR_HMST3		BIT(19)
#define TCU_TMSR_HMST2		BIT(18)
#define TCU_TMSR_HMST1		BIT(17)
#define TCU_TMSR_HMST0		BIT(16)
#define TCU_TMSR_FMST5		BIT(5)
#define TCU_TMSR_FMST4		BIT(4)
#define TCU_TMSR_FMST3		BIT(3)
#define TCU_TMSR_FMST2		BIT(2)
#define TCU_TMSR_FMST1		BIT(1)
#define TCU_TMSR_FMST0		BIT(0)

#define TCU_TMCR_HMCL5		BIT(21)
#define TCU_TMCR_HMCL4		BIT(20)
#define TCU_TMCR_HMCL3		BIT(19)
#define TCU_TMCR_HMCL2		BIT(18)
#define TCU_TMCR_HMCL1		BIT(17)
#define TCU_TMCR_HMCL0		BIT(16)
#define TCU_TMCR_FMCL5		BIT(5)
#define TCU_TMCR_FMCL4		BIT(4)
#define TCU_TMCR_FMCL3		BIT(3)
#define TCU_TMCR_FMCL2		BIT(2)
#define TCU_TMCR_FMCL1		BIT(1)
#define TCU_TMCR_FMCL0		BIT(0)

#define TCU_TSR_WDTS		BIT(16)
#define TCU_TSR_STOP5		BIT(5)
#define TCU_TSR_STOP4		BIT(4)
#define TCU_TSR_STOP3		BIT(3)
#define TCU_TSR_STOP2		BIT(2)
#define TCU_TSR_STOP1		BIT(1)
#define TCU_TSR_STOP0		BIT(0)

#define TCU_TSSR_WDTSS		BIT(16)
#define TCU_TSSR_STPS5		BIT(5)
#define TCU_TSSR_STPS4		BIT(4)
#define TCU_TSSR_STPS3		BIT(3)
#define TCU_TSSR_STPS2		BIT(2)
#define TCU_TSSR_STPS1		BIT(1)
#define TCU_TSSR_STPS0		BIT(0)

#define TCU_TSSR_WDTSC		BIT(16)
#define TCU_TSSR_STPC5		BIT(5)
#define TCU_TSSR_STPC4		BIT(4)
#define TCU_TSSR_STPC3		BIT(3)
#define TCU_TSSR_STPC2		BIT(2)
#define TCU_TSSR_STPC1		BIT(1)
#define TCU_TSSR_STPC0		BIT(0)

#define TER_OSTEN		BIT(15)

#define OSTCSR_CNT_MD		BIT(15)
#define OSTCSR_SD		BIT(9)
#define OSTCSR_PRESCALE_16	(0x2 << 3)
#define OSTCSR_EXT_EN		BIT(2)

int timer_init(void)
{
	void __iomem *regs = (void __iomem *)TCU_BASE;

	writel(OSTCSR_SD, regs + TCU_OSTCSR);
	reset_timer();
	writel(OSTCSR_CNT_MD | OSTCSR_EXT_EN | OSTCSR_PRESCALE_16,
	       regs + TCU_OSTCSR);
	writew(TER_OSTEN, regs + TCU_TESR);
	return 0;
}

void reset_timer(void)
{
	void __iomem *regs = (void __iomem *)TCU_BASE;

	writel(0, regs + TCU_OSTCNTH);
	writel(0, regs + TCU_OSTCNTL);
}

static u64 get_timer64(void)
{
	void __iomem *regs = (void __iomem *)TCU_BASE;
	u32 low = readl(regs + TCU_OSTCNTL);
	u32 high = readl(regs + TCU_OSTCNTHBUF);

	return ((u64)high << 32) | low;
}

ulong get_timer(ulong base)
{
	return lldiv(get_timer64(), 3000) - base;
}

void __udelay(unsigned long usec)
{
	/* OST count increments at 3MHz */
	u64 end = get_timer64() + ((u64)usec * 3);

	while (get_timer64() < end)
		;
}

unsigned long long get_ticks(void)
{
	return get_timer64();
}

void jz4780_tcu_wdt_start(void)
{
	void __iomem *tcu_regs = (void __iomem *)TCU_BASE;

	/* Enable WDT clock */
	writel(TCU_TSSR_WDTSC, tcu_regs + TCU_TSCR);
}
