/*
 * Generic Broadcom Home Networking Division (HND) BME module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
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
 * $Id: hndbme.h 999999 2017-01-04 16:02:31Z $
 */

#ifndef _HNDBME_H_
#define _HNDBME_H_

/**
 * ------------------------------------------------------------------------------
 * Section: BME Memory Types
 *
 * BME configuration is qualified by the src and dst memory and implicates:
 * A. HW Coherency Access
 * B. Over PCIE Access (for discrete WLAN Chipset over PCIe interconnect)
 *    - NotPCIE (burst/outstanding ops)
 *    - 64bit PCIe addressing space(AddrExt, PCI64ADDR_HIGH)
 * ------------------------------------------------------------------------------
 */

/** BME DMA transactions qualified by source or destination memory type */
typedef uint8  bme_mem_t;

#define BME_MEM_DNGL        (1 << 0) /* WLAN Chipset Local SysMem */
#define BME_MEM_PCIE        (1 << 1) /* Router SoC Host DDR over PCIe */
#define BME_MEM_UBUS        (1 << 2) /* Router SoC DDR over UBUS (int-MAC) */
#define BME_MEM_CHIP        (1 << 3) /* MAC CHIP internal RAM */

#define BME_MEM_MASK        (0x0F)
#define BME_MEM_NONE        ((uint8)(~BME_MEM_MASK)) /* undefined */
#define BME_MEM_INV         ((uint8)(~0))

/** Test whether memory is coherent */
#define BME_MEM_ISCO(mem)   ((mem) & (BME_MEM_DNGL | BME_MEM_PCIE | BME_MEM_UBUS))

/** Test whether memory is Router SoC DDR */
#define BME_MEM_ISDDR(mem)  ((mem) & (BME_MEM_PCIE | BME_MEM_UBUS))

/**
 * ------------------------------------------------------------------------------
 *
 * Section: BME Engine Selection Policy and Engine Set Parameter
 *
 * A pool of engines may be subdivided into reserved or shared usage.
 * - Applications on critical datapath may reserve an engine for optimal
 *   performance on SoCs with spare engines for shared usage.
 * - Shared engines may be grouped (subset) to afford parallelism of DMA transfer
 *   and CPU processing. E.g. a list of packets need to be paged in/out of local
 *   SysMem to an extended memory carved out of Router SoC host memory.
 *
 * An engine is referenced by its index (begining from 0) and independent of the
 * actual DMA channel # that the engine uses. (WLAN Chipset abstraction).
 *
 * ------------------------------------------------------------------------------
 */

/** BME Selection Policy */
typedef uint8  bme_sel_t;
#define BME_SEL_RSV         (0)    /* by engine index reservation, [0..N) */
#define BME_SEL_IDX         (1)    /* by engine index selection,   [0..N) */
#define BME_SEL_SET         (2)    /* by subset of shared engine,  bitmap */
#define BME_SEL_ANY         (3)    /* by set of shared engine,     bitmap */

#define BME_SEL_MAX         (4)    /* see bme_sel_str in bme_dump() */

/** BME Selection Parameter: listing eligible engine(s) by index or map */
typedef union bme_set {
	uint8 idx;                 /* BME_SEL_(RSV,IDX): engine index */
	uint8 map;                 /* BME_SEL_(SET,ANY): bitmap of 8 engine */
} bme_set_t;

/** Constructing a bme_set bitmap. Engine indices begin at 0 */
#define BME_SET(eng_idx)    (1 << (eng_idx))
#define BME_SET_ANY         ((uint8)(~0))  /* set of any engine in SoC */

/**
 * ------------------------------------------------------------------------------
 * Section: BME User
 *
 * ByteMoveEngine are shared by several use cases. [FD: Full Dongle Driver]
 *
 * In a discrete WLAN chipset with a FD driver, a generic H2D and D2H user is
 * defined as an alternative to the asynchronous (descriptor based) H2D and D2H
 * mem2mem scheme. H2D and D2H users may register a single engine set, with
 * sync when done.
 *
 * PAGER (IN/OUT) user may enlist a "set of engines" for efficient bulk
 * transfer operations, unlike H2D and D2H user where a single engine suffices.
 * Paging: a list of packets may be transferred by parallelizing SW preparation
 * of a packet and actual DMA transfer. PGR is not limited to "packet" and could
 * be used for offloading FIFO (descriptor rings) or even miscellaneous data
 * segments like WLAN station state (pointer-less).
 *
 * In an integrated WLAN SoC with NIC Mode driver, BME may be leveraged
 *       by a NIC/FD driver for Router Mem2Mem HW assisted copy.
 * ------------------------------------------------------------------------------
 */

#ifndef BME_USR_MAX
#define BME_USR_MAX     8
#endif // endif

#define BME_USR_SYS         (0) /* BME System reserved index */

/* Users are organized by src and dst memory, number of engines, etc */
#define BME_USR_RLM         (1) /* RateLinkMem : SysMem|DDR to MAC mem */
#define BME_USR_H2D         (2) /* FD: Host DDR to Dongle SysMem */
#define BME_USR_D2H         (3) /* FD: Dongle SysMem to Host DDR */
#define BME_USR_FD0         (4) /* FD: Dual Buffer streaming to host */
#define BME_USR_FD1         (BME_USR_FD0 + 1)
#define BME_USR_UN6         (6) /* Undefined */
#define BME_USR_UN7         (7) /* Undefined */

/** see bme_usr_str in bme_dump() */
#if (BME_USR_MAX > 8)
#error "Maximum 8 Users supported"
#endif // endif

#define BME_USR_INV         ((uint8)~0)
#define BME_INVALID         (~0U)

typedef uint8  bme_usr_t;

/**
 * ------------------------------------------------------------------------------
 * Chipsets with multiple (> 2) BME engines may reserve an engine. Criteria for
 * permitting engine reservation (engine dedicated for specific use):
 * - deadline driven datapath, avoid costly DMA engine configuration, always
 *   available as idle, always sync when done, etc.
 * List of reserved DMA engines.
 * ------------------------------------------------------------------------------
 */

/**
 * ------------------------------------------------------------------------------
 * Section: BME Application Interface
 * Caller is responsible for Mutual Exclusion, e.g. between a copy and sync.
 * APIs are not re-entrant.
 * ------------------------------------------------------------------------------
 */

/** Debug dump the BME information */
extern void bme_dump(osl_t * osh, bool verbose);
extern void hnd_cons_bme_dump(void *arg, int argc, char *argv[]);

/** Initialize and attach BME service with OS */
extern int bme_init(si_t * sih, osl_t * osh);

/** De-Initialize and detach BME service from OS */
extern int bme_deinit(si_t * sih, osl_t * osh);

/** Register a user with BME service. Returns a BME key or BME_KEY_INV */
/* bme_set: idx=[BME_SEL_RSV, BME_SEL_IDX], map=[BME_SEL_SET, BME_SEL_ANY] */
extern int bme_register_user(osl_t * osh, bme_usr_t bme_usr,
                     bme_sel_t bme_sel, bme_set_t bme_set,
                     bme_mem_t bme_mem_src, bme_mem_t bme_mem_dst,
                     uint32 hi_src, uint32 hi_dst);
/** De-register a user from BME service. */
extern int bme_unregister_user(osl_t * osh, bme_usr_t bme_usr);

/** Get registered user's key. If user is not registered returns BME_KEY_INV */
extern int bme_get_key(osl_t * osh, bme_usr_t bme_usr);

/** Sync on a transfer completion (poll loop for transfer complete) */
extern void bme_sync_all(osl_t * osh);
extern void bme_sync_eng(osl_t * osh, int eng_idx); /* eng_idx from bme_copy */
extern void bme_sync_usr(osl_t * osh, bme_usr_t bme_usr);

/** Invoke a BME copy service. Returns bme engine index for use in sync calls */
extern int bme_copy(osl_t * osh, uint bme_key_id, const void *src, void *dst, uint len);
extern int bme_copy64(osl_t *osh, uint bme_key_id, const uint64 src, uint64 dst, uint len);

#endif /* _HNDBME_H_ */
