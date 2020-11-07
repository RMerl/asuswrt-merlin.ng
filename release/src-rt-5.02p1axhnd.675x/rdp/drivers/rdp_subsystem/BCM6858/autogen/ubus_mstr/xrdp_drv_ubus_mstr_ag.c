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
#include "xrdp_drv_ubus_mstr_ag.h"

#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_ubus_mstr_en_set(uint8_t ubus_mstr_id, bdmf_boolean en)
{
    uint32_t reg_en=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_en = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, EN, EN, reg_en, en);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, EN, reg_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_en_get(uint8_t ubus_mstr_id, bdmf_boolean *en)
{
    uint32_t reg_en;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, EN, reg_en);

    *en = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, EN, EN, reg_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_req_cntrl_set(uint8_t ubus_mstr_id, const ubus_mstr_req_cntrl *req_cntrl)
{
    uint32_t reg_req_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!req_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (req_cntrl->pkt_tag >= _1BITS_MAX_VAL_) ||
       (req_cntrl->endian_mode >= _2BITS_MAX_VAL_) ||
       (req_cntrl->repin_eswap >= _1BITS_MAX_VAL_) ||
       (req_cntrl->reqout_eswap >= _1BITS_MAX_VAL_) ||
       (req_cntrl->dev_err >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, PKT_ID, reg_req_cntrl, req_cntrl->pkt_id);
    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, PKT_TAG, reg_req_cntrl, req_cntrl->pkt_tag);
    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, ENDIAN_MODE, reg_req_cntrl, req_cntrl->endian_mode);
    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, REPIN_ESWAP, reg_req_cntrl, req_cntrl->repin_eswap);
    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, REQOUT_ESWAP, reg_req_cntrl, req_cntrl->reqout_eswap);
    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, DEV_ERR, reg_req_cntrl, req_cntrl->dev_err);
    reg_req_cntrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, MAX_PKT_LEN, reg_req_cntrl, req_cntrl->max_pkt_len);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, reg_req_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_req_cntrl_get(uint8_t ubus_mstr_id, ubus_mstr_req_cntrl *req_cntrl)
{
    uint32_t reg_req_cntrl;

#ifdef VALIDATE_PARMS
    if(!req_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, reg_req_cntrl);

    req_cntrl->pkt_id = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, PKT_ID, reg_req_cntrl);
    req_cntrl->pkt_tag = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, PKT_TAG, reg_req_cntrl);
    req_cntrl->endian_mode = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, ENDIAN_MODE, reg_req_cntrl);
    req_cntrl->repin_eswap = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, REPIN_ESWAP, reg_req_cntrl);
    req_cntrl->reqout_eswap = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, REQOUT_ESWAP, reg_req_cntrl);
    req_cntrl->dev_err = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, DEV_ERR, reg_req_cntrl);
    req_cntrl->max_pkt_len = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REQ_CNTRL, MAX_PKT_LEN, reg_req_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hyst_ctrl_set(uint8_t ubus_mstr_id, uint16_t cmd_space, uint16_t data_space)
{
    uint32_t reg_hyst_ctrl=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (cmd_space >= _10BITS_MAX_VAL_) ||
       (data_space >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_hyst_ctrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, CMD_SPACE, reg_hyst_ctrl, cmd_space);
    reg_hyst_ctrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, DATA_SPACE, reg_hyst_ctrl, data_space);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, reg_hyst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hyst_ctrl_get(uint8_t ubus_mstr_id, uint16_t *cmd_space, uint16_t *data_space)
{
    uint32_t reg_hyst_ctrl;

#ifdef VALIDATE_PARMS
    if(!cmd_space || !data_space)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, reg_hyst_ctrl);

    *cmd_space = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, CMD_SPACE, reg_hyst_ctrl);
    *data_space = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, DATA_SPACE, reg_hyst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hp_set(uint8_t ubus_mstr_id, const ubus_mstr_hp *hp)
{
    uint32_t reg_hp=0;

#ifdef VALIDATE_PARMS
    if(!hp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (hp->hp_en >= _1BITS_MAX_VAL_) ||
       (hp->hp_sel >= _1BITS_MAX_VAL_) ||
       (hp->hp_comb >= _1BITS_MAX_VAL_) ||
       (hp->hp_cnt_high >= _4BITS_MAX_VAL_) ||
       (hp->hp_cnt_total >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_hp = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HP, HP_EN, reg_hp, hp->hp_en);
    reg_hp = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HP, HP_SEL, reg_hp, hp->hp_sel);
    reg_hp = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HP, HP_COMB, reg_hp, hp->hp_comb);
    reg_hp = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HP, HP_CNT_HIGH, reg_hp, hp->hp_cnt_high);
    reg_hp = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HP, HP_CNT_TOTAL, reg_hp, hp->hp_cnt_total);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, HP, reg_hp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hp_get(uint8_t ubus_mstr_id, ubus_mstr_hp *hp)
{
    uint32_t reg_hp;

#ifdef VALIDATE_PARMS
    if(!hp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, HP, reg_hp);

    hp->hp_en = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HP, HP_EN, reg_hp);
    hp->hp_sel = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HP, HP_SEL, reg_hp);
    hp->hp_comb = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HP, HP_COMB, reg_hp);
    hp->hp_cnt_high = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HP, HP_CNT_HIGH, reg_hp);
    hp->hp_cnt_total = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HP, HP_CNT_TOTAL, reg_hp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_reply_add_set(uint8_t ubus_mstr_id, uint32_t add)
{
    uint32_t reg_reply_add=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reply_add = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REPLY_ADD, ADD, reg_reply_add, add);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, REPLY_ADD, reg_reply_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_reply_add_get(uint8_t ubus_mstr_id, uint32_t *add)
{
    uint32_t reg_reply_add;

#ifdef VALIDATE_PARMS
    if(!add)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, REPLY_ADD, reg_reply_add);

    *add = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REPLY_ADD, ADD, reg_reply_add);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_reply_data_set(uint8_t ubus_mstr_id, uint32_t data)
{
    uint32_t reg_reply_data=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reply_data = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, REPLY_DATA, DATA, reg_reply_data, data);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, REPLY_DATA, reg_reply_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_reply_data_get(uint8_t ubus_mstr_id, uint32_t *data)
{
    uint32_t reg_reply_data;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, REPLY_DATA, reg_reply_data);

    *data = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, REPLY_DATA, DATA, reg_reply_data);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_en,
    bdmf_address_req_cntrl,
    bdmf_address_hyst_ctrl,
    bdmf_address_hp,
    bdmf_address_reply_add,
    bdmf_address_reply_data,
}
bdmf_address;

static int bcm_ubus_mstr_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_ubus_mstr_en:
        err = ag_drv_ubus_mstr_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_mstr_req_cntrl:
    {
        ubus_mstr_req_cntrl req_cntrl = { .pkt_id=parm[2].value.unumber, .pkt_tag=parm[3].value.unumber, .endian_mode=parm[4].value.unumber, .repin_eswap=parm[5].value.unumber, .reqout_eswap=parm[6].value.unumber, .dev_err=parm[7].value.unumber, .max_pkt_len=parm[8].value.unumber};
        err = ag_drv_ubus_mstr_req_cntrl_set(parm[1].value.unumber, &req_cntrl);
        break;
    }
    case cli_ubus_mstr_hyst_ctrl:
        err = ag_drv_ubus_mstr_hyst_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_ubus_mstr_hp:
    {
        ubus_mstr_hp hp = { .hp_en=parm[2].value.unumber, .hp_sel=parm[3].value.unumber, .hp_comb=parm[4].value.unumber, .hp_cnt_high=parm[5].value.unumber, .hp_cnt_total=parm[6].value.unumber};
        err = ag_drv_ubus_mstr_hp_set(parm[1].value.unumber, &hp);
        break;
    }
    case cli_ubus_mstr_reply_add:
        err = ag_drv_ubus_mstr_reply_add_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_mstr_reply_data:
        err = ag_drv_ubus_mstr_reply_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_ubus_mstr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_ubus_mstr_en:
    {
        bdmf_boolean en;
        err = ag_drv_ubus_mstr_en_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_ubus_mstr_req_cntrl:
    {
        ubus_mstr_req_cntrl req_cntrl;
        err = ag_drv_ubus_mstr_req_cntrl_get(parm[1].value.unumber, &req_cntrl);
        bdmf_session_print(session, "pkt_id = %u (0x%x)\n", req_cntrl.pkt_id, req_cntrl.pkt_id);
        bdmf_session_print(session, "pkt_tag = %u (0x%x)\n", req_cntrl.pkt_tag, req_cntrl.pkt_tag);
        bdmf_session_print(session, "endian_mode = %u (0x%x)\n", req_cntrl.endian_mode, req_cntrl.endian_mode);
        bdmf_session_print(session, "repin_eswap = %u (0x%x)\n", req_cntrl.repin_eswap, req_cntrl.repin_eswap);
        bdmf_session_print(session, "reqout_eswap = %u (0x%x)\n", req_cntrl.reqout_eswap, req_cntrl.reqout_eswap);
        bdmf_session_print(session, "dev_err = %u (0x%x)\n", req_cntrl.dev_err, req_cntrl.dev_err);
        bdmf_session_print(session, "max_pkt_len = %u (0x%x)\n", req_cntrl.max_pkt_len, req_cntrl.max_pkt_len);
        break;
    }
    case cli_ubus_mstr_hyst_ctrl:
    {
        uint16_t cmd_space;
        uint16_t data_space;
        err = ag_drv_ubus_mstr_hyst_ctrl_get(parm[1].value.unumber, &cmd_space, &data_space);
        bdmf_session_print(session, "cmd_space = %u (0x%x)\n", cmd_space, cmd_space);
        bdmf_session_print(session, "data_space = %u (0x%x)\n", data_space, data_space);
        break;
    }
    case cli_ubus_mstr_hp:
    {
        ubus_mstr_hp hp;
        err = ag_drv_ubus_mstr_hp_get(parm[1].value.unumber, &hp);
        bdmf_session_print(session, "hp_en = %u (0x%x)\n", hp.hp_en, hp.hp_en);
        bdmf_session_print(session, "hp_sel = %u (0x%x)\n", hp.hp_sel, hp.hp_sel);
        bdmf_session_print(session, "hp_comb = %u (0x%x)\n", hp.hp_comb, hp.hp_comb);
        bdmf_session_print(session, "hp_cnt_high = %u (0x%x)\n", hp.hp_cnt_high, hp.hp_cnt_high);
        bdmf_session_print(session, "hp_cnt_total = %u (0x%x)\n", hp.hp_cnt_total, hp.hp_cnt_total);
        break;
    }
    case cli_ubus_mstr_reply_add:
    {
        uint32_t add;
        err = ag_drv_ubus_mstr_reply_add_get(parm[1].value.unumber, &add);
        bdmf_session_print(session, "add = %u (0x%x)\n", add, add);
        break;
    }
    case cli_ubus_mstr_reply_data:
    {
        uint32_t data;
        err = ag_drv_ubus_mstr_reply_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_ubus_mstr_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t ubus_mstr_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_en_set(%u %u)\n", ubus_mstr_id, en);
        if(!err) ag_drv_ubus_mstr_en_set(ubus_mstr_id, en);
        if(!err) ag_drv_ubus_mstr_en_get( ubus_mstr_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_en_get(%u %u)\n", ubus_mstr_id, en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_mstr_req_cntrl req_cntrl = {.pkt_id=gtmv(m, 8), .pkt_tag=gtmv(m, 1), .endian_mode=gtmv(m, 2), .repin_eswap=gtmv(m, 1), .reqout_eswap=gtmv(m, 1), .dev_err=gtmv(m, 1), .max_pkt_len=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_req_cntrl_set(%u %u %u %u %u %u %u %u)\n", ubus_mstr_id, req_cntrl.pkt_id, req_cntrl.pkt_tag, req_cntrl.endian_mode, req_cntrl.repin_eswap, req_cntrl.reqout_eswap, req_cntrl.dev_err, req_cntrl.max_pkt_len);
        if(!err) ag_drv_ubus_mstr_req_cntrl_set(ubus_mstr_id, &req_cntrl);
        if(!err) ag_drv_ubus_mstr_req_cntrl_get( ubus_mstr_id, &req_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_req_cntrl_get(%u %u %u %u %u %u %u %u)\n", ubus_mstr_id, req_cntrl.pkt_id, req_cntrl.pkt_tag, req_cntrl.endian_mode, req_cntrl.repin_eswap, req_cntrl.reqout_eswap, req_cntrl.dev_err, req_cntrl.max_pkt_len);
        if(err || req_cntrl.pkt_id!=gtmv(m, 8) || req_cntrl.pkt_tag!=gtmv(m, 1) || req_cntrl.endian_mode!=gtmv(m, 2) || req_cntrl.repin_eswap!=gtmv(m, 1) || req_cntrl.reqout_eswap!=gtmv(m, 1) || req_cntrl.dev_err!=gtmv(m, 1) || req_cntrl.max_pkt_len!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cmd_space=gtmv(m, 10);
        uint16_t data_space=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hyst_ctrl_set(%u %u %u)\n", ubus_mstr_id, cmd_space, data_space);
        if(!err) ag_drv_ubus_mstr_hyst_ctrl_set(ubus_mstr_id, cmd_space, data_space);
        if(!err) ag_drv_ubus_mstr_hyst_ctrl_get( ubus_mstr_id, &cmd_space, &data_space);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hyst_ctrl_get(%u %u %u)\n", ubus_mstr_id, cmd_space, data_space);
        if(err || cmd_space!=gtmv(m, 10) || data_space!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ubus_mstr_hp hp = {.hp_en=gtmv(m, 1), .hp_sel=gtmv(m, 1), .hp_comb=gtmv(m, 1), .hp_cnt_high=gtmv(m, 4), .hp_cnt_total=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hp_set(%u %u %u %u %u %u)\n", ubus_mstr_id, hp.hp_en, hp.hp_sel, hp.hp_comb, hp.hp_cnt_high, hp.hp_cnt_total);
        if(!err) ag_drv_ubus_mstr_hp_set(ubus_mstr_id, &hp);
        if(!err) ag_drv_ubus_mstr_hp_get( ubus_mstr_id, &hp);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hp_get(%u %u %u %u %u %u)\n", ubus_mstr_id, hp.hp_en, hp.hp_sel, hp.hp_comb, hp.hp_cnt_high, hp.hp_cnt_total);
        if(err || hp.hp_en!=gtmv(m, 1) || hp.hp_sel!=gtmv(m, 1) || hp.hp_comb!=gtmv(m, 1) || hp.hp_cnt_high!=gtmv(m, 4) || hp.hp_cnt_total!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t add=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_reply_add_set(%u %u)\n", ubus_mstr_id, add);
        if(!err) ag_drv_ubus_mstr_reply_add_set(ubus_mstr_id, add);
        if(!err) ag_drv_ubus_mstr_reply_add_get( ubus_mstr_id, &add);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_reply_add_get(%u %u)\n", ubus_mstr_id, add);
        if(err || add!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_reply_data_set(%u %u)\n", ubus_mstr_id, data);
        if(!err) ag_drv_ubus_mstr_reply_data_set(ubus_mstr_id, data);
        if(!err) ag_drv_ubus_mstr_reply_data_get( ubus_mstr_id, &data);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_reply_data_get(%u %u)\n", ubus_mstr_id, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_ubus_mstr_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_en : reg = &RU_REG(UBUS_MSTR, EN); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_req_cntrl : reg = &RU_REG(UBUS_MSTR, REQ_CNTRL); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_hyst_ctrl : reg = &RU_REG(UBUS_MSTR, HYST_CTRL); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_hp : reg = &RU_REG(UBUS_MSTR, HP); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_reply_add : reg = &RU_REG(UBUS_MSTR, REPLY_ADD); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_reply_data : reg = &RU_REG(UBUS_MSTR, REPLY_DATA); blk = &RU_BLK(UBUS_MSTR); break;
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

bdmfmon_handle_t ag_drv_ubus_mstr_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "ubus_mstr"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "ubus_mstr", "ubus_mstr", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_en[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_req_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pkt_id", "pkt_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_tag", "pkt_tag", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("endian_mode", "endian_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("repin_eswap", "repin_eswap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reqout_eswap", "reqout_eswap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dev_err", "dev_err", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("max_pkt_len", "max_pkt_len", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_hyst_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("cmd_space", "cmd_space", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data_space", "data_space", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_hp[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("hp_en", "hp_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hp_sel", "hp_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hp_comb", "hp_comb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hp_cnt_high", "hp_cnt_high", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hp_cnt_total", "hp_cnt_total", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reply_add[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("add", "add", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reply_data[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="en", .val=cli_ubus_mstr_en, .parms=set_en },
            { .name="req_cntrl", .val=cli_ubus_mstr_req_cntrl, .parms=set_req_cntrl },
            { .name="hyst_ctrl", .val=cli_ubus_mstr_hyst_ctrl, .parms=set_hyst_ctrl },
            { .name="hp", .val=cli_ubus_mstr_hp, .parms=set_hp },
            { .name="reply_add", .val=cli_ubus_mstr_reply_add, .parms=set_reply_add },
            { .name="reply_data", .val=cli_ubus_mstr_reply_data, .parms=set_reply_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_ubus_mstr_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="en", .val=cli_ubus_mstr_en, .parms=set_default },
            { .name="req_cntrl", .val=cli_ubus_mstr_req_cntrl, .parms=set_default },
            { .name="hyst_ctrl", .val=cli_ubus_mstr_hyst_ctrl, .parms=set_default },
            { .name="hp", .val=cli_ubus_mstr_hp, .parms=set_default },
            { .name="reply_add", .val=cli_ubus_mstr_reply_add, .parms=set_default },
            { .name="reply_data", .val=cli_ubus_mstr_reply_data, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_ubus_mstr_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_ubus_mstr_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="EN" , .val=bdmf_address_en },
            { .name="REQ_CNTRL" , .val=bdmf_address_req_cntrl },
            { .name="HYST_CTRL" , .val=bdmf_address_hyst_ctrl },
            { .name="HP" , .val=bdmf_address_hp },
            { .name="REPLY_ADD" , .val=bdmf_address_reply_add },
            { .name="REPLY_DATA" , .val=bdmf_address_reply_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_ubus_mstr_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

