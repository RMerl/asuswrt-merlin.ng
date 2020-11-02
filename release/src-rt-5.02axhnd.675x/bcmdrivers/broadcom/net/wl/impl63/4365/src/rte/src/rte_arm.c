/** @file hnd_rte_arm.c
 *
 * HND RTE support routines for ARM CPU.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_arm.c 706682 2017-06-22 11:58:40Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndcpu.h>
#include <hndarm.h>
#include <sbchipc.h>
#include "rte_pmu_priv.h"
#include <bcmpcie.h>
#include <bcmsdpcm.h>
#include <rte_cons.h>
#include <rte_trap.h>
#include <rte.h>
#include "rte_priv.h"
#include <rte_mem.h>
#include "rte_mem_priv.h"
#include <rte_dev.h>
#include <rte_timer.h>
#include <pcie_core.h>
#include <bcm_buzzz.h>
#include <bcmdevs.h>
#include <bcmutils.h>

#if !(defined(BCMDBG_LOADAVG) && (defined(__ARM_ARCH_7R__) || \
	defined(__ARM_ARCH_7A__))) && !defined(BCM_BOOTLOADER)
static void hnd_cpu_deadman_timer_stop(si_t *sih);
#endif // endif

/* ======================= trap ======================= */

/* Registered halt callback function. */
static hnd_halt_hdlr_t g_hnd_haltfn = NULL;
static void *g_hnd_haltfn_ctx = NULL;

/* Print stack */
void
hnd_print_stack(uint32 sp)
{
	uint i = 0;
	uint j = 0;
	uint32 *stack = (uint32 *)sp;
	uint stack_headroom = (hnd_get_rambottom() - sp) / 4;

	printf("\n   sp+0 %08x %08x %08x %08x\n",
		stack[0], stack[1], stack[2], stack[3]);
	printf("  sp+10 %08x %08x %08x %08x\n\n",
		stack[4], stack[5], stack[6], stack[7]);

	for (i = 0, j = 0; j < 16 && i < stack_headroom; i++) {
		/* make sure it's a odd (thumb) address and at least 0x100 */
		if (!(stack[i] & 1) || stack[i] < 0x100)
			continue;

		/* Check if it's within the RAM or ROM text regions */
		if ((stack[i] <= (uint32)text_end) ||
#if defined(ROMBASE)
		    ((stack[i] >= ROMBASE && (stack[i] <= ROMEND))) ||
#endif /* BCMROMOFFLOAD */
		    FALSE) {
			printf("sp+%x %08x\n", (i*4), stack[i]);
			j++;
		}
	}
}

/* Common non-ISR/non-IRQ related traps handling */
void
hnd_trap_common(trap_t *tr)
{
	BUZZZ_LVL1(HND_TRAP, 2, tr->pc, tr->type);

#if defined(__ARM_ARCH_7M__)
#ifndef RTE_POLL
	if (BCMOVLHW_ENAB(get_hnd_sih()) && (tr->type == TR_BUS)) {
		if (si_arm_ovl_int(get_hnd_sih(), tr->pc))
			return;
	}
#ifdef BCMDBG_LOADAVG
	if (tr->type == TR_SYSTICK) {
		hnd_cpu_load_avg(tr->pc, tr->r14, tr->r13);
		return;
	}
#endif	/* BCMDBG_LOADAVG */
#endif	/* !RTE_POLL */

#ifndef	BCMSIM
	if ((tr->type == TR_FAULT) && (tr->pc  == ((uint32)&osl_busprobe & ~1))) {
		/*	LDR and ADR instruction always sets the least significant bit
		 *	of program address to 1 to indicate thumb mode
		 *	LDR R0, =osl_busprobe ; R0 = osl_busprobe + 1
		 */
		/* printf("busprobe failed for addr 0x%08x\n", tr->pc); */
		*((volatile uint32 *)(tr->r13 + CM3_TROFF_PC)) = tr->pc + 6;
		*((volatile uint32 *)(tr->r13 + CM3_TROFF_R0)) = ~0;
		return;
	}
#endif	/* BCMSIM */

	BCM_BUZZZ_STOP();
#endif /* __ARM_ARCH_7M__ */

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7A__)
#if defined(BCMDBG_ASSERT) && defined(BCMDBG_ASSERT_TRAP)
	/* DBG_ASSERT_TRAP causes a trap/exception when an ASSERT fails, instead of calling
	 * an assert handler to log the file and line number. This is a memory optimization
	 * that eliminates the strings associated with the file/line and the function call
	 * overhead associated with invoking the assert handler. The assert location can be
	 * determined based upon the program counter displayed by the trap handler.
	 *
	 * The system service call (SVC) instruction is used to generate a software
	 * interrupt for a failed assert.
	 */
#if defined(__ARM_ARCH_7M__)
	if (tr->type == TR_SVC) {
#elif defined(__ARM_ARCH_7A__)
	if (tr->type == TR_SWI) {
#endif /* __ARM_ARCH_7M__ */
		/* Adjust the program counter to the assert location. */
		tr->pc -= 2;
		printf("\n\nASSERT pc %x\n", tr->pc);
#if defined(__ARM_ARCH_7A__)
		tr->pc += 2; /* to stay compatible with UTF's trap dump mechanism */
#endif /* __ARM_ARCH_7A__ */
	}
#endif /* BCMDBG_ASSERT && BCMDBG_ASSERT_TRAP */
#endif /* __ARM_ARCH_7M__ || __ARM_ARCH_7A__ */

#if defined(BCMSDIODEV_ENABLED)
	printf("\nFWID 01-%x\nflags %x\n", sdpcm_shared.fwid, sdpcm_shared.flags);
#endif // endif
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		printf("\nFWID 01-%x\nflags %x\n", pciedev_shared.fwid, pciedev_shared.flags);
	}
#endif // endif
#if (defined(BCMUSBDEV_ENABLED) && !defined(BCM_BOOTLOADER)) || defined(FWID)
	printf("\nFWID 01-%x\n", gFWID);
#endif // endif

	/*
	 * ARM7TDMI trap types (__ARM_ARCH_7R__ / __ARM_ARCH_7A__):
	 *	0=RST, 1=UND, 2=SWI/SVC, 3=IAB, 4=DAB, 5=BAD, 6=IRQ, 7=FIQ
	 *
	 * ARM CM3 trap types (__ARM_ARCH_7M__):
	 *	1=RST, 2=NMI, 3=FAULT, 4=MM, 5=BUS, 6=USAGE, 11=SVC,
	 *	12=DMON, 14=PENDSV, 15=SYSTICK, 16+=ISR
	 */
	/* Note that UTF parses the first line, so the format should not be changed. */
	printf("\nTRAP %x(%x): pc %x, lr %x, sp %x, cpsr %x, spsr %x\n",
	       tr->type, (uint32)tr, tr->pc, tr->r14, tr->r13, tr->cpsr, tr->spsr);

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	if (tr->type == TR_DAB)
		printf("  dfsr %x, dfar %x\n",
		       get_arm_data_fault_status(), get_arm_data_fault_address());
	else if (tr->type == TR_IAB)
		printf("  ifsr %x, ifar %x\n",
		       get_arm_instruction_fault_status(), get_arm_instruction_fault_address());
#endif // endif

#if defined(BCMSPACE) || defined(RTE_CONS) || defined(BCM_OL_DEV)
	printf("  r0 %x, r1 %x, r2 %x, r3 %x, r4 %x, r5 %x, r6 %x\n",
	       tr->r0, tr->r1, tr->r2, tr->r3, tr->r4, tr->r5, tr->r6);
	printf("  r7 %x, r8 %x, r9 %x, r10 %x, r11 %x, r12 %x\n",
	       tr->r7, tr->r8, tr->r9, tr->r10, tr->r11, tr->r12);

	hnd_print_stack(tr->r13);
#endif /* BCMSPACE || RTE_CONS || BCM_OL_DEV */

#if defined(BCMSDIODEV_ENABLED)
	/* Fill in structure that be downloaded by the host */
	sdpcm_shared.flags |= SDPCM_SHARED_TRAP;
	sdpcm_shared.trap_addr = (uint32)tr;
#endif // endif

#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		/* Fill in structure that be downloaded by the host */
		pciedev_shared.flags |= PCIE_SHARED_TRAP;
		pciedev_shared.trap_addr = (uint32)tr;
	}
#endif // endif
} /* hnd_trap_common */

#if defined(FIQMODE) && (defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__))
void
hnd_fiqtrap_handler(uint32 epc, uint32 lr, uint32 sp, uint32 cpsr)
{
#ifdef HND_PRINTF_THREAD_SAFE
	in_fiq_handler ++;
#endif	/* HND_PRINTF_THREAD_SAFE */

#if defined(BCMDBG_LOADAVG)
#ifdef EVENT_LOG_ROM_PRINTF_MAP
	/* Save current hook. */
	post_printf_hook hook = get_current_post_printf_hook();
	unregister_post_printf_hook();
#endif // endif
	hnd_cpu_load_avg(epc, lr, sp);
	hnd_cpu_loadavg_timer(get_hnd_sih(), loadavg_time);
#ifdef EVENT_LOG_ROM_PRINTF_MAP
	/* restore the hook. */
	register_post_printf_hook(hook);
#endif // endif
#endif /* BCMDBG_LOADAVG */
#ifdef EVENT_LOG_ROM_PRINTF_MAP
	unregister_post_printf_hook();
#endif // endif
#ifdef HND_PRINTF_THREAD_SAFE
	in_fiq_handler --;
#endif	/* HND_PRINTF_THREAD_SAFE */
} /* hnd_fiqtrap_handler */
#endif /* FIQMODE && (__ARM_ARCH_7R__ || __ARM_ARCH_7A__) */

/*
 * By default we trap on reads/writes to addresses 0-127 in low memory.  This detects
 * illegal NULL pointer dereferences, including some like ((struct xxx)NULL)->field.
 * The trap region may be overridden in a Makefile for debugging special cases.  E.g.,
 * if a write corruption is occurring somewhere from 0x21488 to 0x2149f, one could change
 * the region start to 0x21480, region size to 5 (32 bytes), and trap type to WRITE (6).
 */
#ifndef BCMDBG_TRAP_LG2SIZE
#define BCMDBG_TRAP_LG2SIZE	7
#endif // endif

#ifndef BCMDBG_TRAP_BASE
#define BCMDBG_TRAP_BASE	0x00000000	/* Must be multiple of 2^LG2SIZE */
#endif // endif

#ifndef BCMDBG_TRAP_TYPE
#define BCMDBG_TRAP_TYPE	CM3_DWT_FUNCTION_WP_RDWR	/* 5=rd, 6=wr, 7=rd/wr */
#endif // endif

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
#define	HT_CLOCK_120		120000000
static void hnd_cpu_clock_init(uint hz);
#endif	/* __ARM_ARCH_7R__ || __ARM_ARCH_7A__ */

/* Trap initialization */
void
BCMATTACHFN(hnd_trap_init)(void)
{
#if defined(BCMDBG_TRAP) && !defined(_RTE_SIM_)
	/*
	 * When BCMDBG_TRAP is true, set up CPU to trap on as many errors as possible.
	 * Capabilities vary by CPU.
	 */

#if defined(__ARM_ARCH_7M__)
	/* Enable traps for detecting divide by zero */
	wreg32(CM3_CFGCTRL, rreg32(CM3_CFGCTRL) | CM3_CFGCTRL_DIV_0_TRP);
#endif // endif

#if defined(__ARM_ARCH_7M__)
	/* Disable alignment of trap stack to a multiple of 8 bytes; confuses trap handler */
	wreg32(CM3_CFGCTRL, rreg32(CM3_CFGCTRL) & ~CM3_CFGCTRL_STKALIGN);
#endif // endif

#if defined(__ARM_ARCH_7M__)
	/* Enable DWT (data watchpoint and tracing) functionality and Monitor Mode. */
	wreg32(CM3_DBG_EMCR, rreg32(CM3_DBG_EMCR) | CM3_DBG_EMCR_TRCENA | CM3_DBG_EMCR_MON_EN);

	/* Set address mask to ignore the bottom N bits to match block of given size */
	wreg32(CM3_DWT_MASK0, BCMDBG_TRAP_LG2SIZE);

	/* Set comparator value to match the block at given address. */
	wreg32(CM3_DWT_COMP0, BCMDBG_TRAP_BASE);

	/* Set function; first clear MATCH bit in case already set */
	(void)rreg32(CM3_DWT_FUNCTION0);
	wreg32(CM3_DWT_FUNCTION0, BCMDBG_TRAP_TYPE);

	/* The priority of all exceptions and interrupts defaults to 0 (highest).  The data
	 * watchpoint exception will not occur while handling an interrupt if they're the same
	 * priority, and most of our code executes at interrupt level, so set the priority of
	 * interrupts to the next highest priority, which is 0x20 since priority is configured
	 * for 3 bits and is held in the upper 3 bits of an 8-bit value.
	 */
	wreg32(CM3_NVIC_IRQ_PRIO0, 1U << 5);
#endif /* __ARM_ARCH_7M__ */

#endif /* BCMDBG_TRAP && !_RTE_SIM_ */

#if defined(BCMDBG_LOADAVG) && !defined(__ARM_ARCH_7R__) && !defined(__ARM_ARCH_7A__)
	/* Set up SysTick interrupt for profiling */
	wreg32(CM3_NVIC_TICK_RLDVAL,
	       HT_CLOCK / LOADAVG_HZ);
	wreg32(CM3_NVIC_TICK_CSR,
	       CM3_NVIC_TICK_CSR_CLKSOURCE | CM3_NVIC_TICK_CSR_TICKINT | CM3_NVIC_TICK_CSR_ENABLE);
#endif /* BCMDBG_LOADAVG  && !__ARM_ARCH_7R__ && !__ARM_ARCH_7A__ */

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
/* for CR4, clock at start is configured as 120Mhz [instead of default 80MHz].
 * So, make sure that, c0counts_per_us is inited with approx count before for all delays
 * used before hnd_cpu_init().
 */
	hnd_cpu_clock_init(HT_CLOCK_120);
	/* enable imprecise aborts/exceptions */
	enable_arm_dab();
#endif /* __ARM_ARCH_7R__ */

	/* Set trap handler */
	hnd_set_trap(hnd_trap_handler);
#if defined(FIQMODE) && !defined(FIQ_USE_COMMON_TRAP_HDLR)
	hnd_set_fiqtrap(hnd_fiqtrap_handler);
#endif // endif
} /* hnd_trap_init */

/*
 * Enter and stay in busy loop forever.
 */
void
_hnd_die(bool trap)
{
	BUZZZ_LVL1(HND_DIE, 1, (uint32)__builtin_return_address(0));
	BCM_BUZZZ_STOP();

#ifdef RTE_CONS
	/* Flush out any pending console print messages
	 * ensures there is room for critical log messages that
	 * may follow
	 */
	hnd_cons_flush();
#endif // endif

	/* Force a trap. This will provide a register dump and callstack for post-mortem debug. */
	if (trap) {
		int *null = NULL;
		*null = 0;
	}

#if (defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)) \
	&& (defined(BCMSDIODEV_ENABLED) || defined(BCMUSBDEV_ENABLED) || \
	defined(BCMPCIEDEV_ENABLED))
#if !(defined(BCMDBG_LOADAVG) && (defined(__ARM_ARCH_7R__) || \
	defined(__ARM_ARCH_7A__))) && !defined(BCM_BOOTLOADER)
	/* Make sure deadman timer is cleared. */
	hnd_cpu_deadman_timer_stop(get_hnd_sih());
#endif // endif

	/* Call halt function, if it was registered. */
	if (g_hnd_haltfn != NULL) {
		(*g_hnd_haltfn)(g_hnd_haltfn_ctx);
	}
#endif /* __ARM_ARCH_7M__ && (BCMSDIODEV_ENABLED || BCMUSBDEV_ENABLED) */

#ifdef RTE_CONS
	/* Flush out any console print messages that have just been
	 * added as FW will no longer service THR interrupt
	 */
	hnd_cons_flush();
#endif // endif
	/* Spin forever... */
	while (1) {
		;
	}
}

void
hnd_set_fwhalt(hnd_halt_hdlr_t hdlr, void *ctx)
{
	g_hnd_haltfn = hdlr;
	g_hnd_haltfn_ctx = ctx;
}

/* ================ CPU stats ================= */

#ifdef BCMDBG_CPU
static hnd_timer_t *cu_timer;	/* timer for cpu usage calculation */
#define DEFUPDATEDELAY 5000

hnd_cpu_stats_t cpu_stats;
static uint32 enterwaittime = 0xFFFFFFFF;

static void
hnd_update_stats(hnd_cpu_stats_t *cpustats)
{
	cpustats->totalcpu_cycles = cpu_stats.totalcpu_cycles;
	cpustats->usedcpu_cycles = cpu_stats.usedcpu_cycles;
	cpustats->cpusleep_cycles = cpu_stats.cpusleep_cycles;
	cpustats->min_cpusleep_cycles = cpu_stats.min_cpusleep_cycles;
	cpustats->max_cpusleep_cycles = cpu_stats.max_cpusleep_cycles;
	cpustats->num_wfi_hit = cpu_stats.num_wfi_hit;

	/* clean it off */
	bzero(&cpu_stats, sizeof(cpu_stats));
}

static void
hnd_clear_stats(void)	/* Require separate routine for only stats clearance? */
{
	bzero(&cpu_stats, sizeof(cpu_stats));
}

static void
hnd_print_cpuuse(void *arg, int argc, char *argv[])
{
	si_t *sih = (si_t *)arg;
	printf("CPU stats to be displayed in %d msecs\n", DEFUPDATEDELAY);

	/* Force HT on */
	si_clkctl_cc(sih, CLK_FAST);

	/* Schedule timer for DEFUPDATEDELAY ms */
	hnd_timer_start(cu_timer, DEFUPDATEDELAY, 0);

	/* Clear stats and restart counters */
	hnd_clear_stats();
}

static void
hnd_update_timer(hnd_timer_t *t)
{
	si_t *sih = (si_t *)hnd_timer_get_ctx(t);
	hnd_cpu_stats_t cpustats;

	hnd_update_stats(&cpustats);

	/* Disable FORCE HT, which was enabled at hnd_print_cpuuse */
	si_clkctl_cc(sih, CLK_DYNAMIC);

	printf("Total cpu cycles : %u\n"
			"Used cpu cycles : %u\n"
			"Total sleep cycles (+.05%%): %u\n"
			"Average sleep cycles: %u\n"
			"Min sleep cycles: %u, Max sleep cycles: %u\n"
			"Total number of wfi hit %u\n",
			cpustats.totalcpu_cycles,
			cpustats.usedcpu_cycles,
			cpustats.cpusleep_cycles,
			cpustats.cpusleep_cycles/cpustats.num_wfi_hit,
			cpustats.min_cpusleep_cycles, cpustats.max_cpusleep_cycles,
			cpustats.num_wfi_hit);
}

void
hnd_cpu_stats_upd(uint32 start_time)
{
	uint32 current_time = 0;
	uint32 totalcpu_cycles = 0;
	uint32 usedcpu_cycles = 0;
	uint32 cpuwait_cycles = 0;

	/* get total cpu cycles */
	totalcpu_cycles = (start_time == 0) ? 0 : (enterwaittime - start_time);
	cpu_stats.totalcpu_cycles += totalcpu_cycles;

	/* get used cpu cycles */
	current_time = get_arm_inttimer();
	usedcpu_cycles = (current_time == 0) ? 0 : (enterwaittime - current_time);
	cpu_stats.usedcpu_cycles += usedcpu_cycles;

	/* get sleep cpu cycle */
	cpuwait_cycles = (cpu_stats.last == 0) ? 0 : (cpu_stats.last - start_time);
	cpu_stats.cpusleep_cycles += cpuwait_cycles;

	/* update last cpu usage time */
	cpu_stats.last = current_time;

	if (cpu_stats.num_wfi_hit == 0) {
		cpu_stats.min_cpusleep_cycles = cpuwait_cycles;
		cpu_stats.max_cpusleep_cycles = cpuwait_cycles;
	}

	/* update min max cycles in sleep state */
	cpu_stats.min_cpusleep_cycles =
		((cpuwait_cycles < cpu_stats.min_cpusleep_cycles)) ?
	        cpuwait_cycles : cpu_stats.min_cpusleep_cycles;
	cpu_stats.max_cpusleep_cycles =
		((cpuwait_cycles > cpu_stats.max_cpusleep_cycles)) ?
	        cpuwait_cycles : cpu_stats.max_cpusleep_cycles;
	cpu_stats.num_wfi_hit++;
}
#endif	/* BCMDBG_CPU */

/* ================ CPU profiling ================= */

#ifndef DEADMAN_TIMEOUT
#define DEADMAN_TIMEOUT		0
#endif // endif

#if !(defined(BCMDBG_LOADAVG) && (defined(__ARM_ARCH_7R__) || \
	defined(__ARM_ARCH_7A__)))
static const char BCMATTACHDATA(rstr_deadman_to)[] = "deadman_to";
static hnd_timer_t *deadman_timer = NULL;
static uint32 deadman_to = DEADMAN_TIMEOUT;
#endif // endif

#if !(defined(BCMDBG_LOADAVG) && (defined(__ARM_ARCH_7R__) || \
	defined(__ARM_ARCH_7A__)))
static void
_deadman_timer(hnd_timer_t *timer)
{
	si_t *sih = (si_t *) hnd_timer_get_data(timer);

	/* refresh the deadman timer w/the specified value */
	hnd_cpu_deadman_timer(sih, deadman_to);
}
#endif // endif

#ifdef BCMDBG_LOADAVG
/* Note: load average mechanism also requires BCMDBG_FORCEHT to be defined */

#define LOADAVG_ALPHA		0.999		/* Decay per sample */
#define LOADAVG_PRT_IVAL	200		/* Print interval, in units of Hz */

#define LOADAVG_A		((int)(LOADAVG_ALPHA * 256))

#define LOADAVG_WFI_INSTR	0xbf30
#define LOADAVG_CODESPACE_MASK	0x1fffff

#if BCMDBG_LOADAVG
static unsigned int loadavg;
static unsigned int loadavg_prt;
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
unsigned int loadavg_time;
#endif // endif
#endif /* BCMDBG_LOADAVG */

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
static void
hnd_loadavg_test(void *arg, int argc, char *argv[])
{
	uint32 stime;
	uint32 buf[8];

	/* Busy loop for 20 sec; call function in ROM to spend time in both RAM and ROM */

	stime = hnd_time();

	while (hnd_time() - stime < 20000)
		memset(buf, 0, sizeof(buf));
}
#endif /* RTE_CONS && !BCM_BOOTLOADER */

void
hnd_cpu_load_avg(uint epc, uint lr, uint sp)
{
	/* CPU was idle if it broke out of a wfi instruction. */
	unsigned int idle = ((epc & LOADAVG_CODESPACE_MASK) >= 2 &&
	                     (*(uint16 *)(epc - 2) == LOADAVG_WFI_INSTR));
	unsigned int load = (idle ? 0 : 100) * 256;

	/* Update average using fixed point arithmetic (8.8) */
	loadavg = (loadavg * LOADAVG_A + load * (256 - LOADAVG_A)) >> 8;
	if (++loadavg_prt >= LOADAVG_PRT_IVAL) {
		printf("Load: %2d.%02d (pc=0x%08x lr=0x%08x sp=0x%08x)\n",
		       loadavg >> 8, (100 * (loadavg & 255)) >> 8, epc, lr, sp);
		loadavg_prt = 0;
	}
}
#endif /* BCMDBG_LOADAVG */

int
BCMATTACHFN(hnd_cpu_stats_init)(si_t *sih)
{
#ifdef BCMDBG_CPU
	cu_timer = hnd_timer_create((void *)sih, NULL, hnd_update_timer, NULL);
	if (cu_timer == NULL)
		return BCME_NORESOURCE;
#endif // endif

#if defined(BCMDBG_LOADAVG) && (defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__))
	loadavg_time = si_cpu_clock(sih)/LOADAVG_HZ;
	/* Set the Inttimer first time value to 1 second to wait for initialization done */
	hnd_cpu_loadavg_timer(sih, si_cpu_clock(sih));
#endif /* BCMDBG_LOADAVG  && (__ARM_ARCH_7R__ || __ARM_ARCH_7A__) */

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
#ifdef BCMDBG_LOADAVG
	hnd_cons_add_cmd("lat", hnd_loadavg_test, 0);
#endif // endif
#ifdef BCMDBG_CPU
	hnd_cons_add_cmd("cu", hnd_print_cpuuse, sih);
#endif // endif
#endif /* RTE_CONS  && ! BCM_BOOTLOADER */

	return BCME_OK;
}

#ifndef BCMDBG_LOADAVG
static void
hnd_cpu_deadman_timer_init(si_t *sih)
{
#ifdef __ARM_ARCH_7A__
	uint32 cntfrq = 32000; /* set Counter Frequency of CA7 to 32KHz */
	uint32 cntkctl = 0;
	uint32 cntpctl = (1 << 1) | (1 << 0);

	/* Set ALP ILP Period */
	hnd_pmu_set_gtimer_period();

	/* Enable the timer */
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (cntfrq));

	/* Set time PL1 control register */
	asm volatile("mcr p15, 0, %0, c14, c1, 0" : : "r" (cntkctl));

	/* Set mask bit and set enable bit */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (cntpctl));
	asm volatile("isb");
#endif /* __ARM_ARCH_7A__ */
}

static void
hnd_cpu_deadman_timer_start(si_t *sih, uint32 period)
{
#ifdef __ARM_ARCH_7A__
	uint32 ctrl;
	uint64 count;

	BCM_REFERENCE(sih);

	/* Get PL1 physical timer controller register */
	asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (ctrl));
	/* Set mask bit, i.e., disable interrupt */
	ctrl |= (1 << 1);

	/* Get physical counter */
	asm volatile("isb");
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (count));

	count += (period / 1000000) * 32000;

	/* Set PL1 physical timer controller register */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
	asm volatile("isb");

	/* Set PL1 Physical Comp Value */
	asm volatile("mcrr p15, 2, %Q0, %R0, c14" : : "r" (count));
	asm volatile("isb");

	/* Clear mask bit, i.e., enable interrupt */
	ctrl &= ~(1 << 1);

	/* Set PL1 physical timer controller register */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
	asm volatile("isb");
#else
	hnd_cpu_deadman_timer(sih, period);
#endif /* __ARM_ARCH_7A__ */
	printf("Start deadman timer\n");
}

uint32
hnd_cpu_gtimer_trap_validation(void)
{
#ifdef __ARM_ARCH_7A__
	volatile uint32 wait_cnt;
	uint64 count, cpcount;
	uint32 ctrl;
	int i;

#define GTIMER_VALIDATION_RETRY 2

	/* Get PL1 physical timer controller register */
	asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (ctrl));
	/* Disable interrupt */
	ctrl |= (1 << 1);
	/* Set PL1 physical timer controller register */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
	asm volatile("isb");

	/* Check  physical counter and PL1 Physical Comp Value again */
	/* Get physical counter */
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (count));
	/* Get PL1 Physical Comp Value */
	asm volatile("isb");
	asm volatile("mrrc p15, 2, %Q0, %R0, c14" : "=r" (cpcount));

	/* gtimer assertion */
	if (cpcount <= count) {
		/* HW4365-294, we need to read more times to make sure gtimer assert. */
		for (i = 0; i < GTIMER_VALIDATION_RETRY; i++) {
			/* Wait 1 ms */
			wait_cnt = 800000;
			while (wait_cnt--) {
				;	/* empty */
			}

			asm volatile("isb");
			asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (count));
			if (cpcount > count) {
				/* False gtimer assertion */
				break;
			}
		}
		if (i == GTIMER_VALIDATION_RETRY) {
			/* Real gtimer assertion */
			return 1;
		}
	}

	/* fake deadman timer assert */
	return 0;
#else
	/* Not handle, let it run into FIQ TRAP */
	return 1;
#endif /* __ARM_ARCH_7A__ */
}

void
hnd_cpu_gtimer_fiq_hdl(void)
{
	/* Don't add any local variable in this funciton */
	/* assembly code should be - */
	/* push {lr} */

	if (hnd_cpu_gtimer_trap_validation() == 0)
		return;

	/* Return to deadman_trap */
	asm volatile("ldr r0,=deadman_trap" : :);
	asm volatile("str r0, [sp, #4]" : :);
	return;
}

static void
hnd_cpu_deadman_timer_stop(si_t *sih)
{
#ifdef __ARM_ARCH_7A__
	uint32 ctrl;

	BCM_REFERENCE(sih);

	/* Get PL1 physical timer controller register */
	asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (ctrl));

	/* Set mask bit, i.e., disable interrupt */
	ctrl |= (1 << 1);

	/* Set PL1 physical timer controller register */
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
	asm volatile("isb");
#else
	hnd_cpu_deadman_timer(sih, 0);
#endif /* __ARM_ARCH_7A__ */
}
#endif /* !BCMDBG_LOADAVG */

/* initial deadman timer */
int
BCMATTACHFN(hnd_cpu_deadman_init)(si_t *sih)
{
	/* BCMDBG_LOADAVG also uses Inttimer. */
#if !(defined(BCMDBG_LOADAVG) && (defined(__ARM_ARCH_7R__) || \
	defined(__ARM_ARCH_7A__)))
	char *val;

	/* set up deadman timer if a timeout was specified */
	val = getvar(NULL, rstr_deadman_to);
	if (val) {
		deadman_to = bcm_strtoul(val, NULL, 0);
	}
	else {
		/* Default disable deadman timer if NVRAM is not specified */
		deadman_to = 0;
	}

	if (deadman_to) {
		int32 refresh_time;
		/* refresh every 1 second */
		refresh_time = 1;
		if (refresh_time > 0) {
			hnd_cpu_deadman_timer_init(sih);

			deadman_timer = hnd_timer_create(NULL, (void *)sih, _deadman_timer, NULL);
			if (deadman_timer == NULL)
				return BCME_NORESOURCE;
			if (hnd_timer_start(deadman_timer, refresh_time * 1000, TRUE)) {
				hnd_cpu_deadman_timer_start(sih, deadman_to);
			}
		}
	}
	else
		hnd_cpu_deadman_timer_stop(sih);
#endif /* !(BCMDBG_LOADAVG  && (__ARM_ARCH_7R__ || __ARM_ARCH_7A__)) */

	return BCME_OK;
}

/* ========================== CPU init ========================= */
/*
 * Initialize and background:
 *	hnd_cpu_init: Initialize the world.
 */

/*
 * c0 ticks per usec - used by hnd_delay() and is based on 80Mhz clock.
 * Values are refined after function hnd_cpu_init() has been called, and are recalculated
 * when the CPU frequency changes.
 */
uint32 c0counts_per_us = HT_CLOCK / 1000000;
uint32 c0counts_per_ms = HT_CLOCK / 1000;

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
static void
hnd_cpu_clock_init(uint hz)
{
	c0counts_per_us = hz / 1000000;
}
#endif // endif

#if !defined(RTE_POLL) && defined(__ARM_ARCH_7A__)
#define MPCORE_GIC_DIST_OFF	0x1000
#define MPCORE_GIC_CPUIF_OFF	0x2000

#define GIC_DIST_CTRL 	0x000
#define GIC_DIST_CTR	0x004
#define GIC_DIST_IGROUP	0x080
#define GIC_DIST_ENABLE_SET	0x100
#define GIC_DIST_ENABLE_CLEAR	0x180
#define GIC_DIST_PRI	0x400
#define GIC_DIST_TARGET 0x800
#define GIC_DIST_CONFIG 0xc00

#define GIC_CPU_CTRL 	0x0
#define GIC_CPU_PRIMASK 0x4

static void gic_dist_init(si_t *sih, uint32 base)
{
	unsigned int max_irq, i, igroup;
	uint32 cpumask = 1 << 0;	/* single-cpu for now */

	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;

	*(volatile uint32 *)(base + GIC_DIST_CTRL) = 0;

	/*
	 * Find out how many interrupts are supported.
	 */
	max_irq = *(volatile uint32 *)(base + GIC_DIST_CTR) & 0x1f;
	max_irq = (max_irq + 1) * 32;
	printf("gic_dist_init max_irq %u\n", max_irq);

	/* Secure Physical Timer event (PPI1)
	 * This is the event generated from the Secure Physical Timer and uses
	 * ID29. The interrupt is level-sensitive
	 */
	/* Set all interrupts to Group1 except PPI1 (ID29), (Group1 always signal IRQ) */
	for (i = 0; i < max_irq; i += 32) {
		if (!i)
			igroup = ~(1 << 29); /* except PPI1 (ID29) */
		else
			igroup = 0xffffffff;

		*(volatile uint32 *)(base + GIC_DIST_IGROUP + i * 4 / 32) = igroup;
	}

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (i = 32; i < max_irq; i += 16)
		*(volatile uint32 *)(base + GIC_DIST_CONFIG + i * 4 / 16) = 0;

	/*
	 * Set all global interrupts to this CPU only.
	 */
	for (i = 32; i < max_irq; i += 4)
		*(volatile uint32 *)(base + GIC_DIST_TARGET + i * 4 / 4) = cpumask;

	/*
	 * Set priority on all interrupts.
	 */
	for (i = 0; i < max_irq; i += 4)
		*(volatile uint32 *)(base + GIC_DIST_PRI + i * 4 / 4) = 0xa0a0a0a0;

	/*
	 * Enable all interrupts but the error irqs.
	 */
	for (i = 0; i < max_irq; i += 32) {
		*(volatile uint32 *)(base + GIC_DIST_ENABLE_SET + i * 4 / 32) = 0xffffffff;
	}

	if (BCM4365_CHIP(sih->chip)) {
		/* Disable vasip2mac interrupt as we don't want to process with ARM. */
		*(volatile uint32 *)(base + GIC_DIST_ENABLE_CLEAR + 4) = (1 << 10);
	}

	/* Enable both Group0 and Group1 */
	*(volatile uint32 *)(base + GIC_DIST_CTRL) = 0x3;
}

static void gic_cpu_init(uint32 base)
{
	*(volatile uint32 *)(base + GIC_CPU_PRIMASK) = 0xf0;

	/* Enable both Group0 and Group1 and FIQ on Group0 */
	*(volatile uint32 *)(base + GIC_CPU_CTRL) = 0xb;
}

static void armca7_gic_init(si_t *sih)
{
	uint32 periphbase;

	asm("mrc p15,4,%0,c15,c0,0 @ Read Configuration Base Address Register"
		: "=&r" (periphbase) : : "cc");
	/* Init GIC interrupt distributor */
	gic_dist_init(sih, periphbase + MPCORE_GIC_DIST_OFF);
	/* Initialize the GIC CPU interface for the boot processor */
	gic_cpu_init(periphbase + MPCORE_GIC_CPUIF_OFF);
}
#endif /* !RTE_POLL && __ARM_ARCH_7A__ */

#if defined(RTE_CACHED) && defined(__ARM_ARCH_7A__)
#define CR_M    (1 << 0)        /* MMU enable                           */
#define CR_A    (1 << 1)        /* Alignment abort enable               */
#define CR_C    (1 << 2)        /* Dcache enable                        */
#define CR_W    (1 << 3)        /* Write buffer enable                  */
#define CR_P    (1 << 4)        /* 32-bit exception handler             */
#define CR_D    (1 << 5)        /* 32-bit data address range            */
#define CR_L    (1 << 6)        /* Implementation defined               */
#define CR_B    (1 << 7)        /* Big endian                           */
#define CR_S    (1 << 8)        /* System MMU protection                */
#define CR_R    (1 << 9)        /* ROM MMU protection                   */
#define CR_F    (1 << 10)       /* Implementation defined               */
#define CR_Z    (1 << 11)       /* Implementation defined               */
#define CR_I    (1 << 12)       /* Icache enable                        */
#define CR_V    (1 << 13)       /* Vectors relocated to 0xffff0000      */

#define CR_SMP  (1 << 6)        /* SMP enable */

#define isb() __asm__ __volatile__ ("isb")
#define dsb() __asm__ __volatile__ ("dsb")
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t")

/* defines for L1 MMU 'section' (describing the properties of a 1MB block of memory) */
#define L1_PXN_SHIFT		0 /**< privileged execute-not bit */
#define L1_B_SHIFT		2 /**< bufferable */
#define L1_C_SHIFT		3 /**< cachable */
#define L1_XN_SHIFT		4 /**< execute-not bit */
#define L1_DOMAIN_SHIFT		5
#define L1_P_SHIFT		9
#define L1_AP_SHIFT		10
#define L1_TEX_SHIFT		12
#define L1_APX_SHIFT		15
#define L1_S_SHIFT		16 /**< sharable */
#define L1_NG_SHIFT		17 /**< non global */
#define L1_SBZ_SHIFT		19
#define L1_BASE_ADDR_SHIFT	20 /**< section base address */
#define L1_PAGE_TABLE_PXN	2  /** privileged execute-not bit for a L1 'page table' entry */

#define L1_ENTRY_PAGE_TABLE(page_table_address, pxn) (\
	(1 << 0) | ((pxn) << L1_PAGE_TABLE_PXN) | (page_table_address))

#define L1_ENTRY_NORMAL_MEM(i_1mb_section) (\
	(i_1mb_section << 20) | \
	(1 << 1) | (1 << L1_B_SHIFT) | (1 << L1_C_SHIFT) | (3 << L1_AP_SHIFT) | \
	(4 << L1_TEX_SHIFT) | (1 << L1_S_SHIFT))

#define L1_ENTRY_DEVICE_MEM(i_1mb_section) (\
	(i_1mb_section << 20) | \
	(1 << 1) | (1 << L1_B_SHIFT) | (3 << L1_AP_SHIFT) | (1 << L1_XN_SHIFT) | (1<< L1_PXN_SHIFT))

#define L1_ENTRY_NOTHING(i_1mb_section) 0 /**< exception when ARM tries to access this location */

/* defines for L2 MMU large page (4KB) entries */
#define L2_SMALL_PAGE_SHIFT	1  /**< set to 1 to indicate small page format */
#define L2_B_SHIFT		2  /**< bufferable */
#define L2_C_SHIFT		3  /**< cachable */
#define L2_AP_SHIFT		4
#define L2_SBZ_SHIFT		6
#define L2_APX_SHIFT		9
#define L2_S_SHIFT		10 /**< sharable */
#define L2_NG_SHIFT		11 /**< non global */
#define L2_TEX_SHIFT		12
#define L2_XN_SHIFT		15 /**< execute-not bit */
#define L2_ADDR_SHIFT		16 /**< large page base address */

/** defines a L2 MMU entry for 'normal memory', to be used for ROM and RAM */
#define L2_ENTRY_NORMAL_MEM(i_4k_page) (\
	(((i_4k_page) << 12) & 0xffff0000) | \
	(1 << 0) | (1 << L2_B_SHIFT) | (1 << L2_C_SHIFT) | \
	(3 << L2_AP_SHIFT) | (4 << L2_TEX_SHIFT) | (1 << L2_S_SHIFT))

#define L2_ENTRY_DEVICE_MEM(i_4k_page) (\
	(((i_4k_page) << 12) & 0xffff0000) | \
	(1 << 0) | (1 << L2_B_SHIFT) | (3 << L2_AP_SHIFT) | (1 << L2_XN_SHIFT))

#define L2_ENTRY_NOTHING(i_4k_page) 0 /**< exception when ARM tries to access this location */

#define MB (1024 * 1024)

static uint8 loader_pagetable_array[0x4000*2];
static uint8 l2_pagetable_array[0x400*3];
extern void cpu_inv_cache_all(void);

#define PSR_A_BIT	0x00000100

static void
ca7_caches_on(si_t *sih)
{
	uint32 ptbaddr, *ptb;
	uint32 l2_ptbaddr, *l2_ptb;
	uint32 val;
	uint32 mb;		/**< megabyte counter */
	uint32 page; 		/**< one 4KB page */
	uint32 page_4ks = 32;

	cpu_inv_cache_all();

	/* Invalidate TLB */
	asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r" (0)); // TLBIALL

	/* Enable I$ and prefetch */
	asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
	val |= (CR_I|CR_Z);
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (val) : "cc");
	isb();

	/* page table */
	ptbaddr = (uint32)loader_pagetable_array;
	ptbaddr += (0x4000-1); // 14 bits alignment
	ptbaddr &= ~(0x4000-1);

	printf("Enabling D-cache\n");

	ptb = (uint32 *)ptbaddr;

	/* l2 page table */
	l2_ptbaddr = (uint32)l2_pagetable_array;
	l2_ptbaddr += (0x400-1);
	l2_ptbaddr &= ~(0x400-1);

	/* cur_addr is now 0x0000_0000. ROM/boot-flops start here. */
	ptb[0] = L1_ENTRY_PAGE_TABLE(l2_ptbaddr, 0); /* The first MB uses a 2nd level table */
	l2_ptb = (uint32 *)l2_ptbaddr;
	/* 4365c0 has 819.2KB ROM. However, only addresses 0x0000_0000 .. 0x000b_37ee are used */
	for (page = 0; page < 192; page++) {
		// Each 32-bit entry in a table provides translation information for 4KB of memory.
		l2_ptb[page] = L2_ENTRY_NORMAL_MEM(page);
	}

	/* cur_addr is now 0x00CD_0000. Gap between ROM and RAM starts here. */
	for (; page < 256; page++) {
		l2_ptb[page] = L2_ENTRY_NOTHING(page);
	}

	/* cur_addr is now 0x0010_0000 */
	for (mb = 1; mb < 2; mb++) {
		ptb[mb] = L1_ENTRY_NOTHING(mb);
	}

	/* cur_addr is now 0x0020_0000 */
	/* First 2MB of sysmem is configured as sharable/cacheable/bufferable normal memory */
	for (mb = 2; mb < 4; mb++) {
		ptb[mb] = L1_ENTRY_NORMAL_MEM(mb);
	}

	/* cur_addr is now 0x0040_0000 */
	/* The boundary MB is pointed to the 2nd level table */
	l2_ptbaddr += 0x400;
	ptb[4] = L1_ENTRY_PAGE_TABLE(l2_ptbaddr, 0);
	l2_ptb = (uint32 *)l2_ptbaddr;
	/* Special sysmem configuration between 2M ~ 3M region */
#ifdef RAMSIZE
	if (RAMSIZE_ADJ > (2 * MB))
		page_4ks = (RAMSIZE_ADJ - 2 * MB) >> 12;
	ASSERT(RAMSIZE_ADJ <= 3 * MB);
#endif // endif
	for (page = 0; page < page_4ks; page++) {
		l2_ptb[page] = (0x400000 | L2_ENTRY_NORMAL_MEM(page));
	}

	/* gap between RAM and io space starts here */
	for (; page < 256; page++) {
		l2_ptb[page] = L2_ENTRY_NOTHING(page);
	}

	/* cur_addr is now 0x0030_0000 */
	for (mb = 5; mb < 0x18000000 / MB; mb++) {
		ptb[mb] = L1_ENTRY_NOTHING(mb);
	}

	/* cur_addr is now 0x1800_0000 */
	for (; mb < 0x19400000 / MB; mb++) {
		ptb[mb] = L1_ENTRY_DEVICE_MEM(mb);
	}

	/* cur_addr is now 0x1940_0000 */
	for (; mb < 0x20000000 / MB; mb++) {
		ptb[mb] = L1_ENTRY_NOTHING(mb);
	}

	/* cur_addr is now 0x2000_0000. Start of the 128MB PCIe small address window, used by the
	 * dongle to access host memory.
	 */
	for (; mb < 0x28000000 / MB; mb++) {
		ptb[mb] = L1_ENTRY_DEVICE_MEM(mb);
	}

	/* cur_addr is now 0x2800_0000 */
	for (; mb < 4096; mb++) {
		ptb[mb] = L1_ENTRY_NOTHING(mb);
	}

	/* cur_addr is now 1_0000_0000 */
	dsb(); // flushes mmu tables from write buffer to main memory

	asm volatile("mcr p15, 0, %0, c2, c0, 2" : : "r" (0)); // TTBCR

	/* Apply page table address to CP15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r" (ptb) : "memory"); //TTBR0

	/* Set the access control to 'client' so MMU imposed restrictions apply */
	asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (0x55555555)); // DACR

	/* Enabling MMU and D$ */
	asm("mrc p15, 0, %0, c1, c0, 0	@ get CR" : "=r" (val) : : "cc");
	val |= (CR_C|CR_M);
	asm volatile("mcr p15, 0, %0, c1, c0, 0	@ set CR" : : "r" (val) : "cc");
	isb();
} /* ca7_caches_on */

#ifdef BCM_DMA_TRANSCOHERENT
static void ca7_coherent_on(si_t *sih)
{
	uint32 val;

	hnd_hw_coherent_enable(sih);

#ifndef ATE_BUILD
	pcie_coherent_accenable(si_osh(sih), sih);

#endif // endif
	/* Enabling SMP bit */
	asm("mrc p15, 0, %0, c1, c0, 1  @ get CR" : "=r" (val) : : "cc"); // ACTLR
	val |= CR_SMP;
	asm volatile("mcr p15, 0, %0, c1, c0, 1 @ set CR" : : "r" (val) : "cc"); // ACTLR
	isb();
}
#endif /* BCM_DMA_TRANSCOHERENT */
#endif /* RTE_CACHED && __ARM_ARCH_7A__ */

#ifdef RTE_CACHED
void
BCMATTACHFN(hnd_caches_init)(si_t *sih)
{
#ifdef __ARM_ARCH_7A__
#ifdef BCM_DMA_TRANSCOHERENT
	ca7_coherent_on(sih);
#endif /* BCM_DMA_TRANSCOHERENT */

	/* Enable caches and MMU */
	ca7_caches_on(sih);
#endif /* __ARM_ARCH_7A__ */
}
#endif /* RTE_CACHED */

void
BCMATTACHFN(hnd_cpu_init)(si_t *sih)
{
	si_arm_init(sih);

#ifdef EXT_CBALL
	{
	uint32 *v = (uint32 *)0;
	extern char __traps[], _mainCRTStartup[];

	/*
	 * Write new exception vectors.
	 * EXT_CBALL currently does not link with 'startup' at address 0.
	 */

	v[ 0] = 0xea000000 | ((uint32)_mainCRTStartup / 4 - 2);	/* 0000: b <reset> */
	v[ 1] = 0xe59ff014;				/* 0004: ldr pc, [pc, #20] */
	v[ 2] = 0xe59ff014;				/* 0008: ldr pc, [pc, #20] */
	v[ 3] = 0xe59ff014;				/* 000c: ldr pc, [pc, #20] */
	v[ 4] = 0xe59ff014;				/* 0010: ldr pc, [pc, #20] */
	v[ 5] = 0xe59ff014;				/* 0014: ldr pc, [pc, #20] */
	v[ 6] = 0xe59ff014;				/* 0018: ldr pc, [pc, #20] */
	v[ 7] = 0xe59ff014;				/* 001c: ldr pc, [pc, #20] */
	v[ 8] = (uint32)__traps + 0x00;			/* 0020: =tr_und */
	v[ 9] = (uint32)__traps + 0x10;			/* 0024: =tr_swi */
	v[10] = (uint32)__traps + 0x20;			/* 0028: =tr_iab */
	v[11] = (uint32)__traps + 0x30;			/* 002c: =tr_dab */
	v[12] = (uint32)__traps + 0x40;			/* 0030: =tr_bad */
	v[13] = (uint32)__traps + 0x50;			/* 0034: =tr_irq */
	v[14] = (uint32)__traps + 0x60;			/* 0038: =tr_fiq */
	}
#endif /* EXT_CBALL */

	/* Initialize timers */
	c0counts_per_ms = (si_cpu_clock(sih) + 999) / 1000;
	c0counts_per_us = (si_cpu_clock(sih) + 999999) / 1000000;

#if !defined(RTE_POLL) && defined(__ARM_ARCH_7A__)
	armca7_gic_init(sih);
#endif /* !RTE_POLL && __ARM_ARCH_7A__ */

	/* Use PMU as a low power timer source */
	hnd_pmu_init(sih);

#ifndef BCM_BOOTLOADER
	hnd_cpu_stats_init(sih);
#endif /* BCM_BOOTLOADER */
} /* hnd_cpu_init */

/**
 * Set or query CPU clock frequency
 * parameters:
 *	div: 0 to query, 1 or 2 to set.
 * return value:
 *	if div == 0:
 *		1 for standard frequency, 2 for double frequency, -1 for not capable of switching
 *	if div == 1 or div == 2:
 *		0 for no switch occurred, 1 for double->std switch, 2 for std->double switch
 */
int32
hnd_cpu_clockratio(si_t *sih, uint8 div)
{
	int32 ret;

	ret = si_arm_clockratio(sih, div);

	if (div != 0) {
		/* global vars 'c0counts..' are changed here for hnd_delay() / OSL_DELAY() */
		switch (ret) {
		case 1:
			c0counts_per_ms /= 2; /* because CPU clock frequency dropped */
			c0counts_per_us /= 2;
			break;
		case 2:
			c0counts_per_ms *= 2;
			c0counts_per_us *= 2;
			break;
		}
	}
	return ret;
}

/* ========================== time ========================== */

/*
 * Timing support:
 *	hnd_delay(us): Delay us microseconds.
 */

static uint32 lastcount = 0;

/** updates several time related global variables and returns current time in [ms] */
uint32
hnd_update_now(void)
{
	uint32 diff, ticks, ms;

	ticks = hnd_pmu_get_tick();

	/* The next line assumes that we update at least every 2**32 ticks */
	diff = ticks - lastcount;
	if (diff == 0)
		return 0;
	lastcount = ticks;

	ms = hnd_pmu_accu_tick(diff);

	return ms;
}

static uint32 lastcount_us = 0;

uint32
hnd_update_now_us(void)
{
	uint32 count, diff;

	count = get_arm_cyclecount();

	diff = count - lastcount_us;
	if (diff >= c0counts_per_us) {
		lastcount_us += (diff / c0counts_per_us) * c0counts_per_us;
		return diff / c0counts_per_us;
	}

	return 0;
}

#ifdef THREAD_SUPPORT
static void hnd_delay_ex(uint32 us);

void
hnd_delay(uint32 us)
{
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
	hnd_delay_ex(us);
	osl_ext_interrupt_restore(state);
}
#endif	/* THREAD_SUPPORT */

#ifdef THREAD_SUPPORT
static void
hnd_delay_ex(uint32 us)
#else
void
hnd_delay(uint32 us)
#endif	/* THREAD_SUPPORT */
{
	uint32 curr, c0_wait;

	BUZZZ_LVL1(HND_DELAY, 2, (uint32)__builtin_return_address(0), us);
#ifdef BCMQT
	/* To compensate for the slowness of quickturn */
	us /= 3000;
#endif  /* BCMQT */

	curr = get_arm_cyclecount();
	c0_wait = us * c0counts_per_us;

	while ((get_arm_cyclecount() - curr) < c0_wait) {
		;	/* empty */
	}
}

/* ========================== timer ========================== */

/**
 * The RTOS maintains a linked list of software timers, and requires a hardware timer to generate an
 * interrupt on the software timer that expires first. This function sets that hardware timer.
 */
void
hnd_set_irq_timer(uint ms)
{
	uint32 ticks = hnd_pmu_ms2tick(ms);

	hnd_pmu_set_timer(ticks);
}

void
hnd_ack_irq_timer(void)
{
	hnd_pmu_ack_timer();
}

/* ======================= system ====================== */

void
hnd_wait_irq(si_t *sih)
{
	hnd_cpu_wait(sih);
}

void
hnd_enable_interrupts(void)
{
	enable_arm_irq();
#if defined(FIQMODE) && (defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)) && \
	!defined(BCM_BOOTLOADER)
	enable_arm_fiq();
#endif /* FIQMODE && (__ARM_ARCH_7R__ || __ARM_ARCH_7A__) && !BCM_BOOTLOADER */
}

void
hnd_disable_interrupts(void)
{
	disable_arm_irq();
#if defined(FIQMODE) && (defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)) && \
	!defined(BCM_BOOTLOADER)
	disable_arm_fiq();
#endif /* FIQMODE && (__ARM_ARCH_7R__ || __ARM_ARCH_7A__) && !BCM_BOOTLOADER */
}

/* ======================= dma ====================== */

void *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
hnd_dma_alloc_consistent(uint size, uint16 align_bits, uint *alloced, void *pap,
	char *file, int line)
#else
hnd_dma_alloc_consistent(uint size, uint16 align_bits, uint *alloced, void *pap)
#endif // endif
{
	void *buf;

	/* align on a OSL defined boundary */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if (!(buf = hnd_malloc_align(size, align_bits, file, line)))
#else
	if (!(buf = hnd_malloc_align(size, align_bits)))
#endif // endif
		return NULL;
	ASSERT(ISALIGNED(buf, (1 << align_bits)));
	*alloced = size;

#ifdef CONFIG_XIP
	/*
	 * arm bootloader memory is remapped but backplane addressing is
	 * 0-based
	 *
	 * Background: since the mask rom bootloader code executes in a
	 * read-only memory space apart from SoC RAM, data addresses
	 * specified by bootloader code must be decoded differently from
	 * text addresses. Consequently, the processor must run in a
	 * "memory remap" mode whereby data addresses containing the
	 * MEMORY_REMAP bit are decoded as residing in SoC RAM. However,
	 * backplane agents, e.g., the dma engines, always use 0-based
	 * addresses for SoC RAM, regardless of processor mode.
	 * Consequently it is necessary to strip the MEMORY_REMAP bit
	 * from addresses programmed into backplane agents.
	 */
	*(ulong *)pap = (ulong)buf & ~MEMORY_REMAP;
#else
	*(ulong *)pap = (ulong)buf;
#endif /* CONFIG_XIP */

	return (buf);
}

void
hnd_dma_free_consistent(void *va)
{
	hnd_free(va);
}

/* ======================= debug ===================== */

/*
 * Use a debug comparator reg to cause a trap on stack underflow.
 */

/* Stack protection initialization */
#ifndef BCMDBG_STACKP_LG2SIZE
#define BCMDBG_STACKP_LG2SIZE 5
#endif // endif

void
BCMATTACHFN(hnd_stack_prot)(void *stack_top)
{
#if defined(__ARM_ARCH_7M__)
#if defined(STACK_PROT_TRAP) && defined(BCMDBG_TRAP) && !defined(_RTE_SIM_)
	/* Point at an STACKP_SIZE-aligned and sized area at the end */
	uint32 st = (((uint32) stack_top) + (1 << BCMDBG_STACKP_LG2SIZE) - 1) &
		~((1 << BCMDBG_STACKP_LG2SIZE) - 1);
	wreg32(CM3_DWT_MASK1, BCMDBG_STACKP_LG2SIZE);
	wreg32(CM3_DWT_COMP1, st);
	(void)rreg32(CM3_DWT_FUNCTION1);
	wreg32(CM3_DWT_FUNCTION1, CM3_DWT_FUNCTION_WP_WRITE);
#endif /* STACK_PROT ... */
#endif /* ARCH_7M */
}

void
hnd_memtrace_enab(bool on)
{
#if defined(BCMDBG_TRAP) && !defined(_RTE_SIM_)
#if defined(__ARM_ARCH_7M__)
	uint32 val = rreg32(CM3_DBG_EMCR) | CM3_DBG_EMCR_MON_EN;

	if (on)
		val |= CM3_DBG_EMCR_TRCENA;
	else
		val &= ~CM3_DBG_EMCR_TRCENA;

	/* Enable/Disable DWT (data watchpoint and tracing) functionality and Monitor Mode. */
	wreg32(CM3_DBG_EMCR, val);
#endif /* __ARM_ARCH_7M__ */
#endif /* BCMDBG_TRAP && !_RTE_SIM_ */
}
