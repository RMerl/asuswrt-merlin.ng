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

#include "bcm63158_drivers_xport_ag.h"
#include "bcm63158_xport_xlmac_reg_ag.h"
int ag_drv_xport_xlmac_reg_dir_acc_data_write_set(uint32_t write_data)
{
    uint32_t reg_dir_acc_data_write=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dir_acc_data_write = RU_FIELD_SET(0, XPORT_XLMAC_REG, DIR_ACC_DATA_WRITE, WRITE_DATA, reg_dir_acc_data_write, write_data);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, DIR_ACC_DATA_WRITE, reg_dir_acc_data_write);

    return 0;
}

int ag_drv_xport_xlmac_reg_dir_acc_data_write_get(uint32_t *write_data)
{
    uint32_t reg_dir_acc_data_write=0;

#ifdef VALIDATE_PARMS
    if(!write_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, DIR_ACC_DATA_WRITE, reg_dir_acc_data_write);

    *write_data = RU_FIELD_GET(0, XPORT_XLMAC_REG, DIR_ACC_DATA_WRITE, WRITE_DATA, reg_dir_acc_data_write);

    return 0;
}

int ag_drv_xport_xlmac_reg_dir_acc_data_read_set(uint32_t read_data)
{
    uint32_t reg_dir_acc_data_read=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dir_acc_data_read = RU_FIELD_SET(0, XPORT_XLMAC_REG, DIR_ACC_DATA_READ, READ_DATA, reg_dir_acc_data_read, read_data);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, DIR_ACC_DATA_READ, reg_dir_acc_data_read);

    return 0;
}

int ag_drv_xport_xlmac_reg_dir_acc_data_read_get(uint32_t *read_data)
{
    uint32_t reg_dir_acc_data_read=0;

#ifdef VALIDATE_PARMS
    if(!read_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, DIR_ACC_DATA_READ, reg_dir_acc_data_read);

    *read_data = RU_FIELD_GET(0, XPORT_XLMAC_REG, DIR_ACC_DATA_READ, READ_DATA, reg_dir_acc_data_read);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_addr_0_set(const xport_xlmac_reg_indir_acc_addr_0 *indir_acc_addr_0)
{
    uint32_t reg_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((indir_acc_addr_0->err >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_0->start_busy >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_0->r_w >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_0->reg_port_id >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, ERR, reg_indir_acc_addr_0, indir_acc_addr_0->err);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, START_BUSY, reg_indir_acc_addr_0, indir_acc_addr_0->start_busy);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, R_W, reg_indir_acc_addr_0, indir_acc_addr_0->r_w);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, REG_PORT_ID, reg_indir_acc_addr_0, indir_acc_addr_0->reg_port_id);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, REG_OFFSET, reg_indir_acc_addr_0, indir_acc_addr_0->reg_offset);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, reg_indir_acc_addr_0);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_addr_0_get(xport_xlmac_reg_indir_acc_addr_0 *indir_acc_addr_0)
{
    uint32_t reg_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, reg_indir_acc_addr_0);

    indir_acc_addr_0->err = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, ERR, reg_indir_acc_addr_0);
    indir_acc_addr_0->start_busy = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, START_BUSY, reg_indir_acc_addr_0);
    indir_acc_addr_0->r_w = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, R_W, reg_indir_acc_addr_0);
    indir_acc_addr_0->reg_port_id = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, REG_PORT_ID, reg_indir_acc_addr_0);
    indir_acc_addr_0->reg_offset = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_0, REG_OFFSET, reg_indir_acc_addr_0);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_low_0_set(uint32_t data_low)
{
    uint32_t reg_indir_acc_data_low_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_low_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_0, DATA_LOW, reg_indir_acc_data_low_0, data_low);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_0, reg_indir_acc_data_low_0);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_low_0_get(uint32_t *data_low)
{
    uint32_t reg_indir_acc_data_low_0=0;

#ifdef VALIDATE_PARMS
    if(!data_low)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_0, reg_indir_acc_data_low_0);

    *data_low = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_0, DATA_LOW, reg_indir_acc_data_low_0);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_high_0_set(uint32_t data_high)
{
    uint32_t reg_indir_acc_data_high_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_high_0 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_0, DATA_HIGH, reg_indir_acc_data_high_0, data_high);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_0, reg_indir_acc_data_high_0);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_high_0_get(uint32_t *data_high)
{
    uint32_t reg_indir_acc_data_high_0=0;

#ifdef VALIDATE_PARMS
    if(!data_high)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_0, reg_indir_acc_data_high_0);

    *data_high = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_0, DATA_HIGH, reg_indir_acc_data_high_0);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_addr_1_set(const xport_xlmac_reg_indir_acc_addr_1 *indir_acc_addr_1)
{
    uint32_t reg_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((indir_acc_addr_1->err >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_1->start_busy >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_1->r_w >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_1->reg_port_id >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, ERR, reg_indir_acc_addr_1, indir_acc_addr_1->err);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, START_BUSY, reg_indir_acc_addr_1, indir_acc_addr_1->start_busy);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, R_W, reg_indir_acc_addr_1, indir_acc_addr_1->r_w);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, REG_PORT_ID, reg_indir_acc_addr_1, indir_acc_addr_1->reg_port_id);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, REG_OFFSET, reg_indir_acc_addr_1, indir_acc_addr_1->reg_offset);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, reg_indir_acc_addr_1);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_addr_1_get(xport_xlmac_reg_indir_acc_addr_1 *indir_acc_addr_1)
{
    uint32_t reg_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, reg_indir_acc_addr_1);

    indir_acc_addr_1->err = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, ERR, reg_indir_acc_addr_1);
    indir_acc_addr_1->start_busy = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, START_BUSY, reg_indir_acc_addr_1);
    indir_acc_addr_1->r_w = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, R_W, reg_indir_acc_addr_1);
    indir_acc_addr_1->reg_port_id = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, REG_PORT_ID, reg_indir_acc_addr_1);
    indir_acc_addr_1->reg_offset = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_ADDR_1, REG_OFFSET, reg_indir_acc_addr_1);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_low_1_set(uint32_t data_low)
{
    uint32_t reg_indir_acc_data_low_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_low_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_1, DATA_LOW, reg_indir_acc_data_low_1, data_low);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_1, reg_indir_acc_data_low_1);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_low_1_get(uint32_t *data_low)
{
    uint32_t reg_indir_acc_data_low_1=0;

#ifdef VALIDATE_PARMS
    if(!data_low)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_1, reg_indir_acc_data_low_1);

    *data_low = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_LOW_1, DATA_LOW, reg_indir_acc_data_low_1);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_high_1_set(uint32_t data_high)
{
    uint32_t reg_indir_acc_data_high_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_high_1 = RU_FIELD_SET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_1, DATA_HIGH, reg_indir_acc_data_high_1, data_high);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_1, reg_indir_acc_data_high_1);

    return 0;
}

int ag_drv_xport_xlmac_reg_indir_acc_data_high_1_get(uint32_t *data_high)
{
    uint32_t reg_indir_acc_data_high_1=0;

#ifdef VALIDATE_PARMS
    if(!data_high)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_1, reg_indir_acc_data_high_1);

    *data_high = RU_FIELD_GET(0, XPORT_XLMAC_REG, INDIR_ACC_DATA_HIGH_1, DATA_HIGH, reg_indir_acc_data_high_1);

    return 0;
}

int ag_drv_xport_xlmac_reg_config_set(const xport_xlmac_reg_config *config)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if(!config)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((config->xlmac_reset >= _1BITS_MAX_VAL_) ||
       (config->rx_dual_cycle_tdm_en >= _1BITS_MAX_VAL_) ||
       (config->rx_non_linear_quad_tdm_en >= _1BITS_MAX_VAL_) ||
       (config->rx_flex_tdm_enable >= _1BITS_MAX_VAL_) ||
       (config->mac_mode >= _3BITS_MAX_VAL_) ||
       (config->osts_timer_disable >= _1BITS_MAX_VAL_) ||
       (config->bypass_osts >= _1BITS_MAX_VAL_) ||
       (config->egr_1588_timestamping_mode >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, XLMAC_RESET, reg_config, config->xlmac_reset);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, RX_DUAL_CYCLE_TDM_EN, reg_config, config->rx_dual_cycle_tdm_en);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, RX_NON_LINEAR_QUAD_TDM_EN, reg_config, config->rx_non_linear_quad_tdm_en);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, RX_FLEX_TDM_ENABLE, reg_config, config->rx_flex_tdm_enable);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, MAC_MODE, reg_config, config->mac_mode);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, OSTS_TIMER_DISABLE, reg_config, config->osts_timer_disable);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, BYPASS_OSTS, reg_config, config->bypass_osts);
    reg_config = RU_FIELD_SET(0, XPORT_XLMAC_REG, CONFIG, EGR_1588_TIMESTAMPING_MODE, reg_config, config->egr_1588_timestamping_mode);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, CONFIG, reg_config);

    return 0;
}

int ag_drv_xport_xlmac_reg_config_get(xport_xlmac_reg_config *config)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if(!config)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, CONFIG, reg_config);

    config->xlmac_reset = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, XLMAC_RESET, reg_config);
    config->rx_dual_cycle_tdm_en = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, RX_DUAL_CYCLE_TDM_EN, reg_config);
    config->rx_non_linear_quad_tdm_en = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, RX_NON_LINEAR_QUAD_TDM_EN, reg_config);
    config->rx_flex_tdm_enable = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, RX_FLEX_TDM_ENABLE, reg_config);
    config->mac_mode = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, MAC_MODE, reg_config);
    config->osts_timer_disable = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, OSTS_TIMER_DISABLE, reg_config);
    config->bypass_osts = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, BYPASS_OSTS, reg_config);
    config->egr_1588_timestamping_mode = RU_FIELD_GET(0, XPORT_XLMAC_REG, CONFIG, EGR_1588_TIMESTAMPING_MODE, reg_config);

    return 0;
}

int ag_drv_xport_xlmac_reg_interrupt_check_set(uint8_t xlmac_intr_check)
{
    uint32_t reg_interrupt_check=0;

#ifdef VALIDATE_PARMS
    if((xlmac_intr_check >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_interrupt_check = RU_FIELD_SET(0, XPORT_XLMAC_REG, INTERRUPT_CHECK, XLMAC_INTR_CHECK, reg_interrupt_check, xlmac_intr_check);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, INTERRUPT_CHECK, reg_interrupt_check);

    return 0;
}

int ag_drv_xport_xlmac_reg_interrupt_check_get(uint8_t *xlmac_intr_check)
{
    uint32_t reg_interrupt_check=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_intr_check)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, INTERRUPT_CHECK, reg_interrupt_check);

    *xlmac_intr_check = RU_FIELD_GET(0, XPORT_XLMAC_REG, INTERRUPT_CHECK, XLMAC_INTR_CHECK, reg_interrupt_check);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_0_rxerr_mask_set(uint32_t rsv_err_mask)
{
    uint32_t reg_port_0_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if((rsv_err_mask >= _22BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_port_0_rxerr_mask = RU_FIELD_SET(0, XPORT_XLMAC_REG, PORT_0_RXERR_MASK, RSV_ERR_MASK, reg_port_0_rxerr_mask, rsv_err_mask);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, PORT_0_RXERR_MASK, reg_port_0_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_0_rxerr_mask_get(uint32_t *rsv_err_mask)
{
    uint32_t reg_port_0_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if(!rsv_err_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, PORT_0_RXERR_MASK, reg_port_0_rxerr_mask);

    *rsv_err_mask = RU_FIELD_GET(0, XPORT_XLMAC_REG, PORT_0_RXERR_MASK, RSV_ERR_MASK, reg_port_0_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_1_rxerr_mask_set(uint32_t rsv_err_mask)
{
    uint32_t reg_port_1_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if((rsv_err_mask >= _22BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_port_1_rxerr_mask = RU_FIELD_SET(0, XPORT_XLMAC_REG, PORT_1_RXERR_MASK, RSV_ERR_MASK, reg_port_1_rxerr_mask, rsv_err_mask);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, PORT_1_RXERR_MASK, reg_port_1_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_1_rxerr_mask_get(uint32_t *rsv_err_mask)
{
    uint32_t reg_port_1_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if(!rsv_err_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, PORT_1_RXERR_MASK, reg_port_1_rxerr_mask);

    *rsv_err_mask = RU_FIELD_GET(0, XPORT_XLMAC_REG, PORT_1_RXERR_MASK, RSV_ERR_MASK, reg_port_1_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_2_rxerr_mask_set(uint32_t rsv_err_mask)
{
    uint32_t reg_port_2_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if((rsv_err_mask >= _22BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_port_2_rxerr_mask = RU_FIELD_SET(0, XPORT_XLMAC_REG, PORT_2_RXERR_MASK, RSV_ERR_MASK, reg_port_2_rxerr_mask, rsv_err_mask);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, PORT_2_RXERR_MASK, reg_port_2_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_2_rxerr_mask_get(uint32_t *rsv_err_mask)
{
    uint32_t reg_port_2_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if(!rsv_err_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, PORT_2_RXERR_MASK, reg_port_2_rxerr_mask);

    *rsv_err_mask = RU_FIELD_GET(0, XPORT_XLMAC_REG, PORT_2_RXERR_MASK, RSV_ERR_MASK, reg_port_2_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_3_rxerr_mask_set(uint32_t rsv_err_mask)
{
    uint32_t reg_port_3_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if((rsv_err_mask >= _22BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_port_3_rxerr_mask = RU_FIELD_SET(0, XPORT_XLMAC_REG, PORT_3_RXERR_MASK, RSV_ERR_MASK, reg_port_3_rxerr_mask, rsv_err_mask);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, PORT_3_RXERR_MASK, reg_port_3_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_port_3_rxerr_mask_get(uint32_t *rsv_err_mask)
{
    uint32_t reg_port_3_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if(!rsv_err_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, PORT_3_RXERR_MASK, reg_port_3_rxerr_mask);

    *rsv_err_mask = RU_FIELD_GET(0, XPORT_XLMAC_REG, PORT_3_RXERR_MASK, RSV_ERR_MASK, reg_port_3_rxerr_mask);

    return 0;
}

int ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_set(const xport_xlmac_reg_rmt_lpbk_cntrl *rmt_lpbk_cntrl)
{
    uint32_t reg_rmt_lpbk_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!rmt_lpbk_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rmt_lpbk_cntrl->read_threshold >= _3BITS_MAX_VAL_) ||
       (rmt_lpbk_cntrl->tx_port_id >= _2BITS_MAX_VAL_) ||
       (rmt_lpbk_cntrl->tx_port_sel >= _1BITS_MAX_VAL_) ||
       (rmt_lpbk_cntrl->rxerr_en >= _1BITS_MAX_VAL_) ||
       (rmt_lpbk_cntrl->tx_crc_err >= _1BITS_MAX_VAL_) ||
       (rmt_lpbk_cntrl->tx_crc_mode >= _2BITS_MAX_VAL_) ||
       (rmt_lpbk_cntrl->rmt_loopback_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, READ_THRESHOLD, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->read_threshold);
    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_PORT_ID, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->tx_port_id);
    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_PORT_SEL, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->tx_port_sel);
    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, RXERR_EN, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->rxerr_en);
    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_CRC_ERR, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->tx_crc_err);
    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_CRC_MODE, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->tx_crc_mode);
    reg_rmt_lpbk_cntrl = RU_FIELD_SET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, RMT_LOOPBACK_EN, reg_rmt_lpbk_cntrl, rmt_lpbk_cntrl->rmt_loopback_en);

    RU_REG_WRITE(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, reg_rmt_lpbk_cntrl);

    return 0;
}

int ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_get(xport_xlmac_reg_rmt_lpbk_cntrl *rmt_lpbk_cntrl)
{
    uint32_t reg_rmt_lpbk_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!rmt_lpbk_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, reg_rmt_lpbk_cntrl);

    rmt_lpbk_cntrl->read_threshold = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, READ_THRESHOLD, reg_rmt_lpbk_cntrl);
    rmt_lpbk_cntrl->tx_port_id = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_PORT_ID, reg_rmt_lpbk_cntrl);
    rmt_lpbk_cntrl->tx_port_sel = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_PORT_SEL, reg_rmt_lpbk_cntrl);
    rmt_lpbk_cntrl->rxerr_en = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, RXERR_EN, reg_rmt_lpbk_cntrl);
    rmt_lpbk_cntrl->tx_crc_err = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_CRC_ERR, reg_rmt_lpbk_cntrl);
    rmt_lpbk_cntrl->tx_crc_mode = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, TX_CRC_MODE, reg_rmt_lpbk_cntrl);
    rmt_lpbk_cntrl->rmt_loopback_en = RU_FIELD_GET(0, XPORT_XLMAC_REG, RMT_LPBK_CNTRL, RMT_LOOPBACK_EN, reg_rmt_lpbk_cntrl);

    return 0;
}

