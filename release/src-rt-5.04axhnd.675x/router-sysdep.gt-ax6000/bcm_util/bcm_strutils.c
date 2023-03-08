/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2021:DUAL/GPL:standard

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
