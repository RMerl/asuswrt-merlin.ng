/***********************************************************************
 *
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bcm_retcodes.h"
#include "number_defs.h"

UINT32 sysUtil_getNumCpuThreads(void)
{
   UINT32 count=0;
   char line[512]={0};
   FILE *fs;

   fs = fopen("/proc/cpuinfo", "r");
   if ( fs == NULL )
   {
      fprintf(stderr, "sysUtil_getNumCpuThreads: Could not open /proc/cpuinfo, return 1\n");
      return 1;
   }

   while ( fgets(line, sizeof(line), fs) )
   {
      if (0 == strncmp(line, "processor", 9))
      {
         count++;
      }
   } /* while */

   fclose(fs);

   if (count == 0)
   {
      fprintf(stderr, "sysUtil_getNumCpuThreads: could not find any processors, return 1\n");
      return 1;
   }

   return count;
}


BcmRet sysUtil_getCpuInfo(UINT32 index, UINT32 *frequency, char *architecture)
{
   UINT32 currIndex=0;;
   char line[512]={0};
   char *pChar = NULL;
   FILE *fs = NULL;

   if (frequency == NULL || architecture == NULL)
   {
      return BCMRET_INVALID_ARGUMENTS;
   }

   *frequency = 0;

   fs = fopen("/proc/cpuinfo", "r");
   if (fs == NULL)
   {
      fprintf(stderr, "sysUtil_getCpuInfo: Could not open /proc/cpuinfo\n");
      return BCMRET_INTERNAL_ERROR;
   }

   // In theory, this lib is independent of TR181, but to avoid another
   // translation layer, use the architecture strings defined in TR181.
   // In most cases, it will be "arm".
   strcpy(architecture, "arm");

   while ( fgets(line, sizeof(line), fs) )
   {
      // processor line looks like this: processor	: 1
      if (strncmp(line, "processor", 9) == 0 &&
          (pChar = strstr(line, ":")) != NULL &&
          (pChar + 2) != NULL)
      {
         // pChar+2: read pass ": "
         currIndex = (UINT32) strtoul(pChar+2, (char **)NULL, 10);
         if (currIndex > index)
         {
            break;
         }
      }

      // Change architecture for DESKTOP_LINUX
      if (strncmp(line, "vendor_id", 9) == 0 &&
          (pChar = strstr(line, ":")) != NULL)
      {
         if (strstr(line, "Intel") || strstr(line, "386"))
         {
            strcpy(architecture, "i386");
         }
      }

      // Get processor frequency in MHz.  Handle 2 versions, one for real
      // gateway and one for DESKTOP_LINUX.
      // BogoMIPS        : 1980.41 (arm running linux4.1 BogoMIPS != Freq)
      // cpu MHz		    : 2592.000 (desktop linux running Linux 4.18)
      if (((strncmp(line, "BogoMIPS", 8) == 0) ||
           (strncmp(line, "cpu MHz", 7) == 0)) &&
          ((pChar = strstr(line, ":")) != NULL))
      {
         UINT32 i;
         pChar += 2; // advance to the start of numbers
         // turn the decimal point to string terminator
         for (i=0; i < 6; i++)
         {
            if (pChar[i] == '.' || pChar[i] == '\0')
            {
               pChar[i] = '\0';
               *frequency = (UINT32) strtoul(pChar, (char **)NULL, 10);
               break;
            }
         }
      }
   }

   fclose(fs);
   return (currIndex >= index ? BCMRET_SUCCESS : BCMRET_OBJECT_NOT_FOUND);
}
