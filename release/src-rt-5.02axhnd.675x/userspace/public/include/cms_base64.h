/* ***** BEGIN LICENSE BLOCK *****
 *
 * Copyright (c) 2007 Broadcom Corporation
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
