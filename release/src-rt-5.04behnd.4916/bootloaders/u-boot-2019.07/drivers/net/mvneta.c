// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Marvell NETA network card for Armada XP and Armada 370 SoCs.
 *
 * U-Boot version:
 * Copyright (C) 2014-2015 Stefan Roese <sr@denx.de>
 *
 * Based on the Linux version which is:
 * Copyright (C) 2012 Marvell
 *
 * Rami Rosen <rosenr@marvell.com>
 * Thomas Petazzoni <thomas.petazzoni@free-electrons.com>
 */

#include <common.h>
#include <dm.h>
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

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_PHYLIB)
# error Marvell mvneta requires PHYLIB
#endif

#define CONFIG_NR_CPUS		1
#define ETH_HLEN		14	/* Total octets in header */

/* 2(HW hdr) 14(MAC hdr) 4(CRC) 32(extra for cache prefetch) */
#define WRAP			(2 + ETH_HLEN + 4 + 32)
#define MTU			1500
#define RX_BUFFER_SIZE		(ALIGN(MTU + WRAP, ARCH_DMA_MINALIGN))

#define MVNETA_SMI_TIMEOUT			10000

/* Registers */
#define MVNETA_RXQ_CONFIG_REG(q)                (0x1400 + ((q) << 2))
#define	     MVNETA_RXQ_HW_BUF_ALLOC            BIT(1)
#define      MVNETA_RXQ_PKT_OFFSET_ALL_MASK     (0xf    << 8)
#define      MVNETA_RXQ_PKT_OFFSET_MASK(offs)   ((offs) << 8)
#define MVNETA_RXQ_THRESHOLD_REG(q)             (0x14c0 + ((q) << 2))
#define      MVNETA_RXQ_NON_OCCUPIED(v)         ((v) << 16)
#define MVNETA_RXQ_BASE_ADDR_REG(q)             (0x1480 + ((q) << 2))
#define MVNETA_RXQ_SIZE_REG(q)                  (0x14a0 + ((q) << 2))
#define      MVNETA_RXQ_BUF_SIZE_SHIFT          19
#define      MVNETA_RXQ_BUF_SIZE_MASK           (0x1fff << 19)
#define MVNETA_RXQ_STATUS_REG(q)                (0x14e0 + ((q) << 2))
#define      MVNETA_RXQ_OCCUPIED_ALL_MASK       0x3fff
#define MVNETA_RXQ_STATUS_UPDATE_REG(q)         (0x1500 + ((q) << 2))
#define      MVNETA_RXQ_ADD_NON_OCCUPIED_SHIFT  16
#define      MVNETA_RXQ_ADD_NON_OCCUPIED_MAX    255
#define MVNETA_PORT_RX_RESET                    0x1cc0
#define      MVNETA_PORT_RX_DMA_RESET           BIT(0)
#define MVNETA_PHY_ADDR                         0x2000
#define      MVNETA_PHY_ADDR_MASK               0x1f
#define MVNETA_SMI                              0x2004
#define      MVNETA_PHY_REG_MASK                0x1f
/* SMI register fields */
#define     MVNETA_SMI_DATA_OFFS		0	/* Data */
#define     MVNETA_SMI_DATA_MASK		(0xffff << MVNETA_SMI_DATA_OFFS)
#define     MVNETA_SMI_DEV_ADDR_OFFS		16	/* PHY device address */
#define     MVNETA_SMI_REG_ADDR_OFFS		21	/* PHY device reg addr*/
#define     MVNETA_SMI_OPCODE_OFFS		26	/* Write/Read opcode */
#define     MVNETA_SMI_OPCODE_READ		(1 << MVNETA_SMI_OPCODE_OFFS)
#define     MVNETA_SMI_READ_VALID		(1 << 27)	/* Read Valid */
#define     MVNETA_SMI_BUSY			(1 << 28)	/* Busy */
#define MVNETA_MBUS_RETRY                       0x2010
#define MVNETA_UNIT_INTR_CAUSE                  0x2080
#define MVNETA_UNIT_CONTROL                     0x20B0
#define      MVNETA_PHY_POLLING_ENABLE          BIT(1)
#define MVNETA_WIN_BASE(w)                      (0x2200 + ((w) << 3))
#define MVNETA_WIN_SIZE(w)                      (0x2204 + ((w) << 3))
#define MVNETA_WIN_REMAP(w)                     (0x2280 + ((w) << 2))
#define MVNETA_WIN_SIZE_MASK			(0xffff0000)
#define MVNETA_BASE_ADDR_ENABLE                 0x2290
#define      MVNETA_BASE_ADDR_ENABLE_BIT	0x1
#define MVNETA_PORT_ACCESS_PROTECT              0x2294
#define      MVNETA_PORT_ACCESS_PROTECT_WIN0_RW	0x3
#define MVNETA_PORT_CONFIG                      0x2400
#define      MVNETA_UNI_PROMISC_MODE            BIT(0)
#define      MVNETA_DEF_RXQ(q)                  ((q) << 1)
#define      MVNETA_DEF_RXQ_ARP(q)              ((q) << 4)
#define      MVNETA_TX_UNSET_ERR_SUM            BIT(12)
#define      MVNETA_DEF_RXQ_TCP(q)              ((q) << 16)
#define      MVNETA_DEF_RXQ_UDP(q)              ((q) << 19)
#define      MVNETA_DEF_RXQ_BPDU(q)             ((q) << 22)
#define      MVNETA_RX_CSUM_WITH_PSEUDO_HDR     BIT(25)
#define      MVNETA_PORT_CONFIG_DEFL_VALUE(q)   (MVNETA_DEF_RXQ(q)       | \
						 MVNETA_DEF_RXQ_ARP(q)	 | \
						 MVNETA_DEF_RXQ_TCP(q)	 | \
						 MVNETA_DEF_RXQ_UDP(q)	 | \
						 MVNETA_DEF_RXQ_BPDU(q)	 | \
						 MVNETA_TX_UNSET_ERR_SUM | \
						 MVNETA_RX_CSUM_WITH_PSEUDO_HDR)
#define MVNETA_PORT_CONFIG_EXTEND                0x2404
#define MVNETA_MAC_ADDR_LOW                      0x2414
#define MVNETA_MAC_ADDR_HIGH                     0x2418
#define MVNETA_SDMA_CONFIG                       0x241c
#define      MVNETA_SDMA_BRST_SIZE_16            4
#define      MVNETA_RX_BRST_SZ_MASK(burst)       ((burst) << 1)
#define      MVNETA_RX_NO_DATA_SWAP              BIT(4)
#define      MVNETA_TX_NO_DATA_SWAP              BIT(5)
#define      MVNETA_DESC_SWAP                    BIT(6)
#define      MVNETA_TX_BRST_SZ_MASK(burst)       ((burst) << 22)
#define MVNETA_PORT_STATUS                       0x2444
#define      MVNETA_TX_IN_PRGRS                  BIT(1)
#define      MVNETA_TX_FIFO_EMPTY                BIT(8)
#define MVNETA_RX_MIN_FRAME_SIZE                 0x247c
#define MVNETA_SERDES_CFG			 0x24A0
#define      MVNETA_SGMII_SERDES_PROTO		 0x0cc7
#define      MVNETA_QSGMII_SERDES_PROTO		 0x0667
#define MVNETA_TYPE_PRIO                         0x24bc
#define      MVNETA_FORCE_UNI                    BIT(21)
#define MVNETA_TXQ_CMD_1                         0x24e4
#define MVNETA_TXQ_CMD                           0x2448
#define      MVNETA_TXQ_DISABLE_SHIFT            8
#define      MVNETA_TXQ_ENABLE_MASK              0x000000ff
#define MVNETA_ACC_MODE                          0x2500
#define MVNETA_CPU_MAP(cpu)                      (0x2540 + ((cpu) << 2))
#define      MVNETA_CPU_RXQ_ACCESS_ALL_MASK      0x000000ff
#define      MVNETA_CPU_TXQ_ACCESS_ALL_MASK      0x0000ff00
#define MVNETA_RXQ_TIME_COAL_REG(q)              (0x2580 + ((q) << 2))

/* Exception Interrupt Port/Queue Cause register */

#define MVNETA_INTR_NEW_CAUSE                    0x25a0
#define MVNETA_INTR_NEW_MASK                     0x25a4

/* bits  0..7  = TXQ SENT, one bit per queue.
 * bits  8..15 = RXQ OCCUP, one bit per queue.
 * bits 16..23 = RXQ FREE, one bit per queue.
 * bit  29 = OLD_REG_SUM, see old reg ?
 * bit  30 = TX_ERR_SUM, one bit for 4 ports
 * bit  31 = MISC_SUM,   one bit for 4 ports
 */
#define      MVNETA_TX_INTR_MASK(nr_txqs)        (((1 << nr_txqs) - 1) << 0)
#define      MVNETA_TX_INTR_MASK_ALL             (0xff << 0)
#define      MVNETA_RX_INTR_MASK(nr_rxqs)        (((1 << nr_rxqs) - 1) << 8)
#define      MVNETA_RX_INTR_MASK_ALL             (0xff << 8)

#define MVNETA_INTR_OLD_CAUSE                    0x25a8
#define MVNETA_INTR_OLD_MASK                     0x25ac

/* Data Path Port/Queue Cause Register */
#define MVNETA_INTR_MISC_CAUSE                   0x25b0
#define MVNETA_INTR_MISC_MASK                    0x25b4
#define MVNETA_INTR_ENABLE                       0x25b8

#define MVNETA_RXQ_CMD                           0x2680
#define      MVNETA_RXQ_DISABLE_SHIFT            8
#define      MVNETA_RXQ_ENABLE_MASK              0x000000ff
#define MVETH_TXQ_TOKEN_COUNT_REG(q)             (0x2700 + ((q) << 4))
#define MVETH_TXQ_TOKEN_CFG_REG(q)               (0x2704 + ((q) << 4))
#define MVNETA_GMAC_CTRL_0                       0x2c00
#define      MVNETA_GMAC_MAX_RX_SIZE_SHIFT       2
#define      MVNETA_GMAC_MAX_RX_SIZE_MASK        0x7ffc
#define      MVNETA_GMAC0_PORT_ENABLE            BIT(0)
#define MVNETA_GMAC_CTRL_2                       0x2c08
#define      MVNETA_GMAC2_PCS_ENABLE             BIT(3)
#define      MVNETA_GMAC2_PORT_RGMII             BIT(4)
#define      MVNETA_GMAC2_PORT_RESET             BIT(6)
#define MVNETA_GMAC_STATUS                       0x2c10
#define      MVNETA_GMAC_LINK_UP                 BIT(0)
#define      MVNETA_GMAC_SPEED_1000              BIT(1)
#define      MVNETA_GMAC_SPEED_100               BIT(2)
#define      MVNETA_GMAC_FULL_DUPLEX             BIT(3)
#define      MVNETA_GMAC_RX_FLOW_CTRL_ENABLE     BIT(4)
#define      MVNETA_GMAC_TX_FLOW_CTRL_ENABLE     BIT(5)
#define      MVNETA_GMAC_RX_FLOW_CTRL_ACTIVE     BIT(6)
#define      MVNETA_GMAC_TX_FLOW_CTRL_ACTIVE     BIT(7)
#define MVNETA_GMAC_AUTONEG_CONFIG               0x2c0c
#define      MVNETA_GMAC_FORCE_LINK_DOWN         BIT(0)
#define      MVNETA_GMAC_FORCE_LINK_PASS         BIT(1)
#define      MVNETA_GMAC_FORCE_LINK_UP           (BIT(0) | BIT(1))
#define      MVNETA_GMAC_IB_BYPASS_AN_EN         BIT(3)
#define      MVNETA_GMAC_CONFIG_MII_SPEED        BIT(5)
#define      MVNETA_GMAC_CONFIG_GMII_SPEED       BIT(6)
#define      MVNETA_GMAC_AN_SPEED_EN             BIT(7)
#define      MVNETA_GMAC_SET_FC_EN               BIT(8)
#define      MVNETA_GMAC_ADVERT_FC_EN            BIT(9)
#define      MVNETA_GMAC_CONFIG_FULL_DUPLEX      BIT(12)
#define      MVNETA_GMAC_AN_DUPLEX_EN            BIT(13)
#define      MVNETA_GMAC_SAMPLE_TX_CFG_EN        BIT(15)
#define MVNETA_MIB_COUNTERS_BASE                 0x3080
#define      MVNETA_MIB_LATE_COLLISION           0x7c
#define MVNETA_DA_FILT_SPEC_MCAST                0x3400
#define MVNETA_DA_FILT_OTH_MCAST                 0x3500
#define MVNETA_DA_FILT_UCAST_BASE                0x3600
#define MVNETA_TXQ_BASE_ADDR_REG(q)              (0x3c00 + ((q) << 2))
#define MVNETA_TXQ_SIZE_REG(q)                   (0x3c20 + ((q) << 2))
#define      MVNETA_TXQ_SENT_THRESH_ALL_MASK     0x3fff0000
#define      MVNETA_TXQ_SENT_THRESH_MASK(coal)   ((coal) << 16)
#define MVNETA_TXQ_UPDATE_REG(q)                 (0x3c60 + ((q) << 2))
#define      MVNETA_TXQ_DEC_SENT_SHIFT           16
#define MVNETA_TXQ_STATUS_REG(q)                 (0x3c40 + ((q) << 2))
#define      MVNETA_TXQ_SENT_DESC_SHIFT          16
#define      MVNETA_TXQ_SENT_DESC_MASK           0x3fff0000
#define MVNETA_PORT_TX_RESET                     0x3cf0
#define      MVNETA_PORT_TX_DMA_RESET            BIT(0)
#define MVNETA_TX_MTU                            0x3e0c
#define MVNETA_TX_TOKEN_SIZE                     0x3e14
#define      MVNETA_TX_TOKEN_SIZE_MAX            0xffffffff
#define MVNETA_TXQ_TOKEN_SIZE_REG(q)             (0x3e40 + ((q) << 2))
#define      MVNETA_TXQ_TOKEN_SIZE_MAX           0x7fffffff

/* Descriptor ring Macros */
#define MVNETA_QUEUE_NEXT_DESC(q, index)	\
	(((index) < (q)->last_desc) ? ((index) + 1) : 0)

/* Various constants */

/* Coalescing */
#define MVNETA_TXDONE_COAL_PKTS		16
#define MVNETA_RX_COAL_PKTS		32
#define MVNETA_RX_COAL_USEC		100

/* The two bytes Marvell header. Either contains a special value used
 * by Marvell switches when a specific hardware mode is enabled (not
 * supported by this driver) or is filled automatically by zeroes on
 * the RX side. Those two bytes being at the front of the Ethernet
 * header, they allow to have the IP header aligned on a 4 bytes
 * boundary automatically: the hardware skips those two bytes on its
 * own.
 */
#define MVNETA_MH_SIZE			2

#define MVNETA_VLAN_TAG_LEN             4

#define MVNETA_CPU_D_CACHE_LINE_SIZE    32
#define MVNETA_TX_CSUM_MAX_SIZE		9800
#define MVNETA_ACC_MODE_EXT		1

/* Timeout constants */
#define MVNETA_TX_DISABLE_TIMEOUT_MSEC	1000
#define MVNETA_RX_DISABLE_TIMEOUT_MSEC	1000
#define MVNETA_TX_FIFO_EMPTY_TIMEOUT	10000

#define MVNETA_TX_MTU_MAX		0x3ffff

/* Max number of Rx descriptors */
#define MVNETA_MAX_RXD 16

/* Max number of Tx descriptors */
#define MVNETA_MAX_TXD 16

/* descriptor aligned size */
#define MVNETA_DESC_ALIGNED_SIZE	32

struct mvneta_port {
	void __iomem *base;
	struct mvneta_rx_queue *rxqs;
	struct mvneta_tx_queue *txqs;

	u8 mcast_count[256];
	u16 tx_ring_size;
	u16 rx_ring_size;

	phy_interface_t phy_interface;
	unsigned int link;
	unsigned int duplex;
	unsigned int speed;

	int init;
	int phyaddr;
	struct phy_device *phydev;
#ifdef CONFIG_DM_GPIO
	struct gpio_desc phy_reset_gpio;
#endif
	struct mii_dev *bus;
};

/* The mvneta_tx_desc and mvneta_rx_desc structures describe the
 * layout of the transmit and reception DMA descriptors, and their
 * layout is therefore defined by the hardware design
 */

#define MVNETA_TX_L3_OFF_SHIFT	0
#define MVNETA_TX_IP_HLEN_SHIFT	8
#define MVNETA_TX_L4_UDP	BIT(16)
#define MVNETA_TX_L3_IP6	BIT(17)
#define MVNETA_TXD_IP_CSUM	BIT(18)
#define MVNETA_TXD_Z_PAD	BIT(19)
#define MVNETA_TXD_L_DESC	BIT(20)
#define MVNETA_TXD_F_DESC	BIT(21)
#define MVNETA_TXD_FLZ_DESC	(MVNETA_TXD_Z_PAD  | \
				 MVNETA_TXD_L_DESC | \
				 MVNETA_TXD_F_DESC)
#define MVNETA_TX_L4_CSUM_FULL	BIT(30)
#define MVNETA_TX_L4_CSUM_NOT	BIT(31)

#define MVNETA_RXD_ERR_CRC		0x0
#define MVNETA_RXD_ERR_SUMMARY		BIT(16)
#define MVNETA_RXD_ERR_OVERRUN		BIT(17)
#define MVNETA_RXD_ERR_LEN		BIT(18)
#define MVNETA_RXD_ERR_RESOURCE		(BIT(17) | BIT(18))
#define MVNETA_RXD_ERR_CODE_MASK	(BIT(17) | BIT(18))
#define MVNETA_RXD_L3_IP4		BIT(25)
#define MVNETA_RXD_FIRST_LAST_DESC	(BIT(26) | BIT(27))
#define MVNETA_RXD_L4_CSUM_OK		BIT(30)

struct mvneta_tx_desc {
	u32  command;		/* Options used by HW for packet transmitting.*/
	u16  reserverd1;	/* csum_l4 (for future use)		*/
	u16  data_size;		/* Data size of transmitted packet in bytes */
	u32  buf_phys_addr;	/* Physical addr of transmitted buffer	*/
	u32  reserved2;		/* hw_cmd - (for future use, PMT)	*/
	u32  reserved3[4];	/* Reserved - (for future use)		*/
};

struct mvneta_rx_desc {
	u32  status;		/* Info about received packet		*/
	u16  reserved1;		/* pnc_info - (for future use, PnC)	*/
	u16  data_size;		/* Size of received packet in bytes	*/

	u32  buf_phys_addr;	/* Physical address of the buffer	*/
	u32  reserved2;		/* pnc_flow_id  (for future use, PnC)	*/

	u32  buf_cookie;	/* cookie for access to RX buffer in rx path */
	u16  reserved3;		/* prefetch_cmd, for future use		*/
	u16  reserved4;		/* csum_l4 - (for future use, PnC)	*/

	u32  reserved5;		/* pnc_extra PnC (for future use, PnC)	*/
	u32  reserved6;		/* hw_cmd (for future use, PnC and HWF)	*/
};

struct mvneta_tx_queue {
	/* Number of this TX queue, in the range 0-7 */
	u8 id;

	/* Number of TX DMA descriptors in the descriptor ring */
	int size;

	/* Index of last TX DMA descriptor that was inserted */
	int txq_put_index;

	/* Index of the TX DMA descriptor to be cleaned up */
	int txq_get_index;

	/* Virtual address of the TX DMA descriptors array */
	struct mvneta_tx_desc *descs;

	/* DMA address of the TX DMA descriptors array */
	dma_addr_t descs_phys;

	/* Index of the last TX DMA descriptor */
	int last_desc;

	/* Index of the next TX DMA descriptor to process */
	int next_desc_to_proc;
};

struct mvneta_rx_queue {
	/* rx queue number, in the range 0-7 */
	u8 id;

	/* num of rx descriptors in the rx descriptor ring */
	int size;

	/* Virtual address of the RX DMA descriptors array */
	struct mvneta_rx_desc *descs;

	/* DMA address of the RX DMA descriptors array */
	dma_addr_t descs_phys;

	/* Index of the last RX DMA descriptor */
	int last_desc;

	/* Index of the next RX DMA descriptor to process */
	int next_desc_to_proc;
};

/* U-Boot doesn't use the queues, so set the number to 1 */
static int rxq_number = 1;
static int txq_number = 1;
static int rxq_def;

struct buffer_location {
	struct mvneta_tx_desc *tx_descs;
	struct mvneta_rx_desc *rx_descs;
	u32 rx_buffers;
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

/*
 * Dummy implementation that can be overwritten by a board
 * specific function
 */
__weak int board_network_enable(struct mii_dev *bus)
{
	return 0;
}

/* Utility/helper methods */

/* Write helper method */
static void mvreg_write(struct mvneta_port *pp, u32 offset, u32 data)
{
	writel(data, pp->base + offset);
}

/* Read helper method */
static u32 mvreg_read(struct mvneta_port *pp, u32 offset)
{
	return readl(pp->base + offset);
}

/* Clear all MIB counters */
static void mvneta_mib_counters_clear(struct mvneta_port *pp)
{
	int i;

	/* Perform dummy reads from MIB counters */
	for (i = 0; i < MVNETA_MIB_LATE_COLLISION; i += 4)
		mvreg_read(pp, (MVNETA_MIB_COUNTERS_BASE + i));
}

/* Rx descriptors helper methods */

/* Checks whether the RX descriptor having this status is both the first
 * and the last descriptor for the RX packet. Each RX packet is currently
 * received through a single RX descriptor, so not having each RX
 * descriptor with its first and last bits set is an error
 */
static int mvneta_rxq_desc_is_first_last(u32 status)
{
	return (status & MVNETA_RXD_FIRST_LAST_DESC) ==
		MVNETA_RXD_FIRST_LAST_DESC;
}

/* Add number of descriptors ready to receive new packets */
static void mvneta_rxq_non_occup_desc_add(struct mvneta_port *pp,
					  struct mvneta_rx_queue *rxq,
					  int ndescs)
{
	/* Only MVNETA_RXQ_ADD_NON_OCCUPIED_MAX (255) descriptors can
	 * be added at once
	 */
	while (ndescs > MVNETA_RXQ_ADD_NON_OCCUPIED_MAX) {
		mvreg_write(pp, MVNETA_RXQ_STATUS_UPDATE_REG(rxq->id),
			    (MVNETA_RXQ_ADD_NON_OCCUPIED_MAX <<
			     MVNETA_RXQ_ADD_NON_OCCUPIED_SHIFT));
		ndescs -= MVNETA_RXQ_ADD_NON_OCCUPIED_MAX;
	}

	mvreg_write(pp, MVNETA_RXQ_STATUS_UPDATE_REG(rxq->id),
		    (ndescs << MVNETA_RXQ_ADD_NON_OCCUPIED_SHIFT));
}

/* Get number of RX descriptors occupied by received packets */
static int mvneta_rxq_busy_desc_num_get(struct mvneta_port *pp,
					struct mvneta_rx_queue *rxq)
{
	u32 val;

	val = mvreg_read(pp, MVNETA_RXQ_STATUS_REG(rxq->id));
	return val & MVNETA_RXQ_OCCUPIED_ALL_MASK;
}

/* Update num of rx desc called upon return from rx path or
 * from mvneta_rxq_drop_pkts().
 */
static void mvneta_rxq_desc_num_update(struct mvneta_port *pp,
				       struct mvneta_rx_queue *rxq,
				       int rx_done, int rx_filled)
{
	u32 val;

	if ((rx_done <= 0xff) && (rx_filled <= 0xff)) {
		val = rx_done |
		  (rx_filled << MVNETA_RXQ_ADD_NON_OCCUPIED_SHIFT);
		mvreg_write(pp, MVNETA_RXQ_STATUS_UPDATE_REG(rxq->id), val);
		return;
	}

	/* Only 255 descriptors can be added at once */
	while ((rx_done > 0) || (rx_filled > 0)) {
		if (rx_done <= 0xff) {
			val = rx_done;
			rx_done = 0;
		} else {
			val = 0xff;
			rx_done -= 0xff;
		}
		if (rx_filled <= 0xff) {
			val |= rx_filled << MVNETA_RXQ_ADD_NON_OCCUPIED_SHIFT;
			rx_filled = 0;
		} else {
			val |= 0xff << MVNETA_RXQ_ADD_NON_OCCUPIED_SHIFT;
			rx_filled -= 0xff;
		}
		mvreg_write(pp, MVNETA_RXQ_STATUS_UPDATE_REG(rxq->id), val);
	}
}

/* Get pointer to next RX descriptor to be processed by SW */
static struct mvneta_rx_desc *
mvneta_rxq_next_desc_get(struct mvneta_rx_queue *rxq)
{
	int rx_desc = rxq->next_desc_to_proc;

	rxq->next_desc_to_proc = MVNETA_QUEUE_NEXT_DESC(rxq, rx_desc);
	return rxq->descs + rx_desc;
}

/* Tx descriptors helper methods */

/* Update HW with number of TX descriptors to be sent */
static void mvneta_txq_pend_desc_add(struct mvneta_port *pp,
				     struct mvneta_tx_queue *txq,
				     int pend_desc)
{
	u32 val;

	/* Only 255 descriptors can be added at once ; Assume caller
	 * process TX descriptors in quanta less than 256
	 */
	val = pend_desc;
	mvreg_write(pp, MVNETA_TXQ_UPDATE_REG(txq->id), val);
}

/* Get pointer to next TX descriptor to be processed (send) by HW */
static struct mvneta_tx_desc *
mvneta_txq_next_desc_get(struct mvneta_tx_queue *txq)
{
	int tx_desc = txq->next_desc_to_proc;

	txq->next_desc_to_proc = MVNETA_QUEUE_NEXT_DESC(txq, tx_desc);
	return txq->descs + tx_desc;
}

/* Set rxq buf size */
static void mvneta_rxq_buf_size_set(struct mvneta_port *pp,
				    struct mvneta_rx_queue *rxq,
				    int buf_size)
{
	u32 val;

	val = mvreg_read(pp, MVNETA_RXQ_SIZE_REG(rxq->id));

	val &= ~MVNETA_RXQ_BUF_SIZE_MASK;
	val |= ((buf_size >> 3) << MVNETA_RXQ_BUF_SIZE_SHIFT);

	mvreg_write(pp, MVNETA_RXQ_SIZE_REG(rxq->id), val);
}

static int mvneta_port_is_fixed_link(struct mvneta_port *pp)
{
	/* phy_addr is set to invalid value for fixed link */
	return pp->phyaddr > PHY_MAX_ADDR;
}


/* Start the Ethernet port RX and TX activity */
static void mvneta_port_up(struct mvneta_port *pp)
{
	int queue;
	u32 q_map;

	/* Enable all initialized TXs. */
	mvneta_mib_counters_clear(pp);
	q_map = 0;
	for (queue = 0; queue < txq_number; queue++) {
		struct mvneta_tx_queue *txq = &pp->txqs[queue];
		if (txq->descs != NULL)
			q_map |= (1 << queue);
	}
	mvreg_write(pp, MVNETA_TXQ_CMD, q_map);

	/* Enable all initialized RXQs. */
	q_map = 0;
	for (queue = 0; queue < rxq_number; queue++) {
		struct mvneta_rx_queue *rxq = &pp->rxqs[queue];
		if (rxq->descs != NULL)
			q_map |= (1 << queue);
	}
	mvreg_write(pp, MVNETA_RXQ_CMD, q_map);
}

/* Stop the Ethernet port activity */
static void mvneta_port_down(struct mvneta_port *pp)
{
	u32 val;
	int count;

	/* Stop Rx port activity. Check port Rx activity. */
	val = mvreg_read(pp, MVNETA_RXQ_CMD) & MVNETA_RXQ_ENABLE_MASK;

	/* Issue stop command for active channels only */
	if (val != 0)
		mvreg_write(pp, MVNETA_RXQ_CMD,
			    val << MVNETA_RXQ_DISABLE_SHIFT);

	/* Wait for all Rx activity to terminate. */
	count = 0;
	do {
		if (count++ >= MVNETA_RX_DISABLE_TIMEOUT_MSEC) {
			netdev_warn(pp->dev,
				    "TIMEOUT for RX stopped ! rx_queue_cmd: 0x08%x\n",
				    val);
			break;
		}
		mdelay(1);

		val = mvreg_read(pp, MVNETA_RXQ_CMD);
	} while (val & 0xff);

	/* Stop Tx port activity. Check port Tx activity. Issue stop
	 * command for active channels only
	 */
	val = (mvreg_read(pp, MVNETA_TXQ_CMD)) & MVNETA_TXQ_ENABLE_MASK;

	if (val != 0)
		mvreg_write(pp, MVNETA_TXQ_CMD,
			    (val << MVNETA_TXQ_DISABLE_SHIFT));

	/* Wait for all Tx activity to terminate. */
	count = 0;
	do {
		if (count++ >= MVNETA_TX_DISABLE_TIMEOUT_MSEC) {
			netdev_warn(pp->dev,
				    "TIMEOUT for TX stopped status=0x%08x\n",
				    val);
			break;
		}
		mdelay(1);

		/* Check TX Command reg that all Txqs are stopped */
		val = mvreg_read(pp, MVNETA_TXQ_CMD);

	} while (val & 0xff);

	/* Double check to verify that TX FIFO is empty */
	count = 0;
	do {
		if (count++ >= MVNETA_TX_FIFO_EMPTY_TIMEOUT) {
			netdev_warn(pp->dev,
				    "TX FIFO empty timeout status=0x08%x\n",
				    val);
			break;
		}
		mdelay(1);

		val = mvreg_read(pp, MVNETA_PORT_STATUS);
	} while (!(val & MVNETA_TX_FIFO_EMPTY) &&
		 (val & MVNETA_TX_IN_PRGRS));

	udelay(200);
}

/* Enable the port by setting the port enable bit of the MAC control register */
static void mvneta_port_enable(struct mvneta_port *pp)
{
	u32 val;

	/* Enable port */
	val = mvreg_read(pp, MVNETA_GMAC_CTRL_0);
	val |= MVNETA_GMAC0_PORT_ENABLE;
	mvreg_write(pp, MVNETA_GMAC_CTRL_0, val);
}

/* Disable the port and wait for about 200 usec before retuning */
static void mvneta_port_disable(struct mvneta_port *pp)
{
	u32 val;

	/* Reset the Enable bit in the Serial Control Register */
	val = mvreg_read(pp, MVNETA_GMAC_CTRL_0);
	val &= ~MVNETA_GMAC0_PORT_ENABLE;
	mvreg_write(pp, MVNETA_GMAC_CTRL_0, val);

	udelay(200);
}

/* Multicast tables methods */

/* Set all entries in Unicast MAC Table; queue==-1 means reject all */
static void mvneta_set_ucast_table(struct mvneta_port *pp, int queue)
{
	int offset;
	u32 val;

	if (queue == -1) {
		val = 0;
	} else {
		val = 0x1 | (queue << 1);
		val |= (val << 24) | (val << 16) | (val << 8);
	}

	for (offset = 0; offset <= 0xc; offset += 4)
		mvreg_write(pp, MVNETA_DA_FILT_UCAST_BASE + offset, val);
}

/* Set all entries in Special Multicast MAC Table; queue==-1 means reject all */
static void mvneta_set_special_mcast_table(struct mvneta_port *pp, int queue)
{
	int offset;
	u32 val;

	if (queue == -1) {
		val = 0;
	} else {
		val = 0x1 | (queue << 1);
		val |= (val << 24) | (val << 16) | (val << 8);
	}

	for (offset = 0; offset <= 0xfc; offset += 4)
		mvreg_write(pp, MVNETA_DA_FILT_SPEC_MCAST + offset, val);
}

/* Set all entries in Other Multicast MAC Table. queue==-1 means reject all */
static void mvneta_set_other_mcast_table(struct mvneta_port *pp, int queue)
{
	int offset;
	u32 val;

	if (queue == -1) {
		memset(pp->mcast_count, 0, sizeof(pp->mcast_count));
		val = 0;
	} else {
		memset(pp->mcast_count, 1, sizeof(pp->mcast_count));
		val = 0x1 | (queue << 1);
		val |= (val << 24) | (val << 16) | (val << 8);
	}

	for (offset = 0; offset <= 0xfc; offset += 4)
		mvreg_write(pp, MVNETA_DA_FILT_OTH_MCAST + offset, val);
}

/* This method sets defaults to the NETA port:
 *	Clears interrupt Cause and Mask registers.
 *	Clears all MAC tables.
 *	Sets defaults to all registers.
 *	Resets RX and TX descriptor rings.
 *	Resets PHY.
 * This method can be called after mvneta_port_down() to return the port
 *	settings to defaults.
 */
static void mvneta_defaults_set(struct mvneta_port *pp)
{
	int cpu;
	int queue;
	u32 val;

	/* Clear all Cause registers */
	mvreg_write(pp, MVNETA_INTR_NEW_CAUSE, 0);
	mvreg_write(pp, MVNETA_INTR_OLD_CAUSE, 0);
	mvreg_write(pp, MVNETA_INTR_MISC_CAUSE, 0);

	/* Mask all interrupts */
	mvreg_write(pp, MVNETA_INTR_NEW_MASK, 0);
	mvreg_write(pp, MVNETA_INTR_OLD_MASK, 0);
	mvreg_write(pp, MVNETA_INTR_MISC_MASK, 0);
	mvreg_write(pp, MVNETA_INTR_ENABLE, 0);

	/* Enable MBUS Retry bit16 */
	mvreg_write(pp, MVNETA_MBUS_RETRY, 0x20);

	/* Set CPU queue access map - all CPUs have access to all RX
	 * queues and to all TX queues
	 */
	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++)
		mvreg_write(pp, MVNETA_CPU_MAP(cpu),
			    (MVNETA_CPU_RXQ_ACCESS_ALL_MASK |
			     MVNETA_CPU_TXQ_ACCESS_ALL_MASK));

	/* Reset RX and TX DMAs */
	mvreg_write(pp, MVNETA_PORT_RX_RESET, MVNETA_PORT_RX_DMA_RESET);
	mvreg_write(pp, MVNETA_PORT_TX_RESET, MVNETA_PORT_TX_DMA_RESET);

	/* Disable Legacy WRR, Disable EJP, Release from reset */
	mvreg_write(pp, MVNETA_TXQ_CMD_1, 0);
	for (queue = 0; queue < txq_number; queue++) {
		mvreg_write(pp, MVETH_TXQ_TOKEN_COUNT_REG(queue), 0);
		mvreg_write(pp, MVETH_TXQ_TOKEN_CFG_REG(queue), 0);
	}

	mvreg_write(pp, MVNETA_PORT_TX_RESET, 0);
	mvreg_write(pp, MVNETA_PORT_RX_RESET, 0);

	/* Set Port Acceleration Mode */
	val = MVNETA_ACC_MODE_EXT;
	mvreg_write(pp, MVNETA_ACC_MODE, val);

	/* Update val of portCfg register accordingly with all RxQueue types */
	val = MVNETA_PORT_CONFIG_DEFL_VALUE(rxq_def);
	mvreg_write(pp, MVNETA_PORT_CONFIG, val);

	val = 0;
	mvreg_write(pp, MVNETA_PORT_CONFIG_EXTEND, val);
	mvreg_write(pp, MVNETA_RX_MIN_FRAME_SIZE, 64);

	/* Build PORT_SDMA_CONFIG_REG */
	val = 0;

	/* Default burst size */
	val |= MVNETA_TX_BRST_SZ_MASK(MVNETA_SDMA_BRST_SIZE_16);
	val |= MVNETA_RX_BRST_SZ_MASK(MVNETA_SDMA_BRST_SIZE_16);
	val |= MVNETA_RX_NO_DATA_SWAP | MVNETA_TX_NO_DATA_SWAP;

	/* Assign port SDMA configuration */
	mvreg_write(pp, MVNETA_SDMA_CONFIG, val);

	/* Enable PHY polling in hardware if not in fixed-link mode */
	if (!mvneta_port_is_fixed_link(pp)) {
		val = mvreg_read(pp, MVNETA_UNIT_CONTROL);
		val |= MVNETA_PHY_POLLING_ENABLE;
		mvreg_write(pp, MVNETA_UNIT_CONTROL, val);
	}

	mvneta_set_ucast_table(pp, -1);
	mvneta_set_special_mcast_table(pp, -1);
	mvneta_set_other_mcast_table(pp, -1);
}

/* Set unicast address */
static void mvneta_set_ucast_addr(struct mvneta_port *pp, u8 last_nibble,
				  int queue)
{
	unsigned int unicast_reg;
	unsigned int tbl_offset;
	unsigned int reg_offset;

	/* Locate the Unicast table entry */
	last_nibble = (0xf & last_nibble);

	/* offset from unicast tbl base */
	tbl_offset = (last_nibble / 4) * 4;

	/* offset within the above reg  */
	reg_offset = last_nibble % 4;

	unicast_reg = mvreg_read(pp, (MVNETA_DA_FILT_UCAST_BASE + tbl_offset));

	if (queue == -1) {
		/* Clear accepts frame bit at specified unicast DA tbl entry */
		unicast_reg &= ~(0xff << (8 * reg_offset));
	} else {
		unicast_reg &= ~(0xff << (8 * reg_offset));
		unicast_reg |= ((0x01 | (queue << 1)) << (8 * reg_offset));
	}

	mvreg_write(pp, (MVNETA_DA_FILT_UCAST_BASE + tbl_offset), unicast_reg);
}

/* Set mac address */
static void mvneta_mac_addr_set(struct mvneta_port *pp, unsigned char *addr,
				int queue)
{
	unsigned int mac_h;
	unsigned int mac_l;

	if (queue != -1) {
		mac_l = (addr[4] << 8) | (addr[5]);
		mac_h = (addr[0] << 24) | (addr[1] << 16) |
			(addr[2] << 8) | (addr[3] << 0);

		mvreg_write(pp, MVNETA_MAC_ADDR_LOW, mac_l);
		mvreg_write(pp, MVNETA_MAC_ADDR_HIGH, mac_h);
	}

	/* Accept frames of this address */
	mvneta_set_ucast_addr(pp, addr[5], queue);
}

static int mvneta_write_hwaddr(struct udevice *dev)
{
	mvneta_mac_addr_set(dev_get_priv(dev),
		((struct eth_pdata *)dev_get_platdata(dev))->enetaddr,
		rxq_def);

	return 0;
}

/* Handle rx descriptor fill by setting buf_cookie and buf_phys_addr */
static void mvneta_rx_desc_fill(struct mvneta_rx_desc *rx_desc,
				u32 phys_addr, u32 cookie)
{
	rx_desc->buf_cookie = cookie;
	rx_desc->buf_phys_addr = phys_addr;
}

/* Decrement sent descriptors counter */
static void mvneta_txq_sent_desc_dec(struct mvneta_port *pp,
				     struct mvneta_tx_queue *txq,
				     int sent_desc)
{
	u32 val;

	/* Only 255 TX descriptors can be updated at once */
	while (sent_desc > 0xff) {
		val = 0xff << MVNETA_TXQ_DEC_SENT_SHIFT;
		mvreg_write(pp, MVNETA_TXQ_UPDATE_REG(txq->id), val);
		sent_desc = sent_desc - 0xff;
	}

	val = sent_desc << MVNETA_TXQ_DEC_SENT_SHIFT;
	mvreg_write(pp, MVNETA_TXQ_UPDATE_REG(txq->id), val);
}

/* Get number of TX descriptors already sent by HW */
static int mvneta_txq_sent_desc_num_get(struct mvneta_port *pp,
					struct mvneta_tx_queue *txq)
{
	u32 val;
	int sent_desc;

	val = mvreg_read(pp, MVNETA_TXQ_STATUS_REG(txq->id));
	sent_desc = (val & MVNETA_TXQ_SENT_DESC_MASK) >>
		MVNETA_TXQ_SENT_DESC_SHIFT;

	return sent_desc;
}

/* Display more error info */
static void mvneta_rx_error(struct mvneta_port *pp,
			    struct mvneta_rx_desc *rx_desc)
{
	u32 status = rx_desc->status;

	if (!mvneta_rxq_desc_is_first_last(status)) {
		netdev_err(pp->dev,
			   "bad rx status %08x (buffer oversize), size=%d\n",
			   status, rx_desc->data_size);
		return;
	}

	switch (status & MVNETA_RXD_ERR_CODE_MASK) {
	case MVNETA_RXD_ERR_CRC:
		netdev_err(pp->dev, "bad rx status %08x (crc error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	case MVNETA_RXD_ERR_OVERRUN:
		netdev_err(pp->dev, "bad rx status %08x (overrun error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	case MVNETA_RXD_ERR_LEN:
		netdev_err(pp->dev, "bad rx status %08x (max frame length error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	case MVNETA_RXD_ERR_RESOURCE:
		netdev_err(pp->dev, "bad rx status %08x (resource error), size=%d\n",
			   status, rx_desc->data_size);
		break;
	}
}

static struct mvneta_rx_queue *mvneta_rxq_handle_get(struct mvneta_port *pp,
						     int rxq)
{
	return &pp->rxqs[rxq];
}


/* Drop packets received by the RXQ and free buffers */
static void mvneta_rxq_drop_pkts(struct mvneta_port *pp,
				 struct mvneta_rx_queue *rxq)
{
	int rx_done;

	rx_done = mvneta_rxq_busy_desc_num_get(pp, rxq);
	if (rx_done)
		mvneta_rxq_desc_num_update(pp, rxq, rx_done, rx_done);
}

/* Handle rxq fill: allocates rxq skbs; called when initializing a port */
static int mvneta_rxq_fill(struct mvneta_port *pp, struct mvneta_rx_queue *rxq,
			   int num)
{
	int i;

	for (i = 0; i < num; i++) {
		u32 addr;

		/* U-Boot special: Fill in the rx buffer addresses */
		addr = buffer_loc.rx_buffers + (i * RX_BUFFER_SIZE);
		mvneta_rx_desc_fill(rxq->descs + i, addr, addr);
	}

	/* Add this number of RX descriptors as non occupied (ready to
	 * get packets)
	 */
	mvneta_rxq_non_occup_desc_add(pp, rxq, i);

	return 0;
}

/* Rx/Tx queue initialization/cleanup methods */

/* Create a specified RX queue */
static int mvneta_rxq_init(struct mvneta_port *pp,
			   struct mvneta_rx_queue *rxq)

{
	rxq->size = pp->rx_ring_size;

	/* Allocate memory for RX descriptors */
	rxq->descs_phys = (dma_addr_t)rxq->descs;
	if (rxq->descs == NULL)
		return -ENOMEM;

	WARN_ON(rxq->descs != PTR_ALIGN(rxq->descs, ARCH_DMA_MINALIGN));

	rxq->last_desc = rxq->size - 1;

	/* Set Rx descriptors queue starting address */
	mvreg_write(pp, MVNETA_RXQ_BASE_ADDR_REG(rxq->id), rxq->descs_phys);
	mvreg_write(pp, MVNETA_RXQ_SIZE_REG(rxq->id), rxq->size);

	/* Fill RXQ with buffers from RX pool */
	mvneta_rxq_buf_size_set(pp, rxq, RX_BUFFER_SIZE);
	mvneta_rxq_fill(pp, rxq, rxq->size);

	return 0;
}

/* Cleanup Rx queue */
static void mvneta_rxq_deinit(struct mvneta_port *pp,
			      struct mvneta_rx_queue *rxq)
{
	mvneta_rxq_drop_pkts(pp, rxq);

	rxq->descs             = NULL;
	rxq->last_desc         = 0;
	rxq->next_desc_to_proc = 0;
	rxq->descs_phys        = 0;
}

/* Create and initialize a tx queue */
static int mvneta_txq_init(struct mvneta_port *pp,
			   struct mvneta_tx_queue *txq)
{
	txq->size = pp->tx_ring_size;

	/* Allocate memory for TX descriptors */
	txq->descs_phys = (dma_addr_t)txq->descs;
	if (txq->descs == NULL)
		return -ENOMEM;

	WARN_ON(txq->descs != PTR_ALIGN(txq->descs, ARCH_DMA_MINALIGN));

	txq->last_desc = txq->size - 1;

	/* Set maximum bandwidth for enabled TXQs */
	mvreg_write(pp, MVETH_TXQ_TOKEN_CFG_REG(txq->id), 0x03ffffff);
	mvreg_write(pp, MVETH_TXQ_TOKEN_COUNT_REG(txq->id), 0x3fffffff);

	/* Set Tx descriptors queue starting address */
	mvreg_write(pp, MVNETA_TXQ_BASE_ADDR_REG(txq->id), txq->descs_phys);
	mvreg_write(pp, MVNETA_TXQ_SIZE_REG(txq->id), txq->size);

	return 0;
}

/* Free allocated resources when mvneta_txq_init() fails to allocate memory*/
static void mvneta_txq_deinit(struct mvneta_port *pp,
			      struct mvneta_tx_queue *txq)
{
	txq->descs             = NULL;
	txq->last_desc         = 0;
	txq->next_desc_to_proc = 0;
	txq->descs_phys        = 0;

	/* Set minimum bandwidth for disabled TXQs */
	mvreg_write(pp, MVETH_TXQ_TOKEN_CFG_REG(txq->id), 0);
	mvreg_write(pp, MVETH_TXQ_TOKEN_COUNT_REG(txq->id), 0);

	/* Set Tx descriptors queue starting address and size */
	mvreg_write(pp, MVNETA_TXQ_BASE_ADDR_REG(txq->id), 0);
	mvreg_write(pp, MVNETA_TXQ_SIZE_REG(txq->id), 0);
}

/* Cleanup all Tx queues */
static void mvneta_cleanup_txqs(struct mvneta_port *pp)
{
	int queue;

	for (queue = 0; queue < txq_number; queue++)
		mvneta_txq_deinit(pp, &pp->txqs[queue]);
}

/* Cleanup all Rx queues */
static void mvneta_cleanup_rxqs(struct mvneta_port *pp)
{
	int queue;

	for (queue = 0; queue < rxq_number; queue++)
		mvneta_rxq_deinit(pp, &pp->rxqs[queue]);
}


/* Init all Rx queues */
static int mvneta_setup_rxqs(struct mvneta_port *pp)
{
	int queue;

	for (queue = 0; queue < rxq_number; queue++) {
		int err = mvneta_rxq_init(pp, &pp->rxqs[queue]);
		if (err) {
			netdev_err(pp->dev, "%s: can't create rxq=%d\n",
				   __func__, queue);
			mvneta_cleanup_rxqs(pp);
			return err;
		}
	}

	return 0;
}

/* Init all tx queues */
static int mvneta_setup_txqs(struct mvneta_port *pp)
{
	int queue;

	for (queue = 0; queue < txq_number; queue++) {
		int err = mvneta_txq_init(pp, &pp->txqs[queue]);
		if (err) {
			netdev_err(pp->dev, "%s: can't create txq=%d\n",
				   __func__, queue);
			mvneta_cleanup_txqs(pp);
			return err;
		}
	}

	return 0;
}

static void mvneta_start_dev(struct mvneta_port *pp)
{
	/* start the Rx/Tx activity */
	mvneta_port_enable(pp);
}

static void mvneta_adjust_link(struct udevice *dev)
{
	struct mvneta_port *pp = dev_get_priv(dev);
	struct phy_device *phydev = pp->phydev;
	int status_change = 0;

	if (mvneta_port_is_fixed_link(pp)) {
		debug("Using fixed link, skip link adjust\n");
		return;
	}

	if (phydev->link) {
		if ((pp->speed != phydev->speed) ||
		    (pp->duplex != phydev->duplex)) {
			u32 val;

			val = mvreg_read(pp, MVNETA_GMAC_AUTONEG_CONFIG);
			val &= ~(MVNETA_GMAC_CONFIG_MII_SPEED |
				 MVNETA_GMAC_CONFIG_GMII_SPEED |
				 MVNETA_GMAC_CONFIG_FULL_DUPLEX |
				 MVNETA_GMAC_AN_SPEED_EN |
				 MVNETA_GMAC_AN_DUPLEX_EN);

			if (phydev->duplex)
				val |= MVNETA_GMAC_CONFIG_FULL_DUPLEX;

			if (phydev->speed == SPEED_1000)
				val |= MVNETA_GMAC_CONFIG_GMII_SPEED;
			else
				val |= MVNETA_GMAC_CONFIG_MII_SPEED;

			mvreg_write(pp, MVNETA_GMAC_AUTONEG_CONFIG, val);

			pp->duplex = phydev->duplex;
			pp->speed  = phydev->speed;
		}
	}

	if (phydev->link != pp->link) {
		if (!phydev->link) {
			pp->duplex = -1;
			pp->speed = 0;
		}

		pp->link = phydev->link;
		status_change = 1;
	}

	if (status_change) {
		if (phydev->link) {
			u32 val = mvreg_read(pp, MVNETA_GMAC_AUTONEG_CONFIG);
			val |= (MVNETA_GMAC_FORCE_LINK_PASS |
				MVNETA_GMAC_FORCE_LINK_DOWN);
			mvreg_write(pp, MVNETA_GMAC_AUTONEG_CONFIG, val);
			mvneta_port_up(pp);
		} else {
			mvneta_port_down(pp);
		}
	}
}

static int mvneta_open(struct udevice *dev)
{
	struct mvneta_port *pp = dev_get_priv(dev);
	int ret;

	ret = mvneta_setup_rxqs(pp);
	if (ret)
		return ret;

	ret = mvneta_setup_txqs(pp);
	if (ret)
		return ret;

	mvneta_adjust_link(dev);

	mvneta_start_dev(pp);

	return 0;
}

/* Initialize hw */
static int mvneta_init2(struct mvneta_port *pp)
{
	int queue;

	/* Disable port */
	mvneta_port_disable(pp);

	/* Set port default values */
	mvneta_defaults_set(pp);

	pp->txqs = kzalloc(txq_number * sizeof(struct mvneta_tx_queue),
			   GFP_KERNEL);
	if (!pp->txqs)
		return -ENOMEM;

	/* U-Boot special: use preallocated area */
	pp->txqs[0].descs = buffer_loc.tx_descs;

	/* Initialize TX descriptor rings */
	for (queue = 0; queue < txq_number; queue++) {
		struct mvneta_tx_queue *txq = &pp->txqs[queue];
		txq->id = queue;
		txq->size = pp->tx_ring_size;
	}

	pp->rxqs = kzalloc(rxq_number * sizeof(struct mvneta_rx_queue),
			   GFP_KERNEL);
	if (!pp->rxqs) {
		kfree(pp->txqs);
		return -ENOMEM;
	}

	/* U-Boot special: use preallocated area */
	pp->rxqs[0].descs = buffer_loc.rx_descs;

	/* Create Rx descriptor rings */
	for (queue = 0; queue < rxq_number; queue++) {
		struct mvneta_rx_queue *rxq = &pp->rxqs[queue];
		rxq->id = queue;
		rxq->size = pp->rx_ring_size;
	}

	return 0;
}

/* platform glue : initialize decoding windows */

/*
 * Not like A380, in Armada3700, there are two layers of decode windows for GBE:
 * First layer is:  GbE Address window that resides inside the GBE unit,
 * Second layer is: Fabric address window which is located in the NIC400
 *                  (South Fabric).
 * To simplify the address decode configuration for Armada3700, we bypass the
 * first layer of GBE decode window by setting the first window to 4GB.
 */
static void mvneta_bypass_mbus_windows(struct mvneta_port *pp)
{
	/*
	 * Set window size to 4GB, to bypass GBE address decode, leave the
	 * work to MBUS decode window
	 */
	mvreg_write(pp, MVNETA_WIN_SIZE(0), MVNETA_WIN_SIZE_MASK);

	/* Enable GBE address decode window 0 by set bit 0 to 0 */
	clrbits_le32(pp->base + MVNETA_BASE_ADDR_ENABLE,
		     MVNETA_BASE_ADDR_ENABLE_BIT);

	/* Set GBE address decode window 0 to full Access (read or write) */
	setbits_le32(pp->base + MVNETA_PORT_ACCESS_PROTECT,
		     MVNETA_PORT_ACCESS_PROTECT_WIN0_RW);
}

static void mvneta_conf_mbus_windows(struct mvneta_port *pp)
{
	const struct mbus_dram_target_info *dram;
	u32 win_enable;
	u32 win_protect;
	int i;

	dram = mvebu_mbus_dram_info();
	for (i = 0; i < 6; i++) {
		mvreg_write(pp, MVNETA_WIN_BASE(i), 0);
		mvreg_write(pp, MVNETA_WIN_SIZE(i), 0);

		if (i < 4)
			mvreg_write(pp, MVNETA_WIN_REMAP(i), 0);
	}

	win_enable = 0x3f;
	win_protect = 0;

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;
		mvreg_write(pp, MVNETA_WIN_BASE(i), (cs->base & 0xffff0000) |
			    (cs->mbus_attr << 8) | dram->mbus_dram_target_id);

		mvreg_write(pp, MVNETA_WIN_SIZE(i),
			    (cs->size - 1) & 0xffff0000);

		win_enable &= ~(1 << i);
		win_protect |= 3 << (2 * i);
	}

	mvreg_write(pp, MVNETA_BASE_ADDR_ENABLE, win_enable);
}

/* Power up the port */
static int mvneta_port_power_up(struct mvneta_port *pp, int phy_mode)
{
	u32 ctrl;

	/* MAC Cause register should be cleared */
	mvreg_write(pp, MVNETA_UNIT_INTR_CAUSE, 0);

	ctrl = mvreg_read(pp, MVNETA_GMAC_CTRL_2);

	/* Even though it might look weird, when we're configured in
	 * SGMII or QSGMII mode, the RGMII bit needs to be set.
	 */
	switch (phy_mode) {
	case PHY_INTERFACE_MODE_QSGMII:
		mvreg_write(pp, MVNETA_SERDES_CFG, MVNETA_QSGMII_SERDES_PROTO);
		ctrl |= MVNETA_GMAC2_PCS_ENABLE | MVNETA_GMAC2_PORT_RGMII;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		mvreg_write(pp, MVNETA_SERDES_CFG, MVNETA_SGMII_SERDES_PROTO);
		ctrl |= MVNETA_GMAC2_PCS_ENABLE | MVNETA_GMAC2_PORT_RGMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		ctrl |= MVNETA_GMAC2_PORT_RGMII;
		break;
	default:
		return -EINVAL;
	}

	/* Cancel Port Reset */
	ctrl &= ~MVNETA_GMAC2_PORT_RESET;
	mvreg_write(pp, MVNETA_GMAC_CTRL_2, ctrl);

	while ((mvreg_read(pp, MVNETA_GMAC_CTRL_2) &
		MVNETA_GMAC2_PORT_RESET) != 0)
		continue;

	return 0;
}

/* Device initialization routine */
static int mvneta_init(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mvneta_port *pp = dev_get_priv(dev);
	int err;

	pp->tx_ring_size = MVNETA_MAX_TXD;
	pp->rx_ring_size = MVNETA_MAX_RXD;

	err = mvneta_init2(pp);
	if (err < 0) {
		dev_err(&pdev->dev, "can't init eth hal\n");
		return err;
	}

	mvneta_mac_addr_set(pp, pdata->enetaddr, rxq_def);

	err = mvneta_port_power_up(pp, pp->phy_interface);
	if (err < 0) {
		dev_err(&pdev->dev, "can't power up port\n");
		return err;
	}

	/* Call open() now as it needs to be done before runing send() */
	mvneta_open(dev);

	return 0;
}

/* U-Boot only functions follow here */

/* SMI / MDIO functions */

static int smi_wait_ready(struct mvneta_port *pp)
{
	u32 timeout = MVNETA_SMI_TIMEOUT;
	u32 smi_reg;

	/* wait till the SMI is not busy */
	do {
		/* read smi register */
		smi_reg = mvreg_read(pp, MVNETA_SMI);
		if (timeout-- == 0) {
			printf("Error: SMI busy timeout\n");
			return -EFAULT;
		}
	} while (smi_reg & MVNETA_SMI_BUSY);

	return 0;
}

/*
 * mvneta_mdio_read - miiphy_read callback function.
 *
 * Returns 16bit phy register value, or 0xffff on error
 */
static int mvneta_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mvneta_port *pp = bus->priv;
	u32 smi_reg;
	u32 timeout;

	/* check parameters */
	if (addr > MVNETA_PHY_ADDR_MASK) {
		printf("Error: Invalid PHY address %d\n", addr);
		return -EFAULT;
	}

	if (reg > MVNETA_PHY_REG_MASK) {
		printf("Err: Invalid register offset %d\n", reg);
		return -EFAULT;
	}

	/* wait till the SMI is not busy */
	if (smi_wait_ready(pp) < 0)
		return -EFAULT;

	/* fill the phy address and regiser offset and read opcode */
	smi_reg = (addr << MVNETA_SMI_DEV_ADDR_OFFS)
		| (reg << MVNETA_SMI_REG_ADDR_OFFS)
		| MVNETA_SMI_OPCODE_READ;

	/* write the smi register */
	mvreg_write(pp, MVNETA_SMI, smi_reg);

	/* wait till read value is ready */
	timeout = MVNETA_SMI_TIMEOUT;

	do {
		/* read smi register */
		smi_reg = mvreg_read(pp, MVNETA_SMI);
		if (timeout-- == 0) {
			printf("Err: SMI read ready timeout\n");
			return -EFAULT;
		}
	} while (!(smi_reg & MVNETA_SMI_READ_VALID));

	/* Wait for the data to update in the SMI register */
	for (timeout = 0; timeout < MVNETA_SMI_TIMEOUT; timeout++)
		;

	return mvreg_read(pp, MVNETA_SMI) & MVNETA_SMI_DATA_MASK;
}

/*
 * mvneta_mdio_write - miiphy_write callback function.
 *
 * Returns 0 if write succeed, -EINVAL on bad parameters
 * -ETIME on timeout
 */
static int mvneta_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			     u16 value)
{
	struct mvneta_port *pp = bus->priv;
	u32 smi_reg;

	/* check parameters */
	if (addr > MVNETA_PHY_ADDR_MASK) {
		printf("Error: Invalid PHY address %d\n", addr);
		return -EFAULT;
	}

	if (reg > MVNETA_PHY_REG_MASK) {
		printf("Err: Invalid register offset %d\n", reg);
		return -EFAULT;
	}

	/* wait till the SMI is not busy */
	if (smi_wait_ready(pp) < 0)
		return -EFAULT;

	/* fill the phy addr and reg offset and write opcode and data */
	smi_reg = value << MVNETA_SMI_DATA_OFFS;
	smi_reg |= (addr << MVNETA_SMI_DEV_ADDR_OFFS)
		| (reg << MVNETA_SMI_REG_ADDR_OFFS);
	smi_reg &= ~MVNETA_SMI_OPCODE_READ;

	/* write the smi register */
	mvreg_write(pp, MVNETA_SMI, smi_reg);

	return 0;
}

static int mvneta_start(struct udevice *dev)
{
	struct mvneta_port *pp = dev_get_priv(dev);
	struct phy_device *phydev;

	mvneta_port_power_up(pp, pp->phy_interface);

	if (!pp->init || pp->link == 0) {
		if (mvneta_port_is_fixed_link(pp)) {
			u32 val;

			pp->init = 1;
			pp->link = 1;
			mvneta_init(dev);

			val = MVNETA_GMAC_FORCE_LINK_UP |
			      MVNETA_GMAC_IB_BYPASS_AN_EN |
			      MVNETA_GMAC_SET_FC_EN |
			      MVNETA_GMAC_ADVERT_FC_EN |
			      MVNETA_GMAC_SAMPLE_TX_CFG_EN;

			if (pp->duplex)
				val |= MVNETA_GMAC_CONFIG_FULL_DUPLEX;

			if (pp->speed == SPEED_1000)
				val |= MVNETA_GMAC_CONFIG_GMII_SPEED;
			else if (pp->speed == SPEED_100)
				val |= MVNETA_GMAC_CONFIG_MII_SPEED;

			mvreg_write(pp, MVNETA_GMAC_AUTONEG_CONFIG, val);
		} else {
			/* Set phy address of the port */
			mvreg_write(pp, MVNETA_PHY_ADDR, pp->phyaddr);

			phydev = phy_connect(pp->bus, pp->phyaddr, dev,
					     pp->phy_interface);
			if (!phydev) {
				printf("phy_connect failed\n");
				return -ENODEV;
			}

			pp->phydev = phydev;
			phy_config(phydev);
			phy_startup(phydev);
			if (!phydev->link) {
				printf("%s: No link.\n", phydev->dev->name);
				return -1;
			}

			/* Full init on first call */
			mvneta_init(dev);
			pp->init = 1;
			return 0;
		}
	}

	/* Upon all following calls, this is enough */
	mvneta_port_up(pp);
	mvneta_port_enable(pp);

	return 0;
}

static int mvneta_send(struct udevice *dev, void *packet, int length)
{
	struct mvneta_port *pp = dev_get_priv(dev);
	struct mvneta_tx_queue *txq = &pp->txqs[0];
	struct mvneta_tx_desc *tx_desc;
	int sent_desc;
	u32 timeout = 0;

	/* Get a descriptor for the first part of the packet */
	tx_desc = mvneta_txq_next_desc_get(txq);

	tx_desc->buf_phys_addr = (u32)(uintptr_t)packet;
	tx_desc->data_size = length;
	flush_dcache_range((ulong)packet,
			   (ulong)packet + ALIGN(length, PKTALIGN));

	/* First and Last descriptor */
	tx_desc->command = MVNETA_TX_L4_CSUM_NOT | MVNETA_TXD_FLZ_DESC;
	mvneta_txq_pend_desc_add(pp, txq, 1);

	/* Wait for packet to be sent (queue might help with speed here) */
	sent_desc = mvneta_txq_sent_desc_num_get(pp, txq);
	while (!sent_desc) {
		if (timeout++ > 10000) {
			printf("timeout: packet not sent\n");
			return -1;
		}
		sent_desc = mvneta_txq_sent_desc_num_get(pp, txq);
	}

	/* txDone has increased - hw sent packet */
	mvneta_txq_sent_desc_dec(pp, txq, sent_desc);

	return 0;
}

static int mvneta_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mvneta_port *pp = dev_get_priv(dev);
	int rx_done;
	struct mvneta_rx_queue *rxq;
	int rx_bytes = 0;

	/* get rx queue */
	rxq = mvneta_rxq_handle_get(pp, rxq_def);
	rx_done = mvneta_rxq_busy_desc_num_get(pp, rxq);

	if (rx_done) {
		struct mvneta_rx_desc *rx_desc;
		unsigned char *data;
		u32 rx_status;

		/*
		 * No cache invalidation needed here, since the desc's are
		 * located in a uncached memory region
		 */
		rx_desc = mvneta_rxq_next_desc_get(rxq);

		rx_status = rx_desc->status;
		if (!mvneta_rxq_desc_is_first_last(rx_status) ||
		    (rx_status & MVNETA_RXD_ERR_SUMMARY)) {
			mvneta_rx_error(pp, rx_desc);
			/* leave the descriptor untouched */
			return -EIO;
		}

		/* 2 bytes for marvell header. 4 bytes for crc */
		rx_bytes = rx_desc->data_size - 6;

		/* give packet to stack - skip on first 2 bytes */
		data = (u8 *)(uintptr_t)rx_desc->buf_cookie + 2;
		/*
		 * No cache invalidation needed here, since the rx_buffer's are
		 * located in a uncached memory region
		 */
		*packetp = data;

		/*
		 * Only mark one descriptor as free
		 * since only one was processed
		 */
		mvneta_rxq_desc_num_update(pp, rxq, 1, 1);
	}

	return rx_bytes;
}

static int mvneta_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mvneta_port *pp = dev_get_priv(dev);
	void *blob = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct mii_dev *bus;
	unsigned long addr;
	void *bd_space;
	int ret;
	int fl_node;

	/*
	 * Allocate buffer area for descs and rx_buffers. This is only
	 * done once for all interfaces. As only one interface can
	 * be active. Make this area DMA safe by disabling the D-cache
	 */
	if (!buffer_loc.tx_descs) {
		u32 size;

		/* Align buffer area for descs and rx_buffers to 1MiB */
		bd_space = memalign(1 << MMU_SECTION_SHIFT, BD_SPACE);
		flush_dcache_range((ulong)bd_space, (ulong)bd_space + BD_SPACE);
		mmu_set_region_dcache_behaviour((phys_addr_t)bd_space, BD_SPACE,
						DCACHE_OFF);
		buffer_loc.tx_descs = (struct mvneta_tx_desc *)bd_space;
		size = roundup(MVNETA_MAX_TXD * sizeof(struct mvneta_tx_desc),
				ARCH_DMA_MINALIGN);
		memset(buffer_loc.tx_descs, 0, size);
		buffer_loc.rx_descs = (struct mvneta_rx_desc *)
			((phys_addr_t)bd_space + size);
		size += roundup(MVNETA_MAX_RXD * sizeof(struct mvneta_rx_desc),
				ARCH_DMA_MINALIGN);
		buffer_loc.rx_buffers = (phys_addr_t)(bd_space + size);
	}

	pp->base = (void __iomem *)pdata->iobase;

	/* Configure MBUS address windows */
	if (device_is_compatible(dev, "marvell,armada-3700-neta"))
		mvneta_bypass_mbus_windows(pp);
	else
		mvneta_conf_mbus_windows(pp);

	/* PHY interface is already decoded in mvneta_ofdata_to_platdata() */
	pp->phy_interface = pdata->phy_interface;

	/* fetch 'fixed-link' property from 'neta' node */
	fl_node = fdt_subnode_offset(blob, node, "fixed-link");
	if (fl_node != -FDT_ERR_NOTFOUND) {
		/* set phy_addr to invalid value for fixed link */
		pp->phyaddr = PHY_MAX_ADDR + 1;
		pp->duplex = fdtdec_get_bool(blob, fl_node, "full-duplex");
		pp->speed = fdtdec_get_int(blob, fl_node, "speed", 0);
	} else {
		/* Now read phyaddr from DT */
		addr = fdtdec_get_int(blob, node, "phy", 0);
		addr = fdt_node_offset_by_phandle(blob, addr);
		pp->phyaddr = fdtdec_get_int(blob, addr, "reg", 0);
	}

	bus = mdio_alloc();
	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = mvneta_mdio_read;
	bus->write = mvneta_mdio_write;
	snprintf(bus->name, sizeof(bus->name), dev->name);
	bus->priv = (void *)pp;
	pp->bus = bus;

	ret = mdio_register(bus);
	if (ret)
		return ret;

#ifdef CONFIG_DM_GPIO
	gpio_request_by_name(dev, "phy-reset-gpios", 0,
			     &pp->phy_reset_gpio, GPIOD_IS_OUT);

	if (dm_gpio_is_valid(&pp->phy_reset_gpio)) {
		dm_gpio_set_value(&pp->phy_reset_gpio, 1);
		mdelay(10);
		dm_gpio_set_value(&pp->phy_reset_gpio, 0);
	}
#endif

	return board_network_enable(bus);
}

static void mvneta_stop(struct udevice *dev)
{
	struct mvneta_port *pp = dev_get_priv(dev);

	mvneta_port_down(pp);
	mvneta_port_disable(pp);
}

static const struct eth_ops mvneta_ops = {
	.start		= mvneta_start,
	.send		= mvneta_send,
	.recv		= mvneta_recv,
	.stop		= mvneta_stop,
	.write_hwaddr	= mvneta_write_hwaddr,
};

static int mvneta_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const char *phy_mode;

	pdata->iobase = devfdt_get_addr(dev);

	/* Get phy-mode / phy_interface from DT */
	pdata->phy_interface = -1;
	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode",
			       NULL);
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id mvneta_ids[] = {
	{ .compatible = "marvell,armada-370-neta" },
	{ .compatible = "marvell,armada-xp-neta" },
	{ .compatible = "marvell,armada-3700-neta" },
	{ }
};

U_BOOT_DRIVER(mvneta) = {
	.name	= "mvneta",
	.id	= UCLASS_ETH,
	.of_match = mvneta_ids,
	.ofdata_to_platdata = mvneta_ofdata_to_platdata,
	.probe	= mvneta_probe,
	.ops	= &mvneta_ops,
	.priv_auto_alloc_size = sizeof(struct mvneta_port),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
