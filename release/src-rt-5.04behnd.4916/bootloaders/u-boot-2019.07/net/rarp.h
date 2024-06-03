/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#if defined(CONFIG_CMD_RARP)

#ifndef __RARP_H__
#define __RARP_H__

#include <net.h>

/**********************************************************************/
/*
 *	Global functions and variables.
 */

extern int rarp_try;

/* Process the receipt of a RARP packet */
void rarp_receive(struct ip_udp_hdr *ip, unsigned len);
void rarp_request(void);	/* Send a RARP request */

/**********************************************************************/

#endif /* __RARP_H__ */
#endif
