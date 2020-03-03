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
#include "rdd_defs.h"

int rdd_init(void);
void rdd_load_microcode(uint8_t *runner_a_microcode, uint8_t *runner_b_microcode, uint8_t *runner_c_microcode,
			uint8_t *runner_d_microcode);
void memcpyl_prediction(void *__to, void *__from, unsigned int __n);
void rdd_load_prediction(uint8_t *runner_a_predict, uint8_t *runner_b_predict, uint8_t *runner_c_predict,
			 uint8_t *runner_d_predict);
void rdd_load_microcode(uint8_t *runner_a_microcode, uint8_t *runner_b_microcode, uint8_t *runner_c_microcode,
			uint8_t *runner_d_microcode);
void rdd_runner_enable(void);
void rdd_runner_disable(void);
void rdd_runner_frequency_set(uint16_t freq);
void rdd_scheduler_init(void);
void rdd_local_registers_init(void);
void rdd_pm_counters_init(void);

#endif /*RDD_INIT_COMMON_H*/
