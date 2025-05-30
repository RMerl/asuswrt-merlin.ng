/*
 Copyright 2018 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2018:DUAL/GPL:standard    
 
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
#ifndef _BCMXTM_RUNNER_EX_H_
#define _BCMXTM_RUNNER_EX_H_

#include <rdpa_api.h>
#include <rdpa_mw_cpu_queue_ids.h>

#if defined(CONFIG_BCM_RDP)
#include "rdd_defs.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_runner_defs.h"
#include "rdpa_cpu_helper.h"
#else
#include "rdd_data_structures_auto.h"
#endif

#if IS_ENABLED(CONFIG_BCM_DPI)
#include "bcmdpi.h"
#endif

#if defined(CONFIG_BCM_XRDP)

/* For XRDP, since XTM exists as a separate CPU object instance, we use the
** initial set of queues between CPU Rx/Runner Rx/
**/

#define QUEUE_DROP_THRESHOLD(QSize)  (QSize*1518) /*  in bytes, 256-byte size units. */
#define XTM_RDPA_CPU_PORT     rdpa_cpu_xtm

/* We have 8 MBR profiles as per QM MBR. We need to make sure to map atleast 2
 * priority queues to one profile.
 * Our criteria is to map resv buffers which in turn gets mapped to MBR profile
 * internally based on priority only for now.
 * We can have 16 queues for XTM at max.
 */
#define QUEUE_RESV_BUFFERS(MBR,prio)    (MBR-((MAX_SUB_PRIORITIES-1-(prio/2))*50))

#ifdef RDPA_XTM_CPU_HI_RX_QUEUE_ID
#undef RDPA_XTM_CPU_HI_RX_QUEUE_ID
#endif

#ifdef RDPA_XTM_CPU_LO_RX_QUEUE_ID
#undef RDPA_XTM_CPU_LO_RX_QUEUE_ID
#endif

#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
#define RDPA_XTM_CPU_HW_FIREWALL_RX_QUEUE_ID 0
#define RDPA_XTM_CPU_LO_RX_QUEUE_ID          1
#define RDPA_XTM_CPU_HI_RX_QUEUE_ID          2
#else
#define RDPA_XTM_CPU_LO_RX_QUEUE_ID          0
#define RDPA_XTM_CPU_HI_RX_QUEUE_ID          1
#endif

#ifdef RDPA_XTM_CPU_RX_QUEUE_ID_BASE
#undef RDPA_XTM_CPU_RX_QUEUE_ID_BASE
#endif

#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
#define RDPA_XTM_CPU_RX_QUEUE_ID_BASE RDPA_XTM_CPU_HW_FIREWALL_RX_QUEUE_ID
#else
#define RDPA_XTM_CPU_RX_QUEUE_ID_BASE RDPA_XTM_CPU_LO_RX_QUEUE_ID
#endif

#define US_CHANNEL_OFFSET_DSL    RDD_US_CHANNEL_OFFSET_DSL

#else

/* For RDP, since XTM exists as part of the global CPU object instance, we use the
** middle set of queues defined already in global RDPA definitions between CPU Rx/Runner Rx 
** as the inital set of queues in use for other interfaces.
**/

#define QUEUE_DROP_THRESHOLD(QSize)  QSize

#define QUEUE_RESV_BUFFERS(MBR,prio)  0  /* No support HW accel. Supported in FW in a different way. pri low-high : 0...7 */

#define XTM_RDPA_CPU_PORT            rdpa_cpu_host

#define US_CHANNEL_OFFSET_DSL    RDD_WAN_CHANNEL_1

#endif /* end of else defined(CONFIG_BCM_XRDP) */

#define XTM_RDPA_CPU_HI_RX_QUEUE_IDX    (RDPA_XTM_CPU_HI_RX_QUEUE_ID - RDPA_XTM_CPU_RX_QUEUE_ID_BASE)
#define XTM_RDPA_CPU_LO_RX_QUEUE_IDX    (RDPA_XTM_CPU_LO_RX_QUEUE_ID - RDPA_XTM_CPU_RX_QUEUE_ID_BASE)
#define XTM_RDPA_CPU_RX_QUEUE_IDX(hw_q) (hw_q - RDPA_XTM_CPU_RX_QUEUE_ID_BASE)
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
#define XTM_RDPA_CPU_HW_FIREWALL_RX_QUEUE_IDX    (RDPA_XTM_CPU_HW_FIREWALL_RX_QUEUE_ID - RDPA_XTM_CPU_RX_QUEUE_ID_BASE)
#endif

#ifdef TM_C_CODE
#define RDPA_TM_SECONDARY_SERVICE_QUEUE_ENABLE 0
#define RDPA_TM_SECONDARY_LEVEL rdpa_tm_level_secondary
#define RDPA_TM_SECONDARY_SCHEDULER_MODE rdpa_tm_sched_sp
#define RDPA_TM_SECONDARY_NUM_QUEUES 4
#undef RDPA_TM_SECONDARY_SP_ELEMENTS 
#else
#define RDPA_TM_SECONDARY_SERVICE_QUEUE_ENABLE 1
#define RDPA_TM_SECONDARY_LEVEL rdpa_tm_level_queue
#define RDPA_TM_SECONDARY_SCHEDULER_MODE rdpa_tm_sched_sp_wrr
#define RDPA_TM_SECONDARY_NUM_QUEUES 8
#define RDPA_TM_SECONDARY_SP_ELEMENTS 8
#endif
#define RDPA_TM_SECONDARY_DEFAULT_WEIGHT 1

int bcmxapiex_cpu_object_get (bdmf_object_handle  *xtm_obj);
int bcmxapiex_ring_create_delete(int q_id, int size, rdpa_cpu_rxq_cfg_t *rxq_cfg);
int bcmxapiex_get_pkt_from_ring(int hw_q_id, FkBuff_t **ppFkb, rdpa_cpu_rx_info_t *info);
int bcmxapiex_add_proc_files(struct proc_dir_entry *dir);
int bcmxapiex_del_proc_files(struct proc_dir_entry *dir);
int bcmxapiex_runner_xtm_objects_init(bdmf_object_handle wan);
int bcmxapiex_cfg_cpu_ds_queues (rdpa_cpu_reason reason, uint8_t tc, uint8_t queue_id);
void bcmxapiex_SetOrStartTxQueue (rdpa_tm_queue_cfg_t *pQueueCfg, bdmf_object_handle egress_tm);
void bcmxapiex_StopTxQueue (rdpa_tm_queue_cfg_t *pQueueCfg, bdmf_object_handle egress_tm);
void bcmxapiex_ShutdownTxQueue (UINT32 queueIdx, bdmf_object_handle egress_tm);
int bcmxapiex_dpi_egress_tm_rl_rate_mode_set(bdmf_object_handle tm_attr,
		rdpa_tm_rl_rate_mode rl_rate_mode);
int bcmxapiex_dpi_add_best_effort_sub_queues(bdmf_object_handle owner,
                rdpa_tm_queue_cfg_t *parent_queue, bdmf_index idx);

#endif /* _BCMXTM_RUNNER_EX_H_ */
