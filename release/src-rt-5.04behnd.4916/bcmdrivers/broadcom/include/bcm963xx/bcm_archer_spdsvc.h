/*
    Copyright (c) 2022 Broadcom
    All Rights Reserved

    <:label-BRCM:2022:DUAL/GPL:standard
    
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
* File Name  : bcm_archer_spdsvc.h
*
* Description: Archer Speed Service Header File
*
*******************************************************************************
*/

#ifndef __BCM_ARCHER_SPDSVC_H__
#define __BCM_ARCHER_SPDSVC_H__

/*
**************************************
* Interface for Archer Speed Service
**************************************
*/

#include "spdsvc_defs.h"

typedef union {
    struct {
        uint16_t egress_port;
        uint8_t egress_queue;
        uint8_t tc;
    } enet;
    uint32_t u32;
} archer_spdsvc_tag_t;

typedef int (*spdsvc_tr471_burst_cmpl_func_t)(void);

int archer_spdsvc_tr471_generator_enable(pNBuff_t pNBuff, spdsvc_config_tr471_t *tr471_p,
                                         spdsvc_egress_type_t egress_type, uint32_t tag,
                                         spdsvc_tr471_burst_cmpl_func_t burst_cmpl_func);

int archer_spdsvc_tr471_analyzer_enable(spdsvc_socket_t *socket_p,
                                        spdsvc_config_tr471_t *tr471_p,
                                        tr471_rx_queue_write_t rx_queue_write);

int archer_spdsvc_tr471_analyzer_disable(void);

int archer_spdsvc_get_result(spdsvc_result_t *result_p);

#endif /* __BCM_ARCHER_SPDSVC_H__ */
