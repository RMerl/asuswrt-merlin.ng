/*
 * Copyright (c) 2013 Eugene Krasnikov <k.eugene.e@gmail.com>
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
 */

#ifndef _DXE_H_
#define _DXE_H_

#include "wcn36xx.h"

/*
TX_LOW	= DMA0
TX_HIGH	= DMA4
RX_LOW	= DMA1
RX_HIGH	= DMA3
H2H_TEST_RX_TX = DMA2
*/

/* DXE registers */
#define WCN36XX_DXE_MEM_REG			0x202000

#define WCN36XX_DXE_CCU_INT			0xA0011
#define WCN36XX_DXE_REG_CCU_INT_3660		0x200b10
#define WCN36XX_DXE_REG_CCU_INT_3680		0x2050dc

/* TODO This must calculated properly but not hardcoded */
#define WCN36XX_DXE_CTRL_TX_L			0x328a44
#define WCN36XX_DXE_CTRL_TX_H			0x32ce44
#define WCN36XX_DXE_CTRL_RX_L			0x12ad2f
#define WCN36XX_DXE_CTRL_RX_H			0x12d12f
#define WCN36XX_DXE_CTRL_TX_H_BD		0x30ce45
#define WCN36XX_DXE_CTRL_TX_H_SKB		0x32ce4d
#define WCN36XX_DXE_CTRL_TX_L_BD		0x308a45
#define WCN36XX_DXE_CTRL_TX_L_SKB		0x328a4d

/* TODO This must calculated properly but not hardcoded */
#define WCN36XX_DXE_WQ_TX_L			0x17
#define WCN36XX_DXE_WQ_TX_H			0x17
#define WCN36XX_DXE_WQ_RX_L			0xB
#define WCN36XX_DXE_WQ_RX_H			0x4

/* DXE descriptor control filed */
#define WCN36XX_DXE_CTRL_VALID_MASK (0x00000001)

/* TODO This must calculated properly but not hardcoded */
/* DXE default control register values */
#define WCN36XX_DXE_CH_DEFAULT_CTL_RX_L		0x847EAD2F
#define WCN36XX_DXE_CH_DEFAULT_CTL_RX_H		0x84FED12F
#define WCN36XX_DXE_CH_DEFAULT_CTL_TX_H		0x853ECF4D
#define WCN36XX_DXE_CH_DEFAULT_CTL_TX_L		0x843e8b4d

/* Common DXE registers */
#define WCN36XX_DXE_MEM_CSR			(WCN36XX_DXE_MEM_REG + 0x00)
#define WCN36XX_DXE_REG_CSR_RESET		(WCN36XX_DXE_MEM_REG + 0x00)
#define WCN36XX_DXE_ENCH_ADDR			(WCN36XX_DXE_MEM_REG + 0x04)
#define WCN36XX_DXE_REG_CH_EN			(WCN36XX_DXE_MEM_REG + 0x08)
#define WCN36XX_DXE_REG_CH_DONE			(WCN36XX_DXE_MEM_REG + 0x0C)
#define WCN36XX_DXE_REG_CH_ERR			(WCN36XX_DXE_MEM_REG + 0x10)
#define WCN36XX_DXE_INT_MASK_REG		(WCN36XX_DXE_MEM_REG + 0x18)
#define WCN36XX_DXE_INT_SRC_RAW_REG		(WCN36XX_DXE_MEM_REG + 0x20)
	/* #define WCN36XX_DXE_INT_CH6_MASK	0x00000040 */
	/* #define WCN36XX_DXE_INT_CH5_MASK	0x00000020 */
	#define WCN36XX_DXE_INT_CH4_MASK	0x00000010
	#define WCN36XX_DXE_INT_CH3_MASK	0x00000008
	/* #define WCN36XX_DXE_INT_CH2_MASK	0x00000004 */
	#define WCN36XX_DXE_INT_CH1_MASK	0x00000002
	#define WCN36XX_DXE_INT_CH0_MASK	0x00000001
#define WCN36XX_DXE_0_INT_CLR			(WCN36XX_DXE_MEM_REG + 0x30)
#define WCN36XX_DXE_0_INT_ED_CLR		(WCN36XX_DXE_MEM_REG + 0x34)
#define WCN36XX_DXE_0_INT_DONE_CLR		(WCN36XX_DXE_MEM_REG + 0x38)
#define WCN36XX_DXE_0_INT_ERR_CLR		(WCN36XX_DXE_MEM_REG + 0x3C)

#define WCN36XX_DXE_0_CH0_STATUS		(WCN36XX_DXE_MEM_REG + 0x404)
#define WCN36XX_DXE_0_CH1_STATUS		(WCN36XX_DXE_MEM_REG + 0x444)
#define WCN36XX_DXE_0_CH2_STATUS		(WCN36XX_DXE_MEM_REG + 0x484)
#define WCN36XX_DXE_0_CH3_STATUS		(WCN36XX_DXE_MEM_REG + 0x4C4)
#define WCN36XX_DXE_0_CH4_STATUS		(WCN36XX_DXE_MEM_REG + 0x504)

#define WCN36XX_DXE_REG_RESET			0x5c89

/* Temporary BMU Workqueue 4 */
#define WCN36XX_DXE_BMU_WQ_RX_LOW		0xB
#define WCN36XX_DXE_BMU_WQ_RX_HIGH		0x4
/* DMA channel offset */
#define WCN36XX_DXE_TX_LOW_OFFSET		0x400
#define WCN36XX_DXE_TX_HIGH_OFFSET		0x500
#define WCN36XX_DXE_RX_LOW_OFFSET		0x440
#define WCN36XX_DXE_RX_HIGH_OFFSET		0x4C0

/* Address of the next DXE descriptor */
#define WCN36XX_DXE_CH_NEXT_DESC_ADDR		0x001C
#define WCN36XX_DXE_CH_NEXT_DESC_ADDR_TX_L	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_NEXT_DESC_ADDR)
#define WCN36XX_DXE_CH_NEXT_DESC_ADDR_TX_H	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_NEXT_DESC_ADDR)
#define WCN36XX_DXE_CH_NEXT_DESC_ADDR_RX_L	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_NEXT_DESC_ADDR)
#define WCN36XX_DXE_CH_NEXT_DESC_ADDR_RX_H	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_NEXT_DESC_ADDR)

/* DXE Descriptor source address */
#define WCN36XX_DXE_CH_SRC_ADDR			0x000C
#define WCN36XX_DXE_CH_SRC_ADDR_RX_L		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_SRC_ADDR)
#define WCN36XX_DXE_CH_SRC_ADDR_RX_H		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_SRC_ADDR)

/* DXE Descriptor address destination address */
#define WCN36XX_DXE_CH_DEST_ADDR		0x0014
#define WCN36XX_DXE_CH_DEST_ADDR_TX_L		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_DEST_ADDR)
#define WCN36XX_DXE_CH_DEST_ADDR_TX_H		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_DEST_ADDR)
#define WCN36XX_DXE_CH_DEST_ADDR_RX_L		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_DEST_ADDR)
#define WCN36XX_DXE_CH_DEST_ADDR_RX_H		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_DEST_ADDR)

/* Interrupt status */
#define WCN36XX_DXE_CH_STATUS_REG_ADDR		0x0004
#define WCN36XX_DXE_CH_STATUS_REG_ADDR_TX_L	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_STATUS_REG_ADDR)
#define WCN36XX_DXE_CH_STATUS_REG_ADDR_TX_H	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_STATUS_REG_ADDR)
#define WCN36XX_DXE_CH_STATUS_REG_ADDR_RX_L	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_LOW_OFFSET + \
						 WCN36XX_DXE_CH_STATUS_REG_ADDR)
#define WCN36XX_DXE_CH_STATUS_REG_ADDR_RX_H	(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_HIGH_OFFSET + \
						 WCN36XX_DXE_CH_STATUS_REG_ADDR)


/* DXE default control register */
#define WCN36XX_DXE_REG_CTL_RX_L		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_LOW_OFFSET)
#define WCN36XX_DXE_REG_CTL_RX_H		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_RX_HIGH_OFFSET)
#define WCN36XX_DXE_REG_CTL_TX_H		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_HIGH_OFFSET)
#define WCN36XX_DXE_REG_CTL_TX_L		(WCN36XX_DXE_MEM_REG + \
						 WCN36XX_DXE_TX_LOW_OFFSET)

#define WCN36XX_SMSM_WLAN_TX_ENABLE		0x00000400
#define WCN36XX_SMSM_WLAN_TX_RINGS_EMPTY	0x00000200


/* Interrupt control channel mask */
#define WCN36XX_INT_MASK_CHAN_TX_L		0x00000001
#define WCN36XX_INT_MASK_CHAN_RX_L		0x00000002
#define WCN36XX_INT_MASK_CHAN_RX_H		0x00000008
#define WCN36XX_INT_MASK_CHAN_TX_H		0x00000010

#define WCN36XX_BD_CHUNK_SIZE			128

#define WCN36XX_PKT_SIZE			0xF20
enum wcn36xx_dxe_ch_type {
	WCN36XX_DXE_CH_TX_L,
	WCN36XX_DXE_CH_TX_H,
	WCN36XX_DXE_CH_RX_L,
	WCN36XX_DXE_CH_RX_H
};

/* amount of descriptors per channel */
enum wcn36xx_dxe_ch_desc_num {
	WCN36XX_DXE_CH_DESC_NUMB_TX_L		= 128,
	WCN36XX_DXE_CH_DESC_NUMB_TX_H		= 10,
	WCN36XX_DXE_CH_DESC_NUMB_RX_L		= 512,
	WCN36XX_DXE_CH_DESC_NUMB_RX_H		= 40
};

/**
 * struct wcn36xx_dxe_desc - describes descriptor of one DXE buffer
 *
 * @ctrl: is a union that consists of following bits:
 * union {
 *	u32	valid		:1; //0 = DMA stop, 1 = DMA continue with this
 *				    //descriptor
 *	u32	transfer_type	:2; //0 = Host to Host space
 *	u32	eop		:1; //End of Packet
 *	u32	bd_handling	:1; //if transferType = Host to BMU, then 0
 *				    // means first 128 bytes contain BD, and 1
 *				    // means create new empty BD
 *	u32	siq		:1; // SIQ
 *	u32	diq		:1; // DIQ
 *	u32	pdu_rel		:1; //0 = don't release BD and PDUs when done,
 *				    // 1 = release them
 *	u32	bthld_sel	:4; //BMU Threshold Select
 *	u32	prio		:3; //Specifies the priority level to use for
 *				    // the transfer
 *	u32	stop_channel	:1; //1 = DMA stops processing further, channel
 *				    //requires re-enabling after this
 *	u32	intr		:1; //Interrupt on Descriptor Done
 *	u32	rsvd		:1; //reserved
 *	u32	size		:14;//14 bits used - ignored for BMU transfers,
 *				    //only used for host to host transfers?
 * } ctrl;
 */
struct wcn36xx_dxe_desc {
	u32	ctrl;
	u32	fr_len;

	u32	src_addr_l;
	u32	dst_addr_l;
	u32	phy_next_l;
	u32	src_addr_h;
	u32	dst_addr_h;
	u32	phy_next_h;
} __packed;

/* DXE Control block */
struct wcn36xx_dxe_ctl {
	struct wcn36xx_dxe_ctl	*next;
	struct wcn36xx_dxe_desc	*desc;
	unsigned int		desc_phy_addr;
	int			ctl_blk_order;
	struct sk_buff		*skb;
	spinlock_t              skb_lock;
	void			*bd_cpu_addr;
	dma_addr_t		bd_phy_addr;
};

struct wcn36xx_dxe_ch {
	enum wcn36xx_dxe_ch_type	ch_type;
	void				*cpu_addr;
	dma_addr_t			dma_addr;
	enum wcn36xx_dxe_ch_desc_num	desc_num;
	/* DXE control block ring */
	struct wcn36xx_dxe_ctl		*head_blk_ctl;
	struct wcn36xx_dxe_ctl		*tail_blk_ctl;

	/* DXE channel specific configs */
	u32				dxe_wq;
	u32				ctrl_bd;
	u32				ctrl_skb;
	u32				reg_ctrl;
	u32				def_ctrl;
};

/* Memory Pool for BD headers */
struct wcn36xx_dxe_mem_pool {
	int		chunk_size;
	void		*virt_addr;
	dma_addr_t	phy_addr;
};

struct wcn36xx_vif;
int wcn36xx_dxe_allocate_mem_pools(struct wcn36xx *wcn);
void wcn36xx_dxe_free_mem_pools(struct wcn36xx *wcn);
void wcn36xx_dxe_rx_frame(struct wcn36xx *wcn);
int wcn36xx_dxe_alloc_ctl_blks(struct wcn36xx *wcn);
void wcn36xx_dxe_free_ctl_blks(struct wcn36xx *wcn);
int wcn36xx_dxe_init(struct wcn36xx *wcn);
void wcn36xx_dxe_deinit(struct wcn36xx *wcn);
int wcn36xx_dxe_init_channels(struct wcn36xx *wcn);
int wcn36xx_dxe_tx_frame(struct wcn36xx *wcn,
			 struct wcn36xx_vif *vif_priv,
			 struct sk_buff *skb,
			 bool is_low);
void wcn36xx_dxe_tx_ack_ind(struct wcn36xx *wcn, u32 status);
void *wcn36xx_dxe_get_next_bd(struct wcn36xx *wcn, bool is_low);
#endif	/* _DXE_H_ */
