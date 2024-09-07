/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
************************************************************************/
#ifndef __BCM_TIMESTAMP_H__
#define __BCM_TIMESTAMP_H__

#if defined __cplusplus
extern "C" {
#endif

#include <time.h>

int bcm_libc_clock_gettime(clockid_t, struct timespec *);


/** Given the number of seconds since Jan 1, 1970, return the ISO8601 formatted
 *  date and time in GMT timezone.  If t is 0, then current time will be used.
 *
 *  Example of formatted string: 2022-10-31T21:07:23Z
 *
 *  Note there is also a cmsTms_getXSIDDateTime() which is similar to this
 *  function but formats the date time in the device's timezone.
 *
 * @param t      (IN)  0 or number of seconds since Jan 1, 1970.
 * @param buf    (OUT) a buffer to hold the date time string (should be at
 *                     least 21 bytes).  If the buffer is too short, the result
 *                     is undefined.
 * @param bufLen (IN)  length of buffer.
 */
void bcmTms_getISO8601DateTimeGmt(time_t t, char *buf, size_t bufLen);

// XSIDateTime is the same as ISO8601 DateTime.
void bcmTms_getXSIDateTimeGmt(time_t t, char *buf, size_t bufLen);


#if defined __cplusplus
};
#endif
#endif  /* __BCM_TIMESTAMP_H__ */
