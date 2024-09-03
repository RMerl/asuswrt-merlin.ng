/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************************/

#include "macsec_defs.h"
#include "macsec_dev.h"
#include "macsec_macros.h"
#include "macsec_common.h"

#include "bchp_regs_int.h"
#include "access_macros.h"

#define do_soc_read(o, v) \
{\
    READ_32(&virt_base[o], (v)); \
    /* printk("GET: 0x%px 0x%x\n", &virt_base[o], (uint32_t)(v)); */ \
} while(0)

#define do_soc_write(o, v) \
{\
    /* printk("SET: 0x%px 0x%x\n", &virt_base[o], v); */ \
    WRITE_32(&virt_base[o], (v)); \
} while(0)

int soc_ubus_reg_field_length(int unit, soc_ubus_reg_t reg, uint32_t fld)
{
    int i = 0;
    uint64_t mask = 0;

    mask = (reg->reg_flds[fld].fld_mask >> reg->reg_flds[fld].fld_shift);

    while (i < 64)
    {
        if (!(mask & 0x1))
            break;
        else
        {
            mask >>= 1;
            i++;
        }
    }
    return i;
}

int soc_ubus_reg_is_64(int unit, soc_ubus_reg_s *reg)
{
    if (reg->reg_size == 64)
        return 1;
    else
        return 0;
}

int soc_ubus_reg_field_set(int unit, soc_ubus_reg_t reg, uint32_t *rval, soc_ubus_field_t fld, uint32_t val)
{
    uint64_t fld_mask = reg->reg_flds[fld].fld_mask;
    uint32_t fld_shift = reg->reg_flds[fld].fld_shift;

    /* Clear field from read reg */
    *rval &= ~((uint32_t)fld_mask);

    /* Or in new field value */
    *rval |= ((val << fld_shift) & (uint32_t)fld_mask);

    return 0;
}

uint32_t soc_ubus_reg64_field32_set(int unit, soc_ubus_reg_t reg, uint64_t *rval64, soc_ubus_field_t fld, uint64_t val)
{
    uint64_t fld_mask = reg->reg_flds[fld].fld_mask;
    uint32_t fld_shift = reg->reg_flds[fld].fld_shift;

    /* Clear field from read reg */
    *rval64 &= ~(fld_mask);

    /* Or in new field value */
    *rval64 |= ((((uint64_t)val) << fld_shift) & fld_mask);

    return 0;
}

uint32_t soc_ubus_reg_field32_set(int unit, soc_ubus_reg_t reg, uint32_t *rval32, soc_ubus_field_t fld, uint32_t val)
{
    uint32_t fld_mask = reg->reg_flds[fld].fld_mask;
    uint32_t fld_shift = reg->reg_flds[fld].fld_shift;

    /* Clear field from read reg */
    *rval32 &= ~(fld_mask);

    /* Or in new field value */
    *rval32 |= ((((uint32_t)val) << fld_shift) & fld_mask);

    return 0;
}

uint32_t soc_ubus_reg32_field_get(int unit, soc_ubus_reg_t reg, uint32_t rval32, soc_ubus_field_t fld)
{
    uint32_t fld_mask = reg->reg_flds[fld].fld_mask;
    uint32_t fld_shift = reg->reg_flds[fld].fld_shift;
    uint32_t ret32 = 0;

    /* Clear field from read reg */
    ret32 = ((rval32 & fld_mask) >> fld_shift);

    return ret32;
}

uint64_t soc_ubus_reg64_field_get(int unit, soc_ubus_reg_t reg, uint64_t rval64, soc_ubus_field_t fld)
{
    uint64_t fld_mask = reg->reg_flds[fld].fld_mask;
    uint32_t fld_shift = reg->reg_flds[fld].fld_shift;
    uint64_t ret64 = 0;

    /* Clear field from read reg */
    ret64 = ((rval64 & fld_mask) >> fld_shift);

    return ret64;
}

uint32_t soc_ubus_reg_get(int unit, soc_ubus_reg_t reg, int port, uint64_t *rval)
{
    uint32_t reg_addr = (uint32_t)(reg->offset);
    uint32_t reg_data_hi = 0;
    uint32_t reg_data_lo = 0;
    int width_violation = 0;

    /* Adjust base addr to index for port nuber */
    if (port != REG_PORT_ANY)
        reg_addr += (0x1000 * port);

    if (reg->reg_size == 32)
    {
        PR_ERR("MACSEC_CTRL: WARNING - 64 bit reg read called for 32bit reg!\n");
        width_violation = 1;
    }

    do_soc_read(BCM_PHYS_REG_OFFSET(reg_addr), reg_data_lo);

    if (!width_violation)
        do_soc_read(BCM_PHYS_REG_OFFSET(BCHP_ETH_R2SBUS_BRIDGE_DIRECT_READ_DATA_HIGH), reg_data_hi);

    *rval = reg_data_hi;
    *rval = ((*rval << 32) | reg_data_lo);

    return 0;
}

uint32_t soc_ubus_reg_set(int unit, soc_ubus_reg_t reg, int port, uint64_t rval)
{
    uint32_t reg_addr = (uint32_t)(reg->offset);
    uint32_t reg_data_hi = (uint32_t)(rval >> 32);
    uint32_t reg_data_lo = (uint32_t)rval;
    int width_violation = 0;

    if (reg->reg_size == 32)
    {
        PR_ERR("MACSEC_CTRL: WARNING - 64 bit reg write called for 32bit reg! 0x%x\n", reg_addr);
        width_violation = 1;
    }
    
    /* Adjust base addr to index for port nuber */
    if (port != REG_PORT_ANY)
        reg_addr += (0x1000 * port);
    
    if (!width_violation)
        do_soc_write(BCM_PHYS_REG_OFFSET(BCHP_ETH_R2SBUS_BRIDGE_DIRECT_WRITE_DATA_HIGH), reg_data_hi);

    do_soc_write(BCM_PHYS_REG_OFFSET(reg_addr), reg_data_lo);

    return 0;
}

#define BCHP_ETH_XPORT_0_XPORT_XLMAC_WRAP_0_DIR_ACC_DATA_WRITE  0x837f3000
#define BCHP_ETH_XPORT_0_XPORT_XLMAC_WRAP_0_DIR_ACC_DATA_READ   0x837f3004
uint32_t soc_ubus_xlmac_reg_get(int unit, soc_ubus_reg_t reg, int port, uint64_t *rval)
{
    uint32_t reg_addr = (uint32_t)(reg->offset);
    uint32_t reg_data_hi = 0;
    uint32_t reg_data_lo = 0;
    int width_violation = 0;

    /* Adjust base addr to index for port nuber */
    if (port != REG_PORT_ANY)
        reg_addr += (0x400 * port);

    if (reg->reg_size == 32)
    {
        PR_ERR("MACSEC_CTRL: WARNING - 64 bit reg read called for 32bit reg!\n");
        width_violation = 1;
    }

    do_soc_read(BCM_PHYS_REG_OFFSET(reg_addr), reg_data_lo);

    if (!width_violation)
        do_soc_read(BCM_PHYS_REG_OFFSET(BCHP_ETH_XPORT_0_XPORT_XLMAC_WRAP_0_DIR_ACC_DATA_READ), reg_data_hi);

    *rval = reg_data_hi;
    *rval = ((*rval << 32) | reg_data_lo);

    return 0;
}

uint32_t soc_ubus_xlmac_reg_set(int unit, soc_ubus_reg_t reg, int port, uint64_t rval)
{
    uint32_t reg_addr = (uint32_t)(reg->offset);
    uint32_t reg_data_hi = (uint32_t)(rval >> 32);
    uint32_t reg_data_lo = (uint32_t)rval;
    int width_violation = 0;

    if (reg->reg_size == 32)
    {
        PR_ERR("MACSEC_CTRL: WARNING - 64 bit reg write called for 32bit reg! 0x%x\n", reg_addr);
        width_violation = 1;
    }
    
    /* Adjust base addr to index for port nuber */
    if (port != REG_PORT_ANY)
        reg_addr += (0x400 * port);
    
    if (!width_violation)
        do_soc_write(BCM_PHYS_REG_OFFSET(BCHP_ETH_XPORT_0_XPORT_XLMAC_WRAP_0_DIR_ACC_DATA_WRITE), reg_data_hi);

    do_soc_write(BCM_PHYS_REG_OFFSET(reg_addr), reg_data_lo);

    return 0;
}

uint32_t soc_ubus_reg32_get(int unit, soc_ubus_reg_t reg, int port, uint32_t *rval)
{
    uint32_t val, reg_addr = (uint32_t)(reg->offset);

    /* Adjust base addr to index for port nuber */
    if (port != REG_PORT_ANY)
        reg_addr += (0x1000 * port);

    do_soc_read(BCM_PHYS_REG_OFFSET(reg_addr), val);

    *rval = val;

    return 0;
}

uint32_t soc_ubus_reg32_set(int unit, soc_ubus_reg_t reg, int port, uint32_t rval)
{
    uint32_t reg_addr = (uint32_t)(reg->offset);

    /* Adjust base addr to index for port nuber */
    if (port != REG_PORT_ANY)
        reg_addr += (0x1000 * port);
   
    do_soc_write(BCM_PHYS_REG_OFFSET(reg_addr), rval);

    return 0;
}

int soc_ubus_xlmac_reg_fields32_modify(int unit, soc_ubus_reg_t reg, int port, soc_ubus_field_t *fld, uint32_t *rval)
{
    uint64_t fld_mask = reg->reg_flds[*fld].fld_mask;
    uint32_t fld_shift = reg->reg_flds[*fld].fld_shift;
    uint64_t reg_rd;

    soc_ubus_xlmac_reg_get(unit, reg, port, &reg_rd);
    /* Clear field from read reg */
    reg_rd &= ~(fld_mask);

    /* Or in new field value */
    reg_rd |= ((((uint64_t)*rval) << fld_shift) & fld_mask);
    
    soc_ubus_xlmac_reg_set(unit, reg, port, reg_rd);

    return 0;
}
