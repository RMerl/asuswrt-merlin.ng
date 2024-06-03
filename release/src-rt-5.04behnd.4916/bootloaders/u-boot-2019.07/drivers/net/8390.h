/*

Ported to U-Boot  by Christian Pellegrin <chri@ascensit.com>

Based on sources from the Linux kernel (pcnet_cs.c, 8390.h) and
eCOS(if_dp83902a.c, if_dp83902a.h). Both of these 2 wonderful world
are GPL, so this is, of course, GPL.

*/

/* Generic NS8390 register definitions. */
/* This file is part of Donald Becker's 8390 drivers, and is distributed
   under the same license. Auto-loading of 8390.o only in v2.2 - Paul G.
   Some of these names and comments originated from the Crynwr
   packet drivers, which are distributed under the GPL. */

#ifndef _8390_h
#define _8390_h

/* Some generic ethernet register configurations. */
#define E8390_TX_IRQ_MASK	0xa	/* For register EN0_ISR */
#define E8390_RX_IRQ_MASK	0x5
#define E8390_RXCONFIG		0x4	/* EN0_RXCR: broadcasts, no multicast,errors */
#define E8390_RXOFF		0x20	/* EN0_RXCR: Accept no packets */
#define E8390_TXCONFIG		0x00	/* EN0_TXCR: Normal transmit mode */
#define E8390_TXOFF		0x02	/* EN0_TXCR: Transmitter off */

/*  Register accessed at EN_CMD, the 8390 base addr.  */
#define E8390_STOP	0x01	/* Stop and reset the chip */
#define E8390_START	0x02	/* Start the chip, clear reset */
#define E8390_TRANS	0x04	/* Transmit a frame */
#define E8390_RREAD	0x08	/* Remote read */
#define E8390_RWRITE	0x10	/* Remote write  */
#define E8390_NODMA	0x20	/* Remote DMA */
#define E8390_PAGE0	0x00	/* Select page chip registers */
#define E8390_PAGE1	0x40	/* using the two high-order bits */
#define E8390_PAGE2	0x80	/* Page 3 is invalid. */

/*
 *	Only generate indirect loads given a machine that needs them.
 *      - removed AMIGA_PCMCIA from this list, handled as ISA io now
 */

#define n2k_inb(port)   (*((volatile unsigned char *)(port+CONFIG_DRIVER_NE2000_BASE)))
#define n2k_outb(val,port)  (*((volatile unsigned char *)(port+CONFIG_DRIVER_NE2000_BASE)) = val)

#define EI_SHIFT(x)	(x)

#define E8390_CMD	EI_SHIFT(0x00)  /* The command register (for all pages) */
/* Page 0 register offsets. */
#define EN0_CLDALO	EI_SHIFT(0x01)	/* Low byte of current local dma addr  RD */
#define EN0_STARTPG	EI_SHIFT(0x01)	/* Starting page of ring bfr WR */
#define EN0_CLDAHI	EI_SHIFT(0x02)	/* High byte of current local dma addr  RD */
#define EN0_STOPPG	EI_SHIFT(0x02)	/* Ending page +1 of ring bfr WR */
#define EN0_BOUNDARY	EI_SHIFT(0x03)	/* Boundary page of ring bfr RD WR */
#define EN0_TSR		EI_SHIFT(0x04)	/* Transmit status reg RD */
#define EN0_TPSR	EI_SHIFT(0x04)	/* Transmit starting page WR */
#define EN0_NCR		EI_SHIFT(0x05)	/* Number of collision reg RD */
#define EN0_TCNTLO	EI_SHIFT(0x05)	/* Low  byte of tx byte count WR */
#define EN0_FIFO	EI_SHIFT(0x06)	/* FIFO RD */
#define EN0_TCNTHI	EI_SHIFT(0x06)	/* High byte of tx byte count WR */
#define EN0_ISR		EI_SHIFT(0x07)	/* Interrupt status reg RD WR */
#define EN0_CRDALO	EI_SHIFT(0x08)	/* low byte of current remote dma address RD */
#define EN0_RSARLO	EI_SHIFT(0x08)	/* Remote start address reg 0 */
#define EN0_CRDAHI	EI_SHIFT(0x09)	/* high byte, current remote dma address RD */
#define EN0_RSARHI	EI_SHIFT(0x09)	/* Remote start address reg 1 */
#define EN0_RCNTLO	EI_SHIFT(0x0a)	/* Remote byte count reg WR */
#define EN0_RCNTHI	EI_SHIFT(0x0b)	/* Remote byte count reg WR */
#define EN0_RSR		EI_SHIFT(0x0c)	/* rx status reg RD */
#define EN0_RXCR	EI_SHIFT(0x0c)	/* RX configuration reg WR */
#define EN0_TXCR	EI_SHIFT(0x0d)	/* TX configuration reg WR */
#define EN0_COUNTER0	EI_SHIFT(0x0d)	/* Rcv alignment error counter RD */
#define EN0_DCFG	EI_SHIFT(0x0e)	/* Data configuration reg WR */
#define EN0_COUNTER1	EI_SHIFT(0x0e)	/* Rcv CRC error counter RD */
#define EN0_IMR		EI_SHIFT(0x0f)	/* Interrupt mask reg WR */
#define EN0_COUNTER2	EI_SHIFT(0x0f)	/* Rcv missed frame error counter RD */

/* Bits in EN0_ISR - Interrupt status register */
#define ENISR_RX	0x01	/* Receiver, no error */
#define ENISR_TX	0x02	/* Transmitter, no error */
#define ENISR_RX_ERR	0x04	/* Receiver, with error */
#define ENISR_TX_ERR	0x08	/* Transmitter, with error */
#define ENISR_OVER	0x10	/* Receiver overwrote the ring */
#define ENISR_COUNTERS	0x20	/* Counters need emptying */
#define ENISR_RDC	0x40	/* remote dma complete */
#define ENISR_RESET	0x80	/* Reset completed */
#define ENISR_ALL	0x3f	/* Interrupts we will enable */

/* Bits in EN0_DCFG - Data config register */
#define ENDCFG_WTS	0x01	/* word transfer mode selection */
#define ENDCFG_BOS	0x02	/* byte order selection */
#define ENDCFG_AUTO_INIT   0x10        /* Auto-init to remove packets from ring */
#define ENDCFG_FIFO		0x40	/* 8 bytes */

/* Page 1 register offsets. */
#define EN1_PHYS   EI_SHIFT(0x01)	/* This board's physical enet addr RD WR */
#define EN1_PHYS_SHIFT(i)  EI_SHIFT(i+1) /* Get and set mac address */
#define EN1_CURPAG EI_SHIFT(0x07)	/* Current memory page RD WR */
#define EN1_MULT   EI_SHIFT(0x08)	/* Multicast filter mask array (8 bytes) RD WR */
#define EN1_MULT_SHIFT(i)  EI_SHIFT(8+i) /* Get and set multicast filter */

/* Bits in received packet status byte and EN0_RSR*/
#define ENRSR_RXOK	0x01	/* Received a good packet */
#define ENRSR_CRC	0x02	/* CRC error */
#define ENRSR_FAE	0x04	/* frame alignment error */
#define ENRSR_FO	0x08	/* FIFO overrun */
#define ENRSR_MPA	0x10	/* missed pkt */
#define ENRSR_PHY	0x20	/* physical/multicast address */
#define ENRSR_DIS	0x40	/* receiver disable. set in monitor mode */
#define ENRSR_DEF	0x80	/* deferring */

/* Transmitted packet status, EN0_TSR. */
#define ENTSR_PTX 0x01	/* Packet transmitted without error */
#define ENTSR_ND  0x02	/* The transmit wasn't deferred. */
#define ENTSR_COL 0x04	/* The transmit collided at least once. */
#define ENTSR_ABT 0x08  /* The transmit collided 16 times, and was deferred. */
#define ENTSR_CRS 0x10	/* The carrier sense was lost. */
#define ENTSR_FU  0x20  /* A "FIFO underrun" occurred during transmit. */
#define ENTSR_CDH 0x40	/* The collision detect "heartbeat" signal was lost. */
#define ENTSR_OWC 0x80  /* There was an out-of-window collision. */

#define NIC_RECEIVE_MONITOR_MODE 0x20

#endif /* _8390_h */
