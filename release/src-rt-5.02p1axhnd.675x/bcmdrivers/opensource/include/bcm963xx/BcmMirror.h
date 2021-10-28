/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
/****************************************************************************
 *
 * BcmBlaaMirror.h -- Internal definitions for BLAA Port Mirroring Feature.
 *
 * Description:
 *   Port Mirroring feature definitions for the BLAA driver.
 *
 *
 * Authors: Srinivasan Seshadri.
 *
 * $Revision: 1.1 $
 *
 * Initial CVS checkin.
 *
 ****************************************************************************/
//yongchao mdoify

#ifndef __BCM_BLAA_MIRROR_H
#define __BCM_BLAA_MIRROR_H

#include <portMirror.h>

/* RFC2684 definitions */

#define RFC2684_802_3_LLC_HDR1           0xAAAA0300
#define RFC2684_802_3_LLC_HDR2_NO_FCS    0x80C20001
#define RFC2684_802_3_LLC_HDR2_FCS       0x80C20007
#define RFC2684_802_3_LLC_VC_PAD         0x0000
#define RFC2684_802_3_VPN_BRIDGE_HDR2    0x005E0008

#define RFC2684_802_3_LLC_ENCAP_LEN            10  /* in bytes */
#define RFC2684_802_3_VC_ENCAP_LEN              2  /* in bytes */
#define RFC2684_802_3_VPN_BRIDGE_ENCAP_LEN     16  /* in bytes */

void blaaUtilSetMirrorPort (int mirrorPort, char *pMirrorPort) ;
int blaaMirrorWriteData (struct net_device * dbgDev, struct sk_buff *pSockBuf) ;
void blaaMirrorRfc2684Frame (struct sk_buff *pSockBuf, UINT32 ulRcvLen, UINT32 dir,
                             char *dev) ;

#endif /* __BCM_BLAA_MIRROR_H */
