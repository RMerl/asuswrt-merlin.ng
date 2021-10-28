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

/***********************************************************************/
/*                                                                     */
/*   file:  bcmgmac_47189.h                                            */
/*                                                                     */
/***********************************************************************/
#ifndef _BCM_GMAC_H_
#define _BCM_GMAC_H_

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>

#define GMAC_VERSION            "v0.1"
#define GMAC_VER_STR            GMAC_VERSION
#define GMAC_MODNAME            "Broadcom GMAC"
#define GMAC_DRV_MAJOR          249 /* GMAC Character Device */

#define GMAC_ERROR              (-1)
#define GMAC_SUCCESS            0

volatile EnetCoreMib* gmac_mib_regs(int ethcore);
int gmac_set_active( void );
void gmac_link_status_changed(int ethcore, int link_status, int speed, int duplex);
void gmac_hw_stats( int port,  struct rtnl_link_stats64 *stats);
void gmac_reset_mib( int ethcore );
void gmac_dump_mib( int ethcore, int type);
int gmac_init( void );
void MoCA_eth_init( void );
#endif /* _BCM_GMAC_H_ */

