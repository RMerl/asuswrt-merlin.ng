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

#include <fcntl.h>  /* for open */
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <linux/if_ether.h>  /* for ETH_ALEN (MAC_ADDR_LEN) */
#include <errno.h>
#include "bcm_retcodes.h"
#include "number_defs.h"
#include "board.h" /* for FLASH_ADDR_INFO */
#include "../bcm_boardctl.h"
#include "../bcm_boarddriverctl.h"
#include "boardparms.h"
#include "bcm_ulog.h"
#include "bcm_fsutils.h"
#include "sysutil_fs.h"


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

BcmRet devCtl_getBaseMacAddress(UINT8 *macAddrNum)
{
   return(devCtl_boardDriverIoctl(BOARD_IOCTL_GET_BASE_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            ETH_ALEN,
                            0,
                            NULL));
}


BcmRet devCtl_getMacAddress(UINT8 *macAddrNum, UINT32 ulId)
{
   return(devCtl_boardDriverIoctl(BOARD_IOCTL_GET_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            ETH_ALEN,
                            ulId,
                            NULL));
}

BcmRet devCtl_getMacAddresses(UINT8 *macAddrNum, UINT32 ulId, UINT32 num_addresses)
{
   return(devCtl_boardDriverIoctl(BOARD_IOCTL_ALLOC_MAC_ADDRESSES,
                            0,
                            (char *) macAddrNum,
                            ETH_ALEN,
                            ulId,
                            &num_addresses));
}

BcmRet devCtl_releaseMacAddresses(UINT8 *macAddrNum, UINT32 num_addresses)
{
   return(devCtl_boardDriverIoctl(BOARD_IOCTL_RELEASE_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            ETH_ALEN,
                            0,
                            &num_addresses));
}

BcmRet devCtl_releaseMacAddress(UINT8 *macAddrNum)
{
   return(devCtl_boardDriverIoctl(BOARD_IOCTL_RELEASE_MAC_ADDRESS,
                            0,
                            (char *) macAddrNum,
                            ETH_ALEN,
                            0,
                            NULL));
}


UINT32 devCtl_getNumEnetMacs(void)
{
   UINT32 num;
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_NUM_ENET_MACS,
                           0, NULL,
                           0, 0, &num);
   return (ret == BCMRET_SUCCESS ? num : 0);
}


UINT32 devCtl_getNumEnetPorts(void)
{
   UINT32 num;
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_NUM_ENET_PORTS,
                           0, NULL,
                           0, 0, &num);
   return (ret == BCMRET_SUCCESS ? num : 0);
}

#ifndef DESKTOP_LINUX

/** Open and lock the scratchpad file. Can be blocked if other process
 *  has the lock already. Allocate buffer and read scratchpad data
 * @return file descriptor of the scratchpad file
 *         negaive number means fail
 */

int getScratchPadFile(const char *fname, char** ppBuf, UINT32 *bufLen)
{
    struct stat statbuf;
    int rc, fd;
    int newfile = 0;
    mode_t mode = O_RDWR;

    if ((rc = stat(fname, &statbuf)) < 0 || statbuf.st_size != SP_MAX_LEN) 
    {
       *bufLen = SP_MAX_LEN;
       newfile = 1;
       mode = O_RDWR|O_CREAT|O_TRUNC;
    }
    else
       *bufLen = statbuf.st_size;

    fd = open(fname, mode);
    if (fd == -1)
    {
         bcmuLog_error("failed to open %s error %d", fname, errno);
         return -1;
    }

    /* acquire the file lock for exclusive access , may block if other process has the lock */
    rc = flock(fd, LOCK_EX);
    if (rc != 0)
    {
        bcmuLog_error("failed to lock file %s fd %d rc %d error %d", fname, fd, rc, errno);
        close(fd);
        return -2;
    }

    *ppBuf = malloc( *bufLen);
    if (*ppBuf == NULL)
    {    
        bcmuLog_error("Failed to allocate %d bytes buf", *bufLen);
        flock(fd, LOCK_UN);
        close(fd);
        return -3;
    }

    memset(*ppBuf, 0x0, *bufLen);
    if (!newfile)
    {
        rc = read(fd, *ppBuf, *bufLen);
        if (rc!= *bufLen)
        {
            bcmuLog_error("read file expect %d bytes but reurn %d bytes", *bufLen, rc);
            flock(fd, LOCK_UN);
            close(fd);
            free(*ppBuf);
            return -4;
        }
    }
    else
    {
        rc = write(fd, *ppBuf, *bufLen);
        if (rc != *bufLen)
        {
            bcmuLog_error("bad write on new file, got %d expected %d", rc, *bufLen);
            flock(fd, LOCK_UN);
            close(fd);
            free(*ppBuf);
            return -5;
        }
    }

    return fd;
}

/** Free data buffer and unlock and close the scratchpad file. 
 * @return zero success
 *         negaive number fail
 */

int putScratchPadFile(int fd, char* pBuf, UINT32 bufLen, int update)
{
    int rc;
    int sts = 0;

    if (update) 
    {
        lseek(fd, 0, SEEK_SET);
        rc = write(fd, pBuf, bufLen);
        if (rc != bufLen)
        {
            bcmuLog_error("bad write, got %d expected %d", rc, bufLen);
            sts = -1;
        }
    }

    /* release the file lock first */
    rc = flock(fd, LOCK_UN);
    if (rc != 0)
    {
        bcmuLog_error("failed to unlock fd %d rc %d error %d", fd, rc, errno);
        sts = -2;
    }

    free(pBuf);
    close(fd);

    return sts;
}

SINT32 devCtl_flashReadFile(const char *fname, char *buf,  UINT32 bufLen)
{
   BcmRet ret=BCMRET_SUCCESS;
   struct stat statbuf;
   int fd, rc;

   rc = stat(fname, &statbuf);
   if (rc != 0)
   {
       bcmuLog_error("file %s not exit stat rc %d", fname, rc);
       return BCMRET_INTERNAL_ERROR;
   }
   if (bufLen > statbuf.st_size) 
     bufLen = statbuf.st_size;

   fd = open(fname, O_RDONLY);
   if (fd < 0)
   {
      bcmuLog_error("could not open file %s, errno=%d", fname, errno);
      return BCMRET_INTERNAL_ERROR;
   }

   rc = flock(fd, LOCK_EX);
   if( rc != 0 )
   {
       bcmuLog_error("failed to lock file %s fd %d rc %d error %d", fname, fd, rc, errno);
       close(fd);
       return BCMRET_INTERNAL_ERROR;
   }

   rc = read(fd, buf, bufLen);
   if (rc != bufLen)
   {
      bcmuLog_error("read error, got %d, expected %d", rc, bufLen);
      ret = BCMRET_INTERNAL_ERROR;
   }

   flock(fd, LOCK_UN);
   close(fd);

   return ret;
}

SINT32 devCtl_flashWriteFile(const char *fname, char *buf,  UINT32 bufLen)
{
   BcmRet ret=BCMRET_SUCCESS;
   int fd, rc;

   if ((fd = open(fname, O_RDWR|O_CREAT|O_TRUNC)) < 0)
   {
      bcmuLog_error("could not open %s for write", fname);
      return BCMRET_INTERNAL_ERROR;
   }

   rc = flock(fd, LOCK_EX);
   if( rc != 0 )
   {
       bcmuLog_error("failed to lock file %s fd %d rc %d error %d", fname, fd, rc, errno);
       close(fd);
       return BCMRET_INTERNAL_ERROR;
   }

   rc = write(fd, buf, bufLen);
   if (rc < (SINT32) bufLen)
   {
      bcmuLog_error("write to %s failed, rc=%d", fname, rc);
      ret = BCMRET_INTERNAL_ERROR;
   }

   flock(fd, LOCK_UN);
   close(fd);

   return ret;
}


SINT32 devCtl_scratchPadList(const char *fileName, char *tokBuf, int bufLen)
{
    SINT32 sts;
    UINT32 spDataLen = 0;
    char *pBuf = NULL;
    PSP_TOKEN pToken = NULL;
    char *startPtr = NULL;
    int usedLen;
    int tokenNameLen=0;
    int copiedLen=0;
    int needLen=0;
    int fd;
     
    fd = getScratchPadFile(fileName, &pBuf, &spDataLen);
    if( fd < 0 ) 
        return 0;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        bcmuLog_error("Scratch pad is not initialized");
        if (putScratchPadFile(fd, pBuf, spDataLen, 0) < 0)
        {
            bcmuLog_error("putScratchPadFile() failed");
        }
        return 0;
    }

    // Walk through all the tokens
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;

    while( isalnum(pToken->tokenName[0]) && isascii(pToken->tokenName[0]) && pToken->tokenLen > 0 &&
           ((usedLen + pToken->tokenLen) <= spDataLen))
    {
        tokenNameLen = strlen(pToken->tokenName);
        needLen += tokenNameLen + 1;
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

    if (putScratchPadFile(fd, pBuf, spDataLen, 0) < 0)
    {
        bcmuLog_error("putScratchPadFile() failed");
    }

    return sts;
}


SINT32 devCtl_scratchPadGet(const char *fileName, char *tokenId, char *tokBuf, int bufLen)
{
    UINT32 spDataLen = 0;
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    char *startPtr = NULL;
    int usedLen;
    BcmRet sts = 0;
    int fd;

    if (NULL == tokenId)
    {
        return 0;
    }

    fd = getScratchPadFile(fileName, &pBuf, &spDataLen);
    if( fd < 0 ) 
        return 0;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        bcmuLog_error("Scratch pad is not initialized.\n");
        if (putScratchPadFile(fd, pBuf, spDataLen, 0) < 0)
        {
            bcmuLog_error("putScratchPadFile() failed");
        }
        return 0;
    }

    // search for the token
    usedLen = sizeof(SP_HEADER);
    startPtr = pBuf + sizeof(SP_HEADER);
    pToken = (PSP_TOKEN) startPtr;
    while( isalnum(pToken->tokenName[0]) && isascii(pToken->tokenName[0]) && pToken->tokenLen > 0 &&
        pToken->tokenLen < spDataLen && usedLen < spDataLen)
    {

        if (strncmp(pToken->tokenName, tokenId, TOKEN_NAME_LEN) == 0)
        {
            if ( pToken->tokenLen > bufLen )
            {
               // User may purposely pass in a 0 length buffer just to get
               // the size, so don't log this as an error.
               // printk("The length %d of token %s is greater than buffer len %d.\n", pToken->tokenLen, pToken->tokenName, bufLen);
                sts = pToken->tokenLen * (-1);
            }
            else
            {
                memcpy(tokBuf, startPtr + sizeof(SP_TOKEN), pToken->tokenLen);
                sts = pToken->tokenLen;
            }
            break;
        }

        usedLen += ((pToken->tokenLen + 0x03) & ~0x03);
        startPtr += sizeof(SP_TOKEN) + ((pToken->tokenLen + 0x03) & ~0x03);
        pToken = (PSP_TOKEN) startPtr;
    }

    if (putScratchPadFile(fd, pBuf, spDataLen, 0) < 0)
    {
        bcmuLog_error("putScratchPadFile() failed");
    }

    return sts;
}


SINT32 devCtl_scratchPadSet(const char *fileName, char *tokenId, char *tokBuf, int bufLen)
{
    UINT32 spDataLen = 0;
    PSP_TOKEN pToken = NULL;
    char *pBuf = NULL;
    SP_HEADER SPHead;
    SP_TOKEN SPToken;
    char *curPtr;
    SINT32 sts = -1;
    int tokenNameLen = 0;
    int fd;

    if( !tokenId || !isalnum(tokenId[0]) || !isascii(tokenId[0]) )
    {
        bcmuLog_error("Invalid scratch pad key name. Must start with an ascii letter or number.\n");
        return sts;
    }

    tokenNameLen = strlen(tokenId);
    if( tokenNameLen >= TOKEN_NAME_LEN )
    {
        bcmuLog_error("Token name length %d large than maximum length %d\n", tokenNameLen, TOKEN_NAME_LEN-1);
        return sts;
    }

    fd = getScratchPadFile(fileName, &pBuf, &spDataLen);
    if( fd < 0 ) 
        return 0;

    if( bufLen >= (int) (spDataLen - sizeof(SP_HEADER) - sizeof(SP_TOKEN)) )
    {
        bcmuLog_error("Scratch pad overflow by %zu bytes.  Information not saved.\n",
            bufLen  - spDataLen - sizeof(SP_HEADER) -
            sizeof(SP_TOKEN));
        if (putScratchPadFile(fd, pBuf, spDataLen, 0) < 0)
        {
            bcmuLog_error("putScratchPadFile() failed");
        }
        return sts;
    }


    // form header info.
    memset((char *)&SPHead, 0, sizeof(SP_HEADER));
    memcpy(SPHead.SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN);
    SPHead.SPVersion = SP_VERSION;

    // form token info.
    memset((char*)&SPToken, 0, sizeof(SP_TOKEN));
    strncpy(SPToken.tokenName, tokenId, tokenNameLen);
    SPToken.tokenLen = bufLen;

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0)
    {
        // new sp, so just flash the token
        bcmuLog_notice("No scratch pad found.  Initialize scratch pad...\n");
        memset(pBuf, 0x0, spDataLen);
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
        while( isalnum(pToken->tokenName[0]) && isascii(pToken->tokenName[0]) &&
            strlen(pToken->tokenName) < TOKEN_NAME_LEN &&
            pToken->tokenLen > 0 &&
            pToken->tokenLen < spDataLen &&
            usedLen < spDataLen )
        {
            usedLen += sizeof(SP_TOKEN) + skipLen;
            curPtr += sizeof(SP_TOKEN) + skipLen;
            pToken = (PSP_TOKEN) curPtr;
            skipLen = (pToken->tokenLen + 0x03) & ~0x03;
        }

        if( usedLen + SPToken.tokenLen + sizeof(SP_TOKEN) >
            spDataLen )
        {
            bcmuLog_error("Scratch pad overflow by %d bytes.  Information not saved.\n",
                (usedLen + SPToken.tokenLen + (int)sizeof(SP_TOKEN)) -
                spDataLen);
            if (putScratchPadFile(fd, pBuf, spDataLen, 0) < 0)
            {
                bcmuLog_error("putScratchPadFile() failed");
            }
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
                        spDataLen - (curLen + copyLen) );
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

    }

    sts = putScratchPadFile(fd, pBuf, spDataLen, 1);

    return sts;
}


SINT32 devCtl_scratchPadClearAll(const char *fileName)
{ 
    UINT32 spDataLen = 0;
    SINT32 sts = -1;
    char *pBuf = NULL;
    int fd;
 
    fd = getScratchPadFile(fileName, &pBuf, &spDataLen);
    if( fd < 0 ) 
        return 0;

    memset(pBuf, 0x00, spDataLen);

    sts = putScratchPadFile(fd, pBuf, spDataLen, 1);

    return sts;
}

#endif  /* ifndef DESKTOP_LINUX */

#if defined(EPON_SDK_BUILD)
UINT32 devCtl_getPortMacType(unsigned short port, unsigned int *mac_type)
{
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_PORT_MAC_TYPE,
                           0, NULL,
                           0, port, mac_type);
   return ret;
}

UINT32 devCtl_getNumFePorts(unsigned int *fe_ports)
{
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_NUM_FE_PORTS,
                           0, NULL,
                           0, 0, fe_ports);
   return ret;
}

UINT32 devCtl_getNumGePorts(unsigned int *ge_ports)
{
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_NUM_GE_PORTS,
                           0, NULL,
                           0, 0, ge_ports);
   return ret;
}

UINT32 devCtl_getNumVoipPorts(unsigned int *voip_ports)
{
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_NUM_VOIP_PORTS,
                           0, NULL,
                           0, 0, voip_ports);
   return ret;
}

UINT32 devCtl_getPortMap(unsigned int *port_map)
{
   BcmRet ret;
   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_SWITCH_PORT_MAP,
                           0, NULL,
                           0, 0, port_map);
   return ret;
}
#endif  /* EPON_SDK_BUILD */

UINT32 devCtl_getSdramSize(void)
{
   UINT32 size;
   BcmRet ret;
#ifdef DESKTOP_LINUX
   size = 32*1024*1024;
   ret = BCMRET_SUCCESS;
#else   
   ret = devCtl_boardDriverIoctl(BOARD_IOCTL_GET_SDRAM_SIZE,
                           0, NULL,
                           0, 0, &size);
#endif
   return (ret == BCMRET_SUCCESS ? size : 0);
}

BcmRet devCtl_getChipId(UINT32 *chipId)
{
    return( devCtl_boardDriverIoctl(BOARD_IOCTL_GET_CHIP_ID, 0, NULL, 0, 0, chipId) );
}


BcmRet devCtl_boardDriverIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data)
{
    BOARD_IOCTL_PARMS ioctlParms;
    SINT32 boardFd = 0;
    SINT32 rc = 0;
    BcmRet ret=BCMRET_SUCCESS;

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
            */
           ret = (BcmRet)ioctlParms.result;
        }
        else
        { 
           if (rc < 0)
           {
              bcmuLog_debug("boardIoctl=0x%x action=%d rc=%d errno=%d", boardIoctl, action, rc, errno);
              ret = BCMRET_INVALID_ARGUMENTS;
           }
        
           /* ioctl specific return data */
           if (ret == BCMRET_SUCCESS)
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
                  (boardIoctl == BOARD_IOCTL_FLASH_READ && action == FLASH_SIZE) ||
                  (boardIoctl == BOARD_IOCTL_GET_FLASH_TYPE) ||
                  (boardIoctl == BOARD_IOCTL_GETSET_BOOT_INACTIVE_IMAGE))
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
       bcmuLog_error("Unable to open device %s", BOARD_DEVICE_NAME);
       ret = BCMRET_INTERNAL_ERROR;
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
   int ret = 0;

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
          bcmuLog_debug("fake whole image write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBcmImageSetWhole((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
 
      case BCM_IMAGE_CFE:
          bcmuLog_debug("fake cfe write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBcmImageSetCfe((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
         
      case BCM_IMAGE_FS:
          bcmuLog_debug("fake fs+kernel write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_flashFsKernelImage((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
          
      case PERSISTENT:
          bcmuLog_debug("fake config write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysPersistentSet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;

#ifdef SUPPORT_BACKUP_PSI
      case BACKUP_PSI:
          bcmuLog_debug("fake backup config write, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBackupPsiSet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
#endif

      default:
         ret = -EINVAL;
         bcmuLog_error("unhandled action %d in BOARD_IOCTL_FLASH_WRITE", ctrlParms->action);
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
         ctrlParms->result = 32 * 1024 * 1024; /* 4 MB? */
         ret = 0;
         break;

      case PERSISTENT:
          bcmuLog_debug("fake config read, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysPersistentGet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;

#ifdef SUPPORT_BACKUP_PSI
      case BACKUP_PSI:
          bcmuLog_debug("fake backup psi config read, buf=%p len=%d", ctrlParms->string, ctrlParms->strLen);
          ret = fake_kerSysBackupPsiGet((UINT8 *) ctrlParms->string, ctrlParms->strLen);
          break;
#endif

      default:
         ret = -EINVAL;
         ctrlParms->result = ret;
         bcmuLog_error("unhandled action %d in BOARD_IOCTL_FLASH_READ", ctrlParms->action);
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
         bcmuLog_error("unhandled action %d in BOARD_IOCTL_FLASH_READ", ctrlParms->action);
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
#if defined(CHIP_63268)
      ctrlParms->result = 0x63268;
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
         bcmuLog_error("buf too short, need 5, got %d", ctrlParms->strLen);
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
            bcmuLog_error("buf too short, need %zu, got %d", strlen(boardIdStr)+1, ctrlParms->strLen);
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
              ctrlParms->result = strlen("DESKTOP_LINUX");
              strcpy(ctrlParms->string, "DESKTOP_LINUX");
              break;

          case BOOT_GET_BOOTED_IMAGE_ID:
              ctrlParms->result = BOOTED_PART1_IMAGE;
              break;

          default:
              ret = -EINVAL;
              ctrlParms->result = ret;
              bcmuLog_error("unhandled offset %d in "
                "BOARD_IOCTL_BOOT_IMAGE_OPERATION", ctrlParms->offset);
              break;
      }
      break;

   case BOARD_IOCTL_GET_SEQUENCE_NUMBER:
      ctrlParms->result = 0;
      break;

   case BOARD_IOCTL_MIPS_SOFT_RESET:
      bcmuLog_notice("Soft Reset...\n");
      break;

   default:
      ret = -EINVAL;
      bcmuLog_error("unhandled board ioctl 0x%x", command);
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
        bcmuLog_error("could not get sharedBlks");
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        bcmuLog_error("Scratch pad is not initialized.\n");
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
        bcmuLog_debug("found tokenName=%s copiedLen=%d needLen=%d bufLen=%d", pToken->tokenName, copiedLen, needLen, bufLen);
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
        bcmuLog_error("could not get sharedBlks");
        return sts;
    }

    // pBuf points to SP buf
    pBuf = pShareBuf + fInfo.flash_scratch_pad_blk_offset;  

    if(memcmp(((PSP_HEADER)pBuf)->SPMagicNum, MAGIC_NUMBER, MAGIC_NUM_LEN) != 0) 
    {
        bcmuLog_error("Scratch pad is not initialized.\n");
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
                bcmuLog_error("The length %d of token %s is greater than buffer len %d.", pToken->tokenLen, pToken->tokenName, bufLen);
                sts = pToken->tokenLen * (-1);
            }
            else
            {
                bcmuLog_debug("found token %s, copying out (len=%d)", tokenId, pToken->tokenLen);
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
        bcmuLog_error("Scratch pad overflow by %zu bytes.  Information not saved.\n",
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
        bcmuLog_notice("No scratch pad found.  Initialize scratch pad...\n");
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
            bcmuLog_error("Scratch pad overflow by %zu bytes.  Information not saved.\n",
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
   char path[1024]={0};
   struct stat statbuf;
   int rc, fd;
   BcmRet ret;

   bcmuLog_debug("reading block %d through %d", start_block, start_block+num_blocks);

   if (start_block != 0)
   {
      bcmuLog_error("cannot handle non-zero start block yet.");
      return NULL;
   }

   if (num_blocks > FAKE_NUM_PSP_FLASH_BLOCKS)
   {
      bcmuLog_error("requested more blocks than PSP flash blocks, not handled.");
      return NULL;
   }


   /* first allocate the buffer we will need for the read */
   bufLen = FAKE_FLASH_BLOCK_SIZE * FAKE_NUM_PSP_FLASH_BLOCKS;
   if ((buf = malloc(bufLen)) == NULL)
   {
      bcmuLog_error("malloc of %d bytes failed", bufLen);
      return NULL;
   }
   memset(buf, 0x0, sizeof(bufLen));

   /* form path to the flash file */
   if ((ret = bcmUtl_getBaseDir(path, sizeof(path))) != BCMRET_SUCCESS)
   {
      bcmuLog_error("getBaseDir failed, abort func");
      free(buf);
      return NULL;
   }
   else
   {
      UINT32 offset;

      offset = strlen(path);
      snprintf(&(path[offset]), sizeof(path)-offset, "/%s", FAKE_FLASH_PSP_FILENAME);
   }


   bcmuLog_debug("checking for flash file at %s", path);
   if ((rc = stat(path, &statbuf)) < 0)
   {
      bcmuLog_debug("creating fake flash file and initialize to zeros");
      fd = open(path, O_CREAT|O_RDWR, 0644);
      if (fd < 0)
      {
         bcmuLog_error("create of flash file %s failed, errno=%d", path, errno);
         free(buf);
         return NULL;
      }

      /* fill rest of file with zeros */
      rc = write(fd, buf, bufLen);
      bcmuLog_debug("filler write returned %d", rc);

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
      bcmuLog_error("unexpected rc %d from read, expected %d",
                   rc, num_blocks * FAKE_FLASH_BLOCK_SIZE);
      free(buf);
      buf = NULL;
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
   char path[1024]={0};
   BcmRet ret;


   bcmuLog_debug("setting block %d through %d, buf %p", start_block, start_block+total_blocks, pBuf);

   if (start_block != 0)
   {
      bcmuLog_error("cannot handle non-zero start block yet.");
      return sts;
   }

   if (total_blocks > FAKE_NUM_PSP_FLASH_BLOCKS)
   {
      bcmuLog_error("requested more blocks than PSP flash blocks, not handled.");
      return sts;
   }

   /* form path to the flash file */
   if ((ret = bcmUtl_getBaseDir(path, sizeof(path))) != BCMRET_SUCCESS)
   {
      bcmuLog_error("getBaseDir failed, abort func");
      return sts;
   }
   else
   {
      UINT32 offset;

      offset = strlen(path);
      snprintf(&(path[offset]), sizeof(path)-offset, "/%s", FAKE_FLASH_PSP_FILENAME);
   }

   bcmuLog_debug("opening fake flash file at %s", path);
   if ((fd = open(path, O_RDWR)) < 0)
   {
      bcmuLog_error("Could not open %s", path);
      return sts;
   }

   rc = write(fd, pBuf, total_blocks * FAKE_FLASH_BLOCK_SIZE);
   if (rc != total_blocks * FAKE_FLASH_BLOCK_SIZE)
   {
      bcmuLog_error("bad write, got %d expected %d", rc, total_blocks * FAKE_FLASH_BLOCK_SIZE);
   }
   else
   {
      bcmuLog_debug("%d blocks (%d bytes) written", total_blocks, total_blocks * FAKE_FLASH_BLOCK_SIZE);
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
   free(pShareBuf);

   return;
}


SINT32 fake_flashRead(const char *filename, UINT8 *buf, UINT32 bufLen)
{
   char basedir[1024]={0};
   SINT32 rc, basedirLen;
   UINT32 localBufLen = bufLen;
   BcmRet ret;

   if ((ret = bcmUtl_getBaseDir(basedir, sizeof(basedir))) != BCMRET_SUCCESS)
   {
      bcmuLog_error("getBasedir failed, ret=%d", ret);
      return ret;
   }

   basedirLen = strlen(basedir);
   rc = snprintf(&(basedir[basedirLen]), sizeof(basedir) - basedirLen, "/%s", filename);
   if (rc >= ((SINT32) sizeof(basedir)) - basedirLen)
   {
      bcmuLog_error("basedir var not long enough to hold entire path");
      return BCMRET_RESOURCE_EXCEEDED;
   }

   if (sysUtil_isFilePresent(basedir))
   {
      // localBufLen is not used.
      ret = sysUtil_copyToBuffer(basedir, buf, &localBufLen);
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
   char basedir[1024]={0};
   SINT32 fd, rc, basedirLen;
   BcmRet ret;

   if ((ret = bcmUtl_getBaseDir(basedir, sizeof(basedir))) != BCMRET_SUCCESS)
   {
      bcmuLog_error("getBasedir failed, ret=%d", ret);
      return ret;
   }

   basedirLen = strlen(basedir);
   rc = snprintf(&(basedir[basedirLen]), sizeof(basedir) - basedirLen, "/%s", filename);
   if (rc >= ((SINT32) sizeof(basedir)) - basedirLen)
   {
      bcmuLog_error("basedir var not long enough to hold entire path");
      return BCMRET_RESOURCE_EXCEEDED;
   }

   fd = open(basedir, O_RDWR|O_CREAT|O_TRUNC, 0666);
   if (fd < 0)
   {
      bcmuLog_error("Could not open %s, errno=%d", basedir, errno);
      return BCMRET_INTERNAL_ERROR;
   }

   rc = write(fd, buf, bufLen);
   if (rc != (SINT32) bufLen)
   {
      bcmuLog_error("write failed, expected %u got %d", bufLen, rc);
      ret = BCMRET_INTERNAL_ERROR;
   }
   else
   {
      bcmuLog_debug("wrote buf len=%u to %s", bufLen, basedir);
      ret = BCMRET_SUCCESS;
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
