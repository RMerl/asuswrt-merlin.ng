/*
 * Copyright 1997 Epigram, Inc.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: epipci.h 708017 2017-06-29 14:11:45Z $
 *
*/

#ifndef _pci_h_
#define _pci_h_

/* PCI definitions for VHDL simulation. */

/* These are the command definitions for the C/BE bus.
 * Note that types 4, 5, 8, and 9 are reserved. We definitely
 * don't support DAC, and likely won't cope with intack, special,
 * and I/O.
 */

typedef enum {
	pci_cmd_intack	= 0,
	pci_cmd_special	= 1,
	pci_cmd_io_rd	= 2,
	pci_cmd_io_wr	= 3,
	pci_cmd_rsvd4	= 4,
	pci_cmd_rsvd5	= 5,
	pci_cmd_read	= 6,
	pci_cmd_write	= 7,
	pci_cmd_rsvd8	= 8,
	pci_cmd_rsvd9	= 9,
	pci_cmd_conf_rd	= 10,
	pci_cmd_conf_wr	= 11,
	pci_cmd_read_m	= 12,
	pci_cmd_dac	= 13			/* not supported!!! */,
	pci_cmd_read_ln	= 14,
	pci_cmd_wr_inv	= 15,

	/* out-of-band, simulation support stuff */

	pci_cmd_idle	= 16,			/* no command ready */
	pci_dpty_err	= 17,			/* force PCI data parity error cycle */
	pci_apty_err	= 18,			/* force PCI address parity error cycle */
	pci_pty_ok	= 19,			/* restore correct PCI parity */
	pci_interrupt	= 20,			/* inta assertion from VHDL */
	pci_delays	= 21,			/* slave configure_delays */
	pci_term_style	= 22,			/* slave termination style */
	pci_mem_low	= 23,			/* memory range */
	pci_mem_high	= 24,			/* memory range */
	pci_decode	= 25,			/* slave address decode delay */
	pci_arb_type	= 26,			/* 0 for fixed, 1 for rotate, 2 for park */
						/* (park dev is in high halfword) */
	pci_arb_gnt	= 27,			/* length of each grant before it's revoked */
	pci_xfer_limit	= 28,			/* maximum xfer before disconnect */
	pci_abort_limit	= 29,			/* maximum xfer before tabt */
	pci_pkt_write	= 30,			/* packet write to PCI slave memory */
	pci_pkt_read	= 31,			/* receive a packet into PCI memory */
	/* If we get past 31, we need to fix some code for the masks below */

	pci_cmd_kill	= 99			/* kill the simulator, from socket */
} pcicmd_t;

#define pci_mode pci_arb_type			/* overload this one */

/* These masks are used to figure out where data is */

#define PCI_WR_MASK ((1 << pci_cmd_write) | (1 << pci_cmd_io_wr) | \
		     (1 << pci_cmd_rsvd5) | (1 << pci_cmd_rsvd9) | \
		     (1 << pci_cmd_conf_wr) | (1 << pci_cmd_wr_inv) | (1 << pci_pkt_write))

#define PCI_RD_MASK ((1 << pci_cmd_read) | (1 << pci_cmd_io_rd) | \
		     (1 << pci_cmd_conf_rd) | (1 << pci_pkt_read) | \
		     (1 << pci_cmd_rsvd4) | (1 << pci_cmd_rsvd8) | \
		     (1 << pci_cmd_read_m) | (1 << pci_cmd_read_ln))

#define PCI_CMD_IS_READ(x)	((1 << (x)) & PCI_RD_MASK)
#define PCI_CMD_IS_WRITE(x)	((1 << (x)) & PCI_WR_MASK)
#define PCI_CMD_NEEDS_SIZE(x)	((1 << (x)) & (PCI_RD_MASK | PCI_WR_MASK))

/* parameters for pci_arb_type commands */

#define PCI_ARB_FIXED		0
#define PCI_ARB_ROTATE		1
#define PCI_ARB_REVERSE		2		/* won't use this one */
#define PCI_ARB_PARK		3

#define PCI_DEV_SHIFT		16

#define PCI_MAX_BURST_SIZE	32
#define PCIE_MAX_BURST_SIZE	128		/* in bytes */

#define	MAX_BURST_SIZE \
		((PCI_MAX_BURST_SIZE > PCIE_MAX_BURST_SIZE) ? \
		 PCI_MAX_BURST_SIZE : PCIE_MAX_BURST_SIZE)

#ifndef PCI_CACHE_LINE_SIZE
#define PCI_CACHE_LINE_SIZE	PCI_MAX_BURST_SIZE
#endif // endif

typedef struct
{
	uint32	sequence;		/* sequence number */
	uint32	dest;			/* destination, see below */
	uint32	pci_addr;		/* address in PCI space */
	uint32	pci_addr_hi;		/* high 32bits of a 64bit PCIe address */
	uint32	command;		/* PCI CBE[3:0] command field */
	uint32	size;			/* transaction size (bytes) */
	uint32	first_byte_en;		/* byte enable on first data beat */
	uint32	last_byte_en;		/* byte enable on last data beat */
	uint32	result;			/* see below */
	uint32	data[MAX_BURST_SIZE / sizeof(uint32)];	/* read/write data */
					/* for interrupt, data contains the
					 * value of the PCI INTA# signal,
					 * which is active low
					 */
} pci_transaction_t;

#define PCI_DEST_MASTER		1		/* TXN causes PCI transaction or master config */
#define PCI_DEST_SLAVE		2		/* TXN causes slave configuration */
#define PCI_DEST_ARB		4		/* TXN causes arbiter configuration */
#define PCI_DEST_PCIE		8

#define PCI_RESULT_OK		0		/* successful completion */
#define PCI_RESULT_MABT		1		/* master abort */
#define PCI_RESULT_TABT		2		/* target abort */

/* socket addresses to communicate to/from leapfrog from a driver program */

#define PCI_SOCKET_ADDR_BASE	24680		/* => leapfrog, +1 for => driver */

/* socket support */
struct sockaddr_in;

int fmi_init_socket(int addr);
int fmi_get_msg(int pci_socket, pci_transaction_t *p, struct sockaddr_in *sin);
int fmi_send_msg(int pci_socket, pci_transaction_t *p, struct sockaddr_in *sin);

#endif /* _pci_sim_h_ */
