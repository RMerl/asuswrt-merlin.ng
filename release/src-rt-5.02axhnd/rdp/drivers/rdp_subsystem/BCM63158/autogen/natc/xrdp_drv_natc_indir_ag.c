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
#include "xrdp_drv_natc_indir_ag.h"

bdmf_error_t ag_drv_natc_indir_addr_set(bdmf_boolean w_r, uint16_t natc_entry)
{
    uint32_t reg_c_indir_addr_reg=0;

#ifdef VALIDATE_PARMS
    if((w_r >= _1BITS_MAX_VAL_) ||
       (natc_entry >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_c_indir_addr_reg = RU_FIELD_SET(0, NATC_INDIR, C_INDIR_ADDR_REG, W_R, reg_c_indir_addr_reg, w_r);
    reg_c_indir_addr_reg = RU_FIELD_SET(0, NATC_INDIR, C_INDIR_ADDR_REG, NATC_ENTRY, reg_c_indir_addr_reg, natc_entry);

    RU_REG_WRITE(0, NATC_INDIR, C_INDIR_ADDR_REG, reg_c_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_addr_get(bdmf_boolean *w_r, uint16_t *natc_entry)
{
    uint32_t reg_c_indir_addr_reg;

#ifdef VALIDATE_PARMS
    if(!w_r || !natc_entry)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_INDIR, C_INDIR_ADDR_REG, reg_c_indir_addr_reg);

    *w_r = RU_FIELD_GET(0, NATC_INDIR, C_INDIR_ADDR_REG, W_R, reg_c_indir_addr_reg);
    *natc_entry = RU_FIELD_GET(0, NATC_INDIR, C_INDIR_ADDR_REG, NATC_ENTRY, reg_c_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_data_set(uint8_t zero, const natc_indir_data *data)
{
#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, zero *37 + 0, NATC_INDIR, C_INDIR_DATA_REG, data->data[0]);
    RU_REG_RAM_WRITE(0, zero *37 + 1, NATC_INDIR, C_INDIR_DATA_REG, data->data[1]);
    RU_REG_RAM_WRITE(0, zero *37 + 2, NATC_INDIR, C_INDIR_DATA_REG, data->data[2]);
    RU_REG_RAM_WRITE(0, zero *37 + 3, NATC_INDIR, C_INDIR_DATA_REG, data->data[3]);
    RU_REG_RAM_WRITE(0, zero *37 + 4, NATC_INDIR, C_INDIR_DATA_REG, data->data[4]);
    RU_REG_RAM_WRITE(0, zero *37 + 5, NATC_INDIR, C_INDIR_DATA_REG, data->data[5]);
    RU_REG_RAM_WRITE(0, zero *37 + 6, NATC_INDIR, C_INDIR_DATA_REG, data->data[6]);
    RU_REG_RAM_WRITE(0, zero *37 + 7, NATC_INDIR, C_INDIR_DATA_REG, data->data[7]);
    RU_REG_RAM_WRITE(0, zero *37 + 8, NATC_INDIR, C_INDIR_DATA_REG, data->data[8]);
    RU_REG_RAM_WRITE(0, zero *37 + 9, NATC_INDIR, C_INDIR_DATA_REG, data->data[9]);
    RU_REG_RAM_WRITE(0, zero *37 + 10, NATC_INDIR, C_INDIR_DATA_REG, data->data[10]);
    RU_REG_RAM_WRITE(0, zero *37 + 11, NATC_INDIR, C_INDIR_DATA_REG, data->data[11]);
    RU_REG_RAM_WRITE(0, zero *37 + 12, NATC_INDIR, C_INDIR_DATA_REG, data->data[12]);
    RU_REG_RAM_WRITE(0, zero *37 + 13, NATC_INDIR, C_INDIR_DATA_REG, data->data[13]);
    RU_REG_RAM_WRITE(0, zero *37 + 14, NATC_INDIR, C_INDIR_DATA_REG, data->data[14]);
    RU_REG_RAM_WRITE(0, zero *37 + 15, NATC_INDIR, C_INDIR_DATA_REG, data->data[15]);
    RU_REG_RAM_WRITE(0, zero *37 + 16, NATC_INDIR, C_INDIR_DATA_REG, data->data[16]);
    RU_REG_RAM_WRITE(0, zero *37 + 17, NATC_INDIR, C_INDIR_DATA_REG, data->data[17]);
    RU_REG_RAM_WRITE(0, zero *37 + 18, NATC_INDIR, C_INDIR_DATA_REG, data->data[18]);
    RU_REG_RAM_WRITE(0, zero *37 + 19, NATC_INDIR, C_INDIR_DATA_REG, data->data[19]);
    RU_REG_RAM_WRITE(0, zero *37 + 20, NATC_INDIR, C_INDIR_DATA_REG, data->data[20]);
    RU_REG_RAM_WRITE(0, zero *37 + 21, NATC_INDIR, C_INDIR_DATA_REG, data->data[21]);
    RU_REG_RAM_WRITE(0, zero *37 + 22, NATC_INDIR, C_INDIR_DATA_REG, data->data[22]);
    RU_REG_RAM_WRITE(0, zero *37 + 23, NATC_INDIR, C_INDIR_DATA_REG, data->data[23]);
    RU_REG_RAM_WRITE(0, zero *37 + 24, NATC_INDIR, C_INDIR_DATA_REG, data->data[24]);
    RU_REG_RAM_WRITE(0, zero *37 + 25, NATC_INDIR, C_INDIR_DATA_REG, data->data[25]);
    RU_REG_RAM_WRITE(0, zero *37 + 26, NATC_INDIR, C_INDIR_DATA_REG, data->data[26]);
    RU_REG_RAM_WRITE(0, zero *37 + 27, NATC_INDIR, C_INDIR_DATA_REG, data->data[27]);
    RU_REG_RAM_WRITE(0, zero *37 + 28, NATC_INDIR, C_INDIR_DATA_REG, data->data[28]);
    RU_REG_RAM_WRITE(0, zero *37 + 29, NATC_INDIR, C_INDIR_DATA_REG, data->data[29]);
    RU_REG_RAM_WRITE(0, zero *37 + 30, NATC_INDIR, C_INDIR_DATA_REG, data->data[30]);
    RU_REG_RAM_WRITE(0, zero *37 + 31, NATC_INDIR, C_INDIR_DATA_REG, data->data[31]);
    RU_REG_RAM_WRITE(0, zero *37 + 32, NATC_INDIR, C_INDIR_DATA_REG, data->data[32]);
    RU_REG_RAM_WRITE(0, zero *37 + 33, NATC_INDIR, C_INDIR_DATA_REG, data->data[33]);
    RU_REG_RAM_WRITE(0, zero *37 + 34, NATC_INDIR, C_INDIR_DATA_REG, data->data[34]);
    RU_REG_RAM_WRITE(0, zero *37 + 35, NATC_INDIR, C_INDIR_DATA_REG, data->data[35]);
    RU_REG_RAM_WRITE(0, zero *37 + 36, NATC_INDIR, C_INDIR_DATA_REG, data->data[36]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_data_get(uint8_t zero, natc_indir_data *data)
{
#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero *37 + 0, NATC_INDIR, C_INDIR_DATA_REG, data->data[0]);
    RU_REG_RAM_READ(0, zero *37 + 1, NATC_INDIR, C_INDIR_DATA_REG, data->data[1]);
    RU_REG_RAM_READ(0, zero *37 + 2, NATC_INDIR, C_INDIR_DATA_REG, data->data[2]);
    RU_REG_RAM_READ(0, zero *37 + 3, NATC_INDIR, C_INDIR_DATA_REG, data->data[3]);
    RU_REG_RAM_READ(0, zero *37 + 4, NATC_INDIR, C_INDIR_DATA_REG, data->data[4]);
    RU_REG_RAM_READ(0, zero *37 + 5, NATC_INDIR, C_INDIR_DATA_REG, data->data[5]);
    RU_REG_RAM_READ(0, zero *37 + 6, NATC_INDIR, C_INDIR_DATA_REG, data->data[6]);
    RU_REG_RAM_READ(0, zero *37 + 7, NATC_INDIR, C_INDIR_DATA_REG, data->data[7]);
    RU_REG_RAM_READ(0, zero *37 + 8, NATC_INDIR, C_INDIR_DATA_REG, data->data[8]);
    RU_REG_RAM_READ(0, zero *37 + 9, NATC_INDIR, C_INDIR_DATA_REG, data->data[9]);
    RU_REG_RAM_READ(0, zero *37 + 10, NATC_INDIR, C_INDIR_DATA_REG, data->data[10]);
    RU_REG_RAM_READ(0, zero *37 + 11, NATC_INDIR, C_INDIR_DATA_REG, data->data[11]);
    RU_REG_RAM_READ(0, zero *37 + 12, NATC_INDIR, C_INDIR_DATA_REG, data->data[12]);
    RU_REG_RAM_READ(0, zero *37 + 13, NATC_INDIR, C_INDIR_DATA_REG, data->data[13]);
    RU_REG_RAM_READ(0, zero *37 + 14, NATC_INDIR, C_INDIR_DATA_REG, data->data[14]);
    RU_REG_RAM_READ(0, zero *37 + 15, NATC_INDIR, C_INDIR_DATA_REG, data->data[15]);
    RU_REG_RAM_READ(0, zero *37 + 16, NATC_INDIR, C_INDIR_DATA_REG, data->data[16]);
    RU_REG_RAM_READ(0, zero *37 + 17, NATC_INDIR, C_INDIR_DATA_REG, data->data[17]);
    RU_REG_RAM_READ(0, zero *37 + 18, NATC_INDIR, C_INDIR_DATA_REG, data->data[18]);
    RU_REG_RAM_READ(0, zero *37 + 19, NATC_INDIR, C_INDIR_DATA_REG, data->data[19]);
    RU_REG_RAM_READ(0, zero *37 + 20, NATC_INDIR, C_INDIR_DATA_REG, data->data[20]);
    RU_REG_RAM_READ(0, zero *37 + 21, NATC_INDIR, C_INDIR_DATA_REG, data->data[21]);
    RU_REG_RAM_READ(0, zero *37 + 22, NATC_INDIR, C_INDIR_DATA_REG, data->data[22]);
    RU_REG_RAM_READ(0, zero *37 + 23, NATC_INDIR, C_INDIR_DATA_REG, data->data[23]);
    RU_REG_RAM_READ(0, zero *37 + 24, NATC_INDIR, C_INDIR_DATA_REG, data->data[24]);
    RU_REG_RAM_READ(0, zero *37 + 25, NATC_INDIR, C_INDIR_DATA_REG, data->data[25]);
    RU_REG_RAM_READ(0, zero *37 + 26, NATC_INDIR, C_INDIR_DATA_REG, data->data[26]);
    RU_REG_RAM_READ(0, zero *37 + 27, NATC_INDIR, C_INDIR_DATA_REG, data->data[27]);
    RU_REG_RAM_READ(0, zero *37 + 28, NATC_INDIR, C_INDIR_DATA_REG, data->data[28]);
    RU_REG_RAM_READ(0, zero *37 + 29, NATC_INDIR, C_INDIR_DATA_REG, data->data[29]);
    RU_REG_RAM_READ(0, zero *37 + 30, NATC_INDIR, C_INDIR_DATA_REG, data->data[30]);
    RU_REG_RAM_READ(0, zero *37 + 31, NATC_INDIR, C_INDIR_DATA_REG, data->data[31]);
    RU_REG_RAM_READ(0, zero *37 + 32, NATC_INDIR, C_INDIR_DATA_REG, data->data[32]);
    RU_REG_RAM_READ(0, zero *37 + 33, NATC_INDIR, C_INDIR_DATA_REG, data->data[33]);
    RU_REG_RAM_READ(0, zero *37 + 34, NATC_INDIR, C_INDIR_DATA_REG, data->data[34]);
    RU_REG_RAM_READ(0, zero *37 + 35, NATC_INDIR, C_INDIR_DATA_REG, data->data[35]);
    RU_REG_RAM_READ(0, zero *37 + 36, NATC_INDIR, C_INDIR_DATA_REG, data->data[36]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_c_indir_addr_reg,
    bdmf_address_c_indir_data_reg,
}
bdmf_address;

static int bcm_natc_indir_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_indir_addr:
        err = ag_drv_natc_indir_addr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_natc_indir_data:
    {
        natc_indir_data data = { .data = { parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber, parm[34].value.unumber, parm[35].value.unumber, parm[36].value.unumber, parm[37].value.unumber, parm[38].value.unumber}};
        err = ag_drv_natc_indir_data_set(parm[1].value.unumber, &data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_natc_indir_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_natc_indir_addr:
    {
        bdmf_boolean w_r;
        uint16_t natc_entry;
        err = ag_drv_natc_indir_addr_get(&w_r, &natc_entry);
        bdmf_session_print(session, "w_r = %u (0x%x)\n", w_r, w_r);
        bdmf_session_print(session, "natc_entry = %u (0x%x)\n", natc_entry, natc_entry);
        break;
    }
    case cli_natc_indir_data:
    {
        natc_indir_data data;
        err = ag_drv_natc_indir_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data[0] = %u (0x%x)\n", data.data[0], data.data[0]);
        bdmf_session_print(session, "data[1] = %u (0x%x)\n", data.data[1], data.data[1]);
        bdmf_session_print(session, "data[2] = %u (0x%x)\n", data.data[2], data.data[2]);
        bdmf_session_print(session, "data[3] = %u (0x%x)\n", data.data[3], data.data[3]);
        bdmf_session_print(session, "data[4] = %u (0x%x)\n", data.data[4], data.data[4]);
        bdmf_session_print(session, "data[5] = %u (0x%x)\n", data.data[5], data.data[5]);
        bdmf_session_print(session, "data[6] = %u (0x%x)\n", data.data[6], data.data[6]);
        bdmf_session_print(session, "data[7] = %u (0x%x)\n", data.data[7], data.data[7]);
        bdmf_session_print(session, "data[8] = %u (0x%x)\n", data.data[8], data.data[8]);
        bdmf_session_print(session, "data[9] = %u (0x%x)\n", data.data[9], data.data[9]);
        bdmf_session_print(session, "data[10] = %u (0x%x)\n", data.data[10], data.data[10]);
        bdmf_session_print(session, "data[11] = %u (0x%x)\n", data.data[11], data.data[11]);
        bdmf_session_print(session, "data[12] = %u (0x%x)\n", data.data[12], data.data[12]);
        bdmf_session_print(session, "data[13] = %u (0x%x)\n", data.data[13], data.data[13]);
        bdmf_session_print(session, "data[14] = %u (0x%x)\n", data.data[14], data.data[14]);
        bdmf_session_print(session, "data[15] = %u (0x%x)\n", data.data[15], data.data[15]);
        bdmf_session_print(session, "data[16] = %u (0x%x)\n", data.data[16], data.data[16]);
        bdmf_session_print(session, "data[17] = %u (0x%x)\n", data.data[17], data.data[17]);
        bdmf_session_print(session, "data[18] = %u (0x%x)\n", data.data[18], data.data[18]);
        bdmf_session_print(session, "data[19] = %u (0x%x)\n", data.data[19], data.data[19]);
        bdmf_session_print(session, "data[20] = %u (0x%x)\n", data.data[20], data.data[20]);
        bdmf_session_print(session, "data[21] = %u (0x%x)\n", data.data[21], data.data[21]);
        bdmf_session_print(session, "data[22] = %u (0x%x)\n", data.data[22], data.data[22]);
        bdmf_session_print(session, "data[23] = %u (0x%x)\n", data.data[23], data.data[23]);
        bdmf_session_print(session, "data[24] = %u (0x%x)\n", data.data[24], data.data[24]);
        bdmf_session_print(session, "data[25] = %u (0x%x)\n", data.data[25], data.data[25]);
        bdmf_session_print(session, "data[26] = %u (0x%x)\n", data.data[26], data.data[26]);
        bdmf_session_print(session, "data[27] = %u (0x%x)\n", data.data[27], data.data[27]);
        bdmf_session_print(session, "data[28] = %u (0x%x)\n", data.data[28], data.data[28]);
        bdmf_session_print(session, "data[29] = %u (0x%x)\n", data.data[29], data.data[29]);
        bdmf_session_print(session, "data[30] = %u (0x%x)\n", data.data[30], data.data[30]);
        bdmf_session_print(session, "data[31] = %u (0x%x)\n", data.data[31], data.data[31]);
        bdmf_session_print(session, "data[32] = %u (0x%x)\n", data.data[32], data.data[32]);
        bdmf_session_print(session, "data[33] = %u (0x%x)\n", data.data[33], data.data[33]);
        bdmf_session_print(session, "data[34] = %u (0x%x)\n", data.data[34], data.data[34]);
        bdmf_session_print(session, "data[35] = %u (0x%x)\n", data.data[35], data.data[35]);
        bdmf_session_print(session, "data[36] = %u (0x%x)\n", data.data[36], data.data[36]);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_natc_indir_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean w_r=gtmv(m, 1);
        uint16_t natc_entry=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_natc_indir_addr_set( %u %u)\n", w_r, natc_entry);
        if(!err) ag_drv_natc_indir_addr_set(w_r, natc_entry);
        if(!err) ag_drv_natc_indir_addr_get( &w_r, &natc_entry);
        if(!err) bdmf_session_print(session, "ag_drv_natc_indir_addr_get( %u %u)\n", w_r, natc_entry);
        if(err || w_r!=gtmv(m, 1) || natc_entry!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        natc_indir_data data = {.data={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_natc_indir_data_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, data.data[0], data.data[1], data.data[2], data.data[3], data.data[4], data.data[5], data.data[6], data.data[7], data.data[8], data.data[9], data.data[10], data.data[11], data.data[12], data.data[13], data.data[14], data.data[15], data.data[16], data.data[17], data.data[18], data.data[19], data.data[20], data.data[21], data.data[22], data.data[23], data.data[24], data.data[25], data.data[26], data.data[27], data.data[28], data.data[29], data.data[30], data.data[31], data.data[32], data.data[33], data.data[34], data.data[35], data.data[36]);
        if(!err) ag_drv_natc_indir_data_set(zero, &data);
        if(!err) ag_drv_natc_indir_data_get( zero, &data);
        if(!err) bdmf_session_print(session, "ag_drv_natc_indir_data_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", zero, data.data[0], data.data[1], data.data[2], data.data[3], data.data[4], data.data[5], data.data[6], data.data[7], data.data[8], data.data[9], data.data[10], data.data[11], data.data[12], data.data[13], data.data[14], data.data[15], data.data[16], data.data[17], data.data[18], data.data[19], data.data[20], data.data[21], data.data[22], data.data[23], data.data[24], data.data[25], data.data[26], data.data[27], data.data[28], data.data[29], data.data[30], data.data[31], data.data[32], data.data[33], data.data[34], data.data[35], data.data[36]);
        if(err || data.data[0]!=gtmv(m, 32) || data.data[1]!=gtmv(m, 32) || data.data[2]!=gtmv(m, 32) || data.data[3]!=gtmv(m, 32) || data.data[4]!=gtmv(m, 32) || data.data[5]!=gtmv(m, 32) || data.data[6]!=gtmv(m, 32) || data.data[7]!=gtmv(m, 32) || data.data[8]!=gtmv(m, 32) || data.data[9]!=gtmv(m, 32) || data.data[10]!=gtmv(m, 32) || data.data[11]!=gtmv(m, 32) || data.data[12]!=gtmv(m, 32) || data.data[13]!=gtmv(m, 32) || data.data[14]!=gtmv(m, 32) || data.data[15]!=gtmv(m, 32) || data.data[16]!=gtmv(m, 32) || data.data[17]!=gtmv(m, 32) || data.data[18]!=gtmv(m, 32) || data.data[19]!=gtmv(m, 32) || data.data[20]!=gtmv(m, 32) || data.data[21]!=gtmv(m, 32) || data.data[22]!=gtmv(m, 32) || data.data[23]!=gtmv(m, 32) || data.data[24]!=gtmv(m, 32) || data.data[25]!=gtmv(m, 32) || data.data[26]!=gtmv(m, 32) || data.data[27]!=gtmv(m, 32) || data.data[28]!=gtmv(m, 32) || data.data[29]!=gtmv(m, 32) || data.data[30]!=gtmv(m, 32) || data.data[31]!=gtmv(m, 32) || data.data[32]!=gtmv(m, 32) || data.data[33]!=gtmv(m, 32) || data.data[34]!=gtmv(m, 32) || data.data[35]!=gtmv(m, 32) || data.data[36]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_natc_indir_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_c_indir_addr_reg : reg = &RU_REG(NATC_INDIR, C_INDIR_ADDR_REG); blk = &RU_BLK(NATC_INDIR); break;
    case bdmf_address_c_indir_data_reg : reg = &RU_REG(NATC_INDIR, C_INDIR_DATA_REG); blk = &RU_BLK(NATC_INDIR); break;
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

bdmfmon_handle_t ag_drv_natc_indir_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "natc_indir"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "natc_indir", "natc_indir", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_addr[]={
            BDMFMON_MAKE_PARM("w_r", "w_r", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("natc_entry", "natc_entry", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data[]={
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
            BDMFMON_MAKE_PARM("data18", "data18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data19", "data19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data20", "data20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data21", "data21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data22", "data22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data23", "data23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data24", "data24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data25", "data25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data26", "data26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data27", "data27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data28", "data28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data29", "data29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data30", "data30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data31", "data31", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data32", "data32", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data33", "data33", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data34", "data34", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data35", "data35", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data36", "data36", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="addr", .val=cli_natc_indir_addr, .parms=set_addr },
            { .name="data", .val=cli_natc_indir_data, .parms=set_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_natc_indir_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="addr", .val=cli_natc_indir_addr, .parms=set_default },
            { .name="data", .val=cli_natc_indir_data, .parms=set_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_indir_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_natc_indir_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="C_INDIR_ADDR_REG" , .val=bdmf_address_c_indir_addr_reg },
            { .name="C_INDIR_DATA_REG" , .val=bdmf_address_c_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_natc_indir_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

