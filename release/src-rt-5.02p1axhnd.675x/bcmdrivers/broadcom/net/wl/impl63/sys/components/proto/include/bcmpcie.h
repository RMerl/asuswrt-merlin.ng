/*
 * Broadcom PCIE
 * Software-specific definitions shared between device and host side
 * Explains the shared area between host and dongle
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
 * $Id: bcmpcie.h 783755 2020-02-07 20:57:50Z $
 */

#ifndef	_bcmpcie_h_
#define	_bcmpcie_h_

#include <bcmutils.h>

/* PCIe IPC uses LittleEndian format. Dongle host order is also Little Endian */

/**
 * Design Note:
 *
 * Dongle addresses are explicitly identified as daddr32_t and have a "_daddr32"
 * suffix. Do not cast a daddr32_t to a pointer type in hosts, as hosts may be
 * using 64bit addressing.
 *
 * Host addresses are always 64bit and explicitly identified as haddr64_t and
 * have a "_haddr64" suffix.
 *
 * When host addresses are used in IPC, use below HADDR64_ prefixed macros.
 * When haddr64_t is used in bcmmsgbuf.h, the reqmt for 64b alignment is imposed
 * by explicit padding in the structure definitions.
 *
 * Avoid casting to a uint64 and swab64 on haddr64_t objects.
 *
 * Macros may be modified during debug exercise to perform audits, logging, etc.
 * E.g. to debug a missing htol32 in DHD, Dongle may audit all addresses passed
 * to it to be within a range, or log to buzzz etc by simply modifying the macro
 * during a debug exercise.
 *
 */
/* 32bit dongle addresses (daddr32_t) may be read by 64bit hosts. */
typedef uint32 daddr32_t;

/* 64bit Host addresses are used by DMA engines. */
typedef dma64addr_t haddr64_t; /* No 64bit alignment requirement */

/* haddr64_t to/from uint32 construction without endian conversion. */
/* Retrieve/Get the lo and hi address field without endian conversion. */
#define HADDR64_LO(haddr64)                 ((haddr64).lo)
#define HADDR64_HI(haddr64)                 ((haddr64).hi)

/* Assign/Set the lo and hi address field without endian conversion. */
#define HADDR64_LO_SET(haddr64, u32)        ((haddr64).lo = (u32))
#define HADDR64_HI_SET(haddr64, u32)        ((haddr64).hi = (u32))

/* haddr64_t to/from uint64 construction without endian conversion. */
#define HADDR64_TO_U64(haddr64, haddr_u64) \
	(haddr_u64) = (((uint64)(HADDR64_HI(haddr64))) << 32) \
	            | (HADDR64_LO(haddr64));

#define HADDR64_FROM_U64(haddr64, haddr_u64) \
({ \
	HADDR64_LO_SET((haddr64), (uint32)(haddr_u64)); \
	HADDR64_HI_SET((haddr64), (uint32)((haddr_u64) >> 32)); \
})

#define HADDR64_SET(haddr64_to, haddr64_from) \
({ \
	HADDR64_LO(haddr64_to) = HADDR64_LO(haddr64_from); \
	HADDR64_HI(haddr64_to) = HADDR64_HI(haddr64_from); \
})

/* PCIe IPC is LE: Perform an ltoh32 on address value retrieved */
#define HADDR64_LO_LTOH(haddr64)            (ltoh32(HADDR64_LO(haddr64)))
#define HADDR64_HI_LTOH(haddr64)            (ltoh32(HADDR64_HI(haddr64)))
#define HADDR64_LTOH(haddr64) \
({ \
	/* in place LTOH of both LO and HI */ \
	HADDR64_LO_SET((haddr64), HADDR64_LO_LTOH(haddr64)); \
	HADDR64_HI_SET((haddr64), HADDR64_HI_LTOH(haddr64)); \
})

/* PCIe IPC is LE: Perform an htol32 on value to be assigned */
#define HADDR64_LO_SET_HTOL(haddr64, u32) \
	HADDR64_LO_SET((haddr64), htol32(u32))
#define HADDR64_HI_SET_HTOL(haddr64, u32) \
	HADDR64_HI_SET((haddr64), htol32(u32))

/* Used for consistent debug printing of hi and lo fields in native HostOrder */
#define HADDR64_FMT                         " haddr64 lo 0x%08x hi 0x%08x"
#define HADDR64_VAL(haddr64)      HADDR64_LO(haddr64), HADDR64_HI(haddr64)
#define HADDR64_VAL_LTOH(haddr64) HADDR64_LO_LTOH(haddr64), HADDR64_HI_LTOH(haddr64)

#define HADDR64_ZERO(haddr64) \
({ \
	HADDR64_LO_SET((haddr64), 0U); \
	HADDR64_HI_SET((haddr64), 0U); \
})

#define HADDR64_IS_ZERO(haddr64) \
	((HADDR64_LO(haddr64) | HADDR64_HI(haddr64)) == 0U)

/**
 * Total TxPost flowrings for bcast/mcast and ucast. TxPost flowrings are
 * considered as dynamic h2d rings (created by explicit create request messages.
 * May be overridden by 43xxxxx-roml.mk.
 */
#if !defined(BCMPCIE_MAX_TX_FLOWS)
#define BCMPCIE_MAX_TX_FLOWS	40
#endif /* ! BCMPCIE_MAX_TX_FLOWS */

/*
 * XXX
 *
 * PCIE IPC Revision(Version) History:
 *
 * BISON packages (DHD363, 7.35 and 10.10) use legacy PCIE IPC revision 5.
 * KUDU packages: Firmware and DHD use "BCA revision space [0x80 .. 0xFF]"
 *
 * BCA Revisions set the msbit in the 8bit revision to seperate the revision
 * space from WCC revisions. First BCA revision will begin at 0x81.
 *
 * In BCA revisions, structures such as pciedev_shared_t and ring_info_t are
 * extended and feature flags and capabilities are not compatible with WCC
 * revisions 6 and 7, and under "Work In Progress" scrubbing.
 *
 * All BCA PCIE IPC structures, flags, etc will use a
 * prefix "pcie_ipc_" or "PCIE_IPC_" for clear identification of objects that
 * may be accessed by host DHD or dongle firmware.
 *
 * DHD from KUDU must interwork with firmware from 7.35 and 10.10 that are
 * retained at legacy PCIE IPC revision 0x05. Legacy revision 0x05 is outside
 * the BCA revision space, and uses legacy structure and flags definitions.
 *
 * PCIe IPC: BCA Revisions will use sequence 0x81, 0x82, ...
 *           Major number is BCA_REV and minor number is VERSION.
 *
 * Fields tagged as deprecated may be repurposed. Retain deprecated symbols as
 * they may be referenced in released twigs that share PROTO_BRANCH_17_100.
 * Use an anonymous union { uint32 deprecated_sym; uint32 repurposed_sym; };
 *
 */

/**
 * Revision History:
 *
 * 0x81 : BCA(Kudu) baseline
 * 0x82 : Host Memory Extension Service
 */

/** PCIe IPC: BCA Revisions will use sequence 0x81, 0x82, ... */
#define PCIE_IPC_BCA_REV                0x80 /* msbit=1 in BCA revision base */

/** Makefile or chipset.mk may override PCIE_IPC_VERSION */
#ifndef PCIE_IPC_VERSION
#define PCIE_IPC_VERSION                2
#endif // endif

#define PCIE_IPC_REVISION               (PCIE_IPC_BCA_REV | PCIE_IPC_VERSION)

/** Legacy Dongle Host Driver revision */
#define PCIE_IPC_DEFAULT_HOST_REVISION  5

#define PCIE_IPC_REV_MASK               0x000000FF
#define PCIE_IPC_REV_GET(u32)           ((u32) & PCIE_IPC_REV_MASK)
#define PCIE_IPC_REV_SET(u32, rev) \
	(u32) = ((u32) & ~PCIE_IPC_REV_MASK) | ((rev) & PCIE_IPC_REV_MASK)
#define PCIE_IPC_REV_IS_BCA(u32)        ((u32) & PCIE_IPC_BCA_REV)

#define PCIE_IPC_VER_MASK               (PCIE_IPC_BCA_REV ^ PCIE_IPC_REV_MASK)
#define PCIE_IPC_VER_GET(u32)           ((u32) & PCIE_IPC_VER_MASK)

#define PCIE_IPC_EXTN_PRESENT(rev)      ((rev) & PCIE_IPC_BCA_REV)

#if (PCIE_IPC_REVISION > PCIE_IPC_REV_MASK)
#error "Invalid PCIE_IPC_REVISION"
#endif // endif

/** pcie_ipc::flags listing begin, use prefix "PCIE_IPC_FLAGS_" */
#define PCIE_IPC_FLAGS_REVISION_MASK    0x000000FF /* Dongle IPC revision     */
#define PCIE_IPC_FLAGS_ASSERT_BUILT     0x00000100 /* Dongle compiled +assert */
#define PCIE_IPC_FLAGS_ASSERT           0x00000200 /* Dongle records asserts  */
#define PCIE_IPC_FLAGS_TRAP             0x00000400 /* Dongle collects trap    */
#define PCIE_IPC_FLAGS_UNUSED_00000800  0x00000800
#define PCIE_IPC_FLAGS_UNUSED_00001000  0x00001000
#define PCIE_IPC_FLAGS_UNUSED_00002000  0x00002000
#define PCIE_IPC_FLAGS_UNUSED_00004000  0x00004000
#define PCIE_IPC_FLAGS_UNUSED_00008000  0x00008000
#define PCIE_IPC_FLAGS_DMA_INDEX        0x00010000 /* Dongle DMAs all indices */
#define PCIE_IPC_FLAGS_D2H_SYNC_SEQNUM  0x00020000 /* mod253-SeqNum chkd2hdma */
#define PCIE_IPC_FLAGS_D2H_SYNC_XORCSUM 0x00040000 /* XOR csum based d2h sync */
#define PCIE_IPC_FLAGS_IDLE_FLOW_RING   0x00080000 /* Idle Flowring Mgmt      */
#define PCIE_IPC_FLAGS_2BYTE_INDICES    0x00100000 /* 2B indices              */
#define PCIE_IPC_FLAGS_DHDHDR           0x00200000 /* Host assisted LLCSNAP   */
#define PCIE_IPC_FLAGS_MAC_D11TOD3      0x00400000 /* RxSplitMode4 .11 to .3  */
#define PCIE_IPC_FLAGS_NO_TXPOST_CWI32  0x00800000 /* Dongle does't support CWI32, force to CWI64 */
#define PCIE_IPC_FLAGS_HOSTRDY_SUPPORT  0x10000000 /* Host ready-> PCIH2D_DB1 */

/* When "dmaindex16" is not used, fw must use either "chkd2hdma" or "xorcsum" */
#define PCIE_IPC_FLAGS_D2H_SYNC_MODE_MASK \
	(PCIE_IPC_FLAGS_D2H_SYNC_SEQNUM | PCIE_IPC_FLAGS_D2H_SYNC_XORCSUM)
/** pcie_ipc::flags end of listing ------------------------------------------ */

/** Dongle advertizes capabilities and host ACKs in hcap1 them */

/** pcie_ipc:dcap1 : Dongle capabilities advertized to host "PCIE_IPC_DCAP1_" */
/* 0x00000001 .. 0x00008000 may be used as flags ... no host ACK required     */
#define PCIE_IPC_DCAP1_FAST_DELETE_RING 0x00010000 /* Fast Flowring Delete    */
#define PCIE_IPC_DCAP1_ACWI             0x00020000 /* PCIE IPC ACWI Capable   */
#define PCIE_IPC_DCAP1_IDMA             0x00040000 /* Implicit DMA            */
#define PCIE_IPC_DCAP1_HOST_MEM_EXTN    0x00080000 /* Host Memory Extn IPC    */
#define PCIE_IPC_DCAP1_MSI_MULTI_MSG    0x00100000 /* WIP: BCMPCIE_D2H_MSI    */
#define PCIE_IPC_DCAP1_HOSTFLOWCONTROL  0x00200000 /* WIP: Host Flow Control  */
#define PCIE_IPC_DCAP1_HWA_RXCPL4       0x00400000 /* HWA HWA_RXCPL4          */
#define PCIE_IPC_DCAP1_FLOWRING_TID     0x00800000 /* Ucast Flowrings per TID */
#define PCIE_IPC_DCAP1_HWA_RXPOST_IDMA  0x01000000 /* HWA RX POST IDMA        */
#define PCIE_IPC_DCAP1_HWA_TXCPL_IDMA   0x02000000 /* HWA TXCPL IDMA          */
#define PCIE_IPC_DCAP1_HWA_RXCPL_IDMA   0x04000000 /* HWA RXCPL IDMA          */
#define PCIE_IPC_DCAP1_HWA_TXPOST       0x08000000 /* HWA TXPOST              */
#define PCIE_IPC_DCAP1_HWA_PKTPGR       0x10000000 /* HWA PKTPGR              */
#define PCIE_IPC_DCAP1_NO_OOB_DW        0x20000000 /* WIP: No device wake DS  */
#define PCIE_IPC_DCAP1_INBAND_DS        0x40000000 /* WIP: Inband DS protocol */
#define PCIE_IPC_DCAP1_DAR              0x80000000 /* WIP: Use DAR Registers  */
/** pcie_ipc:dcap1 end of listing ------------------------------------------- */

/** pcie_ipc:dcap2 : Dongle capabilities advertized to host "PCIE_IPC_DCAP2_" */
#define PCIE_IPC_DCAP2_CSI_MONITOR      0x00000001 /* Ch Status Info support  */
/** pcie_ipc:dcap2 end of listing ------------------------------------------- */

/** pcie_ipc:hcap1 : Host capabillities acked to dongle "PCIE_IPC_HCAP1_"     */
#define PCIE_IPC_HCAP1_REVISION_MASK    0x000000FF /* Host IPC Revision       */
#define PCIE_IPC_HCAP1_ADDR64           0x00000100 /* Full 64b addressing     */
#define PCIE_IPC_HCAP1_HW_COHERENCY     0x00000200 /* HW coherency capable    */
#define PCIE_IPC_HCAP1_LIMIT_BL         0x00000400 /* Limit dma burst length  */
	                                               /* Host Capabilities ACKed */
#define PCIE_IPC_HCAP1_FAST_DELETE_RING PCIE_IPC_DCAP1_FAST_DELETE_RING
#define PCIE_IPC_HCAP1_ACWI             PCIE_IPC_DCAP1_ACWI
#define PCIE_IPC_HCAP1_IDMA             PCIE_IPC_DCAP1_IDMA
#define PCIE_IPC_HCAP1_HOST_MEM_EXTN    PCIE_IPC_DCAP1_HOST_MEM_EXTN
#define PCIE_IPC_HCAP1_MSI_MULTI_MSG    PCIE_IPC_DCAP1_MSI_MULTI_MSG   /* WIP */
#define PCIE_IPC_HCAP1_HOSTFLOWCONTROL  PCIE_IPC_DCAP1_HOSTFLOWCONTROL /* WIP */
#define PCIE_IPC_HCAP1_HWA_RXCPL4       PCIE_IPC_DCAP1_HWA_RXCPL4
#define PCIE_IPC_HCAP1_FLOWRING_TID     PCIE_IPC_DCAP1_FLOWRING_TID
#define PCIE_IPC_HCAP1_HWA_RXPOST_IDMA  PCIE_IPC_DCAP1_HWA_RXPOST_IDMA
#define PCIE_IPC_HCAP1_HWA_TXCPL_IDMA   PCIE_IPC_DCAP1_HWA_TXCPL_IDMA
#define PCIE_IPC_HCAP1_HWA_RXCPL_IDMA   PCIE_IPC_DCAP1_HWA_RXCPL_IDMA
#define PCIE_IPC_HCAP1_HWA_TXPOST       PCIE_IPC_DCAP1_HWA_TXPOST
#define PCIE_IPC_HCAP1_HWA_PKTPGR       PCIE_IPC_DCAP1_HWA_PKTPGR
#define PCIE_IPC_HCAP1_NO_OOB_DW        PCIE_IPC_DCAP1_NO_OOB_DW       /* WIP */
#define PCIE_IPC_HCAP1_INBAND_DS        PCIE_IPC_DCAP1_INBAND_DS       /* WIP */
#define PCIE_IPC_HCAP1_DAR              PCIE_IPC_DCAP1_DAR             /* WIP */
/** pcie_ipc:hcap1 end of listing ------------------------------------------- */

/** pcie_ipc:hcap2 : Host capabilities acked to dongle "PCIE_IPC_HCAP2_"      */
#define PCIE_IPC_HCAP2_CSI_MONITOR      PCIE_IPC_DCAP2_CSI_MONITOR
/** pcie_ipc:hcap2 end of listing ------------------------------------------- */

/**
 * PCIE IPC includes a set of "common" and "dynamic" message rings in the H2D
 * and D2H direction.
 *
 * Specification of per ring message formats is in bcmmsguf.h. All messages in
 * a ring are fixed size. Rings are placed in host memory and use a producer
 * consumer paradigm, with a WR and RD index.
 */
#define BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT      0
#define BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT       1
#define BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE    2
#define BCMPCIE_D2H_MSGRING_TX_COMPLETE         3
#define BCMPCIE_D2H_MSGRING_RX_COMPLETE         4

/**
 * For HWA_RXCPL4 features, driver will have 4 rxcpln ring at the same time.
 * Enlarge the BCMPCIE_COMMON_MSGRING_MAX_ID, BCMPCIE_D2H_COMMON_MSGRINGS, and
 * BCMPCIE_COMMON_MSGRINGS.
 */
#define BCMPCIE_COMMON_MSGRING_MAX_ID           7
#define BCMPCIE_H2D_COMMON_MSGRINGS             2
#define BCMPCIE_D2H_COMMON_MSGRINGS             6
#define BCMPCIE_COMMON_MSGRINGS                 8

#define BCMPCIE_H2D_MSGRINGS(max_tx_flows) \
	(BCMPCIE_H2D_COMMON_MSGRINGS + (max_tx_flows))

/** (Rev7) different ring types */
#define BCMPCIE_H2D_RING_TYPE_CTRL_SUBMIT       0x1
#define BCMPCIE_H2D_RING_TYPE_TXFLOW_RING       0x2
#define BCMPCIE_H2D_RING_TYPE_RXBUFPOST         0x3
#define BCMPCIE_H2D_RING_TYPE_TXSUBMIT          0x4
#define BCMPCIE_H2D_RING_TYPE_DBGBUF_SUBMIT     0x5
#define BCMPCIE_H2D_RING_TYPE_BTLOG_SUBMIT      0x6

#define BCMPCIE_D2H_RING_TYPE_CTRL_CPL          0x1
#define BCMPCIE_D2H_RING_TYPE_TX_CPL            0x2
#define BCMPCIE_D2H_RING_TYPE_RX_CPL            0x3
#define BCMPCIE_D2H_RING_TYPE_DBGBUF_CPL        0x4
#define BCMPCIE_D2H_RING_TYPE_AC_RX_COMPLETE    0x5
#define BCMPCIE_D2H_RING_TYPE_BTLOG_CPL         0x6

/**
 * H2D and D2H, WR and RD index, are maintained in the following arrays:
 * - Array of all H2D WR Indices
 * - Array of all H2D RD Indices
 * - Array of all D2H WR Indices
 * - Array of all D2H RD Indices
 *
 * The offset of the WR or RD indexes (for common rings) in these arrays are
 * listed below. Arrays ARE NOT indexed by a ring's id.
 *
 * D2H common rings WR and RD index start from 0, even though their ringids
 * start from BCMPCIE_H2D_COMMON_MSGRINGS
 */

#define BCMPCIE_H2D_RING_IDX(h2d_ring_id) (h2d_ring_id)

enum h2dring_idx {
	/* H2D common rings */
	BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT_IDX =
		BCMPCIE_H2D_RING_IDX(BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT),
	BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT_IDX =
		BCMPCIE_H2D_RING_IDX(BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT),

	/* First TxPost's WR or RD index starts after all H2D common rings */
	BCMPCIE_H2D_MSGRING_TXFLOW_IDX_START =
		BCMPCIE_H2D_RING_IDX(BCMPCIE_H2D_COMMON_MSGRINGS)
};

#define BCMPCIE_D2H_RING_IDX(d2h_ring_id) \
	((d2h_ring_id) - BCMPCIE_H2D_COMMON_MSGRINGS)

enum d2hring_idx {
	/* D2H Common Rings */
	BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE_IDX =
		BCMPCIE_D2H_RING_IDX(BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE),
	BCMPCIE_D2H_MSGRING_TX_COMPLETE_IDX =
		BCMPCIE_D2H_RING_IDX(BCMPCIE_D2H_MSGRING_TX_COMPLETE),
	BCMPCIE_D2H_MSGRING_RX_COMPLETE_IDX =
		BCMPCIE_D2H_RING_IDX(BCMPCIE_D2H_MSGRING_RX_COMPLETE)
};

/**
 * Macros for managing arrays of RD WR indices:
 * rw_index_sz:
 *    - in dongle, rw_index_sz is known at compile time
 *    - in host/DHD, rw_index_sz is derived from advertized pci_shared flags
 *
 *  ring_idx: See h2dring_idx and d2hring_idx
 */

/** Offset of a RD or WR index in H2D or D2H indices array */
#define BCMPCIE_RW_INDEX_OFFSET(rw_index_sz, ring_idx) \
	((rw_index_sz) * (ring_idx))

/** Fetch the address of RD or WR index in H2D or D2H indices array */
#define BCMPCIE_RW_INDEX_ADDR(indices_array_base, rw_index_sz, ring_idx) \
	(void *)((uint32)(indices_array_base) + \
	BCMPCIE_RW_INDEX_OFFSET((rw_index_sz), (ring_idx)))

/** H2D DMA Indices array size: given max flow rings */
#define BCMPCIE_H2D_RW_INDEX_ARRAY_SZ(rw_index_sz, max_tx_flows) \
	((rw_index_sz) * BCMPCIE_H2D_MSGRINGS(max_tx_flows))

/** D2H DMA Indices array size */
#define BCMPCIE_D2H_RW_INDEX_ARRAY_SZ(rw_index_sz) \
	((rw_index_sz) * BCMPCIE_D2H_COMMON_MSGRINGS)

/**
 * pcie_ipc_ring_mem describes one circular buffer used in PCIE IPC messaging.
 * A pcie_ipc_ring_mem may be used to describe a common ring or a dynamic ring
 * like flowring, debug, or info ring.
 *
 * The actual circular buffer is placed in host memory and a pcie_ipc_ring_mem
 * will be maintained in both host and dongle to describe this circular buffer.
 *
 * Dongle uses mem2mem DMA transfers to:
 * - fetch messages produced by host in H2D rings
 * - post messages produced by dongle in D2H rings
 */
typedef struct pcie_ipc_ring_mem
{
	uint16      id;        /* ring id */
	uint8       type;      /* ring type */
	uint8       item_type; /* Item format : WI86, CWI/ACWI 32/64 */
	uint16      max_items; /* Max number of items in ring */
	uint16      item_size; /* Length in bytes of one work item */
	haddr64_t   haddr64;   /* 64 bits address, in host memory */
} pcie_ipc_ring_mem_t;

/**
 * Specifications of the PCIE IPC messaging rings
 */
typedef struct pcie_ipc_rings
{
	struct { /* Fields used upto PCIE IPC rev5 */

		/* List of well know dongle addresses */
		daddr32_t   ring_mem_daddr32; /* Array of pcie_ipc_ring_mem */

		/* Following arrays are indexed using h2dring_idx and d2hring_idx */

		/* Dongle address of arrays of WR or RD indices for all rings */
		daddr32_t   h2d_wr_daddr32; /* Array of all H2D ring's WR indices */
		daddr32_t   h2d_rd_daddr32; /* Array of all H2D ring's RD indices */
		daddr32_t   d2h_wr_daddr32; /* Array of all D2H ring's WR indices */
		daddr32_t   d2h_rd_daddr32; /* Array of all D2H ring's RD indices */

		/* Host address of arrays of WR or RD indices for all rings */
		haddr64_t   h2d_wr_haddr64; /* Array of all H2D ring's WR indices */
		haddr64_t   h2d_rd_haddr64; /* Array of all H2D ring's RD indices */
		haddr64_t   d2h_wr_haddr64; /* Array of all D2H ring's WR indices */
		haddr64_t   d2h_rd_haddr64; /* Array of all D2H ring's RD indices */

		/* Max H2D(submission) and D2H(completion) rings */
		uint16      max_h2d_rings;  /* Max H2D rings: Common + Dynamic */
		uint16      max_d2h_rings;  /* Max D2H rings: Common + Dynamic */
	};

	struct { /* New fields post Rev 5 */

		union {
			struct {
				/* Split of TxPost Flowrings for bcast+mcast and ucast */
				uint16  max_flowrings;  /* Max TxPost flowrings: bcmc + ucast */
				uint16  max_interfaces; /* Max virtual ifs: bcmc flowrings */
			};
			uint32  post_rev5_extn;
		};

		union {
			struct { /* Work Item format: 0 = legacy, 1 = Compact, >1 = ACWI */
				uint8   txpost_format; /* H2D Tx Post WI format */
				uint8   rxpost_format; /* H2D Rx Post WI format */
				uint8   txcpln_format; /* D2H Tx Completion WI format */
				uint8   rxcpln_format; /* D2H Rx Completion WI format */
			};
			uint32  wi_formats;
		};

		/* RxPost and RxCompletion : use ~0 for undefined */
		uint16      rxpost_data_buf_len; /* Fixed length of Rx buffers posted */
		uint16      rxcpln_dataoffset; /* RxSM4: fixed dataoffset for .3 Rx */

		/* Array of all H2D ring's WR indices for IFRM */
		haddr64_t   ifrm_wr_haddr64;

		/* future proof */
		uint32      PAD[13];           /* padded to 32 Words, 128 Bytes */

	}; /* Extension post rev 5 */

} pcie_ipc_rings_t;

/** Rev5 pcie_ipc_rings_t size is limited to (14 x 4Bytes) */
#define PCIE_IPC_RINGS_REV5_SZ      (OFFSETOF(pcie_ipc_rings_t, post_rev5_extn))

/** Post Rev5. pcie_ipc_rings_t is extended and future proofed */
#define PCIE_IPC_RINGS_SZ           (sizeof(pcie_ipc_rings_t)) /* 128 Bytes */

/**
 * PCIE IPC Specifications of Host Memory Extension (HME):
 * -------------------------------------------------------
 *
 * Host memory is treated as an extension of dongle and must NOT be accessed
 * by host CPU complex, upon serving it to the Dongle.
 * Dongle may access the extended memory via non hw coherent transactions.
 * Dongle may directly access over backplane (SBTOPCIE) or utilize available
 * DMA engines, such as, asynchronous mem2mem or synchronous BME.
 *
 * Host Memory Extension management is in bcmhme.h .
 *
 * Implementation Note:
 * --------------------
 * Upto 8 users may request for host memory extensions, in 4 KByte page units.
 * Each user's host memory extension is a discrete region, whose size is
 * bounded (4 MByte - 1 HME page unit) - Linux based DHD.
 *
 * Interface Note:
 * ---------------
 * PCIE_IPC_DCAP1_HOST_MEM_EXTN dongle capability flag in PCIE IPC revision 0x82
 * informs DHD of HME Service. Request and Response information is handshaked
 * using the structure pcie_ipc's fields hme_pages and host_mem_haddr64. When
 * the HME dongle capability is not set, the host_mem<haddr64,len> tuple is used
 * to specify a single contiguous scratch buffer to accommodate legacy users.
 *
 * Theory of Operation:
 * --------------------
 * Dongle Users request host memory region(s) in HME Page units (4 KBytes).
 *   - pcie_ipc::hme_pages[8]     : Each user requests a discrete memory region
 *                                  specified in HME page units. A non-zero
 *                                  value also serves as a dongle capability
 *                                  being enabled.
 *
 * DHD allocates host memory regions and advertizes a table of host memory
 * extension physical addresses. Table address is placed in host_mem_haddr64.
 *   - pcie_ipc::host_mem_haddr64 : Location of a table of haddr64_t describing
 *                                  physical addresses of User extension region.
 *                                  Host memory extension table is in host.
 *                                  Table is indexed by HME User ID.
 *   - pcie_ipc::host_mem_len     : Host reports the sum total bytes of all host
 *                                  memory extensions. This is NOT the size of
 *                                  the Table of HME addresses.
 * In DHD, a host memory extension is represented as a dhd_dma_buf_t abstract
 * data type. A DHD DMA Buffer is aligned onto a cacheline (or 64 Bytes) and
 * is padded to ensure cacheline boundary. While a DHD DMA buffer may be
 * allocated in coherent memory, the HOST CPU complex does not directly access
 * a HME. On a fatal firmware trap, a dump of Host Memory Extensions may be
 * introduced in DHD.
 *
 * Miscellaneous Caveats:
 * ----------------------
 * + User listing is in bcm_hme.h (abstracted from DHD)
 * + Users HTXHDR and PKTPGR are mutually exclusive
 * + Users PKTPGR and MACIFS are listed as discrete Users allowing individual
 *   memory segments to grow up to HME PAGES MAX (future proofing).
 * + Merge Users LCLPKT and PSQPKT to save host memory? Discrete Users for now.
 * + A host memory extension may not wrap over a 32b boundary (i.e. haddr64.hi
 *   must be the same for the entire region).
 *   See dhd_dma_buf_t abstract data type, and dhd_dma_buf_audit().
 *
 */

/**< Maximum HME Users supported. HME User Listing in bcm_hme.h */
#define PCIE_IPC_HME_USERS_MAX      (8)

/** HME Page Size Unit : "number of pages" is represented as an uint16.       */
#define PCIE_IPC_HME_PAGE_SIZE      (4 * 1024) /**< 4 KBytes page units       */

/** Maximum size in Bytes of a host memory extension is (4 MBytes - 4 KBytes) */
#define PCIE_IPC_HME_BYTES_MAX      ((4 * 1024 * 1024) - PCIE_IPC_HME_PAGE_SIZE)

/** Maximum size in pages of a host memory extension (1023 pages) */
#define PCIE_IPC_HME_PAGES_MAX \
	(PCIE_IPC_HME_BYTES_MAX / PCIE_IPC_HME_PAGE_SIZE)

/** PCIE IPC Bytes to/from Pages conversion */
#define PCIE_IPC_HME_PAGES(bytes)  ((bytes) / PCIE_IPC_HME_PAGE_SIZE)
#define PCIE_IPC_HME_BYTES(pages)  ((pages) * PCIE_IPC_HME_PAGE_SIZE)

/** PCIE IPC HME Host 4bit physical address Table Size */
#define PCIE_IPC_HME_HADDR64_TBLSZ (sizeof(haddr64_t) * PCIE_IPC_HME_USERS_MAX)

/**
 * PCIE IPC structure located in dongle's memory.
 *
 * Dongle and Host SW uses this structure to exchange information during initial
 * handshake, to convey PCIE IPC features and capabilities.
 */
typedef struct pcie_ipc
{
	struct /* Rev 5 PCIE IPC shared structure */
	{
		uint32      flags; /**< PCIE IPC revision captured in flags 7:0 */

		daddr32_t   trap_daddr32;

		daddr32_t   assert_exp_daddr32;
		daddr32_t   assert_file_daddr32;
		uint32      assert_line;

		daddr32_t   console_daddr32; /**< address of hnd_cons_t */

		union {
			uint32  msgtrace_daddr32; /**< deprecated: retain symbol */
			uint32  PAD; /**< may be repurposed */
		};

		uint32      fwid;

		uint16      max_tx_pkts; /**< Max Tx packets in dongle */
		uint16      max_rx_pkts; /**< Max Rx packets in dongle */

		union {
			uint32  dma_rxoffset; /**< deprecated: retain symbol */
			uint32  PAD; /**< may be repurposed */
		};

		daddr32_t   h2d_mb_daddr32; /**< location of h2d mailbox in dongle */
		daddr32_t   d2h_mb_daddr32; /**< location of h2d mailbox in dongle */

		daddr32_t   rings_daddr32; /**< rings info and per ring mem in dongle */

		/** host memory extension(s) of dongle memory, used to be scratch mem */
		uint32      host_mem_len; /**< sum total bytes of all HME serviced */
		haddr64_t   host_mem_haddr64; /**< address of table of HME addresses */

		uint32      PAD[3];

		daddr32_t   buzzz_daddr32; /**< BUZZZ structure in dongle memory */
	}; /* 20 Words, 80 Bytes */

	struct /* Post Rev5 PCIE IPC extension */
	{
		union {
			uint32  post_rev5_extn;
			uint32  dcap1; /**< dongle advertized capabilities */
		};
		uint32      dcap2; /**< dongle advertized capabilities */

		uint32      hcap1; /**< host acknowledged capabilities */
		uint32      hcap2; /**< host acknowledged capabilities */

		uint32      host_physaddrhi; /**< fixed host hi32 address CWI */

		union {
			uint32  fatal_logbuf_daddr32; /**< deprecated: retain symbol */
			uint32  PAD; /**< may be repurposed */
		};
		uint32      PAD[2];

		uint16      hme_pages[PCIE_IPC_HME_USERS_MAX]; /**< req in HME pages */

	}; /* 12 Words, 48 Bytes */

} pcie_ipc_t; /* 32 Words, 128 Bytes */

/** Rev5 pcie_ipc_t size is limited to (14 x 4Bytes) */
#define PCIE_IPC_REV5_SZ        (OFFSETOF(pcie_ipc_t, post_rev5_extn))

/** Post Rev5. pcie_ipc_t is extended and future proofed with reserved words */
#define PCIE_IPC_SZ             (sizeof(pcie_ipc_t)) /* 128 Bytes */

/**
 * Feature specific parameters
 *
 * Configurations that may change based upon dongle capabilities need to be
 * handshaked via the IPC, or will be applied equally to all dongle devices.
 */

/** BCM_DHDHDR: DHD can offload a maximum of 3072 * 256 Byte Txheaders. */
#define BCM_DHDHDR_MAXIMUM          3072
#define BCM_DHDHDR_SIZE             256

/** Size of Extended Trap data Buffer */
#define BCMPCIE_EXT_TRAP_DATA_MAXLEN  4096

/* Trap types copied in the pcie_ipc.trap_daddr32 */
#define	FW_INITIATED_TRAP_TYPE	(0x1 << 7)
#define	HEALTHCHECK_NODS_TRAP_TYPE	(0x1 << 6)

/* H2D mail box Data */
#define PCIE_IPC_H2DMB_HOST_D3_INFORM          0x00000001
#define PCIE_IPC_H2DMB_HOST_DS_ACK             0x00000002
#define PCIE_IPC_H2DMB_HOST_DS_NAK             0x00000004
#define PCIE_IPC_H2DMB_HOST_D0_INFORM_IN_USE   0x00000008
#define PCIE_IPC_H2DMB_HOST_D0_INFORM          0x00000010
#define PCIE_IPC_H2DMB_DS_ACTIVE               0x00000020
#define PCIE_IPC_H2DMB_DS_DEVICE_WAKE          0x00000040
#define PCIE_IPC_H2DMB_HOST_IDMA_INITED        0x00000080
#define PCIE_IPC_H2DMB_HOST_ACK_NOINT          0x00010000
#define PCIE_IPC_H2DMB_FW_TRAP                 0x20000000 /**< h2d force TRAP */
#define PCIE_IPC_H2DMB_HOST_CONS_INT           0x80000000 /**< h2d int for console cmds  */

#define PCIE_IPC_H2DMB_DS_HOST_SLEEP_INFORM    PCIE_IPC_H2DMB_HOST_D3_INFORM
#define PCIE_IPC_H2DMB_DS_DEVICE_SLEEP_ACK     PCIE_IPC_H2DMB_HOST_DS_ACK
#define PCIE_IPC_H2DMB_DS_DEVICE_SLEEP_NAK     PCIE_IPC_H2DMB_HOST_DS_NAK
#define PCIE_IPC_H2DMB_D0_INFORM_IN_USE        PCIE_IPC_H2DMB_HOST_D0_INFORM_IN_USE
#define PCIE_IPC_H2DMB_D0_INFORM               PCIE_IPC_H2DMB_HOST_D0_INFORM
#define PCIE_IPC_H2DMB_DS_DEVICE_WAKE_ASSERT   PCIE_IPC_H2DMB_DS_DEVICE_WAKE
#define PCIE_IPC_H2DMB_DS_DEVICE_WAKE_DEASSERT PCIE_IPC_H2DMB_DS_ACTIVE

/* D2H mail box Data */
#define PCIE_IPC_D2HMB_DEV_D3_ACK              0x00000001
#define PCIE_IPC_D2HMB_DEV_DS_ENTER_REQ        0x00000002
#define PCIE_IPC_D2HMB_DEV_DS_EXIT_NOTE        0x00000004
#define PCIE_IPC_D2HMB_DS_HOST_SLEEP_EXIT_ACK  0x00000008
#define PCIE_IPC_D2HMB_DEV_IDMA_INITED         0x00000010
#define PCIE_IPC_D2HMB_DEV_FWHALT              0x10000000
#define PCIE_IPC_D2HMB_DEV_EXT_TRAP_DATA       0x20000000
#define PCIE_IPC_D2HMB_DS_HOST_SLEEP_ACK       PCIE_IPC_D2HMB_DEV_D3_ACK
#define PCIE_IPC_D2HMB_DS_DEVICE_SLEEP_ENTER_REQ PCIE_IPC_D2HMB_DEV_DS_ENTER_REQ
#define PCIE_IPC_D2HMB_DS_DEVICE_SLEEP_EXIT    PCIE_IPC_D2HMB_DEV_DS_EXIT_NOTE
/* Adding maskbits for TRAP information */
#define PCIE_IPC_D2HMB_FWTRAP_MASK             0x0000001F

#define PCIE_IPC_D2HMB_MASK \
	(PCIE_IPC_D2HMB_DEV_D3_ACK | PCIE_IPC_D2HMB_DEV_DS_ENTER_REQ | \
	 PCIE_IPC_D2HMB_DEV_DS_EXIT_NOTE | PCIE_IPC_D2HMB_DEV_IDMA_INITED | \
	 PCIE_IPC_D2HMB_DEV_FWHALT | PCIE_IPC_D2HMB_FWTRAP_MASK | \
	 PCIE_IPC_D2HMB_DEV_EXT_TRAP_DATA)

/** MACROS for manipulating MsgBuf Rings */
#define PREVTXP(i, d)           (((i) == 0) ? ((d) - 1) : ((i) - 1))
#define NEXTTXP(i, d)           ((((i) + 1) >= (d)) ? 0 : ((i) + 1))
#define NEXTNTXP(i, n, d)       ((((i) + (n)) >= (d)) ? 0 : ((i) + (n)))
#define NTXPACTIVE(r, w, d)     (((r) <= (w)) ? ((w) - (r)) : ((d) - (r) + (w)))
#define NTXPAVAIL(r, w, d)      (((d) - NTXPACTIVE((r), (w), (d))) > 1)

#define READ_AVAIL_SPACE(w, r, d) \
	(((w) >= (r)) ? ((w) - (r)) : ((d) - (r)))

#define WRITE_SPACE_AVAIL(r, w, d) \
	((d) - (NTXPACTIVE((r), (w), (d))) - 1)

#define WRITE_SPACE_AVAIL_CONTINUOUS(r, w, d) \
	(((w) >= (r)) ? ((d) - (w)) : ((r) - (w)))

#define CHECK_WRITE_SPACE(r, w, d) \
	((r) > (w)) ? ((r) - (w) - 1) : \
		((r) == 0 || (w) == 0) ? ((d) - (w) - 1) : ((d) - (w))

#define CHECK_NOWRITE_SPACE(r, w, d) \
	(((r) == (w) + 1) || (((r) == 0) && ((w) == ((d) - 1))))

#define IS_RING_SPACE_EMPTY(r, w, d) \
	((r) == (w))

#define IS_RING_SPACE_FULL(r, w, d) \
	(WRITE_SPACE_AVAIL((r), (w), (d)) == 0)

/*
 * In TCM, the RD and WR indices are saved in individual RD and WR index arrays
 * for the H2D and D2H rings. pcie_rw_index_t is used to reference each RD or
 * WR index, in this array.
 *
 * DHD typically directly updates or fetches an index from dongle's memory.
 *
 * When the PCIE_DMA_INDEX feature is enabled, dongle will use the mem2mem DMA
 * to transfer an entire array of RD or WR indices to/from host memory and DHD
 * may directly access local system memory.
 *
 * Legacy -DPCIE_DMA_INDEX uses 32bit RD and WR index sizes.
 *
 * Dongle build -dmaindex16- or -dmaindex32- selects 16bit or 32bit indices with
 * dongle DMAing of indices. Legacy -DPCIE_DMA_INDEX implies 32bit inidices.
 *
 * Dongle will advertize the size of a WR or RD DMA index to DHD.
 */
/* XXX Use of 32bit indices instead of 16bit, doubles the memory in hardware
 * accelerators (2KBytes instead of 1KBytes for 512 rings), unnecessarily.
 * Internal memory in HW accelerators is extremely scarce.
 */
#if defined(PCIE_DMA_INDEX) || defined(PCIE_DMAINDEX32)
typedef uint32 pcie_rw_index_t; /* 32bit WR and RD indices */
#elif defined(PCIE_DMAINDEX16)
typedef uint16 pcie_rw_index_t; /* 16bit WR and RD indices */
#else
typedef uint32 pcie_rw_index_t; /* 32bit WR and RD indices (default) */
#endif /* ! PCIE_DMA_INDEX32 */

#define PCIE_RW_INDEX_SZ    sizeof(pcie_rw_index_t)

/* Implicit DMA index usage
 *  Set0    : D2H rd_index (CtrlCpl + RxCpl + TxCpl, 3 indices, 6 bytes).
 *  Set1..14: H2D wr_index for 32 indices per Set.
 *  Set15   : H2D wr_index for rest indices if any.
 */
#define BCMPCIE_IDMA_D2H_COMMON_INDEX	0
#define BCMPCIE_IDMA_H2D_COMMON_INDEX	1
#define BCMPCIE_IDMA_FLOWS_PER_SET	32
#define BCMPCIE_IDMA_MAX_DESC		16
#define BCMPCIE_IDMA_H2D_MAX_DESC	BCMPCIE_IDMA_MAX_DESC-1

#define PCIE_IPC_DCAP1_IFRM 0
#define PCIE_IPC_HCAP1_IFRM 0
#define BCMPCIE_IFRM_64_INDEX_PER_GROUP	64

#endif	/* _bcmpcie_h_ */
