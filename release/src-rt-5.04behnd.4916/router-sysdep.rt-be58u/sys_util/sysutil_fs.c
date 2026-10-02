/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Ltd
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/statvfs.h>

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


BcmRet sysUtil_readFileToDynamicBuf(const char *filename, UINT8 **buf, UINT32 *bufSize)
{
   size_t sz = 1024, offset = 0;
   UINT8 *localBuf, *newBuf;
   SINT32 fd, rc;

   if ((buf == NULL) || (bufSize == NULL))
   {
      fprintf(stderr, "%s: null input args %p/%p", __FUNCTION__, buf, bufSize);
      return BCMRET_INVALID_ARGUMENTS;
   }

   localBuf = malloc(sz);
   if (localBuf == NULL)
   {
      fprintf(stderr, "%s: initial malloc failed", __FUNCTION__);
      return BCMRET_INTERNAL_ERROR;
   }

   fd = open(filename, 0);
   if (fd < 0)
   {
      fprintf(stderr, "%s: could not open file %s, errno=%d", __FUNCTION__, filename, errno);
      free(localBuf);
      return BCMRET_INVALID_ARGUMENTS;
   }

   do {
      // grow buf if needed
      if (offset == sz) {
         sz *= 2;
         newBuf = realloc(localBuf, sz);
         if (newBuf == NULL)
         {
            fprintf(stderr, "%s: realloc of %d failed", __FUNCTION__, (UINT32) sz);
            // orig localBuf still needs to be freed, which will be done below
            rc = -1;
            break;
         }
         else
         {
            localBuf = newBuf;
         }
      }

      rc = read(fd, &localBuf[offset], sz - offset);
      if (rc < 0) {
         fprintf(stderr, "%s: error during read of %s at offset %d, errno=%d", __FUNCTION__, filename, (UINT32) offset, errno);
         break;
      }
      offset += rc;
   } while (rc != 0);

   close(fd);

   if (rc == 0)
   {
      /* let user know how many bytes was actually copied */
      *bufSize = (UINT32) offset;
      *buf = localBuf;
      return BCMRET_SUCCESS;
   }
   else
   {
      if (localBuf)
      {
         free(localBuf);
      }
      return BCMRET_INTERNAL_ERROR;
   }
}


BcmRet sysUtil_readProcToBuffer(const char *filename, char *buf, UINT32 bufLen)
{
   BcmRet ret = BCMRET_NO_MORE_INSTANCES;
   FILE *fp = NULL;

   if ((fp = fopen(filename, "r")) == NULL)
   {
      return BCMRET_OBJECT_NOT_FOUND;
   }

   if (fgets(buf, bufLen, fp) != NULL )
   {
      ret = BCMRET_SUCCESS;
   }
   fclose(fp);

   return ret;
}


BcmRet sysUtil_writeBufferToFile(const char *filename, const UINT8 *buf, UINT32 bufLen)
{
   BcmRet ret=BCMRET_SUCCESS;
   SINT32 fd, rc;

   if ((fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU)) < 0)
   {
      fprintf(stderr, "%s: could not open %s", __FUNCTION__, filename);
      return BCMRET_INTERNAL_ERROR;
   }

   rc = write(fd, buf, bufLen);

   if (rc < (SINT32) bufLen)
   {
      fprintf(stderr, "%s: write to %s failed, rc=%d", __FUNCTION__, filename, rc);
      ret = BCMRET_INTERNAL_ERROR;
   }

   close(fd);

   return ret;
}


BcmRet sysUtil_appendBufferToFile(const char *filename, const UINT8 *buf, UINT32 bufLen)
{
   BcmRet ret=BCMRET_SUCCESS;
   SINT32 fd, rc;

   if ((fd = open(filename, O_RDWR|O_APPEND)) < 0)
   {
      fprintf(stderr, "%s: could not open %s", __FUNCTION__, filename);
      return BCMRET_INTERNAL_ERROR;
   }

   rc = write(fd, buf, bufLen);

   if (rc < (SINT32) bufLen)
   {
      fprintf(stderr, "%s: write to %s failed, rc=%d", __FUNCTION__, filename, rc);
      ret = BCMRET_INTERNAL_ERROR;
   }

   close(fd);

   return ret;
}


BcmRet sysUtil_getFileSystemInfo(const char *path,
                                 UINT32 *freeKB, UINT32 *totalKB)
{
   int rc;
   struct statvfs vfs;
   memset(&vfs, 0, sizeof(vfs));

   // statvfs does not return good data for /var (the tmp fs, which is probably
   // the most important one).
   rc = statvfs(path, &vfs);
   if (rc < 0)
   {
      fprintf(stderr, "%s: could not statvfs %s, errno=%d",
              __FUNCTION__, path, errno);
      return BCMRET_INVALID_ARGUMENTS;
   }

   if (freeKB != NULL)
   {
      *freeKB = (UINT32) ((vfs.f_bsize * vfs.f_bfree) / 1024);
   }

   if (totalKB != NULL)
   {
      *totalKB = (UINT32) ((vfs.f_bsize * vfs.f_blocks) / 1024);
   }

   return BCMRET_SUCCESS;
}

