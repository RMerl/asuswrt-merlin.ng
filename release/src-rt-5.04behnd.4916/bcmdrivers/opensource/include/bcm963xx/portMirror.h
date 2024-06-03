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
 * portMirror.h -- Internal definitions for Port Mirroring Feature.
 *
 * Description:
 *   Port Mirroring feature definitions. Initially for the BLAA driver.
 *
 *
 * Authors: Srinivasan Seshadri.
 *
 * $Revision: 1.1 $
 *
 * Initial CVS checkin.
 *
 ****************************************************************************/

#ifndef __PORT_MIRROR_H
#define __PORT_MIRROR_H

#define DIR_INVALID                  0
#define DIR_IN                       1
#define DIR_OUT                      2
#define MAX_DIRECTIONS               DIR_OUT

#define FLOW_INVALID                 0
#define RFC2684_802_3_BRIDGING_FLOW  1
#define MAX_FLOWS                    RFC2684_802_3_BRIDGING_FLOW

#define MIRROR_PORT_INVALID          0
#define MIRROR_PORT_ETH0             1
#define MIRROR_PORT_ETH1             2
#define MIRROR_PORT_USB              3
#define MAX_MIRROR_PORTS             MIRROR_PORT_USB

#define MIRROR_DISABLED              1
#define MIRROR_ENABLED               2

typedef struct _MirrorCfg {
    int   dir ;
    int   flowType ;
    int   mirrorPort ;
    int   status ;
} MirrorCfg ;

#define CMD_PORT_MIRRORING           1

#define MIRROR_NAME_MAX_LEN          16

#endif /* #ifndef __PORT_MIRROR_H */
