/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Intel Platform Controller Hub EG20T (codename Topcliff) GMAC Driver
 * Adapted from linux drivers/net/ethernet/oki-semi/pch_gbe/pch_gbe.h
 */

#ifndef _PCH_GBE_H_
#define _PCH_GBE_H_

#define PCH_GBE_TIMEOUT		(3 * CONFIG_SYS_HZ)

#define PCH_GBE_DESC_NUM	4
#define PCH_GBE_ALIGN_SIZE	64

/*
 * Topcliff GBE MAC supports receiving ethernet frames with normal frame size
 * (64-1518 bytes) as well as up to 10318 bytes, however it does not have a
 * register bit to turn off receiving 'jumbo frame', so we have to allocate
 * our own buffer to store the received frames instead of using U-Boot's own.
 */
#define PCH_GBE_RX_FRAME_LEN	ROUND(10318, PCH_GBE_ALIGN_SIZE)

/* Interrupt Status */
/* Interrupt Status Hold */
/* Interrupt Enable */
#define PCH_GBE_INT_RX_DMA_CMPLT	0x00000001
#define PCH_GBE_INT_RX_VALID		0x00000002
#define PCH_GBE_INT_RX_FRAME_ERR	0x00000004
#define PCH_GBE_INT_RX_FIFO_ERR		0x00000008
#define PCH_GBE_INT_RX_DMA_ERR		0x00000010
#define PCH_GBE_INT_RX_DSC_EMP		0x00000020
#define PCH_GBE_INT_TX_CMPLT		0x00000100
#define PCH_GBE_INT_TX_DMA_CMPLT	0x00000200
#define PCH_GBE_INT_TX_FIFO_ERR		0x00000400
#define PCH_GBE_INT_TX_DMA_ERR		0x00000800
#define PCH_GBE_INT_PAUSE_CMPLT		0x00001000
#define PCH_GBE_INT_MIIM_CMPLT		0x00010000
#define PCH_GBE_INT_PHY_INT		0x00100000
#define PCH_GBE_INT_WOL_DET		0x01000000
#define PCH_GBE_INT_TCPIP_ERR		0x10000000

/* Mode */
#define PCH_GBE_MODE_MII_ETHER		0x00000000
#define PCH_GBE_MODE_GMII_ETHER		0x80000000
#define PCH_GBE_MODE_HALF_DUPLEX	0x00000000
#define PCH_GBE_MODE_FULL_DUPLEX	0x40000000
#define PCH_GBE_MODE_FR_BST		0x04000000

/* Reset */
#define PCH_GBE_ALL_RST			0x80000000
#define PCH_GBE_TX_RST			0x00008000
#define PCH_GBE_RX_RST			0x00004000

/* TCP/IP Accelerator Control */
#define PCH_GBE_EX_LIST_EN		0x00000008
#define PCH_GBE_RX_TCPIPACC_OFF		0x00000004
#define PCH_GBE_TX_TCPIPACC_EN		0x00000002
#define PCH_GBE_RX_TCPIPACC_EN		0x00000001

/* MAC RX Enable */
#define PCH_GBE_MRE_MAC_RX_EN		0x00000001

/* RX Flow Control */
#define PCH_GBE_FL_CTRL_EN		0x80000000

/* RX Mode */
#define PCH_GBE_ADD_FIL_EN		0x80000000
#define PCH_GBE_MLT_FIL_EN		0x40000000
#define PCH_GBE_RH_ALM_EMP_4		0x00000000
#define PCH_GBE_RH_ALM_EMP_8		0x00004000
#define PCH_GBE_RH_ALM_EMP_16		0x00008000
#define PCH_GBE_RH_ALM_EMP_32		0x0000c000
#define PCH_GBE_RH_ALM_FULL_4		0x00000000
#define PCH_GBE_RH_ALM_FULL_8		0x00001000
#define PCH_GBE_RH_ALM_FULL_16		0x00002000
#define PCH_GBE_RH_ALM_FULL_32		0x00003000
#define PCH_GBE_RH_RD_TRG_4		0x00000000
#define PCH_GBE_RH_RD_TRG_8		0x00000200
#define PCH_GBE_RH_RD_TRG_16		0x00000400
#define PCH_GBE_RH_RD_TRG_32		0x00000600
#define PCH_GBE_RH_RD_TRG_64		0x00000800
#define PCH_GBE_RH_RD_TRG_128		0x00000a00
#define PCH_GBE_RH_RD_TRG_256		0x00000c00
#define PCH_GBE_RH_RD_TRG_512		0x00000e00

/* TX Mode */
#define PCH_GBE_TM_NO_RTRY		0x80000000
#define PCH_GBE_TM_LONG_PKT		0x40000000
#define PCH_GBE_TM_ST_AND_FD		0x20000000
#define PCH_GBE_TM_SHORT_PKT		0x10000000
#define PCH_GBE_TM_LTCOL_RETX		0x08000000
#define PCH_GBE_TM_TH_TX_STRT_4		0x00000000
#define PCH_GBE_TM_TH_TX_STRT_8		0x00004000
#define PCH_GBE_TM_TH_TX_STRT_16	0x00008000
#define PCH_GBE_TM_TH_TX_STRT_32	0x0000c000
#define PCH_GBE_TM_TH_ALM_EMP_4		0x00000000
#define PCH_GBE_TM_TH_ALM_EMP_8		0x00000800
#define PCH_GBE_TM_TH_ALM_EMP_16	0x00001000
#define PCH_GBE_TM_TH_ALM_EMP_32	0x00001800
#define PCH_GBE_TM_TH_ALM_EMP_64	0x00002000
#define PCH_GBE_TM_TH_ALM_EMP_128	0x00002800
#define PCH_GBE_TM_TH_ALM_EMP_256	0x00003000
#define PCH_GBE_TM_TH_ALM_EMP_512	0x00003800
#define PCH_GBE_TM_TH_ALM_FULL_4	0x00000000
#define PCH_GBE_TM_TH_ALM_FULL_8	0x00000200
#define PCH_GBE_TM_TH_ALM_FULL_16	0x00000400
#define PCH_GBE_TM_TH_ALM_FULL_32	0x00000600

/* MAC Address Mask */
#define PCH_GBE_BUSY			0x80000000

/* MIIM  */
#define PCH_GBE_MIIM_OPER_WRITE		0x04000000
#define PCH_GBE_MIIM_OPER_READ		0x00000000
#define PCH_GBE_MIIM_OPER_READY		0x04000000
#define PCH_GBE_MIIM_PHY_ADDR_SHIFT	21
#define PCH_GBE_MIIM_REG_ADDR_SHIFT	16

/* RGMII Control */
#define PCH_GBE_CRS_SEL			0x00000010
#define PCH_GBE_RGMII_RATE_125M		0x00000000
#define PCH_GBE_RGMII_RATE_25M		0x00000008
#define PCH_GBE_RGMII_RATE_2_5M		0x0000000c
#define PCH_GBE_RGMII_MODE_GMII		0x00000000
#define PCH_GBE_RGMII_MODE_RGMII	0x00000002
#define PCH_GBE_CHIP_TYPE_EXTERNAL	0x00000000
#define PCH_GBE_CHIP_TYPE_INTERNAL	0x00000001

/* DMA Control */
#define PCH_GBE_RX_DMA_EN		0x00000002
#define PCH_GBE_TX_DMA_EN		0x00000001

/* Receive Descriptor bit definitions */
#define PCH_GBE_RXD_ACC_STAT_BCAST	0x00000400
#define PCH_GBE_RXD_ACC_STAT_MCAST	0x00000200
#define PCH_GBE_RXD_ACC_STAT_UCAST	0x00000100
#define PCH_GBE_RXD_ACC_STAT_TCPIPOK	0x000000c0
#define PCH_GBE_RXD_ACC_STAT_IPOK	0x00000080
#define PCH_GBE_RXD_ACC_STAT_TCPOK	0x00000040
#define PCH_GBE_RXD_ACC_STAT_IP6ERR	0x00000020
#define PCH_GBE_RXD_ACC_STAT_OFLIST	0x00000010
#define PCH_GBE_RXD_ACC_STAT_TYPEIP	0x00000008
#define PCH_GBE_RXD_ACC_STAT_MACL	0x00000004
#define PCH_GBE_RXD_ACC_STAT_PPPOE	0x00000002
#define PCH_GBE_RXD_ACC_STAT_VTAGT	0x00000001
#define PCH_GBE_RXD_GMAC_STAT_PAUSE	0x0200
#define PCH_GBE_RXD_GMAC_STAT_MARBR	0x0100
#define PCH_GBE_RXD_GMAC_STAT_MARMLT	0x0080
#define PCH_GBE_RXD_GMAC_STAT_MARIND	0x0040
#define PCH_GBE_RXD_GMAC_STAT_MARNOTMT	0x0020
#define PCH_GBE_RXD_GMAC_STAT_TLONG	0x0010
#define PCH_GBE_RXD_GMAC_STAT_TSHRT	0x0008
#define PCH_GBE_RXD_GMAC_STAT_NOTOCTAL	0x0004
#define PCH_GBE_RXD_GMAC_STAT_NBLERR	0x0002
#define PCH_GBE_RXD_GMAC_STAT_CRCERR	0x0001

/* Transmit Descriptor bit definitions */
#define PCH_GBE_TXD_CTRL_TCPIP_ACC_OFF	0x0008
#define PCH_GBE_TXD_CTRL_ITAG		0x0004
#define PCH_GBE_TXD_CTRL_ICRC		0x0002
#define PCH_GBE_TXD_CTRL_APAD		0x0001
#define PCH_GBE_TXD_WORDS_SHIFT		2
#define PCH_GBE_TXD_GMAC_STAT_CMPLT	0x2000
#define PCH_GBE_TXD_GMAC_STAT_ABT	0x1000
#define PCH_GBE_TXD_GMAC_STAT_EXCOL	0x0800
#define PCH_GBE_TXD_GMAC_STAT_SNGCOL	0x0400
#define PCH_GBE_TXD_GMAC_STAT_MLTCOL	0x0200
#define PCH_GBE_TXD_GMAC_STAT_CRSER	0x0100
#define PCH_GBE_TXD_GMAC_STAT_TLNG	0x0080
#define PCH_GBE_TXD_GMAC_STAT_TSHRT	0x0040
#define PCH_GBE_TXD_GMAC_STAT_LTCOL	0x0020
#define PCH_GBE_TXD_GMAC_STAT_TFUNDFLW	0x0010

/**
 * struct pch_gbe_rx_desc - Receive Descriptor
 * @buffer_addr:	RX Frame Buffer Address
 * @tcp_ip_status:	TCP/IP Accelerator Status
 * @rx_words_eob:	RX word count and Byte position
 * @gbec_status:	GMAC Status
 * @dma_status:		DMA Status
 * @reserved1:		Reserved
 * @reserved2:		Reserved
 */
struct pch_gbe_rx_desc {
	u32 buffer_addr;
	u32 tcp_ip_status;
	u16 rx_words_eob;
	u16 gbec_status;
	u8 dma_status;
	u8 reserved1;
	u16 reserved2;
};

/**
 * struct pch_gbe_tx_desc - Transmit Descriptor
 * @buffer_addr:	TX Frame Buffer Address
 * @length:		Data buffer length
 * @reserved1:		Reserved
 * @tx_words_eob:	TX word count and Byte position
 * @tx_frame_ctrl:	TX Frame Control
 * @dma_status:		DMA Status
 * @reserved2:		Reserved
 * @gbec_status:	GMAC Status
 */
struct pch_gbe_tx_desc {
	u32 buffer_addr;
	u16 length;
	u16 reserved1;
	u16 tx_words_eob;
	u16 tx_frame_ctrl;
	u8 dma_status;
	u8 reserved2;
	u16 gbec_status;
};

/**
 * pch_gbe_regs_mac_adr - structure holding values of mac address registers
 *
 * @high	Denotes the 1st to 4th byte from the initial of MAC address
 * @low		Denotes the 5th to 6th byte from the initial of MAC address
 */
struct pch_gbe_regs_mac_adr {
	u32 high;
	u32 low;
};

/**
 * pch_gbe_regs - structure holding values of MAC registers
 */
struct pch_gbe_regs {
	u32 int_st;
	u32 int_en;
	u32 mode;
	u32 reset;
	u32 tcpip_acc;
	u32 ex_list;
	u32 int_st_hold;
	u32 phy_int_ctrl;
	u32 mac_rx_en;
	u32 rx_fctrl;
	u32 pause_req;
	u32 rx_mode;
	u32 tx_mode;
	u32 rx_fifo_st;
	u32 tx_fifo_st;
	u32 tx_fid;
	u32 tx_result;
	u32 pause_pkt1;
	u32 pause_pkt2;
	u32 pause_pkt3;
	u32 pause_pkt4;
	u32 pause_pkt5;
	u32 reserve[2];
	struct pch_gbe_regs_mac_adr mac_adr[16];
	u32 addr_mask;
	u32 miim;
	u32 mac_addr_load;
	u32 rgmii_st;
	u32 rgmii_ctrl;
	u32 reserve3[3];
	u32 dma_ctrl;
	u32 reserve4[3];
	u32 rx_dsc_base;
	u32 rx_dsc_size;
	u32 rx_dsc_hw_p;
	u32 rx_dsc_hw_p_hld;
	u32 rx_dsc_sw_p;
	u32 reserve5[3];
	u32 tx_dsc_base;
	u32 tx_dsc_size;
	u32 tx_dsc_hw_p;
	u32 tx_dsc_hw_p_hld;
	u32 tx_dsc_sw_p;
	u32 reserve6[3];
	u32 rx_dma_st;
	u32 tx_dma_st;
	u32 reserve7[2];
	u32 wol_st;
	u32 wol_ctrl;
	u32 wol_addr_mask;
};

struct pch_gbe_priv {
	struct pch_gbe_rx_desc rx_desc[PCH_GBE_DESC_NUM];
	struct pch_gbe_tx_desc tx_desc[PCH_GBE_DESC_NUM];
	char rx_buff[PCH_GBE_DESC_NUM][PCH_GBE_RX_FRAME_LEN];
	struct phy_device *phydev;
	struct mii_dev *bus;
	struct pch_gbe_regs *mac_regs;
	struct udevice *dev;
	int rx_idx;
	int tx_idx;
};

#endif /* _PCH_GBE_H_ */
