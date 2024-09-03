/* ***** BEGIN LICENSE BLOCK *****
 *
 * Copyright (c) 2007 Broadcom Corporation
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
 * :> 
 * ***** END LICENSE BLOCK ***** */


#ifndef CMS_BASE64_H_
#define CMS_BASE64_H_

#include "cms.h"


/*!\file cms_base64.h
 * \brief Header file for base64 to binary conversion functions.
 *
 * These functions allow callers to convert base64 encoded strings to
 * binary buffers and back.
 *
 */



/** Encode a binary buffer in ASCII base64 encoding.
 * 
 * @param src    (IN)  Input binary buffer.
 * @param srclen (IN)  Length of input binary buffer.
 * @param dest   (OUT) This function will allocate a buffer and put the
 *             null terminated base64 ASCII string in it.  The caller is
 *             responsible for freeing this buffer.
 *
 * @return CmsRet enum.
 */ 
CmsRet cmsB64_encode(const unsigned char *src, UINT32 srclen, char **dest);


/** Decode a null terminated base64 ASCII string into a binary buffer.
 * 
 * @param base64Str (IN) Input base64 ASCII string.
 * @param binaryBuf (OUT) This function will allocate a buffer and put
 *                   the decoded binary data into the buffer.  The caller
 *                   is responsible for freeing the buffer.
 * @param binaryBufLen (OUT) The length of the binary buffer.
 * 
 * @return CmsRet enum.
 */
CmsRet cmsB64_decode(const char *base64Str, UINT8 **binaryBuf, UINT32 *binaryBufLen);

#endif /*CMS_BASE64_H_*/
