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
 * lport_drv.c
 *
 *  Created on: 6 בספט׳ 2015
 *      Author: yonatani
 */

//includes
#include "bcm6858_drivers_lport_ag.h"
#include "bcm6858_lport_ctrl_ag.h"
#include "bcm6858_lport_xlmac_ag.h"
#include "bcm6858_lport_mab_ag.h"
#include "bcm6858_lport_rgmii_ag.h"
#include "bcm6858_xlmac_conf_ag.h"
#include "bcm6858_mib_conf_ag.h"
#include "bcm6858_lport_led_ag.h"
#include "bcm6858_lport_intr_ag.h"
#include "bcm6858_lport_srds_ag.h"
#include "lport_drv.h"
#include "lport_mdio.h"
#include "serdes_access.h"
#if defined(_CFE_)
typedef  uint32_t uint32;
extern void cfe_usleep(int usec);
#define UDELAY(_a) cfe_usleep(_a)
#else
#include <asm/delay.h>
#include "lport_intr.h"
#include "bcm_map_part.h"
void remap_ru_block_addrs(uint32_t block_index, const ru_block_rec *ru_blocks[]);
#define UDELAY udelay
#endif
#include "mii_shared.h"

/*
 *LPORT MUX OPTIONS:
 *
 *LPORT Block has two instances of XLMAC BLOCKS which has Muxes towards the user ports, these muxes can select the XLMAC function
 *
 * XLMAC0 Options:
 * 1. Port 0 as 10G Merlin (ports 1,2,3 are disabled).
 * 2. All 4 Ports to Merlins in rates 2.5G/1G/100M/10M
 * 3. All 4 ports to QGphy
 * 4. Combination of Merlin & QGphy
 *
 * XLMAC1 Options:
 * 1. Port 4 as 10G Merlin (ports 5,6,7 are disabled).
 * 2. All 3 Ports to RGMII
 * 3. All 4 ports to Merlin in rates 2.5G/1G/100M/10M
 * 4. Combination of Merlin & RGMII
 *
 */

/*This mux define is used to configure register LPORT_REG_CNTRL*/
typedef enum
{
    LPORT_MUX_REG_SERDES = 1,
    LPORT_MUX_REG_RGMII = 0,
    LPORT_MUX_REG_EGPHY = 0
}E_LPORT_MUX_REG;

typedef enum
{
    LPORT_XLMAC_GMII,
    LPORT_XLMAC_XGMII
}E_LPORT_XLMAC_MODE;

static lport_init_s local_lport_cfg = {};
static lport_rgmii_cfg_s local_lport_rgmii_cfg[LPORT_NUM_OF_RGMII] = {{}};

int validate_lport_configuration(LPORT_PORT_MUX_SELECT prt_mux_sel[])
{
    int iter;

    //validate only port 0 & 4 are assigned as possible 10G
    for (iter=0;iter < LPORT_NUM_OF_PORTS;iter++)
    {
        //valivate RGMII only from XLMAC1 only
        if (iter < 4 && prt_mux_sel[iter]== PORT_RGMII)
        {
            pr_err("Port %d can't work in RGMII mode\n", iter);
            return LPORT_ERR_PARAM;
        }
        if (iter > 3 && prt_mux_sel[iter]== PORT_GPHY)
        {
            pr_err("Port %d can't work in GPHY mode\n", iter);
            return LPORT_ERR_PARAM;
        }
    }

    //validate XLMAC0 10G Merlin
    if (prt_mux_sel[0] == PORT_XFI)
    {
        if(prt_mux_sel[1] | prt_mux_sel[2] |prt_mux_sel[3])
        {
            pr_err("Wrong XLMAC0 Configuration,Port0 is 10G Serdes and other ports are configured!\n");
            return LPORT_ERR_PARAM;
        }
    }
    //validate XLMAC1 10Merlin
    if (prt_mux_sel[4] == PORT_XFI)
    {
        if(prt_mux_sel[5] | prt_mux_sel[6] |prt_mux_sel[7])
        {
            pr_err("Wrong XLMAC1 Configuration,Port4 is 10G Serdes and other ports are configured!\n");
            return LPORT_ERR_PARAM;
        }
    }

    //validate no mutual 10G in XLMAC0,XLMAC1
    if(prt_mux_sel[0]==PORT_XFI && prt_mux_sel[4]==PORT_XFI)
    {
        pr_err("Only one 10G port(0,4) is allowed\n");
        return LPORT_ERR_PARAM;
    }

    //validate no collision on serdes mux
    if(prt_mux_sel[1] > PORT_UNAVAIL && prt_mux_sel[1] < PORT_XFI &&
        prt_mux_sel[5] > PORT_UNAVAIL && prt_mux_sel[5] < PORT_XFI)
    {
        pr_err("ERROR:Only one  port(1,5) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }

    if(prt_mux_sel[2] > PORT_UNAVAIL && prt_mux_sel[2] < PORT_XFI &&
        prt_mux_sel[6] > PORT_UNAVAIL && prt_mux_sel[6] < PORT_XFI)
    {
        pr_err("ERROR:Only one  port(2,6) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }

    if(prt_mux_sel[3] > PORT_UNAVAIL && prt_mux_sel[3] < PORT_XFI &&
        prt_mux_sel[7] > PORT_UNAVAIL && prt_mux_sel[7] < PORT_XFI)
    {
        pr_err("ERROR:Only one  port(2,6) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }
    return LPORT_ERR_OK;
}

static int lport_speed_to_xlmac(LPORT_PORT_RATE lport_speed)
{
    int ret = 0;

    switch(lport_speed)
    {
    case LPORT_RATE_10MB:
        ret = 0;
        break;
    case LPORT_RATE_100MB:
        ret = 1;
        break;
    case LPORT_RATE_1000MB:
        ret = 2;
        break;
    case LPORT_RATE_2500MB:
        ret = 3;
        break;
    case LPORT_RATE_10G:
        ret = 4;
        break;
    case LPORT_RATE_UNKNOWN:
    default:
        pr_err("Wrong lport speed %d\n", lport_speed);
    }

    return ret;
}

int xlmac_init(lport_init_s *init_params)
{
    lport_xlmac_ctrl     ctrl;
    lport_xlmac_tx_ctrl  tx_ctrl;
    lport_xlmac_rx_ctrl  rx_ctrl;
    uint32_t i;
    int rc = LPORT_ERR_OK;

    for (i = 0; i < LPORT_NUM_OF_PORTS; i++)
    {
        if (init_params->prt_mux_sel[i] != PORT_UNAVAIL)
        {
            rc = ag_drv_lport_xlmac_ctrl_get(i, &ctrl);
            rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_get(i, &tx_ctrl);

            /*TODO: Change crc_mode to PER_PACKET when HW confirm usage
             * when doing REPLACE packet must be padded by 4 bytes*/
            tx_ctrl.crc_mode = XLMAC_CRC_PERPKT;
            tx_ctrl.pad_en = 1;
            rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_set(i, &tx_ctrl);

            rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_get(i, &rx_ctrl);
            rx_ctrl.strip_crc = 0;
            rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_set(i, &rx_ctrl);
            switch(init_params->prt_mux_sel[i])
            {
            case PORT_XFI:
                rc = rc ? rc : ag_drv_lport_xlmac_mode_set(i, lport_speed_to_xlmac(LPORT_RATE_10G),
                    0, 0);
                break;
            case PORT_HSGMII:
                rc = rc ? rc : ag_drv_lport_xlmac_mode_set(i, lport_speed_to_xlmac(LPORT_RATE_2500MB),
                    0, 0);
                break;
            default:
                rc = rc ? rc : ag_drv_lport_xlmac_mode_set(i, lport_speed_to_xlmac(LPORT_RATE_1000MB),
                    0, 0);
            }

            rc = rc ? rc : ag_drv_lport_xlmac_tx_crc_corrupt_ctrl_set(i ,0 ,0 ,0, 0);

            ctrl.soft_reset = 0;
            rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(i, &ctrl);
        }
    }

    return rc;
}

int msbus_init(lport_init_s *init_params)
{
    lport_mab_cntrl cntrl_xlmac[2];
    lport_mab_tx_wrr_ctrl wrr;
    uint32_t i, j;
    int rc = LPORT_ERR_OK;

    rc = ag_drv_lport_mab_cntrl_get(0, &cntrl_xlmac[0]);
    rc = rc ? rc : ag_drv_lport_mab_cntrl_get(1, &cntrl_xlmac[1]);

    if (rc)
        return rc;

    for (i = 0; i < LPORT_NUM_OF_XLMACS; i++) 
    {
        for (j = 0; j < LPORT_NUM_OF_PORTS_PER_XLMAC; j++)
        {
            uint32_t idx = i * LPORT_NUM_OF_PORTS_PER_XLMAC + j;

            if (init_params->prt_mux_sel[idx] != PORT_UNAVAIL)
            {
                if (init_params->prt_mux_sel[idx] == PORT_XFI)
                {
                    cntrl_xlmac[i].xgmii_rx_rst = 0;
                    cntrl_xlmac[i].xgmii_tx_rst = 0;
                    ag_drv_lport_mab_tx_wrr_ctrl_get(i, &wrr);
                    wrr.p4_weight = 4;
                    wrr.p5_weight = 0;
                    wrr.p6_weight = 0;
                    wrr.p7_weight = 0;
                    ag_drv_lport_mab_tx_wrr_ctrl_set(i, &wrr);
                }
                else
                {
                    cntrl_xlmac[i].gmii_rx_rst &= ~(1 << j);
                    cntrl_xlmac[i].gmii_tx_rst &= ~(1 << j);
                }
            }
        }
    }

    rc = ag_drv_lport_mab_cntrl_set(0 , &cntrl_xlmac[0]);
    rc = rc ? rc : ag_drv_lport_mab_cntrl_set(1 , &cntrl_xlmac[1]);

    return rc;
}

int lport_mux_init(lport_init_s *init_params)
{
    lport_ctrl_control control = {};

    //go over all 8 ports and configure the output

    if(init_params->prt_mux_sel[0] == PORT_XFI)
        control.p0_mode = LPORT_XLMAC_XGMII;
    else
        control.p0_mode = LPORT_XLMAC_GMII;

    if(init_params->prt_mux_sel[4] == PORT_XFI)
        control.p4_mode = LPORT_XLMAC_XGMII;
    else
        control.p4_mode = LPORT_XLMAC_GMII;

    if(init_params->prt_mux_sel[0] == PORT_GPHY)
        control.gport_sel_0 = LPORT_MUX_REG_EGPHY;
    else
        control.gport_sel_0 = LPORT_MUX_REG_SERDES;

    if(init_params->prt_mux_sel[1] == PORT_SGMII_SLAVE ||
        init_params->prt_mux_sel[1] == PORT_SGMII_1000BASE_X ||
        init_params->prt_mux_sel[1] == PORT_HSGMII)
    {
        control.gport_sel_1 = LPORT_MUX_REG_SERDES;
    }
    else
        control.gport_sel_1 = LPORT_MUX_REG_EGPHY;

    if(init_params->prt_mux_sel[2] == PORT_SGMII_SLAVE ||
        init_params->prt_mux_sel[2] == PORT_SGMII_1000BASE_X ||
        init_params->prt_mux_sel[2] == PORT_HSGMII)
    {
        control.gport_sel_2 = LPORT_MUX_REG_SERDES;
    }
    else
        control.gport_sel_2 = LPORT_MUX_REG_EGPHY;

    if (init_params->prt_mux_sel[3] == PORT_SGMII_SLAVE ||
        init_params->prt_mux_sel[3] == PORT_SGMII_1000BASE_X ||
        init_params->prt_mux_sel[3] == PORT_HSGMII)
    {
        control.gport_sel_3 = LPORT_MUX_REG_SERDES;
    }
    else
        control.gport_sel_3 = LPORT_MUX_REG_EGPHY;

    if (init_params->prt_mux_sel[4] != PORT_RGMII)
        control.gport_sel_4 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_4 = LPORT_MUX_REG_RGMII;

    if (init_params->prt_mux_sel[5] != PORT_RGMII)
        control.gport_sel_5 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_5 = LPORT_MUX_REG_RGMII;

    if (init_params->prt_mux_sel[6] != PORT_RGMII)
        control.gport_sel_6 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_6 = LPORT_MUX_REG_RGMII;

    if (init_params->prt_mux_sel[7] != PORT_RGMII)
        control.gport_sel_7 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_7 = LPORT_MUX_REG_RGMII;
    return ag_drv_lport_ctrl_control_set(&control);
}

int rgmii_init(lport_init_s *init_params)
{
    uint32_t i;
    lport_rgmii_cntrl cntrl;
    int rc = LPORT_ERR_OK;

    for (i = LPORT_FIRST_RGMII_PORT; i < LPORT_NUM_OF_PORTS; i++)
    {
        if(init_params->prt_mux_sel[i] == PORT_RGMII)
        {
            rc = ag_drv_lport_rgmii_cntrl_get(i-4, &cntrl);
            cntrl.rgmii_mode_en = 1;
            rc = rc ? rc : ag_drv_lport_rgmii_cntrl_set(i-4, &cntrl);
        }
    }
    return rc;
}

int lport_get_rgmii_cfg(uint32_t portid, lport_rgmii_cfg_s *rgmii_cfg)
{
    lport_rgmii_cntrl cntrl;
    int rc;

    if(portid < LPORT_FIRST_RGMII_PORT)
    {
        pr_err("LPORT RGMII port %d is not RGMII.Should be 4,5,6 only.\n", portid);
        return LPORT_ERR_PARAM;
    }
    memcpy(rgmii_cfg, &local_lport_rgmii_cfg[portid-4], sizeof(lport_rgmii_cfg_s));

    if ((rc = ag_drv_lport_rgmii_cntrl_get(portid-4, &cntrl)))
        return rc;

    rgmii_cfg->eee_enable = cntrl.tx_clk_stop_en;
    rgmii_cfg->rvmii_ref = cntrl.rvmii_ref_sel;
    rgmii_cfg->idelay_dis = cntrl.id_mode_dis;
    rgmii_cfg->portmode = cntrl.port_mode;

    return 0;
}

int lport_set_rgmii_cfg(uint32_t portid, lport_rgmii_cfg_s *rgmii_cfg)
{
    lport_rgmii_cntrl cntrl;
    int rc;

    if(portid < LPORT_FIRST_RGMII_PORT)
    {
        pr_err("LPORT RGMII port %d is not RGMII.Should be 4,5,6 only.\n", portid);
        return LPORT_ERR_PARAM;
    }
    rc = ag_drv_lport_rgmii_cntrl_get(portid-4, &cntrl);
    cntrl.tx_clk_stop_en = rgmii_cfg->eee_enable;
    cntrl.rvmii_ref_sel = rgmii_cfg->rvmii_ref;
    cntrl.id_mode_dis = rgmii_cfg->idelay_dis;
    cntrl.port_mode = rgmii_cfg->portmode;
    cntrl.rgmii_mode_en = rgmii_cfg->valid;
    rc = rc ? rc : ag_drv_lport_rgmii_cntrl_set(portid-4, &cntrl);

    memcpy(&local_lport_rgmii_cfg[portid-4],rgmii_cfg, sizeof(lport_rgmii_cfg_s));
    return rc;
}

static int qgphy_init(lport_init_s *init_params)
{
    uint32_t i;
    uint8_t ports_enabled = 0;
    lport_ctrl_qegphy_cntrl qegphy_cntrl ;
    lport_ctrl_qegphy_status qegphy_status;

    for (i = 0; i < LPORT_LAST_EGPHY_PORT; i++)
    {
        if (init_params->prt_mux_sel[i] == PORT_GPHY)
        {
            ports_enabled |= (1 << i);
        }
    }

    if(!ports_enabled)
    {
        pr_err("LPORT EGPHY not muxed, skipping init\n");
        return LPORT_ERR_OK;
    }

    qegphy_cntrl.phy_phyad = 1;
    qegphy_cntrl.phy_reset = 1;
    qegphy_cntrl.ck25_en = 1;
    qegphy_cntrl.iddq_global_pwr = 0;
    qegphy_cntrl.force_dll_en = 0;
    qegphy_cntrl.ext_pwr_down = 0xf;
    qegphy_cntrl.iddq_bias = 0;

    ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
    UDELAY(900);
    qegphy_cntrl.phy_reset = 0;
    qegphy_cntrl.iddq_global_pwr = 0;
    qegphy_cntrl.force_dll_en = 1;
    qegphy_cntrl.ext_pwr_down = 0;//~ports_enabled;

    ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
    UDELAY(900);

    /* Check for PLL Lock */
    ag_drv_lport_ctrl_qegphy_status_get(&qegphy_status);

    pr_info("LPORT QEGPHY PLL is %s\n",
        qegphy_status.pll_lock ? "Locked!":"Not Locked!");

    return LPORT_ERR_OK;
}

/* default leds configuration:
 * LED0:On for each speed 
 * LED1:On when link up and blinks on activity
 */

int lport_leds_init(lport_init_s *init_params)
{
    lport_led_link_and_speed_encoding_sel spdlnk_sel;
    lport_led_link_and_speed_encoding spdlnk;
    lport_ctrl_led_serial_cntrl led_serial_cntrl;
    lport_led_cntrl led_ctrl;
    int port;

    ag_drv_lport_ctrl_led_serial_cntrl_get(&led_serial_cntrl);

    for (port = 0; port < LPORT_NUM_OF_PORTS; port++)
    {
        if (init_params->prt_mux_sel[port] != PORT_UNAVAIL)
        {
            led_serial_cntrl.port_en |= (3 << (port*2));
            ag_drv_lport_led_link_and_speed_encoding_sel_get(port, &spdlnk_sel);
            spdlnk_sel.sel_1000m_encode = 2;
            spdlnk_sel.sel_100m_encode = 2;
            spdlnk_sel.sel_10g_encode = 2;
            spdlnk_sel.sel_10m_encode = 2;
            spdlnk_sel.sel_2500m_encode = 2;
            ag_drv_lport_led_link_and_speed_encoding_sel_set(port, &spdlnk_sel);

            ag_drv_lport_led_link_and_speed_encoding_get(port, &spdlnk);
            spdlnk.m10g_encode = 0;
            spdlnk.m2500_encode = 0;
            spdlnk.m1000_encode = 0;
            spdlnk.m100_encode = 0;
            spdlnk.m10_encode = 0;
            spdlnk.no_link_encode = 7;
            ag_drv_lport_led_link_and_speed_encoding_set(port, &spdlnk);

            ag_drv_lport_led_cntrl_get(port, &led_ctrl);
            led_ctrl.spdlnk_led0_act_sel = 0;
            led_ctrl.spdlnk_led1_act_sel = 1;
            ag_drv_lport_led_cntrl_set(port, &led_ctrl);
        }
    }
    ag_drv_lport_ctrl_led_serial_cntrl_set(&led_serial_cntrl);

    return 0;
}

int lport_init_driver(lport_init_s *init_params)
{
#ifndef _CFE_
    /* ioRemap virtual addresses of LPORT */
    remap_ru_block_addrs(LPORT_IDX, RU_LPORT_BLOCKS);
#endif

    if(validate_lport_configuration(init_params->prt_mux_sel))
    {
        pr_err("LPORT configuration validation failed\n");
        return LPORT_ERR_PARAM;
    }

    //init lport MUX
    if(lport_mux_init(init_params))
    {
        pr_err("init LPORT MUX failed\n");
        return LPORT_ERR_PARAM;
    }

    //init xlmac
    if(xlmac_init(init_params))
    {
        pr_err("init LPORT XLMAC failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("XLMAC!|");
    //init xlmac
    if(msbus_init(init_params))
    {
        pr_err("init LPORT MSBUS failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("MSBUS!|");
    if (qgphy_init(init_params))
    {
        pr_err("init LPORT EGPHY failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("QGPHY!|");
#ifndef _CFE_
    if (lport_intr_init())
    {
        pr_err("init LPORT interrupts failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("INTR!|");
#endif
#ifndef CONFIG_BRCM_IKOS
    if (lport_serdes_init(init_params))
    {
        pr_err("failed to initialize Merlin Serdes\n");
#ifndef _CFE_
        return LPORT_ERR_PARAM;
#endif
    }
    pr_err("SERDES!|");
#endif
    if (mdio_bus_init())
    {
        pr_err("init LPORT interrupts failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("MDIO!|");

    if (lport_leds_init(init_params))
    {
        pr_err("init LPORT leds failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("LED!\n");
    /*save shadow copy of init configuration for runtime use*/
    memcpy(&local_lport_cfg, init_params, sizeof(local_lport_cfg));

    return LPORT_ERR_OK;
}

int lport_get_port_configuration(uint32_t portid, lport_port_cfg_s *port_conf)
{
    int rc;
    lport_xlmac_ctrl xlmac_ctrl;
    lport_xlmac_tx_ctrl xlmac_tx_ctrl;
    lport_xlmac_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_get(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    port_conf->average_igp = xlmac_tx_ctrl.average_ipg;
    port_conf->pad_en = xlmac_tx_ctrl.pad_en;
    port_conf->local_loopback = xlmac_ctrl.local_lpbk;
    port_conf->pad_threashold = xlmac_tx_ctrl.pad_threshold;
    port_conf->tx_preamble_len = xlmac_tx_ctrl.tx_preamble_length;
    return rc;
}

int lport_set_port_configuration(uint32_t portid, lport_port_cfg_s *port_conf)
{
    int rc;
    lport_xlmac_ctrl xlmac_ctrl;
    lport_xlmac_tx_ctrl xlmac_tx_ctrl;
    lport_xlmac_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_get(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_get(portid, &xlmac_rx_ctrl);

    xlmac_tx_ctrl.average_ipg = port_conf->average_igp;
    xlmac_tx_ctrl.pad_en = port_conf->pad_en;
    xlmac_ctrl.local_lpbk = port_conf->local_loopback;
    rc = rc ? rc : ag_drv_lport_xlmac_mode_set(portid, lport_speed_to_xlmac(port_conf->speed), 0, 0);
    xlmac_tx_ctrl.pad_threshold = port_conf->pad_threashold;
    xlmac_tx_ctrl.tx_preamble_length = port_conf->tx_preamble_len;

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_set(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_set(portid, &xlmac_rx_ctrl);

    if ((local_lport_cfg.prt_mux_sel[portid] == PORT_SGMII_SLAVE) ||
        (local_lport_cfg.prt_mux_sel[portid] == PORT_SGMII_1000BASE_X) ||
        (local_lport_cfg.prt_mux_sel[portid] == PORT_XFI)   ||
        (local_lport_cfg.prt_mux_sel[portid] == PORT_HSGMII))
    {
        rc = rc ? rc : lport_serdes_change_speed(portid, port_conf->speed);
    }

    return rc;
}

int lport_get_pause_configuration(uint32_t portid, lport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    lport_xlmac_rx_ctrl xlmac_rx_ctrl;
    lport_xlmac_pause_ctrl pause_ctrl;

    rc = ag_drv_lport_xlmac_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_mac_sa_get(portid, &flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_mac_sa_get(portid, &flow_ctrl->rx_ctrl_sa);
    rc = rc ? rc : ag_drv_lport_xlmac_pause_ctrl_get(portid, &pause_ctrl);

    flow_ctrl->pause_refresh_en = pause_ctrl.pause_refresh_en;
    flow_ctrl->pause_refresh_timer = pause_ctrl.pause_refresh_timer;
    flow_ctrl->pause_xoff_timer = pause_ctrl.pause_xoff_timer;
    flow_ctrl->rx_pass_ctrl = xlmac_rx_ctrl.rx_pass_ctrl;
    flow_ctrl->rx_pass_pause = xlmac_rx_ctrl.rx_pass_pause;
    flow_ctrl->rx_pause_en = pause_ctrl.rx_pause_en;
    flow_ctrl->tx_pause_en = pause_ctrl.tx_pause_en;

    return rc;
}

int lport_set_pause_configuration(uint32_t portid, lport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    lport_xlmac_rx_ctrl xlmac_rx_ctrl;
    lport_xlmac_pause_ctrl pause_ctrl;

    rc = ag_drv_lport_xlmac_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_mac_sa_set(portid, flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_mac_sa_set(portid, flow_ctrl->rx_ctrl_sa);
    rc = rc ? rc : ag_drv_lport_xlmac_pause_ctrl_get(portid, &pause_ctrl);

    pause_ctrl.pause_refresh_en = flow_ctrl->pause_refresh_en;
    pause_ctrl.pause_refresh_timer = flow_ctrl->pause_refresh_timer;
    pause_ctrl.pause_xoff_timer = flow_ctrl->pause_xoff_timer;
    xlmac_rx_ctrl.rx_pass_ctrl = flow_ctrl->rx_pass_ctrl;
    xlmac_rx_ctrl.rx_pass_pause = flow_ctrl->rx_pass_pause;
    pause_ctrl.rx_pause_en = flow_ctrl->rx_pause_en;
    pause_ctrl.tx_pause_en = flow_ctrl->tx_pause_en;

    rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_set(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_pause_ctrl_set(portid, &pause_ctrl);

    return rc;
}

int lport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en)
{
    int rc;
    lport_xlmac_ctrl xlmac_ctrl;

    rc = ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);

    *rx_en = xlmac_ctrl.rx_en;
    *tx_en = xlmac_ctrl.tx_en;

    return rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);
}

int lport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en)
{
    int rc;
    lport_xlmac_ctrl xlmac_ctrl;

    rc = ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);

    xlmac_ctrl.rx_en = rx_en;
    xlmac_ctrl.tx_en = tx_en;

    return rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);
}

int  lport_get_phyid(uint32_t port, uint16_t *phyid)
{
    lport_ctrl_qegphy_cntrl qegphy_cntrl;
    int rc = LPORT_ERR_OK;

    if(local_lport_cfg.prt_mux_sel[port] == PORT_UNAVAIL)
    {
        pr_err("%s(%d):LPORT failed to reset phy,Port %d is not configured\n"
            ,__FUNCTION__, __LINE__, port);
        return LPORT_ERR_STATE;
    }

    if(local_lport_cfg.prt_mux_sel[port] == PORT_GPHY)
    {
        rc = ag_drv_lport_ctrl_qegphy_cntrl_get(&qegphy_cntrl);
        *phyid = qegphy_cntrl.phy_phyad + port;
    }
    else if (local_lport_cfg.prt_mux_sel[port] == PORT_RGMII)
    {
        if(local_lport_rgmii_cfg[port - LPORT_FIRST_RGMII_PORT].phy_attached)
        {
            *phyid = local_lport_rgmii_cfg[port - LPORT_FIRST_RGMII_PORT].phyid;
        }
        else
        {
            pr_err("%s(%d):LPORT No phy attached to RGMII port %d\n"
                ,__FUNCTION__, __LINE__, port);
            return LPORT_ERR_STATE;
        }
    }
    return rc;
}

int lport_reset_phy_cfg(uint32_t port, lport_port_phycfg_s *phycfg)
{
    int rc = LPORT_ERR_OK;
    uint16_t phyid;
    uint16_t reg_val;

    rc = lport_get_phyid(port, &phyid);
    if(rc)
        return rc;

    /*handle Phy Down */
    if(!phycfg->port_up)
    {
        rc = lport_mdio22_wr(phyid, MII_BMCR, BMCR_POWERDOWN);
        return rc;
    }

    /*now reset and config phy*/
    rc = lport_mdio22_wr(phyid, MII_BMCR, BMCR_RESET);

    UDELAY(200);
    if(phycfg->autoneg_en)
    {
        reg_val = PSB_802_3|ANAR_10HD|ANAR_10FD|ANAR_TXHD|ANAR_TXFD;
        reg_val |= phycfg->rx_pause_en ? ANAR_PAUSE|ANAR_ASYPAUSE:0;
        rc  = rc ? rc : lport_mdio22_wr(phyid, MII_ANAR, reg_val);

        if (phycfg->rate_adv_map & LPORT_RATE_1000MB)
        {
            reg_val = K1TCR_1000BT_FDX | K1TCR_RPTR | K1TCR_1000BT_HDX;
            rc = rc ? rc : lport_mdio22_wr(phyid, MII_K1CTL, reg_val);
        }
    }

    UDELAY(200);
    /*finalize by writing again BMCR*/
    reg_val = (phycfg->rate_adv_map & LPORT_RATE_1000MB) ? BMCR_SPEED1000:BMCR_SPEED100;
    reg_val |= phycfg->duplex ? BMCR_DUPLEX : 0;
    reg_val |= phycfg->autoneg_en ? BMCR_ANENABLE|BMCR_RESTARTAN: 0;

    rc = rc ? rc : lport_mdio22_wr(phyid, MII_BMCR, reg_val);

    return rc;
}

/*This status analysis is for Broadcom's PHYs only as it described in the EGPHY register specification */
int lport_get_brcm_phy_status(uint16_t phyid,lport_port_status_s *port_status)
{
    int rc;
    uint16_t    regval = 0;
    uint8_t     autoneg_hcd;

    rc = lport_mdio22_rd(phyid, MII_AUXSTA, &regval);
    if (rc)
        return rc;

    /*check if link is up*/
    port_status->port_up = (regval & 0x4) ? 1: 0;

    if (!port_status->port_up)
    {
        port_status->rate = LPORT_RATE_UNKNOWN;
        port_status->duplex = 0;
        return LPORT_ERR_OK;
    }

    port_status->autoneg_en = ( regval & 0x8000)?  1 : 0;

    autoneg_hcd = (regval & 0x0700) >> 8;
    switch(autoneg_hcd)
    {
    case 0x1:
        port_status->duplex = LPORT_HALF_DUPLEX;
        port_status->rate = LPORT_RATE_10MB;
        break;
    case 0x2:
        port_status->duplex = LPORT_FULL_DUPLEX;
        port_status->rate = LPORT_RATE_10MB;
        break;
    case 0x3:
        port_status->duplex = LPORT_HALF_DUPLEX;
        port_status->rate = LPORT_RATE_100MB;
        break;
    case 0x5:
        port_status->duplex = LPORT_FULL_DUPLEX;
        port_status->rate = LPORT_RATE_100MB;
        break;
    case 0x6:
        port_status->duplex = LPORT_HALF_DUPLEX;
        port_status->rate = LPORT_RATE_1000MB;
        break;
    case 0x7:
        port_status->duplex = LPORT_FULL_DUPLEX;
        port_status->rate = LPORT_RATE_1000MB;
        break;
    case 0x4:
    default:
        pr_err("%s(%d):LPORT phy autoneg bad status %d\n"
            ,__FUNCTION__, __LINE__, autoneg_hcd);
        port_status->port_up = 0;
        return LPORT_ERR_STATE;
    }
    port_status->rx_pause_en = (regval & 0x1) ? 1 : 0;
    port_status->tx_pause_en = (regval & 0x2) ? 1 : 0;

    return rc;
}

int lport_set_rgmii_ib_status(uint32_t rgmii_id, lport_port_status_s *port_status)
{
    int rc;
    uint8_t ib_status_ovrd;
    uint8_t link_decode;
    uint8_t duplex_decode;
    uint8_t speed_decode;

    ib_status_ovrd = 1;
    link_decode = port_status->port_up;
    duplex_decode = port_status->duplex == LPORT_FULL_DUPLEX ? 1 : 0;
    speed_decode = port_status->rate == LPORT_RATE_10MB ? 0 : port_status->rate == LPORT_RATE_100MB ? 1 :
        port_status->rate == LPORT_RATE_1000MB ? 2 : 3;

    rc = ag_drv_lport_rgmii_ib_status_set(rgmii_id, ib_status_ovrd, link_decode,
        duplex_decode, speed_decode);
    return rc;
}

int lport_get_rgmii_ib_status(uint32_t rgmii_id, lport_port_status_s *port_status)
{
    int rc;
    uint8_t ib_status_ovrd;
    uint8_t link_decode;
    uint8_t duplex_decode;
    uint8_t speed_decode;
    lport_rgmii_cntrl cntrl;

    rc = ag_drv_lport_rgmii_ib_status_get(rgmii_id, &ib_status_ovrd, &link_decode,
        &duplex_decode, &speed_decode);
    if(link_decode)
    {
        port_status->port_up = 1;
        port_status->duplex = duplex_decode ? LPORT_FULL_DUPLEX:LPORT_HALF_DUPLEX;
        switch(speed_decode)
        {
        case 0:
            port_status->rate = LPORT_RATE_10MB;
            break;
        case 1:
            port_status->rate = LPORT_RATE_100MB;
            break;
        case 2:
            port_status->rate = LPORT_RATE_1000MB;
            break;
        default:
            port_status->rate = LPORT_RATE_UNKNOWN;
            return LPORT_ERR_STATE;
        }

        rc = ag_drv_lport_rgmii_cntrl_get(rgmii_id, &cntrl);
        port_status->rx_pause_en = cntrl.rx_pause_en;
        port_status->tx_pause_en = cntrl.tx_pause_en;
    }
    else
    {
        port_status->port_up = 0;
        port_status->rate = LPORT_RATE_UNKNOWN;
    }

    return rc;
}

int lport_get_port_status(uint32_t port, lport_port_status_s *port_status)
{
    int rc = LPORT_ERR_OK;
    lport_ctrl_qegphy_status qegphy_status;
    uint32_t    portstatus;
    uint16_t    phyid;

    switch (local_lport_cfg.prt_mux_sel[port])
    {
    case PORT_UNAVAIL:
        port_status->port_up = 0;
        port_status->rate = LPORT_RATE_UNKNOWN;
        port_status->duplex = 0;
        break;

    case PORT_GPHY:
        /*first try to check for link status through the direct status register*/
        rc = ag_drv_lport_ctrl_qegphy_status_get(&qegphy_status);
        portstatus = qegphy_status.recovered_clk_lock & (1 << port);

        /*only when link is up go further*/
        if (rc || !portstatus)
        {
            port_status->port_up = 0;
            port_status->rate = LPORT_RATE_UNKNOWN;
            return rc;
        }

        rc = lport_get_phyid(port, &phyid);
        rc = rc ? rc : lport_get_brcm_phy_status(phyid, port_status);
        break;

        /*in case of RGMII, check if IB status is enabled before trying to go to phy*/
    case PORT_RGMII:
        if (local_lport_rgmii_cfg[port-LPORT_FIRST_RGMII_PORT].valid)
        {
            if(local_lport_rgmii_cfg[port-LPORT_FIRST_RGMII_PORT].phy_attached &&
                local_lport_rgmii_cfg[port-LPORT_FIRST_RGMII_PORT].ib_status_overide)
            {
                rc = lport_get_phyid(port, &phyid);
                rc = rc ? rc : lport_get_brcm_phy_status(phyid, port_status);
                if (rc)
                {
                    local_lport_rgmii_cfg[port-LPORT_FIRST_RGMII_PORT].phy_attached = 0;
                    pr_err("Port %d: External PHY not attached\n", port);
                }
                rc = rc ? rc : lport_set_rgmii_ib_status(port - LPORT_FIRST_RGMII_PORT, port_status);
            }
            else
            {
                rc = lport_get_rgmii_ib_status(port - LPORT_FIRST_RGMII_PORT, port_status);
            }
        }
        else
        {
            rc = LPORT_ERR_STATE;
        }
        break;

    case PORT_SGMII_SLAVE ... PORT_XFI:
        {
            uint8_t msk = 0x2;
            lport_srds_dual_serdes_0_status srds_0_status = {};
            lport_srds_dual_serdes_1_status srds_1_status = {};
            uint8_t port_stts = 0;

            switch (port)
            {
            case 0:
            case 7:
                msk = 0x1;
            case 1:
            case 4:
                rc = ag_drv_lport_srds_dual_serdes_0_status_get(&srds_0_status);
                port_stts = (srds_0_status.link_status & msk) ? 1 : 0; 
                break;
            case 2:
            case 5:
                msk = 0x1;
            case 3:
            case 6:
                rc = ag_drv_lport_srds_dual_serdes_1_status_get(&srds_1_status);
                port_stts = (srds_1_status.link_status & msk) ? 1 : 0; 
                break;
            }

            if (port_stts)
                rc = lport_serdes_get_status(port, port_status);
            else
                port_status->port_up = port_stts;
        }
    }
    return rc;
}

int lport_rgmii_ate_config(uint32_t port, lport_rgmii_ate_s *rgmii_ate_conf)
{
    int rc;
    lport_rgmii_ate_tx_cntrl ate_tx_cntrl;
    uint32_t rgmii_id = port - LPORT_FIRST_RGMII_PORT;

    rc = ag_drv_lport_rgmii_ate_tx_cntrl_get(rgmii_id, &ate_tx_cntrl);
    ate_tx_cntrl.payload_length = rgmii_ate_conf->payload_len;
    rc = rc ? rc : ag_drv_lport_rgmii_ate_tx_cntrl_set(rgmii_id, &ate_tx_cntrl);

    rc = rc ? rc : ag_drv_lport_rgmii_ate_tx_data_0_set(rgmii_id, rgmii_ate_conf->rgmii_da_mac[1],
        rgmii_ate_conf->rgmii_da_mac[0]);
    rc = rc ? rc : ag_drv_lport_rgmii_ate_tx_data_1_set(rgmii_id, rgmii_ate_conf->rgmii_da_mac[3],
        rgmii_ate_conf->rgmii_da_mac[2]);
    rc = rc ? rc : ag_drv_lport_rgmii_ate_tx_data_2_set(rgmii_id, 0x800,
        rgmii_ate_conf->rgmii_da_mac[5], rgmii_ate_conf->rgmii_da_mac[4]);
    return rc;
}

int lport_rgmii_ate_start(uint32_t port, uint32_t num_of_packets, uint8_t pkt_gen_en)
{
    int rc;
    lport_rgmii_ate_tx_cntrl ate_tx_cntrl;
    lport_rgmii_ate_rx_cntrl_exp_data ate_rx_cntrl_exp_data;
    uint32_t rgmii_id = port - LPORT_FIRST_RGMII_PORT;

    rc = ag_drv_lport_rgmii_ate_rx_cntrl_exp_data_get(rgmii_id, &ate_rx_cntrl_exp_data);
    ate_rx_cntrl_exp_data.ate_en = 1;
    rc = rc ? rc : ag_drv_lport_rgmii_ate_rx_cntrl_exp_data_get(rgmii_id, &ate_rx_cntrl_exp_data);
    rc = rc ? rc : ag_drv_lport_rgmii_ate_tx_cntrl_get(rgmii_id, &ate_tx_cntrl);
    ate_tx_cntrl.pkt_cnt = num_of_packets;
    ate_tx_cntrl.pkt_gen_en = pkt_gen_en;
    rc = rc ? rc : ag_drv_lport_rgmii_ate_tx_cntrl_set(rgmii_id, &ate_tx_cntrl);
    return rc;
}

int lport_port_mtu_get(uint32_t portid, uint16_t *port_mtu)
{
    return ag_drv_lport_xlmac_rx_max_size_get(portid, port_mtu);
}

int lport_port_mtu_set(uint32_t portid, uint16_t port_mtu)
{
    uint8_t xlmacid = portid >> 2;
    uint8_t instance = portid % 4;

    /* set the XLMAC MTU */
    ag_drv_lport_xlmac_rx_max_size_set(portid, port_mtu);

    /* align the MIB max packet size for accurate accounting */
    switch(instance)
    {
    case 0:
        return ag_drv_mib_conf_gport0_max_pkt_size_set(xlmacid, port_mtu);
    case 1:
        return ag_drv_mib_conf_gport1_max_pkt_size_set(xlmacid, port_mtu);
    case 2:
        return ag_drv_mib_conf_gport2_max_pkt_size_set(xlmacid, port_mtu);
    case 3:
        return ag_drv_mib_conf_gport3_max_pkt_size_set(xlmacid, port_mtu);
    default:
        pr_err("%s(%d):Wrong portid %d",__FUNCTION__, __LINE__, portid);
        return LPORT_ERR_PARAM;
    }
}

int lport_get_port_link_status(uint32_t port, uint8_t *link_up)
{
    lport_ctrl_qegphy_status qegphy_status;

    switch(local_lport_cfg.prt_mux_sel[port])
    {
    case PORT_GPHY:
        ag_drv_lport_ctrl_qegphy_status_get(&qegphy_status);
        *link_up = qegphy_status.recovered_clk_lock & (1 << port) ? 1 : 0;
        break;
    case PORT_RGMII:
        {
            uint8_t ib_status_ovrd;
            uint8_t link_decode;
            uint8_t duplex_decode;
            uint8_t speed_decode;

            ag_drv_lport_rgmii_ib_status_get(port - LPORT_FIRST_RGMII_PORT, &ib_status_ovrd, &link_decode,
                &duplex_decode, &speed_decode);
            *link_up = link_decode;
            break;
        }
    case PORT_SGMII_SLAVE:
    case PORT_SGMII_1000BASE_X:
    case PORT_HSGMII:
    case PORT_XFI:
        break;
    case PORT_UNAVAIL:
        *link_up = 0;
        break;
    }

    return LPORT_ERR_OK;
}

int lport_port_eee_set(uint32_t portid, uint8_t enable)
{
    int ret = 0;
    uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
    uint16_t eee_ref_count = 0, eee_wake_timer = 0;
    uint32_t eee_delay_entry_timer = 0;

    if ((ret = ag_drv_lport_xlmac_eee_ctrl_set(portid, 0, enable)))
        return ret;

    /* Determine EEE timers only when EEE is enabled */
    if (enable)
    {
        if ((ret = ag_drv_lport_xlmac_mode_get(portid, &speed_mode, &no_sop_for_crc_hg, &hdr_mode)))
            return ret;

        eee_ref_count = 0x190; /* 400 Mhz */

        switch (speed_mode) {
        case 1: /* 100Mbps */
            {
                eee_wake_timer = 0x1e; /* 30 uS */
                eee_delay_entry_timer = 0x3c; /* 60 uS */
                break;
            }
        case 2: /* 1000Mbps */
            {
                eee_wake_timer = 0x11; /* 17 uS */
                eee_delay_entry_timer = 0x22; /* 34 uS */
                break;
            }
        case 3: /* 2500Mbps */
        case 4: /* 10000Mbps */
            {
                eee_wake_timer = 0x08; /* 8 uS */
                eee_delay_entry_timer = 0x10; /* 16 uS */
                break;
            }
        default:
            return -1;
        }
    }

    if ((ret = ag_drv_lport_xlmac_eee_timers_set(portid, eee_ref_count, eee_wake_timer, eee_delay_entry_timer)))
        return ret;

#ifndef _CFE_
    pr_debug("eee_set: port=%d enable=%d ref_count=0x%x, wake_timer=0x%x, delay_entry_timer=0x%x\n",
        portid, enable, eee_ref_count, eee_wake_timer, eee_delay_entry_timer);
#endif

    return 0;
}

char *lport_rate_to_str(LPORT_PORT_RATE rate)
{
    switch (rate)
    {
    case LPORT_RATE_10MB: return "10M";
    case LPORT_RATE_100MB: return "100M";
    case LPORT_RATE_1000MB: return "1G";
    case LPORT_RATE_2500MB: return "2.5G";
    case LPORT_RATE_10G: return "10G";
    default: return "Unknown";
    }
    return "Unkown";
}

int lport_str_to_rate(char *str)
{
    if (!strcmp(str, "10M"))
        return LPORT_RATE_10MB;
    if (!strcmp(str, "100M"))
        return LPORT_RATE_100MB;
    if (!strcmp(str, "1G"))
        return LPORT_RATE_1000MB;
    if (!strcmp(str, "2.5G"))
        return LPORT_RATE_2500MB;
    if (!strcmp(str, "10G"))
        return LPORT_RATE_10G;

    return LPORT_RATE_UNKNOWN;
}

