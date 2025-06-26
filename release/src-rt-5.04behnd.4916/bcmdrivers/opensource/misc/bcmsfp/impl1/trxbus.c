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
#include <wan_drv.h>
#include <trxbus.h>
#include "opticaldet.h" /* for TRX_SIG_ACTIVE_POLARITY defines */
#define MAX_I2C_BUS 32

struct bus_to_sfp_t
{
    struct sfp_data *psfp;
    void *opticaldet_desc;
    int module_present;  
    int is_pmd;

    int (*mac_trx_detect_cb)(int bus, void *opticaldet_desc);
    int (*mac_trx_remove_cb)(int bus);
    int force_tx_enable_before_detect; /* workaround for AE sfp: registered by _register_mac(), tell bcmsfp to enable tx power before i2c detection */

    int (*serdes_control)(TRX_SIG_ACTIVE_POLARITY lbe, TRX_SIG_ACTIVE_POLARITY tx_sd, int mode);
    int wantop_control;
    struct bcmsfp_ops *sfpops;
};
static struct bus_to_sfp_t bus_to_sfp[MAX_I2C_BUS];

#define sfp_to_bus(b) (int)((struct bus_to_sfp_t *)(b) - &bus_to_sfp[0])

static void *(*opticaldet_detect)(int bus);


static int serdes_control(struct bus_to_sfp_t *b)
{
    TRX_SIG_ACTIVE_POLARITY lbe_polarity, tx_sd_polarity;

    if (!b->serdes_control)
        return 0;

    _trx_get_lbe_polarity(b->opticaldet_desc, &lbe_polarity);
    _trx_get_tx_sd_polarity(b->opticaldet_desc, &tx_sd_polarity);

    return b->serdes_control(lbe_polarity, tx_sd_polarity, b->wantop_control);
}

static int transmitter_control(struct bus_to_sfp_t *b)
{
    if (!b->opticaldet_desc)
        return -ENODEV;

    if (serdes_control(b))
        return -EINVAL;

    if (b->wantop_control == laser_off)
        return 0;

    if (b->sfpops && b->sfpops->mon_write)
        b->sfpops->mon_write(b->psfp, bcmsfp_mon_tx_enable, 0, 1);

    return 0;
}

static int try_detect_enable(struct bus_to_sfp_t *b)
{
    if (opticaldet_detect && b->module_present && !b->opticaldet_desc)
        b->opticaldet_desc = opticaldet_detect(sfp_to_bus(b));

    if (!b->opticaldet_desc)
        return -1;

    if (b->mac_trx_detect_cb && b->mac_trx_detect_cb(sfp_to_bus(b), b->opticaldet_desc))
        b->wantop_control = mac_control;

    transmitter_control(b);

    return 0;
}

void trxbus_register_mac(int bus, int force_tx_enable_before_detect,
    int (*detect_cb)(int bus, void *opticaldet_desc), int (*remove_cb)(int bus))
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

void trxbus_unregister_mac(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return;

    if (b->mac_trx_remove_cb)
        b->mac_trx_remove_cb(bus);

    b->mac_trx_detect_cb = NULL;
}
EXPORT_SYMBOL(trxbus_unregister_mac);

/* Called from bcmsfp to indicate module has been removed */
void trxbus_module_removed(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return;

    if (b->opticaldet_desc && b->serdes_control)
        b->serdes_control(0, 0, laser_off);

    if (b->mac_trx_remove_cb)
        b->mac_trx_remove_cb(bus);

    b->wantop_control = laser_off;
    b->module_present = 0;
    b->is_pmd = 0;
    b->opticaldet_desc = NULL;
}

/* Called from pmd/bcmsfp to indicate module is present: i2c comm. has been estabilished */
int trxbus_module_present(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return -1;

    b->module_present = 1;
    b->wantop_control = laser_off;
    return try_detect_enable(b);
}
EXPORT_SYMBOL(trxbus_module_present);

/* Called from sfp driver probe. setup the dev pointer as upper driver need certain cage
   property before the SFP module is actually plugged in
 */
int trxbus_module_init(int bus, struct sfp_data *psfp, struct bcmsfp_ops *ops, int is_pmd)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus < 0 || bus >= ARRAY_SIZE(bus_to_sfp))
        return -1;
    b->psfp = psfp;
    b->sfpops = ops;
    b->is_pmd = is_pmd;
    return 0;
}
EXPORT_SYMBOL(trxbus_module_init);

/* Called from bcmsfp to indicate module is being probed */
int trxbus_module_probe(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return -1;
    if (b->force_tx_enable_before_detect)
        b->sfpops->mon_write(b->psfp, bcmsfp_mon_force_tx_power, 0, 1);

    return 0;
}

static void try_detect_enable_all(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(bus_to_sfp); i++)
        try_detect_enable(&bus_to_sfp[i]);
}

/* register opticaldet module; will trigger detection of existing trx */
void trxbus_register_detect(void *(*detect)(int bus))
{
    opticaldet_detect = detect;
    try_detect_enable_all();
}
EXPORT_SYMBOL(trxbus_register_detect);

void trxbus_register_serdes(int bus, int (*serdes_control)(TRX_SIG_ACTIVE_POLARITY lbe, TRX_SIG_ACTIVE_POLARITY tx_sd, int mode))
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return;
    b->serdes_control = serdes_control;
}
EXPORT_SYMBOL(trxbus_register_serdes);

/*XXX: can remove this function after all opticaldet_desc is passed by callbacks */
void *trxbus_opticaldet_desc_get(int bus)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus >= ARRAY_SIZE(bus_to_sfp))
        return NULL;
    return b->opticaldet_desc;
}
EXPORT_SYMBOL(trxbus_opticaldet_desc_get);

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

int trxbus_module_read_raw(int bus, int addr, int offset, uint8_t *data, int len)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];

    if (bus < 0 || bus >= ARRAY_SIZE(bus_to_sfp))
    {
        printk("TRXBUS: bus %d is not valid\n", bus);
        return -1;
    }
    if (b->sfpops && b->sfpops->mon_read_raw)
        return b->sfpops->mon_read_raw(b->psfp, addr, offset, data, len);
    return -1;
}
EXPORT_SYMBOL(trxbus_module_read_raw);

int trxbus_mon_read(int bus, enum bcmsfp_mon_attr attr, int channel, long *value)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];
    if (bus < 0 || bus >= ARRAY_SIZE(bus_to_sfp))
    {
        printk("TRXBUS: bus %d is not valid\n", bus);
        return -1;
    }
    if (b->sfpops && b->sfpops->mon_read)
        return b->sfpops->mon_read(b->psfp, attr, channel, value);
    return -1;
}
EXPORT_SYMBOL(trxbus_mon_read);

int trxbus_mon_read_buf(int bus, enum bcmsfp_mon_attr attr, int channel, char **buf, int *len)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];
    if (bus < 0 || bus >= ARRAY_SIZE(bus_to_sfp))
    {
        printk("TRXBUS: bus %d is not valid\n", bus);
        return -1;
    }
    if (b->sfpops && b->sfpops->mon_read_buf)
        return b->sfpops->mon_read_buf(b->psfp, attr, channel, buf, len);
    return -1;
}
EXPORT_SYMBOL(trxbus_mon_read_buf);

int trxbus_mon_write(int bus, enum bcmsfp_mon_attr attr, int channel, long value)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];
    if (bus < 0 || bus >= ARRAY_SIZE(bus_to_sfp))
    {
        printk("TRXBUS: bus %d is not valid\n", bus);
        return -1;
    }
    if (b->sfpops && b->sfpops->mon_write)
        return b->sfpops->mon_write(b->psfp, attr, channel, value);
    return -1;
}
EXPORT_SYMBOL(trxbus_mon_write);

int trxbus_mon_write_buf(int bus, enum bcmsfp_mon_attr attr, int channel, char *buf, int len)
{
    struct bus_to_sfp_t *b = &bus_to_sfp[bus];
    if (bus < 0 || bus >= ARRAY_SIZE(bus_to_sfp))
    {
        printk("TRXBUS: bus %d is not valid\n", bus);
        return -1;
    }
    if (b->sfpops && b->sfpops->mon_write_buf)
        return b->sfpops->mon_write_buf(b->psfp, attr, channel, buf, len);
    return -1;
}
EXPORT_SYMBOL(trxbus_mon_write_buf);

