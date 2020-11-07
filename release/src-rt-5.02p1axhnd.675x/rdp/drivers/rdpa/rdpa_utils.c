/*
 * <:copyright-BRCM:2013:proprietary:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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

#include <rdpa_api.h>
#include <rdd.h>
#include <rdd_utils.h>

void rdpa_fwtrace_clear(void)
{
    f_rdd_fwtrace_clear();
    return;
}
EXPORT_SYMBOL(rdpa_fwtrace_clear);

int rdpa_fwtrace_enable_set(uint32_t enable)
{
    if (f_rdd_fwtrace_enable_set(enable) == BL_LILAC_RDD_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(rdpa_fwtrace_enable_set);

int rdpa_fwtrace_get(LILAC_RDD_RUNNER_INDEX_DTS runner_id,
                            uint32_t *trace_length,
                            uint32_t *trace_buffer)
{
    if (f_rdd_fwtrace_get(runner_id, trace_length, trace_buffer) == BL_LILAC_RDD_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(rdpa_fwtrace_get);

int rdpa_fwtrace_rnr_a_thread_name_get(int thread_id, char *p_name)
{
    int ret_val = 0;
    
    if (thread_id < MAX_RNR_THREADS)
    {
        memcpy(p_name, rnr_a_task_names[thread_id], MAX_THREAD_NAME_SIZE);
    }
    else
    {
        ret_val = -1;
    }

    return ret_val;
}
EXPORT_SYMBOL(rdpa_fwtrace_rnr_a_thread_name_get);

int rdpa_fwtrace_rnr_b_thread_name_get(int thread_id, char *p_name)
{
    int ret_val = 0;
    if (thread_id < MAX_RNR_THREADS)
    {
        memcpy(p_name, rnr_b_task_names[thread_id], MAX_THREAD_NAME_SIZE);
    }
    else
    {
        ret_val = -1;
    }

    return ret_val;
}
EXPORT_SYMBOL(rdpa_fwtrace_rnr_b_thread_name_get);

int rdpa_fwtrace_event_name_get(int event_id, char *p_name)
{
    int ret_val = 0;
    if (event_id < MAX_RNR_EVENTS)
    {
        memcpy(p_name, rdp_fw_trace_events[event_id], MAX_EVENT_NAME_SIZE);
    }
    else
    {
        ret_val = -1;
    }

    return ret_val;
}
EXPORT_SYMBOL(rdpa_fwtrace_event_name_get);







