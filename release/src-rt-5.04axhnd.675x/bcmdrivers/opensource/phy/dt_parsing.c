/*
   Copyright (c) 2019 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard

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
 *  Created on: Oct 2019
 *      Author: ido.brezel@broadcom.com
 */

#include "phy_drv.h"
#include "mac_drv.h"
#include "dt_access.h"

#include <linux/of_platform.h>
#include "brcm_rgmii.h"

extern phy_drv_t *phy_drivers[PHY_TYPE_MAX];

static phy_type_t phy_type_get_by_str(const char *phy_type_str)
{
    phy_drv_t *phy_drv;
    int i;

    for (i = 0; i < PHY_TYPE_MAX; i++)
    {
        phy_drv = phy_drivers[i];
        if (phy_drv && strstr(phy_drv->name, phy_type_str))
            return i;
    }

    return PHY_TYPE_UNKNOWN;
}

static int phy_drv_dt_priv(phy_type_t phy_type, const dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv)
{
    phy_drv_t *phy_drv = phy_drivers[phy_type];

    if (!phy_drv)
    {
        printk("Failed to find phy driver: phy_type=%d\n", phy_type);
        return -1;
    }

    if (!phy_drv->dt_priv)
        return 0;

    return phy_drv->dt_priv(handle, addr, phy_mode, _priv);
}

static int parse_caps_mask(phy_mii_type_t mii_type)
{
    int caps_mask = PHY_CAP_ALL;

#if defined(DSL_DEVICES)  /* caps mask does not apply to DSL platform, which uses static private array for each type */
    return PHY_CAP_ALL;
#endif

    if (mii_type < PHY_MII_TYPE_XFI)
        caps_mask &= ~(PHY_CAP_10000 | PHY_CAP_5000);

    if (mii_type < PHY_MII_TYPE_HSGMII)
        caps_mask &= ~PHY_CAP_2500;

    if (mii_type < PHY_MII_TYPE_GMII)
        caps_mask &= ~(PHY_CAP_1000_HALF | PHY_CAP_1000_FULL);

    return caps_mask;
}

static int parse_inter_phy_types(const dt_handle_t handle)
{
    int inter_phy_types = 0;

#if !defined(DSL_DEVICES)
    int is_2500_base_r = dt_property_read_bool(handle, "2500-Base-R");
    int is_5000_base_r = dt_property_read_bool(handle, "5000-Base-R");
    int is_usxgmii_s = dt_property_read_bool(handle, "USXGMII-S");
    int is_usxgmii_m = dt_property_read_bool(handle, "USXGMII-M");

    if (is_2500_base_r)
        inter_phy_types |= INTER_PHY_TYPE_2P5GBASE_R_M;
    else
        inter_phy_types |= INTER_PHY_TYPE_2P5GBASE_X_M;

    if (is_5000_base_r)
        inter_phy_types |= INTER_PHY_TYPE_5GBASE_R_M;
    else
        inter_phy_types |= INTER_PHY_TYPE_5GIDLE_M;

    if (is_usxgmii_s)
        inter_phy_types |= INTER_PHY_TYPE_USXGMII_M;

    if (is_usxgmii_m)
        inter_phy_types |= INTER_PHY_TYPE_USXGMII_MP_M;
#endif

    return inter_phy_types;
}

static int parse_usxgmii_parameters(phy_dev_t *phy_dev, const dt_handle_t handle)
{
#if defined(DSL_DEVICES)
    int m_type = USXGMII_S;
    const char *m_type_str = dt_property_read_string(handle, "usxgmii-m-type");
    dt_handle_t mphy_hdl = dt_parse_phandle(handle, "mphy-base", 0);

    if (m_type_str)
    {
        for (m_type = USXGMII_M_MAX-1; m_type > USXGMII_S; m_type--)
            if (!strcasecmp(m_type_str, usxgmii_m_type_strs[m_type])) break;
        phy_dev->mphy_base = phy_dev;
        phy_dev->usxgmii_m_type = m_type;
    }
    
    if (dt_is_valid(mphy_hdl))
    {
        phy_dev->mphy_base = phy_drv_find_device(mphy_hdl);
        phy_dev->usxgmii_m_type = phy_dev->mphy_base->usxgmii_m_type;
        phy_dev->usxgmii_m_index = mphy_dev_instance(phy_dev);
        phy_dev->addr = mphy_dev_true_addr(phy_dev);
    }
#endif

    return 0;
}

static int parse_serdes_parameters(phy_dev_t *phy_dev, const dt_handle_t handle)
{
    phy_dev->core_index = dt_property_read_u32_default(handle, "serdes-core", 0);
    phy_dev->lane_index = dt_property_read_u32_default(handle, "serdes-lane", 0);
    phy_dev->inter_phy_types = parse_inter_phy_types(handle);
    parse_usxgmii_parameters(phy_dev, handle);

    return 0;
}

static int parse_gpio_parameters(phy_dev_t *phy_dev, const dt_handle_t handle)
{
    int ret = 0;

    ret |= dt_gpio_request_by_name_optional(handle, "phy-reset", 0, "PHY reset", &phy_dev->gpiod_phy_reset);
    ret |= dt_gpio_request_by_name_optional(handle, "tx-disable", 0, "TX disable", &phy_dev->gpiod_tx_disable);

    return ret;
}

phy_mii_type_t dt_get_phy_mode_mii_type(const dt_handle_t handle)
{
    int i;
    const char *phy_mode_str = dt_property_read_string(handle, "phy-mode");

    if (!phy_mode_str)
        goto Exit;

    for (i = PHY_MII_TYPE_UNKNOWN; i < PHY_MII_TYPE_LAST; i++)
    {
        if (!strcasecmp(phy_mode_str, phy_mii_type(i)))
            return i;
    }

Exit:
    return PHY_MII_TYPE_UNKNOWN;
}
EXPORT_SYMBOL(dt_get_phy_mode_mii_type);

bus_drv_t *bus_drv_get_by_str(const char *bus_type_str);

static phy_dev_t *phy_dev_dt_probe(const dt_handle_t handle)
{
    phy_dev_t *phy_dev = NULL, *phy_dev_next = NULL;
    void *priv = NULL;
    uint32_t addr;
    const phy_type_t phy_type = phy_type_get_by_str(dt_property_read_string(handle, "phy-type"));
    dt_handle_t phy;

    if (phy_type == PHY_TYPE_UNKNOWN)
    {
        printk("Missing or wrong phy-type entry\n");
        goto Exit;
    }

    addr = dt_property_read_u32(handle, "reg");
    if (addr == (uint32_t)(-1))
    {
        printk("Missing reg entry\n");
        goto Exit;
    }

    phy = dt_parse_phandle(handle, "phy-handle", 0);
    if (dt_is_valid(phy))
    {
        if (!(phy_dev_next = phy_drv_find_device(phy)))
        {
            printk("phy_dev probe_defer: %s waiting for phy %s\n", dt_get_name(handle), dt_get_name(phy));
            return ERR_PTR(-EPROBE_DEFER);
        }
    }

    /* XXX: phy_drv_dt_priv API can probably be depricated after removing BP support. Should be
     * called as part of probe, but no need to pass priv outside of phy_drv implementation */
    if ((phy_drv_dt_priv(phy_type, handle, addr, 0, &priv)) == -EPROBE_DEFER)
        return ERR_PTR(-EPROBE_DEFER);

    if (phy_type == PHY_TYPE_CROSSBAR)
    {
        phy_dev = (phy_dev_t *)priv;
        goto Done;
    }

    phy_dev = phy_dev_add(phy_type, addr, priv);
    if (!phy_dev)
    {
        printk("Failed to create phy device: %d:%d\n", phy_type, addr);
        goto Exit;
    }

    phy_dev->bus_drv = bus_drv_get_by_str(dt_property_read_string(dt_parent(handle), "bus-type"));
    phy_dev->swap_pair = dt_property_read_bool(handle, "enet-phy-lane-swap");
    phy_dev->xfi_tx_polarity_inverse = dt_property_read_bool(handle, "phy-xfi-tx-polarity-inverse");
    phy_dev->xfi_rx_polarity_inverse = dt_property_read_bool(handle, "phy-xfi-rx-polarity-inverse");
    phy_dev->inter_phy_types = parse_inter_phy_types(handle);
    phy_dev->core_index = dt_property_read_u32(handle, "serdes-core");
    phy_dev->lane_index = dt_property_read_u32(handle, "serdes-lane");
    phy_dev->inter_phy_types = parse_inter_phy_types(handle);
    phy_dev->flag |= dt_property_read_bool(handle, "phy-external") ? PHY_FLAG_EXTPHY : 0;
    phy_dev->flag |= dt_property_read_bool(handle, "phy-extswitch") ? PHY_FLAG_TO_EXTSW : 0;
    phy_dev->flag |= dt_property_read_bool(handle, "phy-fixed") ? PHY_FLAG_FIXED_CONN: 0;
    phy_dev->flag |= dt_property_read_bool(handle, "wake-on-lan") ? PHY_FLAG_WAKE_ON_LAN : 0;

    parse_gpio_parameters(phy_dev, handle);
    parse_serdes_parameters(phy_dev, handle);

    phy_dev->caps_mask = PHY_CAP_ALL;
    if (dt_property_read_bool(handle, "caps-no-hdx"))
        phy_dev->caps_mask &= ~(PHY_CAP_10_HALF | PHY_CAP_100_HALF | PHY_CAP_1000_HALF);
    if (dt_property_read_bool(handle, "caps-no-10000"))
        phy_dev->caps_mask &= ~(PHY_CAP_10000);
    if (dt_property_read_bool(handle, "caps-no-5000"))
        phy_dev->caps_mask &= ~(PHY_CAP_5000);
    if (dt_property_read_bool(handle, "caps-no-1000"))
        phy_dev->caps_mask &= ~(PHY_CAP_1000_HALF | PHY_CAP_1000_FULL);
    if (dt_property_read_bool(handle, "caps-no-100"))
        phy_dev->caps_mask &= ~(PHY_CAP_100_HALF | PHY_CAP_100_FULL);
    if (dt_property_read_bool(handle, "caps-no-10"))
        phy_dev->caps_mask &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL);

    if (phy_drv_dev_add(phy_dev))
    {
        printk("Failed to add phy device to the driver: %s:%d\n", phy_dev->phy_drv->name, addr);
        phy_dev_del(phy_dev);
        phy_dev = NULL;
        goto Exit;
    }

    if (phy_dev_next)
    {
        phy_dev_next->cascade_prev = phy_dev;
        phy_dev->cascade_next = phy_dev_next;
    }

Done:
    printk("Registered phy device: %s:0x%x\n", phy_dev->phy_drv->name, addr);

Exit:
    return phy_dev;
}

void phy_dev_attach(phy_dev_t *phy_dev, phy_mii_type_t mii_type, int delay_rx, int delay_tx, int instance)
{
    if (phy_dev->phy_drv->phy_type == PHY_TYPE_CROSSBAR)
        return;

    phy_dev->delay_rx = delay_rx;
    phy_dev->delay_tx = delay_tx;
    if (mii_type == PHY_MII_TYPE_RGMII)
        phy_dev->core_index = instance;

    phy_dev->mii_type = mii_type;
    phy_dev->caps_mask &= parse_caps_mask(phy_dev->mii_type);

    if (phy_dev->cascade_next)
        phy_dev_attach(phy_dev->cascade_next, mii_type, delay_rx, delay_tx, instance);
}
EXPORT_SYMBOL(phy_dev_attach);

void mac_dev_rgmii_attach(rgmii_params *params)
{
    rgmii_attach(params);
}
EXPORT_SYMBOL(mac_dev_rgmii_attach);

extern mac_drv_t *mac_drivers[MAC_TYPE_MAX];

static mac_type_t dt_get_mac_type(const dt_handle_t handle)
{
    int i;
    mac_drv_t *mac_drv;
    const char *mac_type_str = dt_property_read_string(handle, "mac-type");

    if (!mac_type_str)
        goto Exit;

    for (i = 0; i < MAC_TYPE_MAX; i++)
    {
        mac_drv = mac_drivers[i];
        if (mac_drv && !strcasecmp(mac_drv->name, mac_type_str))
            return i;
    }

Exit:
    return MAC_TYPE_UNKNOWN;
}

static int mac_drv_dt_priv(mac_type_t mac_type, const dt_handle_t handle, int mac_id, void **priv)
{
    mac_drv_t *mac_drv;

    if (!(mac_drv = mac_drivers[mac_type]))
    {
        printk("Failed to find MAC driver: mac_type=%d\n", mac_type);
        return -1;
    }

    if (!mac_drv->dt_priv)
        return 0;

    return mac_drv->dt_priv(handle, mac_id, priv);
}

mac_dev_t *mac_dev_dt_probe(const dt_handle_t handle, uint32_t port_index)
{
    int ret;
    void *priv;
    mac_dev_t *mac_dev = NULL;
    const mac_type_t mac_type = dt_get_mac_type(handle);

    if (mac_type == MAC_TYPE_UNKNOWN)
        goto Exit;

    ret = mac_drv_dt_priv(mac_type, handle, port_index, &priv);
    if (ret)
        goto Exit;

    mac_dev = mac_dev_add(mac_type, port_index, priv);
    if (!mac_dev)
    {
        printk("Failed to create mac device: %d:%d\n", mac_type, port_index);
        goto Exit;
    }

    printk("Registered mac device: %s:0x%x\n", mac_dev->mac_drv->name, port_index);

Exit:
    return mac_dev;
}
EXPORT_SYMBOL(mac_dev_dt_probe);

static int phy_probe(struct platform_device *pdev)
{
    struct device_node *node = pdev->dev.of_node;
    phy_dev_t *phy_dev;

    if (!node)
        return -ENODEV;

    phy_dev = phy_dev_dt_probe(node);
    if (phy_dev == ERR_PTR(-EPROBE_DEFER))
        return -EPROBE_DEFER;
    if (!phy_dev)
        return -EINVAL;

    phy_dev->dt_handle = node;

    return 0;
}

const struct of_device_id phy_of_platform_table[] = {
    { .compatible = "brcm,bcaphy", .data = (void *)0, },
    { .compatible = "brcm,intelphy", .data = (void *)0, },
    { /* end of list */ },
};

struct platform_driver phy_of_platform_driver = {
    .driver = {
        .name = "bcmbca_phy",
        .of_match_table = phy_of_platform_table,
    },
    .probe = phy_probe,
};

static int __init phy_init(void)
{
    int rc;

    rc = phy_drivers_set();
    if (rc)
        return rc;

    return platform_driver_register(&phy_of_platform_driver);
}

late_initcall(phy_init);
