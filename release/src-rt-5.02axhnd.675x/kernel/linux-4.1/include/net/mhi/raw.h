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
