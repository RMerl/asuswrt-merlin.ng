/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
 *******************************************************************************
 * File Name  : archer_cpu_queues.h
 *
 *******************************************************************************
 */

#ifndef __ARCHER_CPU_QUEUES_H_INCLUDED__
#define __ARCHER_CPU_QUEUES_H_INCLUDED__

#include <linux/slab.h>
#include <linux/nbuff.h>
#include <archer.h>

typedef enum
{
    CPU_RX_HI = 0,
    CPU_RX_LO,

} CPU_RX_QUEUE_NUM;

typedef enum
{
    WAN_CPU_TX = 0,
    LAN_CPU_TX,
    CPU_TX_Q_MAX

} CPU_TX_QUEUE_NUM;

#define CPU_RX_QUEUE_MAX    2
#define CPU_TX_QUEUE_MAX    2

typedef enum {
    ARCHER_HOST_TYPE_ENET,
    ARCHER_HOST_TYPE_XTMRT,
    ARCHER_HOST_TYPE_MAX
} archer_host_type_t;

typedef void (*TX_NOTIFIER)(void);

typedef union {
    struct {
        uint16_t data_len;
        uint8_t ingress_port;
        struct {
            uint8_t rx_csum_verified    : 4;
            uint8_t mcast_fwd_exception : 4;
        };
    };
    uint32_t u32;
} archer_cpu_rx_info_enet_t;

typedef union {
    struct {
        struct {
            uint16_t data_len             : 15;
            uint16_t mcast_fwd_exception  : 1;
        };
        uint16_t desc_status;
    };
    uint32_t u32;
} archer_cpu_rx_info_dsl_t;

typedef union {
    struct {
        uint16_t data_len;
        uint8_t ingress_port;
        uint8_t is_packet_copy;
    };
    uint32_t u32;
} archer_cpu_rx_info_wlan_t;

typedef union {
    archer_cpu_rx_info_enet_t enet;
    archer_cpu_rx_info_dsl_t dsl;
    archer_cpu_rx_info_wlan_t wlan;
    uint32_t u32;
} archer_cpu_rx_info_t;

typedef struct {
    archer_host_type_t host_type;
    int (* tx_queue_notifier_register)(int q_id, TX_NOTIFIER tx_notifier);
    int (* tx_queue_read)(int q_id, pNBuff_t *ppNBuff, int *param_p, int *egress_queue_p);
    int (* tx_queue_not_empty)(int q_id);
    int (* rx_queue_write)(int q_id, uint8_t *pData, archer_cpu_rx_info_t cpu_rx_info);
    void (* recycle_queue_write)(pNBuff_t pNBuff);
    void (* queue_stats)(void);
    struct task_struct *archer_task_p;
} archer_host_hooks_t;

typedef struct {
    int (* queue_write)(int qid, FkBuff_t *fkb_p);
    void (* queue_notify)(int qid);
    void (* queue_stats)(void);
    char *(* queue_dev_name)(int qid);
    } archer_wfd_hooks_t;

typedef uint32_t (* archer_wfd_bulk_get_t)(unsigned long queue_index, unsigned long budget, int *work_done_p);

typedef struct {
    int radio_index;
    int queue_index;
    archer_wfd_bulk_get_t wfd_bulk_get;
} archer_wfd_config_t;

/* function API exported by XTM driver */
typedef struct {
    int (* deviceDetails)(uint32_t devId, uint32_t encap, uint32_t traffic_type, uint32_t bufStatus, uint32_t headerLen, uint32_t trailerLen);
    int (* xtmLinkUp)(uint32_t devId, uint32_t matchId, uint8_t txVcid);
    int (* reInitDma)(void);
    int (* txdmaEnable)(uint32_t dmaIndex, uint32_t txVcid);
    int (* txdmaDisable)(uint32_t dmaIndex);
    int (* setTxChanDropAlg)(int queue_id, archer_drop_config_t *cfg);
    int (* txdmaGetQStats)(int queue_id, archer_txq_stats_t *pstats);
    uint32_t (* txdmaGetQSize)(void);
} archer_xtm_hooks_t;

#endif /* __ARCHER_CPU_QUEUES_H_INCLUDED__ */
