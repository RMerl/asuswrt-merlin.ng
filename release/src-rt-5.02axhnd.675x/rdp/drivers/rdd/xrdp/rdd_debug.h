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

#ifndef _RDD_DEBUG_H
#define _RDD_DEBUG_H

#include "rdpa_types.h"

struct debug_prints_info {
  void* debug_print_buf;
  uint32_t  debug_print_buf_len;       /* allocated print buffer length */
  uint32_t  double_buf_num;            /* incdicates current part of double buffer */
  uint32_t  priority;                  /* messages below priority are not displayed */
  uint32_t  perodicity_ms;             /* print function called every X msec */
  int32_t  num_of_messages_in_period;  /* number of messages to print per period. 0 - disable, -1 show_all */
};

extern struct debug_prints_info rdd_debug_prints_info;

void rdd_debug_init_runners_strings(void);
const char *rdd_debug_get_debug_string(int core_id, int ind, uint32_t params_num );
void rdd_debug_prints_init(void);
void rdd_debug_prints_update_params(int32_t max_prints_per_period, uint32_t period, uint32_t priority);
void rdd_debug_prints_handle(void);

#endif
