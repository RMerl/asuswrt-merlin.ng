/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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
