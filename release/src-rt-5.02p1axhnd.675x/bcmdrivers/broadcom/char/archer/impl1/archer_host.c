/*
  <:copyright-BRCM:2017:proprietary:standard

  Copyright (c) 2017 Broadcom 
  All Rights Reserved

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
*/

/*
*******************************************************************************
*
* File Name  : archer_host.c
*
* Description: Management of Host MAC Addresses
*
*******************************************************************************
*/

#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#if defined(CONFIG_IPV6)
#include <net/ipv6.h>
#endif
#include <net/addrconf.h>
#include "fcachehw.h"

#include "sysport_rsb.h"
#include "sysport_classifier.h"
#if defined(CONFIG_BCM_ARCHER_SIM) && defined(CC_ARCHER_SIM_MLT)
#include "sysport_mlt.h"
#endif
#include "sysport_driver.h"

#include "archer.h"
#include "archer_driver.h"

/*******************************************************************************
 *
 * Global Variables and Definitions
 *
 *******************************************************************************/

#define ARCHER_HOST_MODE_DEFAULT    ARCHER_MODE_L2_L3

typedef enum {
    ARCHER_HOST_OK                                        = 0,
    ARCHER_HOST_ERROR_NOENT                               = 1,
    ARCHER_HOST_ERROR_NORES                               = 2,
    ARCHER_HOST_ERROR_ENTRY_EXISTS                        = 3,
    ARCHER_HOST_ERROR_HOST_MAC_INVALID                    = 4,
    ARCHER_HOST_ERROR_HOST_MAC_ADDR_TABLE_INDEX_INVALID   = 5,
    ARCHER_HOST_ERROR_HOST_DEV_TABLE_INDEX_INVALID        = 6,
    ARCHER_HOST_ERROR_MAX
} ARCHER_HOST_ERROR_t;

/* Device MAC address */
typedef struct {
    BlogEthAddr_t mac_addr;
    uint16_t ref_count;
} host_mac_addr_table_t;

/* Device, and index to MAC address in host_mac_addr_table */
typedef struct {
    struct net_device *dev_p;
    uint16_t mac_index;
    uint16_t ref_count;
} host_dev_table_t;

#define ARCHER_HOST_DEV_TABLE_SIZE       16
#define ARCHER_HOST_MAC_ADDR_TABLE_SIZE  16

static host_mac_addr_table_t host_mac_addr_table_g[ARCHER_HOST_MAC_ADDR_TABLE_SIZE];

static host_dev_table_t host_dev_table_g[ARCHER_HOST_DEV_TABLE_SIZE];

static archer_mode_t archer_mode_g = ARCHER_HOST_MODE_DEFAULT;

/*******************************************************************************
 *
 * Local Functions
 *
 *******************************************************************************/

static inline unsigned char *__getDevAddr(struct net_device *dev_p)
{
    return &dev_p->dev_addr[0];
}

static inline char *__getDevName(struct net_device *dev_p)
{
    return dev_p->name;
}

static ARCHER_HOST_ERROR_t archer_host_mac_addr_table_set(uint32_t xi_table_index,
                                                          const BlogEthAddr_t *xi_host_mac_addr,
                                                          uint16_t xi_ref_count)
{
    if(xi_table_index >= ARCHER_HOST_MAC_ADDR_TABLE_SIZE)
    {
        return ARCHER_HOST_ERROR_HOST_MAC_ADDR_TABLE_INDEX_INVALID;
    }

    /* Store MAC address in a local table so we can return in the get accessor */
    host_mac_addr_table_g[xi_table_index].mac_addr = *xi_host_mac_addr;
    host_mac_addr_table_g[xi_table_index].ref_count = xi_ref_count;

    return ARCHER_HOST_OK;
}

static ARCHER_HOST_ERROR_t archer_host_mac_addr_table_get(uint32_t xi_table_index,
                                                          BlogEthAddr_t *xo_host_mac_addr,
                                                          uint16_t *xo_ref_count)
{
    if(xi_table_index >= ARCHER_HOST_MAC_ADDR_TABLE_SIZE)
    {
        return ARCHER_HOST_ERROR_HOST_MAC_ADDR_TABLE_INDEX_INVALID;
    }

    /* Look up address in local table. */
    *xo_host_mac_addr = host_mac_addr_table_g[xi_table_index].mac_addr;
    *xo_ref_count = host_mac_addr_table_g[xi_table_index].ref_count;

    return ARCHER_HOST_OK;
}

static ARCHER_HOST_ERROR_t archer_host_mac_addr_table_find(uint16_t *index_p, void *val_p)
{
    host_mac_addr_table_t *entry = (host_mac_addr_table_t *)val_p;
    uint32_t ii;

    /*Search for entry with matching address and greater than zero reference count*/
    for(ii = 0; ii < ARCHER_HOST_MAC_ADDR_TABLE_SIZE; ++ii)
    {
        BlogEthAddr_t test_val;
        uint16_t ref_count;
        int res = archer_host_mac_addr_table_get(ii, &test_val, &ref_count);
        BCM_ASSERT(!res);

        if(ref_count && !memcmp(test_val.u8, entry->mac_addr.u8, sizeof(BlogEthAddr_t)))
        {
            /* Entry found. Set the index to return */
            *index_p = ii;

            return ARCHER_HOST_OK;
        }
    }

    /* Not found */
    return ARCHER_HOST_ERROR_NOENT;
}

static ARCHER_HOST_ERROR_t archer_host_mac_addr_table_add(uint16_t *index_p, const void *val_p)
{
    host_mac_addr_table_t *entry = (host_mac_addr_table_t *)val_p;
    BlogEthAddr_t dummy;
    uint16_t ref_count;
    uint32_t ii;
    int res;

    /* Search for entry. Bump reference count if found. */
    if(!archer_host_mac_addr_table_find(index_p, (void *)val_p))
    {
        res = archer_host_mac_addr_table_get(*index_p, &dummy, &ref_count);
        BCM_ASSERT(!res);

        archer_host_mac_addr_table_set(*index_p, &entry->mac_addr, ++ref_count);

        return ARCHER_HOST_OK;
    }

    /* Find a free entry. Reference count 0 means entry is free. */
    for(ii = 0; ii < ARCHER_HOST_MAC_ADDR_TABLE_SIZE; ++ii)
    {
        res = archer_host_mac_addr_table_get(ii, &dummy, &ref_count);
        BCM_ASSERT(!res);

        if(!ref_count)
        {
            /* Free entry found. Use it */
            archer_host_mac_addr_table_set(ii, &entry->mac_addr, 1);

            /* set the index to return */
            *index_p = ii;

            return ARCHER_HOST_OK;
        }
    }

    /* No free entries available */
    return ARCHER_HOST_ERROR_NORES;
}

static ARCHER_HOST_ERROR_t archer_host_mac_addr_table_delete(uint16_t index)
{
    BlogEthAddr_t host_mac_addr;
    uint16_t ref_count;
    int res;

    /*Decrement reference count at index if not zero.*/
    res = archer_host_mac_addr_table_get(index, &host_mac_addr, &ref_count);
    BCM_ASSERT(!res);

    if(ref_count)
    {
        archer_host_mac_addr_table_set(index, &host_mac_addr, --ref_count);

        return ARCHER_HOST_OK;
    }
    else
    {
        return ARCHER_HOST_ERROR_NOENT;
    }
}

static void archer_host_mac_addr_table_dump(void)
{
    BlogEthAddr_t mac_addr;
    uint16_t ref_count;
    uint32_t ii;
    int res;

    bcm_print("idx                  MAC  mac_ref\n");

    for(ii = 0; ii < ARCHER_HOST_MAC_ADDR_TABLE_SIZE; ++ii)
    {
        res = archer_host_mac_addr_table_get(ii, &mac_addr, &ref_count);
        BCM_ASSERT(!res);

        if(ref_count)
        {
            bcm_print("%3u  <%02x:%2x:%02x:%02x:%02x:%02x>  %7d\n", ii, 
                      mac_addr.u8[0], mac_addr.u8[1], mac_addr.u8[2],
                      mac_addr.u8[3], mac_addr.u8[4], mac_addr.u8[5],
                      ref_count);
        }
    }
}

static ARCHER_HOST_ERROR_t archer_host_dev_table_set(uint32_t xi_table_index, struct net_device *xi_dev_p,
                                                     uint16_t xi_mac_index, uint16_t xi_ref_count)
{
    if(xi_table_index >= ARCHER_HOST_DEV_TABLE_SIZE)
    {
        return ARCHER_HOST_ERROR_HOST_DEV_TABLE_INDEX_INVALID;
    }

    /* Store MAC address in a local table so we can return in the get accessor */
    host_dev_table_g[xi_table_index].dev_p = xi_dev_p;
    host_dev_table_g[xi_table_index].mac_index = xi_mac_index;
    host_dev_table_g[xi_table_index].ref_count = xi_ref_count;

    return ARCHER_HOST_OK;
}

static ARCHER_HOST_ERROR_t archer_host_dev_table_get(uint32_t xi_table_index, struct net_device **xo_dev_pp,
                                                     uint16_t *xo_mac_index, uint16_t *xo_ref_count)
{
    if(xi_table_index >= ARCHER_HOST_DEV_TABLE_SIZE)
    {
        return ARCHER_HOST_ERROR_HOST_DEV_TABLE_INDEX_INVALID;
    }

    /* Look up dev in local table. */
    *xo_dev_pp = host_dev_table_g[xi_table_index].dev_p;
    *xo_mac_index = host_dev_table_g[xi_table_index].mac_index;
    *xo_ref_count = host_dev_table_g[xi_table_index].ref_count;

    return ARCHER_HOST_OK;
}

static ARCHER_HOST_ERROR_t archer_host_dev_table_find(uint16_t *index_p, void *val_p)
{
    host_dev_table_t *entry = (host_dev_table_t *)val_p;
    uint32_t ii;

    /* Search for entry with matching dev and greater than zero reference count */
    for(ii = 0; ii < ARCHER_HOST_DEV_TABLE_SIZE; ++ii)
    {
        struct net_device *test_val;
        uint16_t ref_count;
        uint16_t mac_index;
        int res = archer_host_dev_table_get(ii, &test_val, &mac_index, &ref_count);
        BCM_ASSERT(!res);

        if(ref_count && test_val == entry->dev_p)
        {
            /* Entry found. Set the index to return */
            *index_p = ii;

            return ARCHER_HOST_OK;
        }
    }

    /* Not found */
    return ARCHER_HOST_ERROR_NOENT;
}

static ARCHER_HOST_ERROR_t archer_host_dev_table_add(uint16_t *index_p, const void *val_p, uint16_t mac_index)
{
    host_dev_table_t *entry = (host_dev_table_t *)val_p;
    struct net_device *dummy_dev_p;
    uint16_t ref_count;
    uint16_t tmp_mac_index;
    uint32_t ii;
    int res;

    /* Search for entry. Bump reference count if found. */
    if(!archer_host_dev_table_find(index_p, entry))
    {
        return ARCHER_HOST_ERROR_ENTRY_EXISTS;
    }

    /* Find a free entry. Reference count 0 means entry is free. */
    for(ii = 0; ii < ARCHER_HOST_DEV_TABLE_SIZE; ++ii)
    {
        res = archer_host_dev_table_get(ii, &dummy_dev_p, &tmp_mac_index, &ref_count);
        BCM_ASSERT(!res);

        if(!ref_count)
        {
            /* Free entry found. Use it */
            archer_host_dev_table_set(ii, entry->dev_p, mac_index, 1);

            /* set the index to return */
            *index_p = ii;

            return ARCHER_HOST_OK;
        }
    }

    /* No free entries available */
    return ARCHER_HOST_ERROR_NORES;
}

static ARCHER_HOST_ERROR_t archer_host_dev_table_delete(uint16_t index)
{
    struct net_device *dummy_dev_p;
    uint16_t ref_count;
    uint16_t mac_index;
    int res;

    /* Decrement reference count at index if not zero. */
    res = archer_host_dev_table_get(index, &dummy_dev_p, &mac_index, &ref_count);
    BCM_ASSERT(!res);

    if(ref_count)
    {
        archer_host_dev_table_set(index, dummy_dev_p, mac_index, --ref_count);

        return ARCHER_HOST_OK;
    }
    else
    {
        return ARCHER_HOST_ERROR_NOENT;
    }
}

static void archer_host_dev_table_dump(void)
{
    struct net_device *dev_p;
    uint16_t mac_index;
    uint16_t ref_count;
    uint32_t ii;
    int res;

    bcm_print("idx            device               dev_p  dev_ref  mac_idx\n");

    /* Find a free entry. Reference count 0 means entry is free. */
    for(ii = 0; ii < ARCHER_HOST_DEV_TABLE_SIZE; ++ii)
    {
        res = archer_host_dev_table_get(ii, &dev_p, &mac_index, &ref_count);
        BCM_ASSERT(!res);

        if(ref_count)
        {
            /*Look up dev in local table.*/
            bcm_print("%3u  %16s  0x%16p  %7d  %7d\n",
                      ii, __getDevName(dev_p),
                      dev_p, ref_count, mac_index);
        }
    }
}

static ARCHER_HOST_ERROR_t archer_host_dev_mac_table_add(const void *val_p)
{
    struct net_device *dev_p = (struct net_device *)val_p;
    host_mac_addr_table_t host_mac_addr_entry;
    host_dev_table_t host_dev_entry;
    BlogEthAddr_t zero_mac = {};
    uint16_t dev_index;
    uint16_t mac_index;
    int ret;

    if(!memcmp(__getDevAddr(dev_p), zero_mac.u8, sizeof(BlogEthAddr_t)))
    {
        return ARCHER_HOST_ERROR_HOST_MAC_INVALID;
    }

    /* Device entry already exists? If yes, no need to add new device or mac entry. */
    host_dev_entry.dev_p = dev_p;
    ret = archer_host_dev_table_find(&dev_index, &host_dev_entry);
    if(ret == ARCHER_HOST_OK)
    {
        return ret;
    }

    memcpy(host_mac_addr_entry.mac_addr.u8, __getDevAddr(dev_p), ETH_ALEN);
    host_mac_addr_entry.ref_count = 0;
    ret = archer_host_mac_addr_table_add(&mac_index, &host_mac_addr_entry);
    if(ret != ARCHER_HOST_OK)
    {
        return ret;
    }

    host_dev_entry.dev_p = dev_p;
    host_dev_entry.mac_index = 0xFFFF;
    host_dev_entry.ref_count = 0;

    /* Add new device. If add fails, then delete the MAC entry. */
    ret = archer_host_dev_table_add(&dev_index, &host_dev_entry, mac_index);
    if(ret != ARCHER_HOST_OK)
    {
        archer_host_mac_addr_table_delete(mac_index);
    }

    return ret;
}

static ARCHER_HOST_ERROR_t archer_host_dev_mac_table_delete(const void *val_p)
{
    struct net_device *dev_p = (struct net_device *)val_p;
    host_dev_table_t host_dev_entry;
    uint16_t ref_count;
    uint16_t dev_index;
    uint16_t mac_index;
    int ret;

    host_dev_entry.dev_p = dev_p;
    host_dev_entry.mac_index = 0xFFFF;
    host_dev_entry.ref_count = 0;

    /* Search for entry */
    ret = archer_host_dev_table_find(&dev_index, &host_dev_entry);
    if(ret != ARCHER_HOST_OK)
    {
        return ret;
    }

    /* Get mac_index for this device */
    ret = archer_host_dev_table_get(dev_index, &dev_p, &mac_index, &ref_count);
    if (ret != ARCHER_HOST_OK)
    {
        return ret;
    }

    if(!ref_count)
    {
        return ARCHER_HOST_ERROR_NOENT;
    }
    else
    {
        ret = archer_host_dev_table_delete(dev_index);
    }

    ret = archer_host_mac_addr_table_delete(mac_index);

    return ret;
}

#if !defined(CONFIG_BCM_ARCHER_SIM)
static int archer_host_mlt_config(sysport_driver_mlt_cmd_t cmd,
                                  struct net_device *dev_p,
                                  unsigned char *dev_addr)
{
    int rxq_index = (IFF_WANDEV & dev_p->priv_flags) ?
        SYSPORT_DRIVER_RXQ_INDEX_WAN : SYSPORT_DRIVER_RXQ_INDEX_LAN;
    sysport_driver_mlt_data_t mlt_data;
    sysport_driver_mlt_index_t mlt_index;
    uint16_t mac_da_47_32;
    uint32_t mac_da_31_0;

    mlt_data.data1 = 0;
    mlt_data.valid = 1;
    mlt_data.mac_da_type = SYSPORT_DRIVER_MLT_MAC_DA_TYPE_HOST;
    mlt_data.rxq_index = rxq_index;

    memcpy(&mac_da_47_32, &dev_addr[0], 2);
    memcpy(&mac_da_31_0, &dev_addr[2], 4);
    mlt_data.mac_da_47_32 = ntohs(mac_da_47_32);
    mlt_data.mac_da_31_0 = ntohl(mac_da_31_0);

//    bcm_print("%s: cmd %d, valid %d, mac_da_type %d, rxq_index %d, mac_da_47_32 0x%04X, mac_da_31_0 0x%08X\n", __FUNCTION__, cmd, mlt_data.valid, mlt_data.mac_da_type, mlt_data.rxq_index, mlt_data.mac_da_47_32, mlt_data.mac_da_31_0);

    return sysport_driver_mlt_cmd(cmd, &mlt_data, &mlt_index);
}
#endif

static int archer_host_netdev_event_add_host_mac(struct net_device *dev_p)
{
    unsigned char *dev_addr;
    int ret;

    dev_addr = __getDevAddr(dev_p);

    __logInfo("dev=%s : Adding host MAC <0x%02x:%02x:%02x:%02x:%02x:%02x>\n", 
              __getDevName(dev_p),
              dev_addr[0], dev_addr[1], dev_addr[2],
              dev_addr[3], dev_addr[4], dev_addr[5]);

    ret = archer_host_dev_mac_table_add(dev_p);
    if(ret != ARCHER_HOST_OK)
    {
        return ret;
    }

#if defined(CONFIG_BCM_ARCHER_SIM)
#if defined(CC_ARCHER_SIM_MLT)
    {
        sysport_mlt_index_t mlt_index;
        sysport_mlt_info_t info;

        info.u16 = 0;
        info.mac_addr_type = SYSPORT_MLT_MAC_ADDR_TYPE_HOST;

        sysport_mlt_write((sysport_mlt_key_t *)dev_addr, &info, &mlt_index);
    }
#endif
#else
    ret = archer_host_mlt_config(SYSPORT_DRIVER_MLT_CMD_WRITE, dev_p, dev_addr);
    if(ret < SYSPORT_DRIVER_MLT_CMD_SUCCESS)
    {
        __logError("Could not SYSPORT_DRIVER_MLT_CMD_WRITE: ret %d", ret);
    }
#endif

    // XXX: Add Static ARL Entry
    /* if(ret != 0) */
    /* { */
    /*     __logError("Could not configure Switch\n"); */

    /*     ret = archer_host_dev_mac_table_delete(dev_p); */
    /* } */

    return ret;
}

static int archer_host_netdev_event_delete_host_mac(struct net_device *dev_p)
{
    unsigned char *dev_addr;
    int ret;

    dev_addr = __getDevAddr(dev_p);

    __logInfo("dev=%s : Removing host MAC <0x%02x:%02x:%02x:%02x:%02x:%02x>\n", 
              __getDevName(dev_p), 
              dev_addr[0], dev_addr[1], dev_addr[2],
              dev_addr[3], dev_addr[4], dev_addr[5]);

#if defined(CONFIG_BCM_ARCHER_SIM)
#if defined(CC_ARCHER_SIM_MLT)
    {
        sysport_mlt_index_t mlt_index;

        sysport_mlt_invalidate_by_key((sysport_mlt_key_t *)dev_addr, &mlt_index);
    }
#endif
#else
    archer_host_mlt_config(SYSPORT_DRIVER_MLT_CMD_CLEAR, dev_p, dev_addr);
#endif

    // XXX: Remove Static ARL Entry

    ret = archer_host_dev_mac_table_delete(dev_p);

    return ret;
}

static int archer_host_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev_p = NETDEV_NOTIFIER_GET_DEV(ptr);
    int ret;

    __logDebug("\n\ndev=%s event=%lu\n", __getDevName(dev_p), event);

    if(!blog_is_config_netdev_mac(dev_p, 0))
    {
        return NOTIFY_DONE;
    }

    switch (event) {
        case NETDEV_UP:
            __logInfo("Link UP 0x%p\n", dev_p);
            ret = archer_host_netdev_event_add_host_mac(dev_p);
            break;

        case NETDEV_CHANGEADDR:
            __logInfo("MAC Change %p\n", dev_p);
            ret = archer_host_netdev_event_delete_host_mac(dev_p);
            ret = archer_host_netdev_event_add_host_mac(dev_p);
            break;

        case NETDEV_DOWN:
            __logInfo("Link DN 0x%p\n", dev_p);
            ret = archer_host_netdev_event_delete_host_mac(dev_p);
            break;

        case NETDEV_GOING_DOWN:
            __logInfo("Link Going DN 0x%p\n", dev_p);
            ret = archer_host_netdev_event_delete_host_mac(dev_p);
            break;

        case NETDEV_CHANGE:
            if(netif_carrier_ok(dev_p))
            {
                __logInfo("Link Change UP 0x%p\n", dev_p);
                ret = archer_host_netdev_event_add_host_mac(dev_p);
            }
            else
            {
                __logInfo("Link Change DN 0x%p\n", dev_p);
                ret = archer_host_netdev_event_delete_host_mac(dev_p);
            }
            break;
    }

    return NOTIFY_DONE;
}

/*******************************************************************************
 *
 * API
 *
 *******************************************************************************/

#if (defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)) && !defined(CC_ARCHER_SIM_MLT)
int archer_host_mac_address_match(uint8_t *packet_p)
{
    if(archer_mode_g == ARCHER_MODE_L2_L3)
    {
        int ii;

        for(ii = 0; ii < ARCHER_HOST_MAC_ADDR_TABLE_SIZE; ++ii)
        {
            host_mac_addr_table_t *table_p = &host_mac_addr_table_g[ii];

            if(table_p->ref_count)
            {
                uint32_t *table_u32_p = (uint32_t *)&table_p->mac_addr.u16[0];
                uint32_t *packet_u32_p = (uint32_t *)packet_p;

                if(*table_u32_p == *packet_u32_p &&
                   table_p->mac_addr.u16[2] == ((BlogEthAddr_t *)(packet_p))->u16[2])
                {
                    return 1;
                }
            }
        }

        return 0;
    }
    else
    {
        return 1;
    }
}
#endif

int archer_host_mode_set(archer_mode_t mode)
{
    char *mode_name;

    switch(mode)
    {
        case ARCHER_MODE_L3:
            sysport_driver_parser_mode_set(1);
            archer_mode_g = mode;
            mode_name = "L3";
            break;

        case ARCHER_MODE_L2_L3:
            sysport_driver_parser_mode_set(0);
            archer_mode_g = mode;
            mode_name = "L2+L3";
            break;

        default:
            __logError("Invalid mode %d\n", mode);
            return -1;
    }

    bcm_print("Archer Mode set to %s\n", mode_name);

    return 0;
}

static int archer_host_fc_accel_mode_set(uint32_t accel_mode)
{
    int ret;

    switch(accel_mode)
    {
        case BLOG_ACCEL_MODE_L3:
            ret = archer_host_mode_set(ARCHER_MODE_L3);
            break;

        case BLOG_ACCEL_MODE_L23:
            ret = archer_host_mode_set(ARCHER_MODE_L2_L3);
            break;

        default:
            __logError("Invalid accel_mode %d\n", accel_mode);
            ret = -1;
    }

    return ret;
}

void archer_host_info_dump(void)
{
    bcm_print("Archer Mode: %s\n\n",
              (archer_mode_g == ARCHER_MODE_L3) ? "L3" : "L2+L3");

    bcm_print("Archer Device Table\n");
    archer_host_dev_table_dump();

    bcm_print("\nArcher Host MAC Addresses Table\n");
    archer_host_mac_addr_table_dump();
}

static struct notifier_block archer_host_netdev_notifier = {
    .notifier_call = archer_host_netdev_event,
};

int __init archer_host_construct(void)
{
    memset(host_mac_addr_table_g, 0,
           sizeof(host_mac_addr_table_t) * ARCHER_HOST_MAC_ADDR_TABLE_SIZE);

    memset(host_dev_table_g, 0,
           sizeof(host_dev_table_t) * ARCHER_HOST_DEV_TABLE_SIZE);

    archer_host_mode_set(ARCHER_HOST_MODE_DEFAULT);

#if defined(CONFIG_BCM_ARCHER_SIM) && defined(CC_ARCHER_SIM_MLT)
    sysport_mlt_init();
#endif

    register_netdevice_notifier(&archer_host_netdev_notifier);

    /* bind to acceleration mode function hook used by blog/flow_cache */
    blog_accel_mode_set_fn = archer_host_fc_accel_mode_set;

    /* Set the Archer acceleration mode to be in sync with blog/flow cache */
    archer_host_fc_accel_mode_set( blog_support_get_accel_mode() );

    bcm_print("Initialized Archer Host Layer\n");

    return 0;
}

void __exit archer_host_destruct(void)
{
    unregister_netdevice_notifier(&archer_host_netdev_notifier);

    blog_accel_mode_set_fn = NULL;
}
