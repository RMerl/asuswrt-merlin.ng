/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/

#ifndef __SYSPORT_DRIVER_H_INCLUDED__
#define __SYSPORT_DRIVER_H_INCLUDED__

#if defined(CONFIG_BCM_ENET_SYSPORT)
#include "sysport_driver_linux.h"
#else
#include <archer_cpu_queues.h>
#endif

#if defined(CONFIG_BCM947622)
#define CC_SYSPORT_DRIVER_TM
#endif

#define SYSPORT_DRIVER_RXQ_INDEX_WAN   0
#define SYSPORT_DRIVER_RXQ_INDEX_LAN   0

typedef enum {
    SYSPORT_DRIVER_SWITCH_INTERNAL=0,
    SYSPORT_DRIVER_SWITCH_EXTERNAL,
    SYSPORT_DRIVER_SWITCH_NONE,
    SYSPORT_DRIVER_SWITCH_MAX
} sysport_driver_switch_t;

typedef enum {
    SYSPORT_DRIVER_MLT_CMD_WRITE,
    SYSPORT_DRIVER_MLT_CMD_SEARCH,
    SYSPORT_DRIVER_MLT_CMD_CLEAR,
    SYSPORT_DRIVER_MLT_CMD_MAX
} sysport_driver_mlt_cmd_t;

typedef enum {
    SYSPORT_DRIVER_MLT_MAC_DA_TYPE_STA = 0,
    SYSPORT_DRIVER_MLT_MAC_DA_TYPE_HOST,
    SYSPORT_DRIVER_MLT_MAC_DA_TYPE_MAX
} sysport_driver_mlt_mac_da_type_t;

typedef struct {
    union {
        struct {
            uint32_t mac_da_47_32 : 16;
            uint32_t rxq_index    : 3;
            uint32_t mac_da_type  : 1; // sysport_driver_mlt_mac_da_type_t
            uint32_t valid        : 1;
            uint32_t reserved     : 11;
        };
        uint32_t data1;
    };
    union {
        uint32_t mac_da_31_0;
        uint32_t data0;
    };
} sysport_driver_mlt_data_t;

typedef enum {
    SYSPORT_DRIVER_MLT_LOCATION_HASH = 0,
    SYSPORT_DRIVER_MLT_LOCATION_CAM,
    SYSPORT_DRIVER_MLT_LOCATION_MAX
} sysport_driver_mlt_location_t;

typedef union {
    struct {
        uint16_t bin_entry_index  : 2;
        uint16_t bin_index        : 7;
        uint16_t table_index      : 1;
        uint16_t location         : 1; // sysport_driver_mlt_location_t
        uint16_t valid            : 1;
        uint16_t reserved         : 4;
    };
    uint16_t u16;
} sysport_driver_mlt_index_hash_table_t;

typedef union {
    struct {
        uint16_t cam_entry_index : 5;
        uint16_t zeros           : 5;
        uint16_t location        : 1; // sysport_driver_mlt_location_t
        uint16_t valid           : 1;
        uint16_t reserved        : 4;
    };
    uint16_t u16;
} sysport_driver_mlt_index_cam_t;

typedef union {
    sysport_driver_mlt_index_hash_table_t hash_table;
    sysport_driver_mlt_index_cam_t cam;
    struct {
        uint16_t zeros     : 10;
        uint16_t location  : 1; // sysport_mlt_location_t
        uint16_t valid     : 1;
        uint16_t reserved  : 4;
    };
    uint16_t u16;
} sysport_driver_mlt_index_t;

#define SYSPORT_DRIVER_MLT_CMD_TIMEOUT     -4
#define SYSPORT_DRIVER_MLT_CMD_ERROR       -3
#define SYSPORT_DRIVER_MLT_CMD_MISS_FULL   -2
#define SYSPORT_DRIVER_MLT_CMD_MISS        -1
#define SYSPORT_DRIVER_MLT_CMD_SUCCESS      0
#define SYSPORT_DRIVER_MLT_CMD_MISS_WRITE   1
#define SYSPORT_DRIVER_MLT_CMD_HIT_WRITE    2
#define SYSPORT_DRIVER_MLT_CMD_HIT          3

void sysport_driver_parser_mode_set(int l3_force);

int sysport_driver_mlt_cmd(sysport_driver_mlt_cmd_t cmd,
                           sysport_driver_mlt_data_t *data_p,
                           sysport_driver_mlt_index_t *index_p);

void sysport_driver_spe_config(int enable, void *phys_addr_p);

#define SYSPORT_DRIVER_SPE_CMD_INVALIDATE    0
#define SYSPORT_DRIVER_SPE_CMD_SET_STATIC    1
#define SYSPORT_DRIVER_SPE_CMD_CLEAR_STATIC  2

int sysport_driver_spe_cmd(int cmd, uint16_t cmdlist_index);

int sysport_driver_queue_map(void *arg_p);

int sysport_driver_switch_queue_to_txq_index(int logical_port, int switch_queue, int *txq_index_p);

int sysport_driver_switch_mode_get(int logical_port, int txq_index,
                                   sysport_driver_switch_t *switch_mode_p);

int sysport_driver_dev_to_logical_port(void *dev, int *logical_port_p);

int sysport_driver_logical_port_to_phys_port(int logical_port, int *intf_index_p, int *phys_port_p);

int sysport_driver_lookup_port_map(void *arg_p);

void sysport_driver_port_dump(void);

#if defined(CC_SYSPORT_DRIVER_TM)
void sysport_tm_enable(int enable);

int sysport_tm_enabled(void);

void sysport_tm_stats(void);

int sysport_tm_stats_get(int logical_port, int queue_index,
                         uint32_t *txPackets_p, uint32_t *txBytes_p,
                         uint32_t *droppedPackets_p, uint32_t *droppedBytes_p);

int sysport_tm_queue_set(int logical_port, int queue_index,
                         int min_kbps, int min_mbs, int max_kbps, int max_mbs);

int sysport_tm_queue_get(int logical_port, int queue_index,
                         int *min_kbps_p, int *min_mbs_p, int *max_kbps_p, int *max_mbs_p);

int sysport_tm_port_set(int logical_port, int kbps, int mbs);

int sysport_tm_phy_set(int logical_port, int kbps, int mbs);

int sysport_tm_port_get(int logical_port, int *kbps_p, int *mbs_p);

int sysport_tm_arbiter_set(int logical_port, sysport_tm_arbiter_t sysport_tm_arbiter);

int sysport_tm_arbiter_get(int logical_port, sysport_tm_arbiter_t *sysport_tm_arbiter_p);

int sysport_tm_mode_set(int logical_port, sysport_tm_mode_t mode);

int sysport_tm_mode_get(int logical_port, sysport_tm_mode_t *mode_p);

int sysport_tm_drop_set(int logical_port, int queue_index,
                        archer_drop_config_t *drop_config_p);

int sysport_tm_drop_get(int logical_port, int queue_index,
                        archer_drop_config_t *drop_config_p);
#endif /* CC_SYSPORT_DRIVER_TM */

int sysport_driver_drop_config_set(int logical_port, int switch_queue,
                                   archer_drop_config_t *drop_config_p);

int sysport_driver_drop_config_get(int logical_port, int switch_queue,
                                   archer_drop_config_t *drop_config_p);

uint32_t sysport_driver_tx_qsize_get(void);

void sysport_driver_stats(void);

int sysport_driver_txq_stats_get(int logical_port, int switch_queue,
                                 archer_txq_stats_t *stats_p);

void sysport_driver_reg_dump(void);

int sysport_driver_host_bind(archer_host_hooks_t *hooks_p);

int sysport_driver_host_config(void *arg_p);

int __init sysport_driver_construct(void);

int sysport_wol_init(void);
void sysport_wol_intf_dev_mapping (int port_idx, bcmSysport_BlogChnl_t *blog_chnl);
void sysport_wol_mpd_cfg (archer_mpd_cfg_t *mpd_cfg);
void sysport_wol_enter (char *dev_name);

#endif  /* defined(__SYSPORT_DRIVER_H_INCLUDED__) */
