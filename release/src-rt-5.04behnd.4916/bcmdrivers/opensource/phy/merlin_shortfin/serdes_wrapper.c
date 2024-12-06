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

#include <linux/delay.h>
#include "phy_drv.h"

#define srds_access_s phy_dev_s
#define PMD_DEV 1

#include "common/srds_api_err_code.h"
#include "merlin16_shortfin_interface.h"

err_code_t merlin16_shortfin_delay_ms(uint32_t delay_ms)
{
    mdelay(delay_ms);

    return ERR_CODE_NONE;
}

err_code_t merlin16_shortfin_delay_us(uint32_t delay_us)
{
    udelay(delay_us);

    return ERR_CODE_NONE;
}

err_code_t merlin16_shortfin_delay_ns(uint16_t delay_ns)
{
    ndelay(delay_ns);

    return ERR_CODE_NONE;
}

uint8_t merlin16_shortfin_get_core(srds_access_t *sa__)
{
    return sa__->core_index;
}

uint8_t merlin16_shortfin_get_lane(srds_access_t *sa__)
{
    return sa__->lane_index;
}

err_code_t merlin16_shortfin_set_lane(srds_access_t *sa__, uint8_t lane_index)
{
    return ERR_CODE_NONE;
}

err_code_t merlin16_shortfin_pmd_rdt_reg(srds_access_t *sa__, uint16_t address, uint16_t *val)
{
    return phy_dev_c45_read(sa__, PMD_DEV, address, val);
}

err_code_t merlin16_shortfin_pmd_wr_reg(srds_access_t *sa__, uint16_t address, uint16_t val)
{
    return phy_dev_c45_write(sa__, PMD_DEV, address, val);
}

err_code_t merlin16_shortfin_pmd_mwr_reg(srds_access_t *sa__, uint16_t address, uint16_t mask, uint8_t lsb, uint16_t val)
{
    return phy_dev_c45_write_mask(sa__, PMD_DEV, address, mask, lsb, val);
}

err_code_t merlin16_shortfin_pmd_wr_pram(srds_access_t *sa__, uint8_t val)
{
    return 0;
}
