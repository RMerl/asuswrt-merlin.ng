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

/* NE2000 base header file */
#include "ne2000_base.h"

/* find prom (taken from pc_net_cs.c from Linux) */

#include "8390.h"
/*
typedef struct hw_info_t {
	u_int	offset;
	u_char	a0, a1, a2;
	u_int	flags;
} hw_info_t;
*/
#define DELAY_OUTPUT	0x01
#define HAS_MISC_REG	0x02
#define USE_BIG_BUF	0x04
#define HAS_IBM_MISC	0x08
#define IS_DL10019	0x10
#define IS_DL10022	0x20
#define HAS_MII		0x40
#define USE_SHMEM	0x80	/* autodetected */

#define AM79C9XX_HOME_PHY	0x00006B90	/* HomePNA PHY */
#define AM79C9XX_ETH_PHY	0x00006B70	/* 10baseT PHY */
#define MII_PHYID_REV_MASK	0xfffffff0
#define MII_PHYID_REG1		0x02
#define MII_PHYID_REG2		0x03

static hw_info_t hw_info[] = {
	{ /* Accton EN2212 */ 0x0ff0, 0x00, 0x00, 0xe8, DELAY_OUTPUT },
	{ /* Allied Telesis LA-PCM */ 0x0ff0, 0x00, 0x00, 0xf4, 0 },
	{ /* APEX MultiCard */ 0x03f4, 0x00, 0x20, 0xe5, 0 },
	{ /* ASANTE FriendlyNet */ 0x4910, 0x00, 0x00, 0x94,
			DELAY_OUTPUT | HAS_IBM_MISC },
	{ /* Danpex EN-6200P2 */ 0x0110, 0x00, 0x40, 0xc7, 0 },
	{ /* DataTrek NetCard */ 0x0ff0, 0x00, 0x20, 0xe8, 0 },
	{ /* Dayna CommuniCard E */ 0x0110, 0x00, 0x80, 0x19, 0 },
	{ /* D-Link DE-650 */ 0x0040, 0x00, 0x80, 0xc8, 0 },
	{ /* EP-210 Ethernet */ 0x0110, 0x00, 0x40, 0x33, 0 },
	{ /* EP4000 Ethernet */ 0x01c0, 0x00, 0x00, 0xb4, 0 },
	{ /* Epson EEN10B */ 0x0ff0, 0x00, 0x00, 0x48,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* ELECOM Laneed LD-CDWA */ 0xb8, 0x08, 0x00, 0x42, 0 },
	{ /* Hypertec Ethernet */ 0x01c0, 0x00, 0x40, 0x4c, 0 },
	{ /* IBM CCAE */ 0x0ff0, 0x08, 0x00, 0x5a,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM CCAE */ 0x0ff0, 0x00, 0x04, 0xac,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM CCAE */ 0x0ff0, 0x00, 0x06, 0x29,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM FME */ 0x0374, 0x08, 0x00, 0x5a,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* IBM FME */ 0x0374, 0x00, 0x04, 0xac,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Kansai KLA-PCM/T */ 0x0ff0, 0x00, 0x60, 0x87,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0374, 0x08, 0x00, 0x17,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0374, 0x00, 0xc0, 0xa8,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0374, 0x00, 0xa0, 0xb0,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* NSC DP83903 */ 0x0198, 0x00, 0x20, 0xe0,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* I-O DATA PCLA/T */ 0x0ff0, 0x00, 0xa0, 0xb0, 0 },
	{ /* Katron PE-520 */ 0x0110, 0x00, 0x40, 0xf6, 0 },
	{ /* Kingston KNE-PCM/x */ 0x0ff0, 0x00, 0xc0, 0xf0,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Kingston KNE-PCM/x */ 0x0ff0, 0xe2, 0x0c, 0x0f,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Kingston KNE-PC2 */ 0x0180, 0x00, 0xc0, 0xf0, 0 },
	{ /* Maxtech PCN2000 */ 0x5000, 0x00, 0x00, 0xe8, 0 },
	{ /* NDC Instant-Link */ 0x003a, 0x00, 0x80, 0xc6, 0 },
	{ /* NE2000 Compatible */ 0x0ff0, 0x00, 0xa0, 0x0c, 0 },
	{ /* Network General Sniffer */ 0x0ff0, 0x00, 0x00, 0x65,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* Panasonic VEL211 */ 0x0ff0, 0x00, 0x80, 0x45,
			HAS_MISC_REG | HAS_IBM_MISC },
	{ /* PreMax PE-200 */ 0x07f0, 0x00, 0x20, 0xe0, 0 },
	{ /* RPTI EP400 */ 0x0110, 0x00, 0x40, 0x95, 0 },
	{ /* SCM Ethernet */ 0x0ff0, 0x00, 0x20, 0xcb, 0 },
	{ /* Socket EA */ 0x4000, 0x00, 0xc0, 0x1b,
			DELAY_OUTPUT | HAS_MISC_REG | USE_BIG_BUF },
	{ /* Socket LP-E CF+ */ 0x01c0, 0x00, 0xc0, 0x1b, 0 },
	{ /* SuperSocket RE450T */ 0x0110, 0x00, 0xe0, 0x98, 0 },
	{ /* Volktek NPL-402CT */ 0x0060, 0x00, 0x40, 0x05, 0 },
	{ /* NEC PC-9801N-J12 */ 0x0ff0, 0x00, 0x00, 0x4c, 0 },
	{ /* PCMCIA Technology OEM */ 0x01c8, 0x00, 0xa0, 0x0c, 0 },
	{ /* Qemu */ 0x0, 0x52, 0x54, 0x00, 0 },
	{ /* RTL8019AS */ 0x0, 0x0, 0x18, 0x5f, 0 }
};

#define NR_INFO		(sizeof(hw_info)/sizeof(hw_info_t))

#define PCNET_CMD	0x00
#define PCNET_DATAPORT	0x10	/* NatSemi-defined port window offset. */
#define PCNET_RESET	0x1f	/* Issue a read to reset, a write to clear. */
#define PCNET_MISC	0x18	/* For IBM CCAE and Socket EA cards */

static void pcnet_reset_8390(u8* addr)
{
	int i, r;

	n2k_outb(E8390_NODMA + E8390_PAGE0+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", addr + E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE1+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", addr + E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", addr + E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);

	n2k_outb(n2k_inb(PCNET_RESET), PCNET_RESET);

	for (i = 0; i < 100; i++) {
		if ((r = (n2k_inb(EN0_ISR) & ENISR_RESET)) != 0)
			break;
		PRINTK("got %x in reset\n", r);
		udelay(100);
	}
	n2k_outb(ENISR_RESET, EN0_ISR); /* Ack intr. */

	if (i == 100)
		printf("pcnet_reset_8390() did not complete.\n");
} /* pcnet_reset_8390 */

int get_prom(u8* mac_addr, u8* base_addr)
{
	u8 prom[32];
	int i, j;
	struct {
		u_char value, offset;
	} program_seq[] = {
		{E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD}, /* Select page 0*/
		{0x48, EN0_DCFG},		/* Set byte-wide (0x48) access. */
		{0x00, EN0_RCNTLO},		/* Clear the count regs. */
		{0x00, EN0_RCNTHI},
		{0x00, EN0_IMR},		/* Mask completion irq. */
		{0xFF, EN0_ISR},
		{E8390_RXOFF, EN0_RXCR},	/* 0x20 Set to monitor */
		{E8390_TXOFF, EN0_TXCR},	/* 0x02 and loopback mode. */
		{32, EN0_RCNTLO},
		{0x00, EN0_RCNTHI},
		{0x00, EN0_RSARLO},		/* DMA starting at 0x0000. */
		{0x00, EN0_RSARHI},
		{E8390_RREAD+E8390_START, E8390_CMD},
	};

	PRINTK ("trying to get MAC via prom reading\n");

	pcnet_reset_8390 (base_addr);

	mdelay (10);

	for (i = 0; i < ARRAY_SIZE(program_seq); i++)
		n2k_outb (program_seq[i].value, program_seq[i].offset);

	PRINTK ("PROM:");
	for (i = 0; i < 32; i++) {
		prom[i] = n2k_inb (PCNET_DATAPORT);
		PRINTK (" %02x", prom[i]);
	}
	PRINTK ("\n");
	for (i = 0; i < NR_INFO; i++) {
		if ((prom[0] == hw_info[i].a0) &&
			(prom[2] == hw_info[i].a1) &&
			(prom[4] == hw_info[i].a2)) {
			PRINTK ("matched board %d\n", i);
			break;
		}
	}
	if ((i < NR_INFO) || ((prom[28] == 0x57) && (prom[30] == 0x57))) {
		PRINTK ("on exit i is %d/%ld\n", i, NR_INFO);
		PRINTK ("MAC address is ");
		for (j = 0; j < 6; j++) {
			mac_addr[j] = prom[j << 1];
			PRINTK ("%02x:", mac_addr[i]);
		}
		PRINTK ("\n");
		return (i < NR_INFO) ? i : 0;
	}
	return 0;
}
