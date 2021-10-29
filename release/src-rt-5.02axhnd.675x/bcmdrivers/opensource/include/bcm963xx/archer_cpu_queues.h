/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

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

typedef struct {
    archer_host_type_t host_type;
    int (* tx_queue_notifier_register)(int q_id, TX_NOTIFIER tx_notifier);
    int (* tx_queue_read)(int q_id, pNBuff_t *ppNBuff, int *param_p, int *egress_queue_p);
    int (* tx_queue_not_empty)(int q_id);
    int (* rx_queue_write)(int q_id, uint8_t **pData, int data_len, int ingress_port, int param);
    void (* recycle_queue_write)(pNBuff_t pNBuff);
    void (* queue_stats)(void);
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
    uint32_t (* txdmaGetQSize)(void);
} archer_xtm_hooks_t;

#endif /* __ARCHER_CPU_QUEUES_H_INCLUDED__ */
