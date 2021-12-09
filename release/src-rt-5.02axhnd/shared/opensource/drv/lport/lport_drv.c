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
 *  Created on: April 2015
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
void remap_ru_block_addrs(uint32_t block_index, const ru_block_rec *ru_blocks[]);
#define UDELAY udelay
#endif
#include "mii_shared.h"
#include "boardparms.h"
#include "bcm_map_part.h"

int lport_set_rgmii_ib_status(uint32_t rgmii_id, lport_port_status_s *port_status);

#define LPORT_XLMAC_G9991_RSV_MASK (0x20058)
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

static lport_init_s lport_init_cfg = {};
static lport_init_s lport_curr_cfg = {};
static lport_rgmii_cfg_s local_lport_rgmii_cfg[LPORT_NUM_OF_RGMII] = {{}};

int validate_lport_configuration(lport_init_s *init_params)
{
    int iter;
    LPORT_PORT_MUX_SELECT *prt_mux_sel = init_params->prt_mux_sel;

    //validate only port 0 & 4 are assigned as possible 10G
    for (iter = 0; iter < LPORT_NUM_OF_PORTS; iter++)
    {
        //valivate RGMII only from XLMAC1 only
        if (iter < 4 && prt_mux_sel[iter] == PORT_RGMII)
        {
            pr_err("Port %d can't work in RGMII mode\n", iter);
            return LPORT_ERR_PARAM;
        }
        if (iter > 3 && prt_mux_sel[iter] == PORT_GPHY)
        {
            pr_err("Port %d can't work in GPHY mode\n", iter);
            return LPORT_ERR_PARAM;
        }
        if (iter != 0 && iter != 4 && LPORT_IS_XFI_PORT(prt_mux_sel[iter]))
        {
            pr_err("Port %d can't work in XFI mode\n", iter);
            return LPORT_ERR_PARAM;
        }
    }

    //validate no collision on serdes mux
    if (LPORT_IS_SERDES_PORT(prt_mux_sel[0]) && LPORT_IS_SERDES_PORT(prt_mux_sel[7]))
    {
        pr_err("ERROR: Only one port(0,7) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }

    if (LPORT_IS_SERDES_PORT(prt_mux_sel[1]) && LPORT_IS_SERDES_PORT(prt_mux_sel[4]))
    {
        pr_err("ERROR: Only one port(1,4) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }

    if (LPORT_IS_SERDES_PORT(prt_mux_sel[2]) && LPORT_IS_SERDES_PORT(prt_mux_sel[5]))
    {
        pr_err("ERROR: Only one port(2,5) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }

    if (LPORT_IS_SERDES_PORT(prt_mux_sel[3]) && LPORT_IS_SERDES_PORT(prt_mux_sel[6]))
    {
        pr_err("ERROR: Only one port(3,6) can be muxed to Merlin at the same time\n");
        return LPORT_ERR_PARAM;
    }

    return LPORT_ERR_OK;
}

static int lport_speed_to_xlmac(LPORT_PORT_RATE lport_speed)
{
    int ret = 0;

    switch (lport_speed)
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

LPORT_PORT_RATE lport_speed_get(uint8_t portid)
{
    uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
    LPORT_PORT_RATE lport_speed = LPORT_RATE_UNKNOWN;
    
    if (ag_drv_lport_xlmac_mode_get(portid, &speed_mode, &no_sop_for_crc_hg, &hdr_mode))
        return LPORT_RATE_UNKNOWN;
    
    switch (speed_mode)
    {
    case 0: /* 10M */
        lport_speed = LPORT_RATE_10MB;
        break;

    case 1: /* 100Mbps */
        lport_speed = LPORT_RATE_100MB;
        break;

    case 2: /* 1000Mbps */
        lport_speed = LPORT_RATE_1000MB;
        break;

    case 3: /* 2500Mbps */
        lport_speed = LPORT_RATE_2500MB;
        break;

    case 4: /* 5000/10000Mbps */
        lport_speed = LPORT_RATE_10G;
        break;

    default:
        lport_speed = LPORT_RATE_UNKNOWN;
    }
    
    return lport_speed;
}


static int xlmac_init(lport_init_s *init_params)
{
    lport_xlmac_ctrl     ctrl;
    lport_xlmac_tx_ctrl  tx_ctrl;
    lport_xlmac_rx_ctrl  rx_ctrl;
#ifdef CONFIG_BCM_PTP_1588
    xlmac_conf_config config = {};
#endif
    uint32_t i;
    int rc = LPORT_ERR_OK;

    for (i = 0; i < LPORT_NUM_OF_PORTS; i++)
    {
        LPORT_PORT_MUX_SELECT prt_mux_sel = init_params->prt_mux_sel[i];

        if (prt_mux_sel != PORT_UNAVAIL)
        {
            rc = ag_drv_lport_xlmac_ctrl_get(i, &ctrl);
            rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_get(i, &tx_ctrl);

#ifdef G9991
            /*In G9991 Application, DSP engine sends to XLMAC segmented frames which are not
            * native ethernet frames, XLMAC will try to match ethertype to EthernetII and will fail
            * then it will pass to BBH RX failure signal, need to mask this failure bit to prevent it*/
            rc = rc ? rc : ag_drv_xlmac_conf_port_0_rxerr_mask_set(0, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_1_rxerr_mask_set(0, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_2_rxerr_mask_set(0, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_3_rxerr_mask_set(0, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_0_rxerr_mask_set(1, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_1_rxerr_mask_set(1, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_2_rxerr_mask_set(1, LPORT_XLMAC_G9991_RSV_MASK);
            rc = rc ? rc : ag_drv_xlmac_conf_port_3_rxerr_mask_set(1, LPORT_XLMAC_G9991_RSV_MASK);
#endif
            /*TODO: Change crc_mode to PER_PACKET when HW confirm usage
             * when doing REPLACE packet must be padded by 4 bytes*/
            tx_ctrl.crc_mode = XLMAC_CRC_PERPKT;
            tx_ctrl.pad_en = 1;

            /*In order to prevent Underflow*/
            tx_ctrl.tx_threshold = LPORT_TX_THRESHOLD;
            rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_set(i, &tx_ctrl);

            rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_get(i, &rx_ctrl);
            rx_ctrl.strip_crc = 0;
            rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_set(i, &rx_ctrl);

            switch (prt_mux_sel)
            {
            case PORT_SFI ... PORT_XFI:
                rc = rc ? rc : ag_drv_lport_xlmac_mode_set(i, lport_speed_to_xlmac(LPORT_RATE_10G), 0, 0);
                break;
            case PORT_HSGMII:
                rc = rc ? rc : ag_drv_lport_xlmac_mode_set(i, lport_speed_to_xlmac(LPORT_RATE_2500MB), 0, 0);
                break;
            default:
                rc = rc ? rc : ag_drv_lport_xlmac_mode_set(i, lport_speed_to_xlmac(LPORT_RATE_1000MB), 0, 0);
            }

            rc = rc ? rc : ag_drv_lport_xlmac_tx_crc_corrupt_ctrl_set(i ,0 ,0 ,0, 0);

            ctrl.soft_reset = 0;
            rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(i, &ctrl);

            rc = rc ? rc : ag_drv_lport_xlmac_timestamp_adjust_set(i, 0, TS_TSTS_ADJ, TS_OSTS_ADJ);
        }
    }

#ifdef CONFIG_BCM_PTP_1588
    for (i = 0; i < LPORT_NUM_OF_XLMACS; i++)
    {
        rc = rc ? rc : ag_drv_xlmac_conf_config_get(i, &config);

        config.osts_timer_disable = 0;
        config.bypass_osts = 0;
        config.egr_1588_timestamping_mode = 1;

        rc = rc ? rc : ag_drv_xlmac_conf_config_set(i, &config);
    }
#endif

    return rc;
}

static int msbus_init(lport_init_s *init_params)
{
    lport_mab_cntrl mab_cntrl;
    lport_mab_tx_wrr_ctrl wrr;
    uint32_t i, j;
    int rc = LPORT_ERR_OK;

    for (i = 0; i < LPORT_NUM_OF_XLMACS; i++) 
    {
        rc = rc ? rc : ag_drv_lport_mab_cntrl_get(i, &mab_cntrl);
        rc = rc ? rc : ag_drv_lport_mab_tx_wrr_ctrl_get(i, &wrr);

        mab_cntrl.link_down_rst_en = 0x1;

        mab_cntrl.gmii_rx_rst = 0xf;
        mab_cntrl.gmii_tx_rst = 0xf;
        mab_cntrl.xgmii_rx_rst = 0x1;
        mab_cntrl.xgmii_tx_rst = 0x1;

        wrr.p4_weight = 0;
        wrr.p5_weight = 0;
        wrr.p6_weight = 0;
        wrr.p7_weight = 0;

        for (j = 0; j < LPORT_NUM_OF_PORTS_PER_XLMAC; j++)
        {
            uint32_t idx = i * LPORT_NUM_OF_PORTS_PER_XLMAC + j;

            if (init_params->prt_mux_sel[idx] != PORT_UNAVAIL)
            {
                if (LPORT_IS_XFI_PORT(init_params->prt_mux_sel[idx]))
                {
                    mab_cntrl.xgmii_rx_rst = 0x0;
                    mab_cntrl.xgmii_tx_rst = 0x0;
                    wrr.p4_weight = 3;
                }
                else
                {
                    mab_cntrl.gmii_rx_rst &= ~(1 << j);
                    mab_cntrl.gmii_tx_rst &= ~(1 << j);

                    if (j == 0)
                        wrr.p4_weight = 1;
                    if (j == 1)
                        wrr.p5_weight = 1;
                    if (j == 2)
                        wrr.p6_weight = 1;
                    if (j == 3)
                        wrr.p7_weight = 1;
                }
            }
        }

        wrr.arb_mode = 0;
        rc = rc ? rc : ag_drv_lport_mab_tx_wrr_ctrl_set(i, &wrr);
        rc = rc ? rc : ag_drv_lport_mab_cntrl_set(i , &mab_cntrl);
    }

    return rc;
}

int lport_mux_init(lport_init_s *init_params)
{
    lport_ctrl_control control = {};

    //go over all 8 ports and configure the output

    if (LPORT_IS_XFI_PORT(init_params->prt_mux_sel[0]))
        control.p0_mode = LPORT_XLMAC_XGMII;
    else
        control.p0_mode = LPORT_XLMAC_GMII;

    if (LPORT_IS_XFI_PORT(init_params->prt_mux_sel[4]))
        control.p4_mode = LPORT_XLMAC_XGMII;
    else
        control.p4_mode = LPORT_XLMAC_GMII;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[0]))
        control.gport_sel_0 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_0 = LPORT_MUX_REG_EGPHY;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[1]))
        control.gport_sel_1 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_1 = LPORT_MUX_REG_EGPHY;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[2]))
        control.gport_sel_2 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_2 = LPORT_MUX_REG_EGPHY;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[3]))
        control.gport_sel_3 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_3 = LPORT_MUX_REG_EGPHY;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[4]))
        control.gport_sel_4 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_4 = LPORT_MUX_REG_RGMII;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[5]))
        control.gport_sel_5 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_5 = LPORT_MUX_REG_RGMII;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[6]))
        control.gport_sel_6 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_6 = LPORT_MUX_REG_RGMII;

    if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[7]))
        control.gport_sel_7 = LPORT_MUX_REG_SERDES;
    else
        control.gport_sel_7 = LPORT_MUX_REG_RGMII;

    return ag_drv_lport_ctrl_control_set(&control);
}

static int rgmii_init(lport_init_s *init_params)
{
    uint32_t i;
    lport_rgmii_cfg_s rgmii_cfg;
    int rc = LPORT_ERR_OK;
    static const ETHERNET_MAC_INFO *emac_info;
    int port_flags;
    int phy_id;

    if ((emac_info = BpGetEthernetMacInfoArrayPtr()) == NULL)
    {
        pr_err("Error reading Ethernet MAC info from board params\n");
        return -1;
    }

    for (i = LPORT_FIRST_RGMII_PORT; i < LPORT_NUM_OF_PORTS; i++)
    {
        if (init_params->prt_mux_sel[i] != PORT_RGMII)
            continue;

        if (lport_get_rgmii_cfg(i, &rgmii_cfg))
        {
            pr_err("Failed to get RGMII configuration for lport %d\n", i);
            return -1;
        }

        phy_id = emac_info->sw.phy_id[i];
        port_flags = emac_info->sw.port_flags[i];

        rgmii_cfg.ib_status_overide = 0;
        rgmii_cfg.portmode = 3;
        rgmii_cfg.valid = 1;
        rgmii_cfg.delay_rx = IsPortRxInternalDelay(port_flags);
        rgmii_cfg.delay_tx = IsPortTxInternalDelay(port_flags);


        if (lport_set_rgmii_cfg(i, &rgmii_cfg))
        {
            pr_err("Failed to set RGMII configuration for lport %d\n", i);
            return -1;
        }

        if (!(phy_id & PHY_EXTERNAL))
        {

            lport_port_status_s port_status = {};

            port_status.port_up = 1;
            port_status.rate = LPORT_RATE_1000MB;
            port_status.duplex = 1;

            rc = rc ? rc : lport_set_rgmii_ib_status(i - LPORT_FIRST_RGMII_PORT, &port_status);
        }
    }

    return rc;
}

int lport_get_rgmii_cfg(uint32_t portid, lport_rgmii_cfg_s *rgmii_cfg)
{
    lport_rgmii_cntrl cntrl;
    lport_rgmii_rx_clock_delay_cntrl rx_clock_delay_cntrl;
    int rc;

    if (portid < LPORT_FIRST_RGMII_PORT)
    {
        pr_err("LPORT RGMII port %d is not RGMII.Should be 4,5,6 only.\n", portid);
        return LPORT_ERR_PARAM;
    }

    memcpy(rgmii_cfg, &local_lport_rgmii_cfg[portid-4], sizeof(lport_rgmii_cfg_s));

    if ((rc = ag_drv_lport_rgmii_cntrl_get(portid-4, &cntrl)))
        return rc;

    if ((rc = ag_drv_lport_rgmii_rx_clock_delay_cntrl_get(portid-4, &rx_clock_delay_cntrl)))
        return rc;

    rgmii_cfg->eee_enable = cntrl.tx_clk_stop_en;
    rgmii_cfg->rvmii_ref = cntrl.rvmii_ref_sel;
    rgmii_cfg->delay_rx = rx_clock_delay_cntrl.bypass ? 0 : 1;
    rgmii_cfg->delay_tx = cntrl.id_mode_dis ? 0: 1;
    rgmii_cfg->portmode = cntrl.port_mode;

    return 0;
}

int lport_set_rgmii_cfg(uint32_t portid, lport_rgmii_cfg_s *rgmii_cfg)
{
    lport_rgmii_cntrl cntrl;
    lport_rgmii_rx_clock_delay_cntrl rx_clock_delay_cntrl;
    int rc;

    if (portid < LPORT_FIRST_RGMII_PORT)
    {
        pr_err("LPORT RGMII port %d is not RGMII.Should be 4,5,6 only.\n", portid);
        return LPORT_ERR_PARAM;
    }

    if ((rc = ag_drv_lport_rgmii_cntrl_get(portid-4, &cntrl)))
        return rc;

    if ((rc = ag_drv_lport_rgmii_rx_clock_delay_cntrl_get(portid-4, &rx_clock_delay_cntrl)))
        return rc;

    cntrl.tx_clk_stop_en = rgmii_cfg->eee_enable;
    cntrl.rvmii_ref_sel = rgmii_cfg->rvmii_ref;
    cntrl.id_mode_dis = rgmii_cfg->delay_tx ? 0 : 1;
    cntrl.port_mode = rgmii_cfg->portmode;
    cntrl.rgmii_mode_en = rgmii_cfg->valid;

    rx_clock_delay_cntrl.iddq = 0;
    rx_clock_delay_cntrl.bypass = rgmii_cfg->delay_rx ? 0 : 1;
    rx_clock_delay_cntrl.dly_sel = 1;
    rx_clock_delay_cntrl.dly_override = 1;
    rx_clock_delay_cntrl.reset = 0;

    if ((rc = ag_drv_lport_rgmii_cntrl_set(portid-4, &cntrl)))
        return rc;

    if ((rc = ag_drv_lport_rgmii_rx_clock_delay_cntrl_set(portid-4, &rx_clock_delay_cntrl)))
        return rc;

    memcpy(&local_lport_rgmii_cfg[portid-4],rgmii_cfg, sizeof(lport_rgmii_cfg_s));

    return rc;
}

static int qgphy_init(lport_init_s *init_params)
{
    uint32_t i;
    uint8_t ports_enabled = 0;
    lport_ctrl_qegphy_cntrl qegphy_cntrl ;
    lport_ctrl_qegphy_status qegphy_status;
    uint8_t pll_refclk_sel, pll_sel_div5, pll_clk125_250_sel, phy_test_en;

    /* To ensure minimum power consumption, the following steps should be
     * applied to the GPHY control signals. This should be done immediately
     * upon startup, before the decision is made to enable the GPHY or not,
     * in order to cover all cases.
     */

    ag_drv_lport_ctrl_qegphy_cntrl_get(&qegphy_cntrl);
    ag_drv_lport_ctrl_qegphy_test_cntrl_get(&pll_refclk_sel, &pll_sel_div5, &pll_clk125_250_sel, &phy_test_en);

    /* Assert reset_n (active low) */
    qegphy_cntrl.phy_reset = 1;
    ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
    UDELAY(900);

    /* Set GPHY's pll_clk125_250_sel to 250MHz */
    ag_drv_lport_ctrl_qegphy_test_cntrl_set(pll_refclk_sel, pll_sel_div5, 1, phy_test_en);
    UDELAY(900);

    /* Deassert iddq_global_pwr and iddq_bias */
    qegphy_cntrl.iddq_global_pwr = 0;
    qegphy_cntrl.iddq_bias = 0;
    ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
    UDELAY(900);

    for (i = 0; i <= LPORT_LAST_EGPHY_PORT; i++)
    {
        if (init_params->prt_mux_sel[i] == PORT_GPHY)
        {
            ports_enabled |= (1 << i);
        }
    }

    if (!ports_enabled)
    {
        /* Assert iddq_global_pwr and iddq_bias */
        qegphy_cntrl.iddq_global_pwr = 1;
        qegphy_cntrl.iddq_bias = 1;
        ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
        UDELAY(900);

        /* Deassert reset_n */
        qegphy_cntrl.phy_reset = 0;
        ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
        UDELAY(900);

        ag_drv_lport_ctrl_qegphy_test_cntrl_set(pll_refclk_sel, pll_sel_div5, 0, phy_test_en);
        UDELAY(900);

        /* Assert reset_n (active low) */
        qegphy_cntrl.phy_reset = 1;
        ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);

        pr_err("LPORT EGPHY not muxed, skipping init\n");
        return LPORT_ERR_OK;
    }

    qegphy_cntrl.ext_pwr_down = ~ports_enabled & 0x0f;
    ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
    UDELAY(900);

    ag_drv_lport_ctrl_qegphy_test_cntrl_set(pll_refclk_sel, pll_sel_div5, 0, phy_test_en);
    UDELAY(900);

    /* Deassert reset_n */
    qegphy_cntrl.phy_reset = 0;
    ag_drv_lport_ctrl_qegphy_cntrl_set(&qegphy_cntrl);
    UDELAY(900);

    /* Check for PLL Lock */
    ag_drv_lport_ctrl_qegphy_status_get(&qegphy_status);

    if (!qegphy_status.pll_lock)
        pr_info("LPORT QEGPHY PLL is not Locked!");

    return LPORT_ERR_OK;
}

/* default leds configuration:
 * LED0:On for each speed 
 * LED1:On when link up and blinks on activity
 */

int lport_leds_init(lport_init_s *init_params)
{
    LEDS_ADVANCED_INFO led_info = {};
    lport_led_link_and_speed_encoding_sel spdlnk_sel;
    lport_led_link_and_speed_encoding spdlnk;
    lport_ctrl_led_serial_cntrl led_serial_cntrl;
    lport_led_cntrl led_ctrl;
    int port;
    uint8_t activity;
    int rc; 
    uint8_t activity_led = 0;

    rc = BpGetLedsAdvancedInfo(&led_info);
    if (rc != BP_SUCCESS )
    {
        pr_err("Error reading Led Advanced info from board params\n");
        return LPORT_ERR_OK;
    }

    ag_drv_lport_ctrl_led_serial_cntrl_get(&led_serial_cntrl);
    led_serial_cntrl.smode = 2;
    if(led_info.is_activity_led_present)
    {
        activity_led = 1;
        led_serial_cntrl.smode |= 1;
    }

    led_serial_cntrl.port_en = 0;
    for (port = 0; port < LPORT_NUM_OF_PORTS; port++)
    {
        if (init_params->prt_mux_sel[port] != PORT_UNAVAIL)
        {
            int j;

            led_serial_cntrl.port_en |= (1 << port);
            ag_drv_lport_led_link_and_speed_encoding_sel_get(port, &spdlnk_sel);
            ag_drv_lport_led_cntrl_get(port, &led_ctrl);
            ag_drv_lport_led_link_and_speed_encoding_get(port, &spdlnk);

            spdlnk_sel.sel_10g_encode = 0;
            spdlnk_sel.sel_1000m_encode = 0;
            spdlnk_sel.sel_100m_encode = 0;
            spdlnk_sel.sel_2500m_encode = 0;
            spdlnk_sel.sel_10m_encode = 0;
            spdlnk_sel.sel_no_link_encode = 0;

            spdlnk.m10g_encode = 7;
            spdlnk.m2500_encode = 7;
            spdlnk.m1000_encode = 7;
            spdlnk.m100_encode = 7;
            spdlnk.m10_encode = 7;
            spdlnk.no_link_encode = 7;
 
            for (j = 0; j < (MAX_LEDS_PER_PORT - 1); j++)
            {
                uint32_t led_mux = led_info.ledInfo[port].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
                uint32_t led_activity = led_info.ledInfo[port].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
                activity = 0;

                if (led_mux & BP_NET_LED_SPEED_10G)
                    spdlnk.m10g_encode &= ~(1<<j);
                if (led_mux & BP_NET_LED_SPEED_2500)
                    spdlnk.m2500_encode &= ~(1<<j);
                if (led_mux & BP_NET_LED_SPEED_1G)
                    spdlnk.m1000_encode &= ~(1<<j);
                if (led_mux & BP_NET_LED_SPEED_100)
                    spdlnk.m100_encode &= ~(1<<j);
                if (led_mux & BP_NET_LED_SPEED_10)
                    spdlnk.m10_encode &= ~(1<<j);

                if (led_activity & BP_NET_LED_SPEED_10G)
                    spdlnk_sel.sel_10g_encode |= (1<<j);
                if (led_activity & BP_NET_LED_SPEED_2500)
                    spdlnk_sel.sel_2500m_encode |= (1<<j);
                if (led_activity & BP_NET_LED_SPEED_1G)
                    spdlnk_sel.sel_1000m_encode |= (1<<j);
                if (led_activity & BP_NET_LED_SPEED_100)
                    spdlnk_sel.sel_100m_encode |= (1<<j);
                if (led_activity & BP_NET_LED_SPEED_10)
                    spdlnk_sel.sel_10m_encode |= (1<<j);

                /* to configure the speed led to show activity only for specified
                   speeds */
                if (led_activity && !(led_mux & led_activity))
                    activity = 1;

                 switch (j)
                {
                    case 0:
                        led_ctrl.spdlnk_led0_act_sel = activity;
                        break;
                    case 1:
                        led_ctrl.spdlnk_led1_act_sel = activity;
                        break;
                    case 2:
                        led_ctrl.spdlnk_led2_act_sel = activity;
                        break;
                }
            }
            
            if (activity_led)
            {
                if (led_info.ledInfo[port].SpeedLed[3] & BP_NET_LED_SPEED_MASK)
                    led_ctrl.act_led_act_sel = 0;
                else
                    led_ctrl.act_led_act_sel = 1;
            }
            ag_drv_lport_led_link_and_speed_encoding_sel_set(port, &spdlnk_sel);
            ag_drv_lport_led_cntrl_set(port, &led_ctrl);
            ag_drv_lport_led_link_and_speed_encoding_set(port, &spdlnk);
        }
    }
    ag_drv_lport_ctrl_led_serial_cntrl_set(&led_serial_cntrl);

    return 0;
}

int lport_reinit_driver(lport_init_s *init_params, int log)
{
    if (validate_lport_configuration(init_params))
    {
        pr_err("LPORT configuration validation failed\n");
        return LPORT_ERR_PARAM;
    }

    /* init lport mux */
    if (lport_mux_init(init_params))
    {
        pr_err("Init LPORT MUX failed\n");
        return LPORT_ERR_PARAM;
    }

    UDELAY(100);

    /* init serdes */
    if (lport_serdes_init(init_params))
    {
        pr_err("Failed to initialize Merlin Serdes\n");
#ifndef _CFE_
        return LPORT_ERR_PARAM;
#endif
    }
    if (log)
        pr_err("SERDES!\n");

    UDELAY(100);

    /* init msbus */
    if (msbus_init(init_params))
    {
        pr_err("Init LPORT MSBUS failed\n");
        return LPORT_ERR_PARAM;
    }
    if (log)
        pr_err("MSBUS!\n");

    UDELAY(100);

    /* save shadow copy of init configuration for runtime use */
    memcpy(&lport_curr_cfg, init_params, sizeof(lport_curr_cfg));

    return LPORT_ERR_OK;
}

int lport_init_driver(lport_init_s *init_params)
{
#ifndef _CFE_
    /* ioRemap virtual addresses of LPORT */
    remap_ru_block_addrs(LPORT_IDX, RU_LPORT_BLOCKS);
#endif

#ifndef _CFE_
    if (lport_intr_init())
    {
        pr_err("Init LPORT interrupts failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("INTR!\n");
#endif

    if (lport_mdio_bus_init())
    {
        pr_err("Init LPORT interrupts failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("MDIO!\n");

    if (qgphy_init(init_params))
    {
        pr_err("Init LPORT EGPHY failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("QEGPHY!\n");

    if (xlmac_init(init_params))
    {
        pr_err("Init LPORT XLMAC failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("XLMAC!\n");

    UDELAY(100);

    if (lport_reinit_driver(init_params, 1))
        return LPORT_ERR_PARAM;

    if (rgmii_init(init_params))
    {
        pr_err("Init LPORT RGMII failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("RGMII!\n");

    if (lport_leds_init(init_params))
    {
        pr_err("Init LPORT leds failed\n");
        return LPORT_ERR_PARAM;
    }
    pr_err("LED!\n");

    memcpy(&lport_init_cfg, init_params, sizeof(lport_init_cfg));

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
    port_conf->tx_threshold = xlmac_tx_ctrl.tx_threshold;
    port_conf->throt_num = xlmac_tx_ctrl.throt_num;
    port_conf->throt_denom = xlmac_tx_ctrl.throt_denom;
    port_conf->speed = lport_speed_get((uint8_t)portid);

    return rc;
}


int lport_set_port_configuration(uint32_t portid, lport_port_cfg_s *port_conf)
{
    int rc = 0;
    lport_xlmac_ctrl xlmac_ctrl;
    lport_xlmac_tx_ctrl xlmac_tx_ctrl;
    lport_xlmac_rx_ctrl xlmac_rx_ctrl;
    LPORT_PORT_MUX_SELECT prt_mux_sel = lport_curr_cfg.prt_mux_sel[portid];
    LPORT_PORT_MUX_SELECT new_mux_sel = prt_mux_sel;
    lport_init_s lport_cfg = lport_curr_cfg;

    if (prt_mux_sel == PORT_RGMII)
    {
        lport_port_status_s port_status = {};

        port_status.port_up = port_conf->speed == LPORT_RATE_UNKNOWN ? 0 : 1;
        port_status.rate = port_conf->speed;
        port_status.duplex = 1;

        rc = rc ? rc : lport_set_rgmii_ib_status(portid - LPORT_FIRST_RGMII_PORT, &port_status);
    }

    if (LPORT_IS_SERDES_PORT(lport_init_cfg.prt_mux_sel[portid]))
    {
        lport_cfg.prt_mux_sel[portid] =
            port_conf->speed == LPORT_RATE_UNKNOWN ? PORT_UNAVAIL : lport_init_cfg.prt_mux_sel[portid];
        rc = rc ? : lport_reinit_driver(&lport_cfg, 0);
        prt_mux_sel = new_mux_sel = lport_cfg.prt_mux_sel[portid];
    }

    if (port_conf->speed == LPORT_RATE_UNKNOWN)
        return rc;

    if ((prt_mux_sel == PORT_SGMII) || (prt_mux_sel == PORT_HSGMII) || (prt_mux_sel == PORT_XFI))
    {
        if (port_conf->speed == LPORT_RATE_100MB)
            new_mux_sel = PORT_SGMII;
        if (port_conf->speed == LPORT_RATE_1000MB)
            new_mux_sel = PORT_SGMII;
        if (port_conf->speed == LPORT_RATE_2500MB)
            new_mux_sel = PORT_HSGMII;
        if (port_conf->speed == LPORT_RATE_10G)
            new_mux_sel = PORT_XFI;

        if (prt_mux_sel != new_mux_sel)
        {
            lport_init_cfg.prt_mux_sel[portid] = new_mux_sel;
            lport_cfg.prt_mux_sel[portid] = new_mux_sel;
            rc = rc ? : lport_reinit_driver(&lport_cfg, 0);
        }

        if (port_conf->speed == LPORT_RATE_100MB)
            rc = rc ? : lport_serdes_speed_set(portid, port_conf->speed);
    }

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_get(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_get(portid, &xlmac_rx_ctrl);

    xlmac_ctrl.local_lpbk = port_conf->local_loopback;
    rc = rc ? rc : ag_drv_lport_xlmac_mode_set(portid, lport_speed_to_xlmac(port_conf->speed), 0, 0);

    xlmac_tx_ctrl.average_ipg = port_conf->average_igp;
    xlmac_tx_ctrl.pad_en = port_conf->pad_en;
    xlmac_tx_ctrl.pad_threshold = port_conf->pad_threashold;
    xlmac_tx_ctrl.tx_preamble_length = port_conf->tx_preamble_len;
    xlmac_tx_ctrl.throt_num = port_conf->throt_num;
    xlmac_tx_ctrl.throt_denom = port_conf->throt_denom;

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_tx_ctrl_set(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_lport_xlmac_rx_ctrl_set(portid, &xlmac_rx_ctrl);

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

static int lport_port_msbus_enable(uint32_t portid, uint8_t enable)
{
    uint8_t xlmac_id = portid < LPORT_NUM_OF_PORTS_PER_XLMAC ? 0 : 1;
    uint8_t bit = portid < LPORT_NUM_OF_PORTS_PER_XLMAC ? portid : portid - LPORT_NUM_OF_PORTS_PER_XLMAC;
    lport_mab_cntrl mab_cntrl;
    LPORT_PORT_MUX_SELECT prt_mux_sel = lport_init_cfg.prt_mux_sel[portid];
    int rc = LPORT_ERR_OK;

    rc = rc ? rc : ag_drv_lport_mab_cntrl_get(xlmac_id, &mab_cntrl);

    if (prt_mux_sel == PORT_XFI)
    {
        if (enable)
        {
            mab_cntrl.xgmii_rx_rst = 0;
            mab_cntrl.xgmii_tx_rst = 0;
        }
        else
        {
            mab_cntrl.xgmii_rx_rst = 1;
            mab_cntrl.xgmii_tx_rst = 1;
        }
    }
    else
    {
        if (enable)
        {
            mab_cntrl.gmii_rx_rst &= ~(1 << bit);
            mab_cntrl.gmii_tx_rst &= ~(1 << bit);
        }
        else
        {
            mab_cntrl.gmii_rx_rst |= (1 << bit);
            mab_cntrl.gmii_tx_rst |= (1 << bit);
        }
    }

    rc = rc ? rc : ag_drv_lport_mab_cntrl_set(xlmac_id, &mab_cntrl);

    return rc;
}

static int lport_port_xlamc_enable(uint32_t portid, uint8_t enable)
{
    lport_xlmac_ctrl xlmac_ctrl;
    int rc = LPORT_ERR_OK;

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);
    xlmac_ctrl.soft_reset = enable ? 0 : 1;
    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);

    return rc;
}

static int lport_port_xlif_enable(uint32_t portid, uint8_t enable)
{
    int rc = LPORT_ERR_OK;

    volatile xlif_ch *p_xlif_ch = XLIF_REG;

    if (enable)
        p_xlif_ch[portid].xlif_tx_if_channel.set_credits &= ~(1<<12);
    else
        p_xlif_ch[portid].xlif_tx_if_channel.set_credits = (1<<12);

    return rc;
}

static int lport_port_enable(uint32_t portid, uint8_t enable)
{
    int rc = LPORT_ERR_OK;

    if (enable)
    {
        rc = rc ? rc : lport_port_xlif_enable(portid, 1);
        UDELAY(1000);
        rc = rc ? rc : lport_port_xlamc_enable(portid, 1);
        UDELAY(1000);
        rc = rc ? rc : lport_port_msbus_enable(portid, 1);
        UDELAY(1000);
    }
    else
    {
        rc = rc ? rc : lport_port_msbus_enable(portid, 0);
        UDELAY(1000);
        rc = rc ? rc : lport_port_xlamc_enable(portid, 0);
        UDELAY(1000);
        rc = rc ? rc : lport_port_xlif_enable(portid, 0);
        UDELAY(1000);
    }

    return rc;
}

int lport_port_credits_restart(uint32_t portid)
{
    lport_xlmac_ctrl xlmac_ctrl;
    int rc = LPORT_ERR_OK;

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);
    xlmac_ctrl.rx_en = 0;
    xlmac_ctrl.tx_en = 0;
    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);

    rc = rc ? rc : lport_port_enable(portid, 0);
    rc = rc ? rc : lport_port_enable(portid, 1);

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);
    xlmac_ctrl.rx_en = 1;
    xlmac_ctrl.tx_en = 1;
    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);

    return rc;
}

int lport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en)
{
    int rc;
    lport_xlmac_ctrl xlmac_ctrl;

    rc = ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);

    *rx_en = xlmac_ctrl.rx_en;
    *tx_en = xlmac_ctrl.tx_en;

    return rc;
}

int lport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en)
{
    int rc;
    lport_xlmac_ctrl xlmac_ctrl;

    rc = ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);

    if (xlmac_ctrl.rx_en != rx_en && xlmac_ctrl.tx_en != tx_en)
    {
        if (rx_en && tx_en)
            lport_port_enable(portid, 1);
        else if (!rx_en && !tx_en)
            lport_port_enable(portid, 0);
    }

    rc = rc ? rc : ag_drv_lport_xlmac_ctrl_get(portid, &xlmac_ctrl);

    xlmac_ctrl.rx_en = rx_en;
    xlmac_ctrl.tx_en = tx_en;

    return rc ? rc : ag_drv_lport_xlmac_ctrl_set(portid, &xlmac_ctrl);
}

int  lport_get_phyid(uint32_t port, uint16_t *phyid)
{
    lport_ctrl_qegphy_cntrl qegphy_cntrl;
    int rc = LPORT_ERR_OK;

    if (lport_curr_cfg.prt_mux_sel[port] == PORT_UNAVAIL)
    {
        pr_err("%s(%d):LPORT failed to reset phy,Port %d is not configured\n"
            ,__FUNCTION__, __LINE__, port);
        return LPORT_ERR_STATE;
    }

    if (lport_curr_cfg.prt_mux_sel[port] == PORT_GPHY)
    {
        rc = ag_drv_lport_ctrl_qegphy_cntrl_get(&qegphy_cntrl);
        *phyid = qegphy_cntrl.phy_phyad + port;
    }
    else if (lport_curr_cfg.prt_mux_sel[port] == PORT_RGMII)
    {
        if (local_lport_rgmii_cfg[port - LPORT_FIRST_RGMII_PORT].phy_attached)
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
    if (rc)
        return rc;

    /*handle Phy Down */
    if (!phycfg->port_up)
    {
        rc = lport_mdio22_wr(phyid, MII_BMCR, BMCR_POWERDOWN);
        return rc;
    }

    /*now reset and config phy*/
    rc = lport_mdio22_wr(phyid, MII_BMCR, BMCR_RESET);

    UDELAY(200);
    if (phycfg->autoneg_en)
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
    switch (autoneg_hcd)
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
    if (link_decode)
    {
        port_status->port_up = 1;
        port_status->duplex = duplex_decode ? LPORT_FULL_DUPLEX:LPORT_HALF_DUPLEX;
        switch (speed_decode)
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

    switch (lport_curr_cfg.prt_mux_sel[port])
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
            if (local_lport_rgmii_cfg[port-LPORT_FIRST_RGMII_PORT].phy_attached &&
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

    case PORT_SGMII ... PORT_XFI:
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
    ag_drv_lport_xlmac_rx_max_size_set(portid, 0x3fff);

    /* align the MIB max packet size for accurate accounting */
    switch (instance)
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

    switch (lport_curr_cfg.prt_mux_sel[port])
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
    case PORT_SGMII ... PORT_XFI:
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

        eee_ref_count = 0x280; /* 640 Mhz */

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
            {
                eee_wake_timer = 0x20; /* 32 uS */
                eee_delay_entry_timer = 0x320; /* 800 uS */
                break;
            }
        case 4: /* 10000Mbps */
            {
                eee_wake_timer = 0x20; /* 32 uS */
                eee_delay_entry_timer = 0xc8; /* 200 uS */
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

#ifndef _CFE_
int lport_get_next_ts_from_fifo(uint8_t port_id, uint16_t *sequence_id, uint32_t *time_stamp)
{
    int rc;
    uint8_t entry_count, ts_entry_valid;

    ag_drv_lport_xlmac_tx_timestamp_fifo_status_get(port_id, &entry_count);

    pr_debug("lport_get_next_ts_from_fifo: entry_count = %u\n", entry_count);

    if (entry_count)
    {
        rc = ag_drv_lport_xlmac_tx_timestamp_fifo_data_get(port_id, &ts_entry_valid, sequence_id, time_stamp);

        if (rc || !ts_entry_valid)
        {
            pr_err("failed to get TS. rc = %d, valid = %u\n", rc, ts_entry_valid);
            return rc ? rc : LPORT_ERR_INVALID;
        }
    }
    else
        return LPORT_ERR_FIFO_EMPTY;

    return LPORT_ERR_OK;
}
EXPORT_SYMBOL(lport_get_next_ts_from_fifo);
#endif


