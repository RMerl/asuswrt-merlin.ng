/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bcm_boardutils.h"
#include "bcm_ulog.h"
#include "bcm_fsutils.h"
#include "cms_version.h"
#include "os_defs.h"
#include "bcm_bootstate.h"

static const char *rebootSystemCallReasonStr[] = {
   [REBOOT_REASON_NULL]                 = "Null",
   [REBOOT_REASON_SOFTWARE_UPGRADE]     = "Software Upgrade",
   [REBOOT_REASON_MANAGEMENT_REBOOT]    = "Management Reboot",
   [REBOOT_REASON_RESTORE_DEFAULT]      = "Restore Default",
   [REBOOT_REASON_HARDWARE_ABNORMALITY] = "Hardware Abnormality"
};

#define NUM_OF_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

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


BcmRet bcmUtl_getUbootEnvVar(const char *varName, char *buf, UINT32 *bufLen)
{
   char pathBuf[1024]={0};
   int fd;
   ssize_t s;
   BcmRet ret = BCMRET_INTERNAL_ERROR;

   // bufLen needs to hold at least 1 byte of data and 1 byte for null termination.
   if ((varName == NULL) || (buf == NULL) || (bufLen == NULL) || (*bufLen < 2))
   {
      return BCMRET_INVALID_ARGUMENTS;
   }

   snprintf(pathBuf, sizeof(pathBuf), "/proc/environment/%s", varName);
   fd = open(pathBuf, O_RDONLY);
   if (fd < 0)
   {
      // file does not exist
      return BCMRET_OBJECT_NOT_FOUND;
   }

   memset(buf, 0, *bufLen);
   s = read(fd, buf, *bufLen-1);
   if (s >= 0)
   {
      *bufLen = (UINT32) s;
      ret = BCMRET_SUCCESS;
   }
   else
   {
      bcmuLog_error("read of %s failed, bufLen=%d errno=%d",
                    pathBuf, *bufLen, errno);
   }

   close(fd);

   return ret;
}

int bcmUtl_isUbootNoCommitImage(void)
{
   char buf[16]={0};
   UINT32 bufLen = sizeof(buf);
   BcmRet ret;

   ret = bcmUtl_getUbootEnvVar("no_commit_image", buf, &bufLen);
   // matches logic in uboot http_rcv()
   if ((ret == BCMRET_SUCCESS) && (buf[0] != '0'))
   {
      return 1;
   }
   else
   {
      return 0;
   }
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


BcmRet bcmUtl_getModelName(char *buf, UINT32 bufLen)
{
   char strBuf[64]={0};
   const char *fileName = "/proc/environment/boardid";
   UINT32 len;
   FILE *fp = NULL;

   if (buf == NULL)
   {
      bcmuLog_error("NULL buf!");
      return BCMRET_INVALID_ARGUMENTS;
   }

   memset(buf, 0, bufLen);
   if ((fp = fopen(fileName, "r")) == NULL)
   {
      bcmuLog_notice("failed to open file %s. Could be cfe boot image.", fileName);
      return BCMRET_INTERNAL_ERROR;
   }

   if (NULL == fgets(strBuf, sizeof(strBuf), fp))
   {
      bcmuLog_error("Could not read %s", fileName);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   len = strlen(strBuf);

   if ((len > 0) && (strBuf[len-1] == '\n'))  //remove char '\n'
   {
      strBuf[len-1] = '\0';
      len = strlen(strBuf);
   }

   bcmuLog_notice("got len=%d str=%s", len, strBuf);
   if (len >= bufLen)
   {
      bcmuLog_error("buf (len=%d) is too small, need %d",
                    bufLen, len+1);
      return BCMRET_INTERNAL_ERROR;
   }

   snprintf(buf, bufLen, "%s", strBuf);
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
   
   if (NULL == fgets(buf, xlen+1, fp))
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

BcmRet bcmUtl_getProgrammedSerialNumber(char *buf, UINT32 bufLen)
{
   const char *fileName = "/proc/device-tree/chip_info/serial-number";
   char serialbuf[BUFLEN_32]={0};
   UINT32 len;
   FILE *fp = NULL;

   if (buf == NULL)
      return BCMRET_INVALID_ARGUMENTS;

   if ((fp = fopen(fileName, "r")) == NULL)
   {
      bcmuLog_debug("failed to open file %s", fileName);
      return BCMRET_INTERNAL_ERROR;
   }
   
   if (NULL == fgets(serialbuf,BUFLEN_32, fp))
   {
      bcmuLog_error("Could not read %s", fileName);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   len = strlen(serialbuf);
   if (len > bufLen)
   {
      bcmuLog_notice("len=%d read in buffer=%s", len, serialbuf);
      len = bufLen;
   }
   strncpy(buf, serialbuf, len); 
   return BCMRET_SUCCESS;
}

BcmRet bcmUtl_getSerialNumber(char *buf, UINT32 bufLen)
{
   char macAddrBuf[32]={0};
   int i;
   BcmRet ret;

   /* it is possible that we have a serial number programmed, if it's not found
    * then we fall back to the MAC address
    */
   ret = bcmUtl_getProgrammedSerialNumber(buf, bufLen);
   if (ret == BCMRET_SUCCESS)
   {
      return ret;
   }

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
   
   if (NULL == fgets(strBuf, sizeof(strBuf), fp))
   {
      bcmuLog_error("Could not read %s", fileName);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   len = strlen(strBuf);

   if ((len > 0) && (strBuf[len-1] == '\n'))  //remove char '\n'
   {
      strBuf[len-1] = '\0';
      len = strlen(strBuf);
   }

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

void bcmUtl_busybox_reboot(void)
{
   // This is a graceful reboot, just like typing reboot in busybox shell.
   // There is also a copy of this function in bcm_imgUtil, but duplicate
   // implementation here to avoid creating a dependency between bcm_util and
   // bcm_flashutil.
   sync();
   if ( system("reboot") != 0 )
      bcmuLog_error("Failed to run reboot cmd.");
}

/*
 * This function is to read the reason of the last reboot.
 */
static BcmRet bcmUtl_getRebootSystemCallReason(char *buf, UINT32 bufLen)
{
   const char *rebootSystemCallReasonFile = "/data/reboot_reason";

   FILE *fp = NULL;

   memset(buf, 0, bufLen);  
   if ((fp = fopen(rebootSystemCallReasonFile, "r")) == NULL)
   {
      bcmuLog_notice("failed to open file %s.", rebootSystemCallReasonFile);
      return BCMRET_INTERNAL_ERROR;
   }

   if (NULL == fgets(buf, bufLen, fp))
   {
      bcmuLog_error("Could not read %s", rebootSystemCallReasonFile);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }

   fclose(fp);
   return BCMRET_SUCCESS;
}

/*
 * This function is to write the reboot reason to a flash file
 * which can be read after reboot.
 */
static BcmRet bcmUtl_writeRebootSystemCallReason(const char *requestor, const char *reason)
{
   const char *rebootSystemCallReasonFile = "/tmp/reboot_reason";
   char rebootReason[REBOOT_REASON_MAX_LENGTH];
   FILE *fp = NULL;

   snprintf(rebootReason, REBOOT_REASON_MAX_LENGTH, "%s:%s", reason, requestor);

   // need to write the reason
   if ((fp = fopen(rebootSystemCallReasonFile, "w+")) == NULL)
   {
      bcmuLog_error("%s: could not open %s", __FUNCTION__, rebootSystemCallReasonFile);
      return BCMRET_INTERNAL_ERROR;
   }

   fprintf(fp, "%s", rebootReason);
   fclose(fp);

   return BCMRET_SUCCESS;
}

/*
 * This function is to get reset_status.
 * Now only isSwReset is used.
 */
static BcmRet bcmUtl_getResetStatus(UINT8 *isSwReset)
{
   const char *filename = "/proc/bootstate/reset_status";
   char buffer[BUFLEN_64];
   FILE *fp = NULL;
   int value;

   if ((fp = fopen(filename, "r")) == NULL)
   {
      bcmuLog_notice("failed to open file %s.", filename);
      return BCMRET_INTERNAL_ERROR;
   }
   
   if (NULL == fgets(buffer, sizeof(buffer), fp))
   {
      bcmuLog_error("Could not read %s", filename);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   value = strtoul(buffer, NULL, 16);

   if ((value & SW_RESET_STATUS) == SW_RESET_STATUS)
   {
       *isSwReset = 1;
   }

   return BCMRET_SUCCESS;
}

/*
 * This function is to get the reason of the board reboot last time.
 */
BcmRet bcmUtl_getLastRebootReason(char *buf, UINT32 bufLen)
{
   const char *filename = "/proc/bootstate/old_reset_reason";
   UINT8 isSwReset = 0;
   char buffer[BUFLEN_64];
   char rebootReasonBuf[REBOOT_REASON_MAX_LENGTH];
   FILE *fp = NULL;
   int value;

   memset(buf, 0, bufLen);

   if (bcmUtl_getResetStatus(&isSwReset) != BCMRET_SUCCESS)
   {
      bcmuLog_error("bcmUtl_getResetStatus failed");
      return BCMRET_INTERNAL_ERROR;
   }

   // if this is not a soft reset, return "Power On"
   // Todo: other hardware reset
   if (!isSwReset)
   {
      snprintf(buf, bufLen, "%s", "Power On");
      return BCMRET_SUCCESS;
   }

   // this is a soft reset, to read reset_reason
   if ((fp = fopen(filename, "r")) == NULL)
   {
      bcmuLog_notice("failed to open file %s.", filename);
      return BCMRET_INTERNAL_ERROR;
   }
   
   if (NULL == fgets(buffer, sizeof(buffer), fp))
   {
      bcmuLog_error("Could not read %s", filename);
      fclose(fp);
      return BCMRET_INTERNAL_ERROR;
   }
   fclose(fp);

   value = strtoul(buffer, NULL, 16);

   // if this is a reboot system call, read reboot system call reason
   // else the reason is set using reset_reason
   switch (value & BCM_BOOT_REASON_MASK)
   {
      case BCM_BOOT_REASON_REBOOT:
         if (bcmUtl_getRebootSystemCallReason(rebootReasonBuf, REBOOT_REASON_MAX_LENGTH)
            == BCMRET_SUCCESS)
         {
            snprintf(buf, bufLen, "%s:%s", "Reboot System Call", rebootReasonBuf);
         }
         else
         {
            snprintf(buf, bufLen, "%s", "Reboot System Call");
         }
         break;
      case BCM_BOOT_REASON_ACTIVATE:
         snprintf(buf, bufLen, "%s", "Software Upgrade");
         break;
      case BCM_BOOT_REASON_PANIC:
         snprintf(buf, bufLen, "%s", "Kernel Panic");
         break;
      case BCM_BOOT_REASON_WATCHDOG:
         snprintf(buf, bufLen, "%s", "Watchdog");
         break;
      default:
         snprintf(buf, bufLen, "%s", "Unknown");
         break;
   }

   return BCMRET_SUCCESS;
}

void bcmUtl_loggedBusybox_reboot(const char *requestor, RebootReasonType reason)
{
   const char *reasonStr;

   if (reason < NUM_OF_ELEMENTS(rebootSystemCallReasonStr))
   {
      reasonStr = rebootSystemCallReasonStr[reason];
   }
   else
   {
      reasonStr = "Unknown";
   }

   bcmUtl_declareShutdownInProgressEx(requestor, reasonStr, BCM_DECLARE_SHUTDOWN_KNOTICE);
   bcmUtl_writeRebootSystemCallReason(requestor, reasonStr);
   bcmUtl_busybox_reboot();
}

