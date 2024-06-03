/*
Ported to U-Boot by Christian Pellegrin <chri@ascensit.com>

Based on sources from the Linux kernel (pcnet_cs.c, 8390.h) and
eCOS(if_dp83902a.c, if_dp83902a.h). Both of these 2 wonderful world
are GPL, so this is, of course, GPL.


==========================================================================

	dev/dp83902a.h

	National Semiconductor DP83902a ethernet chip

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
 Contributors:	gthomas, jskov
 Date:		2001-06-13
 Purpose:
 Description:

####DESCRIPTIONEND####

==========================================================================

*/

/*
 ------------------------------------------------------------------------
 Macros for accessing DP registers
 These can be overridden by the platform header
*/

#ifndef __NE2000_BASE_H__
#define __NE2000_BASE_H__

/*
 * Debugging details
 *
 * Set to perms of:
 * 0 disables all debug output
 * 1 for process debug output
 * 2 for added data IO output: get_reg, put_reg
 * 4 for packet allocation/free output
 * 8 for only startup status, so we can tell we're installed OK
 */
#if 0
#define DEBUG 0xf
#else
#define DEBUG 0
#endif

#if DEBUG & 1
#define DEBUG_FUNCTION() do { printf("%s\n", __FUNCTION__); } while (0)
#define DEBUG_LINE() do { printf("%d\n", __LINE__); } while (0)
#define PRINTK(args...) printf(args)
#else
#define DEBUG_FUNCTION() do {} while(0)
#define DEBUG_LINE() do {} while(0)
#define PRINTK(args...)
#endif

/* timeout for tx/rx in s */
#define TOUT 5
/* Ether MAC address size */
#define ETHER_ADDR_LEN 6


#define CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA 1
#define CYGACC_CALL_IF_DELAY_US(X) udelay(X)

/* H/W infomation struct */
typedef struct hw_info_t {
	u32 offset;
	u8 a0, a1, a2;
	u32 flags;
} hw_info_t;

typedef struct dp83902a_priv_data {
	u8* base;
	u8* data;
	u8* reset;
	int tx_next;		/* First free Tx page */
	int tx_int;		/* Expecting interrupt from this buffer */
	int rx_next;		/* First free Rx page */
	int tx1, tx2;		/* Page numbers for Tx buffers */
	u32 tx1_key, tx2_key;	/* Used to ack when packet sent */
	int tx1_len, tx2_len;
	bool tx_started, running, hardwired_esa;
	u8 esa[6];
	void* plf_priv;

	/* Buffer allocation */
	int tx_buf1, tx_buf2;
	int rx_buf_start, rx_buf_end;
} dp83902a_priv_data_t;

/* ------------------------------------------------------------------------ */
/* Register offsets */

#define DP_CR		0x00
#define DP_CLDA0	0x01
#define DP_PSTART	0x01	/* write */
#define DP_CLDA1	0x02
#define DP_PSTOP	0x02	/* write */
#define DP_BNDRY	0x03
#define DP_TSR		0x04
#define DP_TPSR		0x04	/* write */
#define DP_NCR		0x05
#define DP_TBCL		0x05	/* write */
#define DP_FIFO		0x06
#define DP_TBCH		0x06	/* write */
#define DP_ISR		0x07
#define DP_CRDA0	0x08
#define DP_RSAL		0x08	/* write */
#define DP_CRDA1	0x09
#define DP_RSAH		0x09	/* write */
#define DP_RBCL		0x0a	/* write */
#define DP_RBCH		0x0b	/* write */
#define DP_RSR		0x0c
#define DP_RCR		0x0c	/* write */
#define DP_FER		0x0d
#define DP_TCR		0x0d	/* write */
#define DP_CER		0x0e
#define DP_DCR		0x0e	/* write */
#define DP_MISSED	0x0f
#define DP_IMR		0x0f	/* write */
#define DP_DATAPORT	0x10	/* "eprom" data port */

#define DP_P1_CR	0x00
#define DP_P1_PAR0	0x01
#define DP_P1_PAR1	0x02
#define DP_P1_PAR2	0x03
#define DP_P1_PAR3	0x04
#define DP_P1_PAR4	0x05
#define DP_P1_PAR5	0x06
#define DP_P1_CURP	0x07
#define DP_P1_MAR0	0x08
#define DP_P1_MAR1	0x09
#define DP_P1_MAR2	0x0a
#define DP_P1_MAR3	0x0b
#define DP_P1_MAR4	0x0c
#define DP_P1_MAR5	0x0d
#define DP_P1_MAR6	0x0e
#define DP_P1_MAR7	0x0f

#define DP_P2_CR	0x00
#define DP_P2_PSTART	0x01
#define DP_P2_CLDA0	0x01	/* write */
#define DP_P2_PSTOP	0x02
#define DP_P2_CLDA1	0x02	/* write */
#define DP_P2_RNPP	0x03
#define DP_P2_TPSR	0x04
#define DP_P2_LNPP	0x05
#define DP_P2_ACH	0x06
#define DP_P2_ACL	0x07
#define DP_P2_RCR	0x0c
#define DP_P2_TCR	0x0d
#define DP_P2_DCR	0x0e
#define DP_P2_IMR	0x0f

/* Command register - common to all pages */

#define DP_CR_STOP	0x01	/* Stop: software reset */
#define DP_CR_START	0x02	/* Start: initialize device */
#define DP_CR_TXPKT	0x04	/* Transmit packet */
#define DP_CR_RDMA	0x08	/* Read DMA (recv data from device) */
#define DP_CR_WDMA	0x10	/* Write DMA (send data to device) */
#define DP_CR_SEND	0x18	/* Send packet */
#define DP_CR_NODMA	0x20	/* Remote (or no) DMA */
#define DP_CR_PAGE0	0x00	/* Page select */
#define DP_CR_PAGE1	0x40
#define DP_CR_PAGE2	0x80
#define DP_CR_PAGEMSK	0x3F	/* Used to mask out page bits */

/* Data configuration register */

#define DP_DCR_WTS	0x01	/* 1=16 bit word transfers */
#define DP_DCR_BOS	0x02	/* 1=Little Endian */
#define DP_DCR_LAS	0x04	/* 1=Single 32 bit DMA mode */
#define DP_DCR_LS	0x08	/* 1=normal mode, 0=loopback */
#define DP_DCR_ARM	0x10	/* 0=no send command (program I/O) */
#define DP_DCR_FIFO_1	0x00	/* FIFO threshold */
#define DP_DCR_FIFO_2	0x20
#define DP_DCR_FIFO_4	0x40
#define DP_DCR_FIFO_6	0x60

#define DP_DCR_INIT	(DP_DCR_LS|DP_DCR_FIFO_4)

/* Interrupt status register */

#define DP_ISR_RxP	0x01	/* Packet received */
#define DP_ISR_TxP	0x02	/* Packet transmitted */
#define DP_ISR_RxE	0x04	/* Receive error */
#define DP_ISR_TxE	0x08	/* Transmit error */
#define DP_ISR_OFLW	0x10	/* Receive overflow */
#define DP_ISR_CNT	0x20	/* Tally counters need emptying */
#define DP_ISR_RDC	0x40	/* Remote DMA complete */
#define DP_ISR_RESET	0x80	/* Device has reset (shutdown, error) */

/* Interrupt mask register */

#define DP_IMR_RxP	0x01	/* Packet received */
#define DP_IMR_TxP	0x02	/* Packet transmitted */
#define DP_IMR_RxE	0x04	/* Receive error */
#define DP_IMR_TxE	0x08	/* Transmit error */
#define DP_IMR_OFLW	0x10	/* Receive overflow */
#define DP_IMR_CNT	0x20	/* Tall counters need emptying */
#define DP_IMR_RDC	0x40	/* Remote DMA complete */

#define DP_IMR_All	0x3F	/* Everything but remote DMA */

/* Receiver control register */

#define DP_RCR_SEP	0x01	/* Save bad(error) packets */
#define DP_RCR_AR	0x02	/* Accept runt packets */
#define DP_RCR_AB	0x04	/* Accept broadcast packets */
#define DP_RCR_AM	0x08	/* Accept multicast packets */
#define DP_RCR_PROM	0x10	/* Promiscuous mode */
#define DP_RCR_MON	0x20	/* Monitor mode - 1=accept no packets */

/* Receiver status register */

#define DP_RSR_RxP	0x01	/* Packet received */
#define DP_RSR_CRC	0x02	/* CRC error */
#define DP_RSR_FRAME	0x04	/* Framing error */
#define DP_RSR_FO	0x08	/* FIFO overrun */
#define DP_RSR_MISS	0x10	/* Missed packet */
#define DP_RSR_PHY	0x20	/* 0=pad match, 1=mad match */
#define DP_RSR_DIS	0x40	/* Receiver disabled */
#define DP_RSR_DFR	0x80	/* Receiver processing deferred */

/* Transmitter control register */

#define DP_TCR_NOCRC	0x01	/* 1=inhibit CRC */
#define DP_TCR_NORMAL	0x00	/* Normal transmitter operation */
#define DP_TCR_LOCAL	0x02	/* Internal NIC loopback */
#define DP_TCR_INLOOP	0x04	/* Full internal loopback */
#define DP_TCR_OUTLOOP	0x08	/* External loopback */
#define DP_TCR_ATD	0x10	/* Auto transmit disable */
#define DP_TCR_OFFSET	0x20	/* Collision offset adjust */

/* Transmit status register */

#define DP_TSR_TxP	0x01	/* Packet transmitted */
#define DP_TSR_COL	0x04	/* Collision (at least one) */
#define DP_TSR_ABT	0x08	/* Aborted because of too many collisions */
#define DP_TSR_CRS	0x10	/* Lost carrier */
#define DP_TSR_FU	0x20	/* FIFO underrun */
#define DP_TSR_CDH	0x40	/* Collision Detect Heartbeat */
#define DP_TSR_OWC	0x80	/* Collision outside normal window */

#define IEEE_8023_MAX_FRAME	1518	/* Largest possible ethernet frame */
#define IEEE_8023_MIN_FRAME	64	/* Smallest possible ethernet frame */

/* Functions */
int get_prom(u8* mac_addr, u8* base_addr);

#endif /* __NE2000_BASE_H__ */
