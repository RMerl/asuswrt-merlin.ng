/*****************************************************************************
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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
 ****************************************************************************/

/*****************************************************************************
*    Description:
*
*      The BCM CPE software Image Incremental Flashing (imageIf) library
*      implementation. This library runs on top of bcm_flashutil. It uses
*      the incremental flashing mechanism to avoid holding the entire image
*      in RAM, thus reduces RAM usage.
*      This library is expected to be platform-independent. The underlying
*      bcm_flashutil library handles the platform-specific operations.
*      Notes about the library:
*
*****************************************************************************/

/* ---- Include Files ----------------------------------------------------- */
#include "bcm_flashutil.h"
#include "bcm_flashutil_nand.h"
#include "bcm_flashutil_emmc.h"
#include "bcm_flashutil_nor.h"

#include <rtconfig.h>
#include "bcm_imgif.h"

#include "bcm_imgif_pkgtb.h"
#include "bcmTag.h"
#include "bcm_hwdefs.h"
#include "bcm_boarddriverctl.h"
#include "genutil_crc.h"
#include "bcm_imgutil.h"

#include <libfdt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ---- Constants and Types ----------------------------------------------- */
#define IMG_WRITE_FS    1  /* Keep enabled:  Enable filesystem writing */
#define CC_IMGIF_DEBUG  0

/* Compile flags */
#define PKGTB_FORKED_WRITES     1   // Fork child processes for writing bootfs/rootfs segments to flash
#define PKGTB_USE_TMP_FILES_ALL 1   // Download entire segments to temp files in /var
#if PKGTB_USE_TMP_FILES_ALL
#define PKGTB_USE_SHELL_SHA256  1   // Use sha256 from linux shell
#else
#define PKGTB_USE_SHELL_SHA256  0   // Do NOT manually change this define! 
#endif
#define PKGTB_FLASH_LOADER      1   // Flash the loader binary

#define PKGTB_UBI_VOL_OVHEAD    (1024*1024)

/* MTD device names */
#define PKGTB_MTD_IMG_DEV_NAME      "image"
#define PKGTB_MTD_LOADER_DEV_NAME   "loader"

/* hash/signature node */
#define PKGTB_HASH_NODENAME         "hash"
#define PKGTB_ALGO_PROP             "algo"
#define PKGTB_VALUE_PROP            "value"
#define PKGTB_IGNORE_PROP           "uboot-ignore"
#define PKGTB_SIG_NODENAME          "signature"

/* image node */
#define PKGTB_IMAGES_PATH           "/images"
#define PKGTB_DATA_PROP             "data"
#define PKGTB_DATA_POSITION_PROP    "data-position"
#define PKGTB_DATA_OFFSET_PROP      "data-offset"
#define PKGTB_DATA_SIZE_PROP        "data-size"
#define PKGTB_TIMESTAMP_PROP        "timestamp"
#define PKGTB_DESC_PROP             "description"
#define PKGTB_ARCH_PROP             "arch"
#define PKGTB_TYPE_PROP             "type"
#define PKGTB_OS_PROP               "os"
#define PKGTB_COMP_PROP             "compression"

/* configuration node */
#define PKGTB_CONFS_PATH            "/configurations"
#define PKGTB_KERNEL_PROP           "kernel"
#define PKGTB_RAMDISK_PROP          "ramdisk"
#define PKGTB_FDT_PROP              "fdt"
#define PKGTB_DEFAULT_PROP          "default"

/* Compat string */
#define PKGTB_COMPAT_PARAM_DELIM    ";"
#define PKGTB_COMPAT_VALUE_DELIM    ","
#define PKGTB_COMPAT_VALUE_REF      "="

/* PKGTB Segment defines */
#define PKGTB_SEG_IDX_MAX 10
#define PKGTB_SEG_IDX_INVALID -1

/* Partition numbers/volids */
#define PKGTB_NAND_BOOTFS1_VOLID    3
#define PKGTB_NAND_ROOTFS1_VOLID    4
#define PKGTB_NAND_BOOTFS2_VOLID    5
#define PKGTB_NAND_ROOTFS2_VOLID    6
#define PKGTB_EMMC_BOOTFS1_VOLID    PKGTB_NAND_BOOTFS1_VOLID    
#define PKGTB_EMMC_ROOTFS1_VOLID    PKGTB_NAND_ROOTFS1_VOLID    
#define PKGTB_EMMC_BOOTFS2_VOLID    PKGTB_NAND_BOOTFS2_VOLID    
#define PKGTB_EMMC_ROOTFS2_VOLID    PKGTB_NAND_ROOTFS2_VOLID    
#define PKGTB_IMG1_PART_NUM         1
#define PKGTB_IMG2_PART_NUM         2

typedef enum
{
    IMGIF_PKGTB_IMG_SEGMENT_PKGT_HDR = 0,   //FDT header
    IMGIF_PKGTB_IMG_SEGMENT_PKGT,           //Actual full FDT
    IMGIF_PKGTB_IMG_SEGMENT_LOADER,    
    IMGIF_PKGTB_IMG_SEGMENT_ROOTFS,
    IMGIF_PKGTB_IMG_SEGMENT_BOOTFS,
    IMGIF_PKGTB_IMG_SEGMENT_UNKNOWN,
} IMGIF_PKGTB_IMG_SEGMENTS;

typedef struct
{
    char * name;
    uint64_t data_offset;
    uint64_t size;
    uint64_t bytesdone;
    IMGIF_PKGTB_IMG_SEGMENTS type;
    unsigned int valid;
    UINT8 * sha256_ptr;
} IMGIF_PKGTB_IMG_SEGMENT_INFO;

/* IMGIF pkgtb context. */
typedef struct
{
    FILE * loader_fh;
    FILE * rootfs_fh;
    FILE * bootfs_fh;
    IMGIF_PKGTB_IMG_SEGMENT_INFO  pkgtb_img_segments[PKGTB_SEG_IDX_MAX];
    int  current_img_segment_idx;
    int  num_img_segments;
    int  num_img_segments_complete;
    unsigned int crc_image_calc;
    uint64_t total_img_bytesdone;
    char * pkgt_ptr;
    char * loader_ptr;
    int loader_seg_index;
    int booted_img_idx;
    int update_img_idx;
    struct fdt_header pkgt_hdr;
    imgif_img_info_t imgInfoExt;
    IMG_FORMAT_PARSER_CB fmtParserCb;
    CAL_CRC32_CB calCrc32Cb;
#if defined(CUSTOM_NAND_SINGLE_IMAGE) || defined(RTCONFIG_SINGLEIMG_B)
    unsigned force_upd_img_idx;
#endif
} imgif_ctx_t;

#define getCtx() (&imgif_ctx)
#define getCtxByHandle(h) ((imgif_ctx_t*)h)

#if CC_IMGIF_DEBUG
#define imgif_debug(fmt, arg...) \
  printf("%s.%u: " fmt , __FUNCTION__, __LINE__, ##arg)
#define DUMPIMGIFCTX(cxtP) dump_ctx(ctxP)
#else
#define imgif_debug(fmt, arg...)
#define DUMPIMGIFCTX(cxtP)
#endif

#define imgif_error(fmt, arg...) \
  printf("ERROR[%s.%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

/* ---- Private Function Prototypes --------------------------------------- */
/* ---- Public Variables -------------------------------------------------- */
/* ---- Private Variables ------------------------------------------------- */
static imgif_ctx_t imgif_ctx;
#if PKGTB_FORKED_WRITES
static int forked_flash_writes = 0;
#endif
/* ---- Functions --------------------------------------------------------- */

/***********************************************************************************
 *                      PKTB SEGMENT PROCESSING FUNCTIONS                          *
 ***********************************************************************************/
#if PKGTB_USE_SHELL_SHA256
static int compare_imgseg_sha256(const char *imgseg_path,
                                 const UINT8 *imgseg_sha)
{
    FILE *fp;
    char cmdBuf[512]={0};
    char output[64+2+sizeof(cmdBuf)+1]={0}; // see comment below about length

    /* Open the command for reading. */
    snprintf(cmdBuf, sizeof(cmdBuf), "sha256sum %s", imgseg_path);
    fp = popen(cmdBuf, "r");
    if (fp == NULL) 
    {
        printf("Failed to run command %s\n", cmdBuf);
        return -1;
    }

    // Example output of sha256sum:
    // f15650dac207ad8ef72dff5eefddd910c7cef2b6d33a15f2b2343029cfa45704  <path>
    // Sha256 is 32 bytes, which takes 64 bytes to represent in hex.
    // So length of output buffer must be 64 + 2 spaces + strlen(path) + null
    if (fgets(output, sizeof(output)-1, fp) == NULL)
    {
        printf("Could not read results of cmd %s\n", cmdBuf);
        pclose(fp);
        return -1;
    }

    // We got the output we need, close fp now.
    pclose(fp);

    // Compare output sha256 with expected value in imgseg_sha
    {
        int rc, i;
        UINT8 byte;

        for (i=0; i < 32; i++)
        {
            rc = sscanf(&(output[i*2]), "%2hhx", &byte);
            if (rc != 1)
            {
                printf("%s: parse failed at [%d] (output=%s)\n",
                       __FUNCTION__, i*2, output);
                return -1;
            }
            else if (byte != imgseg_sha[i])
            {
                printf("%s: compare failed at [%d]: got %2hhx expected %2hhx\n",
                       __FUNCTION__, i, byte, imgseg_sha[i]);
                return -1;
            }
        }
    }

    // if we get here, then sha256sum matched.
    return 0;
}
#endif /* PKGTB_USE_SHELL_SHA256 */


static int run_shell_command(const char* cmd)
{
    int status;  
    int ret = -1;

    status = system(cmd);  

    if (-1 == status)  
    {  
        imgif_debug("%s: Error: run shell command: %s fail. System error!\n", __FUNCTION__, cmd);
    }  
    else  
    {
        if (WIFEXITED(status))	
        {  
            if (0 == WEXITSTATUS(status))  
            {  
                imgif_debug("%s:run shell command %s successfully.\n",__FUNCTION__,cmd);  
                ret = 0;
            }  
            else  
            {  
                imgif_debug("%s:run shell command %s fail, exit code: %d\n",__FUNCTION__,cmd, WEXITSTATUS(status));  
            }  
        }  
        else  
        {  
            imgif_debug("%s:run shell command %s fail, exit status = [%d]\n",__FUNCTION__, cmd, WEXITSTATUS(status));  
        }  
    }  
    return ret;
}
static int pivot_rootfs_to_ramfile(const char* ram_file_name)
{
    char cmd[256];

    //FIXME put commands in a shell script?
    /* only return -1 for several important cmd*/
    if( run_shell_command("mkdir /var/ramdisk") )
        return -1;
    sprintf(cmd, "mount -o loop %s /var/ramdisk", ram_file_name);
    if( run_shell_command(cmd))
        return -1;
    run_shell_command("mount --move /var /var/ramdisk/var");
    run_shell_command("mount --move /data /var/ramdisk/data");
    run_shell_command("mount --move /dev /var/ramdisk/dev");
    run_shell_command("mount --move /mnt /var/ramdisk/mnt");
    run_shell_command("mount --move /sys /var/ramdisk/sys");
    run_shell_command("mount --move /proc /var/ramdisk/proc");
    run_shell_command("mount --move /dev/pts /var/ramdisk/dev/pts");
    run_shell_command("mount --move  /sys/kernel/debug /var/ramdisk/sys/kernel/debug");
    if( run_shell_command("mkdir /var/ramdisk/var/oldroot"))
        return -1;
    if( run_shell_command("pivot_root /var/ramdisk/ /var/ramdisk/var/oldroot/"))
        return -1;
    run_shell_command("umount /var/oldroot/data/");
    run_shell_command("umount /var/oldroot/dev/pts/");
    run_shell_command("umount /var/oldroot/sys/kernel/debug/");
    run_shell_command("umount /var/oldroot/sys/");
    run_shell_command("umount /var/oldroot/proc");
    run_shell_command("umount /var/oldroot/mnt/");
    run_shell_command("rm /dev/root");
    //FIXME maybe need to check which loop device is used,in case it is not /dev/loop0
    run_shell_command("ln -s /dev/loop0 /dev/root");
    run_shell_command("umount /dev/root");
    run_shell_command("mount --move /var/oldroot/var /var");
 
    return 0;
}

static int finalize_tmpfile_write_spinor( int vol_id, char * vol_type,
    char* dev_name_prefix, int seg_index, imgif_ctx_t *ctxP )
{
    char cmd[128];
    char mtd_name[128];
    int ret= -1;
    printf("Flashing %s ....\n", dev_name_prefix);
    switch( ctxP->pkgtb_img_segments[seg_index].type )
    {
        case IMGIF_PKGTB_IMG_SEGMENT_LOADER:
            sprintf(cmd, "/var/%s.tmp",dev_name_prefix);
            sprintf(mtd_name,"/dev/%s",SPI_NOR_LOADER_MTD);
            ret = norWriteFileToMtdPar(cmd,mtd_name);
            break;
        case IMGIF_PKGTB_IMG_SEGMENT_BOOTFS:
            sprintf(cmd, "/var/%s.tmp",dev_name_prefix);
            sprintf(mtd_name,"/dev/%s",SPI_NOR_BOOTFS_MTD);
            ret = norWriteFileToMtdPar(cmd,mtd_name);
            break;
        case IMGIF_PKGTB_IMG_SEGMENT_ROOTFS:
            sprintf(cmd, "/var/%s.tmp",dev_name_prefix);
            if(pivot_rootfs_to_ramfile(cmd))
            {
                printf("pivot rootfs FAILED!\n");
            }
            else
            {
                sprintf(mtd_name,"/dev/%s",SPI_NOR_ROOTFS_MTD);
                ret = norWriteFileToMtdPar(cmd,mtd_name);
                break;
            }
            break;
        default:
            break;
    }
    return ret;
}

#define EXTRA_SPC_SUFFIX_STR "_extra_space"
static uint64_t nand_get_part_extra_bytes( char * part_prefix )
{
    char filename[128];
    char value[10];
    int i;
    unsigned long long extra_bytes = 0;
    unsigned long long size_mult = 1;
    FILE *fp = NULL;

    /* Get extra bytes specified in env */
    sprintf(filename, "/proc/environment/%s%s", part_prefix, 
            EXTRA_SPC_SUFFIX_STR);

    if( access( filename, F_OK ) != -1 ) 
    {
        if ((fp = fopen(filename, "r")) == NULL)
        {
              printf("could not open %s", filename);
              return 0;
        }
        
        if( NULL == fgets(value, 10, fp) )
        {
           printf("Could not read %s", filename);
           fclose(fp);
           return 0;
        }
        fclose(fp);

        /* Parse extra bytes string */
        for( i=0; i< strlen(value); i++)
        {
            switch(value[i]) {
                case 'k':
                case 'K':
                    size_mult = 1024;
                    value[i] = '\0';
                break;

                case 'm':
                case 'M':
                     size_mult = 1024*1024;
                     value[i] = '\0';
                break;

                case 'g':
                case 'G':
                     size_mult = 1024*1024*1024;
                     value[i] = '\0';
                break;

            }
            if( size_mult > 1)
                break;
        } 
        extra_bytes = strtoull(value, NULL, 10) * size_mult;
    }
    else

    if(extra_bytes < PKGTB_UBI_VOL_OVHEAD )
        extra_bytes = PKGTB_UBI_VOL_OVHEAD;
    
    return extra_bytes; 
}

static int finalize_tmpfile_write_nand( int vol_id, char * vol_type,
    char* dev_name_prefix, int seg_index, imgif_ctx_t *ctxP )
{
    char cmd[128];
    int ret= -1;
    printf("Flashing %s ....\n", dev_name_prefix);
    if( ctxP->pkgtb_img_segments[seg_index].type == IMGIF_PKGTB_IMG_SEGMENT_LOADER )
    {
#if CC_IMGIF_DEBUG
        sprintf(cmd, "cp /var/%s.tmp /var/%s.tmp_cooked", dev_name_prefix, dev_name_prefix);
        imgif_debug("%s\n", cmd);
        ret = system(cmd);
#endif /* CC_IMGIF_DEBUG */
        sprintf(cmd, "/var/%s.tmp",dev_name_prefix);

        /* Flash loader */
        ret = nandFlashLoader((unsigned char*)cmd, ctxP->pkgtb_img_segments[seg_index].size);
    }
    else
    {
        /* Delete update ubi volume */
        sprintf(cmd, "/dev/ubi0_%d", vol_id) ;
        if( access( cmd, F_OK ) != -1 ) 
        {
            /* If volume exists then delete it first */
            sprintf(cmd, "ubirmvol /dev/ubi0 -n %d", vol_id) ;
            imgif_debug("%s\n", cmd);
            ret = system(cmd);
        } 
        else
            ret = 0;

        if( ret )
            return ret;

        /* Create update ubi volume */
        sprintf(cmd, "ubimkvol /dev/ubi0 -s %llu -n %d -N %s%d --type=%s", 
            (unsigned long long) (ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone + nand_get_part_extra_bytes(dev_name_prefix)),
            vol_id, dev_name_prefix, ctxP->update_img_idx, vol_type) ;
        imgif_debug("%s\n", cmd);
        ret = system(cmd);

        if( ret )
            return ret;

        /* Update volume with img data */
        sprintf(cmd, "/dev/ubi0_%d", vol_id) ;
        if( nandUbiVolDevNodeExists(cmd) )
        {
            /* update ubi volume if it exists*/
            sprintf(cmd, "ubiupdatevol /dev/ubi0_%d /var/%s.tmp", vol_id, dev_name_prefix);
            imgif_debug("%s\n", cmd);
            ret = system(cmd);
        }
        else
        {
            printf("%s: Error: Ubi volume %s does not exist!\n", __FUNCTION__, cmd);
            ret = -1;
            return ret;
        }
    }
    return ret;
}

static int finalize_tmpfile_write_emmc( int vol_id, char * vol_type,
    char* dev_name_prefix, int seg_index, imgif_ctx_t *ctxP )
{
    char cmd[128];
    char dev_name[64];
    int ret= -1;
    char * block_size;
    int mmc_boot_part = 0;

    printf("Flashing %s to ", dev_name_prefix);
    if( ctxP->pkgtb_img_segments[seg_index].type == IMGIF_PKGTB_IMG_SEGMENT_LOADER )
    {
	mmc_boot_part = emmcGetBootPartIndex();
	sprintf(cmd, "echo 0 > /sys/block/mmcblk0boot%d/force_ro", mmc_boot_part);
	ret = system(cmd);
        sprintf(dev_name, "/dev/mmcblk0boot%d", mmc_boot_part);

        /* Write loader block by block, 512 bytes at a time */
        block_size = "bs=512";
    }
    else
        sprintf(dev_name, "/dev/%s%d", dev_name_prefix, ctxP->update_img_idx);

    printf("%s ....\n", dev_name);

    sprintf(cmd, "dd if=/var/%s.tmp of=%s %s 2>&1", dev_name_prefix, dev_name, block_size);
    imgif_debug("%s\n", cmd);
    ret = system(cmd);

    ret = system("sync");
    return ret;
}

static int finalize_tmpfile_write( int seg_index, imgif_ctx_t *ctxP )
{
    char * dev_name_prefix = NULL;
    char * vol_type = NULL;
    int vol_id = 0;
    char cmd[128];
    int ret = 0;
    pid_t child_pid = 0;
    imgif_flash_info_t flashInfo;

    imgif_pkgtb_get_flash_info(&flashInfo);
    switch(ctxP->pkgtb_img_segments[seg_index].type)
    {
        case IMGIF_PKGTB_IMG_SEGMENT_ROOTFS:
            /* If rootfs complete, write it to flash */
            dev_name_prefix = "rootfs";
            if (flashInfo.flashType != FLASH_INFO_FLAG_NOR)
            {
                vol_id = (ctxP->update_img_idx == PKGTB_IMG1_PART_NUM? 
                    PKGTB_NAND_ROOTFS1_VOLID:PKGTB_NAND_ROOTFS2_VOLID);
                vol_type = "dynamic";
            }
            break;

        case IMGIF_PKGTB_IMG_SEGMENT_BOOTFS:
            /* If bootfs complete, write it to flash */
            dev_name_prefix = "bootfs";
            if(flashInfo.flashType != FLASH_INFO_FLAG_NOR)
            {
                vol_id = (ctxP->update_img_idx == PKGTB_IMG1_PART_NUM? 
                    PKGTB_NAND_BOOTFS1_VOLID:PKGTB_NAND_BOOTFS2_VOLID);
                vol_type = "static";
            }
        break;

        case IMGIF_PKGTB_IMG_SEGMENT_LOADER:
            /* If loader complete, write it to flash */
            dev_name_prefix = "loader";
        break;

        default:
            printf("%s: Nothing to do for segment! %d\n", __FUNCTION__, 
            seg_index);
            return 0;
        break;
    }

    sprintf(cmd, "/var/%s.tmp", dev_name_prefix);
#if PKGTB_USE_SHELL_SHA256
    printf("\nVerifying SHA256 of %s ........", ctxP->pkgtb_img_segments[seg_index].name); 
    if(  compare_imgseg_sha256(cmd , ctxP->pkgtb_img_segments[seg_index].sha256_ptr) == 0 )
        printf("OK!\n");
    else
    {
        printf("FAILED!\n");
        return -1;
    }
#endif        

#if PKGTB_FLASH_LOADER        
    /* Since loader is the last segment to be flashed, sync environment settings */
    if( ctxP->pkgtb_img_segments[seg_index].type == IMGIF_PKGTB_IMG_SEGMENT_LOADER )
    {
        ret = synchLoaderEnv( cmd );
    }
    else
#endif /* PKGTB_FLASH_LOADER */
    {
#if PKGTB_FORKED_WRITES
        child_pid = fork();
        if( child_pid >= 0 )
        {
            if( child_pid > 0 )
            {
                printf("%s: Child PID %ld launched for flashing seg %d (%s)\n",
                        __FUNCTION__, (long)child_pid, seg_index,
                        ctxP->pkgtb_img_segments[seg_index].name);
                       
                forked_flash_writes++;
            }
        }
        else
        {
            printf("%s: Error launching Child for flashing segment %d \n",__FUNCTION__, seg_index);
            return -1;
        }
#endif                
    }

    /* Call flash specific routines */
    if( child_pid == 0 )
    {
        if( ret == 0 )
        {
            imgif_pkgtb_get_flash_info(&flashInfo);
            switch( flashInfo.flashType )
            {
                case FLASH_INFO_FLAG_NAND:
                    ret = finalize_tmpfile_write_nand(vol_id, vol_type, dev_name_prefix, seg_index, ctxP);
                    break;
                case FLASH_INFO_FLAG_EMMC:
                    ret = finalize_tmpfile_write_emmc(vol_id, vol_type, dev_name_prefix, seg_index, ctxP);
                    break;
                case FLASH_INFO_FLAG_NOR:
                    ret = finalize_tmpfile_write_spinor(vol_id, vol_type, dev_name_prefix, seg_index, ctxP);
                    break;
                default:
                    break;
            }
        }
        if( ret )
#if PKGTB_FORKED_WRITES
            printf("%s: Failed to flash segment %d. Child PID %ld, ret %d!!\n", __FUNCTION__, seg_index, (long)getpid(), ret);
#else	    
            printf("%s: Failed to flash segment %d. ret %d!!\n", __FUNCTION__, seg_index, ret);
#endif	    

        /* Delete temp file */
        /*For SPI nor, dont delete rootfs tmp file*/
        if( (flashInfo.flashType != FLASH_INFO_FLAG_NOR) || (ctxP->pkgtb_img_segments[seg_index].type != IMGIF_PKGTB_IMG_SEGMENT_ROOTFS))
        {
            sprintf(cmd,"rm -f /var/%s.tmp", dev_name_prefix);
            system(cmd);
        }
#if PKGTB_FORKED_WRITES
        /* Exit child process if not flashing loader */
        if( ctxP->pkgtb_img_segments[seg_index].type != IMGIF_PKGTB_IMG_SEGMENT_LOADER )
        {
            if( ret ) 
                ret = EXIT_FAILURE;
            else
                ret = EXIT_SUCCESS;
        
            _exit(ret);
        }
#endif        
        return ret;        
    }
    else
        return ret;
}

static void print_imgupdate_progress( imgif_ctx_t * ctxP)
{
#if CC_IMGIF_DEBUG
    int i = 0;
    int size = 0;
    int bytes_done = 0;
   
    for( i=0; i<PKGTB_SEG_IDX_MAX; i++ )
    {
        size += ctxP->pkgtb_img_segments[i].size;
        bytes_done += ctxP->pkgtb_img_segments[i].bytesdone;
    }
    printf("\rImage Update Progress: %d/%d bytes\n", bytes_done, size);
#else
    fprintf(stderr, ".");
#endif    
}

static int process_pkgt_hdr( imgif_ctx_t *ctxP)
{
    int pkgt_size = 0;
    int ret = fdt_check_header(&ctxP->pkgt_hdr);
    if(ret)
    {
        printf("Invalid FDT hdr check failed:%d! Aborting!\n", ret);
    }
    else
    {
        /* Allocate space for entire pkgt */
        pkgt_size = fdt_totalsize(&ctxP->pkgt_hdr);
        ctxP->pkgt_ptr = malloc(pkgt_size);
        memcpy(ctxP->pkgt_ptr, &(ctxP->pkgt_hdr), sizeof(struct fdt_header));

        /* Add another segment to handle actual pgkt */
        ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+1].name = "PKGT";
        ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+1].size = pkgt_size;
        ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+1].data_offset = 0;
        ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+1].bytesdone = sizeof(struct fdt_header);
        ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+1].type = IMGIF_PKGTB_IMG_SEGMENT_PKGT;
        ctxP->num_img_segments++;

#if !PKGTB_USE_SHELL_SHA256
        //FIXME: Initialize SHA256 for current segment
#endif
    }
    return ret;
}

static int process_pkgt_img_segments( imgif_ctx_t *ctxP )
{
    int images_noffset, noffset_hash;
    int noffset;
    int ndepth;
    int count;
#if CC_IMGIF_DEBUG
    int len;
    char* desc;
#endif    
    unsigned int data_size, data_offset;
    const fdt32_t *val;
    int ret = 0;
    const char * hash_name = NULL;
    UINT8* sha256_ptr = NULL;
    
    /* Find images parent node offset */
    ret = fdt_check_header(ctxP->pkgt_ptr);
    if(ret)
    {
        printf("Invalid FDT hdr check failed:%d! Aborting!\n", ret);
    }

    images_noffset = fdt_path_offset(ctxP->pkgt_ptr, PKGTB_IMAGES_PATH);
    if (images_noffset < 0) {
        printf("Can't find images parent node '%s' (%s)\n",
               PKGTB_IMAGES_PATH, fdt_strerror(images_noffset));
        return -1;
    }
    
    /* Process all image subnodes */
    for (ndepth = 0, count = 0,
         noffset = fdt_next_node(ctxP->pkgt_ptr, images_noffset, &ndepth);
        (noffset >= 0) && (ndepth > 0) && ctxP->num_img_segments < PKGTB_SEG_IDX_MAX;
        noffset = fdt_next_node(ctxP->pkgt_ptr, noffset, &ndepth)) 
    {
        if (ndepth == 1) 
        {
            /*
            * Direct child node of the images parent node,
            * i.e. component image node.
            */
            imgif_debug("Found Image %u (%s): ", count,
                   fdt_get_name(ctxP->pkgt_ptr, noffset, NULL));
            printf("\n");

            /* Description */
#if CC_IMGIF_DEBUG
            desc = (char *)fdt_getprop(ctxP->pkgt_ptr, noffset, PKGTB_DESC_PROP, &len);
            imgif_debug("    Description:  ");
            if (ret)
                imgif_debug("unavailable\n");
            else
                imgif_debug("%s\n", desc);
#endif                

            /* Offset - Check for both relative and absolute offsets */
            val = fdt_getprop(ctxP->pkgt_ptr, noffset, PKGTB_DATA_OFFSET_PROP, NULL);
            if( val ) 
            {
                /* Relative offset */
                data_offset = fdt32_to_cpu(*val);
                data_offset += ((fdt_totalsize(ctxP->pkgt_ptr) + 3) & ~3);
            }
            else
            {
                val = fdt_getprop(ctxP->pkgt_ptr, noffset, PKGTB_DATA_POSITION_PROP, NULL);
                if (!val)
                    return -1;

                /* Absolute offset */
                data_offset = fdt32_to_cpu(*val);
            }

            /* Size */
            val = fdt_getprop(ctxP->pkgt_ptr, noffset, PKGTB_DATA_SIZE_PROP, NULL);
            if (!val)
                return -1;
            
            data_size = fdt32_to_cpu(*val);
            imgif_debug("    Offset:0x%08x Size:0x%08x\n", data_offset, data_size);
            count++;

            /* Get hash value */
            fdt_for_each_subnode(noffset_hash, ctxP->pkgt_ptr, noffset) 
            {
                hash_name = fdt_get_name(ctxP->pkgt_ptr, noffset_hash, NULL);
                if (!strncmp(hash_name, PKGTB_HASH_NODENAME, strlen(PKGTB_HASH_NODENAME)))
                {
                    sha256_ptr = (uint8_t *)fdt_getprop(ctxP->pkgt_ptr, noffset_hash, PKGTB_VALUE_PROP, NULL);
                }
            }

            /* Add segment */
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+count].name = (char*)fdt_get_name(ctxP->pkgt_ptr, noffset, NULL);
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+count].size = data_size;
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+count].data_offset = data_offset;
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+count].bytesdone = 0;
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+count].type = IMGIF_PKGTB_IMG_SEGMENT_UNKNOWN;
            if( hash_name && sha256_ptr )
                ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx+count].sha256_ptr = sha256_ptr;

#if !PKGTB_USE_SHELL_SHA256
            //FIXME: Initialize SHA256 for current segment
#endif
            ctxP->num_img_segments++;
        }
    }
    return 0;
}

static void dump_img_segments( imgif_ctx_t * ctxP )
{
    int i;

    printf("PKGTB update image detected. Image Segments found:\n");
    for( i=0; i<ctxP->num_img_segments; i++ )
    {
        printf("    [%d] %s --> offset:0x%08x, size:0x%08x, type:%d\n",
            i, 
            ctxP->pkgtb_img_segments[i].name,
            (unsigned int)ctxP->pkgtb_img_segments[i].data_offset,
            (unsigned int)ctxP->pkgtb_img_segments[i].size,
            ctxP->pkgtb_img_segments[i].type);
    }
    printf("Current_img_idx: %d, Update_img_idx: %d\n", ctxP->booted_img_idx, ctxP->update_img_idx);
}

static int select_pkgt_img_segments( imgif_ctx_t *ctxP, char * name, IMGIF_PKGTB_IMG_SEGMENTS type )
{
    int i;

    for( i=0; i<ctxP->num_img_segments; i++ )
    {
        if ( strcmp(ctxP->pkgtb_img_segments[i].name, name) == 0 )
        {
            ctxP->pkgtb_img_segments[i].type = type;
            return i;
        }
    }
    return PKGTB_SEG_IDX_INVALID;
}

static FILE * init_storage_dev( char * dev_name_prefix, int seg_index, imgif_ctx_t *ctxP)
{
    char file_name[128];
#if PKGTB_USE_TMP_FILES_ALL            
    /* Open temp files for all binaries */
    sprintf(file_name, "/var/%s.tmp", dev_name_prefix);
    imgif_debug("Opening file %s for writing\n", file_name );
    return(fopen(file_name, "w"));
#else
    switch(ctxP->pkgtb_img_segments[seg_index].type)
    {
        case IMGIF_PKGTB_IMG_SEGMENT_LOADER:
            /* loader will still need a temp file */
            sprintf(file_name, "/var/%s.tmp", dev_name_prefix);
            imgif_debug("Opening file %s for writing\n", file_name );
            return(fopen(file_name, "w"));
        break;

        case IMGIF_PKGTB_IMG_SEGMENT_ROOTFS:
        case IMGIF_PKGTB_IMG_SEGMENT_BOOTFS:
            //FIXME: Replace with actual deleting/opening of flash device for rootfs/bootfs
        break;

        default:
        break;
    }
    return NULL;
#endif            
}

static int check_compat_prop(char * prop, char * values)
{
    int ret = -1;
    imgif_flash_info_t flashInfo;
    char blksize[128];
    char * tmp = NULL;
    if( strcmp(prop, "chip") == 0 )
    {
        /* Check if socname matches */
        if( bcmImg_MatchChipId(values) )
            ret = 0;

        if( ret )
            printf("%s: Error! chipid check failed\n", __FUNCTION__);
    }
    else if( strcmp(prop, "flash") == 0 )
    {
        /* Check if flash type matches */
        imgif_pkgtb_get_flash_info(&flashInfo);
        switch( flashInfo.flashType )
        {
            case FLASH_INFO_FLAG_NAND:
                if( (tmp = strstr(values, "nand")) )
                {
                    /* Check if there is a blksize appended */
                    tmp += strlen("nand");
                    if( '9' >= *tmp && *tmp >= '0' )   
                    {
                        sprintf(blksize, "%d", flashInfo.eraseSize/1024);
                        if( strstr(values, blksize) )
                            ret = 0;
                    }
                    else 
                        ret = 0;
                }

                if( ret )
                    printf("%s: Error! Invalid img flash format:%s, supported format:nand%d\n",
                         __FUNCTION__, values, flashInfo.eraseSize/1024);
                    
                break;
            case FLASH_INFO_FLAG_EMMC:
                if( strstr(values, "emmc") )
                    ret = 0;
                if( ret )
                    printf("%s: Error! Invalid img flash format:%s, supported format:emmc\n",
                        __FUNCTION__, values);
                break;
            case FLASH_INFO_FLAG_NOR:
                if( strstr(values, "nor") )
                    ret = 0;

                if( ret )
                    printf("%s: Error! Invalid img flash format:%s, supported format:nor\n",
                        __FUNCTION__, values);
                break;
            
            default:
                break;
        }
    }
    else if( strcmp(prop, "rev") == 0 )
    {
        /* Check chip rev */
        ret = 0;
    }
    else if( strcmp(prop, "fstype") == 0 )
    {
        /* Check filesystem type compatibility */
        ret = 0;
    }
    else if( strcmp(prop, "ddr") == 0 )
    {
        /* Check ddr compatibility */
        ret =0;
    }
    else
    {
        ret = 0;
    }
        
    return ret;
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
        size_t srclen, len;

        srclen = strlen(src);
        if (size <= 0)
                return srclen;

        len = (srclen < size) ? srclen : size - 1;
        memcpy(dst, src, len); /* should not overlap */
        dst[len] = '\0';

        return srclen;
}

int check_pkgtb_compat_string( char* compat_str )
{
    char * token;
    char  * prop;
    char  * values;
    char * comp_str_buf = NULL;
    int ret = 0;
    int msize;

    msize = strlen(compat_str);
    comp_str_buf = malloc(msize);
    memset(comp_str_buf, 0, msize);

    if( comp_str_buf )
    {
        strlcpy(comp_str_buf, compat_str, msize);
        imgif_debug("Comp_str: %s\n", comp_str_buf);
        token = strtok(comp_str_buf, PKGTB_COMPAT_PARAM_DELIM);
        while (token != NULL && !ret)
        {
            imgif_debug("Param: %s\n", token);
            prop = token;
            values = strstr(token, PKGTB_COMPAT_VALUE_REF)+1;
            *(values-1) = '\0';
            imgif_debug("        prop:%s values:%s\n", prop, values);
            ret = check_compat_prop(prop, values);
            token = strtok(NULL, ";");
        }
        if( ret )
            printf("%s: Error! Image compatibility check FAILS! Aborting upgrade!\n", __FUNCTION__);

        free(comp_str_buf);
    }
    else
            printf("%s: Error! Cannot allocate memory for compat string! Aborting upgrade!\n", __FUNCTION__);

    return ret;
}

static int process_pkgt( imgif_ctx_t *ctxP )
{
    char * desc_ptr;
    int conf_noffset;
    int len;
    char path[128];
    int seg_index = 0;
    uint64_t bootfs_rootfs_size = 0;
    uint64_t loader_size = 0;
    uint64_t avail_space = 0;
    int valid_flag;
    uint64_t bootfs_size = 0;
    uint64_t rootfs_size = 0;
    imgif_flash_info_t flashInfo;

    imgif_pkgtb_get_flash_info(&flashInfo);

    /* Iterate through all images and record their offsets and sizes */
    if( process_pkgt_img_segments( ctxP ) < 0 )
        return -1;
    
    /* Identify and select the segments we want via default configuration */
    conf_noffset = fdt_path_offset(ctxP->pkgt_ptr, PKGTB_CONFS_PATH);
    if (conf_noffset < 0) {
        printf("Can't find images parent node '%s' (%s)\n",
               PKGTB_CONFS_PATH, fdt_strerror(conf_noffset));
        return -1;
    }

    /* Get default configuration */
    desc_ptr = (char *)fdt_getprop(ctxP->pkgt_ptr, conf_noffset, PKGTB_DEFAULT_PROP, &len);
    if(desc_ptr)
    {
        imgif_debug("Default Configuration: %s\n", desc_ptr);
        sprintf(path,"%s/%s",PKGTB_CONFS_PATH,desc_ptr);
        conf_noffset = fdt_path_offset(ctxP->pkgt_ptr,path);
        if (conf_noffset < 0) {
            printf("Can't find configuration node '%s' (%s)\n",
                   path, fdt_strerror(conf_noffset));
            return -1;
        }

        /* Check compatibility string */
        desc_ptr = (char *)fdt_getprop(ctxP->pkgt_ptr, conf_noffset, "compatible", &len);
        if(desc_ptr)
        {
            if(check_pkgtb_compat_string(desc_ptr))
                return -1;
        }
        else
        {
            printf("%s: Error! Missing compatibility string! Cannot determine SoC name!\n", __FUNCTION__);
            return -1;
        }
        
        /* Get bootfs */
        desc_ptr = (char *)fdt_getprop(ctxP->pkgt_ptr, conf_noffset, "bootfs", &len);
        if(desc_ptr)
        {
            imgif_debug("    bootfs: %s\n", desc_ptr);
            seg_index = select_pkgt_img_segments( ctxP, desc_ptr, IMGIF_PKGTB_IMG_SEGMENT_BOOTFS);
            if( flashInfo.flashType == FLASH_INFO_FLAG_NOR )
                bootfs_size += ctxP->pkgtb_img_segments[seg_index].size;
            else
                bootfs_rootfs_size += ctxP->pkgtb_img_segments[seg_index].size + nand_get_part_extra_bytes("bootfs");
            ctxP->bootfs_fh = init_storage_dev("bootfs", seg_index, ctxP);
            if( !ctxP->bootfs_fh )
            {
                printf("%s: Error! Cant open file for bootfs flashing!\n", __FUNCTION__);
                return -1;
            }
        }

        /* Get rootfs */
        desc_ptr = (char *)fdt_getprop(ctxP->pkgt_ptr, conf_noffset, "rootfs", &len);
        if(desc_ptr)
        {
            imgif_debug("    rootfs: %s\n", desc_ptr);
            seg_index = select_pkgt_img_segments( ctxP, desc_ptr, IMGIF_PKGTB_IMG_SEGMENT_ROOTFS);
            if( flashInfo.flashType == FLASH_INFO_FLAG_NOR )
                rootfs_size += ctxP->pkgtb_img_segments[seg_index].size;
            else
                bootfs_rootfs_size += ctxP->pkgtb_img_segments[seg_index].size + nand_get_part_extra_bytes("rootfs") ;
            ctxP->rootfs_fh = init_storage_dev("rootfs", seg_index, ctxP);
            if( !ctxP->rootfs_fh )
            {
                printf("%s: Error! Cant open file for rootfs flashing!\n", __FUNCTION__);
                return -1;
            }
        }

        /* Get loader */
        desc_ptr = (char *)fdt_getprop(ctxP->pkgt_ptr, conf_noffset, "loader", &len);
        if(desc_ptr)
        {
            imgif_debug("    loader: %s\n", desc_ptr);
            seg_index = select_pkgt_img_segments( ctxP, desc_ptr, IMGIF_PKGTB_IMG_SEGMENT_LOADER);
            loader_size += ctxP->pkgtb_img_segments[seg_index].size;
            ctxP->loader_ptr = malloc(ctxP->pkgtb_img_segments[seg_index].size);
            ctxP->loader_fh = init_storage_dev("loader", seg_index, ctxP);
            if( !ctxP->loader_fh || !ctxP->loader_ptr )
            {
                if( !ctxP->loader_ptr )
                    printf("%s: Error! Cant alloc memory for loader flashing!\n", __FUNCTION__);
                else
                {
                    printf("%s: Error! Cant open file for loader flashing!\n", __FUNCTION__);
                    free(ctxP->loader_ptr);
                    ctxP->loader_ptr = NULL;
                }
                return -1;
            }
            ctxP->loader_seg_index = seg_index;
        }
        else
            ctxP->loader_seg_index = PKGTB_SEG_IDX_INVALID;

        /* We know all sizes now, determine if it will fit in flash */
        if( flashInfo.flashType == FLASH_INFO_FLAG_NOR )
        {
            if(rootfs_size)
            {
                avail_space = spinorGetAvailSpace(SPI_NOR_ROOTFS_MTD);
                if ( rootfs_size > avail_space)
                {
                    printf("%s: Error! rootfs MTD partition too small for new rootfs!\n",
                        __FUNCTION__);
                    printf("%s: Required space:%llu, Available space:%llu\n", __FUNCTION__, 
                        (unsigned long long)rootfs_size, (unsigned long long)avail_space);
                    if( loader_size )
                    {					
                        free(ctxP->loader_ptr);
                        ctxP->loader_ptr = NULL;
                    } 						
                    return -1;
                }
            }
            if(bootfs_size)
            {
                avail_space = spinorGetAvailSpace(SPI_NOR_BOOTFS_MTD);
                if ( bootfs_size > avail_space)
                {
                    printf("%s: Error! bootfs MTD partition too small for new bootfs !\n",
                         __FUNCTION__);
                    printf("%s: Required space:%llu, Available space:%llu\n", __FUNCTION__, 
                        (unsigned long long)bootfs_size, (unsigned long long)avail_space);
                    if( loader_size )
                    {					
                        free(ctxP->loader_ptr);
                        ctxP->loader_ptr = NULL;
                    }
                    return -1;
                }
            }
        }
        else
        {
            if( bootfs_rootfs_size )
            {
                avail_space = getAvailImgSpace(ctxP->update_img_idx);
                if( bootfs_rootfs_size > avail_space)
                {
                    printf("%s: Error! image partition(s) too small for new bootfs and rootfs!\n",
                        __FUNCTION__);
                    printf("%s: Required space:%llu, Available space:%llu\n", __FUNCTION__, 
                        (unsigned long long)bootfs_rootfs_size, (unsigned long long)avail_space);
                    if( loader_size )
                    {
                        free(ctxP->loader_ptr);
                        ctxP->loader_ptr = NULL;
                    }
                    return -1;
                 }					
            }
        }

        /* Check if loader will fit */
        if( loader_size )
        {
            avail_space = getAvailLoaderSpace(ctxP->update_img_idx);
            if( loader_size > avail_space )
            {
                printf("%s: Error! loader MTD partition too small for new loader binary!\n",
                    __FUNCTION__);
                free(ctxP->loader_ptr);
                ctxP->loader_ptr = NULL;
                return -1;
            }
        }
        
        /* Display all parsed segments */
        dump_img_segments(ctxP);

        /* Clear valid flag in metadata for new image */
        //FIXME SPI nor only support single image
        if( flashInfo.flashType != FLASH_INFO_FLAG_NOR)
        {
            valid_flag = 0;
            setImgValidStatus(ctxP->update_img_idx, &valid_flag);
        }
    }
    else
    {
        printf("%s: Error Default configurations not found in upgrade image!\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

static int handle_segment_completion( imgif_ctx_t *ctxP )
{
    FILE * fh = NULL;
    int ret = 0;

    ctxP->num_img_segments_complete++;

    switch(ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].type)
    {
        case IMGIF_PKGTB_IMG_SEGMENT_PKGT_HDR:
            /* If we have the pkgt header, process its contents */
            return(process_pkgt_hdr(ctxP));

        case IMGIF_PKGTB_IMG_SEGMENT_PKGT:
            /* We now have full pkgt, process it and setup other segments */
            return(process_pkgt(ctxP));

        case IMGIF_PKGTB_IMG_SEGMENT_LOADER:
            /* Loader is cached until all other segments have been flashed */
#if !PKGTB_USE_SHELL_SHA256            
            //FIXME: Compare incr SHA of loader
#endif
        break;

        case IMGIF_PKGTB_IMG_SEGMENT_ROOTFS:
            /* If rootfs complete, write it to flash */
            fh = ctxP->rootfs_fh;
            ctxP->rootfs_fh = NULL;
#if !PKGTB_USE_SHELL_SHA256            
            //FIXME: Compare incr SHA of ROOTFS
#endif
        break;

        case IMGIF_PKGTB_IMG_SEGMENT_BOOTFS:
            /* If bootfs complete, write it to flash */
            fh = ctxP->bootfs_fh;
            ctxP->bootfs_fh = NULL;
#if !PKGTB_USE_SHELL_SHA256            
            //FIXME: Compare incr SHA of BOOTFS
#endif
        break;

        default:
            printf("%s: Nothing to do for segment! %d\n", __FUNCTION__, 
            ctxP->current_img_segment_idx);
        break;
    }

    /* Close file*/
    if( fh )
    {
        ret = system("sync");
        fclose(fh);

#if PKGTB_USE_TMP_FILES_ALL            
        ret = finalize_tmpfile_write( ctxP->current_img_segment_idx, ctxP );
#endif
    }
    return ret;
}

static int finalize_loader( imgif_ctx_t *ctxP )
{
    int ret = 0;
#if PKGTB_FLASH_LOADER        
    /* Write loader data */
    imgif_debug("Writing %llu bytes of loader to file:%p\n", 
        (unsigned long long)ctxP->pkgtb_img_segments[ctxP->loader_seg_index].bytesdone, (ctxP->loader_fh));
    fwrite(ctxP->loader_ptr, 1, ctxP->pkgtb_img_segments[ctxP->loader_seg_index].bytesdone,
        ctxP->loader_fh);
#endif /* PKGTB_FLASH_LOADER */   
    
    if( ferror(ctxP->loader_fh) 
       || feof(ctxP->loader_fh) )
    {
        printf("%s: Error writing segment %d\n",
            __FUNCTION__, ctxP->loader_seg_index);
        ret = -1;
    }
    else
    {
        fclose(ctxP->loader_fh);
        ctxP->loader_fh = NULL;
#if PKGTB_FLASH_LOADER        
        ret = finalize_tmpfile_write( ctxP->loader_seg_index, ctxP );
#endif
    }
    return ret;
}

/***********************************************************************************
 *                        PKGTB IMGIF FUNCTIONS                                    *
 ***********************************************************************************/

/*****************************************************************************
*  FUNCTION:  init_ctx
*  PURPOSE:   initialize pkgtb img write parameters
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int init_ctx(imgif_ctx_t *ctxP)
{
    int ret=-1;
    imgif_flash_info_t flashInfo;

    /* Initialize image segment values */
    ctxP->current_img_segment_idx = 0;
    ctxP->num_img_segments = 1;
    ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].name = "PKGT_HDR";
    ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].size = sizeof(struct fdt_header);
    ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].data_offset = 0;
    ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone = 0;
    ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].type = IMGIF_PKGTB_IMG_SEGMENT_PKGT_HDR;

    /* Initialize whole image crc */
    if (ctxP->calCrc32Cb != NULL)
        ctxP->calCrc32Cb(0, &ctxP->crc_image_calc, NULL, 0);

#if !PKGTB_USE_SHELL_SHA256
    //FIXME: Initialize SHA256 for current segment
#endif

    /* Initialize image size */
    ctxP->total_img_bytesdone = 0;

    /* Get boot partition */
    //FIXME only support single image for spinor
    imgif_pkgtb_get_flash_info(&flashInfo);
    if( flashInfo.flashType == FLASH_INFO_FLAG_NOR )
    {
        ctxP->booted_img_idx = 1;
        ctxP->update_img_idx = 1;
    }
    else
    {
        ret = getBootPartition();
        if( ret == -1 )
        {
            printf("%s: Error while retrieving boot partition number!\n", __FUNCTION__);
            return ret;
        }
        else
        {
            ctxP->booted_img_idx = ret;
            ctxP->update_img_idx = (ctxP->booted_img_idx==PKGTB_IMG1_PART_NUM)?
                PKGTB_IMG2_PART_NUM:PKGTB_IMG1_PART_NUM;
        }
    }
    
#if defined(CUSTOM_NAND_SINGLE_IMAGE) || defined(RTCONFIG_SINGLEIMG_B)
    if (ctxP->force_upd_img_idx) {
        ctxP->update_img_idx = ctxP->force_upd_img_idx;
        printf("%s: Single Image: Force to boote image 1 , update image to %d !!\n", __FUNCTION__, ctxP->update_img_idx);        
    }
#endif    

    return 0;
}

/*****************************************************************************
*  FUNCTION:  deinit_ctx
*  PURPOSE:   free all pkgtb image write related resources
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int deinit_ctx(imgif_ctx_t *ctxP)
{
    imgif_debug("Closing all files\n"); 

#if IMG_WRITE_FS    
    /* Close all open files */
    if(ctxP->loader_fh)      
        fclose(ctxP->loader_fh);   
    if(ctxP->rootfs_fh)
        fclose(ctxP->rootfs_fh);       
    if(ctxP->bootfs_fh)
        fclose(ctxP->bootfs_fh);       
#endif    

    imgif_debug("Freeing all allocated memory\n"); 

    /* Free allocated memory */
    if( ctxP->loader_ptr )
        free(ctxP->loader_ptr);

    if( ctxP->pkgt_ptr )
        free(ctxP->pkgt_ptr);

    /* Clear context */
    memset(ctxP, 0, sizeof(imgif_ctx_t));
    ctxP->loader_seg_index = PKGTB_SEG_IDX_INVALID;
    ctxP->current_img_segment_idx = PKGTB_SEG_IDX_INVALID;
    return 0;
}

/*****************************************************************************
*  FUNCTION:  write_bytes_to_pkgtb_device
*  PURPOSE:   write bytes to pkgtb devices
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*      data (IN) - data pointer
*      pktbuf_num_bytes (IN) - number of bytes to write
*  RETURNS:
*      >0 - number of bytes written
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int write_bytes_to_pkgtb_device( imgif_ctx_t *ctxP, char * data, unsigned int pktbuf_num_bytes )
{
    unsigned int bytes_left, bytes_to_write;
    unsigned int written_bytes = 0;
    unsigned int pktbuf_segdata_offset = 0;
    char * datap = data;
    char * dest_ptr = NULL;
    FILE * dest_fh = NULL;

    imgif_debug("num_bytes:%d\n", pktbuf_num_bytes);

    while( pktbuf_num_bytes )
    {
        /* Start processing writes */
        imgif_debug("segment:%d, type:%d, seg_bytes_done:%llu/%llu, written_bytes:%d/%d\n", 
            ctxP->current_img_segment_idx,
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].type,
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone,
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].size, 
            written_bytes,
            pktbuf_num_bytes+written_bytes); 

        /* Check if data for current segment lies in current pktbuf_num_bytes */
        if( (ctxP->current_img_segment_idx < ctxP->num_img_segments) 
            && (ctxP->total_img_bytesdone + pktbuf_num_bytes > ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].data_offset) )
        {
            /* Data for current segment lies in current chunk of pktbuf_num_bytes */
            if( ctxP->total_img_bytesdone >= ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].data_offset )
                pktbuf_segdata_offset = 0;
            else
                pktbuf_segdata_offset = ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].data_offset - ctxP->total_img_bytesdone;

            /* Calculate how many bytes left to copy for segment */
            bytes_left = ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].size 
                - ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone;

            /* Calculate how many bytes we can actually write */
            bytes_to_write = (bytes_left <= (pktbuf_num_bytes-pktbuf_segdata_offset))? bytes_left:(pktbuf_num_bytes-pktbuf_segdata_offset);
            imgif_debug("segment_offs:%llu, segdata_offs:%d, bytes_to_write:%d\n",
                ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].data_offset, 
                pktbuf_segdata_offset,
                bytes_to_write);
        }
        else
        {
            if( ctxP->current_img_segment_idx >= ctxP->num_img_segments )
            {
               /* If all image data segments have have been written, but still need 
                * to write more bytes this means that image may be corrupted (may 
                * possibly happen if there is extra padding added to end of file). 
                */

                printf("%s: Error: Attempt to write %d unknown img data bytes to invalid segment!\n", 
                        __FUNCTION__, pktbuf_num_bytes);
                return -1;
            }
            else
            {
                /* Data for segment is not in current pktbuf_num_bytes, therefore just consume bytes */
                imgif_debug("CONSUMING %d bytes\n", pktbuf_num_bytes);
            }

            /* Do whole image crc on all data */
            if (ctxP->calCrc32Cb != NULL)
            {
                ctxP->calCrc32Cb(1, &ctxP->crc_image_calc, (unsigned char*)datap, pktbuf_num_bytes);
                imgif_debug("crc img  tally:0x%08x\n", ctxP->crc_image_calc);
            }

            /* Increment our counters by pktbuf_num_bytes and quit loop */
            datap                       += pktbuf_num_bytes;
            written_bytes               += pktbuf_num_bytes;
            ctxP->total_img_bytesdone   += pktbuf_num_bytes;
            pktbuf_num_bytes            = 0;
            pktbuf_segdata_offset       = 0;
            continue;
        }

        /* Figure out whether we need to cache the segment data in memory or write it to fs */
        switch(ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].type)
        {
            case IMGIF_PKGTB_IMG_SEGMENT_PKGT_HDR:
                /* Copy to local buffer */
                dest_ptr = (char*)&(ctxP->pkgt_hdr);
            break;

            case IMGIF_PKGTB_IMG_SEGMENT_PKGT:
                /* Copy to local buffer */
                dest_ptr = ctxP->pkgt_ptr;
            break;

            case IMGIF_PKGTB_IMG_SEGMENT_LOADER:
                /* write to local loader */
                dest_ptr = ctxP->loader_ptr;
            break;

            case IMGIF_PKGTB_IMG_SEGMENT_ROOTFS:
                dest_fh = ctxP->rootfs_fh;
            break;

            case IMGIF_PKGTB_IMG_SEGMENT_BOOTFS:
                dest_fh = ctxP->bootfs_fh;
            break;

            default:
                printf("%s: Invalid segment! %d\n", __FUNCTION__, ctxP->current_img_segment_idx);
                return -1;
            break;
        }

        /* Write data to either local memory or to filesystem */
        if( dest_ptr )
        {
            imgif_debug("Writing %d bytes to ptr:%p\n", bytes_to_write, 
                (dest_ptr + ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone));
            memcpy(dest_ptr + ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone,
                datap+pktbuf_segdata_offset, bytes_to_write); 
            dest_ptr = NULL;
        }
        else if( dest_fh )
        {
#if IMG_WRITE_FS                
            imgif_debug("Writing %d bytes to file:%p\n", bytes_to_write, 
                (dest_fh));
            fwrite(datap+pktbuf_segdata_offset, 1, bytes_to_write, dest_fh);
            if( ferror(dest_fh) || feof(dest_fh) )
            {
                printf("%s: Error writing segment %d\n", 
                        __FUNCTION__,
                        ctxP->current_img_segment_idx);
                return -1;
            }
            dest_fh = NULL;
#endif                
        }

        /* Do whole image crc on all image data including skipped bytes */
        if (ctxP->calCrc32Cb != NULL)
        {
            ctxP->calCrc32Cb(1, &ctxP->crc_image_calc, 
                (unsigned char*)datap, 
                bytes_to_write+pktbuf_segdata_offset);
            imgif_debug("crc img  tally:0x%08x\n", ctxP->crc_image_calc);
        }

#if !PKGTB_USE_SHELL_SHA256
        //FIXME: Add incremental SHA
#endif

        /* Increment bytes done, advance data pointer and adjust remaining bytes */
        ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone += bytes_to_write;
        pktbuf_num_bytes            -= (bytes_to_write + pktbuf_segdata_offset);
        datap                       += (bytes_to_write + pktbuf_segdata_offset);
        written_bytes               += (bytes_to_write + pktbuf_segdata_offset);
        ctxP->total_img_bytesdone   += (bytes_to_write + pktbuf_segdata_offset);
        pktbuf_segdata_offset       = 0;

        imgif_debug("img_bytesdone:%llu, seg_bytes_done:%llu/%llu, written_bytes:%d/%d\n", 
            ctxP->total_img_bytesdone,
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone, 
            ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].size,
            written_bytes, 
            pktbuf_num_bytes+written_bytes); 

        /* Move to next segment if current segment is done */
        if( ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].bytesdone == 
                ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].size )
        {
            imgif_debug("Segment %d completed\n", ctxP->current_img_segment_idx);

            /* Handle one completed segment */
            if( handle_segment_completion(ctxP) )
                return -1;

            /* Increment to next valid segment */
            while( ctxP->current_img_segment_idx < ctxP->num_img_segments  )
            {
                ctxP->current_img_segment_idx++;
                imgif_debug("Segment change:%d->%d\n", 
                    ctxP->current_img_segment_idx-1, 
                    ctxP->current_img_segment_idx);

                /* Break out if valid segment is detected */
                if( ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].size  &&
            (ctxP->pkgtb_img_segments[ctxP->current_img_segment_idx].type 
                        != IMGIF_PKGTB_IMG_SEGMENT_UNKNOWN ))
                    break;
            }
               
        }
        imgif_debug("\n");
    }
    print_imgupdate_progress(ctxP);
    return written_bytes;
}

#if CC_IMGIF_DEBUG
/*****************************************************************************
*  FUNCTION:  dump_ctx
*  PURPOSE:   dump all pkgtb img write related parameters.
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*  RETURNS:
*  NOTES:
*      None.
*****************************************************************************/
static void dump_ctx(imgif_ctx_t *ctxP)
{
    int i = 0;
    imgif_debug("------------------------imgif_pkgtb ctx------------------------------\n");
    for( i=0; i<PKGTB_SEG_IDX_MAX ; i++ )
    {
        imgif_debug("Img segment %d, fl_offset %08llu, size %08llu, bytesdone %08llu\n", i,
        ctxP->pkgtb_img_segments[i].data_offset,
        ctxP->pkgtb_img_segments[i].size,
        ctxP->pkgtb_img_segments[i].bytesdone);
    }
    imgif_debug("crc_image_calc 0x%08x crc_image_exp 0x%08x\n", 
        ctxP->crc_image_calc, ctxP->imgInfoExt.crc);
    imgif_debug("boot_partition          : %d\n", ctxP->booted_img_idx);
    imgif_debug("update_partition        : %d\n", ctxP->update_img_idx);
    imgif_debug("--------------------------------------------------------------------\n");
}
#endif

/*****************************************************************************
*  FUNCTION:  finalize_pkgtb_img_write
*  PURPOSE:   Finalize all write operations.
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*      abortFlag (IN) - flag indicating early abort.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int finalize_pkgtb_img_write(imgif_ctx_t *ctxP, unsigned char abortFlag)
{
    int ret = -1;
    int valid_flag;
    int seq_num = 0;
    int other_image;
    imgif_flash_info_t flashInfo;
#if PKGTB_FORKED_WRITES
    int child_ret = 0;
    pid_t child_pid = 0;
    int write_fails = 0;

    /* Make sure that the other forked flash writes have completed successfully */
    while(forked_flash_writes)
    {
        child_pid = wait(&child_ret);
        child_ret = WEXITSTATUS(child_ret);
        printf("%s: Child with PID %ld exited with status 0x%x.\n",__FUNCTION__, (long)child_pid, child_ret);
        forked_flash_writes--;
        if( child_ret )
            write_fails++;
    }
    if(write_fails)
    {
        printf("%s: Error! Segment flashing failed.\n",__FUNCTION__);
        goto cleanup;
    }
#endif

    if( abortFlag )
        goto cleanup;

    /* Check if the whole image CRC matched */
    if((ctxP->calCrc32Cb != NULL) && (ctxP->imgInfoExt.bitmask & IMG_INFO_BITMASK_CRC))
    {

        ctxP->calCrc32Cb(2, &ctxP->crc_image_calc, NULL, 0);
        if( ctxP->crc_image_calc != ctxP->imgInfoExt.crc )
        {
            printf("%s: Whole Image CRC check failed! expected:0x%08x, calculated:0x%08x\n",
                __FUNCTION__,
                ctxP->imgInfoExt.crc,
                ctxP->crc_image_calc);
            goto cleanup;
        }
    }
    
    /* Check if size matched */
    if ((ctxP->imgInfoExt.bitmask & IMG_INFO_BITMASK_SIZE) && (ctxP->imgInfoExt.size != ctxP->total_img_bytesdone))
    {
        printf("Unmatched file size values, ext=0x%08x, cal=0x%llx",
          ctxP->imgInfoExt.size, (unsigned long long)ctxP->total_img_bytesdone);
        goto cleanup;
    }

#if IMG_WRITE_FS        

    /* Verify we actually wrote out correct number of img segments. */
    if (ctxP->num_img_segments != ctxP->num_img_segments_complete)
    {
        printf("%s: ERROR: img segments in header=%d complete=%d\n",
               __FUNCTION__,
               ctxP->num_img_segments, ctxP->num_img_segments_complete);
        goto cleanup;
    }

    /* Write loader to partition */
    if( ctxP->loader_seg_index != PKGTB_SEG_IDX_INVALID &&
        ctxP->pkgtb_img_segments[ctxP->loader_seg_index].bytesdone )
    {
        if(finalize_loader(ctxP)) 
        {
            printf("%s: Error writing loader segment %d\n",
                    __FUNCTION__,
                    ctxP->loader_seg_index);
            goto cleanup;
        }
    }
    
    /* Set image as valid */
    //FIXME SPI NOR only support single image
    imgif_pkgtb_get_flash_info(&flashInfo);
#if defined(CUSTOM_NAND_SINGLE_IMAGE) || defined(RTCONFIG_SINGLEIMG_B)
    if ( flashInfo.flashType != FLASH_INFO_FLAG_NOR )
    {
        if (ctxP->force_upd_img_idx) {
            int other_seq_num = -1, other_valid_flag = 0;
            valid_flag = 1;
            setImgValidStatus(ctxP->update_img_idx, &valid_flag);
            getImgSeqNum (ctxP->booted_img_idx, &seq_num);

            other_image = ctxP->update_img_idx == 1 ? 2 : 1;
            getImgValidStatus(other_image, &other_valid_flag);
            if (other_valid_flag)
            {
                getImgSeqNum (other_image, &other_seq_num);
            }
            seq_num = (seq_num > other_seq_num)?seq_num:other_seq_num;
            seq_num = (seq_num + 1 ) % 1000;
            if (seq_num < other_seq_num) {
                seq_num = 1;
                other_seq_num = 0;
                setImgSeqNum(ctxP->update_img_idx, seq_num);
                setImgSeqNum(other_image, other_seq_num);
            } else {
                setImgSeqNum(ctxP->update_img_idx, seq_num);
            }
            printf("%s: Single Image: seq_num %d %d valid %d %d update_img_idx %d\n",
                        __FUNCTION__, seq_num, other_seq_num,
                        valid_flag, other_valid_flag, ctxP->update_img_idx);
            if (setBootImageState(BOOT_SET_NEW_IMAGE) != 0) {
                fprintf(stderr,"%s setBootImageState(BOOT_SET_NEW_IMAGE) failed\n", __FUNCTION__);
            } 
        } else {
            valid_flag = 1;
            fprintf(stderr, "%s:%d calling setImgValidStatus\n", __FUNCTION__, __LINE__);
            setImgValidStatus(ctxP->update_img_idx, &valid_flag);
            other_image = ctxP->update_img_idx == 1 ? 2 : 1;
            getImgValidStatus(other_image, &valid_flag);
            if (valid_flag)
            {
                getImgSeqNum (other_image, &seq_num);
                seq_num = (seq_num + 1) % 1000;
            }
            setImgSeqNum( ctxP->update_img_idx, seq_num);
        }

    } 
#else
    if ( flashInfo.flashType != FLASH_INFO_FLAG_NOR )
    {
        valid_flag = 1;
        fprintf(stderr, "%s:%d calling setImgValidStatus\n", __FUNCTION__, __LINE__);
        setImgValidStatus(ctxP->update_img_idx, &valid_flag);
        other_image = ctxP->update_img_idx == 1 ? 2 : 1;
        getImgValidStatus(other_image, &valid_flag);
        if (valid_flag)
        {
            getImgSeqNum (other_image, &seq_num);
            seq_num = (seq_num + 1) % 1000;
        }
        setImgSeqNum( ctxP->update_img_idx, seq_num);
    }
#endif
#endif        
    ret = 0;

cleanup:   
    /* deinitialize context */
    deinit_ctx(ctxP);

    return ret;
}

/*****************************************************************************
*  FUNCTION:  imgif_pkgtb_open
*  PURPOSE:   Initialize IMGIF context.
*  PARAMETERS:
*      fmtParserCb (IN) Image parser callback function.
       calCrc32Cb (IN) Application-specific CRC algorithm callback function.
*  RETURNS:
*      Pointer to the IMGIF context - successful operation.
*      NULL - failed operation, for example, another software upgrade already
*             in progress.
*  NOTES:
*      None.
*****************************************************************************/
IMGIF_HANDLE imgif_pkgtb_open(IMG_FORMAT_PARSER_CB fmtParserCb,
  CAL_CRC32_CB calCrc32Cb)
{
    imgif_ctx_t *ctxP;
    ctxP = getCtx();
    memset(ctxP, 0, sizeof(imgif_ctx_t));

    /* Assign callbacks */
    ctxP->fmtParserCb = fmtParserCb;
    ctxP->calCrc32Cb = calCrc32Cb;

    /* Initialize context */
    if( init_ctx(ctxP) == -1 )
        return NULL;

    return((imgif_ctx_t *)ctxP);
}

/*****************************************************************************
*  FUNCTION:  imgif_pkgtb_write
*  PURPOSE:   Write image block to IMGIF.
*  PARAMETERS:
*      h (IN) - IMGIF context pointer.
*      dataP (IN) - image block data pointer.
*      len (IN) - image block size.
*  RETURNS:
*      >=0 - number of bytes written.
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
int imgif_pkgtb_write(IMGIF_HANDLE h, UINT8 *dataP, int len)
{
    imgif_ctx_t *ctxP = getCtxByHandle(h);
    return(write_bytes_to_pkgtb_device( ctxP, (char*)dataP, len));
}

/*****************************************************************************
*  FUNCTION:  imgif_pkgtb_close
*  PURPOSE:   Close IMGIF context.
*  PARAMETERS:
*      h (IN) - IMGIF context pointer.
*      abortFlag (IN) - TRUE: user aborts the operation.
*  RETURNS:
*      0 - successful operation.
*      -1 - failed operation.
*  NOTES:
*      User may stop an upgrade at any time, when there is an operator,
*      command, protocol failure, or unmatched image size and/or CRC values
*      between IMGIF and user.
*****************************************************************************/
int imgif_pkgtb_close(IMGIF_HANDLE h, UBOOL8 abortFlag)
{
    imgif_ctx_t *ctxP = getCtxByHandle(h);
    DUMPIMGIFCTX(ctxP);
    return(finalize_pkgtb_img_write(ctxP, abortFlag));
}

/*****************************************************************************
*  FUNCTION:  imgif_pkgtb_set_image_info
*  PURPOSE:   Set image info obtained externally.
*  PARAMETERS:
       h (IN) - IMGIF context pointer.
       imgInfoExtP (OUT) - image info pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      This function may be invoked at any time. However, it is typically
*      invoked at the end of the upgrade, when user has received the whole
*      image, and has obtained the image integrity information.
*****************************************************************************/
int imgif_pkgtb_set_image_info(IMGIF_HANDLE h, imgif_img_info_t *imgInfoExtP)
{
    imgif_ctx_t *ctxP = getCtxByHandle(h);

    if ((ctxP != NULL) && (imgInfoExtP != NULL))
    {
#if defined(CUSTOM_NAND_SINGLE_IMAGE) || defined(RTCONFIG_SINGLEIMG_B)
        ctxP->imgInfoExt.bitmask = (imgInfoExtP->bitmask != 0)?
            imgInfoExtP->bitmask: ctxP->imgInfoExt.bitmask;
        ctxP->imgInfoExt.size = (imgInfoExtP->size != 0)?
            imgInfoExtP->size: ctxP->imgInfoExt.size;
        ctxP->imgInfoExt.crc = (imgInfoExtP->crc != 0)?
            imgInfoExtP->crc: ctxP->imgInfoExt.crc;

        ctxP->force_upd_img_idx = (imgInfoExtP->force_upd_img_idx != 0)?
            imgInfoExtP->force_upd_img_idx: ctxP->imgInfoExt.force_upd_img_idx;

        imgif_debug("imgif_pkgtb_set_image_info bitmask %x %x !!",
            ctxP->imgInfoExt.bitmask, imgInfoExtP->bitmask);
        imgif_debug("size %x %x ", ctxP->imgInfoExt.size, imgInfoExtP->size);
        imgif_debug("crc %x %x ", ctxP->imgInfoExt.crc, imgInfoExtP->crc);
        imgif_debug("force_upd_img_idx %x %x !!\n",
            ctxP->imgInfoExt.force_upd_img_idx, imgInfoExtP->force_upd_img_idx);

        if (ctxP->force_upd_img_idx) {
            ctxP->update_img_idx = ctxP->force_upd_img_idx;
            imgif_debug("Override upd img index to %d !!\n", ctxP->update_img_idx);
        }
#else
        ctxP->imgInfoExt.bitmask = imgInfoExtP->bitmask;
        ctxP->imgInfoExt.size = imgInfoExtP->size;
        ctxP->imgInfoExt.crc = imgInfoExtP->crc;
#endif
        return 0;
    }

    return -1;
}

/*****************************************************************************
*  FUNCTION:  imgif_pkgtb_get_flash_info
*  PURPOSE:   Obtain flash info.
*  PARAMETERS:
       h (IN) - IMGIF context pointer.
       flashInfoP (OUT) - flash info pointer.
*  RETURNS:
*      0 - successful operation.
*      < 0 - failed operation.
*  NOTES:
*      This function may be invoked at any time.
*****************************************************************************/
int imgif_pkgtb_get_flash_info(imgif_flash_info_t *flashInfoP)
{
    int ret = -1;
    mtd_info_t *mtdP;
    int mtdFd;
    unsigned int flags;
    UINT32 flashSize=0;

    imgif_debug("Enter, info=%p.", flashInfoP);
    ret = getFlashInfo(&flags);

    if( ret == 0 )
    {
        flashInfoP->flashType = flags;
        if( flags == FLASH_INFO_FLAG_NAND )
        {
            mtdP = get_mtd_device_handle(PKGTB_MTD_IMG_DEV_NAME, &mtdFd, 0);
            if (mtdP == NULL)
            {
                imgif_error("get_mtd_device_handle(%s) failed", PKGTB_MTD_IMG_DEV_NAME);
                return -1;
            }
            else
            {
                flashInfoP->eraseSize = mtdP->erasesize;
                flashInfoP->flashSize = mtdP->size;
                free(mtdP);
                close(mtdFd);
            }
        }
        else if ( flags == FLASH_INFO_FLAG_EMMC )
        {
            flashInfoP->eraseSize = 0;

            if(isLegacyFlashLayout())
            {
                ret = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ,
                                        FLASH_SIZE,
                                        0, 0, 0, &flashSize);
                if( ret == 0 )
                    flashInfoP->flashSize = flashSize;
            }
            else
            {
#ifndef DESKTOP_LINUX
               flashInfoP->flashSize = emmcGetAvailLoaderSpace() + 2 * emmcGetAvailImgSpace(1);
#else
               flashInfoP->flashSize = 0x1000000;
#endif
            }
        }
        else if ( flags == FLASH_INFO_FLAG_NOR )
        {
            mtdP = get_mtd_device_handle(SPI_NOR_FLASH_NAME, &mtdFd, 0);
            if (mtdP == NULL)
            {
                imgif_error("get_mtd_device_handle(%s) failed", SPI_NOR_FLASH_NAME);
                return -1;
            }
            else
            {
                flashInfoP->eraseSize = mtdP->erasesize;
                flashInfoP->flashSize = mtdP->size;
                free(mtdP);
                close(mtdFd);
            }
        }
        else
        {
            imgif_error("Unsupported Flash Device!!\n");
            return -1;
        }
    }
    return(0);
}
