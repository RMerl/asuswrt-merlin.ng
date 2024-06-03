/*
 * Broadcom Full Dongle Host Memory Extension (HME)
 *  Interface to manage Host Memory Extensions.
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
 * $Id: bcmhme.h 830428 2023-09-21 06:37:46Z $
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 */

#ifndef _BCMHME_H
#define _BCMHME_H

#include <typedefs.h>
#include <bcmpcie.h>
#include <hndbme.h>
#ifdef BCM_HMOSCB
#include <wltunable.h>
#endif

struct osl_info;    // forward decl: osl_decl.h
struct pcie_ipc;    // forward decl: bcmpcie.h

/**
 * HME: Memory Management policy.
 *
 * Segment Carving paradigm, is useful for carved allocation of variable sized
 * memory blocks, that persist for the operational duration of the system. This
 * scheme may be used when all blocks are intended to be freed simulataneously,
 * effectively freeing the entire HME memory. E.g. PCIE_IPC_HME_USER_MACIFS.
 *
 * Pool of fixed size memory blocks, with free blocks identified using a
 * dongle resident multi word bitmap.
 *
 * Split-Coalesce support is a WIP. Requires a walk of the free block list in
 * host memory using backplane accesses over PCIE. E.g. PCIE_IPC_HME_USER_LCLPKT
 * Split-Coalesce helps reduce internal and external fragmentation at an
 * exhorbitant cost of CPU cycles for backplane accesses of free-block list.
 * Split-Coalesce paradigm is appropriate when there is a large disparity in
 * typical block-sizes and the sojourn time of allocated blocks.
 *
 * Memory may be allocated from the HME region ONLY after the Dongle and DHD
 * handshake has completed, i.e. the PCIE IPC Link Phase completed.
 *
 */

/**
 *  HME well known User IDs, across PCIe IPC
 *  - PCIe IPC rev 0x82, 0x83: max users is PCIE_IPC_HME_USERS_MAX = 8
 *  - PCIe IPC rev 0x84 onwards, a variable number of users is supported.
 *
 *  bcmhme.h is not PCIe IPC revision control
 */
#define HME_USER_SCRMEM PCIE_IPC_HME_USER_SCRMEM //  0. 4 KByte Scratch memory
#define HME_USER_PKTPGR PCIE_IPC_HME_USER_PKTPGR //  1. HWA2.3 PktPgr feature
#define HME_USER_MACIFS PCIE_IPC_HME_USER_MACIFS //  2. HWA2.3 MACIFs offload
#define HME_USER_LCLPKT PCIE_IPC_HME_USER_LCLPKT //  3. HWA2.3 LCLPKT offload
#define HME_USER_PSQPKT PCIE_IPC_HME_USER_PSQPKT //  4. PSQPKT feature
#define HME_USER_CSIMON PCIE_IPC_HME_USER_CSIMON //  5. CSIMON feature
#define HME_USER_HMOSWP PCIE_IPC_HME_USER_HMOSWP //  6. SW_PAGING feature
#define HME_USER_MBXPGR PCIE_IPC_HME_USER_MBXPGR //  7. STS_XFER with MBX Pager
#define HME_USER_AIR_IQ PCIE_IPC_HME_USER_AIR_IQ //  8. AirIQ feature
#define HME_USER_SBFTBL PCIE_IPC_HME_USER_SBFTBL //  9. Station Buffer Fairness
#define HME_USER_HMOSCB PCIE_IPC_HME_USER_HMOSCB //  10. SCB Offload
#define HME_USER_LOWMAC PCIE_IPC_HME_USER_LOWMAC //  11. 4 KByte - lower MAC
#define HME_USER_CLSBUF PCIE_IPC_HME_USER_CLSBUF //  12. Upper-MAC Console Log Streaming
#define HME_USER_BMPSVC PCIE_IPC_HME_USER_BMPSVC //  13. Buffer Memory Pager Service
#define HME_USER_HMOPPR PCIE_IPC_HME_USER_HMOPPR //  14. PPR Offload
#define HME_USER_MACDBG PCIE_IPC_HME_USER_MACDBG //  15. MACDBG Offload
#define HME_USER_RU_CFG PCIE_IPC_HME_USER_RU_CFG //  16. RU configuration Offload
#define HME_USER_SCMEAS PCIE_IPC_HME_USER_SCMEAS //  17. SCMEAS Scan Measurement Offload
#define HME_USER_HMOEVT PCIE_IPC_HME_USER_HMOEVT //  18. BCM events Offload
#define HME_USER_MLOIPC PCIE_IPC_HME_USER_MLOIPC //  19. Host Memory for MLO IPC rings
#define HME_USER_PCAPXF PCIE_IPC_HME_USER_PCAPXF //  20. PCAP Hdr Xfer

#define HME_USERS_TOTAL    21

#ifdef BCM_HMOSCB
/*
 * Determine how many slots to reserve if no nvram configured:
 * - 1 slot per SCB, based on DEFMAXSCB
 * - 1 slot per BCMC_SCB, based on WLC_MAXBSSCFG (every bss has 1 bcmc scb)
 * - 1 slot per BSSCFG
 */
#define HMOSCB_MAX_ITEMS  (DEFMAXSCB + 2 * (WLC_MAXBSSCFG))
#define HME_HMOSCB_BYTES  (HMOSCB_MAX_ITEMS * HMOSCB_IPC_HME_SCB_SIZE)
#endif

#ifdef BCM_HMO_MACDBG
#define MACDBG_HME_BYTES (384 * 1024)
#endif /* BCM_HMO_MACDBG */

#ifdef BCM_HMO_SCMEAS
#define SCMEAS_HME_BYTES (128 * 1024)
#endif /* BCM_HMO_SCMEAS */

typedef enum hme_policy
{
	HME_MGR_NONE = 0,           // No memory manager
	HME_MGR_SGMT = 1,           // Carve out variable sized segments from front
	HME_MGR_POOL = 2,           // Pool of fixed sized mem blocks
	HME_MGR_SPCL = 3            // Split-Coalesce paradigm: WIP
} hme_policy_t;

typedef enum hme_class          // per HME user memory class
{
	HME_CLASS_FLM = 0,          // Fault MMU: SWP paradigm; Should be first
	HME_CLASS_DBG = 1,          // Debug: Scrachpad, Console logging
	HME_CLASS_HWC = 2,          // HW CORE: Accessed by HWA, M2M, MLC
	HME_CLASS_NTK = 3,          // Network: Accessed by DHD - CSIMON, AIRIQ, PCAPXF
	HME_CLASS_MSC = 4,          // Miscellaneous: May use BME or S2P sbtopcie_xxx()

	HME_CLASS_RAC = 5,          /* RACable: Access in Plane. Uses S2P middle win
				     * so needs special handling to meet 256MB boundary limit
				     */
	HME_CLASS_MAX
} hme_class_t;

/** HME service Initialization. See rte.c :: hnd_init() with backplane osh */
extern int       BCMATTACHFN(hme_init)(struct osl_info *hnd_osh);

/** HME System Debug Dump */
extern void      hme_dump(bool verbose);

/** Fetch the segment avail count */
extern uint16    hme_sgmt_avail(int user_id);

/** Fetch the segments */
extern pcie_ipc_hme_sgmt_t * hme_sgmts(int user_id);

/** Fetch the base address of a HME region */
extern haddr64_t hme_haddr64(int user_id);

/** Fetch the virtual address of a HME region */
extern uint64    hme_phy2virt(int user_id, uint64 haddr_u64);

/** Allocate memory from HME for a given user */
extern haddr64_t hme_get(int user_id, size_t bytes);

/** Deallocate memory into HME for a given user */
extern int       hme_put(int user_id, size_t bytes, haddr64_t haddr64);

/** Query free and used memory in HME for a given user */
extern int       hme_free(int user_id);
extern int       hme_used(int user_id);

/** Fetch SBTOPCIE user mapped to HME user's memory */
extern uint32    hme_sbtopcie_usr(int user_id);

/** Check capabilities */
extern bool      hme_cap(int user_id);

/** Attach a HME User Memory Manager during PCIE IPC Bind phase. */
extern int       BCMATTACHFN(hme_attach_mgr)(int user_id, uint32 pages,
                        uint32 item_size, uint32 items_max, hme_policy_t policy,
                        uint32 align_bits, uint32 bound_bits, uint32 flags,
                        hme_class_t class, uint32 sbtopcie_usr);

/**
 * HME integration into Dongle/Host Bind and Link Phase in pciedev bus layer
 *   + HME Users Memory Extensions are requested using PCIE IPC in Bind Phase
 *   + HME Users Memory Extensions are retrieved from PCIE IPC in Link Phase
 */
extern int      BCMATTACHFN(hme_bind_pcie_ipc)(void *pciedev, struct pcie_ipc *pcie_ipc);
extern int      hme_link_pcie_ipc(void *pciedev, struct pcie_ipc *pcie_ipc);
extern void     hme_dump_pcie_ipc(void *pciedev, struct pcie_ipc *pcie_ipc);

#ifdef HNDBME
/**
 * Library of wrapper routines to leverage ByteMoveEngine based DMA transfers
 * to/from Host Memory Extensions.
 */

/** In HND rte.c, HME registers as BME user BME_USR_H2D and BME_USR_D2H */
extern int BCMATTACHFN(hme_bind_bme)(void); // private to rte.c

/**
 * HME_USER_NOOP may be passed to hme_d2h_xfer or hme_h2d_xfer, to suggest
 * that no HME memory allocation or deallocation operation, is required, and
 * the provided host memory address will be used as-is in the transfer request.
 */
#define HME_USER_NOOP           (-1)

/**
 * hme_d2h_xfer:   Initiate a BME assisted D2H DMA transfer from a dongle memory
 *                 to host memory.
 *                 When a well known HME User Id is specified in hme_user_id, a
 *                 block will be allocated from the users pool allocator, and
 *                 serve as the destination host memory.
 *                 Otherwise, the host memory need not be a HME block.
 *
 * Parameters:
 *    src_daddr32: source dongle memory address
 *    dst_haddr64: pointer to a 64bit destination host memory address
 *                 When hme_user_id == HME_USER_NOOP, a pointer to a valid host
 *                 memory address that may or may not belong to a HME.
 *                 When hme_user_id is a well known HME user id, a HME block
 *                 will be allocated, and its address will be returned through
 *                 this pointer.
 *    bytes      : Number of bytes to transfer in D2H direction
 *    sync       : boolean to request a spin loop until xfer completes. User
 *                 may initiate the transfer and later explicitly invoke
 *                 hme_d2h_sync() to sync on transfer completion.
 *   hme_user_id:  HME_USER_NOOP or one of the well known HME User IDs.
 *                 When a well known HME User is specified, the User's memory
 *                 extension must be managed by hme_mgr_pool_t.
 *                 No support for other HME memory managers.
 *
 * Returns:
 *    < 0        : hme not yet initialized
 *                 or no memory, if allocation was requested.
 *      0        : [case sync = TRUE]  BCME_OK: Success
 *    > 0        : [case sync = FALSE] Sync key
 *                 Sync Key to be used in subsequent hme_d2h_sync() invocation
 */

extern int       hme_d2h_xfer(const void *src_daddr32, haddr64_t *dst_haddr64,
                              uint32 bytes, bool sync, int hme_user_id);

/**
 * hme_h2d_xfer: Initiate a BME assisted H2D DMA transfer from a host memory
 *               to dongle memory. The host memory must be a HME block, if
 *               an explicit deallocation is requested via the parameter
 *               hme_user_id (!= HME_USER_NOOP).
 *
 * Parameters:
 *    src_haddr64: pointer to a 64bit source host memory address
 *    dst_daddr32: destination dongle memory address
 *    bytes      : Number of bytes to transfer in H2D direction
 *    sync       : boolean to request a spin loop until xfer completes. User
 *                 may initiate the transfer and later explicitly invoke
 *                 hme_h2d_sync() to sync on transfer completion.
 *
 * Returns:
 *    < 0        : BCME ERROR, BADLEN,
 *      0        : [case sync = TRUE]  BCME_OK: Success
 *    > 0        : [case sync = FALSE] Sync Key
 *                 Sync Key to be used in subsequent hme_d2h_sync() invocation
 */
extern int       hme_h2d_xfer(haddr64_t * src_haddr64, void * dst_daddr32,
                              uint32 bytes, bool sync, int hme_user_id);

/**
 * hme_swp_xfer: Initiate a BME assisted DMA transfer of a text page from host
 *               to dongle.
 *
 * Parameters:
 *    src_haddr64: pointer to a 64bit source host memory address
 *    dst_daddr32: destination dongle memory address
 */
extern int       hme_swp_xfer(haddr64_t * src_haddr64, void * dst_daddr32);

/** Explicitly sync on a previous xfer call, using the returned key. */
extern void      hme_xfer_sync(int cpy_key);

/** Sync on "all" previous ongoing engine xfer transactions. */
extern void      hme_h2d_sync(void);
extern void      hme_d2h_sync(void);

#endif /* HNDBME */

#ifdef HNDBHM
/*
 * +--------------------------------------------------------------------------+
 *  Boot Host Memory (BHM) Region carving
 *
 *  We now need very large memories (several MBytes) for PacketPager HBM
 *  (e.g. a MAP's PP can support upto 64K Lfrags (each of320 Bytes) for a
 *  total of 20 MBytes. Likewise MAP, would need 86 TxFIFOs with 32 of them
 *  sized to 4K deep for BA512 MLO.
 *  With Router SoC running linux and a default PAGESIZE allowing a maximum of
 *  4MB allocation, a HME serive with runtime kmalloc to serve PP, PQP, MACIFS,
 *  STS, is not a viable solution.
 *
 *  In Linux, The Contiguous Memory Allocator (CMA) framework allows device
 *  drivers to allocate large (> 4MB) physically contiguous memory averting
 *  runtime problems arising from a fragmented memory or native
 *  OS PAGESIZE limitations.
 *
 *  DHD will reserve the memory from CMA region and pass the BHM buffer to
 *  dongle using a TLV (bhm_location_tlv_t).
 *  HME is then extended to manage host memory from pre-allocated BOOT region
 *  as well as the legacy HME regions that are dongle feature dependent
 *  requiring an explicit dongle advertisement of capabilities in the
 *  PCIe bind/link training phase.
 *
 *  By design, there is no bhm_free_request()
 *
 *  HME service will also register SBTOPCIE private window for whole BHM region.
 *  So, if an HME user's memory is carved from BHM the application doesn't
 *  need to reserve a user specific SBTOPCIE private/shared window and can
 *  access the host memory using S2P BHM user returned by hme_sbtopcie_usr()
 *  SBTOPCIE_USR_BHM_1/SBTOPCIE_USR_BHM_2
 *
 * +--------------------------------------------------------------------------+
 */

// Global instance of the BHM LOCATION TLV. During boot, startarm extracts this
// TLV from SysMem Top and makes it available to HME driver via this global.
extern bhm_location_tlv_t bhm_location_tlv_g;

extern bool	bhm_bind_sbtopcie(void);

#endif /* HNDBHM */

#endif /* _BCMHME_H */
