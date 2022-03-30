/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

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

#ifndef _BR_BCM_HOOKS_IEEE1905_H
#define _BR_BCM_HOOKS_IEEE1905_H

#if IS_ENABLED(CONFIG_BCM_IEEE1905)
struct sk_buff;

unsigned int bcm_br_ieee1905_nf(struct sk_buff *skb);
void bcm_br_ieee1905_pt_add(void);
void bcm_br_ieee1905_pt_del(void);
#else
#define bcm_br_ieee1905_nf(arg) NF_ACCEPT
#define bcm_br_ieee1905_pt_add() do {} while (0)
#define bcm_br_ieee1905_pt_del() do {} while (0)
#endif // #if IS_ENABLED(CONFIG_BCM_IEEE1905)
#endif // _BR_BCM_HOOKS_IEEE1905_H
