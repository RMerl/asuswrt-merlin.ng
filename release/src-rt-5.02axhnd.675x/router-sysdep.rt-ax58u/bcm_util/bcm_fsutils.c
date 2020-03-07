/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bcm_fsutils.h"
#include "bcm_ulog.h"

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
