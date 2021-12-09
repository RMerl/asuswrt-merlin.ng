#if defined(CONFIG_BCM_KF_RTPRIO_DEF)
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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

#ifndef _BCM_REALTIME_H_
#define _BCM_REALTIME_H_

/*
 * This file defines the real time priority levels used by the various
 * threads in the system.  It is important that all threads coordinate
 * their priority levels so that the desired effect is achieved.
 * These priorities are also related cgroups, so check the cgroups
 * groupings and cpu allocations (if cgroups is enabled).
 */

/** highest priority threads in the system.
 *
 * Threads at this priority require the absolute minium latency.  However,
 * they should only run very briefly (<2ms per run).
 * These threads should also run at sched policy FIFO.
 */
#define BCM_RTPRIO_HIGH               75


/** priority for the voip DSP.
 *
 * Note this is not for all voip threads, just the DSP thread.
 * The other voice threads should be run at the other priorities that are
 * defined.
 */
#define BCM_RTPRIO_VOIPDSP            35


/** priority for all data forwarding.
 *
 * This is for data and video streaming.  Not clear if we need to split out
 * sub-categories here such as video, versus web data, versus voice.
 * Probably need to use cgroups if a system needs to handle many types of
 * streams.
 * Threads running at this priority should use sched policy Round-Robin.
 */
#define BCM_RTPRIO_DATA                 5

/** priority for all tasks that handle control messages related to data path.
 *
 * ex: bpm tasl handling allocation/free of data buffers.
 * Threads running at this priority should use sched policy Round-Robin.
 */
#define BCM_RTPRIO_DATA_CONTROL         10

#endif /* _BCM_REALTIME_H_ */

#endif
