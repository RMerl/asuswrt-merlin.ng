/*  *********************************************************************
    *
    <:copyright-BRCM:2017:proprietary:standard
    
       Copyright (c) 2017 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :> 
    ********************************************************************* */

#include "rom_main.h"
#include "rom_parms.h"
#if !defined (_BCM960333_)
#include "flash_api.h"
#include "jffs2.h"
#include "bcm_ubi.h"
#endif
#include "bcm_otp.h"
#include "lib_byteorder.h"
#include "bcm_bootimgsts.h"
#include "initdata.h"
#include "bcm63xx_sec.h"

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1) || (BOOT_PRE_CFE==1)
extern int nand_flash_get_sector_size(unsigned short sector);
extern int nand_flash_get_numsectors(void);
extern int nand_flash_read_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len);
#define flash_get_sector_size nand_flash_get_sector_size 
#endif

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
#include "bcm63xx_utils.h"
#endif

#if ((INC_NAND_FLASH_DRIVER==1) || (BOOT_PRE_CFE==1)) && !defined(JTAG_DOWNLOAD)
static int strap_check_nand(void)
{

#if (defined(_BCM963138_) && !(INC_SPI_PROG_NAND==1))
    return 1; // special case for chip bug for BCM963138 
#elif defined(_BCM947189_)
        GCI_BASE->status_idx_reg_wr=GCI_CHIP_STATUS_REGISTER_INDEX_7;
        if((GCI_BASE->status_idx_reg_rd & BOOT_MODE_MASK) == BOOT_MODE_NAND)
                return 1;
        else
                return 0;
#elif defined (_BCM963268_)
    return ( ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL_MASK) >>
        MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_BUS_BOOT_NAND );

#elif defined(_BCM96838_)
    return ( (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_BOOT_CFG_MASK) >> GPIO_PER_STRAP_BUS_BOOT_CFG_SHIFT) != GPIO_PER_STRAP_BUS_SPI_3_BOOT) &&
             (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_BOOT_CFG_MASK) >> GPIO_PER_STRAP_BUS_BOOT_CFG_SHIFT) != GPIO_PER_STRAP_BUS_SPI_4_BOOT)  );

#elif (defined(_BCM963138_) || defined(_BCM963381_) || defined(_BCM963148_))
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SPI_NOR) != MISC_STRAP_BUS_BOOT_SPI_NOR);
#elif defined(_BCM96848_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) != MISC_STRAP_BUS_BOOT_SPI_NOR);
#elif defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND);
#elif defined(_BCM96858_)
    {
        uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                         ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);

        if ( ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) != BOOT_SEL_STRAP_SPI_NOR) &&
             ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) != BOOT_SEL_STRAP_EMMC) )
            return 1;
        else
            return 0;
    }
#else
    return 0;

#endif 
}

#if (INC_SPI_NAND_DRIVER==1)
extern int spi_nand_init(flash_device_info_t **flash_info);
extern int spi_nand_get_sector_size(unsigned short sector);
extern int spi_nand_get_numsectors(void);

extern int spi_nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len);

int strap_check_spinand(void)
{
#if defined(_BCM96838_)
    return ( (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_BOOT_CFG_MASK) >> GPIO_PER_STRAP_BUS_BOOT_CFG_SHIFT) == GPIO_PER_STRAP_BUS_SPI_4_BOOT) &&
             (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_PAGE_SIZE_MASK) >> GPIO_PER_STRAP_BUS_PAGE_SIZE_SHIFT) & GPIO_PER_STRAP_BUS_SPI_NAND_BOOT) );

#elif defined(_BCM963381_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_SPI_NAND_DISABLE) == 0);

#elif defined(_BCM963138_)
    return ( (MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK) &&
            ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND) );

#elif defined(_BCM96848_)
    return (MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SPI_NAND);

#elif defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND);
#elif defined(_BCM96858_)
    {
        uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                         ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);

        return ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_SPI_NAND);
    }
#else
    return 0;

#endif
}
#endif
#endif

#if ((INC_NAND_FLASH_DRIVER==1) || (BOOT_PRE_CFE==1)) && !defined(JTAG_DOWNLOAD)
/* Find uncompressed file cferam.bin on the JFFS2 file system, load it into
 * memory and jump to its entry point function.
 */
int nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len)
{
#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
        return(spi_nand_read_buf(blk, offset, buffer, len));
#endif
    if (strap_check_nand())
        return(nand_flash_read_buf(blk, offset, buffer, len));

    return(FLASH_API_ERROR);
}


#if (BOOT_PRE_CFE==0)
extern unsigned int lib_get_crc32(unsigned char *pdata, unsigned int size, unsigned int crc);
#define getCrc32 lib_get_crc32

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

char g_ubi_seqno_id[] = NAND_CFE_RAM_NAME;
int g_fname_actual_len;
int g_fname_cmp_len;

char g_bname[] = NAND_BOOT_STATE_FILE_NAME;
int g_bname_actual_len = sizeof(g_bname) - 1;
int g_bname_cmp_len = sizeof(g_bname) - 2; /* last one is digit */

static unsigned int read_blk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(nand_read_buf(block, offset, buf + offset, amount));
}

/*
*  A decision to override has to be made in a bit 29 of the parameter
*  1 to override 0 otherwise 
*  bits 0-15 hold an image selector
*  This functionality is for future implementation. 
*  Currently, when overriden 
*  previous image is forced to be selected
* 
*/
static void bootImageFromNand(unsigned long rom_param, cfe_rom_media_params *media_params)
{
    const int bv_invalid = -1;
    const int max_not_jffs2 = MAX_NOT_JFFS2_VALUE;
    int read_len;
    int sec_should_decrypt __attribute__((unused)) = 1;
    int sec_should_decompress  __attribute__((unused)) = 1;

    struct rootfs_info
    {
        int rootfs;
        int start_blk;
        int end_blk;
        int type;
        int committed;
        int boot_val;
        unsigned int ino;
        unsigned int blk;
        unsigned int mctime;
    } rfs_info[2], *prfs_info[2], *rfsi;

#if (CFG_COPY_PSRAM==1)
    unsigned char *buf = (unsigned char *) ((mem_heapstart & 0x0000ffff) | DRAM_BASE);
#else
#if defined(_BCM963148_) || defined(_BCM963138_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
    /* 631x8 run cfe rom in internal memory which is only 512KB, switch heap to ddr 
       in case nand block size is 512KB or larger */
    unsigned char *buf = (unsigned char *) DRAM_BASE + 0x1000;
#else
    unsigned char *buf = (unsigned char *) mem_heapstart;
#endif
#endif

    uint32_t version = 0;
    struct jffs2_raw_dirent *pdir;
    struct jffs2_raw_inode *pino;
    unsigned char *p;
    int i, j, k, done, not_jffs2;
    int num_blks;
    int len, try = 0;
    int boot_prev;
    PNVRAM_DATA nd;
    struct ubi_ec_hdr *ec = (struct ubi_ec_hdr *) buf;
    int jffs2_boot_state = 0;

    g_fname_actual_len = strlen(media_params->boot_file_name);
    g_fname_cmp_len = strlen(media_params->boot_file_name) - 3; /* last three are digits */

#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
    {
        num_blks = spi_nand_get_numsectors();
        len = spi_nand_get_sector_size(0);
    }
    else
#endif
    {
        num_blks = nand_flash_get_numsectors();
        len = nand_flash_get_sector_size(0);
    }

    nd =  (PNVRAM_DATA) (buf + len);

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
    nvram_read(nd);
#else
    nand_read_buf(NVRAM_SECTOR, NVRAM_DATA_OFFSET, (unsigned char *)nd, sizeof(NVRAM_DATA));
#endif

    /* The partition to boot from is stored in the /data partition in a file
     * named boot_state_X where X indicates current or previous partition.
     * In order to not search /data and keep rom in 16KB, we use bootline for simplification 
     * This covers all the cases except where the bootstate is BOOT_SET_NEW_IMAGE_ONCE and 
     * bootline is boot previous. The cferam will boot the appropriate (current or
     * previous) Linux image.
     */
    board_setleds(0x46505330 | NAND_FULL_PARTITION_SEARCH); // FPS0

    if(rom_param & NAND_IMAGESEL_OVERRIDE)
        boot_prev = 1;
    else
        boot_prev = 0;

    for( k = 0, rfsi = rfs_info; k < 2; k++, rfsi++ )
    {
        unsigned int type = 0; // image type

        board_setleds(0x2D2D2D2D);  // ----
        board_setleds(0x50415231 + k);  // PAR# searching partition

        version = 0;
        not_jffs2 = 0;
        rfsi->rootfs = k + NP_ROOTFS_1;
        rfsi->boot_val = bv_invalid;
        rfsi->type = 0;

        if( nd->ulNandPartOfsKb[rfsi->rootfs] > 0 &&
            nd->ulNandPartOfsKb[rfsi->rootfs] < ((num_blks * len) / 1024))
        {
            rfsi->start_blk = nd->ulNandPartOfsKb[rfsi->rootfs] / (len/1024);
            rfsi->end_blk = rfsi->start_blk +
                (nd->ulNandPartSizeKb[rfsi->rootfs] / (len / 1024));
        }
        else
            rfsi->start_blk = rfsi->end_blk = 0;

        if( rfsi->start_blk == 0 || rfsi->start_blk >= rfsi->end_blk ||
            rfsi->start_blk >= num_blks || rfsi->end_blk >= num_blks )
        {
            /* NVRAM_DATA fields for this rootfs are not valid. */
            if( k == 1 )
            {
                /* Skip this rootfs. */
                // board_setleds(0x4e414e36); // NAN6
                continue;
            }

            if( rfs_info[0].boot_val == bv_invalid )
            {
                /* File system info cannot be found for either rootfs.
                 * NVRAM_DATA may not be set.  Use default values.
                 */
                // board_setleds(0x4e414e37); // NAN7
                rfsi = rfs_info;
                rfsi->start_blk = 1;
                rfsi->end_blk = num_blks;
            }
        }

        /* Find the directory entry. */
        for( i = rfsi->start_blk, done = 0; i < rfsi->end_blk && done == 0; i++ )
        {
            /* This loop sequentially reads a NAND flash block into memory and
             * processes it.  First, read a little bit to verify that it is a
             * JFFS2 block.  If it is, read the entire block.
             */
            if( (read_len = nand_read_buf(i, 0, buf, 512)) > 0 )
            {
                pdir = (struct jffs2_raw_dirent *) buf;

                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                {
                    if (!type)
                        type = JFFS2_IMAGE;

                    read_len = nand_read_buf(i, 0, buf, len);

                    /* This loop reads inodes in a block. */
                    p = buf;
                    while( p < buf + len )
                    {
                        pdir = (struct jffs2_raw_dirent *) p;
                        if( (je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK) &&
                            (je32_to_cpu(pdir->hdr_crc) == getCrc32(p,
                            sizeof(struct jffs2_unknown_node) - 4, 0)) )
                        {
                            if( je16_to_cpu(pdir->nodetype) ==
                                    JFFS2_NODETYPE_DIRENT &&
                                g_fname_actual_len == pdir->nsize &&
                                !memcmp(media_params->boot_file_name, pdir->name, g_fname_cmp_len) &&
                                je32_to_cpu(pdir->version) > version &&
                                je32_to_cpu(pdir->ino) != 0 )
                            {
                                /* The desired directory was found. */
                                unsigned char *fname =
                                    pdir->name + g_fname_cmp_len;
                                rfsi->boot_val =
                                    ((fname[0] - '0') * 100) +
                                    ((fname[1] - '0') *  10) +
                                    ((fname[2] - '0') *   1);
                                version = je32_to_cpu(pdir->version);
                                rfsi->mctime = je32_to_cpu(pdir->mctime);
                                rfsi->ino = je32_to_cpu(pdir->ino);
                                rfsi->blk = i;
                                /* Setting 'done = 1' assumes there is only
                                 * one version of the directory entry.
                                 * There will be more than one version if
                                 * the file (cferam.xxx) is renamed,
                                 * modified, copied, etc after it was
                                 * initially flashed.
                                 */
                                rfsi->type = JFFS2_IMAGE;
                                board_setleds(0x4A000000 | (fname[0] << 16) | (fname[1] << 8) | fname[2]); // J###
                                done = NAND_FULL_PARTITION_SEARCH ^ 1;
                                if( done )
                                {
                                    board_setleds(0x4A464653); // JFFS
                                    break;
                                }
                            }

                            p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                            not_jffs2 = 0;
                        }
                        else
                            break;
                    }
                }
                else if ( (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC) && (getCrc32((void *)ec, UBI_EC_HDR_SIZE-4, -1) == be32_to_cpu(ec->hdr_crc)) )
                {
                    char string[3];

                    if (!type)
                        type = UBI_IMAGE;

                    not_jffs2 = 0;

                    for (j = 0; j < 2; j++)
                    {
                        if (j)
                           try = VOLID_METADATA_COPY;

                        else 
                           try = VOLID_METADATA;

                        if (parse_ubi(0, buf, i, rfsi->end_blk, len, try, g_ubi_seqno_id, string, 0, 0, read_blk, 0, 0, 0, 0) == 3)
                        { // successfully retrieved sequence number
                            rfsi->boot_val = ((string[0] - '0') * 100) + ((string[1] - '0') * 10) + (string[2] - '0');
                            board_setleds(0x55000000 | (string[0] << 16) | (string[1] << 8) | string[2]); // U###

                            if (parse_ubi(0, buf, i, rfsi->end_blk, len, try, "committed", string, 0, 0, read_blk, 0, 0, 0, 0) == 1)
                            { // if either copy of metadata is committed then we are committed
                                rfsi->committed = (string[0] == '1') ? 1 : 0;
                                rfsi->type = UBI_IMAGE;
                                board_setleds(0x434F4D00 | string[0]); // COM#
                                break;
                            }
                        }
                    }

                    board_setleds(0x55424923);  // UBI#
                    BOARD_SETLED_HEX16(rfsi->boot_val);
                    rfsi->blk = i;

                    done = 1;
                }
                else
                {
                    if( (max_not_jffs2 > 0) && (not_jffs2++ > max_not_jffs2) )
                    {
                        /* No JFFS2 magic bitmask for consecutive blocks.
                         * Assume this partion does not have a file system
                         * on it.
                         */
                        board_setleds(0x4E4F4E45); // NONE
                        break;
                    }
                }
            }
            else
            {
                read_len = 0;
            }
        }

        if( rfsi->boot_val != bv_invalid )
        {
            board_setleds(0x42540000 + // BT
                ((((((rfsi->boot_val % 100) / 10) + '0') << 8)) |
                 (((rfsi->boot_val % 10) + '0'))));
            board_setleds(
                (((rfsi->blk / 1000) + '0') << 24) |
                ((((rfsi->blk % 1000) / 100) + '0') << 16) |
                (((((rfsi->blk % 1000) % 100) / 10) + '0') << 8) |
                (((((rfsi->blk % 1000) % 100) % 10) + '0') << 0));
        }
        else
            board_setleds(0x4e414e39); // NAN9
    }

    if ( (rfs_info[0].type == JFFS2_IMAGE) || (rfs_info[1].type == JFFS2_IMAGE) )
    { // get JFFS2 boot state from data partition
        unsigned int start_blk, end_blk, version = 0;
        JFFS2_SEARCH_STRUCT jffs2_item[4];
        int j;

        if( nd->ulNandPartOfsKb[NP_DATA] > 0 &&
            nd->ulNandPartOfsKb[NP_DATA] < ((num_blks * len) / 1024))
        {
            start_blk = nd->ulNandPartOfsKb[NP_DATA] / (len/1024);
            end_blk = start_blk + (nd->ulNandPartSizeKb[NP_DATA] / (len / 1024));
        }
        else
            start_blk = end_blk = 0;

        if( start_blk == 0 || start_blk >= end_blk ||
            start_blk >= num_blks || end_blk >= num_blks )
        {
            /* NVRAM_DATA fields are not valid.
             * NVRAM_DATA may not be set.  Use default values.
             */
            start_blk = 1;
            end_blk = num_blks;
        }

        for (j = 0; j < BOOT_STATES; j++)
        {
            jffs2_item[j].bootState = 0;
            jffs2_item[j].version = 0;
        }

        /* Find the directory entry. */
        for( i = start_blk; i < end_blk; i++ )
        {
            /* This loop sequentially reads a NAND flash block into memory and
             * processes it.  First, read a little bit to verify that it is a
             * JFFS2 block.  If it is, read the entire block.
             */
            if( nand_read_buf(i, 0, buf, 4) > 0 )
            {
                pdir = (struct jffs2_raw_dirent *) buf;
                if( (je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK) && (nand_read_buf(i, 0, buf, len) > 0) )
                {
                    /* This loop reads inodes in a block. */
                    p = buf;
                    while( p < buf + len )
                    {
                        pdir = (struct jffs2_raw_dirent *) p;
                        if( (je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK) &&
                            (je32_to_cpu(pdir->hdr_crc) == getCrc32(p,
                            sizeof(struct jffs2_unknown_node) - 4, 0)) )
                        {
                            if( je16_to_cpu(pdir->nodetype) ==
                                    JFFS2_NODETYPE_DIRENT &&
                                g_bname_actual_len == pdir->nsize &&
                                !memcmp(g_bname, pdir->name, g_bname_cmp_len) )
                            { /* boot_state_x was found. */
                                int slot = 0;

                                for (j = 0; j < BOOT_STATES; j++)
                                { // check to see if we found this boot state
                                    if (jffs2_item[j].bootState == pdir->name[g_bname_cmp_len])
                                    {
                                        slot = j;
                                        break;
                                    }
                                    if (!jffs2_item[j].bootState)
                                         slot = j;
                                }

                                if (je32_to_cpu(pdir->version) > jffs2_item[slot].version)
                                { // save the directory entry
                                    jffs2_item[slot].bootState = pdir->name[g_bname_cmp_len];
                                    jffs2_item[slot].version = je32_to_cpu(pdir->version);
                                    jffs2_item[slot].inode = je32_to_cpu(pdir->ino);
                                    jffs2_item[slot].p = p;
                                    jffs2_item[slot].block = i;
                                }
                            }

                            p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                        }
                        else
                            break;
                    }
                }
            }
        }

        i = -1;
        for (j = 0; j < BOOT_STATES; j++)
        { // find highest version boot state existing file
            if (jffs2_item[j].bootState && jffs2_item[j].inode && (jffs2_item[j].version > version) )
            {
                version = jffs2_item[j].version;
                i = j;
            }
        }

        if (i != -1)
            /* Return the current boot state which is the
             * last character of the file name.
             */
            jffs2_boot_state = (int) jffs2_item[i].bootState;
    }

    board_setleds(0x2D2D2D2D);  // ----

    /* Set the rfs_info to the index to boot from. */
    /* Deal with wrap around case */
    if (rfs_info[0].boot_val == 0 && rfs_info[1].boot_val == 999)
        rfs_info[0].boot_val = 1000;
    if (rfs_info[1].boot_val == 0 && rfs_info[0].boot_val == 999)
        rfs_info[1].boot_val = 1000;

    if(rfs_info[1].boot_val == bv_invalid )
    {
        try = 0;
    }
    else if(rfs_info[0].boot_val == bv_invalid )
    {
        try = 1;
    }
    else if (rfs_info[0].boot_val > rfs_info[1].boot_val)
    { // at this point we know there are two images
        if ( (jffs2_boot_state == BOOT_SET_OLD_IMAGE) || (jffs2_boot_state == BOOT_SET_OLD_IMAGE_ONCE) ||
             ( (rfs_info[0].type == UBI_IMAGE) && (rfs_info[1].type == UBI_IMAGE) && !rfs_info[0].committed && rfs_info[1].committed) )
            try = 1;
        else // default case
            try = 0;
    }
    else
    {
        if ( (jffs2_boot_state == BOOT_SET_OLD_IMAGE) || (jffs2_boot_state == BOOT_SET_OLD_IMAGE_ONCE) ||
             ( (rfs_info[0].type == UBI_IMAGE) && (rfs_info[1].type == UBI_IMAGE) && !rfs_info[1].committed && rfs_info[0].committed) )
            try = 0;
        else // default case
            try = 1;
    }

#if !defined(_BCM947189_)
// if pureUBI check if we need to boot the other image
    if ( (rfs_info[0].type == UBI_IMAGE) && (rfs_info[1].type == UBI_IMAGE) && (BOOT_INACTIVE_IMAGE_ONCE_REG & BOOT_INACTIVE_IMAGE_ONCE_MASK) )
        boot_prev = 1;

    BOOT_INACTIVE_IMAGE_ONCE_REG &= ~BOOT_INACTIVE_IMAGE_ONCE_MASK;
#endif

    // swap images if early key abort image swap was selected
    if (boot_prev && rfs_info[0].type && rfs_info[1].type)
        try = !try;

    if (!try)
    {
        /* Boot from first partition image. */
        prfs_info[0] = &rfs_info[0];
        prfs_info[1] = &rfs_info[1];
    }
    else
    {
    /* Boot from second partition image. */
        prfs_info[0] = &rfs_info[1];
        prfs_info[1] = &rfs_info[0];
    }

    /* If the directory entry for the desired file, which is the CFE RAM image,
     * is found, read it into memory and jump to its entry point function.
     * This loop checks for CFE RAM image in two possible locations/partitions/rootfs file sytems.
     */

    for( k = 0; k < 2; k++ )
    {
        unsigned char *pucDest = NULL;
        unsigned char *pucEntry = NULL;
        uint32_t isize = 0;
#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || \
        ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
        uint32_t secCfeRamSize = 0;
#endif
        board_setleds(0x54525931 + try);  // TRY# searching partition

        board_setleds(0x4e414e33); // NAN3
        rfsi = prfs_info[k];
        if( rfsi->boot_val == bv_invalid )
            continue;

        /* When j == 0, get the first inode to find the entry point address.
         * When j == 1, read the file contents into memory.
         */
        if (rfsi->type == JFFS2_IMAGE)
        {
            for( j = 0; j < 2; j++ )
            {
                /* This loop sequentially reads a NAND flash block into memory and
                 * processes it.
                 */
                board_setleds(0x4A465332);  // JFS2

                for(i = rfsi->start_blk, done = 0; i<rfsi->end_blk && done==0; i++)
                {
                    if( nand_read_buf(i, 0, buf, len) > 0 )
                    {
                        /* This loop reads inodes in a block. */
                        p = buf;

                        while( p < buf + len )
                        {
                            /* Verify the first short word is the JFFS2 magic
                             * number.
                             */

                            pino = (struct jffs2_raw_inode *) p;
                            if( (je16_to_cpu(pino->magic) == JFFS2_MAGIC_BITMASK) &&
                                (je32_to_cpu(pino->hdr_crc) == getCrc32(p,
                                sizeof(struct jffs2_unknown_node) - 4, 0)) )
                            {
                                if( je16_to_cpu(pino->nodetype) ==
                                        JFFS2_NODETYPE_INODE &&
                                    je32_to_cpu(pino->ino) == rfsi->ino )
                                {
                                    uint32_t size = je32_to_cpu(pino->dsize);
                                    uint32_t ofs = je32_to_cpu(pino->offset);
                                    uint32_t mtime = je32_to_cpu(pino->mtime);

                                    if( size && mtime == rfsi->mctime )
                                    {
                                        /* A node of the CFE RAM file was found
                                         * with data. */
                                        if( pucDest == NULL )
                                        {
                                            /* The entry point and copy destination
                                             * addresses have not been obtained.
                                             * If this is the first node of the CFE
                                             * RAM file, obtain this information.
                                             */
                                            if( ofs == 0 )
                                            {
                                                /* The first 12 bytes contain a
                                                 * header.  The first word is the
                                                 * entry point address.
                                                 */
                                                memcpy(&pucEntry, pino->data, 4);

                                                pucDest = pucEntry - 12;

                                                isize = je32_to_cpu(pino->isize);

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96866_) || \
        ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
                                                secCfeRamSize = isize;
                                                /* If secure boot, compressed, encrypted CFE RAM */
                                                /* is authenticated within internal memory       */
                                                if (media_params->boot_secure)
                                                   pucDest = (unsigned char *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
#endif
                                                done = 1;
                                                board_setleds(0x52465330 | // RFS0
                                                    rfsi->rootfs);
                                                board_setleds(0x4A464653); // JFFS
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            /* Copy the image to memory. Stop when
                                             * the entire image has been copied.
                                             */
                                            memcpy(pucDest+ofs, pino->data, size);
                                            if( (isize -= size) <= 0 )
                                            {
                                                done = 1;
                                                break;
                                            }
                                        }
                                    }
                                }
    
                                /* Skip to the next inode entry. */
                                p += (je32_to_cpu(pino->totlen) + 0x03) & ~0x03;
                            }
                            else
                                break;
                        }
                    }
                    else
                        board_setleds(0x45525231); // ERR1
                }
            }
        }
        else
        { // process UBI
            uint32_t res;
            board_setleds(0x55424921);  // UBI!

#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
            if (media_params->boot_secure)
            {
                const uint8_t *pHashes = (const uint8_t *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
                res = parse_ubi(0, buf, rfsi->start_blk, rfsi->end_blk, len, VOLID_UBIFILES, 
                    media_params->hash_file_name, (char *)pHashes , 0, 0, read_blk, 0, 0, 0, 0);
                // xprintf("search for hashes.fld returns code = %d\n",res); 
                if (res > SEC_S_MODULUS) { 
                    Booter1Args* sec_args = cfe_sec_get_bootrom_args();
                    xprintf("authenticate...");
                    // authenticate(pHashes, res, sec_args->authArgs.manu);
                    if (sec_verify_signature((uint8_t const*)(&pHashes[SEC_S_MODULUS]), res-SEC_S_MODULUS, &pHashes[0], sec_args->authArgs.manu)) {
                         xprintf("..FAIL\n");
                         die();
                    } else {
                         xprintf("..success\n");
                         parse_boot_hashes((char *)&pHashes[SEC_S_MODULUS], media_params);
                         xprintf("got bootable cferam %s\n",media_params->boot_file_name);
                         sec_should_decrypt = (media_params->boot_file_flags & BOOT_FILE_FLAG_ENCRYPTED) ? 1 : 0 ; 
                         sec_should_decompress = (media_params->boot_file_flags & BOOT_FILE_FLAG_COMPRESSED) ? 1 : 0 ; 
                    }
                }
            }
#endif
            parse_ubi(0, buf, rfsi->start_blk, rfsi->end_blk, len, VOLID_UBIFILES, media_params->boot_file_name, (char *)&pucEntry, 0, 4, read_blk, 0, 0, 0, 0);

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || \
   ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
            // If secure boot, compressed, encrypted CFE RAM
            // is authenticated within internal memory
            if (media_params->boot_secure)
                pucDest = (unsigned char *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
            else
#endif
                pucDest = pucEntry - 12;

            res = parse_ubi(0, buf, rfsi->start_blk, rfsi->end_blk, len, VOLID_UBIFILES, media_params->boot_file_name, (char *)pucDest, 0, 0, read_blk, 0, 0, 0, 0);
            if (!res) {
                pucEntry = 0; // signify an error
            } else {
#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || \
   ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
              if (media_params->boot_secure) {
                  secCfeRamSize = res;
              }
#endif
            }
        }

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || \
          ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
        if (media_params->boot_secure) {
           uint8_t *pEncrCfeRam; 
           int ret;
           Booter1Args* sec_args = cfe_sec_get_bootrom_args();
           if (!sec_args) {
               die();
           }

           /* Authenticate the CFE RAM bootloader */
           board_setleds(0x42544c3f); // BTL?
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
           if (media_params->boot_file_hash_valid) {
	       /* Verify that sha256 hash of cferam matches hash retreived from hash block */
               if (sec_verify_sha256((uint8_t const*)pucDest, secCfeRamSize, (const uint8_t *)media_params->boot_file_hash)) {
                   xprintf("Digest failed\n");
                   die();
               } else {
                   xprintf("Digest has been succesfully matched\n");
               }
               pEncrCfeRam = pucDest; 
           } else
#endif
           {
	       /* Verify the signature located right before the cferam image */
               if (sec_verify_signature((uint8_t const*)(pucDest+SEC_S_MODULUS), secCfeRamSize-SEC_S_MODULUS, pucDest, sec_args->authArgs.manu)) {
                   die();
               }
               pEncrCfeRam = pucDest+SEC_S_MODULUS; 
           }
           board_setleds(0x42544c41); // BTLA
           board_setleds(0x50415353); // PASS

           /* Move pucDest to point to where the authenticated and decrypted (but still compressed) CFE RAM will be put */
           pucDest = (unsigned char *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR);

           /* Get ready to decrypt the CFE RAM bootloader */
           /* decryptWithEk() will whack the content of the iv structure, therefore create a copy and pass that in */
           unsigned char origIv[CIPHER_IV_LEN];
           memcpy((void *)origIv, (void *)sec_args->encrArgs.biv, CIPHER_IV_LEN);

           /* Decrypt the CFE RAM bootloader */
           if (sec_should_decrypt) {
               decryptWithEk(pucDest, (unsigned char *)(&pEncrCfeRam[0]), sec_args->encrArgs.bek,
                            (uint32_t)(secCfeRamSize-SEC_S_SIGNATURE), origIv);
           } else {
               memcpy(pucDest, (unsigned char *)(&pEncrCfeRam[0]), secCfeRamSize);
           }

           /* The reference sw is done with the bek/biv at this point ... cleaning it up */
           /* Any remnants of the keys on the stack will be cleaned up when cfe_launch() runs */
           cfe_sec_reset_keys();
           memset((void *)origIv, 0, CIPHER_IV_LEN);

           /* First 12 bytes are not compressed ... First word of the 12 bytes is the address the cferam is linked to run at */
           /* Note: don't change the line below by adding uintptr_t to make it arch32 and arch64 compatible */
           /* you want it to grab only the first 4 bytes of the 12 bytes in both cases */
           pucEntry = (unsigned char *) (unsigned long)(*(uint32_t *)BTRM_INT_MEM_COMP_CFE_RAM_ADDR);

// decompress or copy RAM+12 for RAM+8 bytes to Entry point , depending on flag
           if (sec_should_decompress ) {
           /* Decompress the image */
           ret = decompressLZMA((unsigned char *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR+12), 
                          (unsigned int)(*(uint32_t *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR + 8)),
                          pucEntry, 23*1024*1024);
           xprintf("Decompress returns code = %d\n",ret); 
           } else {
               memcpy(pucEntry,((unsigned char *)BTRM_INT_MEM_COMP_CFE_RAM_ADDR) + 12,(unsigned int)(*(uint32_t *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR + 8)));
           xprintf("no decompress -- copied to %p \n",pucEntry); 
           }
        }  
#endif

        if( pucEntry && isize <= 0 )
        {
            board_setleds(0x4e414e35); // NAN5

            /* 63138 and 63148 use 16KB below the cfe ram image as the mmu table,
             * so rfs number, cfe number has to be saved 16K down further */
            
           /* Save the rootfs partition that the CFE RAM image boots from
             * at the memory location before the CFE RAM load address. The
             * CFE RAM image uses this value to determine the partition to
             * flash a new rootfs to.
             */
            ROM_PARMS_SET_ROOTFS(pucEntry,(unsigned char)rfsi->rootfs);
            /* Save the sequence numbers so the CFE RAM image does not have
             * to find them again.
             */
            ROM_PARMS_SET_SEQ_P1(pucEntry, (rfs_info[0].boot_val & 0xffff) | NAND_SEQ_MAGIC);
            ROM_PARMS_SET_SEQ_P2(pucEntry, (rfs_info[1].boot_val & 0xffff) | NAND_SEQ_MAGIC);
            if(boot_prev == 1)  {
                ROM_PARMS_SET_ROM_PARM(pucEntry, ((unsigned int)BOOT_SET_OLD_IMAGE)|NAND_IMAGESEL_OVERRIDE);
            } else {
                ROM_PARMS_SET_ROM_PARM(pucEntry, 0);
            }
#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || \
          ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
            if (media_params->boot_secure)
            {
                Booter1Args* sec_args = cfe_sec_get_bootrom_args();
                if (!sec_args) {
                    die();
                }
               /* Copy the authentication credentials from internal memory 
                * into ddr. Cferam on some targets (6838) uses the internal
                * memory. Therefore, linux kernel authentication has to use 
                * these credentials from ddr. Put the data above the 12 bytes
                * of info that were just placed above the cferam */
               ROM_PARMS_AUTH_PARM_SETM(pucEntry, &sec_args->authArgs);
#if defined (_BTRM_DEVEL_)
               ROM_PARMS_SET_DEVEL_ARG(pucEntry, sec_args->otpDieFallThru);
#endif
            }
#endif

            cfe_launch((unsigned long) pucEntry); // never return...
        }
        board_setleds(0x4e414e38); // NAN8
        try ^= 1;
    }

    /* Error occurred. */

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || \
          ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
    if (media_params->boot_secure)
    {
       /* Customer should zero out the bek and the biv at this point because the CFE RAM   */
       /* was not found. Hence, the flash is toast and JTAG needs to be enabled.           */
       cfe_sec_reset_keys();

       /* Cleanup internal memory, and set the external interface enable bit high so that  */
       /* interfaces such as JTAG are suppose to be accessible, they become enabled        */
       cfe_launch(0);
    }
#endif
    board_setleds(0x44494530); // DIE0
    while(1);
}
#endif
#endif

#if (INC_NAND_FLASH_DRIVER==1) && !defined(JTAG_DOWNLOAD)
void bootNand(cfe_rom_media_params *media_params)
{
    if (strap_check_nand())
    {
        bootImageFromNand(rom_option, media_params); /* Will not return. */
    }

    return;
}
#endif

#if (INC_SPI_NAND_DRIVER==1) && !defined(JTAG_DOWNLOAD)
void bootSpiNand(cfe_rom_media_params *media_params)
{
    if (strap_check_spinand())
    {
        bootImageFromNand(rom_option, media_params); /* Will not return. */
    }

    return;
}
#endif
