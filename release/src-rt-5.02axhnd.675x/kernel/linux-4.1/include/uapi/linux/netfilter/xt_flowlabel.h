#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

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

