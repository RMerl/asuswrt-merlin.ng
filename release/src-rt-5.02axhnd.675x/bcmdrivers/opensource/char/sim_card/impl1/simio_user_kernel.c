/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

#include <bcmtypes.h>
#include <linux/types.h>

#include "bcm_OS_Deps.h"

#include <linux/bcm_log.h>
#include "chal_types.h"
#include "chal_simio.h"

#include <simio_def_common.h>
#include "simio.h"
#include "simio_board.h"

#include "sim_export_defs.h"

#include "shared_utils.h"
#include <simio_user_kernel.h>

#define SIM_MAJOR 202
#define SIM_NAME "bcm_sim_card"

//#define CC_SIM_IOCLT_DEBUG

#ifdef CC_SIM_IOCLT_DEBUG
#define simcard_debug(fmt, arg...) {printk(">>> %s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg); fflush(stdout);}
#define simcard_error(fmt, arg...) {printk("ERROR[%s.%u]: " fmt, __FUNCTION__, __LINE__, ##arg); fflush(stdout);}
#else
#define simcard_debug(fmt, arg...)
#define simcard_error(fmt, arg...)
#endif


static struct class *simio_cl = NULL;

static int simio_user_kernel_open(struct inode *i, struct file *f)
{
    return 0;
}

static int simio_user_kernel_close(struct inode *i, struct file *f)
{
    return 0;
}

static long simio_user_kernel_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    long os_ret = -EACCES;
    simio_ioctl_arg_t param;

    if (copy_from_user(&param, (simio_ioctl_arg_t*)arg, sizeof(simio_ioctl_arg_t)))
        goto Error;

    switch (cmd)
    {
    case SIMIO_ACTIVATE:
        param.ret = sim_active(param.sim_id, param.data.active.data, &param.data.active.len);
        simcard_debug("%s: SIMIO_ACTIVATE ret %d\n", __FUNCTION__, param.ret);
        break;
   
    case SIMIO_IS_ONLINE:   
        param.ret = 0;
        param.data.detection_status = sim_online(param.sim_id);
        simcard_debug("%s: %d SIMIO_IS_ONLINE: detection_status %d\n", __FUNCTION__,
            param.sim_id, param.data.detection_status);
        break;

    case SIMIO_SET_BAUDRATE:
        param.ret = sim_set_baud(param.sim_id, param.data.baud_rate.F, param.data.baud_rate.D);
        simcard_debug("%s: %d SIMIO_SET_BAUDRATE: F %u D %u ret %d\n", __FUNCTION__,
            param.sim_id, param.data.baud_rate.F, param.data.baud_rate.D, param.ret);
        break;

    case SIMIO_SET_PROTOCOL:
        param.ret = sim_card_protocol(param.sim_id, param.data.protocol);
        simcard_debug("%s: %d SIMIO_SET_PROTOCOL: protocol %u ret %d\n", __FUNCTION__,
            param.sim_id, param.data.protocol, param.ret);
        break;

    case SIMIO_SET_CONTROL:
        param.ret = sim_card_control(param.sim_id, param.data.control);
        simcard_debug("%s: %d SIMIO_SET_CONTROL: protocol %u ret %d\n", __FUNCTION__, 
            param. sim_id, param.data.control, param.ret);
        break;

    case SIMIO_RESET:
        param.ret = sim_card_reset(param.sim_id, param.data.reset.reset,
            param.data.reset.voltage, param.data.reset.freq);
        simcard_debug("%s: %d SIMIO_RESET: reset %u voltage %u freq %u ret %d\n", __FUNCTION__, 
            param.sim_id, param.data.reset.reset, param.data.reset.voltage,
            param.data.reset.freq, param.ret);
        break;

    case SIMIO_WRITE:
        param.data.io.rx_len = sim_write(param.sim_id, param.data.io.data,
            SIM_CARD_MAX_BUFFER_SIZE);
        if (param.data.io.rx_len < 0)
        {
            param.ret = param.data.io.rx_len;
            param.data.io.rx_len = 0;
        }

        simcard_debug("%s: %d SIMIO_WRITE: tx_len %u rx_len %u ret %d\n", __FUNCTION__, 
            param.sim_id, param.data.io.tx_len, param.data.io.rx_len, param.ret);
        break;

    case SIMIO_READ:   
        param.data.io.rx_len = sim_read(param.sim_id, param.data.io.data, SIM_CARD_MAX_BUFFER_SIZE);
        if (param.data.io.rx_len < 0)
        {
            param.ret = param.data.io.rx_len;
            param.data.io.rx_len = 0;
        }

        simcard_debug("%s: %d SIMIO_READ: tx_len %u rx_len %u ret %d\n", __FUNCTION__, 
            param.sim_id, param.data.io.tx_len, param.data.io.rx_len, param.ret);
        break;

    default:
        os_ret = -EINVAL;
        goto Error;
    }

    os_ret = 0;

    if (copy_to_user((simio_ioctl_arg_t *)arg, &param, sizeof(simio_ioctl_arg_t)))
        os_ret = -EACCES;

Error:

    simcard_error("%s: Done os_ret %ld\n", __FUNCTION__, os_ret);
    return os_ret;
}

void simio_callback(SIMIO_SIGNAL_t simio_sig, UInt16 rsp_len, UInt8 *rsp_data)
{
    switch (simio_sig){
    case SIMIO_SIGNAL_ATRCORRUPTED:
        simcard_debug("user: atr corrupted\n");
        break;
    case SIMIO_SIGNAL_ATR_WRONG_VOLTAGE:
        simcard_debug("user: wrong voltage applied\n");
        break;
    case SIMIO_SIGNAL_SIMINSERT:
        simcard_debug("user: insert\n");
        break;
    case SIMIO_SIGNAL_RSPDATA:
        break;
    case SIMIO_SIGNAL_SIMREMOVED:
        simcard_debug("user: removed\n");
        break;
    case SIMIO_SIGNAL_TIMEOUT:
    case SIMIO_SIGNAL_IDLE:
    case SIMIO_SIGNAL_T1_PARITY:
    case SIMIO_SIGNAL_T1_INVALID_LENGTH:
    case SIMIO_SIGNAL_T1_BWT_TIME_OUT:
    case SIMIO_SIGNAL_SIMRESET:
    default:
        break;
    }
}

void simio_detect_callback(Boolean insert)
{
    if (insert == 0)
        simcard_debug("in user: card out\n");
    else
        simcard_debug("in user: card in\n");
}

static struct file_operations simio_user_kernel_fops =
{
    .owner = THIS_MODULE,
    .open = simio_user_kernel_open,
    .release = simio_user_kernel_close,
    .unlocked_ioctl = simio_user_kernel_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = simio_user_kernel_ioctl
#endif
};

int simio_user_kernel_register(void)
{
    int ret;

    if ((ret = register_chrdev(SIM_MAJOR, SIM_NAME, &simio_user_kernel_fops)) < 0)
    {
        return ret;
    }

    if (IS_ERR(simio_cl = class_create(THIS_MODULE, SIM_NAME)))
    {
        unregister_chrdev(SIM_MAJOR, SIM_NAME);
        return PTR_ERR(simio_cl);
    }

    device_create(simio_cl, NULL, MKDEV(SIM_MAJOR, 0), NULL, SIM_NAME);

    SIMIO_RegisterCB(SIMIO_ID_0, simio_callback, NULL);
    SIMIO_RegisterDetectionCB(SIMIO_ID_0, simio_detect_callback);
    return 0;
}

void simio_user_kernel_unregister(void)
{

    SIMIO_RegisterCB(SIMIO_ID_0, NULL, NULL);
    SIMIO_RegisterDetectionCB(SIMIO_ID_0, NULL);
    
    unregister_chrdev(SIM_MAJOR, SIM_NAME);
    device_destroy(simio_cl, MKDEV(SIM_MAJOR, 0));
    class_destroy(simio_cl);
}

