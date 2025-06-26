/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

/*
 *  Created on: Nov/2019
 *      Author: ido.brezel@broadcom.com
 */

// #define DEBUG
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/gpio/consumer.h>
#include <bcm_bca_extintr.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of.h>
#include "trxbus.h"

struct sfp_eeprom_base
{
    u8 phys_id;
    u8 phys_ext_id;
    u8 connector;
    u8 cc[8];
    u8 more_stuff[9]; /* XXX: fill this */
    char vendor_name[16];
    u8 extended_cc;
    char vendor_oui[3];
    char vendor_pn[16];
    union {
        struct {
            char vendor_rev[4];
            union {
                __be16 optical_wavelength;
                __be16 cable_compliance;
            } __packed passive;
        } __packed sfp;
        struct {
            char vendor_rev[2];
            __be16 optical_wavelength;
            __be16 optical_wavelength_tolerance;
        } __packed xfp;
    } __packed;
    u8 reserved62;
    u8 cc_base;
} __packed;

struct sfp_eeprom_ext
{
    __be16 options;
    u8 br_max;
    u8 br_min;
    char vendor_sn[16];
    char datecode[8];
    u8 diagmon;
    u8 enhopts;
    u8 sff8472_compliance;
    u8 cc_ext;
} __packed;

/* raw SFP module identification information:
 * @base SFP module identification structure
 * @exta extended SFP module identification structure
 *
 * See the SFF-8472 specification and related documents for the definition
 * of these structure members. This can be obtained from
 * ftp://ftp.seagate.com/sff
 */
struct sfp_eeprom
{
    union {
        struct sfp_eeprom_base base;
        u8 baseraw[64];
    };
    union {
        struct sfp_eeprom_ext ext;
        u8 extraw[32];
    };
    u8 vendorraw[32];
    u8 sff8079rsvd[128];
} __packed;

enum {
    SFP_E_INSERT,
    SFP_E_REMOVE,
    SFP_E_TIMEOUT,

    SFP_MOD_REMOVED = 0,
    SFP_MOD_PROBING,
    SFP_MOD_INSERTED,
    SFP_MOD_ERROR,
};

#define T_PROBE_INIT msecs_to_jiffies(500)
#define T_PROBE_RETRY msecs_to_jiffies(1000)
#define T_PROBE_RETRY_NUM 100

typedef enum
{
    GPIO_MODDEF0,
    GPIO_LOS,
    GPIO_TX_FAULT,
    GPIO_TX_DISABLE,
    GPIO_TX_POWER_DOWN,
    GPIO_TX_POWER,
    GPIO_RX_POWER,
    GPIO_MAX,
}GPIOTypes;

/**
 * @brief defines gpio component
 */
struct gpiod_component{
    /**
     * @brief  name of gpio component as it defined in xXx.dts file
     */
    char *name;
    /**
     * @brief defines direction i.e:xxx_IN for input & XXX_ASIS for HW default
     */
    enum gpiod_flags flag;
};

struct sfp_data
{
    struct device *dev;
    struct i2c_adapter *i2c;

    unsigned int (*get_present)(struct sfp_data *);

    struct gpio_desc *gpio[GPIO_MAX];
    struct pinctrl_state *txsd_state;
    int rxsd_pin;
    int mod_state;
    int present;
    int retries_left;
    struct sfp_eeprom eeprom;
    struct delayed_work dwork;
    struct delayed_work dwork_poll;
    struct delayed_work dwork_irq;
    struct mutex st_mutex;
    struct mutex sm_mutex;

    int has_fixed_attribues;

    int force_tx_power;
};

enum { sff_data, sfp_data };

/************** F O R W A R D  D E C L A R A T I O N S ************/
static int sfp_mon_read(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, long *value);
static int sfp_mon_read_buf(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, char **buf, int *len);
static int sfp_mon_write(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, long value);
static int sfp_mon_write_buf(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, char *buf, int len);
static int sfp_mon_read_eeprom_raw(struct sfp_data *psfp, int addr, int reg, uint8_t *data, int len);

/*********************** S T A T I C  D A T A *********************/
/**
 * @brief list of supported gpio components that may be resident in SFP
 */
static const struct gpiod_component gpio_components[GPIO_MAX] = 
{
   { .name = "mod-def0",      .flag = GPIOD_IN  },  
   { .name = "los",           .flag = GPIOD_IN  },
   { .name = "tx-fault",      .flag = GPIOD_IN  },
   { .name = "tx-disable",    .flag = GPIOD_ASIS},
   { .name = "tx-power-down", .flag = GPIOD_ASIS},
   { .name = "tx-power",      .flag = GPIOD_ASIS},
   { .name = "rx-power",      .flag = GPIOD_ASIS},
};
static unsigned long poll_jiffies;
static struct bcmsfp_ops sfp_ops = {
    .mon_read = sfp_mon_read,
    .mon_read_buf = sfp_mon_read_buf,
    .mon_write = sfp_mon_write,
    .mon_write_buf = sfp_mon_write_buf,
    .mon_read_raw = sfp_mon_read_eeprom_raw,
};

static const struct of_device_id of_platform_sfp_table[] = {
    { .compatible = "brcm,sfp", .data = (void *)sfp_data, },
    { .compatible = "brcm,sff", .data = (void *)sff_data, },
    { /* end of list */ },
};

static const char * const event_strings[] = {
    [SFP_E_INSERT] = "insert",
    [SFP_E_REMOVE] = "remove",
    [SFP_E_TIMEOUT] = "timeout",
};

/******************************************************************/
static const char *event_to_str(unsigned short event)
{
    if (event >= ARRAY_SIZE(event_strings))
        return "Unknown event";
    return event_strings[event];
}

static const char *const mod_state_strings[] = {
    [SFP_MOD_REMOVED] = "empty",
    [SFP_MOD_PROBING] = "probe",
    [SFP_MOD_INSERTED] = "present",
    [SFP_MOD_ERROR] = "error",
};

static const char *mod_state_to_str(unsigned short mod_state)
{
    if (mod_state >= ARRAY_SIZE(mod_state_strings))
        return "Unknown module state";
    return mod_state_strings[mod_state];
}

static int i2c_write(struct sfp_data *psfp, u8 bus_addr, u8 dev_addr, void *buf, size_t len)
{
    struct i2c_msg msgs[1];
    int ret;

    msgs[0].addr = bus_addr;
    msgs[0].flags = 0;
    msgs[0].len = 1 + len;
    msgs[0].buf = kmalloc(1 + len, GFP_KERNEL);
    if (!msgs[0].buf)
        return -ENOMEM;

    msgs[0].buf[0] = dev_addr;
    memcpy(&msgs[0].buf[1], buf, len);

    ret = i2c_transfer(psfp->i2c, msgs, ARRAY_SIZE(msgs));

    kfree(msgs[0].buf);

    if (ret < 0)
        return ret;

    return ret == ARRAY_SIZE(msgs) ? len : 0;
}

static int i2c_read(struct sfp_data *psfp, u8 addr, u8 dev_addr, void *buf, int len)
{
    struct i2c_msg msg[2];
    int ret;
    size_t this_len;

    msg[0].addr = addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &dev_addr; /* Offset to read from */

    msg[1].addr = addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    while (len)
    {
        this_len = len;
        if (this_len > 16)
            this_len = 16;

        msg[1].len = this_len;

        ret = i2c_transfer(psfp->i2c, msg, ARRAY_SIZE(msg));
        if (ret < 0)
            return ret;

        if (ret != ARRAY_SIZE(msg))
            break;

        msg[1].buf += this_len;
        dev_addr += this_len;
        len -= this_len;
    }

    return msg[1].buf - (u8 *)buf;
}

static int sfp_module_supported(struct sfp_data *psfp, char *buf, int size)
{
    return 1; /* TODO: */
}

static enum bcmsfp_mon_attr get_mon_ext_attr(struct device_attribute *attr)
{
    return (enum bcmsfp_mon_attr)container_of(attr, struct dev_ext_attribute, attr)->var;
}

static int sfp_mon_read_buf(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, char **buf, int *len);
static ssize_t _show_buf(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    char *s;
    int ret, len = 0;
    enum bcmsfp_mon_attr attr = get_mon_ext_attr(dev_attr);
    struct sfp_data *psfp = dev_get_drvdata(dev);

    ret = sfp_mon_read_buf(psfp, attr, 0, &s, &len);
    if (ret < 0)
        return ret;

    /* When len is not set - be cautious with non-NULL terminated buffers! */
    if (len)
    {
        len = snprintf(buf, len + 1, "%s", s);
        return len + sprintf(buf + len, "\n");
    }
    else
        return sprintf(buf, "%s\n", s);
}

static int sfp_mon_write_buf(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, char *buf, int count);
static ssize_t _store_buf(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    int ret;
    enum bcmsfp_mon_attr attr = get_mon_ext_attr(dev_attr);
    struct sfp_data *psfp = dev_get_drvdata(dev);

    if ((ret = sfp_mon_write_buf(psfp, attr, 0, (char *)buf, count)))
        return ret;

    return count;
}

static int sfp_mon_read(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, long *value);
static ssize_t _show(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    long v;
    int ret;
    enum bcmsfp_mon_attr attr = get_mon_ext_attr(dev_attr);
    struct sfp_data *psfp = dev_get_drvdata(dev);

    ret = sfp_mon_read(psfp, attr, 0, &v);
    if (ret < 0)
        return ret;

    return sprintf(buf, "%ld\n", v) + 1;
}

static int sfp_mon_write(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, long value);
static ssize_t _store(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
    long v;
    int ret;
    enum bcmsfp_mon_attr attr = get_mon_ext_attr(dev_attr);
    struct sfp_data *psfp = dev_get_drvdata(dev);

    ret = kstrtol(buf, 10, &v);
    if (ret)
        return ret;

    if ((ret = sfp_mon_write(psfp, attr, 0, v)) < 0)
        return ret;

    return count;
}

#define DEVICE_SFP_MON_ATTR(_name, _mode, _var) \
    static struct dev_ext_attribute dev_attr_##_name = \
        { __ATTR(_name, _mode, _show, _store), (void *)(_var) }
#define DEVICE_SFP_MON_STR_ATTR(_name, _mode, _var) \
    static struct dev_ext_attribute dev_attr_##_name = \
        { __ATTR(_name, _mode, _show_buf, _store_buf), (void *)(_var) }

/* SFP status & properties */
DEVICE_SFP_MON_ATTR(has_txsd, 0644, bcmsfp_mon_has_txsd);
DEVICE_SFP_MON_ATTR(rxsd_pin, 0644, bcmsfp_mon_rxsd_pin);
DEVICE_SFP_MON_ATTR(present, 0644, bcmsfp_mon_present);
DEVICE_SFP_MON_ATTR(los, 0644, bcmsfp_mon_los);
DEVICE_SFP_MON_ATTR(tx_enable, 0644, bcmsfp_mon_tx_enable);
DEVICE_SFP_MON_ATTR(rx_enable, 0644, bcmsfp_mon_rx_enable);
DEVICE_SFP_MON_ATTR(tx_power_down, 0644, bcmsfp_mon_tx_power_down);
DEVICE_SFP_MON_ATTR(force_tx_power, 0644, bcmsfp_mon_force_tx_power);
DEVICE_SFP_MON_ATTR(trx_compliance, 0644, bcmsfp_mon_trx_compliance);

/* EEPROM ID */
DEVICE_SFP_MON_ATTR(id_phys_id, 0644, bcmsfp_mon_id_phys_id);
DEVICE_SFP_MON_ATTR(id_phys_ext_id, 0644, bcmsfp_mon_id_phys_ext_id);
DEVICE_SFP_MON_ATTR(id_connector, 0644, bcmsfp_mon_id_connector);
DEVICE_SFP_MON_STR_ATTR(id_vendor_name, 0644, bcmsfp_mon_id_vendor_name);
DEVICE_SFP_MON_STR_ATTR(id_vendor_pn, 0644, bcmsfp_mon_id_vendor_pn);
DEVICE_SFP_MON_STR_ATTR(id_vendor_rev, 0644, bcmsfp_mon_id_vendor_rev);
DEVICE_SFP_MON_STR_ATTR(id_vendor_sn, 0644, bcmsfp_mon_id_vendor_sn);
DEVICE_SFP_MON_STR_ATTR(id_date_code, 0644, bcmsfp_mon_id_date_code);
DEVICE_SFP_MON_ATTR(id_optical_wavelength, 0644, bcmsfp_mon_id_optical_wavelength);

/* EEPROM HW monitoring */
DEVICE_SFP_MON_ATTR(mon_rx_power, 0644, bcmsfp_mon_rx_power);
DEVICE_SFP_MON_ATTR(mon_tx_power, 0644, bcmsfp_mon_tx_power);
DEVICE_SFP_MON_ATTR(mon_temp, 0644, bcmsfp_mon_temp);
DEVICE_SFP_MON_ATTR(mon_vcc, 0644, bcmsfp_mon_vcc);
DEVICE_SFP_MON_ATTR(mon_bias_current, 0644, bcmsfp_mon_bias_current);
DEVICE_SFP_MON_ATTR(mon_xfp_rx_channel, 0644, bcmsfp_mon_xfp_rx_channel);
DEVICE_SFP_MON_ATTR(mon_xfp_tx_channel, 0644, bcmsfp_mon_xfp_tx_channel);
DEVICE_SFP_MON_STR_ATTR(mon_xfp_password, 0644, bcmsfp_mon_xfp_password);

/* Ethernet PHY register monitoring */
DEVICE_SFP_MON_STR_ATTR(mon_phy_reg, 0644, bcmsfp_mon_phy_reg);

static struct attribute *sfp_fixed_attributes[] = {
    &dev_attr_present.attr.attr,
    &dev_attr_los.attr.attr,
    &dev_attr_tx_enable.attr.attr,
    &dev_attr_rx_enable.attr.attr,
    &dev_attr_tx_power_down.attr.attr,
    &dev_attr_force_tx_power.attr.attr,
    &dev_attr_has_txsd.attr.attr,
    &dev_attr_rxsd_pin.attr.attr,
    NULL,
};

static struct attribute *sfp_mon_attributes[] = {
    &dev_attr_trx_compliance.attr.attr,

    &dev_attr_id_phys_id.attr.attr,
    &dev_attr_id_phys_ext_id.attr.attr,
    &dev_attr_id_connector.attr.attr,
    &dev_attr_id_vendor_name.attr.attr,
    &dev_attr_id_vendor_pn.attr.attr,
    &dev_attr_id_vendor_rev.attr.attr,
    &dev_attr_id_vendor_sn.attr.attr,
    &dev_attr_id_date_code.attr.attr,
    &dev_attr_id_optical_wavelength.attr.attr,

    &dev_attr_mon_rx_power.attr.attr,
    &dev_attr_mon_tx_power.attr.attr,
    &dev_attr_mon_temp.attr.attr,
    &dev_attr_mon_vcc.attr.attr,
    &dev_attr_mon_bias_current.attr.attr,

    /* XXX: Should register these only for XFP */
    &dev_attr_mon_xfp_rx_channel.attr.attr,
    &dev_attr_mon_xfp_tx_channel.attr.attr,
    &dev_attr_mon_xfp_password.attr.attr,

    &dev_attr_mon_phy_reg.attr.attr,
    NULL,
};

static const struct attribute_group bcmsfp_fixed_attribute_group = {
    .attrs = sfp_fixed_attributes,
};

static const struct attribute_group bcmsfp_attribute_group = {
    .attrs = sfp_mon_attributes,
};

static void sfp_mon_remove(struct sfp_data *psfp)
{
    sysfs_remove_group(&psfp->dev->kobj, &bcmsfp_attribute_group);
}

static int sfp_mon_insert(struct sfp_data *psfp)
{
    return sysfs_create_group(&psfp->dev->kobj, &bcmsfp_attribute_group);
}

static int xfp_table_select(struct sfp_data *psfp)
{
    unsigned char table = 1;
    int ret;

#define TRX_XFP_EEPROM_PAGE_SELECT 127
    if ((ret = i2c_write(psfp, XFP_I2C_ADDR, TRX_XFP_EEPROM_PAGE_SELECT, &table, 1)) < 0)
        return ret;

    dev_info(psfp->dev, "selected page %d\n", table);
    return 0;
}

static int probe_i2c(struct sfp_data *psfp)
{
    struct sfp_eeprom eeprom;
    int ret, eeprom_base = 0, addr = SFP_I2C_A0_ADDR;

    if ((ret = i2c_read(psfp, SFP_I2C_A0_ADDR, 0, &eeprom, 1)) < 0)
    {
        dev_dbg(psfp->dev, "failed to read EEPROM %d\n", ret);
        return -EAGAIN;
    }

#define PHYS_ID_XFP 0x6 /* INF 8077 */
    if (eeprom.base.phys_id == PHYS_ID_XFP)
    {
        eeprom_base = 128;
        if (xfp_table_select(psfp))
            return -EAGAIN;
    }

    if ((ret = i2c_read(psfp, addr, eeprom_base, &eeprom, sizeof(eeprom))) < 0)
    {
        dev_dbg(psfp->dev, "failed to read EEPROM %d\n", ret);
        return -EAGAIN;
    }

    if (ret != sizeof(eeprom))
    {
        dev_err(psfp->dev, "EEPROM short read: %d\n", ret);
        return -EAGAIN;
    }

    dev_info(psfp->dev, "module %.*s %.*s rev %.*s sn %.*s dc %.*s\n",
        (int)sizeof(eeprom.base.vendor_name), eeprom.base.vendor_name,
        (int)sizeof(eeprom.base.vendor_pn), eeprom.base.vendor_pn,
        (psfp->eeprom.base.phys_id == PHYS_ID_XFP) ? (int)sizeof(eeprom.base.xfp.vendor_rev) : (int)sizeof(eeprom.base.sfp.vendor_rev),
        (psfp->eeprom.base.phys_id == PHYS_ID_XFP) ? eeprom.base.xfp.vendor_rev : eeprom.base.sfp.vendor_rev,
        (int)sizeof(eeprom.ext.vendor_sn), eeprom.ext.vendor_sn,
        (int)sizeof(eeprom.ext.datecode), eeprom.ext.datecode);

    if (!sfp_module_supported(psfp, (char *)&eeprom, sizeof(eeprom)))
    {
        dev_err(psfp->dev, "module is not supported\n");
        return -EINVAL;
    }

    if (eeprom.ext.diagmon & SFP_DIAGMON_ADDRMODE)
        dev_warn(psfp->dev,
             "module address swap to access page 0xA2 is not supported.\n");
	
    psfp->eeprom = eeprom;

    sfp_mon_insert(psfp);

    return 0;
}

static unsigned int sff_get_present(struct sfp_data *psfp)
{
    return 1;
}

static unsigned int sfp_i2c_get_present(struct sfp_data *psfp)
{
    char buf;
#define SFP_PROBE_READ_BYTES 1

    return i2c_read(psfp, SFP_I2C_A0_ADDR, 0, &buf, SFP_PROBE_READ_BYTES) == SFP_PROBE_READ_BYTES ? buf : 0;
}

static unsigned int sfp_gpio_get_present(struct sfp_data *psfp)
{
    int v;

    v = gpiod_get_value_cansleep(psfp->gpio[GPIO_MODDEF0]);
    if (v < 0)
        dev_err(psfp->dev, "gpiod_get_value_cansleep failed %d %p\n", v, psfp->gpio[GPIO_MODDEF0]);

    return v;
}

static void sfp_sm_next(struct sfp_data *psfp, int mod_state, int timeout)
{
    dev_dbg(psfp->dev, "SM next: new state %s timeout %d\n", mod_state_to_str(mod_state), timeout);

    psfp->mod_state = mod_state;
    if (timeout)
        mod_delayed_work(system_power_efficient_wq, &psfp->dwork, timeout); 
    else
        cancel_delayed_work(&psfp->dwork);
}

static int _sfp_module_gpio_get(struct sfp_data *psfp, int gpio)
{
    if (psfp->gpio[gpio])
        return gpiod_get_value(psfp->gpio[gpio]);

    return -1;
}
static void _sfp_module_gpio_set(struct sfp_data *psfp, int gpio, int value)
{
    if (psfp->gpio[gpio])
    {
        gpiod_direction_output(psfp->gpio[gpio], value);
    }
}
static int sfp_module_tx_power_get(struct sfp_data *psfp)
{
    return _sfp_module_gpio_get(psfp, GPIO_TX_POWER);
}
static int sfp_module_rx_power_get(struct sfp_data *psfp)
{
    return _sfp_module_gpio_get(psfp, GPIO_RX_POWER);
}

static void sfp_module_power_set(struct sfp_data *psfp, GPIOTypes type, int value)
{
    _sfp_module_gpio_set(psfp, type, value);
}

static void sfp_module_tx_disable_set(struct sfp_data *psfp, int value)
{
    _sfp_module_gpio_set(psfp, GPIO_TX_DISABLE, value);
}

static void sfp_sm_mod_present(struct sfp_data *psfp)
{
    trxbus_module_present(psfp->i2c->nr);
}

static void sfp_sm_mod_remove(struct sfp_data *psfp)
{
    sfp_mon_remove(psfp);

    if (!psfp->force_tx_power)
        sfp_module_power_set(psfp, GPIO_TX_POWER, 0);

    trxbus_module_removed(psfp->i2c->nr);
    memset(&psfp->eeprom, 0, sizeof(struct sfp_eeprom));

    dev_info(psfp->dev, "module removed\n");
}

static void sfp_sm_event(struct sfp_data *psfp, int event)
{
    mutex_lock(&psfp->sm_mutex);
    dev_dbg(psfp->dev, "SM event: current state %s event %s\n", mod_state_to_str(psfp->mod_state), event_to_str(event));

    switch (psfp->mod_state)
    {
        case SFP_MOD_PROBING:
            if (event == SFP_E_REMOVE)
            {
                sfp_sm_next(psfp, SFP_MOD_REMOVED, 0);
            }
            if (event == SFP_E_TIMEOUT)
            {
                int val = probe_i2c(psfp);

                trxbus_module_probe(psfp->i2c->nr);

                psfp->retries_left--;
                if (val == 0)
                {
                    sfp_sm_next(psfp, SFP_MOD_INSERTED, 0);
                    sfp_sm_mod_present(psfp);
                }
                else if (val != -EAGAIN)
                {
                    /* Error, do nothing */
                    sfp_sm_next(psfp, SFP_MOD_ERROR, 0);
                }
                else if (psfp->retries_left <= 0)
                {
                    sfp_sm_next(psfp, SFP_MOD_REMOVED, 0);
                    dev_info(psfp->dev, "Failed to detect SFP: %d retries exhausted\n", T_PROBE_RETRY_NUM);
                }
                else
                    sfp_sm_next(psfp, SFP_MOD_PROBING, T_PROBE_RETRY);
            }
            break;
        case SFP_MOD_INSERTED:
        case SFP_MOD_ERROR:
            if ((event == SFP_E_REMOVE) || (event == SFP_E_INSERT))
            {
                sfp_sm_mod_remove(psfp); 
                sfp_sm_next(psfp, SFP_MOD_REMOVED, 0);
            }
            if (event != SFP_E_INSERT)
                break;
            /* fallthru only if INSERT */
             __attribute__((__fallthrough__));
        default:
            if (event == SFP_E_INSERT)
            {
                psfp->retries_left = T_PROBE_RETRY_NUM;
                dev_info(psfp->dev, "module inserted\n");
                sfp_module_power_set(psfp, GPIO_RX_POWER, 1);
                sfp_sm_next(psfp, SFP_MOD_PROBING, T_PROBE_INIT);
            }
            break;
    }

    mutex_unlock(&psfp->sm_mutex);
}

static void check_sfp_present(struct sfp_data *psfp)
{
    int new_present, present_changed;

    mutex_lock(&psfp->st_mutex);

    new_present = psfp->get_present(psfp);
    present_changed = new_present != psfp->present;
    psfp->present = new_present;

    if (present_changed)
        sfp_sm_event(psfp, new_present ? SFP_E_INSERT : SFP_E_REMOVE);

    mutex_unlock(&psfp->st_mutex);
}

static void sfp_timeout(struct work_struct *work_arg)
{
    struct sfp_data *psfp = container_of(work_arg, struct sfp_data, dwork.work);

    sfp_sm_event(psfp, SFP_E_TIMEOUT);
}

static void sfp_poll(struct work_struct *work_arg)
{
    struct sfp_data *psfp = container_of(work_arg, struct sfp_data, dwork_poll.work);

    check_sfp_present(psfp);
    mod_delayed_work(system_wq, &psfp->dwork_poll, poll_jiffies);
}

static void sfp_cb_irq(struct work_struct *work_arg)
{
    struct sfp_data *psfp = container_of(work_arg, struct sfp_data, dwork_irq.work);

    check_sfp_present(psfp);
}

static irqreturn_t sfp_isr(int irq, void *arg)
{
    struct sfp_data *psfp = arg;

    bcm_bca_extintr_mask(irq);

    dev_dbg(psfp->dev, "sfp_isr interrupt called irq %d!\n", irq);
    mod_delayed_work(system_power_efficient_wq, &psfp->dwork_irq, msecs_to_jiffies(1)); 

    bcm_bca_extintr_clear(irq);
    bcm_bca_extintr_unmask(irq);

    return IRQ_HANDLED;
}

static struct sfp_data *sfp_data_alloc(struct device *dev)
{
    struct sfp_data *psfp;

    psfp = kzalloc(sizeof(*psfp), GFP_KERNEL);
    if (!psfp)
        return ERR_PTR(-ENOMEM);

    psfp->dev = dev;
    psfp->txsd_state = ERR_PTR(-ENODEV);
    psfp->rxsd_pin = -1;

    mutex_init(&psfp->st_mutex);
    mutex_init(&psfp->sm_mutex);
    INIT_DELAYED_WORK(&psfp->dwork, sfp_timeout);
    INIT_DELAYED_WORK(&psfp->dwork_poll, sfp_poll);
    INIT_DELAYED_WORK(&psfp->dwork_irq, sfp_cb_irq);

    return psfp;
}

static int i2c_configure(struct sfp_data *psfp, struct device_node *sfp_node)
{
    struct device_node *i2c_np;

    i2c_np = of_parse_phandle(sfp_node, "i2c-bus", 0);
    if (!i2c_np)
    {
        dev_err(psfp->dev, "missing 'i2c-bus' property\n");
        return -ENODEV;
    }

    psfp->i2c = of_find_i2c_adapter_by_node(i2c_np);
    of_node_put(i2c_np);
    if (!psfp->i2c)
        return -EPROBE_DEFER;

    if (!i2c_check_functionality(psfp->i2c, I2C_FUNC_I2C))
        return -EINVAL;

    return 0;
}

static void sfp_cleanup(void *data)
{
    struct sfp_data *psfp = data;

    if (psfp->has_fixed_attribues)
        sysfs_remove_group(&psfp->dev->kobj, &bcmsfp_fixed_attribute_group);
    cancel_delayed_work_sync(&psfp->dwork);
    cancel_delayed_work_sync(&psfp->dwork_poll);
    cancel_delayed_work_sync(&psfp->dwork_irq);
    if (psfp->i2c)
        i2c_put_adapter(psfp->i2c);
    kfree(psfp);
}

static int sfp_get_rxsd_pin(struct device_node *np)
{
    struct device_node *pinctrl_np = NULL;
    const char *pin_name[4];
    char pinctr[16];
    int count = 0, pin = -1;
    int i;

    count = of_property_read_string_array(np, "pinctrl-names", pin_name, 4);
    for (i = 0; i < count; i++) {
        if (strncmp(pin_name[i], "rx-sd", 5) == 0)
            break;
    }

    if (i == count) {
        return pin;
    }

    snprintf(pinctr, 16, "pinctrl-%d", i);
    pinctrl_np = of_parse_phandle(np, pinctr, 0);
    if (pinctrl_np) {
        of_property_read_u32(pinctrl_np, "pins", &pin);
        of_node_put(pinctrl_np);
    }

    return pin;
}

static int _probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct sfp_data *psfp;
    int ret, i, poll = false;
    const struct of_device_id *of_id;
    struct pinctrl *pctrl;
    char *mode;

    if (!np)
        return -ENODEV;

    psfp = sfp_data_alloc(&pdev->dev);
    if (IS_ERR(psfp))
        return PTR_ERR(psfp);

    platform_set_drvdata(pdev, psfp);
    ret = devm_add_action(psfp->dev, sfp_cleanup, psfp);
    if (ret < 0)
        return ret;

    of_id = of_match_node(of_platform_sfp_table, np);
    if (WARN_ON(!of_id))
        return -EINVAL;

    pctrl = devm_pinctrl_get(dev);
    if (IS_ERR(pctrl))
    {
        if (pctrl != ERR_PTR(-ENODEV))
        {
            dev_err(dev, "Failed to get pinctrl\n");
            return PTR_ERR(pctrl);
        }
    }
    else {
        psfp->txsd_state = pinctrl_lookup_state(pctrl, "tx-sd");
        psfp->rxsd_pin = sfp_get_rxsd_pin(np);
    }

#if defined(CONFIG_BCM_PON)
    /* this code is not needed after PON board dts move tx-sd to default
       state and have a dedicated pinctrl-n for tx-sd and other pins */
    if (psfp->txsd_state != ERR_PTR(-ENODEV) && (ret = pinctrl_select_state(pctrl, psfp->txsd_state)))
        return ret;
#endif

    if ((ret = i2c_configure(psfp, np)))
        return ret;
    if ((ret = sysfs_create_group(&psfp->dev->kobj, &bcmsfp_fixed_attribute_group)))
    {
        dev_err(dev, "Failed to register sysfs (%d)\n", ret);
        return ret;
    }
    psfp->has_fixed_attribues = 1;
    mutex_lock(&psfp->st_mutex);
    for (i = 0; i < GPIO_MAX; i++)
    {
        psfp->gpio[i] = devm_gpiod_get_optional(dev, gpio_components[i].name, gpio_components[i].flag);
        if (IS_ERR(psfp->gpio[i]))
        {
            ret = PTR_ERR(psfp->gpio[i]);
            goto label_unlock_return;
        }
        if (psfp->gpio[i] && gpio_components[i].flag == GPIOD_IN)
        {
            gpiod_direction_input(psfp->gpio[i]);
            if (i == GPIO_MODDEF0) /* Don't want I2C polling for other than module present check */
                poll = true;

            continue;
        }

        if (gpio_components[i].flag == GPIOD_IN)
        {
            ret = bcm_bca_extintr_request(dev, NULL, gpio_components[i].name, sfp_isr, psfp, dev_name(dev), NULL);
            if (ret == -ENOENT)
            {
                if (i == GPIO_MODDEF0) /* Don't want I2C polling for other than module present check */
                    poll = true;

                continue;
            }

            if (ret < 0)
            {
                dev_err(dev, "bcm_bca_extintr_request failed %d\n", ret);
                goto label_unlock_return;
            }

            psfp->gpio[i] = bcm_bca_extintr_get_gpiod(ret);
        }
    }

    sfp_module_power_set(psfp, GPIO_TX_POWER, 0);
    sfp_module_tx_disable_set(psfp, 1);
    psfp->mod_state = SFP_MOD_REMOVED;

    psfp->present = 0;
    /* Default present state checks MODDEF0 gpio */
    psfp->get_present = sfp_gpio_get_present;
    mode = "gpio";

    /* Modules with MODDEF0 / connected to GPIO without interrupt capability are detected by polling */
    if (poll)
    {
        /* If MODDEF0 is not specified, poll for module existance via i2c, otherwise default to polling MODDEF0 */
        if (!psfp->gpio[GPIO_MODDEF0])
        {
            mode = "i2c";
            psfp->get_present = sfp_i2c_get_present;
        }
    }
    /* SFF modules always present */
    if (of_id->data == sff_data)
    {
        psfp->get_present = sff_get_present;
        mode = "sff";
    }

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
    /* Workaround for BCM96858XREF boardid, which has either SFF or SFP on same boardid:
     * meaning it has MODDEF0 declared in SFF boardid, but not connected to it.
     * Also for 6856REF*2, that has separate PON_PRESENT signals for XFP and SFP. */
    if (psfp->gpio[GPIO_MODDEF0] && !sfp_gpio_get_present(psfp) && sfp_i2c_get_present(psfp))
    {
        psfp->get_present = sff_get_present;
        mode = "sff workaround";
    }
#endif

    dev_info(psfp->dev, "registered: polling: %s present: %s\n", poll ? "true" : "false", mode);
    mutex_unlock(&psfp->st_mutex);

    trxbus_module_init(psfp->i2c->nr, psfp, &sfp_ops, 0);

    if (poll)
        mod_delayed_work(system_wq, &psfp->dwork_poll, poll_jiffies);
    else
        check_sfp_present(psfp);

    return 0;

label_unlock_return:
    mutex_unlock(&psfp->st_mutex);
    return ret;
}

static long bcmsfp_trx_compliance_get(struct sfp_data *psfp)
{
    unsigned long compliance_bitmap = 0;

    /* not decide copper rate at here */
    if (psfp->eeprom.base.connector == SFF8024_CONNECTOR_COPPER_PIGTAIL
        || psfp->eeprom.base.connector == SFF8024_CONNECTOR_RJ45)
        return BCM_TRX_COMPLIANCE_UNKNOWN;

    if (psfp->eeprom.base.cc[0] & SFF8472_CC_10GBASE_R_BITS)
        compliance_bitmap |= BCM_SFF8472_CC_10GBASE_R;
    
    if (psfp->eeprom.base.cc[3] & SFF8472_CC_1GBASE_X_BITS)
        compliance_bitmap |= BCM_SFF8472_CC_1GBASE_X;
    
    return compliance_bitmap;
}

struct platform_driver of_platform_sfp_driver = {
    .driver = {
        .name = "brcm_sfp",
        .of_match_table = of_platform_sfp_table,
    },
    .probe = _probe,
};

static __init int bcm_sfp_init(void)
{
    poll_jiffies = msecs_to_jiffies(500);

    return platform_driver_register(&of_platform_sfp_driver);
}

static void bcm_sfp_uninit(void)
{
    platform_driver_unregister(&of_platform_sfp_driver);
}

static int sfp_mon_read_los(struct sfp_data *psfp, long *value)
{
    int v = 1;

    if (psfp->gpio[GPIO_LOS])
    {
        v = gpiod_get_value_cansleep(psfp->gpio[GPIO_LOS]);
        if (v < 0)
            dev_err(psfp->dev, "gpiod_get_value_cansleep failed %d %p\n", v, psfp->gpio[GPIO_LOS]);
    }

    *value = v;
    return 0;
}

static int sfp_mon_write_phy_reg(struct sfp_data *psfp, int reg, long value)
{
    int ret;
    uint16_t val = (uint16_t)value;

    val = cpu_to_be16(val);
    if ((ret = i2c_write(psfp, SFP_I2C_PHY_ADDR, reg, &val, sizeof(val))) < 0)
        return ret;

    return 0;
}

static int sfp_mon_read_phy_reg(struct sfp_data *psfp, int reg, long *value)
{
    int ret;
    uint16_t val;

    if ((ret = i2c_read(psfp, SFP_I2C_PHY_ADDR, reg, &val, sizeof(val))) < 0)
        return ret;

    *value = be16_to_cpu(val);

    return 0;
}

static int sfp_mon_phy_sysfs_request(struct sfp_data *psfp, char *buf, int len)
{
    char cmd;
    unsigned int reg, data;
    long val;
    int argc, rc = -EINVAL;

    argc = sscanf(buf, "%c %d 0x%x", &cmd, &reg, &data);

    if (argc < 2)
        return -EINVAL;
  
    if (argc == 2 && cmd == 'r') {
        rc = sfp_mon_read_phy_reg(psfp, reg, &val);
        printk("read sfp phy register %d data 0x%x rc %d\n", reg, (unsigned int)val, rc);
    }

    if (argc == 3 && cmd == 'w') {
        val = (long)data;
        rc = sfp_mon_write_phy_reg(psfp, reg, val);
        printk("write sfp phy register %d data 0x%x rc %d\n", reg, data, rc);
    }

    return rc;
}

static int sfp_mon_read_eeprom_mon_buf(struct sfp_data *psfp, int reg, u8 *data, int len)
{
    uint16_t addr;
    int ret;

    if (psfp->eeprom.base.phys_id == PHYS_ID_XFP)
        addr = XFP_I2C_ADDR;
    else
        addr = SFP_I2C_A2_ADDR;

    ret = i2c_read(psfp, addr, reg, data, len);
    return ret < 0 ? ret : 0;
}

static int sfp_mon_read_eeprom_raw(struct sfp_data *psfp, int addr, int reg, uint8_t *data, int len)
{
    return i2c_read(psfp, addr, reg, data, len);
}

static int sfp_mon_read_eeprom_mon(struct sfp_data *psfp, int reg, long *value)
{
    uint16_t val;
    int ret;

    ret = sfp_mon_read_eeprom_mon_buf(psfp, reg, (uint8_t*)(&val), sizeof(val));
    *value = be16_to_cpu(val);

    return ret;
}

static int sfp_mon_read_byte_eeprom_mon(struct sfp_data *psfp, int reg, uint8_t *value)
{
    return sfp_mon_read_eeprom_mon_buf(psfp, reg, value, sizeof(*value));
}

#define xfp_mon_write_byte_eeprom_mon(psfp, reg, value) \
    i2c_write(psfp, XFP_I2C_ADDR, reg, &value, sizeof(value))

static int sfp_mon_read_eeprom_xfp_vcc(struct sfp_data *psfp, long *value)
{
#define XFP_AUX_INPUT_TYPES 222
#define XFP_AUX_INPUT_TYPE_VOLT 0b0001
#define XFP_AUX_MEASUREMENT1 106
#define XFP_AUX_MEASUREMENT2 108
    u8 types;
    int ret;

    if ((ret = i2c_read(psfp, XFP_I2C_ADDR, XFP_AUX_INPUT_TYPES, &types, sizeof(types))) < 0)
        return ret;

    if ((types & 0xF0) >> 4 == XFP_AUX_INPUT_TYPE_VOLT)
        return sfp_mon_read_eeprom_mon(psfp, XFP_AUX_MEASUREMENT1, value);

    if ((types & 0x0F) == XFP_AUX_INPUT_TYPE_VOLT)
        return sfp_mon_read_eeprom_mon(psfp, XFP_AUX_MEASUREMENT2, value);

    *value = 0;
    return -EOPNOTSUPP;
}

static int sfp_mon_read(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, long *value)
{
    u8 byte = 0;
    int ret;

    if (psfp->eeprom.base.phys_id == PHYS_ID_XFP)
    {
        switch (attr)
        {
            case bcmsfp_mon_vcc:
                return sfp_mon_read_eeprom_xfp_vcc(psfp, value);
            case bcmsfp_mon_xfp_rx_channel:
#define TRX_RX_CHN_CFG 0x71
                ret = sfp_mon_read_byte_eeprom_mon(psfp, TRX_RX_CHN_CFG, &byte);
                *value = byte;
                return ret;
            case bcmsfp_mon_xfp_tx_channel:
#define TRX_TX_CHN_CFG 0x70
                ret = sfp_mon_read_byte_eeprom_mon(psfp, TRX_TX_CHN_CFG, &byte);
                *value = byte;
                return ret;
            case bcmsfp_mon_id_optical_wavelength:
                *value = be16_to_cpu(psfp->eeprom.base.xfp.optical_wavelength);
                return 0;
            default:
                break;
        }
    }

    /* Common attributes, if not returned in above section */
    switch (attr)
    {
        case bcmsfp_mon_has_txsd:
            *value = (psfp->txsd_state != ERR_PTR(-ENODEV));
            return 0;
        case bcmsfp_mon_rxsd_pin:
            /* this is same to PON los gpio that connected the RX_LOS pin of 
               the SFP. dsl chip use hw function and it is always enabled through
               pinmux default state */
            *value = (long)psfp->rxsd_pin;
            return 0;
        case bcmsfp_mon_present:
            *value = !!psfp->present;
            return 0;
        case bcmsfp_mon_los:
             return sfp_mon_read_los(psfp, value);
        case bcmsfp_mon_force_tx_power:
            *value = psfp->force_tx_power;
            return 0;
        case bcmsfp_mon_id_phys_id:
            *value = psfp->eeprom.base.phys_id;
            return 0;
        case bcmsfp_mon_id_phys_ext_id:
            *value = psfp->eeprom.base.phys_ext_id;
            return 0;
        case bcmsfp_mon_id_connector:
            *value = psfp->eeprom.base.connector;
            return 0;
        case bcmsfp_mon_id_optical_wavelength:
            *value = be16_to_cpu(psfp->eeprom.base.sfp.passive.optical_wavelength);
            return 0;
        case bcmsfp_mon_trx_compliance:
            *value = bcmsfp_trx_compliance_get(psfp);
            return 0;
        case bcmsfp_mon_tx_power:
#define SFP_TX_POWER 0x66
            return sfp_mon_read_eeprom_mon(psfp, SFP_TX_POWER, value);
        case bcmsfp_mon_rx_power:
#define SFP_RX_POWER 0x68
            return sfp_mon_read_eeprom_mon(psfp, SFP_RX_POWER, value);
        case bcmsfp_mon_temp:
#define SFP_TEMP 0x60
            return sfp_mon_read_eeprom_mon(psfp, SFP_TEMP, value);
        case bcmsfp_mon_vcc:
#define SFP_VCC 0x62
            return sfp_mon_read_eeprom_mon(psfp, SFP_VCC, value);
        case bcmsfp_mon_bias_current:
#define SFP_TX_BIAS 0x64
            return sfp_mon_read_eeprom_mon(psfp, SFP_TX_BIAS, value);
        case bcmsfp_mon_phy_reg:
            return sfp_mon_read_phy_reg(psfp, channel, value);
        case bcmsfp_mon_tx_enable:
            *value = (long)sfp_module_tx_power_get(psfp);
            return 0;
        case bcmsfp_mon_rx_enable:
            *value = (long)sfp_module_rx_power_get(psfp);
            return 0;
        default:
            return -EOPNOTSUPP;
    }
}

static int sfp_mon_read_buf(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, char **buf, int *len)
{
    if (psfp->eeprom.base.phys_id == PHYS_ID_XFP)
    {
        switch (attr)
        {
            case bcmsfp_mon_id_vendor_rev:
                *buf = psfp->eeprom.base.xfp.vendor_rev;
                *len = sizeof(psfp->eeprom.base.xfp.vendor_rev);
                return 0;
            default:
                break;
        }
    }

    switch (attr)
    {
        case bcmsfp_mon_id_vendor_name:
            *buf = psfp->eeprom.base.vendor_name;
            *len = sizeof(psfp->eeprom.base.vendor_name);
            return 0;
        case bcmsfp_mon_id_vendor_pn:
            *buf = psfp->eeprom.base.vendor_pn;
            *len = sizeof(psfp->eeprom.base.vendor_pn);
            return 0;
        case bcmsfp_mon_id_vendor_rev:
            *buf = psfp->eeprom.base.sfp.vendor_rev;
            *len = sizeof(psfp->eeprom.base.sfp.vendor_rev);
            return 0;
        case bcmsfp_mon_id_vendor_sn:
            *buf = psfp->eeprom.ext.vendor_sn;
            *len = sizeof(psfp->eeprom.ext.vendor_sn);
            return 0;
        case bcmsfp_mon_id_date_code:
            *buf = psfp->eeprom.ext.datecode;
            *len = sizeof(psfp->eeprom.ext.datecode);
            return 0;
        case bcmsfp_mon_phy_reg:
            *buf = NULL;
            *len = 0;
            return 0;
        default:
            return -EOPNOTSUPP;
    }
}

static int sfp_mon_write_buf(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, char *buf, int len)
{
    if (psfp->eeprom.base.phys_id == PHYS_ID_XFP)
    {
        switch (attr)
        {
            case bcmsfp_mon_xfp_password:
                if (len < 4)
                    return -ENOMEM;

#define XFP_PASSWORD_BASE 0x7b
#define XFP_PASSWORD_LEN 4
                return i2c_write(psfp, XFP_I2C_ADDR, XFP_PASSWORD_BASE, buf, XFP_PASSWORD_LEN);
            default:
                break;
        }
    }

    switch (attr)
    {
        case bcmsfp_mon_phy_reg:
            return sfp_mon_phy_sysfs_request(psfp, buf, len);

        default:
            return -EOPNOTSUPP;
    }
}
    
static int sfp_mon_write(struct sfp_data *psfp, enum bcmsfp_mon_attr attr, int channel, long value)
{
    uint8_t byte = value;

    if (psfp->eeprom.base.phys_id == PHYS_ID_XFP)
    {
        switch (attr)
        {
        case bcmsfp_mon_xfp_rx_channel:
#define TRX_RX_CHN_CFG 0x71
            return xfp_mon_write_byte_eeprom_mon(psfp, TRX_RX_CHN_CFG, byte);
        case bcmsfp_mon_xfp_tx_channel:
#define TRX_TX_CHN_CFG 0x70
            return xfp_mon_write_byte_eeprom_mon(psfp, TRX_TX_CHN_CFG, byte);
        default:
            break;
        }
    }

    switch (attr)
    {
    case bcmsfp_mon_tx_enable:
        sfp_module_power_set(psfp, GPIO_TX_POWER, value);
        dev_dbg(psfp->dev, "tx %sable\n",  value ? "en" : "dis");
        return 0;
    case bcmsfp_mon_rx_enable:
        sfp_module_power_set(psfp, GPIO_RX_POWER, value);
        dev_dbg(psfp->dev, "rx %sable\n",  value ? "en" : "dis");
        return 0;
    case bcmsfp_mon_tx_power_down:
        _sfp_module_gpio_set(psfp, GPIO_TX_POWER_DOWN, value);
        return 0;
    case bcmsfp_mon_force_tx_power:
        /* This mode is required for some AE SFP, which must have TX power enabled in
        * order to read EEPROM.
        * XXX: note if set too late after insert interrupt, bcmsfp will fail detect sfp. */
        psfp->force_tx_power = value;
        sfp_module_power_set(psfp, GPIO_TX_POWER, value);
        return 0;
    case bcmsfp_mon_phy_reg:
        return sfp_mon_write_phy_reg(psfp, channel, value);
    default:
        return -EOPNOTSUPP;
    }
}

module_init(bcm_sfp_init);
module_exit(bcm_sfp_uninit);
MODULE_LICENSE("GPL");
