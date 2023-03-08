/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bcm_fsutils.h"
#include "bcm_ulog.h"
#include "sysutil_fs.h"

#include "cms_params.h"  // for SMD_SHUTDOWN_IN_PROGRESS


BcmRet bcmUtl_getBaseDir(char *pathBuf, UINT32 pathBufLen)
{
   UINT32 rc;

#if defined(DESKTOP_LINUX)
   char pwd[1024]={0};
   UINT32 pwdLen = sizeof(pwd);
   char *str;
   char *envDir;
   struct stat statbuf;

   /*
    * If user has set an env var to tell us where the base dir is,
    * always use it.
    */
   if ((envDir = getenv("SDK_BASE_DIR")) != NULL)
   {
      rc = snprintf(pwd, sizeof(pwd), "%s/userspace", envDir);
      if (rc >= (SINT32) sizeof(pwd))
      {
         bcmuLog_error("env var %s too long (max=%d)", envDir, sizeof(pwd));
         return BCMRET_RESOURCE_EXCEEDED;
      }
      if ((rc = stat(pwd, &statbuf)) == 0)
      {
         /* SDK_BASE_DIR is good, use it. */
         rc = snprintf(pathBuf, pathBufLen, "%s", envDir);
      }
      else
      {
         /* SDK_BASE_DIR is set, but points to bad location */
         return BCMRET_INVALID_ARGUMENTS;
      }
   }
   else
   {
      /*
       * User did not set env var, so try to figure out where base dir is
       * based on current directory.
       */
      if (NULL == getcwd(pwd, pwdLen) ||
          strlen(pwd) == pwdLen - 1)
      {
         bcmuLog_error("Buffer for getcwd is not big enough");
         return BCMRET_INTERNAL_ERROR;
      }

      str = strstr(pwd, "userspace");
      if (str == NULL)
      {
         str = strstr(pwd, "unittests");
      }
      if (str != NULL)
      {
         /*
          * OK, we are running from under a recognized top level directory
          * (userspace or unittests).
          * null terminate the string right before that directory and that
          * should give us the basedir.
          */
         str--;
         *str = 0;

         rc = snprintf(pathBuf, pathBufLen, "%s", pwd);
      }
      else
      {
         // Can't figure out our base dir.
         return BCMRET_INVALID_ARGUMENTS;
      }
   }
   
#elif defined(BUILD_DESKTOP_BEEP)

   rc = snprintf(pathBuf, pathBufLen, "/var");

#else

   // Normal system build.
   rc = snprintf(pathBuf, pathBufLen, "/");

#endif /* DESKTOP_LINUX or BUILD_DESKTOP_BEEP */

   if (rc >= pathBufLen)
   {
      bcmuLog_error("pathBufLen %d is too small for buf %s", pathBufLen, pathBuf);
      return BCMRET_RESOURCE_EXCEEDED;
   }

   return BCMRET_SUCCESS;
}


void bcmUtl_declareShutdownInProgress(const char *requestingApp)
{
   const char *unknown="unknown";
   BcmRet ret = BCMRET_INTERNAL_ERROR;

   if (requestingApp != NULL)
   {
      printf("%s is about to shut system down...", requestingApp);
      ret = sysUtil_writeBufferToFile(SMD_SHUTDOWN_IN_PROGRESS,
                            (UINT8 *) requestingApp, strlen(requestingApp));
   }
   else
   {
      printf("unknown app is about to shut system down...");
      ret = sysUtil_writeBufferToFile(SMD_SHUTDOWN_IN_PROGRESS,
                            (UINT8 *) unknown, strlen(unknown));
   }
   fflush(stdout);

   if (ret != BCMRET_SUCCESS)
   {
      bcmuLog_error("write of %s failed, ret=%d",
                    SMD_SHUTDOWN_IN_PROGRESS, ret);
   }

   return;
}


int bcmUtl_isShutdownInProgress()
{
   return (sysUtil_isFilePresent(SMD_SHUTDOWN_IN_PROGRESS));
}

