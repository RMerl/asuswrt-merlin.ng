/*
 * BCM43XX M2M DMA core hardware definitions.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id:m2mdma _core.h 421139 2013-08-30 17:56:15Z kiranm $
 */

#ifndef	_M2MDMA_CORE_H
#define	_M2MDMA_CORE_H
#include <sbhnddma.h>
/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif

enum {
	M2MDMA_CORE_0 = 0,
	M2MDMA_CORE_1,
	M2MDMA_CORE_MAX
};

/* Maximum number of DMA channels that may be instantiated in a M2MCORE.
 * Actual number of channels instantiated is defined per M2MCORE revision.
 */
#define M2M_CORE_CHANNELS   8
#define M2M_CORE_CH0_OFFSET (0x200)

/* M2MCORE DMA Engine (pair of transmit and receive DMA Processors) Registers */
typedef volatile struct m2m_eng_regs {
	dma64regs_t tx;     /* Transmit DMA Processor */
	uint32      PAD[2];
	dma64regs_t rx;     /* Receive DMA Processor */
	uint32      PAD[2];
} m2m_eng_regs_t;       /* M2M DMA Engine: pair of Tx and Rx DMA Processor */

/* M2MCORE per M2M Channel (DMA Processor pair) IntStatus and IntMask */
typedef volatile struct m2m_int_regs {
	uint32 intstatus;   /* DE=b10, DA=b11, DP=b12, RU=b13; RI=b16, XI=b24 */
	uint32 intmask;
} m2m_int_regs_t;

#define M2M_PGR_SLOT_MAX	2

/* M2M PGR registers used in STS_XFER */
typedef volatile struct m2m_pgr_regs
{
	uint32 mem_base_l;                           /* 0x00 */
	uint32 cfg;                                  /* 0x04 */
	uint32 slot_ctrl[M2M_PGR_SLOT_MAX];          /* 0x08 - 0x0C */
	uint32 slot_status[M2M_PGR_SLOT_MAX];        /* 0x10 - 0x14 */
} m2m_pgr_regs_t;

#define M2M_PGR_CFG_AP_MODE_SHIFT      0
#define M2M_PGR_CFG_AP_MODE_MASK \
	(0x3 << M2M_PGR_CFG_AP_MODE_SHIFT)
#define M2M_PGR_CFG_PGR_CONFIG_SHIFT      4
#define M2M_PGR_CFG_PGR_CONFIG_MASK \
	(0xf << M2M_PGR_CFG_PGR_CONFIG_SHIFT)

/* Rename */
#define M2M_PGR_CFG_OP_ROLE_SHIFT   M2M_PGR_CFG_AP_MODE_SHIFT
#define M2M_PGR_CFG_OP_ROLE_MASK    M2M_PGR_CFG_AP_MODE_MASK
#define M2M_PGR_CFG_OP_MODE_SHIFT   M2M_PGR_CFG_PGR_CONFIG_SHIFT
#define M2M_PGR_CFG_OP_MODE_MASK    M2M_PGR_CFG_PGR_CONFIG_MASK

typedef enum m2m_pgr_op_mode                    /* Pager Operational Mode */
{
	m2m_pgr_op_mode_disabled_e      = 0,    /* Disabled */
	m2m_pgr_op_mode_autonomous_e    = 1,    /* M2M PGR autonomously pages in vacant */
	m2m_pgr_op_mode_directed_e      = 2,    /* SW directs a page-in into slot */
	m2m_pgr_op_mode_max_e           = 3
} m2m_pgr_op_mode_t;

typedef enum m2m_pgr_op_role
{
	m2m_pgr_op_role_disabled_e      = 0,
	m2m_pgr_op_role_consumer_e      = 1,
	m2m_pgr_op_role_producer_e      = 2,
	m2m_pgr_op_role_prodcons_e      = 3
} m2m_pgr_op_role_t;

/*  In autonomous mode, SW writes 0xFFFF into a slot_ctrl to indicate that
 *  the message in the slot has been consumed and the slot is vacated
 */
#define M2M_PGR_MSG_VACATE          (0xFFFF)
/* M2M PGR records in a slot_status that a DMA is ongoing. */
#define M2M_PGR_IN_DMAXFER          (0xFFFF)

#ifdef BCMQT
#define M2M_PGR_IN_POLLLOOP	100
#else
#define M2M_PGR_IN_POLLLOOP	10
#endif

/* M2MCORE STATUS (TXS and PHYRXS) Channel Registers */
typedef volatile struct m2m_status_eng_regs {
	uint32 cfg;                                  /* 0x0 */
	uint32 ctl;                                  /* 0x4 */
	uint32 sts;                                  /* 0x8 */
	uint32 debug;                                /* 0xC */
	uint32 da_base_l;                            /* 0x10 */
	uint32 da_base_h;                            /* 0x14 */
	uint32 size;                                 /* 0x18 */
	uint32 wridx;                                /* 0x1C */
	uint32 rdidx;                                /* 0x20 */
	uint32 dma_template;                         /* 0x24 */
	uint32 sa_base_l;                            /* 0x28 */
	uint32 sa_base_h;                            /* 0x2C */
	uint32 rdidx_reg_l;                          /* 0x30 */
	uint32 rdidx_reg_h;                          /* 0x34 */
	uint32 wridx_reg_l;                          /* 0x38 */
	uint32 wridx_reg_h;                          /* 0x3C */
	m2m_pgr_regs_t pgr;                          /* 0x40 - 0x54 */
} m2m_status_eng_regs_t;

/* M2MCORE CPU DBG Channel Registers */
typedef volatile struct m2m_cpudbg_eng_regs {
	uint32 cfg;                                  /* 0x0 */
	uint32 ctl;                                  /* 0x4 */
	uint32 sts;                                  /* 0x8 */
	uint32 debug;                                /* 0xC */
	uint32 da_base_l;                            /* 0x10 */
	uint32 da_base_h;                            /* 0x14 */
	uint32 size;                                 /* 0x18 */
	uint32 wridx;                                /* 0x1C */
	uint32 rdidx;                                /* 0x20 */
	uint32 dma_template;                         /* 0x24 */
	uint32 sa_base_l;                            /* 0x28 */
	uint32 sa_base_h;                            /* 0x2C */
} m2m_cpudbg_eng_regs_t;

typedef struct m2m_ucls_cmd_sts {
	union {
		uint32 uval32;
		struct {
			uint32 len: 16;
			uint32 id: 12;
			uint32 flags: 3;
			uint32 reserved: 1;
		};
	};
} m2m_ucls_cmd_sts_t;

#define UCLS_PCI64ADDR_HIGH		0x80000000	/* PCI64ADDR_HIGH */
#define UCLS_SLOT_FLAG_NL		0x1		/* New line ended */
#define UCLS_SLOT_FLAG_OVERFLOW		0x2		/* Slot buffer overflow */
#define UCLS_SLOT_FLAG_WRAP		0x4		/* Dongle buffer wrap around */

#define M2M_UCLS_ENG_M2M_CHANNEL	6		/* M2M Core1/Ch6 in corerev >= 134 */

#define M2M_UCLS_SLOT_PING		0
#define M2M_UCLS_SLOT_PONG		1
#define M2M_UCLS_SLOT_MAX		2

/* M2MCORE UCLS Registers */
typedef struct m2m_ucls_eng_regs {
	uint32 cfg;                                  /* 0x0 */
	uint32 ctl;                                  /* 0x4 */
	uint32 debug;                                /* 0x8 */
	uint32 depth;                                /* 0xC */
	uint32 wridx;                                /* 0x10 */
	uint32 rdidx;                                /* 0x14 */
	uint32 doorbell_addr;                        /* 0x18 */
	uint32 dma_template;                         /* 0x1C */
	uint32 da_base_l;                            /* 0x20 */
	uint32 da_base_h;                            /* 0x24 */
	uint32 sa_base;                              /* 0x28 */
	uint32 timer;                                /* 0x2C */
	uint32 slot_cmd[M2M_UCLS_SLOT_MAX];          /* 0x30, 0x34 */
	uint32 slot_sts[M2M_UCLS_SLOT_MAX];          /* 0x38, 0x3C */
} m2m_ucls_eng_regs_t;

/* UCLS_CFG */
#define UCLS_CFG_MODULE_EN_SHIFT		0
#define UCLS_CFG_MODULE_EN_MASK \
	(0x1 << UCLS_CFG_MODULE_EN_SHIFT)
#define UCLS_CFG_STOP_CONDITION_SHIFT		8
#define UCLS_CFG_STOP_CONDITION_MASK \
	(0x3 << UCLS_CFG_STOP_CONDITION_SHIFT)
/* UCLS_CTL */
#define UCLS_CTL_SLOT_BUFSIZE_MASK		(0xffff)
/* UCLS_DEPTH */
#define UCLS_DEPTH_MASK				(0xffff)
/* UCLS_WRIDX */
#define UCLS_WRIDX_MASK				(0xffff)
/* UCLS_RDIDX */
#define UCLS_RDIDX_MASK				(0xffff)
/* UCLS_DMA_TEMPLAGE */
#define UCLS_DMA_TEMPLATE_SA_NOTPCIE_SHIFT	0
#define UCLS_DMA_TEMPLATE_SA_NOTPCIE_MASK \
	(0x1 << UCLS_DMA_TEMPLATE_SA_NOTPCIE_SHIFT)
#define UCLS_DMA_TEMPLATE_SA_COHERENT_SHIFT	1
#define UCLS_DMA_TEMPLATE_SA_COHERENT_MASK \
	(0x1 << UCLS_DMA_TEMPLATE_SA_COHERENT_SHIFT)
#define UCLS_DMA_TEMPLATE_SA_ADDREXT_SHIFT	2
#define UCLS_DMA_TEMPLATE_SA_ADDREXT_MASK \
	(0x3 << UCLS_DMA_TEMPLATE_SA_ADDREXT_SHIFT)
#define UCLS_DMA_TEMPLATE_DA_NOTPCIE_SHIFT	4
#define UCLS_DMA_TEMPLATE_DA_NOTPCIE_MASK \
	(0x1 << UCLS_DMA_TEMPLATE_DA_NOTPCIE_SHIFT)
#define UCLS_DMA_TEMPLATE_DA_COHERENT_SHIFT	5
#define UCLS_DMA_TEMPLATE_DA_COHERENT_MASK \
	(0x1 << UCLS_DMA_TEMPLATE_DA_COHERENT_SHIFT)
#define UCLS_DMA_TEMPLATE_DA_ADDREXT_SHIFT	6
#define UCLS_DMA_TEMPLATE_DA_ADDREXT_MASK \
	(0x3 << UCLS_DMA_TEMPLATE_DA_ADDREXT_SHIFT)
#define UCLS_DMA_TEMPLATE_DA_WCPD_SHIFT		8
#define UCLS_DMA_TEMPLATE_DA_WCPD_MASK \
	(0x1 << UCLS_DMA_TEMPLATE_DA_WCPD_SHIFT)
/* UCLS_TIMER */
#define UCLS_TIMER_LAZYCNT_SHIFT		0
#define UCLS_TIMER_LAZYCNT_MASK			(0xff)
#define UCLS_TIMER_LAZYTIMER_SHIFT		8
#define UCLS_TIMER_LAZYTIMER_MASK \
	(0xffffff << UCLS_TIMER_LAZYTIMER_SHIFT)

/* UCLS_PING_SLOT_CMD/UCLS_PONG_SLOT_CMD/UCLS_PING_SLOT_STS/UCLS_PONG_SLOT_STS */
#define UCLS_SLOT_LEN_SHIFT			0
#define UCLS_SLOT_LEN_MASK			(0xffff)
#define UCLS_SLOT_TRANS_ID_SHIFT		16
#define UCLS_SLOT_TRANS_ID_MASK \
	(0xfff << UCLS_SLOT_TRANS_ID_SHIFT)
#define UCLS_SLOT_FLAG_SHIFT			28
#define UCLS_SLOT_FLAG_MASK \
	(0x7 << UCLS_SLOT_FLAG_SHIFT)

/* UCLS_PING_SLOT_STS/UCLS_PONG_SLOT_STS */
#define UCLS_SLOT_STS_BUSY_SHIFT		31
#define UCLS_SLOT_STS_BUSY_MASK \
	(0x1 << UCLS_SLOT_STS_BUSY_SHIFT)

/* UMAC2LMAC (TAB) Registers */
typedef struct m2m_tab_eng_regs {
	uint32 cfg;                                  /* 0x0 */
	uint32 size;                                 /* 0x4 */
	uint32 lazycnt;                              /* 0x8 */
	uint32 sts;                                  /* 0xC */
	uint32 sa_base_l;                            /* 0x10 */
	uint32 sa_base_h;                            /* 0x14 */
	uint32 depth;                                /* 0x18 */
	uint32 dma_template;                         /* 0x1C */
	uint32 da_base_l;                            /* 0x20 */
	uint32 da_base_h;                            /* 0x24 */
	uint32 wridx;                                /* 0x28 */
	uint32 rdidx;                                /* 0x2C */
	uint32 PAD[2];                               /* 0x30, 0x34 */
	uint32 slot_sts;                             /* 0x38 */
	uint32 debug;                                /* 0x3C */
} m2m_tab_eng_regs_t;

/* Ranging CSI Definitions */
#define M2M_RNGCSI_SLOT_MAX	2

/* Ranging CSI Registers */
typedef struct m2m_rngcsi_eng_regs {
	uint32 cfg;                                  /* 0x0 */
	uint32 size;                                 /* 0x4 */
	uint32 lazycnt;                              /* 0x8 */
	uint32 sts;                                  /* 0xC */
	uint32 sa_base_l;                            /* 0x10 */
	uint32 sa_base_h;                            /* 0x14 */
	uint32 depth;                                /* 0x18 */
	uint32 dma_template;                         /* 0x1C */
	uint32 da_base_l;                            /* 0x20 */
	uint32 da_base_h;                            /* 0x24 */
	uint32 wridx;                                /* 0x28 */
	uint32 rdidx;                                /* 0x2C */
	uint32 offset;                               /* 0x30 */
	uint32 debug;                                /* 0x34 */
	uint32 PAD[2];                               /* 0x38, 0x3C */
	uint32 slot_ctrl[M2M_RNGCSI_SLOT_MAX];       /* 0x40, 0x44 */
	uint32 slot_sts[M2M_RNGCSI_SLOT_MAX];        /* 0x48, 0x4C */
} m2m_rngcsi_eng_regs_t;

/* M2MCORE registers */
typedef volatile struct m2m_core_regs {
	uint32 control;                              /* 0x0 */
	uint32 capabilities;                         /* 0x4 */
	uint32 intcontrol;                           /* 0x8 */
	uint32 PAD[5];
	uint32 intstatus;                            /* 0x20 */
	uint32 PAD[3];
	m2m_int_regs_t int_regs[M2M_CORE_CHANNELS];  /* 0x30 - 0x6c */
	uint32 PAD[36];
	uint32 intrcvlazy[M2M_CORE_CHANNELS];        /* 0x100 - 0x11c */
	uint32 PAD[48];
	uint32 clockctlstatus;                       /* 0x1e0 */
	uint32 workaround;                           /* 0x1e4 */
	uint32 powercontrol;                         /* 0x1e8 */
	uint32 PAD[5];
	m2m_eng_regs_t eng_regs[M2M_CORE_CHANNELS];  /* 0x200 - 0x3fc */
	uint32 PAD[256];
	m2m_status_eng_regs_t txs_regs;              /* 0x800 - 0x854 */
	uint32 PAD[42];
	m2m_status_eng_regs_t phyrxs_regs;           /* 0x900 - 0x954 */
	uint32 PAD[106];
	m2m_cpudbg_eng_regs_t cpudbg_regs;           /* 0xb00 - 0xb2c */
	uint32 PAD[180];
	m2m_ucls_eng_regs_t ucls_regs;               /* 0xe00 - 0xe3c */
	uint32 PAD[48];
	m2m_tab_eng_regs_t tab_regs;                 /* 0xf00 - 0xf3c */
	uint32 PAD[16];
	m2m_rngcsi_eng_regs_t rngcsi_regs;           /* 0xf80 - 0xfcc */
} m2m_core_regs_t;

/** m2m_core_regs_t::capabilities */
#define M2M_CORE_CAPABILITIES_CHANNELCNT_SHIFT		0
#define M2M_CORE_CAPABILITIES_CHANNELCNT_NBITS		4
#define M2M_CORE_CAPABILITIES_CHANNELCNT_MASK		BCM_MASK(M2M_CORE_CAPABILITIES_CHANNELCNT)

#define M2M_CORE_CAPABILITIES_MAXBURSTLEN_SHIFT		4
#define M2M_CORE_CAPABILITIES_MAXBURSTLEN_NBITS		3
#define M2M_CORE_CAPABILITIES_MAXBURSTLEN_MASK		BCM_MASK(M2M_CORE_CAPABILITIES_MAXBURSTLEN)

#define M2M_CORE_CAPABILITIES_MAXREADSOUTSTANDING_SHIFT 7
#define M2M_CORE_CAPABILITIES_MAXREADSOUTSTANDING_NBITS 3
#define M2M_CORE_CAPABILITIES_MAXREADSOUTSTANDING_MASK		\
	BCM_MASK(M2M_CORE_CAPABILITIES_MAXREADSOUTSTANDING)

#define M2M_CORE_CAPABILITIES_SM2MCNT_SHIFT		10
#define M2M_CORE_CAPABILITIES_SM2MCNT_NBITS		4
#define M2M_CORE_CAPABILITIES_SM2MCNT_MASK		BCM_MASK(M2M_CORE_CAPABILITIES_SM2MCNT)

/** m2m_core_regs_t::intcontrol */
#define M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_GE134_SHIFT	20 /**< CPU DBG W_IDX update */
#define M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_GE134_NBITS	1
#define M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_GE134_MASK	\
	BCM_MASK(M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_GE134)

#define M2M_CORE_INTCONTROL_TXSMBOXINT_INTMASK_SHIFT		25 /**< TxStatus MBOX int */
#define M2M_CORE_INTCONTROL_TXSMBOXINT_INTMASK_NBITS		1
#define M2M_CORE_INTCONTROL_TXSMBOXINT_INTMASK_MASK		\
	BCM_MASK(M2M_CORE_INTCONTROL_TXSMBOXINT_INTMASK)

#define M2M_CORE_INTCONTROL_PHYRXSMBOXINT_INTMASK_SHIFT		26 /**< PhyRxStatus MBOX int */
#define M2M_CORE_INTCONTROL_PHYRXSMBOXINT_INTMASK_NBITS		1
#define M2M_CORE_INTCONTROL_PHYRXSMBOXINT_INTMASK_MASK		\
	BCM_MASK(M2M_CORE_INTCONTROL_PHYRXSMBOXINT_INTMASK)

#define M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_SHIFT	29 /**< CPU DBG W_IDX update */
#define M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_NBITS	1
#define M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK_MASK	\
	BCM_MASK(M2M1_CORE_INTCONTROL_CPUDBGWRINDUPD_INTMASK)

#define M2M_CORE_INTCONTROL_TXSWRINDUPD_INTMASK_SHIFT		30 /**< TxStatus W_IDX update */
#define M2M_CORE_INTCONTROL_TXSWRINDUPD_INTMASK_NBITS		1
#define M2M_CORE_INTCONTROL_TXSWRINDUPD_INTMASK_MASK		\
	BCM_MASK(M2M_CORE_INTCONTROL_TXSWRINDUPD_INTMASK)

#define M2M_CORE_INTCONTROL_PHYRXSWRINDUPD_INTMASK_SHIFT	31 /**< PhyRxStatus W_IDX update */
#define M2M_CORE_INTCONTROL_PHYRXSWRINDUPD_INTMASK_NBITS	1
#define M2M_CORE_INTCONTROL_PHYRXSWRINDUPD_INTMASK_MASK		\
	BCM_MASK(M2M_CORE_INTCONTROL_PHYRXSWRINDUPD_INTMASK)

#define M2M1_CORE_INTCONTROL_RNGCSIWRINDUPD_INTMASK_SHIFT	21 /**< Ranging CSI W_IDX update */
#define M2M1_CORE_INTCONTROL_RNGCSIWRINDUPD_INTMASK_NBITS	1
#define M2M1_CORE_INTCONTROL_RNGCSIWRINDUPD_INTMASK_MASK		\
	BCM_MASK(M2M1_CORE_INTCONTROL_RNGCSIWRINDUPD_INTMASK)

/** m2m_int_regs_t intstatus, intmask for traditional and simple m2m channels */
/* Ch# <IntStatus,IntMask> DescErr (DE) */
#define M2M_CORE_CH_DE_NBITS        1
#define M2M_CORE_CH_DE_SHIFT        10
#define M2M_CORE_CH_DE_MASK         BCM_MASK(M2M_CORE_CH_DE)
/* Ch# <IntStatus,IntMask> DataErr (DA) */
#define M2M_CORE_CH_DA_NBITS        1
#define M2M_CORE_CH_DA_SHIFT        11
#define M2M_CORE_CH_DA_MASK         BCM_MASK(M2M_CORE_CH_DA)
/* Ch# <IntStatus,IntMask> DescProtoErr (DP) */
#define M2M_CORE_CH_DP_NBITS        1
#define M2M_CORE_CH_DP_SHIFT        12
#define M2M_CORE_CH_DP_MASK         BCM_MASK(M2M_CORE_CH_DP)
/* Ch# <IntStatus,IntMask> RcvDescUf (RU) */
#define M2M_CORE_CH_RU_NBITS        1
#define M2M_CORE_CH_RU_SHIFT        13
#define M2M_CORE_CH_RU_MASK         BCM_MASK(M2M_CORE_CH_RU)
/* Ch# <IntStatus,IntMask> RcvInt (RI) */
#define M2M_CORE_CH_RI_NBITS        1
#define M2M_CORE_CH_RI_SHIFT        16
#define M2M_CORE_CH_RI_MASK         BCM_MASK(M2M_CORE_CH_RI)
/* Ch# <IntStatus,IntMask> XmtInt (XI) */
#define M2M_CORE_CH_XI_NBITS        1
#define M2M_CORE_CH_XI_SHIFT        24
#define M2M_CORE_CH_XI_MASK         BCM_MASK(M2M_CORE_CH_XI)

/** Per M2M Channel: Interrupt Receive Lazy */
#define M2M_CORE_CH_INTRCVLAZY_TO_NBITS 24  /* TimeOut (TO) microsecs */
#define M2M_CORE_CH_INTRCVLAZY_TO_SHIFT 0
#define M2M_CORE_CH_INTRCVLAZY_TO_MASK  BCM_MASK(M2M_CORE_CH_INTRCVLAZY_TO)
#define M2M_CORE_CH_INTRCVLAZY_FC_NBITS 8   /* FrameCntr (FC) */
#define M2M_CORE_CH_INTRCVLAZY_FC_SHIFT 24
#define M2M_CORE_CH_INTRCVLAZY_FC_MASK  BCM_MASK(M2M_CORE_CH_INTRCVLAZY_FC)

#endif	/* _M2MDMA_CORE_H */
