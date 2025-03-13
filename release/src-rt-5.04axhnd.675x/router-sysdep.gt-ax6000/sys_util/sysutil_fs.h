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
 *  Note this function only works for regular files; for proc files, use
 *  sysUtil_readProcToBuffer().
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


/** Read the contents of a single line /proc file to buffer.
 *  Note this function will only read the first line; will not work if the
 *  /proc file contains multiple lines.
 *
 * @param filename (IN) full pathname to the file.
 * @param buf     (OUT) buffer that will hold contents of the file.
 * @bufLen        (IN) Size of the buffer; caller needs to ensure there is a
 *                     null terminator at the end.
 *
 * @return BcmRet enum.
 */
BcmRet sysUtil_readProcToBuffer(const char *filename, char *buf, UINT32 bufLen);


/** Write contents of the buffer to the specified file.  If file exists,
 *  it will be overwritten.  If file does not exist, it will be created.
 *
 * @param filename (IN) Name of the file.
 * @param buf      (IN) Buffer to write.
 * @param bufLen   (IN) Length of buffer.
 *
 * @return BcmRet enum.
*/
BcmRet sysUtil_writeBufferToFile(const char *filename, const UINT8 *buf, UINT32 bufLen);


/** Get basic info for specified filesystem.
 *
 * @param  path (IN) Path to any location within filesystem.
 * @param  freeKB (OUT) If not NULL, KB free/avail within this filesystem. Note
                        for a read-only filesystem, this will be 0.
 * @param  totalKB (OUT) If not NULL, total size of filesystem in KB.
 *
 * @return BcmRet enum.
 */
BcmRet sysUtil_getFileSystemInfo(const char *path,
                                 UINT32 *freeKB, UINT32 *totalKB);

#endif /* __SYSUTIL_FS_H__ */
