/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
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

#include "rdp_subsystem_common.h"
#include "rdp_drv_cnpl.h"
#include "rdp_common.h"
#include "rdd_defs.h"
#include "XRDP_AG.h"

#ifndef _CFE_
DEFINE_BDMF_FASTLOCK(counter_read_lock);
#endif

/*  
    counter configuration example:
        call the function: ag_drv_cnpl_counter_cfg_set to set up a new counter group 
            cnt_loc_profile: group number (0-15)
            cn_double: set each counter to be single or double (boolean)
            cn0_byts: number of bytes for each counter (1, 2, 4) 
            ba: base address for the counter group (10bit in 8B resolution)
            wrap: freeze or wrap around (boolean)
            clr: clear on read (boolean)

    counter read example:
        1. send read command by calling the function: drv_cnpl_counter_read_command_set
            group: choose the counter group (4bit)
            cntr: choose start counter (14bit)
            size: number of counters to read (8bit)
        2. check status register: ag_drv_cnpl_sw_stat_get
            cn_rd_st: counter read status (0, 1, 2)
        3. read the result from: ag_drv_cnpl_block_sw_if_sw_cnt_rd_get
            rd_idx: read bytes rd_idx*4--rd_idx*4+3 (0-7)
*/

/*
    policer configuration example:
        1. memory reset
        2. call the function: ag_drv_cnpl_policer_cfg_set to set up a new policer
            conf_idx: profile number (0, 1)
            bk_ba: bucket base address (10bit in 8B resolution)
            pa_ba: parameters base address (10bit)
            pl_double: set each policer to be single or double (boolean)
            pl_st: first policer in the group (8bit)
            pl_end: last policer in the group (8bit)
        3. call the function: ag_drv_cnpl_block_policers_configurations_per_up_set to start periodic update
            en: periodic update enable (boolean)
            N: period in 8k cycles quanta (8bit)

    policer read example:
        1. send read command by calling the function: drv_cnpl_policer_read_command_set
            group: choose the policer group (2bit)
            policer_num: choose start policer (8bit)
            reset_after_read: set reset (boolean)
        2. check status register: ag_drv_cnpl_sw_stat_get
            pl_rd_st: policer read status (0, 1, 2)
        3. read the result from: ag_drv_cnpl_block_sw_if_sw_pl_rd_get
            bucket: read bytes bucket*4--bucket*4+3 (0-7)

    policer police example:
        1. send police command by calling the function: drv_cnpl_policer_police_command_set
            group: choose the policer group (2bit)
            policer_num: choose start policer (8bit)
            packet_len: packet length (16bit)
        2. check status register: ag_drv_cnpl_sw_stat_get
            pl_plc_st: policer police status (0, 1, 2)
        3. read the color from: ag_drv_cnpl_block_sw_if_sw_pl_rslt_get
*/

bdmf_error_t drv_cnpl_policer_police(uint8_t *result, uint8_t group, uint8_t policer_num, uint16_t packet_len)
{
    uint16_t time_out = 0;
    bdmf_error_t rc;
    cnpl_sw_stat read_status;

    /* set read request to the policer */
    rc = drv_cnpl_policer_police_command_set(group, policer_num, packet_len);

    /* wait for read done */
    time_out = 0;
    while (!rc && time_out < CNPL_READ_TIMEOUT)
    {
        rc = ag_drv_cnpl_sw_stat_get(&read_status);
        if (rc || read_status.pl_plc_st == CNPL_READ_DONE)
            break;

        time_out++;
    }
    if (time_out == CNPL_READ_TIMEOUT)
        rc = BDMF_ERR_PARM;

    /* read the policer data */
    rc = rc ? rc : ag_drv_cnpl_sw_if_sw_pl_rslt_get(result);

    return rc;
}

bdmf_error_t drv_cnpl_policer_read(void *policers, uint8_t group, uint8_t num_of_policers, bdmf_boolean reset)
{
    uint16_t time_out = 0;
    bdmf_error_t rc;
    cnpl_sw_stat read_status;
    cnpl_policer_cfg policer_cfg;

    rc = ag_drv_cnpl_policer_cfg_get(group, &policer_cfg);

    /* check burst size */
    if (num_of_policers * (policer_cfg.pl_double + 1) > CNPL_READ_POLICER_BUFFER)
        rc = BDMF_ERR_PARM;

    /* set read request to the policer */
    rc = rc ? rc : drv_cnpl_policer_read_command_set(group, num_of_policers, reset);

    /* wait for read done */
    time_out = 0;
    while (!rc && time_out < CNPL_READ_TIMEOUT)
    {
        rc = ag_drv_cnpl_sw_stat_get(&read_status);
        if (rc || read_status.pl_rd_st == CNPL_READ_DONE)
            break;

        time_out++;
    }
    if (time_out == CNPL_READ_TIMEOUT)
        rc = BDMF_ERR_PARM;

    /* read the policers data */
    rc = rc ? rc : drv_cnpl_policer_read_command_get(policers, num_of_policers, !policer_cfg.pl_double);

    return rc;
}

bdmf_error_t drv_cnpl_policer_read_command_get(void *policers, uint8_t num_of_policers, bdmf_boolean single)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t size, i;
    uint32_t *policer_ptr = (uint32_t *)policers;

    /* bucket size is 4B or 8B */
    if (single)
        size = num_of_policers;
    else
        size = num_of_policers * 2;

    for (i = 0; !rc && i < size && i < CNPL_READ_POLICER_BUFFER; i++)
        rc = ag_drv_cnpl_sw_if_sw_cnt_rd_get(i, &policer_ptr[i]);

    return rc;
}

bdmf_error_t drv_cnpl_counter_clr(uint8_t group, uint32_t cntr_id)
{
    uint8_t cntr_bytes_num;
    uint32_t cntr_mem_offset;
    bdmf_error_t rc;
    cnpl_counter_cfg counter_cfg = {};
    uint8_t tmp[CNPL_READ_COUNTER_BUFFER] = {};

    rc = ag_drv_cnpl_counter_cfg_get(group, &counter_cfg);
    if (rc)
    {
        return rc;
    }

    /* check burst size */
    if (counter_cfg.cn0_byts > 0)
        cntr_bytes_num = (counter_cfg.cn_double + 1) * (counter_cfg.cn0_byts * 2);
    else
        cntr_bytes_num = (counter_cfg.cn_double + 1);

    cntr_mem_offset = (counter_cfg.ba <<3) + (cntr_id * cntr_bytes_num);
    MWRITE_BLK_8((uint32_t *)DEVICE_ADDRESS(RU_BLK(CNPL).addr[0] + cntr_mem_offset), tmp, cntr_bytes_num);
    return BDMF_ERR_OK;
}

bdmf_error_t drv_cnpl_counter_set(uint8_t group, uint32_t cntr_id, uint8_t value)
{
    uint8_t cntr_bytes_num;
    uint32_t cntr_mem_offset;
    bdmf_error_t rc;
    cnpl_counter_cfg counter_cfg = {};
    uint8_t tmp[CNPL_READ_COUNTER_BUFFER] = {0, 0, value, 0};

#ifndef _CFE_
    bdmf_fastlock_lock(&counter_read_lock);
#endif

    rc = ag_drv_cnpl_counter_cfg_get(group, &counter_cfg);
    if (rc)
    {
#ifndef _CFE_
        bdmf_fastlock_unlock(&counter_read_lock);
#endif 
        return rc;
    }

    /* check burst size */
    if (counter_cfg.cn0_byts > 0) 
        cntr_bytes_num = (counter_cfg.cn_double + 1) * (counter_cfg.cn0_byts * 2);
    else
        cntr_bytes_num = (counter_cfg.cn_double + 1);

    cntr_mem_offset = (counter_cfg.ba << 3) + (cntr_id * cntr_bytes_num);

    ag_drv_cnpl_memory_data_set((cntr_mem_offset / sizeof(uint32_t)), *(uint32_t *)tmp);

#ifndef _CFE_
    bdmf_fastlock_unlock(&counter_read_lock);
#endif

    return BDMF_ERR_OK;
}

bdmf_error_t drv_cnpl_counter_read(void *counters, uint8_t group, uint16_t start_counter, uint8_t num_of_counters)
{    
    bdmf_error_t rc = BDMF_ERR_OK;
    cnpl_counter_cfg counter_cfg = {};

#if !defined(RDP_SIM) && !defined(XRDP_EMULATION)
    cnpl_sw_stat read_status;
    uint16_t time_out = 0, burst;
#endif
    
#ifndef _CFE_
    bdmf_fastlock_lock(&counter_read_lock);
#endif    
    
#if !defined(RDP_SIM) && !defined(XRDP_EMULATION)
    rc = ag_drv_cnpl_counter_cfg_get(group, &counter_cfg);
    if (rc)
    {
#ifndef _CFE_
        bdmf_fastlock_unlock(&counter_read_lock);
#endif        
        return rc;
    }

    /* check burst size */
    if (counter_cfg.cn0_byts > 0)
        burst = num_of_counters * (counter_cfg.cn_double + 1) * (counter_cfg.cn0_byts * 2);
    else
        burst = num_of_counters * (counter_cfg.cn_double + 1);
    if (burst > CNPL_READ_COUNTER_BUFFER)
        rc = BDMF_ERR_PARM;

    /* set read request */
    rc = rc ? rc : drv_cnpl_counter_read_command_set(group, start_counter, num_of_counters);

    /* wait for read done */
    time_out = 0;
    while (!rc && time_out < CNPL_READ_TIMEOUT)
    {
        read_status.cn_rd_st = CNPL_READ_PROCESS;
        rc = ag_drv_cnpl_sw_stat_get(&read_status);
        if (rc || read_status.cn_rd_st == CNPL_READ_DONE)
            break;

        time_out++;
    }
    if (time_out >= CNPL_READ_TIMEOUT)
        rc = BDMF_ERR_INTERNAL;
#else
    rc = rdp_cpu_counter_read(group, start_counter, counters, num_of_counters, &counter_cfg.cn_double, &counter_cfg.cn0_byts);
#endif

    /* read the HW registers data */
    rc = rc ? rc : drv_cnpl_counter_read_command_get(counters, num_of_counters, counter_cfg.cn_double, counter_cfg.cn0_byts, (start_counter % 2));

#ifndef _CFE_
    bdmf_fastlock_unlock(&counter_read_lock);
#endif
    return rc;
}

bdmf_error_t drv_cnpl_counter_read_command_set(uint8_t group, uint16_t start_counter, uint8_t num_of_counters)
{
    uint32_t cmd;

    if (group >= CNPL_MAX_COUNTER_GROUPS ||
        start_counter >= CNPL_MAX_COUNTER_INDEX)
    {
        return BDMF_ERR_PARM;
    }

    cmd = (CNPL_COUNTER_READ_COMMAND << CNPL_COMMAND_OFFSET) |
          (group << CNPL_COUNTER_READ_COMMAND_GROUP_OFFSET) |
          (start_counter << CNPL_COUNTER_READ_COMMAND_START_OFFSET) |
          (num_of_counters << CNPL_COUNTER_READ_COMMAND_SIZE_OFFSET);

    return ag_drv_cnpl_sw_if_sw_cmd_set(cmd);
}

bdmf_error_t drv_cnpl_counter_read_command_get(uint32_t *counters, uint8_t num_of_counters, bdmf_boolean cn_double, uint8_t cn0_byts, bdmf_boolean is_start_counter_odd)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t i, j = 0;
    uint32_t counter_buff[CNPL_READ_COUNTER_BUFFER] = {};
    uint32_t buff_size_word = 0, buff_size_bytes;

    if (cn0_byts)
       cn0_byts = cn0_byts << 1;
    else
       cn0_byts = 1;
    
    num_of_counters *= (cn_double + 1);
    buff_size_bytes = num_of_counters * cn0_byts;

    buff_size_word = buff_size_bytes / 4;
    if (buff_size_bytes % 4)
        buff_size_word += 1;


#if !defined(RDP_SIM) && !defined(XRDP_EMULATION)    
    for (i = 0; !rc && i < buff_size_word && i < CNPL_READ_COUNTER_BUFFER; i++)
        rc = ag_drv_cnpl_sw_if_sw_cnt_rd_get(i, &counter_buff[i]);

    if (rc)
        return rc;
#else
    for (i = 0; i < buff_size_word && i < CNPL_READ_COUNTER_BUFFER; i++)
    {
        counter_buff[i] = __swap4bytes(counters[i]);
    }
#endif



    switch (cn0_byts)
    {
    case 1:
        for (i=0; i < buff_size_word; i++)
        {
            counters[j++] = (counter_buff[i] >> 24) & 0xFF;
            counters[j++] = (counter_buff[i] >> 16) & 0xFF;
            counters[j++] = (counter_buff[i] >> 8) & 0xFF;
            counters[j++] = counter_buff[i] & 0xFF;
        }
        break;
    case 2:
        for (i=0; i < buff_size_word; i++)
        {
            if (is_start_counter_odd && num_of_counters == 1)/* adjust for odd counter due to memory alignment */
            {                                                /* no support for burst mode (num_of_counters > 1)*/
                 counters[j++] = counter_buff[i] & 0xFFFF;
                 counters[j++] = 0;
            }
            else
            {
                counters[j++] = counter_buff[i] >> 16;
                counters[j++] = counter_buff[i] & 0xFFFF;
            }
        }
        break;
    case 4:
        for (i=0; i < buff_size_word; i++)
            counters[i] = counter_buff[i];  
        break;
    default:
        return BDMF_ERR_INTERNAL;
    }
    return rc;
}

bdmf_error_t drv_cnpl_policer_police_command_set(uint8_t group, uint8_t policer_num, uint16_t packet_len)
{
    uint32_t cmd;

    if (group >= CNPL_MAX_POLICER_GROUPS)
    {
        return BDMF_ERR_PARM;
    }

    cmd = (CNPL_POLICER_POLICE_COMMAND << CNPL_COMMAND_OFFSET) |
          (group << CNPL_POLICER_COMMAND_GROUP_OFFSET) |
          (policer_num << CNPL_POLICER_COMMAND_START_OFFSET) |
          (packet_len << CNPL_POLICER_COMMAND_PACKET_LEN_OFFSET);

    return ag_drv_cnpl_sw_if_sw_cmd_set(cmd);
}

bdmf_error_t drv_cnpl_policer_read_command_set(uint8_t group, uint8_t policer_num, bdmf_boolean reset_after_read)
{
    uint32_t cmd;

    if (group >= CNPL_MAX_POLICER_GROUPS)
    {
        return BDMF_ERR_PARM;
    }

    cmd = (CNPL_POLICER_READ_COMMAND << CNPL_COMMAND_OFFSET) |
          (group << CNPL_POLICER_COMMAND_GROUP_OFFSET) |
          (policer_num << CNPL_POLICER_COMMAND_START_OFFSET) |
          (reset_after_read << CNPL_POLICER_COMMAND_CLEAR_OFFSET);

    return ag_drv_cnpl_sw_if_sw_cmd_set(cmd);
}

uint8_t drv_cnpl_periodic_update_us_to_n_get(uint32_t microseconds)
{
    int _ns = microseconds * 1000;

    if (_ns > CNPL_PERIODIC_UPDATE_QUANTA_NS)
        return (_ns + CNPL_PERIODIC_UPDATE_HALF_QUANTA_NS) / CNPL_PERIODIC_UPDATE_QUANTA_NS;
    return CNPL_PERIODIC_UPDATE_MINIMUM;
}

bdmf_error_t drv_cnpl_memory_data_init()
{
#ifndef XRDP_EMULATION
    MEMSET((uint32_t *)DEVICE_ADDRESS(RU_BLK(CNPL).addr[0] + RU_REG_OFFSET(CNPL, MEMORY_DATA)), 0, (RU_REG_RAM_CNT(CNPL, MEMORY_DATA) + 1) * sizeof(uint32_t));
#endif
    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

int drv_cnpl_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t cntr_cfg[] = {cli_cnpl_counter_cfg};
    static uint32_t policer_cfg[] = {cli_cnpl_policer_cfg};
    static uint32_t policer_calc_type[] = {cli_cnpl_policers_configurations_pl_calc_type};
    static uint32_t misc_cfg[] = {cli_cnpl_policers_configurations_per_up, cli_cnpl_misc_arb_prm};

    /* get counters configurations */
    bdmf_session_print(session, "\nCounters configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, cnpl, cntr_cfg, CNPL_MAX_COUNTER_GROUPS);

    /* get policer configurations */
    bdmf_session_print(session, "\nPolicers configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, cnpl, policer_cfg, CNPL_MAX_POLICER_GROUPS);
    HAL_CLI_PRINT_NUM_OF_LIST(session, cnpl, policer_calc_type, (CNPL_MAX_POLICER_INDEX / 32));

    /* get misc condigurations */
    bdmf_session_print(session, "\nMisc configurations:\n");
    HAL_CLI_PRINT_LIST(session, cnpl, misc_cfg);

    return 0;
}

static int drv_cnpl_cli_group_counter_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    uint32_t cntr_arr[4];
    bdmf_error_t rc;
    cnpl_counter_cfg counter_cfg = {};

    uint8_t cnpl_group_number = parm[parameter_index++].value.unumber; /* input */
    uint8_t cnpl_counter_number = parm[parameter_index++].value.unumber; /* input */
    
    rc = ag_drv_cnpl_counter_cfg_get(cnpl_group_number, &counter_cfg);
    if (rc)
        return rc;

    rc = drv_cnpl_counter_read(cntr_arr, cnpl_group_number, cnpl_counter_number, 1);
    if (!rc)
    {
        if (counter_cfg.cn0_byts == 1)
            bdmf_session_print(session, "counter 1: %d\n\r", cntr_arr[cnpl_counter_number%2]);
        else
            bdmf_session_print(session, "counter 1: %d\n\r", cntr_arr[0]);
        if (counter_cfg.cn_double)
            bdmf_session_print(session, "counter 2: %d\n\r", cntr_arr[1]);
    }
    else
        bdmf_session_print(session, "Can't read counter. err : %d\n\r", rc);

    return 0;
}

static bdmfmon_handle_t cnpl_dir;

void drv_cnpl_cli_init(bdmfmon_handle_t driver_dir)
{
    cnpl_dir = ag_drv_cnpl_cli_init(driver_dir);

    BDMFMON_MAKE_CMD_NOPARM(cnpl_dir, "cfg_get", "cnpl configuration", (bdmfmon_cmd_cb_t)drv_cnpl_cli_config_get);

    BDMFMON_MAKE_CMD(cnpl_dir, "rgc", "read group counter", drv_cnpl_cli_group_counter_get,
        BDMFMON_MAKE_PARM("group", "group number", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("counter", "counter offset", BDMFMON_PARM_NUMBER, 0));
}

void drv_cnpl_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (cnpl_dir)
    {
        bdmfmon_token_destroy(cnpl_dir);
        cnpl_dir = NULL;
    }
}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
/*

*/
#endif /* USE_BDMF_SHELL */

