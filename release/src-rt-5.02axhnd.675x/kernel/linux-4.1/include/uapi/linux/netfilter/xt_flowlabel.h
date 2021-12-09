#if defined(CONFIG_BCM_KF_NETFILTER)
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


/* IP tables module for matching the value of the IPv6 flowlabel field
 *
 * BRCM, Feb, 1. 2019.
 */


#ifndef _XT_FLOWLABEL_H
#define _XT_FLOWLABEL_H

#include <linux/types.h>

#define XT_FLOWLABEL_MAX	cpu_to_be32(0x000FFFFF)


/* match info */
struct xt_flowlabel_info {
	__be32 flowlabel;
	__u8 invert;
};


#endif /* _XT_FLOWLABEL_H */

#endif // defined(CONFIG_BCM_KF_NETFILTER)

