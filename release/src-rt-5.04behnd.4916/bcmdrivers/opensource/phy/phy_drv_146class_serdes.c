/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

/*
 * Phy drivers for 10G Active Ethernet Serdes
 */
#include "phy_drv_dsl_serdes.h"
#include "Serdes146Class/phy_drv_merlin16.h"
#include "serdes_access.h"

static void dsl_merlin16_serdes_link_stats(phy_dev_t *phy_dev);
static int dsl_merlin16_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int _merlin_core_power_op(phy_dev_t *phy_dev, int power_level);
static int dsl_merlin16_light_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->signal_detect_gpio == -1 || phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;

    return merlin_ext_signal_detected(phy_dev);
}

#define MAX_146CORES 3
#define MAX_146USXGMII_PORTS 4
static phy_serdes_t serdes_146class[MAX_146CORES][MAX_146USXGMII_PORTS] =
{
    {{
        .phy_type = PHY_TYPE_146CLASS_SERDES,
#if defined(CONFIG_BCM963146)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR_M,
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96765)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR5KXR10R_M,
#endif
        .link_stats = dsl_merlin16_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .power_mode = SERDES_BASIC_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = dsl_merlin16_speed_set,
        .power_set = _merlin_core_power_op,
        .cur_power_level = -1,
        .light_detected = dsl_merlin16_light_detected,
        .read_txfir_reg = merlin16_read_txfir_reg,
        .write_txfir_reg = merlin16_write_txfir_reg,
        //.lbe_op = dsl_merlin16_lbe_op,
    }},
    {{
        .phy_type = PHY_TYPE_146CLASS_SERDES,
#if defined(CONFIG_BCM963146)
#define CONFIG_BCM963146_TESTING
#if defined(CONFIG_BCM963146_TESTING)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AMUS1KR2KXR5KXR_M,
#else
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR_M,
#endif
#elif defined(CONFIG_BCM94912)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR5KXR10R_M,
#elif defined(CONFIG_BCM96813)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AMUS1KR2KXR5KXR10R_M,
#endif
        .link_stats = dsl_merlin16_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .power_mode = SERDES_BASIC_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = dsl_merlin16_speed_set,
        .light_detected = dsl_merlin16_light_detected,
        .cur_power_level = -1,
        .power_set = _merlin_core_power_op,
        .read_txfir_reg = merlin16_read_txfir_reg,
        .write_txfir_reg = merlin16_write_txfir_reg,
        //.lbe_op = dsl_merlin16_lbe_op,
    }},
    {{
        .phy_type = PHY_TYPE_146CLASS_SERDES,
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AMUS1KR2KXR5KXR10R_M,
        .link_stats = dsl_merlin16_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .power_mode = SERDES_BASIC_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = dsl_merlin16_speed_set,
        .light_detected = dsl_merlin16_light_detected,
        .cur_power_level = -1,
        .power_set = _merlin_core_power_op,
        .read_txfir_reg = merlin16_read_txfir_reg,
        .write_txfir_reg = merlin16_write_txfir_reg,
        //.lbe_op = dsl_merlin16_lbe_op,
    }}
};

static int total_cores;
int phy_drv_serdes146_get_total_cores(void)
{
    return total_cores;
}

phy_dev_t *phy_drv_serdes146_get_phy_dev(int core_num, int port_num)
{
    phy_serdes_t *phy_serdes = &serdes_146class[core_num][port_num];
    phy_dev_t *phy_dev = phy_serdes->phy_dev;
    return phy_dev;
}

static int phy_drv_serdes_146class_init_lock(phy_dev_t *phy_dev)
{
    phy_dev_t *phy_dev_mmaster = mphy_get_master(phy_dev);
    phy_serdes_t *phy_serdes, *phy_serdes_mmaster = phy_dev_mmaster->priv;
    int i, j;

    mutex_lock(&serdes_mutex);

    phy_serdes = &serdes_146class[phy_dev->core_index][phy_dev->usxgmii_m_index];
    if (phy_serdes->inited)
    {
        printk("****** Duplicated Serdes at address %d defined in board parameter.\n", phy_dev->addr);
        BUG();
    }

    /* Polulate all non defined arrays */
    if (serdes_146class[0][1].speed_caps == 0)
    {
        for(i=0; i<ARRAY_SIZE(serdes_146class[0]); i++)
            for(j=1; j<ARRAY_SIZE(serdes_146class); j++)
                if (serdes_146class[j][i].speed_caps == 0)
                    memcpy(&serdes_146class[j][i], &serdes_146class[j][0], sizeof(phy_serdes_t));
    }

    phy_serdes->print_log = 1;
    phy_serdes->power_admin_on = 1;

    phy_serdes->usxgmii_m_index = phy_dev->usxgmii_m_index;
    phy_serdes->core_num = phy_dev->core_index;
    phy_serdes->handle = phy_dev->priv;
    phy_dev->priv = phy_serdes;

    phy_serdes->phy_dev = phy_dev;
    printk("\n" NtcClr "=== Start of 10G Active Ethernet Initialization for core %d port %d ===" DflClr "\n",
        phy_serdes->core_num, phy_dev->usxgmii_m_index);

    if (total_cores <= phy_serdes->core_num)
        total_cores = phy_serdes->core_num + 1;

    phy_dsl_serdes_init(phy_dev);

    if (!phy_dev_is_mphy(phy_dev) || mphy_is_master(phy_dev))
        merlin16_serdes_init(phy_dev);
    else
    {
        phy_serdes->cur_power_level = phy_serdes_mmaster->cur_power_level;
        phy_serdes->inited = 1; /* 1 means finished basic init other than the final speed */
    }

    phy_dsl_serdes_post_init(phy_dev);

    phy_serdes->inited = 2;
    printk(NtcClr "=== End of 10G Active Ethernet Initialization for core %d port %d ===" DflClr "\n",
        phy_serdes->core_num, phy_dev->usxgmii_m_index);

    mutex_unlock(&serdes_mutex);

    return 0;
}

static int dsl_merlin16_xfi_polarity_set(phy_dev_t *phy_dev, int inverse, int tx_dir)
{
    int ret;
    uint16_t val;
    int reg;

    reg = tx_dir? 0xd0e3: 0xd0d3;
    val = merlin_pmi_read16(phy_dev->core_index, phy_dev->lane_index, 1, reg);
    val &= ~1;
    val |= inverse? 1:0;
    ret = merlin_pmi_write16(phy_dev->core_index, phy_dev->lane_index, 1, reg, val, 0);
    val = merlin_pmi_read16(phy_dev->core_index, phy_dev->lane_index, 1, reg);
    return ret;
}

static int dsl_merlin16_xfi_tx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    return dsl_merlin16_xfi_polarity_set(phy_dev, inverse, 1);
}

static int dsl_merlin16_xfi_rx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    return dsl_merlin16_xfi_polarity_set(phy_dev, inverse, 0);
}

static int dsl_merlin16_priv_fun_lock(phy_dev_t *phy_dev, int op_code, va_list ap)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int reg;
    int val;
    uint32_t *valp;
    int ret = 0;
    phy_txfir_t *txfir;

    mutex_lock(&serdes_mutex);

    if ((op_code == PHY_OP_RD_MDIO ||
            op_code == PHY_OP_WR_MDIO) &&
            (ret = dsl_serdes_check_power(phy_dev)))
        goto end;

    switch(op_code)
    {
        case PHY_OP_RD_MDIO:
            reg = va_arg(ap, int);
            valp = va_arg(ap, uint32_t *);
            *valp = merlin_pmi_read16(phy_serdes->core_num, 0, reg >> 16, reg & 0xffff);
            break;
        case PHY_OP_WR_MDIO:
            reg = va_arg(ap, int);
            val = va_arg(ap, int);
            ret = merlin_pmi_write16(phy_serdes->core_num, 0, reg >> 16, reg & 0xffff, (uint16_t)val, 0);
            break;
        case PHY_OP_SET_TXFIR:
            txfir = va_arg(ap, phy_txfir_t *);
            ret = dsl_txfir_set(phy_dev, txfir);
            break;
        case PHY_OP_GET_TXFIR:
            txfir = va_arg(ap, phy_txfir_t *);
            ret = dsl_txfir_get(phy_dev, txfir);
            break;
        default:
            ret = dsl_serdes_priv_fun(phy_dev, op_code, ap);
    }

end:
    mutex_unlock(&serdes_mutex);
    return ret;
}

int merlin16_shortfin_display_diag_data(phy_dev_t *phy_dev, uint16_t  diag_level);
static int _phy_diag_lock(phy_dev_t *phy_dev, int level)
{
    int ret;
    mutex_lock(&serdes_mutex);
    ret = merlin16_shortfin_display_diag_data(phy_dev, level);
    mutex_unlock(&serdes_mutex);
    return ret;
}

static int _phy_tx_cfg_set(phy_dev_t *phy_dev, int8_t pre, int8_t main, int8_t post1, int8_t post2, int8_t hpf)
{
    int rc;

    if(!(rc = merlin16_shortfin_apply_txfir_cfg(phy_dev, pre, main, post1, post2)))
        rc = merlin16_shortfin_config_tx_hpf(phy_dev, hpf);
    return rc;
}

static int _phy_tx_cfg_get(phy_dev_t *phy_dev, int8_t *pre, int8_t *main, int8_t *post1, int8_t *post2, int8_t *hpf)
{
    int rc;
    rc = merlin16_shortfin_read_tx_afe(phy_dev, TX_AFE_PRE, pre) ||
        merlin16_shortfin_read_tx_afe(phy_dev, TX_AFE_MAIN, main) ||
        merlin16_shortfin_read_tx_afe(phy_dev, TX_AFE_POST1, post1) ||
        merlin16_shortfin_read_tx_afe(phy_dev, TX_AFE_POST2, post2) ||
        merlin16_shortfin_rd_tx_hpf_config(phy_dev, hpf);
    return 0;
}

static int _phy_shared_clock_set(phy_dev_t *phy_dev)
{
    phy_dev_t *phy_next = cascade_phy_get_next(phy_dev);

    if (!phy_next || !phy_next->shared_ref_clk_mhz)
        return 0;

    phy_dev->shared_ref_clk_mhz = phy_next->shared_ref_clk_mhz;
	PhySetSharedRefClk(phy_dev);
    printk("Set Serdes at address %d shared reference clock %dMHz.\n", phy_dev->addr, phy_dev->shared_ref_clk_mhz);
    return merlin16_shortfin_set_shared_clock(phy_dev);
}

static int _phy_temp_get(phy_dev_t *phy_dev, int16_t *temp)
{
	srds_access_t *sa__ = phy_dev;
	uint16_t tmon;

	ESTM(tmon=rdc_micro_pvt_tempdata_rmi());
	*temp = (int16_t)((45000000 - (54956 * tmon))/100000);
	return 0;
}

phy_drv_t phy_drv_serdes_146class =
{
    .init = phy_drv_serdes_146class_init_lock,
    .phy_type = PHY_TYPE_146CLASS_SERDES,
    .read_status = phy_dsl_serdes_read_status_lock,
    .apd_get = dsl_serdes_apd_get,
    .apd_set = dsl_serdes_apd_set_lock,
    .speed_set = dsl_serdes_cfg_speed_set_lock,
    .caps_get = dsl_serdes_caps_get,
    .caps_set = dsl_serdes_caps_set_lock,
    .config_speed_get = dsl_serdes_speed_get_lock,
    .priv_fun = dsl_merlin16_priv_fun_lock,
    .power_set = phy_dsl_serdes_power_set_lock,
    .power_get = phy_dsl_serdes_power_get,
    .inter_phy_types_get = dsl_serdes_inter_phy_types_get,
    .get_phy_name = dsl_serdes_get_phy_name,
    .dev_add = phy_dsl_serdes_dev_add_lock,
    .dt_priv = phy_dsl_serdes_dt_priv,
    .leds_init = dsl_phy_leds_init,
    .diag = _phy_diag_lock,
    .c45_read = serdes_access_read,
    .c45_write = serdes_access_write,
    .c45_read_mask = serdes_access_read_mask,
    .c45_write_mask = serdes_access_write_mask,
    .configured_inter_phy_speed_type_set = dsl_serdes_cfg_speed_mode_set_lock,
    .xfi_tx_polarity_inverse_set = dsl_merlin16_xfi_tx_polarity_set,
    .xfi_rx_polarity_inverse_set = dsl_merlin16_xfi_rx_polarity_set,
    .tx_cfg_get = _phy_tx_cfg_get,
    .tx_cfg_set = _phy_tx_cfg_set,
    .shared_clock_set = _phy_shared_clock_set,
    .phy_temp_get = _phy_temp_get,
#if defined(CONFIG_BCM963146)
    .name = "2.5GAE",
#else
    .name = "10GAE",
#endif
};

static void dsl_merlin16_serdes_link_stats(phy_dev_t *phy_dev)
{
    merlin_chk_lane_link_status(phy_dev);
}

static void merlin_set_msbus_clk_source(phy_dev_t *phy_dev, int power_level)
{
#if defined(ETH_PHY_TOP_XPORT0_CLK_CNTRL)
    uint32_t v32;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int power_level0, power_level1;
    int cur_source;
    phy_serdes_t *phy_serdes0, *phy_serdes1;

    if(phy_dev->core_index > 1)
        return;

    if (phy_serdes->cur_power_level == power_level)
        return;

    phy_serdes0 = &serdes_146class[0][0];
    phy_serdes1 = &serdes_146class[1][0];

    if (phy_dev->core_index)
    {
        power_level1 = power_level;
        if (phy_serdes0->inited)
            power_level0 = serdes_146class[0][0].cur_power_level;
        else
            power_level0 = 0;
    }
    else
    {
        power_level0 = power_level;
        if (phy_serdes1->inited)
            power_level1 = serdes_146class[1][0].cur_power_level;
        else
            power_level1 = 0;
    }

    v32 = *ETH_PHY_TOP_XPORT0_CLK_CNTRL;
    cur_source = v32 & ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL;

    /* We want to set MSBUS clock source to external XGPHY if both are available because
        SFP module needs to scan different speedand will constantly affect peer Serdes */
    if (power_level0 && power_level1 &&
        ((phy_serdes0->sfp_module_type == SFP_FIXED_PHY && phy_serdes1->sfp_module_type != SFP_FIXED_PHY) ||
        (phy_serdes1->sfp_module_type == SFP_FIXED_PHY && phy_serdes0->sfp_module_type != SFP_FIXED_PHY)) &&
        ((cur_source == 0 && phy_serdes0->sfp_module_type != SFP_FIXED_PHY) ||
        (cur_source == 1 && phy_serdes1->sfp_module_type != SFP_FIXED_PHY)))
        goto flip;

    if ((cur_source && power_level1) || (!cur_source && power_level0) || (!power_level0 && !power_level1))
        return;

    /* We need to flip source */
flip:
    if (cur_source)
        v32 &= ~ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL;
    else
        v32 |= ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL;

    *ETH_PHY_TOP_XPORT0_CLK_CNTRL = v32;
#endif
}

static int _merlin_core_power_op(phy_dev_t *phy_dev, int power_level)
{
    int ret;
    /*
        For USXGMII-M mode, we will let cascading PHY do the power down and keep
        Serdes on all the time, because Serdes is the aggregation of four ports of
        cascaded Copper PHY, while cascaded Copper PHY has separate four ports
    */
    if (phy_dev_is_mphy(phy_dev))
    {
        if (power_level == 0)
            return 0;
        phy_dev = mphy_get_master(phy_dev);
    }

    merlin_set_msbus_clk_source(phy_dev, power_level);
    ret = merlin_core_power_op(phy_dev, power_level);
    if (power_level == SERDES_POWER_UP)
    {
        phy_serdes_polarity_config(phy_dev);
        phy_dsl_txfir_init(phy_dev);
    }

    return ret;
}

static int dsl_merlin16_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int rc;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    rc = merlin_speed_set(phy_dev, speed, duplex);

    phy_serdes->inited = 3;
    phy_serdes->print_log = 0;
    return rc;
}

