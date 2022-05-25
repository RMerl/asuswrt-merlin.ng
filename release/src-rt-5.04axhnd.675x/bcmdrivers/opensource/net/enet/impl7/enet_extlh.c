/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom
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
 *  Created on: Apr/2020
 *      enet_extlh.c       extended link handler
 *      Author: steven.hsieh@broadcom.com
 */

#include "enet.h"
#include "port.h"
#include "mac_drv.h"
#include "mux_index.h"
#include <crossbar_dev.h>
#include <linux/kthread.h>

#include <bcmnet.h>
#include "bcmenet_common.h"
#include "phy_macsec_common.h"


struct semaphore bcm_link_handler_config;

extern u8 eth_internal_pause_addr[];

void set_mac_eee_by_phy_active(enetx_port_t *p)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = get_active_phy(p->p.phy);
    phy_dev_t *end_phy = cascade_phy_get_last(phy_dev);

    int enabled = 0;

    if (end_phy->link)
    {
        msleep(1000);
        phy_dev_eee_resolution_get(end_phy, &enabled);
    }

    mac_dev_eee_set(mac_dev, enabled);
}

#if defined(SF2_DEVICE)
void link_change_sf2_led_config(enetx_port_t *port, int linkstatus, int speed);
void link_change_sf2_conf_que_thread(enetx_port_t *port, int up);
void port_sf2_dual_tx_shape(enetx_port_t *port, int speed);
int enetxapi_post_sf2_parse(void);
#endif
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
void port_sf2_deep_green_mode_handler(void);
#endif
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
void _extsw_set_port_imp_map_2_5g(int unit);
void _extsw_set_port_imp_map_non_2_5g(int unit);
#endif

/*
 * handle_link_status_change
 */
void extlh_link_change_handler(enetx_port_t *port, int linkstatus, int speed, int duplex)
{
    phy_dev_t *phy_dev = get_active_phy(port->p.phy);
    phy_dev_t *end_phy = cascade_phy_get_last(phy_dev);
    mac_dev_t *mac_dev = port->p.mac;
    mac_cfg_t mac_cfg = {}, old_mac_cfg;
    int i, old_link = 0;
    phy_duplex_t old_duplex;
    phy_speed_t old_speed;
    static char *color[] ={"Brown", "Blue", "Green", "Orange"};
    static char *results[] = {"Invalid", "Good", "Open", "Intra Pair Short", "Inter Pair Short"};

    if (!phy_dev)
        return;

    mac_dev_disable(mac_dev);

#if defined(ARCHER_DEVICE)
    if (port->dev) {    /* Skip inter-connection device */
        bcmFun_t *enet_phy_speed_set = bcmFun_get(BCM_FUN_ID_ENET_PHY_SPEED_SET);
        bcmSysport_PhySpeed_t info;

        if (enet_phy_speed_set && linkstatus)
        {
            info.dev = port->dev;
            info.kbps = speed*1000;
            enet_phy_speed_set(&info);
        }
    }
#if defined(SF2_DUAL)
    port_sf2_dual_tx_shape(port, linkstatus ? speed:0);
#endif
#endif // ARCHER_DEVICE

    down(&bcm_link_handler_config);
#if defined(SF2_DEVICE)
    link_change_sf2_led_config(port, linkstatus, speed);
#endif

    if(port->dev)
        old_link = netif_carrier_ok(port->dev);
    old_speed = phy_dev->speed;
    old_duplex = phy_dev->duplex;
    phy_dev->link = linkstatus;

    msleep(1000);
    mac_dev_cfg_get(mac_dev, &old_mac_cfg);
    if (linkstatus) {

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        port_sf2_deep_green_mode_handler();
#endif

        switch(speed) {
            case 10000:
            case 5000:
            case 2500:
            case 1000:
            case 100:
            case 10:
                mac_cfg.speed = mac_mbps_2_speed(speed);
                phy_dev->speed = phy_mbps_2_speed(speed);
                break;
            default:
                enet_dbg("Incorrect speed to %s, speed: %d", __func__, speed);
                
        }               

        mac_cfg.duplex = phy_dev->duplex = duplex? PHY_DUPLEX_FULL: PHY_DUPLEX_HALF;
        mac_cfg.flag |= phy_dev_is_xgmii_mode(phy_dev)? XPORT_FLAG_XGMII: 0;
        mac_dev_cfg_set(mac_dev, &mac_cfg);

        if (phy_dev->phy_drv->caps_set)  // only update pause from phy if phy cap can be set
            mac_dev_pause_set(mac_dev, phy_dev->pause_rx, phy_dev->pause_tx, port->dev ? port->dev->dev_addr : eth_internal_pause_addr);

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
        if (speed == 2500)
        {
            _extsw_set_port_imp_map_2_5g(1);
        }
#endif

        if (port->dev && !old_link && phy_dev_cable_diag_is_supported(end_phy) && phy_dev_cable_diag_is_enabled(end_phy))
        {
            int result, pair_len[4];
            phy_dev_cable_diag_run(end_phy, &result, pair_len);
            if (result == CD_INVALID)
                printk("Cable Diagnosis Not Successful - Skipped\n");
            else {
                printk("Connected Cable Length: %d.%d meter\n", pair_len[0]/100, pair_len[0]%100);
                if (result != CD_ALL_PAIR_OK) {
                    for (i=0; i<4; i++)
                        if (CD_CODE_PAIR_GET(result, i) != CD_OK)
                            printk("    Pair %s: %s;", color[i], results[CD_CODE_PAIR_GET(result, i)]);
                    printk("\n");
                }
            }

            if (!phy_dev->link) /* If Cable Diag Causes Link Down, skip this round operation */
                goto end;
        }

#if defined(SF2_DEVICE)
        /* notify linux after we have finished setting our internal state */
        if (port->dev && (!old_link || old_speed != phy_dev->speed || old_duplex != phy_dev->duplex))
        {
            if (!old_link)
                link_change_sf2_conf_que_thread(port, 1);
        }
#endif

        if(port->dev)
        { 
            if (netif_carrier_ok(port->dev) == 0)
                netif_carrier_on(port->dev);

            port_link_change(port, 1);
            port_print_status_verbose(port);
        }
        else
        {   /* For internal inter connection device */
            if (old_mac_cfg.speed != mac_cfg.speed) /* Duplex is ignored in hardware */
                port_print_status_verbose(port);
        }

        mac_dev_enable(mac_dev);

    } else {
        /* also flush ARL for link down port */
        if (port->p.ops->fast_age)
            port->p.ops->fast_age(port);

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
        if (speed == 2500)
        {
            _extsw_set_port_imp_map_non_2_5g(1);
        }
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */

        // ethsw_eee_port_enable(sw_port, 0, 0);
        /* notify linux after we have finished setting our internal state */
        if (port->dev)
        {
            if (netif_carrier_ok(port->dev) != 0)
            {
#if defined(SF2_DEVICE)
                link_change_sf2_conf_que_thread(port, 0);
#endif
                netif_carrier_off(port->dev);
                port_link_change(port, 0);
                port_print_status_verbose(port);
            }
        }
        else
        {   /* For internal inter connection device, just in case */
            if (old_mac_cfg.speed != mac_cfg.speed) /* Duplex is ignored in hardware */
                port_print_status_verbose(port);
        }

        mac_dev_cfg_set(mac_dev, &mac_cfg);

        if (phy_dev_cable_diag_is_supported(end_phy) && phy_dev_cable_diag_is_enabled(end_phy))
        {
            int result, pair_len[4];
            phy_dev_cable_diag_run(end_phy, &result, pair_len);
            if (result == CD_INVALID)
                printk("Cable Dignosis Not successful - Skipped.\n");
            else
            {
                printk("PHY(address %d) Links Down due to: ", end_phy->addr);
                switch(result)
                {
                    case CD_ALL_PAIR_OK:
                        printk("Port on other end powered off; Cable length: %d.%d meter.\n",
                                pair_len[0]/100, pair_len[0]%100);
                        break;
                    case CD_ALL_PAIR_OPEN:
                        if ((pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3]) == 0)
                        {
                            printk("Cable is Unplugged on local port.\n");
                        }
                        else if (pair_len[0] == pair_len[1] && pair_len[0] == pair_len[2] && pair_len[0] == pair_len[3])
                        {
                            printk("Cable is Unplugged on remote port with length: %d.%d meter.\n",
                                    pair_len[0]/100, pair_len[0]%100);
                        }
                        else 
                        {
                            printk("Cable Open at Pair Br:%d.%d Bl:%d.%d Gr:%d.%d Or%d.%d meters\n",
                                pair_len[0]/100, pair_len[0]%100, pair_len[1]/100, pair_len[1]%100, 
                                pair_len[2]/100, pair_len[2]%100, pair_len[3]/100, pair_len[3]%100);
                        }
                        break;
                    default:
                        printk("\n");
                        for(i=0; i<4; i++)
                        {
                            if (CD_CODE_PAIR_GET(result, i) == CD_INVALID)
                            {
                                printk("    Pair %s: Cable Diagnosis Failed - Skipped\n", color[i]);
                                continue;
                            }
                                
                            printk("    Pair %s: Cable is %s %s %d.%d meters\n", color[i], 
                                results[CD_CODE_PAIR_GET(result, i)],
                                CD_CODE_PAIR_GET(result, i)==CD_OK? "with": "at",
                                pair_len[i]/100, pair_len[i]%100);
                        }
                        break;
                }
            }
        }
    }

end:
    /* update EEE settings based on link status */
    enetx_queue_work(port, set_mac_eee_by_phy_active);

    up(&bcm_link_handler_config);
}

void extlh_mac2mac_port_handle(enetx_port_t *self)
{
    phy_dev_t *phy_dev;

    if (self->p.phy && (phy_is_mac_to_mac(self->p.phy) || PhyIsPortConnectedToExternalSwitch(self->p.phy))
        && self->p.handle_phy_link_change)
    {
        phy_dev = phy_is_crossbar(self->p.phy)? crossbar_phy_dev_first(self->p.phy):self->p.phy;
        if (phy_dev == NULL)    /* Empty PHY under a crossbar port by phy-crossbar move in run time */
            return;
        phy_dev = cascade_phy_get_last(phy_dev);
        self->p.phy->speed = phy_dev->speed;
        self->p.phy->duplex = phy_dev->duplex;
        self->p.phy->link = 1;
        extlh_link_change_handler(self, self->p.phy->link, phy_speed_2_mbps(self->p.phy->speed), self->p.phy->duplex == PHY_DUPLEX_FULL);
    }
}

void extlh_phy_link_change_cb(void *ctx)
{
    phy_dev_t *phy = ctx;
    phy_dev_t *active_end_phy = get_active_phy(phy);    /* if phy is crossbar get actual phy that triggerred event */
#if defined(ENET_DT)
    enetx_port_t *p = phy->sw_port;
#else
    phy_dev_t *first_phy = cascade_phy_get_first(active_end_phy);
    enetx_port_t *p = first_phy->sw_port;
#endif

    phy_dev_status_propagate(active_end_phy);
    p->p.phy_last_change = (jiffies * 100) / HZ;

#if 0 //TODO146:
    if (!phy->link)
        serdes_work_around(first_phy);
#endif

    if (active_end_phy->link && active_end_phy->macsec_dev)
    {
        macsec_api_data data = { .op = MACSEC_OPER_RESTART };
        phy_dev_macsec_oper(active_end_phy, &data);
    }

    if (p->dev)
    {
        /* Print new status to console */
        extlh_link_change_handler(p, active_end_phy->link, 
            phy_speed_2_mbps(active_end_phy->speed), 
            active_end_phy->duplex == PHY_DUPLEX_FULL);

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        port_sf2_deep_green_mode_handler();
#endif
    }
}

int port_set_wan_role_link(enetx_port_t *port, port_netdev_role_t role)
{
    if (role > PORT_NETDEV_ROLE_WAN)
        return 0;

    if (role == PORT_NETDEV_ROLE_WAN)
    {
        /* Start PHY polling timer */
        if (port->p.port_cap == PORT_CAP_WAN_ONLY)
        {
            port->p.handle_phy_link_change = 1;
            phy_register_polling_timer(port->p.phy, extlh_phy_link_change_cb);
        }
        port->n.flags |= PORT_CFG_AS_WAN;
    }
    else
    {
        /* Stop PHY polling timer */
        if (port->p.port_cap == PORT_CAP_WAN_ONLY)
        {
            port->p.handle_phy_link_change = 0;
            phy_unregister_polling_timer(port->p.phy);
        }
        port->n.flags &= ~PORT_CFG_AS_WAN;
    }

    phy_dev_force_link_reset(port->p.phy);

    return 0;
}

int enetxapi_post_parse(void)
{
    sema_init(&bcm_link_handler_config, 1);
#if defined(SF2_DEVICE)
    enetxapi_post_sf2_parse();
#endif
    return 0;
}

