/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Ltd
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
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
 
#include "sysutil_fs.h"


int sysUtil_isFilePresent(const char *filename)
{
   struct stat statbuf;
   int rc;

   rc = stat(filename, &statbuf);
   if (rc == 0)
   {
      return 1;  // TRUE
   }
   else
   {
      return 0;  // FALSE
   }
}


int sysUtil_getFileSize(const char *filename)
{
   struct stat statbuf;
   int rc;

   rc = stat(filename, &statbuf);
   if (rc == 0)
   {
      return ((int) statbuf.st_size);
   }
   else
   {
      return -1;
   }
}


BcmRet sysUtil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize)
{
   SINT32 actualFileSize;
   SINT32 fd, rc;

   if (-1 == (actualFileSize = sysUtil_getFileSize(filename)))
   {
      fprintf(stderr, "%s: could not get filesize for %s", __FUNCTION__, filename);
      return BCMRET_INTERNAL_ERROR;
   }

   if (*bufSize < (UINT32) actualFileSize)
   {
      fprintf(stderr, "%s: user supplied buffer is %d, filename actual size is %d",
              __FUNCTION__, *bufSize, actualFileSize);
      return BCMRET_RESOURCE_EXCEEDED;
   }

   *bufSize = 0;
       
   fd = open(filename, 0);
   if (fd < 0)
   {
      fprintf(stderr, "%s: could not open file %s, errno=%d", __FUNCTION__, filename, errno);
      return BCMRET_INTERNAL_ERROR;
   }

   rc = read(fd, buf, actualFileSize);
   if (rc != actualFileSize)
   {
      fprintf(stderr, "%s: read error, got %d, expected %d", __FUNCTION__, rc, actualFileSize);
      close(fd);
      return BCMRET_INTERNAL_ERROR;
   }

   close(fd);

   /* let user know how many bytes was actually copied */
   *bufSize = (UINT32) actualFileSize;
   return BCMRET_SUCCESS;
}
