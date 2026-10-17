/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom 
      All Rights Reserved

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

/*
 *  Created on: Apr/2024
 *      Author: stefan.chulski@broadcom.com
 */ 
#ifndef __ENET_RUNNER_EGRESS_TC__
#define __ENET_RUNNER_EGRESS_TC__

#if defined(CONFIG_BCM_ENET_EGRESS_TC)
#include "enet_egress_tc.h"
#endif

#if defined(POLICER_SUPPORT)
#define RDPA_INVALID_KEY     (-1)
#define RDPA_INVALID_KEY_U64 (~0ULL)
#endif

#ifdef TM_C_CODE
#define RDPA_TM_SECONDARY_SERVICE_QUEUE_ENABLE 0
#define RDPA_TM_SECONDARY_LEVEL rdpa_tm_level_secondary
#undef RDPA_TM_SECONDARY_SP_ELEMENTS
#else
#define RDPA_TM_SECONDARY_SERVICE_QUEUE_ENABLE 1
#define RDPA_TM_SECONDARY_LEVEL rdpa_tm_level_queue
#define RDPA_TM_SECONDARY_SP_ELEMENTS  8
#endif

struct egress_tm_sched_dict {
    rdpa_tm_sched_mode mode;
};

struct qid_list_entry {
    int key;
    struct list_head list;
};

#define TC_ETS_MAX_Q_SERVICE_QUEUE_TM     16
#define TC_ETS_MAX_Q_SERVICE_QUEUE_TM_CTC 8

#define TC_CTC_NUM_OF_SUBSIDIARIARIES  7

int port_runner_tc_config_mqprio(enetx_port_t *self, struct tc_mqprio_qopt_offload *mqprio);
int port_runner_tc_config_egress_tm(enetx_port_t *self, bool init);
int port_runner_tc_config_svsq_egress_tm(enetx_port_t *self, bool init);
int port_runner_tc_config_policer(struct flow_action_entry *act);
int port_runner_tc_remove_policer(int index);
void port_runner_update_queues_stat(enetx_port_t *self, egress_tm_node *ets);
void port_runner_svsq_update_queues_stat(enetx_port_t *self, egress_tm_node *ets);
void runner_tc_remove_egress_tm(enetx_port_t *port, egress_tm_node *node);
void runner_tc_get_caps(enetx_port_t *self, void *caps);
#if defined(ENET_GPON) || defined(ENET_EPON)
int runner_pon_tc_config_egress_tm(enetx_port_t *self, bool init);
void port_runner_pon_update_queues_stat(enetx_port_t *self, egress_tm_node *ets);
#endif
#endif /* __ENET_RUNNER_EGRESS_TC__ */