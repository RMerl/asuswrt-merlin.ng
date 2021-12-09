/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
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


#if defined __cplusplus
};
#endif
#endif  /* __BCM_FSUTILS_H__ */
