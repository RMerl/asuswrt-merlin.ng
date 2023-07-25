/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

 :>
 *
 ************************************************************************/


#ifndef __GENUTIL_HEXBINARY_H__
#define __GENUTIL_HEXBINARY_H__

#include "number_defs.h"

/*!\file genutil_hexbinary.h
 * \brief Header file for HexBinary to Binary conversion functions.
 *
 * These functions allow callers to convert HexBinary strings to
 * binary buffers and back.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif


/** Return codes are all SINT32 */
 #define HEXRET_SUCCESS            0
 #define HEXRET_INTERNAL_ERROR     -1
 #define HEXRET_RESOURCE_EXCEEDED  -2
 #define HEXRET_INVALID_ARGUMENTS  -3
 #define HEXRET_CONVERSION_ERROR   -4


/** Convert binary buffer to hex encoding.
 *
 * @param binaryBuf    (IN)  Input binary buffer.
 * @param binaryBufLen (IN)  Length of input binary buffer.  Length must be
 *                           greater than 0.
 * @param hexStr   (IN/OUT) The caller is responsible for passing in a buffer
 *                          of at least binaryBufLen * 2 + 1 bytes.
 *
 *
 * @return CmsRet enum.
 */
SINT32 genUtl_binaryBufToHexString(const UINT8 *binaryBuf,
                                   UINT32 binaryBufLen,
                                   char *hexStr);

/** Same as genUtil_binaryBufToHexString except output buffer is malloc'd for
 *  the caller.  The buffer will be binaryBufLen * 2 + 1 (including the null
 *  termination character for the hexString).  The caller is responsible for
 *  freeing it.
 *
 *  WARNING: the returned buffer is allocated using standard malloc so it
 *  must not be passed into CMS MDM because CMS uses a different kind of
 *  memory allocator.  If using CMS, use the cmsUtl version of these functions.
 */
SINT32 genUtl_binaryBufToHexStringMalloc(const UINT8 *binaryBuf,
                                         UINT32 binaryBufLen,
                                         char **hexStr);



/** Convert a null terminated hex string into a binary buffer.
 *
 * @param hexStr (IN) Input hex string. There must be an even number of
 *                    characters in the string.  This means if the first
 *                    value is less than 128, it must have a preceding 0.
 *                    There must be at least 2 characters: an empty string
 *                    will not be accepted.
 * @param binaryBuf (IN/OUT) The caller must provide a buffer of at least
 *                    strlen(hexStr)/2 bytes.
 *
 * @return HEXRET value.
 */
SINT32 genUtl_hexStringToBinaryBuf(const char *hexStr, UINT8 *binaryBuf);


/** Same as genUtl_hexStringToBinaryBuf except output buffer is malloc'd for
 *  the caller.  The buffer will be strlen(hexStr)/2 bytes long.  The caller is
 *  responsible for freeing it.
 *
 *  WARNING: the returned buffer is allocated using standard malloc so it
 *  must not be passed into CMS MDM because CMS uses a different kind of
 *  memory allocator.  If using CMS, use the cmsUtl version of these functions.
 */
 SINT32 genUtl_hexStringToBinaryBufMalloc(const char *hexStr,
                                          UINT8 **binaryBuf);


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif  /* __GENUTIL_HEXBINARY_H__ */
