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

#ifndef __SCORE_PROCFS_H_INCLUDED__
#define __SCORE_PROCFS_H_INCLUDED__

#include "bcm_score_defs.h"

#define SCORE_PROCFS_DIR_PATH       "score"         /* dir: /procfs/score    */
#define QSTAT_PROCFS_DIR_PATH       "score/qstat"   /* dir: /procfs/score/qstat    */

#define SCORE_PROCFS_HISTORY        4
#define QSTAT_PROCFS_HISTORY        4

typedef struct {
    uint32_t msgs_received_total;               /* total messages received from runner */
    uint32_t msgs_received[SCORE_MAX_SDRS];     /* messages received from runner for sdr */
    uint32_t ctrl_sent_app_total;               /* total control messages sent to apps */
    uint32_t ctrl_sent_app[SCORE_MAX_SDRS];     /* control messages sent per app */
    uint32_t ctrl_dropped_app_total;            /* total control messages dropped */
    uint32_t ctrl_dropped_app[SCORE_MAX_SDRS];  /* control messages dropped per app */
} score_procfs_data_t;

typedef struct {
    uint32_t rnr_received_cnt;        /* received from runner queue */
    uint32_t wlan_dhd_received_cnt;   /* received from wlan */
    uint32_t wlan_nic_received_cnt;   /* received from wlan */
    uint32_t wired_received_cnt;      /* received from wired */
} qstat_procfs_data_t;

extern score_procfs_data_t score_procfs_data_g;
extern qstat_procfs_data_t qstat_procfs_data_g;


int score_procfs_construct(void);
int score_procfs_destroy(void);


#endif // __SCORE_PROCFS_H_INCLUDED__
