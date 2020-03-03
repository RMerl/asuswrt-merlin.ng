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
 * rdpa_spdsvc_ex.c
 * RDPA Speed Service RDP interface
 */
#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include <rdd.h>
#include <rdd_data_structures.h>
#include "rdpa_spdsvc_ex.h"
#include "rdpa_rdd_inline.h"

bdmf_error_t rdpa_rdd_spdsvc_gen_config(const rdpa_spdsvc_generator_t *generator_p)
{
    return rdd_spdsvc_config(generator_p->kbps, generator_p->mbs,
                            generator_p->copies, generator_p->total_length);
}

/* Get generator counters and running status. Analyzer counters remain unchanged */
bdmf_error_t rdpa_rdd_spdsvc_gen_get_result(uint8_t *running_p,
                                            uint32_t *tx_packets_p,
                                            uint32_t *tx_discards_p)
{
    return rdd_spdsvc_get_tx_result(running_p,
                                    tx_packets_p,
                                    tx_discards_p);
}

bdmf_error_t rdpa_rdd_spdsvc_gen_terminate(void)
{
#if defined(OREN)
    return BDMF_ERR_STATE;
#else
    return rdd_spdsvc_terminate();
#endif
}

bdmf_error_t rdpa_rdd_spdsvc_analyzer_config(void)
{
#if defined(BCM_DSL_RDP)

    rdd_spdsvc_reset_rx_time();

#else

#if defined(OREN)
    RDD_SPEED_SERVICE_RX_TIMESTAMPS_DTS *spdsvc_rx_timestamps_ptr;

    spdsvc_rx_timestamps_ptr = (RDD_SPEED_SERVICE_RX_TIMESTAMPS_DTS *)
                (DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) + SPEED_SERVICE_RX_TIMESTAMPS_TABLE_ADDRESS);

    RDD_SPEED_SERVICE_RX_TIMESTAMPS_START_TS_WRITE(0, spdsvc_rx_timestamps_ptr);
    RDD_SPEED_SERVICE_RX_TIMESTAMPS_LAST_TS_WRITE(0, spdsvc_rx_timestamps_ptr);
#endif

#endif
    return BDMF_ERR_OK;
}

bdmf_error_t rdpa_rdd_spdsvc_analyzer_get_rx_time(uint32_t *rx_time_us)
{
#if defined(BCM_DSL_RDP)

    rdd_spdsvc_get_rx_time(rx_time_us);

#else

#if defined(OREN)
    uint32_t timestamp_start, timestamp_last;
    RDD_SPEED_SERVICE_RX_TIMESTAMPS_DTS *spdsvc_rx_timestamps_ptr;

    spdsvc_rx_timestamps_ptr = (RDD_SPEED_SERVICE_RX_TIMESTAMPS_DTS *)
        (DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) + SPEED_SERVICE_RX_TIMESTAMPS_TABLE_ADDRESS);

    RDD_SPEED_SERVICE_RX_TIMESTAMPS_START_TS_READ(timestamp_start, spdsvc_rx_timestamps_ptr);
    RDD_SPEED_SERVICE_RX_TIMESTAMPS_LAST_TS_READ(timestamp_last, spdsvc_rx_timestamps_ptr);
    *rx_time_us = timestamp_last - timestamp_start;
#else
    *rx_time_us = 0;
#endif

#endif
    return BDMF_ERR_OK;
}


