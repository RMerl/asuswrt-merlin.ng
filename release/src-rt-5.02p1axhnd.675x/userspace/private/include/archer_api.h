/***********************************************************************
 *
 *  Copyright (c) 2015  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2015:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*
************************************************************************/

#ifndef _ARCHER_API_H_
#define _ARCHER_API_H_

#include "archer.h"

int archer_cmd_send(archer_ioctl_cmd_t cmd, unsigned long arg);

int archer_sysport_tm_enable(void);

int archer_sysport_tm_disable(void);

int archer_sysport_tm_stats(void);

int archer_sysport_tm_stats_get(const char *if_name, int queue_index,
                                uint32_t *txPackets_p, uint32_t *txBytes_p,
                                uint32_t *droppedPackets_p, uint32_t *droppedBytes_p);

int archer_sysport_tm_queue_set(const char *if_name, int queue_index,
                                int min_kbps, int min_mbs,
                                int max_kbps, int max_mbs);

int archer_sysport_tm_queue_get(const char *if_name, int queue_index,
                                int *min_kbps_p, int *min_mbs_p,
                                int *max_kbps_p, int *max_mbs_p);

int archer_sysport_tm_port_set(const char *if_name, int kbps, int mbs);

int archer_sysport_tm_port_get(const char *if_name, int *kbps_p, int *mbs_p);

int archer_sysport_tm_arbiter_set(const char *if_name, sysport_tm_arbiter_t arbiter);

int archer_sysport_tm_arbiter_get(const char *if_name, sysport_tm_arbiter_t *arbiter_p);

int archer_sysport_tm_mode_set(const char *if_name, sysport_tm_mode_t mode);

int archer_sysport_tm_mode_get(const char *if_name, sysport_tm_mode_t *mode_p);

#endif /* _ARCHER_API_H_ */
