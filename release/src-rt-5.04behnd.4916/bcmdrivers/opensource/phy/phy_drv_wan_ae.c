/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: October 2021
 *      Author: ido.brezel@broadcom.com
 */

/*
 * WAN AE MPCS PHY driver
 */

#include <linux/bcm_log.h>
#include <linux/delay.h>
#include "mpcs.h"
#include "phy_drv.h"
#include "wan_drv.h"
#include "trxbus.h"
#include "bcmsfp.h"
#include "phy_drv_psp.h"

#if defined(CONFIG_BRCM_SMC_BOOT)
#include <bp3_license.h>
#include <pmc_wan.h>
#endif

#define wanae_err(fmt, ...) printk("ERROR: wan_ae:" fmt, ##__VA_ARGS__)
#define DEBUG
#ifdef DEBUG
#define wanae_dbg(fmt, ...) BCM_LOG_INFO(BCM_LOG_ID_GPON_SERDES, "WAN_AE: "fmt, ##__VA_ARGS__)
#else
#define wanae_dbg(fmt, ...)
#endif

static int trxbus_i2cbus = -1;
static phy_speed_t saved_serdes_speed = PHY_SPEED_UNKNOWN;
static int silent_start_enabled;

static int wan_serdes_reset_txfifo(void)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_WAN_SERDES_RESET_TXFIFO);
    return cb ? cb(NULL) : -1;
}

static int wan_serdes_sync_loss(void)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_WAN_SERDES_SYNC_LOSS);
    return cb ? cb(NULL) : -1;
}

static int wan_serdes_conf(serdes_wan_type_t wan_type)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_WAN_SERDES_CONFIG);
    return cb ? cb(&wan_type) : -1;
}

static void sfp_tx_power_en(int enable)
{
    struct device *dev;

    if (trxbus_i2cbus < 0)
        return;

    dev = trxbus_get_dev(trxbus_i2cbus);
    if (dev)
        sfp_mon_write(dev, bcmsfp_mon_tx_enable, 0, enable);
}

/* PCS access will fail if serdes is not configured (when link is down) */
static int is_link(void)
{
    int state = is_pcs_state();
    int is_locked = is_pcs_locked();

    if (state || !is_locked)
        wanae_err("Failed reading expected link status: state %x, locked %x\n", state, is_locked);

    return !state && is_locked;
}

#if defined(CONFIG_BRCM_SMC_BOOT)
static int wan_ae_license_check(phy_speed_t speed)
{
    bp3_license_feature_t bp3_feature = BP3_FEATURE_MAX;

    if (speed == PHY_SPEED_10000)
        bp3_feature = BP3_FEATURE_ACTIVE_ETH_10;
    else if (speed == PHY_SPEED_5000)
        bp3_feature = BP3_FEATURE_ACTIVE_ETH_5;
    else if (speed == PHY_SPEED_2500)
        bp3_feature = BP3_FEATURE_ACTIVE_ETH_2_5;
    else if (speed == PHY_SPEED_1000)
        bp3_feature = BP3_FEATURE_ACTIVE_ETH_2_5;

    return bcm_license_check_msg(bp3_feature);
}
#endif

static int init_mpcs(phy_speed_t speed)
{
    struct mpcs_cfg_s mpcs_cfg = {};

    if (speed == PHY_SPEED_10000)
        mpcs_cfg.cfg_port_speed = eSPEED_10_10;
    else if (speed == PHY_SPEED_5000)
    {
        mpcs_cfg.cfg_port_speed = eSPEED_5_5;
        mpcs_cfg.cfg_5g5g_mode = 1;
    }
    else if (speed == PHY_SPEED_2500)
    {
        mpcs_cfg.cfg_port_speed = eSPEED_2_2;
        mpcs_cfg.cfg_2p5g2p5g_mode = 1;
        mpcs_cfg.cfg_2p5g_is_xgmii = 1;
    }
    else if (speed == PHY_SPEED_1000)
        mpcs_cfg.cfg_port_speed = eSPEED_1_1;
    else
    {
        wanae_err("init_mpcs: unsupported mode (%d)\n", speed);
        return -1;
    }

    mpcs_init(&mpcs_cfg);

    return 0;
}

static int _phy_wan_serdes_config(phy_speed_t speed)
{
    serdes_wan_type_t wan_type = SERDES_WAN_TYPE_NONE;
    int state;

    if (speed == PHY_SPEED_UNKNOWN)
        return 0;

    if (speed == PHY_SPEED_10000)
        wan_type = SERDES_WAN_TYPE_AE_10G;
    else if (speed == PHY_SPEED_5000)
        wan_type = SERDES_WAN_TYPE_AE_5G;
    else if (speed == PHY_SPEED_2500)
        wan_type = SERDES_WAN_TYPE_AE_2_5G;
    else if (speed == PHY_SPEED_1000)
        wan_type = SERDES_WAN_TYPE_AE;

    if (wan_type == SERDES_WAN_TYPE_NONE)
    {
        wanae_err("Unsupported speed mode: %d\n", speed);
        return -1;
    }

#if defined(CONFIG_BRCM_SMC_BOOT)
    if (wan_ae_license_check(speed) <= 0)
        return -1;
#endif

    if (wan_serdes_conf(wan_type))
        return -1;

    wan_serdes_sync_loss();

    if (init_mpcs(speed))
    {
        wanae_err("Failed to initialize MPCS\n");
        return -1;
    }

    /* Handled by pon_drv */
    /* SGB_TOP_16NM_SGB_TOP_AE_RX_FIFO_CTL */
    /* SGB_TOP_16NM_SGB_TOP_AE_TX_FIFO_CTL */
    /* SGB_TOP_16NM_SGB_TOP_AE_EN */
    wan_serdes_reset_txfifo();

    state = is_pcs_state(); /* Clear reg */
#define PCS_STATE_AFTER_SYNC 0x3c0
    if ((state & 0x3ff) != PCS_STATE_AFTER_SYNC)
        wanae_dbg("clearing state; expected 0x%x, got 0x%x\n", PCS_STATE_AFTER_SYNC, state);

    return 0;
}

static int is_signal_detect_change(int signal_detect)
{
    static int prev_signal_detect;
    int is_change = signal_detect != prev_signal_detect;

    if (trxbus_i2cbus < 0)
        is_change = 0;

    wanae_dbg("sigdet is_change %d signal_detect %d prev %d\n", is_change, signal_detect, prev_signal_detect);
    prev_signal_detect = signal_detect;

    return is_change;
}

static void laser_state_set(int link)
{
    trxbus_transmitter_control(trxbus_i2cbus, link ? laser_on : laser_off);
}

static int trx_detect(int bus, void *opticaldet_desc)
{
    is_signal_detect_change(0);
    if (!phy_drv_psp_silent_start_enable())
        laser_state_set(1);

    return 0;
}

static int _trx_init(phy_dev_t *phy_dev)
{
    trxbus_i2cbus = trxbus_dt_bus_get(phy_dev->dt_handle);
    if (trxbus_i2cbus < 0)
    {
        wanae_err("Missing SFP attribute 'trx' in DT\n");
        return 0;
    }

    trxbus_register_mac(trxbus_i2cbus, 1, trx_detect, NULL);

    if (phy_drv_psp_silent_start_enable())
        silent_start_enabled = 1;

    return 0;
}

static int _phy_init(phy_dev_t *phy_dev)
{
#if defined(CONFIG_BRCM_SMC_BOOT)
    if (!phy_dev->cascade_next && pmc_wan_interface_power_control(WAN_INTF_AE, 1))
        return -1;
#endif

   /* Has external PHY ? */
    if (!phy_dev->cascade_next && _trx_init(phy_dev))
        return -1;

#if defined(DSL_DEVICES)
    phy_dev->flag |= PHY_FLAG_FORCE_2P5G_XGMII;
#endif

    return 0;
}

static int sfp_sd_get(void)
{
    long los = 1;
    struct device *dev;

    if (trxbus_i2cbus < 0)
        return 1;

    dev = trxbus_get_dev(trxbus_i2cbus);
    if (dev)
        sfp_mon_read(dev, bcmsfp_mon_los, 0, &los);

    return !los; /* "not" is signal detect */
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    if (speed < PHY_SPEED_1000)
    {
#if defined(CONFIG_BRCM_SMC_BOOT)
        if (phy_dev->cascade_next)
        {
            pmc_wan_interface_power_control(WAN_INTF_AE, 0);
            wan_serdes_conf(SERDES_WAN_TYPE_NONE);
        }
#endif
        return -1;
    }
    else
    {
#if defined(CONFIG_BRCM_SMC_BOOT)
        if (phy_dev->cascade_next)
        {
            pmc_wan_interface_power_control(WAN_INTF_AE, 1);
        }
#endif
    }

    saved_serdes_speed = speed;

    /* Skip serdes configuration if no EXTPHY or no SIGDET from SFP */
    /* checking trxbus_i2cbus, which indicates phy_init was called, as we don't want to perform */
    /* serdes init if it wasn't init */
    if (phy_dev->cascade_next)
    {
        _phy_wan_serdes_config(saved_serdes_speed);
        /* We could wait for polling to update this, but why wait? */

        msleep(1); /* Short delay for mpcs to sync with phy after wan_serdes_config */

        phy_dev->link = is_link();
        if (phy_dev->link)
        {
            phy_dev->duplex = PHY_DUPLEX_FULL;
            phy_dev->speed = speed;
        }

        return 0;
    }

    _phy_wan_serdes_config(saved_serdes_speed);
    /* forces static prev_signal_detect to be no-link, so that when we poll status, serdes will
     * be initialized */
    is_signal_detect_change(0);
    return 0;
}

static int configure_on_sigdet_change(void)
{
    int signal_detect = sfp_sd_get();
    int is_change = is_signal_detect_change(signal_detect);
    int state = -1, lock = -1;

    if (!signal_detect)
        return 0;

    /* - signal, detect changed to on
     * - signal, but no lock
     * - signal, lock, but statemachine change */
    if (is_change || !(lock = is_pcs_locked()) || (state = is_pcs_state()))
    {
        wanae_dbg("poll status: is_change %d, lock %d state 0x%x\n", is_change, lock, state);
        wan_serdes_sync_loss();
        msleep(1);
        mpcs_rx_reset();
        msleep(100);

        state = is_pcs_state(); /* Clear reg */
#define PCS_STATE_AFTER_SYNC 0x3c0
        if ((state & 0x3ff) != PCS_STATE_AFTER_SYNC)
            wanae_dbg("clearing2 state; expected 0x%x, got 0x%x\n", PCS_STATE_AFTER_SYNC, state);

        msleep(300);
        state = is_pcs_state(); /* Clear reg */
        if (state)
            wanae_dbg("clearing3 state; expected 0x0, got 0x%x\n", state);

        return 0; /* Link down so that next poll phy_dev->link would change and cause a MAC config */
    }

    return 1;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    phy_speed_t speed = PHY_SPEED_UNKNOWN;
    int old_link = phy_dev->link;

    if (!(phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED))
       goto exit_dn;

    if (phy_dev->cascade_next)
        phy_dev->link = phy_dev->cascade_next->link ? is_link(): 0;
    else if (configure_on_sigdet_change())
        phy_dev->link = is_link();
    else
        phy_dev->link = 0;

    /* No need to check if it is external phy (phy_dev->cascade_next == NULL), no body will 
            config the silent-start when it is external phy port */
    if (silent_start_enabled && 
        (phy_dev->link != old_link))
    {
        laser_state_set(phy_dev->link);
    }

    if (!phy_dev->link)
       goto exit_dn;

    speed = saved_serdes_speed;
    if (speed != PHY_SPEED_UNKNOWN)
    {
        phy_dev->speed = speed;
        phy_dev->duplex = PHY_DUPLEX_FULL;
        return 0;
    }

exit_dn:
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    return 0;
}

static int _phy_caps_get(phy_dev_t *phy_dev, int caps_type,  uint32_t *pcaps)
{
    if ((caps_type != CAPS_TYPE_ADVERTISE) && (caps_type != CAPS_TYPE_SUPPORTED))
        return 0;

    *pcaps = PHY_CAP_AUTONEG | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000;

    if (phy_dev->pause_rx && phy_dev->pause_tx)
        *pcaps |= PHY_CAP_PAUSE;
    else if (phy_dev->pause_rx)
        *pcaps |= PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM;
    else if (phy_dev->pause_tx)
        *pcaps |= PHY_CAP_PAUSE_ASYM;

    return 0;
}

static int _phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    phy_speed_t speed = PHY_SPEED_UNKNOWN;
    phy_dev->pause_rx = 0; 
    phy_dev->pause_tx = 0; 

    if (caps & PHY_CAP_PAUSE)
    {
        phy_dev->pause_rx = 1; 
        phy_dev->pause_tx = (caps & PHY_CAP_PAUSE_ASYM) ? 0 : 1;
    }
    else if (caps & PHY_CAP_PAUSE_ASYM)
    {
        phy_dev->pause_tx = 1; 
    }

    if (caps & PHY_CAP_10000)
        speed = PHY_SPEED_10000;
    else if (caps & PHY_CAP_5000)
        speed = PHY_SPEED_5000;
    else if (caps & PHY_CAP_2500)
        speed = PHY_SPEED_2500;
    else if (caps & PHY_CAP_1000_FULL)
        speed = PHY_SPEED_1000;

    if (speed != PHY_SPEED_UNKNOWN)
        _phy_speed_set(phy_dev, speed, PHY_DUPLEX_FULL);

    return 0;
}

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    int ret = 0;

    *enable = phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED ? 1 : 0;

    return ret;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    sfp_tx_power_en(enable);

    return 0;
}

static int _phy_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    return mpcs_read(reg, val);
}

static int _phy_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    return mpcs_write(reg, val);
}

static int _phy_int_silent_start_get(phy_dev_t *phy_dev, int *mode)
{
    *mode = silent_start_enabled;
    return 0;
}

static int _phy_int_silent_start_set(phy_dev_t *phy_dev, int mode)
{
    silent_start_enabled = mode;
    if (!phy_dev->link)
        laser_state_set(mode ? 0 : 1);

    return 0;
}

phy_drv_t phy_drv_wan_ae =
{
    .phy_type = PHY_TYPE_WAN_AE,
    .read_status = _phy_read_status,
    .speed_set = _phy_speed_set,
    .caps_get = _phy_caps_get,
    .caps_set = _phy_caps_set,
    .power_get = _phy_power_get,
    .power_set = _phy_power_set,
    .read = _phy_read,
    .write = _phy_write,
    .init = _phy_init,
    .silent_start_set = _phy_int_silent_start_set,
    .silent_start_get = _phy_int_silent_start_get,
    .name = "AE",
};
