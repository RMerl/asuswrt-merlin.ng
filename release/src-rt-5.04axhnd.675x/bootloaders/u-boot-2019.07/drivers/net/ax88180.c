/*
 * ax88180: ASIX AX88180 Non-PCI Gigabit Ethernet u-boot driver
 *
 * This program is free software; you can distribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 * This program is distributed in the hope it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
 * USA.
 */

/*
 * ========================================================================
 * ASIX AX88180 Non-PCI 16/32-bit Gigabit Ethernet Linux Driver
 *
 * The AX88180 Ethernet controller is a high performance and highly
 * integrated local CPU bus Ethernet controller with embedded 40K bytes
 * SRAM and supports both 16-bit and 32-bit SRAM-Like interfaces for any
 * embedded systems.
 * The AX88180 is a single chip 10/100/1000Mbps Gigabit Ethernet
 * controller that supports both MII and RGMII interfaces and is
 * compliant to IEEE 802.3, IEEE 802.3u and IEEE 802.3z standards.
 *
 * Please visit ASIX's web site (http://www.asix.com.tw) for more
 * details.
 *
 * Module Name	: ax88180.c
 * Date		: 2008-07-07
 * History
 * 09/06/2006	: New release for AX88180 US2 chip.
 * 07/07/2008	: Fix up the coding style and using inline functions
 *		  instead of macros
 * ========================================================================
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <malloc.h>
#include <linux/mii.h>
#include "ax88180.h"

/*
 * ===========================================================================
 * Local SubProgram Declaration
 * ===========================================================================
 */
static void ax88180_rx_handler (struct eth_device *dev);
static int ax88180_phy_initial (struct eth_device *dev);
static void ax88180_media_config (struct eth_device *dev);
static unsigned long get_CicadaPHY_media_mode (struct eth_device *dev);
static unsigned long get_MarvellPHY_media_mode (struct eth_device *dev);
static unsigned short ax88180_mdio_read (struct eth_device *dev,
					 unsigned long regaddr);
static void ax88180_mdio_write (struct eth_device *dev,
				unsigned long regaddr, unsigned short regdata);

/*
 * ===========================================================================
 * Local SubProgram Bodies
 * ===========================================================================
 */
static int ax88180_mdio_check_complete (struct eth_device *dev)
{
	int us_cnt = 10000;
	unsigned short tmpval;

	/* MDIO read/write should not take more than 10 ms */
	while (--us_cnt) {
		tmpval = INW (dev, MDIOCTRL);
		if (((tmpval & READ_PHY) == 0) && ((tmpval & WRITE_PHY) == 0))
			break;
	}

	return us_cnt;
}

static unsigned short
ax88180_mdio_read (struct eth_device *dev, unsigned long regaddr)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned long tmpval = 0;

	OUTW (dev, (READ_PHY | (regaddr << 8) | priv->PhyAddr), MDIOCTRL);

	if (ax88180_mdio_check_complete (dev))
		tmpval = INW (dev, MDIODP);
	else
		printf ("Failed to read PHY register!\n");

	return (unsigned short)(tmpval & 0xFFFF);
}

static void
ax88180_mdio_write (struct eth_device *dev, unsigned long regaddr,
		    unsigned short regdata)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;

	OUTW (dev, regdata, MDIODP);

	OUTW (dev, (WRITE_PHY | (regaddr << 8) | priv->PhyAddr), MDIOCTRL);

	if (!ax88180_mdio_check_complete (dev))
		printf ("Failed to write PHY register!\n");
}

static int ax88180_phy_reset (struct eth_device *dev)
{
	unsigned short delay_cnt = 500;

	ax88180_mdio_write (dev, MII_BMCR, (BMCR_RESET | BMCR_ANENABLE));

	/* Wait for the reset to complete, or time out (500 ms) */
	while (ax88180_mdio_read (dev, MII_BMCR) & BMCR_RESET) {
		udelay (1000);
		if (--delay_cnt == 0) {
			printf ("Failed to reset PHY!\n");
			return -1;
		}
	}

	return 0;
}

static void ax88180_mac_reset (struct eth_device *dev)
{
	unsigned long tmpval;
	unsigned char i;

	struct {
		unsigned short offset, value;
	} program_seq[] = {
		{
		MISC, MISC_NORMAL}, {
		RXINDICATOR, DEFAULT_RXINDICATOR}, {
		TXCMD, DEFAULT_TXCMD}, {
		TXBS, DEFAULT_TXBS}, {
		TXDES0, DEFAULT_TXDES0}, {
		TXDES1, DEFAULT_TXDES1}, {
		TXDES2, DEFAULT_TXDES2}, {
		TXDES3, DEFAULT_TXDES3}, {
		TXCFG, DEFAULT_TXCFG}, {
		MACCFG2, DEFAULT_MACCFG2}, {
		MACCFG3, DEFAULT_MACCFG3}, {
		TXLEN, DEFAULT_TXLEN}, {
		RXBTHD0, DEFAULT_RXBTHD0}, {
		RXBTHD1, DEFAULT_RXBTHD1}, {
		RXFULTHD, DEFAULT_RXFULTHD}, {
		DOGTHD0, DEFAULT_DOGTHD0}, {
	DOGTHD1, DEFAULT_DOGTHD1},};

	OUTW (dev, MISC_RESET_MAC, MISC);
	tmpval = INW (dev, MISC);

	for (i = 0; i < ARRAY_SIZE(program_seq); i++)
		OUTW (dev, program_seq[i].value, program_seq[i].offset);
}

static int ax88180_poll_tx_complete (struct eth_device *dev)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned long tmpval, txbs_txdp;
	int TimeOutCnt = 10000;

	txbs_txdp = 1 << priv->NextTxDesc;

	while (TimeOutCnt--) {

		tmpval = INW (dev, TXBS);

		if ((tmpval & txbs_txdp) == 0)
			break;

		udelay (100);
	}

	if (TimeOutCnt)
		return 0;
	else
		return -TimeOutCnt;
}

static void ax88180_rx_handler (struct eth_device *dev)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned long data_size;
	unsigned short rxcurt_ptr, rxbound_ptr, next_ptr;
	int i;
#if defined (CONFIG_DRIVER_AX88180_16BIT)
	unsigned short *rxdata = (unsigned short *)net_rx_packets[0];
#else
	unsigned long *rxdata = (unsigned long *)net_rx_packets[0];
#endif
	unsigned short count;

	rxcurt_ptr = INW (dev, RXCURT);
	rxbound_ptr = INW (dev, RXBOUND);
	next_ptr = (rxbound_ptr + 1) & RX_PAGE_NUM_MASK;

	debug ("ax88180: RX original RXBOUND=0x%04x,"
	       " RXCURT=0x%04x\n", rxbound_ptr, rxcurt_ptr);

	while (next_ptr != rxcurt_ptr) {

		OUTW (dev, RX_START_READ, RXINDICATOR);

		data_size = READ_RXBUF (dev) & 0xFFFF;

		if ((data_size == 0) || (data_size > MAX_RX_SIZE)) {

			OUTW (dev, RX_STOP_READ, RXINDICATOR);

			ax88180_mac_reset (dev);
			printf ("ax88180: Invalid Rx packet length!"
				" (len=0x%04lx)\n", data_size);

			debug ("ax88180: RX RXBOUND=0x%04x,"
			       "RXCURT=0x%04x\n", rxbound_ptr, rxcurt_ptr);
			return;
		}

		rxbound_ptr += (((data_size + 0xF) & 0xFFF0) >> 4) + 1;
		rxbound_ptr &= RX_PAGE_NUM_MASK;

		/* Comput access times */
		count = (data_size + priv->PadSize) >> priv->BusWidth;

		for (i = 0; i < count; i++) {
			*(rxdata + i) = READ_RXBUF (dev);
		}

		OUTW (dev, RX_STOP_READ, RXINDICATOR);

		/* Pass the packet up to the protocol layers. */
		net_process_received_packet(net_rx_packets[0], data_size);

		OUTW (dev, rxbound_ptr, RXBOUND);

		rxcurt_ptr = INW (dev, RXCURT);
		rxbound_ptr = INW (dev, RXBOUND);
		next_ptr = (rxbound_ptr + 1) & RX_PAGE_NUM_MASK;

		debug ("ax88180: RX updated RXBOUND=0x%04x,"
		       "RXCURT=0x%04x\n", rxbound_ptr, rxcurt_ptr);
	}

	return;
}

static int ax88180_phy_initial (struct eth_device *dev)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned long tmp_regval;
	unsigned short phyaddr;

	/* Search for first avaliable PHY chipset */
#ifdef CONFIG_PHY_ADDR
	phyaddr = CONFIG_PHY_ADDR;
#else
	for (phyaddr = 0; phyaddr < 32; ++phyaddr)
#endif
	{
		priv->PhyAddr = phyaddr;
		priv->PhyID0 = ax88180_mdio_read(dev, MII_PHYSID1);
		priv->PhyID1 = ax88180_mdio_read(dev, MII_PHYSID2);

		switch (priv->PhyID0) {
		case MARVELL_ALASKA_PHYSID0:
			debug("ax88180: Found Marvell Alaska PHY family."
			      " (PHY Addr=0x%x)\n", priv->PhyAddr);

			switch (priv->PhyID1) {
			case MARVELL_88E1118_PHYSID1:
				ax88180_mdio_write(dev, M88E1118_PAGE_SEL, 2);
				ax88180_mdio_write(dev, M88E1118_CR,
					M88E1118_CR_DEFAULT);
				ax88180_mdio_write(dev, M88E1118_PAGE_SEL, 3);
				ax88180_mdio_write(dev, M88E1118_LEDCTL,
					M88E1118_LEDCTL_DEFAULT);
				ax88180_mdio_write(dev, M88E1118_LEDMIX,
					M88E1118_LEDMIX_LED050 | M88E1118_LEDMIX_LED150 | 0x15);
				ax88180_mdio_write(dev, M88E1118_PAGE_SEL, 0);
			default: /* Default to 88E1111 Phy */
				tmp_regval = ax88180_mdio_read(dev, M88E1111_EXT_SSR);
				if ((tmp_regval & HWCFG_MODE_MASK) != RGMII_COPPER_MODE)
					ax88180_mdio_write(dev, M88E1111_EXT_SCR,
						DEFAULT_EXT_SCR);
			}

			if (ax88180_phy_reset(dev) < 0)
				return 0;
			ax88180_mdio_write(dev, M88_IER, LINK_CHANGE_INT);

			return 1;

		case CICADA_CIS8201_PHYSID0:
			debug("ax88180: Found CICADA CIS8201 PHY"
			      " chipset. (PHY Addr=0x%x)\n", priv->PhyAddr);

			ax88180_mdio_write(dev, CIS_IMR,
					    (CIS_INT_ENABLE | LINK_CHANGE_INT));

			/* Set CIS_SMI_PRIORITY bit before force the media mode */
			tmp_regval = ax88180_mdio_read(dev, CIS_AUX_CTRL_STATUS);
			tmp_regval &= ~CIS_SMI_PRIORITY;
			ax88180_mdio_write(dev, CIS_AUX_CTRL_STATUS, tmp_regval);

			return 1;

		case 0xffff:
			/* No PHY at this addr */
			break;

		default:
			printf("ax88180: Unknown PHY chipset %#x at addr %#x\n",
			       priv->PhyID0, priv->PhyAddr);
			break;
		}
	}

	printf("ax88180: Unknown PHY chipset!!\n");
	return 0;
}

static void ax88180_media_config (struct eth_device *dev)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned long bmcr_val, bmsr_val;
	unsigned long rxcfg_val, maccfg0_val, maccfg1_val;
	unsigned long RealMediaMode;
	int i;

	/* Waiting 2 seconds for PHY link stable */
	for (i = 0; i < 20000; i++) {
		bmsr_val = ax88180_mdio_read (dev, MII_BMSR);
		if (bmsr_val & BMSR_LSTATUS) {
			break;
		}
		udelay (100);
	}

	bmsr_val = ax88180_mdio_read (dev, MII_BMSR);
	debug ("ax88180: BMSR=0x%04x\n", (unsigned int)bmsr_val);

	if (bmsr_val & BMSR_LSTATUS) {
		bmcr_val = ax88180_mdio_read (dev, MII_BMCR);

		if (bmcr_val & BMCR_ANENABLE) {

			/*
			 * Waiting for Auto-negotiation completion, this may
			 * take up to 5 seconds.
			 */
			debug ("ax88180: Auto-negotiation is "
			       "enabled. Waiting for NWay completion..\n");
			for (i = 0; i < 50000; i++) {
				bmsr_val = ax88180_mdio_read (dev, MII_BMSR);
				if (bmsr_val & BMSR_ANEGCOMPLETE) {
					break;
				}
				udelay (100);
			}
		} else
			debug ("ax88180: Auto-negotiation is disabled.\n");

		debug ("ax88180: BMCR=0x%04x, BMSR=0x%04x\n",
		       (unsigned int)bmcr_val, (unsigned int)bmsr_val);

		/* Get real media mode here */
		switch (priv->PhyID0) {
		case MARVELL_ALASKA_PHYSID0:
			RealMediaMode = get_MarvellPHY_media_mode(dev);
			break;
		case CICADA_CIS8201_PHYSID0:
			RealMediaMode = get_CicadaPHY_media_mode(dev);
			break;
		default:
			RealMediaMode = MEDIA_1000FULL;
			break;
		}

		priv->LinkState = INS_LINK_UP;

		switch (RealMediaMode) {
		case MEDIA_1000FULL:
			debug ("ax88180: 1000Mbps Full-duplex mode.\n");
			rxcfg_val = RXFLOW_ENABLE | DEFAULT_RXCFG;
			maccfg0_val = TXFLOW_ENABLE | DEFAULT_MACCFG0;
			maccfg1_val = GIGA_MODE_EN | RXFLOW_EN |
			    FULLDUPLEX | DEFAULT_MACCFG1;
			break;

		case MEDIA_1000HALF:
			debug ("ax88180: 1000Mbps Half-duplex mode.\n");
			rxcfg_val = DEFAULT_RXCFG;
			maccfg0_val = DEFAULT_MACCFG0;
			maccfg1_val = GIGA_MODE_EN | DEFAULT_MACCFG1;
			break;

		case MEDIA_100FULL:
			debug ("ax88180: 100Mbps Full-duplex mode.\n");
			rxcfg_val = RXFLOW_ENABLE | DEFAULT_RXCFG;
			maccfg0_val = SPEED100 | TXFLOW_ENABLE
			    | DEFAULT_MACCFG0;
			maccfg1_val = RXFLOW_EN | FULLDUPLEX | DEFAULT_MACCFG1;
			break;

		case MEDIA_100HALF:
			debug ("ax88180: 100Mbps Half-duplex mode.\n");
			rxcfg_val = DEFAULT_RXCFG;
			maccfg0_val = SPEED100 | DEFAULT_MACCFG0;
			maccfg1_val = DEFAULT_MACCFG1;
			break;

		case MEDIA_10FULL:
			debug ("ax88180: 10Mbps Full-duplex mode.\n");
			rxcfg_val = RXFLOW_ENABLE | DEFAULT_RXCFG;
			maccfg0_val = TXFLOW_ENABLE | DEFAULT_MACCFG0;
			maccfg1_val = RXFLOW_EN | FULLDUPLEX | DEFAULT_MACCFG1;
			break;

		case MEDIA_10HALF:
			debug ("ax88180: 10Mbps Half-duplex mode.\n");
			rxcfg_val = DEFAULT_RXCFG;
			maccfg0_val = DEFAULT_MACCFG0;
			maccfg1_val = DEFAULT_MACCFG1;
			break;
		default:
			debug ("ax88180: Unknow media mode.\n");
			rxcfg_val = DEFAULT_RXCFG;
			maccfg0_val = DEFAULT_MACCFG0;
			maccfg1_val = DEFAULT_MACCFG1;

			priv->LinkState = INS_LINK_DOWN;
			break;
		}

	} else {
		rxcfg_val = DEFAULT_RXCFG;
		maccfg0_val = DEFAULT_MACCFG0;
		maccfg1_val = DEFAULT_MACCFG1;

		priv->LinkState = INS_LINK_DOWN;
	}

	OUTW (dev, rxcfg_val, RXCFG);
	OUTW (dev, maccfg0_val, MACCFG0);
	OUTW (dev, maccfg1_val, MACCFG1);

	return;
}

static unsigned long get_MarvellPHY_media_mode (struct eth_device *dev)
{
	unsigned long m88_ssr;
	unsigned long MediaMode;

	m88_ssr = ax88180_mdio_read (dev, M88_SSR);
	switch (m88_ssr & SSR_MEDIA_MASK) {
	case SSR_1000FULL:
		MediaMode = MEDIA_1000FULL;
		break;
	case SSR_1000HALF:
		MediaMode = MEDIA_1000HALF;
		break;
	case SSR_100FULL:
		MediaMode = MEDIA_100FULL;
		break;
	case SSR_100HALF:
		MediaMode = MEDIA_100HALF;
		break;
	case SSR_10FULL:
		MediaMode = MEDIA_10FULL;
		break;
	case SSR_10HALF:
		MediaMode = MEDIA_10HALF;
		break;
	default:
		MediaMode = MEDIA_UNKNOWN;
		break;
	}

	return MediaMode;
}

static unsigned long get_CicadaPHY_media_mode (struct eth_device *dev)
{
	unsigned long tmp_regval;
	unsigned long MediaMode;

	tmp_regval = ax88180_mdio_read (dev, CIS_AUX_CTRL_STATUS);
	switch (tmp_regval & CIS_MEDIA_MASK) {
	case CIS_1000FULL:
		MediaMode = MEDIA_1000FULL;
		break;
	case CIS_1000HALF:
		MediaMode = MEDIA_1000HALF;
		break;
	case CIS_100FULL:
		MediaMode = MEDIA_100FULL;
		break;
	case CIS_100HALF:
		MediaMode = MEDIA_100HALF;
		break;
	case CIS_10FULL:
		MediaMode = MEDIA_10FULL;
		break;
	case CIS_10HALF:
		MediaMode = MEDIA_10HALF;
		break;
	default:
		MediaMode = MEDIA_UNKNOWN;
		break;
	}

	return MediaMode;
}

static void ax88180_halt (struct eth_device *dev)
{
	/* Disable AX88180 TX/RX functions */
	OUTW (dev, WAKEMOD, CMD);
}

static int ax88180_init (struct eth_device *dev, bd_t * bd)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned short tmp_regval;

	ax88180_mac_reset (dev);

	/* Disable interrupt */
	OUTW (dev, CLEAR_IMR, IMR);

	/* Disable AX88180 TX/RX functions */
	OUTW (dev, WAKEMOD, CMD);

	/* Fill the MAC address */
	tmp_regval =
	    dev->enetaddr[0] | (((unsigned short)dev->enetaddr[1]) << 8);
	OUTW (dev, tmp_regval, MACID0);

	tmp_regval =
	    dev->enetaddr[2] | (((unsigned short)dev->enetaddr[3]) << 8);
	OUTW (dev, tmp_regval, MACID1);

	tmp_regval =
	    dev->enetaddr[4] | (((unsigned short)dev->enetaddr[5]) << 8);
	OUTW (dev, tmp_regval, MACID2);

	ax88180_media_config (dev);

	OUTW (dev, DEFAULT_RXFILTER, RXFILTER);

	/* Initial variables here */
	priv->FirstTxDesc = TXDP0;
	priv->NextTxDesc = TXDP0;

	/* Check if there is any invalid interrupt status and clear it. */
	OUTW (dev, INW (dev, ISR), ISR);

	/* Start AX88180 TX/RX functions */
	OUTW (dev, (RXEN | TXEN | WAKEMOD), CMD);

	return 0;
}

/* Get a data block via Ethernet */
static int ax88180_recv (struct eth_device *dev)
{
	unsigned short ISR_Status;
	unsigned short tmp_regval;

	/* Read and check interrupt status here. */
	ISR_Status = INW (dev, ISR);

	while (ISR_Status) {
		/* Clear the interrupt status */
		OUTW (dev, ISR_Status, ISR);

		debug ("\nax88180: The interrupt status = 0x%04x\n",
		       ISR_Status);

		if (ISR_Status & ISR_PHY) {
			/* Read ISR register once to clear PHY interrupt bit */
			tmp_regval = ax88180_mdio_read (dev, M88_ISR);
			ax88180_media_config (dev);
		}

		if ((ISR_Status & ISR_RX) || (ISR_Status & ISR_RXBUFFOVR)) {
			ax88180_rx_handler (dev);
		}

		/* Read and check interrupt status again */
		ISR_Status = INW (dev, ISR);
	}

	return 0;
}

/* Send a data block via Ethernet. */
static int ax88180_send(struct eth_device *dev, void *packet, int length)
{
	struct ax88180_private *priv = (struct ax88180_private *)dev->priv;
	unsigned short TXDES_addr;
	unsigned short txcmd_txdp, txbs_txdp;
	unsigned short tmp_data;
	int i;
#if defined (CONFIG_DRIVER_AX88180_16BIT)
	volatile unsigned short *txdata = (volatile unsigned short *)packet;
#else
	volatile unsigned long *txdata = (volatile unsigned long *)packet;
#endif
	unsigned short count;

	if (priv->LinkState != INS_LINK_UP) {
		return 0;
	}

	priv->FirstTxDesc = priv->NextTxDesc;
	txbs_txdp = 1 << priv->FirstTxDesc;

	debug ("ax88180: TXDP%d is available\n", priv->FirstTxDesc);

	txcmd_txdp = priv->FirstTxDesc << 13;
	TXDES_addr = TXDES0 + (priv->FirstTxDesc << 2);

	OUTW (dev, (txcmd_txdp | length | TX_START_WRITE), TXCMD);

	/* Comput access times */
	count = (length + priv->PadSize) >> priv->BusWidth;

	for (i = 0; i < count; i++) {
		WRITE_TXBUF (dev, *(txdata + i));
	}

	OUTW (dev, txcmd_txdp | length, TXCMD);
	OUTW (dev, txbs_txdp, TXBS);
	OUTW (dev, (TXDPx_ENABLE | length), TXDES_addr);

	priv->NextTxDesc = (priv->NextTxDesc + 1) & TXDP_MASK;

	/*
	 * Check the available transmit descriptor, if we had exhausted all
	 * transmit descriptor ,then we have to wait for at least one free
	 * descriptor
	 */
	txbs_txdp = 1 << priv->NextTxDesc;
	tmp_data = INW (dev, TXBS);

	if (tmp_data & txbs_txdp) {
		if (ax88180_poll_tx_complete (dev) < 0) {
			ax88180_mac_reset (dev);
			priv->FirstTxDesc = TXDP0;
			priv->NextTxDesc = TXDP0;
			printf ("ax88180: Transmit time out occurred!\n");
		}
	}

	return 0;
}

static void ax88180_read_mac_addr (struct eth_device *dev)
{
	unsigned short macid0_val, macid1_val, macid2_val;
	unsigned short tmp_regval;
	unsigned short i;

	/* Reload MAC address from EEPROM */
	OUTW (dev, RELOAD_EEPROM, PROMCTRL);

	/* Waiting for reload eeprom completion */
	for (i = 0; i < 500; i++) {
		tmp_regval = INW (dev, PROMCTRL);
		if ((tmp_regval & RELOAD_EEPROM) == 0)
			break;
		udelay (1000);
	}

	/* Get MAC addresses */
	macid0_val = INW (dev, MACID0);
	macid1_val = INW (dev, MACID1);
	macid2_val = INW (dev, MACID2);

	if (((macid0_val | macid1_val | macid2_val) != 0) &&
	    ((macid0_val & 0x01) == 0)) {
		dev->enetaddr[0] = (unsigned char)macid0_val;
		dev->enetaddr[1] = (unsigned char)(macid0_val >> 8);
		dev->enetaddr[2] = (unsigned char)macid1_val;
		dev->enetaddr[3] = (unsigned char)(macid1_val >> 8);
		dev->enetaddr[4] = (unsigned char)macid2_val;
		dev->enetaddr[5] = (unsigned char)(macid2_val >> 8);
	}
}

/* Exported SubProgram Bodies */
int ax88180_initialize (bd_t * bis)
{
	struct eth_device *dev;
	struct ax88180_private *priv;

	dev = (struct eth_device *)malloc (sizeof *dev);

	if (NULL == dev)
		return 0;

	memset (dev, 0, sizeof *dev);

	priv = (struct ax88180_private *)malloc (sizeof (*priv));

	if (NULL == priv)
		return 0;

	memset (priv, 0, sizeof *priv);

	strcpy(dev->name, "ax88180");
	dev->iobase = AX88180_BASE;
	dev->priv = priv;
	dev->init = ax88180_init;
	dev->halt = ax88180_halt;
	dev->send = ax88180_send;
	dev->recv = ax88180_recv;

	priv->BusWidth = BUS_WIDTH_32;
	priv->PadSize = 3;
#if defined (CONFIG_DRIVER_AX88180_16BIT)
	OUTW (dev, (START_BASE >> 8), BASE);
	OUTW (dev, DECODE_EN, DECODE);

	priv->BusWidth = BUS_WIDTH_16;
	priv->PadSize = 1;
#endif

	ax88180_mac_reset (dev);

	/* Disable interrupt */
	OUTW (dev, CLEAR_IMR, IMR);

	/* Disable AX88180 TX/RX functions */
	OUTW (dev, WAKEMOD, CMD);

	ax88180_read_mac_addr (dev);

	eth_register (dev);

	return ax88180_phy_initial (dev);

}
