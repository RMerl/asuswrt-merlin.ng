/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2019:DUAL/GPL:standard

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

   snprintf(filename, sizeof(filename), "/proc/%d/statm", tid);
   if ((fp = fopen(filename, "r")) == NULL)
   {
      /* this pid/tid does not exist */
      return -1;
   }

   {
      int total, rss, shared, text, dat, lib, dirty;
      // Expected format of statm is: total rss shared text data lib dirty
      // but we only care about total.
      // Values are reported by proc as "pages".  assume a page is 4KB
      c = fscanf(fp, "%d %d %d %d %d %d %d",
                     &total, &rss, &shared, &text, &dat, &lib, &dirty);
      if (c > 1)
         info->totalMemKB = total * 4;
   }
   fclose(fp);

   return rval;
}
