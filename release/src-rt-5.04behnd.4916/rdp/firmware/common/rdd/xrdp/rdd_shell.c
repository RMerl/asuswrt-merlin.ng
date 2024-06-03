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
#include "rdp_common.h"
#ifdef EPON
#include "rdd_ghost_reporting.h"
#endif
#include "rdd_cpu_rx.h"
#include "rdd_scheduling.h"
#if !defined(BCM_DSL_XRDP)
#include "rdd_iptv.h"
#endif
#ifdef CONFIG_DHD_RUNNER
#include "rdd_dhd_helper.h"
#endif
#include "rdd_debug.h"
#if defined(XRDP_CODEL) || defined(XRDP_PI2)
void rdd_aqm_shell_cmds_init(bdmfmon_handle_t rdd_dir);
#endif
#ifdef XRDP_CODEL
void rdd_codel_shell_cmds_init(bdmfmon_handle_t rdd_dir);
#endif
#ifdef XRDP_PI2
void rdd_pi2_shell_cmds_init(bdmfmon_handle_t rdd_dir);
#endif
#ifdef BUFFER_CONGESTION_MGT
void rdd_buffer_cong_mgt_shell_cmds_init(bdmfmon_handle_t rdd_dir);
#endif
#if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER)
#include "rdd_cpu_tx_sg.h"
#endif

#if defined(CONFIG_RUNNER_CSO)
bdmfmon_enum_val_t bdmfmon_enum_cso_table[] = {
    { .name = "off", .val = 0},
    { .name = "on", .val = 1},
    { .name = "read", .val = 2},
    { .name = "clear", .val = 3},
    BDMFMON_ENUM_LAST
};
#endif

static int rdd_print_sram_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_tables_list_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_tables_list_img(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_print_sram_table_entries_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);

#ifdef TM_C_CODE
static int rdd_print_non_empty_fifos(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif

#ifdef EPON
static int rdd_print_report_counter(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
static int rdd_trace_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if defined(DEBUG_PRINTS)
static int rdd_set_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if !defined(RDP_SIM)
static int32_t rdd_start_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int32_t rdd_stop_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#endif
#if defined(CONFIG_RNR_FEED_RING)
static int rdd_set_feed_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
static int rdd_set_recycle_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_interrupt_coalescing_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if defined(CONFIG_RUNNER_CSO)
static int rdd_cso_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER)
static int rdd_sg_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#if defined(CONFIG_CPU_TX_MCORE)
static int rdd_sync_counter_disp(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_cpu_tx_mcore_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#if !defined(BCM_DSL_XRDP)
static int rdd_print_iptv_ddr_ctx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_miss_cache_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_ae_pause_frame_ignore(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int rdd_ae_pause_frame_control(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
static int _rdd_ingress_qos_drop_miss_ratio_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if defined(OPERATION_MODE_PRV) && (defined(BCM6858) || defined(BCM6856))
static int rdd_multicast_limit_cfg(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif
#ifdef G9991_COMMON
static int rdd_g9991_flow_control_print_debug(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif

extern uintptr_t rdp_runner_core_addr[NUM_OF_RUNNER_CORES];
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
#ifdef TM_C_CODE
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pnef",   "print non empty pds and update fifos", rdd_print_non_empty_fifos,
        BDMFMON_MAKE_PARM_ENUM("tm_id", "tm_id", tm_identifier_enum_table, 0),
        BDMFMON_MAKE_PARM("only_valid", "0 - print all, 1 - print only valid", BDMFMON_PARM_NUMBER, 0));
#endif


#if defined(CONFIG_RNR_FEED_RING)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "sft",   "Set feed ring threshold", rdd_set_feed_ring_threshold,
        BDMFMON_MAKE_PARM_RANGE("thr", "maximum threshold", BDMFMON_PARM_NUMBER, 0, 0, RING_INTERRUPT_THRESHOLD_MAX));
#endif
    MAKE_BDMF_SHELL_CMD(rdd_directory, "srt",   "Set recycle ring threshold", rdd_set_recycle_ring_threshold,
        BDMFMON_MAKE_PARM_RANGE("thr", "maximum threshold", BDMFMON_PARM_NUMBER, 0, 0, RING_INTERRUPT_THRESHOLD_MAX));
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "intc", "interrupt coalescing statistics", rdd_interrupt_coalescing_stat);
#if defined(CONFIG_RUNNER_CSO)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "cso", "CSO control and statistics", rdd_cso_stat,
        BDMFMON_MAKE_PARM_ENUM_DEFVAL("switch", "cso switch", bdmfmon_enum_cso_table, 0, "read"));
#endif
#if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER)
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "sg", "SG Frag Gather statistics", rdd_sg_stat);
#endif
#if defined(CONFIG_CPU_TX_MCORE)
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "cntrd", "Runner cpu_tx_sync counter display", rdd_sync_counter_disp);
    MAKE_BDMF_SHELL_CMD(rdd_directory, "mcore",   "Enable/Disable runner cpu_tx_mcore task", rdd_cpu_tx_mcore_enable,
        BDMFMON_MAKE_PARM("enable", "0 - disable, 1 - enable", BDMFMON_PARM_NUMBER, 0));
#endif
#if defined(DEBUG_PRINTS)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "printr",   "Set runner prints", rdd_set_runner_prints,
        BDMFMON_MAKE_PARM("max_prints_per_period", "Max number of prints per period (-1 - all, 0 - disable)", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("period", "periodicity of print dump", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("priority", "min priority (0 - LOW, 1 - NORMAL, 2- HIGH, 3 - ERROR)", BDMFMON_PARM_NUMBER, 0)
        );
#if !defined(RDP_SIM)
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "printr_start",   "Start printr thread", rdd_start_runner_prints);
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "printr_stop",    "Stop printr thread", rdd_stop_runner_prints);
#endif
#endif
#if !defined(BCM_DSL_XRDP)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "aepfi",   "AE pause frame ignore", rdd_ae_pause_frame_ignore,
        BDMFMON_MAKE_PARM("ignore", "0 - pause us, 1 - ignore", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "aepfc",   "AE pause frame control", rdd_ae_pause_frame_control,
        BDMFMON_MAKE_PARM_DEFVAL("control", "frame control quanta value (no params - read stats, 0xffff - max timer)", BDMFMON_PARM_NUMBER, 0, -1));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pidc",   "print IPTV DDR Context", rdd_print_iptv_ddr_ctx,
        BDMFMON_MAKE_PARM("channel_idx", "IPTV Channel index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "mice",   "miss cache enable", rdd_miss_cache_enable,
        BDMFMON_MAKE_PARM("table index", "NATC table index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("enable", "0 - disable, 1 - enable", BDMFMON_PARM_NUMBER, 0));
#endif
    MAKE_BDMF_SHELL_CMD(rdd_directory, "iqdmr",   "Ingress qos drop miss ratio", _rdd_ingress_qos_drop_miss_ratio_set,
        BDMFMON_MAKE_PARM("drop miss ratio", "Drop miss ratio", BDMFMON_PARM_NUMBER, 0));

#if defined(OPERATION_MODE_PRV) && (defined(BCM6858) || defined(BCM6856))
    MAKE_BDMF_SHELL_CMD(rdd_directory, "mlc",   "multicast limit config", rdd_multicast_limit_cfg,
        BDMFMON_MAKE_PARM("max", "max number of multicast tasks", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("min", "min number of multicast tasks", BDMFMON_PARM_NUMBER, 0));
#endif
#ifdef G9991_COMMON
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "fcs", "flow_control status", rdd_g9991_flow_control_print_debug);
#endif

#ifdef CONFIG_DHD_RUNNER
    rdd_dhd_helper_shell_cmds_init(rdd_directory);
#endif

#if defined(XRDP_CODEL) || defined(XRDP_PI2)
    rdd_aqm_shell_cmds_init(rdd_directory);
#endif

#ifdef XRDP_CODEL
    rdd_codel_shell_cmds_init(rdd_directory);
#endif

#ifdef XRDP_PI2
    rdd_pi2_shell_cmds_init(rdd_directory);
#endif

#ifdef BUFFER_CONGESTION_MGT
    rdd_buffer_cong_mgt_shell_cmds_init(rdd_directory);
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
        for (j = 0; tbl_ctx->entries[j].callback; j++)
            ;

        bdmf_session_print(session, "%70s %px %10d %10d          [%d]",
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
        strncpy(parm_val, parm[1].value.string, 255);
        str_toupper(parm_val);
    }

    return _rdd_print_sram_tables_list_single_core(session, core_id, n_parms > 1 ? parm_val : NULL);
}

static int32_t rdd_print_sram_tables_list_all(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t core_id;
    char parm_val[256];

    strncpy(parm_val, parm[0].value.string, 255);
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
    strncpy(parm_val, parm[1].value.string, 255);
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
        bdmf_session_print(session, "Index %d, addr %px, size %d, value:\n", start_entry + i, (void *)entry_addr,
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
    strncpy(parm_val, parm[1].value.string, 255);
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

    strncpy(parm_val, parm[0].value.string, 255);
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

#ifdef TM_C_CODE
static int32_t rdd_print_non_empty_fifos(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    bdmf_boolean valid;
    uint16_t pd_fifo_write_ptr;
    uint16_t queue_number;
    PROCESSING_TX_DESCRIPTOR_STRUCT *pd;
    AGGREGATED_PD_DESCRIPTOR_STRUCT *agg_pd;
    PROCESSING_TX_DESCRIPTOR_STRUCT curr_pd;
    int pd_size;
    int ret;
    int i;
    tm_identifier_e tm_identity = parm[0].value.unumber;
    int only_valid = parm[1].value.unumber;
    int core_id = tm_get_core_for_tm(tm_identity);

    ret = 0;
    i = 0;
    if (core_id < 0)
    {
        bdmf_session_print(session, "ERROR Invalid core =%d\n", core_id);
        return 0;
    }
    else
        bdmf_session_print(session, "tm_id %d core =%d\n-----------\n", tm_identity, core_id);

    while (ret == 0)
    {
        if (channel_is_eth(tm_identity) || channel_is_pon(tm_identity))
            ret = rdd_ag_update_fifo_update_fifo_table_get_core(i, &valid, &pd_fifo_write_ptr, &queue_number, core_id);
        else
#ifdef G9991_COMMON
            return 0;
#else
            ret = rdd_ag_service_queues_update_fifo_table_get_core(i, &valid, &pd_fifo_write_ptr, &queue_number, core_id);
#endif
        
        if ((valid) || (only_valid == 0))
        {
            if (valid)
            {
                bdmf_session_print(session, "\e[1;92m");
            }
            bdmf_session_print(session, "#%d valid=%d pd_fifo_write_ptr=0x%x queue_number=%d\n", i, valid, pd_fifo_write_ptr, queue_number);
            bdmf_session_print(session, "\e[0m");
        }
        i++;
    }

    i = 0;

    if (channel_is_eth(tm_identity))
    {
        pd = (PROCESSING_TX_DESCRIPTOR_STRUCT *)RDD_DS_TM_PD_FIFO_TABLE_PTR(core_id);
        pd_size = RDD_DS_TM_PD_FIFO_TABLE_SIZE;
    }
    else if (channel_is_pon(tm_identity))
    {
        pd = (PROCESSING_TX_DESCRIPTOR_STRUCT *)RDD_US_TM_PD_FIFO_TABLE_PTR(core_id);
        pd_size = RDD_US_TM_PD_FIFO_TABLE_SIZE;
    }
    else 
    {
#ifdef G9991_COMMON
        return 0;
#else
        /* SQ*/
        pd = (PROCESSING_TX_DESCRIPTOR_STRUCT *)RDD_SQ_TM_PD_FIFO_TABLE_PTR(core_id);
        pd_size = RDD_SQ_TM_PD_FIFO_TABLE_SIZE;
#endif
    }

    while (i < pd_size)
    {
        curr_pd.word_32[0] = swap4bytes(pd->word_32[0]);
        curr_pd.word_32[1] = swap4bytes(pd->word_32[1]);
        curr_pd.word_32[2] = swap4bytes(pd->word_32[2]);
        curr_pd.word_32[3] = swap4bytes(pd->word_32[3]);

        if (((curr_pd.valid) || (only_valid == 0)) && (curr_pd.word_32[0] != 0))
        {
            if (curr_pd.valid)
            {
                /* green color for valid PD's*/
                bdmf_session_print(session, "\e[1;92m");
            }
            else
            {
                /* blue color for non empty PD's*/
                bdmf_session_print(session, "\e[1;34m");
            }
            bdmf_session_print(session, "PD #%d 0x%x 0x%x 0x%x 0x%x\n", i, curr_pd.word_32[0], curr_pd.word_32[1], curr_pd.word_32[2], curr_pd.word_32[3]);
            if (curr_pd.agg_pd)
            {
                /* print aggregated pd*/
                agg_pd = (AGGREGATED_PD_DESCRIPTOR_STRUCT *)(&curr_pd);
                bdmf_session_print(session, "AGG_PD: valid=%d plen_0=%d plen_1=%d plen_2=%d total_plen=%d num_packets=%d target_mem_0=%d\n", agg_pd->valid, agg_pd->plen_0, 
                    agg_pd->plen_1, agg_pd->plen_2, agg_pd->total_plen, agg_pd->num_packets, agg_pd->target_mem_0);

                bdmf_session_print(session, "first_level_q=%d second_level_q=%d pd_push_to_empty=%d mini_fpm=%d\n", agg_pd->first_level_q, agg_pd->second_level_q, 
                    agg_pd->pd_push_to_empty, agg_pd->mini_fpm);
            }
            else
            {
                /* print non aggregated pd*/
                bdmf_session_print(session, "NON_AGG: valid=%d first_level_q=%d second_level_q=%d force_copy=%d reprocess=%d dont_agg=%d packet_length=%d eh=%d is_emac=%d\n", 
                    curr_pd.valid, curr_pd.first_level_q, curr_pd.second_level_q_spdsvs, curr_pd.force_copy, 
                    curr_pd.reprocess, curr_pd.dont_agg, curr_pd.packet_length, curr_pd.eh, curr_pd.is_emac);
                
                bdmf_session_print(session, "target_mem_0=%d target_mem_1=%d ingress_port=%d ingress_cong=%d abs=%d\n", curr_pd.target_mem_0, curr_pd.target_mem_1, 
                    curr_pd.ingress_port, curr_pd.ingress_cong, curr_pd.abs);
            }
            bdmf_session_print(session, "\e[0m");
        }
        pd++;
        i++;
    }

    return 0;
}
#endif

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
    uint32_t period, priority;

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

#if !defined(RDP_SIM)
static int32_t rdd_start_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdd_debug_prints_start_thread();
    return 0;
}
static int32_t rdd_stop_runner_prints(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdd_debug_prints_stop_thread();
    return 0;
}
#endif
#endif

#if defined(CONFIG_RNR_FEED_RING)
static int32_t rdd_set_feed_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_BYTES_2_BITS_WRITE_G(parm[0].value.unumber, RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR, 0);

    return 0;
}
#endif

static int rdd_set_recycle_ring_threshold(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_WRITE_G(parm[0].value.unumber, RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR, 0);

    return 0;
}

static int rdd_interrupt_coalescing_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    CPU_RING_INTERRUPT_COUNTER_ENTRY_STRUCT *ring_entry;
    CPU_RING_DESCRIPTOR_STRUCT *ring_descriptor_enrty;
    uint32_t ring, timer_period, counter, max_size;
    uint16_t interrupt_id;

#if !defined(RDP_UFC)
    CPU_INTERRUPT_COALESCING_ENTRY_STRUCT *entry;
    entry = (CPU_INTERRUPT_COALESCING_ENTRY_STRUCT *)RDD_CPU_INTERRUPT_COALESCING_TABLE_PTR(get_runner_idx(cpu_rx_runner_image));
    RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_READ(timer_period, entry);
#else
    GENERAL_TIMER_ENTRY_STRUCT *timer_addr;
    timer_addr = ((GENERAL_TIMER_ENTRY_STRUCT *)(RDD_GENERAL_TIMER_PTR(get_runner_idx(cpu_rx_runner_image))) + TIMER_ACTION_CPU_RX_COALESCING);
    RDD_GENERAL_TIMER_ENTRY_TIMEOUT_READ(timer_period, timer_addr);
    timer_period *= GENERAL_TIMER_PERIODICITY;
#endif

    bdmf_session_print(session, "CPU RX Interrupt Coalescing Information\n");
    bdmf_session_print(session, "------------------------------------------------------\n");

    for (ring = 0; ring < RING_ID_LAST; ++ring)
    {
        ring_entry = ((CPU_RING_INTERRUPT_COUNTER_ENTRY_STRUCT *)RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_PTR(get_runner_idx(cpu_rx_runner_image))) + ring;
        ring_descriptor_enrty = ((CPU_RING_DESCRIPTOR_STRUCT *)RDD_CPU_RING_DESCRIPTORS_TABLE_PTR(get_runner_idx(cpu_rx_runner_image))) + ring;
        RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_COUNTER_READ(counter, ring_entry);
        RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_READ(max_size, ring_entry);
        RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_READ(interrupt_id, ring_descriptor_enrty);

        if (!interrupt_id)
            continue;

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
    uint16_t  val, cso_disable;
#if defined(CONFIG_RUNNER_FWD_FLOW_CSO)
    uint32_t forward_cso_packets = 0, forward_cso_ver_error = 0;
#endif
#if defined(CONFIG_RUNNER_CSUM_HW_OFFLOAD)
    uint32_t invalid_offload_result, offload_req_id_mismatch, wakeup_reenable;
#endif

    val = (uint32_t)parm[0].value.number;

    if (val == 2)
    {
        RDD_BYTE_1_BITS_READ_G(cso_disable, RDD_CSO_DISABLE_ADDRESS_ARR, 0);
        if (cso_disable)
        {
            bdmf_session_print(session, "CSO Disabled\n");
        }
        else
        {
            RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_READ_G(good_csum_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_NO_CSO_SUPPORT_PACKETS_READ_G(no_cso_support_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
#if defined(CONFIG_RUNNER_FWD_FLOW_CSO)
            RDD_CSO_CONTEXT_ENTRY_FORWARD_CSO_PACKETS_READ_G(forward_cso_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_FORWARD_CSO_VER_ERROR_READ_G(forward_cso_ver_error, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
#endif
            RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_READ_G(bad_ipv4_hdr_csum_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_READ_G(bad_tcp_udp_csum_packets, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);

            bdmf_session_print(session, "\n\nCSO statistics\n");
            bdmf_session_print(session, "------------------------------------------------------\n");

            bdmf_session_print(session, "\tGood checksum packets            = %u\n", good_csum_packets);
            bdmf_session_print(session, "\tNo CSO support packets           = %u\n", no_cso_support_packets);
#if defined(CONFIG_RUNNER_FWD_FLOW_CSO)
            bdmf_session_print(session, "\tFWD flow CSO packets             = %u\n", forward_cso_packets);
            bdmf_session_print(session, "\tFWD flow CSO ver error           = %u\n", forward_cso_ver_error);
#endif
            bdmf_session_print(session, "\tBad IPV4 hdr checksum packets    = %u\n", bad_ipv4_hdr_csum_packets);
            bdmf_session_print(session, "\tBad TCP/UDP checksum packets     = %u\n", bad_tcp_udp_csum_packets);

#if defined(CONFIG_RUNNER_CSUM_HW_OFFLOAD)
            RDD_CSO_CONTEXT_ENTRY_INVALID_OFFLOAD_RESULT_READ_G(invalid_offload_result, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_OFFLOAD_REQ_ID_MISMATCH_READ_G(offload_req_id_mismatch, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_WAKEUP_REENABLE_READ_G(wakeup_reenable, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);            
            
            bdmf_session_print(session, "\tInvalid Offload result           = %u\n", invalid_offload_result);
            bdmf_session_print(session, "\tOffload req_id mismatch          = %u\n", offload_req_id_mismatch);
            bdmf_session_print(session, "\tWakeup_reenable                  = %u\n", wakeup_reenable);
#endif
        }
    }
    else if (val == 3)
    {
            RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_WRITE_G(0, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_NO_CSO_SUPPORT_PACKETS_WRITE_G(0, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
#if defined(CONFIG_RUNNER_FWD_FLOW_CSO)
            RDD_CSO_CONTEXT_ENTRY_FORWARD_CSO_PACKETS_WRITE_G(0, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_FORWARD_CSO_VER_ERROR_WRITE_G(0, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
#endif
            RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_WRITE_G(0, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
            RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_WRITE_G(0, RDD_CSO_CONTEXT_TABLE_ADDRESS_ARR, 0);
    }
    else
    {
      cso_disable = 1-val;
      RDD_BYTE_1_BITS_WRITE_G(cso_disable, RDD_CSO_DISABLE_ADDRESS_ARR, 0);
    }

    return 0;
}
#endif

#if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER)
static int rdd_sg_stat(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    rdd_sg_debug_info_disp(session);
    
    return 0;
}
#endif

#if defined(CONFIG_CPU_TX_MCORE)
static int rdd_sync_counter_disp(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    rdd_sync_cntr_display(session);
    
    return 0;
}

static int32_t rdd_cpu_tx_mcore_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t enable;

    enable = parm[0].value.unumber;
    if (enable != 0 && enable != 1)
    {
        bdmf_session_print(session, "Bad argument %d (expected 0|1)\n", enable);
        return BDMF_ERR_PARM;
    }

    return rdd_cpu_tx_mcore_cfg(session, enable);
}
#endif

#if !defined(BCM_DSL_XRDP)
static int rdd_print_iptv_ddr_ctx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t ch_idx = parm[0].value.unumber;
    IPTV_DDR_CONTEXT_ENTRY_STRUCT ddr_ctx_entry = {};
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
    bdmf_session_print(session, "\tSSID Vector 0: 0x%x\n\tSSID Vector 1: 0x%x\n\tSSID Vector 2: 0x%x\n",
        ddr_ctx_entry.ssid_vector_0_or_flooding_vport, ddr_ctx_entry.ssid_vector_1, ddr_ctx_entry.ssid_vector_2);
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

    RDD_PAUSE_QUANTA_ENTRY_IGNORE_WRITE_G(pause_ignore, RDD_DIRECT_FLOW_PAUSE_QUANTA_ADDRESS_ARR, 0);

    return 0;
}

static int rdd_ae_pause_frame_control(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t value = parm[0].value.unumber;

    if (value == 0xffffffff)
    {
        RDD_BYTES_4_BITS_READ_G(value, RDD_DIRECT_FLOW_PAUSE_DEBUG_ADDRESS_ARR, 1);
        bdmf_session_print(session, "AE XOFF frames received: %d\n", value);
        RDD_BYTES_4_BITS_READ_G(value, RDD_DIRECT_FLOW_PAUSE_DEBUG_ADDRESS_ARR, 0);
        bdmf_session_print(session, "AE XON frames received: %d\n", value);
        RDD_BYTES_2_BITS_READ_G(value, RDD_DIRECT_FLOW_PAUSE_QUANTA_ADDRESS_ARR, 0);
        bdmf_session_print(session, "Pause quanta:  %d\n", value);
        RDD_BYTES_4_BITS_READ_G(value, RDD_DIRECT_FLOW_PAUSE_DEBUG_ADDRESS_ARR, 2);
        bdmf_session_print(session, "AE calculated pause (us): %d\n", value);
    }
    else
    {
        RDD_BYTES_2_BITS_WRITE_G(value, RDD_DIRECT_FLOW_PAUSE_QUANTA_ADDRESS_ARR, 0);
    }

    return 0;
}
#endif /* !defined(BCM63158) */

static int _rdd_ingress_qos_drop_miss_ratio_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t drop_miss_ratio = parm[0].value.unumber;

    rdd_ingress_qos_drop_miss_ratio_set(drop_miss_ratio);

    return 0;
}

#if defined(OPERATION_MODE_PRV) && (defined(BCM6858) || defined(BCM6856))
static int rdd_multicast_limit_cfg(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t mcast_max = parm[0].value.unumber;
    uint8_t mcast_min = parm[1].value.unumber;

    rdd_mcast_max_tasks_limit_cfg(mcast_max);
    rdd_mcast_min_tasks_limit_cfg(mcast_min);

    return 0;
}
#endif

#ifdef G9991_COMMON
static int rdd_g9991_flow_control_print_debug(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#ifndef MULTIPLE_BBH_TX_LAN
    G9991_SCHEDULING_INFO_ENTRY_STRUCT *entry;
    uint32_t flow_control_vector_0_1, flow_control_vector_2_3;
    uint32_t sid_to_phy_port_mask[RDD_G9991_SCHEDULING_INFO_TABLE_SIZE];
    uint32_t sid_flow_control_status;
    uint32_t i, j;
#endif

    bdmf_session_print(session, "\n\nFlow control Vector\n");
    bdmf_session_print(session, "-------------------\n");

#ifdef MULTIPLE_BBH_TX_LAN
    bdmf_session_print(session, "ERR: not supported for multiple BBH_TX\n");
#else
    RDD_BYTES_4_BITS_READ(flow_control_vector_0_1, (BYTES_4_STRUCT *)RDD_G9991_FLOW_CONTROL_VECTOR_PTR(get_runner_idx(ds_tm_runner_image)));
    RDD_BYTES_4_BITS_READ(flow_control_vector_2_3, (BYTES_4_STRUCT *)RDD_G9991_FLOW_CONTROL_VECTOR_PTR(get_runner_idx(ds_tm_runner_image)));

    entry = ((G9991_SCHEDULING_INFO_ENTRY_STRUCT *)RDD_G9991_SCHEDULING_INFO_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + 0;
    RDD_G9991_SCHEDULING_INFO_ENTRY_PHYSICAL_PORT_MAPPING_READ(sid_to_phy_port_mask[0], entry);

    entry = ((G9991_SCHEDULING_INFO_ENTRY_STRUCT *)RDD_G9991_SCHEDULING_INFO_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + 1;
    RDD_G9991_SCHEDULING_INFO_ENTRY_PHYSICAL_PORT_MAPPING_READ(sid_to_phy_port_mask[1], entry);

    entry = ((G9991_SCHEDULING_INFO_ENTRY_STRUCT *)RDD_G9991_SCHEDULING_INFO_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + 2;
    RDD_G9991_SCHEDULING_INFO_ENTRY_PHYSICAL_PORT_MAPPING_READ(sid_to_phy_port_mask[2], entry);

    entry = ((G9991_SCHEDULING_INFO_ENTRY_STRUCT *)RDD_G9991_SCHEDULING_INFO_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + 3;
    RDD_G9991_SCHEDULING_INFO_ENTRY_PHYSICAL_PORT_MAPPING_READ(sid_to_phy_port_mask[3], entry);

    bdmf_session_print(session, "sid   status\n");
    bdmf_session_print(session, "------------\n");

    for (i = 0; i < 32; i++) {
        bdmf_session_print(session, " %02i: ", i);

        /* find to which hsgmii the currenet sid is belong */
        for (j = 0; j < RDD_G9991_SCHEDULING_INFO_TABLE_SIZE; j++)
            if (sid_to_phy_port_mask[j] & 1 << i)
               break;

        if (j == RDD_G9991_SCHEDULING_INFO_TABLE_SIZE) {
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
#endif
    return 0;
}
#endif
