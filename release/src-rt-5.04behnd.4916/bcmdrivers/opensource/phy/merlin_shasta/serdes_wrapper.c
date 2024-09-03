/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#include <linux/delay.h>
#include "phy_drv.h"

#define srds_access_s phy_dev_s
#define PMD_DEV 1

#include "common/srds_api_err_code.h"
#include "merlin16_shasta_interface.h"

err_code_t merlin16_shasta_delay_ms(uint32_t delay_ms)
{
    mdelay(delay_ms);

    return ERR_CODE_NONE;
}

err_code_t merlin16_shasta_delay_us(uint32_t delay_us)
{
    udelay(delay_us);

    return ERR_CODE_NONE;
}

err_code_t merlin16_shasta_delay_ns(uint16_t delay_ns)
{
    ndelay(delay_ns);

    return ERR_CODE_NONE;
}

uint8_t merlin16_shasta_get_core(srds_access_t *sa__)
{
    return sa__->core_index;
}

uint8_t merlin16_shasta_get_lane(srds_access_t *sa__)
{
    return sa__->lane_index;
}

err_code_t merlin16_shasta_set_lane(srds_access_t *sa__, uint8_t lane_index)
{
    return ERR_CODE_NONE;
}

err_code_t merlin16_shasta_pmd_rdt_reg(srds_access_t *sa__, uint16_t address, uint16_t *val)
{
    return phy_dev_c45_read(sa__, PMD_DEV, address, val);
}

err_code_t merlin16_shasta_pmd_wr_reg(srds_access_t *sa__, uint16_t address, uint16_t val)
{
    return phy_dev_c45_write(sa__, PMD_DEV, address, val);
}

err_code_t merlin16_shasta_pmd_mwr_reg(srds_access_t *sa__, uint16_t address, uint16_t mask, uint8_t lsb, uint16_t val)
{
    return phy_dev_c45_write_mask(sa__, PMD_DEV, address, mask, lsb, val);
}

err_code_t merlin16_shasta_pmd_wr_pram(srds_access_t *sa__, uint8_t val)
{
    return 0;
}
