// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Broadcom
 */
/*
    
*/

#ifndef _RDD_RUNNER_PROJ_DEFS_H
#define _RDD_RUNNER_PROJ_DEFS_H

#define NUM_OF_GLOBAL_REGS                                      0
#define NUM_OF_LOCAL_REGS                                       32
#define NUM_OF_MAIN_RUNNER_THREADS                              16
#define NUM_OF_RUNNER_CORES                                     3

/* TIMER DEFINES */
#define FLUSH_TASK_TIMER_INTERVAL_IN_USEC                       15
#define RATE_LIMITER_TIMER_PERIOD_IN_USEC                       1000
#define CPU_RX_METER_TIMER_PERIOD                               10000
#define CPU_RX_METER_TIMER_PERIOD_IN_USEC                       (CPU_RX_METER_TIMER_PERIOD)
#define FLUSH_TASK_TIMER_INTERVAL                               (FLUSH_TASK_TIMER_INTERVAL_IN_USEC)
#define RATE_LIMITER_TIMER_PERIOD                               (RATE_LIMITER_TIMER_PERIOD_IN_USEC)

/* NATC */
#define NATC_16BYTE_KEY_MASK                                    0xc000

#define RDD_WAN0_VPORT       RDD_VPORT_ID_0
#define RDD_LAN0_VPORT       RDD_VPORT_ID_1
#define RDD_LAN1_VPORT       RDD_VPORT_ID_2
#define RDD_LAN2_VPORT       RDD_VPORT_ID_3
#define RDD_LAN3_VPORT       RDD_VPORT_ID_4
#define RDD_LAN4_VPORT       RDD_VPORT_ID_5
#define RDD_LAN5_VPORT       RDD_VPORT_ID_6
#define RDD_LAN6_VPORT       RDD_VPORT_ID_7
#define RDD_LAN_VPORT_LAST   RDD_LAN6_VPORT

#define RDD_CPU0_VPORT       (RDD_LAN_VPORT_LAST + 1)
#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT

#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT
#define RDD_CPU1_VPORT       (RDD_CPU_VPORT_FIRST + 1)
#define RDD_CPU2_VPORT       (RDD_CPU_VPORT_FIRST + 2)
#define RDD_CPU3_VPORT       (RDD_CPU_VPORT_FIRST + 3)
#define RDD_CPU4_VPORT       (RDD_CPU_VPORT_FIRST + 4)
#define RDD_WLAN0_VPORT      RDD_CPU4_VPORT
#define RDD_CPU5_VPORT       (RDD_CPU_VPORT_FIRST + 5)
#define RDD_WLAN1_VPORT      RDD_CPU5_VPORT
#define RDD_CPU6_VPORT       (RDD_CPU_VPORT_FIRST + 6)
#define RDD_WLAN2_VPORT      RDD_CPU6_VPORT
#define RDD_CPU_VPORT_FLOOD  (RDD_CPU_VPORT_FIRST + 7)
#define RDD_CPU_VPORT_LAST   RDD_CPU_VPORT_FLOOD
#define RDD_CPU_VPORT_MASK ((1 << RDD_CPU0_VPORT) | (1 << RDD_CPU1_VPORT) | (1 << RDD_CPU2_VPORT) | \
    (1 << RDD_CPU3_VPORT) | (1 << RDD_CPU4_VPORT) | (1 << RDD_CPU5_VPORT) | (1 << RDD_CPU6_VPORT) | \
    (1 << RDD_CPU_VPORT_FLOOD))



#endif
