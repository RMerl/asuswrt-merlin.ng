// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007, 2010 Freescale Semiconductor, Inc.
 *
 * Author: Roy Zang <tie-fei.zang@freescale.com>, Sep, 2007
 *
 * Description:
 * ULI 526x Ethernet port driver.
 * Based on the Linux driver: drivers/net/tulip/uli526x.c
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <pci.h>
#include <miiphy.h>

/* some kernel function compatible define */

#undef DEBUG

/* Board/System/Debug information/definition */
#define ULI_VENDOR_ID		0x10B9
#define ULI5261_DEVICE_ID	0x5261
#define ULI5263_DEVICE_ID	0x5263
/* ULi M5261 ID*/
#define PCI_ULI5261_ID		(ULI5261_DEVICE_ID << 16 | ULI_VENDOR_ID)
/* ULi M5263 ID*/
#define PCI_ULI5263_ID		(ULI5263_DEVICE_ID << 16 | ULI_VENDOR_ID)

#define ULI526X_IO_SIZE	0x100
#define TX_DESC_CNT	0x10		/* Allocated Tx descriptors */
#define RX_DESC_CNT	PKTBUFSRX	/* Allocated Rx descriptors */
#define TX_FREE_DESC_CNT	(TX_DESC_CNT - 2) /* Max TX packet count */
#define TX_WAKE_DESC_CNT	(TX_DESC_CNT - 3) /* TX wakeup count */
#define DESC_ALL_CNT		(TX_DESC_CNT + RX_DESC_CNT)
#define TX_BUF_ALLOC		0x300
#define RX_ALLOC_SIZE		PKTSIZE
#define ULI526X_RESET		1
#define CR0_DEFAULT		0
#define CR6_DEFAULT		0x22200000
#define CR7_DEFAULT		0x180c1
#define CR15_DEFAULT		0x06		/* TxJabber RxWatchdog */
#define TDES0_ERR_MASK		0x4302		/* TXJT, LC, EC, FUE */
#define MAX_PACKET_SIZE		1514
#define ULI5261_MAX_MULTICAST	14
#define RX_COPY_SIZE		100
#define MAX_CHECK_PACKET	0x8000

#define ULI526X_10MHF		0
#define ULI526X_100MHF		1
#define ULI526X_10MFD		4
#define ULI526X_100MFD		5
#define ULI526X_AUTO		8

#define ULI526X_TXTH_72		0x400000	/* TX TH 72 byte */
#define ULI526X_TXTH_96		0x404000	/* TX TH 96 byte */
#define ULI526X_TXTH_128	0x0000		/* TX TH 128 byte */
#define ULI526X_TXTH_256	0x4000		/* TX TH 256 byte */
#define ULI526X_TXTH_512	0x8000		/* TX TH 512 byte */
#define ULI526X_TXTH_1K		0xC000		/* TX TH 1K  byte */

/* CR9 definition: SROM/MII */
#define CR9_SROM_READ		0x4800
#define CR9_SRCS		0x1
#define CR9_SRCLK		0x2
#define CR9_CRDOUT		0x8
#define SROM_DATA_0		0x0
#define SROM_DATA_1		0x4
#define PHY_DATA_1		0x20000
#define PHY_DATA_0		0x00000
#define MDCLKH			0x10000

#define PHY_POWER_DOWN	0x800

#define SROM_V41_CODE		0x14

#define SROM_CLK_WRITE(data, ioaddr) do {			\
	outl(data|CR9_SROM_READ|CR9_SRCS, ioaddr);		\
	udelay(5);						\
	outl(data|CR9_SROM_READ|CR9_SRCS|CR9_SRCLK, ioaddr);	\
	udelay(5);						\
	outl(data|CR9_SROM_READ|CR9_SRCS, ioaddr);		\
	udelay(5);						\
	} while (0)

/* Structure/enum declaration */

struct tx_desc {
	u32 tdes0, tdes1, tdes2, tdes3; /* Data for the card */
	char *tx_buf_ptr;		/* Data for us */
	struct tx_desc *next_tx_desc;
};

struct rx_desc {
	u32 rdes0, rdes1, rdes2, rdes3;	/* Data for the card */
	char *rx_buf_ptr;		/* Data for us */
	struct rx_desc *next_rx_desc;
};

struct uli526x_board_info {
	u32 chip_id;	/* Chip vendor/Device ID */
	pci_dev_t pdev;

	long ioaddr;			/* I/O base address */
	u32 cr0_data;
	u32 cr5_data;
	u32 cr6_data;
	u32 cr7_data;
	u32 cr15_data;

	/* pointer for memory physical address */
	dma_addr_t buf_pool_dma_ptr;	/* Tx buffer pool memory */
	dma_addr_t buf_pool_dma_start;	/* Tx buffer pool align dword */
	dma_addr_t desc_pool_dma_ptr;	/* descriptor pool memory */
	dma_addr_t first_tx_desc_dma;
	dma_addr_t first_rx_desc_dma;

	/* descriptor pointer */
	unsigned char *buf_pool_ptr;	/* Tx buffer pool memory */
	unsigned char *buf_pool_start;	/* Tx buffer pool align dword */
	unsigned char *desc_pool_ptr;	/* descriptor pool memory */
	struct tx_desc *first_tx_desc;
	struct tx_desc *tx_insert_ptr;
	struct tx_desc *tx_remove_ptr;
	struct rx_desc *first_rx_desc;
	struct rx_desc *rx_ready_ptr;	/* packet come pointer */
	unsigned long tx_packet_cnt;	/* transmitted packet count */

	u16 PHY_reg4;			/* Saved Phyxcer register 4 value */

	u8 media_mode;			/* user specify media mode */
	u8 op_mode;			/* real work dedia mode */
	u8 phy_addr;

	/* NIC SROM data */
	unsigned char srom[128];
};

enum uli526x_offsets {
	DCR0 = 0x00, DCR1 = 0x08, DCR2 = 0x10, DCR3 = 0x18, DCR4 = 0x20,
	DCR5 = 0x28, DCR6 = 0x30, DCR7 = 0x38, DCR8 = 0x40, DCR9 = 0x48,
	DCR10 = 0x50, DCR11 = 0x58, DCR12 = 0x60, DCR13 = 0x68, DCR14 = 0x70,
	DCR15 = 0x78
};

enum uli526x_CR6_bits {
	CR6_RXSC = 0x2, CR6_PBF = 0x8, CR6_PM = 0x40, CR6_PAM = 0x80,
	CR6_FDM = 0x200, CR6_TXSC = 0x2000, CR6_STI = 0x100000,
	CR6_SFT = 0x200000, CR6_RXA = 0x40000000, CR6_NO_PURGE = 0x20000000
};

/* Global variable declaration -- */

static unsigned char uli526x_media_mode = ULI526X_AUTO;

static struct tx_desc desc_pool_array[DESC_ALL_CNT + 0x20]
	__attribute__ ((aligned(32)));
static char buf_pool[TX_BUF_ALLOC * TX_DESC_CNT + 4];

/* For module input parameter */
static int mode = 8;

/* function declaration -- */
static int uli526x_start_xmit(struct eth_device *dev, void *packet, int length);
static u16 read_srom_word(long, int);
static void uli526x_descriptor_init(struct uli526x_board_info *, unsigned long);
static void allocate_rx_buffer(struct uli526x_board_info *);
static void update_cr6(u32, unsigned long);
static u16 uli_phy_read(unsigned long, u8, u8, u32);
static u16 phy_readby_cr10(unsigned long, u8, u8);
static void uli_phy_write(unsigned long, u8, u8, u16, u32);
static void phy_writeby_cr10(unsigned long, u8, u8, u16);
static void phy_write_1bit(unsigned long, u32, u32);
static u16 phy_read_1bit(unsigned long, u32);
static int uli526x_rx_packet(struct eth_device *);
static void uli526x_free_tx_pkt(struct eth_device *,
		struct uli526x_board_info *);
static void uli526x_reuse_buf(struct rx_desc *);
static void uli526x_init(struct eth_device *);
static void uli526x_set_phyxcer(struct uli526x_board_info *);


static int uli526x_init_one(struct eth_device *, bd_t *);
static void uli526x_disable(struct eth_device *);
static void set_mac_addr(struct eth_device *);

static struct pci_device_id uli526x_pci_tbl[] = {
	{ ULI_VENDOR_ID, ULI5261_DEVICE_ID}, /* 5261 device */
	{ ULI_VENDOR_ID, ULI5263_DEVICE_ID}, /* 5263 device */
	{}
};

/* ULI526X network board routine */

/*
 *	Search ULI526X board, register it
 */

int uli526x_initialize(bd_t *bis)
{
	pci_dev_t devno;
	int card_number = 0;
	struct eth_device *dev;
	struct uli526x_board_info *db;	/* board information structure */

	u32 iobase;
	int idx = 0;

	while (1) {
		/* Find PCI device */
		devno = pci_find_devices(uli526x_pci_tbl, idx++);
		if (devno < 0)
			break;

		pci_read_config_dword(devno, PCI_BASE_ADDRESS_1, &iobase);
		iobase &= ~0xf;

		dev = (struct eth_device *)malloc(sizeof *dev);
		if (!dev) {
			printf("uli526x: Can not allocate memory\n");
			break;
		}
		memset(dev, 0, sizeof(*dev));
		sprintf(dev->name, "uli526x#%d", card_number);
		db = (struct uli526x_board_info *)
			malloc(sizeof(struct uli526x_board_info));

		dev->priv = db;
		db->pdev = devno;
		dev->iobase = iobase;

		dev->init = uli526x_init_one;
		dev->halt = uli526x_disable;
		dev->send = uli526x_start_xmit;
		dev->recv = uli526x_rx_packet;

		/* init db */
		db->ioaddr = dev->iobase;
		/* get chip id */

		pci_read_config_dword(devno, PCI_VENDOR_ID, &db->chip_id);
#ifdef DEBUG
		printf("uli526x: uli526x @0x%x\n", iobase);
		printf("uli526x: chip_id%x\n", db->chip_id);
#endif
		eth_register(dev);
		card_number++;
		pci_write_config_byte(devno, PCI_LATENCY_TIMER, 0x20);
		udelay(10 * 1000);
	}
	return card_number;
}

static int uli526x_init_one(struct eth_device *dev, bd_t *bis)
{

	struct uli526x_board_info *db = dev->priv;
	int i;

	switch (mode) {
	case ULI526X_10MHF:
	case ULI526X_100MHF:
	case ULI526X_10MFD:
	case ULI526X_100MFD:
		uli526x_media_mode = mode;
		break;
	default:
		uli526x_media_mode = ULI526X_AUTO;
		break;
	}

	/* Allocate Tx/Rx descriptor memory */
	db->desc_pool_ptr = (uchar *)&desc_pool_array[0];
	db->desc_pool_dma_ptr = (dma_addr_t)&desc_pool_array[0];
	if (db->desc_pool_ptr == NULL)
		return -1;

	db->buf_pool_ptr = (uchar *)&buf_pool[0];
	db->buf_pool_dma_ptr = (dma_addr_t)&buf_pool[0];
	if (db->buf_pool_ptr == NULL)
		return -1;

	db->first_tx_desc = (struct tx_desc *) db->desc_pool_ptr;
	db->first_tx_desc_dma = db->desc_pool_dma_ptr;

	db->buf_pool_start = db->buf_pool_ptr;
	db->buf_pool_dma_start = db->buf_pool_dma_ptr;

#ifdef DEBUG
	printf("%s(): db->ioaddr= 0x%x\n",
		__FUNCTION__, db->ioaddr);
	printf("%s(): media_mode= 0x%x\n",
		__FUNCTION__, uli526x_media_mode);
	printf("%s(): db->desc_pool_ptr= 0x%x\n",
		__FUNCTION__, db->desc_pool_ptr);
	printf("%s(): db->desc_pool_dma_ptr= 0x%x\n",
		__FUNCTION__, db->desc_pool_dma_ptr);
	printf("%s(): db->buf_pool_ptr= 0x%x\n",
		__FUNCTION__, db->buf_pool_ptr);
	printf("%s(): db->buf_pool_dma_ptr= 0x%x\n",
		__FUNCTION__, db->buf_pool_dma_ptr);
#endif

	/* read 64 word srom data */
	for (i = 0; i < 64; i++)
		((u16 *) db->srom)[i] = cpu_to_le16(read_srom_word(db->ioaddr,
			i));

	/* Set Node address */
	if (((db->srom[0] == 0xff) && (db->srom[1] == 0xff)) ||
	    ((db->srom[0] == 0x00) && (db->srom[1] == 0x00)))
	/* SROM absent, so write MAC address to ID Table */
		set_mac_addr(dev);
	else {		/*Exist SROM*/
		for (i = 0; i < 6; i++)
			dev->enetaddr[i] = db->srom[20 + i];
	}
#ifdef DEBUG
	for (i = 0; i < 6; i++)
		printf("%c%02x", i ? ':' : ' ', dev->enetaddr[i]);
#endif
	db->PHY_reg4 = 0x1e0;

	/* system variable init */
	db->cr6_data = CR6_DEFAULT ;
	db->cr6_data |= ULI526X_TXTH_256;
	db->cr0_data = CR0_DEFAULT;
	uli526x_init(dev);
	return 0;
}

static void uli526x_disable(struct eth_device *dev)
{
#ifdef DEBUG
	printf("uli526x_disable\n");
#endif
	struct uli526x_board_info *db = dev->priv;

	if (!((inl(db->ioaddr + DCR12)) & 0x8)) {
		/* Reset & stop ULI526X board */
		outl(ULI526X_RESET, db->ioaddr + DCR0);
		udelay(5);
		uli_phy_write(db->ioaddr, db->phy_addr, 0, 0x8000, db->chip_id);

		/* reset the board */
		db->cr6_data &= ~(CR6_RXSC | CR6_TXSC);	/* Disable Tx/Rx */
		update_cr6(db->cr6_data, dev->iobase);
		outl(0, dev->iobase + DCR7);		/* Disable Interrupt */
		outl(inl(dev->iobase + DCR5), dev->iobase + DCR5);
	}
}

/*	Initialize ULI526X board
 *	Reset ULI526X board
 *	Initialize TX/Rx descriptor chain structure
 *	Send the set-up frame
 *	Enable Tx/Rx machine
 */

static void uli526x_init(struct eth_device *dev)
{

	struct uli526x_board_info *db = dev->priv;
	u8	phy_tmp;
	u16	phy_value;
	u16 phy_reg_reset;

	/* Reset M526x MAC controller */
	outl(ULI526X_RESET, db->ioaddr + DCR0);	/* RESET MAC */
	udelay(100);
	outl(db->cr0_data, db->ioaddr + DCR0);
	udelay(5);

	/* Phy addr : In some boards,M5261/M5263 phy address != 1 */
	db->phy_addr = 1;
	db->tx_packet_cnt = 0;
	for (phy_tmp = 0; phy_tmp < 32; phy_tmp++) {
		/* peer add */
		phy_value = uli_phy_read(db->ioaddr, phy_tmp, 3, db->chip_id);
		if (phy_value != 0xffff && phy_value != 0) {
			db->phy_addr = phy_tmp;
			break;
		}
	}

#ifdef DEBUG
	printf("%s(): db->ioaddr= 0x%x\n", __FUNCTION__, db->ioaddr);
	printf("%s(): db->phy_addr= 0x%x\n", __FUNCTION__, db->phy_addr);
#endif
	if (phy_tmp == 32)
		printf("Can not find the phy address!!!");

	/* Parser SROM and media mode */
	db->media_mode = uli526x_media_mode;

	if (!(inl(db->ioaddr + DCR12) & 0x8)) {
		/* Phyxcer capability setting */
		phy_reg_reset = uli_phy_read(db->ioaddr,
			db->phy_addr, 0, db->chip_id);
		phy_reg_reset = (phy_reg_reset | 0x8000);
		uli_phy_write(db->ioaddr, db->phy_addr, 0,
			phy_reg_reset, db->chip_id);
		udelay(500);

		/* Process Phyxcer Media Mode */
		uli526x_set_phyxcer(db);
	}
	/* Media Mode Process */
	if (!(db->media_mode & ULI526X_AUTO))
		db->op_mode = db->media_mode;	/* Force Mode */

	/* Initialize Transmit/Receive decriptor and CR3/4 */
	uli526x_descriptor_init(db, db->ioaddr);

	/* Init CR6 to program M526X operation */
	update_cr6(db->cr6_data, db->ioaddr);

	/* Init CR7, interrupt active bit */
	db->cr7_data = CR7_DEFAULT;
	outl(db->cr7_data, db->ioaddr + DCR7);

	/* Init CR15, Tx jabber and Rx watchdog timer */
	outl(db->cr15_data, db->ioaddr + DCR15);

	/* Enable ULI526X Tx/Rx function */
	db->cr6_data |= CR6_RXSC | CR6_TXSC;
	update_cr6(db->cr6_data, db->ioaddr);
	while (!(inl(db->ioaddr + DCR12) & 0x8))
		udelay(10);
}

/*
 *	Hardware start transmission.
 *	Send a packet to media from the upper layer.
 */

static int uli526x_start_xmit(struct eth_device *dev, void *packet, int length)
{
	struct uli526x_board_info *db = dev->priv;
	struct tx_desc *txptr;
	unsigned int len = length;
	/* Too large packet check */
	if (len > MAX_PACKET_SIZE) {
		printf(": big packet = %d\n", len);
		return 0;
	}

	/* No Tx resource check, it never happen nromally */
	if (db->tx_packet_cnt >= TX_FREE_DESC_CNT) {
		printf("No Tx resource %ld\n", db->tx_packet_cnt);
		return 0;
	}

	/* Disable NIC interrupt */
	outl(0, dev->iobase + DCR7);

	/* transmit this packet */
	txptr = db->tx_insert_ptr;
	memcpy((char *)txptr->tx_buf_ptr, (char *)packet, (int)length);
	txptr->tdes1 = cpu_to_le32(0xe1000000 | length);

	/* Point to next transmit free descriptor */
	db->tx_insert_ptr = txptr->next_tx_desc;

	/* Transmit Packet Process */
	if ((db->tx_packet_cnt < TX_DESC_CNT)) {
		txptr->tdes0 = cpu_to_le32(0x80000000);	/* Set owner bit */
		db->tx_packet_cnt++;			/* Ready to send */
		outl(0x1, dev->iobase + DCR1);	/* Issue Tx polling */
	}

	/* Got ULI526X status */
	db->cr5_data = inl(db->ioaddr + DCR5);
	outl(db->cr5_data, db->ioaddr + DCR5);

#ifdef TX_DEBUG
	printf("%s(): length = 0x%x\n", __FUNCTION__, length);
	printf("%s(): cr5_data=%x\n", __FUNCTION__, db->cr5_data);
#endif

	outl(db->cr7_data, dev->iobase + DCR7);
	uli526x_free_tx_pkt(dev, db);

	return length;
}

/*
 *	Free TX resource after TX complete
 */

static void uli526x_free_tx_pkt(struct eth_device *dev,
	struct uli526x_board_info *db)
{
	struct tx_desc *txptr;
	u32 tdes0;

	txptr = db->tx_remove_ptr;
	while (db->tx_packet_cnt) {
		tdes0 = le32_to_cpu(txptr->tdes0);
		/* printf(DRV_NAME ": tdes0=%x\n", tdes0); */
		if (tdes0 & 0x80000000)
			break;

		/* A packet sent completed */
		db->tx_packet_cnt--;

		if (tdes0 != 0x7fffffff) {
#ifdef TX_DEBUG
			printf("%s()tdes0=%x\n", __FUNCTION__, tdes0);
#endif
			if (tdes0 & TDES0_ERR_MASK) {
				if (tdes0 & 0x0002) {	/* UnderRun */
					if (!(db->cr6_data & CR6_SFT)) {
						db->cr6_data = db->cr6_data |
							CR6_SFT;
						update_cr6(db->cr6_data,
							db->ioaddr);
					}
				}
			}
		}

		txptr = txptr->next_tx_desc;
	}/* End of while */

	/* Update TX remove pointer to next */
	db->tx_remove_ptr = txptr;
}


/*
 *	Receive the come packet and pass to upper layer
 */

static int uli526x_rx_packet(struct eth_device *dev)
{
	struct uli526x_board_info *db = dev->priv;
	struct rx_desc *rxptr;
	int rxlen = 0;
	u32 rdes0;

	rxptr = db->rx_ready_ptr;

	rdes0 = le32_to_cpu(rxptr->rdes0);
#ifdef RX_DEBUG
	printf("%s(): rxptr->rdes0=%x\n", __FUNCTION__, rxptr->rdes0);
#endif
	if (!(rdes0 & 0x80000000)) {	/* packet owner check */
		if ((rdes0 & 0x300) != 0x300) {
			/* A packet without First/Last flag */
			/* reuse this buf */
			printf("A packet without First/Last flag");
			uli526x_reuse_buf(rxptr);
		} else {
			/* A packet with First/Last flag */
			rxlen = ((rdes0 >> 16) & 0x3fff) - 4;
#ifdef RX_DEBUG
			printf("%s(): rxlen =%x\n", __FUNCTION__, rxlen);
#endif
			/* error summary bit check */
			if (rdes0 & 0x8000) {
				/* This is a error packet */
				printf("Error: rdes0: %x\n", rdes0);
			}

			if (!(rdes0 & 0x8000) ||
				((db->cr6_data & CR6_PM) && (rxlen > 6))) {

#ifdef RX_DEBUG
				printf("%s(): rx_skb_ptr =%x\n",
					__FUNCTION__, rxptr->rx_buf_ptr);
				printf("%s(): rxlen =%x\n",
					__FUNCTION__, rxlen);

				printf("%s(): buf addr =%x\n",
					__FUNCTION__, rxptr->rx_buf_ptr);
				printf("%s(): rxlen =%x\n",
					__FUNCTION__, rxlen);
				int i;
				for (i = 0; i < 0x20; i++)
					printf("%s(): data[%x] =%x\n",
					__FUNCTION__, i, rxptr->rx_buf_ptr[i]);
#endif

				net_process_received_packet(
					(uchar *)rxptr->rx_buf_ptr, rxlen);
				uli526x_reuse_buf(rxptr);

			} else {
				/* Reuse SKB buffer when the packet is error */
				printf("Reuse buffer, rdes0");
				uli526x_reuse_buf(rxptr);
			}
		}

		rxptr = rxptr->next_rx_desc;
	}

	db->rx_ready_ptr = rxptr;
	return rxlen;
}

/*
 *	Reuse the RX buffer
 */

static void uli526x_reuse_buf(struct rx_desc *rxptr)
{

	if (!(rxptr->rdes0 & cpu_to_le32(0x80000000)))
		rxptr->rdes0 = cpu_to_le32(0x80000000);
	else
		printf("Buffer reuse method error");
}
/*
 *	Initialize transmit/Receive descriptor
 *	Using Chain structure, and allocate Tx/Rx buffer
 */

static void uli526x_descriptor_init(struct uli526x_board_info *db,
	unsigned long ioaddr)
{
	struct tx_desc *tmp_tx;
	struct rx_desc *tmp_rx;
	unsigned char *tmp_buf;
	dma_addr_t tmp_tx_dma, tmp_rx_dma;
	dma_addr_t tmp_buf_dma;
	int i;
	/* tx descriptor start pointer */
	db->tx_insert_ptr = db->first_tx_desc;
	db->tx_remove_ptr = db->first_tx_desc;

	outl(db->first_tx_desc_dma, ioaddr + DCR4);     /* TX DESC address */

	/* rx descriptor start pointer */
	db->first_rx_desc = (void *)db->first_tx_desc +
		sizeof(struct tx_desc) * TX_DESC_CNT;
	db->first_rx_desc_dma =  db->first_tx_desc_dma +
		sizeof(struct tx_desc) * TX_DESC_CNT;
	db->rx_ready_ptr = db->first_rx_desc;
	outl(db->first_rx_desc_dma, ioaddr + DCR3);	/* RX DESC address */
#ifdef DEBUG
	printf("%s(): db->first_tx_desc= 0x%x\n",
		__FUNCTION__, db->first_tx_desc);
	printf("%s(): db->first_rx_desc_dma= 0x%x\n",
		__FUNCTION__, db->first_rx_desc_dma);
#endif
	/* Init Transmit chain */
	tmp_buf = db->buf_pool_start;
	tmp_buf_dma = db->buf_pool_dma_start;
	tmp_tx_dma = db->first_tx_desc_dma;
	for (tmp_tx = db->first_tx_desc, i = 0;
			i < TX_DESC_CNT; i++, tmp_tx++) {
		tmp_tx->tx_buf_ptr = (char *)tmp_buf;
		tmp_tx->tdes0 = cpu_to_le32(0);
		tmp_tx->tdes1 = cpu_to_le32(0x81000000);	/* IC, chain */
		tmp_tx->tdes2 = cpu_to_le32(tmp_buf_dma);
		tmp_tx_dma += sizeof(struct tx_desc);
		tmp_tx->tdes3 = cpu_to_le32(tmp_tx_dma);
		tmp_tx->next_tx_desc = tmp_tx + 1;
		tmp_buf = tmp_buf + TX_BUF_ALLOC;
		tmp_buf_dma = tmp_buf_dma + TX_BUF_ALLOC;
	}
	(--tmp_tx)->tdes3 = cpu_to_le32(db->first_tx_desc_dma);
	tmp_tx->next_tx_desc = db->first_tx_desc;

	 /* Init Receive descriptor chain */
	tmp_rx_dma = db->first_rx_desc_dma;
	for (tmp_rx = db->first_rx_desc, i = 0; i < RX_DESC_CNT;
			i++, tmp_rx++) {
		tmp_rx->rdes0 = cpu_to_le32(0);
		tmp_rx->rdes1 = cpu_to_le32(0x01000600);
		tmp_rx_dma += sizeof(struct rx_desc);
		tmp_rx->rdes3 = cpu_to_le32(tmp_rx_dma);
		tmp_rx->next_rx_desc = tmp_rx + 1;
	}
	(--tmp_rx)->rdes3 = cpu_to_le32(db->first_rx_desc_dma);
	tmp_rx->next_rx_desc = db->first_rx_desc;

	/* pre-allocate Rx buffer */
	allocate_rx_buffer(db);
}

/*
 *	Update CR6 value
 *	Firstly stop ULI526X, then written value and start
 */

static void update_cr6(u32 cr6_data, unsigned long ioaddr)
{

	outl(cr6_data, ioaddr + DCR6);
	udelay(5);
}

/*
 *	Allocate rx buffer,
 */

static void allocate_rx_buffer(struct uli526x_board_info *db)
{
	int index;
	struct rx_desc *rxptr;
	rxptr = db->first_rx_desc;
	u32 addr;

	for (index = 0; index < RX_DESC_CNT; index++) {
		addr = (u32)net_rx_packets[index];
		addr += (16 - (addr & 15));
		rxptr->rx_buf_ptr = (char *) addr;
		rxptr->rdes2 = cpu_to_le32(addr);
		rxptr->rdes0 = cpu_to_le32(0x80000000);
#ifdef DEBUG
		printf("%s(): Number 0x%x:\n", __FUNCTION__, index);
		printf("%s(): addr 0x%x:\n", __FUNCTION__, addr);
		printf("%s(): rxptr address = 0x%x\n", __FUNCTION__, rxptr);
		printf("%s(): rxptr buf address = 0x%x\n", \
			__FUNCTION__, rxptr->rx_buf_ptr);
		printf("%s(): rdes2  = 0x%x\n", __FUNCTION__, rxptr->rdes2);
#endif
		rxptr = rxptr->next_rx_desc;
	}
}

/*
 *	Read one word data from the serial ROM
 */

static u16 read_srom_word(long ioaddr, int offset)
{
	int i;
	u16 srom_data = 0;
	long cr9_ioaddr = ioaddr + DCR9;

	outl(CR9_SROM_READ, cr9_ioaddr);
	outl(CR9_SROM_READ | CR9_SRCS, cr9_ioaddr);

	/* Send the Read Command 110b */
	SROM_CLK_WRITE(SROM_DATA_1, cr9_ioaddr);
	SROM_CLK_WRITE(SROM_DATA_1, cr9_ioaddr);
	SROM_CLK_WRITE(SROM_DATA_0, cr9_ioaddr);

	/* Send the offset */
	for (i = 5; i >= 0; i--) {
		srom_data = (offset & (1 << i)) ? SROM_DATA_1 : SROM_DATA_0;
		SROM_CLK_WRITE(srom_data, cr9_ioaddr);
	}

	outl(CR9_SROM_READ | CR9_SRCS, cr9_ioaddr);

	for (i = 16; i > 0; i--) {
		outl(CR9_SROM_READ | CR9_SRCS | CR9_SRCLK, cr9_ioaddr);
		udelay(5);
		srom_data = (srom_data << 1) | ((inl(cr9_ioaddr) & CR9_CRDOUT)
			? 1 : 0);
		outl(CR9_SROM_READ | CR9_SRCS, cr9_ioaddr);
		udelay(5);
	}

	outl(CR9_SROM_READ, cr9_ioaddr);
	return srom_data;
}

/*
 *	Set 10/100 phyxcer capability
 *	AUTO mode : phyxcer register4 is NIC capability
 *	Force mode: phyxcer register4 is the force media
 */

static void uli526x_set_phyxcer(struct uli526x_board_info *db)
{
	u16 phy_reg;

	/* Phyxcer capability setting */
	phy_reg = uli_phy_read(db->ioaddr,
			db->phy_addr, 4, db->chip_id) & ~0x01e0;

	if (db->media_mode & ULI526X_AUTO) {
		/* AUTO Mode */
		phy_reg |= db->PHY_reg4;
	} else {
		/* Force Mode */
		switch (db->media_mode) {
		case ULI526X_10MHF: phy_reg |= 0x20; break;
		case ULI526X_10MFD: phy_reg |= 0x40; break;
		case ULI526X_100MHF: phy_reg |= 0x80; break;
		case ULI526X_100MFD: phy_reg |= 0x100; break;
		}

	}

	/* Write new capability to Phyxcer Reg4 */
	if (!(phy_reg & 0x01e0)) {
		phy_reg |= db->PHY_reg4;
		db->media_mode |= ULI526X_AUTO;
	}
	uli_phy_write(db->ioaddr, db->phy_addr, 4, phy_reg, db->chip_id);

	/* Restart Auto-Negotiation */
	uli_phy_write(db->ioaddr, db->phy_addr, 0, 0x1200, db->chip_id);
	udelay(50);
}

/*
 *	Write a word to Phy register
 */

static void uli_phy_write(unsigned long iobase, u8 phy_addr, u8 offset,
	u16 phy_data, u32 chip_id)
{
	u16 i;
	unsigned long ioaddr;

	if (chip_id == PCI_ULI5263_ID) {
		phy_writeby_cr10(iobase, phy_addr, offset, phy_data);
		return;
	}
	/* M5261/M5263 Chip */
	ioaddr = iobase + DCR9;

	/* Send 33 synchronization clock to Phy controller */
	for (i = 0; i < 35; i++)
		phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);

	/* Send start command(01) to Phy */
	phy_write_1bit(ioaddr, PHY_DATA_0, chip_id);
	phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);

	/* Send write command(01) to Phy */
	phy_write_1bit(ioaddr, PHY_DATA_0, chip_id);
	phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);

	/* Send Phy address */
	for (i = 0x10; i > 0; i = i >> 1)
		phy_write_1bit(ioaddr, phy_addr & i ?
			PHY_DATA_1 : PHY_DATA_0, chip_id);

	/* Send register address */
	for (i = 0x10; i > 0; i = i >> 1)
		phy_write_1bit(ioaddr, offset & i ?
			PHY_DATA_1 : PHY_DATA_0, chip_id);

	/* written trasnition */
	phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);
	phy_write_1bit(ioaddr, PHY_DATA_0, chip_id);

	/* Write a word data to PHY controller */
	for (i = 0x8000; i > 0; i >>= 1)
		phy_write_1bit(ioaddr, phy_data & i ?
			PHY_DATA_1 : PHY_DATA_0, chip_id);
}

/*
 *	Read a word data from phy register
 */

static u16 uli_phy_read(unsigned long iobase, u8 phy_addr, u8 offset,
			u32 chip_id)
{
	int i;
	u16 phy_data;
	unsigned long ioaddr;

	if (chip_id == PCI_ULI5263_ID)
		return phy_readby_cr10(iobase, phy_addr, offset);
	/* M5261/M5263 Chip */
	ioaddr = iobase + DCR9;

	/* Send 33 synchronization clock to Phy controller */
	for (i = 0; i < 35; i++)
		phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);

	/* Send start command(01) to Phy */
	phy_write_1bit(ioaddr, PHY_DATA_0, chip_id);
	phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);

	/* Send read command(10) to Phy */
	phy_write_1bit(ioaddr, PHY_DATA_1, chip_id);
	phy_write_1bit(ioaddr, PHY_DATA_0, chip_id);

	/* Send Phy address */
	for (i = 0x10; i > 0; i = i >> 1)
		phy_write_1bit(ioaddr, phy_addr & i ?
			PHY_DATA_1 : PHY_DATA_0, chip_id);

	/* Send register address */
	for (i = 0x10; i > 0; i = i >> 1)
		phy_write_1bit(ioaddr, offset & i ?
			PHY_DATA_1 : PHY_DATA_0, chip_id);

	/* Skip transition state */
	phy_read_1bit(ioaddr, chip_id);

	/* read 16bit data */
	for (phy_data = 0, i = 0; i < 16; i++) {
		phy_data <<= 1;
		phy_data |= phy_read_1bit(ioaddr, chip_id);
	}

	return phy_data;
}

static u16 phy_readby_cr10(unsigned long iobase, u8 phy_addr, u8 offset)
{
	unsigned long ioaddr, cr10_value;

	ioaddr = iobase + DCR10;
	cr10_value = phy_addr;
	cr10_value = (cr10_value<<5) + offset;
	cr10_value = (cr10_value<<16) + 0x08000000;
	outl(cr10_value, ioaddr);
	udelay(1);
	while (1) {
		cr10_value = inl(ioaddr);
		if (cr10_value & 0x10000000)
			break;
	}
	return (cr10_value&0x0ffff);
}

static void phy_writeby_cr10(unsigned long iobase, u8 phy_addr,
	u8 offset, u16 phy_data)
{
	unsigned long ioaddr, cr10_value;

	ioaddr = iobase + DCR10;
	cr10_value = phy_addr;
	cr10_value = (cr10_value<<5) + offset;
	cr10_value = (cr10_value<<16) + 0x04000000 + phy_data;
	outl(cr10_value, ioaddr);
	udelay(1);
}
/*
 *	Write one bit data to Phy Controller
 */

static void phy_write_1bit(unsigned long ioaddr, u32 phy_data, u32 chip_id)
{
	outl(phy_data , ioaddr);			/* MII Clock Low */
	udelay(1);
	outl(phy_data  | MDCLKH, ioaddr);	/* MII Clock High */
	udelay(1);
	outl(phy_data , ioaddr);			/* MII Clock Low */
	udelay(1);
}

/*
 *	Read one bit phy data from PHY controller
 */

static u16 phy_read_1bit(unsigned long ioaddr, u32 chip_id)
{
	u16 phy_data;

	outl(0x50000 , ioaddr);
	udelay(1);
	phy_data = (inl(ioaddr) >> 19) & 0x1;
	outl(0x40000 , ioaddr);
	udelay(1);

	return phy_data;
}

/*
 * Set MAC address to ID Table
 */

static void set_mac_addr(struct eth_device *dev)
{
	int i;
	u16 addr;
	struct uli526x_board_info *db = dev->priv;
	outl(0x10000, db->ioaddr + DCR0);	/* Diagnosis mode */
	/* Reset dianostic pointer port */
	outl(0x1c0, db->ioaddr + DCR13);
	outl(0, db->ioaddr + DCR14);	/* Clear reset port */
	outl(0x10, db->ioaddr + DCR14);	/* Reset ID Table pointer */
	outl(0, db->ioaddr + DCR14);	/* Clear reset port */
	outl(0, db->ioaddr + DCR13);	/* Clear CR13 */
	/* Select ID Table access port */
	outl(0x1b0, db->ioaddr + DCR13);
	/* Read MAC address from CR14 */
	for (i = 0; i < 3; i++) {
		addr = dev->enetaddr[2 * i] | (dev->enetaddr[2 * i + 1] << 8);
		outl(addr, db->ioaddr + DCR14);
	}
	/* write end */
	outl(0, db->ioaddr + DCR13);	/* Clear CR13 */
	outl(0, db->ioaddr + DCR0);	/* Clear CR0 */
	udelay(10);
	return;
}
