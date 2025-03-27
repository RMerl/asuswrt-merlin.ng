/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
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

#include "cms.h"
#include "cms_psp.h"
#include "cms_boardioctl.h"

CmsRet cmsPsp_set(const char *key, const void *buf, UINT32 bufLen)
{
   char *currBuf;
   SINT32 count;

   /* reset value with key */
   if (bufLen == 0 && buf == NULL)
   {
      return (devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD,
                                (char *) key, 0, 0, NULL));
   }

   if ((currBuf = calloc(1, bufLen)) == NULL)
   {
      return CMSRET_RESOURCE_EXCEEDED;
   }

   /*
    * Writing to the scratch pad is a non-preemptive time consuming
    * operation that should be avoided.
    * Check if the new data is the same as the old data.
    */
   count = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
                           (char *) key, 0, (SINT32) bufLen, currBuf);
   if (count == (SINT32) bufLen)
   {
      if (memcmp(currBuf, buf, bufLen) == 0)
      {
         free(currBuf);
         /* set is exactly the same as the orig data, no set needed */
         return CMSRET_SUCCESS;
      }

      free(currBuf);
   }
      

   return (devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD,
                             (char *) key, 0, (SINT32) bufLen, (void *) buf));
}


SINT32 cmsPsp_get(const char *key, void *buf, UINT32 bufLen)
{

   return ((SINT32) devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD,
                                      (char *) key, 0, (SINT32) bufLen, buf));
}


SINT32 cmsPsp_list(char *buf, UINT32 bufLen)
{

   return ((SINT32) devCtl_boardIoctl(BOARD_IOCTL_FLASH_LIST, SCRATCH_PAD,
                                      NULL, 0, (SINT32) bufLen, buf));
}


CmsRet cmsPsp_clearAll(void)
{

   return (devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD,
                             "", -1, -1, ""));

}



