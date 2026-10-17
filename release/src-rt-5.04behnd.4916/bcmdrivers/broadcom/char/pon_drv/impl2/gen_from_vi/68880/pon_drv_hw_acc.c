/*
    Copyright 2024 Broadcom Corporation

    <:label-BRCM:2024:DUAL/GPL:standard

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
#include <linux/types.h>
#include <linux/bcm_log.h>

#include "bcmtypes.h"
#include "pon_drv_serdes_util.h"


#define WAN_TOP_BASE    0x80104000

uint32_t set_reg_field(uint32_t reg, uint32_t mask, uint32_t bits __attribute__((unused)), uint32_t shift, uint32_t val)
{
    reg = (reg & ~mask) | ((val << shift) & mask);
    return reg;
}
uint32_t epon_read32(char *description __attribute__((unused)) , uint32_t reg_abs_addr)
{
    uint32_t reg;
    uint32_t offset = reg_abs_addr - WAN_TOP_BASE;

    READ_32(offset, reg);

    return reg;
}
void epon_write32(char *description __attribute__((unused)), uint32_t reg_abs_addr __attribute__((unused)), uint32_t val __attribute__((unused)))
{
    uint32_t reg = val;
    uint32_t offset = reg_abs_addr - WAN_TOP_BASE;

    WRITE_32(offset, reg);
}