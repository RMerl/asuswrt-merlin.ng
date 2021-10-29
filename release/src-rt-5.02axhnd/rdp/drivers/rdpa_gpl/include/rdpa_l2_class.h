/*
* <:copyright-BRCM:2017:DUAL/GPL:standard
* 
*    Copyright (c) 2017 Broadcom 
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
    rdpa_l2_flow_key_t key; /**< shares same struct as L2 ucast key, with some fields ingored and some fields added */
    rdpa_ip_flow_result_t result; /**< L2 flow result, shares same struct as IP Flow result, but some of the fields are 
                                    ingored */
} rdpa_l2_flow_info_t;

/** @} end of l2_class Doxygen group. */
#endif

