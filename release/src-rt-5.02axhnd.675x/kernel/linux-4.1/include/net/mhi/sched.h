#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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
 * File: mhi/sched.h
 *
 * Modem-Host Interface Scheduling
 */

#ifndef MHI_SCHED_H
#define MHI_SCHED_H

#define MHI_NOTIFY_QUEUE_LOW     19
#define MHI_NOTIFY_QUEUE_HIGH    20

extern int
mhi_register_queue_notifier(struct Qdisc *sch,
			struct notifier_block *nb,
			unsigned long cl);

extern int
mhi_unregister_queue_notifier(struct Qdisc *sch,
			struct notifier_block *nb,
			unsigned long cl);

#endif /* MHI_SCHED_H */
#endif /* CONFIG_BCM_KF_MHI */
