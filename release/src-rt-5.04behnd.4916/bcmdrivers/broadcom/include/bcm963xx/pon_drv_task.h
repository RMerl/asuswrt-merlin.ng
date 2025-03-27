/*
   Copyright (c) 2020 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2020:DUAL/GPL:standard

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

#ifndef _PON_DRV_TASK_H_
#define _PON_DRV_TASK_H_
#include <bdmf_system.h>
#include <linux/bcm_log.h>
#include "pon_drv_nl.h"

#define NUM_MSGS 32

typedef enum {
    msg_first,
    msg_info,
    msg_job,
    msg_synce_register_handlers,
    msg_call_synce_wan_linkup_handler,
    msg_call_synce_ae_linkup_handler,
    msg_call_synce_wan_linkdown_handler,
    msg_call_synce_eth_up_handler,
    msg_call_synce_eth_down_handler,
    msg_call_synce_timer_A5_handler,
    msg_call_synce_timer_B1_handler,
    msg_call_synce_timer_C5_handler,
    msg_terminate,
    msg_last,
} pon_drv_msg_type;

#define ILLEGAL_PON_DRV_MSG_TYPE(s)     (((s) <= msg_first) || ((s) >= msg_last))
#define PON_DRV_MSG_TYPE_NAME(s)        (ILLEGAL_PON_DRV_MSG_TYPE(s) ? "out of range" : pon_drv_msg_type_names[s])


typedef struct {
    pon_drv_msg_type type;
    union {
        char info[64];
        char job[NETLINK_MSG_MAX_LEN];   /* null terminated, comma separated.  expand as job names and params grow */
        struct {
            void (*synce_wan_linkup_handler)(void);
            void (*synce_ae_linkup_handler)(void);
            void (*synce_wan_linkdown_handler)(void);
            void (*synce_tA5_handler)(void);
            void (*synce_tB1_handler)(void);
            void (*synce_tC5_handler)(void);
            void (*synce_eth_up_handler)(void);
            void (*synce_eth_down_handler)(void);
        };
    };
} pon_drv_msg;

struct pon_drv_task_data
{
    bdmf_queue_t pon_drv_task_msg_q;
};

extern struct pon_drv_task_data pon_drv_task_data;

void pon_drv_task_init(void);
void pon_drv_task_uninit(void);
int send_message_to_pon_drv_task(pon_drv_msg_type t, ...);

#endif
