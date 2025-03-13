/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

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
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "genutil_hexbinary.h"


SINT32 genUtl_binaryBufToHexStringMalloc(const UINT8 *binaryBuf,
                                         UINT32 binaryBufLen,
                                         char **hexStr)
{
   UINT32 hexStrLen;
   SINT32 ret;

   if (binaryBuf == NULL || binaryBufLen == 0 || hexStr == NULL)
   {
      return HEXRET_INVALID_ARGUMENTS;
   }

   hexStrLen = binaryBufLen * 2 + 1;
   *hexStr = calloc(1, hexStrLen);
   if (*hexStr == NULL)
   {
      return HEXRET_RESOURCE_EXCEEDED;
   }

   ret = genUtl_binaryBufToHexString(binaryBuf, binaryBufLen, *hexStr);

   if (ret != HEXRET_SUCCESS)
   {
      free(*hexStr);
      *hexStr = NULL;
   }

   return ret;
}


SINT32 genUtl_binaryBufToHexString(const UINT8 *binaryBuf, UINT32 binaryBufLen, char *hexStr)
{
   UINT32 i, j;

   if (binaryBuf == NULL || binaryBufLen == 0 || hexStr == NULL)
   {
      return HEXRET_INVALID_ARGUMENTS;
   }

   for (i=0, j=0; i < binaryBufLen; i++, j+=2)
   {
      sprintf(&(hexStr[j]), "%02x", binaryBuf[i]);
   }

   return HEXRET_SUCCESS;
}


SINT32 genUtl_hexStringToBinaryBufMalloc(const char *hexStr,
                                         UINT8 **binaryBuf)
{
   SINT32 ret;

   if (hexStr == NULL || hexStr[0] == '\0' || binaryBuf == NULL)
   {
      return HEXRET_INVALID_ARGUMENTS;
   }

   *binaryBuf = calloc(1, strlen(hexStr)/2);
   if (*binaryBuf == NULL)
   {
      return HEXRET_RESOURCE_EXCEEDED;
   }

   ret = genUtl_hexStringToBinaryBuf(hexStr, *binaryBuf);

   if (ret != HEXRET_SUCCESS)
   {
      free(*binaryBuf);
      *binaryBuf = NULL;
   }

   return ret;
}


SINT32 genUtl_hexStringToBinaryBuf(const char *hexStr, UINT8 *binaryBuf)
{
   UINT32 len;
   UINT32 i, j;

   if (hexStr == NULL || binaryBuf == NULL)
   {
      return HEXRET_INVALID_ARGUMENTS;
   }

   len = strlen(hexStr);
   if ((len < 2) || (len % 2 != 0))
   {
      /* hexStr must be at least 2 and an even number of characters */
      return HEXRET_INVALID_ARGUMENTS;
   }

   /* convert 2 hex digits at a time to handle very long strings */
   for (i=0, j=0; j < len; i++, j+=2)
   {
      UINT64 val;
      char tmpbuf[3];
      tmpbuf[0] = hexStr[j];
      tmpbuf[1] = hexStr[j+1];
      tmpbuf[2] = 0;

      val = strtoul(tmpbuf, NULL, 16);
      /* strtoul will return ULONG_MAX on error */
      if (val > 255)
      {
         return HEXRET_CONVERSION_ERROR;
      }
      else
      {
         binaryBuf[i] = (UINT8) val;
      }
   }

   return HEXRET_SUCCESS;
}
