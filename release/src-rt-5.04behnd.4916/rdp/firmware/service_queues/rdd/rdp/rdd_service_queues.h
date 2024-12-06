/*
   Copyright (c) 2013 Broadcom
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
:>
*/

#ifndef _RDD_SERVICE_QUEUES_H_
#define _RDD_SERVICE_QUEUES_H_

#include "rdd_defs.h"

#define SERVICE_QUEUE_RATE_LIMITER_IDLE     0x3f
#define SERVICE_QUEUE_RATE_LIMITER_OVERALL  32

typedef uint32_t rdd_service_queue_id_t;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queues_initialize                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - initialize                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   Initialize service queues memrory and descriptors                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
void rdd_service_queues_initialize ( void );


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_wan_tx_queue_svcq_scheduler_set                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - configure svcq scheduler on wan_tx_queue                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures service queue scheduler on wan_tx_queue                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   wan_channel - wan channel of wan_tx_queue                                */
/*   rate_cntrl - rate controller of wan channel                              */
/*   tx_queue - queue id of wan queue                                         */
/*   enable - enable or disable service queue scheduling on this queue        */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   bdmf status                                                              */
/*                                                                            */
/******************************************************************************/
BL_LILAC_RDD_ERROR_DTE rdd_wan_tx_queue_svcq_scheduler_set ( RDD_WAN_CHANNEL_ID                   xi_wan_channel_id,
                                                             BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE  xi_rate_controller_id,
                                                             BL_LILAC_RDD_QUEUE_ID_DTE            xi_queue_id,
                                                             bdmf_boolean                         enable );

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queue_cfg                                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - configure ingress (service) queue                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures ingress queue:                                                */
/*   - the packet threshold indicates how many packets can be TX pending      */
/*     behind a ingress queue.                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   dir - queue direction                                                    */
/*   queue_id - ingress queue index                                           */
/*   packet_threshold - ingress queue packet threshold. Overriden if WRED     */
/*   and/or drop precedence is used                                           */
/*   rate_limit - enable/disable rate limiter                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
void rdd_service_queue_cfg(rdd_service_queue_id_t queue_id, uint16_t pkt_threshold, bdmf_boolean rate_limit);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queue_addr_cfg                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - configure service queue DDR address                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   configures GPON TX queue:                                                */
/*   - only 256 TX queues can be configured by dynamic allocation             */
/*   - the packet threshold indicates how many packets can be TX pending      */
/*     behind a TX queue.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   queue_id - GPON TX queue index                                           */
/*   ddr_address - queue address in DDR                                       */
/*   queue_size - max queue size for wrap around                              */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/******************************************************************************/
void rdd_service_queue_addr_cfg (rdd_service_queue_id_t queue_id, uint32_t ddr_address, uint16_t queue_size); 


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queue_status_get                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - read statistics counters                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the operation                        */
/*                                                                            */
/* Registers :                                                                */
/*                                                                            */
/*   none                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   queue_id - the service queue to be read                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   stat - a structure that hold all the counters of a queue                 */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
int rdd_service_queue_status_get(rdd_service_queue_id_t queue_id, rdpa_stat_1way_t *stat);


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queue_overall_rate_limiter_enable                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - service queue overall rate limiter mode set             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the mode of overall rate limiter for the        */
/*   service queues. To enable the rate limiter, budget should be configured  */
/*   in function "rdd_ds_service_queue_rate_limiter_config".                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   enable  - enable / disable  overall rate limiter                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
void rdd_service_queue_overall_rate_limiter_enable(bdmf_boolean enable);

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   rdd_service_queue_rate_limiter_cfg                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   Service queues - TM ingress queues configuration.                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures downstream rate limiter for ingress queue EMAC, */
/*   the rate limiter get periodical budget, and when the budget exceeds, the */
/*   queue does not pass the PD to the egress queue. The budget is accumulated*/
/*   when not used until budget limit is reached.                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   rate_limiter_id                                                          */
/*   allocated_budget - the allocated budget per rate limiter in bytes per    */
/*   second                                                                   */
/*   budget_limit - bucket limit in allocation cycle. the limit budget per    */
/*   rate limiter must be >= MTU, and larger than budget allocation in        */
/*   allocation cycle                                                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   none                                                                     */
/*                                      .                                     */
/*                                                                            */
/******************************************************************************/
void rdd_service_queue_overall_rate_limiter_cfg(rdd_rate_cntrl_params_t *budget);

void rdd_service_queue_rate_limiter_cfg(uint32_t rate_limiter, rdd_rate_cntrl_params_t *budget);

#endif /* _RDD_SERVICE_QUEUES_H_ */
