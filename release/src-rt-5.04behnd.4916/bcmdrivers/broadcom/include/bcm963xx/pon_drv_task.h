/*
   Copyright (c) 2020 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2020:DUAL/GPL:standard

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
