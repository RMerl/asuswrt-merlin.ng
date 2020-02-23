/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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

#ifndef __SYSUTIL_FS_H__
#define __SYSUTIL_FS_H__


#include "number_defs.h"
#include "bcm_retcodes.h"


/*!\file sysutil_fs.h
 * \brief Header file for various useful Linux filesystem operations.
 *
 */


/** Return TRUE (1) if the filename exists.  This function
 *  does not distinguish between different types of files.  So this
 *  function will return TRUE for files, symlinks, directories, sockets,
 *  device nodes, etc.
 *
 * @param filename (IN) full pathname to the file.
 *
 * @return TRUE (1) if the specified filename exists in the filesystem.
 *         Otherwise return FALSE (0).
 */
int sysUtil_isFilePresent(const char *filename);


/** Return the size of the file.
 *
 * @param filename (IN) full pathname to the file.
 *
 * @return -1 if the file does not exist, otherwise, the file size.
 */
int sysUtil_getFileSize(const char *filename);


/** Copy the contents of the file to the specified buffer.
 *
 * @param filename (IN) full pathname to the file.
 * @param buf     (OUT) buffer that will hold contents of the file.
 * @bufSize    (IN/OUT) On input, the size of the buffer, on output
 *                      the actual number of bytes that was copied
 *                      into the buffer.
 *
 * @return BcmRet enum.
 */
BcmRet sysUtil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize);


#endif /* __SYSUTIL_FS_H__ */
