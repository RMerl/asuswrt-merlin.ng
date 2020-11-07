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

#include "drivers_common_ag.h"
#include "force_lbe_control_ag.h"
bdmf_error_t ag_drv_force_lbe_control_force_lbe_control_control_set(const force_lbe_control_force_lbe_control_control *force_lbe_control_control)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if(!force_lbe_control_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((force_lbe_control_control->cfg_force_lbe >= _1BITS_MAX_VAL_) ||
       (force_lbe_control_control->cfg_force_lbe_value >= _1BITS_MAX_VAL_) ||
       (force_lbe_control_control->cfg_force_lbe_oe >= _1BITS_MAX_VAL_) ||
       (force_lbe_control_control->cfg_force_lbe_oe_value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control = RU_FIELD_SET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE, reg_control, force_lbe_control_control->cfg_force_lbe);
    reg_control = RU_FIELD_SET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE_VALUE, reg_control, force_lbe_control_control->cfg_force_lbe_value);
    reg_control = RU_FIELD_SET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE_OE, reg_control, force_lbe_control_control->cfg_force_lbe_oe);
    reg_control = RU_FIELD_SET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE_OE_VALUE, reg_control, force_lbe_control_control->cfg_force_lbe_oe_value);

    RU_REG_WRITE(0, FORCE_LBE_CONTROL, CONTROL, reg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_force_lbe_control_force_lbe_control_control_get(force_lbe_control_force_lbe_control_control *force_lbe_control_control)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if(!force_lbe_control_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, FORCE_LBE_CONTROL, CONTROL, reg_control);

    force_lbe_control_control->cfg_force_lbe = RU_FIELD_GET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE, reg_control);
    force_lbe_control_control->cfg_force_lbe_value = RU_FIELD_GET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE_VALUE, reg_control);
    force_lbe_control_control->cfg_force_lbe_oe = RU_FIELD_GET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE_OE, reg_control);
    force_lbe_control_control->cfg_force_lbe_oe_value = RU_FIELD_GET(0, FORCE_LBE_CONTROL, CONTROL, CFG_FORCE_LBE_OE_VALUE, reg_control);

    return BDMF_ERR_OK;
}

