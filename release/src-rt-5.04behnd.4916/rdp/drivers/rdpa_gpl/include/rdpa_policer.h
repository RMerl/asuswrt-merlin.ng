/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/


#ifndef _RDPA_POLICER_H_
#define _RDPA_POLICER_H_

/**
 * \defgroup policer Traffic Policer
 * API in this group controls Runner Traffic Policing capabilities. Traffic policing allows you to control the maximum
 * rate of traffic.\n
 * Policer can be assigned to any type of  \ref ingress_class_d "classification flow". 
 * \ingroup tm
 * @{
 */

#include "rdd_fw_defs.h"
#if !defined(XRDP)
#define RDPA_TM_MAX_US_POLICER 16 /**< Max number of US policers */
#define RDPA_TM_MAX_DS_POLICER 16 /**< Max number of DS policers */
#define RDPA_TM_MAX_POLICER    (RDPA_TM_MAX_US_POLICER+RDPA_TM_MAX_DS_POLICER)
#else
#if CHIP_VER < RDP_GEN_62
#define RDPA_TM_MAX_POLICER    80 /**< Max number of policers */
#else
#define RDPA_TM_MAX_POLICER    255 /**< Max number of policers - cant use 256 as 255 used for non real policer*/
#endif
#endif

/** Traffic policer type */
typedef enum {
    rdpa_tm_policer_token_bucket,                                       
    rdpa_tm_policer_single_token_bucket = rdpa_tm_policer_token_bucket, /**< Single token bucket */
    rdpa_tm_policer_sr_overflow_dual_token_bucket,    /**< Dual token bucket single rate with overflow */
    rdpa_tm_policer_tr_dual_token_bucket,             /**< Dual token bucket two rate without overflow */  
    rdpa_tm_policer_tr_overflow_dual_token_bucket,    /**< Dual token bucket two rate with overflow */  
    rdpa_tm_policer_type__num_of,   /* Number of possible types */
} rdpa_tm_policer_type;

/** Policer configuration.
 * Underlying type for tm_policer_cfg aggregate type
 */
typedef struct {
    rdpa_tm_policer_type  type;             /**< Policer type */
    bdmf_rate_t commited_rate;              /**< Committed Information Rate (CIR) - bps (100K-10G) */
    uint32_t committed_burst_size;          /**< Committed Burst Size (CBS) - bytes (1K-100M) */
    bdmf_rate_t peak_rate;                  /**< PEAK Information Rate (PIR) - bps (100K-10G) */
    uint32_t peak_burst_size;               /**< PEAK Burst Size (PBS) - bytes (1K-100M) */
    uint8_t dei_mode;                       /**< DEI remark - Used for dual bucket only, affects outer dei only */
    uint8_t color_aware_enabled;            /**< Color aware enabled - Change policer coloring when DEI is set in received packets */ 
    int8_t rl_overhead;                     /**< Policer overhead to be added for every packet */
    /* (OPERATION_MODE_PRV) */
    uint8_t pbit_mode;                      /**< Pbit remark - Used for dual bucket only, affects outer Pbit only */
    uint8_t pbit_green_val;                 /**< Pbit green value - Used for green packets */
    uint8_t pbit_yellow_val;                /**< Pbit yellow value - Used for yellow packets */
} rdpa_tm_policer_cfg_t;

/** Policer statistics.
 * Underlying structure for tm_policer_stat aggregate type
 */
typedef struct {
    rdpa_stat_t green;          /**< green statistics */
    rdpa_stat_t yellow;         /**< yellow statistics */
    rdpa_stat_t red;            /**< Red statistics */
} rdpa_tm_policer_stat_t;


#define RDPA_POLICER_MIN_SR       64000   /**< Min sustain rate */

#if defined BCM6846 || defined(BCM6878)
#define RDPA_POLICER_MAX_SR       2500000000U /**< Max sustain rate */
#elif defined(XRDP)
#define RDPA_POLICER_MAX_SR       10000000000 /**< Max sustain rate */
#else
#define RDPA_POLICER_MAX_SR       1000000000 /**< Max sustain rate */
#endif

/* @} end of policer Doxygen group */

#endif /* _RDPA_POLICER_H_ */
