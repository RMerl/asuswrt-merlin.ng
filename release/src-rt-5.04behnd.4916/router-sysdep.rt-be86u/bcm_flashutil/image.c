/***********************************************************************
 * <:copyright-BRCM:2007:DUAL/GPL:standard
 * 
 *    Copyright (c) 2007 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 ************************************************************************/

/*!\file image.c
 * \brief Most or maybe all of the functionality in this file has been
 *        copied to bcm_imgutil.c.  Going forward, make fixes and enhancements
 *        in that file and redirect calls from this file to that file.
 *  
 */
 
#if defined(BRCM_CMS_BUILD) || defined (BRCM_BDK_BUILD) || defined(SUPPORT_OPENPLAT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <dlfcn.h>

#include "cms.h"
#include "bdk.h"
#include "cms_image.h"
#include "cms_psp.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "bcm_boardutils.h"
#include "bcm_timestamp.h"
#include "bcm_ulog.h"
#include "bcmTag.h" /* in shared/opensource/include/bcm963xx, for FILE_TAG */
#include "board.h" /* in bcmdrivers/opensource/include/bcm963xx, for BCM_IMAGE_CFE */

#include "genutil_crc.h"
#include "bcm_flashutil.h"
#include "bcm_imgutil.h"
#include "bcm_imgutil_api.h"



// This is a local helper function which will do a dlopen on libcms_msg.so
// to grab the cmsImg_sendConfigMsg() function pointer.  The args are the same.
// Doing this allows apps which only want to use cms_util for the
// non-config-file related util function to be independent of libcms_msg.so.
// A few other helper functions which used cmsMsg have also been moved to
// userspace/public/libs/cms_msg/addon_*.c.
static CmsRet local_dl_sendConfigMsg(const char *imagePtr, UINT32 imageLen,
                                     void *msgHandle, int doWrite);
CmsRet    (*cmsImg_sendConfigMsg_fn)(const char *imagePtr, UINT32 imageLen,
                                     void *msgHandle, int doWrite);



UINT32 cmsImg_getImageFlashSize(void)
{
   return (bcmImg_getImageFlashSize());
}


UINT32 cmsImg_getBroadcomImageTagSize(void)
{
   return (bcmImg_getBroadcomImageTagSize());
}


UINT32 cmsImg_getConfigFlashSize(void)
{
   return (bcmImg_getConfigFlashSize());
}


UINT32 cmsImg_getRealConfigFlashSize(void)
{
   return (bcmImg_getRealConfigFlashSize());
}


UBOOL8 cmsImg_willFitInFlash(UINT32 imageSize)
{
   return (bcmImg_willFitInFlash(imageSize));
}


UBOOL8 cmsImg_isBackupConfigFlashAvailable(void)
{
   return (bcmImg_isBackupConfigFlashAvailable());
}


UBOOL8 cmsImg_isConfigFileLikely(const char *buf)
{
   return (bcmImg_isConfigFileLikely(buf));
}


/** General entry point for writing the image.
 *  The image can be a flash image or a config file.
 *  This function will first determine the image type, which has the side
 *  effect of validating it.
 */
CmsRet cmsImg_writeImage(char *imagePtr, UINT32 imageLen, void *msgHandle)
{
   CmsImageFormat format;
   CmsRet ret;
   
   if ((format = cmsImg_validateImage(imagePtr, imageLen, msgHandle)) == CMS_IMAGE_FORMAT_INVALID)
   {
      ret = CMSRET_INVALID_IMAGE;
   }
   else
   {
      ret = cmsImg_writeValidatedImage(imagePtr, imageLen, format, msgHandle);
   }

   return ret;
}


CmsRet cmsImg_writeImage_noReboot(char *imagePtr, UINT32 imageLen, void *msgHandle)
{
   CmsImageFormat format;
   CmsRet ret;
   
   if ((format = cmsImg_validateImage(imagePtr, imageLen, msgHandle)) == CMS_IMAGE_FORMAT_INVALID)
   {
      ret = CMSRET_INVALID_IMAGE;
   }
   else
   {
      ret = cmsImg_writeValidatedImage_noReboot(imagePtr, imageLen, format, msgHandle);
   }

   return ret;
}


CmsRet cmsImg_writeValidatedImage(char *imagePtr, UINT32 imageLen,
                                  CmsImageFormat format, void *msgHandle)
{
   if (0xb0 & (UINT32)format)
   {
      /*
       * Prior to 4.14L.01 release, we had some overloaded bits in the
       * format enum.  They have been moved out to CMS_IMAGE_WR_OPT_XXX,
       * but leave some code here to detect old code and usage.
       * If you see this message and you are sure you are using the code
       * correctly, you can ignore this message or delete it.
       */
      bcmuLog_error("suspicious old bits set in format enum, format=%d (0x%x)",
                   format, format);
   }

   return (cmsImg_writeValidatedImageEx(imagePtr, imageLen,
                                        format, msgHandle, 0));
}


CmsRet cmsImg_writeValidatedImage_noReboot(char *imagePtr, UINT32 imageLen,
                                  CmsImageFormat format, void *msgHandle)
{
   if (0xb0 & (UINT32)format)
   {
      /*
       * Prior to 4.14L.01 release, we had some overloaded bits in the
       * format enum.  They have been moved out to CMS_IMAGE_WR_OPT_XXX,
       * but leave some code here to detect old code and usage.
       * If you see this message and you are sure you are using the code
       * correctly, you can ignore this message or delete it.
       */
      bcmuLog_error("suspicious old bits set in format enum, format=%d (0x%x)",
                   format, format);
   }

   return (cmsImg_writeValidatedImageEx(imagePtr, imageLen,
                                        format, msgHandle, CMS_IMAGE_WR_OPT_NO_REBOOT));
}


void cmsImg_reboot(void)
{
   // This is an immediate hardware reset, which causes a reboot.
   // All CMS and BDK apps should use bcmUtl_loggedBusybox_reboot(), except in
   // case of a new config file, where we DO need an immediate hardware reset.
   bcmuLog_knotice("Requesting immediate hardware reset and reboot");
   bcmImg_reboot();
}


CmsRet cmsImg_writeValidatedImageEx(char *imagePtr, UINT32 imageLen,
                                    CmsImageFormat format, void *msgHandle,
                                    UINT32 opts)
{
   CmsRet ret=CMSRET_SUCCESS;
   int noReboot = 0;

   bcmuLog_notice("Entered: format=0x%x opts=0x%x", format, opts);

   switch(format)
   {
   case CMS_IMAGE_FORMAT_BROADCOM:
   case CMS_IMAGE_FORMAT_FLASH:
      if ((ret = bcmImg_flashImage(imagePtr, imageLen, format, opts, &noReboot)) == CMSRET_SUCCESS)
      {
         printf("FirmwareSelect: image written, COMMIT the NEW_IMAGE in partition %s (noReboot=%d)\n",
                ((getBootPartition() == 1) ? "2" : "1"), noReboot);
         setBootImageState(BOOT_SET_NEW_IMAGE);
         if (noReboot == 0)
         {
            bcmUtl_loggedBusybox_reboot(NULL, REBOOT_REASON_SOFTWARE_UPGRADE);
         }
      }
      else
      {
         bcmuLog_error("Image could not be flashed.");
      }
      break;
      
   case CMS_IMAGE_FORMAT_XML_CFG:
      ret = local_dl_sendConfigMsg(imagePtr, imageLen, msgHandle, 1);
      if (ret == CMSRET_SUCCESS)
      {
         /*
          * Emulate the behavior of the driver when a flash image is written.
          * When we write a config image, also request immediate reboot
          * because we don't want to let any other app save the config file
          * to flash, thus wiping out what we just written.
          */
         bcmuLog_debug("config file download written, request reboot");
         cmsImg_reboot();
      }
      break;

   default:
       bcmuLog_error("Unrecognized image format=%d", format);
       ret = CMSRET_INVALID_IMAGE;
       break;
    }
   
   return ret;
}


#define INCR_WR_BUFLEN  (256*1024)

CmsRet cmsImg_writeImageIncremental(const char *buf, UINT32 bufLen)
{
   UINT32 currChunk;
   UINT32 rcTotal=0;
   UINT32 i=0;
   UINT32 remain=bufLen;
   CmsRet ret=CMSRET_SUCCESS;
   int rc;
   IMGUTIL_HANDLE imgUtilHandle = NULL;
   imgutil_open_parms_t openParams;

   // Initialize BCM flashutil incremental writing context
   memset(&openParams, 0x0, sizeof(imgutil_open_parms_t));
   openParams.options = CMS_IMAGE_WR_OPT_NO_REBOOT;
   imgUtilHandle = img_util_open(&openParams);
   if (imgUtilHandle == IMGUTIL_HANDLE_INVALID)
   {
      bcmuLog_error("img_util_open failed");
      return CMSRET_INTERNAL_ERROR;
   }

   // We already have the entire image in memory, so just feed it to the
   // incremental write interface in chunks.
   while (i < bufLen)
   {
      currChunk = (remain < INCR_WR_BUFLEN) ? remain : INCR_WR_BUFLEN;
      rc = img_util_write(imgUtilHandle, (UINT8*)(&(buf[i])), currChunk);
      if (rc < 0)
      {
         bcmuLog_error("Image write failed at i=%d, rc=%d", i, rc);
         img_util_abort(imgUtilHandle);
         return CMSRET_INTERNAL_ERROR;
      }
      else if (rc != (int)currChunk)
      {
         // Shouldn't happen, but don't make it fatal either.
         bcmuLog_error("unexpected rc at [%d] chunk=%d rc=%d",
                      i, currChunk, rc);
      }

      i += currChunk;
      rcTotal += currChunk;
      remain -= currChunk;
   }

   bcmuLog_debug("write finished: bufLen=%d totalRead=%d rcTotal=%d remain=%d",
                bufLen, i, rcTotal, remain);

   rc = img_util_close(imgUtilHandle, NULL, NULL);
   if (rc != 0)
   {
      bcmuLog_error("img_util_close failed, rc=%d", rc);
      ret = CMSRET_INTERNAL_ERROR;
   }

   bcmuLog_debug("exit ret=%d", ret);
   return ret;
}


CmsImageFormat cmsImg_validateImage(const char *imageBuf, UINT32 imageLen, void *msgHandle)
{
   bcmuLog_notice("Entered: imageBuf=%p len=%d (msgHandle=%p)",
                 imageBuf, imageLen, msgHandle);

   if (imageBuf == NULL)
   {
      bcmuLog_error("imageBuf is NULL");
      return CMS_IMAGE_FORMAT_INVALID;
   }
   
   if ((imageLen > CMS_CONFIG_FILE_DETECTION_LENGTH) &&
       cmsImg_isConfigFileLikely(imageBuf))
   {
      CmsRet ret;

      bcmuLog_debug("possible CMS XML config file format detected");
      if (msgHandle == NULL)
      {
         // cmsImg_isConfigFileLikely will detect the config file strings,
         // so we can be pretty sure it is a config file.  However, since 
         // there is no msgHandle, we cannot validate it.  Caller is
         // responsible for validating (in BDK, there is a MDM_OP for
         // validating distributed config files.)
         printf("possible CMS/BDK XML config file detected, but msgHandle is NULL, so assume config file.\n");
         return CMS_IMAGE_FORMAT_XML_CFG;
      }
      else
      {
         ret = local_dl_sendConfigMsg(imageBuf, imageLen, msgHandle, 0);
         if (ret == CMSRET_SUCCESS)
         {
            bcmuLog_debug("CMS XML config format verified.");
            return CMS_IMAGE_FORMAT_XML_CFG;
         }
         else
         {
            // Not clear what should happen in this case:
            // Is the error due to just the sending of the config file msg?  (Internal error)
            // Or is the config file not valid? (invalid config file).
            // Since we are pretty sure this is a config file, just return
            // some error.
            bcmuLog_error("unable to validate config file, ret=%d", ret);
            return CMS_IMAGE_FORMAT_INVALID;
         }
      }
   }

   bcmuLog_debug("not a config file");
   // At this point, we know this is a firmware image file.  Call bcm_flashutil
   // to detect format and validate.
   return (bcmImg_validateImage(imageBuf, imageLen));
}


CmsRet local_dl_sendConfigMsg(const char *imagePtr, UINT32 imageLen,
                              void *msgHandle, int doWrite)
{
   void *handle;
   CmsRet ret = CMSRET_INTERNAL_ERROR;

   bcmuLog_notice("Entered: attempting to find cmsImg_sendConfigMsg in libcms_msg.so");
   bcmuLog_debug("imagePtr=%p imageLen=%d msgHandle=%p doWrite=%d",
                 imagePtr, imageLen, msgHandle, doWrite);

   // We open and grab the function pointer on every use, although we could
   // be more clever and cache the result.  These config file operations are
   // rare, and usually ends with a reboot.
   handle = dlopen ("libcms_msg.so", RTLD_LAZY);
   if (handle != NULL)
   {
      cmsImg_sendConfigMsg_fn = dlsym(handle, "cmsImg_sendConfigMsg");
      if (cmsImg_sendConfigMsg_fn != NULL)
      {
         ret = (*cmsImg_sendConfigMsg_fn)(imagePtr, imageLen, msgHandle, doWrite);
      }
      else
      {
         bcmuLog_error("Coud not find cmsImg_sendConfigMsg in libcms_msg.so");
      }
      cmsImg_sendConfigMsg_fn = NULL;
      dlclose(handle);
   }
   else
   {
      bcmuLog_error("Could not find libcms_msg.so, config file functions not supported.");
   }

   return ret;
}


void cmsImg_sendLoadStartingMsg(void *msgHandle __attribute__((unused)),
                                const char *connIfName __attribute__((unused)))
{
   // On modern systems which have more memory and memory efficient image util
   // API's, no long need to do anything.
   return;
}


void cmsImg_sendLoadDoneMsg(void *msgHandle __attribute__((unused)))
{
   // On modern systems which have more memory and memory efficient image util
   // API's, no long need to do anything.
   return;
}


UBOOL8 cmsImg_isBcmTaggedImage(const char *imageBuf, UINT32 *imageSize)
{
   return (bcmImg_isBcmTaggedImage(imageBuf, imageSize));
}

CmsImageFormat cmsImg_parseImgHdr(UINT8 *bufP, UINT32 bufLen)
{
   return (bcmImg_parseImgHdr(bufP, bufLen));
}

/* This function retrieves software version number from an image version string
 * which starts with software version followed by image type, build number and
 * build time info.
 * e.g. given the image version string "5041pre1GW_BEEP_BEEPREINSTALL2131745",
 * the output software version number will be "5.04L.01pre1".
 *
 * The parser is based on the following assumptions:
 * (1) Software version string is made up of digits and/or lowercase letters,
 *    e.g. "5041pre1", and is terminated by the uppercase image type letter,
 *    e.g. G, O, H, B, D.
 * (2) The first single digit is the major version number, e.g. "5"
 * (3) The second and third digits are the minor version number, e.g. "04"
 * (4) The remaining characters are the build number suffix, e.g. "1pre1"
 */
void cmsImg_parseImageVersionForSwVersion(const char *imageVersion,
                                          char *swVersion, SINT32 len)
{
   char buf[BUFLEN_256+1] = {0};
   SINT32 length, i, j;

   length = strlen(imageVersion);

   for (i = 0, j = 0; i < length; i++)
   {
      /* Software version string is terminated by the uppercase image type
       * letter.
       */
      if (isupper(imageVersion[i]))
         break;

      buf[j++] = imageVersion[i];
      if (i == 0)
      {
         buf[j++] = '.';   /* major version delimiter */
      }
      else if (i == 2)
      {
         buf[j++] = 'L';   /* L for linux */
         buf[j++] = '.';   /* minor version number delimiter */

         /* The forth character shall be the build number. If the fifth
          * character is also a digit, then the forth and the fifth
          * characters are the build number. Otherwise, insert '0'
          * before the forth character to make the two-digit build number.
          */
         if (!isdigit(imageVersion[4]))
            buf[j++] = '0';
      }
   }
   buf[j] = '\0';

   strncpy(swVersion, buf, len-1);

   bcmuLog_debug("swVersion=%s", swVersion);
}

CmsRet cmsImg_getIdent(SINT32 part, char *key, char *value, SINT32 len)
{
   char keystr[BUFLEN_256+1] = {0};
   char *p1, *p2;
   int start = 0, end = 50;

   *value = '\0';

   bcmuLog_debug("Enter: part=%d key=%s", part, key);

   if ((key == NULL) || (*key == '\0'))
   {
      bcmuLog_error("key not specified");
      return CMSRET_INVALID_ARGUMENTS;
   }

   if (bcmFlash_getIdent(part, &start, &end, key, keystr, sizeof(keystr)-1) <= 0)
   {
      bcmuLog_notice("key %s not found", key);
      return CMSRET_INVALID_ARGUMENTS;
   }
   bcmuLog_debug("keystr=%s", keystr);

   /* e.g. key="imageversion" --> keystr="$imageversion: 5041GW_PURE1812211109 $"
    *      key="imgversion"   --> keystr="$imgversion: 5.04L.01 $"
    */
   if ((p1 = strchr(keystr, ':')) == NULL)
   {
      bcmuLog_error("Invalid key string %s", keystr);
      return CMSRET_INTERNAL_ERROR;
   }

   p1++;
   while (*p1 != '\0' && isspace(*p1))
      p1++;

   p2 = p1;
   while (*p2 != '\0' && *p2 != '$' && !isspace(*p2))
      p2++;
   *p2 = '\0';

   strncpy(value, p1, len-1);

   bcmuLog_debug("value=%s", value);

   return CMSRET_SUCCESS;
}

CmsRet cmsImg_getFirmwareNameVersion(SINT32 part,
                                     char *fullVersionBuf,
                                     char *shortVersionBuf)
{
   CmsRet ret;

   if (fullVersionBuf == NULL)
   {
      bcmuLog_error("fullVersionBuf must be provided");
      return CMSRET_INVALID_ARGUMENTS;
   }

   ret = cmsImg_getIdent(part, "imageversion", fullVersionBuf, IMAGE_VERSION_TAG_SIZE);
   if (ret != CMSRET_SUCCESS)
   {
      return ret;
   }

   // If not provided, then we are done.
   if (shortVersionBuf == NULL)
      return ret;

   cmsImg_getIdent(part, "imgversion", shortVersionBuf, IMAGE_VERSION_TAG_SIZE);
   if ((shortVersionBuf == NULL) || (*shortVersionBuf == '\0'))
   {
      /* "imgversion" key may not exist in older version of firmware image.
       * As a workaround, parse for software version from the "imageversion"
       * key string.
       */  
      cmsImg_parseImageVersionForSwVersion(fullVersionBuf,
                                   shortVersionBuf, IMAGE_VERSION_TAG_SIZE);
   }

   return ret;
}


void cmsImg_writeBootFirmwareImageHint(const char *appName, UINT32 part)
{
   char buf[sizeof(CmsImageBootFirmwareImageHint)+100]={0};  //+100 for various separator chars
   CmsImageBootFirmwareImageHint hint;
   CmsRet ret;

   if ((appName == NULL) || (*appName == '\0'))
   {
      bcmuLog_error("NULL or empty appName");
      return;
   }

   if ((part != 1) && (part != 2))
   {
      bcmuLog_error("Invalid partition number %d, must be 1 or 2", part);
      return;
   }

   memset(&hint, 0, sizeof(hint));
   snprintf(hint.appName, sizeof(hint.appName), "%s", appName);
   bcmTms_getISO8601DateTimeGmt(0, hint.timestamp, sizeof(hint.timestamp));

   snprintf(buf, sizeof(buf), "app=%s;part=%d;ts=%s",
            hint.appName, part, hint.timestamp);

   bcmuLog_notice("writing %s=%s", BOOT_FIRMWARE_IMAGE_KEY, buf);

   ret = cmsPsp_set(BOOT_FIRMWARE_IMAGE_KEY, buf, strlen(buf)+1);
   if (ret != CMSRET_SUCCESS)
   {
      bcmuLog_error("write of %s:%s to scratchpad failed, ret=%d",
                   BOOT_FIRMWARE_IMAGE_KEY, buf, ret);
   }

   return;
}

CmsRet cmsImg_getBootFirmwareImageHint(CmsImageBootFirmwareImageHint *bootFwHint)
{
   char buf[1024]={0};
   char *ptr = NULL;
   int i, j, rc;

   if (bootFwHint == NULL)
   {
      return CMSRET_INVALID_ARGUMENTS;
   }
   memset(bootFwHint, 0, sizeof(CmsImageBootFirmwareImageHint));

   rc = cmsPsp_get(BOOT_FIRMWARE_IMAGE_KEY, buf, sizeof(buf));
   if (rc == 0)
   {
      // key not found, just return
      return CMSRET_NO_MORE_INSTANCES;
   }
   else if (rc < 0)
   {
      bcmuLog_error("value too long, need %d", (rc * -1));
      return CMSRET_RESOURCE_EXCEEDED;
   }

   if (strncmp(buf, "app=", 4))
   {
      bcmuLog_error("bad format, could not find app= in %s", buf);
      return CMSRET_INTERNAL_ERROR;
   }
   for (i=4, j=0; (i < rc) && (j < (int)(sizeof(bootFwHint->appName)-1)) && (buf[i] != ';'); i++, j++)
   {
      bootFwHint->appName[j] = buf[i];
   }

   ptr = strstr(buf, "part=");
   if (ptr == NULL)
   {
      bcmuLog_error("bad format, could not find part= in %s", buf);
      return CMSRET_INTERNAL_ERROR;
   }
   bootFwHint->part = ptr[5] - '0';

   ptr = strstr(buf, "ts=");
   if (ptr == NULL)
   {
      bcmuLog_error("bad format, could not find ts= in %s", buf);
      return CMSRET_INTERNAL_ERROR;
   }
   for (i=3, j=0; (ptr+i < buf+rc) && (j < (int)(sizeof(bootFwHint->timestamp)-1)); i++, j++)
   {
      bootFwHint->timestamp[j] = ptr[i];
   }

   return CMSRET_SUCCESS;
}

void cmsImg_deleteBootFirmwareImageHint(void)
{
   bcmuLog_notice("delete %s", BOOT_FIRMWARE_IMAGE_KEY);
   // Length of 0 will delete the key.
   cmsPsp_set(BOOT_FIRMWARE_IMAGE_KEY, NULL, 0);

   return;
}



#ifdef SUPPORT_TR69C_AUTONOMOUS_TRANSFER_COMPLETE

CmsRet cmsImg_storeImageTransferStats(const CmsImageTransferStats *pStats)
{
   CmsRet ret = CMSRET_SUCCESS;

   if (pStats == NULL)
   {
      ret = cmsPsp_set(CMS_IMAGE_TRANSFER_STATS_PSP_KEY, NULL, 0);
   }
   else if ((ret = cmsPsp_set(CMS_IMAGE_TRANSFER_STATS_PSP_KEY, pStats, sizeof(CmsImageTransferStats))) != CMSRET_SUCCESS)
   {
      bcmuLog_error("set of CMS_IMAGE_TRANSFER_STATS_PSP_KEY failed, ret=%d", ret);
   }
   return (ret);
}

CmsRet cmsImg_clearImageTransferStats(void)
{
   return(cmsPsp_set(CMS_IMAGE_TRANSFER_STATS_PSP_KEY, NULL, 0));
}

CmsRet cmsImg_getImageTransferStats(CmsImageTransferStats *pStats)
{
   int byteRead = 0;
   CmsRet ret = CMSRET_INTERNAL_ERROR;

   if (pStats != NULL)
   {
      byteRead = cmsPsp_get(CMS_IMAGE_TRANSFER_STATS_PSP_KEY, pStats, sizeof(CmsImageTransferStats));
   }
   if (byteRead > 0)
   {
      ret = CMSRET_SUCCESS;
   }
   return (ret);
}


#endif /* SUPPORT_TR69C_AUTONOMOUS_TRANSFER_COMPLETE */
#endif /* BRCM_CMS_BUILD || BRCM_BDK_BUILD  */
