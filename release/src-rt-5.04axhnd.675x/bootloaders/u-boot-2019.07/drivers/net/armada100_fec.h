/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#ifndef __ARMADA100_FEC_H__
#define __ARMADA100_FEC_H__

#define PORT_NUM		0x0

/* RX & TX descriptor command */
#define BUF_OWNED_BY_DMA        (1<<31)

/* RX descriptor status */
#define RX_EN_INT               (1<<23)
#define RX_FIRST_DESC           (1<<17)
#define RX_LAST_DESC            (1<<16)
#define RX_ERROR                (1<<15)

/* TX descriptor command */
#define TX_EN_INT               (1<<23)
#define TX_GEN_CRC              (1<<22)
#define TX_ZERO_PADDING         (1<<18)
#define TX_FIRST_DESC           (1<<17)
#define TX_LAST_DESC            (1<<16)
#define TX_ERROR                (1<<15)

/* smi register */
#define SMI_BUSY                (1<<28)	/* 0 - Write, 1 - Read  */
#define SMI_R_VALID             (1<<27)	/* 0 - Write, 1 - Read  */
#define SMI_OP_W                (0<<26)	/* Write operation      */
#define SMI_OP_R                (1<<26)	/* Read operation */

#define HASH_ADD                0
#define HASH_DELETE             1
#define HASH_ADDR_TABLE_SIZE    0x4000	/* 16K (1/2K address - PCR_HS == 1) */
#define HOP_NUMBER              12

#define PHY_WAIT_ITERATIONS     1000	/* 1000 iterations * 10uS = 10mS max */
#define PHY_WAIT_MICRO_SECONDS  10

#define ETH_HW_IP_ALIGN         2	/* hw aligns IP header */
#define ETH_EXTRA_HEADER        (6+6+2+4)
					/* dest+src addr+protocol id+crc */
#define MAX_PKT_SIZE            1536


/* Bit definitions of the SDMA Config Reg */
#define SDCR_BSZ_OFF            12
#define SDCR_BSZ8               (3<<SDCR_BSZ_OFF)
#define SDCR_BSZ4               (2<<SDCR_BSZ_OFF)
#define SDCR_BSZ2               (1<<SDCR_BSZ_OFF)
#define SDCR_BSZ1               (0<<SDCR_BSZ_OFF)
#define SDCR_BLMR               (1<<6)
#define SDCR_BLMT               (1<<7)
#define SDCR_RIFB               (1<<9)
#define SDCR_RC_OFF             2
#define SDCR_RC_MAX_RETRANS     (0xf << SDCR_RC_OFF)

/* SDMA_CMD */
#define SDMA_CMD_AT             (1<<31)
#define SDMA_CMD_TXDL           (1<<24)
#define SDMA_CMD_TXDH           (1<<23)
#define SDMA_CMD_AR             (1<<15)
#define SDMA_CMD_ERD            (1<<7)


/* Bit definitions of the Port Config Reg */
#define PCR_HS                  (1<<12)
#define PCR_EN                  (1<<7)
#define PCR_PM                  (1<<0)

/* Bit definitions of the Port Config Extend Reg */
#define PCXR_2BSM               (1<<28)
#define PCXR_DSCP_EN            (1<<21)
#define PCXR_MFL_1518           (0<<14)
#define PCXR_MFL_1536           (1<<14)
#define PCXR_MFL_2048           (2<<14)
#define PCXR_MFL_64K            (3<<14)
#define PCXR_FLP                (1<<11)
#define PCXR_PRIO_TX_OFF        3
#define PCXR_TX_HIGH_PRI        (7<<PCXR_PRIO_TX_OFF)

/*
 *  * Bit definitions of the Interrupt Cause Reg
 *   * and Interrupt MASK Reg is the same
 *    */
#define ICR_RXBUF               (1<<0)
#define ICR_TXBUF_H             (1<<2)
#define ICR_TXBUF_L             (1<<3)
#define ICR_TXEND_H             (1<<6)
#define ICR_TXEND_L             (1<<7)
#define ICR_RXERR               (1<<8)
#define ICR_TXERR_H             (1<<10)
#define ICR_TXERR_L             (1<<11)
#define ICR_TX_UDR              (1<<13)
#define ICR_MII_CH              (1<<28)

#define ALL_INTS (ICR_TXBUF_H  | ICR_TXBUF_L  | ICR_TX_UDR |\
				ICR_TXERR_H  | ICR_TXERR_L |\
				ICR_TXEND_H  | ICR_TXEND_L |\
				ICR_RXBUF | ICR_RXERR  | ICR_MII_CH)

#define PHY_MASK               0x0000001f

#define to_darmdfec(_kd) container_of(_kd, struct armdfec_device, dev)
/* Size of a Tx/Rx descriptor used in chain list data structure */
#define ARMDFEC_RXQ_DESC_ALIGNED_SIZE \
	(((sizeof(struct rx_desc) / PKTALIGN) + 1) * PKTALIGN)

#define RX_BUF_OFFSET		0x2
#define RXQ			0x0	/* RX Queue 0 */
#define TXQ			0x1	/* TX Queue 1 */

struct addr_table_entry_t {
	u32 lo;
	u32 hi;
};

/* Bit fields of a Hash Table Entry */
enum hash_table_entry {
	HTEVALID = 1,
	HTESKIP = 2,
	HTERD = 4,
	HTERDBIT = 2
};

struct tx_desc {
	u32 cmd_sts;		/* Command/status field */
	u16 reserved;
	u16 byte_cnt;		/* buffer byte count */
	u8 *buf_ptr;		/* pointer to buffer for this descriptor */
	struct tx_desc *nextdesc_p;	/* Pointer to next descriptor */
};

struct rx_desc {
	u32 cmd_sts;		/* Descriptor command status */
	u16 byte_cnt;		/* Descriptor buffer byte count */
	u16 buf_size;		/* Buffer size */
	u8 *buf_ptr;		/* Descriptor buffer pointer */
	struct rx_desc *nxtdesc_p;	/* Next descriptor pointer */
};

/*
 * Armada100 Fast Ethernet controller Registers
 * Refer Datasheet Appendix A.22
 */
struct armdfec_reg {
	u32 phyadr;			/* PHY Address */
	u32 pad1[3];
	u32 smi;			/* SMI */
	u32 pad2[0xFB];
	u32 pconf;			/* Port configuration */
	u32 pad3;
	u32 pconf_ext;			/* Port configuration extend */
	u32 pad4;
	u32 pcmd;			/* Port Command */
	u32 pad5;
	u32 pstatus;			/* Port Status */
	u32 pad6;
	u32 spar;			/* Serial Parameters */
	u32 pad7;
	u32 htpr;			/* Hash table pointer */
	u32 pad8;
	u32 fcsal;			/* Flow control source address low */
	u32 pad9;
	u32 fcsah;			/* Flow control source address high */
	u32 pad10;
	u32 sdma_conf;			/* SDMA configuration */
	u32 pad11;
	u32 sdma_cmd;			/* SDMA command */
	u32 pad12;
	u32 ic;				/* Interrupt cause */
	u32 iwc;			/* Interrupt write to clear */
	u32 im;				/* Interrupt mask */
	u32 pad13;
	u32 *eth_idscpp[4];		/* Eth0 IP Differentiated Services Code
					   Point to Priority 0 Low */
	u32 eth_vlan_p;			/* Eth0 VLAN Priority Tag to Priority */
	u32 pad14[3];
	struct rx_desc *rxfdp[4];	/* Ethernet First Rx Descriptor
					   Pointer */
	u32 pad15[4];
	struct rx_desc *rxcdp[4];	/* Ethernet Current Rx Descriptor
					   Pointer */
	u32 pad16[0x0C];
	struct tx_desc *txcdp[2];	/* Ethernet Current Tx Descriptor
					   Pointer */
};

struct armdfec_device {
	struct eth_device dev;
	struct armdfec_reg *regs;
	struct tx_desc *p_txdesc;
	struct rx_desc *p_rxdesc;
	struct rx_desc *p_rxdesc_curr;
	u8 *p_rxbuf;
	u8 *p_aligned_txbuf;
	u8 *htpr;		/* hash pointer */
};

#endif /* __ARMADA100_FEC_H__ */
