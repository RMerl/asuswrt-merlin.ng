/*
<:copyright-BRCM:2002:DUAL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
