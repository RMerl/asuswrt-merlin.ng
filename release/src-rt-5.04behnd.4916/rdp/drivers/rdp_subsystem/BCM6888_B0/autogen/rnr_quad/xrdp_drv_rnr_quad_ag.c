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


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_rnr_quad_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_rnr_quad_parser_vid0_set(uint8_t quad_idx, uint16_t vid_0, bdmf_boolean vid_0_en)
{
    uint32_t reg_parser_core_configuration_vid_0_1 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_0 || !vid_0_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_0_1 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_1 || !vid_1_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_2_3 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_2 || !vid_2_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_2_3 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_3 || !vid_3_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_4_5 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_4 || !vid_4_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_4_5 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_5 || !vid_5_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_6_7 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_6 || !vid_6_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_vid_6_7 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!vid_7 || !vid_7_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_ip_filter0_cfg = 0;
    uint32_t reg_parser_core_configuration_ip_filter0_mask_cfg = 0;
    uint32_t reg_parser_core_configuration_ip_filters_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_ip_filter0_cfg = 0;
    uint32_t reg_parser_core_configuration_ip_filter0_mask_cfg = 0;
    uint32_t reg_parser_core_configuration_ip_filters_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!parser_ip0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(uint8_t quad_idx, const rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0 = 0;

#ifdef VALIDATE_PARMS
    if(!parser_hardcoded_ethtype_prof0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_hardcoded_ethtype_prof0->hard_nest_profile >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_hard_nest_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0, parser_hardcoded_ethtype_prof0->hard_nest_profile);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(uint8_t quad_idx, rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0)
{
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0;

#ifdef VALIDATE_PARMS
    if (!parser_hardcoded_ethtype_prof0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, reg_parser_core_configuration_qtag_hard_nest_0);

    parser_hardcoded_ethtype_prof0->hard_nest_profile = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(uint8_t quad_idx, const rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0)
{
    /* Identical to parser_hardcoded_ethtype_prof0 */
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0 = 0;

#ifdef VALIDATE_PARMS
    if(!parser_hardcoded_ethtype_prof0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_hardcoded_ethtype_prof0->hard_nest_profile >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_hard_nest_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0, parser_hardcoded_ethtype_prof0->hard_nest_profile);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(uint8_t quad_idx, rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0)
{
    /* Identical to parser_hardcoded_ethtype_prof0 */
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0;

#ifdef VALIDATE_PARMS
    if (!parser_hardcoded_ethtype_prof0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1, reg_parser_core_configuration_qtag_hard_nest_0);

    parser_hardcoded_ethtype_prof0->hard_nest_profile = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(uint8_t quad_idx, const rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0)
{
    /* Identical to parser_hardcoded_ethtype_prof0 */
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0 = 0;

#ifdef VALIDATE_PARMS
    if(!parser_hardcoded_ethtype_prof0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_hardcoded_ethtype_prof0->hard_nest_profile >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_qtag_hard_nest_0 = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0, parser_hardcoded_ethtype_prof0->hard_nest_profile);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(uint8_t quad_idx, rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0)
{
    /* Identical to parser_hardcoded_ethtype_prof0 */
    uint32_t reg_parser_core_configuration_qtag_hard_nest_0;

#ifdef VALIDATE_PARMS
    if (!parser_hardcoded_ethtype_prof0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2, reg_parser_core_configuration_qtag_hard_nest_0);

    parser_hardcoded_ethtype_prof0->hard_nest_profile = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0, HARD_NEST_PROFILE, reg_parser_core_configuration_qtag_hard_nest_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_0, uint8_t qtag_nest_1_profile_0)
{
    uint32_t reg_parser_core_configuration_qtag_nest = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!qtag_nest_0_profile_0 || !qtag_nest_1_profile_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_qtag_nest = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!qtag_nest_0_profile_1 || !qtag_nest_1_profile_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_qtag_nest = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!qtag_nest_0_profile_2 || !qtag_nest_1_profile_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_max_vlans_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_2, uint8_t max_num_of_vlans)
{
    uint32_t reg_parser_core_configuration_qtag_nest = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (qtag_nest_0_profile_2 >= _3BITS_MAX_VAL_) ||
       (max_num_of_vlans >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_2, reg_parser_core_configuration_qtag_nest, qtag_nest_0_profile_2);
    reg_parser_core_configuration_qtag_nest = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, MAX_NUM_OF_VLANS, reg_parser_core_configuration_qtag_nest, max_num_of_vlans);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_max_vlans_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_2, uint8_t *max_num_of_vlans)
{
    uint32_t reg_parser_core_configuration_qtag_nest;

#ifdef VALIDATE_PARMS
    if (!qtag_nest_0_profile_2 || !max_num_of_vlans)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, reg_parser_core_configuration_qtag_nest);

    *qtag_nest_0_profile_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, QTAG_NEST_0_PROFILE_2, reg_parser_core_configuration_qtag_nest);
    *max_num_of_vlans = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST, MAX_NUM_OF_VLANS, reg_parser_core_configuration_qtag_nest);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_set(uint8_t quad_idx, uint8_t user_ip_prot_0)
{
    uint32_t reg_parser_core_configuration_user_ip_prot = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!user_ip_prot_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_user_ip_prot = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!user_ip_prot_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_user_ip_prot = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!user_ip_prot_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_user_ip_prot = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!user_ip_prot_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt0_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt0_val_l = 0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_h = 0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_l = 0;

#ifdef VALIDATE_PARMS
    if(!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt0_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt0_val_l = 0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_h = 0;
    uint32_t reg_parser_core_configuration_da_filt0_mask_l = 0;

#ifdef VALIDATE_PARMS
    if(!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!parser_da_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt2_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt2_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt3_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt3_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt4_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt4_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt5_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt5_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt6_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt6_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt7_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt7_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt8_val_h = 0;
    uint32_t reg_parser_core_configuration_da_filt8_val_l = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!da_filt_msb || !da_filt_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0 = 0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_rnr_quad_exception_bits_set(uint8_t quad_idx, uint32_t exception_en)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (exception_en >= _20BITS_MAX_VAL_))
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

bdmf_error_t ag_drv_rnr_quad_exception_bits_get(uint8_t quad_idx, uint32_t *exception_en)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg;

#ifdef VALIDATE_PARMS
    if (!exception_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_parser_misc_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!tcp_flags_filt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_parser_misc_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!profile_us)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    *profile_us = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, PROFILE_US, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_disable_l2tp_source_port_set(uint8_t quad_idx, bdmf_boolean disable_l2tp_source_port_check)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (disable_l2tp_source_port_check >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    reg_parser_core_configuration_parser_misc_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, DISABLE_L2TP_SOURCE_PORT_CHECK, reg_parser_core_configuration_parser_misc_cfg, disable_l2tp_source_port_check);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_disable_l2tp_source_port_get(uint8_t quad_idx, bdmf_boolean *disable_l2tp_source_port_check)
{
    uint32_t reg_parser_core_configuration_parser_misc_cfg;

#ifdef VALIDATE_PARMS
    if (!disable_l2tp_source_port_check)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, reg_parser_core_configuration_parser_misc_cfg);

    *disable_l2tp_source_port_check = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG, DISABLE_L2TP_SOURCE_PORT_CHECK, reg_parser_core_configuration_parser_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_set(uint8_t quad_idx, const rnr_quad_parser_snap_conf *parser_snap_conf)
{
    uint32_t reg_parser_core_configuration_snap_org_code = 0;

#ifdef VALIDATE_PARMS
    if(!parser_snap_conf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!parser_snap_conf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!parser_ipv6_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_ipv6_filter->hop_by_hop_match >= _1BITS_MAX_VAL_) ||
       (parser_ipv6_filter->routing_eh >= _1BITS_MAX_VAL_) ||
       (parser_ipv6_filter->dest_opt_eh >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG, reg_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg);

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
    if (!parser_ipv6_filter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_eng = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_ppp_ip_prot_code = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!ppp_code_0 || !ppp_code_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_qtag_ethtype = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!ethtype_qtag_0 || !ethtype_qtag_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_user_ethtype_0_1 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!ethype_0 || !ethype_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_user_ethtype_2_3 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!ethype_2 || !ethype_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_user_ethtype_config = 0;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_user_ethtype_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!parser_core_configuration_user_ethtype_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0 = 0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_da_filt_valid_cfg_0 = 0;

#ifdef VALIDATE_PARMS
    if(!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!da_filter_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_gre_protocol_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!gre_protocol)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_parser_core_configuration_prop_tag_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_prop_tag_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!parser_core_configuration_prop_tag_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_dos_attack_set(uint8_t quad_idx, uint16_t mask)
{
    uint32_t reg_parser_core_configuration_dos_attack = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_dos_attack = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DOS_ATTACK, MASK, reg_parser_core_configuration_dos_attack, mask);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DOS_ATTACK, reg_parser_core_configuration_dos_attack);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_dos_attack_get(uint8_t quad_idx, uint16_t *mask)
{
    uint32_t reg_parser_core_configuration_dos_attack;

#ifdef VALIDATE_PARMS
    if (!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DOS_ATTACK, reg_parser_core_configuration_dos_attack);

    *mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_DOS_ATTACK, MASK, reg_parser_core_configuration_dos_attack);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_set(uint8_t quad_idx, uint16_t v4_size, uint16_t v6_size)
{
    uint32_t reg_parser_core_configuration_icmp_max_size = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (v4_size >= _11BITS_MAX_VAL_) ||
       (v6_size >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_icmp_max_size = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, V4_SIZE, reg_parser_core_configuration_icmp_max_size, v4_size);
    reg_parser_core_configuration_icmp_max_size = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, V6_SIZE, reg_parser_core_configuration_icmp_max_size, v6_size);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, reg_parser_core_configuration_icmp_max_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_get(uint8_t quad_idx, uint16_t *v4_size, uint16_t *v6_size)
{
    uint32_t reg_parser_core_configuration_icmp_max_size;

#ifdef VALIDATE_PARMS
    if (!v4_size || !v6_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, reg_parser_core_configuration_icmp_max_size);

    *v4_size = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, V4_SIZE, reg_parser_core_configuration_icmp_max_size);
    *v6_size = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE, V6_SIZE, reg_parser_core_configuration_icmp_max_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_key_cfg_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_key_cfg *parser_core_configuration_key_cfg)
{
    uint32_t reg_parser_core_configuration_key_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!parser_core_configuration_key_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (parser_core_configuration_key_cfg->l2_exclude_smac >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_key_cfg->tcp_pure_ack_mask >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_key_cfg->incude_dei_in_vlans_crc >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_key_cfg->key_size >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_key_cfg->max_num_of_vlans_in_crc >= _4BITS_MAX_VAL_) ||
       (parser_core_configuration_key_cfg->l3_tcp_pure_ack_mask >= _1BITS_MAX_VAL_) ||
       (parser_core_configuration_key_cfg->rsrv >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L2_TOS_MASK, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->l2_tos_mask);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L3_TOS_MASK, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->l3_tos_mask);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L2_EXCLUDE_SMAC, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->l2_exclude_smac);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, TCP_PURE_ACK_MASK, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->tcp_pure_ack_mask);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, INCUDE_DEI_IN_VLANS_CRC, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->incude_dei_in_vlans_crc);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, KEY_SIZE, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->key_size);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, MAX_NUM_OF_VLANS_IN_CRC, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->max_num_of_vlans_in_crc);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L3_TCP_PURE_ACK_MASK, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->l3_tcp_pure_ack_mask);
    reg_parser_core_configuration_key_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, RSRV, reg_parser_core_configuration_key_cfg, parser_core_configuration_key_cfg->rsrv);

    RU_REG_WRITE(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, reg_parser_core_configuration_key_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_key_cfg_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_key_cfg *parser_core_configuration_key_cfg)
{
    uint32_t reg_parser_core_configuration_key_cfg;

#ifdef VALIDATE_PARMS
    if (!parser_core_configuration_key_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, reg_parser_core_configuration_key_cfg);

    parser_core_configuration_key_cfg->l2_tos_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L2_TOS_MASK, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->l3_tos_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L3_TOS_MASK, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->l2_exclude_smac = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L2_EXCLUDE_SMAC, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->tcp_pure_ack_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, TCP_PURE_ACK_MASK, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->incude_dei_in_vlans_crc = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, INCUDE_DEI_IN_VLANS_CRC, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->key_size = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, KEY_SIZE, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->max_num_of_vlans_in_crc = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, MAX_NUM_OF_VLANS_IN_CRC, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->l3_tcp_pure_ack_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, L3_TCP_PURE_ACK_MASK, reg_parser_core_configuration_key_cfg);
    parser_core_configuration_key_cfg->rsrv = RU_FIELD_GET(quad_idx, RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG, RSRV, reg_parser_core_configuration_key_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_set(uint8_t quad_idx, const rnr_quad_general_config_dma_arb_cfg *general_config_dma_arb_cfg)
{
    uint32_t reg_general_config_dma_arb_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!general_config_dma_arb_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (general_config_dma_arb_cfg->use_fifo_for_ddr_only >= _1BITS_MAX_VAL_) ||
       (general_config_dma_arb_cfg->token_arbiter_is_rr >= _1BITS_MAX_VAL_) ||
       (general_config_dma_arb_cfg->chicken_no_flowctrl >= _1BITS_MAX_VAL_) ||
       (general_config_dma_arb_cfg->flow_ctrl_clear_token >= _1BITS_MAX_VAL_) ||
       (general_config_dma_arb_cfg->ddr_congest_threshold >= _5BITS_MAX_VAL_) ||
       (general_config_dma_arb_cfg->psram_congest_threshold >= _5BITS_MAX_VAL_) ||
       (general_config_dma_arb_cfg->enable_reply_threshold >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, USE_FIFO_FOR_DDR_ONLY, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->use_fifo_for_ddr_only);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, TOKEN_ARBITER_IS_RR, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->token_arbiter_is_rr);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, CHICKEN_NO_FLOWCTRL, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->chicken_no_flowctrl);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, FLOW_CTRL_CLEAR_TOKEN, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->flow_ctrl_clear_token);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, DDR_CONGEST_THRESHOLD, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->ddr_congest_threshold);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, PSRAM_CONGEST_THRESHOLD, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->psram_congest_threshold);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, ENABLE_REPLY_THRESHOLD, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->enable_reply_threshold);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, DDR_REPLY_THRESHOLD, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->ddr_reply_threshold);
    reg_general_config_dma_arb_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, PSRAM_REPLY_THRESHOLD, reg_general_config_dma_arb_cfg, general_config_dma_arb_cfg->psram_reply_threshold);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, reg_general_config_dma_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_get(uint8_t quad_idx, rnr_quad_general_config_dma_arb_cfg *general_config_dma_arb_cfg)
{
    uint32_t reg_general_config_dma_arb_cfg;

#ifdef VALIDATE_PARMS
    if (!general_config_dma_arb_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, reg_general_config_dma_arb_cfg);

    general_config_dma_arb_cfg->use_fifo_for_ddr_only = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, USE_FIFO_FOR_DDR_ONLY, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->token_arbiter_is_rr = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, TOKEN_ARBITER_IS_RR, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->chicken_no_flowctrl = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, CHICKEN_NO_FLOWCTRL, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->flow_ctrl_clear_token = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, FLOW_CTRL_CLEAR_TOKEN, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->ddr_congest_threshold = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, DDR_CONGEST_THRESHOLD, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->psram_congest_threshold = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, PSRAM_CONGEST_THRESHOLD, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->enable_reply_threshold = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, ENABLE_REPLY_THRESHOLD, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->ddr_reply_threshold = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, DDR_REPLY_THRESHOLD, reg_general_config_dma_arb_cfg);
    general_config_dma_arb_cfg->psram_reply_threshold = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG, PSRAM_REPLY_THRESHOLD, reg_general_config_dma_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_set(uint8_t quad_idx, uint32_t val)
{
    uint32_t reg_general_config_psram0_base = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram1_base = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram2_base = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram3_base = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_ddr0_base = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_ddr1_base = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram0_mask = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram1_mask = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram2_mask = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_psram3_mask = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_ddr0_mask = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_ddr1_mask = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_profiling_config = 0;

#ifdef VALIDATE_PARMS
    if(!general_config_profiling_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (general_config_profiling_config->counter_lsb_sel >= _5BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_0 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_1 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_2 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_3 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_4 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_5 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_6 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_7 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_8 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_9 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_10 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_11 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_12 >= _1BITS_MAX_VAL_) ||
       (general_config_profiling_config->enable_trace_core_13 >= _1BITS_MAX_VAL_))
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
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_6, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_6);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_7, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_7);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_8, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_8);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_9, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_9);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_10, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_10);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_11, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_11);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_12, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_12);
    reg_general_config_profiling_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_13, reg_general_config_profiling_config, general_config_profiling_config->enable_trace_core_13);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, reg_general_config_profiling_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_get(uint8_t quad_idx, rnr_quad_general_config_profiling_config *general_config_profiling_config)
{
    uint32_t reg_general_config_profiling_config;

#ifdef VALIDATE_PARMS
    if (!general_config_profiling_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    general_config_profiling_config->enable_trace_core_6 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_6, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_7 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_7, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_8 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_8, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_9 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_9, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_10 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_10, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_11 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_11, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_12 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_12, reg_general_config_profiling_config);
    general_config_profiling_config->enable_trace_core_13 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG, ENABLE_TRACE_CORE_13, reg_general_config_profiling_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread)
{
    uint32_t reg_general_config_bkpt_0_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_1_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_2_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_3_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_4_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_5_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_6_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_7_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!addr || !thread)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_bkpt_gen_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!handler_addr || !update_pc_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_general_config_powersave_config = 0;

#ifdef VALIDATE_PARMS
    if(!general_config_powersave_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (general_config_powersave_config->enable_powersave_core_0 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_1 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_2 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_3 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_4 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_5 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_6 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_7 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_8 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_9 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_10 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_11 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_12 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_powersave_core_13 >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_cpu_if_clk_gating >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_common_reg_clk_gating >= _1BITS_MAX_VAL_) ||
       (general_config_powersave_config->enable_ec_blocks_clk_gating >= _1BITS_MAX_VAL_))
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
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_6, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_6);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_7, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_7);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_8, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_8);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_9, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_9);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_10, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_10);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_11, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_11);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_12, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_12);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_13, reg_general_config_powersave_config, general_config_powersave_config->enable_powersave_core_13);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_CPU_IF_CLK_GATING, reg_general_config_powersave_config, general_config_powersave_config->enable_cpu_if_clk_gating);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_COMMON_REG_CLK_GATING, reg_general_config_powersave_config, general_config_powersave_config->enable_common_reg_clk_gating);
    reg_general_config_powersave_config = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_EC_BLOCKS_CLK_GATING, reg_general_config_powersave_config, general_config_powersave_config->enable_ec_blocks_clk_gating);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, reg_general_config_powersave_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_get(uint8_t quad_idx, rnr_quad_general_config_powersave_config *general_config_powersave_config)
{
    uint32_t reg_general_config_powersave_config;

#ifdef VALIDATE_PARMS
    if (!general_config_powersave_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    general_config_powersave_config->enable_powersave_core_6 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_6, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_7 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_7, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_8 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_8, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_9 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_9, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_10 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_10, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_11 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_11, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_12 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_12, reg_general_config_powersave_config);
    general_config_powersave_config->enable_powersave_core_13 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_POWERSAVE_CORE_13, reg_general_config_powersave_config);
    general_config_powersave_config->enable_cpu_if_clk_gating = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_CPU_IF_CLK_GATING, reg_general_config_powersave_config);
    general_config_powersave_config->enable_common_reg_clk_gating = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_COMMON_REG_CLK_GATING, reg_general_config_powersave_config);
    general_config_powersave_config->enable_ec_blocks_clk_gating = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG, ENABLE_EC_BLOCKS_CLK_GATING, reg_general_config_powersave_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_powersave_status_get(uint8_t quad_idx, rnr_quad_general_config_powersave_status *general_config_powersave_status)
{
    uint32_t reg_general_config_powersave_status;

#ifdef VALIDATE_PARMS
    if (!general_config_powersave_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, reg_general_config_powersave_status);

    general_config_powersave_status->acc_status_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_0, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_1, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_2, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_3, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_4 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_4, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_5 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_5, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_6 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_6, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_7 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_7, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_8 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_8, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_9 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_9, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_10 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_10, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_11 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_11, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_12 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_12, reg_general_config_powersave_status);
    general_config_powersave_status->acc_status_13 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, ACC_STATUS_13, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_0, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_1, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_2, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_3, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_4 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_4, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_5 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_5, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_6 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_6, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_7 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_7, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_8 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_8, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_9 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_9, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_10 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_10, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_11 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_11, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_12 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_12, reg_general_config_powersave_status);
    general_config_powersave_status->core_status_13 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS, CORE_STATUS_13, reg_general_config_powersave_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_0_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_data_bkpt_0_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_0_cfg, data_addr_start);
    reg_general_config_data_bkpt_0_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_0_cfg, data_addr_stop);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG, reg_general_config_data_bkpt_0_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_0_cfg;

#ifdef VALIDATE_PARMS
    if (!data_addr_start || !data_addr_stop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG, reg_general_config_data_bkpt_0_cfg);

    *data_addr_start = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_0_cfg);
    *data_addr_stop = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_0_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_1_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_data_bkpt_1_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_1_cfg, data_addr_start);
    reg_general_config_data_bkpt_1_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_1_cfg, data_addr_stop);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG, reg_general_config_data_bkpt_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_1_cfg;

#ifdef VALIDATE_PARMS
    if (!data_addr_start || !data_addr_stop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG, reg_general_config_data_bkpt_1_cfg);

    *data_addr_start = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_1_cfg);
    *data_addr_stop = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_1_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_2_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_data_bkpt_2_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_2_cfg, data_addr_start);
    reg_general_config_data_bkpt_2_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_2_cfg, data_addr_stop);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG, reg_general_config_data_bkpt_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_2_cfg;

#ifdef VALIDATE_PARMS
    if (!data_addr_start || !data_addr_stop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG, reg_general_config_data_bkpt_2_cfg);

    *data_addr_start = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_2_cfg);
    *data_addr_stop = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_2_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_3_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_data_bkpt_3_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_3_cfg, data_addr_start);
    reg_general_config_data_bkpt_3_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_3_cfg, data_addr_stop);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG, reg_general_config_data_bkpt_3_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop)
{
    uint32_t reg_general_config_data_bkpt_3_cfg;

#ifdef VALIDATE_PARMS
    if (!data_addr_start || !data_addr_stop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG, reg_general_config_data_bkpt_3_cfg);

    *data_addr_start = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG, DATA_ADDR_START, reg_general_config_data_bkpt_3_cfg);
    *data_addr_stop = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG, DATA_ADDR_STOP, reg_general_config_data_bkpt_3_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_set(uint8_t quad_idx, uint8_t thread_0, uint8_t thread_1, uint8_t thread_2, uint8_t thread_3)
{
    uint32_t reg_general_config_data_bkpt_common_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (thread_0 >= _4BITS_MAX_VAL_) ||
       (thread_1 >= _4BITS_MAX_VAL_) ||
       (thread_2 >= _4BITS_MAX_VAL_) ||
       (thread_3 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_data_bkpt_common_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_0, reg_general_config_data_bkpt_common_cfg, thread_0);
    reg_general_config_data_bkpt_common_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_1, reg_general_config_data_bkpt_common_cfg, thread_1);
    reg_general_config_data_bkpt_common_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_2, reg_general_config_data_bkpt_common_cfg, thread_2);
    reg_general_config_data_bkpt_common_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_3, reg_general_config_data_bkpt_common_cfg, thread_3);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, reg_general_config_data_bkpt_common_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_get(uint8_t quad_idx, uint8_t *thread_0, uint8_t *thread_1, uint8_t *thread_2, uint8_t *thread_3)
{
    uint32_t reg_general_config_data_bkpt_common_cfg;

#ifdef VALIDATE_PARMS
    if (!thread_0 || !thread_1 || !thread_2 || !thread_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, reg_general_config_data_bkpt_common_cfg);

    *thread_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_0, reg_general_config_data_bkpt_common_cfg);
    *thread_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_1, reg_general_config_data_bkpt_common_cfg);
    *thread_2 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_2, reg_general_config_data_bkpt_common_cfg);
    *thread_3 = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG, THREAD_3, reg_general_config_data_bkpt_common_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ubus_counter_control_set(uint8_t quad_idx, bdmf_boolean enable_statistics, bdmf_boolean sw_reset, uint8_t dest_pid, uint8_t master_select)
{
    uint32_t reg_general_config_ubus_counter_control = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (enable_statistics >= _1BITS_MAX_VAL_) ||
       (sw_reset >= _1BITS_MAX_VAL_) ||
       (master_select >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_ubus_counter_control = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, ENABLE_STATISTICS, reg_general_config_ubus_counter_control, enable_statistics);
    reg_general_config_ubus_counter_control = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, SW_RESET, reg_general_config_ubus_counter_control, sw_reset);
    reg_general_config_ubus_counter_control = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, DEST_PID, reg_general_config_ubus_counter_control, dest_pid);
    reg_general_config_ubus_counter_control = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, MASTER_SELECT, reg_general_config_ubus_counter_control, master_select);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, reg_general_config_ubus_counter_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ubus_counter_control_get(uint8_t quad_idx, bdmf_boolean *enable_statistics, bdmf_boolean *sw_reset, uint8_t *dest_pid, uint8_t *master_select)
{
    uint32_t reg_general_config_ubus_counter_control;

#ifdef VALIDATE_PARMS
    if (!enable_statistics || !sw_reset || !dest_pid || !master_select)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, reg_general_config_ubus_counter_control);

    *enable_statistics = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, ENABLE_STATISTICS, reg_general_config_ubus_counter_control);
    *sw_reset = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, SW_RESET, reg_general_config_ubus_counter_control);
    *dest_pid = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, DEST_PID, reg_general_config_ubus_counter_control);
    *master_select = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL, MASTER_SELECT, reg_general_config_ubus_counter_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ubus_down_counter_set(uint8_t quad_idx, uint32_t downcnt_value)
{
    uint32_t reg_general_config_ubus_down_counter = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (downcnt_value >= _30BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_ubus_down_counter = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_DOWN_COUNTER, DOWNCNT_VALUE, reg_general_config_ubus_down_counter, downcnt_value);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_DOWN_COUNTER, reg_general_config_ubus_down_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_ubus_down_counter_get(uint8_t quad_idx, uint32_t *downcnt_value)
{
    uint32_t reg_general_config_ubus_down_counter;

#ifdef VALIDATE_PARMS
    if (!downcnt_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_DOWN_COUNTER, reg_general_config_ubus_down_counter);

    *downcnt_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_UBUS_DOWN_COUNTER, DOWNCNT_VALUE, reg_general_config_ubus_down_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_all_xfers_cnt_get(uint8_t quad_idx, uint32_t *counter_value)
{
    uint32_t reg_general_config_all_xfers_cnt;

#ifdef VALIDATE_PARMS
    if (!counter_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_ALL_XFERS_CNT, reg_general_config_all_xfers_cnt);

    *counter_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_ALL_XFERS_CNT, COUNTER_VALUE, reg_general_config_all_xfers_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_read_xfers_cnt_get(uint8_t quad_idx, uint32_t *counter_value)
{
    uint32_t reg_general_config_read_xfers_cnt;

#ifdef VALIDATE_PARMS
    if (!counter_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_READ_XFERS_CNT, reg_general_config_read_xfers_cnt);

    *counter_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_READ_XFERS_CNT, COUNTER_VALUE, reg_general_config_read_xfers_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_read_data_cnt_get(uint8_t quad_idx, uint32_t *counter_value)
{
    uint32_t reg_general_config_read_data_cnt;

#ifdef VALIDATE_PARMS
    if (!counter_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_READ_DATA_CNT, reg_general_config_read_data_cnt);

    *counter_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_READ_DATA_CNT, COUNTER_VALUE, reg_general_config_read_data_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_write_data_cnt_get(uint8_t quad_idx, uint32_t *counter_value)
{
    uint32_t reg_general_config_write_data_cnt;

#ifdef VALIDATE_PARMS
    if (!counter_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_WRITE_DATA_CNT, reg_general_config_write_data_cnt);

    *counter_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_WRITE_DATA_CNT, COUNTER_VALUE, reg_general_config_write_data_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_misc_cfg_set(uint8_t quad_idx, uint8_t ddr_pid)
{
    uint32_t reg_general_config_misc_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_misc_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_MISC_CFG, DDR_PID, reg_general_config_misc_cfg, ddr_pid);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_MISC_CFG, reg_general_config_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_misc_cfg_get(uint8_t quad_idx, uint8_t *ddr_pid)
{
    uint32_t reg_general_config_misc_cfg;

#ifdef VALIDATE_PARMS
    if (!ddr_pid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_MISC_CFG, reg_general_config_misc_cfg);

    *ddr_pid = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_MISC_CFG, DDR_PID, reg_general_config_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_aqm_control_set(uint8_t quad_idx, bdmf_boolean enable_counter, bdmf_boolean enable_random)
{
    uint32_t reg_general_config_aqm_control = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (enable_counter >= _1BITS_MAX_VAL_) ||
       (enable_random >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_aqm_control = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL, ENABLE_COUNTER, reg_general_config_aqm_control, enable_counter);
    reg_general_config_aqm_control = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL, ENABLE_RANDOM, reg_general_config_aqm_control, enable_random);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL, reg_general_config_aqm_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_aqm_control_get(uint8_t quad_idx, bdmf_boolean *enable_counter, bdmf_boolean *enable_random)
{
    uint32_t reg_general_config_aqm_control;

#ifdef VALIDATE_PARMS
    if (!enable_counter || !enable_random)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL, reg_general_config_aqm_control);

    *enable_counter = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL, ENABLE_COUNTER, reg_general_config_aqm_control);
    *enable_random = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL, ENABLE_RANDOM, reg_general_config_aqm_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_aqm_randm_value_get(uint8_t quad_idx, uint32_t *random_value)
{
    uint32_t reg_general_config_aqm_randm_value;

#ifdef VALIDATE_PARMS
    if (!random_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDM_VALUE, reg_general_config_aqm_randm_value);

    *random_value = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDM_VALUE, RANDOM_VALUE, reg_general_config_aqm_randm_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_aqm_random_seed_set(uint8_t quad_idx, uint32_t random_seed)
{
    uint32_t reg_general_config_aqm_random_seed = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_aqm_random_seed = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_SEED, RANDOM_SEED, reg_general_config_aqm_random_seed, random_seed);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_SEED, reg_general_config_aqm_random_seed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_aqm_random_seed_get(uint8_t quad_idx, uint32_t *random_seed)
{
    uint32_t reg_general_config_aqm_random_seed;

#ifdef VALIDATE_PARMS
    if (!random_seed)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_SEED, reg_general_config_aqm_random_seed);

    *random_seed = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_SEED, RANDOM_SEED, reg_general_config_aqm_random_seed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_aqm_random_test_inc_set(uint8_t quad_idx, bdmf_boolean random_inc)
{
    uint32_t reg_general_config_aqm_random_test_inc = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (random_inc >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_aqm_random_test_inc = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_TEST_INC, RANDOM_INC, reg_general_config_aqm_random_test_inc, random_inc);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_TEST_INC, reg_general_config_aqm_random_test_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_multi_psel_cfg_set(uint8_t quad_idx, uint16_t multi_psel_master_sel, uint16_t multi_psel_mask)
{
    uint32_t reg_general_config_multi_psel_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (multi_psel_master_sel >= _14BITS_MAX_VAL_) ||
       (multi_psel_mask >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_config_multi_psel_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG, MULTI_PSEL_MASTER_SEL, reg_general_config_multi_psel_cfg, multi_psel_master_sel);
    reg_general_config_multi_psel_cfg = RU_FIELD_SET(quad_idx, RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG, MULTI_PSEL_MASK, reg_general_config_multi_psel_cfg, multi_psel_mask);

    RU_REG_WRITE(quad_idx, RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG, reg_general_config_multi_psel_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_general_config_multi_psel_cfg_get(uint8_t quad_idx, uint16_t *multi_psel_master_sel, uint16_t *multi_psel_mask)
{
    uint32_t reg_general_config_multi_psel_cfg;

#ifdef VALIDATE_PARMS
    if (!multi_psel_master_sel || !multi_psel_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG, reg_general_config_multi_psel_cfg);

    *multi_psel_master_sel = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG, MULTI_PSEL_MASTER_SEL, reg_general_config_multi_psel_cfg);
    *multi_psel_mask = RU_FIELD_GET(quad_idx, RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG, MULTI_PSEL_MASK, reg_general_config_multi_psel_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_set(uint8_t quad_idx, const rnr_quad_debug_fifo_config *debug_fifo_config)
{
    uint32_t reg_debug_fifo_config = 0;

#ifdef VALIDATE_PARMS
    if(!debug_fifo_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (debug_fifo_config->psram_hdr_sw_rst_0 >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_data_sw_rst_0 >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->ddr_hdr_sw_rst_0 >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->select_fifos_for_debug >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_hdr_sw_rst_1 >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_data_sw_rst_1 >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->ddr_hdr_sw_rst_1 >= _1BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_hdr_sw_rd_addr >= _4BITS_MAX_VAL_) ||
       (debug_fifo_config->psram_data_sw_rd_addr >= _4BITS_MAX_VAL_) ||
       (debug_fifo_config->ddr_hdr_sw_rd_addr >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RST_0, reg_debug_fifo_config, debug_fifo_config->psram_hdr_sw_rst_0);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RST_0, reg_debug_fifo_config, debug_fifo_config->psram_data_sw_rst_0);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RST_0, reg_debug_fifo_config, debug_fifo_config->ddr_hdr_sw_rst_0);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, SELECT_FIFOS_FOR_DEBUG, reg_debug_fifo_config, debug_fifo_config->select_fifos_for_debug);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RST_1, reg_debug_fifo_config, debug_fifo_config->psram_hdr_sw_rst_1);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RST_1, reg_debug_fifo_config, debug_fifo_config->psram_data_sw_rst_1);
    reg_debug_fifo_config = RU_FIELD_SET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RST_1, reg_debug_fifo_config, debug_fifo_config->ddr_hdr_sw_rst_1);
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
    if (!debug_fifo_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, reg_debug_fifo_config);

    debug_fifo_config->psram_hdr_sw_rst_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RST_0, reg_debug_fifo_config);
    debug_fifo_config->psram_data_sw_rst_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RST_0, reg_debug_fifo_config);
    debug_fifo_config->ddr_hdr_sw_rst_0 = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RST_0, reg_debug_fifo_config);
    debug_fifo_config->select_fifos_for_debug = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, SELECT_FIFOS_FOR_DEBUG, reg_debug_fifo_config);
    debug_fifo_config->psram_hdr_sw_rst_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RST_1, reg_debug_fifo_config);
    debug_fifo_config->psram_data_sw_rst_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RST_1, reg_debug_fifo_config);
    debug_fifo_config->ddr_hdr_sw_rst_1 = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RST_1, reg_debug_fifo_config);
    debug_fifo_config->psram_hdr_sw_rd_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_HDR_SW_RD_ADDR, reg_debug_fifo_config);
    debug_fifo_config->psram_data_sw_rd_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, PSRAM_DATA_SW_RD_ADDR, reg_debug_fifo_config);
    debug_fifo_config->ddr_hdr_sw_rd_addr = RU_FIELD_GET(quad_idx, RNR_QUAD, DEBUG_FIFO_CONFIG, DDR_HDR_SW_RD_ADDR, reg_debug_fifo_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_hdr_fifo_status *debug_psram_hdr_fifo_status)
{
    uint32_t reg_debug_psram_hdr_fifo_status;

#ifdef VALIDATE_PARMS
    if (!debug_psram_hdr_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!debug_psram_data_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!debug_ddr_hdr_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!debug_ddr_data_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!read_addr || !used_words)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_ext_flowctrl_config_token_val = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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

bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ext_flowctrl_config2_token_val_2 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 36))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ext_flowctrl_config2_token_val_2 = RU_FIELD_SET(quad_idx, RNR_QUAD, EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2, VAL, reg_ext_flowctrl_config2_token_val_2, val);

    RU_REG_RAM_WRITE(quad_idx, index, RNR_QUAD, EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2, reg_ext_flowctrl_config2_token_val_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(uint8_t quad_idx, uint32_t index, uint32_t *val)
{
    uint32_t reg_ext_flowctrl_config2_token_val_2;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 36))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(quad_idx, index, RNR_QUAD, EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2, reg_ext_flowctrl_config2_token_val_2);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2, VAL, reg_ext_flowctrl_config2_token_val_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ubus_decode_cfg_psram_ubus_decode = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    uint32_t reg_ubus_decode_cfg_ddr_ubus_decode = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
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

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ubus_decode_cfg2_psram_ubus_decode2 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ubus_decode_cfg2_psram_ubus_decode2 = RU_FIELD_SET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2, VAL, reg_ubus_decode_cfg2_psram_ubus_decode2, val);

    RU_REG_RAM_WRITE(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2, reg_ubus_decode_cfg2_psram_ubus_decode2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(uint8_t quad_idx, uint32_t index, uint32_t *val)
{
    uint32_t reg_ubus_decode_cfg2_psram_ubus_decode2;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2, reg_ubus_decode_cfg2_psram_ubus_decode2);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2, VAL, reg_ubus_decode_cfg2_psram_ubus_decode2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(uint8_t quad_idx, uint32_t index, uint32_t val)
{
    uint32_t reg_ubus_decode_cfg2_ddr_ubus_decode2 = 0;

#ifdef VALIDATE_PARMS
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ubus_decode_cfg2_ddr_ubus_decode2 = RU_FIELD_SET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG2_DDR_UBUS_DECODE2, VAL, reg_ubus_decode_cfg2_ddr_ubus_decode2, val);

    RU_REG_RAM_WRITE(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG2_DDR_UBUS_DECODE2, reg_ubus_decode_cfg2_ddr_ubus_decode2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(uint8_t quad_idx, uint32_t index, uint32_t *val)
{
    uint32_t reg_ubus_decode_cfg2_ddr_ubus_decode2;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((quad_idx >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(quad_idx, index, RNR_QUAD, UBUS_DECODE_CFG2_DDR_UBUS_DECODE2, reg_ubus_decode_cfg2_ddr_ubus_decode2);

    *val = RU_FIELD_GET(quad_idx, RNR_QUAD, UBUS_DECODE_CFG2_DDR_UBUS_DECODE2, VAL, reg_ubus_decode_cfg2_ddr_ubus_decode2);

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
    bdmf_address_parser_core_configuration_dos_attack,
    bdmf_address_parser_core_configuration_icmp_max_size,
    bdmf_address_parser_core_configuration_key_cfg,
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
    bdmf_address_general_config_data_bkpt_0_cfg,
    bdmf_address_general_config_data_bkpt_1_cfg,
    bdmf_address_general_config_data_bkpt_2_cfg,
    bdmf_address_general_config_data_bkpt_3_cfg,
    bdmf_address_general_config_data_bkpt_common_cfg,
    bdmf_address_general_config_ubus_counter_control,
    bdmf_address_general_config_ubus_down_counter,
    bdmf_address_general_config_all_xfers_cnt,
    bdmf_address_general_config_read_xfers_cnt,
    bdmf_address_general_config_read_data_cnt,
    bdmf_address_general_config_write_data_cnt,
    bdmf_address_general_config_misc_cfg,
    bdmf_address_general_config_aqm_control,
    bdmf_address_general_config_aqm_randm_value,
    bdmf_address_general_config_aqm_random_seed,
    bdmf_address_general_config_aqm_random_test_inc,
    bdmf_address_general_config_multi_psel_cfg,
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
    bdmf_address_ext_flowctrl_config2_token_val_2,
    bdmf_address_ubus_decode_cfg_psram_ubus_decode,
    bdmf_address_ubus_decode_cfg_ddr_ubus_decode,
    bdmf_address_ubus_decode_cfg2_psram_ubus_decode2,
    bdmf_address_ubus_decode_cfg2_ddr_ubus_decode2,
}
bdmf_address;

static int ag_drv_rnr_quad_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_rnr_quad_parser_vid0:
        ag_err = ag_drv_rnr_quad_parser_vid0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid1:
        ag_err = ag_drv_rnr_quad_parser_vid1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid2:
        ag_err = ag_drv_rnr_quad_parser_vid2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid3:
        ag_err = ag_drv_rnr_quad_parser_vid3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid4:
        ag_err = ag_drv_rnr_quad_parser_vid4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid5:
        ag_err = ag_drv_rnr_quad_parser_vid5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid6:
        ag_err = ag_drv_rnr_quad_parser_vid6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_vid7:
        ag_err = ag_drv_rnr_quad_parser_vid7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_ip0:
    {
        rnr_quad_parser_ip0 parser_ip0 = { .ip_address = parm[2].value.unumber, .ip_address_mask = parm[3].value.unumber, .ip_filter0_dip_en = parm[4].value.unumber, .ip_filter0_valid = parm[5].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_ip0_set(parm[1].value.unumber, &parser_ip0);
        break;
    }
    case cli_rnr_quad_parser_ip1:
    {
        rnr_quad_parser_ip0 parser_ip0 = { .ip_address = parm[2].value.unumber, .ip_address_mask = parm[3].value.unumber, .ip_filter0_dip_en = parm[4].value.unumber, .ip_filter0_valid = parm[5].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_ip1_set(parm[1].value.unumber, &parser_ip0);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof0:
    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0 = { .hard_nest_profile = parm[2].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(parm[1].value.unumber, &parser_hardcoded_ethtype_prof0);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof1:
    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0 = { .hard_nest_profile = parm[2].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(parm[1].value.unumber, &parser_hardcoded_ethtype_prof0);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof2:
    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0 = { .hard_nest_profile = parm[2].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(parm[1].value.unumber, &parser_hardcoded_ethtype_prof0);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof0:
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_qtag_nest_prof1:
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_qtag_nest_prof2:
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_qtag_nest_max_vlans:
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_max_vlans_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol0:
        ag_err = ag_drv_rnr_quad_parser_ip_protocol0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol1:
        ag_err = ag_drv_rnr_quad_parser_ip_protocol1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol2:
        ag_err = ag_drv_rnr_quad_parser_ip_protocol2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_ip_protocol3:
        ag_err = ag_drv_rnr_quad_parser_ip_protocol3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter:
    {
        rnr_quad_parser_da_filter parser_da_filter = { .da_filt_msb = parm[2].value.unumber, .da_filt_lsb = parm[3].value.unumber, .da_filt_mask_msb = parm[4].value.unumber, .da_filt_mask_l = parm[5].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_da_filter_set(parm[1].value.unumber, &parser_da_filter);
        break;
    }
    case cli_rnr_quad_parser_da_filter1:
    {
        rnr_quad_parser_da_filter parser_da_filter = { .da_filt_msb = parm[2].value.unumber, .da_filt_lsb = parm[3].value.unumber, .da_filt_mask_msb = parm[4].value.unumber, .da_filt_mask_l = parm[5].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_da_filter1_set(parm[1].value.unumber, &parser_da_filter);
        break;
    }
    case cli_rnr_quad_parser_da_filter2:
        ag_err = ag_drv_rnr_quad_parser_da_filter2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter3:
        ag_err = ag_drv_rnr_quad_parser_da_filter3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter4:
        ag_err = ag_drv_rnr_quad_parser_da_filter4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter5:
        ag_err = ag_drv_rnr_quad_parser_da_filter5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter6:
        ag_err = ag_drv_rnr_quad_parser_da_filter6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter7:
        ag_err = ag_drv_rnr_quad_parser_da_filter7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_da_filter8:
        ag_err = ag_drv_rnr_quad_parser_da_filter8_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_da_filter_valid:
    {
        rnr_quad_da_filter_valid da_filter_valid = { .da_filt0_valid = parm[2].value.unumber, .da_filt1_valid = parm[3].value.unumber, .da_filt2_valid = parm[4].value.unumber, .da_filt3_valid = parm[5].value.unumber, .da_filt4_valid = parm[6].value.unumber, .da_filt5_valid = parm[7].value.unumber, .da_filt6_valid = parm[8].value.unumber, .da_filt7_valid = parm[9].value.unumber, .da_filt8_valid = parm[10].value.unumber};
        ag_err = ag_drv_rnr_quad_da_filter_valid_set(parm[1].value.unumber, &da_filter_valid);
        break;
    }
    case cli_rnr_quad_exception_bits:
        ag_err = ag_drv_rnr_quad_exception_bits_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_tcp_flags:
        ag_err = ag_drv_rnr_quad_tcp_flags_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_profile_us:
        ag_err = ag_drv_rnr_quad_profile_us_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_disable_l2tp_source_port:
        ag_err = ag_drv_rnr_quad_disable_l2tp_source_port_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_snap_conf:
    {
        rnr_quad_parser_snap_conf parser_snap_conf = { .code = parm[2].value.unumber, .en_rfc1042 = parm[3].value.unumber, .en_8021q = parm[4].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_snap_conf_set(parm[1].value.unumber, &parser_snap_conf);
        break;
    }
    case cli_rnr_quad_parser_ipv6_filter:
    {
        rnr_quad_parser_ipv6_filter parser_ipv6_filter = { .hop_by_hop_match = parm[2].value.unumber, .routing_eh = parm[3].value.unumber, .dest_opt_eh = parm[4].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_ipv6_filter_set(parm[1].value.unumber, &parser_ipv6_filter);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_eng:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_eng_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_qtag_ethtype:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_user_ethtype_0_1:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_user_ethtype_2_3:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_user_ethtype_config:
    {
        rnr_quad_parser_core_configuration_user_ethtype_config parser_core_configuration_user_ethtype_config = { .ethtype_user_prot_0 = parm[2].value.unumber, .ethtype_user_prot_1 = parm[3].value.unumber, .ethtype_user_prot_2 = parm[4].value.unumber, .ethtype_user_prot_3 = parm[5].value.unumber, .ethtype_user_en = parm[6].value.unumber, .ethtype_user_offset_0 = parm[7].value.unumber, .ethtype_user_offset_1 = parm[8].value.unumber, .ethtype_user_offset_2 = parm[9].value.unumber, .ethtype_user_offset_3 = parm[10].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(parm[1].value.unumber, &parser_core_configuration_user_ethtype_config);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1:
    {
        rnr_quad_da_filter_valid da_filter_valid = { .da_filt0_valid = parm[2].value.unumber, .da_filt1_valid = parm[3].value.unumber, .da_filt2_valid = parm[4].value.unumber, .da_filt3_valid = parm[5].value.unumber, .da_filt4_valid = parm[6].value.unumber, .da_filt5_valid = parm[7].value.unumber, .da_filt6_valid = parm[8].value.unumber, .da_filt7_valid = parm[9].value.unumber, .da_filt8_valid = parm[10].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(parm[1].value.unumber, &da_filter_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2:
    {
        rnr_quad_da_filter_valid da_filter_valid = { .da_filt0_valid = parm[2].value.unumber, .da_filt1_valid = parm[3].value.unumber, .da_filt2_valid = parm[4].value.unumber, .da_filt3_valid = parm[5].value.unumber, .da_filt4_valid = parm[6].value.unumber, .da_filt5_valid = parm[7].value.unumber, .da_filt6_valid = parm[8].value.unumber, .da_filt7_valid = parm[9].value.unumber, .da_filt8_valid = parm[10].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(parm[1].value.unumber, &da_filter_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_gre_protocol_cfg:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_prop_tag_cfg:
    {
        rnr_quad_parser_core_configuration_prop_tag_cfg parser_core_configuration_prop_tag_cfg = { .size_profile_0 = parm[2].value.unumber, .size_profile_1 = parm[3].value.unumber, .size_profile_2 = parm[4].value.unumber, .pre_da_dprofile_0 = parm[5].value.unumber, .pre_da_dprofile_1 = parm[6].value.unumber, .pre_da_dprofile_2 = parm[7].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(parm[1].value.unumber, &parser_core_configuration_prop_tag_cfg);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_dos_attack:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_dos_attack_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_icmp_max_size:
        ag_err = ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_parser_core_configuration_key_cfg:
    {
        rnr_quad_parser_core_configuration_key_cfg parser_core_configuration_key_cfg = { .l2_tos_mask = parm[2].value.unumber, .l3_tos_mask = parm[3].value.unumber, .l2_exclude_smac = parm[4].value.unumber, .tcp_pure_ack_mask = parm[5].value.unumber, .incude_dei_in_vlans_crc = parm[6].value.unumber, .key_size = parm[7].value.unumber, .max_num_of_vlans_in_crc = parm[8].value.unumber, .l3_tcp_pure_ack_mask = parm[9].value.unumber, .rsrv = parm[10].value.unumber};
        ag_err = ag_drv_rnr_quad_parser_core_configuration_key_cfg_set(parm[1].value.unumber, &parser_core_configuration_key_cfg);
        break;
    }
    case cli_rnr_quad_general_config_dma_arb_cfg:
    {
        rnr_quad_general_config_dma_arb_cfg general_config_dma_arb_cfg = { .use_fifo_for_ddr_only = parm[2].value.unumber, .token_arbiter_is_rr = parm[3].value.unumber, .chicken_no_flowctrl = parm[4].value.unumber, .flow_ctrl_clear_token = parm[5].value.unumber, .ddr_congest_threshold = parm[6].value.unumber, .psram_congest_threshold = parm[7].value.unumber, .enable_reply_threshold = parm[8].value.unumber, .ddr_reply_threshold = parm[9].value.unumber, .psram_reply_threshold = parm[10].value.unumber};
        ag_err = ag_drv_rnr_quad_general_config_dma_arb_cfg_set(parm[1].value.unumber, &general_config_dma_arb_cfg);
        break;
    }
    case cli_rnr_quad_general_config_psram0_base:
        ag_err = ag_drv_rnr_quad_general_config_psram0_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram1_base:
        ag_err = ag_drv_rnr_quad_general_config_psram1_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram2_base:
        ag_err = ag_drv_rnr_quad_general_config_psram2_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram3_base:
        ag_err = ag_drv_rnr_quad_general_config_psram3_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr0_base:
        ag_err = ag_drv_rnr_quad_general_config_ddr0_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr1_base:
        ag_err = ag_drv_rnr_quad_general_config_ddr1_base_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram0_mask:
        ag_err = ag_drv_rnr_quad_general_config_psram0_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram1_mask:
        ag_err = ag_drv_rnr_quad_general_config_psram1_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram2_mask:
        ag_err = ag_drv_rnr_quad_general_config_psram2_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_psram3_mask:
        ag_err = ag_drv_rnr_quad_general_config_psram3_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr0_mask:
        ag_err = ag_drv_rnr_quad_general_config_ddr0_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_ddr1_mask:
        ag_err = ag_drv_rnr_quad_general_config_ddr1_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_profiling_config:
    {
        rnr_quad_general_config_profiling_config general_config_profiling_config = { .counter_lsb_sel = parm[2].value.unumber, .enable_trace_core_0 = parm[3].value.unumber, .enable_trace_core_1 = parm[4].value.unumber, .enable_trace_core_2 = parm[5].value.unumber, .enable_trace_core_3 = parm[6].value.unumber, .enable_trace_core_4 = parm[7].value.unumber, .enable_trace_core_5 = parm[8].value.unumber, .enable_trace_core_6 = parm[9].value.unumber, .enable_trace_core_7 = parm[10].value.unumber, .enable_trace_core_8 = parm[11].value.unumber, .enable_trace_core_9 = parm[12].value.unumber, .enable_trace_core_10 = parm[13].value.unumber, .enable_trace_core_11 = parm[14].value.unumber, .enable_trace_core_12 = parm[15].value.unumber, .enable_trace_core_13 = parm[16].value.unumber};
        ag_err = ag_drv_rnr_quad_general_config_profiling_config_set(parm[1].value.unumber, &general_config_profiling_config);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_0_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_1_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_2_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_3_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_4_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_5_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_6_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_7_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_bkpt_gen_cfg:
        ag_err = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_powersave_config:
    {
        rnr_quad_general_config_powersave_config general_config_powersave_config = { .time_counter = parm[2].value.unumber, .enable_powersave_core_0 = parm[3].value.unumber, .enable_powersave_core_1 = parm[4].value.unumber, .enable_powersave_core_2 = parm[5].value.unumber, .enable_powersave_core_3 = parm[6].value.unumber, .enable_powersave_core_4 = parm[7].value.unumber, .enable_powersave_core_5 = parm[8].value.unumber, .enable_powersave_core_6 = parm[9].value.unumber, .enable_powersave_core_7 = parm[10].value.unumber, .enable_powersave_core_8 = parm[11].value.unumber, .enable_powersave_core_9 = parm[12].value.unumber, .enable_powersave_core_10 = parm[13].value.unumber, .enable_powersave_core_11 = parm[14].value.unumber, .enable_powersave_core_12 = parm[15].value.unumber, .enable_powersave_core_13 = parm[16].value.unumber, .enable_cpu_if_clk_gating = parm[17].value.unumber, .enable_common_reg_clk_gating = parm[18].value.unumber, .enable_ec_blocks_clk_gating = parm[19].value.unumber};
        ag_err = ag_drv_rnr_quad_general_config_powersave_config_set(parm[1].value.unumber, &general_config_powersave_config);
        break;
    }
    case cli_rnr_quad_general_config_data_bkpt_0_cfg:
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_data_bkpt_1_cfg:
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_data_bkpt_2_cfg:
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_data_bkpt_3_cfg:
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_data_bkpt_common_cfg:
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_rnr_quad_general_config_ubus_counter_control:
        ag_err = ag_drv_rnr_quad_general_config_ubus_counter_control_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_rnr_quad_general_config_ubus_down_counter:
        ag_err = ag_drv_rnr_quad_general_config_ubus_down_counter_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_misc_cfg:
        ag_err = ag_drv_rnr_quad_general_config_misc_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_aqm_control:
        ag_err = ag_drv_rnr_quad_general_config_aqm_control_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_general_config_aqm_random_seed:
        ag_err = ag_drv_rnr_quad_general_config_aqm_random_seed_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_aqm_random_test_inc:
        ag_err = ag_drv_rnr_quad_general_config_aqm_random_test_inc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_rnr_quad_general_config_multi_psel_cfg:
        ag_err = ag_drv_rnr_quad_general_config_multi_psel_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_debug_fifo_config:
    {
        rnr_quad_debug_fifo_config debug_fifo_config = { .psram_hdr_sw_rst_0 = parm[2].value.unumber, .psram_data_sw_rst_0 = parm[3].value.unumber, .ddr_hdr_sw_rst_0 = parm[4].value.unumber, .select_fifos_for_debug = parm[5].value.unumber, .psram_hdr_sw_rst_1 = parm[6].value.unumber, .psram_data_sw_rst_1 = parm[7].value.unumber, .ddr_hdr_sw_rst_1 = parm[8].value.unumber, .psram_hdr_sw_rd_addr = parm[9].value.unumber, .psram_data_sw_rd_addr = parm[10].value.unumber, .ddr_hdr_sw_rd_addr = parm[11].value.unumber};
        ag_err = ag_drv_rnr_quad_debug_fifo_config_set(parm[1].value.unumber, &debug_fifo_config);
        break;
    }
    case cli_rnr_quad_ext_flowctrl_config_token_val:
        ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ext_flowctrl_config2_token_val_2:
        ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode:
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode:
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2:
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2:
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_rnr_quad_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_rnr_quad_parser_vid0:
    {
        uint16_t vid_0;
        bdmf_boolean vid_0_en;
        ag_err = ag_drv_rnr_quad_parser_vid0_get(parm[1].value.unumber, &vid_0, &vid_0_en);
        bdmf_session_print(session, "vid_0 = %u = 0x%x\n", vid_0, vid_0);
        bdmf_session_print(session, "vid_0_en = %u = 0x%x\n", vid_0_en, vid_0_en);
        break;
    }
    case cli_rnr_quad_parser_vid1:
    {
        uint16_t vid_1;
        bdmf_boolean vid_1_en;
        ag_err = ag_drv_rnr_quad_parser_vid1_get(parm[1].value.unumber, &vid_1, &vid_1_en);
        bdmf_session_print(session, "vid_1 = %u = 0x%x\n", vid_1, vid_1);
        bdmf_session_print(session, "vid_1_en = %u = 0x%x\n", vid_1_en, vid_1_en);
        break;
    }
    case cli_rnr_quad_parser_vid2:
    {
        uint16_t vid_2;
        bdmf_boolean vid_2_en;
        ag_err = ag_drv_rnr_quad_parser_vid2_get(parm[1].value.unumber, &vid_2, &vid_2_en);
        bdmf_session_print(session, "vid_2 = %u = 0x%x\n", vid_2, vid_2);
        bdmf_session_print(session, "vid_2_en = %u = 0x%x\n", vid_2_en, vid_2_en);
        break;
    }
    case cli_rnr_quad_parser_vid3:
    {
        uint16_t vid_3;
        bdmf_boolean vid_3_en;
        ag_err = ag_drv_rnr_quad_parser_vid3_get(parm[1].value.unumber, &vid_3, &vid_3_en);
        bdmf_session_print(session, "vid_3 = %u = 0x%x\n", vid_3, vid_3);
        bdmf_session_print(session, "vid_3_en = %u = 0x%x\n", vid_3_en, vid_3_en);
        break;
    }
    case cli_rnr_quad_parser_vid4:
    {
        uint16_t vid_4;
        bdmf_boolean vid_4_en;
        ag_err = ag_drv_rnr_quad_parser_vid4_get(parm[1].value.unumber, &vid_4, &vid_4_en);
        bdmf_session_print(session, "vid_4 = %u = 0x%x\n", vid_4, vid_4);
        bdmf_session_print(session, "vid_4_en = %u = 0x%x\n", vid_4_en, vid_4_en);
        break;
    }
    case cli_rnr_quad_parser_vid5:
    {
        uint16_t vid_5;
        bdmf_boolean vid_5_en;
        ag_err = ag_drv_rnr_quad_parser_vid5_get(parm[1].value.unumber, &vid_5, &vid_5_en);
        bdmf_session_print(session, "vid_5 = %u = 0x%x\n", vid_5, vid_5);
        bdmf_session_print(session, "vid_5_en = %u = 0x%x\n", vid_5_en, vid_5_en);
        break;
    }
    case cli_rnr_quad_parser_vid6:
    {
        uint16_t vid_6;
        bdmf_boolean vid_6_en;
        ag_err = ag_drv_rnr_quad_parser_vid6_get(parm[1].value.unumber, &vid_6, &vid_6_en);
        bdmf_session_print(session, "vid_6 = %u = 0x%x\n", vid_6, vid_6);
        bdmf_session_print(session, "vid_6_en = %u = 0x%x\n", vid_6_en, vid_6_en);
        break;
    }
    case cli_rnr_quad_parser_vid7:
    {
        uint16_t vid_7;
        bdmf_boolean vid_7_en;
        ag_err = ag_drv_rnr_quad_parser_vid7_get(parm[1].value.unumber, &vid_7, &vid_7_en);
        bdmf_session_print(session, "vid_7 = %u = 0x%x\n", vid_7, vid_7);
        bdmf_session_print(session, "vid_7_en = %u = 0x%x\n", vid_7_en, vid_7_en);
        break;
    }
    case cli_rnr_quad_parser_ip0:
    {
        rnr_quad_parser_ip0 parser_ip0;
        ag_err = ag_drv_rnr_quad_parser_ip0_get(parm[1].value.unumber, &parser_ip0);
        bdmf_session_print(session, "ip_address = %u = 0x%x\n", parser_ip0.ip_address, parser_ip0.ip_address);
        bdmf_session_print(session, "ip_address_mask = %u = 0x%x\n", parser_ip0.ip_address_mask, parser_ip0.ip_address_mask);
        bdmf_session_print(session, "ip_filter0_dip_en = %u = 0x%x\n", parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_dip_en);
        bdmf_session_print(session, "ip_filter0_valid = %u = 0x%x\n", parser_ip0.ip_filter0_valid, parser_ip0.ip_filter0_valid);
        break;
    }
    case cli_rnr_quad_parser_ip1:
    {
        rnr_quad_parser_ip0 parser_ip0;
        ag_err = ag_drv_rnr_quad_parser_ip1_get(parm[1].value.unumber, &parser_ip0);
        bdmf_session_print(session, "ip_address = %u = 0x%x\n", parser_ip0.ip_address, parser_ip0.ip_address);
        bdmf_session_print(session, "ip_address_mask = %u = 0x%x\n", parser_ip0.ip_address_mask, parser_ip0.ip_address_mask);
        bdmf_session_print(session, "ip_filter0_dip_en = %u = 0x%x\n", parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_dip_en);
        bdmf_session_print(session, "ip_filter0_valid = %u = 0x%x\n", parser_ip0.ip_filter0_valid, parser_ip0.ip_filter0_valid);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof0:
    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0;
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(parm[1].value.unumber, &parser_hardcoded_ethtype_prof0);
        bdmf_session_print(session, "hard_nest_profile = %u = 0x%x\n", parser_hardcoded_ethtype_prof0.hard_nest_profile, parser_hardcoded_ethtype_prof0.hard_nest_profile);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof1:
    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0;
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(parm[1].value.unumber, &parser_hardcoded_ethtype_prof0);
        bdmf_session_print(session, "hard_nest_profile = %u = 0x%x\n", parser_hardcoded_ethtype_prof0.hard_nest_profile, parser_hardcoded_ethtype_prof0.hard_nest_profile);
        break;
    }
    case cli_rnr_quad_parser_hardcoded_ethtype_prof2:
    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0;
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(parm[1].value.unumber, &parser_hardcoded_ethtype_prof0);
        bdmf_session_print(session, "hard_nest_profile = %u = 0x%x\n", parser_hardcoded_ethtype_prof0.hard_nest_profile, parser_hardcoded_ethtype_prof0.hard_nest_profile);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof0:
    {
        uint8_t qtag_nest_0_profile_0;
        uint8_t qtag_nest_1_profile_0;
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof0_get(parm[1].value.unumber, &qtag_nest_0_profile_0, &qtag_nest_1_profile_0);
        bdmf_session_print(session, "qtag_nest_0_profile_0 = %u = 0x%x\n", qtag_nest_0_profile_0, qtag_nest_0_profile_0);
        bdmf_session_print(session, "qtag_nest_1_profile_0 = %u = 0x%x\n", qtag_nest_1_profile_0, qtag_nest_1_profile_0);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof1:
    {
        uint8_t qtag_nest_0_profile_1;
        uint8_t qtag_nest_1_profile_1;
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof1_get(parm[1].value.unumber, &qtag_nest_0_profile_1, &qtag_nest_1_profile_1);
        bdmf_session_print(session, "qtag_nest_0_profile_1 = %u = 0x%x\n", qtag_nest_0_profile_1, qtag_nest_0_profile_1);
        bdmf_session_print(session, "qtag_nest_1_profile_1 = %u = 0x%x\n", qtag_nest_1_profile_1, qtag_nest_1_profile_1);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_prof2:
    {
        uint8_t qtag_nest_0_profile_2;
        uint8_t qtag_nest_1_profile_2;
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof2_get(parm[1].value.unumber, &qtag_nest_0_profile_2, &qtag_nest_1_profile_2);
        bdmf_session_print(session, "qtag_nest_0_profile_2 = %u = 0x%x\n", qtag_nest_0_profile_2, qtag_nest_0_profile_2);
        bdmf_session_print(session, "qtag_nest_1_profile_2 = %u = 0x%x\n", qtag_nest_1_profile_2, qtag_nest_1_profile_2);
        break;
    }
    case cli_rnr_quad_parser_qtag_nest_max_vlans:
    {
        uint8_t qtag_nest_0_profile_2;
        uint8_t max_num_of_vlans;
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_max_vlans_get(parm[1].value.unumber, &qtag_nest_0_profile_2, &max_num_of_vlans);
        bdmf_session_print(session, "qtag_nest_0_profile_2 = %u = 0x%x\n", qtag_nest_0_profile_2, qtag_nest_0_profile_2);
        bdmf_session_print(session, "max_num_of_vlans = %u = 0x%x\n", max_num_of_vlans, max_num_of_vlans);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol0:
    {
        uint8_t user_ip_prot_0;
        ag_err = ag_drv_rnr_quad_parser_ip_protocol0_get(parm[1].value.unumber, &user_ip_prot_0);
        bdmf_session_print(session, "user_ip_prot_0 = %u = 0x%x\n", user_ip_prot_0, user_ip_prot_0);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol1:
    {
        uint8_t user_ip_prot_1;
        ag_err = ag_drv_rnr_quad_parser_ip_protocol1_get(parm[1].value.unumber, &user_ip_prot_1);
        bdmf_session_print(session, "user_ip_prot_1 = %u = 0x%x\n", user_ip_prot_1, user_ip_prot_1);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol2:
    {
        uint8_t user_ip_prot_2;
        ag_err = ag_drv_rnr_quad_parser_ip_protocol2_get(parm[1].value.unumber, &user_ip_prot_2);
        bdmf_session_print(session, "user_ip_prot_2 = %u = 0x%x\n", user_ip_prot_2, user_ip_prot_2);
        break;
    }
    case cli_rnr_quad_parser_ip_protocol3:
    {
        uint8_t user_ip_prot_3;
        ag_err = ag_drv_rnr_quad_parser_ip_protocol3_get(parm[1].value.unumber, &user_ip_prot_3);
        bdmf_session_print(session, "user_ip_prot_3 = %u = 0x%x\n", user_ip_prot_3, user_ip_prot_3);
        break;
    }
    case cli_rnr_quad_parser_da_filter:
    {
        rnr_quad_parser_da_filter parser_da_filter;
        ag_err = ag_drv_rnr_quad_parser_da_filter_get(parm[1].value.unumber, &parser_da_filter);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", parser_da_filter.da_filt_msb, parser_da_filter.da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_lsb);
        bdmf_session_print(session, "da_filt_mask_msb = %u = 0x%x\n", parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_msb);
        bdmf_session_print(session, "da_filt_mask_l = %u = 0x%x\n", parser_da_filter.da_filt_mask_l, parser_da_filter.da_filt_mask_l);
        break;
    }
    case cli_rnr_quad_parser_da_filter1:
    {
        rnr_quad_parser_da_filter parser_da_filter;
        ag_err = ag_drv_rnr_quad_parser_da_filter1_get(parm[1].value.unumber, &parser_da_filter);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", parser_da_filter.da_filt_msb, parser_da_filter.da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_lsb);
        bdmf_session_print(session, "da_filt_mask_msb = %u = 0x%x\n", parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_msb);
        bdmf_session_print(session, "da_filt_mask_l = %u = 0x%x\n", parser_da_filter.da_filt_mask_l, parser_da_filter.da_filt_mask_l);
        break;
    }
    case cli_rnr_quad_parser_da_filter2:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter2_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter3:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter3_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter4:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter4_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter5:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter5_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter6:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter6_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter7:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter7_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_parser_da_filter8:
    {
        uint16_t da_filt_msb;
        uint32_t da_filt_lsb;
        ag_err = ag_drv_rnr_quad_parser_da_filter8_get(parm[1].value.unumber, &da_filt_msb, &da_filt_lsb);
        bdmf_session_print(session, "da_filt_msb = %u = 0x%x\n", da_filt_msb, da_filt_msb);
        bdmf_session_print(session, "da_filt_lsb = %u = 0x%x\n", da_filt_lsb, da_filt_lsb);
        break;
    }
    case cli_rnr_quad_da_filter_valid:
    {
        rnr_quad_da_filter_valid da_filter_valid;
        ag_err = ag_drv_rnr_quad_da_filter_valid_get(parm[1].value.unumber, &da_filter_valid);
        bdmf_session_print(session, "da_filt0_valid = %u = 0x%x\n", da_filter_valid.da_filt0_valid, da_filter_valid.da_filt0_valid);
        bdmf_session_print(session, "da_filt1_valid = %u = 0x%x\n", da_filter_valid.da_filt1_valid, da_filter_valid.da_filt1_valid);
        bdmf_session_print(session, "da_filt2_valid = %u = 0x%x\n", da_filter_valid.da_filt2_valid, da_filter_valid.da_filt2_valid);
        bdmf_session_print(session, "da_filt3_valid = %u = 0x%x\n", da_filter_valid.da_filt3_valid, da_filter_valid.da_filt3_valid);
        bdmf_session_print(session, "da_filt4_valid = %u = 0x%x\n", da_filter_valid.da_filt4_valid, da_filter_valid.da_filt4_valid);
        bdmf_session_print(session, "da_filt5_valid = %u = 0x%x\n", da_filter_valid.da_filt5_valid, da_filter_valid.da_filt5_valid);
        bdmf_session_print(session, "da_filt6_valid = %u = 0x%x\n", da_filter_valid.da_filt6_valid, da_filter_valid.da_filt6_valid);
        bdmf_session_print(session, "da_filt7_valid = %u = 0x%x\n", da_filter_valid.da_filt7_valid, da_filter_valid.da_filt7_valid);
        bdmf_session_print(session, "da_filt8_valid = %u = 0x%x\n", da_filter_valid.da_filt8_valid, da_filter_valid.da_filt8_valid);
        break;
    }
    case cli_rnr_quad_exception_bits:
    {
        uint32_t exception_en;
        ag_err = ag_drv_rnr_quad_exception_bits_get(parm[1].value.unumber, &exception_en);
        bdmf_session_print(session, "exception_en = %u = 0x%x\n", exception_en, exception_en);
        break;
    }
    case cli_rnr_quad_tcp_flags:
    {
        uint8_t tcp_flags_filt;
        ag_err = ag_drv_rnr_quad_tcp_flags_get(parm[1].value.unumber, &tcp_flags_filt);
        bdmf_session_print(session, "tcp_flags_filt = %u = 0x%x\n", tcp_flags_filt, tcp_flags_filt);
        break;
    }
    case cli_rnr_quad_profile_us:
    {
        uint8_t profile_us;
        ag_err = ag_drv_rnr_quad_profile_us_get(parm[1].value.unumber, &profile_us);
        bdmf_session_print(session, "profile_us = %u = 0x%x\n", profile_us, profile_us);
        break;
    }
    case cli_rnr_quad_disable_l2tp_source_port:
    {
        bdmf_boolean disable_l2tp_source_port_check;
        ag_err = ag_drv_rnr_quad_disable_l2tp_source_port_get(parm[1].value.unumber, &disable_l2tp_source_port_check);
        bdmf_session_print(session, "disable_l2tp_source_port_check = %u = 0x%x\n", disable_l2tp_source_port_check, disable_l2tp_source_port_check);
        break;
    }
    case cli_rnr_quad_parser_snap_conf:
    {
        rnr_quad_parser_snap_conf parser_snap_conf;
        ag_err = ag_drv_rnr_quad_parser_snap_conf_get(parm[1].value.unumber, &parser_snap_conf);
        bdmf_session_print(session, "code = %u = 0x%x\n", parser_snap_conf.code, parser_snap_conf.code);
        bdmf_session_print(session, "en_rfc1042 = %u = 0x%x\n", parser_snap_conf.en_rfc1042, parser_snap_conf.en_rfc1042);
        bdmf_session_print(session, "en_8021q = %u = 0x%x\n", parser_snap_conf.en_8021q, parser_snap_conf.en_8021q);
        break;
    }
    case cli_rnr_quad_parser_ipv6_filter:
    {
        rnr_quad_parser_ipv6_filter parser_ipv6_filter;
        ag_err = ag_drv_rnr_quad_parser_ipv6_filter_get(parm[1].value.unumber, &parser_ipv6_filter);
        bdmf_session_print(session, "hop_by_hop_match = %u = 0x%x\n", parser_ipv6_filter.hop_by_hop_match, parser_ipv6_filter.hop_by_hop_match);
        bdmf_session_print(session, "routing_eh = %u = 0x%x\n", parser_ipv6_filter.routing_eh, parser_ipv6_filter.routing_eh);
        bdmf_session_print(session, "dest_opt_eh = %u = 0x%x\n", parser_ipv6_filter.dest_opt_eh, parser_ipv6_filter.dest_opt_eh);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_eng:
    {
        uint32_t cfg;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_eng_get(parm[1].value.unumber, &cfg);
        bdmf_session_print(session, "cfg = %u = 0x%x\n", cfg, cfg);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code:
    {
        uint16_t ppp_code_0;
        uint16_t ppp_code_1;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(parm[1].value.unumber, &ppp_code_0, &ppp_code_1);
        bdmf_session_print(session, "ppp_code_0 = %u = 0x%x\n", ppp_code_0, ppp_code_0);
        bdmf_session_print(session, "ppp_code_1 = %u = 0x%x\n", ppp_code_1, ppp_code_1);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_qtag_ethtype:
    {
        uint16_t ethtype_qtag_0;
        uint16_t ethtype_qtag_1;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(parm[1].value.unumber, &ethtype_qtag_0, &ethtype_qtag_1);
        bdmf_session_print(session, "ethtype_qtag_0 = %u = 0x%x\n", ethtype_qtag_0, ethtype_qtag_0);
        bdmf_session_print(session, "ethtype_qtag_1 = %u = 0x%x\n", ethtype_qtag_1, ethtype_qtag_1);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_user_ethtype_0_1:
    {
        uint16_t ethype_0;
        uint16_t ethype_1;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(parm[1].value.unumber, &ethype_0, &ethype_1);
        bdmf_session_print(session, "ethype_0 = %u = 0x%x\n", ethype_0, ethype_0);
        bdmf_session_print(session, "ethype_1 = %u = 0x%x\n", ethype_1, ethype_1);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_user_ethtype_2_3:
    {
        uint16_t ethype_2;
        uint16_t ethype_3;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(parm[1].value.unumber, &ethype_2, &ethype_3);
        bdmf_session_print(session, "ethype_2 = %u = 0x%x\n", ethype_2, ethype_2);
        bdmf_session_print(session, "ethype_3 = %u = 0x%x\n", ethype_3, ethype_3);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_user_ethtype_config:
    {
        rnr_quad_parser_core_configuration_user_ethtype_config parser_core_configuration_user_ethtype_config;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(parm[1].value.unumber, &parser_core_configuration_user_ethtype_config);
        bdmf_session_print(session, "ethtype_user_prot_0 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_0, parser_core_configuration_user_ethtype_config.ethtype_user_prot_0);
        bdmf_session_print(session, "ethtype_user_prot_1 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_1, parser_core_configuration_user_ethtype_config.ethtype_user_prot_1);
        bdmf_session_print(session, "ethtype_user_prot_2 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_2, parser_core_configuration_user_ethtype_config.ethtype_user_prot_2);
        bdmf_session_print(session, "ethtype_user_prot_3 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_prot_3, parser_core_configuration_user_ethtype_config.ethtype_user_prot_3);
        bdmf_session_print(session, "ethtype_user_en = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_en, parser_core_configuration_user_ethtype_config.ethtype_user_en);
        bdmf_session_print(session, "ethtype_user_offset_0 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_0, parser_core_configuration_user_ethtype_config.ethtype_user_offset_0);
        bdmf_session_print(session, "ethtype_user_offset_1 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_1, parser_core_configuration_user_ethtype_config.ethtype_user_offset_1);
        bdmf_session_print(session, "ethtype_user_offset_2 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_2, parser_core_configuration_user_ethtype_config.ethtype_user_offset_2);
        bdmf_session_print(session, "ethtype_user_offset_3 = %u = 0x%x\n", parser_core_configuration_user_ethtype_config.ethtype_user_offset_3, parser_core_configuration_user_ethtype_config.ethtype_user_offset_3);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1:
    {
        rnr_quad_da_filter_valid da_filter_valid;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(parm[1].value.unumber, &da_filter_valid);
        bdmf_session_print(session, "da_filt0_valid = %u = 0x%x\n", da_filter_valid.da_filt0_valid, da_filter_valid.da_filt0_valid);
        bdmf_session_print(session, "da_filt1_valid = %u = 0x%x\n", da_filter_valid.da_filt1_valid, da_filter_valid.da_filt1_valid);
        bdmf_session_print(session, "da_filt2_valid = %u = 0x%x\n", da_filter_valid.da_filt2_valid, da_filter_valid.da_filt2_valid);
        bdmf_session_print(session, "da_filt3_valid = %u = 0x%x\n", da_filter_valid.da_filt3_valid, da_filter_valid.da_filt3_valid);
        bdmf_session_print(session, "da_filt4_valid = %u = 0x%x\n", da_filter_valid.da_filt4_valid, da_filter_valid.da_filt4_valid);
        bdmf_session_print(session, "da_filt5_valid = %u = 0x%x\n", da_filter_valid.da_filt5_valid, da_filter_valid.da_filt5_valid);
        bdmf_session_print(session, "da_filt6_valid = %u = 0x%x\n", da_filter_valid.da_filt6_valid, da_filter_valid.da_filt6_valid);
        bdmf_session_print(session, "da_filt7_valid = %u = 0x%x\n", da_filter_valid.da_filt7_valid, da_filter_valid.da_filt7_valid);
        bdmf_session_print(session, "da_filt8_valid = %u = 0x%x\n", da_filter_valid.da_filt8_valid, da_filter_valid.da_filt8_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2:
    {
        rnr_quad_da_filter_valid da_filter_valid;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(parm[1].value.unumber, &da_filter_valid);
        bdmf_session_print(session, "da_filt0_valid = %u = 0x%x\n", da_filter_valid.da_filt0_valid, da_filter_valid.da_filt0_valid);
        bdmf_session_print(session, "da_filt1_valid = %u = 0x%x\n", da_filter_valid.da_filt1_valid, da_filter_valid.da_filt1_valid);
        bdmf_session_print(session, "da_filt2_valid = %u = 0x%x\n", da_filter_valid.da_filt2_valid, da_filter_valid.da_filt2_valid);
        bdmf_session_print(session, "da_filt3_valid = %u = 0x%x\n", da_filter_valid.da_filt3_valid, da_filter_valid.da_filt3_valid);
        bdmf_session_print(session, "da_filt4_valid = %u = 0x%x\n", da_filter_valid.da_filt4_valid, da_filter_valid.da_filt4_valid);
        bdmf_session_print(session, "da_filt5_valid = %u = 0x%x\n", da_filter_valid.da_filt5_valid, da_filter_valid.da_filt5_valid);
        bdmf_session_print(session, "da_filt6_valid = %u = 0x%x\n", da_filter_valid.da_filt6_valid, da_filter_valid.da_filt6_valid);
        bdmf_session_print(session, "da_filt7_valid = %u = 0x%x\n", da_filter_valid.da_filt7_valid, da_filter_valid.da_filt7_valid);
        bdmf_session_print(session, "da_filt8_valid = %u = 0x%x\n", da_filter_valid.da_filt8_valid, da_filter_valid.da_filt8_valid);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_gre_protocol_cfg:
    {
        uint16_t gre_protocol;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(parm[1].value.unumber, &gre_protocol);
        bdmf_session_print(session, "gre_protocol = %u = 0x%x\n", gre_protocol, gre_protocol);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_prop_tag_cfg:
    {
        rnr_quad_parser_core_configuration_prop_tag_cfg parser_core_configuration_prop_tag_cfg;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(parm[1].value.unumber, &parser_core_configuration_prop_tag_cfg);
        bdmf_session_print(session, "size_profile_0 = %u = 0x%x\n", parser_core_configuration_prop_tag_cfg.size_profile_0, parser_core_configuration_prop_tag_cfg.size_profile_0);
        bdmf_session_print(session, "size_profile_1 = %u = 0x%x\n", parser_core_configuration_prop_tag_cfg.size_profile_1, parser_core_configuration_prop_tag_cfg.size_profile_1);
        bdmf_session_print(session, "size_profile_2 = %u = 0x%x\n", parser_core_configuration_prop_tag_cfg.size_profile_2, parser_core_configuration_prop_tag_cfg.size_profile_2);
        bdmf_session_print(session, "pre_da_dprofile_0 = %u = 0x%x\n", parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0);
        bdmf_session_print(session, "pre_da_dprofile_1 = %u = 0x%x\n", parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1);
        bdmf_session_print(session, "pre_da_dprofile_2 = %u = 0x%x\n", parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_dos_attack:
    {
        uint16_t mask;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_dos_attack_get(parm[1].value.unumber, &mask);
        bdmf_session_print(session, "mask = %u = 0x%x\n", mask, mask);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_icmp_max_size:
    {
        uint16_t v4_size;
        uint16_t v6_size;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_get(parm[1].value.unumber, &v4_size, &v6_size);
        bdmf_session_print(session, "v4_size = %u = 0x%x\n", v4_size, v4_size);
        bdmf_session_print(session, "v6_size = %u = 0x%x\n", v6_size, v6_size);
        break;
    }
    case cli_rnr_quad_parser_core_configuration_key_cfg:
    {
        rnr_quad_parser_core_configuration_key_cfg parser_core_configuration_key_cfg;
        ag_err = ag_drv_rnr_quad_parser_core_configuration_key_cfg_get(parm[1].value.unumber, &parser_core_configuration_key_cfg);
        bdmf_session_print(session, "l2_tos_mask = %u = 0x%x\n", parser_core_configuration_key_cfg.l2_tos_mask, parser_core_configuration_key_cfg.l2_tos_mask);
        bdmf_session_print(session, "l3_tos_mask = %u = 0x%x\n", parser_core_configuration_key_cfg.l3_tos_mask, parser_core_configuration_key_cfg.l3_tos_mask);
        bdmf_session_print(session, "l2_exclude_smac = %u = 0x%x\n", parser_core_configuration_key_cfg.l2_exclude_smac, parser_core_configuration_key_cfg.l2_exclude_smac);
        bdmf_session_print(session, "tcp_pure_ack_mask = %u = 0x%x\n", parser_core_configuration_key_cfg.tcp_pure_ack_mask, parser_core_configuration_key_cfg.tcp_pure_ack_mask);
        bdmf_session_print(session, "incude_dei_in_vlans_crc = %u = 0x%x\n", parser_core_configuration_key_cfg.incude_dei_in_vlans_crc, parser_core_configuration_key_cfg.incude_dei_in_vlans_crc);
        bdmf_session_print(session, "key_size = %u = 0x%x\n", parser_core_configuration_key_cfg.key_size, parser_core_configuration_key_cfg.key_size);
        bdmf_session_print(session, "max_num_of_vlans_in_crc = %u = 0x%x\n", parser_core_configuration_key_cfg.max_num_of_vlans_in_crc, parser_core_configuration_key_cfg.max_num_of_vlans_in_crc);
        bdmf_session_print(session, "l3_tcp_pure_ack_mask = %u = 0x%x\n", parser_core_configuration_key_cfg.l3_tcp_pure_ack_mask, parser_core_configuration_key_cfg.l3_tcp_pure_ack_mask);
        bdmf_session_print(session, "rsrv = %u = 0x%x\n", parser_core_configuration_key_cfg.rsrv, parser_core_configuration_key_cfg.rsrv);
        break;
    }
    case cli_rnr_quad_general_config_dma_arb_cfg:
    {
        rnr_quad_general_config_dma_arb_cfg general_config_dma_arb_cfg;
        ag_err = ag_drv_rnr_quad_general_config_dma_arb_cfg_get(parm[1].value.unumber, &general_config_dma_arb_cfg);
        bdmf_session_print(session, "use_fifo_for_ddr_only = %u = 0x%x\n", general_config_dma_arb_cfg.use_fifo_for_ddr_only, general_config_dma_arb_cfg.use_fifo_for_ddr_only);
        bdmf_session_print(session, "token_arbiter_is_rr = %u = 0x%x\n", general_config_dma_arb_cfg.token_arbiter_is_rr, general_config_dma_arb_cfg.token_arbiter_is_rr);
        bdmf_session_print(session, "chicken_no_flowctrl = %u = 0x%x\n", general_config_dma_arb_cfg.chicken_no_flowctrl, general_config_dma_arb_cfg.chicken_no_flowctrl);
        bdmf_session_print(session, "flow_ctrl_clear_token = %u = 0x%x\n", general_config_dma_arb_cfg.flow_ctrl_clear_token, general_config_dma_arb_cfg.flow_ctrl_clear_token);
        bdmf_session_print(session, "ddr_congest_threshold = %u = 0x%x\n", general_config_dma_arb_cfg.ddr_congest_threshold, general_config_dma_arb_cfg.ddr_congest_threshold);
        bdmf_session_print(session, "psram_congest_threshold = %u = 0x%x\n", general_config_dma_arb_cfg.psram_congest_threshold, general_config_dma_arb_cfg.psram_congest_threshold);
        bdmf_session_print(session, "enable_reply_threshold = %u = 0x%x\n", general_config_dma_arb_cfg.enable_reply_threshold, general_config_dma_arb_cfg.enable_reply_threshold);
        bdmf_session_print(session, "ddr_reply_threshold = %u = 0x%x\n", general_config_dma_arb_cfg.ddr_reply_threshold, general_config_dma_arb_cfg.ddr_reply_threshold);
        bdmf_session_print(session, "psram_reply_threshold = %u = 0x%x\n", general_config_dma_arb_cfg.psram_reply_threshold, general_config_dma_arb_cfg.psram_reply_threshold);
        break;
    }
    case cli_rnr_quad_general_config_psram0_base:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram0_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram1_base:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram1_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram2_base:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram2_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram3_base:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram3_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr0_base:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_ddr0_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr1_base:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_ddr1_base_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram0_mask:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram0_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram1_mask:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram1_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram2_mask:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram2_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_psram3_mask:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_psram3_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr0_mask:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_ddr0_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_ddr1_mask:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_general_config_ddr1_mask_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_general_config_profiling_config:
    {
        rnr_quad_general_config_profiling_config general_config_profiling_config;
        ag_err = ag_drv_rnr_quad_general_config_profiling_config_get(parm[1].value.unumber, &general_config_profiling_config);
        bdmf_session_print(session, "counter_lsb_sel = %u = 0x%x\n", general_config_profiling_config.counter_lsb_sel, general_config_profiling_config.counter_lsb_sel);
        bdmf_session_print(session, "enable_trace_core_0 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_0, general_config_profiling_config.enable_trace_core_0);
        bdmf_session_print(session, "enable_trace_core_1 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_1, general_config_profiling_config.enable_trace_core_1);
        bdmf_session_print(session, "enable_trace_core_2 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_2, general_config_profiling_config.enable_trace_core_2);
        bdmf_session_print(session, "enable_trace_core_3 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_3, general_config_profiling_config.enable_trace_core_3);
        bdmf_session_print(session, "enable_trace_core_4 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_4, general_config_profiling_config.enable_trace_core_4);
        bdmf_session_print(session, "enable_trace_core_5 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_5, general_config_profiling_config.enable_trace_core_5);
        bdmf_session_print(session, "enable_trace_core_6 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_6, general_config_profiling_config.enable_trace_core_6);
        bdmf_session_print(session, "enable_trace_core_7 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_7, general_config_profiling_config.enable_trace_core_7);
        bdmf_session_print(session, "enable_trace_core_8 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_8, general_config_profiling_config.enable_trace_core_8);
        bdmf_session_print(session, "enable_trace_core_9 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_9, general_config_profiling_config.enable_trace_core_9);
        bdmf_session_print(session, "enable_trace_core_10 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_10, general_config_profiling_config.enable_trace_core_10);
        bdmf_session_print(session, "enable_trace_core_11 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_11, general_config_profiling_config.enable_trace_core_11);
        bdmf_session_print(session, "enable_trace_core_12 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_12, general_config_profiling_config.enable_trace_core_12);
        bdmf_session_print(session, "enable_trace_core_13 = %u = 0x%x\n", general_config_profiling_config.enable_trace_core_13, general_config_profiling_config.enable_trace_core_13);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_0_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_1_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_2_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_3_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_4_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_5_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_6_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_7_cfg:
    {
        uint16_t addr;
        uint8_t thread;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(parm[1].value.unumber, &addr, &thread);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "thread = %u = 0x%x\n", thread, thread);
        break;
    }
    case cli_rnr_quad_general_config_bkpt_gen_cfg:
    {
        uint16_t handler_addr;
        uint16_t update_pc_value;
        ag_err = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(parm[1].value.unumber, &handler_addr, &update_pc_value);
        bdmf_session_print(session, "handler_addr = %u = 0x%x\n", handler_addr, handler_addr);
        bdmf_session_print(session, "update_pc_value = %u = 0x%x\n", update_pc_value, update_pc_value);
        break;
    }
    case cli_rnr_quad_general_config_powersave_config:
    {
        rnr_quad_general_config_powersave_config general_config_powersave_config;
        ag_err = ag_drv_rnr_quad_general_config_powersave_config_get(parm[1].value.unumber, &general_config_powersave_config);
        bdmf_session_print(session, "time_counter = %u = 0x%x\n", general_config_powersave_config.time_counter, general_config_powersave_config.time_counter);
        bdmf_session_print(session, "enable_powersave_core_0 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_0, general_config_powersave_config.enable_powersave_core_0);
        bdmf_session_print(session, "enable_powersave_core_1 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_1, general_config_powersave_config.enable_powersave_core_1);
        bdmf_session_print(session, "enable_powersave_core_2 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_2, general_config_powersave_config.enable_powersave_core_2);
        bdmf_session_print(session, "enable_powersave_core_3 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_3, general_config_powersave_config.enable_powersave_core_3);
        bdmf_session_print(session, "enable_powersave_core_4 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_4, general_config_powersave_config.enable_powersave_core_4);
        bdmf_session_print(session, "enable_powersave_core_5 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_5, general_config_powersave_config.enable_powersave_core_5);
        bdmf_session_print(session, "enable_powersave_core_6 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_6, general_config_powersave_config.enable_powersave_core_6);
        bdmf_session_print(session, "enable_powersave_core_7 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_7, general_config_powersave_config.enable_powersave_core_7);
        bdmf_session_print(session, "enable_powersave_core_8 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_8, general_config_powersave_config.enable_powersave_core_8);
        bdmf_session_print(session, "enable_powersave_core_9 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_9, general_config_powersave_config.enable_powersave_core_9);
        bdmf_session_print(session, "enable_powersave_core_10 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_10, general_config_powersave_config.enable_powersave_core_10);
        bdmf_session_print(session, "enable_powersave_core_11 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_11, general_config_powersave_config.enable_powersave_core_11);
        bdmf_session_print(session, "enable_powersave_core_12 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_12, general_config_powersave_config.enable_powersave_core_12);
        bdmf_session_print(session, "enable_powersave_core_13 = %u = 0x%x\n", general_config_powersave_config.enable_powersave_core_13, general_config_powersave_config.enable_powersave_core_13);
        bdmf_session_print(session, "enable_cpu_if_clk_gating = %u = 0x%x\n", general_config_powersave_config.enable_cpu_if_clk_gating, general_config_powersave_config.enable_cpu_if_clk_gating);
        bdmf_session_print(session, "enable_common_reg_clk_gating = %u = 0x%x\n", general_config_powersave_config.enable_common_reg_clk_gating, general_config_powersave_config.enable_common_reg_clk_gating);
        bdmf_session_print(session, "enable_ec_blocks_clk_gating = %u = 0x%x\n", general_config_powersave_config.enable_ec_blocks_clk_gating, general_config_powersave_config.enable_ec_blocks_clk_gating);
        break;
    }
    case cli_rnr_quad_general_config_powersave_status:
    {
        rnr_quad_general_config_powersave_status general_config_powersave_status;
        ag_err = ag_drv_rnr_quad_general_config_powersave_status_get(parm[1].value.unumber, &general_config_powersave_status);
        bdmf_session_print(session, "acc_status_0 = %u = 0x%x\n", general_config_powersave_status.acc_status_0, general_config_powersave_status.acc_status_0);
        bdmf_session_print(session, "acc_status_1 = %u = 0x%x\n", general_config_powersave_status.acc_status_1, general_config_powersave_status.acc_status_1);
        bdmf_session_print(session, "acc_status_2 = %u = 0x%x\n", general_config_powersave_status.acc_status_2, general_config_powersave_status.acc_status_2);
        bdmf_session_print(session, "acc_status_3 = %u = 0x%x\n", general_config_powersave_status.acc_status_3, general_config_powersave_status.acc_status_3);
        bdmf_session_print(session, "acc_status_4 = %u = 0x%x\n", general_config_powersave_status.acc_status_4, general_config_powersave_status.acc_status_4);
        bdmf_session_print(session, "acc_status_5 = %u = 0x%x\n", general_config_powersave_status.acc_status_5, general_config_powersave_status.acc_status_5);
        bdmf_session_print(session, "acc_status_6 = %u = 0x%x\n", general_config_powersave_status.acc_status_6, general_config_powersave_status.acc_status_6);
        bdmf_session_print(session, "acc_status_7 = %u = 0x%x\n", general_config_powersave_status.acc_status_7, general_config_powersave_status.acc_status_7);
        bdmf_session_print(session, "acc_status_8 = %u = 0x%x\n", general_config_powersave_status.acc_status_8, general_config_powersave_status.acc_status_8);
        bdmf_session_print(session, "acc_status_9 = %u = 0x%x\n", general_config_powersave_status.acc_status_9, general_config_powersave_status.acc_status_9);
        bdmf_session_print(session, "acc_status_10 = %u = 0x%x\n", general_config_powersave_status.acc_status_10, general_config_powersave_status.acc_status_10);
        bdmf_session_print(session, "acc_status_11 = %u = 0x%x\n", general_config_powersave_status.acc_status_11, general_config_powersave_status.acc_status_11);
        bdmf_session_print(session, "acc_status_12 = %u = 0x%x\n", general_config_powersave_status.acc_status_12, general_config_powersave_status.acc_status_12);
        bdmf_session_print(session, "acc_status_13 = %u = 0x%x\n", general_config_powersave_status.acc_status_13, general_config_powersave_status.acc_status_13);
        bdmf_session_print(session, "core_status_0 = %u = 0x%x\n", general_config_powersave_status.core_status_0, general_config_powersave_status.core_status_0);
        bdmf_session_print(session, "core_status_1 = %u = 0x%x\n", general_config_powersave_status.core_status_1, general_config_powersave_status.core_status_1);
        bdmf_session_print(session, "core_status_2 = %u = 0x%x\n", general_config_powersave_status.core_status_2, general_config_powersave_status.core_status_2);
        bdmf_session_print(session, "core_status_3 = %u = 0x%x\n", general_config_powersave_status.core_status_3, general_config_powersave_status.core_status_3);
        bdmf_session_print(session, "core_status_4 = %u = 0x%x\n", general_config_powersave_status.core_status_4, general_config_powersave_status.core_status_4);
        bdmf_session_print(session, "core_status_5 = %u = 0x%x\n", general_config_powersave_status.core_status_5, general_config_powersave_status.core_status_5);
        bdmf_session_print(session, "core_status_6 = %u = 0x%x\n", general_config_powersave_status.core_status_6, general_config_powersave_status.core_status_6);
        bdmf_session_print(session, "core_status_7 = %u = 0x%x\n", general_config_powersave_status.core_status_7, general_config_powersave_status.core_status_7);
        bdmf_session_print(session, "core_status_8 = %u = 0x%x\n", general_config_powersave_status.core_status_8, general_config_powersave_status.core_status_8);
        bdmf_session_print(session, "core_status_9 = %u = 0x%x\n", general_config_powersave_status.core_status_9, general_config_powersave_status.core_status_9);
        bdmf_session_print(session, "core_status_10 = %u = 0x%x\n", general_config_powersave_status.core_status_10, general_config_powersave_status.core_status_10);
        bdmf_session_print(session, "core_status_11 = %u = 0x%x\n", general_config_powersave_status.core_status_11, general_config_powersave_status.core_status_11);
        bdmf_session_print(session, "core_status_12 = %u = 0x%x\n", general_config_powersave_status.core_status_12, general_config_powersave_status.core_status_12);
        bdmf_session_print(session, "core_status_13 = %u = 0x%x\n", general_config_powersave_status.core_status_13, general_config_powersave_status.core_status_13);
        break;
    }
    case cli_rnr_quad_general_config_data_bkpt_0_cfg:
    {
        uint16_t data_addr_start;
        uint16_t data_addr_stop;
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_get(parm[1].value.unumber, &data_addr_start, &data_addr_stop);
        bdmf_session_print(session, "data_addr_start = %u = 0x%x\n", data_addr_start, data_addr_start);
        bdmf_session_print(session, "data_addr_stop = %u = 0x%x\n", data_addr_stop, data_addr_stop);
        break;
    }
    case cli_rnr_quad_general_config_data_bkpt_1_cfg:
    {
        uint16_t data_addr_start;
        uint16_t data_addr_stop;
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_get(parm[1].value.unumber, &data_addr_start, &data_addr_stop);
        bdmf_session_print(session, "data_addr_start = %u = 0x%x\n", data_addr_start, data_addr_start);
        bdmf_session_print(session, "data_addr_stop = %u = 0x%x\n", data_addr_stop, data_addr_stop);
        break;
    }
    case cli_rnr_quad_general_config_data_bkpt_2_cfg:
    {
        uint16_t data_addr_start;
        uint16_t data_addr_stop;
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_get(parm[1].value.unumber, &data_addr_start, &data_addr_stop);
        bdmf_session_print(session, "data_addr_start = %u = 0x%x\n", data_addr_start, data_addr_start);
        bdmf_session_print(session, "data_addr_stop = %u = 0x%x\n", data_addr_stop, data_addr_stop);
        break;
    }
    case cli_rnr_quad_general_config_data_bkpt_3_cfg:
    {
        uint16_t data_addr_start;
        uint16_t data_addr_stop;
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_get(parm[1].value.unumber, &data_addr_start, &data_addr_stop);
        bdmf_session_print(session, "data_addr_start = %u = 0x%x\n", data_addr_start, data_addr_start);
        bdmf_session_print(session, "data_addr_stop = %u = 0x%x\n", data_addr_stop, data_addr_stop);
        break;
    }
    case cli_rnr_quad_general_config_data_bkpt_common_cfg:
    {
        uint8_t thread_0;
        uint8_t thread_1;
        uint8_t thread_2;
        uint8_t thread_3;
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_get(parm[1].value.unumber, &thread_0, &thread_1, &thread_2, &thread_3);
        bdmf_session_print(session, "thread_0 = %u = 0x%x\n", thread_0, thread_0);
        bdmf_session_print(session, "thread_1 = %u = 0x%x\n", thread_1, thread_1);
        bdmf_session_print(session, "thread_2 = %u = 0x%x\n", thread_2, thread_2);
        bdmf_session_print(session, "thread_3 = %u = 0x%x\n", thread_3, thread_3);
        break;
    }
    case cli_rnr_quad_general_config_ubus_counter_control:
    {
        bdmf_boolean enable_statistics;
        bdmf_boolean sw_reset;
        uint8_t dest_pid;
        uint8_t master_select;
        ag_err = ag_drv_rnr_quad_general_config_ubus_counter_control_get(parm[1].value.unumber, &enable_statistics, &sw_reset, &dest_pid, &master_select);
        bdmf_session_print(session, "enable_statistics = %u = 0x%x\n", enable_statistics, enable_statistics);
        bdmf_session_print(session, "sw_reset = %u = 0x%x\n", sw_reset, sw_reset);
        bdmf_session_print(session, "dest_pid = %u = 0x%x\n", dest_pid, dest_pid);
        bdmf_session_print(session, "master_select = %u = 0x%x\n", master_select, master_select);
        break;
    }
    case cli_rnr_quad_general_config_ubus_down_counter:
    {
        uint32_t downcnt_value;
        ag_err = ag_drv_rnr_quad_general_config_ubus_down_counter_get(parm[1].value.unumber, &downcnt_value);
        bdmf_session_print(session, "downcnt_value = %u = 0x%x\n", downcnt_value, downcnt_value);
        break;
    }
    case cli_rnr_quad_general_config_all_xfers_cnt:
    {
        uint32_t counter_value;
        ag_err = ag_drv_rnr_quad_general_config_all_xfers_cnt_get(parm[1].value.unumber, &counter_value);
        bdmf_session_print(session, "counter_value = %u = 0x%x\n", counter_value, counter_value);
        break;
    }
    case cli_rnr_quad_general_config_read_xfers_cnt:
    {
        uint32_t counter_value;
        ag_err = ag_drv_rnr_quad_general_config_read_xfers_cnt_get(parm[1].value.unumber, &counter_value);
        bdmf_session_print(session, "counter_value = %u = 0x%x\n", counter_value, counter_value);
        break;
    }
    case cli_rnr_quad_general_config_read_data_cnt:
    {
        uint32_t counter_value;
        ag_err = ag_drv_rnr_quad_general_config_read_data_cnt_get(parm[1].value.unumber, &counter_value);
        bdmf_session_print(session, "counter_value = %u = 0x%x\n", counter_value, counter_value);
        break;
    }
    case cli_rnr_quad_general_config_write_data_cnt:
    {
        uint32_t counter_value;
        ag_err = ag_drv_rnr_quad_general_config_write_data_cnt_get(parm[1].value.unumber, &counter_value);
        bdmf_session_print(session, "counter_value = %u = 0x%x\n", counter_value, counter_value);
        break;
    }
    case cli_rnr_quad_general_config_misc_cfg:
    {
        uint8_t ddr_pid;
        ag_err = ag_drv_rnr_quad_general_config_misc_cfg_get(parm[1].value.unumber, &ddr_pid);
        bdmf_session_print(session, "ddr_pid = %u = 0x%x\n", ddr_pid, ddr_pid);
        break;
    }
    case cli_rnr_quad_general_config_aqm_control:
    {
        bdmf_boolean enable_counter;
        bdmf_boolean enable_random;
        ag_err = ag_drv_rnr_quad_general_config_aqm_control_get(parm[1].value.unumber, &enable_counter, &enable_random);
        bdmf_session_print(session, "enable_counter = %u = 0x%x\n", enable_counter, enable_counter);
        bdmf_session_print(session, "enable_random = %u = 0x%x\n", enable_random, enable_random);
        break;
    }
    case cli_rnr_quad_general_config_aqm_randm_value:
    {
        uint32_t random_value;
        ag_err = ag_drv_rnr_quad_general_config_aqm_randm_value_get(parm[1].value.unumber, &random_value);
        bdmf_session_print(session, "random_value = %u = 0x%x\n", random_value, random_value);
        break;
    }
    case cli_rnr_quad_general_config_aqm_random_seed:
    {
        uint32_t random_seed;
        ag_err = ag_drv_rnr_quad_general_config_aqm_random_seed_get(parm[1].value.unumber, &random_seed);
        bdmf_session_print(session, "random_seed = %u = 0x%x\n", random_seed, random_seed);
        break;
    }
    case cli_rnr_quad_general_config_multi_psel_cfg:
    {
        uint16_t multi_psel_master_sel;
        uint16_t multi_psel_mask;
        ag_err = ag_drv_rnr_quad_general_config_multi_psel_cfg_get(parm[1].value.unumber, &multi_psel_master_sel, &multi_psel_mask);
        bdmf_session_print(session, "multi_psel_master_sel = %u = 0x%x\n", multi_psel_master_sel, multi_psel_master_sel);
        bdmf_session_print(session, "multi_psel_mask = %u = 0x%x\n", multi_psel_mask, multi_psel_mask);
        break;
    }
    case cli_rnr_quad_debug_fifo_config:
    {
        rnr_quad_debug_fifo_config debug_fifo_config;
        ag_err = ag_drv_rnr_quad_debug_fifo_config_get(parm[1].value.unumber, &debug_fifo_config);
        bdmf_session_print(session, "psram_hdr_sw_rst_0 = %u = 0x%x\n", debug_fifo_config.psram_hdr_sw_rst_0, debug_fifo_config.psram_hdr_sw_rst_0);
        bdmf_session_print(session, "psram_data_sw_rst_0 = %u = 0x%x\n", debug_fifo_config.psram_data_sw_rst_0, debug_fifo_config.psram_data_sw_rst_0);
        bdmf_session_print(session, "ddr_hdr_sw_rst_0 = %u = 0x%x\n", debug_fifo_config.ddr_hdr_sw_rst_0, debug_fifo_config.ddr_hdr_sw_rst_0);
        bdmf_session_print(session, "select_fifos_for_debug = %u = 0x%x\n", debug_fifo_config.select_fifos_for_debug, debug_fifo_config.select_fifos_for_debug);
        bdmf_session_print(session, "psram_hdr_sw_rst_1 = %u = 0x%x\n", debug_fifo_config.psram_hdr_sw_rst_1, debug_fifo_config.psram_hdr_sw_rst_1);
        bdmf_session_print(session, "psram_data_sw_rst_1 = %u = 0x%x\n", debug_fifo_config.psram_data_sw_rst_1, debug_fifo_config.psram_data_sw_rst_1);
        bdmf_session_print(session, "ddr_hdr_sw_rst_1 = %u = 0x%x\n", debug_fifo_config.ddr_hdr_sw_rst_1, debug_fifo_config.ddr_hdr_sw_rst_1);
        bdmf_session_print(session, "psram_hdr_sw_rd_addr = %u = 0x%x\n", debug_fifo_config.psram_hdr_sw_rd_addr, debug_fifo_config.psram_hdr_sw_rd_addr);
        bdmf_session_print(session, "psram_data_sw_rd_addr = %u = 0x%x\n", debug_fifo_config.psram_data_sw_rd_addr, debug_fifo_config.psram_data_sw_rd_addr);
        bdmf_session_print(session, "ddr_hdr_sw_rd_addr = %u = 0x%x\n", debug_fifo_config.ddr_hdr_sw_rd_addr, debug_fifo_config.ddr_hdr_sw_rd_addr);
        break;
    }
    case cli_rnr_quad_debug_psram_hdr_fifo_status:
    {
        rnr_quad_debug_psram_hdr_fifo_status debug_psram_hdr_fifo_status;
        ag_err = ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(parm[1].value.unumber, &debug_psram_hdr_fifo_status);
        bdmf_session_print(session, "full = %u = 0x%x\n", debug_psram_hdr_fifo_status.full, debug_psram_hdr_fifo_status.full);
        bdmf_session_print(session, "empty = %u = 0x%x\n", debug_psram_hdr_fifo_status.empty, debug_psram_hdr_fifo_status.empty);
        bdmf_session_print(session, "push_wr_cntr = %u = 0x%x\n", debug_psram_hdr_fifo_status.push_wr_cntr, debug_psram_hdr_fifo_status.push_wr_cntr);
        bdmf_session_print(session, "pop_rd_cntr = %u = 0x%x\n", debug_psram_hdr_fifo_status.pop_rd_cntr, debug_psram_hdr_fifo_status.pop_rd_cntr);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", debug_psram_hdr_fifo_status.used_words, debug_psram_hdr_fifo_status.used_words);
        break;
    }
    case cli_rnr_quad_debug_psram_data_fifo_status:
    {
        rnr_quad_debug_psram_data_fifo_status debug_psram_data_fifo_status;
        ag_err = ag_drv_rnr_quad_debug_psram_data_fifo_status_get(parm[1].value.unumber, &debug_psram_data_fifo_status);
        bdmf_session_print(session, "full = %u = 0x%x\n", debug_psram_data_fifo_status.full, debug_psram_data_fifo_status.full);
        bdmf_session_print(session, "empty = %u = 0x%x\n", debug_psram_data_fifo_status.empty, debug_psram_data_fifo_status.empty);
        bdmf_session_print(session, "almost_full = %u = 0x%x\n", debug_psram_data_fifo_status.almost_full, debug_psram_data_fifo_status.almost_full);
        bdmf_session_print(session, "push_wr_cntr = %u = 0x%x\n", debug_psram_data_fifo_status.push_wr_cntr, debug_psram_data_fifo_status.push_wr_cntr);
        bdmf_session_print(session, "pop_rd_cntr = %u = 0x%x\n", debug_psram_data_fifo_status.pop_rd_cntr, debug_psram_data_fifo_status.pop_rd_cntr);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", debug_psram_data_fifo_status.used_words, debug_psram_data_fifo_status.used_words);
        break;
    }
    case cli_rnr_quad_debug_ddr_hdr_fifo_status:
    {
        rnr_quad_debug_ddr_hdr_fifo_status debug_ddr_hdr_fifo_status;
        ag_err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(parm[1].value.unumber, &debug_ddr_hdr_fifo_status);
        bdmf_session_print(session, "full = %u = 0x%x\n", debug_ddr_hdr_fifo_status.full, debug_ddr_hdr_fifo_status.full);
        bdmf_session_print(session, "empty = %u = 0x%x\n", debug_ddr_hdr_fifo_status.empty, debug_ddr_hdr_fifo_status.empty);
        bdmf_session_print(session, "push_wr_cntr = %u = 0x%x\n", debug_ddr_hdr_fifo_status.push_wr_cntr, debug_ddr_hdr_fifo_status.push_wr_cntr);
        bdmf_session_print(session, "pop_rd_cntr = %u = 0x%x\n", debug_ddr_hdr_fifo_status.pop_rd_cntr, debug_ddr_hdr_fifo_status.pop_rd_cntr);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", debug_ddr_hdr_fifo_status.used_words, debug_ddr_hdr_fifo_status.used_words);
        break;
    }
    case cli_rnr_quad_debug_ddr_data_fifo_status:
    {
        rnr_quad_debug_ddr_data_fifo_status debug_ddr_data_fifo_status;
        ag_err = ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(parm[1].value.unumber, &debug_ddr_data_fifo_status);
        bdmf_session_print(session, "full = %u = 0x%x\n", debug_ddr_data_fifo_status.full, debug_ddr_data_fifo_status.full);
        bdmf_session_print(session, "empty = %u = 0x%x\n", debug_ddr_data_fifo_status.empty, debug_ddr_data_fifo_status.empty);
        bdmf_session_print(session, "almost_full = %u = 0x%x\n", debug_ddr_data_fifo_status.almost_full, debug_ddr_data_fifo_status.almost_full);
        bdmf_session_print(session, "wr_cntr = %u = 0x%x\n", debug_ddr_data_fifo_status.wr_cntr, debug_ddr_data_fifo_status.wr_cntr);
        bdmf_session_print(session, "rd_cntr = %u = 0x%x\n", debug_ddr_data_fifo_status.rd_cntr, debug_ddr_data_fifo_status.rd_cntr);
        break;
    }
    case cli_rnr_quad_debug_ddr_data_fifo_status2:
    {
        uint8_t read_addr;
        uint16_t used_words;
        ag_err = ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(parm[1].value.unumber, &read_addr, &used_words);
        bdmf_session_print(session, "read_addr = %u = 0x%x\n", read_addr, read_addr);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        break;
    }
    case cli_rnr_quad_debug_psram_hdr_fifo_data1:
    {
        uint32_t data;
        ag_err = ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_psram_hdr_fifo_data2:
    {
        uint32_t data;
        ag_err = ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_psram_data_fifo_data1:
    {
        uint32_t data;
        ag_err = ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_psram_data_fifo_data2:
    {
        uint32_t data;
        ag_err = ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_ddr_hdr_fifo_data1:
    {
        uint32_t data;
        ag_err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_rnr_quad_debug_ddr_hdr_fifo_data2:
    {
        uint32_t data;
        ag_err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_rnr_quad_ext_flowctrl_config_token_val:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_ext_flowctrl_config2_token_val_2:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2:
    {
        uint32_t val;
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_rnr_quad_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t quad_idx = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint16_t vid_0 = gtmv(m, 12);
        bdmf_boolean vid_0_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid0_set(%u %u %u)\n", quad_idx,
            vid_0, vid_0_en);
        ag_err = ag_drv_rnr_quad_parser_vid0_set(quad_idx, vid_0, vid_0_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid0_get(quad_idx, &vid_0, &vid_0_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid0_get(%u %u %u)\n", quad_idx,
                vid_0, vid_0_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_0 != gtmv(m, 12) || vid_0_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_1 = gtmv(m, 12);
        bdmf_boolean vid_1_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid1_set(%u %u %u)\n", quad_idx,
            vid_1, vid_1_en);
        ag_err = ag_drv_rnr_quad_parser_vid1_set(quad_idx, vid_1, vid_1_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid1_get(quad_idx, &vid_1, &vid_1_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid1_get(%u %u %u)\n", quad_idx,
                vid_1, vid_1_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_1 != gtmv(m, 12) || vid_1_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_2 = gtmv(m, 12);
        bdmf_boolean vid_2_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid2_set(%u %u %u)\n", quad_idx,
            vid_2, vid_2_en);
        ag_err = ag_drv_rnr_quad_parser_vid2_set(quad_idx, vid_2, vid_2_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid2_get(quad_idx, &vid_2, &vid_2_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid2_get(%u %u %u)\n", quad_idx,
                vid_2, vid_2_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_2 != gtmv(m, 12) || vid_2_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_3 = gtmv(m, 12);
        bdmf_boolean vid_3_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid3_set(%u %u %u)\n", quad_idx,
            vid_3, vid_3_en);
        ag_err = ag_drv_rnr_quad_parser_vid3_set(quad_idx, vid_3, vid_3_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid3_get(quad_idx, &vid_3, &vid_3_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid3_get(%u %u %u)\n", quad_idx,
                vid_3, vid_3_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_3 != gtmv(m, 12) || vid_3_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_4 = gtmv(m, 12);
        bdmf_boolean vid_4_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid4_set(%u %u %u)\n", quad_idx,
            vid_4, vid_4_en);
        ag_err = ag_drv_rnr_quad_parser_vid4_set(quad_idx, vid_4, vid_4_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid4_get(quad_idx, &vid_4, &vid_4_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid4_get(%u %u %u)\n", quad_idx,
                vid_4, vid_4_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_4 != gtmv(m, 12) || vid_4_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_5 = gtmv(m, 12);
        bdmf_boolean vid_5_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid5_set(%u %u %u)\n", quad_idx,
            vid_5, vid_5_en);
        ag_err = ag_drv_rnr_quad_parser_vid5_set(quad_idx, vid_5, vid_5_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid5_get(quad_idx, &vid_5, &vid_5_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid5_get(%u %u %u)\n", quad_idx,
                vid_5, vid_5_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_5 != gtmv(m, 12) || vid_5_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_6 = gtmv(m, 12);
        bdmf_boolean vid_6_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid6_set(%u %u %u)\n", quad_idx,
            vid_6, vid_6_en);
        ag_err = ag_drv_rnr_quad_parser_vid6_set(quad_idx, vid_6, vid_6_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid6_get(quad_idx, &vid_6, &vid_6_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid6_get(%u %u %u)\n", quad_idx,
                vid_6, vid_6_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_6 != gtmv(m, 12) || vid_6_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t vid_7 = gtmv(m, 12);
        bdmf_boolean vid_7_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid7_set(%u %u %u)\n", quad_idx,
            vid_7, vid_7_en);
        ag_err = ag_drv_rnr_quad_parser_vid7_set(quad_idx, vid_7, vid_7_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_vid7_get(quad_idx, &vid_7, &vid_7_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_vid7_get(%u %u %u)\n", quad_idx,
                vid_7, vid_7_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (vid_7 != gtmv(m, 12) || vid_7_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_ip0 parser_ip0 = {.ip_address = gtmv(m, 32), .ip_address_mask = gtmv(m, 32), .ip_filter0_dip_en = gtmv(m, 1), .ip_filter0_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip0_set(%u %u %u %u %u)\n", quad_idx,
            parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        ag_err = ag_drv_rnr_quad_parser_ip0_set(quad_idx, &parser_ip0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ip0_get(quad_idx, &parser_ip0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip0_get(%u %u %u %u %u)\n", quad_idx,
                parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_ip0.ip_address != gtmv(m, 32) || parser_ip0.ip_address_mask != gtmv(m, 32) || parser_ip0.ip_filter0_dip_en != gtmv(m, 1) || parser_ip0.ip_filter0_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_ip0 parser_ip0 = {.ip_address = gtmv(m, 32), .ip_address_mask = gtmv(m, 32), .ip_filter0_dip_en = gtmv(m, 1), .ip_filter0_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip1_set(%u %u %u %u %u)\n", quad_idx,
            parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        ag_err = ag_drv_rnr_quad_parser_ip1_set(quad_idx, &parser_ip0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ip1_get(quad_idx, &parser_ip0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip1_get(%u %u %u %u %u)\n", quad_idx,
                parser_ip0.ip_address, parser_ip0.ip_address_mask, parser_ip0.ip_filter0_dip_en, parser_ip0.ip_filter0_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_ip0.ip_address != gtmv(m, 32) || parser_ip0.ip_address_mask != gtmv(m, 32) || parser_ip0.ip_filter0_dip_en != gtmv(m, 1) || parser_ip0.ip_filter0_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0 = {.hard_nest_profile = gtmv(m, 12)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(%u %u)\n", quad_idx,
            parser_hardcoded_ethtype_prof0.hard_nest_profile);
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(quad_idx, &parser_hardcoded_ethtype_prof0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(quad_idx, &parser_hardcoded_ethtype_prof0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(%u %u)\n", quad_idx,
                parser_hardcoded_ethtype_prof0.hard_nest_profile);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_hardcoded_ethtype_prof0.hard_nest_profile != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0 = {.hard_nest_profile = gtmv(m, 12)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(%u %u)\n", quad_idx,
            parser_hardcoded_ethtype_prof0.hard_nest_profile);
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(quad_idx, &parser_hardcoded_ethtype_prof0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(quad_idx, &parser_hardcoded_ethtype_prof0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(%u %u)\n", quad_idx,
                parser_hardcoded_ethtype_prof0.hard_nest_profile);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_hardcoded_ethtype_prof0.hard_nest_profile != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_hardcoded_ethtype_prof0 parser_hardcoded_ethtype_prof0 = {.hard_nest_profile = gtmv(m, 12)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(%u %u)\n", quad_idx,
            parser_hardcoded_ethtype_prof0.hard_nest_profile);
        ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(quad_idx, &parser_hardcoded_ethtype_prof0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(quad_idx, &parser_hardcoded_ethtype_prof0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(%u %u)\n", quad_idx,
                parser_hardcoded_ethtype_prof0.hard_nest_profile);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_hardcoded_ethtype_prof0.hard_nest_profile != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t qtag_nest_0_profile_0 = gtmv(m, 3);
        uint8_t qtag_nest_1_profile_0 = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof0_set(%u %u %u)\n", quad_idx,
            qtag_nest_0_profile_0, qtag_nest_1_profile_0);
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof0_set(quad_idx, qtag_nest_0_profile_0, qtag_nest_1_profile_0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof0_get(quad_idx, &qtag_nest_0_profile_0, &qtag_nest_1_profile_0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof0_get(%u %u %u)\n", quad_idx,
                qtag_nest_0_profile_0, qtag_nest_1_profile_0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (qtag_nest_0_profile_0 != gtmv(m, 3) || qtag_nest_1_profile_0 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t qtag_nest_0_profile_1 = gtmv(m, 3);
        uint8_t qtag_nest_1_profile_1 = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof1_set(%u %u %u)\n", quad_idx,
            qtag_nest_0_profile_1, qtag_nest_1_profile_1);
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof1_set(quad_idx, qtag_nest_0_profile_1, qtag_nest_1_profile_1);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof1_get(quad_idx, &qtag_nest_0_profile_1, &qtag_nest_1_profile_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof1_get(%u %u %u)\n", quad_idx,
                qtag_nest_0_profile_1, qtag_nest_1_profile_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (qtag_nest_0_profile_1 != gtmv(m, 3) || qtag_nest_1_profile_1 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t qtag_nest_0_profile_2 = gtmv(m, 3);
        uint8_t qtag_nest_1_profile_2 = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof2_set(%u %u %u)\n", quad_idx,
            qtag_nest_0_profile_2, qtag_nest_1_profile_2);
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof2_set(quad_idx, qtag_nest_0_profile_2, qtag_nest_1_profile_2);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_qtag_nest_prof2_get(quad_idx, &qtag_nest_0_profile_2, &qtag_nest_1_profile_2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_prof2_get(%u %u %u)\n", quad_idx,
                qtag_nest_0_profile_2, qtag_nest_1_profile_2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (qtag_nest_0_profile_2 != gtmv(m, 3) || qtag_nest_1_profile_2 != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t qtag_nest_0_profile_2 = gtmv(m, 3);
        uint8_t max_num_of_vlans = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_max_vlans_set(%u %u %u)\n", quad_idx,
            qtag_nest_0_profile_2, max_num_of_vlans);
        ag_err = ag_drv_rnr_quad_parser_qtag_nest_max_vlans_set(quad_idx, qtag_nest_0_profile_2, max_num_of_vlans);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_qtag_nest_max_vlans_get(quad_idx, &qtag_nest_0_profile_2, &max_num_of_vlans);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_qtag_nest_max_vlans_get(%u %u %u)\n", quad_idx,
                qtag_nest_0_profile_2, max_num_of_vlans);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (qtag_nest_0_profile_2 != gtmv(m, 3) || max_num_of_vlans != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t user_ip_prot_0 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol0_set(%u %u)\n", quad_idx,
            user_ip_prot_0);
        ag_err = ag_drv_rnr_quad_parser_ip_protocol0_set(quad_idx, user_ip_prot_0);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ip_protocol0_get(quad_idx, &user_ip_prot_0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol0_get(%u %u)\n", quad_idx,
                user_ip_prot_0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (user_ip_prot_0 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t user_ip_prot_1 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol1_set(%u %u)\n", quad_idx,
            user_ip_prot_1);
        ag_err = ag_drv_rnr_quad_parser_ip_protocol1_set(quad_idx, user_ip_prot_1);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ip_protocol1_get(quad_idx, &user_ip_prot_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol1_get(%u %u)\n", quad_idx,
                user_ip_prot_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (user_ip_prot_1 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t user_ip_prot_2 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol2_set(%u %u)\n", quad_idx,
            user_ip_prot_2);
        ag_err = ag_drv_rnr_quad_parser_ip_protocol2_set(quad_idx, user_ip_prot_2);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ip_protocol2_get(quad_idx, &user_ip_prot_2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol2_get(%u %u)\n", quad_idx,
                user_ip_prot_2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (user_ip_prot_2 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t user_ip_prot_3 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol3_set(%u %u)\n", quad_idx,
            user_ip_prot_3);
        ag_err = ag_drv_rnr_quad_parser_ip_protocol3_set(quad_idx, user_ip_prot_3);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ip_protocol3_get(quad_idx, &user_ip_prot_3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ip_protocol3_get(%u %u)\n", quad_idx,
                user_ip_prot_3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (user_ip_prot_3 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_da_filter parser_da_filter = {.da_filt_msb = gtmv(m, 16), .da_filt_lsb = gtmv(m, 32), .da_filt_mask_msb = gtmv(m, 16), .da_filt_mask_l = gtmv(m, 32)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter_set(%u %u %u %u %u)\n", quad_idx,
            parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        ag_err = ag_drv_rnr_quad_parser_da_filter_set(quad_idx, &parser_da_filter);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter_get(quad_idx, &parser_da_filter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter_get(%u %u %u %u %u)\n", quad_idx,
                parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_da_filter.da_filt_msb != gtmv(m, 16) || parser_da_filter.da_filt_lsb != gtmv(m, 32) || parser_da_filter.da_filt_mask_msb != gtmv(m, 16) || parser_da_filter.da_filt_mask_l != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_da_filter parser_da_filter = {.da_filt_msb = gtmv(m, 16), .da_filt_lsb = gtmv(m, 32), .da_filt_mask_msb = gtmv(m, 16), .da_filt_mask_l = gtmv(m, 32)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter1_set(%u %u %u %u %u)\n", quad_idx,
            parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        ag_err = ag_drv_rnr_quad_parser_da_filter1_set(quad_idx, &parser_da_filter);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter1_get(quad_idx, &parser_da_filter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter1_get(%u %u %u %u %u)\n", quad_idx,
                parser_da_filter.da_filt_msb, parser_da_filter.da_filt_lsb, parser_da_filter.da_filt_mask_msb, parser_da_filter.da_filt_mask_l);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_da_filter.da_filt_msb != gtmv(m, 16) || parser_da_filter.da_filt_lsb != gtmv(m, 32) || parser_da_filter.da_filt_mask_msb != gtmv(m, 16) || parser_da_filter.da_filt_mask_l != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter2_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter2_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter2_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter2_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter3_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter3_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter3_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter3_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter4_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter4_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter4_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter4_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter5_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter5_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter5_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter5_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter6_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter6_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter6_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter6_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter7_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter7_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter7_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter7_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t da_filt_msb = gtmv(m, 16);
        uint32_t da_filt_lsb = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter8_set(%u %u %u)\n", quad_idx,
            da_filt_msb, da_filt_lsb);
        ag_err = ag_drv_rnr_quad_parser_da_filter8_set(quad_idx, da_filt_msb, da_filt_lsb);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_da_filter8_get(quad_idx, &da_filt_msb, &da_filt_lsb);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_da_filter8_get(%u %u %u)\n", quad_idx,
                da_filt_msb, da_filt_lsb);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filt_msb != gtmv(m, 16) || da_filt_lsb != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_da_filter_valid da_filter_valid = {.da_filt0_valid = gtmv(m, 1), .da_filt1_valid = gtmv(m, 1), .da_filt2_valid = gtmv(m, 1), .da_filt3_valid = gtmv(m, 1), .da_filt4_valid = gtmv(m, 1), .da_filt5_valid = gtmv(m, 1), .da_filt6_valid = gtmv(m, 1), .da_filt7_valid = gtmv(m, 1), .da_filt8_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_da_filter_valid_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, 
            da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, 
            da_filter_valid.da_filt8_valid);
        ag_err = ag_drv_rnr_quad_da_filter_valid_set(quad_idx, &da_filter_valid);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_da_filter_valid_get(quad_idx, &da_filter_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_da_filter_valid_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, 
                da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, 
                da_filter_valid.da_filt8_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filter_valid.da_filt0_valid != gtmv(m, 1) || da_filter_valid.da_filt1_valid != gtmv(m, 1) || da_filter_valid.da_filt2_valid != gtmv(m, 1) || da_filter_valid.da_filt3_valid != gtmv(m, 1) || da_filter_valid.da_filt4_valid != gtmv(m, 1) || da_filter_valid.da_filt5_valid != gtmv(m, 1) || da_filter_valid.da_filt6_valid != gtmv(m, 1) || da_filter_valid.da_filt7_valid != gtmv(m, 1) || da_filter_valid.da_filt8_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t exception_en = gtmv(m, 20);
        bdmf_session_print(session, "ag_drv_rnr_quad_exception_bits_set(%u %u)\n", quad_idx,
            exception_en);
        ag_err = ag_drv_rnr_quad_exception_bits_set(quad_idx, exception_en);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_exception_bits_get(quad_idx, &exception_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_exception_bits_get(%u %u)\n", quad_idx,
                exception_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (exception_en != gtmv(m, 20))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tcp_flags_filt = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_quad_tcp_flags_set(%u %u)\n", quad_idx,
            tcp_flags_filt);
        ag_err = ag_drv_rnr_quad_tcp_flags_set(quad_idx, tcp_flags_filt);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_tcp_flags_get(quad_idx, &tcp_flags_filt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_tcp_flags_get(%u %u)\n", quad_idx,
                tcp_flags_filt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tcp_flags_filt != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_us = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_rnr_quad_profile_us_set(%u %u)\n", quad_idx,
            profile_us);
        ag_err = ag_drv_rnr_quad_profile_us_set(quad_idx, profile_us);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_profile_us_get(quad_idx, &profile_us);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_profile_us_get(%u %u)\n", quad_idx,
                profile_us);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (profile_us != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean disable_l2tp_source_port_check = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_disable_l2tp_source_port_set(%u %u)\n", quad_idx,
            disable_l2tp_source_port_check);
        ag_err = ag_drv_rnr_quad_disable_l2tp_source_port_set(quad_idx, disable_l2tp_source_port_check);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_disable_l2tp_source_port_get(quad_idx, &disable_l2tp_source_port_check);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_disable_l2tp_source_port_get(%u %u)\n", quad_idx,
                disable_l2tp_source_port_check);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (disable_l2tp_source_port_check != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_snap_conf parser_snap_conf = {.code = gtmv(m, 24), .en_rfc1042 = gtmv(m, 1), .en_8021q = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_snap_conf_set(%u %u %u %u)\n", quad_idx,
            parser_snap_conf.code, parser_snap_conf.en_rfc1042, parser_snap_conf.en_8021q);
        ag_err = ag_drv_rnr_quad_parser_snap_conf_set(quad_idx, &parser_snap_conf);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_snap_conf_get(quad_idx, &parser_snap_conf);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_snap_conf_get(%u %u %u %u)\n", quad_idx,
                parser_snap_conf.code, parser_snap_conf.en_rfc1042, parser_snap_conf.en_8021q);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_snap_conf.code != gtmv(m, 24) || parser_snap_conf.en_rfc1042 != gtmv(m, 1) || parser_snap_conf.en_8021q != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_ipv6_filter parser_ipv6_filter = {.hop_by_hop_match = gtmv(m, 1), .routing_eh = gtmv(m, 1), .dest_opt_eh = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_ipv6_filter_set(%u %u %u %u)\n", quad_idx,
            parser_ipv6_filter.hop_by_hop_match, parser_ipv6_filter.routing_eh, parser_ipv6_filter.dest_opt_eh);
        ag_err = ag_drv_rnr_quad_parser_ipv6_filter_set(quad_idx, &parser_ipv6_filter);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_ipv6_filter_get(quad_idx, &parser_ipv6_filter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_ipv6_filter_get(%u %u %u %u)\n", quad_idx,
                parser_ipv6_filter.hop_by_hop_match, parser_ipv6_filter.routing_eh, parser_ipv6_filter.dest_opt_eh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_ipv6_filter.hop_by_hop_match != gtmv(m, 1) || parser_ipv6_filter.routing_eh != gtmv(m, 1) || parser_ipv6_filter.dest_opt_eh != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t cfg = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_eng_set(%u %u)\n", quad_idx,
            cfg);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_eng_set(quad_idx, cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_eng_get(quad_idx, &cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_eng_get(%u %u)\n", quad_idx,
                cfg);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ppp_code_0 = gtmv(m, 16);
        uint16_t ppp_code_1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(%u %u %u)\n", quad_idx,
            ppp_code_0, ppp_code_1);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(quad_idx, ppp_code_0, ppp_code_1);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(quad_idx, &ppp_code_0, &ppp_code_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(%u %u %u)\n", quad_idx,
                ppp_code_0, ppp_code_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ppp_code_0 != gtmv(m, 16) || ppp_code_1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ethtype_qtag_0 = gtmv(m, 16);
        uint16_t ethtype_qtag_1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(%u %u %u)\n", quad_idx,
            ethtype_qtag_0, ethtype_qtag_1);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(quad_idx, ethtype_qtag_0, ethtype_qtag_1);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(quad_idx, &ethtype_qtag_0, &ethtype_qtag_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(%u %u %u)\n", quad_idx,
                ethtype_qtag_0, ethtype_qtag_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ethtype_qtag_0 != gtmv(m, 16) || ethtype_qtag_1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ethype_0 = gtmv(m, 16);
        uint16_t ethype_1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(%u %u %u)\n", quad_idx,
            ethype_0, ethype_1);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(quad_idx, ethype_0, ethype_1);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(quad_idx, &ethype_0, &ethype_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(%u %u %u)\n", quad_idx,
                ethype_0, ethype_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ethype_0 != gtmv(m, 16) || ethype_1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ethype_2 = gtmv(m, 16);
        uint16_t ethype_3 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(%u %u %u)\n", quad_idx,
            ethype_2, ethype_3);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(quad_idx, ethype_2, ethype_3);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(quad_idx, &ethype_2, &ethype_3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(%u %u %u)\n", quad_idx,
                ethype_2, ethype_3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ethype_2 != gtmv(m, 16) || ethype_3 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_core_configuration_user_ethtype_config parser_core_configuration_user_ethtype_config = {.ethtype_user_prot_0 = gtmv(m, 2), .ethtype_user_prot_1 = gtmv(m, 2), .ethtype_user_prot_2 = gtmv(m, 2), .ethtype_user_prot_3 = gtmv(m, 2), .ethtype_user_en = gtmv(m, 4), .ethtype_user_offset_0 = gtmv(m, 4), .ethtype_user_offset_1 = gtmv(m, 4), .ethtype_user_offset_2 = gtmv(m, 4), .ethtype_user_offset_3 = gtmv(m, 4)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            parser_core_configuration_user_ethtype_config.ethtype_user_prot_0, parser_core_configuration_user_ethtype_config.ethtype_user_prot_1, parser_core_configuration_user_ethtype_config.ethtype_user_prot_2, parser_core_configuration_user_ethtype_config.ethtype_user_prot_3, 
            parser_core_configuration_user_ethtype_config.ethtype_user_en, parser_core_configuration_user_ethtype_config.ethtype_user_offset_0, parser_core_configuration_user_ethtype_config.ethtype_user_offset_1, parser_core_configuration_user_ethtype_config.ethtype_user_offset_2, 
            parser_core_configuration_user_ethtype_config.ethtype_user_offset_3);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(quad_idx, &parser_core_configuration_user_ethtype_config);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(quad_idx, &parser_core_configuration_user_ethtype_config);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                parser_core_configuration_user_ethtype_config.ethtype_user_prot_0, parser_core_configuration_user_ethtype_config.ethtype_user_prot_1, parser_core_configuration_user_ethtype_config.ethtype_user_prot_2, parser_core_configuration_user_ethtype_config.ethtype_user_prot_3, 
                parser_core_configuration_user_ethtype_config.ethtype_user_en, parser_core_configuration_user_ethtype_config.ethtype_user_offset_0, parser_core_configuration_user_ethtype_config.ethtype_user_offset_1, parser_core_configuration_user_ethtype_config.ethtype_user_offset_2, 
                parser_core_configuration_user_ethtype_config.ethtype_user_offset_3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_core_configuration_user_ethtype_config.ethtype_user_prot_0 != gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_prot_1 != gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_prot_2 != gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_prot_3 != gtmv(m, 2) || parser_core_configuration_user_ethtype_config.ethtype_user_en != gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_0 != gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_1 != gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_2 != gtmv(m, 4) || parser_core_configuration_user_ethtype_config.ethtype_user_offset_3 != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_da_filter_valid da_filter_valid = {.da_filt0_valid = gtmv(m, 1), .da_filt1_valid = gtmv(m, 1), .da_filt2_valid = gtmv(m, 1), .da_filt3_valid = gtmv(m, 1), .da_filt4_valid = gtmv(m, 1), .da_filt5_valid = gtmv(m, 1), .da_filt6_valid = gtmv(m, 1), .da_filt7_valid = gtmv(m, 1), .da_filt8_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, 
            da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, 
            da_filter_valid.da_filt8_valid);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(quad_idx, &da_filter_valid);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(quad_idx, &da_filter_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, 
                da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, 
                da_filter_valid.da_filt8_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filter_valid.da_filt0_valid != gtmv(m, 1) || da_filter_valid.da_filt1_valid != gtmv(m, 1) || da_filter_valid.da_filt2_valid != gtmv(m, 1) || da_filter_valid.da_filt3_valid != gtmv(m, 1) || da_filter_valid.da_filt4_valid != gtmv(m, 1) || da_filter_valid.da_filt5_valid != gtmv(m, 1) || da_filter_valid.da_filt6_valid != gtmv(m, 1) || da_filter_valid.da_filt7_valid != gtmv(m, 1) || da_filter_valid.da_filt8_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_da_filter_valid da_filter_valid = {.da_filt0_valid = gtmv(m, 1), .da_filt1_valid = gtmv(m, 1), .da_filt2_valid = gtmv(m, 1), .da_filt3_valid = gtmv(m, 1), .da_filt4_valid = gtmv(m, 1), .da_filt5_valid = gtmv(m, 1), .da_filt6_valid = gtmv(m, 1), .da_filt7_valid = gtmv(m, 1), .da_filt8_valid = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, 
            da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, 
            da_filter_valid.da_filt8_valid);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(quad_idx, &da_filter_valid);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(quad_idx, &da_filter_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                da_filter_valid.da_filt0_valid, da_filter_valid.da_filt1_valid, da_filter_valid.da_filt2_valid, da_filter_valid.da_filt3_valid, 
                da_filter_valid.da_filt4_valid, da_filter_valid.da_filt5_valid, da_filter_valid.da_filt6_valid, da_filter_valid.da_filt7_valid, 
                da_filter_valid.da_filt8_valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (da_filter_valid.da_filt0_valid != gtmv(m, 1) || da_filter_valid.da_filt1_valid != gtmv(m, 1) || da_filter_valid.da_filt2_valid != gtmv(m, 1) || da_filter_valid.da_filt3_valid != gtmv(m, 1) || da_filter_valid.da_filt4_valid != gtmv(m, 1) || da_filter_valid.da_filt5_valid != gtmv(m, 1) || da_filter_valid.da_filt6_valid != gtmv(m, 1) || da_filter_valid.da_filt7_valid != gtmv(m, 1) || da_filter_valid.da_filt8_valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t gre_protocol = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(%u %u)\n", quad_idx,
            gre_protocol);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(quad_idx, gre_protocol);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(quad_idx, &gre_protocol);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(%u %u)\n", quad_idx,
                gre_protocol);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gre_protocol != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_core_configuration_prop_tag_cfg parser_core_configuration_prop_tag_cfg = {.size_profile_0 = gtmv(m, 5), .size_profile_1 = gtmv(m, 5), .size_profile_2 = gtmv(m, 5), .pre_da_dprofile_0 = gtmv(m, 1), .pre_da_dprofile_1 = gtmv(m, 1), .pre_da_dprofile_2 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(%u %u %u %u %u %u %u)\n", quad_idx,
            parser_core_configuration_prop_tag_cfg.size_profile_0, parser_core_configuration_prop_tag_cfg.size_profile_1, parser_core_configuration_prop_tag_cfg.size_profile_2, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0, 
            parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(quad_idx, &parser_core_configuration_prop_tag_cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(quad_idx, &parser_core_configuration_prop_tag_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(%u %u %u %u %u %u %u)\n", quad_idx,
                parser_core_configuration_prop_tag_cfg.size_profile_0, parser_core_configuration_prop_tag_cfg.size_profile_1, parser_core_configuration_prop_tag_cfg.size_profile_2, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0, 
                parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1, parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_core_configuration_prop_tag_cfg.size_profile_0 != gtmv(m, 5) || parser_core_configuration_prop_tag_cfg.size_profile_1 != gtmv(m, 5) || parser_core_configuration_prop_tag_cfg.size_profile_2 != gtmv(m, 5) || parser_core_configuration_prop_tag_cfg.pre_da_dprofile_0 != gtmv(m, 1) || parser_core_configuration_prop_tag_cfg.pre_da_dprofile_1 != gtmv(m, 1) || parser_core_configuration_prop_tag_cfg.pre_da_dprofile_2 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t mask = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_dos_attack_set(%u %u)\n", quad_idx,
            mask);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_dos_attack_set(quad_idx, mask);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_dos_attack_get(quad_idx, &mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_dos_attack_get(%u %u)\n", quad_idx,
                mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t v4_size = gtmv(m, 11);
        uint16_t v6_size = gtmv(m, 11);
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_set(%u %u %u)\n", quad_idx,
            v4_size, v6_size);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_set(quad_idx, v4_size, v6_size);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_get(quad_idx, &v4_size, &v6_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_get(%u %u %u)\n", quad_idx,
                v4_size, v6_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (v4_size != gtmv(m, 11) || v6_size != gtmv(m, 11))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_parser_core_configuration_key_cfg parser_core_configuration_key_cfg = {.l2_tos_mask = gtmv(m, 8), .l3_tos_mask = gtmv(m, 8), .l2_exclude_smac = gtmv(m, 1), .tcp_pure_ack_mask = gtmv(m, 1), .incude_dei_in_vlans_crc = gtmv(m, 1), .key_size = gtmv(m, 1), .max_num_of_vlans_in_crc = gtmv(m, 4), .l3_tcp_pure_ack_mask = gtmv(m, 1), .rsrv = gtmv(m, 7)};
        bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_key_cfg_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            parser_core_configuration_key_cfg.l2_tos_mask, parser_core_configuration_key_cfg.l3_tos_mask, parser_core_configuration_key_cfg.l2_exclude_smac, parser_core_configuration_key_cfg.tcp_pure_ack_mask, 
            parser_core_configuration_key_cfg.incude_dei_in_vlans_crc, parser_core_configuration_key_cfg.key_size, parser_core_configuration_key_cfg.max_num_of_vlans_in_crc, parser_core_configuration_key_cfg.l3_tcp_pure_ack_mask, 
            parser_core_configuration_key_cfg.rsrv);
        ag_err = ag_drv_rnr_quad_parser_core_configuration_key_cfg_set(quad_idx, &parser_core_configuration_key_cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_parser_core_configuration_key_cfg_get(quad_idx, &parser_core_configuration_key_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_parser_core_configuration_key_cfg_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                parser_core_configuration_key_cfg.l2_tos_mask, parser_core_configuration_key_cfg.l3_tos_mask, parser_core_configuration_key_cfg.l2_exclude_smac, parser_core_configuration_key_cfg.tcp_pure_ack_mask, 
                parser_core_configuration_key_cfg.incude_dei_in_vlans_crc, parser_core_configuration_key_cfg.key_size, parser_core_configuration_key_cfg.max_num_of_vlans_in_crc, parser_core_configuration_key_cfg.l3_tcp_pure_ack_mask, 
                parser_core_configuration_key_cfg.rsrv);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (parser_core_configuration_key_cfg.l2_tos_mask != gtmv(m, 8) || parser_core_configuration_key_cfg.l3_tos_mask != gtmv(m, 8) || parser_core_configuration_key_cfg.l2_exclude_smac != gtmv(m, 1) || parser_core_configuration_key_cfg.tcp_pure_ack_mask != gtmv(m, 1) || parser_core_configuration_key_cfg.incude_dei_in_vlans_crc != gtmv(m, 1) || parser_core_configuration_key_cfg.key_size != gtmv(m, 1) || parser_core_configuration_key_cfg.max_num_of_vlans_in_crc != gtmv(m, 4) || parser_core_configuration_key_cfg.l3_tcp_pure_ack_mask != gtmv(m, 1) || parser_core_configuration_key_cfg.rsrv != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_general_config_dma_arb_cfg general_config_dma_arb_cfg = {.use_fifo_for_ddr_only = gtmv(m, 1), .token_arbiter_is_rr = gtmv(m, 1), .chicken_no_flowctrl = gtmv(m, 1), .flow_ctrl_clear_token = gtmv(m, 1), .ddr_congest_threshold = gtmv(m, 5), .psram_congest_threshold = gtmv(m, 5), .enable_reply_threshold = gtmv(m, 1), .ddr_reply_threshold = gtmv(m, 8), .psram_reply_threshold = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_dma_arb_cfg_set(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            general_config_dma_arb_cfg.use_fifo_for_ddr_only, general_config_dma_arb_cfg.token_arbiter_is_rr, general_config_dma_arb_cfg.chicken_no_flowctrl, general_config_dma_arb_cfg.flow_ctrl_clear_token, 
            general_config_dma_arb_cfg.ddr_congest_threshold, general_config_dma_arb_cfg.psram_congest_threshold, general_config_dma_arb_cfg.enable_reply_threshold, general_config_dma_arb_cfg.ddr_reply_threshold, 
            general_config_dma_arb_cfg.psram_reply_threshold);
        ag_err = ag_drv_rnr_quad_general_config_dma_arb_cfg_set(quad_idx, &general_config_dma_arb_cfg);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_dma_arb_cfg_get(quad_idx, &general_config_dma_arb_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_dma_arb_cfg_get(%u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                general_config_dma_arb_cfg.use_fifo_for_ddr_only, general_config_dma_arb_cfg.token_arbiter_is_rr, general_config_dma_arb_cfg.chicken_no_flowctrl, general_config_dma_arb_cfg.flow_ctrl_clear_token, 
                general_config_dma_arb_cfg.ddr_congest_threshold, general_config_dma_arb_cfg.psram_congest_threshold, general_config_dma_arb_cfg.enable_reply_threshold, general_config_dma_arb_cfg.ddr_reply_threshold, 
                general_config_dma_arb_cfg.psram_reply_threshold);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_config_dma_arb_cfg.use_fifo_for_ddr_only != gtmv(m, 1) || general_config_dma_arb_cfg.token_arbiter_is_rr != gtmv(m, 1) || general_config_dma_arb_cfg.chicken_no_flowctrl != gtmv(m, 1) || general_config_dma_arb_cfg.flow_ctrl_clear_token != gtmv(m, 1) || general_config_dma_arb_cfg.ddr_congest_threshold != gtmv(m, 5) || general_config_dma_arb_cfg.psram_congest_threshold != gtmv(m, 5) || general_config_dma_arb_cfg.enable_reply_threshold != gtmv(m, 1) || general_config_dma_arb_cfg.ddr_reply_threshold != gtmv(m, 8) || general_config_dma_arb_cfg.psram_reply_threshold != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_base_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram0_base_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram0_base_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_base_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_base_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram1_base_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram1_base_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_base_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_base_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram2_base_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram2_base_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_base_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_base_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram3_base_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram3_base_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_base_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_base_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_ddr0_base_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_ddr0_base_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_base_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_base_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_ddr1_base_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_ddr1_base_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_base_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_mask_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram0_mask_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram0_mask_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram0_mask_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_mask_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram1_mask_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram1_mask_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram1_mask_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_mask_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram2_mask_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram2_mask_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram2_mask_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_mask_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_psram3_mask_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_psram3_mask_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_psram3_mask_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_mask_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_ddr0_mask_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_ddr0_mask_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr0_mask_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_mask_set(%u %u)\n", quad_idx,
            val);
        ag_err = ag_drv_rnr_quad_general_config_ddr1_mask_set(quad_idx, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_ddr1_mask_get(quad_idx, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ddr1_mask_get(%u %u)\n", quad_idx,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_general_config_profiling_config general_config_profiling_config = {.counter_lsb_sel = gtmv(m, 5), .enable_trace_core_0 = gtmv(m, 1), .enable_trace_core_1 = gtmv(m, 1), .enable_trace_core_2 = gtmv(m, 1), .enable_trace_core_3 = gtmv(m, 1), .enable_trace_core_4 = gtmv(m, 1), .enable_trace_core_5 = gtmv(m, 1), .enable_trace_core_6 = gtmv(m, 1), .enable_trace_core_7 = gtmv(m, 1), .enable_trace_core_8 = gtmv(m, 1), .enable_trace_core_9 = gtmv(m, 1), .enable_trace_core_10 = gtmv(m, 1), .enable_trace_core_11 = gtmv(m, 1), .enable_trace_core_12 = gtmv(m, 1), .enable_trace_core_13 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_profiling_config_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            general_config_profiling_config.counter_lsb_sel, general_config_profiling_config.enable_trace_core_0, general_config_profiling_config.enable_trace_core_1, general_config_profiling_config.enable_trace_core_2, 
            general_config_profiling_config.enable_trace_core_3, general_config_profiling_config.enable_trace_core_4, general_config_profiling_config.enable_trace_core_5, general_config_profiling_config.enable_trace_core_6, 
            general_config_profiling_config.enable_trace_core_7, general_config_profiling_config.enable_trace_core_8, general_config_profiling_config.enable_trace_core_9, general_config_profiling_config.enable_trace_core_10, 
            general_config_profiling_config.enable_trace_core_11, general_config_profiling_config.enable_trace_core_12, general_config_profiling_config.enable_trace_core_13);
        ag_err = ag_drv_rnr_quad_general_config_profiling_config_set(quad_idx, &general_config_profiling_config);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_profiling_config_get(quad_idx, &general_config_profiling_config);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_profiling_config_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                general_config_profiling_config.counter_lsb_sel, general_config_profiling_config.enable_trace_core_0, general_config_profiling_config.enable_trace_core_1, general_config_profiling_config.enable_trace_core_2, 
                general_config_profiling_config.enable_trace_core_3, general_config_profiling_config.enable_trace_core_4, general_config_profiling_config.enable_trace_core_5, general_config_profiling_config.enable_trace_core_6, 
                general_config_profiling_config.enable_trace_core_7, general_config_profiling_config.enable_trace_core_8, general_config_profiling_config.enable_trace_core_9, general_config_profiling_config.enable_trace_core_10, 
                general_config_profiling_config.enable_trace_core_11, general_config_profiling_config.enable_trace_core_12, general_config_profiling_config.enable_trace_core_13);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_config_profiling_config.counter_lsb_sel != gtmv(m, 5) || general_config_profiling_config.enable_trace_core_0 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_1 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_2 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_3 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_4 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_5 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_6 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_7 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_8 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_9 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_10 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_11 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_12 != gtmv(m, 1) || general_config_profiling_config.enable_trace_core_13 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t addr = gtmv(m, 13);
        uint8_t thread = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(%u %u %u)\n", quad_idx,
            addr, thread);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(quad_idx, addr, thread);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(quad_idx, &addr, &thread);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(%u %u %u)\n", quad_idx,
                addr, thread);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 13) || thread != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t handler_addr = gtmv(m, 13);
        uint16_t update_pc_value = gtmv(m, 13);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(%u %u %u)\n", quad_idx,
            handler_addr, update_pc_value);
        ag_err = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(quad_idx, handler_addr, update_pc_value);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(quad_idx, &handler_addr, &update_pc_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(%u %u %u)\n", quad_idx,
                handler_addr, update_pc_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (handler_addr != gtmv(m, 13) || update_pc_value != gtmv(m, 13))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_general_config_powersave_config general_config_powersave_config = {.time_counter = gtmv(m, 8), .enable_powersave_core_0 = gtmv(m, 1), .enable_powersave_core_1 = gtmv(m, 1), .enable_powersave_core_2 = gtmv(m, 1), .enable_powersave_core_3 = gtmv(m, 1), .enable_powersave_core_4 = gtmv(m, 1), .enable_powersave_core_5 = gtmv(m, 1), .enable_powersave_core_6 = gtmv(m, 1), .enable_powersave_core_7 = gtmv(m, 1), .enable_powersave_core_8 = gtmv(m, 1), .enable_powersave_core_9 = gtmv(m, 1), .enable_powersave_core_10 = gtmv(m, 1), .enable_powersave_core_11 = gtmv(m, 1), .enable_powersave_core_12 = gtmv(m, 1), .enable_powersave_core_13 = gtmv(m, 1), .enable_cpu_if_clk_gating = gtmv(m, 1), .enable_common_reg_clk_gating = gtmv(m, 1), .enable_ec_blocks_clk_gating = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_powersave_config_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            general_config_powersave_config.time_counter, general_config_powersave_config.enable_powersave_core_0, general_config_powersave_config.enable_powersave_core_1, general_config_powersave_config.enable_powersave_core_2, 
            general_config_powersave_config.enable_powersave_core_3, general_config_powersave_config.enable_powersave_core_4, general_config_powersave_config.enable_powersave_core_5, general_config_powersave_config.enable_powersave_core_6, 
            general_config_powersave_config.enable_powersave_core_7, general_config_powersave_config.enable_powersave_core_8, general_config_powersave_config.enable_powersave_core_9, general_config_powersave_config.enable_powersave_core_10, 
            general_config_powersave_config.enable_powersave_core_11, general_config_powersave_config.enable_powersave_core_12, general_config_powersave_config.enable_powersave_core_13, general_config_powersave_config.enable_cpu_if_clk_gating, 
            general_config_powersave_config.enable_common_reg_clk_gating, general_config_powersave_config.enable_ec_blocks_clk_gating);
        ag_err = ag_drv_rnr_quad_general_config_powersave_config_set(quad_idx, &general_config_powersave_config);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_powersave_config_get(quad_idx, &general_config_powersave_config);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_powersave_config_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                general_config_powersave_config.time_counter, general_config_powersave_config.enable_powersave_core_0, general_config_powersave_config.enable_powersave_core_1, general_config_powersave_config.enable_powersave_core_2, 
                general_config_powersave_config.enable_powersave_core_3, general_config_powersave_config.enable_powersave_core_4, general_config_powersave_config.enable_powersave_core_5, general_config_powersave_config.enable_powersave_core_6, 
                general_config_powersave_config.enable_powersave_core_7, general_config_powersave_config.enable_powersave_core_8, general_config_powersave_config.enable_powersave_core_9, general_config_powersave_config.enable_powersave_core_10, 
                general_config_powersave_config.enable_powersave_core_11, general_config_powersave_config.enable_powersave_core_12, general_config_powersave_config.enable_powersave_core_13, general_config_powersave_config.enable_cpu_if_clk_gating, 
                general_config_powersave_config.enable_common_reg_clk_gating, general_config_powersave_config.enable_ec_blocks_clk_gating);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_config_powersave_config.time_counter != gtmv(m, 8) || general_config_powersave_config.enable_powersave_core_0 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_1 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_2 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_3 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_4 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_5 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_6 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_7 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_8 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_9 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_10 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_11 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_12 != gtmv(m, 1) || general_config_powersave_config.enable_powersave_core_13 != gtmv(m, 1) || general_config_powersave_config.enable_cpu_if_clk_gating != gtmv(m, 1) || general_config_powersave_config.enable_common_reg_clk_gating != gtmv(m, 1) || general_config_powersave_config.enable_ec_blocks_clk_gating != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_general_config_powersave_status general_config_powersave_status = {.acc_status_0 = gtmv(m, 1), .acc_status_1 = gtmv(m, 1), .acc_status_2 = gtmv(m, 1), .acc_status_3 = gtmv(m, 1), .acc_status_4 = gtmv(m, 1), .acc_status_5 = gtmv(m, 1), .acc_status_6 = gtmv(m, 1), .acc_status_7 = gtmv(m, 1), .acc_status_8 = gtmv(m, 1), .acc_status_9 = gtmv(m, 1), .acc_status_10 = gtmv(m, 1), .acc_status_11 = gtmv(m, 1), .acc_status_12 = gtmv(m, 1), .acc_status_13 = gtmv(m, 1), .core_status_0 = gtmv(m, 1), .core_status_1 = gtmv(m, 1), .core_status_2 = gtmv(m, 1), .core_status_3 = gtmv(m, 1), .core_status_4 = gtmv(m, 1), .core_status_5 = gtmv(m, 1), .core_status_6 = gtmv(m, 1), .core_status_7 = gtmv(m, 1), .core_status_8 = gtmv(m, 1), .core_status_9 = gtmv(m, 1), .core_status_10 = gtmv(m, 1), .core_status_11 = gtmv(m, 1), .core_status_12 = gtmv(m, 1), .core_status_13 = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_powersave_status_get(quad_idx, &general_config_powersave_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_powersave_status_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                general_config_powersave_status.acc_status_0, general_config_powersave_status.acc_status_1, general_config_powersave_status.acc_status_2, general_config_powersave_status.acc_status_3, 
                general_config_powersave_status.acc_status_4, general_config_powersave_status.acc_status_5, general_config_powersave_status.acc_status_6, general_config_powersave_status.acc_status_7, 
                general_config_powersave_status.acc_status_8, general_config_powersave_status.acc_status_9, general_config_powersave_status.acc_status_10, general_config_powersave_status.acc_status_11, 
                general_config_powersave_status.acc_status_12, general_config_powersave_status.acc_status_13, general_config_powersave_status.core_status_0, general_config_powersave_status.core_status_1, 
                general_config_powersave_status.core_status_2, general_config_powersave_status.core_status_3, general_config_powersave_status.core_status_4, general_config_powersave_status.core_status_5, 
                general_config_powersave_status.core_status_6, general_config_powersave_status.core_status_7, general_config_powersave_status.core_status_8, general_config_powersave_status.core_status_9, 
                general_config_powersave_status.core_status_10, general_config_powersave_status.core_status_11, general_config_powersave_status.core_status_12, general_config_powersave_status.core_status_13);
        }
    }

    {
        uint16_t data_addr_start = gtmv(m, 16);
        uint16_t data_addr_stop = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_set(%u %u %u)\n", quad_idx,
            data_addr_start, data_addr_stop);
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_set(quad_idx, data_addr_start, data_addr_stop);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_get(quad_idx, &data_addr_start, &data_addr_stop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_get(%u %u %u)\n", quad_idx,
                data_addr_start, data_addr_stop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data_addr_start != gtmv(m, 16) || data_addr_stop != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t data_addr_start = gtmv(m, 16);
        uint16_t data_addr_stop = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_set(%u %u %u)\n", quad_idx,
            data_addr_start, data_addr_stop);
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_set(quad_idx, data_addr_start, data_addr_stop);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_get(quad_idx, &data_addr_start, &data_addr_stop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_get(%u %u %u)\n", quad_idx,
                data_addr_start, data_addr_stop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data_addr_start != gtmv(m, 16) || data_addr_stop != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t data_addr_start = gtmv(m, 16);
        uint16_t data_addr_stop = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_set(%u %u %u)\n", quad_idx,
            data_addr_start, data_addr_stop);
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_set(quad_idx, data_addr_start, data_addr_stop);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_get(quad_idx, &data_addr_start, &data_addr_stop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_get(%u %u %u)\n", quad_idx,
                data_addr_start, data_addr_stop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data_addr_start != gtmv(m, 16) || data_addr_stop != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t data_addr_start = gtmv(m, 16);
        uint16_t data_addr_stop = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_set(%u %u %u)\n", quad_idx,
            data_addr_start, data_addr_stop);
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_set(quad_idx, data_addr_start, data_addr_stop);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_get(quad_idx, &data_addr_start, &data_addr_stop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_get(%u %u %u)\n", quad_idx,
                data_addr_start, data_addr_stop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data_addr_start != gtmv(m, 16) || data_addr_stop != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t thread_0 = gtmv(m, 4);
        uint8_t thread_1 = gtmv(m, 4);
        uint8_t thread_2 = gtmv(m, 4);
        uint8_t thread_3 = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_set(%u %u %u %u %u)\n", quad_idx,
            thread_0, thread_1, thread_2, thread_3);
        ag_err = ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_set(quad_idx, thread_0, thread_1, thread_2, thread_3);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_get(quad_idx, &thread_0, &thread_1, &thread_2, &thread_3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_get(%u %u %u %u %u)\n", quad_idx,
                thread_0, thread_1, thread_2, thread_3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (thread_0 != gtmv(m, 4) || thread_1 != gtmv(m, 4) || thread_2 != gtmv(m, 4) || thread_3 != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean enable_statistics = gtmv(m, 1);
        bdmf_boolean sw_reset = gtmv(m, 1);
        uint8_t dest_pid = gtmv(m, 8);
        uint8_t master_select = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ubus_counter_control_set(%u %u %u %u %u)\n", quad_idx,
            enable_statistics, sw_reset, dest_pid, master_select);
        ag_err = ag_drv_rnr_quad_general_config_ubus_counter_control_set(quad_idx, enable_statistics, sw_reset, dest_pid, master_select);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_ubus_counter_control_get(quad_idx, &enable_statistics, &sw_reset, &dest_pid, &master_select);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ubus_counter_control_get(%u %u %u %u %u)\n", quad_idx,
                enable_statistics, sw_reset, dest_pid, master_select);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (enable_statistics != gtmv(m, 1) || sw_reset != gtmv(m, 1) || dest_pid != gtmv(m, 8) || master_select != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t downcnt_value = gtmv(m, 30);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ubus_down_counter_set(%u %u)\n", quad_idx,
            downcnt_value);
        ag_err = ag_drv_rnr_quad_general_config_ubus_down_counter_set(quad_idx, downcnt_value);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_ubus_down_counter_get(quad_idx, &downcnt_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_ubus_down_counter_get(%u %u)\n", quad_idx,
                downcnt_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (downcnt_value != gtmv(m, 30))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t counter_value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_all_xfers_cnt_get(quad_idx, &counter_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_all_xfers_cnt_get(%u %u)\n", quad_idx,
                counter_value);
        }
    }

    {
        uint32_t counter_value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_read_xfers_cnt_get(quad_idx, &counter_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_read_xfers_cnt_get(%u %u)\n", quad_idx,
                counter_value);
        }
    }

    {
        uint32_t counter_value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_read_data_cnt_get(quad_idx, &counter_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_read_data_cnt_get(%u %u)\n", quad_idx,
                counter_value);
        }
    }

    {
        uint32_t counter_value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_write_data_cnt_get(quad_idx, &counter_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_write_data_cnt_get(%u %u)\n", quad_idx,
                counter_value);
        }
    }

    {
        uint8_t ddr_pid = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_misc_cfg_set(%u %u)\n", quad_idx,
            ddr_pid);
        ag_err = ag_drv_rnr_quad_general_config_misc_cfg_set(quad_idx, ddr_pid);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_misc_cfg_get(quad_idx, &ddr_pid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_misc_cfg_get(%u %u)\n", quad_idx,
                ddr_pid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_pid != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean enable_counter = gtmv(m, 1);
        bdmf_boolean enable_random = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_aqm_control_set(%u %u %u)\n", quad_idx,
            enable_counter, enable_random);
        ag_err = ag_drv_rnr_quad_general_config_aqm_control_set(quad_idx, enable_counter, enable_random);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_aqm_control_get(quad_idx, &enable_counter, &enable_random);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_aqm_control_get(%u %u %u)\n", quad_idx,
                enable_counter, enable_random);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (enable_counter != gtmv(m, 1) || enable_random != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t random_value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_aqm_randm_value_get(quad_idx, &random_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_aqm_randm_value_get(%u %u)\n", quad_idx,
                random_value);
        }
    }

    {
        uint32_t random_seed = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_aqm_random_seed_set(%u %u)\n", quad_idx,
            random_seed);
        ag_err = ag_drv_rnr_quad_general_config_aqm_random_seed_set(quad_idx, random_seed);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_aqm_random_seed_get(quad_idx, &random_seed);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_aqm_random_seed_get(%u %u)\n", quad_idx,
                random_seed);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (random_seed != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean random_inc = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_aqm_random_test_inc_set(%u %u)\n", quad_idx,
            random_inc);
        ag_err = ag_drv_rnr_quad_general_config_aqm_random_test_inc_set(quad_idx, random_inc);
    }

    {
        uint16_t multi_psel_master_sel = gtmv(m, 14);
        uint16_t multi_psel_mask = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_rnr_quad_general_config_multi_psel_cfg_set(%u %u %u)\n", quad_idx,
            multi_psel_master_sel, multi_psel_mask);
        ag_err = ag_drv_rnr_quad_general_config_multi_psel_cfg_set(quad_idx, multi_psel_master_sel, multi_psel_mask);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_general_config_multi_psel_cfg_get(quad_idx, &multi_psel_master_sel, &multi_psel_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_general_config_multi_psel_cfg_get(%u %u %u)\n", quad_idx,
                multi_psel_master_sel, multi_psel_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (multi_psel_master_sel != gtmv(m, 14) || multi_psel_mask != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_debug_fifo_config debug_fifo_config = {.psram_hdr_sw_rst_0 = gtmv(m, 1), .psram_data_sw_rst_0 = gtmv(m, 1), .ddr_hdr_sw_rst_0 = gtmv(m, 1), .select_fifos_for_debug = gtmv(m, 1), .psram_hdr_sw_rst_1 = gtmv(m, 1), .psram_data_sw_rst_1 = gtmv(m, 1), .ddr_hdr_sw_rst_1 = gtmv(m, 1), .psram_hdr_sw_rd_addr = gtmv(m, 4), .psram_data_sw_rd_addr = gtmv(m, 4), .ddr_hdr_sw_rd_addr = gtmv(m, 4)};
        bdmf_session_print(session, "ag_drv_rnr_quad_debug_fifo_config_set(%u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
            debug_fifo_config.psram_hdr_sw_rst_0, debug_fifo_config.psram_data_sw_rst_0, debug_fifo_config.ddr_hdr_sw_rst_0, debug_fifo_config.select_fifos_for_debug, 
            debug_fifo_config.psram_hdr_sw_rst_1, debug_fifo_config.psram_data_sw_rst_1, debug_fifo_config.ddr_hdr_sw_rst_1, debug_fifo_config.psram_hdr_sw_rd_addr, 
            debug_fifo_config.psram_data_sw_rd_addr, debug_fifo_config.ddr_hdr_sw_rd_addr);
        ag_err = ag_drv_rnr_quad_debug_fifo_config_set(quad_idx, &debug_fifo_config);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_fifo_config_get(quad_idx, &debug_fifo_config);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_fifo_config_get(%u %u %u %u %u %u %u %u %u %u %u)\n", quad_idx,
                debug_fifo_config.psram_hdr_sw_rst_0, debug_fifo_config.psram_data_sw_rst_0, debug_fifo_config.ddr_hdr_sw_rst_0, debug_fifo_config.select_fifos_for_debug, 
                debug_fifo_config.psram_hdr_sw_rst_1, debug_fifo_config.psram_data_sw_rst_1, debug_fifo_config.ddr_hdr_sw_rst_1, debug_fifo_config.psram_hdr_sw_rd_addr, 
                debug_fifo_config.psram_data_sw_rd_addr, debug_fifo_config.ddr_hdr_sw_rd_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (debug_fifo_config.psram_hdr_sw_rst_0 != gtmv(m, 1) || debug_fifo_config.psram_data_sw_rst_0 != gtmv(m, 1) || debug_fifo_config.ddr_hdr_sw_rst_0 != gtmv(m, 1) || debug_fifo_config.select_fifos_for_debug != gtmv(m, 1) || debug_fifo_config.psram_hdr_sw_rst_1 != gtmv(m, 1) || debug_fifo_config.psram_data_sw_rst_1 != gtmv(m, 1) || debug_fifo_config.ddr_hdr_sw_rst_1 != gtmv(m, 1) || debug_fifo_config.psram_hdr_sw_rd_addr != gtmv(m, 4) || debug_fifo_config.psram_data_sw_rd_addr != gtmv(m, 4) || debug_fifo_config.ddr_hdr_sw_rd_addr != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        rnr_quad_debug_psram_hdr_fifo_status debug_psram_hdr_fifo_status = {.full = gtmv(m, 1), .empty = gtmv(m, 1), .push_wr_cntr = gtmv(m, 5), .pop_rd_cntr = gtmv(m, 5), .used_words = gtmv(m, 5)};
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(quad_idx, &debug_psram_hdr_fifo_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(%u %u %u %u %u %u)\n", quad_idx,
                debug_psram_hdr_fifo_status.full, debug_psram_hdr_fifo_status.empty, debug_psram_hdr_fifo_status.push_wr_cntr, debug_psram_hdr_fifo_status.pop_rd_cntr, 
                debug_psram_hdr_fifo_status.used_words);
        }
    }

    {
        rnr_quad_debug_psram_data_fifo_status debug_psram_data_fifo_status = {.full = gtmv(m, 1), .empty = gtmv(m, 1), .almost_full = gtmv(m, 1), .push_wr_cntr = gtmv(m, 5), .pop_rd_cntr = gtmv(m, 5), .used_words = gtmv(m, 5)};
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_psram_data_fifo_status_get(quad_idx, &debug_psram_data_fifo_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_data_fifo_status_get(%u %u %u %u %u %u %u)\n", quad_idx,
                debug_psram_data_fifo_status.full, debug_psram_data_fifo_status.empty, debug_psram_data_fifo_status.almost_full, debug_psram_data_fifo_status.push_wr_cntr, 
                debug_psram_data_fifo_status.pop_rd_cntr, debug_psram_data_fifo_status.used_words);
        }
    }

    {
        rnr_quad_debug_ddr_hdr_fifo_status debug_ddr_hdr_fifo_status = {.full = gtmv(m, 1), .empty = gtmv(m, 1), .push_wr_cntr = gtmv(m, 5), .pop_rd_cntr = gtmv(m, 5), .used_words = gtmv(m, 5)};
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(quad_idx, &debug_ddr_hdr_fifo_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(%u %u %u %u %u %u)\n", quad_idx,
                debug_ddr_hdr_fifo_status.full, debug_ddr_hdr_fifo_status.empty, debug_ddr_hdr_fifo_status.push_wr_cntr, debug_ddr_hdr_fifo_status.pop_rd_cntr, 
                debug_ddr_hdr_fifo_status.used_words);
        }
    }

    {
        rnr_quad_debug_ddr_data_fifo_status debug_ddr_data_fifo_status = {.full = gtmv(m, 1), .empty = gtmv(m, 1), .almost_full = gtmv(m, 1), .wr_cntr = gtmv(m, 9), .rd_cntr = gtmv(m, 9)};
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(quad_idx, &debug_ddr_data_fifo_status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(%u %u %u %u %u %u)\n", quad_idx,
                debug_ddr_data_fifo_status.full, debug_ddr_data_fifo_status.empty, debug_ddr_data_fifo_status.almost_full, debug_ddr_data_fifo_status.wr_cntr, 
                debug_ddr_data_fifo_status.rd_cntr);
        }
    }

    {
        uint8_t read_addr = gtmv(m, 8);
        uint16_t used_words = gtmv(m, 9);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(quad_idx, &read_addr, &used_words);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(%u %u %u)\n", quad_idx,
                read_addr, used_words);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(quad_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(%u %u)\n", quad_idx,
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(quad_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(%u %u)\n", quad_idx,
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(quad_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(%u %u)\n", quad_idx,
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(quad_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(%u %u)\n", quad_idx,
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(quad_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(%u %u)\n", quad_idx,
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(quad_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(%u %u)\n", quad_idx,
                data);
        }
    }

    {
        uint32_t index = gtmv(m, 5);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(%u %u %u)\n", quad_idx, index,
            val);
        ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, index, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(quad_idx, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(%u %u %u)\n", quad_idx, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 5);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(%u %u %u)\n", quad_idx, index,
            val);
        ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(quad_idx, index, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(quad_idx, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(%u %u %u)\n", quad_idx, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(%u %u %u)\n", quad_idx, index,
            val);
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(quad_idx, index, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(quad_idx, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(%u %u %u)\n", quad_idx, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(%u %u %u)\n", quad_idx, index,
            val);
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad_idx, index, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(quad_idx, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(%u %u %u)\n", quad_idx, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(%u %u %u)\n", quad_idx, index,
            val);
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(quad_idx, index, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(quad_idx, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(%u %u %u)\n", quad_idx, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(%u %u %u)\n", quad_idx, index,
            val);
        ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(quad_idx, index, val);
        if (!ag_err)
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(quad_idx, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(%u %u %u)\n", quad_idx, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_rnr_quad_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t quad_idx = parm[2].value.unumber;
    bdmfmon_cmd_parm_t *p_start, *p_stop;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t ext_test_success_cnt = 0;
    uint32_t ext_test_failure_cnt = 0;
    uint32_t start_idx = 0;
    uint32_t stop_idx = 0;

    p_start = bdmfmon_cmd_find(session, "start_idx");
    p_stop = bdmfmon_cmd_find(session, "stop_idx");

    if (p_start)
        start_idx = p_start->value.unumber;
    if (p_stop)
        stop_idx = p_stop->value.unumber;

    if ((start_idx > stop_idx) && (stop_idx != 0))
    {
        bdmf_session_print(session, "ERROR: start_idx must be less than stop_idx\n");
        return BDMF_ERR_PARM;
    }

    m = bdmf_test_method_high; /* "Initialization" method */
    switch (parm[1].value.unumber)
    {
    case cli_rnr_quad_ext_flowctrl_config_token_val:
    {
        uint32_t max_index = 36;
        uint32_t index = gtmv(m, 5);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(quad_idx, index, &val);

            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(%u %u %u)\n", quad_idx, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ext_flowctrl_config_token_val completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_rnr_quad_ext_flowctrl_config2_token_val_2:
    {
        uint32_t max_index = 36;
        uint32_t index = gtmv(m, 5);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(quad_idx, index, &val);

            bdmf_session_print(session, "ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(%u %u %u)\n", quad_idx, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ext_flowctrl_config2_token_val_2 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode:
    {
        uint32_t max_index = 16;
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(quad_idx, index, &val);

            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(%u %u %u)\n", quad_idx, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ubus_decode_cfg_psram_ubus_decode completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode:
    {
        uint32_t max_index = 16;
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(quad_idx, index, &val);

            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(%u %u %u)\n", quad_idx, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ubus_decode_cfg_ddr_ubus_decode completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2:
    {
        uint32_t max_index = 16;
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(quad_idx, index, &val);

            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(%u %u %u)\n", quad_idx, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ubus_decode_cfg2_psram_ubus_decode2 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2:
    {
        uint32_t max_index = 16;
        uint32_t index = gtmv(m, 4);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(%u %u %u)\n", quad_idx, index,
                val);
            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(quad_idx, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(quad_idx, index, &val);

            bdmf_session_print(session, "ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(%u %u %u)\n", quad_idx, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of ubus_decode_cfg2_ddr_ubus_decode2 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_rnr_quad_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_parser_core_configuration_eng: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_ENG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_parser_misc_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_0_1: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_0_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_2_3: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_2_3); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_4_5: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_4_5); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_vid_6_7: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_VID_6_7); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter0_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter1_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter0_mask_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filter1_mask_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ip_filters_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_snap_org_code: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ppp_ip_prot_code: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_ethtype: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ethtype_0_1: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ethtype_2_3: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ethtype_config: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_nest: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_NEST); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_hard_nest_0: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_hard_nest_1: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_qtag_hard_nest_2: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_user_ip_prot: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_USER_IP_PROT); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt2_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt2_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt3_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt3_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt4_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt4_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt5_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt5_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt6_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt6_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt7_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt7_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt8_val_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt8_val_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_mask_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt0_mask_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_mask_l: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt1_mask_h: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt_valid_cfg_0: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt_valid_cfg_1: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_da_filt_valid_cfg_2: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_gre_protocol_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_prop_tag_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_PROP_TAG_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_dos_attack: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_DOS_ATTACK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_icmp_max_size: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_parser_core_configuration_key_cfg: reg = &RU_REG(RNR_QUAD, PARSER_CORE_CONFIGURATION_KEY_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_dma_arb_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DMA_ARB_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram0_base: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM0_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram1_base: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM1_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram2_base: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM2_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram3_base: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM3_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr0_base: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR0_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr1_base: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR1_BASE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram0_mask: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM0_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram1_mask: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM1_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram2_mask: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM2_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_psram3_mask: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PSRAM3_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr0_mask: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR0_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ddr1_mask: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DDR1_MASK); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_profiling_config: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_PROFILING_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_0_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_0_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_1_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_1_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_2_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_2_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_3_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_3_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_4_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_4_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_5_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_5_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_6_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_6_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_7_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_7_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_bkpt_gen_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_BKPT_GEN_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_powersave_config: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_POWERSAVE_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_powersave_status: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_POWERSAVE_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_data_bkpt_0_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_0_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_data_bkpt_1_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_1_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_data_bkpt_2_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_2_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_data_bkpt_3_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_3_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_data_bkpt_common_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_DATA_BKPT_COMMON_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ubus_counter_control: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_UBUS_COUNTER_CONTROL); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_ubus_down_counter: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_UBUS_DOWN_COUNTER); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_all_xfers_cnt: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_ALL_XFERS_CNT); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_read_xfers_cnt: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_READ_XFERS_CNT); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_read_data_cnt: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_READ_DATA_CNT); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_write_data_cnt: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_WRITE_DATA_CNT); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_misc_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_MISC_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_aqm_control: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_AQM_CONTROL); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_aqm_randm_value: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_AQM_RANDM_VALUE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_aqm_random_seed: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_SEED); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_aqm_random_test_inc: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_AQM_RANDOM_TEST_INC); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_general_config_multi_psel_cfg: reg = &RU_REG(RNR_QUAD, GENERAL_CONFIG_MULTI_PSEL_CFG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_fifo_config: reg = &RU_REG(RNR_QUAD, DEBUG_FIFO_CONFIG); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_hdr_fifo_status: reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_data_fifo_status: reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_hdr_fifo_status: reg = &RU_REG(RNR_QUAD, DEBUG_DDR_HDR_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_data_fifo_status: reg = &RU_REG(RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_data_fifo_status2: reg = &RU_REG(RNR_QUAD, DEBUG_DDR_DATA_FIFO_STATUS2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_hdr_fifo_data1: reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_hdr_fifo_data2: reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_HDR_FIFO_DATA2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_data_fifo_data1: reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_psram_data_fifo_data2: reg = &RU_REG(RNR_QUAD, DEBUG_PSRAM_DATA_FIFO_DATA2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_hdr_fifo_data1: reg = &RU_REG(RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA1); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_debug_ddr_hdr_fifo_data2: reg = &RU_REG(RNR_QUAD, DEBUG_DDR_HDR_FIFO_DATA2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ext_flowctrl_config_token_val: reg = &RU_REG(RNR_QUAD, EXT_FLOWCTRL_CONFIG_TOKEN_VAL); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ext_flowctrl_config2_token_val_2: reg = &RU_REG(RNR_QUAD, EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ubus_decode_cfg_psram_ubus_decode: reg = &RU_REG(RNR_QUAD, UBUS_DECODE_CFG_PSRAM_UBUS_DECODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ubus_decode_cfg_ddr_ubus_decode: reg = &RU_REG(RNR_QUAD, UBUS_DECODE_CFG_DDR_UBUS_DECODE); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ubus_decode_cfg2_psram_ubus_decode2: reg = &RU_REG(RNR_QUAD, UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2); blk = &RU_BLK(RNR_QUAD); break;
    case bdmf_address_ubus_decode_cfg2_ddr_ubus_decode2: reg = &RU_REG(RNR_QUAD, UBUS_DECODE_CFG2_DDR_UBUS_DECODE2); blk = &RU_BLK(RNR_QUAD); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_rnr_quad_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "rnr_quad", "rnr_quad", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_parser_vid0[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_0", "vid_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_0_en", "vid_0_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid1[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_1", "vid_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_1_en", "vid_1_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_2", "vid_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_2_en", "vid_2_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid3[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_3", "vid_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_3_en", "vid_3_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid4[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_4", "vid_4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_4_en", "vid_4_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid5[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_5", "vid_5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_5_en", "vid_5_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid6[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_6", "vid_6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_6_en", "vid_6_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_vid7[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_7", "vid_7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("vid_7_en", "vid_7_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip0[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ip_address", "ip_address", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ip_address_mask", "ip_address_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ip_filter0_dip_en", "ip_filter0_dip_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ip_filter0_valid", "ip_filter0_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_hardcoded_ethtype_prof0[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hard_nest_profile", "hard_nest_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_prof0[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_0", "qtag_nest_0_profile_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_1_profile_0", "qtag_nest_1_profile_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_prof1[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_1", "qtag_nest_0_profile_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_1_profile_1", "qtag_nest_1_profile_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_prof2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_2", "qtag_nest_0_profile_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_1_profile_2", "qtag_nest_1_profile_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_qtag_nest_max_vlans[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qtag_nest_0_profile_2", "qtag_nest_0_profile_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_num_of_vlans", "max_num_of_vlans", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol0[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_0", "user_ip_prot_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol1[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_1", "user_ip_prot_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_2", "user_ip_prot_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ip_protocol3[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("user_ip_prot_3", "user_ip_prot_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_mask_msb", "da_filt_mask_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_mask_l", "da_filt_mask_l", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter3[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter4[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter5[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter6[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter7[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_da_filter8[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_msb", "da_filt_msb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt_lsb", "da_filt_lsb", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_da_filter_valid[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt0_valid", "da_filt0_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt1_valid", "da_filt1_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt2_valid", "da_filt2_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt3_valid", "da_filt3_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt4_valid", "da_filt4_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt5_valid", "da_filt5_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt6_valid", "da_filt6_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt7_valid", "da_filt7_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("da_filt8_valid", "da_filt8_valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_exception_bits[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exception_en", "exception_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcp_flags[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tcp_flags_filt", "tcp_flags_filt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profile_us[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_us", "profile_us", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disable_l2tp_source_port[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable_l2tp_source_port_check", "disable_l2tp_source_port_check", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_snap_conf[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("code", "code", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_rfc1042", "en_rfc1042", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_8021q", "en_8021q", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_ipv6_filter[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hop_by_hop_match", "hop_by_hop_match", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("routing_eh", "routing_eh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dest_opt_eh", "dest_opt_eh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_eng[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg", "cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_ppp_ip_prot_code[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ppp_code_0", "ppp_code_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ppp_code_1", "ppp_code_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_qtag_ethtype[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_qtag_0", "ethtype_qtag_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_qtag_1", "ethtype_qtag_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_user_ethtype_0_1[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethype_0", "ethype_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethype_1", "ethype_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_user_ethtype_2_3[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethype_2", "ethype_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethype_3", "ethype_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_user_ethtype_config[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_0", "ethtype_user_prot_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_1", "ethtype_user_prot_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_2", "ethtype_user_prot_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_prot_3", "ethtype_user_prot_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_en", "ethtype_user_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_0", "ethtype_user_offset_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_1", "ethtype_user_offset_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_2", "ethtype_user_offset_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ethtype_user_offset_3", "ethtype_user_offset_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_gre_protocol_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gre_protocol", "gre_protocol", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_prop_tag_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_profile_0", "size_profile_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_profile_1", "size_profile_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size_profile_2", "size_profile_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pre_da_dprofile_0", "pre_da_dprofile_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pre_da_dprofile_1", "pre_da_dprofile_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pre_da_dprofile_2", "pre_da_dprofile_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_dos_attack[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_icmp_max_size[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("v4_size", "v4_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("v6_size", "v6_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_parser_core_configuration_key_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("l2_tos_mask", "l2_tos_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("l3_tos_mask", "l3_tos_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("l2_exclude_smac", "l2_exclude_smac", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tcp_pure_ack_mask", "tcp_pure_ack_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("incude_dei_in_vlans_crc", "incude_dei_in_vlans_crc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_size", "key_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_num_of_vlans_in_crc", "max_num_of_vlans_in_crc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("l3_tcp_pure_ack_mask", "l3_tcp_pure_ack_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rsrv", "rsrv", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_dma_arb_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("use_fifo_for_ddr_only", "use_fifo_for_ddr_only", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token_arbiter_is_rr", "token_arbiter_is_rr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chicken_no_flowctrl", "chicken_no_flowctrl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow_ctrl_clear_token", "flow_ctrl_clear_token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_congest_threshold", "ddr_congest_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_congest_threshold", "psram_congest_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_reply_threshold", "enable_reply_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_reply_threshold", "ddr_reply_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_reply_threshold", "psram_reply_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram0_base[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram1_base[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram2_base[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram3_base[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr0_base[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr1_base[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram0_mask[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram1_mask[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram2_mask[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_psram3_mask[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr0_mask[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ddr1_mask[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_profiling_config[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counter_lsb_sel", "counter_lsb_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_0", "enable_trace_core_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_1", "enable_trace_core_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_2", "enable_trace_core_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_3", "enable_trace_core_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_4", "enable_trace_core_4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_5", "enable_trace_core_5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_6", "enable_trace_core_6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_7", "enable_trace_core_7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_8", "enable_trace_core_8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_9", "enable_trace_core_9", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_10", "enable_trace_core_10", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_11", "enable_trace_core_11", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_12", "enable_trace_core_12", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_trace_core_13", "enable_trace_core_13", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_0_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_1_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_2_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_3_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_4_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_5_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_6_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_7_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread", "thread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_bkpt_gen_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("handler_addr", "handler_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("update_pc_value", "update_pc_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_powersave_config[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("time_counter", "time_counter", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_0", "enable_powersave_core_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_1", "enable_powersave_core_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_2", "enable_powersave_core_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_3", "enable_powersave_core_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_4", "enable_powersave_core_4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_5", "enable_powersave_core_5", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_6", "enable_powersave_core_6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_7", "enable_powersave_core_7", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_8", "enable_powersave_core_8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_9", "enable_powersave_core_9", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_10", "enable_powersave_core_10", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_11", "enable_powersave_core_11", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_12", "enable_powersave_core_12", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_powersave_core_13", "enable_powersave_core_13", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_cpu_if_clk_gating", "enable_cpu_if_clk_gating", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_common_reg_clk_gating", "enable_common_reg_clk_gating", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_ec_blocks_clk_gating", "enable_ec_blocks_clk_gating", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_data_bkpt_0_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_start", "data_addr_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_stop", "data_addr_stop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_data_bkpt_1_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_start", "data_addr_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_stop", "data_addr_stop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_data_bkpt_2_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_start", "data_addr_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_stop", "data_addr_stop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_data_bkpt_3_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_start", "data_addr_start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr_stop", "data_addr_stop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_data_bkpt_common_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread_0", "thread_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread_1", "thread_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread_2", "thread_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thread_3", "thread_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ubus_counter_control[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_statistics", "enable_statistics", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sw_reset", "sw_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dest_pid", "dest_pid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("master_select", "master_select", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_ubus_down_counter[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("downcnt_value", "downcnt_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_misc_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pid", "ddr_pid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_aqm_control[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_counter", "enable_counter", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable_random", "enable_random", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_aqm_random_seed[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("random_seed", "random_seed", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_aqm_random_test_inc[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("random_inc", "random_inc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_config_multi_psel_cfg[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_psel_master_sel", "multi_psel_master_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_psel_mask", "multi_psel_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_fifo_config[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_hdr_sw_rst_0", "psram_hdr_sw_rst_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_data_sw_rst_0", "psram_data_sw_rst_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hdr_sw_rst_0", "ddr_hdr_sw_rst_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("select_fifos_for_debug", "select_fifos_for_debug", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_hdr_sw_rst_1", "psram_hdr_sw_rst_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_data_sw_rst_1", "psram_data_sw_rst_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hdr_sw_rst_1", "ddr_hdr_sw_rst_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_hdr_sw_rd_addr", "psram_hdr_sw_rd_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_data_sw_rd_addr", "psram_data_sw_rd_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_hdr_sw_rd_addr", "ddr_hdr_sw_rd_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ext_flowctrl_config_token_val[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ext_flowctrl_config2_token_val_2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg_psram_ubus_decode[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg_ddr_ubus_decode[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg2_psram_ubus_decode2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_decode_cfg2_ddr_ubus_decode2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "parser_vid0", .val = cli_rnr_quad_parser_vid0, .parms = set_parser_vid0 },
            { .name = "parser_vid1", .val = cli_rnr_quad_parser_vid1, .parms = set_parser_vid1 },
            { .name = "parser_vid2", .val = cli_rnr_quad_parser_vid2, .parms = set_parser_vid2 },
            { .name = "parser_vid3", .val = cli_rnr_quad_parser_vid3, .parms = set_parser_vid3 },
            { .name = "parser_vid4", .val = cli_rnr_quad_parser_vid4, .parms = set_parser_vid4 },
            { .name = "parser_vid5", .val = cli_rnr_quad_parser_vid5, .parms = set_parser_vid5 },
            { .name = "parser_vid6", .val = cli_rnr_quad_parser_vid6, .parms = set_parser_vid6 },
            { .name = "parser_vid7", .val = cli_rnr_quad_parser_vid7, .parms = set_parser_vid7 },
            { .name = "parser_ip0", .val = cli_rnr_quad_parser_ip0, .parms = set_parser_ip0 },
            { .name = "parser_ip1", .val = cli_rnr_quad_parser_ip1, .parms = set_parser_ip0 },
            { .name = "parser_hardcoded_ethtype_prof0", .val = cli_rnr_quad_parser_hardcoded_ethtype_prof0, .parms = set_parser_hardcoded_ethtype_prof0 },
            { .name = "parser_hardcoded_ethtype_prof1", .val = cli_rnr_quad_parser_hardcoded_ethtype_prof1, .parms = set_parser_hardcoded_ethtype_prof0 },
            { .name = "parser_hardcoded_ethtype_prof2", .val = cli_rnr_quad_parser_hardcoded_ethtype_prof2, .parms = set_parser_hardcoded_ethtype_prof0 },
            { .name = "parser_qtag_nest_prof0", .val = cli_rnr_quad_parser_qtag_nest_prof0, .parms = set_parser_qtag_nest_prof0 },
            { .name = "parser_qtag_nest_prof1", .val = cli_rnr_quad_parser_qtag_nest_prof1, .parms = set_parser_qtag_nest_prof1 },
            { .name = "parser_qtag_nest_prof2", .val = cli_rnr_quad_parser_qtag_nest_prof2, .parms = set_parser_qtag_nest_prof2 },
            { .name = "parser_qtag_nest_max_vlans", .val = cli_rnr_quad_parser_qtag_nest_max_vlans, .parms = set_parser_qtag_nest_max_vlans },
            { .name = "parser_ip_protocol0", .val = cli_rnr_quad_parser_ip_protocol0, .parms = set_parser_ip_protocol0 },
            { .name = "parser_ip_protocol1", .val = cli_rnr_quad_parser_ip_protocol1, .parms = set_parser_ip_protocol1 },
            { .name = "parser_ip_protocol2", .val = cli_rnr_quad_parser_ip_protocol2, .parms = set_parser_ip_protocol2 },
            { .name = "parser_ip_protocol3", .val = cli_rnr_quad_parser_ip_protocol3, .parms = set_parser_ip_protocol3 },
            { .name = "parser_da_filter", .val = cli_rnr_quad_parser_da_filter, .parms = set_parser_da_filter },
            { .name = "parser_da_filter1", .val = cli_rnr_quad_parser_da_filter1, .parms = set_parser_da_filter },
            { .name = "parser_da_filter2", .val = cli_rnr_quad_parser_da_filter2, .parms = set_parser_da_filter2 },
            { .name = "parser_da_filter3", .val = cli_rnr_quad_parser_da_filter3, .parms = set_parser_da_filter3 },
            { .name = "parser_da_filter4", .val = cli_rnr_quad_parser_da_filter4, .parms = set_parser_da_filter4 },
            { .name = "parser_da_filter5", .val = cli_rnr_quad_parser_da_filter5, .parms = set_parser_da_filter5 },
            { .name = "parser_da_filter6", .val = cli_rnr_quad_parser_da_filter6, .parms = set_parser_da_filter6 },
            { .name = "parser_da_filter7", .val = cli_rnr_quad_parser_da_filter7, .parms = set_parser_da_filter7 },
            { .name = "parser_da_filter8", .val = cli_rnr_quad_parser_da_filter8, .parms = set_parser_da_filter8 },
            { .name = "da_filter_valid", .val = cli_rnr_quad_da_filter_valid, .parms = set_da_filter_valid },
            { .name = "exception_bits", .val = cli_rnr_quad_exception_bits, .parms = set_exception_bits },
            { .name = "tcp_flags", .val = cli_rnr_quad_tcp_flags, .parms = set_tcp_flags },
            { .name = "profile_us", .val = cli_rnr_quad_profile_us, .parms = set_profile_us },
            { .name = "disable_l2tp_source_port", .val = cli_rnr_quad_disable_l2tp_source_port, .parms = set_disable_l2tp_source_port },
            { .name = "parser_snap_conf", .val = cli_rnr_quad_parser_snap_conf, .parms = set_parser_snap_conf },
            { .name = "parser_ipv6_filter", .val = cli_rnr_quad_parser_ipv6_filter, .parms = set_parser_ipv6_filter },
            { .name = "parser_core_configuration_eng", .val = cli_rnr_quad_parser_core_configuration_eng, .parms = set_parser_core_configuration_eng },
            { .name = "parser_core_configuration_ppp_ip_prot_code", .val = cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code, .parms = set_parser_core_configuration_ppp_ip_prot_code },
            { .name = "parser_core_configuration_qtag_ethtype", .val = cli_rnr_quad_parser_core_configuration_qtag_ethtype, .parms = set_parser_core_configuration_qtag_ethtype },
            { .name = "parser_core_configuration_user_ethtype_0_1", .val = cli_rnr_quad_parser_core_configuration_user_ethtype_0_1, .parms = set_parser_core_configuration_user_ethtype_0_1 },
            { .name = "parser_core_configuration_user_ethtype_2_3", .val = cli_rnr_quad_parser_core_configuration_user_ethtype_2_3, .parms = set_parser_core_configuration_user_ethtype_2_3 },
            { .name = "parser_core_configuration_user_ethtype_config", .val = cli_rnr_quad_parser_core_configuration_user_ethtype_config, .parms = set_parser_core_configuration_user_ethtype_config },
            { .name = "parser_core_configuration_da_filt_valid_cfg_1", .val = cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1, .parms = set_da_filter_valid },
            { .name = "parser_core_configuration_da_filt_valid_cfg_2", .val = cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2, .parms = set_da_filter_valid },
            { .name = "parser_core_configuration_gre_protocol_cfg", .val = cli_rnr_quad_parser_core_configuration_gre_protocol_cfg, .parms = set_parser_core_configuration_gre_protocol_cfg },
            { .name = "parser_core_configuration_prop_tag_cfg", .val = cli_rnr_quad_parser_core_configuration_prop_tag_cfg, .parms = set_parser_core_configuration_prop_tag_cfg },
            { .name = "parser_core_configuration_dos_attack", .val = cli_rnr_quad_parser_core_configuration_dos_attack, .parms = set_parser_core_configuration_dos_attack },
            { .name = "parser_core_configuration_icmp_max_size", .val = cli_rnr_quad_parser_core_configuration_icmp_max_size, .parms = set_parser_core_configuration_icmp_max_size },
            { .name = "parser_core_configuration_key_cfg", .val = cli_rnr_quad_parser_core_configuration_key_cfg, .parms = set_parser_core_configuration_key_cfg },
            { .name = "general_config_dma_arb_cfg", .val = cli_rnr_quad_general_config_dma_arb_cfg, .parms = set_general_config_dma_arb_cfg },
            { .name = "general_config_psram0_base", .val = cli_rnr_quad_general_config_psram0_base, .parms = set_general_config_psram0_base },
            { .name = "general_config_psram1_base", .val = cli_rnr_quad_general_config_psram1_base, .parms = set_general_config_psram1_base },
            { .name = "general_config_psram2_base", .val = cli_rnr_quad_general_config_psram2_base, .parms = set_general_config_psram2_base },
            { .name = "general_config_psram3_base", .val = cli_rnr_quad_general_config_psram3_base, .parms = set_general_config_psram3_base },
            { .name = "general_config_ddr0_base", .val = cli_rnr_quad_general_config_ddr0_base, .parms = set_general_config_ddr0_base },
            { .name = "general_config_ddr1_base", .val = cli_rnr_quad_general_config_ddr1_base, .parms = set_general_config_ddr1_base },
            { .name = "general_config_psram0_mask", .val = cli_rnr_quad_general_config_psram0_mask, .parms = set_general_config_psram0_mask },
            { .name = "general_config_psram1_mask", .val = cli_rnr_quad_general_config_psram1_mask, .parms = set_general_config_psram1_mask },
            { .name = "general_config_psram2_mask", .val = cli_rnr_quad_general_config_psram2_mask, .parms = set_general_config_psram2_mask },
            { .name = "general_config_psram3_mask", .val = cli_rnr_quad_general_config_psram3_mask, .parms = set_general_config_psram3_mask },
            { .name = "general_config_ddr0_mask", .val = cli_rnr_quad_general_config_ddr0_mask, .parms = set_general_config_ddr0_mask },
            { .name = "general_config_ddr1_mask", .val = cli_rnr_quad_general_config_ddr1_mask, .parms = set_general_config_ddr1_mask },
            { .name = "general_config_profiling_config", .val = cli_rnr_quad_general_config_profiling_config, .parms = set_general_config_profiling_config },
            { .name = "general_config_bkpt_0_cfg", .val = cli_rnr_quad_general_config_bkpt_0_cfg, .parms = set_general_config_bkpt_0_cfg },
            { .name = "general_config_bkpt_1_cfg", .val = cli_rnr_quad_general_config_bkpt_1_cfg, .parms = set_general_config_bkpt_1_cfg },
            { .name = "general_config_bkpt_2_cfg", .val = cli_rnr_quad_general_config_bkpt_2_cfg, .parms = set_general_config_bkpt_2_cfg },
            { .name = "general_config_bkpt_3_cfg", .val = cli_rnr_quad_general_config_bkpt_3_cfg, .parms = set_general_config_bkpt_3_cfg },
            { .name = "general_config_bkpt_4_cfg", .val = cli_rnr_quad_general_config_bkpt_4_cfg, .parms = set_general_config_bkpt_4_cfg },
            { .name = "general_config_bkpt_5_cfg", .val = cli_rnr_quad_general_config_bkpt_5_cfg, .parms = set_general_config_bkpt_5_cfg },
            { .name = "general_config_bkpt_6_cfg", .val = cli_rnr_quad_general_config_bkpt_6_cfg, .parms = set_general_config_bkpt_6_cfg },
            { .name = "general_config_bkpt_7_cfg", .val = cli_rnr_quad_general_config_bkpt_7_cfg, .parms = set_general_config_bkpt_7_cfg },
            { .name = "general_config_bkpt_gen_cfg", .val = cli_rnr_quad_general_config_bkpt_gen_cfg, .parms = set_general_config_bkpt_gen_cfg },
            { .name = "general_config_powersave_config", .val = cli_rnr_quad_general_config_powersave_config, .parms = set_general_config_powersave_config },
            { .name = "general_config_data_bkpt_0_cfg", .val = cli_rnr_quad_general_config_data_bkpt_0_cfg, .parms = set_general_config_data_bkpt_0_cfg },
            { .name = "general_config_data_bkpt_1_cfg", .val = cli_rnr_quad_general_config_data_bkpt_1_cfg, .parms = set_general_config_data_bkpt_1_cfg },
            { .name = "general_config_data_bkpt_2_cfg", .val = cli_rnr_quad_general_config_data_bkpt_2_cfg, .parms = set_general_config_data_bkpt_2_cfg },
            { .name = "general_config_data_bkpt_3_cfg", .val = cli_rnr_quad_general_config_data_bkpt_3_cfg, .parms = set_general_config_data_bkpt_3_cfg },
            { .name = "general_config_data_bkpt_common_cfg", .val = cli_rnr_quad_general_config_data_bkpt_common_cfg, .parms = set_general_config_data_bkpt_common_cfg },
            { .name = "general_config_ubus_counter_control", .val = cli_rnr_quad_general_config_ubus_counter_control, .parms = set_general_config_ubus_counter_control },
            { .name = "general_config_ubus_down_counter", .val = cli_rnr_quad_general_config_ubus_down_counter, .parms = set_general_config_ubus_down_counter },
            { .name = "general_config_misc_cfg", .val = cli_rnr_quad_general_config_misc_cfg, .parms = set_general_config_misc_cfg },
            { .name = "general_config_aqm_control", .val = cli_rnr_quad_general_config_aqm_control, .parms = set_general_config_aqm_control },
            { .name = "general_config_aqm_random_seed", .val = cli_rnr_quad_general_config_aqm_random_seed, .parms = set_general_config_aqm_random_seed },
            { .name = "general_config_aqm_random_test_inc", .val = cli_rnr_quad_general_config_aqm_random_test_inc, .parms = set_general_config_aqm_random_test_inc },
            { .name = "general_config_multi_psel_cfg", .val = cli_rnr_quad_general_config_multi_psel_cfg, .parms = set_general_config_multi_psel_cfg },
            { .name = "debug_fifo_config", .val = cli_rnr_quad_debug_fifo_config, .parms = set_debug_fifo_config },
            { .name = "ext_flowctrl_config_token_val", .val = cli_rnr_quad_ext_flowctrl_config_token_val, .parms = set_ext_flowctrl_config_token_val },
            { .name = "ext_flowctrl_config2_token_val_2", .val = cli_rnr_quad_ext_flowctrl_config2_token_val_2, .parms = set_ext_flowctrl_config2_token_val_2 },
            { .name = "ubus_decode_cfg_psram_ubus_decode", .val = cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode, .parms = set_ubus_decode_cfg_psram_ubus_decode },
            { .name = "ubus_decode_cfg_ddr_ubus_decode", .val = cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode, .parms = set_ubus_decode_cfg_ddr_ubus_decode },
            { .name = "ubus_decode_cfg2_psram_ubus_decode2", .val = cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2, .parms = set_ubus_decode_cfg2_psram_ubus_decode2 },
            { .name = "ubus_decode_cfg2_ddr_ubus_decode2", .val = cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2, .parms = set_ubus_decode_cfg2_ddr_ubus_decode2 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_rnr_quad_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ext_flowctrl_config_token_val[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ext_flowctrl_config2_token_val_2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ubus_decode_cfg_psram_ubus_decode[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ubus_decode_cfg_ddr_ubus_decode[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ubus_decode_cfg2_psram_ubus_decode2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ubus_decode_cfg2_ddr_ubus_decode2[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "parser_vid0", .val = cli_rnr_quad_parser_vid0, .parms = get_default },
            { .name = "parser_vid1", .val = cli_rnr_quad_parser_vid1, .parms = get_default },
            { .name = "parser_vid2", .val = cli_rnr_quad_parser_vid2, .parms = get_default },
            { .name = "parser_vid3", .val = cli_rnr_quad_parser_vid3, .parms = get_default },
            { .name = "parser_vid4", .val = cli_rnr_quad_parser_vid4, .parms = get_default },
            { .name = "parser_vid5", .val = cli_rnr_quad_parser_vid5, .parms = get_default },
            { .name = "parser_vid6", .val = cli_rnr_quad_parser_vid6, .parms = get_default },
            { .name = "parser_vid7", .val = cli_rnr_quad_parser_vid7, .parms = get_default },
            { .name = "parser_ip0", .val = cli_rnr_quad_parser_ip0, .parms = get_default },
            { .name = "parser_ip1", .val = cli_rnr_quad_parser_ip1, .parms = get_default },
            { .name = "parser_hardcoded_ethtype_prof0", .val = cli_rnr_quad_parser_hardcoded_ethtype_prof0, .parms = get_default },
            { .name = "parser_hardcoded_ethtype_prof1", .val = cli_rnr_quad_parser_hardcoded_ethtype_prof1, .parms = get_default },
            { .name = "parser_hardcoded_ethtype_prof2", .val = cli_rnr_quad_parser_hardcoded_ethtype_prof2, .parms = get_default },
            { .name = "parser_qtag_nest_prof0", .val = cli_rnr_quad_parser_qtag_nest_prof0, .parms = get_default },
            { .name = "parser_qtag_nest_prof1", .val = cli_rnr_quad_parser_qtag_nest_prof1, .parms = get_default },
            { .name = "parser_qtag_nest_prof2", .val = cli_rnr_quad_parser_qtag_nest_prof2, .parms = get_default },
            { .name = "parser_qtag_nest_max_vlans", .val = cli_rnr_quad_parser_qtag_nest_max_vlans, .parms = get_default },
            { .name = "parser_ip_protocol0", .val = cli_rnr_quad_parser_ip_protocol0, .parms = get_default },
            { .name = "parser_ip_protocol1", .val = cli_rnr_quad_parser_ip_protocol1, .parms = get_default },
            { .name = "parser_ip_protocol2", .val = cli_rnr_quad_parser_ip_protocol2, .parms = get_default },
            { .name = "parser_ip_protocol3", .val = cli_rnr_quad_parser_ip_protocol3, .parms = get_default },
            { .name = "parser_da_filter", .val = cli_rnr_quad_parser_da_filter, .parms = get_default },
            { .name = "parser_da_filter1", .val = cli_rnr_quad_parser_da_filter1, .parms = get_default },
            { .name = "parser_da_filter2", .val = cli_rnr_quad_parser_da_filter2, .parms = get_default },
            { .name = "parser_da_filter3", .val = cli_rnr_quad_parser_da_filter3, .parms = get_default },
            { .name = "parser_da_filter4", .val = cli_rnr_quad_parser_da_filter4, .parms = get_default },
            { .name = "parser_da_filter5", .val = cli_rnr_quad_parser_da_filter5, .parms = get_default },
            { .name = "parser_da_filter6", .val = cli_rnr_quad_parser_da_filter6, .parms = get_default },
            { .name = "parser_da_filter7", .val = cli_rnr_quad_parser_da_filter7, .parms = get_default },
            { .name = "parser_da_filter8", .val = cli_rnr_quad_parser_da_filter8, .parms = get_default },
            { .name = "da_filter_valid", .val = cli_rnr_quad_da_filter_valid, .parms = get_default },
            { .name = "exception_bits", .val = cli_rnr_quad_exception_bits, .parms = get_default },
            { .name = "tcp_flags", .val = cli_rnr_quad_tcp_flags, .parms = get_default },
            { .name = "profile_us", .val = cli_rnr_quad_profile_us, .parms = get_default },
            { .name = "disable_l2tp_source_port", .val = cli_rnr_quad_disable_l2tp_source_port, .parms = get_default },
            { .name = "parser_snap_conf", .val = cli_rnr_quad_parser_snap_conf, .parms = get_default },
            { .name = "parser_ipv6_filter", .val = cli_rnr_quad_parser_ipv6_filter, .parms = get_default },
            { .name = "parser_core_configuration_eng", .val = cli_rnr_quad_parser_core_configuration_eng, .parms = get_default },
            { .name = "parser_core_configuration_ppp_ip_prot_code", .val = cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code, .parms = get_default },
            { .name = "parser_core_configuration_qtag_ethtype", .val = cli_rnr_quad_parser_core_configuration_qtag_ethtype, .parms = get_default },
            { .name = "parser_core_configuration_user_ethtype_0_1", .val = cli_rnr_quad_parser_core_configuration_user_ethtype_0_1, .parms = get_default },
            { .name = "parser_core_configuration_user_ethtype_2_3", .val = cli_rnr_quad_parser_core_configuration_user_ethtype_2_3, .parms = get_default },
            { .name = "parser_core_configuration_user_ethtype_config", .val = cli_rnr_quad_parser_core_configuration_user_ethtype_config, .parms = get_default },
            { .name = "parser_core_configuration_da_filt_valid_cfg_1", .val = cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1, .parms = get_default },
            { .name = "parser_core_configuration_da_filt_valid_cfg_2", .val = cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2, .parms = get_default },
            { .name = "parser_core_configuration_gre_protocol_cfg", .val = cli_rnr_quad_parser_core_configuration_gre_protocol_cfg, .parms = get_default },
            { .name = "parser_core_configuration_prop_tag_cfg", .val = cli_rnr_quad_parser_core_configuration_prop_tag_cfg, .parms = get_default },
            { .name = "parser_core_configuration_dos_attack", .val = cli_rnr_quad_parser_core_configuration_dos_attack, .parms = get_default },
            { .name = "parser_core_configuration_icmp_max_size", .val = cli_rnr_quad_parser_core_configuration_icmp_max_size, .parms = get_default },
            { .name = "parser_core_configuration_key_cfg", .val = cli_rnr_quad_parser_core_configuration_key_cfg, .parms = get_default },
            { .name = "general_config_dma_arb_cfg", .val = cli_rnr_quad_general_config_dma_arb_cfg, .parms = get_default },
            { .name = "general_config_psram0_base", .val = cli_rnr_quad_general_config_psram0_base, .parms = get_default },
            { .name = "general_config_psram1_base", .val = cli_rnr_quad_general_config_psram1_base, .parms = get_default },
            { .name = "general_config_psram2_base", .val = cli_rnr_quad_general_config_psram2_base, .parms = get_default },
            { .name = "general_config_psram3_base", .val = cli_rnr_quad_general_config_psram3_base, .parms = get_default },
            { .name = "general_config_ddr0_base", .val = cli_rnr_quad_general_config_ddr0_base, .parms = get_default },
            { .name = "general_config_ddr1_base", .val = cli_rnr_quad_general_config_ddr1_base, .parms = get_default },
            { .name = "general_config_psram0_mask", .val = cli_rnr_quad_general_config_psram0_mask, .parms = get_default },
            { .name = "general_config_psram1_mask", .val = cli_rnr_quad_general_config_psram1_mask, .parms = get_default },
            { .name = "general_config_psram2_mask", .val = cli_rnr_quad_general_config_psram2_mask, .parms = get_default },
            { .name = "general_config_psram3_mask", .val = cli_rnr_quad_general_config_psram3_mask, .parms = get_default },
            { .name = "general_config_ddr0_mask", .val = cli_rnr_quad_general_config_ddr0_mask, .parms = get_default },
            { .name = "general_config_ddr1_mask", .val = cli_rnr_quad_general_config_ddr1_mask, .parms = get_default },
            { .name = "general_config_profiling_config", .val = cli_rnr_quad_general_config_profiling_config, .parms = get_default },
            { .name = "general_config_bkpt_0_cfg", .val = cli_rnr_quad_general_config_bkpt_0_cfg, .parms = get_default },
            { .name = "general_config_bkpt_1_cfg", .val = cli_rnr_quad_general_config_bkpt_1_cfg, .parms = get_default },
            { .name = "general_config_bkpt_2_cfg", .val = cli_rnr_quad_general_config_bkpt_2_cfg, .parms = get_default },
            { .name = "general_config_bkpt_3_cfg", .val = cli_rnr_quad_general_config_bkpt_3_cfg, .parms = get_default },
            { .name = "general_config_bkpt_4_cfg", .val = cli_rnr_quad_general_config_bkpt_4_cfg, .parms = get_default },
            { .name = "general_config_bkpt_5_cfg", .val = cli_rnr_quad_general_config_bkpt_5_cfg, .parms = get_default },
            { .name = "general_config_bkpt_6_cfg", .val = cli_rnr_quad_general_config_bkpt_6_cfg, .parms = get_default },
            { .name = "general_config_bkpt_7_cfg", .val = cli_rnr_quad_general_config_bkpt_7_cfg, .parms = get_default },
            { .name = "general_config_bkpt_gen_cfg", .val = cli_rnr_quad_general_config_bkpt_gen_cfg, .parms = get_default },
            { .name = "general_config_powersave_config", .val = cli_rnr_quad_general_config_powersave_config, .parms = get_default },
            { .name = "general_config_powersave_status", .val = cli_rnr_quad_general_config_powersave_status, .parms = get_default },
            { .name = "general_config_data_bkpt_0_cfg", .val = cli_rnr_quad_general_config_data_bkpt_0_cfg, .parms = get_default },
            { .name = "general_config_data_bkpt_1_cfg", .val = cli_rnr_quad_general_config_data_bkpt_1_cfg, .parms = get_default },
            { .name = "general_config_data_bkpt_2_cfg", .val = cli_rnr_quad_general_config_data_bkpt_2_cfg, .parms = get_default },
            { .name = "general_config_data_bkpt_3_cfg", .val = cli_rnr_quad_general_config_data_bkpt_3_cfg, .parms = get_default },
            { .name = "general_config_data_bkpt_common_cfg", .val = cli_rnr_quad_general_config_data_bkpt_common_cfg, .parms = get_default },
            { .name = "general_config_ubus_counter_control", .val = cli_rnr_quad_general_config_ubus_counter_control, .parms = get_default },
            { .name = "general_config_ubus_down_counter", .val = cli_rnr_quad_general_config_ubus_down_counter, .parms = get_default },
            { .name = "general_config_all_xfers_cnt", .val = cli_rnr_quad_general_config_all_xfers_cnt, .parms = get_default },
            { .name = "general_config_read_xfers_cnt", .val = cli_rnr_quad_general_config_read_xfers_cnt, .parms = get_default },
            { .name = "general_config_read_data_cnt", .val = cli_rnr_quad_general_config_read_data_cnt, .parms = get_default },
            { .name = "general_config_write_data_cnt", .val = cli_rnr_quad_general_config_write_data_cnt, .parms = get_default },
            { .name = "general_config_misc_cfg", .val = cli_rnr_quad_general_config_misc_cfg, .parms = get_default },
            { .name = "general_config_aqm_control", .val = cli_rnr_quad_general_config_aqm_control, .parms = get_default },
            { .name = "general_config_aqm_randm_value", .val = cli_rnr_quad_general_config_aqm_randm_value, .parms = get_default },
            { .name = "general_config_aqm_random_seed", .val = cli_rnr_quad_general_config_aqm_random_seed, .parms = get_default },
            { .name = "general_config_multi_psel_cfg", .val = cli_rnr_quad_general_config_multi_psel_cfg, .parms = get_default },
            { .name = "debug_fifo_config", .val = cli_rnr_quad_debug_fifo_config, .parms = get_default },
            { .name = "debug_psram_hdr_fifo_status", .val = cli_rnr_quad_debug_psram_hdr_fifo_status, .parms = get_default },
            { .name = "debug_psram_data_fifo_status", .val = cli_rnr_quad_debug_psram_data_fifo_status, .parms = get_default },
            { .name = "debug_ddr_hdr_fifo_status", .val = cli_rnr_quad_debug_ddr_hdr_fifo_status, .parms = get_default },
            { .name = "debug_ddr_data_fifo_status", .val = cli_rnr_quad_debug_ddr_data_fifo_status, .parms = get_default },
            { .name = "debug_ddr_data_fifo_status2", .val = cli_rnr_quad_debug_ddr_data_fifo_status2, .parms = get_default },
            { .name = "debug_psram_hdr_fifo_data1", .val = cli_rnr_quad_debug_psram_hdr_fifo_data1, .parms = get_default },
            { .name = "debug_psram_hdr_fifo_data2", .val = cli_rnr_quad_debug_psram_hdr_fifo_data2, .parms = get_default },
            { .name = "debug_psram_data_fifo_data1", .val = cli_rnr_quad_debug_psram_data_fifo_data1, .parms = get_default },
            { .name = "debug_psram_data_fifo_data2", .val = cli_rnr_quad_debug_psram_data_fifo_data2, .parms = get_default },
            { .name = "debug_ddr_hdr_fifo_data1", .val = cli_rnr_quad_debug_ddr_hdr_fifo_data1, .parms = get_default },
            { .name = "debug_ddr_hdr_fifo_data2", .val = cli_rnr_quad_debug_ddr_hdr_fifo_data2, .parms = get_default },
            { .name = "ext_flowctrl_config_token_val", .val = cli_rnr_quad_ext_flowctrl_config_token_val, .parms = get_ext_flowctrl_config_token_val },
            { .name = "ext_flowctrl_config2_token_val_2", .val = cli_rnr_quad_ext_flowctrl_config2_token_val_2, .parms = get_ext_flowctrl_config2_token_val_2 },
            { .name = "ubus_decode_cfg_psram_ubus_decode", .val = cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode, .parms = get_ubus_decode_cfg_psram_ubus_decode },
            { .name = "ubus_decode_cfg_ddr_ubus_decode", .val = cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode, .parms = get_ubus_decode_cfg_ddr_ubus_decode },
            { .name = "ubus_decode_cfg2_psram_ubus_decode2", .val = cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2, .parms = get_ubus_decode_cfg2_psram_ubus_decode2 },
            { .name = "ubus_decode_cfg2_ddr_ubus_decode2", .val = cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2, .parms = get_ubus_decode_cfg2_ddr_ubus_decode2 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_rnr_quad_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_rnr_quad_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("quad_idx", "quad_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ext_flowctrl_config_token_val", .val = cli_rnr_quad_ext_flowctrl_config_token_val, .parms = ext_test_default},
            { .name = "ext_flowctrl_config2_token_val_2", .val = cli_rnr_quad_ext_flowctrl_config2_token_val_2, .parms = ext_test_default},
            { .name = "ubus_decode_cfg_psram_ubus_decode", .val = cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode, .parms = ext_test_default},
            { .name = "ubus_decode_cfg_ddr_ubus_decode", .val = cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode, .parms = ext_test_default},
            { .name = "ubus_decode_cfg2_psram_ubus_decode2", .val = cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2, .parms = ext_test_default},
            { .name = "ubus_decode_cfg2_ddr_ubus_decode2", .val = cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_rnr_quad_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "PARSER_CORE_CONFIGURATION_ENG", .val = bdmf_address_parser_core_configuration_eng },
            { .name = "PARSER_CORE_CONFIGURATION_PARSER_MISC_CFG", .val = bdmf_address_parser_core_configuration_parser_misc_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_VID_0_1", .val = bdmf_address_parser_core_configuration_vid_0_1 },
            { .name = "PARSER_CORE_CONFIGURATION_VID_2_3", .val = bdmf_address_parser_core_configuration_vid_2_3 },
            { .name = "PARSER_CORE_CONFIGURATION_VID_4_5", .val = bdmf_address_parser_core_configuration_vid_4_5 },
            { .name = "PARSER_CORE_CONFIGURATION_VID_6_7", .val = bdmf_address_parser_core_configuration_vid_6_7 },
            { .name = "PARSER_CORE_CONFIGURATION_IP_FILTER0_CFG", .val = bdmf_address_parser_core_configuration_ip_filter0_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_IP_FILTER1_CFG", .val = bdmf_address_parser_core_configuration_ip_filter1_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_IP_FILTER0_MASK_CFG", .val = bdmf_address_parser_core_configuration_ip_filter0_mask_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_IP_FILTER1_MASK_CFG", .val = bdmf_address_parser_core_configuration_ip_filter1_mask_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_IP_FILTERS_CFG", .val = bdmf_address_parser_core_configuration_ip_filters_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE", .val = bdmf_address_parser_core_configuration_snap_org_code },
            { .name = "PARSER_CORE_CONFIGURATION_PPP_IP_PROT_CODE", .val = bdmf_address_parser_core_configuration_ppp_ip_prot_code },
            { .name = "PARSER_CORE_CONFIGURATION_QTAG_ETHTYPE", .val = bdmf_address_parser_core_configuration_qtag_ethtype },
            { .name = "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_0_1", .val = bdmf_address_parser_core_configuration_user_ethtype_0_1 },
            { .name = "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_2_3", .val = bdmf_address_parser_core_configuration_user_ethtype_2_3 },
            { .name = "PARSER_CORE_CONFIGURATION_USER_ETHTYPE_CONFIG", .val = bdmf_address_parser_core_configuration_user_ethtype_config },
            { .name = "PARSER_CORE_CONFIGURATION_IPV6_HDR_EXT_FLTR_MASK_CFG", .val = bdmf_address_parser_core_configuration_ipv6_hdr_ext_fltr_mask_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_QTAG_NEST", .val = bdmf_address_parser_core_configuration_qtag_nest },
            { .name = "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_0", .val = bdmf_address_parser_core_configuration_qtag_hard_nest_0 },
            { .name = "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_1", .val = bdmf_address_parser_core_configuration_qtag_hard_nest_1 },
            { .name = "PARSER_CORE_CONFIGURATION_QTAG_HARD_NEST_2", .val = bdmf_address_parser_core_configuration_qtag_hard_nest_2 },
            { .name = "PARSER_CORE_CONFIGURATION_USER_IP_PROT", .val = bdmf_address_parser_core_configuration_user_ip_prot },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt0_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT0_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt0_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt1_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT1_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt1_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt2_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT2_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt2_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt3_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT3_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt3_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt4_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT4_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt4_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt5_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT5_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt5_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt6_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT6_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt6_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt7_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT7_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt7_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_L", .val = bdmf_address_parser_core_configuration_da_filt8_val_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT8_VAL_H", .val = bdmf_address_parser_core_configuration_da_filt8_val_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_L", .val = bdmf_address_parser_core_configuration_da_filt0_mask_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT0_MASK_H", .val = bdmf_address_parser_core_configuration_da_filt0_mask_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_L", .val = bdmf_address_parser_core_configuration_da_filt1_mask_l },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT1_MASK_H", .val = bdmf_address_parser_core_configuration_da_filt1_mask_h },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_0", .val = bdmf_address_parser_core_configuration_da_filt_valid_cfg_0 },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_1", .val = bdmf_address_parser_core_configuration_da_filt_valid_cfg_1 },
            { .name = "PARSER_CORE_CONFIGURATION_DA_FILT_VALID_CFG_2", .val = bdmf_address_parser_core_configuration_da_filt_valid_cfg_2 },
            { .name = "PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG", .val = bdmf_address_parser_core_configuration_gre_protocol_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_PROP_TAG_CFG", .val = bdmf_address_parser_core_configuration_prop_tag_cfg },
            { .name = "PARSER_CORE_CONFIGURATION_DOS_ATTACK", .val = bdmf_address_parser_core_configuration_dos_attack },
            { .name = "PARSER_CORE_CONFIGURATION_ICMP_MAX_SIZE", .val = bdmf_address_parser_core_configuration_icmp_max_size },
            { .name = "PARSER_CORE_CONFIGURATION_KEY_CFG", .val = bdmf_address_parser_core_configuration_key_cfg },
            { .name = "GENERAL_CONFIG_DMA_ARB_CFG", .val = bdmf_address_general_config_dma_arb_cfg },
            { .name = "GENERAL_CONFIG_PSRAM0_BASE", .val = bdmf_address_general_config_psram0_base },
            { .name = "GENERAL_CONFIG_PSRAM1_BASE", .val = bdmf_address_general_config_psram1_base },
            { .name = "GENERAL_CONFIG_PSRAM2_BASE", .val = bdmf_address_general_config_psram2_base },
            { .name = "GENERAL_CONFIG_PSRAM3_BASE", .val = bdmf_address_general_config_psram3_base },
            { .name = "GENERAL_CONFIG_DDR0_BASE", .val = bdmf_address_general_config_ddr0_base },
            { .name = "GENERAL_CONFIG_DDR1_BASE", .val = bdmf_address_general_config_ddr1_base },
            { .name = "GENERAL_CONFIG_PSRAM0_MASK", .val = bdmf_address_general_config_psram0_mask },
            { .name = "GENERAL_CONFIG_PSRAM1_MASK", .val = bdmf_address_general_config_psram1_mask },
            { .name = "GENERAL_CONFIG_PSRAM2_MASK", .val = bdmf_address_general_config_psram2_mask },
            { .name = "GENERAL_CONFIG_PSRAM3_MASK", .val = bdmf_address_general_config_psram3_mask },
            { .name = "GENERAL_CONFIG_DDR0_MASK", .val = bdmf_address_general_config_ddr0_mask },
            { .name = "GENERAL_CONFIG_DDR1_MASK", .val = bdmf_address_general_config_ddr1_mask },
            { .name = "GENERAL_CONFIG_PROFILING_CONFIG", .val = bdmf_address_general_config_profiling_config },
            { .name = "GENERAL_CONFIG_BKPT_0_CFG", .val = bdmf_address_general_config_bkpt_0_cfg },
            { .name = "GENERAL_CONFIG_BKPT_1_CFG", .val = bdmf_address_general_config_bkpt_1_cfg },
            { .name = "GENERAL_CONFIG_BKPT_2_CFG", .val = bdmf_address_general_config_bkpt_2_cfg },
            { .name = "GENERAL_CONFIG_BKPT_3_CFG", .val = bdmf_address_general_config_bkpt_3_cfg },
            { .name = "GENERAL_CONFIG_BKPT_4_CFG", .val = bdmf_address_general_config_bkpt_4_cfg },
            { .name = "GENERAL_CONFIG_BKPT_5_CFG", .val = bdmf_address_general_config_bkpt_5_cfg },
            { .name = "GENERAL_CONFIG_BKPT_6_CFG", .val = bdmf_address_general_config_bkpt_6_cfg },
            { .name = "GENERAL_CONFIG_BKPT_7_CFG", .val = bdmf_address_general_config_bkpt_7_cfg },
            { .name = "GENERAL_CONFIG_BKPT_GEN_CFG", .val = bdmf_address_general_config_bkpt_gen_cfg },
            { .name = "GENERAL_CONFIG_POWERSAVE_CONFIG", .val = bdmf_address_general_config_powersave_config },
            { .name = "GENERAL_CONFIG_POWERSAVE_STATUS", .val = bdmf_address_general_config_powersave_status },
            { .name = "GENERAL_CONFIG_DATA_BKPT_0_CFG", .val = bdmf_address_general_config_data_bkpt_0_cfg },
            { .name = "GENERAL_CONFIG_DATA_BKPT_1_CFG", .val = bdmf_address_general_config_data_bkpt_1_cfg },
            { .name = "GENERAL_CONFIG_DATA_BKPT_2_CFG", .val = bdmf_address_general_config_data_bkpt_2_cfg },
            { .name = "GENERAL_CONFIG_DATA_BKPT_3_CFG", .val = bdmf_address_general_config_data_bkpt_3_cfg },
            { .name = "GENERAL_CONFIG_DATA_BKPT_COMMON_CFG", .val = bdmf_address_general_config_data_bkpt_common_cfg },
            { .name = "GENERAL_CONFIG_UBUS_COUNTER_CONTROL", .val = bdmf_address_general_config_ubus_counter_control },
            { .name = "GENERAL_CONFIG_UBUS_DOWN_COUNTER", .val = bdmf_address_general_config_ubus_down_counter },
            { .name = "GENERAL_CONFIG_ALL_XFERS_CNT", .val = bdmf_address_general_config_all_xfers_cnt },
            { .name = "GENERAL_CONFIG_READ_XFERS_CNT", .val = bdmf_address_general_config_read_xfers_cnt },
            { .name = "GENERAL_CONFIG_READ_DATA_CNT", .val = bdmf_address_general_config_read_data_cnt },
            { .name = "GENERAL_CONFIG_WRITE_DATA_CNT", .val = bdmf_address_general_config_write_data_cnt },
            { .name = "GENERAL_CONFIG_MISC_CFG", .val = bdmf_address_general_config_misc_cfg },
            { .name = "GENERAL_CONFIG_AQM_CONTROL", .val = bdmf_address_general_config_aqm_control },
            { .name = "GENERAL_CONFIG_AQM_RANDM_VALUE", .val = bdmf_address_general_config_aqm_randm_value },
            { .name = "GENERAL_CONFIG_AQM_RANDOM_SEED", .val = bdmf_address_general_config_aqm_random_seed },
            { .name = "GENERAL_CONFIG_AQM_RANDOM_TEST_INC", .val = bdmf_address_general_config_aqm_random_test_inc },
            { .name = "GENERAL_CONFIG_MULTI_PSEL_CFG", .val = bdmf_address_general_config_multi_psel_cfg },
            { .name = "DEBUG_FIFO_CONFIG", .val = bdmf_address_debug_fifo_config },
            { .name = "DEBUG_PSRAM_HDR_FIFO_STATUS", .val = bdmf_address_debug_psram_hdr_fifo_status },
            { .name = "DEBUG_PSRAM_DATA_FIFO_STATUS", .val = bdmf_address_debug_psram_data_fifo_status },
            { .name = "DEBUG_DDR_HDR_FIFO_STATUS", .val = bdmf_address_debug_ddr_hdr_fifo_status },
            { .name = "DEBUG_DDR_DATA_FIFO_STATUS", .val = bdmf_address_debug_ddr_data_fifo_status },
            { .name = "DEBUG_DDR_DATA_FIFO_STATUS2", .val = bdmf_address_debug_ddr_data_fifo_status2 },
            { .name = "DEBUG_PSRAM_HDR_FIFO_DATA1", .val = bdmf_address_debug_psram_hdr_fifo_data1 },
            { .name = "DEBUG_PSRAM_HDR_FIFO_DATA2", .val = bdmf_address_debug_psram_hdr_fifo_data2 },
            { .name = "DEBUG_PSRAM_DATA_FIFO_DATA1", .val = bdmf_address_debug_psram_data_fifo_data1 },
            { .name = "DEBUG_PSRAM_DATA_FIFO_DATA2", .val = bdmf_address_debug_psram_data_fifo_data2 },
            { .name = "DEBUG_DDR_HDR_FIFO_DATA1", .val = bdmf_address_debug_ddr_hdr_fifo_data1 },
            { .name = "DEBUG_DDR_HDR_FIFO_DATA2", .val = bdmf_address_debug_ddr_hdr_fifo_data2 },
            { .name = "EXT_FLOWCTRL_CONFIG_TOKEN_VAL", .val = bdmf_address_ext_flowctrl_config_token_val },
            { .name = "EXT_FLOWCTRL_CONFIG2_TOKEN_VAL_2", .val = bdmf_address_ext_flowctrl_config2_token_val_2 },
            { .name = "UBUS_DECODE_CFG_PSRAM_UBUS_DECODE", .val = bdmf_address_ubus_decode_cfg_psram_ubus_decode },
            { .name = "UBUS_DECODE_CFG_DDR_UBUS_DECODE", .val = bdmf_address_ubus_decode_cfg_ddr_ubus_decode },
            { .name = "UBUS_DECODE_CFG2_PSRAM_UBUS_DECODE2", .val = bdmf_address_ubus_decode_cfg2_psram_ubus_decode2 },
            { .name = "UBUS_DECODE_CFG2_DDR_UBUS_DECODE2", .val = bdmf_address_ubus_decode_cfg2_ddr_ubus_decode2 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_rnr_quad_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "quad_idx", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
