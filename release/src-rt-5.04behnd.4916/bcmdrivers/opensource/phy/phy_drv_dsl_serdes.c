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
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * Phy drivers for 63138, 63148, 4908
 */
#include "phy_drv_dsl_serdes.h"
#include "bcmsfp.h"
#include "trxbus.h"

#define PHY_SPEED_HIGHEST_SUPPORT PHY_SPEED_10000

DEFINE_MUTEX(serdes_mutex);

enum {SFP_MODULE_OUT, SFP_MODULE_IN, SFP_LINK_UP};

int phy_dsl_serdes_init(phy_dev_t *phy_dev);
static phy_serdes_t *serdes[MAX_SERDES_NUMBER];
static int serdes_index;

static void set_common_speed_range(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY ||
        !(phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED))
        cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &phy_serdes->common_speed_caps);
    else
        phy_serdes->common_speed_caps = phy_serdes->speed_caps;

    phy_serdes->common_highest_speed = phy_caps_to_max_speed(phy_serdes->common_speed_caps);
    phy_serdes->common_highest_speed_cap = phy_speed_to_cap(phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
    phy_serdes->common_lowest_speed = phy_caps_to_min_speed(phy_serdes->common_speed_caps);
    phy_serdes->common_lowest_speed_cap = phy_speed_to_cap(phy_serdes->common_lowest_speed, PHY_DUPLEX_FULL);
}

static int get_dt_config_xfi(phy_dev_t *phy_dev)
{
    const char *_config_xfi;
    int an_enabled = 0;
    char config_xfi[256];

    phy_serdes_t *phy_serdes = phy_dev->priv;
    dt_handle_t handle = phy_serdes->handle;  // from dt_priv

    _config_xfi = dt_property_read_string(handle, "config-xfi");
    if (!_config_xfi)
        return -1;

    strncpy(config_xfi, _config_xfi, sizeof(config_xfi)-1);
    config_xfi[sizeof(config_xfi)-1]=0;

    if ((an_enabled = strcasecmp(config_xfi+strlen(config_xfi)-strlen("_A"), "_A") == 0))
        config_xfi[strlen(config_xfi)-strlen("_A")] = 0;

    if (strcasecmp(config_xfi, "1000Base-X") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_1000BASE_X;
    if (strcasecmp(config_xfi, "2500Base-X") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_2500BASE_X;
    if (strcasecmp(config_xfi, "2.5GBase-X") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_2P5GBASE_X;
    if (strcasecmp(config_xfi, "5GBase-R") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_5GBASE_R;
    if (strcasecmp(config_xfi, "10GBase-R") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_10GBASE_R;
    if (strcasecmp(config_xfi, "USXGMII") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_USXGMII;

    phy_dev->configured_current_inter_phy_type = phy_dev->current_inter_phy_type;
    if (an_enabled && !INTER_PHY_TYPE_AN_SUPPORT(phy_dev->current_inter_phy_type))
    {
        printkwarn("Serdes at address %d with Mode %s does not support AN enabled, ignore it.\n",
            phy_dev->addr, config_xfi);
        an_enabled = 0;
    }
    if (INTER_PHY_TYPE_AN_ONLY(phy_dev->current_inter_phy_type))
        an_enabled = 1;

    phy_dev->an_enabled = an_enabled;
    if (an_enabled)
        phy_dev->configured_an_enable = PHY_CFG_AN_ON;
    else
        phy_dev->configured_an_enable = PHY_CFG_AN_OFF;
    phy_dev->common_inter_phy_types = phy_dev->configured_inter_phy_types = (1<<phy_dev->current_inter_phy_type);
    if ((phy_dev->inter_phy_types & phy_dev->configured_inter_phy_types) == 0)
        BUG_CHECK("Configured XFI Mode in DT File Is Not Supported by the Serdes at address %d\n", phy_dev->addr);

    return 0;
}

static int get_dt_config_speed(phy_dev_t *phy_dev)
{
    uint32_t speed;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    dt_handle_t handle = phy_serdes->handle;  // from dt_priv

    if ((speed = dt_property_read_u32(handle, "config-speed")) == -1)
    {
        speed = dt_property_read_u32(handle, "config_speed");
        if (speed == -1)
            return -1;
        else
            printkwarn("\"config_speed\" in DT file is deprecated, please use \"config-speed\"");
    }

    switch(speed)
    {
        case 1000:
            phy_serdes->config_speed = PHY_SPEED_1000;
            break;
        case 2500:
            phy_serdes->config_speed = PHY_SPEED_2500;
            break;
        case 5000:
            phy_serdes->config_speed = PHY_SPEED_5000;
            break;
        case 10000:
            phy_serdes->config_speed = PHY_SPEED_10000;
            break;
    }
    return 0;
}

#define SERDES_FIBER_SETTLE_DELAY_MS 200
#define SERDES_SHORT_TMR_MS 300
#define SERDES_LONG_TMR_MS  1500
#define SERDES_TMR_SHORT_WT 5
#define SERDES_TMR_LONG_WT  1

enum {
    SPD_TMR_UPDATE,
    SPD_TMR_RESET,
    SPD_TMR_RESTART,
};

static void dsl_serdes_poll_timer_op(phy_dev_t *phy_dev, int op)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    spd_dtc_tmr_t *tmr = &phy_serdes->tmr;

    if (op != SPD_TMR_RESET && (tmr->short_timer_weight == 0 || tmr->long_timer_weight == 0))
        return;

    switch (op)
    {
        case SPD_TMR_UPDATE:
            tmr->speed_detect_counts++;
            if (tmr->current_long_timer && tmr->speed_detect_counts == tmr->long_timer_weight)
            {
                tmr->speed_detect_interval = tmr->short_value_ms;
                tmr->speed_detect_counts = 0;
                tmr->current_long_timer = 0;
            }
            else if (tmr->speed_detect_counts == tmr->short_timer_weight)
            {
                tmr->speed_detect_interval = tmr->long_value_ms;
                tmr->speed_detect_counts = 0;
                tmr->current_long_timer = 1;
            }
            break;
        case SPD_TMR_RESTART:
            tmr->current_long_timer = 0;
            tmr->speed_detect_interval = tmr->short_value_ms;
            tmr->speed_detect_counts = 0;
            break;
        case SPD_TMR_RESET:
            tmr->current_long_timer = 0;
            tmr->speed_detect_counts = 0;
            tmr->short_value_ms = SERDES_SHORT_TMR_MS;
            tmr->short_timer_weight = SERDES_TMR_SHORT_WT;
            tmr->long_value_ms = SERDES_LONG_TMR_MS;
            tmr->long_timer_weight = 0;
            tmr->speed_detect_interval = tmr->short_value_ms;
            break;
    }
}

static void dsl_serdes_poll_timer_config(phy_dev_t *phy_dev, int stmr, int stmr_wt, int ltmr, int ltmr_wt)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    spd_dtc_tmr_t *tmr = &phy_serdes->tmr;

    if (stmr != -1)
    {
        tmr->short_value_ms = stmr;
        tmr->short_timer_weight = stmr_wt;
    }

    if (ltmr != -1)
    {
        tmr->long_value_ms = ltmr;
        tmr->long_timer_weight = ltmr_wt;
    }
    dsl_serdes_poll_timer_op(phy_dev, SPD_TMR_RESTART);
}

int phy_dsl_serdes_init(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;
    struct device *dev;
    int ret = 0;
    int bug = 0;
    long value;
    int supported_caps;

    if ((serdes_index+1) > ARRAY_SIZE(serdes))
        BUG_CHECK("******** ERROR: Too many Serdeses number to support.\n");

    if (phy_next) /* For external Copper PHY, initialize it first */
    {
        phy_next->phy_drv->init(phy_next);

        /* Adjust usxgmii_m_type for single port definition to accept
            both USXGMII_S and USXGMII_M from device tree */
        if (phy_dev_is_mphy(phy_dev) && phy_next->usxgmii_m_type == USXGMII_S)
            phy_dev->usxgmii_m_type = USXGMII_S;
    }

    serdes[serdes_index++] = phy_serdes;
    phy_dev->inter_phy_types = phy_serdes->inter_phy_types;

    if (phy_dev_is_mphy(phy_dev))
        phy_serdes->speed_caps = phy_usxgmii_m_speed_cap_adjust(phy_dev, phy_serdes->speed_caps);

    dsl_serdes_poll_timer_op(phy_dev, SPD_TMR_RESET);
    phy_serdes->highest_speed = phy_caps_to_max_speed(phy_serdes->speed_caps & (~PHY_CAP_AUTONEG));
    phy_serdes->lowest_speed = phy_caps_to_min_speed(phy_serdes->speed_caps & (~PHY_CAP_AUTONEG));
    phy_serdes->highest_speed_cap = phy_speed_to_cap(phy_serdes->highest_speed, PHY_DUPLEX_FULL);
    phy_serdes->lowest_speed_cap = phy_speed_to_cap(phy_serdes->lowest_speed, PHY_DUPLEX_FULL);
    phy_serdes->usxgmii_m_index = phy_dev->usxgmii_m_index;
    phy_dev->configured_current_inter_phy_type = INTER_PHY_TYPE_AUTO;
    phy_dev->configured_an_enable = PHY_CFG_AN_AUTO;
    dsl_serdes_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &supported_caps);
    phy_serdes->adv_caps = phy_serdes->config_speed == PHY_SPEED_AUTO? (supported_caps & PHY_CAP_PURE_SPEED_CAPS) :
        phy_speed_to_cap(phy_serdes->config_speed, PHY_DUPLEX_FULL);
    set_common_speed_range(phy_dev);

    if (PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev))
    {
            /* Inter switch speed will use definition in the following priority
            1. DT, 2. Init config_speed value in phy_serdes_t 3. highest speed */
        if (get_dt_config_speed(phy_dev) == 0)
            phy_serdes->current_speed = phy_serdes->config_speed;

        if (get_dt_config_xfi(phy_dev) == 0)
        {
            uint32_t supported_speed_caps;
            get_inter_phy_supported_speed_caps(phy_dev->configured_inter_phy_types, &supported_speed_caps);
            phy_serdes->current_speed = phy_serdes->config_speed = phy_caps_to_max_speed(supported_speed_caps);
        }

        if (phy_serdes->config_speed == PHY_SPEED_UNKNOWN)
            phy_serdes->config_speed = phy_serdes->highest_speed;
        phy_serdes->adv_caps = phy_speed_to_cap(phy_serdes->config_speed, PHY_DUPLEX_FULL);
        set_common_speed_range(phy_dev);
        phy_serdes->power_mode = SERDES_NO_POWER_SAVING;
        dsl_serdes_power_set(phy_dev, 1);
        phy_serdes->signal_detect_gpio = -1;
        goto end;
    }

    phy_serdes->laser_status = -1;
    dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF); /* Notify no SFP to turn off laser in the beginning, just in case hardware set on */
    phy_serdes->sfp_module_type = SFP_FIXED_PHY;

    if ((phy_next == NULL))
    {
        phy_serdes->sfp_module_type = SFP_NO_MODULE;
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
        phy_drv_dsl_i2c_create_lock(phy_dev);
#endif
        phy_next = phy_dev->cascade_next;
    }

    /* Check if SFP signal detect pin is defined or not*/
    phy_serdes->signal_detect_gpio = -1;
    if (phy_next && phy_serdes->sfp_module_type == SFP_NO_MODULE) {
        dev = trxbus_get_dev(phy_next->addr);
        if (dev) {
            if (sfp_mon_read(dev, bcmsfp_mon_rxsd_pin, 0, &value) == 0) {
                phy_serdes->signal_detect_gpio = (short)value;
            }
            /* turn on SFP tx power for AE/PON cage */
            sfp_mon_write(dev, bcmsfp_mon_tx_enable, 0, 1);
        }

        if (phy_serdes->signal_detect_gpio == -1) {
            printk("Error: ****** Loss Of Signal Detection pin not defined SFP on bus %d.\n",
                phy_next->addr);
            printk("    Add signal detect pin in the default pinmux state for the sfp node in board dts file.\n");
            bug = 1;
        }
    }

    if (bug)
        BUG_CHECK("Serdes Initialization failed\n");
end:
    return ret;
}

char *dsl_serdes_get_phy_name(phy_dev_t *phy_dev)
{
    static char buf[128];
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int sz = sizeof(buf), n = 0;

    if(phy_dev_is_mphy(phy_dev))
        n += snprintf(buf+n, sz-n, "%s#M%d.%d", phy_dev->phy_drv->name,
            phy_dev->core_index, phy_serdes->usxgmii_m_index);
    else
        n += snprintf(buf+n, sz-n, "%s#L%d.%d", phy_dev->phy_drv->name,
            phy_dev->core_index, phy_dev->lane_index);
    return buf;
}

static int phy_dsl_set_configured_types(phy_dev_t *phy_dev)
{
    phy_dev_t *next_phy;

    cascade_phy_set_common_inter_types(phy_dev);
    phy_dev->configured_inter_phy_types = phy_dev->common_inter_phy_types;
    next_phy = cascade_phy_get_next(phy_dev);
    if (next_phy)
    {
        phy_dev_configured_inter_phy_types_set(next_phy, INTER_PHY_TYPE_UP, phy_dev->common_inter_phy_types);
    }
    return 0;
}

/* best type for SFP module */
static int sfp_phy_get_best_inter_phy_configure_type(phy_dev_t *phy_dev,
    int inter_phy_types, phy_speed_t speed)
{
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    int best_type;

    /* Check if non multi speed mode is available since most SFP module only support non multi speed mode */
    if (phy_i2c->flag & PHY_FLAG_DYNAMIC && !(phy_i2c->flag & PHY_FLAG_NOT_PRESENTED) &&
        (phy_i2c->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE))
        best_type = phy_get_best_inter_phy_configure_type(phy_dev,
                inter_phy_types & ((~INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M)|INTER_PHY_TYPE_SGMII_M), speed);
    else
        best_type = phy_get_best_inter_phy_configure_type(phy_dev,
                inter_phy_types & (~INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M), speed);


    /* If not available for this speed, then check multi speed mode availability */
    if (best_type == INTER_PHY_TYPE_UNKNOWN)
        best_type = phy_get_best_inter_phy_configure_type(phy_dev, inter_phy_types, speed);

    return best_type;
}

void phy_serdes_polarity_config(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_dev->xfi_tx_polarity_inverse)
    {
        if (!phy_dev->phy_drv->xfi_tx_polarity_inverse_set)
            printkwarn("Driver Missing for Tx Polarity Inverse of Serdes %s at %d\n",
                    phy_dev_get_phy_name(phy_dev), phy_dev->addr);
        else if(phy_serdes->print_log)
            printk("Invert XFI Tx Polarity of Serdes %s at %d\n", phy_dev_get_phy_name(phy_dev), phy_dev->addr);
        phy_dev_xfi_tx_polarity_set(phy_dev, phy_dev->xfi_tx_polarity_inverse);
    }

    if (phy_dev->xfi_rx_polarity_inverse)
    {
        if (!phy_dev->phy_drv->xfi_rx_polarity_inverse_set)
            printkwarn("Driver Missing for Rx Polarity Inverse of Serdes %s at %d\n",
                    phy_dev_get_phy_name(phy_dev), phy_dev->addr);
        else if(phy_serdes->print_log)
            printk("Invert XFI Rx Polarity of Serdes %s at %d\n", phy_dev_get_phy_name(phy_dev), phy_dev->addr);
        phy_dev_xfi_rx_polarity_set(phy_dev, phy_dev->xfi_rx_polarity_inverse);
    }
}

static int phy_dsl_txfir_default(phy_txfir_t *txfir)
{
    return (txfir->pre == txfir->pre_def &&
            txfir->main == txfir->main_def &&
            txfir->post1 == txfir->post1_def &&
            txfir->post2 == txfir->post2_def &&
            txfir->hpf == txfir->hpf_def);
}

static int phy_dsl_txfir_default_is_blank(phy_txfir_t *txfir)
{
    return  (txfir->pre_def == 0 && txfir->main_def == 0 && txfir->post1_def == 0 && txfir->post2_def == 0);
}

static int phy_dsl_txfir_is_blank(phy_txfir_t *txfir)
{
    return  (txfir->pre == 0 && txfir->main == 0 && txfir->post1 == 0 && txfir->post2 == 0);
}

static int phy_dsl_verify_txfir(phy_dev_t *phy_dev, phy_txfir_t *txfir)
{
    phy_txfir_t save_txfir;
    int rc;
    int org_power_level;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    org_power_level = phy_serdes->cur_power_level;
    if (phy_serdes->cur_power_level == SERDES_POWER_DOWN)
        dsl_powerup_serdes(phy_dev);

    dsl_phy_tx_cfg_get(phy_dev, &save_txfir.pre, &save_txfir.main,
            &save_txfir.post1_def, &save_txfir.post2, &save_txfir.hpf);

    rc = phy_dev_tx_cfg_set(phy_dev, txfir->pre, txfir->main,
            txfir->post1, txfir->post2, txfir->hpf);

    phy_dev_tx_cfg_set(phy_dev, save_txfir.pre, save_txfir.main,
            save_txfir.post1_def, save_txfir.post2, save_txfir.hpf);

    if (org_power_level != phy_serdes->cur_power_level)
        dsl_powerdown_serdes(phy_dev);

    return rc;
}

static int dsl_set_txfir_default(phy_txfir_t *txfir)
{
    txfir->pre = txfir->pre_def;
    txfir->main = txfir->main_def;
    txfir->post1 = txfir->post1_def;
    txfir->post2 = txfir->post2_def;
    return 0;
}

int phy_dsl_txfir_init(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int i, rc = 0;
    phy_txfir_t *txfir, _txfir;

    if (phy_serdes->read_txfir_reg)
    {
        for (i=0; i<ARRAY_SIZE(phy_dev->txfir); i++)
        {
            int speed[] = {PHY_TXFIR_SPEED_MBPS};

            txfir = &phy_dev->txfir[i];

            txfir->speed_mbps = speed[i];
            if (phy_dsl_txfir_default_is_blank(txfir))
                rc += dsl_phy_tx_cfg_get(phy_dev, &txfir->pre_def, &txfir->main_def,
                        &txfir->post1_def, &txfir->post2_def, &txfir->hpf_def);

            if (phy_dsl_txfir_is_blank(txfir))
                rc += dsl_phy_tx_cfg_get(phy_dev, &txfir->pre, &txfir->main,
                        &txfir->post1, &txfir->post2, &txfir->hpf);

            if (txfir->hpf == -1)
            {
                rc += dsl_phy_tx_cfg_get(phy_dev, &_txfir.pre, &_txfir.main,
                        &_txfir.post1, &_txfir.post2, &_txfir.hpf);
                txfir->hpf = _txfir.hpf;
            }

            if (!phy_dsl_txfir_default(txfir) &&
                    phy_serdes->cur_power_level == SERDES_POWER_UP)
            {
                if (phy_dsl_verify_txfir(phy_dev, txfir))
                {
                    printkwarn("WARNING: TXFIR Value Invalid: pre:%d, main:%d, post1:%d, post2:%d, hpf:%d. Use Default Values.",
                        txfir->pre, txfir->main, txfir->post1, txfir->post2, txfir->hpf);
                    dsl_set_txfir_default(txfir);
                }

                rc += phy_dev_tx_cfg_set(phy_dev, txfir->pre, txfir->main,
                        txfir->post1, txfir->post2, txfir->hpf);
                phy_dev->txfir_reg_speed_idx = i;
            }
        }
    }
    return rc;
}

int phy_dsl_serdes_post_init(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    phy_dsl_set_configured_types(phy_dev);
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
    {

        if (phy_dev->common_inter_phy_types == 0)
            /* Just unbelive this can happen on board design, should be software bug */
#ifdef TUFAX3000_V2
            printk("No common INTER PHY Capablities found between Serdes and PHY! Wrong board design.\n");
#else
            BUG_CHECK("No common INTER PHY Capablities found between Serdes and PHY! Wrong board design.\n");
#endif

        if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
            !IS_USXGMII_MULTI_PORTS(phy_dev) && !phy_dev_is_broadcom_phy(phy_dev->cascade_next))
        /* Work around for some non Broadcom PHYs not sync link status between Copper side and Serdes side */
        /* Exclude external switch connection from power down operation */
        {
            phy_dsl_txfir_init(phy_dev);
            dsl_serdes_power_set(phy_dev, 0);
            return 0;
        }
    }
    else
        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);

    phy_dsl_txfir_init(phy_dev);
    return dsl_serdes_cfg_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
}

static int phy_dsl_serdes_power_set(phy_dev_t *phy_dev, int enable)
{
    int rc = 0;
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;

    phy_serdes->power_admin_on = enable > 0;

    /* Bypass power on when external PHY link is down; - For non BRCM PHY and power saving */
    /* Exclude external switch connection from power down operation */
    if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
        phy_serdes->sfp_module_type == SFP_FIXED_PHY && phy_dev->usxgmii_m_type == USXGMII_M_NONE &&
        phy_dev->cascade_next->link == 0 && enable && phy_serdes->power_mode >= SERDES_BASIC_POWER_SAVING &&
        !phy_dev_is_broadcom_phy(phy_dev->cascade_next))
        return 0;

    dsl_serdes_power_set(phy_dev, enable);

    if (enable)
        dsl_serdes_cfg_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
    return rc;
}

int phy_dsl_serdes_power_set_lock(phy_dev_t *phy_dev, int enable)
{
    int rc;
    mutex_lock(&serdes_mutex);
    rc = phy_dsl_serdes_power_set(phy_dev, enable);
    mutex_unlock(&serdes_mutex);
    return rc;
}

int phy_dsl_serdes_power_get(phy_dev_t *phy_dev, int *enable)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    *enable = phy_serdes->power_admin_on > 0;
    return 0;
}

int dsl_phy_enable_an(phy_dev_t *phy_dev)
{
    u16 val16 = 0;

    phy_bus_read(phy_dev, MII_CONTROL, &val16);

    if (val16 & MII_CONTROL_AN_ENABLE)
        return 0;

    val16 |= MII_CONTROL_AN_ENABLE|MII_CONTROL_RESTART_AUTONEG;
    phy_bus_write(phy_dev, MII_CONTROL, val16);
    msleep(50);
    return 1;
}

#include "crossbar_dev.h"
phy_dev_t *crossbar_get_phy_by_type(int phy_type);

static int dsl_sfp_module_detect(phy_dev_t *phy_dev);
#if 0
static inline int dsl_serdes_msleep_on_sfp_unplug(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    spd_dtc_tmr_t *tmr = &phy_serdes->tmr;
    msleep(tmr->speed_detect_interval);
    return dsl_sfp_module_detect(phy_dev);
}
#endif

int dsl_serdes_priv_fun(phy_dev_t *phy_dev, int op_code, va_list ap)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    spd_dtc_tmr_t *tmr = &phy_serdes->tmr;
    int32_t *stmr_p, *stmr_wt_p, *ltmr_p, *ltmr_wt_p;
    int32_t stmr, stmr_wt, ltmr, ltmr_wt;
    int val;

    switch(op_code)
    {
        case SERDES_OP_TMR_GET:
            stmr_p = va_arg(ap, int32_t *);
            stmr_wt_p = va_arg(ap, int32_t *);
            ltmr_p = va_arg(ap, int32_t *);
            ltmr_wt_p = va_arg(ap, int32_t *);
            *stmr_p = tmr->short_value_ms;
            *stmr_wt_p = tmr->short_timer_weight;
            *ltmr_p = tmr->long_value_ms;
            *ltmr_wt_p = tmr->long_timer_weight;
            break;
        case SERDES_OP_TMR_SET:
            stmr = va_arg(ap, int32_t);
            stmr_wt = va_arg(ap, int32_t);
            ltmr = va_arg(ap, int32_t);
            ltmr_wt = va_arg(ap, int32_t);
            val = va_arg(ap, int);
            if (val)
                dsl_serdes_poll_timer_op(phy_dev, SPD_TMR_RESET);
            else
                dsl_serdes_poll_timer_config(phy_dev, stmr, stmr_wt, ltmr, ltmr_wt);
            break;
        default:
            return -1;
    }

    return 0;
}

static int dsl_speed_mpbs_to_txfir_idx(int speed_mbps)
{
    int speeds[] = {PHY_TXFIR_SPEED_MBPS};
    int idx;

    for (idx=0; idx<ARRAY_SIZE(speeds); idx++)
        if (speed_mbps == speeds[idx])
            return idx;
    return -1;
}

int dsl_txfir_get(phy_dev_t *phy_dev, phy_txfir_t *txfir)
{
    int idx;
    int rc = 0;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (txfir->speed_mbps == 0) { /* 0 means to read current link speed */
        if (!phy_dev->link)
            return -1;
        else
            txfir->speed_mbps = phy_dev->speed;
    }
    idx = dsl_speed_mpbs_to_txfir_idx(txfir->speed_mbps);
    if (idx == -1)
        return -1;

    txfir->pre = phy_dev->txfir[idx].pre;
    txfir->main = phy_dev->txfir[idx].main;
    txfir->post1 = phy_dev->txfir[idx].post1;
    txfir->post2 = phy_dev->txfir[idx].post2;
    txfir->hpf = phy_dev->txfir[idx].hpf;

    /* If link up and read for current speed or link is down, read back register too */
    if (phy_dev->txfir_reg_speed_idx == idx && phy_serdes->cur_power_level == SERDES_POWER_UP)
        rc += dsl_read_txfir_reg(phy_dev, txfir);
    else
        txfir->pre_reg = txfir->main_reg = txfir->post1_reg = txfir->post2_reg = txfir->hpf_reg = 0;

    return rc;
}


int dsl_txfir_set(phy_dev_t *phy_dev, phy_txfir_t *cfg_txfir)
{
    int rc = 0;
    int idx, idx1, idx2;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_txfir_t *txfir;
    int hpf = cfg_txfir->hpf;

    if (cfg_txfir->speed_mbps == -1)
    {
        idx1 = 0;
        idx2 = 4;
    }
    else
    {
        if (cfg_txfir->speed_mbps == 0)
        {
            if (!phy_dev->link)
                return -2;
            cfg_txfir->speed_mbps = phy_dev->speed;
        }
        idx1 = dsl_speed_mpbs_to_txfir_idx(cfg_txfir->speed_mbps);
        idx2 = idx1 + 1;
    }

    for (idx = idx1; idx < idx2; idx++)
    {
        txfir = &phy_dev->txfir[idx];

        if (hpf == -1)
        {
            if (txfir->speed_mbps > 1000)
                cfg_txfir->hpf = 3;
            else
                cfg_txfir->hpf = 0;
        }
        else
            cfg_txfir->hpf = cfg_txfir->hpf;


        if (phy_dsl_txfir_is_blank(cfg_txfir))
            dsl_set_txfir_default(txfir);
        else
        {
            if ((rc = phy_dsl_verify_txfir(phy_dev, cfg_txfir)))
                return rc;

            txfir->pre = cfg_txfir->pre;
            txfir->main = cfg_txfir->main;
            txfir->post1 = cfg_txfir->post1;
            txfir->post2 = cfg_txfir->post2;
            txfir->hpf = cfg_txfir->hpf;
        }

        /* Set and read hardware register if link up speed match or link is down */
        if ((phy_dev->link && phy_dev->speed == txfir->speed_mbps) ||
            (!phy_dev->link && phy_serdes->cur_power_level == SERDES_POWER_UP))
        {
            rc = phy_dev_tx_cfg_set(phy_dev, txfir->pre, txfir->main,
                    txfir->post1, txfir->post2, txfir->hpf);
            /* Read back to IOCTL buffer */
            dsl_read_txfir_reg(phy_dev, cfg_txfir);
            phy_dev->txfir_reg_speed_idx = idx;
        }
    }

    return rc;
}

int dsl_serdes_check_power(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->cur_power_level == SERDES_POWER_DOWN)
    {
        if (!phy_serdes->power_admin_on)
            return ETHCTL_ERROR_POWER_ADMIN_DOWN;
        else
            return ETHCTL_ERROR_POWER_SAVING_DOWN;
    }
    return 0;
}

/* We turn on the power to record configuration if the power is down */
int dsl_serdes_single_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int rc = 0;
    int cfg_speed_cap;
    int org_power_level;

    if (!phy_serdes || !phy_serdes->inited)
        return 0;
    org_power_level = phy_serdes->cur_power_level;

    if (speed == PHY_SPEED_AUTO && !((1<<phy_dev->current_inter_phy_type) & INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M))
    {
        speed = cascade_phy_max_adv_speed_get(phy_dev);
        if (speed == 0)
            return 0;
    }

    cfg_speed_cap = phy_speed_to_cap(speed, PHY_DUPLEX_FULL);
    if (speed != PHY_SPEED_AUTO && !(cfg_speed_cap & phy_serdes->speed_caps)) {
        printk("Not supported speed: 0x%x on PHY addr 0x%x\n", speed, phy_dev->addr);
        rc = -1;
        goto end;
    }

   if (phy_dev->cascade_next && (phy_dev->cascade_next->flag & PHY_FLAG_DYNAMIC) && (phy_serdes->sfp_module_type == SFP_NO_MODULE))
        goto end;

    if (phy_serdes->cur_power_level == SERDES_POWER_DOWN)
        dsl_powerup_serdes(phy_dev);

    phy_serdes->current_speed = speed;
    rc += phy_serdes->speed_set(phy_dev, speed, duplex);

end:
    if (org_power_level != phy_serdes->cur_power_level)
        dsl_powerdown_serdes(phy_dev);

    return rc;
}

static int dsl_serdes_cfg_txfir(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int idx;

    if (!phy_serdes->read_txfir_reg)
        return 0;

    if ((idx = dsl_speed_mpbs_to_txfir_idx(phy_dev->speed)) < 0)
        return -1;

    phy_dev_tx_cfg_set(phy_dev, phy_dev->txfir[idx].pre, phy_dev->txfir[idx].main,
            phy_dev->txfir[idx].post1, phy_dev->txfir[idx].post2, phy_dev->txfir[idx].hpf);
    phy_dev->txfir_reg_speed_idx = idx;

    return 0;
}

static void serdes_link_stats(phy_dev_t *phy_dev)
{
    int old_link = phy_dev->link;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    phy_serdes->link_stats(phy_dev);
    if (!old_link && phy_dev->link)
        dsl_serdes_cfg_txfir(phy_dev);
}

static void sfp_link_status_check(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY || (phy_next->flag & PHY_FLAG_COPPER_SFP_TYPE) ||
            !dsl_serdes_silent_start_supported(phy_dev) || phy_dev->link)
    {
        if(!phy_dev->link && !dsl_serdes_light_detected(phy_dev))
            return;

        if (!phy_dev->link)
            msleep(phy_serdes->tmr.speed_detect_interval);

        serdes_link_stats(phy_dev);
        return;
    }

    if (!dsl_serdes_silent_start_light_detected(phy_dev))
    {
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
        return;
    }

    dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);
    msleep(phy_serdes->tmr.speed_detect_interval);

    if(!phy_dev->link && !dsl_serdes_light_detected(phy_dev))
        return;

    serdes_link_stats(phy_dev);

}

/*
   The testing result shows lower speed will be easier to link up
   during the fibre insertion, thus we are doing retry of the highest
   speed when linked in non highest speed.
 */
#define PHY_CAP_SPEED_MASK ((PHY_CAP_10000 << 1) - 1)
#define IS_PHY_HIGHEST_SPEED_CAP(speed_caps, curSpeed) (!((speed_caps & (PHY_CAP_SPEED_MASK)) & (~((curSpeed<<1)-1))))

static phy_speed_t get_next_lower_speed(phy_speed_t speed, uint32_t adv_caps)
{
    int i;
    phy_speed_t spd;
    static phy_speed_t phy_speed_array[] = {
        PHY_SPEED_10, PHY_SPEED_100, PHY_SPEED_1000, PHY_SPEED_2500,
        PHY_SPEED_5000, PHY_SPEED_10000};

    for(i = ARRAY_SIZE(phy_speed_array) - 1; i >= 0; i--)
    {
        spd = phy_speed_array[i];
        if (spd < speed && (adv_caps == 0 || (phy_speed_to_cap(spd, PHY_DUPLEX_FULL) & adv_caps)))
            break;
    }

    if (i<0)
        return PHY_SPEED_UNKNOWN;

    return phy_speed_array[i];
}

static int dsl_serdes_speed_detect(phy_dev_t *phy_dev)
{
    static int retry = 0;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    u16 rnd;
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    int inter_type;
    int highest_speed_inter_phy_type = INTER_PHY_TYPE_MAX - 1;
    uint32_t inter_phy_types;
    uint32_t supported_speed_caps;
    phy_speed_t speed;

    if (PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev))
        return 1;

    if (phy_serdes->config_speed != PHY_SPEED_AUTO && !INTER_PHY_TYPE_AN_AND_FORCED_SPEED(phy_dev->current_inter_phy_type))
        return 1;

    if (retry == 0 &&  phy_serdes->sfp_status >= SFP_MODULE_IN)
    {
        sfp_link_status_check(phy_dev);
        if (phy_dev->link) goto end;
    }

    if (phy_dev->configured_current_inter_phy_type != INTER_PHY_TYPE_AUTO && phy_dev->configured_an_enable != PHY_CFG_AN_AUTO)
        return 1;

    /* Introduce random phase shift to get bigger chanse to link up on back to back connection */
    if (retry == 0)
    {
        get_random_bytes(&rnd, sizeof(rnd));
        msleep(1000 * (rnd%100) / 100);
        dsl_serdes_poll_timer_op(phy_dev, SPD_TMR_UPDATE);
    }

    /* Scan different multi speed AN Serdes type */
    if (phy_dev->configured_current_inter_phy_type != INTER_PHY_TYPE_AUTO)
    {
        inter_phy_types = (1 << phy_dev->configured_current_inter_phy_type);
        if ((inter_phy_types & phy_dev->common_inter_phy_types) == 0)
            return 1;
    }
    else
        inter_phy_types = phy_dev->configured_inter_phy_types;

    // Let's do SGMII detection in the end to avoid mis link up with 1000Base-X
    inter_phy_types &= (INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M) & (~INTER_PHY_TYPE_SGMII_M); 
    for (inter_type = INTER_PHY_TYPE_MAX - 1; inter_phy_types; inter_type--)
    {
        if ((inter_phy_types & (1<<inter_type)) == 0)
            continue;

        phy_dev->current_inter_phy_type = inter_type;
        phy_i2c->current_inter_phy_type = inter_type;
        phy_dev_speed_set(phy_i2c, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
        if (phy_dev->configured_an_enable == PHY_CFG_AN_AUTO) /* Multi speed always support AN */
            phy_dev->an_enabled = 1;
        dsl_serdes_single_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
        if (!dsl_sfp_module_detect(phy_dev))
            return 0;

        sfp_link_status_check(phy_dev);
        inter_phy_types &= ~(1<<inter_type);
        if (phy_dev->link)
            goto LinkUp;
    }

    for (speed = phy_serdes->common_highest_speed; speed >= phy_serdes->common_lowest_speed;
            speed = get_next_lower_speed(speed, phy_serdes->adv_caps))
    {
        if(!(phy_speed_to_cap(speed, PHY_DUPLEX_FULL) & phy_serdes->common_speed_caps))
            continue;

        /* We need to put SGMII mode ahead of other 1G/100M scanning to avoid mis-linkup */
        if (speed == PHY_SPEED_1000)
        {
            get_inter_phy_supported_speed_caps(INTER_PHY_TYPE_SGMII, &supported_speed_caps);
            supported_speed_caps &= ~PHY_CAP_AUTONEG;
            if ((inter_phy_types = phy_dev->configured_inter_phy_types & INTER_PHY_TYPE_SGMII_M) &&
                    (phy_serdes->adv_caps == 0 || (phy_serdes->adv_caps & supported_speed_caps)))
            {
                inter_type = INTER_PHY_TYPE_SGMII;
                phy_dev->current_inter_phy_type = inter_type;
                phy_i2c->current_inter_phy_type = inter_type;
                if (phy_dev->configured_an_enable == PHY_CFG_AN_AUTO)
                    phy_dev->an_enabled = 1;
                phy_dev_speed_set(phy_i2c, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
                dsl_serdes_single_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
                if (!dsl_sfp_module_detect(phy_dev))
                    return 0;
                sfp_link_status_check(phy_dev);
                if (phy_dev->link)
                    goto LinkUp;
            }
        }

        /* Scan different Serdes type for a certain speed */
        inter_phy_types = phy_dev->configured_inter_phy_types & phy_speed_to_inter_phy_speed_mask(speed);
        for (inter_type = INTER_PHY_TYPE_MIN; inter_phy_types; inter_phy_types &= ~(1<<inter_type), inter_type++)
        {
            if (!dsl_sfp_module_detect(phy_dev))
                return 0;

            if ((inter_phy_types & (1<<inter_type)) == 0)
                continue;

            if (highest_speed_inter_phy_type == INTER_PHY_TYPE_MAX - 1 )
                highest_speed_inter_phy_type = inter_type;

            phy_dev->current_inter_phy_type = inter_type;
            phy_i2c->current_inter_phy_type = inter_type;
            phy_dev_speed_set(phy_i2c, speed, PHY_DUPLEX_FULL);

            if (phy_dev->configured_an_enable == PHY_CFG_AN_AUTO)
            {
                if (INTER_PHY_TYPE_AN_SUPPORT(inter_type))
                {
                    phy_dev->an_enabled = 1;    /* AN has higher priority than non AN */
                    if(dsl_serdes_single_speed_set(phy_dev, speed, PHY_DUPLEX_FULL))
                        goto forced_speed;

                    sfp_link_status_check(phy_dev);
                    if (phy_dev->link)
                        goto LinkUp;
                }

forced_speed:
                phy_dev->an_enabled = 0;
                if(dsl_serdes_single_speed_set(phy_dev, speed, PHY_DUPLEX_FULL))
                    continue;

                sfp_link_status_check(phy_dev);
                if (phy_dev->link)
                    goto LinkUp;
            }
            else
            {
                if(dsl_serdes_single_speed_set(phy_dev, speed, PHY_DUPLEX_FULL))
                    continue;

                sfp_link_status_check(phy_dev);
                if (phy_dev->link)
                    goto LinkUp;
            }
        }
    }

    goto NoLinkUp;

LinkUp:
    if (phy_dev->speed == phy_serdes->common_highest_speed)
        goto end;

    if (retry)
    {
        phy_dev_status_reverse_propagate(phy_dev);
        goto end; /* If we retried already, return; */
    }

    /* Otherwise, take a sleep to let fibre settle down, then retry higher speed */
    retry++;
    phy_dev->link = 0;  /* Set link down to insure lower level driver link up process correct during retry */
    msleep(SERDES_FIBER_SETTLE_DELAY_MS);
    if(!dsl_serdes_speed_detect(phy_dev))
        return 0;
    goto end;

NoLinkUp:
    /*
       No link up here.
       Set speed to highest when in NO_POWER_SAVING_MODE until next detection
     */
    if( phy_serdes->power_mode <= SERDES_BASIC_POWER_SAVING &&
        phy_dev->configured_current_inter_phy_type == INTER_PHY_TYPE_AUTO)
    {
        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
            phy_dev->configured_inter_phy_types, phy_serdes->common_highest_speed);

        if (INTER_PHY_TYPE_IS_MULTI_SPEED_AN(phy_dev->current_inter_phy_type))
            dsl_serdes_single_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
        else
        {
            phy_dev->current_inter_phy_type = highest_speed_inter_phy_type;
            if (phy_dev->configured_an_enable == PHY_CFG_AN_AUTO)
            {
                if (INTER_PHY_TYPE_AN_SUPPORT(phy_dev->current_inter_phy_type))
                    phy_dev->an_enabled = 1;
                else
                    phy_dev->an_enabled = 0;
            }
            dsl_serdes_single_speed_set(phy_dev, phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
        }
    }
end:
    retry = 0;
    return 1;
}

int dsl_serdes_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->adv_caps != caps)
    {
        phy_serdes->adv_caps = caps & (phy_serdes->speed_caps | PHY_CAP_NON_SPEED_CAPS);
        phy_serdes->link_changed = 1;

        if (phy_serdes->sfp_module_type != SFP_FIXED_PHY && phy_serdes->cur_power_level == SERDES_POWER_UP)
        {
            dsl_powerdown_serdes(phy_dev);
            dsl_powerup_serdes(phy_dev);
        }
    }

    return 0;
}

int dsl_serdes_caps_set_lock(phy_dev_t *phy_dev, uint32_t caps)
{
    int ret;

    mutex_lock(&serdes_mutex);
    ret = dsl_serdes_caps_set(phy_dev, caps);
    mutex_unlock(&serdes_mutex);
    return ret;
}

int dsl_serdes_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    *caps = caps_type==CAPS_TYPE_SUPPORTED? phy_serdes->speed_caps: phy_serdes->adv_caps;
    if (caps_type == CAPS_TYPE_SUPPORTED)
         *caps |= PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM | PHY_CAP_REPEATER;

    return 0;
}

int dsl_serdes_cfg_speed_set_lock(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int rc = 0;
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;

    mutex_lock(&serdes_mutex);

    if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
        phy_serdes->sfp_module_type == SFP_FIXED_PHY && !IS_USXGMII_MULTI_PORTS(phy_dev) &&
        !phy_dev_is_broadcom_phy(phy_dev->cascade_next))
    {   /* Work around for some non Broadcom PHYs not sync link status between Copper side and Serdes side */
        /* Exclude external switch connection from power down operation */
        if (phy_dev->cascade_next->link == 0)
        {
            dsl_serdes_power_set(phy_dev, 0);
            phy_dev->link = 0;
            goto ret;
        }
        else
            dsl_serdes_power_set(phy_dev, 1);
    }

    rc = dsl_serdes_cfg_speed_set(phy_dev, speed, duplex);
ret:
    mutex_unlock(&serdes_mutex);
    return rc;
}

static int dsl_serdes_cfg_single_speed_mode_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex,
        inter_phy_types_dir_t dir, int inter_type, int cfg_an_enable)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *copper_phy = phy_dev->cascade_next;
    int rc = 0;
    int i;
    int org_inter_phy_type = phy_dev->current_inter_phy_type;

    if (inter_type != INTER_PHY_TYPE_AUTO && ((1<<inter_type) & phy_dev->inter_phy_types) == 0)
        return -1;

    if (speed != PHY_SPEED_AUTO && (phy_speed_to_cap(speed, PHY_DUPLEX_FULL) & phy_serdes->speed_caps) == 0)
        return -1;

    if (cfg_an_enable !=  PHY_CFG_AN_AUTO)
    {
        if ((cfg_an_enable == PHY_CFG_AN_ON && INTER_PHY_TYPE_FORCED_SPEED_ONLY(inter_type)) ||
            (cfg_an_enable == PHY_CFG_AN_OFF && INTER_PHY_TYPE_AN_ONLY(inter_type)))
            return -1;

        phy_dev->an_enabled = cfg_an_enable == PHY_CFG_AN_ON? 1: 0;
    }

    /* If Serdes connects to external PHY and speed is AUTO, then set speed to external PHY speed,
        to avoid invalid AUTO configuration during the link up and no link down happened in external PHY */
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY && speed == PHY_SPEED_AUTO )
        speed = copper_phy->speed;

    phy_serdes->config_speed = speed;
    cascade_phy_set_common_inter_types(phy_dev);
    phy_dev_configured_current_inter_phy_type_set(phy_dev, inter_type);
    set_common_speed_range(phy_dev);
    phy_dev->configured_an_enable = cfg_an_enable;

    if (inter_type == INTER_PHY_TYPE_AUTO)
        phy_dev->current_inter_phy_type = phy_get_best_inter_phy_configure_type(phy_dev,
                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);
    else
    {
        if (((1<<inter_type) & INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M) == 0)
            speed = phy_type_to_single_speed(inter_type);
        phy_dev_current_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_DOWN, inter_type);
    }

    if (cfg_an_enable == PHY_CFG_AN_AUTO && phy_dev->current_inter_phy_type != INTER_PHY_TYPE_AUTO)
    {
        if (INTER_PHY_TYPE_AN_SUPPORT(phy_dev->current_inter_phy_type))
            phy_dev->an_enabled = 1;
        else
            phy_dev->an_enabled = 0;
    }

    if (!phy_serdes->power_admin_on)
        return 0;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
    {
        dsl_powerup_serdes(phy_dev);

        if (phy_dev->configured_an_enable == PHY_CFG_AN_AUTO)
        {
            if (((1 << phy_dev->current_inter_phy_type) & INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M) &&
                    phy_dev->current_inter_phy_type != INTER_PHY_TYPE_SGMII)
                phy_dev->an_enabled = 1;
            else
                phy_dev->an_enabled = 0;
        }

        rc = dsl_serdes_single_speed_set(phy_dev, speed, duplex);

        /* Restart cascaded PHY AN to trigger XFI link updated */
        if (phy_dev->configured_current_inter_phy_type != INTER_PHY_TYPE_AUTO && copper_phy)
        {
            phy_dev_an_restart(copper_phy);
            msleep(2000);
        }

        if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev))
        {
            if (IS_USER_CONFIG())
                phy_serdes->link_changed = 1;
            else if(!copper_phy->link)
            {
                phy_dev->link = 0;
                if( phy_serdes->power_mode == SERDES_ADVANCED_POWER_SAVING)
                    dsl_powerdown_serdes(phy_dev);
            }
        }

    }
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    else    /* SFP Module */
    {
        if (inter_type == INTER_PHY_TYPE_AUTO)
            phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                    phy_dev->configured_inter_phy_types, phy_serdes->config_speed);

        if (IS_USER_CONFIG())
        /* Force it down for hardware not link down long enough for software to detect */
            phy_serdes->link_changed = 1;

        /* When calling from user configuration. I2C PHY does not know it needs to set current_inter_phy_type
            because it can't distiguish speed scanning call from user configuration call. Only Serdes is aware
            this difference, so Serdes needs to set I2C PHY's current_inter_phy_type here in user configuration case */
        if (phy_dev->current_inter_phy_type != INTER_PHY_TYPE_UNKNOWN)  /* Otherwise need speed scanning */
        {
            phy_dev_current_inter_phy_types_set(copper_phy, INTER_PHY_TYPE_DOWN, phy_dev->current_inter_phy_type);
            rc = dsl_serdes_single_speed_set(phy_dev, speed, duplex);
            rc += phy_dev_speed_set(copper_phy, speed, duplex);
        }
    }
#endif

    /* Restart Copper PHY to trigger XFI AN handshaking for Master-Slave type XFI */
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY &&
    	!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
         INTER_PHY_TYPE_IS_MULTI_SPEED_AN(phy_dev->current_inter_phy_type) &&
            org_inter_phy_type != phy_dev->current_inter_phy_type)
    {
        phy_dev_an_restart(copper_phy);
        msleep(100);
        phy_dev_read_status(copper_phy);
        for (i=0; i<20 && !copper_phy->link; i++)
        {
            phy_dev_read_status(copper_phy);
            msleep(100);
        }
    }
    return rc;
}

int dsl_serdes_cfg_speed_mode_set_lock(phy_dev_t *phy_dev, int adv_phy_caps, phy_duplex_t duplex,
        inter_phy_types_dir_t dir, int inter_type, int cfg_an_enable)
{
    int rc;
    phy_speed_t speed;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int org_adv_caps = phy_serdes->adv_caps;
    int org_inter_type = phy_dev->current_inter_phy_type;
    mutex_lock(&serdes_mutex);

    if (inter_type != INTER_PHY_TYPE_AUTO)
        get_inter_phy_supported_speed_caps((1<<inter_type), &adv_phy_caps);
        
    if (phy_serdes->adv_caps != adv_phy_caps)
    {
        phy_serdes->adv_caps = adv_phy_caps;
        set_common_speed_range(phy_dev);
        phy_serdes->link_changed = 1;
    }

    /* If it is not single speed, set to AUTO */
    speed = phy_caps_to_auto_speed(adv_phy_caps);
    rc = dsl_serdes_cfg_single_speed_mode_set(phy_dev, speed, duplex, dir, inter_type, cfg_an_enable);
    if (rc)
    {
        phy_serdes->adv_caps = org_adv_caps;
        phy_dev_current_inter_phy_types_set(phy_dev, INTER_PHY_TYPE_DOWN, org_inter_type);
        phy_serdes->link_changed = 0;
    }
    mutex_unlock(&serdes_mutex);
    return rc;
}
EXPORT_SYMBOL(dsl_serdes_cfg_speed_mode_set_lock);

int dsl_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    return dsl_serdes_cfg_single_speed_mode_set(phy_dev, speed, duplex, INTER_PHY_TYPE_DOWN,
        phy_dev->configured_current_inter_phy_type, phy_dev->configured_an_enable);
}
EXPORT_SYMBOL(dsl_serdes_cfg_speed_set);

void phy_drv_sfp_group_list(void)
{
    phy_serdes_t *phy_serdes;
    char *serdes_type;
    char *serdes_status;
    int i, sfp_serdeses = 0;
    phy_dev_t *phy_dev;

    for (i = 0; i < MAX_SERDES_NUMBER; i++) {
        if (!serdes[i]) break;;

        phy_serdes = serdes[i];
        phy_dev = phy_serdes->phy_dev;
        if (!(phy_dev->cascade_next->flag & PHY_FLAG_DYNAMIC)) continue;

        sfp_serdeses++;
        switch(phy_serdes->highest_speed_cap) {
            case PHY_CAP_10000:
                serdes_type = "10G AE Serdes";
                break;
            case PHY_CAP_2500:
                serdes_type = "2.5G Serdes";
                break;
            case PHY_CAP_1000_FULL:
                serdes_type = "1G Serdes";
                break;
            default:
                serdes_type = "unknown";
        }

        switch(phy_serdes->sfp_status) {
            case SFP_MODULE_OUT:
                serdes_status = "unplug";
                break;
            case SFP_MODULE_IN:
                serdes_status = "plug in";
                break;
            case SFP_LINK_UP:
                serdes_status = "link up";
                break;
            default:
                serdes_status = "unkown";
                break;
        }

        if (sfp_serdeses == 1)
            printk("%-16s %-16s %-16s\n", "Serdes ID", "Serdes Type", "SFP Status");
        if (phy_serdes->phy_type == PHY_TYPE_158CLASS_SERDES)
            printk("%-16s %-16s %-16s\n", "N/A", serdes_type, serdes_status);
        else
            printk("%-16d %-16s %-16s\n", phy_serdes->phy_dev->addr, serdes_type, serdes_status);
    }

    if (sfp_serdeses == 0)
        printk(" No SFP design in this board\n");
}

static int dsl_sfp_module_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int rc = 0;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;

#if defined(CONFIG_I2C)
    rc = phy_i2c_module_detect(phy_dev);
#endif
    return rc;
}

/*
   Module detection is not going through SGMII,
   so it can be done even under SGMII power down.
 */
static int dsl_sfp_module_detect(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    TRX_TYPE trx_type = TRX_TYPE_ETHERNET;
    phy_speed_t max_spd;
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    struct device *dev;

    /* Don't do module detection for fixed connection desgin */
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;

    if (dsl_sfp_module_detected(phy_dev) == 0)
    {
        if (phy_i2c->flag & PHY_FLAG_NOT_PRESENTED)
            return 0;

        phy_serdes->sfp_module_type = SFP_NO_MODULE;
        phy_i2c->flag |= PHY_FLAG_NOT_PRESENTED;
        printk("SFP Module at Address %d Core %d is Unplugged\n",
                phy_dev->addr, phy_serdes->core_num);
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
        phy_dev->link = 0;
        phy_i2c->name[0] = 0;

        return 0;
    }

    if (phy_serdes->sfp_module_type != SFP_NO_MODULE)   /* Module detection done and no unplug */
        return 1;

#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    msleep(300);    /* Let I2C driver prepare data */

    if (trx_get_type(phy_i2c->addr, &trx_type) == OPTICALDET_NOSFP)  /* I2C driver not sync with us yet */
        return 0;
    if (trx_type != TRX_TYPE_XPON)
        phy_i2c_module_type_detect(phy_dev);
#endif

    if (trx_type == TRX_TYPE_XPON) {
        phy_serdes->sfp_module_type = SFP_GPON_MODULE;
        /* turn off SFP tx power for PON SFP to avoid rogue condition */
        dev = trxbus_get_dev(phy_i2c->addr);
        if (dev)
            sfp_mon_write(dev, bcmsfp_mon_tx_enable, 0, 0);
    }
    else
        phy_serdes->sfp_module_type = SFP_AE_MODULE;

    phy_i2c->flag &= ~PHY_FLAG_NOT_PRESENTED;
    max_spd = phy_max_speed_get(phy_i2c);

    if (phy_serdes->sfp_module_type == SFP_GPON_MODULE)
        printk("GPON Module ");
    else if (phy_i2c->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE) // SGMII SFP_COPPER
        printk("%s SGMII Copper SFP Module ", phy_dev_speed_to_str(max_spd));
    else if (phy_i2c->flag & PHY_FLAG_COPPER_SFP_TYPE) //SFP_COPPER
        printk("%s Copper SFP Module ", phy_dev_speed_to_str(max_spd));
    else
        printk("SFP Module ");
    printk(KERN_CONT "is Plugged in at Serdes address %d core %d lane %d\n", phy_dev->addr,
        phy_dev->core_index, phy_dev->lane_index);

    return 1;
}

int phy_dsl_serdes_read_status(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;
    phy_speed_t org_speed = phy_dev->speed;
    phy_dev_t *copper_phy = phy_dev->cascade_next;
    int org_link = phy_dev->link;


    if (!phy_serdes || !phy_serdes->inited)
        goto read_end;

    if (PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev))
    {
        dsl_serdes_speed_detect(phy_dev);
        serdes_link_stats(phy_dev);
        goto read_end;
    }

    if (phy_serdes->link_changed)   /* Force link temporary down to notify upper layer of event */
    {
        phy_dev->link = 0;
        phy_serdes->link_changed = 0;
        if (phy_serdes->sfp_status == SFP_LINK_UP)
        {
            phy_serdes->sfp_status = SFP_MODULE_IN;
            /* The routine below will turn on power to complete record configuration,
                so it is safe to call and has to be called even in power down */
            if (phy_serdes->config_speed == PHY_SPEED_AUTO)
                dsl_serdes_single_speed_set(phy_dev, phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
            else
                dsl_serdes_single_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
        }
        if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
            goto read_end;
        else
            goto sfp_end;
    }

    if (!phy_serdes->power_admin_on)
    {
        phy_dev->link = 0;
        goto read_end;
    }

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
    {
        if (dsl_serdes_check_power(phy_dev))
            goto read_end;

        serdes_link_stats(phy_dev);
        if (copper_phy->link)
        {
            if (!phy_dev->link && !(phy_serdes->flag & LINK_WARNING))
            {
                printkwarn("Warning: Serdes at %d link does not go up following external copper PHY at %d.",
                        phy_dev->addr, copper_phy->addr);
                phy_serdes->flag |= LINK_WARNING;
            }
        }
        else
            phy_serdes->flag &= ~LINK_WARNING;
        goto read_end;
    }


    switch (phy_serdes->sfp_status)
    {
        case SFP_MODULE_OUT:
sfp_module_out:
            if(phy_serdes->sfp_status == SFP_MODULE_OUT && dsl_sfp_module_detect(phy_dev))
                goto sfp_module_in;

            if ( phy_serdes->sfp_status == SFP_MODULE_IN)
            {
                cascade_phy_set_common_inter_types(phy_dev);
                phy_dsl_set_configured_types(phy_dev);
                set_common_speed_range(phy_dev);
                dsl_serdes_poll_timer_op(phy_dev, SPD_TMR_RESTART);
            }

            phy_serdes->sfp_status = SFP_MODULE_OUT;
            goto sfp_end;

        case SFP_MODULE_IN:
sfp_module_in:
            if(phy_serdes->sfp_status >= SFP_MODULE_IN && !dsl_sfp_module_detect(phy_dev))
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;
                goto sfp_module_out;
            }

            if(phy_serdes->sfp_module_type != SFP_AE_MODULE)    /* PON module */
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;
                goto sfp_end;
            }

            dsl_powerup_serdes(phy_dev);

            if (dsl_serdes_silent_start_light_detected(phy_dev))
                dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);

            if(phy_serdes->sfp_status < SFP_MODULE_IN)
            {
                cascade_phy_set_common_inter_types(phy_dev);
                phy_dsl_set_configured_types(phy_dev);
                set_common_speed_range(phy_dev);

                if (phy_dev->configured_current_inter_phy_type == INTER_PHY_TYPE_AUTO)
                    phy_dev->current_inter_phy_type = phy_next->current_inter_phy_type =
                        sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);
                else
                    phy_dev->current_inter_phy_type = phy_next->current_inter_phy_type =
                        phy_dev->configured_current_inter_phy_type;

                phy_dev_speed_set(phy_next, phy_serdes->config_speed, PHY_DUPLEX_FULL);
                dsl_serdes_single_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
            }

            if(phy_serdes->sfp_status <= SFP_MODULE_IN)
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;

                if (dsl_serdes_silent_start_light_detected(phy_dev))
                    dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);

                /*
                    Even silent start does not detect valid signal ans laser might be off,
                    we still need to do speed detection because SS is speed dependant and
                    work in single direction.
                */
                if (!dsl_serdes_speed_detect(phy_dev))
                {
                    phy_serdes->sfp_status = SFP_MODULE_IN;
                    goto sfp_module_out;
                }

                sfp_link_status_check(phy_dev);
                if (phy_dev->link)
                    goto sfp_link_up;
            }
            phy_serdes->sfp_status = SFP_MODULE_IN;
            goto sfp_end;

        case SFP_LINK_UP:
sfp_link_up:
            if(phy_serdes->sfp_status == SFP_LINK_UP)
            {
                if(!dsl_sfp_module_detect(phy_dev))
                {
                    phy_dev->link = 0;
                    goto sfp_module_out;
                }
                sfp_link_status_check(phy_dev);
                if (org_link && phy_dev->link && org_speed != phy_dev->speed)
                    phy_dev->link = 0;

                if(!phy_dev->link)
                {
                    /* If link goes down, we must put the link to the highest.
                        because link detection assume the phy is in highest speed when
                        link is down and will check link before doing different speed config */
                    if(phy_serdes->config_speed == PHY_SPEED_AUTO &&
                        phy_dev->configured_current_inter_phy_type == INTER_PHY_TYPE_AUTO)
                    {
                        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                                phy_dev->configured_inter_phy_types, phy_serdes->common_highest_speed);
                        if (phy_dev->configured_an_enable == PHY_CFG_AN_AUTO &&
                            INTER_PHY_TYPE_AN_SUPPORT(phy_dev->current_inter_phy_type))
                            phy_dev->an_enabled = 1;
                        else
                            phy_dev->an_enabled = 0;
                        if (phy_dev->current_inter_phy_type != INTER_PHY_TYPE_UNKNOWN)
                            dsl_serdes_single_speed_set(phy_dev, phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
                    }
                    goto sfp_module_in;
                }
            }
            else
                phy_dev_status_reverse_propagate(phy_dev);

            phy_serdes->sfp_status = SFP_LINK_UP;
            goto sfp_end;
    }

sfp_end:
    if ((phy_serdes->power_mode == SERDES_BASIC_POWER_SAVING && phy_serdes->sfp_status == SFP_MODULE_OUT) ||
        (phy_serdes->power_mode == SERDES_ADVANCED_POWER_SAVING && phy_serdes->sfp_status != SFP_LINK_UP))
        dsl_powerdown_serdes(phy_dev);

read_end:
    return 0;
}

int phy_dsl_serdes_dt_priv(dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv)
{
    *_priv = (void *)handle;

    return 0;
}

int phy_dsl_serdes_dev_add_lock(phy_dev_t *phy_dev)
{
    mutex_lock(&serdes_mutex);
    mutex_unlock(&serdes_mutex);
    return 0;
}

int phy_dsl_serdes_read_status_lock(phy_dev_t *phy_dev)
{
    int rc;

    mutex_lock(&serdes_mutex);

    rc = phy_dsl_serdes_read_status(phy_dev);

    mutex_unlock(&serdes_mutex);
    return rc;
}

int phy_dev_sltstt_get(phy_dev_t *phy_dev, int *mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (!dsl_serdes_silent_start_supported(phy_dev))
        return -1;
    *mode = phy_serdes->silent_start_enabled;
    return 0;
}
EXPORT_SYMBOL(phy_dev_sltstt_get);

int phy_dev_sltstt_set(phy_dev_t *phy_dev, int mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (!dsl_serdes_silent_start_supported(phy_dev))
        return -1;
    phy_serdes->silent_start_enabled = !!mode;

    if (phy_serdes->silent_start_enabled)
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
    else
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);
    return 0;
}
EXPORT_SYMBOL(phy_dev_sltstt_set);

int dsl_serdes_power_mode_set(phy_dev_t *phy_dev, int mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->power_mode == mode)
        return 0;

    phy_serdes->power_mode = mode;

    switch (mode)
    {
        case SERDES_NO_POWER_SAVING:
            phy_dsl_serdes_power_set(phy_dev, 1);
            break;
        case SERDES_BASIC_POWER_SAVING:
        case SERDES_ADVANCED_POWER_SAVING:
            phy_dsl_serdes_power_set(phy_dev, 1);
            dsl_powerdown_serdes(phy_dev);
            break;
        case SERDES_FORCE_OFF:
            phy_dsl_serdes_power_set(phy_dev, 0);
            break;
    }

    return 0;
}

int dsl_serdes_apd_get(phy_dev_t *phy_dev, int *enable)
{
    dsl_serdes_power_mode_get(phy_dev, enable);
    return 0;
}

int dsl_serdes_apd_set_lock(phy_dev_t *phy_dev, int enable)
{
    mutex_lock(&serdes_mutex);

    dsl_serdes_power_mode_set(phy_dev, enable);

    mutex_unlock(&serdes_mutex);
    return 0;
}

int dsl_serdes_power_mode_get(phy_dev_t *phy_dev, int *mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (!phy_serdes || !phy_serdes->inited)
        return -1;

   *mode = phy_serdes->power_mode;
    return 0;
}

int dsl_serdes_speed_get_lock(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (!phy_serdes || !phy_serdes->inited)
        return -1;

    mutex_lock(&serdes_mutex);
    *speed = phy_serdes->config_speed;
    *duplex = PHY_DUPLEX_FULL;
    mutex_unlock(&serdes_mutex);
    return 0;
}

int dsl_serdes_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types)
{
    phy_serdes_t *serdes = phy_dev->priv;

    *types = serdes->inter_phy_types;
    return 0;
}

