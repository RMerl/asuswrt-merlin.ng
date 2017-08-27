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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#include "sf2.h"

static int port_sf2_port_init(enetx_port_t *self)
{
    return 0;
}

static int port_sf2_port_uninit(enetx_port_t *self)
{
    return 0;
}

static int port_sf2_sw_init(enetx_port_t *self)
{
    /* TODO: Initialize SF2
    ...
    pmc_switch_power_up();

    // power on and reset the quad and single phy
    phy_ctrl = ETHSW_REG->qphy_ctrl;
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_QPHY_CTRL_PHYAD_BASE_MASK);
    phy_ctrl |= ETHSW_QPHY_CTRL_RESET_MASK|(phy_base<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT);
    ....
    */

    return 0;
}

static int port_sf2_sw_uninit(enetx_port_t *self)
{
}

static int port_sf2_sw_demux(enetx_port_t *sw, int rx_port, FkBuff_t *fkb, enetx_port_t **out_port)
{
    return 0;
}

static int port_sf2_sw_mux(enetx_port_t *tx_port, FkBuff_t *fkb, enetx_port_t **out_port)
{
    return 0;
}

struct sw_ops port_sf2_sw =
{
    .init = &port_sf2_sw_init,
    .uninit = &port_sf2_sw_uninit,
    .port_demux = &port_sf2_sw_demux,
    .port_mux = &port_sf2_sw_mux,
};

struct port_ops port_sf2_port =
{
    .init = &port_sf2_port_init,
    .uninit = &port_sf2_port_uninit,
};

