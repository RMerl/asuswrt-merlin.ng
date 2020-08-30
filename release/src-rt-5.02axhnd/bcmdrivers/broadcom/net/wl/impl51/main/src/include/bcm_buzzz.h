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
 * BCM_BUZZZ_KPI_LEVEL       : Specify KPI instrumentation level
 *
 * As a cortex-a7 is used for several chips running at different Mhz, use ram.mk
 *
 * For Streaming to Host DDR Mode:
 * define BCM_BUZZZ_STREAMING_BUILD below.
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
 * $Id: bcm_buzzz.h 778855 2019-09-11 20:38:52Z $
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * +----------------------------------------------------------------------------
 */

#define BCM_BUZZZ_RET_NONE              (0)
#define BCM_BUZZZ_NULL_STMT             do { /* Nothing */ } while(0)

/* Only include events that use up max 2 args */
#if defined(BCM_BUZZZ_FUNC)
#undef BCM_BUZZZ_4ARGS
#endif // endif

// #define BCM_BUZZZ_STREAMING_BUILD
#define BCM_BUZZZ_STREAMING_FILE        "/tmp/buzzz.log"

#if defined(BCM_BUZZZ)

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
#define BCM_BUZZZ_TRACING_LEVEL         (3)            /* Buzzz tracing level */
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
#define BCM_BUZZZ_TRACING_LEVEL         (3)            /* Buzzz tracing level */
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
#define BCM_BUZZZ_CYCLES_PER_USEC       (CYCLES_PER_USEC)
#define BCM_BUZZZ_LOG_BUFSIZE           (8 * 4 * 1024) /* min 4K, 16Kmultiple */
#define BCM_BUZZZ_TRACING_LEVEL         (3)            /* Buzzz tracing level */
#define BCM_BUZZZ_KPI_LEVEL             (3)            /* Buzzz KPI level */
#define BCM_BUZZZ_COUNTERS              (4)            /* 4 PMU counters */
#endif /* ! CA7 */

#if defined(CA9)
/* Stuff for ARM Cortex-A9 */
#endif // endif

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
#define BCM_BUZZZ_ENABLED               (0)
#define BCM_BUZZZ_DISABLED              (1)
#define BCM_BUZZZ_FALSE                 (0)
#define BCM_BUZZZ_TRUE                  (1)
#define BCM_BUZZZ_INVALID               (~0U)

#define BCM_BUZZZ_INLINE                inline  __attribute__ ((always_inline))
#define BCM_BUZZZ_NOINSTR_FUNC          __attribute__ ((no_instrument_function))

void bcm_buzzz_log0(uint32 evtid);
void bcm_buzzz_log1(uint32 evtid, uint32 arg1);

#if defined(BCM_BUZZZ_STREAMING_BUILD) && defined(BCM_BUZZZ_FUNC)
/* FUNC Mode: arg2 is uint16 */
void bcm_buzzz_log2(uint32 evtid, uint32 arg1, uint16 arg2);
#else  /* ! (BCM_BUZZZ_STREAMING_BUILD && BCM_BUZZZ_FUNC) */
void bcm_buzzz_log2(uint32 evtid, uint32 arg1, uint32 arg2);
#endif /* ! (BCM_BUZZZ_STREAMING_BUILD && BCM_BUZZZ_FUNC) */

#if defined(BCM_BUZZZ_4ARGS)
#if defined(BCM_BUZZZ_FUNC)
#error "BCM_BUZZZ_FUNC incompatible with BCM_BUZZZ_4ARGS"
#endif // endif
#if defined(BCM_BUZZZ_STREAMING_BUILD)
void bcm_buzzz_log3(uint32 evtid, uint32 arg1, uint32 arg2, uint16 arg3);
#define bcm_buzzz_log4(evt, arg1, arg2, arg3, arg4) BCM_BUZZZ_NULL_STMT
#else  /* ! BCM_BUZZZ_STREAMING_BUILD */
void bcm_buzzz_log3(uint32 evtid, uint32 arg1, uint32 arg2, uint32 arg3);
void bcm_buzzz_log4(uint32 evtid, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4);
#endif /* ! BCM_BUZZZ_STREAMING_BUILD */
#else  /* ! BCM_BUZZZ_4ARGS */
#define bcm_buzzz_log3(evt, arg1, arg2, arg3)       BCM_BUZZZ_NULL_STMT
#define bcm_buzzz_log4(evt, arg1, arg2, arg3, arg4) BCM_BUZZZ_NULL_STMT
#endif /* ! BCM_BUZZZ_4ARGS */

/** BUZZZ Exported APIs */
int   bcm_buzzz_init(void * shared);
void  bcm_buzzz_config(uint32 ctr_sel);
void  bcm_buzzz_show(void);
void  bcm_buzzz_dump(void);
void  bcm_buzzz_start(void);
void  bcm_buzzz_stop(void);
void  bcm_buzzz_wrap(void);
uint8 bcm_buzzz_status(void);
uint8 bcm_buzzz_mode(void);
void  bcm_buzzz_skip(uint8 skip);

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
	BCM_BUZZZ_START_COMMAND = 1,
	BCM_BUZZZ_STOP_COMMAND  = 2
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
			uint16  arg3;   /* last arg3 is ONLY 16 bits ! */
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
	uint32          seg_haddr32[BCM_BUZZZ_SEGMENTS];
} bcm_buzzz_t;

#endif /* BCM_BUZZZ_STREAMING_BUILD */

/* bcm_buzzz dump color highlighting */
#if defined(BCM_BUZZZ_EXE)
#define _CLR_(x) ""
#else
#define _CLR_(x) x
#endif // endif
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
 * Subsystem Context logged in the first 8K buffer at start of trace.
 * Last 256 eventIds in a uint16 are reserved for startup subsystem context.
 * +----------------------------------------------------------------------------
 */
typedef enum bcm_buzzz_subsys
{
	BUZZZ_CTX_SUBSYS = 0xF0,
	BUZZZ_RTE_SUBSYS,   /* */
	BUZZZ_BUS_SUBSYS,	/* [ea, ring_id, rd, rd_p, wr, <cfp_flowid, max512>] */
	BUZZZ_MAC_SUBSYS    /* [pkts, mpdus] */
} bcm_buzzz_subsys_t;

typedef struct bcm_buzzz_subsys_hdr
{
	uint8  id;
	uint8  u8;
	uint16 u16;
} bcm_buzzz_subsys_hdr_t;
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
	BUZZZ_KLOG__START_EVT = 0,              // level
	BUZZZ_KLOG(SWAP_EVT)                    // 1

	BUZZZ_KLOG(FUNC_ENT)                    // 2
	BUZZZ_KLOG(FUNC_EXT)                    // 3
	BUZZZ_KLOG(FUNC_LINE)

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
	BUZZZ_KLOG(HND_WORK_ENT)                // 3
	BUZZZ_KLOG(HND_WORK_RTN)                // 3
	BUZZZ_KLOG(HND_DPC_ENT)                 // 3
	BUZZZ_KLOG(HND_DPC_RTN)                 // 3
	BUZZZ_KLOG(HND_DPC_RTN_ERR)             // 1
	BUZZZ_KLOG(HND_TMR_ENT)                 // 3
	BUZZZ_KLOG(HND_TMR_RTN)                 // 3
	BUZZZ_KLOG(HND_TMR_CRT)                 // 4
	BUZZZ_KLOG(HND_TMR_DEL)                 // 4
	BUZZZ_KLOG(HND_TMR_BGN)                 // 5
	BUZZZ_KLOG(HND_TMR_END)                 // 5

	BUZZZ_KLOG(THREADX_IDLE)                // 3
	BUZZZ_KLOG(THREADX_CPU_ISR_ENT)         // 4
	BUZZZ_KLOG(THREADX_CPU_ISR_RTN)         // 4
	BUZZZ_KLOG(THREADX_ISR_ENT)             // 3
	BUZZZ_KLOG(THREADX_ISR_RTN)             // 3
	BUZZZ_KLOG(THREADX_EVT_ISR_ENT)         // 5
	BUZZZ_KLOG(THREADX_EVT_ISR_RTN)         // 5
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
	_M_ "   >>> WORK fn[@%08x]" _N_,                  /* HND_WORK_ENT */       \
	_M_ "   <<< WORK" _N_,                            /* HND_WORK_RTN */       \
	_M_ "    >> DPC fn[@%08x]" _N_,                   /* HND_DPC_ENT */        \
	_M_ "    << DPC" _N_,                             /* HND_DPC_RTN */        \
	_M_ "  <<<< DPC ERROR" _N_,                       /* HND_DPC_RTN_ERR */    \
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
	_R_ "   >>> ISR fn[@%08x]" _N_,                 /* THREADX_ISR_ENT */      \
	_R_ "   <<< ISR" _N_,                           /* THREADX_ISR_RTN */      \
	_R_ "    >> EISR fn[@%08x] evt<%d>" _N_,        /* THREADX_EVT_ISR_ENT */  \
	_R_ "    << EISR" _N_,                          /* THREADX_EVT_ISR_RTN */  \
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
 * BUZZZ datapath KPI instrumentation in addition to default Threadx and RTOS
 * Assumes all HWA blocks are enabled
 */

#if defined(BCM_BUZZZ_KPI_LEVEL) && (BCM_BUZZZ_KPI_LEVEL > 0)
#define BCM_BUZZZ_KPI_PKT_LEVEL         (3)
#define BCM_BUZZZ_KPI_QUE_LEVEL         (3)
#else
#define BCM_BUZZZ_KPI_PKT_LEVEL         (0)
#define BCM_BUZZZ_KPI_QUE_LEVEL         (0)
#endif /* BCM_BUZZZ_KPI_LEVEL */

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

#if defined(BCM_BUZZZ_KPI_QUE_LEVEL) && (BCM_BUZZZ_KPI_QUE_LEVEL > 0)
uint8 * buzzz_bus(uint8 *buzzz_log);
uint8 * buzzz_mac(uint8 *buzzz_log);
#endif /* BCM_BUZZZ_KPI_QUE_LEVEL */

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
#define BCM_BUZZZ_WRAP()                bcm_buzzz_wrap()
#define BCM_BUZZZ_STATUS()              bcm_buzzz_status()
#define BCM_BUZZZ_MODE()                bcm_buzzz_mode()
#define BCM_BUZZZ_SKIP(num)             bcm_buzzz_skip(num)

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

#define BUZZZ_LOCK_DECLARE
#define BUZZZ_LOCK_INT                  BCM_BUZZZ_NULL_STMT
#define BUZZZ_UNLOCK_INT                BCM_BUZZZ_NULL_STMT

#endif  /* ! BCM_BUZZZ */

#define BCM_BUZZZ_NOINSTR_FUNC          __attribute__ ((no_instrument_function))

#endif  /* __bcm_buzzz_h_included__ */
