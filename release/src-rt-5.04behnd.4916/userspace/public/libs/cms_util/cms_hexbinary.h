/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
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
:>
 *
 ************************************************************************/

#ifndef __CMS_HEXBINARY_H__
#define __CMS_HEXBINARY_H__



/*!\file cms_hexbinary.h
 * \brief Header file for HexBinary to Binary conversion functions.
 *
 * These functions allow callers to convert HexBinary strings to
 * binary buffers and back.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Convert binary buffer to hex encoding.
 *
 * @param binaryBuf    (IN)  Input binary buffer.
 * @param binaryBufLen (IN)  Length of input binary buffer.
 * @param hexStr   (OUT) This function will allocate a buffer and put the
 *                       null terminated hex string in it.  The caller is
 *                       responsible for freeing this buffer.
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_binaryBufToHexString(const UINT8 *binaryBuf, UINT32 binaryBufLen, char **hexStr);


/** Same as cmsUtl_binaryBufToHexString except there must be some input data.
 *  i.e. binaryBufLen > 0
 */
CmsRet cmsUtl_binaryBufToHexStringStrict(const UINT8 *binaryBuf,
                                         UINT32 binaryBufLen,
                                         char **hexStr);


/** Convert a null terminated hex string into a binary buffer.
 *
 * @param hexStr (IN) Input hex string. There must be an even number of
 *                    characters in the string.  This means if the first
 *                    value is less than 128, it must have a preceding 0.
 * @param binaryBuf (OUT) This function will allocate a buffer and put
 *                   the decoded binary data into the buffer.  The caller
 *                   is responsible for freeing the buffer.
 * @param binaryBufLen (OUT) The length of the binary buffer.
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_hexStringToBinaryBuf(const char *hexStr, UINT8 **binaryBuf, UINT32 *binaryBufLen);


/** Same as cmsUtl_hexStringToBinaryBuf except there must be some input data.
 *  i.e. strlen(hexStr) > 0
 */
 CmsRet cmsUtl_hexStringToBinaryBufStrict(const char *hexStr,
                                          UINT8 **binaryBuf,
                                          UINT32 *binaryBufLen);


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif  /* __CMS_HEXBINARY_H__ */
