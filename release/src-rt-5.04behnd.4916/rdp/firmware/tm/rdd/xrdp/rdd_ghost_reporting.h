/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#ifndef _RDD_GHOST_REPORTING_H
#define _RDD_GHOST_REPORTING_H

#include "rdd.h"

#define MAX_NUM_OF_WAN_CHANNEL          32

/* xgpon mac report - w.a for A0 */
#define XGPON_MAC_REPORT_ADDRESS        0x80168170

/* API to RDPA level */
bdmf_error_t rdd_ghost_reporting_timer_set(void);
bdmf_error_t rdd_ghost_reporting_mapping_queue_to_wan_channel(uint8_t queue_id, uint8_t wan_channel, bdmf_boolean enable);
void rdd_ghost_reporting_mac_type_init(uint8_t mac_type);
void rdd_ghost_reporting_last_queue_init(uint8_t last_queue);
void rdd_ghost_reporting_set_disable(bdmf_boolean disable);

/* RDD debug function */
/* API to read accumulated report per wan_channel */
bdmf_error_t rdd_ghost_reporting_ctr_get(uint8_t wan_channel, uint32_t *report);

#endif

