/*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  merlin_lport_drv.c                                       */
/*   DATE:    30/11/2015                                               */
/*   PURPOSE: Callback functions for Lport                             */
/*                                                                     */
/***********************************************************************/
#ifdef _CFE_
#include "lib_types.h"
#else
#include <linux/types.h>
#endif
#include "lport_defs.h"
#include "lport_drv.h"
#include "serdes_access.h"
#include "merlin_serdes.h"

#ifndef _CFE_
#include "phy_drv.h"
#endif
extern merlin_sdk_cb_s merlin_callbacks;

#define PHY_ADDR_TO_LANE_INDEX(addr)    addr

#ifndef _CFE_
int merlin_lport_read(phy_dev_t *phy_dev, uint32_t reg, uint16_t *val)
{
    uint16_t lane_index = PHY_ADDR_TO_LANE_INDEX(phy_dev->addr);
    
    E_MERLIN_ID merlin_id = lane_index<MERLIN_LANES_PER_CORE? MERLIN_ID_0: MERLIN_ID_1;
    //read_serdes_reg(merlin_id, reg, 0xFFFF, val);
    (void)merlin_id;
    return 0;
}

int merlin_lport_write(phy_dev_t *phy_dev, uint32_t reg, uint16_t val)
{
    uint16_t lane_index = PHY_ADDR_TO_LANE_INDEX(phy_dev->addr);
    
    E_MERLIN_ID merlin_id = lane_index<MERLIN_LANES_PER_CORE? MERLIN_ID_0: MERLIN_ID_1;
    //write_serdes_reg(merlin_id, reg, 0xFFFF, val);
    (void)merlin_id;
    return 0;
}

int merlin_lport_power_up(phy_dev_t *phy_dev)
{
    uint16_t lane_index = PHY_ADDR_TO_LANE_INDEX(phy_dev->addr);
    
    merlin_link_enable(lane_index);
    
    return 0;
}

int merlin_lport_power_down(phy_dev_t *phy_dev)
{
    uint16_t lane_index = PHY_ADDR_TO_LANE_INDEX(phy_dev->addr);
    
    merlin_link_disable(lane_index);
    
    return 0;
}

int merlin_lport_eee_enable(phy_dev_t *phy_dev)
{
    return 0;
}

int merlin_lport_eee_disable(phy_dev_t *phy_dev)
{
    return 0;
}

int merlin_lport_init(phy_dev_t *phy_dev)
{
    uint16_t lane_index = PHY_ADDR_TO_LANE_INDEX(phy_dev->addr);
    merlin_link_enable(lane_index);
    merlin_auto_enable(lane_index);
    
    return 0;
}
#endif

int merlin_lport_read_status(E_MERLIN_LANE lane_index, lport_port_status_s *port_status)
{
    uint16_t speed=0, duplex=0;
    uint8_t  link_up=0;

    merlin_link_rate_and_duplex_get(lane_index, &speed, &duplex);
    port_status->rate = speed;
    port_status->duplex = duplex;

    merlin_link_status_get(lane_index, &link_up);
    port_status->port_up = link_up;
    
    return 0;
}

void lport_serdes_drv_register(void)
{
    merlin_callbacks.merlin_init  = merlin_init;
    merlin_callbacks.merlin_reset = merlin_datapath_reset;
    merlin_callbacks.merlin_set_cfg = merlin_link_config_set;
    merlin_callbacks.merlin_get_cfg = merlin_link_config_get;
    merlin_callbacks.merlin_get_status = merlin_lport_read_status;
 }
