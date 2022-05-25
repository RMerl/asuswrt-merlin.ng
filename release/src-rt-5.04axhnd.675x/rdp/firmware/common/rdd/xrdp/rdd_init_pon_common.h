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
#ifndef RDD_INIT_COMMON_H
#define RDD_INIT_COMMON_H

#include "rdd.h"
#include "rdd_defs.h"
#include "rdd_init.h"

#include "rdd_platform.h"

#include "rdd_cpu.h"
#include "rdd_tuple_lkp.h"
#include "rdd_tcam_ic.h"
#include "rdd_ingress_filter.h"
#include "rdd_iptv.h"
#if !defined(RDP_UFC)
#include "rdd_bridge.h"
#endif
#include "rdp_drv_rnr.h"
#include "rdd_ag_natc.h"
#if (!defined(OPERATION_MODE_PRV) || defined(RULE_BASED_GRE))
#include "rdd_tunnels_parsing.h"
#endif

void rdd_proj_init(rdd_init_params_t *init_params);
int rdd_init(void);
void rdd_exit(void);
#if defined DUAL_ISSUE
void rdd_global_registers_init(uint32_t core_index, uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][NUM_OF_LOCAL_REGS], uint32_t last_thread);
#else
void rdd_global_registers_init(uint32_t core_index);
#endif
int rdd_cpu_proj_init(void);
void rdd_write_action(uint8_t core_index, uint16_t *action_arr, uint8_t size_of_array, uint8_t *ptr, uint8_t tbl_size);

#endif /*RDD_INIT_COMMON_H*/
