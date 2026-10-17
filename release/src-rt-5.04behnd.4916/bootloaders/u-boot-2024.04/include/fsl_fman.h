/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MPC85xx Internal Memory Map
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_FMAN_H__
#define __FSL_FMAN_H__

#include <asm/types.h>

typedef struct fm_bmi_common {
	u32	fmbm_init;	/* BMI initialization */
	u32	fmbm_cfg1;	/* BMI configuration1 */
	u32	fmbm_cfg2;	/* BMI configuration2 */
	u32	res0[0x5];
	u32	fmbm_ievr;	/* interrupt event register */
	u32	fmbm_ier;	/* interrupt enable register */
	u32	fmbm_ifr;	/* interrupt force register */
	u32	res1[0x5];
	u32	fmbm_arb[0x8];	/* BMI arbitration */
	u32	res2[0x28];
	u32	fmbm_gde;	/* global debug enable */
	u32	fmbm_pp[0x3f];	/* BMI port parameters */
	u32	res3;
	u32	fmbm_pfs[0x3f];	/* BMI port FIFO size */
	u32	res4;
	u32	fmbm_ppid[0x3f];/* port partition ID */
} fm_bmi_common_t;

typedef struct fm_qmi_common {
	u32	fmqm_gc;	/* general configuration register */
	u32	res0;
	u32	fmqm_eie;	/* error interrupt event register */
	u32	fmqm_eien;	/* error interrupt enable register */
	u32	fmqm_eif;	/* error interrupt force register */
	u32	fmqm_ie;	/* interrupt event register */
	u32	fmqm_ien;	/* interrupt enable register */
	u32	fmqm_if;	/* interrupt force register */
	u32	fmqm_gs;	/* global status register */
	u32	fmqm_ts;	/* task status register */
	u32	fmqm_etfc;	/* enqueue total frame counter */
	u32	fmqm_dtfc;	/* dequeue total frame counter */
	u32	fmqm_dc0;	/* dequeue counter 0 */
	u32	fmqm_dc1;	/* dequeue counter 1 */
	u32	fmqm_dc2;	/* dequeue counter 2 */
	u32	fmqm_dc3;	/* dequeue counter 3 */
	u32	fmqm_dfnoc;	/* dequeue FQID not override counter */
	u32	fmqm_dfcc;	/* dequeue FQID from context counter */
	u32	fmqm_dffc;	/* dequeue FQID from FD counter */
	u32	fmqm_dcc;	/* dequeue confirm counter */
	u32	res1[0xc];
	u32	fmqm_dtrc;	/* debug trap configuration register */
	u32	fmqm_efddd;	/* enqueue frame descriptor dynamic debug */
	u32	res3[0x2];
	u32	res4[0xdc];	/* missing debug regs */
} fm_qmi_common_t;

typedef struct fm_bmi {
	u8	res[1024];
} fm_bmi_t;

typedef struct fm_qmi {
	u8	res[1024];
} fm_qmi_t;

struct fm_bmi_rx_port {
	u32 fmbm_rcfg;	/* Rx configuration */
	u32 fmbm_rst;	/* Rx status */
	u32 fmbm_rda;	/* Rx DMA attributes */
	u32 fmbm_rfp;	/* Rx FIFO parameters */
	u32 fmbm_rfed;	/* Rx frame end data */
	u32 fmbm_ricp;	/* Rx internal context parameters */
	u32 fmbm_rim;	/* Rx internal margins */
	u32 fmbm_rebm;	/* Rx external buffer margins */
	u32 fmbm_rfne;	/* Rx frame next engine */
	u32 fmbm_rfca;	/* Rx frame command attributes */
	u32 fmbm_rfpne;	/* Rx frame parser next engine */
	u32 fmbm_rpso;	/* Rx parse start offset */
	u32 fmbm_rpp;	/* Rx policer profile */
	u32 fmbm_rccb;	/* Rx coarse classification base */
	u32 res1[0x2];
	u32 fmbm_rprai[0x8];	/* Rx parse results array Initialization */
	u32 fmbm_rfqid;		/* Rx frame queue ID */
	u32 fmbm_refqid;	/* Rx error frame queue ID */
	u32 fmbm_rfsdm;		/* Rx frame status discard mask */
	u32 fmbm_rfsem;		/* Rx frame status error mask */
	u32 fmbm_rfene;		/* Rx frame enqueue next engine */
	u32 res2[0x23];
	u32 fmbm_ebmpi[0x8];	/* buffer manager pool information */
	u32 fmbm_acnt[0x8];	/* allocate counter */
	u32 res3[0x8];
	u32 fmbm_cgm[0x8];	/* congestion group map */
	u32 fmbm_mpd;		/* BMan pool depletion */
	u32 res4[0x1F];
	u32 fmbm_rstc;		/* Rx statistics counters */
	u32 fmbm_rfrc;		/* Rx frame counters */
	u32 fmbm_rfbc;		/* Rx bad frames counter */
	u32 fmbm_rlfc;		/* Rx large frames counter */
	u32 fmbm_rffc;		/* Rx filter frames counter */
	u32 fmbm_rfdc;		/* Rx frame discard counter */
	u32 fmbm_rfldec;	/* Rx frames list DMA error counter */
	u32 fmbm_rodc;		/* Rx out of buffers discard counter */
	u32 fmbm_rbdc;		/* Rx buffers deallocate counter */
	u32 res5[0x17];
	u32 fmbm_rpc;		/* Rx performance counters */
	u32 fmbm_rpcp;		/* Rx performance count parameters */
	u32 fmbm_rccn;		/* Rx cycle counter */
	u32 fmbm_rtuc;		/* Rx tasks utilization counter */
	u32 fmbm_rrquc;		/* Rx receive queue utilization counter */
	u32 fmbm_rduc;		/* Rx DMA utilization counter */
	u32 fmbm_rfuc;		/* Rx FIFO utilization counter */
	u32 fmbm_rpac;		/* Rx pause activation counter */
	u32 res6[0x18];
	u32 fmbm_rdbg;		/* Rx debug configuration */
};

/* FMBM_RCFG - Rx configuration */
#define FMBM_RCFG_EN		0x80000000 /* port is enabled to receive data */
#define FMBM_RCFG_FDOVR		0x02000000 /* frame discard override */
#define FMBM_RCFG_IM		0x01000000 /* independent mode */

/* FMBM_RST - Rx status */
#define FMBM_RST_BSY		0x80000000 /* Rx port is busy */

/* FMBM_RFCA - Rx frame command attributes */
#define FMBM_RFCA_ORDER		0x80000000
#define FMBM_RFCA_MR_MASK	0x003f0000
#define FMBM_RFCA_MR(x)		((x << 16) & FMBM_RFCA_MR_MASK)

/* FMBM_RSTC - Rx statistics */
#define FMBM_RSTC_EN		0x80000000 /* statistics counters enable */

struct fm_bmi_tx_port {
	u32 fmbm_tcfg;	/* Tx configuration */
	u32 fmbm_tst;	/* Tx status */
	u32 fmbm_tda;	/* Tx DMA attributes */
	u32 fmbm_tfp;	/* Tx FIFO parameters */
	u32 fmbm_tfed;	/* Tx frame end data */
	u32 fmbm_ticp;	/* Tx internal context parameters */
	u32 fmbm_tfne;	/* Tx frame next engine */
	u32 fmbm_tfca;	/* Tx frame command attributes */
	u32 fmbm_tcfqid;/* Tx confirmation frame queue ID */
	u32 fmbm_tfeqid;/* Tx error frame queue ID */
	u32 fmbm_tfene;	/* Tx frame enqueue next engine */
	u32 fmbm_trlmts;/* Tx rate limiter scale */
	u32 fmbm_trlmt;	/* Tx rate limiter */
	u32 res0[0x73];
	u32 fmbm_tstc;	/* Tx statistics counters */
	u32 fmbm_tfrc;	/* Tx frame counter */
	u32 fmbm_tfdc;	/* Tx frames discard counter */
	u32 fmbm_tfledc;/* Tx frame length error discard counter */
	u32 fmbm_tfufdc;/* Tx frame unsupported format discard counter */
	u32 fmbm_tbdc;	/* Tx buffers deallocate counter */
	u32 res1[0x1a];
	u32 fmbm_tpc;	/* Tx performance counters */
	u32 fmbm_tpcp;	/* Tx performance count parameters */
	u32 fmbm_tccn;	/* Tx cycle counter */
	u32 fmbm_ttuc;	/* Tx tasks utilization counter */
	u32 fmbm_ttcquc;/* Tx transmit confirm queue utilization counter */
	u32 fmbm_tduc;	/* Tx DMA utilization counter */
	u32 fmbm_tfuc;	/* Tx FIFO utilization counter */
	u32 res2[0x19];
	u32 fmbm_tdcfg;	/* Tx debug configuration */
};

/* FMBM_TCFG - Tx configuration */
#define FMBM_TCFG_EN	0x80000000 /* port is enabled to transmit data */
#define FMBM_TCFG_IM	0x01000000 /* independent mode enable */

/* FMBM_TST - Tx status */
#define FMBM_TST_BSY		0x80000000 /* Tx port is busy */

/* FMBM_TFCA - Tx frame command attributes */
#define FMBM_TFCA_ORDER		0x80000000
#define FMBM_TFCA_MR_MASK	0x003f0000
#define FMBM_TFCA_MR(x)		((x << 16) & FMBM_TFCA_MR_MASK)

/* FMBM_TSTC - Tx statistics counters */
#define FMBM_TSTC_EN		0x80000000

/* FMBM_INIT - BMI initialization register */
#define FMBM_INIT_START		0x80000000 /* init internal buffers */

/* FMBM_CFG1 - BMI configuration 1 */
#define FMBM_CFG1_FBPS_MASK	0x03ff0000 /* Free buffer pool size */
#define FMBM_CFG1_FBPS_SHIFT	16
#define FMBM_CFG1_FBPO_MASK	0x000003ff /* Free buffer pool offset */

/* FMBM_IEVR - interrupt event */
#define FMBM_IEVR_PEC		0x80000000 /* pipeline table ECC err detected */
#define FMBM_IEVR_LEC		0x40000000 /* linked list RAM ECC error */
#define FMBM_IEVR_SEC		0x20000000 /* statistics count RAM ECC error */
#define FMBM_IEVR_CLEAR_ALL	(FMBM_IEVR_PEC | FMBM_IEVR_LEC | FMBM_IEVR_SEC)

/* FMBM_IER - interrupt enable */
#define FMBM_IER_PECE		0x80000000 /* PEC interrupt enable */
#define FMBM_IER_LECE		0x40000000 /* LEC interrupt enable */
#define FMBM_IER_SECE		0x20000000 /* SEC interrupt enable */

#define FMBM_IER_DISABLE_ALL	0x00000000

/* FMBM_PP - BMI Port Parameters */
#define FMBM_PP_MXT_MASK	0x3f000000 /* Max # tasks */
#define FMBM_PP_MXT(x)		(((x-1) << 24) & FMBM_PP_MXT_MASK)
#define FMBM_PP_MXD_MASK	0x00000f00 /* Max DMA */
#define FMBM_PP_MXD(x)		(((x-1) << 8) & FMBM_PP_MXD_MASK)

/* FMBM_PFS - BMI Port FIFO Size */
#define FMBM_PFS_IFSZ_MASK	0x000003ff /* Internal Fifo Size */
#define FMBM_PFS_IFSZ(x)	(x & FMBM_PFS_IFSZ_MASK)

/* FMQM_GC - global configuration */
#define FMQM_GC_ENQ_EN		0x80000000 /* enqueue enable */
#define FMQM_GC_DEQ_EN		0x40000000 /* dequeue enable */
#define FMQM_GC_STEN		0x10000000 /* enable global stat counters */
#define FMQM_GC_ENQ_THR_MASK	0x00003f00 /* max number of enqueue Tnum */
#define FMQM_GC_ENQ(x)		((x << 8) &  FMQM_GC_ENQ_THR_MAS)
#define FMQM_GC_DEQ_THR_MASK	0x0000003f /* max number of dequeue Tnum */
#define FMQM_GC_DEQ(x)		(x & FMQM_GC_DEQ_THR_MASK)

/* FMQM_EIE - error interrupt event register */
#define FMQM_EIE_DEE		0x80000000 /* double-bit ECC error */
#define FMQM_EIE_DFUPE		0x40000000 /* dequeue from unknown PortID */
#define FMQM_EIE_CLEAR_ALL	(FMQM_EIE_DEE | FMQM_EIE_DFUPE)

/* FMQM_EIEN - error interrupt enable register */
#define FMQM_EIEN_DEEN		0x80000000 /* double-bit ECC error */
#define FMQM_EIEN_DFUPEN	0x40000000 /* dequeue from unknown PortID */
#define FMQM_EIEN_DISABLE_ALL	0x00000000

/* FMQM_IE - interrupt event register */
#define FMQM_IE_SEE		0x80000000 /* single-bit ECC error detected */
#define FMQM_IE_CLEAR_ALL	FMQM_IE_SEE

/* FMQM_IEN - interrupt enable register */
#define FMQM_IEN_SEE		0x80000000 /* single-bit ECC err IRQ enable */
#define FMQM_IEN_DISABLE_ALL	0x00000000

/* NIA - next invoked action */
#define NIA_ENG_RISC		0x00000000
#define NIA_ENG_MASK		0x007c0000

/* action code */
#define NIA_RISC_AC_CC		0x00000006
#define NIA_RISC_AC_IM_TX	0x00000008 /* independent mode Tx */
#define NIA_RISC_AC_IM_RX	0x0000000a /* independent mode Rx */
#define NIA_RISC_AC_HC		0x0000000c

typedef struct fm_parser {
	u8	res[1024];
} fm_parser_t;

typedef struct fm_policer {
	u8	res[4*1024];
} fm_policer_t;

typedef struct fm_keygen {
	u8	res[4*1024];
} fm_keygen_t;

typedef struct fm_dma {
	u32	fmdmsr;		/* status register */
	u32	fmdmmr;		/* mode register */
	u32	fmdmtr;		/* bus threshold register */
	u32	fmdmhy;		/* bus hysteresis register */
	u32	fmdmsetr;	/* SOS emergency threshold register */
	u32	fmdmtah;	/* transfer bus address high register */
	u32	fmdmtal;	/* transfer bus address low register */
	u32	fmdmtcid;	/* transfer bus communication ID register */
	u32	fmdmra;		/* DMA bus internal ram address register */
	u32	fmdmrd;		/* DMA bus internal ram data register */
	u32	res0[0xb];
	u32	fmdmdcr;	/* debug counter */
	u32	fmdmemsr;	/* emrgency smoother register */
	u32	res1;
	u32	fmdmplr[32];	/* FM DMA PID-LIODN # register */
	u32	res[0x3c8];
} fm_dma_t;

/* FMDMSR - Fman DMA status register */
#define FMDMSR_CMDQNE		0x10000000 /* command queue not empty */
#define FMDMSR_BER		0x08000000 /* bus err event occurred on bus */
#define FMDMSR_RDB_ECC		0x04000000 /* read buffer ECC error */
#define FMDMSR_WRB_SECC		0x02000000 /* write buf ECC err sys side */
#define FMDMSR_WRB_FECC		0x01000000 /* write buf ECC err Fman side */
#define FMDMSR_DPEXT_SECC	0x00800000 /* DP external ECC err sys side */
#define FMDMSR_DPEXT_FECC	0x00400000 /* DP external ECC err Fman side */
#define FMDMSR_DPDAT_SECC	0x00200000 /* DP data ECC err on sys side */
#define FMDMSR_DPDAT_FECC	0x00100000 /* DP data ECC err on Fman side */
#define FMDMSR_SPDAT_FECC	0x00080000 /* SP data ECC error Fman side */

#define FMDMSR_CLEAR_ALL	(FMDMSR_BER | FMDMSR_RDB_ECC \
				| FMDMSR_WRB_SECC | FMDMSR_WRB_FECC \
				| FMDMSR_DPEXT_SECC | FMDMSR_DPEXT_FECC \
				| FMDMSR_DPDAT_SECC | FMDMSR_DPDAT_FECC \
				| FMDMSR_SPDAT_FECC)

/* FMDMMR - FMan DMA mode register */
#define FMDMMR_SBER		0x10000000 /* stop the DMA if a bus error */

typedef struct fm_fpm {
	u32	fpmtnc;		/* TNUM control */
	u32	fpmprc;		/* Port_ID control */
	u32	res0;
	u32	fpmflc;		/* flush control */
	u32	fpmdis1;	/* dispatch thresholds1 */
	u32	fpmdis2;	/* dispatch thresholds2 */
	u32	fmepi;		/* error pending interrupts */
	u32	fmrie;		/* rams interrupt enable */
	u32	fpmfcevent[0x4];/* FMan controller event 0-3 */
	u32	res1[0x4];
	u32	fpmfcmask[0x4];	/* FMan controller mask 0-3 */
	u32	res2[0x4];
	u32	fpmtsc1;	/* timestamp control1 */
	u32	fpmtsc2;	/* timestamp control2 */
	u32	fpmtsp;		/* time stamp */
	u32	fpmtsf;		/* time stamp fraction */
	u32	fpmrcr;		/* rams control and event */
	u32	res3[0x3];
	u32	fpmdrd[0x4];	/* data_ram data 0-3 */
	u32	res4[0xc];
	u32	fpmdra;		/* data ram access */
	u32	fm_ip_rev_1;	/* IP block revision 1 */
	u32	fm_ip_rev_2;	/* IP block revision 2 */
	u32	fmrstc;		/* reset command */
	u32	fmcld;		/* classifier debug control */
	u32	fmnpi;		/* normal pending interrupts */
	u32	res5;
	u32	fmfpee;		/* event and enable */
	u32	fpmcev[0x4];	/* CPU event 0-3 */
	u32	res6[0x4];
	u32	fmfp_ps[0x40];	/* port status */
	u32	res7[0x260];
	u32	fpmts[0x80];	/* task status */
	u32	res8[0xa0];
} fm_fpm_t;

/* FMFP_PRC - FPM Port_ID Control Register */
#define FMFPPRC_PORTID_MASK	0x3f000000
#define FMFPPRC_PORTID_SHIFT	24
#define FMFPPRC_ORA_SHIFT	16
#define FMFPPRC_RISC1		0x00000001
#define FMFPPRC_RISC2		0x00000002
#define FMFPPRC_RISC_ALL	(FMFPPRC_RISC1 | FMFPPRC_RSIC2)

/* FPM Flush Control Register */
#define FMFP_FLC_DISP_LIM_NONE	0x00000000 /* no dispatch limitation */

/* FMFP_EE - FPM event and enable register */
#define FMFPEE_DECC		0x80000000 /* double ECC err on FPM ram */
#define FMFPEE_STL		0x40000000 /* stall of task ... */
#define FMFPEE_SECC		0x20000000 /* single ECC error */
#define FMFPEE_RFM		0x00010000 /* release FMan */
#define FMFPEE_DECC_EN		0x00008000 /* double ECC interrupt enable */
#define FMFPEE_STL_EN		0x00004000 /* stall of task interrupt enable */
#define FMFPEE_SECC_EN		0x00002000 /* single ECC err interrupt enable */
#define FMFPEE_EHM		0x00000008 /* external halt enable */
#define FMFPEE_UEC		0x00000004 /* FMan is not halted */
#define FMFPEE_CER		0x00000002 /* only errornous task stalled */
#define FMFPEE_DER		0x00000001 /* DMA error is just reported */

#define FMFPEE_CLEAR_EVENT	(FMFPEE_DECC | FMFPEE_STL | FMFPEE_SECC | \
				 FMFPEE_EHM | FMFPEE_UEC | FMFPEE_CER | \
				 FMFPEE_DER | FMFPEE_RFM)

/* FMFP_RCR - FMan Rams Control and Event */
#define FMFP_RCR_MDEC		0x00008000 /* double ECC error in muram */
#define FMFP_RCR_IDEC		0x00004000 /* double ECC error in iram */

typedef struct fm_imem {
	u32	iadd;		/* instruction address register */
	u32	idata;		/* instruction data register */
	u32	itcfg;		/* timing config register */
	u32	iready;		/* ready register */
	u8	res[0xff0];
} fm_imem_t;
#define IRAM_IADD_AIE		0x80000000 /* address auto increase enable */
#define IRAM_READY		0x80000000 /* ready to use */

typedef struct fm_soft_parser {
	u8	res[4*1024];
} fm_soft_parser_t;

typedef struct fm_dtesc {
	u8	res[4*1024];
} fm_dtsec_t;

typedef struct fm_mdio {
	u8	res0[0x120];
	u32	miimcfg;	/* MII management configuration reg */
	u32	miimcom;	/* MII management command reg */
	u32	miimadd;	/* MII management address reg */
	u32	miimcon;	/* MII management control reg */
	u32	miimstat;	/* MII management status reg  */
	u32	miimind;	/* MII management indication reg */
	u8	res1[0x1000 - 0x138];
} fm_mdio_t;

typedef struct fm_10gec {
	u8	res[4*1024];
} fm_10gec_t;

typedef struct fm_10gec_mdio {
	u8	res[4*1024];
} fm_10gec_mdio_t;

typedef struct fm_memac {
	u8	res[4*1024];
} fm_memac_t;

typedef struct fm_memac_mdio {
	u8	res[4*1024];
} fm_memac_mdio_t;

typedef struct fm_1588 {
	u8	res[4*1024];
} fm_1588_t;

typedef struct ccsr_fman {
	u8			muram[0x80000];
	fm_bmi_common_t		fm_bmi_common;
	fm_qmi_common_t		fm_qmi_common;
	u8			res0[2048];
	struct {
		fm_bmi_t	fm_bmi;
		fm_qmi_t	fm_qmi;
		fm_parser_t	fm_parser;
		u8		res[1024];
	} port[63];
	fm_policer_t		fm_policer;
	fm_keygen_t		fm_keygen;
	fm_dma_t		fm_dma;
	fm_fpm_t		fm_fpm;
	fm_imem_t		fm_imem;
	u8			res1[8*1024];
	fm_soft_parser_t	fm_soft_parser;
	u8			res2[96*1024];
#ifdef CONFIG_SYS_FMAN_V3
	struct {
		fm_memac_t		fm_memac;
		fm_memac_mdio_t		fm_memac_mdio;
	} memac[10];
	u8			res4[32*1024];
	fm_memac_mdio_t		fm_dedicated_mdio[2];
#else
	struct {
		fm_dtsec_t	fm_dtesc;
		fm_mdio_t	fm_mdio;
	} mac_1g[8];		/* support up to 8 1g controllers */
	struct {
		fm_10gec_t		fm_10gec;
		fm_10gec_mdio_t		fm_10gec_mdio;
	} mac_10g[1];
	u8			res4[48*1024];
#endif
	fm_1588_t		fm_1588;
	u8			res5[4*1024];
} ccsr_fman_t;

void fdt_fixup_fman_firmware(void *blob);
#endif /*__FSL_FMAN_H__*/
