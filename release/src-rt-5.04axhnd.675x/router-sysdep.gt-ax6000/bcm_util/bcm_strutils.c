/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2021:DUAL/GPL:standard

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

#include "bcm_strutils.h"

// Comment this out if strsep() is not available, then we will use the
// Broadcom implementation.
#define USE_STRSEP

char *bcmUtl_strtok_r(char *str, const char *delim, char **saveptr)
{
#ifdef USE_STRSEP
   // strsep() seems to do what we need, but could be less portable.
   // does musl and ucLibc have it?
   if (str != NULL)
   {
      *saveptr = str;
   }
   return (strsep(saveptr, delim));
#else
   // This is the Broadcom implementation.
   char *token, *ptr;

   // Check delim char
   if ((delim == NULL) || (delim[0] == '\0') || (delim[1] != '\0'))
   {
      // delim must be a single character.
      return NULL;
   }

   if (saveptr == NULL)  // savePtr always required
      return NULL;

   if ((str != NULL) && (*saveptr != NULL))  // can't specify both
      return NULL;

   token = (str != NULL) ? str : *saveptr;
   if (token == NULL)
   {
      // No more data to parse.
      *saveptr = NULL;
      return NULL;
   }

   ptr = token;
   while ((*ptr != delim[0]) && (*ptr != '\0'))
      ptr++;

   if (*ptr == delim[0])
   {
      // we hit a delimiter, terminate token by overwriting delim with NULL
      *ptr = '\0';
      *saveptr = ptr+1;  // next call, start at next token.
   }
   else
   {
      // we hit end of string, no more data on next call.
      *saveptr = NULL;
   }

   return token;
#endif  /* USE_STRSEP */
}
