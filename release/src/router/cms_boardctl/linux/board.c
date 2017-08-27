/***********************************************************************
 *
 * <:copyright-BRCM:2006:DUAL/GPL:standard
 * 
 *    Copyright (c) 2006 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#include "cms.h"
#include <fcntl.h>  /* for open */
#include <sys/stat.h>
#include <errno.h>
#include "board.h" /* for FLASH_ADDR_INFO */
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "cms_util.h"
#include "boardparms.h"
#include "bcm_flashutil.h"

#ifdef DESKTOP_LINUX
static int fake_board_ioctl(UINT32 boardIoctl, BOARD_IOCTL_PARMS *ioctlParms);
int fake_kerSysScratchPadList(char *tokBuf, int bufLen);
int fake_kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen);
int fake_kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen);
int fake_kerSysScratchPadClearAll(void);
char *fake_getSharedBlks(int start_block, int num_blocks);
int fake_setSharedBlks(int start_block, int total_blocks, char *pBuf);
void fake_retriedKfree(char *pShareBuf);
SINT32 fake_kerSysBcmImageSetWhole(UINT8 *buf, SINT32 bufLen);
SINT32 fake_kerSysBcmImageSetCfe(UINT8 *buf, SINT32 bufLen);
SINT32 fake_flashFsKernelImage(UINT8 *buf, SINT32 bufLen);
SINT32 fake_kerSysPersistentSet(UINT8 *buf, UINT32 bufLen);
SINT32 fake_kerSysPersistentGet(UINT8 *buf, UINT32 bufLen);
SINT32 fake_kerSysBackupPsiSet(UINT8 *buf, UINT32 bufLen);
SINT32 fake_kerSysBackupPsiGet(UINT8 *buf, UINT32 bufLen);


#define FAKE_FLASH_PSP_FILENAME   "flash_persistent_scratch_pad.data"

/* from bcmdrivers/opensource/include/bcm963xx/board.h */
/* how many bytes per flash block? */
#define FAKE_FLASH_BLOCK_SIZE      512
#define FAKE_NUM_PSP_FLASH_BLOCKS  16


static FLASH_ADDR_INFO fInfo = {0, 0, 0, 0, \
/* psp start blk,         num blks,    length in bytes,       blk offset from begining,*/
      0, FAKE_NUM_PSP_FLASH_BLOCKS, FAKE_NUM_PSP_FLASH_BLOCKS*FAKE_FLASH_BLOCK_SIZE, 0, \
                                0, 0, 0, 0, 0, 0, 0 };

#endif  /* DESKTOP_LINUX */


/* All the devctl functions are probably OS dependent, so they would go here */

CmsRet devCtl_getBaseMacAddress(UINT8 *macAddrNum)
{
   return(devCtl_boardIoctl(BOARD_IOCTL_GET_BASE_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            MAC_ADDR_LEN,
                            0,
                            ""));
}


CmsRet devCtl_getMacAddress(UINT8 *macAddrNum, UINT32 ulId)
{
   return(devCtl_boardIoctl(BOARD_IOCTL_GET_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            MAC_ADDR_LEN,
                            ulId,
                            ""));
}

CmsRet devCtl_getMacAddresses(UINT8 *macAddrNum, UINT32 ulId, UINT32 num_addresses)
{
   return(devCtl_boardIoctl(BOARD_IOCTL_ALLOC_MAC_ADDRESSES,
                            0,
                            (char *) macAddrNum,
                            MAC_ADDR_LEN,
                            ulId,
                            &num_addresses));
}

CmsRet devCtl_releaseMacAddresses(UINT8 *macAddrNum, UINT32 num_addresses)
{
   return(devCtl_boardIoctl(BOARD_IOCTL_RELEASE_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            MAC_ADDR_LEN,
                            0,
                            &num_addresses));
}

CmsRet devCtl_releaseMacAddress(UINT8 *macAddrNum)
{
   return(devCtl_boardIoctl(BOARD_IOCTL_RELEASE_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            MAC_ADDR_LEN,
                            0,
                            ""));
}


UINT32 devCtl_getNumEnetMacs(void)
{
   UINT32 num;
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_NUM_ENET_MACS,
                           0, NULL,
                           0, 0, &num);
   return (ret == CMSRET_SUCCESS ? num : 0);
}


UINT32 devCtl_getNumEnetPorts(void)
{
   UINT32 num;
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_NUM_ENET_PORTS,
                           0, NULL,
                           0, 0, &num);
   return (ret == CMSRET_SUCCESS ? num : 0);
}

#if defined(EPON_SDK_BUILD)
UINT32 devCtl_getPortMacType(unsigned short port, unsigned int *mac_type)
{
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_PORT_MAC_TYPE,
                           0, NULL,
                           0, port, mac_type);
   return ret;
}

UINT32 devCtl_getNumFePorts(unsigned int *fe_ports)
{
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_NUM_FE_PORTS,
                           0, NULL,
                           0, 0, fe_ports);
   return ret;
}

UINT32 devCtl_getNumGePorts(unsigned int *ge_ports)
{
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_NUM_GE_PORTS,
                           0, NULL,
                           0, 0, ge_ports);
   return ret;
}

UINT32 devCtl_getNumVoipPorts(unsigned int *voip_ports)
{
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_NUM_VOIP_PORTS,
                           0, NULL,
                           0, 0, voip_ports);
   return ret;
}

UINT32 devCtl_getPortMap(unsigned int *port_map)
{
   CmsRet ret;
   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_SWITCH_PORT_MAP,
                           0, NULL,
                           0, 0, port_map);
   return ret;
}
#endif

UINT32 devCtl_getSdramSize(void)
{
   UINT32 size;
   CmsRet ret;
#ifdef DESKTOP_LINUX
   size = 32*1024*1024;
   ret = CMSRET_SUCCESS;
#else   
   ret = devCtl_boardIoctl(BOARD_IOCTL_GET_SDRAM_SIZE,
                           0, NULL,
                           0, 0, &size);
#endif
   return (ret == CMSRET_SUCCESS ? size : 0);
}

CmsRet devCtl_getChipId(UINT32 *chipId)
{
    return( devCtl_boardIoctl(BOARD_IOCTL_GET_CHIP_ID, 0, "", 0, 0, chipId) );
}

int devCtl_getSequenceNumber(int image)
{
    unsigned int nandFlag=0;

    getFlashInfo(&nandFlag);
    if (nandFlag & FLASH_INFO_FLAG_NAND) // NAND flash, no need to dig into kernel space
        return(getSequenceNumber(image));
    else
        return( (int) devCtl_boardIoctl(BOARD_IOCTL_GET_SEQUENCE_NUMBER, 0, "", 0, image, "") );
}

CmsRet devCtl_setImageState(int new_image, int state, int seq1, int seq2)
{ // new_image = 0 to set boot state of current "image", 1 to set boot state of newly written "image_update", this is only for pureUBI which saves the boot state as part of the image, does not affect JFFS2 since boot_state is written to the /data partiton
    unsigned int nandFlag=0;

    getFlashInfo(&nandFlag);
    if (nandFlag & FLASH_INFO_FLAG_NAND) // NAND flash, no need to dig into kernel space
        return(setBootImageState(new_image, state, seq1, seq2));
    else
        return( devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION, seq1, "", seq2, state, "") );
}

int devCtl_getImageState(void)
{
    unsigned int nandFlag=0;

    getFlashInfo(&nandFlag);
    if (nandFlag & FLASH_INFO_FLAG_NAND) // NAND flash, no need to dig into kernel space
        return(getBootImageState());
    else
        return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION, 0, "", 0, BOOT_GET_BOOT_IMAGE_STATE, "") );
}

int devCtl_getImageVersion(int partition, char *verStr, int verStrSize)
{
    return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION, partition, verStr, verStrSize, BOOT_GET_IMAGE_VERSION, "") );
}

int devCtl_getBootedImagePartition(void)
{
    return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION, 0, "", 1, BOOT_GET_BOOTED_IMAGE_ID, "") );
}

int devCtl_getBootedImageId(void)
{
    return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION, 0, "", 0, BOOT_GET_BOOTED_IMAGE_ID, "") );
}

CmsRet devCtl_getDefaultOpticalParams(unsigned char * pOpticalData)
{    
    CmsRet Ret;
    
    Ret = devCtl_boardIoctl(BOARD_IOCTL_GET_DEFAULT_OPTICAL_PARAMS, 0, (char *)pOpticalData, BP_OPTICAL_PARAMS_LEN, 0, "");

    return (Ret);
}

CmsRet devCtl_getGponOpticsType(void)
{    
    UINT32 OpticsType = 0;
    CmsRet Ret;
    
    Ret = devCtl_boardIoctl(BOARD_IOCTL_GET_GPON_OPTICS_TYPE, 0, "", 0, 0, &OpticsType);

    if (Ret == CMSRET_SUCCESS)
    {
       return (CmsRet) OpticsType;
    }
    else
    {
       return (CmsRet) -1;
    }
}


CmsRet devCtl_boardIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data)
{
    BOARD_IOCTL_PARMS ioctlParms;
    SINT32 boardFd = 0;
    SINT32 rc;
    CmsRet ret=CMSRET_SUCCESS;

#ifdef DESKTOP_LINUX
    /* don't open anything, ioctl to this fd will be faked anyways */
    boardFd = 77777;
#else
    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);
#endif

    if ( boardFd != -1 )
    {
        ioctlParms.string = string;
        ioctlParms.strLen = strLen;
        ioctlParms.offset = offset;
        ioctlParms.action = action;
        ioctlParms.buf    = data;
        ioctlParms.result = -1;

#ifdef DESKTOP_LINUX
        rc = fake_board_ioctl(boardIoctl, &ioctlParms);
#else
        rc = ioctl(boardFd, boardIoctl, &ioctlParms);
        close(boardFd);
#endif


        /*
         * When reading the scratch pad, the return value indicates the count.
         * Check for that condition first.
         */
        if (((boardIoctl == BOARD_IOCTL_FLASH_READ) && (action == SCRATCH_PAD)) ||
            ((boardIoctl == BOARD_IOCTL_FLASH_LIST) && (action == SCRATCH_PAD)) ||
            (boardIoctl == BOARD_IOCTL_BOOT_IMAGE_OPERATION) ||
            (boardIoctl == BOARD_IOCTL_GET_SEQUENCE_NUMBER))
        {
           /*
            * The kernel will either return the number of bytes read,
            * or if the user provided buffer was not big enough, a
            * negative number indicating the number of bytes needed.
            * See cms_psp.h, cmsPsp_get().
            */
           ret = (CmsRet) ioctlParms.result;
        }
        else
        { 
           if (rc < 0)
           {
              cmsLog_debug("boardIoctl=0x%x action=%d rc=%d errno=%d", boardIoctl, action, rc, errno);
              ret = CMSRET_INVALID_ARGUMENTS;
           }
        
           /* ioctl specific return data */
           if (ret == CMSRET_SUCCESS)
           {
              if ((boardIoctl == BOARD_IOCTL_GET_PSI_SIZE) ||
                  (boardIoctl == BOARD_IOCTL_GET_BACKUP_PSI_SIZE) ||
                  (boardIoctl == BOARD_IOCTL_GET_SYSLOG_SIZE) ||
                  (boardIoctl == BOARD_IOCTL_GET_CHIP_ID) ||
                  (boardIoctl == BOARD_IOCTL_GET_CHIP_REV) ||
                  (boardIoctl == BOARD_IOCTL_GET_NUM_ENET_MACS) ||
                  (boardIoctl == BOARD_IOCTL_GET_NUM_FE_PORTS) ||
                  (boardIoctl == BOARD_IOCTL_GET_NUM_GE_PORTS) ||
                  (boardIoctl == BOARD_IOCTL_GET_PORT_MAC_TYPE) ||
                  (boardIoctl == BOARD_IOCTL_GET_NUM_VOIP_PORTS) ||
                  (boardIoctl == BOARD_IOCTL_GET_SWITCH_PORT_MAP) ||
                  (boardIoctl == BOARD_IOCTL_GET_NUM_ENET_PORTS) ||
                  (boardIoctl == BOARD_IOCTL_GET_SDRAM_SIZE) ||               
                  (boardIoctl == BOARD_IOCTL_GET_BTRM_BOOT) ||               
                  (boardIoctl == BOARD_IOCTL_GET_BOOT_SECURE) ||               
                  (boardIoctl == BOARD_IOCTL_FLASH_READ && action == FLASH_SIZE))
              {
                 if (data != NULL)
                 {
                    *((UINT32 *)data) = (UINT32) ioctlParms.result;
                 }
              }
           }
        }
    }
    else
    {
       cmsLog_error("Unable to open device %s", BOARD_DEVICE_NAME);
       ret = CMSRET_INTERNAL_ERROR;
    }

    return ret;
}

#ifdef DESKTOP_LINUX

/** Do first level processing of ioctl.
 *
 * This function follows the logic of
 * bcmdrivers/opensource/char/board/bcm963xx/impl1/board.c:board_ioctl
 */
int fake_board_ioctl(UINT32 command, BOARD_IOCTL_PARMS *ctrlParms)
{
   int ret;

   switch(command)
   {
   case BOARD_IOCTL_FLASH_WRITE:

      switch(ctrlParms->action)
      {
      case SCRATCH_PAD:
         if (ctrlParms->offset == -1)
            ret = fake_kerSysScratchPadClearAll();
         else
            ret = fake_kerSysScratchPadSet(ctrlParms->string, ctrlParms->buf, ctrlParms->offset);
         break;
         
      case BCM_IMAGE_WHOLE:
          cmsLog_debug("fake whole image write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBcmImageSetWhole((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
 
      case BCM_IMAGE_CFE:
          cmsLog_debug("fake cfe write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBcmImageSetCfe((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
         
      case BCM_IMAGE_FS:
          cmsLog_debug("fake fs+kernel write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_flashFsKernelImage((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
          
      case PERSISTENT:
          cmsLog_debug("fake config write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysPersistentSet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;

#ifdef SUPPORT_BACKUP_PSI
      case BACKUP_PSI:
          cmsLog_debug("fake backup config write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBackupPsiSet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
#endif

      default:
         ret = -EINVAL;
         cmsLog_error("unhandled action %d in BOARD_IOCTL_FLASH_WRITE");
         break;
      }

      ctrlParms->result = ret;

      break;

   case BOARD_IOCTL_FLASH_READ:
      switch (ctrlParms->action)
      {
      case SCRATCH_PAD:
         ret = fake_kerSysScratchPadGet(ctrlParms->string, ctrlParms->buf, ctrlParms->offset);
         ctrlParms->result = ret;
         break;
         
      case FLASH_SIZE:
         ctrlParms->result = 4 * 1024 * 1024; /* 4 MB? */
         ret = 0;
         break;

      case PERSISTENT:
          cmsLog_debug("fake config read, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysPersistentGet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;

#ifdef SUPPORT_BACKUP_PSI
      case BACKUP_PSI:
          cmsLog_debug("fake backup psi config read, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBackupPsiGet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
#endif

      default:
         ret = -EINVAL;
         ctrlParms->result = ret;
         cmsLog_error("unhandled action %d in BOARD_IOCTL_FLASH_READ");
         break;
      }

      break;

   case BOARD_IOCTL_FLASH_LIST:
      switch (ctrlParms->action)
      {
      case SCRATCH_PAD:
         ret = fake_kerSysScratchPadList(ctrlParms->buf, ctrlParms->offset);
         ctrlParms->result = ret;
         break;
         
      default:
         ret = -EINVAL;
         ctrlParms->result = ret;
         cmsLog_error("unhandled action %d in BOARD_IOCTL_FLASH_READ");
         break;
      }

      break;

   case BOARD_IOCTL_GET_PSI_SIZE:
      ctrlParms->result = 24 * 1024;
      ret = 0;
      break;

#ifdef SUPPORT_BACKUP_PSI
   case BOARD_IOCTL_GET_BACKUP_PSI_SIZE:
      ctrlParms->result = 24 * 1024;
      ret = 0;
      break;
#endif
      
   case BOARD_IOCTL_GET_NUM_ENET_MACS:
      ctrlParms->result = 2;
      ret = 0;
      break;
	     	
   case BOARD_IOCTL_GET_NUM_ENET_PORTS:
      ctrlParms->result = 4;
      ret = 0;
      break;
      
   case BOARD_IOCTL_GET_CHIP_ID:
#if defined(CHIP_6362)
      ctrlParms->result = 0x6362;
#endif
#if defined(CHIP_6328)
      ctrlParms->result = 0x6328;
#endif
#if defined(CHIP_63268)
      ctrlParms->result = 0x63268;
#endif
#if defined(CHIP_6318)
      ctrlParms->result = 0x6318;
#endif
#if defined(CHIP_63138)
      ctrlParms->result = 0x63138;
#endif
#if defined(CHIP_63381)
      ctrlParms->result = 0x63381;
#endif
#if defined(CHIP_63148)
      ctrlParms->result = 0x63148;
#endif

      ret = 0;
      break;

   case BOARD_IOCTL_GET_CFE_VER:
      if (ctrlParms->strLen < 5)
      {
         cmsLog_error("buf too short, need 5, got %d", ctrlParms->strLen);
         ret = -1;
      }
      else
      {
         ctrlParms->string[0] = 1;
         ctrlParms->string[1] = 0;
         ctrlParms->string[2] = 37;
         ctrlParms->string[3] = 9;
         ctrlParms->string[4] = 14;
         ret = 0;
      }
      break;

   case BOARD_IOCTL_GET_ID:
      /* this is get BOARD_ID */
      {
         const char *boardIdStr="DESKTOP_LINUX";
         if (ctrlParms->strLen < (SINT32) (strlen(boardIdStr) + 1))
         {
            cmsLog_error("buf too short, need %d, got %d", strlen(boardIdStr)+1, ctrlParms->strLen);
            ret = -1;
         }
         else
         {
            sprintf(ctrlParms->string, "%s", boardIdStr);
            ret = 0;
         }
      }
      break;

   case BOARD_IOCTL_GET_BASE_MAC_ADDRESS:
      ctrlParms->string[0] = (char) 0;
      ctrlParms->string[1] = (char) 0x11;
      ctrlParms->string[2] = (char) 0x22;
      ctrlParms->string[3] = (char) 0x33;
      ctrlParms->string[4] = (char) 0x44;
      ctrlParms->string[5] = (char) 0x55;
      ret = 0;
      break;

   case BOARD_IOCTL_BOOT_IMAGE_OPERATION:
      ret = 0;
      switch (ctrlParms->offset)
      {
          case BOOT_SET_PART1_IMAGE:
          case BOOT_SET_PART2_IMAGE:
          case BOOT_SET_PART1_IMAGE_ONCE:
          case BOOT_SET_PART2_IMAGE_ONCE:
          case BOOT_SET_OLD_IMAGE:
          case BOOT_SET_NEW_IMAGE:
          case BOOT_SET_NEW_IMAGE_ONCE:
              ctrlParms->result = 0;
              break;

          case BOOT_GET_BOOT_IMAGE_STATE:
              ctrlParms->result = BOOT_SET_PART1_IMAGE;
              break;

          case BOOT_GET_IMAGE_VERSION:
              ctrlParms->result = strlen("DESKTOP_INUX");
              strcpy(ctrlParms->string, "DESKTOP_LINUX");
              break;

          case BOOT_GET_BOOTED_IMAGE_ID:
              ctrlParms->result = BOOTED_PART1_IMAGE;
              break;

          default:
              ret = -EINVAL;
              ctrlParms->result = ret;
              cmsLog_error("unhandled offset %d in "
                "BOARD_IOCTL_BOOT_IMAGE_OPERATION", ctrlParms->offset);
              break;
      }
      break;

   case BOARD_IOCTL_GET_SEQUENCE_NUMBER:
      ctrlParms->result = 0;
      break;

   case BOARD_IOCTL_MIPS_SOFT_RESET:
      cmsLog_notice("Soft Reset...\n");
      break;

   default:
      ret = -EINVAL;
      cmsLog_error("unhandled board ioctl 0x%x", command);
   }

   return ret;
}


/** Get list of all keys/tokenID's in the scratch pad.
 *
 * This function follows the logic of the same function name in 
 * bcmdrivers/opensource/char/board/bcm963xx/impl1/bcm63xx_flash.c
 *
 * @return greater than 0 means number of bytes copied to tokBuf,
 *         0 means fail,
 *         negative number means provided buffer is not big enough and the
 *         absolute value of the negative number is the number of bytes needed.
 */
int fake_kerSysScratchPadList(char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int tokenNameLen=0;
    int copiedLen=0;
    int needLen=0;
    int sts = 0;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if( (pShareBuf = fake_getSharedBlks(fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk +
        fInfo.flash_scratch_pad_number_blk))) == NULL )
    {
        cmsLog_error("could not get sharedBlks");
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        cmsLog_error("Scratch pad is not initialized.\n");
        fake_retriedKfree(pShareBuf);
        return sts;
    }

    // Walk through all the tokens
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;

    while( pToken->tokenName[0] != '\0' && pToken->tokenLen > 0 &&
           ((usedLen + pToken->tokenLen) <= fInfo.flash_scratch_pad_length))
    {

        tokenNameLen = strlen(pToken->tokenName);
        needLen += tokenNameLen + 1;
        cmsLog_debug("found tokenName=%s copiedLen=%d needLen=%d bufLen=%d", pToken->tokenName, copiedLen, needLen, bufLen);
        if (needLen <= bufLen)
        {
            strcpy(&tokBuf[copiedLen], pToken->tokenName);
            copiedLen += tokenNameLen + 1;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    if ( needLen > bufLen )
    {
        // User may purposely pass in a 0 length buffer just to get
        // the size, so don't log this as an error.
        sts = needLen * (-1);
    }
    else
    {
        sts = copiedLen;
    }

    fake_retriedKfree(pShareBuf);

    return sts;
}


/** Get a buffer from the scratch pad based on tokenId.
 *
 * This function follows the logic of the same function name in 
 * bcmdrivers/opensource/char/board/bcm963xx/impl1/bcm63xx_flash.c
 *
 * @return greater than 0 means number of bytes copied to tokBuf,
 *         0 means fail,
 *         negative number means provided buffer is not big enough and the
 *         absolute value of the negative number is the number of bytes needed.
 */
int fake_kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *pShareBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    int sts = 0;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if( (pShareBuf = fake_getSharedBlks(fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk +
        fInfo.flash_scratch_pad_number_blk))) == NULL )
    {
        cmsLog_error("could not get sharedBlks");
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        cmsLog_error("Scratch pad is not initialized.\n");
        fake_retriedKfree(pShareBuf);
        return sts;
    }

    // search for the token
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;
    while( pToken->tokenName[0] != '\0' && pToken->tokenLen > 0 &&
        pToken->tokenLen < fInfo.flash_scratch_pad_length &&
        usedLen < fInfo.flash_scratch_pad_length )
    {

        if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
        {
            if ( pToken->tokenLen > bufLen )
            {
                cmsLog_error("The length %d of token %s is greater than buffer len %d.", pToken->tokenLen, pToken->tokenName, bufLen);
                sts = pToken->tokenLen * (-1);
            }
            else
            {
                cmsLog_debug("found token %s, copying out (len=%d)", tokenId, pToken->tokenLen);
                memcpy(tokBuf, startPtr + sizeof(SP_TOKEN), pToken->tokenLen);             
                sts = pToken->tokenLen;
            }
            break;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    fake_retriedKfree(pShareBuf);

    return sts;
}


/** Set contents of a scratch pad buffer identified by tokenId.
 *
 * This function follows the logic of the same function name in 
 * bcmdrivers/opensource/char/board/bcm963xx/impl1/bcm63xx_flash.c
 *
 * @return  0 - ok, -1 - fail.
 */
int fake_kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen)
{
    PSP_TOKEN pToken = NULL;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
    SP_HEADER SPHead;
    SP_TOKEN SPToken;
    char *curPtr;
    int sts = -1;

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;

    if( bufLen >= (int) (fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
                         sizeof(SP_TOKEN)) )
    {
        cmsLog_error("Scratch pad overflow by %d bytes.  Information not saved.\n",
            bufLen  - fInfo.flash_scratch_pad_length - sizeof(SP_HEADER) -
            sizeof(SP_TOKEN));
        return sts;
    }

    if( (pShareBuf = fake_getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk +
        fInfo.flash_scratch_pad_number_blk) )) == NULL )
    {
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    // form header info.
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, TOKEN_NAME_LEN - 1);
    SPToken.tokenLen = bufLen;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        cmsLog_notice("No scratch pad found.  Initialize scratch pad...\n");
        memcpy(pBuf, (char *)&SPHead, sizeof(SP_HEADER));
        curPtr = pBuf + sizeof(SP_HEADER);
        memcpy(curPtr, (char *)&SPToken, sizeof(SP_TOKEN));
        curPtr += sizeof(SP_TOKEN);
        if( tokBuf )
            memcpy(curPtr, tokBuf, bufLen);
    }
    else  
    {
        int putAtEnd = 1;
        int curLen;
        int usedLen;
        int skipLen;

        /* Calculate the used length. */
        usedLen = sizeof(SP_HEADER);
        curPtr = pBuf + sizeof(SP_HEADER);
        pToken = (PSP_TOKEN) curPtr;
        skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        while( pToken->tokenName[0] >= 'A' && pToken->tokenName[0] <= 'z' &&
            strlen(pToken->tokenName) < TOKEN_NAME_LEN &&
            pToken->tokenLen > 0 &&
            pToken->tokenLen < fInfo.flash_scratch_pad_length &&
            usedLen < fInfo.flash_scratch_pad_length )
        {
            usedLen += sizeof(SP_TOKEN) + skipLen;
            curPtr += sizeof(SP_TOKEN) + skipLen;
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        }

        if( usedLen + SPToken.tokenLen + sizeof(SP_TOKEN) >
            (UINT32) fInfo.flash_scratch_pad_length )
        {
            cmsLog_error("Scratch pad overflow by %d bytes.  Information not saved.\n",
                (usedLen + SPToken.tokenLen + sizeof(SP_TOKEN)) -
                fInfo.flash_scratch_pad_length);
            /* mwang: did not free pShareBuf, memory leak. */
            return sts;
        }

        curPtr = pBuf + sizeof(SP_HEADER);
        curLen = sizeof(SP_HEADER);
        while( curLen < usedLen )
        {
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
            if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
            {
                // The token id already exists.
                if( tokBuf && pToken->tokenLen == bufLen )
                {
                    // The length of the new data and the existing data is the
                    // same.  Overwrite the existing data.
                    memcpy((curPtr+sizeof(SP_TOKEN)), tokBuf, bufLen);
                    putAtEnd = 0;
                }
                else
                {
                    // The length of the new data and the existing data is
                    // different.  Shift the rest of the scratch pad to this
                    // token's location and put this token's data at the end.
                    char *nextPtr = curPtr + sizeof(SP_TOKEN) + skipLen;
                    int copyLen = usedLen - (curLen+sizeof(SP_TOKEN) + skipLen);
                    memcpy( curPtr, nextPtr, copyLen );
                    memset( curPtr + copyLen, 0x00, 
                        fInfo.flash_scratch_pad_length - (curLen + copyLen) );
                    usedLen -= sizeof(SP_TOKEN) + skipLen;
                }
                break;
            }

            // get next token
            curPtr += sizeof(SP_TOKEN) + skipLen;
            curLen += sizeof(SP_TOKEN) + skipLen;
        } // end while

        if( putAtEnd )
        {
            if( tokBuf )
            {
                memcpy( pBuf + usedLen, &SPToken, sizeof(SP_TOKEN) );
                memcpy( pBuf + usedLen + sizeof(SP_TOKEN), tokBuf, bufLen );
            }
            memcpy( pBuf, &SPHead, sizeof(SP_HEADER) );
        }

    } // else if not new sp

    sts = fake_setSharedBlks(fInfo.flash_scratch_pad_start_blk, 
        (fInfo.flash_scratch_pad_number_blk + fInfo.flash_scratch_pad_start_blk),
        pShareBuf);

    fake_retriedKfree(pShareBuf);

    return sts;
}


/** Wipe out the scratchPad.
 *
 * This function follows the logic of the same function name in 
 * bcmdrivers/opensource/char/board/bcm963xx/impl1/bcm63xx_flash.c
 *
 * @return  0 - ok, -1 - fail.
 */
int fake_kerSysScratchPadClearAll(void)
{ 
    int sts = -1;
    char *pShareBuf = NULL;
    char *pBuf = NULL;
	

    if (fInfo.flash_scratch_pad_length == 0)
        return sts;
    if( (pShareBuf = fake_getSharedBlks( fInfo.flash_scratch_pad_start_blk,
        (fInfo.flash_scratch_pad_start_blk + fInfo.flash_scratch_pad_number_blk) )) == NULL )
        return sts;
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    /* mwang: this actually wipes out the PS_HEADER in the PSP flash area.
     * was that the intent? */
    memset(pBuf, 0x00,  fInfo.flash_scratch_pad_length);
    sts = fake_setSharedBlks(fInfo.flash_scratch_pad_start_blk,    
		(fInfo.flash_scratch_pad_number_blk + fInfo.flash_scratch_pad_start_blk),  pBuf);

   fake_retriedKfree(pShareBuf);
	
   return sts;
}

/** Return a pointer to the beginning of the requested flash buffer.
 *
 * On DESKTOP_LINUX, this just opens the fake flash file, allocates a buffer,
 * read contents of fake flash file into buffer, and returns pointer to the
 * buffer.  Persistent flash data is the only thing in the flash file (for now).
 * If fake flash file is not present, create one and fill it with zeros.
 */
char *fake_getSharedBlks(int start_block, int num_blocks)
{
   UINT32 bufLen;
   char *buf=NULL;
   char path[BUFLEN_1024];
   struct stat statbuf;
   int rc, fd;
   CmsRet ret;

   cmsLog_debug("reading block %d through %d", start_block, start_block+num_blocks);

   if (start_block != 0)
   {
      cmsLog_error("cannot handle non-zero start block yet.");
      return NULL;
   }

   if (num_blocks > FAKE_NUM_PSP_FLASH_BLOCKS)
   {
      cmsLog_error("requested more blocks than PSP flash blocks, not handled.");
      return NULL;
   }


   /* first allocate the buffer we will need for the read */
   bufLen = FAKE_FLASH_BLOCK_SIZE * FAKE_NUM_PSP_FLASH_BLOCKS;
   if ((buf = cmsMem_alloc(bufLen, ALLOC_ZEROIZE)) == NULL)
   {
      cmsLog_error("malloc of %d bytes failed", bufLen);
      return NULL;
   }


   /* form path to the flash file */
   if ((ret = cmsUtl_getBaseDir(path, sizeof(path))) != CMSRET_SUCCESS)
   {
      cmsLog_error("getBaseDir failed, abort func");
      cmsMem_free(buf);
      return NULL;
   }
   else
   {
      UINT32 offset;

      offset = strlen(path);
      snprintf(&(path[offset]), sizeof(path)-offset, "/%s", FAKE_FLASH_PSP_FILENAME);
   }


   cmsLog_debug("checking for flash file at %s", path);
   if ((rc = stat(path, &statbuf)) < 0)
   {
      cmsLog_debug("creating fake flash file and initialize to zeros");
      fd = open(path, O_CREAT|O_RDWR, 0644);
      if (fd < 0)
      {
         cmsLog_error("create of flash file %s failed, errno=%d", path, errno);
         cmsMem_free(buf);
         return NULL;
      }

      /* fill rest of file with zeros */
      rc = write(fd, buf, bufLen);
      cmsLog_debug("filler write returned %d", rc);

      close(fd);
   }


   /*
    * at this point, we know there is a flash file, so just open it and read it.
    * Don't bother with offsets for now.  Just assume PSP is at the beginning
    * of the flash.
    */
   fd = open(path, O_RDWR);
   rc = read(fd, buf, num_blocks * FAKE_FLASH_BLOCK_SIZE);
   if (rc != num_blocks * FAKE_FLASH_BLOCK_SIZE)
   {
      cmsLog_error("unexpected rc %d from read, expected %d",
                   rc, num_blocks * FAKE_FLASH_BLOCK_SIZE);
      CMSMEM_FREE_BUF_AND_NULL_PTR(buf);
   }

   close(fd);

   return buf;
}


/** Write contents of pBuf to flash starting at start_block for total_blocks.
 *
 * @return 0 on success, -1 on failure. 
 */
int fake_setSharedBlks(int start_block, int total_blocks, char *pBuf)
{
   int fd, rc;
   int sts=-1;
   char path[BUFLEN_1024];
   CmsRet ret;


   cmsLog_debug("setting block %d through %d, buf %p", start_block, start_block+total_blocks, pBuf);

   if (start_block != 0)
   {
      cmsLog_error("cannot handle non-zero start block yet.");
      return sts;
   }

   if (total_blocks > FAKE_NUM_PSP_FLASH_BLOCKS)
   {
      cmsLog_error("requested more blocks than PSP flash blocks, not handled.");
      return sts;
   }

   /* form path to the flash file */
   if ((ret = cmsUtl_getBaseDir(path, sizeof(path))) != CMSRET_SUCCESS)
   {
      cmsLog_error("getBaseDir failed, abort func");
      return sts;
   }
   else
   {
      UINT32 offset;

      offset = strlen(path);
      snprintf(&(path[offset]), sizeof(path)-offset, "/%s", FAKE_FLASH_PSP_FILENAME);
   }

   cmsLog_debug("opening fake flash file at %s", path);
   if ((fd = open(path, O_RDWR)) < 0)
   {
      cmsLog_error("Could not open %s", path);
      return sts;
   }

   rc = write(fd, pBuf, total_blocks * FAKE_FLASH_BLOCK_SIZE);
   if (rc != total_blocks * FAKE_FLASH_BLOCK_SIZE)
   {
      cmsLog_error("bad write, got %d expected %d", rc, total_blocks * FAKE_FLASH_BLOCK_SIZE);
   }
   else
   {
      cmsLog_debug("%d blocks (%d bytes) written", total_blocks, total_blocks * FAKE_FLASH_BLOCK_SIZE);
      sts = 0;
   }

   close(fd);

   return sts;
}


/** Free resources used by previous flash read.
 *
 */
void fake_retriedKfree(char *pShareBuf)
{
   cmsMem_free(pShareBuf);

   return;
}


SINT32 fake_flashRead(const char *filename, UINT8 *buf, UINT32 bufLen)
{
   char basedir[BUFLEN_1024]={0};
   SINT32 rc, basedirLen;
   UINT32 localBufLen = bufLen;
   CmsRet ret;

   if ((ret = cmsUtl_getBaseDir(basedir, sizeof(basedir))) != CMSRET_SUCCESS)
   {
      cmsLog_error("getBasedir failed, ret=%d", ret);
      return ret;
   }

   basedirLen = strlen(basedir);
   rc = snprintf(&(basedir[basedirLen]), sizeof(basedir) - basedirLen, "/%s", filename);
   if (rc >= ((SINT32) sizeof(basedir)) - basedirLen)
   {
      cmsLog_error("basedir var not long enough to hold entire path");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   if (cmsFil_isFilePresent(basedir))
   {
      /*
       * cmsFil_readConfigFileToBuf will tell us how many bytes it actually
       * read from the file, but the kernel ioctl does not provide that info.
       * So we just throw away that length here.
       */
      ret = cmsFil_copyToBuffer(basedir, buf, &localBufLen);
   }
   else
   {
      /*
       * If the file does not exist, just return length 0.
       */
      ret = 0;
   }
      
   return ret;
}


SINT32 fake_flashWrite(const char *filename, UINT8 *buf, UINT32 bufLen)
{
   char basedir[BUFLEN_1024]={0};
   SINT32 fd, rc, basedirLen;
   CmsRet ret;

   if ((ret = cmsUtl_getBaseDir(basedir, sizeof(basedir))) != CMSRET_SUCCESS)
   {
      cmsLog_error("getBasedir failed, ret=%d", ret);
      return ret;
   }

   basedirLen = strlen(basedir);
   rc = snprintf(&(basedir[basedirLen]), sizeof(basedir) - basedirLen, "/%s", filename);
   if (rc >= ((SINT32) sizeof(basedir)) - basedirLen)
   {
      cmsLog_error("basedir var not long enough to hold entire path");
      return CMSRET_RESOURCE_EXCEEDED;
   }

   fd = open(basedir, O_RDWR|O_CREAT|O_TRUNC, 0666);
   if (fd < 0)
   {
      cmsLog_error("Could not open %s, errno=%d", basedir, errno);
      return CMSRET_INTERNAL_ERROR;
   }

   rc = write(fd, buf, bufLen);
   if (rc != (SINT32) bufLen)
   {
      cmsLog_error("write failed, expected %u got %d", bufLen, rc);
      ret = CMSRET_INTERNAL_ERROR;
   }
   else
   {
      cmsLog_debug("wrote buf len=%u to %s", bufLen, basedir);
      ret = CMSRET_SUCCESS;
   }

   close(fd);
   
   return ret;
}

SINT32 fake_kerSysBcmImageSetWhole(UINT8 *buf, SINT32 bufLen)
{
    return (fake_flashWrite("whole_flash.image", buf, bufLen));
}

SINT32 fake_kerSysBcmImageSetCfe(UINT8 *buf, SINT32 bufLen)
{
    return (fake_flashWrite("cfe.image", buf, bufLen));
}

SINT32 fake_flashFsKernelImage(UINT8 *buf, SINT32 bufLen)
{
   return (fake_flashWrite("fs_kernel.image", buf, bufLen));
}

SINT32 fake_kerSysPersistentSet(UINT8 *buf, UINT32 bufLen)
{
   return (fake_flashWrite("mdm.config", buf, bufLen));
}

SINT32 fake_kerSysBackupPsiSet(UINT8 *buf, UINT32 bufLen)
{
   return (fake_flashWrite("mdm-backup.config", buf, bufLen));
}

SINT32 fake_kerSysPersistentGet(UINT8 *buf, UINT32 bufLen)
{
   return (fake_flashRead("mdm.config", buf, bufLen));
}

SINT32 fake_kerSysBackupPsiGet(UINT8 *buf, UINT32 bufLen)
{
   return (fake_flashRead("mdm-backup.config", buf, bufLen));
}


#endif  /* DESKTOP_LINUX */
