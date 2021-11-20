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

/***********************************************************************/
/*                                                                     */
/*   file:  bcmgmac.h                                                  */
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
#define GMAC_DRV_MAJOR          326 /* GMAC Character Device */

#define GMAC_ERROR              (-1)
#define GMAC_SUCCESS            0

extern int gmac_set_active( void );
extern void gmac_hw_stats( struct rtnl_link_stats64 *stats );
extern int gmac_dump_mib( int type );
extern void gmac_reset_mib( void );
extern int gmac_init( void );
#endif /* _BCM_GMAC_H_ */

