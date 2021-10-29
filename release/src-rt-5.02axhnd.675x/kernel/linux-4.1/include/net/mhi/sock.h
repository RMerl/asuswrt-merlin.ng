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
 * File: mhi/sock.h
 *
 * MHI socket definitions
 */

#ifndef MHI_SOCK_H
#define MHI_SOCK_H

#include <linux/types.h>
#include <linux/socket.h>

#include <net/sock.h>


extern const struct proto_ops mhi_socket_ops;

extern int  mhi_sock_rcv_unicast(struct sk_buff *skb, u8 l3prot, u32 l3len);
extern int  mhi_sock_rcv_multicast(struct sk_buff *skb, u8 l3prot, u32 l3len);

extern void mhi_sock_hash(struct sock *sk);
extern void mhi_sock_unhash(struct sock *sk);

extern int  mhi_sock_init(void);
extern void mhi_sock_exit(void);

#endif /* MHI_SOCK_H */
#endif /* CONFIG_BCM_KF_MHI */
