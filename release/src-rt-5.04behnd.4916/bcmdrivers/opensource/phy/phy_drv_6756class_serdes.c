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
#include "phy_drv_merlin28.h"
#include "serdes_access.h"
#include "bcm_otp.h"
#include "pmc/pmc_core_api.h"
#include "memory_access.h"

static void dsl_merlin28_serdes_link_stats(phy_dev_t *phy_dev);
static int dsl_merlin28_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int dsl_merlin28_light_detected(phy_dev_t *phy_dev)
{
#if 0
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->signal_detect_gpio == -1 || phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;
    return merlin28_ext_sigal_detected(phy_dev);
#endif

    return 1;
}

static phy_serdes_t serdes_6756class_core[] =
{
    {
        .flag = CORE_STRUCTURE,
    }
};

static int merlin_core_power_op(phy_dev_t *phy_dev, int power_level);
static phy_serdes_t serdes_6756class[1][2] =
{
    {
        {
            .phy_type = PHY_TYPE_6756CLASS_SERDES,
            .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
            .inter_phy_types = INTER_PHY_TYPES_S1K2KR5R_M,
            .link_stats = dsl_merlin28_serdes_link_stats,
            .config_speed = PHY_SPEED_AUTO,
            .power_mode = SERDES_BASIC_POWER_SAVING,
            .power_admin_on = 1,
            .speed_set = dsl_merlin28_speed_set,
            .power_set = merlin_core_power_op,
            .cur_power_level = -1,
            .light_detected = dsl_merlin28_light_detected,
            .flag = LANE_STRUCTURE,
            //.lbe_op = dsl_merlin28_lbe_op,
        },
        {
            .phy_type = PHY_TYPE_6756CLASS_SERDES,
            .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
            .inter_phy_types = INTER_PHY_TYPES_S1K2K_M,
            .link_stats = dsl_merlin28_serdes_link_stats,
            .config_speed = PHY_SPEED_AUTO,
            .power_mode = SERDES_BASIC_POWER_SAVING,
            .power_admin_on = 1,
            .speed_set = dsl_merlin28_speed_set,
            .power_set = merlin28_lane_power_op,
            .light_detected = dsl_merlin28_light_detected,
            .cur_power_level = -1,
            .flag = LANE_STRUCTURE,
            //.lbe_op = dsl_merlin28_lbe_op,
        },
    },
};

static int merlin_core_power_op(phy_dev_t *phy_dev, int power_level)
{
    int ret;

    ret = merlin28_lane_power_op(phy_dev, power_level);
    if (power_level == SERDES_POWER_UP)
        phy_serdes_polarity_config(phy_dev);

    return ret;
}

static int total_lanes;
int phy_drv_serdes6756_get_total_lanes(void)
{
    return total_lanes;
}

static void __iomem *serdes_rescal;
static void __iomem *serdes_base;

#define RESCAL_RESISTOR_VALUE_S     (2)
#define RESCAL_RESISTOR_VALUE_M     (0xf<<2)
#define RESCAL_OVERRIDE             (3<<0)
static int serdes_rescal_probe(dt_device_t *pdev)
{
    int ret;

    serdes_rescal = dt_dev_remap(pdev, 0);
    if (IS_ERR_OR_NULL(serdes_rescal))
    {
        ret = PTR_ERR(serdes_rescal);
        serdes_rescal = NULL;
        dev_err(&pdev->dev, "Missing eserdes_rescal entry\n");
        goto Exit;
    }

    serdes_base = dt_dev_remap(pdev, 1);
    if (IS_ERR_OR_NULL(serdes_base))
    {
        ret = PTR_ERR(serdes_base);
        serdes_base = NULL;
        dev_err(&pdev->dev, "Missing serdes_base  entry\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "serdes_rescal =0x%p\n", serdes_rescal);
    dev_dbg(&pdev->dev, "serdes_base =0x%p\n", serdes_base);
    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,serdes-rescal" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-serdes-rescal",
        .of_match_table = of_platform_table,
    },
    .probe = serdes_rescal_probe,
};
module_platform_driver(of_platform_driver);

static void sgmiiResCal(phy_dev_t *phy_dev)
{
    static int init = 0;
    volatile uint32_t *reg = (volatile uint32_t *)serdes_rescal;
    uint32_t val, reg_val;

    bcm_otp_is_rescal_enabled(&val);
    if (val || init || !serdes_rescal)
        return;
    init = 1;

    GetRCalSetting_1UM_VERT(&val);
    val &= 0xf;
    printk("Setting SGMII Calibration value to 0x%x\n", val);

    reg_val = *reg;
    reg_val &= ~RESCAL_RESISTOR_VALUE_M;
    reg_val |= (val << RESCAL_RESISTOR_VALUE_S)|RESCAL_OVERRIDE;

    *reg = reg_val;
}

phy_dev_t *phy_drv_serdes6756_get_phy_dev(int core_num, int lane_num)
{
    phy_serdes_t *phy_serdes = &serdes_6756class[core_num][lane_num];
    phy_dev_t *phy_dev = phy_serdes->phy_dev;
    return phy_dev;
}

#define XIB_CONTROL         0x200
#define XIB_CONTROL_RX_EN               (1<< 0)
#define XIB_CONTROL_TX_EN               (1<< 1)
#define XIB_CONTROL_LINK_DN_RST_EN      (1<< 4)

void serdes_access_read_raw(uint32_t reg, uint32_t *val)
{
    READ_32(serdes_base + reg, *val);
}

void serdes_access_write_raw(uint32_t reg, uint32_t val)
{
    WRITE_32(serdes_base + reg, val);
}

static int phy_drv_serdes_6756class_init_lock(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes, *_phy_serdes;
    static int xib_inited = 0;

    mutex_lock(&serdes_mutex);

    phy_serdes = _phy_serdes = phy_dev->priv;
    printknotice("=== Start of 5G Active Ethernet Initialization for core %d, lane %d ===", 
        phy_dev->core_index, phy_dev->lane_index);

    if (xib_inited == 0)
    {
        uint32_t val;
        serdes_access_read_raw(XIB_CONTROL, &val);
        val |= XIB_CONTROL_RX_EN | XIB_CONTROL_TX_EN | XIB_CONTROL_LINK_DN_RST_EN;
        serdes_access_write_raw(XIB_CONTROL, val);
        sgmiiResCal(phy_dev);
        xib_inited = 1;
    }

    phy_dsl_serdes_init(phy_dev);
    merlin28_serdes_init(phy_dev);
    phy_dsl_serdes_post_init(phy_dev);
    phy_serdes->inited = 3;

    printknotice("=== End of 5G Active Ethernet Initialization for core %d, lane %d ===", 
        phy_dev->core_index, phy_dev->lane_index);

    mutex_unlock(&serdes_mutex);

    return 0;
}

static int dsl_merlin28_xfi_polarity_set(phy_dev_t *phy_dev, int inverse, int tx_dir)
{
    int ret;
    uint16_t val;
    int reg;

    val = merlin28_pmi_read16(phy_dev->core_index, phy_dev->lane_index, 1, 0xd0e3);
    val &= ~1;
    val |= inverse? 1:0;
    reg = tx_dir? 0xd0e3: 0xd0d3;
    ret = merlin28_pmi_write16(phy_dev->core_index, phy_dev->lane_index, 1, reg, val, 0);
    return ret;
}

static int dsl_merlin28_xfi_tx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    return dsl_merlin28_xfi_polarity_set(phy_dev, inverse, 1);
}

static int dsl_merlin28_xfi_rx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    return dsl_merlin28_xfi_polarity_set(phy_dev, inverse, 0);
}

static int dsl_merlin28_priv_fun(phy_dev_t *phy_dev, int op_code, va_list ap)
{
    int reg;
    int val;
    uint32_t *valp;
    int ret = 0;

    if ((op_code == PHY_OP_RD_MDIO ||
            op_code == PHY_OP_WR_MDIO) && 
            (ret = dsl_serdes_check_power(phy_dev)))
        goto end;

    switch(op_code)
    {
        case PHY_OP_RD_MDIO:
            reg = va_arg(ap, int);
            valp = va_arg(ap, uint32_t *);
            *valp = merlin28_pmi_read16(phy_dev->core_index, phy_dev->lane_index, reg >> 16, reg & 0xffff);
            break;
        case PHY_OP_WR_MDIO:
            reg = va_arg(ap, int);
            val = va_arg(ap, int);
            ret = merlin28_pmi_write16(phy_dev->core_index, phy_dev->lane_index, reg >> 16, reg & 0xffff, (uint16_t)val, 0);
            break;
        default:
            ret = dsl_serdes_priv_fun(phy_dev, op_code, ap);
    }

end:
    return ret;
}

static int _phy_dsl_serdes_dev_add_lock(phy_dev_t *phy_dev)
{
    int rc;
    phy_serdes_t *phy_serdes;

    rc = phy_dsl_serdes_dev_add_lock(phy_dev);
    
    mutex_lock(&serdes_mutex);

    if (phy_dev->core_index > ARRAY_SIZE(serdes_6756class_core))
        BUG_CHECK("Core number exceeds prepared data structure.\n");

    phy_serdes = &serdes_6756class[phy_dev->core_index][phy_dev->lane_index];
    phy_serdes->priv = &serdes_6756class_core[phy_dev->core_index];
    phy_serdes->phy_dev = phy_dev;
    phy_serdes->handle = phy_dev->priv;
    phy_dev->priv = phy_serdes;
    phy_serdes->core_num = phy_dev->core_index;

    if (total_lanes <= phy_dev->lane_index)
        total_lanes = phy_dev->lane_index + 1;

    if (phy_serdes->inited)
        BUG_CHECK("****** Duplicated Serdes at address %d defined in board parameter.\n", phy_dev->addr);

    phy_serdes->inited = 1;

    mutex_unlock(&serdes_mutex);
    return rc;
}

int merlin_mptwo_wrapper_display_diag_data(phy_dev_t *phy_dev, uint32_t level);
static int _phy_diag_lock(phy_dev_t *phy_dev, int level)
{
    int ret;
    mutex_lock(&serdes_mutex);
    ret = merlin_mptwo_wrapper_display_diag_data(phy_dev, level);
    mutex_unlock(&serdes_mutex);
    return ret;
}

phy_drv_t phy_drv_serdes_6756class =
{
    .init = phy_drv_serdes_6756class_init_lock,
    .phy_type = PHY_TYPE_6756CLASS_SERDES,
    .read_status = phy_dsl_serdes_read_status_lock,
    .apd_get = dsl_serdes_apd_get,
    .apd_set = dsl_serdes_apd_set_lock,
    .speed_set = dsl_serdes_cfg_speed_set_lock,
    .caps_get = dsl_serdes_caps_get,
    .caps_set = dsl_serdes_caps_set_lock,
    .config_speed_get = dsl_serdes_speed_get_lock,
    .priv_fun = dsl_merlin28_priv_fun,
    .power_set = phy_dsl_serdes_power_set_lock,
    .power_get = phy_dsl_serdes_power_get,
    .inter_phy_types_get = dsl_serdes_inter_phy_types_get,
    .get_phy_name = dsl_serdes_get_phy_name,
    .dev_add = _phy_dsl_serdes_dev_add_lock,
    .dt_priv = phy_dsl_serdes_dt_priv,
    .leds_init = dsl_phy_leds_init,	
    .diag = _phy_diag_lock,
    .c45_read = serdes_access_read,
    .c45_write = serdes_access_write,
    .c45_read_mask = serdes_access_read_mask,
    .c45_write_mask = serdes_access_write_mask,
    .configured_inter_phy_speed_type_set = dsl_serdes_cfg_speed_mode_set_lock,
    .xfi_tx_polarity_inverse_set = dsl_merlin28_xfi_tx_polarity_set,
    .xfi_rx_polarity_inverse_set = dsl_merlin28_xfi_rx_polarity_set,
    .name = "5GAE",
};

static void dsl_merlin28_serdes_link_stats(phy_dev_t *phy_dev)
{
    merlin28_chk_lane_link_status(phy_dev);
}

static int dsl_merlin28_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    return merlin28_speed_set(phy_dev, speed, duplex);
}

