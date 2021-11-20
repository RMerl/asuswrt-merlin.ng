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

#include "bcm6858_drivers_lport_ag.h"
#include "bcm6858_lport_intr_ag.h"
int ag_drv_lport_intr_status_0_set(const lport_intr_status_0 *status_0)
{
    uint32_t reg_0_cpu_status=0;

#ifdef VALIDATE_PARMS
    if(!status_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((status_0->qgphy_energy_off_intr >= _4BITS_MAX_VAL_) ||
       (status_0->qgphy_energy_on_intr >= _4BITS_MAX_VAL_) ||
       (status_0->mdio_err_intr >= _1BITS_MAX_VAL_) ||
       (status_0->mdio_done_intr >= _1BITS_MAX_VAL_) ||
       (status_0->mib_reg_err_intr >= _2BITS_MAX_VAL_) ||
       (status_0->mac_reg_err_intr >= _2BITS_MAX_VAL_) ||
       (status_0->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, XLMAC_INTR, reg_0_cpu_status, status_0->xlmac_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_0_cpu_status, status_0->tx_timesync_fifo_entry_valid_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, QGPHY_ENERGY_OFF_INTR, reg_0_cpu_status, status_0->qgphy_energy_off_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, QGPHY_ENERGY_ON_INTR, reg_0_cpu_status, status_0->qgphy_energy_on_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, MDIO_ERR_INTR, reg_0_cpu_status, status_0->mdio_err_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, MDIO_DONE_INTR, reg_0_cpu_status, status_0->mdio_done_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, MIB_REG_ERR_INTR, reg_0_cpu_status, status_0->mib_reg_err_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, MAC_REG_ERR_INTR, reg_0_cpu_status, status_0->mac_reg_err_intr);
    reg_0_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_STATUS, UBUS_ERR_INTR, reg_0_cpu_status, status_0->ubus_err_intr);

    RU_REG_WRITE(0, LPORT_INTR, 0_CPU_STATUS, reg_0_cpu_status);

    return 0;
}

int ag_drv_lport_intr_status_0_get(lport_intr_status_0 *status_0)
{
    uint32_t reg_0_cpu_status=0;

#ifdef VALIDATE_PARMS
    if(!status_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 0_CPU_STATUS, reg_0_cpu_status);

    status_0->xlmac_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, XLMAC_INTR, reg_0_cpu_status);
    status_0->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_0_cpu_status);
    status_0->qgphy_energy_off_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, QGPHY_ENERGY_OFF_INTR, reg_0_cpu_status);
    status_0->qgphy_energy_on_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, QGPHY_ENERGY_ON_INTR, reg_0_cpu_status);
    status_0->mdio_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, MDIO_ERR_INTR, reg_0_cpu_status);
    status_0->mdio_done_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, MDIO_DONE_INTR, reg_0_cpu_status);
    status_0->mib_reg_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, MIB_REG_ERR_INTR, reg_0_cpu_status);
    status_0->mac_reg_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, MAC_REG_ERR_INTR, reg_0_cpu_status);
    status_0->ubus_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_STATUS, UBUS_ERR_INTR, reg_0_cpu_status);

    return 0;
}

int ag_drv_lport_intr_set_0_set(const lport_intr_set_0 *set_0)
{
    uint32_t reg_0_cpu_set=0;

#ifdef VALIDATE_PARMS
    if(!set_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((set_0->qgphy_energy_off_intr >= _4BITS_MAX_VAL_) ||
       (set_0->qgphy_energy_on_intr >= _4BITS_MAX_VAL_) ||
       (set_0->mdio_err_intr >= _1BITS_MAX_VAL_) ||
       (set_0->mdio_done_intr >= _1BITS_MAX_VAL_) ||
       (set_0->mib_reg_err_intr >= _2BITS_MAX_VAL_) ||
       (set_0->mac_reg_err_intr >= _2BITS_MAX_VAL_) ||
       (set_0->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, XLMAC_INTR, reg_0_cpu_set, set_0->xlmac_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_0_cpu_set, set_0->tx_timesync_fifo_entry_valid_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, QGPHY_ENERGY_OFF_INTR, reg_0_cpu_set, set_0->qgphy_energy_off_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, QGPHY_ENERGY_ON_INTR, reg_0_cpu_set, set_0->qgphy_energy_on_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, MDIO_ERR_INTR, reg_0_cpu_set, set_0->mdio_err_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, MDIO_DONE_INTR, reg_0_cpu_set, set_0->mdio_done_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, MIB_REG_ERR_INTR, reg_0_cpu_set, set_0->mib_reg_err_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, MAC_REG_ERR_INTR, reg_0_cpu_set, set_0->mac_reg_err_intr);
    reg_0_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_SET, UBUS_ERR_INTR, reg_0_cpu_set, set_0->ubus_err_intr);

    RU_REG_WRITE(0, LPORT_INTR, 0_CPU_SET, reg_0_cpu_set);

    return 0;
}

int ag_drv_lport_intr_set_0_get(lport_intr_set_0 *set_0)
{
    uint32_t reg_0_cpu_set=0;

#ifdef VALIDATE_PARMS
    if(!set_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 0_CPU_SET, reg_0_cpu_set);

    set_0->xlmac_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, XLMAC_INTR, reg_0_cpu_set);
    set_0->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_0_cpu_set);
    set_0->qgphy_energy_off_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, QGPHY_ENERGY_OFF_INTR, reg_0_cpu_set);
    set_0->qgphy_energy_on_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, QGPHY_ENERGY_ON_INTR, reg_0_cpu_set);
    set_0->mdio_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, MDIO_ERR_INTR, reg_0_cpu_set);
    set_0->mdio_done_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, MDIO_DONE_INTR, reg_0_cpu_set);
    set_0->mib_reg_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, MIB_REG_ERR_INTR, reg_0_cpu_set);
    set_0->mac_reg_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, MAC_REG_ERR_INTR, reg_0_cpu_set);
    set_0->ubus_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_SET, UBUS_ERR_INTR, reg_0_cpu_set);

    return 0;
}

int ag_drv_lport_intr_clear_0_set(const lport_intr_clear_0 *clear_0)
{
    uint32_t reg_0_cpu_clear=0;

#ifdef VALIDATE_PARMS
    if(!clear_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((clear_0->qgphy_energy_off_intr >= _4BITS_MAX_VAL_) ||
       (clear_0->qgphy_energy_on_intr >= _4BITS_MAX_VAL_) ||
       (clear_0->mdio_err_intr >= _1BITS_MAX_VAL_) ||
       (clear_0->mdio_done_intr >= _1BITS_MAX_VAL_) ||
       (clear_0->mib_reg_err_intr >= _2BITS_MAX_VAL_) ||
       (clear_0->mac_reg_err_intr >= _2BITS_MAX_VAL_) ||
       (clear_0->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, XLMAC_INTR, reg_0_cpu_clear, clear_0->xlmac_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_0_cpu_clear, clear_0->tx_timesync_fifo_entry_valid_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, QGPHY_ENERGY_OFF_INTR, reg_0_cpu_clear, clear_0->qgphy_energy_off_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, QGPHY_ENERGY_ON_INTR, reg_0_cpu_clear, clear_0->qgphy_energy_on_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, MDIO_ERR_INTR, reg_0_cpu_clear, clear_0->mdio_err_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, MDIO_DONE_INTR, reg_0_cpu_clear, clear_0->mdio_done_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, MIB_REG_ERR_INTR, reg_0_cpu_clear, clear_0->mib_reg_err_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, MAC_REG_ERR_INTR, reg_0_cpu_clear, clear_0->mac_reg_err_intr);
    reg_0_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_CLEAR, UBUS_ERR_INTR, reg_0_cpu_clear, clear_0->ubus_err_intr);

    RU_REG_WRITE(0, LPORT_INTR, 0_CPU_CLEAR, reg_0_cpu_clear);

    return 0;
}

int ag_drv_lport_intr_clear_0_get(lport_intr_clear_0 *clear_0)
{
    uint32_t reg_0_cpu_clear=0;

#ifdef VALIDATE_PARMS
    if(!clear_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 0_CPU_CLEAR, reg_0_cpu_clear);

    clear_0->xlmac_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, XLMAC_INTR, reg_0_cpu_clear);
    clear_0->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_0_cpu_clear);
    clear_0->qgphy_energy_off_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, QGPHY_ENERGY_OFF_INTR, reg_0_cpu_clear);
    clear_0->qgphy_energy_on_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, QGPHY_ENERGY_ON_INTR, reg_0_cpu_clear);
    clear_0->mdio_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, MDIO_ERR_INTR, reg_0_cpu_clear);
    clear_0->mdio_done_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, MDIO_DONE_INTR, reg_0_cpu_clear);
    clear_0->mib_reg_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, MIB_REG_ERR_INTR, reg_0_cpu_clear);
    clear_0->mac_reg_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, MAC_REG_ERR_INTR, reg_0_cpu_clear);
    clear_0->ubus_err_intr = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_CLEAR, UBUS_ERR_INTR, reg_0_cpu_clear);

    return 0;
}

int ag_drv_lport_intr_mask_status_0_set(const lport_intr_mask_status_0 *mask_status_0)
{
    uint32_t reg_0_cpu_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!mask_status_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((mask_status_0->mac_rx_cdc_single_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_0->mac_rx_cdc_double_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_0->mac_tx_cdc_single_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_0->mac_tx_cdc_double_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_0->qgphy_energy_off_intr_mask >= _4BITS_MAX_VAL_) ||
       (mask_status_0->qgphy_energy_on_intr_mask >= _4BITS_MAX_VAL_) ||
       (mask_status_0->mdio_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (mask_status_0->mdio_done_intr_mask >= _1BITS_MAX_VAL_) ||
       (mask_status_0->mib_reg_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_0->mac_reg_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_0->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_RX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mac_rx_cdc_single_bit_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_RX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mac_rx_cdc_double_bit_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_TX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mac_tx_cdc_single_bit_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_TX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mac_tx_cdc_double_bit_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->tx_timesync_fifo_entry_valid_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, QGPHY_ENERGY_OFF_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->qgphy_energy_off_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, QGPHY_ENERGY_ON_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->qgphy_energy_on_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MDIO_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mdio_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MDIO_DONE_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mdio_done_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MIB_REG_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mib_reg_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_REG_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->mac_reg_err_intr_mask);
    reg_0_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_STATUS, UBUS_ERR_INTR_MASK, reg_0_cpu_mask_status, mask_status_0->ubus_err_intr_mask);

    RU_REG_WRITE(0, LPORT_INTR, 0_CPU_MASK_STATUS, reg_0_cpu_mask_status);

    return 0;
}

int ag_drv_lport_intr_mask_status_0_get(lport_intr_mask_status_0 *mask_status_0)
{
    uint32_t reg_0_cpu_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!mask_status_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 0_CPU_MASK_STATUS, reg_0_cpu_mask_status);

    mask_status_0->mac_rx_cdc_single_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_RX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mac_rx_cdc_double_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_RX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mac_tx_cdc_single_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_TX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mac_tx_cdc_double_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_TX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->qgphy_energy_off_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, QGPHY_ENERGY_OFF_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->qgphy_energy_on_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, QGPHY_ENERGY_ON_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mdio_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MDIO_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mdio_done_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MDIO_DONE_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mib_reg_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MIB_REG_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->mac_reg_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, MAC_REG_ERR_INTR_MASK, reg_0_cpu_mask_status);
    mask_status_0->ubus_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_STATUS, UBUS_ERR_INTR_MASK, reg_0_cpu_mask_status);

    return 0;
}

int ag_drv_lport_intr_mask_set_0_set(const lport_intr_mask_set_0 *mask_set_0)
{
    uint32_t reg_0_cpu_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!mask_set_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((mask_set_0->mac_rx_cdc_single_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_0->mac_rx_cdc_double_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_0->mac_tx_cdc_single_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_0->mac_tx_cdc_double_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_0->qgphy_energy_off_intr_mask >= _4BITS_MAX_VAL_) ||
       (mask_set_0->qgphy_energy_on_intr_mask >= _4BITS_MAX_VAL_) ||
       (mask_set_0->mdio_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (mask_set_0->mdio_done_intr_mask >= _1BITS_MAX_VAL_) ||
       (mask_set_0->mib_reg_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_0->mac_reg_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_0->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_RX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mac_rx_cdc_single_bit_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_RX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mac_rx_cdc_double_bit_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_TX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mac_tx_cdc_single_bit_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_TX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mac_tx_cdc_double_bit_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->tx_timesync_fifo_entry_valid_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, QGPHY_ENERGY_OFF_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->qgphy_energy_off_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, QGPHY_ENERGY_ON_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->qgphy_energy_on_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MDIO_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mdio_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MDIO_DONE_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mdio_done_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MIB_REG_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mib_reg_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_REG_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->mac_reg_err_intr_mask);
    reg_0_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_SET, UBUS_ERR_INTR_MASK, reg_0_cpu_mask_set, mask_set_0->ubus_err_intr_mask);

    RU_REG_WRITE(0, LPORT_INTR, 0_CPU_MASK_SET, reg_0_cpu_mask_set);

    return 0;
}

int ag_drv_lport_intr_mask_set_0_get(lport_intr_mask_set_0 *mask_set_0)
{
    uint32_t reg_0_cpu_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!mask_set_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 0_CPU_MASK_SET, reg_0_cpu_mask_set);

    mask_set_0->mac_rx_cdc_single_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_RX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mac_rx_cdc_double_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_RX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mac_tx_cdc_single_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_TX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mac_tx_cdc_double_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_TX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->qgphy_energy_off_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, QGPHY_ENERGY_OFF_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->qgphy_energy_on_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, QGPHY_ENERGY_ON_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mdio_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MDIO_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mdio_done_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MDIO_DONE_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mib_reg_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MIB_REG_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->mac_reg_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, MAC_REG_ERR_INTR_MASK, reg_0_cpu_mask_set);
    mask_set_0->ubus_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_SET, UBUS_ERR_INTR_MASK, reg_0_cpu_mask_set);

    return 0;
}

int ag_drv_lport_intr_mask_clear_0_set(const lport_intr_mask_clear_0 *mask_clear_0)
{
    uint32_t reg_0_cpu_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!mask_clear_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((mask_clear_0->mac_rx_cdc_single_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_0->mac_rx_cdc_double_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_0->mac_tx_cdc_single_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_0->mac_tx_cdc_double_bit_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_0->qgphy_energy_off_intr_mask >= _4BITS_MAX_VAL_) ||
       (mask_clear_0->qgphy_energy_on_intr_mask >= _4BITS_MAX_VAL_) ||
       (mask_clear_0->mdio_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (mask_clear_0->mdio_done_intr_mask >= _1BITS_MAX_VAL_) ||
       (mask_clear_0->mib_reg_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_0->mac_reg_err_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_0->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_RX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mac_rx_cdc_single_bit_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_RX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mac_rx_cdc_double_bit_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_TX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mac_tx_cdc_single_bit_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_TX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mac_tx_cdc_double_bit_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->tx_timesync_fifo_entry_valid_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, QGPHY_ENERGY_OFF_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->qgphy_energy_off_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, QGPHY_ENERGY_ON_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->qgphy_energy_on_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MDIO_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mdio_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MDIO_DONE_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mdio_done_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MIB_REG_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mib_reg_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_REG_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->mac_reg_err_intr_mask);
    reg_0_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, UBUS_ERR_INTR_MASK, reg_0_cpu_mask_clear, mask_clear_0->ubus_err_intr_mask);

    RU_REG_WRITE(0, LPORT_INTR, 0_CPU_MASK_CLEAR, reg_0_cpu_mask_clear);

    return 0;
}

int ag_drv_lport_intr_mask_clear_0_get(lport_intr_mask_clear_0 *mask_clear_0)
{
    uint32_t reg_0_cpu_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!mask_clear_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 0_CPU_MASK_CLEAR, reg_0_cpu_mask_clear);

    mask_clear_0->mac_rx_cdc_single_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_RX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mac_rx_cdc_double_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_RX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mac_tx_cdc_single_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_TX_CDC_SINGLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mac_tx_cdc_double_bit_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_TX_CDC_DOUBLE_BIT_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->qgphy_energy_off_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, QGPHY_ENERGY_OFF_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->qgphy_energy_on_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, QGPHY_ENERGY_ON_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mdio_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MDIO_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mdio_done_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MDIO_DONE_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mib_reg_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MIB_REG_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->mac_reg_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, MAC_REG_ERR_INTR_MASK, reg_0_cpu_mask_clear);
    mask_clear_0->ubus_err_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 0_CPU_MASK_CLEAR, UBUS_ERR_INTR_MASK, reg_0_cpu_mask_clear);

    return 0;
}

int ag_drv_lport_intr_status_1_set(const lport_intr_status_1 *status_1)
{
    uint32_t reg_1_cpu_status=0;

#ifdef VALIDATE_PARMS
    if(!status_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((status_1->mab_status_intr >= _2BITS_MAX_VAL_) ||
       (status_1->rx_remote_fault_intr >= _2BITS_MAX_VAL_) ||
       (status_1->dserdes_sd_off_intr >= _4BITS_MAX_VAL_) ||
       (status_1->dserdes_sd_on_intr >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_1_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_STATUS, MAB_STATUS_INTR, reg_1_cpu_status, status_1->mab_status_intr);
    reg_1_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_STATUS, RX_REMOTE_FAULT_INTR, reg_1_cpu_status, status_1->rx_remote_fault_intr);
    reg_1_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_STATUS, DSERDES_SD_OFF_INTR, reg_1_cpu_status, status_1->dserdes_sd_off_intr);
    reg_1_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_STATUS, DSERDES_SD_ON_INTR, reg_1_cpu_status, status_1->dserdes_sd_on_intr);
    reg_1_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_STATUS, LINK_DOWN_INTR, reg_1_cpu_status, status_1->link_down_intr);
    reg_1_cpu_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_STATUS, LINK_UP_INTR, reg_1_cpu_status, status_1->link_up_intr);

    RU_REG_WRITE(0, LPORT_INTR, 1_CPU_STATUS, reg_1_cpu_status);

    return 0;
}

int ag_drv_lport_intr_status_1_get(lport_intr_status_1 *status_1)
{
    uint32_t reg_1_cpu_status=0;

#ifdef VALIDATE_PARMS
    if(!status_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 1_CPU_STATUS, reg_1_cpu_status);

    status_1->mab_status_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_STATUS, MAB_STATUS_INTR, reg_1_cpu_status);
    status_1->rx_remote_fault_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_STATUS, RX_REMOTE_FAULT_INTR, reg_1_cpu_status);
    status_1->dserdes_sd_off_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_STATUS, DSERDES_SD_OFF_INTR, reg_1_cpu_status);
    status_1->dserdes_sd_on_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_STATUS, DSERDES_SD_ON_INTR, reg_1_cpu_status);
    status_1->link_down_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_STATUS, LINK_DOWN_INTR, reg_1_cpu_status);
    status_1->link_up_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_STATUS, LINK_UP_INTR, reg_1_cpu_status);

    return 0;
}

int ag_drv_lport_intr_set_1_set(const lport_intr_set_1 *set_1)
{
    uint32_t reg_1_cpu_set=0;

#ifdef VALIDATE_PARMS
    if(!set_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((set_1->mab_status_intr >= _2BITS_MAX_VAL_) ||
       (set_1->rx_remote_fault_intr >= _2BITS_MAX_VAL_) ||
       (set_1->dserdes_sd_off_intr >= _4BITS_MAX_VAL_) ||
       (set_1->dserdes_sd_on_intr >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_1_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_SET, MAB_STATUS_INTR, reg_1_cpu_set, set_1->mab_status_intr);
    reg_1_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_SET, RX_REMOTE_FAULT_INTR, reg_1_cpu_set, set_1->rx_remote_fault_intr);
    reg_1_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_SET, DSERDES_SD_OFF_INTR, reg_1_cpu_set, set_1->dserdes_sd_off_intr);
    reg_1_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_SET, DSERDES_SD_ON_INTR, reg_1_cpu_set, set_1->dserdes_sd_on_intr);
    reg_1_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_SET, LINK_DOWN_INTR, reg_1_cpu_set, set_1->link_down_intr);
    reg_1_cpu_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_SET, LINK_UP_INTR, reg_1_cpu_set, set_1->link_up_intr);

    RU_REG_WRITE(0, LPORT_INTR, 1_CPU_SET, reg_1_cpu_set);

    return 0;
}

int ag_drv_lport_intr_set_1_get(lport_intr_set_1 *set_1)
{
    uint32_t reg_1_cpu_set=0;

#ifdef VALIDATE_PARMS
    if(!set_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 1_CPU_SET, reg_1_cpu_set);

    set_1->mab_status_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_SET, MAB_STATUS_INTR, reg_1_cpu_set);
    set_1->rx_remote_fault_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_SET, RX_REMOTE_FAULT_INTR, reg_1_cpu_set);
    set_1->dserdes_sd_off_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_SET, DSERDES_SD_OFF_INTR, reg_1_cpu_set);
    set_1->dserdes_sd_on_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_SET, DSERDES_SD_ON_INTR, reg_1_cpu_set);
    set_1->link_down_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_SET, LINK_DOWN_INTR, reg_1_cpu_set);
    set_1->link_up_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_SET, LINK_UP_INTR, reg_1_cpu_set);

    return 0;
}

int ag_drv_lport_intr_clear_1_set(const lport_intr_clear_1 *clear_1)
{
    uint32_t reg_1_cpu_clear=0;

#ifdef VALIDATE_PARMS
    if(!clear_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((clear_1->mab_status_intr >= _2BITS_MAX_VAL_) ||
       (clear_1->rx_remote_fault_intr >= _2BITS_MAX_VAL_) ||
       (clear_1->dserdes_sd_off_intr >= _4BITS_MAX_VAL_) ||
       (clear_1->dserdes_sd_on_intr >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_1_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_CLEAR, MAB_STATUS_INTR, reg_1_cpu_clear, clear_1->mab_status_intr);
    reg_1_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_CLEAR, RX_REMOTE_FAULT_INTR, reg_1_cpu_clear, clear_1->rx_remote_fault_intr);
    reg_1_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_CLEAR, DSERDES_SD_OFF_INTR, reg_1_cpu_clear, clear_1->dserdes_sd_off_intr);
    reg_1_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_CLEAR, DSERDES_SD_ON_INTR, reg_1_cpu_clear, clear_1->dserdes_sd_on_intr);
    reg_1_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_CLEAR, LINK_DOWN_INTR, reg_1_cpu_clear, clear_1->link_down_intr);
    reg_1_cpu_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_CLEAR, LINK_UP_INTR, reg_1_cpu_clear, clear_1->link_up_intr);

    RU_REG_WRITE(0, LPORT_INTR, 1_CPU_CLEAR, reg_1_cpu_clear);

    return 0;
}

int ag_drv_lport_intr_clear_1_get(lport_intr_clear_1 *clear_1)
{
    uint32_t reg_1_cpu_clear=0;

#ifdef VALIDATE_PARMS
    if(!clear_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 1_CPU_CLEAR, reg_1_cpu_clear);

    clear_1->mab_status_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_CLEAR, MAB_STATUS_INTR, reg_1_cpu_clear);
    clear_1->rx_remote_fault_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_CLEAR, RX_REMOTE_FAULT_INTR, reg_1_cpu_clear);
    clear_1->dserdes_sd_off_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_CLEAR, DSERDES_SD_OFF_INTR, reg_1_cpu_clear);
    clear_1->dserdes_sd_on_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_CLEAR, DSERDES_SD_ON_INTR, reg_1_cpu_clear);
    clear_1->link_down_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_CLEAR, LINK_DOWN_INTR, reg_1_cpu_clear);
    clear_1->link_up_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_CLEAR, LINK_UP_INTR, reg_1_cpu_clear);

    return 0;
}

int ag_drv_lport_intr_mask_status_1_set(const lport_intr_mask_status_1 *mask_status_1)
{
    uint32_t reg_1_cpu_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!mask_status_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((mask_status_1->rx_remote_fault_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_status_1->dserdes_sd_off_intr >= _4BITS_MAX_VAL_) ||
       (mask_status_1->dserdes_sd_on_intr >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_1_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_STATUS, RX_REMOTE_FAULT_INTR_MASK, reg_1_cpu_mask_status, mask_status_1->rx_remote_fault_intr_mask);
    reg_1_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_STATUS, DSERDES_SD_OFF_INTR, reg_1_cpu_mask_status, mask_status_1->dserdes_sd_off_intr);
    reg_1_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_STATUS, DSERDES_SD_ON_INTR, reg_1_cpu_mask_status, mask_status_1->dserdes_sd_on_intr);
    reg_1_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_STATUS, LINK_DOWN_INTR_MASK, reg_1_cpu_mask_status, mask_status_1->link_down_intr_mask);
    reg_1_cpu_mask_status = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_STATUS, LINK_UP_INTR_MASK, reg_1_cpu_mask_status, mask_status_1->link_up_intr_mask);

    RU_REG_WRITE(0, LPORT_INTR, 1_CPU_MASK_STATUS, reg_1_cpu_mask_status);

    return 0;
}

int ag_drv_lport_intr_mask_status_1_get(lport_intr_mask_status_1 *mask_status_1)
{
    uint32_t reg_1_cpu_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!mask_status_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 1_CPU_MASK_STATUS, reg_1_cpu_mask_status);

    mask_status_1->rx_remote_fault_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_STATUS, RX_REMOTE_FAULT_INTR_MASK, reg_1_cpu_mask_status);
    mask_status_1->dserdes_sd_off_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_STATUS, DSERDES_SD_OFF_INTR, reg_1_cpu_mask_status);
    mask_status_1->dserdes_sd_on_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_STATUS, DSERDES_SD_ON_INTR, reg_1_cpu_mask_status);
    mask_status_1->link_down_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_STATUS, LINK_DOWN_INTR_MASK, reg_1_cpu_mask_status);
    mask_status_1->link_up_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_STATUS, LINK_UP_INTR_MASK, reg_1_cpu_mask_status);

    return 0;
}

int ag_drv_lport_intr_mask_set_1_set(const lport_intr_mask_set_1 *mask_set_1)
{
    uint32_t reg_1_cpu_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!mask_set_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((mask_set_1->rx_remote_fault_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_set_1->dserdes_sd_off_intr >= _4BITS_MAX_VAL_) ||
       (mask_set_1->dserdes_sd_on_intr >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_1_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_SET, RX_REMOTE_FAULT_INTR_MASK, reg_1_cpu_mask_set, mask_set_1->rx_remote_fault_intr_mask);
    reg_1_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_SET, DSERDES_SD_OFF_INTR, reg_1_cpu_mask_set, mask_set_1->dserdes_sd_off_intr);
    reg_1_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_SET, DSERDES_SD_ON_INTR, reg_1_cpu_mask_set, mask_set_1->dserdes_sd_on_intr);
    reg_1_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_SET, LINK_DOWN_INTR_MASK, reg_1_cpu_mask_set, mask_set_1->link_down_intr_mask);
    reg_1_cpu_mask_set = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_SET, LINK_UP_INTR_MASK, reg_1_cpu_mask_set, mask_set_1->link_up_intr_mask);

    RU_REG_WRITE(0, LPORT_INTR, 1_CPU_MASK_SET, reg_1_cpu_mask_set);

    return 0;
}

int ag_drv_lport_intr_mask_set_1_get(lport_intr_mask_set_1 *mask_set_1)
{
    uint32_t reg_1_cpu_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!mask_set_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 1_CPU_MASK_SET, reg_1_cpu_mask_set);

    mask_set_1->rx_remote_fault_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_SET, RX_REMOTE_FAULT_INTR_MASK, reg_1_cpu_mask_set);
    mask_set_1->dserdes_sd_off_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_SET, DSERDES_SD_OFF_INTR, reg_1_cpu_mask_set);
    mask_set_1->dserdes_sd_on_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_SET, DSERDES_SD_ON_INTR, reg_1_cpu_mask_set);
    mask_set_1->link_down_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_SET, LINK_DOWN_INTR_MASK, reg_1_cpu_mask_set);
    mask_set_1->link_up_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_SET, LINK_UP_INTR_MASK, reg_1_cpu_mask_set);

    return 0;
}

int ag_drv_lport_intr_mask_clear_1_set(const lport_intr_mask_clear_1 *mask_clear_1)
{
    uint32_t reg_1_cpu_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!mask_clear_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((mask_clear_1->rx_remote_fault_intr_mask >= _2BITS_MAX_VAL_) ||
       (mask_clear_1->dserdes_sd_off_intr >= _4BITS_MAX_VAL_) ||
       (mask_clear_1->dserdes_sd_on_intr >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_1_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, RX_REMOTE_FAULT_INTR_MASK, reg_1_cpu_mask_clear, mask_clear_1->rx_remote_fault_intr_mask);
    reg_1_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, DSERDES_SD_OFF_INTR, reg_1_cpu_mask_clear, mask_clear_1->dserdes_sd_off_intr);
    reg_1_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, DSERDES_SD_ON_INTR, reg_1_cpu_mask_clear, mask_clear_1->dserdes_sd_on_intr);
    reg_1_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, LINK_DOWN_INTR_MASK, reg_1_cpu_mask_clear, mask_clear_1->link_down_intr_mask);
    reg_1_cpu_mask_clear = RU_FIELD_SET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, LINK_UP_INTR_MASK, reg_1_cpu_mask_clear, mask_clear_1->link_up_intr_mask);

    RU_REG_WRITE(0, LPORT_INTR, 1_CPU_MASK_CLEAR, reg_1_cpu_mask_clear);

    return 0;
}

int ag_drv_lport_intr_mask_clear_1_get(lport_intr_mask_clear_1 *mask_clear_1)
{
    uint32_t reg_1_cpu_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!mask_clear_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_INTR, 1_CPU_MASK_CLEAR, reg_1_cpu_mask_clear);

    mask_clear_1->rx_remote_fault_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, RX_REMOTE_FAULT_INTR_MASK, reg_1_cpu_mask_clear);
    mask_clear_1->dserdes_sd_off_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, DSERDES_SD_OFF_INTR, reg_1_cpu_mask_clear);
    mask_clear_1->dserdes_sd_on_intr = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, DSERDES_SD_ON_INTR, reg_1_cpu_mask_clear);
    mask_clear_1->link_down_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, LINK_DOWN_INTR_MASK, reg_1_cpu_mask_clear);
    mask_clear_1->link_up_intr_mask = RU_FIELD_GET(0, LPORT_INTR, 1_CPU_MASK_CLEAR, LINK_UP_INTR_MASK, reg_1_cpu_mask_clear);

    return 0;
}

