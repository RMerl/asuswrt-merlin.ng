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


/*
 * rdpa_spdsvc_ex.c
 * RDPA Speed Service XRDP interface
 */
#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include <rdd.h>
#include <rdpa_spdsvc.h>
#include <rdd_spdsvc.h>
#include "rdpa_spdsvc_ex.h"

bdmf_error_t rdpa_rdd_spdsvc_gen_config(const rdpa_spdsvc_generator_t *generator_p)
{
    return rdd_spdsvc_gen_config(generator_p->kbps, generator_p->mbs,
                                 generator_p->copies, generator_p->total_length,
                                 generator_p->test_time_ms);
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
    return BDMF_ERR_STATE;
}

bdmf_error_t rdpa_rdd_spdsvc_analyzer_config(void)
{
    return rdd_spdsvc_analyzer_config();
}

bdmf_error_t rdpa_rdd_spdsvc_analyzer_delete(void)
{
    return rdd_spdsvc_analyzer_delete();
}

bdmf_error_t rdpa_rdd_spdsvc_analyzer_get_rx_time(uint32_t *rx_time_us)
{
    return rdd_spdsvc_analyzer_get_rx_time(rx_time_us);
}
