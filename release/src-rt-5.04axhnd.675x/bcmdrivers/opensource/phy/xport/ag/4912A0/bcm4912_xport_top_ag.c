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
#include "bcm4912_xport_top_ag.h"
#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_xport_top_ctrl_set(uint8_t xlmac_id, uint8_t p3_mode, uint8_t p2_mode, uint8_t p1_mode, uint8_t p0_mode)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (p3_mode >= _1BITS_MAX_VAL_) ||
       (p2_mode >= _1BITS_MAX_VAL_) ||
       (p1_mode >= _1BITS_MAX_VAL_) ||
       (p0_mode >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_control = RU_FIELD_SET(xlmac_id, XPORT_TOP, CONTROL, P3_MODE, reg_control, p3_mode);
    reg_control = RU_FIELD_SET(xlmac_id, XPORT_TOP, CONTROL, P2_MODE, reg_control, p2_mode);
    reg_control = RU_FIELD_SET(xlmac_id, XPORT_TOP, CONTROL, P1_MODE, reg_control, p1_mode);
    reg_control = RU_FIELD_SET(xlmac_id, XPORT_TOP, CONTROL, P0_MODE, reg_control, p0_mode);

    RU_REG_WRITE(xlmac_id, XPORT_TOP, CONTROL, reg_control);

    return 0;
}

int ag_drv_xport_top_ctrl_get(uint8_t xlmac_id, uint8_t *p3_mode, uint8_t *p2_mode, uint8_t *p1_mode, uint8_t *p0_mode)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if(!p3_mode || !p2_mode || !p1_mode || !p0_mode)
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

    RU_REG_READ(xlmac_id, XPORT_TOP, CONTROL, reg_control);

    *p3_mode = RU_FIELD_GET(xlmac_id, XPORT_TOP, CONTROL, P3_MODE, reg_control);
    *p2_mode = RU_FIELD_GET(xlmac_id, XPORT_TOP, CONTROL, P2_MODE, reg_control);
    *p1_mode = RU_FIELD_GET(xlmac_id, XPORT_TOP, CONTROL, P1_MODE, reg_control);
    *p0_mode = RU_FIELD_GET(xlmac_id, XPORT_TOP, CONTROL, P0_MODE, reg_control);

    return 0;
}

int ag_drv_xport_top_status_get(uint8_t xlmac_id, uint8_t *link_status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!link_status)
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

    RU_REG_READ(xlmac_id, XPORT_TOP, STATUS, reg_status);

    *link_status = RU_FIELD_GET(xlmac_id, XPORT_TOP, STATUS, LINK_STATUS, reg_status);

    return 0;
}

int ag_drv_xport_top_revision_get(uint8_t xlmac_id, uint32_t *xport_rev)
{
    uint32_t reg_revision=0;

#ifdef VALIDATE_PARMS
    if(!xport_rev)
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

    RU_REG_READ(xlmac_id, XPORT_TOP, REVISION, reg_revision);

    *xport_rev = RU_FIELD_GET(xlmac_id, XPORT_TOP, REVISION, XPORT_REV, reg_revision);

    return 0;
}

int ag_drv_xport_top_spare_cntrl_set(uint8_t xlmac_id, uint32_t spare_reg)
{
    uint32_t reg_spare_cntrl=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_spare_cntrl = RU_FIELD_SET(xlmac_id, XPORT_TOP, SPARE_CNTRL, SPARE_REG, reg_spare_cntrl, spare_reg);

    RU_REG_WRITE(xlmac_id, XPORT_TOP, SPARE_CNTRL, reg_spare_cntrl);

    return 0;
}

int ag_drv_xport_top_spare_cntrl_get(uint8_t xlmac_id, uint32_t *spare_reg)
{
    uint32_t reg_spare_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!spare_reg)
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

    RU_REG_READ(xlmac_id, XPORT_TOP, SPARE_CNTRL, reg_spare_cntrl);

    *spare_reg = RU_FIELD_GET(xlmac_id, XPORT_TOP, SPARE_CNTRL, SPARE_REG, reg_spare_cntrl);

    return 0;
}

