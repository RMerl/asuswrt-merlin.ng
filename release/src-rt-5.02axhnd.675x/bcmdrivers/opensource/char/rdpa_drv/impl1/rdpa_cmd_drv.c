/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : rdpa_cmd_drv.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the RDPA Driver.
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/bcm_log.h>
#include <linux/sysrq.h>
#include <linux/device.h>


#include <linux/sysrq.h>

#include <rdpa_drv.h>
#include <rdpa_cmd_tm.h>
#include <rdpa_cmd_iptv.h>
#include <rdpa_cmd_ic.h>
#include <rdpa_cmd_sys.h>
#include <rdpa_cmd_port.h>
#include <rdpa_cmd_br.h>
#include <rdpa_cmd_llid.h>
#include <rdpa_cmd_ds_wan_udp_filter.h>
#include <rdpa_cmd_filter.h>
#include <rdpa_api.h>
#include <rdpa_cmd_dscp_to_pbit.h>
#include <rdpa_cmd_pbit_to_q.h>
#include <rdpa_cmd_misc.h>
#include <rdpa_cmd_cpu.h>
#ifdef CONFIG_BCM_TIME_SYNC_MODULE
#include "time_sync.h"
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include <ingqos.h>
#endif

#undef RDPA_CMD_DECL
#define RDPA_CMD_DECL(x) #x,

#if defined(CONFIG_BCM_LOG)
static const char *rdpa_cmd_ioctl_name[] =
{
    RDPA_CMD_DECL(RDPA_IOC_TM)
    RDPA_CMD_DECL(RDPA_IOC_IPTV)
    RDPA_CMD_DECL(RDPA_IOC_IC)
    RDPA_CMD_DECL(RDPA_IOC_SYS)
    RDPA_CMD_DECL(RDPA_IOC_PORT)
    RDPA_CMD_DECL(RDPA_IOC_BRIDGE)
    RDPA_CMD_DECL(RDPA_IOC_LLID)
    RDPA_CMD_DECL(RDPA_IOC_DS_WAN_UDP_FILTER)
    RDPA_CMD_DECL(RDPA_IOC_RDPA_MW_SET_MCAST_DSCP_REMARK)
    RDPA_CMD_DECL(RDPA_IOC_TIME_SYNC)
    RDPA_CMD_DECL(RDPA_IOC_FILTERS)
    RDPA_CMD_DECL(RDPA_IOC_DSCP_TO_PBIT)
    RDPA_CMD_DECL(RDPA_IOC_PBIT_TO_Q)
    RDPA_CMD_DECL(RDPA_IOC_MISC)
    RDPA_CMD_DECL(RDPA_IOC_MAX)
};
#endif

#if defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP)
#if defined(CONFIG_BLOG)
extern int rdpa_mw_set_mcast_dscp_remark;

static int set_mcast_dscp_remark(int arg)
{
    if (arg < RDPACTL_MCAST_REMARK_DISABLE || arg > 63)
        return -EINVAL;
    
    rdpa_mw_set_mcast_dscp_remark = arg;

    return 0;
}
#endif
#endif
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM_GPON) || defined(CONFIG_BCM_GPON_MODULE) || defined(CONFIG_BCM963158)
extern int rdpa_init_system_fiber(void);
#endif

static struct class *rdpa_cmd_class = NULL;

/*
 *------------------------------------------------------------------------------
 * Function Name: rdpa_cmdIoctl
 * Description  : Main entry point to handle user applications IOCTL requests.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static long rdpa_cmdIoctl(struct file *filep, unsigned int command, unsigned long arg)
{
    rdpa_drv_ioctl_t cmd;
    int ret = RDPA_DRV_SUCCESS;

    if (command >= RDPA_IOC_MAX)
        cmd = RDPA_IOC_MAX;
    else
        cmd = (rdpa_drv_ioctl_t) command;

    BCM_LOG_INFO(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA Char Device: cmd<%d> %s arg<0x%08lX>",
                 command, rdpa_cmd_ioctl_name[command], arg);

    switch( cmd )
    {
        case RDPA_IOC_TM:
        {
            ret = rdpa_cmd_tm_ioctl(arg);
            break;
        }

        case RDPA_IOC_IPTV:
        {
#if defined(CONFIG_BCM_DSL_RDP) || defined(CONFIG_BCM_DSL_XRDP)
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_IPTV is not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_iptv_ioctl(arg);
#endif
            break;
        }

        case RDPA_IOC_IC:
        {
#if defined(CONFIG_BCM_DSL_RDP) || defined(CONFIG_BCM_DSL_XRDP)
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_IC is not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_ic_ioctl(arg);
#endif
            break;
        }

        case RDPA_IOC_SYS:
        {
#if defined(CONFIG_BCM_DSL_RDP)
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_SYS is not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_sys_ioctl(arg);
#endif
            break;
        }

        case RDPA_IOC_PORT:
        {
#if defined(CONFIG_BCM_DSL_RDP) || defined(CONFIG_BCM_DSL_XRDP)
            BCM_LOG_INFO(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_PORT is not supported");
            ret = RDPA_DRV_UNSUPPORTED;
#else
            ret = rdpa_cmd_port_ioctl(arg);
#endif
            break;
        }

        case RDPA_IOC_BRIDGE:
        {
#if defined(CONFIG_BCM_DSL_RDP) || defined(CONFIG_BCM_DSL_XRDP)
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_BRIDGE is not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_br_ioctl(arg);
#endif
            break;
        }

        case RDPA_IOC_LLID:
        {
#if defined(CONFIG_BCM_DSL_RDP)
            /* It used to have defined(CONFIG_BCM_DSL_XRDP) as well, but we only
             * have 63158 as the only DSL_XRDP as this point and it supports EPON.
             * We might need adjustment to the ifdef in the future. */
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_LLIDis not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_llid_ioctl(arg);
#endif
            break;
        }

        case RDPA_IOC_DS_WAN_UDP_FILTER:
        {
#if defined(CONFIG_BCM_PON_RDP)
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA_IOC_DS_WAN_UDP_FILTER is not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_ds_wan_udp_filter_ioctl(arg);
#endif
            break;
        }

#if defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP)
        case RDPA_IOC_RDPA_MW_SET_MCAST_DSCP_REMARK:
        {
#if defined(CONFIG_BLOG)
            ret = set_mcast_dscp_remark(arg);
#else
            ret = 0;
#endif
            break;
        }
#endif

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM_GPON) || defined(CONFIG_BCM_GPON_MODULE) || defined(CONFIG_BCM963158)
        case RDPA_IOC_TIME_SYNC:
        {
#ifdef CONFIG_BCM_TIME_SYNC_MODULE
            ret = time_sync_init();
#endif
            break;
        }
#endif

#if defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP)
        case RDPA_IOC_DSCP_TO_PBIT:
        {
            ret = rdpa_cmd_dscp_to_pbit_ioctl(arg);
            break;
        }
        case RDPA_IOC_PBIT_TO_Q:
        {
            ret = rdpa_cmd_pbit_to_q_ioctl(arg);
            break;
        }
        
        case RDPA_IOC_MISC:
        {
            ret = rdpa_cmd_misc_ioctl(arg);
            break;
        }
#endif

        case RDPA_IOC_FILTERS:
        {
#if defined(CONFIG_BCM_DSL_RDP) || defined(CONFIG_BCM_DSL_XRDP)
            BCM_LOG_ERROR(RDPA_IOC_FILTERS, "RDPA_IOC_FILTERS is not supported");
            ret = RDPA_DRV_ERROR;
#else
            ret = rdpa_cmd_filter_ioctl(arg);
#endif
            break;
        }

        default:
        {
            BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "Invalid Command [%u]", command);
            ret = RDPA_DRV_ERROR;
        }
    }

    return ret;

} /* rdpa_cmdIoctl */

/*
 *------------------------------------------------------------------------------
 * Function Name: rdpa_cmdOpen
 * Description  : Called when an user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int rdpa_cmdOpen(struct inode *inode, struct file *filp)
{
    BCM_LOG_INFO(BCM_LOG_ID_RDPA_CMD_DRV, "OPEN RDPA Char Device");

    return RDPA_DRV_SUCCESS;
}

/* Global file ops */
static struct file_operations rdpa_cmdFops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = rdpa_cmdIoctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = rdpa_cmdIoctl,
#endif
    .open = rdpa_cmdOpen,
};

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

#define RDPA_IQ_SUPPORT_FIELD (RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD | \
                               RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD)

static int rdpa_iq_entry_add(void *iq_param)
{
    uint32_t key_mask = ((iq_param_t *)iq_param)->key_mask;
    iq_action_t *action = &((iq_param_t *)iq_param)->action;
    int rc;

    /* use ingress classification for the following fields and action = PRIO */
    if ((action->type == IQ_ACTION_TYPE_PRIO) &&
        ((key_mask & RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD) == key_mask))
        return rdpa_iq_ingress_class_add(iq_param);

    /* use filter for the following fields and action = TRAP */
    if ((action->type == IQ_ACTION_TYPE_TRAP) &&
        ((key_mask & RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD) == key_mask))
        return rdpa_iq_filter_add(iq_param);

    /* if drop, see if it can be added to filter first. or else add it to
     * ingress classificaiton */
    if (action->type == IQ_ACTION_TYPE_DROP)
    {
        if ((key_mask & RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD) == key_mask)
        {
            rc = rdpa_iq_filter_add(iq_param);
            if (rc == 0)
                return rc;
        }

        if ((key_mask & RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD) == key_mask)
            return rdpa_iq_ingress_class_add(iq_param);
    }

    return -EINVAL;
}

static int rdpa_iq_entry_rem(void *iq_param)
{
    uint32_t key_mask = ((iq_param_t *)iq_param)->key_mask;
    iq_action_t *action = &((iq_param_t *)iq_param)->action;
    int rc;

    /* use ingress classification for the following fields and action = PRIO */
    if ((action->type == IQ_ACTION_TYPE_PRIO) &&
        ((key_mask & RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD) == key_mask))
        return rdpa_iq_ingress_class_rem(iq_param);

    /* use filter for the following fields and action = TRAP */
    if ((action->type == IQ_ACTION_TYPE_TRAP) &&
        ((key_mask & RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD) == key_mask))
        return rdpa_iq_filter_rem(iq_param);

    /* if drop, see if it can be removed from filter first. or else remove it to
     * ingress classificaiton */
    if (action->type == IQ_ACTION_TYPE_DROP)
    {
        if ((key_mask & RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD) == key_mask)
        {
            rc = rdpa_iq_filter_rem(iq_param);
            if (rc == 0)
                return rc;
        }

        if ((key_mask & RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD) == key_mask)
            return rdpa_iq_ingress_class_rem(iq_param);
    }

    return -EINVAL;
}

static const iq_hw_info_t hw_info_db = {
    .mask_capability = RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD,
    .add_entry = rdpa_iq_entry_add,
    .delete_entry = rdpa_iq_entry_rem,
    .set_status = NULL,
    .get_status = rdpa_iq_cpu_get_status,
    .dump_table = NULL,
    .set_congestion_ctrl = rdpa_iq_sys_set_cong_ctrl,
    .get_congestion_ctrl = rdpa_iq_sys_get_cong_ctrl,
};
#endif

/*
 *------------------------------------------------------------------------------
 * Function Name: rdpa_cmd_drv_init
 * Description  : Initial function that is called at system startup that
 *                registers this device. See fapConfig.c
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
int __init rdpa_cmd_drv_init(void)
{
    int ret = 0;

    /* debugging only */
    bcmLog_setLogLevel(BCM_LOG_ID_RDPA_CMD_DRV, BCM_LOG_LEVEL_ERROR /* <-- Temporary / Need to be ---> BCM_LOG_LEVEL_ERROR*/);

    if((ret = register_chrdev(RDPADRV_MAJOR, RDPADRV_NAME, &rdpa_cmdFops)))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "Unable to get major number <%d>", RDPADRV_MAJOR);

        goto out;
    }

    BCM_LOG_INFO(BCM_LOG_ID_RDPA_CMD_DRV, RDPA_DRV_MODNAME " Char Driver " RDPA_DRV_VER_STR " Registered. Device: " RDPADRV_NAME " Ver:<%d>\n", RDPADRV_MAJOR);

    rdpa_cmd_class = class_create(THIS_MODULE, RDPADRV_NAME);
    if (IS_ERR(rdpa_cmd_class)) {
        BCM_LOG_ERROR(BCM_LOG_ID_RDPA_CMD_DRV, "Unable to class_create() for the device [%s]", RDPADRV_NAME);
        ret = PTR_ERR(rdpa_cmd_class);
        goto out_chrdev;
    }

    /* not a big deal if we fail here :-) */
    device_create(rdpa_cmd_class, NULL, MKDEV(RDPADRV_MAJOR, 0), NULL, RDPADRV_NAME);

    rdpa_cmd_tm_init();
#if defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP)
    rdpa_cmd_ic_init();
    rdpa_cmd_br_init();
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    bcm_iq_register_hw(&hw_info_db);
#endif

    BCM_LOG_INFO(BCM_LOG_ID_RDPA_CMD_DRV, "RDPA driver init: OK");

    return 0;

out_chrdev:
    unregister_chrdev(RDPADRV_MAJOR, RDPADRV_NAME);
out:
    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: rdpa_cmd_drv_exit
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void __exit rdpa_cmd_drv_exit(void)
{
    rdpa_cmd_tm_exit();
    unregister_chrdev(RDPADRV_MAJOR, RDPADRV_NAME);

    device_destroy(rdpa_cmd_class, MKDEV(RDPADRV_MAJOR, 0));
    class_destroy(rdpa_cmd_class);
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    bcm_iq_unregister_hw((iq_hw_info_t *)&hw_info_db);
#endif
    BCM_LOG_NOTICE(BCM_LOG_ID_RDPA_CMD_DRV, RDPA_DRV_MODNAME " Char Driver " RDPA_DRV_VER_STR
                   " Unregistered<%d>", RDPADRV_MAJOR);
}

module_init(rdpa_cmd_drv_init);
module_exit(rdpa_cmd_drv_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV(RDPADRV_MAJOR, 0);
MODULE_ALIAS("devname:rdpa_cmd");
