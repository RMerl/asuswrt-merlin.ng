/*
    <:copyright-BRCM:2015-2016:DUAL/GPL:standard
    
       Copyright (c) 2015-2016 Broadcom 
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

#include "bdmf_shell.h"
#include "rdd.h"
#include "rdd_runner_reg_dump.h"
#include "rdd_runner_reg_dump_addrs.h"
#include "rdd_map_auto.h"
#include "rdd_platform.h"
#include "rdd_proj_shell.h"
#include "rdd_cpu_rx.h"
#include "rdd_ghost_reporting.h"

static int rdd_print_sram_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_report_counter(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_trace_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_set_feed_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_set_recycle_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);

extern uintptr_t rdp_runner_core_addr[GROUPED_EN_SEGMENTS_NUM];
uint32_t g_rdd_trace = 0;

#define MAKE_BDMF_SHELL_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

#define MAKE_BDMF_SHELL_CMD(dir, cmd, help, cb, parms...)   \
{                                                           \
    static bdmfmon_cmd_parm_t cmd_parms[] = {               \
        parms,                                              \
        BDMFMON_PARM_LIST_TERMINATOR                        \
    };                                                      \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}

int rdd_make_shell_commands(void)
{
    bdmfmon_handle_t driver_directory, rdd_directory;

    driver_directory = bdmfmon_dir_find(NULL, "driver");

    if (!driver_directory)
    {
        driver_directory = bdmfmon_dir_add(NULL, "driver", "Device Drivers", BDMF_ACCESS_ADMIN, NULL);

        if (!driver_directory)
            return BDMF_ERR_NOMEM;
    }

    rdd_directory = bdmfmon_dir_add(driver_directory, "rdd", "Runner Device Driver", BDMF_ACCESS_ADMIN, NULL);

    if (!rdd_directory)
        return BDMF_ERR_NOMEM;

    MAKE_BDMF_SHELL_CMD(rdd_directory, "prc",   "print US report counter", rdd_print_report_counter,
        BDMFMON_MAKE_PARM_RANGE("tcont", "TCONT\\LLID index", BDMFMON_PARM_NUMBER, 0, 0, (RDD_REPORTING_COUNTER_TABLE_SIZE - 1)));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "ptl",   "print SRAM tables list", rdd_print_sram_tables_list,
        BDMFMON_MAKE_PARM_RANGE("core", "Core ID", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM("name", "Table name, may have wildcards", BDMFMON_PARM_STRING, BDMFMON_PARM_FLAG_OPTIONAL));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pte",   "print table entries", rdd_print_sram_table_entries,
        BDMFMON_MAKE_PARM_RANGE("core", "Core ID", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM("name", "Full Table name", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("index/address", "0 for entry index to start, 1 for entry address to start", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry index", "Start entry index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry address", "Start entry address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("number", "Number of entries", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "rtrc",   "Enable/Disable RDD Trace", rdd_trace_enable,
        BDMFMON_MAKE_PARM("enable", "0 - disable, 1 - enable", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "sft",   "Set feed ring threshold", rdd_set_feed_ring_threshold,
        BDMFMON_MAKE_PARM_RANGE("thr", "maximum threshold", BDMFMON_PARM_NUMBER, 0, 0, RING_INTERRUPT_THRESHOLD_MAX));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "srt",   "Set recycle ring threshold", rdd_set_recycle_ring_threshold,
        BDMFMON_MAKE_PARM_RANGE("thr", "maximum threshold", BDMFMON_PARM_NUMBER, 0, 0, RING_INTERRUPT_THRESHOLD_MAX));

    return rdd_proj_shell_init(rdd_directory);
}

extern TABLE_STRUCT RUNNER_TABLES[];

static bdmf_phys_addr_t seg_addr_get(int seg_num)
{
    switch (seg_num)
    {
    case DDR_INDEX:
    case PSRAM_INDEX:
        /* TODO */
        return (bdmf_phys_addr_t)0xfffff;
    default:
        break;
    }
    return (bdmf_phys_addr_t)rdp_runner_core_addr[seg_num - CORE_0_INDEX];
}

static uint8_t *table_addr_get(TABLE_STRUCT *tbl, int entry_idx)
{
    DUMP_RUNNERREG_STRUCT *tbl_ctx;

    tbl_ctx = tbl->entries;
    return (uint8_t *)(seg_addr_get(tbl->segment) + tbl_ctx->entries[entry_idx].starts);
}

static char *str_toupper(char *str)
{
    uint32_t i, len;

    len = strlen(str);

    for (i = 0; i < len; i++)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = str[i] - 'a' + 'A';
    }

    return str;
}

static int32_t rdd_print_sram_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint32_t i, j, n, core_id;
    char parm_val[256];

    core_id = parm[0].value.unumber;
    if (n_parms > 1)
    {
        strcpy(parm_val, parm[1].value.string);
        str_toupper(parm_val);
    }

    bdmf_session_print(session, "List of Tables for core %d\n\n", core_id);
    bdmf_session_print(session, "%70s %8s %12s %12s %12s\n", "Table Name", "Address", "Entry Len", "Entry Types",
        "Size");
    bdmf_session_print(session, "---------------------------------------------------------------------");
    bdmf_session_print(session, "---------------------------------------------------------------------\n");

    core_id += CORE_0_INDEX;
    for (i = 0, n = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];

        /* Check if table belongs to core */
        if (core_id != tbl->segment)
            continue;

        if (n_parms > 1)
        {
            /* Check if table name complies the pattern. If it doesn't, skip */
            if (!strstr(tbl->table_name, parm_val))
                continue;
        }
        tbl_ctx = tbl->entries;

        /* If this is a union, calc how many entry representations we have */
        for (j = 0; tbl_ctx->entries[j].callback; j++);

        bdmf_session_print(session, "%70s %p %10d %10d          [%d]",
            tbl->table_name, (void *)table_addr_get(tbl, 0), tbl_ctx->length, j, tbl->size_rows);
        if (tbl->size_rows_d2)
            bdmf_session_print(session, "[%d]", tbl->size_rows_d2);
        if (tbl->size_rows_d3)
            bdmf_session_print(session, "[%d]", tbl->size_rows_d3);
        bdmf_session_print(session, "\n");
        n++;
    }

    bdmf_session_print(session, "\nTotal %d tables\n", n);

    return 0;
}

static int32_t rdd_print_sram_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint32_t i, j, core_id;
    char parm_val[256];
    uint32_t start_entry, num_of_entries, is_addr;
    uint8_t *entry_addr, *tbl_addr;

    core_id = parm[0].value.unumber;
    strcpy(parm_val, parm[1].value.string);
    str_toupper(parm_val);

    core_id += CORE_0_INDEX;
    for (i = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];

        /* Check if table belongs to core */
        if (core_id != tbl->segment)
            continue;

        if (!strcmp(tbl->table_name, parm_val))
            break;
    }

    if (i == NUMBER_OF_TABLES) /* Table not found */
    {
        bdmf_session_print(session, "Table %s not found\n", parm_val);
        return BDMF_ERR_PARM;
    }

    tbl_addr = table_addr_get(tbl, 0);
    tbl_ctx = tbl->entries;

    is_addr = parm[2].value.unumber;

    if (is_addr)
    {
#if defined(__LP64__) || defined(_LP64)
        entry_addr = (uint8_t *)(uintptr_t)parm[4].value.unumber64;
#else
        entry_addr = (uint8_t *)parm[4].value.unumber;
#endif
        start_entry = (entry_addr - tbl_addr) / tbl_ctx->length;
    }
    else
    {
        start_entry = parm[3].value.unumber;
        entry_addr = tbl_addr + start_entry * tbl_ctx->length;
    }

    num_of_entries = parm[5].value.unumber;

    for (i = 0; i < num_of_entries; i++, entry_addr += tbl_ctx->length)
    {
        bdmf_session_print(session, "Index %d, addr %p, size %d, value:\n", start_entry + i, (void *)entry_addr,
            tbl_ctx->length);
        bdmf_session_hexdump(session, (unsigned char *)DEVICE_ADDRESS(entry_addr), 0, tbl_ctx->length);
        bdmf_session_print(session, "\n");

        /* It's possible that we have a union of entries. In this case, we want to print all possible transformations.*/
        for (j = 0; tbl_ctx->entries[j].callback; j++)
        {
            tbl_ctx->entries[j].callback(session, (unsigned char *)DEVICE_ADDRESS(entry_addr));
            bdmf_session_print(session, "\n");
        }
    }
    bdmf_session_print(session, "\n\n");
    return 0;
}

static int rdd_print_report_counter(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t channel = parm[0].value.unumber;
    int rc;
    uint32_t report = 0;

    rc = rdd_ghost_reporting_ctr_get(channel, &report);
    if (rc)
        bdmf_session_print(session, "ERROR: while trying to read wan channel counter\n");
    else
        bdmf_session_print(session, "Report counter for wan channel %d is: %d\n", channel, report);

    bdmf_session_print(session, "\n");
    return rc;
}

static int32_t rdd_trace_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t enable;

    enable = parm[0].value.unumber;
    if (enable != 0 && enable != 1)
    {
        bdmf_session_print(session, "Bad argument %d (expected 0|1)\n", enable);
        return BDMF_ERR_PARM;
    }
    g_rdd_trace = enable;
    return 0;
}

static int32_t rdd_set_feed_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_BYTES_2_BITS_WRITE_G(parm[0].value.unumber, RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR, 0);

    return 0;
}

static int rdd_set_recycle_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_BYTES_2_BITS_WRITE_G(parm[0].value.unumber, RDD_CPU_RECYCLE_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR, 0);

    return 0;
}

