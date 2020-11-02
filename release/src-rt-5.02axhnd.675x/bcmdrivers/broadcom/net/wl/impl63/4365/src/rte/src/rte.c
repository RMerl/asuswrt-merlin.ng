/*
 * HND RTE misc. service routines.
 * compressed image.
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
 * $Id: rte.c 735696 2017-12-12 04:49:43Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <osl_ext.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <hndsoc.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndcpu.h>
#ifdef SBPCI
#include <pci_core.h>
#include <hndpci.h>
#include <pcicfg.h>
#endif // endif
#include <hnd_debug.h>
#include <bcmsdpcm.h>
#include <hnd_pktid.h>
#include <hnd_pt.h>
#include "rte_chipc_priv.h"
#include <rte_dev.h>
#include <rte_fetch.h>
#include <rte_pktfetch.h>
#include <rte_mem.h>
#include "rte_mem_priv.h"
#include <rte_heap.h>
#include "rte_heap_priv.h"
#include <rte_uart.h>
#include <rte_cons.h>
#include "rte_cons_priv.h"
#include <rte.h>
#include "rte_priv.h"
#include <rte_assert.h>
#include <rte_trap.h>
#include "rte_isr_priv.h"
#include <rte_timer.h>
#include <bcmstdlib_ext.h>
#include "rte_pmu_priv.h"
#include "rte_pktpool_priv.h"
#if defined(BCMUSBDEV) && defined(BCMUSBDEV_ENABLED) && defined(BCMTRXV2) && \
	!defined(BCM_BOOTLOADER)
#include <trxhdr.h>
#include <usbrdl.h>
#endif /* BCMUSBDEV && BCMUSBDEV_ENABLED && BCMTRXV2 && !BCM_BOOTLOADER */
#ifdef BCM_OL_DEV
#include <bcm_ol_msg.h>
#endif // endif
#ifdef BCMM2MDEV
#include <m2mdma_core.h>
#else
#include <bcmpcie.h>
#endif // endif
#include <epivers.h>
#include <bcm_buzzz.h>
#include <bcmdevs.h>

#ifdef RAMSIZE
uint _ramsize_adj = RAMSIZE;
#endif // endif

#if defined(BCMUSBDEV) && defined(BCMUSBDEV_ENABLED) && defined(BCMTRXV2) && \
	!defined(BCM_BOOTLOADER)
extern char *_vars;
extern uint _varsz;
#endif /* BCMUSBDEV && BCMUSBDEV_ENABLED && BCMTRXV2 && !BCM_BOOTLOADER */

si_t *hnd_sih = NULL;		/* Backplane handle */
osl_t *hnd_osh = NULL;		/* Backplane osl handle */

#if defined(BCMSPLITRX) && !defined(BCMSPLITRX_DISABLED)
bool _bcmsplitrx = TRUE;
uint8 _bcmsplitrx_mode = SPLIT_RXMODE;
#else
bool _bcmsplitrx = FALSE;
uint8 _bcmsplitrx_mode = 0;
#endif // endif

#if defined(BCMPCIEDEV_ENABLED)
bool _pciedevenab = TRUE;
#else
bool _pciedevenab = FALSE;
#endif /* BCMPCIEDEV_ENABLED */

#if defined(BCMPCIEDEV)
pciedev_shared_t pciedev_shared = { };
#endif // endif
#if defined(BCMSDIODEV_ENABLED)
sdpcm_shared_t sdpcm_shared = { };
#endif // endif

#if defined(BCMM2MDEV_ENABLED)
m2m_shared_t m2m_shared = { };
#endif // endif

bool _is_mutephytxerror = FALSE;
#ifdef BCM_DHDHDR
bool _bcmdhdhdr = TRUE;
/* HOST_HDR_FETCH and AMSDU_FRAG_OPT are based upon BCM_DHDHDR */
#ifdef HOST_HDR_FETCH
bool _hosthdrfetch = TRUE;
#endif // endif
#ifdef AMSDU_FRAG_OPT
bool _amsdufrag = TRUE;
#endif // endif
#endif /* BCM_DHDHDR */

/* extern function */

#if defined(DONGLEBUILD) || defined(RTE_TEST)
/*
 * Define reclaimable NVRAM area; startarm.S will copy downloaded NVRAM data to
 * this array and move the _vars pointer to point to this array. The originally
 * downloaded memory area will be released (reused).
 */
#if defined(WLTEST) || defined(BCMDBG_DUMP) || !defined(WLC_HIGH) || defined(ATE_BUILD)
uint8 nvram_array[NVRAM_ARRAY_MAXSIZE] DECLSPEC_ALIGN(4) = {0};
#else
uint8 BCMATTACHDATA(nvram_array)[NVRAM_ARRAY_MAXSIZE] DECLSPEC_ALIGN(4) = {0};
#endif // endif
#endif /* DONGLEBUILD || RTE_TEST */

#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
static void
hnd_dump_test(void *arg, int argc, char *argv[])
{
	char a = 'a';
	uint32 numc = 8;
	uint32 i;

	if (argc == 2)
		numc = atoi(argv[1]);

	for (i = 0; i < numc; i++) {
		putc(a++);
		if ((a - 'a') == 26)
			a = 'a';
	}
	putc('\n');
}

static void
hnd_memdmp(void *arg, int argc, char **argv)
{
	uint32 addr, size, count;

	if (argc < 3) {
		printf("%s: start len [count]\n", argv[0]);
		return;
	}

	addr = bcm_strtoul(argv[1], NULL, 0);
	size = bcm_strtoul(argv[2], NULL, 0);
	count = argv[3] ? bcm_strtoul(argv[3], NULL, 0) : 1;

	for (; count-- > 0; addr += size)
		if (size == 4)
			printf("%08x: %08x\n", addr, *(uint32 *)addr);
		else if (size == 2)
			printf("%08x: %04x\n", addr, *(uint16 *)addr);
		else
			printf("%08x: %02x\n", addr, *(uint8 *)addr);
}

/**
 * Malloc memory to simulate low memory environment
 * Memory waste [kk] in KB usage: mw [kk]
 */
static void
hnd_memwaste(void *arg, int argc, char *argv[])
{
	/* Only process if input argv specifies the mw size(in KB) */
	if (argc > 1)
		printf("%p\n", hnd_malloc(atoi(argv[1]) * 1024));
}
#endif /* RTE_CONS  && !BCM_BOOTLOADER */

#ifdef RAMSIZE
uint
BCMATTACHFN(rte_ramsize)(void)
{
	uint ramsize = RAMSIZE;

	/* DHD could adjust the memory size available for CPU */
	if (HND_RAMSIZE_PTR_MAGIC == *(uint *)(RAMBASE + RAMSIZE_PTR_PTR_0)) {
		ramsize = *(uint *)(RAMBASE + RAMSIZE_PTR_PTR_0 + 4);
	}
	return (ramsize);
}
#endif /* RAMSIZE */

/* ================== debug =================== */

hnd_debug_t hnd_debug_info;

hnd_debug_t *BCMRAMFN(get_hnd_debug_info)(void)
{
	return &hnd_debug_info;
}

void
BCMATTACHFN(hnd_debug_info_init)(void)
{
	hnd_debug_t *hnd_debug_info_ptr;
	hnd_debug_info_ptr = get_hnd_debug_info();
	memset(hnd_debug_info_ptr, 0, sizeof(*(hnd_debug_info_ptr)));

	/* Initialize the debug area */
	hnd_debug_info_ptr->magic = HND_DEBUG_MAGIC;
	hnd_debug_info_ptr->version = HND_DEBUG_VERSION;
	strncpy(hnd_debug_info_ptr->epivers, EPI_VERSION_STR, HND_DEBUG_EPIVERS_MAX_STR_LEN - 1);
	/* Force a null terminator at the end */
	hnd_debug_info_ptr->epivers[HND_DEBUG_EPIVERS_MAX_STR_LEN - 1] = '\0';

#if defined(RAMBASE)
	hnd_debug_info_ptr->ram_base = RAMBASE;
	hnd_debug_info_ptr->ram_size = RAMSIZE_ADJ;
#endif // endif

#ifdef ROMBASE
	hnd_debug_info_ptr->rom_base = ROMBASE;
	hnd_debug_info_ptr->rom_size = ROMEND-ROMBASE;
#else
	hnd_debug_info_ptr->rom_base = 0;
	hnd_debug_info_ptr->rom_size = 0;
#endif /* ROMBASE */

	hnd_debug_info_ptr->event_log_top = NULL;

#if defined(BCMSDIODEV_ENABLED)
	hnd_debug_info_ptr->fwid = sdpcm_shared.fwid;
#else
	hnd_debug_info_ptr->fwid = 0;
#endif // endif

#if defined(BCM_OL_DEV) || defined(RTE_CONS)
	hnd_debug_info_ptr->console = hnd_cons_active_cons_state();
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		pciedev_shared.console_addr = (uint32)hnd_debug_info_ptr->console;
	}
#endif // endif
#if defined(BCMSDIODEV_ENABLED)
	sdpcm_shared.console_addr = (uint32)hnd_debug_info_ptr->console;
#endif // endif
#if defined(BCM_OL_DEV)
	ppcie_shared->console_addr = (uint32)&(hnd_debug_info_ptr->console->log);
#endif // endif
#endif /* BCM_OL_DEV || RTE_CONS */
}

/* ====================== time/timer ==================== */

static uint32 now = 0;

/** Return the up time in miliseconds */
uint32
hnd_time(void)
{
#ifdef THREAD_SUPPORT
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
#endif	/* THREAD_SUPPORT */

	now += hnd_update_now();

#ifdef THREAD_SUPPORT
	osl_ext_interrupt_restore(state);
#endif	/* THREAD_SUPPORT */
	return now;
}

static uint32 now_us = 0;  /* Used in latency test for micro-second precision */

uint32
hnd_time_us(void)
{
	now_us += hnd_update_now_us();

	return now_us;
}

/** Schedule work callback wrapper to delete the timer */
static void
schedule_work_timer_cb(hnd_timer_t *t)
{
	hnd_timer_auxfn_t auxfn = hnd_timer_get_auxfn(t);

	/* invoke client callback with timer pointer */
	ASSERT(auxfn != NULL);

	BUZZZ_LVL5(HND_WORK_ENT, 1, (uint32)auxfn);
	(auxfn)(t);
	BUZZZ_LVL5(HND_WORK_RTN, 0);

	hnd_timer_free(t);
}

/** Schedule a completion handler to run at safe time */
int
hnd_schedule_work(void *context, void *data, hnd_timer_mainfn_t taskfn, int delay)
{
	hnd_timer_t *task;

	BUZZZ_LVL5(HND_SCHED_WORK, 2, (uint32)taskfn, delay);

	if (!(task = hnd_timer_create(context, data, schedule_work_timer_cb,
	                              (hnd_timer_auxfn_t)taskfn))) {
		return BCME_NORESOURCE;
	}

	if (hnd_timer_start(task, delay, FALSE)) {
		return BCME_OK;
	}

	hnd_timer_free(task);
	return BCME_ERROR;
}

/* Sync time with host */
static uint32 host_reftime_delta_ms = 0;

void
BCMRAMFN(hnd_set_reftime_ms)(uint32 reftime_ms)
{
	host_reftime_delta_ms = reftime_ms - hnd_time();
}

uint32
BCMRAMFN(hnd_get_reftime_ms)(void)
{
	return (hnd_time() + host_reftime_delta_ms);
}

/* ======================= assert ======================= */
/* Global ASSERT type */
uint32 g_assert_type = 0;

/* accessor functions */
uint32
BCMRAMFN(get_g_assert_type)(void)
{
	return g_assert_type;
}

void
BCMRAMFN(set_g_assert_type)(uint32 val)
{
	g_assert_type = val;
}

si_t*
BCMRAMFN(get_hnd_sih)(void)
{
	return hnd_sih;
}

void
BCMRAMFN(set_hnd_sih)(si_t *val)
{
	hnd_sih = val;
}

#if defined(BCMDBG_ASSERT)

void
hnd_assert(const char *file, int line)
{
	BUZZZ_LVL1(HND_ASSERT, 2, (uint32)__builtin_return_address(0), line);
	BCM_BUZZZ_STOP();

	/* Format of ASSERT message standardized for automated parsing */
	printf("ASSERT in file %s line %d (ra %p, fa %p)\n",
	       file, line,
	       __builtin_return_address(0), __builtin_frame_address(0));

#ifdef BCMDBG_ASSERT_TYPE
	if (get_g_assert_type() != 0)
		return;
#endif // endif

#ifdef BCMDBG_ASSERT
#if defined(BCMSDIODEV_ENABLED)
	/* Fill in structure that be downloaded by the host */
	sdpcm_shared.flags           |= SDPCM_SHARED_ASSERT;
	sdpcm_shared.assert_exp_addr  = 0;
	sdpcm_shared.assert_file_addr = (uint32)file;
	sdpcm_shared.assert_line      = (uint32)line;
#endif /* BCMSDIODEV_ENABLED */
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		/* Fill in structure that be downloaded by the host */
		pciedev_shared.flags           |= PCIE_SHARED_ASSERT;
		pciedev_shared.assert_exp_addr  = 0;
		pciedev_shared.assert_file_addr = (uint32)file;
		pciedev_shared.assert_line      = (uint32)line;
	}
#endif /* BCMPCIEDEV */
#if defined(BCMM2MDEV_ENABLED)
	/* Fill in structure that be downloaded by the host */
	m2m_shared.flags           |= M2M_SHARED_ASSERT;
	m2m_shared.assert_exp_addr  = 0;
	m2m_shared.assert_file_addr = (uint32)file;
	m2m_shared.assert_line      = (uint32)line;
#endif /* BCMM2MDEV_ENABLED */

	ASSERT_WITH_TRAP(0);
#endif /* BCMDBG_ASSERT */
}

#ifdef BCMDBG_ASSERT_TEST
static void
asserttest(void *arg, int argc, char *argv[])
{
	ASSERT(0);
}
#endif /* BCMDBG_ASSERT_TEST */
#endif // endif

/* ============================= system ============================= */

#ifndef ATE_BUILD
void
hnd_poll(si_t *sih)
{
#ifdef RTE_POLL
	hnd_run_timeouts();
	hnd_dev_poll();
#else
#ifdef BCMDBG_CPU
	set_arm_inttimer(enterwaittime);
#endif // endif
	hnd_wait_irq(sih);
#endif /* RTE_POLL */
}
#endif /* !ATE_BUILD */

/*
 * Note that since the following function, uses the compile time macros
 * like BCMSDIODEV_ENABLED etc which are not gauranteed
 * to be defined for ROM builds, this function is Non-ROM able. Hence its
 * forced to be in RAM using BCMRAMFN.
 */
static void
BCMRAMFN(hnd_write_shared_addr_to_host)(void)
{
	uint32 asm_stk_bottom = 0;
#if !defined(__ARM_ARCH_7R__) && !defined(__ARM_ARCH_7A__)
	asm_stk_bottom = _memsize;
#else
	asm_stk_bottom = _rambottom;
#endif // endif
	/*
	 * Shared structures are used ONLY in case of PCIe, SDIO
	 * and MM2M buses. In case of USB (and other buses if any) the
	 * shared structure is not used, so do this only for PCIe, SDIO
	 * and MM2M case.
	 */
	BCM_REFERENCE(asm_stk_bottom);
#if defined(__arm__) && defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		*(uint32*)(asm_stk_bottom - 4) = (uint32)&pciedev_shared;
	}
#endif /* __arm__  && BCMPCIEDEV */

#if defined(__arm__) && defined(BCMSDIODEV_ENABLED)
	*(uint32*)(asm_stk_bottom - 4) = (uint32)&sdpcm_shared;
#endif /* __arm__  && BCMSDIODEV_ENABLED */

#if defined(__arm__) && defined(BCMM2MDEV_ENABLED)
	*(uint32*)(asm_stk_bottom - 4) = (uint32)&m2m_shared;
#endif /* __arm__  && BCMM2MDEV_ENABLED */
}

void
hnd_idle_init(si_t *sih)
{
	/*
	 * We come here after initialzing everything that is required
	 * to function well (i.e) after c_init(). So we can now write the shared
	 * structure's address to the Host.
	 *
	 * In case of PCIe Full Dongle case the DHD would be waiting in a loop
	 * until this is done.
	 */
	hnd_write_shared_addr_to_host();
#ifndef RTE_POLL
	hnd_set_irq_timer(0);	/* kick start timer interrupts driven by hardware */
	hnd_enable_interrupts();
#endif // endif
#ifdef BCMDBG_CPU
	set_arm_inttimer(enterwaittime);
#endif // endif
}

void
hnd_idle(si_t *sih)
{
	hnd_idle_init(sih);
	hnd_idle_loop(sih);
}

/* ============================ misc =========================== */
void
hnd_unimpl(void)
{
	printf("UNIMPL: ra=%p\n", __builtin_return_address(0));
	hnd_die();
}

/*
 * ======HND======  Initialize and background:
 *
 *	hnd_init: Initialize the world.
 */

#ifdef _RTE_SIM_
extern uchar *sdrambuf;
#endif // endif

#ifndef GLOBAL_STACK
extern uint32 _stackbottom;
#endif // endif

uint32 gFWID;

static void
BCMATTACHFN(hnd_shared_setup)(void)
{
	uint32 asm_ram_bottom = hnd_get_rambottom();
#if defined(BCMSDIODEV_ENABLED)
	uint32 assert_line = sdpcm_shared.assert_line;
#elif defined(BCMM2MDEV_ENABLED)
	uint32 assert_line = m2m_shared.assert_line;
#endif // endif

	(void)asm_ram_bottom;

#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		/* Initialize the structure shared between the host and dongle
		 * over the PCIE bus.  This structure is used for console output
		 * and trap/assert information to the host.  The last word in
		 * memory is overwritten with a pointer to the shared structure.
		 */
		uint32 assert_line = pciedev_shared.assert_line;
		memset(&pciedev_shared, 0, sizeof(pciedev_shared) - 4);
		pciedev_shared.flags = PCIE_SHARED_VERSION;
		pciedev_shared.assert_line = assert_line;
		pciedev_shared.fwid = gFWID;
#if defined(BCMDBG_ASSERT)
		pciedev_shared.flags |= PCIE_SHARED_ASSERT_BUILT;
#endif // endif
#ifdef BCMPCIE_SUPPORT_TX_PUSH_RING
		pciedev_shared.flags |= PCIE_SHARED_TXPUSH_SPRT;
#endif // endif
	}
#endif /* BCMPCIEDEV */

#if defined(BCMSDIODEV_ENABLED)
	/* Initialize the structure shared between the host and dongle
	 * over the SDIO bus.  This structure is used for console output
	 * and trap/assert information to the host.  The last word in
	 * memory is overwritten with a pointer to the shared structure.
	 */
	memset(&sdpcm_shared, 0, sizeof(sdpcm_shared));
	sdpcm_shared.flags = SDPCM_SHARED_VERSION;
	sdpcm_shared.assert_line = assert_line;
	sdpcm_shared.fwid = gFWID;
#if defined(BCMDBG_ASSERT)
	sdpcm_shared.flags |= SDPCM_SHARED_ASSERT_BUILT;
#endif // endif
#endif /* BCMSDIODEV_ENABLED */

#if defined(BCM_OL_DEV)
	/* Initialize the structure shared between the host and dongle
	 * over the PCIE  bus.  This structure is used for read host message buffer
	 * and trap/assert information to the host.  The last word in
	 * memory is overwritten with a pointer to the shared structure.
	 */
#if defined(__arm__)	/* Pointer supported only in startarm.S */
	ppcie_shared = (olmsg_shared_info_t *)(asm_ram_bottom - OLMSG_SHARED_INFO_SZ);
#endif // endif
#endif /* BCM_OL_DEV */

#if defined(BCMM2MDEV_ENABLED)
	/* Initialize the structure shared between the apps and wlan
	 * over the M2M bus.  This structure is used for console output
	 * and trap/assert information to the host.  The last word in
	 * memory is overwritten with a pointer to the shared structure.
	 */
	memset(&m2m_shared, 0, sizeof(m2m_shared));
	m2m_shared.flags = M2M_SHARED_VERSION;
	m2m_shared.assert_line = assert_line;
	m2m_shared.fwid = gFWID;
#if defined(BCMDBG_ASSERT)
	m2m_shared.flags |= M2M_SHARED_ASSERT_BUILT;
#endif // endif
#endif /* BCMM2MDEV_ENABLED */
}

static void
BCMATTACHFN(hnd_mem_setup)(uintptr stacktop)
{
	uchar *ramStart, *ramLimit;
	uintptr stackbottom;

	BCM_REFERENCE(stackbottom);

#if defined(GLOBAL_STACK)
	stackbottom = stacktop - HND_STACK_SIZE;
#else
	stackbottom = stacktop - SVC_STACK_SIZE;
#endif // endif

	/* Initialize malloc arena */
#if defined(EXT_CBALL)
	ramStart = (uchar *)RAMADDRESS;
	ramLimit = ramStart + RAMSZ;
#elif defined(_RTE_SIM_)
	ramStart = sdrambuf;
	ramLimit = ramStart + RAMSZ;
#elif defined(DEVRAM_REMAP)
	{
	extern char _heap_start[], _heap_end[];
	ramStart = (uchar *)_heap_start;
	ramLimit = (uchar *)_heap_end;
	}
#else /* !EXT_CBALL && !_RTE_SIM_ && !DEVRAM_REMAP */
	ramStart = (uchar *)_end;
#if defined(BCMUSBDEV) && defined(BCMUSBDEV_ENABLED) && defined(BCMTRXV2) && \
	!defined(BCM_BOOTLOADER)
	{
	uint32 asm_ram_bottom = hnd_get_rambottom();
	struct trx_header *trx;
	/* Check for NVRAM parameters.
	 * If found, initialize _vars and _varsz. Also, update ramStart
	 * last 4 bytes at the end of RAM is left untouched.
	 */
	trx = (struct trx_header *) (asm_ram_bottom -(SIZEOF_TRXHDR_V2 + 4));
	/* sanity checks */
	if (trx->magic == TRX_MAGIC && ISTRX_V2(trx) && trx->offsets[TRX_OFFSETS_NVM_LEN_IDX]) {
		_varsz = trx->offsets[TRX_OFFSETS_NVM_LEN_IDX];
		_vars = (char *)(text_start + trx->offsets[TRX_OFFSETS_DLFWLEN_IDX]);
		/* overriding ramStart initialization */
		ramStart = (uchar *)_vars + trx->offsets[TRX_OFFSETS_NVM_LEN_IDX] +
			trx->offsets[TRX_OFFSETS_DSG_LEN_IDX] +
			trx->offsets[TRX_OFFSETS_CFG_LEN_IDX];
	}
	}
#endif /* BCMUSBDEV && BCMUSBDEV_ENABLED && BCMTRXV2 && !BCM_BOOTLOADER */
	ramLimit = (uchar *)stackbottom;
#ifdef STACK_PROT_TRAP
	hnd_stack_prot(ramLimit);
#endif // endif
#endif /* !EXT_CBALL && !_RTE_SIM_ && !DEVRAM_REMAP */

#ifdef GLOBAL_STACK
	hnd_stack_init((uint32 *)stackbottom, (uint32 *)stacktop);
#endif // endif
	hnd_arena_init((uintptr)ramStart, (uintptr)ramLimit);

#ifdef HNDLBUFCOMPACT
	hnd_lbuf_fixup_2M_tcm();
#endif // endif
}

si_t *
BCMATTACHFN(hnd_init)(void)
{
	si_t *sih;
#ifdef GLOBAL_STACK
	uint32 stackBottom = 0xdeaddead;
#endif // endif
#ifdef BCM_DHDHDR
	int mutxmax;
#endif // endif

	/* ******** system init ******** */

	hnd_disable_interrupts();

	/* Initialize trap handling */
	hnd_trap_init();

	/* Initialize shared data area between host and dongle */
	hnd_shared_setup();

#ifdef GLOBAL_STACK
	hnd_mem_setup((uintptr)&stackBottom);
#else
	hnd_mem_setup((uintptr)_stackbottom);
#endif // endif

	/* ******** hardware init ******** */

	/* Now that we have initialized memory management let's allocate the osh */

#ifdef SHARED_OSL_CMN
	hnd_osh = osl_attach(NULL, NULL);
#else
	hnd_osh = osl_attach(NULL);
#endif /* SHARED_OSL_CMN */
	ASSERT(hnd_osh);

	/* Initialize ISR and DPC */
	hnd_isr_module_init(hnd_osh);

#if defined(BCM_OL_DEV) || defined(RTE_CONS)
	/* Create a logbuf separately from a console. This logbuf will be
	 * dumped and reused when the console is created later on.
	 */
	hnd_cons_log_init(hnd_osh);
#endif // endif

#ifdef RAMSIZE
	_ramsize_adj = rte_ramsize();
	printf("****** RAMSIZE is 0x%x, RAMSIZE_ADJ is 0x%x *******\n", RAMSIZE, RAMSIZE_ADJ);
#endif // endif

	/* Initialise the debug struct that sits in a known location */
	/* N.B: must be called after hnd_cons_log_init */
	hnd_debug_info_init();

	/* Scan backplane */
	set_hnd_sih(si_kattach(hnd_osh));
	sih = get_hnd_sih();
	ASSERT(sih);

#ifdef RTE_CACHED
	/* Initialize coherent and caches */
	hnd_caches_init(sih);
#endif // endif

	/* Initialize chipcommon related stuff */
	hnd_chipc_init(sih);

	/* Initialize CPU related stuff */
	hnd_cpu_init(sih);

	/* Initialize timer/time */
	hnd_timer_init(sih);

	/* Initialize deadman timer after hnd_timer_init */
	hnd_cpu_deadman_init(sih);

#ifdef RTE_CONS
	/* No printf's go to the UART earlier than this */
#ifndef BCM4350_FPGA
	(void)serial_init_devs(sih, hnd_osh);
	(void)hnd_cons_init(sih, hnd_osh);
#endif // endif
#endif /* RTE_CONS */

#ifdef	SBPCI
	/* Init pci core if there is one */
	hnd_dev_init_pci(sih, hnd_osh);
#endif // endif

#ifdef BCMECICOEX
	/* Initialize ECI registers */
	hnd_eci_init(sih);
#endif // endif

#ifdef WLGPIOHLR
	/* Initialize GPIO */
	rte_gpio_init(sih);
#endif // endif
	hnd_gci_init(sih);

#ifdef HND_PT_GIANT
	mem_pt_init(hnd_osh);
#endif // endif

	_is_mutephytxerror = (getvar(NULL, "mutephytxerror") == NULL) ?
		FALSE : getintvar(NULL, "mutephytxerror");
	if (_is_mutephytxerror)
		printf("%s: _is_mutephytxerror %d \n", __FUNCTION__, _is_mutephytxerror);
#ifdef BCM_DHDHDR
	mutxmax = (getvar(NULL, "mutxmax") == NULL) ? -1 : getintvar(NULL, "mutxmax");

	/* For 4366B1, we can only support MU-0 and MU-2 */
	if (BCM4365_CHIP(sih->chip) && CHIPREV(sih->chiprev) == 3) {
		if (mutxmax != 0 && mutxmax != 2)
			mutxmax = 2;
	}

	if (getvar(NULL, "dhdhdr") != NULL) {
		/* if specified in nvram, use that always */
		_bcmdhdhdr = getintvar(NULL, "dhdhdr");
	} else {
		/* if not specified in nvram, choose based on mutxtmax */
		if (mutxmax == 0 || mutxmax == 2) {
			_bcmdhdhdr = FALSE;
		} else {
			_bcmdhdhdr = TRUE;
		}
	}

	/* opt #2 and #3 are based upon #1 */
#ifdef HOST_HDR_FETCH
	if (_bcmdhdhdr) {
		/* opt #2 and #3 are enabled for MU-4 and MU-8 by default */
		_hosthdrfetch = (getvar(NULL, "htxhdr") == NULL) ? TRUE : getintvar(NULL, "htxhdr");
	} else {
		_hosthdrfetch = FALSE;
	}
#endif /* HOST_HDR_FETCH */
#ifdef AMSDU_FRAG_OPT
	if (_bcmdhdhdr) {
		/* opt #3 is enabled/disabled as same as opt #2 if not specified */
		_amsdufrag = (getvar(NULL, "amsdufrag") == NULL) ?
			_hosthdrfetch : getintvar(NULL, "amsdufrag");
	} else {
		_amsdufrag = FALSE;
	}
#endif /* AMSDU_FRAG_OPT */

	printf("%s: mutxmax %d dhdhdr %u htxhdr %u amsdufrag %u\n", __FUNCTION__, mutxmax,
		BCMDHDHDR_ENAB(), HOST_HDR_FETCH_ENAB(), AMSDUFRAG_ENAB());
#endif /* BCM_DHDHDR */

#if defined(BCMPKTIDMAP)
	/*
	 * Initialize the pktid to pktptr map prior to constructing pktpools,
	 * As part of constructing the pktpools a few packets will ne allocated
	 * an placed into the pools. Each of these packets must have a packet ID.
	 */
	hnd_pktid_init(hnd_osh, PKT_MAXIMUM_ID - 1);
#endif // endif
#if defined(BCMPKTPOOL) && defined(BCMPKTPOOL_ENABLED)
	rte_pktpool_init(hnd_osh);
#endif // endif
	hnd_fetch_module_init(hnd_osh);
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		hnd_pktfetch_module_init(get_hnd_sih(), hnd_osh);
	}
#endif /* BCMPCIEDEV */

	/* Add a few commands */
#if defined(RTE_CONS) && !defined(BCM_BOOTLOADER)
	hnd_cons_add_cmd("dump", hnd_dump_test, (void *)2);
	hnd_cons_add_cmd("mw", hnd_memwaste, 0);
	hnd_cons_add_cmd("md", hnd_memdmp, 0);
#ifdef BCMTRAPTEST
	hnd_cons_add_cmd("tr", traptest, 0);
#endif // endif
#ifdef BCMDBG_ASSERT_TEST
	hnd_cons_add_cmd("as", asserttest, 0);
#endif // endif
#endif /* RTE_CONS  && !BCM_BOOTLOADER */

	hnd_mem_cli_init();
	hnd_heap_cli_init();
	hnd_timer_cli_init();

	return get_hnd_sih();
}
