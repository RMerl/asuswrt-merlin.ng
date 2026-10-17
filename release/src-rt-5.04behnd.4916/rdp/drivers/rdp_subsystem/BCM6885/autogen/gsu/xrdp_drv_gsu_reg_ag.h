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


#ifndef _XRDP_DRV_GSU_REG_AG_H_
#define _XRDP_DRV_GSU_REG_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean dont_transmit_last_pd;
    bdmf_boolean rl_count_dithering_en;
    uint8_t cfg_pck_overhead_bytes_a;
    uint8_t cfg_pck_overhead_bytes_b;
    bdmf_boolean use_extended_memory;
    bdmf_boolean cfg_send_ack_to_rnr;
    uint8_t task_num_for_rnr_ack;
    bdmf_boolean cfg_send_nack_to_bbhtx;
} gsu_reg_misc_cfg;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} gsu_reg_clk_gate_cntrl;


/**********************************************************************************************************************
 * downcnt_value: 
 *     Upon write, load value into downcounter and start profiling window.
 *     Read returns current value of the downcounter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_profiling_downcnt_set(uint8_t gsu_reg_id, uint32_t downcnt_value);
bdmf_error_t ag_drv_gsu_reg_profiling_downcnt_get(uint8_t gsu_reg_id, uint32_t *downcnt_value);

/**********************************************************************************************************************
 * value: 
 *     Profiling counter A
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_profiling_counter_a_get(uint8_t gsu_reg_id, uint32_t *value);

/**********************************************************************************************************************
 * value: 
 *     Profiling counter B
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_profiling_counter_b_get(uint8_t gsu_reg_id, uint32_t *value);

/**********************************************************************************************************************
 * counter_a_mode: 
 *     Mode for counter A
 * counter_b_mode: 
 *     Mode for counter B
 * command_id: 
 *     Command to measure
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_profiling_mode_set(uint8_t gsu_reg_id, bdmf_boolean counter_a_mode, uint8_t counter_b_mode, uint8_t command_id);
bdmf_error_t ag_drv_gsu_reg_profiling_mode_get(uint8_t gsu_reg_id, bdmf_boolean *counter_a_mode, uint8_t *counter_b_mode, uint8_t *command_id);

/**********************************************************************************************************************
 * qm_0_base_queue: 
 *     Base for QM0, for converting absolute queue ID from QM 0 to local queue ID.
 * qm_1_base_queue: 
 *     Base for QM1, for converting absolute queue ID from QM 1 to local queue ID.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_qm_cfg_1_set(uint8_t gsu_reg_id, uint16_t qm_0_base_queue, uint16_t qm_1_base_queue);
bdmf_error_t ag_drv_gsu_reg_qm_cfg_1_get(uint8_t gsu_reg_id, uint16_t *qm_0_base_queue, uint16_t *qm_1_base_queue);

/**********************************************************************************************************************
 * pd_fifo_qm0: 
 *     Pointer to PDFIFO location where QM0 related queues are located
 * pd_fifo_qm1: 
 *     Pointer to PDFIFO location where QM1 related queues are located
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_qm_cfg_2_set(uint8_t gsu_reg_id, uint16_t pd_fifo_qm0, uint16_t pd_fifo_qm1);
bdmf_error_t ag_drv_gsu_reg_qm_cfg_2_get(uint8_t gsu_reg_id, uint16_t *pd_fifo_qm0, uint16_t *pd_fifo_qm1);

/**********************************************************************************************************************
 * num_of_qm_0_queues: 
 *     Hpw many queues are in QM0 - serves for calculationg local Queue ID.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_qm_cfg_3_set(uint8_t gsu_reg_id, uint16_t num_of_qm_0_queues);
bdmf_error_t ag_drv_gsu_reg_qm_cfg_3_get(uint8_t gsu_reg_id, uint16_t *num_of_qm_0_queues);

/**********************************************************************************************************************
 * pd_return_addr: 
 *     Address for returning PDs to Runner memory (byte resolution)
 * status_return_addr: 
 *     Address for returning status to Runner memory (byte resolution)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_transmit_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t pd_return_addr, uint16_t status_return_addr);
bdmf_error_t ag_drv_gsu_reg_transmit_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *pd_return_addr, uint16_t *status_return_addr);

/**********************************************************************************************************************
 * forced_incr_rl_status_addr: 
 *     Address in Runner memory for returning status for RL force increment command
 * periodic_rl_status_addr: 
 *     Address in Runner memory for returning status for RL periodic update command
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_rl_status_cfg_set(uint8_t gsu_reg_id, uint16_t forced_incr_rl_status_addr, uint16_t periodic_rl_status_addr);
bdmf_error_t ag_drv_gsu_reg_rl_status_cfg_get(uint8_t gsu_reg_id, uint16_t *forced_incr_rl_status_addr, uint16_t *periodic_rl_status_addr);

/**********************************************************************************************************************
 * fecth_rl_base_addr: 
 *     Address base for returningresponse to fetchRLcounters command to Runner memory (byte resolution). Target
 *     address will be calculated according to the configured base address and the index passed as an argument in the
 *     command.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_rl_fetch_cfg_set(uint8_t gsu_reg_id, uint16_t fecth_rl_base_addr);
bdmf_error_t ag_drv_gsu_reg_rl_fetch_cfg_get(uint8_t gsu_reg_id, uint16_t *fecth_rl_base_addr);

/**********************************************************************************************************************
 * dont_transmit_last_pd: 
 *     If transmit starts when a queue is full and all transmitted packets come from this queue, dont transmit last
 *     packet. This feature prevents unwanted switching between unequal-weight DWRR inputs. Disabled by default
 * rl_count_dithering_en: 
 *     Enable dithering for RL counters
 * cfg_pck_overhead_bytes_a: 
 *     Configure number of overhead bytes for per-packet overhead feature in RL calculations - profile A
 * cfg_pck_overhead_bytes_b: 
 *     Configure number of overhead bytes for per-packet overhead feature in RL calculations - profile B
 * use_extended_memory: 
 *     GSU will use extended memory (its own and the memory of adjacent GSU)
 * cfg_send_ack_to_rnr: 
 *     If set to 1, send ACK message to Runner instead of BBH TX
 * task_num_for_rnr_ack: 
 *     Task number to wake up when returning ACK in TX status message to Runner - relevant only when
 *     CFG_SEND_ACK_TO_RNR=1
 * cfg_send_nack_to_bbhtx: 
 *     If set to 1, send NACK message to BBH TX instead of RUNNER
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_misc_cfg_set(uint8_t gsu_reg_id, const gsu_reg_misc_cfg *misc_cfg);
bdmf_error_t ag_drv_gsu_reg_misc_cfg_get(uint8_t gsu_reg_id, gsu_reg_misc_cfg *misc_cfg);

/**********************************************************************************************************************
 * pause: 
 *     Write 1 to pause GSU operation
 * step_mode: 
 *     Switch to step mode execution
 * do_single_command: 
 *     In step mode, do single command
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_pause_cfg_set(uint8_t gsu_reg_id, bdmf_boolean pause, bdmf_boolean step_mode, bdmf_boolean do_single_command);
bdmf_error_t ag_drv_gsu_reg_pause_cfg_get(uint8_t gsu_reg_id, bdmf_boolean *pause, bdmf_boolean *step_mode, bdmf_boolean *do_single_command);

/**********************************************************************************************************************
 * aqm_return_addr: 
 *     Address for returning PAQM timestamps to Runner memory (byte resolution)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_aqm_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t aqm_return_addr);
bdmf_error_t ag_drv_gsu_reg_aqm_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *aqm_return_addr);

/**********************************************************************************************************************
 * status_return_addr: 
 *     Address for returning status to Runner memory (byte resolution)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_flush_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t status_return_addr);
bdmf_error_t ag_drv_gsu_reg_flush_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *status_return_addr);

/**********************************************************************************************************************
 * last_valid_node: 
 *     Set last valid node for RL update. GSU will traverse node mask vector searching for eligible RL nodes until
 *     hitting the last node number - then it will stop and report next node 0 in response to Runner.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_rl_update_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t last_valid_node);
bdmf_error_t ag_drv_gsu_reg_rl_update_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *last_valid_node);

/**********************************************************************************************************************
 * last_valid_node: 
 *     Set last valid node for PFC reschedule. GSU will traverse node mask vector searching for eligible PFC nodes
 *     until hitting the last node number - then it will stop.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_pfc_modify_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t last_valid_node);
bdmf_error_t ag_drv_gsu_reg_pfc_modify_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *last_valid_node);

/**********************************************************************************************************************
 * pds_per_queue: 
 *     PD FIFO Size
 *     0 - 4  entries
 *     1 - 8  entries
 *     2 - 16 entries
 *     3 - 32 entries
 *     4 - 64 entries
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_pd_fifo_cfg_set(uint8_t gsu_reg_id, uint8_t pds_per_queue);
bdmf_error_t ag_drv_gsu_reg_pd_fifo_cfg_get(uint8_t gsu_reg_id, uint8_t *pds_per_queue);

/**********************************************************************************************************************
 * status_return_addr: 
 *     Address for returning status to Runner memory (byte resolution)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_pfc_status_cfg_set(uint8_t gsu_reg_id, uint16_t status_return_addr);
bdmf_error_t ag_drv_gsu_reg_pfc_status_cfg_get(uint8_t gsu_reg_id, uint16_t *status_return_addr);

/**********************************************************************************************************************
 * active: 
 *     Status bit indicating GSU is in active state
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_gsu_status_get(uint8_t gsu_reg_id, bdmf_boolean *active);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 *     TRIGGERS_HW
 *     
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     TRIGGERS_HW
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 *     TRIGGERS_HW
 *     
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 *     TRIGGERS_HW
 *     
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_clk_gate_cntrl_set(uint8_t gsu_reg_id, const gsu_reg_clk_gate_cntrl *clk_gate_cntrl);
bdmf_error_t ag_drv_gsu_reg_clk_gate_cntrl_get(uint8_t gsu_reg_id, gsu_reg_clk_gate_cntrl *clk_gate_cntrl);

/**********************************************************************************************************************
 * en_send_queue_id: 
 *     Enable/disable send PD QUEUE ID debug feature
 * queue_id_return_addr: 
 *     Address in Runner memory for returning queue IDs
 **********************************************************************************************************************/
bdmf_error_t ag_drv_gsu_reg_pd_queue_id_en_set(uint8_t gsu_reg_id, bdmf_boolean en_send_queue_id, uint16_t queue_id_return_addr);
bdmf_error_t ag_drv_gsu_reg_pd_queue_id_en_get(uint8_t gsu_reg_id, bdmf_boolean *en_send_queue_id, uint16_t *queue_id_return_addr);

#ifdef USE_BDMF_SHELL
enum
{
    cli_gsu_reg_profiling_downcnt,
    cli_gsu_reg_profiling_counter_a,
    cli_gsu_reg_profiling_counter_b,
    cli_gsu_reg_profiling_mode,
    cli_gsu_reg_qm_cfg_1,
    cli_gsu_reg_qm_cfg_2,
    cli_gsu_reg_qm_cfg_3,
    cli_gsu_reg_transmit_cmd_cfg,
    cli_gsu_reg_rl_status_cfg,
    cli_gsu_reg_rl_fetch_cfg,
    cli_gsu_reg_misc_cfg,
    cli_gsu_reg_pause_cfg,
    cli_gsu_reg_aqm_cmd_cfg,
    cli_gsu_reg_flush_cmd_cfg,
    cli_gsu_reg_rl_update_cmd_cfg,
    cli_gsu_reg_pfc_modify_cmd_cfg,
    cli_gsu_reg_pd_fifo_cfg,
    cli_gsu_reg_pfc_status_cfg,
    cli_gsu_reg_gsu_status,
    cli_gsu_reg_clk_gate_cntrl,
    cli_gsu_reg_pd_queue_id_en,
};

int bcm_gsu_reg_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_gsu_reg_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
