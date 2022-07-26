/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

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

#include "bcm_boardutils.h"
#include "bcm_ulog.h"
#include "cms_version.h"

int bcmUtl_isBootloaderUboot(void)
{
   struct stat statbuf;
   int rc;

   // UBoot creates a /proc/environment dir, CFE does not.
   rc = stat("/proc/environment", &statbuf);
   if (rc == 0)
      return 1;  // found, bootloader is UBoot, return TRUE.

   // Bootloader is CFE.  Return FALSE.
   return 0;
}


BcmRet bcmUtl_getManufacturer(char *buf, UINT32 bufLen)
{
   const char *m = "Broadcom";
   UINT32 len = strlen(m);

   if (buf == NULL)
      return BCMRET_INVALID_ARGUMENTS;
   if (bufLen < len+1)
      return BCMRET_RESOURCE_EXCEEDED;

   strcpy(buf, m);
   return BCMRET_SUCCESS;
}


BcmRet bcmUtl_getHardwareVersion(char *buf, UINT32 bufLen)
{
   char *hw_version = "tmp_hardware1.0";
   UINT32 len = strlen(hw_version);

   if (buf == NULL)
      return BCMRET_INVALID_ARGUMENTS;
   if (bufLen < len+1)
      return BCMRET_RESOURCE_EXCEEDED;

   strcpy(buf, hw_version);
   return BCMRET_SUCCESS;
}


BcmRet bcmUtl_getSoftwareVersion(char *buf, UINT32 bufLen)
{
   UINT32 len = strlen(CMS_RELEASE_VERSION);

   if (buf == NULL)
      return BCMRET_INVALID_ARGUMENTS;
   if (bufLen < len+1)
      return BCMRET_RESOURCE_EXCEEDED;

   strcpy(buf, CMS_RELEASE_VERSION);
   return BCMRET_SUCCESS;
}


BcmRet bcmUtl_getBaseMacAddress(char *macAddrBuf)
{
   const char *fileName = "/proc/environment/basemacaddr";
   const char *fileName2 = "/proc/environment/ethaddr";
   char buf[32]={0};
   int len, xlen=17; // weird: fgets reads in phantom extra chars, limit to 17.
   FILE *fp = NULL;

   if (macAddrBuf == NULL)
      return BCMRET_INVALID_ARGUMENTS;

   if ((fp = fopen(fileName, "r")) == NULL)
   {
      bcmuLog_debug("failed to open file %s, try alternate", fileName);
      if ((fp = fopen(fileName2, "r")) == NULL)
      {
         bcmuLog_error("could not open %s (or %s)", fileName2, fileName);
         return BCMRET_INTERNAL_ERROR;
      }
   }
   
   if( NULL == fgets(buf, xlen+1, fp) )
   {
      bcmuLog_error("Could not read %s", fileName);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   len = strlen(buf);
   bcmuLog_notice("len=%d read in buffer=%s", len, buf);
   if (len != xlen)
   {
      bcmuLog_error("Unexpected mac addr len %d (expected %d)", len, xlen);
      return BCMRET_INTERNAL_ERROR;
   }

   memcpy(macAddrBuf, buf, xlen);  // xlen is 17, exact length of mac addr
   return BCMRET_SUCCESS;
}


BcmRet bcmUtl_getSerialNumber(char *buf, UINT32 bufLen)
{
   char macAddrBuf[32]={0};
   int i;
   BcmRet ret;
   
   if (bufLen < 13)  // macAddr without colons is 12+null term=13
      return BCMRET_RESOURCE_EXCEEDED;

   ret = bcmUtl_getBaseMacAddress(macAddrBuf);
   if (ret != BCMRET_SUCCESS)
   {
      bcmuLog_error("Could not get BaseMacAddress, ret=%d", ret);
      return ret;
   }

   /*
    * Just keep old SDK serialNumber format.
    * remove ':' and convert uppercase letters to lowercase letters
    */
   for (i = 0; i < 18; i++)
   {
      if (macAddrBuf[i] == ':')
         continue;

      if ((macAddrBuf[i] >= 'A') && (macAddrBuf[i] <= 'F'))
         *(buf++) = macAddrBuf[i] + 32;
      else
         *(buf++) = macAddrBuf[i];
   }

   return BCMRET_SUCCESS;
}


BcmRet bcmUtl_getBootloaderVersion(char* versionBuf, UINT32 bufLen)
{
   char strBuf[256]={0};
   const char *fileName = "/sys/firmware/devicetree/base/uboot-version";
   UINT32 len;
   FILE *fp = NULL;

   if (versionBuf == NULL)
   {
      bcmuLog_error("NULL versionBuf!");
      return BCMRET_INVALID_ARGUMENTS;
   }

   if ((fp = fopen(fileName, "r")) == NULL)
   {
      bcmuLog_notice("failed to open file %s. Could be cfe boot image.", fileName);
      return BCMRET_INTERNAL_ERROR;
   }
   
   if( NULL == fgets(strBuf, sizeof(strBuf)-1, fp) )
   {
      bcmuLog_error("Could not read %s", fileName);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   len = strlen(strBuf);
   bcmuLog_notice("got len=%d str=%s", len, strBuf);
   if (len >= bufLen)
   {
      bcmuLog_error("versionBuf buffer (len=%d) is too small, need %d",
                    bufLen, len+1);
      return BCMRET_INTERNAL_ERROR;
   }

   snprintf(versionBuf, bufLen, "%s", strBuf);
   return BCMRET_SUCCESS;
}


void bcmUtl_startRebootWatchdog()
{
   // stop existing wdtd
   if ( system("wdtctl -d stop") != 0 )
      bcmuLog_error("Failed to run wdtctl cmd.");

   // start one final watchdog timer.  Control must reach UBoot before
   // this timer expires.  (Well, actually, if control does not reach UBoot,
   // that is ok too because the timer will expire, the system will reset,
   // and we will go into UBoot anyways.  But this timer will prevent a hang.)
   if ( system("wdtctl -t 60 start") != 0 )
      bcmuLog_error("Failed to run wdtctl cmd.");
}

