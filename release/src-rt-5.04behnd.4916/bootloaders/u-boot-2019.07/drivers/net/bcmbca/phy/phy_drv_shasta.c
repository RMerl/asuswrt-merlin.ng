// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Mar 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Merlin Shasta SerDes PHY driver
 */

#include "phy_drv.h"
#include "serdes_access.h"

#define srds_access_s phy_dev_s
#define prog_seq_u4_tbl prog_entry_ext_t

#include "common/srds_api_err_code.h"
#include "merlin16_shasta_interface.h"
#include "public/merlin16_shasta_fields_public.h"
#include "merlin_shasta/merlin16_shasta_ucode_image.h"
#include "merlin_shasta/MU4_merlin.h"
#include <linux/delay.h>

#define LANES_PER_CORE   4

static int core_enabled[SHASTA_CORES];
static serdes_mode_t lane_mode[SHASTA_CORES][LANES_PER_CORE];
static serdes_mode_t configured_mode[SHASTA_CORES][LANES_PER_CORE];

static inline int is_inter_phy_type_supported(phy_dev_t *phy_dev, int inter_phy_type)
{
    int inter_phy_types;
    phy_dev_t *last_phy_dev = cascade_phy_get_last(phy_dev);

    inter_phy_types = last_phy_dev->inter_phy_types;

    return inter_phy_types & (1 << inter_phy_type) ? 1 : 0;
}

static int _serdes_core_init(phy_dev_t *phy_dev, int enable)
{
    int ret = 0;
    uint8_t core_id = phy_dev->core_index - SHASTA_BASE_CORE;
    struct merlin16_shasta_uc_core_config_st core_conf;

    if (core_enabled[core_id] == enable)
        return 0;

    serdes_access_config(phy_dev, enable);

    if (!enable)
        goto Exit;

    ret |= wrc_merlin16_shasta_mdio_multi_prts_en(phy_dev, 1);
    ret |= wrc_merlin16_shasta_mdio_brcst_port_addr(phy_dev, phy_dev->addr);

    ret |= merlin16_shasta_uc_reset(phy_dev, 1); 
    ret |= merlin16_shasta_ucode_mdio_load(phy_dev, merlin16_shasta_ucode_image, MERLIN16_SHASTA_UCODE_IMAGE_SIZE);
    ret |= merlin16_shasta_ucode_load_verify(phy_dev, merlin16_shasta_ucode_image, MERLIN16_SHASTA_UCODE_IMAGE_SIZE);
    ret |= merlin16_shasta_uc_reset(phy_dev, 0);
    ret |= merlin16_shasta_wait_uc_active(phy_dev);
    ret |= merlin16_shasta_ucode_crc_verify(phy_dev, MERLIN16_SHASTA_UCODE_IMAGE_SIZE, MERLIN16_SHASTA_UCODE_IMAGE_CRC);
    ret |= merlin16_shasta_init_merlin16_shasta_info(phy_dev);

    ret |= phy_dev_prog_ext(phy_dev, MU4_PMD_setup_50_10p3125_VCO);

    ret |= merlin16_shasta_get_uc_core_config(phy_dev, &core_conf);
    core_conf.field.vco_rate = (10.3125 * 4.0) - 22.0; //19.25
    ret |= merlin16_shasta_INTERNAL_set_uc_core_config(phy_dev, core_conf);

    ret |= phy_dev_prog_ext(phy_dev, MU4_datapath_reset_core);
    ret |= serdes_access_wait_pll_lock(phy_dev);

Exit:
    if (!ret)
        core_enabled[core_id] = enable;

    return ret;
}

/* Lane configuration (RAM Variables) */
static int _serdes_lane_uc_cfg(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    struct merlin16_shasta_uc_lane_config_st lane_conf = {};

    if (phy_dev->cascade_next)
        lane_conf.field.media_type = MEDIA_TYPE_COPPER_CABLE;
    else
        lane_conf.field.media_type = MEDIA_TYPE_OPTICS;

    switch (serdes_mode)
    {
    case SERDES_MODE_AN_USXGMII_MASTER:
    case SERDES_MODE_AN_USXGMII_SLAVE:
        lane_conf.field.dfe_on = 1;
        break;
    case SERDES_MODE_FORCE_10G_USXGMII:
    case SERDES_MODE_FORCE_5G_USXGMII:
    case SERDES_MODE_FORCE_2P5G_USXGMII:
    case SERDES_MODE_FORCE_1G_USXGMII:
    case SERDES_MODE_FORCE_100M_USXGMII:
    case SERDES_MODE_FORCE_10G_R:
    case SERDES_MODE_FORCE_5G_R:
    case SERDES_MODE_FORCE_2P5G_R:
    case SERDES_MODE_FORCE_5G:
    case SERDES_MODE_FORCE_2P5G:
    case SERDES_MODE_FORCE_1G_R:
    case SERDES_MODE_FORCE_1G:
    case SERDES_MODE_FORCE_100M:
        break;
    default:
        return -1;
    }

    merlin16_shasta_set_uc_lane_cfg(phy_dev, lane_conf);

    return 0;
}

/* Transmitter Configuration */
static int _serdes_lane_txfir_cfg(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    int ret = 0;

    switch (serdes_mode)
    {
    case SERDES_MODE_AN_USXGMII_MASTER:
    case SERDES_MODE_AN_USXGMII_SLAVE:
    case SERDES_MODE_FORCE_10G_USXGMII:
    case SERDES_MODE_FORCE_5G_USXGMII:
    case SERDES_MODE_FORCE_2P5G_USXGMII:
    case SERDES_MODE_FORCE_1G_USXGMII:
    case SERDES_MODE_FORCE_100M_USXGMII:
    case SERDES_MODE_FORCE_10G_R:
        ret |= merlin16_shasta_apply_txfir_cfg(phy_dev, 1, 36, 1, 0);
        ret |= merlin16_shasta_config_tx_hpf(phy_dev, 3);
        break;
    case SERDES_MODE_FORCE_5G_R:
    case SERDES_MODE_FORCE_2P5G_R:
    case SERDES_MODE_FORCE_1G_R:
    case SERDES_MODE_FORCE_5G:
    case SERDES_MODE_FORCE_2P5G:
        ret |= merlin16_shasta_apply_txfir_cfg(phy_dev, 0, 60, 0, 0);
        ret |= merlin16_shasta_config_tx_hpf(phy_dev, 3);
        break;
    case SERDES_MODE_FORCE_1G:
    case SERDES_MODE_FORCE_100M:
        ret |= merlin16_shasta_apply_txfir_cfg(phy_dev, 0, 36, 0, 0);
        ret |= merlin16_shasta_config_tx_hpf(phy_dev, 0);
        break;
    default:
        return -1;
    }

    return ret;
}

/* Change lane speed */
static int _serdes_lane_speed_cfg(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    int ret = 0;

    switch (serdes_mode)
    {
    case SERDES_MODE_AN_USXGMII_MASTER:
        ret |= phy_dev_prog_ext(phy_dev, MU4_usxgmii_an_master);
        ret |= serdes_access_wait_link_status(phy_dev);
        ret |= phy_dev_prog_ext(phy_dev, MU4_set_usxgmii_an_enable);
        break;
    case SERDES_MODE_AN_USXGMII_SLAVE:
        ret |= phy_dev_prog_ext(phy_dev, MU4_usxgmii_an_slave);
        ret |= serdes_access_wait_link_status(phy_dev);
        ret |= phy_dev_prog_ext(phy_dev, MU4_set_usxgmii_an_enable);
        break;
    case SERDES_MODE_FORCE_10G_USXGMII:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_10g_usxgmii);
        break;
    case SERDES_MODE_FORCE_5G_USXGMII:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_5g_usxgmii);
        break;
    case SERDES_MODE_FORCE_2P5G_USXGMII:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_2p5g_usxgmii);
        break;
    case SERDES_MODE_FORCE_1G_USXGMII:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_1g_usxgmii);
        break; 
    case SERDES_MODE_FORCE_100M_USXGMII:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_100m_usxgmii);
        break; 
    case SERDES_MODE_FORCE_10G_R:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_10g_R);
        break;
    case SERDES_MODE_FORCE_5G_R:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_5g_R);
        break;
    case SERDES_MODE_FORCE_2P5G_R:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_2p5g_R);
        break;
    case SERDES_MODE_FORCE_1G_R:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_1g_R);
        break;
    case SERDES_MODE_FORCE_5G:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_5g);
        break;
    case SERDES_MODE_FORCE_2P5G:
        ret |= phy_dev_c45_write_mask(phy_dev, 0x3, 0x9284, 0x3fff, 0, 0x0021);
        ret |= phy_dev_c45_write_mask(phy_dev, 0x3, 0x9285, 0x1fff, 0, 0x0010);
        ret |= phy_dev_c45_write_mask(phy_dev, 0x3, 0x9286, 0x00ff, 0, 0x0000);
        ret |= phy_dev_c45_write_mask(phy_dev, 0x3, 0x9286, 0xff00, 0, 0x0100);
        ret |= phy_dev_c45_write_mask(phy_dev, 0x3, 0x9287, 0x003f, 0, 0x0000);
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_2p5g_x);
        break;
    case SERDES_MODE_FORCE_1G:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_1g);
        break;
    case SERDES_MODE_FORCE_100M:
        ret |= phy_dev_prog_ext(phy_dev, MU4_force_speed_100m);
        break;
    default:
        return -1;
    }

    return ret;
}

static int _serdes_lane_init(phy_dev_t *phy_dev, int core_enabled, serdes_mode_t serdes_mode)
{
    int ret = 0;
    uint8_t core_id = phy_dev->core_index - SHASTA_BASE_CORE;
    uint8_t lane_id = phy_dev->lane_index;

    if (lane_mode[core_id][lane_id] == serdes_mode)
        return 0;

    ret |= serdes_access_lane_tx_enable(phy_dev, serdes_mode != SERDES_MODE_UNKNOWN);

    if (!core_enabled)
        goto Exit;

    ret |= phy_dev_prog_ext(phy_dev, MU4_change_speed);

#if defined(SERDES_LINK_POWER_DOWN)
    ret |= phy_dev_prog_ext(phy_dev, MU4_powerdn_lane);
#else
    ret |= phy_dev_prog_ext(phy_dev, MU4_en_datapath_reset_lane);
#endif

    if (serdes_mode == SERDES_MODE_UNKNOWN)
        goto Exit;

#if defined(SERDES_LINK_POWER_DOWN)
    ret |= phy_dev_prog_ext(phy_dev, MU4_powerup_lane);
#endif

    ret |= phy_dev_prog_ext(phy_dev, MU4_en_datapath_reset_lane);
    ret |= _serdes_lane_uc_cfg(phy_dev, serdes_mode);
    ret |= _serdes_lane_txfir_cfg(phy_dev, serdes_mode);
    ret |= _serdes_lane_speed_cfg(phy_dev, serdes_mode);
    ret |= phy_dev_prog_ext(phy_dev, MU4_lpi_enable);
    ret |= phy_dev_prog_ext(phy_dev, MU4_datapath_reset_lane);

Exit:
    lane_mode[core_id][lane_id] = serdes_mode;

    return ret;
}

#if defined(SERDES_LINK_POWER_DOWN)
static int _is_other_lane_active(phy_dev_t *phy_dev)
{
    int i;
    uint8_t core_id = phy_dev->core_index - SHASTA_BASE_CORE;
    uint8_t lane_id = phy_dev->lane_index;
    int enabled_lanes = 0;

    for (i = 0; i < LANES_PER_CORE; i++)
    {
        if (i == lane_id)
            continue;

        if (lane_mode[core_id][i] != SERDES_MODE_UNKNOWN)
            enabled_lanes++;
    }

    return enabled_lanes ? 1 : 0;
}
#endif

static int _phy_init_mode(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    int ret;
    int enable_core = 1;
    phy_dev_t base_phy = *phy_dev;

    base_phy.lane_index = 0;
    base_phy.addr = phy_dev->addr - phy_dev->lane_index;

#if defined(SERDES_LINK_POWER_DOWN)
    enable_core = _is_other_lane_active(phy_dev) || serdes_mode != SERDES_MODE_UNKNOWN;
#endif

    if ((ret = _serdes_core_init(&base_phy, enable_core)))
        return ret;

    if ((ret = _serdes_lane_init(phy_dev, enable_core, serdes_mode)))
        return ret;

    return 0;
}

static serdes_mode_t _get_serdes_mode(phy_dev_t *phy_dev)
{
    serdes_mode_t serdes_mode = SERDES_MODE_UNKNOWN;

    if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_10GBASE_R))
        serdes_mode = SERDES_MODE_FORCE_10G_R;
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_5GBASE_R))
        serdes_mode = SERDES_MODE_FORCE_5G_R;
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_2P5GBASE_R))
        serdes_mode = SERDES_MODE_FORCE_2P5G_R;
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_1GBASE_R))
        serdes_mode = SERDES_MODE_FORCE_1G_R;
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_5GBASE_X))
        serdes_mode = SERDES_MODE_FORCE_5G;
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_2P5GBASE_X))
        serdes_mode = SERDES_MODE_FORCE_2P5G;
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_1GBASE_X))
        serdes_mode = SERDES_MODE_FORCE_1G;

    return serdes_mode;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    serdes_mode_t serdes_mode;

    if (phy_dev->cascade_next)
        serdes_mode = SERDES_MODE_UNKNOWN;
    else
        serdes_mode = _get_serdes_mode(phy_dev);

    return _phy_init_mode(phy_dev, serdes_mode);
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;
    serdes_mode_t serdes_mode = SERDES_MODE_UNKNOWN;
    uint8_t core_id = phy_dev->core_index - SHASTA_BASE_CORE;
    uint8_t lane_id = phy_dev->lane_index;

    if (speed == PHY_SPEED_UNKNOWN)
    {
        serdes_mode = SERDES_MODE_UNKNOWN;
    }
    else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_USXGMII))
    {
        if (speed == PHY_SPEED_10000)
            serdes_mode = SERDES_MODE_FORCE_10G_USXGMII;
        else if (speed == PHY_SPEED_5000)
            serdes_mode = SERDES_MODE_FORCE_5G_USXGMII;
        else if (speed == PHY_SPEED_2500)
            serdes_mode = SERDES_MODE_FORCE_2P5G_USXGMII;
        else if (speed == PHY_SPEED_1000)
            serdes_mode = SERDES_MODE_FORCE_1G_USXGMII;
        else if (speed == PHY_SPEED_100)
            serdes_mode = SERDES_MODE_FORCE_100M_USXGMII;
    }
    else
    {
        if (speed == PHY_SPEED_10000)
        {
            if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_10GBASE_R))
                serdes_mode = SERDES_MODE_FORCE_10G_R;
        }
        else if (speed == PHY_SPEED_5000)
        {
            if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_5GBASE_R))
                serdes_mode = SERDES_MODE_FORCE_5G_R;
            else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_5GBASE_X))
                serdes_mode = SERDES_MODE_FORCE_5G;
        }
        else if (speed == PHY_SPEED_2500)
        {
            if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_2P5GBASE_R))
                serdes_mode = SERDES_MODE_FORCE_2P5G_R;
            else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_2P5GBASE_X))
                serdes_mode = SERDES_MODE_FORCE_2P5G;
        }
        else if (speed == PHY_SPEED_1000)
        {
            if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_1GBASE_R))
                serdes_mode = SERDES_MODE_FORCE_1G_R;
            else if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_1GBASE_X))
                serdes_mode = SERDES_MODE_FORCE_1G;
        }
        else if (speed == PHY_SPEED_100)
        {
            serdes_mode = SERDES_MODE_FORCE_100M;
        }
    }

    if ((ret = _phy_init_mode(phy_dev, serdes_mode)))
        return ret;

    configured_mode[core_id][lane_id] = serdes_mode;

    return 0;
}

static int _serdes_enable(phy_dev_t *phy_dev, int8_t module_detect)
{
    uint8_t core_id = phy_dev->core_index - SHASTA_BASE_CORE;
    uint8_t lane_id = phy_dev->lane_index;
    serdes_mode_t serdes_mode = configured_mode[core_id][lane_id];

    if (serdes_mode == SERDES_MODE_UNKNOWN)
    {
        serdes_mode = _get_serdes_mode(phy_dev);
#if defined(SERDES_MODULE_DETECT)
        if (!module_detect)
            serdes_mode = SERDES_MODE_UNKNOWN;
#endif
    }

    if (!(phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED))
        serdes_mode = SERDES_MODE_UNKNOWN;

    _phy_init_mode(phy_dev, serdes_mode);

    return serdes_mode == SERDES_MODE_UNKNOWN;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    int ret = 0;
    uint8_t link_status, module_detect;
    phy_speed_t speed = PHY_SPEED_UNKNOWN;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if (phy_dev->cascade_next)
    {
        if ((ret = serdes_access_get_an_status(phy_dev, &link_status)))
            goto Exit;
    }
    else
    {
        if ((ret = serdes_access_get_status(phy_dev, &link_status, &module_detect)))
            goto Exit;

        if (_serdes_enable(phy_dev, module_detect))
            goto Exit;
    }

    if (link_status)
    {
        if ((ret = serdes_access_get_speed(phy_dev, &speed)))
            return ret;

        if (speed != PHY_SPEED_UNKNOWN)
        {
            phy_dev->link = 1;
            phy_dev->speed = speed;
            phy_dev->duplex = PHY_DUPLEX_FULL;
        }

        return 0;
    }

Exit:
    return ret;
}

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    int ret = 0;

    *enable = phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED ? 1 : 0;

    return ret;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    return 0;
}

static int _phy_caps_get(phy_dev_t *phy_dev, int caps_type,  uint32_t *pcaps)
{
    if ((caps_type != CAPS_TYPE_ADVERTISE) && (caps_type != CAPS_TYPE_SUPPORTED))
        return 0;

    *pcaps = PHY_CAP_AUTONEG;

    if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_USXGMII))
        *pcaps |= PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000;
    if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_10GBASE_R))
        *pcaps |= PHY_CAP_10000;
    if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_5GBASE_R) || is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_5GBASE_X))
        *pcaps |= PHY_CAP_5000;
    if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_2P5GBASE_R) || is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_2P5GBASE_X))
        *pcaps |= PHY_CAP_2500;
    if (is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_1GBASE_R) || is_inter_phy_type_supported(phy_dev, INTER_PHY_TYPE_1GBASE_X))
        *pcaps |= PHY_CAP_1000_FULL | PHY_CAP_100_FULL;

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
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
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
        _phy_speed_set(phy_dev, PHY_SPEED_10000, PHY_DUPLEX_FULL);
    else if (caps & PHY_CAP_5000)
        _phy_speed_set(phy_dev, PHY_SPEED_5000, PHY_DUPLEX_FULL);
    else if (caps & PHY_CAP_2500)
        _phy_speed_set(phy_dev, PHY_SPEED_2500, PHY_DUPLEX_FULL);
    else if (caps & PHY_CAP_1000_FULL)
        _phy_speed_set(phy_dev, PHY_SPEED_1000, PHY_DUPLEX_FULL);
    else if (caps & PHY_CAP_100_FULL)
        _phy_speed_set(phy_dev, PHY_SPEED_100, PHY_DUPLEX_FULL);

    return 0;
}

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
    /* TODO: implement LED configuration */
    return 0;
}

static int _phy_diag(phy_dev_t *phy_dev, int level)
{
    int ret;

    phy_dev_enable_read_status(phy_dev, 0);
    ret = merlin16_shasta_display_diag_data(phy_dev, level);
    phy_dev_enable_read_status(phy_dev, 1);

    return ret;
}

static int _phy_tx_cfg_get(phy_dev_t *phy_dev, int8_t *pre, int8_t *main, int8_t *post1, int8_t *post2, int8_t *hpf)
{
    return merlin16_shasta_read_tx_afe(phy_dev, TX_AFE_PRE, pre) || 
        merlin16_shasta_read_tx_afe(phy_dev, TX_AFE_MAIN, main) ||
        merlin16_shasta_read_tx_afe(phy_dev, TX_AFE_POST1, post1) ||
        merlin16_shasta_read_tx_afe(phy_dev, TX_AFE_POST2, post2) ||
        merlin16_shasta_rd_tx_hpf_config(phy_dev, (uint8_t *)hpf);
}

static int _phy_tx_cfg_set(phy_dev_t *phy_dev, int8_t pre, int8_t main, int8_t post1, int8_t post2, int8_t hpf)
{
    return merlin16_shasta_apply_txfir_cfg(phy_dev, pre, main, post1, post2) ||
        merlin16_shasta_config_tx_hpf(phy_dev, hpf);
}

phy_drv_t phy_drv_shasta =
{
    .phy_type = PHY_TYPE_SHASTA,
    .read_status = _phy_read_status,
    .speed_set = _phy_speed_set,
    .power_get = _phy_power_get,
    .power_set = _phy_power_set,
    .caps_get = _phy_caps_get,
    .caps_set = _phy_caps_set,
    .c45_read = serdes_access_read,
    .c45_write = serdes_access_write,
    .c45_read_mask = serdes_access_read_mask,
    .c45_write_mask = serdes_access_write_mask,
    .leds_init = _phy_leds_init,
    .diag = _phy_diag,
    .tx_cfg_get = _phy_tx_cfg_get,
    .tx_cfg_set = _phy_tx_cfg_set,
    .init = _phy_init,
    .name = "SHASTA",
};
