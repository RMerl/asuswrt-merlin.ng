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
#include "syspvsw.h"
#include "crossbar_dev.h"
#include "mux_index.h"
#include "enet.h"


int enetxapi_post_sysp_config(void)
{
  
    return 0;
}

// =========== sysp port ops =============================

int port_sysp_port_init(enetx_port_t *self)
{
    phy_dev_t *phy_dev;

    if (self->has_interface) {
        self->n.blog_phy = BLOG_ENETPHY;
        self->n.blog_chnl = self->n.blog_chnl_rx = root_sw->n.blog_chnl++;
        enet_dbgv("%s port_id=%d blog_chnl=%d\n", self->obj_name, self->p.port_id, self->n.blog_chnl);

        if (mux_set_rx_index(self->p.parent_sw, self->n.blog_chnl, self))
            return -1;
    }

    enet_dbg("Initialized %s role %s\n", self->obj_name, (self->n.port_netdev_role==PORT_NETDEV_ROLE_WAN)?"wan":"lan" );

    phy_dev = self->p.phy;
    if (phy_dev && IsRGMII(phy_dev->meta_id))
    {
        // select crossbar RGMII endpoint
        SYSPORT_MISC->SYSTEMPORT_MISC_CROSSBAR3X2_CONTROL |= 1 << self->p.mac->mac_id;
        enet_dbgv("crossbar: %s - RGMII\n", self->obj_name);
    }
    return 0;
}

extern void link_change_handler(enetx_port_t *port, int linkstatus, int speed, int duplex);
extern int speed_macro_2_mbps(phy_speed_t spd);

void port_sysp_port_open(enetx_port_t *self)
{
    // if connect to external switch, set link up and enable mac
    if (self->p.phy && self->p.phy->phy_drv->phy_type == PHY_TYPE_MAC2MAC)
    {
        self->p.phy->link = 1;
       if (IsPortConnectedToExternalSwitch(self->p.phy->meta_id))
            mac_dev_enable(self->p.mac);
        else
            link_change_handler(self, self->p.phy->link, speed_macro_2_mbps(self->p.phy->speed), self->p.phy->duplex == PHY_DUPLEX_FULL);
    }
    else if (self->p.phy && IsPortConnectedToExternalSwitch(self->p.phy->meta_id))
    {
        mac_dev_enable(self->p.mac);
    }
    else
        port_generic_open(self);
}


int port_sysp_mib_dump(enetx_port_t *self, int all)
{
    /* based on impl5\bcmsw_runner.c bcmeapi_ethsw_dump_mib() */
    mac_stats_t         mac_stats;
    int                 port = self->p.mac->mac_id;
    uint64_t            errcnt = 0;

    mac_dev_stats_get(self->p.mac, &mac_stats);

    printk("\nSysport Stats : Port# %d\n",port);

    /* Display Tx statistics */
     /* Display Tx statistics */
    printk("\n");
    printk("TxUnicastPkts:          %10llu \n", mac_stats.tx_unicast_packet);
    printk("TxMulticastPkts:        %10llu \n", mac_stats.tx_multicast_packet);
    printk("TxBroadcastPkts:        %10llu \n", mac_stats.tx_broadcast_packet);
    printk("TxDropPkts:             %10llu \n", mac_stats.tx_error);

    /* Display remaining tx stats only if requested */
    if (all) {
        printk("TxBytes:                %10llu \n", mac_stats.tx_byte);
        printk("TxFragments:            %10llu \n", mac_stats.tx_fragments_frame);
        printk("TxCol:                  %10llu \n", mac_stats.tx_total_collision);
        printk("TxSingleCol:            %10llu \n", mac_stats.tx_single_collision);
        printk("TxMultipleCol:          %10llu \n", mac_stats.tx_multiple_collision);
        printk("TxDeferredTx:           %10llu \n", mac_stats.tx_deferral_packet);
        printk("TxLateCol:              %10llu \n", mac_stats.tx_late_collision);
        printk("TxExcessiveCol:         %10llu \n", mac_stats.tx_excessive_collision);
        printk("TxPausePkts:            %10llu \n", mac_stats.tx_pause_control_frame);
        printk("TxExcessivePkts:        %10llu \n", mac_stats.tx_excessive_deferral_packet);
        printk("TxJabberFrames:         %10llu \n", mac_stats.tx_jabber_frame);
        printk("TxFcsError:             %10llu \n", mac_stats.tx_fcs_error);
        printk("TxCtrlFrames:           %10llu \n", mac_stats.tx_control_frame);
        printk("TxOverSzFrames:         %10llu \n", mac_stats.tx_oversize_frame);
        printk("TxUnderSzFrames:        %10llu \n", mac_stats.tx_undersize_frame);
        printk("TxUnderrun:             %10llu \n", mac_stats.tx_underrun);
        printk("TxPkts64Octets:         %10llu \n", mac_stats.tx_frame_64);
        printk("TxPkts65to127Octets:    %10llu \n", mac_stats.tx_frame_65_127);
        printk("TxPkts128to255Octets:   %10llu \n", mac_stats.tx_frame_128_255);
        printk("TxPkts256to511Octets:   %10llu \n", mac_stats.tx_frame_256_511);
        printk("TxPkts512to1023Octets:  %10llu \n", mac_stats.tx_frame_512_1023);
        printk("TxPkts1024to1518Octets: %10llu \n", mac_stats.tx_frame_1024_1518);
        printk("TxPkts1519toMTUOctets:  %10llu \n", mac_stats.tx_frame_1519_mtu);
    }
    else {
        errcnt = 0;
        errcnt += mac_stats.tx_total_collision;
        errcnt += mac_stats.tx_single_collision;
        errcnt += mac_stats.tx_multiple_collision;
        errcnt += mac_stats.tx_deferral_packet;
        errcnt += mac_stats.tx_late_collision;
        errcnt += mac_stats.tx_excessive_collision;
        errcnt += mac_stats.tx_excessive_deferral_packet;
        errcnt += mac_stats.tx_jabber_frame;
        errcnt += mac_stats.tx_fcs_error;
        errcnt += mac_stats.tx_undersize_frame;
        errcnt += mac_stats.tx_underrun;
        printk("TxOtherErrors:          %10llu \n", errcnt);
    }

    /* Display Rx statistics */
    printk("\n");
    printk("RxUnicastPkts:          %10llu \n", mac_stats.rx_unicast_packet);
    printk("RxMulticastPkts:        %10llu \n", mac_stats.rx_multicast_packet);
    printk("RxBroadcastPkts:        %10llu \n", mac_stats.rx_broadcast_packet);

    /* Display remaining rx stats only if requested */
    if (all) {
        printk("RxBytes:                %10llu \n", mac_stats.rx_byte);
        printk("RxJabbers:              %10llu \n", mac_stats.rx_jabber);
        printk("RxAlignErrs:            %10llu \n", mac_stats.rx_alignment_error);
        printk("RxFCSErrs:              %10llu \n", mac_stats.rx_fcs_error);
        printk("RxFragments:            %10llu \n", mac_stats.rx_fragments);
        printk("RxOversizePkts:         %10llu \n", mac_stats.rx_oversize_packet);
        printk("RxUndersizePkts:        %10llu \n", mac_stats.rx_undersize_packet);
        printk("RxPausePkts:            %10llu \n", mac_stats.rx_pause_control_frame);
        printk("RxOverflow:             %10llu \n", mac_stats.rx_overflow);
        printk("RxCtrlPkts:             %10llu \n", mac_stats.rx_control_frame);
        printk("RxUnknownOp:            %10llu \n", mac_stats.rx_unknown_opcode);
        printk("RxLenError:             %10llu \n", mac_stats.rx_frame_length_error);
        printk("RxCodeError:            %10llu \n", mac_stats.rx_code_error);
        printk("RxCarrierSenseErr:      %10llu \n", mac_stats.rx_carrier_sense_error);
        printk("RxPkts64Octets:         %10llu \n", mac_stats.rx_frame_64);
        printk("RxPkts65to127Octets:    %10llu \n", mac_stats.rx_frame_65_127);
        printk("RxPkts128to255Octets:   %10llu \n", mac_stats.rx_frame_128_255);
        printk("RxPkts256to511Octets:   %10llu \n", mac_stats.rx_frame_256_511);
        printk("RxPkts512to1023Octets:  %10llu \n", mac_stats.rx_frame_512_1023);
        printk("RxPkts1024to1522Octets: %10llu \n", mac_stats.rx_frame_1024_1518);
        printk("RxPkts1523toMTU:        %10llu \n", mac_stats.rx_frame_1519_mtu);
    }
    else {
        errcnt = 0;
        errcnt += mac_stats.rx_jabber;
        errcnt += mac_stats.rx_alignment_error;
        errcnt += mac_stats.rx_fcs_error;
        errcnt += mac_stats.rx_oversize_packet;
        errcnt += mac_stats.rx_undersize_packet;
        errcnt += mac_stats.rx_overflow;
        errcnt += mac_stats.rx_unknown_opcode;
        errcnt += mac_stats.rx_frame_length_error;
        errcnt += mac_stats.rx_code_error;
        errcnt += mac_stats.rx_carrier_sense_error;
        printk("RxOtherErrors:          %10llu \n", errcnt);
    }
    return 0;
}

// add by Andrew
int port_sysp_mib_dump_us(enetx_port_t *self, void *ethswctl)
{
    /* based on impl5\bcmsw_runner.c bcmeapi_ethsw_dump_mib() */
    struct ethswctl_data *e = (struct ethswctl_data *)ethswctl;
    mac_stats_t         mac_stats;
    //int                 port = self->p.mac->mac_id;

    mac_dev_stats_get(self->p.mac, &mac_stats);

    /* Calculate Tx statistics */
    e->port_stats.txPackets = mac_stats.tx_unicast_packet + 
                              mac_stats.tx_multicast_packet + 
                              mac_stats.tx_broadcast_packet;
    e->port_stats.txDrops = mac_stats.tx_error;
    e->port_stats.txBytes = mac_stats.tx_byte;

    /* Calculate Rx statistics */
    e->port_stats.rxPackets = mac_stats.rx_unicast_packet + 
                              mac_stats.rx_multicast_packet + 
                              mac_stats.rx_broadcast_packet;
    e->port_stats.rxBytes = mac_stats.rx_byte;
    e->port_stats.rxDrops = 0;
    e->port_stats.rxDiscards = 0;

    return 0;
}
// end of add

int port_sysp_port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    bcmFun_t *enet_port_role_notify = bcmFun_get(BCM_FUN_ID_ENET_PORT_ROLE_NOTIFY);

    /* registered modules need to be aware of port role changes */
    if (enet_port_role_notify)
    {
        BCM_EnetPortRole_t port_role;

        port_role.sysport = self->p.mac->mac_id;
        port_role.port = 0;
        port_role.is_wan = (role == PORT_NETDEV_ROLE_WAN);

        enet_port_role_notify(&port_role);
    }

    return 0;
}

int port_sysp_mtu_set(enetx_port_t *self, int mtu)
{
    // translate max payload to max frame in hw 
    mtu += ENET_MAX_MTU_EXTRA_SIZE;
    return port_generic_mtu_set(self, mtu);
}

// =========== sysp switch ops ===========================

int port_sysp_sw_init(enetx_port_t *self)
{
    return 0;
}

int port_sysp_sw_uninit(enetx_port_t *self)
{
    return 0;
}


port_ops_t port_sysp_port_mac =
{
    .stats_get = port_generic_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_generic_mtu_set,
    .mib_dump = port_sysp_mib_dump,
    .mib_dump_us = port_sysp_mib_dump_us, // add by Andrew
};
