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


#ifndef _RDD_DHD_HELPER_H
#define _RDD_DHD_HELPER_H



#include "rdd_dhd_helper_common.h"
#include "rdp_cpu_ring.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_proj_cntr.h"
#include "rdp_drv_fpm.h"
#include "rdp_drv_qm.h"
#include "rdp_mm.h"
#include "rdd_init.h"


#define RDD_CPU_TX_SKB_LIMIT_DEFAULT                        4096

#define BACKUP_QUEUE_ENTRIES                                (BACKUP_INDEX_FIFO_SIZE*RDPA_MAX_RADIOS)
#define BACKUP_QUEUE_SIZE_BYTES                             (BACKUP_QUEUE_ENTRIES*sizeof(RDD_DHD_BACKUP_ENTRY_DTS))

#if !defined(_CFE_) && defined(CONFIG_DHD_RUNNER)
#define RDD_DHD_BACKUP_QUEUE_MEM_SIZE                       (BACKUP_QUEUE_SIZE_BYTES + (BACKUP_QUEUE_ENTRIES) + 0x4000)
#else
#define RDD_DHD_BACKUP_QUEUE_MEM_SIZE                       (0)
#endif

void rdd_dhd_hw_cfg(RDD_DHD_HW_CONFIGURATION_DTS *dhd_hw_config);
void rdd_rx_post_descr_init(uint32_t radio_idx, uint8_t *descr_ptr, uint32_t fpm_buffer_number);
void rdd_complete_ring_init(uint32_t radio_idx, RING_DESCTIPTOR  *descriptor, bdmf_phys_addr_t phy_addr);
int rdd_dhd_helper_fpm_thresholds_set(uint16_t low_th, uint16_t high_th, uint16_t excl_th);
int rdd_dhd_helper_fpm_thresholds_get(uint16_t *low_th, uint16_t *high_th, uint16_t *excl_th);
int rdd_dhd_helper_tx_total_fpm_used_get(uint32_t *used);

extern int flow_ring_format[RDPA_MAX_RADIOS];
extern void *natc_mem_end_addr;
extern int32_t natc_mem_available_size;
extern dpi_params_t *p_dpi_cfg;

/* Waking up TX complete or RX complete threads */
static inline void rdd_dhd_helper_wakeup(uint32_t radio_idx, bdmf_boolean is_tx_complete)
{
#ifndef RDP_SIM   
   if (is_tx_complete)
      ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(dhd_complete_runner_image), DHD_TX_COMPLETE_0_THREAD_NUMBER + radio_idx);
   else
      ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(dhd_complete_runner_image), DHD_RX_COMPLETE_0_THREAD_NUMBER + radio_idx);   
#else 
  if (is_tx_complete)
      rdp_cpu_runner_wakeup(get_runner_idx(dhd_complete_runner_image), DHD_TX_COMPLETE_0_THREAD_NUMBER + radio_idx);
   else
      rdp_cpu_runner_wakeup(get_runner_idx(dhd_complete_runner_image), DHD_RX_COMPLETE_0_THREAD_NUMBER + radio_idx);          
#endif   
}

void rdd_wlan_mcast_dft_init(bdmf_phys_addr_t dft_phys_addr);

#endif /* _RDD_DHD_HELPER_H */

