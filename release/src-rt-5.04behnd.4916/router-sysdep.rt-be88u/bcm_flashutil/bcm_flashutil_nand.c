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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for loff_t */
#endif
#include <fcntl.h>
#include <net/if.h>

#include "genutil_crc.h"
#include "bcm_flashutil.h"
#include "bcm_flashutil_private.h"
#include "bcm_flashutil_nand.h"

#include "bcm_btrm_gen3_common.h"

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h> // close

#include "bcmTag.h" /* in shared/opensource/include/bcm963xx, for FILE_TAG */
#include "board.h" /* in bcmdrivers/opensource/include/bcm963xx, for BCM_IMAGE_CFE */
#include "flash_api.h"
/* map getCrc32 used in bcm_ubi.h to our userspace impl genUtl_getCrc32 */
#define getCrc32 genUtl_getCrc32
#include "bcm_ubi.h"

#include <dirent.h>
#include <fcntl.h> // for open
#include <mtd/mtd-user.h>
#include <linux/jffs2.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <inttypes.h>
#include <asm/setup.h>

#define IS_ERR_OR_NULL(x) ((x)==0)
#define IS_ERR(x) ((x)<0)
#define IS_NULL(x) ((x)==0)

#define ERROR -1
#define SUCCESS 0

#define NAND_CFE_RAM_NAME_LEN     strlen(NAND_CFE_RAM_NAME)
#define NAND_CFE_RAM_NAME_CMP_LEN (strlen(NAND_CFE_RAM_NAME) - 3)
#define NAND_CFE_RAM_RSVD_BLK     9

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

#define CRC_LENGTH 4

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

/* -------------------------------------------------------------------
   this function scans a UBI device for a particular volume and then a
   data name/value element within that volume, and either sets it
   if the write callback function pointer is non-zero, or returns
   the data. It requires a single block sized buffer created
   by the calling code
   -------------------------------------------------------------------
*/
static unsigned int parse_ubi(
    unsigned char * start, // pointer to start of image buffer
    unsigned char * buf, // single block buffer created outside this function we can use
    unsigned int start_blk,
    unsigned int end_blk,
    unsigned int blk_size,
    int volume_id, // volume id can be from 0 to 127, -1 signifies we are bypassing verification of the header (reading UBI data instead of direct from mtd which has UBI header data)
    char * name,
    char * data,
    char ** dataP, // if populated, returns pointer to location of volume_id block
    int header,
    unsigned int (*readblk)(unsigned char * start, unsigned int blk, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd),
    unsigned int (*writeblk)(unsigned char * start, unsigned int blk, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd), // leave NULL if reading
    unsigned int (*eraseblk)(unsigned int blk, unsigned int blk_size, void * mtd, int mtd_fd),
    void * mtd,
    int mtd_fd
)
{
    unsigned int i;

    struct ubi_ec_hdr *ec = (struct ubi_ec_hdr *) buf;
    struct ubi_vid_hdr *vid;

    unsigned int lnum = 0;
    unsigned int end;
    unsigned int state = 1;
    unsigned int from; // file buffer
    unsigned int to = 0; // local buffers
    unsigned int next_entry = 0; // length of record (length to next entry)
    unsigned int data_point = 0; // pointer to entry data name
    unsigned int data_length = 0; // length of entry data
    unsigned int retry = 0;
    unsigned int status = 0;
    unsigned int crc_calc = -1, crc_grab = -1; // data crc

RETRY:
    for( i = start_blk; i < end_blk; i++ )
    { // read ec header
        if (volume_id == -1)
        {
            if ((end = readblk(start, i, 0, blk_size, buf, blk_size, mtd, mtd_fd)))
            {
                vid = 0; // won't be used but need to set it to something
                from = 0; // file buffer
                goto CONT;
            }
            else
                continue;
        }
        if (!readblk(start, i, 0, blk_size, buf, UBI_EC_HDR_SIZE, mtd, mtd_fd))
            continue;

        if ( (be32toh(ec->magic) == UBI_EC_HDR_MAGIC) && (getCrc32((void *)ec, UBI_EC_HDR_SIZE-4, -1) == be32toh(ec->hdr_crc)) )
        { // read up to vid header
            if(!readblk(start, i, UBI_EC_HDR_SIZE, blk_size, buf, be32toh(ec->vid_hdr_offset) + UBI_VID_HDR_SIZE - UBI_EC_HDR_SIZE, mtd, mtd_fd))
                continue;

            vid = (struct ubi_vid_hdr *) (buf + be32toh(ec->vid_hdr_offset));

            if( (be32toh(vid->magic) == UBI_VID_HDR_MAGIC) && (be32toh(vid->vol_id) == (unsigned int)volume_id) && (be32toh(vid->lnum) == lnum) && (getCrc32((void *)vid, UBI_VID_HDR_SIZE-4, -1) == be32toh(vid->hdr_crc)) )
            { // read rest of block
                if(!readblk(start, i, be32toh(ec->vid_hdr_offset) + UBI_VID_HDR_SIZE, blk_size, buf, blk_size - (be32toh(ec->vid_hdr_offset) + UBI_VID_HDR_SIZE), mtd, mtd_fd))
                    continue;

                if ( (vid->vol_type == UBI_VID_DYNAMIC) || (getCrc32((void *)buf + be32toh(ec->data_offset), be32toh(vid->data_size), -1) == be32toh(vid->data_crc)) )
                { // if dynamic volume or data crc matches
                    retry = 1; // found our block, allow for spin through blocks again if we get to the end just in case they are out of order

                    /* Dynamic volumes can have the data_size set to non zero after being scrubbed
                     * by UBI (copy_flag set). In this case the data_size can be unreliable as
                     * UBI truncates the ending 0xff from the byte count (see ubi_eba_copy_leb() in
                     * drivers/mtd/ubi/eba.c). Instead, keep using the block size as the files stored
                     * in UBI raw volumes and their offset/size are calculated based on the block size
                     */
                    if ((vid->vol_type == UBI_VID_STATIC) && vid->data_size)
                        end = be32toh(vid->data_size) + be32toh(ec->data_offset);
                    else
                        end = blk_size; // assume data takes the rest of block if not told otherwise

                    from = be32toh(ec->data_offset);
CONT:
                    switch(state)
                    {
                        default:
                            if (volume_id != -1)
                                from = be32toh(ec->data_offset);
                            else
                                from = 0;

                            to = 0;
                            state = 0;
                            // fall through

                        case 0: // advance to next entry
                            if (next_entry >= (end - from))
                            { // go to next entry
                                next_entry -= (end - from);
                                //lnum += length / (len - offset);
                                //from = length % (len - offset);
                                lnum++;
                                break;
                            }

                            from += next_entry;
                            state++;
                            // fall through

                        case 1: // advance to next entry
                            crc_calc = -1;
                            to = 0;
                            state++;
                            // fall through

                        case 2: // get pointer to next entry
                            while((from < end) && (to < 4))
                            {
                                crc_calc = getCrc32(&buf[from], 1, crc_calc);
                                to++;
                                next_entry <<= 8;
                                next_entry |= buf[from++];
                            }

                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            if (!next_entry)
                            { // we are past the last entry which means we found our metadata block but didn't find our entry
                                return(0);
                            }

                            to = 0;
                            state++;
                            // fall through

                        case 3: // get pointer to entry data
                            while((from < end) && (to < 4))
                            {
                                crc_calc = getCrc32(&buf[from], 1, crc_calc);
                                to++;
                                data_point <<= 8;
                                data_point |= buf[from++];
                                next_entry--;
                            }

                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            if (data_point > next_entry)
                            {
                                printf("ERROR!!! Data pointer greater than total entry size\n");

                                return(0);
                            }

                            to = 0;
                            state++;
                            // fall through

                        case 4: // get length of entry data
                            while((from < end) && (to < 4))
                            {
                                crc_calc = getCrc32(&buf[from], 1, crc_calc);
                                to++;
                                data_length <<= 8;
                                data_length |= buf[from++];
                                data_point--;
                                next_entry--;
                            }

                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            if (data_length > next_entry)
                            {
                                printf("ERROR!!! Data length greater than total entry size\n");

                                return(0);
                            }

                            to = 0;
                            state++;
                            // fall through

                        case 5: // check header crc
                            while((from < end) && (to < 4))
                            {
                                crc_grab = (crc_grab << 8) | buf[from++];
                                to++;
                                data_point--;
                                next_entry--;
                            }

                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            if (crc_calc != crc_grab)
                                return(0);

                            crc_calc = -1;
                            to = 0;
                            state++;
                            // fall through

                        case 6: // compare entry name to what we're searching for
                            while((from < end) && (buf[from] >= 0x20) && (name[to] >= 0x20) && (buf[from] == name[to]) && data_point)
                            {
                                crc_calc = getCrc32(&buf[from++], 1, crc_calc);
                                to++;
                                data_point--;
                                next_entry--;
                            }

                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            if ((buf[from] >= 0x20) || (name[to] >= 0x20))
                            { // entry name doesn't match what we're searching for
                                state = 0;
                                goto CONT;
                            }

                            if (header < 0) // return data length if that's all that's requested
                                return(data_length);

                            state++;
                            // fall through

                        case 7: // move past entry name
                            while((from < end) && data_point)
                            {
                                crc_calc = getCrc32(&buf[from++], 1, crc_calc);
                                data_point--;
                            }

                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            if (header > 0)
                            { // grab just the header if requested, for example just the cferam header info instead of the whole cferam
                                data_length = header;

                                if (data_length > next_entry)
                                {
                                    printf("ERROR!!! Requested data header length greater than total entry size\n");

                                    return(0);
                                }
                            }

                            to = 0;
                            state++;
                            // fall through

                        case 8: // get/put data
                            while((from < end) && data_length) // 4=crc length
                            {
                                if (writeblk)
                                    buf[from] = data[to];
                                else
                                    data[to] = buf[from];

                                crc_calc = getCrc32(&buf[from++], 1, crc_calc);

                                to++;
                                data_length--;
                            }
                            if (writeblk && to && (from >= end))
                            { // update only if we touched data and are at the end of the block
                                if ((volume_id != -1) && (vid->vol_type == UBI_VID_STATIC) && vid->data_size)
                                { // only update the CRC's if static volume, otherwise changing data CRC or size from zero on dynamic volume will cause kernel panic
                                    vid->data_crc = be32toh(getCrc32((void *)buf, be32toh(vid->data_size), -1));
                                    vid->hdr_crc = be32toh(getCrc32((void *)vid, UBI_VID_HDR_SIZE-4, -1));
                                }

                                if (eraseblk)
                                    if (!eraseblk(i, blk_size, mtd, mtd_fd))
                                        return(0);

                                if(!writeblk(start, i, 0, blk_size, buf, blk_size, mtd, mtd_fd))
                                    return(0); 
                            }
                            if (from >= end) // get more data
                            {
                                lnum++;
                                break;
                            }

                            status = to;
                            data_length = 4;
                            state++;
                            // fall through

                        case 9: // check/put data crc
                            while((from < end) && data_length)
                            {
                                if (writeblk)
                                {
                                    buf[from] = crc_calc >> 24;
                                    crc_calc <<= 8;
                                }
                                else
                                {
                                    crc_grab = (crc_grab << 8) | buf[from];
                                }

                                data_length--;
                                from++;
                            }
                            if (writeblk)
                            { // we fell through, write back since data would have been updated in previous state
                                if ((volume_id != -1) && (vid->vol_type == UBI_VID_STATIC) && vid->data_size)
                                { // only update the CRC's if static volume, otherwise changing data CRC or size from zero on dynamic volume will cause kernel panic
                                    vid->data_crc = be32toh(getCrc32((void *)buf, be32toh(vid->data_size), -1));
                                    vid->hdr_crc = be32toh(getCrc32((void *)vid, UBI_VID_HDR_SIZE-4, -1));
                                }

                                if (eraseblk)
                                    if (!eraseblk(i, blk_size, mtd, mtd_fd))
                                        return(0);

                                if(!writeblk(start, i, 0, blk_size, buf, blk_size, mtd, mtd_fd))
                                    return(0); 
                            }
                            if (data_length) // get more data, but only if we need it
                            {
                                lnum++;
                                break;
                            }
                            if (writeblk)
                            {
                                if (dataP) // return block which matches found entry
                                    *dataP = (char *)start + (i * blk_size);

                                return(status);
                            }
                            else if (crc_calc == crc_grab)
                                return(status);
                            else
                                return(0);

                    }
                }
            }
        }
    }

    if (retry) // spin through blocks again in case they are out of order
    {
        retry = 0;

        goto RETRY;
    }

    return(0);
}

static int exist_mtd_dev_nm(const char * check __attribute__((unused)))
{
#ifndef DESKTOP_LINUX
    FILE *fp;
    char line[MAX_MTD_NAME_SIZE];
    char name[MAX_MTD_NAME_SIZE];
    char compare[MAX_MTD_NAME_SIZE];

    if ( (fp = fopen("/proc/mtd","r")) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not open /proc/mtd\n");
        return(0);
    }

    snprintf(compare, sizeof(compare), "%s%s%s", "\"", check, "\"");

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%*s %*s %*s %s", name);

        if(!strcmp(name, compare))
        {
            fclose(fp);
            return(1);
        }
    }

    fclose(fp);

    return(0);
#else
    return(1);
#endif /* DESKTOP_LINUX */
}


int get_mtd_device_name(const char * check __attribute__((unused)), char * device __attribute__((unused)))
{
#ifndef DESKTOP_LINUX
    FILE *fp;
    char mtdn[MAX_MTD_NAME_SIZE];
    char line[MAX_MTD_NAME_SIZE];
    char name[MAX_MTD_NAME_SIZE];
    char compare[MAX_MTD_NAME_SIZE];

    if ( (fp = fopen("/proc/mtd","r")) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not open /proc/mtd\n");
        return(0);
    }

    snprintf(compare, sizeof(compare), "%s%s%s", "\"", check, "\"");

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s %*s %*s %s", mtdn, name);

        if(!strcmp(name, compare))
        {
            fclose(fp);

            mtdn[strlen(mtdn) - 1] = 0; // get rid of trailing colon
            snprintf(device, MAX_MTD_NAME_SIZE, "%s%.10s", "/dev/", mtdn);

            return(1);
        }
    }

    fclose(fp);
    return(0);
#else
    return(1);
#endif
}


#ifndef DESKTOP_LINUX
static char *get_ubi_device_attr(const char *dev_name, const char *attr_name)
{
    char file_name[100];
    FILE *file;
    char *buf = NULL;
    size_t buf_size = 0;
    ssize_t len;

    if (snprintf(file_name, sizeof(file_name), "/sys/class/ubi/%s/%s", dev_name, attr_name) >= (int)sizeof(file_name))
    {
        return NULL;
    }

    /* Read one line from the attribute file */
    file = fopen(file_name, "r");
    if (!file)
        return NULL;
    len = getline(&buf, &buf_size, file);
    fclose(file);
    if (len < 0)
        return NULL;

    /* Remove newline if present */
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = 0;

    return buf;
}
#endif


int get_ubi_device_name(const char * check __attribute__((unused)), char * device __attribute__((unused)))
{
#ifndef DESKTOP_LINUX
    struct dirent *dirent;
    DIR *dir;
    int ret = 1;

    dir = opendir("/sys/class/ubi");
    if (!dir)
        return 1;

    while ((dirent = readdir(dir)))
    {
        char *name;
        int match;

        if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
            continue;

        /* Read and compare the volume name */
        name = get_ubi_device_attr(dirent->d_name, "name");
        match = name && !strcmp(name, check);
        free(name);

        if (match)
        {
            /* Return failure if this is truncated */
            ret = snprintf(device, MAX_MTD_NAME_SIZE, "/dev/%s", dirent->d_name) >= MAX_MTD_NAME_SIZE;
            break;
        }
    }

    closedir(dir);
    return ret;
#else
    return 1;
#endif
}


mtd_info_t * get_mtd_device_handle(const char * check, int * mtd_fd, int * mtdblock_fd)
{
#ifndef DESKTOP_LINUX
    mtd_info_t * mtd;
    FILE *fp;
    char mtdn[MAX_MTD_NAME_SIZE];
    char line[MAX_MTD_NAME_SIZE];
    char name[MAX_MTD_NAME_SIZE];
    char compare[MAX_MTD_NAME_SIZE];

    if ( (mtd = malloc(sizeof(mtd_info_t))) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not allocate memory for mtd structure!\n");
        return(0);
    }

    if ( (fp = fopen("/proc/mtd","r")) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not open /proc/mtd\n");
        free(mtd);
        return(0);
    }

    snprintf(compare, sizeof(compare), "%s%s%s", "\"", check, "\"");

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%s %*s %*s %s", mtdn, name);

        if(!strcmp(name, compare))
        {
            fclose(fp);

            mtdn[strlen(mtdn) - 1] = 0; // get rid of trailing colon

            snprintf(name, MAX_MTD_NAME_SIZE, "%s%.10s", "/dev/", mtdn);

            if ((*mtd_fd = open(name, O_RDWR)) < 0)
            {
                fprintf(stderr, "ERROR!!! Could not open %s\n", name);
                free(mtd);
                return(0);
            }

            if (ioctl(*mtd_fd, MEMGETINFO, mtd) < 0)
            { // cannot MEMGETINFO for mtdblock devices
                fprintf(stderr, "ERROR!!! Could not get MTD information!\n");
                close(*mtd_fd);
                free(mtd);
                return(0);
            }

            flashutil_debug("MTD info: \n"
              "type=0x%x, flags=0x%x, size=0x%x \n"
              "erasesize=0x%x, writesize=0x%x, oobsize=0x%x",
              mtd->type, mtd->flags, mtd->size,
              mtd->erasesize, mtd->writesize, mtd->oobsize);

            if (mtdblock_fd)
            { // proper writing whereby UBI takes care of erasing can only be done through mtdblock, does not work via mtd
                snprintf(name,MAX_MTD_NAME_SIZE,  "%s%.10s", "/dev/mtdblock", &mtdn[3]); // get rid of leading "mtd"

                if ((*mtdblock_fd = open(name, O_RDWR)) < 0)
                {
                    fprintf(stderr, "ERROR!!! Could not open %s\n", name);
                    close(*mtd_fd);
                    free(mtd);
                    return(0);
                }
            }

            return(mtd);
        }
    }

    fclose(fp);
    free(mtd);
    printf("%s:MTD partition/device %s not opened\n", check, __FUNCTION__);
    return(0);
#else
    mtd_info_t * mtd;

    flashutil_print("MTD partition/device %s, mtdblock_fd %p", check, mtdblock_fd);
    if ((mtd = malloc(sizeof(mtd_info_t))) == 0)
    {
        flashutil_error("malloc(mtd_info_t) failed");
        return(0);
    }

    mtd->type = 0;
    mtd->flags = 0;
    mtd->size = 0x60000000;
    mtd->erasesize = 0x20000;
    mtd->writesize = 0;
    mtd->oobsize = 0;
    *mtd_fd = 0;

    flashutil_debug("MTD info: \n"
      "type=0x%x, flags=0x%x, size=0x%x \n"
      "erasesize=0x%x, writesize=0x%x, oobsize=0x%x",
      mtd->type, mtd->flags, mtd->size,
      mtd->erasesize, mtd->writesize, mtd->oobsize);

    return(mtd);
#endif
}


void put_mtd_device(mtd_info_t * mtd, int mtd_fd __attribute__((unused)),  int mtdblock_fd __attribute__((unused)))
{
    flashutil_debug("mtd=%p, mtd_fd=%d", mtd, mtd_fd);
    free(mtd);
#ifndef DESKTOP_LINUX
    close(mtd_fd);

    if(mtdblock_fd >= 0)
        close(mtdblock_fd);
#endif
}

#ifndef DESKTOP_LINUX
/* Userspace mtdchar read function does not report NAND error, the reason for this is that the read function ignores a driver
 * error and instead returns an incorrect read length code (the same length as was requested) and does not update errno.
 * The kernel writers justification for this is that there is still useable data that could be retrieved, however this is
 * assuming that the driver did indeed read the data. I believe a better method is to present the data but still return an
 * error code leaving it up to the caller to determine what to do with the data (if any.) However in the interest of tweaking
 * the kernel as little as possible this mread function was created to return an error. */

static int mread(int mtd_fd, void *data_ptr, int data_len)
{
   struct mtd_ecc_stats ecc_stats;
   int status, fails;

   ioctl(mtd_fd, ECCGETSTATS, &ecc_stats);
   fails = ecc_stats.failed;

   status = read(mtd_fd, data_ptr, data_len);
   if (status < 0)
      return(status);

   ioctl(mtd_fd, ECCGETSTATS, &ecc_stats);

   if (fails != ecc_stats.failed)
      return(-1);

   return(status);
}


static int nandReadBlk(mtd_info_t *mtd, int blk_addr, int data_len, 
                       unsigned char *data_ptr, int mtd_fd)
{
   if (lseek(mtd_fd, blk_addr, SEEK_SET) < 0)
   {
      fprintf(stderr, "ERROR!!! Could not block seek to 0x%x!\n", blk_addr);
      return(-1);
   }

   if (mread(mtd_fd, data_ptr, data_len) < 0)
   {
      fprintf(stderr, "ERROR!!! Could not read block at offset 0x%x!\n", blk_addr);
      return(-1);
   }

   return(0);
}
#else
static int nandReadBlk(mtd_info_t *mtd __attribute__((unused)),
                       int blk_addr __attribute__((unused)),
                       int data_len __attribute__((unused)),
                       unsigned char *data_ptr __attribute__((unused)),
                       int mtd_fd __attribute__((unused)))
{
   flashutil_debug("mtd=%p, blkaddr=0x%x, len=%d, data=%p, fd=%d",
     mtd, blk_addr, data_len, data_ptr, mtd_fd);
   return(0);
}
#endif /* DESKTOP_LINUX */


/* Write data, must pass function an aligned block address */
int nandWriteBlk(mtd_info_t *mtd, int blk_addr, int data_len, unsigned char *data_ptr, int mtd_fd, int write_JFFS2_clean_marker)
{
#ifndef DESKTOP_LINUX
#if !defined(CONFIG_CPU_BIG_ENDIAN)
   const unsigned short jffs2_clean_marker[] = {JFFS2_MAGIC_BITMASK, JFFS2_NODETYPE_CLEANMARKER, 0x0008, 0x0000};
#else
   const unsigned short jffs2_clean_marker[] = {JFFS2_MAGIC_BITMASK, JFFS2_NODETYPE_CLEANMARKER, 0x0000, 0x0008};
#endif
   struct mtd_write_req ops;
   int page_addr, byte, sts = 0;

   flashutil_debug("mtd=%p, blkaddr=0x%x, len=%d, data=%p, fd=%d, marker=%d",
     mtd, blk_addr, data_len, data_ptr, mtd_fd, write_JFFS2_clean_marker);

   for (page_addr = 0; page_addr < data_len; page_addr += mtd->writesize)
   {
      memset(&ops, 0x00, sizeof(ops));

      // check to see if whole page is FFs
      for (byte = 0; (byte < (int)mtd->writesize) && ((page_addr + byte) < data_len); byte++)
      {
         if ( *(unsigned char *)(data_ptr + page_addr + byte) != 0xFF )
         {
            ops.start = blk_addr + page_addr;
            ops.len = (int) mtd->writesize < (data_len - page_addr) ? mtd->writesize : (unsigned int) data_len - page_addr;
            ops.usr_data = (uint64_t)(unsigned long)(data_ptr + page_addr);
            break;
         }
      }

      if (write_JFFS2_clean_marker)
      {
         ops.mode = MTD_OPS_AUTO_OOB;
         ops.usr_oob = (uint64_t)(unsigned long)jffs2_clean_marker;
         ops.ooblen = sizeof(jffs2_clean_marker);
         write_JFFS2_clean_marker = 0; // write clean marker to first page only
      }

      if (ops.len || ops.ooblen)
      {
         if ((sts = ioctl(mtd_fd, MEMWRITE, &ops)) != 0)
         {
            break;
         }
      }
   }

   return(sts);
#else
   flashutil_print("mtd=%p, blkaddr=0x%x, len=%d, data=%p, fd=%d, marker=%d",
     mtd, blk_addr, data_len, data_ptr, mtd_fd, write_JFFS2_clean_marker);
   return 0;
#endif
}

static int nandIsBadBlk(int blk_addr, int mtd_fd)
{
#ifndef DESKTOP_LINUX
   loff_t offset = blk_addr;

   int ret = ioctl(mtd_fd, MEMGETBADBLOCK, &offset);  
   if ( ret > 0) {
       fprintf(stderr, "Bad block at 0x%x\n", blk_addr);
       return 1;
   }

   if( ret < 0 ) {
       /* bad block check not available?? Treat as bad block. We are only dealing with NAND */
       fprintf(stderr, "MTD get bad block failed at 0x%x, ret %d\n", blk_addr, ret);
       return 1;
   }
#else
   flashutil_print("addr=0x%x, fd=%d", blk_addr, mtd_fd);
#endif  
   return(0);
}

/* Erase the specified NAND flash block. */
int nandEraseBlk(mtd_info_t *mtd, int blk_addr, int mtd_fd)
{
#ifndef DESKTOP_LINUX
   erase_info_t erase;

   erase.start = blk_addr;
   erase.length = mtd->erasesize;

   if (ioctl(mtd_fd, MEMERASE, &erase) < 0)
   {
      fprintf(stderr, "Could not erase block\n");
      return(-1);
   }
   flashutil_debug("mtd=%p, addr=0x%x, fd=%d", mtd, blk_addr, mtd_fd);
#else
   flashutil_print("mtd=%p, addr=0x%x, fd=%d", mtd, blk_addr, mtd_fd);
#endif

   return(0);
}

/* Note: the following functions is for CFEROM flashing debugging. */
int nandEraseBlk_print(mtd_info_t *mtd, int blk_addr, int mtd_fd)
{
   flashutil_print("mtd=%p, addr=0x%x, fd=%d", mtd, blk_addr, mtd_fd);
   return(0);
}


/***********************************************************************
 * Function Name: scan_partition
 * Description  : scans mtd names to determine type
 * Returns      : None, JFFS2 or (pure) UBI image type
 ***********************************************************************/
int nand_image_type(unsigned char * buf)
{ // scans buffer for image type
    int ret = 0;
    struct ubi_ec_hdr * ec = (struct ubi_ec_hdr *) buf;
    struct jffs2_raw_dirent *pdir = (struct jffs2_raw_dirent *) buf;

    if( nandIsLegacyFlashLayout() )
    {
        if(je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK)
            ret = JFFS2_IMAGE;
        else if(be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC)
            ret = UBI_IMAGE;
    }
    else
        ret = UBI_IMAGE;


    return(ret);
}


static int scan_partition(char * name)
{
    int i;
    int mtd_fd;
    mtd_info_t * mtd;
    unsigned char buf[4];
    int ret = 0;

    mtd = get_mtd_device_handle(name, &mtd_fd, 0);

    if( !IS_ERR_OR_NULL(mtd) )
    { // Find the partition type
        for( i = 0; i < (int)mtd->size; i += mtd->erasesize )
        {
            if(IS_ERR(nandReadBlk(mtd, i, 4, buf, mtd_fd)))
                continue;        

            if ((ret = nand_image_type(buf)))
                break;

        }

        put_mtd_device(mtd, mtd_fd, -1);
    }

    return(ret);
}


#ifndef DESKTOP_LINUX
/***********************************************************************
 * Function Name: findBootBlockInImage
 * Description  : find the boot code blk number in image buffer
 * Returns      : number of block boot code takes, -1 if invalid image
 ***********************************************************************/
static int findBootBlockInImage(mtd_info_t *mtd, unsigned char *pImage, unsigned int imageSize)
{
    int i = 0;
    int  blk_size = mtd->erasesize;
    unsigned char* pImageEnd = pImage + imageSize;

    while( (nand_image_type(pImage) != JFFS2_IMAGE) && (nand_image_type(pImage) != UBI_IMAGE) ) 
    {
        i++;
        pImage += blk_size;
        if( pImage >= pImageEnd )
        {
            i = -1;
            break;
        }
    } 

    return i;
}

/***********************************************************************
 * Function Name: available_space_in_partition
 * Description  : find usable space in mtd partiton
 * Returns      : number of usable bytes 
 ***********************************************************************/
static unsigned int available_space_in_partition(mtd_info_t *mtd, int mtd_fd)
{
    unsigned int total_size = mtd->size;
    unsigned int block_size = mtd->erasesize;
    unsigned int block_addr, avail_size = total_size;
      
    for( block_addr = 0; block_addr < total_size; block_addr += block_size )
    {
        if( nandIsBadBlk(block_addr, mtd_fd) ) 
            avail_size -= block_size;
    }

    return avail_size;
}

/***********************************************************************
 * Function Name: checkImageSizeForNand
 * Description  : check if the image can fit into the target partition
 * Returns      : 0 if fit and non zero if it does not 
 ***********************************************************************/
static int checkImageSizeForNand(mtd_info_t *mtd, int mtd_fd, unsigned char* imagePtr, unsigned int imageSize)
{
    int rc = 0;
    int blk_size = mtd->erasesize;
    int boot_img_blk, boot_img_size;
    unsigned int avail_part_size = available_space_in_partition(mtd, mtd_fd);
    unsigned int required_fs_size;

    boot_img_blk = findBootBlockInImage(mtd, imagePtr, imageSize);
    if( boot_img_blk < 0 )
        return -1;
    boot_img_size = boot_img_blk*blk_size;
    required_fs_size = imageSize - boot_img_size + (NAND_CFE_RAM_RSVD_BLK-1)*blk_size;

    if ( avail_part_size < required_fs_size ) {
        fprintf(stderr,"\nFS Image(%d) plus reserve(%d) bigger than the partition(%d)\n", imageSize-boot_img_size, (NAND_CFE_RAM_RSVD_BLK-1)*blk_size, avail_part_size);
            rc = -1;
    }
    
    return rc;
}
#endif /* DESKTOP_LINUX */


/***********************************************************************
 * Function Name: nandGetBootPartition
 * Description  : Returns the booted partition
 * Returns      : boot partition of 1 or 2, error if another value
 ***********************************************************************/
int nandGetBootPartition(void)
{
    int ret = -1;

#ifndef DESKTOP_LINUX
    FILE *fp = NULL;
    char line[COMMAND_LINE_SIZE] = {0};
    char name[MAX_MTD_NAME_SIZE] = {0};
    static int cached_boot_partition = -1;

    if (cached_boot_partition != -1)
    {
        return cached_boot_partition;
    }

    if( nandIsLegacyFlashLayout())
    {
        if ( (fp = fopen("/proc/mtd","r")) == 0)
        {
            fprintf(stderr, "ERROR!!! Could not open /proc/mtd\n");
            return(0);
        }

        while(fgets(line, COMMAND_LINE_SIZE, fp))
        {
            sscanf(line, "%*s %*s %*s %s", name);

            if(!strcmp("\"image\"", name))
            {
                ret = 1;
                break;
            }

            if(!strcmp("\"image_update\"", name))
            {
                ret = 2;
                break;
            }
        }
        fclose(fp);
    }
    else
    {
        /* Use get_rootfs_dev script */
        fp = popen("/rom/etc/get_rootfs_dev.sh", "r");
        if (fp == NULL) {
            fprintf(stderr, "Failed to run /rom/etc/get_rootfs_dev.sh command\n" );
            return 0;
        }

        if (fgets(line, COMMAND_LINE_SIZE, fp) != NULL)
        {
            if( strstr(line, "ubiblock"))
            {
                sscanf(line,"/dev/ubiblock0_" "%d", &ret);
                /* Translate from volume id to image number */
                //FIXME: Magic numbers
                if (ret == 4)
                    ret = 1;
                else if (ret == 6)
                    ret = 2;
                else
                    ret = 0;
            }
            else
            {
                sscanf(line,"ubi:rootfs" "%d", &ret);
		if ((ret != 1) && (ret != 2))
                    ret = 0;
            }
        }
        pclose(fp);
    }

    cached_boot_partition = ret;
#endif /* DESKTOP_LINUX */

    return(ret);
}


unsigned int nvramDataOffset(const mtd_info_t * mtd __attribute__((unused)))
{
    return( (NVRAM_DATA_REL_OFFSET+(IMAGE_OFFSET-(IMAGE_OFFSET/((unsigned int)mtd->erasesize))*((unsigned int)mtd->erasesize))) );
}


static unsigned int nvramSector(const mtd_info_t * mtd __attribute__((unused)))
{
    return( (IMAGE_OFFSET/((unsigned int)mtd->erasesize)) );
}


#ifndef DESKTOP_LINUX
#define MAX_NVRAM_MIRROR_LOOKUP_OFFSET 20*1024*1024



int nvRamMirrorSearch( PNVRAM_DATA nv)
{
    mtd_info_t * mtd;
    int mtd_fd;
    int search_complete=0;
    unsigned char *buff=NULL;
    loff_t offset=0;


    if ( (mtd = get_mtd_device_handle("nvram", &mtd_fd, 0)) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not open nvram partition!\n");
        search_complete=-1;
    }
    if (lseek(mtd_fd, 0, SEEK_SET) < 0)
    {
        fprintf(stderr, "ERROR!!! Could not seek to 0\n");
        search_complete=-1;
    }
    //buff=malloc(sizeof(NVRAM_DATA)+strlen(NVRAM_DATA_SIGN));
    buff=malloc(2048);
    if(buff == NULL)
    {
        fprintf(stderr, "ERROR!!! memory allocation failed\n");
        search_complete=-1;
    }


    for(offset=0;search_complete == 0 && offset < MAX_NVRAM_MIRROR_LOOKUP_OFFSET;offset += 2048) {
        memset(buff, '\0', 2048);
        if(mread(mtd_fd, buff, 2048) < 0)
        {
            fprintf(stderr, "ERROR!!! Cound not get NVRAM data!\n");
        }
        else
        {
            if(offset%mtd->erasesize == 0) {
                if(check_jffs_ubi_magic(buff) == 1) {
                    printf("NVRAM_MIRROR SCAN: OFFSET blk [%" PRIx64 "] \n",  offset);
                    search_complete = 2;
                    continue;
                } 
            } 
            ;
            //check nvram data signature
            if(is_nvram_offset(buff, 0, 1, NULL, NULL) == IMG_NVRAM) {
                memcpy(nv, buff+strlen(NVRAM_DATA_SIGN), sizeof(NVRAM_DATA));
                search_complete = 1;
                break;
            }
        }
    }
    put_mtd_device(mtd, mtd_fd, -1);
    free(buff);
    return search_complete;
}
#endif


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
int nandReadNvramData(void *nvramData)
{
#ifndef DESKTOP_LINUX
    uint32_t crc = CRC_INITIAL_VALUE, savedCrc;
    mtd_info_t * mtd;
    int mtd_fd;
    int status = 0;
    NVRAM_DATA *pNvramData = (NVRAM_DATA*)nvramData;

    if ( (mtd = get_mtd_device_handle("nvram", &mtd_fd, 0)) == 0)
    {
        fprintf(stderr, "ERROR!!! Could not open nvram partition!\n");
        return(0);
    }

    if (lseek(mtd_fd, nvramDataOffset(mtd), SEEK_SET) < 0)
    {
        fprintf(stderr, "ERROR!!! Could not seek to 0x%x in nvram partition!\n", nvramDataOffset(mtd));
    }
    else if (mread(mtd_fd, pNvramData, sizeof(NVRAM_DATA)) < 0)
    {
        fprintf(stderr, "ERROR!!! Cound not get NVRAM data!\n");
    }

    put_mtd_device(mtd, mtd_fd, -1);

    savedCrc = pNvramData->ulCheckSum;
    pNvramData->ulCheckSum = 0;
    crc = genUtl_getCrc32((unsigned char *)pNvramData, sizeof(NVRAM_DATA), crc);
    if (savedCrc == crc)
    {
        // this can happen if we write a new cfe image into flash.
        // The new image will have an invalid nvram section which will
        // get updated to the inMemNvramData.  We detect it here and
        // commonImageWrite will restore previous copy of nvram data.
        //kfree(pNvramData);
        status = 1;
    }
    else
    {
        status=nvRamMirrorSearch(pNvramData);
    }

    return(status);
#else
    flashutil_print("readNvramData(%p)", nvramData);
    return 1;
#endif
}

// mtd block access functions
static unsigned int read_blk(unsigned char * start __attribute__((unused)), unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(nandReadBlk((mtd_info_t *)mtd, ((block * blk_size) + offset), amount, buf + offset, mtd_fd)+1);
}


static unsigned int write_blk(unsigned char * start __attribute__((unused)), unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(nandWriteBlk((mtd_info_t *)mtd, ((block * blk_size) + offset), amount, buf + offset, mtd_fd, 0)+1);
}


static unsigned int erase_blk(unsigned int block, unsigned int blk_size, void * mtd, int mtd_fd)
{
    return(nandEraseBlk((mtd_info_t *)mtd, (block * blk_size), mtd_fd)+1);
}


// UBI block access functions
#ifndef DESKTOP_LINUX
static unsigned int read_block(unsigned char * start __attribute__((unused)), unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
//    FILE *fp = fopen("/dev/mtdblock9","r"); // two ways could do this, either with file pointer or file descriptor

//    if (fseek(fp, (block * blk_size) + offset, SEEK_SET) < 0)
    if (lseek(mtd_fd, (block * blk_size) + offset, SEEK_SET) < 0)
         return(0);

//    if (fread(buf, amount, 1, fp) < 0)
    if (mread(mtd_fd, buf, amount) < 0)
        return(0);

//    fclose(fp);
    return(amount);
}


static unsigned int write_block(unsigned char * start __attribute__((unused)), unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
//    FILE *fp = fopen("/dev/mtdblock9","w"); // two ways could do this, either with file pointer or file descriptor

//    if (fseek(fp, (block * blk_size) + offset, SEEK_SET) < 0)
    if (lseek(mtd_fd, (block * blk_size) + offset, SEEK_SET) < 0)
        return(0);

//    if (fwrite(buf, amount, 1, fp) < 0)
    if (write(mtd_fd, buf, amount) < 0)
        return(0);

//    fclose(fp);
    return(amount);
}


#else
static unsigned int read_block(unsigned char * start __attribute__((unused)), unsigned int block __attribute__((unused)), unsigned int offset __attribute__((unused)), unsigned int blk_size __attribute__((unused)), unsigned char * buf __attribute__((unused)), unsigned int amount __attribute__((unused)), void * mtd __attribute__((unused)), int mtd_fd __attribute__((unused)))
{
    return(0);
}

static unsigned int write_block(unsigned char * start __attribute__((unused)), unsigned int block __attribute__((unused)), unsigned int offset __attribute__((unused)), unsigned int blk_size __attribute__((unused)), unsigned char * buf __attribute__((unused)), unsigned int amount __attribute__((unused)), void * mtd __attribute__((unused)), int mtd_fd __attribute__((unused)))
{
    return(0);
}
#endif


int nandGetSequenceNumber(int imageNumber)
{ /* NAND Flash */
#ifndef DESKTOP_LINUX
    int seqNumber = -1;
#else
    int seqNumber = imageNumber;
#endif /* DESKTOP_LINUX */
    FILE *fp;
    unsigned char * buf = NULL;
    char fname[] = NAND_CFE_RAM_NAME;
    int i, type, partitionIsBootedPartition = (nandGetBootPartition() == imageNumber);

    /* first check if we already have the number in cache */
    if (imageNumber == 1)
        fp = fopen("/tmp/seqNumFirstImg", "r");
    else
        fp = fopen("/tmp/seqNumSecondImg", "r");

    if (fp != NULL)
    {
        int status;

        status = fread(&seqNumber, sizeof(seqNumber), 1, fp);
        fclose(fp);

        if (status == 1)
        {
            return(seqNumber);
        }
    }

    if( !nandIsLegacyFlashLayout() )
    {
        if (getImgSeqNum(imageNumber, &seqNumber))
            printf("%s: Error! Could not retrieve sequence number for image %d\n", __FUNCTION__, imageNumber);
        return seqNumber;
    }

    if (partitionIsBootedPartition)
    { // scan mtd device names
        if (exist_mtd_dev_nm("METADATA") || exist_mtd_dev_nm("METADATACOPY"))
            type = UBI_IMAGE;
        else
            type = JFFS2_IMAGE;
    }
    else // get from non-booted image
        type = scan_partition("image_update");

    /* If full secure boot is in play, the CFE RAM file is the encrypted version */
    if (type == JFFS2_IMAGE)
    {
        if (otp_is_boot_secure())
            strcpy(fname, NAND_CFE_RAM_SECBT_NAME);
        else
        {
            if (otp_is_boot_mfg_secure())
                strcpy(fname, NAND_CFE_RAM_SECBT_MFG_NAME);
        }
    }

    // Find the directory entry.
    if (type == JFFS2_IMAGE)
    {
        char cferam_buf[32], cferam_fmt[32];
        int rc;

        if (!partitionIsBootedPartition)
        {
            strcpy(cferam_fmt, "/tmp/mnt/");
            rc = system("mkdir /tmp/mnt");

            if( !IS_ERR_OR_NULL(exist_mtd_dev_nm("bootfs_update")) ) // check if JFFS2 with UBIFS
            { // JFFS2 with UBIFS
                rc = system("mount -t jffs2 mtd:bootfs_update /tmp/mnt -r");
            }
            else
            { // JFFS2 only
                rc = system("mount -t jffs2 mtd:rootfs_update /tmp/mnt -r");
            }

            if (rc < 0)
            {
               fprintf(stderr, "ERROR!!! mount command failed!\n");
               return(seqNumber);
            }
        }
        else
        { // get from current booted image
            if( !IS_ERR_OR_NULL(exist_mtd_dev_nm("bootfs")) )
            { // JFFS2 with UBIFS
                strcpy(cferam_fmt, "/bootfs/");
            }
            else
            { // JFFS2 only
                strcpy(cferam_fmt, "/");
            }
        }

        /* Find the sequence number of the specified partition. */
        fname[strlen(fname) - 3] = '\0'; /* remove last three chars */
        strcat(cferam_fmt, fname);
        strcat(cferam_fmt, "%3.3d");

        for( i = 0; i < 1000; i++ )
        {
            sprintf(cferam_buf, cferam_fmt, i);
            fp = fopen(cferam_buf, "r");
            if (fp != NULL)
            {
                fclose(fp);

                /* Seqence number found. */
                seqNumber = i;
                break;
            }
        }

        if( !partitionIsBootedPartition )
        {
            if (system("umount /tmp/mnt") < 0)
            {
              fprintf(stderr, "ERROR!!! umount command failed!\n");
            }
        }
    }

    if (type == UBI_IMAGE)
    {
        int mtd_fd;
        mtd_info_t * mtd;
        char imageSequence[3];
        unsigned int len;

        if (partitionIsBootedPartition)
        { // use UBI interface
            int mtdblock_fd;

            for (i = 0; i < 2; i++)
            {
                mtd = get_mtd_device_handle(i ? "METADATACOPY" : "METADATA", &mtd_fd, &mtdblock_fd);

                if ( IS_ERR_OR_NULL(mtd) )
                {
                    fprintf(stderr, "ERROR!!! unable to open mtd partition %s\n", i ? "METADATACOPY" : "METADATA");
                    continue;
                }

                len = mtd->erasesize;

                if (!buf)
                {
                    if ((len == 0) || ((buf = malloc(len)) == NULL))
                    {
                        fprintf(stderr, "ERROR!!! cannot allocate block memory for buffer\n");
                        put_mtd_device(mtd, mtd_fd, mtdblock_fd);
                        continue;
                    }
                }

                if (parse_ubi(0, buf, 0, mtd->size / len, len, -1, fname, imageSequence, 0, 0, read_block, 0, 0, mtd, mtdblock_fd) == 3)
                {
                    seqNumber = ((imageSequence[0] - '0') * 100) + ((imageSequence[1] - '0') * 10) + (imageSequence[2] - '0');
                    put_mtd_device(mtd, mtd_fd, mtdblock_fd);

                    break;
                }
                else
                    printf("ERROR!!! Could not find %s partition image metadata volume %d\n", (imageNumber == 1) ? "first" : "second", i);

                put_mtd_device(mtd, mtd_fd, mtdblock_fd);
            }

            if (buf)
                free(buf);
        }
        else
        { // use mtd interface
            mtd = get_mtd_device_handle("image_update", &mtd_fd, 0);

            if ( IS_ERR_OR_NULL(mtd) )
            {
                fprintf(stderr, "ERROR!!! unable to open image %d\n", imageNumber);
                return(seqNumber);
            }

            len = mtd->erasesize;

            if ((buf = malloc(len)) == 0)
            {
                fprintf(stderr, "ERROR!!! cannot allocate block memory for buffer\n");
                put_mtd_device(mtd, mtd_fd, -1);
                return(seqNumber);
            }

            for (i = 0; i < 2; i++)
            {
                if (parse_ubi(0, buf, 0, mtd->size / len, len, i ? VOLID_METADATA_COPY : VOLID_METADATA, fname, imageSequence, 0, 0, read_blk, 0, 0, mtd, mtd_fd) == 3)
                {
                    seqNumber = ((imageSequence[0] - '0') * 100) + ((imageSequence[1] - '0') * 10) + (imageSequence[2] - '0');
                    break;
                }
            }

            free(buf);
            put_mtd_device(mtd, mtd_fd, -1);
        }
    }

    if (seqNumber != -1)
    {
        FILE *fp;

        if (imageNumber == 1)
            fp = fopen("/tmp/seqNumFirstImg", "w");
        else
            fp = fopen("/tmp/seqNumSecondImg", "w");

        if (fp != NULL)
        {
            fwrite(&seqNumber, sizeof(seqNumber), 1, fp);
            fclose(fp);
        }
    }

    return(seqNumber);
}


static unsigned int readBlk(unsigned char * start, unsigned int block,
  unsigned int offset, unsigned int blk_size, unsigned char * buf,
  unsigned int amount, void * mtd __attribute__((unused)),
  int mtd_fd __attribute__((unused)))
{
    memcpy(buf + offset, start + (block * blk_size) + offset, amount);
    return(amount);
}

static unsigned int writeBlk(unsigned char * start, unsigned int block,
  unsigned int offset, unsigned int blk_size, unsigned char * buf,
  unsigned int amount, void * mtd __attribute__((unused)),
  int mtd_fd __attribute__((unused)))
{
    memcpy(start + (block * blk_size) + offset, buf + offset, amount);
    return(amount);
}

/*
 * nandUpdateSeqNum
 *
 * Read the sequence number from rootfs partition only.  The sequence number is
 * the extension on the cferam file.  Add one to the sequence number
 * and change the extenstion of the cferam in the image to be flashed to that
 * number.
 */
unsigned char *nandUpdateSeqNum(unsigned char *imagePtr, int imageSize, int blkLen, int seq, int *found)
{
    char fname[] = NAND_CFE_RAM_NAME;
    int fname_actual_len = strlen(fname);
    int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
    char * ret = NULL;
    int boot = 0;

    /* If full secure boot is in play, the CFE RAM file is the encrypted version */
    if (nand_image_type(imagePtr) == JFFS2_IMAGE)
    {
        if (otp_is_boot_secure())
            strcpy(fname, NAND_CFE_RAM_SECBT_NAME);
        else
        {
            if (otp_is_boot_mfg_secure())
                strcpy(fname, NAND_CFE_RAM_SECBT_MFG_NAME);
        }
    }

    if ( seq == -1 )
    { // get sequence number if we are not provided with it
        if ((boot = nandGetBootPartition()) > 0)
            seq = nandGetSequenceNumber(boot); // get sequence number of booted partition
    }

    if( (seq != -1) && !nandIsLegacyFlashLayout() )
    {
	seq = (seq + 1) % 999; // set newly written image sequence number one greater than other image
        if ((boot = nandGetBootPartition()) > 0)
            setImgSeqNum(boot == 1 ? 2 : 1, seq);

        return((unsigned char *)ret);
    }

    if( seq != -1 )
    {
        unsigned char *buf, *p;
        struct jffs2_raw_dirent *pdir;
        struct ubi_ec_hdr *ec;
        unsigned long version = 0;
        int done = 0;
        unsigned int type = 0; // image type

        while (!nand_image_type(imagePtr) && (imageSize > 0))
        {
            imagePtr += blkLen;
            imageSize -= blkLen;
        }

        /* Confirm that we did find a JFFS2_MAGIC_BITMASK or UBIFS magic number. If not, we are done */
        if (imageSize <= 0)
        {
            done = 1;
        }

        /* Increment the new highest sequence number. Add it to the CFE RAM
         * file name.
         */
        seq++;
        if (seq > 999)
        {
            seq = 0;
        }

        /* Search the image and replace the last three characters of file
         * cferam.000 with the new sequence number.
         */
        for(p = imagePtr; p < imagePtr+imageSize && done == 0; p += blkLen)
        {
            ec = (struct ubi_ec_hdr *) p;

            if( (nand_image_type(p) == UBI_IMAGE) && (genUtl_getCrc32((void *)ec, UBI_EC_HDR_SIZE-4, CRC_INITIAL_VALUE) == be32_to_cpu(ec->hdr_crc)) )
            { // will only need to spin through the blocks once since the image is built with volumes in order

                unsigned char * buffer;
                char seqstr[3] = {(seq / 100) + '0', ((seq % 100) / 10) + '0', ((seq % 100) % 10) + '0'};
                int i, try;

                if (!(buffer = (unsigned char *) malloc(blkLen)))
                    return(0);

                for (i = 0; i < 2; i++)
                {
                    if (i)
                        try = VOLID_METADATA_COPY;
                    else
                        try = VOLID_METADATA;

                    if (parse_ubi((unsigned char *)imagePtr, buffer, (p - imagePtr) / blkLen, imageSize / blkLen, blkLen, try, fname, seqstr, &ret, 0, readBlk, writeBlk, 0, 0, 0) == 3)
                    {
                        printf("\nUpdating pureUBI metadata %d sequence number to %d", i, seq);
                        (*found)++;
                    }
                }

                free(buffer);

                done = 1;
            }
            else
            {
                if (!type)
                    type = JFFS2_IMAGE;

                buf = p;
                while( buf < (p + blkLen) )
                {
                    pdir = (struct jffs2_raw_dirent *) buf;
                    if( nand_image_type(buf) == JFFS2_IMAGE )
                    {
                        if( je16_to_cpu(pdir->nodetype) == JFFS2_NODETYPE_DIRENT &&
                            fname_actual_len == pdir->nsize &&
                            !memcmp(fname, pdir->name, fname_cmp_len) &&
                            je32_to_cpu(pdir->version) > version &&
                            je32_to_cpu(pdir->ino) != 0 )
                         {
                            printf("Updating JFFS2 sequence number to %d\n", seq);
                            /* File cferam.000 found. Change the extension to the
                             * new sequence number and recalculate file name CRC.
                             */
                            buf = (unsigned char *)pdir->name + fname_cmp_len;
                            buf[0] = (seq / 100) + '0';
                            buf[1] = ((seq % 100) / 10) + '0';
                            buf[2] = ((seq % 100) % 10) + '0';
                            buf[3] = '\0';

                            je32_to_cpu(pdir->name_crc) = genUtl_getCrc32(pdir->name, (uint32_t)fname_actual_len, 0);

                            version = je32_to_cpu(pdir->version);

                            /* Setting 'done = 1' assumes there is only one version
                             * of the directory entry.
                             */
                            done = 1;
                            ret = (char *)p;  /* Pointer to the block containing CFERAM directory entry in the image to be flashed last to finalize image */
                            (*found)++;
                            break;
                        }

                        buf += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                    }
                    else
                    {
                        done = 1;
                        break;
                    }
                }
            }
        }
    }

    if (seq != -1)
    { // update image_update sequence number for the image that is to be written
        FILE *fp;

        if(nandGetBootPartition() == 1)
            fp = fopen("/tmp/seqNumSecondImg", "w");
        else
            fp = fopen("/tmp/seqNumFirstImg", "w");

        if (fp != NULL)
        {
            fwrite(&seq, sizeof(seq), 1, fp);
            fclose(fp);
        }
    }

    return((unsigned char *)ret);
}


static int get_image_version( uint8_t *imagePtr, int imageSize, int erasesize, char *image_name, int image_name_len )
{
    int type;
    unsigned char *buf, *p;
    char fname[] = IMAGE_VERSION_FILE_NAME;
    int fname_len = strlen(fname);
    struct jffs2_raw_dirent *pdir;
    struct jffs2_raw_inode *pino;
    unsigned long version = 0;
    jint32_t ino;
    int ret = -1;

    while (imageSize > 0)
    {
        type = nand_image_type(imagePtr);
        if ((type == JFFS2_IMAGE) || (type == UBI_IMAGE))
            break;

        imagePtr += erasesize;
        imageSize -= erasesize;
    }

    /* Confirm that we did find a magic bitmask. If not, we are done */
    if (imageSize <= 0)
    {
        return -1;
    }

    if (type == JFFS2_IMAGE)
    {
        for(buf = imagePtr; buf < imagePtr+imageSize; buf += 4)
        {
            p = buf;
            while( p < buf + erasesize )
            {
                pdir = (struct jffs2_raw_dirent *) p;
                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK  &&
                      je32_to_cpu(pdir->hdr_crc) == genUtl_getCrc32(p,
                            sizeof(struct jffs2_unknown_node) - 4, 0) )
                {
                    if( je16_to_cpu(pdir->nodetype) == JFFS2_NODETYPE_DIRENT)
                    {

                        if(
                           fname_len == pdir->nsize &&
                           !memcmp(fname, pdir->name, fname_len) &&
                           je32_to_cpu(pdir->version) > version &&
                           je32_to_cpu(pdir->ino) != 0 )
                        {
                            ino.v32=je32_to_cpu(pdir->ino);
                            je32_to_cpu(pdir->name_crc) = genUtl_getCrc32(pdir->name,
                            (unsigned long) fname_len, 0);
                            version = je32_to_cpu(pdir->version);
                            p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                            pino = (struct jffs2_raw_inode *) p;
                            if( je16_to_cpu(pino->magic) == JFFS2_MAGIC_BITMASK )
                            {
                                if(je16_to_cpu(pino->nodetype)==JFFS2_NODETYPE_INODE &&
                                je32_to_cpu(pino->ino) == ino.v32)
                                {
                                    if(image_name != NULL)
                                    {
                                        memcpy(image_name, pino->data, (int)pino->dsize.v32 > image_name_len ? image_name_len : (int)pino->dsize.v32);
                                        return 0;
                                    }
                                    else
                                        break;
                                }
                            }
                        }
                    }
                    p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                }
                else
                    break;
            }
        }
    }

    if (!image_name)
        return(ret);

    for(buf = imagePtr; (buf < imagePtr+imageSize) && (ret == -1); buf++)
    { // search whole image for version string
        if (!memcmp(buf, IDENT_TAG, strlen(IDENT_TAG)))
        {
            unsigned char * start;
            int size = 0;

            buf += 20;
            start = buf;

            while ((*buf >= ' ') && (*buf <= '~') && (buf < imagePtr+imageSize) && (size < 128))
            {
                if (!memcmp(buf, " $\n", 3))
                { // found terminator, done
                    memcpy(image_name, start, size);
                    image_name[size] = 0;
                    ret = 0;
                    break;
                }

                buf++;
                size++;
            }
        }
    }

    return(ret);
}

#ifndef DESKTOP_LINUX
// NAND flash bcm image
// return:
// 0 - ok
// !0 - the sector number fail to be flashed (should not be 0)
static int bcmNandImageSet( char *rootfs_part, unsigned char *image_ptr, int img_size, NVRAM_DATA * inMemNvramData_buf)
{
    int sts = -1;
    int blk_addr;
    unsigned char *cferam_ptr;
    int cferam_found = 0;
    int rsrvd_for_cferam;
    unsigned char *end_ptr = image_ptr + img_size;
    int mtd0_fd;
    mtd_info_t * mtd0 = NULL;
    WFI_TAG wt = {0,0,0,0,0};
    int nvramXferSize;

    uint32_t btrmEnabled = otp_is_btrm_boot();

    if( !nandIsLegacyFlashLayout() )
    {
        printf("%s: WARNING:Function not implemented for new flash layout\n", __FUNCTION__);
        return 0;
    }

    mtd0 = get_mtd_device_handle("image", &mtd0_fd, 0);

    if( !IS_ERR_OR_NULL(mtd0) )
    {
        unsigned int chip_id;
        chip_id = get_chip_id();

        int blksize = mtd0->erasesize / 1024;

        /* Reserve room to flash block containing directory entry for CFERAM. */
        rsrvd_for_cferam = NAND_CFE_RAM_RSVD_BLK * mtd0->erasesize;

        memcpy(&wt, end_ptr, sizeof(wt));

#if defined(CHIP_FAMILY_ID_HEX)
        chip_id = CHIP_FAMILY_ID_HEX;
#endif
        if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            wt.wfiChipId != chip_id )
        {
            int id_ok = 0;

            if (id_ok == 0) {
                fprintf(stderr, "Chip Id error.  Image Chip Id = %x, Board Chip Id = "
                    "%x\n", wt.wfiChipId, chip_id);
                put_mtd_device(mtd0, mtd0_fd, -1);
                return -1;
            }
        }
        else if( wt.wfiFlashType == WFI_NOR_FLASH )
        {
            fprintf(stderr, "\nERROR: Image does not support a NAND flash device.\n\n");
            put_mtd_device(mtd0, mtd0_fd, -1);
            return -1;
        }
        else if( (wt.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            ((wt.wfiFlashType < WFI_NANDTYPE_FLASH_MIN && wt.wfiFlashType > WFI_NANDTYPE_FLASH_MAX) ||
              blksize != WFI_NANDTYPE_TO_BKSIZE(wt.wfiFlashType) ) )
        {
            fprintf(stderr, "\nERROR: NAND flash block size %dKB does not work with an "
                "image built with %dKB block size\n\n", blksize,WFI_NANDTYPE_TO_BKSIZE(wt.wfiFlashType));
            put_mtd_device(mtd0, mtd0_fd, -1);
            return -1;
        }
        else if ( (get_flash_type() != FLASH_IFC_SPINAND) &&
                (((  (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) && (! btrmEnabled)) ||
                 ((! (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) && (  btrmEnabled))) )
        {
            fprintf(stderr, "The image type does not match the OTP configuration of the SoC. Aborting.\n");
            put_mtd_device(mtd0, mtd0_fd, -1);
            return -1;
        }
        else
        {
            put_mtd_device(mtd0, mtd0_fd, -1);
            mtd0 = get_mtd_device_handle(rootfs_part, &mtd0_fd, 0);

            if( IS_ERR_OR_NULL(mtd0) )
            {
                fprintf(stderr, "ERROR!!! Could not access MTD partition %s\n", rootfs_part);
                return -1;
            }

            if( mtd0->size == 0LL )
            {
                fprintf(stderr, "ERROR!!! Flash device is configured to use only one file system!\n");
                put_mtd_device(mtd0, mtd0_fd, -1);
                return -1;
            }
        }
    }

    if( !IS_ERR_OR_NULL(mtd0) )
    {
        int ofs;
        int writelen;
        int writing_ubifs;

        if( checkImageSizeForNand(mtd0, mtd0_fd, image_ptr, img_size) )
        {
            put_mtd_device(mtd0, mtd0_fd, -1);
            return -1;
        }

        if( nand_image_type(image_ptr) )
        { /* Downloaded image does not contain CFE ROM boot loader */
            ofs = 0;
        }
        else
        {
            /* Downloaded image contains CFE ROM boot loader. */
            PNVRAM_DATA pnd = (PNVRAM_DATA) (image_ptr + nvramSector(mtd0) * ((unsigned int)mtd0->writesize) + nvramDataOffset(mtd0));

	    if( verifyImageDDRType(wt.wfiFlags, inMemNvramData_buf) != 0 ) {
                put_mtd_device(mtd0, mtd0_fd, -1);
                return -1;
            }

            ofs = mtd0->erasesize;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(BCM_XRDP)
#if defined(CONFIG_BCM94908)
            // if we are flashing a NAND device with CFEROM image
            if(get_flash_type() == FLASH_IFC_NAND ) {
                if(nand_image_type(image_ptr) == 0)
                {
                    int remain = IMAGE_OFFSET%mtd0->erasesize;
                    memset(image_ptr, '\0', remain);
                }
            }
            else if(get_flash_type() != FLASH_IFC_SPINAND) {
#endif
                /* check if it is zero padded for backward compatiblity */
                if( (wt.wfiFlags&WFI_FLAG_HAS_PMC) == 0 )
                {
                    unsigned int *pImg  = (unsigned int *)image_ptr;
                    unsigned char * pBuf = (unsigned char *)image_ptr;
                    int block_start, block_end, remain, block;
                    mtd_info_t *mtd1;
                    int mtd1_fd;

                    if( *pImg == 0 && *(pImg+1) == 0 && *(pImg+2) == 0 && *(pImg+3) == 0 )
                    {
                        /* the first 64KB are for PMC in 631x8, need to preserve that for cfe/linux image update if it is not for PMC image update. */
                        block_start = 0;
                        block_end = IMAGE_OFFSET/mtd0->erasesize;
                        remain = IMAGE_OFFSET%mtd0->erasesize;

                        mtd1 = get_mtd_device_handle("nvram", &mtd1_fd, 0);
                        if( !IS_ERR_OR_NULL(mtd1) )
                        {
                            for( block = block_start; block < block_end; block++ )
                            {
                                nandReadBlk(mtd1, block*mtd1->erasesize, mtd1->erasesize, pBuf, mtd1_fd);
                                pBuf += mtd1->erasesize;
                            }

                            if( remain )
                            {
                                block = block_end;
                                nandReadBlk(mtd1, block*mtd1->erasesize, remain, pBuf, mtd1_fd);
                            }

                            put_mtd_device(mtd1, mtd1_fd, -1);
                        }
                        else
                        {
                            fprintf(stderr, "Failed to get nvram mtd device\n");
                            put_mtd_device(mtd0, mtd0_fd, -1);
                            return -1;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Invalid NAND image.No PMC image or padding\n");
                        put_mtd_device(mtd0, mtd0_fd, -1);
                        return -1;
                    }
                }
#if defined(CONFIG_BCM94908)
            }
#endif
#endif

            nvramXferSize = sizeof(NVRAM_DATA);
            /* Copy NVRAM data to block to be flashed so it is preserved. */
            memcpy((unsigned char *) pnd, inMemNvramData_buf, nvramXferSize);

            /* Recalculate the nvramData CRC. */
            pnd->ulCheckSum = 0;
            pnd->ulCheckSum = genUtl_getCrc32((unsigned char *)pnd, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
        }

        /*
         * Scan downloaded image for cferam.000 directory entry and change file extension
         * to cfe.YYY where YYY is the current cfe.XXX + 1. If full secure boot is in play,
         * the file to be updated is secram.000 and not cferam.000
         */
        cferam_ptr = nandUpdateSeqNum(image_ptr, img_size, mtd0->erasesize, -1, &cferam_found);

        if( (cferam_ptr == NULL) || !cferam_found)
        {
            fprintf(stderr, "\nERROR: Invalid image. ram.000 not found.\n\n");
            put_mtd_device(mtd0, mtd0_fd, -1);
            return -1;
        }

#if defined(BCM_PON) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
        if ((wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) && (ofs != 0))
        {
            /* These targets support bootrom boots which is currently enabled. the "nvram" */
            /* mtd device may be bigger than just the first nand block. Check that the new */
            /* image plays nicely with the current partition table settings. */
            int mtd1_fd;
            mtd_info_t *mtd1 = get_mtd_device_handle("nvram", &mtd1_fd, 0);
            if( !IS_ERR_OR_NULL(mtd1) )
            {
                uint32_t *pHdr = (uint32_t *)image_ptr;
                pHdr += (mtd1->erasesize / 4); /* pHdr points to the top of the 2nd nand block */
                for( blk_addr = mtd1->erasesize; blk_addr < (int) mtd1->size; blk_addr += mtd1->erasesize )
                {
                    /* If we are inside the for() loop, "nvram" mtd is larger than 1 block */
                    pHdr += (mtd1->erasesize / 4);
                }

                if ( !nand_image_type((unsigned char *)pHdr) )
                {
                    fprintf(stderr, "New sw image does not match the partition table. Aborting.\n");
                    put_mtd_device(mtd0, mtd0_fd, -1);
                    put_mtd_device(mtd1, mtd1_fd, -1);
                    return -1;
                }
                put_mtd_device(mtd1, mtd1_fd, -1);
            }
            else
            {
                fprintf(stderr, "Failed to get nvram mtd device\n");
                put_mtd_device(mtd0, mtd0_fd, -1);
                return -1;
            }
        }
#endif

        { // try to make sure we have enough memory to program the image
            char * temp;
            FILE *fp;
            int status = -ENOMEM;

            if ( (fp = fopen("/proc/sys/vm/drop_caches","w")) != NULL )
            { // clear the caches
                fwrite("3\n", sizeof(char), 2, fp);
                fclose(fp);
            }

            if ( (temp = calloc(mtd0->erasesize, sizeof(char))) != NULL )
            {
                status = (mread(mtd0_fd, temp, mtd0->erasesize));
                free(temp);
            }

            if ((temp == NULL) || (status == -ENOMEM))
            {
                fprintf(stderr, "Failed to allocate memory, aborting image write!!!\n");
                put_mtd_device(mtd0, mtd0_fd, -1);
                return -1;
            }
        }

        if( 0 != ofs ) /* Image contains CFE ROM boot loader. */
        {
            /* Prepare to flash the CFE ROM boot loader. */
            int mtd1_fd;
            mtd_info_t *mtd1 = get_mtd_device_handle("nvram", &mtd1_fd, 0);

            if( !IS_ERR_OR_NULL(mtd1) )
            {
                int iterations = 10;
                int status = -1;

                while(iterations--)
                {
                    if (nandEraseBlk(mtd1, 0, mtd1_fd) == 0)
                    {
                        if ((status = nandWriteBlk(mtd1, 0, mtd1->erasesize, image_ptr, mtd1_fd, 1)) == 0)
                            break;

                        printf("ERROR WRITING CFE ROM BLOCK!!!\n");
                    }
                }

                if (status)
                {
                    printf("Failed to write CFEROM, quitting\n");
                    put_mtd_device(mtd0, mtd0_fd, -1);
                    put_mtd_device(mtd1, mtd1_fd, -1);
                    return -1;
                }

                image_ptr += ofs;

#if defined(BCM_PON) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
                if (wt.wfiFlags & WFI_FLAG_SUPPORTS_BTRM)
                {
                    /* We have already checked that the new sw image matches the partition table. Therefore */
                    /* burn the rest of the "nvram" mtd (if any) */
                    for( blk_addr = mtd1->erasesize; blk_addr < (int) mtd1->size; blk_addr += mtd1->erasesize )
                    {
                        if (nandEraseBlk(mtd1, blk_addr, mtd1_fd) == 0)
                        {
                            if (nandWriteBlk(mtd1, blk_addr, mtd1->erasesize, image_ptr, mtd1_fd, 1) != 0)
                                printf("ERROR WRITING BLOCK!!! at address 0x%x wihin NAND partition nvram\n", blk_addr);

                            image_ptr += ofs;
                        }
                    }
                }
#endif

                put_mtd_device(mtd1, mtd1_fd, -1);
            }
            else
            {
                fprintf(stderr, "Failed to get nvram mtd device!!!\n");
                put_mtd_device(mtd0, mtd0_fd, -1);
                return -1;
            }
        }

        /* Erase blocks containing directory entry for CFERAM before flashing the image. */
        for( blk_addr = 0; blk_addr < rsrvd_for_cferam; blk_addr += mtd0->erasesize )
        {
            nandEraseBlk(mtd0, blk_addr, mtd0_fd);
        }

        /* Flash the image except for CFERAM directory entry, during which all the blocks in the partition (other than CFE) will be erased */
        writing_ubifs = 0;
        for( blk_addr = rsrvd_for_cferam; blk_addr < (int) mtd0->size; blk_addr += mtd0->erasesize )
        {
            printf(".");

            if (nandEraseBlk(mtd0, blk_addr, mtd0_fd) == 0)
            { // block was erased successfully, no need to put clean marker in a block we are writing JFFS2 data to but we do it for backward compatibility
                if ( image_ptr == cferam_ptr )
                { // skip CFERAM directory entry block and back the block pointer up a block
                    image_ptr += mtd0->erasesize;
                    blk_addr -= mtd0->erasesize;
                }
                else
                { /* Write a block of the image to flash. */
                    if( image_ptr < end_ptr )
                    { // if any data left, prepare to write it out
                        writelen = ((image_ptr + mtd0->erasesize) <= end_ptr)
                            ? (int) mtd0->erasesize : (int) (end_ptr - image_ptr);
                    }
                    else
                        writelen = 0;

                    if (writelen) /* Write data with or without JFFS2 clean marker */
                    {
                        if (nandWriteBlk(mtd0, blk_addr, writelen, image_ptr, mtd0_fd, !writing_ubifs) != 0 )
                        {
                            printf("Error writing Block 0x%8.8x, skipping\n", blk_addr);
                        }
                        else
                        { // successful write, increment counter and check for UBIFS split marker if data was written
                            image_ptr += writelen;

                            if (!strncmp(BCM_BCMFS_TAG, (char *)image_ptr - 0x100, strlen(BCM_BCMFS_TAG)))
                            {
                                if (!strncmp(BCM_BCMFS_TYPE_UBIFS, (char *)image_ptr - 0x100 + strlen(BCM_BCMFS_TAG), strlen(BCM_BCMFS_TYPE_UBIFS)))
                                { // check for UBIFS split marker
                                    writing_ubifs = 1;
                                    printf("U");
                                }
                            }
                        }
                    }
                }
            }
            fflush(stdout);
        }

        printf("\n\n");

        sts = flashCferam(mtd0, mtd0_fd, rsrvd_for_cferam, cferam_ptr, 0);

        if( sts )
        {
            /*
             * Even though we try to recover here, this is really bad because
             * we have stopped the other CPU and we cannot restart it.  So we
             * really should try hard to make sure flash writes will never fail.
             */
            printf("nandWriteBlk: write failed at blk=%d\n", blk_addr);
            sts = (blk_addr > (int) mtd0->erasesize) ? blk_addr / mtd0->erasesize : 1;
        }
    }

    if( !IS_ERR_OR_NULL(mtd0) )
    {
        put_mtd_device(mtd0, mtd0_fd, -1);
    }

    return sts;
}
#endif /* DESKTOP_LINUX */


#ifdef DESKTOP_LINUX
int writeImageToNand( unsigned char *string __attribute__((unused)), int size __attribute__((unused)))
{
    return SUCCESS; 
}
#else /* DESKTOP_LINUX */
int writeImageToNand( unsigned char *string, int size )
{
    NVRAM_DATA * pNvramData;
    int ret = SUCCESS;

    if (NULL == (pNvramData = malloc(sizeof(NVRAM_DATA))))
    {
        fprintf(stderr, "Memory allocation failed");
        return ERROR;
    }

    // Get a copy of the nvram before we do the image write operation
    if (nandReadNvramData(pNvramData))
    {
        unsigned int flags=0;
        ret = getFlashInfo(&flags);
        if (!IS_ERR(ret))
        {
           if (flags & FLASH_INFO_FLAG_NAND)
           { /* NAND flash */
              char *rootfs_part = "image_update";
              int rc;

              rc = bcmNandImageSet(rootfs_part, string, size, pNvramData);
              if (rc != 0)
              {
                 fprintf(stderr, "bcmNandImageSet failed, rc=%d", rc);
                 ret = ERROR;
              }
           }
           else
           { /* NOR flash */
              fprintf(stderr, "This function should not be called when using NOR flash, flags=0x%x", flags);
              ret = ERROR;
           }
        }
    }
    else
    {
        ret = ERROR;
    }

    free(pNvramData);

    return ret;
}
#endif /* DESKTOP_LINUX */


int validateWfiTag(void *wtp, int blksize, uint32_t btrmEnabled)
{
    unsigned int chip_id = 0;
    WFI_TAG *wt = (WFI_TAG*)wtp;

    chip_id = get_chip_id();

#if defined(CHIP_FAMILY_ID_HEX)
    chip_id = CHIP_FAMILY_ID_HEX;
#endif

#ifdef DESKTOP_LINUX
    chip_id = wt->wfiChipId;
#endif

    if( (wt->wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
        wt->wfiChipId != chip_id )
    {
        flashutil_error("Chip Id error. Image Chip Id = %x, Board Chip Id = %x",
          wt->wfiChipId, chip_id);
        return -1;
    }
    else if( wt->wfiFlashType == WFI_NOR_FLASH )
    {
        flashutil_error("Image does not support a NAND flash device.");
        return -1;
    }
    else if( (wt->wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
        ((wt->wfiFlashType < WFI_NANDTYPE_FLASH_MIN && wt->wfiFlashType > WFI_NANDTYPE_FLASH_MAX) ||
        blksize != WFI_NANDTYPE_TO_BKSIZE(wt->wfiFlashType) ) )
    {
        flashutil_error("NAND flash block size %dKB does not work with an "
          "image built with %dKB block size", blksize, WFI_NANDTYPE_TO_BKSIZE(wt->wfiFlashType));
        return -1;
    }
    else if ( (get_flash_type() != FLASH_IFC_SPINAND) &&
            ((( (wt->wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) && (! btrmEnabled)) ||
             ((! (wt->wfiFlags & WFI_FLAG_SUPPORTS_BTRM)) && (  btrmEnabled))) )
    {
        flashutil_error("The image type does not match the OTP configuration of the SoC. Aborting.");
        return -1;
    }

    return 0;
}

int flashCferam(mtd_info_t *mtd, int mtd_fd, int rsrvd_for_cferam, unsigned char *cferam_ptr, unsigned char *cferam_ptr2)
{
    int blk_addr;
    int first_done = 0;

    flashutil_debug("enter, mtd=%p, fd=%d, rsrvd_for_cferam=0x%x",
      mtd, mtd_fd, rsrvd_for_cferam);

    for( blk_addr = 0; blk_addr < rsrvd_for_cferam; blk_addr += mtd->erasesize )
    {
        if( nandIsBadBlk(blk_addr, mtd_fd) == 0 ) 
        {
            /* Write CFERAM sequence number block of the image to flash. */
            if (!first_done)
            {
                if (nandWriteBlk(mtd, blk_addr, cferam_ptr ? mtd->erasesize : 0, cferam_ptr, mtd_fd, 1) == 0)
                { // rsrvd_for_cferam number of blocks reserved for CFERAM block in order to guarantee at least one block good, CFERAM block with cferam entry written last, write only one block and leave the rest blank, cferom/filesystem must patch this back together
                    printf(".");

                    if (!cferam_ptr2)
                    {
                        fflush(stdout);
                        return(0);
                    }

                    first_done = 1;
                    continue;
                }
            }
            else
            {
                if (nandWriteBlk(mtd, blk_addr, cferam_ptr2 ? mtd->erasesize : 0, cferam_ptr2, mtd_fd, 1) == 0)
                { // write second CFERAM block if there is one
                    printf(".");
                    fflush(stdout);

                    return(0);
                }
            }
        }
    }

    return(-1);
}

/*
	hunt_ptr - pointer to the buffer that holds the gen3 boot block
		the boot block  contains NVRAM_DATA and the CFEROM images
		NVRAM_DATA images is prefixed with "nVrAmDat" signature
		whereas CFEROM has its own header and signature
	size_to_search - size of the boot block, typically 1MB
	nvram_mirror_info -  this function will populate this structure based on what is found in the input buffer
*/
static void populate_boot_block_info_struct(char *hunt_ptr, int size_to_search, PNVRAM_DATA pnd, BOOT_BLOCK_MIRROR_INFO *nvram_mirror_info, mtd_info_t *mtd)
{
    char *saved_hunt_ptr=hunt_ptr;
    int nvram_mirr_info_idx=0, rom_size=0, cferom_crc=0;

    if(hunt_ptr != NULL && nvram_mirror_info != NULL)
    {
        memset(nvram_mirror_info,'\0',sizeof(BOOT_BLOCK_MIRROR_INFO));
        nvram_mirror_info->active_idx=0; // always make the primary as active
        nvram_mirror_info->offset[nvram_mirr_info_idx]=nvramDataOffset(mtd);
        nvram_mirror_info->image_type[nvram_mirr_info_idx]=IMG_NVRAM;
        nvram_mirror_info->write_fail_count[nvram_mirr_info_idx]=0;
       	nvram_mirror_info->cferom_crc=0;
        nvram_mirr_info_idx++;


        while(size_to_search > 2048)
        {
            if(is_nvram_offset((unsigned char*)hunt_ptr, 0, 1, NULL, NULL) == IMG_NVRAM)
            {
                nvram_mirror_info->offset[nvram_mirr_info_idx]=(hunt_ptr - saved_hunt_ptr);
                nvram_mirror_info->image_type[nvram_mirr_info_idx]=IMG_NVRAM;
                nvram_mirror_info->write_fail_count[nvram_mirr_info_idx]=0;
                nvram_mirr_info_idx++;
                memcpy(hunt_ptr+strlen(NVRAM_DATA_SIGN), (unsigned char *)pnd, sizeof(NVRAM_DATA));
            }

            else  if(is_cferom_offset((unsigned char*)hunt_ptr,  0, &rom_size, &cferom_crc, NULL) == IMG_CFEROM)
            {
                nvram_mirror_info->offset[nvram_mirr_info_idx]=(hunt_ptr - saved_hunt_ptr);
                nvram_mirror_info->image_type[nvram_mirr_info_idx]=IMG_CFEROM;
                nvram_mirror_info->image_size[nvram_mirr_info_idx]=rom_size;
                nvram_mirror_info->write_fail_count[nvram_mirr_info_idx]=0;
                if(nvram_mirror_info->cferom_crc == 0)
                    nvram_mirror_info->cferom_crc=cferom_crc;
                nvram_mirr_info_idx++;
            }
            hunt_ptr+=1024;
            size_to_search-=1024;
        }
    }
}
/*
	Write the nvram_mirror_info struture to BOOT_BLOCK_MIRROR_INFO_FILE file
*/
static void write_boot_block_info_file(BOOT_BLOCK_MIRROR_INFO *nvram_mirror_info)
{
    int fd=0;
    int status=0;

    fd=open(BOOT_BLOCK_MIRROR_INFO_FILE, O_CREAT|O_WRONLY|O_TRUNC, 0644);

    if(fd>=0)
    {
        status=write(fd, nvram_mirror_info, sizeof(*nvram_mirror_info));
        if(status == -1)
            printf("error writing %s file\n", BOOT_BLOCK_MIRROR_INFO_FILE);
        close(fd);
        sync();
    }

}
/*
	populate the nvram_mirror_info by searching through input buffer
	and write the struture to BOOT_BLOCK_MIRROR_INFO_FILE file
*/
void create_boot_block_info_file(char *ptr, int size, PNVRAM_DATA pnd, mtd_info_t *mtd )
{
    BOOT_BLOCK_MIRROR_INFO nvram_mirror_info;
    populate_boot_block_info_struct(ptr, size, pnd, &nvram_mirror_info, mtd);
    write_boot_block_info_file(&nvram_mirror_info);
}

int handleCferom(mtd_info_t *mtd, char *image_ptr,unsigned int image_size __attribute__ ((__unused__)), unsigned int wfiFlags, void *inMemNvramData)
{
    int nvramXferSize;
    NVRAM_DATA *inMemNvramData_buf = (NVRAM_DATA*)inMemNvramData;

    /* Downloaded image contains CFE ROM boot loader. */
    PNVRAM_DATA pnd = (PNVRAM_DATA) (image_ptr + nvramSector(mtd) * ((unsigned int)mtd->writesize) + nvramDataOffset(mtd));

    flashutil_debug("enter, mtd=%p, image=%p, wfiFlags=%d, nvram=%p",
      mtd, image_ptr, wfiFlags, inMemNvramData_buf);

    if( verifyImageDDRType(wfiFlags, inMemNvramData_buf) != 0 )
        return -1;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908)
#if defined(CONFIG_BCM94908)
    if(get_flash_type() == FLASH_IFC_NAND ) {
        if(nand_image_type((unsigned char*)image_ptr) == 0)
        {
            int remain = IMAGE_OFFSET%mtd->erasesize;
            memset(image_ptr, '\0', remain);
        }
    }
    else if(get_flash_type() != FLASH_IFC_SPINAND) {
#endif
    /* check if it is zero padded for backward compatiblity */
    if( (wfiFlags&WFI_FLAG_HAS_PMC) == 0 )
    {
        unsigned int *pImg  = (unsigned int*)image_ptr;
        unsigned char * pBuf = (unsigned char*)image_ptr;
        int block_start, block_end, remain, block;
        mtd_info_t *mtd1;
        int mtd1_fd;

        if( *pImg == 0 && *(pImg+1) == 0 && *(pImg+2) == 0 && *(pImg+3) == 0 )
        {
            /* the first 64KB are for PMC in 631x8, need to preserve that for cfe/linux image update if it is not for PMC image update. */
            block_start = 0;
            block_end = IMAGE_OFFSET/mtd->erasesize;
            remain = IMAGE_OFFSET%mtd->erasesize;

            mtd1 = get_mtd_device_handle("nvram", &mtd1_fd, 0);
            if( !IS_ERR_OR_NULL(mtd1) )
            {
                for( block = block_start; block < block_end; block++ )
                {
                    nandReadBlk(mtd1, block*mtd1->erasesize, mtd1->erasesize, pBuf, mtd1_fd);
                    pBuf += mtd1->erasesize;
                }

                if( remain )
                {
                    block = block_end;
                    nandReadBlk(mtd1, block*mtd1->erasesize, remain, pBuf, mtd1_fd);
                }

                put_mtd_device(mtd1, mtd1_fd, -1);
            }
            else
            {
                fprintf(stderr, "Failed to get nvram mtd device\n");
                return -1;
            }
        }
        else
        {
            fprintf(stderr, "Invalid NAND image.No PMC image or padding\n");
            return -1;
        }
    }
#if defined(CONFIG_BCM94908)
   }
#endif
#else
    flashutil_print("wfiFlags=%d", wfiFlags);
#endif

    nvramXferSize = sizeof(NVRAM_DATA);

    /* Copy NVRAM data to block to be flashed so it is preserved. */
    memcpy((unsigned char *) pnd, inMemNvramData_buf, nvramXferSize);

    /* Recalculate the nvramData CRC. */
    pnd->ulCheckSum = 0;
    pnd->ulCheckSum = genUtl_getCrc32((unsigned char *)pnd, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);

    unlink(BOOT_BLOCK_MIRROR_INFO_FILE);

    return 0;
}


int flashCferom(unsigned char *image_ptr, unsigned int size)
{
    /* Prepare to flash the CFE ROM boot loader. */
    int mtd_fd;
    mtd_info_t *mtd = get_mtd_device_handle("nvram", &mtd_fd, 0);
    int address = 0;
    int status = 0;
    BOOT_BLOCK_MIRROR_INFO nvram_mirror_info;

    flashutil_debug("enter, image=%p, size=%d",
      image_ptr, size);

    if( !IS_ERR_OR_NULL(mtd) )
    {
        /* Downloaded image contains CFE ROM boot loader. */
        PNVRAM_DATA pnd = (PNVRAM_DATA) (image_ptr + nvramSector(mtd) * ((unsigned int)mtd->writesize) + nvramDataOffset(mtd));

        printf("\n+\n");

        populate_boot_block_info_struct((char*)image_ptr, size, pnd, &nvram_mirror_info, mtd);  //skip first page as 4908 has precferom

        while(size)
        {
            int iterations = 3;

            while(iterations--)
            {
                if (nandEraseBlk(mtd, address, mtd_fd) == 0)
                {
                    if (nandWriteBlk(mtd, address, mtd->erasesize, image_ptr, mtd_fd, 1) != 0)
                    {
                        printf("ERROR WRITING CFE ROM BLOCK AT ADDRESS 0x%x!!!\n", address);
                        if (iterations == 1)
                            status = -1;
                    }
                    else
                        break;
                }
                else
                {
                    printf("ERROR ERASING CFEROM BLOCK AT ADDRESS 0x%x!!!\n", address);
                    if (iterations == 1)
                        status = -1;
                }
            }

            image_ptr += mtd->erasesize;
            address += mtd->erasesize;
            size -= mtd->erasesize;
        }

        put_mtd_device(mtd, mtd_fd, -1);
    }
    else
    {
        flashutil_error("Failed to get nvram mtd device!!!");
        return -1;
    }
    printf("\n+\n");
    if(get_flash_type() == FLASH_IFC_NAND || get_flash_type() == FLASH_IFC_SPINAND ) {
        write_boot_block_info_file(&nvram_mirror_info);
        printf("\n+\n");
    }

    return(status);
}

int nandFlashLoader(unsigned char *file_name, uint64_t file_size) 
{
    /* Prepare to flash the uboot boot loader. */
    int mtd_fd;
    mtd_info_t *mtd = get_mtd_device_handle("loader", &mtd_fd, 0);
    int address = 0;
    int status;
    FILE* file_ptr;
    unsigned char * image_ptr = NULL;
    int bytes_read = 0;
    int part_size;

    flashutil_debug("enter, image=%p, size=%d",
      image_ptr, file_size);

    if( !IS_ERR_OR_NULL(mtd) )
    {
        image_ptr = malloc(mtd->erasesize);
        if( !image_ptr )
        {
            printf("%s: Error, cannot allocate memory for writing\n", __FUNCTION__);
            put_mtd_device(mtd, mtd_fd, -1);
            return -1;
        }

        file_ptr = fopen((char*)file_name, "r");
        if( !file_ptr )
        {
            printf("%s: Error, cannot open %s\n", __FUNCTION__, file_name);
            free( image_ptr );
            put_mtd_device(mtd, mtd_fd, -1);
            return -1;
        }

        part_size = mtd->size;
        status = 0;

        flashutil_debug(">>>>>>>> partition size 0x%x, file size 0x%llx <<<<<<<<\n", part_size, file_size);

        while((file_size > 0) && (part_size > 0))
        {
            if (!status)
                bytes_read = fread(image_ptr, mtd->erasesize, 1, file_ptr); 

            status = -1;
            if( bytes_read )
            {
                if( nandIsBadBlk(address, mtd_fd) == 0 )
                {
                    int iterations = 3;
                    while(iterations--)
                    {
                        if (nandEraseBlk(mtd, address, mtd_fd) == 0)
                        {
                            if (nandWriteBlk(mtd, address, mtd->erasesize, image_ptr, mtd_fd, 0) != 0)
                            {
                                printf("ERROR WRITING LOADER AT ADDRESS 0x%x!!!\n", address);
                            }
                            else
                            {
                                status = 0;
                                break;
                            }
                        }
                        else
                        {
                            printf("ERROR ERASING LOADER BLOCK AT ADDRESS 0x%x!!!\n", address);
                        }
                    }
                }

                if (status)
                    printf("Skipping bad block at 0x%x\n", address);

            }

            flashutil_debug(">>>>>>>> %s file offset 0x%x to partition offset 0x%x <<<<<<<<\n", status ? "did not write" : "wrote", mtd->size - (int)file_size, address);

            address += mtd->erasesize;
            part_size -= mtd->erasesize;

            if (!status)
                file_size -= mtd->erasesize;
        }

        fclose(file_ptr);
        free(image_ptr);
        put_mtd_device(mtd, mtd_fd, -1);

    }
    else
    {
        flashutil_error("Failed to get loader mtd device!!!");
        return -1;
    }

    printf("Flashing loader finished\n");

    return(0);
}

int nandGetImageVersion(uint8_t *imagePtr, int imageSize, char *image_name, int image_name_len)
{    
    mtd_info_t *mtd;
    int mtd_fd;
    int ret = -1;

    mtd = get_mtd_device_handle("image", &mtd_fd, 0);
    if (!IS_ERR_OR_NULL(mtd))
    {
        ret = get_image_version(imagePtr, imageSize, mtd->erasesize, image_name, image_name_len);
        put_mtd_device(mtd, mtd_fd, -1);
    }
    return ret;
}

uint64_t nandGetAvailImgSpace( int update_img_idx )
{ 
    char sysfs_path[128];
    uint64_t erase_blk_size = 0;
    uint64_t avail_erase_blks = 0;
    uint64_t avail_size = 0;
    
    sprintf(sysfs_path,"/sys/class/ubi/ubi0/eraseblock_size"); 
    if ((erase_blk_size = getSysfsBytes(sysfs_path)) > 0x100000)
    {
        fprintf(stderr, "ERROR!!! block size larger than maximum sanity check block size!\n");
        return(0);
    }

    sprintf(sysfs_path,"/sys/class/ubi/ubi0/avail_eraseblocks");
    avail_erase_blks = getSysfsBytes(sysfs_path);
    avail_size = erase_blk_size * avail_erase_blks;

    /* Bootfs size */
    sprintf(sysfs_path,"/sys/class/ubi/ubi0_%d/reserved_ebs",(update_img_idx==1?3:5));
    avail_erase_blks = getSysfsBytes(sysfs_path);
    avail_size += erase_blk_size * avail_erase_blks;
    
    /* Rootfs size */
    sprintf(sysfs_path,"/sys/class/ubi/ubi0_%d/reserved_ebs",(update_img_idx==1?4:6));
    avail_erase_blks = getSysfsBytes(sysfs_path);
    avail_size += erase_blk_size * avail_erase_blks;

    return avail_size;
}

uint64_t nandGetAvailLoaderSpace(void) 
{
    uint64_t loader_size = 0;
    int mtd_fd;
    mtd_info_t *mtd = get_mtd_device_handle("loader", &mtd_fd, 0);

    if (mtd)
    {
        loader_size = mtd->size;
        put_mtd_device(mtd, mtd_fd, -1);
    }

    return loader_size;
}

int get_mtd_master_size(unsigned int *psize __attribute__((unused)))
{
#ifndef DESKTOP_LINUX
    FILE *fp;
    char line[256]={0};
    char *mtd_master = NULL, *mtd_end = NULL;
    mtd_info_t *mtd;
    int mtd_fd;

    fp = fopen("/proc/cmdline","r");
    if (fp == NULL)
    {
       fprintf(stderr, "Could not open /proc/cmdline");
       return 0;
    }
    /* Search for 'mtdparts=' from cmdline for mtd master*/
    while(fgets(line, sizeof(line), fp))
    {
        if ((mtd_master = strstr(line, "mtdparts=")))
            break;
    }
    fclose(fp);

    if (!mtd_master)
    {
       fprintf(stderr, "failed to find mtd master from cmdline");
       return 0;
    }
    mtd_master += 9;
    mtd_end = strchr(mtd_master, ':');
    if (!mtd_end)
    {
       fprintf(stderr, "invalid mtd master format %s", mtd_master);
       return 0;
    }
    *mtd_end = '\0';
    mtd = get_mtd_device_handle(mtd_master, &mtd_fd, 0);
    *psize = 0;
    if (mtd)
    {
        *psize = (mtd->size/1024);
        put_mtd_device(mtd, mtd_fd, -1);
    }
#endif

    return(0);
}

/* mdev can take upto 2 seconds to create dev_nodes */
#define DEV_NODE_CHK_TIMEOUT_USEC   3000000       
#define DEV_NODE_CHK_INTRVL_USEC     100000       
int nandUbiVolDevNodeExists( char * dev_path, int exists)
{
    int dev_node_wait_cnt=DEV_NODE_CHK_TIMEOUT_USEC;
    int wait=0;
    struct stat st;

    /* If we have just created a ubi volume mdev might take upto 2 seconds
     * to create the associated dev_nodes, so we wait and check every .5s
     */
    while( dev_node_wait_cnt > 0 )
    {
        usleep(DEV_NODE_CHK_INTRVL_USEC);

        if( exists ) 
        {
            /* Check if node exists - If node doesnt exist we sleep and check again */
            if (access( dev_path, F_OK ) ||  stat( dev_path, &st)) 
                wait=1;
        } 
        else
        {
            /* Check if node doesnt exist - If node exists we sleep and check again */
            if (!access( dev_path, F_OK ) || !stat( dev_path, &st)) 
                wait=1;
        }
        
        if( wait )
        {
            dev_node_wait_cnt -= DEV_NODE_CHK_INTRVL_USEC;
            wait=0;
        }
        else
            break;
    }

    if(dev_node_wait_cnt <= 0)
    {
        printf("%s: %s_wait_usec_left:%d\n", __FUNCTION__, (exists?"exist":"notexist"),
                                 dev_node_wait_cnt);
    }

    return (dev_node_wait_cnt>0);
}

int getNandMetadata( char * data, int size , int mdata_idx)
{
    char mdata_fname[128];
    FILE * mdata_fh;
    int num_bytes = 0;
    sprintf(mdata_fname, "/dev/ubi0_%d", mdata_idx);
    mdata_fh = fopen(mdata_fname, "r");
    
    if( mdata_fh )
    {
        num_bytes = fread(data, 1, size, mdata_fh);

        if( ferror(mdata_fh) || (num_bytes <= 0))
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

int setNandMetadata( char * data, int size, int mdata_idx )
{
    FILE * mdata_fh = fopen("/var/mdata.tmp", "w");
    char cmd[128];
    int rc = 0;
    if( mdata_fh ) 
    {
       fwrite(data, 1, size, mdata_fh);
       fclose(mdata_fh);
    }
    else
    {
        printf("%s: Error writing metadata!\n", __FUNCTION__);
        return -1;
    }

    /* Create Metadata device if it doesnt exist */
    snprintf(cmd, sizeof(cmd), "/dev/ubi0_%d", mdata_idx) ;
    if( access( cmd, F_OK ) ) 
    {
        snprintf(cmd, sizeof(cmd), "ubimkvol /dev/ubi0 -s %d -n %d -N metadata%d --type=static", size, mdata_idx, mdata_idx);
        flashutil_debug("=> %s\n", cmd);
        rc = system(cmd);
        if( rc )
            goto err_exit;

        /* Wait for mdev to catchup */
        snprintf(cmd, sizeof(cmd), "/dev/ubi0_%d", mdata_idx);
        if( !nandUbiVolDevNodeExists(cmd, 1) )
        {
            fprintf(stderr, "ERROR!!! %s doesnt exist!\n", cmd);
            rc = -1;
            goto err_exit;
        }
    }

    /* Update volume with metadata */
    snprintf(cmd, sizeof(cmd), "ubiupdatevol /dev/ubi0_%d /var/mdata.tmp", mdata_idx);
    flashutil_debug("=> %s\n", cmd);
    rc = system(cmd);
    rc |= system("rm -rf /var/mdata.tmp");
    if (rc < 0)
    {
       fprintf(stderr, "ERROR!!! ubi command failed!\n");
    }

err_exit:
    return rc;
}


/***********************************************************************
 * Function Name: nandCommit
 * Description  : Gets/sets the commit flag of a pureUBI image.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int nandCommit( int partition, char *commit_flag )
{
    int ret = -1;
    int mtd_fd, mtdblock_fd;
    mtd_info_t * mtd;
    int i;
    unsigned char * buf = NULL;
    unsigned int len;
    int write = (*commit_flag != 0);

    if( nandIsLegacyFlashLayout())
    {
        if (partition == nandGetBootPartition())
        { // use UBI interface
            if (!exist_mtd_dev_nm("METADATA") && !exist_mtd_dev_nm("METADATACOPY"))
                return(ret);

            for (i = 0; i < 2; i++)
            {
                mtd = get_mtd_device_handle(i ? "METADATACOPY" : "METADATA", &mtd_fd, &mtdblock_fd);

                if ( IS_ERR_OR_NULL(mtd) )
                {
                    fprintf(stderr, "ERROR!!! unable to open mtd partition %s\n", i ? "METADATACOPY" : "METADATA");
                    continue;
                }

                len = mtd->erasesize;

                if ( !buf && ((buf = malloc(len)) == NULL) )
                {
                    fprintf(stderr, "ERROR!!! cannot allocate block memory for buffer\n");
                    put_mtd_device(mtd, mtd_fd, mtdblock_fd);
                    continue;
                }

                if (parse_ubi(0, buf, 0, mtd->size / len, len, -1, "committed", commit_flag, 0, 0, read_block, write ? write_block : 0, /* write ? erase_block :*/ 0, mtd, mtdblock_fd) == 1)
                {
                    ret = 0;  // if either copy of metadata is committed then we are committed
                    if (!write)
                    {
                        put_mtd_device(mtd, mtd_fd, mtdblock_fd);
                        break;
                    }
                }
                else
                    printf("ERROR!!! Could not find %s partition image metadata volume %d\n", (partition == 1) ? "first" : "second", i);

                put_mtd_device(mtd, mtd_fd, mtdblock_fd);
            }

            if (buf)
                free(buf);
        }
        else
        { // use mtd interface
            if (scan_partition("image_update") != UBI_IMAGE)
                return(ret);

            mtd = get_mtd_device_handle("image_update", &mtd_fd, 0);

            if ( IS_ERR_OR_NULL(mtd) )
            {
                fprintf(stderr, "ERROR!!! unable to open mtd partition image_update\n");
                return(ret);
            }

            len = mtd->erasesize;

            if ((buf = malloc(len)) == 0)
            {
                fprintf(stderr, "ERROR!!! cannot allocate block memory for buffer\n");
                put_mtd_device(mtd, mtd_fd, -1);
                return(ret);
            }

            for (i = 0; i < 2; i++)
            {
                if (parse_ubi(0, buf, 0, mtd->size / len, len, i ? VOLID_METADATA_COPY : VOLID_METADATA, "committed", commit_flag, 0, 0, read_blk, write ? write_blk : 0, write ? erase_blk : 0, mtd, mtd_fd) == 1)
                {
                    ret = 0;  // if either copy of metadata is committed then we are committed
                    if (!write)
                        break;
                }
                else
                    printf("ERROR!!! Could not find image_update metadata volume %d\n", i);
            }

            put_mtd_device(mtd, mtd_fd, -1);
            free(buf);
        }
    }
    else
    {
	    printf("ERROR!!! This function should not be called for NEW flash layout\n");
    }

    return( ret );
}


/***********************************************************************
 * Function Name: nandIsBootDevice
 * Description  : Determines whether boot device is nand
 * Returns      : 0 - bootdevice is not nand, 1 - bootdevice IS nand 
 ***********************************************************************/
int nandIsBootDevice(void)
{
    int found = 0;
#ifndef DESKTOP_LINUX
    FILE *fp;
    char line[COMMAND_LINE_SIZE]={0};
    fp = popen("/rom/etc/get_rootfs_dev.sh", "r");
    if (fp == NULL) {
    	printf("Failed to run /rom/etc/get_rootfs_dev.sh command\n" );
    	return 0;
    }

    if(fgets(line, sizeof(line), fp))
    {
        if ( strstr(line, "ubiblock") || strstr(line, "ubi:rootfs") )
            found = 1;
    }
    pclose(fp);
#endif
    return found;
}


/***********************************************************************
 * Function Name: nandIsLegacyFlashLayout
 * Description  : Determines whether boot device is using legacy flash layout
 * Returns      : 1 - flashlayout is legacy 
 ***********************************************************************/
int nandIsLegacyFlashLayout(void)
{
    FILE *fp;
    char line[256]={0};
    char *mtd_name = "\"loader\"";
    char name[MAX_MTD_NAME_SIZE]={0};
    int legacy_layout = 1;

    fp = fopen("/proc/mtd","r");
    if (fp == NULL)
    {
       fprintf(stderr, "Could not open /proc/mtd");
       return ERROR;
    }

    /* Search for 'loader' partition from new flash layout */
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%*s %*s %*s %s", name);

        if(strcmp(mtd_name, name) == 0)
        {
            legacy_layout = 0;
            break;
        }
    }
    fclose(fp);
    return legacy_layout;
}

/***********************************************************************
 * Function Name: getBootedValue (devCtl_getBootedImagePartition)
 * Description  : Gets the state of an image update from flash.
 * Returns      : state constant or -1 for failure
 ***********************************************************************/
extern int devCtl_getBootedImageId(void);

int nandGetBootedValue(void)
{
    if (nandGetBootPartition() == 1)
        return(BOOTED_PART1_IMAGE);
    else
        return(BOOTED_PART2_IMAGE);
}

/*
	calculates and compare checksum for CFEROM binary
*/
static int chksum_sbi(unsigned char *pSbi, int *cferom_crc)
{
    // sbiLen is the length of the entire UBI image including the 4 byte CRC
    SbiUnauthHdrBeginning *pUHdr = (SbiUnauthHdrBeginning *)pSbi;
    uint32_t              sbiLen = pUHdr->sbiSize;
    uint32_t              unauthHdrSize = pUHdr->hdrLen;
    unsigned char         *pCrcStart = pSbi + unauthHdrSize;
    uint32_t              crcLen = sbiLen - (RSA_S_MODULUS8 * 2) - unauthHdrSize - CRC_LEN;
    uint32_t              crc = CRC_INITIAL_VALUE;


    // We are about to authenticate the UBI itself, print out "UBI?"
    // Perform the CRC calc
    // crc is only across the same data that the signature is across
    // therefore exclude the unauthenticated header, and the trailer
    crc = genUtl_getCrc32(pCrcStart, crcLen, crc);

    if (memcmp(&crc, pSbi + sbiLen - CRC_LEN, CRC_LEN) != 0)
    {
        // we have failed the UBI image crc, print out "UBIF"
        return -1;
    }

    if(cferom_crc != NULL)
        *cferom_crc=crc;

    // we have passed the UBI image crc, print out "UBIP"
    return 0;
}

/*
	This function will check if thre is a valid CFEROM binary at the start of the buffer
	if save_cferom is not NULL, it will allocate required amount of memory and copy the cferom
	into that buffer
*/
int is_cferom_offset(unsigned char *buffer, int offset, int *rom_size, int *cferom_crc, unsigned char **save_cferom)
{
    struct hdr_chksum {
        SbiUnauthHdrBeginning  unauthHdr;
        uint32_t chksum;
    }__attribute__((__packed__)) *hdchk;
    SbiUnauthHdrBeginning *pHdr;
    SbiAuthHdrBeginning   *authHdrBgn;
    int auth_hdr_offset=0;
    uint32_t crc = CRC32_INIT_VALUE;
    int rc=0;

    hdchk=(    struct hdr_chksum  *)(buffer+offset);
    if (hdchk->unauthHdr.magic_1 == BTRM_SBI_UNAUTH_MGC_NUM_1)
    {
        if (hdchk->unauthHdr.magic_2 == BTRM_SBI_UNAUTH_MGC_NUM_2)
        {
            //verify image
            // Do a preliminary upper / lower boundary check of the size of the unauthenticated header
            if ((hdchk->unauthHdr.hdrLen >= sizeof(SbiUnauthHdrBeginning)) && (hdchk->unauthHdr.hdrLen < BTRM_SBI_UNAUTH_HDR_MAX_SIZE))
            {
                // Perform a CRC calc on the header
                pHdr=&hdchk->unauthHdr;
                crc = CRC32_INIT_VALUE;
                crc = genUtl_getCrc32((uint8_t*)pHdr, hdchk->unauthHdr.hdrLen-sizeof(uint32_t), crc);
                /* coverity[illegal_address] */
                if (memcmp(&crc, ((uint8_t*)pHdr) + (hdchk->unauthHdr.hdrLen-sizeof(uint32_t)), CRC_LEN) == 0)
                {
                    //
                    if (hdchk->unauthHdr.sbiSize < BTRM_SBI_IMAGE_MAX_ALLOWED_SIZE)
                    {
                        // Check that the sbi size value is at least big enough integer to cover the
                        // unauthenticated header and trailer.
                        if (hdchk->unauthHdr.sbiSize <= ((RSA_S_MODULUS8 * 2) + hdchk->unauthHdr.hdrLen + CRC_LEN))
                        {
                            // Print out "UHDF" which stands for "Unath HeaDer Failed"
                        }
                        else
                        {
                            //get the image
                            auth_hdr_offset=offset+hdchk->unauthHdr.hdrLen;
                            authHdrBgn = (SbiAuthHdrBeginning   *)(buffer+auth_hdr_offset);
                            //ppCopyFunc(auth_hdr_offset, (unsigned char *)&authHdrBgn,  sizeof(SbiAuthHdrBeginning));
                            // Retrieve the (untrusted) authHdrSize from the header itself
                            // Do a preliminary upper / lower boundary check of the authenticated header size
                            if ((authHdrBgn->hdrLen < sizeof(SbiAuthHdrBeginning)) || (authHdrBgn->hdrLen > BTRM_SBI_AUTH_HDR_MAX_SIZE))
                            {
                            }
                            else
                            {
                                // Make sure that the COTs within the authenticated header are such that everything in
                                // this header is word aligned. For example, mfg ROE COT version 2 should have 2 bytes of padding
                                if (authHdrBgn->hdrLen % sizeof(uint32_t) != 0)
                                {
                                }
                                else
                                {
                                    //checksum sbi
                                    rc=chksum_sbi((unsigned char*)(buffer+offset), cferom_crc);
                                    if(rc == 0)
                                    {
                                        if(save_cferom != NULL )
                                        {
                                            if(save_cferom[0] == NULL)
                                                save_cferom[0]=malloc(hdchk->unauthHdr.sbiSize);
                                            if(save_cferom[0] != NULL)
                                            {
                                                memcpy(save_cferom[0], buffer+offset, hdchk->unauthHdr.sbiSize);
                                                *rom_size=hdchk->unauthHdr.sbiSize;
                                            }
                                        }

                                        rc=IMG_CFEROM;
                                    }
                                    //prev_offset=offset;
                                    //break;
                                }
                            }
                        }

                    }
                }

            }
        }
    }
    return rc;
}
/*
	function checks for the NVRAM_DATA_SIGN at the beginning of the buffer
	if found it will perform the checksum check on the data that follows the signature
	if check_nvram_data_sign is 0,  the NVRAM_DATA_SIGN will not be checked
*/
int is_nvram_offset(unsigned char *buffer, int offset, int check_nvram_data_sign, int *image_size, PNVRAM_DATA current_nvram)
{
    char *sign=NVRAM_DATA_SIGN;
    unsigned int crc, temp_crc;
    int rc=IMG_MISSING;

    if(!check_nvram_data_sign)
    {
        sign="\0";
    }
    if(strncmp((char*)buffer+offset, sign, strlen(sign)) == 0)
    {
        crc=((PNVRAM_DATA)(buffer+offset+strlen(sign)))->ulCheckSum;
        ((PNVRAM_DATA)(buffer+offset+strlen(sign)))->ulCheckSum=0;
        if(crc == (temp_crc=genUtl_getCrc32((unsigned char *) (buffer+offset+strlen(sign)), sizeof(NVRAM_DATA), CRC_INITIAL_VALUE)))
        {
            rc=IMG_NVRAM;
            if(current_nvram != NULL)
                if(crc != current_nvram->ulCheckSum)
                    rc=CRC_MISMATCH;
            if(image_size != NULL)
                *image_size=sizeof(NVRAM_DATA)+strlen(sign);
        }
        else
        {
            rc=CRC_MISMATCH;
        }
        //restore the crc
        ((PNVRAM_DATA)(buffer+offset+strlen(sign)))->ulCheckSum=crc;
    }
    return rc;
}
