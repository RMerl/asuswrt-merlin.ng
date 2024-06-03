/*
   Copyright (c) 2015 Broadcom
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

#include "bcm4912_drivers_xport_ag.h"
#include "bcm4912_xport_intr_ag.h"
#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_xport_intr_cpu_status_set(uint8_t xlmac_id, const xport_intr_cpu_status *cpu_status)
{
    uint32_t reg_cpu_status=0;

#ifdef VALIDATE_PARMS
    if(!cpu_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cpu_status->link_down_intr >= _4BITS_MAX_VAL_) ||
       (cpu_status->link_up_intr >= _4BITS_MAX_VAL_) ||
       (cpu_status->tx_timesync_fifo_entry_valid_intr >= _4BITS_MAX_VAL_) ||
       (cpu_status->xlmac_intr >= _4BITS_MAX_VAL_) ||
       (cpu_status->mac_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (cpu_status->mab_status_intr >= _1BITS_MAX_VAL_) ||
       (cpu_status->mib_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (cpu_status->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, LINK_DOWN_INTR, reg_cpu_status, cpu_status->link_down_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, LINK_UP_INTR, reg_cpu_status, cpu_status->link_up_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_cpu_status, cpu_status->tx_timesync_fifo_entry_valid_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, XLMAC_INTR, reg_cpu_status, cpu_status->xlmac_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, MAC_REG_ERR_INTR, reg_cpu_status, cpu_status->mac_reg_err_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, MAB_STATUS_INTR, reg_cpu_status, cpu_status->mab_status_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, MIB_REG_ERR_INTR, reg_cpu_status, cpu_status->mib_reg_err_intr);
    reg_cpu_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_STATUS, UBUS_ERR_INTR, reg_cpu_status, cpu_status->ubus_err_intr);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, CPU_STATUS, reg_cpu_status);

    return 0;
}

int ag_drv_xport_intr_cpu_status_get(uint8_t xlmac_id, xport_intr_cpu_status *cpu_status)
{
    uint32_t reg_cpu_status=0;

#ifdef VALIDATE_PARMS
    if(!cpu_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, CPU_STATUS, reg_cpu_status);

    cpu_status->link_down_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, LINK_DOWN_INTR, reg_cpu_status);
    cpu_status->link_up_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, LINK_UP_INTR, reg_cpu_status);
    cpu_status->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_cpu_status);
    cpu_status->xlmac_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, XLMAC_INTR, reg_cpu_status);
    cpu_status->mac_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, MAC_REG_ERR_INTR, reg_cpu_status);
    cpu_status->mab_status_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, MAB_STATUS_INTR, reg_cpu_status);
    cpu_status->mib_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, MIB_REG_ERR_INTR, reg_cpu_status);
    cpu_status->ubus_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_STATUS, UBUS_ERR_INTR, reg_cpu_status);

    return 0;
}

int ag_drv_xport_intr_cpu_set_set(uint8_t xlmac_id, const xport_intr_cpu_set *cpu_set)
{
    uint32_t reg_cpu_set=0;

#ifdef VALIDATE_PARMS
    if(!cpu_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cpu_set->link_down_intr >= _4BITS_MAX_VAL_) ||
       (cpu_set->link_up_intr >= _4BITS_MAX_VAL_) ||
       (cpu_set->tx_timesync_fifo_entry_valid_intr >= _4BITS_MAX_VAL_) ||
       (cpu_set->xlmac_intr >= _4BITS_MAX_VAL_) ||
       (cpu_set->mac_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (cpu_set->mab_status_intr >= _1BITS_MAX_VAL_) ||
       (cpu_set->mib_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (cpu_set->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, LINK_DOWN_INTR, reg_cpu_set, cpu_set->link_down_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, LINK_UP_INTR, reg_cpu_set, cpu_set->link_up_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_cpu_set, cpu_set->tx_timesync_fifo_entry_valid_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, XLMAC_INTR, reg_cpu_set, cpu_set->xlmac_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, MAC_REG_ERR_INTR, reg_cpu_set, cpu_set->mac_reg_err_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, MAB_STATUS_INTR, reg_cpu_set, cpu_set->mab_status_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, MIB_REG_ERR_INTR, reg_cpu_set, cpu_set->mib_reg_err_intr);
    reg_cpu_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_SET, UBUS_ERR_INTR, reg_cpu_set, cpu_set->ubus_err_intr);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, CPU_SET, reg_cpu_set);

    return 0;
}

int ag_drv_xport_intr_cpu_set_get(uint8_t xlmac_id, xport_intr_cpu_set *cpu_set)
{
    uint32_t reg_cpu_set=0;

#ifdef VALIDATE_PARMS
    if(!cpu_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, CPU_SET, reg_cpu_set);

    cpu_set->link_down_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, LINK_DOWN_INTR, reg_cpu_set);
    cpu_set->link_up_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, LINK_UP_INTR, reg_cpu_set);
    cpu_set->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_cpu_set);
    cpu_set->xlmac_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, XLMAC_INTR, reg_cpu_set);
    cpu_set->mac_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, MAC_REG_ERR_INTR, reg_cpu_set);
    cpu_set->mab_status_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, MAB_STATUS_INTR, reg_cpu_set);
    cpu_set->mib_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, MIB_REG_ERR_INTR, reg_cpu_set);
    cpu_set->ubus_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_SET, UBUS_ERR_INTR, reg_cpu_set);

    return 0;
}

int ag_drv_xport_intr_cpu_clear_set(uint8_t xlmac_id, const xport_intr_cpu_clear *cpu_clear)
{
    uint32_t reg_cpu_clear=0;

#ifdef VALIDATE_PARMS
    if(!cpu_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cpu_clear->link_down_intr >= _4BITS_MAX_VAL_) ||
       (cpu_clear->link_up_intr >= _4BITS_MAX_VAL_) ||
       (cpu_clear->tx_timesync_fifo_entry_valid_intr >= _4BITS_MAX_VAL_) ||
       (cpu_clear->xlmac_intr >= _4BITS_MAX_VAL_) ||
       (cpu_clear->mac_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (cpu_clear->mab_status_intr >= _1BITS_MAX_VAL_) ||
       (cpu_clear->mib_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (cpu_clear->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, LINK_DOWN_INTR, reg_cpu_clear, cpu_clear->link_down_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, LINK_UP_INTR, reg_cpu_clear, cpu_clear->link_up_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_cpu_clear, cpu_clear->tx_timesync_fifo_entry_valid_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, XLMAC_INTR, reg_cpu_clear, cpu_clear->xlmac_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, MAC_REG_ERR_INTR, reg_cpu_clear, cpu_clear->mac_reg_err_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, MAB_STATUS_INTR, reg_cpu_clear, cpu_clear->mab_status_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, MIB_REG_ERR_INTR, reg_cpu_clear, cpu_clear->mib_reg_err_intr);
    reg_cpu_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_CLEAR, UBUS_ERR_INTR, reg_cpu_clear, cpu_clear->ubus_err_intr);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, CPU_CLEAR, reg_cpu_clear);

    return 0;
}

int ag_drv_xport_intr_cpu_clear_get(uint8_t xlmac_id, xport_intr_cpu_clear *cpu_clear)
{
    uint32_t reg_cpu_clear=0;

#ifdef VALIDATE_PARMS
    if(!cpu_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, CPU_CLEAR, reg_cpu_clear);

    cpu_clear->link_down_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, LINK_DOWN_INTR, reg_cpu_clear);
    cpu_clear->link_up_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, LINK_UP_INTR, reg_cpu_clear);
    cpu_clear->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_cpu_clear);
    cpu_clear->xlmac_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, XLMAC_INTR, reg_cpu_clear);
    cpu_clear->mac_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, MAC_REG_ERR_INTR, reg_cpu_clear);
    cpu_clear->mab_status_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, MAB_STATUS_INTR, reg_cpu_clear);
    cpu_clear->mib_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, MIB_REG_ERR_INTR, reg_cpu_clear);
    cpu_clear->ubus_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_CLEAR, UBUS_ERR_INTR, reg_cpu_clear);

    return 0;
}

int ag_drv_xport_intr_cpu_mask_status_set(uint8_t xlmac_id, const xport_intr_cpu_mask_status *cpu_mask_status)
{
    uint32_t reg_cpu_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!cpu_mask_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cpu_mask_status->link_down_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_status->link_up_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_status->tx_timesync_fifo_entry_valid_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_status->xlmac_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_status->mac_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_status->mab_status_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_status->mib_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_status->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, LINK_DOWN_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->link_down_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, LINK_UP_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->link_up_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->tx_timesync_fifo_entry_valid_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, XLMAC_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->xlmac_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, MAC_REG_ERR_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->mac_reg_err_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, MAB_STATUS_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->mab_status_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, MIB_REG_ERR_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->mib_reg_err_intr_mask);
    reg_cpu_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, UBUS_ERR_INTR_MASK, reg_cpu_mask_status, cpu_mask_status->ubus_err_intr_mask);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, reg_cpu_mask_status);

    return 0;
}

int ag_drv_xport_intr_cpu_mask_status_get(uint8_t xlmac_id, xport_intr_cpu_mask_status *cpu_mask_status)
{
    uint32_t reg_cpu_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!cpu_mask_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, reg_cpu_mask_status);

    cpu_mask_status->link_down_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, LINK_DOWN_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->link_up_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, LINK_UP_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->xlmac_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, XLMAC_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->mac_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, MAC_REG_ERR_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->mab_status_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, MAB_STATUS_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->mib_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, MIB_REG_ERR_INTR_MASK, reg_cpu_mask_status);
    cpu_mask_status->ubus_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_STATUS, UBUS_ERR_INTR_MASK, reg_cpu_mask_status);

    return 0;
}

int ag_drv_xport_intr_cpu_mask_set_set(uint8_t xlmac_id, const xport_intr_cpu_mask_set *cpu_mask_set)
{
    uint32_t reg_cpu_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!cpu_mask_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cpu_mask_set->link_down_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_set->link_up_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_set->tx_timesync_fifo_entry_valid_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_set->xlmac_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_set->mac_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_set->mab_status_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_set->mib_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_set->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, LINK_DOWN_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->link_down_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, LINK_UP_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->link_up_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->tx_timesync_fifo_entry_valid_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, XLMAC_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->xlmac_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, MAC_REG_ERR_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->mac_reg_err_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, MAB_STATUS_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->mab_status_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, MIB_REG_ERR_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->mib_reg_err_intr_mask);
    reg_cpu_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_SET, UBUS_ERR_INTR_MASK, reg_cpu_mask_set, cpu_mask_set->ubus_err_intr_mask);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, CPU_MASK_SET, reg_cpu_mask_set);

    return 0;
}

int ag_drv_xport_intr_cpu_mask_set_get(uint8_t xlmac_id, xport_intr_cpu_mask_set *cpu_mask_set)
{
    uint32_t reg_cpu_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!cpu_mask_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, CPU_MASK_SET, reg_cpu_mask_set);

    cpu_mask_set->link_down_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, LINK_DOWN_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->link_up_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, LINK_UP_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->xlmac_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, XLMAC_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->mac_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, MAC_REG_ERR_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->mab_status_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, MAB_STATUS_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->mib_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, MIB_REG_ERR_INTR_MASK, reg_cpu_mask_set);
    cpu_mask_set->ubus_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_SET, UBUS_ERR_INTR_MASK, reg_cpu_mask_set);

    return 0;
}

int ag_drv_xport_intr_cpu_mask_clear_set(uint8_t xlmac_id, const xport_intr_cpu_mask_clear *cpu_mask_clear)
{
    uint32_t reg_cpu_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!cpu_mask_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cpu_mask_clear->link_down_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_clear->link_up_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_clear->tx_timesync_fifo_entry_valid_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_clear->xlmac_intr_mask >= _4BITS_MAX_VAL_) ||
       (cpu_mask_clear->mac_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_clear->mab_status_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_clear->mib_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (cpu_mask_clear->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, LINK_DOWN_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->link_down_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, LINK_UP_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->link_up_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->tx_timesync_fifo_entry_valid_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, XLMAC_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->xlmac_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, MAC_REG_ERR_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->mac_reg_err_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, MAB_STATUS_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->mab_status_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, MIB_REG_ERR_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->mib_reg_err_intr_mask);
    reg_cpu_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, UBUS_ERR_INTR_MASK, reg_cpu_mask_clear, cpu_mask_clear->ubus_err_intr_mask);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, reg_cpu_mask_clear);

    return 0;
}

int ag_drv_xport_intr_cpu_mask_clear_get(uint8_t xlmac_id, xport_intr_cpu_mask_clear *cpu_mask_clear)
{
    uint32_t reg_cpu_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!cpu_mask_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, reg_cpu_mask_clear);

    cpu_mask_clear->link_down_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, LINK_DOWN_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->link_up_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, LINK_UP_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->xlmac_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, XLMAC_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->mac_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, MAC_REG_ERR_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->mab_status_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, MAB_STATUS_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->mib_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, MIB_REG_ERR_INTR_MASK, reg_cpu_mask_clear);
    cpu_mask_clear->ubus_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, CPU_MASK_CLEAR, UBUS_ERR_INTR_MASK, reg_cpu_mask_clear);

    return 0;
}

int ag_drv_xport_intr_pci_status_set(uint8_t xlmac_id, const xport_intr_pci_status *pci_status)
{
    uint32_t reg_pci_status=0;

#ifdef VALIDATE_PARMS
    if(!pci_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (pci_status->link_down_intr >= _4BITS_MAX_VAL_) ||
       (pci_status->link_up_intr >= _4BITS_MAX_VAL_) ||
       (pci_status->tx_timesync_fifo_entry_valid_intr >= _4BITS_MAX_VAL_) ||
       (pci_status->xlmac_intr >= _4BITS_MAX_VAL_) ||
       (pci_status->mac_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (pci_status->mab_status_intr >= _1BITS_MAX_VAL_) ||
       (pci_status->mib_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (pci_status->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, LINK_DOWN_INTR, reg_pci_status, pci_status->link_down_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, LINK_UP_INTR, reg_pci_status, pci_status->link_up_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_pci_status, pci_status->tx_timesync_fifo_entry_valid_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, XLMAC_INTR, reg_pci_status, pci_status->xlmac_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, MAC_REG_ERR_INTR, reg_pci_status, pci_status->mac_reg_err_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, MAB_STATUS_INTR, reg_pci_status, pci_status->mab_status_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, MIB_REG_ERR_INTR, reg_pci_status, pci_status->mib_reg_err_intr);
    reg_pci_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_STATUS, UBUS_ERR_INTR, reg_pci_status, pci_status->ubus_err_intr);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, PCI_STATUS, reg_pci_status);

    return 0;
}

int ag_drv_xport_intr_pci_status_get(uint8_t xlmac_id, xport_intr_pci_status *pci_status)
{
    uint32_t reg_pci_status=0;

#ifdef VALIDATE_PARMS
    if(!pci_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, PCI_STATUS, reg_pci_status);

    pci_status->link_down_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, LINK_DOWN_INTR, reg_pci_status);
    pci_status->link_up_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, LINK_UP_INTR, reg_pci_status);
    pci_status->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_pci_status);
    pci_status->xlmac_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, XLMAC_INTR, reg_pci_status);
    pci_status->mac_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, MAC_REG_ERR_INTR, reg_pci_status);
    pci_status->mab_status_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, MAB_STATUS_INTR, reg_pci_status);
    pci_status->mib_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, MIB_REG_ERR_INTR, reg_pci_status);
    pci_status->ubus_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_STATUS, UBUS_ERR_INTR, reg_pci_status);

    return 0;
}

int ag_drv_xport_intr_pci_set_set(uint8_t xlmac_id, const xport_intr_pci_set *pci_set)
{
    uint32_t reg_pci_set=0;

#ifdef VALIDATE_PARMS
    if(!pci_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (pci_set->link_down_intr >= _4BITS_MAX_VAL_) ||
       (pci_set->link_up_intr >= _4BITS_MAX_VAL_) ||
       (pci_set->tx_timesync_fifo_entry_valid_intr >= _4BITS_MAX_VAL_) ||
       (pci_set->xlmac_intr >= _4BITS_MAX_VAL_) ||
       (pci_set->mac_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (pci_set->mab_status_intr >= _1BITS_MAX_VAL_) ||
       (pci_set->mib_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (pci_set->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, LINK_DOWN_INTR, reg_pci_set, pci_set->link_down_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, LINK_UP_INTR, reg_pci_set, pci_set->link_up_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_pci_set, pci_set->tx_timesync_fifo_entry_valid_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, XLMAC_INTR, reg_pci_set, pci_set->xlmac_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, MAC_REG_ERR_INTR, reg_pci_set, pci_set->mac_reg_err_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, MAB_STATUS_INTR, reg_pci_set, pci_set->mab_status_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, MIB_REG_ERR_INTR, reg_pci_set, pci_set->mib_reg_err_intr);
    reg_pci_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_SET, UBUS_ERR_INTR, reg_pci_set, pci_set->ubus_err_intr);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, PCI_SET, reg_pci_set);

    return 0;
}

int ag_drv_xport_intr_pci_set_get(uint8_t xlmac_id, xport_intr_pci_set *pci_set)
{
    uint32_t reg_pci_set=0;

#ifdef VALIDATE_PARMS
    if(!pci_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, PCI_SET, reg_pci_set);

    pci_set->link_down_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, LINK_DOWN_INTR, reg_pci_set);
    pci_set->link_up_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, LINK_UP_INTR, reg_pci_set);
    pci_set->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_pci_set);
    pci_set->xlmac_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, XLMAC_INTR, reg_pci_set);
    pci_set->mac_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, MAC_REG_ERR_INTR, reg_pci_set);
    pci_set->mab_status_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, MAB_STATUS_INTR, reg_pci_set);
    pci_set->mib_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, MIB_REG_ERR_INTR, reg_pci_set);
    pci_set->ubus_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_SET, UBUS_ERR_INTR, reg_pci_set);

    return 0;
}

int ag_drv_xport_intr_pci_clear_set(uint8_t xlmac_id, const xport_intr_pci_clear *pci_clear)
{
    uint32_t reg_pci_clear=0;

#ifdef VALIDATE_PARMS
    if(!pci_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (pci_clear->link_down_intr >= _4BITS_MAX_VAL_) ||
       (pci_clear->link_up_intr >= _4BITS_MAX_VAL_) ||
       (pci_clear->tx_timesync_fifo_entry_valid_intr >= _4BITS_MAX_VAL_) ||
       (pci_clear->xlmac_intr >= _4BITS_MAX_VAL_) ||
       (pci_clear->mac_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (pci_clear->mab_status_intr >= _1BITS_MAX_VAL_) ||
       (pci_clear->mib_reg_err_intr >= _1BITS_MAX_VAL_) ||
       (pci_clear->ubus_err_intr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, LINK_DOWN_INTR, reg_pci_clear, pci_clear->link_down_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, LINK_UP_INTR, reg_pci_clear, pci_clear->link_up_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_pci_clear, pci_clear->tx_timesync_fifo_entry_valid_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, XLMAC_INTR, reg_pci_clear, pci_clear->xlmac_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, MAC_REG_ERR_INTR, reg_pci_clear, pci_clear->mac_reg_err_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, MAB_STATUS_INTR, reg_pci_clear, pci_clear->mab_status_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, MIB_REG_ERR_INTR, reg_pci_clear, pci_clear->mib_reg_err_intr);
    reg_pci_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_CLEAR, UBUS_ERR_INTR, reg_pci_clear, pci_clear->ubus_err_intr);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, PCI_CLEAR, reg_pci_clear);

    return 0;
}

int ag_drv_xport_intr_pci_clear_get(uint8_t xlmac_id, xport_intr_pci_clear *pci_clear)
{
    uint32_t reg_pci_clear=0;

#ifdef VALIDATE_PARMS
    if(!pci_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, PCI_CLEAR, reg_pci_clear);

    pci_clear->link_down_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, LINK_DOWN_INTR, reg_pci_clear);
    pci_clear->link_up_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, LINK_UP_INTR, reg_pci_clear);
    pci_clear->tx_timesync_fifo_entry_valid_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR, reg_pci_clear);
    pci_clear->xlmac_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, XLMAC_INTR, reg_pci_clear);
    pci_clear->mac_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, MAC_REG_ERR_INTR, reg_pci_clear);
    pci_clear->mab_status_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, MAB_STATUS_INTR, reg_pci_clear);
    pci_clear->mib_reg_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, MIB_REG_ERR_INTR, reg_pci_clear);
    pci_clear->ubus_err_intr = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_CLEAR, UBUS_ERR_INTR, reg_pci_clear);

    return 0;
}

int ag_drv_xport_intr_pci_mask_status_set(uint8_t xlmac_id, const xport_intr_pci_mask_status *pci_mask_status)
{
    uint32_t reg_pci_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!pci_mask_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (pci_mask_status->link_down_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_status->link_up_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_status->tx_timesync_fifo_entry_valid_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_status->xlmac_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_status->mac_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_status->mab_status_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_status->mib_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_status->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, LINK_DOWN_INTR_MASK, reg_pci_mask_status, pci_mask_status->link_down_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, LINK_UP_INTR_MASK, reg_pci_mask_status, pci_mask_status->link_up_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_pci_mask_status, pci_mask_status->tx_timesync_fifo_entry_valid_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, XLMAC_INTR_MASK, reg_pci_mask_status, pci_mask_status->xlmac_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, MAC_REG_ERR_INTR_MASK, reg_pci_mask_status, pci_mask_status->mac_reg_err_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, MAB_STATUS_INTR_MASK, reg_pci_mask_status, pci_mask_status->mab_status_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, MIB_REG_ERR_INTR_MASK, reg_pci_mask_status, pci_mask_status->mib_reg_err_intr_mask);
    reg_pci_mask_status = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, UBUS_ERR_INTR_MASK, reg_pci_mask_status, pci_mask_status->ubus_err_intr_mask);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, reg_pci_mask_status);

    return 0;
}

int ag_drv_xport_intr_pci_mask_status_get(uint8_t xlmac_id, xport_intr_pci_mask_status *pci_mask_status)
{
    uint32_t reg_pci_mask_status=0;

#ifdef VALIDATE_PARMS
    if(!pci_mask_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, reg_pci_mask_status);

    pci_mask_status->link_down_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, LINK_DOWN_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->link_up_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, LINK_UP_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->xlmac_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, XLMAC_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->mac_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, MAC_REG_ERR_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->mab_status_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, MAB_STATUS_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->mib_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, MIB_REG_ERR_INTR_MASK, reg_pci_mask_status);
    pci_mask_status->ubus_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_STATUS, UBUS_ERR_INTR_MASK, reg_pci_mask_status);

    return 0;
}

int ag_drv_xport_intr_pci_mask_set_set(uint8_t xlmac_id, const xport_intr_pci_mask_set *pci_mask_set)
{
    uint32_t reg_pci_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!pci_mask_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (pci_mask_set->link_down_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_set->link_up_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_set->tx_timesync_fifo_entry_valid_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_set->xlmac_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_set->mac_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_set->mab_status_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_set->mib_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_set->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, LINK_DOWN_INTR_MASK, reg_pci_mask_set, pci_mask_set->link_down_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, LINK_UP_INTR_MASK, reg_pci_mask_set, pci_mask_set->link_up_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_pci_mask_set, pci_mask_set->tx_timesync_fifo_entry_valid_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, XLMAC_INTR_MASK, reg_pci_mask_set, pci_mask_set->xlmac_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, MAC_REG_ERR_INTR_MASK, reg_pci_mask_set, pci_mask_set->mac_reg_err_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, MAB_STATUS_INTR_MASK, reg_pci_mask_set, pci_mask_set->mab_status_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, MIB_REG_ERR_INTR_MASK, reg_pci_mask_set, pci_mask_set->mib_reg_err_intr_mask);
    reg_pci_mask_set = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_SET, UBUS_ERR_INTR_MASK, reg_pci_mask_set, pci_mask_set->ubus_err_intr_mask);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, PCI_MASK_SET, reg_pci_mask_set);

    return 0;
}

int ag_drv_xport_intr_pci_mask_set_get(uint8_t xlmac_id, xport_intr_pci_mask_set *pci_mask_set)
{
    uint32_t reg_pci_mask_set=0;

#ifdef VALIDATE_PARMS
    if(!pci_mask_set)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, PCI_MASK_SET, reg_pci_mask_set);

    pci_mask_set->link_down_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, LINK_DOWN_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->link_up_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, LINK_UP_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->xlmac_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, XLMAC_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->mac_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, MAC_REG_ERR_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->mab_status_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, MAB_STATUS_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->mib_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, MIB_REG_ERR_INTR_MASK, reg_pci_mask_set);
    pci_mask_set->ubus_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_SET, UBUS_ERR_INTR_MASK, reg_pci_mask_set);

    return 0;
}

int ag_drv_xport_intr_pci_mask_clear_set(uint8_t xlmac_id, const xport_intr_pci_mask_clear *pci_mask_clear)
{
    uint32_t reg_pci_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!pci_mask_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (pci_mask_clear->link_down_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_clear->link_up_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_clear->tx_timesync_fifo_entry_valid_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_clear->xlmac_intr_mask >= _4BITS_MAX_VAL_) ||
       (pci_mask_clear->mac_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_clear->mab_status_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_clear->mib_reg_err_intr_mask >= _1BITS_MAX_VAL_) ||
       (pci_mask_clear->ubus_err_intr_mask >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, LINK_DOWN_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->link_down_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, LINK_UP_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->link_up_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->tx_timesync_fifo_entry_valid_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, XLMAC_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->xlmac_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, MAC_REG_ERR_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->mac_reg_err_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, MAB_STATUS_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->mab_status_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, MIB_REG_ERR_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->mib_reg_err_intr_mask);
    reg_pci_mask_clear = RU_FIELD_SET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, UBUS_ERR_INTR_MASK, reg_pci_mask_clear, pci_mask_clear->ubus_err_intr_mask);

    RU_REG_WRITE(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, reg_pci_mask_clear);

    return 0;
}

int ag_drv_xport_intr_pci_mask_clear_get(uint8_t xlmac_id, xport_intr_pci_mask_clear *pci_mask_clear)
{
    uint32_t reg_pci_mask_clear=0;

#ifdef VALIDATE_PARMS
    if(!pci_mask_clear)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, reg_pci_mask_clear);

    pci_mask_clear->link_down_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, LINK_DOWN_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->link_up_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, LINK_UP_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->tx_timesync_fifo_entry_valid_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->xlmac_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, XLMAC_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->mac_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, MAC_REG_ERR_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->mab_status_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, MAB_STATUS_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->mib_reg_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, MIB_REG_ERR_INTR_MASK, reg_pci_mask_clear);
    pci_mask_clear->ubus_err_intr_mask = RU_FIELD_GET(xlmac_id, XPORT_INTR, PCI_MASK_CLEAR, UBUS_ERR_INTR_MASK, reg_pci_mask_clear);

    return 0;
}

