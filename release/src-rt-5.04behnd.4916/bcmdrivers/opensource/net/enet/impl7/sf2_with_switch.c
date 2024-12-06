/*
   <:copyright-BRCM:2018:DUAL/GPL:standard

      Copyright (c) 2018 Broadcom
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
 *  Created on: May/2018
 *      Author: steven.hsieh@broadcom.com
 */

#include "bcmenet_common.h"
#include "mux_index.h"

#include "sf2.h"
#include "sf2_common.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
#include "archer_enet.h"
#include "shared_utils.h"   // for UtilGetChipRev()
#include <linux/bcm_skb_defines.h>

// =========== sf2 port ops =============================
static int _port_sf2_port_init(enetx_port_t *self)
{
#ifdef CONFIG_BLOG
    if (self->has_interface) {
        int blog_chnlX = root_sw->n.blog_chnl++;
        blog_chnl_set(BLOG_ENETPHY, blog_chnlX, blog_chnlX, self);

        if (mux_set_rx_index(self->p.parent_sw, self->n.blog_chnl, self))
            return -1;

        /* also register demux at root for receive processing if port not on root sw */
        if (!PORT_ON_ROOT_SW(self))
            if (mux_set_rx_index(root_sw, self->n.blog_chnl, self))
                return -1;
    }
#endif

    if (self->p.child_sw) {
        int unit = PORT_ON_ROOT_SW(self)?0:1;
        int id = self->port_info.port;
        uint32_t val32;
        uint8_t  val8;

        // setup lightstacking control
        if (!self->port_info.swtag_use_pvid) {
            extsw_rreg_wrap(unit, PAGE_CONTROL, REG_LIGHTSTACK_CTRL, &val32, sizeof(val32));
            val32 |= REG_LSTACK_EN | REG_LSTACK_MASTER | REG_LSTACK_PORT0_LEGACY |
#if defined(CONFIG_BCM96756)
                     (UtilGetChipRev() == 0xA0 ? REG_LSTACK_FWD_MAP2_WORKAROUND : 0) | 
#endif
                     (id << REG_LSTACK_PORT0_SHIFT);
            extsw_wreg_wrap(unit, PAGE_CONTROL, REG_LIGHTSTACK_CTRL, &val32, sizeof(val32));
            enet_dbg("lightstacking ctrl = %x\n", val32);
        }

        // set STP state to forward
        extsw_rreg_wrap(unit, PAGE_CONTROL, REG_PORT_CTRL+id, &val8, sizeof(val8));
        val8 &= ~REG_PORT_STP_MASK;
        val8 |= REG_PORT_STP_STATE_FORWARDING;
        extsw_wreg_wrap(unit, PAGE_CONTROL, REG_PORT_CTRL+id, &val8, sizeof(val8));

        // init txq schedule to all WRR
        extsw_rreg_wrap(unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0+id, &val8, sizeof(val8));
        val8 &= ~PN_QOS_SCHED_SEL_M;
        val8 |= SF2_ALL_Q_WRR;
        extsw_wreg_wrap(unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0+id, &val8, sizeof(val8));
    }
    enet_dbg("Initialized %s role %s\n", self->obj_name, (self->n.port_netdev_role==PORT_NETDEV_ROLE_WAN)?"wan":"lan" );
    
    if (self->p.phy && self->p.phy->cascade_next && 
        self->p.phy->cascade_next->flag & PHY_FLAG_ON_MEZZANINE && self->p.phy->cascade_next->flag & PHY_FLAG_NOT_PRESENTED) {
        enet_err("mark mezzine card not present\n");
        self->p.phy = NULL;
    }
    return 0;
}

// based on impl5\bcmenet_runner_inline.h:bcmeapi_pkt_xmt_dispatch()
static int dispatch_pkt_sf2_lan(dispatch_info_t *dispatch_info)
{
    int rc;
    /* TODO : hardcode the dispatch is through LAN interface for now */
    rc = cpu_queues_tx_send (LAN_CPU_TX, dispatch_info);

    if (rc < 0)
    {
        /* skb is already released by rdpa_cpu_tx_port_enet_lan() or cpu_queues_tx_send() */
        INC_STAT_DBG(dispatch_info->port,tx_dropped_accelerator_lan_fail);   /* don't increment tx_dropped, which is incremented by caller */
        return -1;
    }

    return 0;
}

static int sf2_ext_role_not_lan_map;  // set port bitmap on external switch when port is wan or soft-switching
static int _port_sw_port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    if (self->p.parent_sw == sf2_sw_ext && self->has_interface) {
        int new_not_lan_map = sf2_ext_role_not_lan_map & ~(1<<self->port_info.port);
        enetx_port_t *ls_master = sf2_sw_ext->s.parent_port;

        if (role != PORT_NETDEV_ROLE_LAN)
            new_not_lan_map |= 1<<self->port_info.port;

        if (!sf2_ext_role_not_lan_map && new_not_lan_map) {
            // when ext_sw has 1st wan port or soft switching 
            enet_dbgv("internal switch lightstacking port l2 switching: disable\n");
            ls_master->p.ops->role_set(ls_master, PORT_NETDEV_ROLE_NONE);
            // clear out arl entries for this port
            ls_master->p.ops->fast_age(ls_master);
        } else if (!new_not_lan_map && sf2_ext_role_not_lan_map) {
            // when all ext_sw ports are lan and hw-switching.
            enet_dbgv("internal switch lightstacking port l2 switching: enable\n");
            ls_master->p.ops->role_set(ls_master, PORT_NETDEV_ROLE_LAN);
        }

        sf2_ext_role_not_lan_map = new_not_lan_map;
    }
    return(port_sw_port_role_set(self, role));
}

int ext_sw_imp_port = IMP_PORT_ID;
// =========== sf2 switch ops ===========================
int _port_sf2_sw_init(enetx_port_t *self)
{
    // if ext_sw (53134) has p5 and P8_SGMII_SEL==0, IMP port is p5
    if (self == sf2_sw_ext && unit_port_array[1][5]) {
        uint32_t val32;
        extsw_rreg_wrap(1,1/*PAGE_STATUS*/, 0x70/*REG_STRAP_VAL*/, &val32, sizeof(val32));
        if (!(val32 & (1<<9)/*REG_STRAP_P8_SEL_SGMII*/)) {
            ext_sw_imp_port = 5;
        }
    }
    return port_sf2_sw_init(self);
}

static int port_sf2_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_type = PORT_TYPE_SF2_PORT;
    *port_id = port_info->port;

    if (port_info->is_undef)
        *port_type = PORT_TYPE_SF2_MAC;

    return 0;
}

sw_ops_t port_sf2_sw =
{
    .init = _port_sf2_sw_init,
    .uninit = port_sf2_sw_uninit,
    .mux_port_rx = mux_get_rx_index,    // external switch does not have demux
//  .mux_port_tx = port_sf2_sw_mux,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_sf2_sw_port_id_on_sw,
    .hw_sw_state_set = port_sf2_sw_hw_sw_state_set,
    .hw_sw_state_get = port_sf2_sw_hw_sw_state_get,
    .config_trunk = port_sf2_sw_config_trunk,
    .update_pbvlan = port_sf2_sw_update_pbvlan,
    .rreg = extsw_rreg_wrap,
    .wreg = extsw_wreg_wrap,
    .fast_age = port_sw_fast_age,
};

port_ops_t port_sf2_port =
{
    .init = _port_sf2_port_init,
    .dispatch_pkt = dispatch_pkt_sf2_lan,
    .stats_clear = port_generic_stats_clear,
#ifdef EMBEDDED_BRCMTAG_TX_INSERT
    .tx_pkt_mod = port_sf2_tx_pkt_mod,  /* insert brcm tag for port on external switch */
#endif
    .stats_get = port_generic_stats_get,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .open = port_sf2_generic_open,
    .mtu_set = port_generic_mtu_set,
    .tx_q_remap = port_sf2_tx_q_remap,
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
    .role_set = _port_sw_port_role_set,
    .stp_set = port_sw_port_stp_set,
    .fast_age = port_sw_port_fast_age,
#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
    .switchdev_ops = 
    {
        .switchdev_port_attr_get = sf2_switchdev_port_attr_get,
        .switchdev_port_attr_set = sf2_switchdev_port_attr_set, 
    }
#endif
};

port_ops_t port_sf2_port_mac =
{
    .stats_get = port_generic_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_generic_mtu_set,
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
};

//#define SW53134_USE_VLAN_TAG    1       // configure 53134 to use VLAN tag instead of BRCM tag for testing

static int _rx_pkt_swtag_mod(enetx_port_t *port, struct sk_buff *skb)
{
#if defined(SW53134_USE_VLAN_TAG)
    if (!port->port_info.swtag_use_pvid) return 0;
#endif
    // strip vlan tag from external switch port
    memmove(skb->data + VLAN_HLEN, skb->data, 2 * ETH_ALEN);
    __skb_pull(skb, VLAN_HLEN);
    skb->mac_header += VLAN_HLEN;
    skb_reset_mac_len(skb);
    return 0;
}

static int _tx_pkt_swtag_mod(enetx_port_t *port, pNBuff_t pNBuff)
{
    int ret = 0;
#if defined(SW53134_USE_VLAN_TAG)
    if (!port->port_info.swtag_use_pvid) return 0;
#endif
    if (IS_SKBUFF_PTR(pNBuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
        uint32_t mark = nbuff_get_mark(pNBuff);
        uint16_t vlan_id = EXT_SW_VLAN_TAG_PREFIX | port->n.blog_chnl_rx | ((SKBMARK_GET_Q_PRIO(mark)) << VLAN_PRIO_SHIFT);
        int ret;
        ret = __vlan_insert_inner_tag(skb, htons(ETH_P_8021Q), vlan_id, ETH_HLEN);
    }
    return ret;
}

int enetxapi_post_config(void)
{
#if defined(SF2_DUAL)
    // on internal switch - disable hw-switching for parent port of ext switch */
    if (nb_sw_ext)
        port_sw_port_role_set(nb_sw_ext->s.parent_port, PORT_NETDEV_ROLE_NONE);

#if defined(SW53134_USE_VLAN_TAG)
//#define SW53134_HW_SWITCHING 1
#define SF2SW_RREG      extsw_rreg_wrap
#define SF2SW_WREG      extsw_wreg_wrap

    if (sf2_sw_ext && sf2_sw_ext->port_info.swtag_use_pvid)
    {
        struct ethswctl_data e;
        int ndx;
        // on external switch:
        // * disable hw-switching for external switch port
        // * setup pbvlan for ports
        // * setup vlan table for tag/untag
        for (ndx = 0; ndx < 4; ndx++)   // assume 53134.p0-p3
        {
            enetx_port_t *port = unit_port_array[1][ndx];

            if (!port || !port->dev) continue;

            port->port_info.swtag_use_pvid = 1;
            port->p.ops->rx_pkt_swtag_mod = _rx_pkt_swtag_mod;
            port->p.ops->tx_pkt_swtag_mod = _tx_pkt_swtag_mod;
#if !defined(SW53134_HW_SWITCHING)
            port_sw_port_role_set(port, PORT_NETDEV_ROLE_NONE);
#endif
            e.type = TYPE_SET;
            e.unit = 1; e.port = ndx;
            e.vid = EXT_SW_VLAN_TAG_PREFIX + port->n.blog_chnl_rx;
            ioctl_extsw_defvlan(&e);
            
            e.type = TYPE_SET;
            e.unit = 1;
            e.vid = EXT_SW_VLAN_TAG_PREFIX + port->n.blog_chnl_rx;
            
#if defined(SW53134_HW_SWITCHING)
            e.untag_map = 0xf;
            e.fwd_map = 0x100 | e.untag_map;
#else
            e.fwd_map = 0x100 | (1<<ndx);
            e.untag_map = 1<<ndx;
#endif            
            ioctl_extsw_vlan(&e);
        }
        // * enable SVL
        e.type = TYPE_SET;
        e.unit = 1;
        e.sw_ctrl_type = bcmSwitch8021QControl;
        e.val = 1; // SVL
        ioctl_extsw_control(&e);
        // * enable unknown ucast, mcast, ipmc to flood
        {   uint16_t val16 = 0x10f;
            SF2SW_WREG(1, PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &val16, sizeof(val16));
            SF2SW_WREG(1, PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &val16, sizeof(val16));
            SF2SW_WREG(1, PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &val16, sizeof(val16));
        }
        // * disable learning on p8
        {
            uint16_t val16;
            SF2SW_RREG(1, PAGE_CONTROL, REG_DISABLE_LEARNING, &val16, sizeof(val16));
            val16 |= 0x100;
            SF2SW_WREG(1, PAGE_CONTROL, REG_DISABLE_LEARNING, &val16, sizeof(val16));
        }
        // * EN_MAN2WAN = 1
        {   uint16_t val16;
            SF2SW_RREG(1, PAGE_CONTROL, REG_WAN_PORT_MAP, &val16, sizeof(val16));
            val16 |= EN_MAN_TO_WAN;
            SF2SW_WREG(1, PAGE_CONTROL, REG_WAN_PORT_MAP, &val16, sizeof(val16));
        }
        // * disable BrcmTag
        {   uint8_t val8 = 0;
            SF2SW_WREG(1, PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
        }
        
        // on internal switch:
        // * disable hw-switching for parent port
        port_sw_port_role_set(sf2_sw_ext->s.parent_port, PORT_NETDEV_ROLE_NONE);
    }
#endif //SW53134_USE_VLAN_TAG
#endif //SF2_DUAL
    return enetxapi_post_sf2_config();
}

#if defined(SF2_DUAL)
int port_nonbrcm_sw_init(enetx_port_t *self)
{
    return 0;
}

int port_nonbrcm_sw_uninit(enetx_port_t *self)
{
    return 0;
}

static int port_nonbrcm_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_type = PORT_TYPE_NONBRCM_PORT;
    *port_id = port_info->port;

    return 0;
}

int sw_config_trunk_chnl_rx(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add);

sw_ops_t port_nonbrcm_sw =
{
    .init = port_nonbrcm_sw_init,
    .uninit = port_nonbrcm_sw_uninit,
    .mux_port_rx = mux_get_rx_index,    // external switch does not have demux
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_nonbrcm_sw_port_id_on_sw,
    .config_trunk = sw_config_trunk_chnl_rx,
};


int port_nonbrcm_port_init(enetx_port_t *self)
{
#ifdef CONFIG_BLOG
    if (self->has_interface) {
        int blog_chnlX = root_sw->n.blog_chnl++;
        blog_chnl_set(BLOG_ENETPHY, blog_chnlX, blog_chnlX, self);

        if (mux_set_rx_index(self->p.parent_sw, self->n.blog_chnl, self))
            return -1;

        /* also register demux at root for receive processing if port not on root sw */
        if (!PORT_ON_ROOT_SW(self))
            if (mux_set_rx_index(root_sw, self->n.blog_chnl, self))
                return -1;
    }
#endif

    enet_dbg("Initialized %s role %s\n", self->obj_name, (self->n.port_netdev_role==PORT_NETDEV_ROLE_WAN)?"wan":"lan" );

    return 0;
}

void port_nonbrcm_port_open(enetx_port_t *self)
{
    self->port_info.swtag_use_pvid = 1;
    self->p.ops->rx_pkt_swtag_mod = _rx_pkt_swtag_mod;
    self->p.ops->tx_pkt_swtag_mod = _tx_pkt_swtag_mod;
    port_sf2_generic_open(self);
}

int port_nonbrcm_mib_dump(enetx_port_t *self, int all)
{
    /* based on impl5\bcmsw_runner.c bcmeapi_ethsw_dump_mib() */
    mac_stats_t         *mac_stats = &(self->p.mac->stats);
    int                 port = self->port_info.port;

    if (mac_dev_stats_get(self->p.mac, mac_stats))
        return -1;

    printk("\nnonBrcm port Stats : Port# %d  TBD...\n",port);

    return 0;
}

int port_nonbrcm_port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    bcmFun_t *enet_port_role_notify = bcmFun_get(BCM_FUN_ID_ENET_PORT_ROLE_NOTIFY);

    /* registered modules need to be aware of port role changes */
    if (enet_port_role_notify)
    {
        BCM_EnetPortRole_t port_role;

        port_role.sysport = 0;
        port_role.switch_id = 1;
        port_role.port = self->port_info.port;
        port_role.is_wan = (role == PORT_NETDEV_ROLE_WAN);

        enet_port_role_notify(&port_role);
    }

    return 0;
}

port_ops_t port_nonbrcm_port =
{
    .init = port_nonbrcm_port_init,
    .open = port_nonbrcm_port_open,
    .dispatch_pkt = dispatch_pkt_sf2_lan,
    .stats_get = port_generic_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_generic_mtu_set,
    .mib_dump = port_nonbrcm_mib_dump,
    .print_status = port_print_status_verbose,
    .role_set = port_nonbrcm_port_role_set,
};
#endif // SF2_DUAL
