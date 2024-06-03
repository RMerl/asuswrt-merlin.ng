/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday FTMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

#ifndef __FTMAC100_H
#define __FTMAC100_H

struct ftmac100 {
	unsigned int	isr;		/* 0x00 */
	unsigned int	imr;		/* 0x04 */
	unsigned int	mac_madr;	/* 0x08 */
	unsigned int	mac_ladr;	/* 0x0c */
	unsigned int	maht0;		/* 0x10 */
	unsigned int	maht1;		/* 0x14 */
	unsigned int	txpd;		/* 0x18 */
	unsigned int	rxpd;		/* 0x1c */
	unsigned int	txr_badr;	/* 0x20 */
	unsigned int	rxr_badr;	/* 0x24 */
	unsigned int	itc;		/* 0x28 */
	unsigned int	aptc;		/* 0x2c */
	unsigned int	dblac;		/* 0x30 */
	unsigned int	pad1[3];	/* 0x34 - 0x3c */
	unsigned int	pad2[16];	/* 0x40 - 0x7c */
	unsigned int	pad3[2];	/* 0x80 - 0x84 */
	unsigned int	maccr;		/* 0x88 */
	unsigned int	macsr;		/* 0x8c */
	unsigned int	phycr;		/* 0x90 */
	unsigned int	phywdata;	/* 0x94 */
	unsigned int	fcr;		/* 0x98 */
	unsigned int	bpr;		/* 0x9c */
	unsigned int	pad4[8];	/* 0xa0 - 0xbc */
	unsigned int	pad5;		/* 0xc0 */
	unsigned int	ts;		/* 0xc4 */
	unsigned int	dmafifos;	/* 0xc8 */
	unsigned int	tm;		/* 0xcc */
	unsigned int	pad6;		/* 0xd0 */
	unsigned int	tx_mcol_scol;	/* 0xd4 */
	unsigned int	rpf_aep;	/* 0xd8 */
	unsigned int	xm_pg;		/* 0xdc */
	unsigned int	runt_tlcc;	/* 0xe0 */
	unsigned int	crcer_ftl;	/* 0xe4 */
	unsigned int	rlc_rcc;	/* 0xe8 */
	unsigned int	broc;		/* 0xec */
	unsigned int	mulca;		/* 0xf0 */
	unsigned int	rp;		/* 0xf4 */
	unsigned int	xp;		/* 0xf8 */
};

/*
 * Interrupt status register & interrupt mask register
 */
#define FTMAC100_INT_RPKT_FINISH	(1 << 0)
#define FTMAC100_INT_NORXBUF		(1 << 1)
#define FTMAC100_INT_XPKT_FINISH	(1 << 2)
#define FTMAC100_INT_NOTXBUF		(1 << 3)
#define FTMAC100_INT_XPKT_OK		(1 << 4)
#define FTMAC100_INT_XPKT_LOST		(1 << 5)
#define FTMAC100_INT_RPKT_SAV		(1 << 6)
#define FTMAC100_INT_RPKT_LOST		(1 << 7)
#define FTMAC100_INT_AHB_ERR		(1 << 8)
#define FTMAC100_INT_PHYSTS_CHG		(1 << 9)

/*
 * Automatic polling timer control register
 */
#define FTMAC100_APTC_RXPOLL_CNT(x)	(((x) & 0xf) << 0)
#define FTMAC100_APTC_RXPOLL_TIME_SEL	(1 << 4)
#define FTMAC100_APTC_TXPOLL_CNT(x)	(((x) & 0xf) << 8)
#define FTMAC100_APTC_TXPOLL_TIME_SEL	(1 << 12)

/*
 * MAC control register
 */
#define FTMAC100_MACCR_XDMA_EN		(1 << 0)
#define FTMAC100_MACCR_RDMA_EN		(1 << 1)
#define FTMAC100_MACCR_SW_RST		(1 << 2)
#define FTMAC100_MACCR_LOOP_EN		(1 << 3)
#define FTMAC100_MACCR_CRC_DIS		(1 << 4)
#define FTMAC100_MACCR_XMT_EN		(1 << 5)
#define FTMAC100_MACCR_ENRX_IN_HALFTX	(1 << 6)
#define FTMAC100_MACCR_RCV_EN		(1 << 8)
#define FTMAC100_MACCR_HT_MULTI_EN	(1 << 9)
#define FTMAC100_MACCR_RX_RUNT		(1 << 10)
#define FTMAC100_MACCR_RX_FTL		(1 << 11)
#define FTMAC100_MACCR_RCV_ALL		(1 << 12)
#define FTMAC100_MACCR_CRC_APD		(1 << 14)
#define FTMAC100_MACCR_FULLDUP		(1 << 15)
#define FTMAC100_MACCR_RX_MULTIPKT	(1 << 16)
#define FTMAC100_MACCR_RX_BROADPKT	(1 << 17)

/*
 * Transmit descriptor, aligned to 16 bytes
 */
struct ftmac100_txdes {
	unsigned int	txdes0;
	unsigned int	txdes1;
	unsigned int	txdes2;	/* TXBUF_BADR */
	unsigned int	txdes3;	/* not used by HW */
} __attribute__ ((aligned(16)));

#define FTMAC100_TXDES0_TXPKT_LATECOL	(1 << 0)
#define FTMAC100_TXDES0_TXPKT_EXSCOL	(1 << 1)
#define FTMAC100_TXDES0_TXDMA_OWN	(1 << 31)

#define FTMAC100_TXDES1_TXBUF_SIZE(x)	((x) & 0x7ff)
#define FTMAC100_TXDES1_LTS		(1 << 27)
#define FTMAC100_TXDES1_FTS		(1 << 28)
#define FTMAC100_TXDES1_TX2FIC		(1 << 29)
#define FTMAC100_TXDES1_TXIC		(1 << 30)
#define FTMAC100_TXDES1_EDOTR		(1 << 31)

/*
 * Receive descriptor, aligned to 16 bytes
 */
struct ftmac100_rxdes {
	unsigned int	rxdes0;
	unsigned int	rxdes1;
	unsigned int	rxdes2;	/* RXBUF_BADR */
	unsigned int	rxdes3;	/* not used by HW */
} __attribute__ ((aligned(16)));

#define FTMAC100_RXDES0_RFL(des)	((des) & 0x7ff)
#define FTMAC100_RXDES0_MULTICAST	(1 << 16)
#define FTMAC100_RXDES0_BROADCAST	(1 << 17)
#define FTMAC100_RXDES0_RX_ERR		(1 << 18)
#define FTMAC100_RXDES0_CRC_ERR		(1 << 19)
#define FTMAC100_RXDES0_FTL		(1 << 20)
#define FTMAC100_RXDES0_RUNT		(1 << 21)
#define FTMAC100_RXDES0_RX_ODD_NB	(1 << 22)
#define FTMAC100_RXDES0_LRS		(1 << 28)
#define FTMAC100_RXDES0_FRS		(1 << 29)
#define FTMAC100_RXDES0_RXDMA_OWN	(1 << 31)

#define FTMAC100_RXDES1_RXBUF_SIZE(x)	((x) & 0x7ff)
#define FTMAC100_RXDES1_EDORR		(1 << 31)

#endif	/* __FTMAC100_H */
