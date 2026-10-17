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
#include "xrdp_drv_psram_mem_ag.h"

#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_psram_mem_memory_data_set(uint8_t psram_mem_id, uint32_t psram_enrty, const psram_mem_memory_data *memory_data)
{
#ifdef VALIDATE_PARMS
    if(!memory_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((psram_mem_id >= BLOCK_ADDR_COUNT) ||
       (psram_enrty >= 4096))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 0, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[0]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 1, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[1]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 2, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[2]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 3, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[3]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 4, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[4]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 5, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[5]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 6, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[6]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 7, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[7]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 8, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[8]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 9, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[9]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 10, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[10]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 11, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[11]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 12, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[12]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 13, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[13]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 14, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[14]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 15, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[15]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 16, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[16]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 17, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[17]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 18, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[18]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 19, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[19]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 20, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[20]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 21, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[21]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 22, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[22]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 23, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[23]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 24, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[24]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 25, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[25]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 26, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[26]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 27, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[27]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 28, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[28]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 29, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[29]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 30, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[30]);
    RU_REG_RAM_WRITE(psram_mem_id, psram_enrty * 32 + 31, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[31]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_mem_memory_data_get(uint8_t psram_mem_id, uint32_t psram_enrty, psram_mem_memory_data *memory_data)
{
#ifdef VALIDATE_PARMS
    if (!memory_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((psram_mem_id >= BLOCK_ADDR_COUNT) ||
       (psram_enrty >= 4096))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 0, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[0]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 1, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[1]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 2, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[2]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 3, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[3]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 4, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[4]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 5, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[5]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 6, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[6]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 7, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[7]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 8, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[8]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 9, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[9]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 10, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[10]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 11, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[11]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 12, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[12]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 13, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[13]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 14, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[14]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 15, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[15]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 16, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[16]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 17, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[17]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 18, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[18]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 19, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[19]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 20, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[20]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 21, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[21]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 22, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[22]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 23, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[23]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 24, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[24]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 25, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[25]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 26, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[26]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 27, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[27]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 28, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[28]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 29, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[29]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 30, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[30]);
    RU_REG_RAM_READ(psram_mem_id, psram_enrty * 32 + 31, PSRAM_MEM, MEMORY_DATA, memory_data->memory_data[31]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_memory_data,
}
bdmf_address;

static int ag_drv_psram_mem_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_psram_mem_memory_data:
    {
        psram_mem_memory_data memory_data = { .memory_data = { parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber, parm[11].value.unumber, parm[12].value.unumber, parm[13].value.unumber, parm[14].value.unumber, parm[15].value.unumber, parm[16].value.unumber, parm[17].value.unumber, parm[18].value.unumber, parm[19].value.unumber, parm[20].value.unumber, parm[21].value.unumber, parm[22].value.unumber, parm[23].value.unumber, parm[24].value.unumber, parm[25].value.unumber, parm[26].value.unumber, parm[27].value.unumber, parm[28].value.unumber, parm[29].value.unumber, parm[30].value.unumber, parm[31].value.unumber, parm[32].value.unumber, parm[33].value.unumber, parm[34].value.unumber}};
        ag_err = ag_drv_psram_mem_memory_data_set(parm[1].value.unumber, parm[2].value.unumber, &memory_data);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_psram_mem_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_psram_mem_memory_data:
    {
        psram_mem_memory_data memory_data;
        ag_err = ag_drv_psram_mem_memory_data_get(parm[1].value.unumber, parm[2].value.unumber, &memory_data);
        bdmf_session_print(session, "memory_data[0] = %u = 0x%x\n", memory_data.memory_data[0], memory_data.memory_data[0]);
        bdmf_session_print(session, "memory_data[1] = %u = 0x%x\n", memory_data.memory_data[1], memory_data.memory_data[1]);
        bdmf_session_print(session, "memory_data[2] = %u = 0x%x\n", memory_data.memory_data[2], memory_data.memory_data[2]);
        bdmf_session_print(session, "memory_data[3] = %u = 0x%x\n", memory_data.memory_data[3], memory_data.memory_data[3]);
        bdmf_session_print(session, "memory_data[4] = %u = 0x%x\n", memory_data.memory_data[4], memory_data.memory_data[4]);
        bdmf_session_print(session, "memory_data[5] = %u = 0x%x\n", memory_data.memory_data[5], memory_data.memory_data[5]);
        bdmf_session_print(session, "memory_data[6] = %u = 0x%x\n", memory_data.memory_data[6], memory_data.memory_data[6]);
        bdmf_session_print(session, "memory_data[7] = %u = 0x%x\n", memory_data.memory_data[7], memory_data.memory_data[7]);
        bdmf_session_print(session, "memory_data[8] = %u = 0x%x\n", memory_data.memory_data[8], memory_data.memory_data[8]);
        bdmf_session_print(session, "memory_data[9] = %u = 0x%x\n", memory_data.memory_data[9], memory_data.memory_data[9]);
        bdmf_session_print(session, "memory_data[10] = %u = 0x%x\n", memory_data.memory_data[10], memory_data.memory_data[10]);
        bdmf_session_print(session, "memory_data[11] = %u = 0x%x\n", memory_data.memory_data[11], memory_data.memory_data[11]);
        bdmf_session_print(session, "memory_data[12] = %u = 0x%x\n", memory_data.memory_data[12], memory_data.memory_data[12]);
        bdmf_session_print(session, "memory_data[13] = %u = 0x%x\n", memory_data.memory_data[13], memory_data.memory_data[13]);
        bdmf_session_print(session, "memory_data[14] = %u = 0x%x\n", memory_data.memory_data[14], memory_data.memory_data[14]);
        bdmf_session_print(session, "memory_data[15] = %u = 0x%x\n", memory_data.memory_data[15], memory_data.memory_data[15]);
        bdmf_session_print(session, "memory_data[16] = %u = 0x%x\n", memory_data.memory_data[16], memory_data.memory_data[16]);
        bdmf_session_print(session, "memory_data[17] = %u = 0x%x\n", memory_data.memory_data[17], memory_data.memory_data[17]);
        bdmf_session_print(session, "memory_data[18] = %u = 0x%x\n", memory_data.memory_data[18], memory_data.memory_data[18]);
        bdmf_session_print(session, "memory_data[19] = %u = 0x%x\n", memory_data.memory_data[19], memory_data.memory_data[19]);
        bdmf_session_print(session, "memory_data[20] = %u = 0x%x\n", memory_data.memory_data[20], memory_data.memory_data[20]);
        bdmf_session_print(session, "memory_data[21] = %u = 0x%x\n", memory_data.memory_data[21], memory_data.memory_data[21]);
        bdmf_session_print(session, "memory_data[22] = %u = 0x%x\n", memory_data.memory_data[22], memory_data.memory_data[22]);
        bdmf_session_print(session, "memory_data[23] = %u = 0x%x\n", memory_data.memory_data[23], memory_data.memory_data[23]);
        bdmf_session_print(session, "memory_data[24] = %u = 0x%x\n", memory_data.memory_data[24], memory_data.memory_data[24]);
        bdmf_session_print(session, "memory_data[25] = %u = 0x%x\n", memory_data.memory_data[25], memory_data.memory_data[25]);
        bdmf_session_print(session, "memory_data[26] = %u = 0x%x\n", memory_data.memory_data[26], memory_data.memory_data[26]);
        bdmf_session_print(session, "memory_data[27] = %u = 0x%x\n", memory_data.memory_data[27], memory_data.memory_data[27]);
        bdmf_session_print(session, "memory_data[28] = %u = 0x%x\n", memory_data.memory_data[28], memory_data.memory_data[28]);
        bdmf_session_print(session, "memory_data[29] = %u = 0x%x\n", memory_data.memory_data[29], memory_data.memory_data[29]);
        bdmf_session_print(session, "memory_data[30] = %u = 0x%x\n", memory_data.memory_data[30], memory_data.memory_data[30]);
        bdmf_session_print(session, "memory_data[31] = %u = 0x%x\n", memory_data.memory_data[31], memory_data.memory_data[31]);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_psram_mem_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t psram_mem_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t psram_enrty = gtmv(m, 12);
        psram_mem_memory_data memory_data = {.memory_data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        bdmf_session_print(session, "ag_drv_psram_mem_memory_data_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_mem_id, psram_enrty,
            memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], 
            memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], 
            memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], 
            memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], 
            memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], 
            memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], 
            memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], 
            memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);
        ag_err = ag_drv_psram_mem_memory_data_set(psram_mem_id, psram_enrty, &memory_data);
        if (!ag_err)
            ag_err = ag_drv_psram_mem_memory_data_get(psram_mem_id, psram_enrty, &memory_data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_psram_mem_memory_data_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_mem_id, psram_enrty,
                memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], 
                memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], 
                memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], 
                memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], 
                memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], 
                memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], 
                memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], 
                memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (memory_data.memory_data[0] != gtmv(m, 32) || memory_data.memory_data[1] != gtmv(m, 32) || memory_data.memory_data[2] != gtmv(m, 32) || memory_data.memory_data[3] != gtmv(m, 32) || memory_data.memory_data[4] != gtmv(m, 32) || memory_data.memory_data[5] != gtmv(m, 32) || memory_data.memory_data[6] != gtmv(m, 32) || memory_data.memory_data[7] != gtmv(m, 32) || memory_data.memory_data[8] != gtmv(m, 32) || memory_data.memory_data[9] != gtmv(m, 32) || memory_data.memory_data[10] != gtmv(m, 32) || memory_data.memory_data[11] != gtmv(m, 32) || memory_data.memory_data[12] != gtmv(m, 32) || memory_data.memory_data[13] != gtmv(m, 32) || memory_data.memory_data[14] != gtmv(m, 32) || memory_data.memory_data[15] != gtmv(m, 32) || memory_data.memory_data[16] != gtmv(m, 32) || memory_data.memory_data[17] != gtmv(m, 32) || memory_data.memory_data[18] != gtmv(m, 32) || memory_data.memory_data[19] != gtmv(m, 32) || memory_data.memory_data[20] != gtmv(m, 32) || memory_data.memory_data[21] != gtmv(m, 32) || memory_data.memory_data[22] != gtmv(m, 32) || memory_data.memory_data[23] != gtmv(m, 32) || memory_data.memory_data[24] != gtmv(m, 32) || memory_data.memory_data[25] != gtmv(m, 32) || memory_data.memory_data[26] != gtmv(m, 32) || memory_data.memory_data[27] != gtmv(m, 32) || memory_data.memory_data[28] != gtmv(m, 32) || memory_data.memory_data[29] != gtmv(m, 32) || memory_data.memory_data[30] != gtmv(m, 32) || memory_data.memory_data[31] != gtmv(m, 32))
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
static int ag_drv_psram_mem_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t psram_mem_id = parm[2].value.unumber;
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
    case cli_psram_mem_memory_data:
    {
        uint32_t max_psram_enrty = 4096;
        uint32_t psram_enrty = gtmv(m, 12);
        psram_mem_memory_data memory_data = {.memory_data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32) } };

        if ((start_idx >= max_psram_enrty) || (stop_idx >= max_psram_enrty))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_psram_enrty);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (psram_enrty = 0; psram_enrty < max_psram_enrty; psram_enrty++)
        {
            bdmf_session_print(session, "ag_drv_psram_mem_memory_data_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_mem_id, psram_enrty,
                memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], 
                memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], 
                memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], 
                memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], 
                memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], 
                memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], 
                memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], 
                memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);
            ag_err = ag_drv_psram_mem_memory_data_set(psram_mem_id, psram_enrty, &memory_data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", psram_enrty);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        memory_data.memory_data[0] = gtmv(m, 32);
        memory_data.memory_data[1] = gtmv(m, 32);
        memory_data.memory_data[2] = gtmv(m, 32);
        memory_data.memory_data[3] = gtmv(m, 32);
        memory_data.memory_data[4] = gtmv(m, 32);
        memory_data.memory_data[5] = gtmv(m, 32);
        memory_data.memory_data[6] = gtmv(m, 32);
        memory_data.memory_data[7] = gtmv(m, 32);
        memory_data.memory_data[8] = gtmv(m, 32);
        memory_data.memory_data[9] = gtmv(m, 32);
        memory_data.memory_data[10] = gtmv(m, 32);
        memory_data.memory_data[11] = gtmv(m, 32);
        memory_data.memory_data[12] = gtmv(m, 32);
        memory_data.memory_data[13] = gtmv(m, 32);
        memory_data.memory_data[14] = gtmv(m, 32);
        memory_data.memory_data[15] = gtmv(m, 32);
        memory_data.memory_data[16] = gtmv(m, 32);
        memory_data.memory_data[17] = gtmv(m, 32);
        memory_data.memory_data[18] = gtmv(m, 32);
        memory_data.memory_data[19] = gtmv(m, 32);
        memory_data.memory_data[20] = gtmv(m, 32);
        memory_data.memory_data[21] = gtmv(m, 32);
        memory_data.memory_data[22] = gtmv(m, 32);
        memory_data.memory_data[23] = gtmv(m, 32);
        memory_data.memory_data[24] = gtmv(m, 32);
        memory_data.memory_data[25] = gtmv(m, 32);
        memory_data.memory_data[26] = gtmv(m, 32);
        memory_data.memory_data[27] = gtmv(m, 32);
        memory_data.memory_data[28] = gtmv(m, 32);
        memory_data.memory_data[29] = gtmv(m, 32);
        memory_data.memory_data[30] = gtmv(m, 32);
        memory_data.memory_data[31] = gtmv(m, 32);

        for (psram_enrty = start_idx; psram_enrty <= stop_idx; psram_enrty++)
        {
            bdmf_session_print(session, "ag_drv_psram_mem_memory_data_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_mem_id, psram_enrty,
                memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], 
                memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], 
                memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], 
                memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], 
                memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], 
                memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], 
                memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], 
                memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);
            ag_err = ag_drv_psram_mem_memory_data_set(psram_mem_id, psram_enrty, &memory_data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", psram_enrty);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (psram_enrty = 0; psram_enrty < max_psram_enrty; psram_enrty++)
        {
            if (psram_enrty < start_idx || psram_enrty > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_psram_mem_memory_data_get(psram_mem_id, psram_enrty, &memory_data);

            bdmf_session_print(session, "ag_drv_psram_mem_memory_data_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", psram_mem_id, psram_enrty,
                memory_data.memory_data[0], memory_data.memory_data[1], memory_data.memory_data[2], memory_data.memory_data[3], 
                memory_data.memory_data[4], memory_data.memory_data[5], memory_data.memory_data[6], memory_data.memory_data[7], 
                memory_data.memory_data[8], memory_data.memory_data[9], memory_data.memory_data[10], memory_data.memory_data[11], 
                memory_data.memory_data[12], memory_data.memory_data[13], memory_data.memory_data[14], memory_data.memory_data[15], 
                memory_data.memory_data[16], memory_data.memory_data[17], memory_data.memory_data[18], memory_data.memory_data[19], 
                memory_data.memory_data[20], memory_data.memory_data[21], memory_data.memory_data[22], memory_data.memory_data[23], 
                memory_data.memory_data[24], memory_data.memory_data[25], memory_data.memory_data[26], memory_data.memory_data[27], 
                memory_data.memory_data[28], memory_data.memory_data[29], memory_data.memory_data[30], memory_data.memory_data[31]);

            if (memory_data.memory_data[0] != gtmv(m, 32) || 
                memory_data.memory_data[1] != gtmv(m, 32) || 
                memory_data.memory_data[2] != gtmv(m, 32) || 
                memory_data.memory_data[3] != gtmv(m, 32) || 
                memory_data.memory_data[4] != gtmv(m, 32) || 
                memory_data.memory_data[5] != gtmv(m, 32) || 
                memory_data.memory_data[6] != gtmv(m, 32) || 
                memory_data.memory_data[7] != gtmv(m, 32) || 
                memory_data.memory_data[8] != gtmv(m, 32) || 
                memory_data.memory_data[9] != gtmv(m, 32) || 
                memory_data.memory_data[10] != gtmv(m, 32) || 
                memory_data.memory_data[11] != gtmv(m, 32) || 
                memory_data.memory_data[12] != gtmv(m, 32) || 
                memory_data.memory_data[13] != gtmv(m, 32) || 
                memory_data.memory_data[14] != gtmv(m, 32) || 
                memory_data.memory_data[15] != gtmv(m, 32) || 
                memory_data.memory_data[16] != gtmv(m, 32) || 
                memory_data.memory_data[17] != gtmv(m, 32) || 
                memory_data.memory_data[18] != gtmv(m, 32) || 
                memory_data.memory_data[19] != gtmv(m, 32) || 
                memory_data.memory_data[20] != gtmv(m, 32) || 
                memory_data.memory_data[21] != gtmv(m, 32) || 
                memory_data.memory_data[22] != gtmv(m, 32) || 
                memory_data.memory_data[23] != gtmv(m, 32) || 
                memory_data.memory_data[24] != gtmv(m, 32) || 
                memory_data.memory_data[25] != gtmv(m, 32) || 
                memory_data.memory_data[26] != gtmv(m, 32) || 
                memory_data.memory_data[27] != gtmv(m, 32) || 
                memory_data.memory_data[28] != gtmv(m, 32) || 
                memory_data.memory_data[29] != gtmv(m, 32) || 
                memory_data.memory_data[30] != gtmv(m, 32) || 
                memory_data.memory_data[31] != gtmv(m, 32) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", psram_enrty);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of memory_data completed. Number of tested entries %u.\n", max_psram_enrty);
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
static int ag_drv_psram_mem_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_memory_data: reg = &RU_REG(PSRAM_MEM, MEMORY_DATA); blk = &RU_BLK(PSRAM_MEM); break;
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

bdmfmon_handle_t ag_drv_psram_mem_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "psram_mem", "psram_mem", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_memory_data[] = {
            BDMFMON_MAKE_PARM("psram_mem_id", "psram_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_enrty", "psram_enrty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("memory_data0", "memory_data0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data1", "memory_data1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data2", "memory_data2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data3", "memory_data3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data4", "memory_data4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data5", "memory_data5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data6", "memory_data6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data7", "memory_data7", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data8", "memory_data8", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data9", "memory_data9", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data10", "memory_data10", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data11", "memory_data11", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data12", "memory_data12", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data13", "memory_data13", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data14", "memory_data14", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data15", "memory_data15", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data16", "memory_data16", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data17", "memory_data17", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data18", "memory_data18", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data19", "memory_data19", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data20", "memory_data20", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data21", "memory_data21", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data22", "memory_data22", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data23", "memory_data23", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data24", "memory_data24", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data25", "memory_data25", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data26", "memory_data26", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data27", "memory_data27", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data28", "memory_data28", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data29", "memory_data29", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data30", "memory_data30", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("memory_data31", "memory_data31", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "memory_data", .val = cli_psram_mem_memory_data, .parms = set_memory_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_psram_mem_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_memory_data[] = {
            BDMFMON_MAKE_PARM("psram_mem_id", "psram_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_enrty", "psram_enrty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "memory_data", .val = cli_psram_mem_memory_data, .parms = get_memory_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_psram_mem_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_psram_mem_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("psram_mem_id", "psram_mem_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("psram_mem_id", "psram_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "memory_data", .val = cli_psram_mem_memory_data, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_psram_mem_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "MEMORY_DATA", .val = bdmf_address_memory_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_psram_mem_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "psram_mem_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
