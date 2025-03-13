/***********************************************************************
 * <:copyright-BRCM:2016:DUAL/GPL:standard
 * 
 *    Copyright (c) 2016 Broadcom 
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

/*!\file bcm_imgutil.c
 * \brief Broadcom firmware image utils.  Most of this file is copied from
 *        cms_util/image.c   This file is the correct place for most
 *        Broadcom image utils, since the image functions applies to 
 *        non-CMS builds as well.   Going forward, make fixes and enhancements
 *        in this file and redirect calls from image.c to this file.
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#ifdef DESKTOP_LINUX
#include <arpa/inet.h>
#endif /* DESKTOP_LINUX */

#include "bcmTag.h"
#include "board.h"
#include "bcm_boarddriverctl.h"

#include "genutil_crc.h"
#include "bcm_flashutil.h"
#include "bcm_flashutil_nand.h"
#include "bcm_flashutil_emmc.h"
#include "bcm_flashutil_nor.h"
#include "bcm_imgutil.h"
#include "libfdt.h"

/* #define CC_IMGUTIL_DEBUG 1 */
#define cmsLog_error(fmt, arg...) \
  printf("ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#if defined(CC_IMGUTIL_DEBUG)
#define cmsLog_debug(fmt, arg...) \
  printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
  #define cmsLog_debug(fmt, arg...)
#endif /* CC_IMGUTIL_DEBUG */
#if defined(CC_IMGUTIL_NOTICE)
#define cmsLog_notice(fmt, arg...) \
  printf("%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
  #define cmsLog_notice(fmt, arg...)
#endif /* CC_IMGUTIL_NOTICE */


static UBOOL8 matchChipId(const char *strTagChipId, const char *signature2);
CmsRet bcmImg_verifyBroadcomFileTag(FILE_TAG *pTag, UBOOL8 fullImageB, int imageLen);
void dumpBcmFileTag(FILE_TAG *tagP);


UBOOL8 bcmImg_MatchChipId( const char *strTagChipId )
{
    return(matchChipId(strTagChipId, NULL));
}

/**
 * @return TRUE if the modem's chip id matches that of the image.
 */
static UBOOL8 matchChipId(const char *strTagChipId, const char *signature2 __attribute__((unused)))
{
    UINT32 tagChipId = 0;
    UINT32 chipId;
    UBOOL8 match;

    /* this is the image's chip id */
    tagChipId = (UINT32) strtol(strTagChipId, NULL, 16);

    /* get the system's chip id */
    devCtl_getChipId(&chipId);
    if (tagChipId == chipId)
    {
        match = TRUE;
    }
    else if (tagChipId == CHIP_FAMILY_ID_HEX)
    {
        match = TRUE;
    }
    else
    {
        cmsLog_error("Chip Id error.  Image Chip Id = %04x, Board Chip Id = %04x.", tagChipId, chipId);
        match = FALSE;
#ifdef DESKTOP_LINUX
        match = TRUE; /* For debugging. */
#endif
    }

    return match;
}

// verify the tag of the image
CmsRet bcmImg_verifyBroadcomFileTag(FILE_TAG *pTag, UBOOL8 fullImageB, int imageLen)
{
    UINT32 crc;
    int totalImageSize;
    int tagVer, curVer;
    UINT32 tokenCrc, imageCrc, *pTokenCrc, *pImageCrc;


    cmsLog_debug("start of pTag=%p tagversion %02x %02x %02x %02x", pTag,
                  pTag->tagVersion[0],
                  pTag->tagVersion[1],
                  pTag->tagVersion[2],
                  pTag->tagVersion[3]);

    pTokenCrc = (UINT32 *)pTag->tagValidationToken;
    tokenCrc = *pTokenCrc;
    pImageCrc = (UINT32 *)pTag->imageValidationToken;
    imageCrc = *pImageCrc;
#ifdef DESKTOP_LINUX
    /* assume desktop is running on little endien intel, but the CRC has been
     * written for big endien mips, so swap before compare.
     * FIXME! this part of code might not work with Little-endian target system.
     */
    tokenCrc = htonl(tokenCrc);
    imageCrc = htonl(imageCrc);
#endif

    // check tag validate token first
    crc = CRC_INITIAL_VALUE;
    crc = genUtl_getCrc32((UINT8 *) pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);
    if (crc != tokenCrc)
    {
        /* this function is called even when we are not sure the image is
         * a broadcom image.  So if crc fails, it is not a big deal.  It just
         * means this is not a broadcom image.
         */
        cmsLog_debug("token crc failed, this is not a valid broadcom image");
        cmsLog_debug("calculated crc=0x%x tokenCrc=0x%x", crc, tokenCrc);
        return CMSRET_INVALID_IMAGE;
    }
    cmsLog_debug("header CRC is OK.");

    tagVer = atoi((char *) pTag->tagVersion);
    curVer = atoi(BCM_TAG_VER);

    if (tagVer != curVer)
    {
       cmsLog_error("Firmware tag version [%d] is not compatible with the current Tag version [%d]", tagVer, curVer);
       return CMSRET_INVALID_IMAGE;
    }

    cmsLog_debug("tarVer=%d, curVar=%d", tagVer, curVer);

    if (!matchChipId((char *) pTag->chipId, (char *) pTag->signiture_2))
    {
       cmsLog_error("chipid check failed");
       return CMSRET_INVALID_IMAGE;
    }

    cmsLog_debug("chipid is OK");

    if (fullImageB == FALSE)
    {
       cmsLog_debug("Good broadcom image header");
       return CMSRET_SUCCESS;
    }

    // check imageCrc
    totalImageSize = atoi((char *) pTag->totalImageLen);
    cmsLog_debug("totalImageLen=%d, imageLen=%d, TAG_LEN=%d\n", totalImageSize, imageLen, TAG_LEN);

    if (totalImageSize > (imageLen -TAG_LEN))
    {
        cmsLog_error("invalid size\n");
        return CMSRET_INVALID_IMAGE;
    }
    crc = CRC_INITIAL_VALUE;
    crc = genUtl_getCrc32(((UINT8 *)pTag + TAG_LEN), (UINT32) totalImageSize, crc);
    if (crc != imageCrc)
    {
        /*
         * This should not happen.  We already passed the crc check on the header,
         * so we should pass the crc check on the image.  If this fails, something
         * is wrong.
         */
        cmsLog_error("image crc failed after broadcom header crc succeeded");
        cmsLog_error("calculated crc=0x%x imageCrc=0x%x totalImageSize=%d", crc, imageCrc, totalImageSize);
        return CMSRET_INVALID_IMAGE;
    }
    cmsLog_debug("image crc is OK");

    cmsLog_debug("Good broadcom image");

    return CMSRET_SUCCESS;
}


// depending on the image type, do the brcm image or whole flash image
CmsRet bcmImg_flashImage(char *imagePtr, UINT32 imageLen, CmsImageFormat format, UINT32 opts, int * noReboot)
{
    FILE_TAG *pTag = (FILE_TAG *) imagePtr;
    int cfeSize, rootfsSize, kernelSize;
    unsigned long cfeAddr, rootfsAddr, kernelAddr;
    UINT32 flags=0;
    CmsRet ret;

    if( opts & CMS_IMAGE_WR_OPT_PART1 )
    {
        *noReboot = (opts & CMS_IMAGE_WR_OPT_NO_REBOOT) ? FLASH_PART1_NO_REBOOT : FLASH_PART1_REBOOT;
    }
    else if( opts & CMS_IMAGE_WR_OPT_PART2 )
    {
        *noReboot = (opts & CMS_IMAGE_WR_OPT_NO_REBOOT) ? FLASH_PART2_NO_REBOOT : FLASH_PART2_REBOOT;
    }
    else
    {
        *noReboot = (opts & CMS_IMAGE_WR_OPT_NO_REBOOT) ? FLASH_PARTDEFAULT_NO_REBOOT : FLASH_PARTDEFAULT_REBOOT;
    }

    cmsLog_debug("format=%d noReboot=0x%x", format, *noReboot);

    if (format != CMS_IMAGE_FORMAT_FLASH && format != CMS_IMAGE_FORMAT_BROADCOM)
    {
       cmsLog_error("invalid image format %d", format);
       return CMSRET_INVALID_IMAGE;
    }

    if (format == CMS_IMAGE_FORMAT_FLASH)
    {
        cmsLog_notice("Flash whole image...");
        // Pass zero for the base address of whole image flash. It will be filled by kernel code
        // was sysFlashImageSet
        if ((ret = getFlashInfo(&flags)) == CMSRET_SUCCESS)
        {
           if(flags & FLASH_INFO_FLAG_NOR)
           { // NOR flash device
              ret = devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE,
                                      BCM_IMAGE_WHOLE,
                                      imagePtr,
                                      imageLen-TOKEN_LEN,
                                      *noReboot, 0);
           }
           else
           { // NAND flash device
              ret = writeImageToNand( (unsigned char *)imagePtr, imageLen - TOKEN_LEN );
           }
        }

        if (ret != CMSRET_SUCCESS)
        {
           cmsLog_error("Failed to flash whole image");
           return CMSRET_IMAGE_FLASH_FAILED;
        }
        else
        {
           return CMSRET_SUCCESS;
        }
    }

    /* this must be a broadcom format image */
    // check imageCrc
    cfeSize = rootfsSize = kernelSize = 0;

    // check cfe's existence
    cfeAddr = (unsigned long) strtoul((char *) pTag->cfeAddress, NULL, 10);
    cfeSize = atoi((char *) pTag->cfeLen);
    // check kernel existence
    kernelAddr = (unsigned long) strtoul((char *) pTag->kernelAddress, NULL, 10);
    kernelSize = atoi((char *) pTag->kernelLen);
    // check root filesystem existence
    rootfsAddr = (unsigned long) strtoul((char *) pTag->rootfsAddress, NULL, 10);
    rootfsSize = atoi((char *) pTag->rootfsLen);
    cmsLog_debug("cfeSize=%d kernelSize=%d rootfsSize=%d", cfeSize, kernelSize, rootfsSize);

    if (cfeAddr)
    {
        printf("Flashing CFE...\n");

        ret = devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE,
                                BCM_IMAGE_CFE,
                                imagePtr+TAG_LEN,
                                cfeSize,
                                (int) cfeAddr, 0);
        if (ret != CMSRET_SUCCESS)
        {
            cmsLog_error("Failed to flash CFE");
            return CMSRET_IMAGE_FLASH_FAILED;
        }
    }

    if (rootfsAddr && kernelAddr)
    {
        char *tagFs = imagePtr;

        // tag is alway at the sector start of fs
        if (cfeAddr)
        {
            tagFs = imagePtr + cfeSize;       // will trash cfe memory, but cfe is already flashed
            memcpy(tagFs, imagePtr, TAG_LEN);
        }

        printf("Flashing root file system and kernel...\n");
        /* only the buf pointer and length is needed, the offset parameter
         * was present in the legacy code, but is not used. */
        ret = devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE,
                                BCM_IMAGE_FS,
                                tagFs,
                                TAG_LEN+rootfsSize+kernelSize,
                                *noReboot, 0);
        if (ret != CMSRET_SUCCESS)
        {
            cmsLog_error("Failed to flash root file system and kernel");
            return CMSRET_IMAGE_FLASH_FAILED;
        }
    }

    cmsLog_notice("Image flash done.");

    return CMSRET_SUCCESS;
}


CmsRet get_mtd_device_size(const char * partition, unsigned int * size)
{
    FILE *fp;
    char sze[MAX_MTD_NAME_SIZE]     = {0};
    char line[MAX_MTD_NAME_SIZE*2]  = {0};
    char name[MAX_MTD_NAME_SIZE]    = {0};
    char compare[MAX_MTD_NAME_SIZE] = {0};

    if ( (fp = fopen("/proc/mtd","r")) == 0)
    {
        printf("ERROR in %s!!! Could not open /proc/mtd\n", __FUNCTION__);

        return(CMSRET_OPEN_FILE_ERROR);
    }

    snprintf(compare, sizeof(compare), "%s%s%s", "\"", partition, "\"");

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%*s %s %*s %s", sze, name);

        if(!strcmp(name, compare))
        {
            fclose(fp);

            *size = (unsigned int)strtol(sze, NULL, 16);

            return(CMSRET_SUCCESS);
        }
    }

    fclose(fp);

    printf("%s: MTD partition/device %s not found/opened\n", __FUNCTION__, partition);
    return(CMSRET_OBJECT_NOT_FOUND);
}


unsigned int bcmImg_getImageFlashSize(void)
{
   UINT32 flashSize=0;
   CmsRet ret = CMSRET_OBJECT_NOT_FOUND;

   if(isLegacyFlashLayout())
   {
      ret = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ,
                              FLASH_SIZE,
                              0, 0, 0, &flashSize);
   }
   else
   {
      unsigned int gFlashInfo = 0;

      getFlashInfo(&gFlashInfo);

      if (gFlashInfo == FLASH_INFO_FLAG_NAND)
      {
         ret = get_mtd_device_size("image", &flashSize);
      }
      else if (gFlashInfo == FLASH_INFO_FLAG_EMMC)
      {
#ifndef DESKTOP_LINUX
         flashSize = emmcGetAvailLoaderSpace() + 2 * emmcGetAvailImgSpace(1);
#else
         flashSize = 0x1000000;
#endif
         ret = CMSRET_SUCCESS;
      }
      else if (gFlashInfo == FLASH_INFO_FLAG_NOR) 
            ret = get_mtd_device_size(SPI_NOR_FLASH_NAME, &flashSize);
   } 	  

   if (ret != CMSRET_SUCCESS)
   {
      cmsLog_error("Could not get flash size, return 0");
      flashSize = 0;
   }

   return flashSize;
}


unsigned int bcmImg_getBroadcomImageTagSize(void)
{
   return TOKEN_LEN;
}


unsigned int bcmImg_getConfigFlashSize(void)
{
   UINT32 realSize;

   realSize = bcmImg_getRealConfigFlashSize();

#ifdef COMPRESSED_CONFIG_FILE
   /*
    * A multiplier of 2 is now too small for some of the big voice and WLAN configs,
    * so allow for the possibility of 4x compression ratio.  In a recent test on the
    * 6368 with wireless enabled, I got a compression ratio of 3.5.
    * The real test comes in management.c: after we do the
    * compression, writeValidatedConfigBuf will verify that the compressed buffer can
    * fit into the flash.
    * A 4x multiplier should be OK for small memory systems such as the 6338.
    * The kernel does not allocate physical pages until they are touched.
    * However, allocating an overly large buffer could be rejected immediately by the
    * kernel because it does not know we don't actually plan to use the entire buffer.
    * So if this is a problem on the 6338, we could improve this algorithm to
    * use a smaller factor on low memory systems.
    */
   realSize = realSize * 4;
#endif

   return realSize;
}


unsigned int bcmImg_getRealConfigFlashSize(void)
{
   CmsRet ret;
   UINT32 size=0;
   UINT32 flags=0;

   if ((ret = getFlashInfo(&flags)) == CMSRET_SUCCESS)
   {
      if(flags & FLASH_INFO_FLAG_NOR)
      {
#ifndef DESKTOP_LINUX
         if( norIsNewFlashLayout())
            size=128*1024;
         else
#endif
         {
            ret = devCtl_boardIoctl(BOARD_IOCTL_GET_PSI_SIZE, 0, NULL, 0, 0, (void *)&size);
            if (ret != CMSRET_SUCCESS)
            {
               cmsLog_error("boardIoctl to get config flash size failed.");
               size = 0;
            }
         }	 
      }
      else
      {
         /*
          * If we are using NAND, config file is stored in the /data partition,
          * so config file size is only limited by the size of the /data
          * partition.  For now, implement a quick solution and hardcode the
          * NAND config file size to 128KB (so far, nothing has hit this limit).
          * In the future, could improve this solution by:
          * - checking against size of /data partition
          * - if we have 1 copy or 2 (for backup config file).
          *   whether we are doing a distributed MDM config (done by BDK)
          * - reserve the space on the FS so there is no chance FS is too
          *   full to accept the config file.
          */
         size=128*1024;  // 128KB
      }
   }
   else
   {
      cmsLog_error("could not determine NAND or NOR");
   }

   cmsLog_debug("returning size=%d", size);

   return size;
}


unsigned char bcmImg_willFitInFlash(unsigned int imageSize)
{
   UINT32 flashSize;

#ifdef DESKTOP_LINUX
   flashSize = 2 * (imageSize + CMS_IMAGE_OVERHEAD);
#else
   flashSize = bcmImg_getImageFlashSize();

   cmsLog_debug("flash size is %u bytes, imageSize=%u bytes", flashSize, imageSize);
#endif /* DESKTOP_LINUX */

   return (flashSize > (imageSize + CMS_IMAGE_OVERHEAD));
}


unsigned char bcmImg_isBackupConfigFlashAvailable(void)
{
   static UBOOL8 firstTime=TRUE;
   static UBOOL8 avail=FALSE;
   CmsRet ret;

   if (firstTime)
   {
      UINT32 size=0;
      UINT32 flags=0;

      if ((ret = getFlashInfo(&flags)) == CMSRET_SUCCESS)
      {
         if(flags & FLASH_INFO_FLAG_NOR)
         {
#ifndef DESKTOP_LINUX
            if( norIsNewFlashLayout() )
                avail = TRUE;
            else
#endif
            {
                ret = devCtl_boardIoctl(BOARD_IOCTL_GET_BACKUP_PSI_SIZE, 0, NULL,
                                        0, 0, (void *)&size);
                if (ret == CMSRET_SUCCESS)
                {
                    avail = TRUE;
                }
            }
         }
         else
         {
            // Backup PSI file is always available in NAND
            avail = TRUE;
         }
      }
      else
      {
         cmsLog_error("could not determine NAND or NOR");
      }

      firstTime = FALSE;
   }

   return avail;
}


unsigned char bcmImg_isConfigFileLikely(const char *buf)
{
   const char *header = "<?xml version";
   const char *dslCpeConfig = "<DslCpeConfig";
   UINT32 len, i;
   UBOOL8 likely=FALSE;

   if (strncmp(buf, "<?xml version", strlen(header)) == 0)
   {
      len = strlen(dslCpeConfig);
      for (i=20; i<50 && !likely; i++)
      {
         if (strncmp(&(buf[i]), dslCpeConfig, len) == 0)
         {
            likely = TRUE;
         }
      }
   }

   cmsLog_debug("returning likely=%d", likely);

   return likely;
}


void bcmImg_reboot(void)
{
    devCtl_boardIoctl(BOARD_IOCTL_MIPS_SOFT_RESET, 0, NULL, 0, 0, 0);
}


imgutil_ret_e bcmImg_getImageVersion(char *imagePtr, int imageLen, char *imageName, int imageNameLen)
{
    int ret;

    ret = getImageVersion((uint8_t*)imagePtr, imageLen, imageName, imageNameLen);
    if (ret != 0)
    {
        return IMGUTIL_PROC_INVALID_IMG;
    }

    return IMGUTIL_PROC_OK;
}


CmsImageFormat bcmImg_validateImage(const char *imageBuf, UINT32 imageLen)
{
   CmsImageFormat result = CMS_IMAGE_FORMAT_INVALID;

   if (imageBuf == NULL)
   {
      cmsLog_error("FORMAT_INVALID");
      return CMS_IMAGE_FORMAT_INVALID;
   }

   if ((imageLen > sizeof(struct fdt_header)) && (fdt_check_header(imageBuf) == 0))
   {
      /* Found a valid Broadcom defined TAG record at the beginning of the image */
      cmsLog_debug("Broadcom pktb format verified.");

      //FIXME: Cannot detemine total size of image at this point since we may not 
      //have the entire dtb in memory. Will do that later
      result = CMS_IMAGE_FORMAT_BROADCOM;
   }
   else if ((imageLen > sizeof(FILE_TAG)) && (bcmImg_verifyBroadcomFileTag((FILE_TAG *) imageBuf, 1, imageLen) == CMSRET_SUCCESS))
   {
      UINT32 maxLen;

      /* Found a valid Broadcom defined TAG record at the beginning of the image */
      cmsLog_debug("Broadcom format verified.");
      maxLen = bcmImg_getImageFlashSize() + bcmImg_getBroadcomImageTagSize();
      if (imageLen > maxLen)
      {
         cmsLog_error("broadcom image is too large for flash, got %u, max %u", imageLen, maxLen);
      }
      else
      {
         result = CMS_IMAGE_FORMAT_BROADCOM;
      }
   }
   else
   {
      /* if It is not a Broadcom flash format file.  Now check if it is a
       * flash image format file.  A flash image format file must have a
       * CRC at the end of the image.
       */
      UINT32 crc = CRC_INITIAL_VALUE;
      UINT32 imageCrc;
      UINT8 *crcPtr;

      if (imageLen > TOKEN_LEN)
      {
         crcPtr = (UINT8 *) (imageBuf + (imageLen - TOKEN_LEN));
         /*
          * CRC may not be word aligned, so extract the bytes out one-by-one.
          * Whole image CRC is calculated, then htonl, then written out using
          * fwrite (see addvtoken.c in hostTools).  Because of the way we are
          * extracting the CRC here, we don't have to swap for endieness when
          * doing compares on desktop Linux and modem (?).
          */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
         imageCrc = crcPtr[0] | (crcPtr[1] << 8) | (crcPtr[2] << 16) | (crcPtr[3] << 24);
#else
         imageCrc = (crcPtr[0] << 24) | (crcPtr[1] << 16) | (crcPtr[2] << 8) | crcPtr[3];
#endif

         crc = genUtl_getCrc32((unsigned char *) imageBuf, imageLen - TOKEN_LEN, crc);
#ifndef DESKTOP_LINUX
         if (crc == imageCrc)
#else
         if ((crc == imageCrc) || crc == htonl(imageCrc))
#endif
         {
            UINT32 maxLen;

            cmsLog_debug("Whole flash image format [%d bytes] verified.", imageLen);
            maxLen = bcmImg_getImageFlashSize();
            if (imageLen > maxLen)
            {
               cmsLog_error("whole image is too large for flash, got %u, max %u", imageLen, maxLen);
            }
            else
            {
               result = CMS_IMAGE_FORMAT_FLASH;
            }
         }
         else
         {
#if defined(EPON_SDK_BUILD)
            cmsLog_debug("Could not determine image format [%d bytes]", imageLen);
#else
            cmsLog_error("Could not determine image format [%d bytes]", imageLen);
#endif
            cmsLog_debug("calculated crc=0x%x image crc=0x%x", crc, imageCrc);
         }
      }
   }

   cmsLog_debug("returning image format %d", result);

   return result;
}


UBOOL8 bcmImg_isBcmTaggedImage(const char *imageBuf, UINT32 *imageSize)
{
   FILE_TAG *pTag = (FILE_TAG *)imageBuf;
   UINT32 crc;
   UINT32 tokenCrc, *pTokenCrc;
   UBOOL8 isBcmImg = FALSE;

   pTokenCrc = (UINT32 *)pTag->tagValidationToken;
   tokenCrc = *pTokenCrc;
#ifdef DESKTOP_LINUX
    /* assume desktop is running on little endien intel, but the CRC has been
     * written for big endien mips, so swap before compare.
     * FIXME! this part of code might not work with Little-endian target system.
     */
    tokenCrc = htonl(tokenCrc);
#endif

   /* check tag validate token to see if it is a valid bcmTag */
   crc = CRC_INITIAL_VALUE;
   crc = genUtl_getCrc32((UINT8 *) pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);
   if (crc == tokenCrc)
   {
      isBcmImg = TRUE;
      *imageSize = (UINT32) atoi((char *)pTag->totalImageLen) + TAG_LEN;
      cmsLog_debug("It's a broadcom tagged image with length %d", *imageSize );
   }
   else
   {
      cmsLog_debug("token crc failed, this is not a valid broadcom image");
   }

   return isBcmImg;

}

CmsImageFormat bcmImg_parseImgHdr(UINT8 *bufP, UINT32 bufLen)
{
   CmsImageFormat result = CMS_IMAGE_FORMAT_INVALID;

   if (bufP == NULL)
   {
       cmsLog_debug("NULL bufP pointer.");
       result = CMS_IMAGE_FORMAT_INVALID;
   }
   else if ((bufLen > CMS_CONFIG_FILE_DETECTION_LENGTH) &&
            bcmImg_isConfigFileLikely((char*)bufP))
   {
       cmsLog_debug("Detected possible CMS XML config file format");
       result = CMS_IMAGE_FORMAT_XML_CFG;
   }
#ifdef SUPPORT_MODSW_LINUXPFP
   else if (cmsImg_isModSwLinuxPfp((UINT8*)bufP, bufLen))
   {
       cmsLog_debug("Detected Modular Software Linux PFP format.");
       result = CMS_IMAGE_FORMAT_MODSW_LINUXPFP;
   }
#endif
   else if ((bufLen > sizeof(FILE_TAG)) &&
     (bcmImg_verifyBroadcomFileTag((FILE_TAG*)bufP, 0, bufLen) == CMSRET_SUCCESS))
   {
       cmsLog_debug("Detected Broadcom image format.");
       result = CMS_IMAGE_FORMAT_BROADCOM;
   }

   return result;
}

void dumpBcmFileTag(FILE_TAG *tagP)
{
    printf("--------------------------------------------------------------\n");
    printf("Broadcom image tag:\n");
    printf("\ttagVersion: %02x %02x %02x %02x\n",
      tagP->tagVersion[0], tagP->tagVersion[1],
      tagP->tagVersion[2], tagP->tagVersion[3]);
    printf("\tendian: %s\n", tagP->bigEndian);
    printf("\ttotalImageLen: %s\n", tagP->totalImageLen);
    printf("\tcfeAddress: %s\n", tagP->cfeAddress);
    printf("\tcfeLen: %s\n", tagP->cfeLen);
    printf("\trootfsAddress: %s\n", tagP->rootfsAddress);
    printf("\trootfsLen: %s\n", tagP->rootfsLen);
    printf("\tkernelAddress: %s\n", tagP->kernelAddress);
    printf("\tkernelLen: %s\n", tagP->kernelLen);
    printf("\timageSequence: %s\n", tagP->imageSequence);
    printf("\timageVersion: %s\n", tagP->imageVersion);
    printf("--------------------------------------------------------------\n");
}

/*
 * IMGUTIL_TODO: the following logic is replicated in a few places.
 * To consolidate.
 * ./private/apps/tr69c/main/informer.c: allocSize -= 64 * 1024;
 * ./gpl/apps/ftpd/ftpd/commands.c:   totalAllocatedSize -= 64 * 1024;
 */

/*
 * If maxBufSize is initialized, try that (with allocSize = maxBufSize)
 * and if still failing, try reducing 64K from allocSize and try again as
 * long as allocSize > CMS_IMAGE_REQUIRED_LEN.
 */
UINT8* bcmImg_allocImgBuf(UINT32 maxBufSize, UINT32 *allocBufSizeP)
{
    UINT32 allocSize;
    UINT8 *bufP = NULL;

    if (maxBufSize > 0)
    {
        allocSize = maxBufSize;
    }
    else
    {
        allocSize = bcmImg_getImageFlashSize() +
          bcmImg_getBroadcomImageTagSize();
    }

    cmsLog_debug("bcmImg_allocImgBuf() begin, %d", allocSize);

    if ((bufP = (UINT8*)malloc(allocSize)) == NULL)
    {
        /* TODO: this logic in commands.c does not seem to be correct. */
        /* if (maxBufSize > 0) */
        {
            while (((bufP = (UINT8*)malloc(allocSize)) == NULL) &&
              (allocSize > CMS_IMAGE_REQUIRED_LEN))
            {
               allocSize -= 64 * 1024;
               cmsLog_debug("Try allocating %d kb", allocSize/1024);
            }
        }

        if (bufP == NULL)
        {
            cmsLog_error("Not enough memory to alloc %u bytes.", allocSize);
            *allocBufSizeP = 0;
            return NULL;
        }
    }

    cmsLog_debug("bcmImg_allocImgBuf() end, %d", allocSize);

    *allocBufSizeP = allocSize;
    return bufP;
}
