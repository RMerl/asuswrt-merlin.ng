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
#include "rdd_runner_reg_dump.h"
#include "rdd_runner_reg_dump_addrs.h"
#ifdef WL4908
#include "rdd_utils.h"
#endif
extern uint8_t *g_runner_ddr_base_addr;
extern rdpa_bpm_buffer_size_t g_bpm_buffer_size;
extern uint8_t *g_runner_psram_base_addr;

#if !(defined(WL4908) && defined(BDMF_SYSTEM_SIM))
static void p_rdd_print_bbh_rx_descriptors_helper(bdmf_session_handle, RDD_BBH_RX_DESCRIPTOR_DTS *, uint32_t);

int p_rdd_print_wan_bbh_rx_descriptors_normal(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_BBH_RX_DESCRIPTOR_DTS *wan_bbh_rx_descriptor_ptr;
    uint32_t wan_id;
    uint32_t wan_rx_fifo_size;

    wan_id = (int32_t)parm[0].value.unumber;

    switch (wan_id)
    {
    case 0:
        wan_bbh_rx_descriptor_ptr = (RDD_BBH_RX_DESCRIPTOR_DTS *)RDD_WAN_RX_NORMAL_DESCRIPTORS_PTR();
        wan_rx_fifo_size = RDD_WAN_RX_NORMAL_DESCRIPTORS_SIZE;
        break;
    default:
        bdmf_session_print(session, "UT: Not enough parameters\n\n\r");
        return BDMF_ERR_PARM;
    }

    bdmf_session_print(session, "WAN%u RX Descriptors - Normal:\n", (uint32_t)wan_id);
    bdmf_session_print(session, "------------------------------\n");

    p_rdd_print_bbh_rx_descriptors_helper(session, wan_bbh_rx_descriptor_ptr, wan_rx_fifo_size);

    return 0;
}

int p_rdd_print_lan_bbh_rx_descriptors_normal(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_BBH_RX_DESCRIPTOR_DTS *lan_bbh_rx_descriptor_ptr;
    uint32_t lan_id;
    uint32_t lan_rx_fifo_size;

    lan_id = (int32_t)parm[0].value.unumber;

    switch (lan_id)
    {
    case 0:
        lan_bbh_rx_descriptor_ptr = (RDD_BBH_RX_DESCRIPTOR_DTS *)RDD_LAN0_RX_DESCRIPTORS_PTR();
        lan_rx_fifo_size = RDD_LAN0_RX_DESCRIPTORS_SIZE;
        break;

    case 1:
        lan_bbh_rx_descriptor_ptr = (RDD_BBH_RX_DESCRIPTOR_DTS *)RDD_LAN1_RX_DESCRIPTORS_PTR();
        lan_rx_fifo_size = RDD_LAN1_RX_DESCRIPTORS_SIZE;
        break;

    case 2:
        lan_bbh_rx_descriptor_ptr = (RDD_BBH_RX_DESCRIPTOR_DTS *)RDD_LAN2_RX_DESCRIPTORS_PTR();
        lan_rx_fifo_size = RDD_LAN2_RX_DESCRIPTORS_SIZE;
        break;

    default:
        bdmf_session_print(session, "UT: Not enough parameters\n\n\r");
        return BDMF_ERR_PARM;
    }

    bdmf_session_print(session, "LAN%u RX Descriptors (normal):\n", (uint32_t)lan_id);
    bdmf_session_print(session, "------------------------------\n");

    p_rdd_print_bbh_rx_descriptors_helper(session, lan_bbh_rx_descriptor_ptr, lan_rx_fifo_size);

    return 0;
}

static void p_rdd_print_bbh_rx_descriptors_helper(bdmf_session_handle session, RDD_BBH_RX_DESCRIPTOR_DTS *rx_descriptor_ptr, uint32_t rx_fifo_size)
{
    uint32_t last_sbn;
    uint32_t packet_length;
    uint8_t   error;
    uint32_t ih_buffer_number;
    uint8_t   target_memory;
    uint32_t buffer_number;
    uint32_t error_type;
    uint32_t i;
    uint32_t payload_offset;
    uint32_t ddr_id;

    bdmf_session_print ( session, "last SBN | FStat Cell | Flow ID | Packet length | Error | FStat Error | Error type | IH BN | Target Memory | Buffer Number\n" );

    for (i = 0; i < rx_fifo_size; i++)
    {
        RDD_BBH_RX_DESCRIPTOR_LAST_SBN_READ ( last_sbn, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_PAYLOAD_OFFSET_READ ( payload_offset, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_PACKET_LENGTH_READ ( packet_length, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_ERROR_READ ( error, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_ERROR_TYPE_READ ( error_type, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_IH_BUFFER_NUMBER_READ ( ih_buffer_number, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_TARGET_MEMORY_READ ( target_memory, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_DDR_ID_READ ( ddr_id, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_BUFFER_NUMBER_READ ( buffer_number, rx_descriptor_ptr );

        bdmf_session_print(session, " 0x%-5x |    %-7u     |    %-7u    |   %1u   |   0x%-4x   |  %-3u  |       %1u       |   %1u    |    0x%-5x\n",
            last_sbn, payload_offset, packet_length, error, error_type, ih_buffer_number, target_memory, ddr_id, buffer_number);
        rx_descriptor_ptr++;
    }
}

int p_rdd_print_packet_buffer(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t *packet_buffer_ptr;
    uint32_t buffer_number;
    uint32_t target_memory;
    uint32_t payload_offset;
    uint32_t packet_length;
    uint32_t value;
    uint32_t i;

    buffer_number = parm[1].value.unumber;
    target_memory = parm[2].value.unumber;
    payload_offset = parm[4].value.unumber;
    packet_length = parm[5].value.unumber;

    if (target_memory == 0)
        packet_buffer_ptr = (uint8_t *)(g_runner_ddr_base_addr + buffer_number * g_bpm_buffer_size + payload_offset);
    else
        /* The packet is PSRAM */
        packet_buffer_ptr = (uint8_t *)(g_runner_psram_base_addr + buffer_number * 128);

    for (i = 0; i < packet_length; i++, packet_buffer_ptr++)
    {
        if ((i % 16) == 0)
            bdmf_session_print(session, "%p  ", packet_buffer_ptr);

        /* MREAD_8(packet_buffer_ptr, value); */
        value = *(uint8_t *)packet_buffer_ptr;
        bdmf_session_print(session, "%02x ", value);

        if (((i + 1) % 16) == 0)
            bdmf_session_print(session, "\n");
    }

    bdmf_session_print(session, "\n");

    return 0;
}

int p_rdd_start_profiling(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_MAIN_PROFILING_CFG runner_profiling_cfg_register;
    uint32_t runner_id;
    uint32_t is_pico;
    uint32_t task1;
    uint32_t task2;
    uint32_t trace_pc_en;

    runner_id = (uint32_t)parm[0].value.unumber;
    is_pico = (uint32_t)parm[1].value.unumber;
    task1 = (uint32_t)parm[2].value.unumber;
    task2 = (uint32_t)parm[3].value.unumber;
    trace_pc_en = (uint32_t)parm[4].value.unumber;

    if (task1 >= (32 / (is_pico + 1)))
    {
        bdmf_session_print(session, "UT: Invalid task 1 value\n\n\r");
        return BDMF_ERR_PARM;
    }

    if (task1 >= (32 / (is_pico + 1)))
    {
        bdmf_session_print (session, "UT: Invalid task 2 value\n\n\r");
        return BDMF_ERR_PARM;
    }

    /* Fill profiling configuration register fields (inc. start profiling bit) */
    if (runner_id == 0)
        runner_profiling_cfg_register.trace_base_addr = (DS_PROFILING_BUFFER_RUNNER_ADDRESS >> 3);
    else
        runner_profiling_cfg_register.trace_base_addr = (US_PROFILING_BUFFER_RUNNER_ADDRESS >> 3);

    runner_profiling_cfg_register.trace_pc_en = trace_pc_en;
    runner_profiling_cfg_register.prof_task1 = task1;
    runner_profiling_cfg_register.prof_task2 = task2;
    runner_profiling_cfg_register.prof_start = 1;
    runner_profiling_cfg_register.prof_stop = 0;
    runner_profiling_cfg_register.rsv1 = 0;
    runner_profiling_cfg_register.rsv2 = 0;

    /* Write profiling configuration register */
    if (is_pico == 0)
        if (runner_id == 0)
            RUNNER_REGS_0_CFG_MAIN_PROFILING_CFG_WRITE(runner_profiling_cfg_register);
        else
            RUNNER_REGS_1_CFG_MAIN_PROFILING_CFG_WRITE(runner_profiling_cfg_register);
    else
        if (runner_id == 0)
            RUNNER_REGS_0_CFG_PICO_PROFILING_CFG_WRITE(runner_profiling_cfg_register);
        else
            RUNNER_REGS_1_CFG_PICO_PROFILING_CFG_WRITE(runner_profiling_cfg_register);

    return 0;
}

int p_rdd_stop_profiling(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_MAIN_PROFILING_CFG runner_profiling_cfg_register;
    uint32_t runner_id;
    uint32_t is_pico;

    runner_id = (uint32_t)parm[0].value.unumber;
    is_pico = (uint32_t)parm[1].value.unumber;

    /* Read profiling configuration register */
    if (is_pico == 0)
        if (runner_id == 0)
            RUNNER_REGS_0_CFG_MAIN_PROFILING_CFG_READ(runner_profiling_cfg_register);
        else
            RUNNER_REGS_1_CFG_MAIN_PROFILING_CFG_READ(runner_profiling_cfg_register);
    else
        if (runner_id == 0)
            RUNNER_REGS_0_CFG_PICO_PROFILING_CFG_READ(runner_profiling_cfg_register);
        else
            RUNNER_REGS_1_CFG_PICO_PROFILING_CFG_READ(runner_profiling_cfg_register);

    /* Update profiling configuration register fields (stop profiling bit) */
    runner_profiling_cfg_register.prof_start = 0;
    runner_profiling_cfg_register.prof_stop = 1;

    /* Write updated profiling configuration register */
    if (is_pico == 0)
        if (runner_id == 0)
            RUNNER_REGS_0_CFG_MAIN_PROFILING_CFG_WRITE(runner_profiling_cfg_register);
        else
            RUNNER_REGS_1_CFG_MAIN_PROFILING_CFG_WRITE(runner_profiling_cfg_register);
    else
        if (runner_id == 0)
            RUNNER_REGS_0_CFG_PICO_PROFILING_CFG_WRITE(runner_profiling_cfg_register);
        else
            RUNNER_REGS_1_CFG_PICO_PROFILING_CFG_WRITE(runner_profiling_cfg_register);

    return 0;
}

int p_rdd_print_profiling_registers(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_MAIN_PROFILING_STS profiling_status;
    RUNNER_REGS_CFG_MAIN_STALL_CNT1 stall_counter1;
    RUNNER_REGS_CFG_MAIN_STALL_CNT2 stall_counter2;
    RUNNER_REGS_CFG_MAIN_TASK1_CNT task1_counter;
    RUNNER_REGS_CFG_MAIN_TASK2_CNT task2_counter;
    RUNNER_REGS_CFG_MAIN_IDLE_CNT1 idle_cnt;
    RUNNER_REGS_CFG_MAIN_JMP_CNT jmp_counter;

    /* Print profiling registers for all runners */
    RUNNER_REGS_0_CFG_MAIN_PROFILING_STS_READ(profiling_status);
    RUNNER_REGS_0_CFG_MAIN_STALL_CNT1_READ(stall_counter1);
    RUNNER_REGS_0_CFG_MAIN_STALL_CNT2_READ(stall_counter2);
    RUNNER_REGS_0_CFG_MAIN_TASK1_CNT_READ(task1_counter);
    RUNNER_REGS_0_CFG_MAIN_TASK2_CNT_READ(task2_counter);
    RUNNER_REGS_0_CFG_MAIN_IDLE_CNT1_READ(idle_cnt);
    RUNNER_REGS_0_CFG_MAIN_JMP_CNT_READ(jmp_counter);

    bdmf_session_print(session, "Main Runner A :\n");
    bdmf_session_print(session, "Next PC                    : 0x%04x\n", (uint32_t)(profiling_status.agu_next_pc << 3));
    bdmf_session_print(session, "Current thread             : %u\n", (uint32_t)profiling_status.curr_thread_num);
    bdmf_session_print(session, "Idle                       : %u\n", (uint32_t)profiling_status.idle_no_active_task);
    bdmf_session_print(session, "Task 1 stall count - ACC   : %u\n", (uint32_t)stall_counter1.acc_stall_cnt);
    bdmf_session_print(session, "Task 1 stall count - LD    : %u\n", (uint32_t)stall_counter1.ld_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - STORE : %u\n", (uint32_t)stall_counter2.store_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - LDIO  : %u\n", (uint32_t)stall_counter2.ldio_stall_cnt);
    bdmf_session_print(session, "Task 1 task count          : %u\n", (uint32_t)task1_counter.task1_cnt);
    bdmf_session_print(session, "Task 2 task count          : %u\n", (uint32_t)task2_counter.task2_cnt);
    bdmf_session_print(session, "Idle count                 : %u\n", (uint32_t)idle_cnt.idle_cnt);
    bdmf_session_print(session, "Mispredicted taken jumps   : %u\n", (uint32_t)jmp_counter.taken_jmp_cnt);
    bdmf_session_print(session, "Mispredicted untaken jumps : %u\n", (uint32_t)jmp_counter.untaken_jmp_cnt);

    RUNNER_REGS_0_CFG_PICO_PROFILING_STS_READ(profiling_status);
    RUNNER_REGS_0_CFG_PICO_STALL_CNT1_READ(stall_counter1);
    RUNNER_REGS_0_CFG_PICO_STALL_CNT2_READ(stall_counter2);
    RUNNER_REGS_0_CFG_PICO_TASK_CNT1_READ(task1_counter);
    RUNNER_REGS_0_CFG_PICO_TASK_CNT2_READ(task2_counter);
    RUNNER_REGS_0_CFG_PICO_IDLE_CNT1_READ(idle_cnt);
    RUNNER_REGS_0_CFG_PICO_JMP_CNT_READ(jmp_counter);

    bdmf_session_print(session, "Pico Runner A :\n");
    bdmf_session_print(session, "Next PC                    : 0x%04x\n", (uint32_t)(profiling_status.agu_next_pc << 3));
    bdmf_session_print(session, "Current thread             : %u\n", (uint32_t)profiling_status.curr_thread_num);
    bdmf_session_print(session, "Idle                       : %u\n", (uint32_t)profiling_status.idle_no_active_task);
    bdmf_session_print(session, "Task 1 stall count - ACC   : %u\n", (uint32_t)stall_counter1.acc_stall_cnt);
    bdmf_session_print(session, "Task 1 stall count - LD    : %u\n", (uint32_t)stall_counter1.ld_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - STORE : %u\n", (uint32_t)stall_counter2.store_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - LDIO  : %u\n", (uint32_t)stall_counter2.ldio_stall_cnt);
    bdmf_session_print(session, "Task 1 task count          : %u\n", (uint32_t)task1_counter.task1_cnt);
    bdmf_session_print(session, "Task 2 task count          : %u\n", (uint32_t)task2_counter.task2_cnt);
    bdmf_session_print(session, "Idle count                 : %u\n", (uint32_t)idle_cnt.idle_cnt);
    bdmf_session_print(session, "Mispredicted taken jumps   : %u\n", (uint32_t)jmp_counter.taken_jmp_cnt);
    bdmf_session_print(session, "Mispredicted untaken jumps : %u\n", (uint32_t)jmp_counter.untaken_jmp_cnt);

    RUNNER_REGS_1_CFG_MAIN_PROFILING_STS_READ(profiling_status);
    RUNNER_REGS_1_CFG_MAIN_STALL_CNT1_READ(stall_counter1);
    RUNNER_REGS_1_CFG_MAIN_STALL_CNT2_READ(stall_counter2);
    RUNNER_REGS_1_CFG_MAIN_TASK1_CNT_READ(task1_counter);
    RUNNER_REGS_1_CFG_MAIN_TASK2_CNT_READ(task2_counter);
    RUNNER_REGS_1_CFG_MAIN_IDLE_CNT1_READ(idle_cnt);
    RUNNER_REGS_1_CFG_MAIN_JMP_CNT_READ(jmp_counter);

    bdmf_session_print(session, "Main Runner B :\n");
    bdmf_session_print(session, "Next PC                    : 0x%04x\n", (uint32_t)(profiling_status.agu_next_pc << 3));
    bdmf_session_print(session, "Current thread             : %u\n", (uint32_t)profiling_status.curr_thread_num);
    bdmf_session_print(session, "Idle                       : %u\n", (uint32_t)profiling_status.idle_no_active_task);
    bdmf_session_print(session, "Task 1 stall count - ACC   : %u\n", (uint32_t)stall_counter1.acc_stall_cnt);
    bdmf_session_print(session, "Task 1 stall count - LD    : %u\n", (uint32_t)stall_counter1.ld_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - STORE : %u\n", (uint32_t)stall_counter2.store_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - LDIO  : %u\n", (uint32_t)stall_counter2.ldio_stall_cnt);
    bdmf_session_print(session, "Task 1 task count          : %u\n", (uint32_t)task1_counter.task1_cnt);
    bdmf_session_print(session, "Task 2 task count          : %u\n", (uint32_t)task2_counter.task2_cnt);
    bdmf_session_print(session, "Idle count                 : %u\n", (uint32_t)idle_cnt.idle_cnt);
    bdmf_session_print(session, "Mispredicted taken jumps   : %u\n", (uint32_t)jmp_counter.taken_jmp_cnt);
    bdmf_session_print(session, "Mispredicted untaken jumps : %u\n", (uint32_t)jmp_counter.untaken_jmp_cnt);


    RUNNER_REGS_1_CFG_PICO_PROFILING_STS_READ(profiling_status);
    RUNNER_REGS_1_CFG_PICO_STALL_CNT1_READ(stall_counter1);
    RUNNER_REGS_1_CFG_PICO_STALL_CNT2_READ(stall_counter2);
    RUNNER_REGS_1_CFG_PICO_TASK_CNT1_READ(task1_counter);
    RUNNER_REGS_1_CFG_PICO_TASK_CNT2_READ(task2_counter);
    RUNNER_REGS_1_CFG_PICO_IDLE_CNT1_READ(idle_cnt);
    RUNNER_REGS_1_CFG_PICO_JMP_CNT_READ(jmp_counter);

    bdmf_session_print(session, "Pico Runner B :\n");
    bdmf_session_print(session, "Next PC                    : 0x%04x\n", (uint32_t)(profiling_status.agu_next_pc << 3));
    bdmf_session_print(session, "Current thread             : %u\n", (uint32_t)profiling_status.curr_thread_num);
    bdmf_session_print(session, "Idle                       : %u\n", (uint32_t)profiling_status.idle_no_active_task);
    bdmf_session_print(session, "Task 1 stall count - ACC   : %u\n", (uint32_t)stall_counter1.acc_stall_cnt);
    bdmf_session_print(session, "Task 1 stall count - LD    : %u\n", (uint32_t)stall_counter1.ld_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - STORE : %u\n", (uint32_t)stall_counter2.store_stall_cnt);
    bdmf_session_print(session, "Task 2 stall count - LDIO  : %u\n", (uint32_t)stall_counter2.ldio_stall_cnt);
    bdmf_session_print(session, "Task 1 task count          : %u\n", (uint32_t)task1_counter.task1_cnt);
    bdmf_session_print(session, "Task 2 task count          : %u\n", (uint32_t)task2_counter.task2_cnt);
    bdmf_session_print(session, "Idle count                 : %u\n", (uint32_t)idle_cnt.idle_cnt);
    bdmf_session_print(session, "Mispredicted taken jumps   : %u\n", (uint32_t)jmp_counter.taken_jmp_cnt);
    bdmf_session_print(session, "Mispredicted untaken jumps : %u\n", (uint32_t)jmp_counter.untaken_jmp_cnt);

    return 0;
}
#endif /*!(defined(WL4908) && defined(BDMF_SYSTEM_SIM))*/

#if !defined(FIRMWARE_INIT)
int p_rdd_runner_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_GLOBAL_CTRL runner_global_ctrl_register;
    uint32_t runner_id;
    uint32_t is_pico;
    uint32_t runner_mode;

    runner_id = (uint32_t)parm[0].value.unumber;
    is_pico = (uint32_t)parm[1].value.unumber;
    runner_mode = (uint32_t)parm[2].value.unumber;

    RUNNER_REGS_CFG_GLOBAL_CTRL_READ(runner_id, runner_global_ctrl_register);

    /* Update runner enabled register field (main or pico) */
    if (is_pico == 0)
        runner_global_ctrl_register.main_en = runner_mode;
    else
        runner_global_ctrl_register.pico_en = runner_mode;

    RUNNER_REGS_CFG_GLOBAL_CTRL_WRITE(runner_id, runner_global_ctrl_register);

    return 0;
}

int p_rdd_breakpoint_config(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_MAIN_BKPT_CFG bkpt_cfg_register;
    uint32_t runner_id;
    uint32_t is_pico;
    uint32_t step_mode;

    runner_id = (uint32_t)parm[0].value.unumber;
    is_pico = (uint32_t)parm[1].value.unumber;
    step_mode = (uint32_t)parm[2].value.unumber;

    bkpt_cfg_register.step_mode = step_mode;
    bkpt_cfg_register.rsv = 0x0;
    bkpt_cfg_register.new_pc_val = 0x0;
    bkpt_cfg_register.new_flags_val = 0x0;

    if (runner_id)
        bkpt_cfg_register.handler_addr = (is_pico ? ADDRESS_OF(runner_d, debug_routine) : ADDRESS_OF(runner_b, debug_routine)) >> 2;
    else
        bkpt_cfg_register.handler_addr = (is_pico ? ADDRESS_OF(runner_c, debug_routine) : ADDRESS_OF(runner_a, debug_routine)) >> 2;

    /* Update breakpoint cfg register (main or pico) */
    if (is_pico == 0)
        RUNNER_REGS_CFG_MAIN_BKPT_CFG_WRITE(runner_id, bkpt_cfg_register);
    else
        RUNNER_REGS_CFG_PICO_BKPT_CFG_WRITE(runner_id, bkpt_cfg_register);

    return 0;
}

int p_rdd_set_breakpoint(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_MAIN_BKPT_0 bkpt_cfg_register;
    uint32_t runner_id;
    uint32_t is_pico;
    uint32_t breakpoint_id;
    uint32_t breakpoint_enable;
    uint32_t breakpoint_address;
    uint32_t use_thread;
    uint32_t thread_id;

    runner_id = (uint32_t)parm[0].value.unumber;
    is_pico = (uint32_t)parm[1].value.unumber;
    breakpoint_id = (uint32_t)parm[2].value.unumber;
    breakpoint_enable = (uint32_t)parm[3].value.unumber;
    breakpoint_address = (uint32_t)parm[4].value.unumber;
    use_thread = (uint32_t)parm[5].value.unumber;
    thread_id = (uint32_t)parm[6].value.unumber;

    if (thread_id > 31 || (thread_id > 15 && is_pico == 1))
    {
        bdmf_session_print(session, "UT: Invalid parameter: thread\n\n\r");
        return BDMF_ERR_PARM;
    }

    /* Fill breakpoint register fields */
    bkpt_cfg_register.addr = breakpoint_address >> 2;
    bkpt_cfg_register.enable = breakpoint_enable;
    bkpt_cfg_register.rsv = 0x0;
    bkpt_cfg_register.thread = thread_id;
    bkpt_cfg_register.use_thread = use_thread;

    /* Read breakpoint register */
    switch (breakpoint_id)
    {
    case 0:
        if (is_pico == 0)
            RUNNER_REGS_CFG_MAIN_BKPT_0_WRITE(runner_id, bkpt_cfg_register);
        else
            RUNNER_REGS_CFG_PICO_BKPT_0_WRITE(runner_id, bkpt_cfg_register);
        break;
    case 1:
        if (is_pico == 0)
            RUNNER_REGS_CFG_MAIN_BKPT_1_WRITE(runner_id, bkpt_cfg_register);
        else
            RUNNER_REGS_CFG_PICO_BKPT_1_WRITE(runner_id, bkpt_cfg_register);
        break;
    case 2:
        if (is_pico == 0)
            RUNNER_REGS_CFG_MAIN_BKPT_2_WRITE(runner_id, bkpt_cfg_register);
        else
            RUNNER_REGS_CFG_PICO_BKPT_2_WRITE(runner_id, bkpt_cfg_register);

        break;
    case 3:
        if (is_pico == 0)
            RUNNER_REGS_CFG_MAIN_BKPT_3_WRITE(runner_id, bkpt_cfg_register);
        else
            RUNNER_REGS_CFG_PICO_BKPT_3_WRITE(runner_id, bkpt_cfg_register);

        break;
    default:
        bdmf_session_print(session, "UT: Invalid parameter\n\n\r");
        return BDMF_ERR_PARM;
    }

    return 0;
}

int p_rdd_print_breakpoint_status(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RUNNER_REGS_CFG_MAIN_BKPT_STS bkpt_sts_register;
    uint32_t runner_id;
    uint32_t is_pico;
    uint32_t i;
    uint32_t *debug_buffer_ptr;
    uint32_t *peripheral_register_ptr;
    uint32_t register_value;
    uint32_t current_thread_number;

    runner_id  = (uint32_t)parm[0].value.unumber;
    is_pico = (uint32_t)parm[1].value.unumber;

    /* Update breakpoint cfg register (main or pico) */
    if (is_pico == 0)
        RUNNER_REGS_CFG_MAIN_BKPT_STS_READ(runner_id, bkpt_sts_register);
    else
        RUNNER_REGS_CFG_PICO_BKPT_STS_READ(runner_id, bkpt_sts_register);

    if (bkpt_sts_register.active == 1)
        bdmf_session_print(session, "Runner %u %s Breakpoint active\n", (uint32_t)runner_id, (is_pico == 0) ? "MAIN" : "PICO");
    else
        bdmf_session_print(session, "Runner %u %s Breakpoint not active\n", (uint32_t)runner_id, (is_pico == 0) ? "MAIN" : "PICO");

    if (runner_id == 0)
    {
        debug_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_DEBUG_BUFFER_ADDRESS);
        peripheral_register_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_DEBUG_PERIPHERALS_STATUS_REGISTER_ADDRESS);
    }
    else
    {
        debug_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_DEBUG_BUFFER_ADDRESS);
        peripheral_register_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_DEBUG_PERIPHERALS_STATUS_REGISTER_ADDRESS);
    }

    MREAD_8(((uint8_t *)peripheral_register_ptr + 1), current_thread_number);
    current_thread_number = current_thread_number & 31;

    bdmf_session_print(session, "----------------\n");
    bdmf_session_print(session, "Current thread number: 0x%-8x\n\n", (uint32_t)current_thread_number);
    bdmf_session_print(session, "Breakpoint address:  0x0%-4x\n\n", bkpt_sts_register.bkpt_addr << 2);

    for (i = 0; i < 32; i++)
    {
        MREAD_32(debug_buffer_ptr + i, register_value);
        bdmf_session_print(session, "register %2u = 0x%-8x\n", (uint32_t)i, (uint32_t)register_value);
    }

    bdmf_session_print(session, "\n");

    return 0;
}
#endif /* !defined(FIRMWARE_INIT) */

extern unsigned int SEGMENTS_ADDRESSES[];
#if !(defined(WL4908) && defined(BDMF_SYSTEM_SIM))
static char *seg_names[] = { "Private A", "Private B", "Common A", "Common B", "DDR", "PSRAM" };
extern TABLE_STRUCT RUNNER_TABLES[];

static uint8_t* table_addr_get(TABLE_STRUCT *tbl, int entry_idx)
{
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint8_t *addr;
    uint32_t addr_offset;

    tbl_ctx = tbl->entries;
    addr_offset = SEGMENTS_ADDRESSES[tbl->segment] + tbl_ctx->entries[entry_idx].starts;

    if (tbl->segment == DDR_INDEX)
        addr = g_runner_ddr_base_addr + addr_offset;
    else
        addr =  (uint8_t*)DEVICE_ADDRESS(addr_offset);

    return addr;
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

int32_t p_rdd_print_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint32_t i, j, n;
    char parm_val[256];

    if (n_parms)
    {
        strcpy(parm_val, parm[0].value.string);
        bdmf_session_print(session, "Param %s, parm_val %s\n", parm[0].value.string, parm_val);
        str_toupper(parm_val);
        bdmf_session_print(session, "parm_val after convert %s\n", parm_val);
    }

    bdmf_session_print(session, "List of Tables\n\n");
    bdmf_session_print(session, "%70s %8s %15s %12s %12s %12s\n", "Table Name", "Address", "Segment", "Entry Len", "Entry Types", "Size");
    bdmf_session_print(session, "---------------------------------------------------------------------");
    bdmf_session_print(session, "---------------------------------------------------------------------\n");

    for (i = 0, n = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];

        if (n_parms)
        {
            /* Check if table name complies the pattern. If it doesn't, skip */
            if (!strstr(tbl->table_name, parm_val))
                continue;
        }
        tbl_ctx = tbl->entries;

        /* If this is a union, calc how many entry representations we have */
        for (j = 0; tbl_ctx->entries[j].callback; j++)
        {}
        bdmf_session_print(session, "%70s 0x%p %15s %10d %10d          [%d]",
            tbl->table_name, (void *)table_addr_get(tbl, 0), seg_names[tbl->segment], tbl_ctx->length, j, tbl->size_rows);
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

int32_t p_rdd_print_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    uint32_t i, j;
    char parm_val[256];
    uint32_t start_entry, num_of_entries, is_addr;
    uint8_t *entry_addr, *tbl_addr;

    strcpy(parm_val, parm[0].value.string);
    str_toupper(parm_val);

    for (i = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];

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

    is_addr = parm[1].value.unumber;

    if (is_addr)
    {
#if defined(__LP64__) || defined(_LP64)
        entry_addr = (uint8_t*)(uintptr_t)parm[3].value.unumber64;
#else
        entry_addr = (uint8_t*)parm[3].value.unumber;
#endif
        start_entry = (entry_addr - tbl_addr) / tbl_ctx->length;
    }
    else
    {
        start_entry = parm[2].value.unumber;
        entry_addr = tbl_addr + start_entry * tbl_ctx->length;
    }

    num_of_entries = parm[4].value.unumber;

    for (i = 0; i < num_of_entries; i++, entry_addr += tbl_ctx->length)
    {
        bdmf_session_print(session, "Index %d, addr 0x%p, size %d, value:\n", start_entry + i, (void *)entry_addr, tbl_ctx->length);
        bdmf_session_hexdump(session, (unsigned char *)entry_addr, 0, tbl_ctx->length);
        bdmf_session_print(session, "\n");

        /* It's possible that we have a union of entries. In this case, we want to print all possible transformations. */
        for (j = 0; tbl_ctx->entries[j].callback; j++)
        {
            tbl_ctx->entries[j].callback(session, (unsigned char *)entry_addr);
            bdmf_session_print(session, "\n");
        }
    }
    bdmf_session_print(session, "\n\n");
    return 0;
}
#endif /*!(defined(WL4908) && defined(BDMF_SYSTEM_SIM))*/

#ifndef BDMF_SYSTEM_SIM
int p_rdd_fwtrace_enable ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint32_t                            enable;

    enable = ( uint32_t )parm[ 0 ].value.unumber;
    
    if ( enable >= 2 )
    {
        bdmf_session_print ( session, "Invalid enable parameter" );
        return ( BDMF_ERR_PARM );
    }

    f_rdd_fwtrace_enable_set(enable);

    return ( 0 );
}

int p_rdd_fwtrace_clear ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    f_rdd_fwtrace_clear();

    return ( 0 );
}

int p_rdd_fwtrace_print ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
#ifdef RUNNER_FWTRACE
    uint32_t i;
    uint32_t threadNum;
    uint32_t eventNum;
    uint32_t timeInNs;
    uint16_t trace_length;
    volatile uint32_t *fwtrace_buf_ptr;
    volatile uint32_t *fwtrace_offset_ptr;
    uint32_t bEventStrings;

    bEventStrings = ( uint32_t )parm[ 0 ].value.unumber;
    
    if ( bEventStrings >= 2 )
    {
        bdmf_session_print ( session, "Invalid event strings parameter.  Must be 1 or 0" );
        return ( BDMF_ERR_PARM );
    }
  
    fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET) + RUNNER_FWTRACE_MAINA_BASE_ADDRESS);
    fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS );
    trace_length = ntohs(*fwtrace_offset_ptr) & 0x7FFF;
    bdmf_session_print (  session, "MainA-TimeNs Thread EventType length 0x%x, read 0x%x:\n", trace_length, ntohs(*fwtrace_offset_ptr));
    for (i=0;i<trace_length;i++)
    {
        // Convert time to ns from 20ns increments
        timeInNs = RDD_FWTRACE_READ_TIME_CNTR(fwtrace_buf_ptr) * TIMER_PERIOD_NS;
        eventNum = RDD_FWTRACE_READ_EVENT( fwtrace_buf_ptr );
        threadNum = RDD_FWTRACE_READ_THREAD( fwtrace_buf_ptr );

        if (bEventStrings)
        {
            bdmf_session_print (  session, "%u %s: %s\n", timeInNs, RnrATaskNames[threadNum], rdpFwTraceEvents[eventNum]);
        }
        else
        {
            bdmf_session_print (  session, "%u %s: %d\n", timeInNs, RnrATaskNames[threadNum], eventNum);
        }
        
#ifdef RUNNER_FWTRACE_32BIT
        i++;
#endif
    }        
    
    fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET) + RUNNER_FWTRACE_PICOA_BASE_ADDRESS);
    fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RUNNER_FWTRACE_PICOA_CURR_OFFSET_ADDRESS );
    trace_length = ntohs(*fwtrace_offset_ptr) & 0x7FFF;
    bdmf_session_print (  session, "PicoA-TimeNs Thread EventType length 0x%x, read 0x%x:\n", trace_length, ntohs(*fwtrace_offset_ptr));
    for (i=0;i<trace_length;i++)
    {
        // Convert time to ns from 20ns increments
        timeInNs = RDD_FWTRACE_READ_TIME_CNTR(fwtrace_buf_ptr) * TIMER_PERIOD_NS;
        eventNum = RDD_FWTRACE_READ_EVENT( fwtrace_buf_ptr );
        threadNum = RDD_FWTRACE_READ_THREAD( fwtrace_buf_ptr );
#ifdef FWTRACE_READ_TASKID
        threadNum +=32;
#endif
        if (bEventStrings)
        {
            bdmf_session_print (  session, "%u %s: %s\n", timeInNs, RnrATaskNames[threadNum], rdpFwTraceEvents[eventNum]);
        }
        else
        {
            bdmf_session_print (  session, "%u %s: %d\n", timeInNs, RnrATaskNames[threadNum], eventNum);
        }
#ifdef RUNNER_FWTRACE_32BIT
        i++;
#endif
    }        

    fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET) + RUNNER_FWTRACE_MAINB_BASE_ADDRESS);
    fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS );
    trace_length = ntohs(*fwtrace_offset_ptr) & 0x7FFF;
    bdmf_session_print (  session, "MainB-TimeNs Thread EventType length 0x%x, read 0x%x:\n", trace_length, ntohs(*fwtrace_offset_ptr));
    for (i=0;i<trace_length;i++)
    {
        // Convert time to ns from 20ns increments
        timeInNs = RDD_FWTRACE_READ_TIME_CNTR(fwtrace_buf_ptr) * TIMER_PERIOD_NS;
        eventNum = RDD_FWTRACE_READ_EVENT( fwtrace_buf_ptr );
        threadNum = RDD_FWTRACE_READ_THREAD( fwtrace_buf_ptr );

        if (bEventStrings)
        {
            bdmf_session_print (  session, "%u %s: %s\n", timeInNs, RnrBTaskNames[threadNum], rdpFwTraceEvents[eventNum]);
        }
        else
        {
            bdmf_session_print (  session, "%u %s: %d\n", timeInNs, RnrBTaskNames[threadNum], eventNum);
        }
#ifdef RUNNER_FWTRACE_32BIT
        i++;
#endif

    }        

    fwtrace_buf_ptr = (volatile uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET) + RUNNER_FWTRACE_PICOB_BASE_ADDRESS);
    fwtrace_offset_ptr = ( volatile uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + RUNNER_FWTRACE_PICOB_CURR_OFFSET_ADDRESS );
    trace_length = ntohs(*fwtrace_offset_ptr) & 0x7FFF;
    bdmf_session_print (  session, "PicoB-TimeNs Thread EventType length 0x%x, read 0x%x:\n", trace_length, ntohs(*fwtrace_offset_ptr));
    for (i=0;i<trace_length;i++)
    {
        // Convert time to ns from 20ns increments
        timeInNs = RDD_FWTRACE_READ_TIME_CNTR(fwtrace_buf_ptr) * TIMER_PERIOD_NS;
        eventNum = RDD_FWTRACE_READ_EVENT( fwtrace_buf_ptr );
        threadNum = RDD_FWTRACE_READ_THREAD( fwtrace_buf_ptr );

#ifdef FWTRACE_READ_TASKID
        threadNum +=32;
#endif
        if (bEventStrings)
        {
            bdmf_session_print (  session, "%u %s: %s\n", timeInNs, RnrBTaskNames[threadNum], rdpFwTraceEvents[eventNum]);
        }
        else
        {
            bdmf_session_print (  session, "%u %s: %d\n", timeInNs, RnrBTaskNames[threadNum], eventNum);
        }
#ifdef RUNNER_FWTRACE_32BIT
        i++;
#endif
    } 
#endif

    return ( 0 );
}
#endif /*!BDMF_SYSTEM_SIM*/

