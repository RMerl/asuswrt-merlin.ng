// SPDX-License-Identifier: GPL-2.0+
/*------------------------------------------------------------------------
 * lan91c96.c
 * This is a driver for SMSC's LAN91C96 single-chip Ethernet device, based
 * on the SMC91111 driver from U-Boot.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Rolf Offermanns <rof@sysgo.de>
 *
 * Copyright (C) 2001 Standard Microsystems Corporation (SMSC)
 *       Developed by Simple Network Magic Corporation (SNMC)
 * Copyright (C) 1996 by Erik Stahlman (ES)
 *
 * Information contained in this file was obtained from the LAN91C96
 * manual from SMC.  To get a copy, if you really want one, you can find
 * information under www.smsc.com.
 *
 * "Features" of the SMC chip:
 *   6144 byte packet memory. ( for the 91C96 )
 *   EEPROM for configuration
 *   AUI/TP selection  ( mine has 10Base2/10BaseT select )
 *
 * Arguments:
 *	io	= for the base address
 *	irq	= for the IRQ
 *
 * author:
 *	Erik Stahlman				( erik@vt.edu )
 *	Daris A Nevil				( dnevil@snmc.com )
 *
 *
 * Hardware multicast code from Peter Cammaert ( pc@denkart.be )
 *
 * Sources:
 *    o   SMSC LAN91C96 databook (www.smsc.com)
 *    o   smc91111.c (u-boot driver)
 *    o   smc9194.c (linux kernel driver)
 *    o   lan91c96.c (Intel Diagnostic Manager driver)
 *
 * History:
 *	04/30/03  Mathijs Haarman	Modified smc91111.c (u-boot version)
 *					for lan91c96
 *---------------------------------------------------------------------------
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include "lan91c96.h"
#include <net.h>
#include <linux/compiler.h>

/*------------------------------------------------------------------------
 *
 * Configuration options, for the experienced user to change.
 *
 -------------------------------------------------------------------------*/

/* Use power-down feature of the chip */
#define POWER_DOWN	0

/*
 * Wait time for memory to be free.  This probably shouldn't be
 * tuned that much, as waiting for this means nothing else happens
 * in the system
*/
#define MEMORY_WAIT_TIME 16

#define SMC_DEBUG 0

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
 *
 * The internal workings of the driver.  If you are changing anything
 * here with the SMC stuff, you should have the datasheet and know
 * what you are doing.
 *
 *------------------------------------------------------------------------
 */
#define DRIVER_NAME "LAN91C96"
#define SMC_ALLOC_MAX_TRY 5
#define SMC_TX_TIMEOUT 30

#define ETH_ZLEN 60

#ifdef  CONFIG_LAN91C96_USE_32_BIT
#define USE_32_BIT  1
#else
#undef USE_32_BIT
#endif

/* See if a MAC address is defined in the current environment. If so use it. If not
 . print a warning and set the environment and other globals with the default.
 . If an EEPROM is present it really should be consulted.
*/
static int smc_get_ethaddr(bd_t *bd, struct eth_device *dev);
static int get_rom_mac(struct eth_device *dev, unsigned char *v_rom_mac);

/* ------------------------------------------------------------
 * Internal routines
 * ------------------------------------------------------------
 */

static unsigned char smc_mac_addr[] = { 0xc0, 0x00, 0x00, 0x1b, 0x62, 0x9c };

/*
 * This function must be called before smc_open() if you want to override
 * the default mac address.
 */

static void smc_set_mac_addr(const unsigned char *addr)
{
	int i;

	for (i = 0; i < sizeof (smc_mac_addr); i++) {
		smc_mac_addr[i] = addr[i];
	}
}

/***********************************************
 * Show available memory                       *
 ***********************************************/
void dump_memory_info(struct eth_device *dev)
{
	__maybe_unused word mem_info;
	word old_bank;

	old_bank = SMC_inw(dev, LAN91C96_BANK_SELECT) & 0xF;

	SMC_SELECT_BANK(dev, 0);
	mem_info = SMC_inw(dev, LAN91C96_MIR);
	PRINTK2 ("Memory: %4d available\n", (mem_info >> 8) * 2048);

	SMC_SELECT_BANK(dev, old_bank);
}

/*
 * A rather simple routine to print out a packet for debugging purposes.
 */
#if SMC_DEBUG > 2
static void print_packet (byte *, int);
#endif

static int poll4int (struct eth_device *dev, byte mask, int timeout)
{
	int tmo = get_timer (0) + timeout * CONFIG_SYS_HZ;
	int is_timeout = 0;
	word old_bank = SMC_inw(dev, LAN91C96_BANK_SELECT);

	PRINTK2 ("Polling...\n");
	SMC_SELECT_BANK(dev, 2);
	while ((SMC_inw(dev, LAN91C96_INT_STATS) & mask) == 0) {
		if (get_timer (0) >= tmo) {
			is_timeout = 1;
			break;
		}
	}

	/* restore old bank selection */
	SMC_SELECT_BANK(dev, old_bank);

	if (is_timeout)
		return 1;
	else
		return 0;
}

/*
 * Function: smc_reset
 * Purpose:
 *	This sets the SMC91111 chip to its normal state, hopefully from whatever
 *	mess that any other DOS driver has put it in.
 *
 * Maybe I should reset more registers to defaults in here?  SOFTRST  should
 * do that for me.
 *
 * Method:
 *	1.  send a SOFT RESET
 *	2.  wait for it to finish
 *	3.  enable autorelease mode
 *	4.  reset the memory management unit
 *	5.  clear all interrupts
 *
*/
static void smc_reset(struct eth_device *dev)
{
	PRINTK2("%s:smc_reset\n", dev->name);

	/* This resets the registers mostly to defaults, but doesn't
	   affect EEPROM.  That seems unnecessary */
	SMC_SELECT_BANK(dev, 0);
	SMC_outw(dev, LAN91C96_RCR_SOFT_RST, LAN91C96_RCR);

	udelay (10);

	/* Disable transmit and receive functionality */
	SMC_outw(dev, 0, LAN91C96_RCR);
	SMC_outw(dev, 0, LAN91C96_TCR);

	/* set the control register */
	SMC_SELECT_BANK(dev, 1);
	SMC_outw(dev, SMC_inw(dev, LAN91C96_CONTROL) | LAN91C96_CTR_BIT_8,
			  LAN91C96_CONTROL);

	/* Disable all interrupts */
	SMC_outb(dev, 0, LAN91C96_INT_MASK);
}

/*
 * Function: smc_enable
 * Purpose: let the chip talk to the outside work
 * Method:
 *	1.  Initialize the Memory Configuration Register
 *	2.  Enable the transmitter
 *	3.  Enable the receiver
*/
static void smc_enable(struct eth_device *dev)
{
	PRINTK2("%s:smc_enable\n", dev->name);
	SMC_SELECT_BANK(dev, 0);

	/* Initialize the Memory Configuration Register. See page
	   49 of the LAN91C96 data sheet for details. */
	SMC_outw(dev, LAN91C96_MCR_TRANSMIT_PAGES, LAN91C96_MCR);

	/* Initialize the Transmit Control Register */
	SMC_outw(dev, LAN91C96_TCR_TXENA, LAN91C96_TCR);
	/* Initialize the Receive Control Register
	 * FIXME:
	 * The promiscuous bit set because I could not receive ARP reply
	 * packets from the server when I send a ARP request. It only works
	 * when I set the promiscuous bit
	 */
	SMC_outw(dev, LAN91C96_RCR_RXEN | LAN91C96_RCR_PRMS, LAN91C96_RCR);
}

/*
 * Function: smc_shutdown
 * Purpose:  closes down the SMC91xxx chip.
 * Method:
 *	1. zero the interrupt mask
 *	2. clear the enable receive flag
 *	3. clear the enable xmit flags
 *
 * TODO:
 *   (1) maybe utilize power down mode.
 *	Why not yet?  Because while the chip will go into power down mode,
 *	the manual says that it will wake up in response to any I/O requests
 *	in the register space.   Empirical results do not show this working.
 */
static void smc_shutdown(struct eth_device *dev)
{
	PRINTK2("%s:smc_shutdown\n", dev->name);

	/* no more interrupts for me */
	SMC_SELECT_BANK(dev, 2);
	SMC_outb(dev, 0, LAN91C96_INT_MASK);

	/* and tell the card to stay away from that nasty outside world */
	SMC_SELECT_BANK(dev, 0);
	SMC_outb(dev, 0, LAN91C96_RCR);
	SMC_outb(dev, 0, LAN91C96_TCR);
}


/*
 * Function:  smc_hardware_send_packet(struct net_device * )
 * Purpose:
 *	This sends the actual packet to the SMC9xxx chip.
 *
 * Algorithm:
 *	First, see if a saved_skb is available.
 *		( this should NOT be called if there is no 'saved_skb'
 *	Now, find the packet number that the chip allocated
 *	Point the data pointers at it in memory
 *	Set the length word in the chip's memory
 *	Dump the packet to chip memory
 *	Check if a last byte is needed ( odd length packet )
 *		if so, set the control flag right
 *	Tell the card to send it
 *	Enable the transmit interrupt, so I know if it failed
 *	Free the kernel data if I actually sent it.
 */
static int smc_send_packet(struct eth_device *dev, void *packet,
		int packet_length)
{
	byte packet_no;
	byte *buf;
	int length;
	int numPages;
	int try = 0;
	int time_out;
	byte status;


	PRINTK3("%s:smc_hardware_send_packet\n", dev->name);

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
	numPages >>= 8;				/* Divide by 256 */

	if (numPages > 7) {
		printf("%s: Far too big packet error. \n", dev->name);
		return 0;
	}

	/* now, try to allocate the memory */

	SMC_SELECT_BANK(dev, 2);
	SMC_outw(dev, LAN91C96_MMUCR_ALLOC_TX | numPages, LAN91C96_MMU);

  again:
	try++;
	time_out = MEMORY_WAIT_TIME;
	do {
		status = SMC_inb(dev, LAN91C96_INT_STATS);
		if (status & LAN91C96_IST_ALLOC_INT) {

			SMC_outb(dev, LAN91C96_IST_ALLOC_INT,
					LAN91C96_INT_STATS);
			break;
		}
	} while (--time_out);

	if (!time_out) {
		PRINTK2 ("%s: memory allocation, try %d failed ...\n",
				 dev->name, try);
		if (try < SMC_ALLOC_MAX_TRY)
			goto again;
		else
			return 0;
	}

	PRINTK2 ("%s: memory allocation, try %d succeeded ...\n",
			 dev->name, try);

	/* I can send the packet now.. */
	buf = (byte *) packet;

	/* If I get here, I _know_ there is a packet slot waiting for me */
	packet_no = SMC_inb(dev, LAN91C96_ARR);
	if (packet_no & LAN91C96_ARR_FAILED) {
		/* or isn't there?  BAD CHIP! */
		printf("%s: Memory allocation failed. \n", dev->name);
		return 0;
	}

	/* we have a packet address, so tell the card to use it */
	SMC_outb(dev, packet_no, LAN91C96_PNR);

	/* point to the beginning of the packet */
	SMC_outw(dev, LAN91C96_PTR_AUTO_INCR, LAN91C96_POINTER);

	PRINTK3("%s: Trying to xmit packet of length %x\n",
			 dev->name, length);

#if SMC_DEBUG > 2
	printf ("Transmitting Packet\n");
	print_packet (buf, length);
#endif

	/* send the packet length ( +6 for status, length and ctl byte )
	   and the status word ( set to zeros ) */
#ifdef USE_32_BIT
	SMC_outl(dev, (length + 6) << 16, LAN91C96_DATA_HIGH);
#else
	SMC_outw(dev, 0, LAN91C96_DATA_HIGH);
	/* send the packet length ( +6 for status words, length, and ctl */
	SMC_outw(dev, (length + 6), LAN91C96_DATA_HIGH);
#endif /* USE_32_BIT */

	/* send the actual data
	 * I _think_ it's faster to send the longs first, and then
	 * mop up by sending the last word.  It depends heavily
	 * on alignment, at least on the 486.  Maybe it would be
	 * a good idea to check which is optimal?  But that could take
	 * almost as much time as is saved?
	 */
#ifdef USE_32_BIT
	SMC_outsl(dev, LAN91C96_DATA_HIGH, buf, length >> 2);
	if (length & 0x2)
		SMC_outw(dev, *((word *) (buf + (length & 0xFFFFFFFC))),
				  LAN91C96_DATA_HIGH);
#else
	SMC_outsw(dev, LAN91C96_DATA_HIGH, buf, (length) >> 1);
#endif /* USE_32_BIT */

	/* Send the last byte, if there is one.   */
	if ((length & 1) == 0) {
		SMC_outw(dev, 0, LAN91C96_DATA_HIGH);
	} else {
		SMC_outw(dev, buf[length - 1] | 0x2000, LAN91C96_DATA_HIGH);
	}

	/* and let the chipset deal with it */
	SMC_outw(dev, LAN91C96_MMUCR_ENQUEUE, LAN91C96_MMU);

	/* poll for TX INT */
	if (poll4int (dev, LAN91C96_MSK_TX_INT, SMC_TX_TIMEOUT)) {
		/* sending failed */
		PRINTK2("%s: TX timeout, sending failed...\n", dev->name);

		/* release packet */
		SMC_outw(dev, LAN91C96_MMUCR_RELEASE_TX, LAN91C96_MMU);

		/* wait for MMU getting ready (low) */
		while (SMC_inw(dev, LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY)
			udelay (10);

		PRINTK2("MMU ready\n");


		return 0;
	} else {
		/* ack. int */
		SMC_outw(dev, LAN91C96_IST_TX_INT, LAN91C96_INT_STATS);

		PRINTK2("%s: Sent packet of length %d \n", dev->name, length);

		/* release packet */
		SMC_outw(dev, LAN91C96_MMUCR_RELEASE_TX, LAN91C96_MMU);

		/* wait for MMU getting ready (low) */
		while (SMC_inw(dev, LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY)
			udelay (10);

		PRINTK2 ("MMU ready\n");
	}

	return length;
}


/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int smc_open(bd_t *bd, struct eth_device *dev)
{
	int i, err;			/* used to set hw ethernet address */

	PRINTK2("%s:smc_open\n", dev->name);

	/* reset the hardware */

	smc_reset(dev);
	smc_enable(dev);

	SMC_SELECT_BANK(dev, 1);
	/* set smc_mac_addr, and sync it with u-boot globals */
	err = smc_get_ethaddr(bd, dev);
	if (err < 0)
		return -1;
#ifdef USE_32_BIT
	for (i = 0; i < 6; i += 2) {
		word address;

		address = smc_mac_addr[i + 1] << 8;
		address |= smc_mac_addr[i];
		SMC_outw(dev, address, LAN91C96_IA0 + i);
	}
#else
	for (i = 0; i < 6; i++)
		SMC_outb(dev, smc_mac_addr[i], LAN91C96_IA0 + i);
#endif
	return 0;
}

/*-------------------------------------------------------------
 *
 * smc_rcv -  receive a packet from the card
 *
 * There is ( at least ) a packet waiting to be read from
 * chip-memory.
 *
 * o Read the status
 * o If an error, record it
 * o otherwise, read in the packet
 *-------------------------------------------------------------
 */
static int smc_rcv(struct eth_device *dev)
{
	int packet_number;
	word status;
	word packet_length;
	int is_error = 0;

#ifdef USE_32_BIT
	dword stat_len;
#endif


	SMC_SELECT_BANK(dev, 2);
	packet_number = SMC_inw(dev, LAN91C96_FIFO);

	if (packet_number & LAN91C96_FIFO_RXEMPTY) {
		return 0;
	}

	PRINTK3("%s:smc_rcv\n", dev->name);
	/*  start reading from the start of the packet */
	SMC_outw(dev, LAN91C96_PTR_READ | LAN91C96_PTR_RCV |
			  LAN91C96_PTR_AUTO_INCR, LAN91C96_POINTER);

	/* First two words are status and packet_length */
#ifdef USE_32_BIT
	stat_len = SMC_inl(dev, LAN91C96_DATA_HIGH);
	status = stat_len & 0xffff;
	packet_length = stat_len >> 16;
#else
	status = SMC_inw(dev, LAN91C96_DATA_HIGH);
	packet_length = SMC_inw(dev, LAN91C96_DATA_HIGH);
#endif

	packet_length &= 0x07ff;	/* mask off top bits */

	PRINTK2 ("RCV: STATUS %4x LENGTH %4x\n", status, packet_length);

	if (!(status & FRAME_FILTER)) {
		/* Adjust for having already read the first two words */
		packet_length -= 4;		/*4; */


		/* set odd length for bug in LAN91C111, */
		/* which never sets RS_ODDFRAME */
		/* TODO ? */


#ifdef USE_32_BIT
		PRINTK3 (" Reading %d dwords (and %d bytes) \n",
			 packet_length >> 2, packet_length & 3);
		/* QUESTION:  Like in the TX routine, do I want
		   to send the DWORDs or the bytes first, or some
		   mixture.  A mixture might improve already slow PIO
		   performance  */
		SMC_insl(dev, LAN91C96_DATA_HIGH, net_rx_packets[0],
			 packet_length >> 2);
		/* read the left over bytes */
		if (packet_length & 3) {
			int i;

			byte *tail = (byte *)(net_rx_packets[0] +
				(packet_length & ~3));
			dword leftover = SMC_inl(dev, LAN91C96_DATA_HIGH);

			for (i = 0; i < (packet_length & 3); i++)
				*tail++ = (byte) (leftover >> (8 * i)) & 0xff;
		}
#else
		PRINTK3(" Reading %d words and %d byte(s)\n",
			(packet_length >> 1), packet_length & 1);
		SMC_insw(dev, LAN91C96_DATA_HIGH, net_rx_packets[0],
			 packet_length >> 1);

#endif /* USE_32_BIT */

#if	SMC_DEBUG > 2
		printf ("Receiving Packet\n");
		print_packet((byte *)net_rx_packets[0], packet_length);
#endif
	} else {
		/* error ... */
		/* TODO ? */
		is_error = 1;
	}

	while (SMC_inw(dev, LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY)
		udelay (1);		/* Wait until not busy */

	/*  error or good, tell the card to get rid of this packet */
	SMC_outw(dev, LAN91C96_MMUCR_RELEASE_RX, LAN91C96_MMU);

	while (SMC_inw(dev, LAN91C96_MMU) & LAN91C96_MMUCR_NO_BUSY)
		udelay (1);		/* Wait until not busy */

	if (!is_error) {
		/* Pass the packet up to the protocol layers. */
		net_process_received_packet(net_rx_packets[0], packet_length);
		return packet_length;
	} else {
		return 0;
	}

}

/*----------------------------------------------------
 * smc_close
 *
 * this makes the board clean up everything that it can
 * and not talk to the outside world.   Caused by
 * an 'ifconfig ethX down'
 *
 -----------------------------------------------------*/
static int smc_close(struct eth_device *dev)
{
	PRINTK2("%s:smc_close\n", dev->name);

	/* clear everything */
	smc_shutdown(dev);

	return 0;
}

#if SMC_DEBUG > 2
static void print_packet(byte *buf, int length)
{
#if 0
	int i;
	int remainder;
	int lines;

	printf ("Packet of length %d \n", length);

	lines = length / 16;
	remainder = length % 16;

	for (i = 0; i < lines; i++) {
		int cur;

		for (cur = 0; cur < 8; cur++) {
			byte a, b;

			a = *(buf++);
			b = *(buf++);
			printf ("%02x%02x ", a, b);
		}
		printf ("\n");
	}
	for (i = 0; i < remainder / 2; i++) {
		byte a, b;

		a = *(buf++);
		b = *(buf++);
		printf ("%02x%02x ", a, b);
	}
	printf ("\n");
#endif /* 0 */
}
#endif /* SMC_DEBUG > 2 */

static int  lan91c96_init(struct eth_device *dev, bd_t *bd)
{
	return smc_open(bd, dev);
}

static void lan91c96_halt(struct eth_device *dev)
{
	smc_close(dev);
}

static int lan91c96_recv(struct eth_device *dev)
{
	return smc_rcv(dev);
}

static int lan91c96_send(struct eth_device *dev, void *packet,
		int length)
{
	return smc_send_packet(dev, packet, length);
}

/* smc_get_ethaddr
 *
 * This checks both the environment and the ROM for an ethernet address. If
 * found, the environment takes precedence.
 */

static int smc_get_ethaddr(bd_t *bd, struct eth_device *dev)
{
	uchar v_mac[6];

	if (!eth_env_get_enetaddr("ethaddr", v_mac)) {
		/* get ROM mac value if any */
		if (!get_rom_mac(dev, v_mac)) {
			printf("\n*** ERROR: ethaddr is NOT set !!\n");
			return -1;
		}
		eth_env_set_enetaddr("ethaddr", v_mac);
	}

	smc_set_mac_addr(v_mac); /* use old function to update smc default */
	PRINTK("Using MAC Address %pM\n", v_mac);
	return 0;
}

/*
 * get_rom_mac()
 * Note, this has omly been tested for the OMAP730 P2.
 */

static int get_rom_mac(struct eth_device *dev, unsigned char *v_rom_mac)
{
	int i;
	SMC_SELECT_BANK(dev, 1);
	for (i=0; i<6; i++)
	{
		v_rom_mac[i] = SMC_inb(dev, LAN91C96_IA0 + i);
	}
	return (1);
}

/* Structure to detect the device IDs */
struct id_type {
	u8 id;
	char *name;
};
static struct id_type supported_chips[] = {
	{0, ""}, /* Dummy entry to prevent id check failure */
	{9, "LAN91C110"},
	{8, "LAN91C100FD"},
	{7, "LAN91C100"},
	{5, "LAN91C95"},
	{4, "LAN91C94/96"},
	{3, "LAN91C90/92"},
};
/* lan91c96_detect_chip
 * See:
 * http://www.embeddedsys.com/subpages/resources/images/documents/LAN91C96_datasheet.pdf
 * page 71 - that is the closest we get to detect this device
 */
static int lan91c96_detect_chip(struct eth_device *dev)
{
	u8 chip_id;
	int r;
	SMC_SELECT_BANK(dev, 3);
	chip_id = (SMC_inw(dev, 0xA) & LAN91C96_REV_CHIPID) >> 4;
	SMC_SELECT_BANK(dev, 0);
	for (r = 0; r < ARRAY_SIZE(supported_chips); r++)
		if (chip_id == supported_chips[r].id)
			return r;
	return 0;
}

int lan91c96_initialize(u8 dev_num, int base_addr)
{
	struct eth_device *dev;
	int r = 0;

	dev = malloc(sizeof(*dev));
	if (!dev) {
		return 0;
	}
	memset(dev, 0, sizeof(*dev));

	dev->iobase = base_addr;

	/* Try to detect chip. Will fail if not present. */
	r = lan91c96_detect_chip(dev);
	if (!r) {
		free(dev);
		return 0;
	}
	get_rom_mac(dev, dev->enetaddr);

	dev->init = lan91c96_init;
	dev->halt = lan91c96_halt;
	dev->send = lan91c96_send;
	dev->recv = lan91c96_recv;
	sprintf(dev->name, "%s-%hu", supported_chips[r].name, dev_num);

	eth_register(dev);
	return 0;
}
