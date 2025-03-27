/*
    Copyright (c) 2022 Broadcom
    All Rights Reserved

    <:label-BRCM:2022:DUAL/GPL:standard
    
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
