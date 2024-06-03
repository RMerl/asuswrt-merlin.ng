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
 *  Created on: Mar 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Merlin Mptwo SerDes PHY driver
 */

#include "phy_drv.h"
#include "serdes_access.h"
#include "merlin_mptwo_functions.h"
#include "merlin_mptwo/merlin_mptwo_ucode_image.h"
#include <linux/delay.h>

#define LANES_PER_CORE   2

static int core_enabled[MPTWO_CORES];
static serdes_mode_t lane_mode[MPTWO_CORES][LANES_PER_CORE];

int merlin_mptwo_wrapper_display_diag_data(phy_dev_t *phy_dev, uint32_t level);
int merlin_mptwo_wrapper_ucode_mdio_load(phy_dev_t *phy_dev, uint8_t *ucode_image, uint16_t ucode_len);
int merlin_mptwo_wrapper_ucode_ram_read(phy_dev_t *phy_dev, uint16_t size, uint16_t addr, uint16_t *val);
int merlin_mptwo_wrapper_ucode_ram_write(phy_dev_t *phy_dev, uint16_t size, uint16_t addr, uint16_t val);
int merlin_mptwo_wrapper_txfir_cfg_get(phy_dev_t *phy_dev, int8_t *pre, int8_t *main, int8_t *post1, int8_t *post2);
int merlin_mptwo_wrapper_txfir_cfg_set(phy_dev_t *phy_dev, int8_t pre, int8_t main, int8_t post1, int8_t post2);
int merlin_mptwo_wrapper_tx_hpf_get(phy_dev_t *phy_dev, uint8_t *val);
int merlin_mptwo_wrapper_tx_hpf_set(phy_dev_t *phy_dev, uint8_t val);
int merlin_mptwo_wrapper_get_uc_lane_cfg(phy_dev_t *phy_dev, struct merlin_mptwo_uc_lane_config_st *val);
int merlin_mptwo_wrapper_set_uc_lane_cfg(phy_dev_t *phy_dev, struct merlin_mptwo_uc_lane_config_st val);
int merlin_mptwo_wrppaer_lane_pwrdn(phy_dev_t *phy_dev, enum srds_core_pwrdn_mode_enum mode);

static int _serdes_core_init(phy_dev_t *phy_dev, int enable)
{
    int ret = 0;
    uint8_t core_id = phy_dev->core_index - MPTWO_BASE_CORE;

    if (core_enabled[core_id] == enable)
        return 0;

    serdes_access_config(phy_dev, enable);

    if (!enable)
        goto Exit;

    phy_dev_c45_write_mask(phy_dev, 0x1, 0xffdd, MASK_BIT(15), 15, 0x1); /* enable multiport */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xffdc, MASK_BITS_BETWEEN(0, 4), 0, phy_dev->addr); /* broadcast address */

    if ((ret = merlin_mptwo_wrapper_ucode_mdio_load(phy_dev, merlin_mptwo_ucode_image, MERLIN_MPTWO_UCODE_IMAGE_SIZE)))
        goto Exit;

    /* PMD Setup 50MHz */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b9, MASK_BIT(0), 0, 0x0); /* mmd_resetb */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b3, MASK_BITS_BETWEEN(11, 15), 11, 0x4); /* div */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b6, MASK_BITS_BETWEEN(12, 15), 12, 0x0); /* i_ndiv_frac_l */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b7, MASK_BITS_BETWEEN(0, 13), 0, 0x1000); /* i_ndiv_frac_h */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b8, MASK_BIT(14), 14, 0x1); /* i_pll_sdm_pwrdnb */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b8, MASK_BIT(13), 13, 0x1); /* mmd_en */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b8, MASK_BIT(12), 12, 0x0); /* mmd_prsc4or5pwdb */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b8, MASK_BIT(11), 11, 0x1); /* mmd_prsc8or9pwdb */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b8, MASK_BIT(10), 10, 0x1); /* mmd_div_range */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b8, MASK_BITS_BETWEEN(0, 9), 0, 0xce); /* i_ndiv_int */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0b9, MASK_BITS_BETWEEN(0, 2), 0, 0x3); /* i_pll_frac_mode, mmd_resetb */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9100, MASK_BITS_BETWEEN(13, 15), 13, 0x6); /* refclk_sel */

    /* AN Timer Speed Up */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9107, MASK_BITS_BETWEEN(15,15), 15, 0x00); /* tick_override */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9250, MASK_BITS_BETWEEN(0,15), 0, 0x029a); /* cl73_restart_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9251, MASK_BITS_BETWEEN(0,15), 0, 0x029a); /* cl37_ack_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9252, MASK_BITS_BETWEEN(0,15), 0, 0xa000); /* cl37_error_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9253, MASK_BITS_BETWEEN(0,15), 0, 0x10ee); /* cl73_break_link_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9254, MASK_BITS_BETWEEN(0,15), 0, 0xa000); /* cl73_error_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9255, MASK_BITS_BETWEEN(0,15), 0, 0x0bb8); /* cl73_pd_dme_lock_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9256, MASK_BITS_BETWEEN(0,15), 0, 0x1b00); /* cl73_link_up_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9257, MASK_BITS_BETWEEN(0,15), 0, 0x8235); /* link_fail_inhibit_timer_cl72 */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x9258, MASK_BITS_BETWEEN(0,15), 0, 0x8235); /* link_fail_inhibit_timer_ncl72 */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x925a, MASK_BITS_BETWEEN(0,15), 0, 0x8235); /* cl72_max_wait_timer */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0x925c, MASK_BITS_BETWEEN(0,15), 0, 0x000f); /* ignore_link_timer_period */

    /* Datapath Reset */
    phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0f2, MASK_BIT(0), 0, 0x1); /* core_dp_s_rstb */
    udelay(10000);

    /* afe_hw_version */
    merlin_mptwo_wrapper_ucode_ram_write(phy_dev, 1, CORE_VAR_RAM_BASE + 0x0f, 0x1);

    if ((ret = serdes_access_wait_pll_lock(phy_dev)))
        goto Exit;

Exit:
    if (!ret)
        core_enabled[core_id] = enable;

    return ret;
}

/* Lane configuration (RAM Variables) */
static int _serdes_lane_uc_cfg(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    struct merlin_mptwo_uc_lane_config_st lane_conf;

    merlin_mptwo_wrapper_get_uc_lane_cfg(phy_dev, &lane_conf);

    switch (serdes_mode)
    {
    case SERDES_MODE_FORCE_1G:
        break; 
    case SERDES_MODE_AN_1G_SGMII:
    case SERDES_MODE_AN_SGMII_SLAVE:
        lane_conf.field.an_enabled = 1;
        lane_conf.field.lane_cfg_from_pcs = 1;
        break;
    case SERDES_MODE_AN_1G_IEEE_CL37:
    case SERDES_MODE_AN_1G_USER_CL37:
        lane_conf.field.media_type = MEDIA_TYPE_OPTICS;
        lane_conf.field.an_enabled = 1;
        lane_conf.field.lane_cfg_from_pcs = 1;
        break;
    case SERDES_MODE_FORCE_2P5G:
        break;
    case SERDES_MODE_FORCE_10G_R:
        break;
    case SERDES_MODE_SFI:
        lane_conf.field.dfe_on = 1;
        lane_conf.field.media_type = MEDIA_TYPE_OPTICS;
        lane_conf.field.unreliable_los = 0;
        break;
    case SERDES_MODE_AN_10G_KR_IEEE_CL73:
        lane_conf.field.dfe_on = 1;
        lane_conf.field.an_enabled = 1;
        lane_conf.field.lane_cfg_from_pcs = 1;
        break;
    default:
        return -1;
    }

    merlin_mptwo_wrapper_set_uc_lane_cfg(phy_dev, lane_conf);

    return 0;
}

/* Transmitter Configuration */
static int _serdes_lane_txfir_cfg(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    int ret = 0;

    switch (serdes_mode)
    {
    case SERDES_MODE_FORCE_1G:
    case SERDES_MODE_AN_1G_SGMII:
    case SERDES_MODE_AN_SGMII_SLAVE:
        merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, 0, 40, 0, 0);
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, 0);
        break;
    case SERDES_MODE_AN_1G_IEEE_CL37:
    case SERDES_MODE_AN_1G_USER_CL37:
        merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, 2, 44, 11, 2);
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, 0);
        break;
    case SERDES_MODE_FORCE_2P5G:
        merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, 0, 60, 0, 0);
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, 4); /* XXX ?? */
        break;
    case SERDES_MODE_SFI:
        merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, 2, 44, 11, 2);
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, 0);
        break;
    case SERDES_MODE_AN_10G_KR_IEEE_CL73:
        merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, 0, 36, 0, 0);
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, 3);
        break;
    case SERDES_MODE_FORCE_10G_R:
        merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, 0, 46, 8, 0);
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, 0);
        break;
    default:
        return -1;
    }

    return ret;
}

/* Change lane speed */
static int _serdes_lane_speed_cfg(phy_dev_t *phy_dev, serdes_mode_t serdes_mode)
{
    switch (serdes_mode)
    {
    case SERDES_MODE_FORCE_1G:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x1); /* SW_actual_speed_force_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x2); /* SW_actual_speed */
        break; 
    case SERDES_MODE_AN_1G_IEEE_CL37:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x1); /* use_ieee_reg_ctrl_sel */
        phy_dev_c45_write_mask(phy_dev, 0x0, 0x0004, MASK_BIT(5), 5, 0x1); /* duplex */
        phy_dev_c45_write_mask(phy_dev, 0x0, 0x0000, MASK_BIT(12), 12, 0x1); /* AN enable */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case SERDES_MODE_AN_1G_USER_CL37:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /*use_ieee_reg_ctrl_sel */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc481, MASK_BIT(4), 4, 0x1); /* duplex */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc480, MASK_BIT(6), 6, 0x1); /* AN enable */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case SERDES_MODE_AN_1G_SGMII:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /* use_ieee_reg_ctrl_sel */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc481, MASK_BIT(2), 2, 0x1); /* duplex */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc481, MASK_BITS_BETWEEN(0,1), 0, 0x2); /* speed */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc481, MASK_BIT(9), 9, 0x1); /* mater */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /* AN enable */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case SERDES_MODE_AN_SGMII_SLAVE:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x2); /* use_ieee_reg_ctrl_sel */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc480, MASK_BITS_BETWEEN(6,7), 6, 0x3); /* AN enable */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_e n*/
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case SERDES_MODE_FORCE_2P5G:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x1); /* SW_actual_speed_force_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x3); /* SW_actual_speed */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0x9270, MASK_BITS_BETWEEN(0, 15), 0, 0x0021); /* Reg2p5G_ClockCount0 */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0x9233, MASK_BITS_BETWEEN(0, 15), 0, 0x0002); /* Reg2p5G_modulo */
        break;
    case SERDES_MODE_AN_10G_KR_IEEE_CL73:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(12,13), 12, 0x0); /* use_ieee_reg_ctrl_sel */
        phy_dev_c45_write_mask(phy_dev, 0x7, 0x0011, MASK_BITS_BETWEEN(5,15), 5, 0x4); /* speed advertisment */
        phy_dev_c45_write_mask(phy_dev, 0x7, 0x0010, MASK_BITS_BETWEEN(0,4), 0, 0x1); /* base selector */
        phy_dev_c45_write_mask(phy_dev, 0x7, 0x0012, MASK_BITS_BETWEEN(14,15), 14, 0x0); /* FEC supported */
        phy_dev_c45_write_mask(phy_dev, 0x7, 0x0000, MASK_BITS_BETWEEN(12,12), 12, 0x1); /* AN enable */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x0); /* SW_actual_speed_force_en */
        break;
    case SERDES_MODE_SFI:
    case SERDES_MODE_FORCE_10G_R:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(11), 11, 0x0); /* credit_sw_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(6), 6, 0x1); /* SW_actual_speed_force_en */
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0xf); /* SW_actual_speed */
        break;
    default:
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc457, MASK_BITS_BETWEEN(0, 15), 0, 0x0000);
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(0, 15), 0, 0x1002);
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc433, MASK_BITS_BETWEEN(0, 15), 0, 0x01c8);
        phy_dev_c45_write_mask(phy_dev, 0x1, 0xd081, MASK_BITS_BETWEEN(0, 15), 0, 0x0001);
        phy_dev_c45_write_mask(phy_dev, 0x1, 0xd082, MASK_BITS_BETWEEN(0, 15), 0, 0x0033);
        return 0;
    }

    phy_dev_c45_write_mask(phy_dev, 0x3, 0xc457, MASK_BIT(0), 0, 0x1); /* rx_rstb_lane */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BIT(7), 7, 0x1); /* mac_creditenable */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0xc433, MASK_BIT(1), 1, 0x1); /* tx_rstb_lane */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0xc433, MASK_BIT(0), 0, 0x1); /* enable_tx_lane */

    /* LPI pass-through */
    phy_dev_c45_write_mask(phy_dev, 0x3, 0xc450, MASK_BIT(2), 2, 0x1); /* lpi_enable */

    return 0;
}

static int _serdes_lane_init(phy_dev_t *phy_dev, int core_enabled, serdes_mode_t serdes_mode)
{
    int ret = 0;
    uint8_t core_id = phy_dev->core_index - MPTWO_BASE_CORE;
    uint8_t lane_id = phy_dev->lane_index;

    if (lane_mode[core_id][lane_id] == serdes_mode)
        return 0;

    ret |= serdes_access_lane_tx_enable(phy_dev, serdes_mode != SERDES_MODE_UNKNOWN);

    if (!core_enabled)
        goto Exit;

    ret |= phy_dev_c45_write_mask(phy_dev, 0x1, 0xd082, MASK_BITS_BETWEEN(0, 15), 0, 0x0000);
    ret |= phy_dev_c45_write_mask(phy_dev, 0x3, 0x0000, MASK_BIT(15), 15, 0x1); /* lane reset */
    ret |= phy_dev_c45_write_mask(phy_dev, 0x1, 0xd081, MASK_BIT(1), 1, 0x0); /* ln_dp_s_rstb */
    ret |= phy_dev_c45_write_mask(phy_dev, 0x1, 0xd0e3, MASK_BIT(1), 1, 0x1); /* byppass pmd tx oversampling */

#if defined(SERDES_LINK_POWER_DOWN)
    ret |= merlin_mptwo_wrppaer_lane_pwrdn(phy_dev, PWRDN);
#endif

    if (serdes_mode == SERDES_MODE_UNKNOWN)
        goto Exit;

    ret |= _serdes_lane_uc_cfg(phy_dev, serdes_mode);
    ret |= _serdes_lane_txfir_cfg(phy_dev, serdes_mode);
    ret |= _serdes_lane_speed_cfg(phy_dev, serdes_mode);

#if defined(SERDES_LINK_POWER_DOWN)
    ret |= merlin_mptwo_wrppaer_lane_pwrdn(phy_dev, PWR_ON);
#endif

    ret |= phy_dev_c45_write_mask(phy_dev, 0x1, 0xd081, MASK_BIT(1), 1, 0x1); /* ln_dp_s_rstb */
    udelay(10000);

Exit:
    if (!ret)
        lane_mode[core_id][lane_id] = serdes_mode;

    return ret;
}

#if defined(SERDES_LINK_POWER_DOWN)
static int _is_other_lane_active(phy_dev_t *phy_dev)
{
    int i;
    uint8_t core_id = phy_dev->core_index - MPTWO_BASE_CORE;
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

    if (phy_dev->cascade_next)
        serdes_mode = SERDES_MODE_UNKNOWN;
    else if (phy_dev->mii_type == PHY_MII_TYPE_XFI)
        serdes_mode = SERDES_MODE_FORCE_10G_R;
    else if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        serdes_mode = SERDES_MODE_FORCE_2P5G;
    else if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        serdes_mode = SERDES_MODE_FORCE_1G;

    return serdes_mode;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    return _phy_init_mode(phy_dev, _get_serdes_mode(phy_dev));
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;
    serdes_mode_t serdes_mode;

    switch (speed)
    {
    case PHY_SPEED_10000:
    case PHY_SPEED_5000:
        serdes_mode = SERDES_MODE_FORCE_10G_R;
        break;
    case PHY_SPEED_2500:
        serdes_mode = SERDES_MODE_FORCE_2P5G;
        break;
    case PHY_SPEED_1000:
        serdes_mode = SERDES_MODE_FORCE_1G;
        break;
    case PHY_SPEED_100:
        serdes_mode = SERDES_MODE_FORCE_1G;
        break;
    default:
        serdes_mode = SERDES_MODE_UNKNOWN;
        break;
    }

    if ((ret = _phy_init_mode(phy_dev, serdes_mode)))
        return ret;

    if (speed == PHY_SPEED_100)
    {
        phy_dev_c45_write_mask(phy_dev, 0x1, 0xd081, MASK_BIT(1), 1, 0x0); /* ln_dp_s_rstb */
        udelay(10000);
        phy_dev_c45_write_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, 0x1); /* SW_actual_speed */
        phy_dev_c45_write_mask(phy_dev, 0x1, 0xd081, MASK_BIT(1), 1, 0x1); /* ln_dp_s_rstb */
        udelay(10000);
    }

    return 0;
}

static int _get_serdes_speed(phy_dev_t *phy_dev, phy_speed_t *speed)
{
    uint16_t port_speed, port_up;

    phy_dev_c45_read_mask(phy_dev, 0x3, 0xc474, MASK_BIT(1), 1, &port_up);
    phy_dev_c45_read_mask(phy_dev, 0x3, 0xc30b, MASK_BITS_BETWEEN(0, 5), 0, &port_speed);
    *speed = PHY_SPEED_UNKNOWN;

    if (!port_up)
        return 0;

    switch (port_speed)
    {
    case 0x00:
        *speed = PHY_SPEED_10;
        break;
    case 0x01:
        *speed = PHY_SPEED_100;
        break;
    case 0x02:
    case 0x0d:
    case 0x34:
        *speed = PHY_SPEED_1000;
        break;
    case 0x03:
    case 0x04:
        *speed = PHY_SPEED_2500;
        break;
    case 0x0f:
    case 0x1b:
    case 0x1f:
    case 0x33:
        *speed = PHY_SPEED_10000;
        break;
    default:
        break;
    }

    return 0;
}

#if defined(SERDES_SPEED_DETECT)
serdes_mode_t detect_modes[] =
{
    SERDES_MODE_UNKNOWN,
    SERDES_MODE_FORCE_10G_R,
    SERDES_MODE_AN_1G_IEEE_CL37,
    SERDES_MODE_AN_1G_USER_CL37,
    SERDES_MODE_AN_SGMII_SLAVE,
    SERDES_MODE_AN_1G_SGMII,
};

static int _set_next_serdes_mode(phy_dev_t *phy_dev)
{
    int i, found = 0;
    int count = sizeof(detect_modes)/sizeof(detect_modes[0]);
    serdes_mode_t serdes_mode = phy_dev->current_inter_phy_type;

    for (i = 0; i < count - 1; i++)
    {
        if (serdes_mode == detect_modes[i])
        {
            found = 1;
            break;
        }
    }

    if (found)
        serdes_mode = detect_modes[i + 1];
    else
        serdes_mode = SERDES_MODE_UNKNOWN;

    phy_dev->current_inter_phy_type = serdes_mode;

    return _phy_init_mode(phy_dev, serdes_mode);
}
#endif

static int _serdes_enable(phy_dev_t *phy_dev, int enable, int8_t module_detect)
{
    int quit = 1;
    serdes_mode_t serdes_mode = SERDES_MODE_UNKNOWN;

    if (phy_dev->cascade_next)
        return 0;

    if (enable)
        serdes_mode = _get_serdes_mode(phy_dev);

#if defined(SERDES_MODULE_DETECT)
    if (enable == module_detect)
        quit = 0;
#endif

    if (enable == (phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED))
        quit = 0;

    if (quit)
        return 0;

    _phy_init_mode(phy_dev, serdes_mode);
    phy_dev->current_inter_phy_type = serdes_mode;

    return 1;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    int ret = 0;
    uint8_t link_status, module_detect;
    phy_speed_t speed = PHY_SPEED_UNKNOWN;

    if ((ret = serdes_access_get_status(phy_dev, &link_status, &module_detect)))
        goto Exit_Dn;

    if (_serdes_enable(phy_dev, 0, module_detect))
        goto Exit_Dn;

    if (link_status)
    {
        if ((ret = _get_serdes_speed(phy_dev, &speed)))
            goto Exit_Dn;

        if (speed != PHY_SPEED_UNKNOWN)
        {
            phy_dev->link = 1;
            phy_dev->speed = speed;
            phy_dev->duplex = PHY_DUPLEX_FULL;
            return 0;
        }

        goto Exit_Dn;
    }

#if defined(SERDES_SPEED_DETECT)
    if (!phy_dev->cascade_next)
        return _set_next_serdes_mode(phy_dev);
#endif

    if (_serdes_enable(phy_dev, 1, module_detect))
        goto Exit_Dn;

Exit_Dn:
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
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

    *pcaps = PHY_CAP_AUTONEG | PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000;

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

    return 0;
}

extern int lport_led_init(void *leds_info);

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
    return lport_led_init(leds_info);
}

static int _phy_diag(phy_dev_t *phy_dev, int level)
{
    int ret;

    phy_dev_enable_read_status(phy_dev, 0);
    ret = merlin_mptwo_wrapper_display_diag_data(phy_dev, level);
    phy_dev_enable_read_status(phy_dev, 1);

    return ret;
}

static int _phy_tx_cfg_get(phy_dev_t *phy_dev, int8_t *pre, int8_t *main, int8_t *post1, int8_t *post2, int8_t *hpf)
{
    return merlin_mptwo_wrapper_txfir_cfg_get(phy_dev, pre, main, post1, post2) ||
        merlin_mptwo_wrapper_tx_hpf_get(phy_dev, (uint8_t *)hpf);
}

static int _phy_tx_cfg_set(phy_dev_t *phy_dev, int8_t pre, int8_t main, int8_t post1, int8_t post2, int8_t hpf)
{
    return merlin_mptwo_wrapper_txfir_cfg_set(phy_dev, pre, main, post1, post2) ||
        merlin_mptwo_wrapper_tx_hpf_set(phy_dev, (uint8_t)hpf);
}

phy_drv_t phy_drv_mptwo =
{
    .phy_type = PHY_TYPE_MPTWO,
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
    .name = "MPTWO",
};
