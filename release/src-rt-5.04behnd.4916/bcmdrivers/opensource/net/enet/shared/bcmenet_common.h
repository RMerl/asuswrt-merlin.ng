/*
 Copyright 2002-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
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
#ifndef _BCMENET_COMMON_H_
#define _BCMENET_COMMON_H_

#include "bcm_mm.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <bcmnet.h>


#define BRCM_TYPE2               0x888A // Big Endian
#define BRCM_TYPE2_LE            0x8A88 // Little Endian
#define BRCM_TAG_TYPE2_LEN       4
#define BRCM_TAG2_EGRESS         0x2000
#define BRCM_TAG2_EGRESS_TC_MASK 0x1c00

typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint16 brcm_tag;
    uint16 encap_proto;
} __attribute__((packed)) BcmEnet_hdr2;


/* Below macros should control the logical switch port number mapping.
 */
#include <port_platform.h>

#define LOGICAL_PORT_TO_UNIT_NUMBER(port)   ( (port) / MAX_SWITCH_PORTS  )
#define LOGICAL_PORT_TO_PHYSICAL_PORT(port) ( (port) % MAX_SWITCH_PORTS )
#define PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit) ( (port) + (unit) * MAX_SWITCH_PORTS )

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
// based on enet\shared\bcmenet_common.h
#define BCM_ENET_IFG        20 /* bytes */
#define BCM_ENET_CRC_LEN    4  /* bytes */
#define BCM_ENET_OVERHEAD   (BCM_ENET_CRC_LEN + BCM_ENET_IFG) /* bytes */
#endif

#endif /* _BCMENET_COMMON_H_ */
