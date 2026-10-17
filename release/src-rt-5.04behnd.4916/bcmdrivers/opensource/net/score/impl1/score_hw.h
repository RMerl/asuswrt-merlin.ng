/*
   <:copyright-BRCM:2024:DUAL/GPL:standard

      Copyright (c) 2024 Broadcom
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

#ifndef ___SCORE_HW_H___
#define ___SCORE_HW_H___


typedef struct _score_channel {
    int rx_q; /* Mapping to HW queue id */
    wait_queue_head_t rxq_wqh; /*event queue for thread*/
    volatile unsigned long rxq_cond;
    struct task_struct *rx_thread;
} score_channel;

#define US1024_TO_MS(v)                 (((v)*1024)/1000)
#define US128_TO_MS(v)                  (((v)*128)/1000)
#define MS_TO_US128(v)                  (((v)*1000)>>7)

#if IS_ENABLED(CONFIG_BCM_RDPA)
#include "score_runner.h"
#define MSEC_UNIT_ADJUST(v)             US1024_TO_MS(v)
#define MSEC_UNIT_ADJUST_WLAN_NIC(v)    US128_TO_MS(v)
#elif IS_ENABLED(CONFIG_BCM_ARCHER)
#include "score_archer.h"
#define MSEC_UNIT_ADJUST(v)             (v)
#define MSEC_UNIT_ADJUST_WLAN_NIC(v)    (v)
#else
#error "SCORE not supported"
#endif

#endif /* ___SCORE_HW_H___ */
