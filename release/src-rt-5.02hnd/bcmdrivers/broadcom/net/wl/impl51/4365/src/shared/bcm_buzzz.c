/*
 * +----------------------------------------------------------------------------
 *
 * BCM_BUZZZ Performance tracing tool for:
 *    ARM Cortex-R4 (HndRTE), ARM Cortex-M3 (BMOS) and ARM Cortex-A7 (ThreadX)
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id$
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * +----------------------------------------------------------------------------
 */
#include <typedefs.h>
#include <bcm_buzzz.h>
#include <osl.h>
#include <bcmpcie.h>


#if defined(BCM_BUZZZ)

#define BCM_BUZZZ_TEST

#define BCM_BUZZZ_PR(x)             printf x
#define BCM_BUZZZ_ER(x)             printf x

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CR4)
/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Performance Monitoring Unit manipulations
 * +----------------------------------------------------------------------------
 */

#if !defined(__ARM_ARCH_7R__)
#error "Mismatch ARM Arch 7R and Cortex CR4"
#endif /* ! __ARM_ARCH_7R__ */

#include <arminc.h>

#define BCM_BUZZZ_KEVT_COUNTERS     BCM_BUZZZ_COUNTERS /* 3 counters logged */
#define BCM_BUZZZ_ARM_IDCODE        (0x14)

/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Performance Monitor Control (PMCR) Register
 *  MRC p15, 0, <Rd>, c9, c12, 0 ; Read PMCR Register
 *  MCR p15, 0, <Rd>, c9, c12, 0 ; Write PMCR Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_PMCR {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 enable      : 1;    /* E: enable all counters incld cctr  */
		uint32 evt_reset   : 1;    /* P: event counter reset             */
		uint32 cctr_reset  : 1;    /* C: cycle counter reset             */
		uint32 clk_divider : 1;    /* D: cycle count divider: 0= 1:1     */
		uint32 export_en   : 1;    /* X: export enable                   */
		uint32 prohibit    : 1;    /* DP: disable in prohibited regions  */
		uint32 reserved    : 5;    /* ReadAZ, Wr: ShouldBeZero/Preserved */
		uint32 counter     : 5;    /* N: number of event counters        */
		uint32 id_code     : 8;    /* IDCODE: identification code        */
		                           /* 0x04=Cortex-R4                     */
		uint32 impl_code   : 8;    /* IMP: implementer code 0x41=ARM     */
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_PMCR_RD(void)
{
	union cp15_c9_c12_PMCR pmcr;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr.u32));
	return pmcr.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_PMCR_WR(const uint32 v32)
{
	union cp15_c9_c12_PMCR pmcr;
	pmcr.u32 = v32;
	pmcr.reserved = 0;  /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 0" : : "r"(pmcr.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Count Enable Set (CNTENS) Register
 *  MRC p15, 0, <Rd>, c9, c12, 1 ; Read CNTENS Register
 *  MCR p15, 0, <Rd>, c9, c12, 1 ; Write CNTENS Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_CNTENS {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0        : 1;    /* P0: Enable counter#0 set  */
		uint32 ctr1        : 1;    /* P1: Enable counter#1 set  */
		uint32 ctr2        : 1;    /* P2: Enable counter#2 set  */
		uint32 reserved    : 28;
		uint32 cctr        : 1;    /* C: Enable cycle count set */
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_CNTENS_RD(void)
{
	union cp15_c9_c12_CNTENS ctens;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(ctens.u32));
	return ctens.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_CNTENS_WR(const uint32 v32)
{
	union cp15_c9_c12_CNTENS ctens;
	ctens.u32 = v32;
	ctens.reserved = 0; /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 1" : : "r"(ctens.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Count Enable Clear (CNTENC) Register
 *  MRC p15, 0, <Rd>, c9, c12, 2 ; Read CNTENC Register
 *  MCR p15, 0, <Rd>, c9, c12, 2 ; Write CNTENC Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_CNTENC {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0        : 1;    /* P0: Enable counter#0 clear  */
		uint32 ctr1        : 1;    /* P1: Enable counter#1 clear  */
		uint32 ctr2        : 1;    /* P2: Enable counter#2 clear  */
		uint32 reserved    : 28;
		uint32 cctr        : 1;    /* C: Enable cycle count clear */
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_CNTENC_RD(void)
{
	union cp15_c9_c12_CNTENC ctenc;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 2" : "=r"(ctenc.u32));
	return ctenc.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_CNTENC_WR(const uint32 v32)
{
	union cp15_c9_c12_CNTENC ctenc;
	ctenc.u32 = v32;
	ctenc.reserved = 0; /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 2" : : "r"(ctenc.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Overflow Flag Status (FLAG) Register
 *  MRC p15, 0, <Rd>, c9, c12, 3 ; Read FLAG Register
 *  MCR p15, 0, <Rd>, c9, c12, 3 ; Write FLAG Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_FLAG {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0        : 1;    /* P0: Enable counter#0 overflow  */
		uint32 ctr1        : 1;    /* P1: Enable counter#1 overflow  */
		uint32 ctr2        : 1;    /* P2: Enable counter#2 overflow  */
		uint32 reserved    : 28;
		uint32 cctr        : 1;    /* C: Enable cycle count overflow */
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_FLAG_RD(void)
{
	union cp15_c9_c12_FLAG flag;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 3" : "=r"(flag.u32));
	return flag.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_FLAG_WR(const uint32 v32)
{
	union cp15_c9_c12_FLAG flag;
	flag.u32 = v32;
	flag.reserved = 0; /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 3" : : "r"(flag.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Software Increment (SWINCR) Register
 *  MRC p15, 0, <Rd>, c9, c12, 4 ; Read SWINCR Register
 *  MCR p15, 0, <Rd>, c9, c12, 4 ; Write SWINCR Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_SWINCR {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0        : 1;    /* P0: Increment counter#0  */
		uint32 ctr1        : 1;    /* P1: Increment counter#1  */
		uint32 ctr2        : 1;    /* P2: Increment counter#2  */
		uint32 reserved    : 29;
	};
};
/* may only be used with EVTSEL register = 0x0 */
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_SWINC_RD(void)
{
	union cp15_c9_c12_SWINCR swincr;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 4" : "=r"(swincr.u32));
	return swincr.u32;
}
/* may only be used with EVTSEL register = 0x0 */
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_SWINCR_WR(const uint32 v32)
{
	union cp15_c9_c12_SWINCR swincr;
	swincr.u32 = v32;
	swincr.reserved = 0;    /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 4" : : "r"(swincr.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Performance Counter Selection (PMNXSEL) Register
 *  MCR p15, 0, <Rd>, c9, c12, 5 ; Write PMNXSELRegister
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_PMNXSEL {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr_sel   : 5;      /* event counter selecter */
		uint32 reserved  :25;      /* reserved               */
	};
};
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_PMNXSEL_WR(const uint32 v32)
{
	union cp15_c9_c12_PMNXSEL pmnxsel;
	pmnxsel.u32 = v32;
	pmnxsel.reserved = 0;
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 5" : : "r"(pmnxsel.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Cycle Count (CCNT) Register
 *  MRC p15, 0, <Rd>, c9, c13, 0; Read CCNT Register
 *  MCR p15, 0, <Rd>, c9, c13, 0; Write CCNT Register
 * +----------------------------------------------------------------------------
 */
struct cp15_c9_c13_CCNT {
	uint32 u32;
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_CCNT_RD(void)
{
	struct cp15_c9_c13_CCNT ccnt;
	__asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(ccnt.u32));
	return ccnt.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_CCNT_WR(const uint32 v32)
{
	struct cp15_c9_c13_CCNT ccnt;
	ccnt.u32 = v32;
	__asm__ volatile("mcr p15, 0, %0, c9, c13, 0" : : "r"(ccnt.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Event Selection (EVTSEL0 to EVTSEL2) Registers
 *  MRC p15, 0, <Rd>, c9, c13, 1; Read EVTSELx Register
 *  MCR p15, 0, <Rd>, c9, c13, 1; Write EVTSELx Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c13_EVTSEL {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 evt_type :  8;      /* event type to count */
		uint32 reserved : 24;
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_EVTSEL_RD(void)
{
	union cp15_c9_c13_EVTSEL evtsel;
	__asm__ volatile("mrc p15, 0, %0, c9, c13, 1" : "=r"(evtsel.u32));
	return evtsel.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_EVTSEL_WR(const uint32 v32)
{
	union cp15_c9_c13_EVTSEL evtsel;
	evtsel.u32 = v32;
	evtsel.reserved = 0;    /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c13, 1" : : "r"(evtsel.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Performance Count (PMC0-PMC2) Register
 *  MRC p15, 0, <Rd>, c9, c13, 2; Read PMCx Register
 *  MCR p15, 0, <Rd>, c9, c13, 2; Write PMCx Register
 * +----------------------------------------------------------------------------
 */
struct cp15_c9_c13_PMC {
	uint32 u32;
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_PMC_RD(void)
{
	struct cp15_c9_c13_PMC pmc;
	__asm__ volatile("mrc p15, 0, %0, c9, c13, 2" : "=r"(pmc.u32));
	return pmc.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_PMC_WR(const uint32 v32)
{
	struct cp15_c9_c13_PMC pmc;
	pmc.u32 = v32;
	__asm__ volatile("mcr p15, 0, %0, c9, c13, 2" : : "r"(pmc.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 User Enable (USEREN) Register
 *  MRC p15, 0, <Rd>, c9, c14, 0; Read USEREN Register
 *  MCR p15, 0, <Rd>, c9, c14, 0; Write USEREN Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c14_USEREN {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 enable   : 1;        /* user mode enable  */
		uint32 reserved : 31;
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_USEREN_RD(void)
{
	union cp15_c9_c14_USEREN useren;
	__asm__ volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(useren.u32));
	return useren.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_USEREN_WR(const uint32 v32)
{
	union cp15_c9_c14_USEREN useren;
	useren.u32 = v32;
	useren.reserved = 0;    /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c14, 0" : : "r"(useren.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Counter Overflow Interrupt Enable Set (INTENS) Register
 *  MRC p15, 0, <Rd>, c9, c14, 1 ; Read INTENS Register
 *  MCR p15, 0, <Rd>, c9, c14, 1 ; Write INTENS Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c14_INTENS {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0        : 1;    /* P0: Enable counter#0 interrupt set */
		uint32 ctr1        : 1;    /* P1: Enable counter#1 interrupt set */
		uint32 ctr2        : 1;    /* P2: Enable counter#2 interrupt set */
		uint32 reserved    : 28;
		uint32 cctr        : 1;    /* C: Enable cyclecount interrupt set */
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_INTENS_RD(void)
{
	union cp15_c9_c14_INTENS intens;
	__asm__ volatile("mrc p15, 0, %0, c9, c14, 1" : "=r"(intens.u32));
	return intens.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_INTENS_WR(const uint32 v32)
{
	union cp15_c9_c14_INTENS intens;
	intens.u32 = v32;
	intens.reserved = 0;    /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c14, 1" : : "r"(intens.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARM CR4 Counter Overflow Interrupt Enable Clear (INTENC) Register
 *  MRC p15, 0, <Rd>, c9, c14, 2 ; Read INTENC Register
 *  MCR p15, 0, <Rd>, c9, c14, 2 ; Write INTENC Register
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c14_INTENC {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0        : 1;    /* P0: Enable counter#0 interrupt clr */
		uint32 ctr1        : 1;    /* P1: Enable counter#1 interrupt clr */
		uint32 ctr2        : 1;    /* P2: Enable counter#2 interrupt clr */
		uint32 reserved    : 28;
		uint32 cctr        : 1;    /* C: Enable cyclecount interrupt clr */
	};
};
static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armcr4_INTENC_RD(void)
{
	union cp15_c9_c14_INTENC intenc;
	__asm__ volatile("mrc p15, 0, %0, c9, c14, 2" : "=r"(intenc.u32));
	return intenc.u32;
}
static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armcr4_INTENC_WR(const uint32 v32)
{
	union cp15_c9_c14_INTENC intenc;
	intenc.u32 = v32;
	intenc.reserved = 0;    /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c14, 2" : : "r"(intenc.u32));
}


static void BCM_BUZZZ_NOINSTR_FUNC
_armcr4_pmu_enable(void)
{
	union cp15_c9_c12_PMCR pmcr;
	union cp15_c9_c12_CNTENS cntens;
	union cp15_c9_c12_FLAG flag;

	/* Disable overflow interrupts on 3 buzzz counters: ctr0 .. ctr2 */
	flag.u32 = __armcr4_FLAG_RD();
	flag.ctr0 = flag.ctr1 = flag.ctr2 = 0;
	__armcr4_FLAG_WR(flag.u32);

	/* Set Enable bit in PMCR */
	pmcr.u32 = __armcr4_PMCR_RD();          /* mrc p15, 0, r0, c9, c12, 0 */
	pmcr.enable = 1;                        /* orr r0, r0, #0x01          */
	__armcr4_PMCR_WR(pmcr.u32);             /* mcr p15, 0, r0, c9, c12, 0 */

	/* Enable the 3 buzzz counters: ctr0 to ctr2 */
	cntens.u32 = __armcr4_CNTENS_RD();      /* mrc p15, 0, r0, c9, c12, 1 */
	cntens.ctr0 = cntens.ctr1 = cntens.ctr2 = 1;
	__armcr4_CNTENS_WR(cntens.u32);         /* mcr p15, 0, r0, c9, c12, 1 */
}

static void BCM_BUZZZ_NOINSTR_FUNC
_armcr4_pmu_disable(void)
{
	union cp15_c9_c12_CNTENC cntenc;

	/*
	 * Do not reset the pmcr.enable as this will disable the PMCCNTR cyclecnt.
	 * The OS may rely on PMCCNTR being enabled.
	 */
	cntenc.u32 = 0U;
	cntenc.ctr0 = cntenc.ctr1 = cntenc.ctr2 = 1;
	__armcr4_CNTENC_WR(cntenc.u32);
}

/* Macro equivalent to _armcr4_pmc_read_buzzz_ctr() */
#define __armcr4_pmu_read_buzzz_ctr(ctr_sel)                                   \
({                                                                             \
	uint32 v32 = ctr_sel;                                                      \
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 5" : : "r"(v32));               \
	__asm__ volatile("mrc p15, 0, %0, c9, c13, 2" : "=r"(v32));                \
	v32;                                                                       \
})

static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
_armcr4_pmu_read_buzzz_ctr(const uint32 ctr_sel)
{
	uint32 v32;
	union cp15_c9_c12_PMNXSEL pmnxsel;

	pmnxsel.u32 = 0U;
	pmnxsel.ctr_sel = ctr_sel;
	__armcr4_PMNXSEL_WR(pmnxsel.u32);

	v32 = __armcr4_PMC_RD();

	return v32;
}

#define BCM_BUZZZ_READ_COUNTER(ctrsel) __armcr4_pmu_read_buzzz_ctr(ctrsel)

static void BCM_BUZZZ_NOINSTR_FUNC
_armcr4_pmu_config_buzzz_ctr(const uint32 ctr_sel, const uint32 evt_type)
{
	union cp15_c9_c13_EVTSEL evtsel;
	union cp15_c9_c12_PMNXSEL pmnxsel;

	/* Select counter to be configured */
	pmnxsel.u32 = 0U;
	pmnxsel.ctr_sel = ctr_sel;
	__armcr4_PMNXSEL_WR(pmnxsel.u32);

	/* Configure event to be counted for selected counter */
	evtsel.u32 = 0U;
	evtsel.evt_type = evt_type;
	__armcr4_EVTSEL_WR(evtsel.u32);
}

#endif /*  BCM_BUZZZ_CONFIG_CPU_ARM_CR4 */


#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CM3)
/*
 * +----------------------------------------------------------------------------
 * ARM CM3 performance counter manipulations
 * +----------------------------------------------------------------------------
 */

#if !defined(__ARM_ARCH_7M__)
#error "Mismatch ARM Arch 7M and Cortex CM3"
#endif /* ! __ARM_ARCH_7M__ */

#include <arminc.h>

/* Debug Exception and Monitor Control Register */
#define BCM_BUZZZ_DEMCR                (0xE000EDFC)
#define BCM_BUZZZ_DEMCR_TRCENA         (1 << 24)

/* Available Debug Watchpoint and Trace (DWT) counters */
#define BCM_BUZZZ_DWT_CTRL             (0xE0001000)

#define BCM_BUZZZ_DWT_CYCCNT           (0xE0001004)
#define BCM_BUZZZ_DWT_CYCEVTENA        (1 << 22)
#define BCM_BUZZZ_DWT_CYCCNTENA        (1 << 0)

#define BCM_BUZZZ_DWT_CPICNT           (0xE0001008)
#define BCM_BUZZZ_DWT_CPIEVTENA        (1 << 17)

#define BCM_BUZZZ_DWT_EXCCNT           (0xE000100C)
#define BCM_BUZZZ_DWT_EXCEVTENA        (1 << 18)

#define BCM_BUZZZ_DWT_SLEEPCNT         (0xE0001010)
#define BCM_BUZZZ_DWT_SLEEPEVTENA      (1 << 19)

#define BCM_BUZZZ_DWT_LSUCNT           (0xE0001014)
#define BCM_BUZZZ_DWT_LSUEVTENA        (1 << 20)

#define BCM_BUZZZ_DWT_FOLDCNT          (0xE0001018)
#define BCM_BUZZZ_DWT_FOLDEVTENA       (1 << 21)

/* Enable all DWT counters */
#define BCM_BUZZZ_DWT_CTRL_ENAB                                                \
	(BCM_BUZZZ_DWT_CYCCNTENA | BCM_BUZZZ_DWT_CPIEVTENA |                       \
	 BCM_BUZZZ_DWT_EXCEVTENA | BCM_BUZZZ_DWT_SLEEPEVTENA |                     \
	 BCM_BUZZZ_DWT_LSUEVTENA | BCM_BUZZZ_DWT_FOLDEVTENA)

#define BCM_BUZZZ_ARM_IDCODE           (0x03) /* part num - 0xc23 */
#define BCM_BUZZZ_KEVT_GROUPS          (1)

/* Pack 8 bit CM3 counters */
typedef union cm3_cnts {
	uint32 u32;
	uint8 u8[4];
	struct {
		uint8 cpicnt;
		uint8 exccnt;
		uint8 sleepcnt;
		uint8 lsucnt;
	};
} cm3_cnts_t;

static void BCM_BUZZZ_NOINSTR_FUNC
_armcm3_enable_dwt(void)
{
	uint32 v32;
	volatile uint32 * reg32;

	/* Set Trace Enable bit24 in Debug Exception and Monitor Control Register */
	reg32 = (volatile uint32 *)BCM_BUZZZ_DEMCR;
	v32 = *reg32; *reg32 = v32 | BCM_BUZZZ_DEMCR_TRCENA;           /* OR_REG */

	/* Reset values in selected counters */
	reg32 = (volatile uint32 *)BCM_BUZZZ_DWT_CYCCNT; *reg32 = 0;   /* W_REG */

	reg32 = (volatile uint32 *)BCM_BUZZZ_DWT_CTRL; /* Enable DWT */
	v32 = *reg32; v32 &= 0xF0000000; /* Save the NUMCOMP ReadOnly bits 28::31 */
	/* Ensure bit 12 is not set! */
	*reg32 = v32 | BCM_BUZZZ_DWT_CTRL_ENAB;                       /* OR_REG */
}


static void BCM_BUZZZ_NOINSTR_FUNC
_armcm3_disable_dwt(void)
{
	uint32 v32;
	volatile uint32 * reg32;

	reg32 = (volatile uint32 *)BCM_BUZZZ_DWT_CTRL;
	v32 = *reg32; *reg32 = v32 & 0xF0000000;                   /* AND_REG */

	/* Clr Trace Enable bit24 in Debug Exception and Monitor Control Register */
	reg32 = (volatile uint32 *)BCM_BUZZZ_DEMCR;
	v32 = *reg32; *reg32 = v32 & ~BCM_BUZZZ_DEMCR_TRCENA;      /* AND_REG */
}

#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CM3 */

/* -------------------------------------------------------------------------- */
#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA7) || defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA9)

/*
 * +----------------------------------------------------------------------------
 * ARMv7 CA7 and CA9 Performance Monitoring Unit manipulations
 * +----------------------------------------------------------------------------
 */

#if !defined(__ARM_ARCH_7A__)
#error "Mismatch ARM Arch 7A and Cortex CA7/CA9"
#endif

#include <arminc.h>

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA7)
/*
 * ARMv7 Cortex-A7 has 4 counters.
 * BUZZZ uses ARMv7 A7 counters #0,#1,#2,#3 as BUZZZ counters #0,#1,#2,#3
 */
#define BCM_BUZZZ_KEVT_COUNTERS        BCM_BUZZZ_COUNTERS
#define BCM_BUZZZ_ARM_CTR_SEL(ctr_idx) ((ctr_idx) + 0)
#define BCM_BUZZZ_ARM_CTR_BIT(ctr_idx) (1 << BCM_BUZZZ_ARM_CTR_SEL(ctr_idx))
#define BCM_BUZZZ_ARM_IDCODE           (0x07)
#define BCM_BUZZZ_KEVT_GROUPS          (9)

typedef
struct bcm_buzzz_kevt_ctl
{
	uint8 u8[BCM_BUZZZ_KEVT_COUNTERS];
} bcm_buzzz_kevt_ctl_t;

static
bcm_buzzz_kevt_ctl_t bcm_buzzz_kevtctl_g[BCM_BUZZZ_KEVT_GROUPS] =
{
	/* group #0 RESET */
	{{    0U,   0U,   0U,   0U }},

	/* group #1 GENERAL */
	{{ BCM_BUZZZ_ARMV7_INSTRCNT_EVT,       /* 0x08 Instruction executed */
	   BCM_BUZZZ_ARMV7_CYCLECNT_EVT,       /* 0x11 Cycle count */
	   BCM_BUZZZ_ARMV7_BRMISS_EVT,         /* 0x10 Branch mispredicted */
	   BCM_BUZZZ_ARMV7_DATAMEMACC_EVT }},  /* 0x13 Data memory access */

	/* group #2 ICache */
	{{ BCM_BUZZZ_ARMV7_INSTRCNT_EVT,       /* 0x08 Instruction executed */
	   BCM_BUZZZ_ARMV7_CYCLECNT_EVT,       /* 0x11 Cycle count */
	   BCM_BUZZZ_ARMV7_ICACHEACC_EVT,      /* 0x14 Instruction cache access */
	   BCM_BUZZZ_ARMV7_ICACHEMISS_EVT }},  /* 0x01 Intruction cache miss */

	/* group #3 DCache */
	{{ BCM_BUZZZ_ARMV7_DATAMEMACC_EVT,     /* 0x13 Data memory access */
	   BCM_BUZZZ_ARMV7_DCACHEEVICT_EVT,    /* 0x15 Data cache eviction */
	   BCM_BUZZZ_ARMV7_DCACHEACC_EVT,      /* 0x04 L1 data cache access */
	   BCM_BUZZZ_ARMV7_DCACHEMISS_EVT }},  /* 0x03 L1 Data cache miss */

	/* group #4 Data Access */
	{{ BCM_BUZZZ_ARMV7_DATAMEMACC_EVT,     /* 0x13 Data memory access */
	   BCM_BUZZZ_ARMV7_DATASNOOP_EVT,      /* 0xca Data snooped fr other proc */
	   BCM_BUZZZ_ARMV7_DATAREAD_EVT,       /* 0x06 Data read executed */
	   BCM_BUZZZ_ARMV7_DATAWRITE_EVT }},   /* 0x07 Data write executed */

	/* group #5 Exception */
	{{ BCM_BUZZZ_ARMV7_EXPCNT_EVT,         /* 0x09 Exception taken */
	   BCM_BUZZZ_ARMV7_IRQEXCP_EVT,        /* 0x86 IRQ exceptions taken */
	   BCM_BUZZZ_ARMV7_UNALIGNED_EVT,      /* 0x0f Unaligned access executed */
	   BCM_BUZZZ_ARMV7_WR_STALL_EVT }},    /* 0xc9 Pipeline stall, WR store full */

	/* group #6 Program Flow */
	{{ BCM_BUZZZ_ARMV7_EXPRTN_EVT,         /* 0x0a Exception return executed */
	   BCM_BUZZZ_ARMV7_SWCHGPC_EVT,        /* 0x0c SW change of PC executed */
	   BCM_BUZZZ_ARMV7_BICNT_EVT,          /* 0x0d B/BL/BLX immed executed */
	   BCM_BUZZZ_ARMV7_PROCRTN_EVT }},     /* 0x0e Procedure return executed */

	/* group #7 Bus */
	{{ BCM_BUZZZ_ARMV7_BUSSACC_EVT,        /* 0x19 Bus accesses */
	   BCM_BUZZZ_ARMV7_BUSCYCLE_EVT,       /* 0x1d Bus cycle */
	   BCM_BUZZZ_ARMV7_BUSRDACC_EVT,       /* 0x60 Bus access read */
	   BCM_BUZZZ_ARMV7_BUSWRACC_EVT }},    /* 0x61 Bus access write */

	/* group #8 Prefetch External memory request */
	{{ BCM_BUZZZ_ARMV7_EXTMEMREQ_EVT,      /* 0xc0 External memory request */
	   BCM_BUZZZ_ARMV7_NC_EXTMEMREQ_EVT,   /* 0xc1 Non cacheable ext mem req */
	   BCM_BUZZZ_ARMV7_PREF_LFILL_EVT,     /* 0xc2 Prefetch line fill */
	   BCM_BUZZZ_ARMV7_PREF_DROP_EVT }}    /* 0xc3 Prefetch line fill dropped */

};
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CA7 */

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA9)
/* Assuming a single-core A9, unlike the Host-side dual core A9 */
/*
 * ARMv7 Cortex-A9 has 6 counters. First 2 are reserved for system usage.
 * BUZZZ uses ARMv7 A9 counters #2,#3,#4,#5 as BUZZZ counters #0,#1,#2,#3
 */
#define BCM_BUZZZ_KEVT_COUNTERS        (BCM_BUZZZ_COUNTERS - 2)
#define BCM_BUZZZ_ARM_CTR_SEL(ctr_idx) ((ctr_idx) + 2)
#define BCM_BUZZZ_ARM_CTR_BIT(ctr_idx) (1 << BCM_BUZZZ_ARM_CTR_SEL(ctr_idx))
#define BCM_BUZZZ_ARM_IDCODE           (0x09)
#define BCM_BUZZZ_KEVT_GROUPS          (9)

typedef
struct bcm_buzzz_kevt_ctl
{
	uint8 u8[BCM_BUZZZ_KEVT_COUNTERS];
} bcm_buzzz_kevt_ctl_t;

static
bcm_buzzz_kevt_ctl_t bcm_buzzz_kevtctl_g[BCM_BUZZZ_KEVT_GROUPS] =
{
	/*  ctl0  ctl1  ctl2  ctl3 */
	{{    0U,   0U,   0U,   0U }},      /* group  0 RESET            */
	{{  0x68, 0x11, 0x10, 0x09 }},      /* group  1 GENERAL          */
	{{  0x68, 0x01, 0x60, 0x66 }},      /* group  2 ICACHE           */
	{{  0x04, 0x03, 0x61, 0x65 }},      /* group  3 DCACHE           */
	{{  0x02, 0x05, 0x82, 0x83 }},      /* group  4 TLB              */
	{{  0x06, 0x07, 0x81, 0x86 }},      /* group  5 DATA             */
	{{  0x93, 0x0F, 0x0A, 0x62 }},      /* group  6 SPURIOUS         */
	{{  0x0C, 0x0D, 0x0E, 0x12 }},      /* group  7 BRANCHES         */
	{{  0x63, 0x64, 0x91, 0x92 }}       /* group  8 MISCELLANEOUS    */
};
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CA9 */

union cp15_c9_REG {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr0     :  1; /* A7:Ctr#0  A9:rsvd  */
		uint32 ctr1     :  1; /* A7:Ctr#1  A9:rsvd  */
		uint32 ctr2     :  1; /* A7:Ctr#2  A9:Ctr#0 */
		uint32 ctr3     :  1; /* A7:Ctr#3  A9:Ctr#1 */
		uint32 ctr4     :  1; /* A7:none   A9:Ctr#2 */
		uint32 ctr5     :  1; /* A7:none   A9:Ctr#3 */
		uint32 ctr6     :  1;
		uint32 ctr7     :  1;
		uint32 none     : 23;
		uint32 cycle    :  1; /* cycle count register                       */
	};
};

/*
 * +----------------------------------------------------------------------------
 * ARMv7 Performance Monitor Control Register
 *  MRC p15, 0, <Rd>, c9, c12, 0 ; Read PMCR
 *  MCR p15, 0, <Rd>, c9, c12, 0 ; Write PMCR
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_PMCR {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 enable      : 1;    /* E: enable all counters incld cctr  */
		uint32 evt_reset   : 1;    /* P: event counter reset             */
		uint32 cctr_reset  : 1;    /* C: cycle counter reset             */
		uint32 clk_divider : 1;    /* D: cycle count divider: 0= 1:1     */
		uint32 export_en   : 1;    /* X: export enable                   */
		uint32 prohibit    : 1;    /* DP: disable in prohibited regions  */
		uint32 reserved    : 5;    /* ReadAZ, Wr: ShouldBeZero/Preserved */
		uint32 counters    : 5;    /* N: number of event counters        */
		uint32 id_code     : 8;    /* IDCODE: identification code        */
		                           /* 0x07=Cortex-A7, 0x09=Cortex-A9     */
		uint32 impl_code   : 8;    /* IMP: implementer code 0x41=ARM     */
	};
};

static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMCR_RD(void)
{
	union cp15_c9_c12_PMCR pmcr;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr.u32));
	if (pmcr.id_code != BCM_BUZZZ_ARM_IDCODE) {
		BCM_BUZZZ_ER(("ERROR: PMCR id_code mismatch 0x%02X != 0x%02X\n",
		          pmcr.id_code, BCM_BUZZZ_ARM_IDCODE));
	}
	BCM_BUZZZ_PR(("RD PMCR IMP%u ID%u N[%u] DP[%u] X[%u] D[%u] C[%u] P[%u] E[%u]",
		pmcr.impl_code, pmcr.id_code, pmcr.counters, pmcr.prohibit,
		pmcr.export_en, pmcr.clk_divider, pmcr.cctr_reset, pmcr.evt_reset,
		pmcr.enable));
	return pmcr.u32;
}

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMCR_WR(const uint32 v32)
{
	union cp15_c9_c12_PMCR pmcr;
	pmcr.u32 = v32;
	pmcr.reserved = 0;  /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 0" : : "r"(pmcr.u32));
	BCM_BUZZZ_PR(("RD PMCR IMP%u ID%u N[%u] DP[%u] X[%u] D[%u] C[%u] P[%u] E[%u]",
		pmcr.impl_code, pmcr.id_code, pmcr.counters, pmcr.prohibit,
		pmcr.export_en, pmcr.clk_divider, pmcr.cctr_reset, pmcr.evt_reset,
		pmcr.enable));
}


/*
 * +----------------------------------------------------------------------------
 * ARMv7 Performance Monitor event counter SELection Register
 *  MRC p15, 0, <Rd>, c9, c12, 5 ; Read PMSELR
 *  MCR p15, 0, <Rd>, c9, c12, 5 ; Write PMSELR
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c12_PMSELR {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 ctr_select: 8; /* event counter selector */
		uint32 reserved  :24; /* reserved               */
	};
};

static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMSELR_RD(void)
{
	union cp15_c9_c12_PMSELR pmselr;
	__asm__ volatile("mrc p15, 0, %0, c9, c12, 5" : "=r"(pmselr.u32));
	return pmselr.u32;
}

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMSELR_WR(const uint32 v32)
{
	union cp15_c9_c12_PMSELR pmselr;
	pmselr.u32 = v32;
	pmselr.reserved = 0;  /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c12, 5" : : "r"(pmselr.u32));
}


/*
 * +----------------------------------------------------------------------------
 * ARMv7 Performance Monitor EVent TYPE selection Register
 *  MRC p15, 0, <Rd>, c9, c13, 1 ; Read PMXEVTYPER
 *  MCR p15, 0, <Rd>, c9, c13, 1 ; Write PMXEVTYPER
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c13_PMXEVTYPER {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 evt_type :  8; /* event type to count */
		uint32 reserved : 24; /* reserved            */
	};
};

static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMXEVTYPER_RD(void)
{
	union cp15_c9_c13_PMXEVTYPER pmxevtyper;
	__asm__ volatile("mrc p15, 0, %0, c9, c13, 1" : "=r"(pmxevtyper.u32));
	return pmxevtyper.u32;
}

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMXEVTYPER_WR(const uint32 v32)
{
	union cp15_c9_c13_PMXEVTYPER pmxevtyper;
	pmxevtyper.u32 = v32;
	pmxevtyper.reserved = 0;  /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c13, 1" : : "r"(pmxevtyper.u32));
}

/*
 * +----------------------------------------------------------------------------
 * ARMv7 Performance Monitor User Enable Register
 *  MRC p15, 0, <Rd>, c9, c14, 0 ; Read PMUSERENR
 *  MCR p15, 0, <Rd>, c9, c14, 0 ; Write PMUSERENR
 * +----------------------------------------------------------------------------
 */
union cp15_c9_c14_PMUSERENR {
	uint32 u32;
	struct {    /* Little Endian */
		uint32 enable   :  1; /* user mode enable */
		uint32 reserved : 31; /* reserved         */
	};
};

static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMUSERENR_RD(void)
{
	union cp15_c9_c14_PMUSERENR pmuserenr;
	__asm__ volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuserenr.u32));
	return pmuserenr.u32;
}

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
__armv7_PMUSERENR_WR(const uint32 v32)
{
	union cp15_c9_c14_PMUSERENR pmuserenr;
	pmuserenr.u32 = v32;
	pmuserenr.reserved = 0;  /* Should Be Zero Preserved */
	__asm__ volatile("mcr p15, 0, %0, c9, c14, 0" : : "r"(pmuserenr.u32));
}

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
_armv7_pmu_enable(void)
{
	int ctr_idx;
	union cp15_c9_c12_PMCR pmcr;
	union cp15_c9_REG pmcntenset;
	union cp15_c9_REG pmintenclr;

	/* Set Enable bit in PMCR */
	pmcr.u32 = __armv7_PMCR_RD();
	pmcr.enable = 1;
	__armv7_PMCR_WR(pmcr.u32);

	/*
	 * As we do not want interrupts on event counters overflow,
	 * lets not bother with clearing the PMOVSR flags for the N counters.
	 */

	/*
	 * Disable overflow interrupts on 4 buzzz counters:
	 *     Cortex-A7 ctr0 to ctr3
	 *     Cortex-A9 ctr2 to ctr5
	 */
	pmintenclr.u32 = 0U;
	for (ctr_idx = 0; ctr_idx < BCM_BUZZZ_KEVT_COUNTERS; ctr_idx++) {
		pmintenclr.u32 += BCM_BUZZZ_ARM_CTR_BIT(ctr_idx);
	}
	asm volatile("mcr p15, 0, %0, c9, c14, 2" : : "r"(pmintenclr.u32));
	BCM_BUZZZ_PR(("Disable overflow interrupts PMINTENCLR[%08x]",
	              pmintenclr.u32));

	/*
	 * Enable the 4 buzzz counters:
	 *     Cortex-A7 ctr0 to ctr3
	 *     Cortex-A9 ctr2 to ctr5
	 */
	pmcntenset.u32 = 0U;
	for (ctr_idx = 0; ctr_idx < BCM_BUZZZ_KEVT_COUNTERS; ctr_idx++) {
		pmcntenset.u32 += BCM_BUZZZ_ARM_CTR_BIT(ctr_idx);
	}
	asm volatile("mcr p15, 0, %0, c9, c12, 1" : : "r"(pmcntenset.u32));
	BCM_BUZZZ_PR(("Enable CTR PMCNTENSET[%08x]", pmcntenset.u32));
}

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
_armv7_pmu_disable(void)
{
	int ctr_idx;
	union cp15_c9_REG pmcntenclr;

	/*
	 * Do not disable the PMCR::enable bit as this would also disable the
	 * PMCCNTR cycle count register.
	 */

	/*
	 * Disable the 4 buzzz counters:
	 *     Cortex-A7 ctr0 to ctr3
	 *     Cortex-A9 ctr2 to ctr5
	 */
	pmcntenclr.u32 = 0U;
	for (ctr_idx = 0; ctr_idx < BCM_BUZZZ_KEVT_COUNTERS; ctr_idx++) {
		pmcntenclr.u32 += BCM_BUZZZ_ARM_CTR_BIT(ctr_idx);
	}
	asm volatile("mcr p15, 0, %0, c9, c12, 2" : : "r"(pmcntenclr.u32));
	BCM_BUZZZ_PR(("Disable CTR2-5 PMCNTENCLR[%08x]", pmcntenclr.u32));
}

static BCM_BUZZZ_INLINE uint32 BCM_BUZZZ_NOINSTR_FUNC
_armv7_pmu_read_buzzz_ctr(const uint32 ctr_idx)
{
	uint32 v32;
	union cp15_c9_c12_PMSELR pmselr;
	pmselr.u32 = BCM_BUZZZ_ARM_CTR_SEL(ctr_idx); /* use buzzz mapped ctr */
	asm volatile("mcr p15, 0, %0, c9, c12, 5" : : "r"(pmselr.u32));
	asm volatile("mrc p15, 0, %0, c9, c13, 2" : "=r"(v32));
	return v32;
}

#define BCM_BUZZZ_READ_COUNTER(ctr_idx)  _armv7_pmu_read_buzzz_ctr(ctr_idx)

static BCM_BUZZZ_INLINE void BCM_BUZZZ_NOINSTR_FUNC
_armv7_pmu_config_buzzz_ctr(const uint32 ctr_idx, const uint32 evt_type)
{
	union cp15_c9_c13_PMXEVTYPER pmxevtyper;
	union cp15_c9_c12_PMSELR pmselr;

	pmselr.u32 = 0U;    /* Select the Counter using PMSELR */
	pmselr.ctr_select = BCM_BUZZZ_ARM_CTR_SEL(ctr_idx);
	asm volatile("mcr p15, 0, %0, c9, c12, 5" : : "r"(pmselr.u32));

	pmxevtyper.u32 = 0U;
	pmxevtyper.evt_type = evt_type; /* Configure the event type */
	asm volatile("mcr p15, 0, %0, c9, c13, 1" : : "r"(pmxevtyper.u32));

	BCM_BUZZZ_PR(("Config Ctr<%u> PMSELR[%08x] PMXEVTYPER[%08x]",
		ctr_idx, pmselr.u32, pmxevtyper.u32));
}

#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CA7 || BCM_BUZZZ_CONFIG_CPU_ARM_CA9 */


/* -------------------------------------------------------------------------- */

typedef struct bcm_buzzz_log
{
#if defined(BCM_BUZZZ_KEVT_COUNTERS)
	uint32 ctr[BCM_BUZZZ_KEVT_COUNTERS];
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CR4 */

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CM3)
	uint32 cyccnt;
	cm3_cnts_t cnts; /* Counters are compacted into cm3_cnts */
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CM3 */

	bcm_buzzz_arg0_t arg0;
	uint32 arg1;                  /* upto 1 argument */
	uint32 arg2;

#if defined(BCM_BUZZZ_4ARGS)
	uint32 arg3, arg4;      /* upto 4 arguments */
#endif /*   BCM_BUZZZ_4ARGS */

} bcm_buzzz_log_t;


static bcm_buzzz_t bcm_buzzz_g =
{
	.log    = (uint32)NULL,
	.cur    = (uint32)NULL,
	.end    = (uint32)NULL,

	.count  = 0U,
	.status = BCM_BUZZZ_DISABLED,
	.wrap   = BCM_BUZZZ_FALSE,

	.cpu_idcode = BCM_BUZZZ_ARM_IDCODE,
#if defined(BCM_BUZZZ_KEVT_COUNTERS)
	.counters  = BCM_BUZZZ_KEVT_COUNTERS, /* Cortex-M4, A7, A9 */
#else
	.counters  = BCM_BUZZZ_COUNTERS,      /* Cortex-M3 */
#endif
	.group     = 1, /* default group */

	.buffer_sz = BCM_BUZZZ_LOG_BUFSIZE,
	.log_sz    = sizeof(bcm_buzzz_log_t),
	.eventid   = { /* default events selected */
#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CR4)
	               BCM_BUZZZ_ARMCR4_INSTRCNT_EVT,
	               BCM_BUZZZ_ARMCR4_CYCLECNT_EVT,
	               BCM_BUZZZ_ARMCR4_BRMISS_EVT,
	               0, 0, 0, 0, 0
#elif defined(__ARM_ARCH_7A__)
#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA7)
	               BCM_BUZZZ_ARMV7_INSTRCNT_EVT,
#elif defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA9)
	               BCM_BUZZZ_ARMV7_SPECINST_EVT,
#endif
	               BCM_BUZZZ_ARMV7_CYCLECNT_EVT,
	               BCM_BUZZZ_ARMV7_BRMISS_EVT,
	               BCM_BUZZZ_ARMV7_DATAMEMACC_EVT,
	               0, 0, 0, 0
#endif /* __ARM_ARCH_7A__ */
	             } /* stop supporting this ... */
};

#define BCM_BUZZZ_G (bcm_buzzz_g)


#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CR4)
#define _BCM_BUZZZ_PREAMBLE(log_p, evt_id, num_args)                           \
({                                                                             \
	if (BCM_BUZZZ_G.status != BCM_BUZZZ_ENABLED) return;                       \
	log_p = (bcm_buzzz_log_t *)BCM_BUZZZ_G.cur;                                \
	(log_p)->arg0.klog.id = (evt_id);                                          \
	(log_p)->arg0.klog.args = (num_args);                                      \
	(log_p)->ctr[0] = BCM_BUZZZ_READ_COUNTER(0);                               \
	(log_p)->ctr[1] = BCM_BUZZZ_READ_COUNTER(1);                               \
	(log_p)->ctr[2] = BCM_BUZZZ_READ_COUNTER(2);                               \
})
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CR4 */


#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CM3)
#define _BCM_BUZZZ_PREAMBLE(log_p, evt_id, num_args)                           \
({                                                                             \
	cm3_cnts_t cm3_cnts;                                                       \
	volatile uint8  * reg8;                                                    \
	volatile uint32 * reg32;                                                   \
	if (BCM_BUZZZ_G.status != BCM_BUZZZ_ENABLED) return;                       \
	log_p = (bcm_buzzz_log_t*)BCM_BUZZZ_G.cur;                                 \
	reg32 = (volatile uint32*)BCM_BUZZZ_DWT_CYCCNT; (log_p)->cyccnt = *reg32;  \
	reg8 = (volatile uint8*)BCM_BUZZZ_DWT_CPICNT; cm3_cnts.cpicnt = *reg8;     \
	reg8 = (volatile uint8*)BCM_BUZZZ_DWT_EXCCNT; cm3_cnts.exccnt = *reg8;     \
	reg8 = (volatile uint8*)BCM_BUZZZ_DWT_SLEEPCNT; cm3_cnts.sleepcnt = *reg8; \
	reg8 = (volatile uint8 *)BCM_BUZZZ_DWT_LSUCNT; cm3_cnts.lsucnt = *reg8;    \
	(log_p)->cnts.u32 = cm3_cnts.u32;                                          \
	reg8 = (volatile uint8*)BCM_BUZZZ_DWT_FOLDCNT;                             \
	(log_p)->arg0.u32 = (*reg8) | ((num_args) << 8) | ((evt_id) << 16);        \
})
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CM3 */


#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA7) || defined(BCM_BUZZZ_CONFIG_CPU_ARM_CA9)
#define _BCM_BUZZZ_PREAMBLE(log_p, evt_id, num_args)                           \
({                                                                             \
	if (BCM_BUZZZ_G.status != BCM_BUZZZ_ENABLED) return;                       \
	log_p = (bcm_buzzz_log_t *)BCM_BUZZZ_G.cur;                                \
	(log_p)->arg0.klog.id = (evt_id);                                          \
	(log_p)->arg0.klog.args = (num_args);                                      \
	(log_p)->ctr[0] = BCM_BUZZZ_READ_COUNTER(0);                               \
	(log_p)->ctr[1] = BCM_BUZZZ_READ_COUNTER(1);                               \
	(log_p)->ctr[2] = BCM_BUZZZ_READ_COUNTER(2);                               \
	(log_p)->ctr[3] = BCM_BUZZZ_READ_COUNTER(3);                               \
})
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CA7 || BCM_BUZZZ_CONFIG_CPU_ARM_CA9 */


#define _BCM_BUZZZ_POSTAMBLE()                                                 \
({                                                                             \
	BCM_BUZZZ_G.cur = (uint32)(((bcm_buzzz_log_t *)BCM_BUZZZ_G.cur) + 1);      \
	BCM_BUZZZ_G.count++;                                                       \
	if (BCM_BUZZZ_G.cur >= BCM_BUZZZ_G.end) {                                  \
		BCM_BUZZZ_G.wrap = BCM_BUZZZ_TRUE;                                     \
		BCM_BUZZZ_G.cur = BCM_BUZZZ_G.log;                                     \
	}                                                                          \
})


void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_log0(uint32 evt_id)
{
	bcm_buzzz_log_t * log_p;
	_BCM_BUZZZ_PREAMBLE(log_p, evt_id, 0);
	_BCM_BUZZZ_POSTAMBLE();
}

void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_log1(uint32 evt_id, uint32 arg1)
{
	bcm_buzzz_log_t * log_p;
	_BCM_BUZZZ_PREAMBLE(log_p, evt_id, 1);
	log_p->arg1 = arg1;
	_BCM_BUZZZ_POSTAMBLE();
}


void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_log2(uint32 evt_id, uint32 arg1, uint32 arg2)
{
	bcm_buzzz_log_t * log_p;
	_BCM_BUZZZ_PREAMBLE(log_p, evt_id, 2);
	log_p->arg1 = arg1;
	log_p->arg2 = arg2;
	_BCM_BUZZZ_POSTAMBLE();
}

#if defined(BCM_BUZZZ_4ARGS)
void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_log3(uint32 evt_id, uint32 arg1, uint32 arg2, uint32 arg3)
{
	bcm_buzzz_log_t * log_p;
	_BCM_BUZZZ_PREAMBLE(log_p, evt_id, 3);
	log_p->arg1 = arg1;
	log_p->arg2 = arg2;
	log_p->arg3 = arg3;
	_BCM_BUZZZ_POSTAMBLE();
}

void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_log4(uint32 evt_id,
           uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4)
{
	bcm_buzzz_log_t * log_p;
	_BCM_BUZZZ_PREAMBLE(log_p, evt_id, 4);
	log_p->arg1 = arg1;
	log_p->arg2 = arg2;
	log_p->arg3 = arg3;
	log_p->arg4 = arg4;
	_BCM_BUZZZ_POSTAMBLE();
}
#endif /* BCM_BUZZZ_4ARGS */


/*
 * -----------------------------------------------------------------------------
 * Show the tracing state of buzzz.
 *
 * bcm_buzzz_show() may be directly invoked in dongle source,
 * or via: wl -i eth### bus:buzzz_show
 *
 * -----------------------------------------------------------------------------
 */
void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_show(void)
{
	printf("BCM_BUZZZ log<%08x> cur<%08x> end<%08x> "
		   " count<%u> status<%u> wrap<%u>\n"
		   "cpu<0x%02X> counters<%d> group<%d> buffer_sz<%d> log_sz<%d>\n",
		   (int)BCM_BUZZZ_G.log, (int)BCM_BUZZZ_G.cur, (int)BCM_BUZZZ_G.end,
		   BCM_BUZZZ_G.count, BCM_BUZZZ_G.status, BCM_BUZZZ_G.wrap,
		   BCM_BUZZZ_G.cpu_idcode, BCM_BUZZZ_G.counters, BCM_BUZZZ_G.group,
		   BCM_BUZZZ_G.buffer_sz, BCM_BUZZZ_G.log_sz);
}

void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_dump()
{
	bcm_buzzz_show();
	BCM_BUZZZ_ER(("Use: dhd -i eth## buzzz_dump\n"));
}


/*
 * -----------------------------------------------------------------------------
 * Start kernel event tracing. Configures the performance counters and enables
 * performance counting. Tool status is set enabled.
 *
 * bcm_buzzz_start() may be directly invoked in dongle source,
 * or via: wl -i eth### bus:buzzz_start
 *
 * -----------------------------------------------------------------------------
 */
void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_start(void)
{
	int ctr;

	if (BCM_BUZZZ_G.log == NULL) {
		BCM_BUZZZ_ER(("ERROR: bcm_buzzz not registered\n"));
		return;
	}
	BCM_BUZZZ_PR(("bcm_buzzz_start\n"));

	BCM_BUZZZ_G.wrap  = BCM_BUZZZ_FALSE;
	BCM_BUZZZ_G.cur   = BCM_BUZZZ_G.log;
	BCM_BUZZZ_G.end   = (uint32)((char*)(BCM_BUZZZ_G.log)
	              + (BCM_BUZZZ_LOG_BUFSIZE - BCM_BUZZZ_LOGENTRY_MAXSZ));

	BCM_BUZZZ_G.cpu_idcode = BCM_BUZZZ_ARM_IDCODE;

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CR4)
	/* Select counter and configure event */
	for (ctr = 0; ctr < BCM_BUZZZ_KEVT_COUNTERS; ctr++)
		_armcr4_pmu_config_buzzz_ctr(ctr, BCM_BUZZZ_G.eventid[ctr]);
	_armcr4_pmu_enable();
#endif  /*  BCM_BUZZZ_CONFIG_CPU_ARM_CR4 */

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CM3)
	ctr = 0;
	_armcm3_enable_dwt();
#endif  /*  BCM_BUZZZ_CONFIG_CPU_ARM_CM3 */

#if defined(__ARM_ARCH_7A__)
	for (ctr = 0; ctr < BCM_BUZZZ_KEVT_COUNTERS; ctr++)
		_armv7_pmu_config_buzzz_ctr(ctr, BCM_BUZZZ_G.eventid[ctr]);
	_armv7_pmu_enable();
#endif /* __ARM_ARCH_7A__ */

	BCM_BUZZZ_G.status = BCM_BUZZZ_ENABLED;
}


/*
 * -----------------------------------------------------------------------------
 * Stops kernel event tracing. Disable performance counting and tool status set
 * to disabled.
 *
 * bcm_buzzz_stop() may be directly invoked in dongle source,
 * or via: wl -i eth### bus:buzzz_stop
 *
 * -----------------------------------------------------------------------------
 */
#if defined(BCM_BUZZZ_THREADX) || defined(BCM_BUZZZ_HNDRTE)
extern void hnd_delay(uint32 us);
extern uint32 hnd_time_us(void);
#endif /* BCM_BUZZZ_THREADX || BCM_BUZZZ_HNDRTE */

void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_stop(void)
{
	int ctr;

#if defined(BCM_BUZZZ_TEST)
	/* Estimate the over head per event counter type using back-to-back calls */
	BUZZZ_LVL1(BUZZZ_0, 0);
	BUZZZ_LVL1(BUZZZ_0, 0);
	BUZZZ_LVL1(BUZZZ_0, 0);
	hnd_delay(100);
	BUZZZ_LVL1(BUZZZ_0, 0);
	hnd_delay(1000);
	BUZZZ_LVL1(BUZZZ_0, 0);
	hnd_delay(10000);
	BUZZZ_LVL1(BUZZZ_0, 0);

	BUZZZ_LVL1(BUZZZ_1, 1, hnd_time_us());
	BUZZZ_LVL1(BUZZZ_1, 1, hnd_time_us());
	BUZZZ_LVL1(BUZZZ_1, 1, hnd_time_us());
	BUZZZ_LVL1(BUZZZ_1, 1, hnd_time_us());

#if defined(BCM_BUZZZ_4ARGS)
	BUZZZ_LVL1(BUZZZ_2, 2, 1, 1);
	BUZZZ_LVL1(BUZZZ_2, 2, 2, 2);
	BUZZZ_LVL1(BUZZZ_2, 2, 3, 3);
	BUZZZ_LVL1(BUZZZ_2, 2, 4, 4);
#endif /* BCM_BUZZZ_4ARGS */
#endif /* BCM_BUZZZ_TEST */

	if (BCM_BUZZZ_G.status != BCM_BUZZZ_DISABLED) {
		BCM_BUZZZ_G.status = BCM_BUZZZ_DISABLED;

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CR4)
		for (ctr = 0; ctr < BCM_BUZZZ_KEVT_COUNTERS; ctr++)
			_armcr4_pmu_config_buzzz_ctr(ctr, 0x0);
		_armcr4_pmu_disable();
#endif  /*  BCM_BUZZZ_CONFIG_CPU_ARM_CR4 */

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CM3)
		_armcm3_disable_dwt();
#endif  /*  BCM_BUZZZ_CONFIG_CPU_ARM_CM3 */

#if defined(__ARM_ARCH_7A__)
		for (ctr = 0; ctr < BCM_BUZZZ_KEVT_COUNTERS; ctr++)
			_armv7_pmu_config_buzzz_ctr(ctr, 0x0);
		_armv7_pmu_disable();
#endif /* __ARM_ARCH_7A__ */

		BCM_BUZZZ_PR(("bcm_buzzz_stop\n"));
	}
}


/*
 * Use bcm_buzzz_config() to select a different 3rd counter.
 *
 * By default, on Cortex-R4, the 1st 2 counters are cycle count and instruction
 * count, and the third counter is Branch Misprediction count.
 * You may overwrite the 3rd counter by selecting and alternative event.
 *
 * bcm_buzzz_config() may be directly invoked in dongle source,
 * or via: wl -i eth### bus:buzzz_config
 *
 */

void BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_config(uint32 grp_sel)
{
	if (BCM_BUZZZ_G.status == BCM_BUZZZ_ENABLED) {
		BCM_BUZZZ_ER(("ERROR: bcm_buzzz is enabled!\n"));
		return;
	}

#if defined(BCM_BUZZZ_CONFIG_CPU_ARM_CR4)
	BCM_BUZZZ_G.eventid[2] = grp_sel; /* Event for 3rd counter only */
	BCM_BUZZZ_PR(("ARM CR4: CfgCtr[%02X, %02X, %02X]\n",
	    BCM_BUZZZ_G.eventid[0], BCM_BUZZZ_G.eventid[1], BCM_BUZZZ_G.eventid[2]));
#endif /* BCM_BUZZZ_CONFIG_CPU_ARM_CR4 */

#if defined(__ARM_ARCH_7A__)
	int ctr_idx;

	if ((grp_sel == 0) || (grp_sel > BCM_BUZZZ_KEVT_GROUPS)) {
		BCM_BUZZZ_ER(("ERROR: Invalid group<%u>\n", grp_sel));
		return;
	}

	/* Apply the event configuration for the selected group */
	for (ctr_idx = 0; ctr_idx < BCM_BUZZZ_KEVT_COUNTERS; ctr_idx++) {
		BCM_BUZZZ_G.eventid[ctr_idx] = bcm_buzzz_kevtctl_g[grp_sel].u8[ctr_idx];
		BCM_BUZZZ_PR(("ARMv7 CfgCtr[%d] = %02X\n",
			ctr_idx, BCM_BUZZZ_G.eventid[ctr_idx]));
	}
#endif /* __ARM_ARCH_7A__ */

	BCM_BUZZZ_G.group = grp_sel;
	BCM_BUZZZ_PR(("BCM_BUZZZ:grp<%d>\n", BCM_BUZZZ_G.group));
}


/*
 * Allocate the log buffer and register the BCM_BUZZZ_G.
 * Event format list is exported to host side using bcm_buzzz.h
 *
 * Invoke this once in a datapath module's init.
 */
int BCM_BUZZZ_NOINSTR_FUNC
bcm_buzzz_init(void * shared)
{
	pciedev_shared_t *sh = (pciedev_shared_t *)shared;
	void * buffer_p = NULL;

	if ((buffer_p = MALLOC(NULL, BCM_BUZZZ_LOG_BUFSIZE)) == NULL) {
		BCM_BUZZZ_ER(("ERROR: BCM_BUZZZ malloc<%d>\n", BCM_BUZZZ_LOG_BUFSIZE));
		return -1;
	}

	BCM_BUZZZ_G.log = (uint32)buffer_p;

	/* Register with Host side */
	sh->buzzz = (uint32)&BCM_BUZZZ_G;

	BCM_BUZZZ_PR(("Registered BCM_BUZZZ_G<%p> log<%08x:%d>\n",
		&BCM_BUZZZ_G, (int)BCM_BUZZZ_G.log, BCM_BUZZZ_LOG_BUFSIZE));

	return 0;
}

#endif /* BCM_BUZZZ */
