// SPDX-License-Identifier: GPL-2.0+
/*------------------------------------------------------------------------
 . smc91111.c
 . This is a driver for SMSC's 91C111 single-chip Ethernet device.
 .
 . (C) Copyright 2002
 . Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 . Rolf Offermanns <rof@sysgo.de>
 .
 . Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 .	 Developed by Simple Network Magic Corporation (SNMC)
 . Copyright (C) 1996 by Erik Stahlman (ES)
 .
 .
 . Information contained in this file was obtained from the LAN91C111
 . manual from SMC.  To get a copy, if you really want one, you can find
 . information under www.smsc.com.
 .
 .
 . "Features" of the SMC chip:
 .   Integrated PHY/MAC for 10/100BaseT Operation
 .   Supports internal and external MII
 .   Integrated 8K packet memory
 .   EEPROM interface for configuration
 .
 . Arguments:
 .	io	= for the base address
 .	irq	= for the IRQ
 .
 . author:
 .	Erik Stahlman				( erik@vt.edu )
 .	Daris A Nevil				( dnevil@snmc.com )
 .
 .
 . Hardware multicast code from Peter Cammaert ( pc@denkart.be )
 .
 . Sources:
 .    o	  SMSC LAN91C111 databook (www.smsc.com)
 .    o	  smc9194.c by Erik Stahlman
 .    o	  skeleton.c by Donald Becker ( becker@cesdis.gsfc.nasa.gov )
 .
 . History:
 .	06/19/03  Richard Woodruff Made u-boot environment aware and added mac addr checks.
 .	10/17/01  Marco Hasewinkel Modify for DNP/1110
 .	07/25/01  Woojung Huh	   Modify for ADS Bitsy
 .	04/25/01  Daris A Nevil	   Initial public release through SMSC
 .	03/16/01  Daris A Nevil	   Modified smc9194.c for use with LAN91C111
 ----------------------------------------------------------------------------*/

#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include "smc91111.h"
#include <net.h>

/* Use power-down feature of the chip */
#define POWER_DOWN	0

#define NO_AUTOPROBE

#define SMC_DEBUG 0

#if SMC_DEBUG > 1
static const char version[] =
	"smc91111.c:v1.0 04/25/01 by Daris A Nevil (dnevil@snmc.com)\n";
#endif

/* Autonegotiation timeout in seconds */
#ifndef CONFIG_SMC_AUTONEG_TIMEOUT
#define CONFIG_SMC_AUTONEG_TIMEOUT 10
#endif

/*------------------------------------------------------------------------
 .
 . Configuration options, for the experienced user to change.
 .
 -------------------------------------------------------------------------*/

/*
 . Wait time for memory to be free.  This probably shouldn't be
 . tuned that much, as waiting for this means nothing else happens
 . in the system
*/
#define MEMORY_WAIT_TIME 16


#if (SMC_DEBUG > 2 )
#define PRINTK3(args...) printf(args)
#else
#define PRINTK3(args...)
#endif

#if SMC_DEBUG > 1
#define PRINTK2(args...) printf(args)
#else
#define PRINTK2(args...)
#endif

#ifdef SMC_DEBUG
#define PRINTK(args...) printf(args)
#else
#define PRINTK(args...)
#endif


/*------------------------------------------------------------------------
 .
 . The internal workings of the driver.	 If you are changing anything
 . here with the SMC stuff, you should have the datasheet and know
 . what you are doing.
 .
 -------------------------------------------------------------------------*/

/* Memory sizing constant */
#define LAN91C111_MEMORY_MULTIPLIER	(1024*2)

#ifndef CONFIG_SMC91111_BASE
#error "SMC91111 Base address must be passed to initialization funciton"
/* #define CONFIG_SMC91111_BASE 0x20000300 */
#endif

#define SMC_DEV_NAME "SMC91111"
#define SMC_PHY_ADDR 0x0000
#define SMC_ALLOC_MAX_TRY 5
#define SMC_TX_TIMEOUT 30

#define SMC_PHY_CLOCK_DELAY 1000

#define ETH_ZLEN 60

#ifdef	CONFIG_SMC_USE_32_BIT
#define USE_32_BIT  1
#else
#undef USE_32_BIT
#endif

#ifdef SHARED_RESOURCES
extern void swap_to(int device_id);
#else
# define swap_to(x)
#endif

#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_phy_configure(struct eth_device *dev);
#endif /* !CONFIG_SMC91111_EXT_PHY */

/*
 ------------------------------------------------------------
 .
 . Internal routines
 .
 ------------------------------------------------------------
*/

#ifdef CONFIG_SMC_USE_IOFUNCS
/*
 * input and output functions
 *
 * Implemented due to inx,outx macros accessing the device improperly
 * and putting the device into an unkown state.
 *
 * For instance, on Sharp LPD7A400 SDK, affects were chip memory
 * could not be free'd (hence the alloc failures), duplicate packets,
 * packets being corrupt (shifted) on the wire, etc.  Switching to the
 * inx,outx functions fixed this problem.
 */

static inline word SMC_inw(struct eth_device *dev, dword offset)
{
	word v;
	v = *((volatile word*)(dev->iobase + offset));
	barrier(); *(volatile u32*)(0xc0000000);
	return v;
}

static inline void SMC_outw(struct eth_device *dev, word value, dword offset)
{
	*((volatile word*)(dev->iobase + offset)) = value;
	barrier(); *(volatile u32*)(0xc0000000);
}

static inline byte SMC_inb(struct eth_device *dev, dword offset)
{
	word  _w;

	_w = SMC_inw(dev, offset & ~((dword)1));
	return (offset & 1) ? (byte)(_w >> 8) : (byte)(_w);
}

static inline void SMC_outb(struct eth_device *dev, byte value, dword offset)
{
	word  _w;

	_w = SMC_inw(dev, offset & ~((dword)1));
	if (offset & 1)
		*((volatile word*)(dev->iobase + (offset & ~((dword)1)))) =
			(value<<8) | (_w & 0x00ff);
	else
		*((volatile word*)(dev->iobase + offset)) =
			value | (_w & 0xff00);
}

static inline void SMC_insw(struct eth_device *dev, dword offset,
	volatile uchar* buf, dword len)
{
	volatile word *p = (volatile word *)buf;

	while (len-- > 0) {
		*p++ = SMC_inw(dev, offset);
		barrier();
		*((volatile u32*)(0xc0000000));
	}
}

static inline void SMC_outsw(struct eth_device *dev, dword offset,
	uchar* buf, dword len)
{
	volatile word *p = (volatile word *)buf;

	while (len-- > 0) {
		SMC_outw(dev, *p++, offset);
		barrier();
		*(volatile u32*)(0xc0000000);
	}
}
#endif  /* CONFIG_SMC_USE_IOFUNCS */

/*
 . A rather simple routine to print out a packet for debugging purposes.
*/
#if SMC_DEBUG > 2
static void print_packet( byte *, int );
#endif

#define tx_done(dev) 1

static int poll4int (struct eth_device *dev, byte mask, int timeout)
{
	int tmo = get_timer (0) + timeout * CONFIG_SYS_HZ;
	int is_timeout = 0;
	word old_bank = SMC_inw (dev, BSR_REG);

	PRINTK2 ("Polling...\n");
	SMC_SELECT_BANK (dev, 2);
	while ((SMC_inw (dev, SMC91111_INT_REG) & mask) == 0) {
		if (get_timer (0) >= tmo) {
			is_timeout = 1;
			break;
		}
	}

	/* restore old bank selection */
	SMC_SELECT_BANK (dev, old_bank);

	if (is_timeout)
		return 1;
	else
		return 0;
}

/* Only one release command at a time, please */
static inline void smc_wait_mmu_release_complete (struct eth_device *dev)
{
	int count = 0;

	/* assume bank 2 selected */
	while (SMC_inw (dev, MMU_CMD_REG) & MC_BUSY) {
		udelay (1);	/* Wait until not busy */
		if (++count > 200)
			break;
	}
}

/*
 . Function: smc_reset( void )
 . Purpose:
 .	This sets the SMC91111 chip to its normal state, hopefully from whatever
 .	mess that any other DOS driver has put it in.
 .
 . Maybe I should reset more registers to defaults in here?  SOFTRST  should
 . do that for me.
 .
 . Method:
 .	1.  send a SOFT RESET
 .	2.  wait for it to finish
 .	3.  enable autorelease mode
 .	4.  reset the memory management unit
 .	5.  clear all interrupts
 .
*/
static void smc_reset (struct eth_device *dev)
{
	PRINTK2 ("%s: smc_reset\n", SMC_DEV_NAME);

	/* This resets the registers mostly to defaults, but doesn't
	   affect EEPROM.  That seems unnecessary */
	SMC_SELECT_BANK (dev, 0);
	SMC_outw (dev, RCR_SOFTRST, RCR_REG);

	/* Setup the Configuration Register */
	/* This is necessary because the CONFIG_REG is not affected */
	/* by a soft reset */

	SMC_SELECT_BANK (dev, 1);
#if defined(CONFIG_SMC91111_EXT_PHY)
	SMC_outw (dev, CONFIG_DEFAULT | CONFIG_EXT_PHY, CONFIG_REG);
#else
	SMC_outw (dev, CONFIG_DEFAULT, CONFIG_REG);
#endif


	/* Release from possible power-down state */
	/* Configuration register is not affected by Soft Reset */
	SMC_outw (dev, SMC_inw (dev, CONFIG_REG) | CONFIG_EPH_POWER_EN,
		CONFIG_REG);

	SMC_SELECT_BANK (dev, 0);

	/* this should pause enough for the chip to be happy */
	udelay (10);

	/* Disable transmit and receive functionality */
	SMC_outw (dev, RCR_CLEAR, RCR_REG);
	SMC_outw (dev, TCR_CLEAR, TCR_REG);

	/* set the control register */
	SMC_SELECT_BANK (dev, 1);
	SMC_outw (dev, CTL_DEFAULT, CTL_REG);

	/* Reset the MMU */
	SMC_SELECT_BANK (dev, 2);
	smc_wait_mmu_release_complete (dev);
	SMC_outw (dev, MC_RESET, MMU_CMD_REG);
	while (SMC_inw (dev, MMU_CMD_REG) & MC_BUSY)
		udelay (1);	/* Wait until not busy */

	/* Note:  It doesn't seem that waiting for the MMU busy is needed here,
	   but this is a place where future chipsets _COULD_ break.  Be wary
	   of issuing another MMU command right after this */

	/* Disable all interrupts */
	SMC_outb (dev, 0, IM_REG);
}

/*
 . Function: smc_enable
 . Purpose: let the chip talk to the outside work
 . Method:
 .	1.  Enable the transmitter
 .	2.  Enable the receiver
 .	3.  Enable interrupts
*/
static void smc_enable(struct eth_device *dev)
{
	PRINTK2("%s: smc_enable\n", SMC_DEV_NAME);
	SMC_SELECT_BANK( dev, 0 );
	/* see the header file for options in TCR/RCR DEFAULT*/
	SMC_outw( dev, TCR_DEFAULT, TCR_REG );
	SMC_outw( dev, RCR_DEFAULT, RCR_REG );

	/* clear MII_DIS */
/*	smc_write_phy_register(PHY_CNTL_REG, 0x0000); */
}

/*
 . Function: smc_halt
 . Purpose:  closes down the SMC91xxx chip.
 . Method:
 .	1. zero the interrupt mask
 .	2. clear the enable receive flag
 .	3. clear the enable xmit flags
 .
 . TODO:
 .   (1) maybe utilize power down mode.
 .	Why not yet?  Because while the chip will go into power down mode,
 .	the manual says that it will wake up in response to any I/O requests
 .	in the register space.	 Empirical results do not show this working.
*/
static void smc_halt(struct eth_device *dev)
{
	PRINTK2("%s: smc_halt\n", SMC_DEV_NAME);

	/* no more interrupts for me */
	SMC_SELECT_BANK( dev, 2 );
	SMC_outb( dev, 0, IM_REG );

	/* and tell the card to stay away from that nasty outside world */
	SMC_SELECT_BANK( dev, 0 );
	SMC_outb( dev, RCR_CLEAR, RCR_REG );
	SMC_outb( dev, TCR_CLEAR, TCR_REG );

	swap_to(FLASH);
}


/*
 . Function:  smc_send(struct net_device * )
 . Purpose:
 .	This sends the actual packet to the SMC9xxx chip.
 .
 . Algorithm:
 .	First, see if a saved_skb is available.
 .		( this should NOT be called if there is no 'saved_skb'
 .	Now, find the packet number that the chip allocated
 .	Point the data pointers at it in memory
 .	Set the length word in the chip's memory
 .	Dump the packet to chip memory
 .	Check if a last byte is needed ( odd length packet )
 .		if so, set the control flag right
 .	Tell the card to send it
 .	Enable the transmit interrupt, so I know if it failed
 .	Free the kernel data if I actually sent it.
*/
static int smc_send(struct eth_device *dev, void *packet, int packet_length)
{
	byte packet_no;
	byte *buf;
	int length;
	int numPages;
	int try = 0;
	int time_out;
	byte status;
	byte saved_pnr;
	word saved_ptr;

	/* save PTR and PNR registers before manipulation */
	SMC_SELECT_BANK (dev, 2);
	saved_pnr = SMC_inb( dev, PN_REG );
	saved_ptr = SMC_inw( dev, PTR_REG );

	PRINTK3 ("%s: smc_hardware_send_packet\n", SMC_DEV_NAME);

	length = ETH_ZLEN < packet_length ? packet_length : ETH_ZLEN;

	/* allocate memory
	 ** The MMU wants the number of pages to be the number of 256 bytes
	 ** 'pages', minus 1 ( since a packet can't ever have 0 pages :) )
	 **
	 ** The 91C111 ignores the size bits, but the code is left intact
	 ** for backwards and future compatibility.
	 **
	 ** Pkt size for allocating is data length +6 (for additional status
	 ** words, length and ctl!)
	 **
	 ** If odd size then last byte is included in this header.
	 */
	numPages = ((length & 0xfffe) + 6);
	numPages >>= 8;		/* Divide by 256 */

	if (numPages > 7) {
		printf ("%s: Far too big packet error. \n", SMC_DEV_NAME);
		return 0;
	}

	/* now, try to allocate the memory */
	SMC_SELECT_BANK (dev, 2);
	SMC_outw (dev, MC_ALLOC | numPages, MMU_CMD_REG);

	/* FIXME: the ALLOC_INT bit never gets set *
	 * so the following will always give a	   *
	 * memory allocation error.		   *
	 * same code works in armboot though	   *
	 * -ro
	 */

again:
	try++;
	time_out = MEMORY_WAIT_TIME;
	do {
		status = SMC_inb (dev, SMC91111_INT_REG);
		if (status & IM_ALLOC_INT) {
			/* acknowledge the interrupt */
			SMC_outb (dev, IM_ALLOC_INT, SMC91111_INT_REG);
			break;
		}
	} while (--time_out);

	if (!time_out) {
		PRINTK2 ("%s: memory allocation, try %d failed ...\n",
			 SMC_DEV_NAME, try);
		if (try < SMC_ALLOC_MAX_TRY)
			goto again;
		else
			return 0;
	}

	PRINTK2 ("%s: memory allocation, try %d succeeded ...\n",
		 SMC_DEV_NAME, try);

	buf = (byte *) packet;

	/* If I get here, I _know_ there is a packet slot waiting for me */
	packet_no = SMC_inb (dev, AR_REG);
	if (packet_no & AR_FAILED) {
		/* or isn't there?  BAD CHIP! */
		printf ("%s: Memory allocation failed. \n", SMC_DEV_NAME);
		return 0;
	}

	/* we have a packet address, so tell the card to use it */
	SMC_outb (dev, packet_no, PN_REG);

	/* do not write new ptr value if Write data fifo not empty */
	while ( saved_ptr & PTR_NOTEMPTY )
		printf ("Write data fifo not empty!\n");

	/* point to the beginning of the packet */
	SMC_outw (dev, PTR_AUTOINC, PTR_REG);

	PRINTK3 ("%s: Trying to xmit packet of length %x\n",
		 SMC_DEV_NAME, length);

#if SMC_DEBUG > 2
	printf ("Transmitting Packet\n");
	print_packet (buf, length);
#endif

	/* send the packet length ( +6 for status, length and ctl byte )
	   and the status word ( set to zeros ) */
#ifdef USE_32_BIT
	SMC_outl (dev, (length + 6) << 16, SMC91111_DATA_REG);
#else
	SMC_outw (dev, 0, SMC91111_DATA_REG);
	/* send the packet length ( +6 for status words, length, and ctl */
	SMC_outw (dev, (length + 6), SMC91111_DATA_REG);
#endif

	/* send the actual data
	   . I _think_ it's faster to send the longs first, and then
	   . mop up by sending the last word.  It depends heavily
	   . on alignment, at least on the 486.	 Maybe it would be
	   . a good idea to check which is optimal?  But that could take
	   . almost as much time as is saved?
	 */
#ifdef USE_32_BIT
	SMC_outsl (dev, SMC91111_DATA_REG, buf, length >> 2);
	if (length & 0x2)
		SMC_outw (dev, *((word *) (buf + (length & 0xFFFFFFFC))),
			  SMC91111_DATA_REG);
#else
	SMC_outsw (dev, SMC91111_DATA_REG, buf, (length) >> 1);
#endif /* USE_32_BIT */

	/* Send the last byte, if there is one.	  */
	if ((length & 1) == 0) {
		SMC_outw (dev, 0, SMC91111_DATA_REG);
	} else {
		SMC_outw (dev, buf[length - 1] | 0x2000, SMC91111_DATA_REG);
	}

	/* and let the chipset deal with it */
	SMC_outw (dev, MC_ENQUEUE, MMU_CMD_REG);

	/* poll for TX INT */
	/* if (poll4int (dev, IM_TX_INT, SMC_TX_TIMEOUT)) { */
	/* poll for TX_EMPTY INT - autorelease enabled */
	if (poll4int(dev, IM_TX_EMPTY_INT, SMC_TX_TIMEOUT)) {
		/* sending failed */
		PRINTK2 ("%s: TX timeout, sending failed...\n", SMC_DEV_NAME);

		/* release packet */
		/* no need to release, MMU does that now */

		/* wait for MMU getting ready (low) */
		while (SMC_inw (dev, MMU_CMD_REG) & MC_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


		return 0;
	} else {
		/* ack. int */
		SMC_outb (dev, IM_TX_EMPTY_INT, SMC91111_INT_REG);
		/* SMC_outb (IM_TX_INT, SMC91111_INT_REG); */
		PRINTK2 ("%s: Sent packet of length %d \n", SMC_DEV_NAME,
			 length);

		/* release packet */
		/* no need to release, MMU does that now */

		/* wait for MMU getting ready (low) */
		while (SMC_inw (dev, MMU_CMD_REG) & MC_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


	}

	/* restore previously saved registers */
	SMC_outb( dev, saved_pnr, PN_REG );
	SMC_outw( dev, saved_ptr, PTR_REG );

	return length;
}

static int smc_write_hwaddr(struct eth_device *dev)
{
	int i;

	swap_to(ETHERNET);
	SMC_SELECT_BANK (dev, 1);
#ifdef USE_32_BIT
	for (i = 0; i < 6; i += 2) {
		word address;

		address = dev->enetaddr[i + 1] << 8;
		address |= dev->enetaddr[i];
		SMC_outw(dev, address, (ADDR0_REG + i));
	}
#else
	for (i = 0; i < 6; i++)
		SMC_outb(dev, dev->enetaddr[i], (ADDR0_REG + i));
#endif
	swap_to(FLASH);
	return 0;
}

/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int smc_init(struct eth_device *dev, bd_t *bd)
{
	swap_to(ETHERNET);

	PRINTK2 ("%s: smc_init\n", SMC_DEV_NAME);

	/* reset the hardware */
	smc_reset (dev);
	smc_enable (dev);

	/* Configure the PHY */
#ifndef CONFIG_SMC91111_EXT_PHY
	smc_phy_configure (dev);
#endif

	/* conservative setting (10Mbps, HalfDuplex, no AutoNeg.) */
/*	SMC_SELECT_BANK(dev, 0); */
/*	SMC_outw(dev, 0, RPC_REG); */

	printf(SMC_DEV_NAME ": MAC %pM\n", dev->enetaddr);

	return 0;
}

/*-------------------------------------------------------------
 .
 . smc_rcv -  receive a packet from the card
 .
 . There is ( at least ) a packet waiting to be read from
 . chip-memory.
 .
 . o Read the status
 . o If an error, record it
 . o otherwise, read in the packet
 --------------------------------------------------------------
*/
static int smc_rcv(struct eth_device *dev)
{
	int	packet_number;
	word	status;
	word	packet_length;
	int	is_error = 0;
#ifdef USE_32_BIT
	dword stat_len;
#endif
	byte saved_pnr;
	word saved_ptr;

	SMC_SELECT_BANK(dev, 2);
	/* save PTR and PTR registers */
	saved_pnr = SMC_inb( dev, PN_REG );
	saved_ptr = SMC_inw( dev, PTR_REG );

	packet_number = SMC_inw( dev, RXFIFO_REG );

	if ( packet_number & RXFIFO_REMPTY ) {

		return 0;
	}

	PRINTK3("%s: smc_rcv\n", SMC_DEV_NAME);
	/*  start reading from the start of the packet */
	SMC_outw( dev, PTR_READ | PTR_RCV | PTR_AUTOINC, PTR_REG );

	/* First two words are status and packet_length */
#ifdef USE_32_BIT
	stat_len = SMC_inl(dev, SMC91111_DATA_REG);
	status = stat_len & 0xffff;
	packet_length = stat_len >> 16;
#else
	status		= SMC_inw( dev, SMC91111_DATA_REG );
	packet_length	= SMC_inw( dev, SMC91111_DATA_REG );
#endif

	packet_length &= 0x07ff;  /* mask off top bits */

	PRINTK2("RCV: STATUS %4x LENGTH %4x\n", status, packet_length );

	if ( !(status & RS_ERRORS ) ){
		/* Adjust for having already read the first two words */
		packet_length -= 4; /*4; */


		/* set odd length for bug in LAN91C111, */
		/* which never sets RS_ODDFRAME */
		/* TODO ? */


#ifdef USE_32_BIT
		PRINTK3(" Reading %d dwords (and %d bytes)\n",
			packet_length >> 2, packet_length & 3 );
		/* QUESTION:  Like in the TX routine, do I want
		   to send the DWORDs or the bytes first, or some
		   mixture.  A mixture might improve already slow PIO
		   performance	*/
		SMC_insl(dev, SMC91111_DATA_REG, net_rx_packets[0],
			 packet_length >> 2);
		/* read the left over bytes */
		if (packet_length & 3) {
			int i;

			byte *tail = (byte *)(net_rx_packets[0] +
				(packet_length & ~3));
			dword leftover = SMC_inl(dev, SMC91111_DATA_REG);
			for (i=0; i<(packet_length & 3); i++)
				*tail++ = (byte) (leftover >> (8*i)) & 0xff;
		}
#else
		PRINTK3(" Reading %d words and %d byte(s)\n",
			(packet_length >> 1 ), packet_length & 1 );
		SMC_insw(dev, SMC91111_DATA_REG , net_rx_packets[0],
			 packet_length >> 1);

#endif /* USE_32_BIT */

#if	SMC_DEBUG > 2
		printf("Receiving Packet\n");
		print_packet(net_rx_packets[0], packet_length);
#endif
	} else {
		/* error ... */
		/* TODO ? */
		is_error = 1;
	}

	while ( SMC_inw( dev, MMU_CMD_REG ) & MC_BUSY )
		udelay(1); /* Wait until not busy */

	/*  error or good, tell the card to get rid of this packet */
	SMC_outw( dev, MC_RELEASE, MMU_CMD_REG );

	while ( SMC_inw( dev, MMU_CMD_REG ) & MC_BUSY )
		udelay(1); /* Wait until not busy */

	/* restore saved registers */
	SMC_outb( dev, saved_pnr, PN_REG );
	SMC_outw( dev, saved_ptr, PTR_REG );

	if (!is_error) {
		/* Pass the packet up to the protocol layers. */
		net_process_received_packet(net_rx_packets[0], packet_length);
		return packet_length;
	} else {
		return 0;
	}

}


#if 0
/*------------------------------------------------------------
 . Modify a bit in the LAN91C111 register set
 .-------------------------------------------------------------*/
static word smc_modify_regbit(struct eth_device *dev, int bank, int ioaddr, int reg,
	unsigned int bit, int val)
{
	word regval;

	SMC_SELECT_BANK( dev, bank );

	regval = SMC_inw( dev, reg );
	if (val)
		regval |= bit;
	else
		regval &= ~bit;

	SMC_outw( dev, regval, 0 );
	return(regval);
}


/*------------------------------------------------------------
 . Retrieve a bit in the LAN91C111 register set
 .-------------------------------------------------------------*/
static int smc_get_regbit(struct eth_device *dev, int bank, int ioaddr, int reg, unsigned int bit)
{
	SMC_SELECT_BANK( dev, bank );
	if ( SMC_inw( dev, reg ) & bit)
		return(1);
	else
		return(0);
}


/*------------------------------------------------------------
 . Modify a LAN91C111 register (word access only)
 .-------------------------------------------------------------*/
static void smc_modify_reg(struct eth_device *dev, int bank, int ioaddr, int reg, word val)
{
	SMC_SELECT_BANK( dev, bank );
	SMC_outw( dev, val, reg );
}


/*------------------------------------------------------------
 . Retrieve a LAN91C111 register (word access only)
 .-------------------------------------------------------------*/
static int smc_get_reg(struct eth_device *dev, int bank, int ioaddr, int reg)
{
	SMC_SELECT_BANK( dev, bank );
	return(SMC_inw( dev, reg ));
}

#endif /* 0 */

/*---PHY CONTROL AND CONFIGURATION----------------------------------------- */

#if (SMC_DEBUG > 2 )

/*------------------------------------------------------------
 . Debugging function for viewing MII Management serial bitstream
 .-------------------------------------------------------------*/
static void smc_dump_mii_stream (byte * bits, int size)
{
	int i;

	printf ("BIT#:");
	for (i = 0; i < size; ++i) {
		printf ("%d", i % 10);
	}

	printf ("\nMDOE:");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDOE)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\nMDO :");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDO)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\nMDI :");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDI)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\n");
}
#endif

/*------------------------------------------------------------
 . Reads a register from the MII Management serial interface
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static word smc_read_phy_register (struct eth_device *dev, byte phyreg)
{
	int oldBank;
	int i;
	byte mask;
	word mii_reg;
	byte bits[64];
	int clk_idx = 0;
	int input_idx;
	word phydata;
	byte phyaddr = SMC_PHY_ADDR;

	/* 32 consecutive ones on MDO to establish sync */
	for (i = 0; i < 32; ++i)
		bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Start code <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Read command <10> */
	bits[clk_idx++] = MII_MDOE | MII_MDO;
	bits[clk_idx++] = MII_MDOE;

	/* Output the PHY address, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyaddr & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Output the phy register number, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyreg & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Tristate and turnaround (2 bit times) */
	bits[clk_idx++] = 0;
	/*bits[clk_idx++] = 0; */

	/* Input starts at this bit time */
	input_idx = clk_idx;

	/* Will input 16 bits */
	for (i = 0; i < 16; ++i)
		bits[clk_idx++] = 0;

	/* Final clock bit */
	bits[clk_idx++] = 0;

	/* Save the current bank */
	oldBank = SMC_inw (dev, BANK_SELECT);

	/* Select bank 3 */
	SMC_SELECT_BANK (dev, 3);

	/* Get the current MII register value */
	mii_reg = SMC_inw (dev, MII_REG);

	/* Turn off all MII Interface bits */
	mii_reg &= ~(MII_MDOE | MII_MCLK | MII_MDI | MII_MDO);

	/* Clock all 64 cycles */
	for (i = 0; i < sizeof bits; ++i) {
		/* Clock Low - output data */
		SMC_outw (dev, mii_reg | bits[i], MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);


		/* Clock Hi - input data */
		SMC_outw (dev, mii_reg | bits[i] | MII_MCLK, MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);
		bits[i] |= SMC_inw (dev, MII_REG) & MII_MDI;
	}

	/* Return to idle state */
	/* Set clock to low, data to low, and output tristated */
	SMC_outw (dev, mii_reg, MII_REG);
	udelay (SMC_PHY_CLOCK_DELAY);

	/* Restore original bank select */
	SMC_SELECT_BANK (dev, oldBank);

	/* Recover input data */
	phydata = 0;
	for (i = 0; i < 16; ++i) {
		phydata <<= 1;

		if (bits[input_idx++] & MII_MDI)
			phydata |= 0x0001;
	}

#if (SMC_DEBUG > 2 )
	printf ("smc_read_phy_register(): phyaddr=%x,phyreg=%x,phydata=%x\n",
		phyaddr, phyreg, phydata);
	smc_dump_mii_stream (bits, sizeof bits);
#endif

	return (phydata);
}


/*------------------------------------------------------------
 . Writes a register to the MII Management serial interface
 .-------------------------------------------------------------*/
static void smc_write_phy_register (struct eth_device *dev, byte phyreg,
	word phydata)
{
	int oldBank;
	int i;
	word mask;
	word mii_reg;
	byte bits[65];
	int clk_idx = 0;
	byte phyaddr = SMC_PHY_ADDR;

	/* 32 consecutive ones on MDO to establish sync */
	for (i = 0; i < 32; ++i)
		bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Start code <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Write command <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Output the PHY address, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyaddr & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Output the phy register number, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyreg & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Tristate and turnaround (2 bit times) */
	bits[clk_idx++] = 0;
	bits[clk_idx++] = 0;

	/* Write out 16 bits of data, msb first */
	mask = 0x8000;
	for (i = 0; i < 16; ++i) {
		if (phydata & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Final clock bit (tristate) */
	bits[clk_idx++] = 0;

	/* Save the current bank */
	oldBank = SMC_inw (dev, BANK_SELECT);

	/* Select bank 3 */
	SMC_SELECT_BANK (dev, 3);

	/* Get the current MII register value */
	mii_reg = SMC_inw (dev, MII_REG);

	/* Turn off all MII Interface bits */
	mii_reg &= ~(MII_MDOE | MII_MCLK | MII_MDI | MII_MDO);

	/* Clock all cycles */
	for (i = 0; i < sizeof bits; ++i) {
		/* Clock Low - output data */
		SMC_outw (dev, mii_reg | bits[i], MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);


		/* Clock Hi - input data */
		SMC_outw (dev, mii_reg | bits[i] | MII_MCLK, MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);
		bits[i] |= SMC_inw (dev, MII_REG) & MII_MDI;
	}

	/* Return to idle state */
	/* Set clock to low, data to low, and output tristated */
	SMC_outw (dev, mii_reg, MII_REG);
	udelay (SMC_PHY_CLOCK_DELAY);

	/* Restore original bank select */
	SMC_SELECT_BANK (dev, oldBank);

#if (SMC_DEBUG > 2 )
	printf ("smc_write_phy_register(): phyaddr=%x,phyreg=%x,phydata=%x\n",
		phyaddr, phyreg, phydata);
	smc_dump_mii_stream (bits, sizeof bits);
#endif
}
#endif /* !CONFIG_SMC91111_EXT_PHY */


/*------------------------------------------------------------
 . Configures the specified PHY using Autonegotiation. Calls
 . smc_phy_fixed() if the user has requested a certain config.
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_phy_configure (struct eth_device *dev)
{
	int timeout;
	word my_phy_caps;	/* My PHY capabilities */
	word my_ad_caps;	/* My Advertised capabilities */
	word status = 0;	/*;my status = 0 */

	PRINTK3 ("%s: smc_program_phy()\n", SMC_DEV_NAME);

	/* Reset the PHY, setting all other bits to zero */
	smc_write_phy_register (dev, PHY_CNTL_REG, PHY_CNTL_RST);

	/* Wait for the reset to complete, or time out */
	timeout = 6;		/* Wait up to 3 seconds */
	while (timeout--) {
		if (!(smc_read_phy_register (dev, PHY_CNTL_REG)
		      & PHY_CNTL_RST)) {
			/* reset complete */
			break;
		}

		mdelay(500);	/* wait 500 millisecs */
	}

	if (timeout < 1) {
		printf ("%s:PHY reset timed out\n", SMC_DEV_NAME);
		goto smc_phy_configure_exit;
	}

	/* Read PHY Register 18, Status Output */
	/* lp->lastPhy18 = smc_read_phy_register(PHY_INT_REG); */

	/* Enable PHY Interrupts (for register 18) */
	/* Interrupts listed here are disabled */
	smc_write_phy_register (dev, PHY_MASK_REG, 0xffff);

	/* Configure the Receive/Phy Control register */
	SMC_SELECT_BANK (dev, 0);
	SMC_outw (dev, RPC_DEFAULT, RPC_REG);

	/* Copy our capabilities from PHY_STAT_REG to PHY_AD_REG */
	my_phy_caps = smc_read_phy_register (dev, PHY_STAT_REG);
	my_ad_caps = PHY_AD_CSMA;	/* I am CSMA capable */

	if (my_phy_caps & PHY_STAT_CAP_T4)
		my_ad_caps |= PHY_AD_T4;

	if (my_phy_caps & PHY_STAT_CAP_TXF)
		my_ad_caps |= PHY_AD_TX_FDX;

	if (my_phy_caps & PHY_STAT_CAP_TXH)
		my_ad_caps |= PHY_AD_TX_HDX;

	if (my_phy_caps & PHY_STAT_CAP_TF)
		my_ad_caps |= PHY_AD_10_FDX;

	if (my_phy_caps & PHY_STAT_CAP_TH)
		my_ad_caps |= PHY_AD_10_HDX;

	/* Update our Auto-Neg Advertisement Register */
	smc_write_phy_register (dev, PHY_AD_REG, my_ad_caps);

	/* Read the register back.  Without this, it appears that when */
	/* auto-negotiation is restarted, sometimes it isn't ready and */
	/* the link does not come up. */
	smc_read_phy_register(dev, PHY_AD_REG);

	PRINTK2 ("%s: phy caps=%x\n", SMC_DEV_NAME, my_phy_caps);
	PRINTK2 ("%s: phy advertised caps=%x\n", SMC_DEV_NAME, my_ad_caps);

	/* Restart auto-negotiation process in order to advertise my caps */
	smc_write_phy_register (dev, PHY_CNTL_REG,
				PHY_CNTL_ANEG_EN | PHY_CNTL_ANEG_RST);

	/* Wait for the auto-negotiation to complete.  This may take from */
	/* 2 to 3 seconds. */
	/* Wait for the reset to complete, or time out */
	timeout = CONFIG_SMC_AUTONEG_TIMEOUT * 2;
	while (timeout--) {

		status = smc_read_phy_register (dev, PHY_STAT_REG);
		if (status & PHY_STAT_ANEG_ACK) {
			/* auto-negotiate complete */
			break;
		}

		mdelay(500);	/* wait 500 millisecs */

		/* Restart auto-negotiation if remote fault */
		if (status & PHY_STAT_REM_FLT) {
			printf ("%s: PHY remote fault detected\n",
				SMC_DEV_NAME);

			/* Restart auto-negotiation */
			printf ("%s: PHY restarting auto-negotiation\n",
				SMC_DEV_NAME);
			smc_write_phy_register (dev, PHY_CNTL_REG,
						PHY_CNTL_ANEG_EN |
						PHY_CNTL_ANEG_RST |
						PHY_CNTL_SPEED |
						PHY_CNTL_DPLX);
		}
	}

	if (timeout < 1) {
		printf ("%s: PHY auto-negotiate timed out\n", SMC_DEV_NAME);
	}

	/* Fail if we detected an auto-negotiate remote fault */
	if (status & PHY_STAT_REM_FLT) {
		printf ("%s: PHY remote fault detected\n", SMC_DEV_NAME);
	}

	/* Re-Configure the Receive/Phy Control register */
	SMC_outw (dev, RPC_DEFAULT, RPC_REG);

smc_phy_configure_exit:	;

}
#endif /* !CONFIG_SMC91111_EXT_PHY */


#if SMC_DEBUG > 2
static void print_packet( byte * buf, int length )
{
	int i;
	int remainder;
	int lines;

	printf("Packet of length %d \n", length );

#if SMC_DEBUG > 3
	lines = length / 16;
	remainder = length % 16;

	for ( i = 0; i < lines ; i ++ ) {
		int cur;

		for ( cur = 0; cur < 8; cur ++ ) {
			byte a, b;

			a = *(buf ++ );
			b = *(buf ++ );
			printf("%02x%02x ", a, b );
		}
		printf("\n");
	}
	for ( i = 0; i < remainder/2 ; i++ ) {
		byte a, b;

		a = *(buf ++ );
		b = *(buf ++ );
		printf("%02x%02x ", a, b );
	}
	printf("\n");
#endif
}
#endif

int smc91111_initialize(u8 dev_num, int base_addr)
{
	struct smc91111_priv *priv;
	struct eth_device *dev;
	int i;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return 0;
	dev = malloc(sizeof(*dev));
	if (!dev) {
		free(priv);
		return 0;
	}

	memset(dev, 0, sizeof(*dev));
	priv->dev_num = dev_num;
	dev->priv = priv;
	dev->iobase = base_addr;

	swap_to(ETHERNET);
	SMC_SELECT_BANK(dev, 1);
	for (i = 0; i < 6; ++i)
		dev->enetaddr[i] = SMC_inb(dev, (ADDR0_REG + i));
	swap_to(FLASH);

	dev->init = smc_init;
	dev->halt = smc_halt;
	dev->send = smc_send;
	dev->recv = smc_rcv;
	dev->write_hwaddr = smc_write_hwaddr;
	sprintf(dev->name, "%s-%hu", SMC_DEV_NAME, dev_num);

	eth_register(dev);
	return 0;
}
