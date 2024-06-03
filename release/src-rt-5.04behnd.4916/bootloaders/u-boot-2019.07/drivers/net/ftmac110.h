/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday 10/100Mbps Ethernet Controller
 *
 * (C) Copyright 2013 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 */

#ifndef _FTMAC110_H
#define _FTMAC110_H

struct ftmac110_regs {
	uint32_t isr;    /* 0x00: Interrups Status Register */
	uint32_t imr;    /* 0x04: Interrupt Mask Register */
	uint32_t mac[2]; /* 0x08: MAC Address */
	uint32_t mht[2]; /* 0x10: Multicast Hash Table Register */
	uint32_t txpd;   /* 0x18: Tx Poll Demand Register */
	uint32_t rxpd;   /* 0x1c: Rx Poll Demand Register */
	uint32_t txba;   /* 0x20: Tx Ring Base Address Register */
	uint32_t rxba;   /* 0x24: Rx Ring Base Address Register */
	uint32_t itc;    /* 0x28: Interrupt Timer Control Register */
	uint32_t aptc;   /* 0x2C: Automatic Polling Timer Control Register */
	uint32_t dblac;  /* 0x30: DMA Burst Length&Arbitration Control */
	uint32_t revr;   /* 0x34: Revision Register */
	uint32_t fear;   /* 0x38: Feature Register */
	uint32_t rsvd[19];
	uint32_t maccr;  /* 0x88: MAC Control Register */
	uint32_t macsr;  /* 0x8C: MAC Status Register */
	uint32_t phycr;  /* 0x90: PHY Control Register */
	uint32_t phydr;  /* 0x94: PHY Data Register */
	uint32_t fcr;    /* 0x98: Flow Control Register */
	uint32_t bpr;    /* 0x9C: Back Pressure Register */
};

/*
 * Interrupt status/mask register(ISR/IMR) bits
 */
#define ISR_ALL          0x3ff
#define ISR_PHYSTCHG     (1 << 9) /* phy status change */
#define ISR_AHBERR       (1 << 8) /* bus error */
#define ISR_RXLOST       (1 << 7) /* rx lost */
#define ISR_RXFIFO       (1 << 6) /* rx to fifo */
#define ISR_TXLOST       (1 << 5) /* tx lost */
#define ISR_TXOK         (1 << 4) /* tx to ethernet */
#define ISR_NOTXBUF      (1 << 3) /* out of tx buffer */
#define ISR_TXFIFO       (1 << 2) /* tx to fifo */
#define ISR_NORXBUF      (1 << 1) /* out of rx buffer */
#define ISR_RXOK         (1 << 0) /* rx to buffer */

/*
 * MACCR control bits
 */
#define MACCR_100M       (1 << 18) /* 100Mbps mode */
#define MACCR_RXBCST     (1 << 17) /* rx broadcast packet */
#define MACCR_RXMCST     (1 << 16) /* rx multicast packet */
#define MACCR_FD         (1 << 15) /* full duplex */
#define MACCR_CRCAPD     (1 << 14) /* tx crc append */
#define MACCR_RXALL      (1 << 12) /* rx all packets */
#define MACCR_RXFTL      (1 << 11) /* rx packet even it's > 1518 byte */
#define MACCR_RXRUNT     (1 << 10) /* rx packet even it's < 64 byte */
#define MACCR_RXMCSTHT   (1 << 9)  /* rx multicast hash table */
#define MACCR_RXEN       (1 << 8)  /* rx enable */
#define MACCR_RXINHDTX   (1 << 6)  /* rx in half duplex tx */
#define MACCR_TXEN       (1 << 5)  /* tx enable */
#define MACCR_CRCDIS     (1 << 4)  /* tx packet even it's crc error */
#define MACCR_LOOPBACK   (1 << 3)  /* loop-back */
#define MACCR_RESET      (1 << 2)  /* reset */
#define MACCR_RXDMAEN    (1 << 1)  /* rx dma enable */
#define MACCR_TXDMAEN    (1 << 0)  /* tx dma enable */

/*
 * PHYCR control bits
 */
#define PHYCR_READ       (1 << 26)
#define PHYCR_WRITE      (1 << 27)
#define PHYCR_REG_SHIFT  21
#define PHYCR_ADDR_SHIFT 16

/*
 * ITC control bits
 */

/* Tx Cycle Length */
#define ITC_TX_CYCLONG   (1 << 15) /* 100Mbps=81.92us; 10Mbps=819.2us */
#define ITC_TX_CYCNORM   (0 << 15) /* 100Mbps=5.12us;  10Mbps=51.2us */
/* Tx Threshold: Aggregate n interrupts as 1 interrupt */
#define ITC_TX_THR(n)    (((n) & 0x7) << 12)
/* Tx Interrupt Timeout = n * Tx Cycle */
#define ITC_TX_ITMO(n)   (((n) & 0xf) << 8)
/* Rx Cycle Length */
#define ITC_RX_CYCLONG   (1 << 7)  /* 100Mbps=81.92us; 10Mbps=819.2us */
#define ITC_RX_CYCNORM   (0 << 7)  /* 100Mbps=5.12us;  10Mbps=51.2us */
/* Rx Threshold: Aggregate n interrupts as 1 interrupt */
#define ITC_RX_THR(n)    (((n) & 0x7) << 4)
/* Rx Interrupt Timeout = n * Rx Cycle */
#define ITC_RX_ITMO(n)   (((n) & 0xf) << 0)

#define ITC_DEFAULT \
	(ITC_TX_THR(1) | ITC_TX_ITMO(0) | ITC_RX_THR(1) | ITC_RX_ITMO(0))

/*
 * APTC contrl bits
 */

/* Tx Cycle Length */
#define APTC_TX_CYCLONG  (1 << 12) /* 100Mbps=81.92us; 10Mbps=819.2us */
#define APTC_TX_CYCNORM  (0 << 12) /* 100Mbps=5.12us;  10Mbps=51.2us */
/* Tx Poll Timeout = n * Tx Cycle, 0=No auto polling */
#define APTC_TX_PTMO(n)  (((n) & 0xf) << 8)
/* Rx Cycle Length */
#define APTC_RX_CYCLONG  (1 << 4)  /* 100Mbps=81.92us; 10Mbps=819.2us */
#define APTC_RX_CYCNORM  (0 << 4)  /* 100Mbps=5.12us;  10Mbps=51.2us */
/* Rx Poll Timeout = n * Rx Cycle, 0=No auto polling */
#define APTC_RX_PTMO(n)  (((n) & 0xf) << 0)

#define APTC_DEFAULT     (APTC_TX_PTMO(0) | APTC_RX_PTMO(1))

/*
 * DBLAC contrl bits
 */
#define DBLAC_BURST_MAX_ANY  (0 << 14) /* un-limited */
#define DBLAC_BURST_MAX_32X4 (2 << 14) /* max = 32 x 4 bytes */
#define DBLAC_BURST_MAX_64X4 (3 << 14) /* max = 64 x 4 bytes */
#define DBLAC_RXTHR_EN       (1 << 9)  /* enable rx threshold arbitration */
#define DBLAC_RXTHR_HIGH(n)  (((n) & 0x7) << 6) /* upper bound = n/8 fifo */
#define DBLAC_RXTHR_LOW(n)   (((n) & 0x7) << 3) /* lower bound = n/8 fifo */
#define DBLAC_BURST_CAP16    (1 << 2)  /* support burst 16 */
#define DBLAC_BURST_CAP8     (1 << 1)  /* support burst 8 */
#define DBLAC_BURST_CAP4     (1 << 0)  /* support burst 4 */

#define DBLAC_DEFAULT \
	(DBLAC_RXTHR_EN | DBLAC_RXTHR_HIGH(6) | DBLAC_RXTHR_LOW(2))

/*
 * descriptor structure
 */
struct ftmac110_desc {
	uint64_t ctrl;
	uint32_t pbuf;
	void    *vbuf;
};

#define FTMAC110_RXD_END        ((uint64_t)1 << 63)
#define FTMAC110_RXD_BUFSZ(x)   (((uint64_t)(x) & 0x7ff) << 32)

#define FTMAC110_RXD_OWNER      ((uint64_t)1 << 31) /* owner: 1=HW, 0=SW */
#define FTMAC110_RXD_FRS        ((uint64_t)1 << 29) /* first pkt desc */
#define FTMAC110_RXD_LRS        ((uint64_t)1 << 28) /* last pkt desc */
#define FTMAC110_RXD_ODDNB      ((uint64_t)1 << 22) /* odd nibble */
#define FTMAC110_RXD_RUNT       ((uint64_t)1 << 21) /* runt pkt */
#define FTMAC110_RXD_FTL        ((uint64_t)1 << 20) /* frame too long */
#define FTMAC110_RXD_CRC        ((uint64_t)1 << 19) /* pkt crc error */
#define FTMAC110_RXD_ERR        ((uint64_t)1 << 18) /* bus error */
#define FTMAC110_RXD_ERRMASK    ((uint64_t)0x1f << 18)
#define FTMAC110_RXD_BCST       ((uint64_t)1 << 17) /* Bcst pkt */
#define FTMAC110_RXD_MCST       ((uint64_t)1 << 16) /* Mcst pkt */
#define FTMAC110_RXD_LEN(x)     ((uint64_t)((x) & 0x7ff))

#define FTMAC110_RXD_CLRMASK	\
	(FTMAC110_RXD_END | FTMAC110_RXD_BUFSZ(0x7ff))

#define FTMAC110_TXD_END    ((uint64_t)1 << 63) /* end of ring */
#define FTMAC110_TXD_TXIC   ((uint64_t)1 << 62) /* tx done interrupt */
#define FTMAC110_TXD_TX2FIC ((uint64_t)1 << 61) /* tx fifo interrupt */
#define FTMAC110_TXD_FTS    ((uint64_t)1 << 60) /* first pkt desc */
#define FTMAC110_TXD_LTS    ((uint64_t)1 << 59) /* last pkt desc */
#define FTMAC110_TXD_LEN(x) ((uint64_t)((x) & 0x7ff) << 32)

#define FTMAC110_TXD_OWNER  ((uint64_t)1 << 31)	/* owner: 1=HW, 0=SW */
#define FTMAC110_TXD_COL    ((uint64_t)3)		/* collision */

#define FTMAC110_TXD_CLRMASK    \
	(FTMAC110_TXD_END)

#endif  /* FTMAC110_H */
