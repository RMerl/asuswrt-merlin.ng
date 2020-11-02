/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include "bdmf_system.h"
#include "bdmf_sysb_chain.h"
#include "wfd_dev_priv.h"
#include "rate_limiter.h"

static struct proc_dir_entry *rl_file_conf;

typedef struct _rate_limit_t {
    uint32_t rate;
    uint32_t burst;
    uint32_t last_ts;
    uint32_t tokens;
    uint32_t drops;
} rate_limit_t;

/* Rate limiter for downstream and upstream directions of every interface for every radio */
static rate_limit_t dev_rate_limit[WFD_MAX_OBJECTS][WIFI_MW_MAX_NUM_IF][2] = {};

#define PROC_CMD_MAX_LEN 64
#define KBITS_TO_BYTES(mbits) (mbits*125)
#define BYTES_TO_KBITS(bytes) (bytes/125)
#define MBITS_TO_BYTES(mbits) (mbits*125000)
#define BYTES_TO_MBITS(bytes) (bytes/125000)

static uint32_t dbg_trace_rate = 0;
static uint32_t dbg_trace_count = 0;
#define rl_trace(fmt, params...) do { if (dbg_trace_rate && !--dbg_trace_count)  \
    { printk(fmt, params); dbg_trace_count = dbg_trace_rate; } \
  } while (0)

static ssize_t rate_limit_get_proc(struct file *file, char *buff, size_t len, loff_t *offset)
{
    uint32_t radio_id;

    if (*offset)
        return 0;

    for (radio_id = 0; radio_id < WFD_MAX_OBJECTS; radio_id++)
    {
        uint32_t if_id;

        if (!wfd_is_radio_valid(radio_id))
            continue;

        for (if_id = 0; if_id < WIFI_MW_MAX_NUM_IF; if_id++)
        {
            uint32_t dir;
            struct net_device *dev;

            if (!(dev = wfd_dev_by_id_get(radio_id, if_id)))
                continue;

            for (dir = 0; dir <= 1; dir++)
            {
                if (!dev_rate_limit[radio_id][if_id][dir].rate)
                    continue;

                *offset += sprintf(buff + *offset, "%s [%s]:\n\trate: %u Mbps, burst: %u Mbps\n\tdrops: %u\n", dev->name, 
                        dir == RL_DIR_RX ? "RX" : "TX",
                        BYTES_TO_MBITS(dev_rate_limit[radio_id][if_id][dir].rate), 
                        BYTES_TO_MBITS(dev_rate_limit[radio_id][if_id][dir].burst),
                        dev_rate_limit[radio_id][if_id][dir].drops);

                dev_rate_limit[radio_id][if_id][dir].drops = 0;
            }
        }
    }

    return *offset;
}

static ssize_t rate_limit_set_proc(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[PROC_CMD_MAX_LEN];
    uint32_t rate, burst, radio_id, if_id, dir;
    char dev_name[32];
    int ret;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    ret = sscanf(input, "%32s %u %u %u", dev_name, &dir, &rate, &burst);

    if (ret == 2 && !strcmp(dev_name, "tr"))
    {
        dbg_trace_count = dbg_trace_rate = dir;
        printk("Debug trace rate set to %u\n", dbg_trace_rate);
        return len;
    }

    if ((ret = sscanf(input, "%32s %u %u %u", dev_name, &dir, &rate, &burst)) < 4)
        goto Usage;

    if (!wfd_dev_by_name_get(dev_name, &radio_id, &if_id))
    {
        printk("Couldn't find netdev \'%s\'", dev_name);
        return len;
    }

    if (rate > 10000 || burst > 10000 || rate > burst || dir > 1)
        goto Usage;

    dev_rate_limit[radio_id][if_id][dir].rate = MBITS_TO_BYTES(rate);
    dev_rate_limit[radio_id][if_id][dir].tokens = dev_rate_limit[radio_id][if_id][dir].burst = MBITS_TO_BYTES(burst);
    dev_rate_limit[radio_id][if_id][dir].last_ts = 0;

    printk("%s (radio %d, if %d) [%s]: set rate/burst %u Mbits/%u Mbits\n", dev_name, radio_id, if_id, dir == RL_DIR_RX ? "RX" : "TX", rate, burst);
    goto Exit;

Usage:
    printk("\nUsage: <dev name> <0(TX)/1(RX)> <rate Kbit/s> <burst Kbit>\nAcceptable range: 1 - 10000 Kbit/s, burst >= rate\n");
Exit:
    return len;
}

static uint32_t __rl_should_drop(rate_limit_t *rl, uint32_t size)
{
    uint32_t diff_ts;

    if (!rl || !rl->rate)
        return 0;

    if (!rl->last_ts)
    {
        /* Initialize */
        rl->last_ts = jiffies;
        return 0;
    }

    if ((diff_ts = jiffies - rl->last_ts))
    {
        if (diff_ts < HZ)
        {
            rl_trace("diff_ts %u; size %u; tokens %u -> %u\n", diff_ts, size, rl->tokens, min((rl->tokens + (rl->rate*diff_ts)/HZ), 
                        rl->burst));
            rl->tokens = min((rl->tokens + (rl->rate*diff_ts)/HZ), rl->burst);
        }
        else
        {
            rl_trace("diff_ts %u; size %u; tokens %u -> %u\n", diff_ts, size, rl->tokens, rl->burst);
            rl->tokens = rl->burst;
        }

        rl->last_ts = jiffies;
    }

    if (!rl->tokens || size > rl->tokens)
    {
        rl->drops++;
        rl_trace("dropped %d packets\n", rl->drops);
        return 1;
    }

    rl->tokens -= size;

    return 0;
}

uint32_t rl_should_drop(uint32_t wfd_id, uint32_t if_id, int dir, uint32_t size)
{
    return __rl_should_drop(&dev_rate_limit[wfd_id][if_id][dir], size);
}

static uint32_t wl_chain_drop(rate_limit_t *rl, struct sk_buff *skb)
{
    uint32_t drop_count = 0;
    struct sk_buff *next;

    next = bdmf_sysb_chain_next(skb); 
    while (skb) 
    {
        next = bdmf_sysb_chain_next(skb);
        bdmf_sysb_free(skb);
        rl->drops++;
        rl_trace("dropped %d packets\n", rl->drops);
        skb = next;
        drop_count++;
    }

    return drop_count;
}

/* Test whether the skbs chain fits into the given rate. The function can drop the whole chain or part of it (tail) 
 * Return value: 0 - some part of the chain (1 or more packets from the head) should be transmitted
 *               1 - all packets were dropped
 */
uint32_t rl_chain_check_and_drop(uint32_t wfd_id, uint32_t if_id, int dir, struct sk_buff *skb)
{
    int is_chained;
    struct sk_buff *prev = NULL, *curr = skb;
    rate_limit_t *rl = &dev_rate_limit[wfd_id][if_id][dir];
    uint32_t drop_count;

    if (!rl->rate)
        return 0;

    is_chained = bdmf_sysb_is_chained(skb);
    if (__rl_should_drop(rl, skb->len))
    {
        if (is_chained)
        {
            drop_count = wl_chain_drop(rl, skb);
            rl->drops += drop_count - 1; /* -1 because rl_should_drop increases the counter */
            rl_trace("dropped %d packets\n", rl->drops);
        }
        else
            bdmf_sysb_free(skb);

        /* all packets are dropped */
        return 1;
    }
    else if (!is_chained)
        return 0; /* single packet - can pass */

    /* Go through the chain and check every packet if it fits into the given rate */
    prev = skb;
    curr = bdmf_sysb_chain_next(skb);
    while (curr && !__rl_should_drop(rl, curr->len))
    {
        prev = curr;
        curr = bdmf_sysb_chain_next(curr);
    }
    if (!curr)
        return 0; /* all tested packets can pass */

    /* drop the rest of the chain */
    bdmf_sysb_chain_link_set(prev, NULL);
    drop_count = wl_chain_drop(rl, curr);
    rl->drops += drop_count - 1; /* -1 because rl_should_drop increases the counter */
    rl_trace("dropped %d packets\n", rl->drops);

    /* some packets from the head can pass */
    return 0;
}

static const struct file_operations rate_limit_fops = {
    .read  = rate_limit_get_proc,
    .write = rate_limit_set_proc,
};

void rate_limiter_init(void)
{
    rl_file_conf = proc_create("wfd/rate_limit", 0644, NULL, &rate_limit_fops);
}

