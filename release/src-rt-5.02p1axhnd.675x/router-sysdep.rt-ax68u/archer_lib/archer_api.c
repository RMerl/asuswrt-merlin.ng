/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2019:proprietary:standard

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
*
************************************************************************/

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <net/if.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "archer_api.h"

//#define CC_ARCHER_API_DEBUG

#if defined(CC_ARCHER_API_DEBUG)
#define archer_api_debug(fmt, arg...) printf("%s,%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
#define archer_api_debug(fmt, arg...)
#endif

#define archer_api_error(fmt, arg...) fprintf(stderr, "ERROR[%s,%u]: " fmt, __FUNCTION__, __LINE__, ##arg)

int archer_cmd_send(archer_ioctl_cmd_t cmd, unsigned long arg)
{
    int ret;
    int fd;

    fd = open(ARCHER_DRV_DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        archer_api_error("%s: %s", ARCHER_DRV_DEVICE_NAME, strerror(errno));

        return -EINVAL;
    }

    ret = ioctl(fd, cmd, arg);
    if(ret)
    {
        archer_api_error("ioctl: %s\n", strerror(errno));
    }

    close(fd);

    return ret;
}

int archer_sysport_tm_enable(void)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_ENABLE;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_disable(void)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_DISABLE;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_stats(void)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_STATS;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_stats_get(const char *if_name, int queue_index,
                                uint32_t *txPackets_p, uint32_t *txBytes_p,
                                uint32_t *droppedPackets_p, uint32_t *droppedBytes_p)
{
    sysport_tm_arg_t tm_arg;
    int ret;

    tm_arg.cmd = SYSPORT_TM_CMD_STATS_GET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);
    tm_arg.queue_index = queue_index;

    ret = archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
    if(ret)
    {
        archer_api_error("Could not archer_cmd_send");

        return ret;
    }

    *txPackets_p = tm_arg.stats.txPackets;
    *txBytes_p = tm_arg.stats.txBytes;
    *droppedPackets_p = tm_arg.stats.droppedPackets;
    *droppedBytes_p = tm_arg.stats.droppedBytes;

    return 0;
}
int archer_sysport_tm_queue_set(const char *if_name, int queue_index,
                                int min_kbps, int min_mbs,
                                int max_kbps, int max_mbs)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_QUEUE_SET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);
    tm_arg.queue_index = queue_index;
    tm_arg.min_kbps = min_kbps;
    tm_arg.min_mbs = min_mbs;
    tm_arg.max_kbps = max_kbps;
    tm_arg.max_mbs = max_mbs;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_queue_get(const char *if_name, int queue_index,
                                int *min_kbps_p, int *min_mbs_p,
                                int *max_kbps_p, int *max_mbs_p)
{
    sysport_tm_arg_t tm_arg;
    int ret;

    tm_arg.cmd = SYSPORT_TM_CMD_QUEUE_GET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);
    tm_arg.queue_index = queue_index;

    ret = archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
    if(ret)
    {
        archer_api_error("Could not archer_cmd_send");

        return ret;
    }

    *min_kbps_p = tm_arg.min_kbps;
    *min_mbs_p = tm_arg.min_mbs;
    *max_kbps_p = tm_arg.max_kbps;
    *max_mbs_p = tm_arg.max_mbs;

    return 0;
}

int archer_sysport_tm_port_set(const char *if_name, int kbps, int mbs)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_PORT_SET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);
    tm_arg.min_kbps = kbps;
    tm_arg.min_mbs = mbs;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_port_get(const char *if_name, int *kbps_p, int *mbs_p)
{
    sysport_tm_arg_t tm_arg;
    int ret;

    tm_arg.cmd = SYSPORT_TM_CMD_PORT_GET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);

    ret = archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
    if(ret)
    {
        archer_api_error("Could not archer_cmd_send");

        return ret;
    }

    *kbps_p = tm_arg.min_kbps;
    *mbs_p = tm_arg.min_mbs;

    return 0;
}

int archer_sysport_tm_arbiter_set(const char *if_name, sysport_tm_arbiter_t arbiter)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_ARBITER_SET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);
    tm_arg.arbiter = arbiter;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_arbiter_get(const char *if_name, sysport_tm_arbiter_t *arbiter_p)
{
    sysport_tm_arg_t tm_arg;
    int ret;

    tm_arg.cmd = SYSPORT_TM_CMD_ARBITER_GET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);

    ret = archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
    if(ret)
    {
        archer_api_error("Could not archer_cmd_send");

        return ret;
    }

    *arbiter_p = tm_arg.arbiter;

    return 0;
}

int archer_sysport_tm_mode_set(const char *if_name, sysport_tm_mode_t mode)
{
    sysport_tm_arg_t tm_arg;

    tm_arg.cmd = SYSPORT_TM_CMD_MODE_SET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);
    tm_arg.mode = mode;

    return archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
}

int archer_sysport_tm_mode_get(const char *if_name, sysport_tm_mode_t *mode_p)
{
    sysport_tm_arg_t tm_arg;
    int ret;

    tm_arg.cmd = SYSPORT_TM_CMD_MODE_GET;
    strncpy(tm_arg.if_name, if_name, ARCHER_IFNAMSIZ);

    ret = archer_cmd_send(ARCHER_IOC_SYSPORT_TM, (unsigned long)&tm_arg);
    if(ret)
    {
        archer_api_error("Could not archer_cmd_send");

        return ret;
    }

    *mode_p = tm_arg.mode;

    return 0;
}
