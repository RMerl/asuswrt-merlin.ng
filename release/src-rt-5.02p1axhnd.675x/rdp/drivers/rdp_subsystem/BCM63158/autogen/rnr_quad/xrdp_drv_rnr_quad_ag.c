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

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_rnr_quad_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_rnr_quad_parser_vid0_set(uint8_t quad_idx, uint16_t vid_0, bdmf_boolean vid_0_en)
{
    uint32_t reg_parser_core_configuration_vid_0_1=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_0 >= _12BITS_MAX_VAL_) ||
       (vid_0_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, reg_parser_core_configuration_vid_0_1);

    reg_parser_core_configuration_vid_0_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_0, reg_parser_core_configuration_vid_0_1, vid_0);
    reg_parser_core_configuration_vid_0_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_0_EN, reg_parser_core_configuration_vid_0_1, vid_0_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, reg_parser_core_configuration_vid_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid0_get(uint8_t quad_idx, uint16_t *vid_0, bdmf_boolean *vid_0_en)
{
    uint32_t reg_parser_core_configuration_vid_0_1;

#ifdef VALIDATE_PARMS
    if(!vid_0 || !vid_0_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, reg_parser_core_configuration_vid_0_1);

    *vid_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_0, reg_parser_core_configuration_vid_0_1);
    *vid_0_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_0_EN, reg_parser_core_configuration_vid_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid1_set(uint8_t quad_idx, uint16_t vid_1, bdmf_boolean vid_1_en)
{
    uint32_t reg_parser_core_configuration_vid_0_1=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_1 >= _12BITS_MAX_VAL_) ||
       (vid_1_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, reg_parser_core_configuration_vid_0_1);

    reg_parser_core_configuration_vid_0_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_1, reg_parser_core_configuration_vid_0_1, vid_1);
    reg_parser_core_configuration_vid_0_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_1_EN, reg_parser_core_configuration_vid_0_1, vid_1_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, reg_parser_core_configuration_vid_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid1_get(uint8_t quad_idx, uint16_t *vid_1, bdmf_boolean *vid_1_en)
{
    uint32_t reg_parser_core_configuration_vid_0_1;

#ifdef VALIDATE_PARMS
    if(!vid_1 || !vid_1_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, reg_parser_core_configuration_vid_0_1);

    *vid_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_1, reg_parser_core_configuration_vid_0_1);
    *vid_1_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1, VID_1_EN, reg_parser_core_configuration_vid_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid2_set(uint8_t quad_idx, uint16_t vid_2, bdmf_boolean vid_2_en)
{
    uint32_t reg_parser_core_configuration_vid_2_3=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_2 >= _12BITS_MAX_VAL_) ||
       (vid_2_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, reg_parser_core_configuration_vid_2_3);

    reg_parser_core_configuration_vid_2_3 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_2, reg_parser_core_configuration_vid_2_3, vid_2);
    reg_parser_core_configuration_vid_2_3 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_2_EN, reg_parser_core_configuration_vid_2_3, vid_2_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, reg_parser_core_configuration_vid_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid2_get(uint8_t quad_idx, uint16_t *vid_2, bdmf_boolean *vid_2_en)
{
    uint32_t reg_parser_core_configuration_vid_2_3;

#ifdef VALIDATE_PARMS
    if(!vid_2 || !vid_2_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, reg_parser_core_configuration_vid_2_3);

    *vid_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_2, reg_parser_core_configuration_vid_2_3);
    *vid_2_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_2_EN, reg_parser_core_configuration_vid_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid3_set(uint8_t quad_idx, uint16_t vid_3, bdmf_boolean vid_3_en)
{
    uint32_t reg_parser_core_configuration_vid_2_3=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_3 >= _12BITS_MAX_VAL_) ||
       (vid_3_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, reg_parser_core_configuration_vid_2_3);

    reg_parser_core_configuration_vid_2_3 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_3, reg_parser_core_configuration_vid_2_3, vid_3);
    reg_parser_core_configuration_vid_2_3 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_3_EN, reg_parser_core_configuration_vid_2_3, vid_3_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, reg_parser_core_configuration_vid_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid3_get(uint8_t quad_idx, uint16_t *vid_3, bdmf_boolean *vid_3_en)
{
    uint32_t reg_parser_core_configuration_vid_2_3;

#ifdef VALIDATE_PARMS
    if(!vid_3 || !vid_3_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, reg_parser_core_configuration_vid_2_3);

    *vid_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_3, reg_parser_core_configuration_vid_2_3);
    *vid_3_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3, VID_3_EN, reg_parser_core_configuration_vid_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid4_set(uint8_t quad_idx, uint16_t vid_4, bdmf_boolean vid_4_en)
{
    uint32_t reg_parser_core_configuration_vid_4_5=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_4 >= _12BITS_MAX_VAL_) ||
       (vid_4_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, reg_parser_core_configuration_vid_4_5);

    reg_parser_core_configuration_vid_4_5 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_4, reg_parser_core_configuration_vid_4_5, vid_4);
    reg_parser_core_configuration_vid_4_5 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_4_EN, reg_parser_core_configuration_vid_4_5, vid_4_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, reg_parser_core_configuration_vid_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid4_get(uint8_t quad_idx, uint16_t *vid_4, bdmf_boolean *vid_4_en)
{
    uint32_t reg_parser_core_configuration_vid_4_5;

#ifdef VALIDATE_PARMS
    if(!vid_4 || !vid_4_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, reg_parser_core_configuration_vid_4_5);

    *vid_4 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_4, reg_parser_core_configuration_vid_4_5);
    *vid_4_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_4_EN, reg_parser_core_configuration_vid_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid5_set(uint8_t quad_idx, uint16_t vid_5, bdmf_boolean vid_5_en)
{
    uint32_t reg_parser_core_configuration_vid_4_5=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_5 >= _12BITS_MAX_VAL_) ||
       (vid_5_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, reg_parser_core_configuration_vid_4_5);

    reg_parser_core_configuration_vid_4_5 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_5, reg_parser_core_configuration_vid_4_5, vid_5);
    reg_parser_core_configuration_vid_4_5 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_5_EN, reg_parser_core_configuration_vid_4_5, vid_5_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, reg_parser_core_configuration_vid_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid5_get(uint8_t quad_idx, uint16_t *vid_5, bdmf_boolean *vid_5_en)
{
    uint32_t reg_parser_core_configuration_vid_4_5;

#ifdef VALIDATE_PARMS
    if(!vid_5 || !vid_5_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, reg_parser_core_configuration_vid_4_5);

    *vid_5 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_5, reg_parser_core_configuration_vid_4_5);
    *vid_5_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5, VID_5_EN, reg_parser_core_configuration_vid_4_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid6_set(uint8_t quad_idx, uint16_t vid_6, bdmf_boolean vid_6_en)
{
    uint32_t reg_parser_core_configuration_vid_6_7=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_6 >= _12BITS_MAX_VAL_) ||
       (vid_6_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, reg_parser_core_configuration_vid_6_7);

    reg_parser_core_configuration_vid_6_7 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_6, reg_parser_core_configuration_vid_6_7, vid_6);
    reg_parser_core_configuration_vid_6_7 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_6_EN, reg_parser_core_configuration_vid_6_7, vid_6_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, reg_parser_core_configuration_vid_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid6_get(uint8_t quad_idx, uint16_t *vid_6, bdmf_boolean *vid_6_en)
{
    uint32_t reg_parser_core_configuration_vid_6_7;

#ifdef VALIDATE_PARMS
    if(!vid_6 || !vid_6_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, reg_parser_core_configuration_vid_6_7);

    *vid_6 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_6, reg_parser_core_configuration_vid_6_7);
    *vid_6_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_6_EN, reg_parser_core_configuration_vid_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid7_set(uint8_t quad_idx, uint16_t vid_7, bdmf_boolean vid_7_en)
{
    uint32_t reg_parser_core_configuration_vid_6_7=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (vid_7 >= _12BITS_MAX_VAL_) ||
       (vid_7_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, reg_parser_core_configuration_vid_6_7);

    reg_parser_core_configuration_vid_6_7 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_7, reg_parser_core_configuration_vid_6_7, vid_7);
    reg_parser_core_configuration_vid_6_7 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_7_EN, reg_parser_core_configuration_vid_6_7, vid_7_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, reg_parser_core_configuration_vid_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_vid7_get(uint8_t quad_idx, uint16_t *vid_7, bdmf_boolean *vid_7_en)
{
    uint32_t reg_parser_core_configuration_vid_6_7;

#ifdef VALIDATE_PARMS
    if(!vid_7 || !vid_7_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, reg_parser_core_configuration_vid_6_7);

    *vid_7 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_7, reg_parser_core_configuration_vid_6_7);
    *vid_7_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7, VID_7_EN, reg_parser_core_configuration_vid_6_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip0_set(uint8_t quad_idx, const rnr_quad_parser_ip0 *parser_ip0)
{
    uint32_t reg_parser_core_configuration_ip_filter0_cfg=0;
    uint32_t reg_parser_core_configuration_ip_filter0_mask_cfg=0;
    uint32_t reg_parser_core_configuration_ip_filters_cfg=0;

#ifdef VALIDATE_PARMS
    if(!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_ip0->ip_filter0_dip_en >= _1BITS_MAX_VAL_) ||
       (parser_ip0->ip_filter0_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, reg_parser_core_configuration_ip_filters_cfg);

    reg_parser_core_configuration_ip_filter0_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, IP_ADDRESS, reg_parser_core_configuration_ip_filter0_cfg, parser_ip0->ip_address);
    reg_parser_core_configuration_ip_filter0_mask_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, IP_ADDRESS_MASK, reg_parser_core_configuration_ip_filter0_mask_cfg, parser_ip0->ip_address_mask);
    reg_parser_core_configuration_ip_filters_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_DIP_EN, reg_parser_core_configuration_ip_filters_cfg, parser_ip0->ip_filter0_dip_en);
    reg_parser_core_configuration_ip_filters_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_VALID, reg_parser_core_configuration_ip_filters_cfg, parser_ip0->ip_filter0_valid);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, reg_parser_core_configuration_ip_filter0_cfg);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, reg_parser_core_configuration_ip_filter0_mask_cfg);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, reg_parser_core_configuration_ip_filters_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip0_get(uint8_t quad_idx, rnr_quad_parser_ip0 *parser_ip0)
{
    uint32_t reg_parser_core_configuration_ip_filter0_cfg;
    uint32_t reg_parser_core_configuration_ip_filter0_mask_cfg;
    uint32_t reg_parser_core_configuration_ip_filters_cfg;

#ifdef VALIDATE_PARMS
    if(!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, reg_parser_core_configuration_ip_filter0_cfg);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, reg_parser_core_configuration_ip_filter0_mask_cfg);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, reg_parser_core_configuration_ip_filters_cfg);

    parser_ip0->ip_address = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, IP_ADDRESS, reg_parser_core_configuration_ip_filter0_cfg);
    parser_ip0->ip_address_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, IP_ADDRESS_MASK, reg_parser_core_configuration_ip_filter0_mask_cfg);
    parser_ip0->ip_filter0_dip_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_DIP_EN, reg_parser_core_configuration_ip_filters_cfg);
    parser_ip0->ip_filter0_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_VALID, reg_parser_core_configuration_ip_filters_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip1_set(uint8_t quad_idx, const rnr_quad_parser_ip0 *parser_ip0)
{
    /* Identical to parser_ip0 */
    uint32_t reg_parser_core_configuration_ip_filter0_cfg=0;
    uint32_t reg_parser_core_configuration_ip_filter0_mask_cfg=0;
    uint32_t reg_parser_core_configuration_ip_filters_cfg=0;

#ifdef VALIDATE_PARMS
    if(!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_ip0->ip_filter0_dip_en >= _1BITS_MAX_VAL_) ||
       (parser_ip0->ip_filter0_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, reg_parser_core_configuration_ip_filters_cfg);

    reg_parser_core_configuration_ip_filter0_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, IP_ADDRESS, reg_parser_core_configuration_ip_filter0_cfg, parser_ip0->ip_address);
    reg_parser_core_configuration_ip_filter0_mask_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, IP_ADDRESS_MASK, reg_parser_core_configuration_ip_filter0_mask_cfg, parser_ip0->ip_address_mask);
    reg_parser_core_configuration_ip_filters_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_DIP_EN, reg_parser_core_configuration_ip_filters_cfg, parser_ip0->ip_filter0_dip_en);
    reg_parser_core_configuration_ip_filters_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_VALID, reg_parser_core_configuration_ip_filters_cfg, parser_ip0->ip_filter0_valid);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG, reg_parser_core_configuration_ip_filter0_cfg);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG, reg_parser_core_configuration_ip_filter0_mask_cfg);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, reg_parser_core_configuration_ip_filters_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip1_get(uint8_t quad_idx, rnr_quad_parser_ip0 *parser_ip0)
{
    /* Identical to parser_ip0 */
    uint32_t reg_parser_core_configuration_ip_filter0_cfg;
    uint32_t reg_parser_core_configuration_ip_filter0_mask_cfg;
    uint32_t reg_parser_core_configuration_ip_filters_cfg;

#ifdef VALIDATE_PARMS
    if(!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG, reg_parser_core_configuration_ip_filter0_cfg);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG, reg_parser_core_configuration_ip_filter0_mask_cfg);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, reg_parser_core_configuration_ip_filters_cfg);

    parser_ip0->ip_address = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG, IP_ADDRESS, reg_parser_core_configuration_ip_filter0_cfg);
    parser_ip0->ip_address_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG, IP_ADDRESS_MASK, reg_parser_core_configuration_ip_filter0_mask_cfg);
    parser_ip0->ip_filter0_dip_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_DIP_EN, reg_parser_core_configuration_ip_filters_cfg);
    parser_ip0->ip_filter0_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG, IP_FILTER0_VALID, reg_parser_core_configuration_ip_filters_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(uint8_t quad_idx, uint16_t hard_nest_profile)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (hard_nest_profile >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_hard_nest_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0, hard_nest_profile);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(uint8_t quad_idx, uint16_t *hard_nest_profile)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0;

#ifdef VALIDATE_PARMS
    if(!hard_nest_profile)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, reg_parser_core_configuration_qtag_hard_nest_0);

    *hard_nest_profile = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(uint8_t quad_idx, uint16_t hard_nest_profile)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_1=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (hard_nest_profile >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_hard_nest_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_1, hard_nest_profile);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, reg_parser_core_configuration_qtag_hard_nest_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(uint8_t quad_idx, uint16_t *hard_nest_profile)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_1;

#ifdef VALIDATE_PARMS
    if(!hard_nest_profile)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, reg_parser_core_configuration_qtag_hard_nest_1);

    *hard_nest_profile = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(uint8_t quad_idx, uint16_t hard_nest_profile)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_2=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (hard_nest_profile >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_hard_nest_2 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_2, hard_nest_profile);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, reg_parser_core_configuration_qtag_hard_nest_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(uint8_t quad_idx, uint16_t *hard_nest_profile)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_2;

#ifdef VALIDATE_PARMS
    if(!hard_nest_profile)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, reg_parser_core_configuration_qtag_hard_nest_2);

    *hard_nest_profile = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_0, uint8_t qtag_nest_1_profile_0)
{
    uint32_t reg_parser_core_configuration_qtag_nest=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (qtag_nest_0_profile_0 >= _3BITS_MAX_VAL_) ||
       (qtag_nest_1_profile_0 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_0, reg_parser_core_configuration_qtag_nest, qtag_nest_0_profile_0);
    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_1_PROFILE_0, reg_parser_core_configuration_qtag_nest, qtag_nest_1_profile_0);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_0, uint8_t *qtag_nest_1_profile_0)
{
    uint32_t reg_parser_core_configuration_qtag_nest;

#ifdef VALIDATE_PARMS
    if(!qtag_nest_0_profile_0 || !qtag_nest_1_profile_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    *qtag_nest_0_profile_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_0, reg_parser_core_configuration_qtag_nest);
    *qtag_nest_1_profile_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_1_PROFILE_0, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof1_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_1, uint8_t qtag_nest_1_profile_1)
{
    uint32_t reg_parser_core_configuration_qtag_nest=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (qtag_nest_0_profile_1 >= _3BITS_MAX_VAL_) ||
       (qtag_nest_1_profile_1 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_1, reg_parser_core_configuration_qtag_nest, qtag_nest_0_profile_1);
    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_1_PROFILE_1, reg_parser_core_configuration_qtag_nest, qtag_nest_1_profile_1);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof1_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_1, uint8_t *qtag_nest_1_profile_1)
{
    uint32_t reg_parser_core_configuration_qtag_nest;

#ifdef VALIDATE_PARMS
    if(!qtag_nest_0_profile_1 || !qtag_nest_1_profile_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    *qtag_nest_0_profile_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_1, reg_parser_core_configuration_qtag_nest);
    *qtag_nest_1_profile_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_1_PROFILE_1, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof2_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_2, uint8_t qtag_nest_1_profile_2)
{
    uint32_t reg_parser_core_configuration_qtag_nest=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (qtag_nest_0_profile_2 >= _3BITS_MAX_VAL_) ||
       (qtag_nest_1_profile_2 >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_2, reg_parser_core_configuration_qtag_nest, qtag_nest_0_profile_2);
    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_1_PROFILE_2, reg_parser_core_configuration_qtag_nest, qtag_nest_1_profile_2);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof2_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_2, uint8_t *qtag_nest_1_profile_2)
{
    uint32_t reg_parser_core_configuration_qtag_nest;

#ifdef VALIDATE_PARMS
    if(!qtag_nest_0_profile_2 || !qtag_nest_1_profile_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    *qtag_nest_0_profile_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_2, reg_parser_core_configuration_qtag_nest);
    *qtag_nest_1_profile_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_1_PROFILE_2, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_set(uint8_t quad_idx, uint8_t user_ip_prot_0)
{
    uint32_t reg_parser_core_configuration_user_ip_prot=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    reg_parser_core_configuration_user_ip_prot = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_0, reg_parser_core_configuration_user_ip_prot, user_ip_prot_0);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_get(uint8_t quad_idx, uint8_t *user_ip_prot_0)
{
    uint32_t reg_parser_core_configuration_user_ip_prot;

#ifdef VALIDATE_PARMS
    if(!user_ip_prot_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    *user_ip_prot_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_0, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol1_set(uint8_t quad_idx, uint8_t user_ip_prot_1)
{
    uint32_t reg_parser_core_configuration_user_ip_prot=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    reg_parser_core_configuration_user_ip_prot = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_1, reg_parser_core_configuration_user_ip_prot, user_ip_prot_1);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol1_get(uint8_t quad_idx, uint8_t *user_ip_prot_1)
{
    uint32_t reg_parser_core_configuration_user_ip_prot;

#ifdef VALIDATE_PARMS
    if(!user_ip_prot_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    *user_ip_prot_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_1, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol2_set(uint8_t quad_idx, uint8_t user_ip_prot_2)
{
    uint32_t reg_parser_core_configuration_user_ip_prot=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    reg_parser_core_configuration_user_ip_prot = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_2, reg_parser_core_configuration_user_ip_prot, user_ip_prot_2);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol2_get(uint8_t quad_idx, uint8_t *user_ip_prot_2)
{
    uint32_t reg_parser_core_configuration_user_ip_prot;

#ifdef VALIDATE_PARMS
    if(!user_ip_prot_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    *user_ip_prot_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_2, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol3_set(uint8_t quad_idx, uint8_t user_ip_prot_3)
{
    uint32_t reg_parser_core_configuration_user_ip_prot=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    reg_parser_core_configuration_user_ip_prot = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_3, reg_parser_core_configuration_user_ip_prot, user_ip_prot_3);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol3_get(uint8_t quad_idx, uint8_t *user_ip_prot_3)
{
    uint32_t reg_parser_core_configuration_user_ip_prot;

#ifdef VALIDATE_PARMS
    if(!user_ip_prot_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, reg_parser_core_configuration_user_ip_prot);

    *user_ip_prot_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT, USER_IP_PROT_3, reg_parser_core_configuration_user_ip_prot);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter_set(uint8_t quad_idx, const rnr_quad_parser_da_filter *parser_da_filter)
{
    uint32_t reg_parser_core_configuration_da_filt0_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt0_val_l=0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_h=0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_l=0;

#ifdef VALIDATE_PARMS
    if(!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt0_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt0_val_h, parser_da_filter->da_filt_msb);
    reg_parser_core_configuration_da_filt0_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt0_val_l, parser_da_filter->da_filt_lsb);
    reg_parser_core_configuration_da_filt0_mask_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, DA_FILT_MASK_MSB, reg_parser_core_configuration_da_filt0_mask_h, parser_da_filter->da_filt_mask_msb);
    reg_parser_core_configuration_da_filt0_mask_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, DA_FILT_MASK_L, reg_parser_core_configuration_da_filt0_mask_l, parser_da_filter->da_filt_mask_l);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, reg_parser_core_configuration_da_filt0_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, reg_parser_core_configuration_da_filt0_val_l);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, reg_parser_core_configuration_da_filt0_mask_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, reg_parser_core_configuration_da_filt0_mask_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter_get(uint8_t quad_idx, rnr_quad_parser_da_filter *parser_da_filter)
{
    uint32_t reg_parser_core_configuration_da_filt0_val_h;
    uint32_t reg_parser_core_configuration_da_filt0_val_l;
    uint32_t reg_parser_core_configuration_da_filt0_mask_h;
    uint32_t reg_parser_core_configuration_da_filt0_mask_l;

#ifdef VALIDATE_PARMS
    if(!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, reg_parser_core_configuration_da_filt0_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, reg_parser_core_configuration_da_filt0_val_l);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, reg_parser_core_configuration_da_filt0_mask_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, reg_parser_core_configuration_da_filt0_mask_l);

    parser_da_filter->da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt0_val_h);
    parser_da_filter->da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt0_val_l);
    parser_da_filter->da_filt_mask_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, DA_FILT_MASK_MSB, reg_parser_core_configuration_da_filt0_mask_h);
    parser_da_filter->da_filt_mask_l = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, DA_FILT_MASK_L, reg_parser_core_configuration_da_filt0_mask_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter1_set(uint8_t quad_idx, const rnr_quad_parser_da_filter *parser_da_filter)
{
    /* Identical to parser_da_filter */
    uint32_t reg_parser_core_configuration_da_filt0_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt0_val_l=0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_h=0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_l=0;

#ifdef VALIDATE_PARMS
    if(!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt0_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt0_val_h, parser_da_filter->da_filt_msb);
    reg_parser_core_configuration_da_filt0_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt0_val_l, parser_da_filter->da_filt_lsb);
    reg_parser_core_configuration_da_filt0_mask_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, DA_FILT_MASK_MSB, reg_parser_core_configuration_da_filt0_mask_h, parser_da_filter->da_filt_mask_msb);
    reg_parser_core_configuration_da_filt0_mask_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, DA_FILT_MASK_L, reg_parser_core_configuration_da_filt0_mask_l, parser_da_filter->da_filt_mask_l);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H, reg_parser_core_configuration_da_filt0_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L, reg_parser_core_configuration_da_filt0_val_l);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H, reg_parser_core_configuration_da_filt0_mask_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L, reg_parser_core_configuration_da_filt0_mask_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter1_get(uint8_t quad_idx, rnr_quad_parser_da_filter *parser_da_filter)
{
    /* Identical to parser_da_filter */
    uint32_t reg_parser_core_configuration_da_filt0_val_h;
    uint32_t reg_parser_core_configuration_da_filt0_val_l;
    uint32_t reg_parser_core_configuration_da_filt0_mask_h;
    uint32_t reg_parser_core_configuration_da_filt0_mask_l;

#ifdef VALIDATE_PARMS
    if(!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H, reg_parser_core_configuration_da_filt0_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L, reg_parser_core_configuration_da_filt0_val_l);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H, reg_parser_core_configuration_da_filt0_mask_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L, reg_parser_core_configuration_da_filt0_mask_l);

    parser_da_filter->da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt0_val_h);
    parser_da_filter->da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt0_val_l);
    parser_da_filter->da_filt_mask_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H, DA_FILT_MASK_MSB, reg_parser_core_configuration_da_filt0_mask_h);
    parser_da_filter->da_filt_mask_l = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L, DA_FILT_MASK_L, reg_parser_core_configuration_da_filt0_mask_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter2_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt2_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt2_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt2_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt2_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt2_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt2_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H, reg_parser_core_configuration_da_filt2_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L, reg_parser_core_configuration_da_filt2_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter2_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt2_val_h;
    uint32_t reg_parser_core_configuration_da_filt2_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H, reg_parser_core_configuration_da_filt2_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L, reg_parser_core_configuration_da_filt2_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt2_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt2_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter3_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt3_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt3_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt3_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt3_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt3_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt3_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H, reg_parser_core_configuration_da_filt3_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L, reg_parser_core_configuration_da_filt3_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter3_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt3_val_h;
    uint32_t reg_parser_core_configuration_da_filt3_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H, reg_parser_core_configuration_da_filt3_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L, reg_parser_core_configuration_da_filt3_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt3_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt3_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter4_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt4_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt4_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt4_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt4_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt4_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt4_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H, reg_parser_core_configuration_da_filt4_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L, reg_parser_core_configuration_da_filt4_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter4_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt4_val_h;
    uint32_t reg_parser_core_configuration_da_filt4_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H, reg_parser_core_configuration_da_filt4_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L, reg_parser_core_configuration_da_filt4_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt4_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt4_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter5_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt5_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt5_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt5_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt5_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt5_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt5_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H, reg_parser_core_configuration_da_filt5_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L, reg_parser_core_configuration_da_filt5_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter5_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt5_val_h;
    uint32_t reg_parser_core_configuration_da_filt5_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H, reg_parser_core_configuration_da_filt5_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L, reg_parser_core_configuration_da_filt5_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt5_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt5_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter6_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt6_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt6_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt6_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt6_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt6_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt6_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H, reg_parser_core_configuration_da_filt6_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L, reg_parser_core_configuration_da_filt6_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter6_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt6_val_h;
    uint32_t reg_parser_core_configuration_da_filt6_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H, reg_parser_core_configuration_da_filt6_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L, reg_parser_core_configuration_da_filt6_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt6_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt6_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter7_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt7_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt7_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt7_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt7_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt7_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt7_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H, reg_parser_core_configuration_da_filt7_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L, reg_parser_core_configuration_da_filt7_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter7_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt7_val_h;
    uint32_t reg_parser_core_configuration_da_filt7_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H, reg_parser_core_configuration_da_filt7_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L, reg_parser_core_configuration_da_filt7_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt7_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt7_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter8_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt8_val_h=0;
    uint32_t reg_parser_core_configuration_da_filt8_val_l=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt8_val_h = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt8_val_h, da_filt_msb);
    reg_parser_core_configuration_da_filt8_val_l = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt8_val_l, da_filt_lsb);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H, reg_parser_core_configuration_da_filt8_val_h);
    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L, reg_parser_core_configuration_da_filt8_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_da_filter8_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb)
{
    uint32_t reg_parser_core_configuration_da_filt8_val_h;
    uint32_t reg_parser_core_configuration_da_filt8_val_l;

#ifdef VALIDATE_PARMS
    if(!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H, reg_parser_core_configuration_da_filt8_val_h);
    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L, reg_parser_core_configuration_da_filt8_val_l);

    *da_filt_msb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H, DA_FILT_MSB, reg_parser_core_configuration_da_filt8_val_h);
    *da_filt_lsb = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L, DA_FILT_LSB, reg_parser_core_configuration_da_filt8_val_l);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_da_filter_valid_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid)
{
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0=0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (da_filter_valid->da_filt0_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt1_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt2_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt3_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt4_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt5_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt6_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt7_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt8_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT0_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt0_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT1_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt1_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT2_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt2_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT3_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt3_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT4_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt4_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT5_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt5_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT6_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt6_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT7_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt7_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT8_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt8_valid);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, reg_parser_core_configuration_da_filt_valid_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_da_filter_valid_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid)
{
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, reg_parser_core_configuration_da_filt_valid_cfg_0);

    da_filter_valid->da_filt0_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT0_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt1_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT1_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt2_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT2_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt3_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT3_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt4_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT4_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt5_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT5_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt6_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT6_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt7_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT7_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt8_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT8_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_exception_bits_set(uint8_t quad_idx, uint16_t exception_en)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (exception_en >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    reg_parser_core_configuration_parser_misc_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, EXCEPTION_EN, reg_parser_core_configuration_parser_misc_cfg, exception_en);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_exception_bits_get(uint8_t quad_idx, uint16_t *exception_en)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg;

#ifdef VALIDATE_PARMS
    if(!exception_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    *exception_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, EXCEPTION_EN, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_tcp_flags_set(uint8_t quad_idx, uint8_t tcp_flags_filt)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    reg_parser_core_configuration_parser_misc_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, TCP_FLAGS_FILT, reg_parser_core_configuration_parser_misc_cfg, tcp_flags_filt);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_tcp_flags_get(uint8_t quad_idx, uint8_t *tcp_flags_filt)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg;

#ifdef VALIDATE_PARMS
    if(!tcp_flags_filt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    *tcp_flags_filt = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, TCP_FLAGS_FILT, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_profile_us_set(uint8_t quad_idx, uint8_t profile_us)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (profile_us >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    reg_parser_core_configuration_parser_misc_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, PROFILE_US, reg_parser_core_configuration_parser_misc_cfg, profile_us);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_profile_us_get(uint8_t quad_idx, uint8_t *profile_us)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg;

#ifdef VALIDATE_PARMS
    if(!profile_us)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    *profile_us = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, PROFILE_US, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_set(uint8_t quad_idx, const rnr_quad_parser_snap_conf *parser_snap_conf)
{
    uint32_t reg_parser_core_configuration_snap_org_code=0;

#ifdef VALIDATE_PARMS
    if(!parser_snap_conf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_snap_conf->code >= _24BITS_MAX_VAL_) ||
       (parser_snap_conf->en_rfc1042 >= _1BITS_MAX_VAL_) ||
       (parser_snap_conf->en_8021q >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_snap_org_code = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, CODE, reg_parser_core_configuration_snap_org_code, parser_snap_conf->code);
    reg_parser_core_configuration_snap_org_code = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, EN_RFC1042, reg_parser_core_configuration_snap_org_code, parser_snap_conf->en_rfc1042);
    reg_parser_core_configuration_snap_org_code = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, EN_8021Q, reg_parser_core_configuration_snap_org_code, parser_snap_conf->en_8021q);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, reg_parser_core_configuration_snap_org_code);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_get(uint8_t quad_idx, rnr_quad_parser_snap_conf *parser_snap_conf)
{
    uint32_t reg_parser_core_configuration_snap_org_code;

#ifdef VALIDATE_PARMS
    if(!parser_snap_conf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, reg_parser_core_configuration_snap_org_code);

    parser_snap_conf->code = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, CODE, reg_parser_core_configuration_snap_org_code);
    parser_snap_conf->en_rfc1042 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, EN_RFC1042, reg_parser_core_configuration_snap_org_code);
    parser_snap_conf->en_8021q = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE, EN_8021Q, reg_parser_core_configuration_snap_org_code);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ipv6_filter_set(uint8_t quad_idx, const rnr_quad_parser_ipv6_filter *parser_ipv6_filter)
{
    uint32_t reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg=0;

#ifdef VALIDATE_PARMS
    if(!parser_ipv6_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_ipv6_filter->hop_by_hop_match >= _1BITS_MAX_VAL_) ||
       (parser_ipv6_filter->routing_eh >= _1BITS_MAX_VAL_) ||
       (parser_ipv6_filter->dest_opt_eh >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, HOP_BY_HOP_MATCH, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg, parser_ipv6_filter->hop_by_hop_match);
    reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, ROUTING_EH, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg, parser_ipv6_filter->routing_eh);
    reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, DEST_OPT_EH, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg, parser_ipv6_filter->dest_opt_eh);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ipv6_filter_get(uint8_t quad_idx, rnr_quad_parser_ipv6_filter *parser_ipv6_filter)
{
    uint32_t reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg;

#ifdef VALIDATE_PARMS
    if(!parser_ipv6_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg);

    parser_ipv6_filter->hop_by_hop_match = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, HOP_BY_HOP_MATCH, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg);
    parser_ipv6_filter->routing_eh = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, ROUTING_EH, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg);
    parser_ipv6_filter->dest_opt_eh = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, DEST_OPT_EH, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_eng_set(uint8_t quad_idx, uint32_t cfg)
{
    uint32_t reg_parser_core_configuration_eng=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_eng = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ENG, CFG, reg_parser_core_configuration_eng, cfg);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ENG, reg_parser_core_configuration_eng);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_eng_get(uint8_t quad_idx, uint32_t *cfg)
{
    uint32_t reg_parser_core_configuration_eng;

#ifdef VALIDATE_PARMS
    if(!cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ENG, reg_parser_core_configuration_eng);

    *cfg = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ENG, CFG, reg_parser_core_configuration_eng);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(uint8_t quad_idx, uint16_t ppp_code_0, uint16_t ppp_code_1)
{
    uint32_t reg_parser_core_configuration_ppp_ip_prot_code=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_ppp_ip_prot_code = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, PPP_CODE_0, reg_parser_core_configuration_ppp_ip_prot_code, ppp_code_0);
    reg_parser_core_configuration_ppp_ip_prot_code = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, PPP_CODE_1, reg_parser_core_configuration_ppp_ip_prot_code, ppp_code_1);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, reg_parser_core_configuration_ppp_ip_prot_code);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(uint8_t quad_idx, uint16_t *ppp_code_0, uint16_t *ppp_code_1)
{
    uint32_t reg_parser_core_configuration_ppp_ip_prot_code;

#ifdef VALIDATE_PARMS
    if(!ppp_code_0 || !ppp_code_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, reg_parser_core_configuration_ppp_ip_prot_code);

    *ppp_code_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, PPP_CODE_0, reg_parser_core_configuration_ppp_ip_prot_code);
    *ppp_code_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE, PPP_CODE_1, reg_parser_core_configuration_ppp_ip_prot_code);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(uint8_t quad_idx, uint16_t ethtype_qtag_0, uint16_t ethtype_qtag_1)
{
    uint32_t reg_parser_core_configuration_qtag_ethtype=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_ethtype = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, ETHTYPE_QTAG_0, reg_parser_core_configuration_qtag_ethtype, ethtype_qtag_0);
    reg_parser_core_configuration_qtag_ethtype = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, ETHTYPE_QTAG_1, reg_parser_core_configuration_qtag_ethtype, ethtype_qtag_1);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, reg_parser_core_configuration_qtag_ethtype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(uint8_t quad_idx, uint16_t *ethtype_qtag_0, uint16_t *ethtype_qtag_1)
{
    uint32_t reg_parser_core_configuration_qtag_ethtype;

#ifdef VALIDATE_PARMS
    if(!ethtype_qtag_0 || !ethtype_qtag_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, reg_parser_core_configuration_qtag_ethtype);

    *ethtype_qtag_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, ETHTYPE_QTAG_0, reg_parser_core_configuration_qtag_ethtype);
    *ethtype_qtag_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE, ETHTYPE_QTAG_1, reg_parser_core_configuration_qtag_ethtype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(uint8_t quad_idx, uint16_t ethype_0, uint16_t ethype_1)
{
    uint32_t reg_parser_core_configuration_user_ethtype_0_1=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_user_ethtype_0_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, ETHYPE_0, reg_parser_core_configuration_user_ethtype_0_1, ethype_0);
    reg_parser_core_configuration_user_ethtype_0_1 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, ETHYPE_1, reg_parser_core_configuration_user_ethtype_0_1, ethype_1);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, reg_parser_core_configuration_user_ethtype_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(uint8_t quad_idx, uint16_t *ethype_0, uint16_t *ethype_1)
{
    uint32_t reg_parser_core_configuration_user_ethtype_0_1;

#ifdef VALIDATE_PARMS
    if(!ethype_0 || !ethype_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, reg_parser_core_configuration_user_ethtype_0_1);

    *ethype_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, ETHYPE_0, reg_parser_core_configuration_user_ethtype_0_1);
    *ethype_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1, ETHYPE_1, reg_parser_core_configuration_user_ethtype_0_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(uint8_t quad_idx, uint16_t ethype_2, uint16_t ethype_3)
{
    uint32_t reg_parser_core_configuration_user_ethtype_2_3=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_user_ethtype_2_3 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, ETHYPE_2, reg_parser_core_configuration_user_ethtype_2_3, ethype_2);
    reg_parser_core_configuration_user_ethtype_2_3 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, ETHYPE_3, reg_parser_core_configuration_user_ethtype_2_3, ethype_3);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, reg_parser_core_configuration_user_ethtype_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(uint8_t quad_idx, uint16_t *ethype_2, uint16_t *ethype_3)
{
    uint32_t reg_parser_core_configuration_user_ethtype_2_3;

#ifdef VALIDATE_PARMS
    if(!ethype_2 || !ethype_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, reg_parser_core_configuration_user_ethtype_2_3);

    *ethype_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, ETHYPE_2, reg_parser_core_configuration_user_ethtype_2_3);
    *ethype_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3, ETHYPE_3, reg_parser_core_configuration_user_ethtype_2_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_user_ethtype_config *parser_core_configuration_user_ethtype_config)
{
    uint32_t reg_parser_core_configuration_user_ethtype_config=0;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_user_ethtype_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_prot_0 >= _2BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_prot_1 >= _2BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_prot_2 >= _2BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_prot_3 >= _2BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_en >= _4BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_offset_0 >= _4BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_offset_1 >= _4BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_offset_2 >= _4BITS_MAX_VAL_) ||
       (parser_core_configuration_user_ethtype_config->ethtype_user_offset_3 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_0, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_prot_0);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_1, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_prot_1);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_2, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_prot_2);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_3, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_prot_3);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_EN, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_en);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_0, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_offset_0);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_1, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_offset_1);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_2, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_offset_2);
    reg_parser_core_configuration_user_ethtype_config = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_3, reg_parser_core_configuration_user_ethtype_config, parser_core_configuration_user_ethtype_config->ethtype_user_offset_3);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, reg_parser_core_configuration_user_ethtype_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_user_ethtype_config *parser_core_configuration_user_ethtype_config)
{
    uint32_t reg_parser_core_configuration_user_ethtype_config;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_user_ethtype_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, reg_parser_core_configuration_user_ethtype_config);

    parser_core_configuration_user_ethtype_config->ethtype_user_prot_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_0, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_prot_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_1, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_prot_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_2, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_prot_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_PROT_3, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_en = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_EN, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_offset_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_0, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_offset_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_1, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_offset_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_2, reg_parser_core_configuration_user_ethtype_config);
    parser_core_configuration_user_ethtype_config->ethtype_user_offset_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG, ETHTYPE_USER_OFFSET_3, reg_parser_core_configuration_user_ethtype_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid)
{
    /* Identical to da_filter_valid */
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0=0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (da_filter_valid->da_filt0_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt1_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt2_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt3_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt4_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt5_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt6_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt7_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt8_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT0_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt0_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT1_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt1_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT2_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt2_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT3_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt3_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT4_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt4_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT5_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt5_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT6_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt6_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT7_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt7_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT8_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt8_valid);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1, reg_parser_core_configuration_da_filt_valid_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid)
{
    /* Identical to da_filter_valid */
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1, reg_parser_core_configuration_da_filt_valid_cfg_0);

    da_filter_valid->da_filt0_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT0_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt1_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT1_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt2_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT2_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt3_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT3_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt4_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT4_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt5_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT5_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt6_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT6_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt7_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT7_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt8_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT8_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid)
{
    /* Identical to da_filter_valid */
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0=0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (da_filter_valid->da_filt0_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt1_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt2_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt3_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt4_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt5_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt6_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt7_valid >= _1BITS_MAX_VAL_) ||
       (da_filter_valid->da_filt8_valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT0_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt0_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT1_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt1_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT2_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt2_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT3_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt3_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT4_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt4_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT5_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt5_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT6_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt6_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT7_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt7_valid);
    reg_parser_core_configuration_da_filt_valid_cfg_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT8_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0, da_filter_valid->da_filt8_valid);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2, reg_parser_core_configuration_da_filt_valid_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid)
{
    /* Identical to da_filter_valid */
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2, reg_parser_core_configuration_da_filt_valid_cfg_0);

    da_filter_valid->da_filt0_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT0_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt1_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT1_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt2_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT2_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt3_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT3_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt4_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT4_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt5_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT5_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt6_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT6_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt7_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT7_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);
    da_filter_valid->da_filt8_valid = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0, DA_FILT8_VALID, reg_parser_core_configuration_da_filt_valid_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(uint8_t quad_idx, uint16_t gre_protocol)
{
    uint32_t reg_parser_core_configuration_gre_protocol_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_gre_protocol_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG, GRE_PROTOCOL, reg_parser_core_configuration_gre_protocol_cfg, gre_protocol);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG, reg_parser_core_configuration_gre_protocol_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(uint8_t quad_idx, uint16_t *gre_protocol)
{
    uint32_t reg_parser_core_configuration_gre_protocol_cfg;

#ifdef VALIDATE_PARMS
    if(!gre_protocol)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG, reg_parser_core_configuration_gre_protocol_cfg);

    *gre_protocol = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG, GRE_PROTOCOL, reg_parser_core_configuration_gre_protocol_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_prop_tag_cfg *parser_core_configuration_prop_tag_cfg)
{
    uint32_t reg_parser_core_configuration_prop_tag_cfg=0;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_prop_tag_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_core_configuration_prop_tag_cfg->size_profile_0 >= _5BITS_MAX_VAL_) ||
       (parser_core_configuration_prop_tag_cfg->size_profile_1 >= _5BITS_MAX_VAL_) ||
       (parser_core_configuration_prop_tag_cfg->size_profile_2 >= _5BITS_MAX_VAL_) ||
       (parser_core_configuration_prop_tag_cfg->pre_da_dprofile_0 >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_prop_tag_cfg->pre_da_dprofile_1 >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_prop_tag_cfg->pre_da_dprofile_2 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_prop_tag_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, SIZE_PROFILE_0, reg_parser_core_configuration_prop_tag_cfg, parser_core_configuration_prop_tag_cfg->size_profile_0);
    reg_parser_core_configuration_prop_tag_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, SIZE_PROFILE_1, reg_parser_core_configuration_prop_tag_cfg, parser_core_configuration_prop_tag_cfg->size_profile_1);
    reg_parser_core_configuration_prop_tag_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, SIZE_PROFILE_2, reg_parser_core_configuration_prop_tag_cfg, parser_core_configuration_prop_tag_cfg->size_profile_2);
    reg_parser_core_configuration_prop_tag_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, PRE_DA_DPROFILE_0, reg_parser_core_configuration_prop_tag_cfg, parser_core_configuration_prop_tag_cfg->pre_da_dprofile_0);
    reg_parser_core_configuration_prop_tag_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, PRE_DA_DPROFILE_1, reg_parser_core_configuration_prop_tag_cfg, parser_core_configuration_prop_tag_cfg->pre_da_dprofile_1);
    reg_parser_core_configuration_prop_tag_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, PRE_DA_DPROFILE_2, reg_parser_core_configuration_prop_tag_cfg, parser_core_configuration_prop_tag_cfg->pre_da_dprofile_2);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, reg_parser_core_configuration_prop_tag_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_prop_tag_cfg *parser_core_configuration_prop_tag_cfg)
{
    uint32_t reg_parser_core_configuration_prop_tag_cfg;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_prop_tag_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, reg_parser_core_configuration_prop_tag_cfg);

    parser_core_configuration_prop_tag_cfg->size_profile_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, SIZE_PROFILE_0, reg_parser_core_configuration_prop_tag_cfg);
    parser_core_configuration_prop_tag_cfg->size_profile_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, SIZE_PROFILE_1, reg_parser_core_configuration_prop_tag_cfg);
    parser_core_configuration_prop_tag_cfg->size_profile_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, SIZE_PROFILE_2, reg_parser_core_configuration_prop_tag_cfg);
    parser_core_configuration_prop_tag_cfg->pre_da_dprofile_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, PRE_DA_DPROFILE_0, reg_parser_core_configuration_prop_tag_cfg);
    parser_core_configuration_prop_tag_cfg->pre_da_dprofile_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, PRE_DA_DPROFILE_1, reg_parser_core_configuration_prop_tag_cfg);
    parser_core_configuration_prop_tag_cfg->pre_da_dprofile_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG, PRE_DA_DPROFILE_2, reg_parser_core_configuration_prop_tag_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_set(uint8_t quad_idx, bdmf_boolean use_fifo_for_ddr_only, bdmf_boolean token_arbiter_is_rr, bdmf_boolean chicken_no_flowctrl, uint8_t congest_threshold)
{
    uint32_t reg_general_config_dma_arb_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (use_fifo_for_ddr_only >= _1BITS_MAX_VAL_) ||
       (token_arbiter_is_rr >= _1BITS_MAX_VAL_) ||
       (chicken_no_flowctrl >= _1BITS_MAX_VAL_) ||
       (congest_threshold >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, USE_FIFO_FOR_DDR_ONLY, reg_general_config_dma_arb_cfg, use_fifo_for_ddr_only);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, TOKEN_ARBITER_IS_RR, reg_general_config_dma_arb_cfg, token_arbiter_is_rr);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, CHICKEN_NO_FLOWCTRL, reg_general_config_dma_arb_cfg, chicken_no_flowctrl);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, CONGEST_THRESHOLD, reg_general_config_dma_arb_cfg, congest_threshold);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, reg_general_config_dma_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_get(uint8_t quad_idx, bdmf_boolean *use_fifo_for_ddr_only, bdmf_boolean *token_arbiter_is_rr, bdmf_boolean *chicken_no_flowctrl, uint8_t *congest_threshold)
{
    uint32_t reg_general_config_dma_arb_cfg;

#ifdef VALIDATE_PARMS
    if(!use_fifo_for_ddr_only || !token_arbiter_is_rr || !chicken_no_flowctrl || !congest_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, reg_general_config_dma_arb_cfg);

    *use_fifo_for_ddr_only = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, USE_FIFO_FOR_DDR_ONLY, reg_general_config_dma_arb_cfg);
    *token_arbiter_is_rr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, TOKEN_ARBITER_IS_RR, reg_general_config_dma_arb_cfg);
    *chicken_no_flowctrl = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, CHICKEN_NO_FLOWCTRL, reg_general_config_dma_arb_cfg);
    *congest_threshold = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, CONGEST_THRESHOLD, reg_general_config_dma_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram0_base=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram0_base = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_BASE, VAL, reg_general_config_psram0_base, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_BASE, reg_general_config_psram0_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram0_base;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_BASE, reg_general_config_psram0_base);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_BASE, VAL, reg_general_config_psram0_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram1_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram1_base=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram1_base = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_BASE, VAL, reg_general_config_psram1_base, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_BASE, reg_general_config_psram1_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram1_base_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram1_base;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_BASE, reg_general_config_psram1_base);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_BASE, VAL, reg_general_config_psram1_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram2_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram2_base=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram2_base = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_BASE, VAL, reg_general_config_psram2_base, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_BASE, reg_general_config_psram2_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram2_base_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram2_base;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_BASE, reg_general_config_psram2_base);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_BASE, VAL, reg_general_config_psram2_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram3_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram3_base=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram3_base = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_BASE, VAL, reg_general_config_psram3_base, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_BASE, reg_general_config_psram3_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram3_base_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram3_base;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_BASE, reg_general_config_psram3_base);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_BASE, VAL, reg_general_config_psram3_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_ddr0_base=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_ddr0_base = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_BASE, VAL, reg_general_config_ddr0_base, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_BASE, reg_general_config_ddr0_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_base_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_ddr0_base;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_BASE, reg_general_config_ddr0_base);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_BASE, VAL, reg_general_config_ddr0_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_ddr1_base=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_ddr1_base = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_BASE, VAL, reg_general_config_ddr1_base, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_BASE, reg_general_config_ddr1_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_base_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_ddr1_base;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_BASE, reg_general_config_ddr1_base);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_BASE, VAL, reg_general_config_ddr1_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram0_mask_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram0_mask=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram0_mask = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_MASK, VAL, reg_general_config_psram0_mask, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_MASK, reg_general_config_psram0_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram0_mask_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram0_mask;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_MASK, reg_general_config_psram0_mask);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM0_MASK, VAL, reg_general_config_psram0_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram1_mask_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram1_mask=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram1_mask = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_MASK, VAL, reg_general_config_psram1_mask, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_MASK, reg_general_config_psram1_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram1_mask_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram1_mask;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_MASK, reg_general_config_psram1_mask);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM1_MASK, VAL, reg_general_config_psram1_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram2_mask_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram2_mask=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram2_mask = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_MASK, VAL, reg_general_config_psram2_mask, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_MASK, reg_general_config_psram2_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram2_mask_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram2_mask;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_MASK, reg_general_config_psram2_mask);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM2_MASK, VAL, reg_general_config_psram2_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram3_mask_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram3_mask=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_psram3_mask = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_MASK, VAL, reg_general_config_psram3_mask, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_MASK, reg_general_config_psram3_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram3_mask_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_psram3_mask;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_MASK, reg_general_config_psram3_mask);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PSRAM3_MASK, VAL, reg_general_config_psram3_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_mask_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_ddr0_mask=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_ddr0_mask = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_MASK, VAL, reg_general_config_ddr0_mask, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_MASK, reg_general_config_ddr0_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_mask_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_ddr0_mask;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_MASK, reg_general_config_ddr0_mask);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR0_MASK, VAL, reg_general_config_ddr0_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_mask_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_ddr1_mask=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_ddr1_mask = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_MASK, VAL, reg_general_config_ddr1_mask, val);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_MASK, reg_general_config_ddr1_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_mask_get(uint8_t quad_idx, uint32_t *val)
{
    uint32_t reg_general_config_ddr1_mask;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_MASK, reg_general_config_ddr1_mask);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DDR1_MASK, VAL, reg_general_config_ddr1_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_set(uint8_t quad_idx, const rnr_quad_general_config_profiling_config *general_config_profiling_config)
{
    uint32_t reg_general_config_profiling_config=0;

#ifdef VALIDATE_PARMS
    if(!general_config_profiling_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (general_config_profiling_config->counter_lsb_sel >= _5BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_0 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_1 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_2 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_3 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_4 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_5 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, COUNTER_LSB_SEL, reg_general_config_profiling_config, general_config_profiling_config->counter_lsb_sel);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_0, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_0);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_1, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_1);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_2, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_2);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_3, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_3);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_4, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_4);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_5, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_5);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, reg_general_config_profiling_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_get(uint8_t quad_idx, rnr_quad_general_config_profiling_config *general_config_profiling_config)
{
    uint32_t reg_general_config_profiling_config;

#ifdef VALIDATE_PARMS
    if(!general_config_profiling_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, reg_general_config_profiling_config);

    general_config_profiling_config->counter_lsb_sel = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, COUNTER_LSB_SEL, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_0, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_1, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_2, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_3, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_4 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_4, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_5 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_5, reg_general_config_profiling_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_0_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_0_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG, ADDR, reg_general_config_bkpt_0_cfg, addr);
    reg_general_config_bkpt_0_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG, THREAD, reg_general_config_bkpt_0_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG, reg_general_config_bkpt_0_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_0_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG, reg_general_config_bkpt_0_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG, ADDR, reg_general_config_bkpt_0_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG, THREAD, reg_general_config_bkpt_0_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_1_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_1_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG, ADDR, reg_general_config_bkpt_1_cfg, addr);
    reg_general_config_bkpt_1_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG, THREAD, reg_general_config_bkpt_1_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG, reg_general_config_bkpt_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_1_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG, reg_general_config_bkpt_1_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG, ADDR, reg_general_config_bkpt_1_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG, THREAD, reg_general_config_bkpt_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_2_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_2_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG, ADDR, reg_general_config_bkpt_2_cfg, addr);
    reg_general_config_bkpt_2_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG, THREAD, reg_general_config_bkpt_2_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG, reg_general_config_bkpt_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_2_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG, reg_general_config_bkpt_2_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG, ADDR, reg_general_config_bkpt_2_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG, THREAD, reg_general_config_bkpt_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_3_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_3_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG, ADDR, reg_general_config_bkpt_3_cfg, addr);
    reg_general_config_bkpt_3_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG, THREAD, reg_general_config_bkpt_3_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG, reg_general_config_bkpt_3_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_3_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG, reg_general_config_bkpt_3_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG, ADDR, reg_general_config_bkpt_3_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG, THREAD, reg_general_config_bkpt_3_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_4_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_4_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG, ADDR, reg_general_config_bkpt_4_cfg, addr);
    reg_general_config_bkpt_4_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG, THREAD, reg_general_config_bkpt_4_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG, reg_general_config_bkpt_4_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_4_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG, reg_general_config_bkpt_4_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG, ADDR, reg_general_config_bkpt_4_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG, THREAD, reg_general_config_bkpt_4_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_5_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_5_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG, ADDR, reg_general_config_bkpt_5_cfg, addr);
    reg_general_config_bkpt_5_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG, THREAD, reg_general_config_bkpt_5_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG, reg_general_config_bkpt_5_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_5_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG, reg_general_config_bkpt_5_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG, ADDR, reg_general_config_bkpt_5_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG, THREAD, reg_general_config_bkpt_5_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_6_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_6_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG, ADDR, reg_general_config_bkpt_6_cfg, addr);
    reg_general_config_bkpt_6_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG, THREAD, reg_general_config_bkpt_6_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG, reg_general_config_bkpt_6_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_6_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG, reg_general_config_bkpt_6_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG, ADDR, reg_general_config_bkpt_6_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG, THREAD, reg_general_config_bkpt_6_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_7_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (addr >= _13BITS_MAX_VAL_) ||
       (thread >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_7_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG, ADDR, reg_general_config_bkpt_7_cfg, addr);
    reg_general_config_bkpt_7_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG, THREAD, reg_general_config_bkpt_7_cfg, thread);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG, reg_general_config_bkpt_7_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread)
{
    uint32_t reg_general_config_bkpt_7_cfg;

#ifdef VALIDATE_PARMS
    if(!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG, reg_general_config_bkpt_7_cfg);

    *addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG, ADDR, reg_general_config_bkpt_7_cfg);
    *thread = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG, THREAD, reg_general_config_bkpt_7_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(uint8_t quad_idx, uint16_t handler_addr, uint16_t update_pc_value)
{
    uint32_t reg_general_config_bkpt_gen_cfg=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (handler_addr >= _13BITS_MAX_VAL_) ||
       (update_pc_value >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_bkpt_gen_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG, HANDLER_ADDR, reg_general_config_bkpt_gen_cfg, handler_addr);
    reg_general_config_bkpt_gen_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG, UPDATE_PC_VALUE, reg_general_config_bkpt_gen_cfg, update_pc_value);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG, reg_general_config_bkpt_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(uint8_t quad_idx, uint16_t *handler_addr, uint16_t *update_pc_value)
{
    uint32_t reg_general_config_bkpt_gen_cfg;

#ifdef VALIDATE_PARMS
    if(!handler_addr || !update_pc_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG, reg_general_config_bkpt_gen_cfg);

    *handler_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG, HANDLER_ADDR, reg_general_config_bkpt_gen_cfg);
    *update_pc_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG, UPDATE_PC_VALUE, reg_general_config_bkpt_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_set(uint8_t quad_idx, const rnr_quad_general_config_powersave_config *general_config_powersave_config)
{
    uint32_t reg_general_config_powersave_config=0;

#ifdef VALIDATE_PARMS
    if(!general_config_powersave_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (general_config_powersave_config->enable_powersave_core_0 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_1 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_2 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_3 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_4 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_5 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, TIME_COUNTER, reg_general_config_powersave_config, general_config_powersave_config->time_counter);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_0, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_0);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_1, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_1);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_2, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_2);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_3, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_3);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_4, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_4);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_5, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_5);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, reg_general_config_powersave_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_get(uint8_t quad_idx, rnr_quad_general_config_powersave_config *general_config_powersave_config)
{
    uint32_t reg_general_config_powersave_config;

#ifdef VALIDATE_PARMS
    if(!general_config_powersave_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, reg_general_config_powersave_config);

    general_config_powersave_config->time_counter = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, TIME_COUNTER, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_0, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_1, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_2, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_3, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_4 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_4, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_5 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_5, reg_general_config_powersave_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_powersave_status_get(uint8_t quad_idx, rnr_quad_general_config_powersave_status *general_config_powersave_status)
{
    uint32_t reg_general_config_powersave_status;

#ifdef VALIDATE_PARMS
    if(!general_config_powersave_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, reg_general_config_powersave_status);

    general_config_powersave_status->core_0_status = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_0_STATUS, reg_general_config_powersave_status);
    general_config_powersave_status->core_1_status = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_1_STATUS, reg_general_config_powersave_status);
    general_config_powersave_status->core_2_status = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_2_STATUS, reg_general_config_powersave_status);
    general_config_powersave_status->core_3_status = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_3_STATUS, reg_general_config_powersave_status);
    general_config_powersave_status->core_4_status = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_4_STATUS, reg_general_config_powersave_status);
    general_config_powersave_status->core_5_status = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_5_STATUS, reg_general_config_powersave_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_set(uint8_t quad_idx, const rnr_quad_debug_fifo_config *debug_fifo_config)
{
    uint32_t reg_debug_fifo_config=0;

#ifdef VALIDATE_PARMS
    if(!debug_fifo_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (debug_fifo_config->psram_hdr_sw_rst >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_data_sw_rst >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->ddr_hdr_sw_rst >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_hdr_sw_rd_addr >= _4BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_data_sw_rd_addr >= _4BITS_MAX_VAL_) ||
       (debug_fifo_config->ddr_hdr_sw_rd_addr >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RST, reg_debug_fifo_config, debug_fifo_config->psram_hdr_sw_rst);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RST, reg_debug_fifo_config, debug_fifo_config->psram_data_sw_rst);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RST, reg_debug_fifo_config, debug_fifo_config->ddr_hdr_sw_rst);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RD_ADDR, reg_debug_fifo_config, debug_fifo_config->psram_hdr_sw_rd_addr);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RD_ADDR, reg_debug_fifo_config, debug_fifo_config->psram_data_sw_rd_addr);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RD_ADDR, reg_debug_fifo_config, debug_fifo_config->ddr_hdr_sw_rd_addr);

    RU_REG_WRITE(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, reg_debug_fifo_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_get(uint8_t quad_idx, rnr_quad_debug_fifo_config *debug_fifo_config)
{
    uint32_t reg_debug_fifo_config;

#ifdef VALIDATE_PARMS
    if(!debug_fifo_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, reg_debug_fifo_config);

    debug_fifo_config->psram_hdr_sw_rst = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RST, reg_debug_fifo_config);
    debug_fifo_config->psram_data_sw_rst = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RST, reg_debug_fifo_config);
    debug_fifo_config->ddr_hdr_sw_rst = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RST, reg_debug_fifo_config);
    debug_fifo_config->psram_hdr_sw_rd_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RD_ADDR, reg_debug_fifo_config);
    debug_fifo_config->psram_data_sw_rd_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RD_ADDR, reg_debug_fifo_config);
    debug_fifo_config->ddr_hdr_sw_rd_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RD_ADDR, reg_debug_fifo_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_hdr_fifo_status *debug_psram_hdr_fifo_status)
{
    uint32_t reg_debug_psram_hdr_fifo_status;

#ifdef VALIDATE_PARMS
    if(!debug_psram_hdr_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS, reg_debug_psram_hdr_fifo_status);

    debug_psram_hdr_fifo_status->full = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS, FULL, reg_debug_psram_hdr_fifo_status);
    debug_psram_hdr_fifo_status->empty = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS, EMPTY, reg_debug_psram_hdr_fifo_status);
    debug_psram_hdr_fifo_status->push_wr_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS, PUSH_WR_CNTR, reg_debug_psram_hdr_fifo_status);
    debug_psram_hdr_fifo_status->pop_rd_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS, POP_RD_CNTR, reg_debug_psram_hdr_fifo_status);
    debug_psram_hdr_fifo_status->used_words = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS, USED_WORDS, reg_debug_psram_hdr_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_data_fifo_status *debug_psram_data_fifo_status)
{
    uint32_t reg_debug_psram_data_fifo_status;

#ifdef VALIDATE_PARMS
    if(!debug_psram_data_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, reg_debug_psram_data_fifo_status);

    debug_psram_data_fifo_status->full = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, FULL, reg_debug_psram_data_fifo_status);
    debug_psram_data_fifo_status->empty = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, EMPTY, reg_debug_psram_data_fifo_status);
    debug_psram_data_fifo_status->almost_full = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, ALMOST_FULL, reg_debug_psram_data_fifo_status);
    debug_psram_data_fifo_status->push_wr_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, PUSH_WR_CNTR, reg_debug_psram_data_fifo_status);
    debug_psram_data_fifo_status->pop_rd_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, POP_RD_CNTR, reg_debug_psram_data_fifo_status);
    debug_psram_data_fifo_status->used_words = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS, USED_WORDS, reg_debug_psram_data_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_ddr_hdr_fifo_status *debug_ddr_hdr_fifo_status)
{
    uint32_t reg_debug_ddr_hdr_fifo_status;

#ifdef VALIDATE_PARMS
    if(!debug_ddr_hdr_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS, reg_debug_ddr_hdr_fifo_status);

    debug_ddr_hdr_fifo_status->full = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS, FULL, reg_debug_ddr_hdr_fifo_status);
    debug_ddr_hdr_fifo_status->empty = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS, EMPTY, reg_debug_ddr_hdr_fifo_status);
    debug_ddr_hdr_fifo_status->push_wr_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS, PUSH_WR_CNTR, reg_debug_ddr_hdr_fifo_status);
    debug_ddr_hdr_fifo_status->pop_rd_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS, POP_RD_CNTR, reg_debug_ddr_hdr_fifo_status);
    debug_ddr_hdr_fifo_status->used_words = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS, USED_WORDS, reg_debug_ddr_hdr_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_ddr_data_fifo_status *debug_ddr_data_fifo_status)
{
    uint32_t reg_debug_ddr_data_fifo_status;

#ifdef VALIDATE_PARMS
    if(!debug_ddr_data_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS, reg_debug_ddr_data_fifo_status);

    debug_ddr_data_fifo_status->full = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS, FULL, reg_debug_ddr_data_fifo_status);
    debug_ddr_data_fifo_status->empty = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS, EMPTY, reg_debug_ddr_data_fifo_status);
    debug_ddr_data_fifo_status->almost_full = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS, ALMOST_FULL, reg_debug_ddr_data_fifo_status);
    debug_ddr_data_fifo_status->wr_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS, WR_CNTR, reg_debug_ddr_data_fifo_status);
    debug_ddr_data_fifo_status->rd_cntr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS, RD_CNTR, reg_debug_ddr_data_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(uint8_t quad_idx, uint8_t *read_addr, uint16_t *used_words)
{
    uint32_t reg_debug_ddr_data_fifo_status2;

#ifdef VALIDATE_PARMS
    if(!read_addr || !used_words)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS2, reg_debug_ddr_data_fifo_status2);

    *read_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS2, READ_ADDR, reg_debug_ddr_data_fifo_status2);
    *used_words = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS2, USED_WORDS, reg_debug_ddr_data_fifo_status2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(uint8_t quad_idx, uint32_t *data)
{
    uint32_t reg_debug_psram_hdr_fifo_data1;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA1, reg_debug_psram_hdr_fifo_data1);

    *data = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA1, DATA, reg_debug_psram_hdr_fifo_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(uint8_t quad_idx, uint32_t *data)
{
    uint32_t reg_debug_psram_hdr_fifo_data2;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA2, reg_debug_psram_hdr_fifo_data2);

    *data = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA2, DATA, reg_debug_psram_hdr_fifo_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(uint8_t quad_idx, uint32_t *data)
{
    uint32_t reg_debug_psram_data_fifo_data1;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA1, reg_debug_psram_data_fifo_data1);

    *data = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA1, DATA, reg_debug_psram_data_fifo_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(uint8_t quad_idx, uint32_t *data)
{
    uint32_t reg_debug_psram_data_fifo_data2;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA2, reg_debug_psram_data_fifo_data2);

    *data = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA2, DATA, reg_debug_psram_data_fifo_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(uint8_t quad_idx, uint32_t *data)
{
    uint32_t reg_debug_ddr_hdr_fifo_data1;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA1, reg_debug_ddr_hdr_fifo_data1);

    *data = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA1, DATA, reg_debug_ddr_hdr_fifo_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(uint8_t quad_idx, uint32_t *data)
{
    uint32_t reg_debug_ddr_hdr_fifo_data2;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA2, reg_debug_ddr_hdr_fifo_data2);

    *data = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA2, DATA, reg_debug_ddr_hdr_fifo_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ext_flowctrl_config_token_val=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 36))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ext_flowctrl_config_token_val = RU_FIELD_SET(quad_idx, RNR_QUAD, EXT_FLOWCTRL_CONFIG_TOKEN_VAL, VAL, reg_ext_flowctrl_config_token_val, val);

    RU_REG_RAM_WRITE(quad_idx, index, RNR_QUAD, EXT_FLOWCTRL_CONFIG_TOKEN_VAL, reg_ext_flowctrl_config_token_val);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(uint8_t quad_idx, uint32_t index, uint32_t *val)
{
    uint32_t reg_ext_flowctrl_config_token_val;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 36))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(quad_idx, index, RNR_QUAD, EXT_FLOWCTRL_CONFIG_TOKEN_VAL, reg_ext_flowctrl_config_token_val);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, EXT_FLOWCTRL_CONFIG_TOKEN_VAL, VAL, reg_ext_flowctrl_config_token_val);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ubus_decode_cfg_psram_ubus_decode=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ubus_decode_cfg_psram_ubus_decode = RU_FIELD_SET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG_PSRAM_UBUS_DECODE, VAL, reg_ubus_decode_cfg_psram_ubus_decode, val);

    RU_REG_RAM_WRITE(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG_PSRAM_UBUS_DECODE, reg_ubus_decode_cfg_psram_ubus_decode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(uint8_t quad_idx, uint32_t index, uint32_t *val)
{
    uint32_t reg_ubus_decode_cfg_psram_ubus_decode;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG_PSRAM_UBUS_DECODE, reg_ubus_decode_cfg_psram_ubus_decode);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG_PSRAM_UBUS_DECODE, VAL, reg_ubus_decode_cfg_psram_ubus_decode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ubus_decode_cfg_ddr_ubus_decode=0;

#ifdef VALIDATE_PARMS
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ubus_decode_cfg_ddr_ubus_decode = RU_FIELD_SET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG_DDR_UBUS_DECODE, VAL, reg_ubus_decode_cfg_ddr_ubus_decode, val);

    RU_REG_RAM_WRITE(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG_DDR_UBUS_DECODE, reg_ubus_decode_cfg_ddr_ubus_decode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(uint8_t quad_idx, uint32_t index, uint32_t *val)
{
    uint32_t reg_ubus_decode_cfg_ddr_ubus_decode;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG_DDR_UBUS_DECODE, reg_ubus_decode_cfg_ddr_ubus_decode);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG_DDR_UBUS_DECODE, VAL, reg_ubus_decode_cfg_ddr_ubus_decode);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_parser_core_configuration_eng,
    bdmf_address_parser_core_configuration_parser_misc_cfg,
    bdmf_address_parser_core_configuration_vid_0_1,
    bdmf_address_parser_core_configuration_vid_2_3,
    bdmf_address_parser_core_configuration_vid_4_5,
    bdmf_address_parser_core_configuration_vid_6_7,
    bdmf_address_parser_core_configuration_ip_filter0_cfg,
    bdmf_address_parser_core_configuration_ip_filter1_cfg,
    bdmf_address_parser_core_configuration_ip_filter0_mask_cfg,
    bdmf_address_parser_core_configuration_ip_filter1_mask_cfg,
    bdmf_address_parser_core_configuration_ip_filters_cfg,
    bdmf_address_parser_core_configuration_snap_org_code,
    bdmf_address_parser_core_configuration_ppp_ip_prot_code,
    bdmf_address_parser_core_configuration_qtag_ethtype,
    bdmf_address_parser_core_configuration_user_ethtype_0_1,
    bdmf_address_parser_core_configuration_user_ethtype_2_3,
    bdmf_address_parser_core_configuration_user_ethtype_config,
    bdmf_address_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg,
    bdmf_address_parser_core_configuration_qtag_nest,
    bdmf_address_parser_core_configuration_qtag_hard_nest_0,
    bdmf_address_parser_core_configuration_qtag_hard_nest_1,
    bdmf_address_parser_core_configuration_qtag_hard_nest_2,
    bdmf_address_parser_core_configuration_user_ip_prot,
    bdmf_address_parser_core_configuration_da_filt0_val_l,
    bdmf_address_parser_core_configuration_da_filt0_val_h,
    bdmf_address_parser_core_configuration_da_filt1_val_l,
    bdmf_address_parser_core_configuration_da_filt1_val_h,
    bdmf_address_parser_core_configuration_da_filt2_val_l,
    bdmf_address_parser_core_configuration_da_filt2_val_h,
    bdmf_address_parser_core_configuration_da_filt3_val_l,
    bdmf_address_parser_core_configuration_da_filt3_val_h,
    bdmf_address_parser_core_configuration_da_filt4_val_l,
    bdmf_address_parser_core_configuration_da_filt4_val_h,
    bdmf_address_parser_core_configuration_da_filt5_val_l,
    bdmf_address_parser_core_configuration_da_filt5_val_h,
    bdmf_address_parser_core_configuration_da_filt6_val_l,
    bdmf_address_parser_core_configuration_da_filt6_val_h,
    bdmf_address_parser_core_configuration_da_filt7_val_l,
    bdmf_address_parser_core_configuration_da_filt7_val_h,
    bdmf_address_parser_core_configuration_da_filt8_val_l,
    bdmf_address_parser_core_configuration_da_filt8_val_h,
    bdmf_address_parser_core_configuration_da_filt0_mask_l,
    bdmf_address_parser_core_configuration_da_filt0_mask_h,
    bdmf_address_parser_core_configuration_da_filt1_mask_l,
    bdmf_address_parser_core_configuration_da_filt1_mask_h,
    bdmf_address_parser_core_configuration_da_filt_valid_cfg_0,
    bdmf_address_parser_core_configuration_da_filt_valid_cfg_1,
    bdmf_address_parser_core_configuration_da_filt_valid_cfg_2,
    bdmf_address_parser_core_configuration_gre_protocol_cfg,
    bdmf_address_parser_core_configuration_prop_tag_cfg,
    bdmf_address_general_config_dma_arb_cfg,
    bdmf_address_general_config_psram0_base,
    bdmf_address_general_config_psram1_base,
    bdmf_address_general_config_psram2_base,
    bdmf_address_general_config_psram3_base,
    bdmf_address_general_config_ddr0_base,
    bdmf_address_general_config_ddr1_base,
    bdmf_address_general_config_psram0_mask,
    bdmf_address_general_config_psram1_mask,
    bdmf_address_general_config_psram2_mask,
    bdmf_address_general_config_psram3_mask,
    bdmf_address_general_config_ddr0_mask,
    bdmf_address_general_config_ddr1_mask,
    bdmf_address_general_config_profiling_config,
    bdmf_address_general_config_bkpt_0_cfg,
    bdmf_address_general_config_bkpt_1_cfg,
    bdmf_address_general_config_bkpt_2_cfg,
    bdmf_address_general_config_bkpt_3_cfg,
    bdmf_address_general_config_bkpt_4_cfg,
    bdmf_address_general_config_bkpt_5_cfg,
    bdmf_address_general_config_bkpt_6_cfg,
    bdmf_address_general_config_bkpt_7_cfg,
    bdmf_address_general_config_bkpt_gen_cfg,
    bdmf_address_general_config_powersave_config,
    bdmf_address_general_config_powersave_status,
    bdmf_address_debug_fifo_config,
    bdmf_address_debug_psram_hdr_fifo_status,
    bdmf_address_debug_psram_data_fifo_status,
    bdmf_address_debug_ddr_hdr_fifo_status,
    bdmf_address_debug_ddr_data_fifo_status,
    bdmf_address_debug_ddr_data_fifo_status2,
    bdmf_address_debug_psram_hdr_fifo_data1,
    bdmf_address_debug_psram_hdr_fifo_data2,
    bdmf_address_debug_psram_data_fifo_data1,
    bdmf_address_debug_psram_data_fifo_data2,
    bdmf_address_debug_ddr_hdr_fifo_data1,
    bdmf_address_debug_ddr_hdr_fifo_data2,
    bdmf_address_ext_flowctrl_config_token_val,
    bdmf_address_ubus_decode_cfg_psram_ubus_decode,
    bdmf_address_ubus_decode_cfg_ddr_ubus_decode,
}
bdmf_address;

static int bcm_rnr_quad_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_rnr_quad_parser_vid0:
        err = ag_drv_rnr_quad_parser_vid0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid1:
        err = ag_drv_rnr_quad_parser_vid1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid2:
        err = ag_drv_rnr_quad_parser_vid2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid3:
        err = ag_drv_rnr_quad_parser_vid3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid4:
        err = ag_drv_rnr_quad_parser_vid4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid5:
        err = ag_drv_rnr_quad_parser_vid5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid6:
        err = ag_drv_rnr_quad_parser_vid6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid7:
        err = ag_drv_rnr_quad_parser_vid7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_ip0:
    {
        rnr_quad_parser_ip0 parser_ip0 = { .ip_address=parm[2].value.unumber, .ip_address_mask=parm[3].value.unumber, .ip_filter0_dip_en=parm[4].value.unumber, .ip_filter0_valid=parm[5].value.unumber};
        err = ag_drv_rnr_quad_parser_ip0_set(parm[1].value.unumber, &parser_ip0);
        break;
    }
    case cli_rnr_quad_parser_ip1:
    {
        rnr_quad_parser_ip0 parser_ip0 = { .ip_address=parm[2].value.unumber, .ip_address_mask=parm[3].value.unumber, .ip_filter0_dip_en=parm[4].value.unumber, .ip_filter0_valid=parm[5].value.unumber};
        err = ag_drv_rnr_quad_parser_ip1_set(parm[1].value.unumber, &parser_ip0);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof0:
        err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_hardcoded_ethtype_prof1:
        err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_hardcoded_ethtype_prof2:
        err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_qtag_nest_prof0:
        err = ag_drv_rnr_quad_parser_qtag_nest_prof0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_qtag_nest_prof1:
        err = ag_drv_rnr_quad_parser_qtag_nest_prof1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_qtag_nest_prof2:
        err = ag_drv_rnr_quad_parser_qtag_nest_prof2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol0:
        err = ag_drv_rnr_quad_parser_ip_protocol0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol1:
        err = ag_drv_rnr_quad_parser_ip_protocol1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol2:
        err = ag_drv_rnr_quad_parser_ip_protocol2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol3:
        err = ag_drv_rnr_quad_parser_ip_protocol3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter:
    {
        rnr_quad_parser_da_filter parser_da_filter = { .da_filt_msb=parm[2].value.unumber, .da_filt_lsb=parm[3].value.unumber, .da_filt_mask_msb=parm[4].value.unumber, .da_filt_mask_l=parm[5].value.unumber};
        err = ag_drv_rnr_quad_parser_da_filter_set(parm[1].value.unumber, &parser_da_filter);
        break;
    }
    case cli_rnr_quad_parser_da_filter1:
    {
        rnr_quad_parser_da_filter parser_da_filter = { .da_filt_msb=parm[2].value.unumber, .da_filt_lsb=parm[3].value.unumber, .da_filt_mask_msb=parm[4].value.unumber, .da_filt_mask_l=parm[5].value.unumber};
        err = ag_drv_rnr_quad_parser_da_filter1_set(parm[1].value.unumber, &parser_da_filter);
        break;
    }
    case cli_rnr_quad_parser_da_filter2:
        err = ag_drv_rnr_quad_parser_da_filter2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter3:
        err = ag_drv_rnr_quad_parser_da_filter3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter4:
        err = ag_drv_rnr_quad_parser_da_filter4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter5:
        err = ag_drv_rnr_quad_parser_da_filter5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter6:
        err = ag_drv_rnr_quad_parser_da_filter6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter7:
        err = ag_drv_rnr_quad_parser_da_filter7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter8:
        err = ag_drv_rnr_quad_parser_da_filter8_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_da_filter_valid:
    {
        rnr_quad_da_filter_valid da_filter_valid = { .da_filt0_valid=parm[2].value.unumber, .da_filt1_valid=parm[3].value.unumber, .da_filt2_valid=parm[4].value.unumber, .da_filt3_valid=parm[5].value.unumber, .da_filt4_valid=parm[6].value.unumber, .da_filt5_valid=parm[7].value.unumber, .da_filt6_valid=parm[8].value.unumber, .da_filt7_valid=parm[9].value.unumber, .da_filt8_valid=parm[10].value.unumber};
        err = ag_drv_rnr_quad_da_filter_valid_set(parm[1].value.unumber, &da_filter_valid);
        break;
    }
    case cli_rnr_quad_exception_bits:
        err = ag_drv_rnr_quad_exception_bits_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_tcp_flags:
        err = ag_drv_rnr_quad_tcp_flags_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_profile_us:
        err = ag_drv_rnr_quad_profile_us_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_snap_conf:
    {
        rnr_quad_parser_snap_conf parser_snap_conf = { .code=parm[2].value.unumber, .en_rfc1042=parm[3].value.unumber, .en_8021q=parm[4].value.unumber};
        err = ag_drv_rnr_quad_parser_snap_conf_set(parm[1].value.unumber, &parser_snap_conf);
        break;
    }
    case cli_rnr_quad_parser_ipv6_filter:
    {
        rnr_quad_parser_ipv6_filter parser_ipv6_filter = { .hop_by_hop_match=parm[2].value.unumber, .routing_eh=parm[3].value.unumber, .dest_opt_eh=parm[4].value.unumber};
        err = ag_drv_rnr_quad_parser_ipv6_filter_set(parm[1].value.unumber, &parser_ipv6_filter);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_eng:
        err = ag_drv_rnr_quad_parser_core_configuration_eng_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code:
        err = ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_qtag_ethtype:
        err = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_user_ethtype_0_1:
        err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_user_ethtype_2_3:
        err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_user_ethtype_config:
    {
        rnr_quad_parser_core_configuration_user_ethtype_config parser_core_configuration_user_ethtype_config = { .ethtype_user_prot_0=parm[2].value.unumber, .ethtype_user_prot_1=parm[3].value.unumber, .ethtype_user_prot_2=parm[4].value.unumber, .ethtype_user_prot_3=parm[5].value.unumber, .ethtype_user_en=parm[6].value.unumber, .ethtype_user_offset_0=parm[7].value.unumber, .ethtype_user_offset_1=parm[8].value.unumber, .ethtype_user_offset_2=parm[9].value.unumber, .ethtype_user_offset_3=parm[10].value.unumber};
        err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(parm[1].value.unumber, &parser_core_configuration_user_ethtype_config);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1:
    {
        rnr_quad_da_filter_valid da_filter_valid = { .da_filt0_valid=parm[2].value.unumber, .da_filt1_valid=parm[3].value.unumber, .da_filt2_valid=parm[4].value.unumber, .da_filt3_valid=parm[5].value.unumber, .da_filt4_valid=parm[6].value.unumber, .da_filt5_valid=parm[7].value.unumber, .da_filt6_valid=parm[8].value.unumber, .da_filt7_valid=parm[9].value.unumber, .da_filt8_valid=parm[10].value.unumber};
        err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(parm[1].value.unumber, &da_filter_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2:
    {
        rnr_quad_da_filter_valid da_filter_valid = { .da_filt0_valid=parm[2].value.unumber, .da_filt1_valid=parm[3].value.unumber, .da_filt2_valid=parm[4].value.unumber, .da_filt3_valid=parm[5].value.unumber, .da_filt4_valid=parm[6].value.unumber, .da_filt5_valid=parm[7].value.unumber, .da_filt6_valid=parm[8].value.unumber, .da_filt7_valid=parm[9].value.unumber, .da_filt8_valid=parm[10].value.unumber};
        err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(parm[1].value.unumber, &da_filter_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_gre_protocol_cfg:
        err = ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_prop_tag_cfg:
    {
        rnr_quad_parser_core_configuration_prop_tag_cfg parser_core_configuration_prop_tag_cfg = { .size_profile_0=parm[2].value.unumber, .size_profile_1=parm[3].value.unumber, .size_profile_2=parm[4].value.unumber, .pre_da_dprofile_0=parm[5].value.unumber, .pre_da_dprofile_1=parm[6].value.unumber, .pre_da_dprofile_2=parm[7].value.unumber};
        err = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(parm[1].value.unumber, &parser_core_configuration_prop_tag_cfg);
        break;
    }
    case cli_rnr_quad_general_config_dma_arb_cfg:
        err = ag_drv_rnr_quad_general_config_dma_arb_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram0_base:
        err = ag_drv_rnr_quad_general_config_psram0_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram1_base:
        err = ag_drv_rnr_quad_general_config_psram1_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram2_base:
        err = ag_drv_rnr_quad_general_config_psram2_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram3_base:
        err = ag_drv_rnr_quad_general_config_psram3_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr0_base:
        err = ag_drv_rnr_quad_general_config_ddr0_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr1_base:
        err = ag_drv_rnr_quad_general_config_ddr1_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram0_mask:
        err = ag_drv_rnr_quad_general_config_psram0_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram1_mask:
        err = ag_drv_rnr_quad_general_config_psram1_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram2_mask:
        err = ag_drv_rnr_quad_general_config_psram2_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram3_mask:
        err = ag_drv_rnr_quad_general_config_psram3_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr0_mask:
        err = ag_drv_rnr_quad_general_config_ddr0_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr1_mask:
        err = ag_drv_rnr_quad_general_config_ddr1_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_profiling_config:
    {
        rnr_quad_general_config_profiling_config general_config_profiling_config = { .counter_lsb_sel=parm[2].value.unumber, .enable_trace_core_0=parm[3].value.unumber, .enable_trace_core_1=parm[4].value.unumber, .enable_trace_core_2=parm[5].value.unumber, .enable_trace_core_3=parm[6].value.unumber, .enable_trace_core_4=parm[7].value.unumber, .enable_trace_core_5=parm[8].value.unumber};
        err = ag_drv_rnr_quad_general_config_profiling_config_set(parm[1].value.unumber, &general_config_profiling_config);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_0_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_1_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_2_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_3_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_4_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_5_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_6_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_7_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_gen_cfg:
        err = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_powersave_config:
    {
        rnr_quad_general_config_powersave_config general_config_powersave_config = { .time_counter=parm[2].value.unumber, .enable_powersave_core_0=parm[3].value.unumber, .enable_powersave_core_1=parm[4].value.unumber, .enable_powersave_core_2=parm[5].value.unumber, .enable_powersave_core_3=parm[6].value.unumber, .enable_powersave_core_4=parm[7].value.unumber, .enable_powersave_core_5=parm[8].value.unumber};
        err = ag_drv_rnr_quad_general_config_powersave_config_set(parm[1].value.unumber, &general_config_powersave_config);
        break;
    }
    case cli_rnr_quad_debug_fifo_config:
    {
        rnr_quad_debug_fifo_config debug_fifo_config = { .psram_hdr_sw_rst=parm[2].value.unumber, .psram_data_sw_rst=parm[3].value.unumber, .ddr_hdr_sw_rst=parm[4].value.unumber, .psram_hdr_sw_rd_addr=parm[5].value.unumber, .psram_data_sw_rd_addr=parm[6].value.unumber, .ddr_hdr_sw_rd_addr=parm[7].value.unumber};
        err = ag_drv_rnr_quad_debug_fifo_config_set(parm[1].value.unumber, &debug_fifo_config);
        break;
    }
    case cli_rnr_quad_ext_flowctrl_config_token_val:
        err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode:
        err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode:
        err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_rnr_quad_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_rnr_quad_parser_vid0:
    {
        uint16_t vid_0;
        bdmf_boolean vid_0_en;
        err = ag_drv_rnr_quad_parser_vid0_get(parm[1].value.unumber, &vid_0, &vid_0_en);
        bdmf_session_print(session, "vid_0 = %u (0x%x)\n", vid_0, vid_0);
        bdmf_session_print(session, "vid_0_en = %u (0x%x)\n", vid_0_en, vid_0_en);
        break;
    }
    case cli_rnr_quad_parser_vid1:
    {
        uint16_t vid_1;
        bdmf_boolean vid_1_en;
        err = ag_drv_rnr_quad_parser_vid1_get(parm[1].value.unumber, &vid_1, &vid_1_en);
        bdmf_session_print(session, "vid_1 = %u (0x%x)\n", vid_1, vid_1);
        bdmf_session_print(session, "vid_1_en = %u (0x%x)\n", vid_1_en, vid_1_en);
        break;
    }
    case cli_rnr_quad_parser_vid2:
    {
        uint16_t vid_2;
        bdmf_boolean vid_2_en;
        err = ag_drv_rnr_quad_parser_vid2_get(parm[1].value.unumber, &vid_2, &vid_2_en);
        bdmf_session_print(session, "vid_2 = %u (0x%x)\n", vid_2, vid_2);
        bdmf_session_print(session, "vid_2_en = %u (0x%x)\n", vid_2_en, vid_2_en);
        break;
    }
    case cli_rnr_quad_parser_vid3:
    {
        uint16_t vid_3;
        bdmf_boolean vid_3_en;
        err = ag_drv_rnr_quad_parser_vid3_get(parm[1].value.unumber, &vid_3, &vid_3_en);
        bdmf_session_print(session, "vid_3 = %u (0x%x)\n", vid_3, vid_3);
        bdmf_session_print(session, "vid_3_en = %u (0x%x)\n", vid_3_en, vid_3_en);
        break;
    }
    case cli_rnr_quad_parser_vid4:
    {
        uint16_t vid_4;
        bdmf_boolean vid_4_en;
        err = ag_drv_rnr_quad_parser_vid4_get(parm[1].value.unumber, &vid_4, &vid_4_en);
        bdmf_session_print(session, "vid_4 = %u (0x%x)\n", vid_4, vid_4);
        bdmf_session_print(session, "vid_4_en = %u (0x%x)\n", vid_4_en, vid_4_en);
        break;
    }
    case cli_rnr_quad_parser_vid5:
    {
        uint16_t vid_5;
        bdmf_boolean vid_5_en;
        err = ag_drv_rnr_quad_parser_vid5_get(parm[1].value.unumber, &vid_5, &vid_5_en);
        bdmf_session_print(session, "vid_5 = %u (0x%x)\n", vid_5, vid_5);
        bdmf_session_print(session, "vid_5_en = %u (0x%x)\n", vid_5_en, vid_5_en);
        break;
    }
    case cli_rnr_quad_parser_vid6:
    {
        uint16_t vid_6;
        bdmf_boolean vid_6_en;
        err = ag_drv_rnr_quad_parser_vid6_get(parm[1].value.unumber, &vid_6, &vid_6_en);
        bdmf_session_print(session, "vid_6 = %u (0x%x)\n", vid_6, vid_6);
        bdmf_session_print(session, "vid_6_en = %u (0x%x)\n", vid_6_en, vid_6_en);
        break;
    }
    case cli_rnr_quad_parser_vid7:
    {
        uint16_t vid_7;
        bdmf_boolean vid_7_en;
        err = ag_drv_rnr_quad_parser_vid7_get(parm[1].value.unumber, &vid_7, &vid_7_en);
        bdmf_session_print(session, "vid_7 = %u (0x%x)\n", vid_7, vid_7);
        bdmf_session_print(session, "vid_7_en = %u (0x%x)\n", vid_7_en, vid_7_en);
        break;
    }
    case cli_rnr_quad_parser_ip0:
    {
        rnr_quad_parser_ip0 parser_ip0;
        err = ag_drv_rnr_quad_parser_ip0_get(parm[1].value.unumber, &parser_ip0);
        bdmf_session_print(session, "ip_address = %u (0x%x)\n", parser_ip0.ip_address, parser_ip0.ip_address);
        bdmf_session_print(session, "ip_address_mask = %u (0x%x)\n", parser_ip0.ip_address_mask, parser_ip0.ip_address_mask);
        bdmf_session_print(session, "ip_filter0_dip_en = %u (0x%x)\n", parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_dip_en);
        bdmf_session_print(session, "ip_filter0_valid = %u (0x%x)\n", parser_ip0.ip_filter0_valid, parser_ip0.ip_filter0_valid);
        break;
    }
    case cli_rnr_quad_parser_ip1:
    {
        rnr_quad_parser_ip0 parser_ip0;
        err = ag_drv_rnr_quad_parser_ip1_get(parm[1].value.unumber, &parser_ip0);
        bdmf_session_print(session, "ip_address = %u (0x%x)\n", parser_ip0.ip_address, parser_ip0.ip_address);
        bdmf_session_print(session, "ip_address_mask = %u (0x%x)\n", parser_ip0.ip_address_mask, parser_ip0.ip_address_mask);
        bdmf_session_print(session, "ip_filter0_dip_en = %u (0x%x)\n", parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_dip_en);
        bdmf_session_print(session, "ip_filter0_valid = %u (0x%x)\n", parser_ip0.ip_filter0_valid, parser_ip0.ip_filter0_valid);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof0:
    {
        uint16_t hard_nest_profile;
        err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(parm[1].value.unumber, &hard_nest_profile);
        bdmf_session_print(session, "hard_nest_profile = %u (0x%x)\n", hard_nest_profile, hard_nest_profile);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof1:
    {
        uint16_t hard_nest_profile;
        err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(parm[1].value.unumber, &hard_nest_profile);
        bdmf_session_print(session, "hard_nest_profile = %u (0x%x)\n", hard_nest_profile, hard_nest_profile);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof2:
    {
        uint16_t hard_nest_profile;
        err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(parm[1].value.unumber, &hard_nest_profile);
        bdmf_session_print(session, "hard_nest_profile = %u (0x%x)\n", hard_nest_profile, hard_nest_profile);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof0:
    {
        uint8_t qtag_nest_0_profile_0;
        uint8_t qtag_nest_1_profile_0;
        err = ag_drv_rnr_quad_parser_qtag_nest_prof0_get(parm[1].value.unumber, &qtag_nest_0_profile_0, &qtag_nest_1_profile_0);
        bdmf_session_print(session, "qtag_nest_0_profile_0 = %u (0x%x)\n", qtag_nest_0_profile_0, qtag_nest_0_profile_0);
        bdmf_session_print(session, "qtag_nest_1_profile_0 = %u (0x%x)\n", qtag_nest_1_profile_0, qtag_nest_1_profile_0);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof1:
    {
        uint8_t qtag_nest_0_profile_1;
        uint8_t qtag_nest_1_profile_1;
        err = ag_drv_rnr_quad_parser_qtag_nest_prof1_get(parm[1].value.unumber, &qtag_nest_0_profile_1, &qtag_nest_1_profile_1);
        bdmf_session_print(session, "qtag_nest_0_profile_1 = %u (0x%x)\n", qtag_nest_0_profile_1, qtag_nest_0_profile_1);
        bdmf_session_print(session, "qtag_nest_1_profile_1 = %u (0x%x)\n", qtag_nest_1_profile_1, qtag_nest_1_profile_1);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof2:
    {
        uint8_t qtag_nest_0_profile_2;
        uint8_t qtag_nest_1_profile_2;
        err = ag_drv_rnr_quad_parser_qtag_nest_prof2_get(parm[1].value.unumber, &qtag_nest_0_profile_2, &qtag_nest_1_profile_2);
        bdmf_session_print(session, "qtag_nest_0_profile_2 = %u (0x%x)\n", qtag_nest_0_profile_2, qtag_nest_0_profile_2);
        bdmf_session_print(session, "qtag_nest_1_profile_2 = %u (0x%x)\n", qtag_nest_1_profile_2, qtag_nest_1_profile_2);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol0:
    {
        uint8_t user_ip_prot_0;
        err = ag_drv_rnr_quad_parser_ip_protocol0_get(parm[1].value.unumber, &user_ip_prot_0);
        bdmf_session_print(session, "user_ip_prot_0 = %u (0x%x)\n", user_ip_prot_0, user_ip_prot_0);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol1:
    {
        uint8_t user_ip_prot_1;
        err = ag_drv_rnr_quad_parser_ip_protocol1_get(parm[1].value.unumber, &user_ip_prot_1);
        bdmf_session_print(session, "user_ip_prot_1 = %u (0x%x)\n", user_ip_prot_1, user_ip_prot_1);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol2:
    {
        uint8_t user_ip_prot_2;
        err = ag_drv_rnr_quad_parser_ip_protocol2_get(parm[1].value.unumber, &user_ip_prot_2);
        bdmf_session_print(session, "user_ip_prot_2 = %u (0x%x)\n", user_ip_prot_2, user_ip_prot_2);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol3:
    {
        uint8_t user_ip_prot_3;
        err = ag_drv_rnr_quad_parser_ip_protocol3_get(parm[1].value.unumber, &user_ip_prot_3);
        bdmf_session_print(session, "user_ip_prot_3 = %u (0x%x)\n", user_ip_prot_3, user_ip_prot_3);
        break;
    }
    case cli_rnr_quad_parser_da_filter:
    {
        rnr_quad_parser_da_filter parser_da_filter;
        err = ag_drv_rnr_quad_parser_da_filter_get(parm[1].value.unumber, &parser_da_filter);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", parser_da_filter.da_filt_msb, parser_da_filter.da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_lsb);
        bdmf_session_print(session, "da_filt_mask_msb = %u (0x%x)\n", parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_msb);
        bdmf_session_print(session, "da_filt_mask_l = %u (0x%x)\n", parser_da_filter.da_filt_mask_l, parser_da_filter.da_filt_mask_l);
        break;
    }
    case cli_rnr_quad_parser_da_filter1:
    {
        rnr_quad_parser_da_filter parser_da_filter;
        err = ag_drv_rnr_quad_parser_da_filter1_get(parm[1].value.unumber, &parser_da_filter);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", parser_da_filter.da_filt_msb, parser_da_filter.da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_lsb);
        bdmf_session_print(session, "da_filt_mask_msb = %u (0x%x)\n", parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_msb);
        bdmf_session_print(session, "da_filt_mask_l = %u (0x%x)\n", parser_da_filter.da_filt_mask_l, parser_da_filter.da_filt_mask_l);
        break;
    }
    case cli_rnr_quad_parser_da_filter2:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter2_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter3:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter3_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter4:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter4_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter5:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter5_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter6:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter6_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter7:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter7_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter8:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        err = ag_drv_rnr_quad_parser_da_filter8_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u (0x%x)\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u (0x%x)\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_da_filter_valid:
    {
        rnr_quad_da_filter_valid da_filter_valid;
        err = ag_drv_rnr_quad_da_filter_valid_get(parm[1].value.unumber, &da_filter_valid);
        bdmf_session_print(session, "da_filt0_valid = %u (0x%x)\n", da_filter_valid.da_filt0_valid, da_filter_valid.da_filt0_valid);
        bdmf_session_print(session, "da_filt1_valid = %u (0x%x)\n", da_filter_valid.da_filt1_valid, da_filter_valid.da_filt1_valid);
        bdmf_session_print(session, "da_filt2_valid = %u (0x%x)\n", da_filter_valid.da_filt2_valid, da_filter_valid.da_filt2_valid);
        bdmf_session_print(session, "da_filt3_valid = %u (0x%x)\n", da_filter_valid.da_filt3_valid, da_filter_valid.da_filt3_valid);
        bdmf_session_print(session, "da_filt4_valid = %u (0x%x)\n", da_filter_valid.da_filt4_valid, da_filter_valid.da_filt4_valid);
        bdmf_session_print(session, "da_filt5_valid = %u (0x%x)\n", da_filter_valid.da_filt5_valid, da_filter_valid.da_filt5_valid);
        bdmf_session_print(session, "da_filt6_valid = %u (0x%x)\n", da_filter_valid.da_filt6_valid, da_filter_valid.da_filt6_valid);
        bdmf_session_print(session, "da_filt7_valid = %u (0x%x)\n", da_filter_valid.da_filt7_valid, da_filter_valid.da_filt7_valid);
        bdmf_session_print(session, "da_filt8_valid = %u (0x%x)\n", da_filter_valid.da_filt8_valid, da_filter_valid.da_filt8_valid);
        break;
    }
    case cli_rnr_quad_exception_bits:
    {
        uint16_t exception_en;
        err = ag_drv_rnr_quad_exception_bits_get(parm[1].value.unumber, &exception_en);
        bdmf_session_print(session, "exception_en = %u (0x%x)\n", exception_en, exception_en);
        break;
    }
    case cli_rnr_quad_tcp_flags:
    {
        uint8_t tcp_flags_filt;
        err = ag_drv_rnr_quad_tcp_flags_get(parm[1].value.unumber, &tcp_flags_filt);
        bdmf_session_print(session, "tcp_flags_filt = %u (0x%x)\n", tcp_flags_filt, tcp_flags_filt);
        break;
    }
    case cli_rnr_quad_profile_us:
    {
        uint8_t profile_us;
        err = ag_drv_rnr_quad_profile_us_get(parm[1].value.unumber, &profile_us);
        bdmf_session_print(session, "profile_us = %u (0x%x)\n", profile_us, profile_us);
        break;
    }
    case cli_rnr_quad_parser_snap_conf:
    {
        rnr_quad_parser_snap_conf parser_snap_conf;
        err = ag_drv_rnr_quad_parser_snap_conf_get(parm[1].value.unumber, &parser_snap_conf);
        bdmf_session_print(session, "code = %u (0x%x)\n", parser_snap_conf.code, parser_snap_conf.code);
        bdmf_session_print(session, "en_rfc1042 = %u (0x%x)\n", parser_snap_conf.en_rfc1042, parser_snap_conf.en_rfc1042);
        bdmf_session_print(session, "en_8021q = %u (0x%x)\n", parser_snap_conf.en_8021q, parser_snap_conf.en_8021q);
        break;
    }
    case cli_rnr_quad_parser_ipv6_filter:
    {
        rnr_quad_parser_ipv6_filter parser_ipv6_filter;
        err = ag_drv_rnr_quad_parser_ipv6_filter_get(parm[1].value.unumber, &parser_ipv6_filter);
        bdmf_session_print(session, "hop_by_hop_match = %u (0x%x)\n", parser_ipv6_filter.hop_by_hop_match, parser_ipv6_filter.hop_by_hop_match);
        bdmf_session_print(session, "routing_eh = %u (0x%x)\n", parser_ipv6_filter.routing_eh, parser_ipv6_filter.routing_eh);
        bdmf_session_print(session, "dest_opt_eh = %u (0x%x)\n", parser_ipv6_filter.dest_opt_eh, parser_ipv6_filter.dest_opt_eh);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_eng:
    {
        uint32_t cfg;
        err = ag_drv_rnr_quad_parser_core_configuration_eng_get(parm[1].value.unumber, &cfg);
        bdmf_session_print(session, "cfg = %u (0x%x)\n", cfg, cfg);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code:
    {
        uint16_t ppp_code_0;
        uint16_t ppp_code_1;
        err = ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(parm[1].value.unumber, &ppp_code_0, &ppp_code_1);
        bdmf_session_print(session, "ppp_code_0 = %u (0x%x)\n", ppp_code_0, ppp_code_0);
        bdmf_session_print(session, "ppp_code_1 = %u (0x%x)\n", ppp_code_1, ppp_code_1);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_qtag_ethtype:
    {
        uint16_t ethtype_qtag_0;
        uint16_t ethtype_qtag_1;
        err = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(parm[1].value.unumber, &ethtype_qtag_0, &ethtype_qtag_1);
        bdmf_session_print(session, "ethtype_qtag_0 = %u (0x%x)\n", ethtype_qtag_0, ethtype_qtag_0);
        bdmf_session_print(session, "ethtype_qtag_1 = %u (0x%x)\n", ethtype_qtag_1, ethtype_qtag_1);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_user_ethtype_0_1:
    {
        uint16_t ethype_0;
        uint16_t ethype_1;
        err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(parm[1].value.unumber, &ethype_0, &ethype_1);
        bdmf_session_print(session, "ethype_0 = %u (0x%x)\n", ethype_0, ethype_0);
        bdmf_session_print(session, "ethype_1 = %u (0x%x)\n", ethype_1, ethype_1);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_user_ethtype_2_3:
    {
        uint16_t ethype_2;
        uint16_t ethype_3;
        err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(parm[1].value.unumber, &ethype_2, &ethype_3);
        bdmf_session_print(session, "ethype_2 = %u (0x%x)\n", ethype_2, ethype_2);
        bdmf_session_print(session, "ethype_3 = %u (0x%x)\n", ethype_3, ethype_3);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_user_ethtype_config:
    {
        rnr_quad_parser_core_configuration_user_ethtype_config parser_core_configuration_user_ethtype_config;
        err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(parm[1].value.unumber, &parser_core_configuration_user_ethtype_config);
        bdmf_session_print(session, "ethtype_user_prot_0 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_0, parser_core_configuration_user_ethtype_config.ethtype_user_prot_0);
        bdmf_session_print(session, "ethtype_user_prot_1 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_1, parser_core_configuration_user_ethtype_config.ethtype_user_prot_1);
        bdmf_session_print(session, "ethtype_user_prot_2 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_2, parser_core_configuration_user_ethtype_config.ethtype_user_prot_2);
        bdmf_session_print(session, "ethtype_user_prot_3 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_3, parser_core_configuration_user_ethtype_config.ethtype_user_prot_3);
        bdmf_session_print(session, "ethtype_user_en = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_en, parser_core_configuration_user_ethtype_config.ethtype_user_en);
        bdmf_session_print(session, "ethtype_user_offset_0 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_0, parser_core_configuration_user_ethtype_config.ethtype_user_offset_0);
        bdmf_session_print(session, "ethtype_user_offset_1 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_1, parser_core_configuration_user_ethtype_config.ethtype_user_offset_1);
        bdmf_session_print(session, "ethtype_user_offset_2 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_2, parser_core_configuration_user_ethtype_config.ethtype_user_offset_2);
        bdmf_session_print(session, "ethtype_user_offset_3 = %u (0x%x)\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_3, parser_core_configuration_user_ethtype_config.ethtype_user_offset_3);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1:
    {
        rnr_quad_da_filter_valid da_filter_valid;
        err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(parm[1].value.unumber, &da_filter_valid);
        bdmf_session_print(session, "da_filt0_valid = %u (0x%x)\n", da_filter_valid.da_filt0_valid, da_filter_valid.da_filt0_valid);
        bdmf_session_print(session, "da_filt1_valid = %u (0x%x)\n", da_filter_valid.da_filt1_valid, da_filter_valid.da_filt1_valid);
        bdmf_session_print(session, "da_filt2_valid = %u (0x%x)\n", da_filter_valid.da_filt2_valid, da_filter_valid.da_filt2_valid);
        bdmf_session_print(session, "da_filt3_valid = %u (0x%x)\n", da_filter_valid.da_filt3_valid, da_filter_valid.da_filt3_valid);
        bdmf_session_print(session, "da_filt4_valid = %u (0x%x)\n", da_filter_valid.da_filt4_valid, da_filter_valid.da_filt4_valid);
        bdmf_session_print(session, "da_filt5_valid = %u (0x%x)\n", da_filter_valid.da_filt5_valid, da_filter_valid.da_filt5_valid);
        bdmf_session_print(session, "da_filt6_valid = %u (0x%x)\n", da_filter_valid.da_filt6_valid, da_filter_valid.da_filt6_valid);
        bdmf_session_print(session, "da_filt7_valid = %u (0x%x)\n", da_filter_valid.da_filt7_valid, da_filter_valid.da_filt7_valid);
        bdmf_session_print(session, "da_filt8_valid = %u (0x%x)\n", da_filter_valid.da_filt8_valid, da_filter_valid.da_filt8_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2:
    {
        rnr_quad_da_filter_valid da_filter_valid;
        err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(parm[1].value.unumber, &da_filter_valid);
        bdmf_session_print(session, "da_filt0_valid = %u (0x%x)\n", da_filter_valid.da_filt0_valid, da_filter_valid.da_filt0_valid);
        bdmf_session_print(session, "da_filt1_valid = %u (0x%x)\n", da_filter_valid.da_filt1_valid, da_filter_valid.da_filt1_valid);
        bdmf_session_print(session, "da_filt2_valid = %u (0x%x)\n", da_filter_valid.da_filt2_valid, da_filter_valid.da_filt2_valid);
        bdmf_session_print(session, "da_filt3_valid = %u (0x%x)\n", da_filter_valid.da_filt3_valid, da_filter_valid.da_filt3_valid);
        bdmf_session_print(session, "da_filt4_valid = %u (0x%x)\n", da_filter_valid.da_filt4_valid, da_filter_valid.da_filt4_valid);
        bdmf_session_print(session, "da_filt5_valid = %u (0x%x)\n", da_filter_valid.da_filt5_valid, da_filter_valid.da_filt5_valid);
        bdmf_session_print(session, "da_filt6_valid = %u (0x%x)\n", da_filter_valid.da_filt6_valid, da_filter_valid.da_filt6_valid);
        bdmf_session_print(session, "da_filt7_valid = %u (0x%x)\n", da_filter_valid.da_filt7_valid, da_filter_valid.da_filt7_valid);
        bdmf_session_print(session, "da_filt8_valid = %u (0x%x)\n", da_filter_valid.da_filt8_valid, da_filter_valid.da_filt8_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_gre_protocol_cfg:
    {
        uint16_t gre_protocol;
        err = ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(parm[1].value.unumber, &gre_protocol);
        bdmf_session_print(session, "gre_protocol = %u (0x%x)\n", gre_protocol, gre_protocol);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_prop_tag_cfg:
    {
        rnr_quad_parser_core_configuration_prop_tag_cfg parser_core_configuration_prop_tag_cfg;
        err = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(parm[1].value.unumber, &parser_core_configuration_prop_tag_cfg);
        bdmf_session_print(session, "size_profile_0 = %u (0x%x)\n", parser_core_configuration_prop_tag_cfg.size_profile_0, parser_core_configuration_prop_tag_cfg.size_profile_0);
        bdmf_session_print(session, "size_profile_1 = %u (0x%x)\n", parser_core_configuration_prop_tag_cfg.size_profile_1, parser_core_configuration_prop_tag_cfg.size_profile_1);
        bdmf_session_print(session, "size_profile_2 = %u (0x%x)\n", parser_core_configuration_prop_tag_cfg.size_profile_2, parser_core_configuration_prop_tag_cfg.size_profile_2);
        bdmf_session_print(session, "pre_da_dprofile_0 = %u (0x%x)\n", parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0);
        bdmf_session_print(session, "pre_da_dprofile_1 = %u (0x%x)\n", parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1);
        bdmf_session_print(session, "pre_da_dprofile_2 = %u (0x%x)\n", parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2);
        break;
    }
    case cli_rnr_quad_general_config_dma_arb_cfg:
    {
        bdmf_boolean use_fifo_for_ddr_only;
        bdmf_boolean token_arbiter_is_rr;
        bdmf_boolean chicken_no_flowctrl;
        uint8_t congest_threshold;
        err = ag_drv_rnr_quad_general_config_dma_arb_cfg_get(parm[1].value.unumber, &use_fifo_for_ddr_only, &token_arbiter_is_rr, &chicken_no_flowctrl, &congest_threshold);
        bdmf_session_print(session, "use_fifo_for_ddr_only = %u (0x%x)\n", use_fifo_for_ddr_only, use_fifo_for_ddr_only);
        bdmf_session_print(session, "token_arbiter_is_rr = %u (0x%x)\n", token_arbiter_is_rr, token_arbiter_is_rr);
        bdmf_session_print(session, "chicken_no_flowctrl = %u (0x%x)\n", chicken_no_flowctrl, chicken_no_flowctrl);
        bdmf_session_print(session, "congest_threshold = %u (0x%x)\n", congest_threshold, congest_threshold);
        break;
    }
    case cli_rnr_quad_general_config_psram0_base:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram0_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram1_base:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram1_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram2_base:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram2_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram3_base:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram3_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr0_base:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_ddr0_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr1_base:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_ddr1_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram0_mask:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram0_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram1_mask:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram1_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram2_mask:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram2_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram3_mask:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_psram3_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr0_mask:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_ddr0_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr1_mask:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_general_config_ddr1_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_profiling_config:
    {
        rnr_quad_general_config_profiling_config general_config_profiling_config;
        err = ag_drv_rnr_quad_general_config_profiling_config_get(parm[1].value.unumber, &general_config_profiling_config);
        bdmf_session_print(session, "counter_lsb_sel = %u (0x%x)\n", general_config_profiling_config.counter_lsb_sel, general_config_profiling_config.counter_lsb_sel);
        bdmf_session_print(session, "enable_trace_core_0 = %u (0x%x)\n", general_config_profiling_config.enable_trace_core_0, general_config_profiling_config.enable_trace_core_0);
        bdmf_session_print(session, "enable_trace_core_1 = %u (0x%x)\n", general_config_profiling_config.enable_trace_core_1, general_config_profiling_config.enable_trace_core_1);
        bdmf_session_print(session, "enable_trace_core_2 = %u (0x%x)\n", general_config_profiling_config.enable_trace_core_2, general_config_profiling_config.enable_trace_core_2);
        bdmf_session_print(session, "enable_trace_core_3 = %u (0x%x)\n", general_config_profiling_config.enable_trace_core_3, general_config_profiling_config.enable_trace_core_3);
        bdmf_session_print(session, "enable_trace_core_4 = %u (0x%x)\n", general_config_profiling_config.enable_trace_core_4, general_config_profiling_config.enable_trace_core_4);
        bdmf_session_print(session, "enable_trace_core_5 = %u (0x%x)\n", general_config_profiling_config.enable_trace_core_5, general_config_profiling_config.enable_trace_core_5);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_0_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_1_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_2_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_3_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_4_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_5_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_6_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_7_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        err = ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "thread = %u (0x%x)\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_gen_cfg:
    {
        uint16_t handler_addr;
        uint16_t update_pc_value;
        err = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(parm[1].value.unumber, &handler_addr, &update_pc_value);
        bdmf_session_print(session, "handler_addr = %u (0x%x)\n", handler_addr, handler_addr);
        bdmf_session_print(session, "update_pc_value = %u (0x%x)\n", update_pc_value, update_pc_value);
        break;
    }
    case cli_rnr_quad_general_config_powersave_config:
    {
        rnr_quad_general_config_powersave_config general_config_powersave_config;
        err = ag_drv_rnr_quad_general_config_powersave_config_get(parm[1].value.unumber, &general_config_powersave_config);
        bdmf_session_print(session, "time_counter = %u (0x%x)\n", general_config_powersave_config.time_counter, general_config_powersave_config.time_counter);
        bdmf_session_print(session, "enable_powersave_core_0 = %u (0x%x)\n", general_config_powersave_config.enable_powersave_core_0, general_config_powersave_config.enable_powersave_core_0);
        bdmf_session_print(session, "enable_powersave_core_1 = %u (0x%x)\n", general_config_powersave_config.enable_powersave_core_1, general_config_powersave_config.enable_powersave_core_1);
        bdmf_session_print(session, "enable_powersave_core_2 = %u (0x%x)\n", general_config_powersave_config.enable_powersave_core_2, general_config_powersave_config.enable_powersave_core_2);
        bdmf_session_print(session, "enable_powersave_core_3 = %u (0x%x)\n", general_config_powersave_config.enable_powersave_core_3, general_config_powersave_config.enable_powersave_core_3);
        bdmf_session_print(session, "enable_powersave_core_4 = %u (0x%x)\n", general_config_powersave_config.enable_powersave_core_4, general_config_powersave_config.enable_powersave_core_4);
        bdmf_session_print(session, "enable_powersave_core_5 = %u (0x%x)\n", general_config_powersave_config.enable_powersave_core_5, general_config_powersave_config.enable_powersave_core_5);
        break;
    }
    case cli_rnr_quad_general_config_powersave_status:
    {
        rnr_quad_general_config_powersave_status general_config_powersave_status;
        err = ag_drv_rnr_quad_general_config_powersave_status_get(parm[1].value.unumber, &general_config_powersave_status);
        bdmf_session_print(session, "core_0_status = %u (0x%x)\n", general_config_powersave_status.core_0_status, general_config_powersave_status.core_0_status);
        bdmf_session_print(session, "core_1_status = %u (0x%x)\n", general_config_powersave_status.core_1_status, general_config_powersave_status.core_1_status);
        bdmf_session_print(session, "core_2_status = %u (0x%x)\n", general_config_powersave_status.core_2_status, general_config_powersave_status.core_2_status);
        bdmf_session_print(session, "core_3_status = %u (0x%x)\n", general_config_powersave_status.core_3_status, general_config_powersave_status.core_3_status);
        bdmf_session_print(session, "core_4_status = %u (0x%x)\n", general_config_powersave_status.core_4_status, general_config_powersave_status.core_4_status);
        bdmf_session_print(session, "core_5_status = %u (0x%x)\n", general_config_powersave_status.core_5_status, general_config_powersave_status.core_5_status);
        break;
    }
    case cli_rnr_quad_debug_fifo_config:
    {
        rnr_quad_debug_fifo_config debug_fifo_config;
        err = ag_drv_rnr_quad_debug_fifo_config_get(parm[1].value.unumber, &debug_fifo_config);
        bdmf_session_print(session, "psram_hdr_sw_rst = %u (0x%x)\n", debug_fifo_config.psram_hdr_sw_rst, debug_fifo_config.psram_hdr_sw_rst);
        bdmf_session_print(session, "psram_data_sw_rst = %u (0x%x)\n", debug_fifo_config.psram_data_sw_rst, debug_fifo_config.psram_data_sw_rst);
        bdmf_session_print(session, "ddr_hdr_sw_rst = %u (0x%x)\n", debug_fifo_config.ddr_hdr_sw_rst, debug_fifo_config.ddr_hdr_sw_rst);
        bdmf_session_print(session, "psram_hdr_sw_rd_addr = %u (0x%x)\n", debug_fifo_config.psram_hdr_sw_rd_addr, debug_fifo_config.psram_hdr_sw_rd_addr);
        bdmf_session_print(session, "psram_data_sw_rd_addr = %u (0x%x)\n", debug_fifo_config.psram_data_sw_rd_addr, debug_fifo_config.psram_data_sw_rd_addr);
        bdmf_session_print(session, "ddr_hdr_sw_rd_addr = %u (0x%x)\n", debug_fifo_config.ddr_hdr_sw_rd_addr, debug_fifo_config.ddr_hdr_sw_rd_addr);
        break;
    }
    case cli_rnr_quad_debug_psram_hdr_fifo_status:
    {
        rnr_quad_debug_psram_hdr_fifo_status debug_psram_hdr_fifo_status;
        err = ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(parm[1].value.unumber, &debug_psram_hdr_fifo_status);
        bdmf_session_print(session, "full = %u (0x%x)\n", debug_psram_hdr_fifo_status.full, debug_psram_hdr_fifo_status.full);
        bdmf_session_print(session, "empty = %u (0x%x)\n", debug_psram_hdr_fifo_status.empty, debug_psram_hdr_fifo_status.empty);
        bdmf_session_print(session, "push_wr_cntr = %u (0x%x)\n", debug_psram_hdr_fifo_status.push_wr_cntr, debug_psram_hdr_fifo_status.push_wr_cntr);
        bdmf_session_print(session, "pop_rd_cntr = %u (0x%x)\n", debug_psram_hdr_fifo_status.pop_rd_cntr, debug_psram_hdr_fifo_status.pop_rd_cntr);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", debug_psram_hdr_fifo_status.used_words, debug_psram_hdr_fifo_status.used_words);
        break;
    }
    case cli_rnr_quad_debug_psram_data_fifo_status:
    {
        rnr_quad_debug_psram_data_fifo_status debug_psram_data_fifo_status;
        err = ag_drv_rnr_quad_debug_psram_data_fifo_status_get(parm[1].value.unumber, &debug_psram_data_fifo_status);
        bdmf_session_print(session, "full = %u (0x%x)\n", debug_psram_data_fifo_status.full, debug_psram_data_fifo_status.full);
        bdmf_session_print(session, "empty = %u (0x%x)\n", debug_psram_data_fifo_status.empty, debug_psram_data_fifo_status.empty);
        bdmf_session_print(session, "almost_full = %u (0x%x)\n", debug_psram_data_fifo_status.almost_full, debug_psram_data_fifo_status.almost_full);
        bdmf_session_print(session, "push_wr_cntr = %u (0x%x)\n", debug_psram_data_fifo_status.push_wr_cntr, debug_psram_data_fifo_status.push_wr_cntr);
        bdmf_session_print(session, "pop_rd_cntr = %u (0x%x)\n", debug_psram_data_fifo_status.pop_rd_cntr, debug_psram_data_fifo_status.pop_rd_cntr);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", debug_psram_data_fifo_status.used_words, debug_psram_data_fifo_status.used_words);
        break;
    }
    case cli_rnr_quad_debug_ddr_hdr_fifo_status:
    {
        rnr_quad_debug_ddr_hdr_fifo_status debug_ddr_hdr_fifo_status;
        err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(parm[1].value.unumber, &debug_ddr_hdr_fifo_status);
        bdmf_session_print(session, "full = %u (0x%x)\n", debug_ddr_hdr_fifo_status.full, debug_ddr_hdr_fifo_status.full);
        bdmf_session_print(session, "empty = %u (0x%x)\n", debug_ddr_hdr_fifo_status.empty, debug_ddr_hdr_fifo_status.empty);
        bdmf_session_print(session, "push_wr_cntr = %u (0x%x)\n", debug_ddr_hdr_fifo_status.push_wr_cntr, debug_ddr_hdr_fifo_status.push_wr_cntr);
        bdmf_session_print(session, "pop_rd_cntr = %u (0x%x)\n", debug_ddr_hdr_fifo_status.pop_rd_cntr, debug_ddr_hdr_fifo_status.pop_rd_cntr);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", debug_ddr_hdr_fifo_status.used_words, debug_ddr_hdr_fifo_status.used_words);
        break;
    }
    case cli_rnr_quad_debug_ddr_data_fifo_status:
    {
        rnr_quad_debug_ddr_data_fifo_status debug_ddr_data_fifo_status;
        err = ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(parm[1].value.unumber, &debug_ddr_data_fifo_status);
        bdmf_session_print(session, "full = %u (0x%x)\n", debug_ddr_data_fifo_status.full, debug_ddr_data_fifo_status.full);
        bdmf_session_print(session, "empty = %u (0x%x)\n", debug_ddr_data_fifo_status.empty, debug_ddr_data_fifo_status.empty);
        bdmf_session_print(session, "almost_full = %u (0x%x)\n", debug_ddr_data_fifo_status.almost_full, debug_ddr_data_fifo_status.almost_full);
        bdmf_session_print(session, "wr_cntr = %u (0x%x)\n", debug_ddr_data_fifo_status.wr_cntr, debug_ddr_data_fifo_status.wr_cntr);
        bdmf_session_print(session, "rd_cntr = %u (0x%x)\n", debug_ddr_data_fifo_status.rd_cntr, debug_ddr_data_fifo_status.rd_cntr);
        break;
    }
    case cli_rnr_quad_debug_ddr_data_fifo_status2:
    {
        uint8_t read_addr;
        uint16_t used_words;
        err = ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(parm[1].value.unumber, &read_addr, &used_words);
        bdmf_session_print(session, "read_addr = %u (0x%x)\n", read_addr, read_addr);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        break;
    }
    case cli_rnr_quad_debug_psram_hdr_fifo_data1:
    {
        uint32_t data;
        err = ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_psram_hdr_fifo_data2:
    {
        uint32_t data;
        err = ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_psram_data_fifo_data1:
    {
        uint32_t data;
        err = ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_psram_data_fifo_data2:
    {
        uint32_t data;
        err = ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_ddr_hdr_fifo_data1:
    {
        uint32_t data;
        err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_ddr_hdr_fifo_data2:
    {
        uint32_t data;
        err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_rnr_quad_ext_flowctrl_config_token_val:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode:
    {
        uint32_t val;
        err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_rnr_quad_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t quad_idx = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint16_t vid_0=gtmv(m, 12);
        bdmf_boolean vid_0_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid0_set(%u %u %u)\n", quad_idx, vid_0, vid_0_en);
        if(!err) ag_drv_rnr_quad_parser_vid0_set(quad_idx, vid_0, vid_0_en);
        if(!err) ag_drv_rnr_quad_parser_vid0_get( quad_idx, &vid_0, &vid_0_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid0_get(%u %u %u)\n", quad_idx, vid_0, vid_0_en);
        if(err || vid_0!=gtmv(m, 12) || vid_0_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_1=gtmv(m, 12);
        bdmf_boolean vid_1_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid1_set(%u %u %u)\n", quad_idx, vid_1, vid_1_en);
        if(!err) ag_drv_rnr_quad_parser_vid1_set(quad_idx, vid_1, vid_1_en);
        if(!err) ag_drv_rnr_quad_parser_vid1_get( quad_idx, &vid_1, &vid_1_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid1_get(%u %u %u)\n", quad_idx, vid_1, vid_1_en);
        if(err || vid_1!=gtmv(m, 12) || vid_1_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_2=gtmv(m, 12);
        bdmf_boolean vid_2_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid2_set(%u %u %u)\n", quad_idx, vid_2, vid_2_en);
        if(!err) ag_drv_rnr_quad_parser_vid2_set(quad_idx, vid_2, vid_2_en);
        if(!err) ag_drv_rnr_quad_parser_vid2_get( quad_idx, &vid_2, &vid_2_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid2_get(%u %u %u)\n", quad_idx, vid_2, vid_2_en);
        if(err || vid_2!=gtmv(m, 12) || vid_2_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_3=gtmv(m, 12);
        bdmf_boolean vid_3_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid3_set(%u %u %u)\n", quad_idx, vid_3, vid_3_en);
        if(!err) ag_drv_rnr_quad_parser_vid3_set(quad_idx, vid_3, vid_3_en);
        if(!err) ag_drv_rnr_quad_parser_vid3_get( quad_idx, &vid_3, &vid_3_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid3_get(%u %u %u)\n", quad_idx, vid_3, vid_3_en);
        if(err || vid_3!=gtmv(m, 12) || vid_3_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_4=gtmv(m, 12);
        bdmf_boolean vid_4_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid4_set(%u %u %u)\n", quad_idx, vid_4, vid_4_en);
        if(!err) ag_drv_rnr_quad_parser_vid4_set(quad_idx, vid_4, vid_4_en);
        if(!err) ag_drv_rnr_quad_parser_vid4_get( quad_idx, &vid_4, &vid_4_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid4_get(%u %u %u)\n", quad_idx, vid_4, vid_4_en);
        if(err || vid_4!=gtmv(m, 12) || vid_4_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_5=gtmv(m, 12);
        bdmf_boolean vid_5_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid5_set(%u %u %u)\n", quad_idx, vid_5, vid_5_en);
        if(!err) ag_drv_rnr_quad_parser_vid5_set(quad_idx, vid_5, vid_5_en);
        if(!err) ag_drv_rnr_quad_parser_vid5_get( quad_idx, &vid_5, &vid_5_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid5_get(%u %u %u)\n", quad_idx, vid_5, vid_5_en);
        if(err || vid_5!=gtmv(m, 12) || vid_5_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_6=gtmv(m, 12);
        bdmf_boolean vid_6_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid6_set(%u %u %u)\n", quad_idx, vid_6, vid_6_en);
        if(!err) ag_drv_rnr_quad_parser_vid6_set(quad_idx, vid_6, vid_6_en);
        if(!err) ag_drv_rnr_quad_parser_vid6_get( quad_idx, &vid_6, &vid_6_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid6_get(%u %u %u)\n", quad_idx, vid_6, vid_6_en);
        if(err || vid_6!=gtmv(m, 12) || vid_6_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t vid_7=gtmv(m, 12);
        bdmf_boolean vid_7_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid7_set(%u %u %u)\n", quad_idx, vid_7, vid_7_en);
        if(!err) ag_drv_rnr_quad_parser_vid7_set(quad_idx, vid_7, vid_7_en);
        if(!err) ag_drv_rnr_quad_parser_vid7_get( quad_idx, &vid_7, &vid_7_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid7_get(%u %u %u)\n", quad_idx, vid_7, vid_7_en);
        if(err || vid_7!=gtmv(m, 12) || vid_7_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_ip0 parser_ip0 = {.ip_address=gtmv(m, 32), .ip_address_mask=gtmv(m, 32), .ip_filter0_dip_en=gtmv(m, 1), .ip_filter0_valid=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip0_set(%u %u %u %u %u)\n", quad_idx, parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        if(!err) ag_drv_rnr_quad_parser_ip0_set(quad_idx, &parser_ip0);
        if(!err) ag_drv_rnr_quad_parser_ip0_get( quad_idx, &parser_ip0);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip0_get(%u %u %u %u %u)\n", quad_idx, parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        if(err || parser_ip0.ip_address!=gtmv(m, 32) || parser_ip0.ip_address_mask!=gtmv(m, 32) || parser_ip0.ip_filter0_dip_en!=gtmv(m, 1) || parser_ip0.ip_filter0_valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_ip0 parser_ip0 = {.ip_address=gtmv(m, 32), .ip_address_mask=gtmv(m, 32), .ip_filter0_dip_en=gtmv(m, 1), .ip_filter0_valid=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip1_set(%u %u %u %u %u)\n", quad_idx, parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        if(!err) ag_drv_rnr_quad_parser_ip1_set(quad_idx, &parser_ip0);
        if(!err) ag_drv_rnr_quad_parser_ip1_get( quad_idx, &parser_ip0);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip1_get(%u %u %u %u %u)\n", quad_idx, parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        if(err || parser_ip0.ip_address!=gtmv(m, 32) || parser_ip0.ip_address_mask!=gtmv(m, 32) || parser_ip0.ip_filter0_dip_en!=gtmv(m, 1) || parser_ip0.ip_filter0_valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t hard_nest_profile=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(%u %u)\n", quad_idx, hard_nest_profile);
        if(!err) ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(quad_idx, hard_nest_profile);
        if(!err) ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get( quad_idx, &hard_nest_profile);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(%u %u)\n", quad_idx, hard_nest_profile);
        if(err || hard_nest_profile!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t hard_nest_profile=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(%u %u)\n", quad_idx, hard_nest_profile);
        if(!err) ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(quad_idx, hard_nest_profile);
        if(!err) ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get( quad_idx, &hard_nest_profile);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(%u %u)\n", quad_idx, hard_nest_profile);
        if(err || hard_nest_profile!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t hard_nest_profile=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(%u %u)\n", quad_idx, hard_nest_profile);
        if(!err) ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(quad_idx, hard_nest_profile);
        if(!err) ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get( quad_idx, &hard_nest_profile);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(%u %u)\n", quad_idx, hard_nest_profile);
        if(err || hard_nest_profile!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t qtag_nest_0_profile_0=gtmv(m, 3);
        uint8_t qtag_nest_1_profile_0=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof0_set(%u %u %u)\n", quad_idx, qtag_nest_0_profile_0, qtag_nest_1_profile_0);
        if(!err) ag_drv_rnr_quad_parser_qtag_nest_prof0_set(quad_idx, qtag_nest_0_profile_0, qtag_nest_1_profile_0);
        if(!err) ag_drv_rnr_quad_parser_qtag_nest_prof0_get( quad_idx, &qtag_nest_0_profile_0, &qtag_nest_1_profile_0);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof0_get(%u %u %u)\n", quad_idx, qtag_nest_0_profile_0, qtag_nest_1_profile_0);
        if(err || qtag_nest_0_profile_0!=gtmv(m, 3) || qtag_nest_1_profile_0!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t qtag_nest_0_profile_1=gtmv(m, 3);
        uint8_t qtag_nest_1_profile_1=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof1_set(%u %u %u)\n", quad_idx, qtag_nest_0_profile_1, qtag_nest_1_profile_1);
        if(!err) ag_drv_rnr_quad_parser_qtag_nest_prof1_set(quad_idx, qtag_nest_0_profile_1, qtag_nest_1_profile_1);
        if(!err) ag_drv_rnr_quad_parser_qtag_nest_prof1_get( quad_idx, &qtag_nest_0_profile_1, &qtag_nest_1_profile_1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof1_get(%u %u %u)\n", quad_idx, qtag_nest_0_profile_1, qtag_nest_1_profile_1);
        if(err || qtag_nest_0_profile_1!=gtmv(m, 3) || qtag_nest_1_profile_1!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t qtag_nest_0_profile_2=gtmv(m, 3);
        uint8_t qtag_nest_1_profile_2=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof2_set(%u %u %u)\n", quad_idx, qtag_nest_0_profile_2, qtag_nest_1_profile_2);
        if(!err) ag_drv_rnr_quad_parser_qtag_nest_prof2_set(quad_idx, qtag_nest_0_profile_2, qtag_nest_1_profile_2);
        if(!err) ag_drv_rnr_quad_parser_qtag_nest_prof2_get( quad_idx, &qtag_nest_0_profile_2, &qtag_nest_1_profile_2);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof2_get(%u %u %u)\n", quad_idx, qtag_nest_0_profile_2, qtag_nest_1_profile_2);
        if(err || qtag_nest_0_profile_2!=gtmv(m, 3) || qtag_nest_1_profile_2!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t user_ip_prot_0=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol0_set(%u %u)\n", quad_idx, user_ip_prot_0);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol0_set(quad_idx, user_ip_prot_0);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol0_get( quad_idx, &user_ip_prot_0);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol0_get(%u %u)\n", quad_idx, user_ip_prot_0);
        if(err || user_ip_prot_0!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t user_ip_prot_1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol1_set(%u %u)\n", quad_idx, user_ip_prot_1);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol1_set(quad_idx, user_ip_prot_1);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol1_get( quad_idx, &user_ip_prot_1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol1_get(%u %u)\n", quad_idx, user_ip_prot_1);
        if(err || user_ip_prot_1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t user_ip_prot_2=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol2_set(%u %u)\n", quad_idx, user_ip_prot_2);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol2_set(quad_idx, user_ip_prot_2);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol2_get( quad_idx, &user_ip_prot_2);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol2_get(%u %u)\n", quad_idx, user_ip_prot_2);
        if(err || user_ip_prot_2!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t user_ip_prot_3=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol3_set(%u %u)\n", quad_idx, user_ip_prot_3);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol3_set(quad_idx, user_ip_prot_3);
        if(!err) ag_drv_rnr_quad_parser_ip_protocol3_get( quad_idx, &user_ip_prot_3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol3_get(%u %u)\n", quad_idx, user_ip_prot_3);
        if(err || user_ip_prot_3!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_da_filter parser_da_filter = {.da_filt_msb=gtmv(m, 16), .da_filt_lsb=gtmv(m, 32), .da_filt_mask_msb=gtmv(m, 16), .da_filt_mask_l=gtmv(m, 32)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter_set(%u %u %u %u %u)\n", quad_idx, parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        if(!err) ag_drv_rnr_quad_parser_da_filter_set(quad_idx, &parser_da_filter);
        if(!err) ag_drv_rnr_quad_parser_da_filter_get( quad_idx, &parser_da_filter);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter_get(%u %u %u %u %u)\n", quad_idx, parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        if(err || parser_da_filter.da_filt_msb!=gtmv(m, 16) || parser_da_filter.da_filt_lsb!=gtmv(m, 32) || parser_da_filter.da_filt_mask_msb!=gtmv(m, 16) || parser_da_filter.da_filt_mask_l!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_da_filter parser_da_filter = {.da_filt_msb=gtmv(m, 16), .da_filt_lsb=gtmv(m, 32), .da_filt_mask_msb=gtmv(m, 16), .da_filt_mask_l=gtmv(m, 32)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter1_set(%u %u %u %u %u)\n", quad_idx, parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        if(!err) ag_drv_rnr_quad_parser_da_filter1_set(quad_idx, &parser_da_filter);
        if(!err) ag_drv_rnr_quad_parser_da_filter1_get( quad_idx, &parser_da_filter);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter1_get(%u %u %u %u %u)\n", quad_idx, parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        if(err || parser_da_filter.da_filt_msb!=gtmv(m, 16) || parser_da_filter.da_filt_lsb!=gtmv(m, 32) || parser_da_filter.da_filt_mask_msb!=gtmv(m, 16) || parser_da_filter.da_filt_mask_l!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter2_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter2_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter2_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter2_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter3_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter3_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter3_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter3_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter4_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter4_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter4_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter4_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter5_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter5_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter5_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter5_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter6_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter6_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter6_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter6_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter7_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter7_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter7_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter7_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t da_filt_msb=gtmv(m, 16);
        uint32_t da_filt_lsb=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter8_set(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter8_set(quad_idx, da_filt_msb, da_filt_lsb);
        if(!err) ag_drv_rnr_quad_parser_da_filter8_get( quad_idx, &da_filt_msb, &da_filt_lsb);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter8_get(%u %u %u)\n", quad_idx, da_filt_msb, da_filt_lsb);
        if(err || da_filt_msb!=gtmv(m, 16) || da_filt_lsb!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_da_filter_valid da_filter_valid = {.da_filt0_valid=gtmv(m, 1), .da_filt1_valid=gtmv(m, 1), .da_filt2_valid=gtmv(m, 1), .da_filt3_valid=gtmv(m, 1), .da_filt4_valid=gtmv(m, 1), .da_filt5_valid=gtmv(m, 1), .da_filt6_valid=gtmv(m, 1), .da_filt7_valid=gtmv(m, 1), .da_filt8_valid=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_da_filter_valid_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, da_filter_valid.da_filt8_valid);
        if(!err) ag_drv_rnr_quad_da_filter_valid_set(quad_idx, &da_filter_valid);
        if(!err) ag_drv_rnr_quad_da_filter_valid_get( quad_idx, &da_filter_valid);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_da_filter_valid_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, da_filter_valid.da_filt8_valid);
        if(err || da_filter_valid.da_filt0_valid!=gtmv(m, 1) || da_filter_valid.da_filt1_valid!=gtmv(m, 1) || da_filter_valid.da_filt2_valid!=gtmv(m, 1) || da_filter_valid.da_filt3_valid!=gtmv(m, 1) || da_filter_valid.da_filt4_valid!=gtmv(m, 1) || da_filter_valid.da_filt5_valid!=gtmv(m, 1) || da_filter_valid.da_filt6_valid!=gtmv(m, 1) || da_filter_valid.da_filt7_valid!=gtmv(m, 1) || da_filter_valid.da_filt8_valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t exception_en=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_exception_bits_set(%u %u)\n", quad_idx, exception_en);
        if(!err) ag_drv_rnr_quad_exception_bits_set(quad_idx, exception_en);
        if(!err) ag_drv_rnr_quad_exception_bits_get( quad_idx, &exception_en);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_exception_bits_get(%u %u)\n", quad_idx, exception_en);
        if(err || exception_en!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tcp_flags_filt=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_tcp_flags_set(%u %u)\n", quad_idx, tcp_flags_filt);
        if(!err) ag_drv_rnr_quad_tcp_flags_set(quad_idx, tcp_flags_filt);
        if(!err) ag_drv_rnr_quad_tcp_flags_get( quad_idx, &tcp_flags_filt);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_tcp_flags_get(%u %u)\n", quad_idx, tcp_flags_filt);
        if(err || tcp_flags_filt!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t profile_us=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_profile_us_set(%u %u)\n", quad_idx, profile_us);
        if(!err) ag_drv_rnr_quad_profile_us_set(quad_idx, profile_us);
        if(!err) ag_drv_rnr_quad_profile_us_get( quad_idx, &profile_us);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_profile_us_get(%u %u)\n", quad_idx, profile_us);
        if(err || profile_us!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_snap_conf parser_snap_conf = {.code=gtmv(m, 24), .en_rfc1042=gtmv(m, 1), .en_8021q=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_snap_conf_set(%u %u %u %u)\n", quad_idx, parser_snap_conf.code, parser_snap_conf.en_rfc1042, parser_snap_conf.en_8021q);
        if(!err) ag_drv_rnr_quad_parser_snap_conf_set(quad_idx, &parser_snap_conf);
        if(!err) ag_drv_rnr_quad_parser_snap_conf_get( quad_idx, &parser_snap_conf);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_snap_conf_get(%u %u %u %u)\n", quad_idx, parser_snap_conf.code, parser_snap_conf.en_rfc1042, parser_snap_conf.en_8021q);
        if(err || parser_snap_conf.code!=gtmv(m, 24) || parser_snap_conf.en_rfc1042!=gtmv(m, 1) || parser_snap_conf.en_8021q!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_ipv6_filter parser_ipv6_filter = {.hop_by_hop_match=gtmv(m, 1), .routing_eh=gtmv(m, 1), .dest_opt_eh=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ipv6_filter_set(%u %u %u %u)\n", quad_idx, parser_ipv6_filter.hop_by_hop_match, parser_ipv6_filter.routing_eh, parser_ipv6_filter.dest_opt_eh);
        if(!err) ag_drv_rnr_quad_parser_ipv6_filter_set(quad_idx, &parser_ipv6_filter);
        if(!err) ag_drv_rnr_quad_parser_ipv6_filter_get( quad_idx, &parser_ipv6_filter);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_ipv6_filter_get(%u %u %u %u)\n", quad_idx, parser_ipv6_filter.hop_by_hop_match, parser_ipv6_filter.routing_eh, parser_ipv6_filter.dest_opt_eh);
        if(err || parser_ipv6_filter.hop_by_hop_match!=gtmv(m, 1) || parser_ipv6_filter.routing_eh!=gtmv(m, 1) || parser_ipv6_filter.dest_opt_eh!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfg=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_eng_set(%u %u)\n", quad_idx, cfg);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_eng_set(quad_idx, cfg);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_eng_get( quad_idx, &cfg);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_eng_get(%u %u)\n", quad_idx, cfg);
        if(err || cfg!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ppp_code_0=gtmv(m, 16);
        uint16_t ppp_code_1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(%u %u %u)\n", quad_idx, ppp_code_0, ppp_code_1);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(quad_idx, ppp_code_0, ppp_code_1);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get( quad_idx, &ppp_code_0, &ppp_code_1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(%u %u %u)\n", quad_idx, ppp_code_0, ppp_code_1);
        if(err || ppp_code_0!=gtmv(m, 16) || ppp_code_1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ethtype_qtag_0=gtmv(m, 16);
        uint16_t ethtype_qtag_1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(%u %u %u)\n", quad_idx, ethtype_qtag_0, ethtype_qtag_1);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(quad_idx, ethtype_qtag_0, ethtype_qtag_1);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get( quad_idx, &ethtype_qtag_0, &ethtype_qtag_1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(%u %u %u)\n", quad_idx, ethtype_qtag_0, ethtype_qtag_1);
        if(err || ethtype_qtag_0!=gtmv(m, 16) || ethtype_qtag_1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ethype_0=gtmv(m, 16);
        uint16_t ethype_1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(%u %u %u)\n", quad_idx, ethype_0, ethype_1);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(quad_idx, ethype_0, ethype_1);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get( quad_idx, &ethype_0, &ethype_1);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(%u %u %u)\n", quad_idx, ethype_0, ethype_1);
        if(err || ethype_0!=gtmv(m, 16) || ethype_1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ethype_2=gtmv(m, 16);
        uint16_t ethype_3=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(%u %u %u)\n", quad_idx, ethype_2, ethype_3);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(quad_idx, ethype_2, ethype_3);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get( quad_idx, &ethype_2, &ethype_3);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(%u %u %u)\n", quad_idx, ethype_2, ethype_3);
        if(err || ethype_2!=gtmv(m, 16) || ethype_3!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_core_configuration_user_ethtype_config parser_core_configuration_user_ethtype_config = {.ethtype_user_prot_0=gtmv(m, 2), .ethtype_user_prot_1=gtmv(m, 2), .ethtype_user_prot_2=gtmv(m, 2), .ethtype_user_prot_3=gtmv(m, 2), .ethtype_user_en=gtmv(m, 4), .ethtype_user_offset_0=gtmv(m, 4), .ethtype_user_offset_1=gtmv(m, 4), .ethtype_user_offset_2=gtmv(m, 4), .ethtype_user_offset_3=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, parser_core_configuration_user_ethtype_config.ethtype_user_prot_0, parser_core_configuration_user_ethtype_config.ethtype_user_prot_1, parser_core_configuration_user_ethtype_config.ethtype_user_prot_2, parser_core_configuration_user_ethtype_config.ethtype_user_prot_3, parser_core_configuration_user_ethtype_config.ethtype_user_en, parser_core_configuration_user_ethtype_config.ethtype_user_offset_0, parser_core_configuration_user_ethtype_config.ethtype_user_offset_1, parser_core_configuration_user_ethtype_config.ethtype_user_offset_2, parser_core_configuration_user_ethtype_config.ethtype_user_offset_3);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(quad_idx, &parser_core_configuration_user_ethtype_config);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get( quad_idx, &parser_core_configuration_user_ethtype_config);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, parser_core_configuration_user_ethtype_config.ethtype_user_prot_0, parser_core_configuration_user_ethtype_config.ethtype_user_prot_1, parser_core_configuration_user_ethtype_config.ethtype_user_prot_2, parser_core_configuration_user_ethtype_config.ethtype_user_prot_3, parser_core_configuration_user_ethtype_config.ethtype_user_en, parser_core_configuration_user_ethtype_config.ethtype_user_offset_0, parser_core_configuration_user_ethtype_config.ethtype_user_offset_1, parser_core_configuration_user_ethtype_config.ethtype_user_offset_2, parser_core_configuration_user_ethtype_config.ethtype_user_offset_3);
        if(err || parser_core_configuration_user_ethtype_config.ethtype_user_prot_0!=gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_prot_1!=gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_prot_2!=gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_prot_3!=gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_en!=gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_0!=gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_1!=gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_2!=gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_3!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_da_filter_valid da_filter_valid = {.da_filt0_valid=gtmv(m, 1), .da_filt1_valid=gtmv(m, 1), .da_filt2_valid=gtmv(m, 1), .da_filt3_valid=gtmv(m, 1), .da_filt4_valid=gtmv(m, 1), .da_filt5_valid=gtmv(m, 1), .da_filt6_valid=gtmv(m, 1), .da_filt7_valid=gtmv(m, 1), .da_filt8_valid=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, da_filter_valid.da_filt8_valid);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(quad_idx, &da_filter_valid);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get( quad_idx, &da_filter_valid);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, da_filter_valid.da_filt8_valid);
        if(err || da_filter_valid.da_filt0_valid!=gtmv(m, 1) || da_filter_valid.da_filt1_valid!=gtmv(m, 1) || da_filter_valid.da_filt2_valid!=gtmv(m, 1) || da_filter_valid.da_filt3_valid!=gtmv(m, 1) || da_filter_valid.da_filt4_valid!=gtmv(m, 1) || da_filter_valid.da_filt5_valid!=gtmv(m, 1) || da_filter_valid.da_filt6_valid!=gtmv(m, 1) || da_filter_valid.da_filt7_valid!=gtmv(m, 1) || da_filter_valid.da_filt8_valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_da_filter_valid da_filter_valid = {.da_filt0_valid=gtmv(m, 1), .da_filt1_valid=gtmv(m, 1), .da_filt2_valid=gtmv(m, 1), .da_filt3_valid=gtmv(m, 1), .da_filt4_valid=gtmv(m, 1), .da_filt5_valid=gtmv(m, 1), .da_filt6_valid=gtmv(m, 1), .da_filt7_valid=gtmv(m, 1), .da_filt8_valid=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, da_filter_valid.da_filt8_valid);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(quad_idx, &da_filter_valid);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get( quad_idx, &da_filter_valid);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx, da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, da_filter_valid.da_filt8_valid);
        if(err || da_filter_valid.da_filt0_valid!=gtmv(m, 1) || da_filter_valid.da_filt1_valid!=gtmv(m, 1) || da_filter_valid.da_filt2_valid!=gtmv(m, 1) || da_filter_valid.da_filt3_valid!=gtmv(m, 1) || da_filter_valid.da_filt4_valid!=gtmv(m, 1) || da_filter_valid.da_filt5_valid!=gtmv(m, 1) || da_filter_valid.da_filt6_valid!=gtmv(m, 1) || da_filter_valid.da_filt7_valid!=gtmv(m, 1) || da_filter_valid.da_filt8_valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t gre_protocol=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(%u %u)\n", quad_idx, gre_protocol);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(quad_idx, gre_protocol);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get( quad_idx, &gre_protocol);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(%u %u)\n", quad_idx, gre_protocol);
        if(err || gre_protocol!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_parser_core_configuration_prop_tag_cfg parser_core_configuration_prop_tag_cfg = {.size_profile_0=gtmv(m, 5), .size_profile_1=gtmv(m, 5), .size_profile_2=gtmv(m, 5), .pre_da_dprofile_0=gtmv(m, 1), .pre_da_dprofile_1=gtmv(m, 1), .pre_da_dprofile_2=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(%u %u %u %u %u %u %u)\n", quad_idx, parser_core_configuration_prop_tag_cfg.size_profile_0, parser_core_configuration_prop_tag_cfg.size_profile_1, parser_core_configuration_prop_tag_cfg.size_profile_2, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(quad_idx, &parser_core_configuration_prop_tag_cfg);
        if(!err) ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get( quad_idx, &parser_core_configuration_prop_tag_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(%u %u %u %u %u %u %u)\n", quad_idx, parser_core_configuration_prop_tag_cfg.size_profile_0, parser_core_configuration_prop_tag_cfg.size_profile_1, parser_core_configuration_prop_tag_cfg.size_profile_2, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2);
        if(err || parser_core_configuration_prop_tag_cfg.size_profile_0!=gtmv(m, 5) || parser_core_configuration_prop_tag_cfg.size_profile_1!=gtmv(m, 5) || parser_core_configuration_prop_tag_cfg.size_profile_2!=gtmv(m, 5) || parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0!=gtmv(m, 1) || parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1!=gtmv(m, 1) || parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean use_fifo_for_ddr_only=gtmv(m, 1);
        bdmf_boolean token_arbiter_is_rr=gtmv(m, 1);
        bdmf_boolean chicken_no_flowctrl=gtmv(m, 1);
        uint8_t congest_threshold=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_dma_arb_cfg_set(%u %u %u %u %u)\n", quad_idx, use_fifo_for_ddr_only, token_arbiter_is_rr, chicken_no_flowctrl, congest_threshold);
        if(!err) ag_drv_rnr_quad_general_config_dma_arb_cfg_set(quad_idx, use_fifo_for_ddr_only, token_arbiter_is_rr, chicken_no_flowctrl, congest_threshold);
        if(!err) ag_drv_rnr_quad_general_config_dma_arb_cfg_get( quad_idx, &use_fifo_for_ddr_only, &token_arbiter_is_rr, &chicken_no_flowctrl, &congest_threshold);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_dma_arb_cfg_get(%u %u %u %u %u)\n", quad_idx, use_fifo_for_ddr_only, token_arbiter_is_rr, chicken_no_flowctrl, congest_threshold);
        if(err || use_fifo_for_ddr_only!=gtmv(m, 1) || token_arbiter_is_rr!=gtmv(m, 1) || chicken_no_flowctrl!=gtmv(m, 1) || congest_threshold!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_base_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram0_base_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram0_base_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_base_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_base_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram1_base_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram1_base_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_base_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_base_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram2_base_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram2_base_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_base_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_base_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram3_base_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram3_base_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_base_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_base_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr0_base_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr0_base_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_base_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_base_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr1_base_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr1_base_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_base_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_mask_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram0_mask_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram0_mask_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_mask_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_mask_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram1_mask_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram1_mask_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_mask_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_mask_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram2_mask_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram2_mask_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_mask_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_mask_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram3_mask_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_psram3_mask_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_mask_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_mask_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr0_mask_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr0_mask_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_mask_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_mask_set(%u %u)\n", quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr1_mask_set(quad_idx, val);
        if(!err) ag_drv_rnr_quad_general_config_ddr1_mask_get( quad_idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_mask_get(%u %u)\n", quad_idx, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_general_config_profiling_config general_config_profiling_config = {.counter_lsb_sel=gtmv(m, 5), .enable_trace_core_0=gtmv(m, 1), .enable_trace_core_1=gtmv(m, 1), .enable_trace_core_2=gtmv(m, 1), .enable_trace_core_3=gtmv(m, 1), .enable_trace_core_4=gtmv(m, 1), .enable_trace_core_5=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_profiling_config_set(%u %u %u %u %u %u %u %u)\n", quad_idx, general_config_profiling_config.counter_lsb_sel, general_config_profiling_config.enable_trace_core_0, general_config_profiling_config.enable_trace_core_1, general_config_profiling_config.enable_trace_core_2, general_config_profiling_config.enable_trace_core_3, general_config_profiling_config.enable_trace_core_4, general_config_profiling_config.enable_trace_core_5);
        if(!err) ag_drv_rnr_quad_general_config_profiling_config_set(quad_idx, &general_config_profiling_config);
        if(!err) ag_drv_rnr_quad_general_config_profiling_config_get( quad_idx, &general_config_profiling_config);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_profiling_config_get(%u %u %u %u %u %u %u %u)\n", quad_idx, general_config_profiling_config.counter_lsb_sel, general_config_profiling_config.enable_trace_core_0, general_config_profiling_config.enable_trace_core_1, general_config_profiling_config.enable_trace_core_2, general_config_profiling_config.enable_trace_core_3, general_config_profiling_config.enable_trace_core_4, general_config_profiling_config.enable_trace_core_5);
        if(err || general_config_profiling_config.counter_lsb_sel!=gtmv(m, 5) || general_config_profiling_config.enable_trace_core_0!=gtmv(m, 1) || general_config_profiling_config.enable_trace_core_1!=gtmv(m, 1) || general_config_profiling_config.enable_trace_core_2!=gtmv(m, 1) || general_config_profiling_config.enable_trace_core_3!=gtmv(m, 1) || general_config_profiling_config.enable_trace_core_4!=gtmv(m, 1) || general_config_profiling_config.enable_trace_core_5!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_0_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_1_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_2_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_3_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_4_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_5_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_6_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t addr=gtmv(m, 13);
        uint8_t thread=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(%u %u %u)\n", quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(quad_idx, addr, thread);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_7_cfg_get( quad_idx, &addr, &thread);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(%u %u %u)\n", quad_idx, addr, thread);
        if(err || addr!=gtmv(m, 13) || thread!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t handler_addr=gtmv(m, 13);
        uint16_t update_pc_value=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(%u %u %u)\n", quad_idx, handler_addr, update_pc_value);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(quad_idx, handler_addr, update_pc_value);
        if(!err) ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get( quad_idx, &handler_addr, &update_pc_value);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(%u %u %u)\n", quad_idx, handler_addr, update_pc_value);
        if(err || handler_addr!=gtmv(m, 13) || update_pc_value!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_general_config_powersave_config general_config_powersave_config = {.time_counter=gtmv(m, 8), .enable_powersave_core_0=gtmv(m, 1), .enable_powersave_core_1=gtmv(m, 1), .enable_powersave_core_2=gtmv(m, 1), .enable_powersave_core_3=gtmv(m, 1), .enable_powersave_core_4=gtmv(m, 1), .enable_powersave_core_5=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_powersave_config_set(%u %u %u %u %u %u %u %u)\n", quad_idx, general_config_powersave_config.time_counter, general_config_powersave_config.enable_powersave_core_0, general_config_powersave_config.enable_powersave_core_1, general_config_powersave_config.enable_powersave_core_2, general_config_powersave_config.enable_powersave_core_3, general_config_powersave_config.enable_powersave_core_4, general_config_powersave_config.enable_powersave_core_5);
        if(!err) ag_drv_rnr_quad_general_config_powersave_config_set(quad_idx, &general_config_powersave_config);
        if(!err) ag_drv_rnr_quad_general_config_powersave_config_get( quad_idx, &general_config_powersave_config);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_powersave_config_get(%u %u %u %u %u %u %u %u)\n", quad_idx, general_config_powersave_config.time_counter, general_config_powersave_config.enable_powersave_core_0, general_config_powersave_config.enable_powersave_core_1, general_config_powersave_config.enable_powersave_core_2, general_config_powersave_config.enable_powersave_core_3, general_config_powersave_config.enable_powersave_core_4, general_config_powersave_config.enable_powersave_core_5);
        if(err || general_config_powersave_config.time_counter!=gtmv(m, 8) || general_config_powersave_config.enable_powersave_core_0!=gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_1!=gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_2!=gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_3!=gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_4!=gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_5!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_general_config_powersave_status general_config_powersave_status = {.core_0_status=gtmv(m, 1), .core_1_status=gtmv(m, 1), .core_2_status=gtmv(m, 1), .core_3_status=gtmv(m, 1), .core_4_status=gtmv(m, 1), .core_5_status=gtmv(m, 1)};
        if(!err) ag_drv_rnr_quad_general_config_powersave_status_get( quad_idx, &general_config_powersave_status);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_general_config_powersave_status_get(%u %u %u %u %u %u %u)\n", quad_idx, general_config_powersave_status.core_0_status, general_config_powersave_status.core_1_status, general_config_powersave_status.core_2_status, general_config_powersave_status.core_3_status, general_config_powersave_status.core_4_status, general_config_powersave_status.core_5_status);
    }
    {
        rnr_quad_debug_fifo_config debug_fifo_config = {.psram_hdr_sw_rst=gtmv(m, 1), .psram_data_sw_rst=gtmv(m, 1), .ddr_hdr_sw_rst=gtmv(m, 1), .psram_hdr_sw_rd_addr=gtmv(m, 4), .psram_data_sw_rd_addr=gtmv(m, 4), .ddr_hdr_sw_rd_addr=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_fifo_config_set(%u %u %u %u %u %u %u)\n", quad_idx, debug_fifo_config.psram_hdr_sw_rst, debug_fifo_config.psram_data_sw_rst, debug_fifo_config.ddr_hdr_sw_rst, debug_fifo_config.psram_hdr_sw_rd_addr, debug_fifo_config.psram_data_sw_rd_addr, debug_fifo_config.ddr_hdr_sw_rd_addr);
        if(!err) ag_drv_rnr_quad_debug_fifo_config_set(quad_idx, &debug_fifo_config);
        if(!err) ag_drv_rnr_quad_debug_fifo_config_get( quad_idx, &debug_fifo_config);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_fifo_config_get(%u %u %u %u %u %u %u)\n", quad_idx, debug_fifo_config.psram_hdr_sw_rst, debug_fifo_config.psram_data_sw_rst, debug_fifo_config.ddr_hdr_sw_rst, debug_fifo_config.psram_hdr_sw_rd_addr, debug_fifo_config.psram_data_sw_rd_addr, debug_fifo_config.ddr_hdr_sw_rd_addr);
        if(err || debug_fifo_config.psram_hdr_sw_rst!=gtmv(m, 1) || debug_fifo_config.psram_data_sw_rst!=gtmv(m, 1) || debug_fifo_config.ddr_hdr_sw_rst!=gtmv(m, 1) || debug_fifo_config.psram_hdr_sw_rd_addr!=gtmv(m, 4) || debug_fifo_config.psram_data_sw_rd_addr!=gtmv(m, 4) || debug_fifo_config.ddr_hdr_sw_rd_addr!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rnr_quad_debug_psram_hdr_fifo_status debug_psram_hdr_fifo_status = {.full=gtmv(m, 1), .empty=gtmv(m, 1), .push_wr_cntr=gtmv(m, 5), .pop_rd_cntr=gtmv(m, 5), .used_words=gtmv(m, 5)};
        if(!err) ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get( quad_idx, &debug_psram_hdr_fifo_status);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(%u %u %u %u %u %u)\n", quad_idx, debug_psram_hdr_fifo_status.full, debug_psram_hdr_fifo_status.empty, debug_psram_hdr_fifo_status.push_wr_cntr, debug_psram_hdr_fifo_status.pop_rd_cntr, debug_psram_hdr_fifo_status.used_words);
    }
    {
        rnr_quad_debug_psram_data_fifo_status debug_psram_data_fifo_status = {.full=gtmv(m, 1), .empty=gtmv(m, 1), .almost_full=gtmv(m, 1), .push_wr_cntr=gtmv(m, 5), .pop_rd_cntr=gtmv(m, 5), .used_words=gtmv(m, 5)};
        if(!err) ag_drv_rnr_quad_debug_psram_data_fifo_status_get( quad_idx, &debug_psram_data_fifo_status);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_data_fifo_status_get(%u %u %u %u %u %u %u)\n", quad_idx, debug_psram_data_fifo_status.full, debug_psram_data_fifo_status.empty, debug_psram_data_fifo_status.almost_full, debug_psram_data_fifo_status.push_wr_cntr, debug_psram_data_fifo_status.pop_rd_cntr, debug_psram_data_fifo_status.used_words);
    }
    {
        rnr_quad_debug_ddr_hdr_fifo_status debug_ddr_hdr_fifo_status = {.full=gtmv(m, 1), .empty=gtmv(m, 1), .push_wr_cntr=gtmv(m, 5), .pop_rd_cntr=gtmv(m, 5), .used_words=gtmv(m, 5)};
        if(!err) ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get( quad_idx, &debug_ddr_hdr_fifo_status);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(%u %u %u %u %u %u)\n", quad_idx, debug_ddr_hdr_fifo_status.full, debug_ddr_hdr_fifo_status.empty, debug_ddr_hdr_fifo_status.push_wr_cntr, debug_ddr_hdr_fifo_status.pop_rd_cntr, debug_ddr_hdr_fifo_status.used_words);
    }
    {
        rnr_quad_debug_ddr_data_fifo_status debug_ddr_data_fifo_status = {.full=gtmv(m, 1), .empty=gtmv(m, 1), .almost_full=gtmv(m, 1), .wr_cntr=gtmv(m, 9), .rd_cntr=gtmv(m, 9)};
        if(!err) ag_drv_rnr_quad_debug_ddr_data_fifo_status_get( quad_idx, &debug_ddr_data_fifo_status);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(%u %u %u %u %u %u)\n", quad_idx, debug_ddr_data_fifo_status.full, debug_ddr_data_fifo_status.empty, debug_ddr_data_fifo_status.almost_full, debug_ddr_data_fifo_status.wr_cntr, debug_ddr_data_fifo_status.rd_cntr);
    }
    {
        uint8_t read_addr=gtmv(m, 8);
        uint16_t used_words=gtmv(m, 9);
        if(!err) ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get( quad_idx, &read_addr, &used_words);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(%u %u %u)\n", quad_idx, read_addr, used_words);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get( quad_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(%u %u)\n", quad_idx, data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get( quad_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(%u %u)\n", quad_idx, data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_rnr_quad_debug_psram_data_fifo_data1_get( quad_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(%u %u)\n", quad_idx, data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_rnr_quad_debug_psram_data_fifo_data2_get( quad_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(%u %u)\n", quad_idx, data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get( quad_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(%u %u)\n", quad_idx, data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get( quad_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(%u %u)\n", quad_idx, data);
    }
    {
        uint32_t index=gtmv(m, 2);
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(%u %u %u)\n", quad_idx, index, val);
        if(!err) ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, index, val);
        if(!err) ag_drv_rnr_quad_ext_flowctrl_config_token_val_get( quad_idx, index, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(%u %u %u)\n", quad_idx, index, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t index=gtmv(m, 4);
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(%u %u %u)\n", quad_idx, index, val);
        if(!err) ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(quad_idx, index, val);
        if(!err) ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get( quad_idx, index, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(%u %u %u)\n", quad_idx, index, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t index=gtmv(m, 4);
        uint32_t val=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(%u %u %u)\n", quad_idx, index, val);
        if(!err) ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad_idx, index, val);
        if(!err) ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get( quad_idx, index, &val);
        if(!err) bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(%u %u %u)\n", quad_idx, index, val);
        if(err || val!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_rnr_quad_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_parser_core_configuration_eng : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_ENG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_parser_misc_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_0_1 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_2_3 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_4_5 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_6_7 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter0_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter1_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter0_mask_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter1_mask_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filters_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_snap_org_code : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ppp_ip_prot_code : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_ethtype : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ethtype_0_1 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ethtype_2_3 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ethtype_config : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_nest : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_hard_nest_0 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_hard_nest_1 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_hard_nest_2 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ip_prot : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt2_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt2_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt3_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt3_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt4_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt4_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt5_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt5_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt6_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt6_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt7_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt7_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt8_val_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt8_val_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_mask_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_mask_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_mask_l : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_mask_h : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt_valid_cfg_0 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt_valid_cfg_1 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt_valid_cfg_2 : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_gre_protocol_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_prop_tag_cfg : reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_dma_arb_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram0_base : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM0_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram1_base : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM1_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram2_base : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM2_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram3_base : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM3_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr0_base : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR0_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr1_base : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR1_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram0_mask : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM0_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram1_mask : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM1_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram2_mask : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM2_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram3_mask : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM3_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr0_mask : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR0_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr1_mask : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR1_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_profiling_config : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_0_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_1_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_2_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_3_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_4_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_5_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_6_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_7_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_gen_cfg : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_powersave_config : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_powersave_status : reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_fifo_config : reg = &RU_REG(RNR_QUAD, DEBUG_FIFO_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_hdr_fifo_status : reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_data_fifo_status : reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_hdr_fifo_status : reg = &RU_REG(RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_data_fifo_status : reg = &RU_REG(RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_data_fifo_status2 : reg = &RU_REG(RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_hdr_fifo_data1 : reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_hdr_fifo_data2 : reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_data_fifo_data1 : reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_data_fifo_data2 : reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_hdr_fifo_data1 : reg = &RU_REG(RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_hdr_fifo_data2 : reg = &RU_REG(RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ext_flowctrl_config_token_val : reg = &RU_REG(RNR_QUAD, EXT_FLOWCTRL_CONFIG_TOKEN_VAL); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ubus_decode_cfg_psram_ubus_decode : reg = &RU_REG(RNR_QUAD, UBUS_DECODE_CFG_PSRAM_UBUS_DECODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ubus_decode_cfg_ddr_ubus_decode : reg = &RU_REG(RNR_QUAD, UBUS_DECODE_CFG_DDR_UBUS_DECODE); blk = &RU_BLK(RNR_QUAD); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%lX\n", j, (blk->addr[i] + reg->addr + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%lX\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_rnr_quad_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "rnr_quad"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "rnr_quad", "rnr_quad", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_parser_vid0[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_0", "vid_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_0_en", "vid_0_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid1[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_1", "vid_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_1_en", "vid_1_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid2[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_2", "vid_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_2_en", "vid_2_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid3[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_3", "vid_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_3_en", "vid_3_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid4[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_4", "vid_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_4_en", "vid_4_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid5[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_5", "vid_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_5_en", "vid_5_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid6[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_6", "vid_6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_6_en", "vid_6_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid7[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("vid_7", "vid_7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vid_7_en", "vid_7_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip0[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("ip_address", "ip_address", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ip_address_mask", "ip_address_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ip_filter0_dip_en", "ip_filter0_dip_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ip_filter0_valid", "ip_filter0_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_hardcoded_ethtype_prof0[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("hard_nest_profile", "hard_nest_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_hardcoded_ethtype_prof1[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("hard_nest_profile", "hard_nest_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_hardcoded_ethtype_prof2[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("hard_nest_profile", "hard_nest_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_prof0[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_0", "qtag_nest_0_profile_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_1_profile_0", "qtag_nest_1_profile_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_prof1[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_1", "qtag_nest_0_profile_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_1_profile_1", "qtag_nest_1_profile_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_prof2[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_2", "qtag_nest_0_profile_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_1_profile_2", "qtag_nest_1_profile_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol0[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_0", "user_ip_prot_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol1[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_1", "user_ip_prot_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol2[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_2", "user_ip_prot_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol3[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_3", "user_ip_prot_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_mask_msb", "da_filt_mask_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_mask_l", "da_filt_mask_l", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter2[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter3[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter4[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter5[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter6[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter7[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter8[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_da_filter_valid[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("da_filt0_valid", "da_filt0_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt1_valid", "da_filt1_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt2_valid", "da_filt2_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt3_valid", "da_filt3_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt4_valid", "da_filt4_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt5_valid", "da_filt5_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt6_valid", "da_filt6_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt7_valid", "da_filt7_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt8_valid", "da_filt8_valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_exception_bits[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("exception_en", "exception_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcp_flags[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("tcp_flags_filt", "tcp_flags_filt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profile_us[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("profile_us", "profile_us", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_snap_conf[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("code", "code", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_rfc1042", "en_rfc1042", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_8021q", "en_8021q", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ipv6_filter[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("hop_by_hop_match", "hop_by_hop_match", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("routing_eh", "routing_eh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dest_opt_eh", "dest_opt_eh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_eng[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("cfg", "cfg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_ppp_ip_prot_code[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("ppp_code_0", "ppp_code_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ppp_code_1", "ppp_code_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_qtag_ethtype[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("ethtype_qtag_0", "ethtype_qtag_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_qtag_1", "ethtype_qtag_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_user_ethtype_0_1[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("ethype_0", "ethype_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethype_1", "ethype_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_user_ethtype_2_3[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("ethype_2", "ethype_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethype_3", "ethype_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_user_ethtype_config[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_0", "ethtype_user_prot_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_1", "ethtype_user_prot_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_2", "ethtype_user_prot_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_3", "ethtype_user_prot_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_en", "ethtype_user_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_0", "ethtype_user_offset_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_1", "ethtype_user_offset_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_2", "ethtype_user_offset_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_3", "ethtype_user_offset_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_gre_protocol_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("gre_protocol", "gre_protocol", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_prop_tag_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("size_profile_0", "size_profile_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("size_profile_1", "size_profile_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("size_profile_2", "size_profile_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pre_da_dprofile_0", "pre_da_dprofile_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pre_da_dprofile_1", "pre_da_dprofile_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pre_da_dprofile_2", "pre_da_dprofile_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_dma_arb_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("use_fifo_for_ddr_only", "use_fifo_for_ddr_only", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token_arbiter_is_rr", "token_arbiter_is_rr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("chicken_no_flowctrl", "chicken_no_flowctrl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("congest_threshold", "congest_threshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram0_base[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram1_base[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram2_base[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram3_base[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr0_base[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr1_base[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram0_mask[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram1_mask[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram2_mask[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram3_mask[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr0_mask[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr1_mask[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_profiling_config[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("counter_lsb_sel", "counter_lsb_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_0", "enable_trace_core_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_1", "enable_trace_core_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_2", "enable_trace_core_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_3", "enable_trace_core_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_4", "enable_trace_core_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_5", "enable_trace_core_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_0_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_1_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_2_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_3_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_4_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_5_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_6_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_7_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_gen_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("handler_addr", "handler_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("update_pc_value", "update_pc_value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_powersave_config[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("time_counter", "time_counter", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_0", "enable_powersave_core_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_1", "enable_powersave_core_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_2", "enable_powersave_core_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_3", "enable_powersave_core_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_4", "enable_powersave_core_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_5", "enable_powersave_core_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_fifo_config[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("psram_hdr_sw_rst", "psram_hdr_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psram_data_sw_rst", "psram_data_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hdr_sw_rst", "ddr_hdr_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psram_hdr_sw_rd_addr", "psram_hdr_sw_rd_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psram_data_sw_rd_addr", "psram_data_sw_rd_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hdr_sw_rd_addr", "ddr_hdr_sw_rd_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ext_flowctrl_config_token_val[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg_psram_ubus_decode[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg_ddr_ubus_decode[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="parser_vid0", .val=cli_rnr_quad_parser_vid0, .parms=set_parser_vid0 },
            { .name="parser_vid1", .val=cli_rnr_quad_parser_vid1, .parms=set_parser_vid1 },
            { .name="parser_vid2", .val=cli_rnr_quad_parser_vid2, .parms=set_parser_vid2 },
            { .name="parser_vid3", .val=cli_rnr_quad_parser_vid3, .parms=set_parser_vid3 },
            { .name="parser_vid4", .val=cli_rnr_quad_parser_vid4, .parms=set_parser_vid4 },
            { .name="parser_vid5", .val=cli_rnr_quad_parser_vid5, .parms=set_parser_vid5 },
            { .name="parser_vid6", .val=cli_rnr_quad_parser_vid6, .parms=set_parser_vid6 },
            { .name="parser_vid7", .val=cli_rnr_quad_parser_vid7, .parms=set_parser_vid7 },
            { .name="parser_ip0", .val=cli_rnr_quad_parser_ip0, .parms=set_parser_ip0 },
            { .name="parser_ip1", .val=cli_rnr_quad_parser_ip1, .parms=set_parser_ip0 },
            { .name="parser_hardcoded_ethtype_prof0", .val=cli_rnr_quad_parser_hardcoded_ethtype_prof0, .parms=set_parser_hardcoded_ethtype_prof0 },
            { .name="parser_hardcoded_ethtype_prof1", .val=cli_rnr_quad_parser_hardcoded_ethtype_prof1, .parms=set_parser_hardcoded_ethtype_prof1 },
            { .name="parser_hardcoded_ethtype_prof2", .val=cli_rnr_quad_parser_hardcoded_ethtype_prof2, .parms=set_parser_hardcoded_ethtype_prof2 },
            { .name="parser_qtag_nest_prof0", .val=cli_rnr_quad_parser_qtag_nest_prof0, .parms=set_parser_qtag_nest_prof0 },
            { .name="parser_qtag_nest_prof1", .val=cli_rnr_quad_parser_qtag_nest_prof1, .parms=set_parser_qtag_nest_prof1 },
            { .name="parser_qtag_nest_prof2", .val=cli_rnr_quad_parser_qtag_nest_prof2, .parms=set_parser_qtag_nest_prof2 },
            { .name="parser_ip_protocol0", .val=cli_rnr_quad_parser_ip_protocol0, .parms=set_parser_ip_protocol0 },
            { .name="parser_ip_protocol1", .val=cli_rnr_quad_parser_ip_protocol1, .parms=set_parser_ip_protocol1 },
            { .name="parser_ip_protocol2", .val=cli_rnr_quad_parser_ip_protocol2, .parms=set_parser_ip_protocol2 },
            { .name="parser_ip_protocol3", .val=cli_rnr_quad_parser_ip_protocol3, .parms=set_parser_ip_protocol3 },
            { .name="parser_da_filter", .val=cli_rnr_quad_parser_da_filter, .parms=set_parser_da_filter },
            { .name="parser_da_filter1", .val=cli_rnr_quad_parser_da_filter1, .parms=set_parser_da_filter },
            { .name="parser_da_filter2", .val=cli_rnr_quad_parser_da_filter2, .parms=set_parser_da_filter2 },
            { .name="parser_da_filter3", .val=cli_rnr_quad_parser_da_filter3, .parms=set_parser_da_filter3 },
            { .name="parser_da_filter4", .val=cli_rnr_quad_parser_da_filter4, .parms=set_parser_da_filter4 },
            { .name="parser_da_filter5", .val=cli_rnr_quad_parser_da_filter5, .parms=set_parser_da_filter5 },
            { .name="parser_da_filter6", .val=cli_rnr_quad_parser_da_filter6, .parms=set_parser_da_filter6 },
            { .name="parser_da_filter7", .val=cli_rnr_quad_parser_da_filter7, .parms=set_parser_da_filter7 },
            { .name="parser_da_filter8", .val=cli_rnr_quad_parser_da_filter8, .parms=set_parser_da_filter8 },
            { .name="da_filter_valid", .val=cli_rnr_quad_da_filter_valid, .parms=set_da_filter_valid },
            { .name="exception_bits", .val=cli_rnr_quad_exception_bits, .parms=set_exception_bits },
            { .name="tcp_flags", .val=cli_rnr_quad_tcp_flags, .parms=set_tcp_flags },
            { .name="profile_us", .val=cli_rnr_quad_profile_us, .parms=set_profile_us },
            { .name="parser_snap_conf", .val=cli_rnr_quad_parser_snap_conf, .parms=set_parser_snap_conf },
            { .name="parser_ipv6_filter", .val=cli_rnr_quad_parser_ipv6_filter, .parms=set_parser_ipv6_filter },
            { .name="parser_core_configuration_eng", .val=cli_rnr_quad_parser_core_configuration_eng, .parms=set_parser_core_configuration_eng },
            { .name="parser_core_configuration_ppp_ip_prot_code", .val=cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code, .parms=set_parser_core_configuration_ppp_ip_prot_code },
            { .name="parser_core_configuration_qtag_ethtype", .val=cli_rnr_quad_parser_core_configuration_qtag_ethtype, .parms=set_parser_core_configuration_qtag_ethtype },
            { .name="parser_core_configuration_user_ethtype_0_1", .val=cli_rnr_quad_parser_core_configuration_user_ethtype_0_1, .parms=set_parser_core_configuration_user_ethtype_0_1 },
            { .name="parser_core_configuration_user_ethtype_2_3", .val=cli_rnr_quad_parser_core_configuration_user_ethtype_2_3, .parms=set_parser_core_configuration_user_ethtype_2_3 },
            { .name="parser_core_configuration_user_ethtype_config", .val=cli_rnr_quad_parser_core_configuration_user_ethtype_config, .parms=set_parser_core_configuration_user_ethtype_config },
            { .name="parser_core_configuration_da_filt_valid_cfg_1", .val=cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1, .parms=set_da_filter_valid },
            { .name="parser_core_configuration_da_filt_valid_cfg_2", .val=cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2, .parms=set_da_filter_valid },
            { .name="parser_core_configuration_gre_protocol_cfg", .val=cli_rnr_quad_parser_core_configuration_gre_protocol_cfg, .parms=set_parser_core_configuration_gre_protocol_cfg },
            { .name="parser_core_configuration_prop_tag_cfg", .val=cli_rnr_quad_parser_core_configuration_prop_tag_cfg, .parms=set_parser_core_configuration_prop_tag_cfg },
            { .name="general_config_dma_arb_cfg", .val=cli_rnr_quad_general_config_dma_arb_cfg, .parms=set_general_config_dma_arb_cfg },
            { .name="general_config_psram0_base", .val=cli_rnr_quad_general_config_psram0_base, .parms=set_general_config_psram0_base },
            { .name="general_config_psram1_base", .val=cli_rnr_quad_general_config_psram1_base, .parms=set_general_config_psram1_base },
            { .name="general_config_psram2_base", .val=cli_rnr_quad_general_config_psram2_base, .parms=set_general_config_psram2_base },
            { .name="general_config_psram3_base", .val=cli_rnr_quad_general_config_psram3_base, .parms=set_general_config_psram3_base },
            { .name="general_config_ddr0_base", .val=cli_rnr_quad_general_config_ddr0_base, .parms=set_general_config_ddr0_base },
            { .name="general_config_ddr1_base", .val=cli_rnr_quad_general_config_ddr1_base, .parms=set_general_config_ddr1_base },
            { .name="general_config_psram0_mask", .val=cli_rnr_quad_general_config_psram0_mask, .parms=set_general_config_psram0_mask },
            { .name="general_config_psram1_mask", .val=cli_rnr_quad_general_config_psram1_mask, .parms=set_general_config_psram1_mask },
            { .name="general_config_psram2_mask", .val=cli_rnr_quad_general_config_psram2_mask, .parms=set_general_config_psram2_mask },
            { .name="general_config_psram3_mask", .val=cli_rnr_quad_general_config_psram3_mask, .parms=set_general_config_psram3_mask },
            { .name="general_config_ddr0_mask", .val=cli_rnr_quad_general_config_ddr0_mask, .parms=set_general_config_ddr0_mask },
            { .name="general_config_ddr1_mask", .val=cli_rnr_quad_general_config_ddr1_mask, .parms=set_general_config_ddr1_mask },
            { .name="general_config_profiling_config", .val=cli_rnr_quad_general_config_profiling_config, .parms=set_general_config_profiling_config },
            { .name="general_config_bkpt_0_cfg", .val=cli_rnr_quad_general_config_bkpt_0_cfg, .parms=set_general_config_bkpt_0_cfg },
            { .name="general_config_bkpt_1_cfg", .val=cli_rnr_quad_general_config_bkpt_1_cfg, .parms=set_general_config_bkpt_1_cfg },
            { .name="general_config_bkpt_2_cfg", .val=cli_rnr_quad_general_config_bkpt_2_cfg, .parms=set_general_config_bkpt_2_cfg },
            { .name="general_config_bkpt_3_cfg", .val=cli_rnr_quad_general_config_bkpt_3_cfg, .parms=set_general_config_bkpt_3_cfg },
            { .name="general_config_bkpt_4_cfg", .val=cli_rnr_quad_general_config_bkpt_4_cfg, .parms=set_general_config_bkpt_4_cfg },
            { .name="general_config_bkpt_5_cfg", .val=cli_rnr_quad_general_config_bkpt_5_cfg, .parms=set_general_config_bkpt_5_cfg },
            { .name="general_config_bkpt_6_cfg", .val=cli_rnr_quad_general_config_bkpt_6_cfg, .parms=set_general_config_bkpt_6_cfg },
            { .name="general_config_bkpt_7_cfg", .val=cli_rnr_quad_general_config_bkpt_7_cfg, .parms=set_general_config_bkpt_7_cfg },
            { .name="general_config_bkpt_gen_cfg", .val=cli_rnr_quad_general_config_bkpt_gen_cfg, .parms=set_general_config_bkpt_gen_cfg },
            { .name="general_config_powersave_config", .val=cli_rnr_quad_general_config_powersave_config, .parms=set_general_config_powersave_config },
            { .name="debug_fifo_config", .val=cli_rnr_quad_debug_fifo_config, .parms=set_debug_fifo_config },
            { .name="ext_flowctrl_config_token_val", .val=cli_rnr_quad_ext_flowctrl_config_token_val, .parms=set_ext_flowctrl_config_token_val },
            { .name="ubus_decode_cfg_psram_ubus_decode", .val=cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode, .parms=set_ubus_decode_cfg_psram_ubus_decode },
            { .name="ubus_decode_cfg_ddr_ubus_decode", .val=cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode, .parms=set_ubus_decode_cfg_ddr_ubus_decode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_rnr_quad_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ext_flowctrl_config_token_val[]={
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg_psram_ubus_decode[]={
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg_ddr_ubus_decode[]={
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="parser_vid0", .val=cli_rnr_quad_parser_vid0, .parms=set_default },
            { .name="parser_vid1", .val=cli_rnr_quad_parser_vid1, .parms=set_default },
            { .name="parser_vid2", .val=cli_rnr_quad_parser_vid2, .parms=set_default },
            { .name="parser_vid3", .val=cli_rnr_quad_parser_vid3, .parms=set_default },
            { .name="parser_vid4", .val=cli_rnr_quad_parser_vid4, .parms=set_default },
            { .name="parser_vid5", .val=cli_rnr_quad_parser_vid5, .parms=set_default },
            { .name="parser_vid6", .val=cli_rnr_quad_parser_vid6, .parms=set_default },
            { .name="parser_vid7", .val=cli_rnr_quad_parser_vid7, .parms=set_default },
            { .name="parser_ip0", .val=cli_rnr_quad_parser_ip0, .parms=set_default },
            { .name="parser_ip1", .val=cli_rnr_quad_parser_ip1, .parms=set_default },
            { .name="parser_hardcoded_ethtype_prof0", .val=cli_rnr_quad_parser_hardcoded_ethtype_prof0, .parms=set_default },
            { .name="parser_hardcoded_ethtype_prof1", .val=cli_rnr_quad_parser_hardcoded_ethtype_prof1, .parms=set_default },
            { .name="parser_hardcoded_ethtype_prof2", .val=cli_rnr_quad_parser_hardcoded_ethtype_prof2, .parms=set_default },
            { .name="parser_qtag_nest_prof0", .val=cli_rnr_quad_parser_qtag_nest_prof0, .parms=set_default },
            { .name="parser_qtag_nest_prof1", .val=cli_rnr_quad_parser_qtag_nest_prof1, .parms=set_default },
            { .name="parser_qtag_nest_prof2", .val=cli_rnr_quad_parser_qtag_nest_prof2, .parms=set_default },
            { .name="parser_ip_protocol0", .val=cli_rnr_quad_parser_ip_protocol0, .parms=set_default },
            { .name="parser_ip_protocol1", .val=cli_rnr_quad_parser_ip_protocol1, .parms=set_default },
            { .name="parser_ip_protocol2", .val=cli_rnr_quad_parser_ip_protocol2, .parms=set_default },
            { .name="parser_ip_protocol3", .val=cli_rnr_quad_parser_ip_protocol3, .parms=set_default },
            { .name="parser_da_filter", .val=cli_rnr_quad_parser_da_filter, .parms=set_default },
            { .name="parser_da_filter1", .val=cli_rnr_quad_parser_da_filter1, .parms=set_default },
            { .name="parser_da_filter2", .val=cli_rnr_quad_parser_da_filter2, .parms=set_default },
            { .name="parser_da_filter3", .val=cli_rnr_quad_parser_da_filter3, .parms=set_default },
            { .name="parser_da_filter4", .val=cli_rnr_quad_parser_da_filter4, .parms=set_default },
            { .name="parser_da_filter5", .val=cli_rnr_quad_parser_da_filter5, .parms=set_default },
            { .name="parser_da_filter6", .val=cli_rnr_quad_parser_da_filter6, .parms=set_default },
            { .name="parser_da_filter7", .val=cli_rnr_quad_parser_da_filter7, .parms=set_default },
            { .name="parser_da_filter8", .val=cli_rnr_quad_parser_da_filter8, .parms=set_default },
            { .name="da_filter_valid", .val=cli_rnr_quad_da_filter_valid, .parms=set_default },
            { .name="exception_bits", .val=cli_rnr_quad_exception_bits, .parms=set_default },
            { .name="tcp_flags", .val=cli_rnr_quad_tcp_flags, .parms=set_default },
            { .name="profile_us", .val=cli_rnr_quad_profile_us, .parms=set_default },
            { .name="parser_snap_conf", .val=cli_rnr_quad_parser_snap_conf, .parms=set_default },
            { .name="parser_ipv6_filter", .val=cli_rnr_quad_parser_ipv6_filter, .parms=set_default },
            { .name="parser_core_configuration_eng", .val=cli_rnr_quad_parser_core_configuration_eng, .parms=set_default },
            { .name="parser_core_configuration_ppp_ip_prot_code", .val=cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code, .parms=set_default },
            { .name="parser_core_configuration_qtag_ethtype", .val=cli_rnr_quad_parser_core_configuration_qtag_ethtype, .parms=set_default },
            { .name="parser_core_configuration_user_ethtype_0_1", .val=cli_rnr_quad_parser_core_configuration_user_ethtype_0_1, .parms=set_default },
            { .name="parser_core_configuration_user_ethtype_2_3", .val=cli_rnr_quad_parser_core_configuration_user_ethtype_2_3, .parms=set_default },
            { .name="parser_core_configuration_user_ethtype_config", .val=cli_rnr_quad_parser_core_configuration_user_ethtype_config, .parms=set_default },
            { .name="parser_core_configuration_da_filt_valid_cfg_1", .val=cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1, .parms=set_default },
            { .name="parser_core_configuration_da_filt_valid_cfg_2", .val=cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2, .parms=set_default },
            { .name="parser_core_configuration_gre_protocol_cfg", .val=cli_rnr_quad_parser_core_configuration_gre_protocol_cfg, .parms=set_default },
            { .name="parser_core_configuration_prop_tag_cfg", .val=cli_rnr_quad_parser_core_configuration_prop_tag_cfg, .parms=set_default },
            { .name="general_config_dma_arb_cfg", .val=cli_rnr_quad_general_config_dma_arb_cfg, .parms=set_default },
            { .name="general_config_psram0_base", .val=cli_rnr_quad_general_config_psram0_base, .parms=set_default },
            { .name="general_config_psram1_base", .val=cli_rnr_quad_general_config_psram1_base, .parms=set_default },
            { .name="general_config_psram2_base", .val=cli_rnr_quad_general_config_psram2_base, .parms=set_default },
            { .name="general_config_psram3_base", .val=cli_rnr_quad_general_config_psram3_base, .parms=set_default },
            { .name="general_config_ddr0_base", .val=cli_rnr_quad_general_config_ddr0_base, .parms=set_default },
            { .name="general_config_ddr1_base", .val=cli_rnr_quad_general_config_ddr1_base, .parms=set_default },
            { .name="general_config_psram0_mask", .val=cli_rnr_quad_general_config_psram0_mask, .parms=set_default },
            { .name="general_config_psram1_mask", .val=cli_rnr_quad_general_config_psram1_mask, .parms=set_default },
            { .name="general_config_psram2_mask", .val=cli_rnr_quad_general_config_psram2_mask, .parms=set_default },
            { .name="general_config_psram3_mask", .val=cli_rnr_quad_general_config_psram3_mask, .parms=set_default },
            { .name="general_config_ddr0_mask", .val=cli_rnr_quad_general_config_ddr0_mask, .parms=set_default },
            { .name="general_config_ddr1_mask", .val=cli_rnr_quad_general_config_ddr1_mask, .parms=set_default },
            { .name="general_config_profiling_config", .val=cli_rnr_quad_general_config_profiling_config, .parms=set_default },
            { .name="general_config_bkpt_0_cfg", .val=cli_rnr_quad_general_config_bkpt_0_cfg, .parms=set_default },
            { .name="general_config_bkpt_1_cfg", .val=cli_rnr_quad_general_config_bkpt_1_cfg, .parms=set_default },
            { .name="general_config_bkpt_2_cfg", .val=cli_rnr_quad_general_config_bkpt_2_cfg, .parms=set_default },
            { .name="general_config_bkpt_3_cfg", .val=cli_rnr_quad_general_config_bkpt_3_cfg, .parms=set_default },
            { .name="general_config_bkpt_4_cfg", .val=cli_rnr_quad_general_config_bkpt_4_cfg, .parms=set_default },
            { .name="general_config_bkpt_5_cfg", .val=cli_rnr_quad_general_config_bkpt_5_cfg, .parms=set_default },
            { .name="general_config_bkpt_6_cfg", .val=cli_rnr_quad_general_config_bkpt_6_cfg, .parms=set_default },
            { .name="general_config_bkpt_7_cfg", .val=cli_rnr_quad_general_config_bkpt_7_cfg, .parms=set_default },
            { .name="general_config_bkpt_gen_cfg", .val=cli_rnr_quad_general_config_bkpt_gen_cfg, .parms=set_default },
            { .name="general_config_powersave_config", .val=cli_rnr_quad_general_config_powersave_config, .parms=set_default },
            { .name="general_config_powersave_status", .val=cli_rnr_quad_general_config_powersave_status, .parms=set_default },
            { .name="debug_fifo_config", .val=cli_rnr_quad_debug_fifo_config, .parms=set_default },
            { .name="debug_psram_hdr_fifo_status", .val=cli_rnr_quad_debug_psram_hdr_fifo_status, .parms=set_default },
            { .name="debug_psram_data_fifo_status", .val=cli_rnr_quad_debug_psram_data_fifo_status, .parms=set_default },
            { .name="debug_ddr_hdr_fifo_status", .val=cli_rnr_quad_debug_ddr_hdr_fifo_status, .parms=set_default },
            { .name="debug_ddr_data_fifo_status", .val=cli_rnr_quad_debug_ddr_data_fifo_status, .parms=set_default },
            { .name="debug_ddr_data_fifo_status2", .val=cli_rnr_quad_debug_ddr_data_fifo_status2, .parms=set_default },
            { .name="debug_psram_hdr_fifo_data1", .val=cli_rnr_quad_debug_psram_hdr_fifo_data1, .parms=set_default },
            { .name="debug_psram_hdr_fifo_data2", .val=cli_rnr_quad_debug_psram_hdr_fifo_data2, .parms=set_default },
            { .name="debug_psram_data_fifo_data1", .val=cli_rnr_quad_debug_psram_data_fifo_data1, .parms=set_default },
            { .name="debug_psram_data_fifo_data2", .val=cli_rnr_quad_debug_psram_data_fifo_data2, .parms=set_default },
            { .name="debug_ddr_hdr_fifo_data1", .val=cli_rnr_quad_debug_ddr_hdr_fifo_data1, .parms=set_default },
            { .name="debug_ddr_hdr_fifo_data2", .val=cli_rnr_quad_debug_ddr_hdr_fifo_data2, .parms=set_default },
            { .name="ext_flowctrl_config_token_val", .val=cli_rnr_quad_ext_flowctrl_config_token_val, .parms=set_ext_flowctrl_config_token_val },
            { .name="ubus_decode_cfg_psram_ubus_decode", .val=cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode, .parms=set_ubus_decode_cfg_psram_ubus_decode },
            { .name="ubus_decode_cfg_ddr_ubus_decode", .val=cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode, .parms=set_ubus_decode_cfg_ddr_ubus_decode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_rnr_quad_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_rnr_quad_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("quad_idx", "quad_idx", quad_idx_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="PARSER_CORE_CONFIGURATION_ENG" , .val=bdmf_address_parser_core_configuration_eng },
            { .name="PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG" , .val=bdmf_address_parser_core_configuration_parser_misc_cfg },
            { .name="PARSER_CORE_CONFIGURATION_VID_0_1" , .val=bdmf_address_parser_core_configuration_vid_0_1 },
            { .name="PARSER_CORE_CONFIGURATION_VID_2_3" , .val=bdmf_address_parser_core_configuration_vid_2_3 },
            { .name="PARSER_CORE_CONFIGURATION_VID_4_5" , .val=bdmf_address_parser_core_configuration_vid_4_5 },
            { .name="PARSER_CORE_CONFIGURATION_VID_6_7" , .val=bdmf_address_parser_core_configuration_vid_6_7 },
            { .name="PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG" , .val=bdmf_address_parser_core_configuration_ip_filter0_cfg },
            { .name="PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG" , .val=bdmf_address_parser_core_configuration_ip_filter1_cfg },
            { .name="PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG" , .val=bdmf_address_parser_core_configuration_ip_filter0_mask_cfg },
            { .name="PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG" , .val=bdmf_address_parser_core_configuration_ip_filter1_mask_cfg },
            { .name="PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG" , .val=bdmf_address_parser_core_configuration_ip_filters_cfg },
            { .name="PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE" , .val=bdmf_address_parser_core_configuration_snap_org_code },
            { .name="PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE" , .val=bdmf_address_parser_core_configuration_ppp_ip_prot_code },
            { .name="PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE" , .val=bdmf_address_parser_core_configuration_qtag_ethtype },
            { .name="PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1" , .val=bdmf_address_parser_core_configuration_user_ethtype_0_1 },
            { .name="PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3" , .val=bdmf_address_parser_core_configuration_user_ethtype_2_3 },
            { .name="PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG" , .val=bdmf_address_parser_core_configuration_user_ethtype_config },
            { .name="PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG" , .val=bdmf_address_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg },
            { .name="PARSER_CORE_CONFIGURATION_QTAG_NEST" , .val=bdmf_address_parser_core_configuration_qtag_nest },
            { .name="PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0" , .val=bdmf_address_parser_core_configuration_qtag_hard_nest_0 },
            { .name="PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1" , .val=bdmf_address_parser_core_configuration_qtag_hard_nest_1 },
            { .name="PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2" , .val=bdmf_address_parser_core_configuration_qtag_hard_nest_2 },
            { .name="PARSER_CORE_CONFIGURATION_USER_IP_PROT" , .val=bdmf_address_parser_core_configuration_user_ip_prot },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt0_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt0_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt1_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt1_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt2_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt2_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt3_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt3_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt4_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt4_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt5_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt5_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt6_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt6_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt7_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt7_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L" , .val=bdmf_address_parser_core_configuration_da_filt8_val_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H" , .val=bdmf_address_parser_core_configuration_da_filt8_val_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L" , .val=bdmf_address_parser_core_configuration_da_filt0_mask_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H" , .val=bdmf_address_parser_core_configuration_da_filt0_mask_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L" , .val=bdmf_address_parser_core_configuration_da_filt1_mask_l },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H" , .val=bdmf_address_parser_core_configuration_da_filt1_mask_h },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0" , .val=bdmf_address_parser_core_configuration_da_filt_valid_cfg_0 },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1" , .val=bdmf_address_parser_core_configuration_da_filt_valid_cfg_1 },
            { .name="PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2" , .val=bdmf_address_parser_core_configuration_da_filt_valid_cfg_2 },
            { .name="PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG" , .val=bdmf_address_parser_core_configuration_gre_protocol_cfg },
            { .name="PARSER_CORE_CONFIGURATION_PROP_TAG_CFG" , .val=bdmf_address_parser_core_configuration_prop_tag_cfg },
            { .name="GENERAL_CONFIG_DMA_ARB_CFG" , .val=bdmf_address_general_config_dma_arb_cfg },
            { .name="GENERAL_CONFIG_PSRAM0_BASE" , .val=bdmf_address_general_config_psram0_base },
            { .name="GENERAL_CONFIG_PSRAM1_BASE" , .val=bdmf_address_general_config_psram1_base },
            { .name="GENERAL_CONFIG_PSRAM2_BASE" , .val=bdmf_address_general_config_psram2_base },
            { .name="GENERAL_CONFIG_PSRAM3_BASE" , .val=bdmf_address_general_config_psram3_base },
            { .name="GENERAL_CONFIG_DDR0_BASE" , .val=bdmf_address_general_config_ddr0_base },
            { .name="GENERAL_CONFIG_DDR1_BASE" , .val=bdmf_address_general_config_ddr1_base },
            { .name="GENERAL_CONFIG_PSRAM0_MASK" , .val=bdmf_address_general_config_psram0_mask },
            { .name="GENERAL_CONFIG_PSRAM1_MASK" , .val=bdmf_address_general_config_psram1_mask },
            { .name="GENERAL_CONFIG_PSRAM2_MASK" , .val=bdmf_address_general_config_psram2_mask },
            { .name="GENERAL_CONFIG_PSRAM3_MASK" , .val=bdmf_address_general_config_psram3_mask },
            { .name="GENERAL_CONFIG_DDR0_MASK" , .val=bdmf_address_general_config_ddr0_mask },
            { .name="GENERAL_CONFIG_DDR1_MASK" , .val=bdmf_address_general_config_ddr1_mask },
            { .name="GENERAL_CONFIG_PROFILING_CONFIG" , .val=bdmf_address_general_config_profiling_config },
            { .name="GENERAL_CONFIG_BKPT_0_CFG" , .val=bdmf_address_general_config_bkpt_0_cfg },
            { .name="GENERAL_CONFIG_BKPT_1_CFG" , .val=bdmf_address_general_config_bkpt_1_cfg },
            { .name="GENERAL_CONFIG_BKPT_2_CFG" , .val=bdmf_address_general_config_bkpt_2_cfg },
            { .name="GENERAL_CONFIG_BKPT_3_CFG" , .val=bdmf_address_general_config_bkpt_3_cfg },
            { .name="GENERAL_CONFIG_BKPT_4_CFG" , .val=bdmf_address_general_config_bkpt_4_cfg },
            { .name="GENERAL_CONFIG_BKPT_5_CFG" , .val=bdmf_address_general_config_bkpt_5_cfg },
            { .name="GENERAL_CONFIG_BKPT_6_CFG" , .val=bdmf_address_general_config_bkpt_6_cfg },
            { .name="GENERAL_CONFIG_BKPT_7_CFG" , .val=bdmf_address_general_config_bkpt_7_cfg },
            { .name="GENERAL_CONFIG_BKPT_GEN_CFG" , .val=bdmf_address_general_config_bkpt_gen_cfg },
            { .name="GENERAL_CONFIG_POWERSAVE_CONFIG" , .val=bdmf_address_general_config_powersave_config },
            { .name="GENERAL_CONFIG_POWERSAVE_STATUS" , .val=bdmf_address_general_config_powersave_status },
            { .name="DEBUG_FIFO_CONFIG" , .val=bdmf_address_debug_fifo_config },
            { .name="DEBUG_PSRAM_HDR_FIFO_STATUS" , .val=bdmf_address_debug_psram_hdr_fifo_status },
            { .name="DEBUG_PSRAM_DATA_FIFO_STATUS" , .val=bdmf_address_debug_psram_data_fifo_status },
            { .name="DEBUG_DDR_HDR_FIFO_STATUS" , .val=bdmf_address_debug_ddr_hdr_fifo_status },
            { .name="DEBUG_DDR_DATA_FIFO_STATUS" , .val=bdmf_address_debug_ddr_data_fifo_status },
            { .name="DEBUG_DDR_DATA_FIFO_STATUS2" , .val=bdmf_address_debug_ddr_data_fifo_status2 },
            { .name="DEBUG_PSRAM_HDR_FIFO_DATA1" , .val=bdmf_address_debug_psram_hdr_fifo_data1 },
            { .name="DEBUG_PSRAM_HDR_FIFO_DATA2" , .val=bdmf_address_debug_psram_hdr_fifo_data2 },
            { .name="DEBUG_PSRAM_DATA_FIFO_DATA1" , .val=bdmf_address_debug_psram_data_fifo_data1 },
            { .name="DEBUG_PSRAM_DATA_FIFO_DATA2" , .val=bdmf_address_debug_psram_data_fifo_data2 },
            { .name="DEBUG_DDR_HDR_FIFO_DATA1" , .val=bdmf_address_debug_ddr_hdr_fifo_data1 },
            { .name="DEBUG_DDR_HDR_FIFO_DATA2" , .val=bdmf_address_debug_ddr_hdr_fifo_data2 },
            { .name="EXT_FLOWCTRL_CONFIG_TOKEN_VAL" , .val=bdmf_address_ext_flowctrl_config_token_val },
            { .name="UBUS_DECODE_CFG_PSRAM_UBUS_DECODE" , .val=bdmf_address_ubus_decode_cfg_psram_ubus_decode },
            { .name="UBUS_DECODE_CFG_DDR_UBUS_DECODE" , .val=bdmf_address_ubus_decode_cfg_ddr_ubus_decode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_rnr_quad_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "quad_idx", quad_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

