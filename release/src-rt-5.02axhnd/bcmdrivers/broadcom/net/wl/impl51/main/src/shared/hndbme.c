/*
 * Generic Broadcom Home Networking Division (HND) BME module.
 * This supports chips with revs >= 128.
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
 * $Id: hndbme.c 999999 2017-01-04 16:02:31Z $
 */

/**
 * @file
 * @brief
 * Source file for HNDBME module. This file contains the functionality to initialize and run the
 * BME engine. BME stands for Byte Move Engine. The BME is a non-descriptor based DMA engine
 * capable of handling a single asynchronous transfer at a time, which gets programmed solely
 * through registers.
 */

/**
 * XXX For more information, see:
 * Confluence:[BME%2C+the+non-descriptor+based+DMA+engine]
 */

#include <osl.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <siutils.h>
#include <sbhnddma.h>       /* for dma64regs_t and DMA descriptor programming */
#include <m2mdma_core.h>    /* for m2m_core_regs_t, m2m_eng_regs_t */

#include <hndbme.h>

/**
 * ------------------------------------------------------------------------------
 * Section: BME Conditional Compile
 * ------------------------------------------------------------------------------
 */

/** Compile time (.mk) override */
#ifndef BME_ENG_MAX
#define BME_ENG_MAX     4
#endif // endif
#if (BME_ENG_MAX > 8)
#error "Maximum 8 Engines supported"
#endif // endif

#define BME_NOOP                    do { /* no-op */ } while(0)
#define BME_PRINT                   printf

/** Conditional Compile: Designer builds for extended debug and statistics */
// #define BME_DEBUG_BUILD
#define BME_STATS_BUILD

#define BME_CTRS_RDZERO             /* Clear On Read */
#if defined(BME_CTRS_RDZERO)
#define BME_CTR_ZERO(ctr)           (ctr) = 0U
#else
#define BME_CTR_ZERO(ctr)           BME_NOOP
#endif // endif

#if defined(BME_DEBUG_BUILD)
#define BME_DEBUG(expr)             expr
#else  /* ! BME_DEBUG_BUILD */
#define BME_DEBUG(expr)             BME_NOOP
#endif /* ! BME_DEBUG_BUILD */

#if defined(BME_STATS_BUILD)
#define BME_STATS(expr)             expr
#define BME_STATS_ZERO(ctr)         BME_CTR_ZERO(ctr)
#else  /* ! BME_STATS_BUILD */
#define BME_STATS(expr)             BME_NOOP
#define BME_STATS_ZERO(expr)        BME_NOOP
#endif /* ! BME_STATS_BUILD */

#define BME_BURSTLEN            2           /* 2 ^ (N+4) => 64 byte burst */
#define BME_PCI64ADDR_HIGH      0x80000000  /* hnddma_priv.h PCI64ADDR_HIGH */

/**
 * ------------------------------------------------------------------------------
 * Section: SoC spec XXX
 *
 * Byte Move Engine instances for HW Assisted Copy.
 *
 * BME is part of M2MDMA Core (Id 0x844)
 *
 * Reference: [Twiki: M2MDmaRev0, CurrentDmaProgGuide]
 *            CRBCAM2MDMA-54 "Request for non descriptor based DMA"
 *
 * Rev  CHIPID       BME_ENG  DMA CHANNELS      COMMENT
 *   3  43684 Bx     2        2, 3
 * 128  63178/6750   1        2                 MAC uses Ch #3 [Tx/Rx Status]
 * 128  47622/6755   1        2                 MAC uses Ch #3 [Tx/Rx Status]
 * 129  6710         1        2                 MAC uses Ch #3 [Tx/Rx Status]
 * 130  43684 C0     2        2, 3              CRWLDMA-168 WC 1b15 per desc
 * 131  6715         4        2, 4, 5, 6        MAC uses Ch #3 [Tx/Rx Status]
 *                                              CRBCAM2MDMA-73 Support 8 channel
 *                                              CRBCAM2MDMA-75 Ch#7 PSMx to Host
 *
 * Traditional M2M (descriptor based): Ch #0, #1 (in all M2MCORE revisions)
 * Simple M2M (non descriptor based) : Ch #2, (3*), #4, #5, #6
 *
 * When Ch #3 is re-pruposed by MAC, it must be excluded from BME pool of engines
 *
 * Implementation assumes mutual exclusion is provided by caller.
 *
 * ------------------------------------------------------------------------------
 */

/**
 * ------------------------------------------------------------------------------
 * BME specific register specification:
 *
 * USE register sepcifications from sbhnddma.h and m2mdma_core.h.
 * Only new BME registers or "re-purposed" dma64regs may be listed here, using
 * naming convention from sbhnddma.h and m2mdma_core.h.
 * Note: BME_XYZ_MASK is already shifted, to leverage BCM_BIT_MANIP_MACROS.
 * ------------------------------------------------------------------------------
 */

/**
 * BME re-purposed dma64regs_t XmtPtr (D64 XP).
 * This is specific to Simple M2M (BME) (not common to m2mdma_core.h)
 */
/** BME dma64regs_t XmtPtr Buffer Byte Count (D64 XP BC) */
#define BME_D64_XP_BC_NBITS     13
#define BME_D64_XP_BC_SHIFT     0
#define BME_D64_XP_BC_MASK      BCM_MASK(BME_D64_XP_BC)
/** BME dma64regs_t XmtPtr AddrExt (D64 XP AE) */
#define BME_D64_XP_AE_NBITS     1
#define BME_D64_XP_AE_SHIFT     13
#define BME_D64_XP_AE_MASK      BCM_MASK(BME_D64_XP_AE)
/** BME dma64regs_t XmtPtr Coherent (D64 XP CO) */
#define BME_D64_XP_CO_NBITS     1
#define BME_D64_XP_CO_SHIFT     14
#define BME_D64_XP_CO_MASK      BCM_MASK(BME_D64_XP_CO)
/** BME dma64regs_t XmtPtr Start|Busy (D64 XP SB) */
#define BME_D64_XP_SB_NBITS     1
#define BME_D64_XP_SB_SHIFT     31
#define BME_D64_XP_SB_MASK      BCM_MASK(BME_D64_XP_SB)

/**
 * BME re-purposed dma64regs_t RcvPtr (D64 RP)
 * This is specific to Simple M2M (BME) (not common to m2mdma_core.h)
 */
/** BME dma64regs_t RcvPtr AddrExt (D64 RP AE) */
#define BME_D64_RP_AE_NBITS     1
#define BME_D64_RP_AE_SHIFT     13
#define BME_D64_RP_AE_MASK      BCM_MASK(BME_D64_RP_AE)
/** BME dma64regs_t RcvPtr Coherent (D64 RP CO) */
#define BME_D64_RP_CO_NBITS     1
#define BME_D64_RP_CO_SHIFT     14
#define BME_D64_RP_CO_MASK      BCM_MASK(BME_D64_RP_CO)

/**
 * ------------------------------------------------------------------------------
 * Section: BME Direction
 * ------------------------------------------------------------------------------
 */

/** Encode SRC and DST memory into a DIR 2-Byte tuple<uint8,uint8> */
#define BME_SRC2DIR(src)        ((src) << 0) /* lsbyte in direction tuple */
#define BME_DST2DIR(dst)        ((dst) << 8) /* msbyte in direction tuple */

#define BME_DIR(src, dst)       (BME_SRC2DIR(src) | BME_DST2DIR(dst))

#define BME_DIR2SRC(dir)        ((dir) & BME_MEM_MASK)
#define BME_DIR2DST(dir)        (((dir) >> 8) & BME_MEM_MASK)

#define BME_DIR_UNDEF           (BME_DIR(BME_MEM_NONE, BME_MEM_NONE))

/**
 * Classic Dongle Driver Mode (PCIe)
 * ---------------------------------
 * WLAN Chipset SoC SysMem --> Router SoC Host DDR Mem (over PCIe)
 * Router SoC Host DDR Mem --> WLAN Chipset SoC SysMem (over PCIe)
 * WLAN Chipset SoC SysMem --> WLAN Chipset MAC IntMem (not over PCIe)
 * WLAN Chipset MAC IntMem --> WLAN Chipset MAC IntMem (not over PCIe)
 * WLAN Chipset SoC SysMem --> WLAN Chipset SoC SysMem (not over PCIe)
 */
#define BME_DIR_DNGL2PCIE       (BME_DIR(BME_MEM_DNGL, BME_MEM_PCIE))
#define BME_DIR_PCIE2DNGL       (BME_DIR(BME_MEM_PCIE, BME_MEM_DNGL))
#define BME_DIR_DNGL2CHIP       (BME_DIR(BME_MEM_DNGL, BME_MEM_CHIP))
#define BME_DIR_CHIP2DNGL       (BME_DIR(BME_MEM_CHIP, BME_MEM_DNGL))
#define BME_DIR_DNGL2DNGL       (BME_DIR(BME_MEM_DNGL, BME_MEM_DNGL))

/**
 * Classic NIC Mode Driver (PCIe)
 * ------------------------------
 * Router SoC Host DDR Mem --> WLAN Chipset MAC IntMem (over PCIe)
 * WLAN Chipset MAC IntMem --> Router SoC Host DDR Mem (over PCIe)
 */
#define BME_DIR_PCIE2CHIP       (BME_DIR(BME_MEM_PCIE, BME_MEM_CHIP))
#define BME_DIR_CHIP2PCIE       (BME_DIR(BME_MEM_CHIP, BME_MEM_PCIE))

/**
 * Integrated NIC Mode Driver (UBUS)
 * ---------------------------------
 * Router SoC Host DDR Mem --> WLAN Chipset MAC IntMem (not over PCIe)
 * WLAN Chipset MAC IntMem --> Router SoC Host DDR Mem (not over PCIe)
 * Router SoC Host DDR Mem --> Router SoC Host DDR Mem (not over PCIe) *memcpy*
 */
#define BME_DIR_UBUS2CHIP       (BME_DIR(BME_MEM_UBUS, BME_MEM_CHIP))
#define BME_DIR_CHIP2UBUS       (BME_DIR(BME_MEM_CHIP, BME_MEM_UBUS))
#define BME_DIR_UBUS2UBUS       (BME_DIR(BME_MEM_UBUS, BME_MEM_UBUS))

/**
 * UBUS based Dongle Mode (Low SysMem)
 * -----------------------------------
 * WLAN Chipset SoC SysMem --> Router SoC Host DDR Mem (not over PCIe, over UBUS)
 * Router SoC Host DDR Mem --> WLAN Chipset SoC SysMem (not over PCIe, over UBUS)
 */
#define BME_DIR_DNGL2UBUS       (BME_DIR(BME_MEM_DNGL, BME_MEM_UBUS))
#define BME_DIR_UBUS2DNGL       (BME_DIR(BME_MEM_UBUS, BME_MEM_DNGL))

/**
 * Dongle or NIC Mode
 * ------------------
 * WLAN Chipset MAC IntMem --> WLAN Chipset MAC IntMem (not over PCIe)
 */
#define BME_DIR_CHIP2CHIP       (BME_DIR(BME_MEM_CHIP, BME_MEM_CHIP))

/** Excluding WLAN Chipset's MAC Internal Memory, all other mem are "coherent" */
#define BME_MEM_COHERENT        (BME_MEM_DNGL | BME_MEM_PCIE | BME_MEM_UBUS)

/** When either src or dst is BME_MEM_PCIE, a DMA op is over PCIe */
#define BME_DIR_OVERPCIE        (BME_DIR(BME_MEM_PCIE, BME_MEM_PCIE))

/**
 * ------------------------------------------------------------------------------
 * Section: Helper Macros
 * ------------------------------------------------------------------------------
 */
#define BME_ISCOHERENT(mem)     ((mem) & BME_MEM_COHERENT)
#define BME_ISOVERPCIE(dir)     ((dir) & BME_DIR_OVERPCIE)

/** Macros used for PCI_BUS based drivers (e.g. NIC Mode driver, DHD driver) */
#define BME_SWITCHCORE(_sih_, _orig_id_, _intr_val_) \
({ \
	*_orig_id_ = 0; \
	*_intr_val_ = 0; \
	if (BUSTYPE(_sih_->bustype) == PCI_BUS) { \
		si_switch_core(_sih_, M2MDMA_CORE_ID, _orig_id_, _intr_val_); \
	} \
})

#define BME_RESTORECORE(_sih_, _core_id_, _intr_val_) \
if (BUSTYPE(_sih_->bustype) == PCI_BUS) \
	si_restore_core(_sih_, _core_id_, _intr_val_);

/** Data Barrier */
#if defined(__ARM_ARCH_7A__)
#define BME_DATA_BARRIER()      __asm__ __volatile__ ("dsb") /* or just dmb */
#else
#define BME_DATA_BARRIER()      do { /* no-op */ } while (0)
#endif /* __ARM_ARCH_7A__ */

/**
 * ------------------------------------------------------------------------------
 * Section: Typedefs
 * ------------------------------------------------------------------------------
 */

/** BME DMA Direction : <bme_src,bme_dst> tuple */
typedef union bme_dir {
	struct {
		bme_mem_t   src;                    /* lsbyte in dir tuple */
		bme_mem_t   dst;                    /* msbyte in dir tuple */
	} mem;
	uint16          u16;                    /* 2byte dir tuple */
} bme_dir_t;

/** BME configuration for a given user */
typedef struct bme_cfg {                    /* per user specification */
	bme_mem_t       src;                    /* src memory specification */
	bme_mem_t       dst;                    /* dst memory specification */
	bme_sel_t       sel;                    /* engine selection policy */
	bme_usr_t       usr;                    /* registered bme user id */
} bme_cfg_t;

/** BME key returned to user upon registration, with summarized user config */
typedef union bme_key {     /* wrapper to bme_cfg accessible as a u32 key */
	bme_cfg_t       cfg;
	uint32          u32;
} bme_key_t;

#define BME_KEY(bme_mem_src, bme_mem_dst, bme_sel, bme_usr) ({ \
	bme_key_t _bme_key; \
	_bme_key.u32 = 0; \
	_bme_key.cfg.src = bme_mem_src; _bme_key.cfg.dst = bme_mem_dst; \
	_bme_key.cfg.sel = bme_sel;     _bme_key.cfg.usr = bme_usr; \
	_bme_key.u32; \
})

/** Default System User uses key 0x00FFF0F0 */
#define BME_KEY_SYS \
	    BME_KEY(BME_MEM_NONE, BME_MEM_NONE, BME_SET_ANY, BME_USR_SYS)

/** Un-registered user will have key 0xFFFFFFFF */
#define BME_KEY_INV \
		BME_KEY(BME_MEM_INV, BME_MEM_INV, BME_SET_ANY, BME_USR_INV)

#define BME_REG_INV             (~0U)

/** Per engine register values */
typedef struct bme_reg {
	uint32          xmt_ptr;                /* tx::dma64regs::ptr -(BC, SB) */
	uint32          rcv_ptr;                /* rx::dma64regs::ptr */
	uint32          xmt_addrhigh;           /* tx::dma64regs::addrhigh */
	uint32          rcv_addrhigh;           /* rx::dma64regs::addrhigh */
} bme_reg_t;

/** Per User run-time context */
typedef struct bme_ctx {
	bme_set_t       busy_set;               /* bitmap: 0=IDLE, 1=BUSY. Runtime */
	bme_set_t       bme_set;                /* eligible engine bitmap */
	bme_sel_t       bme_sel;                /* BME selection policy */
	bme_usr_t       bme_usr;                /* bme_usr : redundant */

	bme_key_t       bme_key;                /* user configuration key */

	bme_mem_t       src_type;
	bme_mem_t       dst_type;
	uint32          hi_src;                 /* hi addr src */
	uint32          hi_dst;                 /* hi addr dst */

	bme_reg_t       bme_reg;                /* default [Xmt,Rcv]::[Ptr,AddrHi] values */

#if defined(BME_STATS_BUILD)
	uint32          xfers;                  /* transfer statistics */
	uint32          bytes;                  /* octet statistics */
	uint32          loops;                  /* loops for busy test */
	uint32          error;                  /* error statistics */
#endif   /* BME_STATS_BUILD */
} bme_ctx_t;

/** Per BME register set */
typedef struct bme_eng {
	bme_ctx_t                 *bme_ctx_prev;   /* last user of this engine */
	bme_reg_t                 bme_reg_cached;  /* cached [Xmt,Rcv]::[Ptr,AddrHi] values */
	volatile m2m_eng_regs_t   *m2m_eng_regs;   /* Transmit/Receive DMA registers */
#if defined(BME_STATS_BUILD)
	uint32          xfers;                  /* transfer statistics */
	uint32          loops;                  /* loops for busy test */
#endif   /* BME_STATS_BUILD */
} bme_eng_t;

/** BME Service Driver context: per engine, per user runtime and config state */
typedef struct bme {
	uint8           engines;                /* total available engines */
	bme_set_t       idle_set;               /* bitmap: 0=BUSY, 1=IDLE, Runtime */

	bme_eng_t       bme_eng[BME_ENG_MAX];   /* engine register and last user */

	bme_ctx_t       bme_ctx[BME_USR_MAX];   /* user context: config, state */

	volatile m2m_core_regs_t *m2m_core_regs;    /* m2m core registers */
	uint32          m2m_core_id;            /* m2m core id */
	uint32          m2m_core_rev;           /* m2m core revision */

	void           *osh;                    /* OS abstraction layer handler */
	si_t           *sih;                    /* SOC Interconnect handler */
} bme_t;

#if defined(DONGLEBUILD)
bme_t * g_bme = NULL;				/* single global bme instance */
#endif /* DONGLEBUILD */

/**
 * Human readable debug printing
 */
const char * bme_sel_str[BME_SEL_MAX] =
	{"RSV", "IDX", "SET", "ANY"};

#define BME_SEL_STR(bme_sel) \
({ \
	const char * _bme_sel_str; \
	_bme_sel_str = ((bme_sel) < BME_SEL_MAX) ? bme_sel_str[bme_sel] : "INV"; \
	_bme_sel_str; \
})

const char * bme_usr_str[BME_USR_MAX] =
	{"SYS", "RLM", "H2D", "D2H", "FD0", "FD1", "UN6", "UN7" };

#define BME_USR_STR(bme_usr) \
({ \
	const char * _bme_usr_str; \
	_bme_usr_str = ((bme_usr) < BME_USR_MAX) ? bme_usr_str[bme_usr] : "INV"; \
	_bme_usr_str; \
})

const char * bme_mem_str[BME_MEM_CHIP + 1] =
	{"UNKN", "DNGL", "PCIE", "UNKN", "UBUS", "UNKN", "UNKN", "UNKN", "CHIP" };

#define BME_MEM_STR(bme_mem) \
({ \
	const char * _bme_mem_str; \
	_bme_mem_str = ((bme_mem) < (BME_MEM_CHIP + 1)) ? \
		bme_mem_str[bme_mem] : "UNKN"; \
	_bme_mem_str; \
})

#define BME_KEY_FMT             "[%s,%s,%s,%s] "
#define BME_KEY_VAL(bme_key) \
	BME_MEM_STR((bme_key).cfg.src), BME_MEM_STR((bme_key).cfg.dst), \
	BME_SEL_STR((bme_key).cfg.sel), BME_USR_STR((bme_key).cfg.usr)

/**
 * ------------------------------------------------------------------------------
 * Section: Functional Interface
 * ------------------------------------------------------------------------------
 */

/**
 * ------------------------------------------------------------------------------
 * bme_dump() Debug dump of BME state.
 * Avoid asserts in dump function.
 * Verbose dump: print each engine state to include DMA engine registers.
 * ------------------------------------------------------------------------------
 */
void /* Debug dump the BME information. verbose implies engine state */
bme_dump(osl_t * osh, bool verbose)
{
	bme_t * bme;
	uint8 usr_idx; /* bme_ctx[] iterator */

	ASSERT(osh != (osl_t *)NULL);
	bme = (bme_t *)OSH_GETBME(osh);

	if (bme == (bme_t *)NULL) {
		BME_PRINT("BME Service not initialized\n");
		return;
	}

	BME_PRINT(
		"\nBME: engines %u M2M DMA Core id 0x%x rev %d regs %p idle 0x%02x\n",
		bme->engines, bme->m2m_core_id, bme->m2m_core_rev, bme->m2m_core_regs,
		bme->idle_set.map);

	BME_PRINT("\n    Usr Sel Set Bsy [Smem,Dmem,Sel,Usr] HiAddr_Src HiAddr_Dst "
		"XmtPtr_u32 RcvPtr_u32 Xmt_Hi_u32 Rcv_Hi_u32");
	BME_STATS(BME_PRINT(" [xfers bytes loops error]"));

	for (usr_idx = 0; usr_idx < BME_USR_MAX; ++usr_idx) {
		bme_ctx_t * bme_ctx = &bme->bme_ctx[usr_idx];

		if (bme_ctx->bme_key.u32 != BME_KEY_INV) {
			BME_PRINT("\n    %s %s x%02x x%02x " BME_KEY_FMT "0x%08x 0x%08x "
				"0x%08x 0x%08x 0x%08x 0x%08x",
				BME_USR_STR(bme_ctx->bme_usr), BME_SEL_STR(bme_ctx->bme_sel),
				bme_ctx->bme_set.map, bme_ctx->busy_set.map,
				BME_KEY_VAL(bme_ctx->bme_key),
				bme_ctx->hi_src, bme_ctx->hi_dst,
				bme_ctx->bme_reg.xmt_ptr, bme_ctx->bme_reg.rcv_ptr,
				bme_ctx->bme_reg.xmt_addrhigh, bme_ctx->bme_reg.rcv_addrhigh);

			BME_STATS(BME_PRINT(" [%u %u %u %u]",
				bme_ctx->xfers, bme_ctx->bytes, bme_ctx->loops, bme_ctx->error));
			BME_STATS(BME_CTR_ZERO(bme_ctx->xfers));
			BME_STATS(BME_CTR_ZERO(bme_ctx->bytes));
			BME_STATS(BME_CTR_ZERO(bme_ctx->loops));
			BME_STATS(BME_CTR_ZERO(bme_ctx->error));
		}
	}
	BME_PRINT("\n");

	if (verbose) { /* dump engine state ? */
		uint8 eng_idx; /* bme_eng[] iterator */
		bme_eng_t * bme_eng;
		volatile dma64regs_t * dma64regs_xmt; /* M2M DMA engine's Tx */
		volatile dma64regs_t * dma64regs_rcv; /* M2M DMA engine's Rx */

		BME_PRINT("\n    Eng Usr DmaREG"
			   " XmtPtr_REG Xmt_Hi_REG Xmt_Lo_REG"
			   " RcvPtr_REG Rcv_Hi_REG Rcv_Lo_REG");
		BME_STATS(BME_PRINT(" [xfers loops]"));

		for (eng_idx = 0; eng_idx < bme->engines; ++eng_idx) {

			bme_eng = &bme->bme_eng[eng_idx];
			if (bme_eng->m2m_eng_regs == NULL) {
				BME_PRINT("     %u. %s NULL\n",
					eng_idx, BME_USR_STR(bme_eng->bme_ctx_prev->bme_usr));
				continue;
			}

			dma64regs_xmt = &bme_eng->m2m_eng_regs->tx;
			dma64regs_rcv = &bme_eng->m2m_eng_regs->rx;

			BME_PRINT(
				"\n     %u. %s 0x%04x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
				eng_idx, BME_USR_STR(bme_eng->bme_ctx_prev->bme_usr),
				(uint)((uintptr)bme_eng->m2m_eng_regs -
					(uintptr)bme->m2m_core_regs),
				R_REG(osh, &dma64regs_xmt->ptr),
				R_REG(osh, &dma64regs_xmt->addrhigh),
				R_REG(osh, &dma64regs_xmt->addrlow),
				R_REG(osh, &dma64regs_rcv->ptr),
				R_REG(osh, &dma64regs_rcv->addrhigh),
				R_REG(osh, &dma64regs_rcv->addrlow));
			BME_STATS(BME_PRINT(" [%u %u]", bme_eng->xfers, bme_eng->loops));
			BME_STATS(BME_CTR_ZERO(bme_eng->xfers));
			BME_STATS(BME_CTR_ZERO(bme_eng->loops));
		}
	}
	BME_PRINT("\n");

} /* bme_dump() */

void /* Debug dump the BME information. Dongle specific */
hnd_cons_bme_dump(void *arg, int argc, char *argv[])
{
	osl_t *osh = (osl_t *)arg;
	bool verbose = FALSE;

	/* Usage:  dhd -i eth1 cons "bme -v" */
	if (argc >= 2) {
		/* dhd -i eth1 cons "bme -v" */
		if (strcmp(argv[1], "-v") == 0)
			verbose = TRUE;
	}

	bme_dump(osh, verbose);
}   /* hnd_cons_bme_dump() */

/**
 * ------------------------------------------------------------------------------
 * bme_init: Initialize the BME Service and add to the OS.
 * - Determine the number of Simple M2M Channels available for BME
 *   SM2M Channels re-purposed for MAC are not enlisted.
 * - Initialize and Enable each engine, defaulting to SYS user context.
 * - Reset all user contexts, ready for per user registration.
 *
 * In dongle builds, bme_init may be invoked in RTE-OS as well as in individual
 * subsystems such as bus, hwa, wl, etc. Individual subsystems invoke bme_init
 * to initialize their respective OS handler with BME support.
 * ------------------------------------------------------------------------------
 */
int /* Initialize BME service with OS */
BCMATTACHFN(bme_init)(si_t * sih, osl_t * osh)
{
	bme_t * bme;
	bme_set_t bme_set; /* bitmap of available engines */

	uint32 m2m_core_id, m2m_core_rev, bme_engines;
	volatile m2m_core_regs_t * m2m_core_regs;

	uint32 saved_core_id, saved_intr_val;
	int ret = BCME_OK;

#ifdef DONGLEBUILD
	if (g_bme != NULL) { /* non RTE-OS */
		OSH_SETBME(osh, g_bme);
		return ret;
	}
	/* RTE-OS : continue with singleton BME service initialization */
#else /* ! DONGLEBUILD */
	if (OSH_GETBME(osh) != NULL) { /* skip initialization */
		return ret;
	}
#endif /* ! DONGLEBUILD */

	ASSERT(BME_KEY_INV == BME_INVALID);

	ASSERT(OSH_GETBME(osh) == NULL);

	/* BME is part of the M2MDMA Core */
	if (si_findcoreidx(sih, M2MDMA_CORE_ID, 0) == BADIDX) {
		BME_PRINT("BME M2M core not available\n");
		return ret;
	}

	BME_PRINT("BME Service Initialization\n");

	m2m_core_regs = (volatile m2m_core_regs_t *)
		si_switch_core(sih, M2MDMA_CORE_ID, &saved_core_id, &saved_intr_val);
	ASSERT(m2m_core_regs != NULL);

	/* Take M2M core out of reset if it's not */
	if (!si_iscoreup(sih))
		si_core_reset(sih, 0, 0);

	m2m_core_id  = si_coreid(sih);
	m2m_core_rev = si_corerev(sih);

	if (m2m_core_rev < 3) {
		BME_PRINT("BME: Unavailable in M2M Core rev %d\n", m2m_core_rev);
		ret = BCME_UNSUPPORTED;
		goto done;
	}
	BME_PRINT("BME: M2M Core id 0x%x rev %u\n", m2m_core_id, m2m_core_rev);

	/* Instantiate a BME service into OSH */
	bme = (bme_t *)MALLOCZ(osh, sizeof(*bme));
	if (bme == (bme_t *)NULL) {
		BME_PRINT("BME: out of memory, malloced %d bytes\n", MALLOCED(osh));
		ret = BCME_NORESOURCE;
		goto done;
	}

	{   /* Sub-section: Prepare each BME Engine */

		uint32 v32; /* used in REG read/write */
		uint8  eng_idx; /* engine index used in iterator */
		uint32 sm2m_ch; /* simple m2m dma channel iterator */
		bool   sm2m_ch3_unavail = FALSE; /* DMA Ch3 may be re-purposed by MAC */
		bool   sm2m_ch7_unavail = FALSE; /* DMA Ch7 may be re-purposed by MAC */

		/* Determine number of Simple M2M that may be deployed for BME service */
		v32 = R_REG(osh, &m2m_core_regs->capabilities);

		BME_DEBUG(
			BME_PRINT("BME MAX[%u]: CoreCapabilities : "
				"ChannelCnt %u+1 MaxBurstLen %u "
				"MaxReadOutstanding %u SM2MCnt %u\n",
				BME_ENG_MAX,
				BCM_GBF(v32, M2M_CORE_CAPABILITIES_CHANNELCNT),
				BCM_GBF(v32, M2M_CORE_CAPABILITIES_MAXBURSTLEN),
				BCM_GBF(v32, M2M_CORE_CAPABILITIES_MAXREADSOUTSTANDING),
				BCM_GBF(v32, M2M_CORE_CAPABILITIES_SM2MCNT)));

		/* SM2MCnt: the number of Simple M2MDMA channel pairs */
		bme_engines = BCM_GBF(v32, M2M_CORE_CAPABILITIES_SM2MCNT);

		/* 43684 Bx, Cx: Channel 3 is available */
		if (m2m_core_rev == 3 || m2m_core_rev == 130) {
			sm2m_ch3_unavail = FALSE;
		} else if (m2m_core_rev >= 4) {
			sm2m_ch3_unavail = TRUE; /* ch #3 unavailable for BME service */
			ASSERT(bme_engines >= 1);
			bme_engines -= 1;

			/* 6715 */
			if (m2m_core_rev >= 131) {
				sm2m_ch7_unavail = TRUE; /* ch #7 unavailable for BME service */
				ASSERT(bme_engines >= 1);
				bme_engines -= 1;
			}
		}

		bme_set.map = 0; /* no engines enabled */

		/* Traditional M2M [ch #0, ch #1].  Register based M2M begins at ch #2 */
		sm2m_ch = 2; /* First Simple M2M DMA channel eligible for BME is ch #2 */

		for (eng_idx = 0; eng_idx < bme_engines; ++eng_idx, ++sm2m_ch) {
			bme_eng_t *bme_eng;

			/* Skip dma channel #3 if re-purposed by MAC, i.e. unavail for BME */
			if ((sm2m_ch == 3) && (sm2m_ch3_unavail == TRUE))
				sm2m_ch++;

			/* Skip dma channel #7 if re-purposed by MAC, i.e. unavail for BME */
			if ((sm2m_ch == 7) && (sm2m_ch7_unavail == TRUE))
				sm2m_ch++;

			/* Per engine BME context defaults to BME System user */
			bme_eng = &bme->bme_eng[eng_idx];
			bme_eng->bme_ctx_prev = &bme->bme_ctx[BME_USR_SYS]; /* SYS default */
			bme_eng->m2m_eng_regs = &m2m_core_regs->eng_regs[sm2m_ch];

			/* Disable all Interrupts from this channel */
			W_REG(osh, &m2m_core_regs->int_regs[sm2m_ch].intmask, 0U);

			/* Invalidate cached register values */
			bme_eng->bme_reg_cached.xmt_ptr      = BME_REG_INV;
			bme_eng->bme_reg_cached.rcv_ptr      = BME_REG_INV;
			bme_eng->bme_reg_cached.xmt_addrhigh = BME_REG_INV;
			bme_eng->bme_reg_cached.rcv_addrhigh = BME_REG_INV;

			/* XXX Some fields of XmtCtrl and RcvCtrl apply to descr processor.
			 * Use read-modify-write, to update Enable and BurstLen.
			 */
			/* Enable the transmit and receive channels of this engine */
			v32 = R_REG(osh, &bme_eng->m2m_eng_regs->tx.control);
			v32 = BCM_CBF(v32, D64_XC_BL);
			v32 |= (D64_XC_XE /* transmit channel control transmit enable */
				| BCM_SBF(BME_BURSTLEN, D64_XC_BL));
			W_REG(osh, &bme_eng->m2m_eng_regs->tx.control, v32);

			v32 = R_REG(osh, &bme_eng->m2m_eng_regs->rx.control);
			v32 = BCM_CBF(v32, D64_RC_BL);
			v32 |= (D64_RC_RE /* receive channel control receive enable */
				| BCM_SBF(BME_BURSTLEN, D64_RC_BL));
			W_REG(osh, &bme_eng->m2m_eng_regs->rx.control, v32);

			bme_set.map |= BME_SET(eng_idx); /* enlist engine into bme_set */

			BME_STATS(bme_eng->xfers = 0U);
			BME_STATS(bme_eng->loops = 0U);

		}

	}   /* Prepare each BME Engine: bme_eng[] */

	{   /* Sub-section: Prepare each USR context */

		uint8 usr_idx; /* usr index used as iterator */
		for (usr_idx = 0; usr_idx < BME_USR_MAX; ++usr_idx)
		{
			bme_ctx_t * bme_ctx = &bme->bme_ctx[usr_idx];

			bme_ctx->busy_set.map    = 0;
			bme_ctx->bme_set.map     = 0; /* no engine(s) assigned */
			bme_ctx->bme_sel         = BME_SEL_ANY;
			bme_ctx->bme_usr         = usr_idx;

			bme_ctx->bme_key.u32     = BME_KEY_INV; /* ~0U */
			bme_ctx->hi_src          = 0U;
			bme_ctx->hi_dst          = 0U;

			BME_STATS(bme_ctx->xfers = 0U);
			BME_STATS(bme_ctx->bytes = 0U);
			BME_STATS(bme_ctx->loops = 0U);
			BME_STATS(bme_ctx->error = 0U);
		}

	}   /* Prepare each USR context bme_ctx[] */

	{   /* Sub-section: Prepare System user context */
		bme_ctx_t * bme_sys = &bme->bme_ctx[BME_USR_SYS];

		bme_sys->busy_set.map         = 0;
		bme_sys->bme_set.map          = bme_set.map;
		bme_sys->bme_sel              = BME_SET_ANY;
		bme_sys->bme_usr              = BME_USR_SYS; /* redundant */

		bme_sys->bme_key.u32          = BME_KEY_SYS;
		bme_sys->hi_src               = ~0U;
		bme_sys->hi_dst               = ~0U;

		bme_sys->bme_reg.xmt_ptr      = BME_REG_INV;
		bme_sys->bme_reg.rcv_ptr      = BME_REG_INV;
		bme_sys->bme_reg.xmt_addrhigh = BME_REG_INV;
		bme_sys->bme_reg.rcv_addrhigh = BME_REG_INV;
	}

	{   /* Sub-section: Data fill rest of BME Service state */
		bme->engines       = bme_engines;   /* total number of engines */
		bme->idle_set.map  = bme_set.map;   /* all engines are idle */
		bme->m2m_core_regs = m2m_core_regs;
		bme->m2m_core_id   = m2m_core_id;
		bme->m2m_core_rev  = m2m_core_rev;
		bme->osh           = osh;
		bme->sih           = sih;
	}

#ifdef DONGLEBUILD
	/* Save bme instance */
	g_bme = bme;
#endif /* DONGLEBUILD */

	/* Register the BME service with the OS */
	OSH_SETBME(osh, bme);

done:
	si_restore_core(sih, saved_core_id, saved_intr_val);

	return ret;

}   /* bme_init() */

/**
 * ------------------------------------------------------------------------------
 * bme_deinit: De-initialize the BME Service and remove from the OS.
 * ------------------------------------------------------------------------------
 */
int /* Deinitialize BME service */
BCMATTACHFN(bme_deinit)(si_t * sih, osl_t * osh)
{
	bme_t * bme;
	uint32 saved_core_id, saved_intr_val;

	bme = OSH_GETBME(osh);

#ifdef DONGLEBUILD
	if ((g_bme == NULL) || (g_bme->osh != osh)) {
		return BCME_OK;
	}
	ASSERT(g_bme == bme);
#endif /* DONGLEBUILD */

	if (bme != NULL) {
		bme_ctx_t * bme_sys = &bme->bme_ctx[BME_USR_SYS];
		uint8  idx;

		ASSERT(bme_sys->bme_key.u32 == BME_KEY_SYS);

		/* verify all users deregistered */
		for (idx = BME_USR_SYS + 1 /* skip */; idx < BME_USR_MAX; idx++) {
			if (bme->bme_ctx[idx].bme_key.u32 != BME_KEY_INV) {
				BME_PRINT("BME ERROR: user %s didn't deregister\n",
					BME_USR_STR(idx));
				ASSERT(0);
			}
		}

		/* Disable each BME Engine */

		BME_SWITCHCORE(sih, &saved_core_id, &saved_intr_val);
		for (idx = 0; idx < BME_ENG_MAX; idx++) {
			bme_eng_t *bme_eng;

			if ((bme_sys->bme_set.map & (1 << idx)) == 0)
				continue;

			/* Per engine BME context defaults to BME System user */
			bme_eng = &bme->bme_eng[idx];

			/* Enable the transmit and receive channels of this engine */
			AND_REG(osh, &bme_eng->m2m_eng_regs->tx.control, ~D64_XC_XE);
			AND_REG(osh, &bme_eng->m2m_eng_regs->rx.control, ~D64_RC_RE);
		}
		BME_RESTORECORE(sih, saved_core_id, saved_intr_val);

		MFREE(osh, bme, sizeof(*bme));
	}
	OSH_SETBME(osh, NULL);

	return BCME_OK;

}   /* bme_deinit() */

/**
 * ------------------------------------------------------------------------------
 * bme_register_user: Register a user with the BME service
 *
 * Reservation of a BME is treated as a best effort attempt. All user contexts
 * are traversed. A reservation requested is attended by removing the engine from
 * the set of engines, when more than one engine belongs to a set (by-SET/ANY).
 *
 * Use compile time reservation, as this is best effort.
 *
 * Helper function _bme_reserve() audits whether the requested engine may be
 * indeed reserved.
 *
 * ------------------------------------------------------------------------------
 */
static int /* Register a user with reservation policy - helper routine */
_bme_reserve(bme_t * bme, uint8 eng_idx)
{
	uint8 usr_idx;
	int ret = BCME_OK;
	bme_set_t bme_set_rsv;
	bme_ctx_t * bme_ctx;

	ASSERT(eng_idx < bme->engines);

	bme_set_rsv.map = BME_SET(eng_idx);
	bme_ctx = &bme->bme_ctx[BME_USR_SYS];

	if ((bme_ctx->bme_set.map & bme_set_rsv.map) == 0) {
		BME_PRINT("BME ERROR: engine %u unavailable in SYS set 0x%x\n",
			eng_idx, bme_ctx->bme_set.map);
		ret = BCME_NORESOURCE;
	}

	/* Check if engine to be reserved is assigned to another user context */
	for (usr_idx = 0; usr_idx < BME_USR_MAX; ++usr_idx) /* iterate bme_ctx */
	{
		bme_ctx = &bme->bme_ctx[usr_idx];
		if (bme_ctx->bme_key.u32 == BME_KEY_INV)
			continue;
		if ((bme_ctx->bme_set.map ^ bme_set_rsv.map) == 0) {
			BME_PRINT("BME %u occupied by user %s sel %s set %x\n", eng_idx,
				BME_USR_STR(usr_idx), BME_SEL_STR(bme_ctx->bme_sel),
				bme_ctx->bme_set.map);
			ret = BCME_NORESOURCE;
		}
	}

	/* Remove engine from all current users (including SYS) */
	if (ret != BCME_NORESOURCE) {
		for (usr_idx = 0; usr_idx < BME_USR_MAX; ++usr_idx)
		{
			bme_ctx = &bme->bme_ctx[usr_idx];
			if (bme_ctx->bme_key.u32 == BME_KEY_INV)
				continue;
			bme_ctx->bme_set.map &= ~bme_set_rsv.map;
		}
	}

	BME_PRINT("BME: reserve eng_idx %u\n", eng_idx);

	return ret;

}   /* bme_reserve() */

/**
 * Compute BME engine registers.
 *
 * @param bme_reg	Pointer to target register set.
 * @param src_type	Source memory type, @see BME_MEM_*.
 * @param dst_type	Destination memory type, @see BME_MEM_*.
 * @param hi_src	Upper 32 bits of the 64-bit source address.
 * @param hi_dst	Upper 32 bits of the 64-bit destination address.
 */

static void
bme_compute_regs(bme_reg_t *bme_reg, bme_mem_t src_type, bme_mem_t dst_type,
	uint32 hi_src, uint32 hi_dst)
{
	/* Configure if cache coherent transactions are needed. The number of bytes to
	 * copy will be configured just before the actual transaction is started.
	 */
	bme_reg->xmt_ptr = (BME_MEM_ISCO(src_type) ? BCM_SBIT(BME_D64_XP_CO) : 0);
	bme_reg->rcv_ptr = (BME_MEM_ISCO(dst_type) ? BCM_SBIT(BME_D64_RP_CO) : 0);

	/* Adjust address MSB and enable AddrExt for PCIe if needed. */
	if (src_type == BME_MEM_PCIE) {
		if (hi_src & BME_PCI64ADDR_HIGH)
			bme_reg->xmt_ptr |= BCM_SBIT(BME_D64_XP_AE);
		hi_src |= BME_PCI64ADDR_HIGH;
	}
	if (dst_type == BME_MEM_PCIE) {
		if (hi_dst & BME_PCI64ADDR_HIGH)
			bme_reg->rcv_ptr |= BCM_SBIT(BME_D64_XP_AE);
		hi_dst |= BME_PCI64ADDR_HIGH;
	}

	/* Configure high addresses. */
	bme_reg->xmt_addrhigh = hi_src;
	bme_reg->rcv_addrhigh = hi_dst;

}   /* bme_compute_regs() */

int /* Register a user with the BME service, return -1 on error */
bme_register_user(osl_t * osh, bme_usr_t bme_usr,
         bme_sel_t bme_sel, bme_set_t bme_set,
         bme_mem_t src_type, bme_mem_t dst_type,
         uint32 hi_src, uint32 hi_dst)
{
	int ret;
	bme_t * bme;
	bme_ctx_t * bme_ctx;

	bme = (bme_t *)OSH_GETBME(osh);
	ASSERT(bme != (bme_t *)NULL);

	ASSERT(bme_usr < BME_USR_MAX);
	ASSERT(bme_sel < BME_SEL_MAX);
	ASSERT((uint8)(src_type | dst_type) <= (uint8)BME_MEM_MASK);

	bme_ctx = &bme->bme_ctx[bme_usr];

	if (bme_ctx->bme_key.u32 != BME_KEY_INV) {
		BME_PRINT("BME ERROR: reserve eng %u duplicate usr %s sel %s set %x\n",
			bme_set.idx, BME_USR_STR(bme_usr), BME_SEL_STR(bme_ctx->bme_sel),
			bme_ctx->bme_set.map);
		BME_STATS(bme_ctx->error++);
		/* Do not over-design. update stats and continue. */
	}

	/* Subsection: Check conformance of selection policy and engine set */
	switch (bme_sel) {
		case BME_SEL_RSV:
			if (_bme_reserve(bme, bme_set.idx) == BCME_NORESOURCE) {
				BME_PRINT("BME ERROR: reserve %u user %s\n",
					bme_set.idx, BME_USR_STR(bme_usr));
				BME_STATS(bme_ctx->error++);
				ASSERT(0);
				return (int)BME_KEY_INV;
			}
		case BME_SEL_IDX:
			ASSERT(bme_set.idx < bme->engines);
			bme_set.map = BME_SET(bme_set.idx); /* convert idx to map */
			break;

		case BME_SEL_SET:
		case BME_SEL_ANY:
			/* BME_USR_SYS defines engines eligible for sharing */
			bme_set.map &= bme->bme_ctx[BME_USR_SYS].bme_set.map;

			if (bme_set.map == 0) {
				BME_PRINT("BME ERROR: set alloc user %s sel %s set %x\n",
					BME_USR_STR(bme_usr), BME_SEL_STR(bme_ctx->bme_sel),
					bme_set.map);
				BME_STATS(bme_ctx->error++);
				ASSERT(0);
				return (int)BME_KEY_INV;
			}
			break;
	}

	/* Subsection: Data fill the new user's context */
	bme_ctx->busy_set.map = 0;
	bme_ctx->bme_set.map  = bme_set.map;
	bme_ctx->bme_sel      = bme_sel;
	bme_ctx->bme_usr      = bme_usr; /* redundant */

	bme_ctx->bme_key.u32 = BME_KEY(src_type, dst_type, bme_sel, bme_usr);

	bme_ctx->src_type    = src_type;
	bme_ctx->dst_type    = dst_type;
	bme_ctx->hi_src      = hi_src;
	bme_ctx->hi_dst      = hi_dst;

	/* Subsection: Precompute default register values */
	bme_compute_regs(&bme_ctx->bme_reg, src_type, dst_type, hi_src, hi_dst);

	BME_STATS(bme_ctx->xfers = bme_ctx->bytes =
	          bme_ctx->loops = bme_ctx->error = 0U);

	BME_PRINT("BME:register key<%x> user %s sel %s set %x mem %x %x hi %x %x\n",
		bme_ctx->bme_key.u32,
		BME_USR_STR(bme_ctx->bme_usr), BME_SEL_STR(bme_ctx->bme_sel),
		bme_ctx->bme_set.map,
		src_type, dst_type, bme_ctx->hi_src, bme_ctx->hi_dst);

	ret = (int)bme_ctx->bme_key.u32;
	ASSERT(ret != (int)BME_KEY_INV);

	return ret;

}   /* bme_register_user() */

int /* Deregister a user */
bme_unregister_user(osl_t * osh, bme_usr_t bme_usr)
{
	bme_t * bme;
	bme_ctx_t * bme_ctx;
	int ret = 0;

	bme = (bme_t *)OSH_GETBME(osh);
	ASSERT(bme != (bme_t *)NULL);

	ASSERT(bme_usr < BME_USR_MAX);

	/* ensure ongoing copies, if any, are finished */
	bme_sync_usr(osh, bme_usr);

	bme_ctx = &bme->bme_ctx[bme_usr];

	if (bme_ctx->bme_key.u32 == BME_KEY_INV) {
		BME_PRINT("BME ERROR: unregister invalid usr %s sel %s set %x\n",
			BME_USR_STR(bme_usr), BME_SEL_STR(bme_ctx->bme_sel),
			bme_ctx->bme_set.map);
		BME_STATS(bme_ctx->error++);
		/* Do not over-design. update stats and continue. */
		ret = BME_USR_INV;
	} else {
		/* invalidate */
		bme_ctx->bme_key.u32 = BME_KEY_INV;
	}

	return ret;
}   /* bme_unregister_user() */

int /* Get the registered user key */
bme_get_key(osl_t * osh, bme_usr_t bme_usr)
{
	bme_t * bme;
	bme_ctx_t * bme_ctx;

	ASSERT(bme_usr < BME_USR_MAX);

	bme = (bme_t *)OSH_GETBME(osh);
	ASSERT(bme != (bme_t *)NULL);

	bme_ctx = &bme->bme_ctx[bme_usr];
	return bme_ctx->bme_key.u32;

} /* bme_get_user() */

/**
 * ------------------------------------------------------------------------------
 * Poll Synchronizing for a BME to complete its outstanding transaction.
 *
 * Check whether BME engine is IDLE, by using bme::idle_set bitmap
 *
 * Precondition for a BUSY engine:
 * -------------------------------
 * When a BME user 'U' transaction starts on engine id 'E'
 * 1. bme::idle_set will have IDLE bit for bit position 'E' cleared.
 * 2. bme::bme_eng[E]::bme_ctx points to bme::bme_ctx[U]
 * 3. bme::bme_ctx[U].busy_set will have BUSY bit for bit postion 'E' set
 *
 * Synchronization Algorithm:
 * 1. Poll loop on Engine 'E' XmtPtr SB (Start/Busy), until cleared.
 * 2. Clr bme::bme_ctx[U].busy_set BUSY bit for bit position 'E'
 * 3. Set bme::idle_set IDLE bit for bit position 'E'
 *
 * Variants of Synchronization APIs:
 * - bme_sync_all: poll all engines in bme_set = ((1 << bme::engines) - 1)
 * - bme_sync_eng: poll engine specified
 * - bme_sync_set: poll engines allocated for bme_usr 'U' bme::bme_ctx[U].bme_set
 * ------------------------------------------------------------------------------
 */

static INLINE void
__bme_sync_eng(bme_t * bme, uint8 eng_idx)
{
	bme_set_t bme_set;
	bme_eng_t * bme_eng;
	bme_ctx_t * bme_ctx;
	uint32 saved_core_id, saved_intr_val;

	ASSERT(eng_idx < bme->engines);
	bme_set.map = (1 << eng_idx);

	/* Check whether BME engine is IDLE, by using bme::idle_set bitmaps */
	if (bme->idle_set.map & bme_set.map) {
		goto is_idle;
	}

	/* Assert Preconditions for BUSY engine */
	bme_eng = &bme->bme_eng[eng_idx];
	ASSERT(bme_eng->m2m_eng_regs != NULL);

	/* 1. bme::idle_set will have IDLE bit for bit position 'E' cleared. */
	// ASSERT(bme->idle_set.map & bme_set.map) checked above

	/* 2. bme::bme_eng[E]::bme_ctx points to bme::bme_ctx[U] */
	bme_ctx = bme_eng->bme_ctx_prev;
	ASSERT(bme_eng->bme_ctx_prev != &bme->bme_ctx[BME_USR_SYS]);
	ASSERT(bme_ctx->bme_key.u32 != BME_KEY_INV);

	/* 3. bme::bme_ctx[U].busy_set will have BUSY bit for bit postion 'E' set */
	ASSERT(bme_ctx->busy_set.map & bme_set.map);

	/* --- Synchronization Algorithm --- */

	BME_SWITCHCORE(bme->sih, &saved_core_id, &saved_intr_val);

	/* 1. Poll loop on Engine 'E' XmtPtr SB (Start/Busy), until cleared. */
	{	/* Poll Start/Busy bit in DMA engines Transmit channel XmtPtr */
		uint32 v32;
		BME_STATS(uint32 loops = 0U);
		do {
			v32 = R_REG(bme->osh, &bme_eng->m2m_eng_regs->tx.ptr);
			if ((v32 & BCM_MASK(BME_D64_XP_SB)) == 0U)
				break; /* DMA engine not busy, break out of loop */
			BME_STATS(loops++);
			/* Policy: CPU relax? Feed watchdog N times ...? */
		} while (1);

		BME_STATS(bme_eng->loops += loops);
		BME_STATS(bme_ctx->loops += loops);
	}

	BME_RESTORECORE(bme->sih, saved_core_id, saved_intr_val);

	/* 2. Clr bme::bme_ctx[U].busy_set BUSY bit */
	bme_ctx->busy_set.map ^= bme_set.map;

	/* 3. Set bme::idle_set IDLE bit for bit position 'E' */
	bme->idle_set.map     |= bme_set.map;

is_idle:
	/* miscellaneous designer debug/logging ... statistics high water mark */
	return;

}   /* __bme_sync_eng() */

void
bme_sync_all(osl_t * osh)
{
	uint8 eng_idx;
	bme_t * bme;
	bme_set_t idle_set;

	BME_DEBUG(BME_PRINT("BME: sync all\n"));

	ASSERT(osh != (osl_t *)NULL);
	bme = (bme_t *)OSH_GETBME(osh);

	ASSERT(bme != (bme_t *)NULL);

	if (bme->idle_set.map == (BME_SET(bme->engines) - 1))
		return; /* all BME engines are idle */

	/* __bme_sync_eng may change bme->idle_set - take local copy */
	idle_set.map = bme->idle_set.map;

	for (eng_idx = 0; eng_idx < bme->engines; ++eng_idx) {
		if (idle_set.map & (1 << eng_idx))
			continue;
		__bme_sync_eng(bme, eng_idx); /* bme_sync_eng(osh, eng_idx); */
	}

}   /* bme_sync_all() */

void
bme_sync_eng(osl_t * osh, int eng_idx)
{
	bme_t * bme;

	ASSERT(osh != (osl_t *)NULL);
	bme = (bme_t *)OSH_GETBME(osh);

	ASSERT(bme != (bme_t *)NULL);

	BME_DEBUG(BME_PRINT("BME: sync eng<%d>\n", eng_idx));

	__bme_sync_eng(bme, eng_idx);

}    /* bme_sync_eng() */

void
bme_sync_usr(osl_t * osh, bme_usr_t bme_usr)
{
	bme_t * bme;
	uint8 eng_idx;
	bme_set_t busy_set;
	bme_ctx_t * bme_ctx;

	BME_DEBUG(BME_PRINT("BME: sync user<%s>\n", BME_USR_STR(bme_usr)));

	ASSERT(osh != (osl_t *)NULL);
	bme = (bme_t *)OSH_GETBME(osh);
	ASSERT(bme != (bme_t *)NULL);

	ASSERT(bme_usr < BME_USR_MAX);
	bme_ctx = &bme->bme_ctx[bme_usr];
	ASSERT(bme_ctx->bme_key.u32 != BME_KEY_INV);

	if (bme_ctx->busy_set.map == 0)
		return;

	/* __bme_sync_eng may change bme_ctx->busy_set - take local copy */
	busy_set.map = bme_ctx->busy_set.map;

	for (eng_idx = 0; eng_idx < bme->engines; ++eng_idx) {
		if (busy_set.map & (1 << eng_idx))
			__bme_sync_eng(bme, eng_idx);
	}

}   /* bme_sync_usr() */

/**
 * ------------------------------------------------------------------------------
 * Variants of bme_copy
 *
 * ------------------------------------------------------------------------------
 */

/**
 * Configure BME engine registers.
 *
 * Write the specified values into DMA RcvPtr and [Xmt,Rcv].addrhigh. As an optimization,
 * these register values are cached in the BME engine instance and only written if needed.
 *
 * @param osh		Pointer to OS handle.
 * @param bme_eng	Pointer to BME engine handle.
 * @param reg_new	Pointer to new BME engine register values.
 */

static void
bme_configure(osl_t *osh, bme_eng_t *bme_eng, bme_reg_t *reg_new)
{
	volatile m2m_eng_regs_t *m2m_eng_regs = bme_eng->m2m_eng_regs;
	bme_reg_t *reg_cached = &bme_eng->bme_reg_cached;

	ASSERT(reg_cached->xmt_addrhigh == BME_REG_INV ||
		R_REG(osh, &m2m_eng_regs->tx.addrhigh) == reg_cached->xmt_addrhigh);

	if (reg_cached->xmt_addrhigh != reg_new->xmt_addrhigh) {
		W_REG(osh, &m2m_eng_regs->tx.addrhigh, reg_new->xmt_addrhigh);
		reg_cached->xmt_addrhigh = reg_new->xmt_addrhigh;
	}

	ASSERT(reg_cached->rcv_addrhigh == BME_REG_INV ||
		R_REG(osh, &m2m_eng_regs->rx.addrhigh) == reg_cached->rcv_addrhigh);

	if (reg_cached->rcv_addrhigh != reg_new->rcv_addrhigh) {
		W_REG(osh, &m2m_eng_regs->rx.addrhigh, reg_new->rcv_addrhigh);
		reg_cached->rcv_addrhigh = reg_new->rcv_addrhigh;
	}

	ASSERT(reg_cached->rcv_ptr == BME_REG_INV ||
		R_REG(osh, &m2m_eng_regs->rx.ptr) == reg_cached->rcv_ptr);

	if (reg_cached->rcv_ptr != reg_new->rcv_ptr) {
		W_REG(osh, &m2m_eng_regs->rx.ptr, reg_new->rcv_ptr);
		reg_cached->rcv_ptr = reg_new->rcv_ptr;
	}
}   /* bme_configure() */

static inline bme_ctx_t*
bme_get_user_ctx(bme_t *bme, uint bme_key_id)
{
	bme_key_t bme_key;
	bme_ctx_t *bme_ctx;

	ASSERT(bme != NULL);

	bme_key.u32 = (uint32)bme_key_id;
	ASSERT(bme_key.cfg.usr < BME_USR_MAX);
	bme_ctx = &bme->bme_ctx[bme_key.cfg.usr];

	ASSERT(bme_ctx->bme_key.u32 != BME_KEY_INV);
	return bme_ctx;

}   /* bme_get_user_ctx() */

/**
 * Setup a copy operation from source address to destination address.
 *
 * If reg_override is NULL, [Xmt,Rcv].addrhigh and AddrExt/Coherent flags as determined during
 * the call to @see bme_register_user are used.
 *
 * If reg_override is not NULL, [Xmt,Rcv].addrhigh and AddrExt/Coherent flags are determined
 * from that argument.
 *
 * @param osh		Pointer to OS handle.
 * @param bme_key_id	User handle as returned by @see bme_register_user.
 * @param src		Source address.
 * @param dst		Destination address.
 * @param len		Number of bytes to copy.
 * @param reg_override	Pointer to register values for use instead of default values, or NULL.
 * @return		Used engine index.
 */

static int
bme_copy_internal(osl_t *osh, uint bme_key_id, const uint32 src, uint32 dst, uint len,
	bme_reg_t *reg_override)
{
	uint8 eng_idx;
	bme_t * bme;
	bme_set_t bme_set;
	bme_eng_t * bme_eng;
	bme_ctx_t * bme_ctx_next;
	bme_reg_t * bme_reg_cur;
	uint32 saved_core_id, saved_intr_val;

	BME_DEBUG(BME_PRINT("BME: copy bme_key_id<%d> src<%p> dst<%p> len<%u> ovr<%p>\n",
		bme_key_id, (void*)(uintptr)src, (void*)(uintptr)dst, len, reg_override));

	ASSERT(osh != (osl_t *)NULL);
	bme = (bme_t *)OSH_GETBME(osh);
	ASSERT(bme != (bme_t *)NULL);

	bme_ctx_next = bme_get_user_ctx(bme, bme_key_id);

	ASSERT(len != 0 && (len & ~BME_D64_XP_BC_MASK) == 0);

	/* Check if a engine in user's eligible engines is idle, presently */
	bme_set.map = bme_ctx_next->bme_set.map & bme->idle_set.map;

	if (bme_set.map) { /* at least one idle engine eligible to user */
		for (eng_idx = 0; eng_idx < bme->engines; ++eng_idx) { /* ffs() */
			if (bme_set.map & BME_SET(eng_idx)) {   /* eligible engine */
				break;                              /* engine is idle  */
			}
		}
	} else { /* no idle engine eligible to user, so sync on a completion */
		bme_set.map = bme_ctx_next->bme_set.map;    /* user's eligible engines */
		for (eng_idx = 0; eng_idx < bme->engines; ++eng_idx) { /* ffs() */
			if (bme_set.map & BME_SET(eng_idx)) {   /* eligible engine    */
				__bme_sync_eng(bme, eng_idx);       /* sync on completion */
				break;                              /* engine is now idle */
			}
		}
	}

	ASSERT(eng_idx < bme->engines);
	ASSERT(bme_ctx_next->bme_set.map & BME_SET(eng_idx));

	bme_eng = &bme->bme_eng[eng_idx];

	/* Use either user default or overridden RcvPtr and [Xmt,Rcv].addrhigh values */
	bme_reg_cur = reg_override ? reg_override : &bme_ctx_next->bme_reg;

	BME_SWITCHCORE(bme->sih, &saved_core_id, &saved_intr_val);

	ASSERT((R_REG(osh, &bme_eng->m2m_eng_regs->tx.ptr) & BCM_SBIT(BME_D64_XP_SB)) == 0);

	/* Setup DMA engine's RcvPtr and [Xmt,Rcv].addrhigh */
	bme_configure(osh, bme_eng, bme_reg_cur);

	W_REG(osh, &bme_eng->m2m_eng_regs->rx.addrlow, dst);
	W_REG(osh, &bme_eng->m2m_eng_regs->tx.addrlow, src);

	BME_DATA_BARRIER();

	/* Kick start the DMA engine using either user default or overridden XmtPtr value */
	W_REG(osh, &bme_eng->m2m_eng_regs->tx.ptr,
		bme_reg_cur->xmt_ptr | BCM_SBF(len, BME_D64_XP_BC) | BCM_SBIT(BME_D64_XP_SB));

	BME_RESTORECORE(bme->sih, saved_core_id, saved_intr_val);

	/* Place engine into busy state */
	bme->idle_set.map &= ~(BME_SET(eng_idx));
	bme_ctx_next->busy_set.map |= BME_SET(eng_idx);
	bme_eng->bme_ctx_prev = bme_ctx_next;

	BME_STATS(bme_eng->xfers++);

	BME_STATS(bme_ctx_next->xfers++);
	BME_STATS(bme_ctx_next->bytes += len);

	return (int)eng_idx;

}   /* bme_copy_internal() */

/**
 * Copy data from source address to destination address.
 *
 * The hi_src, hi_dst, src_type and dst_type arguments specified to @see bme_register_user will be
 * used to determine the full 64-bit source and destination addresses as well as AddressExtension
 * and Coherent flags.
 *
 * This function will block if no BME engine eligible to user is available.
 *
 * @param osh		Pointer to OS handle.
 * @param bme_key_id	User handle as returned by @see bme_register_user.
 * @param src		Source pointer.
 * @param dst		Destination pointer.
 * @param len		Number of bytes to copy.
 * @return		Used engine index.
 */

int
bme_copy(osl_t * osh, uint bme_key_id, const void *src, void *dst, uint len)
{
	if (sizeof(src) == sizeof(uint32)) {
		return bme_copy_internal(osh, bme_key_id,
			(uint32)(uintptr)src, (uint32)(uintptr)dst, len, NULL);
	}

	ASSERT(sizeof(src) == sizeof(uint64));
	return bme_copy64(osh, bme_key_id, (uint64)(uintptr)src, (uint64)(uintptr)dst, len);
}   /* bme_copy() */

/**
 * Copy data from source address to destination address.
 *
 * The src_type and dst_type arguments specified to @see bme_register_user will be used to
 * determine the full 64-bit source and destination addresses as well as AddressExtension
 * and Coherent flags.
 *
 * This function will block if no BME engine eligible to user is available.
 *
 * @param osh		Pointer to OS handle.
 * @param bme_key_id	User handle as returned by @see bme_register_user.
 * @param src		Source address (64-bit).
 * @param dst		Destination address (64-bit).
 * @param len		Number of bytes to copy.
 * @return		Used engine index.
 */

int
bme_copy64(osl_t *osh, uint bme_key_id, uint64 src, uint64 dst, uint len)
{
	bme_reg_t bme_reg = { 0 };
	bme_ctx_t *bme_ctx;

	bme_ctx = bme_get_user_ctx(OSH_GETBME(osh), bme_key_id);

	/* Override user default values for [Xmt,Rcv].addrhigh and AddrExt */
	bme_compute_regs(&bme_reg, bme_ctx->src_type, bme_ctx->dst_type,
		(uint32)(src >> 32), (uint32)(dst >> 32));

	return bme_copy_internal(osh, bme_key_id,
		(uint32)src, (uint32)dst, len, &bme_reg);
}   /* bme_copy64() */
