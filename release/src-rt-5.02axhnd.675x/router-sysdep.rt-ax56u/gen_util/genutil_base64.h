/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

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
 ***********************************************************************/


#ifndef GENUTIL_BASE64_H_
#define GENUTIL_BASE64_H_

#include "number_defs.h"

/*!\file genutil_base64.h
 * \brief Header file for base64 to binary conversion functions.
 *
 * These functions allow callers to convert base64 encoded strings to
 * binary buffers and back.
 *
 */

#define B64RET_SUCCESS            0
#define B64RET_INTERNAL_ERROR     -1
#define B64RET_RESOURCE_EXCEEDED  -2
#define B64RET_INVALID_ARGUMENTS  -3


/** Encode a binary buffer in ASCII base64 encoding.
 *
 * @param src    (IN)  Input binary buffer.
 * @param srclen (IN)  Length of input binary buffer.  Must be greater than 0.
 * @param b64StrBuf (IN/OUT) The caller is responsible for providing a buffer
 *                     of at least the length specified by genUtl_getEncodedBufferLength.
 * @param b64StrBufLen (IN) Length of b64StrBuf.
 *
 * @return B64RET value.
 */
SINT32 genUtl_b64Encode(const unsigned char *src, UINT32 srclen,
                         char *b64StrBuf, UINT32 b64StrBufLen);


/** Return the size of buffer needed to hold srclen bytes encoded to Base64.
 */
UINT32 genUtl_b64EncodedBufferLength(UINT32 srclen);


/** Same as genUtil_b64Encode except output buffer is malloc'd for
 *  the caller.  The returned b64HexStr will be null terminated.  The caller
 *  is responsible for freeing it.
 *
 *  WARNING: the returned buffer is allocated using standard malloc so it
 *  must not be passed into CMS MDM because CMS uses a different kind of
 *  memory allocator.  If using CMS, use the cmsUtl version of these functions.
 */
 SINT32 genUtl_b64EncodeMalloc(const unsigned char *src, UINT32 srclen,
                               char **b64StrBuf);


/** Decode a null terminated base64 ASCII string into a binary buffer.
 *
 * @param b64Str    (IN) Input base64 ASCII string.
 * @param binaryBuf (OUT) Caller is responsible for allocating a buf of
 *           at least the length returned by genUtl_getDecodedBufferLength(b64StrLen).
 * @param binaryBufLen (IN/OUT) The caller sets to to the length of the binary
 *           buffer.  On successful return, this will indicate the actual amount
 *           of data in binaryBuf.
 *
 * @return B64ret value.
 */
SINT32 genUtl_b64Decode(const char *b64Str,
                     UINT8 *binaryBuf, UINT32 *binaryBufLen);


/** Return the size of buffer needed to hold b64StrLen bytes decoded to binary.
  */
UINT32 genUtl_b64DecodedBufferLength(UINT32 b64StrLen);


/** Same as genUtl_b64Decode except output buffer is malloc'd for
 *  the caller.  Length of returned data is in binaryBufLen.  The caller is
 *  responsible for freeing the buffer.
 *
 *  WARNING: the returned buffer is allocated using standard malloc so it
 *  must not be passed into CMS MDM because CMS uses a different kind of
 *  memory allocator.  If using CMS, use the cmsUtl version of these functions.
 */
SINT32 genUtl_b64DecodeMalloc(const char *b64Str,
                           UINT8 **binaryBuf, UINT32 *binaryBufLen);

#endif /* GENUTIL_BASE64_H_*/
