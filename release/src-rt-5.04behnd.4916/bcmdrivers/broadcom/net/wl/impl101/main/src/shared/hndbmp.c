/**
 * +--------------------------------------------------------------------------+
 *
 *  Dongle mode Buffer Memory Pager service.
 *
 *  BMP is a RTE service and instantiated after HME, and BME are started.
 *
 *  A user application registers with the BMP service by specifying the number
 *  of buffers in the Host Buffer Memory and the Dongle Buffer Memory.
 *
 *  The Host Buffer Memory storage is allocated via the Host Memory Extension
 *  service as HME_USER_BMPSVC
 *  This HME region will include the buffer pools needed for BA64, BA256, IVTW
 *  e.g. HME region for ARQ includes a buffer pool for AMPDU RxReorder
 *  Queues for BA64 and buffer pool for AMPDU RxReorder Queues for BA256. Each
 *  buffer would include memory for the ARQ array of Host PktIds and the bitmap
 *
 *  Every buffer also includes an 8 Byte BMP header preamble, for the user
 *  application to save some subsystem context. E.g. An ARQ's buffer may save
 *  a SCB pointer or ID, a TID, and some state like expected seqnum or holes.
 *
 *  User applications track buffers using a 16bit ID, which is essentially the
 *  buffer index in the HBM pool.
 *
 *  User application must NOT have pointers into a BMP buffer, as the buffer
 *  may occupy a different DBM slot in an epoch.
 *
 *  User applications must NOT directly access a buffer in HME. A HBM buffer
 *  identified by its HBM index, must be first paired with a DBM buffer and
 *  accessed via the DBM. This also applies to the BMP buffer header preamble.
 *
 *  The notion of an epoch is used by the BMP service, with a begin, sync and
 *  end demarcation of an epoch. In the case of AMPDU RxReordering Queue user
 *  application, the start of an epoch is when RxLfrags PageIN handling starts
 *  with a CFP binning process. Each CFP bin corresponds to a list of packets
 *  destined to a SCB's TID via a lookup on <AMT index, TID>. Each time a new
 *  bin is allocated, the SCB is queried whether a ARQBMP HBM buffer had been
 *  previously allocated and a DBM buffer is paired and tagged as IN-USE into
 *  a list of active DBM buffers. If a fresh DBM buffer was used for pairing
 *  then a PageIN from HBM to DBM occurs. After all HWA Paged-IN RxLfrags are
 *  binned, all ongoing PageIN DMAs are synced before handing the RxLfrags to
 *  the WLAN stack. After the WLAN protocol stack has processed all RxLfrags
 *  an epoch is completed, allowing the BMP service to return all DBM buffers
 *  into a LRU list. WLAN protocol stack may indicate to BMP service that a
 *  DBM buffer is "clean", aside from the BMP header preamble, allowing such
 *  DBM buffers to be free for pairing in the next epoch.
 *
 *  When an User tags an HBM buffer as clear, if the HBM buffer was in the DBM
 *  cache, then it is retained in the cache. BMP tracks this clean DBM buffer
 *  in a DBM list of clear buffers.
 *  When a HBM buffer needs to be paired and the DBM Idle list is depleted,
 *  then a DBM buffer from the clear list may be evicted from the cache (BMP
 *  Header needs to be flushed to HBM) before using the DBM buffer to cache
 *  the new HBM buffer. This policy is applied across all users (i.e. this
 *  policy is not a per-user registration attribute).
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
 * $Id: hndbmp.c 831709 2023-10-25 02:34:58Z $
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * +--------------------------------------------------------------------------+
 */

#if defined(DONGLEBUILD) && defined(HNDBMP)
#include <osl.h>
#include <bcmhme.h>
#include <bcmutils.h>
#include <sbtopcie.h>

#include <hndbmp.h>

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service compile-time conditional compiles
 * +--------------------------------------------------------------------------+
 */

#define BMP_HBM_THRESH(_hbm_total)  ((_hbm_total) >> 3) /* one eight */
#define BMP_DUMP_IDX_MAX            (16)

// #define BMP_UTEST_BUILD
// #define BMP_USR_UTEST               BMP_USR_ARQW0
// #define BMP_DEBUG_BUILD

#if defined(BCMDBG)
#define BMP_AUDIT_BUILD
#define BMP_STATS_BUILD
#define BMP_RDCLR_BUILD
#endif

#define BMP_NOOP                    do { /* no-op */ } while(0)
#define BMP_PRINT                   printf
#define BMP_ERROR                   printf

#if defined(BMP_DEBUG_BUILD)
#define BMP_DEBUG(_expr_)           _expr_
#else  /* ! BMP_DEBUG_BUILD */
#define BMP_DEBUG(_expr_)           BMP_NOOP
#endif /* ! BMP_DEBUG_BUILD */

#if defined(BMP_AUDIT_BUILD)
#define BMP_ASSERT(_expr_)          ASSERT(_expr_)
#else  /* ! BMP_AUDIT_BUILD */
#define BMP_ASSERT(_expr_)          BMP_NOOP
#endif /* ! BMP_AUDIT_BUILD */

#define BMP_BPRINT(b, fmt, ...)     ((b) ? \
	bcm_bprintf(b, fmt, ##__VA_ARGS__) : \
	BMP_PRINT(fmt, ##__VA_ARGS__))

/**
 * +--------------------------------------------------------------------------+
 *  Section: Design Engineering Statistics
 * +--------------------------------------------------------------------------+
 */

#if defined(BMP_STATS_BUILD)
#define BMP_STATS(_expr_)           _expr_
#define BMP_STATS_ADD(_l, _r, _var) (_l)->stats._var += (_r)->stats._var
#else  /* ! BMP_STATS_BUILD */
#define BMP_STATS(_expr_)           BMP_NOOP
#define BMP_STATS_ADD(_l, _r, _var) BMP_NOOP
#endif /* ! BMP_STATS_BUILD */

#if defined(BMP_RDCLR_BUILD)
#define BMP_CTR_ZERO(_ctr)          (_ctr) = 0U
#else
#define BMP_CTR_ZERO(_ctr)          BMP_NOOP
#endif

//  Zero out a bmp_stats_t in user or system
#define BMP_STATS_ZERO(_p)          \
({  BMP_CTR_ZERO((_p)->stats.get);  \
	BMP_CTR_ZERO((_p)->stats.put);  \
	BMP_CTR_ZERO((_p)->stats.pgo);  \
	BMP_CTR_ZERO((_p)->stats.pgi);  \
	BMP_CTR_ZERO((_p)->stats.clr);  \
	BMP_CTR_ZERO((_p)->stats.err); /* Caution: errors also zeroed out */ \
})

//  Accumulate stats from per user into system
#define BMP_STATS_ACCUM(_l, _r)     \
({  BMP_STATS_ADD(_l, _r, get);     \
	BMP_STATS_ADD(_l, _r, put);     \
	BMP_STATS_ADD(_l, _r, pgo);     \
	BMP_STATS_ADD(_l, _r, pgi);     \
	BMP_STATS_ADD(_l, _r, clr);     \
	BMP_STATS_ADD(_l, _r, err);     \
})

//  Formatted statistics printing
#define BMP_STATS_FMT               \
	"get %u put %u pgo %u pgi %u clr %u err %u"
#define BMP_STATS_VAL(_p)           \
	(_p)->stats.get,                \
	(_p)->stats.put,                \
	(_p)->stats.pgo,                \
	(_p)->stats.pgi,                \
	(_p)->stats.clr,                \
	(_p)->stats.err

#define BMP_HDR_FMT                 "HDR 0x%08x 0x%08x"
#define BMP_HDR_VAL(_hdr)           (_hdr)->u32[0], (_hdr)->u32[1]

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service constants and macros
 * +--------------------------------------------------------------------------+
 */

//  Reserved idx value 0 is invalid. HBM index is 16bit and DBM index is 8bit
#define BMP_HBM_INV     ((uint16) (BMP_IDX_INV))
#define BMP_DBM_INV     ((uint8)  (BMP_IDX_INV))
#define BMP_BUF_NULL    ((void*)  NULL)

#define BMP_HBM_MAX     (65535) // 0xFFFF 16bit HBM index into HBM pool
#define BMP_DBM_MAX     (255)   // 0xFF    8bit DBM index into DBM pool

#define BMP_BME_MAX     (8)     // Maximum 8 BME Chn in a M2M core

typedef enum bmp_flag            // Flags used to track a BMP object
{
	BMP_FSM_IDL     = 0,        // DBM's bmp_obj is idle and free for PageIN

	BMP_FSM_PGO     = 1 << 0,   // DBM's bmp_obj is being paged out to HBM
	BMP_FSM_PGI     = 1 << 1,   // DBM's bmp_obj is being paged into
	BMP_FSM_CLR     = 1 << 2,   // DBM's bmp_obj is paired but clear
	BMP_FSM_USE     = 1 << 3,   // DBM's bmp_obj is in-use in either LRU or ACT

} bmp_flag_t;

#define BMP_FSM_POP     (BMP_FSM_PGO | BMP_FSM_PGI)

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service typedefs
 * +--------------------------------------------------------------------------+
 */

typedef struct bmp_stats        // Statistics per-user level, and, system level
{
	uint32          get;        // Count of allocations
	uint32          put;        // Count of de-allocations
	uint32          pgo;        // Count of PageOUT D2H DMA transfers
	uint32          pgi;        // Count of PageIN  H2D DMA transfers
	uint32          clr;        // Count of clears, i.e. buffer data is garbage
	uint32          err;        // Count of errors
} bmp_stats_t;

// HBM: HME region, Pool Management
typedef union bmp_hme           // HME region never crosses a 32b boundary
{
	void          * ptr;        // Low 32bit as a pointer
	uintptr         uptr;       // Used in uintptr arithmetic
	uint64          u64;        // Used in BME mem2mem programming
	haddr64_t       haddr64;    // dma64addr[lo, hi]. lo used as uintptr
} bmp_hme_t;

typedef struct bmp_hbm          // HBM Pool Management
{
	uint32          idl_cnt;    // Count of free HBM buffers in idle bmap
	uint32          clr_cnt;    // Count of clear HBM  buffer in clear bmap
	bcm_mwbmap_t  * idl_bmap;   // Multi word bitmap allocator, tracks idle idx
	bcm_mwbmap_t  * clr_bmap;   // Multi word bitmap tracks clr buffers

	bmp_hme_t       hme;        // HME region base
	uint32          hme_sz;     // HME region size
} bmp_hbm_t;

//  DBM: Finite State Machine, BMP Object Pool, and HBM to DBM Index Cache
typedef struct bmp_fsm          // FSM managing elements in one of several dll
{
	uint8           idl_cnt;    // Count of DBM's bmp_obj in IDLE  dll list
	uint8           clr_cnt;    // Count of DBM's bmp_obj in Clean dll list
	dll_t           idl_dll;    // Free list of BMP objects
	dll_t           clr_dll;    // List of cached but clean DBMs
	dll_t           lru_dll;    // Least Recently Used list of BMP objects
	dll_t           act_dll;    // Current epoch Active list of BMP objects
} bmp_fsm_t;

typedef struct bmp_obj          // An element in the DBM pool, with BMP buffer
{
	dll_t           dll;        // manage in one of several dll's in FSM
	uint8           flags;      // FSM flags, see bmp_flag_t
	uint8           dbm_idx;    // Element's index in DBM pool, self's index
	uint16          hbm_idx;    // Element's index in HBM pool

	uint8           buffer[];   // BMP assumes a bmp_hdr_t is overlayed here
} bmp_obj_t;

typedef struct bmp_dbm          // DBM pool
{
	uintptr         base;       // Base of DBM objects, dummy index 0
	uint8         * cache;      // Table listing HBM buffers cached in DBM
	bmp_fsm_t       fsm;        // Finite state machine tracking DBM elements
} bmp_dbm_t;

typedef struct bmp_usr          // USR context to include HBM and DBM
{
	bmp_hbm_t       hbm;        // HBM state
	bmp_dbm_t       dbm;        // DBM state

	uint16          buffer_sz;  // Size of each buffer
	uint16          hbm_max;    // Maximum HBM index, [1 .. hbm_max]
	uint16          hbm_thr;    // Depletion threshold crossing
	uint8           dbm_max;    // Maximum DBM index, [1 .. dbm_max]
	uint8           idx;        // Self user index
	uint32          epoch;      // Epoch counter, just for diagnostics

	bmp_cb_fn_t     cb_fn;      // User's callback function
	void          * cb_data;    // User's callback context

	void          * memory;     // Memory for DBM resources
	uint32          memory_sz;  // Size of memory for DBM resources
#if defined(BMP_STATS_BUILD)
	bmp_stats_t     stats;      // BMP user level statistics
#endif
} bmp_usr_t;

typedef struct bmp_dma
{
} bmp_dma_t;

typedef struct bmp_sys
{
	bmp_usr_t       usr[BMP_USR_TOTAL];

	int             h2d_bme;    // BME_USR_H2D bme key
	int             d2h_bme;    // BME_USR_D2H bme key

	osl_t         * osh;        // RTE osh
#if defined(BMP_STATS_BUILD)
	bmp_stats_t     stats;      // BMP service level stats
#endif
} bmp_sys_t;

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service globals
 * +--------------------------------------------------------------------------+
 */

//  Singleton, BMP service global
bmp_sys_t bmp_sys_g;

//  W0 == BA64 and W1 == BA256. BA128 uses W1's buffers.
static const char * bmp_usr_str[] = { "ARQW0", "ARQW1", "IVTW0", "IVTW1", "TAFSC",
	"TXSRB", "TXSRQ"};

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service accessors
 * +--------------------------------------------------------------------------+
 */
#define BMP_SYS()           (&bmp_sys_g)

//  Audit User index
#define BMP_AUDIT_USR_IDX(_usr_idx)                                         \
	BMP_ASSERT((_usr_idx) < BMP_USR_TOTAL)

//  Given a usr_idx, locate the bmp_usr_t
#define BMP_USR(_bmp_sys, _usr_idx)                                         \
({  bmp_usr_t * __bmp_usr;                                                  \
	BMP_AUDIT_USR_IDX(_usr_idx);                                            \
	__bmp_usr = &((_bmp_sys)->usr[_usr_idx]);                               \
	BMP_ASSERT(__bmp_usr->memory != NULL);                                  \
	__bmp_usr;                                                              \
})

//  BMP user member accessors
#define BMP_HBM(_bmp_usr)   (&((_bmp_usr)->hbm))
#define BMP_DBM(_bmp_usr)   (&((_bmp_usr)->dbm))
#define BMP_FSM(_bmp_usr)   (&((BMP_DBM(_bmp_usr))->fsm))

/**
 * Audit HBM and DBM indices to be in the range of the allocated pool.
 */

//  Audit the HBM index, ensuring that it is in the range [1, hbm_max].
#define BMP_AUDIT_HBM_IDX(_bmp_usr, _hbm_idx)                               \
({                                                                          \
	BMP_ASSERT((_hbm_idx) != BMP_HBM_INV);                                  \
	BMP_ASSERT((_hbm_idx) <= (_bmp_usr)->hbm_max);                          \
})

//  Audit DBM index, ensuring that it is in the range [1, dbm_max].
#define BMP_AUDIT_DBM_IDX(_bmp_usr, _dbm_idx)                               \
({                                                                          \
	BMP_ASSERT((_dbm_idx) != BMP_DBM_INV);                                  \
	BMP_ASSERT((_dbm_idx) <= (_bmp_usr)->dbm_max);                          \
})

/**
 *  HBM Index to DBM index cache
 */

//  Ascertain that the HBM index is not cached, i.e. paired with a DBM index.
#define BMP_AUDIT_HBM_IDX_UNCACHED(_bmp_usr, _hbm_idx)                      \
({                                                                          \
	BMP_ASSERT((BMP_DBM(_bmp_usr))->cache[_hbm_idx] == BMP_DBM_INV);        \
})

//  Access the HBM to DBM index cache, returning the DBM index, may be invalid.
#define BMP_CACHE_GET(_bmp_usr, _hbm_idx)                                   \
({  uint8 __dbm_idx;                                                        \
	BMP_AUDIT_HBM_IDX(_bmp_usr, _hbm_idx);                                  \
	__dbm_idx = (BMP_DBM(_bmp_usr))->cache[_hbm_idx];                       \
	__dbm_idx; /* DBM index may be invalid */                               \
})

//  Explicitly clear the HBM to DBM index cache, for the specified HBM index
#define BMP_CACHE_INV(_bmp_usr, _hbm_idx)                                   \
({                                                                          \
	BMP_AUDIT_HBM_IDX(_bmp_usr, _hbm_idx);                                  \
	(BMP_DBM(_bmp_usr))->cache[_hbm_idx] = BMP_DBM_INV;                     \
})

//  Setup the HBM to DBM index cache by assigning a "valid" DBM index.
#define BMP_CACHE_SET(_bmp_usr, _hbm_idx, _dbm_idx)                         \
({                                                                          \
	BMP_AUDIT_DBM_IDX(_bmp_usr, _dbm_idx);                                  \
	BMP_AUDIT_HBM_IDX_UNCACHED(_bmp_usr, _hbm_idx);                         \
	(BMP_DBM(_bmp_usr))->cache[_hbm_idx] = (_dbm_idx);                      \
})

/**
 *  Object management
 */
//  Get the size of a DBM BMP object to include the buffer.
#define BMP_OBJ_SIZE(_bmp_usr)                                              \
	(sizeof(bmp_obj_t) + ((_bmp_usr)->buffer_sz))

//  Convert DBM index to a bmp_obj_t pointer. Caller should audit DBM index.
#define BMP_OBJ(_bmp_usr, _dbm_idx)                                         \
({  bmp_obj_t * __bmp_obj;                                                  \
	BMP_AUDIT_DBM_IDX(_bmp_usr, _dbm_idx);                                  \
	__bmp_obj = ((bmp_obj_t*)((_bmp_usr)->dbm.base +                        \
	                          (BMP_OBJ_SIZE(_bmp_usr) * (_dbm_idx))));      \
	__bmp_obj;                                                              \
})

//  Zero out a DBM buffer: just BMP header or entire BMP buffer
#if defined(BMP_DEBUG)
#define BMP_DBM_ZERO(_bmp_usr, _dbm_idx)                                    \
({  void * __bmp_buf;                                                       \
	__bmp_buf = (void*)(BMP_OBJ(_bmp_usr, _dbm_idx)->buffer);               \
	memset(__bmp_buf, 0, (_bmp_usr)->buffer_sz);                            \
})
#else  /* ! BMP_DEBUG */
#define BMP_DBM_ZERO(_bmp_usr, _dbm_idx)                                    \
({  bmp_hdr_t * __bmp_hdr;                                                  \
	__bmp_hdr = (bmp_hdr_t*)(BMP_OBJ(_bmp_usr, _dbm_idx)->buffer);          \
	__bmp_hdr->u64 = 0;                                                     \
})
#endif /* ! BMP_DEBUG */

//  Convert a DBM index to the DBM buffer. Caller should audit DBM index.
#define BMP_BUF(_bmp_usr, _dbm_idx)                                         \
({  void * __bmp_buf;                                                       \
	__bmp_buf =  (void*)(BMP_OBJ(_bmp_usr, _dbm_idx)->buffer);              \
	__bmp_buf;                                                              \
})

//  Convert a bmp_obj::dll pointer to the containing bmp_obj pointer.
#define BMP_OBJ_DLL(_obj_dll)                                               \
	((bmp_obj_t*)CONTAINEROF(_obj_dll, bmp_obj_t, dll))

//  Initialize the BMP header in a DBM buffer.
#define BMP_DBM_HDR_SET(_bmp_usr, _dbm_idx, __bmp_hdr)                      \
({  bmp_hdr_t * __dbm_hdr;                                                  \
	BMP_AUDIT_DBM_IDX(_bmp_usr, _dbm_idx);                                  \
	__dbm_hdr = (bmp_hdr_t*)BMP_BUF(_bmp_usr, _dbm_idx);                    \
	__dbm_hdr->u32[0] = (__bmp_hdr)->u32[0];                                \
	__dbm_hdr->u32[1] = (__bmp_hdr)->u32[1];                                \
})

//  Given a HBM index, locate the start of the HBM buffer in HME as haddr_u64.
#define BMP_HBM_HADDR_U64(_bmp_usr, _hbm_idx)                               \
({  bmp_hme_t __bmp_hme;                                                    \
	BMP_AUDIT_HBM_IDX(_bmp_usr, _hbm_idx);                                  \
	__bmp_hme.u64 = (_bmp_usr)->hbm.hme.u64                                 \
	              + (uint64)((_bmp_usr)->buffer_sz * (_hbm_idx));		\
	__bmp_hme.u64;                                                          \
})

/**
 *  BCM MultiWord Bitmap allocator wrappers with audits
 */

//  Allocate HBM buffer from the free idle list.
#define BMP_HBM_IDL_GET(_bmp_usr, _bmp_hbm)                                 \
({  bmp_idx_t __hbm_idx;                                                    \
	BMP_ASSERT((_bmp_hbm)->idl_cnt > 0);                                    \
	__hbm_idx = bcm_mwbmap_alloc((_bmp_hbm)->idl_bmap);                     \
	BMP_AUDIT_HBM_IDX(_bmp_usr, __hbm_idx);                                 \
	BMP_AUDIT_HBM_IDX_UNCACHED(_bmp_usr, __hbm_idx);                        \
	--((_bmp_hbm)->idl_cnt);                                                \
	__hbm_idx;                                                              \
})

//  Deallocate a HBM buffer into the free idle list.
#define BMP_HBM_IDL_PUT(_bmp_usr, _bmp_hbm, _hbm_idx)                       \
({                                                                          \
	BMP_AUDIT_HBM_IDX(_bmp_usr, _hbm_idx);                                  \
	BMP_AUDIT_HBM_IDX_UNCACHED(_bmp_usr, _hbm_idx);                         \
	BMP_ASSERT(!bcm_mwbmap_isfree((_bmp_hbm)->idl_bmap, _hbm_idx));         \
	bcm_mwbmap_free((_bmp_hbm)->idl_bmap, _hbm_idx);                        \
	++((_bmp_hbm)->idl_cnt);                                                \
})

//  Allocate an HBM buffer stored in clear pool.
#define BMP_HBM_CLR_GET(_bmp_usr, _bmp_hbm)                                 \
({  bmp_idx_t __hbm_idx;                                                    \
	BMP_ASSERT((_bmp_hbm)->clr_cnt > 0);                                    \
	__hbm_idx = bcm_mwbmap_alloc((_bmp_hbm)->clr_bmap);                     \
	BMP_AUDIT_HBM_IDX(_bmp_usr, __hbm_idx);                                 \
	BMP_AUDIT_HBM_IDX_UNCACHED(_bmp_usr, __hbm_idx);                        \
	BMP_ASSERT(!bcm_mwbmap_isfree((_bmp_hbm)->idl_bmap, __hbm_idx));        \
	--((_bmp_hbm)->clr_cnt);                                                \
	__hbm_idx;                                                              \
})

//  Store an allocated HBM buffer into pool of clear buffers.
#define BMP_HBM_CLR_PUT(_bmp_usr, _bmp_hbm, _hbm_idx)                       \
({                                                                          \
	BMP_AUDIT_HBM_IDX(_bmp_usr, _hbm_idx);                                  \
	BMP_ASSERT(!bcm_mwbmap_isfree((_bmp_hbm)->idl_bmap, _hbm_idx));         \
	BMP_ASSERT(!bcm_mwbmap_isfree((_bmp_hbm)->clr_bmap, _hbm_idx));         \
	bcm_mwbmap_free((_bmp_hbm)->clr_bmap, _hbm_idx);                        \
	++((_bmp_hbm)->clr_cnt);                                                \
})

//  Check and remove a specified HBM buffer from the pool of clear HBM buffers.
#define BMP_HBM_CLR_REM(_bmp_usr, _bmp_hbm, _hbm_idx)                       \
({                                                                          \
	bool _is_clr = FALSE;                                                   \
	if (bcm_mwbmap_isfree((_bmp_hbm)->clr_bmap, _hbm_idx)) {                \
		BMP_ASSERT(!bcm_mwbmap_isfree((_bmp_hbm)->idl_bmap, _hbm_idx));     \
		bcm_mwbmap_force((_bmp_hbm)->clr_bmap, hbm_idx);                    \
		--((_bmp_hbm)->clr_cnt);                                            \
		_is_clr = TRUE;                                                     \
	}                                                                       \
	_is_clr;                                                                \
})

// +--------------------------------------------------------------------------+
//  Section: BMP service non-exported helper routines
// +--------------------------------------------------------------------------+

//  Access the HBM buffer's BMP header using SBTOPCIE programmed IO
static void     _bmp_hdr(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr,
                         bmp_idx_t hbm_idx, bmp_hdr_t * bmp_hdr,
                         uint32 sbtopcie_dir);

//  Page OP: PageOUT to HBM or PageIN from HBM
static int      _bmp_pop(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr,
                         bmp_obj_t * bmp_obj, bmp_dir_t bmp_dir);

//  Attach a DBM buffer, no PageIN of HBM buffer performed during pairing
static uint8    _bmp_map(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr,
                         bmp_idx_t   hbm_idx);

static void     _bmp_unm(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr,
                         bmp_obj_t * bmp_obj);

/**
 *  Access the BMP header preamble in the HBM buffer using SBTOPCIE
 *  HBM Read or Write access is performed using sbtopcie_dir parameter
 *  SBTOPCIE_DIR_H2D or SBTOPCIE_DIR_D2H, respectively.
 */
void
_bmp_hdr(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr,
         bmp_idx_t   hbm_idx, bmp_hdr_t * bmp_hdr,
         uint32      sbtopcie_dir)
{
	uintptr     daddr_uptr  = (uintptr)bmp_hdr;
	uint64      haddr_u64   = BMP_HBM_HADDR_U64(bmp_usr, hbm_idx);
	size_t      hdr_size    = sizeof(bmp_hdr_t); // single uint64 access

	sbtopcie_pio(daddr_uptr, haddr_u64, hdr_size, sbtopcie_dir);

}   // _bmp_hdr()

/**
 *  PageIN or PageOUT a DBM buffer, to include the BMP header preamble.
 *
 *  Given a BMP object, the source DBM and destination HBM buffers are located
 *  using their corresponding buffer index. A BME copy is issued.
 *
 *  Caller is required to sync for DMA completion before using the DBM buffer.
 */
int
_bmp_pop(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr,
         bmp_obj_t * bmp_obj, bmp_dir_t bmp_dir)
{
	bmp_hbm_t * bmp_hbm;
	uint8       dbm_idx;
	uint16      hbm_idx;

	uint32      buf_len;
	uint64      daddr_u64;                              // Dongle SysMem
	uint64      haddr_u64;                              // HME addresses
	int         bme_eng;

	bmp_hbm     = BMP_HBM(bmp_usr);
	dbm_idx     = bmp_obj->dbm_idx;
	hbm_idx     = bmp_obj->hbm_idx;

	BCM_REFERENCE(dbm_idx);
	BMP_AUDIT_DBM_IDX(bmp_usr, dbm_idx);
	BMP_AUDIT_HBM_IDX(bmp_usr, hbm_idx);

	buf_len     = (uint32)bmp_usr->buffer_sz;           // transfer length
	daddr_u64   = (uint64)((uintptr)bmp_obj->buffer);
	haddr_u64   = bmp_hbm->hme.u64 + (hbm_idx * buf_len);

	// Invoke the BME transfer
	if (bmp_dir == bmp_dir_d2h)
	{
		bme_eng = bme_copy64(bmp_sys->osh, bmp_sys->d2h_bme,
		                     daddr_u64, haddr_u64, buf_len);
		bmp_obj->flags |= BMP_FSM_PGO;                 // PageOUT in progress

		BMP_STATS(bmp_usr->stats.pgo++);
		BMP_DEBUG(BMP_PRINT("BMP: %s PGO dbm %3u hbm %3u\n",
		                    bmp_usr_str[bmp_usr->idx], dbm_idx, hbm_idx));
	}
	else /* bmp_dir == bmp_dir_h2d */
	{
		bme_eng = bme_copy64(bmp_sys->osh, bmp_sys->h2d_bme,
		                     haddr_u64, daddr_u64, buf_len);
		bmp_obj->flags |= BMP_FSM_PGI;                  // PageIN  in progress

		BMP_STATS(bmp_usr->stats.pgi++);
		BMP_DEBUG(BMP_PRINT("BMP: %s PGI hbm %3u> dbm %3u\n",
		                    bmp_usr_str[bmp_usr->idx], hbm_idx, dbm_idx));
	}

	// DMA not synced, sync using returned bme_eng
	return bme_eng;                                     // BME engine's index

}   // _bmp_pop()

/**
 *  Given an HBM buffer index, allocate a DBM buffer and pair them.
 *
 *  DBM's FSM checks for a idle BMP object in the Idle List. If Idle list is
 *  empty then the LRU list is checked next. If the LRU list is not empty
 *  then the head BMP object is first evicted, by flushing the buffer to
 *  its HBM buffer if needed, un-pairing the BMP object from the previous HBM
 *  buffer. The BMP object allocated from the IDL or LRU list is paired with
 *  the new HBM buffer using the DBM's cache table and tagged as in use.
 *
 *  No explicit PageIN is performed from HBM to DBM, to include the BMP header.
 *
 *  Returns: dbm_idx or BMP_DBM_INV
 */
uint8
_bmp_map(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr, bmp_idx_t hbm_idx)
{
	bmp_fsm_t * bmp_fsm;
	int         bme_eng;

	dll_t     * obj_dll;
	bmp_obj_t * bmp_obj;
	bmp_hdr_t * bmp_hdr;
	uint8       dbm_idx;

	bmp_fsm     = BMP_FSM(bmp_usr);
	bme_eng     = BCME_ERROR;

	BMP_AUDIT_HBM_IDX_UNCACHED(bmp_usr, hbm_idx);

	// Attempt DBM buffer allocation from IDL list or LRU list.
	// DBM buffers allocated from LRU list, need a PGO if not tagged as clear.

	if (bmp_fsm->idl_cnt)                               // Non empty idle list
	{
		obj_dll = dll_head_p(&bmp_fsm->idl_dll);        // Get an idle DBM buf
		bmp_obj = BMP_OBJ_DLL(obj_dll);                 // Container BMP object
		BMP_ASSERT(bmp_obj->flags   == BMP_FSM_IDL);    // FSM IDLE audit
		BMP_ASSERT(bmp_obj->hbm_idx == BMP_HBM_INV);    // Audit not HBM paired
		--bmp_fsm->idl_cnt;                             // DLL deletion later
	}
	else if (bmp_fsm->clr_cnt)                          // Non empty clear list
	{
		obj_dll = dll_head_p(&bmp_fsm->clr_dll);        // Get an clear DBM buf
		bmp_obj = BMP_OBJ_DLL(obj_dll);                 // Container BMP object
		BMP_ASSERT(bmp_obj->flags & BMP_FSM_CLR);       // FSM Clear audit
		BMP_ASSERT(bmp_obj->hbm_idx != BMP_HBM_INV);    // Audit not HBM paired
		--bmp_fsm->clr_cnt;                             // DLL deletion later

		// Flush the DBM's bmp_hdr to HBM before unmapping DBM
		bmp_hdr = (bmp_hdr_t*)bmp_obj->buffer;          // DBM bmp_hdr to flush
		_bmp_hdr(bmp_sys, bmp_usr, bmp_obj->hbm_idx, bmp_hdr, SBTOPCIE_DIR_D2H);
		BMP_CACHE_INV(bmp_usr, bmp_obj->hbm_idx);       // Invalidate DBM cache
		bmp_obj->hbm_idx = BMP_HBM_INV;
	}
	else if (!dll_empty(&bmp_fsm->lru_dll))             // Non empty LRU list
	{
		obj_dll = dll_head_p(&bmp_fsm->lru_dll);        // Get an LRU DBM buf
		bmp_obj = BMP_OBJ_DLL(obj_dll);                 // Container BMP object
		BMP_ASSERT(!(bmp_obj->flags & (BMP_FSM_PGI | BMP_FSM_PGO)));

		/* Inform user to unlink this DBM buffer as it is being re-allocated */
		bmp_hdr = (bmp_hdr_t*)bmp_obj->buffer;
		bmp_usr->cb_fn(bmp_usr->cb_data, BMP_CB_DBM_UNLINK, bmp_obj->hbm_idx, bmp_hdr);

		bme_eng = _bmp_pop(bmp_sys, bmp_usr,
		                   bmp_obj, bmp_dir_d2h);       // Evict DBM: PageOUT
		BMP_CACHE_INV(bmp_usr, bmp_obj->hbm_idx);       // Detach previous HBM
		bmp_obj->hbm_idx = BMP_HBM_INV;
	} else {
		goto _bmp_map_failure;                          // All DBM bufs in use
	}

	dbm_idx          = bmp_obj->dbm_idx;
	BMP_CACHE_SET(bmp_usr, hbm_idx, dbm_idx);           // Setup H2D pairing
	bmp_obj->hbm_idx = hbm_idx;
	bmp_obj->flags   = BMP_FSM_USE;                     // Tag DBM buf "in use"

	dll_delete(obj_dll);                                // Delete from src dll
	dll_prepend(&bmp_fsm->act_dll, obj_dll);            // Prepend into act dll

	BMP_STATS(bmp_usr->stats.get++);
	BMP_DEBUG(BMP_PRINT("BMP: %s MAP hbm %3u dbm %3u\n",
	                    bmp_usr_str[bmp_usr->idx], hbm_idx, dbm_idx));

	if (bme_eng != BCME_ERROR) {                        // Sync DBM PGO
		bme_sync_eng(bmp_sys->osh, bme_eng);
	}

	BMP_DBM_ZERO(bmp_usr, dbm_idx);                     // Zero out BMP header

	return dbm_idx;                                     // Paired DBM buf index

_bmp_map_failure:

	BMP_STATS(bmp_usr->stats.err++);
	BMP_ERROR("BMP: %s MAP hbm %3u: fatal dbm no resource\n",
	          bmp_usr_str[bmp_usr->idx], hbm_idx);

	return BMP_DBM_INV;

}   // _bmp_map()

void
_bmp_unm(bmp_sys_t * bmp_sys, bmp_usr_t * bmp_usr, bmp_obj_t * bmp_obj)
{
	uint8       dbm_idx = bmp_obj->dbm_idx;
	uint16      hbm_idx = bmp_obj->hbm_idx;
	bmp_fsm_t * bmp_fsm = BMP_FSM(bmp_usr);

	if (bmp_obj->flags & BMP_FSM_CLR) {
		--bmp_fsm->clr_cnt;                             // Delete clr dll below
	}
	BMP_DBM_ZERO(bmp_usr, dbm_idx);                     // Unlink DBM, zero out
	bmp_obj->hbm_idx    = BMP_HBM_INV;
	bmp_obj->flags      = BMP_FSM_IDL;                  // DBM is idle
	BMP_CACHE_INV(bmp_usr, hbm_idx);                    // Un-cache from DBM

	dll_delete(&bmp_obj->dll);                          // either: clr lru act
	dll_prepend(&bmp_fsm->idl_dll, &bmp_obj->dll);      // Move to idle list
	++bmp_fsm->idl_cnt;                                 // Return DBM to Idle

}   // _bmp_unm()

// +--------------------------------------------------------------------------+
//  Section: BMP object management APIs
//  All BMP object management APIs return a 16bit HBM Idx or BMP_IDX_ERR
// +--------------------------------------------------------------------------+

/**
 *  Given a HBM buffer index, fetch the paired DBM buffer object and return the
 *  buffer pointer.
 *  - In internal builds, audit that the HBM buffer is tracked as allocated.
 *  - If the HBM buffer is not paired with a DBM buffer, pair with a DBM buffer
 */
void *
bmp_buf(uint32 usr_idx, bmp_idx_t hbm_idx)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_obj_t * bmp_obj;
	bmp_hbm_t * bmp_hbm;
	bmp_fsm_t * bmp_fsm;
	uint8       dbm_idx;
	void      * buffer;

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);
	bmp_hbm     = BMP_HBM(bmp_usr);
	bmp_fsm     = BMP_FSM(bmp_usr);

	dbm_idx     = BMP_CACHE_GET(bmp_usr, hbm_idx);      // lkup hbm2dbm cache

	if (dbm_idx == BMP_DBM_INV)                         // Not cached in DBM
	{
		int         bme_eng;

		// Allocate a DBM buffer and setup Cache map.
		// A buffer allocated from LRU list is evicted(PageOUT) first.
		dbm_idx = _bmp_map(bmp_sys, bmp_usr, hbm_idx);  // Map HBM to DBM
		if (dbm_idx == BMP_DBM_INV)                     // Pairing failed
		{
			BMP_STATS(bmp_usr->stats.err++);
			BMP_ERROR("BMP: %s BUF hbm %3u: fatal DBM no resource\n",
			          bmp_usr_str[bmp_usr->idx], hbm_idx);
			return NULL;
		}
		bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);

		if (BMP_HBM_CLR_REM(bmp_usr, bmp_hbm, hbm_idx)) {      // Remove from Clr pool
			// If clear, PageIN only BMP header
			_bmp_hdr(bmp_sys, bmp_usr, hbm_idx,
			         (bmp_hdr_t*)bmp_obj->buffer, SBTOPCIE_DIR_H2D);
		} else {
			// PageIN full HBM buf to DBM
			bme_eng = _bmp_pop(bmp_sys, bmp_usr, bmp_obj, bmp_dir_h2d);
			bme_sync_eng(bmp_sys->osh, bme_eng);            // Sync on DMA done
		}
	}
	else	// Move to active list from clr or lru or act
	{
		bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);

		ASSERT(bmp_obj->hbm_idx == hbm_idx);
		if (BMP_HBM_CLR_REM(bmp_usr, bmp_hbm, hbm_idx)) {      // Remove from Clr pool
			ASSERT(bmp_obj->flags & BMP_FSM_CLR);
			--bmp_fsm->clr_cnt;                     // Delete clr dll below
		}

		dll_delete(&bmp_obj->dll);                      // either: clr lru act
		dll_prepend(&bmp_fsm->act_dll, &bmp_obj->dll);  // Prepend into act dll
	}

	bmp_obj->flags   = BMP_FSM_USE;                 // Tag DBM buf "in use"

	buffer      = BMP_BUF(bmp_usr, dbm_idx);            // Locate buffer
	BMP_DEBUG(BMP_PRINT("BMP: %s BUF hbm %3u dbm %3u 0x%p\n",
	                    bmp_usr_str[usr_idx], hbm_idx, dbm_idx, buffer));
	return buffer;

}   // bmp_buf()

/**
 *  Allocate a "new" HBM buffer.
 *
 *  If there are free IDLE HBM buffers, allocate new HBM buffer from IDLE list.
 *  If there are no free IDLE HBM buffers, then the list of garbage HBM
 *  buffers is used to fetch a previously allocated HBM buffer. In order to
 *  reuse this HBM buffer from the CLEAR list, the buffer needs to be unlinked
 *  from the User context. The BMP header of this clear HBM buffer is fetched
 *  using a SBTOPCIE H2D programmed IO access and the User is informed via a
 *  BMP_CB_HBM_UNLINK callback. User dettaches the reference to the previously
 *  allocated HBM buffer, using the BMP header. NOTE: The BMP header in the HBM
 *  buffer is explicitly zeroed out before invoking the BMP_CB_HBM_UNLINK.
 *
 *  NOTE: the HBM buffer is not paired with a DBM buffer.
 *
 *  The HBM buffer's index is returned to the User and is placed in clear pool.
 *
 *  A failure to allocate a HBM buffer is reported as a fatal error. User
 *  applications need to review the HBM pool engineering.
 */
bmp_idx_t
bmp_get(uint32 usr_idx, bmp_hdr_t * usr_hdr)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_hbm_t * bmp_hbm;
	bmp_idx_t   hbm_idx;
	uint8       dbm_idx;

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);
	bmp_hbm     = BMP_HBM(bmp_usr);
	dbm_idx     = BMP_DBM_INV;

	if (bmp_hbm->idl_cnt)                               // Check idle bmap
	{
		hbm_idx = BMP_HBM_IDL_GET(bmp_usr, bmp_hbm);    // Allocate an idle
	}
	else if (bmp_hbm->clr_cnt)                          // Check used but clear
	{
		bmp_obj_t * bmp_obj;
		bmp_hdr_t   bmp_hdr;
		hbm_idx = BMP_HBM_CLR_GET(bmp_usr, bmp_hbm);    // Reuse a cleared HBM

		dbm_idx = BMP_CACHE_GET(bmp_usr, hbm_idx);
		if (dbm_idx != BMP_DBM_INV)
		{
			// Use BMP Header from DBM, and Unmap DBM
			bmp_obj     = BMP_OBJ(bmp_usr, dbm_idx);
			bmp_hdr.u64 = ((bmp_hdr_t*)(bmp_obj->buffer))->u64;
			_bmp_unm(bmp_sys, bmp_usr, bmp_obj);
		} else {
			// Fetch the HBM buffer's BMP header preamble
			_bmp_hdr(bmp_sys, bmp_usr, hbm_idx, &bmp_hdr, SBTOPCIE_DIR_H2D);
		}

		/*
		 * Inform user to unlink this HBM buffer as it is being re-allocated
		 * NOTE: As the HBM buffer was previously tagged as clean, user may not
		 * attempt to cache and access it within the callback!
		 */
		bmp_usr->cb_fn(bmp_usr->cb_data, BMP_CB_HBM_UNLINK, hbm_idx, &bmp_hdr);
	}
	else
	{
		hbm_idx     = BMP_IDX_INV;
		BMP_STATS(bmp_usr->stats.err++);
		BMP_ERROR("BMP: %s GET : fatal hbm no resource\n", bmp_usr_str[usr_idx]);
		goto bmp_get_failure;
	}

	// Unless necessary, no threshold check with callback of low HBM resource

	BMP_HBM_CLR_PUT(bmp_usr, bmp_hbm, hbm_idx);         // Store in Clear pool
	BMP_STATS(bmp_usr->stats.get++);

	if (usr_hdr) {                                      // Reverse link
		_bmp_hdr(bmp_sys, bmp_usr, hbm_idx, usr_hdr, SBTOPCIE_DIR_D2H);
		BMP_DEBUG(BMP_PRINT("BMP: %s GET hbm %3u " BMP_HDR_FMT "\n",
		          bmp_usr_str[usr_idx], hbm_idx, BMP_HDR_VAL(usr_hdr)));
	} else {
		BMP_DEBUG(BMP_PRINT("BMP: %s GET hbm %3u\n",
		          bmp_usr_str[usr_idx], hbm_idx));
	}

bmp_get_failure:
	return hbm_idx; // HBM index may be invalid.

}   // bmp_get()

/**
 *  Deallocate a previously allocated HBM buffer.
 *
 *  Determine if the HBM buffer is cached in DBM and free DBM buffer if cached.
 *  If the HBM existed in the clear pool, remove it. Deallocate the HBM buffer.
 *
 *  The BMP header in the HBM buffer is explicitly cleared in BMP Debug build,
 *  using a SBTOPCIE PIO in D2H direction.
 */
void
bmp_put(uint32 usr_idx, bmp_idx_t hbm_idx)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_hbm_t * bmp_hbm;
	uint8       dbm_idx;

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);
	bmp_hbm     = BMP_HBM(bmp_usr);

	// If HBM buffer is cached in DBM, uncache by freeing DBM buffer
	dbm_idx     = BMP_CACHE_GET(bmp_usr, hbm_idx);      // Check whether cached
	if (dbm_idx != BMP_DBM_INV)                         // If cached in DBM
	{
		bmp_fsm_t * bmp_fsm = BMP_FSM(bmp_usr);
		bmp_obj_t * bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);

		if (bmp_obj->flags & BMP_FSM_CLR) {
			--bmp_fsm->clr_cnt;                         // Delete clr dll below
		}
		BMP_DBM_ZERO(bmp_usr, dbm_idx);                 // Unlink DBM, zero out
		BMP_ASSERT(hbm_idx == bmp_obj->hbm_idx);
		bmp_obj->hbm_idx    = BMP_HBM_INV;
		dll_delete(&bmp_obj->dll);                      // either: clr lru act
		dll_prepend(&bmp_fsm->idl_dll, &bmp_obj->dll);  // Move to idle list
		++bmp_fsm->idl_cnt;                             // Return DBM to Idle
		BMP_CACHE_INV(bmp_usr, hbm_idx);                // Un-cache from DBM
		bmp_obj->flags      = BMP_FSM_IDL;              // DBM is idle
	}

#if defined(BMP_DEBUG)
	{
		bmp_hdr_t   hbm_hdr;
		hbm_hdr.u64 = 0;                                // Unlink HBM buffer
		_bmp_hdr(bmp_sys, bmp_usr, hbm_idx, &hbm_hdr,
		         SBTOPCIE_DIR_D2H);                     // PCIE Write to HBM
	}
#endif /* BMP_DEBUG */

	// Free HBM index, by removing from CLR bmap and return to Idle bmap
	BMP_HBM_CLR_REM(bmp_usr, bmp_hbm, hbm_idx);         // Remove from Clr pool
	BMP_HBM_IDL_PUT(bmp_usr, bmp_hbm, hbm_idx);         // Deallocate to Idle

	BMP_STATS(bmp_usr->stats.put++);
	BMP_DEBUG(BMP_PRINT("BMP: %s PUT hbm %3u dbm %3u\n",
	                    bmp_usr_str[usr_idx], hbm_idx, dbm_idx));
}   // bmp_put()

/**
 *  Setup the BMP header preamble, to link buffer to user context
 *
 *  If the HBM buffer is paired with a DBM buffer, then the BMP header is only
 *  updated in the local cached DBM buffer, else, an explicit programmed IO
 *  access over PCIe is performed to update in HBM.
 */
void
bmp_set(uint32 usr_idx, bmp_idx_t hbm_idx, bmp_hdr_t * bmp_hdr)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	uint8       dbm_idx;

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);

	dbm_idx     = BMP_CACHE_GET(bmp_usr, hbm_idx);      // lkup hbm2dbm cache

	if (dbm_idx != BMP_DBM_INV) {                       // Cached in DBM
		BMP_DBM_HDR_SET(bmp_usr, dbm_idx, bmp_hdr);     // Update HDR in DBM
	} else {
		_bmp_hdr(bmp_sys, bmp_usr, hbm_idx, bmp_hdr,
		         SBTOPCIE_DIR_D2H);                     // PCIE Write to HBM
	}

	BMP_DEBUG(BMP_PRINT("BMP: %s SET hbm %3u " BMP_HDR_FMT "\n",
	                    bmp_usr_str[usr_idx], hbm_idx, BMP_HDR_VAL(bmp_hdr)));
}   // bmp_set()

/**
 *  User informs BMP that a HBM buffer is clear, i.e. it does not contain any
 *  useful data other than the BMP header reverse link. BMP service may uncache
 *  the DBM buffer and place the HBM buffer in the clear list. HBM buffers in
 *  the clear list may be re-used if the HBM idle list is empty.
 */
void
bmp_clr(uint32 usr_idx, bmp_idx_t hbm_idx)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_hbm_t * bmp_hbm;
	uint8       dbm_idx;

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);
	bmp_hbm     = BMP_HBM(bmp_usr);

	dbm_idx     = BMP_CACHE_GET(bmp_usr, hbm_idx);      // Check whether cached
	if (dbm_idx != BMP_DBM_INV)
	{
		bmp_fsm_t * bmp_fsm = BMP_FSM(bmp_usr);
		bmp_obj_t * bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);

		if (bmp_obj->flags & BMP_FSM_CLR)
			return;

		dll_delete(&bmp_obj->dll);                      // lru_dll or act_dll
		dll_prepend(&bmp_fsm->clr_dll, &bmp_obj->dll);  // Move to Clear list
		++bmp_fsm->clr_cnt;                             // Return DBM to Clear
		bmp_obj->flags      = BMP_FSM_CLR;
	}

	BMP_HBM_CLR_PUT(bmp_usr, bmp_hbm, hbm_idx);         // Store in Clear pool
	BMP_STATS(bmp_usr->stats.clr++);

}   // bmp_clr()

/**
 *  Perform a Pager OP, either a PageOUT or a PageIN
 */
void
bmp_pop(uint32 usr_idx, bmp_idx_t hbm_idx, bmp_dir_t bmp_dir)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	uint8       dbm_idx;

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);

	dbm_idx     = BMP_CACHE_GET(bmp_usr, hbm_idx);      // lkup hbm2dbm cache

	BMP_DEBUG(BMP_PRINT("BMP: %s %s hbm %3u dbm %3u\n", bmp_usr_str[usr_idx],
	                    (bmp_dir == bmp_dir_d2h) ? "PGO" : "PGI",
	                    hbm_idx, dbm_idx));

	if (dbm_idx == BMP_DBM_INV)
	{
		void      * buffer;
		if (bmp_dir == bmp_dir_d2h)
			return;                                     // No DBM to PageOUT
		buffer      = bmp_buf(usr_idx, hbm_idx);        // Maps and PageIN
		if (buffer  == NULL) {
			BMP_ERROR("BMP: %s PGI hbm %3u : fatal DBM no resource\n",
			          bmp_usr_str[bmp_usr->idx], hbm_idx);
		}
	}
	else
	{
		int         bme_eng;
		bmp_obj_t * bmp_obj;
		if (bmp_dir == bmp_dir_h2d)
			return;                                     // HBM buffer is stale
		bmp_obj     = BMP_OBJ(bmp_usr, dbm_idx);
		if (bmp_obj->flags & BMP_FSM_CLR) {             // PageOUT BMP Header
			_bmp_hdr(bmp_sys, bmp_usr, hbm_idx,
			         (bmp_hdr_t*)bmp_obj->buffer, SBTOPCIE_DIR_D2H);
			return;
		}
		bme_eng     = _bmp_pop(bmp_sys, bmp_usr, bmp_obj, bmp_dir_d2h);
		bme_sync_eng(bmp_sys->osh, bme_eng);            // PageOUT DBM buffer
	}

}   // bmp_pop()

/**
 *  Access the run-time HBM usage statistics in terms of number of idle HBM
 *  buffers available for allocation and number of HBM buffers that are
 *  allocated but do not contain any valid data, i.e. they have been cleared.
 */
void
bmp_cnt(uint32 usr_idx, uint32 * dbm_idl_cnt,
                        uint32 * hbm_idl_cnt, uint32 * hbm_clr_cnt)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_hbm_t * bmp_hbm;
	bmp_fsm_t * bmp_fsm;

	BMP_ASSERT((dbm_idl_cnt != NULL) &&
	           (hbm_idl_cnt != NULL) && (hbm_clr_cnt != NULL));

	bmp_sys     = BMP_SYS();
	bmp_usr     = BMP_USR(bmp_sys, usr_idx);
	bmp_hbm     = BMP_HBM(bmp_usr);
	bmp_fsm     = BMP_FSM(bmp_usr);

	*dbm_idl_cnt = (uint32)bmp_fsm->idl_cnt;
	*hbm_idl_cnt = (uint32)bmp_hbm->idl_cnt;
	*hbm_clr_cnt = (uint32)bmp_hbm->clr_cnt;

}   // bmp_cnt()

// +--------------------------------------------------------------------------+
//  BMP FSM transitions APIs : Commence or Terminate a BMP epoch
// +--------------------------------------------------------------------------+

/**
 *  Begin an epoch pre WLAN RxProcess - start of CFP binning
 */
void
bmp_bgn(uint32 usr_idx)
{
	bmp_sys_t * bmp_sys = BMP_SYS();
	bmp_usr_t * bmp_usr = &bmp_sys->usr[usr_idx];       // Must be registered
	BMP_ASSERT(dll_empty(&(BMP_FSM(bmp_usr)->act_dll)));
	++bmp_usr->epoch;
}   // bmp_bgn()

/**
 *  End an epoch post WLAN RxProcess - end of RxCpl handoffs to Bus layer
 */
void
bmp_end(uint32 usr_idx)
{
	bmp_sys_t * bmp_sys = BMP_SYS();
	bmp_usr_t * bmp_usr = &bmp_sys->usr[usr_idx];       // Must be registered
	bmp_hbm_t * bmp_hbm = BMP_HBM(bmp_usr);
	bmp_fsm_t * bmp_fsm = BMP_FSM(bmp_usr);
	uint16      hbm_thr;

	if (!dll_empty(&bmp_fsm->act_dll)) {
		dll_join(&bmp_fsm->act_dll, &bmp_fsm->lru_dll); // Move all to lru_dll
	}

	// Inform user application that allocatable HBM resource is low.
	hbm_thr     = (uint16)(bmp_hbm->idl_cnt + bmp_hbm->clr_cnt);
	if (hbm_thr <= bmp_usr->hbm_thr) {
		bmp_usr->cb_fn(bmp_usr->cb_data, BMP_CB_HBM_THRESH, BMP_HBM_INV, NULL);
	}

}   // bmp_end()

/**
 *  Sync DMA completions of all ongoing PageOUTs or PageINs
 */
void
bmp_sync(void)
{
	bmp_sys_t * bmp_sys = BMP_SYS();
	bme_sync_all(bmp_sys->osh);
}   // bmp_sync()

// +--------------------------------------------------------------------------+
//  Section: BMP service debug and audit APIs
// +--------------------------------------------------------------------------+

/**
 *  Audit the BMP service state for a given service user.
 *  - HBM allocation: Traverse the idle bmap and audit idle count
 *  - DBM pairing: Traverse hbm2dbm cache and audit the paired DBM's FSM flags
 */
#define __ASSERT(exp) \
	do { if (!(exp)) printf("=== XSSERT ===: %u %s\n", __LINE__, #exp); } while (0)

void
bmp_audit_usr(uint32 usr_idx)
{
#if defined(BCMDBG) || defined(BMP_DEBUG_BUILD)
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_hbm_t * bmp_hbm;
	bmp_dbm_t * bmp_dbm;
	bmp_fsm_t * bmp_fsm;
	bmp_obj_t * bmp_obj;
	uint16      hbm_idx;
	uint8       dbm_idx;
	uint32      idl_cnt, clr_cnt, use_cnt;

	bmp_sys     = BMP_SYS();
	bmp_usr     = &bmp_sys->usr[usr_idx];
	if (bmp_usr->memory == NULL) return;
	BMP_ASSERT(usr_idx == bmp_usr->idx);

	bmp_hbm     = BMP_HBM(bmp_usr);
	bmp_dbm     = BMP_DBM(bmp_usr);
	bmp_fsm     = BMP_FSM(bmp_usr);

	// Audit the HBM idle and clear bmap

	__ASSERT(bmp_hbm->idl_cnt == bcm_mwbmap_free_cnt(bmp_hbm->idl_bmap));
	__ASSERT(bmp_hbm->clr_cnt == bcm_mwbmap_free_cnt(bmp_hbm->clr_bmap));

	// Traverse HBM pool
	use_cnt     = 0;
	for (hbm_idx = 1; hbm_idx <= bmp_usr->hbm_max; ++hbm_idx)
	{
		dbm_idx = BMP_CACHE_GET(bmp_usr, hbm_idx);      // Get cached DBM index

		// A HBM index in Free pool must not be in DBM cache.
		if (bcm_mwbmap_isfree(bmp_hbm->idl_bmap, hbm_idx)) {
			__ASSERT(dbm_idx == BMP_DBM_INV);
		}

		// A HBM entry in Clear pool must not be in HBM Idle pool.
		if (bcm_mwbmap_isfree(bmp_hbm->clr_bmap, hbm_idx)) {
			__ASSERT(!bcm_mwbmap_isfree(bmp_hbm->idl_bmap, hbm_idx));
		}

		// Cached DBM entry must be flagged INUSE and linked back to HBM
		if (dbm_idx != BMP_DBM_INV)
		{
			bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);
			// Not in DBM Idle and Clr list
			__ASSERT(bmp_obj->flags & (BMP_FSM_USE | BMP_FSM_CLR));
			__ASSERT(bmp_obj->hbm_idx == hbm_idx);        // Refers back to HBM
			__ASSERT(!bcm_mwbmap_isfree(bmp_hbm->idl_bmap, hbm_idx));
			++use_cnt;                                  // Cached DBM count
		} // else  HBM can be allocated without a cached DBM
	}   // Iterate hbm_idx [1 .. (hbm_total - 1)]

	// DBM Idle count must match = (total DBM - cached DBM)
	__ASSERT(bmp_fsm->idl_cnt == (bmp_usr->dbm_max - use_cnt));

	// Traverse DBM Idle list: DBM entry must be Idle and not HBM paired
	idl_cnt     = 0;
	{
		dll_t * obj_dll;
		dll_t * idl_dll = &bmp_fsm->idl_dll;
		dll_for_each(obj_dll, idl_dll) {
			bmp_obj = BMP_OBJ_DLL(obj_dll);
			__ASSERT(bmp_obj->flags == BMP_FSM_IDL);      // DBM obj is untagged
			__ASSERT(bmp_obj->hbm_idx == BMP_HBM_INV);    // Not paired with HBM
			++idl_cnt;
		}
		__ASSERT(bmp_fsm->idl_cnt == idl_cnt);
	}   // Iterate DBM Idle list

	// Traverse DBM Clear list: Must be paired with a HBM, cached and linked
	clr_cnt     = 0;
	{
		dll_t * obj_dll;
		dll_t * clr_dll = &bmp_fsm->clr_dll;
		dll_for_each(obj_dll, clr_dll) {
			bmp_obj = BMP_OBJ_DLL(obj_dll);
			__ASSERT(bmp_obj->flags & BMP_FSM_CLR);       // DBM obj tagged CLR
			__ASSERT(bmp_obj->hbm_idx != BMP_HBM_INV);    // Paired with HBM
			__ASSERT(bmp_dbm->cache[bmp_obj->hbm_idx] == bmp_obj->dbm_idx);
			__ASSERT(!bcm_mwbmap_isfree(bmp_hbm->idl_bmap, bmp_obj->hbm_idx));
			++clr_cnt;
		}
		__ASSERT(bmp_fsm->clr_cnt == clr_cnt);
	}   // Iterate DBM Clear list

	// Traverse DBM table: DBM entries not IDLE must be cached
	use_cnt     = 0;
	for (dbm_idx = 1; dbm_idx <= bmp_usr->dbm_max; ++dbm_idx)
	{
		bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);
		__ASSERT(bmp_obj->dbm_idx == dbm_idx);            // self reference
		hbm_idx = bmp_obj->hbm_idx;                     // DBM obj's HBM
		if (hbm_idx != BMP_HBM_INV) {                   // Paired with HBM
			// Used: Active or LRU or CLR
			__ASSERT(bmp_obj->flags & (BMP_FSM_USE | BMP_FSM_CLR));
			__ASSERT(bmp_dbm->cache[hbm_idx] == dbm_idx); // Must be in DBM cache
			__ASSERT(!bcm_mwbmap_isfree(bmp_hbm->idl_bmap, hbm_idx));
			++use_cnt;
		} else {
			__ASSERT(bmp_obj->flags == BMP_FSM_IDL);
		}
	}   // Iterate dbm_idx [1 .. (dbm_total - 1)]

#endif /* BCMDBG */

}   // bmp_audit_usr()

void
bmp_dump_usr(uint32 usr_idx, bool verbose, struct bcmstrbuf *b)
{
	bmp_sys_t * bmp_sys;
	bmp_usr_t * bmp_usr;
	bmp_hbm_t * bmp_hbm;
	bmp_dbm_t * bmp_dbm;
	bmp_fsm_t * bmp_fsm;

	bmp_sys     = BMP_SYS();
	bmp_usr     = &bmp_sys->usr[usr_idx];
	if (bmp_usr->memory == NULL) return;
	BMP_ASSERT(usr_idx == bmp_usr->idx);

	bmp_hbm     = BMP_HBM(bmp_usr);
	bmp_dbm     = BMP_DBM(bmp_usr);
	bmp_fsm     = BMP_FSM(bmp_usr);

	BMP_BPRINT(b, "BMP USR %u %s buffer_sz %u cb 0x%p(0x%p) epoch %u\n",
	       usr_idx, bmp_usr_str[usr_idx], bmp_usr->buffer_sz,
	       bmp_usr->cb_fn, bmp_usr->cb_data, bmp_usr->epoch);
	BMP_BPRINT(b, "\tHBM max %4u thr %u idle %u clear %u HME" HADDR64_FMT " sz %u\n",
	       bmp_usr->hbm_max, bmp_usr->hbm_thr,
	       bmp_hbm->idl_cnt, bmp_hbm->clr_cnt,
	       HADDR64_VAL(bmp_hbm->hme.haddr64), bmp_hbm->hme_sz);
	BMP_BPRINT(b, "\tDBM max %4u memory [0x%p %u] base 0x%08x cache 0x%p\n",
	       bmp_usr->dbm_max, bmp_usr->memory, bmp_usr->memory_sz,
	       bmp_dbm->base, bmp_dbm->cache);
	BMP_BPRINT(b, "\tFSM Idle %u Clear %u Used %u\n",
	       bmp_fsm->idl_cnt, bmp_fsm->clr_cnt,
	       bmp_usr->dbm_max - bmp_fsm->idl_cnt);

	BMP_STATS(BMP_BPRINT(b, "\tUSR STATS " BMP_STATS_FMT "\n",
	                 BMP_STATS_VAL(bmp_usr)));
	BMP_STATS(BMP_STATS_ZERO(bmp_usr));

	if (verbose == TRUE)
	{
		uint8       dbm_idx;
		uint16      hbm_idx;
		uint16      max_idx;
		bmp_hme_t   hme;
		bmp_obj_t  *bmp_obj;
		hme.u64     = bmp_hbm->hme.u64;
		BMP_BPRINT(b, "HBM Dump: max %u buffer_sz %u\n",
		       bmp_usr->hbm_max, bmp_usr->buffer_sz);
		max_idx     = MIN(bmp_usr->hbm_max, BMP_DUMP_IDX_MAX);
		for (hbm_idx = 0; hbm_idx <= max_idx; ++hbm_idx) {
			hme.u64 += bmp_usr->buffer_sz;
			if (!bcm_mwbmap_isfree((bmp_hbm)->idl_bmap, hbm_idx)) {
				BMP_BPRINT(b, "%3u. " HADDR64_FMT "\n",
					hbm_idx, HADDR64_VAL(hme.haddr64));
			}
		}
		BMP_BPRINT(b, "DBM Dump: max %u obj_size %u\n",
		       bmp_usr->dbm_max, BMP_OBJ_SIZE(bmp_usr));
		max_idx     = MIN(bmp_usr->dbm_max, BMP_DUMP_IDX_MAX);
		for (dbm_idx = 1; dbm_idx <= max_idx; ++dbm_idx) {
			bmp_obj = BMP_OBJ(bmp_usr, dbm_idx);
			BMP_BPRINT(b, "%3u hbm_idx %3u. obj %p buf %p\n", dbm_idx, bmp_obj->hbm_idx,
			       BMP_OBJ(bmp_usr, dbm_idx), BMP_BUF(bmp_usr, dbm_idx));
		}
	}

	bmp_audit_usr(usr_idx);                             // Audit user

}   // bmp_dump_usr()

void // Dump BMP system state for all registered users
bmp_dump_sys(bool verbose)
{
	uint32  usr_idx;

	bmp_sys_t * bmp_sys = BMP_SYS();

	printf("BMP Rev %u BME key H2D:0x%08x D2H:0x%08x\n",
	       HNDBMP, bmp_sys->h2d_bme, bmp_sys->d2h_bme);

	for (usr_idx = 0; usr_idx < BMP_USR_TOTAL; ++usr_idx)
	{
		BMP_STATS({                                 // accumulate into SYS
			bmp_usr_t * bmp_usr = &bmp_sys->usr[usr_idx];
			if (bmp_usr->memory != NULL) BMP_STATS_ACCUM(bmp_sys, bmp_usr);
		});

		bmp_dump_usr(usr_idx, verbose, NULL);
	}

	BMP_STATS({
		printf("\n");
		printf("\tSYS STATS " BMP_STATS_FMT "\n", BMP_STATS_VAL(bmp_sys));
		memset(&bmp_sys->stats, 0, sizeof(bmp_stats_t));
	});

}   // bmp_dump_sys()

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service Unit Test and Demo for WLAN Integration
 * +--------------------------------------------------------------------------+
 */
#ifdef BMP_UTEST_BUILD
int bmp_utest_g;
static int
bmp_utest_cb_fn(void * cb_data, bmp_cb_reason_t reason,
                bmp_idx_t hbm_idx, bmp_hdr_t * bmp_hdr)
{
	if (bmp_hdr) {
		printf("bmp_utest_cb_fn 0x%p reason %u HBM %u " BMP_HDR_FMT "\n",
			cb_data, reason, hbm_idx, BMP_HDR_VAL(bmp_hdr));
	} else {
		printf("bmp_utest_cb_fn 0x%p reason %u HBM %u\n",
			cb_data, reason, hbm_idx);
	}

	return BCME_OK;
}   // bmp_utest_cb_fn()

#endif /* BMP_UTEST_BUILD */

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service configuration for a user
 * +--------------------------------------------------------------------------+
 */
int // User registers with BMP, parameterizing the BMP service
bmp_reg(uint32 usr_idx, uint32 buffer_sz, uint32 hbm_total, uint32 dbm_total,
        bmp_cb_fn_t cb_fn, void * cb_data)
{
	bmp_sys_t * bmp_sys;  // BMP system global
	bmp_usr_t * bmp_usr;  // BMP User state
	bmp_hbm_t * bmp_hbm;  // BMP HBM state for a user
	bmp_dbm_t * bmp_dbm;  // BMP DBM state for a user
	bmp_obj_t * bmp_obj;  // BMP object in DBM
	bmp_fsm_t * bmp_fsm;  // BMP finite state machine

	uint8       dbm_idx;  // DBM iterator
	uint16      hbm_idx;  // HBM iterator
	uint32      obj_size; // Size of bmp_obj_t includes a buffer of buffer size
	uint32      bytes;    // Size of memories
	uint8     * memory;   // Memory for DBM bmp_obj_t and other state

	// Audit user registration parameters to conform with BMP service design
	BMP_AUDIT_USR_IDX(usr_idx);
	BMP_ASSERT(buffer_sz <= BMP_DMA_MAX);               // Bound DMA to 4KB
	BMP_ASSERT((buffer_sz % BMP_HDR_BYTES) == 0);       // uint64 alignment

	// Buffer Index #0 is reserved. In DBM, no memory is allocated for DBM #0
	BMP_ASSERT((hbm_total >= 2) && (hbm_total <= BMP_HBM_MAX));
	BMP_ASSERT((dbm_total >= 2) && (dbm_total <= BMP_DBM_MAX));
	BMP_ASSERT(cb_fn    != NULL);

	bmp_sys             = BMP_SYS();
	bmp_usr             = &bmp_sys->usr[usr_idx];

	if (bmp_usr->memory) {
		BMP_ERROR("BMP: %s REG : warning already registered\n",
		          bmp_usr_str[usr_idx]);
		return BCME_OK;
	}

	bmp_usr->buffer_sz  = (uint16)buffer_sz;
	obj_size            = BMP_OBJ_SIZE(bmp_usr);        // bmp_obj_t + buffer

	/*
	 * +----------------------------------------------------------------------+
	 *  FAILURE IN RESOURCE ALLOCATIONS IS NOT RECOVERRABLE, SO ASSERT.
	 * +----------------------------------------------------------------------+
	 */

	// +----- Setup HBM ------------------------------------------------------+
	bmp_hbm             = BMP_HBM(bmp_usr);             // User's HBM context

	bytes               = (buffer_sz * hbm_total);      // Includes HBM idx 0
	ASSERT(bytes <= hme_free(HME_USER_BMPSVC));         // Large enough HME
	bmp_hbm->hme.haddr64 = hme_get(HME_USER_BMPSVC, bytes);    // HME uses SGMT policy
	ASSERT(!HADDR64_IS_ZERO(bmp_hbm->hme.haddr64));     // Audit HME address
	bmp_hbm->hme_sz     = bytes;                        // HME region size

	bmp_hbm->idl_bmap   = bcm_mwbmap_init(bmp_sys->osh, hbm_total);
	ASSERT(bmp_hbm->idl_bmap != NULL);
	bcm_mwbmap_force(bmp_hbm->idl_bmap, BMP_IDX_INV);   // Reserve HBM idx 0
	bmp_hbm->idl_cnt    = bcm_mwbmap_free_cnt(bmp_hbm->idl_bmap);
	BMP_ASSERT(bmp_hbm->idl_cnt == (hbm_total - 1));

	bmp_hbm->clr_bmap   = bcm_mwbmap_init(bmp_sys->osh, hbm_total);
	ASSERT(bmp_hbm->clr_bmap != NULL);
	for (hbm_idx = 0; hbm_idx < hbm_total; ++hbm_idx) { // Entire bitmap
		bcm_mwbmap_force(bmp_hbm->clr_bmap, hbm_idx);   // Mark as not clear
	}
	bmp_hbm->clr_cnt    = bcm_mwbmap_free_cnt(bmp_hbm->clr_bmap);
	BMP_ASSERT(bmp_hbm->clr_cnt == 0);

	bmp_usr->hbm_max    = (uint16)(hbm_total - 1);      // [0 .. (hbm_total-1)]
	bmp_usr->hbm_thr    = BMP_HBM_THRESH(hbm_total);

	// +----- Setup DBM ------------------------------------------------------+
	bmp_dbm             = BMP_DBM(bmp_usr);

	// allocate one less DBM bmp_obj_t as index 0 does not exist
	bytes               = (obj_size * (dbm_total - 1))  // DBM pool of bmp_obs
	                    + (sizeof(uint8) * hbm_total);  // hbm2dbm cache
	memory              = (uint8*) MALLOCZ(bmp_sys->osh, bytes);
	ASSERT(memory != NULL);                             // Assert on failure

	// DBM object pool is faked back by one DBM object, as DBM idx 0 is unused.
	bmp_dbm->base       = (uintptr)memory - obj_size;   // Subtract one DBM obj
	bmp_dbm->cache      = memory + (obj_size * (dbm_total - 1)); // Cache table
	for (hbm_idx = 0; hbm_idx <= bmp_usr->hbm_max; ++hbm_idx) {
		bmp_dbm->cache[hbm_idx] = BMP_DBM_INV;
	}   // Iterate hbm_idx

	bmp_fsm             = &bmp_dbm->fsm;                // User's FSM context
	bmp_fsm->idl_cnt    = 0;
	bmp_fsm->clr_cnt    = 0;
	dll_init(&bmp_fsm->idl_dll);
	dll_init(&bmp_fsm->clr_dll);
	dll_init(&bmp_fsm->lru_dll);
	dll_init(&bmp_fsm->act_dll);

	for (dbm_idx = 1; dbm_idx < dbm_total; ++dbm_idx)   // Idx 0 does not exist
	{
		bmp_obj         = (bmp_obj_t*)
		                  (bmp_usr->dbm.base + (obj_size * dbm_idx));

		dll_init(&bmp_obj->dll);
		bmp_obj->dbm_idx = dbm_idx;                     // Self
		bmp_obj->hbm_idx = BMP_HBM_INV;                 // Not caching a HBM

		dll_append(&bmp_fsm->idl_dll, &bmp_obj->dll);   // Add to idle list
		bmp_fsm->idl_cnt++;

		bmp_obj->flags  = BMP_FSM_IDL;                  // Object tagged IDLE
	}   // Iterate dbm_idx

	// Setup rest of BMP user
	bmp_usr->dbm_max    = dbm_total - 1;                // [1 .. (dbm_total-1)]

	bmp_usr->cb_fn      = cb_fn;                        // Callback handler
	bmp_usr->cb_data    = cb_data;

	bmp_usr->memory     = memory;                       // Sysmem malloc ptr
	bmp_usr->memory_sz  = bytes;                        // Sysmem malloc bytes

	bmp_usr->idx        = usr_idx;                      // Self index

#if defined(BMP_DEBUG_BUILD)
	bmp_dump_usr(usr_idx, /* verbose = */ FALSE, NULL);       // Dump user state
#endif /* BMP_DEBUG_BUILD */

	return BCME_OK;

}   // bmp_reg()

/**
 * +--------------------------------------------------------------------------+
 *  Section: BMP service initialization and command line control
 * +--------------------------------------------------------------------------+
 */

int // See rte.c. Initialize the BMP RTE service
BCMATTACHFN(bmp_init)(si_t * sih, osl_t * osh)
{
	bmp_sys_t * bmp_sys = BMP_SYS();

	ASSERT(sizeof(bmp_hdr_t) == sizeof(uint64));

	memset(bmp_sys, 0, sizeof(bmp_sys_t));

	bmp_sys->h2d_bme = bme_get_key(osh, BME_USR_H2D);
	bmp_sys->d2h_bme = bme_get_key(osh, BME_USR_D2H);

	bmp_sys->osh = osh; // RTE's osh used by BMP for Memory and BME service

	BMP_PRINT("BMP Rev %d BME key H2D:0x%08x D2H:0x%08x\n",
	          HNDBMP, bmp_sys->h2d_bme, bmp_sys->d2h_bme);

	return BCME_OK;
}   // bmp_init()

static void // BMP service command handler usage
_bmp_help(void)
{
	printf("BMP Rev %u\n"
	       "\tUsage: wl -i <ifname> cmd bmp -[d|v]\n"
	       "\t\t-a <usr_idx>: Audit user\n", HNDBMP);
	printf("\t\t-u <usr_idx>: Dump user\n"
	       "\t\t-U <usr_idx>: Dump user verbose\n");
	printf("\t\t-d:           Dump service\n"
	       "\t\t-v:           Dump service verbose\n");

#ifdef BMP_UTEST_BUILD
	printf("\n\n\t\tUNIT TEST\n");
	printf("\t\t-r:           Register ARQ64,  Buf=266B,  HBM=512, DBM=64\n"
	       "\t\t                       ARQ256, Buf=1040B, HBM=384, DBM=48\n");
	printf("\t\t-b <hbm_idx>: Buf <hbm_idx>\n"
	       "\t\t-g:           Get <hbm_idx>\n"
	       "\t\t-p <hbm_idx>: Put <hbm_idx>\n");
	printf("\t\t-c <hbm_idx>: Clr a <hbm_idx>\n"
	       "\t\t-o <hbm_idx>: PGO <hbm_idx>\n"
	       "\t\t-i <hbm_idx>: PGI <hbm_idx>\n"
	       "\t\t-s:           CNT \n");
#endif /* BMP_UTEST_BUILD */

}   // _bmp_help()

/*
 * +--------------------------------------------------------------------------+
 *  BMP service RTE console command handler (registered in rte.c)
 *      wl  -i <ifname> cmd  bmp -[a|u|U] <usr_idx> | -[d|v]
 *      dhd -i <ifname> cons bmp -[a|u|U] <usr_idx> | -[d|v]
 * +--------------------------------------------------------------------------+
 */
#ifdef BMP_UTEST_BUILD
#endif /* BMP_UTEST_BUILD */

static int // Parse the user index in argv[2] for commands -[a|u|U]
_bmp_parse_usr_idx(int argc, char * argv[])
{
	int usr_idx;
	if (argc < 3) {
		printf("unpecified user index\n");
		return BCME_ERROR;
	}
	usr_idx = atoi(argv[2]);
	if ((usr_idx < 0) || (usr_idx >= BMP_USR_TOTAL)) {
		printf("invalid user index %d\n", usr_idx);
		return BCME_ERROR;
	}
	return usr_idx;
}   // _bmp_parse_usr_idx()

/**
 *  BMP Service command handler attached to RTE.
 *  - see rte.c: hnd_cons_add_cmd
 */
void
bmp_cmd(void *arg, int argc, char *argv[])
{
	char      * p;
	char        op_code;

#ifdef BMP_UTEST_BUILD
	void      * buffer;
	bmp_idx_t   hbm_idx, bmp_idx;
	const char* usr_arqw0 = bmp_usr_str[BMP_USR_UTEST];

#define _ARGV2HBMIDX_() \
({  int _hbm_idx_ = (bmp_idx_t) atoi(argv[2]); \
	if (_hbm_idx_ == BMP_IDX_INV) { printf("Invalid HBM index 0\n"); return; } \
	_hbm_idx_; \
})
#endif /* BMP_UTEST_BUILD */

	if (argc < 2)       goto cmd_invalid;           // Need op code chatacter

	p       = argv[1];                              // Options to bmp command
	if (p[0] != '-')    goto cmd_invalid;           // Op code prefix '-'

	op_code = p[1];                                 // Single character op code
	switch (op_code)
	{
#ifdef BMP_UTEST_BUILD
		case 'r': // Register a User
			bmp_reg(BMP_USR_UTEST,
			        272, 512, 64, bmp_utest_cb_fn, &bmp_utest_g);

			bmp_reg(BMP_USR_ARQW1,
			        1064, 384, 48, bmp_utest_cb_fn, &bmp_utest_g);
			break;

		case 'b': // Cache a HBM buffer, if not in DBM cache, and return buffer
			hbm_idx = _ARGV2HBMIDX_();
			buffer  = bmp_buf(BMP_USR_UTEST, hbm_idx);
			printf("\t0x%p = bmp_buf(%s %u)\n", buffer, usr_arqw0, hbm_idx);
			break;

		case 'g': // Allocate a HBM buffer and return HBM index uncached
			bmp_idx = bmp_get(BMP_USR_UTEST, NULL);
			printf("\t%u = bmp_get %s\n", bmp_idx, usr_arqw0);
			break;

		case 'p': // Free a HBM buffer, releasing the cached DBM
			hbm_idx = _ARGV2HBMIDX_();
			bmp_put(BMP_USR_UTEST, hbm_idx);
			printf("\tbmp_put(%s %u)\n", usr_arqw0, hbm_idx);
			break;

		case 'c': // Tag an HBM buffer as clean
			hbm_idx = _ARGV2HBMIDX_();
			bmp_clr(BMP_USR_UTEST, hbm_idx);
			printf("\tbmp_clr(%s %u)\n", usr_arqw0, hbm_idx);
			break;

		case 'o': // Explicitly PageOUT a HBM buffer
			hbm_idx = _ARGV2HBMIDX_();
			bmp_pop(BMP_USR_UTEST, hbm_idx, bmp_dir_d2h);
			printf("\tbmp_pop(%s %u PGO)\n", usr_arqw0, hbm_idx);
			break;

		case 'i': // Explicitly PageIN a HBM buffer
			hbm_idx = _ARGV2HBMIDX_();
			bmp_pop(BMP_USR_UTEST, hbm_idx, bmp_dir_h2d);
			printf("\tbmp_pop(%s %u PGI)\n", usr_arqw0, hbm_idx);
			break;

		case 's': // Statistics
		{
			uint32 dbm_idl_cnt, hbm_idl_cnt, hbm_clr_cnt;
			dbm_idl_cnt = hbm_idl_cnt = hbm_clr_cnt = 0;
			bmp_cnt(BMP_USR_UTEST, &dbm_idl_cnt, &hbm_idl_cnt, &hbm_clr_cnt);
			printf("\tbmp stats(%s dbm free %u hbm free %u clear %u)\n",
			       usr_arqw0, dbm_idl_cnt, hbm_idl_cnt, hbm_clr_cnt);
			break;
		}

		case 'e': // Begin an epoch
			bmp_bgn(BMP_USR_UTEST);
			printf("\tbmp_bgn %s\n", usr_arqw0);
			break;

		case 'n': // End an epoch
			bmp_end(BMP_USR_UTEST);
			printf("\tbmp_end %s\n", usr_arqw0);
			break;

#endif /* BMP_UTEST_BUILD */

		case 'a':
		{
			int usr_idx = _bmp_parse_usr_idx(argc, argv);
			if (usr_idx == BCME_ERROR)  goto cmd_invalid;
			printf("BMP User %s audit\n", bmp_usr_str[usr_idx]);
			bmp_audit_usr(usr_idx);
			break;
		}

		case 'u':
		case 'U':
		{
			bool verbose = (op_code == 'U') ? TRUE : FALSE;
			int usr_idx = _bmp_parse_usr_idx(argc, argv);

			if (usr_idx == BCME_ERROR)  goto cmd_invalid;
			bmp_dump_usr(usr_idx, verbose, NULL);
			break;
		}

		case 'd': bmp_dump_sys(FALSE);  break;      // verbose = FALSE
		case 'v': bmp_dump_sys(TRUE);   break;      // verbose = TRUE

		default: goto cmd_invalid;

	} // switch op_code

	return;

cmd_invalid:
	_bmp_help();

}   // bmp_cmd()

#endif /* HNDBMP */
