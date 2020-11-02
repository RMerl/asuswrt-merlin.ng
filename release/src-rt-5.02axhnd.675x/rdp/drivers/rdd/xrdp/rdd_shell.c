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
#include "rdd_cpu_rx.h"
#ifdef EPON
#include "rdd_ghost_reporting.h"
#endif
#include "rdd_cpu_rx.h"
#include "rdd_scheduling.h"
#if !defined(BCM63158)
#include "rdd_iptv.h"
#endif
#ifdef CONFIG_DHD_RUNNER
#include "rdd_dhd_helper.h"
#endif
#include "rdd_debug.h"

static int rdd_print_sram_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_tables_list_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_tables_list_img(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_table_entries_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#ifdef EPON
static int rdd_print_report_counter(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
static int rdd_trace_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if defined(DEBUG_PRINTS)
static int rdd_set_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
static int rdd_set_feed_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_set_recycle_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_interrupt_coalescing_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if defined(CONFIG_RUNNER_CSO)
static int rdd_cso_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#if !defined(BCM63158)
static int rdd_print_iptv_ddr_ctx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_miss_cache_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_ae_pause_frame_ignore(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
static int _rdd_ingress_qos_drop_miss_ratio_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if defined BCM6858 || defined BCM6856
static int rdd_multicast_limit_cfg(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#ifdef G9991
static int rdd_g9991_flow_control_print_debug(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif

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

#ifdef EPON
    MAKE_BDMF_SHELL_CMD(rdd_directory, "prc",   "print US report counter", rdd_print_report_counter,
        BDMFMON_MAKE_PARM_RANGE("tcont", "TCONT\\LLID index", BDMFMON_PARM_NUMBER, 0, 0, (RDD_REPORTING_COUNTER_TABLE_SIZE - 1)));
#endif
    MAKE_BDMF_SHELL_CMD(rdd_directory, "ptl",   "print SRAM tables list", rdd_print_sram_tables_list,
        BDMFMON_MAKE_PARM_RANGE("core", "Core ID", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM("name", "Table name, may have wildcards", BDMFMON_PARM_STRING, BDMFMON_PARM_FLAG_OPTIONAL));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "ptal",   "print SRAM tables list on all cores", rdd_print_sram_tables_list_all,
        BDMFMON_MAKE_PARM("name", "Table name, may have wildcards", BDMFMON_PARM_STRING, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "ptil",   "print SRAM tables list on all cores running specific image",
        rdd_print_sram_tables_list_img,
        BDMFMON_MAKE_PARM_RANGE("core", "Core ID", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM("name", "Table name, may have wildcards", BDMFMON_PARM_STRING, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pte",   "print table entries", rdd_print_sram_table_entries,
        BDMFMON_MAKE_PARM_RANGE("core", "Core ID", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM("name", "Full Table name", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("index/address", "0 for entry index to start, 1 for entry address to start", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry index", "Start entry index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry address", "Start entry address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("number", "Number of entries", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "ptae",   "print table entries on all cores", rdd_print_sram_table_entries_all,
        BDMFMON_MAKE_PARM("name", "Full Table name", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("start entry index", "Start entry index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("number", "Number of entries", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "rtrc",   "Enable/Disable RDD Trace", rdd_trace_enable,
        BDMFMON_MAKE_PARM("enable", "0 - disable, 1 - enable", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "sft",   "Set feed ring threshold", rdd_set_feed_ring_threshold,
        BDMFMON_MAKE_PARM_RANGE("thr", "maximum threshold", BDMFMON_PARM_NUMBER, 0, 0, RING_INTERRUPT_THRESHOLD_MAX));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "srt",   "Set recycle ring threshold", rdd_set_recycle_ring_threshold,
        BDMFMON_MAKE_PARM_RANGE("thr", "maximum threshold", BDMFMON_PARM_NUMBER, 0, 0, RING_INTERRUPT_THRESHOLD_MAX));
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "intc", "interrupt coalescing statistics", rdd_interrupt_coalescing_stat);
#if defined(CONFIG_RUNNER_CSO)
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "cso", "CSO statistics", rdd_cso_stat);
#endif
#if defined(DEBUG_PRINTS)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "rprint",   "Set runner prints", rdd_set_runner_prints,
        BDMFMON_MAKE_PARM("max_prints_per_period", "Max number of prints per period (-1 - all, 0 - disable)", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("period", "periodicity of print dump", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("priority", "min priority (0 - LOW, 1 - NORMAL, 2- HIGH, 3 - ERROR)", BDMFMON_PARM_NUMBER, 0));
#endif
#if !defined(BCM63158)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "aepfi",   "AE pause frame ignore", rdd_ae_pause_frame_ignore,
        BDMFMON_MAKE_PARM("ignore", "0 - pause us, 1 - ignore", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pidc",   "print IPTV DDR Context", rdd_print_iptv_ddr_ctx,
        BDMFMON_MAKE_PARM("channel_idx", "IPTV Channel index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "mice",   "miss cache enable", rdd_miss_cache_enable,
        BDMFMON_MAKE_PARM("table index", "NATC table index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("enable", "0 - disable, 1 - enable", BDMFMON_PARM_NUMBER, 0));
#endif
    MAKE_BDMF_SHELL_CMD(rdd_directory, "iqdmr",   "Ingress qos drop miss ratio", _rdd_ingress_qos_drop_miss_ratio_set,
        BDMFMON_MAKE_PARM("drop miss ratio", "Drop miss ratio", BDMFMON_PARM_NUMBER, 0));

#if defined BCM6858 || defined BCM6856
    MAKE_BDMF_SHELL_CMD(rdd_directory, "mlc",   "multicast limit config", rdd_multicast_limit_cfg,
        BDMFMON_MAKE_PARM("max", "max number of multicast tasks", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("min", "min number of multicast tasks", BDMFMON_PARM_NUMBER, 0));
#endif
#ifdef G9991
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "fcs", "flow_control status", rdd_g9991_flow_control_print_debug);
#endif

#ifdef CONFIG_DHD_RUNNER
    rdd_dhd_helper_shell_cmds_init(rdd_directory);
#endif

    return BDMF_ERR_OK;
}

extern TABLE_STRUCT RUNNER_TABLES[];

static uint8_t *seg_addr_get(int seg_num)
{
    switch (seg_num)
    {
    case DDR_INDEX:
    case PSRAM_INDEX:
        /* TODO */
        return (uint8_t *)0xff;
    default:
        break;
    }
    return (uint8_t *)rdp_runner_core_addr[seg_num - CORE_0_INDEX];
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

static int32_t _rdd_print_sram_tables_list_single_core(bdmf_session_handle session, uint32_t core_id, char *tbl_name)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint32_t i, j, n;

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

        if (tbl_name)
        {
            /* Check if table name complies the pattern. If it doesn't, skip */
            if (!strstr(tbl->table_name, tbl_name))
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

static int32_t rdd_print_sram_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t core_id;
    char parm_val[256];

    core_id = parm[0].value.unumber;
    if (n_parms > 1)
    {
        strcpy(parm_val, parm[1].value.string);
        str_toupper(parm_val);
    }

    return _rdd_print_sram_tables_list_single_core(session, core_id, n_parms > 1 ? parm_val : NULL);
}

static int32_t rdd_print_sram_tables_list_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t core_id;
    char parm_val[256];

    strcpy(parm_val, parm[0].value.string);
    str_toupper(parm_val);

    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        _rdd_print_sram_tables_list_single_core(session, core_id, parm_val);
        bdmf_session_print(session, "**********************\n\n");
    }

    return 0;
}

static int32_t rdd_print_sram_tables_list_img(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t core_id, img_id;
    char parm_val[256];

    img_id = parm[0].value.unumber;
    strcpy(parm_val, parm[1].value.string);
    str_toupper(parm_val);

    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        if (rdp_core_to_image_map[core_id] != img_id)
            continue;
        _rdd_print_sram_tables_list_single_core(session, core_id, parm_val);
        bdmf_session_print(session, "**********************\n\n");
    }

    return 0;
}

static int32_t _rdd_print_sram_table_entries_single(bdmf_session_handle session, uint32_t core_id, char *tbl_name,
    uint32_t start_entry, uint32_t num_of_entries, uint32_t is_addr, uint8_t *entry_addr)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint32_t i, j;
    uint8_t *tbl_addr;

    core_id += CORE_0_INDEX;
    for (i = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];

        /* Check if table belongs to core */
        if (core_id != tbl->segment)
            continue;

        if (!strcmp(tbl->table_name, tbl_name))
            break;
    }

    if (i == NUMBER_OF_TABLES) /* Table not found */
    {
        bdmf_session_print(session, "Table %s not found\n", tbl_name);
        return BDMF_ERR_PARM;
    }

    tbl_addr = table_addr_get(tbl, 0);
    tbl_ctx = tbl->entries;

    if (is_addr)
        start_entry = (entry_addr - tbl_addr) / tbl_ctx->length;
    else
        entry_addr = tbl_addr + start_entry * tbl_ctx->length;

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

static int32_t rdd_print_sram_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t core_id;
    char parm_val[256];
    uint32_t start_entry = 0, num_of_entries, is_addr;
    uint8_t *entry_addr = NULL;

    core_id = parm[0].value.unumber;
    strcpy(parm_val, parm[1].value.string);
    str_toupper(parm_val);
    is_addr = parm[2].value.unumber;

    if (is_addr)
    {
#if defined(__LP64__) || defined(_LP64)
        entry_addr = (uint8_t *)(uintptr_t)parm[4].value.unumber64;
#else
        entry_addr = (uint8_t *)parm[4].value.unumber;
#endif
    }
    else
    {
        start_entry = parm[3].value.unumber;
    }
    num_of_entries = parm[5].value.unumber;

    return _rdd_print_sram_table_entries_single(session, core_id, parm_val, start_entry, num_of_entries, is_addr,
        entry_addr);
}

static int32_t rdd_print_sram_table_entries_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t core_id;
    char parm_val[256];
    uint32_t start_entry, num_of_entries;

    strcpy(parm_val, parm[0].value.string);
    str_toupper(parm_val);
    start_entry = parm[1].value.unumber;
    num_of_entries = parm[2].value.unumber;

    for (core_id = 0; core_id < NUM_OF_RUNNER_CORES; core_id++)
    {
        bdmf_session_print(session, "Core %d\n-----------\n", core_id);
        _rdd_print_sram_table_entries_single(session, core_id, parm_val, start_entry, num_of_entries, 0, NULL);
    }
    return 0;
}

#ifdef EPON
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
#endif

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

#if defined(DEBUG_PRINTS)
static int32_t rdd_set_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    int32_t max_prints_per_period;
    uint32_t period,priority;

    max_prints_per_period = parm[0].value.number;
    period = parm[1].value.unumber;
    priority = parm[2].value.unumber;

    if (period == 0 || period > 10000)
    {
        bdmf_session_print(session, "Bad argument %d (expected 1-10000)\n", period);
        return BDMF_ERR_PARM;
    }

    if (priority > 3)
    {
        bdmf_session_print(session, "Bad argument %d (expected 0-3)\n", period);
        return BDMF_ERR_PARM;
    }

    rdd_debug_prints_update_params(max_prints_per_period, period, priority);

    return 0;
}
#endif

static int32_t rdd_set_feed_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_BYTES_2_BITS_WRITE_G(parm[0].value.unumber, RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR, 0);

    return 0;
}

static int rdd_set_recycle_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_WRITE_G(parm[0].value.unumber, RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR, 0);

    return 0;
}

static int rdd_interrupt_coalescing_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS *entry;
    RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS *ring_entry;
    RDD_CPU_RING_DESCRIPTOR_DTS *ring_descriptor_enrty;
    uint32_t ring, timer_period, counter, max_size;
    uint16_t interrupt_id;

    entry = (RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS *)RDD_CPU_INTERRUPT_COALESCING_TABLE_PTR(get_runner_idx(cpu_rx_runner_image));
    RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_READ(timer_period, entry);

    bdmf_session_print(session, "CPU RX Interrupt Coalescing Information\n");
    bdmf_session_print(session, "------------------------------------------------------\n");

    for (ring = 0; ring < RING_ID_LAST; ++ring)
    {
        ring_entry = ((RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS *)RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_PTR(get_runner_idx(cpu_rx_runner_image))) + ring;
        ring_descriptor_enrty = ((RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_RING_DESCRIPTORS_TABLE_PTR(get_runner_idx(cpu_rx_runner_image))) + ring;
        RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_COUNTER_READ(counter, ring_entry);
        RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_READ(max_size, ring_entry);
        RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_READ(interrupt_id, ring_descriptor_enrty);

        if (!interrupt_id) continue;

        bdmf_session_print(session, "Interrupt coalescing values for CPU RX Ring Queue = %d:\n", ring);
        bdmf_session_print(session, "\tconfigured timeout (us)          = %d\n", timer_period);
        bdmf_session_print(session, "\tconfigured maximum packet count  = %d\n", max_size);
        bdmf_session_print(session, "\tcurrent packet count             = %d\n\n", counter);
    }

    return 0;
}

#if defined(CONFIG_RUNNER_CSO)
static int rdd_cso_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t good_csum_packets, no_cso_support_packets, bad_ipv4_hdr_csum_packets, bad_tcp_udp_csum_packets;

    RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_READ_G(good_csum_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
    RDD_CSO_CONTEXT_ENTRY_NO_CSO_SUPPORT_PACKETS_READ_G(no_cso_support_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
    RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_READ_G(bad_ipv4_hdr_csum_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
    RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_READ_G(bad_tcp_udp_csum_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);

    bdmf_session_print(session, "\n\nCSO statistics\n");
    bdmf_session_print(session, "------------------------------------------------------\n");

    bdmf_session_print(session, "\tGood checksum packets            = %d\n", good_csum_packets);
    bdmf_session_print(session, "\tNo CSO support packets           = %d\n", no_cso_support_packets);
    bdmf_session_print(session, "\tBad IPV4 hdr checksum packets    = %d\n", bad_ipv4_hdr_csum_packets);
    bdmf_session_print(session, "\tBad TCP/UDP checksum packets     = %d\n", bad_tcp_udp_csum_packets);

    return 0;
}
#endif

#if !defined(BCM63158)
static int rdd_print_iptv_ddr_ctx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t ch_idx = parm[0].value.unumber;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ddr_ctx_entry = {};
    int i;

    rdd_iptv_ddr_context_entry_get(&ddr_ctx_entry, IPTV_CTX_INDEX_GET(ch_idx));

    bdmf_session_print(session, "Valid: %d\n", ddr_ctx_entry.valid);
    bdmf_session_print(session, "VID: %d (%x), Any_VID: %d\n", ddr_ctx_entry.vid, ddr_ctx_entry.vid,
        ddr_ctx_entry.any_vid);
    bdmf_session_print(session,  "IP Version: %s\n", ddr_ctx_entry.ip_ver ? "IPv6" : "IPv4");
#ifdef __KERNEL__
    {
        int v4_addr;
        struct in6_addr v6_addr;

        v4_addr = ddr_ctx_entry.src_ip;
        bdmf_session_print(session, "\tSource IPv4 address: %pI4\n", &v4_addr);
        memcpy(&v6_addr, &ddr_ctx_entry.dst_ipv6_addr, 16);
        bdmf_session_print(session, "\tGroup IPv6 address:  %pI6\n", &v6_addr);
        memcpy(&v6_addr, &ddr_ctx_entry.src_ipv6_addr, 16);
        bdmf_session_print(session, "\tSource IPv6 address: %pI6\n", &v6_addr);
    }
#else
    {
        uint32_t ip = ddr_ctx_entry.src_ip;
        char buf[64];

        inet_ntop(AF_INET, &ip, buf, sizeof(buf));
        bdmf_session_print(session, "\tSource IPv4 address: %s\n", buf);
        inet_ntop(AF_INET6, &ddr_ctx_entry.dst_ipv6_addr, buf, sizeof(buf));
        bdmf_session_print(session, "\tGroup IPv6 address:  %s\n", buf);
        inet_ntop(AF_INET6, &ddr_ctx_entry.src_ipv6_addr, buf, sizeof(buf));
        bdmf_session_print(session, "\tSource IPv6 address: %s\n", buf);
    }
#endif
    bdmf_session_print(session, "Counter ID: %d\n", ddr_ctx_entry.cntr_id);
    bdmf_session_print(session, "Pool number: %d, Replications: %d\n", ddr_ctx_entry.pool_num,
        ddr_ctx_entry.replications);
    bdmf_session_print(session, "Egress Ports Vector: 0x%x\n", ddr_ctx_entry.egress_ports_vector);
    bdmf_session_print(session, "WLAN Mcast Index: 0x%d\n", ddr_ctx_entry.wlan_mcast_index);
    bdmf_session_print(session, "\tSSID Vector 0: 0x%x\n\tSSID Vector 1: 0x%x\n\tSSID Vector 2: 0x%x\n",
        ddr_ctx_entry.ssid_vector_0_or_flooding_vport, ddr_ctx_entry.ssid_vector_1, ddr_ctx_entry.ssid_vector_2);
    bdmf_session_print(session, "WLAN Mcast TX Priority: 0x%d\n", ddr_ctx_entry.wlan_mcast_tx_prio);
    bdmf_session_print(session, "WLAN Mcast Proxy Enabled (DHD offload only): %s\n",
        ddr_ctx_entry.wlan_mcast_proxy_enabled ? "Yes" : "No");
    bdmf_session_print(session, "Rule Based Results list: ");
    for (i = 0; i < RDD_IPTV_RULE_BASED_RESULT_RULE_NUMBER; i++)
        bdmf_session_print(session, "%d ", ddr_ctx_entry.result[i]);
    bdmf_session_print(session, "\nNext Entry index: %d\n", ddr_ctx_entry.next_entry_idx);

    return 0;
}

static int rdd_miss_cache_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t tbl_idx = parm[0].value.unumber;
    uint32_t enable = parm[1].value.unumber;

    RDD_NATC_TBL_CONFIGURATION_MISS_CACHE_ENABLE_WRITE_G(enable, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);

    return 0;
}

static int rdd_ae_pause_frame_ignore(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t pause_ignore = parm[0].value.unumber;

    RDD_PAUSE_QUANTA_ENTRY_IGNORE_WRITE_G(pause_ignore, RDD_DIRECT_PROCESSING_PAUSE_QUANTA_ADDRESS_ARR, 0);

    return 0;
}
#endif /* !defined(BCM63158) */

static int _rdd_ingress_qos_drop_miss_ratio_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t drop_miss_ratio = parm[0].value.unumber;

    rdd_ingress_qos_drop_miss_ratio_set(drop_miss_ratio);

    return 0;
}

#if defined BCM6858 || defined BCM6856
static int rdd_multicast_limit_cfg(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t mcast_max = parm[0].value.unumber;
    uint8_t mcast_min = parm[1].value.unumber;

    rdd_mcast_max_tasks_limit_cfg(mcast_max);
    rdd_mcast_min_tasks_limit_cfg(mcast_min);

    return 0;
}
#endif

#ifdef G9991
static int rdd_g9991_flow_control_print_debug(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t flow_control_vector_0_1, flow_control_vector_2_3;
    uint32_t sid_to_phy_port_mask[RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_SIZE];
    uint32_t sid_flow_control_status;
    uint32_t i, j;

    bdmf_session_print(session, "\n\nFlow control Vector\n");
    bdmf_session_print(session, "-------------------\n");

    RDD_BYTES_4_BITS_READ(flow_control_vector_0_1, (RDD_BYTES_4_DTS *)RDD_G9991_FLOW_CONTROL_VECTOR_PTR(DS_TM_CORE_BBH_0_1));
    RDD_BYTES_4_BITS_READ(flow_control_vector_2_3, (RDD_BYTES_4_DTS *)RDD_G9991_FLOW_CONTROL_VECTOR_PTR(DS_TM_CORE_BBH_2_3));

    RDD_BYTES_4_BITS_READ(sid_to_phy_port_mask[0], (RDD_BYTES_4_DTS *)RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_PTR(DS_TM_CORE_BBH_0_1) + 0);
    RDD_BYTES_4_BITS_READ(sid_to_phy_port_mask[1], (RDD_BYTES_4_DTS *)RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_PTR(DS_TM_CORE_BBH_0_1) + 4);
    RDD_BYTES_4_BITS_READ(sid_to_phy_port_mask[2], (RDD_BYTES_4_DTS *)RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_PTR(DS_TM_CORE_BBH_2_3) + 8);
    RDD_BYTES_4_BITS_READ(sid_to_phy_port_mask[3], (RDD_BYTES_4_DTS *)RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_PTR(DS_TM_CORE_BBH_2_3) + 12);

    bdmf_session_print(session, "sid   status\n");
    bdmf_session_print(session, "------------\n");

    for (i = 0; i < 32; i++) {
        bdmf_session_print(session, " %02i: ", i);

        /* find to which hsgmii the currenet sid is belong */
        for (j = 0; j < RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_SIZE; j++)
            if (sid_to_phy_port_mask[j] & 1 << i)
               break;

        if (j == RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_SIZE) {
            bdmf_session_print(session, "not configured\n");
            continue;
        }

        if (j == 0 || j == 1)
            sid_flow_control_status = flow_control_vector_0_1 & (1 << i);
        else
            sid_flow_control_status = flow_control_vector_2_3 & (1 << i);

        if (sid_flow_control_status)
            bdmf_session_print(session, "enabled\n");
        else
            bdmf_session_print(session, "blocked\n");
    }

    return 0;
}
#endif
