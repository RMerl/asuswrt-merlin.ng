/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
:>
*/
#ifndef RDD_SHELL_COMMON_H
#define RDD_SHELL_COMMON_H

#include "bdmf_shell.h"
#include "rdd.h"

int p_rdd_print_wan_bbh_rx_descriptors_normal(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_print_lan_bbh_rx_descriptors_normal(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_print_packet_buffer(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_start_profiling(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_stop_profiling(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_print_profiling_registers(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);

#if !defined(FIRMWARE_INIT)
int p_rdd_runner_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_breakpoint_config(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_set_breakpoint(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_print_breakpoint_status(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#endif

int p_rdd_fwtrace_enable( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
int p_rdd_fwtrace_clear ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
int p_rdd_fwtrace_print ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );

int p_rdd_print_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int p_rdd_print_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);

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


#endif /*RDD_SHELL_COMMON_H*/
