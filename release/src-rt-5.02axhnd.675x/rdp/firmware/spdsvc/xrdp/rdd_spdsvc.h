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
 * rdd_spdsvc.h
 */

#ifndef _RDD_SPDSVC_H_
#define _RDD_SPDSVC_H_

#include "rdd_cpu_tx.h"

#if !defined(G9991)
#include "rdpa_dhd_helper_basic.h"
#endif

#if defined(BCM63158)
// on BCM63158, each timer tick is half a usec
#define TIMER_TICKS_PER_USEC                                            2
#else
#define TIMER_TICKS_PER_USEC                                            1
#endif

bdmf_error_t rdd_spdsvc_init ( void );

bdmf_error_t rdd_spdsvc_gen_config ( uint32_t xi_kbps,
                                 uint32_t xi_mbs,
                                 uint32_t xi_copies,
                                 uint32_t xi_total_length,
                                 uint32_t xi_test_time_ms );

bdmf_error_t rdd_spdsvc_get_tx_result ( uint8_t *xo_running_p,
                                        uint32_t *xo_tx_packets_p,
                                        uint32_t *xo_tx_discards_p );

bdmf_error_t rdd_spdsvc_gen_start(pbuf_t *pbuf,
                                  const rdpa_cpu_tx_info_t *info,
                                  RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx);

#if !defined(G9991)
bdmf_error_t rdd_wlan_spdsvc_gen_start(pbuf_t *pbuf,
                                       const rdpa_dhd_tx_post_info_t *info,
                                       RDD_DHD_CPU_QM_DESCRIPTOR_DTS *cpu_tx);
bdmf_error_t rdd_wlan_spdsvc_gen_complete(void);
#endif

bdmf_error_t rdd_spdsvc_analyzer_config(void);

bdmf_error_t rdd_spdsvc_analyzer_delete(void);

bdmf_error_t rdd_spdsvc_analyzer_get_rx_time(uint32_t *rx_time_us);

void rdd_spdsvc_gen_params_init(uint32_t kbps, uint32_t mbs, uint32_t total_lengh,
    uint32_t copies);

#endif /* _RDD_SPDSVC_H_ */
