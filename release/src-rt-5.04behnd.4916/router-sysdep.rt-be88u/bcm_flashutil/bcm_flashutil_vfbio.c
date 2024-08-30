/***********************************************************************
 * <:copyright-BRCM:2007:DUAL/GPL:standard
 *
 *    Copyright (c) 2007 Broadcom
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
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // types
#include <string.h>
#include <stddef.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bcm_hwdefs.h"
#include "genutil_crc.h"
#include "bcm_flashutil.h"
#include "bcm_flashutil_private.h"
#include "bcm_flashutil_vfbio.h"
#include "vfbio_lvm.h"

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

#define VFBIO_CFE_RAM_NAME            NAND_CFE_RAM_NAME
#define VFBIO_CFE_RAM_SECBT_NAME      NAND_CFE_RAM_SECBT_NAME
#define VFBIO_CFE_RAM_SECBT_MFG_NAME  NAND_CFE_RAM_SECBT_MFG_NAME
#define VFBIO_CFE_RAM_NAME_LEN        strlen(VFBIO_CFE_RAM_NAME)
#define VFBIO_CFE_RAM_NAME_CMP_LEN    (strlen(VFBIO_CFE_RAM_NAME) - 3)

#define CRC_LENGTH 4

/* Bits 5, 4, 3 of ext_csd[PARTITION_CONFIG(179)] */
#define VFBIO_BOOT_PART_SHIFT 3
#define VFBIO_BOOT_PART_MASK 0x7

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
 * Function Name: vfbioGetBootPartition
 * Description  : Gets boot partition number
 * Returns      : -1 - failure
 ***********************************************************************/
int vfbioGetBootPartition(void)
{
    char rootfs_devname[256]={0};
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
        return -1;
    }

    /* Read the output a line at a time - output it. */
    fgets(rootfs_devname, sizeof(rootfs_devname), fp);
    pclose(fp);

    if (strstr(rootfs_devname, "1"))
        boot_partition = 1;
    else if (strstr(rootfs_devname, "2"))
        boot_partition = 2;

    cached_boot_partition = boot_partition;
    return boot_partition;
}

/***********************************************************************
 * Function Name: vfbioGetNvramOffset
 * Description  : Get offset to nvram data in inmemory copy of cferom
 * Returns      : -1 - failure
 ***********************************************************************/
int vfbioGetNvramOffset( char * cfe_start_addr )
{
    int nvram_data_offset = 0;
#if NVRAM_PARSING_SUPPORT
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

    /* Add offset to NVRAM data */
    nvram_data_offset += NVRAM_DATA_REL_OFFSET;
#endif /* NVRAM_PARSING_SUPPORT */

    return nvram_data_offset;
}
/***********************************************************************
 * Function Name: vfbioReadNvramDAta
 * Description  : Gets nvram data
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int vfbioReadNvramData(void *nvramData)
{
    int ret = -1;
    FILE *fp;

    fp = fopen(VFBIO_DEV_PNAME_NVRAM, "rw");

    if (fp != NULL)
    {
        if( (fread(nvramData, 1, sizeof(NVRAM_DATA), fp) < sizeof(NVRAM_DATA)) || ferror(fp) )
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
 * Function Name: vfbioGetSequenceNumber
 * Description  : Gets sequence number of image
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int vfbioGetSequenceNumber(int image_number)
{
    int seqNumber = -1;

    if (getImgSeqNum(image_number, &seqNumber))
        printf("%s: Error! Could not retrieve sequence number for image %d\n", __FUNCTION__, image_number);
    return seqNumber;
}

/***********************************************************************
 * Function Name: vfbioGetImageVersion
 * Description  : Gets imageversion
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int vfbioGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name
    , int image_name_len)
{
    int ret = -1;

    printf("%s: Error! function not implemented for VFBIO\n", __FUNCTION__);

    return ret;
}

/***********************************************************************
 * Function Name: vfbioWriteBootImageState
 * Description  : Gets/sets the commit flag of an image.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int vfbioCommit( int partition, char *string )
{
    printf("ERROR!!! This function should not be called for NEW flash layout\n");
    return -1;
}

/***********************************************************************
 * Function Name: vfbioIsBootDevice
 * Description  : Determines whether boot device is vfbio
 * Returns      : 0 - bootdevice is not vfbio, 1 - bootdevice IS vfbio
 ***********************************************************************/
int vfbioIsBootDevice(void)
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
        if( strstr(line, VFBIO_DEV_LINUX_PREFIX) )
            found = 1;
    }
    pclose(fp);
#endif
    return found;
}

/***********************************************************************
 * Function Name: vfbioIsLegacyFlashLayout
 * Description  : Determines whether boot device is using legacy flash layout
 * Returns      : 0 - flashlayout is new
 ***********************************************************************/
int vfbioIsLegacyFlashLayout(void)
{
    return 0;
}


/***********************************************************************
 * Function Name: vfbioGetBootedValue (devCtl_getBootedImagePartition)
 * Description  : Gets the which partition we booted from.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
int vfbioGetBootedValue(void)
{
    int bootPartition = vfbioGetBootPartition();

    if( bootPartition == 1 )
        return BOOTED_PART1_IMAGE;
    else if( bootPartition == 2 )
        return BOOTED_PART2_IMAGE;
    else
        return -1;
}

/***********************************************************************
 * Function Name: vfbioVerifyImageDDRType
 * Description  : Verify if image ddr type match board(nvram) ddr type.
 * Returns      : 0 success or -1 for failure
 ***********************************************************************/
int vfbioVerifyImageDDRType(uint32_t imageFlags)
{
    NVRAM_DATA nvram;
    int ret = -1;

    if( vfbioReadNvramData(&nvram) == 0 )
        ret = verifyImageDDRType(imageFlags, &nvram);

    return ret;
}

static uint64_t vfbioGetPartSize( char * full_dev_path )
{
    char cmd[128];
    char size[128];
    uint64_t size_bytes = 0;
    int64_t temp;
    unsigned int temp2;
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

    temp = strtoull(size, NULL, 10);
    getFlashTotalSize(&temp2);
    if ((temp > 0) && (temp <= ((uint64_t)temp2 * 1024)))
        size_bytes += (uint64_t)temp;

    /* close */
    pclose(fp);
    return size_bytes;
}

uint64_t vfbioGetAvailLoaderSpace( void )
{
    char dev_name[32];
    uint64_t space = 0;

    snprintf(dev_name, sizeof(dev_name)- 1, VFBIO_DEV_PNAME_FMT_ARMBL, 1);
    space += vfbioGetPartSize(dev_name);
    snprintf(dev_name, sizeof(dev_name)- 1, VFBIO_DEV_PNAME_FMT_MEMINIT, 1);
    space += vfbioGetPartSize(dev_name);
    snprintf(dev_name, sizeof(dev_name)- 1, VFBIO_DEV_PNAME_FMT_SMCOS, 1);
    space += vfbioGetPartSize(dev_name);
    snprintf(dev_name, sizeof(dev_name)- 1, VFBIO_DEV_PNAME_FMT_SMCBL, 1);
    space += vfbioGetPartSize(dev_name);
    return space;
}

uint64_t vfbioGetAvailImgSpace( int update_img_idx )
{
    uint64_t dev_size_total=0, dev_size_avail=0;
    uint64_t size_bytes;
    char bootfs_dev_name[32];
    char rootfs_dev_name[32];

    snprintf(bootfs_dev_name, sizeof(bootfs_dev_name)- 1, VFBIO_DEV_PNAME_FMT_BOOTFS, update_img_idx);
    snprintf(rootfs_dev_name, sizeof(rootfs_dev_name)- 1, VFBIO_DEV_PNAME_FMT_ROOTFS, update_img_idx);

    /* Add available size on device since images are in dynamic LUNs that can be recreated */
    vfbio_device_get_info(&dev_size_total, &dev_size_avail);

    size_bytes = vfbioGetPartSize(bootfs_dev_name);
    size_bytes += vfbioGetPartSize(rootfs_dev_name);
    size_bytes += dev_size_avail;

    return size_bytes;
}

int vfbioGetFlashSize(unsigned int *psize __attribute__((unused)))
{
    uint64_t total_size = 0, free_size;
    int rc;

    rc = vfbio_device_get_info(&total_size, &free_size);
    *psize = (unsigned int)total_size;

    return rc;
}

int vfbioWriteImage(int lun_id, uint8_t *imagePtr, int imageSize)
{
    return vfbio_lun_write(lun_id, imagePtr, imageSize);
}
