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

#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/pinctrl/consumer.h>
#include "port.h"

unsigned int bandwidth = 0, bandwidth_sum = 0;

#define RGMII_PINS_MAX 20
extern int bcmbca_pinctrl_get_pins_by_state(struct pinctrl_state *state, int *pins, int *num_pins);

static int of_sw_probe(struct device *dev, struct device_node *np, enetx_port_t *parent_port);
extern int bcm_enet_init_post(void);

#define REMOVE_ME
#ifdef REMOVE_ME
/* Add translation array - port-> unit,port */
static void unit_port_compat_set(int unit, int port, enetx_port_t *p)
{
    unit_port_array[unit][port] = p;
    enet_dbg("backward compat unit %d port %d -> %s\n", unit, port, p->obj_name);
}
#endif

static port_cap_t port_cap_get(struct device_node *port_dn, enetx_port_t *p)
{
    const char *full_name = of_node_full_name(port_dn);
    port_cap_t port_cap = PORT_CAP_NONE;
#if defined(enet_dbg_enabled)
    static char *cap2string[PORT_CAP_MAX] = 
                    {[PORT_CAP_NONE]="", 
                     [PORT_CAP_MGMT]="", 
                     [PORT_CAP_LAN_WAN]="LAN/WAN", 
                     [PORT_CAP_LAN_ONLY]="LAN Only", 
                     [PORT_CAP_WAN_ONLY]="WAN Only", 
                     [PORT_CAP_WAN_PREFERRED]="WAN Preferred"};
#endif

    if (of_property_read_bool(port_dn, "wan-only"))
    {
        if (of_property_read_bool(port_dn, "cap-lan-only"))
            enet_err("ERROR: ****** Conflict WAN Only Defined on %s:\n", full_name);
        else
            port_cap = PORT_CAP_WAN_ONLY;
    }
    else if (of_property_read_bool(port_dn, "wan-preferred"))
    {
        if (of_property_read_bool(port_dn, "cap-lan-only") ||
                of_property_read_bool(port_dn, "cap-wan-only"))
            enet_err("ERROR: ****** Conflict WAN Prefered Defined on %s:\n", full_name);
        else
            port_cap = PORT_CAP_WAN_PREFERRED;
    }
    else if (of_property_read_bool(port_dn, "lan-only"))
    {
        if (of_property_read_bool(port_dn, "cap-wan-only"))
            enet_err("ERROR: ****** Conflict LAN Only Defined on %s\n", full_name);
        else
            port_cap = PORT_CAP_LAN_ONLY;
    }

    if (port_cap == PORT_CAP_NONE)
    {
        if (of_property_read_bool(port_dn, "cap-lan-only"))
            port_cap = PORT_CAP_LAN_ONLY;
        else if (of_property_read_bool(port_dn, "cap-wan-only"))
            port_cap = PORT_CAP_WAN_ONLY;
        else if (of_property_read_bool(port_dn, "cap-wan-preferred"))
            port_cap = PORT_CAP_WAN_PREFERRED;
        else
            port_cap = PORT_CAP_LAN_WAN;
    }

    enet_dbg("port %s: %s LAN/WAN Capability: %s\n", full_name, p->dev->name, cap2string[port_cap]);
    return port_cap;
}

#ifdef ENET_DT_TEST
extern enetx_port_t *root_sw2;
#endif

static const char *port_type_string[] =
{
    FOREACH_PORT_TYPE(GENERATE_STRING)
};


static port_type_t sw_type_get(char *sw_type)
{
    int i;

    for (i = 0; i < sizeof(port_type_string)/sizeof(port_type_string[0]); i++)
    {
        if (!strcmp(port_type_string[i] + strlen("PORT_TYPE_"), sw_type))
            return i;
    }

    return -1;
}

static int mii_pins_get(struct device *dev, struct device_node *np, int pins[], int *num_pins)
{
    struct pinctrl *pinctrl = devm_pinctrl_get(dev);
    struct pinctrl_state *mii_pinctrl_state = NULL;
    char *state_name = (char *)of_get_property(np, "mii-pinctrl-state", NULL);

    if (!state_name)
        return -1;

    mii_pinctrl_state = pinctrl_lookup_state(pinctrl, state_name);
    if (IS_ERR(mii_pinctrl_state))
    {
        dev_err(dev, "Cannot resolve mii pin state: %s(ret=%ld).\n", state_name, PTR_ERR(mii_pinctrl_state));
        return -1;
    }

    return bcmbca_pinctrl_get_pins_by_state(mii_pinctrl_state, pins, num_pins);
}

extern phy_mii_type_t dt_get_phy_mode_mii_type(struct device_node *np);

#if defined(CONFIG_BCM_BCA_LED)
static void port_request_network_leds(struct device_node *dn, enetx_port_t *p)
{
    int i, sw_id = PORT_ON_ROOT_SW(p) ? 0 : 1;

    /* fill in the sw number and invalid port number */
    for (i = 0; i < MAX_PHYS_PER_CROSSBAR_GROUP; i++) {
        p->p.leds_info[i].sw_id = sw_id;
        p->p.leds_info[i].port_id = 0xff;
    }
    bca_led_request_network_leds(dn, p->p.leds_info);
}
#endif

#if defined(CONFIG_BRCM_SMC_BOOT)
#include <bp3_license.h>

#if defined(CONFIG_BCM968880)
#define GPHY0  0
#define GPHY1  1
#define GPHY2  2
#define GPHY3  3
#define XGPHY0 5
#define WANAE  6
#define XGPHY1 7
#define SLAN0  8
#define SLAN1  9
#define SLAN2  10
#define SLAN3  11
#define QLAN0  12
#define QLAN1  13
#define QLAN2  14
#define QLAN3  15
#elif defined(CONFIG_BCM96837)
#define GPHY0  0
#define GPHY1  1
#define GPHY2  2
#define GPHY3  3
#define WANAE  4
#define SLAN0  5
#define SLAN1  6
#define SLAN2  7
#define SLAN3  8
#define QLAN0  9
#define QLAN1  10
#define QLAN2  11
#define QLAN3  12
#define XGPHY0 -1
#define XGPHY1 -2
#else
#error
#endif

static int enet_port_license_check(uint32_t port_index, int speed)
{
    if (!speed)
        return 1;

    switch (port_index)
    {
    case GPHY0:
        return bcm_license_check_msg(BP3_FEATURE_EGPHY0) > 0;
    case GPHY1:
        return bcm_license_check_msg(BP3_FEATURE_EGPHY1) > 0;
    case GPHY2:
        return bcm_license_check_msg(BP3_FEATURE_EGPHY2) > 0;
    case GPHY3:
        return bcm_license_check_msg(BP3_FEATURE_EGPHY3) > 0;
    case XGPHY0:
        switch (speed)
        {
        case 10000:
            return bcm_license_check_msg(BP3_FEATURE_XGPHY0_10) > 0;
        case 5000:
            return bcm_license_check_msg(BP3_FEATURE_XGPHY0_5) > 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_XGPHY0_2_5) > 0;
        }
    case XGPHY1:
            switch (speed)
            {
            case 10000:
                return bcm_license_check_msg(BP3_FEATURE_XGPHY1_10) > 0;
            case 5000:
                return bcm_license_check_msg(BP3_FEATURE_XGPHY1_5) > 0;
            default:
                return bcm_license_check_msg(BP3_FEATURE_XGPHY1_2_5) > 0;
            }
    case SLAN0:
        switch (speed)
        {
        case 10000:
            return bcm_license_check_msg(BP3_FEATURE_SLAN_10) > 0;
        case 5000:
            return bcm_license_check_msg(BP3_FEATURE_SLAN_5) > 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_SLAN_2_5) > 0 || bcm_license_check_msg(BP3_FEATURE_SLAN_4_2_5) > 0;
        }
    case SLAN1:
    case SLAN2:
    case SLAN3:
        switch (speed)
        {
        case 10000:
        case 5000:
            return 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_SLAN_4_2_5) > 0;
        }
    case QLAN0:
        switch (speed)
        {
        case 10000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN0_10) > 0;
        case 5000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN0_5) > 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_QLAN0_2_5) > 0;
        }
    case QLAN1:
        switch (speed)
        {
        case 10000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN1_10) > 0;
        case 5000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN1_5) > 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_QLAN1_2_5) > 0;
        }
    case QLAN2:
        switch (speed)
        {
        case 10000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN2_10) > 0;
        case 5000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN2_5) > 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_QLAN2_2_5) > 0;
        }
    case QLAN3:
        switch (speed)
        {
        case 10000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN3_10) > 0;
        case 5000:
            return bcm_license_check_msg(BP3_FEATURE_QLAN3_5) > 0;
        default:
            return bcm_license_check_msg(BP3_FEATURE_QLAN3_2_5) > 0;
        }
    default:
        return 0;
    }

    return 0;
}
#endif

extern int dt_get_port_speed(const dt_handle_t port_handle);

static int of_port_probe(struct device *dev, enetx_port_t *sw, struct device_node *port_dn, int unit, int compat_port)
{
    struct device_node *phy_dn, *link_dn;
    enetx_port_t *p = NULL;
    uint32_t is_management = of_property_read_bool(port_dn, "management"); // PORT_FLAG_MGMT, ATTACHED_FLAG_CONTROL
    uint32_t is_detect = of_property_read_bool(port_dn, "detect");
    uint32_t is_delayed = of_property_read_bool(port_dn, "delayed-init");
    uint32_t is_wan = of_property_read_bool(port_dn, "is-wan");
    uint32_t is_attached = of_property_read_bool(port_dn, "error-sample"); // ATTACHED_FLAG_ES
    phy_mii_type_t mii_type;
    const char *mac_type_str;
    const char *full_name = of_node_full_name(port_dn);
    const unsigned int *port_reg;
    uint32_t port_index = -1;
    char *port_name = (char *)of_get_property(port_dn, "label", NULL);
    rgmii_params params = {};
    int speed;
    int rc = -1;

    port_info_t port_info =
    {
        .is_management = is_management,
#if defined(DSL_DEVICES)
        .is_undef = is_management,
#endif
        .is_detect = is_detect,
        .is_wan = is_wan,
        .is_attached = is_attached,
        .is_wanconf_ae = of_property_read_bool(port_dn, "is-wanconf-ae"),
        .is_wanconf_mux_ae = of_property_read_bool(port_dn, "is-wanconf-mux-ae"),
        .is_wanconf_mux_pon = of_property_read_bool(port_dn, "is-wanconf-mux-pon"),
    };

    port_reg = of_get_property(port_dn, "reg", NULL);
    if (!port_reg)
    {
        enet_err("Missing reg property\n");
        goto error;
    }

    port_index = be32_to_cpup(port_reg);
    port_info.port = port_index;

    speed = dt_get_port_speed(port_dn);

    if (speed && bandwidth && bandwidth_sum + speed > bandwidth)
    {
        enet_err("Skipping port_create for port %s due to bandwidth overflow: %d > %d\n",
            of_node_full_name(port_dn), bandwidth_sum + speed, bandwidth);
        return 0;
    }

    bandwidth_sum += speed;

    link_dn = of_parse_phandle(port_dn, "link", 0);
    if (link_dn) /* Used by G9991 lag */
        port_info.is_attached = 1;

    if (port_create(&port_info, sw, &p) > 0)
    {
        enet_dbg("Skipping port_create for %s\n", full_name);
        return 1;
    }
    else if (!p)
    {
        enet_err("Failed to create %s\n", full_name);
        goto error;
    }

    strncpy(p->obj_name, full_name, sizeof(p->obj_name)-1);
    p->obj_name[sizeof(p->obj_name)-1]='\0';

    mii_type = dt_get_phy_mode_mii_type(port_dn);

    if (mii_type == PHY_MII_TYPE_RGMII)
    {
        of_property_read_u32(port_dn, "rgmii-intf", &params.instance);
        params.delay_rx = of_property_read_bool(port_dn, "rx-delay");
        params.delay_tx = of_property_read_bool(port_dn, "tx-delay");
        params.is_1p8v = of_property_read_bool(port_dn, "rgmii-1p8v");
        params.is_3p3v = of_property_read_bool(port_dn, "rgmii-3p3v");
        params.is_disabled = of_property_read_bool(port_dn, "rgmii-disabled");
    }

    if ((phy_dn = of_parse_phandle(port_dn, "phy-handle", 0)))
    {
        if (!(p->p.phy = phy_drv_find_device(phy_dn)))
        {
            enet_err("Failed to find phy of devicenode (%s)\n", of_node_full_name(phy_dn));
#ifndef ENET_DT_TEST
            goto error;
#endif
        }

#ifndef ENET_DT_TEST
        phy_dev_attach(p->p.phy, mii_type, !params.delay_rx, !params.delay_tx, params.instance);
#endif

#if defined(DSL_DEVICES)
		// FIXME_DT: unify DSL & PON cascade handling
        // DSL devices port points 1st of cascade PHY instead of last
        if (!phy_is_pon_wan_ae_serdes(p->p.phy))
        {
            p->p.phy->sw_port = p;
            if (p->p.phy->cascade_next)
                p->p.phy->cascade_next->sw_port = p;
        }
        else
#endif
            p->p.phy = cascade_phy_get_last(p->p.phy);
    }

    if ((mac_type_str = of_get_property(port_dn, "mac-type", NULL)))
    {
#ifndef ENET_DT_TEST
        if (!(p->p.mac = mac_dev_dt_probe(port_dn, port_index)))
        {
            enet_err("Failed to create mac for port %d\n", port_index);
            goto error;
        }
#endif

        if (mii_type == PHY_MII_TYPE_RGMII)
        {
            int pins[RGMII_PINS_MAX], num_pins = RGMII_PINS_MAX;

            if (mii_pins_get(dev, port_dn, pins, &num_pins))
                num_pins = 0;

            params.pins = pins;
            params.num_pins = num_pins;
            mac_dev_rgmii_attach(&params);
        }
    }

    if (!link_dn && !is_attached && !port_info.is_undef) /* is_attached used by g9991 ES interface */
    {
        p->has_interface = 1;
        if (port_name)
        {
            strncpy(p->name, port_name, sizeof(p->name)-1);
            p->name[sizeof(p->name)-1]='\0';
        }
    }

    /* XXX: TODO CAPS */
    if (port_info.is_management)
        p->p.port_cap = PORT_CAP_MGMT;
    else
        p->p.port_cap = port_cap_get(port_dn, p);

#if defined(CONFIG_BCM_BCA_LED)
    port_request_network_leds(port_dn, p);
#endif

#if defined(CONFIG_BRCM_SMC_BOOT)
    enet_port_license_check(port_index, speed);
#endif

    if (is_delayed)
    {
        p->p.delayed_mac = p->p.mac;
        p->p.delayed_phy = p->p.phy;
        p->p.mac = NULL;
        p->p.phy = NULL;
    }

    if (port_info.is_detect)
        goto done;

#ifdef REMOVE_ME
#if defined(DSL_DEVICES)
    if (!phy_is_pon_wan_ae_serdes(p->p.phy))
        unit_port_compat_set(unit, p->port_info.port, p);
    else
#endif
        unit_port_compat_set(unit, compat_port, p);
#endif

    if (link_dn)
    {
        if ((rc = of_sw_probe(dev, link_dn, p)))
            goto error;
    }

done:
    enet_dbg("port create: success: devicenode: (%s)\n", full_name);
    return 0;
error:
    enet_err("port create: failed: initialize port index (%d) from devicenode: (%s)\n", port_index, full_name);
    return rc;
}

static int of_sw_probe(struct device *dev, struct device_node *np, enetx_port_t *parent_port)
{
    struct device_node *child, *port;
    const char *port_name;
    port_type_t sw_type;
    enetx_port_t *sw;
    int rc;
    static int g_unit = -1;
    int unit, compat_port = 0;
    char *sw_type_str = (char *)of_get_property(np, "sw-type", NULL);
    const unsigned int *pbandwidth = of_get_property(np, "bandwidth", NULL);

    if (!parent_port && pbandwidth)
        bandwidth = be32_to_cpup(pbandwidth);

    if (sw_type_str == NULL)
    {
        enet_err("failed to create switch sw-type missing in dts\n");
        return -1;
    }

    sw_type = sw_type_get(sw_type_str);
    if (!(sw = sw_create(sw_type, parent_port)))
    {
        enet_err("failed to create switch %d (%s)\n", sw_type, sw_type_str);
        return -1;
    }

    if ((port_name = (char *)of_get_property(np, "label", NULL)))
    {
        sw->has_interface = 1;
        strncpy(sw->name, port_name, sizeof(sw->name)-1);
		sw->name[sizeof(sw->name)-1]='\0';
    }

    if (!parent_port)
#ifndef ENET_DT_TEST
        root_sw = sw;
#else
        root_sw2 = sw;
#endif

    g_unit++;
    unit = g_unit;
#ifdef REMOVE_ME
    {
        const unsigned int *unit_reg;

        unit_reg = of_get_property(np, "unit", NULL);
        if (unit_reg)
            unit = be32_to_cpup(unit_reg);
    }
#endif

    for_each_available_child_of_node(np, child)
    {
        for_each_available_child_of_node(child, port)
        {
            if ((rc = of_port_probe(dev, sw, port, unit, compat_port)) < 0)
                return rc;

            if (rc == 0)
                compat_port++;
        };
    };

    sw->s.reset_gpiod = gpiod_get_from_of_node(np, "switch-reset", 0, GPIOD_ASIS, "SW reset");
    if (PTR_ERR(sw->s.reset_gpiod) == -ENOENT || PTR_ERR(sw->s.reset_gpiod) == -ENOSYS)
        sw->s.reset_gpiod = NULL;

    return 0;
}

static int enet_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    int ret;

    pr_notice_once("bcmenet DT\n");

    if (np)
    {
        ret = of_sw_probe(dev, np, NULL);
        if (ret)
        {
            dev_err(dev, "Error probe (ret=%d).\n", ret);
            return -EINVAL;
        }
    }

    ret = bcm_enet_init_post();
    if (ret)
    {
        dev_err(dev, "Error init_post (ret=%d).\n", ret);
        return -ENODEV;
    }

    return 0;
}

const struct of_device_id of_platform_enet_table[] = {
    { .compatible = "brcm,enet", .data = (void *)0, },
    { /* end of list */ },
};

static struct platform_driver of_platform_enet_driver = {
    .driver = {
        .name = "of_bcmenet",
        .of_match_table = of_platform_enet_table,
    },
    .probe = enet_probe,
};

int register_enet_platform_device(void)
{
    return platform_driver_register(&of_platform_enet_driver);
};

