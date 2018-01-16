/*
 * BCM43XX ARM core routines. Contains functions that are ARM, but not RTOS specific (so ideally no
 * RTE specific constructs should be used in this file).
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
 *
 * $Id: hndarm.c 641509 2016-06-03 03:21:08Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <arminc.h>
#include <sbhndarm.h>
#include <hndpmu.h>
#include <bcmdevs.h>
#include <sbsocram.h>
#ifdef BCMTCAM
#include <hndtcam.h>
#endif

#if defined(CONFIG_XIP) && defined(BCMTCAM)
extern uint8 patch_pair;
#endif /* CONFIG_XIP && BCMTCAM */


#ifdef __ARM_ARCH_7R__
extern uint32 _armcr4_idx;
#else
static uint32 _armcr4_idx = 0;
#endif
static uint32 get_armcr4_idx(void);

#ifdef BCMDBG
#define	HNDARM_ERROR(args)	printf args
#else
#define	HNDARM_ERROR(args)
#endif	/* BCMDBG */

#ifdef BCMDBG
#define	HNDARM_MSG(args)	printf args
#else
#define	HNDARM_MSG(args)
#endif	/* BCMDBG */

#ifdef BCM_OL_DEV
#undef HNDARM_MSG
#define	HNDARM_MSG(args)
#endif

#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
#define VUSBD_43242_BT_2_WL_INTR	5
#ifdef VUSB_4350b1
#define VUSBD_4350_BT_2_WL_INTR		7
#else
#define VUSBD_4350_BT_2_WL_INTR		11
#endif

void si_set_BT_irq(si_t *sih);
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */

uint32
BCMRAMFN(get_armcr4_idx)(void)
{
	return _armcr4_idx;
}

#if defined(__ARM_ARCH_4T__)
/* point to either arm7sr0_wait() or arm_wfi() */
static void (*arm7s_wait)(si_t *sih) = NULL;

/* Stub. arm7tdmi-s has only one IRQ */
uint
BCMINITFN(si_irq)(si_t *sih)
{
	return 0;
}

void
hnd_cpu_wait(si_t *sih)
{
	if (arm7s_wait != NULL) {
		arm7s_wait(sih);
		return;
	}
	while (1);
}

#elif defined(__ARM_ARCH_7M__)
/*
 * Map SB cores sharing the ARM IRQ0 to virtual dedicated OS IRQs.
 * Per-port BSP code is required to provide necessary translations between
 * the shared ARM IRQ0 and the virtual OS IRQs based on SB core flag.
 *
 * See sb_irq() for the mapping.
 */
static uint shirq_map_base = 0;

/**
 * Returns the ARM IRQ assignment of the current core. On Cortex-M3
 * it's mapped 1-to-1 with the backplane flag:
 *
 *	IRQ 0 - shared by multiple cores based on isrmask register
 *	IRQ 1 to 14 <==> flag + 1
 *	IRQ 15 - serr
 */
static uint
BCMINITFN(si_getirq)(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	void *regs;
	uint flag = si_flag(sih);
	uint idx = si_coreidx(sih);
	uint irq;

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	if (R_REG(osh, ARMREG(regs, isrmask)) & (1 << flag))
		irq = ARMCM3_SHARED_INT;
	else
		irq = flag + 1;

	si_setcoreidx(sih, idx);
	return irq;
}

#ifdef __ARM_ARCH_7M__
static void
BCMATTACHFN(si_armcm3_serror_int)(si_t *sih, bool enable)
{
	osl_t *osh = si_osh(sih);
	void *regs;
	uint idx = si_coreidx(sih);
	uint32 isrmask;
	uint32 nmimask;

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	isrmask = R_REG(osh, ARMREG(regs, isrmask));
	nmimask = R_REG(osh, ARMREG(regs, nmimask));

	if (enable) {
		isrmask |= (1 << 31);
		nmimask |= (1 << 31);
		W_REG(osh, ARMREG(regs, isrmask), isrmask);
		W_REG(osh, ARMREG(regs, nmimask), nmimask);
	}
	else {
		isrmask &= ~(1 << 31);
		nmimask &= ~(1 << 31);
		W_REG(osh, ARMREG(regs, isrmask), isrmask);
		W_REG(osh, ARMREG(regs, nmimask), nmimask);
	}
	si_setcoreidx(sih, idx);
}

/**
 * Assigns the specified ARM IRQ to the specified core. Shared ARM
 * IRQ 0 may be assigned more than once. ARM IRQ is enabled after
 * the assignment.
 */
static void
BCMATTACHFN(si_setirq)(si_t *sih, uint irq, uint coreid, uint coreunit)
{
	osl_t *osh = si_osh(sih);
	void *regs;
	uint32 flag;
	uint idx = si_coreidx(sih);

	regs = si_setcore(sih, coreid, coreunit);
	ASSERT(regs);

	flag = si_flag(sih);
	ASSERT(irq == flag + 1 || irq == ARMCM3_SHARED_INT);

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	if (irq != ARMCM3_SHARED_INT) {
		uint32 isrmask = R_REG(osh, ARMREG(regs, isrmask)) & ~(1 << flag);
		if (isrmask == 0)
			disable_nvic_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
		enable_nvic_ints(ARMCM3_INT(flag + 1));
		W_REG(osh, ARMREG(regs, isrmask), isrmask);
	}
	else {
		disable_nvic_ints(ARMCM3_INT(flag + 1));
		enable_nvic_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
		OR_REG(osh, ARMREG(regs, isrmask), (1 << flag));
	}

	si_setcoreidx(sih, idx);
}
#endif /* __ARM_ARCH_7M__ */

#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
void si_set_BT_irq(si_t *sih)
{
	int flag = VUSBD_43242_BT_2_WL_INTR;
	osl_t *osh;
	void *regs;

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	osh = si_osh(sih);

	disable_nvic_ints(ARMCM3_INT(flag + 1));
	enable_nvic_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
	OR_REG(osh, ARMREG(regs, isrmask), (1 << flag));
}
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */
/**
 * Return the ARM IRQ assignment of the current core. If necessary
 * map cores sharing the ARM IRQ0 to virtual dedicated OS IRQs.
 */
uint
BCMINITFN(si_irq)(si_t *sih)
{
	uint irq = si_getirq(sih);
	if (irq == ARMCM3_SHARED_INT && 0 != shirq_map_base)
		irq = si_flag(sih) + shirq_map_base;
	return irq;
}

void
hnd_cpu_wait(si_t *sih)
{
	__asm__ __volatile__("wfi");
}
#elif defined(__ARM_ARCH_7R__)
#ifndef RTE_POLL

/**
 * Starting with D11 >= 40
 *   - D11 main int (macintmask/status) --> PCIe
 *   - D11 alternate int (altintmask[6]/altmacintmask/status) --> ARM
 * Switch to either main or alternate interrupt as OOB BUS A input to coreid
 */
#define		IRQ_SRC_MAIN	1
#define		IRQ_SRC_ALT	2


/*
 * In previous chips there is no need to reroute oob interrupts because
 * the core index matchs to oob_aout signal.However It's not the
 * case in 43909 anymore so we have to assign the oob_ain signal for
 * each core and set oobselin wrapper.
 */

#ifdef REROUTE_OOBINT
static uint
BCMATTACHFN(si_setirq_oob)(si_t *sih, uint coreid, uint coreunit)
{
	uint save_idx, core_idx;
	uint8 flag = 0, curflag, shift = 0;
	uint32 oobselina, addr = 0;
	uint8 oob_sel = 0;

	save_idx = si_coreidx(sih);
	core_idx = si_findcoreidx(sih, coreid, coreunit);
	ASSERT(core_idx != BADIDX);
	switch (coreid) {
		case CC_CORE_ID:
			oob_sel = CC_OOB;
			break;
		case M2MDMA_CORE_ID:
			oob_sel = M2MDMA_OOB;
			break;
		case PMU_CORE_ID:
			oob_sel = PMU_OOB;
			break;
		case D11_CORE_ID:
			oob_sel = D11_OOB;
			break;
		case SDIOD_CORE_ID:
			oob_sel = SDIOD_OOB;
			break;
		case ARMCR4_CORE_ID:
			oob_sel = WLAN_OOB;
			break;
		default:
			ASSERT(0);
	}
	shift = (oob_sel % 4) * 8;
	if (oob_sel < 4)
		addr = AI_OOBSELINA30;
	else
		addr = AI_OOBSELINA74;

	si_setcoreidx(sih, core_idx);
	flag = si_flag(sih);


	(void) si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	oobselina = si_wrapperreg(sih, addr, 0, 0);
	curflag = (oobselina >> shift) & AI_OOBSEL_MASK;
	if (curflag != flag)
		si_wrapperreg(sih, addr, AI_OOBSEL_MASK << shift, flag << shift);

	si_setcoreidx(sih, save_idx);
	return oob_sel;
}
#endif /* REROUTE_OOBINT */

static void
BCMATTACHFN(si_setirq_src)(si_t *sih, uint coreid, uint8 irqsrc)
{
	uint save_idx, core_idx;
	uint8 flag = 0, curflag, main, alt, shift = 0;
	uint32 oobselina, addr = 0;

	save_idx = si_coreidx(sih);
	core_idx = si_findcoreidx(sih, coreid, 0);
	ASSERT(core_idx != BADIDX);

	shift = (core_idx % 4) * 8;
	if (core_idx < 4)
		addr = AI_OOBSELINA30;
	else
		addr = AI_OOBSELINA74;

	si_setcoreidx(sih, core_idx);
	main = si_flag(sih);
	alt = si_flag_alt(sih);

	if (irqsrc == IRQ_SRC_MAIN)
		flag = main;
	else if (irqsrc == IRQ_SRC_ALT)
		flag = alt;
	else
		ASSERT(0);

	(void) si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	oobselina = si_wrapperreg(sih, addr, 0, 0);
	curflag = (oobselina >> shift) & AI_OOBSEL_MASK;
	if (curflag != flag)
		si_wrapperreg(sih, addr, AI_OOBSEL_MASK << shift, flag << shift);

	si_setcoreidx(sih, save_idx);
}

static void
BCMATTACHFN(si_setirq)(si_t *sih, uint irq, uint coreid, uint coreunit)
{
	osl_t *osh = si_osh(sih);
	void *regs;
	uint idx, core_idx;

	idx = si_coreidx(sih);
#ifndef REROUTE_OOBINT
	core_idx = si_findcoreidx(sih, coreid, coreunit);
#else
	core_idx = si_setirq_oob(sih, coreid, coreunit);
#endif /* REROUTE_OOBINT */

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);
	OR_REG(osh, ARMREG(regs, isrmask), (1 << core_idx));
	si_setcoreidx(sih, idx);
}
#endif /*  RTE_POLL */

uint
BCMINITFN(si_irq)(si_t *sih)
{
	return 0;
}

void
hnd_cpu_wait(si_t *sih)
{
	__asm__ __volatile__("wfi");
}
#elif defined(__ARM_ARCH_7A__)
#ifndef RTE_POLL
/**
 * Starting with D11 >= 40
 *   - D11 main int (macintmask/status) --> PCIe
 *   - D11 alternate int (altintmask[6]/altmacintmask/status) --> ARM
 * Switch to either main or alternate interrupt as OOB BUS A input to coreid
 */
#define		IRQ_SRC_MAIN	1
#define		IRQ_SRC_ALT	2

static void
BCMATTACHFN(si_setirq_src)(si_t *sih, uint coreid, uint8 irqsrc)
{
	osl_t *osh = si_osh(sih);
	void *regs;
	uint idx, irq;

	idx = si_coreidx(sih);
	if (irqsrc == IRQ_SRC_MAIN)
		irq = si_flag(sih);
	else if (irqsrc == IRQ_SRC_ALT)
		irq = si_flag_alt(sih);
	else
		ASSERT(0);

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);
	/* How to enable irq in GIC? */
	BCM_REFERENCE(regs);
	BCM_REFERENCE(osh);
	BCM_REFERENCE(irq);

	si_setcoreidx(sih, idx);
}

static void
BCMATTACHFN(si_setirq)(si_t *sih, uint irq, uint coreid, uint coreunit)
{
	osl_t *osh = si_osh(sih);
	void *regs;
	uint idx, oob_irq;

	idx = si_coreidx(sih);
	oob_irq = si_flag(sih);

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);
	/* How to enable irq in GIC? */
	BCM_REFERENCE(regs);
	BCM_REFERENCE(osh);
	BCM_REFERENCE(oob_irq);

	si_setcoreidx(sih, idx);
}
#endif /*  RTE_POLL */

uint
BCMINITFN(si_irq_alt)(si_t *sih)
{
	return si_flag_alt(sih);
}

uint
BCMINITFN(si_irq)(si_t *sih)
{
	return si_flag(sih);
}

void
hnd_cpu_wait(si_t *sih)
{
	__asm__ __volatile__("wfi");
}

void
hnd_hw_coherent_enable(si_t *sih)
{
#define CCI400_S3_SNOOP_CTL	0x18304000
	if (BCM4365_CHIP(sih->chip)) {
		/* Turn on CCI-400 S3 SNOOP function */
		W_REG(si_osh(sih), (uint32 *)CCI400_S3_SNOOP_CTL, 0x1);
	}
}

static void
hnd_cpu_gtimer_upd(uint32 period)
{
	uint64 count;
	uint32 ctrl;

	/* Get physical counter */
	asm volatile("isb");
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (count));

	/* Get PL1 physical timer controller register */
	asm volatile("isb");
	asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (ctrl));
	/* Set mask bit, i.e., disable interrupt */
	ctrl |= (1 << 1);
	/* Set PL1 physical timer controller register */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
	asm volatile("isb");

	count += (period / 1000000) * 32000;

	/* Set PL1 Physical Comp Value */
	asm volatile("mcrr p15, 2, %Q0, %R0, c14" : : "r" (count));
	asm volatile("isb");

	/* Get PL1 physical timer controller register */
	asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (ctrl));
	/* Clear mask bit, i.e., enable interrupt */
	ctrl &= ~(1 << 1);
	/* Set PL1 physical timer controller register */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
	asm volatile("isb");
}
#endif	/* __ARM_ARCH_7A__ */

#if defined(__ARM_ARCH_7R__)
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
#define VUSBD_4350_BTINTR_OOBSHIFT	(4 * 6)
#define VUSBD_4350_BTINTR_ISRMASKSHIFT	7
#define VUSBD_4350_ARM_CORE_BASE  0x18002000
#define VUSBD_4350_ARM_ISRMASK 0x18
void si_set_BT_irq(si_t *sih)
{

	uint idx;
	uint32 flag = 0x80 | VUSBD_4350_BT_2_WL_INTR;
	uint32 addr = AI_OOBSELINA74;
	volatile uint32 *isrmask = (void *)(VUSBD_4350_ARM_CORE_BASE + VUSBD_4350_ARM_ISRMASK);

#ifdef VUSB_4350b1
	uint32 regVal;
	/* Use the interrupt 7 for BT */
	regVal = *((volatile uint32 *)0x1810b100);
	regVal &= 0xffff00ff;
	regVal |= 0x00008700;
	*((volatile uint32 *)0x1810b100) = regVal;
#endif /* VUSB_4350b1 */

	idx = si_coreidx(sih);

	si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());

	/* set interrupt routing */
	si_wrapperreg(sih, addr, AI_OOBSEL_MASK << VUSBD_4350_BTINTR_OOBSHIFT,
	flag << VUSBD_4350_BTINTR_OOBSHIFT);
	si_setcoreidx(sih, idx);

	/* set ISRMASK */
	*isrmask |= (1 << VUSBD_4350_BTINTR_ISRMASKSHIFT);

}
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */
#endif /* __ARM_ARCH_7R__ */

static const char BCMATTACHDATA(rstr_ramstbydis)[] = "ramstbydis";
static const char BCMATTACHDATA(rstr_deadman_to)[] = "deadman_to";

/**
 * Initializes clocks and interrupts. SB and NVRAM access must be
 * initialized prior to calling.
 */
void *
BCMATTACHFN(si_arm_init)(si_t *sih)
{
	sbsocramregs_t *sr;
	osl_t *osh;
	void *armr;

#ifdef EXT_CBALL
	return NULL;
#endif	/* !EXT_CBALL */

	osh = si_osh(sih);

	/* Enable/Disable SOCRAM memory standby */
	sr = si_setcore(sih, SOCRAM_CORE_ID, 0);
	if (sr != NULL) {
		uint32 bank;
		uint32 rev;
		uint32 port_type;
		bool wasup;

		if (!(wasup = si_iscoreup(sih))) {
			printf("[DBG] SOCRAM NOT UP!!!\n");
			si_core_reset(sih, 0, 0);
		}
		bank = (R_REG(osh, &sr->coreinfo) & SRCI_SRNB_MASK) >> SRCI_SRNB_SHIFT;
		port_type = (R_REG(osh, &sr->coreinfo) & SRCI_PT_MASK) >> SRCI_PT_SHIFT;
		ASSERT(bank);

		/* SOCRAM standby is disabled by default in corerev >= 4 so
		 * enable it with a fixed standby timer value equivelant to
		 * 1.2ms of backplane clocks at 80Mhz. Use nvram variable
		 * "ramstbydis" with non-zero value to use hardware default.
		 */
		rev = si_corerev(sih);
		if ((rev >= 5) || (rev == 4 && (port_type == SRCI_PT_CM3AHB_OCP))) {
			if (getintvar(NULL, rstr_ramstbydis) == 0) {
				uint32 ctlval = ISSIM_ENAB(sih) ? 8 : 0x17fff;
				uint32 binfo;
				bool standby = FALSE;
				while (bank--) {
					W_REG(osh, &sr->bankidx, bank);
					if ((rev < 8) || (rev == 12))
						standby = TRUE;
					else {
						binfo = R_REG(osh, &sr->bankinfo);
						if (binfo & SOCRAM_BANKINFO_STDBY_MASK) {
							if (binfo & SOCRAM_BANKINFO_STDBY_TIMER)
								standby = TRUE;
						}
					}
					if (standby) {
						ctlval |=  (1 << SRSC_SBYEN_SHIFT);
						ctlval |=  (1 << SRSC_SBYOVRVAL_SHIFT);
						W_REG(osh, &sr->standbyctrl, ctlval);
					}
				}
				/* disable whole memory standby */
				AND_REG(osh, &sr->pwrctl, ~(1 << SRPC_PMU_STBYDIS_SHIFT));
			}
		}
		/* else {} */
	}

	/* Cache ARM core register base and core rev */
	armr = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(armr);

#if defined(__ARM_ARCH_7M__)

	/* Make CM3 Not Sleeping clk req to be HT. 	*/
	/* In simpler words: 				*/
	/* CM3 does not request for HT by default 	*/
	/* But the dongle code almost assume e.g., OSL_DELAY */
	/* that we are running on HT clk		*/
	/* and so we set the CM3 to request HT when it is not sleeping */


	OR_REG(osh, ARMREG(armr, corecontrol), (1 << ACC_NOTSLEEPINGCLKREQ_SHIFT));

	/* don't reset on any serror but generate an interrupt */
	OR_REG(osh, ARMREG(armr, corecontrol), (1 << 1));

#elif defined(__ARM_ARCH_7R__)
	/* don't reset on any serror but generate an interrupt */
	OR_REG(osh, ARMREG(armr, corecontrol), (1 << 1));
#endif	/* __ARM_ARCH_7M__ */

#ifndef HT_POWERSAVE
	/* Now that it's safe, allow ARM to request HT */
	W_REG(osh, ARMREG(armr, clk_ctl_st), 0);
#endif /* HT_POWERSAVE */
	if (sih->ccrev >= 40)
		OR_REG(osh, ARMREG(armr, clk_ctl_st), CCS_HQCLKREQ);

#ifndef HT_POWERSAVE
	SPINWAIT(((R_REG(osh, ARMREG(armr, clk_ctl_st)) & CCS_HTAVAIL) == 0),
	         PMU_MAX_TRANSITION_DLY);
	/* Need to assert if HT is not available by now */
	ASSERT(R_REG(osh, ARMREG(armr, clk_ctl_st)) & CCS_HTAVAIL);
#endif /* HT_POWERSAVE */

	/* Initialize CPU sleep/wait mechanism */
#if defined(__ARM_ARCH_4T__)
	arm7s_wait = arm_wfi;
#elif defined(__ARM_ARCH_7M__)

	/* No need to setup wait for this architecture */

#elif	defined(__ARM_ARCH_7R__)

	/* No need to setup wait for this architecture as this could be BOOTLOADER. */

#endif	/* ARM */

	/* Initialize interrupts/IRQs */
#if defined(__ARM_ARCH_4T__)
	W_REG(osh, ARMREG(armr, irqmask), 0xffffffff);
	W_REG(osh, ARMREG(armr, fiqmask), 0);

#elif defined(__ARM_ARCH_7M__)
	switch (CHIPID(sih->chip)) {
	case BCM4329_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		break;
	case BCM4319_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		break;
	case BCM4322_CHIP_ID:	case BCM43221_CHIP_ID:	case BCM43231_CHIP_ID:
	case BCM4342_CHIP_ID:
	case BCM43235_CHIP_ID:	case BCM43236_CHIP_ID:	case BCM43238_CHIP_ID:
	case BCM43234_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		break;
	case BCM4336_CHIP_ID:
	case BCM43362_CHIP_ID:
	case BCM43237_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		si_armcm3_serror_int(sih, TRUE);
		break;
	case BCM4314_CHIP_ID: /* Same as 4330 */
	case BCM4334_CHIP_ID: /* Same as 4330 */
	case BCM43340_CHIP_ID:
	case BCM43341_CHIP_ID:
	case BCM4330_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		if (CST4330_CHIPMODE_SDIOD(sih->chipst))
			si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		else
			si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		si_armcm3_serror_int(sih, TRUE);
		break;
	case BCM43239_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		if (CST43239_CHIPMODE_SDIOD(sih->chipst))
			si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		else
			si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		si_armcm3_serror_int(sih, TRUE);
		break;
	case BCM4324_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		if (CST4324_CHIPMODE_SDIOD(sih->chipst))
			si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		else
			si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		si_armcm3_serror_int(sih, TRUE);
		break;
	case BCM43242_CHIP_ID:
	case BCM43243_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		si_armcm3_serror_int(sih, TRUE);
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
		si_set_BT_irq(sih);
#endif
		break;
	case BCM43143_CHIP_ID:
		si_setirq(sih, ARMCM3_SHARED_INT, CC_CORE_ID, 0);
		si_setirq(sih, ARMCM3_SHARED_INT, D11_CORE_ID, 0);
		if (CST43143_CHIPMODE_SDIOD(sih->chipst))
			si_setirq(sih, ARMCM3_SHARED_INT, SDIOD_CORE_ID, 0);
		else
			si_setirq(sih, ARMCM3_SHARED_INT, USB20D_CORE_ID, 0);
		si_armcm3_serror_int(sih, TRUE);
		break;
	default:
		/* fix interrupt for the new chips! */
		ASSERT(0);
		break;
	}

#ifndef BCM_BOOTLOADER
	/* deadman timer: enable NMI for arm core interrupts */
	if (getintvar(NULL, rstr_deadman_to)) {
		void *regs;
		uint idx;
		uint32 nmimask;
		uint32 intmask;

		idx = si_coreidx(sih);
		regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
		nmimask = R_REG(osh, ARMREG(regs, nmimask));
		intmask = R_REG(osh, ARMREG(regs, intmask));

		/* enable NMI from ARM core */
		nmimask |= (1 << si_coreidx(sih));
		W_REG(osh, ARMREG(regs, nmimask), nmimask);
		/* enable IntTimer interrupts from ARM core */
		intmask |= ARMCM3_INTMASK_TIMER;
		W_REG(osh, ARMREG(regs, intmask), intmask);
		si_setcoreidx(sih, idx);
	}
#endif /* !BCM_BOOTLOADER */

#elif defined(__ARM_ARCH_7R__)
#ifndef	RTE_POLL
	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM43526_CHIP_ID:
		if (sih->chipst & CST4360_MODE_USB) {
			si_setirq(sih, 0, CC_CORE_ID, 0);
		}
		if (sih->chipst & CST4360_MODE_USB) {
			si_setirq(sih, 0, USB20D_CORE_ID, 0);
		}
		si_setirq(sih, 0, D11_CORE_ID, 0);
#ifdef BCM_OL_DEV
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, PCIE2_CORE_ID, 0);
		/* to set dot11 alt mac interrupt */
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif
		break;
		/* add chip common and d11 if necessory for arm offloading */
	case BCM43602_CHIP_ID:
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, D11_CORE_ID, 0);
		si_setirq(sih, 0, PCIE2_CORE_ID, 0);
#ifndef BCM_OL_DEV
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#else
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif
		break;
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID: {
		uint coreunit = 0;
		si_setirq(sih, 0, CC_CORE_ID, 0);
		coreunit = si_numd11coreunits(sih);
		while (0 != coreunit) {
			si_setirq(sih, 0, D11_CORE_ID, (coreunit - 1));
			coreunit = coreunit -1;
		}
		if (CST4335_CHIPMODE_SDIOD(sih->chipst))
			si_setirq(sih, 0, SDIOD_CORE_ID, 0);
		else if (
			 (CHIPID(sih->chip) == BCM4345_CHIP_ID &&
			  CST4345_CHIPMODE_USB20D(sih->chipst)) ||
			 (CHIPID(sih->chip) == BCM4335_CHIP_ID &&
			  CST4335_CHIPMODE_USB20D(sih->chipst)))
			si_setirq(sih, 0, USB20D_CORE_ID, 0);
		else if (CST4335_CHIPMODE_PCIE(sih->chipst))
			si_setirq(sih, 0, PCIE2_CORE_ID, 0);

#ifndef BCM_OL_DEV
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#else
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif

		break;
	}
	case BCM4349_CHIP_GRPID: {
		uint coreunit = 0;
		si_setirq(sih, 0, CC_CORE_ID, 0);
		coreunit = si_numd11coreunits(sih);
		while (0 != coreunit) {
			si_setirq(sih, 0, D11_CORE_ID, (coreunit - 1));
			coreunit = coreunit -1;
		}
		if (CST4349_CHIPMODE_SDIOD(sih->chipst))
			si_setirq(sih, 0, SDIOD_CORE_ID, 0);
		else if (CST4349_CHIPMODE_PCIE(sih->chipst))
			si_setirq(sih, 0, PCIE2_CORE_ID, 0);
#ifndef BCM_OL_DEV
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#else
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif
		break;
	}
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, D11_CORE_ID, 0);
#ifdef BCM4350_FPGA
		if (1)
#else
		if (CST4350_CHIPMODE_USB30D(sih->chipst) ||
		    CST4350_CHIPMODE_USB30D_WL(sih->chipst))
#endif
		{
			si_setirq(sih, 0, USB30D_CORE_ID, 0);
		}
		else if (CST4350_CHIPMODE_USB20D(sih->chipst)) {
			si_setirq(sih, 0, USB20D_CORE_ID, 0);
		}
		else if (CST4350_CHIPMODE_SDIOD(sih->chipst)) {
			si_setirq(sih, 0, SDIOD_CORE_ID, 0);
		}
		else if (CST4350_CHIPMODE_PCIE(sih->chipst)) {
			si_setirq(sih, 0, PCIE2_CORE_ID, 0);
		}
#ifndef BCM_OL_DEV
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#else
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
		si_set_BT_irq(sih);
#endif
		break;

	case BCM43909_CHIP_ID:
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, D11_CORE_ID, 0);
		si_setirq(sih, 0, PMU_CORE_ID, 0);
#ifdef BCMM2MDEV_ENABLED
		si_setirq(sih, 0, M2MDMA_CORE_ID, 0);
#else
		si_setirq(sih, 0, SDIOD_CORE_ID, 0);
#endif /* BCMM2MDEV_ENABLED */
		break;
	default:
		/* fix interrupt for the new chips! */
		ASSERT(0);
		break;
	}

#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB())
	{
		uint idx;

		/* fixup lazy interrupt issue in pcie full dongle */
		idx = si_coreidx(sih);
		si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
		si_wrapperreg(sih, AI_OOBSELINA30, ~0, 0x83828180);
		si_setcoreidx(sih, idx);
#ifdef PCIE_PHANTOM_DEV
		/* Enable sdio/usb/pcie core for phanton devices */
		si_setirq(sih, 0, SDIOD_CORE_ID, 0);
		si_setirq(sih, 0, USB20D_CORE_ID, 0);
		si_setirq(sih, 0, PCIE2_CORE_ID, 0);
#endif /* PCIE_PHANTOM_DEV */
	}
#endif /* BCMPCIEDEV */

#if !defined(BCM_BOOTLOADER) && defined(FIQMODE)
	{
	void *regs;
	uint idx;
	uint armcoreidx;
	uint32 nmimask;
	uint32 intmask;
	uint32 isrmask;

	idx = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	nmimask = R_REG(osh, ARMREG(regs, nmimask));
	intmask = R_REG(osh, ARMREG(regs, intmask));
	isrmask = R_REG(osh, ARMREG(regs, isrmask));

#ifdef REROUTE_OOBINT
	armcoreidx = si_setirq_oob(sih, ARMCR4_CORE_ID, get_armcr4_idx());
#else
	armcoreidx = si_coreidx(sih);
#endif /* REROUTE_OOBINT */
	/* enable FIQ from ARM core */
	nmimask |= (1 << armcoreidx);
	W_REG(osh, ARMREG(regs, nmimask), nmimask);
	isrmask |= (1 << armcoreidx);
	W_REG(osh, ARMREG(regs, isrmask), isrmask);

	/* enable IntTimer interrupts from ARM core */
	intmask |= ARMCR4_INTMASK_TIMER;
	W_REG(osh, ARMREG(regs, intmask), intmask);
	si_setcoreidx(sih, idx);
	}
#endif /* BCMDBG_LOADAVG && !BCM_BOOTLOADER */
#endif /* RTE_POLL */
#elif defined(__ARM_ARCH_7A__)

#ifndef	RTE_POLL
	switch (CHIPID(sih->chip)) {
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, D11_CORE_ID, 0);
		si_setirq(sih, 0, PCIE2_CORE_ID, 0);
#ifndef BCM_OL_DEV
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#else
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif
		break;
	default:
		/* fix interrupt for the new chips! */
		ASSERT(0);
		break;
	}
#endif /* RTE_POLL */

#endif	/* ARM */

#ifdef	__ARM_ARCH_7R__
	/* Enable WFIClkStop */
	OR_REG(osh, ARMREG(armr, corecontrol), (ACC_WFICLKSTOP));
#endif /* __ARM_ARCH_7R__ */

#if !defined(__ARM_ARCH_7R__) && !defined(__ARM_ARCH_7A__)
	/* Enable reset & error loggings */
	OR_REG(osh, ARMREG(armr, resetlog), (SBRESETLOG | SERRORLOG));
#endif

	return armr;
} /* si_arm_init */

/** returns [Hz] units */
uint32
BCMINITFN(si_cpu_clock)(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_cpu_clock(sih, si_osh(sih));

	return si_clock(sih);
}

/** returns [Hz] units */
uint32
BCMINITFN(si_mem_clock)(si_t *sih)
{
	if (PMUCTL_ENAB(sih))
		return si_pmu_mem_clock(sih, si_osh(sih));

	return si_clock(sih);
}

/**
 * Set or query ARM-CR4 clock ratio.
 * parameters:
 *	div: 0 to query, 1 or 2 to set.
 * return value:
 *	if div == 0:
 *		1 for standard frequency, 2 for double frequency, -1 for not capable of switching
 *	if div == 1 or div == 2:
 *		0 for no switch occurred, 1 for double->std switch, 2 for std->double switch
 */
int32
si_arm_clockratio(si_t *sih, uint8 div)
{
	int32 ret = -1;
#ifdef	__ARM_ARCH_7R__
	void *regs;
	uint idx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);
	uint32 val, clocks, isrmask;

	regs = si_setcore(sih, ARMCR4_CORE_ID, get_armcr4_idx());

	if (si_corerev(sih) < 7) {
		/* Not capable of switching, return -1 */
		HNDARM_ERROR(("Not capable of clockratio switching, old core rev\n"));
		goto done;
	}
	val = (R_REG(osh, ARMREG(regs, corecapabilities)) & ACC_CLOCKMODE_MASK)
		>> ACC_CLOCKMODE_SHIFT;
	if (val != ACC_CLOCKMODE_SYNCH) {
		/* Not capable of switching, return -1 */
		HNDARM_ERROR(("Not capable of clockratio switching, cap: %d\n", val));
		goto done;
	}

	val = (R_REG(osh, ARMREG(regs, corecontrol)) & ACC_CLOCKRATIO_MASK)
		>> ACC_CLOCKRATIO_SHIFT;
	ret = (val == ACC_CLOCKRATIO_2_TO_1) ? 2 : 1;

	if (div == 0) {
		/* No change requested */
		HNDARM_MSG(("No change requested, cur ratio %d ~ div %d\n", val, ret));
		goto done;
	} else if (div == ret) {
		/* No change required */
		HNDARM_MSG(("No change required, cur ratio %d ~ div %d\n", val, ret));
		ret = 0;
		goto done;
	} else if (div > 2) {
		/* Divisor unsupported */
		HNDARM_ERROR(("Invalid divisor value: %d, no change applied (div:%d)\n", div, ret));
		ret = -1;
		goto done;
	}

	clocks = R_REG(osh, ARMREG(regs, clk_ctl_st));
	if ((clocks & (CCS_ARMFASTCLOCKSTATUS | CCS_BP_ON_HT)) !=
		(CCS_ARMFASTCLOCKSTATUS | CCS_BP_ON_HT)) {
		/* Request HT (and Fast clock) and wait to get them */
		OR_REG(osh, ARMREG(regs, clk_ctl_st), (CCS_ARMFASTCLOCKREQ | CCS_FORCEHT));
		OSL_DELAY(65);
		SPINWAIT((R_REG(osh, ARMREG(regs, clk_ctl_st)) &
			(CCS_ARMFASTCLOCKSTATUS | CCS_BP_ON_HT)) !=
			(CCS_ARMFASTCLOCKSTATUS | CCS_BP_ON_HT), PMU_MAX_TRANSITION_DLY);
	}
	if ((R_REG(osh, ARMREG(regs, clk_ctl_st)) & (CCS_ARMFASTCLOCKSTATUS | CCS_BP_ON_HT))
		!= (CCS_ARMFASTCLOCKSTATUS | CCS_BP_ON_HT)) {
		HNDARM_ERROR(("Clocks not available, no change applied (div:%d)\n", ret));
		ret = -1;
		goto done;
	}

	/* Disable interrupt generation */
	isrmask = R_REG(osh, ARMREG(regs, isrmask));
	W_REG(osh, ARMREG(regs, isrmask), 0);

	/* Clear any pending interrupt */
	W_REG(osh, ARMREG(regs, nmiisrst), 0xffffffff);

	/* Enable clockstable interrupt */
	W_REG(osh, ARMREG(regs, isrmask), ARMCR4_INTMASK_CLOCKSTABLE);

	val = R_REG(osh, ARMREG(regs, corecontrol)) & ~ACC_CLOCKRATIO_MASK;
	if (div == 2)
		val |= (ACC_CLOCKRATIO_2_TO_1 << ACC_CLOCKRATIO_SHIFT); /* set clock 2:1 */
	W_REG(osh, ARMREG(regs, corecontrol), val); /* set clock ratio */

	/* Put ARM to sleep, wait for int */
	__asm__ __volatile__("wfi");

	val = R_REG(osh, ARMREG(regs, nmiisrst));
	HNDARM_MSG(("Interupt status after divisor change: %x\n", val));
	ASSERT(val & ARMCR4_INTMASK_CLOCKSTABLE);
	/* Clear interrupt */
	W_REG(osh, ARMREG(regs, nmiisrst), ARMCR4_INTMASK_CLOCKSTABLE);

	/* restore situation */
	W_REG(osh, ARMREG(regs, isrmask), isrmask);
	if (div == 1) {
		/* Can disable fast ARM clock after transition */
		clocks &= ~CCS_ARMFASTCLOCKREQ;
	} else {
		/* Need to keep fast ARM clock after transition */
		clocks |= CCS_ARMFASTCLOCKREQ;
	}
	W_REG(osh, ARMREG(regs, clk_ctl_st), (clocks & 0xFFFF));
	ret = div;

done:
	si_setcoreidx(sih, idx);
#endif /* __ARM_ARCH_7R__ */

	return ret;
} /* si_arm_clockratio */

/** Start chipc watchdog timer and wait till reset */
void
hnd_cpu_reset(si_t *sih)
{
	si_watchdog(sih, 1);
	while (1);
}

void
hnd_cpu_jumpto(void *addr)
{
#if defined(CONFIG_XIP) && defined(BCMTCAM)
	if (patch_pair) {
		hnd_tcam_patchdisable();
	}
#endif /* CONFIG_XIP && BCMTCAM */
	arm_jumpto(addr);
}

uint32
si_arm_sflags(si_t *sih)
{
	uint idx = si_coreidx(sih);
	uint32 sflags;

	si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	sflags = si_core_sflags(sih, 0, 0);
	si_setcoreidx(sih, idx);

	return sflags;
}

#ifndef BCM_BOOTLOADER
#if defined(BCMDBG_LOADAVG) && defined(__ARM_ARCH_7R__)
/** IntTimer generates a FIQ when it counts down to 0 */
void
hnd_cpu_loadavg_timer(si_t *sih, uint32 val)
{
	void *regs;
	uint idx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	W_REG(osh, ARMREG(regs, intstatus), ARMCR4_INTMASK_TIMER);
	W_REG(osh, ARMREG(regs, inttimer), val);
	si_setcoreidx(sih, idx);
}
#else
/** deadman timer: generates an interrupt if the timer expires */
void
hnd_cpu_deadman_timer(si_t *sih, uint32 val)
{
#ifndef EXT_CBALL
	void *regs;
	uint idx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);

#ifdef __ARM_ARCH_7M__
	/* CM3 - NMI interrupt */
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	W_REG(osh, ARMREG(regs, inttimer), val);
	si_setcoreidx(sih, idx);
#elif defined(__ARM_ARCH_7A__)
	BCM_REFERENCE(regs);
	BCM_REFERENCE(idx);
	BCM_REFERENCE(osh);
	hnd_cpu_gtimer_upd(val);
#else
	/* CR4 - FIQ interrupt */
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	W_REG(osh, ARMREG(regs, intstatus), 1);
	W_REG(osh, ARMREG(regs, inttimer), val);
	si_setcoreidx(sih, idx);
#endif /* __ARM_ARCH_7M__ */
#endif /* EXT_CBALL */
}
#endif /* BCMDBG_LOADAVG && __ARM_ARCH_7R__ */
#endif /* BCM_BOOTLOADER */

#ifdef BCMOVLHW
int
si_arm_ovl_remap(si_t *sih, void *virt, void *phys, uint size)
{
	osl_t *osh;
	void *regs;
	uint save;
	uint32 val;
	int idx, i;
	int err = BCME_OK;

	if (size > ARMCM3_OVL_SZ_64KB || si_arm_ovl_vaildaddr(sih, virt)) {
		return BCME_BADARG;
	}

	osh = si_osh(sih);
	save = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	if (si_corerev(sih) < 5) {
		err = BCME_UNSUPPORTED;
		goto exit;
	}

	/* Find free overlay */
	for (i = 0; i < ARMCM3_OVL_MAX; i++) {
		W_REG(osh, ARMREG(regs, ovlidx), i);

		if (!(R_REG(osh, ARMREG(regs, ovlmatch)) & ARMCM3_OVL_VALID)) {
			idx = i;
			break;
		}
	}

	if (i == ARMCM3_OVL_MAX) {
		err = BCME_ERROR;
		goto exit;
	}

	W_REG(osh, ARMREG(regs, ovlidx), idx);

	/* Update physical address */
	val = (((uint32) phys) & ARMCM3_OVL_ADDR_MASK) | (size << ARMCM3_OVL_SZ_SHIFT);
	W_REG(osh, ARMREG(regs, ovladdr), val);

	/* Update virtual address and enable it w/ valid bit */
	val = (((uint32) virt) & ARMCM3_OVL_ADDR_MASK) | (size << ARMCM3_OVL_SZ_SHIFT) |
		ARMCM3_OVL_VALID;
	W_REG(osh, ARMREG(regs, ovlmatch), val);

exit:
	si_setcoreidx(sih, save);
	return err;
}


bool
si_arm_ovl_vaildaddr(si_t *sih, void *virt)
{
	osl_t *osh;
	void *regs;
	bool act = FALSE;
	uint save, i;
	uint32 reg, size;

	osh = si_osh(sih);
	save = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	if (si_corerev(sih) < 5)
		goto exit;

	for (i = 0; i < ARMCM3_OVL_MAX; i++) {
		W_REG(osh, ARMREG(regs, ovlidx), i);

		reg = R_REG(osh, ARMREG(regs, ovlmatch));
		if (reg & ARMCM3_OVL_VALID) {
			size = ((reg & ARMCM3_OVL_SZ_MASK) >> ARMCM3_OVL_SZ_SHIFT);
			reg &= ARMCM3_OVL_ADDR_MASK;
			if ((uint32)virt >= reg && (uint32)virt < reg + (0x200 << size)) {
				act = TRUE;
				break;
			}
		}
	}

exit:
	si_setcoreidx(sih, save);
	return act;
}

/*
 * TRUE: HW overlay
 * FALSE: Otherwise
 */
bool
si_arm_ovl_int(si_t *sih, uint32 pc)
{
	/* FIX: TODO */
	return FALSE;
}

bool
si_arm_ovl_isenab(si_t *sih)
{
	osl_t *osh;
	void *regs;
	bool act = FALSE;
	uint save, i;

	osh = si_osh(sih);
	save = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	if (si_corerev(sih) < 5)
		goto exit;

	for (i = 0; i < ARMCM3_OVL_MAX; i++) {
		W_REG(osh, ARMREG(regs, ovlidx), i);

		if (R_REG(osh, ARMREG(regs, ovlmatch)) & ARMCM3_OVL_VALID) {
			act = TRUE;
			break;
		}
	}

exit:
	si_setcoreidx(sih, save);
	return act;
}


int
si_arm_ovl_reset(si_t *sih)
{
	osl_t *osh;
	void *regs;
	uint save, i;
	int err = BCME_OK;

	osh = si_osh(sih);
	save = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	if (si_corerev(sih) < 5) {
		err = BCME_UNSUPPORTED;
		goto exit;
	}

	for (i = 0; i < ARMCM3_OVL_MAX; i++) {
		W_REG(osh, ARMREG(regs, ovlidx), i);
		AND_REG(osh, ARMREG(regs, ovlmatch), ~ARMCM3_OVL_VALID);
	}

exit:
	si_setcoreidx(sih, save);
	return err;
}
#endif /* BCMOVLHW */

/* routine to force clock ctl to request HT when deep sleep disabled */
uint32
si_arm_disable_deepsleep(si_t *sih, bool disable)
{
	osl_t *osh = si_osh(sih);
	uint save = si_coreidx(sih);
	void *regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	uint32 clk_ctl_st;

	if (disable)
		OR_REG(osh, ARMREG(regs, clk_ctl_st), CCS_HTAREQ);
	else
		AND_REG(osh, ARMREG(regs, clk_ctl_st), ~CCS_HTAREQ);
	clk_ctl_st = R_REG(osh, ARMREG(regs, clk_ctl_st));

	si_setcoreidx(sih, save);
	return clk_ctl_st;
}
