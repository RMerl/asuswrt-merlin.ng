/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
*
*    Copyright (c) 2014 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
