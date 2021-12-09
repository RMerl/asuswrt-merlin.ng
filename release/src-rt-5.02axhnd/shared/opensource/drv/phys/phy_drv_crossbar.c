/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

/*
 *  Created on: Jul 2017
 *      Author: ido.brezel@broadcom.com
 */

/*
 * Phy driver wrapper for crossbar
 */

#include "phy_drv.h"
#include "phy_drv_crossbar.h"

#define dprintk printk

phy_drv_t phy_drv_crossbar;

typedef struct
{
    int crossbar_id;
    int internal_endpoint;
    phy_dev_t self;
    phy_dev_t *phys[MAX_PHYS_PER_CROSSBAR_GROUP];
    int external_endpoint[MAX_PHYS_PER_CROSSBAR_GROUP];
    phy_dev_t *active_phy;
    select_cb_t select;
} crossbar_group_t;

static crossbar_group_t crossbars[MAX_CROSSBAR_GROUPS];

static int crossbar_get_index(phy_dev_t *phy_dev_crossbar)
{
    int i;
    
    if (!phy_dev_crossbar)
        return -1;
        
    for (i = 0; i < MAX_CROSSBAR_GROUPS; i++)
    {
        if (&crossbars[i].self == phy_dev_crossbar)
            return i;
    }

    return -1;
}

static crossbar_group_t *crossbar_get(phy_dev_t *phy_dev_crossbar)
{
    int i = crossbar_get_index(phy_dev_crossbar);

    if (i == -1)
        return NULL;
            
    return &crossbars[i];
}

static int crossbar_idx_by_phy(phy_dev_t *phy_dev, int *group_idx, int *phys_idx)
{
    int i, j;

    if (!phy_dev)
        return -1;
    for (i = 0; i < MAX_CROSSBAR_GROUPS ; i++)
    {
        if (!crossbars[i].self.phy_drv)
            continue;

        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            if (crossbars[i].phys[j] == phy_dev)
            {
                *group_idx = i;
                *phys_idx = j;
                return 0;
            }
        }
    }

    return -1;
}

/*
phy_dev_t *crossbar_group_phy_by_indices(int crossbar_id, int internal_endpoint, int external_endpoint)
{
    int i, j;

    for (i = 0; i < MAX_CROSSBAR_GROUPS ; i++)
    {
        if (!crossbars[i].self.phy_drv || crossbars[i].crossbar_id == crossbar_id)
            continue;

        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            if (crossbars[i].phys[j] && crossbars[i].external_endpoint[j] == external_endpoint)
                return crossbars[i].phys[j];
        }
    }

    return NULL;
}
EXPORT_SYMBOL(crossbar_group_phy_by_indices);
*/

phy_dev_t *crossbar_group(int crossbar_id, int internal_endpoint)
{
    int i;

    for (i = 0; i < MAX_CROSSBAR_GROUPS ; i++)
    {
        if (crossbars[i].self.phy_drv && crossbars[i].crossbar_id == crossbar_id && crossbars[i].internal_endpoint == internal_endpoint)
            return &crossbars[i].self;
    }

    return NULL;
}
EXPORT_SYMBOL(crossbar_group);

static crossbar_group_t *crossbar_get_by_phy(phy_dev_t *phy_dev)
{
    int i, j;

    if (!crossbar_idx_by_phy(phy_dev, &i, &j))
        return &crossbars[i];

    return NULL;
}

int crossbar_external_endpoint(phy_dev_t *phy_dev)
{
    int i, j;

    if (!crossbar_idx_by_phy(phy_dev, &i, &j))
        return crossbars[i].external_endpoint[j];

    return -1;
}
EXPORT_SYMBOL(crossbar_external_endpoint);

int crossbar_info_by_phy(phy_dev_t *phy_dev, int *crossbar_id, int *internal_endpoint, int *external_endpoint)
{
    int i, j;

    if (!crossbar_idx_by_phy(phy_dev, &i, &j))
    {
        if (crossbar_id)
            *crossbar_id = crossbars[i].crossbar_id;
        if (internal_endpoint)
            *internal_endpoint = crossbars[i].internal_endpoint;
        if (external_endpoint)
            *external_endpoint = crossbars[i].external_endpoint[j];
        return 0;
    }
    return -1;
}
EXPORT_SYMBOL(crossbar_info_by_phy);

/*
 * When the active phy's link is up/down, power down/up all other phys in group.
 */
static void _crossbar_link_powerset(crossbar_group_t *crossbar)
{
    int j;

    for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
    {
        if (!crossbar->phys[j])
            continue;
        /* Skip active phy */
        if (crossbar->phys[j] == crossbar->active_phy)
            continue;

        /* if active phy is cascade, skip phy on the same cascade */
        if (crossbar->active_phy && is_cascade_phy(crossbar->active_phy) &&
            (cascade_phy_get_first(crossbar->active_phy) == cascade_phy_get_first(crossbar->phys[j])))
            continue;

        phy_dev_power_set(crossbar->phys[j], !crossbar->active_phy->link);
    }
}

static void crossbar_link_change_cb(void *ctx)
{
    phy_dev_t *phy_dev_crossbar, *phy_dev = ctx;
    phy_dev_t *first_phy = cascade_phy_get_first(phy_dev);
    crossbar_group_t *crossbar = crossbar_get_by_phy(first_phy);
    phy_dev_crossbar = &crossbar->self;

    /* No change in active_phy, return */
    if (phy_dev != crossbar->active_phy && crossbar->active_phy->link)
        return;

    phy_dev_crossbar->link = phy_dev->link;
    phy_dev_crossbar->speed = phy_dev->speed;
    phy_dev_crossbar->duplex = phy_dev->duplex;
    phy_dev_crossbar->pause_rx = phy_dev->pause_rx;
    phy_dev_crossbar->pause_tx = phy_dev->pause_tx;

    if (crossbar->select && phy_dev != crossbar->active_phy)
    {
        crossbar->select(crossbar->crossbar_id, crossbar->internal_endpoint, crossbar_external_endpoint(phy_dev), first_phy);
        crossbar->active_phy = phy_dev;     /* in case active_phy is end_phy */
        phy_dev_crossbar->addr = phy_dev->addr;
        phy_dev_crossbar->meta_id = phy_dev->meta_id;
    }

    if (phy_dev_crossbar->flag & PHY_FLAG_POWER_SET_ENABLED)
        _crossbar_link_powerset(crossbar);

    if (phy_dev_crossbar->link_change_cb)
        phy_dev_crossbar->link_change_cb(phy_dev_crossbar->link_change_ctx);
}

int phy_drv_crossbar_group_phy_del(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev)
{
    int j;
    crossbar_group_t *crossbar = crossbar_get(phy_dev_crossbar);

    if (!crossbar)
    {
        printk("crossbar: Failed to find crossbar for phy_dev_crossbar %p\n", phy_dev_crossbar);
        goto Exit;
    }
    
    for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
    {
        if (crossbar->phys[j] == phy_dev)
        {
            // III power down ??

            crossbar->active_phy = crossbar->phys[j] = NULL;
            return 0;
        }
    }

Exit:
    printk("crossbar[%d]: failed to remove phy_dev %p\n", crossbar_get_index(&crossbar->self), phy_dev);

    return -1;
}
EXPORT_SYMBOL(phy_drv_crossbar_group_phy_del);

int phy_drv_crossbar_group_phy_add(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev, int external_endpoint)
{
    int i, j, rc = -1;
    crossbar_group_t *crossbar = crossbar_get(phy_dev_crossbar);

    if (!crossbar)
    {
        printk("crossbar: Failed to find crossbar for phy_dev_crossbar %p\n", phy_dev_crossbar);
        goto Exit;
    }
    
    if (!phy_dev)
    {
        printk("crossbar: Missing phy_dev %p\n", phy_dev);
        goto Exit;
    }
    
    if (!crossbar_idx_by_phy(phy_dev, &i, &j))
    {
        printk("crossbar[%d]: phy %p already enslaved to another crossbar group\n", crossbar_get_index(&crossbar->self), phy_dev);
        goto Exit;
    }

    for (i = 0; i < MAX_PHYS_PER_CROSSBAR_GROUP && crossbar->phys[i]; i++);

    if (i == MAX_PHYS_PER_CROSSBAR_GROUP)
    {
        goto Exit;
    }

    crossbar->phys[i] = phy_dev;
    crossbar->external_endpoint[i] = external_endpoint;
    rc = 0;

Exit:
    dprintk("crossbar[%d]: Add cb_idx=%d ext_ep=%d %s%s:%d\n", crossbar_get_index(&crossbar->self), crossbar->crossbar_id, external_endpoint, rc ? "failed " : "", phy_dev->phy_drv->name, phy_dev->addr); // crossbar idx

    return rc;
}
EXPORT_SYMBOL(phy_drv_crossbar_group_phy_add);

phy_dev_t *phy_drv_crossbar_group_alloc(int crossbar_id, int internal_endpoint, select_cb_t select)
{
    int i;
    crossbar_group_t *crossbar = NULL;
    
    for (i = 0; i < MAX_CROSSBAR_GROUPS; i++)
    {
        crossbar = &crossbars[i];
        if (crossbar->self.phy_drv && crossbar->crossbar_id == crossbar_id &&
            crossbar->internal_endpoint == internal_endpoint)
        {
            printk("crossbar: Alloc failed, internal endpoint %d already registered with another group\n", internal_endpoint);
            return NULL;
        }
    }

    for (i = 0; i < MAX_CROSSBAR_GROUPS && crossbars[i].self.phy_drv; i++);

    if (i == MAX_CROSSBAR_GROUPS)
    {
        printk("crossbar: Failed to allocate new crossbar, Max crossbars reached\n");
        return NULL;
    }

    crossbars[i].self.phy_drv = &phy_drv_crossbar;
    crossbars[i].crossbar_id = crossbar_id;
    crossbars[i].internal_endpoint = internal_endpoint;
    crossbars[i].select = select;
    dprintk("crossbar[%d]: Allocated cb_idx=%d int_ep=%d\n", i, crossbar_id, internal_endpoint);

    return &crossbars[i].self;
}
EXPORT_SYMBOL(phy_drv_crossbar_group_alloc);

void phy_drv_crossbar_group_list(void)
{
    int i, j;
    phy_dev_t *phy_dev;

    printk("|==========================================|\n");
    printk("|  Id  |  active |   Phy   |   Bus  | Addr |\n");
    printk("|==========================================|\n");
    

    for (i = 0; i < MAX_CROSSBAR_GROUPS ; i++)
    {
        if (!crossbars[i].self.phy_drv)
            continue;
        
        printk("|   %1d  | I%1d-     |         |        |      |\n", i, crossbars[i].internal_endpoint);

        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            phy_dev = crossbars[i].phys[j];
            while (phy_dev)
            {
                printk("|      ");
                printk("|   -E%1d%2s ", crossbars[i].external_endpoint[j], crossbars[i].active_phy == phy_dev ? "*" : " ");
                printk("| %7s ", phy_dev->phy_drv->name);
                printk("| %6s ", phy_dev->mii_type == PHY_MII_TYPE_UNKNOWN ? "" : phy_dev_mii_type_to_str(phy_dev->mii_type));
                printk("| 0x%02x ", phy_dev->addr);
                printk("|\n");
                phy_dev = cascade_phy_get_next(phy_dev);    // traverse thru cascade phys if available
            }
        }

        printk("|==========================================|\n");
    }
}
EXPORT_SYMBOL(phy_drv_crossbar_group_list);

phy_dev_t *crossbar_phy_dev_first(phy_dev_t *phy_dev_crossbar)
{
    int j;
    crossbar_group_t *crossbar = crossbar_get(phy_dev_crossbar);

    if (crossbar)
    {
        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            if (crossbar->phys[j])
                return crossbar->phys[j];
        }
    }

    return NULL;
}
EXPORT_SYMBOL(crossbar_phy_dev_first);

phy_dev_t *crossbar_phy_dev_next(phy_dev_t *phy_dev)
{
    int i, j;

    for (i = 0; i < MAX_CROSSBAR_GROUPS ; i++)
    {
        if (!crossbars[i].self.phy_drv)
            continue;

        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            if (crossbars[i].phys[j] == phy_dev)
            {
                for (j++; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
                {
                    if (crossbars[i].phys[j])
                        return crossbars[i].phys[j];
                }
                return NULL;
            }
        }
    }

    return NULL;
}
EXPORT_SYMBOL(crossbar_phy_dev_next);

phy_dev_t *crossbar_phy_dev_active(phy_dev_t *phy_dev_crossbar)
{
    crossbar_group_t *crossbar = crossbar_get(phy_dev_crossbar);

    if (crossbar)
        return crossbar->active_phy;

    return NULL;
}
EXPORT_SYMBOL(crossbar_phy_dev_active);

int crossbar_phys_in_one_group(phy_dev_t *phy1, phy_dev_t *phy2)
{
    return (crossbar_get_by_phy(phy1) == crossbar_get_by_phy(phy2));
}
EXPORT_SYMBOL(crossbar_phys_in_one_group);

int crossbar_external_to_internal_endpoint(int crossbar_id, int external_endpoint)
{
    int i, j;
    
    for (i = 0; i < MAX_CROSSBAR_GROUPS; i++)
    {
        if (crossbars[i].crossbar_id != crossbar_id )
            continue;

        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            if (crossbars[i].phys[j] && crossbars[i].external_endpoint[j] == external_endpoint)
                return crossbars[i].internal_endpoint;
        }
    }
    return -1;
}
EXPORT_SYMBOL(crossbar_external_to_internal_endpoint);

phy_dev_t *crossbar_get_phy_by_type(int phy_type)
{
    int i, j;
    
    for (i = 0; i < MAX_CROSSBAR_GROUPS; i++)
    {
        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            phy_dev_t *phy = crossbars[i].phys[j];
            if (phy && phy->phy_drv->phy_type == phy_type)
                return phy;
        }
    }
    return NULL;
}
EXPORT_SYMBOL(crossbar_get_phy_by_type);

int crossbar_group_external_endpoint_count(phy_dev_t *phy_dev_crossbar, int *external_map)
{
    int j, count = 0;
    int map = 0;
    crossbar_group_t *crossbar = crossbar_get(phy_dev_crossbar);
    if (crossbar)
    {
        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
            if (crossbar->phys[j])
            {
                count++;
                map |= 1 << crossbar->external_endpoint[j];
            }
    }
    else
        return -1;  // not a valid crossbar_phy

    if (external_map)
        *external_map = map;
    return count;
}
EXPORT_SYMBOL(crossbar_group_external_endpoint_count);

phy_dev_t *crossbar_group_phy_get(phy_dev_t *phy_dev_crossbar, int external_endpoint)
{
    crossbar_group_t *crossbar = crossbar_get(phy_dev_crossbar);
    int j;
    if (crossbar)
    {
        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
            if (crossbar->phys[j] && crossbar->external_endpoint[j] == external_endpoint)
                return crossbar->phys[j];
    }
    return NULL;
}
EXPORT_SYMBOL(crossbar_group_phy_get);

int crossbar_set_active_external_endpoint(int crossbar_id, int internal_endpoint, int external_endpoint)
{
    int i, j;
    for (i = 0; i < MAX_CROSSBAR_GROUPS; i++)
    {
        if (crossbars[i].crossbar_id == crossbar_id && crossbars[i].internal_endpoint == internal_endpoint)
        {
            for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
            {
                if (crossbars[i].phys[j] && crossbars[i].external_endpoint[j] == external_endpoint)
                {
                    crossbars[i].active_phy = crossbars[i].phys[j];
                    crossbars[i].self.addr = crossbars[i].phys[j]->addr;
                    crossbars[i].self.meta_id = crossbars[i].phys[j]->meta_id;
                    return 0;
                }
            }
        }
    }
    return -1;
}
EXPORT_SYMBOL(crossbar_set_active_external_endpoint);

int crossbar_get_int_ext_mapping(int crossbar_id, int max_internal_endpoint, int max_external_endpoint, int *endpoint_pairs)
{
    int i, j;
    int ext_unused_map = ( 1<<max_external_endpoint) - 1;

    for (i = 0; i < max_internal_endpoint; i++)
        endpoint_pairs[i] = -1;
        
    for (i = 0; i < MAX_CROSSBAR_GROUPS; i++)
    {
        if (crossbars[i].crossbar_id != crossbar_id )
            continue;

        for (j = 0; j < MAX_PHYS_PER_CROSSBAR_GROUP; j++)
        {
            if (crossbars[i].phys[j])
            {
                endpoint_pairs[crossbars[i].internal_endpoint] = crossbars[i].external_endpoint[j];
                ext_unused_map &= ~(1<<crossbars[i].external_endpoint[j]);
            }
        }
    }

    // fill in unpopulated internal endpoint with unused external endpoint
    for (i = 0; i < max_internal_endpoint; i++)
    {
        if (endpoint_pairs[i] == -1)
        {
            for (j = 0; j < max_external_endpoint; j++)
            {
                if (ext_unused_map & (1<<j))
                {
                    endpoint_pairs[i] = j;
                    ext_unused_map &= ~(1<<j);
                }
            }
        }
    }
    return 0;
}
EXPORT_SYMBOL(crossbar_get_int_ext_mapping);

#define PHY_CB_DBG  0

#define PHY_CB(name) cb_##name
#define PHY_ALL(_name, proto, args)\
    static int PHY_CB(_name) proto\
    {\
        phy_dev_t *phy_dev = crossbar_phy_dev_first(phy_dev_crossbar);\
        if (PHY_CB_DBG) dprintk("crossbar[%d]: ALL(%s)\n", crossbar_get_index(phy_dev_crossbar), #_name);\
        for (; phy_dev; phy_dev = crossbar_phy_dev_next(phy_dev))\
        {\
            if (PHY_CB_DBG) dprintk("  %s:%x:%pK %s\n", phy_dev->phy_drv->name, phy_dev->addr, phy_dev->priv, phy_dev->phy_drv->_name? #_name:"none");\
            cascade_phy_dev_ ## _name args;\
        }\
        return 0;\
    }

#define PHY_ONE(_name,do_cascade, proto, args)\
    static int PHY_CB(_name) proto\
    {\
        phy_dev_t *phy_dev = crossbar_phy_dev_active(phy_dev_crossbar);\
        if (PHY_CB_DBG) dprintk("crossbar[%d]: ONE(%s) active: %s:%x\n", crossbar_get_index(phy_dev_crossbar), #_name, (phy_dev ? phy_dev->phy_drv->name : "NONE"), phy_dev->addr);\
        if (!phy_dev) return 0; /* BUG?? */\
        return do_cascade ## phy_dev_ ## _name args;\
    }

/* Beware which operations operate on ACTIVE or ALL phys */
PHY_ONE(read,, (phy_dev_t *phy_dev_crossbar, uint16_t reg, uint16_t *val), (phy_dev, reg, val));
PHY_ONE(write,, (phy_dev_t *phy_dev_crossbar, uint16_t reg, uint16_t val), (phy_dev, reg, val));
/* III: Should this be only on active ? */
PHY_ALL(power_set, (phy_dev_t *phy_dev_crossbar, int enable), (phy_dev, enable));
PHY_ONE(apd_get,, (phy_dev_t *phy_dev_crossbar, int *enable), (phy_dev, enable));
PHY_ALL(apd_set, (phy_dev_t *phy_dev_crossbar, int enable), (phy_dev, enable));
PHY_ONE(eee_get,, (phy_dev_t *phy_dev_crossbar, int *enable), (phy_dev, enable));
PHY_ALL(eee_set, (phy_dev_t *phy_dev_crossbar, int enable), (phy_dev, enable));
PHY_ONE(eee_resolution_get,, (phy_dev_t *phy_dev_crossbar, int *enable), (phy_dev, enable));
PHY_ONE(read_status,, (phy_dev_t *phy_dev_crossbar), (phy_dev));
PHY_ALL(speed_set, (phy_dev_t *phy_dev_crossbar, phy_speed_t speed, phy_duplex_t duplex), (phy_dev, speed, duplex));
PHY_ONE(caps_get, cascade_, (phy_dev_t *phy_dev_crossbar, int caps_type, uint32_t *caps), (phy_dev, caps_type, caps));
PHY_ALL(caps_set, (phy_dev_t *phy_dev_crossbar, uint32_t caps), (phy_dev, caps));
PHY_ONE(phyid_get,, (phy_dev_t *phy_dev_crossbar, uint32_t *phyid), (phy_dev, phyid));
static int cb_init(phy_dev_t *phy_dev_crossbar)
{
    phy_dev_t *phy_dev = crossbar_phy_dev_first(phy_dev_crossbar);
    if (PHY_CB_DBG) dprintk("crossbar[%d]: ALL(init)\n", crossbar_get_index(phy_dev_crossbar));
    for (; phy_dev; phy_dev = crossbar_phy_dev_next(phy_dev))
    {
        if (PHY_CB_DBG) dprintk("  %s:%x:%pK %s\n", phy_dev->phy_drv->name, phy_dev->addr, phy_dev->priv, phy_dev->phy_drv->init? "init":"none");
        phy_register_polling_timer(phy_dev, crossbar_link_change_cb);
        phy_dev_init(phy_dev);
    }
    return 0;
}
PHY_ALL(isolate_phy, (phy_dev_t *phy_dev_crossbar, int isolate), (phy_dev, isolate));

phy_drv_t phy_drv_crossbar =
{
    .phy_type = PHY_TYPE_CROSSBAR,
    .name = "crossbar",
    .read = PHY_CB(read),
    .write = PHY_CB(write),
    .power_set = PHY_CB(power_set),
    .apd_get = PHY_CB(apd_get),
    .apd_set = PHY_CB(apd_set),
    .eee_get = PHY_CB(eee_get),
    .eee_set = PHY_CB(eee_set),
    .eee_resolution_get = PHY_CB(eee_resolution_get),
    .read_status = PHY_CB(read_status),
    .speed_set = PHY_CB(speed_set),
    .caps_get = PHY_CB(caps_get),
    .caps_set = PHY_CB(caps_set),
    .phyid_get = PHY_CB(phyid_get),
    .isolate_phy = PHY_CB(isolate_phy),
    .init = PHY_CB(init),
};

