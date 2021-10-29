/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#ifndef _RDPA_SPDTEST_COMMON_H_
#define _RDPA_SPDTEST_COMMON_H_

#include "rdpa_types.h"

/** Minimum transmitted packet size for padding */
#define RDPA_SPDTEST_MIN_TX_PD_LEN 60


/** Speed Test Engine Reference Packet, includes both packet header and payload.\n
 * required by the Runner Speed Test Engine for TX tests.\n
 */
typedef struct
{
    uint16_t size;           /**< Reference packet size */
    void *data;              /**< Reference packet pointer */
    union {
        struct {
            uint16_t payload_offset; /**< Reference packet payload offset */
        } udp;
        uint16_t h;
    };
} rdpa_spdtest_ref_pkt_t;

#endif /* _RDPA_SPDTEST_H_ */
