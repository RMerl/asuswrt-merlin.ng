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
        if (phy_drv && !strcmp(phy_drv->name, phy_type_str))
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
#if defined(DSL_DEVICES)  /* caps mask does not apply to DSL platform, which uses static private array for each type */
    return PHY_CAP_ALL;
#else
    int caps_mask = PHY_CAP_ALL;

    if (mii_type < PHY_MII_TYPE_XFI)
        caps_mask &= ~(PHY_CAP_10000 | PHY_CAP_5000);

    if (mii_type < PHY_MII_TYPE_HSGMII)
        caps_mask &= ~PHY_CAP_2500;

    if (mii_type < PHY_MII_TYPE_GMII)
        caps_mask &= ~(PHY_CAP_1000_HALF | PHY_CAP_1000_FULL);

    return caps_mask;
#endif
}

static int parse_inter_phy_types(const dt_handle_t handle)
{
    int inter_phy_types = 0;

    if (dt_property_read_bool(handle, "1000-Base-X"))
        inter_phy_types |= INTER_PHY_TYPE_1GBASE_X_M;
    if (dt_property_read_bool(handle, "1000-Base-R"))
        inter_phy_types |= INTER_PHY_TYPE_1GBASE_R_M;
    if (dt_property_read_bool(handle, "2500-Base-X"))
        inter_phy_types |= INTER_PHY_TYPE_2P5GBASE_X_M;
    if (dt_property_read_bool(handle, "2500-Base-R"))
        inter_phy_types |= INTER_PHY_TYPE_2P5GBASE_R_M;
    if (dt_property_read_bool(handle, "5000-Base-X"))
        inter_phy_types |= INTER_PHY_TYPE_5GBASE_X_M;
    if (dt_property_read_bool(handle, "5000-Base-R"))
        inter_phy_types |= INTER_PHY_TYPE_5GBASE_R_M;
    if (dt_property_read_bool(handle, "10000-Base-X"))
        inter_phy_types |= INTER_PHY_TYPE_10GBASE_X_M;
    if (dt_property_read_bool(handle, "10000-Base-R"))
        inter_phy_types |= INTER_PHY_TYPE_10GBASE_R_M;
    if (dt_property_read_bool(handle, "USXGMII-S"))
        inter_phy_types |= INTER_PHY_TYPE_USXGMII_M;
    if (dt_property_read_bool(handle, "USXGMII-M"))
        inter_phy_types |= INTER_PHY_TYPE_USXGMII_MP_M;

#if defined(CONFIG_BCM96858)
    inter_phy_types &= ~INTER_PHY_TYPE_5GBASE_X_M;
    inter_phy_types |= INTER_PHY_TYPE_5GIDLE_M;
#endif

    return inter_phy_types;
}

static int parse_usxgmii_parameters(phy_dev_t *phy_dev, const dt_handle_t handle)
{
#if defined(DSL_DEVICES)
    int m_type = USXGMII_S;
    const char *m_type_str = dt_property_read_string(handle, "usxgmii-m-type");

    if (m_type_str == NULL)
        return 0;

    for (m_type = USXGMII_M_MAX-1; m_type > USXGMII_S; m_type--)
        if (!strcasecmp(m_type_str, usxgmii_m_type_strs[m_type]))
            break;
    phy_dev->usxgmii_m_type = m_type;
    phy_dev->usxgmii_m_index = dt_property_read_u32(handle, "mphy-index");
    phy_dev->addr = mphy_dev_true_addr(phy_dev);
#else
    if (phy_dev->inter_phy_types & INTER_PHY_TYPE_USXGMII_M)
        phy_dev->usxgmii_m_type = USXGMII_S;
    if (phy_dev->inter_phy_types & INTER_PHY_TYPE_USXGMII_MP_M)
        phy_dev->usxgmii_m_type = USXGMII_M_10G_Q;
#endif

    return 0;
}

static int parse_txfir_speed_idx(char *spd)
{
    char *str[] = {PHY_TXFIR_SPEED_STRING_ARRAY};
    int i;

    for (i=0; i<ARRAY_SIZE(str); i++)
        if (strcasecmp(spd, str[i]) == 0)
            return i;

    return -1;
}

static int parse_serdes_txfir(phy_dev_t *phy_dev, const dt_handle_t handle)
{
    const char *str;
    char buf[256], *s1, *s2 = 0, *tk, *sp;
    int idx, len;
    uint64_t val64;

    str = dt_property_read_string(handle, "txfir");
    if (!str)
        return 0;

    for (s1=(char *)str; *s1; s2++, s1=s2) {
        s2 = strchr(s1, ';');

        if (!s2)
            s2 = s1 + strlen(s1);

        len = s2 - s1;
        if (len > sizeof(buf))
        {
            printk(" Too long Txfir string definition: %s\n", str);
            goto error;
        }
        strncpy(buf, s1, len);
        buf[len] = 0;
        sp = buf;
        tk = strsep(&sp, ":");
        if (!tk)
            goto error;
        if ((idx = parse_txfir_speed_idx(tk)) == -1)
            goto error;

        tk = strsep(&sp, ",");
        if (!tk)
            goto error;
        if (kstrtoul(tk, 0, (unsigned long *)&val64) < 0)
            goto error;
        phy_dev->txfir[idx].pre = val64;

        tk = strsep(&sp, ",");
        if (!tk)
            goto error;
        if (kstrtoul(tk, 0, (unsigned long *)&val64) < 0)
            goto error;
        phy_dev->txfir[idx].main = val64;

        tk = strsep(&sp, ",");
        if (!tk)
            goto error;
        if (kstrtoul(tk, 0, (unsigned long *)&val64) < 0)
            goto error;
        phy_dev->txfir[idx].post1 = val64;

        tk = strsep(&sp, ",");
        if (!tk)
            goto error;
        if (kstrtoul(tk, 0, (unsigned long *)&val64) < 0)
            goto error;
        phy_dev->txfir[idx].post2 = val64;

        tk = strsep(&sp, ",");
        if (!tk)
        {
            phy_dev->txfir[idx].hpf = -1;
        }
        else
        {
            if (kstrtoul(tk, 0, (unsigned long *)&val64) < 0)
                goto error;
            phy_dev->txfir[idx].hpf = val64;
        }

        if (*s2 == 0)
            break;
    }


    return 0;
error:
    printk(" Error of Txfir string definition: %s\n", str);
    return -1;
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

    ret |= dt_gpio_request_by_name(handle, "phy-power", 0, "PHY power", &phy_dev->gpiod_phy_power, 0);
    ret |= dt_gpio_request_by_name(handle, "phy-reset", 0, "PHY reset", &phy_dev->gpiod_phy_reset, 1);
    ret |= dt_gpio_request_by_name(handle, "tx-disable", 0, "TX disable", &phy_dev->gpiod_tx_disable, 1);

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
        if (!strcasecmp(phy_mode_str, phy_dev_mii_type_to_str(i)))
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
        printk("Missing or wrong phy-type entry %s\n", dt_property_read_string(handle, "phy-type"));
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
    phy_dev->eee = !dt_property_read_bool(handle, "caps-no-eee");
    phy_dev->swap_pair = dt_property_read_bool(handle, "enet-phy-lane-swap");
    phy_dev->xfi_tx_polarity_inverse = dt_property_read_bool(handle, "phy-xfi-tx-polarity-inverse");
    phy_dev->xfi_rx_polarity_inverse = dt_property_read_bool(handle, "phy-xfi-rx-polarity-inverse");
    phy_dev->flag |= dt_property_read_bool(handle, "phy-external") ? PHY_FLAG_EXTPHY : 0;
    phy_dev->flag |= dt_property_read_bool(handle, "phy-extswitch") ? PHY_FLAG_TO_EXTSW : 0;
    phy_dev->flag |= dt_property_read_bool(handle, "phy-fixed") ? PHY_FLAG_FIXED_CONN: 0;
    phy_dev->flag |= dt_property_read_bool(handle, "wake-on-lan") ? PHY_FLAG_WAKE_ON_LAN : 0;
    phy_dev->flag |= dt_property_read_bool(handle, "force-2p5g-10gvco") ? PHY_FLAG_FORCE_2P5G_10GVCO: 0;

    phy_dev->shared_ref_clk_mhz = dt_property_read_u32_default(handle, "shared-ref-clk-mhz", 0);

    parse_gpio_parameters(phy_dev, handle);
    parse_serdes_parameters(phy_dev, handle);
    parse_serdes_txfir(phy_dev, handle);

    phy_dev->caps_mask = PHY_CAP_ALL;
    if (dt_property_read_bool(handle, "caps-no-hdx"))
        phy_dev->caps_mask &= ~(PHY_CAP_10_HALF | PHY_CAP_100_HALF | PHY_CAP_1000_HALF);
    if (dt_property_read_bool(handle, "caps-no-10000"))
        phy_dev->caps_mask &= ~(PHY_CAP_10000);
    if (dt_property_read_bool(handle, "caps-no-5000"))
        phy_dev->caps_mask &= ~(PHY_CAP_5000);
    if (dt_property_read_bool(handle, "caps-no-2500"))
        phy_dev->caps_mask &= ~(PHY_CAP_2500);
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
    unsigned long mac_index;

    if (mac_type == MAC_TYPE_UNKNOWN)
        goto Exit;

    /* Fallback to default mac_id == <reg> */
    mac_index = dt_property_read_u32_default(handle, "mac-index", port_index);

    ret = mac_drv_dt_priv(mac_type, handle, mac_index, &priv);
    if (ret)
        goto Exit;

    mac_dev = mac_dev_add(mac_type, mac_index, priv);
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

int dt_xfi_port_get(void)
{
    struct device_node *np, *child, *sec;
    char *phy_mode;
    int port_index;
    int xfi_index = -1;
    const unsigned int *port_reg;

    if (!(np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
        return -1;

    for_each_available_child_of_node(np, sec)
    {
        for_each_available_child_of_node(sec, child)
        {
            port_reg = of_get_property(child, "reg", NULL);
            if (!port_reg)
                continue;

            port_index = be32_to_cpup(port_reg);
            if ((phy_mode = (char *)of_get_property(child, "phy-mode", NULL)) &&
                !strcmp("xfi", phy_mode))
            {
                xfi_index = port_index;
                break;
            }
        }
    }

    of_node_put(np);

    return xfi_index;
}
EXPORT_SYMBOL(dt_xfi_port_get);

uint32_t dt_g9991_phys_port_vec_get(void)
{
    struct device_node *np, *child, *sec;
    int port_index;
    const unsigned int *port_reg;
    uint32_t vec = 0;

    if (!(np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
        return -1;

    for_each_available_child_of_node(np, sec)
    {
        for_each_available_child_of_node(sec, child)
        {
            port_reg = of_get_property(child, "reg", NULL);
            if (!port_reg)
                continue;

            port_index = be32_to_cpup(port_reg);
            if (of_get_property(child, "link", NULL))
                vec |= (1 << port_index);
        }
    }

    of_node_put(np);

    return vec;
}
EXPORT_SYMBOL(dt_g9991_phys_port_vec_get);

static const dt_handle_t dt_parse_last_phandle(const dt_handle_t handle)
{
    dt_handle_t next_handle = dt_parse_phandle(handle, "phy-handle", 0);

    if (!next_handle)
        return handle;

    return dt_parse_last_phandle(next_handle);
}

int dt_get_port_speed(const dt_handle_t port_handle)
{
    int is_detect = dt_property_read_bool(port_handle, "detect");
    dt_handle_t phy_handle = dt_parse_phandle(port_handle, "phy-handle", 0);
    int port_speed = 0;
    phy_mii_type_t mii_type = dt_get_phy_mode_mii_type(port_handle);
    uint32_t caps_mask = parse_caps_mask(mii_type);

    if (is_detect || !dt_is_valid(phy_handle))
        return 0;

    phy_handle = dt_parse_last_phandle(phy_handle);

    if (dt_property_read_bool(phy_handle, "caps-no-10000"))
        caps_mask &= ~(PHY_CAP_10000);
    if (dt_property_read_bool(phy_handle, "caps-no-5000"))
        caps_mask &= ~(PHY_CAP_5000);
    if (dt_property_read_bool(phy_handle, "caps-no-2500"))
        caps_mask &= ~(PHY_CAP_2500);

    if (caps_mask & PHY_CAP_10000)
        port_speed = 10000;
    else if (caps_mask & PHY_CAP_5000)
        port_speed = 5000;
    else if (caps_mask & PHY_CAP_2500)
        port_speed = 2500;
    else
        port_speed = 1000;

    return port_speed;
}
EXPORT_SYMBOL(dt_get_port_speed);

#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
uint32_t dt_ports_speed_get(uint32_t *ports_speeds)
{
    struct device_node *np, *child, *sec;

    if (!(np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
        return -1;

    for_each_available_child_of_node(np, sec)
    {
        for_each_available_child_of_node(sec, child)
        {
            const unsigned int *port_reg;
            int port_index;

            port_reg = of_get_property(child, "reg", NULL);
            if (!port_reg)
                continue;

            port_index = be32_to_cpup(port_reg);
            ports_speeds[port_index] = dt_get_port_speed(child);
        }
    }

    of_node_put(np);

    return 0;
}
EXPORT_SYMBOL(dt_ports_speed_get);
#endif

const struct of_device_id phy_of_platform_table[] = {
    { .compatible = "brcm,bcaphy", .data = (void *)0, },
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
