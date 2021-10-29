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
