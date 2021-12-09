/*
   Copyright 2007-2010 Broadcom Corp. All Rights Reserved.

   <:label-BRCM:2011:DUAL/GPL:standard

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

#define _BCMENET_LOCAL_
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/nbuff.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethswdefs.h"
#include "ethsw.h"
#include "bcmswshared.h"
#include "ethsw_phy.h"
#include "bcmswaccess.h"
#include "bcmsw.h"
#include "eth_pwrmngt.h"

#define ADVERTISE_REPEATER	0x0400

extern struct semaphore bcm_ethlock_switch_config;
extern uint8_t port_in_loopback_mode[TOTAL_SWITCH_PORTS];
extern atomic_t phy_write_ref_cnt;
extern atomic_t phy_read_ref_cnt;
extern int vport_cnt;  /* number of vports: bitcount of Enetinfo.sw.port_map */

static uint8_t  hw_switching_state = HW_SWITCHING_ENABLED;

extern extsw_info_t extSwInfo;

void ethsw_init_table(BcmEnet_devctrl *pDevCtrl)
{
}

int ethsw_set_mac(int logical_port, PHY_STAT ps)
{
    /* WARNING - this must NOT be called for external switch ports */
    uint16 sw_port = LOGICAL_PORT_TO_PHYSICAL_PORT(logical_port);

    if ( (LOGICAL_PORT_TO_UNIT_NUMBER(logical_port) != 0) || /* Internal switch or Runner port */
         (port_in_loopback_mode[sw_port]) )
    {
        printk("Ext switch or port_in_loopback_mode 0x%02x\n", logical_port);
        return 0;
    }

    return ethsw_set_mac_hw(sw_port, ps);
}

int ethsw_set_rx_tx_flow_control(int logical_port, int rxEn, int txEn)
{
    unsigned char v8;
    uint16        sw_port = LOGICAL_PORT_TO_PHYSICAL_PORT(logical_port);

    down(&bcm_ethlock_switch_config);
    if ( 0 == (LOGICAL_PORT_TO_UNIT_NUMBER(logical_port)) )
    {
        ethsw_rreg(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1);
    }
    else
    {
        extsw_rreg(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1);
    }
    v8 &= ~(REG_PORT_STATE_TX_FLOWCTL | REG_PORT_STATE_RX_FLOWCTL);
    if ( rxEn )
    {
       v8 |= REG_PORT_STATE_RX_FLOWCTL;
    }
    if ( txEn )
    {
       v8 |= REG_PORT_STATE_TX_FLOWCTL;
    }

    if ( 0 == (LOGICAL_PORT_TO_UNIT_NUMBER(logical_port)) )
    {
        ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1);
    }
    else
    {
        extsw_wreg(PAGE_CONTROL, REG_PORT_STATE + sw_port, &v8, 1);
    }
    up(&bcm_ethlock_switch_config);

    return 0;
}



void ethsw_switch_power_off(void *context)
{
#ifdef DYING_GASP_API
    enet_send_dying_gasp_pkt();
#endif
}

// end power management routines

void ethsw_phyport_rreg(int port, int reg, uint16 *data)
{
    int unit = LOGICAL_PORT_TO_UNIT_NUMBER(port);
    int phy_id = enet_logport_to_phyid(port);
    int phys_port;

    if (unit > 0 && (pVnetDev0_g->extSwitch->accessType == MBUS_SPI || pVnetDev0_g->extSwitch->accessType == MBUS_HS_SPI))
    {
        phys_port = LOGICAL_PORT_TO_PHYSICAL_PORT(port);
        extsw_rreg_wrap(PAGE_INTERNAL_PHY_MII + phys_port, reg*2, (uint8 *)data, 2);
    }
    else
    {
        ethsw_phy_rreg(phy_id, reg, data);
    }
}

void ethsw_phyport_wreg(int port, int reg, uint16 *data)
{
    int unit = LOGICAL_PORT_TO_UNIT_NUMBER(port);
    int phy_id = enet_logport_to_phyid(port);
    int phys_port;

    if (unit > 0 && (pVnetDev0_g->extSwitch->accessType == MBUS_SPI || pVnetDev0_g->extSwitch->accessType == MBUS_HS_SPI))
    {
        phys_port = LOGICAL_PORT_TO_PHYSICAL_PORT(port);
        extsw_wreg_wrap(PAGE_INTERNAL_PHY_MII+phys_port, reg*2, (uint8 *)data, 2);
    }
    else
    {
        ethsw_phy_wreg(phy_id, reg, data);
    }
}

/*
 * Clause 45 register read
 * port argument passed is logical port.
 *
 */

void ethsw_phyport_c45_rreg(int log_port, int regg, int regr, uint16 *pdata16) {
   uint16 val16;
   val16 = regg;
   ethsw_phyport_wreg(log_port, 0x0d, &val16);
   val16 = regr;
   ethsw_phyport_wreg(log_port, 0x0e, &val16);
   val16 = 0x4000 | regg;
   ethsw_phyport_wreg(log_port, 0x0d, &val16);
   ethsw_phyport_rreg(log_port, 0x0e, pdata16);
}

/*
 * Clause 45 register writes
 * port argument passed is logical port.
 *
 */
void ethsw_phyport_c45_wreg(int log_port, int regg, int regr, uint16 *pdata16) {
   uint16 val16;
   val16 = regg;
   ethsw_phyport_wreg(log_port, 0x0d, &val16);
   val16 = regr;
   ethsw_phyport_wreg(log_port, 0x0e, &val16);
   val16 = 0x4000 | regg;
   ethsw_phyport_wreg(log_port, 0x0d, &val16);
   ethsw_phyport_wreg(log_port, 0x0e, pdata16);
}


/*
 * Clause 45 register read
 * argument is phy_id
 *
 */

void ethsw_phy_c45_rreg(int phy_id, int regg, int regr, uint16 *pdata16) {
   uint16 val16;
   val16 = regg;
   ethsw_phy_wreg(phy_id, 0x0d, &val16);
   val16 = regr;
   ethsw_phy_wreg(phy_id, 0x0e, &val16);
   val16 = 0x4000 | regg;
   ethsw_phy_wreg(phy_id, 0x0d, &val16);
   ethsw_phy_rreg(phy_id, 0x0e, pdata16);
}

/*
 * Clause 45 register writes
 * argument is phy_id
 *
 */
void ethsw_phy_c45_wreg(int phy_id, int regg, int regr, uint16 *pdata16) {
   uint16 val16;
   val16 = regg;
   ethsw_phy_wreg(phy_id, 0x0d, &val16);
   val16 = regr;
   ethsw_phy_wreg(phy_id, 0x0e, &val16);
   val16 = 0x4000 | regg;
   ethsw_phy_wreg(phy_id, 0x0d, &val16);
   ethsw_phy_wreg(phy_id, 0x0e, pdata16);
}

/*
 **  caution: when unit = 0; the phy_ids for Internal and External PHY
 **  could be duplicated duplicated,  thus the further restriction  mapping of
 ** phy id -> phys_port  is not unique.
 */
static int ethsw_phyid_to_phys_port(int phy_id, int unit)
{
    int i;

    for (i = 0; i < MAX_SWITCH_PORTS &&
            (pVnetDev0_g->EnetInfo[unit].sw.phy_id[i] & 0x1f) != (phy_id & 0x1f) ; i++);

    if (i == MAX_SWITCH_PORTS)
    {
        BCM_ENET_DEBUG("%s phy-to-port association not found \n", __FUNCTION__);
        return -1;
    }

    return i;
}

int ethsw_phyport_rreg32(int phy_id, u32 reg, u32 *data, int flags)
{
    int unit = (flags & ETHCTL_FLAG_ACCESS_EXTSW_PHY)? 1: 0;
    int phys_port, rc = 0;

    if (unit > 0 && pVnetDev0_g->extSwitch->accessType != MBUS_MDIO)
    {
        phys_port = ethsw_phyid_to_phys_port(phy_id, unit);
        if(phys_port != -1)
        {
            uint16 _data; 
            extsw_rreg_wrap(PAGE_INTERNAL_PHY_MII + phys_port,
                    reg*2, &_data, 2);
            *data = _data;
        }
    }
    else
    {
        rc = ethsw_phy_rreg32(phy_id, reg, data, flags);
    }
    return rc;
}

int ethsw_phyport_rreg2(int phy_id, int reg, uint16 *data, int flags)
{
    u32 _data;
    int rc;

    rc = ethsw_phyport_rreg32(phy_id, reg, &_data, flags);
    *data = _data;
    return rc;
}

int ethsw_phyport_wreg32(int phy_id, int reg, u32 *data, int flags)
{
    int unit = (flags & ETHCTL_FLAG_ACCESS_EXTSW_PHY)? 1: 0;
    int phys_port, rc = 0;

    if (unit > 0 && pVnetDev0_g->extSwitch->accessType != MBUS_MDIO)
    {
        phys_port = ethsw_phyid_to_phys_port(phy_id, unit);
        if(phys_port != -1)
        {
            uint16 _data = *data; 
            extsw_wreg_wrap(PAGE_INTERNAL_PHY_MII + phys_port, reg*2, (uint8 *)&_data, 2);
        }
    }
    else
    {
        rc = ethsw_phy_wreg32(phy_id, reg, data, flags);
    }
    return rc;
}

int ethsw_phyport_wreg2(int phy_id, int reg, uint16 *data, int flags)
{
    u32 _data = *data;
    int rc;
    rc = ethsw_phyport_wreg32(phy_id, reg, &_data, flags);
    return rc;
}

static void ethsw_phy_advertise_caps_each(int phy_id, int *mii_cap, int *gmii_cap)
{
    uint16 cap_mask;
    *mii_cap = *gmii_cap = 0;

    if(!IsPhyConnected(phy_id) || !IsPhyAdvCapConfigValid(phy_id))
        return;

    ethsw_phy_rreg(phy_id, MII_ADVERTISE, &cap_mask);
    cap_mask &= (~ADVERTISE_ALL);
    if (phy_id & ADVERTISE_10HD)
        cap_mask |= ADVERTISE_10HALF;
    if (phy_id & ADVERTISE_10FD)
        cap_mask |= ADVERTISE_10FULL;
    if (phy_id & ADVERTISE_100HD)
        cap_mask |= ADVERTISE_100HALF;
    if (phy_id & ADVERTISE_100FD)
        cap_mask |= ADVERTISE_100FULL;
    *mii_cap = cap_mask;
    ethsw_phy_wreg(phy_id, MII_ADVERTISE, &cap_mask);

    ethsw_phy_rreg(phy_id, MII_CTRL1000, &cap_mask);
    cap_mask &= (~(ADVERTISE_1000HALF | ADVERTISE_1000FULL));
    cap_mask |= ADVERTISE_REPEATER; /* Favor clock master for better compatibility when in EEE */
    if (phy_id & ADVERTISE_1000HD)
        cap_mask |= ADVERTISE_1000HALF;
    if (phy_id & ADVERTISE_1000FD)
        cap_mask |= ADVERTISE_1000FULL;
    *gmii_cap = cap_mask;
    ethsw_phy_wreg(phy_id, MII_CTRL1000, &cap_mask);
}

static void ethsw_phy_advertise_caps (void)
{
    unsigned int portmap, port, phy_id, unit, log_port, cb_port, mii_cap, gmii_cap;
    ETHERNET_MAC_INFO *info = EnetGetEthernetMacInfo();

    /* Now control advertising if boardparms says so */
    for (unit=0; unit < BP_MAX_ENET_MACS; unit++)
    {
        portmap = info[unit].sw.port_map;
        for (port = 0; portmap && port < (TOTAL_SWITCH_PORTS - 1); port++)
        {
            if ((portmap & (1U<<port)) == 0)
                continue;

            log_port = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
            cb_port = enet_get_first_crossbar_port(log_port);
            if (cb_port == BP_CROSSBAR_NOT_DEFINED)
            {
                phy_id = info[unit].sw.phy_id[port];
                ethsw_phy_advertise_caps_each(phy_id, &mii_cap, &gmii_cap);
                if (mii_cap)
                    printk("%s switch port %d; Adv capability change : MII=0x%04x, GMII=0x%04x\n",
                        (unit?"Ext":"Int"), (unsigned int)port, mii_cap, gmii_cap);
            }
            else
            {
                for (;cb_port != BP_CROSSBAR_NOT_DEFINED;
                        cb_port = enet_get_next_crossbar_port(log_port, cb_port))
                {
                    phy_id = info[unit].sw.crossbar[cb_port].phy_id;
                    ethsw_phy_advertise_caps_each(phy_id, &mii_cap, &gmii_cap);
                    if (mii_cap)
                        printk("Cross bar port %d of %s switch port %d; Adv capability change : MII=0x%04x, GMII=0x%04x\n", 
                                cb_port, (unit?"Ext":"Int"), (unsigned int)port, mii_cap, gmii_cap);
                }
            }
        }
    }
}

void ethsw_phy_config()
{
    ethsw_setup_led();
#if defined(CONFIG_BCM_ETH_HWAPD_PWRSAVE)
    ethsw_setup_hw_apd(1);
#endif

    ethsw_setup_phys();

    ethsw_phy_handle_exception_cases();

    ethsw_phy_advertise_caps();

    ethsw_phy_apply_init_bp();
}

void ethsw_init_config(int unit, uint32_t port_map,  int wanPort)
{
    bcmeapi_ethsw_init_hw(unit, port_map, wanPort);

    /* Initialize the Internal switch config */
    bcmeapi_ethsw_init_config();

    /* Initialize the external switch config */
    extsw_init_config();
}

int bcmeapi_ioctl_ethsw_port_mirror_get(struct ethswctl_data *e)
{
    e->port_mirror_cfg.tx_port = -1;
    e->port_mirror_cfg.rx_port = -1;

    if (e->unit == 0)
    {
        ethsw_port_mirror_get(&e->port_mirror_cfg.enable, &e->port_mirror_cfg.mirror_port, 
            &e->port_mirror_cfg.ing_pmap, &e->port_mirror_cfg.eg_pmap,
            &e->port_mirror_cfg.blk_no_mrr, &e->port_mirror_cfg.tx_port, &e->port_mirror_cfg.rx_port);
    }
    else if (bcm63xx_enet_isExtSwPresent())
    {
        bcmsw_port_mirror_get(&e->port_mirror_cfg.enable, &e->port_mirror_cfg.mirror_port, 
            &e->port_mirror_cfg.ing_pmap, &e->port_mirror_cfg.eg_pmap, &e->port_mirror_cfg.blk_no_mrr);
    }
    else
    {
        printk(" Error_get: this router does not have external switch\n");
        return BCM_E_ERROR;
    }
    return 0;
}

int bcmeapi_ioctl_ethsw_port_mirror_set(struct ethswctl_data *e)
{
    if (e->unit == 0)
    {
        ethsw_port_mirror_set(e->port_mirror_cfg.enable,e->port_mirror_cfg.mirror_port,
                              e->port_mirror_cfg.ing_pmap,e->port_mirror_cfg.eg_pmap,
                              e->port_mirror_cfg.blk_no_mrr,
                              e->port_mirror_cfg.tx_port,
                              e->port_mirror_cfg.rx_port);
    }
    else if (bcm63xx_enet_isExtSwPresent())
    {
        bcmsw_port_mirror_set(e->port_mirror_cfg.enable,e->port_mirror_cfg.mirror_port,
                              e->port_mirror_cfg.ing_pmap,e->port_mirror_cfg.eg_pmap,
                              e->port_mirror_cfg.blk_no_mrr);
    }
    else
    {
        printk(" Error_set: this router does not have external switch\n");
        return BCM_E_ERROR;
    }
    return 0;
}
int bcmeapi_ioctl_ethsw_port_trunk_set(struct ethswctl_data *e)
{
    if (bcm63xx_enet_isExtSwPresent())
    {
        bcmsw_port_trunk_set(e->port_trunk_cfg.hash_sel);
    }
    else
    {
        printk(" Error_set: this router does not have external switch\n");
        return BCM_E_ERROR;
    }
    return 0;
}
int bcmeapi_ioctl_ethsw_port_trunk_get(struct ethswctl_data *e)
{
    if (bcm63xx_enet_isExtSwPresent())
    {
        bcmsw_port_trunk_get(&e->port_trunk_cfg.enable, &e->port_trunk_cfg.hash_sel,
                             &e->port_trunk_cfg.grp0_pmap, &e->port_trunk_cfg.grp1_pmap);
    }
    else
    {
        printk(" Error_set: this router does not have external switch\n");
        return BCM_E_ERROR;
    }
    return 0;
}
/*
 * Function:
 *      bcmeapi_ioctl_ethsw_arl_access
 * Purpose:
 *      ARL table accesses
 * Returns:
 *      BCM_E_XXX
 */
int bcmeapi_ioctl_ethsw_arl_access(struct ethswctl_data *e)
{
    int ret;

    switch(e->type)
    {
        case TYPE_GET:
            BCM_ENET_DEBUG("e->mac: %02x %02x %02x %02x %02x %02x", e->mac[5],
                    e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0]);
            BCM_ENET_DEBUG("e->vid: %d", e->vid);

            switch (e->unit)
            {
                case 0:
                    ret = enet_arl_read( e->mac, &e->vid, &e->val );
                    break;
                case 1:
                    if(!bcm63xx_enet_isExtSwPresent())
                    {
                        printk(" Error: this router does not have external switch\n");
                        return BCM_E_ERROR;
                    }
                    ret = enet_arl_read_ext( e->mac, &e->vid, &e->val );
                    break;
                default:
                    e->unit = 0;
                    if ((ret = enet_arl_read( e->mac, &e->vid, &e->val)))
                    {
                        break;
                    }

                    if (bcm63xx_enet_isExtSwPresent()) {
                        e->unit = 1;
                        ret = enet_arl_read_ext( e->mac, &e->vid, &e->val );
                    }
            }

            if (ret == FALSE)
            {
                return BCM_E_ERROR;
            }
            break;

        case TYPE_SET:
            BCM_ENET_DEBUG("e->mac: %02x %02x %02x %02x %02x %02x", e->mac[5],
                    e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0]);
            BCM_ENET_DEBUG("e->vid: %d", e->vid);

            /* if an external switch is present, e->unit will determine the
               access function */
            if (e->unit == 1)
            {
                if (bcm63xx_enet_isExtSwPresent())
                {
                    if(e->vid == 0xffff && (e->val & ARL_DATA_ENTRY_VALID) == 0)
                    {
                        enet_arl_remove_ext(e->mac);
                    }
                    else
                    {
                        enet_arl_write_ext(e->mac, e->vid, e->val);
                    }
                }
                else
                {
                    printk(" Error: No External Switch in this Router.\n");
                }
            }
            else
            {
                if(e->vid == 0xffff && (e->val & ARL_DATA_ENTRY_VALID) == 0)
                {
                    enet_arl_remove(e->mac);
                }
                else
                {
                    enet_arl_write(e->mac, e->vid, e->val);
                }
            }
            break;

        case TYPE_DUMP:
            enet_arl_dump();
            enet_arl_dump_multiport_arl();
            if (bcm63xx_enet_isExtSwPresent())
            {
                enet_arl_dump_ext();
                enet_arl_dump_ext_multiport_arl();
            }
            break;

        case TYPE_FLUSH:
            /* Flush the ARL table */
            fast_age_all(0);
            if (bcm63xx_enet_isExtSwPresent()) {
                fast_age_all_ext(0);
            }
            break;

        default:
            return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

int ethsw_set_hw_switching(uint32 state)
{
    down(&bcm_ethlock_switch_config);
    /*Don't do anything if already enabled/disabled.
     *Enable is implemented by restoring values saved by disable_hw_switching().
     *This check is necessary to make sure we get correct behavior when
     *enable_hw_switching() is called without a preceeding disable_hw_switching() call.
     */

    if (hw_switching_state != state) {
        if (bcm63xx_enet_isExtSwPresent()) {
            if (state == HW_SWITCHING_ENABLED) {
                bcmsw_enable_hw_switching();
            }
            else {
                bcmsw_disable_hw_switching();
            }
        }
        else {
            if (state == HW_SWITCHING_ENABLED) {
                ethsw_enable_hw_switching();
            }
            else {
                ethsw_disable_hw_switching();
            }
        }
        hw_switching_state = state;
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
int ethsw_get_hw_switching_state(void)
{
    return hw_switching_state;
}
static void restart_autoneg(int phyid, int unit)
{
    uint16_t v16;

    /* read control register */
    ethsw_phy_read_reg(phyid, MII_BMCR, &v16, unit);
    BCM_ENET_DEBUG("MII_BMCR Read Value = %4x", v16);

    /* Write control register wth AN_EN and RESTART_AN bits set */
    v16 |= (BMCR_ANENABLE | BMCR_ANRESTART);
    BCM_ENET_DEBUG("MII_BMCR Written Value = %4x", v16);
    ethsw_phy_write_reg(phyid, MII_BMCR, &v16, unit);
}

static void set_pause_capability(int unit, int port, int req_flow_ctrl)
{
    uint16_t an_adv, v16, bmcr;
    uint32_t override_val;
    int phyid =  BCMSW_PHY_GET_PHYID(unit, port);
    int start_port, end_port;

    if (port == SWITCH_PORTS_ALL_PHYS) {
       start_port = 0;
       end_port = EPHY_PORTS-1;
    } else {
       start_port = end_port = port;
    }
    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG("given req_flow_ctrl = %4x", req_flow_ctrl);
    if (unit == 0) {
        ethsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&override_val, 4);
    } else {
        extsw_rreg_wrap(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&override_val, 4);
    }
    BCM_ENET_DEBUG("override_val read = %4x", (unsigned int)override_val);

    for (port = start_port; port <= end_port; port++) {
        override_val &= (~((1 << port) | (1 << (port + TOTAL_SWITCH_PORTS))));
        /* resolve pause mode and advertisement
         * Please refer to Table 28B-3 of the 802.3ab-1999 spec */
        switch (req_flow_ctrl) {
            case PAUSE_FLOW_CTRL_AUTO:
            case PAUSE_FLOW_CTRL_BOTH:
            case PAUSE_FLOW_CTRL_BCMSWITCH_ON:
                v16 = (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
                override_val |= ((1 << port) | (1 << (port +TOTAL_SWITCH_PORTS)));
                break;

            case PAUSE_FLOW_CTRL_TX:
                v16 = ADVERTISE_PAUSE_ASYM;
                override_val |= (1 << port);
                break;

            case PAUSE_FLOW_CTRL_RX:
                v16 = ADVERTISE_PAUSE_CAP;
                override_val |= (1 << (port +TOTAL_SWITCH_PORTS));
                break;

            case PAUSE_FLOW_CTRL_BCMSWITCH_OFF:
                override_val &= ~REG_PAUSE_CAPBILITY_OVERRIDE;
                break;

            case PAUSE_FLOW_CTRL_NONE:
            default:
                v16 = 0;
                break;
        }

        if (req_flow_ctrl != PAUSE_FLOW_CTRL_BCMSWITCH_OFF) {
            phyid = BCMSW_PHY_GET_PHYID(unit,port);
            if ((req_flow_ctrl != PAUSE_FLOW_CTRL_BCMSWITCH_ON) && (port < EPHY_PORTS) && (phyid != -1)) {
                ethsw_phy_read_reg(phyid, MII_BMCR, &bmcr, unit);
                if (bmcr & BMCR_ANENABLE) {
                    ethsw_phy_read_reg(phyid, MII_ADVERTISE, &an_adv, unit);
                    BCM_ENET_DEBUG("an_adv read from PHY = %4x", an_adv);
                    an_adv &= ~(ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
                    an_adv |= v16;
                    BCM_ENET_DEBUG("an_adv written to PHY = %4x", an_adv);
                    ethsw_phy_write_reg(phyid, MII_ADVERTISE, &an_adv, unit);
                    restart_autoneg(phyid, unit);
                } else {
                    override_val |= REG_PAUSE_CAPBILITY_OVERRIDE;
                }
            } else {
                override_val |= REG_PAUSE_CAPBILITY_OVERRIDE;
            }
        }
    } // for all ports

    BCM_ENET_DEBUG("val written to REG_PAUSE_CAPABILITY = %4x",
                   (unsigned int)override_val);
    if (unit == 0) {
        ethsw_wreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&override_val, 4);
    } else {
        extsw_wreg_wrap(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&override_val, 4);
    }

    up(&bcm_ethlock_switch_config);

}

static void get_pause_capability(int unit, int port, int *flow_ctrl)
{
    uint16_t an_adv, v16;
    uint32_t val;
    int phyid = BCMSW_PHY_GET_PHYID(unit, port);

    down(&bcm_ethlock_switch_config);

    if (unit == 0) {
        ethsw_rreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&val, 4);
    } else {
        extsw_rreg_wrap(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8_t *)&val, 4);
    }
    if (val & REG_PAUSE_CAPBILITY_OVERRIDE) {
    if ((val & (1 << port)) && (val & (1 << (port +TOTAL_SWITCH_PORTS)))) {
        *flow_ctrl = PAUSE_FLOW_CTRL_BOTH;
        } else if (val & (1 << port)) {
        *flow_ctrl = PAUSE_FLOW_CTRL_TX;
        } else if (val & (1 << (port + TOTAL_SWITCH_PORTS))) {
        *flow_ctrl = PAUSE_FLOW_CTRL_RX;
        } else {
        *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
        }
    } else if ((port < EPHY_PORTS) && (phyid != -1)) {
        ethsw_phy_read_reg(phyid, MII_BMCR, &v16, unit);
        if (v16 & BMCR_ANENABLE) {
            /*  Read ANAR */
            ethsw_phy_read_reg(phyid, MII_ADVERTISE, &an_adv, unit);
            BCM_ENET_DEBUG("an_adv = %4x", an_adv);

          switch (an_adv & (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM)) {
                case (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM):
                *flow_ctrl = PAUSE_FLOW_CTRL_BOTH;
                break;

                case ADVERTISE_PAUSE_ASYM:
                    *flow_ctrl = PAUSE_FLOW_CTRL_TX;
                    break;

                case ADVERTISE_PAUSE_CAP:
                    *flow_ctrl = PAUSE_FLOW_CTRL_RX;
                    break;

                default:
                    *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
                    break;
          }
        } else {
            *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
        }
    } else {
        *flow_ctrl = PAUSE_FLOW_CTRL_NONE;
    }
    BCM_ENET_DEBUG("*flow_ctrl = %4x", *flow_ctrl);

    up(&bcm_ethlock_switch_config);
}
int bcmeapi_ioctl_ethsw_port_pause_capability (struct ethswctl_data *e)
{
    if ((e->port >= TOTAL_SWITCH_PORTS && e->port != SWITCH_PORTS_ALL_PHYS) ||
        (e->type == TYPE_GET && e->port == SWITCH_PORTS_ALL_PHYS)) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        get_pause_capability(e->unit, e->port, &e->val);
    } else {
        set_pause_capability(e->unit, e->port, e->val);
    }

    return 0;
}

MODULE_LICENSE("GPL");
