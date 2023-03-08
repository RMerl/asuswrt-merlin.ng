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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // types
#include <string.h>
#include <net/if.h>
#include <fcntl.h> // for open
#include <linux/errno.h>
#include <linux/kernel.h>
#include <sys/ioctl.h>
#include <unistd.h> // close

#include "board.h" /* in bcmdrivers/opensource/include/bcm963xx, for BCM_IMAGE_CFE */
#include "bcm_boarddriverctl.h"
#include "bcm_ulog.h"
#include "sysutil_fs.h"

#include "bcm_flashutil.h"
#include "bcm_flashutil_private.h"
#include "bcm_flashutil_nand.h"
#include "bcm_flashutil_emmc.h"
#include "bcm_flashutil_nor.h"
#include "flash_api.h"
#include <genutil_crc.h>
#include "os_defs.h"



#define IS_ERR_OR_NULL(x) ((x)==0)
#define IS_ERR(x) ((x)<0)
#define IS_NULL(x) ((x)==0)

#define ERROR -1
#define SUCCESS 0

/* #define FLASHUTIL_DEBUG 1 */
#if defined(FLASHUTIL_DEBUG)
#define flashutil_debug(fmt, arg...) \
  fprintf(stderr, "%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
#define flashutil_debug(fmt, arg...)
#endif
#define flashutil_print(fmt, arg...) \
  fprintf(stderr, "%s.%u: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define flashutil_error(fmt, arg...) \
  fprintf(stderr, "ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)


/*
 * gFlashInfo is the cached flash info variable.  0 means no value yet.
 * non-zero means we already got the value and don't need to run low level
 * functions anymore.
 */
static unsigned int gFlashInfo = 0;

#ifndef DESKTOP_LINUX

#define UPDATE_FLASH_INFO()    if(gFlashInfo == 0) {getFlashInfo(&gFlashInfo);}

#ifdef DISABLE_NOR_RAW_PARTITION
static int disable_nor_raw_partition = 1;
#else
static int disable_nor_raw_partition = 0;
#endif

/* Prototypes */
static unsigned int pkgtb_getCrc32(const unsigned char *pdata, unsigned int size, unsigned int crc_initial);
#endif /* DESKTOP_LINUX */


/** Main entry point for board ioctl actions.  See comments in
 *  bcm_boarddriverctl.h for detailed info about this function.
 */
BcmRet devCtl_boardIoctl(UINT32 boardIoctl,
                         BOARD_IOCTL_ACTION action,
                         char *string,
                         SINT32 strLen,
                         SINT32 offset,
                         void *data)
{
#ifndef DESKTOP_LINUX
    BcmRet ret=BCMRET_SUCCESS;

    UPDATE_FLASH_INFO();

    if( (((boardIoctl == BOARD_IOCTL_FLASH_READ || boardIoctl == BOARD_IOCTL_FLASH_WRITE) &&
        ((action == SCRATCH_PAD) || (action == PERSISTENT) || (action == BACKUP_PSI) || (action == SYSLOG))) ||
        (boardIoctl == BOARD_IOCTL_FLASH_LIST)) &&
        ((gFlashInfo & FLASH_INFO_FLAG_NOR) == 0 || disable_nor_raw_partition == 1)
      )
    {
        ret = devCtl_flashConfigAccess(boardIoctl, action, string, strLen, offset, data);
        return ret;
    }
#endif

   return (devCtl_boardDriverIoctl(boardIoctl, action,
                                   string, strLen, offset, data));
}

#ifndef DESKTOP_LINUX
void composeConfigFileName(UINT32 boardIoctl, BOARD_IOCTL_ACTION action,
                       const char *compName, char *fname, unsigned int len);
                           
BcmRet devCtl_flashConfigAccess(UINT32 boardIoctl, BOARD_IOCTL_ACTION action,
                                char *string, SINT32 strLen,
                                SINT32 offset, void *data)
{
    BcmRet ret = BCMRET_SUCCESS;

    if( action == SCRATCH_PAD )
    {
        switch(boardIoctl)
        {
        case BOARD_IOCTL_FLASH_LIST:
            ret = devCtl_scratchPadList(SCRATCH_PAD_FILE_NAME, data, offset);
            break;

        case BOARD_IOCTL_FLASH_READ:
            ret = devCtl_scratchPadGet(SCRATCH_PAD_FILE_NAME, string, data, offset);
            break;

        case BOARD_IOCTL_FLASH_WRITE:
            if( offset == -1 )
                ret = devCtl_scratchPadClearAll(SCRATCH_PAD_FILE_NAME);
            else
                ret = devCtl_scratchPadSet(SCRATCH_PAD_FILE_NAME, string, data, offset);
            break;
        default:
            bcmuLog_error("unhandled BOARD_IOCTL code %d for scratch pad", boardIoctl);
            ret = BCMRET_INVALID_ARGUMENTS;
            break;
        }
    }
    else
    {
        char fname[MAX_PSI_FILE_NAME_LEN]={0};

        switch(action)
        {
        case PERSISTENT:
        case BACKUP_PSI:
            composeConfigFileName(boardIoctl, action, (char *)data,
                                  fname, sizeof(fname));
            break;

        case SYSLOG:
            snprintf(fname, sizeof(fname), "%s", SYSLOG_FILE_NAME);
            break;

        default:
            bcmuLog_error("unhandled action %d in BOARD_IOCTL code %d",
                          action, boardIoctl);
            return BCMRET_INVALID_ARGUMENTS;
        }

        if ( boardIoctl == BOARD_IOCTL_FLASH_READ )
        {
            bcmuLog_notice("reading from fname %s", fname);
            ret = devCtl_flashReadFile(fname, string, strLen);
        }
        else if( boardIoctl == BOARD_IOCTL_FLASH_WRITE )
        {
            bcmuLog_notice("writing to fname %s (len=%d)", fname, strLen);
            ret = devCtl_flashWriteFile(fname, string, strLen);
            sync();
        }
        else
        {
            bcmuLog_error("unhandled BOARD_IOCTL code %d", boardIoctl);
            ret = BCMRET_INVALID_ARGUMENTS;
        }
    }

    return ret;
}


void composeConfigFileName(UINT32 boardIoctl, BOARD_IOCTL_ACTION action,
                           const char *compName, char *fname, unsigned int len)
{
    // Distributed MDM config files has _compName.
    if (compName != NULL && compName[0] != '\0')
    {
        if (action == PERSISTENT)
            snprintf(fname, len, "%s_%s", PSI_FILE_NAME, compName);
        else
            snprintf(fname, len, "%s_%s", PSI_BACKUP_FILE_NAME, compName);
    }
    else
    {
        if (action == PERSISTENT)
            snprintf(fname, len, "%s", PSI_FILE_NAME);
        else
            snprintf(fname, len, "%s", PSI_BACKUP_FILE_NAME);
    }

    if (boardIoctl == BOARD_IOCTL_FLASH_WRITE)
    {
        // There is no fallback for write operation.  Write it to which ever
        // file was specified.
        return;
    }

    if (sysUtil_isFilePresent(fname))
    {
        return;
    }

    // Distributed MDM filename does not exist, fallback to monolithic name
    bcmuLog_notice("%s does not exist, fallback", fname);
    if (action == PERSISTENT)
       snprintf(fname, len, "%s", PSI_FILE_NAME);
    else
       snprintf(fname, len, "%s", PSI_BACKUP_FILE_NAME);

    return;
}

#endif  /* DESKTOP_LINUX */


/** read the nvramData struct from the in-memory copy of nvram.
 * The caller is not required to have flashImageMutex when calling this
 * function.  However, if the caller is doing a read-modify-write of
 * the nvram data, then the caller must hold flashImageMutex.  This function
 * does not know what the caller is going to do with this data, so it
 * cannot assert flashImageMutex held or not when this function is called.
 *
 * @return pointer to NVRAM_DATA buffer which the caller must free
 *         or NULL if there was an error
 */
int readNvramData(void *nvramData)
{
#ifndef DESKTOP_LINUX
    int ret = 0;
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            ret = nandReadNvramData(nvramData);
        break;

        case FLASH_INFO_FLAG_EMMC:
            ret = emmcReadNvramData(nvramData);
        break;

        default:
        break;
    }
    return ret;
#else
    flashutil_print("readNvramData(%p)", nvramData);
    return 1;
#endif
}


int getFlashInfo(unsigned int *flags)
{
    if (flags == NULL)
    {
       fprintf(stderr, "flags is NULL!");
       return ERROR;
    }

    if (gFlashInfo != 0)
    {
        /* use cached result */
        *flags = gFlashInfo;
        return SUCCESS;
    }

    return (getFlashInfoUncached(flags));
}

int getFlashInfoUncached(unsigned int *flags)
{
    if (flags == NULL)
    {
       fprintf(stderr, "flags is NULL!");
       return ERROR;
    }
    else
    {
       *flags = 0;
    }

#ifndef DESKTOP_LINUX
    {
        /* First try NAND, and if not NAND, see if it is EMMC.
         * TOOD: There seems to be a bug in libc where if you run several
         * popens back to back, libc loses track of the fp and the fgets hangs.
         * So in the emmc case, we would first run a popen in nandIsBootDevice,
         * and then immediately run another popen in emmcIsBootDevice, so 
         * there might be a hang.  If so, put a sleep(1) before the call to
         * emmcIsBootDevice().
         */
        if( nandIsBootDevice() )
        {
            *flags = FLASH_INFO_FLAG_NAND;
        }
        else if( emmcIsBootDevice() )
        {
            *flags = FLASH_INFO_FLAG_EMMC;
        }
        else
        {
            /* If not NAND and not EMMC, default to NOR */
            *flags = FLASH_INFO_FLAG_NOR;
        }
    }
#else
    *flags = FLASH_INFO_FLAG_NAND;
#endif

    /* cache the result */
    gFlashInfo = *flags;

    return SUCCESS;
}

int getFlashTotalSize(unsigned int *size)
{
    unsigned int value = 0;
    BcmRet ret = BCMRET_SUCCESS;

#ifdef DESKTOP_LINUX
    value = 512*1024*1024; // 512MB
#else
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            ret = get_mtd_master_size(&value);
        break;
        case FLASH_INFO_FLAG_EMMC:
            ret = emmcGetFlashSize(&value);
        break;
        case FLASH_INFO_FLAG_NOR:
        default:
            fprintf(stderr, "Not implemented yet.\n");
        break;
    }
#endif
    *size = value;

    return ret;
}

unsigned int get_flash_type(void)
{
    unsigned int value = 0;

    devCtl_boardIoctl(BOARD_IOCTL_GET_FLASH_TYPE, 0, NULL, 0, 0, (void *)&value);

    return(value);
}

unsigned int otp_is_btrm_boot(void)
{
    unsigned int value = 0;
#ifndef DESKTOP_LINUX
    devCtl_boardIoctl(BOARD_IOCTL_GET_BTRM_BOOT, 0, NULL, 0, 0, (void *)&value);
#endif
    return(value);
}

unsigned int otp_is_boot_secure(void)
{
    unsigned int value = 0;
#ifndef DESKTOP_LINUX
    devCtl_boardIoctl(BOARD_IOCTL_GET_BOOT_SECURE, 0, NULL, 0, 0, (void *)&value);
#endif
    return(value);
}

/* Only gen3 bootroms and later support a manufacturing secure stage (ie 4908, 6858, etc) */
unsigned int otp_is_boot_mfg_secure(void)
{
    unsigned int value = 0;
#ifndef DESKTOP_LINUX
    devCtl_boardIoctl(BOARD_IOCTL_GET_BOOT_MFG_SECURE, 0, NULL, 0, 0, (void *)&value);
#endif
    return(value);
}


unsigned int get_chip_id(void)
{
    unsigned int chip_id = 0;
#ifndef DESKTOP_LINUX
    devCtl_boardIoctl(BOARD_IOCTL_GET_CHIP_ID, 0, NULL, 0, 0, (void *)&chip_id);
#endif
    return(chip_id);
}


#ifndef DESKTOP_LINUX
unsigned int getset_boot_inactive_image(int flag)
{
    unsigned int value = 0;
    devCtl_boardIoctl(BOARD_IOCTL_GETSET_BOOT_INACTIVE_IMAGE, 0, NULL, 0, flag, (void *)&value);
    return(value);
}
#else
unsigned int getset_boot_inactive_image(int flag __attribute__((unused)))
{
    return(0);
}
#endif


int devCtl_getSequenceNumber(int image)
{
    unsigned int flag=0;

    getFlashInfo(&flag);
    if (flag & (FLASH_INFO_FLAG_NAND | FLASH_INFO_FLAG_EMMC)) // NAND flash, no need to dig into kernel space
        return(getSequenceNumber(image));
    else
        return( (int) devCtl_boardIoctl(BOARD_IOCTL_GET_SEQUENCE_NUMBER, 0, NULL, 0, image, NULL) );
}

int getSequenceNumber(int imageNumber)
{ /* NAND Flash */
#ifndef DESKTOP_LINUX
    int seqNumber = -1;
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            seqNumber = nandGetSequenceNumber(imageNumber);
        break;

        case FLASH_INFO_FLAG_EMMC:
            seqNumber = emmcGetSequenceNumber(imageNumber);
        break;

        case FLASH_INFO_FLAG_NOR:
            if( !isLegacyFlashLayout() )
            {
                //FIXME: SPINOR flash only support single image.
                seqNumber = imageNumber;
            }
        break;

        default:
        break;
    }
#else
    int seqNumber = imageNumber;
#endif /* DESKTOP_LINUX */

    return(seqNumber);
}

int getNextSequenceNumber( int seqNumImg1, int seqNumImg2 )
{
    int seq_num = -1;

    if( (seqNumImg1 == -1) && (seqNumImg2 == -1) )
        seq_num = 999;
    else
        seq_num = seqNumImg1 > seqNumImg2 ? seqNumImg1+1:seqNumImg2+1;

    /* Handle wrap-around case */
    if( seq_num > 999 )
        seq_num = 0;

    /* Handle zero case */
    if( (seq_num == seqNumImg1) || (seq_num == seqNumImg2) )
        seq_num++;

    return seq_num;
}

//#define DUMP_VER_STR 1
int devCtl_getImageVersion(int partition, char *verStr, int verStrSize)
{
    char tag[BUFLEN_256+1]={0};
    int status, i;
    int start = -1;
    int strSize = -1;
    int end = -1;
    int keyLen = strlen("$imageversion");
 
    memset(tag, 0x0, sizeof(tag));
    status = bcmFlash_getIdent(partition, &start, &end, "imageversion",
                               tag, sizeof(tag)-1);

#ifdef DUMP_VER_STR
    printf("%s: Image version tag[%d]:\n", __FUNCTION__, status);
    for(i=0; i< BUFLEN_256+1; i++)
    {
	    printf("%c", tag[i]);
    }
    printf("\n");
#endif

    if (status > keyLen)
    {
        for (i = keyLen; i<(BUFLEN_256+1) && i<status; i++)
        {
            /* Skip the leading characters that are not meaningful. */
            if ((tag[i] == ' ') || (tag[i] == ':') || (tag[i] == '$'))
		    continue;
            else
                break;
        }
        strSize = ((status - i) >= verStrSize) ? 
			verStrSize : (status - i);
#ifdef DUMP_VER_STR
	printf("%s: offset:%d, size:%d \n",__FUNCTION__,  i, strSize);
#endif

        memcpy(verStr, &tag[i], strSize);
        status = strSize;
    }
    else
    {
        status = -1;
        fprintf(stderr, "\nImage version not found.\n");
    }
 
    return(status);
 
}

uint64_t getSysfsBytes(char * pathname)
{
    FILE *fp;
    char line[256]={0};
    char *temp;
    uint64_t bytes = 0;

    if( access( pathname, F_OK ) == 0 ) 
    {
        fp = fopen(pathname,"r");
        if (fp == NULL)
        {
           printf("%s: Error! Could not open %s!\n", __FUNCTION__, pathname);
        }
        else
        {
            if ( fgets(line, sizeof(line), fp) )
                bytes = strtoull(line, &temp, 10);
            else
                printf("%s: Error reading sysfs entry %s!\n", __FUNCTION__, pathname);

            fclose(fp);
        }
    }
    return bytes;
}

#ifdef DESKTOP_LINUX

int devCtl_getImageState(void)
{
    return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                             0, NULL, 0, BOOT_GET_BOOT_IMAGE_STATE, NULL) );
}

int devCtl_getBootedImagePartition(void)
{
    return( (int) devCtl_boardDriverIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                                0, NULL, 1, BOOT_GET_BOOTED_IMAGE_ID, NULL) );
}

int getImageVersion(uint8_t *imagePtr __attribute__((unused)), int imageSize __attribute__((unused)), char *image_name, int image_name_len)
{
    // See also bcm_boardctl/linux/board.c BOOT_GET_IMAGE_VERSION
    strncpy(image_name, "DESKTOPLINUX", image_name_len);
    return 0;
}

int setBootImageState(int newState)
{
   /* this will go to the fake boardioctl handler */
   return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                                   0, NULL, 0, newState, NULL) );
}

int devCtl_setImageState(int state)
{
   /* this will go to the fake boardioctl handler */
   return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                                   0, NULL, 0, state, NULL) );
}

int getBootImageState(void)
{
   /* this will go to the fake boardioctl handler */
   return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                         0, NULL, 0, BOOT_GET_BOOT_IMAGE_STATE, NULL) );
}

int getBootedValue(void)
{
    /* this will go to the fake boardioctl handler */
    return( (int) devCtl_boardDriverIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                                          0, NULL, 1, BOOT_GET_BOOTED_IMAGE_ID,
                                          NULL) );
}

int verifyImageDDRType(uint32_t wfiFlags __attribute__((unused)), PNVRAM_DATA pNVRAM __attribute__((unused)))
{
    return 0;
}

int getBootPartition( void )
{
    return 1;
}

int getUpgradePartition( void )
{
    return 1;
}

int commit( int partition __attribute__((unused)), char *string __attribute__((unused)))
{
    return 0;
}

int bcmFlash_getIdent(int part __attribute__((unused)), int *start __attribute__((unused)), int *end __attribute__((unused)), const char * key __attribute__((unused)), char * line __attribute__((unused)), int len __attribute__((unused)))
{
    return 0;
}

int isLegacyFlashLayout(void)
{
    return -1;
}

int setImgSeqNum( int img_idx __attribute__((unused)), int seq __attribute__((unused)))
{
    return 0;
}

int getImgSeqNum( int img_idx __attribute__((unused)), int * seq __attribute__((unused)))
{
    return 0;
}
#else  /* end of DESKTOP_LINUX, start of real system code */

int verifyImageDDRType(uint32_t wfiFlags, PNVRAM_DATA pNVRAM)
{
#if defined(DDR_TYPE_CHECK)
     uint32_t brdtype, imgtype;
     int rc = -1;
     char* brdstr, *imgstr;

     if( pNVRAM ) {
       brdtype = pNVRAM->ulMemoryConfig&BP_DDR_TYPE_MASK;

         imgtype = wfiFlags&WFI_FLAG_DDR_TYPE_MASK;
	 brdstr = (brdtype == BP_DDR_TYPE_DDR3) ? "DDR3" : "DDR4";

         if( imgtype == WFI_FLAG_DDR_TYPE_NONE ) {
             fprintf(stderr, "\nImage ddr type not set. Probably an old image that does not support DDR type flag.\n");
             fprintf(stderr, "If you are sure the image has same DDR type as the board DDR type %s, use CFE and force command to update.\n", brdstr);
             rc = -1;
         } else {
             imgstr = (imgtype == WFI_FLAG_DDR_TYPE_DDR3) ? "DDR3" : "DDR4";
             if( imgstr[3] != brdstr[3] ) {
                 fprintf(stderr, "\nMismatch image ddr type %s board ddr type %s, flash aborted!!!\n", imgstr, brdstr);
                 rc = -1;
             } else
                 rc = 0;
         }
     }

     return rc;
#else
     return 0;
#endif  /* DDR_TYPE_CHECK */
}

uint64_t getAvailImgSpace(int update_img_idx)
{
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            return(nandGetAvailImgSpace(update_img_idx));
        break;
        case FLASH_INFO_FLAG_EMMC:
            return(emmcGetAvailImgSpace(update_img_idx));
        break;
        case FLASH_INFO_FLAG_NOR:
            if( !isLegacyFlashLayout())
            {
                //FIXME  return whole flash size.
                return(spinorGetAvailSpace(SPI_NOR_MTD));
            }
        break;
        default:
        break;
    }
    return 0;
}

uint64_t getAvailLoaderSpace(int update_img_idx)
{
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            return(nandGetAvailLoaderSpace());
        break;
        case FLASH_INFO_FLAG_EMMC:
            //FIXME: Pass image index to emmc
            return(emmcGetAvailLoaderSpace());
        break;
        case FLASH_INFO_FLAG_NOR:
            return( spinorGetAvailSpace(SPI_NOR_LOADER_MTD));
        break;

        default:
        break;
    }
    return 0;
}

int synchLoaderEnv( char * loader_fname )
{
    char * in_mem_env = NULL;
    FILE* file_ptr;
    char * ptr = NULL;
    int tmp_buf_len = 255;
    char buffer[tmp_buf_len];
    uint64_t env_size;
    char * retstr = NULL;
    int ret = -1;
    uint32_t env_magic = 0;
    int bytes;
    int written_bytes=0;
    UBOOT_ENV_HDR * env_hdr = NULL;
    char * boot_magic_ptr;
    int len;
    int i;
    
    /* Get env size */
    file_ptr = fopen(PROC_BOOT_MAGIC, "r");
    if( !file_ptr )
    {
        printf("%s: Error, cannot open %s\n", __FUNCTION__, PROC_BOOT_MAGIC);
        ret = -1;
        goto exit_loader_synch;
    }
    retstr = fgets((char*)buffer, tmp_buf_len, file_ptr);
    if( retstr )
    {
    	env_size = strtoul(buffer,&ptr, 10); 
    }
    else
    {
        printf("%s: Error, Read failed from %s\n", __FUNCTION__, PROC_BOOT_MAGIC);
        ret = -1;
        goto exit_loader_synch;
    }
    fclose(file_ptr);

    /* Delete env_boot_magic in flash so that it is updated by bootloader on next boot */
    if(system("echo 'env_boot_magic=' > /proc/nvram/set"))
    {
        printf("%s: Error, Could not clear 'env_boot_magic' env variable\n", __FUNCTION__);
        ret = -1;
        goto exit_loader_synch;
    }
    sync();

    /* Open and read the temp environment into memory */
    in_mem_env = malloc( env_size + sizeof(UBOOT_ENV_HDR));
    if( !in_mem_env )
    {
        printf("%s: Error, failed to allocate memory for env!\n", __FUNCTION__);
        ret = -1;
        goto exit_loader_synch;
    }
    file_ptr = fopen(PROC_ENV_RAW, "r"); 
    if( !file_ptr )
    {
        printf("%s: Error, cannot open %s\n", __FUNCTION__, PROC_ENV_RAW);
        ret = -1;
        goto exit_loader_synch;
    }
    bytes = fread(in_mem_env, 1, env_size + sizeof(UBOOT_ENV_HDR), file_ptr); 
    if( !bytes )
    {
        printf("%s: Error, Read failed from %s\n", __FUNCTION__, PROC_ENV_RAW);
        ret = -1;
        goto exit_loader_synch;
    }
    fclose(file_ptr);

    /* Clear bootmagic in in_memory_copy of env if not already cleared 
     * This captures those cases where customers disable /proc/nvram/set */
    for( i=sizeof(UBOOT_ENV_HDR); i<env_size; )
    {
        len = strlen(in_mem_env+i);
        boot_magic_ptr = strstr(in_mem_env+i, "env_boot_magic="); 
        if( boot_magic_ptr )
        {
            /* Check if env_boot_magic is already cleared */
            if( *(boot_magic_ptr + len - 1) == '=')
                break;

            /* Overwrite env_boot_magic variable */
            memcpy( in_mem_env+i, in_mem_env+i+len+1, env_size - (i + len + 1));
            /* Zero out the rear of the env */
            memset( in_mem_env+env_size-(len+1), '\0', len+1);
            break;
        }
        else
        {
            i += len + 1;
        }
    }

    /* Set proper header for in memory environment */
    printf("Read %d env bytes\n", (int)(bytes - sizeof(UBOOT_ENV_HDR)));
    env_hdr = (UBOOT_ENV_HDR*)in_mem_env;
    env_hdr->magic = UBOOT_ENV_MAGIC;
    env_hdr->size = (uint32_t)env_size;
    env_hdr->crc = pkgtb_getCrc32((unsigned char*)(in_mem_env + sizeof(UBOOT_ENV_HDR)), env_size-4, 0);

    /* Find environment in loader, overwrite it with current environment */
    file_ptr = fopen(loader_fname, "r+"); 
    if( !file_ptr )
    {
        printf("%s: Error, cannot open %s\n", __FUNCTION__, loader_fname);
        ret = -1;
        goto exit_loader_synch;
    }
    while( fread((char*)&env_magic, 4, 1, file_ptr) )
    {
        fseek( file_ptr, -4, SEEK_CUR );
        if( env_magic == UBOOT_ENV_MAGIC )
        {
            printf("Found MAGIC at 0x%08x\n", (unsigned int)ftell(file_ptr));
            ret = fwrite( in_mem_env, 1, env_size, file_ptr);
            if( !ret )
            {
                printf("%s: Error, Write failed to %s\n", __FUNCTION__, loader_fname);
                ret = -1;
                goto exit_loader_synch;
            }

            written_bytes+=env_size;
        }
	else
	{
            fseek( file_ptr, BOOT_MAGIC_OFFS, SEEK_CUR );
	}
    }

    if( !written_bytes )
    {
        printf("%s: Error loader environment not synched to current env!\n", __FUNCTION__);
        ret = -1;
    }
    else
        ret = 0;

exit_loader_synch:    
    if( in_mem_env )
        free(in_mem_env);

    if( file_ptr )
        fclose(file_ptr);

    return ret;
}

int getImageVersion(uint8_t *imagePtr, int imageSize, char *image_name, int image_name_len)
{
    int ret = -1;
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            ret = nandGetImageVersion(imagePtr, imageSize, image_name, image_name_len);
        break;

        case FLASH_INFO_FLAG_EMMC:
            ret = emmcGetImageVersion(imagePtr, imageSize, image_name, image_name_len);
        break;

        default:
        break;
    }
    return ret;
}


/***********************************************************************
 * Function Name: bcmFlash_getIdent
 * Description  : Get the ident value
 *  part        : Partition to search, 1 for first, 2 for second, 3 for boot, 4 for non-boot
 *  key         : ident key value, leave blank to return all ident values
 *  buf         : buffer to return ident values
 *  len         : length of passed buffer
 *  start       : starting block to search, -1 means from beginning or partition, returns block where tag was found
 *  end         : ending block to search, -1 means end of partition
 * Returns      : return size on success (0 if nothing found), -1 on failure 
 ***********************************************************************/
#define TRAILER 2

int bcmFlash_getIdent(int part, int *start, int *end, const char *key, char *line, int len)
{
    int ret = 0; // total size of all entries
    int boot = getBootPartition();
    char name[MAX_MTD_NAME_SIZE];
    char *buf;
    FILE *fp;
    int point, read;
    int size; // size of entry
    int blksize;

    UPDATE_FLASH_INFO();
    if ( (gFlashInfo == FLASH_INFO_FLAG_NAND) ||
	     ( gFlashInfo == FLASH_INFO_FLAG_NOR) )
    {
        int mtd_fd;
        mtd_info_t *mtd;

        //FIXME should spinor flash return 0?
        if( gFlashInfo == FLASH_INFO_FLAG_NOR)
            mtd = get_mtd_device_handle(SPI_NOR_BOOTFS_MTD_NAME, &mtd_fd, 0);
        else	
            mtd = get_mtd_device_handle("image", &mtd_fd, 0);
        if (!mtd)
        {
            fprintf(stderr, "ERROR!!! Could not get image device handle\n");
            put_mtd_device(mtd, mtd_fd, -1);
            return(-1);
        }

        blksize = mtd->erasesize;
        put_mtd_device(mtd, mtd_fd, -1);

        if (!blksize)
        {
            fprintf(stderr, "ERROR!!! Could not find block size\n");
            return(-1);
        }
    }
    else if (gFlashInfo == FLASH_INFO_FLAG_EMMC)
    {
        blksize = 0x20000; // match NAND minimum block size
    }
    else
        return(-1);

    if (gFlashInfo == FLASH_INFO_FLAG_NAND)
    {
        if( isLegacyFlashLayout() ) 
        {
            if ( (part == boot) || (part == 3) )
                ret = get_mtd_device_name("image", name);
            else
                ret = get_mtd_device_name("image_update", name);

            if (ret < 0)
                return(-1);
        }
        else
        {
            if ( (part == 1) || ((part == 3) && (boot == 1)) || ((part == 4) && (boot == 2)) )
                ret = get_mtd_device_name("bootfs1", name);
            else
                ret = get_mtd_device_name("bootfs2", name);

            if (ret < 0)
                return(-1);
        }
    }
    else if (gFlashInfo == FLASH_INFO_FLAG_EMMC)
    {
        if ( (part == 1) || ((part == 3) && (boot == 1)) || ((part == 4) && (boot == 2)) )
            strcpy(name, "/dev/bootfs1");
        else
            strcpy(name, "/dev/bootfs2");
    }
    else if (gFlashInfo == FLASH_INFO_FLAG_NOR)
    {
        //FIXME...NOR flash support single image
        if( (part == 1) || (part == 3))
            ret = get_mtd_device_name(SPI_NOR_BOOTFS_MTD_NAME, name);
        else
            return(-1);
        if (ret < 0)
            return(-1);
    }
    else
        return(-1);

    fp = fopen(name,"rb");
    if (fp == NULL)
    {
       fprintf(stderr, "ERROR!!! Could not open device %s\n", name);
       return (-1);
    }

    if ( (buf = malloc(blksize)) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not allocate memory for block buffer\n");
        fclose(fp);
        return(-1);
    }

    if (key)
        snprintf(name, MAX_MTD_NAME_SIZE, "$%s", key);
    else
        strcpy(name, "$");

    if (*start >= 0)
        fseek(fp, blksize * *start, SEEK_SET);
    else
        *start = 0;

    ret = 0;
    while ( ((*end < 0) || (*start <= *end)) && ((read = fread(buf, 1, blksize, fp)) != 0) )
    {
        for(point = 0; (point < (read - (strlen(name) + TRAILER))); point++)
        { // search block for version string
            if (!memcmp(buf + point, name, strlen(name)))
            {
                size = strlen(name);

                // check for valid tag
                while ( ((point + size) < (read - TRAILER))  &&
			( ( (*(buf + point + size) >= 'A') && (*(buf + point + size) <= 'Z') ) ||
                          ( (*(buf + point + size) >= 'a') && (*(buf + point + size) <= 'z') ) ) )
		{
                    size++;
		}

                if ( ( point+size < (read - TRAILER) ) && (*(buf + point + size) != ':') )
                    continue;

                while ( ((point + size) < (read - TRAILER)) && 
			(*(buf + point + size) >= ' ') && (*(buf + point + size) <= '~') )
                 { // continue search for valid entry
                    if (!memcmp(buf + point + size, " $", TRAILER))
                    { // found terminator and thus entry
                        size += TRAILER;

                        if (size <= len)
                        {
                            //memcpy(&line[ret], buf + point, size);
                            snprintf(&line[ret], len, "%.*s", size, buf + point);

                            len -= size;
                        }

                        ret += size;

                        if (key)
                            goto DONE;

                        point += size;
                        break;
                    }

                    size++;
                }
            }
        }

        (*start)++;
    }
DONE:
    fclose(fp);
    free(buf);

    return( ret );
}


/***********************************************************************
 * Function Name: getBootPartition
 * Description  : Returns the booted partition.
 * Returns      : boot partition or -1 for failure
 ***********************************************************************/
#include <libfdt.h>
#include "util.c"

int getBootPartition( void )
{
    int ret = -1;

    char *blob;
    int len;
    int *value = 0;
    int offset;

    blob = utilfdt_read("/sys/firmware/fdt", NULL);
    if (blob)
    {
        if ( (offset = fdt_path_offset(blob, "/chosen")) >= 0 )
        {
            if ( (value = (int *)fdt_getprop(blob, offset, "active_image", &len)) )
            {
                if ( (ret = fdt32_to_cpu(*(fdt32_t *)value)) && (ret > 0) && (ret < 3) )
                    return(ret);
            }
        }
    }

    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            ret = nandGetBootPartition();
        break;

        case FLASH_INFO_FLAG_EMMC:
            ret = emmcGetBootPartition();
        break;

        case FLASH_INFO_FLAG_NOR:
            //FIXME spi nor flash only support single image.
            ret = 1;
        break;

        default:
        break;
    }
    return ret;
}


/***********************************************************************
 * Function Name: getUpgradePartition
 * Description  : Returns the upgrade  partition.
 * Returns      : upgrade partition
 ***********************************************************************/
int getUpgradePartition( void )
{
    int part = getBootPartition();
    UPDATE_FLASH_INFO();
    if( (gFlashInfo != FLASH_INFO_FLAG_NOR) && (part != -1) )
        part = ((part==1)?2:1);

    return part;
}

static unsigned int pkgtb_getCrc32(const unsigned char *pdata, unsigned int size, unsigned int crc_initial)
{
    unsigned int crc_final = genUtl_getCrc32(pdata, size, crc_initial ^ 0xffffffff) ^ 0xffffffff;
    return crc_final;
}

static int validate_metadata(MDATA *mdatap, int * committed, int *valid, int *seq)
{
    int valid_img_idx[2] = {0};
    int committed_idx = 0;
    int seq_img_idx[2] = {-1};
    uint32_t crc;
    int ret = 0;
    int i;
    char * commitp = mdatap->mdata_obj.data;
    char * validp =  commitp + strlen(commitp) + 1;
    char * seqp =    validp  + strlen(validp)  + 1;
    crc = pkgtb_getCrc32((unsigned char*)mdatap->mdata_obj.data, (mdatap->size - 4) & 0xffff, 0);

    if( crc ==  mdatap->mdata_obj.crc ) 
    {
        sscanf(commitp,"COMMITTED=%d", &committed_idx);
        sscanf(validp,"VALID=%d,%d", &valid_img_idx[0], &valid_img_idx[1]);
        //FIXME: Magic numbers
        for( i=0; i<2; i++ )
        {
            if( valid_img_idx[i] )
                valid[valid_img_idx[i]-1] = valid_img_idx[i];
        }
        *committed = committed_idx;

        // preset the sequence numbers incase the field doesn't exist
        if ((*committed == 1) && valid[0])
        { 
            seq[0] = 1;
            if (valid[1])
                seq[1] = 0;
        }
        else if ((*committed == 2) && valid[1])
        {
            seq[1] = 1;
            if (valid[0])
                seq[0] = 0;
        }

        sscanf(seqp,"SEQ=%d,%d", &seq_img_idx[0], &seq_img_idx[1]);
        for( i=0; i<2; i++ )
        {
            if( seq_img_idx[i] != -1)
                seq[i] = seq_img_idx[i];
        }

        printf("rd_metadata: committed %d valid %d,%d seq %d,%d\n", *committed, valid[0], valid[1], seq[0], seq[1]);
    }
    else
    {
        printf("ERROR: metadata crc failed! exp: 0x%08x  calc: 0x%08x\n", mdatap->mdata_obj.crc, crc);
        *committed = 0;
        valid[0] = 0;
        valid[1] = 0;
        ret = -1;
    }
    return ret;
}

static int set_metadata_val( int * committed, int * valid, int * seq )
{
    MDATA * mdatap;
    uint32_t crc;
    int i, mdata_idx;
    int ret = -1;
    UPDATE_FLASH_INFO();

    mdatap = (MDATA *)malloc(PKGTB_METADATA_RDWR_SIZE);
    mdatap->word0 = PKGTB_METADATA_SIZE;
    mdatap->size = PKGTB_METADATA_SIZE;
    i = sprintf((char*)mdatap->mdata_obj.data, "COMMITTED=%d",*committed) + 1;
    i = i + sprintf((char*)&mdatap->mdata_obj.data[i], "VALID=%d,%d",valid[0],valid[1]) + 1;
    i = i + sprintf((char*)&mdatap->mdata_obj.data[i], "SEQ=%d,%d",seq[0],seq[1]) + 1;
    mdatap->mdata_obj.data[i] = '\0';
    crc = pkgtb_getCrc32((unsigned char*)mdatap->mdata_obj.data, (mdatap->size - 4) & 0xffff, 0);
    memcpy(&mdatap->mdata_obj.crc, &crc, sizeof(crc));

    printf("wr_metadata: committed %d valid %d,%d seq %d,%d\n", *committed, valid[0], valid[1], seq[0], seq[1]);

    //FIXME: Magic numbers
    for( mdata_idx=1; mdata_idx<=2; mdata_idx++ )
    {
        switch( gFlashInfo )
        {
            case FLASH_INFO_FLAG_NAND:
                ret = setNandMetadata((char*)mdatap, PKGTB_METADATA_RDWR_SIZE, mdata_idx);
            break;

            case FLASH_INFO_FLAG_EMMC:
                ret = setEmmcMetadata((char*)mdatap, PKGTB_METADATA_RDWR_SIZE, mdata_idx);
            break;

            case FLASH_INFO_FLAG_NOR:
                //FIXME nor  not support metadata
                ret = 0;
            break;
            default:
            break;
        }
    }

    free(mdatap);
    return ret;
}

static int get_metadata_val( int * committed, int * valid, int * seq)
{
    MDATA * mdatap = (MDATA *)malloc(PKGTB_METADATA_MAX_SIZE);
    int ret = -1;
    int num_bytes_read = 0;
    int mdata_idx;
    UPDATE_FLASH_INFO();

    //FIXME: Magic numbers
    for( mdata_idx=1; (mdata_idx<=2) && (ret < 0); mdata_idx++ )
    {
        switch( gFlashInfo )
        {
            case FLASH_INFO_FLAG_NAND:
                num_bytes_read = getNandMetadata( (char*)mdatap, PKGTB_METADATA_RDWR_SIZE, mdata_idx);
            break;

            case FLASH_INFO_FLAG_EMMC:
                num_bytes_read = getEmmcMetadata( (char*)mdatap, PKGTB_METADATA_RDWR_SIZE, mdata_idx);
            break;

            case FLASH_INFO_FLAG_NOR:
                //FIXME SPI NOR not support
                free(mdatap);
                return ret;
            break;

            default:
            break;
        }

        if( num_bytes_read >=  PKGTB_METADATA_SIZE )
            ret = validate_metadata(mdatap, committed, valid, seq);
        else
            fprintf(stderr, "ERROR!!! unable to retrieve metadata%d!", mdata_idx);
    }
    free(mdatap);

    if( ret )
        fprintf(stderr, "ERROR!!! unable to retrieve/parse metadata!");

    return ret;
}

static int getImgCommitStatus( int img_idx, char * commit_flag  )
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    if( committed_img == img_idx )
        *commit_flag = '1';
    else
        *commit_flag = '0';
    return 0;
}

static int setImgCommitStatus( int img_idx , char * commit_flag)
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    if( *commit_flag == '1' )
    {
        valid_imgs[img_idx-1] = img_idx;
        committed_img = img_idx;
    }
    else
    {
        if( committed_img == img_idx )
            committed_img = 0;
    }

    set_metadata_val(&committed_img, valid_imgs, seq_imgs);
    return 0;
}

int setImgValidStatus( int img_idx, int * valid)
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    if( *valid )
        valid_imgs[img_idx-1] = img_idx;
    else        
        valid_imgs[img_idx-1] = 0;

    set_metadata_val(&committed_img, valid_imgs, seq_imgs);
    return 0;
}

int getImgValidStatus( int img_idx, int * valid)
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    if( valid_imgs[img_idx-1] == img_idx )
        *valid = 1;
    else        
        *valid = 0;
    return 0;
}

int setImgSeqNum( int img_idx, int seq)
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    if( valid_imgs[img_idx-1] )
        seq_imgs[img_idx-1] = seq;

    set_metadata_val(&committed_img, valid_imgs, seq_imgs);
    return 0;
}

int getImgSeqNum( int img_idx, int * seq)
{
    int committed_img = 0;
    //FIXME: Magic numbers
    int valid_imgs[2] = {0};
    int seq_imgs[2] = {-1};
    int ret = get_metadata_val(&committed_img, valid_imgs, seq_imgs);

    if (ret)
        return(ret);

    *seq = seq_imgs[img_idx-1];

    return 0;
}


/***********************************************************************
 * Function Name: commit
 * Description  : Gets/sets the commit flag for an image.
 * Returns      : 0 for success or -1 for failure
 ***********************************************************************/
int commit( int partition, char *string )
{
    int ret = -1;
    UPDATE_FLASH_INFO();

    if( isLegacyFlashLayout())
    {
        switch( gFlashInfo )
        {
            case FLASH_INFO_FLAG_NAND:
                ret = nandCommit(partition, string);
            break;

            case FLASH_INFO_FLAG_EMMC:
                ret = emmcCommit(partition, string);
            break;

            default:
            break;
        }
    }
    else
    {
        int write = (*string != 0);
        /* New flash layout */
        flashutil_debug("%s: Write:%d, part:%d\n", __FUNCTION__, write, partition);
        if( write )
            ret = setImgCommitStatus(partition, string);
        else
            ret = getImgCommitStatus(partition, string);
    }
    return ret;
}


/***********************************************************************
 * devCtl_setImageState - for all flash types (safer to call this)
 * setBootImageState - for NAND or EMMC flash only
 * Description  : Persistently sets the state of an image update.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int devCtl_setImageState(int state)
{
   unsigned int flag=0;

   getFlashInfo(&flag);
   if (flag & (FLASH_INFO_FLAG_NAND | FLASH_INFO_FLAG_EMMC)) // NAND flash, no need to dig into kernel space
      return(setBootImageState(state));
   else
   {
      if( norIsNewFlashLayout())
	  	return 0;
      return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                                      0, NULL, 0, state, NULL) );
   	}
}

int getBootReason(void)
{
    char buffer[64];
    int rc;

    FILE *fp;
    
    if (!(fp = fopen("/proc/bootstate/reset_reason", "r")))
    {
        printf("%s: Error, cannot open %s\n", __FUNCTION__, "/proc/bootstate/reset_reason");
        return(-1);
    }

    if (!fgets((char*)buffer, 64, fp))
    {
        fclose(fp);
        printf("%s: Error, Read failed from %s\n", __FUNCTION__, "/proc/bootstate/reset_reason");
        return(-1);
    }

    fclose(fp);

    rc = strtoul(buffer, NULL, 16);

    return(rc);
}

int setBootReason( int reason )
{
    char command[64];
    int rc;

    snprintf(command, sizeof command, "echo 0x%x > /proc/bootstate/reset_reason", reason);
    rc = system(command);

    return(rc);
}

int setBootImageState( int newState )
{
    int ret = -1;
    int currState;
    int seq1 = -1; 
    int seq2 = -1;
    char commitflag1 = 0;
    char commitflag2 = 0;
    int pureubi1;
    int pureubi2;

    /* Get current boot state variables */
    currState = getBootImageState();
    seq1 = getSequenceNumber(1);
    seq2 = getSequenceNumber(2);
    pureubi1 = commit(1, &commitflag1);
    pureubi2 = commit(2, &commitflag2);

#ifdef NO_AUTOCOMMIT_IMAGE
    if (newState == BOOT_SET_NEW_IMAGE)
    {
        FILE *fp;

        fp = fopen("/data/commit_image_after_reboot","w");

        if (!IS_NULL(fp))
        {
            fclose(fp);
            printf("Not committing image now but will do so after successful reboot to Linux\n");
            newState = BOOT_SET_NEW_IMAGE_ONCE;
        }
        else
            printf("ERROR!!! Could not create data file, committing image now\n");
    }
#endif

    if ((seq1 == 0) && (seq2 == 999))
        seq1 = 1000;
    if ((seq2 == 0) && (seq1 == 999))
        seq2 = 1000;

    switch(newState)
    { // convert state to OMCI states
        case BOOT_SET_NEW_IMAGE:
            if( seq1 > seq2 )
                newState = BOOT_SET_PART1_IMAGE;
            else
                newState = BOOT_SET_PART2_IMAGE;
            break;

        case BOOT_SET_OLD_IMAGE:
            if( seq2 > seq1 )
                newState = BOOT_SET_PART1_IMAGE;
            else
                newState = BOOT_SET_PART2_IMAGE;
            break;

        case BOOT_SET_NEW_IMAGE_ONCE:
            if( seq1 > seq2 )
                newState = BOOT_SET_PART1_IMAGE_ONCE;
            else
                newState = BOOT_SET_PART2_IMAGE_ONCE;
            break;

        case BOOT_SET_OLD_IMAGE_ONCE:
            if( seq2 > seq1 )
                newState = BOOT_SET_PART1_IMAGE_ONCE;
            else
                newState = BOOT_SET_PART2_IMAGE_ONCE;
            break;

        default:
            break;
    }

    if( currState == newState )
        return(0);

    if (isLegacyFlashLayout() && !pureubi1 && !pureubi2)
    { // two pureUBI images
        switch(newState)
        {
            case BOOT_SET_PART1_IMAGE:
            case BOOT_SET_PART2_IMAGE_ONCE:
                commit(1, "1");
                if (seq2 > seq1)
                    commit(2, "0");
                break;

            case BOOT_SET_PART2_IMAGE:
            case BOOT_SET_PART1_IMAGE_ONCE:
                commit(2, "1");
                if (seq1 > seq2)
                    commit(1, "0");
                break;

            default:
                break;
        }

        if ( (newState == BOOT_SET_PART1_IMAGE_ONCE) || (newState == BOOT_SET_PART2_IMAGE_ONCE) )
        { // set bit to boot the inactive image
            getset_boot_inactive_image(1);
        }
    }

    if (!isLegacyFlashLayout())
    {
        int valid1;
        int valid2;

        getImgValidStatus(1, &valid1);
        getImgValidStatus(2, &valid2);

        switch(newState)
        { // new layout works differently, there's only a single commit value so committing one image automatically uncommits the other
            case BOOT_SET_PART1_IMAGE:
            case BOOT_SET_PART2_IMAGE_ONCE:
                if ((commitflag1 != '1') && valid1)
                    commit(1, "1");
                break;

            case BOOT_SET_PART2_IMAGE:
            case BOOT_SET_PART1_IMAGE_ONCE:
                if ((commitflag2 != '1') && valid2)
                    commit(2, "1");
                break;

            default:
                break;
        }

        if (valid1 && valid2 && ((newState == BOOT_SET_PART1_IMAGE_ONCE) || (newState == BOOT_SET_PART2_IMAGE_ONCE)) )
        { // set bit to boot the inactive image
            setBootReason(BCM_BOOT_REASON_ACTIVATE);
        }
	else
            setBootReason(0);

    }

    { /* NAND flash, old method of boot_state */
        FILE *fp;
        char state_name[] = "/data/" NAND_BOOT_STATE_FILE_NAME;

        /* Update the image state persistently using "new image" and "old image"
         * states.  Convert "partition" states to "new image" state for
         * compatibility with the non-OMCI image update.
         */
        switch(newState)
        { // convert OMCI state to internal state
            case BOOT_SET_PART1_IMAGE:
                if( seq1 > seq2 )
                    newState = BOOT_SET_NEW_IMAGE;
                else
                    newState = BOOT_SET_OLD_IMAGE;
                break;

            case BOOT_SET_PART2_IMAGE:
                if( seq2 > seq1 )
                    newState = BOOT_SET_NEW_IMAGE;
                else
                    newState = BOOT_SET_OLD_IMAGE;
                break;

            case BOOT_SET_PART1_IMAGE_ONCE:
                if( seq1 > seq2 )
                    newState = BOOT_SET_NEW_IMAGE_ONCE;
                else
                    newState = BOOT_SET_OLD_IMAGE_ONCE;
                break;

            case BOOT_SET_PART2_IMAGE_ONCE:
                if( seq2 > seq1 )
                    newState = BOOT_SET_NEW_IMAGE_ONCE;
                else
                    newState = BOOT_SET_OLD_IMAGE_ONCE;
                break;

            default:
                break;
        }

        /* Remove old file:
         * This must happen before a new file is created so that the new file-
         * name will have a higher version in the FS, this is also the reason
         * why renaming might not work well (higher version might exist as
         * deleted in FS) However this discrimination should be fixed now in CFE */
        {
            int rc;
            char command[64];

            state_name[strlen(state_name) - 1] = '*';
            snprintf(command, sizeof command, "rm %s >/dev/null 2>&1", state_name); // suppress command output in case no files were found to delete
            rc = system(command);
            if (rc < 0)
            {
               fprintf(stderr, "rm command failed.\n");
            }
        }

        /* Create new state file name. */
        state_name[strlen(state_name) - 1] = newState;

        fp = fopen(state_name,"w");

        if (!IS_NULL(fp))
        {
            fwrite(state_name, strlen(state_name), 1, fp);

            fclose(fp);
        }
        else
            printf("Unable to open '%s'.\n", state_name);

        ret = 0;
    }

    return( ret );
}


/***********************************************************************
* devCtl_getImageState - for all flash types (safer to call this)
* getBootImageState - for NAND or EMMC flash only
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int devCtl_getImageState(void)
{
   unsigned int flag=0;

   getFlashInfo(&flag);
   if (flag & (FLASH_INFO_FLAG_NAND | FLASH_INFO_FLAG_EMMC))
      return(getBootImageState());
   else
      return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                             0, NULL, 0, BOOT_GET_BOOT_IMAGE_STATE, NULL) );
}

int getBootImageState(void)
{
    int seq1 = -1;
    int seq2 = -1;
    char commitflag1 = 0;
    char commitflag2 = 0;
    int ret = (getBootPartition() == 1) ? BOOT_SET_PART1_IMAGE : BOOT_SET_PART2_IMAGE;
    int pureubi1 = commit(1, &commitflag1);
    int pureubi2 = commit(2, &commitflag2);

    seq1 = getSequenceNumber(1);
    seq2 = getSequenceNumber(2);

    if ((seq1 == 0) && (seq2 == 999))
        seq1 = 1000;
    if ((seq2 == 0) && (seq1 == 999))
        seq2 = 1000;

    /* Handle new flash layout */
    if( !isLegacyFlashLayout() ) 
    {
        if (commitflag1 == '1')
        {
            if (getBootReason() == BCM_BOOT_REASON_ACTIVATE)
                ret = BOOT_SET_PART2_IMAGE_ONCE;
            else
                ret = BOOT_SET_PART1_IMAGE;
        }
        else if (commitflag2 == '1')
        {
            if (getBootReason() == BCM_BOOT_REASON_ACTIVATE)
                ret = BOOT_SET_PART1_IMAGE_ONCE;
            else
                ret = BOOT_SET_PART2_IMAGE;
        }

        return ret;
    }

    if (!pureubi1 && !pureubi2)
    { // two pureUBI images, boot state undefined if neither flag is 1
        if (seq1 > seq2)
        {
            if (commitflag1 == '1')
                ret = BOOT_SET_PART1_IMAGE;
            else if (commitflag2 == '1')
                ret = BOOT_SET_PART2_IMAGE;
        }
        else
        {
            if (commitflag2 == '1')
                ret = BOOT_SET_PART2_IMAGE;
            else if (commitflag1 == '1')
                ret = BOOT_SET_PART1_IMAGE;
        }

        if (getset_boot_inactive_image(-1) == 1)
        {
            if (ret == BOOT_SET_PART1_IMAGE)
                ret = BOOT_SET_PART2_IMAGE_ONCE;
            if (ret == BOOT_SET_PART2_IMAGE)
                ret = BOOT_SET_PART1_IMAGE_ONCE;
        }
    }
    else if ((seq1 != -1) && (seq2 != -1))
    { // two images and at least one JFFS2 image, boot state is in data partition
        /* NAND flash */
        char states[] = {BOOT_SET_NEW_IMAGE, BOOT_SET_OLD_IMAGE,
            BOOT_SET_NEW_IMAGE_ONCE, BOOT_SET_OLD_IMAGE_ONCE};
        char boot_state_name[] = "/data/" NAND_BOOT_STATE_FILE_NAME;
        int i;

        /* The boot state is stored as the last character of a file name on
         * the data partition, /data/boot_state_X, where X is
         * BOOT_SET_NEW_IMAGE, BOOT_SET_OLD_IMAGE, BOOT_SET_NEW_IMAGE_ONCE.
         */
        for( i = 0; i < (int)sizeof(states); i++ )
        {
            FILE *fp;

            boot_state_name[strlen(boot_state_name) - 1] = states[i];

            fp = fopen(boot_state_name,"r");

            if (fp != NULL)
            {
                fclose(fp);

                ret = (int) states[i];
                break;
            }
        }

        switch(ret)
        { // convert state to OMCI states
            case BOOT_SET_NEW_IMAGE:
                if( seq1 > seq2 )
                    ret = BOOT_SET_PART1_IMAGE;
                else
                    ret = BOOT_SET_PART2_IMAGE;
                break;

            case BOOT_SET_OLD_IMAGE:
                if( seq2 > seq1 )
                    ret = BOOT_SET_PART1_IMAGE;
                else
                    ret = BOOT_SET_PART2_IMAGE;
                break;

            case BOOT_SET_NEW_IMAGE_ONCE:
                if( seq1 > seq2 )
                    ret = BOOT_SET_PART1_IMAGE_ONCE;
                else
                    ret = BOOT_SET_PART2_IMAGE_ONCE;
                break;

            case BOOT_SET_OLD_IMAGE_ONCE:
                if( seq2 > seq1 )
                    ret = BOOT_SET_PART1_IMAGE_ONCE;
                else
                    ret = BOOT_SET_PART2_IMAGE_ONCE;
                break;

            default:
                break;
        }
    }

    return( ret );
}


int devCtl_getBootedImageId(void)
{ // this call is ok as the "0" value does not call getSequenceNumber which won't work with pureUBI images
    return( (int) devCtl_boardIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                               0, NULL, 0, BOOT_GET_BOOTED_IMAGE_ID, NULL) );
}

/***********************************************************************
 * devCtl_getBootedImagePartition -- for all flash types (safer to call this)
 * getBootedValue for NAND and EMMC types only.
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int devCtl_getBootedImagePartition(void)
{
     unsigned int flag=0;

     getFlashInfo(&flag);
     if (flag & (FLASH_INFO_FLAG_NAND | FLASH_INFO_FLAG_EMMC))
         return(getBootedValue());
     else
         return( (int) devCtl_boardDriverIoctl(BOARD_IOCTL_BOOT_IMAGE_OPERATION,
                                0, NULL, 1, BOOT_GET_BOOTED_IMAGE_ID, NULL) );
}

int isLegacyFlashLayout(void)
{
    int ret = -1;
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            ret = nandIsLegacyFlashLayout();
        break;

        case FLASH_INFO_FLAG_EMMC:
            ret = emmcIsLegacyFlashLayout();
        break;

        case FLASH_INFO_FLAG_NOR:
            ret = !norIsNewFlashLayout();
        break;

        default:
        break;
    }
    return ret;
}

int getBootedValue(void)
{
    int ret = -1;
    UPDATE_FLASH_INFO();
    switch( gFlashInfo )
    {
        case FLASH_INFO_FLAG_NAND:
            ret = nandGetBootedValue();
        break;

        case FLASH_INFO_FLAG_EMMC:
            ret = emmcGetBootedValue();
        break;

        default:
        break;
    }
    return ret;
}
#endif /* DESKTOP_LINUX */
