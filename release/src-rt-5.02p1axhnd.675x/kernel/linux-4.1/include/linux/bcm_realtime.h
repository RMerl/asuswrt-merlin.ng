#if defined(CONFIG_BCM_KF_RTPRIO_DEF)
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
