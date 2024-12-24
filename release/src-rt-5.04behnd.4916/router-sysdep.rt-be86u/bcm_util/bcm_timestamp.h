/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
