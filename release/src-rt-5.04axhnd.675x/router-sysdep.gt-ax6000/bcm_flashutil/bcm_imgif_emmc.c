/*****************************************************************************
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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
#include "bcm_flashutil_emmc.h"
#include "bcm_imgif.h"
#include "bcm_imgif_emmc.h"
#include "bcmTag.h"
#include "bcm_hwdefs.h"
#include "bcm_boarddriverctl.h"
#include "emmc_linux_defs.h"
#include "genutil_crc.h"

/* ---- Constants and Types ----------------------------------------------- */
#define IMG_WRITE_FS    1  /* Keep enabled:  Enable filesystem writing */
#define IMG_WRITE_NVRAM 0  /* Keep disabled: Image update does NOT modify NVRAM */
#define CC_IMGIF_DEBUG  0

/* Must assume an order to the parts for this to work
 * TAG, cferom, roots, bootfs, metadata */
typedef enum
{
    IMGIF_EMMC_IMG_SEGMENT_TAG = 0,
    IMGIF_EMMC_IMG_SEGMENT_CFEROM,    
    IMGIF_EMMC_IMG_SEGMENT_ROOTFS,
    IMGIF_EMMC_IMG_SEGMENT_BOOTFS,
    IMGIF_EMMC_IMG_SEGMENT_MDATA,
    IMGIF_EMMC_IMG_SEGMENT_MAX
} IMGIF_EMMC_IMG_SEGMENTS;

typedef struct
{
    unsigned int flash_offset;
    unsigned int size;
    unsigned int bytesdone;
} IMGIF_EMMC_IMG_SEGMENT_INFO;

/* IMGIF emmc context. */
typedef struct
{
#if IMG_WRITE_NVRAM 
    FILE * nvram_fh;
#endif    
    FILE * cferom_fh;
    FILE * rootfs_fh;
    FILE * bootfs_fh;
    FILE * mdata_primary_fh;
    FILE * mdata_backup_fh;
    IMGIF_EMMC_IMG_SEGMENT_INFO  emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MAX];
    IMGIF_EMMC_IMG_SEGMENTS current_img_segment;
    unsigned int crc_body_exp;
    unsigned int crc_body_calc;
    unsigned int crc_image_calc;
    unsigned int image_size_calc;
    char * mdata_ptr;
    char * cferom_ptr;
    int bootp_seqnum; 
    int updatep_seqnum;
    int booted_partition;
    int update_partition;
    FILE_TAG tag;

    imgif_img_info_t imgInfoExt;

    IMG_FORMAT_PARSER_CB fmtParserCb;
    CAL_CRC32_CB calCrc32Cb;
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
/* ---- Functions --------------------------------------------------------- */

static void print_imgupdate_progress( imgif_ctx_t * ctxP)
{
#if CC_IMGIF_DEBUG
     int i = 0;
     int size = 0;
     int bytes_done = 0;
   
     for( i=0; i<IMGIF_EMMC_IMG_SEGMENT_MAX; i++ )
     {
         size += ctxP->emmc_img_segments[i].size;
         bytes_done += ctxP->emmc_img_segments[i].bytesdone;
     }
     printf("\rImage Update Progress: %d/%d bytes\n", bytes_done, size);
#else
    fprintf(stderr, ".");
#endif    
}

/*****************************************************************************
*  FUNCTION:  process_image_tag
*  PURPOSE:   process a complete broadcom image tag
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*      pTag (IN) - Broadcom image tag.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      This function does the bulk of resource allocation for emmc img writes.
*****************************************************************************/
static int process_image_tag( imgif_ctx_t *ctxP, PFILE_TAG pTag )
{
    int ret = -1;
    char mdata_dev_name_rootfs[] = EMMC_DEV_PNAME_FMT_STR_ROOTFS;
    char mdata_dev_name_bootfs[] = EMMC_DEV_PNAME_FMT_STR_BOOTFS;
    char mdata_dev_name_primary[] = EMMC_DEV_PNAME_FMT_STR_MDATA;
    char mdata_dev_name_backup[] = EMMC_DEV_PNAME_FMT_STR_MDATA;
    UINT32 * crc_ptr = (UINT32*)(pTag->imageValidationToken);

    /* Dump image tag */
    imgif_debug("--------------------Broadcom image tag------------------------\n");
    imgif_debug("tagVersion: %02x %02x %02x %02x\n",
      pTag->tagVersion[0], pTag->tagVersion[1],
      pTag->tagVersion[2], pTag->tagVersion[3]);
    imgif_debug("endian: %s\n", pTag->bigEndian);
    imgif_debug("totalImageLen: %s\n", pTag->totalImageLen);
    imgif_debug("cfeAddress: %s\n", pTag->cfeAddress);
    imgif_debug("cfeLen: %s\n", pTag->cfeLen);
    imgif_debug("rootfsAddress: %s\n", pTag->rootfsAddress);
    imgif_debug("rootfsLen: %s\n", pTag->rootfsLen);
    imgif_debug("kernelAddress: %s\n", pTag->bootfsAddress);
    imgif_debug("kernelLen: %s\n", pTag->bootfsLen);
    imgif_debug("mdataAddress: %s\n", pTag->mdataAddress);
    imgif_debug("mdataLen: %s\n", pTag->mdataLen);
    imgif_debug("imageSequence: %s\n", pTag->imageSequence);
    imgif_debug("imageVersion: %s\n", pTag->imageVersion);
    imgif_debug("imageFlags: 0x%08x\n", pTag->imageFlags);
    imgif_debug("imageCrc: 0x%08x\n", *crc_ptr);
    imgif_debug("--------------------------------------------------------------\n");

    if( atoi(pTag->cfeLen) )
    {
        if( emmcVerifyImageDDRType(pTag->imageFlags) )
            return ret;
    }

    /* Initialize emmc image segment data */
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_CFEROM].size           = atoi(pTag->cfeLen);
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_CFEROM].flash_offset   = atoi(pTag->cfeAddress);
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_ROOTFS].size           = atoi(pTag->rootfsLen);        
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_ROOTFS].flash_offset   = atoi(pTag->rootfsAddress);
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_BOOTFS].size           = atoi(pTag->bootfsLen);        
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_BOOTFS].flash_offset   = atoi(pTag->bootfsAddress);
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MDATA].size         = atoi(pTag->mdataLen);        
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MDATA].flash_offset = atoi(pTag->mdataAddress);

    /* Store expected image content crc */
    ctxP->crc_body_exp = *crc_ptr;

    /* Allocated memory for temp storage */
    ctxP->mdata_ptr = malloc(atoi(pTag->mdataLen));
    if( atoi(pTag->cfeLen))
        ctxP->cferom_ptr = malloc(atoi(pTag->cfeLen));
    if( !ctxP->mdata_ptr 
        || (atoi(pTag->cfeLen) && !ctxP->cferom_ptr) )
    {
        printf("%s: Error allocating memory for temp storage!\n", __FUNCTION__);
        return ret;
    }
    /* Generate full pathnames for bootfs and rootfs devs */
    sprintf(mdata_dev_name_rootfs, EMMC_DEV_PNAME_FMT_STR_ROOTFS, ctxP->update_partition);
    sprintf(mdata_dev_name_bootfs, EMMC_DEV_PNAME_FMT_STR_BOOTFS, ctxP->update_partition);

    /* Generate full pathnames for metadata devs */
    sprintf(mdata_dev_name_primary, EMMC_DEV_PNAME_FMT_STR_MDATA, ctxP->update_partition,
            EMMC_IMG_IDX_START);
    sprintf(mdata_dev_name_backup, EMMC_DEV_PNAME_FMT_STR_MDATA, ctxP->update_partition, 
            EMMC_NUM_MDATA);

#if IMG_WRITE_FS
    /* Open cferom and nvram devices */
    if( atoi(pTag->cfeLen) )
    {
#if IMG_WRITE_NVRAM 
        ctxP->nvram_fh      = fopen(EMMC_DEV_PNAME_NVRAM, "r+");
        imgif_debug("Opening file %s for writing\n", EMMC_DEV_PNAME_NVRAM);
#endif        
        ctxP->cferom_fh     = fopen(EMMC_DEV_PNAME_CFE, "r+");
        imgif_debug("Opening file %s for writing\n", EMMC_DEV_PNAME_CFE);
    }

    /* Open rootfs, bootfs, and metadata devices */
    imgif_debug("Opening file %s for writing\n", mdata_dev_name_rootfs);
    imgif_debug("Opening file %s for writing\n", mdata_dev_name_bootfs);
    imgif_debug("Opening file %s for writing\n", mdata_dev_name_primary);
    imgif_debug("Opening file %s for writing\n", mdata_dev_name_backup);
    ctxP->rootfs_fh         = fopen(mdata_dev_name_rootfs, "r+");
    ctxP->bootfs_fh         = fopen(mdata_dev_name_bootfs, "r+");
    ctxP->mdata_primary_fh  = fopen(mdata_dev_name_primary, "r+");
    ctxP->mdata_backup_fh   = fopen(mdata_dev_name_backup, "r+");

    if(     (atoi(pTag->cfeLen) && !ctxP->cferom_fh)
#if IMG_WRITE_NVRAM
         || (atoi(pTag->cfeLen) && !ctxP->nvram_fh)
#endif         
         || !ctxP->rootfs_fh 
         || !ctxP->bootfs_fh
         || !ctxP->mdata_primary_fh 
         || !ctxP->mdata_backup_fh )
    {
        printf("%s: Failed to open emmc devices!\n", __FUNCTION__);
        ret = -1;
    }
    else
        ret = 0;
#else
    ret = 0;    
#endif /* IMG_WRITE_FS */

    return ret;
}

/*****************************************************************************
*  FUNCTION:  init_ctx
*  PURPOSE:   initialize emmc img write parameters
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int init_ctx(imgif_ctx_t *ctxP)
{
    int ret = -1;

    /* Initialize image segment values */
    ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_TAG].size = TAG_LEN;
    ctxP->current_img_segment = IMGIF_EMMC_IMG_SEGMENT_TAG;

    /* Initialize whole image crc */
    if (ctxP->calCrc32Cb != NULL)
        ctxP->calCrc32Cb(0, &ctxP->crc_image_calc, NULL, 0);

    /* Initialize image contents crc */
    genUtl_getCrc32Staged(0, &ctxP->crc_body_calc, NULL, 0);

    /* Initialize image size */
    ctxP->image_size_calc = 0;

    /* Get booted and update partition numbers */
    ret = emmcGetBootPartition();
    if( ret == -1 )
    {
        printf("%s: Error while retrieving boot partition number!\n", __FUNCTION__);
        return ret;
    }
    else
    {
        ctxP->booted_partition = ret;
        ctxP->update_partition = 
            (ctxP->booted_partition==EMMC_IMG_IDX_START)?EMMC_NUM_IMGS:EMMC_IMG_IDX_START;
    }

    /* Get sequence numbers */
    ctxP->bootp_seqnum = getSequenceNumber(ctxP->booted_partition);
    ctxP->updatep_seqnum = getSequenceNumber(ctxP->update_partition);
    return 0;
}

/*****************************************************************************
*  FUNCTION:  deinit_ctx
*  PURPOSE:   free all emmc image write related resources
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
#if IMG_WRITE_NVRAM
    if(ctxP->nvram_fh)
        fclose(ctxP->nvram_fh);    
#endif  
    if(ctxP->cferom_fh)      
        fclose(ctxP->cferom_fh);   
    fclose(ctxP->rootfs_fh);       
    fclose(ctxP->bootfs_fh);       
    fclose(ctxP->mdata_primary_fh);
    fclose(ctxP->mdata_backup_fh);
#endif    

    imgif_debug("Freeing all allocated memory\n"); 

    /* Free allocated memory */
    if( ctxP->cferom_ptr )
        free(ctxP->cferom_ptr);

    if( ctxP->mdata_ptr )
        free(ctxP->mdata_ptr);

    return 0;
}

/*****************************************************************************
*  FUNCTION:  write_bytes_to_emmc_device
*  PURPOSE:   write bytes to emmc devices
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*      data (IN) - data pointer
*      num_bytes (IN) - number of bytes to write
*  RETURNS:
*      >0 - number of bytes written
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int write_bytes_to_emmc_device( imgif_ctx_t *ctxP, char * data, unsigned int num_bytes )
{
    int bytes_left, bytes_to_write;
    int written_bytes = 0;
    char * datap = data;
    char * dest_ptr = NULL;
    FILE * dest_fh = NULL;

    imgif_debug("num_bytes:%d\n", num_bytes);

    while( num_bytes )
    {
        /* If all image data segments have have been written, but we still need to write more bytes
         * , this means that image may be corrupted (may possibly happen if there is extra padding added
         *  to end of file). In this case we dont write the extra bytes and simply return with a invalid crc
         */
        if( ctxP->current_img_segment == IMGIF_EMMC_IMG_SEGMENT_MAX )
        {
            /* Clear CRC calc to indicate failiure */
            ctxP->crc_body_calc = 0;
            printf("%s: Error: Attempt to write %d unknown img data bytes to invalid segment %d!\n", 
                    __FUNCTION__, num_bytes, ctxP->current_img_segment);
            return -1;
        }

        /* Start processing writes */
        imgif_debug("Segment:%d, segment size:%d, bytes_done:%d, currwrite:%d\n", 
            ctxP->current_img_segment,
            ctxP->emmc_img_segments[ctxP->current_img_segment].size, 
            ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone,
            num_bytes);

        /* Calculate how many bytes left to copy for segment */
        bytes_left = ctxP->emmc_img_segments[ctxP->current_img_segment].size 
                - ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone;

        /* Calculate how many bytes we can actually write */
        bytes_to_write = (bytes_left <= num_bytes)? bytes_left:num_bytes;

        /* Figure out whether we need to cache the segment data in memory or write it to fs */
        switch(ctxP->current_img_segment)
        {
            case IMGIF_EMMC_IMG_SEGMENT_TAG:
                /* Copy to local buffer */
                dest_ptr = (char*)&ctxP->tag;
            break;

            case IMGIF_EMMC_IMG_SEGMENT_CFEROM:
                /* write to local cferom */
                dest_ptr = ctxP->cferom_ptr;
            break;

            case IMGIF_EMMC_IMG_SEGMENT_MDATA:
                /* write to local metadata */
                dest_ptr = ctxP->mdata_ptr;
            break;

            case IMGIF_EMMC_IMG_SEGMENT_ROOTFS:
                dest_fh = ctxP->rootfs_fh;
            break;

            case IMGIF_EMMC_IMG_SEGMENT_BOOTFS:
                dest_fh = ctxP->bootfs_fh;
            break;

            default:
                printf("%s: Invalid segment! %d\n", __FUNCTION__, ctxP->current_img_segment);
                return -1;
            break;
        }

        /* Write data to either local memory or to filesystem */
        if( dest_ptr )
        {
            imgif_debug("Writing %d bytes to ptr:0x%08x\n", bytes_to_write, 
                (unsigned int)(dest_ptr 
                + ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone));
            memcpy(dest_ptr + ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone,
                    datap, bytes_to_write); 
            dest_ptr = NULL;
        }
        else if( dest_fh )
        {
#if IMG_WRITE_FS                
            imgif_debug("Writing %d bytes to file:0x%08x\n", bytes_to_write, 
                (unsigned int)(dest_fh));
            fwrite(datap, 1, bytes_to_write, dest_fh);
            if( ferror(dest_fh) || feof(dest_fh) )
            {
                printf("%s: Error writing segment %d\n", 
                        __FUNCTION__,
                        ctxP->current_img_segment);
                return -1;
            }
            dest_fh = NULL;
#endif                
        }

        /* Do whole image crc on all data */
        if (ctxP->calCrc32Cb != NULL)
                ctxP->calCrc32Cb(1, &ctxP->crc_image_calc, (unsigned char*)datap, bytes_to_write);

        /* Do content crc on non-tag data */
        if( ctxP->current_img_segment != IMGIF_EMMC_IMG_SEGMENT_TAG )
                genUtl_getCrc32Staged(1, &ctxP->crc_body_calc, (unsigned char*)datap, bytes_to_write);

        imgif_debug("crc img  tally:0x%08x\n", ctxP->crc_image_calc);
        imgif_debug("crc body tally:0x%08x\n", ctxP->crc_body_calc);

        /* Increment bytes done, advance data pointer and adjust remaining bytes */
        ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone += bytes_to_write;
        datap += bytes_to_write;
        num_bytes -= bytes_to_write;
        written_bytes += bytes_to_write;
        ctxP->image_size_calc += bytes_to_write;

        imgif_debug("written_bytes:%d\n", written_bytes);
        imgif_debug("bytes left: current_write:%d segment:%d\n", num_bytes, 
            ctxP->emmc_img_segments[ctxP->current_img_segment].size -
            ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone);

        /* Move to next segment if current segment is done */
        if( ctxP->emmc_img_segments[ctxP->current_img_segment].bytesdone == 
                ctxP->emmc_img_segments[ctxP->current_img_segment].size )
        {
            /* If we have the full image tag, process its contents */
            if( ctxP->current_img_segment == IMGIF_EMMC_IMG_SEGMENT_TAG )
            {
                if( process_image_tag(ctxP, &ctxP->tag) == -1 )
                    return -1;
            }

            /* Increment to next valid segment */
            while( ctxP->current_img_segment < IMGIF_EMMC_IMG_SEGMENT_MAX )
            {
                ctxP->current_img_segment++;
                imgif_debug("segment change:%d->%d\n", ctxP->current_img_segment-1, ctxP->current_img_segment);

                /* Break out if valid segment is detected */
                if( (ctxP->current_img_segment < IMGIF_EMMC_IMG_SEGMENT_MAX) &&
                    ctxP->emmc_img_segments[ctxP->current_img_segment].size )
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
*  PURPOSE:   dump all emmc img write related parameters.
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*  RETURNS:
*  NOTES:
*      None.
*****************************************************************************/
static void dump_ctx(imgif_ctx_t *ctxP)
{
    int i = 0;
    imgif_debug("------------------------imgif_emmc ctx------------------------------\n");
    for( i=0; i<IMGIF_EMMC_IMG_SEGMENT_MAX ; i++ )
    {
        imgif_debug("Img segment %d, fl_offset %08d, size %08d, bytesdone %08d\n", i,
        ctxP->emmc_img_segments[i].flash_offset,
        ctxP->emmc_img_segments[i].size,
        ctxP->emmc_img_segments[i].bytesdone);
    }
    imgif_debug("crc_image_calc 0x%08x crc_image_exp 0x%08x\n", ctxP->crc_image_calc, ctxP->imgInfoExt.crc);
    imgif_debug("crc_body_calc 0x%08x crc_body_exp 0x%08x\n", ctxP->crc_body_calc, ctxP->crc_body_exp);
    imgif_debug("boot_partition          : %d\n", ctxP->booted_partition);
    imgif_debug("update_partition        : %d\n", ctxP->update_partition);
    imgif_debug("boot_partition_seq      : %d\n", ctxP->bootp_seqnum); 
    imgif_debug("update_partition_seq    : %d\n", ctxP->updatep_seqnum);
    imgif_debug("--------------------------------------------------------------------\n");
}
#endif

#if IMG_WRITE_NVRAM 
/*****************************************************************************
*  FUNCTION:  flash_nvram_data
*  PURPOSE:   Write data to emmc nvram device
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      We dont update the nvram during webgui image updates. Keep this function
*      disabled
*****************************************************************************/
static int flash_nvram_data( imgif_ctx_t *ctxP )
{
    int ret = -1
    NVRAM_DATA * nvram_ptr;
    
    /* Handle NVRAM update */
    if( ctxP->cferom_ptr )
    {
        nvram_ptr = (NVRAM_DATA *)(ctxP->cferom_ptr + emmcGetNvramOffset((char*)(ctxP->cferom_ptr)));
        
#if IMG_WRITE_FS        
        fwrite((char*)nvram_ptr, 1, 
                        sizeof(NVRAM_DATA),
                        ctxP->nvram_fh);
        
        if( ferror(ctxP->nvram_fh) 
           || feof(ctxP->nvram_fh) )
        {
            printf("%s: Error writing NVRAM\n",
                    __FUNCTION__);
        }
        else
            ret = 0;
#endif            
    }
    else
    {
        printf("%s: Error writing NVRAM, invalid cferom_ptr\n",
                __FUNCTION__);
    }

    return ret;
}
#endif /* IMG_WRITE_NVRAM */

/*****************************************************************************
*  FUNCTION:  finalize_emmc_img_write
*  PURPOSE:   Finalize all write operations.
*  PARAMETERS:
*      ctxP (IN) - IMGIF context pointer.
*      abortFlag (IN) - flag indicating early abort.
*  RETURNS:
*      -1 - failed operation.
*  NOTES:
*      None.
*****************************************************************************/
static int finalize_emmc_img_write(imgif_ctx_t *ctxP, UBOOL8 abortFlag)
{
    int ret = -1;

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
    if ((ctxP->imgInfoExt.bitmask & IMG_INFO_BITMASK_SIZE) && (ctxP->imgInfoExt.size != ctxP->image_size_calc))
    {
        printf("Unmatched file size values, ext=0x%08x, cal=0x%08x",
          ctxP->imgInfoExt.size, ctxP->image_size_calc);
        goto cleanup;;
    }

    /* Check if image contents CRC matched expected value */
    if( ctxP->crc_body_calc == ctxP->crc_body_exp )
    {
#if IMG_WRITE_FS        
        unsigned int seq_num;

        /* Write cferom to partition */
        if ( ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_CFEROM].bytesdone )
        {
            /* Enable write access to emmc physical boot partition */
            imgif_debug("cfe partition unlock command: %s\n", EMMC_BOOT_PARTITION_UNLOCK_CMD);
            system(EMMC_BOOT_PARTITION_UNLOCK_CMD);

            /* Seek to specified offset in partition and write cferom data */
            imgif_debug("Setting cferom file pointer to offset 0x%08x\n", IMAGE_OFFSET);
            if(fseek(ctxP->cferom_fh, IMAGE_OFFSET, SEEK_SET) != 0)
            {
                printf("%s: Error during cferom fseek\n", __FUNCTION__);
                goto cleanup;
            }

            /* Write cferom data */
            imgif_debug("Writing %d bytes of cferom to file:0x%08x\n", 
                ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_CFEROM].bytesdone, 
                (unsigned int)(ctxP->cferom_fh));
            fwrite(ctxP->cferom_ptr, 1, 
                            ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_CFEROM].bytesdone,
                            ctxP->cferom_fh);

            if( ferror(ctxP->cferom_fh) 
               || feof(ctxP->cferom_fh) )
            {
                printf("%s: Error writing segment %d\n",
                        __FUNCTION__,
                        IMGIF_EMMC_IMG_SEGMENT_CFEROM);
                goto cleanup;
            }

#if IMG_WRITE_NVRAM 
            if( flash_nvram_data( ctxP ) == -1 )
                goto cleanup;
#endif            
        }

        /* Calculate new sequence number */
        imgif_debug("Old sequence numbers %d %d\n", ctxP->bootp_seqnum, ctxP->updatep_seqnum); 
        seq_num = getNextSequenceNumber(ctxP->bootp_seqnum, ctxP->updatep_seqnum);

        /* Update inmemory copy of mdata with new sequence number */
        imgif_debug("Updating to new sequence number %d\n", seq_num); 
        if( emmcUpdateMdataSeqnum(ctxP->mdata_ptr, 0, seq_num) == -1 )
        {
            printf("%s: Error updating metadata squence number\n", __FUNCTION__);
            goto cleanup;
        }

        /* Update inmemory copy of mdata with valid commit flag */
        imgif_debug("Updating commit flag\n"); 
        if( emmcUpdateMdataCommitFlag(ctxP->mdata_ptr, 0, 1) == -1 )
        {
            printf("%s: Error updating metadata commit flag\n", __FUNCTION__);
            goto cleanup;
        }

        /* Write primary metadata to partition */
        imgif_debug("Writing %d bytes of primary mdata to file:0x%08x\n", 
            ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MDATA].bytesdone, 
            (unsigned int)(ctxP->mdata_primary_fh));
        fwrite(ctxP->mdata_ptr, 1, 
                        ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MDATA].bytesdone,
                        ctxP->mdata_primary_fh);

        /* Write backup metadata to partition */
        imgif_debug("Writing %d bytes of backup mdata to file:0x%08x\n", 
            ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MDATA].bytesdone, 
            (unsigned int)(ctxP->mdata_backup_fh));
        fwrite(ctxP->mdata_ptr, 1, 
                        ctxP->emmc_img_segments[IMGIF_EMMC_IMG_SEGMENT_MDATA].bytesdone,
                        ctxP->mdata_backup_fh);

        if(  ferror(ctxP->mdata_primary_fh) 
           ||  feof(ctxP->mdata_primary_fh) 
           ||ferror(ctxP->mdata_backup_fh) 
           ||  feof(ctxP->mdata_backup_fh) )
        {
            printf("%s: Error writing segment %d\n",
                    __FUNCTION__,
                    IMGIF_EMMC_IMG_SEGMENT_MDATA);
            goto cleanup;
        }
#endif        
        ret = 0;
    }
    else
    {
        printf("%s: Image contents CRC check failed! expected:0x%08x, calculated:0x%08x\n",
                __FUNCTION__,
                ctxP->crc_body_exp,
                ctxP->crc_body_calc);

        ret = -1;
    }

cleanup:   
    /* deinitialize context */
    deinit_ctx(ctxP);

    return ret;
}

/*****************************************************************************
*  FUNCTION:  imgif_emmc_open
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
IMGIF_HANDLE imgif_emmc_open(IMG_FORMAT_PARSER_CB fmtParserCb,
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

    return((IMGIF_HANDLE)ctxP);
}

/*****************************************************************************
*  FUNCTION:  imgif_emmc_write
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
int imgif_emmc_write(IMGIF_HANDLE h, UINT8 *dataP, int len)
{
    imgif_ctx_t *ctxP = getCtxByHandle(h);
    return(write_bytes_to_emmc_device( ctxP, (char*)dataP, len));
}

/*****************************************************************************
*  FUNCTION:  imgif_emmc_close
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
int imgif_emmc_close(IMGIF_HANDLE h, UBOOL8 abortFlag)
{
    imgif_ctx_t *ctxP = getCtxByHandle(h);
    DUMPIMGIFCTX(ctxP);
    return(finalize_emmc_img_write(ctxP, abortFlag));
}

/*****************************************************************************
*  FUNCTION:  imgif_emmc_set_image_info
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
int imgif_emmc_set_image_info(IMGIF_HANDLE h, imgif_img_info_t *imgInfoExtP)
{
    imgif_ctx_t *ctxP = getCtxByHandle(h);

    if ((ctxP != NULL) && (imgInfoExtP != NULL))
    {
        ctxP->imgInfoExt.bitmask = imgInfoExtP->bitmask;
        ctxP->imgInfoExt.size = imgInfoExtP->size;
        ctxP->imgInfoExt.crc = imgInfoExtP->crc;
        return 0;
    }

    return -1;
}

/*****************************************************************************
*  FUNCTION:  imgif_emmc_get_flash_info
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
int imgif_emmc_get_flash_info(imgif_flash_info_t *flashInfoP)
{
    int ret = -1;
    unsigned int flags;
    UINT32 flashSize=0;
    ret = getFlashInfo(&flags);
    if( ret == 0 )
    {
        flashInfoP->flashType = flags;
        flashInfoP->eraseSize = 0;
        ret = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ,
                                FLASH_SIZE,
                                0, 0, 0, &flashSize);
        if( ret == 0 )
            flashInfoP->flashSize = flashSize;
    }

    return(0);
}
