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
 * file mhi.h
 *
 * Modem-Host Interface (MHI) kernel interface
 */

#ifndef LINUX_MHI_H
#define LINUX_MHI_H

#include <linux/types.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <asm/byteorder.h>


struct mhi_sock {
	struct sock	sk;
	int		sk_l3proto;
	int		sk_ifindex;
};

struct sockaddr_mhi {
	sa_family_t	sa_family;
	int		sa_ifindex;
	__u8	sa_zero[
		sizeof(struct sockaddr)
		- sizeof(sa_family_t)
		- sizeof(int)];
};


static inline struct mhi_sock *mhi_sk(struct sock *sk)
{
	return (struct mhi_sock *)sk;
}

static inline struct sockaddr_mhi *sa_mhi(struct sockaddr *sa)
{
	return (struct sockaddr_mhi *)sa;
}

#endif
#endif /* CONFIG_BCM_KF_MHI */
