#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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
