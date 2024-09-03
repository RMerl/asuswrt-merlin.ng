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
