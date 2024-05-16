/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
*
*    Copyright (c) 2021 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/

#include "linux/delay.h"
#include "linux/kthread.h"
#include <linux/string.h>
#include "bcmtypes.h"
#include "pon_drv.h"
#include "pon_drv_nl.h"
#include "pon_drv_task.h"
#include "pon_drv_serdes_util.h"

#define INITIALIZER(tag) [tag] = #tag  // can be used in any order to init an array of field names

const char *pon_drv_msg_type_names[] = {
    INITIALIZER(msg_first),
    INITIALIZER(msg_info),
    INITIALIZER(msg_job),
    INITIALIZER(msg_terminate),
    INITIALIZER(msg_synce_register_handlers),
    INITIALIZER(msg_call_synce_wan_linkup_handler),
    INITIALIZER(msg_call_synce_ae_linkup_handler),
    INITIALIZER(msg_call_synce_wan_linkdown_handler),
    INITIALIZER(msg_call_synce_eth_up_handler),
    INITIALIZER(msg_call_synce_eth_down_handler),
    INITIALIZER(msg_call_synce_timer_A5_handler),
    INITIALIZER(msg_call_synce_timer_B1_handler),
    INITIALIZER(msg_call_synce_timer_C5_handler),
    INITIALIZER(msg_last),
};

/* data structure */
struct pon_drv_task_data pon_drv_task_data;

/* sender.  can be invoked in atomic context */
int send_message_to_pon_drv_task(pon_drv_msg_type t, ...)
{
    bdmf_error_t msg_error;
    va_list parameters;
    pon_drv_msg pon_drv_message;
    char *str;

    /* Init the va database */
    va_start(parameters, t);
    pon_drv_message.type = t;
    switch (pon_drv_message.type) {
        case msg_info:
            str = va_arg(parameters, char *);
            BCM_ASSERT(snprintf(pon_drv_message.info, sizeof(pon_drv_message.info), "%s", str) < sizeof(pon_drv_message.info));
            break;

        case msg_job:
            str = va_arg(parameters, char *);
            BCM_ASSERT(snprintf(pon_drv_message.job, sizeof(pon_drv_message.job), "%s", str) < sizeof(pon_drv_message.job));
            break;

        case msg_synce_register_handlers:
            pon_drv_message.synce_wan_linkup_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_ae_linkup_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_wan_linkdown_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_tA5_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_tB1_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_tC5_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_eth_up_handler = va_arg(parameters, void (*)(void));
            pon_drv_message.synce_eth_down_handler = va_arg(parameters, void (*)(void));
            break;

        case msg_call_synce_wan_linkup_handler:
            __logDebug("%s: msg_call_synce_wan_linkup_handler", __FUNCTION__);
            break;

        case msg_call_synce_ae_linkup_handler:
            __logDebug("%s: msg_call_synce_ae_linkup_handler", __FUNCTION__);
            break;

        case msg_call_synce_eth_up_handler:
            __logDebug("%s: msg_call_synce_eth_up_handler", __FUNCTION__);
            break;

        case msg_call_synce_eth_down_handler:
            __logDebug("%s: msg_call_synce_eth_down_handler", __FUNCTION__);
            break;

        case msg_call_synce_wan_linkdown_handler:
            __logDebug("%s: msg_call_synce_wan_linkdown_handler", __FUNCTION__);
            break;

        case msg_call_synce_timer_A5_handler:
            __logDebug("%s: msg_call_synce_timer_A5_handler", __FUNCTION__);
            break;

        case msg_call_synce_timer_B1_handler:
            __logDebug("%s: msg_call_synce_timer_B1_handler", __FUNCTION__);
            break;

        case msg_call_synce_timer_C5_handler:
            __logDebug("%s: msg_call_synce_timer_C5_handler", __FUNCTION__);
            break;

        case msg_terminate:
            break;

        default:
            __logError("unexpected: default in switch, pon_drv_message.type==%d(%s)", pon_drv_message.type, PON_DRV_MSG_TYPE_NAME(pon_drv_message.type));
            dump_stack();
            break;
    }
    va_end(parameters);
    msg_error = bdmf_queue_send(&pon_drv_task_data.pon_drv_task_msg_q,
        (char *)(&pon_drv_message), sizeof(pon_drv_message));
    if (msg_error != BDMF_ERR_OK)
    {
        __logError("Error sending message %d", msg_error);
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(send_message_to_pon_drv_task);

/* thread */
struct task_struct *pon_drv_thread = NULL;
int  pon_drv_thread_arg = 0;

static int driver_thread(void *arg)
{
    uint32_t length;
    bdmf_error_t bdmf_error;
    pon_drv_msg msg;
    void (*synce_wan_linkup_handler)(void) = NULL;
    void (*synce_ae_linkup_handler)(void) = NULL;
    void (*synce_wan_linkdown_handler)(void) = NULL;
    void (*synce_eth_up_handler)(void) = NULL;
    void (*synce_eth_down_handler)(void) = NULL;
    void (*synce_tA5_handler)(void) = NULL;
    void (*synce_tB1_handler)(void) = NULL;
    void (*synce_tC5_handler)(void) = NULL;

    __logInfo("pon_drv_task thread started.");

    while(!kthread_should_stop())
    {
        length = sizeof(msg);

        bdmf_error = bdmf_queue_receive(&pon_drv_task_data.pon_drv_task_msg_q, (char *)&msg, &length);
        if (bdmf_error != BDMF_ERR_OK)
        {
            __logError("Error receving message %d\n", bdmf_error);
            continue;
        }

        __logDebug("received msg, type=%d(%s)", msg.type, PON_DRV_MSG_TYPE_NAME(msg.type));
        switch (msg.type) {
            case msg_info:
                __logInfo("%s, jiffies=%lu", msg.info, jiffies);
                break;

            case msg_terminate:
                __logDebug("%s terminating...", __FUNCTION__);
                return 0;

            case msg_job:
                __logDebug("job list='%s'", msg.job);
                netlink_invoke_serdes_job(msg.job);
                break;

            case msg_synce_register_handlers:
                __logDebug("%s: msg_synce_register_handlers", __FUNCTION__);
                synce_wan_linkup_handler = msg.synce_wan_linkup_handler;
                synce_ae_linkup_handler = msg.synce_ae_linkup_handler;
                synce_wan_linkdown_handler = msg.synce_wan_linkdown_handler;
                synce_eth_up_handler = msg.synce_eth_up_handler;
                synce_eth_down_handler = msg.synce_eth_down_handler;
                synce_tA5_handler = msg.synce_tA5_handler;
                synce_tB1_handler = msg.synce_tB1_handler;
                synce_tC5_handler = msg.synce_tC5_handler;
                break;

            case msg_call_synce_wan_linkup_handler:
                __logDebug("%s: msg_call_synce_wan_linkup_handler", __FUNCTION__);
                if (synce_wan_linkup_handler)
                    (*synce_wan_linkup_handler)();
                else
                    __logError("unexpected: synce_wan_linkup_handler called before registered");
                break;

            case msg_call_synce_ae_linkup_handler:
                __logDebug("%s: msg_call_synce_ae_linkup_handler", __FUNCTION__);
                if (synce_ae_linkup_handler)
                    (*synce_ae_linkup_handler)();
                else
                    __logError("unexpected: synce_ae_linkup_handler called before registered");
                break;

            case msg_call_synce_timer_A5_handler:
                __logDebug("%s: msg_call_synce_timer_A5_handler", __FUNCTION__);
                if (synce_tA5_handler)
                    (*synce_tA5_handler)();
                else
                    __logError("unexpected: synce_tA5_handler called before registered");
                break;

            case msg_call_synce_timer_B1_handler:
                __logDebug("%s: msg_call_synce_timer_B1_handler", __FUNCTION__);
                if (synce_tB1_handler)
                    (*synce_tB1_handler)();
                else
                    __logError("unexpected: synce_tB1_handler called before registered");
                break;

            case msg_call_synce_timer_C5_handler:
                __logDebug("%s: msg_call_synce_timer_C5_handler", __FUNCTION__);
                if (synce_tC5_handler)
                    (*synce_tC5_handler)();
                else
                    __logError("unexpected: synce_tC5_handler called before registered");
                break;

            case msg_call_synce_eth_up_handler:
                __logDebug("%s: msg_call_synce_eth_up_handler", __FUNCTION__);
                if (synce_eth_up_handler)
                    (*synce_eth_up_handler)();
                else
                    __logError("unexpected: synce_eth_up_handler called before registered");
                break;

            case msg_call_synce_eth_down_handler:
                __logDebug("%s: msg_call_synce_eth_down_handler", __FUNCTION__);
                if (synce_eth_down_handler)
                    (*synce_eth_down_handler)();
                else
                    __logError("unexpected: synce_eth_down_handler called before registered");
                break;

            case msg_call_synce_wan_linkdown_handler:
                __logDebug("%s: msg_call_synce_wan_linkdown_handler", __FUNCTION__);
                if (synce_wan_linkdown_handler)
                    (*synce_wan_linkdown_handler)();
                else
                    __logError("unexpected: synce_wan_linkdown_handler called before registered");
                break;

            default:
                __logError("unexpected: default in switch, msg.type==%d(%s)", msg.type, PON_DRV_MSG_TYPE_NAME(msg.type));
                dump_stack();
                break;
        }
    }

    msleep(100);
    __logInfo("pon_drv_task thread asked to stop.");
    return 0;
}

/* init */
void pon_drv_task_init(void)
{
    bdmf_error_t bdmf_error;

    __logInfo("pon_drv_task_init.");
    bdmf_error = bdmf_queue_create(&pon_drv_task_data.pon_drv_task_msg_q, NUM_MSGS, sizeof(pon_drv_msg));
    if (bdmf_error != BDMF_ERR_OK)
    {
        __logError("bdmf_queue_create failed.");
    }

    if (IS_ERR(pon_drv_thread = kthread_run(driver_thread, &pon_drv_thread_arg, "pon_drv thread")))
    {
        __logError("Thread creation failed, rc=0x%lx", PTR_ERR(pon_drv_thread));
    }
    bcmFun_reg(BCM_FUN_ID_SEND_MESSAGE_TO_PON_DRV_TASK, (bcmFun_t *)send_message_to_pon_drv_task);
    bcmFun_reg(BCM_FUN_ID_WAN_SERDES_TYPE_GET, (bcmFun_t *)wan_serdes_type_get);
    bcmFun_reg(BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB_WITH_OUTPUT, (bcmFun_t *)netlink_invoke_serdes_job_to);
    bcmFun_reg(BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB, (bcmFun_t *)netlink_invoke_serdes_job);
    bcmFun_reg(BCM_FUN_ID_NCO_SW_HOLD_VAL_GET, (bcmFun_t *)nco_sw_hold_val_get);
}

/* uninit */
void pon_drv_task_uninit(void)
{
    int rc;

    __logInfo("pon_drv_task_uninit.");

    if ((rc = kthread_stop(pon_drv_thread)))
    {
        __logError("Thread stopping failed, rc=0x%lx", rc);
    }

    pon_drv_thread = NULL;

    bcmFun_dereg(BCM_FUN_ID_SEND_MESSAGE_TO_PON_DRV_TASK);
    bcmFun_dereg(BCM_FUN_ID_WAN_SERDES_TYPE_GET);
    bcmFun_dereg(BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB_WITH_OUTPUT);
    bcmFun_dereg(BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB);
}
