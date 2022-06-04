/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
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
