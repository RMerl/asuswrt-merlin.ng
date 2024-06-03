/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef _CLASS_CSR_H_
#define _CLASS_CSR_H_

/*
 * @file class_csr.h.
 * class_csr - block containing all the classifier control and status register.
 * Mapped on CBUS and accessible from all PE's and ARM.
 */
#define CLASS_VERSION			(CLASS_CSR_BASE_ADDR + 0x000)
#define CLASS_TX_CTRL			(CLASS_CSR_BASE_ADDR + 0x004)
#define CLASS_INQ_PKTPTR		(CLASS_CSR_BASE_ADDR + 0x010)
/* (ddr_hdr_size[24:16], lmem_hdr_size[5:0]) */
#define CLASS_HDR_SIZE			(CLASS_CSR_BASE_ADDR + 0x014)
/* LMEM header size for the Classifier block.
 * Data in the LMEM is written from this offset.
 */
#define CLASS_HDR_SIZE_LMEM(off)	((off) & 0x3f)
/* DDR header size for the Classifier block.
 * Data in the DDR is written from this offset.
 */
#define CLASS_HDR_SIZE_DDR(off)		(((off) & 0x1ff) << 16)

/* DMEM address of first [15:0] and second [31:16] buffers on QB side. */
#define CLASS_PE0_QB_DM_ADDR0		(CLASS_CSR_BASE_ADDR + 0x020)
/* DMEM address of third [15:0] and fourth [31:16] buffers on QB side. */
#define CLASS_PE0_QB_DM_ADDR1		(CLASS_CSR_BASE_ADDR + 0x024)

/* DMEM address of first [15:0] and second [31:16] buffers on RO side. */
#define CLASS_PE0_RO_DM_ADDR0		(CLASS_CSR_BASE_ADDR + 0x060)
/* DMEM address of third [15:0] and fourth [31:16] buffers on RO side. */
#define CLASS_PE0_RO_DM_ADDR1		(CLASS_CSR_BASE_ADDR + 0x064)

/*
 * @name Class PE memory access. Allows external PE's and HOST to
 * read/write PMEM/DMEM memory ranges for each classifier PE.
 */
#define CLASS_MEM_ACCESS_ADDR		(CLASS_CSR_BASE_ADDR + 0x100)
/* Internal Memory Access Write Data [31:0] */
#define CLASS_MEM_ACCESS_WDATA		(CLASS_CSR_BASE_ADDR + 0x104)
/* Internal Memory Access Read Data [31:0] */
#define CLASS_MEM_ACCESS_RDATA		(CLASS_CSR_BASE_ADDR + 0x108)
#define CLASS_TM_INQ_ADDR		(CLASS_CSR_BASE_ADDR + 0x114)
#define CLASS_PE_STATUS			(CLASS_CSR_BASE_ADDR + 0x118)

#define CLASS_PE_SYS_CLK_RATIO		(CLASS_CSR_BASE_ADDR + 0x200)
#define CLASS_AFULL_THRES		(CLASS_CSR_BASE_ADDR + 0x204)
#define CLASS_GAP_BETWEEN_READS		(CLASS_CSR_BASE_ADDR + 0x208)
#define CLASS_MAX_BUF_CNT		(CLASS_CSR_BASE_ADDR + 0x20c)
#define CLASS_TSQ_FIFO_THRES		(CLASS_CSR_BASE_ADDR + 0x210)
#define CLASS_TSQ_MAX_CNT		(CLASS_CSR_BASE_ADDR + 0x214)
#define CLASS_IRAM_DATA_0		(CLASS_CSR_BASE_ADDR + 0x218)
#define CLASS_IRAM_DATA_1		(CLASS_CSR_BASE_ADDR + 0x21c)
#define CLASS_IRAM_DATA_2		(CLASS_CSR_BASE_ADDR + 0x220)
#define CLASS_IRAM_DATA_3		(CLASS_CSR_BASE_ADDR + 0x224)

#define CLASS_BUS_ACCESS_ADDR		(CLASS_CSR_BASE_ADDR + 0x228)
/* bit 23:0 of PE peripheral address are stored in CLASS_BUS_ACCESS_ADDR */
#define CLASS_BUS_ACCESS_ADDR_MASK	(0x0001FFFF)

#define CLASS_BUS_ACCESS_WDATA		(CLASS_CSR_BASE_ADDR + 0x22c)
#define CLASS_BUS_ACCESS_RDATA		(CLASS_CSR_BASE_ADDR + 0x230)

/*
 * (route_entry_size[9:0], route_hash_size[23:16]
 * (this is actually ln2(size)))
 */
#define CLASS_ROUTE_HASH_ENTRY_SIZE	(CLASS_CSR_BASE_ADDR + 0x234)
#define CLASS_ROUTE_ENTRY_SIZE(size)	 ((size) & 0x1ff)
#define CLASS_ROUTE_HASH_SIZE(hash_bits) (((hash_bits) & 0xff) << 16)

#define CLASS_ROUTE_TABLE_BASE		(CLASS_CSR_BASE_ADDR + 0x238)
#define CLASS_ROUTE_MULTI		(CLASS_CSR_BASE_ADDR + 0x23c)
#define CLASS_SMEM_OFFSET		(CLASS_CSR_BASE_ADDR + 0x240)
#define CLASS_LMEM_BUF_SIZE		(CLASS_CSR_BASE_ADDR + 0x244)
#define CLASS_VLAN_ID			(CLASS_CSR_BASE_ADDR + 0x248)
#define CLASS_BMU1_BUF_FREE		(CLASS_CSR_BASE_ADDR + 0x24c)
#define CLASS_USE_TMU_INQ		(CLASS_CSR_BASE_ADDR + 0x250)
#define CLASS_VLAN_ID1			(CLASS_CSR_BASE_ADDR + 0x254)

#define CLASS_BUS_ACCESS_BASE		(CLASS_CSR_BASE_ADDR + 0x258)
/* bit 31:24 of PE peripheral address are stored in CLASS_BUS_ACCESS_BASE */
#define CLASS_BUS_ACCESS_BASE_MASK	(0xFF000000)

#define CLASS_HIF_PARSE			(CLASS_CSR_BASE_ADDR + 0x25c)

#define CLASS_HOST_PE0_GP		(CLASS_CSR_BASE_ADDR + 0x260)
#define CLASS_PE0_GP			(CLASS_CSR_BASE_ADDR + 0x264)
#define CLASS_HOST_PE1_GP		(CLASS_CSR_BASE_ADDR + 0x268)
#define CLASS_PE1_GP			(CLASS_CSR_BASE_ADDR + 0x26c)
#define CLASS_HOST_PE2_GP		(CLASS_CSR_BASE_ADDR + 0x270)
#define CLASS_PE2_GP			(CLASS_CSR_BASE_ADDR + 0x274)
#define CLASS_HOST_PE3_GP		(CLASS_CSR_BASE_ADDR + 0x278)
#define CLASS_PE3_GP			(CLASS_CSR_BASE_ADDR + 0x27c)
#define CLASS_HOST_PE4_GP		(CLASS_CSR_BASE_ADDR + 0x280)
#define CLASS_PE4_GP			(CLASS_CSR_BASE_ADDR + 0x284)
#define CLASS_HOST_PE5_GP		(CLASS_CSR_BASE_ADDR + 0x288)
#define CLASS_PE5_GP			(CLASS_CSR_BASE_ADDR + 0x28c)

#define CLASS_PE_INT_SRC		(CLASS_CSR_BASE_ADDR + 0x290)
#define CLASS_PE_INT_ENABLE		(CLASS_CSR_BASE_ADDR + 0x294)

#define CLASS_TPID0_TPID1		(CLASS_CSR_BASE_ADDR + 0x298)
#define CLASS_TPID2			(CLASS_CSR_BASE_ADDR + 0x29c)

#define CLASS_L4_CHKSUM_ADDR		(CLASS_CSR_BASE_ADDR + 0x2a0)

#define CLASS_PE0_DEBUG			(CLASS_CSR_BASE_ADDR + 0x2a4)
#define CLASS_PE1_DEBUG			(CLASS_CSR_BASE_ADDR + 0x2a8)
#define CLASS_PE2_DEBUG			(CLASS_CSR_BASE_ADDR + 0x2ac)
#define CLASS_PE3_DEBUG			(CLASS_CSR_BASE_ADDR + 0x2b0)
#define CLASS_PE4_DEBUG			(CLASS_CSR_BASE_ADDR + 0x2b4)
#define CLASS_PE5_DEBUG			(CLASS_CSR_BASE_ADDR + 0x2b8)

#define CLASS_STATE			(CLASS_CSR_BASE_ADDR + 0x2bc)
#define CLASS_AXI_CTRL			(CLASS_CSR_BASE_ADDR + 0x2d0)

/* CLASS defines */
#define CLASS_PBUF_SIZE			0x100	/* Fixed by hardware */
#define CLASS_PBUF_HEADER_OFFSET	0x80	/* Can be configured */

#define CLASS_PBUF0_BASE_ADDR		0x000	/* Can be configured */
/* Can be configured */
#define CLASS_PBUF1_BASE_ADDR	(CLASS_PBUF0_BASE_ADDR + CLASS_PBUF_SIZE)
/* Can be configured */
#define CLASS_PBUF2_BASE_ADDR	(CLASS_PBUF1_BASE_ADDR + CLASS_PBUF_SIZE)
/* Can be configured */
#define CLASS_PBUF3_BASE_ADDR	(CLASS_PBUF2_BASE_ADDR + CLASS_PBUF_SIZE)

#define CLASS_PBUF0_HEADER_BASE_ADDR	(CLASS_PBUF0_BASE_ADDR +\
						CLASS_PBUF_HEADER_OFFSET)
#define CLASS_PBUF1_HEADER_BASE_ADDR	(CLASS_PBUF1_BASE_ADDR +\
						CLASS_PBUF_HEADER_OFFSET)
#define CLASS_PBUF2_HEADER_BASE_ADDR	(CLASS_PBUF2_BASE_ADDR +\
						CLASS_PBUF_HEADER_OFFSET)
#define CLASS_PBUF3_HEADER_BASE_ADDR	(CLASS_PBUF3_BASE_ADDR +\
						CLASS_PBUF_HEADER_OFFSET)

#define CLASS_PE0_RO_DM_ADDR0_VAL	((CLASS_PBUF1_BASE_ADDR << 16) |\
						CLASS_PBUF0_BASE_ADDR)
#define CLASS_PE0_RO_DM_ADDR1_VAL	((CLASS_PBUF3_BASE_ADDR << 16) |\
						CLASS_PBUF2_BASE_ADDR)

#define CLASS_PE0_QB_DM_ADDR0_VAL	((CLASS_PBUF1_HEADER_BASE_ADDR << 16)\
						| CLASS_PBUF0_HEADER_BASE_ADDR)
#define CLASS_PE0_QB_DM_ADDR1_VAL	((CLASS_PBUF3_HEADER_BASE_ADDR << 16)\
						| CLASS_PBUF2_HEADER_BASE_ADDR)

#define CLASS_ROUTE_SIZE		128
#define CLASS_ROUTE_HASH_BITS		20
#define CLASS_ROUTE_HASH_MASK		(BIT(CLASS_ROUTE_HASH_BITS) - 1)

#define TWO_LEVEL_ROUTE		BIT(0)
#define PHYNO_IN_HASH		BIT(1)
#define HW_ROUTE_FETCH		BIT(3)
#define HW_BRIDGE_FETCH		BIT(5)
#define IP_ALIGNED		BIT(6)
#define ARC_HIT_CHECK_EN	BIT(7)
#define CLASS_TOE		BIT(11)
#define HASH_CRC_PORT		BIT(12)
#define HASH_CRC_IP		BIT(13)
#define HASH_CRC_PORT_IP	GENMASK(13, 12)
#define QB2BUS_LE		BIT(15)

#define	TCP_CHKSUM_DROP		BIT(0)
#define	UDP_CHKSUM_DROP		BIT(1)
#define	IPV4_CHKSUM_DROP	BIT(9)

struct class_cfg {
	u32 route_table_baseaddr;
	u32 route_table_hash_bits;
};

#endif /* _CLASS_CSR_H_ */
