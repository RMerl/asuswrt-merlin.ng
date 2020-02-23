/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
void rdd_ghost_reporting_set_disable(bdmf_boolean disable);

/* RDD debug function */
/* API to read accumulated report per wan_channel */
bdmf_error_t rdd_ghost_reporting_ctr_get(uint8_t wan_channel, uint32_t *report);

#endif

