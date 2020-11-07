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
#include "xrdp_drv_natc_eng_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_eng_command_status_set(uint8_t eng_idx, const natc_eng_command_status *command_status)
{
    uint32_t reg_command_status=0;

#ifdef VALIDATE_PARMS
    if(!command_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT) ||
       (command_status->add_cmd_ddr_miss >= _1BITS_MAX_VAL_) ||
       (command_status->add_cmd_ddr_bin >= _3BITS_MAX_VAL_) ||
       (command_status->add_cmd_mode >= _1BITS_MAX_VAL_) ||
       (command_status->del_cmd_mode >= _1BITS_MAX_VAL_) ||
       (command_status->cache_flush >= _1BITS_MAX_VAL_) ||
       (command_status->decr_count >= _1BITS_MAX_VAL_) ||
       (command_status->nat_tbl >= _3BITS_MAX_VAL_) ||
       (command_status->multihash_count >= _4BITS_MAX_VAL_) ||
       (command_status->cache_hit >= _1BITS_MAX_VAL_) ||
       (command_status->miss >= _1BITS_MAX_VAL_) ||
       (command_status->error >= _1BITS_MAX_VAL_) ||
       (command_status->busy >= _1BITS_MAX_VAL_) ||
       (command_status->unused0 >= _1BITS_MAX_VAL_) ||
       (command_status->command >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, ADD_CMD_DDR_MISS, reg_command_status, command_status->add_cmd_ddr_miss);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, ADD_CMD_DDR_BIN, reg_command_status, command_status->add_cmd_ddr_bin);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, DEL_CMD_DDR_BIN, reg_command_status, command_status->del_cmd_ddr_bin);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, ADD_CMD_MODE, reg_command_status, command_status->add_cmd_mode);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, DEL_CMD_MODE, reg_command_status, command_status->del_cmd_mode);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, CACHE_FLUSH, reg_command_status, command_status->cache_flush);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, DECR_COUNT, reg_command_status, command_status->decr_count);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, NAT_TBL, reg_command_status, command_status->nat_tbl);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, MULTIHASH_COUNT, reg_command_status, command_status->multihash_count);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, CACHE_HIT, reg_command_status, command_status->cache_hit);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, MISS, reg_command_status, command_status->miss);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, ERROR, reg_command_status, command_status->error);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, BUSY, reg_command_status, command_status->busy);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, UNUSED0, reg_command_status, command_status->unused0);
    reg_command_status = RU_FIELD_SET(eng_idx, NATC_ENG, COMMAND_STATUS, COMMAND, reg_command_status, command_status->command);

    RU_REG_WRITE(eng_idx, NATC_ENG, COMMAND_STATUS, reg_command_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_command_status_get(uint8_t eng_idx, natc_eng_command_status *command_status)
{
    uint32_t reg_command_status;

#ifdef VALIDATE_PARMS
    if(!command_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(eng_idx, NATC_ENG, COMMAND_STATUS, reg_command_status);

    command_status->add_cmd_ddr_miss = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, ADD_CMD_DDR_MISS, reg_command_status);
    command_status->add_cmd_ddr_bin = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, ADD_CMD_DDR_BIN, reg_command_status);
    command_status->del_cmd_ddr_bin = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, DEL_CMD_DDR_BIN, reg_command_status);
    command_status->add_cmd_mode = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, ADD_CMD_MODE, reg_command_status);
    command_status->del_cmd_mode = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, DEL_CMD_MODE, reg_command_status);
    command_status->cache_flush = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, CACHE_FLUSH, reg_command_status);
    command_status->decr_count = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, DECR_COUNT, reg_command_status);
    command_status->nat_tbl = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, NAT_TBL, reg_command_status);
    command_status->multihash_count = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, MULTIHASH_COUNT, reg_command_status);
    command_status->cache_hit = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, CACHE_HIT, reg_command_status);
    command_status->miss = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, MISS, reg_command_status);
    command_status->error = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, ERROR, reg_command_status);
    command_status->busy = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, BUSY, reg_command_status);
    command_status->unused0 = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, UNUSED0, reg_command_status);
    command_status->command = RU_FIELD_GET(eng_idx, NATC_ENG, COMMAND_STATUS, COMMAND, reg_command_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_hash_get(uint8_t eng_idx, uint32_t *hash)
{
    uint32_t reg_hash;

#ifdef VALIDATE_PARMS
    if(!hash)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(eng_idx, NATC_ENG, HASH, reg_hash);

    *hash = RU_FIELD_GET(eng_idx, NATC_ENG, HASH, HASH, reg_hash);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_hit_count_get(uint8_t eng_idx, uint32_t *hit_count)
{
    uint32_t reg_hit_count;

#ifdef VALIDATE_PARMS
    if(!hit_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(eng_idx, NATC_ENG, HIT_COUNT, reg_hit_count);

    *hit_count = RU_FIELD_GET(eng_idx, NATC_ENG, HIT_COUNT, HIT_COUNT, reg_hit_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_byte_count_get(uint8_t eng_idx, uint32_t *byte_count)
{
    uint32_t reg_byte_count;

#ifdef VALIDATE_PARMS
    if(!byte_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(eng_idx, NATC_ENG, BYTE_COUNT, reg_byte_count);

    *byte_count = RU_FIELD_GET(eng_idx, NATC_ENG, BYTE_COUNT, BYTE_COUNT, reg_byte_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_pkt_len_set(uint8_t eng_idx, uint16_t unused, uint16_t pkt_len)
{
    uint32_t reg_pkt_len=0;

#ifdef VALIDATE_PARMS
    if((eng_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pkt_len = RU_FIELD_SET(eng_idx, NATC_ENG, PKT_LEN, UNUSED, reg_pkt_len, unused);
    reg_pkt_len = RU_FIELD_SET(eng_idx, NATC_ENG, PKT_LEN, PKT_LEN, reg_pkt_len, pkt_len);

    RU_REG_WRITE(eng_idx, NATC_ENG, PKT_LEN, reg_pkt_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_pkt_len_get(uint8_t eng_idx, uint16_t *unused, uint16_t *pkt_len)
{
    uint32_t reg_pkt_len;

#ifdef VALIDATE_PARMS
    if(!unused || !pkt_len)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(eng_idx, NATC_ENG, PKT_LEN, reg_pkt_len);

    *unused = RU_FIELD_GET(eng_idx, NATC_ENG, PKT_LEN, UNUSED, reg_pkt_len);
    *pkt_len = RU_FIELD_GET(eng_idx, NATC_ENG, PKT_LEN, PKT_LEN, reg_pkt_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_key_result_set(uint8_t eng_idx, uint8_t zero, const natc_eng_key_result *key_result)
{
#ifdef VALIDATE_PARMS
    if(!key_result)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(eng_idx, zero *18 + 0, NATC_ENG, KEY_RESULT, key_result->data[0]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 1, NATC_ENG, KEY_RESULT, key_result->data[1]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 2, NATC_ENG, KEY_RESULT, key_result->data[2]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 3, NATC_ENG, KEY_RESULT, key_result->data[3]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 4, NATC_ENG, KEY_RESULT, key_result->data[4]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 5, NATC_ENG, KEY_RESULT, key_result->data[5]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 6, NATC_ENG, KEY_RESULT, key_result->data[6]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 7, NATC_ENG, KEY_RESULT, key_result->data[7]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 8, NATC_ENG, KEY_RESULT, key_result->data[8]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 9, NATC_ENG, KEY_RESULT, key_result->data[9]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 10, NATC_ENG, KEY_RESULT, key_result->data[10]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 11, NATC_ENG, KEY_RESULT, key_result->data[11]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 12, NATC_ENG, KEY_RESULT, key_result->data[12]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 13, NATC_ENG, KEY_RESULT, key_result->data[13]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 14, NATC_ENG, KEY_RESULT, key_result->data[14]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 15, NATC_ENG, KEY_RESULT, key_result->data[15]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 16, NATC_ENG, KEY_RESULT, key_result->data[16]);
    RU_REG_RAM_WRITE(eng_idx, zero *18 + 17, NATC_ENG, KEY_RESULT, key_result->data[17]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_eng_key_result_get(uint8_t eng_idx, uint8_t zero, natc_eng_key_result *key_result)
{
#ifdef VALIDATE_PARMS
    if(!key_result)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((eng_idx >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(eng_idx, zero *18 + 0, NATC_ENG, KEY_RESULT, key_result->data[0]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 1, NATC_ENG, KEY_RESULT, key_result->data[1]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 2, NATC_ENG, KEY_RESULT, key_result->data[2]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 3, NATC_ENG, KEY_RESULT, key_result->data[3]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 4, NATC_ENG, KEY_RESULT, key_result->data[4]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 5, NATC_ENG, KEY_RESULT, key_result->data[5]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 6, NATC_ENG, KEY_RESULT, key_result->data[6]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 7, NATC_ENG, KEY_RESULT, key_result->data[7]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 8, NATC_ENG, KEY_RESULT, key_result->data[8]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 9, NATC_ENG, KEY_RESULT, key_result->data[9]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 10, NATC_ENG, KEY_RESULT, key_result->data[10]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 11, NATC_ENG, KEY_RESULT, key_result->data[11]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 12, NATC_ENG, KEY_RESULT, key_result->data[12]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 13, NATC_ENG, KEY_RESULT, key_result->data[13]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 14, NATC_ENG, KEY_RESULT, key_result->data[14]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 15, NATC_ENG, KEY_RESULT, key_result->data[15]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 16, NATC_ENG, KEY_RESULT, key_result->data[16]);
    RU_REG_RAM_READ(eng_idx, zero *18 + 17, NATC_ENG, KEY_RESULT, key_result->data[17]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_command_status,
    bdmf_address_hash,
    bdmf_address_hit_count,
    bdmf_address_byte_count,
    bdmf_address_pkt_len,
    bdmf_address_key_result,
}
bdmf_address;

static int bcm_natc_eng_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_eng_command_status:
    {
        natc_eng_command_status command_status = { .add_cmd_ddr_miss=parm[2].value.unumber, .add_cmd_ddr_bin=parm[3].value.unumber, .del_cmd_ddr_bin=parm[4].value.unumber, .add_cmd_mode=parm[5].value.unumber, .del_cmd_mode=parm[6].value.unumber, .cache_flush=parm[7].value.unumber, .decr_count=parm[8].value.unumber, .nat_tbl=parm[9].value.unumber, .multihash_count=parm[10].value.unumber, .cache_hit=parm[11].value.unumber, .miss=parm[12].value.unumber, .error=parm[13].value.unumber, .busy=parm[14].value.unumber, .unused0=parm[15].value.unumber, .command=parm[16].value.unumber};
        err = ag_drv_natc_eng_command_status_set(parm[1].value.unumber, &command_status);
        break;
    }
    case cli_natc_eng_pkt_len:
        err = ag_drv_natc_eng_pkt_len_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_natc_eng_key_result:
    {
        natc_eng_key_result key_result = { .data = { parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber}};
        err = ag_drv_natc_eng_key_result_set(parm[1].value.unumber, parm[2].value.unumber, &key_result);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_natc_eng_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_eng_command_status:
    {
        natc_eng_command_status command_status;
        err = ag_drv_natc_eng_command_status_get(parm[1].value.unumber, &command_status);
        bdmf_session_print(session, "add_cmd_ddr_miss = %u (0x%x)\n", command_status.add_cmd_ddr_miss, command_status.add_cmd_ddr_miss);
        bdmf_session_print(session, "add_cmd_ddr_bin = %u (0x%x)\n", command_status.add_cmd_ddr_bin, command_status.add_cmd_ddr_bin);
        bdmf_session_print(session, "del_cmd_ddr_bin = %u (0x%x)\n", command_status.del_cmd_ddr_bin, command_status.del_cmd_ddr_bin);
        bdmf_session_print(session, "add_cmd_mode = %u (0x%x)\n", command_status.add_cmd_mode, command_status.add_cmd_mode);
        bdmf_session_print(session, "del_cmd_mode = %u (0x%x)\n", command_status.del_cmd_mode, command_status.del_cmd_mode);
        bdmf_session_print(session, "cache_flush = %u (0x%x)\n", command_status.cache_flush, command_status.cache_flush);
        bdmf_session_print(session, "decr_count = %u (0x%x)\n", command_status.decr_count, command_status.decr_count);
        bdmf_session_print(session, "nat_tbl = %u (0x%x)\n", command_status.nat_tbl, command_status.nat_tbl);
        bdmf_session_print(session, "multihash_count = %u (0x%x)\n", command_status.multihash_count, command_status.multihash_count);
        bdmf_session_print(session, "cache_hit = %u (0x%x)\n", command_status.cache_hit, command_status.cache_hit);
        bdmf_session_print(session, "miss = %u (0x%x)\n", command_status.miss, command_status.miss);
        bdmf_session_print(session, "error = %u (0x%x)\n", command_status.error, command_status.error);
        bdmf_session_print(session, "busy = %u (0x%x)\n", command_status.busy, command_status.busy);
        bdmf_session_print(session, "unused0 = %u (0x%x)\n", command_status.unused0, command_status.unused0);
        bdmf_session_print(session, "command = %u (0x%x)\n", command_status.command, command_status.command);
        break;
    }
    case cli_natc_eng_hash:
    {
        uint32_t hash;
        err = ag_drv_natc_eng_hash_get(parm[1].value.unumber, &hash);
        bdmf_session_print(session, "hash = %u (0x%x)\n", hash, hash);
        break;
    }
    case cli_natc_eng_hit_count:
    {
        uint32_t hit_count;
        err = ag_drv_natc_eng_hit_count_get(parm[1].value.unumber, &hit_count);
        bdmf_session_print(session, "hit_count = %u (0x%x)\n", hit_count, hit_count);
        break;
    }
    case cli_natc_eng_byte_count:
    {
        uint32_t byte_count;
        err = ag_drv_natc_eng_byte_count_get(parm[1].value.unumber, &byte_count);
        bdmf_session_print(session, "byte_count = %u (0x%x)\n", byte_count, byte_count);
        break;
    }
    case cli_natc_eng_pkt_len:
    {
        uint16_t unused;
        uint16_t pkt_len;
        err = ag_drv_natc_eng_pkt_len_get(parm[1].value.unumber, &unused, &pkt_len);
        bdmf_session_print(session, "unused = %u (0x%x)\n", unused, unused);
        bdmf_session_print(session, "pkt_len = %u (0x%x)\n", pkt_len, pkt_len);
        break;
    }
    case cli_natc_eng_key_result:
    {
        natc_eng_key_result key_result;
        err = ag_drv_natc_eng_key_result_get(parm[1].value.unumber, parm[2].value.unumber, &key_result);
        bdmf_session_print(session, "data[0] = %u (0x%x)\n", key_result.data[0], key_result.data[0]);
        bdmf_session_print(session, "data[1] = %u (0x%x)\n", key_result.data[1], key_result.data[1]);
        bdmf_session_print(session, "data[2] = %u (0x%x)\n", key_result.data[2], key_result.data[2]);
        bdmf_session_print(session, "data[3] = %u (0x%x)\n", key_result.data[3], key_result.data[3]);
        bdmf_session_print(session, "data[4] = %u (0x%x)\n", key_result.data[4], key_result.data[4]);
        bdmf_session_print(session, "data[5] = %u (0x%x)\n", key_result.data[5], key_result.data[5]);
        bdmf_session_print(session, "data[6] = %u (0x%x)\n", key_result.data[6], key_result.data[6]);
        bdmf_session_print(session, "data[7] = %u (0x%x)\n", key_result.data[7], key_result.data[7]);
        bdmf_session_print(session, "data[8] = %u (0x%x)\n", key_result.data[8], key_result.data[8]);
        bdmf_session_print(session, "data[9] = %u (0x%x)\n", key_result.data[9], key_result.data[9]);
        bdmf_session_print(session, "data[10] = %u (0x%x)\n", key_result.data[10], key_result.data[10]);
        bdmf_session_print(session, "data[11] = %u (0x%x)\n", key_result.data[11], key_result.data[11]);
        bdmf_session_print(session, "data[12] = %u (0x%x)\n", key_result.data[12], key_result.data[12]);
        bdmf_session_print(session, "data[13] = %u (0x%x)\n", key_result.data[13], key_result.data[13]);
        bdmf_session_print(session, "data[14] = %u (0x%x)\n", key_result.data[14], key_result.data[14]);
        bdmf_session_print(session, "data[15] = %u (0x%x)\n", key_result.data[15], key_result.data[15]);
        bdmf_session_print(session, "data[16] = %u (0x%x)\n", key_result.data[16], key_result.data[16]);
        bdmf_session_print(session, "data[17] = %u (0x%x)\n", key_result.data[17], key_result.data[17]);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_natc_eng_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t eng_idx = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        natc_eng_command_status command_status = {.add_cmd_ddr_miss=gtmv(m, 1), .add_cmd_ddr_bin=gtmv(m, 3), .del_cmd_ddr_bin=gtmv(m, 8), .add_cmd_mode=gtmv(m, 1), .del_cmd_mode=gtmv(m, 1), .cache_flush=gtmv(m, 1), .decr_count=gtmv(m, 1), .nat_tbl=gtmv(m, 3), .multihash_count=gtmv(m, 4), .cache_hit=gtmv(m, 1), .miss=gtmv(m, 1), .error=gtmv(m, 1), .busy=gtmv(m, 1), .unused0=gtmv(m, 1), .command=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_command_status_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", eng_idx, command_status.add_cmd_ddr_miss, command_status.add_cmd_ddr_bin, command_status.del_cmd_ddr_bin, command_status.add_cmd_mode, command_status.del_cmd_mode, command_status.cache_flush, command_status.decr_count, command_status.nat_tbl, command_status.multihash_count, command_status.cache_hit, command_status.miss, command_status.error, command_status.busy, command_status.unused0, command_status.command);
        if(!err) ag_drv_natc_eng_command_status_set(eng_idx, &command_status);
        if(!err) ag_drv_natc_eng_command_status_get( eng_idx, &command_status);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_command_status_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", eng_idx, command_status.add_cmd_ddr_miss, command_status.add_cmd_ddr_bin, command_status.del_cmd_ddr_bin, command_status.add_cmd_mode, command_status.del_cmd_mode, command_status.cache_flush, command_status.decr_count, command_status.nat_tbl, command_status.multihash_count, command_status.cache_hit, command_status.miss, command_status.error, command_status.busy, command_status.unused0, command_status.command);
        if(err || command_status.add_cmd_ddr_miss!=gtmv(m, 1) || command_status.add_cmd_ddr_bin!=gtmv(m, 3) || command_status.del_cmd_ddr_bin!=gtmv(m, 8) || command_status.add_cmd_mode!=gtmv(m, 1) || command_status.del_cmd_mode!=gtmv(m, 1) || command_status.cache_flush!=gtmv(m, 1) || command_status.decr_count!=gtmv(m, 1) || command_status.nat_tbl!=gtmv(m, 3) || command_status.multihash_count!=gtmv(m, 4) || command_status.cache_hit!=gtmv(m, 1) || command_status.miss!=gtmv(m, 1) || command_status.error!=gtmv(m, 1) || command_status.busy!=gtmv(m, 1) || command_status.unused0!=gtmv(m, 1) || command_status.command!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t hash=gtmv(m, 32);
        if(!err) ag_drv_natc_eng_hash_get( eng_idx, &hash);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_hash_get(%u %u)\n", eng_idx, hash);
    }
    {
        uint32_t hit_count=gtmv(m, 32);
        if(!err) ag_drv_natc_eng_hit_count_get( eng_idx, &hit_count);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_hit_count_get(%u %u)\n", eng_idx, hit_count);
    }
    {
        uint32_t byte_count=gtmv(m, 32);
        if(!err) ag_drv_natc_eng_byte_count_get( eng_idx, &byte_count);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_byte_count_get(%u %u)\n", eng_idx, byte_count);
    }
    {
        uint16_t unused=gtmv(m, 16);
        uint16_t pkt_len=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_pkt_len_set(%u %u %u)\n", eng_idx, unused, pkt_len);
        if(!err) ag_drv_natc_eng_pkt_len_set(eng_idx, unused, pkt_len);
        if(!err) ag_drv_natc_eng_pkt_len_get( eng_idx, &unused, &pkt_len);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_pkt_len_get(%u %u %u)\n", eng_idx, unused, pkt_len);
        if(err || unused!=gtmv(m, 16) || pkt_len!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        natc_eng_key_result key_result = {.data={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_key_result_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", eng_idx, zero, key_result.data[0], key_result.data[1], key_result.data[2], key_result.data[3], key_result.data[4], key_result.data[5], key_result.data[6], key_result.data[7], key_result.data[8], key_result.data[9], key_result.data[10], key_result.data[11], key_result.data[12], key_result.data[13], key_result.data[14], key_result.data[15], key_result.data[16], key_result.data[17]);
        if(!err) ag_drv_natc_eng_key_result_set(eng_idx, zero, &key_result);
        if(!err) ag_drv_natc_eng_key_result_get( eng_idx, zero, &key_result);
        if(!err) bdmf_session_print(session, "ag_drv_natc_eng_key_result_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", eng_idx, zero, key_result.data[0], key_result.data[1], key_result.data[2], key_result.data[3], key_result.data[4], key_result.data[5], key_result.data[6], key_result.data[7], key_result.data[8], key_result.data[9], key_result.data[10], key_result.data[11], key_result.data[12], key_result.data[13], key_result.data[14], key_result.data[15], key_result.data[16], key_result.data[17]);
        if(err || key_result.data[0]!=gtmv(m, 32) || key_result.data[1]!=gtmv(m, 32) || key_result.data[2]!=gtmv(m, 32) || key_result.data[3]!=gtmv(m, 32) || key_result.data[4]!=gtmv(m, 32) || key_result.data[5]!=gtmv(m, 32) || key_result.data[6]!=gtmv(m, 32) || key_result.data[7]!=gtmv(m, 32) || key_result.data[8]!=gtmv(m, 32) || key_result.data[9]!=gtmv(m, 32) || key_result.data[10]!=gtmv(m, 32) || key_result.data[11]!=gtmv(m, 32) || key_result.data[12]!=gtmv(m, 32) || key_result.data[13]!=gtmv(m, 32) || key_result.data[14]!=gtmv(m, 32) || key_result.data[15]!=gtmv(m, 32) || key_result.data[16]!=gtmv(m, 32) || key_result.data[17]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_natc_eng_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_command_status : reg = &RU_REG(NATC_ENG, COMMAND_STATUS); blk = &RU_BLK(NATC_ENG); break;
    case bdmf_address_hash : reg = &RU_REG(NATC_ENG, HASH); blk = &RU_BLK(NATC_ENG); break;
    case bdmf_address_hit_count : reg = &RU_REG(NATC_ENG, HIT_COUNT); blk = &RU_BLK(NATC_ENG); break;
    case bdmf_address_byte_count : reg = &RU_REG(NATC_ENG, BYTE_COUNT); blk = &RU_BLK(NATC_ENG); break;
    case bdmf_address_pkt_len : reg = &RU_REG(NATC_ENG, PKT_LEN); blk = &RU_BLK(NATC_ENG); break;
    case bdmf_address_key_result : reg = &RU_REG(NATC_ENG, KEY_RESULT); blk = &RU_BLK(NATC_ENG); break;
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

bdmfmon_handle_t ag_drv_natc_eng_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "natc_eng"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "natc_eng", "natc_eng", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_command_status[]={
            BDMFMON_MAKE_PARM_ENUM("eng_idx", "eng_idx", eng_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("add_cmd_ddr_miss", "add_cmd_ddr_miss", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("add_cmd_ddr_bin", "add_cmd_ddr_bin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("del_cmd_ddr_bin", "del_cmd_ddr_bin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("add_cmd_mode", "add_cmd_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("del_cmd_mode", "del_cmd_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_flush", "cache_flush", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("decr_count", "decr_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nat_tbl", "nat_tbl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("multihash_count", "multihash_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cache_hit", "cache_hit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("miss", "miss", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("error", "error", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("busy", "busy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unused0", "unused0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("command", "command", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_len[]={
            BDMFMON_MAKE_PARM_ENUM("eng_idx", "eng_idx", eng_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("unused", "unused", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_len", "pkt_len", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_key_result[]={
            BDMFMON_MAKE_PARM_ENUM("eng_idx", "eng_idx", eng_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data0", "data0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data1", "data1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data2", "data2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data3", "data3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data4", "data4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data5", "data5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data6", "data6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data7", "data7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data8", "data8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data9", "data9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data10", "data10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data11", "data11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data12", "data12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data13", "data13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data14", "data14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data15", "data15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data16", "data16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data17", "data17", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="command_status", .val=cli_natc_eng_command_status, .parms=set_command_status },
            { .name="pkt_len", .val=cli_natc_eng_pkt_len, .parms=set_pkt_len },
            { .name="key_result", .val=cli_natc_eng_key_result, .parms=set_key_result },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_natc_eng_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("eng_idx", "eng_idx", eng_idx_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_key_result[]={
            BDMFMON_MAKE_PARM("eng_idx", "eng_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="command_status", .val=cli_natc_eng_command_status, .parms=set_default },
            { .name="hash", .val=cli_natc_eng_hash, .parms=set_default },
            { .name="hit_count", .val=cli_natc_eng_hit_count, .parms=set_default },
            { .name="byte_count", .val=cli_natc_eng_byte_count, .parms=set_default },
            { .name="pkt_len", .val=cli_natc_eng_pkt_len, .parms=set_default },
            { .name="key_result", .val=cli_natc_eng_key_result, .parms=set_key_result },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_eng_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_natc_eng_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("eng_idx", "eng_idx", eng_idx_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="COMMAND_STATUS" , .val=bdmf_address_command_status },
            { .name="HASH" , .val=bdmf_address_hash },
            { .name="HIT_COUNT" , .val=bdmf_address_hit_count },
            { .name="BYTE_COUNT" , .val=bdmf_address_byte_count },
            { .name="PKT_LEN" , .val=bdmf_address_pkt_len },
            { .name="KEY_RESULT" , .val=bdmf_address_key_result },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_natc_eng_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "eng_idx", eng_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

