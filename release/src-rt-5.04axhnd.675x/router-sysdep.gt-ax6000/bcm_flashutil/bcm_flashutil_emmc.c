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
#include <stddef.h>
#include <net/if.h>


#include "bcm_hwdefs.h"
#include "emmc_linux_defs.h"
#include "genutil_crc.h"
#include "bcm_flashutil.h"
#include "bcm_flashutil_private.h"
#include "bcm_flashutil_emmc.h"

#include <sys/ioctl.h>
#include <unistd.h> // close

#include "bcmTag.h" /* in shared/opensource/include/bcm963xx, for FILE_TAG */
#include "board.h" /* in bcmdrivers/opensource/include/bcm963xx, for BCM_IMAGE_CFE */
#include "flash_api.h"

#include <fcntl.h> // for open
#include <linux/errno.h>
#include <linux/kernel.h>
#include <asm/byteorder.h>
#include <asm/setup.h>

#define NVRAM_PARSING_SUPPORT  0    /* Leave this disabled: Image update does not modify NVRAM */
#if NVRAM_PARSING_SUPPORT    
#include "../../../../cfe/cfe/board/common/include/bcm_btrm_common.h"
#endif

#define IS_ERR_OR_NULL(x) ((x)==0)
#define IS_ERR(x) ((x)<0)
#define IS_NULL(x) ((x)==0)

#define ERROR -1
#define SUCCESS 0

#define EMMC_CFE_RAM_NAME            NAND_CFE_RAM_NAME            
#define EMMC_CFE_RAM_SECBT_NAME      NAND_CFE_RAM_SECBT_NAME
#define EMMC_CFE_RAM_SECBT_MFG_NAME  NAND_CFE_RAM_SECBT_MFG_NAME
#define EMMC_CFE_RAM_NAME_LEN        strlen(EMMC_CFE_RAM_NAME)
#define EMMC_CFE_RAM_NAME_CMP_LEN    (strlen(EMMC_CFE_RAM_NAME) - 3)

#define CRC_LENGTH 4

/* Bits 5, 4, 3 of ext_csd[PARTITION_CONFIG(179)] */
#define EMMC_BOOT_PART_SHIFT 3
#define EMMC_BOOT_PART_MASK 0x7

#define be16_to_cpu(x) __be16_to_cpu(x)
#define be32_to_cpu(x) __be32_to_cpu(x)
#define cpu_to_be32(x) __cpu_to_be32(x)

/* This struct describes the fixed header of a bootfs entry */
typedef struct
{
    unsigned int nextEntryOffset;   // Relative to this field
    unsigned int dataOffset;        // Relative to this field
    unsigned int dataLength;
    unsigned int hdrCrc;
} BOOTFS_ENTRY_FIXED_HDR;

typedef enum
{
    BOOTFS_DATA_READ,
    BOOTFS_DATA_WRITE,
} BOOTFS_DATA_OP;


/***********************************************************************
 * Function Name: metadata_dup
 * Description  : retrieve metadata from file in metadata bootfs
 * Returns      : Null - failure
 * Note         : Caller must free allocated metadata memory 
 ***********************************************************************/
static char * metadata_dup ( int imageNumber, int index , int * metadata_size)
{
    FILE * fp;
    char * metadata_ptr = NULL;
    char mdata_dev_name[] = EMMC_DEV_PNAME_FMT_STR_MDATA;

    sprintf(mdata_dev_name, EMMC_DEV_PNAME_FMT_STR_MDATA, imageNumber, index);
    fp = fopen(mdata_dev_name, "r");

    if( !fp )
    {
        printf("%s: Error opening %s!\n", __FUNCTION__, mdata_dev_name);
        return metadata_ptr;
    }

    /* Determine size of metadata */
    fseek(fp, 0L, SEEK_END);
    *metadata_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    /* Allocate buffer to hold metadata */
    metadata_ptr = malloc(*metadata_size);
    if( !metadata_ptr )
    {
        printf("%s: Error allocating memory for metadata\n", __FUNCTION__);
        return metadata_ptr;
    }

    /* Read all metadata into memory */
    fread(metadata_ptr, 1, *metadata_size, fp);
    if( ferror(fp) )
    {
        printf("%s: Error reading %s!\n", __FUNCTION__, mdata_dev_name);
        perror("metadata read error");
        free(metadata_ptr);
        metadata_ptr = NULL;
    }
    fclose(fp);

    return metadata_ptr;
}

/***********************************************************************
 * Function Name: metadata_commit_all
 * Description  : write metadata to all metadata partitions
 * Returns      : -1 - failure
 * Note         : Caller must free allocated metadata memory 
 ***********************************************************************/
static int metadata_commit_all ( int imageNumber, char * metadata_ptr, int metadata_size)
{
    FILE * fp;
    int i;
    char mdata_dev_name[] = EMMC_DEV_PNAME_FMT_STR_MDATA;
    int ret = -1;

    for( i=EMMC_IMG_IDX_START; i<=EMMC_NUM_MDATA; i++ )
    {
        sprintf(mdata_dev_name, EMMC_DEV_PNAME_FMT_STR_MDATA, imageNumber, i);

        fp = fopen(mdata_dev_name, "r+");

        if( !fp )
        {
            printf("%s: Error opening %s!\n", __FUNCTION__, mdata_dev_name);
            return ret;
        }

        fseek(fp, 0L, SEEK_SET);

        /* write metadata to partition */
        fwrite(metadata_ptr, 1, metadata_size, fp);
        if( ferror(fp) )
        {
            printf("%s: Error writing %s !\n", __FUNCTION__, mdata_dev_name);
	    perror("metadata write error");
        }
        else
            ret = 0;

        fclose(fp);
    }

    return ret;
}
/***********************************************************************
 * Function Name: rw_inmemory_metadata_data
 * Description  : retrieve data from metadata in memory
 * Returns      : -1 - failure
 ***********************************************************************/
static int rw_inmemory_metadata_data(  char * metadata_ptr, char * filename, char * data
    , uint32_t data_len, BOOTFS_DATA_OP op)
{
    BOOTFS_ENTRY_FIXED_HDR bootfs_entry_hdr;
    uint8_t * bootfs_entry_data_ptr;
    uint8_t * bootfs_entry_data_crc_ptr;
    uint8_t filename_match = 0;
    uint32_t bootfs_hdr_crc;
    uint32_t data_crc = 0;
    BOOTFS_ENTRY_FIXED_HDR * bootfs_entry_ptr;
    
    bootfs_entry_ptr = (BOOTFS_ENTRY_FIXED_HDR *)metadata_ptr;

    /* Search for matching bootfs entry */
    do
    {
        /* Copy over fixed header to avoid any misaligned fields */
        memcpy(&bootfs_entry_hdr, bootfs_entry_ptr, sizeof(BOOTFS_ENTRY_FIXED_HDR));

        /* check if valid entry */
        if( !be32_to_cpu(bootfs_entry_hdr.nextEntryOffset) )
        {
            printf("%s: Bootfs entry update failed! Matching bootfs entry not found", __FUNCTION__);
            return -1;
        }

        /* Check if hdr crc is correct */
        bootfs_hdr_crc = CRC32_INIT_VALUE;
        bootfs_hdr_crc = genUtl_getCrc32( (void *)bootfs_entry_ptr, 
                offsetof(BOOTFS_ENTRY_FIXED_HDR,hdrCrc), bootfs_hdr_crc );                                        
        if( be32_to_cpu(bootfs_entry_hdr.hdrCrc) != bootfs_hdr_crc )
        {
            printf("%s: Bootfs entry update failed! Corrupt hdr_crc: 0x%08x, Expected: 0x%08x\n",
                    __FUNCTION__, bootfs_hdr_crc, be32_to_cpu(bootfs_entry_hdr.hdrCrc));
            return -1;
        }

        /* See if filename matches */
        if( strcmp( filename, (char *)bootfs_entry_ptr 
                + sizeof(BOOTFS_ENTRY_FIXED_HDR)) == 0 )
        {
            filename_match = 1;
        }
        else
        {
            /* Move on to the next entry */
            bootfs_entry_ptr = 
                    (BOOTFS_ENTRY_FIXED_HDR *)((char *)&bootfs_entry_ptr->dataOffset 
                    + be32_to_cpu(bootfs_entry_hdr.nextEntryOffset));
        }

    } while ( !filename_match );

    /* Check if data_length matches, updates cannot change the data length */
    if( data_len != be32_to_cpu(bootfs_entry_hdr.dataLength) )
    {
        printf("%s: Bootfs entry update failed! Incompatible data_length: %d, Required length: %d\n",
                __FUNCTION__, data_len, be32_to_cpu(bootfs_entry_hdr.dataLength));
        return -1;
    }

    bootfs_entry_data_ptr = (uint8_t *)&(bootfs_entry_ptr->dataLength) 
            + be32_to_cpu(bootfs_entry_hdr.dataOffset);
    bootfs_entry_data_crc_ptr = bootfs_entry_data_ptr 
            + be32_to_cpu(bootfs_entry_hdr.dataLength);
    data_crc = CRC32_INIT_VALUE;

    /* Data Write: Update data field */
    if( op == BOOTFS_DATA_WRITE )
    {
        memcpy(bootfs_entry_data_ptr, data,be32_to_cpu(bootfs_entry_hdr.dataLength));
    }

    /* Compute data crc over filename, padding and data */
    data_crc = cpu_to_be32(genUtl_getCrc32( (uint8_t*)bootfs_entry_ptr 
            + sizeof(BOOTFS_ENTRY_FIXED_HDR),
            (uint32_t)((uint8_t *)bootfs_entry_data_crc_ptr 
            - ((uint8_t*)bootfs_entry_ptr + sizeof(BOOTFS_ENTRY_FIXED_HDR))),
            data_crc));

    if( op == BOOTFS_DATA_WRITE )
    {
        /* Data Write: Write the updated crc */
        *(unsigned int*)(bootfs_entry_data_crc_ptr) = data_crc;
    }
    else
    {
        /* Data Read: Check if data crc matches */
        if( data_crc == *(unsigned int*)bootfs_entry_data_crc_ptr )
        {
            /* Read data field */
            memcpy(data, bootfs_entry_data_ptr,
                    be32_to_cpu(bootfs_entry_hdr.dataLength));
        }
        else
        {
            printf("%s: Bootfs entry read failed! CRC mismatch: expected:0x%08x, actual:0x%08x\n",
                    __FUNCTION__, 
                    *(unsigned int*)bootfs_entry_data_crc_ptr,
                    data_crc);
            return -1;
        }
            
    }
    
    return 0; 
}

/***********************************************************************
 * Function Name: rw_inmemory_metadata_data
 * Description  : retrieve data from metadata in filesystem
 * Returns      : -1 - failure
 ***********************************************************************/
static int rw_metadata_data( int image_number, char * filename, char * data, uint32_t data_len, BOOTFS_DATA_OP op)
{
    int i;
    char * metadata_ptr = NULL;
    int metadata_size = 0;
    int ret = -1;

    /* Try both copies of metadata */
    for( i=EMMC_IMG_IDX_START; i<=EMMC_NUM_MDATA; i++ )
    {
        /* retrieve metadata from file */
        metadata_ptr = metadata_dup(image_number, i, &metadata_size);

        if( metadata_ptr )
        {
            /* Get/set data from/to inmemory metadata */
            ret = rw_inmemory_metadata_data(  metadata_ptr, filename, data, data_len, op);

            if( (ret == 0) && (op == BOOTFS_DATA_WRITE))
            {
                /* write metadata back to file */
                ret = metadata_commit_all( image_number, metadata_ptr, metadata_size );
            }

            /* Free malloced metadata */
            free(metadata_ptr);
            metadata_ptr = NULL;

            /* Early exit if we were successfull */
	    if( ret == 0 )
                break;
        }
    }

    return ret;
}

/***********************************************************************
 * Function Name: emmcGetBootPartition
 * Description  : Gets boot partition number
 * Returns      : -1 - failure
 ***********************************************************************/
int emmcGetBootPartition(void)
{
    char rootfs_devname[256]={0};
    char rootfs_devname1[256]={0};
    char * rootfs_devidx;    
    char * rootfs_devidx1;
    int len = 0;
    int boot_partition = -1;
    static int cached_boot_partition = -1;
    FILE *fp;

    if (cached_boot_partition != -1)
    {
        return cached_boot_partition;
    }

    /* Open the command for reading. */
    fp = popen("/etc/get_rootfs_dev.sh", "r");
    if (fp == NULL) {
    	printf("Failed to run /etc/get_rootfs_dev.sh command\n" );
    	return 0;
    }
    
    /* Read the output a line at a time - output it. */
    fgets(rootfs_devname, sizeof(rootfs_devname), fp);
    len = strlen(rootfs_devname);
    pclose(fp);

    if( len )
    {
        len = readlink(EMMC_DEV_PNAME_ROOTFS(1), rootfs_devname1
            , sizeof(rootfs_devname1)-1);
        if( len )
        {
            /* Move pointer to start of mmc device index */
            rootfs_devidx  = strstr(rootfs_devname , EMMC_DEV_LINUX_PREFIX);
            rootfs_devidx1 = strstr(rootfs_devname1, EMMC_DEV_LINUX_PREFIX);
            if( rootfs_devidx && rootfs_devidx1 )
            {
                if( strcmp(rootfs_devidx, rootfs_devidx1) == 0 )
                {
                    /* /dev/root and /dev/rootfs1 point to same device */
                    boot_partition = 1;
                }
                else
                    boot_partition = 2;
            }
        }
    }

    cached_boot_partition = boot_partition;
    return boot_partition;
}

/***********************************************************************
 * Function Name: emmcGetNvramOffset
 * Description  : Get offset to nvram data in inmemory copy of cferom
 * Returns      : -1 - failure
 ***********************************************************************/
int emmcGetNvramOffset( char * cfe_start_addr )
{
    int nvram_data_offset = 0;
#if NVRAM_PARSING_SUPPORT    
#if defined(CONFIG_BCM963138) 
    /* Gen2 Image - fixed header sizes*/
    nvram_data_offset =  BTRM_SBI_SIGNED_COT_ELEM_SIZE * 2
                       + BTRM_SBI_AUTH_HDR_SIZE       
                       + BTRM_SBI_UNAUTH_HDR_SIZE;
#else		       
    /* Gen3 Image - variable header sizes*/
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
#endif /* NVRAM_PARSING_SUPPORT */   

    return nvram_data_offset;
}

/***********************************************************************
 * Function Name: emmcUpdateMdataSeqnum
 * Description  : Updates sequencenumber either in memory or in filesystem
 * Args         : (optional)mdata_ptr - pointer to inmemory metadata
 *              : (optional)image_num - image number of target partition
 *              : seq_num - sequence number
 * Returns      : -1 - failure
 ***********************************************************************/
int emmcUpdateMdataSeqnum( char * mdata_ptr, int image_num, int seq_num )
{
    int ret = -1;
    char fname[] = EMMC_CFE_RAM_NAME;
    char data[4] = {0};

    sprintf(data, "%03d", seq_num);

    if( mdata_ptr )
    {
        ret = rw_inmemory_metadata_data(  mdata_ptr, fname, data, 3, BOOTFS_DATA_WRITE);
    }
    else if( image_num )
    {
        ret = rw_metadata_data( image_num, fname, data, 3, BOOTFS_DATA_WRITE);
    }

    return ret;
}

/***********************************************************************
 * Function Name: emmcUpdateMdataCommitFlag
 * Description  : Updates commit flag either in memory or in filesystem
 * Args         : (optional)mdata_ptr - pointer to inmemory metadata
 *              : (optional)image_num - image number of target partition
 *              : flag - flag value
 * Returns      : -1 - failure
 ***********************************************************************/
int emmcUpdateMdataCommitFlag(char * mdata_ptr, int image_num, int flag )
{
    int ret = -1;
    char fname[] = "committed";
    char data[4] = {0};

    sprintf(data, "%d", flag);

    if( mdata_ptr )
    {
        ret = rw_inmemory_metadata_data(  mdata_ptr, fname, data, strlen(data), BOOTFS_DATA_WRITE);
    }
    else if( image_num )
    {
        ret = rw_metadata_data( image_num, fname, data, strlen(data), BOOTFS_DATA_WRITE);
    }

    return ret;
}

/***********************************************************************
 * Function Name: emmcReadNvramDAta
 * Description  : Gets nvram data
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int emmcReadNvramData(void *nvramData)
{
    int ret = -1;
    FILE *fp;

    fp = fopen(EMMC_DEV_PNAME_NVRAM, "rw");

    if (fp != NULL)
    {
        fread(nvramData, 1, sizeof(NVRAM_DATA), fp);

        if( ferror(fp) )
            printf("%s: Error reading NVRAM data\n", __FUNCTION__);
        else
            ret = 0;
            
        fclose(fp);
    }
    else
        printf("%s: Error opening NVRAM device\n", __FUNCTION__);

    return ret;
}

/***********************************************************************
 * Function Name: emmcGetSequenceNumber
 * Description  : Gets sequence number of image
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int emmcGetSequenceNumber(int image_number)
{ 
    char fname[] = EMMC_CFE_RAM_NAME;
    char seq_num[4] = {0};
    int seqNumber = -1;
    int ret = -1;

    if( !emmcIsLegacyFlashLayout() )
    {
        if (getImgSeqNum(image_number, &seqNumber))
            printf("%s: Error! Could not retrieve sequence number for image %d\n", __FUNCTION__, image_number);
        return seqNumber;
    }

    ret = rw_metadata_data( image_number, fname, seq_num, 3, BOOTFS_DATA_READ);
    if( ret == 0 )
    {
        seqNumber = atoi(seq_num);
    }
    else
    {
        printf("%s: Error! Could not retrieve sequence number for image %d\n", __FUNCTION__, image_number);
    }
        
    return(seqNumber);
}

/***********************************************************************
 * Function Name: emmcGetImageVersion
 * Description  : Gets imageversion
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int emmcGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name
    , int image_name_len)
{    
    int ret = -1;

    printf("%s: Error! function not implemented for EMMC\n", __FUNCTION__);

    return ret;
}

/***********************************************************************
 * Function Name: emmcWriteBootImageState
 * Description  : Gets/sets the commit flag of an image.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int emmcCommit( int partition, char *string )
{
    int ret = -1;
    char fname[] = "committed";

    if( emmcIsLegacyFlashLayout())
    {
        ret = rw_metadata_data(partition, fname, string, 1, (*string == 0) ? BOOTFS_DATA_READ : BOOTFS_DATA_WRITE);

        if( ret )
            printf("%s: Error! Could not access commit flag for image %d\n", __FUNCTION__, partition);
    }
    else
    {
	    printf("ERROR!!! This function should not be called for NEW flash layout\n");
    }

    return( ret );
}

/***********************************************************************
 * Function Name: emmcUpdateSequenceNumber
 * Description  : updates the sequence number on the specified partition
 *                to be the highest.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int emmcUpdateSequenceNumber(int incSeqNumPart, int seqPart1, int seqPart2)
{
    int ret = -1;

    if( !emmcIsLegacyFlashLayout() )
    {
        printf("%s: WARNING:Function not implemented for new flash layout\n", __FUNCTION__);
    }

    printf("%s: Error! function not implemented for EMMC\n", __FUNCTION__);

    return( ret );
}

/***********************************************************************
 * Function Name: emmcIsBootDevice
 * Description  : Determines whether boot device is emmc
 * Returns      : 0 - bootdevice is not emmc, 1 - bootdevice IS emmc 
 ***********************************************************************/
int emmcIsBootDevice(void)
{
    int found = 0;
#ifndef DESKTOP_LINUX
    FILE *fp;
    char line[COMMAND_LINE_SIZE]={0};
    fp = popen("/etc/get_rootfs_dev.sh", "r");
    if (fp == NULL) {
    	printf("Failed to run /etc/get_rootfs_dev.sh command\n" );
    	return 0;
    }

    if(fgets(line, sizeof(line), fp))
    {
        if( strstr(line, "mmcblk") )
            found = 1;
    }
    pclose(fp);
#endif
    return found;
}

/***********************************************************************
 * Function Name: emmcIsLegacyFlashLayout
 * Description  : Determines whether boot device is using legacy flash layout
 * Returns      : 1 - flashlayout is legacy 
 ***********************************************************************/
int emmcIsLegacyFlashLayout(void)
{
    FILE *file = NULL;
    file = fopen("/dev/nvram", "r");
    if(file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}


/***********************************************************************
 * Function Name: emmcGetBootedValue (devCtl_getBootedImagePartition)
 * Description  : Gets the which partition we booted from.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int emmcGetBootedValue(void)
{
    int bootPartition = emmcGetBootPartition();

    if( bootPartition == 1 )
        return BOOTED_PART1_IMAGE;
    else if( bootPartition == 2 )
        return BOOTED_PART2_IMAGE;
    else
        return -1;
}

/***********************************************************************
 * Function Name: emmcVerifyImageDDRType
 * Description  : Verify if image ddr type match board(nvram) ddr type.
 * Returns      : 0 success or -1 for failure
 ***********************************************************************/
int emmcVerifyImageDDRType(uint32_t imageFlags)
{
    NVRAM_DATA nvram;
    int ret = -1;

    if( emmcReadNvramData(&nvram) == 0 )
        ret = verifyImageDDRType(imageFlags, &nvram);

    return ret;
}

static uint64_t emmcGetPartSize( char * full_dev_path )
{
	char cmd[128];
	char size[128];
	uint64_t size_bytes = 0;
	FILE *fp;

	sprintf(cmd, "blockdev --getsize64 %s",full_dev_path);

	/* Open the command for reading. */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		return size_bytes;
	}
	
	/* Read the output a line at a time - output it. */
	fgets(size, sizeof(size), fp);
	size_bytes += strtoull(size, NULL, 10);
	
	/* close */
	pclose(fp);	
	return size_bytes;
}

int emmcGetBootPartIndex( void ) {
	char cmd[128];
	char part_conf_str[128];
	char * part_conf_val = NULL;
	int boot_part_index = 0; //Default points to first boot partition
	FILE *fp;

	sprintf(cmd, "mmc extcsd read /dev/mmcblk0 | grep PARTITION_CONFIG");

	/* Open the command for reading. */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		return boot_part_index;
	}
	
	/* Read the output a line at a time - output it. */
	fgets(part_conf_str, sizeof(part_conf_str), fp);
        part_conf_val = strstr(part_conf_str, "0x");
	if( part_conf_val ) {
		boot_part_index = (strtoull(part_conf_val, NULL, 16) >> EMMC_BOOT_PART_SHIFT) 
					& EMMC_BOOT_PART_MASK;
	}

	/* Remap boot part number (1-based) to linux index (0-based) */
	if( boot_part_index ) 
		boot_part_index--;
	
	/* close */
	pclose(fp);	
	return boot_part_index;
}

uint64_t emmcGetAvailLoaderSpace( void ) {
	char full_part_name[128];
	uint64_t size_bytes = 0;

	sprintf(full_part_name, "/dev/mmcblk0boot%d", emmcGetBootPartIndex());
	size_bytes = emmcGetPartSize(full_part_name);

	return size_bytes;
}

uint64_t emmcGetAvailImgSpace( int update_img_idx )
{
	char full_part_name[128];
	char * partitions[] = {"bootfs","rootfs"};
	uint64_t size_bytes = 0;

        //FIXME: Magic numbers
	for( int i=0; i<2; i++ )
	{
		sprintf(full_part_name, "/dev/%s%d",partitions[i], update_img_idx);
		size_bytes += emmcGetPartSize(full_part_name);
	}
	return size_bytes;
}

int emmcGetFlashSize(unsigned int *psize __attribute__((unused)))
{ 
    char sysfs_path[128];
    uint64_t mmcblk0_size = 0;
    uint64_t mmcblk0boot0_size = 0;
    uint64_t mmcblk0boot1_size = 0;
    uint64_t mmcblk0rpmb = 0;

    sprintf(sysfs_path, "/sys/class/block/mmcblk0/size"); 
    mmcblk0_size = getSysfsBytes(sysfs_path);
    sprintf(sysfs_path, "/sys/class/block/mmcblk0boot0/size");
    mmcblk0boot0_size = getSysfsBytes(sysfs_path);
    sprintf(sysfs_path, "/sys/class/block/mmcblk0boot1/size"); 
    mmcblk0boot1_size = getSysfsBytes(sysfs_path);
    sprintf(sysfs_path, "/sys/class/block/mmcblk0rpmb/size");
    mmcblk0rpmb = getSysfsBytes(sysfs_path);

    *psize = (512*(mmcblk0_size+mmcblk0boot0_size+mmcblk0boot1_size+mmcblk0rpmb)/1024);
    return 0;
}


int getEmmcMetadata( char * data, int size , int mdata_idx)
{
    char mdata_fname[128];
    FILE * mdata_fh;
    int num_bytes = 0;
    sprintf(mdata_fname, "/dev/metadata%d", mdata_idx);
    mdata_fh = fopen(mdata_fname, "r");
    
    if( mdata_fh )
    {
        num_bytes = fread(data, 1, size, mdata_fh);

        if( ferror(mdata_fh) )
        {
            printf("%s: Error fread metadata%d!\n", __FUNCTION__, mdata_idx);
            num_bytes = 0;
        }

        fclose(mdata_fh);
    }
    else
    {
        printf("%s: Error fopen metadata%d!\n", __FUNCTION__, mdata_idx);
    }

    return num_bytes;
}

int setEmmcMetadata( char * data, int size, int mdata_idx)
{
    FILE * fp;
    char mdata_dev_name[] = EMMC_DEV_PNAME_FMT_STR_MDATA;
    int ret = -1;

    sprintf(mdata_dev_name, "/dev/metadata%d", mdata_idx);

    fp = fopen(mdata_dev_name, "r+");

    if( !fp )
    {
        printf("%s: Error opening %s!\n", __FUNCTION__, mdata_dev_name);
        return ret;
    }

    fseek(fp, 0L, SEEK_SET);

    /* write metadata to partition */
    fwrite(data, 1, size, fp);
    if( ferror(fp) )
    {
        printf("%s: Error writing %s !\n", __FUNCTION__, mdata_dev_name);
        perror("metadata write error");
    }
    else
        ret = 0;

    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    return ret;
}
