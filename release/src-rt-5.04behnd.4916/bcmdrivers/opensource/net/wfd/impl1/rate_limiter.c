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
bool wfd_rl_enabled = false;
bool wfd_sta_rl_enabled = false;

typedef struct _rate_limit_t {
    uint32_t rate;
    uint32_t burst;
    uint32_t last_ts;
    uint32_t tokens;
    uint32_t drops;
} rate_limit_t;

/* Rate limiter for downstream and upstream directions of every interface for every radio */
static rate_limit_t dev_rate_limit[WFD_MAX_OBJECTS][WIFI_MW_MAX_NUM_IF][2] = {};
static rate_limit_t dev_sta_rate_limit[WFD_MAX_OBJECTS][WIFI_MW_MAX_NUM_IF][2] = {};

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

static uint32_t wl_chain_drop(rate_limit_t *rl, pNBuff_t pNBuff)
{
    uint32_t drop_count = 0;
    pNBuff_t *next;

    next = bdmf_sysb_chain_next(pNBuff); 
    while (pNBuff) 
    {
        next = bdmf_sysb_chain_next(pNBuff);
        bdmf_sysb_free(pNBuff);
        rl->drops++;
        rl_trace("dropped %d packets\n", rl->drops);
        pNBuff = next;
        drop_count++;
    }

    return drop_count;
}

static uint32_t __rl_chain_check_and_drop(rate_limit_t *rl, pNBuff_t pNBuff)
{
    int is_chained;
    pNBuff_t *prev = NULL, *curr = pNBuff;
    uint32_t drop_count;

    if (!rl->rate)
        return 0;

    is_chained = bdmf_sysb_is_chained(pNBuff);
    if (__rl_should_drop(rl, bdmf_sysb_length(pNBuff)))
    {
        if (is_chained)
        {
            drop_count = wl_chain_drop(rl, pNBuff);
            rl->drops += drop_count - 1; /* -1 because rl_should_drop increases the counter */
            rl_trace("dropped %d packets\n", rl->drops);
        }
        else
            bdmf_sysb_free(pNBuff);

        /* all packets are dropped */
        return 1;
    }
    else if (!is_chained)
        return 0; /* single packet - can pass */

    /* Go through the chain and check every packet if it fits into the given rate */
    prev = pNBuff;
    curr = bdmf_sysb_chain_next(pNBuff);
    while (curr && !__rl_should_drop(rl, bdmf_sysb_length(curr)))
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
/* Test whether the pNBuffs chain fits into the given rate. The function can drop the whole chain or part of it (tail) 
 * Return value: 0 - some part of the chain (1 or more packets from the head) should be transmitted
 *               1 - all packets were dropped
 */
uint32_t rl_chain_check_and_drop(uint32_t wfd_id, uint32_t if_id, int dir, pNBuff_t pNBuff)
{
    rate_limit_t *rl = &dev_rate_limit[wfd_id][if_id][dir];
    
    return __rl_chain_check_and_drop(rl, pNBuff);
}

#ifdef BCM_PKTFWD
#include "bcm_pktfwd.h"

static void * rl_pktfwd_sta_cb_get_by_key(uint8_t unit, uint16_t key)
{
    d3lut_elem_t * d3lut_elem;

    d3lut_elem = d3lut_k2e(d3lut_gp, (pktfwd_key_t)key);
    if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
        return (d3lut_elem->ext.cb);
    } else {
        return NULL;
    }
}

static void * rl_pktfwd_sta_cb_get_by_mac(uint8_t unit, uint8_t *sta_mac)
{
    d3lut_elem_t * d3lut_elem;

    d3lut_elem = d3lut_lkup(d3lut_gp, sta_mac, unit);
    if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
       return  (d3lut_elem->ext.cb);
    } else {
        return NULL;
    }
}

static void rl_pktfwd_sta_rl_dump_per_ssid (uint8_t unit, uint8_t ssid)
{
    d3lut_elem_t *d3lut_elem;
    d3fwd_ext_t *ext;
    int i = 0, dir = 0;
               
    for (i = 0; i < D3LUT_ELEM_TOT; i++)
    {
        d3lut_elem = D3LUT_TABLE_ELEM(d3lut_gp->elem_base, i);
        if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
            ext = &d3lut_elem->ext;
            if (ext->inuse && (ext->ssid == ssid) && (d3lut_elem->key.domain == unit))
            {
                rate_limit_t *rl_cb = (rate_limit_t *)d3lut_elem->ext.cb;
                if (rl_cb != NULL)
                {
                    for (dir = 0; dir <= 1; dir++)
                    {
                        if (!rl_cb[dir].rate)
                            continue;
                            
                        printk("station : [%02x:%02x:%02x:%02x:%02x:%02x][wl%d][ssid%d][%s]:\n\trate: %u Mbps, burst: %u Mbps\n\tdrops: %u\n", 
                        d3lut_elem->sym.v8[0],d3lut_elem->sym.v8[1], d3lut_elem->sym.v8[2],d3lut_elem->sym.v8[3],d3lut_elem->sym.v8[4], d3lut_elem->sym.v8[5], 
                            unit, ssid, dir == 1 ? "RX" : "TX",
                        BYTES_TO_MBITS(rl_cb[dir].rate), BYTES_TO_MBITS(rl_cb[dir].burst), rl_cb[dir].drops);
                    }
                }
            }
        }
    }
}

static void rl_pktfwd_sta_rl_update_per_ssid (uint8_t unit, uint8_t ssid, rate_limit_t *rl)
{
    d3lut_elem_t *d3lut_elem;
    d3fwd_ext_t *ext;
    int i = 0;
               
    for (i = 0; i < D3LUT_ELEM_TOT; i++)
    {
        d3lut_elem = D3LUT_TABLE_ELEM(d3lut_gp->elem_base, i);
        if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
            ext = &d3lut_elem->ext;
            if (ext->inuse && (ext->ssid == ssid) && (d3lut_elem->key.domain == unit))
            {
                rate_limit_t *rl_cb = (rate_limit_t *)d3lut_elem->ext.cb;
                if (rl_cb != NULL)
                {
                    memcpy(&(rl_cb[0]), &(rl[0]), sizeof(rate_limit_t));
                    memcpy(&(rl_cb[1]), &(rl[1]), sizeof(rate_limit_t));
                }
            }
        }
    }
}

static void rl_pktfwd_sta_cb_get(d3lut_elem_t *d3lut_elem)
{
    /* only focus on WLAN domain */
    if ((d3lut_elem == NULL) || (d3lut_elem->key.domain == PKTFWD_XDOMAIN_IDX))
    {
        return;
    }
    
    if (d3lut_elem->ext.cb == NULL)
    {
        d3lut_elem->ext.cb = kmalloc(2 * sizeof(rate_limit_t), GFP_ATOMIC);
    }
}

static void rl_pktfwd_sta_cb_set(d3lut_elem_t *d3lut_elem)
{
    /* only focus on WLAN domain */
    if ((d3lut_elem == NULL) || (d3lut_elem->key.domain == PKTFWD_XDOMAIN_IDX))
    {
        return;
    }
    
    if (d3lut_elem->ext.cb != NULL)
    {
        rate_limit_t *rl_cb = (rate_limit_t *)d3lut_elem->ext.cb;
        int wl_unit = d3lut_elem->key.domain;
        int ssid = d3lut_elem->ext.ssid;
        
        /* 0 - tx, 1 - rx */
        memcpy(&rl_cb[0], &dev_sta_rate_limit[wl_unit][ssid][0], sizeof(rate_limit_t));
        memcpy(&rl_cb[1], &dev_sta_rate_limit[wl_unit][ssid][1], sizeof(rate_limit_t));
    }
}

static void rl_pktfwd_sta_cb_put(d3lut_elem_t *d3lut_elem)
{
    if ((d3lut_elem == NULL) || (d3lut_elem->ext.cb == NULL))
        return;

    /* just reset not free cb to avoid cb pointer exclusive protection */
    memset(d3lut_elem->ext.cb, 0, 2 * sizeof(rate_limit_t));
}

static void rl_pktfwd_sta_cb_free(d3lut_elem_t *d3lut_elem)
{
    if (d3lut_elem == NULL)
    {
        d3lut_elem_t *d3lut_elem;
        int i = 0;
                   
        for (i = 0; i < D3LUT_ELEM_TOT; i++)
        {
            d3lut_elem = D3LUT_TABLE_ELEM(d3lut_gp->elem_base, i);
            if ((d3lut_elem != NULL) && (d3lut_elem->ext.cb != NULL))
                kfree(d3lut_elem->ext.cb);
        }
        return;
    }
    else if (d3lut_elem->ext.cb != NULL)
    {
        kfree(d3lut_elem->ext.cb);
    }
}

static void rl_pktfwd_sta_cb_op(d3lut_elem_t *d3lut_elem, d3fwd_ext_cb_op_t op)
{
    switch (op)
    {
        case D3LUT_ELEM_EXT_CB_GET:
            rl_pktfwd_sta_cb_get(d3lut_elem);
            break;
        case D3LUT_ELEM_EXT_CB_SET:
            rl_pktfwd_sta_cb_set(d3lut_elem);
            break;
        case D3LUT_ELEM_EXT_CB_PUT:
            rl_pktfwd_sta_cb_put(d3lut_elem);
            break;
        case D3LUT_ELEM_EXT_CB_FREE:
            rl_pktfwd_sta_cb_free(d3lut_elem);
            break;
        default:
            break;
    }
}
#endif

uint32_t rl_sta_tx_should_drop(uint8_t wl_unit, uint16_t key, uint32_t size)
{
#ifdef BCM_PKTFWD
    rate_limit_t *sta_tx_rl = NULL;
    uint32_t *cb = NULL;

    cb = rl_pktfwd_sta_cb_get_by_key(wl_unit, key);
    if(cb == NULL){
        return 0;
    }
    sta_tx_rl = &((rate_limit_t*)cb)[0];

    return __rl_should_drop(sta_tx_rl, size);
#else
    return 0;
#endif
}

uint32_t rl_sta_rx_should_drop(uint8_t wl_unit, pNBuff_t pNBuff)
{
#ifdef BCM_PKTFWD
    rate_limit_t *sta_rx_rl = NULL;
    struct ethhdr *mh = NULL;
    struct sk_buff *skb = NULL;
    uint32_t *cb = NULL;

    if (!IS_SKBUFF_PTR(pNBuff)){
        printk("rl_per_sta_rx_should_drop rx is FKB!\r\n");
        return 0;
    }

    skb = (struct sk_buff *)pNBuff;
    skb_reset_mac_header(skb);
    mh = eth_hdr(skb);

    cb = rl_pktfwd_sta_cb_get_by_mac(wl_unit,mh->h_source);
    if(cb == NULL){
        return 0;
    }
    sta_rx_rl = &((rate_limit_t*)cb)[1];
    
    return __rl_chain_check_and_drop(sta_rx_rl, pNBuff);
#else
    return 0;
#endif
}

static ssize_t rate_limit_get_proc(struct file *file, char __user *buff, size_t len, loff_t *offset)
{
    uint32_t radio_id;
    char outbuf[1024];
    int sz = sizeof(outbuf);

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

                *offset += snprintf(outbuf + *offset, sz - *offset, "%s [%s]:\n\trate: %u Mbps, burst: %u Mbps\n\tdrops: %u\n", dev->name, 
                        dir == RL_DIR_RX ? "RX" : "TX",
                        BYTES_TO_MBITS(dev_rate_limit[radio_id][if_id][dir].rate), 
                        BYTES_TO_MBITS(dev_rate_limit[radio_id][if_id][dir].burst),
                        dev_rate_limit[radio_id][if_id][dir].drops);

                dev_rate_limit[radio_id][if_id][dir].drops = 0;
            }
            
            for (dir = 0; dir <= 1; dir++)
            {
                if (!dev_sta_rate_limit[radio_id][if_id][dir].rate)
                    continue;

                *offset += sprintf(outbuf + *offset, "%s [%s]: station RL:\n\trate: %u Mbps, burst: %u Mbps\n", dev->name, 
                        dir == RL_DIR_RX ? "RX" : "TX",
                BYTES_TO_MBITS(dev_sta_rate_limit[radio_id][if_id][dir].rate), 
                BYTES_TO_MBITS(dev_sta_rate_limit[radio_id][if_id][dir].burst));            
            }
#ifdef BCM_PKTFWD
            rl_pktfwd_sta_rl_dump_per_ssid(radio_id, if_id);
#endif
        }
    }
    
    *offset += sprintf(outbuf + *offset, "\nSSID rate limit function %s.\n", wfd_rl_enabled ? "enabled": "disabled");
    *offset += sprintf(outbuf + *offset, "STA  rate limit function %s.\n", wfd_sta_rl_enabled ? "enabled": "disabled");

    if (copy_to_user(buff, outbuf, *offset))
        return -EFAULT;

    return *offset;
}

static ssize_t rate_limit_set_proc(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[PROC_CMD_MAX_LEN];
    uint32_t rate, burst, radio_id, if_id, dir, type = 0;
    char name[32];
    int ret;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    if ((ret = sscanf(input, "%32s %u %u %u %u", name, &dir, &rate, &burst, &type)) < 4)
    {
        if (ret == 1)
        {
            if (!strcmp(name, "rl_disable"))
            {
                wfd_rl_enabled = false;
            }
            else if (!strcmp(name, "rl_enable"))
            {
                wfd_rl_enabled = true;
            }
            else if (!strcmp(name, "sta_rl_disable"))
            {
                wfd_sta_rl_enabled = false;
            }
            else if (!strcmp(name, "sta_rl_enable"))
            {
                wfd_sta_rl_enabled = true;
            }
            else
            {
                 printk("\nUnsupported command\n");
            }
            return len;
        }
        else if (ret == 2 && !strcmp(name, "tr"))
        {
            dbg_trace_count = dbg_trace_rate = dir;
            printk("Debug trace rate set to %u\n", dbg_trace_rate);
            return len;
        }
        else
        {
            goto Usage;
        }
    }

    if (!wfd_dev_by_name_get(name, &radio_id, &if_id))
    {
        printk("Couldn't find netdev \'%s\'", name);
        return len;
    }

    if (rate > 10000 || burst > 10000 || rate > burst || dir > 1 || type > 1){
        goto Usage;
    }

    if (type == 0)
    {
        wfd_rl_enabled = true;
        dev_rate_limit[radio_id][if_id][dir].rate = MBITS_TO_BYTES(rate);
        dev_rate_limit[radio_id][if_id][dir].tokens = dev_rate_limit[radio_id][if_id][dir].burst = MBITS_TO_BYTES(burst);
        dev_rate_limit[radio_id][if_id][dir].last_ts = 0;
        printk("\n%s (radio %d, if %d) [%s]: set rate/burst %u Mbits/%u Mbits\n", name, radio_id, if_id, dir == RL_DIR_RX ? "RX" : "TX", rate, burst);
    }
    else if (type == 1)
    {
        wfd_sta_rl_enabled = true;
        dev_sta_rate_limit[radio_id][if_id][dir].rate = MBITS_TO_BYTES(rate);
        dev_sta_rate_limit[radio_id][if_id][dir].tokens = dev_sta_rate_limit[radio_id][if_id][dir].burst = MBITS_TO_BYTES(burst);
        dev_sta_rate_limit[radio_id][if_id][dir].last_ts = 0;
        
#ifdef BCM_PKTFWD
        // set online station RL
        rl_pktfwd_sta_rl_update_per_ssid(radio_id, if_id, dev_sta_rate_limit[radio_id][if_id]);
#endif
        printk("\n%s (radio %d, if %d, station RL) [%s]: set rate/burst %u Mbits/%u Mbits\n", name, radio_id, if_id, dir == RL_DIR_RX ? "RX" : "TX", rate, burst);
    }
    else
    {
        goto Usage;
    }
    goto Exit;

Usage:
    printk("\nUsage: <dev name> <0(TX)/1(RX)> <rate Mbit/s> <burst Mbit>  [0(ssid RL)/1(station RL)]\nRate/burst acceptable range: 1 - 10000 Mbit/s, burst >= rate \n");
    
Exit:
    return len;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))  
static const struct file_operations rate_limit_fops = {
    .read  = rate_limit_get_proc,
    .write = rate_limit_set_proc,
};
#else
static const struct proc_ops rate_limit_fops = {
    .proc_read  = rate_limit_get_proc,
    .proc_write = rate_limit_set_proc,
};
#endif

void rate_limiter_init(void)
{
    rl_file_conf = proc_create("wfd/rate_limit", 0644, NULL, &rate_limit_fops);
#ifdef BCM_PKTFWD
    d3lut_elem_ext_cb_op = rl_pktfwd_sta_cb_op;
#endif
}