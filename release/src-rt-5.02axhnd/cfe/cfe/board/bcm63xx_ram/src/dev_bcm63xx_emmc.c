/***************************************************************************
 <:copyright-BRCM:2016:DUAL/GPL:standard
 
    Copyright (c) 2016 Broadcom 
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
 ***************************************************************************/

#if (INC_EMMC_FLASH_DRIVER==1)
#include "lib_types.h"
#include "lib_printf.h"
#include "cfe_timer.h"
#include "cfe.h"
#include "bcm_map.h"
#include "bcm63xx_auth.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "dev_bcm63xx_flash.h"
#include "bcm63xx_util.h"
#include "flash_api.h"
#include "exception.h"
#include "shared_utils.h"
#include "btrm_if.h"
#include "dev_emmcflash.h"
#include "dev_bcm63xx_emmc_common.h"
#include "cfe_gpt_common.h"
#include "bcm63xx_blparms.h"
#include "btrm_if.h"
#include "lib_byteorder.h"
#include "rom_parms.h"
#include "bcm_otp.h"
#include "addrspace.h"
#include "lib_string.h"
#include "lib_malloc.h"
#include "bcm63xx_nvram.h"
#include "bcm_btrm_common.h"

/* Defines and structs */
#if defined(CONFIG_ARM64)
#define LA_DEFAULT_FLAGS    LOADFLG_64BITIMG
#else
#define LA_DEFAULT_FLAGS    0x0
#endif

#define KERNEL_PARTITION_SIZE          (10*1024*1024)
#define CFE_PARTITION_SIZE_EMMC        (1*1024*1024)
#define PARTITION_SIZE_FILL_FLASH      0

#define DEV_EMMC_DBG_DISABLE_EMMCBOOT_RW    0   /* Disables all reads/writes to eMMC device during bootup */
#define DEV_EMMC_DBG_DISABLE_GPT            0   /* Disables creating/reading of GPT tables */

/* Extern Variables */
extern cfe_driver_t emmcflashdrv;
extern queue_t cfe_devices;

/* Local Variables */
#if DEBUG_CRC
static unsigned long long byteSum = 0;
static unsigned long long numByteSum1 = 0;
#endif

static char bootCfeVersion[CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE] = {0}; 
static char root_device_name[EMMC_MAX_PART_NAME];

const static emmcflash_logicalpart_spec_t EmmcDefaultDataPartition [] = {
    {CFE_GPT_PRIMRY_SIZE,       PRIMARY_GPT_HDR_PART_NAME,  EMMC_PART_DATA, 0},  /* 32K GPT Primary */
    {EMMC_DFLT_NVRAM_SIZE,      EMMC_PNAME_STR_NVRAM,       EMMC_PART_DATA, 0},    
    {EMMC_DFLT_BOOTFS_SIZE,     EMMC_PNAME_STR_BOOTFS(1),   EMMC_PART_DATA, 0},    
    {EMMC_DFLT_ROOTFS_SIZE,     EMMC_PNAME_STR_ROOTFS(1),   EMMC_PART_DATA, 0},    
    {EMMC_DFLT_MDATA_SIZE,      EMMC_PNAME_STR_MDATA(1,1),  EMMC_PART_DATA, 0},    
    {EMMC_DFLT_MDATA_SIZE,      EMMC_PNAME_STR_MDATA(1,2),  EMMC_PART_DATA, 0},    
    {EMMC_DFLT_BOOTFS_SIZE,     EMMC_PNAME_STR_BOOTFS(2),   EMMC_PART_DATA, 0},    
    {EMMC_DFLT_ROOTFS_SIZE,     EMMC_PNAME_STR_ROOTFS(2),   EMMC_PART_DATA, 0},    
    {EMMC_DFLT_MDATA_SIZE,      EMMC_PNAME_STR_MDATA(2,1),  EMMC_PART_DATA, 0},    
    {EMMC_DFLT_MDATA_SIZE,      EMMC_PNAME_STR_MDATA(2,2),  EMMC_PART_DATA, 0},    
    {EMMC_DFLT_DATA_SIZE,       EMMC_PNAME_STR_DATA,  	    EMMC_PART_DATA, 0},    
    {PARTITION_SIZE_FILL_FLASH, EMMC_PNAME_STR_UNALLOC,     EMMC_PART_DATA, 0},    
    {CFE_GPT_BACKUP_SIZE,       BACKUP_GPT_HDR_PART_NAME,   EMMC_PART_DATA, 0},    /* 32K GPT Backup */
};

static emmcflash_logicalpart_spec_t EmmcBoot1Partition [] = {               
    { PARTITION_SIZE_FILL_FLASH,   "cfe",                   EMMC_PART_BOOT1, 0},   // CFE
};

static emmcflash_logicalpart_spec_t EmmcBoot2Partition [] = {               
    { PARTITION_SIZE_FILL_FLASH,   "cfe",                   EMMC_PART_BOOT2, 0},   // CFE
};

/* Global Variables */
static int boot_only_img_idx = 0;
static int allow_img_update_repartition = 0;

/* Extern function prototypes */
extern int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize);
#ifdef USE_LZ4_DECOMPRESSOR
extern int LZ4_decompress_fast (const char* source, char* dest, int originalSize);
#endif

/* Local funcs prototypes */
static int emmc_check_image_size( PFILE_TAG pTag);
static int enable_emmc_flash( uint64_t *emmc_flash_size, uint8_t boot_mode );
static int enable_emmc_gpt( cfe_gpt_probe_t *fgptprobe, emmcflash_probe_t *femmcprobe, int attr, uint8_t num_parts, int force );
static int parse_emmc_bootfs( char * flashdev, char * filename, char * data_buffer, int * data_length );
static int emmc_erase_phys_part(int emmcPhysPartAttr);
static int emmc_delete_cfe_devs(int emmcPhysPartAttr, emmcflash_probe_t *femmcprobe);
static int get_new_image_idx( void );
static int emmc_update_image_seq_num( uint8_t *imagePtr, int imageSize );
static int flash_imagedata_to_partition( char * part_name, uint8_t * pData, int offset, int length );
static int read_nvram_data( PNVRAM_DATA pNvramData );
static int read_cferom_version( char * version );
static int write_nvram_data( PNVRAM_DATA pNvramData );
static int get_nvram_offset( char * cfe_start_addr );
static int emmc_delete_cfe_devs(int emmcPhysPartAttr, emmcflash_probe_t *femmcprobe);
static int emmc_erase_phys_part(int emmcPhysPartAttr);
static uint64_t emmc_get_part_size(char * flashdev);
static int enable_emmc_flash( uint64_t *emmc_flash_size, uint8_t boot_mode );
static uint64_t enable_emmc_logicalpartition( emmcflash_logicalpart_spec_t *EmmcPartition, emmcflash_probe_t *femmcprobe, int attr, uint8_t num_parts );
static void ui_emmc_dumphex( unsigned char *pAddr, unsigned int offset, int nLen, int sz );
#if DEBUG_CRC
static unsigned int debugGetCrc32(unsigned char * dev_buffer, unsigned int remDataSize, unsigned int dataCrc);
#endif
static uint64_t enable_emmc_logicalpartition_from_gpt( cfe_gpt_probe_t * fgptprobe, emmcflash_probe_t * femmcprobe, int attr);

/***************************************************************************
 *                                                                         *
 *         Local functions                                                 *
 *                                                                         *
 ***************************************************************************/


static int read_cferom_version( char * version )
{
    int ret = 0;
    int fh = 0;
    int cfe_ver_offset = 0;
    uint64_t image_offset;
    char flash_dev[] = EMMC_CFE_PNAME_CFE;
#if !defined(_BCM963138_) 
    int i;
    uint64_t cferom_partition_size = emmc_get_part_size(EMMC_CFE_PNAME_CFE);
    char temp_buf[sizeof(SbiAuthHdrBeginning)+BTRM_SBI_UNAUTH_HDR_MAX_SIZE];
#endif    

    /* The cfe version string lies in the cfe rom, just above the start
     * of the embedded nvram data. Inorder to retrieve the version info
     * we need to find out where exactly the info lies after the optional
     * unauth/auth headers */
    ret = cfe_getdevinfo(flash_dev);
    if (ret < 0) 
    {
        printf("Device %s not found\n", flash_dev);
        return CFE_ERR_IOERR;
    }

    fh = cfe_open(flash_dev);

#if defined(_BCM963138_) 
    /* Find the offset to the cfe version string, for Gen2 images offset is fixed */
    cfe_ver_offset = get_nvram_offset(0) - CFE_VERSION_REL_OFFSET;
    image_offset = IMAGE_OFFSET;
#else
    /* Read full unauthhdr plus beginning of authhdr, search at every 1k boundary */
    for( i=0; i < cferom_partition_size/1024; i++ )
    {
        ret = cfe_readblk(fh,
                    i*1024,
                    (unsigned char*)temp_buf,
                    (sizeof(SbiAuthHdrBeginning)+BTRM_SBI_UNAUTH_HDR_MAX_SIZE));

        if (ret < 0)
            break;

        if (((SbiUnauthHdrBeginning*)temp_buf)->magic_1 == BTRM_SBI_UNAUTH_MGC_NUM_1)
        {
           if (((SbiUnauthHdrBeginning*)temp_buf)->magic_2 == BTRM_SBI_UNAUTH_MGC_NUM_2)
           {
               /* We found a cferom */
               image_offset = i*1024;
               break;
           }
        }
    }

    if (ret < 0)
    {
        printf("eMMC ioctl error\n");
        cfe_close(fh);
        return(CFE_ERR_IOERR);
    }
    
    if ( i==cferom_partition_size/1024 )
    {
        printf("eMMC could not find cferom\n");
        cfe_close(fh);
        return(CFE_ERR_IOERR);
    }

    /* Find the offset to the cfe version string */
    cfe_ver_offset = get_nvram_offset(temp_buf) - CFE_VERSION_REL_OFFSET;
#endif    

    /* Read the cfe version string */
    ret = cfe_readblk(fh,
                    cfe_ver_offset + image_offset,
                    (unsigned char*)version,
                    (CFE_VERSION_MARK_SIZE+CFE_VERSION_SIZE));

    cfe_close(fh);

    if (ret < 0)
    {
        printf("eMMC ioctl error\n");
        return(CFE_ERR_IOERR);
    }
    else
        ret = CFE_OK;
        
    printf("%s: %d.%d.%d-%d.%d\n",
    			version,
			version[CFE_VERSION_MARK_SIZE+0],
			version[CFE_VERSION_MARK_SIZE+1],
			version[CFE_VERSION_MARK_SIZE+2],
			version[CFE_VERSION_MARK_SIZE+3],
			version[CFE_VERSION_MARK_SIZE+4]);
    return ret;

}

static int read_nvram_data( PNVRAM_DATA pNvramData )
{
    int ret = 0;
    int fh = 0;
    char flash_dev[] = EMMC_CFE_PNAME_NVRAM;

    ret = cfe_getdevinfo(flash_dev);
    if (ret < 0) {
        printf("Device %s not found\n", flash_dev);
        return CFE_ERR_IOERR;
    }

    fh = cfe_open(flash_dev);

#if !DEV_EMMC_DBG_DISABLE_EMMCBOOT_RW
    ret = cfe_read(fh, (unsigned char*)pNvramData, sizeof(NVRAM_DATA));
#endif

    cfe_close(fh);

    if (ret < 0)
    {
        printf("eMMC ioctl error\n");
        ret = CFE_ERR_IOERR;
    }
    else
        ret = CFE_OK;
        
    return ret;

}

static int write_nvram_data( PNVRAM_DATA pNvramData )
{
    int ret = 0;
    int fh = 0;
    char flash_dev[] = EMMC_CFE_PNAME_NVRAM;

    ret = cfe_getdevinfo(flash_dev);
    if (ret < 0) {
        printf("Device %s not found\n", flash_dev);
        return CFE_ERR_IOERR;
    }

    fh = cfe_open(flash_dev);

#if !DEV_EMMC_DBG_DISABLE_EMMCBOOT_RW
    ret = cfe_write(fh, (unsigned char*)pNvramData, sizeof(NVRAM_DATA));
#endif

    cfe_close(fh);

    if (ret < 0)
    {
            printf("eMMC ioctl error\n");
            ret = CFE_ERR_IOERR;
    }
    else
        ret = CFE_OK;

    return ret;

}

static int get_nvram_offset( char * cfe_start_addr )
{
    int nvram_data_offset = 0;
#if defined(_BCM963138_) 
    /* Gen2 Image - fixed header sizes*/
    (void) cfe_start_addr;
    nvram_data_offset =  BTRM_SBI_SIGNED_COT_ELEM_SIZE * 2
                       + BTRM_SBI_AUTH_HDR_SIZE       
                       + BTRM_SBI_UNAUTH_HDR_SIZE;
#else
    /* Gen3 Image */
    SbiUnauthHdrBeginning *pUHdr;
    SbiAuthHdrBeginning   *pAuthHdr;
    uint32_t hdr_size_bytes = 0;

    /* Determine size of auth/unauth headers */
    pUHdr = (SbiUnauthHdrBeginning *)cfe_start_addr;
    hdr_size_bytes = pUHdr->hdrLen;
    pAuthHdr = (SbiAuthHdrBeginning *)(cfe_start_addr + hdr_size_bytes);
    hdr_size_bytes += pAuthHdr->hdrLen;
    
    /* Add offset to start of actual CFEROM image */
    nvram_data_offset += hdr_size_bytes;
#endif    

    /* Add offset to NVRAM data */
    nvram_data_offset += NVRAM_DATA_REL_OFFSET;

    return nvram_data_offset;
}


static int flash_imagedata_to_partition( char * part_name, uint8_t * pData, int offset, int length )
{
    int fh = 0; 
    int ret = 0;
    cfe_device_t *dev = NULL;

    /* Try to retrieve device structure */
    dev = cfe_finddev(part_name);
    if (!dev) {
        printf("Device %s not found!\n", part_name); 
        return -1;
    }
    
    /* Check partition sizes */
    if (length > ((emmcflash_cfepart_cfg_t *)dev->dev_softc)->fp_size)
    {
        printf("Imgdata size %d is greater than partition %s size %d!\n", length, part_name, 
            ((emmcflash_cfepart_cfg_t *)dev->dev_softc)->fp_size);
        return -1;
    }

    /* open device and flash data */
    fh = cfe_open(part_name);
    if (fh < 0) 
    {
        printf("Cannot open device %s, fh: %d!\n", part_name, fh);
        ret = -1;
    }
    else
    {
        printf("...Flashing %d bytes @ offset %d to %s\n", length, offset, part_name);
        ret = cfe_writeblk(fh, offset, pData, length);
        if( ret < 0 || ret != length )
        {
            printf("Failed to flash %d bytes to partition %s, ret:%d!\n", length, part_name, ret);
            ret = -1;
        }
        cfe_close(fh);
        ret = 0;
    }

    return ret;
}

static int emmc_update_image_seq_num( uint8_t *imagePtr, int imageSize )
{
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    char fname[] = NAND_CFE_RAM_NAME;
    int seq; 
    int seq2;
    int cm_flag_unused;
    int ret = 0;

    get_seqnum_from_emmc_mdata(1, &seq , &cm_flag_unused, &parse_emmc_bootfs); 
    get_seqnum_from_emmc_mdata(2, &seq2, &cm_flag_unused, &parse_emmc_bootfs);

    /* deal with wrap around case */
    if ((seq == 0 && seq2 == 999) || (seq == 999 && seq2 == 0))
        seq = 0;
    else
        seq = (seq >= seq2) ? seq : seq2;

    if( seq != -1 )
    {
        /* Increment the new highest sequence number. Add it to the CFE RAM
         * file name.
         */
        seq++;
        if (seq > 999)
            seq = 0;
    }
    else
    {
        /* No valid sequence number -> no valid images. Therefore
         * use default starting sequence number of 999
         */
        seq = 999;
    }

    /* Form sequence number ascii string */    
    char seqstr[3] = {(seq / 100) + '0', ((seq % 100) / 10) + '0', ((seq % 100) % 10) + '0'};

    /* Find and update sequence number and committed flag bootfs entries in the metadata*/
    /* Update committed flag */
    ret = update_inmemory_metadata_data(  ((char *)imagePtr + atoi(pTag->mdataAddress)),
                                            "committed", 
                                            (uint8_t *)"1", 
                                            1);
    if( ret == -1 )
    {
        printf("Failed to update 'committed' flag!\n");
        return ret;
    }
                                            
    /* Update sequence number */
    ret = update_inmemory_metadata_data(  ((char *)imagePtr + atoi(pTag->mdataAddress)),
                                            fname, 
                                            (uint8_t *)seqstr, 
                                            sizeof(seqstr));
    if( ret == -1 )
    {
        printf("Failed to update sequence number!\n");
        return ret;
    }

    return(ret);
}

static int get_new_image_idx( void )
{
    /* This will return which image current cfe ram is from 
     * i.e the current image number */
    int curr_image_idx = (int) CFE_RAM_ROM_PARMS_GET_ROOTFS;

    printf("Current image index: %d\n", curr_image_idx);
    printf("New     image index: %d\n", ((curr_image_idx==NP_ROOTFS_1)? NP_ROOTFS_2:NP_ROOTFS_1));

    /* return opposite of current image to specify new image index */
    return ( ((curr_image_idx==NP_ROOTFS_1)? NP_ROOTFS_2:NP_ROOTFS_1) );
}

static int emmc_delete_cfe_devs(int emmcPhysPartAttr, emmcflash_probe_t *femmcprobe)
{
    queue_t             *qb;
    cfe_device_t        *dev;
    emmcflash_cfepart_cfg_t     *part;            
    int probeCopied = 0;

    if( !femmcprobe )
    {
        printf("%s: ERROR femmcprobe is NULL!\n");
        return CFE_ERR;
    }

    for (qb = cfe_devices.q_prev; qb != &cfe_devices; qb = qb->q_prev)
    {
        dev = (cfe_device_t *)qb;
        if((strncmp(dev->dev_fullname, emmcflashdrv.drv_bootname, strlen(emmcflashdrv.drv_bootname)) == 0))
        {    
            part = dev->dev_softc;                
            if( part && part->fp_dev && part->fp_dev->fd_probe.flash_part_attr == emmcPhysPartAttr )
            {
                if( !probeCopied )
                {
                    /* preserve emmcprobe */
                    memcpy(femmcprobe, &part->fp_dev->fd_probe, sizeof(emmcflash_probe_t));
                    probeCopied = 1;
                }    

                /* remove cfe device */
                cfe_detach_dev((cfe_device_t *)qb);

                /* free part->fp_dev->fd_sectorbuffer and part->fp_dev */
                KFREE(part->fp_dev->fd_sectorbuffer);
                KFREE(part->fp_dev);
            }
        }
    }  
    return CFE_OK;
}


static int emmc_erase_phys_part(int emmcPhysPartAttr)
{
    queue_t             *qb;
    cfe_device_t        *dev;
    emmcflash_cfepart_cfg_t     *part;            

    for (qb = cfe_devices.q_prev; qb != &cfe_devices; qb = qb->q_prev)
    {
        dev = (cfe_device_t *)qb;
        if((strncmp(dev->dev_fullname, emmcflashdrv.drv_bootname, strlen(emmcflashdrv.drv_bootname)) == 0))
        {    
            part = dev->dev_softc;                
            if( part && part->fp_dev && part->fp_dev->fd_probe.flash_part_attr == emmcPhysPartAttr )
            {    
                 printf("Erasing %s\n", dev->dev_fullname);
                 emmc_erase_partition(dev->dev_fullname);
            }
        }
    }  
    return CFE_OK;
}


static int map_image_num_to_linux_rootfs_part(int image_num)
{
    /* TODO: Make this better. This will return the partition
     * number that will show up in linux device list i.e 
     * /dev/mmcblk0p<x>.
     * This assumes 4 logical partitions for each image i.e
     * (0) emmcflash0.nvram    -> /dev/mmcblk0p1
     * (1) emmcflash0.bootfs1  -> /dev/mmcblk0p2-+
     * (2) emmcflash0.rootfs1  -> /dev/mmcblk0p3 |__ Image 1
     * (3) emmcflash0.mdata1_1 -> /dev/mmcblk0p4 |
     * (4) emmcflash0.mdata1_2 -> /dev/mmcblk0p5-+
     * (5) emmcflash0.bootfs2  -> /dev/mmcblk0p6-+
     * (6) emmcflash0.rootfs2  -> /dev/mmcblk0p7 |__ Image 2
     * (7) emmcflash0.mdata2_1 -> /dev/mmcblk0p8 |
     * (8) emmcflash0.mdata2_2 -> /dev/mmcblk0p9-+
     * (9) emmcflash0.data     -> /dev/mmcblk0p10
     * in this specific order */
    return ((4*image_num)-1);
}

#if EMMC_PHY_PARTITION
static int create_emmc_GP_partitions( pemmcflash_probe_t * emmcdataprobe )
{
    int PartitionError=CFE_OK;

    /* [Step 1] Physical Partition(GP1~4, ENH_DATA) in Data Area */
    if( !pemmcdataprobe->emmc_config.config.PartitionCompleted )
    {
        printf("\n\n !!! Partitioning eMMC in Data Area !!! \n\n ");

        /* Do partition physically !!! */
        PartitionError = emmc_Partition_DataArea( &pemmcdataprobe->emmc_config );
        if( PartitionError == CFE_ERR )
        {
            printf("\n\n !!! eMMC Partition Error !!! \n\n");
            return CFE_ERR;
        }
        
    }

    /* [Step 2] Logical Partition & GPT : eMMC GP1~4/ENH_DATA in Data Area */
    // Partitioned physically. Start logical partition.
    if( pemmcdataprobe->emmc_config.config.PartitionCompleted && (PartitionError != CFE_ERR) )
    {
        /* Create logical and GPT partitions for the newly created  physical paritions */
    }

    return PartitionError;
}
#endif

static uint64_t create_default_partitions(emmcflash_probe_t * pemmcdataprobe,
                                          emmcflash_probe_t * pemmcboot1probe,
                                          emmcflash_probe_t * pemmcboot2probe,
                                          cfe_gpt_probe_t * pemmcdatagptprobe)
{
    int res;
    uint8_t num_parts;
    uint64_t temp_size;
    uint64_t emmc_flash_size = 0;

    /* [Step 1] Logical Partition & GPT(GUID Partition Table) : eMMC Data Area */
    num_parts = sizeof(EmmcDefaultDataPartition)/sizeof(EmmcDefaultDataPartition[0]);
    temp_size = enable_emmc_logicalpartition( (emmcflash_logicalpart_spec_t *)&(EmmcDefaultDataPartition[0]), pemmcdataprobe, EMMC_PART_DATA, num_parts );

    /* Try and initialize GPT partitions using the EmmcDefaultDataPartition table.
     * if this call returns CFE_ERR_WRONGDEVTYPE, this means that code encountered
     * an existing GPT partition table on the device whose partitions did not match
     * EMMCDefaultDataPartition table. In this case, simply recreate the CFE partitions
     * using information from the GPT partitions that were discovered
     * A
     */
    if( temp_size )
    {
        if( (res = enable_emmc_gpt( pemmcdatagptprobe, pemmcdataprobe, EMMC_PART_DATA, num_parts, 0)) == CFE_ERR_WRONGDEVTYPE )
        {
            /* ignore default partition settings and recreate cfe partitions from gpt*/
            temp_size = enable_emmc_logicalpartition_from_gpt( pemmcdatagptprobe, pemmcdataprobe, EMMC_PART_DATA );
        }
        else if ( res != CFE_OK )
        {
            printf("\n\n !!! eMMC GPT partitioning Failed !!! \n\n");
            res = CFE_ERR;
            goto err_exit;
        }
    }
    else
    {
        printf("\n\n !!! eMMC Logical CFE partitioning Failed, Ptype:%d !!! \n\n", EMMC_PART_DATA);
        emmc_flash_size = 0;
        goto err_exit;
    }
    emmc_flash_size = temp_size;
    
    /* [Step 2] Logical Partition : eMMC Boot1 */
    memcpy( pemmcboot1probe, pemmcdataprobe, sizeof(emmcflash_probe_t) );
    num_parts = sizeof(EmmcBoot1Partition)/sizeof(EmmcBoot1Partition[0]);
    temp_size = enable_emmc_logicalpartition( EmmcBoot1Partition, pemmcboot1probe, EMMC_PART_BOOT1, num_parts );
    if( !temp_size )
    {
        printf("\n\n !!! eMMC Logical CFE partitioning Failed, Ptype:%d !!! \n\n", EMMC_PART_BOOT1);
        emmc_flash_size = 0;
        goto err_exit;
    }
    emmc_flash_size += temp_size;
    
    /* [Step 3] Logical Partition : eMMC Boot2 */
    memcpy( pemmcboot2probe, pemmcdataprobe, sizeof(emmcflash_probe_t) );
    num_parts = sizeof(EmmcBoot2Partition)/sizeof(EmmcBoot2Partition[0]);
    temp_size = enable_emmc_logicalpartition( EmmcBoot2Partition, pemmcboot2probe, EMMC_PART_BOOT2, num_parts);
    if( !temp_size )
    {
        printf("\n\n !!! eMMC Logical CFE partitioning Failed, Ptype:%d !!! \n\n", EMMC_PART_BOOT2);
        emmc_flash_size = 0;
        goto err_exit;
    }
    emmc_flash_size += temp_size;

err_exit:
    return emmc_flash_size;
}

static int enable_emmc_flash( uint64_t *emmc_flash_size, uint8_t boot_mode )
{
    emmcflash_probe_t * pemmcdataprobe  = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );
    emmcflash_probe_t * pemmcboot1probe = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );
    emmcflash_probe_t * pemmcboot2probe = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );
    cfe_gpt_probe_t * pemmcdatagptprobe = (cfe_gpt_probe_t *)KMALLOC( sizeof(cfe_gpt_probe_t), 0 );

    if( !pemmcdataprobe || !pemmcboot1probe || !pemmcboot2probe || !pemmcdatagptprobe )
    {
        printf("\n\n !!! Failed to allocate memory for eMMC initialization !!! \n\n");
        return CFE_ERR;
    }

    /* [Step 1] eMMC Initialization */
    memset( pemmcdataprobe, 0, sizeof(emmcflash_probe_t) );
    pemmcdataprobe->emmc_config.config.CFEBootMode = boot_mode;
    if( emmc_Initialize( &pemmcdataprobe->emmc_config ) != CFE_OK )
        return CFE_ERR;
        
    /* Ensure that read/write block length matches our supported length */
    if( (pemmcdataprobe->emmc_config.config.ReadBlkLen != EMMC_DFLT_BLOCK_SIZE) 
        || (pemmcdataprobe->emmc_config.config.WriteBlkLen != EMMC_DFLT_BLOCK_SIZE) )
    {
        printf("\n\n !!! Unsupported read/write block sizes: %d/%d, supported size: %d !!! \n\n", 
                        pemmcdataprobe->emmc_config.config.ReadBlkLen,
                        pemmcdataprobe->emmc_config.config.WriteBlkLen,
                        EMMC_DFLT_BLOCK_SIZE);
        return CFE_ERR;
    }
    
    /* [Step 2] Partition device */
    *emmc_flash_size =  create_default_partitions( pemmcdataprobe, pemmcboot1probe,
                                                   pemmcboot2probe, pemmcdatagptprobe );
    
    /* [Step 3] Print eMMC partition information.*/
    if( boot_mode == BOOT_FROM_EMMC )
    {
        if( (pemmcdataprobe->emmc_config.config.BootPartitionEnable == 1) || (pemmcdataprobe->emmc_config.config.BootPartitionEnable == 2) )
            printf("\n [Booted from eMMC BOOT%d Partition]\n", pemmcdataprobe->emmc_config.config.BootPartitionEnable);
    }
    
    printf("EMMC Addr Mode: %s, ReadBlkLen %d bytes, WriteBlklen %d bytes\n", 
        (pemmcdataprobe->emmc_config.config.OCR_SectorMode == EMMC_ON)? "Sector":"Byte",
        pemmcdataprobe->emmc_config.config.ReadBlkLen,
        pemmcdataprobe->emmc_config.config.WriteBlkLen );  
    
    printf("EMMC device: %s %s v%d.%d, Serial 0x%08x, Size %dMB\n", 
        emmc_get_manufacturer_name(pemmcdataprobe->emmc_config.CID.MID),
        pemmcdataprobe->emmc_config.CID.PNM,
        (pemmcdataprobe->emmc_config.CID.PRV >> 4), (pemmcdataprobe->emmc_config.CID.PRV & 0x0F),
        pemmcdataprobe->emmc_config.CID.PSN,    
        ((pemmcdataprobe->emmc_config.config.DataSize + pemmcdataprobe->emmc_config.config.GP1Size + pemmcdataprobe->emmc_config.config.GP2Size
        + pemmcdataprobe->emmc_config.config.GP3Size + pemmcdataprobe->emmc_config.config.GP4Size + pemmcdataprobe->emmc_config.config.RPMBSize
        + pemmcdataprobe->emmc_config.config.Boot1Size + pemmcdataprobe->emmc_config.config.Boot2Size)>>20) );    
    
    printf(" [eMMC Partition Information] : \n");
    printf("  Partition  :  Physical,   Partitioned\n");
    if( pemmcdataprobe->emmc_config.config.OCR_SectorMode == EMMC_ON )
    {
        printf("  - Data     :  %6uMB,   %6uMB\n", 
            (uint32_t)(pemmcdataprobe->flash_phy_size>>20), (uint32_t)(pemmcdataprobe->flash_log_size>>20));
#if EMMC_ENH_PARTITION
        printf("  -  ENH Data:  %6uMB,            Address(0x%012llX%)!\n", 
            (uint32_t)(pemmcdataprobe->emmc_config.config.DataEnhSize>>20), pemmcdataprobe->emmc_config.config.DataEnhAddr); 
#endif
    }
    else
    {
        printf("  - Data     :  %6uMB),  %6uMB!\n", (uint32_t)(pemmcdataprobe->flash_phy_size>>20), (uint32_t)(pemmcdataprobe->flash_log_size>>20));
    }
    printf("  - Boot1    :  %6uMB,   %6uKB\n", (uint32_t)(pemmcboot1probe->flash_phy_size>>20), (uint32_t)(pemmcboot1probe->flash_log_size>>10)); 
    printf("  - Boot2    :  %6uMB,   %6uKB\n", (uint32_t)(pemmcboot2probe->flash_phy_size>>20), (uint32_t)(pemmcboot2probe->flash_log_size>>10)); 
    printf("\n");

    KFREE(pemmcdataprobe ); 
    KFREE(pemmcboot1probe); 
    KFREE(pemmcboot2probe); 
    KFREE(pemmcdatagptprobe);  

    if( !*emmc_flash_size )
        return CFE_ERR;
      
    return CFE_OK;
}

static uint64_t enable_emmc_logicalpartition_from_gpt( cfe_gpt_probe_t * fgptprobe, emmcflash_probe_t * femmcprobe, int attr)
{
    uint8_t     i;
    uint64_t    fp_size=0;

    emmc_delete_cfe_devs(attr, femmcprobe);

    /* Sanity Check */
    if( fgptprobe->num_parts > EMMC_MAX_DATA_PARTS )
    {
        printf("Cant create %d logical partitions! Max paritition count = %d\n", fgptprobe->num_parts, EMMC_MAX_DATA_PARTS );
        printf("Will restrict CFE visible partition count to %d to allow recovery\n", EMMC_MAX_DATA_PARTS );
	fgptprobe->num_parts = EMMC_MAX_DATA_PARTS;
    }
    
    for( i=0; i < fgptprobe->num_parts; i++ )
    {
        /* Fill in emmc probe parameters from the gpt probe */
        femmcprobe->flash_part_spec[i].fp_size = fgptprobe->cfe_parts[i].fp_size;
        femmcprobe->flash_part_spec[i].fp_offset_bytes = fgptprobe->cfe_parts[i].fp_offset_bytes;
        strncpy(femmcprobe->flash_part_spec[i].fp_name, fgptprobe->cfe_parts[i].fp_name, strlen(fgptprobe->cfe_parts[i].fp_name)+1);
        femmcprobe->flash_part_spec[i].fp_partition = attr;
        fp_size += fgptprobe->cfe_parts[i].fp_size;
    }
    femmcprobe->flash_nparts = i;
    femmcprobe->flash_part_attr = attr;

    cfe_add_device( &emmcflashdrv, 0, 0, femmcprobe);      // Add eMMC flash driver
    
    if( (femmcprobe->flash_log_size > femmcprobe->flash_phy_size) || (fp_size > femmcprobe->flash_phy_size) )
    {
        printf("\n\n !!! [ERROR] Partition %d : logical %u %u is bigger than physical %u !!!\n\n"
           , attr, femmcprobe->flash_log_size, fp_size, femmcprobe->flash_phy_size);
    }
    
    return femmcprobe->flash_phy_size;
}

static uint64_t enable_emmc_logicalpartition( emmcflash_logicalpart_spec_t *EmmcPartition, emmcflash_probe_t *femmcprobe, int attr, uint8_t num_parts )
{
    uint8_t        i;
    uint64_t    fp_size=0;

    /* Sanity Check */
    if( num_parts > EMMC_MAX_DATA_PARTS )
    {
        printf("Cant create %d logical partitions! Max paritition count = %d\n", num_parts, EMMC_MAX_DATA_PARTS );
        return 0;
    }

    for( i=0; i < num_parts; i++ )
    {
	/* Creating new partitions, we let the partitioning code 
	 * take care of aligning the partitions properly, so set to 0 
	 */
        femmcprobe->flash_part_spec[i].fp_offset_bytes = 0;

	/* Update sizes, names and partition types */
        femmcprobe->flash_part_spec[i].fp_size = EmmcPartition[i].fp_size;
        strncpy(femmcprobe->flash_part_spec[i].fp_name, EmmcPartition[i].fp_name, strlen(EmmcPartition[i].fp_name)+1);
        femmcprobe->flash_part_spec[i].fp_partition = attr;
        fp_size += EmmcPartition[i].fp_size;
    }
    femmcprobe->flash_nparts = i;
    femmcprobe->flash_part_attr = attr;
    cfe_add_device( &emmcflashdrv, 0, 0, femmcprobe);  
    
    if( (femmcprobe->flash_log_size > femmcprobe->flash_phy_size) || (fp_size > femmcprobe->flash_phy_size) )
    {
        printf("\n\n !!! [ERROR] Partition %d : logical %u %u is bigger than physical %u. !!!\n\n"
           , attr, femmcprobe->flash_log_size, fp_size, femmcprobe->flash_phy_size);
    }
    
    return femmcprobe->flash_phy_size;
    
}

static int init_emmc_gpt(cfe_gpt_probe_t *cfe_gpt_probe, int force_overwrite)
{
    int res;

    /* Allocate memory */
    cfe_gpt_probe->ptr_gpt_prim = (uint8_t *)KMALLOC( CFE_GPT_BACKUP_SIZE, cfe_gpt_probe->block_size );
    cfe_gpt_probe->ptr_gpt_back = (uint8_t *)KMALLOC( CFE_GPT_BACKUP_SIZE, cfe_gpt_probe->block_size );

    if( !cfe_gpt_probe->ptr_gpt_prim || !cfe_gpt_probe->ptr_gpt_back )
    {
        printf("Failed to allocate memory for GPT\n");
        return CFE_ERR;
    }

    /* Open files */
    cfe_gpt_probe->fd_prim = cfe_open( cfe_gpt_probe->dev_gpt0_name );
    cfe_gpt_probe->fd_back = cfe_open( cfe_gpt_probe->dev_gpt1_name );
    
    if( cfe_gpt_probe->fd_prim < 0 || cfe_gpt_probe->fd_back < 0 )
    {
        printf("Failed to open CFE partitions %s %d\n", cfe_gpt_probe->dev_gpt0_name , cfe_gpt_probe->dev_gpt1_name);
        return CFE_ERR;
    }
    
#if !DEV_EMMC_DBG_DISABLE_EMMCBOOT_RW
    /* Assign funcs */
    cfe_gpt_probe->read_func = &cfe_readblk;
    cfe_gpt_probe->write_func = &cfe_writeblk;
#endif
    
    /* Set offsets to 0 - Not needed in cferam */
    cfe_gpt_probe->offset_prim = 0; 
    cfe_gpt_probe->offset_back = 0;

    if( !force_overwrite )
       res = cfe_gpt_run( cfe_gpt_probe );

    if( force_overwrite || (res == CFE_ERR_ENVNOTFOUND) )
    {
        if( res == CFE_ERR_ENVNOTFOUND )
            printf("No Valid GPT header found. Initializing new GPT headers\n");

        if((res = cfe_gpt_init( cfe_gpt_probe )) != CFE_OK )
        {
            printf("GPT initialization failed. !!!\n");
        }
        else
        {    
            printf("GPT initialization success. !!!\n");
        }        
    }
    else if( res < 0 )
    {
        printf("Failed to find valid GPT headers! Did not attempt to create new GPT headers\n");
    }
    
    KFREE(cfe_gpt_probe->ptr_gpt_prim);
    KFREE(cfe_gpt_probe->ptr_gpt_back);

    cfe_close(cfe_gpt_probe->fd_prim);
    cfe_close(cfe_gpt_probe->fd_back);
    return res;
}

static int enable_emmc_gpt( cfe_gpt_probe_t *fgptprobe, emmcflash_probe_t *femmcprobe, int attr, uint8_t num_parts, int force )
{
    queue_t            *qb;
    cfe_device_t    *dev;
    uint8_t            i;
    int res;
    cfe_gpt_probe_t *fgptprobe_backup;

#if DEV_EMMC_DBG_DISABLE_GPT
    printf("Skipping GPT read/creation!!\n");
    return CFE_OK;
#endif    

    /* Alloc Memory */
    fgptprobe_backup = (cfe_gpt_probe_t *)KMALLOC( sizeof(cfe_gpt_probe_t), 0 );

    if( !fgptprobe_backup )
    {
        printf("Failed to allocate memory for GPT initialization\n");
        return CFE_ERR;
    }

    //--------------------------------------------
    // [Step 1-1] Getting GPT partition name to read/write GPT
    for (qb = cfe_devices.q_prev; qb != &cfe_devices; qb = qb->q_prev)
    {
        dev = (cfe_device_t *)qb;
        if((strncmp(dev->dev_fullname, "emmc",4) == 0))
        {    
            if( !strncmp( (const char*)&(dev->dev_fullname[strlen(dev->dev_fullname) - strlen(PRIMARY_GPT_HDR_PART_NAME)] ),
                PRIMARY_GPT_HDR_PART_NAME,
                strlen(PRIMARY_GPT_HDR_PART_NAME)))
            {
                fgptprobe->dev_gpt0_name = dev->dev_fullname;    // gpt0, Primary GPT
            }
            else if(!strncmp( (const char*)&(dev->dev_fullname[strlen(dev->dev_fullname) - strlen(BACKUP_GPT_HDR_PART_NAME)] ),
                BACKUP_GPT_HDR_PART_NAME,
                strlen(BACKUP_GPT_HDR_PART_NAME)))
            {
                fgptprobe->dev_gpt1_name = dev->dev_fullname;    // gpt1, Backup GPT        
            }        
        }
    }  

    //--------------------------------------------
    // [Step 1-2] Probe parameters
    for (i = 0; i < num_parts; i++)
    {
        fgptprobe->cfe_parts[i].fp_size      = femmcprobe->flash_part_spec[i].fp_size;
        fgptprobe->cfe_parts[i].fp_offset_bytes = femmcprobe->flash_part_spec[i].fp_offset_bytes;
        fgptprobe->cfe_parts[i].fp_partition = femmcprobe->flash_part_spec[i].fp_partition;
        strncpy(fgptprobe->cfe_parts[i].fp_name, femmcprobe->flash_part_spec[i].fp_name, strlen(femmcprobe->flash_part_spec[i].fp_name)+1);
    }
    fgptprobe->num_parts             = num_parts;
    fgptprobe->block_size            = femmcprobe->emmc_config.config.ReadBlkLen;
    fgptprobe->block_size_bit        = femmcprobe->emmc_config.config.ReadBlkLenBit4Addr;
    switch( attr )
    {
        case EMMC_PART_DATA: fgptprobe->lba_size = (uint64_t)(femmcprobe->emmc_config.config.DataSize >> fgptprobe->block_size_bit); break;
        case EMMC_RW_GP1:     fgptprobe->lba_size = (uint64_t)(femmcprobe->emmc_config.config.GP1Size  >> fgptprobe->block_size_bit); break;
        case EMMC_RW_GP2:     fgptprobe->lba_size = (uint64_t)(femmcprobe->emmc_config.config.GP2Size  >> fgptprobe->block_size_bit); break;
        case EMMC_RW_GP3:     fgptprobe->lba_size = (uint64_t)(femmcprobe->emmc_config.config.GP3Size  >> fgptprobe->block_size_bit); break;
        case EMMC_RW_GP4:     fgptprobe->lba_size = (uint64_t)(femmcprobe->emmc_config.config.GP4Size  >> fgptprobe->block_size_bit); break;
        default:             fgptprobe->lba_size = (uint64_t)(femmcprobe->emmc_config.config.DataSize >> fgptprobe->block_size_bit); break;
    }    
    fgptprobe->cfe_FirstUsableLba    = (uint64_t)(CFE_GPT_PRIMRY_SIZE >> fgptprobe->block_size_bit);
    fgptprobe->cfe_LastUsableLba     = fgptprobe->lba_size - (uint64_t)(CFE_GPT_BACKUP_SIZE >> fgptprobe->block_size_bit);
    
    /* Backup gpt probe since it can be modified by cfe_gpt_run */
    memcpy(fgptprobe_backup, fgptprobe, sizeof(cfe_gpt_probe_t));

    //--------------------------------------------
    // [Step 2] GPT
    //printf("[Step 1] cfe_gpt_run\n");
    if( (res = init_emmc_gpt(fgptprobe, force)) == CFE_OK && !force  )
    {
        int gptMismatch = 0;

	if( fgptprobe->num_parts != fgptprobe_backup->num_parts )
        {
            gptMismatch = 1;
        }
        else
        {
            for( i=0; i<fgptprobe->num_parts; i++ )
            {
                if( (fgptprobe->cfe_parts[i].fp_size != fgptprobe_backup->cfe_parts[i].fp_size)
                 || (fgptprobe->cfe_parts[i].fp_offset_bytes != fgptprobe_backup->cfe_parts[i].fp_offset_bytes) )
                {
                    gptMismatch = 1;
                    break;
                }
            }
        }

        if( gptMismatch )
        {
            printf("GPT partitions exist, Updating cfe partitions!\n");
            res = CFE_ERR_WRONGDEVTYPE;
        }
        else
            printf("GPT validation success. !!!\n");
    }
    
    KFREE(fgptprobe_backup);

    return res;
}

static int parse_emmc_bootfs( char * flashdev, char * filename, char * data_buffer, int * data_length )
{
    int ret = 0;
    int fh = 0;
               
    unsigned int emmcBlkSize = EMMC_DFLT_BLOCK_SIZE;

    ret = cfe_getdevinfo(flashdev);
    if (ret < 0) {
        printf("Device %s not found\n", flashdev); 
        return -1;
    }
    
    fh = cfe_open(flashdev);      

    if (fh < 0) {
        printf("Cannot open Device %s, fh: %d\n", flashdev, fh); 
        return -1;
    }
    
    /* Malloc buffer of 2 * blocksize */
    char * dev_buffer = (char*)KMALLOC( 2*emmcBlkSize, emmcBlkSize );

    if (!dev_buffer) {
        printf("Cannot allocate memory for staging buffer!\n"); 
        return -1;
    }

    ret = parse_emmc_bootfs_common( fh, 0, filename, dev_buffer, data_buffer, data_length, emmcBlkSize, &cfe_readblk);

    cfe_close(fh);

    KFREE(dev_buffer);

    return ret;
}

static void ui_emmc_dumphex( unsigned char *pAddr, unsigned int offset, int nLen, int sz )
{
    int a;
    unsigned char *j;
    unsigned char *crow;
    unsigned short *hrow;
    unsigned int wrow[4];
    int i;
    crow = (unsigned char *)wrow;
    hrow = (unsigned short *)wrow;
    a = 0;
    pAddr = (unsigned char *)((uintptr_t)pAddr & (~(sz-1)));
    do {
        if ((a & 15) == 0) {
            printf("%08x: ", offset +a);
        }
        if (a < nLen * sz) {
            switch (sz) {
            case 1:
                printf("%02x ",crow[a & 15] = *(unsigned char *)(pAddr+a));
                break;
            case 2:
                printf("%04x ",hrow[(a/2)&7] = *(unsigned short *)(pAddr+a));
                break;
            case 4:
                printf("%08x ",wrow[(a/4)&3] = *(unsigned int *)(pAddr+a));
                break;
            }
        } else {
            for (i = 0; i < sz*2 + 1 ; i++) {
                printf(" ");
            }
        }
        if (((a + sz) & 15) == 0) {
            for (i = 0, j = pAddr +(a & ~15) ; (j < pAddr + nLen*sz) && (j <= pAddr +a ) ; j++, i++) {
                printf("%c", (crow[i] >= ' ' && crow[i] <= '~') ? crow[i] : '.');
            }
            printf("\n");
        }
        a = a + sz;
    } while (a < ((nLen*sz+15) & ~15));
}

#if DEBUG_CRC
static unsigned int debugGetCrc32(unsigned char * dev_buffer, unsigned int remDataSize, unsigned int dataCrc)
{
    unsigned int i;
    unsigned int dataCrcInt = dataCrc;
    if( dataCrc == CRC32_INIT_VALUE )
    {
        byteSum = 0; 
        numByteSum1 = 0; 
    }
    
    for( i=0; i<remDataSize; i++ )
    {
        dataCrcInt = getCrc32( dev_buffer+i, 1, dataCrcInt); 
        printf("%02x ",  *(dev_buffer+i));               
        numByteSum1++;
        if( (numByteSum1 & 15) == 0)
            printf("\n");
        byteSum += *(dev_buffer+i);
    }     
    
    return dataCrcInt;
}
#endif

static uint64_t emmc_get_part_size(char * flashdev)
{
    int ret = 0;
    cfe_device_t *dev = NULL;
    uint64_t size = 0;

    if( flashdev )
        ret = cfe_getdevinfo(flashdev);
    else
        return size;
                
    dev = cfe_finddev(flashdev);

    if (ret < 0 || !dev) 
    {
        size = 0;
    }
    else    
        size = ((emmcflash_cfepart_cfg_t *)dev->dev_softc)->fp_size;
    
    return size;
}

static int emmc_check_image_size( PFILE_TAG pTag) 
{
    int ret = CFE_ERR;
    uint64_t bootfs_partition_size = emmc_get_part_size(EMMC_CFE_PNAME_BOOTFS(1));
    uint64_t rootfs_partition_size = emmc_get_part_size(EMMC_CFE_PNAME_ROOTFS(1));
    uint64_t cferom_partition_size = emmc_get_part_size(EMMC_CFE_PNAME_CFE);
    uint64_t mdata_partition_size  = emmc_get_part_size(EMMC_CFE_PNAME_MDATA(1, 1));

    if (   atoi(pTag->cfeLen)    <= cferom_partition_size
        && atoi(pTag->bootfsLen) <= bootfs_partition_size
        && atoi(pTag->rootfsLen) <= rootfs_partition_size
        && atoi(pTag->mdataLen)  <=  mdata_partition_size )
        ret = CFE_OK;

    return ret;
}

/***************************************************************************
 *                                                                         *
 *  eMMC device API for CFE                                                *
 *                                                                         *
 ***************************************************************************/
char *get_emmc_chosen_root(void)
{
	return root_device_name;
}

void get_emmc_boot_cfe_version(char **version, int *size)
{
	*version=bootCfeVersion;
	*size=sizeof(bootCfeVersion);
}

int emmc_dump_bootfs(char * flashdev, char * filename)
{
    int ret = 0;
    
    if (!flashdev) 
    {
        printf("Emmc logical partition not specified\n"); 
        return CFE_ERR_IOERR;
    }
                    
    ret = parse_emmc_bootfs( flashdev, filename, NULL, NULL );
    
    if (ret < 0) 
    {
            printf("eMMC ioctl error\n");
            ret = CFE_ERR_IOERR;
    }
    return ret;
}

int emmc_erase_img( int img_num , int erase_cferom_nvram)
{
    char part_name[EMMC_MAX_PART_NAME];
    int ret = 0;
    if( erase_cferom_nvram )
    {
        sprintf(part_name, EMMC_CFE_PNAME_CFE);
        ret += emmc_erase_partition(part_name);
    }
    sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_BOOTFS, img_num);
    ret += emmc_erase_partition(part_name);
    sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_ROOTFS, img_num); 
    ret += emmc_erase_partition(part_name);
    sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_MDATA, img_num, 1);
    ret += emmc_erase_partition(part_name);
    sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_MDATA, img_num, 2);
    ret += emmc_erase_partition(part_name);

    return ret;
}

int emmc_erase_partition(char * flashdev)
{
    int ret = 0;
    int fh = 0;
    flash_range_t erase_range;
    cfe_device_t *dev = NULL;

    if( flashdev )
        ret = cfe_getdevinfo(flashdev);
    else
        ret = -1;
                
    dev = cfe_finddev(flashdev);

    if (ret < 0 || !dev) {
        printf("Device %s not found\n", flashdev); 
        return CFE_ERR_IOERR;
    }
    
    fh = cfe_open(flashdev);

    if (fh < 0) {
        printf("Cannot open Device %s, fh: %d\n", flashdev, fh); 
        return -1;
    }
    
    /* Set erase range to cover entire partition size */
    erase_range.range_base = 0;
    erase_range.range_length = ((emmcflash_cfepart_cfg_t *)dev->dev_softc)->fp_size - 1;
    
    ret = cfe_ioctl( fh, IOCTL_FLASH_ERASE_RANGE, (void*)&erase_range, sizeof(flash_range_t), 0, 0 );
        
    cfe_close(fh);
    
    if (ret != 0) 
    {
            printf("eMMC ioctl error\n");
            ret = CFE_ERR_IOERR;
    }
    return ret;
}
    
int emmc_write_part_words(char * flashdev, unsigned int offset, unsigned int write_val)
{
    int ret = 0;
    int fh = 0;
    unsigned char *dev_buffer;
    
    if (!flashdev) 
    {
        printf("eMMC ioctl error: flash partition not specified\n");        
        return CFE_ERR_IOERR;     
    }
           
    ret = cfe_getdevinfo(flashdev);
    if (ret < 0) {
        printf("Device %s not found\n", flashdev); 
        return CFE_ERR_IOERR;
    }
    
    fh = cfe_open(flashdev);

    if (fh < 0) {
        printf("Cannot open Device %s, fh: %d\n", flashdev, fh); 
        return -1;
    }
      
    dev_buffer = (unsigned char*)KMALLOC( sizeof(unsigned int), EMMC_DFLT_BLOCK_SIZE );

    if (!dev_buffer) {
        printf("Cannot allocate memory for staging buffer!\n"); 
        return -1;
    }

    *(unsigned int *)dev_buffer = write_val;    
    ret = cfe_writeblk(fh ,offset ,dev_buffer, sizeof(unsigned int));    
    KFREE(dev_buffer);
    
    cfe_close(fh);
    
    if (ret < 0) 
    {
            printf("eMMC ioctl error\n");
            ret = CFE_ERR_IOERR;
    }
    return ret;
}

int emmc_read_part_words(char * flashdev, unsigned int offset, unsigned int num_words)
{
    int ret = 0;
    int fh = 0;        
    unsigned char *dev_buffer;
        
    if (!flashdev) 
        flashdev = EMMC_CFE_PNAME_NVRAM;
           
    num_words = (num_words ? num_words: 64);
            
    ret = cfe_getdevinfo(flashdev);
    if (ret < 0) {
        printf("Device %s not found\n", flashdev); 
        return CFE_ERR_IOERR;
    }
    
    fh = cfe_open(flashdev);

    if (fh < 0) {
        printf("Cannot open Device %s, fh: %d\n", flashdev, fh); 
        return -1;
    }
      
    dev_buffer = (unsigned char*)KMALLOC( num_words*4, EMMC_DFLT_BLOCK_SIZE );

    if (!dev_buffer) {
        printf("Cannot allocate memory for staging buffer!\n"); 
        return -1;
    }
    
    ret = cfe_readblk(fh ,offset ,dev_buffer, num_words*4);
    
    printf("Dump of eMMC partition:%s at offset:0x%08x\n", flashdev, offset);
    ui_emmc_dumphex((unsigned char *)dev_buffer, offset, 4*num_words, 1);    
    KFREE(dev_buffer);
    
    cfe_close(fh);
    
    if (ret < 0) 
    {
            printf("eMMC ioctl error\n");
            ret = CFE_ERR_IOERR;
    }
    return ret;
}

int emmc_get_info(void)
{
    int ret = 0;
    int fh = 0;
    char *flashdev = 0;

    queue_t *qb;
    cfe_device_t *dev;

    for (qb = cfe_devices.q_next; qb != &cfe_devices; qb = qb->q_next) 
    {
        dev = (cfe_device_t *) qb;

        if (strncmp(dev->dev_fullname, "emmc",4) == 0)
        {
            flashdev = dev->dev_fullname;
            break;
        }
    }
        
    if( flashdev )
        ret = cfe_getdevinfo(flashdev);
    else
        ret = -1;
                
    if (ret < 0) {
        printf("Device %s not found\n", flashdev); 
        return CFE_ERR_IOERR;
    }
    
    fh = cfe_open(flashdev);

    if (fh < 0) {
        printf("Cannot open Device %s, fh: %d\n", flashdev, fh); 
        return -1;
    }
    
    ret = cfe_ioctl( fh, IOCTL_EMMC_INFO, 0, 0, 0, 0 );
        
    cfe_close(fh);
    
    if (ret != 0) 
    {
            printf("eMMC ioctl error\n");
            ret = CFE_ERR_IOERR;
    }
    return ret;
}

int emmc_init_dev( void )
{  
    int ret=0;
    uint64_t emmc_flash_size = 0;    
    uint64_t boot_mode = BOOT_FROM_EMMC;
    
    emmc_set_device_name( );
    if( (ret = enable_emmc_flash( &emmc_flash_size, boot_mode )) != CFE_OK )
    {
        printf("!!! eMMC init failed !!!\n");
        ret = -1;
    }
    
#if (SKIP_FLASH==0)
    if(NVRAM_INIT()) {
       printf("ERROR: Can't initialize NVRAM\n");
    } 

    while ((BpSetBoardId((char*)NVRAM.szBoardId) != BP_SUCCESS))
    {
        printf("\n*** Board is not initialized properly ***\n\n");
        setBoardParam();
    }
#endif    
    return ret;
}    

int emmc_go_idle(void)
{
    int ret = 0;
    int fh = 0;
    char *flashdev = 0;

    queue_t *qb;
    cfe_device_t *dev;

    for (qb = cfe_devices.q_next; qb != &cfe_devices; qb = qb->q_next) 
    {
        dev = (cfe_device_t *) qb;

        if (strncmp(dev->dev_fullname, "emmc",4) == 0)
        {
            flashdev = dev->dev_fullname;
            break;
        }
    }
        
    if( flashdev )
        ret = cfe_getdevinfo(flashdev);
    else
        ret = -1;
                
    if (ret < 0) {
        return CFE_ERR_IOERR;
    }
    
    fh = cfe_open(flashdev);

    if (fh < 0) {
        printf("Cannot open Device %s, fh: %d\n", flashdev, fh); 
        return -1;
    }
    
    ret = cfe_ioctl( fh, IOCTL_EMMC_RESET, 0, 0, 0, 0 );
        
    cfe_close(fh);
    
    return ret;    
}

int emmc_boot_os_image(int imageNum)
{
    unsigned char *buf = (unsigned char *) (mem_topofmem & 0xfffffff0) + 1024;
    char fname_lz[] = NAND_FLASH_BOOT_IMAGE_LZ;
#ifdef USE_LZ4_DECOMPRESSOR
    char fname_lz4[] = NAND_FLASH_BOOT_IMAGE_LZ4;
#endif
    
    int emmcBlkSize = EMMC_DFLT_BLOCK_SIZE;
    int len = 4 * emmcBlkSize;
    
    unsigned char *image = (unsigned char *) buf + len;
    unsigned int  image_hdr_size = 0, image_sig_size = 0, image_size;
    char          brcmMagic[] = {'B','R','C','M'};    
    int ret = -1;
    int bootImg;
    bcm_image_hdr_t image_hdr;
    cfe_loadargs_t la;
    NVRAM_DATA nvramData;
    
    /* Retrieve NVRAM parameters */
    NVRAM_COPY_TO(&nvramData);

    memset((unsigned char *) &la, 0x0, sizeof(la));
    la.la_flags = LA_DEFAULT_FLAGS;
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (load_hash_block(0,0) != 0)
        die();
#endif // CONFIG_CFE_SUPPORT_HASH_BLOCK

#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_) || \
defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM963158_) || \
defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM96856_)
#ifndef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (bcm_otp_is_boot_secure()) {
#if (SEC_S_SIGNATURE&3) != 0
#error "SEC_S_SIGNATURE must be 4 byte aligned"   
#endif  
          if( (image_sig_size = load_file_from_next_bootfs( NAND_FLASH_BOOT_SIG_NAME, (char *)image, &bootImg, 
                         &imageNum, &parse_emmc_bootfs )) < 0 ) {
              printf("Unable to load signature !\n");
              return image_sig_size;
          }
          if (SEC_S_SIGNATURE != image_sig_size) 
          {
              printf("Image is corrupt %s\n",NAND_FLASH_BOOT_SIG_NAME);
              return -1;
          }
          image += image_sig_size; 
    }
#endif // !CONFIG_CFE_SUPPORT_HASH_BLOCK
#endif

    /* Try and load kernel from partition */
#ifdef USE_LZ4_DECOMPRESSOR
    if( (image_size = load_file_from_next_bootfs( fname_lz4, (char *)image, &bootImg, 
            &imageNum, &parse_emmc_bootfs )) < 0 )
#endif        
    {
        if( (image_size = load_file_from_next_bootfs( fname_lz, (char *)image, &bootImg, 
                &imageNum, &parse_emmc_bootfs )) < 0 )
        {
            printf("Unable to load kernel image!\n");
            return image_size;
        }
    }
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (hash_block_start) {
        unsigned char hash[SHA256_S_DIGEST8];
        int ret;
        unsigned int content_len;
        // printf("look for hash for %s\n",NAND_FLASH_BOOT_IMAGE_LZ);
        // printf("start of hash block %x %x %x %x\n",hash_block_start[0],hash_block_start[1],hash_block_start[2],hash_block_start[3]);
        ret = find_boot_hash(&content_len, hash, hash_block_start, NAND_FLASH_BOOT_IMAGE_LZ); // FIXME -- LZ4 not supported
        if (ret == 0)  {
            printf("failed to find hash for %s\n",NAND_FLASH_BOOT_IMAGE_LZ);
            die();
        } else {
            printf("got hash for %s\n",NAND_FLASH_BOOT_IMAGE_LZ);
           if (sec_verify_sha256((uint8_t const*)image, image_size, (const uint8_t *)hash)) {
               printf("Kernel Digest failed\n");
               die();
           } else {
               printf("Kernel Digest OK\n");
           }

        }

    }
#endif // CONFIG_CFE_SUPPORT_HASH_BLOCK
    /* If we are booting from a specified or only image
     * we store the index in a global variable. This variable
     * will be used in the dtb retrieval function to bypass 
     * the image search and speed things up */
    if( bootImg == BOOTED_ONLY_IMAGE )
        boot_only_img_idx = imageNum;

    /* Suffice to mention that there's no way 
       to tell what endianness was used for the image header...  */
    memcpy(&image_hdr, image, sizeof(image_hdr)); 
    /* leagacy adjustments:
       new image format contains broadcom signature and uncompressed length.*/
    image_hdr_size = (image_hdr.magic == *(uint32_t*)brcmMagic)? sizeof(image_hdr) : sizeof(image_hdr)-sizeof(uint32_t);
    image += image_hdr_size;

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    /* ARM linux kernel compiled for virtual address at 0xc0008000,
       convert to physical address for CFE */
    la.la_address =(long)(((uintptr_t)image_hdr.la)&0xfffffff);
    la.la_entrypt = (long)(((uintptr_t)image_hdr.entrypt)&0xfffffff);
#else
    la.la_address = (long)image_hdr.la;
    la.la_entrypt = (long)image_hdr.entrypt;
#endif

#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_) || \
defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM963158_) || \
defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM96856_)
    // only use this secure boot authentication if hash_block was not already checked
    if (!hash_block_start && (bcm_otp_is_boot_secure())) {
          char *sig_obj = NULL;
          Booter1AuthArgs authArgs;
          /* Authenticate vmlinux */
          /* cferom found and ran cferam... cferam was not somehow loaded by jtag */
          /* Security credentials should be available. */
          CFE_RAM_ROM_PARMS_AUTH_PARM_GETM(&authArgs);
          /* Authenticate the vmlinux.lz image */
          printf("%s", "Authenticating vmlinux.lz ... "); 
          /* header size on vmlinux is 12 bytes on aarch32 and 20 bytes on aarch64 */
		
          sig_obj = (char*)authenticate((uint8_t *)(image - image_sig_size - image_hdr_size), 
                                              image_hdr.len + image_sig_size + image_hdr_size, authArgs.manu);
          if (!sig_obj) { 
          	printf("vmlinux.lz cannot be authenticated. Stoppping\n"); 
          	while(1);
       	  }
    }
#endif

#ifdef USE_LZ4_DECOMPRESSOR
    if (image_hdr.len_uncomp) {           
        ret = LZ4_decompress_fast((const char *)image, (char *)(void*)la.la_address, image_hdr.len_uncomp) != image_hdr.len;
    } else
#endif
    {
        ret = decompressLZMA((unsigned char*)image, image_hdr.len, (unsigned char*)la.la_address, (RAMAPP_TEXT - la.la_address)) < 0;
    }

    if (ret) 
        printf("Failed to decompress %s image.  ret = %d Corrupted image?\n",image_hdr.len_uncomp? "LZ4":"LZMA",ret);
    else
    {
        /* Pass eMMC ROOTFS to Linux Kernel */
        sprintf(root_device_name, EMMC_LINUX_DEV_NAME_FMTSTR, 
                0, map_image_num_to_linux_rootfs_part(imageNum));
         
        /* TODO: Remove NAND reference */
        blparms_set_int(NAND_RFS_OFS_NAME, 260096);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
        /* Continue to save the rootfs offset the word before the 
         * destination (load) address for backwards compatibility.
         */
        *(unsigned int *) (image - 4) = (start_blk * len) / 1024;
#endif

        /* Pass the age of the Linux image that is being booted to Linux.
         * BOOTED_OLD_IMAGE, BOOTED_NEW_IMAGE or BOOTED_ONLY_IMAGE
         */
        blparms_set_int(BOOTED_IMAGE_ID_NAME, bootImg);

        /* Pass board parameters to be loaded in Linux by function in
         * kerSysEarlyFlashInit in file bcm63xx_flash.c, bypassing requirement 
         * for early driver support
         */
        blparms_set_str(BOARD_ID_NAME, nvramData.szBoardId);
        blparms_set_str(VOICE_BOARD_ID_NAME, nvramData.szVoiceBoardId);
        blparms_set_int(BOARD_STUFF_NAME, nvramData.ulBoardStuffOption);

        /* Pass the boot cfe version in dtb */
        read_cferom_version( (char*)bootCfeVersion );

        printf("Decompression %s Image OK!\n",image_hdr.len_uncomp ? "LZ4":"LZMA");
        printf("Entry at 0x%p\n",la.la_entrypt);

        cfe_go(&la);  /* never return... */
    }
    return( ret );
}

int emmc_dump_gpt_dataPhysPart(void)
{
    uint64_t bootfs_sizeKb, rootfs_sizeKb, data_sizeKb, misc1_sizeKb, misc2_sizeKb, misc3_sizeKb, misc4_sizeKb;

    bootfs_sizeKb = emmc_get_part_size(EMMC_CFE_PNAME_BOOTFS(1))/1024;
    rootfs_sizeKb = emmc_get_part_size(EMMC_CFE_PNAME_ROOTFS(1))/1024;
    data_sizeKb   = emmc_get_part_size(EMMC_CFE_PNAME_DATA     )/1024;
    misc1_sizeKb  = emmc_get_part_size(EMMC_CFE_PNAME_MISC(1)  )/1024;
    misc2_sizeKb  = emmc_get_part_size(EMMC_CFE_PNAME_MISC(2)  )/1024;
    misc3_sizeKb  = emmc_get_part_size(EMMC_CFE_PNAME_MISC(3)  )/1024;
    misc4_sizeKb  = emmc_get_part_size(EMMC_CFE_PNAME_MISC(4)  )/1024;
    
    printf("emmc format command for current partition configuration:\n");
    printf("emmcfmtgpt bootfsKB rootfsKB dataKB misc1KB misc2KB misc3KB misc4KB\n");
    printf("emmcfmtgpt %llu %llu %llu %llu %llu %llu %llu\n", bootfs_sizeKb, rootfs_sizeKb, 
        data_sizeKb, misc1_sizeKb, misc2_sizeKb, misc3_sizeKb, misc4_sizeKb);
    return 0;
}

int emmc_format_gpt_dataPhysPart(unsigned int bootfs_sizekb, unsigned int rootfs_sizekb, unsigned int data_sizekb,
                    unsigned int misc1_sizekb, unsigned int misc2_sizekb, unsigned int misc3_sizekb, unsigned int misc4_sizekb) 
{
    int                 res = 0;
    emmcflash_probe_t   *femmcprobe = NULL;
    emmcflash_logicalpart_spec_t * newPart = NULL;
    int emmcPhysPartAttr = EMMC_PART_DATA;
    uint8_t num_parts    = 0;
    uint8_t             j;

    cfe_gpt_probe_t * pemmcdatagptprobe = (cfe_gpt_probe_t *)KMALLOC( sizeof(cfe_gpt_probe_t), 0 );
    femmcprobe = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );

    if (!pemmcdatagptprobe || !femmcprobe) {
        printf("Failed to allocate memory for partitioning!\n"); 
        return -1;
    }

    /* Erase logical partitions */
    emmc_erase_phys_part(emmcPhysPartAttr);

    /* Delete current cfe devices */
    emmc_delete_cfe_devs(emmcPhysPartAttr, femmcprobe);

    //--------------------------------------------
    // [Step 2-1] Define new partitions
    newPart = (emmcflash_logicalpart_spec_t*)KMALLOC( sizeof(emmcflash_logicalpart_spec_t)*EMMC_MAX_DATA_PARTS, 0 ); 

    if (!newPart) {
        printf("Failed to allocate memory for new partitions!\n"); 
        return -1;
    }
    
    /* clear partition table */
    memset(newPart, 0, sizeof(newPart));

    /* Add primary  gpt hdr partition */
    newPart[num_parts].fp_size = (uint64_t)CFE_GPT_PRIMRY_SIZE;
    strcpy(newPart[num_parts].fp_name, PRIMARY_GPT_HDR_PART_NAME);
    newPart[num_parts++].fp_partition = EMMC_PART_DATA;

    /* Add NVRAM partition */
    newPart[num_parts].fp_size = (uint64_t)EMMC_DFLT_NVRAM_SIZE;
    strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_NVRAM);
    newPart[num_parts++].fp_partition = EMMC_PART_DATA;

    /* Add bootfs, rootfs and metadata partitions */
    for(j=0; j<EMMC_NUM_IMGS; j++ )
    {
        newPart[num_parts].fp_size = (uint64_t)bootfs_sizekb * 1024;
        strcpy(newPart[num_parts].fp_name, (!j?EMMC_PNAME_STR_BOOTFS(1):EMMC_PNAME_STR_BOOTFS(2)));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
        
        newPart[num_parts].fp_size = (uint64_t)rootfs_sizekb * 1024;
        strcpy(newPart[num_parts].fp_name, (!j?EMMC_PNAME_STR_ROOTFS(1):EMMC_PNAME_STR_ROOTFS(2)));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
        
        newPart[num_parts].fp_size = (uint64_t)CFE_GPT_PRIMRY_SIZE;
        strcpy(newPart[num_parts].fp_name, (!j?EMMC_PNAME_STR_MDATA(1,1):EMMC_PNAME_STR_MDATA(2,1)));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
        
        newPart[num_parts].fp_size = (uint64_t)CFE_GPT_PRIMRY_SIZE;
        strcpy(newPart[num_parts].fp_name, (!j?EMMC_PNAME_STR_MDATA(1,2):EMMC_PNAME_STR_MDATA(2,2)));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    }

    /* Add data partition */
    newPart[num_parts].fp_size = (uint64_t)data_sizekb * 1024;
    strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_DATA);
    newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    
    /* Add misc partitions */
    if( misc1_sizekb > 0 )
    {
        newPart[num_parts].fp_size = (uint64_t)misc1_sizekb * 1024;
        strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_MISC(1));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    }

    if( misc2_sizekb > 0 )
    {
        newPart[num_parts].fp_size = (uint64_t)misc2_sizekb * 1024;
        strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_MISC(2));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    }

    if( misc3_sizekb > 0 )
    {
        newPart[num_parts].fp_size = (uint64_t)misc3_sizekb * 1024;
        strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_MISC(3));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    }

    if( misc4_sizekb > 0 )
    {
        newPart[num_parts].fp_size = (uint64_t)misc4_sizekb * 1024;
        strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_MISC(4));
        newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    }

    /* Add unallocated  partition */
    newPart[num_parts].fp_size = (uint64_t)PARTITION_SIZE_FILL_FLASH;
    strcpy(newPart[num_parts].fp_name, EMMC_PNAME_STR_UNALLOC);
    newPart[num_parts++].fp_partition = EMMC_PART_DATA;
    
    /* Add backup gpt hdr partition */
    newPart[num_parts].fp_size = (uint64_t)CFE_GPT_PRIMRY_SIZE;
    strcpy(newPart[num_parts].fp_name, BACKUP_GPT_HDR_PART_NAME);
    newPart[num_parts++].fp_partition = EMMC_PART_DATA;

    /* Sanity Check */
    if( num_parts > EMMC_MAX_DATA_PARTS )
    {
        printf("\n\n !!! eMMC GPT re-partitioning Failed, too many new partitions %d > %d!!! \n\n", num_parts, EMMC_MAX_DATA_PARTS);
        res = CFE_ERR;
    }

    /* Create CFE logical partitions */
    if( (res != CFE_ERR) && enable_emmc_logicalpartition( newPart, femmcprobe, emmcPhysPartAttr, num_parts ) )
    {
        /* Create GPT partitions */
        res = enable_emmc_gpt( pemmcdatagptprobe, femmcprobe, EMMC_PART_DATA, num_parts, 1);
        if( res != CFE_OK )
        {
            printf("\n\n !!! eMMC GPT re-partitioning Failed !!! \n\n");
        }
    }
    else
    {
        printf("eMMC Logical CFE re-partitioning failed!");
    }

    KFREE(newPart);
    KFREE(pemmcdatagptprobe);
    KFREE(femmcprobe);
    
    return res;
}

int emmc_erase_psi(void)
{
    return(emmc_erase_partition(EMMC_CFE_PNAME_DATA));
}

int emmc_erase_all(int leave_blank)
{
    int                 res = 0;
    emmcflash_probe_t   * pemmcdataprobe=  NULL;
    emmcflash_probe_t   * pemmcboot1probe= NULL;
    emmcflash_probe_t   * pemmcboot2probe= NULL;

    cfe_gpt_probe_t * pemmcdatagptprobe = (cfe_gpt_probe_t *)KMALLOC( sizeof(cfe_gpt_probe_t), 0 );

    pemmcdataprobe = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );
    pemmcboot1probe = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );
    pemmcboot2probe = (emmcflash_probe_t *)KMALLOC( sizeof(emmcflash_probe_t), 0 );

    if (!pemmcdatagptprobe || !pemmcdataprobe || !pemmcboot1probe || !pemmcboot2probe ) 
    {
        printf("Failed to allocate memory for re-partitioning!\n"); 
        return -1;
    }

    /* Erase all logical partitions within physical partitions */
    emmc_erase_phys_part(EMMC_PART_DATA);
    emmc_erase_phys_part(EMMC_PART_BOOT1);
    emmc_erase_phys_part(EMMC_PART_BOOT2);

    /* Delete current cfe devices */
    emmc_delete_cfe_devs(EMMC_PART_DATA,  pemmcdataprobe );
    emmc_delete_cfe_devs(EMMC_PART_BOOT1, pemmcboot1probe);
    emmc_delete_cfe_devs(EMMC_PART_BOOT2, pemmcboot2probe);

    if( !leave_blank )
    {
        create_default_partitions( pemmcdataprobe, pemmcboot1probe, pemmcboot2probe, pemmcdatagptprobe );
    }
        
    KFREE(pemmcdatagptprobe);
    KFREE(pemmcdataprobe);
    KFREE(pemmcboot1probe);
    KFREE(pemmcboot2probe);
    
    return res;
}

int emmc_flash_image( PFILE_TAG pTag, uint8_t *imagePtr ) 
{
    int cfeSize;
    uint32_t cfeAddr, rootfsAddr, bootfsAddr, mdataAddr, bootfsSize, rootfsSize;
    uint64_t nvram_offset;
    int status = CFE_ERR; 
    int new_idx = 0;
    int update_nvram = 0;

    char part_name[EMMC_MAX_PART_NAME] = EMMC_CFE_PNAME_CFE;
    NVRAM_DATA *nvramData, *tmpNvramData;
    cfeSize = cfeAddr = rootfsAddr = bootfsAddr = mdataAddr = 0;
    
    /* Allocate memory for NVRAM */
    nvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
    if (!nvramData) {
        goto err_out;
    }
    tmpNvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
    if (!tmpNvramData) {
        goto err_out;
    }

    /* save existing NVRAM data into a local structure */
    NVRAM_COPY_TO(nvramData);

    /* Update sequence number and committed flag in metadata */
    if((status =  emmc_update_image_seq_num( imagePtr, atoi(pTag->totalImageLen) ))< 0) {
        goto err_out;
    }

    /* Retrieve addresses from Tag */
    cfeAddr = atoi(pTag->cfeAddress);
    cfeSize = atoi(pTag->cfeLen);
    rootfsAddr = atoi(pTag->rootfsAddress);
    bootfsAddr = atoi(pTag->bootfsAddress);
    mdataAddr = atoi(pTag->mdataAddress);
    bootfsSize = atoi(pTag->bootfsLen);
    rootfsSize = atoi(pTag->rootfsLen);

    printf("cfe:0x%08x, root:0x%08x, boot:0x%08x, mdata:0x%08x\n", cfeAddr, rootfsAddr, bootfsAddr, mdataAddr);

    /* Check partition and image sizes */
    if((status = emmc_check_image_size(pTag)) < 0)
    {
        if( allow_img_update_repartition )
        {
            printf("Re-partitioning eMMC based on image size!\n");
            allow_img_update_repartition = 0;
            if((status = emmc_format_gpt_dataPhysPart( 
                                (bootfsSize < EMMC_DFLT_BOOTFS_SIZE? EMMC_DFLT_BOOTFS_SIZE:bootfsSize)/1024 + 1,
                                (rootfsSize < EMMC_DFLT_ROOTFS_SIZE? EMMC_DFLT_ROOTFS_SIZE:rootfsSize)/1024 + 1,
                                (EMMC_DFLT_DATA_SIZE)/1024 + 1, 
                                0, 0, 0, 0)) < 0)
                goto err_out;
        }
        else
        {
            printf("Error: Image will not fit into current partitions!\n");
            goto err_out;
        }
    }
    
    /* flash cferom if exists */
    if (cfeSize)
    {
        /* Flash CFEROM image in BOOT partition @ IMAGE_OFFSET */
        if((status = flash_imagedata_to_partition( part_name, imagePtr + cfeAddr, IMAGE_OFFSET, cfeSize)) < 0)
            goto err_out;

        /* Get pointer to new embedded NVRAM data - have to redo this when nvram is nolonger embedded in image */
        nvram_offset = get_nvram_offset((char*)(imagePtr + cfeAddr));
        if( nvram_offset < cfeSize )
            tmpNvramData = (NVRAM_DATA *)((char*)imagePtr + cfeAddr + nvram_offset);
        else
        {
            printf("Error: Cannot find NVRAM in new Image!\n");
            goto err_out;
        }

        /* If current NVRAM is invalid, then this means that eMMC was erased
         * We therefore force the new image to be written to first image 
         * partition and we accept the new NVRAM  
         * */
        if(((unsigned char )nvramData->szBoardId[0]) == 0xff &&
           ((unsigned char )nvramData->szBoardId[1]) == 0xff) 
        {
            new_idx=NP_ROOTFS_1;
#if defined(_BCM963138_) || defined(_BCM963148_)
            strncpy(tmpNvramData->szBoardId, "PROMPT", NVRAM_BOARD_ID_STRING_LEN*sizeof(char));
            tmpNvramData->ulCheckSum = 0;
            tmpNvramData->ulCheckSum = getCrc32((unsigned char *) tmpNvramData, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
#endif
            update_nvram = 1;
        }
        else
        {
            /* If current NVRAM is valid, we only overwrite it if the new
             * NVRAM is also valid
             */
            if( (BpSetBoardId(tmpNvramData->szBoardId) != BP_SUCCESS) ||
                (BpSetVoiceBoardId(tmpNvramData->szVoiceBoardId) != BP_SUCCESS))
            {
                /* New NVRAM data is invalid, keep using old one */
                printf("New NVRAM is invalid, preserving previous NVRAM\n");
            }
            else
            {
                /* New NVRAM data is valid, write to NVRAM partition */
                update_nvram = 1;
            }
        }

        if( update_nvram )
            NVRAM_UPDATE(NULL);
    }
    else
    {
        printf("No CFEROM found! cfe size is 0\n");
    }

    /* Figure out which image partitions to write to */
    if( !new_idx )
        new_idx = get_new_image_idx();

    /* Flash rootfs, bootfs and metadata in eMMC DATA partition */
    if( rootfsAddr && bootfsAddr && mdataAddr )
    {
        printf("Flashing root file system and bootfs at image idx: %d\n", new_idx);

        sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_BOOTFS, new_idx);
        if((status = flash_imagedata_to_partition( part_name, imagePtr + bootfsAddr, 0, atoi(pTag->bootfsLen))) < 0)
            goto err_out;

        sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_ROOTFS, new_idx); 
        if((status = flash_imagedata_to_partition( part_name, imagePtr + rootfsAddr, 0, atoi(pTag->rootfsLen))) < 0)
            goto err_out;

        sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_MDATA, new_idx, 1);
        if((status = flash_imagedata_to_partition( part_name, imagePtr + mdataAddr, 0, atoi(pTag->mdataLen))) < 0)
            goto err_out;

        sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_MDATA, new_idx, 2);
        if((status = flash_imagedata_to_partition( part_name, imagePtr + mdataAddr, 0, atoi(pTag->mdataLen))) < 0)
            goto err_out;
    }

err_out:
    if ( status < 0 )
        printf("\n*** Image flash FAILED! *** !\n");
    else
        printf("\n*** Image flash done *** !\n");

    if (tmpNvramData)
      KFREE(tmpNvramData);
    if (nvramData)
      KFREE(nvramData);
    return status;
}

int emmc_nvram_get(PNVRAM_DATA pNvramData)
{
    return (read_nvram_data(pNvramData));
}


int emmc_nvram_set(PNVRAM_DATA pNvramData)
{
    return (write_nvram_data(pNvramData));
}

int emmc_load_bootfs_file( const char* fname, unsigned int fnsize,
               unsigned char** file, unsigned int* file_size )
{
    int ret = -1;
    int imageNum = 0;
    void * dst_buf = (void*)cfe_get_mempool_ptr();
    if (!dst_buf) {
         return ret;
    }

    /* If we are booting from only image then
     * main boot function will set the boot_only_img_idx
     * this way we dont have to search multiple images
     * for our DTB
     */
    if( boot_only_img_idx )
        imageNum = boot_only_img_idx;

    /* Find dtb file in bootfs partition */
    if( (ret = load_file_from_next_bootfs( (char *)fname, (char *)dst_buf, 0, &imageNum, &parse_emmc_bootfs )) < 0 )
    {
        printf("Unable to load file %s!\n", fname);
    }
    else
    {
       *file = dst_buf;
       *file_size = ret;
       ret = 0;
    }

    if( boot_only_img_idx )
        boot_only_img_idx = 0;

    return ret;

}

void emmc_allow_img_update_repartitions(int val)
{
    /* Set flag to allow repartitions during image updates if required */
    if( val )
        allow_img_update_repartition = 1;
    else
        allow_img_update_repartition = 0;
}

int emmc_get_img_update_repart_web_var( char * var_value, int * length )
{
    *length = sprintf(var_value, "%d", allow_img_update_repartition);
    return 0;
}

#endif /* INC_EMMC_FLASH_DRIVER */
