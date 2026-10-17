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

/*
*******************************************************************************
* File Name  : archer_gpl_hooks.h
*
*******************************************************************************
*/

#ifndef __ARCHER_GPL_HOOKS_H__
#define __ARCHER_GPL_HOOKS_H__

#include "bcm_score_defs.h"

// SCORE

#if defined(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)
#define ARCHER_TIMESTAMP_HZ  10000 // 100us
#else
#define ARCHER_TIMESTAMP_HZ  10000 // 100us
#endif

#define ARCHER_TIMESTAMP_2_MSEC(_timestamp)             \
    ( (_timestamp) / (ARCHER_TIMESTAMP_HZ / 1000) )

typedef struct {
    int (* profile_add)(uint32_t *dcp_id_p, const score_profile_cfg_t *dcp_cfg_p);
    int (* profile_get)(uint32_t dcp_id, score_profile_cfg_t *dcp_cfg_p);
    int (* profile_delete)(uint32_t dcp_id);
    void (* qstat_enable)(int enable);
    void (* qstat_dump)(void);
} archer_gpl_score_hooks_t;

typedef struct {
    int (* bnl_data_write)(score_bnl_data_t *bnl_data_p);
    void (* bnl_data_notify)(void);
    void (* bnl_queue_dump)(void);
} archer_gpl_score_callbacks_t;

extern archer_gpl_score_hooks_t archer_gpl_score_hooks_g;

extern archer_gpl_score_callbacks_t archer_gpl_score_callbacks_g;

static inline int archer_gpl_score_profile_add(uint32_t *dcp_id_p,
                                                 const score_profile_cfg_t *dcp_cfg_p)
{
    return archer_gpl_score_hooks_g.profile_add(dcp_id_p, dcp_cfg_p);
}

static inline int archer_gpl_score_profile_get(uint32_t dcp_id,
                                                 score_profile_cfg_t *dcp_cfg_p)
{
    return archer_gpl_score_hooks_g.profile_get(dcp_id, dcp_cfg_p);
}

static inline int archer_gpl_score_profile_delete(uint32_t dcp_id)
{
    return archer_gpl_score_hooks_g.profile_delete(dcp_id);
}

static inline void archer_gpl_score_qstat_enable(int enable)
{
    archer_gpl_score_hooks_g.qstat_enable(enable);
}

static inline void archer_gpl_score_qstat_dump(void)
{
    archer_gpl_score_hooks_g.qstat_dump();
}

int archer_gpl_score_bind(archer_gpl_score_hooks_t *hooks_p);

static inline int archer_gpl_score_bnl_data_write(score_bnl_data_t *bnl_data_p)
{
    if(likely(archer_gpl_score_callbacks_g.bnl_data_write))
    {
        return archer_gpl_score_callbacks_g.bnl_data_write(bnl_data_p);
    }

    return -1;
}

static inline void archer_gpl_score_bnl_data_notify(void)
{
    if(likely(archer_gpl_score_callbacks_g.bnl_data_notify))
    {
        archer_gpl_score_callbacks_g.bnl_data_notify();
    }
}

static inline void archer_gpl_score_bnl_queue_dump(void)
{
    if(likely(archer_gpl_score_callbacks_g.bnl_queue_dump))
    {
        archer_gpl_score_callbacks_g.bnl_queue_dump();
    }
}

int archer_gpl_score_callbacks_bind(archer_gpl_score_callbacks_t *callbacks_p);

#endif /* __ARCHER_GPL_HOOKS_H__ */
