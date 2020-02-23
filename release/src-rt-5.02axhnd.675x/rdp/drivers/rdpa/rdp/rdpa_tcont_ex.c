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

/*
 * rdpa_tcont_ex.c
 *
 *  Created on: June 22, 2014
 *      Author: Yoni Itah
 */
 
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdp_drv_bbh.h"
#include "rdd_ih_defs.h"
#include "rdd.h"

uint8_t tcont_tc_table_get(bdmf_object_handle tcont)
{
    bdmf_link_handle link = NULL;
    bdmf_number tc_table;

    while ((link = bdmf_get_next_us_link(tcont, link)))
    {
        if (bdmf_us_link_to_object(link)->drv == rdpa_tc_to_queue_drv())
        {
            rdpa_tc_to_queue_table_get(bdmf_us_link_to_object(link), &tc_table);
            return (uint8_t)tc_table;
        }
    }
    return 0;
}

uint8_t tcont_pbit_table_get(bdmf_object_handle tcont)
{
    bdmf_link_handle link = NULL;
    bdmf_number pbit_table = 0;

    while ((link = bdmf_get_next_us_link(tcont, link)))
    {
        if (bdmf_us_link_to_object(link)->drv == rdpa_pbit_to_queue_drv())
        {
            rdpa_pbit_to_queue_table_get(bdmf_us_link_to_object(link), &pbit_table);
            return (uint8_t)pbit_table;
        }
    }
    return 0;
}

int rdpa_tcont_sr_dba_callback(uint32_t tcont_id, uint32_t *runner_ddr_occupancy)
{
    return rdd_wan_channel_byte_counter_read(tcont_id, runner_ddr_occupancy);
}

int rdpa_tcont_tcont_id_to_channel_id(int tcont_id)
{
    if (tcont_id)
        return tcont_id - 1;
    else
        return RDPA_MAX_TCONT - 1;
}

