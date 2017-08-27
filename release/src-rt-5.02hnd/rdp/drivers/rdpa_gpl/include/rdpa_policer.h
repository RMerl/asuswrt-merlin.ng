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

#define RDPA_TM_MAX_US_POLICER 16 /**< Max number of US policers */
#define RDPA_TM_MAX_DS_POLICER 16 /**< Max number of DS policers */

/** Traffic policer type */
typedef enum {
    rdpa_tm_policer_token_bucket,   /**< Simple tocken bucket */

    rdpa_tm_policer_type__num_of,   /* Number of possible types */
} rdpa_tm_policer_type;

/** Traffic policer action type */
typedef enum {
    rdpa_tm_policer_action_none,        /**< Do nothing */
    rdpa_tm_policer_action_drop,        /**< Discard */

    rdpa_tm_policer_action__num_of,     /* Number of possible actions */
} rdpa_tm_policer_action;

/** Policer configuration.
 * Underlying type for tm_policer_cfg aggregate type
 */
typedef struct {
    rdpa_tm_policer_type  type;             /**< Policer type */
    uint32_t commited_rate;                 /**< Committed Information Rate (CIR) - bps */
    uint32_t committed_burst_size;          /**< Committed Burst Size (CBS) - bytes */
    rdpa_tm_policer_action red_action;      /**< Action for non-conforming packets */
} rdpa_tm_policer_cfg_t;

/** Policer statistics.
 * Underlying structure for tm_policer_stat aggregate type
 */
typedef struct {
    rdpa_stat_t red;            /**< Red statistics */
} rdpa_tm_policer_stat_t;

#define RDPA_POLICER_MIN_SR       64000   /**< Min sustain rate */
#define RDPA_POLICER_MAX_SR       1000000000 /**< Max sustain rate */
#define RDPA_POLICER_SR_QUANTA    100   /**< Rate quanta */

/* @} end of policer Doxygen group */

#endif /* _RDPA_POLICER_H_ */
