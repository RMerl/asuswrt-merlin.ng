/*
 * Broadcom PCIE
 * Software-specific definitions shared between device and host side
 * Explains the shared area between host and dongle
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
 * $Id: bcmpcie.h 832751 2023-11-13 06:52:24Z $
 */

#ifndef _bcmpcie_h_
#define _bcmpcie_h_

#if !defined(_LANGUAGE_ASSEMBLY) && !defined(__ASSEMBLY__)
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
#define HADDR64_LO(_h64_)                   ((_h64_).lo)
#define HADDR64_HI(_h64_)                   ((_h64_).hi)

/* Assign/Set the lo and hi address field without endian conversion. */
#define HADDR64_LO_SET(_h64_, _u32_)        ((_h64_).lo = (_u32_))
#define HADDR64_HI_SET(_h64_, _u32_)        ((_h64_).hi = (_u32_))

/* haddr64_t to/from uint64 construction without endian conversion. */
#define HADDR64_TO_U64(_h64_, _u64_) \
	(_u64_) = (((uint64)(HADDR64_HI(_h64_))) << 32) | (HADDR64_LO(_h64_))

#define HADDR64_FROM_U64(_h64_, _u64_) \
({ \
	HADDR64_LO_SET((_h64_), (uint32)(_u64_)); \
	HADDR64_HI_SET((_h64_), (uint32)((_u64_) >> 32)); \
})

#define HADDR64_SET(_h64_to_, _h64_from_) \
({ \
	HADDR64_LO(_h64_to_) = HADDR64_LO(_h64_from_); \
	HADDR64_HI(_h64_to_) = HADDR64_HI(_h64_from_); \
})

/* PCIe IPC is LE: Perform an ltoh32 on address value retrieved */
#define HADDR64_LO_LTOH(_h64_)              (ltoh32(HADDR64_LO(_h64_)))
#define HADDR64_HI_LTOH(_h64_)              (ltoh32(HADDR64_HI(_h64_)))
#define HADDR64_LTOH(_h64_) \
({ \
	/* in place LTOH of both LO and HI */ \
	HADDR64_LO_SET((_h64_), HADDR64_LO_LTOH(_h64_)); \
	HADDR64_HI_SET((_h64_), HADDR64_HI_LTOH(_h64_)); \
})

/* PCIe IPC is LE: Perform an htol32 on value to be assigned */
#define HADDR64_LO_SET_HTOL(_h64_, _u32_) \
	HADDR64_LO_SET((_h64_), htol32(_u32_))
#define HADDR64_HI_SET_HTOL(_h64_, _u32_) \
	HADDR64_HI_SET((_h64_), htol32(_u32_))

/* Used for consistent debug printing of hi and lo fields in native HostOrder */
#define HADDR64_FMT                         " haddr64 hi 0x%08x lo 0x%08x"
#define HADDR64_VAL(_h64_)      HADDR64_HI(_h64_), HADDR64_LO(_h64_)
#define HADDR64_VAL_LTOH(_h64_) HADDR64_HI_LTOH(_h64_), HADDR64_LO_LTOH(_h64_)
#define HADDR_U64_VAL(_u64_)    (uint32)((_u64_) >> 32), (uint32)(_u64_)

#define HADDR64_ZERO(_h64_) \
({ \
	HADDR64_LO_SET((_h64_), 0U); \
	HADDR64_HI_SET((_h64_), 0U); \
})

#define HADDR64_IS_ZERO(_h64_) \
	((HADDR64_LO(_h64_) | HADDR64_HI(_h64_)) == 0U)

/**
 * Revision History:
 *
 * 0x81 : BCA(Kudu) baseline
 * 0x82 : Host Memory Extension Service
 * 0x83 : HYBRIDFW Host Memory Offload SW_PAGING (HME User HMOSWP)
 * 0x84 : HME support for variable number of Users
 * 0x85 : IOCTL REQ PKTBUFSZ support for large size 2560 Bytes
 * 0x86 : Executable Vector Table Relocation
 * 0x87 : CPU DBG M2M DMA fields added to pcie_ipc struct
 * 0x88 : HME LOWMAC user support
 * 0x89 : HME Upper-MAC Console Log Streaming (UCLS) user support
 * 0x8a : HME HMO BCM EVENT user support
 * 0x8b : pcie_ipc_hme_user_t extension and Host memory for MLOIPC
 */

/** PCIe IPC: BCA Revisions will use sequence 0x81, 0x82, ... */
#define PCIE_IPC_BCA_REV                0x80 /* msbit=1 in BCA revision base */

/** Makefile or chipset.mk may override PCIE_IPC_VERSION */
#ifndef PCIE_IPC_VERSION
#define PCIE_IPC_VERSION                0xB
#endif

#define PCIE_IPC_REVISION               (PCIE_IPC_BCA_REV | PCIE_IPC_VERSION)

/** Legacy Dongle Host Driver revision */
#define PCIE_IPC_DEFAULT_HOST_REVISION  5

#define PCIE_IPC_REV_MASK               0x000000FF
#define PCIE_IPC_REV_GET(_u32_)         ((_u32_) & PCIE_IPC_REV_MASK)
#define PCIE_IPC_REV_SET(_u32_, _rev_) \
	(_u32_) = ((_u32_) & ~PCIE_IPC_REV_MASK) | ((_rev_) & PCIE_IPC_REV_MASK)
#define PCIE_IPC_REV_IS_BCA(_u32_)      ((_u32_) & PCIE_IPC_BCA_REV)

#define PCIE_IPC_VER_MASK               (PCIE_IPC_BCA_REV ^ PCIE_IPC_REV_MASK)
#define PCIE_IPC_VER_GET(_u32_)         ((_u32_) & PCIE_IPC_VER_MASK)

#define PCIE_IPC_EXTN_PRESENT(_rev_)    ((_rev_) & PCIE_IPC_BCA_REV)

#if (PCIE_IPC_REVISION > PCIE_IPC_REV_MASK)
#error "Invalid PCIE_IPC_REVISION"
#endif

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

/* When PCIE_DMA_INDEX is not used, must use either "chkd2hdma" or "xorcsum" */
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
#define PCIE_IPC_DCAP2_PCIE_HBQD        0x00000002 /* Host backup queue depth */
#define PCIE_IPC_DCAP2_RX_CMPL_V2       0x00000004 /* 0.11 header tid in rx WI */
#define PCIE_IPC_DCAP2_RX_CMPL_RCH      0x00000008 /* RxCpl Host info of PDU in headroom  */
#define PCIE_IPC_DCAP2_MAC_CPU_DBG      0x00000010 /* MAC CPU DBG             */
#define PCIE_IPC_DCAP2_BA256_CFG	0x00000020 /* Deep flow ring/rxpost ring for BA256 */
#define PCIE_IPC_DCAP2_UCLS_SW          0x00000040 /* UCLS supported by SW    */
#define PCIE_IPC_DCAP2_UCLS_HW          0x00000080 /* UCLS supported by HW    */
#define PCIE_IPC_DCAP2_IDMA64           0x00000100 /* Implicit DMA FRG_ID 64 */
#define PCIE_IPC_DCAP2_AIRIQ            0x00000200 /* AirIQ support          */
#define PCIE_IPC_DCAP2_HMO_EVENT        0x00000400 /* HMO EVENT supported */
#define PCIE_IPC_DCAP2_PCAP             0x00000800 /* Packet Capture support */
#define PCIE_IPC_DCAP2_PCAPXF           0x00001000 /* Packet Capture hdr xfer */
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
#define PCIE_IPC_HCAP2_PCIE_HBQD        PCIE_IPC_DCAP2_PCIE_HBQD
#define PCIE_IPC_HCAP2_RX_CMPL_V2       PCIE_IPC_DCAP2_RX_CMPL_V2
#define PCIE_IPC_HCAP2_RX_CMPL_RCH      PCIE_IPC_DCAP2_RX_CMPL_RCH
#define PCIE_IPC_HCAP2_BA256_CFG        PCIE_IPC_DCAP2_BA256_CFG
#define PCIE_IPC_HCAP2_UCLS_SW          PCIE_IPC_DCAP2_UCLS_SW
#define PCIE_IPC_HCAP2_UCLS_HW          PCIE_IPC_DCAP2_UCLS_HW
#define PCIE_IPC_HCAP2_IDMA64           PCIE_IPC_DCAP2_IDMA64
#define PCIE_IPC_HCAP2_AIRIQ            PCIE_IPC_DCAP2_AIRIQ
#define PCIE_IPC_HCAP2_HMO_EVENT        PCIE_IPC_DCAP2_HMO_EVENT
#define PCIE_IPC_HCAP2_PCAP             PCIE_IPC_DCAP2_PCAP
#define PCIE_IPC_HCAP2_PCAPXF           PCIE_IPC_DCAP2_PCAPXF
/** pcie_ipc:hcap2 end of listing ------------------------------------------- */

/**
 * Total TxPost flowrings for bcast/mcast and ucast. TxPost flowrings are
 * considered as dynamic h2d rings (created by explicit create request messages.
 * May be overridden by 43xxxxx-roml.mk.
 */
#if !defined(BCMPCIE_MAX_TX_FLOWS)
#define BCMPCIE_MAX_TX_FLOWS                    40
#endif /* ! BCMPCIE_MAX_TX_FLOWS */

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

#define BCMPCIE_H2D_MSGRINGS(_max_tx_flows_) \
	(BCMPCIE_H2D_COMMON_MSGRINGS + (_max_tx_flows_))

/** (Rev7) different ring types */
#define BCMPCIE_H2D_RING_TYPE_CTRL_SUBMIT       0x1
#define BCMPCIE_H2D_RING_TYPE_TXFLOW_RING       0x2
#define BCMPCIE_H2D_RING_TYPE_RXBUFPOST         0x3
#define BCMPCIE_H2D_RING_TYPE_TXSUBMIT          0x4
#define BCMPCIE_H2D_RING_TYPE_DBGBUF_SUBMIT     0x5
#define BCMPCIE_H2D_RING_TYPE_BTLOG_SUBMIT      0x6
#define BCMPCIE_H2D_RING_TYPE_PCAP_SUBMIT       0x7

#define BCMPCIE_D2H_RING_TYPE_CTRL_CPL          0x1
#define BCMPCIE_D2H_RING_TYPE_TX_CPL            0x2
#define BCMPCIE_D2H_RING_TYPE_RX_CPL            0x3
#define BCMPCIE_D2H_RING_TYPE_DBGBUF_CPL        0x4
#define BCMPCIE_D2H_RING_TYPE_AC_RX_COMPLETE    0x5
#define BCMPCIE_D2H_RING_TYPE_BTLOG_CPL         0x6
#define BCMPCIE_D2H_RING_TYPE_PCAP_CPL          0x7

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

#define BCMPCIE_H2D_RING_IDX(_h2d_ring_id_) (_h2d_ring_id_)

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

#define BCMPCIE_D2H_RING_IDX(_d2h_ring_id_) \
	((_d2h_ring_id_) - BCMPCIE_H2D_COMMON_MSGRINGS)

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
#define BCMPCIE_RW_INDEX_OFFSET(_rw_index_sz_, _ring_idx_) \
	((_rw_index_sz_) * (_ring_idx_))

/** Fetch the address of RD or WR index in H2D or D2H indices array */
#define BCMPCIE_RW_INDEX_ADDR(_indices_array_base_, _rw_index_sz_, _ring_idx_) \
	(void *)((uint32)(_indices_array_base_) + \
	BCMPCIE_RW_INDEX_OFFSET((_rw_index_sz_), (_ring_idx_)))

/** H2D DMA Indices array size: given max flow rings */
#define BCMPCIE_H2D_RW_INDEX_ARRAY_SZ(_rw_index_sz_, _max_tx_flows_) \
	((_rw_index_sz_) * BCMPCIE_H2D_MSGRINGS(_max_tx_flows_))

/** D2H DMA Indices array size */
#define BCMPCIE_D2H_RW_INDEX_ARRAY_SZ(_rw_index_sz_) \
	((_rw_index_sz_) * BCMPCIE_D2H_COMMON_MSGRINGS)

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
			/* ONLY for compile time size computation of pre and post rev 5 */
			uint32  __post_rev5_extn;
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
#define PCIE_IPC_RINGS_REV5_SZ      (OFFSETOF(pcie_ipc_rings_t, __post_rev5_extn))

/** Post Rev5. pcie_ipc_rings_t is extended and future proofed */
#define PCIE_IPC_RINGS_SZ           (sizeof(pcie_ipc_rings_t)) /* 128 Bytes */

/**
 * PCIE IPC Specifications of Host Memory Extension (HME):
 * -------------------------------------------------------
 *
 * Host memory is treated as an extension of dongle. There are two forms of
 * host memory extensions,
 * 1. Private extension: Once a DMA-able memory buffer in host is allocated, the
 *    buffer is solely accessed by the dongle. Dongle may access this memory
 *    using direct programmed IO (aka SBTOPCIE) or via a M2M or D11 DMA engine.
 *    Host CPU complex (including network processors) do not access this memory.
 * 2. Shared extension: DMA-able memory buffer is accessed by both the Host CPU
 *    complex and the dongle CPU complex | HW cores.
 *    Applications based on producer consumer paradigm may instantiate circular
 *    rings of application structures, with WR, RD indices. Shared extensions
 *    need to take into consideration coherency on both host and dongle.
 *
 * In dongle, Host Memory Extension (HME) is provided as a RTE service, and
 * is implemented in bcmhme.[h,c]. bcmhme.h is not PCIe IPC revision control.
 * Likewise, applications using HME service with Shared extension paradigm, need
 * to ensure that DHD continues to be backward compatible when application
 * specification changes, using an application revision control.
 *
 * Implementation Note:
 * --------------------
 * Several users may request for host memory extensions, in 4 KByte page units.
 * Each user's host memory extension is a discrete region, whose size is
 * bounded (4 MByte - 1 HME page unit) - Linux based DHD.
 *
 * Prefix "host_mem" and "hme" are used interchangeably in DHD and Dongle.
 *
 * Interface Note:
 * ---------------
 * Prior to HME service introduction, the DHD host_mem scratch buffer of 8 Bytes
 * was extended to serve each new application/feature. The scratch buffer needed
 * to be sized to the max to accommodate all potential applications/features as
 * DHD services all dongle feature sets and chipsets.
 *
 * HME PCIE IP Handshake:
 * - PCIE_IPC_DCAP1_HOST_MEM_EXTN: HME capability advertized by dongle
 * - PCIE_IPC_HCAP1_HOST_MEM_EXTN: HME capability acknowledged by Host
 *
 * All HME regions can ONLY be allocated at initial PCIe IPC handshake.
 *
 * Host memory extensions are specified in units of 4 KByte HME Pages.
 * HME service enabled in dongle firmware build using -hme- and implemented in
 * bcmhme.[h,c].
 *
 * Pre PCIe IPC rev 0x84:
 * ----------------------
 * - PCIE_IPC_HME_USERS_MAX: Fixed max of 8 well known HME users.
 * - pcie_ipc::hme_pages[8]: Dongle users advertize HME region size in HME pages
 *	    When an HME based application is not built, a value 0 pages is used.
 * - pcie_ipc::host_mem_len: DHD accumulates the total number of pages across
 *      all HME users. This does not include any memory wasted for alignment or
 *      padding in each dhd_dma_buf_t.
 * - pcie_ipc::host_mem_haddr64: 64 bit address of a table of physical addresses
 *      of user HME regions. DHD, on allocation  of a HME region per user, saves
 *      the physical address in a table. Dongle HME service downloads the table
 *      of physical addresses in the PCIe handshake link phase.
 *      Prior to HME service, the pcie_ipc::host_mem_haddr64 would be the base
 *      physical address of the extended scratch buffer that was a contiguous
 *      memory from which each application would carve out it's region.
 *
 * PCIe IPC rev 0x84:
 * ------------------
 * HME service is enhanced to permit larger than 8 HME users with memory
 * attibutes that may be specified per user, in addition to HME pages.
 *
 * HME specification is carried via a discrete pcie_ipc_hme_t object permitting
 * a varible number of HME users defined by pcie_ipc_hme_user_t.
 * This discrete pcie_ipc_hme_t object is linked into the pcie_ipc_t object.
 *
 * Filled by Dongle:
 * - pcie_ipc::host_mem_users: Total number of pcie_ipc_hme_user_t in the array
 *      pcie_ipc_hme::user[]
 * - pcie_ipc::host_mem_size: Size of the struct pcie_ipc_hme, to include the
 *      the size of the variable array of pcie_ipc_hme_user_t
 * - pcie_ipc::host_mem_daddr32: 32 bit dongle address of the pcie_ipc_hme_t
 *      object including the array of variable number of pcie_ipc_hme_user_t.
 *      pcie_ipc::host_mem_daddr32 links the pcie_ipc_hme_t into pcie_ipc_t.
 *
 * Filled by DHD:
 * - pcie_ipc::host_mem_len: Sum total bytes of all DHD service HME regions
 * - pcie_ipc::host_mem_haddr64: Host 64 bit physical address of a table of
 *      per-user HME region's physical addresses.
 *
 * pcie_ipc_hme object carries a preamble to the array of pcie_ipc_hme_user_t
 * - pcie_ipc_hme::version: HME Service Version
 * - pcie_ipc_hme::users: same as pcie_ipc::host_mem_users
 * - pcie_ipc_hme::size: same as pcie_ipc::host_mem_size
 *
 *   Following fields are filled by HME service on completion of the PCIe IPC
 *   training (bus link phase)
 * - pcie_ipc_hme::bytes: Total bytes of all HME regions (pcie_ipc::host_mem_len)
 * - pcie_ipc_hme::haddr64: Physical address of the host resident table of per
 *       per-user HME physical addresses (pcie_ipc::host_mem_haddr64)
 *
 * Each HME user may explicitly specify memory attributes in pcie_ipc_hme_user_t
 * - pcie_ipc_hme_user::user_id: corresponds to index into array
 * - pcie_ipc_hme_user::align_bits: memory base alignment
 * - pcie_ipc_hme_user::bound_bits: memory region must not overlap boundary
 * - pcie_ipc_hme_user::sgmt_avail: Number of memory segments allocated by Host
 * - pcie_ipc_hme_user::flags: See below HME DMA-able buffer attributes
 * - pcie_ipc_hme_user::pages: Size of the HME region in units of HME pages
 * - pcie_ipc_hme_user::name: Debug printing HME user acronymn
 * - pcie_ipc_hme_user::bhm_offset: Offset of HME memory from BHM base address
 *
 * HME DMA-able buffer attributes. DHD may use these attributes to carve out
 * HME regions from a pre-allocated (e.g. reserved from bootloader).
 *
 *  HYBRIDFW:  Used to save dongle firmware text segment. DHD may protect this
 *             region from access by Router CPU complex. Exclusive access only
 *             to dongle SoC, after the dongle's host resident firmware portion
 *             has been copied.
 *   PRIVATE:  DHD does not access it, other than in PCIE IPC handshake. Private
 *             regions, may be allocated by DHD in un-cacheable memory.
 *   ALIGNED:  Alignment attributes specified in pcie_ipc_hme_user::align_bits
 *  BOUNDARY:  HME region must not cross a window boundary.
 *  SBTOPCIE:  HME region may be pio accessed using sbtopcie.
 *  DMA_XFER:  HME region may be accessed using DMA engines (PCIe,M2M,HWA,D11).
 *       BHM:  HME region carved from Boot Host Memory (BHM)
 *      SGMT:  HME region allocated in 4MB segments.
 *
 * PCIE_IPC_HME_FLAGS_BHM:
 *  To support the growing capabilities of WLAN devices, some HME users need
 *  very large memories (several MBs) but the Linux memory allocator has an
 *  upper limit of 4MB (PAGESIZE) contiguous memory.
 *  To serve the requirement of large memory, DHD uses the CMA and reserves
 *  a large chunk of contiguous memory from the CMA region during boot-up
 *  and pass the CMA memory address to the dongle using Boot Host Memory (BHM)
 *  TLV (bhm_location_tlv_t).
 *  HME service is then extended to carve host memory from the pre-allocated
 *  Boot Host memory (BHM). DHD will skip the memory allocation for the HME
 *  user with flag PCIE_IPC_HME_FLAGS_BHM set.
 *
 * PCIE_IPC_HME_FLAG_SGMT:
 *  CMA is not enabled by default in all Host platforms and also is dependent
 *  on Host OS configuration. In platforms without CMA (BHM), DHD will allocate
 *  HME user memory in multiple segments for the HME user with a large memory
 *  requirement and then place the Physical addr of HME user memory segmentation
 *  info (pcie_ipc_hme_sgmt_info_t) in the per-user HME region's table.
 *  If the PCIE_IPC_HME_FLAG_SGMT flag is set for the user then dongle fetches
 *  the segmentation information and service the Host memory requests for the
 *  HME user.
 *  Limitations: If the HME user requires more than 4MB of contiguous memory
 *               then HME segmentation will not help. For such user case,
 *               BHM is the only option available now.
 *
 * PCIe IPC rev 0x8B:
 * ------------------
 *  Host Memory allocated for the Dongle HME users might be accessed by three
 *  subsystems Host (DHD), UMAC (WLAN SW) and LMAC (ucode/D11 MAC).
 *  But Dongle doesn't exchange the Host memory details (Physical Address, Len)
 *  of HME users carved from BHM memory with the Host driver (DHD).
 *  So in IPC rev 0x8B, pcie_ipc_hme_user_t is extended to send Host Memory
 *  details from dongl to DHD
 *  If the HME user is carved from BHM memory then the dongle sends offset of
 *  Host Memory from the BHM base address in pcie_ipc_hme_user_t::bhm_offset.
 *  DHD then uses the bhm_offset to calculate the Host memory addr of HME user.
 *
 *
 * Theory of Operation:
 * --------------------
 * Dongle PCIe IPC Bind Phase:
 *   Each dongle user requests HME pages from the HME service. HME service data
 *   fill the pcie_ipc_hme_t with each user's request in pcie_ipc_hme::users[].
 *   HME binds the pcie_ipc_hme_t to the pcie_ipc via,
 *   pcie_ipc::<host_mem_users, host_mem_size, host_mem_daddr32>
 *   Dongle advertizes HME service from DHD via pcie_ipc::dcap1
 *
 * DHD PCIe Bus and Protocol layers, services all user's HME requests and
 *   places physical address of the table of per-user HME region's physical
 *   address into pcie_ipc::host_mem_haddr64, along with the sum total bytes
 *   of all HME user's regions into pcie_ipc::host_mem_len.
 *
 * Dongle PCIe IPC Link Phase:
 *   Dongle HME service DMA fetches the table of per-user's HME regions physical
 *   addresses and completes the HME service initialization. Users may avail of
 *   the HME service, upon completion of the dongle PCIE IPC link Phase.
 *
 *
 * Caveat 1: HME for HYBRIDFW (aka SW_PAGING)
 *   Dongle Host Memory Offload of the Text segment with SW_PAGING feature needs
 *   to avail of the HME region prior to the Link phase. DHD detects that the
 *   dongle uses a "hybridfw" wherein the dongle firmware is partitioned into a
 *   host resident and a dongle resident component. DHD pre-allocates a HME
 *   region for the host resident firmware component, fills it up and informs
 *   the dongle of this HME location and size using a TLV host_location_info_t.
 *
 *   The host_location_info_t structure is placed at a well known location
 *   computed from the top of the dongle memory. DHD also downloads the dongle
 *   resident firmware and kickstarts the dongle CPU. Dongle fetches the host
 *   resident firmware from the host_location_info_t at the well known address.
 *
 *   HYBRIDFW HME region attributes are defined below: PCIE_IPC_HYBRIDFW_###
 *
 */

#define PCIE_IPC_HME_USER_SCRMEM 0  /* Scratch memory: QT, log, debug, etc    */
#define PCIE_IPC_HME_USER_PKTPGR 1  /* PktPgr: Host Tx|Rx Packet Pool Memory  */
#define PCIE_IPC_HME_USER_MACIFS 2  /* MACIFs: AQM/TxDMA + Rx FIFO Memory     */
#define PCIE_IPC_HME_USER_LCLPKT 3  /* LCLPKT: Pageout of Dngl allocated Pkts */
#define PCIE_IPC_HME_USER_PSQPKT 4  /* PSQPKT: PS queue of packets offload    */
#define PCIE_IPC_HME_USER_CSIMON 5  /* Channel State Information Monitor      */
#define PCIE_IPC_HME_USER_HMOSWP 6  /* HYBRIDFW Host Memory Offload SW_PAGING */
#define PCIE_IPC_HME_USER_MBXPGR 7  /* STS_XFER Mailbox with "Status" Paging  */
#define PCIE_IPC_HME_USER_AIR_IQ 8  /* AirIQ feature                          */
#define PCIE_IPC_HME_USER_SBFTBL 9  /* Host Station Buffer Fairness Table     */
#define PCIE_IPC_HME_USER_HMOSCB 10 /* SCB Offload                            */
#define PCIE_IPC_HME_USER_LOWMAC 11 /* Host memory for lower MAC resources    */
#define PCIE_IPC_HME_USER_CLSBUF 12 /* Upper-Mac Console Log Streaming        */
#define PCIE_IPC_HME_USER_BMPSVC 13 /* Buffer Memory Pager (BMP) service      */
#define PCIE_IPC_HME_USER_HMOPPR 14 /* Host Memory Offload PPR		      */
#define PCIE_IPC_HME_USER_MACDBG 15 /* Host Memory Offload MACDBG	      */
#define PCIE_IPC_HME_USER_RU_CFG 16 /* Host Memory Offload RU_CFG	      */
#define PCIE_IPC_HME_USER_SCMEAS 17 /* Host Memory Offload scan measurement   */
#define PCIE_IPC_HME_USER_HMOEVT 18 /* Host Memory Offload Brcm events	      */
#define PCIE_IPC_HME_USER_MLOIPC 19 /* Host Memory for MLO IPC rings          */
#define PCIE_IPC_HME_USER_PCAPXF 20 /* Host Memory for PCAP hdr xfer          */

/** HME Page Size Unit : "number of pages" is represented as an uint16.       */
#define PCIE_IPC_HME_PAGE_SIZE      (4 * 1024) /**< 4 KBytes page units       */

/** Maximum size in Bytes of a host memory extension is (16 MBytes) */
#define PCIE_IPC_HME_BYTES_MAX      (16 * 1024 * 1024)

/** Maximum size in pages of a host memory extension (4096 pages) */
#define PCIE_IPC_HME_PAGES_MAX \
	(PCIE_IPC_HME_BYTES_MAX / PCIE_IPC_HME_PAGE_SIZE)

/** Maximum size in Bytes of a host memory extension is [4MB - 4K] if no BHM */
#define PCIE_IPC_HME_NOBHM_BYTES_MAX (4 * 1024 * 1023)

/** Maximum size in pages of a host memory extension (1023 pages) */
#define PCIE_IPC_HME_NOBHM_PAGES_MAX \
	(PCIE_IPC_HME_NOBHM_BYTES_MAX / PCIE_IPC_HME_PAGE_SIZE)

/** PCIE IPC Bytes to/from Pages conversion */
#define PCIE_IPC_HME_PAGES(_bytes_) \
	(((_bytes_) + (PCIE_IPC_HME_PAGE_SIZE - 1)) / PCIE_IPC_HME_PAGE_SIZE)
#define PCIE_IPC_HME_BYTES(_pages_)  ((_pages_) * PCIE_IPC_HME_PAGE_SIZE)

#define PCIE_IPC_HME_USERS_MAX      (8)
#define PCIE_IPC_HME_HADDR64_TBLSZ  (sizeof(haddr64_t) * PCIE_IPC_HME_USERS_MAX)

/**
 * PCIE IPC Host Memory Extension Specification [PCIE IPC Rev 0x84]
 */
#define PCIE_IPC_HME_FLAG_NONE     (0)
#define PCIE_IPC_HME_FLAG_HYBRIDFW (1 << 0) /**< Hybrid Firmware HMO SWPaging */
#define PCIE_IPC_HME_FLAG_PRIVATE  (1 << 1) /**< Access is private to dongle  */
#define PCIE_IPC_HME_FLAG_ALIGNED  (1 << 2) /**< Base alignment required      */
#define PCIE_IPC_HME_FLAG_BOUNDARY (1 << 3) /**< Wraparound boundary required */
#define PCIE_IPC_HME_FLAG_SBTOPCIE (1 << 4) /**< User may pio using sbtopcie  */
#define PCIE_IPC_HME_FLAG_DMA_XFER (1 << 5) /**< User may DMA to/from memory  */
#define PCIE_IPC_HME_FLAG_BHM      (1 << 6) /**< Memory carved from BHM       */
#define PCIE_IPC_HME_FLAG_SGMT     (1 << 7) /**< Memory allow segmentation    */

#define PCIE_IPC_HME_USER_NAME_SIZE (8)

/** Default HME memory attributes */
#define PCIE_IPC_HME_ALIGN_BITS     (8)  /* 256 Byte alignment */
#define PCIE_IPC_HME_BOUND_BITS     (25) /*  32 MByte boundary window */

#define PCIE_IPC_HME_FLAGS         \
	(PCIE_IPC_HME_FLAG_ALIGNED  | PCIE_IPC_HME_FLAG_BOUNDARY | \
	 PCIE_IPC_HME_FLAG_SBTOPCIE | PCIE_IPC_HME_FLAG_DMA_XFER)

#define PCIE_IPC_HME_SGMT_MAGIC     0x53474d54 /* Magic # of HME segment 'SGMT'  */
typedef struct pcie_ipc_hme_sgmt {  /**< HME segment                          */
	uint32    bytes;                /**< bytes of this segment                */
	haddr64_t haddr64;              /**< address of this segment              */
} pcie_ipc_hme_sgmt_t;

typedef struct pcie_ipc_hme_sgmt_info { /**< HME segment info for a user          */
	uint32 magic;                   /**< Magic number of HME segment          */
	uint16 size;                    /**< size of this struct with sgmts       */
	uint16 avail;                   /**< number of sgmts                      */
	pcie_ipc_hme_sgmt_t sgmt[0];    /**< Segments                             */
} pcie_ipc_hme_sgmt_info_t;

typedef struct pcie_ipc_hme_user {  /**< User's HME region attributes         */
	struct /* Rev 0x8A PCIE IPC HME shared structure */
	{
		uint8  user_id;         /**< user index                           */
		uint8  align_bits;      /**< alignment: used in 1 << align_bits   */
		uint8  bound_bits;      /**< boundary:  used in 1 << bound_bits   */
		uint8  sgmt_avail;      /**< segment number filled by Host        */
		uint16 flags;           /**< memory characteristics               */
		uint16 pages;           /**< user requested pages in 4KB units    */
		char   name[PCIE_IPC_HME_USER_NAME_SIZE]; /**< user name with \0  */
	}; /* 4 Words, 16 Bytes */

	struct /* Post Rev 0x8A PCIE IPC HME extension */
	{
		union {
			/* ONLY for compile time size computation of pre and post rev 0x8A */
			uint32  __post_rev_x8A_extn;
			uint32  bhm_offset;  /**< Offset from BHM Base address    */
		};
		uint32 PAD[3];
	}; /* 4 Words, 16 Bytes */

} pcie_ipc_hme_user_t, pcie_ipc_host_mem_user_t;

typedef struct pcie_ipc_hme {       /**< Variable sized HME structure         */
	uint8       version;            /**< total number of users in table       */
	uint8       users;              /**< total number of users in table       */
	uint16      size;               /**< size of this struct with user table  */
	uint32      bytes;              /**< sum total bytes of all HME serviced  */
	haddr64_t   haddr64;            /**< address of table of HME addresses    */
	pcie_ipc_hme_user_t user[0];    /**< variable sized array of hme users    */
} pcie_ipc_hme_t, pcie_ipc_host_mem_t;

#define PCIE_IPC_HME_USER_FMT \
	" HME User%2u %6s Pages %3u Bytes %7u Align %3u Bound 0x%08x Flags 0x%04x"
#define PCIE_IPC_HME_USER_VAL(_user_) \
	(_user_).user_id, (_user_).name, \
	(_user_).pages, PCIE_IPC_HME_BYTES((_user_).pages), \
	(1 << (_user_).align_bits), (1 << (_user_).bound_bits) - 1, (_user_).flags

/** Rev 0x8A pcie_ipc_hme_user_t size is limited to 16 Bytes */
#define PCIE_IPC_HME_REV_x8A_SIZE(_users_)	\
	(sizeof(pcie_ipc_hme_t) + (OFFSETOF(pcie_ipc_hme_user_t, __post_rev_x8A_extn) * (_users_)))
#define PCIE_IPC_HME_USER_REV_x8A_SIZE		\
	(OFFSETOF(pcie_ipc_hme_user_t, __post_rev_x8A_extn))

/** Post Rev 0x8A pcie_ipc_hme_user_t is extended and future proofed with reserved words */
#define PCIE_IPC_HOST_MEM_SIZE(_users_) \
	(sizeof(pcie_ipc_hme_t) + (sizeof(pcie_ipc_hme_user_t) * (_users_)))
#define PCIE_IPC_HME_SIZE(_users_)		PCIE_IPC_HOST_MEM_SIZE(_users_)

#define PCIE_IPC_HME_USER_SIZE			(sizeof(pcie_ipc_hme_user_t))

/*
 * Hope to achieve better cache locality by using a prime multiple of the cache
 * line size.
 */
#define HMOSCB_IPC_HME_SCB_SIZE    (269 * 64)
#define HMOSCB_HME_PAGES_PER_SCB   (PCIE_IPC_HME_PAGES(HMOSCB_IPC_HME_SCB_SIZE))
/* Max SCB's = floor(Max pages / pages-per-scb) */
#define HMOSCB_IPC_HME_MAXSCB      (PCIE_IPC_HME_PAGES_MAX / HMOSCB_HME_PAGES_PER_SCB)
#define HMOSCB_IPC_HME_PAGES       (HMOSCB_IPC_HME_MAXSCB * HMOSCB_HME_PAGES_PER_SCB)
#define HMOSCB_IPC_HME_BYTES       (PCIE_IPC_HME_BYTES(HMOSCB_IPC_HME_PAGES))

/**
 * PCIE IPC Specifications for Boot Host Memory (BHM):
 * ----------------------
 * DHD will allocate memory from Boot Reseved Memory (CMA) and pass it to the
 * dongle via a SysMem TLV identified by the signature BHM_LOCATION_TLV_SIGNATURE.
 * The dongle will then carve HME/MLO user memory from BHM region.
 */

#define BHM_LOCATION_TLV_SIZE		(20)

typedef struct bhm_location_tlv {
	uint32 haddr64_lo;          /* Lo 32b Physical address of BHM in host */
	uint32 haddr64_hi;          /* Hi 32b Physical address of BHM in host */
	uint32 haddr64_va_lo;       /* Lo 32b Virtual address of BHM in host */
	uint32 haddr64_va_hi;       /* Hi 32b Virtual address of BHM in host */
	uint32 bhm_size;            /* Size of BHM reserved in host */
	uint32 tlv_size;            /* 20B TLV content */
	uint32 tlv_signature;       /* BHM_LOCATION_TLV_SIGNATURE */
} bhm_location_tlv_t;

/**
 * PCIE IPC Specifications for Host Memory Offload (HMO):
 * -------------------------------------------------------
 *
 * Host Memory Offload features includes:
 * - text offload, aka "SW_PAGING" in dongle and "HYBRIDFW" in DHD, and
 * - (ro)data offload with Access-in-Place (AIP).
 * Both use the host_location_info struct to convey the host addresses at which
 * the pageable text segment and the (ro)data segment are placed in the host
 * DDR, in a reverse TLV (r-TLV) fashion.
 *
 * Symbols HYBRIDFW in DHD and SW_PAGING/HMOSWP in dongle, imply the same.
 *
 * NOTE: Even though HYBRIDFW is tracked in dongle as HME User HMOSWP, DHD
 * allocates the dma buffer before dongle advertizes the DMA buffer attributes.
 * HYBRIDFW related DMA buffer attributes are PCIE IPC rev controlled.
 */

typedef struct host_location_info {
	uint32 addr_lo;
	uint32 addr_hi;
	uint32 binary_size;
	uint32 tlv_size;
	uint32 tlv_signature;
} host_location_info_t;

/** Symbols HYBRIDFW in DHD and SW_PAGING/HMOSWP in dongle, imply the same */
#define PCIE_IPC_HYBRIDFW_HME_USER PCIE_IPC_HME_USER_HMOSWP

/** HYBRIDFW DMA buffer attributes for alignment and boundary */
#define PCIE_IPC_HYBRIDFW_ALIGN_BITS (12) /**< 4 KByte aligned DMA buffer */
#define PCIE_IPC_HYBRIDFW_BOUND_BITS (25) /**< DMA buffer within 32 MB region */

#define PCIE_IPC_HYBRIDFW_MEM_FLAGS  \
	(PCIE_IPC_HME_FLAG_HYBRIDFW | PCIE_IPC_HME_FLAG_PRIVATE | PCIE_IPC_HME_FLAGS)

/**
 * Dongle Hybrid Firmware COde FiLe Magic Number, with Dongle and Host resident
 * rtecdc text segments. Host resident firmware is paged on demand by the dongle
 * with MMU translations, as per Dongle SW Paging feature.
 */
#define PCIE_IPC_HYBRIDFW_MAGICNUM  (0x434F464Cu) /* 'C', 'O', 'F', 'L' */
#define PCIE_IPC_HYBRIDFW_TYPE_DNGL (0u) /**< Dngl resident rtecdc.bin */
#define PCIE_IPC_HYBRIDFW_TYPE_HOST (1u) /**< Host resident rtecdc_pageable.bin */

/**
 * PCIE IPC structure located in dongle's memory.
 * ----------------------------------------------
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
			uint32  msgtrace_daddr32;
			uint32  btrace_daddr32;	/* Address of btrace IPC area (btrace_ipc_t) */
		};

		uint32      fwid;

		uint16      max_tx_pkts; /**< Max Tx packets in dongle */
		uint16      max_rx_pkts; /**< Max Rx packets in dongle */

		union {
			uint32  dma_rxoffset;
			uint32  PAD; /**< may be repurposed */
		};

		daddr32_t   h2d_mb_daddr32; /**< location of h2d mailbox in dongle */
		daddr32_t   d2h_mb_daddr32; /**< location of h2d mailbox in dongle */

		daddr32_t   rings_daddr32; /**< rings info and per ring mem in dongle */

		/** Host filled Host Memory Extension information */
		uint32      host_mem_len; /**< sum total bytes of all HME serviced */
		haddr64_t   host_mem_haddr64; /**< address of table of HME addresses */
		/** Dongle filled Host Memory Extension pcie_ipc_hme_t */
		uint16      host_mem_users; /**< maximum number of HME users */
		uint16      host_mem_size; /**< sizeof pcie_ipc_hme_t with user table */
		uint32      host_mem_daddr32; /**< dongle location of pcie_ipc_hme_t */

		uint8       max_rch_sdu_cnt; /**< Max no of SDUs in an RCH (bcmpcie_rxcple_rch) */
		uint8       PAD[3];

		daddr32_t   buzzz_daddr32; /**< BUZZZ structure in dongle memory */
	}; /* 20 Words, 80 Bytes */

	struct /* Post Rev5 PCIE IPC extension */
	{
		union {
			/* ONLY for compile time size computation of pre and post rev 5 */
			uint32  __post_rev5_extn;
			uint32  dcap1; /**< dongle advertized capabilities */
		};
		uint32      dcap2; /**< dongle advertized capabilities */

		uint32      hcap1; /**< host acknowledged capabilities */
		uint32      hcap2; /**< host acknowledged capabilities */

		uint32      host_physaddrhi; /**< fixed host hi32 address CWI */

		union {
			uint32  fatal_logbuf_daddr32;
			struct {            /**< Rev 0x82, 0x83 */
				uint32  PAD[3];
				uint16  hme_pages[PCIE_IPC_HME_USERS_MAX]; /**< req HME pages */
			};
			uint32 PAD[7]; /**< Rev 0x84 */
			struct { /**< Rev 0x87 */
				haddr64_t cpudbg_dmamem_haddr64; /**< CPUDBG entry m2mdma hostmem */
				uint32    cpudbg_config;         /**< CPUDBG m2mdma setup config */
				uint32    PAD[4];
			};
			struct { /** Rev 0x8a */
				uint16	ucls_ring_size;		/**< UCLS circular ring size */
				uint16	ucls_ring_depth;	/**< UCLS circular ring depth */
				uint32	ucls_daddr32;		/**< UCLS registers emulation */
				uint16	hmo_event_ring_size;	/**< HMOEVENT circular ring size */
				uint16	hmo_event_ring_depth;	/**< HMOEVENT circular ring depth */
				uint32	hmo_event_rd_daddr32;	/**< HMOEVENT rd addr in dongle */
				uint32	hmo_event_wr_daddr32;	/**< HMOEVENT wr addr in dongle */
				uint32	PAD[2];
			};
		};
	}; /* 12 Words, 48 Bytes */

} pcie_ipc_t; /* 32 Words, 128 Bytes */

/** Rev5 pcie_ipc_t size is limited to (14 x 4 Bytes) */
#define PCIE_IPC_REV5_SZ        (OFFSETOF(pcie_ipc_t, __post_rev5_extn))

/** Post Rev5. pcie_ipc_t is extended and future proofed with reserved words */
#define PCIE_IPC_SZ             (sizeof(pcie_ipc_t)) /* 128 Bytes */

/**
 * Feature specific parameters
 *
 * Configurations that may change based upon dongle capabilities need to be
 * handshaked via the IPC, or will be applied equally to all dongle devices.
 */

/** BCM_DHDHDR: DHD can offload a maximum of 3072 * 256 Byte Txheaders. */
#define BCM_DHDHDR_MAXIMUM              3072
#define BCM_DHDHDR_SIZE                 256

/** Size of Extended Trap data Buffer */
#define BCMPCIE_EXT_TRAP_DATA_MAXLEN    4096

/* Trap types copied in the pcie_ipc.trap_daddr32 */
#define FW_INITIATED_TRAP_TYPE          (0x1 << 7)
#define HEALTHCHECK_NODS_TRAP_TYPE      (0x1 << 6)

/* H2D mail box Data */
#define PCIE_IPC_H2DMB_HOST_D3_INFORM          0x00000001
#define PCIE_IPC_H2DMB_HOST_DS_ACK             0x00000002
#define PCIE_IPC_H2DMB_HOST_DS_NAK             0x00000004
#define PCIE_IPC_H2DMB_HOST_D0_INFORM_IN_USE   0x00000008
#define PCIE_IPC_H2DMB_HOST_D0_INFORM          0x00000010
#define PCIE_IPC_H2DMB_DS_ACTIVE               0x00000020
#define PCIE_IPC_H2DMB_DS_DEVICE_WAKE          0x00000040
#define PCIE_IPC_H2DMB_HOST_IDMA_INITED        0x00000080
/* New H2D mail box events for MLO.
 * Do not want to override the previous bits because we 4366 is still supported by the dhd drivers.
 */
#define PCIE_IPC_H2DMB_MLC_BIND                0x00000100 /* Proceed to MLC_BIND after MLC_ATTACH */
#define PCIE_IPC_H2DMB_MLC_LINK                0x00000200 /* Proceed to MLC_LINK */
#define PCIE_IPC_H2DMB_MLC_BRIDGE              0x00000400 /* Notify to enter MLC_BRIDGE state */
#define PCIE_IPC_H2DMB_MLC_SYNC                0x00000800 /* Notify to enter MLC_SYNC state */
#define PCIE_IPC_H2DMB_MLC_READY               0x00001000 /* Notify to enter MLC_READY state */
#define PCIE_IPC_H2DMB_MLC_HALT                0x00002000 /* Specific to MLO, similar to FW_TRAP */
#define PCIE_IPC_H2DMB_MLC_SUSPEND             0x00004000 /* Notify to MLC_SUSPEND state */
#define PCIE_IPC_H2DMB_MLC_SUSPEND_HALT        0x00008000 /* Notify to MLC_SUSPEND_HALT state */

#define PCIE_IPC_H2DMB_HOST_ACK_NOINT          0x10000000
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

/* New D2H mail box events for MLO.
 */
#define PCIE_IPC_D2HMB_MLC_BIND                0x00000100 /* Dongle informs the MLC_BIND state */
#define PCIE_IPC_D2HMB_MLC_LINK                0x00000200 /* Dongle informs the MLC_LINK state */
#define PCIE_IPC_D2HMB_MLC_BRIDGE              0x00000400 /* Dongle informs the MLC_BRIDGE state */
#define PCIE_IPC_D2HMB_MLC_SYNC                0x00000800 /* Dongle informs the MLC_SYNC state */
#define PCIE_IPC_D2HMB_MLC_HALT                0x00001000 /* Dongle informs the MLC_HALT state */
#define PCIE_IPC_D2HMB_MLC_SUSPEND             0x00002000 /* Dongle informs the MLC_SUSPEND state */
#define PCIE_IPC_D2HMB_MLC_DOWN                0x00004000 /* Dongle informs the MLC_DOWN done */
#define PCIE_IPC_D2HMB_MLC_BH                  0x00008000 /* Dongle informs the MLC_BH done */
#define PCIE_IPC_D2HMB_MLC_ACTIVE              0x00010000 /* Dongle informs the MLC_ACTIVE state */
#define PCIE_IPC_D2HMB_MLC_BH_REQ              0x00020000 /* Dongle requests MLO BH procedure */
#define PCIE_IPC_D2HMB_MLC_BH_HALT_REQ         0x00040000 /* Dongle requests MLO BH with PSMWD */
#define PCIE_IPC_D2HMB_MLC_DOWN_BH             0x00080000 /* Dongle informs the MLC_DOWN_BH done */

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

#define READ_AVAIL_SPACE(w, r, d)   (((w) >= (r)) ? ((w) - (r)) : ((d) - (r)))

#define WRITE_SPACE_AVAIL(r, w, d)  ((d) - (NTXPACTIVE((r), (w), (d))) - 1)

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
 * When the PCIE_DMA_INDEX flag is enabled, dongle will use the mem2mem DMA
 * to transfer an entire array of RD or WR indices to/from host memory and DHD
 * may directly access local system memory.
 *
 * Dongle typically uses 16bit RD and WR index sizes with DMAing of indices.
 *
 * Dongle build -hbqd- uses 32bit DMA index size with 16bit RD and WR index and
 * 16bit queue depth (QD)
 *
 * Dongle will advertize the size of a WR or RD DMA index to DHD.
 */
#if defined(PCIE_HBQD)
typedef union pcie_rw_index {
	uint32 value;               /* 32bit: combined value */
	struct {
		uint16 idx;             /* 16bit: WR and RD index */
		uint16 qd;              /* 16bit: queue depth */
	};
} pcie_rw_index_t;              /* 32bit: 16bit WR and RD index + 16bit QD */
#else /* !PCIE_HBQD */
typedef uint16 pcie_rw_index_t; /* 16bit WR and RD indices (default) */
#endif /* !PCIE_HBQD */

#define PCIE_RW_INDEX_SZ                sizeof(pcie_rw_index_t)

/* Implicit DMA index usage
 *  Set0    : D2H rd_index (CtrlCpl + RxCpl + TxCpl, 3 indices, 6 bytes).
 *  Set1..14: H2D wr_index for 32 or 16 (PCIE_HBQD) indices per Set.
 *  Set15   : H2D wr_index for rest indices if any.
 */
#define BCMPCIE_IDMA_D2H_COMMON_INDEX        0
#define BCMPCIE_IDMA_H2D_COMMON_INDEX        1
#define BCMPCIE_IDMA_BYTES_PER_SET           64
/* Obsolete. Use BCMPCIE_IDMA_BYTES_PER_SET/(negotiated indices size) instead */
#define BCMPCIE_IDMA_FLOWS_PER_SET           32
#define BCMPCIE_IDMA_SET16                   16
#define BCMPCIE_IDMA_SET64                   64

#define BCMPCIE_IDMA_MAX_DESC                16
#define BCMPCIE_IDMA_H2D_MAX_DESC            (BCMPCIE_IDMA_MAX_DESC - 1)

#define PCIE_IPC_DCAP1_IFRM 0
#define PCIE_IPC_HCAP1_IFRM 0
#define BCMPCIE_IFRM_64_INDEX_PER_GROUP 64

/**
 * +--------------------------------------------------------------------------+
 *
 *  MLO related PCIe IPC. In a mix of NIC and Dongle mode, both WLAN and DHD
 *  participate in the MLO AP bringups. The term AP unit applies to the
 *  NIC mode WLAN unit or Dongle mode DHD unit.
 *
 *  A max of 4 APs may participate in MLO.
 *
 *  An AP assigned the logical mlo unit value 0 takes on the Main-AP (MAP) role.
 *  All other APs assume the Auxillary AP role. A mlo unit of -1, implies that
 *  the AP is not MLO capable.
 *
 *  - S2S: Upper MAC (Sw driver) to Upper MAC (Sw driver) messaging over PCIe
 *         uses a max of two S2S rings for the AAP to MAP (A2M) and the MAP to
 *         AAP (M2A).
 *  - H2S: Per AP, a set of maximum 4 H2S rings may be deployed for status
 *         transfer from the lower MAC (hw D11) to upper MAC (sw driver) over
 *         PCIe. All S2S and H2S rings are host DDR resident.
 *
 *  WL/DHD manages a table of PCIe IPC MLO structures per MLO participating AP.
 *  The address of this table is passed to the dongle during boot via a SysMem
 *  TLV identified by the signature PCIE_IPC_MLO_TLV_SIGNATURE.
 *
 *  WL/DHD uses it's WLAN unit number to locate the [AP_UNIT,MLO_UNIT] tuple in
 *  the nvram "wl_mlo_config". An AP assigned a  MLO unit value of 0 assumes
 *  the MAP role. All other APs, with a valid MLO unit will assume the AAP roles
 *  with the MLO unit value giving them a AAP# numbering scheme.
 *
 *  WL/DHD data fills the pcie_ipc_mlo::ap[x] before writing the TLV and taking
 *  the dongle processor out of reset. Dongle may DMA/PIO its pcie_ipc_mlo_ap_t
 *  and attach itself. WL/DHD binds to the PCIE IPC and provides back to WL/DHD
 *  the dongle PCIe IPC MLO attributes.
 *
 * +--------------------------------------------------------------------------+
 */

#define PCIE_IPC_MLO_VERSION        1

/* Maximum 4 WLAN AP probed units. All WL APs need not be MLO capable. */
#define PCIE_IPC_AP_UNIT_MAX        (4)
#define PCIE_IPC_AP_UNIT_INV        (-1)

/* Maximum 4 WLAN APs may participate in MLO. */
#define PCIE_IPC_MLO_UNIT_MAX       (PCIE_IPC_AP_UNIT_MAX)
#define PCIE_IPC_MLO_UNIT_MAP       (0)
#define PCIE_IPC_MLO_UNIT_INV       (-1)

/* MLO AP operation modes */
typedef enum pcie_ipc_mlo_opmode
{
	PCIE_IPC_MLO_OPMODE_NIC = 0,    /* NIC */
	PCIE_IPC_MLO_OPMODE_FD  = 1,    /* FD */
	PCIE_IPC_MLO_OPMODE_MAX = 2
} pcie_ipc_mlo_opmode_t;

/* MLO over PCIe BAR mapping size */
#define PCIE_IPC_MLO_BAR_SIZE       (8192 * 1024)

/**
 * Inter MLO-AP Upper MAC Software to Upper MAC Software (S2S) messaging rings.
 *  No S2S rings between AAP pairs. S2S rings exist only between AAP and MAP.
 *  - s2s_a2m are S2S rings where an AAP sends messages to an MAP
 *  - s2s_m2a are S2S rings where an MAP sends messages to an AAP
 */
#define PCIE_IPC_MLO_S2S_PRIO_MAX   (2) /* Low and High priority S2S rings */
#define PCIE_IPC_MLO_A2M_TOT        (PCIE_IPC_MLO_S2S_PRIO_MAX)
#define PCIE_IPC_MLO_M2A_TOT(_mlo_total_) \
	(PCIE_IPC_MLO_S2S_PRIO_MAX * ((_mlo_total_) - 1))

/**
 * MLO Status transfer from Lower MAC Hardware to Upper MAC Software (H2S) rings
 * A maximum of 4 H2S rings (PhyRxPkt, TxUlTrig, Rate_Sel and TxStatus) exist.
 */
#define PCIE_IPC_MLO_H2S_RING_MAX   (4) /* Maximum 4 LowerMAC to UpperMAC */

/* DHD boots a dongle firmware by specifying the AP's MLO attributes in a TLV */
typedef struct pcie_ipc_mlo_tlv {
	uint32 haddr64_lo;          /* Lo 32b address of pcie_ipc_mlo_t in host */
	uint32 haddr64_hi;          /* Hi 32b address of pcie_ipc_mlo_t in host */
	uint32 ap_unit;             /* WLAN AP unit assigned to the dongle */
	uint32 mlo_unit;            /* Logical MLO AP unit assigned */
	uint32 tlv_size;            /* 16B TLV content */
	uint32 tlv_signature;       /* PCIE_IPC_MLO_TLV_SIGNATURE */
} pcie_ipc_mlo_tlv_t;

typedef struct pcie_ipc_mlo_ring {      /* S2S and H2S ring info */
	uint64	  haddr_u64;		    /* Physical addr of Host ring in uint64 */
	uint64	  haddr_va_u64;		    /* Virtual addr of Host ring in uint64 (FD-NIC) */
	uint16    elem_size;                /* Size of element in ring, in bytes */
	uint16    elems_max;                /* Depth of ring in number of elems */
} pcie_ipc_mlo_ring_t, pcie_ipc_mlo_s2s_t, pcie_ipc_mlo_h2s_t;

#define PCIE_IPC_MLO_RING_FMT       HADDR64_FMT " elem size %4u max %4u"
#define PCIE_IPC_MLO_RING_VAL(ring) \
	HADDR_U64_VAL((ring)->haddr_u64), (ring)->elem_size, (ring)->elems_max

/* Attributes of a MLO participating AP are carried in a pcie_ipc_mlo_ap_t */
#define PCIE_IPC_MLO_AP_SIZE    (256)   /* Size of pcie_ipc_mlo_ap_t bytes */
#define PCIE_IPC_MLO_SYS_SIZE   (256)   /* Size of pcie_ipc_mlo_sys_t bytes */

/* Size of MLO shared memory */
#define PCIE_IPC_MLO_SHM_ZONE_SIZE (64) /* Size of MLO SHM zone */
#define PCIE_IPC_MLO_SHM_ZONE_LMAC (0)  /* LMAC SHM zone */
#define PCIE_IPC_MLO_SHM_ZONE_UMAC (1)  /* UMAC SHM zone */
#define PCIE_IPC_MLO_SHM_ZONE_UN1  (2)  /* Unused SHM zone */
#define PCIE_IPC_MLO_SHM_ZONE_UN2  (3)  /* Unused SHM zone */
#define PCIE_IPC_MLO_SHM_ZONE_MAX  (4)  /* Number of SHM zones */
#define PCIE_IPC_MLO_SHM_SIZE   (PCIE_IPC_MLO_SHM_ZONE_SIZE * PCIE_IPC_MLO_SHM_ZONE_MAX)

typedef union pcie_ipc_mlo_ap           /* MLO attributes per AP */
{
	struct {
		 int8           mlo_unit;       /* Logical MLO AP unit assigned */
		uint8           ap_unit;        /* DHD/WLAN AP unit */
		uint8           cpu_unit;       /* CPU Core unit */
		uint8           PAD;            /* Unused */

		uint16          bus_info;       /* PCIe unit(b15..b8), gen(b7..b4), lane(b3..b0) */
		uint8           s2s_rings_tot;  /* Total number of S2S rings AAP2MAP */
		uint8           h2s_rings_tot;  /* Total number of H2S rings Status */

		uint32          chip_id;        /* AP's chip id */

		/* AXI backplane attributes per AP */
		uint32          enum_base_bp;   /* AXI backplane base */
		uint32          mlc_axi_base;   /* MLC AXI base */
		uint32          mlc_apb_base;   /* MLC APB base */
		uint32          icc_axi_base;   /* ICC AXI base */

		haddr64_t       bar2_haddr64;   /* AP's BAR2 base address */

		/* OOB memory for MLC InterAP Mem2Mem DMA transfer over PCIE of 1 KBytes */
		daddr32_t       oemem_daddr32[PCIE_IPC_MLO_UNIT_MAX]; /* mlo_unit index */

		/* AAP specifies both the AAPtoMAP and MAPtoAAP ring attributes */
		pcie_ipc_mlo_s2s_t s2s_a2m[PCIE_IPC_MLO_S2S_PRIO_MAX]; /* A2M rings */
		pcie_ipc_mlo_s2s_t s2s_m2a[PCIE_IPC_MLO_S2S_PRIO_MAX]; /* M2A rings */

		/* Each MLO participating AP specifies the H2S ring attributes */
		pcie_ipc_mlo_h2s_t h2s_sts[PCIE_IPC_MLO_H2S_RING_MAX]; /* Status ring */

		uint16          nfifo_mlo;      /* Total fifos reserved for MLO */
		uint16          hw_fifo_mlo_start; /* Starting HW MAP TXFIFO for MLO */
	};
	uint8               u8[PCIE_IPC_MLO_AP_SIZE];
	uint32              u32[PCIE_IPC_MLO_AP_SIZE / 4];

} pcie_ipc_mlo_ap_t;    /* Ascertain 256 Bytes */

typedef union pcie_ipc_mlo_sys {        /* System-wide MLO attributes */
	struct {
		uint8           mlo_version;    /* WLAN MLO driver version */
		uint8           mlo_total;      /* Total APs participating in MLO */
		uint8           ap_unit_map;    /* WLAN AP unit assigned MAP role */
		uint8           mlo_ap_bm;	/* Bitmap of MLO APs from nvram */
		int8            mlo_unit[PCIE_IPC_AP_UNIT_MAX]; /* indexed AP unit */
		uint8           op_mode[PCIE_IPC_AP_UNIT_MAX];	/* operating mode - NIC/FD */
	};
	uint8               u8[PCIE_IPC_MLO_SYS_SIZE];
	uint32              u32[PCIE_IPC_MLO_SYS_SIZE / 4];
} pcie_ipc_mlo_sys_t;

/* PCIE IPC for MLO: all AP's MLO attributes and System-wide MLO attributes */
typedef struct pcie_ipc_mlo {
	pcie_ipc_mlo_ap_t   ap[PCIE_IPC_AP_UNIT_MAX]; /* Indexed by AP unit */
	pcie_ipc_mlo_sys_t  sys;
	uint8               shm[PCIE_IPC_MLO_SHM_SIZE];
} pcie_ipc_mlo_t;

/* MLO BCMC seq Definitions
 * In 12b seqnum, 0,1,2 and 0xFFF are excluded
 */
#define MLO_BCMC_SEQNUM_MAX			0xFFE
#define MLO_BCMC_SEQNUM_START			0x3
#define MLO_BCMC_SEQNO_ISVALID(seqno) \
	(((seqno) >= MLO_BCMC_SEQNUM_START) && ((seqno) <= MLO_BCMC_SEQNUM_MAX))
#endif /* !defined(_LANGUAGE_ASSEMBLY) && !defined(__ASSEMBLY__) */

/* --- Below used in assembly files. --- */

/**
 * +--------------------------------------------------------------------------+
 *
 * SysMem TOP (LISTING IN REVERSE ADDRESS ORDER FROM SYSMEM TOP).
 *
 * See startarm-ca#.S, where the sysmemtop is traversed in reverse address order
 * (via the stackpointer) and the below is extracted.
 *
 * ----------------------------------------------------------------------------
 *
 * BCMHOSTVARS: nvram variables downloaded to dongle by DHD
 *      _varsz = multiple of 4 encoded as (~len << 16 | len).
 *      nvram_array = array of nvram variables, NVRAM_ARRAY_MAXSIZE
 *              copied from sysmem top into nvram_array during startarm
 *
 * ----------------------------------------------------------------------------
 *
 * SW_PAGING TLV: see host_location_info_t accessed as host_page_info[] in
 *              swpaging.c. This TLV is 20 Bytes in size, and DHD informs dongle
 *              of the location in DDR where the offloaded text segment resides.
 *
 *      tlv_signature = BCM_HOST_PAGE_LOCATION_SIGNATURE
 *      tlv_size      = 12B
 *      binary_size
 *      addr_hi
 *      addr_lo
 *
 * ----------------------------------------------------------------------------
 *
 * MLO CONFIG TLV: see pcie_ipc_mlo_tlv_t
 *
 *      tlv_signature = PCIE_IPC_MLO_TLV_SIGNATURE
 *      tlv_size      = 16B
 *      ap_unit         DHD interface probe unit
 *      mlo_unit        logical MLO AP id, aka mlo_unit
 *      haddr64_hi      location of the multi-AP configuration in DDR, hi32
 *      haddr64_lo      location of the multi-AP configuration in DDR, lo32
 *
 * ----------------------------------------------------------------------------
 *
 * BHM LOCATION TLV: see bhm_location_tlv_t
 *
 *      tlv_signature = BHM_LOCATION_TLV_SIGNATURE
 *      tlv_size      = 20B
 *      haddr64_hi      location of Boot Host Memory in DDR, hi32
 *      haddr64_lo      location of Boot Host Memory in DDR, lo32
 *
 * +--------------------------------------------------------------------------+
 */

#define BCM_HOST_PAGE_LOCATION_SIGNATURE    0xFEED10C5 /* HMO SWPAGING Text */
#define BCM_HOST_AIP_LOCATION_SIGNATURE     0xFEED10C6 /* HMO AIP Data */
#define PCIE_IPC_MLO_TLV_SIGNATURE          0xFEED10C7 /* pcie_ipc_mlo_tlv_t */
#define BHM_LOCATION_TLV_SIGNATURE          0xFEED10C8 /* bhm_location_tlv_t */
#define PCIE_IPC_TLV_TOTAL_RANGE            0x100      /* 256 bytes for all TLV */

#endif /* _bcmpcie_h_ */
