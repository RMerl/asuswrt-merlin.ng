/*
 * BCM43XX ARM core routines. Contains functions that are ARM, but not RTOS specific (so ideally no
 * RTE specific constructs should be used in this file).
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
 * $Id: hndarm.c 779771 2019-10-07 11:26:40Z $
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
#endif // endif

#if defined(CONFIG_XIP) && defined(BCMTCAM)
extern uint8 patch_pair;
#endif /* CONFIG_XIP && BCMTCAM */

#ifdef __ARM_ARCH_7R__
extern char _ram_mpu_region_start[];
extern char _ram_mpu_region_end[];
extern uint32 _armcr4_idx;
#else
static uint32 _armcr4_idx = 0;
#endif // endif
static uint32 get_armcr4_idx(void);

#ifdef BCMDBG_ERR
#define	HNDARM_ERROR(args)	printf args
#else
#define	HNDARM_ERROR(args)
#endif	/* BCMDBG_ERR */

#ifdef BCMDBG
#define	HNDARM_MSG(args)	printf args
#else
#define	HNDARM_MSG(args)
#endif	/* BCMDBG */

#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
#define VUSBD_43242_BT_2_WL_INTR	5
#ifdef VUSB_4350b1
#define VUSBD_4350_BT_2_WL_INTR		7
#else
#define VUSBD_4350_BT_2_WL_INTR		11
#endif // endif

void si_set_BT_irq(si_t *sih);
#endif /* BCMUSBDEV_COMPOSITE && BCM_VUSBD */

uint32
BCMRAMFN(get_armcr4_idx)(void)
{
	return _armcr4_idx;
}

#if defined(__ARM_ARCH_7M__)
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
	volatile void *regs;
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
	volatile void *regs;
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
	volatile void *regs;
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
	volatile void *regs;

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

#define		IRQ_SRC_MAIN	1
#define		IRQ_SRC_ALT	2

/**
 * Starting with D11 >= 40
 *   - D11 main int (macintmask/status) --> PCIe
 *   - D11 alternate int (altintmask[6]/altmacintmask/status) --> ARM
 * Switch to either main or alternate interrupt as OOB BUS A input to coreid
 */
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
	volatile void *regs;
	uint idx, core_idx;

	idx = si_coreidx(sih);
	core_idx = si_findcoreidx(sih, coreid, coreunit);

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

#define		IRQ_SRC_MAIN	1
#define		IRQ_SRC_ALT	2

/**
 * Starting with D11 >= 40
 *   - D11 main int (macintmask/status) --> PCIe
 *   - D11 alternate int (altintmask[6]/altmacintmask/status) --> ARM
 * Switch to either main or alternate interrupt as OOB BUS A input to coreid
 */
static void
BCMATTACHFN(si_setirq_src)(si_t *sih, uint coreid, uint8 irqsrc)
{
	osl_t *osh = si_osh(sih);
	volatile void *regs;
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
	volatile void *regs;
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
#define CCI400_S3_SNOOP_CTL	0x304000
#define CCI500_S1_SNOOP_CTL	0x602000
	if (BCM4365_CHIP(sih->chip) ||
	   BCM43684_CHIP(sih->chip)) {
		/* Turn on CCI-400 S3 SNOOP function */
		W_REG(si_osh(sih), (uint32 *)(SI_ENUM_BASE(sih) + CCI400_S3_SNOOP_CTL), 0x1);
	} else if (BCM6715_CHIP(sih->chip)) {
		/* Turn on CCI-500 S1 SNOOP function */
		W_REG(si_osh(sih), (uint32 *)(SI_ENUM_BASE(sih) + CCI500_S1_SNOOP_CTL), 0x1);
	}
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

#if defined(__ARM_ARCH_7R__)
/* set the standby timer value of ROM and RAM, -1: don't set */
static void
si_set_mem_stby_timer(si_t *sih, int rom_timer, int ram_timer)
{
	osl_t *osh = si_osh(sih);
	uint32 corecap;
	uint32 nab = 0;
	uint32 nbb = 0;
	uint32 totb = 0;
	uint32 idx = 0;
	uint32 nrb = 0;
	uint32 val;
	uint32 bxinfo = 0;
	volatile void *regs;
	uint coreidx;

	coreidx = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	corecap = R_REG(osh, ARMREG(regs, corecapabilities));

	nrb = (corecap & ARMCR4_ROMNB_MASK) >> ARMCR4_ROMNB_SHIFT;
	nab = (corecap & ARMCR4_TCBANB_MASK) >> ARMCR4_TCBANB_SHIFT;
	nbb = (corecap & ARMCR4_TCBBNB_MASK) >> ARMCR4_TCBBNB_SHIFT;
	totb = nab + nbb;

	/* ROM */
	for (idx = 0; idx < nrb; idx++) {
		W_REG(osh, ARMREG(regs, bankidx), idx | ARMCR4_MT_ROM);

		bxinfo = R_REG(osh, ARMREG(regs, bankinfo));
		if (bxinfo & (ARMCR4_STBY_SUPPORTED | ARMCR4_STBY_TIMER_PRESENT)) {
			if (rom_timer != -1) {
				val = ((rom_timer & ARMCR4_TIMER_VAL_MASK) |
					ARMCR4_STBY_TIMER_ENABLE) & ~ARMCR4_STBY_OVERRIDE;
			} else {
				val = 0;
			}
			W_REG(osh, ARMREG(regs, bankstbyctl), val);
		}
	}

	/* RAM */
	for (idx = 0; idx < totb; idx++) {
		W_REG(osh, ARMREG(regs, bankidx), idx | ARMCR4_MT_RAM);

		bxinfo = R_REG(osh, ARMREG(regs, bankinfo));
		if (bxinfo & (ARMCR4_STBY_SUPPORTED | ARMCR4_STBY_TIMER_PRESENT)) {
			if (ram_timer != -1) {
				val = ((ram_timer & ARMCR4_TIMER_VAL_MASK) |
					ARMCR4_STBY_TIMER_ENABLE) & ~ARMCR4_STBY_OVERRIDE;
			} else {
				val = 0;
			}
			W_REG(osh, ARMREG(regs, bankstbyctl), val);
		}
	}

	si_setcoreidx(sih, coreidx);
}

static void
si_enable_arm_clkgating(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	volatile void *regs;
	uint idx;

	idx = si_coreidx(sih);
	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

	OR_REG(osh, ARMREG(regs, powerctl), 1 << ARM_ENAB_MEM_CLK_GATE_SHIFT);

	si_setcoreidx(sih, idx);
}
#endif /* __ARM_ARCH_7R__ */

/**
 * Initializes clocks and interrupts. SB and NVRAM access must be
 * initialized prior to calling.
 */
volatile void *
BCMATTACHFN(si_arm_init)(si_t *sih)
{
	sbsocramregs_t *sr;
	osl_t *osh;
	volatile void *armr;

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
		/* XXX According to h/w team PR50125 does not exist in socsram_3abh_ocp
		 * design that is used in 4322 hence we leave it enabled. It uses rev 3.
		 */
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

	/* PR64984 :WAR, make sure that HT is available before usb core
	 * reset. Usb core needs HT to be present before its core reset so
	 * that usb phy gets a clean reset.
	 */

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
	if (CCREV(sih->ccrev) >= 40)
		OR_REG(osh, ARMREG(armr, clk_ctl_st), CCS_HQCLKREQ);

#ifndef HT_POWERSAVE
	SPINWAIT(((R_REG(osh, ARMREG(armr, clk_ctl_st)) & CCS_HTAVAIL) == 0),
	         PMU_MAX_TRANSITION_DLY);
	/* Need to assert if HT is not available by now */
	ASSERT(R_REG(osh, ARMREG(armr, clk_ctl_st)) & CCS_HTAVAIL);
#endif /* HT_POWERSAVE */

	/* Initialize CPU sleep/wait mechanism */
#if defined(__ARM_ARCH_7M__)

	/* No need to setup wait for this architecture */

#elif	defined(__ARM_ARCH_7R__)

	/* No need to setup wait for this architecture as this could be BOOTLOADER. */

#endif	/* ARM */

#if defined(__ARM_ARCH_7R__)
	if (ARM_CLKGATING_ENAB(sih)) {
		si_enable_arm_clkgating(sih);
	}

	if (BCM4347_CHIP(sih->chip)) {
		si_set_mem_stby_timer(sih, ROM_STBY_TIMER_4347, RAM_STBY_TIMER_4347);
	}
#endif /* __ARM_ARCH_7R__ */

#if defined(__ARM_ARCH_7M__)
	switch (CHIPID(sih->chip)) {
	default:
		/* fix interrupt for the new chips! */
		ASSERT(0);
		break;
	}

#if !defined(BCM_BOOTLOADER)
	hnd_arm_inttimer_init(sih);
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
		break;
		/* add chip common and d11 if necessory for arm offloading */
	CASE_BCM43602_CHIP:
		/* XXX: Assuming CR4 role alone, need to restructure to fit CM3 role
		 */
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, D11_CORE_ID, 0);
		si_setirq(sih, 0, PCIE2_CORE_ID, 0);
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
		break;

	case BCM4347_CHIP_GRPID: {
		uint coreunit = 0;
		si_setirq(sih, 0, CC_CORE_ID, 0);
		coreunit = si_numd11coreunits(sih);
		while (0 != coreunit) {
			si_setirq(sih, 0, D11_CORE_ID, (coreunit - 1));
			coreunit = coreunit -1;
		}

		if (CST4347_CHIPMODE_SDIOD(sih->chipst)) {
			si_setirq(sih, 0, SDIOD_CORE_ID, 0);
		} else if (CST4347_CHIPMODE_PCIE(sih->chipst)) {
			si_setirq(sih, 0, PCIE2_CORE_ID, 0);
		} else {
			/* Only SDIO or PCIe supported */
			ASSERT(0);
		}
		break;
	}
#ifdef UNRELEASEDCHIP
	case BCM4369_CHIP_GRPID: {
		uint coreunit = 0;
		si_setirq(sih, 0, CC_CORE_ID, 0);
		coreunit = si_numd11coreunits(sih);
		while (0 != coreunit) {
			si_setirq(sih, 0, D11_CORE_ID, (coreunit - 1));
			coreunit = coreunit -1;
		}

		if (CST4369_CHIPMODE_PCIE(sih->chipst)) {
			si_setirq(sih, 0, PCIE2_CORE_ID, 0);
		} else {
			/* Only SDIO or PCIe supported */
			ASSERT(0);
		}

#ifndef BCM_OL_DEV
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#else
		si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_ALT);
#endif // endif
		break;
	}
#endif /* UNRELEASEDCHIP */

	case BCM43570_CHIP_ID:
		si_setirq(sih, 0, CC_CORE_ID, 0);
		si_setirq(sih, 0, D11_CORE_ID, 0);
#ifdef BCM4350_FPGA
		if (1)
#else
		if (CST4350_CHIPMODE_USB30D(sih->chipst) ||
		    CST4350_CHIPMODE_USB30D_WL(sih->chipst))
#endif // endif
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
		if (BUSTYPE(sih->bustype) == SI_BUS)
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
#if defined(BCMUSBDEV_COMPOSITE) && defined(BCM_VUSBD)
		si_set_BT_irq(sih);
#endif // endif
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
	}
#endif /* BCMPCIEDEV */

#if !defined(BCM_BOOTLOADER)
	hnd_arm_inttimer_init(sih);
#endif /* !BCM_BOOTLOADER */

#endif /* RTE_POLL */
#elif defined(__ARM_ARCH_7A__)

#ifndef	RTE_POLL
	switch (CHIPID(sih->chip)) {
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	CASE_BCM43684_CHIP:
	CASE_BCM6715_CHIP:
		si_setirq(sih, 0, CC_CORE_ID, 0); /* assigns CC IRQ to ARM IRQ 0 */
		si_setirq(sih, 0, D11_CORE_ID, 0);
		si_setirq(sih, 0, PCIE2_CORE_ID, 0);
		if (BUSTYPE(sih->bustype) == SI_BUS) {
			/* d11 core has both main interrupt and alt interrupt */
			si_setirq_src(sih, D11_CORE_ID, IRQ_SRC_MAIN);
		}
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

	/* Allow arm to req ALP clock when not sleeping. This is only supported
	 * on chips where the BP and CPU clocks are asynchronous.
	 */
	if (((R_REG(osh, ARMREG(armr, corecapabilities)) & ACC_CLOCKMODE_MASK)
	     >> ACC_CLOCKMODE_SHIFT) == ACC_CLOCKMODE_ASYNC) {
		AND_REG(osh, ARMREG(armr, corecontrol), ~(ACC_REQALP));
	}
#endif /* __ARM_ARCH_7R__ */

#if !defined(__ARM_ARCH_7R__) && !defined(__ARM_ARCH_7A__)
	/* Enable reset & error loggings */
	OR_REG(osh, ARMREG(armr, resetlog), (SBRESETLOG | SERRORLOG));
#endif // endif

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
 * Set or query ARM-CR4 clock ratio. ARM-CR4 cannot be put in low power ('wfi') state when ARM clock
 * is at double frequency.
 *
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
	volatile void *regs;
	uint idx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);
	uint32 val, clocks, isrmask;

	regs = si_setcore(sih, ARMCR4_CORE_ID, get_armcr4_idx());
	ASSERT(regs);

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

	/* Put ARM to sleep, wait for 'clock stable' interrupt  */
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

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)

/**
 * Initialize the ARM Interrupt Timer.
 *
 * Configure the ARM CR4/CM3 shim IntTimer to generate a FIQ/NMI when reaching zero.
 *
 * @param sih		SI handle.
 */

void
hnd_arm_inttimer_init(si_t *sih)
{
	volatile void *regs;
	uint idx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);
	uint32 nmimask;

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs != NULL);

	/* Enable the propagation of interrupts onto the ARM FIQ (CR4) or INTNMI (M3) input */
	nmimask = R_REG(osh, ARMREG(regs, nmimask));
	nmimask |= (1 << si_coreidx(sih));
	W_REG(osh, ARMREG(regs, nmimask), nmimask);

	si_setcoreidx(sih, idx);
}

/**
 * Set the ARM Interrupt Timer value.
 *
 * The IntTimer register decrements on every core clock. An interrupt will be generated
 * when the counter decrements from 1 to 0. Setting the value to zero will disable the timer.
 *
 * @note Timer will not decrement during WFI (idle) state.
 *
 * @param sih		SI handle.
 * @param ticks		Timeout in ticks, 0 to disable.
 */

void
hnd_arm_inttimer_set_timer(si_t *sih, uint32 ticks)
{
	volatile void *regs;
	uint idx = si_coreidx(sih);
	osl_t *osh = si_osh(sih);
	uint32	intmask;

	/* This assumes the following to support both CR4 and CM3:
	 * ARMCR4_INTMASK_TIMER == ARMCM3_INTMASK_TIMER
	 * ARMCR4_INTSTATUS_TIMER == ARMCM3_INTSTATUS_TIMER
	 */

	regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	ASSERT(regs != NULL);

	/* Disable timer interrupt and reset timer value */
	intmask = R_REG(osh, ARMREG(regs, intmask));
	if (intmask & ARMCR4_INTMASK_TIMER) {
		W_REG(osh, ARMREG(regs, intmask), (intmask & ~ARMCR4_INTMASK_TIMER));
		W_REG(osh, ARMREG(regs, inttimer), 0);
	}

	/* Reset interrupt status */
	W_REG(osh, ARMREG(regs, intstatus), ARMCR4_INTSTATUS_TIMER);
	if (ticks != 0) {
		/* Enable timer interrupt and write new timer value */
		W_REG(osh, ARMREG(regs, intmask), (intmask | ARMCR4_INTMASK_TIMER));
		W_REG(osh, ARMREG(regs, inttimer), ticks);
	}

	si_setcoreidx(sih, idx);
}

/**
 * Acknowledge/disable the IntTimer.
 *
 * @param sih		SI handle.
 */

inline void
hnd_arm_inttimer_ack_timer(si_t *sih)
{
	/* Disable timer interrupt and reset interrupt status */
	hnd_arm_inttimer_set_timer(sih, 0);
}

#endif /* __ARM_ARCH_7R__ || __ARM_ARCH_7M__ */

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

bool
si_arm_deepsleep_disabled(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	uint save = si_coreidx(sih);
	volatile void *regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	uint32 clk_ctl_st;
	bool ret_val;

	ASSERT(regs);
	clk_ctl_st = R_REG(osh, ARMREG(regs, clk_ctl_st));
	if ((clk_ctl_st & CCS_HTAVAIL) == CCS_HTAVAIL) {
		ret_val = TRUE;
	} else {
		ret_val = FALSE;
	}

	si_setcoreidx(sih, save);
	return ret_val;
}

/* routine to force clock ctl to request HT when deep sleep disabled */
uint32
si_arm_disable_deepsleep(si_t *sih, bool disable)
{
	osl_t *osh = si_osh(sih);
	uint save = si_coreidx(sih);
	volatile void *regs = si_setcore(sih, ARM_CORE_ID, get_armcr4_idx());
	uint32 clk_ctl_st;

	ASSERT(regs);
	if (disable)
	{
		OR_REG(osh, ARMREG(regs, clk_ctl_st), CCS_HTAREQ);
		SPINWAIT((((R_REG(osh, ARMREG(regs, clk_ctl_st))) &
			CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
	}
	else
		AND_REG(osh, ARMREG(regs, clk_ctl_st), ~CCS_HTAREQ);

	clk_ctl_st = R_REG(osh, ARMREG(regs, clk_ctl_st));
	if (disable && ((clk_ctl_st & CCS_HTAVAIL) == 0)) {
		HNDARM_ERROR(("%s: Clock not available, clk_ctl_st=0x%x\n",
			__FUNCTION__, clk_ctl_st));
	}

	si_setcoreidx(sih, save);
	return clk_ctl_st;
}

#ifdef	__ARM_ARCH_7R__
#if defined(MPU_RAM_PROTECT_ENABLED)
/*
*
 * cr4_mpu_set_region is calling "mpu_set_region" the assembly
 * implementation of the MPU region set-up
*
*/
extern void mpu_set_region(uint32 region, uint32 base_address, uint32 size, uint32 control);
extern void disable_mpu_region(uint32 region);

typedef struct mpu_regions_info {
	uint32 base_address;
	uint32 length;
	uint16  ap_val;
	uint16  subregion;
} mpu_regions_info_t;

static mpu_regions_info_t mpu_regions[MAX_MPU_REGION];

static mpu_regions_info_t *
BCMRAMFN(mpu_regions_get)(void)
{
	return (mpu_regions);
}

/* calculate start and size for mpu region */
/* end is 1 byte above the end of the region we would like to protect. */
int cr4_calculate_mpu_region(uint32 start, uint32 end, uint32 *p_align_start,
	uint32 *pindex)
{
	uint32 index = 0;
	uint32 align_start;
	uint32 size;

	/* invalid start and end */
	if (start > end) {
		return BCME_ERROR;
	}

	/* bad return pointers */
	if ((p_align_start == NULL) || (pindex == NULL)) {
		return BCME_ERROR;
	}

	size = end - start;

	/* find  2^x that is the closest to the size */
	/* mpu region size have to be 2^x */
	while ((size >>= 1) != 0) {
		index++;
	}

	/* start address have to be align to MPU region size */
	align_start = ALIGN_SIZE(start, (1 << index));

	/* If we cannot find an aligned 2^x area to protect */
	/* in the region, we can definitely find */
	/* aligned 2^(x-1) area */
	if ((align_start  + (1<<index)) > end) {
		index--;
		align_start = ALIGN_SIZE(start, (1 << index));
	}

	(*p_align_start) = align_start;
	(*pindex) = index;
	return BCME_OK;
}

/*
 *
 * the function will initialize 1 of the 8 MPU regions with the following
 * parameters. Please check the "Cortex-R4" reference manual to
 * Understand the parameters.
 * region - choose which MPU region we would like to change. Regions 0 - 4
 *		already have MPU default values in init
 *	base address - base address for the region we would like to change. The
 *		base address has to be aligned with the region size
 *	size_index - index to determine region size region_size = 2^(size_index+1)
 *	control - the control register is a logical or of the following values
 *	C,B bits define the inner cache policy
 *		00 - non Cacheable
 *		01 - Write-back, write-allocate
 *		10 - Write-through, no write-allocate
 *		11 - Write-back, no write-allocate
 *	TEX - refer to the reference manual
 *	AP defines if the memory is protected.
 *		000 All accesses generate a permission fault
 *		011 Full accesses
 *		110 Privileged/User read-only
 *	for other settings refer to reference manual
 *	XN Execute never. Determines if a region of memory is executableun:
 *		0 = all instruction fetches enabled
 *		1 = no instruction fetches enabled
 *
*/
int cr4_mpu_set_region(uint32 region, uint32 base_address, uint32 size_index,
	uint32 control, uint32 subregion)
{
	mpu_regions_info_t *mpu_regions_ptr = mpu_regions_get();

	/* the mpu have 8 regions 0 -7 */
	if (region >= MAX_MPU_REGION) {
		return BCME_ERROR;
	}

	/* region size is between 2^(4+1) and 2^(0x1f+1) */
	if (size_index < 4 || size_index > 0x1f) {
		return BCME_ERROR;
	}

	/* base address have to be aligned to 2^(size_index+1) */
	if (base_address && (base_address & ((1 << (size_index+1))-1))) {
		return BCME_ERROR;
	}

	mpu_regions_ptr[region].base_address = base_address;
	mpu_regions_ptr[region].length = size_index;
	mpu_regions_ptr[region].ap_val = (control & AP_VAL_MASK) >> AP_VAL_SHIFT;
	mpu_regions_ptr[region].subregion = subregion >> 8;

	mpu_set_region(region, base_address, subregion | size_index<<1, control);

	return BCME_OK;

}

void disable_rodata_mpu_protection(void)
{
	uint32 i;

	/* Disable MPU protection only for rodata */
	for (i = LOWER_RO_RAM_CODE_MPU_REGION; i < MAX_MPU_REGION; i++) {
		disable_mpu_region(i);
	}
}

void disable_mpu_protection(bool disable)
{
	if (disable) {
		/* Have regions 0 with all access enabled */
		cr4_mpu_set_region(0, 0, RS_VAL_4GB,
			AP_VAL_011 | TEX_VAL_000 | S_BIT_ON | B_BIT_ON, 0);
	} else {
		/* Have regions 0 with all access disabled */
		cr4_mpu_set_region(0, 0, RS_VAL_4GB,
			AP_VAL_000 | TEX_VAL_000 | S_BIT_ON | B_BIT_ON, 0);
	}
}

/* for a given region that start in start_addess and end in end_address */
/* we try to find the biggest area we can protect with the numnber of mpu */
/* reginion that are available */
/* if we have 3 MPU regions available we will protect: */
/* 1. the biggest region possible between start_addess and end_address. */
/* 2. the biggest region possible between start_address and region 1 start address. */
/* 3. the biggest resion possible between region 1 end address and end address. */
/* if we have less then 3 regions available we will assign them to 1 - 3  */
/* when we calculate the biggest region we can protect we use the funciton */
/* "cr4_calculate_mpu_region" this funciton will return the base addres that is */
/* allign to the size we can protect */
void
mpu_protect_best_fit(uint32 mpu_region_start, uint32 mpu_region_end,
	uint32 start_addess, uint32 end_address)
{
	uint32	next_start_address;
	uint32	first_region_allign_start_address;
	uint32	region_size_index;
	uint32	region_start_align;
	uint32	next_mpu_region = mpu_region_start;

	if (next_mpu_region > mpu_region_end)
		return;

	if (cr4_calculate_mpu_region(start_addess, end_address, &region_start_align,
		&region_size_index) != BCME_OK)
		return;

	first_region_allign_start_address = region_start_align;

	/* set mpu write protection to address maximum size of code area */
	if (cr4_mpu_set_region(next_mpu_region, region_start_align,
			region_size_index - 1, AP_VAL_110| TEX_VAL_110 | C_BIT_ON, 0) != BCME_OK)
		return;

	next_mpu_region++;
	if (next_mpu_region > mpu_region_end)
		return;

	next_start_address = region_start_align + (1 << region_size_index);
	if (cr4_calculate_mpu_region(next_start_address, end_address, &region_start_align,
			&region_size_index) != BCME_OK)
		return;

	/* set mpu write protection to address maximum size of code area */
	if (cr4_mpu_set_region(next_mpu_region, region_start_align,
			region_size_index - 1, AP_VAL_110| TEX_VAL_110| C_BIT_ON, 0) != BCME_OK)
		return;

	next_mpu_region++;
	if (next_mpu_region > mpu_region_end)
		return;

	if (cr4_calculate_mpu_region(start_addess, first_region_allign_start_address,
		&region_start_align, &region_size_index) != BCME_OK)
	return;

	/* set mpu write protection to address maximum size of code area */
	if (cr4_mpu_set_region(next_mpu_region, region_start_align,
			region_size_index - 1, AP_VAL_110| TEX_VAL_110| C_BIT_ON, 0) != BCME_OK)
		return;
	next_mpu_region++;
}

/* Fill up the structures according to the MPU settings done in assembly */
void
cr4_mpu_get_assembly_region_addresses(uint32 *rom_mpu_end, uint32 *ram_mpu_end)
{
	mpu_regions_info_t *mpu_regions_ptr = mpu_regions_get();

	if (mpu_regions_ptr[0].length != 0 || mpu_regions_ptr[1].length != 0 ||
		mpu_regions_ptr[2].length != 0 || mpu_regions_ptr[3].length != 0) {
		return;
	}
	/* Following regions are programmed in assembly */
	mpu_regions_ptr[0].base_address = 0;
	mpu_regions_ptr[0].length = RS_VAL_4GB;
	mpu_regions_ptr[0].ap_val = AP_VAL_000 >> AP_VAL_SHIFT;
	mpu_regions_ptr[0].subregion = 0;

	mpu_regions_ptr[1].base_address = 0;
	mpu_regions_ptr[1].length = RS_VAL_4MB;
	mpu_regions_ptr[1].ap_val = AP_VAL_011 >> AP_VAL_SHIFT;
	mpu_regions_ptr[1].subregion = (SUBR_VAL_7 | SUBR_VAL_8) >> 8;

	mpu_regions_ptr[2].base_address = 0;
	mpu_regions_ptr[2].length = RS_VAL_2MB;
	mpu_regions_ptr[2].ap_val = AP_VAL_110 >> AP_VAL_SHIFT;
	mpu_regions_ptr[2].subregion = (SUBR_VAL_6 | SUBR_VAL_7 | SUBR_VAL_8) >> 8;

	mpu_regions_ptr[3].base_address = 0x18000000;
	mpu_regions_ptr[3].length = RS_VAL_128MB;
	mpu_regions_ptr[3].ap_val = AP_VAL_011 >> AP_VAL_SHIFT;
	mpu_regions_ptr[3].subregion = (SUBR_VAL_6 | SUBR_VAL_7 | SUBR_VAL_8) >> 8;

	mpu_regions_ptr[4].base_address = 0xE8000000;
	/**
	* Base Address % Length needs to be 0 and this value covers both
	* 0xE800XXXX and 0xE880XXXX in one MPU region
	*/
	mpu_regions_ptr[4].length = RS_VAL_128MB;
	mpu_regions_ptr[4].ap_val = AP_VAL_011 >> AP_VAL_SHIFT;
	mpu_regions_ptr[4].subregion = 0;

	*rom_mpu_end = ROM_CODE_MPU_END_ASSEMBLY;
	*ram_mpu_end = RAM_CODE_MPU_END_ASSEMBLY;
}

void dump_mpu_regions(void)
{
	uint i;

	mpu_regions_info_t *mpu_regions_ptr = mpu_regions_get();

#if defined(RAMBASE)
	printf("Base:0x%x, RAM end:0x%x, RAM Code Start:0x%p, RAM Code End:0x%p\n",
		MEMBASE, MEMBASE+RAMSIZE, _ram_mpu_region_start, _ram_mpu_region_end);
#endif /* RAMBASE */

	for (i = 0; i < MAX_MPU_REGION; i++)
	{
		printf("region: %01d base_address: %08x length:"
			" %08x ap_val: %02x subregions: %02x\n",
			i,
			mpu_regions_ptr[i].base_address,
			mpu_regions_ptr[i].length,
			mpu_regions_ptr[i].ap_val,
			mpu_regions_ptr[i].subregion);
	}
}

#endif  /* MPU_RAM_PROTECT_ENABLED */
#endif	/* __ARM_ARCH_7R__ */
