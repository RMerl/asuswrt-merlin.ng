/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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
* :>
*/


#include "rdd_ghost_reporting.h"
#include "rdd_runner_proj_defs.h"
#include "rdp_drv_rnr.h"
#include "XRDP_AG.h"

bdmf_error_t rdd_ghost_reporting_timer_set(void)
{
    RDD_GHOST_REPORTING_GLOBAL_CFG_DTS *entry;
    bdmf_error_t rc = BDMF_ERR_OK;

    RDD_BTRACE("\n");

    entry = RDD_GHOST_REPORTING_GLOBAL_CFG_PTR(get_runner_idx(reporting_runner_image));

    RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY_TIMER_INTERVAL_WRITE(GHOST_REPORTING_TIMER_INTERVAL, entry);

    /* enable ghost reporting task */
    rdd_ghost_reporting_set_disable(0);

    /* Make sure the timer configured before the cpu weakeup */
    WMB();

    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(reporting_runner_image), REPORTING_THREAD_NUMBER);

    return rc;
}

bdmf_error_t rdd_ghost_reporting_mapping_queue_to_wan_channel(uint8_t queue_id, uint8_t wan_channel, bdmf_boolean enable)
{
    RDD_REPORTING_QUEUE_DESCRIPTOR_DTS *entry;
    uint16_t counter_ptr;
    RDD_BYTES_2_DTS *bit_vector_ptr;
    uint16_t queue_vector;

    RDD_BTRACE("queue_id = %d, wan_channel = %d\n", queue_id, wan_channel);

    if ((queue_id >= RDD_REPORTING_QUEUE_DESCRIPTOR_TABLE_SIZE) ||
        (wan_channel >= RDD_REPORTING_COUNTER_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    /* firmware will read 2 bytes at once, and check bitmask of these 16 bits. Because of endianess issue,
     * host side has to perform bitmask operation on 2 bytes value as well */
    bit_vector_ptr = (RDD_BYTES_2_DTS *)RDD_QUEUE_TO_REPORT_BIT_VECTOR_PTR(get_runner_idx(reporting_runner_image));
    bit_vector_ptr += queue_id >> 4;
    RDD_BYTES_2_BITS_READ(queue_vector, bit_vector_ptr);

#if defined(BCM_PON_XRDP)
    if ((wan_channel == BBH_QUEUE_WAN_1_ENTRY_ID) || !enable )
#else
    /* if WAN Channel is not PON channels, just return. also clears the bit vector
     * If it is, also set it to bit vector. */
    if ((wan_channel < RDD_US_CHANNEL_OFFSET_TCONT) || (wan_channel > RDD_US_CHANNEL_OFFSET_TCONT_END ) || !enable)
#endif
    {
        queue_vector &= ~(1 << (queue_id & 0xf));
        RDD_BYTES_2_BITS_WRITE(queue_vector, bit_vector_ptr);

        return BDMF_ERR_OK;
    }

    queue_vector |= 1 << (queue_id & 0xf);
    RDD_BYTES_2_BITS_WRITE(queue_vector, bit_vector_ptr);

    entry = ((RDD_REPORTING_QUEUE_DESCRIPTOR_DTS *)RDD_REPORTING_QUEUE_DESCRIPTOR_TABLE_PTR(get_runner_idx(reporting_runner_image))) + queue_id;
    counter_ptr = REPORTING_COUNTER_ADDRESS + (wan_channel * sizeof(RDD_BYTES_4_DTS));
    RDD_REPORTING_QUEUE_DESCRIPTOR_COUNTER_PTR_WRITE(counter_ptr, entry);

    return BDMF_ERR_OK;
}

void rdd_ghost_reporting_mac_type_init(uint8_t mac_type)
{
    RDD_GHOST_REPORTING_GLOBAL_CFG_DTS *entry;

    RDD_BTRACE("mac_type = %d\n", mac_type);

    entry = RDD_GHOST_REPORTING_GLOBAL_CFG_PTR(get_runner_idx(reporting_runner_image));
    RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY_MAC_TYPE_WRITE(mac_type, entry);
}

void rdd_ghost_reporting_set_disable(bdmf_boolean disable)
{
    RDD_GHOST_REPORTING_GLOBAL_CFG_DTS *entry;

    RDD_BTRACE("disable = %d\n", disable);

    entry = RDD_GHOST_REPORTING_GLOBAL_CFG_PTR(get_runner_idx(reporting_runner_image));
    RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY_DISABLE_WRITE(disable, entry);
}

/* RDD debug function */
bdmf_error_t rdd_ghost_reporting_ctr_get(uint8_t wan_channel, uint32_t *report)
{
    RDD_BYTES_4_DTS *entry;

    RDD_BTRACE("wan_channel = %d, report = %p\n", wan_channel, report);

    if ((wan_channel >= RDD_REPORTING_COUNTER_TABLE_SIZE) ||
        (report == NULL))
    {
        return BDMF_ERR_PARM;
    }

    entry = (RDD_BYTES_4_DTS *)RDD_REPORTING_COUNTER_TABLE_PTR(get_runner_idx(reporting_runner_image)) + wan_channel;
    RDD_BYTES_4_BITS_READ(*report, entry);

    return BDMF_ERR_OK;
}

