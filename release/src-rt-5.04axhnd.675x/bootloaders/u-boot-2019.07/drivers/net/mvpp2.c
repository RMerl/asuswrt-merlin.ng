/*
 * Driver for Marvell PPv2 network controller for Armada 375 SoC.
 *
 * Copyright (C) 2014 Marvell
 *
 * Marcin Wojtas <mw@semihalf.com>
 *
 * U-Boot version:
 * Copyright (C) 2016-2017 Stefan Roese <sr@denx.de>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <net.h>
#include <netdev.h>
#include <config.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <phy.h>
#include <miiphy.h>
#include <watchdog.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/compat.h>
#include <linux/mbus.h>
#include <asm-generic/gpio.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

#define __verify_pcpu_ptr(ptr)						\
do {									\
	const void __percpu *__vpp_verify = (typeof((ptr) + 0))NULL;	\
	(void)__vpp_verify;						\
} while (0)

#define VERIFY_PERCPU_PTR(__p)						\
({									\
	__verify_pcpu_ptr(__p);						\
	(typeof(*(__p)) __kernel __force *)(__p);			\
})

#define per_cpu_ptr(ptr, cpu)	({ (void)(cpu); VERIFY_PERCPU_PTR(ptr); })
#define smp_processor_id()	0
#define num_present_cpus()	1
#define for_each_present_cpu(cpu)			\
	for ((cpu) = 0; (cpu) < 1; (cpu)++)

#define NET_SKB_PAD	max(32, MVPP2_CPU_D_CACHE_LINE_SIZE)

#define CONFIG_NR_CPUS		1

/* 2(HW hdr) 14(MAC hdr) 4(CRC) 32(extra for cache prefetch) */
#define WRAP			(2 + ETH_HLEN + 4 + 32)
#define MTU			1500
#define RX_BUFFER_SIZE		(ALIGN(MTU + WRAP, ARCH_DMA_MINALIGN))

#define MVPP2_SMI_TIMEOUT			10000

/* RX Fifo Registers */
#define MVPP2_RX_DATA_FIFO_SIZE_REG(port)	(0x00 + 4 * (port))
#define MVPP2_RX_ATTR_FIFO_SIZE_REG(port)	(0x20 + 4 * (port))
#define MVPP2_RX_MIN_PKT_SIZE_REG		0x60
#define MVPP2_RX_FIFO_INIT_REG			0x64

/* RX DMA Top Registers */
#define MVPP2_RX_CTRL_REG(port)			(0x140 + 4 * (port))
#define     MVPP2_RX_LOW_LATENCY_PKT_SIZE(s)	(((s) & 0xfff) << 16)
#define     MVPP2_RX_USE_PSEUDO_FOR_CSUM_MASK	BIT(31)
#define MVPP2_POOL_BUF_SIZE_REG(pool)		(0x180 + 4 * (pool))
#define     MVPP2_POOL_BUF_SIZE_OFFSET		5
#define MVPP2_RXQ_CONFIG_REG(rxq)		(0x800 + 4 * (rxq))
#define     MVPP2_SNOOP_PKT_SIZE_MASK		0x1ff
#define     MVPP2_SNOOP_BUF_HDR_MASK		BIT(9)
#define     MVPP2_RXQ_POOL_SHORT_OFFS		20
#define     MVPP21_RXQ_POOL_SHORT_MASK		0x700000
#define     MVPP22_RXQ_POOL_SHORT_MASK		0xf00000
#define     MVPP2_RXQ_POOL_LONG_OFFS		24
#define     MVPP21_RXQ_POOL_LONG_MASK		0x7000000
#define     MVPP22_RXQ_POOL_LONG_MASK		0xf000000
#define     MVPP2_RXQ_PACKET_OFFSET_OFFS	28
#define     MVPP2_RXQ_PACKET_OFFSET_MASK	0x70000000
#define     MVPP2_RXQ_DISABLE_MASK		BIT(31)

/* Parser Registers */
#define MVPP2_PRS_INIT_LOOKUP_REG		0x1000
#define     MVPP2_PRS_PORT_LU_MAX		0xf
#define     MVPP2_PRS_PORT_LU_MASK(port)	(0xff << ((port) * 4))
#define     MVPP2_PRS_PORT_LU_VAL(port, val)	((val) << ((port) * 4))
#define MVPP2_PRS_INIT_OFFS_REG(port)		(0x1004 + ((port) & 4))
#define     MVPP2_PRS_INIT_OFF_MASK(port)	(0x3f << (((port) % 4) * 8))
#define     MVPP2_PRS_INIT_OFF_VAL(port, val)	((val) << (((port) % 4) * 8))
#define MVPP2_PRS_MAX_LOOP_REG(port)		(0x100c + ((port) & 4))
#define     MVPP2_PRS_MAX_LOOP_MASK(port)	(0xff << (((port) % 4) * 8))
#define     MVPP2_PRS_MAX_LOOP_VAL(port, val)	((val) << (((port) % 4) * 8))
#define MVPP2_PRS_TCAM_IDX_REG			0x1100
#define MVPP2_PRS_TCAM_DATA_REG(idx)		(0x1104 + (idx) * 4)
#define     MVPP2_PRS_TCAM_INV_MASK		BIT(31)
#define MVPP2_PRS_SRAM_IDX_REG			0x1200
#define MVPP2_PRS_SRAM_DATA_REG(idx)		(0x1204 + (idx) * 4)
#define MVPP2_PRS_TCAM_CTRL_REG			0x1230
#define     MVPP2_PRS_TCAM_EN_MASK		BIT(0)

/* Classifier Registers */
#define MVPP2_CLS_MODE_REG			0x1800
#define     MVPP2_CLS_MODE_ACTIVE_MASK		BIT(0)
#define MVPP2_CLS_PORT_WAY_REG			0x1810
#define     MVPP2_CLS_PORT_WAY_MASK(port)	(1 << (port))
#define MVPP2_CLS_LKP_INDEX_REG			0x1814
#define     MVPP2_CLS_LKP_INDEX_WAY_OFFS	6
#define MVPP2_CLS_LKP_TBL_REG			0x1818
#define     MVPP2_CLS_LKP_TBL_RXQ_MASK		0xff
#define     MVPP2_CLS_LKP_TBL_LOOKUP_EN_MASK	BIT(25)
#define MVPP2_CLS_FLOW_INDEX_REG		0x1820
#define MVPP2_CLS_FLOW_TBL0_REG			0x1824
#define MVPP2_CLS_FLOW_TBL1_REG			0x1828
#define MVPP2_CLS_FLOW_TBL2_REG			0x182c
#define MVPP2_CLS_OVERSIZE_RXQ_LOW_REG(port)	(0x1980 + ((port) * 4))
#define     MVPP2_CLS_OVERSIZE_RXQ_LOW_BITS	3
#define     MVPP2_CLS_OVERSIZE_RXQ_LOW_MASK	0x7
#define MVPP2_CLS_SWFWD_P2HQ_REG(port)		(0x19b0 + ((port) * 4))
#define MVPP2_CLS_SWFWD_PCTRL_REG		0x19d0
#define     MVPP2_CLS_SWFWD_PCTRL_MASK(port)	(1 << (port))

/* Descriptor Manager Top Registers */
#define MVPP2_RXQ_NUM_REG			0x2040
#define MVPP2_RXQ_DESC_ADDR_REG			0x2044
#define     MVPP22_DESC_ADDR_OFFS		8
#define MVPP2_RXQ_DESC_SIZE_REG			0x2048
#define     MVPP2_RXQ_DESC_SIZE_MASK		0x3ff0
#define MVPP2_RXQ_STATUS_UPDATE_REG(rxq)	(0x3000 + 4 * (rxq))
#define     MVPP2_RXQ_NUM_PROCESSED_OFFSET	0
#define     MVPP2_RXQ_NUM_NEW_OFFSET		16
#define MVPP2_RXQ_STATUS_REG(rxq)		(0x3400 + 4 * (rxq))
#define     MVPP2_RXQ_OCCUPIED_MASK		0x3fff
#define     MVPP2_RXQ_NON_OCCUPIED_OFFSET	16
#define     MVPP2_RXQ_NON_OCCUPIED_MASK		0x3fff0000
#define MVPP2_RXQ_THRESH_REG			0x204c
#define     MVPP2_OCCUPIED_THRESH_OFFSET	0
#define     MVPP2_OCCUPIED_THRESH_MASK		0x3fff
#define MVPP2_RXQ_INDEX_REG			0x2050
#define MVPP2_TXQ_NUM_REG			0x2080
#define MVPP2_TXQ_DESC_ADDR_REG			0x2084
#define MVPP2_TXQ_DESC_SIZE_REG			0x2088
#define     MVPP2_TXQ_DESC_SIZE_MASK		0x3ff0
#define MVPP2_AGGR_TXQ_UPDATE_REG		0x2090
#define MVPP2_TXQ_THRESH_REG			0x2094
#define     MVPP2_TRANSMITTED_THRESH_OFFSET	16
#define     MVPP2_TRANSMITTED_THRESH_MASK	0x3fff0000
#define MVPP2_TXQ_INDEX_REG			0x2098
#define MVPP2_TXQ_PREF_BUF_REG			0x209c
#define     MVPP2_PREF_BUF_PTR(desc)		((desc) & 0xfff)
#define     MVPP2_PREF_BUF_SIZE_4		(BIT(12) | BIT(13))
#define     MVPP2_PREF_BUF_SIZE_16		(BIT(12) | BIT(14))
#define     MVPP2_PREF_BUF_THRESH(val)		((val) << 17)
#define     MVPP2_TXQ_DRAIN_EN_MASK		BIT(31)
#define MVPP2_TXQ_PENDING_REG			0x20a0
#define     MVPP2_TXQ_PENDING_MASK		0x3fff
#define MVPP2_TXQ_INT_STATUS_REG		0x20a4
#define MVPP2_TXQ_SENT_REG(txq)			(0x3c00 + 4 * (txq))
#define     MVPP2_TRANSMITTED_COUNT_OFFSET	16
#define     MVPP2_TRANSMITTED_COUNT_MASK	0x3fff0000
#define MVPP2_TXQ_RSVD_REQ_REG			0x20b0
#define     MVPP2_TXQ_RSVD_REQ_Q_OFFSET		16
#define MVPP2_TXQ_RSVD_RSLT_REG			0x20b4
#define     MVPP2_TXQ_RSVD_RSLT_MASK		0x3fff
#define MVPP2_TXQ_RSVD_CLR_REG			0x20b8
#define     MVPP2_TXQ_RSVD_CLR_OFFSET		16
#define MVPP2_AGGR_TXQ_DESC_ADDR_REG(cpu)	(0x2100 + 4 * (cpu))
#define     MVPP22_AGGR_TXQ_DESC_ADDR_OFFS	8
#define MVPP2_AGGR_TXQ_DESC_SIZE_REG(cpu)	(0x2140 + 4 * (cpu))
#define     MVPP2_AGGR_TXQ_DESC_SIZE_MASK	0x3ff0
#define MVPP2_AGGR_TXQ_STATUS_REG(cpu)		(0x2180 + 4 * (cpu))
#define     MVPP2_AGGR_TXQ_PENDING_MASK		0x3fff
#define MVPP2_AGGR_TXQ_INDEX_REG(cpu)		(0x21c0 + 4 * (cpu))

/* MBUS bridge registers */
#define MVPP2_WIN_BASE(w)			(0x4000 + ((w) << 2))
#define MVPP2_WIN_SIZE(w)			(0x4020 + ((w) << 2))
#define MVPP2_WIN_REMAP(w)			(0x4040 + ((w) << 2))
#define MVPP2_BASE_ADDR_ENABLE			0x4060

/* AXI Bridge Registers */
#define MVPP22_AXI_BM_WR_ATTR_REG		0x4100
#define MVPP22_AXI_BM_RD_ATTR_REG		0x4104
#define MVPP22_AXI_AGGRQ_DESCR_RD_ATTR_REG	0x4110
#define MVPP22_AXI_TXQ_DESCR_WR_ATTR_REG	0x4114
#define MVPP22_AXI_TXQ_DESCR_RD_ATTR_REG	0x4118
#define MVPP22_AXI_RXQ_DESCR_WR_ATTR_REG	0x411c
#define MVPP22_AXI_RX_DATA_WR_ATTR_REG		0x4120
#define MVPP22_AXI_TX_DATA_RD_ATTR_REG		0x4130
#define MVPP22_AXI_RD_NORMAL_CODE_REG		0x4150
#define MVPP22_AXI_RD_SNOOP_CODE_REG		0x4154
#define MVPP22_AXI_WR_NORMAL_CODE_REG		0x4160
#define MVPP22_AXI_WR_SNOOP_CODE_REG		0x4164

/* Values for AXI Bridge registers */
#define MVPP22_AXI_ATTR_CACHE_OFFS		0
#define MVPP22_AXI_ATTR_DOMAIN_OFFS		12

#define MVPP22_AXI_CODE_CACHE_OFFS		0
#define MVPP22_AXI_CODE_DOMAIN_OFFS		4

#define MVPP22_AXI_CODE_CACHE_NON_CACHE		0x3
#define MVPP22_AXI_CODE_CACHE_WR_CACHE		0x7
#define MVPP22_AXI_CODE_CACHE_RD_CACHE		0xb

#define MVPP22_AXI_CODE_DOMAIN_OUTER_DOM	2
#define MVPP22_AXI_CODE_DOMAIN_SYSTEM		3

/* Interrupt Cause and Mask registers */
#define MVPP2_ISR_RX_THRESHOLD_REG(rxq)		(0x5200 + 4 * (rxq))
#define MVPP21_ISR_RXQ_GROUP_REG(rxq)		(0x5400 + 4 * (rxq))

#define MVPP22_ISR_RXQ_GROUP_INDEX_REG          0x5400
#define MVPP22_ISR_RXQ_GROUP_INDEX_SUBGROUP_MASK 0xf
#define MVPP22_ISR_RXQ_GROUP_INDEX_GROUP_MASK   0x380
#define MVPP22_ISR_RXQ_GROUP_INDEX_GROUP_OFFSET 7

#define MVPP22_ISR_RXQ_GROUP_INDEX_SUBGROUP_MASK 0xf
#define MVPP22_ISR_RXQ_GROUP_INDEX_GROUP_MASK   0x380

#define MVPP22_ISR_RXQ_SUB_GROUP_CONFIG_REG     0x5404
#define MVPP22_ISR_RXQ_SUB_GROUP_STARTQ_MASK    0x1f
#define MVPP22_ISR_RXQ_SUB_GROUP_SIZE_MASK      0xf00
#define MVPP22_ISR_RXQ_SUB_GROUP_SIZE_OFFSET    8

#define MVPP2_ISR_ENABLE_REG(port)		(0x5420 + 4 * (port))
#define     MVPP2_ISR_ENABLE_INTERRUPT(mask)	((mask) & 0xffff)
#define     MVPP2_ISR_DISABLE_INTERRUPT(mask)	(((mask) << 16) & 0xffff0000)
#define MVPP2_ISR_RX_TX_CAUSE_REG(port)		(0x5480 + 4 * (port))
#define     MVPP2_CAUSE_RXQ_OCCUP_DESC_ALL_MASK	0xffff
#define     MVPP2_CAUSE_TXQ_OCCUP_DESC_ALL_MASK	0xff0000
#define     MVPP2_CAUSE_RX_FIFO_OVERRUN_MASK	BIT(24)
#define     MVPP2_CAUSE_FCS_ERR_MASK		BIT(25)
#define     MVPP2_CAUSE_TX_FIFO_UNDERRUN_MASK	BIT(26)
#define     MVPP2_CAUSE_TX_EXCEPTION_SUM_MASK	BIT(29)
#define     MVPP2_CAUSE_RX_EXCEPTION_SUM_MASK	BIT(30)
#define     MVPP2_CAUSE_MISC_SUM_MASK		BIT(31)
#define MVPP2_ISR_RX_TX_MASK_REG(port)		(0x54a0 + 4 * (port))
#define MVPP2_ISR_PON_RX_TX_MASK_REG		0x54bc
#define     MVPP2_PON_CAUSE_RXQ_OCCUP_DESC_ALL_MASK	0xffff
#define     MVPP2_PON_CAUSE_TXP_OCCUP_DESC_ALL_MASK	0x3fc00000
#define     MVPP2_PON_CAUSE_MISC_SUM_MASK		BIT(31)
#define MVPP2_ISR_MISC_CAUSE_REG		0x55b0

/* Buffer Manager registers */
#define MVPP2_BM_POOL_BASE_REG(pool)		(0x6000 + ((pool) * 4))
#define     MVPP2_BM_POOL_BASE_ADDR_MASK	0xfffff80
#define MVPP2_BM_POOL_SIZE_REG(pool)		(0x6040 + ((pool) * 4))
#define     MVPP2_BM_POOL_SIZE_MASK		0xfff0
#define MVPP2_BM_POOL_READ_PTR_REG(pool)	(0x6080 + ((pool) * 4))
#define     MVPP2_BM_POOL_GET_READ_PTR_MASK	0xfff0
#define MVPP2_BM_POOL_PTRS_NUM_REG(pool)	(0x60c0 + ((pool) * 4))
#define     MVPP2_BM_POOL_PTRS_NUM_MASK		0xfff0
#define MVPP2_BM_BPPI_READ_PTR_REG(pool)	(0x6100 + ((pool) * 4))
#define MVPP2_BM_BPPI_PTRS_NUM_REG(pool)	(0x6140 + ((pool) * 4))
#define     MVPP2_BM_BPPI_PTR_NUM_MASK		0x7ff
#define     MVPP2_BM_BPPI_PREFETCH_FULL_MASK	BIT(16)
#define MVPP2_BM_POOL_CTRL_REG(pool)		(0x6200 + ((pool) * 4))
#define     MVPP2_BM_START_MASK			BIT(0)
#define     MVPP2_BM_STOP_MASK			BIT(1)
#define     MVPP2_BM_STATE_MASK			BIT(4)
#define     MVPP2_BM_LOW_THRESH_OFFS		8
#define     MVPP2_BM_LOW_THRESH_MASK		0x7f00
#define     MVPP2_BM_LOW_THRESH_VALUE(val)	((val) << \
						MVPP2_BM_LOW_THRESH_OFFS)
#define     MVPP2_BM_HIGH_THRESH_OFFS		16
#define     MVPP2_BM_HIGH_THRESH_MASK		0x7f0000
#define     MVPP2_BM_HIGH_THRESH_VALUE(val)	((val) << \
						MVPP2_BM_HIGH_THRESH_OFFS)
#define MVPP2_BM_INTR_CAUSE_REG(pool)		(0x6240 + ((pool) * 4))
#define     MVPP2_BM_RELEASED_DELAY_MASK	BIT(0)
#define     MVPP2_BM_ALLOC_FAILED_MASK		BIT(1)
#define     MVPP2_BM_BPPE_EMPTY_MASK		BIT(2)
#define     MVPP2_BM_BPPE_FULL_MASK		BIT(3)
#define     MVPP2_BM_AVAILABLE_BP_LOW_MASK	BIT(4)
#define MVPP2_BM_INTR_MASK_REG(pool)		(0x6280 + ((pool) * 4))
#define MVPP2_BM_PHY_ALLOC_REG(pool)		(0x6400 + ((pool) * 4))
#define     MVPP2_BM_PHY_ALLOC_GRNTD_MASK	BIT(0)
#define MVPP2_BM_VIRT_ALLOC_REG			0x6440
#define MVPP2_BM_ADDR_HIGH_ALLOC		0x6444
#define     MVPP2_BM_ADDR_HIGH_PHYS_MASK	0xff
#define     MVPP2_BM_ADDR_HIGH_VIRT_MASK	0xff00
#define     MVPP2_BM_ADDR_HIGH_VIRT_SHIFT	8
#define MVPP2_BM_PHY_RLS_REG(pool)		(0x6480 + ((pool) * 4))
#define     MVPP2_BM_PHY_RLS_MC_BUFF_MASK	BIT(0)
#define     MVPP2_BM_PHY_RLS_PRIO_EN_MASK	BIT(1)
#define     MVPP2_BM_PHY_RLS_GRNTD_MASK		BIT(2)
#define MVPP2_BM_VIRT_RLS_REG			0x64c0
#define MVPP21_BM_MC_RLS_REG			0x64c4
#define     MVPP2_BM_MC_ID_MASK			0xfff
#define     MVPP2_BM_FORCE_RELEASE_MASK		BIT(12)
#define MVPP22_BM_ADDR_HIGH_RLS_REG		0x64c4
#define     MVPP22_BM_ADDR_HIGH_PHYS_RLS_MASK	0xff
#define	    MVPP22_BM_ADDR_HIGH_VIRT_RLS_MASK	0xff00
#define     MVPP22_BM_ADDR_HIGH_VIRT_RLS_SHIFT	8
#define MVPP22_BM_MC_RLS_REG			0x64d4
#define MVPP22_BM_POOL_BASE_HIGH_REG		0x6310
#define MVPP22_BM_POOL_BASE_HIGH_MASK		0xff

/* TX Scheduler registers */
#define MVPP2_TXP_SCHED_PORT_INDEX_REG		0x8000
#define MVPP2_TXP_SCHED_Q_CMD_REG		0x8004
#define     MVPP2_TXP_SCHED_ENQ_MASK		0xff
#define     MVPP2_TXP_SCHED_DISQ_OFFSET		8
#define MVPP2_TXP_SCHED_CMD_1_REG		0x8010
#define MVPP2_TXP_SCHED_PERIOD_REG		0x8018
#define MVPP2_TXP_SCHED_MTU_REG			0x801c
#define     MVPP2_TXP_MTU_MAX			0x7FFFF
#define MVPP2_TXP_SCHED_REFILL_REG		0x8020
#define     MVPP2_TXP_REFILL_TOKENS_ALL_MASK	0x7ffff
#define     MVPP2_TXP_REFILL_PERIOD_ALL_MASK	0x3ff00000
#define     MVPP2_TXP_REFILL_PERIOD_MASK(v)	((v) << 20)
#define MVPP2_TXP_SCHED_TOKEN_SIZE_REG		0x8024
#define     MVPP2_TXP_TOKEN_SIZE_MAX		0xffffffff
#define MVPP2_TXQ_SCHED_REFILL_REG(q)		(0x8040 + ((q) << 2))
#define     MVPP2_TXQ_REFILL_TOKENS_ALL_MASK	0x7ffff
#define     MVPP2_TXQ_REFILL_PERIOD_ALL_MASK	0x3ff00000
#define     MVPP2_TXQ_REFILL_PERIOD_MASK(v)	((v) << 20)
#define MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(q)	(0x8060 + ((q) << 2))
#define     MVPP2_TXQ_TOKEN_SIZE_MAX		0x7fffffff
#define MVPP2_TXQ_SCHED_TOKEN_CNTR_REG(q)	(0x8080 + ((q) << 2))
#define     MVPP2_TXQ_TOKEN_CNTR_MAX		0xffffffff

/* TX general registers */
#define MVPP2_TX_SNOOP_REG			0x8800
#define MVPP2_TX_PORT_FLUSH_REG			0x8810
#define     MVPP2_TX_PORT_FLUSH_MASK(port)	(1 << (port))

/* LMS registers */
#define MVPP2_SRC_ADDR_MIDDLE			0x24
#define MVPP2_SRC_ADDR_HIGH			0x28
#define MVPP2_PHY_AN_CFG0_REG			0x34
#define     MVPP2_PHY_AN_STOP_SMI0_MASK		BIT(7)
#define MVPP2_MNG_EXTENDED_GLOBAL_CTRL_REG	0x305c
#define     MVPP2_EXT_GLOBAL_CTRL_DEFAULT	0x27

/* Per-port registers */
#define MVPP2_GMAC_CTRL_0_REG			0x0
#define      MVPP2_GMAC_PORT_EN_MASK		BIT(0)
#define      MVPP2_GMAC_PORT_TYPE_MASK		BIT(1)
#define      MVPP2_GMAC_MAX_RX_SIZE_OFFS	2
#define      MVPP2_GMAC_MAX_RX_SIZE_MASK	0x7ffc
#define      MVPP2_GMAC_MIB_CNTR_EN_MASK	BIT(15)
#define MVPP2_GMAC_CTRL_1_REG			0x4
#define      MVPP2_GMAC_PERIODIC_XON_EN_MASK	BIT(1)
#define      MVPP2_GMAC_GMII_LB_EN_MASK		BIT(5)
#define      MVPP2_GMAC_PCS_LB_EN_BIT		6
#define      MVPP2_GMAC_PCS_LB_EN_MASK		BIT(6)
#define      MVPP2_GMAC_SA_LOW_OFFS		7
#define MVPP2_GMAC_CTRL_2_REG			0x8
#define      MVPP2_GMAC_INBAND_AN_MASK		BIT(0)
#define      MVPP2_GMAC_SGMII_MODE_MASK		BIT(0)
#define      MVPP2_GMAC_PCS_ENABLE_MASK		BIT(3)
#define      MVPP2_GMAC_PORT_RGMII_MASK		BIT(4)
#define      MVPP2_GMAC_PORT_DIS_PADING_MASK	BIT(5)
#define      MVPP2_GMAC_PORT_RESET_MASK		BIT(6)
#define      MVPP2_GMAC_CLK_125_BYPS_EN_MASK	BIT(9)
#define MVPP2_GMAC_AUTONEG_CONFIG		0xc
#define      MVPP2_GMAC_FORCE_LINK_DOWN		BIT(0)
#define      MVPP2_GMAC_FORCE_LINK_PASS		BIT(1)
#define      MVPP2_GMAC_EN_PCS_AN		BIT(2)
#define      MVPP2_GMAC_AN_BYPASS_EN		BIT(3)
#define      MVPP2_GMAC_CONFIG_MII_SPEED	BIT(5)
#define      MVPP2_GMAC_CONFIG_GMII_SPEED	BIT(6)
#define      MVPP2_GMAC_AN_SPEED_EN		BIT(7)
#define      MVPP2_GMAC_FC_ADV_EN		BIT(9)
#define      MVPP2_GMAC_EN_FC_AN		BIT(11)
#define      MVPP2_GMAC_CONFIG_FULL_DUPLEX	BIT(12)
#define      MVPP2_GMAC_AN_DUPLEX_EN		BIT(13)
#define      MVPP2_GMAC_CHOOSE_SAMPLE_TX_CONFIG	BIT(15)
#define MVPP2_GMAC_PORT_FIFO_CFG_1_REG		0x1c
#define      MVPP2_GMAC_TX_FIFO_MIN_TH_OFFS	6
#define      MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK	0x1fc0
#define      MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(v)	(((v) << 6) & \
					MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK)
#define MVPP2_GMAC_CTRL_4_REG			0x90
#define      MVPP2_GMAC_CTRL4_EXT_PIN_GMII_SEL_MASK	BIT(0)
#define      MVPP2_GMAC_CTRL4_DP_CLK_SEL_MASK	BIT(5)
#define      MVPP2_GMAC_CTRL4_SYNC_BYPASS_MASK	BIT(6)
#define      MVPP2_GMAC_CTRL4_QSGMII_BYPASS_ACTIVE_MASK	BIT(7)

/*
 * Per-port XGMAC registers. PPv2.2 only, only for GOP port 0,
 * relative to port->base.
 */

/* Port Mac Control0 */
#define MVPP22_XLG_CTRL0_REG			0x100
#define      MVPP22_XLG_PORT_EN			BIT(0)
#define      MVPP22_XLG_MAC_RESETN		BIT(1)
#define      MVPP22_XLG_RX_FC_EN		BIT(7)
#define      MVPP22_XLG_MIBCNT_DIS		BIT(13)
/* Port Mac Control1 */
#define MVPP22_XLG_CTRL1_REG			0x104
#define      MVPP22_XLG_MAX_RX_SIZE_OFFS	0
#define      MVPP22_XLG_MAX_RX_SIZE_MASK	0x1fff
/* Port Interrupt Mask */
#define MVPP22_XLG_INTERRUPT_MASK_REG		0x118
#define      MVPP22_XLG_INTERRUPT_LINK_CHANGE	BIT(1)
/* Port Mac Control3 */
#define MVPP22_XLG_CTRL3_REG			0x11c
#define      MVPP22_XLG_CTRL3_MACMODESELECT_MASK	(7 << 13)
#define      MVPP22_XLG_CTRL3_MACMODESELECT_GMAC	(0 << 13)
#define      MVPP22_XLG_CTRL3_MACMODESELECT_10GMAC	(1 << 13)
/* Port Mac Control4 */
#define MVPP22_XLG_CTRL4_REG			0x184
#define      MVPP22_XLG_FORWARD_802_3X_FC_EN	BIT(5)
#define      MVPP22_XLG_FORWARD_PFC_EN		BIT(6)
#define      MVPP22_XLG_MODE_DMA_1G		BIT(12)
#define      MVPP22_XLG_EN_IDLE_CHECK_FOR_LINK	BIT(14)

/* XPCS registers */

/* Global Configuration 0 */
#define MVPP22_XPCS_GLOBAL_CFG_0_REG		0x0
#define      MVPP22_XPCS_PCSRESET		BIT(0)
#define      MVPP22_XPCS_PCSMODE_OFFS		3
#define      MVPP22_XPCS_PCSMODE_MASK		(0x3 << \
						 MVPP22_XPCS_PCSMODE_OFFS)
#define      MVPP22_XPCS_LANEACTIVE_OFFS	5
#define      MVPP22_XPCS_LANEACTIVE_MASK	(0x3 << \
						 MVPP22_XPCS_LANEACTIVE_OFFS)

/* MPCS registers */

#define PCS40G_COMMON_CONTROL			0x14
#define      FORWARD_ERROR_CORRECTION_MASK	BIT(10)

#define PCS_CLOCK_RESET				0x14c
#define      TX_SD_CLK_RESET_MASK		BIT(0)
#define      RX_SD_CLK_RESET_MASK		BIT(1)
#define      MAC_CLK_RESET_MASK			BIT(2)
#define      CLK_DIVISION_RATIO_OFFS		4
#define      CLK_DIVISION_RATIO_MASK		(0x7 << CLK_DIVISION_RATIO_OFFS)
#define      CLK_DIV_PHASE_SET_MASK		BIT(11)

/* System Soft Reset 1 */
#define GOP_SOFT_RESET_1_REG			0x108
#define     NETC_GOP_SOFT_RESET_OFFS		6
#define     NETC_GOP_SOFT_RESET_MASK		(0x1 << \
						 NETC_GOP_SOFT_RESET_OFFS)

/* Ports Control 0 */
#define NETCOMP_PORTS_CONTROL_0_REG		0x110
#define     NETC_BUS_WIDTH_SELECT_OFFS		1
#define     NETC_BUS_WIDTH_SELECT_MASK		(0x1 << \
						 NETC_BUS_WIDTH_SELECT_OFFS)
#define     NETC_GIG_RX_DATA_SAMPLE_OFFS	29
#define     NETC_GIG_RX_DATA_SAMPLE_MASK	(0x1 << \
						 NETC_GIG_RX_DATA_SAMPLE_OFFS)
#define     NETC_CLK_DIV_PHASE_OFFS		31
#define     NETC_CLK_DIV_PHASE_MASK		(0x1 << NETC_CLK_DIV_PHASE_OFFS)
/* Ports Control 1 */
#define NETCOMP_PORTS_CONTROL_1_REG		0x114
#define     NETC_PORTS_ACTIVE_OFFSET(p)		(0 + p)
#define     NETC_PORTS_ACTIVE_MASK(p)		(0x1 << \
						 NETC_PORTS_ACTIVE_OFFSET(p))
#define     NETC_PORT_GIG_RF_RESET_OFFS(p)	(28 + p)
#define     NETC_PORT_GIG_RF_RESET_MASK(p)	(0x1 << \
						 NETC_PORT_GIG_RF_RESET_OFFS(p))
#define NETCOMP_CONTROL_0_REG			0x120
#define     NETC_GBE_PORT0_SGMII_MODE_OFFS	0
#define     NETC_GBE_PORT0_SGMII_MODE_MASK	(0x1 << \
						 NETC_GBE_PORT0_SGMII_MODE_OFFS)
#define     NETC_GBE_PORT1_SGMII_MODE_OFFS	1
#define     NETC_GBE_PORT1_SGMII_MODE_MASK	(0x1 << \
						 NETC_GBE_PORT1_SGMII_MODE_OFFS)
#define     NETC_GBE_PORT1_MII_MODE_OFFS	2
#define     NETC_GBE_PORT1_MII_MODE_MASK	(0x1 << \
						 NETC_GBE_PORT1_MII_MODE_OFFS)

#define MVPP22_SMI_MISC_CFG_REG			(MVPP22_SMI + 0x04)
#define      MVPP22_SMI_POLLING_EN		BIT(10)

#define MVPP22_SMI_PHY_ADDR_REG(port)		(MVPP22_SMI + 0x04 + \
						 (0x4 * (port)))

#define MVPP2_CAUSE_TXQ_SENT_DESC_ALL_MASK	0xff

/* Descriptor ring Macros */
#define MVPP2_QUEUE_NEXT_DESC(q, index) \
	(((index) < (q)->last_desc) ? ((index) + 1) : 0)

/* SMI: 0xc0054 -> offset 0x54 to lms_base */
#define MVPP21_SMI				0x0054
/* PP2.2: SMI: 0x12a200 -> offset 0x1200 to iface_base */
#define MVPP22_SMI				0x1200
#define     MVPP2_PHY_REG_MASK			0x1f
/* SMI register fields */
#define     MVPP2_SMI_DATA_OFFS			0	/* Data */
#define     MVPP2_SMI_DATA_MASK			(0xffff << MVPP2_SMI_DATA_OFFS)
#define     MVPP2_SMI_DEV_ADDR_OFFS		16	/* PHY device address */
#define     MVPP2_SMI_REG_ADDR_OFFS		21	/* PHY device reg addr*/
#define     MVPP2_SMI_OPCODE_OFFS		26	/* Write/Read opcode */
#define     MVPP2_SMI_OPCODE_READ		(1 << MVPP2_SMI_OPCODE_OFFS)
#define     MVPP2_SMI_READ_VALID		(1 << 27)	/* Read Valid */
#define     MVPP2_SMI_BUSY			(1 << 28)	/* Busy */

#define     MVPP2_PHY_ADDR_MASK			0x1f
#define     MVPP2_PHY_REG_MASK			0x1f

/* Additional PPv2.2 offsets */
#define MVPP22_MPCS				0x007000
#define MVPP22_XPCS				0x007400
#define MVPP22_PORT_BASE			0x007e00
#define MVPP22_PORT_OFFSET			0x001000
#define MVPP22_RFU1				0x318000

/* Maximum number of ports */
#define MVPP22_GOP_MAC_NUM			4

/* Sets the field located at the specified in data */
#define MVPP2_RGMII_TX_FIFO_MIN_TH		0x41
#define MVPP2_SGMII_TX_FIFO_MIN_TH		0x5
#define MVPP2_SGMII2_5_TX_FIFO_MIN_TH		0xb

/* Net Complex */
enum mv_netc_topology {
	MV_NETC_GE_MAC2_SGMII		=	BIT(0),
	MV_NETC_GE_MAC3_SGMII		=	BIT(1),
	MV_NETC_GE_MAC3_RGMII		=	BIT(2),
};

enum mv_netc_phase {
	MV_NETC_FIRST_PHASE,
	MV_NETC_SECOND_PHASE,
};

enum mv_netc_sgmii_xmi_mode {
	MV_NETC_GBE_SGMII,
	MV_NETC_GBE_XMII,
};

enum mv_netc_mii_mode {
	MV_NETC_GBE_RGMII,
	MV_NETC_GBE_MII,
};

enum mv_netc_lanes {
	MV_NETC_LANE_23,
	MV_NETC_LANE_45,
};

/* Various constants */

/* Coalescing */
#define MVPP2_TXDONE_COAL_PKTS_THRESH	15
#define MVPP2_TXDONE_HRTIMER_PERIOD_NS	1000000UL
#define MVPP2_RX_COAL_PKTS		32
#define MVPP2_RX_COAL_USEC		100

/* The two bytes Marvell header. Either contains a special value used
 * by Marvell switches when a specific hardware mode is enabled (not
 * supported by this driver) or is filled automatically by zeroes on
 * the RX side. Those two bytes being at the front of the Ethernet
 * header, they allow to have the IP header aligned on a 4 bytes
 * boundary automatically: the hardware skips those two bytes on its
 * own.
 */
#define MVPP2_MH_SIZE			2
#define MVPP2_ETH_TYPE_LEN		2
#define MVPP2_PPPOE_HDR_SIZE		8
#define MVPP2_VLAN_TAG_LEN		4

/* Lbtd 802.3 type */
#define MVPP2_IP_LBDT_TYPE		0xfffa

#define MVPP2_CPU_D_CACHE_LINE_SIZE	32
#define MVPP2_TX_CSUM_MAX_SIZE		9800

/* Timeout constants */
#define MVPP2_TX_DISABLE_TIMEOUT_MSEC	1000
#define MVPP2_TX_PENDING_TIMEOUT_MSEC	1000

#define MVPP2_TX_MTU_MAX		0x7ffff

/* Maximum number of T-CONTs of PON port */
#define MVPP2_MAX_TCONT			16

/* Maximum number of supported ports */
#define MVPP2_MAX_PORTS			4

/* Maximum number of TXQs used by single port */
#define MVPP2_MAX_TXQ			8

/* Default number of TXQs in use */
#define MVPP2_DEFAULT_TXQ		1

/* Dfault number of RXQs in use */
#define MVPP2_DEFAULT_RXQ		1
#define CONFIG_MV_ETH_RXQ		8	/* increment by 8 */

/* Max number of Rx descriptors */
#define MVPP2_MAX_RXD			16

/* Max number of Tx descriptors */
#define MVPP2_MAX_TXD			16

/* Amount of Tx descriptors that can be reserved at once by CPU */
#define MVPP2_CPU_DESC_CHUNK		16

/* Max number of Tx descriptors in each aggregated queue */
#define MVPP2_AGGR_TXQ_SIZE		16

/* Descriptor aligned size */
#define MVPP2_DESC_ALIGNED_SIZE		32

/* Descriptor alignment mask */
#define MVPP2_TX_DESC_ALIGN		(MVPP2_DESC_ALIGNED_SIZE - 1)

/* RX FIFO constants */
#define MVPP21_RX_FIFO_PORT_DATA_SIZE		0x2000
#define MVPP21_RX_FIFO_PORT_ATTR_SIZE		0x80
#define MVPP22_RX_FIFO_10GB_PORT_DATA_SIZE	0x8000
#define MVPP22_RX_FIFO_2_5GB_PORT_DATA_SIZE	0x2000
#define MVPP22_RX_FIFO_1GB_PORT_DATA_SIZE	0x1000
#define MVPP22_RX_FIFO_10GB_PORT_ATTR_SIZE	0x200
#define MVPP22_RX_FIFO_2_5GB_PORT_ATTR_SIZE	0x80
#define MVPP22_RX_FIFO_1GB_PORT_ATTR_SIZE	0x40
#define MVPP2_RX_FIFO_PORT_MIN_PKT		0x80

/* TX general registers */
#define MVPP22_TX_FIFO_SIZE_REG(eth_tx_port)	(0x8860 + ((eth_tx_port) << 2))
#define MVPP22_TX_FIFO_SIZE_MASK		0xf

/* TX FIFO constants */
#define MVPP2_TX_FIFO_DATA_SIZE_10KB		0xa
#define MVPP2_TX_FIFO_DATA_SIZE_3KB		0x3

/* RX buffer constants */
#define MVPP2_SKB_SHINFO_SIZE \
	0

#define MVPP2_RX_PKT_SIZE(mtu) \
	ALIGN((mtu) + MVPP2_MH_SIZE + MVPP2_VLAN_TAG_LEN + \
	      ETH_HLEN + ETH_FCS_LEN, MVPP2_CPU_D_CACHE_LINE_SIZE)

#define MVPP2_RX_BUF_SIZE(pkt_size)	((pkt_size) + NET_SKB_PAD)
#define MVPP2_RX_TOTAL_SIZE(buf_size)	((buf_size) + MVPP2_SKB_SHINFO_SIZE)
#define MVPP2_RX_MAX_PKT_SIZE(total_size) \
	((total_size) - NET_SKB_PAD - MVPP2_SKB_SHINFO_SIZE)

#define MVPP2_BIT_TO_BYTE(bit)		((bit) / 8)

/* IPv6 max L3 address size */
#define MVPP2_MAX_L3_ADDR_SIZE		16

/* Port flags */
#define MVPP2_F_LOOPBACK		BIT(0)

/* Marvell tag types */
enum mvpp2_tag_type {
	MVPP2_TAG_TYPE_NONE = 0,
	MVPP2_TAG_TYPE_MH   = 1,
	MVPP2_TAG_TYPE_DSA  = 2,
	MVPP2_TAG_TYPE_EDSA = 3,
	MVPP2_TAG_TYPE_VLAN = 4,
	MVPP2_TAG_TYPE_LAST = 5
};

/* Parser constants */
#define MVPP2_PRS_TCAM_SRAM_SIZE	256
#define MVPP2_PRS_TCAM_WORDS		6
#define MVPP2_PRS_SRAM_WORDS		4
#define MVPP2_PRS_FLOW_ID_SIZE		64
#define MVPP2_PRS_FLOW_ID_MASK		0x3f
#define MVPP2_PRS_TCAM_ENTRY_INVALID	1
#define MVPP2_PRS_TCAM_DSA_TAGGED_BIT	BIT(5)
#define MVPP2_PRS_IPV4_HEAD		0x40
#define MVPP2_PRS_IPV4_HEAD_MASK	0xf0
#define MVPP2_PRS_IPV4_MC		0xe0
#define MVPP2_PRS_IPV4_MC_MASK		0xf0
#define MVPP2_PRS_IPV4_BC_MASK		0xff
#define MVPP2_PRS_IPV4_IHL		0x5
#define MVPP2_PRS_IPV4_IHL_MASK		0xf
#define MVPP2_PRS_IPV6_MC		0xff
#define MVPP2_PRS_IPV6_MC_MASK		0xff
#define MVPP2_PRS_IPV6_HOP_MASK		0xff
#define MVPP2_PRS_TCAM_PROTO_MASK	0xff
#define MVPP2_PRS_TCAM_PROTO_MASK_L	0x3f
#define MVPP2_PRS_DBL_VLANS_MAX		100

/* Tcam structure:
 * - lookup ID - 4 bits
 * - port ID - 1 byte
 * - additional information - 1 byte
 * - header data - 8 bytes
 * The fields are represented by MVPP2_PRS_TCAM_DATA_REG(5)->(0).
 */
#define MVPP2_PRS_AI_BITS			8
#define MVPP2_PRS_PORT_MASK			0xff
#define MVPP2_PRS_LU_MASK			0xf
#define MVPP2_PRS_TCAM_DATA_BYTE(offs)		\
				    (((offs) - ((offs) % 2)) * 2 + ((offs) % 2))
#define MVPP2_PRS_TCAM_DATA_BYTE_EN(offs)	\
					      (((offs) * 2) - ((offs) % 2)  + 2)
#define MVPP2_PRS_TCAM_AI_BYTE			16
#define MVPP2_PRS_TCAM_PORT_BYTE		17
#define MVPP2_PRS_TCAM_LU_BYTE			20
#define MVPP2_PRS_TCAM_EN_OFFS(offs)		((offs) + 2)
#define MVPP2_PRS_TCAM_INV_WORD			5
/* Tcam entries ID */
#define MVPP2_PE_DROP_ALL		0
#define MVPP2_PE_FIRST_FREE_TID		1
#define MVPP2_PE_LAST_FREE_TID		(MVPP2_PRS_TCAM_SRAM_SIZE - 31)
#define MVPP2_PE_IP6_EXT_PROTO_UN	(MVPP2_PRS_TCAM_SRAM_SIZE - 30)
#define MVPP2_PE_MAC_MC_IP6		(MVPP2_PRS_TCAM_SRAM_SIZE - 29)
#define MVPP2_PE_IP6_ADDR_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 28)
#define MVPP2_PE_IP4_ADDR_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 27)
#define MVPP2_PE_LAST_DEFAULT_FLOW	(MVPP2_PRS_TCAM_SRAM_SIZE - 26)
#define MVPP2_PE_FIRST_DEFAULT_FLOW	(MVPP2_PRS_TCAM_SRAM_SIZE - 19)
#define MVPP2_PE_EDSA_TAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 18)
#define MVPP2_PE_EDSA_UNTAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 17)
#define MVPP2_PE_DSA_TAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 16)
#define MVPP2_PE_DSA_UNTAGGED		(MVPP2_PRS_TCAM_SRAM_SIZE - 15)
#define MVPP2_PE_ETYPE_EDSA_TAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 14)
#define MVPP2_PE_ETYPE_EDSA_UNTAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 13)
#define MVPP2_PE_ETYPE_DSA_TAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 12)
#define MVPP2_PE_ETYPE_DSA_UNTAGGED	(MVPP2_PRS_TCAM_SRAM_SIZE - 11)
#define MVPP2_PE_MH_DEFAULT		(MVPP2_PRS_TCAM_SRAM_SIZE - 10)
#define MVPP2_PE_DSA_DEFAULT		(MVPP2_PRS_TCAM_SRAM_SIZE - 9)
#define MVPP2_PE_IP6_PROTO_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 8)
#define MVPP2_PE_IP4_PROTO_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 7)
#define MVPP2_PE_ETH_TYPE_UN		(MVPP2_PRS_TCAM_SRAM_SIZE - 6)
#define MVPP2_PE_VLAN_DBL		(MVPP2_PRS_TCAM_SRAM_SIZE - 5)
#define MVPP2_PE_VLAN_NONE		(MVPP2_PRS_TCAM_SRAM_SIZE - 4)
#define MVPP2_PE_MAC_MC_ALL		(MVPP2_PRS_TCAM_SRAM_SIZE - 3)
#define MVPP2_PE_MAC_PROMISCUOUS	(MVPP2_PRS_TCAM_SRAM_SIZE - 2)
#define MVPP2_PE_MAC_NON_PROMISCUOUS	(MVPP2_PRS_TCAM_SRAM_SIZE - 1)

/* Sram structure
 * The fields are represented by MVPP2_PRS_TCAM_DATA_REG(3)->(0).
 */
#define MVPP2_PRS_SRAM_RI_OFFS			0
#define MVPP2_PRS_SRAM_RI_WORD			0
#define MVPP2_PRS_SRAM_RI_CTRL_OFFS		32
#define MVPP2_PRS_SRAM_RI_CTRL_WORD		1
#define MVPP2_PRS_SRAM_RI_CTRL_BITS		32
#define MVPP2_PRS_SRAM_SHIFT_OFFS		64
#define MVPP2_PRS_SRAM_SHIFT_SIGN_BIT		72
#define MVPP2_PRS_SRAM_UDF_OFFS			73
#define MVPP2_PRS_SRAM_UDF_BITS			8
#define MVPP2_PRS_SRAM_UDF_MASK			0xff
#define MVPP2_PRS_SRAM_UDF_SIGN_BIT		81
#define MVPP2_PRS_SRAM_UDF_TYPE_OFFS		82
#define MVPP2_PRS_SRAM_UDF_TYPE_MASK		0x7
#define MVPP2_PRS_SRAM_UDF_TYPE_L3		1
#define MVPP2_PRS_SRAM_UDF_TYPE_L4		4
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_OFFS	85
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_MASK	0x3
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD		1
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_IP4_ADD	2
#define MVPP2_PRS_SRAM_OP_SEL_SHIFT_IP6_ADD	3
#define MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS		87
#define MVPP2_PRS_SRAM_OP_SEL_UDF_BITS		2
#define MVPP2_PRS_SRAM_OP_SEL_UDF_MASK		0x3
#define MVPP2_PRS_SRAM_OP_SEL_UDF_ADD		0
#define MVPP2_PRS_SRAM_OP_SEL_UDF_IP4_ADD	2
#define MVPP2_PRS_SRAM_OP_SEL_UDF_IP6_ADD	3
#define MVPP2_PRS_SRAM_OP_SEL_BASE_OFFS		89
#define MVPP2_PRS_SRAM_AI_OFFS			90
#define MVPP2_PRS_SRAM_AI_CTRL_OFFS		98
#define MVPP2_PRS_SRAM_AI_CTRL_BITS		8
#define MVPP2_PRS_SRAM_AI_MASK			0xff
#define MVPP2_PRS_SRAM_NEXT_LU_OFFS		106
#define MVPP2_PRS_SRAM_NEXT_LU_MASK		0xf
#define MVPP2_PRS_SRAM_LU_DONE_BIT		110
#define MVPP2_PRS_SRAM_LU_GEN_BIT		111

/* Sram result info bits assignment */
#define MVPP2_PRS_RI_MAC_ME_MASK		0x1
#define MVPP2_PRS_RI_DSA_MASK			0x2
#define MVPP2_PRS_RI_VLAN_MASK			(BIT(2) | BIT(3))
#define MVPP2_PRS_RI_VLAN_NONE			0x0
#define MVPP2_PRS_RI_VLAN_SINGLE		BIT(2)
#define MVPP2_PRS_RI_VLAN_DOUBLE		BIT(3)
#define MVPP2_PRS_RI_VLAN_TRIPLE		(BIT(2) | BIT(3))
#define MVPP2_PRS_RI_CPU_CODE_MASK		0x70
#define MVPP2_PRS_RI_CPU_CODE_RX_SPEC		BIT(4)
#define MVPP2_PRS_RI_L2_CAST_MASK		(BIT(9) | BIT(10))
#define MVPP2_PRS_RI_L2_UCAST			0x0
#define MVPP2_PRS_RI_L2_MCAST			BIT(9)
#define MVPP2_PRS_RI_L2_BCAST			BIT(10)
#define MVPP2_PRS_RI_PPPOE_MASK			0x800
#define MVPP2_PRS_RI_L3_PROTO_MASK		(BIT(12) | BIT(13) | BIT(14))
#define MVPP2_PRS_RI_L3_UN			0x0
#define MVPP2_PRS_RI_L3_IP4			BIT(12)
#define MVPP2_PRS_RI_L3_IP4_OPT			BIT(13)
#define MVPP2_PRS_RI_L3_IP4_OTHER		(BIT(12) | BIT(13))
#define MVPP2_PRS_RI_L3_IP6			BIT(14)
#define MVPP2_PRS_RI_L3_IP6_EXT			(BIT(12) | BIT(14))
#define MVPP2_PRS_RI_L3_ARP			(BIT(13) | BIT(14))
#define MVPP2_PRS_RI_L3_ADDR_MASK		(BIT(15) | BIT(16))
#define MVPP2_PRS_RI_L3_UCAST			0x0
#define MVPP2_PRS_RI_L3_MCAST			BIT(15)
#define MVPP2_PRS_RI_L3_BCAST			(BIT(15) | BIT(16))
#define MVPP2_PRS_RI_IP_FRAG_MASK		0x20000
#define MVPP2_PRS_RI_UDF3_MASK			0x300000
#define MVPP2_PRS_RI_UDF3_RX_SPECIAL		BIT(21)
#define MVPP2_PRS_RI_L4_PROTO_MASK		0x1c00000
#define MVPP2_PRS_RI_L4_TCP			BIT(22)
#define MVPP2_PRS_RI_L4_UDP			BIT(23)
#define MVPP2_PRS_RI_L4_OTHER			(BIT(22) | BIT(23))
#define MVPP2_PRS_RI_UDF7_MASK			0x60000000
#define MVPP2_PRS_RI_UDF7_IP6_LITE		BIT(29)
#define MVPP2_PRS_RI_DROP_MASK			0x80000000

/* Sram additional info bits assignment */
#define MVPP2_PRS_IPV4_DIP_AI_BIT		BIT(0)
#define MVPP2_PRS_IPV6_NO_EXT_AI_BIT		BIT(0)
#define MVPP2_PRS_IPV6_EXT_AI_BIT		BIT(1)
#define MVPP2_PRS_IPV6_EXT_AH_AI_BIT		BIT(2)
#define MVPP2_PRS_IPV6_EXT_AH_LEN_AI_BIT	BIT(3)
#define MVPP2_PRS_IPV6_EXT_AH_L4_AI_BIT		BIT(4)
#define MVPP2_PRS_SINGLE_VLAN_AI		0
#define MVPP2_PRS_DBL_VLAN_AI_BIT		BIT(7)

/* DSA/EDSA type */
#define MVPP2_PRS_TAGGED		true
#define MVPP2_PRS_UNTAGGED		false
#define MVPP2_PRS_EDSA			true
#define MVPP2_PRS_DSA			false

/* MAC entries, shadow udf */
enum mvpp2_prs_udf {
	MVPP2_PRS_UDF_MAC_DEF,
	MVPP2_PRS_UDF_MAC_RANGE,
	MVPP2_PRS_UDF_L2_DEF,
	MVPP2_PRS_UDF_L2_DEF_COPY,
	MVPP2_PRS_UDF_L2_USER,
};

/* Lookup ID */
enum mvpp2_prs_lookup {
	MVPP2_PRS_LU_MH,
	MVPP2_PRS_LU_MAC,
	MVPP2_PRS_LU_DSA,
	MVPP2_PRS_LU_VLAN,
	MVPP2_PRS_LU_L2,
	MVPP2_PRS_LU_PPPOE,
	MVPP2_PRS_LU_IP4,
	MVPP2_PRS_LU_IP6,
	MVPP2_PRS_LU_FLOWS,
	MVPP2_PRS_LU_LAST,
};

/* L3 cast enum */
enum mvpp2_prs_l3_cast {
	MVPP2_PRS_L3_UNI_CAST,
	MVPP2_PRS_L3_MULTI_CAST,
	MVPP2_PRS_L3_BROAD_CAST
};

/* Classifier constants */
#define MVPP2_CLS_FLOWS_TBL_SIZE	512
#define MVPP2_CLS_FLOWS_TBL_DATA_WORDS	3
#define MVPP2_CLS_LKP_TBL_SIZE		64

/* BM constants */
#define MVPP2_BM_POOLS_NUM		1
#define MVPP2_BM_LONG_BUF_NUM		16
#define MVPP2_BM_SHORT_BUF_NUM		16
#define MVPP2_BM_POOL_SIZE_MAX		(16*1024 - MVPP2_BM_POOL_PTR_ALIGN/4)
#define MVPP2_BM_POOL_PTR_ALIGN		128
#define MVPP2_BM_SWF_LONG_POOL(port)	0

/* BM cookie (32 bits) definition */
#define MVPP2_BM_COOKIE_POOL_OFFS	8
#define MVPP2_BM_COOKIE_CPU_OFFS	24

/* BM short pool packet size
 * These value assure that for SWF the total number
 * of bytes allocated for each buffer will be 512
 */
#define MVPP2_BM_SHORT_PKT_SIZE		MVPP2_RX_MAX_PKT_SIZE(512)

enum mvpp2_bm_type {
	MVPP2_BM_FREE,
	MVPP2_BM_SWF_LONG,
	MVPP2_BM_SWF_SHORT
};

/* Definitions */

/* Shared Packet Processor resources */
struct mvpp2 {
	/* Shared registers' base addresses */
	void __iomem *base;
	void __iomem *lms_base;
	void __iomem *iface_base;

	void __iomem *mpcs_base;
	void __iomem *xpcs_base;
	void __iomem *rfu1_base;

	u32 netc_config;

	/* List of pointers to port structures */
	struct mvpp2_port **port_list;

	/* Aggregated TXQs */
	struct mvpp2_tx_queue *aggr_txqs;

	/* BM pools */
	struct mvpp2_bm_pool *bm_pools;

	/* PRS shadow table */
	struct mvpp2_prs_shadow *prs_shadow;
	/* PRS auxiliary table for double vlan entries control */
	bool *prs_double_vlans;

	/* Tclk value */
	u32 tclk;

	/* HW version */
	enum { MVPP21, MVPP22 } hw_version;

	/* Maximum number of RXQs per port */
	unsigned int max_port_rxqs;

	int probe_done;
	u8 num_ports;
};

struct mvpp2_pcpu_stats {
	u64	rx_packets;
	u64	rx_bytes;
	u64	tx_packets;
	u64	tx_bytes;
};

struct mvpp2_port {
	u8 id;

	/* Index of the port from the "group of ports" complex point
	 * of view
	 */
	int gop_id;

	int irq;

	struct mvpp2 *priv;

	/* Per-port registers' base address */
	void __iomem *base;
	void __iomem *mdio_base;

	struct mvpp2_rx_queue **rxqs;
	struct mvpp2_tx_queue **txqs;

	int pkt_size;

	u32 pending_cause_rx;

	/* Per-CPU port control */
	struct mvpp2_port_pcpu __percpu *pcpu;

	/* Flags */
	unsigned long flags;

	u16 tx_ring_size;
	u16 rx_ring_size;
	struct mvpp2_pcpu_stats __percpu *stats;

	struct phy_device *phy_dev;
	phy_interface_t phy_interface;
	int phy_node;
	int phyaddr;
	struct mii_dev *bus;
#ifdef CONFIG_DM_GPIO
	struct gpio_desc phy_reset_gpio;
	struct gpio_desc phy_tx_disable_gpio;
#endif
	int init;
	unsigned int link;
	unsigned int duplex;
	unsigned int speed;

	unsigned int phy_speed;		/* SGMII 1Gbps vs 2.5Gbps */

	struct mvpp2_bm_pool *pool_long;
	struct mvpp2_bm_pool *pool_short;

	/* Index of first port's physical RXQ */
	u8 first_rxq;

	u8 dev_addr[ETH_ALEN];
};

/* The mvpp2_tx_desc and mvpp2_rx_desc structures describe the
 * layout of the transmit and reception DMA descriptors, and their
 * layout is therefore defined by the hardware design
 */

#define MVPP2_TXD_L3_OFF_SHIFT		0
#define MVPP2_TXD_IP_HLEN_SHIFT		8
#define MVPP2_TXD_L4_CSUM_FRAG		BIT(13)
#define MVPP2_TXD_L4_CSUM_NOT		BIT(14)
#define MVPP2_TXD_IP_CSUM_DISABLE	BIT(15)
#define MVPP2_TXD_PADDING_DISABLE	BIT(23)
#define MVPP2_TXD_L4_UDP		BIT(24)
#define MVPP2_TXD_L3_IP6		BIT(26)
#define MVPP2_TXD_L_DESC		BIT(28)
#define MVPP2_TXD_F_DESC		BIT(29)

#define MVPP2_RXD_ERR_SUMMARY		BIT(15)
#define MVPP2_RXD_ERR_CODE_MASK		(BIT(13) | BIT(14))
#define MVPP2_RXD_ERR_CRC		0x0
#define MVPP2_RXD_ERR_OVERRUN		BIT(13)
#define MVPP2_RXD_ERR_RESOURCE		(BIT(13) | BIT(14))
#define MVPP2_RXD_BM_POOL_ID_OFFS	16
#define MVPP2_RXD_BM_POOL_ID_MASK	(BIT(16) | BIT(17) | BIT(18))
#define MVPP2_RXD_HWF_SYNC		BIT(21)
#define MVPP2_RXD_L4_CSUM_OK		BIT(22)
#define MVPP2_RXD_IP4_HEADER_ERR	BIT(24)
#define MVPP2_RXD_L4_TCP		BIT(25)
#define MVPP2_RXD_L4_UDP		BIT(26)
#define MVPP2_RXD_L3_IP4		BIT(28)
#define MVPP2_RXD_L3_IP6		BIT(30)
#define MVPP2_RXD_BUF_HDR		BIT(31)

/* HW TX descriptor for PPv2.1 */
struct mvpp21_tx_desc {
	u32 command;		/* Options used by HW for packet transmitting.*/
	u8  packet_offset;	/* the offset from the buffer beginning	*/
	u8  phys_txq;		/* destination queue ID			*/
	u16 data_size;		/* data size of transmitted packet in bytes */
	u32 buf_dma_addr;	/* physical addr of transmitted buffer	*/
	u32 buf_cookie;		/* cookie for access to TX buffer in tx path */
	u32 reserved1[3];	/* hw_cmd (for future use, BM, PON, PNC) */
	u32 reserved2;		/* reserved (for future use)		*/
};

/* HW RX descriptor for PPv2.1 */
struct mvpp21_rx_desc {
	u32 status;		/* info about received packet		*/
	u16 reserved1;		/* parser_info (for future use, PnC)	*/
	u16 data_size;		/* size of received packet in bytes	*/
	u32 buf_dma_addr;	/* physical address of the buffer	*/
	u32 buf_cookie;		/* cookie for access to RX buffer in rx path */
	u16 reserved2;		/* gem_port_id (for future use, PON)	*/
	u16 reserved3;		/* csum_l4 (for future use, PnC)	*/
	u8  reserved4;		/* bm_qset (for future use, BM)		*/
	u8  reserved5;
	u16 reserved6;		/* classify_info (for future use, PnC)	*/
	u32 reserved7;		/* flow_id (for future use, PnC) */
	u32 reserved8;
};

/* HW TX descriptor for PPv2.2 */
struct mvpp22_tx_desc {
	u32 command;
	u8  packet_offset;
	u8  phys_txq;
	u16 data_size;
	u64 reserved1;
	u64 buf_dma_addr_ptp;
	u64 buf_cookie_misc;
};

/* HW RX descriptor for PPv2.2 */
struct mvpp22_rx_desc {
	u32 status;
	u16 reserved1;
	u16 data_size;
	u32 reserved2;
	u32 reserved3;
	u64 buf_dma_addr_key_hash;
	u64 buf_cookie_misc;
};

/* Opaque type used by the driver to manipulate the HW TX and RX
 * descriptors
 */
struct mvpp2_tx_desc {
	union {
		struct mvpp21_tx_desc pp21;
		struct mvpp22_tx_desc pp22;
	};
};

struct mvpp2_rx_desc {
	union {
		struct mvpp21_rx_desc pp21;
		struct mvpp22_rx_desc pp22;
	};
};

/* Per-CPU Tx queue control */
struct mvpp2_txq_pcpu {
	int cpu;

	/* Number of Tx DMA descriptors in the descriptor ring */
	int size;

	/* Number of currently used Tx DMA descriptor in the
	 * descriptor ring
	 */
	int count;

	/* Number of Tx DMA descriptors reserved for each CPU */
	int reserved_num;

	/* Index of last TX DMA descriptor that was inserted */
	int txq_put_index;

	/* Index of the TX DMA descriptor to be cleaned up */
	int txq_get_index;
};

struct mvpp2_tx_queue {
	/* Physical number of this Tx queue */
	u8 id;

	/* Logical number of this Tx queue */
	u8 log_id;

	/* Number of Tx DMA descriptors in the descriptor ring */
	int size;

	/* Number of currently used Tx DMA descriptor in the descriptor ring */
	int count;

	/* Per-CPU control of physical Tx queues */
	struct mvpp2_txq_pcpu __percpu *pcpu;

	u32 done_pkts_coal;

	/* Virtual address of thex Tx DMA descriptors array */
	struct mvpp2_tx_desc *descs;

	/* DMA address of the Tx DMA descriptors array */
	dma_addr_t descs_dma;

	/* Index of the last Tx DMA descriptor */
	int last_desc;

	/* Index of the next Tx DMA descriptor to process */
	int next_desc_to_proc;
};

struct mvpp2_rx_queue {
	/* RX queue number, in the range 0-31 for physical RXQs */
	u8 id;

	/* Num of rx descriptors in the rx descriptor ring */
	int size;

	u32 pkts_coal;
	u32 time_coal;

	/* Virtual address of the RX DMA descriptors array */
	struct mvpp2_rx_desc *descs;

	/* DMA address of the RX DMA descriptors array */
	dma_addr_t descs_dma;

	/* Index of the last RX DMA descriptor */
	int last_desc;

	/* Index of the next RX DMA descriptor to process */
	int next_desc_to_proc;

	/* ID of port to which physical RXQ is mapped */
	int port;

	/* Port's logic RXQ number to which physical RXQ is mapped */
	int logic_rxq;
};

union mvpp2_prs_tcam_entry {
	u32 word[MVPP2_PRS_TCAM_WORDS];
	u8  byte[MVPP2_PRS_TCAM_WORDS * 4];
};

union mvpp2_prs_sram_entry {
	u32 word[MVPP2_PRS_SRAM_WORDS];
	u8  byte[MVPP2_PRS_SRAM_WORDS * 4];
};

struct mvpp2_prs_entry {
	u32 index;
	union mvpp2_prs_tcam_entry tcam;
	union mvpp2_prs_sram_entry sram;
};

struct mvpp2_prs_shadow {
	bool valid;
	bool finish;

	/* Lookup ID */
	int lu;

	/* User defined offset */
	int udf;

	/* Result info */
	u32 ri;
	u32 ri_mask;
};

struct mvpp2_cls_flow_entry {
	u32 index;
	u32 data[MVPP2_CLS_FLOWS_TBL_DATA_WORDS];
};

struct mvpp2_cls_lookup_entry {
	u32 lkpid;
	u32 way;
	u32 data;
};

struct mvpp2_bm_pool {
	/* Pool number in the range 0-7 */
	int id;
	enum mvpp2_bm_type type;

	/* Buffer Pointers Pool External (BPPE) size */
	int size;
	/* Number of buffers for this pool */
	int buf_num;
	/* Pool buffer size */
	int buf_size;
	/* Packet size */
	int pkt_size;

	/* BPPE virtual base address */
	unsigned long *virt_addr;
	/* BPPE DMA base address */
	dma_addr_t dma_addr;

	/* Ports using BM pool */
	u32 port_map;
};

/* Static declaractions */

/* Number of RXQs used by single port */
static int rxq_number = MVPP2_DEFAULT_RXQ;
/* Number of TXQs used by single port */
static int txq_number = MVPP2_DEFAULT_TXQ;

static int base_id;

#define MVPP2_DRIVER_NAME "mvpp2"
#define MVPP2_DRIVER_VERSION "1.0"

/*
 * U-Boot internal data, mostly uncached buffers for descriptors and data
 */
struct buffer_location {
	struct mvpp2_tx_desc *aggr_tx_descs;
	struct mvpp2_tx_desc *tx_descs;
	struct mvpp2_rx_desc *rx_descs;
	unsigned long *bm_pool[MVPP2_BM_POOLS_NUM];
	unsigned long *rx_buffer[MVPP2_BM_LONG_BUF_NUM];
	int first_rxq;
};

/*
 * All 4 interfaces use the same global buffer, since only one interface
 * can be enabled at once
 */
static struct buffer_location buffer_loc;

/*
 * Page table entries are set to 1MB, or multiples of 1MB
 * (not < 1MB). driver uses less bd's so use 1MB bdspace.
 */
#define BD_SPACE	(1 << 20)

/* Utility/helper methods */

static void mvpp2_write(struct mvpp2 *priv, u32 offset, u32 data)
{
	writel(data, priv->base + offset);
}

static u32 mvpp2_read(struct mvpp2 *priv, u32 offset)
{
	return readl(priv->base + offset);
}

static void mvpp2_txdesc_dma_addr_set(struct mvpp2_port *port,
				      struct mvpp2_tx_desc *tx_desc,
				      dma_addr_t dma_addr)
{
	if (port->priv->hw_version == MVPP21) {
		tx_desc->pp21.buf_dma_addr = dma_addr;
	} else {
		u64 val = (u64)dma_addr;

		tx_desc->pp22.buf_dma_addr_ptp &= ~GENMASK_ULL(40, 0);
		tx_desc->pp22.buf_dma_addr_ptp |= val;
	}
}

static void mvpp2_txdesc_size_set(struct mvpp2_port *port,
				  struct mvpp2_tx_desc *tx_desc,
				  size_t size)
{
	if (port->priv->hw_version == MVPP21)
		tx_desc->pp21.data_size = size;
	else
		tx_desc->pp22.data_size = size;
}

static void mvpp2_txdesc_txq_set(struct mvpp2_port *port,
				 struct mvpp2_tx_desc *tx_desc,
				 unsigned int txq)
{
	if (port->priv->hw_version == MVPP21)
		tx_desc->pp21.phys_txq = txq;
	else
		tx_desc->pp22.phys_txq = txq;
}

static void mvpp2_txdesc_cmd_set(struct mvpp2_port *port,
				 struct mvpp2_tx_desc *tx_desc,
				 unsigned int command)
{
	if (port->priv->hw_version == MVPP21)
		tx_desc->pp21.command = command;
	else
		tx_desc->pp22.command = command;
}

static void mvpp2_txdesc_offset_set(struct mvpp2_port *port,
				    struct mvpp2_tx_desc *tx_desc,
				    unsigned int offset)
{
	if (port->priv->hw_version == MVPP21)
		tx_desc->pp21.packet_offset = offset;
	else
		tx_desc->pp22.packet_offset = offset;
}

static dma_addr_t mvpp2_rxdesc_dma_addr_get(struct mvpp2_port *port,
					    struct mvpp2_rx_desc *rx_desc)
{
	if (port->priv->hw_version == MVPP21)
		return rx_desc->pp21.buf_dma_addr;
	else
		return rx_desc->pp22.buf_dma_addr_key_hash & GENMASK_ULL(40, 0);
}

static unsigned long mvpp2_rxdesc_cookie_get(struct mvpp2_port *port,
					     struct mvpp2_rx_desc *rx_desc)
{
	if (port->priv->hw_version == MVPP21)
		return rx_desc->pp21.buf_cookie;
	else
		return rx_desc->pp22.buf_cookie_misc & GENMASK_ULL(40, 0);
}

static size_t mvpp2_rxdesc_size_get(struct mvpp2_port *port,
				    struct mvpp2_rx_desc *rx_desc)
{
	if (port->priv->hw_version == MVPP21)
		return rx_desc->pp21.data_size;
	else
		return rx_desc->pp22.data_size;
}

static u32 mvpp2_rxdesc_status_get(struct mvpp2_port *port,
				   struct mvpp2_rx_desc *rx_desc)
{
	if (port->priv->hw_version == MVPP21)
		return rx_desc->pp21.status;
	else
		return rx_desc->pp22.status;
}

static void mvpp2_txq_inc_get(struct mvpp2_txq_pcpu *txq_pcpu)
{
	txq_pcpu->txq_get_index++;
	if (txq_pcpu->txq_get_index == txq_pcpu->size)
		txq_pcpu->txq_get_index = 0;
}

/* Get number of physical egress port */
static inline int mvpp2_egress_port(struct mvpp2_port *port)
{
	return MVPP2_MAX_TCONT + port->id;
}

/* Get number of physical TXQ */
static inline int mvpp2_txq_phys(int port, int txq)
{
	return (MVPP2_MAX_TCONT + port) * MVPP2_MAX_TXQ + txq;
}

/* Parser configuration routines */

/* Update parser tcam and sram hw entries */
static int mvpp2_prs_hw_write(struct mvpp2 *priv, struct mvpp2_prs_entry *pe)
{
	int i;

	if (pe->index > MVPP2_PRS_TCAM_SRAM_SIZE - 1)
		return -EINVAL;

	/* Clear entry invalidation bit */
	pe->tcam.word[MVPP2_PRS_TCAM_INV_WORD] &= ~MVPP2_PRS_TCAM_INV_MASK;

	/* Write tcam index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, pe->index);
	for (i = 0; i < MVPP2_PRS_TCAM_WORDS; i++)
		mvpp2_write(priv, MVPP2_PRS_TCAM_DATA_REG(i), pe->tcam.word[i]);

	/* Write sram index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_SRAM_IDX_REG, pe->index);
	for (i = 0; i < MVPP2_PRS_SRAM_WORDS; i++)
		mvpp2_write(priv, MVPP2_PRS_SRAM_DATA_REG(i), pe->sram.word[i]);

	return 0;
}

/* Read tcam entry from hw */
static int mvpp2_prs_hw_read(struct mvpp2 *priv, struct mvpp2_prs_entry *pe)
{
	int i;

	if (pe->index > MVPP2_PRS_TCAM_SRAM_SIZE - 1)
		return -EINVAL;

	/* Write tcam index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, pe->index);

	pe->tcam.word[MVPP2_PRS_TCAM_INV_WORD] = mvpp2_read(priv,
			      MVPP2_PRS_TCAM_DATA_REG(MVPP2_PRS_TCAM_INV_WORD));
	if (pe->tcam.word[MVPP2_PRS_TCAM_INV_WORD] & MVPP2_PRS_TCAM_INV_MASK)
		return MVPP2_PRS_TCAM_ENTRY_INVALID;

	for (i = 0; i < MVPP2_PRS_TCAM_WORDS; i++)
		pe->tcam.word[i] = mvpp2_read(priv, MVPP2_PRS_TCAM_DATA_REG(i));

	/* Write sram index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_SRAM_IDX_REG, pe->index);
	for (i = 0; i < MVPP2_PRS_SRAM_WORDS; i++)
		pe->sram.word[i] = mvpp2_read(priv, MVPP2_PRS_SRAM_DATA_REG(i));

	return 0;
}

/* Invalidate tcam hw entry */
static void mvpp2_prs_hw_inv(struct mvpp2 *priv, int index)
{
	/* Write index - indirect access */
	mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, index);
	mvpp2_write(priv, MVPP2_PRS_TCAM_DATA_REG(MVPP2_PRS_TCAM_INV_WORD),
		    MVPP2_PRS_TCAM_INV_MASK);
}

/* Enable shadow table entry and set its lookup ID */
static void mvpp2_prs_shadow_set(struct mvpp2 *priv, int index, int lu)
{
	priv->prs_shadow[index].valid = true;
	priv->prs_shadow[index].lu = lu;
}

/* Update ri fields in shadow table entry */
static void mvpp2_prs_shadow_ri_set(struct mvpp2 *priv, int index,
				    unsigned int ri, unsigned int ri_mask)
{
	priv->prs_shadow[index].ri_mask = ri_mask;
	priv->prs_shadow[index].ri = ri;
}

/* Update lookup field in tcam sw entry */
static void mvpp2_prs_tcam_lu_set(struct mvpp2_prs_entry *pe, unsigned int lu)
{
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_LU_BYTE);

	pe->tcam.byte[MVPP2_PRS_TCAM_LU_BYTE] = lu;
	pe->tcam.byte[enable_off] = MVPP2_PRS_LU_MASK;
}

/* Update mask for single port in tcam sw entry */
static void mvpp2_prs_tcam_port_set(struct mvpp2_prs_entry *pe,
				    unsigned int port, bool add)
{
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_PORT_BYTE);

	if (add)
		pe->tcam.byte[enable_off] &= ~(1 << port);
	else
		pe->tcam.byte[enable_off] |= 1 << port;
}

/* Update port map in tcam sw entry */
static void mvpp2_prs_tcam_port_map_set(struct mvpp2_prs_entry *pe,
					unsigned int ports)
{
	unsigned char port_mask = MVPP2_PRS_PORT_MASK;
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_PORT_BYTE);

	pe->tcam.byte[MVPP2_PRS_TCAM_PORT_BYTE] = 0;
	pe->tcam.byte[enable_off] &= ~port_mask;
	pe->tcam.byte[enable_off] |= ~ports & MVPP2_PRS_PORT_MASK;
}

/* Obtain port map from tcam sw entry */
static unsigned int mvpp2_prs_tcam_port_map_get(struct mvpp2_prs_entry *pe)
{
	int enable_off = MVPP2_PRS_TCAM_EN_OFFS(MVPP2_PRS_TCAM_PORT_BYTE);

	return ~(pe->tcam.byte[enable_off]) & MVPP2_PRS_PORT_MASK;
}

/* Set byte of data and its enable bits in tcam sw entry */
static void mvpp2_prs_tcam_data_byte_set(struct mvpp2_prs_entry *pe,
					 unsigned int offs, unsigned char byte,
					 unsigned char enable)
{
	pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE(offs)] = byte;
	pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE_EN(offs)] = enable;
}

/* Get byte of data and its enable bits from tcam sw entry */
static void mvpp2_prs_tcam_data_byte_get(struct mvpp2_prs_entry *pe,
					 unsigned int offs, unsigned char *byte,
					 unsigned char *enable)
{
	*byte = pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE(offs)];
	*enable = pe->tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE_EN(offs)];
}

/* Set ethertype in tcam sw entry */
static void mvpp2_prs_match_etype(struct mvpp2_prs_entry *pe, int offset,
				  unsigned short ethertype)
{
	mvpp2_prs_tcam_data_byte_set(pe, offset + 0, ethertype >> 8, 0xff);
	mvpp2_prs_tcam_data_byte_set(pe, offset + 1, ethertype & 0xff, 0xff);
}

/* Set bits in sram sw entry */
static void mvpp2_prs_sram_bits_set(struct mvpp2_prs_entry *pe, int bit_num,
				    int val)
{
	pe->sram.byte[MVPP2_BIT_TO_BYTE(bit_num)] |= (val << (bit_num % 8));
}

/* Clear bits in sram sw entry */
static void mvpp2_prs_sram_bits_clear(struct mvpp2_prs_entry *pe, int bit_num,
				      int val)
{
	pe->sram.byte[MVPP2_BIT_TO_BYTE(bit_num)] &= ~(val << (bit_num % 8));
}

/* Update ri bits in sram sw entry */
static void mvpp2_prs_sram_ri_update(struct mvpp2_prs_entry *pe,
				     unsigned int bits, unsigned int mask)
{
	unsigned int i;

	for (i = 0; i < MVPP2_PRS_SRAM_RI_CTRL_BITS; i++) {
		int ri_off = MVPP2_PRS_SRAM_RI_OFFS;

		if (!(mask & BIT(i)))
			continue;

		if (bits & BIT(i))
			mvpp2_prs_sram_bits_set(pe, ri_off + i, 1);
		else
			mvpp2_prs_sram_bits_clear(pe, ri_off + i, 1);

		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_RI_CTRL_OFFS + i, 1);
	}
}

/* Update ai bits in sram sw entry */
static void mvpp2_prs_sram_ai_update(struct mvpp2_prs_entry *pe,
				     unsigned int bits, unsigned int mask)
{
	unsigned int i;
	int ai_off = MVPP2_PRS_SRAM_AI_OFFS;

	for (i = 0; i < MVPP2_PRS_SRAM_AI_CTRL_BITS; i++) {

		if (!(mask & BIT(i)))
			continue;

		if (bits & BIT(i))
			mvpp2_prs_sram_bits_set(pe, ai_off + i, 1);
		else
			mvpp2_prs_sram_bits_clear(pe, ai_off + i, 1);

		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_AI_CTRL_OFFS + i, 1);
	}
}

/* Read ai bits from sram sw entry */
static int mvpp2_prs_sram_ai_get(struct mvpp2_prs_entry *pe)
{
	u8 bits;
	int ai_off = MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_AI_OFFS);
	int ai_en_off = ai_off + 1;
	int ai_shift = MVPP2_PRS_SRAM_AI_OFFS % 8;

	bits = (pe->sram.byte[ai_off] >> ai_shift) |
	       (pe->sram.byte[ai_en_off] << (8 - ai_shift));

	return bits;
}

/* In sram sw entry set lookup ID field of the tcam key to be used in the next
 * lookup interation
 */
static void mvpp2_prs_sram_next_lu_set(struct mvpp2_prs_entry *pe,
				       unsigned int lu)
{
	int sram_next_off = MVPP2_PRS_SRAM_NEXT_LU_OFFS;

	mvpp2_prs_sram_bits_clear(pe, sram_next_off,
				  MVPP2_PRS_SRAM_NEXT_LU_MASK);
	mvpp2_prs_sram_bits_set(pe, sram_next_off, lu);
}

/* In the sram sw entry set sign and value of the next lookup offset
 * and the offset value generated to the classifier
 */
static void mvpp2_prs_sram_shift_set(struct mvpp2_prs_entry *pe, int shift,
				     unsigned int op)
{
	/* Set sign */
	if (shift < 0) {
		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_SHIFT_SIGN_BIT, 1);
		shift = 0 - shift;
	} else {
		mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_SHIFT_SIGN_BIT, 1);
	}

	/* Set value */
	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_SHIFT_OFFS)] =
							   (unsigned char)shift;

	/* Reset and set operation */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_SHIFT_OFFS,
				  MVPP2_PRS_SRAM_OP_SEL_SHIFT_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_OP_SEL_SHIFT_OFFS, op);

	/* Set base offset as current */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_BASE_OFFS, 1);
}

/* In the sram sw entry set sign and value of the user defined offset
 * generated to the classifier
 */
static void mvpp2_prs_sram_offset_set(struct mvpp2_prs_entry *pe,
				      unsigned int type, int offset,
				      unsigned int op)
{
	/* Set sign */
	if (offset < 0) {
		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_UDF_SIGN_BIT, 1);
		offset = 0 - offset;
	} else {
		mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_UDF_SIGN_BIT, 1);
	}

	/* Set value */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_UDF_OFFS,
				  MVPP2_PRS_SRAM_UDF_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_UDF_OFFS, offset);
	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_UDF_OFFS +
					MVPP2_PRS_SRAM_UDF_BITS)] &=
	      ~(MVPP2_PRS_SRAM_UDF_MASK >> (8 - (MVPP2_PRS_SRAM_UDF_OFFS % 8)));
	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_UDF_OFFS +
					MVPP2_PRS_SRAM_UDF_BITS)] |=
				(offset >> (8 - (MVPP2_PRS_SRAM_UDF_OFFS % 8)));

	/* Set offset type */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_UDF_TYPE_OFFS,
				  MVPP2_PRS_SRAM_UDF_TYPE_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_UDF_TYPE_OFFS, type);

	/* Set offset operation */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_MASK);
	mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS, op);

	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS +
					MVPP2_PRS_SRAM_OP_SEL_UDF_BITS)] &=
					     ~(MVPP2_PRS_SRAM_OP_SEL_UDF_MASK >>
				    (8 - (MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS % 8)));

	pe->sram.byte[MVPP2_BIT_TO_BYTE(MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS +
					MVPP2_PRS_SRAM_OP_SEL_UDF_BITS)] |=
			     (op >> (8 - (MVPP2_PRS_SRAM_OP_SEL_UDF_OFFS % 8)));

	/* Set base offset as current */
	mvpp2_prs_sram_bits_clear(pe, MVPP2_PRS_SRAM_OP_SEL_BASE_OFFS, 1);
}

/* Find parser flow entry */
static struct mvpp2_prs_entry *mvpp2_prs_flow_find(struct mvpp2 *priv, int flow)
{
	struct mvpp2_prs_entry *pe;
	int tid;

	pe = kzalloc(sizeof(*pe), GFP_KERNEL);
	if (!pe)
		return NULL;
	mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_FLOWS);

	/* Go through the all entires with MVPP2_PRS_LU_FLOWS */
	for (tid = MVPP2_PRS_TCAM_SRAM_SIZE - 1; tid >= 0; tid--) {
		u8 bits;

		if (!priv->prs_shadow[tid].valid ||
		    priv->prs_shadow[tid].lu != MVPP2_PRS_LU_FLOWS)
			continue;

		pe->index = tid;
		mvpp2_prs_hw_read(priv, pe);
		bits = mvpp2_prs_sram_ai_get(pe);

		/* Sram store classification lookup ID in AI bits [5:0] */
		if ((bits & MVPP2_PRS_FLOW_ID_MASK) == flow)
			return pe;
	}
	kfree(pe);

	return NULL;
}

/* Return first free tcam index, seeking from start to end */
static int mvpp2_prs_tcam_first_free(struct mvpp2 *priv, unsigned char start,
				     unsigned char end)
{
	int tid;

	if (start > end)
		swap(start, end);

	if (end >= MVPP2_PRS_TCAM_SRAM_SIZE)
		end = MVPP2_PRS_TCAM_SRAM_SIZE - 1;

	for (tid = start; tid <= end; tid++) {
		if (!priv->prs_shadow[tid].valid)
			return tid;
	}

	return -EINVAL;
}

/* Enable/disable dropping all mac da's */
static void mvpp2_prs_mac_drop_all_set(struct mvpp2 *priv, int port, bool add)
{
	struct mvpp2_prs_entry pe;

	if (priv->prs_shadow[MVPP2_PE_DROP_ALL].valid) {
		/* Entry exist - update port only */
		pe.index = MVPP2_PE_DROP_ALL;
		mvpp2_prs_hw_read(priv, &pe);
	} else {
		/* Entry doesn't exist - create new */
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);
		pe.index = MVPP2_PE_DROP_ALL;

		/* Non-promiscuous mode for all ports - DROP unknown packets */
		mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_DROP_MASK,
					 MVPP2_PRS_RI_DROP_MASK);

		mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
		mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);

		/* Update shadow table */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(&pe, port, add);

	mvpp2_prs_hw_write(priv, &pe);
}

/* Set port to promiscuous mode */
static void mvpp2_prs_mac_promisc_set(struct mvpp2 *priv, int port, bool add)
{
	struct mvpp2_prs_entry pe;

	/* Promiscuous mode - Accept unknown packets */

	if (priv->prs_shadow[MVPP2_PE_MAC_PROMISCUOUS].valid) {
		/* Entry exist - update port only */
		pe.index = MVPP2_PE_MAC_PROMISCUOUS;
		mvpp2_prs_hw_read(priv, &pe);
	} else {
		/* Entry doesn't exist - create new */
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);
		pe.index = MVPP2_PE_MAC_PROMISCUOUS;

		/* Continue - set next lookup */
		mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_DSA);

		/* Set result info bits */
		mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L2_UCAST,
					 MVPP2_PRS_RI_L2_CAST_MASK);

		/* Shift to ethertype */
		mvpp2_prs_sram_shift_set(&pe, 2 * ETH_ALEN,
					 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);

		/* Update shadow table */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(&pe, port, add);

	mvpp2_prs_hw_write(priv, &pe);
}

/* Accept multicast */
static void mvpp2_prs_mac_multi_set(struct mvpp2 *priv, int port, int index,
				    bool add)
{
	struct mvpp2_prs_entry pe;
	unsigned char da_mc;

	/* Ethernet multicast address first byte is
	 * 0x01 for IPv4 and 0x33 for IPv6
	 */
	da_mc = (index == MVPP2_PE_MAC_MC_ALL) ? 0x01 : 0x33;

	if (priv->prs_shadow[index].valid) {
		/* Entry exist - update port only */
		pe.index = index;
		mvpp2_prs_hw_read(priv, &pe);
	} else {
		/* Entry doesn't exist - create new */
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);
		pe.index = index;

		/* Continue - set next lookup */
		mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_DSA);

		/* Set result info bits */
		mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L2_MCAST,
					 MVPP2_PRS_RI_L2_CAST_MASK);

		/* Update tcam entry data first byte */
		mvpp2_prs_tcam_data_byte_set(&pe, 0, da_mc, 0xff);

		/* Shift to ethertype */
		mvpp2_prs_sram_shift_set(&pe, 2 * ETH_ALEN,
					 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);

		/* Update shadow table */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(&pe, port, add);

	mvpp2_prs_hw_write(priv, &pe);
}

/* Parser per-port initialization */
static void mvpp2_prs_hw_port_init(struct mvpp2 *priv, int port, int lu_first,
				   int lu_max, int offset)
{
	u32 val;

	/* Set lookup ID */
	val = mvpp2_read(priv, MVPP2_PRS_INIT_LOOKUP_REG);
	val &= ~MVPP2_PRS_PORT_LU_MASK(port);
	val |=  MVPP2_PRS_PORT_LU_VAL(port, lu_first);
	mvpp2_write(priv, MVPP2_PRS_INIT_LOOKUP_REG, val);

	/* Set maximum number of loops for packet received from port */
	val = mvpp2_read(priv, MVPP2_PRS_MAX_LOOP_REG(port));
	val &= ~MVPP2_PRS_MAX_LOOP_MASK(port);
	val |= MVPP2_PRS_MAX_LOOP_VAL(port, lu_max);
	mvpp2_write(priv, MVPP2_PRS_MAX_LOOP_REG(port), val);

	/* Set initial offset for packet header extraction for the first
	 * searching loop
	 */
	val = mvpp2_read(priv, MVPP2_PRS_INIT_OFFS_REG(port));
	val &= ~MVPP2_PRS_INIT_OFF_MASK(port);
	val |= MVPP2_PRS_INIT_OFF_VAL(port, offset);
	mvpp2_write(priv, MVPP2_PRS_INIT_OFFS_REG(port), val);
}

/* Default flow entries initialization for all ports */
static void mvpp2_prs_def_flow_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;
	int port;

	for (port = 0; port < MVPP2_MAX_PORTS; port++) {
		memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
		mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
		pe.index = MVPP2_PE_FIRST_DEFAULT_FLOW - port;

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(&pe, 0);

		/* Set flow ID*/
		mvpp2_prs_sram_ai_update(&pe, port, MVPP2_PRS_FLOW_ID_MASK);
		mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_DONE_BIT, 1);

		/* Update shadow table and hw entry */
		mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_FLOWS);
		mvpp2_prs_hw_write(priv, &pe);
	}
}

/* Set default entry for Marvell Header field */
static void mvpp2_prs_mh_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));

	pe.index = MVPP2_PE_MH_DEFAULT;
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MH);
	mvpp2_prs_sram_shift_set(&pe, MVPP2_MH_SIZE,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_MAC);

	/* Unmask all ports */
	mvpp2_prs_tcam_port_map_set(&pe, MVPP2_PRS_PORT_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MH);
	mvpp2_prs_hw_write(priv, &pe);
}

/* Set default entires (place holder) for promiscuous, non-promiscuous and
 * multicast MAC addresses
 */
static void mvpp2_prs_mac_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));

	/* Non-promiscuous mode for all ports - DROP unknown packets */
	pe.index = MVPP2_PE_MAC_NON_PROMISCUOUS;
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_MAC);

	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_DROP_MASK,
				 MVPP2_PRS_RI_DROP_MASK);
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);

	/* Unmask all ports */
	mvpp2_prs_tcam_port_map_set(&pe, MVPP2_PRS_PORT_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_MAC);
	mvpp2_prs_hw_write(priv, &pe);

	/* place holders only - no ports */
	mvpp2_prs_mac_drop_all_set(priv, 0, false);
	mvpp2_prs_mac_promisc_set(priv, 0, false);
	mvpp2_prs_mac_multi_set(priv, MVPP2_PE_MAC_MC_ALL, 0, false);
	mvpp2_prs_mac_multi_set(priv, MVPP2_PE_MAC_MC_IP6, 0, false);
}

/* Match basic ethertypes */
static int mvpp2_prs_etype_init(struct mvpp2 *priv)
{
	struct mvpp2_prs_entry pe;
	int tid;

	/* Ethertype: PPPoE */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_PPP_SES);

	mvpp2_prs_sram_shift_set(&pe, MVPP2_PPPOE_HDR_SIZE,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_PPPOE);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_PPPOE_MASK,
				 MVPP2_PRS_RI_PPPOE_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_PPPOE_MASK,
				MVPP2_PRS_RI_PPPOE_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: ARP */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_ARP);

	/* Generate flow in the next iteration*/
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_ARP,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = true;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_ARP,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: LBTD */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, MVPP2_IP_LBDT_TYPE);

	/* Generate flow in the next iteration*/
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_CPU_CODE_RX_SPEC |
				 MVPP2_PRS_RI_UDF3_RX_SPECIAL,
				 MVPP2_PRS_RI_CPU_CODE_MASK |
				 MVPP2_PRS_RI_UDF3_MASK);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = true;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_CPU_CODE_RX_SPEC |
				MVPP2_PRS_RI_UDF3_RX_SPECIAL,
				MVPP2_PRS_RI_CPU_CODE_MASK |
				MVPP2_PRS_RI_UDF3_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: IPv4 without options */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_IP);
	mvpp2_prs_tcam_data_byte_set(&pe, MVPP2_ETH_TYPE_LEN,
				     MVPP2_PRS_IPV4_HEAD | MVPP2_PRS_IPV4_IHL,
				     MVPP2_PRS_IPV4_HEAD_MASK |
				     MVPP2_PRS_IPV4_IHL_MASK);

	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_IP4);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_IP4,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Skip eth_type + 4 bytes of IP header */
	mvpp2_prs_sram_shift_set(&pe, MVPP2_ETH_TYPE_LEN + 4,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_IP4,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: IPv4 with options */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	pe.index = tid;

	/* Clear tcam data before updating */
	pe.tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE(MVPP2_ETH_TYPE_LEN)] = 0x0;
	pe.tcam.byte[MVPP2_PRS_TCAM_DATA_BYTE_EN(MVPP2_ETH_TYPE_LEN)] = 0x0;

	mvpp2_prs_tcam_data_byte_set(&pe, MVPP2_ETH_TYPE_LEN,
				     MVPP2_PRS_IPV4_HEAD,
				     MVPP2_PRS_IPV4_HEAD_MASK);

	/* Clear ri before updating */
	pe.sram.word[MVPP2_PRS_SRAM_RI_WORD] = 0x0;
	pe.sram.word[MVPP2_PRS_SRAM_RI_CTRL_WORD] = 0x0;
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_IP4_OPT,
				 MVPP2_PRS_RI_L3_PROTO_MASK);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_IP4_OPT,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Ethertype: IPv6 without options */
	tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
					MVPP2_PE_LAST_FREE_TID);
	if (tid < 0)
		return tid;

	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = tid;

	mvpp2_prs_match_etype(&pe, 0, PROT_IPV6);

	/* Skip DIP of IPV6 header */
	mvpp2_prs_sram_shift_set(&pe, MVPP2_ETH_TYPE_LEN + 8 +
				 MVPP2_MAX_L3_ADDR_SIZE,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_IP6);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_IP6,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Set L3 offset */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = false;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_IP6,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	/* Default entry for MVPP2_PRS_LU_L2 - Unknown ethtype */
	memset(&pe, 0, sizeof(struct mvpp2_prs_entry));
	mvpp2_prs_tcam_lu_set(&pe, MVPP2_PRS_LU_L2);
	pe.index = MVPP2_PE_ETH_TYPE_UN;

	/* Unmask all ports */
	mvpp2_prs_tcam_port_map_set(&pe, MVPP2_PRS_PORT_MASK);

	/* Generate flow in the next iteration*/
	mvpp2_prs_sram_bits_set(&pe, MVPP2_PRS_SRAM_LU_GEN_BIT, 1);
	mvpp2_prs_sram_next_lu_set(&pe, MVPP2_PRS_LU_FLOWS);
	mvpp2_prs_sram_ri_update(&pe, MVPP2_PRS_RI_L3_UN,
				 MVPP2_PRS_RI_L3_PROTO_MASK);
	/* Set L3 offset even it's unknown L3 */
	mvpp2_prs_sram_offset_set(&pe, MVPP2_PRS_SRAM_UDF_TYPE_L3,
				  MVPP2_ETH_TYPE_LEN,
				  MVPP2_PRS_SRAM_OP_SEL_UDF_ADD);

	/* Update shadow table and hw entry */
	mvpp2_prs_shadow_set(priv, pe.index, MVPP2_PRS_LU_L2);
	priv->prs_shadow[pe.index].udf = MVPP2_PRS_UDF_L2_DEF;
	priv->prs_shadow[pe.index].finish = true;
	mvpp2_prs_shadow_ri_set(priv, pe.index, MVPP2_PRS_RI_L3_UN,
				MVPP2_PRS_RI_L3_PROTO_MASK);
	mvpp2_prs_hw_write(priv, &pe);

	return 0;
}

/* Parser default initialization */
static int mvpp2_prs_default_init(struct udevice *dev,
				  struct mvpp2 *priv)
{
	int err, index, i;

	/* Enable tcam table */
	mvpp2_write(priv, MVPP2_PRS_TCAM_CTRL_REG, MVPP2_PRS_TCAM_EN_MASK);

	/* Clear all tcam and sram entries */
	for (index = 0; index < MVPP2_PRS_TCAM_SRAM_SIZE; index++) {
		mvpp2_write(priv, MVPP2_PRS_TCAM_IDX_REG, index);
		for (i = 0; i < MVPP2_PRS_TCAM_WORDS; i++)
			mvpp2_write(priv, MVPP2_PRS_TCAM_DATA_REG(i), 0);

		mvpp2_write(priv, MVPP2_PRS_SRAM_IDX_REG, index);
		for (i = 0; i < MVPP2_PRS_SRAM_WORDS; i++)
			mvpp2_write(priv, MVPP2_PRS_SRAM_DATA_REG(i), 0);
	}

	/* Invalidate all tcam entries */
	for (index = 0; index < MVPP2_PRS_TCAM_SRAM_SIZE; index++)
		mvpp2_prs_hw_inv(priv, index);

	priv->prs_shadow = devm_kcalloc(dev, MVPP2_PRS_TCAM_SRAM_SIZE,
					sizeof(struct mvpp2_prs_shadow),
					GFP_KERNEL);
	if (!priv->prs_shadow)
		return -ENOMEM;

	/* Always start from lookup = 0 */
	for (index = 0; index < MVPP2_MAX_PORTS; index++)
		mvpp2_prs_hw_port_init(priv, index, MVPP2_PRS_LU_MH,
				       MVPP2_PRS_PORT_LU_MAX, 0);

	mvpp2_prs_def_flow_init(priv);

	mvpp2_prs_mh_init(priv);

	mvpp2_prs_mac_init(priv);

	err = mvpp2_prs_etype_init(priv);
	if (err)
		return err;

	return 0;
}

/* Compare MAC DA with tcam entry data */
static bool mvpp2_prs_mac_range_equals(struct mvpp2_prs_entry *pe,
				       const u8 *da, unsigned char *mask)
{
	unsigned char tcam_byte, tcam_mask;
	int index;

	for (index = 0; index < ETH_ALEN; index++) {
		mvpp2_prs_tcam_data_byte_get(pe, index, &tcam_byte, &tcam_mask);
		if (tcam_mask != mask[index])
			return false;

		if ((tcam_mask & tcam_byte) != (da[index] & mask[index]))
			return false;
	}

	return true;
}

/* Find tcam entry with matched pair <MAC DA, port> */
static struct mvpp2_prs_entry *
mvpp2_prs_mac_da_range_find(struct mvpp2 *priv, int pmap, const u8 *da,
			    unsigned char *mask, int udf_type)
{
	struct mvpp2_prs_entry *pe;
	int tid;

	pe = kzalloc(sizeof(*pe), GFP_KERNEL);
	if (!pe)
		return NULL;
	mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_MAC);

	/* Go through the all entires with MVPP2_PRS_LU_MAC */
	for (tid = MVPP2_PE_FIRST_FREE_TID;
	     tid <= MVPP2_PE_LAST_FREE_TID; tid++) {
		unsigned int entry_pmap;

		if (!priv->prs_shadow[tid].valid ||
		    (priv->prs_shadow[tid].lu != MVPP2_PRS_LU_MAC) ||
		    (priv->prs_shadow[tid].udf != udf_type))
			continue;

		pe->index = tid;
		mvpp2_prs_hw_read(priv, pe);
		entry_pmap = mvpp2_prs_tcam_port_map_get(pe);

		if (mvpp2_prs_mac_range_equals(pe, da, mask) &&
		    entry_pmap == pmap)
			return pe;
	}
	kfree(pe);

	return NULL;
}

/* Update parser's mac da entry */
static int mvpp2_prs_mac_da_accept(struct mvpp2 *priv, int port,
				   const u8 *da, bool add)
{
	struct mvpp2_prs_entry *pe;
	unsigned int pmap, len, ri;
	unsigned char mask[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int tid;

	/* Scan TCAM and see if entry with this <MAC DA, port> already exist */
	pe = mvpp2_prs_mac_da_range_find(priv, (1 << port), da, mask,
					 MVPP2_PRS_UDF_MAC_DEF);

	/* No such entry */
	if (!pe) {
		if (!add)
			return 0;

		/* Create new TCAM entry */
		/* Find first range mac entry*/
		for (tid = MVPP2_PE_FIRST_FREE_TID;
		     tid <= MVPP2_PE_LAST_FREE_TID; tid++)
			if (priv->prs_shadow[tid].valid &&
			    (priv->prs_shadow[tid].lu == MVPP2_PRS_LU_MAC) &&
			    (priv->prs_shadow[tid].udf ==
						       MVPP2_PRS_UDF_MAC_RANGE))
				break;

		/* Go through the all entries from first to last */
		tid = mvpp2_prs_tcam_first_free(priv, MVPP2_PE_FIRST_FREE_TID,
						tid - 1);
		if (tid < 0)
			return tid;

		pe = kzalloc(sizeof(*pe), GFP_KERNEL);
		if (!pe)
			return -1;
		mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_MAC);
		pe->index = tid;

		/* Mask all ports */
		mvpp2_prs_tcam_port_map_set(pe, 0);
	}

	/* Update port mask */
	mvpp2_prs_tcam_port_set(pe, port, add);

	/* Invalidate the entry if no ports are left enabled */
	pmap = mvpp2_prs_tcam_port_map_get(pe);
	if (pmap == 0) {
		if (add) {
			kfree(pe);
			return -1;
		}
		mvpp2_prs_hw_inv(priv, pe->index);
		priv->prs_shadow[pe->index].valid = false;
		kfree(pe);
		return 0;
	}

	/* Continue - set next lookup */
	mvpp2_prs_sram_next_lu_set(pe, MVPP2_PRS_LU_DSA);

	/* Set match on DA */
	len = ETH_ALEN;
	while (len--)
		mvpp2_prs_tcam_data_byte_set(pe, len, da[len], 0xff);

	/* Set result info bits */
	ri = MVPP2_PRS_RI_L2_UCAST | MVPP2_PRS_RI_MAC_ME_MASK;

	mvpp2_prs_sram_ri_update(pe, ri, MVPP2_PRS_RI_L2_CAST_MASK |
				 MVPP2_PRS_RI_MAC_ME_MASK);
	mvpp2_prs_shadow_ri_set(priv, pe->index, ri, MVPP2_PRS_RI_L2_CAST_MASK |
				MVPP2_PRS_RI_MAC_ME_MASK);

	/* Shift to ethertype */
	mvpp2_prs_sram_shift_set(pe, 2 * ETH_ALEN,
				 MVPP2_PRS_SRAM_OP_SEL_SHIFT_ADD);

	/* Update shadow table and hw entry */
	priv->prs_shadow[pe->index].udf = MVPP2_PRS_UDF_MAC_DEF;
	mvpp2_prs_shadow_set(priv, pe->index, MVPP2_PRS_LU_MAC);
	mvpp2_prs_hw_write(priv, pe);

	kfree(pe);

	return 0;
}

static int mvpp2_prs_update_mac_da(struct mvpp2_port *port, const u8 *da)
{
	int err;

	/* Remove old parser entry */
	err = mvpp2_prs_mac_da_accept(port->priv, port->id, port->dev_addr,
				      false);
	if (err)
		return err;

	/* Add new parser entry */
	err = mvpp2_prs_mac_da_accept(port->priv, port->id, da, true);
	if (err)
		return err;

	/* Set addr in the device */
	memcpy(port->dev_addr, da, ETH_ALEN);

	return 0;
}

/* Set prs flow for the port */
static int mvpp2_prs_def_flow(struct mvpp2_port *port)
{
	struct mvpp2_prs_entry *pe;
	int tid;

	pe = mvpp2_prs_flow_find(port->priv, port->id);

	/* Such entry not exist */
	if (!pe) {
		/* Go through the all entires from last to first */
		tid = mvpp2_prs_tcam_first_free(port->priv,
						MVPP2_PE_LAST_FREE_TID,
					       MVPP2_PE_FIRST_FREE_TID);
		if (tid < 0)
			return tid;

		pe = kzalloc(sizeof(*pe), GFP_KERNEL);
		if (!pe)
			return -ENOMEM;

		mvpp2_prs_tcam_lu_set(pe, MVPP2_PRS_LU_FLOWS);
		pe->index = tid;

		/* Set flow ID*/
		mvpp2_prs_sram_ai_update(pe, port->id, MVPP2_PRS_FLOW_ID_MASK);
		mvpp2_prs_sram_bits_set(pe, MVPP2_PRS_SRAM_LU_DONE_BIT, 1);

		/* Update shadow table */
		mvpp2_prs_shadow_set(port->priv, pe->index, MVPP2_PRS_LU_FLOWS);
	}

	mvpp2_prs_tcam_port_map_set(pe, (1 << port->id));
	mvpp2_prs_hw_write(port->priv, pe);
	kfree(pe);

	return 0;
}

/* Classifier configuration routines */

/* Update classification flow table registers */
static void mvpp2_cls_flow_write(struct mvpp2 *priv,
				 struct mvpp2_cls_flow_entry *fe)
{
	mvpp2_write(priv, MVPP2_CLS_FLOW_INDEX_REG, fe->index);
	mvpp2_write(priv, MVPP2_CLS_FLOW_TBL0_REG,  fe->data[0]);
	mvpp2_write(priv, MVPP2_CLS_FLOW_TBL1_REG,  fe->data[1]);
	mvpp2_write(priv, MVPP2_CLS_FLOW_TBL2_REG,  fe->data[2]);
}

/* Update classification lookup table register */
static void mvpp2_cls_lookup_write(struct mvpp2 *priv,
				   struct mvpp2_cls_lookup_entry *le)
{
	u32 val;

	val = (le->way << MVPP2_CLS_LKP_INDEX_WAY_OFFS) | le->lkpid;
	mvpp2_write(priv, MVPP2_CLS_LKP_INDEX_REG, val);
	mvpp2_write(priv, MVPP2_CLS_LKP_TBL_REG, le->data);
}

/* Classifier default initialization */
static void mvpp2_cls_init(struct mvpp2 *priv)
{
	struct mvpp2_cls_lookup_entry le;
	struct mvpp2_cls_flow_entry fe;
	int index;

	/* Enable classifier */
	mvpp2_write(priv, MVPP2_CLS_MODE_REG, MVPP2_CLS_MODE_ACTIVE_MASK);

	/* Clear classifier flow table */
	memset(&fe.data, 0, MVPP2_CLS_FLOWS_TBL_DATA_WORDS);
	for (index = 0; index < MVPP2_CLS_FLOWS_TBL_SIZE; index++) {
		fe.index = index;
		mvpp2_cls_flow_write(priv, &fe);
	}

	/* Clear classifier lookup table */
	le.data = 0;
	for (index = 0; index < MVPP2_CLS_LKP_TBL_SIZE; index++) {
		le.lkpid = index;
		le.way = 0;
		mvpp2_cls_lookup_write(priv, &le);

		le.way = 1;
		mvpp2_cls_lookup_write(priv, &le);
	}
}

static void mvpp2_cls_port_config(struct mvpp2_port *port)
{
	struct mvpp2_cls_lookup_entry le;
	u32 val;

	/* Set way for the port */
	val = mvpp2_read(port->priv, MVPP2_CLS_PORT_WAY_REG);
	val &= ~MVPP2_CLS_PORT_WAY_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_CLS_PORT_WAY_REG, val);

	/* Pick the entry to be accessed in lookup ID decoding table
	 * according to the way and lkpid.
	 */
	le.lkpid = port->id;
	le.way = 0;
	le.data = 0;

	/* Set initial CPU queue for receiving packets */
	le.data &= ~MVPP2_CLS_LKP_TBL_RXQ_MASK;
	le.data |= port->first_rxq;

	/* Disable classification engines */
	le.data &= ~MVPP2_CLS_LKP_TBL_LOOKUP_EN_MASK;

	/* Update lookup ID table entry */
	mvpp2_cls_lookup_write(port->priv, &le);
}

/* Set CPU queue number for oversize packets */
static void mvpp2_cls_oversize_rxq_set(struct mvpp2_port *port)
{
	u32 val;

	mvpp2_write(port->priv, MVPP2_CLS_OVERSIZE_RXQ_LOW_REG(port->id),
		    port->first_rxq & MVPP2_CLS_OVERSIZE_RXQ_LOW_MASK);

	mvpp2_write(port->priv, MVPP2_CLS_SWFWD_P2HQ_REG(port->id),
		    (port->first_rxq >> MVPP2_CLS_OVERSIZE_RXQ_LOW_BITS));

	val = mvpp2_read(port->priv, MVPP2_CLS_SWFWD_PCTRL_REG);
	val |= MVPP2_CLS_SWFWD_PCTRL_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_CLS_SWFWD_PCTRL_REG, val);
}

/* Buffer Manager configuration routines */

/* Create pool */
static int mvpp2_bm_pool_create(struct udevice *dev,
				struct mvpp2 *priv,
				struct mvpp2_bm_pool *bm_pool, int size)
{
	u32 val;

	/* Number of buffer pointers must be a multiple of 16, as per
	 * hardware constraints
	 */
	if (!IS_ALIGNED(size, 16))
		return -EINVAL;

	bm_pool->virt_addr = buffer_loc.bm_pool[bm_pool->id];
	bm_pool->dma_addr = (dma_addr_t)buffer_loc.bm_pool[bm_pool->id];
	if (!bm_pool->virt_addr)
		return -ENOMEM;

	if (!IS_ALIGNED((unsigned long)bm_pool->virt_addr,
			MVPP2_BM_POOL_PTR_ALIGN)) {
		dev_err(&pdev->dev, "BM pool %d is not %d bytes aligned\n",
			bm_pool->id, MVPP2_BM_POOL_PTR_ALIGN);
		return -ENOMEM;
	}

	mvpp2_write(priv, MVPP2_BM_POOL_BASE_REG(bm_pool->id),
		    lower_32_bits(bm_pool->dma_addr));
	if (priv->hw_version == MVPP22)
		mvpp2_write(priv, MVPP22_BM_POOL_BASE_HIGH_REG,
			    (upper_32_bits(bm_pool->dma_addr) &
			    MVPP22_BM_POOL_BASE_HIGH_MASK));
	mvpp2_write(priv, MVPP2_BM_POOL_SIZE_REG(bm_pool->id), size);

	val = mvpp2_read(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id));
	val |= MVPP2_BM_START_MASK;
	mvpp2_write(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id), val);

	bm_pool->type = MVPP2_BM_FREE;
	bm_pool->size = size;
	bm_pool->pkt_size = 0;
	bm_pool->buf_num = 0;

	return 0;
}

/* Set pool buffer size */
static void mvpp2_bm_pool_bufsize_set(struct mvpp2 *priv,
				      struct mvpp2_bm_pool *bm_pool,
				      int buf_size)
{
	u32 val;

	bm_pool->buf_size = buf_size;

	val = ALIGN(buf_size, 1 << MVPP2_POOL_BUF_SIZE_OFFSET);
	mvpp2_write(priv, MVPP2_POOL_BUF_SIZE_REG(bm_pool->id), val);
}

/* Free all buffers from the pool */
static void mvpp2_bm_bufs_free(struct udevice *dev, struct mvpp2 *priv,
			       struct mvpp2_bm_pool *bm_pool)
{
	int i;

	for (i = 0; i < bm_pool->buf_num; i++) {
		/* Allocate buffer back from the buffer manager */
		mvpp2_read(priv, MVPP2_BM_PHY_ALLOC_REG(bm_pool->id));
	}

	bm_pool->buf_num = 0;
}

/* Cleanup pool */
static int mvpp2_bm_pool_destroy(struct udevice *dev,
				 struct mvpp2 *priv,
				 struct mvpp2_bm_pool *bm_pool)
{
	u32 val;

	mvpp2_bm_bufs_free(dev, priv, bm_pool);
	if (bm_pool->buf_num) {
		dev_err(dev, "cannot free all buffers in pool %d\n", bm_pool->id);
		return 0;
	}

	val = mvpp2_read(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id));
	val |= MVPP2_BM_STOP_MASK;
	mvpp2_write(priv, MVPP2_BM_POOL_CTRL_REG(bm_pool->id), val);

	return 0;
}

static int mvpp2_bm_pools_init(struct udevice *dev,
			       struct mvpp2 *priv)
{
	int i, err, size;
	struct mvpp2_bm_pool *bm_pool;

	/* Create all pools with maximum size */
	size = MVPP2_BM_POOL_SIZE_MAX;
	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++) {
		bm_pool = &priv->bm_pools[i];
		bm_pool->id = i;
		err = mvpp2_bm_pool_create(dev, priv, bm_pool, size);
		if (err)
			goto err_unroll_pools;
		mvpp2_bm_pool_bufsize_set(priv, bm_pool, RX_BUFFER_SIZE);
	}
	return 0;

err_unroll_pools:
	dev_err(&pdev->dev, "failed to create BM pool %d, size %d\n", i, size);
	for (i = i - 1; i >= 0; i--)
		mvpp2_bm_pool_destroy(dev, priv, &priv->bm_pools[i]);
	return err;
}

static int mvpp2_bm_init(struct udevice *dev, struct mvpp2 *priv)
{
	int i, err;

	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++) {
		/* Mask BM all interrupts */
		mvpp2_write(priv, MVPP2_BM_INTR_MASK_REG(i), 0);
		/* Clear BM cause register */
		mvpp2_write(priv, MVPP2_BM_INTR_CAUSE_REG(i), 0);
	}

	/* Allocate and initialize BM pools */
	priv->bm_pools = devm_kcalloc(dev, MVPP2_BM_POOLS_NUM,
				     sizeof(struct mvpp2_bm_pool), GFP_KERNEL);
	if (!priv->bm_pools)
		return -ENOMEM;

	err = mvpp2_bm_pools_init(dev, priv);
	if (err < 0)
		return err;
	return 0;
}

/* Attach long pool to rxq */
static void mvpp2_rxq_long_pool_set(struct mvpp2_port *port,
				    int lrxq, int long_pool)
{
	u32 val, mask;
	int prxq;

	/* Get queue physical ID */
	prxq = port->rxqs[lrxq]->id;

	if (port->priv->hw_version == MVPP21)
		mask = MVPP21_RXQ_POOL_LONG_MASK;
	else
		mask = MVPP22_RXQ_POOL_LONG_MASK;

	val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(prxq));
	val &= ~mask;
	val |= (long_pool << MVPP2_RXQ_POOL_LONG_OFFS) & mask;
	mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(prxq), val);
}

/* Set pool number in a BM cookie */
static inline u32 mvpp2_bm_cookie_pool_set(u32 cookie, int pool)
{
	u32 bm;

	bm = cookie & ~(0xFF << MVPP2_BM_COOKIE_POOL_OFFS);
	bm |= ((pool & 0xFF) << MVPP2_BM_COOKIE_POOL_OFFS);

	return bm;
}

/* Get pool number from a BM cookie */
static inline int mvpp2_bm_cookie_pool_get(unsigned long cookie)
{
	return (cookie >> MVPP2_BM_COOKIE_POOL_OFFS) & 0xFF;
}

/* Release buffer to BM */
static inline void mvpp2_bm_pool_put(struct mvpp2_port *port, int pool,
				     dma_addr_t buf_dma_addr,
				     unsigned long buf_phys_addr)
{
	if (port->priv->hw_version == MVPP22) {
		u32 val = 0;

		if (sizeof(dma_addr_t) == 8)
			val |= upper_32_bits(buf_dma_addr) &
				MVPP22_BM_ADDR_HIGH_PHYS_RLS_MASK;

		if (sizeof(phys_addr_t) == 8)
			val |= (upper_32_bits(buf_phys_addr)
				<< MVPP22_BM_ADDR_HIGH_VIRT_RLS_SHIFT) &
				MVPP22_BM_ADDR_HIGH_VIRT_RLS_MASK;

		mvpp2_write(port->priv, MVPP22_BM_ADDR_HIGH_RLS_REG, val);
	}

	/* MVPP2_BM_VIRT_RLS_REG is not interpreted by HW, and simply
	 * returned in the "cookie" field of the RX
	 * descriptor. Instead of storing the virtual address, we
	 * store the physical address
	 */
	mvpp2_write(port->priv, MVPP2_BM_VIRT_RLS_REG, buf_phys_addr);
	mvpp2_write(port->priv, MVPP2_BM_PHY_RLS_REG(pool), buf_dma_addr);
}

/* Refill BM pool */
static void mvpp2_pool_refill(struct mvpp2_port *port, u32 bm,
			      dma_addr_t dma_addr,
			      phys_addr_t phys_addr)
{
	int pool = mvpp2_bm_cookie_pool_get(bm);

	mvpp2_bm_pool_put(port, pool, dma_addr, phys_addr);
}

/* Allocate buffers for the pool */
static int mvpp2_bm_bufs_add(struct mvpp2_port *port,
			     struct mvpp2_bm_pool *bm_pool, int buf_num)
{
	int i;

	if (buf_num < 0 ||
	    (buf_num + bm_pool->buf_num > bm_pool->size)) {
		netdev_err(port->dev,
			   "cannot allocate %d buffers for pool %d\n",
			   buf_num, bm_pool->id);
		return 0;
	}

	for (i = 0; i < buf_num; i++) {
		mvpp2_bm_pool_put(port, bm_pool->id,
				  (dma_addr_t)buffer_loc.rx_buffer[i],
				  (unsigned long)buffer_loc.rx_buffer[i]);

	}

	/* Update BM driver with number of buffers added to pool */
	bm_pool->buf_num += i;

	return i;
}

/* Notify the driver that BM pool is being used as specific type and return the
 * pool pointer on success
 */
static struct mvpp2_bm_pool *
mvpp2_bm_pool_use(struct mvpp2_port *port, int pool, enum mvpp2_bm_type type,
		  int pkt_size)
{
	struct mvpp2_bm_pool *new_pool = &port->priv->bm_pools[pool];
	int num;

	if (new_pool->type != MVPP2_BM_FREE && new_pool->type != type) {
		netdev_err(port->dev, "mixing pool types is forbidden\n");
		return NULL;
	}

	if (new_pool->type == MVPP2_BM_FREE)
		new_pool->type = type;

	/* Allocate buffers in case BM pool is used as long pool, but packet
	 * size doesn't match MTU or BM pool hasn't being used yet
	 */
	if (((type == MVPP2_BM_SWF_LONG) && (pkt_size > new_pool->pkt_size)) ||
	    (new_pool->pkt_size == 0)) {
		int pkts_num;

		/* Set default buffer number or free all the buffers in case
		 * the pool is not empty
		 */
		pkts_num = new_pool->buf_num;
		if (pkts_num == 0)
			pkts_num = type == MVPP2_BM_SWF_LONG ?
				   MVPP2_BM_LONG_BUF_NUM :
				   MVPP2_BM_SHORT_BUF_NUM;
		else
			mvpp2_bm_bufs_free(NULL,
					   port->priv, new_pool);

		new_pool->pkt_size = pkt_size;

		/* Allocate buffers for this pool */
		num = mvpp2_bm_bufs_add(port, new_pool, pkts_num);
		if (num != pkts_num) {
			dev_err(dev, "pool %d: %d of %d allocated\n",
				new_pool->id, num, pkts_num);
			return NULL;
		}
	}

	return new_pool;
}

/* Initialize pools for swf */
static int mvpp2_swf_bm_pool_init(struct mvpp2_port *port)
{
	int rxq;

	if (!port->pool_long) {
		port->pool_long =
		       mvpp2_bm_pool_use(port, MVPP2_BM_SWF_LONG_POOL(port->id),
					 MVPP2_BM_SWF_LONG,
					 port->pkt_size);
		if (!port->pool_long)
			return -ENOMEM;

		port->pool_long->port_map |= (1 << port->id);

		for (rxq = 0; rxq < rxq_number; rxq++)
			mvpp2_rxq_long_pool_set(port, rxq, port->pool_long->id);
	}

	return 0;
}

/* Port configuration routines */

static void mvpp2_port_mii_set(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG);

	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
		val |= MVPP2_GMAC_INBAND_AN_MASK;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		val |= MVPP2_GMAC_PORT_RGMII_MASK;
	default:
		val &= ~MVPP2_GMAC_PCS_ENABLE_MASK;
	}

	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);
}

static void mvpp2_port_fc_adv_enable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_AUTONEG_CONFIG);
	val |= MVPP2_GMAC_FC_ADV_EN;
	writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
}

static void mvpp2_port_enable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val |= MVPP2_GMAC_PORT_EN_MASK;
	val |= MVPP2_GMAC_MIB_CNTR_EN_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);
}

static void mvpp2_port_disable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val &= ~(MVPP2_GMAC_PORT_EN_MASK);
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);
}

/* Set IEEE 802.3x Flow Control Xon Packet Transmission Mode */
static void mvpp2_port_periodic_xon_disable(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_1_REG) &
		    ~MVPP2_GMAC_PERIODIC_XON_EN_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_1_REG);
}

/* Configure loopback port */
static void mvpp2_port_loopback_set(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_1_REG);

	if (port->speed == 1000)
		val |= MVPP2_GMAC_GMII_LB_EN_MASK;
	else
		val &= ~MVPP2_GMAC_GMII_LB_EN_MASK;

	if (port->phy_interface == PHY_INTERFACE_MODE_SGMII)
		val |= MVPP2_GMAC_PCS_LB_EN_MASK;
	else
		val &= ~MVPP2_GMAC_PCS_LB_EN_MASK;

	writel(val, port->base + MVPP2_GMAC_CTRL_1_REG);
}

static void mvpp2_port_reset(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG) &
		    ~MVPP2_GMAC_PORT_RESET_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);

	while (readl(port->base + MVPP2_GMAC_CTRL_2_REG) &
	       MVPP2_GMAC_PORT_RESET_MASK)
		continue;
}

/* Change maximum receive size of the port */
static inline void mvpp2_gmac_max_rx_size_set(struct mvpp2_port *port)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val &= ~MVPP2_GMAC_MAX_RX_SIZE_MASK;
	val |= (((port->pkt_size - MVPP2_MH_SIZE) / 2) <<
		    MVPP2_GMAC_MAX_RX_SIZE_OFFS);
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);
}

/* PPv2.2 GoP/GMAC config */

/* Set the MAC to reset or exit from reset */
static int gop_gmac_reset(struct mvpp2_port *port, int reset)
{
	u32 val;

	/* read - modify - write */
	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG);
	if (reset)
		val |= MVPP2_GMAC_PORT_RESET_MASK;
	else
		val &= ~MVPP2_GMAC_PORT_RESET_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);

	return 0;
}

/*
 * gop_gpcs_mode_cfg
 *
 * Configure port to working with Gig PCS or don't.
 */
static int gop_gpcs_mode_cfg(struct mvpp2_port *port, int en)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG);
	if (en)
		val |= MVPP2_GMAC_PCS_ENABLE_MASK;
	else
		val &= ~MVPP2_GMAC_PCS_ENABLE_MASK;
	/* enable / disable PCS on this port */
	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);

	return 0;
}

static int gop_bypass_clk_cfg(struct mvpp2_port *port, int en)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG);
	if (en)
		val |= MVPP2_GMAC_CLK_125_BYPS_EN_MASK;
	else
		val &= ~MVPP2_GMAC_CLK_125_BYPS_EN_MASK;
	/* enable / disable PCS on this port */
	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);

	return 0;
}

static void gop_gmac_sgmii2_5_cfg(struct mvpp2_port *port)
{
	u32 val, thresh;

	/*
	 * Configure minimal level of the Tx FIFO before the lower part
	 * starts to read a packet
	 */
	thresh = MVPP2_SGMII2_5_TX_FIFO_MIN_TH;
	val = readl(port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);
	val &= ~MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK;
	val |= MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(thresh);
	writel(val, port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);

	/* Disable bypass of sync module */
	val = readl(port->base + MVPP2_GMAC_CTRL_4_REG);
	val |= MVPP2_GMAC_CTRL4_SYNC_BYPASS_MASK;
	/* configure DP clock select according to mode */
	val |= MVPP2_GMAC_CTRL4_DP_CLK_SEL_MASK;
	/* configure QSGMII bypass according to mode */
	val |= MVPP2_GMAC_CTRL4_QSGMII_BYPASS_ACTIVE_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_4_REG);

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	/*
	 * Configure GIG MAC to 1000Base-X mode connected to a fiber
	 * transceiver
	 */
	val |= MVPP2_GMAC_PORT_TYPE_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);

	/* configure AN 0x9268 */
	val = MVPP2_GMAC_EN_PCS_AN |
		MVPP2_GMAC_AN_BYPASS_EN |
		MVPP2_GMAC_CONFIG_MII_SPEED  |
		MVPP2_GMAC_CONFIG_GMII_SPEED     |
		MVPP2_GMAC_FC_ADV_EN    |
		MVPP2_GMAC_CONFIG_FULL_DUPLEX |
		MVPP2_GMAC_CHOOSE_SAMPLE_TX_CONFIG;
	writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
}

static void gop_gmac_sgmii_cfg(struct mvpp2_port *port)
{
	u32 val, thresh;

	/*
	 * Configure minimal level of the Tx FIFO before the lower part
	 * starts to read a packet
	 */
	thresh = MVPP2_SGMII_TX_FIFO_MIN_TH;
	val = readl(port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);
	val &= ~MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK;
	val |= MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(thresh);
	writel(val, port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);

	/* Disable bypass of sync module */
	val = readl(port->base + MVPP2_GMAC_CTRL_4_REG);
	val |= MVPP2_GMAC_CTRL4_SYNC_BYPASS_MASK;
	/* configure DP clock select according to mode */
	val &= ~MVPP2_GMAC_CTRL4_DP_CLK_SEL_MASK;
	/* configure QSGMII bypass according to mode */
	val |= MVPP2_GMAC_CTRL4_QSGMII_BYPASS_ACTIVE_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_4_REG);

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	/* configure GIG MAC to SGMII mode */
	val &= ~MVPP2_GMAC_PORT_TYPE_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);

	/* configure AN */
	val = MVPP2_GMAC_EN_PCS_AN |
		MVPP2_GMAC_AN_BYPASS_EN |
		MVPP2_GMAC_AN_SPEED_EN  |
		MVPP2_GMAC_EN_FC_AN     |
		MVPP2_GMAC_AN_DUPLEX_EN |
		MVPP2_GMAC_CHOOSE_SAMPLE_TX_CONFIG;
	writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
}

static void gop_gmac_rgmii_cfg(struct mvpp2_port *port)
{
	u32 val, thresh;

	/*
	 * Configure minimal level of the Tx FIFO before the lower part
	 * starts to read a packet
	 */
	thresh = MVPP2_RGMII_TX_FIFO_MIN_TH;
	val = readl(port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);
	val &= ~MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK;
	val |= MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(thresh);
	writel(val, port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);

	/* Disable bypass of sync module */
	val = readl(port->base + MVPP2_GMAC_CTRL_4_REG);
	val |= MVPP2_GMAC_CTRL4_SYNC_BYPASS_MASK;
	/* configure DP clock select according to mode */
	val &= ~MVPP2_GMAC_CTRL4_DP_CLK_SEL_MASK;
	val |= MVPP2_GMAC_CTRL4_QSGMII_BYPASS_ACTIVE_MASK;
	val |= MVPP2_GMAC_CTRL4_EXT_PIN_GMII_SEL_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_4_REG);

	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	/* configure GIG MAC to SGMII mode */
	val &= ~MVPP2_GMAC_PORT_TYPE_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);

	/* configure AN 0xb8e8 */
	val = MVPP2_GMAC_AN_BYPASS_EN |
		MVPP2_GMAC_AN_SPEED_EN   |
		MVPP2_GMAC_EN_FC_AN      |
		MVPP2_GMAC_AN_DUPLEX_EN  |
		MVPP2_GMAC_CHOOSE_SAMPLE_TX_CONFIG;
	writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
}

/* Set the internal mux's to the required MAC in the GOP */
static int gop_gmac_mode_cfg(struct mvpp2_port *port)
{
	u32 val;

	/* Set TX FIFO thresholds */
	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
		if (port->phy_speed == 2500)
			gop_gmac_sgmii2_5_cfg(port);
		else
			gop_gmac_sgmii_cfg(port);
		break;

	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		gop_gmac_rgmii_cfg(port);
		break;

	default:
		return -1;
	}

	/* Jumbo frame support - 0x1400*2= 0x2800 bytes */
	val = readl(port->base + MVPP2_GMAC_CTRL_0_REG);
	val &= ~MVPP2_GMAC_MAX_RX_SIZE_MASK;
	val |= 0x1400 << MVPP2_GMAC_MAX_RX_SIZE_OFFS;
	writel(val, port->base + MVPP2_GMAC_CTRL_0_REG);

	/* PeriodicXonEn disable */
	val = readl(port->base + MVPP2_GMAC_CTRL_1_REG);
	val &= ~MVPP2_GMAC_PERIODIC_XON_EN_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_1_REG);

	return 0;
}

static void gop_xlg_2_gig_mac_cfg(struct mvpp2_port *port)
{
	u32 val;

	/* relevant only for MAC0 (XLG0 and GMAC0) */
	if (port->gop_id > 0)
		return;

	/* configure 1Gig MAC mode */
	val = readl(port->base + MVPP22_XLG_CTRL3_REG);
	val &= ~MVPP22_XLG_CTRL3_MACMODESELECT_MASK;
	val |= MVPP22_XLG_CTRL3_MACMODESELECT_GMAC;
	writel(val, port->base + MVPP22_XLG_CTRL3_REG);
}

static int gop_gpcs_reset(struct mvpp2_port *port, int reset)
{
	u32 val;

	val = readl(port->base + MVPP2_GMAC_CTRL_2_REG);
	if (reset)
		val &= ~MVPP2_GMAC_SGMII_MODE_MASK;
	else
		val |= MVPP2_GMAC_SGMII_MODE_MASK;
	writel(val, port->base + MVPP2_GMAC_CTRL_2_REG);

	return 0;
}

/* Set the internal mux's to the required PCS in the PI */
static int gop_xpcs_mode(struct mvpp2_port *port, int num_of_lanes)
{
	u32 val;
	int lane;

	switch (num_of_lanes) {
	case 1:
		lane = 0;
		break;
	case 2:
		lane = 1;
		break;
	case 4:
		lane = 2;
		break;
	default:
		return -1;
	}

	/* configure XG MAC mode */
	val = readl(port->priv->xpcs_base + MVPP22_XPCS_GLOBAL_CFG_0_REG);
	val &= ~MVPP22_XPCS_PCSMODE_MASK;
	val &= ~MVPP22_XPCS_LANEACTIVE_MASK;
	val |= (2 * lane) << MVPP22_XPCS_LANEACTIVE_OFFS;
	writel(val, port->priv->xpcs_base + MVPP22_XPCS_GLOBAL_CFG_0_REG);

	return 0;
}

static int gop_mpcs_mode(struct mvpp2_port *port)
{
	u32 val;

	/* configure PCS40G COMMON CONTROL */
	val = readl(port->priv->mpcs_base + PCS40G_COMMON_CONTROL);
	val &= ~FORWARD_ERROR_CORRECTION_MASK;
	writel(val, port->priv->mpcs_base + PCS40G_COMMON_CONTROL);

	/* configure PCS CLOCK RESET */
	val = readl(port->priv->mpcs_base + PCS_CLOCK_RESET);
	val &= ~CLK_DIVISION_RATIO_MASK;
	val |= 1 << CLK_DIVISION_RATIO_OFFS;
	writel(val, port->priv->mpcs_base + PCS_CLOCK_RESET);

	val &= ~CLK_DIV_PHASE_SET_MASK;
	val |= MAC_CLK_RESET_MASK;
	val |= RX_SD_CLK_RESET_MASK;
	val |= TX_SD_CLK_RESET_MASK;
	writel(val, port->priv->mpcs_base + PCS_CLOCK_RESET);

	return 0;
}

/* Set the internal mux's to the required MAC in the GOP */
static int gop_xlg_mac_mode_cfg(struct mvpp2_port *port, int num_of_act_lanes)
{
	u32 val;

	/* configure 10G MAC mode */
	val = readl(port->base + MVPP22_XLG_CTRL0_REG);
	val |= MVPP22_XLG_RX_FC_EN;
	writel(val, port->base + MVPP22_XLG_CTRL0_REG);

	val = readl(port->base + MVPP22_XLG_CTRL3_REG);
	val &= ~MVPP22_XLG_CTRL3_MACMODESELECT_MASK;
	val |= MVPP22_XLG_CTRL3_MACMODESELECT_10GMAC;
	writel(val, port->base + MVPP22_XLG_CTRL3_REG);

	/* read - modify - write */
	val = readl(port->base + MVPP22_XLG_CTRL4_REG);
	val &= ~MVPP22_XLG_MODE_DMA_1G;
	val |= MVPP22_XLG_FORWARD_PFC_EN;
	val |= MVPP22_XLG_FORWARD_802_3X_FC_EN;
	val &= ~MVPP22_XLG_EN_IDLE_CHECK_FOR_LINK;
	writel(val, port->base + MVPP22_XLG_CTRL4_REG);

	/* Jumbo frame support: 0x1400 * 2 = 0x2800 bytes */
	val = readl(port->base + MVPP22_XLG_CTRL1_REG);
	val &= ~MVPP22_XLG_MAX_RX_SIZE_MASK;
	val |= 0x1400 << MVPP22_XLG_MAX_RX_SIZE_OFFS;
	writel(val, port->base + MVPP22_XLG_CTRL1_REG);

	/* unmask link change interrupt */
	val = readl(port->base + MVPP22_XLG_INTERRUPT_MASK_REG);
	val |= MVPP22_XLG_INTERRUPT_LINK_CHANGE;
	val |= 1; /* unmask summary bit */
	writel(val, port->base + MVPP22_XLG_INTERRUPT_MASK_REG);

	return 0;
}

/* Set PCS to reset or exit from reset */
static int gop_xpcs_reset(struct mvpp2_port *port, int reset)
{
	u32 val;

	/* read - modify - write */
	val = readl(port->priv->xpcs_base + MVPP22_XPCS_GLOBAL_CFG_0_REG);
	if (reset)
		val &= ~MVPP22_XPCS_PCSRESET;
	else
		val |= MVPP22_XPCS_PCSRESET;
	writel(val, port->priv->xpcs_base + MVPP22_XPCS_GLOBAL_CFG_0_REG);

	return 0;
}

/* Set the MAC to reset or exit from reset */
static int gop_xlg_mac_reset(struct mvpp2_port *port, int reset)
{
	u32 val;

	/* read - modify - write */
	val = readl(port->base + MVPP22_XLG_CTRL0_REG);
	if (reset)
		val &= ~MVPP22_XLG_MAC_RESETN;
	else
		val |= MVPP22_XLG_MAC_RESETN;
	writel(val, port->base + MVPP22_XLG_CTRL0_REG);

	return 0;
}

/*
 * gop_port_init
 *
 * Init physical port. Configures the port mode and all it's elements
 * accordingly.
 * Does not verify that the selected mode/port number is valid at the
 * core level.
 */
static int gop_port_init(struct mvpp2_port *port)
{
	int mac_num = port->gop_id;
	int num_of_act_lanes;

	if (mac_num >= MVPP22_GOP_MAC_NUM) {
		netdev_err(NULL, "%s: illegal port number %d", __func__,
			   mac_num);
		return -1;
	}

	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		gop_gmac_reset(port, 1);

		/* configure PCS */
		gop_gpcs_mode_cfg(port, 0);
		gop_bypass_clk_cfg(port, 1);

		/* configure MAC */
		gop_gmac_mode_cfg(port);
		/* pcs unreset */
		gop_gpcs_reset(port, 0);

		/* mac unreset */
		gop_gmac_reset(port, 0);
		break;

	case PHY_INTERFACE_MODE_SGMII:
		/* configure PCS */
		gop_gpcs_mode_cfg(port, 1);

		/* configure MAC */
		gop_gmac_mode_cfg(port);
		/* select proper Mac mode */
		gop_xlg_2_gig_mac_cfg(port);

		/* pcs unreset */
		gop_gpcs_reset(port, 0);
		/* mac unreset */
		gop_gmac_reset(port, 0);
		break;

	case PHY_INTERFACE_MODE_SFI:
		num_of_act_lanes = 2;
		mac_num = 0;
		/* configure PCS */
		gop_xpcs_mode(port, num_of_act_lanes);
		gop_mpcs_mode(port);
		/* configure MAC */
		gop_xlg_mac_mode_cfg(port, num_of_act_lanes);

		/* pcs unreset */
		gop_xpcs_reset(port, 0);

		/* mac unreset */
		gop_xlg_mac_reset(port, 0);
		break;

	default:
		netdev_err(NULL, "%s: Requested port mode (%d) not supported\n",
			   __func__, port->phy_interface);
		return -1;
	}

	return 0;
}

static void gop_xlg_mac_port_enable(struct mvpp2_port *port, int enable)
{
	u32 val;

	val = readl(port->base + MVPP22_XLG_CTRL0_REG);
	if (enable) {
		/* Enable port and MIB counters update */
		val |= MVPP22_XLG_PORT_EN;
		val &= ~MVPP22_XLG_MIBCNT_DIS;
	} else {
		/* Disable port */
		val &= ~MVPP22_XLG_PORT_EN;
	}
	writel(val, port->base + MVPP22_XLG_CTRL0_REG);
}

static void gop_port_enable(struct mvpp2_port *port, int enable)
{
	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_SGMII:
		if (enable)
			mvpp2_port_enable(port);
		else
			mvpp2_port_disable(port);
		break;

	case PHY_INTERFACE_MODE_SFI:
		gop_xlg_mac_port_enable(port, enable);

		break;
	default:
		netdev_err(NULL, "%s: Wrong port mode (%d)\n", __func__,
			   port->phy_interface);
		return;
	}
}

/* RFU1 functions */
static inline u32 gop_rfu1_read(struct mvpp2 *priv, u32 offset)
{
	return readl(priv->rfu1_base + offset);
}

static inline void gop_rfu1_write(struct mvpp2 *priv, u32 offset, u32 data)
{
	writel(data, priv->rfu1_base + offset);
}

static u32 mvpp2_netc_cfg_create(int gop_id, phy_interface_t phy_type)
{
	u32 val = 0;

	if (gop_id == 2) {
		if (phy_type == PHY_INTERFACE_MODE_SGMII)
			val |= MV_NETC_GE_MAC2_SGMII;
	}

	if (gop_id == 3) {
		if (phy_type == PHY_INTERFACE_MODE_SGMII)
			val |= MV_NETC_GE_MAC3_SGMII;
		else if (phy_type == PHY_INTERFACE_MODE_RGMII ||
			 phy_type == PHY_INTERFACE_MODE_RGMII_ID)
			val |= MV_NETC_GE_MAC3_RGMII;
	}

	return val;
}

static void gop_netc_active_port(struct mvpp2 *priv, int gop_id, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, NETCOMP_PORTS_CONTROL_1_REG);
	reg &= ~(NETC_PORTS_ACTIVE_MASK(gop_id));

	val <<= NETC_PORTS_ACTIVE_OFFSET(gop_id);
	val &= NETC_PORTS_ACTIVE_MASK(gop_id);

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_PORTS_CONTROL_1_REG, reg);
}

static void gop_netc_mii_mode(struct mvpp2 *priv, int gop_id, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, NETCOMP_CONTROL_0_REG);
	reg &= ~NETC_GBE_PORT1_MII_MODE_MASK;

	val <<= NETC_GBE_PORT1_MII_MODE_OFFS;
	val &= NETC_GBE_PORT1_MII_MODE_MASK;

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_CONTROL_0_REG, reg);
}

static void gop_netc_gop_reset(struct mvpp2 *priv, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, GOP_SOFT_RESET_1_REG);
	reg &= ~NETC_GOP_SOFT_RESET_MASK;

	val <<= NETC_GOP_SOFT_RESET_OFFS;
	val &= NETC_GOP_SOFT_RESET_MASK;

	reg |= val;

	gop_rfu1_write(priv, GOP_SOFT_RESET_1_REG, reg);
}

static void gop_netc_gop_clock_logic_set(struct mvpp2 *priv, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, NETCOMP_PORTS_CONTROL_0_REG);
	reg &= ~NETC_CLK_DIV_PHASE_MASK;

	val <<= NETC_CLK_DIV_PHASE_OFFS;
	val &= NETC_CLK_DIV_PHASE_MASK;

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_PORTS_CONTROL_0_REG, reg);
}

static void gop_netc_port_rf_reset(struct mvpp2 *priv, int gop_id, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, NETCOMP_PORTS_CONTROL_1_REG);
	reg &= ~(NETC_PORT_GIG_RF_RESET_MASK(gop_id));

	val <<= NETC_PORT_GIG_RF_RESET_OFFS(gop_id);
	val &= NETC_PORT_GIG_RF_RESET_MASK(gop_id);

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_PORTS_CONTROL_1_REG, reg);
}

static void gop_netc_gbe_sgmii_mode_select(struct mvpp2 *priv, int gop_id,
					   u32 val)
{
	u32 reg, mask, offset;

	if (gop_id == 2) {
		mask = NETC_GBE_PORT0_SGMII_MODE_MASK;
		offset = NETC_GBE_PORT0_SGMII_MODE_OFFS;
	} else {
		mask = NETC_GBE_PORT1_SGMII_MODE_MASK;
		offset = NETC_GBE_PORT1_SGMII_MODE_OFFS;
	}
	reg = gop_rfu1_read(priv, NETCOMP_CONTROL_0_REG);
	reg &= ~mask;

	val <<= offset;
	val &= mask;

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_CONTROL_0_REG, reg);
}

static void gop_netc_bus_width_select(struct mvpp2 *priv, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, NETCOMP_PORTS_CONTROL_0_REG);
	reg &= ~NETC_BUS_WIDTH_SELECT_MASK;

	val <<= NETC_BUS_WIDTH_SELECT_OFFS;
	val &= NETC_BUS_WIDTH_SELECT_MASK;

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_PORTS_CONTROL_0_REG, reg);
}

static void gop_netc_sample_stages_timing(struct mvpp2 *priv, u32 val)
{
	u32 reg;

	reg = gop_rfu1_read(priv, NETCOMP_PORTS_CONTROL_0_REG);
	reg &= ~NETC_GIG_RX_DATA_SAMPLE_MASK;

	val <<= NETC_GIG_RX_DATA_SAMPLE_OFFS;
	val &= NETC_GIG_RX_DATA_SAMPLE_MASK;

	reg |= val;

	gop_rfu1_write(priv, NETCOMP_PORTS_CONTROL_0_REG, reg);
}

static void gop_netc_mac_to_xgmii(struct mvpp2 *priv, int gop_id,
				  enum mv_netc_phase phase)
{
	switch (phase) {
	case MV_NETC_FIRST_PHASE:
		/* Set Bus Width to HB mode = 1 */
		gop_netc_bus_width_select(priv, 1);
		/* Select RGMII mode */
		gop_netc_gbe_sgmii_mode_select(priv, gop_id, MV_NETC_GBE_XMII);
		break;

	case MV_NETC_SECOND_PHASE:
		/* De-assert the relevant port HB reset */
		gop_netc_port_rf_reset(priv, gop_id, 1);
		break;
	}
}

static void gop_netc_mac_to_sgmii(struct mvpp2 *priv, int gop_id,
				  enum mv_netc_phase phase)
{
	switch (phase) {
	case MV_NETC_FIRST_PHASE:
		/* Set Bus Width to HB mode = 1 */
		gop_netc_bus_width_select(priv, 1);
		/* Select SGMII mode */
		if (gop_id >= 1) {
			gop_netc_gbe_sgmii_mode_select(priv, gop_id,
						       MV_NETC_GBE_SGMII);
		}

		/* Configure the sample stages */
		gop_netc_sample_stages_timing(priv, 0);
		/* Configure the ComPhy Selector */
		/* gop_netc_com_phy_selector_config(netComplex); */
		break;

	case MV_NETC_SECOND_PHASE:
		/* De-assert the relevant port HB reset */
		gop_netc_port_rf_reset(priv, gop_id, 1);
		break;
	}
}

static int gop_netc_init(struct mvpp2 *priv, enum mv_netc_phase phase)
{
	u32 c = priv->netc_config;

	if (c & MV_NETC_GE_MAC2_SGMII)
		gop_netc_mac_to_sgmii(priv, 2, phase);
	else
		gop_netc_mac_to_xgmii(priv, 2, phase);

	if (c & MV_NETC_GE_MAC3_SGMII) {
		gop_netc_mac_to_sgmii(priv, 3, phase);
	} else {
		gop_netc_mac_to_xgmii(priv, 3, phase);
		if (c & MV_NETC_GE_MAC3_RGMII)
			gop_netc_mii_mode(priv, 3, MV_NETC_GBE_RGMII);
		else
			gop_netc_mii_mode(priv, 3, MV_NETC_GBE_MII);
	}

	/* Activate gop ports 0, 2, 3 */
	gop_netc_active_port(priv, 0, 1);
	gop_netc_active_port(priv, 2, 1);
	gop_netc_active_port(priv, 3, 1);

	if (phase == MV_NETC_SECOND_PHASE) {
		/* Enable the GOP internal clock logic */
		gop_netc_gop_clock_logic_set(priv, 1);
		/* De-assert GOP unit reset */
		gop_netc_gop_reset(priv, 1);
	}

	return 0;
}

/* Set defaults to the MVPP2 port */
static void mvpp2_defaults_set(struct mvpp2_port *port)
{
	int tx_port_num, val, queue, ptxq, lrxq;

	if (port->priv->hw_version == MVPP21) {
		/* Configure port to loopback if needed */
		if (port->flags & MVPP2_F_LOOPBACK)
			mvpp2_port_loopback_set(port);

		/* Update TX FIFO MIN Threshold */
		val = readl(port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);
		val &= ~MVPP2_GMAC_TX_FIFO_MIN_TH_ALL_MASK;
		/* Min. TX threshold must be less than minimal packet length */
		val |= MVPP2_GMAC_TX_FIFO_MIN_TH_MASK(64 - 4 - 2);
		writel(val, port->base + MVPP2_GMAC_PORT_FIFO_CFG_1_REG);
	}

	/* Disable Legacy WRR, Disable EJP, Release from reset */
	tx_port_num = mvpp2_egress_port(port);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG,
		    tx_port_num);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_CMD_1_REG, 0);

	/* Close bandwidth for all queues */
	for (queue = 0; queue < MVPP2_MAX_TXQ; queue++) {
		ptxq = mvpp2_txq_phys(port->id, queue);
		mvpp2_write(port->priv,
			    MVPP2_TXQ_SCHED_TOKEN_CNTR_REG(ptxq), 0);
	}

	/* Set refill period to 1 usec, refill tokens
	 * and bucket size to maximum
	 */
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PERIOD_REG, 0xc8);
	val = mvpp2_read(port->priv, MVPP2_TXP_SCHED_REFILL_REG);
	val &= ~MVPP2_TXP_REFILL_PERIOD_ALL_MASK;
	val |= MVPP2_TXP_REFILL_PERIOD_MASK(1);
	val |= MVPP2_TXP_REFILL_TOKENS_ALL_MASK;
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_REFILL_REG, val);
	val = MVPP2_TXP_TOKEN_SIZE_MAX;
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_TOKEN_SIZE_REG, val);

	/* Set MaximumLowLatencyPacketSize value to 256 */
	mvpp2_write(port->priv, MVPP2_RX_CTRL_REG(port->id),
		    MVPP2_RX_USE_PSEUDO_FOR_CSUM_MASK |
		    MVPP2_RX_LOW_LATENCY_PKT_SIZE(256));

	/* Enable Rx cache snoop */
	for (lrxq = 0; lrxq < rxq_number; lrxq++) {
		queue = port->rxqs[lrxq]->id;
		val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(queue));
		val |= MVPP2_SNOOP_PKT_SIZE_MASK |
			   MVPP2_SNOOP_BUF_HDR_MASK;
		mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(queue), val);
	}
}

/* Enable/disable receiving packets */
static void mvpp2_ingress_enable(struct mvpp2_port *port)
{
	u32 val;
	int lrxq, queue;

	for (lrxq = 0; lrxq < rxq_number; lrxq++) {
		queue = port->rxqs[lrxq]->id;
		val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(queue));
		val &= ~MVPP2_RXQ_DISABLE_MASK;
		mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(queue), val);
	}
}

static void mvpp2_ingress_disable(struct mvpp2_port *port)
{
	u32 val;
	int lrxq, queue;

	for (lrxq = 0; lrxq < rxq_number; lrxq++) {
		queue = port->rxqs[lrxq]->id;
		val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(queue));
		val |= MVPP2_RXQ_DISABLE_MASK;
		mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(queue), val);
	}
}

/* Enable transmit via physical egress queue
 * - HW starts take descriptors from DRAM
 */
static void mvpp2_egress_enable(struct mvpp2_port *port)
{
	u32 qmap;
	int queue;
	int tx_port_num = mvpp2_egress_port(port);

	/* Enable all initialized TXs. */
	qmap = 0;
	for (queue = 0; queue < txq_number; queue++) {
		struct mvpp2_tx_queue *txq = port->txqs[queue];

		if (txq->descs != NULL)
			qmap |= (1 << queue);
	}

	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG, qmap);
}

/* Disable transmit via physical egress queue
 * - HW doesn't take descriptors from DRAM
 */
static void mvpp2_egress_disable(struct mvpp2_port *port)
{
	u32 reg_data;
	int delay;
	int tx_port_num = mvpp2_egress_port(port);

	/* Issue stop command for active channels only */
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);
	reg_data = (mvpp2_read(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG)) &
		    MVPP2_TXP_SCHED_ENQ_MASK;
	if (reg_data != 0)
		mvpp2_write(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG,
			    (reg_data << MVPP2_TXP_SCHED_DISQ_OFFSET));

	/* Wait for all Tx activity to terminate. */
	delay = 0;
	do {
		if (delay >= MVPP2_TX_DISABLE_TIMEOUT_MSEC) {
			netdev_warn(port->dev,
				    "Tx stop timed out, status=0x%08x\n",
				    reg_data);
			break;
		}
		mdelay(1);
		delay++;

		/* Check port TX Command register that all
		 * Tx queues are stopped
		 */
		reg_data = mvpp2_read(port->priv, MVPP2_TXP_SCHED_Q_CMD_REG);
	} while (reg_data & MVPP2_TXP_SCHED_ENQ_MASK);
}

/* Rx descriptors helper methods */

/* Get number of Rx descriptors occupied by received packets */
static inline int
mvpp2_rxq_received(struct mvpp2_port *port, int rxq_id)
{
	u32 val = mvpp2_read(port->priv, MVPP2_RXQ_STATUS_REG(rxq_id));

	return val & MVPP2_RXQ_OCCUPIED_MASK;
}

/* Update Rx queue status with the number of occupied and available
 * Rx descriptor slots.
 */
static inline void
mvpp2_rxq_status_update(struct mvpp2_port *port, int rxq_id,
			int used_count, int free_count)
{
	/* Decrement the number of used descriptors and increment count
	 * increment the number of free descriptors.
	 */
	u32 val = used_count | (free_count << MVPP2_RXQ_NUM_NEW_OFFSET);

	mvpp2_write(port->priv, MVPP2_RXQ_STATUS_UPDATE_REG(rxq_id), val);
}

/* Get pointer to next RX descriptor to be processed by SW */
static inline struct mvpp2_rx_desc *
mvpp2_rxq_next_desc_get(struct mvpp2_rx_queue *rxq)
{
	int rx_desc = rxq->next_desc_to_proc;

	rxq->next_desc_to_proc = MVPP2_QUEUE_NEXT_DESC(rxq, rx_desc);
	prefetch(rxq->descs + rxq->next_desc_to_proc);
	return rxq->descs + rx_desc;
}

/* Set rx queue offset */
static void mvpp2_rxq_offset_set(struct mvpp2_port *port,
				 int prxq, int offset)
{
	u32 val;

	/* Convert offset from bytes to units of 32 bytes */
	offset = offset >> 5;

	val = mvpp2_read(port->priv, MVPP2_RXQ_CONFIG_REG(prxq));
	val &= ~MVPP2_RXQ_PACKET_OFFSET_MASK;

	/* Offset is in */
	val |= ((offset << MVPP2_RXQ_PACKET_OFFSET_OFFS) &
		    MVPP2_RXQ_PACKET_OFFSET_MASK);

	mvpp2_write(port->priv, MVPP2_RXQ_CONFIG_REG(prxq), val);
}

/* Obtain BM cookie information from descriptor */
static u32 mvpp2_bm_cookie_build(struct mvpp2_port *port,
				 struct mvpp2_rx_desc *rx_desc)
{
	int cpu = smp_processor_id();
	int pool;

	pool = (mvpp2_rxdesc_status_get(port, rx_desc) &
		MVPP2_RXD_BM_POOL_ID_MASK) >>
		MVPP2_RXD_BM_POOL_ID_OFFS;

	return ((pool & 0xFF) << MVPP2_BM_COOKIE_POOL_OFFS) |
	       ((cpu & 0xFF) << MVPP2_BM_COOKIE_CPU_OFFS);
}

/* Tx descriptors helper methods */

/* Get number of Tx descriptors waiting to be transmitted by HW */
static int mvpp2_txq_pend_desc_num_get(struct mvpp2_port *port,
				       struct mvpp2_tx_queue *txq)
{
	u32 val;

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PENDING_REG);

	return val & MVPP2_TXQ_PENDING_MASK;
}

/* Get pointer to next Tx descriptor to be processed (send) by HW */
static struct mvpp2_tx_desc *
mvpp2_txq_next_desc_get(struct mvpp2_tx_queue *txq)
{
	int tx_desc = txq->next_desc_to_proc;

	txq->next_desc_to_proc = MVPP2_QUEUE_NEXT_DESC(txq, tx_desc);
	return txq->descs + tx_desc;
}

/* Update HW with number of aggregated Tx descriptors to be sent */
static void mvpp2_aggr_txq_pend_desc_add(struct mvpp2_port *port, int pending)
{
	/* aggregated access - relevant TXQ number is written in TX desc */
	mvpp2_write(port->priv, MVPP2_AGGR_TXQ_UPDATE_REG, pending);
}

/* Get number of sent descriptors and decrement counter.
 * The number of sent descriptors is returned.
 * Per-CPU access
 */
static inline int mvpp2_txq_sent_desc_proc(struct mvpp2_port *port,
					   struct mvpp2_tx_queue *txq)
{
	u32 val;

	/* Reading status reg resets transmitted descriptor counter */
	val = mvpp2_read(port->priv, MVPP2_TXQ_SENT_REG(txq->id));

	return (val & MVPP2_TRANSMITTED_COUNT_MASK) >>
		MVPP2_TRANSMITTED_COUNT_OFFSET;
}

static void mvpp2_txq_sent_counter_clear(void *arg)
{
	struct mvpp2_port *port = arg;
	int queue;

	for (queue = 0; queue < txq_number; queue++) {
		int id = port->txqs[queue]->id;

		mvpp2_read(port->priv, MVPP2_TXQ_SENT_REG(id));
	}
}

/* Set max sizes for Tx queues */
static void mvpp2_txp_max_tx_size_set(struct mvpp2_port *port)
{
	u32	val, size, mtu;
	int	txq, tx_port_num;

	mtu = port->pkt_size * 8;
	if (mtu > MVPP2_TXP_MTU_MAX)
		mtu = MVPP2_TXP_MTU_MAX;

	/* WA for wrong Token bucket update: Set MTU value = 3*real MTU value */
	mtu = 3 * mtu;

	/* Indirect access to registers */
	tx_port_num = mvpp2_egress_port(port);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);

	/* Set MTU */
	val = mvpp2_read(port->priv, MVPP2_TXP_SCHED_MTU_REG);
	val &= ~MVPP2_TXP_MTU_MAX;
	val |= mtu;
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_MTU_REG, val);

	/* TXP token size and all TXQs token size must be larger that MTU */
	val = mvpp2_read(port->priv, MVPP2_TXP_SCHED_TOKEN_SIZE_REG);
	size = val & MVPP2_TXP_TOKEN_SIZE_MAX;
	if (size < mtu) {
		size = mtu;
		val &= ~MVPP2_TXP_TOKEN_SIZE_MAX;
		val |= size;
		mvpp2_write(port->priv, MVPP2_TXP_SCHED_TOKEN_SIZE_REG, val);
	}

	for (txq = 0; txq < txq_number; txq++) {
		val = mvpp2_read(port->priv,
				 MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq));
		size = val & MVPP2_TXQ_TOKEN_SIZE_MAX;

		if (size < mtu) {
			size = mtu;
			val &= ~MVPP2_TXQ_TOKEN_SIZE_MAX;
			val |= size;
			mvpp2_write(port->priv,
				    MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq),
				    val);
		}
	}
}

/* Free Tx queue skbuffs */
static void mvpp2_txq_bufs_free(struct mvpp2_port *port,
				struct mvpp2_tx_queue *txq,
				struct mvpp2_txq_pcpu *txq_pcpu, int num)
{
	int i;

	for (i = 0; i < num; i++)
		mvpp2_txq_inc_get(txq_pcpu);
}

static inline struct mvpp2_rx_queue *mvpp2_get_rx_queue(struct mvpp2_port *port,
							u32 cause)
{
	int queue = fls(cause) - 1;

	return port->rxqs[queue];
}

static inline struct mvpp2_tx_queue *mvpp2_get_tx_queue(struct mvpp2_port *port,
							u32 cause)
{
	int queue = fls(cause) - 1;

	return port->txqs[queue];
}

/* Rx/Tx queue initialization/cleanup methods */

/* Allocate and initialize descriptors for aggr TXQ */
static int mvpp2_aggr_txq_init(struct udevice *dev,
			       struct mvpp2_tx_queue *aggr_txq,
			       int desc_num, int cpu,
			       struct mvpp2 *priv)
{
	u32 txq_dma;

	/* Allocate memory for TX descriptors */
	aggr_txq->descs = buffer_loc.aggr_tx_descs;
	aggr_txq->descs_dma = (dma_addr_t)buffer_loc.aggr_tx_descs;
	if (!aggr_txq->descs)
		return -ENOMEM;

	/* Make sure descriptor address is cache line size aligned  */
	BUG_ON(aggr_txq->descs !=
	       PTR_ALIGN(aggr_txq->descs, MVPP2_CPU_D_CACHE_LINE_SIZE));

	aggr_txq->last_desc = aggr_txq->size - 1;

	/* Aggr TXQ no reset WA */
	aggr_txq->next_desc_to_proc = mvpp2_read(priv,
						 MVPP2_AGGR_TXQ_INDEX_REG(cpu));

	/* Set Tx descriptors queue starting address indirect
	 * access
	 */
	if (priv->hw_version == MVPP21)
		txq_dma = aggr_txq->descs_dma;
	else
		txq_dma = aggr_txq->descs_dma >>
			MVPP22_AGGR_TXQ_DESC_ADDR_OFFS;

	mvpp2_write(priv, MVPP2_AGGR_TXQ_DESC_ADDR_REG(cpu), txq_dma);
	mvpp2_write(priv, MVPP2_AGGR_TXQ_DESC_SIZE_REG(cpu), desc_num);

	return 0;
}

/* Create a specified Rx queue */
static int mvpp2_rxq_init(struct mvpp2_port *port,
			  struct mvpp2_rx_queue *rxq)

{
	u32 rxq_dma;

	rxq->size = port->rx_ring_size;

	/* Allocate memory for RX descriptors */
	rxq->descs = buffer_loc.rx_descs;
	rxq->descs_dma = (dma_addr_t)buffer_loc.rx_descs;
	if (!rxq->descs)
		return -ENOMEM;

	BUG_ON(rxq->descs !=
	       PTR_ALIGN(rxq->descs, MVPP2_CPU_D_CACHE_LINE_SIZE));

	rxq->last_desc = rxq->size - 1;

	/* Zero occupied and non-occupied counters - direct access */
	mvpp2_write(port->priv, MVPP2_RXQ_STATUS_REG(rxq->id), 0);

	/* Set Rx descriptors queue starting address - indirect access */
	mvpp2_write(port->priv, MVPP2_RXQ_NUM_REG, rxq->id);
	if (port->priv->hw_version == MVPP21)
		rxq_dma = rxq->descs_dma;
	else
		rxq_dma = rxq->descs_dma >> MVPP22_DESC_ADDR_OFFS;
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_ADDR_REG, rxq_dma);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_SIZE_REG, rxq->size);
	mvpp2_write(port->priv, MVPP2_RXQ_INDEX_REG, 0);

	/* Set Offset */
	mvpp2_rxq_offset_set(port, rxq->id, NET_SKB_PAD);

	/* Add number of descriptors ready for receiving packets */
	mvpp2_rxq_status_update(port, rxq->id, 0, rxq->size);

	return 0;
}

/* Push packets received by the RXQ to BM pool */
static void mvpp2_rxq_drop_pkts(struct mvpp2_port *port,
				struct mvpp2_rx_queue *rxq)
{
	int rx_received, i;

	rx_received = mvpp2_rxq_received(port, rxq->id);
	if (!rx_received)
		return;

	for (i = 0; i < rx_received; i++) {
		struct mvpp2_rx_desc *rx_desc = mvpp2_rxq_next_desc_get(rxq);
		u32 bm = mvpp2_bm_cookie_build(port, rx_desc);

		mvpp2_pool_refill(port, bm,
				  mvpp2_rxdesc_dma_addr_get(port, rx_desc),
				  mvpp2_rxdesc_cookie_get(port, rx_desc));
	}
	mvpp2_rxq_status_update(port, rxq->id, rx_received, rx_received);
}

/* Cleanup Rx queue */
static void mvpp2_rxq_deinit(struct mvpp2_port *port,
			     struct mvpp2_rx_queue *rxq)
{
	mvpp2_rxq_drop_pkts(port, rxq);

	rxq->descs             = NULL;
	rxq->last_desc         = 0;
	rxq->next_desc_to_proc = 0;
	rxq->descs_dma         = 0;

	/* Clear Rx descriptors queue starting address and size;
	 * free descriptor number
	 */
	mvpp2_write(port->priv, MVPP2_RXQ_STATUS_REG(rxq->id), 0);
	mvpp2_write(port->priv, MVPP2_RXQ_NUM_REG, rxq->id);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_ADDR_REG, 0);
	mvpp2_write(port->priv, MVPP2_RXQ_DESC_SIZE_REG, 0);
}

/* Create and initialize a Tx queue */
static int mvpp2_txq_init(struct mvpp2_port *port,
			  struct mvpp2_tx_queue *txq)
{
	u32 val;
	int cpu, desc, desc_per_txq, tx_port_num;
	struct mvpp2_txq_pcpu *txq_pcpu;

	txq->size = port->tx_ring_size;

	/* Allocate memory for Tx descriptors */
	txq->descs = buffer_loc.tx_descs;
	txq->descs_dma = (dma_addr_t)buffer_loc.tx_descs;
	if (!txq->descs)
		return -ENOMEM;

	/* Make sure descriptor address is cache line size aligned  */
	BUG_ON(txq->descs !=
	       PTR_ALIGN(txq->descs, MVPP2_CPU_D_CACHE_LINE_SIZE));

	txq->last_desc = txq->size - 1;

	/* Set Tx descriptors queue starting address - indirect access */
	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_ADDR_REG, txq->descs_dma);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_SIZE_REG, txq->size &
					     MVPP2_TXQ_DESC_SIZE_MASK);
	mvpp2_write(port->priv, MVPP2_TXQ_INDEX_REG, 0);
	mvpp2_write(port->priv, MVPP2_TXQ_RSVD_CLR_REG,
		    txq->id << MVPP2_TXQ_RSVD_CLR_OFFSET);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PENDING_REG);
	val &= ~MVPP2_TXQ_PENDING_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PENDING_REG, val);

	/* Calculate base address in prefetch buffer. We reserve 16 descriptors
	 * for each existing TXQ.
	 * TCONTS for PON port must be continuous from 0 to MVPP2_MAX_TCONT
	 * GBE ports assumed to be continious from 0 to MVPP2_MAX_PORTS
	 */
	desc_per_txq = 16;
	desc = (port->id * MVPP2_MAX_TXQ * desc_per_txq) +
	       (txq->log_id * desc_per_txq);

	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG,
		    MVPP2_PREF_BUF_PTR(desc) | MVPP2_PREF_BUF_SIZE_16 |
		    MVPP2_PREF_BUF_THRESH(desc_per_txq / 2));

	/* WRR / EJP configuration - indirect access */
	tx_port_num = mvpp2_egress_port(port);
	mvpp2_write(port->priv, MVPP2_TXP_SCHED_PORT_INDEX_REG, tx_port_num);

	val = mvpp2_read(port->priv, MVPP2_TXQ_SCHED_REFILL_REG(txq->log_id));
	val &= ~MVPP2_TXQ_REFILL_PERIOD_ALL_MASK;
	val |= MVPP2_TXQ_REFILL_PERIOD_MASK(1);
	val |= MVPP2_TXQ_REFILL_TOKENS_ALL_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_SCHED_REFILL_REG(txq->log_id), val);

	val = MVPP2_TXQ_TOKEN_SIZE_MAX;
	mvpp2_write(port->priv, MVPP2_TXQ_SCHED_TOKEN_SIZE_REG(txq->log_id),
		    val);

	for_each_present_cpu(cpu) {
		txq_pcpu = per_cpu_ptr(txq->pcpu, cpu);
		txq_pcpu->size = txq->size;
	}

	return 0;
}

/* Free allocated TXQ resources */
static void mvpp2_txq_deinit(struct mvpp2_port *port,
			     struct mvpp2_tx_queue *txq)
{
	txq->descs             = NULL;
	txq->last_desc         = 0;
	txq->next_desc_to_proc = 0;
	txq->descs_dma         = 0;

	/* Set minimum bandwidth for disabled TXQs */
	mvpp2_write(port->priv, MVPP2_TXQ_SCHED_TOKEN_CNTR_REG(txq->id), 0);

	/* Set Tx descriptors queue starting address and size */
	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_ADDR_REG, 0);
	mvpp2_write(port->priv, MVPP2_TXQ_DESC_SIZE_REG, 0);
}

/* Cleanup Tx ports */
static void mvpp2_txq_clean(struct mvpp2_port *port, struct mvpp2_tx_queue *txq)
{
	struct mvpp2_txq_pcpu *txq_pcpu;
	int delay, pending, cpu;
	u32 val;

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);
	val = mvpp2_read(port->priv, MVPP2_TXQ_PREF_BUF_REG);
	val |= MVPP2_TXQ_DRAIN_EN_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG, val);

	/* The napi queue has been stopped so wait for all packets
	 * to be transmitted.
	 */
	delay = 0;
	do {
		if (delay >= MVPP2_TX_PENDING_TIMEOUT_MSEC) {
			netdev_warn(port->dev,
				    "port %d: cleaning queue %d timed out\n",
				    port->id, txq->log_id);
			break;
		}
		mdelay(1);
		delay++;

		pending = mvpp2_txq_pend_desc_num_get(port, txq);
	} while (pending);

	val &= ~MVPP2_TXQ_DRAIN_EN_MASK;
	mvpp2_write(port->priv, MVPP2_TXQ_PREF_BUF_REG, val);

	for_each_present_cpu(cpu) {
		txq_pcpu = per_cpu_ptr(txq->pcpu, cpu);

		/* Release all packets */
		mvpp2_txq_bufs_free(port, txq, txq_pcpu, txq_pcpu->count);

		/* Reset queue */
		txq_pcpu->count = 0;
		txq_pcpu->txq_put_index = 0;
		txq_pcpu->txq_get_index = 0;
	}
}

/* Cleanup all Tx queues */
static void mvpp2_cleanup_txqs(struct mvpp2_port *port)
{
	struct mvpp2_tx_queue *txq;
	int queue;
	u32 val;

	val = mvpp2_read(port->priv, MVPP2_TX_PORT_FLUSH_REG);

	/* Reset Tx ports and delete Tx queues */
	val |= MVPP2_TX_PORT_FLUSH_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_TX_PORT_FLUSH_REG, val);

	for (queue = 0; queue < txq_number; queue++) {
		txq = port->txqs[queue];
		mvpp2_txq_clean(port, txq);
		mvpp2_txq_deinit(port, txq);
	}

	mvpp2_txq_sent_counter_clear(port);

	val &= ~MVPP2_TX_PORT_FLUSH_MASK(port->id);
	mvpp2_write(port->priv, MVPP2_TX_PORT_FLUSH_REG, val);
}

/* Cleanup all Rx queues */
static void mvpp2_cleanup_rxqs(struct mvpp2_port *port)
{
	int queue;

	for (queue = 0; queue < rxq_number; queue++)
		mvpp2_rxq_deinit(port, port->rxqs[queue]);
}

/* Init all Rx queues for port */
static int mvpp2_setup_rxqs(struct mvpp2_port *port)
{
	int queue, err;

	for (queue = 0; queue < rxq_number; queue++) {
		err = mvpp2_rxq_init(port, port->rxqs[queue]);
		if (err)
			goto err_cleanup;
	}
	return 0;

err_cleanup:
	mvpp2_cleanup_rxqs(port);
	return err;
}

/* Init all tx queues for port */
static int mvpp2_setup_txqs(struct mvpp2_port *port)
{
	struct mvpp2_tx_queue *txq;
	int queue, err;

	for (queue = 0; queue < txq_number; queue++) {
		txq = port->txqs[queue];
		err = mvpp2_txq_init(port, txq);
		if (err)
			goto err_cleanup;
	}

	mvpp2_txq_sent_counter_clear(port);
	return 0;

err_cleanup:
	mvpp2_cleanup_txqs(port);
	return err;
}

/* Adjust link */
static void mvpp2_link_event(struct mvpp2_port *port)
{
	struct phy_device *phydev = port->phy_dev;
	int status_change = 0;
	u32 val;

	if (phydev->link) {
		if ((port->speed != phydev->speed) ||
		    (port->duplex != phydev->duplex)) {
			u32 val;

			val = readl(port->base + MVPP2_GMAC_AUTONEG_CONFIG);
			val &= ~(MVPP2_GMAC_CONFIG_MII_SPEED |
				 MVPP2_GMAC_CONFIG_GMII_SPEED |
				 MVPP2_GMAC_CONFIG_FULL_DUPLEX |
				 MVPP2_GMAC_AN_SPEED_EN |
				 MVPP2_GMAC_AN_DUPLEX_EN);

			if (phydev->duplex)
				val |= MVPP2_GMAC_CONFIG_FULL_DUPLEX;

			if (phydev->speed == SPEED_1000)
				val |= MVPP2_GMAC_CONFIG_GMII_SPEED;
			else if (phydev->speed == SPEED_100)
				val |= MVPP2_GMAC_CONFIG_MII_SPEED;

			writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);

			port->duplex = phydev->duplex;
			port->speed  = phydev->speed;
		}
	}

	if (phydev->link != port->link) {
		if (!phydev->link) {
			port->duplex = -1;
			port->speed = 0;
		}

		port->link = phydev->link;
		status_change = 1;
	}

	if (status_change) {
		if (phydev->link) {
			val = readl(port->base + MVPP2_GMAC_AUTONEG_CONFIG);
			val |= (MVPP2_GMAC_FORCE_LINK_PASS |
				MVPP2_GMAC_FORCE_LINK_DOWN);
			writel(val, port->base + MVPP2_GMAC_AUTONEG_CONFIG);
			mvpp2_egress_enable(port);
			mvpp2_ingress_enable(port);
		} else {
			mvpp2_ingress_disable(port);
			mvpp2_egress_disable(port);
		}
	}
}

/* Main RX/TX processing routines */

/* Display more error info */
static void mvpp2_rx_error(struct mvpp2_port *port,
			   struct mvpp2_rx_desc *rx_desc)
{
	u32 status = mvpp2_rxdesc_status_get(port, rx_desc);
	size_t sz = mvpp2_rxdesc_size_get(port, rx_desc);

	switch (status & MVPP2_RXD_ERR_CODE_MASK) {
	case MVPP2_RXD_ERR_CRC:
		netdev_err(port->dev, "bad rx status %08x (crc error), size=%zu\n",
			   status, sz);
		break;
	case MVPP2_RXD_ERR_OVERRUN:
		netdev_err(port->dev, "bad rx status %08x (overrun error), size=%zu\n",
			   status, sz);
		break;
	case MVPP2_RXD_ERR_RESOURCE:
		netdev_err(port->dev, "bad rx status %08x (resource error), size=%zu\n",
			   status, sz);
		break;
	}
}

/* Reuse skb if possible, or allocate a new skb and add it to BM pool */
static int mvpp2_rx_refill(struct mvpp2_port *port,
			   struct mvpp2_bm_pool *bm_pool,
			   u32 bm, dma_addr_t dma_addr)
{
	mvpp2_pool_refill(port, bm, dma_addr, (unsigned long)dma_addr);
	return 0;
}

/* Set hw internals when starting port */
static void mvpp2_start_dev(struct mvpp2_port *port)
{
	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_SGMII:
		mvpp2_gmac_max_rx_size_set(port);
	default:
		break;
	}

	mvpp2_txp_max_tx_size_set(port);

	if (port->priv->hw_version == MVPP21)
		mvpp2_port_enable(port);
	else
		gop_port_enable(port, 1);
}

/* Set hw internals when stopping port */
static void mvpp2_stop_dev(struct mvpp2_port *port)
{
	/* Stop new packets from arriving to RXQs */
	mvpp2_ingress_disable(port);

	mvpp2_egress_disable(port);

	if (port->priv->hw_version == MVPP21)
		mvpp2_port_disable(port);
	else
		gop_port_enable(port, 0);
}

static int mvpp2_phy_connect(struct udevice *dev, struct mvpp2_port *port)
{
	struct phy_device *phy_dev;

	if (!port->init || port->link == 0) {
		phy_dev = phy_connect(port->bus, port->phyaddr, dev,
				      port->phy_interface);
		port->phy_dev = phy_dev;
		if (!phy_dev) {
			netdev_err(port->dev, "cannot connect to phy\n");
			return -ENODEV;
		}
		phy_dev->supported &= PHY_GBIT_FEATURES;
		phy_dev->advertising = phy_dev->supported;

		port->phy_dev = phy_dev;
		port->link    = 0;
		port->duplex  = 0;
		port->speed   = 0;

		phy_config(phy_dev);
		phy_startup(phy_dev);
		if (!phy_dev->link) {
			printf("%s: No link\n", phy_dev->dev->name);
			return -1;
		}

		port->init = 1;
	} else {
		mvpp2_egress_enable(port);
		mvpp2_ingress_enable(port);
	}

	return 0;
}

static int mvpp2_open(struct udevice *dev, struct mvpp2_port *port)
{
	unsigned char mac_bcast[ETH_ALEN] = {
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int err;

	err = mvpp2_prs_mac_da_accept(port->priv, port->id, mac_bcast, true);
	if (err) {
		netdev_err(dev, "mvpp2_prs_mac_da_accept BC failed\n");
		return err;
	}
	err = mvpp2_prs_mac_da_accept(port->priv, port->id,
				      port->dev_addr, true);
	if (err) {
		netdev_err(dev, "mvpp2_prs_mac_da_accept MC failed\n");
		return err;
	}
	err = mvpp2_prs_def_flow(port);
	if (err) {
		netdev_err(dev, "mvpp2_prs_def_flow failed\n");
		return err;
	}

	/* Allocate the Rx/Tx queues */
	err = mvpp2_setup_rxqs(port);
	if (err) {
		netdev_err(port->dev, "cannot allocate Rx queues\n");
		return err;
	}

	err = mvpp2_setup_txqs(port);
	if (err) {
		netdev_err(port->dev, "cannot allocate Tx queues\n");
		return err;
	}

	if (port->phy_node) {
		err = mvpp2_phy_connect(dev, port);
		if (err < 0)
			return err;

		mvpp2_link_event(port);
	} else {
		mvpp2_egress_enable(port);
		mvpp2_ingress_enable(port);
	}

	mvpp2_start_dev(port);

	return 0;
}

/* No Device ops here in U-Boot */

/* Driver initialization */

static void mvpp2_port_power_up(struct mvpp2_port *port)
{
	struct mvpp2 *priv = port->priv;

	/* On PPv2.2 the GoP / interface configuration has already been done */
	if (priv->hw_version == MVPP21)
		mvpp2_port_mii_set(port);
	mvpp2_port_periodic_xon_disable(port);
	if (priv->hw_version == MVPP21)
		mvpp2_port_fc_adv_enable(port);
	mvpp2_port_reset(port);
}

/* Initialize port HW */
static int mvpp2_port_init(struct udevice *dev, struct mvpp2_port *port)
{
	struct mvpp2 *priv = port->priv;
	struct mvpp2_txq_pcpu *txq_pcpu;
	int queue, cpu, err;

	if (port->first_rxq + rxq_number >
	    MVPP2_MAX_PORTS * priv->max_port_rxqs)
		return -EINVAL;

	/* Disable port */
	mvpp2_egress_disable(port);
	if (priv->hw_version == MVPP21)
		mvpp2_port_disable(port);
	else
		gop_port_enable(port, 0);

	port->txqs = devm_kcalloc(dev, txq_number, sizeof(*port->txqs),
				  GFP_KERNEL);
	if (!port->txqs)
		return -ENOMEM;

	/* Associate physical Tx queues to this port and initialize.
	 * The mapping is predefined.
	 */
	for (queue = 0; queue < txq_number; queue++) {
		int queue_phy_id = mvpp2_txq_phys(port->id, queue);
		struct mvpp2_tx_queue *txq;

		txq = devm_kzalloc(dev, sizeof(*txq), GFP_KERNEL);
		if (!txq)
			return -ENOMEM;

		txq->pcpu = devm_kzalloc(dev, sizeof(struct mvpp2_txq_pcpu),
					 GFP_KERNEL);
		if (!txq->pcpu)
			return -ENOMEM;

		txq->id = queue_phy_id;
		txq->log_id = queue;
		txq->done_pkts_coal = MVPP2_TXDONE_COAL_PKTS_THRESH;
		for_each_present_cpu(cpu) {
			txq_pcpu = per_cpu_ptr(txq->pcpu, cpu);
			txq_pcpu->cpu = cpu;
		}

		port->txqs[queue] = txq;
	}

	port->rxqs = devm_kcalloc(dev, rxq_number, sizeof(*port->rxqs),
				  GFP_KERNEL);
	if (!port->rxqs)
		return -ENOMEM;

	/* Allocate and initialize Rx queue for this port */
	for (queue = 0; queue < rxq_number; queue++) {
		struct mvpp2_rx_queue *rxq;

		/* Map physical Rx queue to port's logical Rx queue */
		rxq = devm_kzalloc(dev, sizeof(*rxq), GFP_KERNEL);
		if (!rxq)
			return -ENOMEM;
		/* Map this Rx queue to a physical queue */
		rxq->id = port->first_rxq + queue;
		rxq->port = port->id;
		rxq->logic_rxq = queue;

		port->rxqs[queue] = rxq;
	}


	/* Create Rx descriptor rings */
	for (queue = 0; queue < rxq_number; queue++) {
		struct mvpp2_rx_queue *rxq = port->rxqs[queue];

		rxq->size = port->rx_ring_size;
		rxq->pkts_coal = MVPP2_RX_COAL_PKTS;
		rxq->time_coal = MVPP2_RX_COAL_USEC;
	}

	mvpp2_ingress_disable(port);

	/* Port default configuration */
	mvpp2_defaults_set(port);

	/* Port's classifier configuration */
	mvpp2_cls_oversize_rxq_set(port);
	mvpp2_cls_port_config(port);

	/* Provide an initial Rx packet size */
	port->pkt_size = MVPP2_RX_PKT_SIZE(PKTSIZE_ALIGN);

	/* Initialize pools for swf */
	err = mvpp2_swf_bm_pool_init(port);
	if (err)
		return err;

	return 0;
}

static int phy_info_parse(struct udevice *dev, struct mvpp2_port *port)
{
	int port_node = dev_of_offset(dev);
	const char *phy_mode_str;
	int phy_node;
	u32 id;
	u32 phyaddr = 0;
	int phy_mode = -1;

	/* Default mdio_base from the same eth base */
	if (port->priv->hw_version == MVPP21)
		port->mdio_base = port->priv->lms_base + MVPP21_SMI;
	else
		port->mdio_base = port->priv->iface_base + MVPP22_SMI;

	phy_node = fdtdec_lookup_phandle(gd->fdt_blob, port_node, "phy");

	if (phy_node > 0) {
		ofnode phy_ofnode;
		fdt_addr_t phy_base;

		phyaddr = fdtdec_get_int(gd->fdt_blob, phy_node, "reg", 0);
		if (phyaddr < 0) {
			dev_err(&pdev->dev, "could not find phy address\n");
			return -1;
		}

		phy_ofnode = ofnode_get_parent(offset_to_ofnode(phy_node));
		phy_base = ofnode_get_addr(phy_ofnode);
		port->mdio_base = (void *)phy_base;

		if (port->mdio_base < 0) {
			dev_err(&pdev->dev, "could not find mdio base address\n");
			return -1;
		}
	} else {
		phy_node = 0;
	}

	phy_mode_str = fdt_getprop(gd->fdt_blob, port_node, "phy-mode", NULL);
	if (phy_mode_str)
		phy_mode = phy_get_interface_by_name(phy_mode_str);
	if (phy_mode == -1) {
		dev_err(&pdev->dev, "incorrect phy mode\n");
		return -EINVAL;
	}

	id = fdtdec_get_int(gd->fdt_blob, port_node, "port-id", -1);
	if (id == -1) {
		dev_err(&pdev->dev, "missing port-id value\n");
		return -EINVAL;
	}

#ifdef CONFIG_DM_GPIO
	gpio_request_by_name(dev, "phy-reset-gpios", 0,
			     &port->phy_reset_gpio, GPIOD_IS_OUT);
	gpio_request_by_name(dev, "marvell,sfp-tx-disable-gpio", 0,
			     &port->phy_tx_disable_gpio, GPIOD_IS_OUT);
#endif

	/*
	 * ToDo:
	 * Not sure if this DT property "phy-speed" will get accepted, so
	 * this might change later
	 */
	/* Get phy-speed for SGMII 2.5Gbps vs 1Gbps setup */
	port->phy_speed = fdtdec_get_int(gd->fdt_blob, port_node,
					 "phy-speed", 1000);

	port->id = id;
	if (port->priv->hw_version == MVPP21)
		port->first_rxq = port->id * rxq_number;
	else
		port->first_rxq = port->id * port->priv->max_port_rxqs;
	port->phy_node = phy_node;
	port->phy_interface = phy_mode;
	port->phyaddr = phyaddr;

	return 0;
}

#ifdef CONFIG_DM_GPIO
/* Port GPIO initialization */
static void mvpp2_gpio_init(struct mvpp2_port *port)
{
	if (dm_gpio_is_valid(&port->phy_reset_gpio)) {
		dm_gpio_set_value(&port->phy_reset_gpio, 1);
		mdelay(10);
		dm_gpio_set_value(&port->phy_reset_gpio, 0);
	}

	if (dm_gpio_is_valid(&port->phy_tx_disable_gpio))
		dm_gpio_set_value(&port->phy_tx_disable_gpio, 0);
}
#endif

/* Ports initialization */
static int mvpp2_port_probe(struct udevice *dev,
			    struct mvpp2_port *port,
			    int port_node,
			    struct mvpp2 *priv)
{
	int err;

	port->tx_ring_size = MVPP2_MAX_TXD;
	port->rx_ring_size = MVPP2_MAX_RXD;

	err = mvpp2_port_init(dev, port);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to init port %d\n", port->id);
		return err;
	}
	mvpp2_port_power_up(port);

#ifdef CONFIG_DM_GPIO
	mvpp2_gpio_init(port);
#endif

	priv->port_list[port->id] = port;
	priv->num_ports++;
	return 0;
}

/* Initialize decoding windows */
static void mvpp2_conf_mbus_windows(const struct mbus_dram_target_info *dram,
				    struct mvpp2 *priv)
{
	u32 win_enable;
	int i;

	for (i = 0; i < 6; i++) {
		mvpp2_write(priv, MVPP2_WIN_BASE(i), 0);
		mvpp2_write(priv, MVPP2_WIN_SIZE(i), 0);

		if (i < 4)
			mvpp2_write(priv, MVPP2_WIN_REMAP(i), 0);
	}

	win_enable = 0;

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		mvpp2_write(priv, MVPP2_WIN_BASE(i),
			    (cs->base & 0xffff0000) | (cs->mbus_attr << 8) |
			    dram->mbus_dram_target_id);

		mvpp2_write(priv, MVPP2_WIN_SIZE(i),
			    (cs->size - 1) & 0xffff0000);

		win_enable |= (1 << i);
	}

	mvpp2_write(priv, MVPP2_BASE_ADDR_ENABLE, win_enable);
}

/* Initialize Rx FIFO's */
static void mvpp2_rx_fifo_init(struct mvpp2 *priv)
{
	int port;

	for (port = 0; port < MVPP2_MAX_PORTS; port++) {
		if (priv->hw_version == MVPP22) {
			if (port == 0) {
				mvpp2_write(priv,
					    MVPP2_RX_DATA_FIFO_SIZE_REG(port),
					    MVPP22_RX_FIFO_10GB_PORT_DATA_SIZE);
				mvpp2_write(priv,
					    MVPP2_RX_ATTR_FIFO_SIZE_REG(port),
					    MVPP22_RX_FIFO_10GB_PORT_ATTR_SIZE);
			} else if (port == 1) {
				mvpp2_write(priv,
					    MVPP2_RX_DATA_FIFO_SIZE_REG(port),
					    MVPP22_RX_FIFO_2_5GB_PORT_DATA_SIZE);
				mvpp2_write(priv,
					    MVPP2_RX_ATTR_FIFO_SIZE_REG(port),
					    MVPP22_RX_FIFO_2_5GB_PORT_ATTR_SIZE);
			} else {
				mvpp2_write(priv,
					    MVPP2_RX_DATA_FIFO_SIZE_REG(port),
					    MVPP22_RX_FIFO_1GB_PORT_DATA_SIZE);
				mvpp2_write(priv,
					    MVPP2_RX_ATTR_FIFO_SIZE_REG(port),
					    MVPP22_RX_FIFO_1GB_PORT_ATTR_SIZE);
			}
		} else {
			mvpp2_write(priv, MVPP2_RX_DATA_FIFO_SIZE_REG(port),
				    MVPP21_RX_FIFO_PORT_DATA_SIZE);
			mvpp2_write(priv, MVPP2_RX_ATTR_FIFO_SIZE_REG(port),
				    MVPP21_RX_FIFO_PORT_ATTR_SIZE);
		}
	}

	mvpp2_write(priv, MVPP2_RX_MIN_PKT_SIZE_REG,
		    MVPP2_RX_FIFO_PORT_MIN_PKT);
	mvpp2_write(priv, MVPP2_RX_FIFO_INIT_REG, 0x1);
}

/* Initialize Tx FIFO's */
static void mvpp2_tx_fifo_init(struct mvpp2 *priv)
{
	int port, val;

	for (port = 0; port < MVPP2_MAX_PORTS; port++) {
		/* Port 0 supports 10KB TX FIFO */
		if (port == 0) {
			val = MVPP2_TX_FIFO_DATA_SIZE_10KB &
				MVPP22_TX_FIFO_SIZE_MASK;
		} else {
			val = MVPP2_TX_FIFO_DATA_SIZE_3KB &
				MVPP22_TX_FIFO_SIZE_MASK;
		}
		mvpp2_write(priv, MVPP22_TX_FIFO_SIZE_REG(port), val);
	}
}

static void mvpp2_axi_init(struct mvpp2 *priv)
{
	u32 val, rdval, wrval;

	mvpp2_write(priv, MVPP22_BM_ADDR_HIGH_RLS_REG, 0x0);

	/* AXI Bridge Configuration */

	rdval = MVPP22_AXI_CODE_CACHE_RD_CACHE
		<< MVPP22_AXI_ATTR_CACHE_OFFS;
	rdval |= MVPP22_AXI_CODE_DOMAIN_OUTER_DOM
		<< MVPP22_AXI_ATTR_DOMAIN_OFFS;

	wrval = MVPP22_AXI_CODE_CACHE_WR_CACHE
		<< MVPP22_AXI_ATTR_CACHE_OFFS;
	wrval |= MVPP22_AXI_CODE_DOMAIN_OUTER_DOM
		<< MVPP22_AXI_ATTR_DOMAIN_OFFS;

	/* BM */
	mvpp2_write(priv, MVPP22_AXI_BM_WR_ATTR_REG, wrval);
	mvpp2_write(priv, MVPP22_AXI_BM_RD_ATTR_REG, rdval);

	/* Descriptors */
	mvpp2_write(priv, MVPP22_AXI_AGGRQ_DESCR_RD_ATTR_REG, rdval);
	mvpp2_write(priv, MVPP22_AXI_TXQ_DESCR_WR_ATTR_REG, wrval);
	mvpp2_write(priv, MVPP22_AXI_TXQ_DESCR_RD_ATTR_REG, rdval);
	mvpp2_write(priv, MVPP22_AXI_RXQ_DESCR_WR_ATTR_REG, wrval);

	/* Buffer Data */
	mvpp2_write(priv, MVPP22_AXI_TX_DATA_RD_ATTR_REG, rdval);
	mvpp2_write(priv, MVPP22_AXI_RX_DATA_WR_ATTR_REG, wrval);

	val = MVPP22_AXI_CODE_CACHE_NON_CACHE
		<< MVPP22_AXI_CODE_CACHE_OFFS;
	val |= MVPP22_AXI_CODE_DOMAIN_SYSTEM
		<< MVPP22_AXI_CODE_DOMAIN_OFFS;
	mvpp2_write(priv, MVPP22_AXI_RD_NORMAL_CODE_REG, val);
	mvpp2_write(priv, MVPP22_AXI_WR_NORMAL_CODE_REG, val);

	val = MVPP22_AXI_CODE_CACHE_RD_CACHE
		<< MVPP22_AXI_CODE_CACHE_OFFS;
	val |= MVPP22_AXI_CODE_DOMAIN_OUTER_DOM
		<< MVPP22_AXI_CODE_DOMAIN_OFFS;

	mvpp2_write(priv, MVPP22_AXI_RD_SNOOP_CODE_REG, val);

	val = MVPP22_AXI_CODE_CACHE_WR_CACHE
		<< MVPP22_AXI_CODE_CACHE_OFFS;
	val |= MVPP22_AXI_CODE_DOMAIN_OUTER_DOM
		<< MVPP22_AXI_CODE_DOMAIN_OFFS;

	mvpp2_write(priv, MVPP22_AXI_WR_SNOOP_CODE_REG, val);
}

/* Initialize network controller common part HW */
static int mvpp2_init(struct udevice *dev, struct mvpp2 *priv)
{
	const struct mbus_dram_target_info *dram_target_info;
	int err, i;
	u32 val;

	/* Checks for hardware constraints (U-Boot uses only one rxq) */
	if ((rxq_number > priv->max_port_rxqs) ||
	    (txq_number > MVPP2_MAX_TXQ)) {
		dev_err(&pdev->dev, "invalid queue size parameter\n");
		return -EINVAL;
	}

	if (priv->hw_version == MVPP22)
		mvpp2_axi_init(priv);
	else {
		/* MBUS windows configuration */
		dram_target_info = mvebu_mbus_dram_info();
		if (dram_target_info)
			mvpp2_conf_mbus_windows(dram_target_info, priv);
	}

	if (priv->hw_version == MVPP21) {
		/* Disable HW PHY polling */
		val = readl(priv->lms_base + MVPP2_PHY_AN_CFG0_REG);
		val |= MVPP2_PHY_AN_STOP_SMI0_MASK;
		writel(val, priv->lms_base + MVPP2_PHY_AN_CFG0_REG);
	} else {
		/* Enable HW PHY polling */
		val = readl(priv->iface_base + MVPP22_SMI_MISC_CFG_REG);
		val |= MVPP22_SMI_POLLING_EN;
		writel(val, priv->iface_base + MVPP22_SMI_MISC_CFG_REG);
	}

	/* Allocate and initialize aggregated TXQs */
	priv->aggr_txqs = devm_kcalloc(dev, num_present_cpus(),
				       sizeof(struct mvpp2_tx_queue),
				       GFP_KERNEL);
	if (!priv->aggr_txqs)
		return -ENOMEM;

	for_each_present_cpu(i) {
		priv->aggr_txqs[i].id = i;
		priv->aggr_txqs[i].size = MVPP2_AGGR_TXQ_SIZE;
		err = mvpp2_aggr_txq_init(dev, &priv->aggr_txqs[i],
					  MVPP2_AGGR_TXQ_SIZE, i, priv);
		if (err < 0)
			return err;
	}

	/* Rx Fifo Init */
	mvpp2_rx_fifo_init(priv);

	/* Tx Fifo Init */
	if (priv->hw_version == MVPP22)
		mvpp2_tx_fifo_init(priv);

	if (priv->hw_version == MVPP21)
		writel(MVPP2_EXT_GLOBAL_CTRL_DEFAULT,
		       priv->lms_base + MVPP2_MNG_EXTENDED_GLOBAL_CTRL_REG);

	/* Allow cache snoop when transmiting packets */
	mvpp2_write(priv, MVPP2_TX_SNOOP_REG, 0x1);

	/* Buffer Manager initialization */
	err = mvpp2_bm_init(dev, priv);
	if (err < 0)
		return err;

	/* Parser default initialization */
	err = mvpp2_prs_default_init(dev, priv);
	if (err < 0)
		return err;

	/* Classifier default initialization */
	mvpp2_cls_init(priv);

	return 0;
}

/* SMI / MDIO functions */

static int smi_wait_ready(struct mvpp2_port *priv)
{
	u32 timeout = MVPP2_SMI_TIMEOUT;
	u32 smi_reg;

	/* wait till the SMI is not busy */
	do {
		/* read smi register */
		smi_reg = readl(priv->mdio_base);
		if (timeout-- == 0) {
			printf("Error: SMI busy timeout\n");
			return -EFAULT;
		}
	} while (smi_reg & MVPP2_SMI_BUSY);

	return 0;
}

/*
 * mpp2_mdio_read - miiphy_read callback function.
 *
 * Returns 16bit phy register value, or 0xffff on error
 */
static int mpp2_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mvpp2_port *priv = bus->priv;
	u32 smi_reg;
	u32 timeout;

	/* check parameters */
	if (addr > MVPP2_PHY_ADDR_MASK) {
		printf("Error: Invalid PHY address %d\n", addr);
		return -EFAULT;
	}

	if (reg > MVPP2_PHY_REG_MASK) {
		printf("Err: Invalid register offset %d\n", reg);
		return -EFAULT;
	}

	/* wait till the SMI is not busy */
	if (smi_wait_ready(priv) < 0)
		return -EFAULT;

	/* fill the phy address and regiser offset and read opcode */
	smi_reg = (addr << MVPP2_SMI_DEV_ADDR_OFFS)
		| (reg << MVPP2_SMI_REG_ADDR_OFFS)
		| MVPP2_SMI_OPCODE_READ;

	/* write the smi register */
	writel(smi_reg, priv->mdio_base);

	/* wait till read value is ready */
	timeout = MVPP2_SMI_TIMEOUT;

	do {
		/* read smi register */
		smi_reg = readl(priv->mdio_base);
		if (timeout-- == 0) {
			printf("Err: SMI read ready timeout\n");
			return -EFAULT;
		}
	} while (!(smi_reg & MVPP2_SMI_READ_VALID));

	/* Wait for the data to update in the SMI register */
	for (timeout = 0; timeout < MVPP2_SMI_TIMEOUT; timeout++)
		;

	return readl(priv->mdio_base) & MVPP2_SMI_DATA_MASK;
}

/*
 * mpp2_mdio_write - miiphy_write callback function.
 *
 * Returns 0 if write succeed, -EINVAL on bad parameters
 * -ETIME on timeout
 */
static int mpp2_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			   u16 value)
{
	struct mvpp2_port *priv = bus->priv;
	u32 smi_reg;

	/* check parameters */
	if (addr > MVPP2_PHY_ADDR_MASK) {
		printf("Error: Invalid PHY address %d\n", addr);
		return -EFAULT;
	}

	if (reg > MVPP2_PHY_REG_MASK) {
		printf("Err: Invalid register offset %d\n", reg);
		return -EFAULT;
	}

	/* wait till the SMI is not busy */
	if (smi_wait_ready(priv) < 0)
		return -EFAULT;

	/* fill the phy addr and reg offset and write opcode and data */
	smi_reg = value << MVPP2_SMI_DATA_OFFS;
	smi_reg |= (addr << MVPP2_SMI_DEV_ADDR_OFFS)
		| (reg << MVPP2_SMI_REG_ADDR_OFFS);
	smi_reg &= ~MVPP2_SMI_OPCODE_READ;

	/* write the smi register */
	writel(smi_reg, priv->mdio_base);

	return 0;
}

static int mvpp2_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2_rx_desc *rx_desc;
	struct mvpp2_bm_pool *bm_pool;
	dma_addr_t dma_addr;
	u32 bm, rx_status;
	int pool, rx_bytes, err;
	int rx_received;
	struct mvpp2_rx_queue *rxq;
	u8 *data;

	/* Process RX packets */
	rxq = port->rxqs[0];

	/* Get number of received packets and clamp the to-do */
	rx_received = mvpp2_rxq_received(port, rxq->id);

	/* Return if no packets are received */
	if (!rx_received)
		return 0;

	rx_desc = mvpp2_rxq_next_desc_get(rxq);
	rx_status = mvpp2_rxdesc_status_get(port, rx_desc);
	rx_bytes = mvpp2_rxdesc_size_get(port, rx_desc);
	rx_bytes -= MVPP2_MH_SIZE;
	dma_addr = mvpp2_rxdesc_dma_addr_get(port, rx_desc);

	bm = mvpp2_bm_cookie_build(port, rx_desc);
	pool = mvpp2_bm_cookie_pool_get(bm);
	bm_pool = &port->priv->bm_pools[pool];

	/* In case of an error, release the requested buffer pointer
	 * to the Buffer Manager. This request process is controlled
	 * by the hardware, and the information about the buffer is
	 * comprised by the RX descriptor.
	 */
	if (rx_status & MVPP2_RXD_ERR_SUMMARY) {
		mvpp2_rx_error(port, rx_desc);
		/* Return the buffer to the pool */
		mvpp2_pool_refill(port, bm, dma_addr, dma_addr);
		return 0;
	}

	err = mvpp2_rx_refill(port, bm_pool, bm, dma_addr);
	if (err) {
		netdev_err(port->dev, "failed to refill BM pools\n");
		return 0;
	}

	/* Update Rx queue management counters */
	mb();
	mvpp2_rxq_status_update(port, rxq->id, 1, 1);

	/* give packet to stack - skip on first n bytes */
	data = (u8 *)dma_addr + 2 + 32;

	if (rx_bytes <= 0)
		return 0;

	/*
	 * No cache invalidation needed here, since the rx_buffer's are
	 * located in a uncached memory region
	 */
	*packetp = data;

	return rx_bytes;
}

static int mvpp2_send(struct udevice *dev, void *packet, int length)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2_tx_queue *txq, *aggr_txq;
	struct mvpp2_tx_desc *tx_desc;
	int tx_done;
	int timeout;

	txq = port->txqs[0];
	aggr_txq = &port->priv->aggr_txqs[smp_processor_id()];

	/* Get a descriptor for the first part of the packet */
	tx_desc = mvpp2_txq_next_desc_get(aggr_txq);
	mvpp2_txdesc_txq_set(port, tx_desc, txq->id);
	mvpp2_txdesc_size_set(port, tx_desc, length);
	mvpp2_txdesc_offset_set(port, tx_desc,
				(dma_addr_t)packet & MVPP2_TX_DESC_ALIGN);
	mvpp2_txdesc_dma_addr_set(port, tx_desc,
				  (dma_addr_t)packet & ~MVPP2_TX_DESC_ALIGN);
	/* First and Last descriptor */
	mvpp2_txdesc_cmd_set(port, tx_desc,
			     MVPP2_TXD_L4_CSUM_NOT | MVPP2_TXD_IP_CSUM_DISABLE
			     | MVPP2_TXD_F_DESC | MVPP2_TXD_L_DESC);

	/* Flush tx data */
	flush_dcache_range((unsigned long)packet,
			   (unsigned long)packet + ALIGN(length, PKTALIGN));

	/* Enable transmit */
	mb();
	mvpp2_aggr_txq_pend_desc_add(port, 1);

	mvpp2_write(port->priv, MVPP2_TXQ_NUM_REG, txq->id);

	timeout = 0;
	do {
		if (timeout++ > 10000) {
			printf("timeout: packet not sent from aggregated to phys TXQ\n");
			return 0;
		}
		tx_done = mvpp2_txq_pend_desc_num_get(port, txq);
	} while (tx_done);

	timeout = 0;
	do {
		if (timeout++ > 10000) {
			printf("timeout: packet not sent\n");
			return 0;
		}
		tx_done = mvpp2_txq_sent_desc_proc(port, txq);
	} while (!tx_done);

	return 0;
}

static int mvpp2_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mvpp2_port *port = dev_get_priv(dev);

	/* Load current MAC address */
	memcpy(port->dev_addr, pdata->enetaddr, ETH_ALEN);

	/* Reconfigure parser accept the original MAC address */
	mvpp2_prs_update_mac_da(port, port->dev_addr);

	switch (port->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_SGMII:
		mvpp2_port_power_up(port);
	default:
		break;
	}

	mvpp2_open(dev, port);

	return 0;
}

static void mvpp2_stop(struct udevice *dev)
{
	struct mvpp2_port *port = dev_get_priv(dev);

	mvpp2_stop_dev(port);
	mvpp2_cleanup_rxqs(port);
	mvpp2_cleanup_txqs(port);
}

static int mvpp22_smi_phy_addr_cfg(struct mvpp2_port *port)
{
	writel(port->phyaddr, port->priv->iface_base +
	       MVPP22_SMI_PHY_ADDR_REG(port->gop_id));

	return 0;
}

static int mvpp2_base_probe(struct udevice *dev)
{
	struct mvpp2 *priv = dev_get_priv(dev);
	void *bd_space;
	u32 size = 0;
	int i;

	/* Save hw-version */
	priv->hw_version = dev_get_driver_data(dev);

	/*
	 * U-Boot special buffer handling:
	 *
	 * Allocate buffer area for descs and rx_buffers. This is only
	 * done once for all interfaces. As only one interface can
	 * be active. Make this area DMA-safe by disabling the D-cache
	 */

	/* Align buffer area for descs and rx_buffers to 1MiB */
	bd_space = memalign(1 << MMU_SECTION_SHIFT, BD_SPACE);
	mmu_set_region_dcache_behaviour((unsigned long)bd_space,
					BD_SPACE, DCACHE_OFF);

	buffer_loc.aggr_tx_descs = (struct mvpp2_tx_desc *)bd_space;
	size += MVPP2_AGGR_TXQ_SIZE * MVPP2_DESC_ALIGNED_SIZE;

	buffer_loc.tx_descs =
		(struct mvpp2_tx_desc *)((unsigned long)bd_space + size);
	size += MVPP2_MAX_TXD * MVPP2_DESC_ALIGNED_SIZE;

	buffer_loc.rx_descs =
		(struct mvpp2_rx_desc *)((unsigned long)bd_space + size);
	size += MVPP2_MAX_RXD * MVPP2_DESC_ALIGNED_SIZE;

	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++) {
		buffer_loc.bm_pool[i] =
			(unsigned long *)((unsigned long)bd_space + size);
		if (priv->hw_version == MVPP21)
			size += MVPP2_BM_POOL_SIZE_MAX * 2 * sizeof(u32);
		else
			size += MVPP2_BM_POOL_SIZE_MAX * 2 * sizeof(u64);
	}

	for (i = 0; i < MVPP2_BM_LONG_BUF_NUM; i++) {
		buffer_loc.rx_buffer[i] =
			(unsigned long *)((unsigned long)bd_space + size);
		size += RX_BUFFER_SIZE;
	}

	/* Clear the complete area so that all descriptors are cleared */
	memset(bd_space, 0, size);

	/* Save base addresses for later use */
	priv->base = (void *)devfdt_get_addr_index(dev, 0);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	if (priv->hw_version == MVPP21) {
		priv->lms_base = (void *)devfdt_get_addr_index(dev, 1);
		if (IS_ERR(priv->lms_base))
			return PTR_ERR(priv->lms_base);
	} else {
		priv->iface_base = (void *)devfdt_get_addr_index(dev, 1);
		if (IS_ERR(priv->iface_base))
			return PTR_ERR(priv->iface_base);

		/* Store common base addresses for all ports */
		priv->mpcs_base = priv->iface_base + MVPP22_MPCS;
		priv->xpcs_base = priv->iface_base + MVPP22_XPCS;
		priv->rfu1_base = priv->iface_base + MVPP22_RFU1;
	}

	if (priv->hw_version == MVPP21)
		priv->max_port_rxqs = 8;
	else
		priv->max_port_rxqs = 32;

	return 0;
}

static int mvpp2_probe(struct udevice *dev)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2 *priv = dev_get_priv(dev->parent);
	struct mii_dev *bus;
	int err;

	/* Only call the probe function for the parent once */
	if (!priv->probe_done)
		err = mvpp2_base_probe(dev->parent);

	port->priv = dev_get_priv(dev->parent);

	/* Create and register the MDIO bus driver */
	bus = mdio_alloc();
	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = mpp2_mdio_read;
	bus->write = mpp2_mdio_write;
	snprintf(bus->name, sizeof(bus->name), dev->name);
	bus->priv = (void *)port;
	port->bus = bus;

	err = mdio_register(bus);
	if (err)
		return err;

	err = phy_info_parse(dev, port);
	if (err)
		return err;

	/*
	 * We need the port specific io base addresses at this stage, since
	 * gop_port_init() accesses these registers
	 */
	if (priv->hw_version == MVPP21) {
		int priv_common_regs_num = 2;

		port->base = (void __iomem *)devfdt_get_addr_index(
			dev->parent, priv_common_regs_num + port->id);
		if (IS_ERR(port->base))
			return PTR_ERR(port->base);
	} else {
		port->gop_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "gop-port-id", -1);
		if (port->id == -1) {
			dev_err(&pdev->dev, "missing gop-port-id value\n");
			return -EINVAL;
		}

		port->base = priv->iface_base + MVPP22_PORT_BASE +
			port->gop_id * MVPP22_PORT_OFFSET;

		/* Set phy address of the port */
		if(port->phy_node)
			mvpp22_smi_phy_addr_cfg(port);

		/* GoP Init */
		gop_port_init(port);
	}

	if (!priv->probe_done) {
		/* Initialize network controller */
		err = mvpp2_init(dev, priv);
		if (err < 0) {
			dev_err(&pdev->dev, "failed to initialize controller\n");
			return err;
		}
		priv->num_ports = 0;
		priv->probe_done = 1;
	}

	err = mvpp2_port_probe(dev, port, dev_of_offset(dev), priv);
	if (err)
		return err;

	if (priv->hw_version == MVPP22) {
		priv->netc_config |= mvpp2_netc_cfg_create(port->gop_id,
							   port->phy_interface);

		/* Netcomplex configurations for all ports */
		gop_netc_init(priv, MV_NETC_FIRST_PHASE);
		gop_netc_init(priv, MV_NETC_SECOND_PHASE);
	}

	return 0;
}

/*
 * Empty BM pool and stop its activity before the OS is started
 */
static int mvpp2_remove(struct udevice *dev)
{
	struct mvpp2_port *port = dev_get_priv(dev);
	struct mvpp2 *priv = port->priv;
	int i;

	priv->num_ports--;

	if (priv->num_ports)
		return 0;

	for (i = 0; i < MVPP2_BM_POOLS_NUM; i++)
		mvpp2_bm_pool_destroy(dev, priv, &priv->bm_pools[i]);

	return 0;
}

static const struct eth_ops mvpp2_ops = {
	.start		= mvpp2_start,
	.send		= mvpp2_send,
	.recv		= mvpp2_recv,
	.stop		= mvpp2_stop,
};

static struct driver mvpp2_driver = {
	.name	= "mvpp2",
	.id	= UCLASS_ETH,
	.probe	= mvpp2_probe,
	.remove = mvpp2_remove,
	.ops	= &mvpp2_ops,
	.priv_auto_alloc_size = sizeof(struct mvpp2_port),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags	= DM_FLAG_ACTIVE_DMA,
};

/*
 * Use a MISC device to bind the n instances (child nodes) of the
 * network base controller in UCLASS_ETH.
 */
static int mvpp2_base_bind(struct udevice *parent)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(parent);
	struct uclass_driver *drv;
	struct udevice *dev;
	struct eth_pdata *plat;
	char *name;
	int subnode;
	u32 id;
	int base_id_add;

	/* Lookup eth driver */
	drv = lists_uclass_lookup(UCLASS_ETH);
	if (!drv) {
		puts("Cannot find eth driver\n");
		return -ENOENT;
	}

	base_id_add = base_id;

	fdt_for_each_subnode(subnode, blob, node) {
		/* Increment base_id for all subnodes, also the disabled ones */
		base_id++;

		/* Skip disabled ports */
		if (!fdtdec_get_is_enabled(blob, subnode))
			continue;

		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;

		id = fdtdec_get_int(blob, subnode, "port-id", -1);
		id += base_id_add;

		name = calloc(1, 16);
		if (!name) {
			free(plat);
			return -ENOMEM;
		}
		sprintf(name, "mvpp2-%d", id);

		/* Create child device UCLASS_ETH and bind it */
		device_bind(parent, &mvpp2_driver, name, plat, subnode, &dev);
		dev_set_of_offset(dev, subnode);
	}

	return 0;
}

static const struct udevice_id mvpp2_ids[] = {
	{
		.compatible = "marvell,armada-375-pp2",
		.data = MVPP21,
	},
	{
		.compatible = "marvell,armada-7k-pp22",
		.data = MVPP22,
	},
	{ }
};

U_BOOT_DRIVER(mvpp2_base) = {
	.name	= "mvpp2_base",
	.id	= UCLASS_MISC,
	.of_match = mvpp2_ids,
	.bind	= mvpp2_base_bind,
	.priv_auto_alloc_size = sizeof(struct mvpp2),
};
