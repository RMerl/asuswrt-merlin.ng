/*
Ported to U-Boot by Christian Pellegrin <chri@ascensit.com>

Based on sources from the Linux kernel (pcnet_cs.c, 8390.h) and
eCOS(if_dp83902a.c, if_dp83902a.h). Both of these 2 wonderful world
are GPL, so this is, of course, GPL.

==========================================================================

dev/if_dp83902a.c

Ethernet device driver for NS DP83902a ethernet controller

==========================================================================
####ECOSGPLCOPYRIGHTBEGIN####
-------------------------------------------
This file is part of eCos, the Embedded Configurable Operating System.
Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

eCos is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 or (at your option) any later version.

eCos is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with eCos; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

As a special exception, if other files instantiate templates or use macros
or inline functions from this file, or you compile this file and link it
with other works to produce a work based on this file, this file does not
by itself cause the resulting work to be covered by the GNU General Public
License. However the source code for this file must still be made available
in accordance with section (3) of the GNU General Public License.

This exception does not invalidate any other reasons why a work based on
this file might be covered by the GNU General Public License.

Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
at http://sources.redhat.com/ecos/ecos-license/
-------------------------------------------
####ECOSGPLCOPYRIGHTEND####
####BSDCOPYRIGHTBEGIN####

-------------------------------------------

Portions of this software may have been derived from OpenBSD or other sources,
and are covered by the appropriate copyright disclaimers included herein.

-------------------------------------------

####BSDCOPYRIGHTEND####
==========================================================================
#####DESCRIPTIONBEGIN####

Author(s):	gthomas
Contributors:	gthomas, jskov, rsandifo
Date:		2001-06-13
Purpose:
Description:

FIXME:		Will fail if pinged with large packets (1520 bytes)
Add promisc config
Add SNMP

####DESCRIPTIONEND####

==========================================================================
*/

#include <common.h>
#include <command.h>
#include <environment.h>
#include <net.h>
#include <malloc.h>
#include <linux/compiler.h>

/* forward definition of function used for the uboot interface */
void uboot_push_packet_len(int len);
void uboot_push_tx_done(int key, int val);

/* NE2000 base header file */
#include "ne2000_base.h"

#if defined(CONFIG_DRIVER_AX88796L)
/* AX88796L support */
#include "ax88796.h"
#else
/* Basic NE2000 chip support */
#include "ne2000.h"
#endif

static dp83902a_priv_data_t nic; /* just one instance of the card supported */

/**
 * This function reads the MAC address from the serial EEPROM,
 * used if PROM read fails. Does nothing for ax88796 chips (sh boards)
 */
static bool
dp83902a_init(unsigned char *enetaddr)
{
	dp83902a_priv_data_t *dp = &nic;
	u8* base;
#if defined(NE2000_BASIC_INIT)
	int i;
#endif

	DEBUG_FUNCTION();

	base = dp->base;
	if (!base)
		return false;	/* No device found */

	DEBUG_LINE();

#if defined(NE2000_BASIC_INIT)
	/* AX88796L doesn't need */
	/* Prepare ESA */
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1);	/* Select page 1 */
	/* Use the address from the serial EEPROM */
	for (i = 0; i < 6; i++)
		DP_IN(base, DP_P1_PAR0+i, dp->esa[i]);
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0);	/* Select page 0 */

	printf("NE2000 - %s ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
		"eeprom",
		dp->esa[0],
		dp->esa[1],
		dp->esa[2],
		dp->esa[3],
		dp->esa[4],
		dp->esa[5] );

	memcpy(enetaddr, dp->esa, 6); /* Use MAC from serial EEPROM */
#endif	/* NE2000_BASIC_INIT */
	return true;
}

static void
dp83902a_stop(void)
{
	dp83902a_priv_data_t *dp = &nic;
	u8 *base = dp->base;

	DEBUG_FUNCTION();

	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_STOP);	/* Brutal */
	DP_OUT(base, DP_ISR, 0xFF);		/* Clear any pending interrupts */
	DP_OUT(base, DP_IMR, 0x00);		/* Disable all interrupts */

	dp->running = false;
}

/*
 * This function is called to "start up" the interface. It may be called
 * multiple times, even when the hardware is already running. It will be
 * called whenever something "hardware oriented" changes and should leave
 * the hardware ready to send/receive packets.
 */
static void
dp83902a_start(u8 * enaddr)
{
	dp83902a_priv_data_t *dp = &nic;
	u8 *base = dp->base;
	int i;

	debug("The MAC is %pM\n", enaddr);

	DEBUG_FUNCTION();

	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_STOP); /* Brutal */
	DP_OUT(base, DP_DCR, DP_DCR_INIT);
	DP_OUT(base, DP_RBCH, 0);		/* Remote byte count */
	DP_OUT(base, DP_RBCL, 0);
	DP_OUT(base, DP_RCR, DP_RCR_MON);	/* Accept no packets */
	DP_OUT(base, DP_TCR, DP_TCR_LOCAL);	/* Transmitter [virtually] off */
	DP_OUT(base, DP_TPSR, dp->tx_buf1);	/* Transmitter start page */
	dp->tx1 = dp->tx2 = 0;
	dp->tx_next = dp->tx_buf1;
	dp->tx_started = false;
	dp->running = true;
	DP_OUT(base, DP_PSTART, dp->rx_buf_start); /* Receive ring start page */
	DP_OUT(base, DP_BNDRY, dp->rx_buf_end - 1); /* Receive ring boundary */
	DP_OUT(base, DP_PSTOP, dp->rx_buf_end);	/* Receive ring end page */
	dp->rx_next = dp->rx_buf_start - 1;
	dp->running = true;
	DP_OUT(base, DP_ISR, 0xFF);		/* Clear any pending interrupts */
	DP_OUT(base, DP_IMR, DP_IMR_All);	/* Enable all interrupts */
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1 | DP_CR_STOP);	/* Select page 1 */
	DP_OUT(base, DP_P1_CURP, dp->rx_buf_start);	/* Current page - next free page for Rx */
	dp->running = true;
	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		/* FIXME */
		/*((vu_short*)( base + ((DP_P1_PAR0 + i) * 2) +
		 * 0x1400)) = enaddr[i];*/
		DP_OUT(base, DP_P1_PAR0+i, enaddr[i]);
	}
	/* Enable and start device */
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
	DP_OUT(base, DP_TCR, DP_TCR_NORMAL); /* Normal transmit operations */
	DP_OUT(base, DP_RCR, DP_RCR_AB); /* Accept broadcast, no errors, no multicast */
	dp->running = true;
}

/*
 * This routine is called to start the transmitter. It is split out from the
 * data handling routine so it may be called either when data becomes first
 * available or when an Tx interrupt occurs
 */

static void
dp83902a_start_xmit(int start_page, int len)
{
	dp83902a_priv_data_t *dp = (dp83902a_priv_data_t *) &nic;
	u8 *base = dp->base;

	DEBUG_FUNCTION();

#if DEBUG & 1
	printf("Tx pkt %d len %d\n", start_page, len);
	if (dp->tx_started)
		printf("TX already started?!?\n");
#endif

	DP_OUT(base, DP_ISR, (DP_ISR_TxP | DP_ISR_TxE));
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
	DP_OUT(base, DP_TBCL, len & 0xFF);
	DP_OUT(base, DP_TBCH, len >> 8);
	DP_OUT(base, DP_TPSR, start_page);
	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_TXPKT | DP_CR_START);

	dp->tx_started = true;
}

/*
 * This routine is called to send data to the hardware. It is known a-priori
 * that there is free buffer space (dp->tx_next).
 */
static void
dp83902a_send(u8 *data, int total_len, u32 key)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	u8 *base = dp->base;
	int len, start_page, pkt_len, i, isr;
#if DEBUG & 4
	int dx;
#endif

	DEBUG_FUNCTION();

	len = pkt_len = total_len;
	if (pkt_len < IEEE_8023_MIN_FRAME)
		pkt_len = IEEE_8023_MIN_FRAME;

	start_page = dp->tx_next;
	if (dp->tx_next == dp->tx_buf1) {
		dp->tx1 = start_page;
		dp->tx1_len = pkt_len;
		dp->tx1_key = key;
		dp->tx_next = dp->tx_buf2;
	} else {
		dp->tx2 = start_page;
		dp->tx2_len = pkt_len;
		dp->tx2_key = key;
		dp->tx_next = dp->tx_buf1;
	}

#if DEBUG & 5
	printf("TX prep page %d len %d\n", start_page, pkt_len);
#endif

	DP_OUT(base, DP_ISR, DP_ISR_RDC);	/* Clear end of DMA */
	{
		/*
		 * Dummy read. The manual sez something slightly different,
		 * but the code is extended a bit to do what Hitachi's monitor
		 * does (i.e., also read data).
		 */

		__maybe_unused u16 tmp;
		int len = 1;

		DP_OUT(base, DP_RSAL, 0x100 - len);
		DP_OUT(base, DP_RSAH, (start_page - 1) & 0xff);
		DP_OUT(base, DP_RBCL, len);
		DP_OUT(base, DP_RBCH, 0);
		DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_RDMA | DP_CR_START);
		DP_IN_DATA(dp->data, tmp);
	}

#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA
	/*
	 * Stall for a bit before continuing to work around random data
	 * corruption problems on some platforms.
	 */
	CYGACC_CALL_IF_DELAY_US(1);
#endif

	/* Send data to device buffer(s) */
	DP_OUT(base, DP_RSAL, 0);
	DP_OUT(base, DP_RSAH, start_page);
	DP_OUT(base, DP_RBCL, pkt_len & 0xFF);
	DP_OUT(base, DP_RBCH, pkt_len >> 8);
	DP_OUT(base, DP_CR, DP_CR_WDMA | DP_CR_START);

	/* Put data into buffer */
#if DEBUG & 4
	printf(" sg buf %08lx len %08x\n ", (u32)data, len);
	dx = 0;
#endif
	while (len > 0) {
#if DEBUG & 4
		printf(" %02x", *data);
		if (0 == (++dx % 16)) printf("\n ");
#endif

		DP_OUT_DATA(dp->data, *data++);
		len--;
	}
#if DEBUG & 4
	printf("\n");
#endif
	if (total_len < pkt_len) {
#if DEBUG & 4
		printf("  + %d bytes of padding\n", pkt_len - total_len);
#endif
		/* Padding to 802.3 length was required */
		for (i = total_len; i < pkt_len;) {
			i++;
			DP_OUT_DATA(dp->data, 0);
		}
	}

#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA
	/*
	 * After last data write, delay for a bit before accessing the
	 * device again, or we may get random data corruption in the last
	 * datum (on some platforms).
	 */
	CYGACC_CALL_IF_DELAY_US(1);
#endif

	/* Wait for DMA to complete */
	do {
		DP_IN(base, DP_ISR, isr);
	} while ((isr & DP_ISR_RDC) == 0);

	/* Then disable DMA */
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);

	/* Start transmit if not already going */
	if (!dp->tx_started) {
		if (start_page == dp->tx1) {
			dp->tx_int = 1; /* Expecting interrupt from BUF1 */
		} else {
			dp->tx_int = 2; /* Expecting interrupt from BUF2 */
		}
		dp83902a_start_xmit(start_page, pkt_len);
	}
}

/*
 * This function is called when a packet has been received. It's job is
 * to prepare to unload the packet from the hardware. Once the length of
 * the packet is known, the upper layer of the driver can be told. When
 * the upper layer is ready to unload the packet, the internal function
 * 'dp83902a_recv' will be called to actually fetch it from the hardware.
 */
static void
dp83902a_RxEvent(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	u8 *base = dp->base;
	__maybe_unused u8 rsr;
	u8 rcv_hdr[4];
	int i, len, pkt, cur;

	DEBUG_FUNCTION();

	DP_IN(base, DP_RSR, rsr);
	while (true) {
		/* Read incoming packet header */
		DP_OUT(base, DP_CR, DP_CR_PAGE1 | DP_CR_NODMA | DP_CR_START);
		DP_IN(base, DP_P1_CURP, cur);
		DP_OUT(base, DP_P1_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
		DP_IN(base, DP_BNDRY, pkt);

		pkt += 1;
		if (pkt == dp->rx_buf_end)
			pkt = dp->rx_buf_start;

		if (pkt == cur) {
			break;
		}
		DP_OUT(base, DP_RBCL, sizeof(rcv_hdr));
		DP_OUT(base, DP_RBCH, 0);
		DP_OUT(base, DP_RSAL, 0);
		DP_OUT(base, DP_RSAH, pkt);
		if (dp->rx_next == pkt) {
			if (cur == dp->rx_buf_start)
				DP_OUT(base, DP_BNDRY, dp->rx_buf_end - 1);
			else
				DP_OUT(base, DP_BNDRY, cur - 1); /* Update pointer */
			return;
		}
		dp->rx_next = pkt;
		DP_OUT(base, DP_ISR, DP_ISR_RDC); /* Clear end of DMA */
		DP_OUT(base, DP_CR, DP_CR_RDMA | DP_CR_START);
#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_RX_DMA
		CYGACC_CALL_IF_DELAY_US(10);
#endif

		/* read header (get data size)*/
		for (i = 0; i < sizeof(rcv_hdr);) {
			DP_IN_DATA(dp->data, rcv_hdr[i++]);
		}

#if DEBUG & 5
		printf("rx hdr %02x %02x %02x %02x\n",
			rcv_hdr[0], rcv_hdr[1], rcv_hdr[2], rcv_hdr[3]);
#endif
		len = ((rcv_hdr[3] << 8) | rcv_hdr[2]) - sizeof(rcv_hdr);

		/* data read */
		uboot_push_packet_len(len);

		if (rcv_hdr[1] == dp->rx_buf_start)
			DP_OUT(base, DP_BNDRY, dp->rx_buf_end - 1);
		else
			DP_OUT(base, DP_BNDRY, rcv_hdr[1] - 1); /* Update pointer */
	}
}

/*
 * This function is called as a result of the "eth_drv_recv()" call above.
 * It's job is to actually fetch data for a packet from the hardware once
 * memory buffers have been allocated for the packet. Note that the buffers
 * may come in pieces, using a scatter-gather list. This allows for more
 * efficient processing in the upper layers of the stack.
 */
static void
dp83902a_recv(u8 *data, int len)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	u8 *base = dp->base;
	int i, mlen;
	u8 saved_char = 0;
	bool saved;
#if DEBUG & 4
	int dx;
#endif

	DEBUG_FUNCTION();

#if DEBUG & 5
	printf("Rx packet %d length %d\n", dp->rx_next, len);
#endif

	/* Read incoming packet data */
	DP_OUT(base, DP_CR, DP_CR_PAGE0 | DP_CR_NODMA | DP_CR_START);
	DP_OUT(base, DP_RBCL, len & 0xFF);
	DP_OUT(base, DP_RBCH, len >> 8);
	DP_OUT(base, DP_RSAL, 4);		/* Past header */
	DP_OUT(base, DP_RSAH, dp->rx_next);
	DP_OUT(base, DP_ISR, DP_ISR_RDC); /* Clear end of DMA */
	DP_OUT(base, DP_CR, DP_CR_RDMA | DP_CR_START);
#ifdef CYGHWR_NS_DP83902A_PLF_BROKEN_RX_DMA
	CYGACC_CALL_IF_DELAY_US(10);
#endif

	saved = false;
	for (i = 0; i < 1; i++) {
		if (data) {
			mlen = len;
#if DEBUG & 4
			printf(" sg buf %08lx len %08x \n", (u32) data, mlen);
			dx = 0;
#endif
			while (0 < mlen) {
				/* Saved byte from previous loop? */
				if (saved) {
					*data++ = saved_char;
					mlen--;
					saved = false;
					continue;
				}

				{
					u8 tmp;
					DP_IN_DATA(dp->data, tmp);
#if DEBUG & 4
					printf(" %02x", tmp);
					if (0 == (++dx % 16)) printf("\n ");
#endif
					*data++ = tmp;
					mlen--;
				}
			}
#if DEBUG & 4
			printf("\n");
#endif
		}
	}
}

static void
dp83902a_TxEvent(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	u8 *base = dp->base;
	__maybe_unused u8 tsr;
	u32 key;

	DEBUG_FUNCTION();

	DP_IN(base, DP_TSR, tsr);
	if (dp->tx_int == 1) {
		key = dp->tx1_key;
		dp->tx1 = 0;
	} else {
		key = dp->tx2_key;
		dp->tx2 = 0;
	}
	/* Start next packet if one is ready */
	dp->tx_started = false;
	if (dp->tx1) {
		dp83902a_start_xmit(dp->tx1, dp->tx1_len);
		dp->tx_int = 1;
	} else if (dp->tx2) {
		dp83902a_start_xmit(dp->tx2, dp->tx2_len);
		dp->tx_int = 2;
	} else {
		dp->tx_int = 0;
	}
	/* Tell higher level we sent this packet */
	uboot_push_tx_done(key, 0);
}

/*
 * Read the tally counters to clear them. Called in response to a CNT
 * interrupt.
 */
static void
dp83902a_ClearCounters(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	u8 *base = dp->base;
	__maybe_unused u8 cnt1, cnt2, cnt3;

	DP_IN(base, DP_FER, cnt1);
	DP_IN(base, DP_CER, cnt2);
	DP_IN(base, DP_MISSED, cnt3);
	DP_OUT(base, DP_ISR, DP_ISR_CNT);
}

/*
 * Deal with an overflow condition. This code follows the procedure set
 * out in section 7.0 of the datasheet.
 */
static void
dp83902a_Overflow(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *)&nic;
	u8 *base = dp->base;
	u8 isr;

	/* Issue a stop command and wait 1.6ms for it to complete. */
	DP_OUT(base, DP_CR, DP_CR_STOP | DP_CR_NODMA);
	CYGACC_CALL_IF_DELAY_US(1600);

	/* Clear the remote byte counter registers. */
	DP_OUT(base, DP_RBCL, 0);
	DP_OUT(base, DP_RBCH, 0);

	/* Enter loopback mode while we clear the buffer. */
	DP_OUT(base, DP_TCR, DP_TCR_LOCAL);
	DP_OUT(base, DP_CR, DP_CR_START | DP_CR_NODMA);

	/*
	 * Read in as many packets as we can and acknowledge any and receive
	 * interrupts. Since the buffer has overflowed, a receive event of
	 * some kind will have occurred.
	 */
	dp83902a_RxEvent();
	DP_OUT(base, DP_ISR, DP_ISR_RxP|DP_ISR_RxE);

	/* Clear the overflow condition and leave loopback mode. */
	DP_OUT(base, DP_ISR, DP_ISR_OFLW);
	DP_OUT(base, DP_TCR, DP_TCR_NORMAL);

	/*
	 * If a transmit command was issued, but no transmit event has occurred,
	 * restart it here.
	 */
	DP_IN(base, DP_ISR, isr);
	if (dp->tx_started && !(isr & (DP_ISR_TxP|DP_ISR_TxE))) {
		DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_TXPKT | DP_CR_START);
	}
}

static void
dp83902a_poll(void)
{
	struct dp83902a_priv_data *dp = (struct dp83902a_priv_data *) &nic;
	u8 *base = dp->base;
	u8 isr;

	DP_OUT(base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0 | DP_CR_START);
	DP_IN(base, DP_ISR, isr);
	while (0 != isr) {
		/*
		 * The CNT interrupt triggers when the MSB of one of the error
		 * counters is set. We don't much care about these counters, but
		 * we should read their values to reset them.
		 */
		if (isr & DP_ISR_CNT) {
			dp83902a_ClearCounters();
		}
		/*
		 * Check for overflow. It's a special case, since there's a
		 * particular procedure that must be followed to get back into
		 * a running state.a
		 */
		if (isr & DP_ISR_OFLW) {
			dp83902a_Overflow();
		} else {
			/*
			 * Other kinds of interrupts can be acknowledged simply by
			 * clearing the relevant bits of the ISR. Do that now, then
			 * handle the interrupts we care about.
			 */
			DP_OUT(base, DP_ISR, isr);	/* Clear set bits */
			if (!dp->running) break;	/* Is this necessary? */
			/*
			 * Check for tx_started on TX event since these may happen
			 * spuriously it seems.
			 */
			if (isr & (DP_ISR_TxP|DP_ISR_TxE) && dp->tx_started) {
				dp83902a_TxEvent();
			}
			if (isr & (DP_ISR_RxP|DP_ISR_RxE)) {
				dp83902a_RxEvent();
			}
		}
		DP_IN(base, DP_ISR, isr);
	}
}


/* U-Boot specific routines */
static u8 *pbuf = NULL;

static int pkey = -1;
static int initialized = 0;

void uboot_push_packet_len(int len) {
	PRINTK("pushed len = %d\n", len);
	if (len >= 2000) {
		printf("NE2000: packet too big\n");
		return;
	}
	dp83902a_recv(&pbuf[0], len);

	/*Just pass it to the upper layer*/
	net_process_received_packet(&pbuf[0], len);
}

void uboot_push_tx_done(int key, int val) {
	PRINTK("pushed key = %d\n", key);
	pkey = key;
}

/**
 * Setup the driver and init MAC address according to doc/README.enetaddr
 * Called by ne2k_register() before registering the driver @eth layer
 *
 * @param struct ethdevice of this instance of the driver for dev->enetaddr
 * @return 0 on success, -1 on error (causing caller to print error msg)
 */
static int ne2k_setup_driver(struct eth_device *dev)
{
	PRINTK("### ne2k_setup_driver\n");

	if (!pbuf) {
		pbuf = malloc(2000);
		if (!pbuf) {
			printf("Cannot allocate rx buffer\n");
			return -1;
		}
	}

#ifdef CONFIG_DRIVER_NE2000_CCR
	{
		vu_char *p = (vu_char *) CONFIG_DRIVER_NE2000_CCR;

		PRINTK("CCR before is %x\n", *p);
		*p = CONFIG_DRIVER_NE2000_VAL;
		PRINTK("CCR after is %x\n", *p);
	}
#endif

	nic.base = (u8 *) CONFIG_DRIVER_NE2000_BASE;

	nic.data = nic.base + DP_DATA;
	nic.tx_buf1 = START_PG;
	nic.tx_buf2 = START_PG2;
	nic.rx_buf_start = RX_START;
	nic.rx_buf_end = RX_END;

	/*
	 * According to doc/README.enetaddr, drivers shall give priority
	 * to the MAC address value in the environment, so we do not read
	 * it from the prom or eeprom if it is specified in the environment.
	 */
	if (!eth_env_get_enetaddr("ethaddr", dev->enetaddr)) {
		/* If the MAC address is not in the environment, get it: */
		if (!get_prom(dev->enetaddr, nic.base)) /* get MAC from prom */
			dp83902a_init(dev->enetaddr);   /* fallback: seeprom */
		/* And write it into the environment otherwise eth_write_hwaddr
		 * returns -1 due to eth_env_get_enetaddr_by_index() failing,
		 * and this causes "Warning: failed to set MAC address", and
		 * cmd_bdinfo has no ethaddr value which it can show: */
		eth_env_set_enetaddr("ethaddr", dev->enetaddr);
	}
	return 0;
}

static int ne2k_init(struct eth_device *dev, bd_t *bd)
{
	dp83902a_start(dev->enetaddr);
	initialized = 1;
	return 0;
}

static void ne2k_halt(struct eth_device *dev)
{
	debug("### ne2k_halt\n");
	if(initialized)
		dp83902a_stop();
	initialized = 0;
}

static int ne2k_recv(struct eth_device *dev)
{
	dp83902a_poll();
	return 1;
}

static int ne2k_send(struct eth_device *dev, void *packet, int length)
{
	int tmo;

	debug("### ne2k_send\n");

	pkey = -1;

	dp83902a_send((u8 *) packet, length, 666);
	tmo = get_timer (0) + TOUT * CONFIG_SYS_HZ;
	while(1) {
		dp83902a_poll();
		if (pkey != -1) {
			PRINTK("Packet sucesfully sent\n");
			return 0;
		}
		if (get_timer (0) >= tmo) {
			printf("transmission error (timoeut)\n");
			return 0;
		}

	}
	return 0;
}

/**
 * Setup the driver for use and register it with the eth layer
 * @return 0 on success, -1 on error (causing caller to print error msg)
 */
int ne2k_register(void)
{
	struct eth_device *dev;

	dev = calloc(sizeof(*dev), 1);
	if (dev == NULL)
		return -1;

	if (ne2k_setup_driver(dev))
		return -1;

	dev->init = ne2k_init;
	dev->halt = ne2k_halt;
	dev->send = ne2k_send;
	dev->recv = ne2k_recv;

	strcpy(dev->name, "NE2000");

	return eth_register(dev);
}
