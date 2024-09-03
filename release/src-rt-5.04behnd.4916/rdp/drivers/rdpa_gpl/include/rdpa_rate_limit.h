/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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
    int16_t     budget_id_sir;             /**< used budget_id_sir for reuse */
    int16_t     budget_id_pir;             /**< used budget_id_pir for reuse */
    bdmf_object_handle _obj_;              /**< ref for original rl object */
} rdpa_tm_rl_cfg_t;

/* @} end of rate_limit Doxygen group */


int rdpa_rate_limit_get_data(bdmf_object_handle _obj_, rdpa_tm_rl_cfg_t *cfg);

/* rate limiter object defines*/
#define RDPA_TM_MAX_RATE_LIMIT    256   /*< Max number of rate limiters */
#define RL_LINK_EMPTY -1                /* empty link to rte limit */
#define RL_LINK_OBJECT -2               /* object link to rate limit */

#define RL_SIR 0
#define RL_PIR 1
#endif /* _RDPA_RATE_LIMIT_H_ */
