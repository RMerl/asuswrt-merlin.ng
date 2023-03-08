/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
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

#ifndef __BCM_FSUTILS_H__
#define __BCM_FSUTILS_H__

#include "number_defs.h"
#include "bcm_retcodes.h"

#if defined __cplusplus
extern "C" {
#endif

/*!\file bcm_fsutils.h
 * \brief Broadcom filesystem helper functions.
 *
 * This file contains Broadcom specific filesystem operations.
 * Generic file system operations should go into the sys_util lib.
 */

/** Return the path to the CommEngine build directory in the given buffer.
 *
 * This function should only be used in the DESKTOP_LINUX mode, but if
 * it is called on a real gateway build, it will return /.
 *
 * @param pathBuf   (OUT) Contains the pathBuf.
 * @param pathBufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return CmsRet enum.
 */
BcmRet bcmUtl_getBaseDir(char *pathBuf, UINT32 pathBufLen);


/** Write a hint file in tmp filesystem to indicate shutdown is in progress.
 *  This allows apps to exit silently instead of printing an error if it
 *  loses connection to some other app.  Note this function does NOT actually
 *  shut down the system.  The calling app needs to do that separately.
 *
 * @param requestingApp (IN) provide the string name of the app declaring
 *        the shutdown.
 */
void bcmUtl_declareShutdownInProgress(const char *requestingApp);

/** Return 1 if shutdown is in progress (hint file present), else return 0. */
int bcmUtl_isShutdownInProgress();


#if defined __cplusplus
};
#endif
#endif  /* __BCM_FSUTILS_H__ */
