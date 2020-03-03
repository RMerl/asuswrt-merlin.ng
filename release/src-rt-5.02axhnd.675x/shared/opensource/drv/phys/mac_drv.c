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
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

#include "mac_drv.h"

#define MAX_MAC_DEVS 16

extern mac_drv_t mac_drv_unimac;
extern mac_drv_t mac_drv_lport;
extern mac_drv_t mac_drv_gmac;
extern mac_drv_t mac_drv_sf2;
#ifdef MAC_XPORT
extern mac_drv_t mac_drv_xport;
#endif
static mac_drv_t *mac_drivers[MAC_TYPE_MAX] = {};

int mac_driver_set(mac_drv_t *mac_drv)
{
    if (mac_drivers[mac_drv->mac_type])
    {
        printk("Failed adding mac driver %s: already set\n", mac_drv->name);
        return -1;
    }
    else
    {
        mac_drivers[mac_drv->mac_type] = mac_drv;
        return 0;
    }
}
EXPORT_SYMBOL(mac_driver_set);

int mac_driver_init(mac_type_t mac_type)
{
    mac_drv_t *mac_drv;

    if (!(mac_drv = mac_drivers[mac_type]))
        return 0;

    return mac_drv_init(mac_drv);
}
EXPORT_SYMBOL(mac_driver_init);

int mac_drivers_set(void)
{
    int ret = 0;

#ifdef MAC_UNIMAC
    ret |= mac_driver_set(&mac_drv_unimac);
#endif
#ifdef MAC_LPORT
    ret |= mac_driver_set(&mac_drv_lport);
#endif
#ifdef MAC_GMAC
    ret |= mac_driver_set(&mac_drv_gmac);
#endif
#if defined(MAC_SF2) || defined(MAC_SF2_EXTERNAL)
    ret |= mac_driver_set(&mac_drv_sf2);
#endif
#ifdef MAC_XPORT
    ret |= mac_driver_set(&mac_drv_xport);
#endif

    return ret;
}
EXPORT_SYMBOL(mac_drivers_set);

int mac_drivers_init(void)
{
    int ret = 0;

    ret |= mac_driver_init(MAC_TYPE_UNIMAC);
    ret |= mac_driver_init(MAC_TYPE_LPORT);
    ret |= mac_driver_init(MAC_TYPE_GMAC);
    ret |= mac_driver_init(MAC_TYPE_SF2);
    ret |= mac_driver_init(MAC_TYPE_XPORT);

    return ret;
}
EXPORT_SYMBOL(mac_drivers_init);

static mac_dev_t mac_devices[MAX_MAC_DEVS] = {};

#ifdef __KERNEL__
/* For internal use only by proc interface */
int mac_devices_internal_index(mac_dev_t *mac_dev)
{
    uint32_t i;

    for (i = 0; i < MAX_MAC_DEVS; i++)
    {
        if (&mac_devices[i] == mac_dev)
            return i;
    }

    return -1;
}
EXPORT_SYMBOL(mac_devices_internal_index);
#endif

static mac_dev_t *mac_dev_get(mac_type_t mac_type, int mac_id)
{
    int i;
    mac_dev_t *mac_dev = NULL;

    for (i = 0; i < MAX_MAC_DEVS; i++)
    {
        if (!mac_devices[i].mac_drv)
            continue;

        if (mac_devices[i].mac_drv->mac_type != mac_type)
            continue;

        if (mac_devices[i].mac_id != mac_id)
            continue;

        mac_dev = &mac_devices[i];
        break;
    }

    return mac_dev;
}

mac_dev_t *mac_dev_add(mac_type_t mac_type, int mac_id, void *priv)
{
    uint32_t i;
    mac_drv_t *mac_drv = NULL;
    mac_dev_t *mac_dev = NULL;

    if (!(mac_drv = mac_drivers[mac_type]))
    {
        printk("Failed to find MAC driver: mac_type=%d\n", mac_type);
        return NULL;
    }

    if ((mac_dev = mac_dev_get(mac_type, mac_id)))
    {
        printk("Mac device already exists: %s:%d\n", mac_drv->name, mac_id);
        return NULL;
    }

    for (i = 0; i < MAX_MAC_DEVS && mac_devices[i].mac_drv != NULL; i++);

    if (i ==  MAX_MAC_DEVS)
    {
        printk("Failed adding mac device: %s:%d\n", mac_drv->name, mac_id);
        return NULL;
    }

    mac_dev = &mac_devices[i];
    
    mac_dev->mac_drv = mac_drv;
    mac_dev->mac_id = mac_id;
    mac_dev->priv = priv;

    if (mac_drv_dev_add(mac_dev))
    {
        printk("Failed to add MAC device to the driver: %s:%d\n", mac_drv->name, mac_id);
        mac_dev_del(mac_dev);
        return NULL;
    }

    return mac_dev;
}
EXPORT_SYMBOL(mac_dev_add);

int mac_dev_del(mac_dev_t *mac_dev)
{
    mac_drv_dev_del(mac_dev);
    memset(mac_dev, 0, sizeof(mac_dev_t));

    return 0;
}
EXPORT_SYMBOL(mac_dev_del);
