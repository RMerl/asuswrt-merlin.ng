/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2019:DUAL/GPL:standard

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
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include "sysutil_proc.h"

int sysUtl_getThreadInfoFromProc(int tid, ProcThreadInfo *info)
{
   FILE *fp;
   char filename[256]={0};
   int c, rval=-2;
   int pid=0; // unused

   // Since we have to use the magic number of 63 below, check the #define
   // has not changed.
   if (PROC_THREAD_NAME_LEN != 64)
   {
      printf("%s: must fix code to match PROC_THREAD_NAME_LEN (%d != 64)",
             __FUNCTION__, PROC_THREAD_NAME_LEN);
      return -2;
   }

   if (info == NULL)
   {
      return -2;
   }
   memset(info, 0, sizeof(ProcThreadInfo));

   snprintf(filename, sizeof(filename), "/proc/%d/stat", tid);
   if ((fp = fopen(filename, "r")) == NULL)
   {
      /* this pid/tid does not exist */
      return -1;
   }

   // Read at most PROC_THREAD_NAME_LEN-1 (63) chars into name buffer.
   c = fscanf(fp, "%d (%63s %c", &pid, info->name, &(info->status));
   if (c == 3 && strlen(info->name) > 1)
   {
      // trim closing parenthesis from name.
      info->name[strlen(info->name)-1] = '\0';
      rval = 0;
   }
   else
   {
      printf("%s: could not parse %s\n", __FUNCTION__, filename);
      rval = -2;
   }
   fclose(fp);

   return rval;
}
