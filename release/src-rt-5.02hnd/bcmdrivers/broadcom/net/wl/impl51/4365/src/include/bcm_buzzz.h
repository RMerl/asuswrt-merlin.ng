#ifndef __bcm_buzzz_h_included__
#define __bcm_buzzz_h_included__

/*
 * +----------------------------------------------------------------------------
 *
 * BCM BUZZZ Performance tracing tool for ARM Cortex-R4, Cortex-M3
 *
 * BCM_BUZZZ_CYCLES_PER_USEC : Specify Processor speed
 * BCM_BUZZZ_LOG_BUFSIZE     : Specify log buffer size
 * BCM_BUZZZ_TRACING_LEVEL   : Specify tracing level
 *
 *
 * This file is shared between DHD and Dongle firmware.
 * Several implementations of buzzz exist:
 * - router linux
 * - standalone for proprietary 3rd party OS
 * - dongle
 * All exported APIs in dongle buzzz take the bcm_buzzz prefix and logging
 * macros use BUZZZ_LVL#(). Tracing macros not prefixed with "BCM_"
 *
 *
 * Reference dongle/shared/startarm.S, to pick the DFLAGS for -march and -mcpu
 *
 *    __ARM_ARCH_7R__    : CR4, hndrte  -march=armv7-r -mcpu=cortex-r4
 *    __ARM_ARCH_7M__    : CM3, bmos    -march=armv7-m -mcpu=cortex-m3
 *    __ARM_ARCH_7A__ CA7: CA7, threadX -march=armv7-a -mcpu=cortex-a7
 *    __ARM_ARCH_7A__ CA9: not reqd for dongle, uses ARMv7 apis as in CA7
 *
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

#define BCM_BUZZZ_NULL_STMT             do { /* Nothing */ } while(0)

#if defined(BCM_BUZZZ)

#define BCM_BUZZZ_COUNTERS_MAX          (8)

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
/* -mcpu=cortex-a7 -DCA7 */
#define BCM_BUZZZ_CA7_INSTRCNT_OVHD     (0)
#define BCM_BUZZZ_CA7_CYCCNT_OVHD       (0)
#define BCM_BUZZZ_CA7_BRMISS_OVHD       (0)
#define BCM_BUZZZ_CA7_DATAMEMACC_OVHD   (0)

/* -------------------------------------------------------------------------- */

/*
 * +----------------------------------------------------------------------------
 * ARM Cortex CR4, HNDRTE configuration
 * +----------------------------------------------------------------------------
 */
#if defined(__ARM_ARCH_7R__)
#define BCM_BUZZZ_HNDRTE
#define BCM_BUZZZ_CONFIG_CPU_ARM_CR4                   /* -mcpu=cortex-cr4 */
#define BCM_BUZZZ_CYCLES_PER_USEC       (320)
#define BCM_BUZZZ_LOG_BUFSIZE           (4 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_TRACING_LEVEL         (5)            /* Buzzz tracing level */
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
#define BCM_BUZZZ_CYCLES_PER_USEC       (160)
#define BCM_BUZZZ_LOG_BUFSIZE           (4 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_TRACING_LEVEL         (5)            /* Buzzz tracing level */
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
#define BCM_BUZZZ_ARMV7_SWINC_EVT		(0x00)	/* Software increment */
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
#define BCM_BUZZZ_ARMV7_DCACHEEVICT_EVT (0x15) /* Data cache eviction */
#define BCM_BUZZZ_ARMV7_2DCACHEACC_EVT  (0x16)  /* L2 data cache access */
#define BCM_BUZZZ_ARMV7_2DCACHEMISS_EVT (0x17) /* L2 data cache misses */
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
#define BCM_BUZZZ_ARMV7_PREF_DROP_EVT   (0xc3) /* Prefetch line fill dropped */
#define BCM_BUZZZ_ARMV7_WR_STALL_EVT    (0xc9) /* Wipeline stall, WR store full */
#define BCM_BUZZZ_ARMV7_DATASNOOP_EVT   (0xca) /* Data snooped fr other proc */

#if defined(CA7)
/*
 * +----------------------------------------------------------------------------
 * ARM Cortex CA7, ThreadX configuration
 * +----------------------------------------------------------------------------
 */
#define BCM_BUZZZ_THREADX
#define BCM_BUZZZ_CONFIG_CPU_ARM_CA7                   /* -mcpu=cortex-a7 */
#define BCM_BUZZZ_CYCLES_PER_USEC       (800)
#define BCM_BUZZZ_LOG_BUFSIZE           (4 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_TRACING_LEVEL         (5)            /* Buzzz tracing level */
#define BCM_BUZZZ_COUNTERS              (4)            /* 4 PMU counters */
#endif /* ! CA7 */

#if defined(CA9)
/* Stuff for ARM Cortex-A9 */
#endif

#endif /* __ARM_ARCH_7A__ */


/*
 * +----------------------------------------------------------------------------
 * Architecture independent definitions and APIs
 * +----------------------------------------------------------------------------
 */
#define BCM_BUZZZ_LOGENTRY_MAXSZ        (64)

#define BCM_BUZZZ_ERROR                 (-1)
#define BCM_BUZZZ_SUCCESS               (0)
#define BCM_BUZZZ_FAILURE               BCM_BUZZZ_ERROR
#define BCM_BUZZZ_DISABLED              (0)
#define BCM_BUZZZ_ENABLED               (1)
#define BCM_BUZZZ_FALSE                 (0)
#define BCM_BUZZZ_TRUE                  (1)
#define BCM_BUZZZ_INVALID               (~0U)

#define BCM_BUZZZ_INLINE                inline  __attribute__ ((always_inline))
#define BCM_BUZZZ_NOINSTR_FUNC          __attribute__ ((no_instrument_function))

extern void bcm_buzzz_log0(uint32 evtid);
extern void bcm_buzzz_log1(uint32 evtid, uint32 arg1);
extern void bcm_buzzz_log2(uint32 evtid, uint32 arg1, uint32 arg2);

/* BCM_BUZZZ used for debugging may require more than one arg per log entry */
#if defined(BCM_BUZZZ_4ARGS)
extern void bcm_buzzz_log3(uint32 evtid, uint32 arg1, uint32 arg2, uint32 arg3);
extern void bcm_buzzz_log4(uint32 evtid, uint32 arg1, uint32 arg2,
                                      uint32 arg3, uint32 arg4);
#else  /* ! BCM_BUZZZ_4ARGS */
#define bcm_buzzz_log3(evt, arg1, arg2, arg3)       BCM_BUZZZ_NULL_STMT
#define bcm_buzzz_log4(evt, arg1, arg2, arg3, arg4) BCM_BUZZZ_NULL_STMT
#endif /* ! BCM_BUZZZ_4ARGS */


extern int  bcm_buzzz_init(void * shared);
extern void bcm_buzzz_config(uint32 ctr_sel);
extern void bcm_buzzz_show(void);
extern void bcm_buzzz_dump(void);
extern void bcm_buzzz_start(void);
extern void bcm_buzzz_stop(void);

typedef struct bcm_buzzz
{
	uint32          log;
	uint32          cur;        /* pointer to next log entry */
	uint32          end;        /* pointer to end of log entry */

	uint16          count;      /* count of logs, wraps on 64K */
	uint8           status;     /* runtime status */
	uint8           wrap;       /* log buffer wrapped */

	uint8           cpu_idcode; /* ARM idcode CM3=0x03, CR4=0x04,CR7=0x07 */
	uint8           counters;   /* number of counter logged */
	uint8           group;      /* event group */
	uint8           rsvd;

	uint32          buffer_sz;
	uint32          log_sz;

	uint32          eventid[BCM_BUZZZ_COUNTERS_MAX]; /* event selected */
} bcm_buzzz_t;

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


typedef enum bcm_buzzz_ctrl {
	BCM_BUZZZ_START_COMMAND = 1,
	BCM_BUZZZ_STOP_COMMAND  = 2
} bcm_buzzz_ctrl_t;


/* bcm_buzzz dump color highlighting */
#undef  _R_
#undef  _B_
#undef  _C_
#undef  _G_
#undef  _M_
#undef  _H_
#undef  _N_
#undef  _FAIL_
/* _B_ Blue, _C_ Cyan, _G_ Green, _M_ Magenta, _H_ Red_on_Black,  */
#define _R_     "\e[0;31m"
#define _B_     "\e[0;34m"
#define _C_     "\e[0;38m"
#define _G_     "\e[0;32m"
#define _M_     "\e[0;35m"
#define _H_     "\e[0;31m;40m"
#define _N_     "\e[0m"
#define _FAIL_  _H_ " === FAILURE ===" _N_

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

#undef BUZZZ_KLOG
#define BUZZZ_KLOG(event)       BUZZZ_KLOG__ ## event,
typedef enum bcm_buzzz_KLOG_dpid    /* List of datapath event point ids */
{
	BUZZZ_KLOG__START_EVT = 0,

	BUZZZ_KLOG(BUZZZ_0)
	BUZZZ_KLOG(BUZZZ_1)
	BUZZZ_KLOG(BUZZZ_2)
	BUZZZ_KLOG(BUZZZ_3)
	BUZZZ_KLOG(BUZZZ_4)

	BUZZZ_KLOG(HND_DIE)
	BUZZZ_KLOG(HND_TRAP)
	BUZZZ_KLOG(HND_ASSERT)
	BUZZZ_KLOG(HND_DELAY)
	BUZZZ_KLOG(HND_MALLOC)
	BUZZZ_KLOG(HND_FREE)
	BUZZZ_KLOG(HND_SCHED_WORK)
	BUZZZ_KLOG(HND_WORK_ENT)
	BUZZZ_KLOG(HND_WORK_RTN)
	BUZZZ_KLOG(HND_DPC_ENT)
	BUZZZ_KLOG(HND_DPC_RTN)
	BUZZZ_KLOG(HND_DPC_RTN_ERR)
	BUZZZ_KLOG(HND_TMR_ENT)
	BUZZZ_KLOG(HND_TMR_RTN)
	BUZZZ_KLOG(HND_TMR_CRT)
	BUZZZ_KLOG(HND_TMR_DEL)
	BUZZZ_KLOG(HND_TMR_BGN)
	BUZZZ_KLOG(HND_TMR_END)

	BUZZZ_KLOG(THREADX_IDLE)
	BUZZZ_KLOG(THREADX_CPU_ISR_ENT)
	BUZZZ_KLOG(THREADX_CPU_ISR_RTN)
	BUZZZ_KLOG(THREADX_ISR_ENT)
	BUZZZ_KLOG(THREADX_ISR_RTN)
	BUZZZ_KLOG(THREADX_EVT_ISR_ENT)
	BUZZZ_KLOG(THREADX_EVT_ISR_RTN)

	BUZZZ_KLOG__LAST_EVT

} bcm_buzzz_KLOG_dpid_t;

#endif /* BCM_BUZZZ */

/*
 * All format strings must be listed in the same sequence (comma seperated) as
 * their corresponding eventids in enum list bcm_buzzz_KLOG_dpid_t.
 *
 * NOTE: While constructing the format string, please use "ra[@%p]" when you
 * log the return address using (uint32)__builtin_return_address(0). A script
 * such as buzzz_sym.pl may then be used to convert all instruction addresses
 * to their symbol-names using the corresponding rtecdc.map.
 *     buzzz_smp.pl -m <rtecdc.map> -i <logfile> -o <outfile>
 *
 * Likewise, any function pointer may be logged with a corresponding "fn[@%p]"
 */
#define BCM_BUZZZ_FMT_STRINGS                                                  \
{                                                                              \
	"START_EVT",                          /* START_EVT */                      \
	                                                                           \
	"buzzz_log0",                         /* BUZZZ_0 */                        \
	"buzzz_log1 arg<%u>",                 /* BUZZZ_1 */                        \
	"buzzz_log2 arg<%u:%u>",              /* BUZZZ_2 */                        \
	"buzzz_log3 arg<%u:%u:%u>",           /* BUZZZ_3 */                        \
	"buzzz_log4 arg<%u:%u:%u:%u>",        /* BUZZZ_4 */                        \
	                                                                           \
	/* HND THREADX: with color formatting */                                   \
	_H_ ":::die ra[@%p]" _N_,             /* HND_DIE */                        \
	_H_ ":::trap pc[@%p] type<%d>" _N_,   /* HND_TRAP */                       \
	_H_ ":::assert ra[@%p] line<%d>" _N_, /* HND_ASSERT */                     \
	_B_ ":::delay ra[@%p] usec<%d>" _N_,  /* HND_DELAY */                      \
	_B_ ":::malloc ra[@%p] size<%d>" _N_, /* HND_MALLOC */                     \
	_B_ ":::free ra[@%p]" _N_,            /* HND_FREE */                       \
	_B_ ":::sched fn[@%p] delay<%u>" _N_, /* HND_SCHED_WORK */                 \
	_M_ "   >>> WORK fn[@%p]" _N_,        /* HND_WORK_ENT */                   \
	_M_ "   <<< WORK" _N_,                /* HND_WORK_RTN */                   \
	_M_ "   >>>> DPC evt<%d>" _N_,        /* HND_DPC_ENT */                    \
	_M_ "   <<<< DPC" _N_,                /* HND_DPC_RTN */                    \
	_M_ "   <<<< DPC ERR" _N_,            /* HND_DPC_RTN_ERR */                \
	_M_ "   >>>> TMR fn[@%p]" _N_,        /* HND_TMR_ENT */                    \
	_M_ "   <<<< TMR" _N_,                /* HND_TMR_RTN */                    \
	_B_ ":::tmr_create ra[@%p]" _N_,      /* HND_TMR_CRT */                    \
	_B_ ":::tmr_delete ra[@%p]" _N_,      /* HND_TMR_DEL */                    \
	_B_ ":::tmr_start ra[@%p]" _N_,       /* HND_TMR_BGN */                    \
	_B_ ":::tmr_stop ra[@%p]" _N_,        /* HND_TMR_END */                    \
	                                                                           \
	_G_ ":::idle loop" _N_,               /* THREADX_IDLE */                   \
	_R_ "  >>>> CPU ISR lr<@%p>" _N_,     /* THREADX_CPU_ISR_ENT */            \
	_R_ "  <<<< CPU ISR" _N_,             /* THREADX_CPU_ISR_RTN */            \
	_R_ "   >>> ISR fn<@%p>" _N_,         /* THREADX_ISR_ENT */                \
	_R_ "   <<< ISR" _N_,                 /* THREADX_ISR_RTN */                \
	_R_ ">>>>>> EISR fn<@%p> evt<%d>" _N_, /* THREADX_EVT_ISR_ENT */           \
	_R_ "<<<<<< EISR" _N_,                /* THREADX_EVT_ISR_RTN */            \
	                                                                           \
	/* User Events without color formatting */                                 \
	                                                                           \
	"LAST_EVENT"                                                               \
}


/*
 * +----------------------------------------------------------------------------
 *  Insert instrumentation in code at various tracing levels using
 *
 *   BUZZZ_LVL#(EVENT_ENUM, NUM_ARGS, ARGS_LIST)
 *
 *     #         : Compile time tracing level, BCM_BUZZZ_TRACING_LEVEL >= #
 *     EVENT_ENUM: Enum added to buzzz_KLOG_dpid_t using BUZZZ_KLOG()
 *     NUM_ARGS  : Number of arguments to log, max 4 arguments
 *     ARGS_LIST : List of arguments, comma seperated
 *
 * +----------------------------------------------------------------------------
 */

#undef BUZZZ_LVL1
#if defined(BCM_BUZZZ_TRACING_LEVEL) && (BCM_BUZZZ_TRACING_LEVEL >= 1)
#define BUZZZ_LVL1(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_TRACING_LEVEL >= 1 */
#define BUZZZ_LVL1(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_TRACING_LEVEL >= 1 */


#undef BUZZZ_LVL2
#if defined(BCM_BUZZZ_TRACING_LEVEL) && (BCM_BUZZZ_TRACING_LEVEL >= 2)
#define BUZZZ_LVL2(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_TRACING_LEVEL >= 2 */
#define BUZZZ_LVL2(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_TRACING_LEVEL >= 2 */


#undef BUZZZ_LVL3
#if defined(BCM_BUZZZ_TRACING_LEVEL) && (BCM_BUZZZ_TRACING_LEVEL >= 3)
#define BUZZZ_LVL3(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_TRACING_LEVEL >= 3 */
#define BUZZZ_LVL3(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_TRACING_LEVEL >= 3 */


#undef BUZZZ_LVL4
#if defined(BCM_BUZZZ_TRACING_LEVEL) && (BCM_BUZZZ_TRACING_LEVEL >= 4)
#define BUZZZ_LVL4(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_TRACING_LEVEL >= 4 */
#define BUZZZ_LVL4(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_TRACING_LEVEL >= 4 */


#undef BUZZZ_LVL5
#if defined(BCM_BUZZZ_TRACING_LEVEL) && (BCM_BUZZZ_TRACING_LEVEL >= 5)
#define BUZZZ_LVL5(ID, N, ARG...)   bcm_buzzz_log ##N(BUZZZ_KLOG__ ##ID, ##ARG)
#else   /* ! BCM_BUZZZ_TRACING_LEVEL >= 5 */
#define BUZZZ_LVL5(ID, N, ARG...)   BCM_BUZZZ_NULL_STMT
#endif  /* ! BCM_BUZZZ_TRACING_LEVEL >= 5 */

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

#else	/* ! BCM_BUZZZ */

#define BCM_BUZZZ_INIT(shared)          BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_CONFIG(ctr_sel)       BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_SHOW()                BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_DUMP()                BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_START()               BCM_BUZZZ_NULL_STMT
#define BCM_BUZZZ_STOP()                BCM_BUZZZ_NULL_STMT

#endif	/* ! BCM_BUZZZ */

#endif  /* __bcm_buzzz_h_included__ */
