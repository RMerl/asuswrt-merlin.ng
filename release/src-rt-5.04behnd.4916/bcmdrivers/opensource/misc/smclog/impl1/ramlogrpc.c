/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 Broadcom Ltd.
 */
/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

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

/****************************************************************************
 *
 * Filename:    ramlogrpc.c
 *
 ****************************************************************************
 *
 * Description: ramlog RPC definitions
 *
 ****************************************************************************/

#include "ramlogrpc.h"

#include <linux/kernel.h>
#include <asm/byteorder.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/ktime.h>
#include <linux/dma-mapping.h>
#include <asm/div64.h>
#include "bcmcache.h"
#include <itc_rpc.h>
#include <itc_msg_defs.h>
#include <itc_channel_defs.h>

enum
{
    RPC_FUNCTION_GET_ENCRYPTED_RAMLOG = 9,
    RPC_FUNCTION_ADD_TO_RAMLOG,
    RPC_FUNCTION_CLEAR_RAMLOG,
    RPC_FUNCTION_SET_RAMLOG_LEVEL,
    RPC_FUNCTION_SET_RAMLOG_UART_FLAG,
    RPC_FUNCTION_GET_RAMLOG_COUNT,
    RPC_FUNCTION_GET_RAMLOG_LAST_UPDATE
};

#define	RAMLOG_DQM_TIMEOUT			10000	/* milliseconds */

int register_ramlog_rpc(struct ramlog_rpc_data *ramlog, struct device *dev,
    const char *tunnel_name)
{
    if (!ramlog)
        return -EINVAL;

    ramlog->tunnel_name[0] = '\0';
    if (tunnel_name && *tunnel_name) {
        strncpy(ramlog->tunnel_name, tunnel_name,
            sizeof(ramlog->tunnel_name));
        ramlog->tunnel_name[sizeof(ramlog->tunnel_name)-1] = '\0';
    }
    ramlog->tunnel = rpc_get_fifo_tunnel_id((char *) ramlog->tunnel_name);
    if (ramlog->tunnel < 0) {
        dev_err(dev, "%s: Unable to obtain RPC tunnel ID.\n",
            __func__);
        return -EIO;
    }
    ramlog->dev = dev;
    sema_init(&ramlog->sem, 1);
    return 0;
}

void release_ramlog_rpc(struct ramlog_rpc_data *ramlog)
{
    if (!ramlog)
        return;
    memset(ramlog, 0, sizeof(struct ramlog_rpc_data));
}

int register_ramlog_rpc_from_platform_device(struct ramlog_rpc_data *ramlog,
    struct platform_device *pdev)
{
    struct device_node *phan_node;
    struct device_node *of_node = pdev->dev.of_node;
    const char *dev_name;

    phan_node = of_parse_phandle(of_node, "rpc-channel", 0);
    if (!phan_node) {
        dev_err(&pdev->dev, "Unable to retrieve rpc-channel phandle ");
        return -EINVAL;
    }

    if (of_property_read_string(phan_node, "dev-name", &dev_name)) {
        dev_err(&pdev->dev, "%s: Missing dev-name property!\n",
            of_node_full_name(of_node));
        of_node_put(phan_node);
        return -EINVAL;
    }
    of_node_put(phan_node);

    return register_ramlog_rpc(ramlog, &pdev->dev, dev_name);
}

uint32_t get_encrypted_ramlog_entries(dma_addr_t encrypted_dma_addr,
    uint32_t maxbufsize, struct ramlog_rpc_data *ramlog,
    uint32_t *count_ptr, uint32_t *update_ptr,
    uint32_t start_index)
{
    rpc_msg msg;
    int status;

    if (!ramlog)
        return -EINVAL;
#ifdef	SMC_RAMLOG_VERBOSE
    dev_info(ramlog->dev, "buffer=%#lx size=%u (%#x) index=%#x\n",
        (unsigned long) encrypted_dma_addr,
        (unsigned int) maxbufsize, (unsigned int) maxbufsize,
        (unsigned int) start_index);
#endif	/* SMC_RAMLOG_VERBOSE */
    /* alignment to 8 bytes needed to acvoid lock up of SMC IOP DMA rev. A0 */
    maxbufsize = RAMLOG_ALIGNED(maxbufsize);
    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS,
        RPC_FUNCTION_GET_ENCRYPTED_RAMLOG, 0,
        (uint32_t) (((unsigned long) encrypted_dma_addr) &
        0xffffffff),
        ((maxbufsize & 0xffffff) |
        (uint32_t) ((((uint64_t) encrypted_dma_addr) >> 8) &
        0xff000000)),
        (start_index & 0xffffff));

    barrier();
    if (maxbufsize)
        dma_sync_single_for_device(ramlog->dev, encrypted_dma_addr,
            maxbufsize, DMA_BIDIRECTIONAL);
    status = rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
    if (status < 0)
        return status;
    start_index = (msg.data[0] & 0xffffff);
    if ((start_index & 0x800000) != 0)
        start_index |= 0xff000000;
    if (count_ptr)
        *count_ptr = (msg.data[1] & 0xffffff);
    if (update_ptr)
        *update_ptr = msg.data[2];
    if (maxbufsize)
        dma_sync_single_for_cpu(ramlog->dev, encrypted_dma_addr,
            maxbufsize, DMA_BIDIRECTIONAL);
    return start_index;
}

int clear_ramlog(struct ramlog_rpc_data *ramlog)
{
    rpc_msg msg;

    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS,
        RPC_FUNCTION_CLEAR_RAMLOG, 0, 0, 0, 0);
    return rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
}

int add_to_ramlog(struct ramlog_rpc_data *ramlog, unsigned int severity,
    unsigned int length, dma_addr_t message_dma_addr)
{
    rpc_msg msg;
    int status;

    if (!ramlog)
        return -EINVAL;
    memset(&msg, 0, sizeof(msg));
    /* alignment to 8 bytes needed to acvoid lock up of SMC IOP DMA rev. A0 */
    length = RAMLOG_ALIGNED(length);

    rpc_msg_init(&msg, RPC_SERVICE_SYS,
        RPC_FUNCTION_ADD_TO_RAMLOG, 0,
        (uint32_t) (((unsigned long) message_dma_addr) &
        0xffffffff),
        ((length & 0xffffff) |
        (uint32_t) ((((uint64_t) message_dma_addr) >> 8) &
        0xff000000)),
        (uint32_t) (severity & 0xffff));

    barrier();
    if (length)
        dma_sync_single_for_device(ramlog->dev, message_dma_addr,
            length, DMA_BIDIRECTIONAL);
    status = rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
    if (status < 0)
        return status;
    status = (int) ((msg.data[0] & 0xff000000) >> 24);
    if (status > 0)
        status = -status;
    if (length)
        dma_sync_single_for_cpu(ramlog->dev, message_dma_addr,
            length, DMA_BIDIRECTIONAL);
    return status;
}

int set_ramlog_level(struct ramlog_rpc_data *ramlog, unsigned int level,
    unsigned int length, dma_addr_t source_dma_addr)
{
    rpc_msg msg;
    int status;

    if (!ramlog)
        return -EINVAL;
    memset(&msg, 0, sizeof(msg));
    /* alignment to 8 bytes needed to acvoid lock up of SMC IOP DMA rev. A0 */
    length = RAMLOG_ALIGNED(length);
    if (length > 0)
        rpc_msg_init(&msg, RPC_SERVICE_SYS,
            RPC_FUNCTION_SET_RAMLOG_LEVEL, 0,
            (uint32_t) (((unsigned long) source_dma_addr) &
            0xffffffff),
            ((length & 0xffffff) |
            (uint32_t) ((((uint64_t) source_dma_addr)
            >> 8) & 0xff000000)),
            (uint32_t) (level & 0xffff));
    else
        rpc_msg_init(&msg, RPC_SERVICE_SYS,
            RPC_FUNCTION_SET_RAMLOG_LEVEL, 0, 0, 0,
            (uint32_t) (level & 0xffff));

    barrier();
    if (length)
        dma_sync_single_for_device(ramlog->dev, source_dma_addr,
            length, DMA_BIDIRECTIONAL);
    status = rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
    if (status < 0)
        return status;
    status = (int) ((msg.data[0] & 0xff000000) >> 24);
    if (status > 0)
        status = -status;
    if (length)
        dma_sync_single_for_cpu(ramlog->dev, source_dma_addr,
            length, DMA_BIDIRECTIONAL);
    return status;
}

int set_ramlog_uart_flag(struct ramlog_rpc_data *ramlog, unsigned int flag)
{
    rpc_msg msg;
    int status;

    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS,
        RPC_FUNCTION_SET_RAMLOG_UART_FLAG, 0,
        (uint32_t) (flag ? 1 : 0), 0, 0);
    status = rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
    if (status < 0)
        return status;
    status = (int) ((msg.data[0] & 0xff000000) >> 24);
    if (status > 0)
        status = -status;
    return status;
}

unsigned int get_ramlog_count(struct ramlog_rpc_data *ramlog)
{
    rpc_msg msg;
    int status;

    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS,
        RPC_FUNCTION_GET_RAMLOG_COUNT, 0, 0, 0, 0);
    status = rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
    if (status < 0)
        return 0;
    return (unsigned int) (msg.data[0] & 0xffffff);
}

uint32_t get_ramlog_last_update(struct ramlog_rpc_data *ramlog)
{
    rpc_msg msg;
    int status;

    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS,
        RPC_FUNCTION_GET_RAMLOG_LAST_UPDATE, 0, 0, 0, 0);
    status = rpc_send_request_timeout(ramlog->tunnel, &msg,
        RAMLOG_DQM_TIMEOUT / 1000);
    if (status < 0)
        return 0;
    return msg.data[1];
}
