/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_common.h"
#include "rdpa_rdd_map.h"
#include "rdd_runner_proj_defs.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_port_int.h"

const int rdpa_fwd_act2rdd_fc_fwd_act[] =
{
    [rdpa_forward_action_host] = RDD_FC_FWD_ACTION_CPU,
    [rdpa_forward_action_drop] = RDD_FC_FWD_ACTION_DROP
};

rdpa_ports rdpa_rdd_egress_port_vector_to_ports(rdd_vport_vector_t egress_port_vector, bdmf_boolean is_iptv)
{
    rdpa_ports ports = 0;
    rdd_vport_id_t i, ep;
    rdpa_if port;

    for (i = RDD_VPORT_FIRST; i <= PROJ_DEFS_RDD_VPORT_LAST; i++)
    {
        ep = 1LL << i;
        if (!(egress_port_vector & ep))
            continue;
        port = rdpa_port_vport_to_rdpa_if(i);
        if (port != rdpa_if_none) /* Port configured */ 
            ports |= rdpa_if_id(port);
    }
    return ports;
}

rdd_vport_vector_t rdpa_ports_to_rdd_egress_port_vector(rdpa_ports ports, bdmf_boolean is_iptv)
{
    rdd_vport_vector_t vport_vector = 0;
    rdd_vport_id_t vport;

    while (ports)
    {
        vport = rdpa_port_rdpa_if_to_vport(rdpa_port_get_next(&ports));
        if (vport != PROJ_DEFS_RDD_VPORT_LAST + 1)
            vport_vector |= 1LL << vport;
    }
    return vport_vector;
}
