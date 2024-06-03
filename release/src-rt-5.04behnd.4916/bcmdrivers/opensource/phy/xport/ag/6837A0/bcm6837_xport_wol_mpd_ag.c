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

#include "bcm6837_drivers_xport_ag.h"
#include "bcm6837_xport_wol_mpd_ag.h"

#define BLOCK_ADDR_COUNT 3

int ag_drv_xport_wol_mpd_config_set(uint8_t xlmac_id, uint8_t psw_en, uint8_t mseq_len)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (psw_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, CONFIG, PSW_EN, reg_config, psw_en);
    reg_config = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, CONFIG, MSEQ_LEN, reg_config, mseq_len);

    RU_REG_WRITE(xlmac_id, XPORT_WOL_MPD, CONFIG, reg_config);

    return 0;
}

int ag_drv_xport_wol_mpd_config_get(uint8_t xlmac_id, uint8_t *psw_en, uint8_t *mseq_len)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if(!psw_en || !mseq_len)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, CONFIG, reg_config);

    *psw_en = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, CONFIG, PSW_EN, reg_config);
    *mseq_len = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, CONFIG, MSEQ_LEN, reg_config);

    return 0;
}

int ag_drv_xport_wol_mpd_control_set(uint8_t xlmac_id, uint8_t mpd_en)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (mpd_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_control = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, CONTROL, MPD_EN, reg_control, mpd_en);

    RU_REG_WRITE(xlmac_id, XPORT_WOL_MPD, CONTROL, reg_control);

    return 0;
}

int ag_drv_xport_wol_mpd_control_get(uint8_t xlmac_id, uint8_t *mpd_en)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if(!mpd_en)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, CONTROL, reg_control);

    *mpd_en = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, CONTROL, MPD_EN, reg_control);

    return 0;
}

int ag_drv_xport_wol_mpd_status_get(uint8_t xlmac_id, uint8_t *mp_detected)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!mp_detected)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, STATUS, reg_status);

    *mp_detected = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, STATUS, MP_DETECTED, reg_status);

    return 0;
}

int ag_drv_xport_wol_mpd_mseq_mac_da_low_set(uint8_t xlmac_id, uint32_t mac_da_31_0)
{
    uint32_t reg_mseq_mac_da_low=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_mseq_mac_da_low = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_LOW, MAC_DA_31_0, reg_mseq_mac_da_low, mac_da_31_0);

    RU_REG_WRITE(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_LOW, reg_mseq_mac_da_low);

    return 0;
}

int ag_drv_xport_wol_mpd_mseq_mac_da_low_get(uint8_t xlmac_id, uint32_t *mac_da_31_0)
{
    uint32_t reg_mseq_mac_da_low=0;

#ifdef VALIDATE_PARMS
    if(!mac_da_31_0)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_LOW, reg_mseq_mac_da_low);

    *mac_da_31_0 = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_LOW, MAC_DA_31_0, reg_mseq_mac_da_low);

    return 0;
}

int ag_drv_xport_wol_mpd_mseq_mac_da_hi_set(uint8_t xlmac_id, uint16_t mac_da_47_32)
{
    uint32_t reg_mseq_mac_da_hi=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_mseq_mac_da_hi = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_HI, MAC_DA_47_32, reg_mseq_mac_da_hi, mac_da_47_32);

    RU_REG_WRITE(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_HI, reg_mseq_mac_da_hi);

    return 0;
}

int ag_drv_xport_wol_mpd_mseq_mac_da_hi_get(uint8_t xlmac_id, uint16_t *mac_da_47_32)
{
    uint32_t reg_mseq_mac_da_hi=0;

#ifdef VALIDATE_PARMS
    if(!mac_da_47_32)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_HI, reg_mseq_mac_da_hi);

    *mac_da_47_32 = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, MSEQ_MAC_DA_HI, MAC_DA_47_32, reg_mseq_mac_da_hi);

    return 0;
}

int ag_drv_xport_wol_mpd_psw_low_set(uint8_t xlmac_id, uint32_t psw_31_0)
{
    uint32_t reg_psw_low=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_psw_low = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, PSW_LOW, PSW_31_0, reg_psw_low, psw_31_0);

    RU_REG_WRITE(xlmac_id, XPORT_WOL_MPD, PSW_LOW, reg_psw_low);

    return 0;
}

int ag_drv_xport_wol_mpd_psw_low_get(uint8_t xlmac_id, uint32_t *psw_31_0)
{
    uint32_t reg_psw_low=0;

#ifdef VALIDATE_PARMS
    if(!psw_31_0)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, PSW_LOW, reg_psw_low);

    *psw_31_0 = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, PSW_LOW, PSW_31_0, reg_psw_low);

    return 0;
}

int ag_drv_xport_wol_mpd_psw_hi_set(uint8_t xlmac_id, uint16_t psw_47_32)
{
    uint32_t reg_psw_hi=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_psw_hi = RU_FIELD_SET(xlmac_id, XPORT_WOL_MPD, PSW_HI, PSW_47_32, reg_psw_hi, psw_47_32);

    RU_REG_WRITE(xlmac_id, XPORT_WOL_MPD, PSW_HI, reg_psw_hi);

    return 0;
}

int ag_drv_xport_wol_mpd_psw_hi_get(uint8_t xlmac_id, uint16_t *psw_47_32)
{
    uint32_t reg_psw_hi=0;

#ifdef VALIDATE_PARMS
    if(!psw_47_32)
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

    RU_REG_READ(xlmac_id, XPORT_WOL_MPD, PSW_HI, reg_psw_hi);

    *psw_47_32 = RU_FIELD_GET(xlmac_id, XPORT_WOL_MPD, PSW_HI, PSW_47_32, reg_psw_hi);

    return 0;
}

