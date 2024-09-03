/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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


#ifndef _RDD_MIRRORING_H
#define _RDD_MIRRORING_H

#include "rdd.h"
#include "rdd_common.h"
#include "rdpa_types.h"
#include "rdp_platform.h"

#if defined(BCM_DSL_XRDP)
#define RDD_MIRRORING_GET_TM_LAN_TASK(lan_port) IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER
#elif !defined(G9991_COMMON)
#define RDD_MIRRORING_GET_TM_LAN_TASK(lan_port) (IMAGE_0_DS_TM_LAN0_THREAD_NUMBER + lan_port)
#endif /* G9991_COMMON */
#define IS_MIRRORING_CFG(qm_queue) (qm_queue <= QM_QUEUE_LAST) 
#define ALL_DS_CORES (-1)


typedef enum tx_mirror_type_e
{
    MIRROR_OF_LAN = 0,  /* true if port being mirrored is emac port */
    MIRROR_OF_WAN = 1,  /* WAN including AE in 6888\6837 */
    MIRROR_OF_WLAN = 2, /* WLAN handle in processing */
} tx_mirror_type_e;

typedef struct
{
    bbh_id_e src_tx_bbh_queue_id;
    tx_mirror_type_e tx_mirror_type;
    int wlan_radio_idx; /* -1 for unset */
    uint16_t rx_dst_queue;
    uint16_t tx_dst_queue;
    rdd_vport_id_t  rx_dst_vport;
    rdd_vport_id_t  tx_dst_vport;
#if defined(MULTIPLE_BBH_TX_LAN)
    int32_t core_id;
#endif 
#ifdef CONFIG_MIRROR_MAX_PKT_SIZE
    uint16_t tx_truncate_size;
#endif
} rdd_mirroring_cfg_t;

/* API to RDPA level */
void rdd_mirroring_set(rdd_mirroring_cfg_t *rdd_mirroring_cfg);
void rdd_mirroring_set_rx(rdd_mirroring_cfg_t *rdd_mirroring_cfg);
void rdd_mirroring_set_tx(rdd_mirroring_cfg_t *rdd_mirroring_cfg);
void rdd_mirroring_set_tx_src_en(bdmf_boolean is_lan, int wlan_radio_idx, bbh_id_e src_tx_bbh_queue_id, int32_t core_id, bdmf_boolean mirror_enable);
void rdd_mirror_tx_disable(void);

/* Init Function */
void rdd_mirroring_cfg_init(void);

#endif
