/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pko.
 */

#ifndef __CVMX_PKO_DEFS_H__
#define __CVMX_PKO_DEFS_H__

#define CVMX_PKO_CHANNEL_LEVEL			(0x00015400000800F0ull)
#define CVMX_PKO_DPFI_ENA			(0x0001540000C00018ull)
#define CVMX_PKO_DPFI_FLUSH			(0x0001540000C00008ull)
#define CVMX_PKO_DPFI_FPA_AURA			(0x0001540000C00010ull)
#define CVMX_PKO_DPFI_STATUS			(0x0001540000C00000ull)
#define CVMX_PKO_DQX_BYTES(offset)		(0x00015400000000D8ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_CIR(offset)		(0x0001540000280018ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_DROPPED_BYTES(offset)	(0x00015400000000C8ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_DROPPED_PACKETS(offset)	(0x00015400000000C0ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_FIFO(offset)		(0x0001540000300078ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_PACKETS(offset)		(0x00015400000000D0ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_PICK(offset)		(0x0001540000300070ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_PIR(offset)		(0x0001540000280020ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_POINTERS(offset)		(0x0001540000280078ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_SCHEDULE(offset)		(0x0001540000280008ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_SCHED_STATE(offset)	(0x0001540000280028ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_SHAPE(offset)		(0x0001540000280010ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_SHAPE_STATE(offset)	(0x0001540000280030ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_SW_XOFF(offset)		(0x00015400002800E0ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_TOPOLOGY(offset)		(0x0001540000300000ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_WM_BUF_CNT(offset)		(0x00015400008000E8ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_WM_BUF_CTL(offset)		(0x00015400008000F0ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_WM_BUF_CTL_W1C(offset)	(0x00015400008000F8ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_WM_CNT(offset)		(0x0001540000000050ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_WM_CTL(offset)		(0x0001540000000040ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQX_WM_CTL_W1C(offset)		(0x0001540000000048ull + ((offset) & 1023) * 512)
#define CVMX_PKO_DQ_CSR_BUS_DEBUG		(0x00015400003001F8ull)
#define CVMX_PKO_DQ_DEBUG			(0x0001540000300128ull)
#define CVMX_PKO_DRAIN_IRQ			(0x0001540000000140ull)
#define CVMX_PKO_ENABLE				(0x0001540000D00008ull)
#define CVMX_PKO_FORMATX_CTL(offset)		(0x0001540000900800ull + ((offset) & 127) * 8)
#define CVMX_PKO_L1_SQA_DEBUG			(0x0001540000080128ull)
#define CVMX_PKO_L1_SQB_DEBUG			(0x0001540000080130ull)
#define CVMX_PKO_L1_SQX_CIR(offset)		(0x0001540000000018ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_DROPPED_BYTES(offset)	(0x0001540000000088ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_DROPPED_PACKETS(offset) (0x0001540000000080ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_GREEN(offset)		(0x0001540000080058ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_GREEN_BYTES(offset)	(0x00015400000000B8ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_GREEN_PACKETS(offset)	(0x00015400000000B0ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_LINK(offset)		(0x0001540000000038ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_PICK(offset)		(0x0001540000080070ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_RED(offset)		(0x0001540000080068ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_RED_BYTES(offset)	(0x0001540000000098ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_RED_PACKETS(offset)	(0x0001540000000090ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_SCHEDULE(offset)	(0x0001540000000008ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_SHAPE(offset)		(0x0001540000000010ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_SHAPE_STATE(offset)	(0x0001540000000030ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_SW_XOFF(offset)		(0x00015400000000E0ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_TOPOLOGY(offset)	(0x0001540000080000ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_YELLOW(offset)		(0x0001540000080060ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_YELLOW_BYTES(offset)	(0x00015400000000A8ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQX_YELLOW_PACKETS(offset)	(0x00015400000000A0ull + ((offset) & 31) * 512)
#define CVMX_PKO_L1_SQ_CSR_BUS_DEBUG		(0x00015400000801F8ull)
#define CVMX_PKO_L2_SQA_DEBUG			(0x0001540000100128ull)
#define CVMX_PKO_L2_SQB_DEBUG			(0x0001540000100130ull)
#define CVMX_PKO_L2_SQX_CIR(offset)		(0x0001540000080018ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_GREEN(offset)		(0x0001540000100058ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_PICK(offset)		(0x0001540000100070ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_PIR(offset)		(0x0001540000080020ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_POINTERS(offset)	(0x0001540000080078ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_RED(offset)		(0x0001540000100068ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_SCHEDULE(offset)	(0x0001540000080008ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_SCHED_STATE(offset)	(0x0001540000080028ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_SHAPE(offset)		(0x0001540000080010ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_SHAPE_STATE(offset)	(0x0001540000080030ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_SW_XOFF(offset)		(0x00015400000800E0ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_TOPOLOGY(offset)	(0x0001540000100000ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQX_YELLOW(offset)		(0x0001540000100060ull + ((offset) & 511) * 512)
#define CVMX_PKO_L2_SQ_CSR_BUS_DEBUG		(0x00015400001001F8ull)
#define CVMX_PKO_L3_L2_SQX_CHANNEL(offset)	(0x0001540000080038ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQA_DEBUG			(0x0001540000180128ull)
#define CVMX_PKO_L3_SQB_DEBUG			(0x0001540000180130ull)
#define CVMX_PKO_L3_SQX_CIR(offset)		(0x0001540000100018ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_GREEN(offset)		(0x0001540000180058ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_PICK(offset)		(0x0001540000180070ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_PIR(offset)		(0x0001540000100020ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_POINTERS(offset)	(0x0001540000100078ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_RED(offset)		(0x0001540000180068ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_SCHEDULE(offset)	(0x0001540000100008ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_SCHED_STATE(offset)	(0x0001540000100028ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_SHAPE(offset)		(0x0001540000100010ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_SHAPE_STATE(offset)	(0x0001540000100030ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_SW_XOFF(offset)		(0x00015400001000E0ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_TOPOLOGY(offset)	(0x0001540000180000ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQX_YELLOW(offset)		(0x0001540000180060ull + ((offset) & 511) * 512)
#define CVMX_PKO_L3_SQ_CSR_BUS_DEBUG		(0x00015400001801F8ull)
#define CVMX_PKO_L4_SQA_DEBUG			(0x0001540000200128ull)
#define CVMX_PKO_L4_SQB_DEBUG			(0x0001540000200130ull)
#define CVMX_PKO_L4_SQX_CIR(offset)		(0x0001540000180018ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_GREEN(offset)		(0x0001540000200058ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_PICK(offset)		(0x0001540000200070ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_PIR(offset)		(0x0001540000180020ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_POINTERS(offset)	(0x0001540000180078ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_RED(offset)		(0x0001540000200068ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_SCHEDULE(offset)	(0x0001540000180008ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_SCHED_STATE(offset)	(0x0001540000180028ull + ((offset) & 511) * 512)
#define CVMX_PKO_L4_SQX_SHAPE(offset)		(0x0001540000180010ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_SHAPE_STATE(offset)	(0x0001540000180030ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_SW_XOFF(offset)		(0x00015400001800E0ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_TOPOLOGY(offset)	(0x0001540000200000ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQX_YELLOW(offset)		(0x0001540000200060ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L4_SQ_CSR_BUS_DEBUG		(0x00015400002001F8ull)
#define CVMX_PKO_L5_SQA_DEBUG			(0x0001540000280128ull)
#define CVMX_PKO_L5_SQB_DEBUG			(0x0001540000280130ull)
#define CVMX_PKO_L5_SQX_CIR(offset)		(0x0001540000200018ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_GREEN(offset)		(0x0001540000280058ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_PICK(offset)		(0x0001540000280070ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_PIR(offset)		(0x0001540000200020ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_POINTERS(offset)	(0x0001540000200078ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_RED(offset)		(0x0001540000280068ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_SCHEDULE(offset)	(0x0001540000200008ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_SCHED_STATE(offset)	(0x0001540000200028ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_SHAPE(offset)		(0x0001540000200010ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_SHAPE_STATE(offset)	(0x0001540000200030ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_SW_XOFF(offset)		(0x00015400002000E0ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_TOPOLOGY(offset)	(0x0001540000280000ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQX_YELLOW(offset)		(0x0001540000280060ull + ((offset) & 1023) * 512)
#define CVMX_PKO_L5_SQ_CSR_BUS_DEBUG		(0x00015400002801F8ull)
#define CVMX_PKO_LUTX(offset)			(0x0001540000B00000ull + ((offset) & 1023) * 8)
#define CVMX_PKO_LUT_BIST_STATUS		(0x0001540000B02018ull)
#define CVMX_PKO_LUT_ECC_CTL0			(0x0001540000BFFFD0ull)
#define CVMX_PKO_LUT_ECC_DBE_STS0		(0x0001540000BFFFF0ull)
#define CVMX_PKO_LUT_ECC_DBE_STS_CMB0		(0x0001540000BFFFD8ull)
#define CVMX_PKO_LUT_ECC_SBE_STS0		(0x0001540000BFFFF8ull)
#define CVMX_PKO_LUT_ECC_SBE_STS_CMB0		(0x0001540000BFFFE8ull)
#define CVMX_PKO_MACX_CFG(offset)		(0x0001540000900000ull + ((offset) & 31) * 8)
#define CVMX_PKO_MCI0_CRED_CNTX(offset)		(0x0001540000A40000ull + ((offset) & 31) * 8)
#define CVMX_PKO_MCI0_MAX_CREDX(offset)		(0x0001540000A00000ull + ((offset) & 31) * 8)
#define CVMX_PKO_MCI1_CRED_CNTX(offset)		(0x0001540000A80100ull + ((offset) & 31) * 8)
#define CVMX_PKO_MCI1_MAX_CREDX(offset)		(0x0001540000A80000ull + ((offset) & 31) * 8)
#define CVMX_PKO_MEM_COUNT0			(0x0001180050001080ull)
#define CVMX_PKO_MEM_COUNT1			(0x0001180050001088ull)
#define CVMX_PKO_MEM_DEBUG0			(0x0001180050001100ull)
#define CVMX_PKO_MEM_DEBUG1			(0x0001180050001108ull)
#define CVMX_PKO_MEM_DEBUG10			(0x0001180050001150ull)
#define CVMX_PKO_MEM_DEBUG11			(0x0001180050001158ull)
#define CVMX_PKO_MEM_DEBUG12			(0x0001180050001160ull)
#define CVMX_PKO_MEM_DEBUG13			(0x0001180050001168ull)
#define CVMX_PKO_MEM_DEBUG14			(0x0001180050001170ull)
#define CVMX_PKO_MEM_DEBUG2			(0x0001180050001110ull)
#define CVMX_PKO_MEM_DEBUG3			(0x0001180050001118ull)
#define CVMX_PKO_MEM_DEBUG4			(0x0001180050001120ull)
#define CVMX_PKO_MEM_DEBUG5			(0x0001180050001128ull)
#define CVMX_PKO_MEM_DEBUG6			(0x0001180050001130ull)
#define CVMX_PKO_MEM_DEBUG7			(0x0001180050001138ull)
#define CVMX_PKO_MEM_DEBUG8			(0x0001180050001140ull)
#define CVMX_PKO_MEM_DEBUG9			(0x0001180050001148ull)
#define CVMX_PKO_MEM_IPORT_PTRS			(0x0001180050001030ull)
#define CVMX_PKO_MEM_IPORT_QOS			(0x0001180050001038ull)
#define CVMX_PKO_MEM_IQUEUE_PTRS		(0x0001180050001040ull)
#define CVMX_PKO_MEM_IQUEUE_QOS			(0x0001180050001048ull)
#define CVMX_PKO_MEM_PORT_PTRS			(0x0001180050001010ull)
#define CVMX_PKO_MEM_PORT_QOS			(0x0001180050001018ull)
#define CVMX_PKO_MEM_PORT_RATE0			(0x0001180050001020ull)
#define CVMX_PKO_MEM_PORT_RATE1			(0x0001180050001028ull)
#define CVMX_PKO_MEM_QUEUE_PTRS			(0x0001180050001000ull)
#define CVMX_PKO_MEM_QUEUE_QOS			(0x0001180050001008ull)
#define CVMX_PKO_MEM_THROTTLE_INT		(0x0001180050001058ull)
#define CVMX_PKO_MEM_THROTTLE_PIPE		(0x0001180050001050ull)
#define CVMX_PKO_NCB_BIST_STATUS		(0x0001540000EFFF00ull)
#define CVMX_PKO_NCB_ECC_CTL0			(0x0001540000EFFFD0ull)
#define CVMX_PKO_NCB_ECC_DBE_STS0		(0x0001540000EFFFF0ull)
#define CVMX_PKO_NCB_ECC_DBE_STS_CMB0		(0x0001540000EFFFD8ull)
#define CVMX_PKO_NCB_ECC_SBE_STS0		(0x0001540000EFFFF8ull)
#define CVMX_PKO_NCB_ECC_SBE_STS_CMB0		(0x0001540000EFFFE8ull)
#define CVMX_PKO_NCB_INT			(0x0001540000E00010ull)
#define CVMX_PKO_NCB_TX_ERR_INFO		(0x0001540000E00008ull)
#define CVMX_PKO_NCB_TX_ERR_WORD		(0x0001540000E00000ull)
#define CVMX_PKO_PDM_BIST_STATUS		(0x00015400008FFF00ull)
#define CVMX_PKO_PDM_CFG			(0x0001540000800000ull)
#define CVMX_PKO_PDM_CFG_DBG			(0x0001540000800FF8ull)
#define CVMX_PKO_PDM_CP_DBG			(0x0001540000800190ull)
#define CVMX_PKO_PDM_DQX_MINPAD(offset)		(0x00015400008F0000ull + ((offset) & 1023) * 8)
#define CVMX_PKO_PDM_DRPBUF_DBG			(0x00015400008000B0ull)
#define CVMX_PKO_PDM_DWPBUF_DBG			(0x00015400008000A8ull)
#define CVMX_PKO_PDM_ECC_CTL0			(0x00015400008FFFD0ull)
#define CVMX_PKO_PDM_ECC_CTL1			(0x00015400008FFFD8ull)
#define CVMX_PKO_PDM_ECC_DBE_STS0		(0x00015400008FFFF0ull)
#define CVMX_PKO_PDM_ECC_DBE_STS_CMB0		(0x00015400008FFFE0ull)
#define CVMX_PKO_PDM_ECC_SBE_STS0		(0x00015400008FFFF8ull)
#define CVMX_PKO_PDM_ECC_SBE_STS_CMB0		(0x00015400008FFFE8ull)
#define CVMX_PKO_PDM_FILLB_DBG0			(0x00015400008002A0ull)
#define CVMX_PKO_PDM_FILLB_DBG1			(0x00015400008002A8ull)
#define CVMX_PKO_PDM_FILLB_DBG2			(0x00015400008002B0ull)
#define CVMX_PKO_PDM_FLSHB_DBG0			(0x00015400008002B8ull)
#define CVMX_PKO_PDM_FLSHB_DBG1			(0x00015400008002C0ull)
#define CVMX_PKO_PDM_INTF_DBG_RD		(0x0001540000900F20ull)
#define CVMX_PKO_PDM_ISRD_DBG			(0x0001540000800090ull)
#define CVMX_PKO_PDM_ISRD_DBG_DQ		(0x0001540000800290ull)
#define CVMX_PKO_PDM_ISRM_DBG			(0x0001540000800098ull)
#define CVMX_PKO_PDM_ISRM_DBG_DQ		(0x0001540000800298ull)
#define CVMX_PKO_PDM_MEM_ADDR			(0x0001540000800018ull)
#define CVMX_PKO_PDM_MEM_DATA			(0x0001540000800010ull)
#define CVMX_PKO_PDM_MEM_RW_CTL			(0x0001540000800020ull)
#define CVMX_PKO_PDM_MEM_RW_STS			(0x0001540000800028ull)
#define CVMX_PKO_PDM_MWPBUF_DBG			(0x00015400008000A0ull)
#define CVMX_PKO_PDM_STS			(0x0001540000800008ull)
#define CVMX_PKO_PEB_BIST_STATUS		(0x0001540000900D00ull)
#define CVMX_PKO_PEB_ECC_CTL0			(0x00015400009FFFD0ull)
#define CVMX_PKO_PEB_ECC_CTL1			(0x00015400009FFFA8ull)
#define CVMX_PKO_PEB_ECC_DBE_STS0		(0x00015400009FFFF0ull)
#define CVMX_PKO_PEB_ECC_DBE_STS_CMB0		(0x00015400009FFFD8ull)
#define CVMX_PKO_PEB_ECC_SBE_STS0		(0x00015400009FFFF8ull)
#define CVMX_PKO_PEB_ECC_SBE_STS_CMB0		(0x00015400009FFFE8ull)
#define CVMX_PKO_PEB_ECO			(0x0001540000901000ull)
#define CVMX_PKO_PEB_ERR_INT			(0x0001540000900C00ull)
#define CVMX_PKO_PEB_EXT_HDR_DEF_ERR_INFO	(0x0001540000900C08ull)
#define CVMX_PKO_PEB_FCS_SOP_ERR_INFO		(0x0001540000900C18ull)
#define CVMX_PKO_PEB_JUMP_DEF_ERR_INFO		(0x0001540000900C10ull)
#define CVMX_PKO_PEB_MACX_CFG_WR_ERR_INFO	(0x0001540000900C50ull)
#define CVMX_PKO_PEB_MAX_LINK_ERR_INFO		(0x0001540000900C48ull)
#define CVMX_PKO_PEB_NCB_CFG			(0x0001540000900308ull)
#define CVMX_PKO_PEB_PAD_ERR_INFO		(0x0001540000900C28ull)
#define CVMX_PKO_PEB_PSE_FIFO_ERR_INFO		(0x0001540000900C20ull)
#define CVMX_PKO_PEB_SUBD_ADDR_ERR_INFO		(0x0001540000900C38ull)
#define CVMX_PKO_PEB_SUBD_SIZE_ERR_INFO		(0x0001540000900C40ull)
#define CVMX_PKO_PEB_TRUNC_ERR_INFO		(0x0001540000900C30ull)
#define CVMX_PKO_PEB_TSO_CFG			(0x0001540000900310ull)
#define CVMX_PKO_PQA_DEBUG			(0x0001540000000128ull)
#define CVMX_PKO_PQB_DEBUG			(0x0001540000000130ull)
#define CVMX_PKO_PQ_CSR_BUS_DEBUG		(0x00015400000001F8ull)
#define CVMX_PKO_PQ_DEBUG_GREEN			(0x0001540000000058ull)
#define CVMX_PKO_PQ_DEBUG_LINKS			(0x0001540000000068ull)
#define CVMX_PKO_PQ_DEBUG_YELLOW		(0x0001540000000060ull)
#define CVMX_PKO_PSE_DQ_BIST_STATUS		(0x0001540000300138ull)
#define CVMX_PKO_PSE_DQ_ECC_CTL0		(0x0001540000300100ull)
#define CVMX_PKO_PSE_DQ_ECC_DBE_STS0		(0x0001540000300118ull)
#define CVMX_PKO_PSE_DQ_ECC_DBE_STS_CMB0	(0x0001540000300120ull)
#define CVMX_PKO_PSE_DQ_ECC_SBE_STS0		(0x0001540000300108ull)
#define CVMX_PKO_PSE_DQ_ECC_SBE_STS_CMB0	(0x0001540000300110ull)
#define CVMX_PKO_PSE_PQ_BIST_STATUS		(0x0001540000000138ull)
#define CVMX_PKO_PSE_PQ_ECC_CTL0		(0x0001540000000100ull)
#define CVMX_PKO_PSE_PQ_ECC_DBE_STS0		(0x0001540000000118ull)
#define CVMX_PKO_PSE_PQ_ECC_DBE_STS_CMB0	(0x0001540000000120ull)
#define CVMX_PKO_PSE_PQ_ECC_SBE_STS0		(0x0001540000000108ull)
#define CVMX_PKO_PSE_PQ_ECC_SBE_STS_CMB0	(0x0001540000000110ull)
#define CVMX_PKO_PSE_SQ1_BIST_STATUS		(0x0001540000080138ull)
#define CVMX_PKO_PSE_SQ1_ECC_CTL0		(0x0001540000080100ull)
#define CVMX_PKO_PSE_SQ1_ECC_DBE_STS0		(0x0001540000080118ull)
#define CVMX_PKO_PSE_SQ1_ECC_DBE_STS_CMB0	(0x0001540000080120ull)
#define CVMX_PKO_PSE_SQ1_ECC_SBE_STS0		(0x0001540000080108ull)
#define CVMX_PKO_PSE_SQ1_ECC_SBE_STS_CMB0	(0x0001540000080110ull)
#define CVMX_PKO_PSE_SQ2_BIST_STATUS		(0x0001540000100138ull)
#define CVMX_PKO_PSE_SQ2_ECC_CTL0		(0x0001540000100100ull)
#define CVMX_PKO_PSE_SQ2_ECC_DBE_STS0		(0x0001540000100118ull)
#define CVMX_PKO_PSE_SQ2_ECC_DBE_STS_CMB0	(0x0001540000100120ull)
#define CVMX_PKO_PSE_SQ2_ECC_SBE_STS0		(0x0001540000100108ull)
#define CVMX_PKO_PSE_SQ2_ECC_SBE_STS_CMB0	(0x0001540000100110ull)
#define CVMX_PKO_PSE_SQ3_BIST_STATUS		(0x0001540000180138ull)
#define CVMX_PKO_PSE_SQ3_ECC_CTL0		(0x0001540000180100ull)
#define CVMX_PKO_PSE_SQ3_ECC_DBE_STS0		(0x0001540000180118ull)
#define CVMX_PKO_PSE_SQ3_ECC_DBE_STS_CMB0	(0x0001540000180120ull)
#define CVMX_PKO_PSE_SQ3_ECC_SBE_STS0		(0x0001540000180108ull)
#define CVMX_PKO_PSE_SQ3_ECC_SBE_STS_CMB0	(0x0001540000180110ull)
#define CVMX_PKO_PSE_SQ4_BIST_STATUS		(0x0001540000200138ull)
#define CVMX_PKO_PSE_SQ4_ECC_CTL0		(0x0001540000200100ull)
#define CVMX_PKO_PSE_SQ4_ECC_DBE_STS0		(0x0001540000200118ull)
#define CVMX_PKO_PSE_SQ4_ECC_DBE_STS_CMB0	(0x0001540000200120ull)
#define CVMX_PKO_PSE_SQ4_ECC_SBE_STS0		(0x0001540000200108ull)
#define CVMX_PKO_PSE_SQ4_ECC_SBE_STS_CMB0	(0x0001540000200110ull)
#define CVMX_PKO_PSE_SQ5_BIST_STATUS		(0x0001540000280138ull)
#define CVMX_PKO_PSE_SQ5_ECC_CTL0		(0x0001540000280100ull)
#define CVMX_PKO_PSE_SQ5_ECC_DBE_STS0		(0x0001540000280118ull)
#define CVMX_PKO_PSE_SQ5_ECC_DBE_STS_CMB0	(0x0001540000280120ull)
#define CVMX_PKO_PSE_SQ5_ECC_SBE_STS0		(0x0001540000280108ull)
#define CVMX_PKO_PSE_SQ5_ECC_SBE_STS_CMB0	(0x0001540000280110ull)
#define CVMX_PKO_PTFX_STATUS(offset)		(0x0001540000900100ull + ((offset) & 31) * 8)
#define CVMX_PKO_PTF_IOBP_CFG			(0x0001540000900300ull)
#define CVMX_PKO_PTGFX_CFG(offset)		(0x0001540000900200ull + ((offset) & 7) * 8)
#define CVMX_PKO_REG_BIST_RESULT		(0x0001180050000080ull)
#define CVMX_PKO_REG_CMD_BUF			(0x0001180050000010ull)
#define CVMX_PKO_REG_CRC_CTLX(offset)		(0x0001180050000028ull + ((offset) & 1) * 8)
#define CVMX_PKO_REG_CRC_ENABLE			(0x0001180050000020ull)
#define CVMX_PKO_REG_CRC_IVX(offset)		(0x0001180050000038ull + ((offset) & 1) * 8)
#define CVMX_PKO_REG_DEBUG0			(0x0001180050000098ull)
#define CVMX_PKO_REG_DEBUG1			(0x00011800500000A0ull)
#define CVMX_PKO_REG_DEBUG2			(0x00011800500000A8ull)
#define CVMX_PKO_REG_DEBUG3			(0x00011800500000B0ull)
#define CVMX_PKO_REG_DEBUG4			(0x00011800500000B8ull)
#define CVMX_PKO_REG_ENGINE_INFLIGHT		(0x0001180050000050ull)
#define CVMX_PKO_REG_ENGINE_INFLIGHT1		(0x0001180050000318ull)
#define CVMX_PKO_REG_ENGINE_STORAGEX(offset)	(0x0001180050000300ull + ((offset) & 1) * 8)
#define CVMX_PKO_REG_ENGINE_THRESH		(0x0001180050000058ull)
#define CVMX_PKO_REG_ERROR			(0x0001180050000088ull)
#define CVMX_PKO_REG_FLAGS			(0x0001180050000000ull)
#define CVMX_PKO_REG_GMX_PORT_MODE		(0x0001180050000018ull)
#define CVMX_PKO_REG_INT_MASK			(0x0001180050000090ull)
#define CVMX_PKO_REG_LOOPBACK_BPID		(0x0001180050000118ull)
#define CVMX_PKO_REG_LOOPBACK_PKIND		(0x0001180050000068ull)
#define CVMX_PKO_REG_MIN_PKT			(0x0001180050000070ull)
#define CVMX_PKO_REG_PREEMPT			(0x0001180050000110ull)
#define CVMX_PKO_REG_QUEUE_MODE			(0x0001180050000048ull)
#define CVMX_PKO_REG_QUEUE_PREEMPT		(0x0001180050000108ull)
#define CVMX_PKO_REG_QUEUE_PTRS1		(0x0001180050000100ull)
#define CVMX_PKO_REG_READ_IDX			(0x0001180050000008ull)
#define CVMX_PKO_REG_THROTTLE			(0x0001180050000078ull)
#define CVMX_PKO_REG_TIMESTAMP			(0x0001180050000060ull)
#define CVMX_PKO_SHAPER_CFG			(0x00015400000800F8ull)
#define CVMX_PKO_STATE_UID_IN_USEX_RD(offset)	(0x0001540000900F00ull + ((offset) & 3) * 8)
#define CVMX_PKO_STATUS				(0x0001540000D00000ull)
#define CVMX_PKO_TXFX_PKT_CNT_RD(offset)	(0x0001540000900E00ull + ((offset) & 31) * 8)

/**
 * cvmx_pko_channel_level
 */
union cvmx_pko_channel_level {
	u64 u64;
	struct cvmx_pko_channel_level_s {
		u64 reserved_1_63 : 63;
		u64 cc_level : 1;
	} s;
	struct cvmx_pko_channel_level_s cn73xx;
	struct cvmx_pko_channel_level_s cn78xx;
	struct cvmx_pko_channel_level_s cn78xxp1;
	struct cvmx_pko_channel_level_s cnf75xx;
};

typedef union cvmx_pko_channel_level cvmx_pko_channel_level_t;

/**
 * cvmx_pko_dpfi_ena
 */
union cvmx_pko_dpfi_ena {
	u64 u64;
	struct cvmx_pko_dpfi_ena_s {
		u64 reserved_1_63 : 63;
		u64 enable : 1;
	} s;
	struct cvmx_pko_dpfi_ena_s cn73xx;
	struct cvmx_pko_dpfi_ena_s cn78xx;
	struct cvmx_pko_dpfi_ena_s cn78xxp1;
	struct cvmx_pko_dpfi_ena_s cnf75xx;
};

typedef union cvmx_pko_dpfi_ena cvmx_pko_dpfi_ena_t;

/**
 * cvmx_pko_dpfi_flush
 */
union cvmx_pko_dpfi_flush {
	u64 u64;
	struct cvmx_pko_dpfi_flush_s {
		u64 reserved_1_63 : 63;
		u64 flush_en : 1;
	} s;
	struct cvmx_pko_dpfi_flush_s cn73xx;
	struct cvmx_pko_dpfi_flush_s cn78xx;
	struct cvmx_pko_dpfi_flush_s cn78xxp1;
	struct cvmx_pko_dpfi_flush_s cnf75xx;
};

typedef union cvmx_pko_dpfi_flush cvmx_pko_dpfi_flush_t;

/**
 * cvmx_pko_dpfi_fpa_aura
 */
union cvmx_pko_dpfi_fpa_aura {
	u64 u64;
	struct cvmx_pko_dpfi_fpa_aura_s {
		u64 reserved_12_63 : 52;
		u64 node : 2;
		u64 laura : 10;
	} s;
	struct cvmx_pko_dpfi_fpa_aura_s cn73xx;
	struct cvmx_pko_dpfi_fpa_aura_s cn78xx;
	struct cvmx_pko_dpfi_fpa_aura_s cn78xxp1;
	struct cvmx_pko_dpfi_fpa_aura_s cnf75xx;
};

typedef union cvmx_pko_dpfi_fpa_aura cvmx_pko_dpfi_fpa_aura_t;

/**
 * cvmx_pko_dpfi_status
 */
union cvmx_pko_dpfi_status {
	u64 u64;
	struct cvmx_pko_dpfi_status_s {
		u64 ptr_cnt : 32;
		u64 reserved_27_31 : 5;
		u64 xpd_fif_cnt : 4;
		u64 dalc_fif_cnt : 4;
		u64 alc_fif_cnt : 5;
		u64 reserved_13_13 : 1;
		u64 isrd_ptr1_rtn_full : 1;
		u64 isrd_ptr0_rtn_full : 1;
		u64 isrm_ptr1_rtn_full : 1;
		u64 isrm_ptr0_rtn_full : 1;
		u64 isrd_ptr1_val : 1;
		u64 isrd_ptr0_val : 1;
		u64 isrm_ptr1_val : 1;
		u64 isrm_ptr0_val : 1;
		u64 ptr_req_pend : 1;
		u64 ptr_rtn_pend : 1;
		u64 fpa_empty : 1;
		u64 dpfi_empty : 1;
		u64 cache_flushed : 1;
	} s;
	struct cvmx_pko_dpfi_status_s cn73xx;
	struct cvmx_pko_dpfi_status_s cn78xx;
	struct cvmx_pko_dpfi_status_cn78xxp1 {
		u64 ptr_cnt : 32;
		u64 reserved_13_31 : 19;
		u64 isrd_ptr1_rtn_full : 1;
		u64 isrd_ptr0_rtn_full : 1;
		u64 isrm_ptr1_rtn_full : 1;
		u64 isrm_ptr0_rtn_full : 1;
		u64 isrd_ptr1_val : 1;
		u64 isrd_ptr0_val : 1;
		u64 isrm_ptr1_val : 1;
		u64 isrm_ptr0_val : 1;
		u64 ptr_req_pend : 1;
		u64 ptr_rtn_pend : 1;
		u64 fpa_empty : 1;
		u64 dpfi_empty : 1;
		u64 cache_flushed : 1;
	} cn78xxp1;
	struct cvmx_pko_dpfi_status_s cnf75xx;
};

typedef union cvmx_pko_dpfi_status cvmx_pko_dpfi_status_t;

/**
 * cvmx_pko_dq#_bytes
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_BYTES.
 *
 */
union cvmx_pko_dqx_bytes {
	u64 u64;
	struct cvmx_pko_dqx_bytes_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_dqx_bytes_s cn73xx;
	struct cvmx_pko_dqx_bytes_s cn78xx;
	struct cvmx_pko_dqx_bytes_s cn78xxp1;
	struct cvmx_pko_dqx_bytes_s cnf75xx;
};

typedef union cvmx_pko_dqx_bytes cvmx_pko_dqx_bytes_t;

/**
 * cvmx_pko_dq#_cir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_dqx_cir {
	u64 u64;
	struct cvmx_pko_dqx_cir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_dqx_cir_s cn73xx;
	struct cvmx_pko_dqx_cir_s cn78xx;
	struct cvmx_pko_dqx_cir_s cn78xxp1;
	struct cvmx_pko_dqx_cir_s cnf75xx;
};

typedef union cvmx_pko_dqx_cir cvmx_pko_dqx_cir_t;

/**
 * cvmx_pko_dq#_dropped_bytes
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_BYTES.
 *
 */
union cvmx_pko_dqx_dropped_bytes {
	u64 u64;
	struct cvmx_pko_dqx_dropped_bytes_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_dqx_dropped_bytes_s cn73xx;
	struct cvmx_pko_dqx_dropped_bytes_s cn78xx;
	struct cvmx_pko_dqx_dropped_bytes_s cn78xxp1;
	struct cvmx_pko_dqx_dropped_bytes_s cnf75xx;
};

typedef union cvmx_pko_dqx_dropped_bytes cvmx_pko_dqx_dropped_bytes_t;

/**
 * cvmx_pko_dq#_dropped_packets
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_PACKETS.
 *
 */
union cvmx_pko_dqx_dropped_packets {
	u64 u64;
	struct cvmx_pko_dqx_dropped_packets_s {
		u64 reserved_40_63 : 24;
		u64 count : 40;
	} s;
	struct cvmx_pko_dqx_dropped_packets_s cn73xx;
	struct cvmx_pko_dqx_dropped_packets_s cn78xx;
	struct cvmx_pko_dqx_dropped_packets_s cn78xxp1;
	struct cvmx_pko_dqx_dropped_packets_s cnf75xx;
};

typedef union cvmx_pko_dqx_dropped_packets cvmx_pko_dqx_dropped_packets_t;

/**
 * cvmx_pko_dq#_fifo
 */
union cvmx_pko_dqx_fifo {
	u64 u64;
	struct cvmx_pko_dqx_fifo_s {
		u64 reserved_15_63 : 49;
		u64 p_con : 1;
		u64 head : 7;
		u64 tail : 7;
	} s;
	struct cvmx_pko_dqx_fifo_s cn73xx;
	struct cvmx_pko_dqx_fifo_s cn78xx;
	struct cvmx_pko_dqx_fifo_s cn78xxp1;
	struct cvmx_pko_dqx_fifo_s cnf75xx;
};

typedef union cvmx_pko_dqx_fifo cvmx_pko_dqx_fifo_t;

/**
 * cvmx_pko_dq#_packets
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_PACKETS.
 *
 */
union cvmx_pko_dqx_packets {
	u64 u64;
	struct cvmx_pko_dqx_packets_s {
		u64 reserved_40_63 : 24;
		u64 count : 40;
	} s;
	struct cvmx_pko_dqx_packets_s cn73xx;
	struct cvmx_pko_dqx_packets_s cn78xx;
	struct cvmx_pko_dqx_packets_s cn78xxp1;
	struct cvmx_pko_dqx_packets_s cnf75xx;
};

typedef union cvmx_pko_dqx_packets cvmx_pko_dqx_packets_t;

/**
 * cvmx_pko_dq#_pick
 *
 * This CSR contains the meta for the DQ, and is for debug and reconfiguration
 * only and should never be written. See also PKO_META_DESC_S.
 */
union cvmx_pko_dqx_pick {
	u64 u64;
	struct cvmx_pko_dqx_pick_s {
		u64 dq : 10;
		u64 color : 2;
		u64 child : 10;
		u64 bubble : 1;
		u64 p_con : 1;
		u64 c_con : 1;
		u64 uid : 7;
		u64 jump : 1;
		u64 fpd : 1;
		u64 ds : 1;
		u64 adjust : 9;
		u64 pir_dis : 1;
		u64 cir_dis : 1;
		u64 red_algo_override : 2;
		u64 length : 16;
	} s;
	struct cvmx_pko_dqx_pick_s cn73xx;
	struct cvmx_pko_dqx_pick_s cn78xx;
	struct cvmx_pko_dqx_pick_s cn78xxp1;
	struct cvmx_pko_dqx_pick_s cnf75xx;
};

typedef union cvmx_pko_dqx_pick cvmx_pko_dqx_pick_t;

/**
 * cvmx_pko_dq#_pir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_dqx_pir {
	u64 u64;
	struct cvmx_pko_dqx_pir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_dqx_pir_s cn73xx;
	struct cvmx_pko_dqx_pir_s cn78xx;
	struct cvmx_pko_dqx_pir_s cn78xxp1;
	struct cvmx_pko_dqx_pir_s cnf75xx;
};

typedef union cvmx_pko_dqx_pir cvmx_pko_dqx_pir_t;

/**
 * cvmx_pko_dq#_pointers
 *
 * This register has the same bit fields as PKO_L3_SQ(0..255)_POINTERS.
 *
 */
union cvmx_pko_dqx_pointers {
	u64 u64;
	struct cvmx_pko_dqx_pointers_s {
		u64 reserved_26_63 : 38;
		u64 prev : 10;
		u64 reserved_10_15 : 6;
		u64 next : 10;
	} s;
	struct cvmx_pko_dqx_pointers_cn73xx {
		u64 reserved_24_63 : 40;
		u64 prev : 8;
		u64 reserved_8_15 : 8;
		u64 next : 8;
	} cn73xx;
	struct cvmx_pko_dqx_pointers_s cn78xx;
	struct cvmx_pko_dqx_pointers_s cn78xxp1;
	struct cvmx_pko_dqx_pointers_cn73xx cnf75xx;
};

typedef union cvmx_pko_dqx_pointers cvmx_pko_dqx_pointers_t;

/**
 * cvmx_pko_dq#_sched_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHED_STATE.
 *
 */
union cvmx_pko_dqx_sched_state {
	u64 u64;
	struct cvmx_pko_dqx_sched_state_s {
		u64 reserved_25_63 : 39;
		u64 rr_count : 25;
	} s;
	struct cvmx_pko_dqx_sched_state_s cn73xx;
	struct cvmx_pko_dqx_sched_state_s cn78xx;
	struct cvmx_pko_dqx_sched_state_s cn78xxp1;
	struct cvmx_pko_dqx_sched_state_s cnf75xx;
};

typedef union cvmx_pko_dqx_sched_state cvmx_pko_dqx_sched_state_t;

/**
 * cvmx_pko_dq#_schedule
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHEDULE.
 *
 */
union cvmx_pko_dqx_schedule {
	u64 u64;
	struct cvmx_pko_dqx_schedule_s {
		u64 reserved_28_63 : 36;
		u64 prio : 4;
		u64 rr_quantum : 24;
	} s;
	struct cvmx_pko_dqx_schedule_s cn73xx;
	struct cvmx_pko_dqx_schedule_s cn78xx;
	struct cvmx_pko_dqx_schedule_s cn78xxp1;
	struct cvmx_pko_dqx_schedule_s cnf75xx;
};

typedef union cvmx_pko_dqx_schedule cvmx_pko_dqx_schedule_t;

/**
 * cvmx_pko_dq#_shape
 *
 * This register has the same bit fields as PKO_L3_SQ()_SHAPE.
 *
 */
union cvmx_pko_dqx_shape {
	u64 u64;
	struct cvmx_pko_dqx_shape_s {
		u64 reserved_27_63 : 37;
		u64 schedule_list : 2;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} s;
	struct cvmx_pko_dqx_shape_s cn73xx;
	struct cvmx_pko_dqx_shape_cn78xx {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} cn78xx;
	struct cvmx_pko_dqx_shape_cn78xx cn78xxp1;
	struct cvmx_pko_dqx_shape_s cnf75xx;
};

typedef union cvmx_pko_dqx_shape cvmx_pko_dqx_shape_t;

/**
 * cvmx_pko_dq#_shape_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cvmx_pko_dqx_shape_state {
	u64 u64;
	struct cvmx_pko_dqx_shape_state_s {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 color : 2;
		u64 pir_accum : 26;
		u64 cir_accum : 26;
	} s;
	struct cvmx_pko_dqx_shape_state_s cn73xx;
	struct cvmx_pko_dqx_shape_state_s cn78xx;
	struct cvmx_pko_dqx_shape_state_s cn78xxp1;
	struct cvmx_pko_dqx_shape_state_s cnf75xx;
};

typedef union cvmx_pko_dqx_shape_state cvmx_pko_dqx_shape_state_t;

/**
 * cvmx_pko_dq#_sw_xoff
 *
 * This register has the same bit fields as PKO_L1_SQ()_SW_XOFF.
 *
 */
union cvmx_pko_dqx_sw_xoff {
	u64 u64;
	struct cvmx_pko_dqx_sw_xoff_s {
		u64 reserved_4_63 : 60;
		u64 drain_irq : 1;
		u64 drain_null_link : 1;
		u64 drain : 1;
		u64 xoff : 1;
	} s;
	struct cvmx_pko_dqx_sw_xoff_s cn73xx;
	struct cvmx_pko_dqx_sw_xoff_s cn78xx;
	struct cvmx_pko_dqx_sw_xoff_s cn78xxp1;
	struct cvmx_pko_dqx_sw_xoff_s cnf75xx;
};

typedef union cvmx_pko_dqx_sw_xoff cvmx_pko_dqx_sw_xoff_t;

/**
 * cvmx_pko_dq#_topology
 */
union cvmx_pko_dqx_topology {
	u64 u64;
	struct cvmx_pko_dqx_topology_s {
		u64 reserved_26_63 : 38;
		u64 parent : 10;
		u64 reserved_0_15 : 16;
	} s;
	struct cvmx_pko_dqx_topology_cn73xx {
		u64 reserved_24_63 : 40;
		u64 parent : 8;
		u64 reserved_0_15 : 16;
	} cn73xx;
	struct cvmx_pko_dqx_topology_s cn78xx;
	struct cvmx_pko_dqx_topology_s cn78xxp1;
	struct cvmx_pko_dqx_topology_cn73xx cnf75xx;
};

typedef union cvmx_pko_dqx_topology cvmx_pko_dqx_topology_t;

/**
 * cvmx_pko_dq#_wm_buf_cnt
 */
union cvmx_pko_dqx_wm_buf_cnt {
	u64 u64;
	struct cvmx_pko_dqx_wm_buf_cnt_s {
		u64 reserved_36_63 : 28;
		u64 count : 36;
	} s;
	struct cvmx_pko_dqx_wm_buf_cnt_s cn73xx;
	struct cvmx_pko_dqx_wm_buf_cnt_s cn78xx;
	struct cvmx_pko_dqx_wm_buf_cnt_s cn78xxp1;
	struct cvmx_pko_dqx_wm_buf_cnt_s cnf75xx;
};

typedef union cvmx_pko_dqx_wm_buf_cnt cvmx_pko_dqx_wm_buf_cnt_t;

/**
 * cvmx_pko_dq#_wm_buf_ctl
 */
union cvmx_pko_dqx_wm_buf_ctl {
	u64 u64;
	struct cvmx_pko_dqx_wm_buf_ctl_s {
		u64 reserved_51_63 : 13;
		u64 enable : 1;
		u64 reserved_49_49 : 1;
		u64 intr : 1;
		u64 reserved_36_47 : 12;
		u64 threshold : 36;
	} s;
	struct cvmx_pko_dqx_wm_buf_ctl_s cn73xx;
	struct cvmx_pko_dqx_wm_buf_ctl_s cn78xx;
	struct cvmx_pko_dqx_wm_buf_ctl_s cn78xxp1;
	struct cvmx_pko_dqx_wm_buf_ctl_s cnf75xx;
};

typedef union cvmx_pko_dqx_wm_buf_ctl cvmx_pko_dqx_wm_buf_ctl_t;

/**
 * cvmx_pko_dq#_wm_buf_ctl_w1c
 */
union cvmx_pko_dqx_wm_buf_ctl_w1c {
	u64 u64;
	struct cvmx_pko_dqx_wm_buf_ctl_w1c_s {
		u64 reserved_49_63 : 15;
		u64 intr : 1;
		u64 reserved_0_47 : 48;
	} s;
	struct cvmx_pko_dqx_wm_buf_ctl_w1c_s cn73xx;
	struct cvmx_pko_dqx_wm_buf_ctl_w1c_s cn78xx;
	struct cvmx_pko_dqx_wm_buf_ctl_w1c_s cn78xxp1;
	struct cvmx_pko_dqx_wm_buf_ctl_w1c_s cnf75xx;
};

typedef union cvmx_pko_dqx_wm_buf_ctl_w1c cvmx_pko_dqx_wm_buf_ctl_w1c_t;

/**
 * cvmx_pko_dq#_wm_cnt
 */
union cvmx_pko_dqx_wm_cnt {
	u64 u64;
	struct cvmx_pko_dqx_wm_cnt_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_dqx_wm_cnt_s cn73xx;
	struct cvmx_pko_dqx_wm_cnt_s cn78xx;
	struct cvmx_pko_dqx_wm_cnt_s cn78xxp1;
	struct cvmx_pko_dqx_wm_cnt_s cnf75xx;
};

typedef union cvmx_pko_dqx_wm_cnt cvmx_pko_dqx_wm_cnt_t;

/**
 * cvmx_pko_dq#_wm_ctl
 */
union cvmx_pko_dqx_wm_ctl {
	u64 u64;
	struct cvmx_pko_dqx_wm_ctl_s {
		u64 reserved_52_63 : 12;
		u64 ncb_query_rsp : 1;
		u64 enable : 1;
		u64 kind : 1;
		u64 intr : 1;
		u64 threshold : 48;
	} s;
	struct cvmx_pko_dqx_wm_ctl_s cn73xx;
	struct cvmx_pko_dqx_wm_ctl_s cn78xx;
	struct cvmx_pko_dqx_wm_ctl_s cn78xxp1;
	struct cvmx_pko_dqx_wm_ctl_s cnf75xx;
};

typedef union cvmx_pko_dqx_wm_ctl cvmx_pko_dqx_wm_ctl_t;

/**
 * cvmx_pko_dq#_wm_ctl_w1c
 */
union cvmx_pko_dqx_wm_ctl_w1c {
	u64 u64;
	struct cvmx_pko_dqx_wm_ctl_w1c_s {
		u64 reserved_49_63 : 15;
		u64 intr : 1;
		u64 reserved_0_47 : 48;
	} s;
	struct cvmx_pko_dqx_wm_ctl_w1c_s cn73xx;
	struct cvmx_pko_dqx_wm_ctl_w1c_s cn78xx;
	struct cvmx_pko_dqx_wm_ctl_w1c_s cn78xxp1;
	struct cvmx_pko_dqx_wm_ctl_w1c_s cnf75xx;
};

typedef union cvmx_pko_dqx_wm_ctl_w1c cvmx_pko_dqx_wm_ctl_w1c_t;

/**
 * cvmx_pko_dq_csr_bus_debug
 */
union cvmx_pko_dq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_dq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_dq_csr_bus_debug_s cn73xx;
	struct cvmx_pko_dq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_dq_csr_bus_debug_s cn78xxp1;
	struct cvmx_pko_dq_csr_bus_debug_s cnf75xx;
};

typedef union cvmx_pko_dq_csr_bus_debug cvmx_pko_dq_csr_bus_debug_t;

/**
 * cvmx_pko_dq_debug
 */
union cvmx_pko_dq_debug {
	u64 u64;
	struct cvmx_pko_dq_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_dq_debug_s cn73xx;
	struct cvmx_pko_dq_debug_s cn78xx;
	struct cvmx_pko_dq_debug_s cn78xxp1;
	struct cvmx_pko_dq_debug_s cnf75xx;
};

typedef union cvmx_pko_dq_debug cvmx_pko_dq_debug_t;

/**
 * cvmx_pko_drain_irq
 */
union cvmx_pko_drain_irq {
	u64 u64;
	struct cvmx_pko_drain_irq_s {
		u64 reserved_1_63 : 63;
		u64 intr : 1;
	} s;
	struct cvmx_pko_drain_irq_s cn73xx;
	struct cvmx_pko_drain_irq_s cn78xx;
	struct cvmx_pko_drain_irq_s cn78xxp1;
	struct cvmx_pko_drain_irq_s cnf75xx;
};

typedef union cvmx_pko_drain_irq cvmx_pko_drain_irq_t;

/**
 * cvmx_pko_enable
 */
union cvmx_pko_enable {
	u64 u64;
	struct cvmx_pko_enable_s {
		u64 reserved_1_63 : 63;
		u64 enable : 1;
	} s;
	struct cvmx_pko_enable_s cn73xx;
	struct cvmx_pko_enable_s cn78xx;
	struct cvmx_pko_enable_s cn78xxp1;
	struct cvmx_pko_enable_s cnf75xx;
};

typedef union cvmx_pko_enable cvmx_pko_enable_t;

/**
 * cvmx_pko_format#_ctl
 *
 * Describes packet marking calculations for YELLOW and for RED_SEND packets.
 * PKO_SEND_HDR_S[FORMAT] selects the CSR used for the packet descriptor.
 *
 * All the packet marking calculations assume big-endian bits within a byte.
 *
 * For example, if MARKPTR is 3 and [OFFSET] is 5 and the packet is YELLOW,
 * the PKO marking hardware would do this:
 *
 * _  byte[3]<2:0> |=   Y_VAL<3:1>
 * _  byte[3]<2:0> &= ~Y_MASK<3:1>
 * _  byte[4]<7>   |=   Y_VAL<0>
 * _  byte[4]<7>   &= ~Y_MASK<0>
 *
 * where byte[3] is the 3rd byte in the packet, and byte[4] the 4th.
 *
 * For another example, if MARKPTR is 3 and [OFFSET] is 0 and the packet is RED_SEND,
 *
 * _   byte[3]<7:4> |=   R_VAL<3:0>
 * _   byte[3]<7:4> &= ~R_MASK<3:0>
 */
union cvmx_pko_formatx_ctl {
	u64 u64;
	struct cvmx_pko_formatx_ctl_s {
		u64 reserved_27_63 : 37;
		u64 offset : 11;
		u64 y_mask : 4;
		u64 y_val : 4;
		u64 r_mask : 4;
		u64 r_val : 4;
	} s;
	struct cvmx_pko_formatx_ctl_s cn73xx;
	struct cvmx_pko_formatx_ctl_s cn78xx;
	struct cvmx_pko_formatx_ctl_s cn78xxp1;
	struct cvmx_pko_formatx_ctl_s cnf75xx;
};

typedef union cvmx_pko_formatx_ctl cvmx_pko_formatx_ctl_t;

/**
 * cvmx_pko_l1_sq#_cir
 */
union cvmx_pko_l1_sqx_cir {
	u64 u64;
	struct cvmx_pko_l1_sqx_cir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l1_sqx_cir_s cn73xx;
	struct cvmx_pko_l1_sqx_cir_s cn78xx;
	struct cvmx_pko_l1_sqx_cir_s cn78xxp1;
	struct cvmx_pko_l1_sqx_cir_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_cir cvmx_pko_l1_sqx_cir_t;

/**
 * cvmx_pko_l1_sq#_dropped_bytes
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_BYTES.
 *
 */
union cvmx_pko_l1_sqx_dropped_bytes {
	u64 u64;
	struct cvmx_pko_l1_sqx_dropped_bytes_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_l1_sqx_dropped_bytes_s cn73xx;
	struct cvmx_pko_l1_sqx_dropped_bytes_s cn78xx;
	struct cvmx_pko_l1_sqx_dropped_bytes_s cn78xxp1;
	struct cvmx_pko_l1_sqx_dropped_bytes_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_dropped_bytes cvmx_pko_l1_sqx_dropped_bytes_t;

/**
 * cvmx_pko_l1_sq#_dropped_packets
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_PACKETS.
 *
 */
union cvmx_pko_l1_sqx_dropped_packets {
	u64 u64;
	struct cvmx_pko_l1_sqx_dropped_packets_s {
		u64 reserved_40_63 : 24;
		u64 count : 40;
	} s;
	struct cvmx_pko_l1_sqx_dropped_packets_s cn73xx;
	struct cvmx_pko_l1_sqx_dropped_packets_s cn78xx;
	struct cvmx_pko_l1_sqx_dropped_packets_s cn78xxp1;
	struct cvmx_pko_l1_sqx_dropped_packets_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_dropped_packets cvmx_pko_l1_sqx_dropped_packets_t;

/**
 * cvmx_pko_l1_sq#_green
 */
union cvmx_pko_l1_sqx_green {
	u64 u64;
	struct cvmx_pko_l1_sqx_green_s {
		u64 reserved_41_63 : 23;
		u64 rr_active : 1;
		u64 active_vec : 20;
		u64 reserved_19_19 : 1;
		u64 head : 9;
		u64 reserved_9_9 : 1;
		u64 tail : 9;
	} s;
	struct cvmx_pko_l1_sqx_green_s cn73xx;
	struct cvmx_pko_l1_sqx_green_s cn78xx;
	struct cvmx_pko_l1_sqx_green_s cn78xxp1;
	struct cvmx_pko_l1_sqx_green_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_green cvmx_pko_l1_sqx_green_t;

/**
 * cvmx_pko_l1_sq#_green_bytes
 */
union cvmx_pko_l1_sqx_green_bytes {
	u64 u64;
	struct cvmx_pko_l1_sqx_green_bytes_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_l1_sqx_green_bytes_s cn73xx;
	struct cvmx_pko_l1_sqx_green_bytes_s cn78xx;
	struct cvmx_pko_l1_sqx_green_bytes_s cn78xxp1;
	struct cvmx_pko_l1_sqx_green_bytes_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_green_bytes cvmx_pko_l1_sqx_green_bytes_t;

/**
 * cvmx_pko_l1_sq#_green_packets
 */
union cvmx_pko_l1_sqx_green_packets {
	u64 u64;
	struct cvmx_pko_l1_sqx_green_packets_s {
		u64 reserved_40_63 : 24;
		u64 count : 40;
	} s;
	struct cvmx_pko_l1_sqx_green_packets_s cn73xx;
	struct cvmx_pko_l1_sqx_green_packets_s cn78xx;
	struct cvmx_pko_l1_sqx_green_packets_s cn78xxp1;
	struct cvmx_pko_l1_sqx_green_packets_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_green_packets cvmx_pko_l1_sqx_green_packets_t;

/**
 * cvmx_pko_l1_sq#_link
 */
union cvmx_pko_l1_sqx_link {
	u64 u64;
	struct cvmx_pko_l1_sqx_link_s {
		u64 reserved_49_63 : 15;
		u64 link : 5;
		u64 reserved_32_43 : 12;
		u64 cc_word_cnt : 20;
		u64 cc_packet_cnt : 10;
		u64 cc_enable : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_l1_sqx_link_cn73xx {
		u64 reserved_48_63 : 16;
		u64 link : 4;
		u64 reserved_32_43 : 12;
		u64 cc_word_cnt : 20;
		u64 cc_packet_cnt : 10;
		u64 cc_enable : 1;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_l1_sqx_link_s cn78xx;
	struct cvmx_pko_l1_sqx_link_s cn78xxp1;
	struct cvmx_pko_l1_sqx_link_cn73xx cnf75xx;
};

typedef union cvmx_pko_l1_sqx_link cvmx_pko_l1_sqx_link_t;

/**
 * cvmx_pko_l1_sq#_pick
 *
 * This CSR contains the meta for the L1 SQ, and is for debug and reconfiguration
 * only and should never be written. See also PKO_META_DESC_S.
 */
union cvmx_pko_l1_sqx_pick {
	u64 u64;
	struct cvmx_pko_l1_sqx_pick_s {
		u64 dq : 10;
		u64 color : 2;
		u64 child : 10;
		u64 bubble : 1;
		u64 p_con : 1;
		u64 c_con : 1;
		u64 uid : 7;
		u64 jump : 1;
		u64 fpd : 1;
		u64 ds : 1;
		u64 adjust : 9;
		u64 pir_dis : 1;
		u64 cir_dis : 1;
		u64 red_algo_override : 2;
		u64 length : 16;
	} s;
	struct cvmx_pko_l1_sqx_pick_s cn73xx;
	struct cvmx_pko_l1_sqx_pick_s cn78xx;
	struct cvmx_pko_l1_sqx_pick_s cn78xxp1;
	struct cvmx_pko_l1_sqx_pick_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_pick cvmx_pko_l1_sqx_pick_t;

/**
 * cvmx_pko_l1_sq#_red
 *
 * This register has the same bit fields as PKO_L1_SQ()_YELLOW.
 *
 */
union cvmx_pko_l1_sqx_red {
	u64 u64;
	struct cvmx_pko_l1_sqx_red_s {
		u64 reserved_19_63 : 45;
		u64 head : 9;
		u64 reserved_9_9 : 1;
		u64 tail : 9;
	} s;
	struct cvmx_pko_l1_sqx_red_s cn73xx;
	struct cvmx_pko_l1_sqx_red_s cn78xx;
	struct cvmx_pko_l1_sqx_red_s cn78xxp1;
	struct cvmx_pko_l1_sqx_red_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_red cvmx_pko_l1_sqx_red_t;

/**
 * cvmx_pko_l1_sq#_red_bytes
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_BYTES.
 *
 */
union cvmx_pko_l1_sqx_red_bytes {
	u64 u64;
	struct cvmx_pko_l1_sqx_red_bytes_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_l1_sqx_red_bytes_s cn73xx;
	struct cvmx_pko_l1_sqx_red_bytes_s cn78xx;
	struct cvmx_pko_l1_sqx_red_bytes_s cn78xxp1;
	struct cvmx_pko_l1_sqx_red_bytes_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_red_bytes cvmx_pko_l1_sqx_red_bytes_t;

/**
 * cvmx_pko_l1_sq#_red_packets
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_PACKETS.
 *
 */
union cvmx_pko_l1_sqx_red_packets {
	u64 u64;
	struct cvmx_pko_l1_sqx_red_packets_s {
		u64 reserved_40_63 : 24;
		u64 count : 40;
	} s;
	struct cvmx_pko_l1_sqx_red_packets_s cn73xx;
	struct cvmx_pko_l1_sqx_red_packets_s cn78xx;
	struct cvmx_pko_l1_sqx_red_packets_s cn78xxp1;
	struct cvmx_pko_l1_sqx_red_packets_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_red_packets cvmx_pko_l1_sqx_red_packets_t;

/**
 * cvmx_pko_l1_sq#_schedule
 */
union cvmx_pko_l1_sqx_schedule {
	u64 u64;
	struct cvmx_pko_l1_sqx_schedule_s {
		u64 dummy : 40;
		u64 rr_quantum : 24;
	} s;
	struct cvmx_pko_l1_sqx_schedule_cn73xx {
		u64 reserved_24_63 : 40;
		u64 rr_quantum : 24;
	} cn73xx;
	struct cvmx_pko_l1_sqx_schedule_cn73xx cn78xx;
	struct cvmx_pko_l1_sqx_schedule_s cn78xxp1;
	struct cvmx_pko_l1_sqx_schedule_cn73xx cnf75xx;
};

typedef union cvmx_pko_l1_sqx_schedule cvmx_pko_l1_sqx_schedule_t;

/**
 * cvmx_pko_l1_sq#_shape
 */
union cvmx_pko_l1_sqx_shape {
	u64 u64;
	struct cvmx_pko_l1_sqx_shape_s {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_18_23 : 6;
		u64 link : 5;
		u64 reserved_9_12 : 4;
		u64 adjust : 9;
	} s;
	struct cvmx_pko_l1_sqx_shape_cn73xx {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_17_23 : 7;
		u64 link : 4;
		u64 reserved_9_12 : 4;
		u64 adjust : 9;
	} cn73xx;
	struct cvmx_pko_l1_sqx_shape_s cn78xx;
	struct cvmx_pko_l1_sqx_shape_s cn78xxp1;
	struct cvmx_pko_l1_sqx_shape_cn73xx cnf75xx;
};

typedef union cvmx_pko_l1_sqx_shape cvmx_pko_l1_sqx_shape_t;

/**
 * cvmx_pko_l1_sq#_shape_state
 *
 * This register must not be written during normal operation.
 *
 */
union cvmx_pko_l1_sqx_shape_state {
	u64 u64;
	struct cvmx_pko_l1_sqx_shape_state_s {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 color2 : 1;
		u64 color : 1;
		u64 reserved_26_51 : 26;
		u64 cir_accum : 26;
	} s;
	struct cvmx_pko_l1_sqx_shape_state_cn73xx {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 reserved_53_53 : 1;
		u64 color : 1;
		u64 reserved_26_51 : 26;
		u64 cir_accum : 26;
	} cn73xx;
	struct cvmx_pko_l1_sqx_shape_state_cn73xx cn78xx;
	struct cvmx_pko_l1_sqx_shape_state_s cn78xxp1;
	struct cvmx_pko_l1_sqx_shape_state_cn73xx cnf75xx;
};

typedef union cvmx_pko_l1_sqx_shape_state cvmx_pko_l1_sqx_shape_state_t;

/**
 * cvmx_pko_l1_sq#_sw_xoff
 */
union cvmx_pko_l1_sqx_sw_xoff {
	u64 u64;
	struct cvmx_pko_l1_sqx_sw_xoff_s {
		u64 reserved_4_63 : 60;
		u64 drain_irq : 1;
		u64 drain_null_link : 1;
		u64 drain : 1;
		u64 xoff : 1;
	} s;
	struct cvmx_pko_l1_sqx_sw_xoff_s cn73xx;
	struct cvmx_pko_l1_sqx_sw_xoff_s cn78xx;
	struct cvmx_pko_l1_sqx_sw_xoff_s cn78xxp1;
	struct cvmx_pko_l1_sqx_sw_xoff_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_sw_xoff cvmx_pko_l1_sqx_sw_xoff_t;

/**
 * cvmx_pko_l1_sq#_topology
 */
union cvmx_pko_l1_sqx_topology {
	u64 u64;
	struct cvmx_pko_l1_sqx_topology_s {
		u64 reserved_41_63 : 23;
		u64 prio_anchor : 9;
		u64 reserved_21_31 : 11;
		u64 link : 5;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_l1_sqx_topology_cn73xx {
		u64 reserved_40_63 : 24;
		u64 prio_anchor : 8;
		u64 reserved_20_31 : 12;
		u64 link : 4;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_l1_sqx_topology_s cn78xx;
	struct cvmx_pko_l1_sqx_topology_s cn78xxp1;
	struct cvmx_pko_l1_sqx_topology_cn73xx cnf75xx;
};

typedef union cvmx_pko_l1_sqx_topology cvmx_pko_l1_sqx_topology_t;

/**
 * cvmx_pko_l1_sq#_yellow
 */
union cvmx_pko_l1_sqx_yellow {
	u64 u64;
	struct cvmx_pko_l1_sqx_yellow_s {
		u64 reserved_19_63 : 45;
		u64 head : 9;
		u64 reserved_9_9 : 1;
		u64 tail : 9;
	} s;
	struct cvmx_pko_l1_sqx_yellow_s cn73xx;
	struct cvmx_pko_l1_sqx_yellow_s cn78xx;
	struct cvmx_pko_l1_sqx_yellow_s cn78xxp1;
	struct cvmx_pko_l1_sqx_yellow_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_yellow cvmx_pko_l1_sqx_yellow_t;

/**
 * cvmx_pko_l1_sq#_yellow_bytes
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_BYTES.
 *
 */
union cvmx_pko_l1_sqx_yellow_bytes {
	u64 u64;
	struct cvmx_pko_l1_sqx_yellow_bytes_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_l1_sqx_yellow_bytes_s cn73xx;
	struct cvmx_pko_l1_sqx_yellow_bytes_s cn78xx;
	struct cvmx_pko_l1_sqx_yellow_bytes_s cn78xxp1;
	struct cvmx_pko_l1_sqx_yellow_bytes_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_yellow_bytes cvmx_pko_l1_sqx_yellow_bytes_t;

/**
 * cvmx_pko_l1_sq#_yellow_packets
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN_PACKETS.
 *
 */
union cvmx_pko_l1_sqx_yellow_packets {
	u64 u64;
	struct cvmx_pko_l1_sqx_yellow_packets_s {
		u64 reserved_40_63 : 24;
		u64 count : 40;
	} s;
	struct cvmx_pko_l1_sqx_yellow_packets_s cn73xx;
	struct cvmx_pko_l1_sqx_yellow_packets_s cn78xx;
	struct cvmx_pko_l1_sqx_yellow_packets_s cn78xxp1;
	struct cvmx_pko_l1_sqx_yellow_packets_s cnf75xx;
};

typedef union cvmx_pko_l1_sqx_yellow_packets cvmx_pko_l1_sqx_yellow_packets_t;

/**
 * cvmx_pko_l1_sq_csr_bus_debug
 */
union cvmx_pko_l1_sq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_l1_sq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_l1_sq_csr_bus_debug_s cn73xx;
	struct cvmx_pko_l1_sq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_l1_sq_csr_bus_debug_s cn78xxp1;
	struct cvmx_pko_l1_sq_csr_bus_debug_s cnf75xx;
};

typedef union cvmx_pko_l1_sq_csr_bus_debug cvmx_pko_l1_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l1_sqa_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l1_sqa_debug {
	u64 u64;
	struct cvmx_pko_l1_sqa_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l1_sqa_debug_s cn73xx;
	struct cvmx_pko_l1_sqa_debug_s cn78xx;
	struct cvmx_pko_l1_sqa_debug_s cn78xxp1;
	struct cvmx_pko_l1_sqa_debug_s cnf75xx;
};

typedef union cvmx_pko_l1_sqa_debug cvmx_pko_l1_sqa_debug_t;

/**
 * cvmx_pko_l1_sqb_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l1_sqb_debug {
	u64 u64;
	struct cvmx_pko_l1_sqb_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l1_sqb_debug_s cn73xx;
	struct cvmx_pko_l1_sqb_debug_s cn78xx;
	struct cvmx_pko_l1_sqb_debug_s cn78xxp1;
	struct cvmx_pko_l1_sqb_debug_s cnf75xx;
};

typedef union cvmx_pko_l1_sqb_debug cvmx_pko_l1_sqb_debug_t;

/**
 * cvmx_pko_l2_sq#_cir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l2_sqx_cir {
	u64 u64;
	struct cvmx_pko_l2_sqx_cir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l2_sqx_cir_s cn73xx;
	struct cvmx_pko_l2_sqx_cir_s cn78xx;
	struct cvmx_pko_l2_sqx_cir_s cn78xxp1;
	struct cvmx_pko_l2_sqx_cir_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_cir cvmx_pko_l2_sqx_cir_t;

/**
 * cvmx_pko_l2_sq#_green
 *
 * This register has the same bit fields as PKO_L1_SQ()_GREEN.
 *
 */
union cvmx_pko_l2_sqx_green {
	u64 u64;
	struct cvmx_pko_l2_sqx_green_s {
		u64 reserved_41_63 : 23;
		u64 rr_active : 1;
		u64 active_vec : 20;
		u64 reserved_19_19 : 1;
		u64 head : 9;
		u64 reserved_9_9 : 1;
		u64 tail : 9;
	} s;
	struct cvmx_pko_l2_sqx_green_s cn73xx;
	struct cvmx_pko_l2_sqx_green_s cn78xx;
	struct cvmx_pko_l2_sqx_green_s cn78xxp1;
	struct cvmx_pko_l2_sqx_green_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_green cvmx_pko_l2_sqx_green_t;

/**
 * cvmx_pko_l2_sq#_pick
 *
 * This CSR contains the meta for the L2 SQ, and is for debug and reconfiguration
 * only and should never be written. See also PKO_META_DESC_S.
 */
union cvmx_pko_l2_sqx_pick {
	u64 u64;
	struct cvmx_pko_l2_sqx_pick_s {
		u64 dq : 10;
		u64 color : 2;
		u64 child : 10;
		u64 bubble : 1;
		u64 p_con : 1;
		u64 c_con : 1;
		u64 uid : 7;
		u64 jump : 1;
		u64 fpd : 1;
		u64 ds : 1;
		u64 adjust : 9;
		u64 pir_dis : 1;
		u64 cir_dis : 1;
		u64 red_algo_override : 2;
		u64 length : 16;
	} s;
	struct cvmx_pko_l2_sqx_pick_s cn73xx;
	struct cvmx_pko_l2_sqx_pick_s cn78xx;
	struct cvmx_pko_l2_sqx_pick_s cn78xxp1;
	struct cvmx_pko_l2_sqx_pick_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_pick cvmx_pko_l2_sqx_pick_t;

/**
 * cvmx_pko_l2_sq#_pir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l2_sqx_pir {
	u64 u64;
	struct cvmx_pko_l2_sqx_pir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l2_sqx_pir_s cn73xx;
	struct cvmx_pko_l2_sqx_pir_s cn78xx;
	struct cvmx_pko_l2_sqx_pir_s cn78xxp1;
	struct cvmx_pko_l2_sqx_pir_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_pir cvmx_pko_l2_sqx_pir_t;

/**
 * cvmx_pko_l2_sq#_pointers
 */
union cvmx_pko_l2_sqx_pointers {
	u64 u64;
	struct cvmx_pko_l2_sqx_pointers_s {
		u64 reserved_25_63 : 39;
		u64 prev : 9;
		u64 reserved_9_15 : 7;
		u64 next : 9;
	} s;
	struct cvmx_pko_l2_sqx_pointers_cn73xx {
		u64 reserved_24_63 : 40;
		u64 prev : 8;
		u64 reserved_8_15 : 8;
		u64 next : 8;
	} cn73xx;
	struct cvmx_pko_l2_sqx_pointers_s cn78xx;
	struct cvmx_pko_l2_sqx_pointers_s cn78xxp1;
	struct cvmx_pko_l2_sqx_pointers_cn73xx cnf75xx;
};

typedef union cvmx_pko_l2_sqx_pointers cvmx_pko_l2_sqx_pointers_t;

/**
 * cvmx_pko_l2_sq#_red
 *
 * This register has the same bit fields as PKO_L1_SQ()_RED.
 *
 */
union cvmx_pko_l2_sqx_red {
	u64 u64;
	struct cvmx_pko_l2_sqx_red_s {
		u64 reserved_19_63 : 45;
		u64 head : 9;
		u64 reserved_9_9 : 1;
		u64 tail : 9;
	} s;
	struct cvmx_pko_l2_sqx_red_s cn73xx;
	struct cvmx_pko_l2_sqx_red_s cn78xx;
	struct cvmx_pko_l2_sqx_red_s cn78xxp1;
	struct cvmx_pko_l2_sqx_red_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_red cvmx_pko_l2_sqx_red_t;

/**
 * cvmx_pko_l2_sq#_sched_state
 */
union cvmx_pko_l2_sqx_sched_state {
	u64 u64;
	struct cvmx_pko_l2_sqx_sched_state_s {
		u64 reserved_25_63 : 39;
		u64 rr_count : 25;
	} s;
	struct cvmx_pko_l2_sqx_sched_state_s cn73xx;
	struct cvmx_pko_l2_sqx_sched_state_s cn78xx;
	struct cvmx_pko_l2_sqx_sched_state_s cn78xxp1;
	struct cvmx_pko_l2_sqx_sched_state_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_sched_state cvmx_pko_l2_sqx_sched_state_t;

/**
 * cvmx_pko_l2_sq#_schedule
 */
union cvmx_pko_l2_sqx_schedule {
	u64 u64;
	struct cvmx_pko_l2_sqx_schedule_s {
		u64 reserved_28_63 : 36;
		u64 prio : 4;
		u64 rr_quantum : 24;
	} s;
	struct cvmx_pko_l2_sqx_schedule_s cn73xx;
	struct cvmx_pko_l2_sqx_schedule_s cn78xx;
	struct cvmx_pko_l2_sqx_schedule_s cn78xxp1;
	struct cvmx_pko_l2_sqx_schedule_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_schedule cvmx_pko_l2_sqx_schedule_t;

/**
 * cvmx_pko_l2_sq#_shape
 */
union cvmx_pko_l2_sqx_shape {
	u64 u64;
	struct cvmx_pko_l2_sqx_shape_s {
		u64 reserved_27_63 : 37;
		u64 schedule_list : 2;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} s;
	struct cvmx_pko_l2_sqx_shape_s cn73xx;
	struct cvmx_pko_l2_sqx_shape_cn78xx {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} cn78xx;
	struct cvmx_pko_l2_sqx_shape_cn78xx cn78xxp1;
	struct cvmx_pko_l2_sqx_shape_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_shape cvmx_pko_l2_sqx_shape_t;

/**
 * cvmx_pko_l2_sq#_shape_state
 *
 * This register must not be written during normal operation.
 *
 */
union cvmx_pko_l2_sqx_shape_state {
	u64 u64;
	struct cvmx_pko_l2_sqx_shape_state_s {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 color : 2;
		u64 pir_accum : 26;
		u64 cir_accum : 26;
	} s;
	struct cvmx_pko_l2_sqx_shape_state_s cn73xx;
	struct cvmx_pko_l2_sqx_shape_state_s cn78xx;
	struct cvmx_pko_l2_sqx_shape_state_s cn78xxp1;
	struct cvmx_pko_l2_sqx_shape_state_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_shape_state cvmx_pko_l2_sqx_shape_state_t;

/**
 * cvmx_pko_l2_sq#_sw_xoff
 *
 * This register has the same bit fields as PKO_L1_SQ()_SW_XOFF.
 *
 */
union cvmx_pko_l2_sqx_sw_xoff {
	u64 u64;
	struct cvmx_pko_l2_sqx_sw_xoff_s {
		u64 reserved_4_63 : 60;
		u64 drain_irq : 1;
		u64 drain_null_link : 1;
		u64 drain : 1;
		u64 xoff : 1;
	} s;
	struct cvmx_pko_l2_sqx_sw_xoff_s cn73xx;
	struct cvmx_pko_l2_sqx_sw_xoff_s cn78xx;
	struct cvmx_pko_l2_sqx_sw_xoff_s cn78xxp1;
	struct cvmx_pko_l2_sqx_sw_xoff_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_sw_xoff cvmx_pko_l2_sqx_sw_xoff_t;

/**
 * cvmx_pko_l2_sq#_topology
 */
union cvmx_pko_l2_sqx_topology {
	u64 u64;
	struct cvmx_pko_l2_sqx_topology_s {
		u64 reserved_41_63 : 23;
		u64 prio_anchor : 9;
		u64 reserved_21_31 : 11;
		u64 parent : 5;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_l2_sqx_topology_cn73xx {
		u64 reserved_40_63 : 24;
		u64 prio_anchor : 8;
		u64 reserved_20_31 : 12;
		u64 parent : 4;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_l2_sqx_topology_s cn78xx;
	struct cvmx_pko_l2_sqx_topology_s cn78xxp1;
	struct cvmx_pko_l2_sqx_topology_cn73xx cnf75xx;
};

typedef union cvmx_pko_l2_sqx_topology cvmx_pko_l2_sqx_topology_t;

/**
 * cvmx_pko_l2_sq#_yellow
 *
 * This register has the same bit fields as PKO_L1_SQ()_YELLOW.
 *
 */
union cvmx_pko_l2_sqx_yellow {
	u64 u64;
	struct cvmx_pko_l2_sqx_yellow_s {
		u64 reserved_19_63 : 45;
		u64 head : 9;
		u64 reserved_9_9 : 1;
		u64 tail : 9;
	} s;
	struct cvmx_pko_l2_sqx_yellow_s cn73xx;
	struct cvmx_pko_l2_sqx_yellow_s cn78xx;
	struct cvmx_pko_l2_sqx_yellow_s cn78xxp1;
	struct cvmx_pko_l2_sqx_yellow_s cnf75xx;
};

typedef union cvmx_pko_l2_sqx_yellow cvmx_pko_l2_sqx_yellow_t;

/**
 * cvmx_pko_l2_sq_csr_bus_debug
 */
union cvmx_pko_l2_sq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_l2_sq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_l2_sq_csr_bus_debug_s cn73xx;
	struct cvmx_pko_l2_sq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_l2_sq_csr_bus_debug_s cn78xxp1;
	struct cvmx_pko_l2_sq_csr_bus_debug_s cnf75xx;
};

typedef union cvmx_pko_l2_sq_csr_bus_debug cvmx_pko_l2_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l2_sqa_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l2_sqa_debug {
	u64 u64;
	struct cvmx_pko_l2_sqa_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l2_sqa_debug_s cn73xx;
	struct cvmx_pko_l2_sqa_debug_s cn78xx;
	struct cvmx_pko_l2_sqa_debug_s cn78xxp1;
	struct cvmx_pko_l2_sqa_debug_s cnf75xx;
};

typedef union cvmx_pko_l2_sqa_debug cvmx_pko_l2_sqa_debug_t;

/**
 * cvmx_pko_l2_sqb_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l2_sqb_debug {
	u64 u64;
	struct cvmx_pko_l2_sqb_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l2_sqb_debug_s cn73xx;
	struct cvmx_pko_l2_sqb_debug_s cn78xx;
	struct cvmx_pko_l2_sqb_debug_s cn78xxp1;
	struct cvmx_pko_l2_sqb_debug_s cnf75xx;
};

typedef union cvmx_pko_l2_sqb_debug cvmx_pko_l2_sqb_debug_t;

/**
 * cvmx_pko_l3_l2_sq#_channel
 *
 * PKO_CHANNEL_LEVEL[CC_LEVEL] determines whether this CSR array is associated to
 * the L2 SQs or the L3 SQs.
 */
union cvmx_pko_l3_l2_sqx_channel {
	u64 u64;
	struct cvmx_pko_l3_l2_sqx_channel_s {
		u64 reserved_44_63 : 20;
		u64 cc_channel : 12;
		u64 cc_word_cnt : 20;
		u64 cc_packet_cnt : 10;
		u64 cc_enable : 1;
		u64 hw_xoff : 1;
	} s;
	struct cvmx_pko_l3_l2_sqx_channel_s cn73xx;
	struct cvmx_pko_l3_l2_sqx_channel_s cn78xx;
	struct cvmx_pko_l3_l2_sqx_channel_s cn78xxp1;
	struct cvmx_pko_l3_l2_sqx_channel_s cnf75xx;
};

typedef union cvmx_pko_l3_l2_sqx_channel cvmx_pko_l3_l2_sqx_channel_t;

/**
 * cvmx_pko_l3_sq#_cir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l3_sqx_cir {
	u64 u64;
	struct cvmx_pko_l3_sqx_cir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l3_sqx_cir_s cn73xx;
	struct cvmx_pko_l3_sqx_cir_s cn78xx;
	struct cvmx_pko_l3_sqx_cir_s cn78xxp1;
	struct cvmx_pko_l3_sqx_cir_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_cir cvmx_pko_l3_sqx_cir_t;

/**
 * cvmx_pko_l3_sq#_green
 */
union cvmx_pko_l3_sqx_green {
	u64 u64;
	struct cvmx_pko_l3_sqx_green_s {
		u64 reserved_41_63 : 23;
		u64 rr_active : 1;
		u64 active_vec : 20;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l3_sqx_green_cn73xx {
		u64 reserved_41_63 : 23;
		u64 rr_active : 1;
		u64 active_vec : 20;
		u64 reserved_18_19 : 2;
		u64 head : 8;
		u64 reserved_8_9 : 2;
		u64 tail : 8;
	} cn73xx;
	struct cvmx_pko_l3_sqx_green_s cn78xx;
	struct cvmx_pko_l3_sqx_green_s cn78xxp1;
	struct cvmx_pko_l3_sqx_green_cn73xx cnf75xx;
};

typedef union cvmx_pko_l3_sqx_green cvmx_pko_l3_sqx_green_t;

/**
 * cvmx_pko_l3_sq#_pick
 *
 * This CSR contains the meta for the L3 SQ, and is for debug and reconfiguration
 * only and should never be written. See also PKO_META_DESC_S.
 */
union cvmx_pko_l3_sqx_pick {
	u64 u64;
	struct cvmx_pko_l3_sqx_pick_s {
		u64 dq : 10;
		u64 color : 2;
		u64 child : 10;
		u64 bubble : 1;
		u64 p_con : 1;
		u64 c_con : 1;
		u64 uid : 7;
		u64 jump : 1;
		u64 fpd : 1;
		u64 ds : 1;
		u64 adjust : 9;
		u64 pir_dis : 1;
		u64 cir_dis : 1;
		u64 red_algo_override : 2;
		u64 length : 16;
	} s;
	struct cvmx_pko_l3_sqx_pick_s cn73xx;
	struct cvmx_pko_l3_sqx_pick_s cn78xx;
	struct cvmx_pko_l3_sqx_pick_s cn78xxp1;
	struct cvmx_pko_l3_sqx_pick_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_pick cvmx_pko_l3_sqx_pick_t;

/**
 * cvmx_pko_l3_sq#_pir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l3_sqx_pir {
	u64 u64;
	struct cvmx_pko_l3_sqx_pir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l3_sqx_pir_s cn73xx;
	struct cvmx_pko_l3_sqx_pir_s cn78xx;
	struct cvmx_pko_l3_sqx_pir_s cn78xxp1;
	struct cvmx_pko_l3_sqx_pir_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_pir cvmx_pko_l3_sqx_pir_t;

/**
 * cvmx_pko_l3_sq#_pointers
 *
 * This register has the same bit fields as PKO_L2_SQ()_POINTERS.
 *
 */
union cvmx_pko_l3_sqx_pointers {
	u64 u64;
	struct cvmx_pko_l3_sqx_pointers_s {
		u64 reserved_25_63 : 39;
		u64 prev : 9;
		u64 reserved_9_15 : 7;
		u64 next : 9;
	} s;
	struct cvmx_pko_l3_sqx_pointers_cn73xx {
		u64 reserved_24_63 : 40;
		u64 prev : 8;
		u64 reserved_8_15 : 8;
		u64 next : 8;
	} cn73xx;
	struct cvmx_pko_l3_sqx_pointers_s cn78xx;
	struct cvmx_pko_l3_sqx_pointers_s cn78xxp1;
	struct cvmx_pko_l3_sqx_pointers_cn73xx cnf75xx;
};

typedef union cvmx_pko_l3_sqx_pointers cvmx_pko_l3_sqx_pointers_t;

/**
 * cvmx_pko_l3_sq#_red
 *
 * This register has the same bit fields as PKO_L3_SQ()_YELLOW.
 *
 */
union cvmx_pko_l3_sqx_red {
	u64 u64;
	struct cvmx_pko_l3_sqx_red_s {
		u64 reserved_20_63 : 44;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l3_sqx_red_cn73xx {
		u64 reserved_18_63 : 46;
		u64 head : 8;
		u64 reserved_8_9 : 2;
		u64 tail : 8;
	} cn73xx;
	struct cvmx_pko_l3_sqx_red_s cn78xx;
	struct cvmx_pko_l3_sqx_red_s cn78xxp1;
	struct cvmx_pko_l3_sqx_red_cn73xx cnf75xx;
};

typedef union cvmx_pko_l3_sqx_red cvmx_pko_l3_sqx_red_t;

/**
 * cvmx_pko_l3_sq#_sched_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHED_STATE.
 *
 */
union cvmx_pko_l3_sqx_sched_state {
	u64 u64;
	struct cvmx_pko_l3_sqx_sched_state_s {
		u64 reserved_25_63 : 39;
		u64 rr_count : 25;
	} s;
	struct cvmx_pko_l3_sqx_sched_state_s cn73xx;
	struct cvmx_pko_l3_sqx_sched_state_s cn78xx;
	struct cvmx_pko_l3_sqx_sched_state_s cn78xxp1;
	struct cvmx_pko_l3_sqx_sched_state_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_sched_state cvmx_pko_l3_sqx_sched_state_t;

/**
 * cvmx_pko_l3_sq#_schedule
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHEDULE.
 *
 */
union cvmx_pko_l3_sqx_schedule {
	u64 u64;
	struct cvmx_pko_l3_sqx_schedule_s {
		u64 reserved_28_63 : 36;
		u64 prio : 4;
		u64 rr_quantum : 24;
	} s;
	struct cvmx_pko_l3_sqx_schedule_s cn73xx;
	struct cvmx_pko_l3_sqx_schedule_s cn78xx;
	struct cvmx_pko_l3_sqx_schedule_s cn78xxp1;
	struct cvmx_pko_l3_sqx_schedule_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_schedule cvmx_pko_l3_sqx_schedule_t;

/**
 * cvmx_pko_l3_sq#_shape
 */
union cvmx_pko_l3_sqx_shape {
	u64 u64;
	struct cvmx_pko_l3_sqx_shape_s {
		u64 reserved_27_63 : 37;
		u64 schedule_list : 2;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} s;
	struct cvmx_pko_l3_sqx_shape_s cn73xx;
	struct cvmx_pko_l3_sqx_shape_cn78xx {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} cn78xx;
	struct cvmx_pko_l3_sqx_shape_cn78xx cn78xxp1;
	struct cvmx_pko_l3_sqx_shape_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_shape cvmx_pko_l3_sqx_shape_t;

/**
 * cvmx_pko_l3_sq#_shape_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cvmx_pko_l3_sqx_shape_state {
	u64 u64;
	struct cvmx_pko_l3_sqx_shape_state_s {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 color : 2;
		u64 pir_accum : 26;
		u64 cir_accum : 26;
	} s;
	struct cvmx_pko_l3_sqx_shape_state_s cn73xx;
	struct cvmx_pko_l3_sqx_shape_state_s cn78xx;
	struct cvmx_pko_l3_sqx_shape_state_s cn78xxp1;
	struct cvmx_pko_l3_sqx_shape_state_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_shape_state cvmx_pko_l3_sqx_shape_state_t;

/**
 * cvmx_pko_l3_sq#_sw_xoff
 *
 * This register has the same bit fields as PKO_L1_SQ()_SW_XOFF
 *
 */
union cvmx_pko_l3_sqx_sw_xoff {
	u64 u64;
	struct cvmx_pko_l3_sqx_sw_xoff_s {
		u64 reserved_4_63 : 60;
		u64 drain_irq : 1;
		u64 drain_null_link : 1;
		u64 drain : 1;
		u64 xoff : 1;
	} s;
	struct cvmx_pko_l3_sqx_sw_xoff_s cn73xx;
	struct cvmx_pko_l3_sqx_sw_xoff_s cn78xx;
	struct cvmx_pko_l3_sqx_sw_xoff_s cn78xxp1;
	struct cvmx_pko_l3_sqx_sw_xoff_s cnf75xx;
};

typedef union cvmx_pko_l3_sqx_sw_xoff cvmx_pko_l3_sqx_sw_xoff_t;

/**
 * cvmx_pko_l3_sq#_topology
 */
union cvmx_pko_l3_sqx_topology {
	u64 u64;
	struct cvmx_pko_l3_sqx_topology_s {
		u64 reserved_42_63 : 22;
		u64 prio_anchor : 10;
		u64 reserved_25_31 : 7;
		u64 parent : 9;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_l3_sqx_topology_cn73xx {
		u64 reserved_40_63 : 24;
		u64 prio_anchor : 8;
		u64 reserved_24_31 : 8;
		u64 parent : 8;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_l3_sqx_topology_s cn78xx;
	struct cvmx_pko_l3_sqx_topology_s cn78xxp1;
	struct cvmx_pko_l3_sqx_topology_cn73xx cnf75xx;
};

typedef union cvmx_pko_l3_sqx_topology cvmx_pko_l3_sqx_topology_t;

/**
 * cvmx_pko_l3_sq#_yellow
 */
union cvmx_pko_l3_sqx_yellow {
	u64 u64;
	struct cvmx_pko_l3_sqx_yellow_s {
		u64 reserved_20_63 : 44;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l3_sqx_yellow_cn73xx {
		u64 reserved_18_63 : 46;
		u64 head : 8;
		u64 reserved_8_9 : 2;
		u64 tail : 8;
	} cn73xx;
	struct cvmx_pko_l3_sqx_yellow_s cn78xx;
	struct cvmx_pko_l3_sqx_yellow_s cn78xxp1;
	struct cvmx_pko_l3_sqx_yellow_cn73xx cnf75xx;
};

typedef union cvmx_pko_l3_sqx_yellow cvmx_pko_l3_sqx_yellow_t;

/**
 * cvmx_pko_l3_sq_csr_bus_debug
 */
union cvmx_pko_l3_sq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_l3_sq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_l3_sq_csr_bus_debug_s cn73xx;
	struct cvmx_pko_l3_sq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_l3_sq_csr_bus_debug_s cn78xxp1;
	struct cvmx_pko_l3_sq_csr_bus_debug_s cnf75xx;
};

typedef union cvmx_pko_l3_sq_csr_bus_debug cvmx_pko_l3_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l3_sqa_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l3_sqa_debug {
	u64 u64;
	struct cvmx_pko_l3_sqa_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l3_sqa_debug_s cn73xx;
	struct cvmx_pko_l3_sqa_debug_s cn78xx;
	struct cvmx_pko_l3_sqa_debug_s cn78xxp1;
	struct cvmx_pko_l3_sqa_debug_s cnf75xx;
};

typedef union cvmx_pko_l3_sqa_debug cvmx_pko_l3_sqa_debug_t;

/**
 * cvmx_pko_l3_sqb_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l3_sqb_debug {
	u64 u64;
	struct cvmx_pko_l3_sqb_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l3_sqb_debug_s cn73xx;
	struct cvmx_pko_l3_sqb_debug_s cn78xx;
	struct cvmx_pko_l3_sqb_debug_s cn78xxp1;
	struct cvmx_pko_l3_sqb_debug_s cnf75xx;
};

typedef union cvmx_pko_l3_sqb_debug cvmx_pko_l3_sqb_debug_t;

/**
 * cvmx_pko_l4_sq#_cir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l4_sqx_cir {
	u64 u64;
	struct cvmx_pko_l4_sqx_cir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l4_sqx_cir_s cn78xx;
	struct cvmx_pko_l4_sqx_cir_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_cir cvmx_pko_l4_sqx_cir_t;

/**
 * cvmx_pko_l4_sq#_green
 *
 * This register has the same bit fields as PKO_L3_SQ()_GREEN.
 *
 */
union cvmx_pko_l4_sqx_green {
	u64 u64;
	struct cvmx_pko_l4_sqx_green_s {
		u64 reserved_41_63 : 23;
		u64 rr_active : 1;
		u64 active_vec : 20;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l4_sqx_green_s cn78xx;
	struct cvmx_pko_l4_sqx_green_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_green cvmx_pko_l4_sqx_green_t;

/**
 * cvmx_pko_l4_sq#_pick
 *
 * This CSR contains the meta for the L4 SQ, and is for debug and reconfiguration
 * only and should never be written. See also PKO_META_DESC_S.
 */
union cvmx_pko_l4_sqx_pick {
	u64 u64;
	struct cvmx_pko_l4_sqx_pick_s {
		u64 dq : 10;
		u64 color : 2;
		u64 child : 10;
		u64 bubble : 1;
		u64 p_con : 1;
		u64 c_con : 1;
		u64 uid : 7;
		u64 jump : 1;
		u64 fpd : 1;
		u64 ds : 1;
		u64 adjust : 9;
		u64 pir_dis : 1;
		u64 cir_dis : 1;
		u64 red_algo_override : 2;
		u64 length : 16;
	} s;
	struct cvmx_pko_l4_sqx_pick_s cn78xx;
	struct cvmx_pko_l4_sqx_pick_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_pick cvmx_pko_l4_sqx_pick_t;

/**
 * cvmx_pko_l4_sq#_pir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l4_sqx_pir {
	u64 u64;
	struct cvmx_pko_l4_sqx_pir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l4_sqx_pir_s cn78xx;
	struct cvmx_pko_l4_sqx_pir_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_pir cvmx_pko_l4_sqx_pir_t;

/**
 * cvmx_pko_l4_sq#_pointers
 */
union cvmx_pko_l4_sqx_pointers {
	u64 u64;
	struct cvmx_pko_l4_sqx_pointers_s {
		u64 reserved_26_63 : 38;
		u64 prev : 10;
		u64 reserved_10_15 : 6;
		u64 next : 10;
	} s;
	struct cvmx_pko_l4_sqx_pointers_s cn78xx;
	struct cvmx_pko_l4_sqx_pointers_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_pointers cvmx_pko_l4_sqx_pointers_t;

/**
 * cvmx_pko_l4_sq#_red
 *
 * This register has the same bit fields as PKO_L3_SQ()_YELLOW.
 *
 */
union cvmx_pko_l4_sqx_red {
	u64 u64;
	struct cvmx_pko_l4_sqx_red_s {
		u64 reserved_20_63 : 44;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l4_sqx_red_s cn78xx;
	struct cvmx_pko_l4_sqx_red_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_red cvmx_pko_l4_sqx_red_t;

/**
 * cvmx_pko_l4_sq#_sched_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHED_STATE.
 *
 */
union cvmx_pko_l4_sqx_sched_state {
	u64 u64;
	struct cvmx_pko_l4_sqx_sched_state_s {
		u64 reserved_25_63 : 39;
		u64 rr_count : 25;
	} s;
	struct cvmx_pko_l4_sqx_sched_state_s cn78xx;
	struct cvmx_pko_l4_sqx_sched_state_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_sched_state cvmx_pko_l4_sqx_sched_state_t;

/**
 * cvmx_pko_l4_sq#_schedule
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHEDULE.
 *
 */
union cvmx_pko_l4_sqx_schedule {
	u64 u64;
	struct cvmx_pko_l4_sqx_schedule_s {
		u64 reserved_28_63 : 36;
		u64 prio : 4;
		u64 rr_quantum : 24;
	} s;
	struct cvmx_pko_l4_sqx_schedule_s cn78xx;
	struct cvmx_pko_l4_sqx_schedule_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_schedule cvmx_pko_l4_sqx_schedule_t;

/**
 * cvmx_pko_l4_sq#_shape
 *
 * This register has the same bit fields as PKO_L3_SQ()_SHAPE.
 *
 */
union cvmx_pko_l4_sqx_shape {
	u64 u64;
	struct cvmx_pko_l4_sqx_shape_s {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} s;
	struct cvmx_pko_l4_sqx_shape_s cn78xx;
	struct cvmx_pko_l4_sqx_shape_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_shape cvmx_pko_l4_sqx_shape_t;

/**
 * cvmx_pko_l4_sq#_shape_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cvmx_pko_l4_sqx_shape_state {
	u64 u64;
	struct cvmx_pko_l4_sqx_shape_state_s {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 color : 2;
		u64 pir_accum : 26;
		u64 cir_accum : 26;
	} s;
	struct cvmx_pko_l4_sqx_shape_state_s cn78xx;
	struct cvmx_pko_l4_sqx_shape_state_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_shape_state cvmx_pko_l4_sqx_shape_state_t;

/**
 * cvmx_pko_l4_sq#_sw_xoff
 *
 * This register has the same bit fields as PKO_L1_SQ()_SW_XOFF.
 *
 */
union cvmx_pko_l4_sqx_sw_xoff {
	u64 u64;
	struct cvmx_pko_l4_sqx_sw_xoff_s {
		u64 reserved_4_63 : 60;
		u64 drain_irq : 1;
		u64 drain_null_link : 1;
		u64 drain : 1;
		u64 xoff : 1;
	} s;
	struct cvmx_pko_l4_sqx_sw_xoff_s cn78xx;
	struct cvmx_pko_l4_sqx_sw_xoff_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_sw_xoff cvmx_pko_l4_sqx_sw_xoff_t;

/**
 * cvmx_pko_l4_sq#_topology
 */
union cvmx_pko_l4_sqx_topology {
	u64 u64;
	struct cvmx_pko_l4_sqx_topology_s {
		u64 reserved_42_63 : 22;
		u64 prio_anchor : 10;
		u64 reserved_25_31 : 7;
		u64 parent : 9;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_l4_sqx_topology_s cn78xx;
	struct cvmx_pko_l4_sqx_topology_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_topology cvmx_pko_l4_sqx_topology_t;

/**
 * cvmx_pko_l4_sq#_yellow
 *
 * This register has the same bit fields as PKO_L3_SQ()_YELLOW.
 *
 */
union cvmx_pko_l4_sqx_yellow {
	u64 u64;
	struct cvmx_pko_l4_sqx_yellow_s {
		u64 reserved_20_63 : 44;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l4_sqx_yellow_s cn78xx;
	struct cvmx_pko_l4_sqx_yellow_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqx_yellow cvmx_pko_l4_sqx_yellow_t;

/**
 * cvmx_pko_l4_sq_csr_bus_debug
 */
union cvmx_pko_l4_sq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_l4_sq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_l4_sq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_l4_sq_csr_bus_debug_s cn78xxp1;
};

typedef union cvmx_pko_l4_sq_csr_bus_debug cvmx_pko_l4_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l4_sqa_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l4_sqa_debug {
	u64 u64;
	struct cvmx_pko_l4_sqa_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l4_sqa_debug_s cn78xx;
	struct cvmx_pko_l4_sqa_debug_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqa_debug cvmx_pko_l4_sqa_debug_t;

/**
 * cvmx_pko_l4_sqb_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l4_sqb_debug {
	u64 u64;
	struct cvmx_pko_l4_sqb_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l4_sqb_debug_s cn78xx;
	struct cvmx_pko_l4_sqb_debug_s cn78xxp1;
};

typedef union cvmx_pko_l4_sqb_debug cvmx_pko_l4_sqb_debug_t;

/**
 * cvmx_pko_l5_sq#_cir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l5_sqx_cir {
	u64 u64;
	struct cvmx_pko_l5_sqx_cir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l5_sqx_cir_s cn78xx;
	struct cvmx_pko_l5_sqx_cir_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_cir cvmx_pko_l5_sqx_cir_t;

/**
 * cvmx_pko_l5_sq#_green
 *
 * This register has the same bit fields as PKO_L3_SQ()_GREEN.
 *
 */
union cvmx_pko_l5_sqx_green {
	u64 u64;
	struct cvmx_pko_l5_sqx_green_s {
		u64 reserved_41_63 : 23;
		u64 rr_active : 1;
		u64 active_vec : 20;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l5_sqx_green_s cn78xx;
	struct cvmx_pko_l5_sqx_green_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_green cvmx_pko_l5_sqx_green_t;

/**
 * cvmx_pko_l5_sq#_pick
 *
 * This CSR contains the meta for the L5 SQ, and is for debug and reconfiguration
 * only and should never be written. See also PKO_META_DESC_S.
 */
union cvmx_pko_l5_sqx_pick {
	u64 u64;
	struct cvmx_pko_l5_sqx_pick_s {
		u64 dq : 10;
		u64 color : 2;
		u64 child : 10;
		u64 bubble : 1;
		u64 p_con : 1;
		u64 c_con : 1;
		u64 uid : 7;
		u64 jump : 1;
		u64 fpd : 1;
		u64 ds : 1;
		u64 adjust : 9;
		u64 pir_dis : 1;
		u64 cir_dis : 1;
		u64 red_algo_override : 2;
		u64 length : 16;
	} s;
	struct cvmx_pko_l5_sqx_pick_s cn78xx;
	struct cvmx_pko_l5_sqx_pick_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_pick cvmx_pko_l5_sqx_pick_t;

/**
 * cvmx_pko_l5_sq#_pir
 *
 * This register has the same bit fields as PKO_L1_SQ()_CIR.
 *
 */
union cvmx_pko_l5_sqx_pir {
	u64 u64;
	struct cvmx_pko_l5_sqx_pir_s {
		u64 reserved_41_63 : 23;
		u64 burst_exponent : 4;
		u64 burst_mantissa : 8;
		u64 reserved_17_28 : 12;
		u64 rate_divider_exponent : 4;
		u64 rate_exponent : 4;
		u64 rate_mantissa : 8;
		u64 enable : 1;
	} s;
	struct cvmx_pko_l5_sqx_pir_s cn78xx;
	struct cvmx_pko_l5_sqx_pir_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_pir cvmx_pko_l5_sqx_pir_t;

/**
 * cvmx_pko_l5_sq#_pointers
 *
 * This register has the same bit fields as PKO_L4_SQ()_POINTERS.
 *
 */
union cvmx_pko_l5_sqx_pointers {
	u64 u64;
	struct cvmx_pko_l5_sqx_pointers_s {
		u64 reserved_26_63 : 38;
		u64 prev : 10;
		u64 reserved_10_15 : 6;
		u64 next : 10;
	} s;
	struct cvmx_pko_l5_sqx_pointers_s cn78xx;
	struct cvmx_pko_l5_sqx_pointers_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_pointers cvmx_pko_l5_sqx_pointers_t;

/**
 * cvmx_pko_l5_sq#_red
 *
 * This register has the same bit fields as PKO_L3_SQ()_YELLOW.
 *
 */
union cvmx_pko_l5_sqx_red {
	u64 u64;
	struct cvmx_pko_l5_sqx_red_s {
		u64 reserved_20_63 : 44;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l5_sqx_red_s cn78xx;
	struct cvmx_pko_l5_sqx_red_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_red cvmx_pko_l5_sqx_red_t;

/**
 * cvmx_pko_l5_sq#_sched_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHED_STATE.
 *
 */
union cvmx_pko_l5_sqx_sched_state {
	u64 u64;
	struct cvmx_pko_l5_sqx_sched_state_s {
		u64 reserved_25_63 : 39;
		u64 rr_count : 25;
	} s;
	struct cvmx_pko_l5_sqx_sched_state_s cn78xx;
	struct cvmx_pko_l5_sqx_sched_state_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_sched_state cvmx_pko_l5_sqx_sched_state_t;

/**
 * cvmx_pko_l5_sq#_schedule
 *
 * This register has the same bit fields as PKO_L2_SQ()_SCHEDULE.
 *
 */
union cvmx_pko_l5_sqx_schedule {
	u64 u64;
	struct cvmx_pko_l5_sqx_schedule_s {
		u64 reserved_28_63 : 36;
		u64 prio : 4;
		u64 rr_quantum : 24;
	} s;
	struct cvmx_pko_l5_sqx_schedule_s cn78xx;
	struct cvmx_pko_l5_sqx_schedule_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_schedule cvmx_pko_l5_sqx_schedule_t;

/**
 * cvmx_pko_l5_sq#_shape
 */
union cvmx_pko_l5_sqx_shape {
	u64 u64;
	struct cvmx_pko_l5_sqx_shape_s {
		u64 reserved_25_63 : 39;
		u64 length_disable : 1;
		u64 reserved_13_23 : 11;
		u64 yellow_disable : 1;
		u64 red_disable : 1;
		u64 red_algo : 2;
		u64 adjust : 9;
	} s;
	struct cvmx_pko_l5_sqx_shape_s cn78xx;
	struct cvmx_pko_l5_sqx_shape_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_shape cvmx_pko_l5_sqx_shape_t;

/**
 * cvmx_pko_l5_sq#_shape_state
 *
 * This register has the same bit fields as PKO_L2_SQ()_SHAPE_STATE.
 * This register must not be written during normal operation.
 */
union cvmx_pko_l5_sqx_shape_state {
	u64 u64;
	struct cvmx_pko_l5_sqx_shape_state_s {
		u64 reserved_60_63 : 4;
		u64 tw_timestamp : 6;
		u64 color : 2;
		u64 pir_accum : 26;
		u64 cir_accum : 26;
	} s;
	struct cvmx_pko_l5_sqx_shape_state_s cn78xx;
	struct cvmx_pko_l5_sqx_shape_state_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_shape_state cvmx_pko_l5_sqx_shape_state_t;

/**
 * cvmx_pko_l5_sq#_sw_xoff
 *
 * This register has the same bit fields as PKO_L1_SQ()_SW_XOFF.
 *
 */
union cvmx_pko_l5_sqx_sw_xoff {
	u64 u64;
	struct cvmx_pko_l5_sqx_sw_xoff_s {
		u64 reserved_4_63 : 60;
		u64 drain_irq : 1;
		u64 drain_null_link : 1;
		u64 drain : 1;
		u64 xoff : 1;
	} s;
	struct cvmx_pko_l5_sqx_sw_xoff_s cn78xx;
	struct cvmx_pko_l5_sqx_sw_xoff_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_sw_xoff cvmx_pko_l5_sqx_sw_xoff_t;

/**
 * cvmx_pko_l5_sq#_topology
 */
union cvmx_pko_l5_sqx_topology {
	u64 u64;
	struct cvmx_pko_l5_sqx_topology_s {
		u64 reserved_42_63 : 22;
		u64 prio_anchor : 10;
		u64 reserved_26_31 : 6;
		u64 parent : 10;
		u64 reserved_5_15 : 11;
		u64 rr_prio : 4;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_l5_sqx_topology_s cn78xx;
	struct cvmx_pko_l5_sqx_topology_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_topology cvmx_pko_l5_sqx_topology_t;

/**
 * cvmx_pko_l5_sq#_yellow
 *
 * This register has the same bit fields as PKO_L3_SQ()_YELLOW.
 *
 */
union cvmx_pko_l5_sqx_yellow {
	u64 u64;
	struct cvmx_pko_l5_sqx_yellow_s {
		u64 reserved_20_63 : 44;
		u64 head : 10;
		u64 tail : 10;
	} s;
	struct cvmx_pko_l5_sqx_yellow_s cn78xx;
	struct cvmx_pko_l5_sqx_yellow_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqx_yellow cvmx_pko_l5_sqx_yellow_t;

/**
 * cvmx_pko_l5_sq_csr_bus_debug
 */
union cvmx_pko_l5_sq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_l5_sq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_l5_sq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_l5_sq_csr_bus_debug_s cn78xxp1;
};

typedef union cvmx_pko_l5_sq_csr_bus_debug cvmx_pko_l5_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l5_sqa_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l5_sqa_debug {
	u64 u64;
	struct cvmx_pko_l5_sqa_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l5_sqa_debug_s cn78xx;
	struct cvmx_pko_l5_sqa_debug_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqa_debug cvmx_pko_l5_sqa_debug_t;

/**
 * cvmx_pko_l5_sqb_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_l5_sqb_debug {
	u64 u64;
	struct cvmx_pko_l5_sqb_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_l5_sqb_debug_s cn78xx;
	struct cvmx_pko_l5_sqb_debug_s cn78xxp1;
};

typedef union cvmx_pko_l5_sqb_debug cvmx_pko_l5_sqb_debug_t;

/**
 * cvmx_pko_lut#
 *
 * PKO_LUT has a location for each used PKI_CHAN_E. The following table
 * shows the mapping between LINK/MAC_NUM's, PKI_CHAN_E channels, and
 * PKO_LUT indices.
 *
 * <pre>
 *   LINK/   PKI_CHAN_E    Corresponding
 * MAC_NUM   Range         PKO_LUT index   Description
 * -------   -----------   -------------   -----------------
 *     0     0x000-0x03F   0x040-0x07F     LBK Loopback
 *     1     0x100-0x13F   0x080-0x0BF     DPI packet output
 *     2     0x800-0x80F   0x000-0x00F     BGX0 Logical MAC 0
 *     3     0x810-0x81F   0x010-0x01F     BGX0 Logical MAC 1
 *     4     0x820-0x82F   0x020-0x02F     BGX0 Logical MAC 2
 *     5     0x830-0x83F   0x030-0x03F     BGX0 Logical MAC 3
 * </pre>
 */
union cvmx_pko_lutx {
	u64 u64;
	struct cvmx_pko_lutx_s {
		u64 reserved_16_63 : 48;
		u64 valid : 1;
		u64 reserved_14_14 : 1;
		u64 pq_idx : 5;
		u64 queue_number : 9;
	} s;
	struct cvmx_pko_lutx_cn73xx {
		u64 reserved_16_63 : 48;
		u64 valid : 1;
		u64 reserved_13_14 : 2;
		u64 pq_idx : 4;
		u64 reserved_8_8 : 1;
		u64 queue_number : 8;
	} cn73xx;
	struct cvmx_pko_lutx_s cn78xx;
	struct cvmx_pko_lutx_s cn78xxp1;
	struct cvmx_pko_lutx_cn73xx cnf75xx;
};

typedef union cvmx_pko_lutx cvmx_pko_lutx_t;

/**
 * cvmx_pko_lut_bist_status
 */
union cvmx_pko_lut_bist_status {
	u64 u64;
	struct cvmx_pko_lut_bist_status_s {
		u64 reserved_1_63 : 63;
		u64 bist_status : 1;
	} s;
	struct cvmx_pko_lut_bist_status_s cn73xx;
	struct cvmx_pko_lut_bist_status_s cn78xx;
	struct cvmx_pko_lut_bist_status_s cn78xxp1;
	struct cvmx_pko_lut_bist_status_s cnf75xx;
};

typedef union cvmx_pko_lut_bist_status cvmx_pko_lut_bist_status_t;

/**
 * cvmx_pko_lut_ecc_ctl0
 */
union cvmx_pko_lut_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_lut_ecc_ctl0_s {
		u64 c2q_lut_ram_flip : 2;
		u64 c2q_lut_ram_cdis : 1;
		u64 reserved_0_60 : 61;
	} s;
	struct cvmx_pko_lut_ecc_ctl0_s cn73xx;
	struct cvmx_pko_lut_ecc_ctl0_s cn78xx;
	struct cvmx_pko_lut_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_lut_ecc_ctl0_s cnf75xx;
};

typedef union cvmx_pko_lut_ecc_ctl0 cvmx_pko_lut_ecc_ctl0_t;

/**
 * cvmx_pko_lut_ecc_dbe_sts0
 */
union cvmx_pko_lut_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_lut_ecc_dbe_sts0_s {
		u64 c2q_lut_ram_dbe : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_lut_ecc_dbe_sts0_s cn73xx;
	struct cvmx_pko_lut_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_lut_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_lut_ecc_dbe_sts0_s cnf75xx;
};

typedef union cvmx_pko_lut_ecc_dbe_sts0 cvmx_pko_lut_ecc_dbe_sts0_t;

/**
 * cvmx_pko_lut_ecc_dbe_sts_cmb0
 */
union cvmx_pko_lut_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s {
		u64 lut_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_lut_ecc_dbe_sts_cmb0 cvmx_pko_lut_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_lut_ecc_sbe_sts0
 */
union cvmx_pko_lut_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_lut_ecc_sbe_sts0_s {
		u64 c2q_lut_ram_sbe : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_lut_ecc_sbe_sts0_s cn73xx;
	struct cvmx_pko_lut_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_lut_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_lut_ecc_sbe_sts0_s cnf75xx;
};

typedef union cvmx_pko_lut_ecc_sbe_sts0 cvmx_pko_lut_ecc_sbe_sts0_t;

/**
 * cvmx_pko_lut_ecc_sbe_sts_cmb0
 */
union cvmx_pko_lut_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s {
		u64 lut_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_lut_ecc_sbe_sts_cmb0 cvmx_pko_lut_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_mac#_cfg
 *
 * These registers create the links between the MACs and the TxFIFO used to store the data,
 * and hold the per-MAC configuration bits.  These registers must be disabled (FIFO_NUM set
 * to 31) prior to reconfiguration of any of the other bits.
 *
 * <pre>
 *   CSR Name       Associated MAC
 *   ------------   -------------------
 *   PKO_MAC0_CFG   LBK loopback
 *   PKO_MAC1_CFG   DPI packet output
 *   PKO_MAC2_CFG   BGX0  logical MAC 0
 *   PKO_MAC3_CFG   BGX0  logical MAC 1
 *   PKO_MAC4_CFG   BGX0  logical MAC 2
 *   PKO_MAC5_CFG   BGX0  logical MAC 3
 *   PKO_MAC6_CFG   SRIO0 logical MAC 0
 *   PKO_MAC7_CFG   SRIO0 logical MAC 1
 *   PKO_MAC8_CFG   SRIO1 logical MAC 0
 *   PKO_MAC9_CFG   SRIO1 logical MAC 1
 * </pre>
 */
union cvmx_pko_macx_cfg {
	u64 u64;
	struct cvmx_pko_macx_cfg_s {
		u64 reserved_17_63 : 47;
		u64 min_pad_ena : 1;
		u64 fcs_ena : 1;
		u64 fcs_sop_off : 8;
		u64 skid_max_cnt : 2;
		u64 fifo_num : 5;
	} s;
	struct cvmx_pko_macx_cfg_s cn73xx;
	struct cvmx_pko_macx_cfg_s cn78xx;
	struct cvmx_pko_macx_cfg_s cn78xxp1;
	struct cvmx_pko_macx_cfg_s cnf75xx;
};

typedef union cvmx_pko_macx_cfg cvmx_pko_macx_cfg_t;

/**
 * cvmx_pko_mci0_cred_cnt#
 */
union cvmx_pko_mci0_cred_cntx {
	u64 u64;
	struct cvmx_pko_mci0_cred_cntx_s {
		u64 reserved_13_63 : 51;
		u64 cred_cnt : 13;
	} s;
	struct cvmx_pko_mci0_cred_cntx_s cn78xx;
	struct cvmx_pko_mci0_cred_cntx_s cn78xxp1;
};

typedef union cvmx_pko_mci0_cred_cntx cvmx_pko_mci0_cred_cntx_t;

/**
 * cvmx_pko_mci0_max_cred#
 */
union cvmx_pko_mci0_max_credx {
	u64 u64;
	struct cvmx_pko_mci0_max_credx_s {
		u64 reserved_12_63 : 52;
		u64 max_cred_lim : 12;
	} s;
	struct cvmx_pko_mci0_max_credx_s cn78xx;
	struct cvmx_pko_mci0_max_credx_s cn78xxp1;
};

typedef union cvmx_pko_mci0_max_credx cvmx_pko_mci0_max_credx_t;

/**
 * cvmx_pko_mci1_cred_cnt#
 */
union cvmx_pko_mci1_cred_cntx {
	u64 u64;
	struct cvmx_pko_mci1_cred_cntx_s {
		u64 reserved_13_63 : 51;
		u64 cred_cnt : 13;
	} s;
	struct cvmx_pko_mci1_cred_cntx_s cn73xx;
	struct cvmx_pko_mci1_cred_cntx_s cn78xx;
	struct cvmx_pko_mci1_cred_cntx_s cn78xxp1;
	struct cvmx_pko_mci1_cred_cntx_s cnf75xx;
};

typedef union cvmx_pko_mci1_cred_cntx cvmx_pko_mci1_cred_cntx_t;

/**
 * cvmx_pko_mci1_max_cred#
 */
union cvmx_pko_mci1_max_credx {
	u64 u64;
	struct cvmx_pko_mci1_max_credx_s {
		u64 reserved_12_63 : 52;
		u64 max_cred_lim : 12;
	} s;
	struct cvmx_pko_mci1_max_credx_s cn73xx;
	struct cvmx_pko_mci1_max_credx_s cn78xx;
	struct cvmx_pko_mci1_max_credx_s cn78xxp1;
	struct cvmx_pko_mci1_max_credx_s cnf75xx;
};

typedef union cvmx_pko_mci1_max_credx cvmx_pko_mci1_max_credx_t;

/**
 * cvmx_pko_mem_count0
 *
 * Notes:
 * Total number of packets seen by PKO, per port
 * A write to this address will clear the entry whose index is specified as COUNT[5:0].
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_count0 {
	u64 u64;
	struct cvmx_pko_mem_count0_s {
		u64 reserved_32_63 : 32;
		u64 count : 32;
	} s;
	struct cvmx_pko_mem_count0_s cn30xx;
	struct cvmx_pko_mem_count0_s cn31xx;
	struct cvmx_pko_mem_count0_s cn38xx;
	struct cvmx_pko_mem_count0_s cn38xxp2;
	struct cvmx_pko_mem_count0_s cn50xx;
	struct cvmx_pko_mem_count0_s cn52xx;
	struct cvmx_pko_mem_count0_s cn52xxp1;
	struct cvmx_pko_mem_count0_s cn56xx;
	struct cvmx_pko_mem_count0_s cn56xxp1;
	struct cvmx_pko_mem_count0_s cn58xx;
	struct cvmx_pko_mem_count0_s cn58xxp1;
	struct cvmx_pko_mem_count0_s cn61xx;
	struct cvmx_pko_mem_count0_s cn63xx;
	struct cvmx_pko_mem_count0_s cn63xxp1;
	struct cvmx_pko_mem_count0_s cn66xx;
	struct cvmx_pko_mem_count0_s cn68xx;
	struct cvmx_pko_mem_count0_s cn68xxp1;
	struct cvmx_pko_mem_count0_s cn70xx;
	struct cvmx_pko_mem_count0_s cn70xxp1;
	struct cvmx_pko_mem_count0_s cnf71xx;
};

typedef union cvmx_pko_mem_count0 cvmx_pko_mem_count0_t;

/**
 * cvmx_pko_mem_count1
 *
 * Notes:
 * Total number of bytes seen by PKO, per port
 * A write to this address will clear the entry whose index is specified as COUNT[5:0].
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_count1 {
	u64 u64;
	struct cvmx_pko_mem_count1_s {
		u64 reserved_48_63 : 16;
		u64 count : 48;
	} s;
	struct cvmx_pko_mem_count1_s cn30xx;
	struct cvmx_pko_mem_count1_s cn31xx;
	struct cvmx_pko_mem_count1_s cn38xx;
	struct cvmx_pko_mem_count1_s cn38xxp2;
	struct cvmx_pko_mem_count1_s cn50xx;
	struct cvmx_pko_mem_count1_s cn52xx;
	struct cvmx_pko_mem_count1_s cn52xxp1;
	struct cvmx_pko_mem_count1_s cn56xx;
	struct cvmx_pko_mem_count1_s cn56xxp1;
	struct cvmx_pko_mem_count1_s cn58xx;
	struct cvmx_pko_mem_count1_s cn58xxp1;
	struct cvmx_pko_mem_count1_s cn61xx;
	struct cvmx_pko_mem_count1_s cn63xx;
	struct cvmx_pko_mem_count1_s cn63xxp1;
	struct cvmx_pko_mem_count1_s cn66xx;
	struct cvmx_pko_mem_count1_s cn68xx;
	struct cvmx_pko_mem_count1_s cn68xxp1;
	struct cvmx_pko_mem_count1_s cn70xx;
	struct cvmx_pko_mem_count1_s cn70xxp1;
	struct cvmx_pko_mem_count1_s cnf71xx;
};

typedef union cvmx_pko_mem_count1 cvmx_pko_mem_count1_t;

/**
 * cvmx_pko_mem_debug0
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.cmnd[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug0 {
	u64 u64;
	struct cvmx_pko_mem_debug0_s {
		u64 fau : 28;
		u64 cmd : 14;
		u64 segs : 6;
		u64 size : 16;
	} s;
	struct cvmx_pko_mem_debug0_s cn30xx;
	struct cvmx_pko_mem_debug0_s cn31xx;
	struct cvmx_pko_mem_debug0_s cn38xx;
	struct cvmx_pko_mem_debug0_s cn38xxp2;
	struct cvmx_pko_mem_debug0_s cn50xx;
	struct cvmx_pko_mem_debug0_s cn52xx;
	struct cvmx_pko_mem_debug0_s cn52xxp1;
	struct cvmx_pko_mem_debug0_s cn56xx;
	struct cvmx_pko_mem_debug0_s cn56xxp1;
	struct cvmx_pko_mem_debug0_s cn58xx;
	struct cvmx_pko_mem_debug0_s cn58xxp1;
	struct cvmx_pko_mem_debug0_s cn61xx;
	struct cvmx_pko_mem_debug0_s cn63xx;
	struct cvmx_pko_mem_debug0_s cn63xxp1;
	struct cvmx_pko_mem_debug0_s cn66xx;
	struct cvmx_pko_mem_debug0_s cn68xx;
	struct cvmx_pko_mem_debug0_s cn68xxp1;
	struct cvmx_pko_mem_debug0_s cn70xx;
	struct cvmx_pko_mem_debug0_s cn70xxp1;
	struct cvmx_pko_mem_debug0_s cnf71xx;
};

typedef union cvmx_pko_mem_debug0 cvmx_pko_mem_debug0_t;

/**
 * cvmx_pko_mem_debug1
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.curr[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug1 {
	u64 u64;
	struct cvmx_pko_mem_debug1_s {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 ptr : 40;
	} s;
	struct cvmx_pko_mem_debug1_s cn30xx;
	struct cvmx_pko_mem_debug1_s cn31xx;
	struct cvmx_pko_mem_debug1_s cn38xx;
	struct cvmx_pko_mem_debug1_s cn38xxp2;
	struct cvmx_pko_mem_debug1_s cn50xx;
	struct cvmx_pko_mem_debug1_s cn52xx;
	struct cvmx_pko_mem_debug1_s cn52xxp1;
	struct cvmx_pko_mem_debug1_s cn56xx;
	struct cvmx_pko_mem_debug1_s cn56xxp1;
	struct cvmx_pko_mem_debug1_s cn58xx;
	struct cvmx_pko_mem_debug1_s cn58xxp1;
	struct cvmx_pko_mem_debug1_s cn61xx;
	struct cvmx_pko_mem_debug1_s cn63xx;
	struct cvmx_pko_mem_debug1_s cn63xxp1;
	struct cvmx_pko_mem_debug1_s cn66xx;
	struct cvmx_pko_mem_debug1_s cn68xx;
	struct cvmx_pko_mem_debug1_s cn68xxp1;
	struct cvmx_pko_mem_debug1_s cn70xx;
	struct cvmx_pko_mem_debug1_s cn70xxp1;
	struct cvmx_pko_mem_debug1_s cnf71xx;
};

typedef union cvmx_pko_mem_debug1 cvmx_pko_mem_debug1_t;

/**
 * cvmx_pko_mem_debug10
 *
 * Notes:
 * Internal per-engine state intended for debug use only - pko.dat.ptr.ptrs1, pko.dat.ptr.ptrs2
 * This CSR is a memory of 10 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug10 {
	u64 u64;
	struct cvmx_pko_mem_debug10_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug10_cn30xx {
		u64 fau : 28;
		u64 cmd : 14;
		u64 segs : 6;
		u64 size : 16;
	} cn30xx;
	struct cvmx_pko_mem_debug10_cn30xx cn31xx;
	struct cvmx_pko_mem_debug10_cn30xx cn38xx;
	struct cvmx_pko_mem_debug10_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug10_cn50xx {
		u64 reserved_49_63 : 15;
		u64 ptrs1 : 17;
		u64 reserved_17_31 : 15;
		u64 ptrs2 : 17;
	} cn50xx;
	struct cvmx_pko_mem_debug10_cn50xx cn52xx;
	struct cvmx_pko_mem_debug10_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug10_cn50xx cn56xx;
	struct cvmx_pko_mem_debug10_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug10_cn50xx cn58xx;
	struct cvmx_pko_mem_debug10_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug10_cn50xx cn61xx;
	struct cvmx_pko_mem_debug10_cn50xx cn63xx;
	struct cvmx_pko_mem_debug10_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug10_cn50xx cn66xx;
	struct cvmx_pko_mem_debug10_cn50xx cn68xx;
	struct cvmx_pko_mem_debug10_cn50xx cn68xxp1;
	struct cvmx_pko_mem_debug10_cn50xx cn70xx;
	struct cvmx_pko_mem_debug10_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug10_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug10 cvmx_pko_mem_debug10_t;

/**
 * cvmx_pko_mem_debug11
 *
 * Notes:
 * Internal per-engine state intended for debug use only - pko.out.sta.state[22:0]
 * This CSR is a memory of 10 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug11 {
	u64 u64;
	struct cvmx_pko_mem_debug11_s {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 reserved_0_39 : 40;
	} s;
	struct cvmx_pko_mem_debug11_cn30xx {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 ptr : 40;
	} cn30xx;
	struct cvmx_pko_mem_debug11_cn30xx cn31xx;
	struct cvmx_pko_mem_debug11_cn30xx cn38xx;
	struct cvmx_pko_mem_debug11_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug11_cn50xx {
		u64 reserved_23_63 : 41;
		u64 maj : 1;
		u64 uid : 3;
		u64 sop : 1;
		u64 len : 1;
		u64 chk : 1;
		u64 cnt : 13;
		u64 mod : 3;
	} cn50xx;
	struct cvmx_pko_mem_debug11_cn50xx cn52xx;
	struct cvmx_pko_mem_debug11_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug11_cn50xx cn56xx;
	struct cvmx_pko_mem_debug11_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug11_cn50xx cn58xx;
	struct cvmx_pko_mem_debug11_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug11_cn50xx cn61xx;
	struct cvmx_pko_mem_debug11_cn50xx cn63xx;
	struct cvmx_pko_mem_debug11_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug11_cn50xx cn66xx;
	struct cvmx_pko_mem_debug11_cn50xx cn68xx;
	struct cvmx_pko_mem_debug11_cn50xx cn68xxp1;
	struct cvmx_pko_mem_debug11_cn50xx cn70xx;
	struct cvmx_pko_mem_debug11_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug11_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug11 cvmx_pko_mem_debug11_t;

/**
 * cvmx_pko_mem_debug12
 *
 * Notes:
 * Internal per-engine x4 state intended for debug use only - pko.out.ctl.cmnd[63:0]
 * This CSR is a memory of 40 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug12 {
	u64 u64;
	struct cvmx_pko_mem_debug12_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug12_cn30xx {
		u64 data : 64;
	} cn30xx;
	struct cvmx_pko_mem_debug12_cn30xx cn31xx;
	struct cvmx_pko_mem_debug12_cn30xx cn38xx;
	struct cvmx_pko_mem_debug12_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug12_cn50xx {
		u64 fau : 28;
		u64 cmd : 14;
		u64 segs : 6;
		u64 size : 16;
	} cn50xx;
	struct cvmx_pko_mem_debug12_cn50xx cn52xx;
	struct cvmx_pko_mem_debug12_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug12_cn50xx cn56xx;
	struct cvmx_pko_mem_debug12_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug12_cn50xx cn58xx;
	struct cvmx_pko_mem_debug12_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug12_cn50xx cn61xx;
	struct cvmx_pko_mem_debug12_cn50xx cn63xx;
	struct cvmx_pko_mem_debug12_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug12_cn50xx cn66xx;
	struct cvmx_pko_mem_debug12_cn68xx {
		u64 state : 64;
	} cn68xx;
	struct cvmx_pko_mem_debug12_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug12_cn50xx cn70xx;
	struct cvmx_pko_mem_debug12_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug12_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug12 cvmx_pko_mem_debug12_t;

/**
 * cvmx_pko_mem_debug13
 *
 * Notes:
 * Internal per-engine x4 state intended for debug use only - pko.out.ctl.head[63:0]
 * This CSR is a memory of 40 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug13 {
	u64 u64;
	struct cvmx_pko_mem_debug13_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug13_cn30xx {
		u64 reserved_51_63 : 13;
		u64 widx : 17;
		u64 ridx2 : 17;
		u64 widx2 : 17;
	} cn30xx;
	struct cvmx_pko_mem_debug13_cn30xx cn31xx;
	struct cvmx_pko_mem_debug13_cn30xx cn38xx;
	struct cvmx_pko_mem_debug13_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug13_cn50xx {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 ptr : 40;
	} cn50xx;
	struct cvmx_pko_mem_debug13_cn50xx cn52xx;
	struct cvmx_pko_mem_debug13_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug13_cn50xx cn56xx;
	struct cvmx_pko_mem_debug13_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug13_cn50xx cn58xx;
	struct cvmx_pko_mem_debug13_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug13_cn50xx cn61xx;
	struct cvmx_pko_mem_debug13_cn50xx cn63xx;
	struct cvmx_pko_mem_debug13_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug13_cn50xx cn66xx;
	struct cvmx_pko_mem_debug13_cn68xx {
		u64 state : 64;
	} cn68xx;
	struct cvmx_pko_mem_debug13_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug13_cn50xx cn70xx;
	struct cvmx_pko_mem_debug13_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug13_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug13 cvmx_pko_mem_debug13_t;

/**
 * cvmx_pko_mem_debug14
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko.prt.psb.save[63:0]
 * This CSR is a memory of 132 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug14 {
	u64 u64;
	struct cvmx_pko_mem_debug14_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug14_cn30xx {
		u64 reserved_17_63 : 47;
		u64 ridx : 17;
	} cn30xx;
	struct cvmx_pko_mem_debug14_cn30xx cn31xx;
	struct cvmx_pko_mem_debug14_cn30xx cn38xx;
	struct cvmx_pko_mem_debug14_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug14_cn52xx {
		u64 data : 64;
	} cn52xx;
	struct cvmx_pko_mem_debug14_cn52xx cn52xxp1;
	struct cvmx_pko_mem_debug14_cn52xx cn56xx;
	struct cvmx_pko_mem_debug14_cn52xx cn56xxp1;
	struct cvmx_pko_mem_debug14_cn52xx cn61xx;
	struct cvmx_pko_mem_debug14_cn52xx cn63xx;
	struct cvmx_pko_mem_debug14_cn52xx cn63xxp1;
	struct cvmx_pko_mem_debug14_cn52xx cn66xx;
	struct cvmx_pko_mem_debug14_cn52xx cn70xx;
	struct cvmx_pko_mem_debug14_cn52xx cn70xxp1;
	struct cvmx_pko_mem_debug14_cn52xx cnf71xx;
};

typedef union cvmx_pko_mem_debug14 cvmx_pko_mem_debug14_t;

/**
 * cvmx_pko_mem_debug2
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.head[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug2 {
	u64 u64;
	struct cvmx_pko_mem_debug2_s {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 ptr : 40;
	} s;
	struct cvmx_pko_mem_debug2_s cn30xx;
	struct cvmx_pko_mem_debug2_s cn31xx;
	struct cvmx_pko_mem_debug2_s cn38xx;
	struct cvmx_pko_mem_debug2_s cn38xxp2;
	struct cvmx_pko_mem_debug2_s cn50xx;
	struct cvmx_pko_mem_debug2_s cn52xx;
	struct cvmx_pko_mem_debug2_s cn52xxp1;
	struct cvmx_pko_mem_debug2_s cn56xx;
	struct cvmx_pko_mem_debug2_s cn56xxp1;
	struct cvmx_pko_mem_debug2_s cn58xx;
	struct cvmx_pko_mem_debug2_s cn58xxp1;
	struct cvmx_pko_mem_debug2_s cn61xx;
	struct cvmx_pko_mem_debug2_s cn63xx;
	struct cvmx_pko_mem_debug2_s cn63xxp1;
	struct cvmx_pko_mem_debug2_s cn66xx;
	struct cvmx_pko_mem_debug2_s cn68xx;
	struct cvmx_pko_mem_debug2_s cn68xxp1;
	struct cvmx_pko_mem_debug2_s cn70xx;
	struct cvmx_pko_mem_debug2_s cn70xxp1;
	struct cvmx_pko_mem_debug2_s cnf71xx;
};

typedef union cvmx_pko_mem_debug2 cvmx_pko_mem_debug2_t;

/**
 * cvmx_pko_mem_debug3
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.resp[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug3 {
	u64 u64;
	struct cvmx_pko_mem_debug3_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug3_cn30xx {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 ptr : 40;
	} cn30xx;
	struct cvmx_pko_mem_debug3_cn30xx cn31xx;
	struct cvmx_pko_mem_debug3_cn30xx cn38xx;
	struct cvmx_pko_mem_debug3_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug3_cn50xx {
		u64 data : 64;
	} cn50xx;
	struct cvmx_pko_mem_debug3_cn50xx cn52xx;
	struct cvmx_pko_mem_debug3_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug3_cn50xx cn56xx;
	struct cvmx_pko_mem_debug3_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug3_cn50xx cn58xx;
	struct cvmx_pko_mem_debug3_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug3_cn50xx cn61xx;
	struct cvmx_pko_mem_debug3_cn50xx cn63xx;
	struct cvmx_pko_mem_debug3_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug3_cn50xx cn66xx;
	struct cvmx_pko_mem_debug3_cn50xx cn68xx;
	struct cvmx_pko_mem_debug3_cn50xx cn68xxp1;
	struct cvmx_pko_mem_debug3_cn50xx cn70xx;
	struct cvmx_pko_mem_debug3_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug3_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug3 cvmx_pko_mem_debug3_t;

/**
 * cvmx_pko_mem_debug4
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.state[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug4 {
	u64 u64;
	struct cvmx_pko_mem_debug4_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug4_cn30xx {
		u64 data : 64;
	} cn30xx;
	struct cvmx_pko_mem_debug4_cn30xx cn31xx;
	struct cvmx_pko_mem_debug4_cn30xx cn38xx;
	struct cvmx_pko_mem_debug4_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug4_cn50xx {
		u64 cmnd_segs : 3;
		u64 cmnd_siz : 16;
		u64 cmnd_off : 6;
		u64 uid : 3;
		u64 dread_sop : 1;
		u64 init_dwrite : 1;
		u64 chk_once : 1;
		u64 chk_mode : 1;
		u64 active : 1;
		u64 static_p : 1;
		u64 qos : 3;
		u64 qcb_ridx : 5;
		u64 qid_off_max : 4;
		u64 qid_off : 4;
		u64 qid_base : 8;
		u64 wait : 1;
		u64 minor : 2;
		u64 major : 3;
	} cn50xx;
	struct cvmx_pko_mem_debug4_cn52xx {
		u64 curr_siz : 8;
		u64 curr_off : 16;
		u64 cmnd_segs : 6;
		u64 cmnd_siz : 16;
		u64 cmnd_off : 6;
		u64 uid : 2;
		u64 dread_sop : 1;
		u64 init_dwrite : 1;
		u64 chk_once : 1;
		u64 chk_mode : 1;
		u64 wait : 1;
		u64 minor : 2;
		u64 major : 3;
	} cn52xx;
	struct cvmx_pko_mem_debug4_cn52xx cn52xxp1;
	struct cvmx_pko_mem_debug4_cn52xx cn56xx;
	struct cvmx_pko_mem_debug4_cn52xx cn56xxp1;
	struct cvmx_pko_mem_debug4_cn50xx cn58xx;
	struct cvmx_pko_mem_debug4_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug4_cn52xx cn61xx;
	struct cvmx_pko_mem_debug4_cn52xx cn63xx;
	struct cvmx_pko_mem_debug4_cn52xx cn63xxp1;
	struct cvmx_pko_mem_debug4_cn52xx cn66xx;
	struct cvmx_pko_mem_debug4_cn68xx {
		u64 curr_siz : 9;
		u64 curr_off : 16;
		u64 cmnd_segs : 6;
		u64 cmnd_siz : 16;
		u64 cmnd_off : 6;
		u64 dread_sop : 1;
		u64 init_dwrite : 1;
		u64 chk_once : 1;
		u64 chk_mode : 1;
		u64 reserved_6_6 : 1;
		u64 minor : 2;
		u64 major : 4;
	} cn68xx;
	struct cvmx_pko_mem_debug4_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug4_cn52xx cn70xx;
	struct cvmx_pko_mem_debug4_cn52xx cn70xxp1;
	struct cvmx_pko_mem_debug4_cn52xx cnf71xx;
};

typedef union cvmx_pko_mem_debug4 cvmx_pko_mem_debug4_t;

/**
 * cvmx_pko_mem_debug5
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.state[127:64]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug5 {
	u64 u64;
	struct cvmx_pko_mem_debug5_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug5_cn30xx {
		u64 dwri_mod : 1;
		u64 dwri_sop : 1;
		u64 dwri_len : 1;
		u64 dwri_cnt : 13;
		u64 cmnd_siz : 16;
		u64 uid : 1;
		u64 xfer_wor : 1;
		u64 xfer_dwr : 1;
		u64 cbuf_fre : 1;
		u64 reserved_27_27 : 1;
		u64 chk_mode : 1;
		u64 active : 1;
		u64 qos : 3;
		u64 qcb_ridx : 5;
		u64 qid_off : 3;
		u64 qid_base : 7;
		u64 wait : 1;
		u64 minor : 2;
		u64 major : 4;
	} cn30xx;
	struct cvmx_pko_mem_debug5_cn30xx cn31xx;
	struct cvmx_pko_mem_debug5_cn30xx cn38xx;
	struct cvmx_pko_mem_debug5_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug5_cn50xx {
		u64 curr_ptr : 29;
		u64 curr_siz : 16;
		u64 curr_off : 16;
		u64 cmnd_segs : 3;
	} cn50xx;
	struct cvmx_pko_mem_debug5_cn52xx {
		u64 reserved_54_63 : 10;
		u64 nxt_inflt : 6;
		u64 curr_ptr : 40;
		u64 curr_siz : 8;
	} cn52xx;
	struct cvmx_pko_mem_debug5_cn52xx cn52xxp1;
	struct cvmx_pko_mem_debug5_cn52xx cn56xx;
	struct cvmx_pko_mem_debug5_cn52xx cn56xxp1;
	struct cvmx_pko_mem_debug5_cn50xx cn58xx;
	struct cvmx_pko_mem_debug5_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug5_cn61xx {
		u64 reserved_56_63 : 8;
		u64 ptp : 1;
		u64 major_3 : 1;
		u64 nxt_inflt : 6;
		u64 curr_ptr : 40;
		u64 curr_siz : 8;
	} cn61xx;
	struct cvmx_pko_mem_debug5_cn61xx cn63xx;
	struct cvmx_pko_mem_debug5_cn61xx cn63xxp1;
	struct cvmx_pko_mem_debug5_cn61xx cn66xx;
	struct cvmx_pko_mem_debug5_cn68xx {
		u64 reserved_57_63 : 7;
		u64 uid : 3;
		u64 ptp : 1;
		u64 nxt_inflt : 6;
		u64 curr_ptr : 40;
		u64 curr_siz : 7;
	} cn68xx;
	struct cvmx_pko_mem_debug5_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug5_cn61xx cn70xx;
	struct cvmx_pko_mem_debug5_cn61xx cn70xxp1;
	struct cvmx_pko_mem_debug5_cn61xx cnf71xx;
};

typedef union cvmx_pko_mem_debug5 cvmx_pko_mem_debug5_t;

/**
 * cvmx_pko_mem_debug6
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.port[63:0]
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug6 {
	u64 u64;
	struct cvmx_pko_mem_debug6_s {
		u64 reserved_38_63 : 26;
		u64 qos_active : 1;
		u64 reserved_0_36 : 37;
	} s;
	struct cvmx_pko_mem_debug6_cn30xx {
		u64 reserved_11_63 : 53;
		u64 qid_offm : 3;
		u64 static_p : 1;
		u64 work_min : 3;
		u64 dwri_chk : 1;
		u64 dwri_uid : 1;
		u64 dwri_mod : 2;
	} cn30xx;
	struct cvmx_pko_mem_debug6_cn30xx cn31xx;
	struct cvmx_pko_mem_debug6_cn30xx cn38xx;
	struct cvmx_pko_mem_debug6_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug6_cn50xx {
		u64 reserved_11_63 : 53;
		u64 curr_ptr : 11;
	} cn50xx;
	struct cvmx_pko_mem_debug6_cn52xx {
		u64 reserved_37_63 : 27;
		u64 qid_offres : 4;
		u64 qid_offths : 4;
		u64 preempter : 1;
		u64 preemptee : 1;
		u64 preempted : 1;
		u64 active : 1;
		u64 statc : 1;
		u64 qos : 3;
		u64 qcb_ridx : 5;
		u64 qid_offmax : 4;
		u64 qid_off : 4;
		u64 qid_base : 8;
	} cn52xx;
	struct cvmx_pko_mem_debug6_cn52xx cn52xxp1;
	struct cvmx_pko_mem_debug6_cn52xx cn56xx;
	struct cvmx_pko_mem_debug6_cn52xx cn56xxp1;
	struct cvmx_pko_mem_debug6_cn50xx cn58xx;
	struct cvmx_pko_mem_debug6_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug6_cn52xx cn61xx;
	struct cvmx_pko_mem_debug6_cn52xx cn63xx;
	struct cvmx_pko_mem_debug6_cn52xx cn63xxp1;
	struct cvmx_pko_mem_debug6_cn52xx cn66xx;
	struct cvmx_pko_mem_debug6_cn68xx {
		u64 reserved_38_63 : 26;
		u64 qos_active : 1;
		u64 qid_offths : 5;
		u64 preempter : 1;
		u64 preemptee : 1;
		u64 active : 1;
		u64 static_p : 1;
		u64 qos : 3;
		u64 qcb_ridx : 7;
		u64 qid_offmax : 5;
		u64 qid_off : 5;
		u64 qid_base : 8;
	} cn68xx;
	struct cvmx_pko_mem_debug6_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug6_cn70xx {
		u64 reserved_63_37 : 27;
		u64 qid_offres : 4;
		u64 qid_offths : 4;
		u64 preempter : 1;
		u64 preemptee : 1;
		u64 preempted : 1;
		u64 active : 1;
		u64 staticb : 1;
		u64 qos : 3;
		u64 qcb_ridx : 5;
		u64 qid_offmax : 4;
		u64 qid_off : 4;
		u64 qid_base : 8;
	} cn70xx;
	struct cvmx_pko_mem_debug6_cn70xx cn70xxp1;
	struct cvmx_pko_mem_debug6_cn52xx cnf71xx;
};

typedef union cvmx_pko_mem_debug6 cvmx_pko_mem_debug6_t;

/**
 * cvmx_pko_mem_debug7
 *
 * Notes:
 * Internal per-queue state intended for debug use only - pko_prt_qsb.state[63:0]
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug7 {
	u64 u64;
	struct cvmx_pko_mem_debug7_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_mem_debug7_cn30xx {
		u64 reserved_58_63 : 6;
		u64 dwb : 9;
		u64 start : 33;
		u64 size : 16;
	} cn30xx;
	struct cvmx_pko_mem_debug7_cn30xx cn31xx;
	struct cvmx_pko_mem_debug7_cn30xx cn38xx;
	struct cvmx_pko_mem_debug7_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug7_cn50xx {
		u64 qos : 5;
		u64 tail : 1;
		u64 buf_siz : 13;
		u64 buf_ptr : 33;
		u64 qcb_widx : 6;
		u64 qcb_ridx : 6;
	} cn50xx;
	struct cvmx_pko_mem_debug7_cn50xx cn52xx;
	struct cvmx_pko_mem_debug7_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug7_cn50xx cn56xx;
	struct cvmx_pko_mem_debug7_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug7_cn50xx cn58xx;
	struct cvmx_pko_mem_debug7_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug7_cn50xx cn61xx;
	struct cvmx_pko_mem_debug7_cn50xx cn63xx;
	struct cvmx_pko_mem_debug7_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug7_cn50xx cn66xx;
	struct cvmx_pko_mem_debug7_cn68xx {
		u64 buf_siz : 11;
		u64 buf_ptr : 37;
		u64 qcb_widx : 8;
		u64 qcb_ridx : 8;
	} cn68xx;
	struct cvmx_pko_mem_debug7_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug7_cn50xx cn70xx;
	struct cvmx_pko_mem_debug7_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug7_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug7 cvmx_pko_mem_debug7_t;

/**
 * cvmx_pko_mem_debug8
 *
 * Notes:
 * Internal per-queue state intended for debug use only - pko_prt_qsb.state[91:64]
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug8 {
	u64 u64;
	struct cvmx_pko_mem_debug8_s {
		u64 reserved_59_63 : 5;
		u64 tail : 1;
		u64 reserved_0_57 : 58;
	} s;
	struct cvmx_pko_mem_debug8_cn30xx {
		u64 qos : 5;
		u64 tail : 1;
		u64 buf_siz : 13;
		u64 buf_ptr : 33;
		u64 qcb_widx : 6;
		u64 qcb_ridx : 6;
	} cn30xx;
	struct cvmx_pko_mem_debug8_cn30xx cn31xx;
	struct cvmx_pko_mem_debug8_cn30xx cn38xx;
	struct cvmx_pko_mem_debug8_cn30xx cn38xxp2;
	struct cvmx_pko_mem_debug8_cn50xx {
		u64 reserved_28_63 : 36;
		u64 doorbell : 20;
		u64 reserved_6_7 : 2;
		u64 static_p : 1;
		u64 s_tail : 1;
		u64 static_q : 1;
		u64 qos : 3;
	} cn50xx;
	struct cvmx_pko_mem_debug8_cn52xx {
		u64 reserved_29_63 : 35;
		u64 preempter : 1;
		u64 doorbell : 20;
		u64 reserved_7_7 : 1;
		u64 preemptee : 1;
		u64 static_p : 1;
		u64 s_tail : 1;
		u64 static_q : 1;
		u64 qos : 3;
	} cn52xx;
	struct cvmx_pko_mem_debug8_cn52xx cn52xxp1;
	struct cvmx_pko_mem_debug8_cn52xx cn56xx;
	struct cvmx_pko_mem_debug8_cn52xx cn56xxp1;
	struct cvmx_pko_mem_debug8_cn50xx cn58xx;
	struct cvmx_pko_mem_debug8_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug8_cn61xx {
		u64 reserved_42_63 : 22;
		u64 qid_qqos : 8;
		u64 reserved_33_33 : 1;
		u64 qid_idx : 4;
		u64 preempter : 1;
		u64 doorbell : 20;
		u64 reserved_7_7 : 1;
		u64 preemptee : 1;
		u64 static_p : 1;
		u64 s_tail : 1;
		u64 static_q : 1;
		u64 qos : 3;
	} cn61xx;
	struct cvmx_pko_mem_debug8_cn52xx cn63xx;
	struct cvmx_pko_mem_debug8_cn52xx cn63xxp1;
	struct cvmx_pko_mem_debug8_cn61xx cn66xx;
	struct cvmx_pko_mem_debug8_cn68xx {
		u64 reserved_50_63 : 14;
		u64 qid_qqos : 8;
		u64 qid_idx : 5;
		u64 preempter : 1;
		u64 doorbell : 20;
		u64 reserved_9_15 : 7;
		u64 qid_qos : 6;
		u64 qid_tail : 1;
		u64 buf_siz : 2;
	} cn68xx;
	struct cvmx_pko_mem_debug8_cn68xx cn68xxp1;
	struct cvmx_pko_mem_debug8_cn61xx cn70xx;
	struct cvmx_pko_mem_debug8_cn61xx cn70xxp1;
	struct cvmx_pko_mem_debug8_cn61xx cnf71xx;
};

typedef union cvmx_pko_mem_debug8 cvmx_pko_mem_debug8_t;

/**
 * cvmx_pko_mem_debug9
 *
 * Notes:
 * Internal per-engine state intended for debug use only - pko.dat.ptr.ptrs0, pko.dat.ptr.ptrs3
 * This CSR is a memory of 10 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug9 {
	u64 u64;
	struct cvmx_pko_mem_debug9_s {
		u64 reserved_49_63 : 15;
		u64 ptrs0 : 17;
		u64 reserved_0_31 : 32;
	} s;
	struct cvmx_pko_mem_debug9_cn30xx {
		u64 reserved_28_63 : 36;
		u64 doorbell : 20;
		u64 reserved_5_7 : 3;
		u64 s_tail : 1;
		u64 static_q : 1;
		u64 qos : 3;
	} cn30xx;
	struct cvmx_pko_mem_debug9_cn30xx cn31xx;
	struct cvmx_pko_mem_debug9_cn38xx {
		u64 reserved_28_63 : 36;
		u64 doorbell : 20;
		u64 reserved_6_7 : 2;
		u64 static_p : 1;
		u64 s_tail : 1;
		u64 static_q : 1;
		u64 qos : 3;
	} cn38xx;
	struct cvmx_pko_mem_debug9_cn38xx cn38xxp2;
	struct cvmx_pko_mem_debug9_cn50xx {
		u64 reserved_49_63 : 15;
		u64 ptrs0 : 17;
		u64 reserved_17_31 : 15;
		u64 ptrs3 : 17;
	} cn50xx;
	struct cvmx_pko_mem_debug9_cn50xx cn52xx;
	struct cvmx_pko_mem_debug9_cn50xx cn52xxp1;
	struct cvmx_pko_mem_debug9_cn50xx cn56xx;
	struct cvmx_pko_mem_debug9_cn50xx cn56xxp1;
	struct cvmx_pko_mem_debug9_cn50xx cn58xx;
	struct cvmx_pko_mem_debug9_cn50xx cn58xxp1;
	struct cvmx_pko_mem_debug9_cn50xx cn61xx;
	struct cvmx_pko_mem_debug9_cn50xx cn63xx;
	struct cvmx_pko_mem_debug9_cn50xx cn63xxp1;
	struct cvmx_pko_mem_debug9_cn50xx cn66xx;
	struct cvmx_pko_mem_debug9_cn50xx cn68xx;
	struct cvmx_pko_mem_debug9_cn50xx cn68xxp1;
	struct cvmx_pko_mem_debug9_cn50xx cn70xx;
	struct cvmx_pko_mem_debug9_cn50xx cn70xxp1;
	struct cvmx_pko_mem_debug9_cn50xx cnf71xx;
};

typedef union cvmx_pko_mem_debug9 cvmx_pko_mem_debug9_t;

/**
 * cvmx_pko_mem_iport_ptrs
 *
 * Notes:
 * This CSR is a memory of 128 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IPORT.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iport_ptrs {
	u64 u64;
	struct cvmx_pko_mem_iport_ptrs_s {
		u64 reserved_63_63 : 1;
		u64 crc : 1;
		u64 static_p : 1;
		u64 qos_mask : 8;
		u64 min_pkt : 3;
		u64 reserved_31_49 : 19;
		u64 pipe : 7;
		u64 reserved_21_23 : 3;
		u64 intr : 5;
		u64 reserved_13_15 : 3;
		u64 eid : 5;
		u64 reserved_7_7 : 1;
		u64 ipid : 7;
	} s;
	struct cvmx_pko_mem_iport_ptrs_s cn68xx;
	struct cvmx_pko_mem_iport_ptrs_s cn68xxp1;
};

typedef union cvmx_pko_mem_iport_ptrs cvmx_pko_mem_iport_ptrs_t;

/**
 * cvmx_pko_mem_iport_qos
 *
 * Notes:
 * Sets the QOS mask, per port.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_IPORT_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other port state.  The engine to which port PID is mapped is engine
 * EID.  Note that the port to engine mapping must be the same as was previously programmed via the
 * PKO_MEM_IPORT_PTRS CSR.
 * This CSR is a memory of 128 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IPORT.  A read of
 * any entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iport_qos {
	u64 u64;
	struct cvmx_pko_mem_iport_qos_s {
		u64 reserved_61_63 : 3;
		u64 qos_mask : 8;
		u64 reserved_13_52 : 40;
		u64 eid : 5;
		u64 reserved_7_7 : 1;
		u64 ipid : 7;
	} s;
	struct cvmx_pko_mem_iport_qos_s cn68xx;
	struct cvmx_pko_mem_iport_qos_s cn68xxp1;
};

typedef union cvmx_pko_mem_iport_qos cvmx_pko_mem_iport_qos_t;

/**
 * cvmx_pko_mem_iqueue_ptrs
 *
 * Notes:
 * Sets the queue to port mapping and the initial command buffer pointer, per queue.  Unused queues must
 * set BUF_PTR=0.  Each queue may map to at most one port.  No more than 32 queues may map to a port.
 * The set of queues that is mapped to a port must be a contiguous array of queues.  The port to which
 * queue QID is mapped is port IPID.  The index of queue QID in port IPID's queue list is IDX.  The last
 * queue in port IPID's queue array must have its TAIL bit set.
 * STATIC_Q marks queue QID as having static priority.  STATIC_P marks the port IPID to which QID is
 * mapped as having at least one queue with static priority.  If any QID that maps to IPID has static
 * priority, then all QID that map to IPID must have STATIC_P set.  Queues marked as static priority
 * must be contiguous and begin at IDX 0.  The last queue that is marked as having static priority
 * must have its S_TAIL bit set.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IQUEUE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iqueue_ptrs {
	u64 u64;
	struct cvmx_pko_mem_iqueue_ptrs_s {
		u64 s_tail : 1;
		u64 static_p : 1;
		u64 static_q : 1;
		u64 qos_mask : 8;
		u64 buf_ptr : 31;
		u64 tail : 1;
		u64 index : 5;
		u64 reserved_15_15 : 1;
		u64 ipid : 7;
		u64 qid : 8;
	} s;
	struct cvmx_pko_mem_iqueue_ptrs_s cn68xx;
	struct cvmx_pko_mem_iqueue_ptrs_s cn68xxp1;
};

typedef union cvmx_pko_mem_iqueue_ptrs cvmx_pko_mem_iqueue_ptrs_t;

/**
 * cvmx_pko_mem_iqueue_qos
 *
 * Notes:
 * Sets the QOS mask, per queue.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_IQUEUE_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other queue state.  The port to which queue QID is mapped is port
 * IPID.  Note that the queue to port mapping must be the same as was previously programmed via the
 * PKO_MEM_IQUEUE_PTRS CSR.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IQUEUE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iqueue_qos {
	u64 u64;
	struct cvmx_pko_mem_iqueue_qos_s {
		u64 reserved_61_63 : 3;
		u64 qos_mask : 8;
		u64 reserved_15_52 : 38;
		u64 ipid : 7;
		u64 qid : 8;
	} s;
	struct cvmx_pko_mem_iqueue_qos_s cn68xx;
	struct cvmx_pko_mem_iqueue_qos_s cn68xxp1;
};

typedef union cvmx_pko_mem_iqueue_qos cvmx_pko_mem_iqueue_qos_t;

/**
 * cvmx_pko_mem_port_ptrs
 *
 * Notes:
 * Sets the port to engine mapping, per port.  Ports marked as static priority need not be contiguous,
 * but they must be the lowest numbered PIDs mapped to this EID and must have QOS_MASK=0xff.  If EID==8
 * or EID==9, then PID[1:0] is used to direct the packet to the correct port on that interface.
 * EID==15 can be used for unused PKO-internal ports.
 * The reset configuration is the following:
 *   PID EID(ext port) BP_PORT QOS_MASK STATIC_P
 *   -------------------------------------------
 *     0   0( 0)             0     0xff        0
 *     1   1( 1)             1     0xff        0
 *     2   2( 2)             2     0xff        0
 *     3   3( 3)             3     0xff        0
 *     4   0( 0)             4     0xff        0
 *     5   1( 1)             5     0xff        0
 *     6   2( 2)             6     0xff        0
 *     7   3( 3)             7     0xff        0
 *     8   0( 0)             8     0xff        0
 *     9   1( 1)             9     0xff        0
 *    10   2( 2)            10     0xff        0
 *    11   3( 3)            11     0xff        0
 *    12   0( 0)            12     0xff        0
 *    13   1( 1)            13     0xff        0
 *    14   2( 2)            14     0xff        0
 *    15   3( 3)            15     0xff        0
 *   -------------------------------------------
 *    16   4(16)            16     0xff        0
 *    17   5(17)            17     0xff        0
 *    18   6(18)            18     0xff        0
 *    19   7(19)            19     0xff        0
 *    20   4(16)            20     0xff        0
 *    21   5(17)            21     0xff        0
 *    22   6(18)            22     0xff        0
 *    23   7(19)            23     0xff        0
 *    24   4(16)            24     0xff        0
 *    25   5(17)            25     0xff        0
 *    26   6(18)            26     0xff        0
 *    27   7(19)            27     0xff        0
 *    28   4(16)            28     0xff        0
 *    29   5(17)            29     0xff        0
 *    30   6(18)            30     0xff        0
 *    31   7(19)            31     0xff        0
 *   -------------------------------------------
 *    32   8(32)            32     0xff        0
 *    33   8(33)            33     0xff        0
 *    34   8(34)            34     0xff        0
 *    35   8(35)            35     0xff        0
 *   -------------------------------------------
 *    36   9(36)            36     0xff        0
 *    37   9(37)            37     0xff        0
 *    38   9(38)            38     0xff        0
 *    39   9(39)            39     0xff        0
 *
 * This CSR is a memory of 48 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_ptrs {
	u64 u64;
	struct cvmx_pko_mem_port_ptrs_s {
		u64 reserved_62_63 : 2;
		u64 static_p : 1;
		u64 qos_mask : 8;
		u64 reserved_16_52 : 37;
		u64 bp_port : 6;
		u64 eid : 4;
		u64 pid : 6;
	} s;
	struct cvmx_pko_mem_port_ptrs_s cn52xx;
	struct cvmx_pko_mem_port_ptrs_s cn52xxp1;
	struct cvmx_pko_mem_port_ptrs_s cn56xx;
	struct cvmx_pko_mem_port_ptrs_s cn56xxp1;
	struct cvmx_pko_mem_port_ptrs_s cn61xx;
	struct cvmx_pko_mem_port_ptrs_s cn63xx;
	struct cvmx_pko_mem_port_ptrs_s cn63xxp1;
	struct cvmx_pko_mem_port_ptrs_s cn66xx;
	struct cvmx_pko_mem_port_ptrs_s cn70xx;
	struct cvmx_pko_mem_port_ptrs_s cn70xxp1;
	struct cvmx_pko_mem_port_ptrs_s cnf71xx;
};

typedef union cvmx_pko_mem_port_ptrs cvmx_pko_mem_port_ptrs_t;

/**
 * cvmx_pko_mem_port_qos
 *
 * Notes:
 * Sets the QOS mask, per port.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_PORT_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other port state.  The engine to which port PID is mapped is engine
 * EID.  Note that the port to engine mapping must be the same as was previously programmed via the
 * PKO_MEM_PORT_PTRS CSR.
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_qos {
	u64 u64;
	struct cvmx_pko_mem_port_qos_s {
		u64 reserved_61_63 : 3;
		u64 qos_mask : 8;
		u64 reserved_10_52 : 43;
		u64 eid : 4;
		u64 pid : 6;
	} s;
	struct cvmx_pko_mem_port_qos_s cn52xx;
	struct cvmx_pko_mem_port_qos_s cn52xxp1;
	struct cvmx_pko_mem_port_qos_s cn56xx;
	struct cvmx_pko_mem_port_qos_s cn56xxp1;
	struct cvmx_pko_mem_port_qos_s cn61xx;
	struct cvmx_pko_mem_port_qos_s cn63xx;
	struct cvmx_pko_mem_port_qos_s cn63xxp1;
	struct cvmx_pko_mem_port_qos_s cn66xx;
	struct cvmx_pko_mem_port_qos_s cn70xx;
	struct cvmx_pko_mem_port_qos_s cn70xxp1;
	struct cvmx_pko_mem_port_qos_s cnf71xx;
};

typedef union cvmx_pko_mem_port_qos cvmx_pko_mem_port_qos_t;

/**
 * cvmx_pko_mem_port_rate0
 *
 * Notes:
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_rate0 {
	u64 u64;
	struct cvmx_pko_mem_port_rate0_s {
		u64 reserved_51_63 : 13;
		u64 rate_word : 19;
		u64 rate_pkt : 24;
		u64 reserved_7_7 : 1;
		u64 pid : 7;
	} s;
	struct cvmx_pko_mem_port_rate0_cn52xx {
		u64 reserved_51_63 : 13;
		u64 rate_word : 19;
		u64 rate_pkt : 24;
		u64 reserved_6_7 : 2;
		u64 pid : 6;
	} cn52xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn52xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn56xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn56xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn61xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn63xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn63xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn66xx;
	struct cvmx_pko_mem_port_rate0_s cn68xx;
	struct cvmx_pko_mem_port_rate0_s cn68xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn70xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn70xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cnf71xx;
};

typedef union cvmx_pko_mem_port_rate0 cvmx_pko_mem_port_rate0_t;

/**
 * cvmx_pko_mem_port_rate1
 *
 * Notes:
 * Writing PKO_MEM_PORT_RATE1[PID,RATE_LIM] has the side effect of setting the corresponding
 * accumulator to zero.
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_rate1 {
	u64 u64;
	struct cvmx_pko_mem_port_rate1_s {
		u64 reserved_32_63 : 32;
		u64 rate_lim : 24;
		u64 reserved_7_7 : 1;
		u64 pid : 7;
	} s;
	struct cvmx_pko_mem_port_rate1_cn52xx {
		u64 reserved_32_63 : 32;
		u64 rate_lim : 24;
		u64 reserved_6_7 : 2;
		u64 pid : 6;
	} cn52xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn52xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn56xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn56xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn61xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn63xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn63xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn66xx;
	struct cvmx_pko_mem_port_rate1_s cn68xx;
	struct cvmx_pko_mem_port_rate1_s cn68xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn70xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn70xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cnf71xx;
};

typedef union cvmx_pko_mem_port_rate1 cvmx_pko_mem_port_rate1_t;

/**
 * cvmx_pko_mem_queue_ptrs
 *
 * Notes:
 * Sets the queue to port mapping and the initial command buffer pointer, per queue
 * Each queue may map to at most one port.  No more than 16 queues may map to a port.  The set of
 * queues that is mapped to a port must be a contiguous array of queues.  The port to which queue QID
 * is mapped is port PID.  The index of queue QID in port PID's queue list is IDX.  The last queue in
 * port PID's queue array must have its TAIL bit set.  Unused queues must be mapped to port 63.
 * STATIC_Q marks queue QID as having static priority.  STATIC_P marks the port PID to which QID is
 * mapped as having at least one queue with static priority.  If any QID that maps to PID has static
 * priority, then all QID that map to PID must have STATIC_P set.  Queues marked as static priority
 * must be contiguous and begin at IDX 0.  The last queue that is marked as having static priority
 * must have its S_TAIL bit set.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_queue_ptrs {
	u64 u64;
	struct cvmx_pko_mem_queue_ptrs_s {
		u64 s_tail : 1;
		u64 static_p : 1;
		u64 static_q : 1;
		u64 qos_mask : 8;
		u64 buf_ptr : 36;
		u64 tail : 1;
		u64 index : 3;
		u64 port : 6;
		u64 queue : 7;
	} s;
	struct cvmx_pko_mem_queue_ptrs_s cn30xx;
	struct cvmx_pko_mem_queue_ptrs_s cn31xx;
	struct cvmx_pko_mem_queue_ptrs_s cn38xx;
	struct cvmx_pko_mem_queue_ptrs_s cn38xxp2;
	struct cvmx_pko_mem_queue_ptrs_s cn50xx;
	struct cvmx_pko_mem_queue_ptrs_s cn52xx;
	struct cvmx_pko_mem_queue_ptrs_s cn52xxp1;
	struct cvmx_pko_mem_queue_ptrs_s cn56xx;
	struct cvmx_pko_mem_queue_ptrs_s cn56xxp1;
	struct cvmx_pko_mem_queue_ptrs_s cn58xx;
	struct cvmx_pko_mem_queue_ptrs_s cn58xxp1;
	struct cvmx_pko_mem_queue_ptrs_s cn61xx;
	struct cvmx_pko_mem_queue_ptrs_s cn63xx;
	struct cvmx_pko_mem_queue_ptrs_s cn63xxp1;
	struct cvmx_pko_mem_queue_ptrs_s cn66xx;
	struct cvmx_pko_mem_queue_ptrs_s cn70xx;
	struct cvmx_pko_mem_queue_ptrs_s cn70xxp1;
	struct cvmx_pko_mem_queue_ptrs_s cnf71xx;
};

typedef union cvmx_pko_mem_queue_ptrs cvmx_pko_mem_queue_ptrs_t;

/**
 * cvmx_pko_mem_queue_qos
 *
 * Notes:
 * Sets the QOS mask, per queue.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_QUEUE_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other queue state.  The port to which queue QID is mapped is port
 * PID.  Note that the queue to port mapping must be the same as was previously programmed via the
 * PKO_MEM_QUEUE_PTRS CSR.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_queue_qos {
	u64 u64;
	struct cvmx_pko_mem_queue_qos_s {
		u64 reserved_61_63 : 3;
		u64 qos_mask : 8;
		u64 reserved_13_52 : 40;
		u64 pid : 6;
		u64 qid : 7;
	} s;
	struct cvmx_pko_mem_queue_qos_s cn30xx;
	struct cvmx_pko_mem_queue_qos_s cn31xx;
	struct cvmx_pko_mem_queue_qos_s cn38xx;
	struct cvmx_pko_mem_queue_qos_s cn38xxp2;
	struct cvmx_pko_mem_queue_qos_s cn50xx;
	struct cvmx_pko_mem_queue_qos_s cn52xx;
	struct cvmx_pko_mem_queue_qos_s cn52xxp1;
	struct cvmx_pko_mem_queue_qos_s cn56xx;
	struct cvmx_pko_mem_queue_qos_s cn56xxp1;
	struct cvmx_pko_mem_queue_qos_s cn58xx;
	struct cvmx_pko_mem_queue_qos_s cn58xxp1;
	struct cvmx_pko_mem_queue_qos_s cn61xx;
	struct cvmx_pko_mem_queue_qos_s cn63xx;
	struct cvmx_pko_mem_queue_qos_s cn63xxp1;
	struct cvmx_pko_mem_queue_qos_s cn66xx;
	struct cvmx_pko_mem_queue_qos_s cn70xx;
	struct cvmx_pko_mem_queue_qos_s cn70xxp1;
	struct cvmx_pko_mem_queue_qos_s cnf71xx;
};

typedef union cvmx_pko_mem_queue_qos cvmx_pko_mem_queue_qos_t;

/**
 * cvmx_pko_mem_throttle_int
 *
 * Notes:
 * Writing PACKET and WORD with 0 resets both counts for INT to 0 rather than add 0.
 * Otherwise, writes to this CSR add to the existing WORD/PACKET counts for the interface INT.
 *
 * PKO tracks the number of (8-byte) WORD's and PACKET's in-flight (sum total in both PKO
 * and the interface MAC) on the interface. (When PKO first selects a packet from a PKO queue, it
 * increments the counts appropriately. When the interface MAC has (largely) completed sending
 * the words/packet, PKO decrements the count appropriately.) When PKO_REG_FLAGS[ENA_THROTTLE]
 * is set and the most-significant bit of the WORD or packet count for a interface is set,
 * PKO will not transfer any packets over the interface. Software can limit the amount of
 * packet data and/or the number of packets that OCTEON can send out the chip after receiving backpressure
 * from the interface/pipe via these per-pipe throttle counts when PKO_REG_FLAGS[ENA_THROTTLE]=1.
 * For example, to limit the number of packets outstanding in the interface to N, preset PACKET for
 * the pipe to the value 0x20-N (0x20 is the smallest PACKET value with the most-significant bit set).
 *
 * This CSR is a memory of 32 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an INTERFACE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_throttle_int {
	u64 u64;
	struct cvmx_pko_mem_throttle_int_s {
		u64 reserved_47_63 : 17;
		u64 word : 15;
		u64 reserved_14_31 : 18;
		u64 packet : 6;
		u64 reserved_5_7 : 3;
		u64 intr : 5;
	} s;
	struct cvmx_pko_mem_throttle_int_s cn68xx;
	struct cvmx_pko_mem_throttle_int_s cn68xxp1;
};

typedef union cvmx_pko_mem_throttle_int cvmx_pko_mem_throttle_int_t;

/**
 * cvmx_pko_mem_throttle_pipe
 *
 * Notes:
 * Writing PACKET and WORD with 0 resets both counts for PIPE to 0 rather than add 0.
 * Otherwise, writes to this CSR add to the existing WORD/PACKET counts for the PKO pipe PIPE.
 *
 * PKO tracks the number of (8-byte) WORD's and PACKET's in-flight (sum total in both PKO
 * and the interface MAC) on the pipe. (When PKO first selects a packet from a PKO queue, it
 * increments the counts appropriately. When the interface MAC has (largely) completed sending
 * the words/packet, PKO decrements the count appropriately.) When PKO_REG_FLAGS[ENA_THROTTLE]
 * is set and the most-significant bit of the WORD or packet count for a PKO pipe is set,
 * PKO will not transfer any packets over the PKO pipe. Software can limit the amount of
 * packet data and/or the number of packets that OCTEON can send out the chip after receiving backpressure
 * from the interface/pipe via these per-pipe throttle counts when PKO_REG_FLAGS[ENA_THROTTLE]=1.
 * For example, to limit the number of packets outstanding in the pipe to N, preset PACKET for
 * the pipe to the value 0x20-N (0x20 is the smallest PACKET value with the most-significant bit set).
 *
 * This CSR is a memory of 128 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is a PIPE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_throttle_pipe {
	u64 u64;
	struct cvmx_pko_mem_throttle_pipe_s {
		u64 reserved_47_63 : 17;
		u64 word : 15;
		u64 reserved_14_31 : 18;
		u64 packet : 6;
		u64 reserved_7_7 : 1;
		u64 pipe : 7;
	} s;
	struct cvmx_pko_mem_throttle_pipe_s cn68xx;
	struct cvmx_pko_mem_throttle_pipe_s cn68xxp1;
};

typedef union cvmx_pko_mem_throttle_pipe cvmx_pko_mem_throttle_pipe_t;

/**
 * cvmx_pko_ncb_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_ncb_bist_status {
	u64 u64;
	struct cvmx_pko_ncb_bist_status_s {
		u64 ncbi_l2_out_ram_bist_status : 1;
		u64 ncbi_pp_out_ram_bist_status : 1;
		u64 ncbo_pdm_cmd_dat_ram_bist_status : 1;
		u64 ncbi_l2_pdm_pref_ram_bist_status : 1;
		u64 ncbo_pp_fif_ram_bist_status : 1;
		u64 ncbo_skid_fif_ram_bist_status : 1;
		u64 reserved_0_57 : 58;
	} s;
	struct cvmx_pko_ncb_bist_status_s cn73xx;
	struct cvmx_pko_ncb_bist_status_s cn78xx;
	struct cvmx_pko_ncb_bist_status_s cn78xxp1;
	struct cvmx_pko_ncb_bist_status_s cnf75xx;
};

typedef union cvmx_pko_ncb_bist_status cvmx_pko_ncb_bist_status_t;

/**
 * cvmx_pko_ncb_ecc_ctl0
 */
union cvmx_pko_ncb_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_ncb_ecc_ctl0_s {
		u64 ncbi_l2_out_ram_flip : 2;
		u64 ncbi_l2_out_ram_cdis : 1;
		u64 ncbi_pp_out_ram_flip : 2;
		u64 ncbi_pp_out_ram_cdis : 1;
		u64 ncbo_pdm_cmd_dat_ram_flip : 2;
		u64 ncbo_pdm_cmd_dat_ram_cdis : 1;
		u64 ncbi_l2_pdm_pref_ram_flip : 2;
		u64 ncbi_l2_pdm_pref_ram_cdis : 1;
		u64 ncbo_pp_fif_ram_flip : 2;
		u64 ncbo_pp_fif_ram_cdis : 1;
		u64 ncbo_skid_fif_ram_flip : 2;
		u64 ncbo_skid_fif_ram_cdis : 1;
		u64 reserved_0_45 : 46;
	} s;
	struct cvmx_pko_ncb_ecc_ctl0_s cn73xx;
	struct cvmx_pko_ncb_ecc_ctl0_s cn78xx;
	struct cvmx_pko_ncb_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_ncb_ecc_ctl0_s cnf75xx;
};

typedef union cvmx_pko_ncb_ecc_ctl0 cvmx_pko_ncb_ecc_ctl0_t;

/**
 * cvmx_pko_ncb_ecc_dbe_sts0
 */
union cvmx_pko_ncb_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s {
		u64 ncbi_l2_out_ram_dbe : 1;
		u64 ncbi_pp_out_ram_dbe : 1;
		u64 ncbo_pdm_cmd_dat_ram_dbe : 1;
		u64 ncbi_l2_pdm_pref_ram_dbe : 1;
		u64 ncbo_pp_fif_ram_dbe : 1;
		u64 ncbo_skid_fif_ram_dbe : 1;
		u64 reserved_0_57 : 58;
	} s;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s cn73xx;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s cnf75xx;
};

typedef union cvmx_pko_ncb_ecc_dbe_sts0 cvmx_pko_ncb_ecc_dbe_sts0_t;

/**
 * cvmx_pko_ncb_ecc_dbe_sts_cmb0
 */
union cvmx_pko_ncb_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s {
		u64 ncb_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_ncb_ecc_dbe_sts_cmb0 cvmx_pko_ncb_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_ncb_ecc_sbe_sts0
 */
union cvmx_pko_ncb_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s {
		u64 ncbi_l2_out_ram_sbe : 1;
		u64 ncbi_pp_out_ram_sbe : 1;
		u64 ncbo_pdm_cmd_dat_ram_sbe : 1;
		u64 ncbi_l2_pdm_pref_ram_sbe : 1;
		u64 ncbo_pp_fif_ram_sbe : 1;
		u64 ncbo_skid_fif_ram_sbe : 1;
		u64 reserved_0_57 : 58;
	} s;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s cn73xx;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s cnf75xx;
};

typedef union cvmx_pko_ncb_ecc_sbe_sts0 cvmx_pko_ncb_ecc_sbe_sts0_t;

/**
 * cvmx_pko_ncb_ecc_sbe_sts_cmb0
 */
union cvmx_pko_ncb_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s {
		u64 ncb_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_ncb_ecc_sbe_sts_cmb0 cvmx_pko_ncb_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_ncb_int
 */
union cvmx_pko_ncb_int {
	u64 u64;
	struct cvmx_pko_ncb_int_s {
		u64 reserved_2_63 : 62;
		u64 tso_segment_cnt : 1;
		u64 ncb_tx_error : 1;
	} s;
	struct cvmx_pko_ncb_int_s cn73xx;
	struct cvmx_pko_ncb_int_s cn78xx;
	struct cvmx_pko_ncb_int_s cn78xxp1;
	struct cvmx_pko_ncb_int_s cnf75xx;
};

typedef union cvmx_pko_ncb_int cvmx_pko_ncb_int_t;

/**
 * cvmx_pko_ncb_tx_err_info
 */
union cvmx_pko_ncb_tx_err_info {
	u64 u64;
	struct cvmx_pko_ncb_tx_err_info_s {
		u64 reserved_32_63 : 32;
		u64 wcnt : 5;
		u64 src : 12;
		u64 dst : 8;
		u64 tag : 4;
		u64 eot : 1;
		u64 sot : 1;
		u64 valid : 1;
	} s;
	struct cvmx_pko_ncb_tx_err_info_s cn73xx;
	struct cvmx_pko_ncb_tx_err_info_s cn78xx;
	struct cvmx_pko_ncb_tx_err_info_s cn78xxp1;
	struct cvmx_pko_ncb_tx_err_info_s cnf75xx;
};

typedef union cvmx_pko_ncb_tx_err_info cvmx_pko_ncb_tx_err_info_t;

/**
 * cvmx_pko_ncb_tx_err_word
 */
union cvmx_pko_ncb_tx_err_word {
	u64 u64;
	struct cvmx_pko_ncb_tx_err_word_s {
		u64 err_word : 64;
	} s;
	struct cvmx_pko_ncb_tx_err_word_s cn73xx;
	struct cvmx_pko_ncb_tx_err_word_s cn78xx;
	struct cvmx_pko_ncb_tx_err_word_s cn78xxp1;
	struct cvmx_pko_ncb_tx_err_word_s cnf75xx;
};

typedef union cvmx_pko_ncb_tx_err_word cvmx_pko_ncb_tx_err_word_t;

/**
 * cvmx_pko_pdm_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pdm_bist_status {
	u64 u64;
	struct cvmx_pko_pdm_bist_status_s {
		u64 flshb_cache_lo_ram_bist_status : 1;
		u64 flshb_cache_hi_ram_bist_status : 1;
		u64 isrm_ca_iinst_ram_bist_status : 1;
		u64 isrm_ca_cm_ram_bist_status : 1;
		u64 isrm_st_ram2_bist_status : 1;
		u64 isrm_st_ram1_bist_status : 1;
		u64 isrm_st_ram0_bist_status : 1;
		u64 isrd_st_ram3_bist_status : 1;
		u64 isrd_st_ram2_bist_status : 1;
		u64 isrd_st_ram1_bist_status : 1;
		u64 isrd_st_ram0_bist_status : 1;
		u64 drp_hi_ram_bist_status : 1;
		u64 drp_lo_ram_bist_status : 1;
		u64 dwp_hi_ram_bist_status : 1;
		u64 dwp_lo_ram_bist_status : 1;
		u64 mwp_hi_ram_bist_status : 1;
		u64 mwp_lo_ram_bist_status : 1;
		u64 fillb_m_rsp_ram_hi_bist_status : 1;
		u64 fillb_m_rsp_ram_lo_bist_status : 1;
		u64 fillb_d_rsp_ram_hi_bist_status : 1;
		u64 fillb_d_rsp_ram_lo_bist_status : 1;
		u64 fillb_d_rsp_dat_fifo_bist_status : 1;
		u64 fillb_m_rsp_dat_fifo_bist_status : 1;
		u64 flshb_m_dat_ram_bist_status : 1;
		u64 flshb_d_dat_ram_bist_status : 1;
		u64 minpad_ram_bist_status : 1;
		u64 mwp_hi_spt_ram_bist_status : 1;
		u64 mwp_lo_spt_ram_bist_status : 1;
		u64 buf_wm_ram_bist_status : 1;
		u64 reserved_0_34 : 35;
	} s;
	struct cvmx_pko_pdm_bist_status_s cn73xx;
	struct cvmx_pko_pdm_bist_status_s cn78xx;
	struct cvmx_pko_pdm_bist_status_s cn78xxp1;
	struct cvmx_pko_pdm_bist_status_s cnf75xx;
};

typedef union cvmx_pko_pdm_bist_status cvmx_pko_pdm_bist_status_t;

/**
 * cvmx_pko_pdm_cfg
 */
union cvmx_pko_pdm_cfg {
	u64 u64;
	struct cvmx_pko_pdm_cfg_s {
		u64 reserved_13_63 : 51;
		u64 dis_lpd_w2r_fill : 1;
		u64 en_fr_w2r_ptr_swp : 1;
		u64 dis_flsh_cache : 1;
		u64 pko_pad_minlen : 7;
		u64 diag_mode : 1;
		u64 alloc_lds : 1;
		u64 alloc_sts : 1;
	} s;
	struct cvmx_pko_pdm_cfg_s cn73xx;
	struct cvmx_pko_pdm_cfg_s cn78xx;
	struct cvmx_pko_pdm_cfg_s cn78xxp1;
	struct cvmx_pko_pdm_cfg_s cnf75xx;
};

typedef union cvmx_pko_pdm_cfg cvmx_pko_pdm_cfg_t;

/**
 * cvmx_pko_pdm_cfg_dbg
 */
union cvmx_pko_pdm_cfg_dbg {
	u64 u64;
	struct cvmx_pko_pdm_cfg_dbg_s {
		u64 reserved_32_63 : 32;
		u64 cp_stall_thrshld : 32;
	} s;
	struct cvmx_pko_pdm_cfg_dbg_s cn73xx;
	struct cvmx_pko_pdm_cfg_dbg_s cn78xx;
	struct cvmx_pko_pdm_cfg_dbg_s cn78xxp1;
	struct cvmx_pko_pdm_cfg_dbg_s cnf75xx;
};

typedef union cvmx_pko_pdm_cfg_dbg cvmx_pko_pdm_cfg_dbg_t;

/**
 * cvmx_pko_pdm_cp_dbg
 */
union cvmx_pko_pdm_cp_dbg {
	u64 u64;
	struct cvmx_pko_pdm_cp_dbg_s {
		u64 reserved_16_63 : 48;
		u64 stateless_fif_cnt : 6;
		u64 reserved_5_9 : 5;
		u64 op_fif_not_full : 5;
	} s;
	struct cvmx_pko_pdm_cp_dbg_s cn73xx;
	struct cvmx_pko_pdm_cp_dbg_s cn78xx;
	struct cvmx_pko_pdm_cp_dbg_s cn78xxp1;
	struct cvmx_pko_pdm_cp_dbg_s cnf75xx;
};

typedef union cvmx_pko_pdm_cp_dbg cvmx_pko_pdm_cp_dbg_t;

/**
 * cvmx_pko_pdm_dq#_minpad
 */
union cvmx_pko_pdm_dqx_minpad {
	u64 u64;
	struct cvmx_pko_pdm_dqx_minpad_s {
		u64 reserved_1_63 : 63;
		u64 minpad : 1;
	} s;
	struct cvmx_pko_pdm_dqx_minpad_s cn73xx;
	struct cvmx_pko_pdm_dqx_minpad_s cn78xx;
	struct cvmx_pko_pdm_dqx_minpad_s cn78xxp1;
	struct cvmx_pko_pdm_dqx_minpad_s cnf75xx;
};

typedef union cvmx_pko_pdm_dqx_minpad cvmx_pko_pdm_dqx_minpad_t;

/**
 * cvmx_pko_pdm_drpbuf_dbg
 */
union cvmx_pko_pdm_drpbuf_dbg {
	u64 u64;
	struct cvmx_pko_pdm_drpbuf_dbg_s {
		u64 reserved_43_63 : 21;
		u64 sel_nxt_ptr : 1;
		u64 load_val : 1;
		u64 rdy : 1;
		u64 cur_state : 3;
		u64 reserved_33_36 : 4;
		u64 track_rd_cnt : 6;
		u64 track_wr_cnt : 6;
		u64 reserved_17_20 : 4;
		u64 mem_addr : 13;
		u64 mem_en : 4;
	} s;
	struct cvmx_pko_pdm_drpbuf_dbg_s cn73xx;
	struct cvmx_pko_pdm_drpbuf_dbg_s cn78xx;
	struct cvmx_pko_pdm_drpbuf_dbg_s cn78xxp1;
	struct cvmx_pko_pdm_drpbuf_dbg_s cnf75xx;
};

typedef union cvmx_pko_pdm_drpbuf_dbg cvmx_pko_pdm_drpbuf_dbg_t;

/**
 * cvmx_pko_pdm_dwpbuf_dbg
 */
union cvmx_pko_pdm_dwpbuf_dbg {
	u64 u64;
	struct cvmx_pko_pdm_dwpbuf_dbg_s {
		u64 reserved_48_63 : 16;
		u64 cmd_proc : 1;
		u64 reserved_46_46 : 1;
		u64 mem_data_val : 1;
		u64 insert_np : 1;
		u64 reserved_43_43 : 1;
		u64 sel_nxt_ptr : 1;
		u64 load_val : 1;
		u64 rdy : 1;
		u64 cur_state : 3;
		u64 mem_rdy : 1;
		u64 reserved_33_35 : 3;
		u64 track_rd_cnt : 6;
		u64 track_wr_cnt : 6;
		u64 reserved_19_20 : 2;
		u64 insert_dp : 2;
		u64 mem_addr : 13;
		u64 mem_en : 4;
	} s;
	struct cvmx_pko_pdm_dwpbuf_dbg_cn73xx {
		u64 reserved_48_63 : 16;
		u64 cmd_proc : 1;
		u64 reserved_46_46 : 1;
		u64 mem_data_val : 1;
		u64 insert_np : 1;
		u64 reserved_43_43 : 1;
		u64 sel_nxt_ptr : 1;
		u64 load_val : 1;
		u64 rdy : 1;
		u64 reserved_37_39 : 3;
		u64 mem_rdy : 1;
		u64 reserved_19_35 : 17;
		u64 insert_dp : 2;
		u64 reserved_15_16 : 2;
		u64 mem_addr : 11;
		u64 mem_en : 4;
	} cn73xx;
	struct cvmx_pko_pdm_dwpbuf_dbg_s cn78xx;
	struct cvmx_pko_pdm_dwpbuf_dbg_s cn78xxp1;
	struct cvmx_pko_pdm_dwpbuf_dbg_cn73xx cnf75xx;
};

typedef union cvmx_pko_pdm_dwpbuf_dbg cvmx_pko_pdm_dwpbuf_dbg_t;

/**
 * cvmx_pko_pdm_ecc_ctl0
 */
union cvmx_pko_pdm_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pdm_ecc_ctl0_s {
		u64 flshb_cache_lo_ram_flip : 2;
		u64 flshb_cache_lo_ram_cdis : 1;
		u64 flshb_cache_hi_ram_flip : 2;
		u64 flshb_cache_hi_ram_cdis : 1;
		u64 isrm_ca_iinst_ram_flip : 2;
		u64 isrm_ca_iinst_ram_cdis : 1;
		u64 isrm_ca_cm_ram_flip : 2;
		u64 isrm_ca_cm_ram_cdis : 1;
		u64 isrm_st_ram2_flip : 2;
		u64 isrm_st_ram2_cdis : 1;
		u64 isrm_st_ram1_flip : 2;
		u64 isrm_st_ram1_cdis : 1;
		u64 isrm_st_ram0_flip : 2;
		u64 isrm_st_ram0_cdis : 1;
		u64 isrd_st_ram3_flip : 2;
		u64 isrd_st_ram3_cdis : 1;
		u64 isrd_st_ram2_flip : 2;
		u64 isrd_st_ram2_cdis : 1;
		u64 isrd_st_ram1_flip : 2;
		u64 isrd_st_ram1_cdis : 1;
		u64 isrd_st_ram0_flip : 2;
		u64 isrd_st_ram0_cdis : 1;
		u64 drp_hi_ram_flip : 2;
		u64 drp_hi_ram_cdis : 1;
		u64 drp_lo_ram_flip : 2;
		u64 drp_lo_ram_cdis : 1;
		u64 dwp_hi_ram_flip : 2;
		u64 dwp_hi_ram_cdis : 1;
		u64 dwp_lo_ram_flip : 2;
		u64 dwp_lo_ram_cdis : 1;
		u64 mwp_hi_ram_flip : 2;
		u64 mwp_hi_ram_cdis : 1;
		u64 mwp_lo_ram_flip : 2;
		u64 mwp_lo_ram_cdis : 1;
		u64 fillb_m_rsp_ram_hi_flip : 2;
		u64 fillb_m_rsp_ram_hi_cdis : 1;
		u64 fillb_m_rsp_ram_lo_flip : 2;
		u64 fillb_m_rsp_ram_lo_cdis : 1;
		u64 fillb_d_rsp_ram_hi_flip : 2;
		u64 fillb_d_rsp_ram_hi_cdis : 1;
		u64 fillb_d_rsp_ram_lo_flip : 2;
		u64 fillb_d_rsp_ram_lo_cdis : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_pdm_ecc_ctl0_cn73xx {
		u64 flshb_cache_lo_ram_flip : 2;
		u64 flshb_cache_lo_ram_cdis : 1;
		u64 flshb_cache_hi_ram_flip : 2;
		u64 flshb_cache_hi_ram_cdis : 1;
		u64 isrm_ca_iinst_ram_flip : 2;
		u64 isrm_ca_iinst_ram_cdis : 1;
		u64 isrm_ca_cm_ram_flip : 2;
		u64 isrm_ca_cm_ram_cdis : 1;
		u64 isrm_st_ram2_flip : 2;
		u64 isrm_st_ram2_cdis : 1;
		u64 isrm_st_ram1_flip : 2;
		u64 isrm_st_ram1_cdis : 1;
		u64 isrm_st_ram0_flip : 2;
		u64 isrm_st_ram0_cdis : 1;
		u64 isrd_st_ram3_flip : 2;
		u64 isrd_st_ram3_cdis : 1;
		u64 isrd_st_ram2_flip : 2;
		u64 isrd_st_ram2_cdis : 1;
		u64 isrd_st_ram1_flip : 2;
		u64 isrd_st_ram1_cdis : 1;
		u64 isrd_st_ram0_flip : 2;
		u64 isrd_st_ram0_cdis : 1;
		u64 drp_hi_ram_flip : 2;
		u64 drp_hi_ram_cdis : 1;
		u64 drp_lo_ram_flip : 2;
		u64 drp_lo_ram_cdis : 1;
		u64 dwp_hi_ram_flip : 2;
		u64 dwp_hi_ram_cdis : 1;
		u64 dwp_lo_ram_flip : 2;
		u64 dwp_lo_ram_cdis : 1;
		u64 reserved_13_18 : 6;
		u64 fillb_m_rsp_ram_hi_flip : 2;
		u64 fillb_m_rsp_ram_hi_cdis : 1;
		u64 fillb_m_rsp_ram_lo_flip : 2;
		u64 fillb_m_rsp_ram_lo_cdis : 1;
		u64 fillb_d_rsp_ram_hi_flip : 2;
		u64 fillb_d_rsp_ram_hi_cdis : 1;
		u64 fillb_d_rsp_ram_lo_flip : 2;
		u64 fillb_d_rsp_ram_lo_cdis : 1;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_pdm_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pdm_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_pdm_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pdm_ecc_ctl0 cvmx_pko_pdm_ecc_ctl0_t;

/**
 * cvmx_pko_pdm_ecc_ctl1
 */
union cvmx_pko_pdm_ecc_ctl1 {
	u64 u64;
	struct cvmx_pko_pdm_ecc_ctl1_s {
		u64 reserved_15_63 : 49;
		u64 buf_wm_ram_flip : 2;
		u64 buf_wm_ram_cdis : 1;
		u64 mwp_mem0_ram_flip : 2;
		u64 mwp_mem1_ram_flip : 2;
		u64 mwp_mem2_ram_flip : 2;
		u64 mwp_mem3_ram_flip : 2;
		u64 mwp_ram_cdis : 1;
		u64 minpad_ram_flip : 2;
		u64 minpad_ram_cdis : 1;
	} s;
	struct cvmx_pko_pdm_ecc_ctl1_s cn73xx;
	struct cvmx_pko_pdm_ecc_ctl1_s cn78xx;
	struct cvmx_pko_pdm_ecc_ctl1_s cn78xxp1;
	struct cvmx_pko_pdm_ecc_ctl1_s cnf75xx;
};

typedef union cvmx_pko_pdm_ecc_ctl1 cvmx_pko_pdm_ecc_ctl1_t;

/**
 * cvmx_pko_pdm_ecc_dbe_sts0
 */
union cvmx_pko_pdm_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s {
		u64 flshb_cache_lo_ram_dbe : 1;
		u64 flshb_cache_hi_ram_dbe : 1;
		u64 isrm_ca_iinst_ram_dbe : 1;
		u64 isrm_ca_cm_ram_dbe : 1;
		u64 isrm_st_ram2_dbe : 1;
		u64 isrm_st_ram1_dbe : 1;
		u64 isrm_st_ram0_dbe : 1;
		u64 isrd_st_ram3_dbe : 1;
		u64 isrd_st_ram2_dbe : 1;
		u64 isrd_st_ram1_dbe : 1;
		u64 isrd_st_ram0_dbe : 1;
		u64 drp_hi_ram_dbe : 1;
		u64 drp_lo_ram_dbe : 1;
		u64 dwp_hi_ram_dbe : 1;
		u64 dwp_lo_ram_dbe : 1;
		u64 mwp_hi_ram_dbe : 1;
		u64 mwp_lo_ram_dbe : 1;
		u64 fillb_m_rsp_ram_hi_dbe : 1;
		u64 fillb_m_rsp_ram_lo_dbe : 1;
		u64 fillb_d_rsp_ram_hi_dbe : 1;
		u64 fillb_d_rsp_ram_lo_dbe : 1;
		u64 minpad_ram_dbe : 1;
		u64 mwp_hi_spt_ram_dbe : 1;
		u64 mwp_lo_spt_ram_dbe : 1;
		u64 buf_wm_ram_dbe : 1;
		u64 reserved_0_38 : 39;
	} s;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s cn73xx;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s cnf75xx;
};

typedef union cvmx_pko_pdm_ecc_dbe_sts0 cvmx_pko_pdm_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pdm_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pdm_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s {
		u64 pdm_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pdm_ecc_dbe_sts_cmb0 cvmx_pko_pdm_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pdm_ecc_sbe_sts0
 */
union cvmx_pko_pdm_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s {
		u64 flshb_cache_lo_ram_sbe : 1;
		u64 flshb_cache_hi_ram_sbe : 1;
		u64 isrm_ca_iinst_ram_sbe : 1;
		u64 isrm_ca_cm_ram_sbe : 1;
		u64 isrm_st_ram2_sbe : 1;
		u64 isrm_st_ram1_sbe : 1;
		u64 isrm_st_ram0_sbe : 1;
		u64 isrd_st_ram3_sbe : 1;
		u64 isrd_st_ram2_sbe : 1;
		u64 isrd_st_ram1_sbe : 1;
		u64 isrd_st_ram0_sbe : 1;
		u64 drp_hi_ram_sbe : 1;
		u64 drp_lo_ram_sbe : 1;
		u64 dwp_hi_ram_sbe : 1;
		u64 dwp_lo_ram_sbe : 1;
		u64 mwp_hi_ram_sbe : 1;
		u64 mwp_lo_ram_sbe : 1;
		u64 fillb_m_rsp_ram_hi_sbe : 1;
		u64 fillb_m_rsp_ram_lo_sbe : 1;
		u64 fillb_d_rsp_ram_hi_sbe : 1;
		u64 fillb_d_rsp_ram_lo_sbe : 1;
		u64 minpad_ram_sbe : 1;
		u64 mwp_hi_spt_ram_sbe : 1;
		u64 mwp_lo_spt_ram_sbe : 1;
		u64 buf_wm_ram_sbe : 1;
		u64 reserved_0_38 : 39;
	} s;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s cn73xx;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s cnf75xx;
};

typedef union cvmx_pko_pdm_ecc_sbe_sts0 cvmx_pko_pdm_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pdm_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pdm_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s {
		u64 pdm_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pdm_ecc_sbe_sts_cmb0 cvmx_pko_pdm_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pdm_fillb_dbg0
 */
union cvmx_pko_pdm_fillb_dbg0 {
	u64 u64;
	struct cvmx_pko_pdm_fillb_dbg0_s {
		u64 reserved_57_63 : 7;
		u64 pd_seq : 5;
		u64 resp_pd_seq : 5;
		u64 d_rsp_lo_ram_addr_sel : 2;
		u64 d_rsp_hi_ram_addr_sel : 2;
		u64 d_rsp_rd_seq : 5;
		u64 d_rsp_fifo_rd_seq : 5;
		u64 d_fill_req_fifo_val : 1;
		u64 d_rsp_ram_valid : 32;
	} s;
	struct cvmx_pko_pdm_fillb_dbg0_s cn73xx;
	struct cvmx_pko_pdm_fillb_dbg0_s cn78xx;
	struct cvmx_pko_pdm_fillb_dbg0_s cn78xxp1;
	struct cvmx_pko_pdm_fillb_dbg0_s cnf75xx;
};

typedef union cvmx_pko_pdm_fillb_dbg0 cvmx_pko_pdm_fillb_dbg0_t;

/**
 * cvmx_pko_pdm_fillb_dbg1
 */
union cvmx_pko_pdm_fillb_dbg1 {
	u64 u64;
	struct cvmx_pko_pdm_fillb_dbg1_s {
		u64 reserved_57_63 : 7;
		u64 mp_seq : 5;
		u64 resp_mp_seq : 5;
		u64 m_rsp_lo_ram_addr_sel : 2;
		u64 m_rsp_hi_ram_addr_sel : 2;
		u64 m_rsp_rd_seq : 5;
		u64 m_rsp_fifo_rd_seq : 5;
		u64 m_fill_req_fifo_val : 1;
		u64 m_rsp_ram_valid : 32;
	} s;
	struct cvmx_pko_pdm_fillb_dbg1_s cn73xx;
	struct cvmx_pko_pdm_fillb_dbg1_s cn78xx;
	struct cvmx_pko_pdm_fillb_dbg1_s cn78xxp1;
	struct cvmx_pko_pdm_fillb_dbg1_s cnf75xx;
};

typedef union cvmx_pko_pdm_fillb_dbg1 cvmx_pko_pdm_fillb_dbg1_t;

/**
 * cvmx_pko_pdm_fillb_dbg2
 */
union cvmx_pko_pdm_fillb_dbg2 {
	u64 u64;
	struct cvmx_pko_pdm_fillb_dbg2_s {
		u64 reserved_9_63 : 55;
		u64 fillb_sm : 5;
		u64 reserved_3_3 : 1;
		u64 iobp0_credit_cntr : 3;
	} s;
	struct cvmx_pko_pdm_fillb_dbg2_s cn73xx;
	struct cvmx_pko_pdm_fillb_dbg2_s cn78xx;
	struct cvmx_pko_pdm_fillb_dbg2_s cn78xxp1;
	struct cvmx_pko_pdm_fillb_dbg2_s cnf75xx;
};

typedef union cvmx_pko_pdm_fillb_dbg2 cvmx_pko_pdm_fillb_dbg2_t;

/**
 * cvmx_pko_pdm_flshb_dbg0
 */
union cvmx_pko_pdm_flshb_dbg0 {
	u64 u64;
	struct cvmx_pko_pdm_flshb_dbg0_s {
		u64 reserved_44_63 : 20;
		u64 flshb_sm : 7;
		u64 flshb_ctl_sm : 9;
		u64 cam_hptr : 5;
		u64 cam_tptr : 5;
		u64 expected_stdns : 6;
		u64 d_flshb_eot_cntr : 3;
		u64 m_flshb_eot_cntr : 3;
		u64 ncbi_credit_cntr : 6;
	} s;
	struct cvmx_pko_pdm_flshb_dbg0_s cn73xx;
	struct cvmx_pko_pdm_flshb_dbg0_s cn78xx;
	struct cvmx_pko_pdm_flshb_dbg0_s cn78xxp1;
	struct cvmx_pko_pdm_flshb_dbg0_s cnf75xx;
};

typedef union cvmx_pko_pdm_flshb_dbg0 cvmx_pko_pdm_flshb_dbg0_t;

/**
 * cvmx_pko_pdm_flshb_dbg1
 */
union cvmx_pko_pdm_flshb_dbg1 {
	u64 u64;
	struct cvmx_pko_pdm_flshb_dbg1_s {
		u64 cam_stdn : 32;
		u64 cam_valid : 32;
	} s;
	struct cvmx_pko_pdm_flshb_dbg1_s cn73xx;
	struct cvmx_pko_pdm_flshb_dbg1_s cn78xx;
	struct cvmx_pko_pdm_flshb_dbg1_s cn78xxp1;
	struct cvmx_pko_pdm_flshb_dbg1_s cnf75xx;
};

typedef union cvmx_pko_pdm_flshb_dbg1 cvmx_pko_pdm_flshb_dbg1_t;

/**
 * cvmx_pko_pdm_intf_dbg_rd
 *
 * For diagnostic use only.
 *
 */
union cvmx_pko_pdm_intf_dbg_rd {
	u64 u64;
	struct cvmx_pko_pdm_intf_dbg_rd_s {
		u64 reserved_48_63 : 16;
		u64 in_flight : 8;
		u64 pdm_req_cred_cnt : 8;
		u64 pse_buf_waddr : 8;
		u64 pse_buf_raddr : 8;
		u64 resp_buf_waddr : 8;
		u64 resp_buf_raddr : 8;
	} s;
	struct cvmx_pko_pdm_intf_dbg_rd_s cn73xx;
	struct cvmx_pko_pdm_intf_dbg_rd_s cn78xx;
	struct cvmx_pko_pdm_intf_dbg_rd_s cn78xxp1;
	struct cvmx_pko_pdm_intf_dbg_rd_s cnf75xx;
};

typedef union cvmx_pko_pdm_intf_dbg_rd cvmx_pko_pdm_intf_dbg_rd_t;

/**
 * cvmx_pko_pdm_isrd_dbg
 */
union cvmx_pko_pdm_isrd_dbg {
	u64 u64;
	struct cvmx_pko_pdm_isrd_dbg_s {
		u64 isrd_vals_in : 4;
		u64 reserved_59_59 : 1;
		u64 req_hptr : 6;
		u64 rdy_hptr : 6;
		u64 reserved_44_46 : 3;
		u64 in_arb_reqs : 8;
		u64 in_arb_gnts : 7;
		u64 cmt_arb_reqs : 7;
		u64 cmt_arb_gnts : 7;
		u64 in_use : 4;
		u64 has_cred : 4;
		u64 val_exec : 7;
	} s;
	struct cvmx_pko_pdm_isrd_dbg_s cn73xx;
	struct cvmx_pko_pdm_isrd_dbg_s cn78xx;
	struct cvmx_pko_pdm_isrd_dbg_cn78xxp1 {
		u64 reserved_44_63 : 20;
		u64 in_arb_reqs : 8;
		u64 in_arb_gnts : 7;
		u64 cmt_arb_reqs : 7;
		u64 cmt_arb_gnts : 7;
		u64 in_use : 4;
		u64 has_cred : 4;
		u64 val_exec : 7;
	} cn78xxp1;
	struct cvmx_pko_pdm_isrd_dbg_s cnf75xx;
};

typedef union cvmx_pko_pdm_isrd_dbg cvmx_pko_pdm_isrd_dbg_t;

/**
 * cvmx_pko_pdm_isrd_dbg_dq
 */
union cvmx_pko_pdm_isrd_dbg_dq {
	u64 u64;
	struct cvmx_pko_pdm_isrd_dbg_dq_s {
		u64 reserved_46_63 : 18;
		u64 pebrd_sic_dq : 10;
		u64 reserved_34_35 : 2;
		u64 pebfill_sic_dq : 10;
		u64 reserved_22_23 : 2;
		u64 fr_sic_dq : 10;
		u64 reserved_10_11 : 2;
		u64 cp_sic_dq : 10;
	} s;
	struct cvmx_pko_pdm_isrd_dbg_dq_s cn73xx;
	struct cvmx_pko_pdm_isrd_dbg_dq_s cn78xx;
	struct cvmx_pko_pdm_isrd_dbg_dq_s cn78xxp1;
	struct cvmx_pko_pdm_isrd_dbg_dq_s cnf75xx;
};

typedef union cvmx_pko_pdm_isrd_dbg_dq cvmx_pko_pdm_isrd_dbg_dq_t;

/**
 * cvmx_pko_pdm_isrm_dbg
 */
union cvmx_pko_pdm_isrm_dbg {
	u64 u64;
	struct cvmx_pko_pdm_isrm_dbg_s {
		u64 val_in : 3;
		u64 reserved_34_60 : 27;
		u64 in_arb_reqs : 7;
		u64 in_arb_gnts : 6;
		u64 cmt_arb_reqs : 6;
		u64 cmt_arb_gnts : 6;
		u64 in_use : 3;
		u64 has_cred : 3;
		u64 val_exec : 3;
	} s;
	struct cvmx_pko_pdm_isrm_dbg_s cn73xx;
	struct cvmx_pko_pdm_isrm_dbg_s cn78xx;
	struct cvmx_pko_pdm_isrm_dbg_cn78xxp1 {
		u64 reserved_34_63 : 30;
		u64 in_arb_reqs : 7;
		u64 in_arb_gnts : 6;
		u64 cmt_arb_reqs : 6;
		u64 cmt_arb_gnts : 6;
		u64 in_use : 3;
		u64 has_cred : 3;
		u64 val_exec : 3;
	} cn78xxp1;
	struct cvmx_pko_pdm_isrm_dbg_s cnf75xx;
};

typedef union cvmx_pko_pdm_isrm_dbg cvmx_pko_pdm_isrm_dbg_t;

/**
 * cvmx_pko_pdm_isrm_dbg_dq
 */
union cvmx_pko_pdm_isrm_dbg_dq {
	u64 u64;
	struct cvmx_pko_pdm_isrm_dbg_dq_s {
		u64 reserved_34_63 : 30;
		u64 ack_sic_dq : 10;
		u64 reserved_22_23 : 2;
		u64 fr_sic_dq : 10;
		u64 reserved_10_11 : 2;
		u64 cp_sic_dq : 10;
	} s;
	struct cvmx_pko_pdm_isrm_dbg_dq_s cn73xx;
	struct cvmx_pko_pdm_isrm_dbg_dq_s cn78xx;
	struct cvmx_pko_pdm_isrm_dbg_dq_s cn78xxp1;
	struct cvmx_pko_pdm_isrm_dbg_dq_s cnf75xx;
};

typedef union cvmx_pko_pdm_isrm_dbg_dq cvmx_pko_pdm_isrm_dbg_dq_t;

/**
 * cvmx_pko_pdm_mem_addr
 */
union cvmx_pko_pdm_mem_addr {
	u64 u64;
	struct cvmx_pko_pdm_mem_addr_s {
		u64 memsel : 3;
		u64 reserved_17_60 : 44;
		u64 memaddr : 14;
		u64 reserved_2_2 : 1;
		u64 membanksel : 2;
	} s;
	struct cvmx_pko_pdm_mem_addr_s cn73xx;
	struct cvmx_pko_pdm_mem_addr_s cn78xx;
	struct cvmx_pko_pdm_mem_addr_s cn78xxp1;
	struct cvmx_pko_pdm_mem_addr_s cnf75xx;
};

typedef union cvmx_pko_pdm_mem_addr cvmx_pko_pdm_mem_addr_t;

/**
 * cvmx_pko_pdm_mem_data
 */
union cvmx_pko_pdm_mem_data {
	u64 u64;
	struct cvmx_pko_pdm_mem_data_s {
		u64 data : 64;
	} s;
	struct cvmx_pko_pdm_mem_data_s cn73xx;
	struct cvmx_pko_pdm_mem_data_s cn78xx;
	struct cvmx_pko_pdm_mem_data_s cn78xxp1;
	struct cvmx_pko_pdm_mem_data_s cnf75xx;
};

typedef union cvmx_pko_pdm_mem_data cvmx_pko_pdm_mem_data_t;

/**
 * cvmx_pko_pdm_mem_rw_ctl
 */
union cvmx_pko_pdm_mem_rw_ctl {
	u64 u64;
	struct cvmx_pko_pdm_mem_rw_ctl_s {
		u64 reserved_2_63 : 62;
		u64 read : 1;
		u64 write : 1;
	} s;
	struct cvmx_pko_pdm_mem_rw_ctl_s cn73xx;
	struct cvmx_pko_pdm_mem_rw_ctl_s cn78xx;
	struct cvmx_pko_pdm_mem_rw_ctl_s cn78xxp1;
	struct cvmx_pko_pdm_mem_rw_ctl_s cnf75xx;
};

typedef union cvmx_pko_pdm_mem_rw_ctl cvmx_pko_pdm_mem_rw_ctl_t;

/**
 * cvmx_pko_pdm_mem_rw_sts
 */
union cvmx_pko_pdm_mem_rw_sts {
	u64 u64;
	struct cvmx_pko_pdm_mem_rw_sts_s {
		u64 reserved_1_63 : 63;
		u64 readdone : 1;
	} s;
	struct cvmx_pko_pdm_mem_rw_sts_s cn73xx;
	struct cvmx_pko_pdm_mem_rw_sts_s cn78xx;
	struct cvmx_pko_pdm_mem_rw_sts_s cn78xxp1;
	struct cvmx_pko_pdm_mem_rw_sts_s cnf75xx;
};

typedef union cvmx_pko_pdm_mem_rw_sts cvmx_pko_pdm_mem_rw_sts_t;

/**
 * cvmx_pko_pdm_mwpbuf_dbg
 */
union cvmx_pko_pdm_mwpbuf_dbg {
	u64 u64;
	struct cvmx_pko_pdm_mwpbuf_dbg_s {
		u64 reserved_49_63 : 15;
		u64 str_proc : 1;
		u64 cmd_proc : 1;
		u64 str_val : 1;
		u64 mem_data_val : 1;
		u64 insert_np : 1;
		u64 insert_mp : 1;
		u64 sel_nxt_ptr : 1;
		u64 load_val : 1;
		u64 rdy : 1;
		u64 cur_state : 3;
		u64 mem_rdy : 1;
		u64 str_rdy : 1;
		u64 contention_type : 2;
		u64 track_rd_cnt : 6;
		u64 track_wr_cnt : 6;
		u64 mem_wen : 4;
		u64 mem_addr : 13;
		u64 mem_en : 4;
	} s;
	struct cvmx_pko_pdm_mwpbuf_dbg_cn73xx {
		u64 reserved_49_63 : 15;
		u64 str_proc : 1;
		u64 cmd_proc : 1;
		u64 str_val : 1;
		u64 mem_data_val : 1;
		u64 insert_np : 1;
		u64 insert_mp : 1;
		u64 sel_nxt_ptr : 1;
		u64 load_val : 1;
		u64 rdy : 1;
		u64 cur_state : 3;
		u64 mem_rdy : 1;
		u64 str_rdy : 1;
		u64 contention_type : 2;
		u64 reserved_21_32 : 12;
		u64 mem_wen : 4;
		u64 reserved_15_16 : 2;
		u64 mem_addr : 11;
		u64 mem_en : 4;
	} cn73xx;
	struct cvmx_pko_pdm_mwpbuf_dbg_s cn78xx;
	struct cvmx_pko_pdm_mwpbuf_dbg_s cn78xxp1;
	struct cvmx_pko_pdm_mwpbuf_dbg_cn73xx cnf75xx;
};

typedef union cvmx_pko_pdm_mwpbuf_dbg cvmx_pko_pdm_mwpbuf_dbg_t;

/**
 * cvmx_pko_pdm_sts
 */
union cvmx_pko_pdm_sts {
	u64 u64;
	struct cvmx_pko_pdm_sts_s {
		u64 reserved_38_63 : 26;
		u64 cp_stalled_thrshld_hit : 1;
		u64 reserved_35_36 : 2;
		u64 mwpbuf_data_val_err : 1;
		u64 drpbuf_data_val_err : 1;
		u64 dwpbuf_data_val_err : 1;
		u64 reserved_30_31 : 2;
		u64 qcmd_iobx_err_sts : 4;
		u64 qcmd_iobx_err : 1;
		u64 sendpkt_lmtdma_err_sts : 4;
		u64 sendpkt_lmtdma_err : 1;
		u64 sendpkt_lmtst_err_sts : 4;
		u64 sendpkt_lmtst_err : 1;
		u64 fpa_no_ptrs : 1;
		u64 reserved_12_13 : 2;
		u64 cp_sendpkt_err_no_drp_code : 2;
		u64 cp_sendpkt_err_no_drp : 1;
		u64 reserved_7_8 : 2;
		u64 cp_sendpkt_err_drop_code : 3;
		u64 cp_sendpkt_err_drop : 1;
		u64 reserved_1_2 : 2;
		u64 desc_crc_err : 1;
	} s;
	struct cvmx_pko_pdm_sts_s cn73xx;
	struct cvmx_pko_pdm_sts_s cn78xx;
	struct cvmx_pko_pdm_sts_s cn78xxp1;
	struct cvmx_pko_pdm_sts_s cnf75xx;
};

typedef union cvmx_pko_pdm_sts cvmx_pko_pdm_sts_t;

/**
 * cvmx_pko_peb_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_peb_bist_status {
	u64 u64;
	struct cvmx_pko_peb_bist_status_s {
		u64 reserved_26_63 : 38;
		u64 add_work_fifo : 1;
		u64 pdm_pse_buf_ram : 1;
		u64 iobp0_fifo_ram : 1;
		u64 iobp1_fifo_ram : 1;
		u64 state_mem0 : 1;
		u64 reserved_19_20 : 2;
		u64 state_mem3 : 1;
		u64 iobp1_uid_fifo_ram : 1;
		u64 nxt_link_ptr_ram : 1;
		u64 pd_bank0_ram : 1;
		u64 pd_bank1_ram : 1;
		u64 pd_bank2_ram : 1;
		u64 pd_bank3_ram : 1;
		u64 pd_var_bank_ram : 1;
		u64 pdm_resp_buf_ram : 1;
		u64 tx_fifo_pkt_ram : 1;
		u64 tx_fifo_hdr_ram : 1;
		u64 tx_fifo_crc_ram : 1;
		u64 ts_addwork_ram : 1;
		u64 send_mem_ts_fifo : 1;
		u64 send_mem_stdn_fifo : 1;
		u64 send_mem_fifo : 1;
		u64 pkt_mrk_ram : 1;
		u64 peb_st_inf_ram : 1;
		u64 peb_sm_jmp_ram : 1;
	} s;
	struct cvmx_pko_peb_bist_status_cn73xx {
		u64 reserved_26_63 : 38;
		u64 add_work_fifo : 1;
		u64 pdm_pse_buf_ram : 1;
		u64 iobp0_fifo_ram : 1;
		u64 iobp1_fifo_ram : 1;
		u64 state_mem0 : 1;
		u64 reserved_19_20 : 2;
		u64 state_mem3 : 1;
		u64 iobp1_uid_fifo_ram : 1;
		u64 nxt_link_ptr_ram : 1;
		u64 pd_bank0_ram : 1;
		u64 reserved_13_14 : 2;
		u64 pd_bank3_ram : 1;
		u64 pd_var_bank_ram : 1;
		u64 pdm_resp_buf_ram : 1;
		u64 tx_fifo_pkt_ram : 1;
		u64 tx_fifo_hdr_ram : 1;
		u64 tx_fifo_crc_ram : 1;
		u64 ts_addwork_ram : 1;
		u64 send_mem_ts_fifo : 1;
		u64 send_mem_stdn_fifo : 1;
		u64 send_mem_fifo : 1;
		u64 pkt_mrk_ram : 1;
		u64 peb_st_inf_ram : 1;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_peb_bist_status_cn73xx cn78xx;
	struct cvmx_pko_peb_bist_status_s cn78xxp1;
	struct cvmx_pko_peb_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_peb_bist_status cvmx_pko_peb_bist_status_t;

/**
 * cvmx_pko_peb_ecc_ctl0
 */
union cvmx_pko_peb_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_peb_ecc_ctl0_s {
		u64 iobp1_uid_fifo_ram_flip : 2;
		u64 iobp1_uid_fifo_ram_cdis : 1;
		u64 iobp0_fifo_ram_flip : 2;
		u64 iobp0_fifo_ram_cdis : 1;
		u64 iobp1_fifo_ram_flip : 2;
		u64 iobp1_fifo_ram_cdis : 1;
		u64 pdm_resp_buf_ram_flip : 2;
		u64 pdm_resp_buf_ram_cdis : 1;
		u64 pdm_pse_buf_ram_flip : 2;
		u64 pdm_pse_buf_ram_cdis : 1;
		u64 peb_sm_jmp_ram_flip : 2;
		u64 peb_sm_jmp_ram_cdis : 1;
		u64 peb_st_inf_ram_flip : 2;
		u64 peb_st_inf_ram_cdis : 1;
		u64 pd_bank3_ram_flip : 2;
		u64 pd_bank3_ram_cdis : 1;
		u64 pd_bank2_ram_flip : 2;
		u64 pd_bank2_ram_cdis : 1;
		u64 pd_bank1_ram_flip : 2;
		u64 pd_bank1_ram_cdis : 1;
		u64 pd_bank0_ram_flip : 2;
		u64 pd_bank0_ram_cdis : 1;
		u64 pd_var_bank_ram_flip : 2;
		u64 pd_var_bank_ram_cdis : 1;
		u64 tx_fifo_crc_ram_flip : 2;
		u64 tx_fifo_crc_ram_cdis : 1;
		u64 tx_fifo_hdr_ram_flip : 2;
		u64 tx_fifo_hdr_ram_cdis : 1;
		u64 tx_fifo_pkt_ram_flip : 2;
		u64 tx_fifo_pkt_ram_cdis : 1;
		u64 add_work_fifo_flip : 2;
		u64 add_work_fifo_cdis : 1;
		u64 send_mem_fifo_flip : 2;
		u64 send_mem_fifo_cdis : 1;
		u64 send_mem_stdn_fifo_flip : 2;
		u64 send_mem_stdn_fifo_cdis : 1;
		u64 send_mem_ts_fifo_flip : 2;
		u64 send_mem_ts_fifo_cdis : 1;
		u64 nxt_link_ptr_ram_flip : 2;
		u64 nxt_link_ptr_ram_cdis : 1;
		u64 pkt_mrk_ram_flip : 2;
		u64 pkt_mrk_ram_cdis : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_peb_ecc_ctl0_cn73xx {
		u64 iobp1_uid_fifo_ram_flip : 2;
		u64 iobp1_uid_fifo_ram_cdis : 1;
		u64 iobp0_fifo_ram_flip : 2;
		u64 iobp0_fifo_ram_cdis : 1;
		u64 iobp1_fifo_ram_flip : 2;
		u64 iobp1_fifo_ram_cdis : 1;
		u64 pdm_resp_buf_ram_flip : 2;
		u64 pdm_resp_buf_ram_cdis : 1;
		u64 pdm_pse_buf_ram_flip : 2;
		u64 pdm_pse_buf_ram_cdis : 1;
		u64 reserved_46_48 : 3;
		u64 peb_st_inf_ram_flip : 2;
		u64 peb_st_inf_ram_cdis : 1;
		u64 pd_bank3_ram_flip : 2;
		u64 pd_bank3_ram_cdis : 1;
		u64 reserved_34_39 : 6;
		u64 pd_bank0_ram_flip : 2;
		u64 pd_bank0_ram_cdis : 1;
		u64 pd_var_bank_ram_flip : 2;
		u64 pd_var_bank_ram_cdis : 1;
		u64 tx_fifo_crc_ram_flip : 2;
		u64 tx_fifo_crc_ram_cdis : 1;
		u64 tx_fifo_hdr_ram_flip : 2;
		u64 tx_fifo_hdr_ram_cdis : 1;
		u64 tx_fifo_pkt_ram_flip : 2;
		u64 tx_fifo_pkt_ram_cdis : 1;
		u64 add_work_fifo_flip : 2;
		u64 add_work_fifo_cdis : 1;
		u64 send_mem_fifo_flip : 2;
		u64 send_mem_fifo_cdis : 1;
		u64 send_mem_stdn_fifo_flip : 2;
		u64 send_mem_stdn_fifo_cdis : 1;
		u64 send_mem_ts_fifo_flip : 2;
		u64 send_mem_ts_fifo_cdis : 1;
		u64 nxt_link_ptr_ram_flip : 2;
		u64 nxt_link_ptr_ram_cdis : 1;
		u64 pkt_mrk_ram_flip : 2;
		u64 pkt_mrk_ram_cdis : 1;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_peb_ecc_ctl0_cn73xx cn78xx;
	struct cvmx_pko_peb_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_peb_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_peb_ecc_ctl0 cvmx_pko_peb_ecc_ctl0_t;

/**
 * cvmx_pko_peb_ecc_ctl1
 */
union cvmx_pko_peb_ecc_ctl1 {
	u64 u64;
	struct cvmx_pko_peb_ecc_ctl1_s {
		u64 ts_addwork_ram_flip : 2;
		u64 ts_addwork_ram_cdis : 1;
		u64 state_mem0_flip : 2;
		u64 state_mem0_cdis : 1;
		u64 reserved_52_57 : 6;
		u64 state_mem3_flip : 2;
		u64 state_mem3_cdis : 1;
		u64 reserved_0_48 : 49;
	} s;
	struct cvmx_pko_peb_ecc_ctl1_s cn73xx;
	struct cvmx_pko_peb_ecc_ctl1_cn78xx {
		u64 ts_addwork_ram_flip : 2;
		u64 ts_addwork_ram_cdis : 1;
		u64 reserved_0_60 : 61;
	} cn78xx;
	struct cvmx_pko_peb_ecc_ctl1_cn78xx cn78xxp1;
	struct cvmx_pko_peb_ecc_ctl1_s cnf75xx;
};

typedef union cvmx_pko_peb_ecc_ctl1 cvmx_pko_peb_ecc_ctl1_t;

/**
 * cvmx_pko_peb_ecc_dbe_sts0
 */
union cvmx_pko_peb_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_peb_ecc_dbe_sts0_s {
		u64 iobp1_uid_fifo_ram_dbe : 1;
		u64 iobp0_fifo_ram_dbe : 1;
		u64 iobp1_fifo_ram_dbe : 1;
		u64 pdm_resp_buf_ram_dbe : 1;
		u64 pdm_pse_buf_ram_dbe : 1;
		u64 peb_sm_jmp_ram_dbe : 1;
		u64 peb_st_inf_ram_dbe : 1;
		u64 pd_bank3_ram_dbe : 1;
		u64 pd_bank2_ram_dbe : 1;
		u64 pd_bank1_ram_dbe : 1;
		u64 pd_bank0_ram_dbe : 1;
		u64 pd_var_bank_ram_dbe : 1;
		u64 tx_fifo_crc_ram_dbe : 1;
		u64 tx_fifo_hdr_ram_dbe : 1;
		u64 tx_fifo_pkt_ram_dbe : 1;
		u64 add_work_fifo_dbe : 1;
		u64 send_mem_fifo_dbe : 1;
		u64 send_mem_stdn_fifo_dbe : 1;
		u64 send_mem_ts_fifo_dbe : 1;
		u64 nxt_link_ptr_ram_dbe : 1;
		u64 pkt_mrk_ram_dbe : 1;
		u64 ts_addwork_ram_dbe : 1;
		u64 state_mem0_dbe : 1;
		u64 reserved_39_40 : 2;
		u64 state_mem3_dbe : 1;
		u64 reserved_0_37 : 38;
	} s;
	struct cvmx_pko_peb_ecc_dbe_sts0_cn73xx {
		u64 iobp1_uid_fifo_ram_dbe : 1;
		u64 iobp0_fifo_ram_dbe : 1;
		u64 iobp1_fifo_ram_dbe : 1;
		u64 pdm_resp_buf_ram_dbe : 1;
		u64 pdm_pse_buf_ram_dbe : 1;
		u64 reserved_58_58 : 1;
		u64 peb_st_inf_ram_dbe : 1;
		u64 pd_bank3_ram_dbe : 1;
		u64 reserved_54_55 : 2;
		u64 pd_bank0_ram_dbe : 1;
		u64 pd_var_bank_ram_dbe : 1;
		u64 tx_fifo_crc_ram_dbe : 1;
		u64 tx_fifo_hdr_ram_dbe : 1;
		u64 tx_fifo_pkt_ram_dbe : 1;
		u64 add_work_fifo_dbe : 1;
		u64 send_mem_fifo_dbe : 1;
		u64 send_mem_stdn_fifo_dbe : 1;
		u64 send_mem_ts_fifo_dbe : 1;
		u64 nxt_link_ptr_ram_dbe : 1;
		u64 pkt_mrk_ram_dbe : 1;
		u64 ts_addwork_ram_dbe : 1;
		u64 state_mem0_dbe : 1;
		u64 reserved_39_40 : 2;
		u64 state_mem3_dbe : 1;
		u64 reserved_0_37 : 38;
	} cn73xx;
	struct cvmx_pko_peb_ecc_dbe_sts0_cn78xx {
		u64 iobp1_uid_fifo_ram_dbe : 1;
		u64 iobp0_fifo_ram_dbe : 1;
		u64 iobp1_fifo_ram_dbe : 1;
		u64 pdm_resp_buf_ram_dbe : 1;
		u64 pdm_pse_buf_ram_dbe : 1;
		u64 reserved_58_58 : 1;
		u64 peb_st_inf_ram_dbe : 1;
		u64 pd_bank3_ram_dbe : 1;
		u64 reserved_54_55 : 2;
		u64 pd_bank0_ram_dbe : 1;
		u64 pd_var_bank_ram_dbe : 1;
		u64 tx_fifo_crc_ram_dbe : 1;
		u64 tx_fifo_hdr_ram_dbe : 1;
		u64 tx_fifo_pkt_ram_dbe : 1;
		u64 add_work_fifo_dbe : 1;
		u64 send_mem_fifo_dbe : 1;
		u64 send_mem_stdn_fifo_dbe : 1;
		u64 send_mem_ts_fifo_dbe : 1;
		u64 nxt_link_ptr_ram_dbe : 1;
		u64 pkt_mrk_ram_dbe : 1;
		u64 ts_addwork_ram_dbe : 1;
		u64 reserved_0_41 : 42;
	} cn78xx;
	struct cvmx_pko_peb_ecc_dbe_sts0_cn78xxp1 {
		u64 iobp1_uid_fifo_ram_dbe : 1;
		u64 iobp0_fifo_ram_dbe : 1;
		u64 iobp1_fifo_ram_dbe : 1;
		u64 pdm_resp_buf_ram_dbe : 1;
		u64 pdm_pse_buf_ram_dbe : 1;
		u64 peb_sm_jmp_ram_dbe : 1;
		u64 peb_st_inf_ram_dbe : 1;
		u64 pd_bank3_ram_dbe : 1;
		u64 pd_bank2_ram_dbe : 1;
		u64 pd_bank1_ram_dbe : 1;
		u64 pd_bank0_ram_dbe : 1;
		u64 pd_var_bank_ram_dbe : 1;
		u64 tx_fifo_crc_ram_dbe : 1;
		u64 tx_fifo_hdr_ram_dbe : 1;
		u64 tx_fifo_pkt_ram_dbe : 1;
		u64 add_work_fifo_dbe : 1;
		u64 send_mem_fifo_dbe : 1;
		u64 send_mem_stdn_fifo_dbe : 1;
		u64 send_mem_ts_fifo_dbe : 1;
		u64 nxt_link_ptr_ram_dbe : 1;
		u64 pkt_mrk_ram_dbe : 1;
		u64 ts_addwork_ram_dbe : 1;
		u64 reserved_0_41 : 42;
	} cn78xxp1;
	struct cvmx_pko_peb_ecc_dbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_peb_ecc_dbe_sts0 cvmx_pko_peb_ecc_dbe_sts0_t;

/**
 * cvmx_pko_peb_ecc_dbe_sts_cmb0
 */
union cvmx_pko_peb_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s {
		u64 peb_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_peb_ecc_dbe_sts_cmb0 cvmx_pko_peb_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_peb_ecc_sbe_sts0
 */
union cvmx_pko_peb_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_peb_ecc_sbe_sts0_s {
		u64 iobp1_uid_fifo_ram_sbe : 1;
		u64 iobp0_fifo_ram_sbe : 1;
		u64 iobp1_fifo_ram_sbe : 1;
		u64 pdm_resp_buf_ram_sbe : 1;
		u64 pdm_pse_buf_ram_sbe : 1;
		u64 peb_sm_jmp_ram_sbe : 1;
		u64 peb_st_inf_ram_sbe : 1;
		u64 pd_bank3_ram_sbe : 1;
		u64 pd_bank2_ram_sbe : 1;
		u64 pd_bank1_ram_sbe : 1;
		u64 pd_bank0_ram_sbe : 1;
		u64 pd_var_bank_ram_sbe : 1;
		u64 tx_fifo_crc_ram_sbe : 1;
		u64 tx_fifo_hdr_ram_sbe : 1;
		u64 tx_fifo_pkt_ram_sbe : 1;
		u64 add_work_fifo_sbe : 1;
		u64 send_mem_fifo_sbe : 1;
		u64 send_mem_stdn_fifo_sbe : 1;
		u64 send_mem_ts_fifo_sbe : 1;
		u64 nxt_link_ptr_ram_sbe : 1;
		u64 pkt_mrk_ram_sbe : 1;
		u64 ts_addwork_ram_sbe : 1;
		u64 state_mem0_sbe : 1;
		u64 reserved_39_40 : 2;
		u64 state_mem3_sbe : 1;
		u64 reserved_0_37 : 38;
	} s;
	struct cvmx_pko_peb_ecc_sbe_sts0_cn73xx {
		u64 iobp1_uid_fifo_ram_sbe : 1;
		u64 iobp0_fifo_ram_sbe : 1;
		u64 iobp1_fifo_ram_sbe : 1;
		u64 pdm_resp_buf_ram_sbe : 1;
		u64 pdm_pse_buf_ram_sbe : 1;
		u64 reserved_58_58 : 1;
		u64 peb_st_inf_ram_sbe : 1;
		u64 pd_bank3_ram_sbe : 1;
		u64 reserved_54_55 : 2;
		u64 pd_bank0_ram_sbe : 1;
		u64 pd_var_bank_ram_sbe : 1;
		u64 tx_fifo_crc_ram_sbe : 1;
		u64 tx_fifo_hdr_ram_sbe : 1;
		u64 tx_fifo_pkt_ram_sbe : 1;
		u64 add_work_fifo_sbe : 1;
		u64 send_mem_fifo_sbe : 1;
		u64 send_mem_stdn_fifo_sbe : 1;
		u64 send_mem_ts_fifo_sbe : 1;
		u64 nxt_link_ptr_ram_sbe : 1;
		u64 pkt_mrk_ram_sbe : 1;
		u64 ts_addwork_ram_sbe : 1;
		u64 state_mem0_sbe : 1;
		u64 reserved_39_40 : 2;
		u64 state_mem3_sbe : 1;
		u64 reserved_0_37 : 38;
	} cn73xx;
	struct cvmx_pko_peb_ecc_sbe_sts0_cn78xx {
		u64 iobp1_uid_fifo_ram_sbe : 1;
		u64 iobp0_fifo_ram_sbe : 1;
		u64 iobp1_fifo_ram_sbe : 1;
		u64 pdm_resp_buf_ram_sbe : 1;
		u64 pdm_pse_buf_ram_sbe : 1;
		u64 reserved_58_58 : 1;
		u64 peb_st_inf_ram_sbe : 1;
		u64 pd_bank3_ram_sbe : 1;
		u64 reserved_54_55 : 2;
		u64 pd_bank0_ram_sbe : 1;
		u64 pd_var_bank_ram_sbe : 1;
		u64 tx_fifo_crc_ram_sbe : 1;
		u64 tx_fifo_hdr_ram_sbe : 1;
		u64 tx_fifo_pkt_ram_sbe : 1;
		u64 add_work_fifo_sbe : 1;
		u64 send_mem_fifo_sbe : 1;
		u64 send_mem_stdn_fifo_sbe : 1;
		u64 send_mem_ts_fifo_sbe : 1;
		u64 nxt_link_ptr_ram_sbe : 1;
		u64 pkt_mrk_ram_sbe : 1;
		u64 ts_addwork_ram_sbe : 1;
		u64 reserved_0_41 : 42;
	} cn78xx;
	struct cvmx_pko_peb_ecc_sbe_sts0_cn78xxp1 {
		u64 iobp1_uid_fifo_ram_sbe : 1;
		u64 iobp0_fifo_ram_sbe : 1;
		u64 iobp1_fifo_ram_sbe : 1;
		u64 pdm_resp_buf_ram_sbe : 1;
		u64 pdm_pse_buf_ram_sbe : 1;
		u64 peb_sm_jmp_ram_sbe : 1;
		u64 peb_st_inf_ram_sbe : 1;
		u64 pd_bank3_ram_sbe : 1;
		u64 pd_bank2_ram_sbe : 1;
		u64 pd_bank1_ram_sbe : 1;
		u64 pd_bank0_ram_sbe : 1;
		u64 pd_var_bank_ram_sbe : 1;
		u64 tx_fifo_crc_ram_sbe : 1;
		u64 tx_fifo_hdr_ram_sbe : 1;
		u64 tx_fifo_pkt_ram_sbe : 1;
		u64 add_work_fifo_sbe : 1;
		u64 send_mem_fifo_sbe : 1;
		u64 send_mem_stdn_fifo_sbe : 1;
		u64 send_mem_ts_fifo_sbe : 1;
		u64 nxt_link_ptr_ram_sbe : 1;
		u64 pkt_mrk_ram_sbe : 1;
		u64 ts_addwork_ram_sbe : 1;
		u64 reserved_0_41 : 42;
	} cn78xxp1;
	struct cvmx_pko_peb_ecc_sbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_peb_ecc_sbe_sts0 cvmx_pko_peb_ecc_sbe_sts0_t;

/**
 * cvmx_pko_peb_ecc_sbe_sts_cmb0
 */
union cvmx_pko_peb_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s {
		u64 peb_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_peb_ecc_sbe_sts_cmb0 cvmx_pko_peb_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_peb_eco
 */
union cvmx_pko_peb_eco {
	u64 u64;
	struct cvmx_pko_peb_eco_s {
		u64 reserved_32_63 : 32;
		u64 eco_rw : 32;
	} s;
	struct cvmx_pko_peb_eco_s cn73xx;
	struct cvmx_pko_peb_eco_s cn78xx;
	struct cvmx_pko_peb_eco_s cnf75xx;
};

typedef union cvmx_pko_peb_eco cvmx_pko_peb_eco_t;

/**
 * cvmx_pko_peb_err_int
 */
union cvmx_pko_peb_err_int {
	u64 u64;
	struct cvmx_pko_peb_err_int_s {
		u64 reserved_10_63 : 54;
		u64 peb_macx_cfg_wr_err : 1;
		u64 peb_max_link_err : 1;
		u64 peb_subd_size_err : 1;
		u64 peb_subd_addr_err : 1;
		u64 peb_trunc_err : 1;
		u64 peb_pad_err : 1;
		u64 peb_pse_fifo_err : 1;
		u64 peb_fcs_sop_err : 1;
		u64 peb_jump_def_err : 1;
		u64 peb_ext_hdr_def_err : 1;
	} s;
	struct cvmx_pko_peb_err_int_s cn73xx;
	struct cvmx_pko_peb_err_int_s cn78xx;
	struct cvmx_pko_peb_err_int_s cn78xxp1;
	struct cvmx_pko_peb_err_int_s cnf75xx;
};

typedef union cvmx_pko_peb_err_int cvmx_pko_peb_err_int_t;

/**
 * cvmx_pko_peb_ext_hdr_def_err_info
 */
union cvmx_pko_peb_ext_hdr_def_err_info {
	u64 u64;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s cn73xx;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s cn78xx;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s cn78xxp1;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_ext_hdr_def_err_info cvmx_pko_peb_ext_hdr_def_err_info_t;

/**
 * cvmx_pko_peb_fcs_sop_err_info
 */
union cvmx_pko_peb_fcs_sop_err_info {
	u64 u64;
	struct cvmx_pko_peb_fcs_sop_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_fcs_sop_err_info_s cn73xx;
	struct cvmx_pko_peb_fcs_sop_err_info_s cn78xx;
	struct cvmx_pko_peb_fcs_sop_err_info_s cn78xxp1;
	struct cvmx_pko_peb_fcs_sop_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_fcs_sop_err_info cvmx_pko_peb_fcs_sop_err_info_t;

/**
 * cvmx_pko_peb_jump_def_err_info
 */
union cvmx_pko_peb_jump_def_err_info {
	u64 u64;
	struct cvmx_pko_peb_jump_def_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_jump_def_err_info_s cn73xx;
	struct cvmx_pko_peb_jump_def_err_info_s cn78xx;
	struct cvmx_pko_peb_jump_def_err_info_s cn78xxp1;
	struct cvmx_pko_peb_jump_def_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_jump_def_err_info cvmx_pko_peb_jump_def_err_info_t;

/**
 * cvmx_pko_peb_macx_cfg_wr_err_info
 */
union cvmx_pko_peb_macx_cfg_wr_err_info {
	u64 u64;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s {
		u64 reserved_8_63 : 56;
		u64 val : 1;
		u64 mac : 7;
	} s;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s cn73xx;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s cn78xx;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s cn78xxp1;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_macx_cfg_wr_err_info cvmx_pko_peb_macx_cfg_wr_err_info_t;

/**
 * cvmx_pko_peb_max_link_err_info
 */
union cvmx_pko_peb_max_link_err_info {
	u64 u64;
	struct cvmx_pko_peb_max_link_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_max_link_err_info_s cn73xx;
	struct cvmx_pko_peb_max_link_err_info_s cn78xx;
	struct cvmx_pko_peb_max_link_err_info_s cn78xxp1;
	struct cvmx_pko_peb_max_link_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_max_link_err_info cvmx_pko_peb_max_link_err_info_t;

/**
 * cvmx_pko_peb_ncb_cfg
 */
union cvmx_pko_peb_ncb_cfg {
	u64 u64;
	struct cvmx_pko_peb_ncb_cfg_s {
		u64 reserved_1_63 : 63;
		u64 rstp : 1;
	} s;
	struct cvmx_pko_peb_ncb_cfg_s cn73xx;
	struct cvmx_pko_peb_ncb_cfg_s cn78xx;
	struct cvmx_pko_peb_ncb_cfg_s cn78xxp1;
	struct cvmx_pko_peb_ncb_cfg_s cnf75xx;
};

typedef union cvmx_pko_peb_ncb_cfg cvmx_pko_peb_ncb_cfg_t;

/**
 * cvmx_pko_peb_pad_err_info
 */
union cvmx_pko_peb_pad_err_info {
	u64 u64;
	struct cvmx_pko_peb_pad_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_pad_err_info_s cn73xx;
	struct cvmx_pko_peb_pad_err_info_s cn78xx;
	struct cvmx_pko_peb_pad_err_info_s cn78xxp1;
	struct cvmx_pko_peb_pad_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_pad_err_info cvmx_pko_peb_pad_err_info_t;

/**
 * cvmx_pko_peb_pse_fifo_err_info
 */
union cvmx_pko_peb_pse_fifo_err_info {
	u64 u64;
	struct cvmx_pko_peb_pse_fifo_err_info_s {
		u64 reserved_25_63 : 39;
		u64 link : 5;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_pse_fifo_err_info_cn73xx {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} cn73xx;
	struct cvmx_pko_peb_pse_fifo_err_info_s cn78xx;
	struct cvmx_pko_peb_pse_fifo_err_info_cn73xx cn78xxp1;
	struct cvmx_pko_peb_pse_fifo_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_pse_fifo_err_info cvmx_pko_peb_pse_fifo_err_info_t;

/**
 * cvmx_pko_peb_subd_addr_err_info
 */
union cvmx_pko_peb_subd_addr_err_info {
	u64 u64;
	struct cvmx_pko_peb_subd_addr_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_subd_addr_err_info_s cn73xx;
	struct cvmx_pko_peb_subd_addr_err_info_s cn78xx;
	struct cvmx_pko_peb_subd_addr_err_info_s cn78xxp1;
	struct cvmx_pko_peb_subd_addr_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_subd_addr_err_info cvmx_pko_peb_subd_addr_err_info_t;

/**
 * cvmx_pko_peb_subd_size_err_info
 */
union cvmx_pko_peb_subd_size_err_info {
	u64 u64;
	struct cvmx_pko_peb_subd_size_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_subd_size_err_info_s cn73xx;
	struct cvmx_pko_peb_subd_size_err_info_s cn78xx;
	struct cvmx_pko_peb_subd_size_err_info_s cn78xxp1;
	struct cvmx_pko_peb_subd_size_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_subd_size_err_info cvmx_pko_peb_subd_size_err_info_t;

/**
 * cvmx_pko_peb_trunc_err_info
 */
union cvmx_pko_peb_trunc_err_info {
	u64 u64;
	struct cvmx_pko_peb_trunc_err_info_s {
		u64 reserved_20_63 : 44;
		u64 val : 1;
		u64 fifo : 7;
		u64 chan : 12;
	} s;
	struct cvmx_pko_peb_trunc_err_info_s cn73xx;
	struct cvmx_pko_peb_trunc_err_info_s cn78xx;
	struct cvmx_pko_peb_trunc_err_info_s cn78xxp1;
	struct cvmx_pko_peb_trunc_err_info_s cnf75xx;
};

typedef union cvmx_pko_peb_trunc_err_info cvmx_pko_peb_trunc_err_info_t;

/**
 * cvmx_pko_peb_tso_cfg
 */
union cvmx_pko_peb_tso_cfg {
	u64 u64;
	struct cvmx_pko_peb_tso_cfg_s {
		u64 reserved_44_63 : 20;
		u64 fsf : 12;
		u64 reserved_28_31 : 4;
		u64 msf : 12;
		u64 reserved_12_15 : 4;
		u64 lsf : 12;
	} s;
	struct cvmx_pko_peb_tso_cfg_s cn73xx;
	struct cvmx_pko_peb_tso_cfg_s cn78xx;
	struct cvmx_pko_peb_tso_cfg_s cn78xxp1;
	struct cvmx_pko_peb_tso_cfg_s cnf75xx;
};

typedef union cvmx_pko_peb_tso_cfg cvmx_pko_peb_tso_cfg_t;

/**
 * cvmx_pko_pq_csr_bus_debug
 */
union cvmx_pko_pq_csr_bus_debug {
	u64 u64;
	struct cvmx_pko_pq_csr_bus_debug_s {
		u64 csr_bus_debug : 64;
	} s;
	struct cvmx_pko_pq_csr_bus_debug_s cn73xx;
	struct cvmx_pko_pq_csr_bus_debug_s cn78xx;
	struct cvmx_pko_pq_csr_bus_debug_s cn78xxp1;
	struct cvmx_pko_pq_csr_bus_debug_s cnf75xx;
};

typedef union cvmx_pko_pq_csr_bus_debug cvmx_pko_pq_csr_bus_debug_t;

/**
 * cvmx_pko_pq_debug_green
 */
union cvmx_pko_pq_debug_green {
	u64 u64;
	struct cvmx_pko_pq_debug_green_s {
		u64 g_valid : 32;
		u64 cred_ok_n : 32;
	} s;
	struct cvmx_pko_pq_debug_green_s cn73xx;
	struct cvmx_pko_pq_debug_green_s cn78xx;
	struct cvmx_pko_pq_debug_green_s cn78xxp1;
	struct cvmx_pko_pq_debug_green_s cnf75xx;
};

typedef union cvmx_pko_pq_debug_green cvmx_pko_pq_debug_green_t;

/**
 * cvmx_pko_pq_debug_links
 */
union cvmx_pko_pq_debug_links {
	u64 u64;
	struct cvmx_pko_pq_debug_links_s {
		u64 links_ready : 32;
		u64 peb_lnk_rdy_ir : 32;
	} s;
	struct cvmx_pko_pq_debug_links_s cn73xx;
	struct cvmx_pko_pq_debug_links_s cn78xx;
	struct cvmx_pko_pq_debug_links_s cn78xxp1;
	struct cvmx_pko_pq_debug_links_s cnf75xx;
};

typedef union cvmx_pko_pq_debug_links cvmx_pko_pq_debug_links_t;

/**
 * cvmx_pko_pq_debug_yellow
 */
union cvmx_pko_pq_debug_yellow {
	u64 u64;
	struct cvmx_pko_pq_debug_yellow_s {
		u64 y_valid : 32;
		u64 reserved_28_31 : 4;
		u64 link_vv : 28;
	} s;
	struct cvmx_pko_pq_debug_yellow_s cn73xx;
	struct cvmx_pko_pq_debug_yellow_s cn78xx;
	struct cvmx_pko_pq_debug_yellow_s cn78xxp1;
	struct cvmx_pko_pq_debug_yellow_s cnf75xx;
};

typedef union cvmx_pko_pq_debug_yellow cvmx_pko_pq_debug_yellow_t;

/**
 * cvmx_pko_pqa_debug
 */
union cvmx_pko_pqa_debug {
	u64 u64;
	struct cvmx_pko_pqa_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_pqa_debug_s cn73xx;
	struct cvmx_pko_pqa_debug_s cn78xx;
	struct cvmx_pko_pqa_debug_s cn78xxp1;
	struct cvmx_pko_pqa_debug_s cnf75xx;
};

typedef union cvmx_pko_pqa_debug cvmx_pko_pqa_debug_t;

/**
 * cvmx_pko_pqb_debug
 *
 * This register has the same bit fields as PKO_PQA_DEBUG.
 *
 */
union cvmx_pko_pqb_debug {
	u64 u64;
	struct cvmx_pko_pqb_debug_s {
		u64 dbg_vec : 64;
	} s;
	struct cvmx_pko_pqb_debug_s cn73xx;
	struct cvmx_pko_pqb_debug_s cn78xx;
	struct cvmx_pko_pqb_debug_s cn78xxp1;
	struct cvmx_pko_pqb_debug_s cnf75xx;
};

typedef union cvmx_pko_pqb_debug cvmx_pko_pqb_debug_t;

/**
 * cvmx_pko_pse_dq_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_dq_bist_status {
	u64 u64;
	struct cvmx_pko_pse_dq_bist_status_s {
		u64 reserved_8_63 : 56;
		u64 rt7_sram : 1;
		u64 rt6_sram : 1;
		u64 rt5_sram : 1;
		u64 reserved_4_4 : 1;
		u64 rt3_sram : 1;
		u64 rt2_sram : 1;
		u64 rt1_sram : 1;
		u64 rt0_sram : 1;
	} s;
	struct cvmx_pko_pse_dq_bist_status_cn73xx {
		u64 reserved_5_63 : 59;
		u64 wt_sram : 1;
		u64 reserved_2_3 : 2;
		u64 rt1_sram : 1;
		u64 rt0_sram : 1;
	} cn73xx;
	struct cvmx_pko_pse_dq_bist_status_cn78xx {
		u64 reserved_9_63 : 55;
		u64 wt_sram : 1;
		u64 rt7_sram : 1;
		u64 rt6_sram : 1;
		u64 rt5_sram : 1;
		u64 rt4_sram : 1;
		u64 rt3_sram : 1;
		u64 rt2_sram : 1;
		u64 rt1_sram : 1;
		u64 rt0_sram : 1;
	} cn78xx;
	struct cvmx_pko_pse_dq_bist_status_cn78xx cn78xxp1;
	struct cvmx_pko_pse_dq_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_dq_bist_status cvmx_pko_pse_dq_bist_status_t;

/**
 * cvmx_pko_pse_dq_ecc_ctl0
 */
union cvmx_pko_pse_dq_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_dq_ecc_ctl0_s {
		u64 dq_wt_ram_flip : 2;
		u64 dq_wt_ram_cdis : 1;
		u64 dq_rt7_flip : 2;
		u64 dq_rt7_cdis : 1;
		u64 dq_rt6_flip : 2;
		u64 dq_rt6_cdis : 1;
		u64 dq_rt5_flip : 2;
		u64 dq_rt5_cdis : 1;
		u64 dq_rt4_flip : 2;
		u64 dq_rt4_cdis : 1;
		u64 dq_rt3_flip : 2;
		u64 dq_rt3_cdis : 1;
		u64 dq_rt2_flip : 2;
		u64 dq_rt2_cdis : 1;
		u64 dq_rt1_flip : 2;
		u64 dq_rt1_cdis : 1;
		u64 dq_rt0_flip : 2;
		u64 dq_rt0_cdis : 1;
		u64 reserved_0_36 : 37;
	} s;
	struct cvmx_pko_pse_dq_ecc_ctl0_cn73xx {
		u64 dq_wt_ram_flip : 2;
		u64 dq_wt_ram_cdis : 1;
		u64 reserved_43_60 : 18;
		u64 dq_rt1_flip : 2;
		u64 dq_rt1_cdis : 1;
		u64 dq_rt0_flip : 2;
		u64 dq_rt0_cdis : 1;
		u64 reserved_0_36 : 37;
	} cn73xx;
	struct cvmx_pko_pse_dq_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_dq_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_pse_dq_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_dq_ecc_ctl0 cvmx_pko_pse_dq_ecc_ctl0_t;

/**
 * cvmx_pko_pse_dq_ecc_dbe_sts0
 */
union cvmx_pko_pse_dq_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_s {
		u64 dq_wt_ram_dbe : 1;
		u64 dq_rt7_dbe : 1;
		u64 dq_rt6_dbe : 1;
		u64 dq_rt5_dbe : 1;
		u64 dq_rt4_dbe : 1;
		u64 dq_rt3_dbe : 1;
		u64 dq_rt2_dbe : 1;
		u64 dq_rt1_dbe : 1;
		u64 dq_rt0_dbe : 1;
		u64 reserved_0_54 : 55;
	} s;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_cn73xx {
		u64 dq_wt_ram_dbe : 1;
		u64 reserved_57_62 : 6;
		u64 dq_rt1_dbe : 1;
		u64 dq_rt0_dbe : 1;
		u64 reserved_0_54 : 55;
	} cn73xx;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_dq_ecc_dbe_sts0 cvmx_pko_pse_dq_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_dq_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_dq_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s {
		u64 pse_dq_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_dq_ecc_dbe_sts_cmb0 cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_dq_ecc_sbe_sts0
 */
union cvmx_pko_pse_dq_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_s {
		u64 dq_wt_ram_sbe : 1;
		u64 dq_rt7_sbe : 1;
		u64 dq_rt6_sbe : 1;
		u64 dq_rt5_sbe : 1;
		u64 dq_rt4_sbe : 1;
		u64 dq_rt3_sbe : 1;
		u64 dq_rt2_sbe : 1;
		u64 dq_rt1_sbe : 1;
		u64 dq_rt0_sbe : 1;
		u64 reserved_0_54 : 55;
	} s;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_cn73xx {
		u64 dq_wt_ram_sbe : 1;
		u64 reserved_57_62 : 6;
		u64 dq_rt1_sbe : 1;
		u64 dq_rt0_sbe : 1;
		u64 reserved_0_54 : 55;
	} cn73xx;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_dq_ecc_sbe_sts0 cvmx_pko_pse_dq_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_dq_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_dq_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s {
		u64 pse_dq_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_dq_ecc_sbe_sts_cmb0 cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_pq_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_pq_bist_status {
	u64 u64;
	struct cvmx_pko_pse_pq_bist_status_s {
		u64 reserved_15_63 : 49;
		u64 tp_sram : 1;
		u64 irq_fifo_sram : 1;
		u64 wmd_sram : 1;
		u64 wms_sram : 1;
		u64 cxd_sram : 1;
		u64 dqd_sram : 1;
		u64 dqs_sram : 1;
		u64 pqd_sram : 1;
		u64 pqr_sram : 1;
		u64 pqy_sram : 1;
		u64 pqg_sram : 1;
		u64 std_sram : 1;
		u64 st_sram : 1;
		u64 reserved_1_1 : 1;
		u64 cxs_sram : 1;
	} s;
	struct cvmx_pko_pse_pq_bist_status_cn73xx {
		u64 reserved_15_63 : 49;
		u64 tp_sram : 1;
		u64 reserved_13_13 : 1;
		u64 wmd_sram : 1;
		u64 reserved_11_11 : 1;
		u64 cxd_sram : 1;
		u64 dqd_sram : 1;
		u64 dqs_sram : 1;
		u64 pqd_sram : 1;
		u64 pqr_sram : 1;
		u64 pqy_sram : 1;
		u64 pqg_sram : 1;
		u64 std_sram : 1;
		u64 st_sram : 1;
		u64 reserved_1_1 : 1;
		u64 cxs_sram : 1;
	} cn73xx;
	struct cvmx_pko_pse_pq_bist_status_s cn78xx;
	struct cvmx_pko_pse_pq_bist_status_s cn78xxp1;
	struct cvmx_pko_pse_pq_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_pq_bist_status cvmx_pko_pse_pq_bist_status_t;

/**
 * cvmx_pko_pse_pq_ecc_ctl0
 */
union cvmx_pko_pse_pq_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_pq_ecc_ctl0_s {
		u64 pq_cxs_ram_flip : 2;
		u64 pq_cxs_ram_cdis : 1;
		u64 pq_cxd_ram_flip : 2;
		u64 pq_cxd_ram_cdis : 1;
		u64 irq_fifo_sram_flip : 2;
		u64 irq_fifo_sram_cdis : 1;
		u64 tp_sram_flip : 2;
		u64 tp_sram_cdis : 1;
		u64 pq_std_ram_flip : 2;
		u64 pq_std_ram_cdis : 1;
		u64 pq_st_ram_flip : 2;
		u64 pq_st_ram_cdis : 1;
		u64 pq_wmd_ram_flip : 2;
		u64 pq_wmd_ram_cdis : 1;
		u64 pq_wms_ram_flip : 2;
		u64 pq_wms_ram_cdis : 1;
		u64 reserved_0_39 : 40;
	} s;
	struct cvmx_pko_pse_pq_ecc_ctl0_cn73xx {
		u64 pq_cxs_ram_flip : 2;
		u64 pq_cxs_ram_cdis : 1;
		u64 pq_cxd_ram_flip : 2;
		u64 pq_cxd_ram_cdis : 1;
		u64 reserved_55_57 : 3;
		u64 tp_sram_flip : 2;
		u64 tp_sram_cdis : 1;
		u64 pq_std_ram_flip : 2;
		u64 pq_std_ram_cdis : 1;
		u64 pq_st_ram_flip : 2;
		u64 pq_st_ram_cdis : 1;
		u64 pq_wmd_ram_flip : 2;
		u64 pq_wmd_ram_cdis : 1;
		u64 reserved_0_42 : 43;
	} cn73xx;
	struct cvmx_pko_pse_pq_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_pq_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_pse_pq_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_pq_ecc_ctl0 cvmx_pko_pse_pq_ecc_ctl0_t;

/**
 * cvmx_pko_pse_pq_ecc_dbe_sts0
 */
union cvmx_pko_pse_pq_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_s {
		u64 pq_cxs_ram_dbe : 1;
		u64 pq_cxd_ram_dbe : 1;
		u64 irq_fifo_sram_dbe : 1;
		u64 tp_sram_dbe : 1;
		u64 pq_std_ram_dbe : 1;
		u64 pq_st_ram_dbe : 1;
		u64 pq_wmd_ram_dbe : 1;
		u64 pq_wms_ram_dbe : 1;
		u64 reserved_0_55 : 56;
	} s;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_cn73xx {
		u64 pq_cxs_ram_dbe : 1;
		u64 pq_cxd_ram_dbe : 1;
		u64 reserved_61_61 : 1;
		u64 tp_sram_dbe : 1;
		u64 pq_std_ram_dbe : 1;
		u64 pq_st_ram_dbe : 1;
		u64 pq_wmd_ram_dbe : 1;
		u64 reserved_0_56 : 57;
	} cn73xx;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_pq_ecc_dbe_sts0 cvmx_pko_pse_pq_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_pq_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_pq_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s {
		u64 pse_pq_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_pq_ecc_dbe_sts_cmb0 cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_pq_ecc_sbe_sts0
 */
union cvmx_pko_pse_pq_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_s {
		u64 pq_cxs_ram_sbe : 1;
		u64 pq_cxd_ram_sbe : 1;
		u64 irq_fifo_sram_sbe : 1;
		u64 tp_sram_sbe : 1;
		u64 pq_std_ram_sbe : 1;
		u64 pq_st_ram_sbe : 1;
		u64 pq_wmd_ram_sbe : 1;
		u64 pq_wms_ram_sbe : 1;
		u64 reserved_0_55 : 56;
	} s;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_cn73xx {
		u64 pq_cxs_ram_sbe : 1;
		u64 pq_cxd_ram_sbe : 1;
		u64 reserved_61_61 : 1;
		u64 tp_sram_sbe : 1;
		u64 pq_std_ram_sbe : 1;
		u64 pq_st_ram_sbe : 1;
		u64 pq_wmd_ram_sbe : 1;
		u64 reserved_0_56 : 57;
	} cn73xx;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_pq_ecc_sbe_sts0 cvmx_pko_pse_pq_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_pq_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_pq_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s {
		u64 pse_pq_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_pq_ecc_sbe_sts_cmb0 cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq1_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq1_bist_status {
	u64 u64;
	struct cvmx_pko_pse_sq1_bist_status_s {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 pc_sram : 1;
		u64 xon_sram : 1;
		u64 cc_sram : 1;
		u64 vc1_sram : 1;
		u64 vc0_sram : 1;
		u64 reserved_21_22 : 2;
		u64 tp1_sram : 1;
		u64 tp0_sram : 1;
		u64 xo_sram : 1;
		u64 rt_sram : 1;
		u64 reserved_9_16 : 8;
		u64 tw1_cmd_fifo : 1;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 cxd_sram : 1;
		u64 cxs_sram : 1;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} s;
	struct cvmx_pko_pse_sq1_bist_status_cn73xx {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 pc_sram : 1;
		u64 xon_sram : 1;
		u64 cc_sram : 1;
		u64 vc1_sram : 1;
		u64 vc0_sram : 1;
		u64 reserved_20_22 : 3;
		u64 tp0_sram : 1;
		u64 xo_sram : 1;
		u64 rt_sram : 1;
		u64 reserved_9_16 : 8;
		u64 tw1_cmd_fifo : 1;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 cxd_sram : 1;
		u64 cxs_sram : 1;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} cn73xx;
	struct cvmx_pko_pse_sq1_bist_status_s cn78xx;
	struct cvmx_pko_pse_sq1_bist_status_s cn78xxp1;
	struct cvmx_pko_pse_sq1_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq1_bist_status cvmx_pko_pse_sq1_bist_status_t;

/**
 * cvmx_pko_pse_sq1_ecc_ctl0
 */
union cvmx_pko_pse_sq1_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_sq1_ecc_ctl0_s {
		u64 cxs_ram_flip : 2;
		u64 cxs_ram_cdis : 1;
		u64 cxd_ram_flip : 2;
		u64 cxd_ram_cdis : 1;
		u64 vc1_sram_flip : 2;
		u64 vc1_sram_cdis : 1;
		u64 vc0_sram_flip : 2;
		u64 vc0_sram_cdis : 1;
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 pc_ram_flip : 2;
		u64 pc_ram_cdis : 1;
		u64 tw1_cmd_fifo_ram_flip : 2;
		u64 tw1_cmd_fifo_ram_cdis : 1;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 tp1_sram_flip : 2;
		u64 tp1_sram_cdis : 1;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 sts1_ram_flip : 2;
		u64 sts1_ram_cdis : 1;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 std1_ram_flip : 2;
		u64 std1_ram_cdis : 1;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_9 : 10;
	} s;
	struct cvmx_pko_pse_sq1_ecc_ctl0_cn73xx {
		u64 cxs_ram_flip : 2;
		u64 cxs_ram_cdis : 1;
		u64 cxd_ram_flip : 2;
		u64 cxd_ram_cdis : 1;
		u64 reserved_55_57 : 3;
		u64 vc0_sram_flip : 2;
		u64 vc0_sram_cdis : 1;
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 pc_ram_flip : 2;
		u64 pc_ram_cdis : 1;
		u64 reserved_37_39 : 3;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 reserved_31_33 : 3;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 reserved_25_27 : 3;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 reserved_19_21 : 3;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_9 : 10;
	} cn73xx;
	struct cvmx_pko_pse_sq1_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_sq1_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_pse_sq1_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq1_ecc_ctl0 cvmx_pko_pse_sq1_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq1_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq1_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_s {
		u64 cxs_ram_dbe : 1;
		u64 cxd_ram_dbe : 1;
		u64 vc1_sram_dbe : 1;
		u64 vc0_sram_dbe : 1;
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 pc_ram_dbe : 1;
		u64 tw1_cmd_fifo_ram_dbe : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 tp1_sram_dbe : 1;
		u64 tp0_sram_dbe : 1;
		u64 sts1_ram_dbe : 1;
		u64 sts0_ram_dbe : 1;
		u64 std1_ram_dbe : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_45 : 46;
	} s;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_cn73xx {
		u64 cxs_ram_dbe : 1;
		u64 cxd_ram_dbe : 1;
		u64 reserved_61_61 : 1;
		u64 vc0_sram_dbe : 1;
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 pc_ram_dbe : 1;
		u64 reserved_55_55 : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 reserved_53_53 : 1;
		u64 tp0_sram_dbe : 1;
		u64 reserved_51_51 : 1;
		u64 sts0_ram_dbe : 1;
		u64 reserved_49_49 : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_45 : 46;
	} cn73xx;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq1_ecc_dbe_sts0 cvmx_pko_pse_sq1_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s {
		u64 pse_sq1_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq1_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq1_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_s {
		u64 cxs_ram_sbe : 1;
		u64 cxd_ram_sbe : 1;
		u64 vc1_sram_sbe : 1;
		u64 vc0_sram_sbe : 1;
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 pc_ram_sbe : 1;
		u64 tw1_cmd_fifo_ram_sbe : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 tp1_sram_sbe : 1;
		u64 tp0_sram_sbe : 1;
		u64 sts1_ram_sbe : 1;
		u64 sts0_ram_sbe : 1;
		u64 std1_ram_sbe : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_45 : 46;
	} s;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_cn73xx {
		u64 cxs_ram_sbe : 1;
		u64 cxd_ram_sbe : 1;
		u64 reserved_61_61 : 1;
		u64 vc0_sram_sbe : 1;
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 pc_ram_sbe : 1;
		u64 reserved_55_55 : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 reserved_53_53 : 1;
		u64 tp0_sram_sbe : 1;
		u64 reserved_51_51 : 1;
		u64 sts0_ram_sbe : 1;
		u64 reserved_49_49 : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_45 : 46;
	} cn73xx;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq1_ecc_sbe_sts0 cvmx_pko_pse_sq1_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s {
		u64 pse_sq1_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq2_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq2_bist_status {
	u64 u64;
	struct cvmx_pko_pse_sq2_bist_status_s {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 reserved_21_27 : 7;
		u64 tp1_sram : 1;
		u64 tp0_sram : 1;
		u64 reserved_18_18 : 1;
		u64 rt_sram : 1;
		u64 reserved_9_16 : 8;
		u64 tw1_cmd_fifo : 1;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 reserved_3_4 : 2;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} s;
	struct cvmx_pko_pse_sq2_bist_status_cn73xx {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 reserved_20_27 : 8;
		u64 tp0_sram : 1;
		u64 reserved_18_18 : 1;
		u64 rt_sram : 1;
		u64 reserved_8_16 : 9;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 reserved_3_4 : 2;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} cn73xx;
	struct cvmx_pko_pse_sq2_bist_status_s cn78xx;
	struct cvmx_pko_pse_sq2_bist_status_s cn78xxp1;
	struct cvmx_pko_pse_sq2_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq2_bist_status cvmx_pko_pse_sq2_bist_status_t;

/**
 * cvmx_pko_pse_sq2_ecc_ctl0
 */
union cvmx_pko_pse_sq2_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_sq2_ecc_ctl0_s {
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 tw1_cmd_fifo_ram_flip : 2;
		u64 tw1_cmd_fifo_ram_cdis : 1;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 tp1_sram_flip : 2;
		u64 tp1_sram_cdis : 1;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 sts1_ram_flip : 2;
		u64 sts1_ram_cdis : 1;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 std1_ram_flip : 2;
		u64 std1_ram_cdis : 1;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_24 : 25;
	} s;
	struct cvmx_pko_pse_sq2_ecc_ctl0_cn73xx {
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 reserved_52_54 : 3;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 reserved_46_48 : 3;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 reserved_40_42 : 3;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 reserved_34_36 : 3;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_24 : 25;
	} cn73xx;
	struct cvmx_pko_pse_sq2_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_sq2_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_pse_sq2_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq2_ecc_ctl0 cvmx_pko_pse_sq2_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq2_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq2_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_s {
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 tw1_cmd_fifo_ram_dbe : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 tp1_sram_dbe : 1;
		u64 tp0_sram_dbe : 1;
		u64 sts1_ram_dbe : 1;
		u64 sts0_ram_dbe : 1;
		u64 std1_ram_dbe : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_50 : 51;
	} s;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_cn73xx {
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 reserved_60_60 : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 reserved_58_58 : 1;
		u64 tp0_sram_dbe : 1;
		u64 reserved_56_56 : 1;
		u64 sts0_ram_dbe : 1;
		u64 reserved_54_54 : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_50 : 51;
	} cn73xx;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq2_ecc_dbe_sts0 cvmx_pko_pse_sq2_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s {
		u64 pse_sq2_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq2_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq2_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_s {
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 tw1_cmd_fifo_ram_sbe : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 tp1_sram_sbe : 1;
		u64 tp0_sram_sbe : 1;
		u64 sts1_ram_sbe : 1;
		u64 sts0_ram_sbe : 1;
		u64 std1_ram_sbe : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_50 : 51;
	} s;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_cn73xx {
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 reserved_60_60 : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 reserved_58_58 : 1;
		u64 tp0_sram_sbe : 1;
		u64 reserved_56_56 : 1;
		u64 sts0_ram_sbe : 1;
		u64 reserved_54_54 : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_50 : 51;
	} cn73xx;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq2_ecc_sbe_sts0 cvmx_pko_pse_sq2_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s {
		u64 pse_sq2_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq3_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq3_bist_status {
	u64 u64;
	struct cvmx_pko_pse_sq3_bist_status_s {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 reserved_23_27 : 5;
		u64 tp3_sram : 1;
		u64 tp2_sram : 1;
		u64 tp1_sram : 1;
		u64 tp0_sram : 1;
		u64 reserved_18_18 : 1;
		u64 rt_sram : 1;
		u64 reserved_15_16 : 2;
		u64 tw3_cmd_fifo : 1;
		u64 reserved_12_13 : 2;
		u64 tw2_cmd_fifo : 1;
		u64 reserved_9_10 : 2;
		u64 tw1_cmd_fifo : 1;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 reserved_3_4 : 2;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} s;
	struct cvmx_pko_pse_sq3_bist_status_cn73xx {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 reserved_20_27 : 8;
		u64 tp0_sram : 1;
		u64 reserved_18_18 : 1;
		u64 rt_sram : 1;
		u64 reserved_8_16 : 9;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 reserved_3_4 : 2;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} cn73xx;
	struct cvmx_pko_pse_sq3_bist_status_s cn78xx;
	struct cvmx_pko_pse_sq3_bist_status_s cn78xxp1;
	struct cvmx_pko_pse_sq3_bist_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq3_bist_status cvmx_pko_pse_sq3_bist_status_t;

/**
 * cvmx_pko_pse_sq3_ecc_ctl0
 */
union cvmx_pko_pse_sq3_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_sq3_ecc_ctl0_s {
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 tw3_cmd_fifo_ram_flip : 2;
		u64 tw3_cmd_fifo_ram_cdis : 1;
		u64 tw2_cmd_fifo_ram_flip : 2;
		u64 tw2_cmd_fifo_ram_cdis : 1;
		u64 tw1_cmd_fifo_ram_flip : 2;
		u64 tw1_cmd_fifo_ram_cdis : 1;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 tp3_sram_flip : 2;
		u64 tp3_sram_cdis : 1;
		u64 tp2_sram_flip : 2;
		u64 tp2_sram_cdis : 1;
		u64 tp1_sram_flip : 2;
		u64 tp1_sram_cdis : 1;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 sts3_ram_flip : 2;
		u64 sts3_ram_cdis : 1;
		u64 sts2_ram_flip : 2;
		u64 sts2_ram_cdis : 1;
		u64 sts1_ram_flip : 2;
		u64 sts1_ram_cdis : 1;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 std3_ram_flip : 2;
		u64 std3_ram_cdis : 1;
		u64 std2_ram_flip : 2;
		u64 std2_ram_cdis : 1;
		u64 std1_ram_flip : 2;
		u64 std1_ram_cdis : 1;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_pse_sq3_ecc_ctl0_cn73xx {
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 reserved_46_54 : 9;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 reserved_34_42 : 9;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 reserved_22_30 : 9;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 reserved_10_18 : 9;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_0 : 1;
	} cn73xx;
	struct cvmx_pko_pse_sq3_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_sq3_ecc_ctl0_s cn78xxp1;
	struct cvmx_pko_pse_sq3_ecc_ctl0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq3_ecc_ctl0 cvmx_pko_pse_sq3_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq3_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq3_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_s {
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 tw3_cmd_fifo_ram_dbe : 1;
		u64 tw2_cmd_fifo_ram_dbe : 1;
		u64 tw1_cmd_fifo_ram_dbe : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 tp3_sram_dbe : 1;
		u64 tp2_sram_dbe : 1;
		u64 tp1_sram_dbe : 1;
		u64 tp0_sram_dbe : 1;
		u64 sts3_ram_dbe : 1;
		u64 sts2_ram_dbe : 1;
		u64 sts1_ram_dbe : 1;
		u64 sts0_ram_dbe : 1;
		u64 std3_ram_dbe : 1;
		u64 std2_ram_dbe : 1;
		u64 std1_ram_dbe : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_42 : 43;
	} s;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_cn73xx {
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 reserved_58_60 : 3;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 reserved_54_56 : 3;
		u64 tp0_sram_dbe : 1;
		u64 reserved_50_52 : 3;
		u64 sts0_ram_dbe : 1;
		u64 reserved_46_48 : 3;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_42 : 43;
	} cn73xx;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq3_ecc_dbe_sts0 cvmx_pko_pse_sq3_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s {
		u64 pse_sq3_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq3_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq3_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_s {
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 tw3_cmd_fifo_ram_sbe : 1;
		u64 tw2_cmd_fifo_ram_sbe : 1;
		u64 tw1_cmd_fifo_ram_sbe : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 tp3_sram_sbe : 1;
		u64 tp2_sram_sbe : 1;
		u64 tp1_sram_sbe : 1;
		u64 tp0_sram_sbe : 1;
		u64 sts3_ram_sbe : 1;
		u64 sts2_ram_sbe : 1;
		u64 sts1_ram_sbe : 1;
		u64 sts0_ram_sbe : 1;
		u64 std3_ram_sbe : 1;
		u64 std2_ram_sbe : 1;
		u64 std1_ram_sbe : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_42 : 43;
	} s;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_cn73xx {
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 reserved_58_60 : 3;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 reserved_54_56 : 3;
		u64 tp0_sram_sbe : 1;
		u64 reserved_50_52 : 3;
		u64 sts0_ram_sbe : 1;
		u64 reserved_46_48 : 3;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_42 : 43;
	} cn73xx;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_s cn78xxp1;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_cn73xx cnf75xx;
};

typedef union cvmx_pko_pse_sq3_ecc_sbe_sts0 cvmx_pko_pse_sq3_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s {
		u64 pse_sq3_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s cn73xx;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s cn78xxp1;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s cnf75xx;
};

typedef union cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq4_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq4_bist_status {
	u64 u64;
	struct cvmx_pko_pse_sq4_bist_status_s {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 reserved_23_27 : 5;
		u64 tp3_sram : 1;
		u64 tp2_sram : 1;
		u64 tp1_sram : 1;
		u64 tp0_sram : 1;
		u64 reserved_18_18 : 1;
		u64 rt_sram : 1;
		u64 reserved_15_16 : 2;
		u64 tw3_cmd_fifo : 1;
		u64 reserved_12_13 : 2;
		u64 tw2_cmd_fifo : 1;
		u64 reserved_9_10 : 2;
		u64 tw1_cmd_fifo : 1;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 reserved_3_4 : 2;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} s;
	struct cvmx_pko_pse_sq4_bist_status_s cn78xx;
	struct cvmx_pko_pse_sq4_bist_status_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq4_bist_status cvmx_pko_pse_sq4_bist_status_t;

/**
 * cvmx_pko_pse_sq4_ecc_ctl0
 */
union cvmx_pko_pse_sq4_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_sq4_ecc_ctl0_s {
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 tw3_cmd_fifo_ram_flip : 2;
		u64 tw3_cmd_fifo_ram_cdis : 1;
		u64 tw2_cmd_fifo_ram_flip : 2;
		u64 tw2_cmd_fifo_ram_cdis : 1;
		u64 tw1_cmd_fifo_ram_flip : 2;
		u64 tw1_cmd_fifo_ram_cdis : 1;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 tp3_sram_flip : 2;
		u64 tp3_sram_cdis : 1;
		u64 tp2_sram_flip : 2;
		u64 tp2_sram_cdis : 1;
		u64 tp1_sram_flip : 2;
		u64 tp1_sram_cdis : 1;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 sts3_ram_flip : 2;
		u64 sts3_ram_cdis : 1;
		u64 sts2_ram_flip : 2;
		u64 sts2_ram_cdis : 1;
		u64 sts1_ram_flip : 2;
		u64 sts1_ram_cdis : 1;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 std3_ram_flip : 2;
		u64 std3_ram_cdis : 1;
		u64 std2_ram_flip : 2;
		u64 std2_ram_cdis : 1;
		u64 std1_ram_flip : 2;
		u64 std1_ram_cdis : 1;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_pse_sq4_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_sq4_ecc_ctl0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq4_ecc_ctl0 cvmx_pko_pse_sq4_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq4_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq4_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts0_s {
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 tw3_cmd_fifo_ram_dbe : 1;
		u64 tw2_cmd_fifo_ram_dbe : 1;
		u64 tw1_cmd_fifo_ram_dbe : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 tp3_sram_dbe : 1;
		u64 tp2_sram_dbe : 1;
		u64 tp1_sram_dbe : 1;
		u64 tp0_sram_dbe : 1;
		u64 sts3_ram_dbe : 1;
		u64 sts2_ram_dbe : 1;
		u64 sts1_ram_dbe : 1;
		u64 sts0_ram_dbe : 1;
		u64 std3_ram_dbe : 1;
		u64 std2_ram_dbe : 1;
		u64 std1_ram_dbe : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_42 : 43;
	} s;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq4_ecc_dbe_sts0 cvmx_pko_pse_sq4_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_s {
		u64 pse_sq4_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq4_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq4_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts0_s {
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 tw3_cmd_fifo_ram_sbe : 1;
		u64 tw2_cmd_fifo_ram_sbe : 1;
		u64 tw1_cmd_fifo_ram_sbe : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 tp3_sram_sbe : 1;
		u64 tp2_sram_sbe : 1;
		u64 tp1_sram_sbe : 1;
		u64 tp0_sram_sbe : 1;
		u64 sts3_ram_sbe : 1;
		u64 sts2_ram_sbe : 1;
		u64 sts1_ram_sbe : 1;
		u64 sts0_ram_sbe : 1;
		u64 std3_ram_sbe : 1;
		u64 std2_ram_sbe : 1;
		u64 std1_ram_sbe : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_42 : 43;
	} s;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq4_ecc_sbe_sts0 cvmx_pko_pse_sq4_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_s {
		u64 pse_sq4_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq5_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq5_bist_status {
	u64 u64;
	struct cvmx_pko_pse_sq5_bist_status_s {
		u64 reserved_29_63 : 35;
		u64 sc_sram : 1;
		u64 reserved_23_27 : 5;
		u64 tp3_sram : 1;
		u64 tp2_sram : 1;
		u64 tp1_sram : 1;
		u64 tp0_sram : 1;
		u64 reserved_18_18 : 1;
		u64 rt_sram : 1;
		u64 reserved_15_16 : 2;
		u64 tw3_cmd_fifo : 1;
		u64 reserved_12_13 : 2;
		u64 tw2_cmd_fifo : 1;
		u64 reserved_9_10 : 2;
		u64 tw1_cmd_fifo : 1;
		u64 std_sram : 1;
		u64 sts_sram : 1;
		u64 tw0_cmd_fifo : 1;
		u64 reserved_3_4 : 2;
		u64 nt_sram : 1;
		u64 pt_sram : 1;
		u64 wt_sram : 1;
	} s;
	struct cvmx_pko_pse_sq5_bist_status_s cn78xx;
	struct cvmx_pko_pse_sq5_bist_status_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq5_bist_status cvmx_pko_pse_sq5_bist_status_t;

/**
 * cvmx_pko_pse_sq5_ecc_ctl0
 */
union cvmx_pko_pse_sq5_ecc_ctl0 {
	u64 u64;
	struct cvmx_pko_pse_sq5_ecc_ctl0_s {
		u64 sq_pt_ram_flip : 2;
		u64 sq_pt_ram_cdis : 1;
		u64 sq_nt_ram_flip : 2;
		u64 sq_nt_ram_cdis : 1;
		u64 rt_ram_flip : 2;
		u64 rt_ram_cdis : 1;
		u64 tw3_cmd_fifo_ram_flip : 2;
		u64 tw3_cmd_fifo_ram_cdis : 1;
		u64 tw2_cmd_fifo_ram_flip : 2;
		u64 tw2_cmd_fifo_ram_cdis : 1;
		u64 tw1_cmd_fifo_ram_flip : 2;
		u64 tw1_cmd_fifo_ram_cdis : 1;
		u64 tw0_cmd_fifo_ram_flip : 2;
		u64 tw0_cmd_fifo_ram_cdis : 1;
		u64 tp3_sram_flip : 2;
		u64 tp3_sram_cdis : 1;
		u64 tp2_sram_flip : 2;
		u64 tp2_sram_cdis : 1;
		u64 tp1_sram_flip : 2;
		u64 tp1_sram_cdis : 1;
		u64 tp0_sram_flip : 2;
		u64 tp0_sram_cdis : 1;
		u64 sts3_ram_flip : 2;
		u64 sts3_ram_cdis : 1;
		u64 sts2_ram_flip : 2;
		u64 sts2_ram_cdis : 1;
		u64 sts1_ram_flip : 2;
		u64 sts1_ram_cdis : 1;
		u64 sts0_ram_flip : 2;
		u64 sts0_ram_cdis : 1;
		u64 std3_ram_flip : 2;
		u64 std3_ram_cdis : 1;
		u64 std2_ram_flip : 2;
		u64 std2_ram_cdis : 1;
		u64 std1_ram_flip : 2;
		u64 std1_ram_cdis : 1;
		u64 std0_ram_flip : 2;
		u64 std0_ram_cdis : 1;
		u64 wt_ram_flip : 2;
		u64 wt_ram_cdis : 1;
		u64 sc_ram_flip : 2;
		u64 sc_ram_cdis : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pko_pse_sq5_ecc_ctl0_s cn78xx;
	struct cvmx_pko_pse_sq5_ecc_ctl0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq5_ecc_ctl0 cvmx_pko_pse_sq5_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq5_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq5_ecc_dbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts0_s {
		u64 sq_pt_ram_dbe : 1;
		u64 sq_nt_ram_dbe : 1;
		u64 rt_ram_dbe : 1;
		u64 tw3_cmd_fifo_ram_dbe : 1;
		u64 tw2_cmd_fifo_ram_dbe : 1;
		u64 tw1_cmd_fifo_ram_dbe : 1;
		u64 tw0_cmd_fifo_ram_dbe : 1;
		u64 tp3_sram_dbe : 1;
		u64 tp2_sram_dbe : 1;
		u64 tp1_sram_dbe : 1;
		u64 tp0_sram_dbe : 1;
		u64 sts3_ram_dbe : 1;
		u64 sts2_ram_dbe : 1;
		u64 sts1_ram_dbe : 1;
		u64 sts0_ram_dbe : 1;
		u64 std3_ram_dbe : 1;
		u64 std2_ram_dbe : 1;
		u64 std1_ram_dbe : 1;
		u64 std0_ram_dbe : 1;
		u64 wt_ram_dbe : 1;
		u64 sc_ram_dbe : 1;
		u64 reserved_0_42 : 43;
	} s;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq5_ecc_dbe_sts0 cvmx_pko_pse_sq5_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_s {
		u64 pse_sq5_dbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq5_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq5_ecc_sbe_sts0 {
	u64 u64;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts0_s {
		u64 sq_pt_ram_sbe : 1;
		u64 sq_nt_ram_sbe : 1;
		u64 rt_ram_sbe : 1;
		u64 tw3_cmd_fifo_ram_sbe : 1;
		u64 tw2_cmd_fifo_ram_sbe : 1;
		u64 tw1_cmd_fifo_ram_sbe : 1;
		u64 tw0_cmd_fifo_ram_sbe : 1;
		u64 tp3_sram_sbe : 1;
		u64 tp2_sram_sbe : 1;
		u64 tp1_sram_sbe : 1;
		u64 tp0_sram_sbe : 1;
		u64 sts3_ram_sbe : 1;
		u64 sts2_ram_sbe : 1;
		u64 sts1_ram_sbe : 1;
		u64 sts0_ram_sbe : 1;
		u64 std3_ram_sbe : 1;
		u64 std2_ram_sbe : 1;
		u64 std1_ram_sbe : 1;
		u64 std0_ram_sbe : 1;
		u64 wt_ram_sbe : 1;
		u64 sc_ram_sbe : 1;
		u64 reserved_0_42 : 43;
	} s;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts0_s cn78xx;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq5_ecc_sbe_sts0 cvmx_pko_pse_sq5_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0 {
	u64 u64;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_s {
		u64 pse_sq5_sbe_cmb0 : 1;
		u64 reserved_0_62 : 63;
	} s;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_s cn78xx;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_s cn78xxp1;
};

typedef union cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_ptf#_status
 */
union cvmx_pko_ptfx_status {
	u64 u64;
	struct cvmx_pko_ptfx_status_s {
		u64 reserved_30_63 : 34;
		u64 tx_fifo_pkt_credit_cnt : 10;
		u64 total_in_flight_cnt : 8;
		u64 in_flight_cnt : 7;
		u64 mac_num : 5;
	} s;
	struct cvmx_pko_ptfx_status_s cn73xx;
	struct cvmx_pko_ptfx_status_s cn78xx;
	struct cvmx_pko_ptfx_status_s cn78xxp1;
	struct cvmx_pko_ptfx_status_s cnf75xx;
};

typedef union cvmx_pko_ptfx_status cvmx_pko_ptfx_status_t;

/**
 * cvmx_pko_ptf_iobp_cfg
 */
union cvmx_pko_ptf_iobp_cfg {
	u64 u64;
	struct cvmx_pko_ptf_iobp_cfg_s {
		u64 reserved_44_63 : 20;
		u64 iobp1_ds_opt : 1;
		u64 iobp0_l2_allocate : 1;
		u64 iobp1_magic_addr : 35;
		u64 max_read_size : 7;
	} s;
	struct cvmx_pko_ptf_iobp_cfg_s cn73xx;
	struct cvmx_pko_ptf_iobp_cfg_s cn78xx;
	struct cvmx_pko_ptf_iobp_cfg_s cn78xxp1;
	struct cvmx_pko_ptf_iobp_cfg_s cnf75xx;
};

typedef union cvmx_pko_ptf_iobp_cfg cvmx_pko_ptf_iobp_cfg_t;

/**
 * cvmx_pko_ptgf#_cfg
 *
 * This register configures a PKO TX FIFO group. PKO supports up to 17 independent
 * TX FIFOs, where 0-15 are physical and 16 is Virtual/NULL. (PKO drops packets
 * targeting the NULL FIFO, returning their buffers to the FPA.) PKO puts each
 * FIFO into one of five groups:
 *
 * <pre>
 *    CSR Name       FIFO's in FIFO Group
 *   ------------------------------------
 *   PKO_PTGF0_CFG      0,  1,  2,  3
 *   PKO_PTGF1_CFG      4,  5,  6,  7
 *   PKO_PTGF2_CFG      8,  9, 10, 11
 *   PKO_PTGF3_CFG     12, 13, 14, 15
 *   PKO_PTGF4_CFG      Virtual/NULL
 * </pre>
 */
union cvmx_pko_ptgfx_cfg {
	u64 u64;
	struct cvmx_pko_ptgfx_cfg_s {
		u64 reserved_7_63 : 57;
		u64 reset : 1;
		u64 rate : 3;
		u64 size : 3;
	} s;
	struct cvmx_pko_ptgfx_cfg_cn73xx {
		u64 reserved_7_63 : 57;
		u64 reset : 1;
		u64 reserved_5_5 : 1;
		u64 rate : 2;
		u64 size : 3;
	} cn73xx;
	struct cvmx_pko_ptgfx_cfg_s cn78xx;
	struct cvmx_pko_ptgfx_cfg_s cn78xxp1;
	struct cvmx_pko_ptgfx_cfg_cn73xx cnf75xx;
};

typedef union cvmx_pko_ptgfx_cfg cvmx_pko_ptgfx_cfg_t;

/**
 * cvmx_pko_reg_bist_result
 *
 * Notes:
 * Access to the internal BiST results
 * Each bit is the BiST result of an individual memory (per bit, 0=pass and 1=fail).
 */
union cvmx_pko_reg_bist_result {
	u64 u64;
	struct cvmx_pko_reg_bist_result_s {
		u64 reserved_0_63 : 64;
	} s;
	struct cvmx_pko_reg_bist_result_cn30xx {
		u64 reserved_27_63 : 37;
		u64 psb2 : 5;
		u64 count : 1;
		u64 rif : 1;
		u64 wif : 1;
		u64 ncb : 1;
		u64 out : 1;
		u64 crc : 1;
		u64 chk : 1;
		u64 qsb : 2;
		u64 qcb : 2;
		u64 pdb : 4;
		u64 psb : 7;
	} cn30xx;
	struct cvmx_pko_reg_bist_result_cn30xx cn31xx;
	struct cvmx_pko_reg_bist_result_cn30xx cn38xx;
	struct cvmx_pko_reg_bist_result_cn30xx cn38xxp2;
	struct cvmx_pko_reg_bist_result_cn50xx {
		u64 reserved_33_63 : 31;
		u64 csr : 1;
		u64 iob : 1;
		u64 out_crc : 1;
		u64 out_ctl : 3;
		u64 out_sta : 1;
		u64 out_wif : 1;
		u64 prt_chk : 3;
		u64 prt_nxt : 1;
		u64 prt_psb : 6;
		u64 ncb_inb : 2;
		u64 prt_qcb : 2;
		u64 prt_qsb : 3;
		u64 dat_dat : 4;
		u64 dat_ptr : 4;
	} cn50xx;
	struct cvmx_pko_reg_bist_result_cn52xx {
		u64 reserved_35_63 : 29;
		u64 csr : 1;
		u64 iob : 1;
		u64 out_dat : 1;
		u64 out_ctl : 3;
		u64 out_sta : 1;
		u64 out_wif : 1;
		u64 prt_chk : 3;
		u64 prt_nxt : 1;
		u64 prt_psb : 8;
		u64 ncb_inb : 2;
		u64 prt_qcb : 2;
		u64 prt_qsb : 3;
		u64 prt_ctl : 2;
		u64 dat_dat : 2;
		u64 dat_ptr : 4;
	} cn52xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn52xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cn56xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn56xxp1;
	struct cvmx_pko_reg_bist_result_cn50xx cn58xx;
	struct cvmx_pko_reg_bist_result_cn50xx cn58xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cn61xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn63xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn63xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cn66xx;
	struct cvmx_pko_reg_bist_result_cn68xx {
		u64 reserved_36_63 : 28;
		u64 crc : 1;
		u64 csr : 1;
		u64 iob : 1;
		u64 out_dat : 1;
		u64 reserved_31_31 : 1;
		u64 out_ctl : 2;
		u64 out_sta : 1;
		u64 out_wif : 1;
		u64 prt_chk : 3;
		u64 prt_nxt : 1;
		u64 prt_psb7 : 1;
		u64 reserved_21_21 : 1;
		u64 prt_psb : 6;
		u64 ncb_inb : 2;
		u64 prt_qcb : 2;
		u64 prt_qsb : 3;
		u64 prt_ctl : 2;
		u64 dat_dat : 2;
		u64 dat_ptr : 4;
	} cn68xx;
	struct cvmx_pko_reg_bist_result_cn68xxp1 {
		u64 reserved_35_63 : 29;
		u64 csr : 1;
		u64 iob : 1;
		u64 out_dat : 1;
		u64 reserved_31_31 : 1;
		u64 out_ctl : 2;
		u64 out_sta : 1;
		u64 out_wif : 1;
		u64 prt_chk : 3;
		u64 prt_nxt : 1;
		u64 prt_psb7 : 1;
		u64 reserved_21_21 : 1;
		u64 prt_psb : 6;
		u64 ncb_inb : 2;
		u64 prt_qcb : 2;
		u64 prt_qsb : 3;
		u64 prt_ctl : 2;
		u64 dat_dat : 2;
		u64 dat_ptr : 4;
	} cn68xxp1;
	struct cvmx_pko_reg_bist_result_cn70xx {
		u64 reserved_30_63 : 34;
		u64 csr : 1;
		u64 iob : 1;
		u64 out_dat : 1;
		u64 out_ctl : 1;
		u64 out_sta : 1;
		u64 out_wif : 1;
		u64 prt_chk : 3;
		u64 prt_nxt : 1;
		u64 prt_psb : 8;
		u64 ncb_inb : 1;
		u64 prt_qcb : 1;
		u64 prt_qsb : 2;
		u64 prt_ctl : 2;
		u64 dat_dat : 2;
		u64 dat_ptr : 4;
	} cn70xx;
	struct cvmx_pko_reg_bist_result_cn70xx cn70xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cnf71xx;
};

typedef union cvmx_pko_reg_bist_result cvmx_pko_reg_bist_result_t;

/**
 * cvmx_pko_reg_cmd_buf
 *
 * Notes:
 * Sets the command buffer parameters
 * The size of the command buffer segments is measured in uint64s.  The pool specifies (1 of 8 free
 * lists to be used when freeing command buffer segments.
 */
union cvmx_pko_reg_cmd_buf {
	u64 u64;
	struct cvmx_pko_reg_cmd_buf_s {
		u64 reserved_23_63 : 41;
		u64 pool : 3;
		u64 reserved_13_19 : 7;
		u64 size : 13;
	} s;
	struct cvmx_pko_reg_cmd_buf_s cn30xx;
	struct cvmx_pko_reg_cmd_buf_s cn31xx;
	struct cvmx_pko_reg_cmd_buf_s cn38xx;
	struct cvmx_pko_reg_cmd_buf_s cn38xxp2;
	struct cvmx_pko_reg_cmd_buf_s cn50xx;
	struct cvmx_pko_reg_cmd_buf_s cn52xx;
	struct cvmx_pko_reg_cmd_buf_s cn52xxp1;
	struct cvmx_pko_reg_cmd_buf_s cn56xx;
	struct cvmx_pko_reg_cmd_buf_s cn56xxp1;
	struct cvmx_pko_reg_cmd_buf_s cn58xx;
	struct cvmx_pko_reg_cmd_buf_s cn58xxp1;
	struct cvmx_pko_reg_cmd_buf_s cn61xx;
	struct cvmx_pko_reg_cmd_buf_s cn63xx;
	struct cvmx_pko_reg_cmd_buf_s cn63xxp1;
	struct cvmx_pko_reg_cmd_buf_s cn66xx;
	struct cvmx_pko_reg_cmd_buf_s cn68xx;
	struct cvmx_pko_reg_cmd_buf_s cn68xxp1;
	struct cvmx_pko_reg_cmd_buf_cn70xx {
		u64 reserved_23_63 : 41;
		u64 pool : 3;
		u64 reserved_19_13 : 7;
		u64 size : 13;
	} cn70xx;
	struct cvmx_pko_reg_cmd_buf_cn70xx cn70xxp1;
	struct cvmx_pko_reg_cmd_buf_s cnf71xx;
};

typedef union cvmx_pko_reg_cmd_buf cvmx_pko_reg_cmd_buf_t;

/**
 * cvmx_pko_reg_crc_ctl#
 *
 * Notes:
 * Controls datapath reflection when calculating CRC
 *
 */
union cvmx_pko_reg_crc_ctlx {
	u64 u64;
	struct cvmx_pko_reg_crc_ctlx_s {
		u64 reserved_2_63 : 62;
		u64 invres : 1;
		u64 refin : 1;
	} s;
	struct cvmx_pko_reg_crc_ctlx_s cn38xx;
	struct cvmx_pko_reg_crc_ctlx_s cn38xxp2;
	struct cvmx_pko_reg_crc_ctlx_s cn58xx;
	struct cvmx_pko_reg_crc_ctlx_s cn58xxp1;
};

typedef union cvmx_pko_reg_crc_ctlx cvmx_pko_reg_crc_ctlx_t;

/**
 * cvmx_pko_reg_crc_enable
 *
 * Notes:
 * Enables CRC for the GMX ports.
 *
 */
union cvmx_pko_reg_crc_enable {
	u64 u64;
	struct cvmx_pko_reg_crc_enable_s {
		u64 reserved_32_63 : 32;
		u64 enable : 32;
	} s;
	struct cvmx_pko_reg_crc_enable_s cn38xx;
	struct cvmx_pko_reg_crc_enable_s cn38xxp2;
	struct cvmx_pko_reg_crc_enable_s cn58xx;
	struct cvmx_pko_reg_crc_enable_s cn58xxp1;
};

typedef union cvmx_pko_reg_crc_enable cvmx_pko_reg_crc_enable_t;

/**
 * cvmx_pko_reg_crc_iv#
 *
 * Notes:
 * Determines the IV used by the CRC algorithm
 * * PKO_CRC_IV
 *  PKO_CRC_IV controls the initial state of the CRC algorithm.  Octane can
 *  support a wide range of CRC algorithms and as such, the IV must be
 *  carefully constructed to meet the specific algorithm.  The code below
 *  determines the value to program into Octane based on the algorthim's IV
 *  and width.  In the case of Octane, the width should always be 32.
 *
 *  PKO_CRC_IV0 sets the IV for ports 0-15 while PKO_CRC_IV1 sets the IV for
 *  ports 16-31.
 *
 *   @verbatim
 *   unsigned octane_crc_iv(unsigned algorithm_iv, unsigned poly, unsigned w)
 *   [
 *     int i;
 *     int doit;
 *     unsigned int current_val = algorithm_iv;
 *
 *     for(i = 0; i < w; i++) [
 *       doit = current_val & 0x1;
 *
 *       if(doit) current_val ^= poly;
 *       assert(!(current_val & 0x1));
 *
 *       current_val = (current_val >> 1) | (doit << (w-1));
 *     ]
 *
 *     return current_val;
 *   ]
 *   @endverbatim
 */
union cvmx_pko_reg_crc_ivx {
	u64 u64;
	struct cvmx_pko_reg_crc_ivx_s {
		u64 reserved_32_63 : 32;
		u64 iv : 32;
	} s;
	struct cvmx_pko_reg_crc_ivx_s cn38xx;
	struct cvmx_pko_reg_crc_ivx_s cn38xxp2;
	struct cvmx_pko_reg_crc_ivx_s cn58xx;
	struct cvmx_pko_reg_crc_ivx_s cn58xxp1;
};

typedef union cvmx_pko_reg_crc_ivx cvmx_pko_reg_crc_ivx_t;

/**
 * cvmx_pko_reg_debug0
 *
 * Notes:
 * Note that this CSR is present only in chip revisions beginning with pass2.
 *
 */
union cvmx_pko_reg_debug0 {
	u64 u64;
	struct cvmx_pko_reg_debug0_s {
		u64 asserts : 64;
	} s;
	struct cvmx_pko_reg_debug0_cn30xx {
		u64 reserved_17_63 : 47;
		u64 asserts : 17;
	} cn30xx;
	struct cvmx_pko_reg_debug0_cn30xx cn31xx;
	struct cvmx_pko_reg_debug0_cn30xx cn38xx;
	struct cvmx_pko_reg_debug0_cn30xx cn38xxp2;
	struct cvmx_pko_reg_debug0_s cn50xx;
	struct cvmx_pko_reg_debug0_s cn52xx;
	struct cvmx_pko_reg_debug0_s cn52xxp1;
	struct cvmx_pko_reg_debug0_s cn56xx;
	struct cvmx_pko_reg_debug0_s cn56xxp1;
	struct cvmx_pko_reg_debug0_s cn58xx;
	struct cvmx_pko_reg_debug0_s cn58xxp1;
	struct cvmx_pko_reg_debug0_s cn61xx;
	struct cvmx_pko_reg_debug0_s cn63xx;
	struct cvmx_pko_reg_debug0_s cn63xxp1;
	struct cvmx_pko_reg_debug0_s cn66xx;
	struct cvmx_pko_reg_debug0_s cn68xx;
	struct cvmx_pko_reg_debug0_s cn68xxp1;
	struct cvmx_pko_reg_debug0_s cn70xx;
	struct cvmx_pko_reg_debug0_s cn70xxp1;
	struct cvmx_pko_reg_debug0_s cnf71xx;
};

typedef union cvmx_pko_reg_debug0 cvmx_pko_reg_debug0_t;

/**
 * cvmx_pko_reg_debug1
 */
union cvmx_pko_reg_debug1 {
	u64 u64;
	struct cvmx_pko_reg_debug1_s {
		u64 asserts : 64;
	} s;
	struct cvmx_pko_reg_debug1_s cn50xx;
	struct cvmx_pko_reg_debug1_s cn52xx;
	struct cvmx_pko_reg_debug1_s cn52xxp1;
	struct cvmx_pko_reg_debug1_s cn56xx;
	struct cvmx_pko_reg_debug1_s cn56xxp1;
	struct cvmx_pko_reg_debug1_s cn58xx;
	struct cvmx_pko_reg_debug1_s cn58xxp1;
	struct cvmx_pko_reg_debug1_s cn61xx;
	struct cvmx_pko_reg_debug1_s cn63xx;
	struct cvmx_pko_reg_debug1_s cn63xxp1;
	struct cvmx_pko_reg_debug1_s cn66xx;
	struct cvmx_pko_reg_debug1_s cn68xx;
	struct cvmx_pko_reg_debug1_s cn68xxp1;
	struct cvmx_pko_reg_debug1_s cn70xx;
	struct cvmx_pko_reg_debug1_s cn70xxp1;
	struct cvmx_pko_reg_debug1_s cnf71xx;
};

typedef union cvmx_pko_reg_debug1 cvmx_pko_reg_debug1_t;

/**
 * cvmx_pko_reg_debug2
 */
union cvmx_pko_reg_debug2 {
	u64 u64;
	struct cvmx_pko_reg_debug2_s {
		u64 asserts : 64;
	} s;
	struct cvmx_pko_reg_debug2_s cn50xx;
	struct cvmx_pko_reg_debug2_s cn52xx;
	struct cvmx_pko_reg_debug2_s cn52xxp1;
	struct cvmx_pko_reg_debug2_s cn56xx;
	struct cvmx_pko_reg_debug2_s cn56xxp1;
	struct cvmx_pko_reg_debug2_s cn58xx;
	struct cvmx_pko_reg_debug2_s cn58xxp1;
	struct cvmx_pko_reg_debug2_s cn61xx;
	struct cvmx_pko_reg_debug2_s cn63xx;
	struct cvmx_pko_reg_debug2_s cn63xxp1;
	struct cvmx_pko_reg_debug2_s cn66xx;
	struct cvmx_pko_reg_debug2_s cn68xx;
	struct cvmx_pko_reg_debug2_s cn68xxp1;
	struct cvmx_pko_reg_debug2_s cn70xx;
	struct cvmx_pko_reg_debug2_s cn70xxp1;
	struct cvmx_pko_reg_debug2_s cnf71xx;
};

typedef union cvmx_pko_reg_debug2 cvmx_pko_reg_debug2_t;

/**
 * cvmx_pko_reg_debug3
 */
union cvmx_pko_reg_debug3 {
	u64 u64;
	struct cvmx_pko_reg_debug3_s {
		u64 asserts : 64;
	} s;
	struct cvmx_pko_reg_debug3_s cn50xx;
	struct cvmx_pko_reg_debug3_s cn52xx;
	struct cvmx_pko_reg_debug3_s cn52xxp1;
	struct cvmx_pko_reg_debug3_s cn56xx;
	struct cvmx_pko_reg_debug3_s cn56xxp1;
	struct cvmx_pko_reg_debug3_s cn58xx;
	struct cvmx_pko_reg_debug3_s cn58xxp1;
	struct cvmx_pko_reg_debug3_s cn61xx;
	struct cvmx_pko_reg_debug3_s cn63xx;
	struct cvmx_pko_reg_debug3_s cn63xxp1;
	struct cvmx_pko_reg_debug3_s cn66xx;
	struct cvmx_pko_reg_debug3_s cn68xx;
	struct cvmx_pko_reg_debug3_s cn68xxp1;
	struct cvmx_pko_reg_debug3_s cn70xx;
	struct cvmx_pko_reg_debug3_s cn70xxp1;
	struct cvmx_pko_reg_debug3_s cnf71xx;
};

typedef union cvmx_pko_reg_debug3 cvmx_pko_reg_debug3_t;

/**
 * cvmx_pko_reg_debug4
 */
union cvmx_pko_reg_debug4 {
	u64 u64;
	struct cvmx_pko_reg_debug4_s {
		u64 asserts : 64;
	} s;
	struct cvmx_pko_reg_debug4_s cn68xx;
	struct cvmx_pko_reg_debug4_s cn68xxp1;
};

typedef union cvmx_pko_reg_debug4 cvmx_pko_reg_debug4_t;

/**
 * cvmx_pko_reg_engine_inflight
 *
 * Notes:
 * Sets the maximum number of inflight packets, per engine.  Values greater than 4 are illegal.
 * Setting an engine's value to 0 effectively stops the engine.
 */
union cvmx_pko_reg_engine_inflight {
	u64 u64;
	struct cvmx_pko_reg_engine_inflight_s {
		u64 engine15 : 4;
		u64 engine14 : 4;
		u64 engine13 : 4;
		u64 engine12 : 4;
		u64 engine11 : 4;
		u64 engine10 : 4;
		u64 engine9 : 4;
		u64 engine8 : 4;
		u64 engine7 : 4;
		u64 engine6 : 4;
		u64 engine5 : 4;
		u64 engine4 : 4;
		u64 engine3 : 4;
		u64 engine2 : 4;
		u64 engine1 : 4;
		u64 engine0 : 4;
	} s;
	struct cvmx_pko_reg_engine_inflight_cn52xx {
		u64 reserved_40_63 : 24;
		u64 engine9 : 4;
		u64 engine8 : 4;
		u64 engine7 : 4;
		u64 engine6 : 4;
		u64 engine5 : 4;
		u64 engine4 : 4;
		u64 engine3 : 4;
		u64 engine2 : 4;
		u64 engine1 : 4;
		u64 engine0 : 4;
	} cn52xx;
	struct cvmx_pko_reg_engine_inflight_cn52xx cn52xxp1;
	struct cvmx_pko_reg_engine_inflight_cn52xx cn56xx;
	struct cvmx_pko_reg_engine_inflight_cn52xx cn56xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx {
		u64 reserved_56_63 : 8;
		u64 engine13 : 4;
		u64 engine12 : 4;
		u64 engine11 : 4;
		u64 engine10 : 4;
		u64 engine9 : 4;
		u64 engine8 : 4;
		u64 engine7 : 4;
		u64 engine6 : 4;
		u64 engine5 : 4;
		u64 engine4 : 4;
		u64 engine3 : 4;
		u64 engine2 : 4;
		u64 engine1 : 4;
		u64 engine0 : 4;
	} cn61xx;
	struct cvmx_pko_reg_engine_inflight_cn63xx {
		u64 reserved_48_63 : 16;
		u64 engine11 : 4;
		u64 engine10 : 4;
		u64 engine9 : 4;
		u64 engine8 : 4;
		u64 engine7 : 4;
		u64 engine6 : 4;
		u64 engine5 : 4;
		u64 engine4 : 4;
		u64 engine3 : 4;
		u64 engine2 : 4;
		u64 engine1 : 4;
		u64 engine0 : 4;
	} cn63xx;
	struct cvmx_pko_reg_engine_inflight_cn63xx cn63xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx cn66xx;
	struct cvmx_pko_reg_engine_inflight_s cn68xx;
	struct cvmx_pko_reg_engine_inflight_s cn68xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx cn70xx;
	struct cvmx_pko_reg_engine_inflight_cn61xx cn70xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx cnf71xx;
};

typedef union cvmx_pko_reg_engine_inflight cvmx_pko_reg_engine_inflight_t;

/**
 * cvmx_pko_reg_engine_inflight1
 *
 * Notes:
 * Sets the maximum number of inflight packets, per engine.  Values greater than 8 are illegal.
 * Setting an engine's value to 0 effectively stops the engine.
 */
union cvmx_pko_reg_engine_inflight1 {
	u64 u64;
	struct cvmx_pko_reg_engine_inflight1_s {
		u64 reserved_16_63 : 48;
		u64 engine19 : 4;
		u64 engine18 : 4;
		u64 engine17 : 4;
		u64 engine16 : 4;
	} s;
	struct cvmx_pko_reg_engine_inflight1_s cn68xx;
	struct cvmx_pko_reg_engine_inflight1_s cn68xxp1;
};

typedef union cvmx_pko_reg_engine_inflight1 cvmx_pko_reg_engine_inflight1_t;

/**
 * cvmx_pko_reg_engine_storage#
 *
 * Notes:
 * The PKO has 40KB of local storage, consisting of 20, 2KB chunks.  Up to 15 contiguous chunks may be mapped per engine.
 * The total of all mapped storage must not exceed 40KB.
 */
union cvmx_pko_reg_engine_storagex {
	u64 u64;
	struct cvmx_pko_reg_engine_storagex_s {
		u64 engine15 : 4;
		u64 engine14 : 4;
		u64 engine13 : 4;
		u64 engine12 : 4;
		u64 engine11 : 4;
		u64 engine10 : 4;
		u64 engine9 : 4;
		u64 engine8 : 4;
		u64 engine7 : 4;
		u64 engine6 : 4;
		u64 engine5 : 4;
		u64 engine4 : 4;
		u64 engine3 : 4;
		u64 engine2 : 4;
		u64 engine1 : 4;
		u64 engine0 : 4;
	} s;
	struct cvmx_pko_reg_engine_storagex_s cn68xx;
	struct cvmx_pko_reg_engine_storagex_s cn68xxp1;
};

typedef union cvmx_pko_reg_engine_storagex cvmx_pko_reg_engine_storagex_t;

/**
 * cvmx_pko_reg_engine_thresh
 *
 * Notes:
 * When not enabled, packet data may be sent as soon as it is written into PKO's internal buffers.
 * When enabled and the packet fits entirely in the PKO's internal buffer, none of the packet data will
 * be sent until all of it has been written into the PKO's internal buffer.  Note that a packet is
 * considered to fit entirely only if the packet's size is <= BUFFER_SIZE-8.  When enabled and the
 * packet does not fit entirely in the PKO's internal buffer, none of the packet data will be sent until
 * at least BUFFER_SIZE-256 bytes of the packet have been written into the PKO's internal buffer
 * (note that BUFFER_SIZE is a function of PKO_REG_GMX_PORT_MODE above)
 */
union cvmx_pko_reg_engine_thresh {
	u64 u64;
	struct cvmx_pko_reg_engine_thresh_s {
		u64 reserved_20_63 : 44;
		u64 mask : 20;
	} s;
	struct cvmx_pko_reg_engine_thresh_cn52xx {
		u64 reserved_10_63 : 54;
		u64 mask : 10;
	} cn52xx;
	struct cvmx_pko_reg_engine_thresh_cn52xx cn52xxp1;
	struct cvmx_pko_reg_engine_thresh_cn52xx cn56xx;
	struct cvmx_pko_reg_engine_thresh_cn52xx cn56xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx {
		u64 reserved_14_63 : 50;
		u64 mask : 14;
	} cn61xx;
	struct cvmx_pko_reg_engine_thresh_cn63xx {
		u64 reserved_12_63 : 52;
		u64 mask : 12;
	} cn63xx;
	struct cvmx_pko_reg_engine_thresh_cn63xx cn63xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx cn66xx;
	struct cvmx_pko_reg_engine_thresh_s cn68xx;
	struct cvmx_pko_reg_engine_thresh_s cn68xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx cn70xx;
	struct cvmx_pko_reg_engine_thresh_cn61xx cn70xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx cnf71xx;
};

typedef union cvmx_pko_reg_engine_thresh cvmx_pko_reg_engine_thresh_t;

/**
 * cvmx_pko_reg_error
 *
 * Notes:
 * Note that this CSR is present only in chip revisions beginning with pass2.
 *
 */
union cvmx_pko_reg_error {
	u64 u64;
	struct cvmx_pko_reg_error_s {
		u64 reserved_4_63 : 60;
		u64 loopback : 1;
		u64 currzero : 1;
		u64 doorbell : 1;
		u64 parity : 1;
	} s;
	struct cvmx_pko_reg_error_cn30xx {
		u64 reserved_2_63 : 62;
		u64 doorbell : 1;
		u64 parity : 1;
	} cn30xx;
	struct cvmx_pko_reg_error_cn30xx cn31xx;
	struct cvmx_pko_reg_error_cn30xx cn38xx;
	struct cvmx_pko_reg_error_cn30xx cn38xxp2;
	struct cvmx_pko_reg_error_cn50xx {
		u64 reserved_3_63 : 61;
		u64 currzero : 1;
		u64 doorbell : 1;
		u64 parity : 1;
	} cn50xx;
	struct cvmx_pko_reg_error_cn50xx cn52xx;
	struct cvmx_pko_reg_error_cn50xx cn52xxp1;
	struct cvmx_pko_reg_error_cn50xx cn56xx;
	struct cvmx_pko_reg_error_cn50xx cn56xxp1;
	struct cvmx_pko_reg_error_cn50xx cn58xx;
	struct cvmx_pko_reg_error_cn50xx cn58xxp1;
	struct cvmx_pko_reg_error_cn50xx cn61xx;
	struct cvmx_pko_reg_error_cn50xx cn63xx;
	struct cvmx_pko_reg_error_cn50xx cn63xxp1;
	struct cvmx_pko_reg_error_cn50xx cn66xx;
	struct cvmx_pko_reg_error_s cn68xx;
	struct cvmx_pko_reg_error_s cn68xxp1;
	struct cvmx_pko_reg_error_cn50xx cn70xx;
	struct cvmx_pko_reg_error_cn50xx cn70xxp1;
	struct cvmx_pko_reg_error_cn50xx cnf71xx;
};

typedef union cvmx_pko_reg_error cvmx_pko_reg_error_t;

/**
 * cvmx_pko_reg_flags
 *
 * Notes:
 * When set, ENA_PKO enables the PKO picker and places the PKO in normal operation.  When set, ENA_DWB
 * enables the use of DontWriteBacks during the buffer freeing operations.  When not set, STORE_BE inverts
 * bits[2:0] of the STORE0 byte write address.  When set, RESET causes a 4-cycle reset pulse to the
 * entire box.
 */
union cvmx_pko_reg_flags {
	u64 u64;
	struct cvmx_pko_reg_flags_s {
		u64 reserved_9_63 : 55;
		u64 dis_perf3 : 1;
		u64 dis_perf2 : 1;
		u64 dis_perf1 : 1;
		u64 dis_perf0 : 1;
		u64 ena_throttle : 1;
		u64 reset : 1;
		u64 store_be : 1;
		u64 ena_dwb : 1;
		u64 ena_pko : 1;
	} s;
	struct cvmx_pko_reg_flags_cn30xx {
		u64 reserved_4_63 : 60;
		u64 reset : 1;
		u64 store_be : 1;
		u64 ena_dwb : 1;
		u64 ena_pko : 1;
	} cn30xx;
	struct cvmx_pko_reg_flags_cn30xx cn31xx;
	struct cvmx_pko_reg_flags_cn30xx cn38xx;
	struct cvmx_pko_reg_flags_cn30xx cn38xxp2;
	struct cvmx_pko_reg_flags_cn30xx cn50xx;
	struct cvmx_pko_reg_flags_cn30xx cn52xx;
	struct cvmx_pko_reg_flags_cn30xx cn52xxp1;
	struct cvmx_pko_reg_flags_cn30xx cn56xx;
	struct cvmx_pko_reg_flags_cn30xx cn56xxp1;
	struct cvmx_pko_reg_flags_cn30xx cn58xx;
	struct cvmx_pko_reg_flags_cn30xx cn58xxp1;
	struct cvmx_pko_reg_flags_cn61xx {
		u64 reserved_9_63 : 55;
		u64 dis_perf3 : 1;
		u64 dis_perf2 : 1;
		u64 reserved_4_6 : 3;
		u64 reset : 1;
		u64 store_be : 1;
		u64 ena_dwb : 1;
		u64 ena_pko : 1;
	} cn61xx;
	struct cvmx_pko_reg_flags_cn30xx cn63xx;
	struct cvmx_pko_reg_flags_cn30xx cn63xxp1;
	struct cvmx_pko_reg_flags_cn61xx cn66xx;
	struct cvmx_pko_reg_flags_s cn68xx;
	struct cvmx_pko_reg_flags_cn68xxp1 {
		u64 reserved_7_63 : 57;
		u64 dis_perf1 : 1;
		u64 dis_perf0 : 1;
		u64 ena_throttle : 1;
		u64 reset : 1;
		u64 store_be : 1;
		u64 ena_dwb : 1;
		u64 ena_pko : 1;
	} cn68xxp1;
	struct cvmx_pko_reg_flags_cn61xx cn70xx;
	struct cvmx_pko_reg_flags_cn61xx cn70xxp1;
	struct cvmx_pko_reg_flags_cn61xx cnf71xx;
};

typedef union cvmx_pko_reg_flags cvmx_pko_reg_flags_t;

/**
 * cvmx_pko_reg_gmx_port_mode
 *
 * Notes:
 * The system has a total of 2 + 4 + 4 ports and 2 + 1 + 1 engines (GM0 + PCI + LOOP).
 * This CSR sets the number of GMX0 ports and amount of local storage per engine.
 * It has no effect on the number of ports or amount of local storage per engine for PCI and LOOP.
 * When both GMX ports are used (MODE0=3), each GMX engine has 10kB of local
 * storage.  Increasing MODE0 to 4 decreases the number of GMX ports to 1 and
 * increases the local storage for the one remaining PKO GMX engine to 20kB.
 * MODE0 value 0, 1, and 2, or greater than 4 are illegal.
 *
 * MODE0   GMX0  PCI   LOOP  GMX0                       PCI            LOOP
 *         ports ports ports storage/engine             storage/engine storage/engine
 * 3       2     4     4      10.0kB                    2.5kB          2.5kB
 * 4       1     4     4      20.0kB                    2.5kB          2.5kB
 */
union cvmx_pko_reg_gmx_port_mode {
	u64 u64;
	struct cvmx_pko_reg_gmx_port_mode_s {
		u64 reserved_6_63 : 58;
		u64 mode1 : 3;
		u64 mode0 : 3;
	} s;
	struct cvmx_pko_reg_gmx_port_mode_s cn30xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn31xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn38xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn38xxp2;
	struct cvmx_pko_reg_gmx_port_mode_s cn50xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn52xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn52xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s cn56xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn56xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s cn58xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn58xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s cn61xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn63xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn63xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s cn66xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn70xx;
	struct cvmx_pko_reg_gmx_port_mode_s cn70xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s cnf71xx;
};

typedef union cvmx_pko_reg_gmx_port_mode cvmx_pko_reg_gmx_port_mode_t;

/**
 * cvmx_pko_reg_int_mask
 *
 * Notes:
 * When a mask bit is set, the corresponding interrupt is enabled.
 *
 */
union cvmx_pko_reg_int_mask {
	u64 u64;
	struct cvmx_pko_reg_int_mask_s {
		u64 reserved_4_63 : 60;
		u64 loopback : 1;
		u64 currzero : 1;
		u64 doorbell : 1;
		u64 parity : 1;
	} s;
	struct cvmx_pko_reg_int_mask_cn30xx {
		u64 reserved_2_63 : 62;
		u64 doorbell : 1;
		u64 parity : 1;
	} cn30xx;
	struct cvmx_pko_reg_int_mask_cn30xx cn31xx;
	struct cvmx_pko_reg_int_mask_cn30xx cn38xx;
	struct cvmx_pko_reg_int_mask_cn30xx cn38xxp2;
	struct cvmx_pko_reg_int_mask_cn50xx {
		u64 reserved_3_63 : 61;
		u64 currzero : 1;
		u64 doorbell : 1;
		u64 parity : 1;
	} cn50xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn52xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn52xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx cn56xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn56xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx cn58xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn58xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx cn61xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn63xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn63xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx cn66xx;
	struct cvmx_pko_reg_int_mask_s cn68xx;
	struct cvmx_pko_reg_int_mask_s cn68xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx cn70xx;
	struct cvmx_pko_reg_int_mask_cn50xx cn70xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx cnf71xx;
};

typedef union cvmx_pko_reg_int_mask cvmx_pko_reg_int_mask_t;

/**
 * cvmx_pko_reg_loopback_bpid
 *
 * Notes:
 * None.
 *
 */
union cvmx_pko_reg_loopback_bpid {
	u64 u64;
	struct cvmx_pko_reg_loopback_bpid_s {
		u64 reserved_59_63 : 5;
		u64 bpid7 : 6;
		u64 reserved_52_52 : 1;
		u64 bpid6 : 6;
		u64 reserved_45_45 : 1;
		u64 bpid5 : 6;
		u64 reserved_38_38 : 1;
		u64 bpid4 : 6;
		u64 reserved_31_31 : 1;
		u64 bpid3 : 6;
		u64 reserved_24_24 : 1;
		u64 bpid2 : 6;
		u64 reserved_17_17 : 1;
		u64 bpid1 : 6;
		u64 reserved_10_10 : 1;
		u64 bpid0 : 6;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_pko_reg_loopback_bpid_s cn68xx;
	struct cvmx_pko_reg_loopback_bpid_s cn68xxp1;
};

typedef union cvmx_pko_reg_loopback_bpid cvmx_pko_reg_loopback_bpid_t;

/**
 * cvmx_pko_reg_loopback_pkind
 *
 * Notes:
 * None.
 *
 */
union cvmx_pko_reg_loopback_pkind {
	u64 u64;
	struct cvmx_pko_reg_loopback_pkind_s {
		u64 reserved_59_63 : 5;
		u64 pkind7 : 6;
		u64 reserved_52_52 : 1;
		u64 pkind6 : 6;
		u64 reserved_45_45 : 1;
		u64 pkind5 : 6;
		u64 reserved_38_38 : 1;
		u64 pkind4 : 6;
		u64 reserved_31_31 : 1;
		u64 pkind3 : 6;
		u64 reserved_24_24 : 1;
		u64 pkind2 : 6;
		u64 reserved_17_17 : 1;
		u64 pkind1 : 6;
		u64 reserved_10_10 : 1;
		u64 pkind0 : 6;
		u64 num_ports : 4;
	} s;
	struct cvmx_pko_reg_loopback_pkind_s cn68xx;
	struct cvmx_pko_reg_loopback_pkind_s cn68xxp1;
};

typedef union cvmx_pko_reg_loopback_pkind cvmx_pko_reg_loopback_pkind_t;

/**
 * cvmx_pko_reg_min_pkt
 *
 * Notes:
 * This CSR is used with PKO_MEM_IPORT_PTRS[MIN_PKT] to select the minimum packet size.  Packets whose
 * size in bytes < (SIZEn+1) are zero-padded to (SIZEn+1) bytes.  Note that this does not include CRC bytes.
 * SIZE0=0 is read-only and is used when no padding is desired.
 */
union cvmx_pko_reg_min_pkt {
	u64 u64;
	struct cvmx_pko_reg_min_pkt_s {
		u64 size7 : 8;
		u64 size6 : 8;
		u64 size5 : 8;
		u64 size4 : 8;
		u64 size3 : 8;
		u64 size2 : 8;
		u64 size1 : 8;
		u64 size0 : 8;
	} s;
	struct cvmx_pko_reg_min_pkt_s cn68xx;
	struct cvmx_pko_reg_min_pkt_s cn68xxp1;
};

typedef union cvmx_pko_reg_min_pkt cvmx_pko_reg_min_pkt_t;

/**
 * cvmx_pko_reg_preempt
 */
union cvmx_pko_reg_preempt {
	u64 u64;
	struct cvmx_pko_reg_preempt_s {
		u64 reserved_16_63 : 48;
		u64 min_size : 16;
	} s;
	struct cvmx_pko_reg_preempt_s cn52xx;
	struct cvmx_pko_reg_preempt_s cn52xxp1;
	struct cvmx_pko_reg_preempt_s cn56xx;
	struct cvmx_pko_reg_preempt_s cn56xxp1;
	struct cvmx_pko_reg_preempt_s cn61xx;
	struct cvmx_pko_reg_preempt_s cn63xx;
	struct cvmx_pko_reg_preempt_s cn63xxp1;
	struct cvmx_pko_reg_preempt_s cn66xx;
	struct cvmx_pko_reg_preempt_s cn68xx;
	struct cvmx_pko_reg_preempt_s cn68xxp1;
	struct cvmx_pko_reg_preempt_s cn70xx;
	struct cvmx_pko_reg_preempt_s cn70xxp1;
	struct cvmx_pko_reg_preempt_s cnf71xx;
};

typedef union cvmx_pko_reg_preempt cvmx_pko_reg_preempt_t;

/**
 * cvmx_pko_reg_queue_mode
 *
 * Notes:
 * Sets the number of queues and amount of local storage per queue
 * The system has a total of 256 queues and (256*8) words of local command storage.  This CSR sets the
 * number of queues that are used.  Increasing the value of MODE by 1 decreases the number of queues
 * by a power of 2 and increases the local storage per queue by a power of 2.
 * MODEn queues storage/queue
 * 0     256     64B ( 8 words)
 * 1     128    128B (16 words)
 * 2      64    256B (32 words)
 */
union cvmx_pko_reg_queue_mode {
	u64 u64;
	struct cvmx_pko_reg_queue_mode_s {
		u64 reserved_2_63 : 62;
		u64 mode : 2;
	} s;
	struct cvmx_pko_reg_queue_mode_s cn30xx;
	struct cvmx_pko_reg_queue_mode_s cn31xx;
	struct cvmx_pko_reg_queue_mode_s cn38xx;
	struct cvmx_pko_reg_queue_mode_s cn38xxp2;
	struct cvmx_pko_reg_queue_mode_s cn50xx;
	struct cvmx_pko_reg_queue_mode_s cn52xx;
	struct cvmx_pko_reg_queue_mode_s cn52xxp1;
	struct cvmx_pko_reg_queue_mode_s cn56xx;
	struct cvmx_pko_reg_queue_mode_s cn56xxp1;
	struct cvmx_pko_reg_queue_mode_s cn58xx;
	struct cvmx_pko_reg_queue_mode_s cn58xxp1;
	struct cvmx_pko_reg_queue_mode_s cn61xx;
	struct cvmx_pko_reg_queue_mode_s cn63xx;
	struct cvmx_pko_reg_queue_mode_s cn63xxp1;
	struct cvmx_pko_reg_queue_mode_s cn66xx;
	struct cvmx_pko_reg_queue_mode_s cn68xx;
	struct cvmx_pko_reg_queue_mode_s cn68xxp1;
	struct cvmx_pko_reg_queue_mode_s cn70xx;
	struct cvmx_pko_reg_queue_mode_s cn70xxp1;
	struct cvmx_pko_reg_queue_mode_s cnf71xx;
};

typedef union cvmx_pko_reg_queue_mode cvmx_pko_reg_queue_mode_t;

/**
 * cvmx_pko_reg_queue_preempt
 *
 * Notes:
 * Per QID, setting both PREEMPTER=1 and PREEMPTEE=1 is illegal and sets only PREEMPTER=1.
 * This CSR is used with PKO_MEM_QUEUE_PTRS and PKO_REG_QUEUE_PTRS1.  When programming queues, the
 * programming sequence must first write PKO_REG_QUEUE_PREEMPT, then PKO_REG_QUEUE_PTRS1 and then
 * PKO_MEM_QUEUE_PTRS for each queue.  Preemption is supported only on queues that are ultimately
 * mapped to engines 0-7.  It is illegal to set preemptee or preempter for a queue that is ultimately
 * mapped to engines 8-11.
 *
 * Also, PKO_REG_ENGINE_INFLIGHT must be at least 2 for any engine on which preemption is enabled.
 *
 * See the descriptions of PKO_MEM_QUEUE_PTRS for further explanation of queue programming.
 */
union cvmx_pko_reg_queue_preempt {
	u64 u64;
	struct cvmx_pko_reg_queue_preempt_s {
		u64 reserved_2_63 : 62;
		u64 preemptee : 1;
		u64 preempter : 1;
	} s;
	struct cvmx_pko_reg_queue_preempt_s cn52xx;
	struct cvmx_pko_reg_queue_preempt_s cn52xxp1;
	struct cvmx_pko_reg_queue_preempt_s cn56xx;
	struct cvmx_pko_reg_queue_preempt_s cn56xxp1;
	struct cvmx_pko_reg_queue_preempt_s cn61xx;
	struct cvmx_pko_reg_queue_preempt_s cn63xx;
	struct cvmx_pko_reg_queue_preempt_s cn63xxp1;
	struct cvmx_pko_reg_queue_preempt_s cn66xx;
	struct cvmx_pko_reg_queue_preempt_s cn68xx;
	struct cvmx_pko_reg_queue_preempt_s cn68xxp1;
	struct cvmx_pko_reg_queue_preempt_s cn70xx;
	struct cvmx_pko_reg_queue_preempt_s cn70xxp1;
	struct cvmx_pko_reg_queue_preempt_s cnf71xx;
};

typedef union cvmx_pko_reg_queue_preempt cvmx_pko_reg_queue_preempt_t;

/**
 * cvmx_pko_reg_queue_ptrs1
 *
 * Notes:
 * This CSR is used with PKO_MEM_QUEUE_PTRS and PKO_MEM_QUEUE_QOS to allow access to queues 128-255
 * and to allow up mapping of up to 16 queues per port.  When programming queues 128-255, the
 * programming sequence must first write PKO_REG_QUEUE_PTRS1 and then write PKO_MEM_QUEUE_PTRS or
 * PKO_MEM_QUEUE_QOS for each queue.
 * See the descriptions of PKO_MEM_QUEUE_PTRS and PKO_MEM_QUEUE_QOS for further explanation of queue
 * programming.
 */
union cvmx_pko_reg_queue_ptrs1 {
	u64 u64;
	struct cvmx_pko_reg_queue_ptrs1_s {
		u64 reserved_2_63 : 62;
		u64 idx3 : 1;
		u64 qid7 : 1;
	} s;
	struct cvmx_pko_reg_queue_ptrs1_s cn50xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn52xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn52xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s cn56xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn56xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s cn58xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn58xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s cn61xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn63xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn63xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s cn66xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn70xx;
	struct cvmx_pko_reg_queue_ptrs1_s cn70xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s cnf71xx;
};

typedef union cvmx_pko_reg_queue_ptrs1 cvmx_pko_reg_queue_ptrs1_t;

/**
 * cvmx_pko_reg_read_idx
 *
 * Notes:
 * Provides the read index during a CSR read operation to any of the CSRs that are physically stored
 * as memories.  The names of these CSRs begin with the prefix "PKO_MEM_".
 * IDX[7:0] is the read index.  INC[7:0] is an increment that is added to IDX[7:0] after any CSR read.
 * The intended use is to initially write this CSR such that IDX=0 and INC=1.  Then, the entire
 * contents of a CSR memory can be read with consecutive CSR read commands.
 */
union cvmx_pko_reg_read_idx {
	u64 u64;
	struct cvmx_pko_reg_read_idx_s {
		u64 reserved_16_63 : 48;
		u64 inc : 8;
		u64 index : 8;
	} s;
	struct cvmx_pko_reg_read_idx_s cn30xx;
	struct cvmx_pko_reg_read_idx_s cn31xx;
	struct cvmx_pko_reg_read_idx_s cn38xx;
	struct cvmx_pko_reg_read_idx_s cn38xxp2;
	struct cvmx_pko_reg_read_idx_s cn50xx;
	struct cvmx_pko_reg_read_idx_s cn52xx;
	struct cvmx_pko_reg_read_idx_s cn52xxp1;
	struct cvmx_pko_reg_read_idx_s cn56xx;
	struct cvmx_pko_reg_read_idx_s cn56xxp1;
	struct cvmx_pko_reg_read_idx_s cn58xx;
	struct cvmx_pko_reg_read_idx_s cn58xxp1;
	struct cvmx_pko_reg_read_idx_s cn61xx;
	struct cvmx_pko_reg_read_idx_s cn63xx;
	struct cvmx_pko_reg_read_idx_s cn63xxp1;
	struct cvmx_pko_reg_read_idx_s cn66xx;
	struct cvmx_pko_reg_read_idx_s cn68xx;
	struct cvmx_pko_reg_read_idx_s cn68xxp1;
	struct cvmx_pko_reg_read_idx_s cn70xx;
	struct cvmx_pko_reg_read_idx_s cn70xxp1;
	struct cvmx_pko_reg_read_idx_s cnf71xx;
};

typedef union cvmx_pko_reg_read_idx cvmx_pko_reg_read_idx_t;

/**
 * cvmx_pko_reg_throttle
 *
 * Notes:
 * This CSR is used with PKO_MEM_THROTTLE_PIPE and PKO_MEM_THROTTLE_INT.  INT_MASK corresponds to the
 * interfaces listed in the description for PKO_MEM_IPORT_PTRS[INT].  Set INT_MASK[N] to enable the
 * updating of PKO_MEM_THROTTLE_PIPE and PKO_MEM_THROTTLE_INT counts for packets destined for
 * interface N.  INT_MASK has no effect on the updates caused by CSR writes to PKO_MEM_THROTTLE_PIPE
 * and PKO_MEM_THROTTLE_INT.  Note that this does not disable the throttle logic, just the updating of
 * the interface counts.
 */
union cvmx_pko_reg_throttle {
	u64 u64;
	struct cvmx_pko_reg_throttle_s {
		u64 reserved_32_63 : 32;
		u64 int_mask : 32;
	} s;
	struct cvmx_pko_reg_throttle_s cn68xx;
	struct cvmx_pko_reg_throttle_s cn68xxp1;
};

typedef union cvmx_pko_reg_throttle cvmx_pko_reg_throttle_t;

/**
 * cvmx_pko_reg_timestamp
 *
 * Notes:
 * None.
 *
 */
union cvmx_pko_reg_timestamp {
	u64 u64;
	struct cvmx_pko_reg_timestamp_s {
		u64 reserved_4_63 : 60;
		u64 wqe_word : 4;
	} s;
	struct cvmx_pko_reg_timestamp_s cn61xx;
	struct cvmx_pko_reg_timestamp_s cn63xx;
	struct cvmx_pko_reg_timestamp_s cn63xxp1;
	struct cvmx_pko_reg_timestamp_s cn66xx;
	struct cvmx_pko_reg_timestamp_s cn68xx;
	struct cvmx_pko_reg_timestamp_s cn68xxp1;
	struct cvmx_pko_reg_timestamp_s cn70xx;
	struct cvmx_pko_reg_timestamp_s cn70xxp1;
	struct cvmx_pko_reg_timestamp_s cnf71xx;
};

typedef union cvmx_pko_reg_timestamp cvmx_pko_reg_timestamp_t;

/**
 * cvmx_pko_shaper_cfg
 */
union cvmx_pko_shaper_cfg {
	u64 u64;
	struct cvmx_pko_shaper_cfg_s {
		u64 reserved_2_63 : 62;
		u64 color_aware : 1;
		u64 red_send_as_yellow : 1;
	} s;
	struct cvmx_pko_shaper_cfg_s cn73xx;
	struct cvmx_pko_shaper_cfg_s cn78xx;
	struct cvmx_pko_shaper_cfg_s cn78xxp1;
	struct cvmx_pko_shaper_cfg_s cnf75xx;
};

typedef union cvmx_pko_shaper_cfg cvmx_pko_shaper_cfg_t;

/**
 * cvmx_pko_state_uid_in_use#_rd
 *
 * For diagnostic use only.
 *
 */
union cvmx_pko_state_uid_in_usex_rd {
	u64 u64;
	struct cvmx_pko_state_uid_in_usex_rd_s {
		u64 in_use : 64;
	} s;
	struct cvmx_pko_state_uid_in_usex_rd_s cn73xx;
	struct cvmx_pko_state_uid_in_usex_rd_s cn78xx;
	struct cvmx_pko_state_uid_in_usex_rd_s cn78xxp1;
	struct cvmx_pko_state_uid_in_usex_rd_s cnf75xx;
};

typedef union cvmx_pko_state_uid_in_usex_rd cvmx_pko_state_uid_in_usex_rd_t;

/**
 * cvmx_pko_status
 */
union cvmx_pko_status {
	u64 u64;
	struct cvmx_pko_status_s {
		u64 pko_rdy : 1;
		u64 reserved_24_62 : 39;
		u64 c2qlut_rdy : 1;
		u64 ppfi_rdy : 1;
		u64 iobp1_rdy : 1;
		u64 ncb_rdy : 1;
		u64 pse_rdy : 1;
		u64 pdm_rdy : 1;
		u64 peb_rdy : 1;
		u64 csi_rdy : 1;
		u64 reserved_5_15 : 11;
		u64 ncb_bist_status : 1;
		u64 c2qlut_bist_status : 1;
		u64 pdm_bist_status : 1;
		u64 peb_bist_status : 1;
		u64 pse_bist_status : 1;
	} s;
	struct cvmx_pko_status_cn73xx {
		u64 pko_rdy : 1;
		u64 reserved_62_24 : 39;
		u64 c2qlut_rdy : 1;
		u64 ppfi_rdy : 1;
		u64 iobp1_rdy : 1;
		u64 ncb_rdy : 1;
		u64 pse_rdy : 1;
		u64 pdm_rdy : 1;
		u64 peb_rdy : 1;
		u64 csi_rdy : 1;
		u64 reserved_15_5 : 11;
		u64 ncb_bist_status : 1;
		u64 c2qlut_bist_status : 1;
		u64 pdm_bist_status : 1;
		u64 peb_bist_status : 1;
		u64 pse_bist_status : 1;
	} cn73xx;
	struct cvmx_pko_status_cn73xx cn78xx;
	struct cvmx_pko_status_cn73xx cn78xxp1;
	struct cvmx_pko_status_cn73xx cnf75xx;
};

typedef union cvmx_pko_status cvmx_pko_status_t;

/**
 * cvmx_pko_txf#_pkt_cnt_rd
 */
union cvmx_pko_txfx_pkt_cnt_rd {
	u64 u64;
	struct cvmx_pko_txfx_pkt_cnt_rd_s {
		u64 reserved_8_63 : 56;
		u64 cnt : 8;
	} s;
	struct cvmx_pko_txfx_pkt_cnt_rd_s cn73xx;
	struct cvmx_pko_txfx_pkt_cnt_rd_s cn78xx;
	struct cvmx_pko_txfx_pkt_cnt_rd_s cn78xxp1;
	struct cvmx_pko_txfx_pkt_cnt_rd_s cnf75xx;
};

typedef union cvmx_pko_txfx_pkt_cnt_rd cvmx_pko_txfx_pkt_cnt_rd_t;

#endif
