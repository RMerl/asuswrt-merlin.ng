/*
* <:copyright-BRCM:2017:DUAL/GPL:standard
* 
*    Copyright (c) 2017 Broadcom 
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

#ifndef _RDPA_L2_CLASS_H_
#define _RDPA_L2_CLASS_H_

#include <bdmf_data_types.h>
#include "rdpa_ip_class.h"

/** \defgroup l2_class L2 Flow Classification
 * L2 flows are used for fast L2 traffic bridging in Gateway device.\n
 * The classifier identifies L2 flows using L2 key\n
 * { dst_mac, src_mac, vtag[2], eth_type, vtag_num, ingress interface, SSID or WAN flow }.\n
 * @{
 */

#include "rdpa_l2_common.h"

/** L2 flow key.\n
 * This key is used to classify traffic.\n
 */
typedef struct {
    uint32_t  hw_flow_id;            /**< L2 flow HW flow ID */
    rdpa_l2_flow_key_t key; /**< shares same struct as L2 ucast key, with some fields ingored and some fields added */
    rdpa_ip_flow_result_t result; /**< L2 flow result, shares same struct as IP Flow result, but some of the fields are 
                                    ingored */
} rdpa_l2_flow_info_t;

/** @} end of l2_class Doxygen group. */
#endif

