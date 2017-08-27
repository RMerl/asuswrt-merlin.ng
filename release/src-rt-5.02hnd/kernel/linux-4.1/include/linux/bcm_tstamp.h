#if defined(CONFIG_BCM_KF_TSTAMP)
/*
<:copyright-BRCM:2011:GPL/GPL:standard

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

#ifndef _BCM_TSTAMP_H_
#define _BCM_TSTAMP_H_

#include <linux/types.h>

/*
 * This is a just a simple set of utility functions for measuring
 * small amounts of time (<20 seconds) using the MIPS c0 counter.
 * The main limitation with this implementation is that the c0 counter
 * will roll over about every 21 seconds, so measurements of time that
 * are longer than 20 seconds will be unreliable.
 * These functions maintain no state (other than the initial multipliers
 * and divisors based on clock speed), so no SMP locking is needed to
 * use these functions.
 * It is OK to read a starting timestamp on one CPU, and then read the
 * ending timestamp on the other.  The c0 counters of both CPU's are
 * within about 20 cycles of each other, and bcm_tstamp_delta() tries
 * to detect if a migration plus read of a slightly behind end timestamp
 * has happened (seems extremely unlikely though).  In this case, it
 * returns 1 cycle (instead of 4 billion cycles, which is unlikely unless
 * you are measuring something that is close to 20 seconds long.)
 */

/** Get current timestamp
 */
u32 bcm_tstamp_read(void);

/** Return the number of cycles elapsed between start and end.
 */
u32 bcm_tstamp_delta(u32 start, u32 end);

/** Return the number of cycles elapsed between start and now.
 */
u32 bcm_tstamp_elapsed(u32 start);

/** Convert a timestamp to microseconds.
 */
u32 bcm_tstamp2us(u32 i);

/** Convert a timestamp to nanoseconds.  Note 64 bit return val.
 */
u64 bcm_tstamp2ns(u32 i);

#endif /* _BCM_TSTAMP_H_ */
#endif  /* defined(BCM_KF_TSTAMP) */
