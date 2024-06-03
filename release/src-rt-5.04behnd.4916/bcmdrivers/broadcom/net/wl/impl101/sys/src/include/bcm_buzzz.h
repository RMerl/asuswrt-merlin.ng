#ifndef __bcm_buzzz_h_included__
#define __bcm_buzzz_h_included__

/*
 * +----------------------------------------------------------------------------
 *
 * BCM BUZZZ Performance tracing tool for ARM Cortex-R4, Cortex-M3
 *
 * BCM_BUZZZ_CYCLES_PER_USEC : Specify Processor speed
 * BCM_BUZZZ_LOG_BUFSIZE     : Specify log buffer size
 *
 * BCM_BUZZZ_LOG_LEVEL**     : Specify default Log tracing level
 * BCM_BUZZZ_KPI_LEVEL**     : Specify Key Performace Indication tracing level
 *
 * BCM_BUZZZ_LOG_LEVEL       : enables BUZZZ_LVL#(), where # is the level.
 * BCM_BUZZZ_KPI_LEVEL       : enables 2 forms of KPIs logging levels, namely,
 *  + BCM_BUZZZ_KPI_PKT_LEVEL: every packet is tracked through the driver using
 *                             BUZZZ_KPI_PKT#()
 *  + BCM_BUZZZ_KPI_QUE_LEVEL: HW-FIFO and SW-Queue operations are tracked using
 *                             BUZZZ_KPI_QUE#()
 * BCM_BUZZZ_SPP_LEVEL       : SubPPDUProcessing: Tracking the scheduling of
 *                             TxPost, TxStatus and RxProcessing datapaths
 *                             based on Dongle packet storage availability,
 *                             using BUZZZ_SPP#()
 *
 * (**) BCM_BUZZZ_LAT (Latency KPI) overrides all LOG, KPI and SPP Levels.
 *      There is no levels for BCM_BUZZZ_LAT.
 *
 * There are no levels for BCM_BUZZZ_LAT, at this time.
 *
 * As a cortex-a7 is used for several chips running at different Mhz, use ram.mk
 * to define the CYCLES_PER_USEC, which is also used by HND CPU Util tool.
 *
 *
 * BUZZZ Streaming Mode is enabled by default (BCM_BUZZZ_STREAMING_BUILD)
 * A maximum 3 arguments (2 x uint32) and (1 x uint16) may be logged.
 * Non-streaming mode may be used for debug, with dongle circular log buffer
 * enabled in wrap mode, until an HALT.
 *
 * BCM_BUZZZ_4ARGS is deprecated. To log 4 arguments, invoke back-to-back log
 *
 *
 * This file is shared between DHD and Dongle firmware.
 * Several implementations of buzzz exist:
 * - router linux: "BUZZZ"
 * - standalone for proprietary 3rd party OS: "DHD_BUZZ"
 * - dongle builds: BCM_BUZZZ (as-in this header file).
 * All exported APIs in dongle buzzz take the bcm_buzzz prefix and logging
 * macros use BUZZZ_LVL#(). Tracing macros are not prefixed with "BCM_"
 *
 *
 * Reference dongle/shared/startarm.S, to pick the DFLAGS for -march and -mcpu
 *
 *    __ARM_ARCH_7R__    : CR4, hndrte  -march=armv7-r -mcpu=cortex-r4
 *    __ARM_ARCH_7M__    : CM3, bmos    -march=armv7-m -mcpu=cortex-m3
 *    __ARM_ARCH_7A__ CA7: CA7, threadX -march=armv7-a -mcpu=cortex-a7
 *    __ARM_ARCH_7A__ CA9: not reqd for dongle, uses ARMv7 apis as in CA7
 *    __ARM_ARCH_8A__ CA53: -mcpu=cortex-a53 built in __ARM_32BIT_STATE (6717)
 *
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: bcm_buzzz.h 828277 2023-08-05 09:37:04Z $
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * +----------------------------------------------------------------------------
 */

#define BCM_BUZZZ_RET_NONE              (0)
#define BCM_BUZZZ_NULL_STMT             do { /* Nothing */ } while(0)

#define BCM_BUZZZ_VERSION               (4)
#define BCM_BUZZZ_HDRTYPE_NIC           (0x80)

/* Only include events that use up max 2 args */
#if defined(BCM_BUZZZ_FUNC)
#undef BCM_BUZZZ_4ARGS
#endif

#if defined(BCM_BUZZZ)

#define BCM_BUZZZ_STREAMING_BUILD
#define BCM_BUZZZ_STREAMING_FILE        "/tmp/buzzz.log"
#define BCM_BUZZZ_STREAMING_MAXSIZE     (38 * BCM_BUZZZ_HOSTMEM_SEGSZ) /* 152MB */

/* Buzzz Streaming to Host only verified for Dongle Cortex A7 with ThreadX */

#define BCM_BUZZZ_COUNTERS_MAX          (8)

#define BCM_BUZZZ_MODE_NONE             (0)
#define BCM_BUZZZ_MODE_FUNC             (1)
#define BCM_BUZZZ_MODE_EVENT            (2)

/*
 * Overhead was computed by making back to back bcm_buzzz_log() calls
 * You may estimate the overhead by making back-to-back calls to
 * bcm_buzzz_log0() and fill in these values, so the host side bcm_buzzz_dump
 * will appropriately subtract them in the log dump.
 */

/* __ARM_ARCH_7R__ : -march=armv7-r and -mcpu=cortex-cr4 */
#define BCM_BUZZZ_CR4_CYCLECNT_OVHD     (80)
#define BCM_BUZZZ_CR4_INSTRCNT_OVHD     (34)
#define BCM_BUZZZ_CR4_BRMISPRD_OVHD     (1)

/* __ARM_ARCH_7M__ : -march=armv7-m and -mcpu=cortex-cm3 */
#define BCM_BUZZZ_CM3_CYCCNT_OVHD       (100) /* 114 - 138 */
#define BCM_BUZZZ_CM3_CPICNT_OVHD       (4)
#define BCM_BUZZZ_CM3_EXCCNT_OVHD       (0)
#define BCM_BUZZZ_CM3_SLEEPCNT_OVHD     (0)
#define BCM_BUZZZ_CM3_LSUCNT_OVHD       (90)
#define BCM_BUZZZ_CM3_FOLDCNT_OVHD      (0)

/* __ARM_ARCH_7A__ : -march=armv7-a */
/* -mcpu=cortex-a7 -DCA7 : Instrumentation overheads not estimated, i.e. 0 */
#define BCM_BUZZZ_CA7_INSTRCNT_OVHD     (0)
#define BCM_BUZZZ_CA7_CYCCNT_OVHD       (0)
#define BCM_BUZZZ_CA7_BRMISS_OVHD       (0)
#define BCM_BUZZZ_CA7_DATAMEMACC_OVHD   (0)

/* __ARM_ARCH_8A__ : -march=armv8-a */
/* -mcpu=cortex-a53 -DCA53 : Instrumentation overheads not estimated, i.e. 0 */
#define BCM_BUZZZ_CA53_INSTRCNT_OVHD    (0)
#define BCM_BUZZZ_CA53_CYCCNT_OVHD      (0)
#define BCM_BUZZZ_CA53_BRMISS_OVHD      (0)
#define BCM_BUZZZ_CA53_DATAMEMACC_OVHD  (0)

extern uint32 cycles_per_usec_g;

/* -------------------------------------------------------------------------- */

/*
 * +----------------------------------------------------------------------------
 * ARM Cortex CR4, HNDRTE configuration
 * +----------------------------------------------------------------------------
 */
#if defined(__ARM_ARCH_7R__)
#define BCM_BUZZZ_HNDRTE
#define BCM_BUZZZ_CONFIG_CPU_ARM_CR4                   /* -mcpu=cortex-cr4 */
#define BCM_BUZZZ_CYCLES_PER_USEC       (cycles_per_usec_g)
#define BCM_BUZZZ_LOG_BUFSIZE           (4 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_COUNTERS              (3)            /* 3 PMU counters */

/*
 * Event ref. value definitions from the CR4 TRM.
 * Please check the TRM for more event definitions.
 */
#define BCM_BUZZZ_ARMCR4_SWINC_EVT		(0x00)	/* Software increment */
#define BCM_BUZZZ_ARMCR4_ICACHEMISS_EVT	(0x01)	/* Intruction cache miss */
#define BCM_BUZZZ_ARMCR4_DCACHEMISS_EVT	(0x03)	/* Data cache miss */
#define BCM_BUZZZ_ARMCR4_DATAREAD_EVT	(0x06)	/* Data read executed */
#define BCM_BUZZZ_ARMCR4_DATAWRITE_EVT	(0x07)	/* Data write executed */
#define BCM_BUZZZ_ARMCR4_INSTRCNT_EVT	(0x08)	/* Instruction executed */
#define BCM_BUZZZ_ARMCR4_EXPCNT_EVT		(0x09)	/* Exception taken */
#define BCM_BUZZZ_ARMCR4_EXPRTN_EVT		(0x0a)	/* Exception return executed */
#define BCM_BUZZZ_ARMCR4_CTXIDCHG_EVT	(0x0b)	/* Context ID change executed */
#define BCM_BUZZZ_ARMCR4_SWCHGPC_EVT	(0x0c)	/* PC change executed */
#define BCM_BUZZZ_ARMCR4_BICNT_EVT		(0x0d)	/* B/BL/BLX immed executed */
#define BCM_BUZZZ_ARMCR4_PROCRTN_EVT	(0x0e)	/* Procedure return executed */
#define BCM_BUZZZ_ARMCR4_UNALIGNED_EVT	(0x0f)	/* Unaligned access executed */
#define BCM_BUZZZ_ARMCR4_BRMISS_EVT		(0x10)	/* Branch mispredictions */
#define BCM_BUZZZ_ARMCR4_CYCLECNT_EVT	(0x11)	/* Cycle count */
#define BCM_BUZZZ_ARMCR4_BRHIT_EVT		(0x12)	/* Branches predicted */
#endif /* __ARM_ARCH_7R__ */

/*
 * +----------------------------------------------------------------------------
 * ARM Cortex CM3, BMOS (Bare Metal OS) configuration
 * +----------------------------------------------------------------------------
 */
#if defined(__ARM_ARCH_7M__)
#define BCM_BUZZZ_BMOS                                 /* Bare Metal OS */
#define BCM_BUZZZ_CONFIG_CPU_ARM_CM3                   /* -mcpu=cortex-m3 */
#define BCM_BUZZZ_CYCLES_PER_USEC       (cycles_per_usec_g)
#define BCM_BUZZZ_LOG_BUFSIZE           (4 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_COUNTERS              (6)            /* 6 PMU counters */
#endif /* __ARM_ARCH_7M__ */

#if defined(__ARM_ARCH_7A__)
/*
 * +----------------------------------------------------------------------------
 * ARMv7 Performance Monitoring Unit Event listing
 * +----------------------------------------------------------------------------
 */

/*
 * Event ref. value definitions from the ARMv7 Architecture Reference Manual
 * Definition of terms and events in Section C12.8 ARM DDI 0406C.b ID072512
 *
 * Please check each Cortex-A# TRM for more event definitions.
 */
#define BCM_BUZZZ_ARMV7_SWINC_EVT       (0x00)  /* Software increment */
#define BCM_BUZZZ_ARMV7_ICACHEMISS_EVT  (0x01)  /* Intruction cache miss */
#define BCM_BUZZZ_ARMV7_DCACHEMISS_EVT  (0x03)  /* L1 Data cache miss */
#define BCM_BUZZZ_ARMV7_DCACHEACC_EVT   (0x04)  /* L1 data cache access */
#define BCM_BUZZZ_ARMV7_DATAREAD_EVT    (0x06)  /* Data read executed */
#define BCM_BUZZZ_ARMV7_DATAWRITE_EVT   (0x07)  /* Data write executed */
#define BCM_BUZZZ_ARMV7_INSTRCNT_EVT    (0x08)  /* Instruction executed */
#define BCM_BUZZZ_ARMV7_EXPCNT_EVT      (0x09)  /* Exception taken */
#define BCM_BUZZZ_ARMV7_EXPRTN_EVT      (0x0a)  /* Exception return executed */
#define BCM_BUZZZ_ARMV7_CTXIDCHG_EVT    (0x0b)  /* Context ID change executed */
#define BCM_BUZZZ_ARMV7_SWCHGPC_EVT     (0x0c)  /* PC change executed */
#define BCM_BUZZZ_ARMV7_BICNT_EVT       (0x0d)  /* B/BL/BLX immed executed */
#define BCM_BUZZZ_ARMV7_PROCRTN_EVT     (0x0e)  /* Procedure return executed */
#define BCM_BUZZZ_ARMV7_UNALIGNED_EVT   (0x0f)  /* Unaligned access executed */
#define BCM_BUZZZ_ARMV7_BRMISS_EVT      (0x10)  /* Branch mispredicted */
#define BCM_BUZZZ_ARMV7_CYCLECNT_EVT    (0x11)  /* Cycle count */
#define BCM_BUZZZ_ARMV7_BRHIT_EVT       (0x12)  /* Branches predicted */
#define BCM_BUZZZ_ARMV7_DATAMEMACC_EVT  (0x13)  /* Data memory access */
#define BCM_BUZZZ_ARMV7_ICACHEACC_EVT   (0x14)  /* Instruction cache access */
#define BCM_BUZZZ_ARMV7_DCACHEEVICT_EVT (0x15)  /* Data cache eviction */
#define BCM_BUZZZ_ARMV7_2DCACHEACC_EVT  (0x16)  /* L2 data cache access */
#define BCM_BUZZZ_ARMV7_2DCACHEMISS_EVT (0x17)  /* L2 data cache misses */
#define BCM_BUZZZ_ARMV7_2DCACHEWB_EVT   (0x18)  /* L2 data cache write back */
#define BCM_BUZZZ_ARMV7_BUSSACC_EVT     (0x19)  /* Bus accesses */
#define BCM_BUZZZ_ARMV7_BUSCYCLE_EVT    (0x1d)  /* Bus cycle */
#define BCM_BUZZZ_ARMV7_BUSRDACC_EVT    (0x60)  /* Bus access read */
#define BCM_BUZZZ_ARMV7_BUSWRACC_EVT    (0x61)  /* Bus access write */
#define BCM_BUZZZ_ARMV7_SPECINST_EVT    (0x68)  /* Speculative instr executed */
#define BCM_BUZZZ_ARMV7_IRQEXCP_EVT     (0x86)  /* IRQ exceptions taken */
#define BCM_BUZZZ_ARMV7_FIQEXCP_EVT     (0x87)  /* FIQ expceptions taken */
#define BCM_BUZZZ_ARMV7_EXTMEMREQ_EVT   (0xc0)  /* External memory request */
#define BCM_BUZZZ_ARMV7_NC_EXTMEMREQ_EVT (0xc1) /* Non cacheable ext mem req */
#define BCM_BUZZZ_ARMV7_PREF_LFILL_EVT  (0xc2)  /* Prefetch line fill */
#define BCM_BUZZZ_ARMV7_PREF_DROP_EVT   (0xc3)  /* Prefetch line fill dropped */
#define BCM_BUZZZ_ARMV7_WR_STALL_EVT    (0xc9)  /* Wipeline stall, WR store full */
#define BCM_BUZZZ_ARMV7_DATASNOOP_EVT   (0xca)  /* Data snooped fr other proc */

#if defined(CA7)
/*
 * +----------------------------------------------------------------------------
 * ARM Cortex CA7, ThreadX configuration
 * +----------------------------------------------------------------------------
 */
#define BCM_BUZZZ_THREADX
#define BCM_BUZZZ_CONFIG_CPU_ARM_CA7                   /* -mcpu=cortex-a7 */
#define BCM_BUZZZ_CYCLES_PER_USEC       (cycles_per_usec_g)
#define BCM_BUZZZ_LOG_BUFSIZE           (8 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_COUNTERS              (4)            /* 4 PMU counters */
#endif /* CA7 */

#if defined(CA9)
/* Stuff for ARM Cortex-A9 */
#endif

#endif /* __ARM_ARCH_7A__ */

#if defined(__ARM_ARCH_8A__)
/*
 * +----------------------------------------------------------------------------
 * ARMv8 Performance Monitoring Unit Event listing
 * +----------------------------------------------------------------------------
 */

/*
 * Event ref. value definitions from the ARMv8 Architecture Reference Manual
 * Definition of terms and events in Section C12.9 ARM events
 * DDI0500E_cortex_a53_r0p3_trm.pdf
 *
 * Please check each Cortex-A# TRM for more event definitions.
 *
 * Reference section C12.6 for AArch32
 * Cortex53 Events not supported in AArch32 mode are commented out.
 * Events commencing at 0xc0 have not been evaluated
 *
 * PMCEID0: 0x67ffbfff PMCEID1: 0x00000000
 */
#define BCM_BUZZZ_ARMV8_SWINC_EVT       (0x00)  // Software increment
#define BCM_BUZZZ_ARMV8_ICACHEMISS_EVT  (0x01)  // L1 Intruction cache refill
#define BCM_BUZZZ_ARMV8_ITLBMISS_EVT    (0x02)  // L1 Instruction TLB refill
#define BCM_BUZZZ_ARMV8_DCACHEMISS_EVT  (0x03)  // L1 Data cache miss
#define BCM_BUZZZ_ARMV8_DCACHEACC_EVT   (0x04)  // L1 data cache access
#define BCM_BUZZZ_ARMV8_DTLBMISS_EVT    (0x05)  // L1 Data TLB refill
#define BCM_BUZZZ_ARMV8_DATAREAD_EVT    (0x06)  // Data read executed
#define BCM_BUZZZ_ARMV8_DATAWRITE_EVT   (0x07)  // Data write executed
#define BCM_BUZZZ_ARMV8_INSTRCNT_EVT    (0x08)  // Instruction executed
#define BCM_BUZZZ_ARMV8_EXPCNT_EVT      (0x09)  // Exception taken
#define BCM_BUZZZ_ARMV8_EXPRTN_EVT      (0x0a)  // Exception return executed
#define BCM_BUZZZ_ARMV8_CTXIDCHG_EVT    (0x0b)  // Context ID change executed
#define BCM_BUZZZ_ARMV8_SWCHGPC_EVT     (0x0c)  // PC change executed
#define BCM_BUZZZ_ARMV8_BRIMMED_EVT     (0x0d)  // B/BL/BLX immed executed
//      BCM_BUZZZ_ARMV8_PROCRTN_EVT      0x0e      Not implemented in AArch32
#define BCM_BUZZZ_ARMV8_UNALIGNED_EVT   (0x0f)  // Unaligned access executed
#define BCM_BUZZZ_ARMV8_BRMISS_EVT      (0x10)  // Branch mispredicted
#define BCM_BUZZZ_ARMV8_CYCLECNT_EVT    (0x11)  // Cycle count
#define BCM_BUZZZ_ARMV8_BRHIT_EVT       (0x12)  // Branches predicted
#define BCM_BUZZZ_ARMV8_DATAMEMACC_EVT  (0x13)  // Data memory access
#define BCM_BUZZZ_ARMV8_ICACHEACC_EVT   (0x14)  // Instruction cache access
#define BCM_BUZZZ_ARMV8_DCACHEEVICT_EVT (0x15)  // L1 Data cache write back
#define BCM_BUZZZ_ARMV8_L2DCACHEACC_EVT (0x16)  // L2 data cache access
#define BCM_BUZZZ_ARMV8_L2DCACHEMISS_EVT (0x17) // L2 data cache refill
#define BCM_BUZZZ_ARMV8_L2DCACHEWB_EVT  (0x18)  // L2 data cache write back
#define BCM_BUZZZ_ARMV8_BUSACC_EVT      (0x19)  // Bus accesses
#define BCM_BUZZZ_ARMV8_MEMERR_EVT      (0x1a)  // Local Memory Error
//      BCM_BUZZZ_ARMV8_SPECINST_EVT     0x1b      Not implemented in AArch32
//      BCM_BUZZZ_ARMV8_TTBRWR_EVT       0x1c      Not implemented in AArch32
#define BCM_BUZZZ_ARMV8_BUSCYCLE_EVT    (0x1d)  // Bus cycle
#define BCM_BUZZZ_ARMV8_CHAIN           (0x1e)  // Count in Odd, even Ctr ovflw
//      BCM_BUZZZ_ARMV8_L1DALLOC_EVT     0x1f      Not implemented

#define BCM_BUZZZ_ARMV8_BUSRDACC_EVT    (0x60)  // Bus access read
#define BCM_BUZZZ_ARMV8_BUSWRACC_EVT    (0x61)  // Bus access write
//      BCM_BUZZZ_ARMV8_BISPEC_EVT       0x7a      BrIndirect Spec executed
#define BCM_BUZZZ_ARMV8_IRQEXCP_EVT     (0x86)  // IRQ exceptions taken
#define BCM_BUZZZ_ARMV8_FIQEXCP_EVT     (0x87)  // FIQ expceptions taken
#define BCM_BUZZZ_ARMV8_EXTMEMREQ_EVT   (0xc0)  // External memory request
#define BCM_BUZZZ_ARMV8_NC_EXTMEMREQ_EVT (0xc1) // Non cacheable ext mem req
#define BCM_BUZZZ_ARMV8_PREF_LFILL_EVT  (0xc2)  // Prefetch line fill
#define BCM_BUZZZ_ARMV8_ICACHETHROT_EVT (0xc3)  // ICache throttle occurred
#define BCM_BUZZZ_ARMV8_RDALLENTER_EVT  (0xc4)  // Entering RD Allocate Mode
#define BCM_BUZZZ_ARMV8_RDALLMODE_EVT   (0xc5)  // RD Allocate Mode
#define BCM_BUZZZ_ARMV8_PREDECERR_EVT   (0xc6)  // Pre-decode Error
#define BCM_BUZZZ_ARMV8_WRSTALL_EVT     (0xc7)  // Pipeline stall WR store full
#define BCM_BUZZZ_ARMV8_DATASNOOP_EVT   (0xc8)  // Data snooped fr other proc
#define BCM_BUZZZ_ARMV8_BRCOND_EVT      (0xc9)  // Conditional Branch executed
#define BCM_BUZZZ_ARMV8_BRINDMISPRED_EVT (0xca) // Indirect Branch mispredicted
#define BCM_BUZZZ_ARMV8_BRINDMISPREDADDRMISCMP_EVT (0xcb) // Address Miscmp
#define BCM_BUZZZ_ARMV8_BRCONDMISPRED_EVT (0xcc) // Cond Branch mispredicted
#define BCM_BUZZZ_ARMV8_L1IMEMERR_EVT   (0xd0)  // L1 I-Cache memory error
#define BCM_BUZZZ_ARMV8_L1DMEMERR_EVT   (0xd1)  // D1 D-Cache memory error
#define BCM_BUZZZ_ARMV8_TLBMEMERR_EVT   (0xd2)  // TLB memory error

#if defined(CA53) && defined(__ARM_32BIT_STATE) // Dongle BCM6717
/*
 * +----------------------------------------------------------------------------
 *  -DBCM6717: ARM Cortex CA53 in 32b state, ThreadX configuration
 * +----------------------------------------------------------------------------
 */
#define BCM_BUZZZ_THREADX
#define BCM_BUZZZ_CONFIG_CPU_ARM_CA53_AARCH32          /* -mcpu=cortex-a53 */
#define BCM_BUZZZ_CYCLES_PER_USEC       (cycles_per_usec_g)
#define BCM_BUZZZ_LOG_BUFSIZE           (8 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_COUNTERS              (6)            /* 6 PMU counters */
#endif /* CA53 */

#endif /* __ARM_ARCH_8A__ && __ARM_32BIT_STATE */

/*
 * +----------------------------------------------------------------------------
 *
 *  BUZZZ System Tracing and KPI Build Levels
 *
 *  For Packet Latency, System and KPI logs are not required and tracing is
 *  explicitly disabled to avoid buffer overflows.
 *
 * +----------------------------------------------------------------------------
 */
#if defined(BCM_BUZZZ)
#define BCM_BUZZZ_LOG_LEVEL             (3) // RTE, HND, WL tracing level
#endif

#if defined(BCM_BUZZZ_KPI)
#define BCM_BUZZZ_KPI_LEVEL             (3) // KPI: Packet and Queue
#endif  /* BCM_BUZZZ_KPI */

#if defined(BCM_BUZZZ_SPP)
#define BCM_BUZZZ_SPP_LEVEL             (3) // KPI: Sub PPDU Processing Module
#else
#define BCM_BUZZZ_SPP_LEVEL             (0)
#endif  /* BCM_BUZZZ_SPP */

#if defined(BCM_BUZZZ_LAT)
/* Disable RTE/HND and KPI(PKT, QUE, SPP) runtime event logging */
#undef  BCM_BUZZZ_LOG_LEVEL
#undef  BCM_BUZZZ_KPI_LEVEL
#undef  BCM_BUZZZ_SPP_LEVEL
#define BCM_BUZZZ_LOG_LEVEL             (0) // Disable default Logging
#define BCM_BUZZZ_KPI_LEVEL             (0) // Disable PKT and QUE logging
#define BCM_BUZZZ_SPP_LEVEL             (0) // Disable SPP logging
#endif  /* BCM_BUZZZ_LAT */

#if defined(BCM_BUZZZ_KPI_LEVEL) && (BCM_BUZZZ_KPI_LEVEL > 0)
#define BCM_BUZZZ_KPI_PKT_LEVEL         (3) // KPI PKT datapath logging
#define BCM_BUZZZ_KPI_QUE_LEVEL         (3) // KPI QUE enqueue/dequeue logging
#else
#define BCM_BUZZZ_KPI_PKT_LEVEL         (0)
#define BCM_BUZZZ_KPI_QUE_LEVEL         (0)
#endif  /* BCM_BUZZZ_KPI_LEVEL */

/**
 * +----------------------------------------------------------------------------
 * Independent of a per KPI type level, a preamble snapshot may be taken.
 * Snapshot is placed in the BUZZZ Header (at the start of a buzzz trace).
 * Post-processing scripts may leverage these subsystem snapshot.
 *
 * Subsystems for which preambles snapshots are included at start of trace:
 *  - BUS (PCIe facing interfaces), presently TxPost Flowrings.
 *  - MAC (D11 LowerMAC facing interfaces), presently AQM & TxDMA TxFIFOs.
 *  - SCB (UpperMAC state), presently SCB state.
 * +----------------------------------------------------------------------------
 */
#if defined(BCM_BUZZZ_KPI) || defined(BCM_BUZZZ_SPP) || defined(BCM_BUZZZ_LAT)
#define BCM_BUZZZ_HDR_BUILD                 // preamble header logging enabled
#endif

/* Defines the number of arguments and their types */
#if defined(BCM_BUZZZ_STREAMING_BUILD)
#define BCM_BUZZZ_1ARGS
#if defined(BCM_BUZZZ_FUNC)
#define BCM_BUZZZ_2ARGS
#define BCM_BUZZZ_2ARG16                    // 2nd argument is a u16
#undef  BCM_BUZZZ_3ARGS
#else  /* !BCM_BUZZZ_FUNC */
#define BCM_BUZZZ_1ARGS
#define BCM_BUZZZ_2ARGS                     // 2nd argument is u32
#define BCM_BUZZZ_3ARGS                     // 3rd argument is u16
#endif /* !BCM_BUZZZ_FUNC */
#endif /* BCM_BUZZZ_STREAMING_BUILD */

/* BCM_BUZZZ_4ARGS is DEPRECATED */

/*
 * +----------------------------------------------------------------------------
 * Architecture independent definitions and APIs
 * +----------------------------------------------------------------------------
 */
#define BCM_BUZZZ_LOG_MAXSZ             (64)

#define BCM_BUZZZ_ERROR                 (-1)
#define BCM_BUZZZ_SUCCESS               (0)
#define BCM_BUZZZ_FAILURE               BCM_BUZZZ_ERROR
#define BCM_BUZZZ_ENABLED               (0)
#define BCM_BUZZZ_DISABLED              (1)
#define BCM_BUZZZ_FALSE                 (0)
#define BCM_BUZZZ_TRUE                  (1)
#define BCM_BUZZZ_INVALID               (~0U)

#define BCM_BUZZZ_INLINE                inline  __attribute__ ((always_inline))
#define BCM_BUZZZ_NOINSTR_FUNC          __attribute__ ((no_instrument_function))

void bcm_buzzz_log0(uint32 evtid);
void bcm_buzzz_log1(uint32 evtid, uint32 arg1);

#if defined(BCM_BUZZZ_2ARGS)
#if defined(BCM_BUZZZ_2ARG16)
void bcm_buzzz_log2(uint32 evtid, uint32 arg1, uint16 arg2); // FUNC mode
#else
void bcm_buzzz_log2(uint32 evtid, uint32 arg1, uint32 arg2);
#endif
#else  /* ! BCM_BUZZZ_2ARGS */
#define bcm_buzzz_log2(evt, arg1, arg2)             BCM_BUZZZ_NULL_STMT
#endif /* ! BCM_BUZZZ_2ARGS */

#if defined(BCM_BUZZZ_3ARGS)
#if defined(BCM_BUZZZ_4ARGS)
/* BCM_BUZZZ_4ARGS is DEPRECATED. For log3 , invoke log2 + log1 */
void bcm_buzzz_log3(uint32 evtid, uint32 arg1, uint32 arg2, uint32 arg3);
#else
void bcm_buzzz_log3(uint32 evtid, uint32 arg1, uint32 arg2, uint16 arg3);
#endif
#else  /* ! BCM_BUZZZ_3ARGS */
#define bcm_buzzz_log3(evt, arg1, arg2, arg3)       BCM_BUZZZ_NULL_STMT
#endif /* ! BCM_BUZZZ_3ARGS */

#if defined(BCM_BUZZZ_4ARGS)
/* BCM_BUZZZ_4ARGS is DEPRECATED. For log4 , invoke log2 + log2 */
void bcm_buzzz_log4(uint32 evtid, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4);
#else
#define bcm_buzzz_log4(evt, arg1, arg2, arg3, arg4) BCM_BUZZZ_NULL_STMT
#endif /* ! BCM_BUZZZ_4ARGS */

/** BUZZZ Exported APIs */
int   bcm_buzzz_init(void * shared);
void  bcm_buzzz_config(uint32 ctr_sel);
void  bcm_buzzz_show(void);
void  bcm_buzzz_dump(void);
void  bcm_buzzz_start(void);
void  bcm_buzzz_stop(void);
void  bcm_buzzz_wrap(uint32 val);
uint8 bcm_buzzz_status(void);
uint8 bcm_buzzz_mode(void);
void  bcm_buzzz_skip(uint8 skip);
void  bcm_buzzz_emit_counters(void);

void  bcm_buzzz_log_tsf(void);                                  // BCM_BUZZZ_LAT
void  bcm_buzzz_mac_read_tsf(uint32 *tsf_l, uint32 * tsf_h);    // BCM_BUZZZ_LAT

#if defined(BCM_BUZZZ_HDR_BUILD)
/* BCM_BUZZZ subsys header preamble snapshots logged at the start of a trace */
uintptr bcm_buzzz_hdr_bus(uintptr cur, uintptr end); // BCM_BUZZ_[KPI, LAT]
uintptr bcm_buzzz_hdr_mac(uintptr cur, uintptr end); // BCM_BUZZ_[KPI, LAT]
uintptr bcm_buzzz_hdr_scb(uintptr cur, uintptr end); // BCM_BUZZ_[KPI, LAT]
uintptr bcm_buzzz_hdr_spp(uintptr cur, uintptr end); // BCM_BUZZZ_SPP
#endif  /* BCM_BUZZZ_HDR_BUILD */

#if !defined(BCM_BUZZZ_STREAMING_BUILD)
/** BUZZZ Control Structure */
typedef struct bcm_buzzz
{
	uint32          log;
	uint32          cur;        /* pointer to next log entry */
	uint32          end;        /* pointer to end of log entry */

	uint32          count;      /* count of logs, wraps on 64K */
	uint8           status;     /* runtime status */
	uint8           wrap;       /* log buffer wrapped */
	uint8           mode;       /* the current logging mode */
	uint8           skip;       /* buzzz starts to skip */

	/* if !BCM_BUZZZ_FUNC */
	uint8           cpu_idcode; /* ARM idcode CM3=0x03, CR4=0x04,CR7=0x07 */
	uint8           counters;   /* number of counter logged */
	uint8           group;      /* event group */
	uint8           rsvd;

	uint32          eventid[BCM_BUZZZ_COUNTERS_MAX]; /* event selected */
	/* endif !BCM_BUZZZ_FUNC */

	uint32          buffer_sz;
	uint32          log_sz;
} bcm_buzzz_t;

/** BUZZZ Kernel Log */
typedef struct bcm_buzzz_klog
{
	uint8  cnt;
	uint8  args;          /* number of arguments logged */
	uint16 id;            /* index into registerd format strings */
	/* Dongle is a single-core CPU, else we need nbits for CPU core id */
} bcm_buzzz_klog_t;

typedef union bcm_buzzz_arg0
{
	uint32 u32;
	bcm_buzzz_klog_t klog;
} bcm_buzzz_arg0_t;

#else /* BCM_BUZZZ_STREAMING_BUILD */

/* Callback invoked by pcie bus layer on DMA complete */
void bcm_buzzz_d2h_done(uint32 copy_len, uint32 index, uint32 offset);

#endif /* BCM_BUZZZ_STREAMING_BUILD */

typedef enum bcm_buzzz_ctrl {
	BCM_BUZZZ_STOP_COMMAND = 0,
	BCM_BUZZZ_START_COMMAND,
	BCM_BUZZZ_SEG_FULL
} bcm_buzzz_ctrl_t;

/* Exported for dhd dump */
typedef union bcm_buzzz_cm3_cnts { /* Pack 8 bit CM3 counters */
	uint32 u32;
	uint8  u8[4];
	struct {
		uint8 cpicnt;
		uint8 exccnt;
		uint8 sleepcnt;
		uint8 lsucnt;
	};
} bcm_buzzz_cm3_cnts_t;

#endif /* BCM_BUZZZ */

#ifdef BCM_BUZZZ_STREAMING_BUILD
/*
 * Host and Dongle buzzz streaming mode uses this configuration.
 *
 * Host allocates a total of 32 MBytes, in 8 segments of 4 MBytes each.
 * Dongle uses a 16 Kbyte trace buffer (dual buffer mode).
 * 256 buzzz log buffers, each of 16 KBytes, fit into one 4 Mbyte host segment.
 * 1024 buzzz logs, each of 16 Bytes, fit into one 16 Kbyte buzzz log buffer.
 *
 * Layout shared between dongle firmware, dhd and desktop application.
 */

typedef union bcm_buzzz_log
{
#if defined(BCM_BUZZZ_FUNC)
	uint32          u32[2];
	struct {
		struct {
			uint8   id;     /* eventid : max 255, around 32 in use */
			uint8   args;   /* number of arguments logged */
			uint16  arg2;   /* last arg2 is ONLY 16 bits ! */
		};
		uint32      arg1;   /* caller or arg1 for user logging */
	};
#else  /* ! BCM_BUZZZ_FUNC */
	uint32          u32[4];
	struct {
		struct {
			uint8   id;     /* eventid : max 255, around 32 in use */
			uint8   args;   /* number of arguments logged */
			uint16  arg3;   /* BCM_BUZZZ_3ARGS: last arg3 is ONLY 16 bits ! */
		};
		uint32      cycctr; /* cycle counter */
		uint32      arg1;   /* 32 bit arg0 */
		uint32      arg2;   /* 32 bit arg1 */
	};
#endif /* ! BCM_BUZZZ_FUNC */
} bcm_buzzz_log_t;

#define BCM_BUZZZ_DNGLMEM_SIZE      (2 * 4 * 1024)     /* 8 KBytes buffer */

#define BCM_BUZZZ_HOSTMEM_TOTSZ     (32 * 1024 * 1024) /* 32 MBytes total */
#define BCM_BUZZZ_HOSTMEM_SEGSZ     (4  * 1024 * 1024) /*  4 MBytes segment */

#define BCM_BUZZZ_SEGMENTS  (BCM_BUZZZ_HOSTMEM_TOTSZ / BCM_BUZZZ_HOSTMEM_SEGSZ)

/** BUZZZ Streaming mode state for host buffer segments in use */
typedef struct bcm_buzzz_ctx
{
	uint32          log;        /* pointer to log buffer */
	uint32          cur;        /* pointer to next log entry */
	uint32          end;        /* pointer to end of log buffer */
} bcm_buzzz_ctx_t;

#ifndef BCM_BUZZZ_EXE

/** BUZZZ Streaming mode state */
typedef struct bcm_buzzz        /* pointers not permissible in this structure */
{
	uint8           status;     /* current logging status */
	uint8           wrap;       /* wrap requested */
	uint16          overflows;  /* number of overflows of dngl log buffers */
	int             in_dma;     /* num dma in progress */

	uint32          cur_ctx_ptr; /* pointer to current context */
	bcm_buzzz_ctx_t ctx[2];     /* dual logging context */

	uint32          fwid;

	uint32          seg_index;
	uint32          seg_offset;
	uint64		seg_haddr_u64[BCM_BUZZZ_SEGMENTS];

	bool            seg_avail[BCM_BUZZZ_SEGMENTS];
} bcm_buzzz_t;
#endif /* BCM_BUZZZ_EXE */

#endif /* BCM_BUZZZ_STREAMING_BUILD */

#define BUZZZ_KEVT_COUNTERS (2)

typedef /* Unified log structure for function and kevt logging */
struct buzzz_log_nic
{
	union {
		uint32     u32;
		struct {
			uint16 event; /* log entry event id or FUNC_ENTRY/EXIT */
			uint8  core;  /* CPU processor id */
			uint8  args;  /* number of valid args in this log entry */
		};
	};
	uint32 u1;
	union {
		struct {
			uint32 u2, u3;
		};
		uintptr uptr;
	};

	/* ARM PMU counters logged only in KEVT mode */
	uint32 ctr[BUZZZ_KEVT_COUNTERS];

} buzzz_log_nic_t;

/* bcm_buzzz dump color highlighting: */
#if defined(BCM_BUZZZ_EXE)
#define _CLR_(x) ""
#else
#define _CLR_(x) x
#endif

#undef  _R_
#undef  _B_
#undef  _C_
#undef  _G_
#undef  _M_
#undef  _H_
#undef  _N_
#undef  _FAIL_

/* _R_ Red, _B_ Blue, _C_ Cyan, _G_ Green, _M_ Magenta, _H_ Red_on_Black  */
#define _R_     _CLR_("\e[0;31m")
#define _B_     _CLR_("\e[0;34m")
#define _C_     _CLR_("\e[0;38m")
#define _G_     _CLR_("\e[0;32m")
#define _M_     _CLR_("\e[0;35m")
#define _H_     _CLR_("\e[0;31m;40m")
#define _N_     _CLR_("\e[0m")
#define _FAIL_  _H_ " === FAILURE ===" _N_

/*
 * +----------------------------------------------------------------------------
 * BUZZZ Subsystem Context logged in the first 8K buffer at start of trace.
 * Last 256 eventIds in a uint16 are reserved for startup subsystem snapshot.
 *
 * Following structures are shared by the WLAN driver and the offline parser.
 * +----------------------------------------------------------------------------
 */

#define BCM_BUZZZ_HDR_LOG_MAXSZ     (64)

typedef enum bcm_buzzz_hdr_id
{
	BCM_BUZZZ_HDR_CTX = 0xF0,
	BCM_BUZZZ_HDR_RTE,  //
	BCM_BUZZZ_HDR_BUS,  // Flowrings: ea, ring, rd, rd_pend, wr, flowid, max512
	BCM_BUZZZ_HDR_MAC,  // TxFIFOs: pkts, mpdus
	BCM_BUZZZ_HDR_SCB,  // STA Info: assoc id, tx ini and rx_resp bitmap, ea
	BCM_BUZZZ_HDR_SPP,  // Several SPP state
	BCM_BUZZZ_HDR_PADDING = 0xFF // HDR Section Padding(0xFF), parser interprets it as id field
} bcm_buzzz_hdr_id_t;

typedef struct bcm_buzzz_hdr_log
{
	uint8  id;          // bcm_buzzz_hdr_id_t
	uint8  u8;          // application specific, e.g. mac: txfifo, rxfifo
	uint16 u16;         // application specific, e.g. n items
} bcm_buzzz_hdr_log_t;

/* BCM_BUZZZ_HDR_BUS: PCIe BUS facing preamble: TxPost flowring contexts */
typedef struct bcm_buzzz_hdr_bus_ring
{
	uint16 idx;         // flowring id
	uint16 rd;          // Read index
	uint16 rd_pend;     // Read pending index - deprecated with PQP
	uint16 wr;          // Write index
	uint16 scb_flowid;  // SCB's Id bound to the flowring
	uint8  max512;      // TxPost ring depth in units of 512 elements
	uint8  tid;         // flowring priority aka tid
	uint8  pad[2];      // padding, to align log to 20 Bytes
	char   ea[6];       // SCB ethernet address, ETHER_ADDR_LEN = 6
} bcm_buzzz_hdr_bus_ring_t; // 20B size

/* BCM_BUZZZ_HDR_MAC: D11 LowerMAC facing preamble: TxFIFO(s) context */
typedef struct bcm_buzzz_hdr_mac_fifo
{
	uint16 pkt_count;   // 802.3 SDUs
	uint16 mpdu_count;  // 802.11 PDUs
} bcm_buzzz_hdr_mac_fifo_t;

/* BCM_BUZZZ_HDR_SCB: WLAN UpperMAC STA Info preamble */
typedef struct bcm_buzzz_hdr_scb
{
	uint16 aid;         // Assoc Id
	uint16 tx_ini_bmp;  // Initiator bitmap
	uint16 rx_resp_bmp; // Responder bitmap
	uint8 ea[6];        // SCB ethernet address, ETHER_ADDR_LEN = 6
} bcm_buzzz_hdr_scb_t;

typedef enum bcm_buzzz_evtype {
	BCM_BUZZZ_EVNONE = 0,
	BCM_BUZZZ_EVENT
} bcm_buzzz_evtype_t;

/*
 * +--------------------------------------------------------------------------+
 *  SPP Instrumentation
 * +--------------------------------------------------------------------------+
 *  Packet Storage:
 *    + PKTSIZE: Size of a Lfrag
 *    + LCL = Data buffer external to a Packet (eg. Dongle originating MGMT pkt)
 *  SPP Scheduling Sources:
 *    + Downstream (TxProcess TXP + TxStatus TXS)
 *    + Upstream (RxProcess RXP),
 *  LFrag Packet Storage (Buffer Manager) in HWA Packet Pager (LBM in PQP):
 *    + Packets Dongle Buffer Manager (DBM)
 *    + Packets in Host Buffer Manager (HBM)
 *    + LCL buffers in LCL Buffer Manager (LBM)
 *    + TOT = Packets occupying BM resources, i.e SPP Inflight(PAGEIN) + Active
 *      On a PAGEOUT or Packet Free, TOT is decremented, i.e. BM resource freed.
 *    + HWM = High Watermark, clear on read using a periodic timer
 *    + MAX = Maximum number of Packets (i.e buffers) in a BM
 *  SW managed pre-allocated pool or reservation count for emergency allocation
 *    + SW_POOL or Reserved: e.g. 16 RXP pool + 8 TXP pool + 64 PQP pool + 256
 * +--------------------------------------------------------------------------+
 */

/* SPP preamble state: Used for BCM_BUZZZ_HDR_SPP */
#define BCM_BUZZZ_SPP_VERSION  (1)

typedef enum bcm_buzzz_spp_var
{
	BCM_BUZZZ_SPP_TXP_TOT   =  0,   // TXP Source: Inflight + Active
	BCM_BUZZZ_SPP_STS_TOT   =  1,   // STS Source: Inflight + Active
	BCM_BUZZZ_SPP_RXP_TOT   =  2,   // RXP Source: Inflight + Active

	BCM_BUZZZ_SPP_TXP_PND   =  3,   // TXP Source: PAGEIN Pending in Request(s)
	BCM_BUZZZ_SPP_STS_PND   =  4,   // STS Source: PAGEIN Pending in Request(s)
	BCM_BUZZZ_SPP_RXP_PND   =  5,   // RXP Source: PAGEIN Pending in Request(s)

	BCM_BUZZZ_SPP_DBM_TOT   =  6,   // Dongle BM : Inflight + Active + Reserved
	BCM_BUZZZ_SPP_HBM_TOT   =  7,   // Host   BM : Active
	BCM_BUZZZ_SPP_DBM_MAX   =  8,   // Dongle BM : Packet storage size
	BCM_BUZZZ_SPP_HBM_MAX   =  9,   // HOST   BM : Packet storage size

	BCM_BUZZZ_SPP_QUE_LEN   = 10,   // Req Queue : Length
	BCM_BUZZZ_SPP_QUE_CNT   = 11,   // Local  BM : Pending Count

	BCM_BUZZZ_SPP_SW_POOL   = 12,   // Packets in SW managed pools or reserved
	BCM_BUZZZ_SPP_PKTSIZE   = 13,   // Size of a Lfrag

	BCM_BUZZZ_SPP_VAR_MAX   = 14
} bcm_buzzz_spp_var_t;

#define BCM_BUZZZ_SPP_VAR_NAMES {               \
	"TXP_TOT", "STS_TOT", "RXP_TOT",            \
	"TXP_PND", "STS_PND", "RXP_PND",            \
	"DBM_TOT", "HBM_TOT", "DBM_MAX", "HBM_MAX", \
	"QUE_LEN", "QUE_CNT",                       \
	"SW_POOL", "PKTSIZE"                        \
}

typedef struct bcm_buzzz_hdr_spp
{
	uint16 var;                     // SPP Variable type: bcm_buzzz_spp_var
	uint16 val;                     // Value of the SPP variable
} bcm_buzzz_hdr_spp_t;

/*
 * +----------------------------------------------------------------------------
 *
 * CAUTION: impact on ROM invalidations.
 *
 * Three steps to insert an instrumentation point.
 *
 * Step #1. List event in enum bcm_buzzz_KLOG_dpid
 *          E.g. BUZZZ_KLOG(SAMPLE_EVENT_NAME)
 *
 * Step #2. Register the event string to be used in bcm_buzzz_dump()
 *          Add an entry in bcm_buzzz.h: BCM_BUZZZ_FMT_STRINGS
 *
 * Step #3. Insert instrumentationi, at a desired compile time tracing level
 *          E.g. BUZZZ_LVL#(SAMPLE_EVENT_NAME, 1, (uint32)pkt);
 *          See note below on BUZZZ_LVL#()
 *
 * +----------------------------------------------------------------------------
 */

#ifdef BCM_BUZZZ_LAT_NIC
/* reserving 256 ids for BUZZZ_KNL events (in include/uapi/linux/buzzz.h) */
#define BUZZZ_KLOG_START    256
#else
#define BUZZZ_KLOG_START   0
#endif /* BCM_BUZZZ_LAT_NIC */

/* buzzz(_bupc) application uses this offset generate BUZZZ_KLOG id */
#define BUZZZ_KLOG_OFFSET   256

#undef BUZZZ_KLOG
#define BUZZZ_KLOG(event)       BUZZZ_KLOG__ ## event,
typedef enum bcm_buzzz_KLOG_dpid    /* List of datapath event point ids */
{
	BUZZZ_KLOG__START_EVT = BUZZZ_KLOG_START, // level
	BUZZZ_KLOG(SWAP_EVT)                    // 1

	BUZZZ_KLOG(FUNC_ENT)                    // 2
	BUZZZ_KLOG(FUNC_EXT)                    // 3
	BUZZZ_KLOG(FUNC_LINE)

	BUZZZ_KLOG(CPU_COUNTER_0)               // 3
	BUZZZ_KLOG(CPU_COUNTER_1)               // 3
	BUZZZ_KLOG(CPU_COUNTER_2)               // 3
	BUZZZ_KLOG(CPU_COUNTER_3)               // 3

	BUZZZ_KLOG(MODE)                        // 1

	BUZZZ_KLOG(BUZZZ_0)                     // user defined
	BUZZZ_KLOG(BUZZZ_1)                     // user defined
	BUZZZ_KLOG(BUZZZ_2)                     // user defined
	BUZZZ_KLOG(BUZZZ_3)                     // user defined
	BUZZZ_KLOG(BUZZZ_4)                     // user defined

	BUZZZ_KLOG(HND_CPUUTIL_EPOCH)           // 1
	BUZZZ_KLOG(HND_CPUUTIL_TRANS)           // 5

	BUZZZ_KLOG(HND_DIE)                     // 1
	BUZZZ_KLOG(HND_TRAP)                    // 1
	BUZZZ_KLOG(HND_ASSERT)                  // 1
	BUZZZ_KLOG(HND_DELAY)                   // 4
	BUZZZ_KLOG(HND_MALLOC)                  // 4
	BUZZZ_KLOG(HND_FREE)                    // 4
	BUZZZ_KLOG(HND_SO_MALLOC)               // 4
	BUZZZ_KLOG(HND_SO_FREE)                 // 4
	BUZZZ_KLOG(HND_SCHED_WORK)              // 3
	BUZZZ_KLOG(HND_WORKTMR_ENT)             // 3
	BUZZZ_KLOG(HND_WORKTMR_RTN)             // 3
	BUZZZ_KLOG(HND_WORKLET_ENT)             // 3 retain "DPC" in format: kw.pl
	BUZZZ_KLOG(HND_WORKLET_RTN)             // 3 retain "DPC" in format: kw.pl
	BUZZZ_KLOG(HND_WORKLET_REASON)          // 5
	BUZZZ_KLOG(HND_TMR_ENT)                 // 3
	BUZZZ_KLOG(HND_TMR_RTN)                 // 3
	BUZZZ_KLOG(HND_TMR_CRT)                 // 4
	BUZZZ_KLOG(HND_TMR_DEL)                 // 4
	BUZZZ_KLOG(HND_TMR_BGN)                 // 5
	BUZZZ_KLOG(HND_TMR_END)                 // 5

	BUZZZ_KLOG(THREADX_IDLE)                // 3
	BUZZZ_KLOG(THREADX_CPU_ISR_ENT)         // 4
	BUZZZ_KLOG(THREADX_CPU_ISR_RTN)         // 4
	BUZZZ_KLOG(THREADX_PRI_ISR_ENT)         // 3
	BUZZZ_KLOG(THREADX_PRI_ISR_RTN)         // 3
	BUZZZ_KLOG(THREADX_SEC_ISR_ENT)         // 5
	BUZZZ_KLOG(THREADX_SEC_ISR_RTN)         // 5
	BUZZZ_KLOG(THREADX_SCHED_THREAD)        // 3

	BUZZZ_KLOG(KPI_PKT_MAC_RXFIFO)          // 1
	BUZZZ_KLOG(KPI_PKT_BUS_RXCMPL)          // 1
	BUZZZ_KLOG(KPI_PKT_BUS_RXBMRC)          // 3
	BUZZZ_KLOG(KPI_PKT_BUS_RXDROP)          // 3
	BUZZZ_KLOG(KPI_PKT_BUS_TXPOST)          // 1
	BUZZZ_KLOG(KPI_PKT_MAC_TXMPDU)          // 1
	BUZZZ_KLOG(KPI_PKT_MAC_TXMSDU)          // 1
	BUZZZ_KLOG(KPI_PKT_MAC_TXSTAT)          // 1
	BUZZZ_KLOG(KPI_PKT_BUS_TXCMPL)          // 1
	BUZZZ_KLOG(KPI_PKT_BUS_TXSUPP)          // 3

	BUZZZ_KLOG(KPI_QUE_BUS_WR_UPD)          // 1
	BUZZZ_KLOG(KPI_QUE_BUS_RP_UPD)          // 1
	BUZZZ_KLOG(KPI_QUE_BUS_RP_REW)          // 1
	BUZZZ_KLOG(KPI_QUE_BUS_RD_UPD)          // 1
	BUZZZ_KLOG(KPI_QUE_MAC_WR_UPD)          // 1
	BUZZZ_KLOG(KPI_QUE_MAC_RD_UPD)          // 1

	BUZZZ_KLOG(KPI_PKTC_FWT1)               // 0
	BUZZZ_KLOG(KPI_PKTC_FWT2)               // 0
	BUZZZ_KLOG(KPI_PKTC_FWT3)               // 0
	BUZZZ_KLOG(KPI_PKTC_FWT4)               // 0

	BUZZZ_KLOG(KPI_PKT_FWT1)                // 0
	BUZZZ_KLOG(KPI_PKT_FWT2)                // 0
	BUZZZ_KLOG(KPI_PKT_FWT3)                // 0
	BUZZZ_KLOG(KPI_PKT_FWT4)                // 0

	BUZZZ_KLOG(KPI_PKTC_FWR1)               // 0 TBD
	BUZZZ_KLOG(KPI_PKTC_FWR1_1)             // 0 TBD
	BUZZZ_KLOG(KPI_PKTC_FWR2)               // 0

	BUZZZ_KLOG(KPI_PKT_FWR0)                // 0
	BUZZZ_KLOG(KPI_PKT_FWR1_MPDU)           // 0
	BUZZZ_KLOG(KPI_PKT_FWR1_MPDU2)          // 0
	BUZZZ_KLOG(KPI_PKT_FWR1_AMPDU)          // 0
	BUZZZ_KLOG(KPI_PKT_FWR1_AMPDU2)         // 0
	BUZZZ_KLOG(KPI_PKT_FWR2)               // 0
	BUZZZ_KLOG(KPI_TSF_SYNC)                // 0

	// SPP Per Source Events: GET = PageIN, PUT = PageOut or Free
	BUZZZ_KLOG(SPP_TXP_GET)                 // 1
	BUZZZ_KLOG(SPP_TXP_PUT)                 // 1
	BUZZZ_KLOG(SPP_STS_GET)                 // 1
	BUZZZ_KLOG(SPP_STS_PUT)                 // 1
	BUZZZ_KLOG(SPP_RXP_GET)                 // 1
	BUZZZ_KLOG(SPP_RXP_PUT)                 // 1
	// SPP Requests: BGN CNT END = Enqueue, Service and Dequeue from HoldQueue
	BUZZZ_KLOG(SPP_TXP_BGN)                 // 3
	BUZZZ_KLOG(SPP_TXP_CNT)                 // 3
	BUZZZ_KLOG(SPP_TXP_END)                 // 3
	BUZZZ_KLOG(SPP_STS_BGN)                 // 3
	BUZZZ_KLOG(SPP_STS_CNT)                 // 3
	BUZZZ_KLOG(SPP_STS_END)                 // 3
	BUZZZ_KLOG(SPP_RXP_BGN)                 // 3
	BUZZZ_KLOG(SPP_RXP_CNT)                 // 3
	BUZZZ_KLOG(SPP_RXP_END)                 // 3
	// SPP Periodic: TOT and HWM for DBM HBM and SPP Pending count
	BUZZZ_KLOG(SPP_TMR_TOT)                 // 1
	BUZZZ_KLOG(SPP_TMR_HWM)                 // 1

	// KPI T3UCODE
	BUZZZ_KLOG(KPI_T3UCODETXS_FPKT)         // 0
	BUZZZ_KLOG(KPI_T3UCODETXS_LPKT)         // 0
	BUZZZ_KLOG(KPI_T3UCODETXS)              // 0
	BUZZZ_KLOG(KPI_T3UCODETXS_FLPKT)        // 0
	// KPI ULMU
	BUZZZ_KLOG(KPI_ULMU_TRIGTXS)            // 0
	BUZZZ_KLOG(KPI_ULMU_TRIGTXS1)           // 0
	BUZZZ_KLOG(KPI_ULMU_TRIGTXS2)           // 0
	BUZZZ_KLOG(KPI_ULMU_DRVUTXD)            // 0
	BUZZZ_KLOG(KPI_SCB_PS)                  // 0
	BUZZZ_KLOG(KPI_QUE_BUS_SPP_ENQ)         // 1
	BUZZZ_KLOG(KPI_ULMU_OMI)                // 1

	BUZZZ_KLOG__LAST_EVT

} bcm_buzzz_KLOG_dpid_t;

/*
 * All format strings must be listed in the same sequence (comma seperated) as
 * their corresponding eventids in enum list bcm_buzzz_KLOG_dpid_t.
 *
 * NOTE: While constructing the format string, please use "ra[@%08x]" when you
 * log the return address using (uint32)__builtin_return_address(0). A script
 * such as buzzz_sym.pl may then be used to convert all instruction addresses
 * to their symbol-names using the corresponding rtecdc.map.
 *     buzzz_sym.pl -m <rtecdc.map> -i <logfile> -o <outfile>
 *
 * Likewise, any function pointer may be logged with a corresponding "fn[@%08x]"
 */
#define BCM_BUZZZ_FMT_STRINGS                                                  \
{                                                                              \
	"START_EVT",                                      /* START_EVT */          \
	"SWAP_EVT seg index<%u> offset<%u>",              /* SWAP_EVT */           \
	                                                                           \
	"=> [@%08x]",                                     /* FUNC_ENT */           \
	"<= [@%08x]",                                     /* FUNC_EXT */           \
	"line #%u",                                       /* FUNC_LINE */          \
	                                                                           \
	/* Used only as a fallback, buzzz has better custom rendering */           \
	"CPU counter 0 id<%u> value<0x%08x>",             /* CPU_COUNTER_0 */      \
	"CPU counter 1 id<%u> value<0x%08x>",             /* CPU_COUNTER_1 */      \
	"CPU counter 2 id<%u> value<0x%08x>",             /* CPU_COUNTER_2 */      \
	"CPU counter 3 id<%u> value<0x%08x>",             /* CPU_COUNTER_3 */      \
	                                                                           \
	"MHz<%u> FWID<0x%x>",                             /* MODE */               \
	"buzzz_log0",                                     /* BUZZZ_0 */            \
	"buzzz_log1 arg<%u>",                             /* BUZZZ_1 */            \
	"buzzz_log2 arg<%u:%u>",                          /* BUZZZ_2 */            \
	"buzzz_log3 arg<%u:%u:%u>",                       /* BUZZZ_3 */            \
	"buzzz_log4 arg<%u:%u:%u:%u>",                    /* BUZZZ_4 */            \
	                                                                           \
	/* HND THREADX: with color formatting */                                   \
	_G_ "CPUUTIL epoch %u" _N_,                       /* HND_CPUUTIL_EPOCH */  \
	_G_ "CPUUTIL trans fn[@%08x] %u" _N_,             /* HND_CPUUTIL_TRANS */  \
	                                                                           \
	_H_ ":::die ra[@%08x]" _N_,                       /* HND_DIE */            \
	_H_ ":::trap pc[@%08x] type<%d>" _N_,             /* HND_TRAP */           \
	_H_ ":::assert ra[@%08x] line<%d>" _N_,           /* HND_ASSERT */         \
	_B_ ":::delay ra[@%08x] usec<%d>" _N_,            /* HND_DELAY */          \
	_B_ ":::malloc ra[@%08x] size<%d>" _N_,           /* HND_MALLOC */         \
	_B_ ":::free ra[@%08x]" _N_,                      /* HND_FREE */           \
	_B_ ":::so_malloc ra[@%08x] size<%d>" _N_,        /* HND_SO_MALLOC */      \
	_B_ ":::so_free ra[@%08x]" _N_,                   /* HND_SO_FREE */        \
	_B_ ":::sched fn[@%08x] delay<%u>" _N_,           /* HND_SCHED_WORK */     \
	_M_ "   >>> WORKTMR fn[@%08x]" _N_,               /* HND_WORKTMR_ENT */    \
	_M_ "   <<< WORKTMR" _N_,                         /* HND_WORKTMR_RTN */    \
	_M_ "    >> DPC fn[@%08x]" _N_,                   /* HND_WORKLET_ENT */    \
	_M_ "    << DPC" _N_,                             /* HND_WORKLET_RTN */    \
	_M_ "  <<<< WORKLET SCHED %u" _N_,                /* HND_WORKLET_REASON */ \
	_M_ "   >>> TMR fn[@%08x]" _N_,                   /* HND_TMR_ENT */        \
	_M_ "   <<< TMR" _N_,                             /* HND_TMR_RTN */        \
	_B_ ":::tmr_create ra[@%08x]" _N_,                /* HND_TMR_CRT */        \
	_B_ ":::tmr_delete ra[@%08x]" _N_,                /* HND_TMR_DEL */        \
	_B_ ":::tmr_start ra[@%08x]" _N_,                 /* HND_TMR_BGN */        \
	_B_ ":::tmr_stop ra[@%08x]" _N_,                  /* HND_TMR_END */        \
	                                                                           \
	_G_ ":::idle loop" _N_,                         /* THREADX_IDLE */         \
	_R_ "  >>>> CPU ISR int<0x%08x>" _N_,           /* THREADX_CPU_ISR_ENT */  \
	_R_ "  <<<< CPU ISR" _N_,                       /* THREADX_CPU_ISR_RTN */  \
	_R_ "   >>> ISR fn[@%08x]" _N_,                 /* THREADX_PRI_ISR_ENT */  \
	_R_ "   <<< ISR" _N_,                           /* THREADX_PRI_ISR_RTN */  \
	_R_ "    >> SISR fn[@%08x] grp<%u>" _N_,        /* THREADX_SEC_ISR_ENT */  \
	_R_ "    << SISR" _N_,                          /* THREADX_SEC_ISR_RTN */  \
	_M_ "     ! SCHED thread-%c::%u" _N_,           /* THREADX_SCHED_THREAD */ \
	                                                                           \
	                                                                           \
	_C_ "       MAC_RXFIFO pkt<0x%08x>" _N_,          /* KPI_PKT_MAC_RXFIFO */ \
	_C_ "       BUS_RXCMPL pkt<0x%08x>" _N_,          /* KPI_PKT_BUS_RXCMPL */ \
	_C_ "       BUS_RXBMRC pkt<0x%08x>" _N_,          /* KPI_PKT_BUS_RXBMRC */ \
	_C_ "       BUS_RXDROP pkt<0x%08x>" _N_,          /* KPI_PKT_BUS_RXDROP */ \
	_C_ "       BUS_TXPOST pkt<0x%08x> ring<%u>" _N_, /* KPI_PKT_BUS_TXPOST */ \
	_C_ "       MAC_TXMPDU pkt<0x%08x> fifo<%u>" _N_, /* KPI_PKT_MAC_TXMPDU */ \
	_C_ "       MAC_TXMSDU pkt<0x%08x> fifo<%u>" _N_, /* KPI_PKT_MAC_TXMSDU */ \
	_C_ "       MAC_TXSTAT ncons<%u>   fifo<%u>" _N_, /* KPI_PKT_MAC_TXSTAT */ \
	_C_ "       BUS_TXCMPL pkt<0x%08x> ring<%u>" _N_, /* KPI_PKT_BUS_TXCMPL */ \
	_C_ "       BUS_TXSUPP pkt<0x%08x> ring<%u>" _N_, /* KPI_PKT_BUS_TXSUPP */ \
	                                                                           \
	_C_ "       BUS_WR WR<%u> ring<%u>" _N_,          /* KPI_QUE_BUS_WR_UPD */ \
	_C_ "       BUS_RP RP<%u> ring<%u>" _N_,          /* KPI_QUE_BUS_RP_UPD */ \
	_C_ "       BUS_RP RB<%u> ring<%u>" _N_,          /* KPI_QUE_BUS_RP_REW */ \
	_C_ "       BUS_RD RD<%u> ring<%u>" _N_,          /* KPI_QUE_BUS_RD_UPD */ \
	_C_ "       MAC +MPDU<%u> fifo<%u>" _N_,          /* KPI_QUE_MAC_WR_UPD */ \
	_C_ "       MAC -MPDU<%u> fifo<%u>" _N_,          /* KPI_QUE_MAC_RD_UPD */ \
	                                                                           \
	_C_ "       C_FWT1 pktid<0x%08x> ring<%u> count<%u>" _N_, /* KPI_PKTC_FWT1 */ \
	_C_ "       C_FWT2 pktid<0x%08x> ring<%u> count<%u>" _N_, /* KPI_PKTC_FWT2 */ \
	_C_ "       C_FWT3 pktid<0x%08x> ring<%u> count<%u>" _N_, /* KPI_PKTC_FWT3 */ \
	_C_ "       C_FWT4 pktid<0x%08x> ring<%u> count<%u>" _N_, /* KPI_PKTC_FWT4 */ \
	                                                                           \
	_C_ "       FWT1 pktid<0x%08x> ring<%u> index<%u>" _N_, /* KPI_PKT_FWT1 */ \
	_C_ "       FWT2 pktid<0x%08x> ring<%u>" _N_,           /* KPI_PKT_FWT2 */ \
	_C_ "       FWT3 pktid<0x%08x> ring<%u>" _N_,           /* KPI_PKT_FWT3 */ \
	_C_ "       FWT4 pktid<0x%08x> ring<%u>" _N_,           /* KPI_PKT_FWT4 */ \
	                                                                           \
	_C_ "       C_FWR1 pktid<0x%08x> count<%u>" _N_,   /* KPI_PKTC_FWR1 TBD */ \
	_C_ "       C_FWR1_1 pktid<0x%08x> count<%u>" _N_, /* KPI_PKTC_FWR1 TBD */ \
	_C_ "       C_FWR2 pktid<0x%08x> count<%u>" _N_,   /* KPI_PKTC_FWR2 */     \
	                                                                           \
	_C_ "       FWR0 pktid<0x%08x> mac_tsf<%u> AID<%d> prio<%d>" _N_, /* KPI_PKT_FWR0 */ \
	_C_ "       FWR1_MPDU first_pktid<0x%08x> last_pktid<%08x>" _N_, /* KPI_PKT_FWR1_MPDU */ \
					/* KPI_PKT_FWR1_MPDU2 */ \
	_C_ "       FWR1_MPDU first<0x%08x> last<0x%08x> count<%d> AID<%d> prio<%d>" _N_, \
	_C_ "       FWR1_AMPDU first<0x%08x> last<0x%08x>" _N_, /* KPI_PKT_FWR1_AMPDU */ \
					/* KPI_PKT_FWR1_AMPDU2 */ \
	_C_ "       FWR1_AMPDU first<0x%08x> last<0x%08x> count<%d> AID<%d> prio<%d>" _N_, \
	_C_ "       FWR2 pktid<0x%08x>" _N_,                    /* KPI_PKT_FWR2 */ \
	_C_ "       TSFSYNC TSFL<%u> TSFH<%u> TSF64<%llu>" _N_, /* KPI_TSF_SYNC */ \
	                                                                           \
	                                                                           \
	_C_ "       SPP SRC TXP GET | %5u | %5u | %5u |" _N_, /* SPP_TXP_GET */    \
	_C_ "       SPP SRC TXP PUT | %5u | %5u | %5u |" _N_, /* SPP_TXP_PUT */    \
	_C_ "       SPP SRC STS GET | %5u | %5u | %5u |" _N_, /* SPP_STS_GET */    \
	_C_ "       SPP SRC STS PUT | %5u | %5u | %5u |" _N_, /* SPP_STS_PUT */    \
	_C_ "       SPP SRC RXP GET | %5u | %5u | %5u |" _N_, /* SPP_RXP_GET */    \
	_C_ "       SPP SRC RXP PUT | %5u | %5u | %5u |" _N_, /* SPP_RXP_PUT */    \
	_C_ "       SPP REQ TXP BGN | %5u | %5u | %5u |" _N_, /* SPP_TXP_BGN */    \
	_C_ "       SPP REQ TXP CNT | %5u | %5u | %5u |" _N_, /* SPP_TXP_CNT */    \
	_C_ "       SPP REQ TXP END | %5u | %5u | %5u |" _N_, /* SPP_TXP_END */    \
	_C_ "       SPP REQ STS BGN | %5u | %5u | %5u |" _N_, /* SPP_STS_BGN */    \
	_C_ "       SPP REQ STS CNT | %5u | %5u | %5u |" _N_, /* SPP_STS_CNT */    \
	_C_ "       SPP REQ STS END | %5u | %5u | %5u |" _N_, /* SPP_STS_END */    \
	_C_ "       SPP REQ RXP BGN | %5u | %5u | %5u |" _N_, /* SPP_RXP_BGN */    \
	_C_ "       SPP REQ RXP CNT | %5u | %5u | %5u |" _N_, /* SPP_RXP_CNT */    \
	_C_ "       SPP REQ RXP END | %5u | %5u | %5u |" _N_, /* SPP_RXP_END */    \
	_C_ "       SPP TMR DHQ TOT | %5u | %5u | %5u |" _N_, /* SPP_TMR_TOT */    \
	_C_ "       SPP TMR DHQ HWM | %5u | %5u | %5u |" _N_, /* SPP_TMR_HWM */    \
	                                                                           \
	                                                                           \
	_C_ "       T3UCODETXS_FPKT pktid<0x%08x> ENQ_TSFL<%u>" _N_, /* KPI_T3UCODETXS_FPKT */ \
	_C_ "       T3UCODETXS_LPKT pktid<0x%08x> ENQ_TSFL<%u>" _N_, /* KPI_T3UCODETXS_LPKT */ \
	_C_ "       T3UCODETXS TXSTART_TSFL<%u> TXS_TSFL<%u> ring<%u>" _N_, /* KPI_T3UCODETXS */ \
	                                                /* KPI_T3UCODETXS_FLPKT */ \
	_C_ "       T3UCODETXS FLPKT first_pktid<%u> last_pktid<%u>" _N_,          \
	                                                                           \
	                                                                           \
	_C_ "       ULMU_TRIGTXS" _N_,                  /* KPI_ULMU_TRIGTXS */     \
	_C_ "       ULMU_TRIGTXS1" _N_,                 /* KPI_ULMU_TRIGTXS1 */    \
	_C_ "       ULMU_TRIGTXS2" _N_,                 /* KPI_ULMU_TRIGTXS2 */    \
	_C_ "       ULMU_TRIGDRVUTXD aid=%d relcnt=%d cmd=%04x" _N_, /* KPI_ULMU_DRVUTXD */ \
	                                                                           \
	_C_ "       SCB_PS_STATE aid=%u PS=%u" _N_,           /* KPI_SCB_PS */     \
	_C_ "       BUS_SPP_ENQ SPP_ENQIDX<%u> ring<%u>" _N_, /* KPI_QUE_BUS_SPP_ENQ */ \
	_C_ "       KPI_ULMU_OMI aid<%u> eligible<%u>" _N_,   /* KPI_ULMU_OMI */   \
	                                                                           \
	"LAST_EVENT"                                                               \
}

/*
 * +----------------------------------------------------------------------------
 *  Insert instrumentation in code at various tracing levels using
 *
 *   BUZZZ_LVL#(EVENT_ENUM, NUM_ARGS, ARGS_LIST)
 *
 *     #         : Compile time tracing level, BCM_BUZZZ_LOG_LEVEL >= #
 *     EVENT_ENUM: Enum added to buzzz_KLOG_dpid_t using BUZZZ_KLOG()
 *     NUM_ARGS  : Number of arguments to log, max 4 arguments
 *     ARGS_LIST : List of arguments, comma seperated
 *
 * +----------------------------------------------------------------------------
 */

/* level independent generic log */
#if defined(BCM_BUZZZ_LAT_NIC)
#define BUZZZ_LVL(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#else
#define BUZZZ_LVL(ID, N, ARG...)    bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#endif  /* ! BCM_BUZZZ_LAT_NIC */

#undef BUZZZ_LVL1
#if defined(BCM_BUZZZ_LOG_LEVEL) && (BCM_BUZZZ_LOG_LEVEL >= 1)
#define BUZZZ_LVL1(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_LOG_LEVEL >= 1 */
#define BUZZZ_LVL1(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_LOG_LEVEL >= 1 */

#undef BUZZZ_LVL2
#if defined(BCM_BUZZZ_LOG_LEVEL) && (BCM_BUZZZ_LOG_LEVEL >= 2)
#define BUZZZ_LVL2(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_LOG_LEVEL >= 2 */
#define BUZZZ_LVL2(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_LOG_LEVEL >= 2 */

#undef BUZZZ_LVL3
#if defined(BCM_BUZZZ_LOG_LEVEL) && (BCM_BUZZZ_LOG_LEVEL >= 3)
#define BUZZZ_LVL3(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_LOG_LEVEL >= 3 */
#define BUZZZ_LVL3(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_LOG_LEVEL >= 3 */

#undef BUZZZ_LVL4
#if defined(BCM_BUZZZ_LOG_LEVEL) && (BCM_BUZZZ_LOG_LEVEL >= 4)
#define BUZZZ_LVL4(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_LOG_LEVEL >= 4 */
#define BUZZZ_LVL4(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_LOG_LEVEL >= 4 */

#undef BUZZZ_LVL5
#if defined(BCM_BUZZZ_LOG_LEVEL) && (BCM_BUZZZ_LOG_LEVEL >= 5)
#define BUZZZ_LVL5(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_LOG_LEVEL >= 5 */
#define BUZZZ_LVL5(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_LOG_LEVEL >= 5 */

// + BCM_BUZZZ_LAT +-----------------------------------------------------------
#if defined(BCM_BUZZZ_LAT_NIC)
#include <linux/buzzz_kevt.h>
#define BUZZZ_KPI_LAT(ID, N, ARG...) buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#define BUZZZ_KPI_LAT_BYTES(ptr, len) buzzz_log_bytes(ptr, len)
#define BUZZZ_QUE_ID(scb, p) (SCB_AID(scb) | PKTPRIO(p) << 12)
#elif defined(BCM_BUZZZ_LAT)
#define BUZZZ_KPI_LAT(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else
#define BUZZZ_KPI_LAT(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif /* BCM_BUZZZ_LAT */

// + BCM_BUZZZ_KPI_PKT_LEVEL +-------------------------------------------------
#if (BCM_BUZZZ_KPI_PKT_LEVEL >= 1)
#define BUZZZ_KPI_PKT1(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_KPI_PKT_LEVEL >= 1 */
#define BUZZZ_KPI_PKT1(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_KPI_PKT_LEVEL >= 1 */

#if (BCM_BUZZZ_KPI_PKT_LEVEL >= 2)
#define BUZZZ_KPI_PKT2(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_KPI_PKT_LEVEL >= 2 */
#define BUZZZ_KPI_PKT2(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_KPI_PKT_LEVEL >= 2 */

#if (BCM_BUZZZ_KPI_PKT_LEVEL >= 3)
#define BUZZZ_KPI_PKT3(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_KPI_PKT_LEVEL >= 3 */
#define BUZZZ_KPI_PKT3(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_KPI_PKT_LEVEL >= 3 */

// + BCM_BUZZZ_KPI_QUE_LEVEL +-------------------------------------------------
#if (BCM_BUZZZ_KPI_QUE_LEVEL >= 1)
#define BUZZZ_KPI_QUE1(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_KPI_QUE_LEVEL >= 1 */
#define BUZZZ_KPI_QUE1(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_KPI_QUE_LEVEL >= 1 */

#if (BCM_BUZZZ_KPI_QUE_LEVEL >= 2)
#define BUZZZ_KPI_QUE2(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_KPI_QUE_LEVEL >= 2 */
#define BUZZZ_KPI_QUE2(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_KPI_QUE_LEVEL >= 2 */

#if (BCM_BUZZZ_KPI_QUE_LEVEL >= 3)
#define BUZZZ_KPI_QUE3(ID, N, ARG...) bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_KPI_QUE_LEVEL >= 3 */
#define BUZZZ_KPI_QUE3(ID, N, ARG...) BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_KPI_QUE_LEVEL >= 3 */

#ifdef BCM_BUZZZ_STREAMING_EXT
void bcm_buzzz_wlc_event(uint32 evtype, uint32 status, uint32 reason);
#endif /* BCM_BUZZZ_STREAMING_EXT */

// + BCM_BUZZZ_SPP_LEVEL +-------------------------------------------------
#if (BCM_BUZZZ_SPP_LEVEL >= 1)
#define BUZZZ_SPP1(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_SPP_LEVEL >= 1 */
#define BUZZZ_SPP1(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_SPP_LEVEL >= 1 */

#if (BCM_BUZZZ_SPP_LEVEL >= 2)
#define BUZZZ_SPP2(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_SPP_LEVEL >= 2 */
#define BUZZZ_SPP2(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_SPP_LEVEL >= 2 */

#if (BCM_BUZZZ_SPP_LEVEL >= 3)
#define BUZZZ_SPP3(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_SPP_LEVEL >= 3 */
#define BUZZZ_SPP3(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_SPP_LEVEL >= 3 */

/*
 * BUZZZ exported APIs that default to noop when BCM_BUZZZ is not defined
 */
#if defined(BCM_BUZZZ)

#define BCM_BUZZZ_INIT(shared)          bcm_buzzz_init(shared)
#define BCM_BUZZZ_CONFIG(ctr_sel)       bcm_buzzz_config(ctr_sel)
#define BCM_BUZZZ_SHOW()                bcm_buzzz_show()
#define BCM_BUZZZ_DUMP()                bcm_buzzz_dump()
#define BCM_BUZZZ_START()               bcm_buzzz_start()
#define BCM_BUZZZ_STOP()                bcm_buzzz_stop()
#define BCM_BUZZZ_WRAP(val)             bcm_buzzz_wrap(val)
#define BCM_BUZZZ_STATUS()              bcm_buzzz_status()
#define BCM_BUZZZ_MODE()                bcm_buzzz_mode()
#define BCM_BUZZZ_SKIP(num)             bcm_buzzz_skip(num)
#define BCM_BUZZZ_EMIT_COUNTERS()       bcm_buzzz_emit_counters()

#if defined(BCM_BUZZZ_THREADX)
#include <tx_api.h> /* includes tx_port.h */
#define BUZZZ_LOCK_DECLARE              TX_INTERRUPT_SAVE_AREA
#define BUZZZ_LOCK_INT                  TX_DISABLE
#define BUZZZ_UNLOCK_INT                TX_RESTORE
#else   /* ! BCM_BUZZZ_THREADX */
#define BUZZZ_LOCK_DECLARE
#define BUZZZ_LOCK_INT                  BCM_BUZZZ_NULL_STMT
#define BUZZZ_UNLOCK_INT                BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_THREADX */

#else   /* ! BCM_BUZZZ */

#define BCM_BUZZZ_INIT(shared)          BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_CONFIG(ctr_sel)       BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_SHOW()                BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_DUMP()                BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_START()               BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_STOP()                BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_WRAP()                BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_STATUS()              BCM_BUZZZ_RET_NONE
#define BCM_BUZZZ_MODE()                BCM_BUZZZ_RET_NONE
#define BCM_BUZZZ_SKIP(num)             BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_EMIT_COUNTERS()       BCM_BUZZZ_NULL_STMT

#define BUZZZ_LOCK_DECLARE
#define BUZZZ_LOCK_INT                  BCM_BUZZZ_NULL_STMT
#define BUZZZ_UNLOCK_INT                BCM_BUZZZ_NULL_STMT

#endif  /* ! BCM_BUZZZ */

#define BCM_BUZZZ_NOINSTR_FUNC          __attribute__ ((no_instrument_function))
#endif  /* __bcm_buzzz_h_included__ */
