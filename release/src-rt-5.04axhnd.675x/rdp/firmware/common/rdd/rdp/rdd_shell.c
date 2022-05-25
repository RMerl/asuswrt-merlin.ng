/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#include "rdd_shell_common.h"
#include "rdd_natc_lkp.h"
#include "rdd_ip_flow.h"
#include "rdp_natcache.h"

extern uint32_t g_runner_ddr0_base_addr;
extern uint32_t g_runner_ddr1_base_addr;
extern uint32_t g_runner_psram_base_addr;
extern uint32_t g_runner_nat_cache_key_ptr;
extern uint32_t g_runner_nat_cache_context_ptr;
extern uint32_t g_runner_ddr_base_addr;

static int p_rdd_flow_pm_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_bridge_port_pm_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_various_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_check_lists(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_print_connections_lookup_table(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_print_connections_context_table(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_print_connections_context_statistics(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_rdd_print_nat_cache_internal_memory(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);

int f_rdd_make_shell_commands(void)
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

    MAKE_BDMF_SHELL_CMD(rdd_directory, "pwrdn", "print wan rx descriptors - normal", p_rdd_print_wan_bbh_rx_descriptors_normal,
       BDMFMON_MAKE_PARM_RANGE("WAN", "WAN number", BDMFMON_PARM_NUMBER, 0, 0, 1));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "plrdn", "print lan rx descriptors - normal", p_rdd_print_lan_bbh_rx_descriptors_normal,
        BDMFMON_MAKE_PARM_RANGE("LAN", "LAN number", BDMFMON_PARM_NUMBER, 0, 0, 2));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pdb",    "print buffer packet data", p_rdd_print_packet_buffer,
        BDMFMON_MAKE_PARM("buffer_number", "FPM/SBPM Buffer number (hex)", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("target_memory", "Target memory", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("ddr_id", "DDR id", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM("payload_offset", "Payload offset (hex)", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("length", "Buffer length (hex)", BDMFMON_PARM_HEX, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pfc", "print flow pm counters", p_rdd_flow_pm_counters_get,
        BDMFMON_MAKE_PARM_RANGE("flow", "Flow id", BDMFMON_PARM_NUMBER, 0, 0, 255),
        BDMFMON_MAKE_PARM_RANGE("pm_type", "0-RX,1-TX,2-both", BDMFMON_PARM_NUMBER, 0, 1, 2));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pbpc", "print bridge port pm counters", p_rdd_bridge_port_pm_counters_get,
        BDMFMON_MAKE_PARM_RANGE("bridge_port", "0:WAN-0,1-3:LAN,8-WIFI,9:WAN-1", BDMFMON_PARM_NUMBER, 0, 0, 9));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pvdc", "print various drop counters", p_rdd_various_counters_get,
        BDMFMON_MAKE_PARM_RANGE("us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "sp",      "start profiling", p_rdd_start_profiling,
        BDMFMON_MAKE_PARM_RANGE("runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("task1", "Task #1 (up to 15 in Pico)", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM_RANGE("task2", "Task #2 (up to 15 in Pico)", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM_RANGE("trace_pc", "Trace PC enable", BDMFMON_PARM_NUMBER, 0, 0, 31));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "sp1",     "stop profiling", p_rdd_stop_profiling,
        BDMFMON_MAKE_PARM_RANGE("main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1));
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "ppr",     "print profiling registers", p_rdd_print_profiling_registers);
#if !defined(FIRMWARE_INIT)
    MAKE_BDMF_SHELL_CMD(rdd_directory, "re",      "enable or disable runner",  p_rdd_runner_enable,
        BDMFMON_MAKE_PARM_RANGE("runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("enable", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "bc",      "configure breakpoint", p_rdd_breakpoint_config,
        BDMFMON_MAKE_PARM_RANGE("runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("step_mode", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "bs",      "set breakpoint", p_rdd_set_breakpoint,
        BDMFMON_MAKE_PARM_RANGE("runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("index", "Breakpoint index", BDMFMON_PARM_NUMBER, 0, 0, 3),
        BDMFMON_MAKE_PARM_RANGE("enable", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("breakpoint_address", "Address (16 bit)", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("use_thread", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("thread", "Thread number", BDMFMON_PARM_NUMBER, 0, 0, 31));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pbs",     "print breakpoint status", p_rdd_print_breakpoint_status,
        BDMFMON_MAKE_PARM_RANGE("runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1));
#endif
    MAKE_BDMF_SHELL_CMD(rdd_directory, "cl",      "check lists", p_rdd_check_lists,
        BDMFMON_MAKE_PARM("ds_free", "Number of PDs to print in DS free PD pool", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pclt",    "print connections lookup table", p_rdd_print_connections_lookup_table,
        BDMFMON_MAKE_PARM("start", "Start index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("number", "Number of connections", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pcct",    "print connections context table", p_rdd_print_connections_context_table,
        BDMFMON_MAKE_PARM("start", "Start index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("number", "Number of connections", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pccs",    "print connections context statistics", p_rdd_print_connections_context_statistics,
        BDMFMON_MAKE_PARM("start", "Start index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_directory, "pncim", "print NAT cache internal memory", p_rdd_print_nat_cache_internal_memory);
    MAKE_BDMF_SHELL_CMD(rdd_directory, "ptl",   "print tables list", p_rdd_print_tables_list,
        BDMFMON_MAKE_PARM("name", "Table name, may have wildcards", BDMFMON_PARM_STRING, BDMFMON_PARM_FLAG_OPTIONAL));
    MAKE_BDMF_SHELL_CMD(rdd_directory, "pte",   "print table entries", p_rdd_print_table_entries,
        BDMFMON_MAKE_PARM("name", "Full Table name", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("index/address", "0 for entry index to start, 1 for entry address to start", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry index", "Start entry index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry address", "Start entry address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("number", "Number of entries", BDMFMON_PARM_NUMBER, 0));

    return 0;
}



static int p_rdd_flow_pm_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdd_flow_pm_counters_t flow_counters;
    uint32_t direction;
    uint32_t flow_id;
    int rdd_error;

    flow_id = parm[0].value.unumber;
    direction = parm[1].value.unumber;

    rdd_error = rdd_flow_pm_counters_get(flow_id, direction, 0, &flow_counters);

    if (!rdd_error)
    {
        bdmf_session_print(session, "Flow ID %d PM  counters:\n", (uint32_t)flow_id);
        bdmf_session_print(session, "==============================\n");

        if (direction == 0)
        {
            bdmf_session_print(session, "Good RX bytes:    %-6u\n", (uint32_t)flow_counters.good_rx_bytes);
            bdmf_session_print(session, "Good RX packets:  %-6u\n", (uint32_t)flow_counters.good_rx_packet);
            bdmf_session_print(session, "RX drops:         %-6u\n", (uint32_t)flow_counters.error_rx_packets_discard);
        }

        if (direction == 1)
        {
            bdmf_session_print(session, "Good TX bytes:    %-6u\n", (uint32_t)flow_counters.good_tx_bytes);
            bdmf_session_print(session, "Good TX packets:  %-6u\n", (uint32_t)flow_counters.good_tx_packet);
            bdmf_session_print(session, "TX drops:         %-6u\n", (uint32_t)flow_counters.error_tx_packets_discard);
        }
    }
    else
        bdmf_session_print(session, "rdd_flow_pm_counters_get returned error\n");

    return rdd_error;
}

static int p_rdd_bridge_port_pm_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdd_vport_pm_counters_t pm_counters;
    uint32_t bridge_port;
    int rdd_error;

    bridge_port = parm[0].value.unumber;

    rdd_error = rdd_vport_pm_counters_get(bridge_port, 0, &pm_counters);

    if (!rdd_error)
    {
        bdmf_session_print(session, "   Bridge port PM counters:\n");
        bdmf_session_print(session, "==============================\n");
        bdmf_session_print(session, "bridge valid rx packets:    %-6u\n", (uint32_t)pm_counters.rx_valid);
        bdmf_session_print(session, "bridge valid tx packets:    %-6u\n", (uint32_t)pm_counters.tx_valid);
        bdmf_session_print(session, "bridge filtered packets:    %-6u\n", (uint32_t)pm_counters.bridge_filtered_packets);
        bdmf_session_print(session, "bridge tx packets discard:  %-6u\n", (uint32_t)pm_counters.bridge_tx_packets_discard);
        bdmf_session_print(session, "error rx bpm congestion:    %-6u\n", (uint32_t)pm_counters.error_rx_bpm_congestion);
    }
    else
        bdmf_session_print(session, "rdd_vport_pm_counters_get returned error\n");

    return rdd_error;
}

static int p_rdd_various_counters_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    rdd_various_counters_t various_counters = {};
    bdmf_error_t rdd_error;
    rdpa_traffic_dir direction;
    uint32_t index;

    static uint8_t ingress_filters_arr[][16] = {"IGMP", "ICMPV6", "USER_0", "USER_1", "USER_2", "USER_3", "PPPOE_D",
        "PPPOE_S", "ARP", "1588", "802.1X", "802.1AG_CFM", "BROADCAST", "MULTICAST"};
    static uint8_t l4_filters_arr[][16] = {"ERROR", "EXCEPTION", "IP_FIRST_FRAG", "IP_FRAGMENT", "ICMP", "ESP", "GRE",
        "AH", "IPv6", "USER_DEFINED_0", "USER_DEFINED_1", "USER_DEFINED_2", "USER_DEFINED_3", "UNKNOWN", "EMPTY",
        "EMPTY"};
    direction = parm[0].value.unumber;
    bdmf_session_print(session, "call rdd_various_counters_get\n");
    rdd_error = rdd_various_counters_get(direction, 0xffffffff, 0, &various_counters);
    bdmf_session_print(session, "end rdd_various_counters_get\n");
    if (rdd_error)
        bdmf_session_print(session, "rdd_various_counters_get returned error\n");
    else
    {
        bdmf_session_print(session, "   %-s counters:\n", (direction == rdpa_dir_ds) ? "Downstream" : "Upstream");
        bdmf_session_print(session, "==============================\n");

        if (direction == rdpa_dir_us)
        {
            bdmf_session_print(session, "ACL OUI drop:                               %-6u\n",
                various_counters.acl_oui_drop);
#ifdef UNDEF
            bdmf_session_print(session, "ACL L2 drop:                                %-6u\n", various_counters.acl_l2_drop);
            bdmf_session_print(session, "ACL L3 drop:                                %-6u\n", various_counters.acl_l3_drop);
            bdmf_session_print(session, "Local switching congestion:                 %-6u\n", various_counters.local_switching_congestion);
            bdmf_session_print(session, "EPON DDR queue drop:                        %-6u\n", various_counters.us_ddr_queue_drop);
#endif
        }
#ifdef UNDEF
        else
        {
            bdmf_session_print(session, "dst mac non router drop:                    %-6u\n", various_counters.dst_mac_non_router_drop);
            bdmf_session_print(session, "firewall drop:                              %-6u\n", various_counters.firewall_drop);
            bdmf_session_print(session, "invalid layer2 protocol drop:               %-6u\n", various_counters.invalid_layer2_protocol_drop);
            bdmf_session_print(session, "IPTV layer 3 drop:                          %-6u\n", various_counters.iptv_layer3_drop);
            bdmf_session_print(session, "DS policers drop:                           %-6u\n", various_counters.downstream_policers_drop);
            bdmf_session_print(session, "EMAC loopback drop:                         %-6u\n", various_counters.emac_loopback_drop);
            bdmf_session_print(session, "Dual Stack Lite congestion drop:            %-6u\n", various_counters.dual_stack_lite_congestion_drop);
            bdmf_session_print(session, "Absolute Address List Overflow drop:        %-6u\n", various_counters.absolute_address_list_overflow_drop);
        }
        bdmf_session_print(session, "ETH flow action drop:                       %-6u\n", various_counters.eth_flow_action_drop);
        bdmf_session_print(session, "SA lookup failure drop:                     %-6u\n", various_counters.sa_lookup_failure_drop);
        bdmf_session_print(session, "DA lookup failure drop:                     %-6u\n", various_counters.da_lookup_failure_drop);
        bdmf_session_print(session, "SA action drop:                             %-6u\n", various_counters.sa_action_drop);
        bdmf_session_print(session, "DA action drop:                             %-6u\n", various_counters.da_action_drop);
        bdmf_session_print(session, "Forw. matrix disabled drop:                 %-6u\n", various_counters.forwarding_matrix_disabled_drop);
        bdmf_session_print(session, "VLAN switching drop:                        %-6u\n", various_counters.vlan_switching_drop);
#endif
        bdmf_session_print(session, "Connection action drop:                     %-6u\n",
            various_counters.connection_action_drop);
        for (index = 0; index < 16; index++)
        {
            bdmf_session_print(session, "%-16s (ingress filter %-2u) drop:  %-6u\n", ingress_filters_arr[index],
                (unsigned) index, (unsigned) various_counters.ingress_filters_drop[index]);
        }
        bdmf_session_print(session, "2\n");

        for (index = 0; index <= RDD_LAYER4_FILTER_UNKNOWN; index++)
        {
            bdmf_session_print(session, "%-16s (layer4 filter %-2u) drop:   %-6u\n", l4_filters_arr[index],
                (unsigned) index, (unsigned) various_counters.layer4_filters_drop[index]);
        }
        bdmf_session_print(session, "3\n");
        bdmf_session_print(session, "Header error drop:                          %-6u\n",
            (unsigned) various_counters.ip_validation_filter_drop[0]);
        bdmf_session_print(session, "4\n");
        bdmf_session_print(session, "IP fragments drop:                          %-6u\n",
            (unsigned) various_counters.ip_validation_filter_drop[1]);
        bdmf_session_print(session, "5\n");
        bdmf_session_print(session, "TPID detect drop:                           %-6u\n",
            (unsigned) various_counters.tpid_detect_drop);
        bdmf_session_print(session, "Done\n");
    }

    return rdd_error;
}



static uint32_t p_rdd_check_list(bdmf_session_handle session, uint32_t head_address, uint32_t tail_address,
    uint32_t max_list_size, uint32_t memory_segment_offset, uint32_t print_descriptors)
{
    RDD_PACKET_DESCRIPTOR_DTS *packet_descriptor_ptr;
    uint32_t next_packet_descriptor_address;
    uint32_t i;

    if (print_descriptors > 0)
        bdmf_session_print(session, "PD    1: 0x%04X\n", head_address);

    packet_descriptor_ptr = (RDD_PACKET_DESCRIPTOR_DTS *)(memory_segment_offset + head_address);

    if (head_address == tail_address)
        return 1;

    for (i = 0; i < max_list_size; i++)
    {
        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_READ(next_packet_descriptor_address, packet_descriptor_ptr);

        if (next_packet_descriptor_address == tail_address)
            return i + 2;

        if (print_descriptors > i)
            bdmf_session_print(session, "PD %-4u: 0x%04X\n", (i + 2), head_address);

        packet_descriptor_ptr = (RDD_PACKET_DESCRIPTOR_DTS *)(memory_segment_offset + next_packet_descriptor_address);
    }

    return 0;
}

static void p_rdd_check_lists_ds_free_pd_pool(bdmf_session_handle session, uint32_t print_descriptors)
{
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *free_packet_descriptors_pool_descriptor;
    uint32_t head_address;
    uint32_t tail_address;
    uint32_t ingress_counter;
    uint32_t egress_counter;
    uint32_t list_size;

    free_packet_descriptors_pool_descriptor =
        (RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS);

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_READ(head_address, free_packet_descriptors_pool_descriptor);
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_READ(tail_address, free_packet_descriptors_pool_descriptor);
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_READ(ingress_counter, free_packet_descriptors_pool_descriptor);
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_READ(egress_counter, free_packet_descriptors_pool_descriptor);

    bdmf_session_print(session, "DS free PD pool:\n");

    list_size = p_rdd_check_list(session, head_address, tail_address, RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE,
         (uint32_t)DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET), print_descriptors);

    bdmf_session_print(session, "List counter         %-4u\n", ingress_counter - egress_counter);

    if (list_size != RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE)
       bdmf_session_print(session, "PDs             ERROR\n");
    else
       bdmf_session_print(session, "PDs number      %-4u\n", list_size);

    return;
}

static int p_rdd_check_lists(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t print_ds_free_pd_pool;

    print_ds_free_pd_pool = (uint32_t)parm[0].value.unumber;

    bdmf_session_print(session, "Lists Integrity check:\n");
    bdmf_session_print(session, "----------------------\n\n");

    /* DS free PD pool */
    p_rdd_check_lists_ds_free_pd_pool(session, print_ds_free_pd_pool);
    bdmf_session_print(session, "\n");

    return 0;
}

static void f_rdd_print_connection_lookup_helper(bdmf_session_handle session, uint32_t index, uint32_t table_format)
{
    rdpa_ip_flow_key_t connection_entry;
    uint32_t ip_address;
    uint32_t context_idx;
    uint32_t connection_valid = 0;
    bdmf_error_t rc;

    rc = rdd_ip_flow_get(NULL, index, &connection_entry, &context_idx);

    if (rc == BDMF_ERR_RANGE)
        bdmf_session_print(session, "Index is out of range\n");
    else if (rc == BDMF_ERR_NOENT)
    {
        if (table_format == 0)
            bdmf_session_print(session, "Valid:                   %6d\n", connection_valid);
        else
        {
            bdmf_session_print(session, " %-6d ", index);
            bdmf_session_print(session, " %-6d\n", connection_valid);
        }
    }
    else if (rc == BDMF_ERR_OK)
    {
        connection_valid = 1;

        if (table_format == 0)
        {
            bdmf_session_print(session, "Valid:                   %6d\n", connection_valid);
            bdmf_session_print(session, "Protocol:                %6d\n", connection_entry.prot);

            if (connection_entry.dst_ip.family == 0)
                bdmf_session_print(session, "IP Version:              IPV4\n");
            else
                bdmf_session_print(session, "IP Version:              IPV6\n");

            ip_address = swap4bytes(connection_entry.src_ip.addr.ipv4);

            if (connection_entry.dst_ip.family == 0)
                bdmf_session_print(session, "Source IP:               %pI4\n", &ip_address);
            else
                bdmf_session_print(session, "Source IP CRC:           0x%08x\n", ip_address);

            ip_address = swap4bytes(connection_entry.dst_ip.addr.ipv4);

            if (connection_entry.dst_ip.family == 0)
                bdmf_session_print(session, "Destination IP:          %pI4\n", &ip_address);
            else
                bdmf_session_print(session, "Destination IP CRC:      0x%08x\n", ip_address);

            bdmf_session_print(session, "Source port:             %6d\n", connection_entry.src_port);
            bdmf_session_print(session, "Destination port:        %6d\n", connection_entry.dst_port);
        }
        else
        {
            bdmf_session_print(session, " %-6d ", index);
            bdmf_session_print(session, " %-6d ", connection_valid);
            bdmf_session_print(session, " %-9d ", connection_entry.prot);
            bdmf_session_print(session, " %-10d ", connection_entry.src_port);
            bdmf_session_print(session, " %-11d ", connection_entry.dst_port);

            ip_address = swap4bytes(connection_entry.src_ip.addr.ipv4);

            if (connection_entry.dst_ip.family == 0)
                bdmf_session_print(session, " %pI4  ", &ip_address);
            else
                bdmf_session_print(session, "  0x%08x   ", ip_address);

            ip_address = swap4bytes(connection_entry.dst_ip.addr.ipv4);

            if (connection_entry.dst_ip.family == 0)
                bdmf_session_print(session, " %pI4  ", &ip_address);
            else
                bdmf_session_print(session, "  0x%08x    ", ip_address);

            if (connection_entry.dst_ip.family == 0)
                bdmf_session_print(session, " IPV4\n");
            else
                bdmf_session_print(session, " IPV6\n");
        }
    }
}

static int p_rdd_print_connections_lookup_table(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t start_index;
    uint32_t number_of_connections;
    uint32_t i;

    start_index = parm[0].value.unumber;
    number_of_connections = parm[1].value.unumber;

    bdmf_session_print(session, " Connections Lookup Table\n");
    bdmf_session_print(session, "-------------------------\n");

    bdmf_session_print(session, " index | valid | protocol | src. port | dest. port |    src. ip    |    dst. ip    | IP version\n");

    for (i = start_index; i < (start_index + number_of_connections); i++)
    {
        if (i >= RDD_NAT_CACHE_TABLE_SIZE)
            break;

        f_rdd_print_connection_lookup_helper(session, i, 1);
    }

    return 0;
}

static void f_rdd_print_connection_context_helper(bdmf_session_handle session, uint32_t index, uint32_t table_format)
{
    RDD_NAT_CACHE_LKP_ENTRY_DTS *connection_lookup_entry_ptr;
    rdd_fc_context_t fc_ctx;
    uint32_t hit_cnt, bytes_cnt, connection_valid, i;
    bdmf_error_t rc = BDMF_ERR_OK;

    connection_lookup_entry_ptr = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[index];
    RDD_NAT_CACHE_LKP_ENTRY_VALID_READ(connection_valid, connection_lookup_entry_ptr);

    if (connection_valid)
    {
        rc = rdd_context_entry_get(index, &fc_ctx);

        if (rc)
            bdmf_session_print(session, "Index is out of range\n");

        rdd_ip_flow_counters_get(index, &hit_cnt, &bytes_cnt);

        bdmf_session_print(session, "Hit count:               %6u\n", hit_cnt);
        bdmf_session_print(session, "Byte count:              %6u\n", bytes_cnt);
        bdmf_session_print(session, "NAT IP:                  %pI4\n", &fc_ctx.nat_ip.addr.ipv4);
        bdmf_session_print(session, "NAT port:                %6d\n", fc_ctx.nat_port);
        bdmf_session_print(session, "Virt Egress port:        %6d\n", fc_ctx.vir_egress_port);
        bdmf_session_print(session, "Phy Egress port:         %6d\n", fc_ctx.phy_egress_port);
        bdmf_session_print(session, "WIFI ssid:               %6d\n", fc_ctx.wifi_ssid);
        bdmf_session_print(session, "L2 offset:               %6d\n", fc_ctx.l2_hdr_offset);
        bdmf_session_print(session, "L2 size:                 %6d\n", fc_ctx.l2_hdr_size);

        for (i = 0; i < fc_ctx.l2_hdr_size; i++)
            bdmf_session_print(session, "0x%02x ", fc_ctx.l2_header[i]);

        bdmf_session_print(session, "\n");
    }
}

static int p_rdd_print_connections_context_table(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t start_index;
    uint32_t number_of_connections;
    uint32_t i;

    start_index = parm[0].value.unumber;
    number_of_connections = parm[1].value.unumber;

    bdmf_session_print(session, " Connections Context Table\n");
    bdmf_session_print(session, "--------------------------\n");

    for (i = start_index; i < (start_index + number_of_connections); i++)
    {
        if (i >= RDD_NAT_CACHE_TABLE_SIZE)
            break;

        f_rdd_print_connection_context_helper(session, i, 1);
    }

    return 0;
}

static int p_rdd_print_connections_context_statistics(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t start_index;
    uint32_t hit_count, byte_count;

    start_index = parm[0].value.unumber;

    bdmf_session_print(session, " Connections Context statistics\n");
    bdmf_session_print(session, "-------------------------------\n");

    rdd_ip_flow_counters_get(start_index, &hit_count, &byte_count);

    bdmf_session_print(session, "packet counter = %d   byte counter = %d\n", hit_count, byte_count);

    return 0;
}

static int p_rdd_print_nat_cache_internal_memory(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t *indirect_addr_regs_addr;
    uint32_t *indirect_data_regs_addr;
    uint32_t nat_cache_entry_idx;
    uint32_t nat_cache_data_word_idx;
    uint32_t register_value;

    indirect_addr_regs_addr = (uint32_t *)NATCACHE_RDP_INDIRECT_ADDRESS;
    indirect_data_regs_addr = (uint32_t *)NATCACHE_RDP_INDIRECT_DATA;

    for (nat_cache_entry_idx = 0; nat_cache_entry_idx < 1024; nat_cache_entry_idx++)
    {
       register_value = (0 << 10) | nat_cache_entry_idx;
       WRITE_32(indirect_addr_regs_addr, register_value);

       READ_32(indirect_data_regs_addr, register_value);

       if (register_value == 0)
           continue;

       bdmf_session_print(session, "Entry: 0x%x\n", nat_cache_entry_idx);

       for (nat_cache_data_word_idx = 0; nat_cache_data_word_idx < 20; nat_cache_data_word_idx++)
       {
           READ_32(indirect_data_regs_addr + nat_cache_data_word_idx, register_value);
           bdmf_session_print(session, "0x%08x ", register_value);

           /* 2 words per line */
           if (nat_cache_data_word_idx & 1)
               bdmf_session_print(session, "\n");
       }
    }

    return 0;
}

