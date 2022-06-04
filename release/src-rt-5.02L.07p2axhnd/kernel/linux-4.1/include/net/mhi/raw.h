#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/*
 * File: mhi/raw.h
 *
 * MHI RAW socket definitions
 */

#ifndef MHI_RAW_H
#define MHI_RAW_H

#include <linux/types.h>
#include <linux/socket.h>

#include <net/sock.h>


extern int mhi_raw_sock_create(
	struct net *net,
	struct socket *sock,
	int proto,
	int kern);

extern int  mhi_raw_proto_init(void);
extern void mhi_raw_proto_exit(void);

#endif /* MHI_RAW_H */
#endif /* CONFIG_BCM_KF_MHI */
