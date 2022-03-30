// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_psram.h"

bdmf_error_t ag_drv_psram_memory_data_set(uint32_t psram_enrty, const psram_memory_data *memory_data)
{
    int i;
#ifdef VALIDATE_PARMS
    if(!memory_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((psram_enrty >= 1536))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    for (i = 0; i < 32; i++)
        RU_REG_RAM_WRITE(0, psram_enrty *32 + i, PSRAM, MEMORY_DATA, memory_data->memory_data[i]);
#if 0
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 0, PSRAM, MEMORY_DATA, memory_data->memory_data[0]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 1, PSRAM, MEMORY_DATA, memory_data->memory_data[1]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 2, PSRAM, MEMORY_DATA, memory_data->memory_data[2]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 3, PSRAM, MEMORY_DATA, memory_data->memory_data[3]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 4, PSRAM, MEMORY_DATA, memory_data->memory_data[4]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 5, PSRAM, MEMORY_DATA, memory_data->memory_data[5]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 6, PSRAM, MEMORY_DATA, memory_data->memory_data[6]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 7, PSRAM, MEMORY_DATA, memory_data->memory_data[7]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 8, PSRAM, MEMORY_DATA, memory_data->memory_data[8]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 9, PSRAM, MEMORY_DATA, memory_data->memory_data[9]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 10, PSRAM, MEMORY_DATA, memory_data->memory_data[10]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 11, PSRAM, MEMORY_DATA, memory_data->memory_data[11]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 12, PSRAM, MEMORY_DATA, memory_data->memory_data[12]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 13, PSRAM, MEMORY_DATA, memory_data->memory_data[13]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 14, PSRAM, MEMORY_DATA, memory_data->memory_data[14]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 15, PSRAM, MEMORY_DATA, memory_data->memory_data[15]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 16, PSRAM, MEMORY_DATA, memory_data->memory_data[16]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 17, PSRAM, MEMORY_DATA, memory_data->memory_data[17]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 18, PSRAM, MEMORY_DATA, memory_data->memory_data[18]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 19, PSRAM, MEMORY_DATA, memory_data->memory_data[19]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 20, PSRAM, MEMORY_DATA, memory_data->memory_data[20]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 21, PSRAM, MEMORY_DATA, memory_data->memory_data[21]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 22, PSRAM, MEMORY_DATA, memory_data->memory_data[22]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 23, PSRAM, MEMORY_DATA, memory_data->memory_data[23]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 24, PSRAM, MEMORY_DATA, memory_data->memory_data[24]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 25, PSRAM, MEMORY_DATA, memory_data->memory_data[25]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 26, PSRAM, MEMORY_DATA, memory_data->memory_data[26]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 27, PSRAM, MEMORY_DATA, memory_data->memory_data[27]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 28, PSRAM, MEMORY_DATA, memory_data->memory_data[28]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 29, PSRAM, MEMORY_DATA, memory_data->memory_data[29]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 30, PSRAM, MEMORY_DATA, memory_data->memory_data[30]);
    RU_REG_RAM_WRITE(0, psram_enrty *32 + 31, PSRAM, MEMORY_DATA, memory_data->memory_data[31]);
#endif

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_psram_memory_data_get(uint32_t psram_enrty, psram_memory_data *memory_data)
{
    int i;
#ifdef VALIDATE_PARMS
    if(!memory_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((psram_enrty >= 1536))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    for (i = 0; i < 32; i++)
        RU_REG_RAM_READ(0, psram_enrty *32 + i, PSRAM, MEMORY_DATA, memory_data->memory_data[i]);

#if 0
    RU_REG_RAM_READ(0, psram_enrty *32 + 0, PSRAM, MEMORY_DATA, memory_data->memory_data[0]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 1, PSRAM, MEMORY_DATA, memory_data->memory_data[1]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 2, PSRAM, MEMORY_DATA, memory_data->memory_data[2]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 3, PSRAM, MEMORY_DATA, memory_data->memory_data[3]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 4, PSRAM, MEMORY_DATA, memory_data->memory_data[4]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 5, PSRAM, MEMORY_DATA, memory_data->memory_data[5]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 6, PSRAM, MEMORY_DATA, memory_data->memory_data[6]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 7, PSRAM, MEMORY_DATA, memory_data->memory_data[7]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 8, PSRAM, MEMORY_DATA, memory_data->memory_data[8]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 9, PSRAM, MEMORY_DATA, memory_data->memory_data[9]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 10, PSRAM, MEMORY_DATA, memory_data->memory_data[10]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 11, PSRAM, MEMORY_DATA, memory_data->memory_data[11]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 12, PSRAM, MEMORY_DATA, memory_data->memory_data[12]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 13, PSRAM, MEMORY_DATA, memory_data->memory_data[13]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 14, PSRAM, MEMORY_DATA, memory_data->memory_data[14]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 15, PSRAM, MEMORY_DATA, memory_data->memory_data[15]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 16, PSRAM, MEMORY_DATA, memory_data->memory_data[16]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 17, PSRAM, MEMORY_DATA, memory_data->memory_data[17]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 18, PSRAM, MEMORY_DATA, memory_data->memory_data[18]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 19, PSRAM, MEMORY_DATA, memory_data->memory_data[19]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 20, PSRAM, MEMORY_DATA, memory_data->memory_data[20]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 21, PSRAM, MEMORY_DATA, memory_data->memory_data[21]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 22, PSRAM, MEMORY_DATA, memory_data->memory_data[22]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 23, PSRAM, MEMORY_DATA, memory_data->memory_data[23]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 24, PSRAM, MEMORY_DATA, memory_data->memory_data[24]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 25, PSRAM, MEMORY_DATA, memory_data->memory_data[25]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 26, PSRAM, MEMORY_DATA, memory_data->memory_data[26]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 27, PSRAM, MEMORY_DATA, memory_data->memory_data[27]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 28, PSRAM, MEMORY_DATA, memory_data->memory_data[28]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 29, PSRAM, MEMORY_DATA, memory_data->memory_data[29]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 30, PSRAM, MEMORY_DATA, memory_data->memory_data[30]);
    RU_REG_RAM_READ(0, psram_enrty *32 + 31, PSRAM, MEMORY_DATA, memory_data->memory_data[31]);
#endif

    return BDMF_ERR_OK;
}

