/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#ifndef _RDD_INIT_H
#define _RDD_INIT_H

typedef struct
{
    uint8_t *ddr0_runner_base_ptr;
    int is_basic;
} rdd_init_params_t;

int rdd_init(void);
void rdd_exit(void);

int rdd_data_structures_init(rdd_init_params_t *init_params);

#define DHD_TX_COMPLETE_0_THREAD_NUMBER   IMAGE_2_CPU_IF_2_DHD_TX_COMPLETE_0_THREAD_NUMBER
#define DHD_RX_COMPLETE_0_THREAD_NUMBER   IMAGE_2_CPU_IF_2_DHD_RX_COMPLETE_0_THREAD_NUMBER
#define DHD_TIMER_THREAD_NUMBER   IMAGE_1_CPU_IF_1_DHD_TIMER_THREAD_NUMBER

#endif /* _RDD_INIT_H */
