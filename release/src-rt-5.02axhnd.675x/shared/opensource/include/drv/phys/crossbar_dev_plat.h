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

#ifndef __CROSSBAR_DEV_PLAT_H__
#define __CROSSBAR_DEV_PLAT_H__

#include "bcm_map_part.h"

#define MAX_CROSSBARS 3
#define MAX_CROSSBAR_INT_ENDPOINTS 4
#define MAX_CROSSBAR_EXT_ENDPOINTS 6


/* III: move functions to C file ? */

typedef struct sw_port_s {
    int unit;
    int port;
} sw_port_t;

#if defined(CONFIG_5x3_CROSSBAR_SUPPORT) || defined(CONFIG_4x2_CROSSBAR_SUPPORT) || defined(CONFIG_3x2_CROSSBAR_SUPPORT) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM947622)

#if defined(CONFIG_5x3_CROSSBAR_SUPPORT) /* 63138B0 onwards 5x3 crossbar */
#define CB_PHY_PORT_MASK                    0x7
#define CB_PHY_PORT_SHIFT                   0x3
#else
#define CB_PHY_PORT_MASK                    0x3
#define CB_PHY_PORT_SHIFT                   0x2
#endif

#if defined(CONFIG_3x2_CROSSBAR_SUPPORT) || defined(CONFIG_4x2_CROSSBAR_SUPPORT) || defined(CONFIG_BCM947622)
#define CB0_INT_EPS     2
#else
#define CB0_INT_EPS     3
#endif

static inline int cb_sel_get_ext_ep(uint32_t val32, int int_ep)
{
    int ext_ep;
#if defined(CONFIG_BCM947622)
    if (int_ep)
        ext_ep = (val32&SYSPORT1_USE_RGMII) ? 1 : 2;
    else
        ext_ep = (val32&SYSPORT0_USE_RGMII) ? 1 : 0;
#else //!47622
#if defined(CONFIG_BCM963158)   /* Work around for 63158 twisted register mapping */
    if (int_ep == 2) int_ep = 3;
#endif
    ext_ep = (val32 >> (int_ep * CB_PHY_PORT_SHIFT)) & CB_PHY_PORT_MASK;
#if defined(CONFIG_BCM963158)   /* Work around for 63158 twisted register mapping */
    if (ext_ep == 2) ext_ep = 3; else
    if (ext_ep == 3) ext_ep = 2;
#endif
#endif //!47622
    return ext_ep;
}
static inline uint32_t cb_sel_set_ext_ep(uint32_t val32, int int_ep, int ext_ep)
{
#if defined(CONFIG_BCM947622)
    //restriction int_ep0 only can connect to ext_ep0, ext_ep1; and int_ep1 only can connect to ext_ep1, ext_ep2
    return (ext_ep == 1)? (val32 | 1 << int_ep) : (val32 & ~(1 << int_ep)) ;
#else //!47622
#if defined(CONFIG_BCM963158)   /* Work around for 63158 twisted register mapping */
    if (int_ep == 2) int_ep = 3;
    if (ext_ep == 2) ext_ep = 3; else
    if (ext_ep == 3) ext_ep = 2;
#endif
    val32 &= ~(CB_PHY_PORT_MASK << (int_ep * CB_PHY_PORT_SHIFT)); /* Reset config for the port */
    return val32 | (ext_ep & CB_PHY_PORT_MASK) << (int_ep * CB_PHY_PORT_SHIFT); 
#endif //!47622
}

/*
    crossbar_plat_select() setup hw connection between internal & external endpoint of a crossbar.
    caller can specify connection in two ways:
      1) specify crossbar_id, int_ep, and ext_ep,  and phy_dev as NULL,  or
      2) specify valid phy_dev that is connected to a crossbar ext_ep. 
*/
static int crossbar_plat_select(int crossbar_id, int int_ep, int ext_ep, phy_dev_t *phy_dev)
{
    uint32_t val32 = 0;
#if defined(CONFIG_BCM947622)
    volatile uint32_t *crb_reg = (void *)&(SYSPORT_MISC->SYSTEMPORT_MISC_CROSSBAR3X2_CONTROL);
#else
    volatile uint32_t *crb_reg = (void *)(SWITCH_CROSSBAR_REG);
#endif
    int i, old_ext_ep;

    if (phy_dev && crossbar_info_by_phy(phy_dev, &crossbar_id, &int_ep, &ext_ep))
    {
        printk("crossbar Mux: phy_dev is not valid\n");
        return -1;
    }
    
    printk("crossbar Mux: connect cb_idx:%d  int_ep %d to ext_ep %d\n", crossbar_id, int_ep, ext_ep);
    crossbar_set_active_external_endpoint(crossbar_id, int_ep, ext_ep);

    // ignore crossbar_id for now, since all devices contain just one configurable crossbar 
    val32 = *crb_reg;
    old_ext_ep = cb_sel_get_ext_ep(val32, int_ep);
    
    if (old_ext_ep == ext_ep)   // no change, no op
        return 0;
    val32 = cb_sel_set_ext_ep(val32, int_ep, ext_ep);   // set new ext_ep
    // scan for conflict, change the other int_ep to this int_ep's old ext_ep value
    for (i = 0; i < CB0_INT_EPS; i++)
        if (i != int_ep && cb_sel_get_ext_ep(val32, i) == ext_ep)
            val32 = cb_sel_set_ext_ep(val32, i, old_ext_ep);
    *crb_reg = val32; 

    return 0;
}

/*  two dimensional array indicating position of crossbar internal endpoint relating to unit/port.
        1st dimension - number of crossbars ( currently only one)
        2nd dimension - number of internal endpoints within crossbar
                        terminated by {-1, N}   where N indicates number of external endpoints for this crossbar.
 */
static const sw_port_t crossbar_plat_int_endpoints[MAX_CROSSBARS][MAX_CROSSBAR_INT_ENDPOINTS] = {
#if defined(CONFIG_BCM963158)
    {{1, 4}, {1, 6}, {0, 5}, {-1, 4}},
#elif defined(CONFIG_5x3_CROSSBAR_SUPPORT)
    {{1, 3}, {1, 4}, {0, 0}, {-1, 5}},
#elif defined(CONFIG_BCM947622)
    {{0, 0}, {0, 1}, {-1, 3}},
#elif defined(CONFIG_3x2_CROSSBAR_SUPPORT)
    {{1, 7}, {0, 3}, {-1, 3}},
#elif defined(CONFIG_4x2_CROSSBAR_SUPPORT)
    {{1, 4}, {0, 0}, {-1, 4}},
#endif
    {{-1}}
};


#if defined(CRB_5X3_QGPHY3_WORKAROUND)
int crossbar_external_to_internal_endpoint(int crossbar_id, int external_endpoint);
int sf2_set_mac_port_state(int phy_port, int link, int speed, int duplex);
void sf2_force_mac_up(int port);

static inline void _crossbar_plat_qgphy3_work_around(void)
{   // get QGPHY3 (cb_idx:0 ext_ep:4) int_ep connection
    int internal_endpoint = crossbar_external_to_internal_endpoint(0, 4);

    // if QGPHY3 is connected to SF2 port3 (cb_idx:0 int_ep:0), or not connected, no issue
    if (internal_endpoint == 0 || internal_endpoint == -1)
        return;

    // get SGPHY4 (cb_idx:0 ext_ep:1) int_ep connection
    internal_endpoint = crossbar_external_to_internal_endpoint(0, 1);
    
    // if SGPHY4 is connected to SF2 port3, big problem
    if (internal_endpoint == 0)
    {
        printk ("***** Error Board Configuration: QGPHY3 is not connected to P3, SGPHY4 is member of P3\n");
        printk ("         If SGPHY4 is not linked up at 1Gbps, QGPHY3 will not work in 1Gbps link\n");
        BUG();
    }
    
    // if SF2 port3 is used (not connected to QGPHY3 or SGPHY4), bigger problem
    if (crossbar_phy_dev_first(crossbar_group(0, 0)))
    {
        printk ("***** Error Board Configuration: QGPHY3 is not connected to P3, SGPHY4 is not member of P3\n");
        printk ("         P3 is being used. QGPHY3 will not work in 1Gbps link\n");
        BUG();
    }
    
    // if SGPHY4 is used, also a problem
    if (internal_endpoint != -1)
    {
        printk ("***** Error Board Configuration: QGPHY3 is not connected to P3, SGPHY4 is not member of P3\n");
        printk ("         P3 is not being used, but SGPHY4 is being used. QGPHY3 will not work in 1Gbps link\n");
        BUG();
    }
    
    // both SF2 port3 and SGPHY4 are not being used, add software work around to connect them in 1G
    printk (" Work around hardware limitation by connecting unused P3 to SGPHY4 for QPHY3 to work correctly\n");
    crossbar_plat_select(0, 0, 1, NULL);
    
    sf2_set_mac_port_state(3, 1, 1000, 1);

    /* Turn off SF2 port 3 Lower Power Saving */
    sf2_force_mac_up(3);
}
#endif

int crossbar_get_int_ext_mapping(int crossbar_id, int max_internal_endpoint, int max_external_endpoint, int *endpoint_pairs);

static int crossbar_plat_finalize(void)
{
    const sw_port_t *sp;
    int id, endpoint;
    int max_int_ep, max_ext_ep;
    int endpoint_pairs[MAX_CROSSBAR_INT_ENDPOINTS];

    // do crossbar configuration
    for (id = 0, sp = crossbar_plat_int_endpoints[id]; id < MAX_CROSSBARS && sp->unit != -1; id++)
    {   // find max internal endpoint
        for (endpoint = 0; endpoint < MAX_CROSSBAR_INT_ENDPOINTS && sp->unit != -1; endpoint++, sp++);
        max_int_ep = endpoint; 
        max_ext_ep = sp->port;
        crossbar_get_int_ext_mapping(id, max_int_ep, max_ext_ep, endpoint_pairs);
        for (endpoint = 0; endpoint < max_int_ep; endpoint++)
            crossbar_plat_select(id, endpoint, endpoint_pairs[endpoint], NULL);
    }
#if defined(CRB_5X3_QGPHY3_WORKAROUND)
    _crossbar_plat_qgphy3_work_around();
#endif
    return 0;
}
#else
static int crossbar_plat_select(int crossbar_id, int int_ep, int ext_ep, phy_dev_t *phy_dev)
{
    printk("crossbar: connect endpoints int %d to ext %d\n", int_ep, ext_ep);
    return 0;
}

static const sw_port_t crossbar_plat_int_endpoints[MAX_CROSSBARS][MAX_CROSSBAR_INT_ENDPOINTS] = {
    {{0, 1}, {-1}},
    {{-1}}
};

static int crossbar_plat_finalize(void)
{
    return 0;
}
#endif

static inline int crossbar_phy_moveable(phy_dev_t *phy_dev_crossbar, phy_dev_t *phy_dev, int external_endpoint)
{
#if defined(CONFIG_BCM947622)
    // 47622 only has one crossbar - only 2nd phy endpoint (RGMII) is moveable
    return (external_endpoint == 1);
#else
    return 1;
#endif
}

#endif

