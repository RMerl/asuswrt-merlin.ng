/*
   <:copyright-BRCM:2024:DUAL/GPL:standard
   
      Copyright (c) 2024 Broadcom 
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
*******************************************************************************
*
* File Name  : bcmnet_wol.c
*
* Description: This file implements WOL / ACPI feature.
*
*******************************************************************************
*/

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "port.h"
#include "enet.h"
#include "bcmethsw.h"
#include "wol.h"
#include "bcmenet_wol.h"

#if defined(CONFIG_BCM96813)
extern void __iomem *xport_virt_base;
#endif

#if !defined(WOL_HW_SWITCH)
void wol_enable(char *dev_name, wol_params_t *wol_params) { }
#else
static uint32_t port_to_wol_offset [] = {
    WOL_PORT_0_OFFSET,
    WOL_PORT_1_OFFSET,
    WOL_PORT_2_OFFSET,
    WOL_PORT_3_OFFSET,
    WOL_PORT_4_OFFSET,
    WOL_PORT_5_OFFSET,
    WOL_PORT_6_OFFSET };


static int get_dev_port_number(char* dev_name)
{
    struct net_device *dev = NULL;

    dev = dev_get_by_name(&init_net, dev_name);
    if (dev) {
    	enetx_netdev *ndev;
	int port;

      	ndev = netdev_priv(dev);
	port = ndev->port->port_info.port;
        dev_put(dev);
	return port;
    }

    return -1;
}

static void disable_mpd_detection(int port)
{
    ET_WOL_DET *wol_intf;
    uint32_t wol_offset;
#if defined(CONFIG_BCM96813)
    long wol_base = (long)xport_virt_base;
#else
    long wol_base = (long)sw_mmap_base.wol_base;
#endif

    if ((wol_offset = port_to_wol_offset[port]) == WOL_PORT_MISSING) {
        printk("Port %d does not support WoL\n", port);
	return;
    }

    wol_intf = (ET_WOL_DET *)(wol_base + wol_offset);

    wol_intf->wol_mpd.control &= ~ET_WOL_DET_MPD_CTRL_EN_M;

#if !defined(CONFIG_BCM96813)
    wol_intf->wol_ctl.control &= ~(ET_XPHY_WOL_CONTROL_WOL_EN_M |
				   ET_XPHY_WOL_CONTROL_WOL_RX_EN_M |
				   ET_XPHY_WOL_CONTROL_WOL_TX_EN_M);
#endif
}

int wol_status_show(struct seq_file *m, void *v)
{
    int port;
    char* status;
    ET_WOL_DET *wol_intf;
    uint32_t wol_offset;
#if defined(CONFIG_BCM96813)
    long wol_base = (long)xport_virt_base;
#else
    long wol_base = (long)sw_mmap_base.wol_base;
#endif

    for (port = 0; port < WOL_PORT_COUNT; port++) {

        if ((wol_offset = port_to_wol_offset[port]) == WOL_PORT_MISSING)
	    continue;

	wol_intf = (ET_WOL_DET *)(wol_base + wol_offset);

	if ((wol_intf->wol_mpd.control & ET_WOL_DET_MPD_CTRL_EN_M) &&
	    !(wol_intf->wol_mpd.status & ET_WOL_DET_MPD_STAT_DETECTED))
	    status = "WOL";
	else
	    status = "Active";
        seq_printf(m, "Port %d in %s mode\n", port, status);

	/* If detected, activate the interface by disabling detection */
	if (wol_intf->wol_mpd.status & ET_WOL_DET_MPD_STAT_DETECTED)
	    disable_mpd_detection(port);
    }

    return 0;
}

void wol_enable(char *dev_name, wol_params_t *wol_params)
{
    ET_WOL_DET *wol_intf;
    uint32_t mac_h, mac_l;
    unsigned char *mac_addr;
    int port;
    uint32_t wol_offset;
#if defined(CONFIG_BCM96813)
    long wol_base = (long)xport_virt_base;
#else
    long wol_base = (long)sw_mmap_base.wol_base;
#endif

    port = get_dev_port_number(dev_name);

    if (port == -1) {
        printk("%s does not exist\n", dev_name);
	return;
    }

    if ((wol_offset = port_to_wol_offset[port]) == WOL_PORT_MISSING) {
        printk("%s does not support WoL\n", dev_name);
	return;
    }

    mac_addr = wol_params->mac_addr;

    /* configure the mac address for MPD */
    mac_h = (uint32_t)mac_addr[0] << 8  | (uint32_t)mac_addr[1];
    mac_l = (uint32_t)mac_addr[2] << 24 | (uint32_t)mac_addr[3] << 16 |
            (uint32_t)mac_addr[4] << 8  | mac_addr[5];

    wol_intf = (ET_WOL_DET *)(wol_base + wol_offset);
    wol_intf->wol_mpd.mac_hi = mac_h;
    wmb();
    wol_intf->wol_mpd.mac_low = mac_l;
    wmb();

    wol_intf->wol_ctl.control = ET_XPHY_WOL_CONTROL_WOL_EN_M |
      				ET_XPHY_WOL_CONTROL_WOL_RX_EN_M |
      				ET_XPHY_WOL_CONTROL_WOL_TX_EN_M;

    /* Clear the detection status bit before enabling it */
    wol_intf->wol_mpd.control &= ~ET_WOL_DET_MPD_CTRL_EN_M;
    wol_intf->wol_mpd.control |= ET_WOL_DET_MPD_CTRL_EN_M;
}
#endif /* WOL_HW_SWITCH */
