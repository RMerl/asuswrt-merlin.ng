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

#ifndef _RDPA_RATE_LIMIT_H_
#define _RDPA_RATE_LIMIT_H_

/**
 * \defgroup rate_limit Traffic Rate_limit
 * API in this group controls Runner Traffic Rate limit capabilities. it allows you to limit the
 * rate of traffic.\n
 * Rate_limit can be assigned to queue or egress_tm 
 * \ingroup tm
 * @{
 */

typedef struct {
    bdmf_rate_t af_rate;                   /**< AF (assured forwarding) rate min rate - 100,000bps */
    bdmf_rate_t be_rate;                   /**< BE (best effort) rate */
} rdpa_rl_cfg_t;

/** Rate_limit configuration.
 * Underlying type for tm_rate_limit_cfg aggregate type
 */
typedef struct {
    bdmf_rate_t af_rate;                   /**< AF (assured forwarding) rate min rate - 100,000bps */
    bdmf_rate_t be_rate;                   /**< BE (best effort) rate */
    bdmf_rate_t burst_size;                /**< Burst size */
    uint32_t    residue;                   /**< residue */
} rdpa_tm_rl_cfg_t;

/* @} end of rate_limit Doxygen group */

#define RDPA_TM_MAX_RATE_LIMIT    256 /**< Max number of rate limiters */
int rdpa_rate_limit_get_data(bdmf_object_handle _obj_, rdpa_tm_rl_cfg_t *cfg);

#endif /* _RDPA_RATE_LIMIT_H_ */
