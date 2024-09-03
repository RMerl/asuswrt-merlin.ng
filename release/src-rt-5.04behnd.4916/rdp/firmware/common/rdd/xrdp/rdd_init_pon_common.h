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
#if CHIP_VER >= RDP_GEN_50
void rdd_global_registers_init(uint32_t core_index, uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][NUM_OF_LOCAL_REGS], uint32_t last_thread);
#else
void rdd_global_registers_init(uint32_t core_index);
#endif
int rdd_cpu_proj_init(void);
void rdd_write_action(uint8_t core_index, uint16_t *action_arr, uint8_t size_of_array, uint8_t *ptr, uint8_t tbl_size);

#endif /*RDD_INIT_COMMON_H*/
