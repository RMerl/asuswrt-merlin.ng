/***********************************************************************
 *
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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
 *
************************************************************************/

#ifndef __SYSUTIL_H__
#define __SYSUTIL_H__

/*!\file sysutil.h
 * \brief Header file for various system utility functions. 
 *
 */
#include "number_defs.h"
#include "bcm_retcodes.h"

/** Get the thread id of the calling process.  For single threaded processes,
 *  the thread id is the same as process id (pid).
 *
 * @return the thread id.
 */
int sysUtl_getThreadId();

/** Same as sysUtl_getThreadId().
 */
int sysUtl_gettid();


/** Return the number of CPU threads on system.
 *
 * @returns At least 1.
 */
UINT32 sysUtil_getNumCpuThreads(void);

/** Get frequency and architecture information of the given processor ID.
 *
 * @param index        (IN)  The 0-based processor index.
 * @param frequency    (OUT) The processor frequency in MHz
 * @param architecture (OUT) Caller must pass in buffer of at least 32 bytes.
 * 
 * Return BcmRet
 */
BcmRet sysUtil_getCpuInfo(UINT32 index, UINT32 *frequency, char *architecture);

#endif /* __SYSUTIL_H__ */
