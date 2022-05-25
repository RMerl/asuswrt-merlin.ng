/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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
#include <linux/of.h>
#include <linux/i2c.h>
#include "bcmsfp.h"
#include <wan_drv.h>
#define MAX_I2C_BUS 32

extern int _sfp_mon_write(struct device *dev, enum bcmsfp_mon_attr attr, int channel, long value);

struct bus_to_sfp_t
{
    struct device *dev;
    void *opticaldet_desc;
    int is_pmd;

    int (*mac_trx_detect_cb)(void *opticaldet_desc);
    int (*mac_trx_remove_cb)(void);
    int force_tx_enable_before_detect; /* workaround for AE sfp: registered by _register_mac(), tell bcmsfp to enable tx power before i2c detection */

    int (*serdes_control)(void *opticaldet_desc, int mode);
    int wantop_control;
};
static struct bus_to_sfp_t bus_to_sfp[MAX_I2C_BUS];
static void *(*opticaldet_detect)(struct device *dev, int is_pmd);

static int transmitter_control(struct bus_to_sfp_t *b)
{
    if (!b->opticaldet_desc)
        return -ENODEV;

    if (b->serdes_control && b->serdes_control(b->opticaldet_desc, b->wantop_control))
        return -EINVAL;

    if (b->wantop_control == laser_off)
        return 0;

    if (b->dev)
        _sfp_mon_write(b->dev, bcmsfp_mon_tx_enable, 0, 1);

    return 0;
}

static int try_detect_enable(struct bus_to_sfp_t *b)
{
    if (opticaldet_detect && !b->opticaldet_desc)
        b->opticaldet_desc = opticaldet_detect(b->dev, b->is_pmd);

    if (!b->opticaldet_desc)
        return -1;

    if (b->mac_trx_detect_cb)
    {
        if (b->mac_trx_detect_cb(b->opticaldet_desc))
            b->wantop_control = mac_control;
    }

    transmitter_control(b);

    return 0;
}

void trxbus_register_mac(int bus, int force_tx_enable_before_detect, int (*detect_cb)(void *opticaldet_desc), int (*remove_cb)(void))
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return;

    b->mac_trx_detect_cb = detect_cb;
    b->mac_trx_remove_cb = remove_cb;
    b->force_tx_enable_before_detect = force_tx_enable_before_detect;

    try_detect_enable(b);
}
EXPORT_SYMBOL(trxbus_register_mac);

/* Called from bcmsfp to indicate module has been removed */
void trxbus_module_removed(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return;

    if (b->mac_trx_remove_cb)
        b->mac_trx_remove_cb();

    b->is_pmd = 0;
    b->opticaldet_desc = NULL;
    b->dev = NULL;
}

static int module_present(struct bus_to_sfp_t *b, struct device *dev, int is_pmd)
{
    b->dev = dev;
    b->is_pmd = is_pmd;
    b->wantop_control = laser_off;
    try_detect_enable(b);
    return 0;
}

/* Called from pmd/bcmsfp to indicate module is present: i2c comm. has been estabilished */
int trxbus_module_present(int bus, struct device *dev, int is_pmd)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return -1;
    return module_present(b, dev, is_pmd);
}
EXPORT_SYMBOL(trxbus_module_present);

/* Called from bcmsfp to indicate module is being probed */
int trxbus_module_probe(int bus, struct device *dev)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return -1;
    if (b->force_tx_enable_before_detect)
        _sfp_mon_write(dev, bcmsfp_mon_force_tx_power, 0, 1);

    return 0;
}

static void try_detect_enable_all(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(bus_to_sfp); i++)
        try_detect_enable(&bus_to_sfp[i]);
}

/* register opticaldet module; will trigger detection of existing trx */
void trxbus_register_detect(void *(*detect)(struct device *dev, int is_pmd))
{
    opticaldet_detect = detect;
    try_detect_enable_all();
}
EXPORT_SYMBOL(trxbus_register_detect);

void trxbus_register_serdes(int bus, int (*serdes_control)(void *opticaldet_desc, int mode))
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return;
    b->serdes_control = serdes_control;
}
EXPORT_SYMBOL(trxbus_register_serdes);

struct device *bcm_i2c_legacy_sfp_get_dev(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return NULL;
    return b->dev;
}
EXPORT_SYMBOL(bcm_i2c_legacy_sfp_get_dev);

/*XXX: can remove this function after all opticaldet_desc is passed by callbacks */
void *bcm_i2c_legacy_opticaldet_desc_get_dev(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return NULL;
    return b->opticaldet_desc;
}
EXPORT_SYMBOL(bcm_i2c_legacy_opticaldet_desc_get_dev);

void trxbus_transmitter_control(int bus, int mode)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    b->wantop_control = mode;
    transmitter_control(b);
}
EXPORT_SYMBOL(trxbus_transmitter_control);

void trxbus_mac_transmit_ready_set(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    b->wantop_control = mac_control;
    try_detect_enable(b);
}
EXPORT_SYMBOL(trxbus_mac_transmit_ready_set);

int trxbus_is_pmd(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return -1;
    return b->is_pmd;
}
EXPORT_SYMBOL(trxbus_is_pmd);

int trxbus_dt_bus_get(struct device_node *dn)
{
    struct device_node *i2c_node, *trx_node = of_parse_phandle(dn, "trx", 0);
    struct i2c_adapter *i2c_adap;
    int bus;

    if (!trx_node)
        return -EINVAL;

    i2c_node = of_parse_phandle(trx_node, "i2c-bus", 0);
    if (!i2c_node)
        return -EINVAL;

    i2c_adap = of_find_i2c_adapter_by_node(i2c_node);
    if (!i2c_adap)
        return -ENOMEM;

    of_node_put(i2c_node);
    bus = i2c_adap->nr;
    i2c_put_adapter(i2c_adap);

    return bus;
}
EXPORT_SYMBOL(trxbus_dt_bus_get);

