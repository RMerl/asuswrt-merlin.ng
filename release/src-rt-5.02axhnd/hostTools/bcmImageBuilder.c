/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

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
 *
 ************************************************************************/
 
//**************************************************************************************
// File Name  : bcmImageBuilder.c
//
// Description: Build a image with a header (tag), a compressed kernel and
//              a compress JFFS2 root file system
//
// Created    : 04/23/2002  
//
// Modified   : 05/18/2002  
//              Redefined the tag info.  Add imageType and getting all the flash address
//              from "board.h" which is shared by CFE and web uploading code.
//
//            : 04/18/2003 seanl
//              add the support for cramfs and new flash image layout:
//              1). minicfe : 64k
//              2). TAG:      256b
//              3). cramfs:   
//              4). kernel:
//              5). psi:      16k (default)
//              
//
//**************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include "include/bcmTargetEndian.h"

#define  BCMTAG_EXE_USE
#include "bcmTag.h"

/* The BCM963xx flash memory map contains five sections: boot loader,
 * file system, kernel, nvram storage, persistent storage.  This program
 * defines the starting address for the boot loader, file system and kernel.
 * The run-time flash driver determines the starting address of the nvram
 * and persistent storage sections based on the flash part size.
 */
#include "board.h"

/* Typedefs.h */
typedef unsigned char byte;
typedef unsigned int UINT32;

#define BUF_SIZE        100 * 1024
static byte buffer[BUF_SIZE];

/*
     struct option {
             char *name;
             int has_arg;
             int *flag;
             int val;
     };

     The name field should contain the option name without the leading double
     dash.
     The has_arg field should be one of:
     no_argument        no argument to the option is expect.
     required_argument  an argument to the option is required.
     optional_argument  an argument to the option may be presented.
     If flag is non-NULL, then the integer pointed to by it will be set to the
     value in the val field. If the flag field is NULL, then the val field
     will be returned. Setting flag to NULL and setting val to the correspond-
     ing short option will make this function act just like getopt(3).
*/
static struct option longopts[] = {
    { "help", 0, 0, 'h' },
    { "version", 0, 0, 'v' },
    { "littleendian", 0, 0, 'l'},
    { "bigendian", 0, 0, 'b'},
    { "chip", 1, 0, 0},
    { "board", 1, 0, 0},
    { "blocksize", 1, 0, 0},
    { "image-version", 1, 0, 0},
    { "output", 1, 0, 0},
    { "cfefile", 1, 0, 0},
    { "rootfsfile", 1, 0, 0},
    { "kernelfile", 1, 0, 0},
    { "bootfsfile", 1, 0, 0},
    { "dtbfile", 1, 0, 0},
    { "mdatafile", 1, 0, 0},
    { "include-cfe", 0, 0, 'i'},
    { 0, 0, 0, 0 }
};

static char version[] = "Broadcom Image Builder version 0.3";

static char usage[] = "Usage:\t%s {-h|--help|-v|--version}\n"
"\t[--littleendian]\n"
"\t--chip <chipid> -- chip id {63268|6328|6362|6368|6816|6818|6828}\n"
"\t--board <boardid> --blocksize {64|128}\n"
"\t--output <filename> --cfefile <filename>\n"
"\tNOR Image:\n"
"\t[--rootfsfile <filename> --kernelfile <filename> --dtbfile <filename> [-i|--include-cfe]]\n"
"\teMMC Image:\n"
"\t[--rootfsfile <filename> --bootfsfile <filename> --mdatafile <filename> [-i|--include-cfe]]\n";

static char *progname;

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

/***************************************************************************
// Function Name: build image
// Description  : builds a complete norflash or emmc image
// Parameters   : little endian - endianess.
//                chipid        - chipid
//                board         - boardid
//                blksize       - flash block size for combined cfe(NOR) or cferom(EMMC)
//                imageversiona - image version
//                outputfile    - final output image name
//                cfefile       - Name of cfe(NOR) or cferom(EMMC) binary
//                rootfsfile    - root fs image name
//                kernelfile    - compressed kernel(NOR) or bootfs image(EMMC) name
//                dtb_mdata_file- dtb(NOR) or metadata blob(EMMC) name
//                includecfe    - flag to to include cfe(NOR) or cferom(EMMC)
//                emmcimage     - flag to indicate an emmcimage build
// Returns      : none.
****************************************************************************/
static void build_image(int littleendian, char *chipid, char *board, unsigned int blksize, 
    char *imageversion, char *outputfile, char *cfefile, char *rootfsfile, char *kernelfile, char *dtb_mdata_file, int includecfe, int emmcimage)
{
    FILE *stream_cfe = NULL;
    FILE *stream_rootfs = NULL;
    FILE *stream_kernel = NULL;
    FILE *stream_dtb_mdata = NULL;
    FILE *stream_output = NULL;
    unsigned int total_size, cfe_size, rootfs_size, kernel_size, dtb_mdata_size, count, numWritten;
    unsigned int cfeaddr = 0;
    unsigned int fskerneladdr;
    FILE_TAG tag;
    UINT32 imageCrc, tagCrc, fsCrc, kernelCrc, dtb_mdata_crc;

    /* emmc image starts right after the TAG */
    if( emmcimage )
        cfeaddr = 0;
    else
        cfeaddr = IMAGE_BASE;

    if ((stream_cfe = fopen(cfefile, "rb")) == NULL) {
        printf("bcmImageBuilder: Failed to open cfe file: %s\n", cfefile);
        fclose(stream_cfe);
        exit(1);
    }

    if (strlen(rootfsfile) != 0 && strlen(kernelfile) != 0 && strlen(dtb_mdata_file) != 0) {
        if ((stream_rootfs = fopen(rootfsfile, "rb")) == NULL) {
            printf("bcmImageBuilder: Failed to open root file system file: %s\n", rootfsfile);
            fclose(stream_cfe);
            exit(1);
        }
        if ((stream_kernel = fopen(kernelfile, "rb")) == NULL) {
            printf("bcmImageBuilder: Filed to open kernel file: %s\n", kernelfile);
            fclose(stream_cfe);
            fclose(stream_rootfs);
            exit(1);
        }
        if ((stream_dtb_mdata = fopen(dtb_mdata_file, "rb")) == NULL) {
            printf("bcmImageBuilder: Filed to open dtb file: %s\n", dtb_mdata_file);
            fclose(stream_cfe);
            fclose(stream_rootfs);
            fclose(stream_kernel);
            exit(1);
        }
    }

    if((stream_output = fopen(outputfile, "wb")) == NULL) {
        printf("bcmImageBuilder: Failed to create output file: %s\n", outputfile);
        fclose(stream_cfe);
        fclose(stream_kernel);
        fclose(stream_rootfs);
        fclose(stream_dtb_mdata);
        exit(1);
    }


    total_size = cfe_size  = rootfs_size = kernel_size = dtb_mdata_size = count = 0;
    imageCrc = CRC32_INIT_VALUE;
    fsCrc = CRC32_INIT_VALUE;
    kernelCrc = CRC32_INIT_VALUE;

    while (!feof( stream_cfe )) {
        count = fread( buffer, sizeof(byte), BUF_SIZE, stream_cfe);
        if (ferror( stream_cfe)) {
            perror( "bcmImageBuilder: Read error" );
            exit(1);
        }
        imageCrc = getCrc32(buffer, count, imageCrc);
        /* Total up actual bytes read */
        total_size += count;
        cfe_size += count;
    }

    if( cfe_size > blksize ){
        printf("bcmImageBuilder: cfe size too large, maximum size %d\n", blksize);
        fclose(stream_cfe);
        fclose(stream_kernel);
        fclose(stream_rootfs);
        fclose(stream_dtb_mdata);
        exit(1);
    }

    /* For eMMC: image layout doesnt correspond to flash layout, so start 
     * bootfs right after.
     * For NOR:  Calculate address after cfe in flash;  Tag is before fs now */
    if( emmcimage )
        /* If cferom is included, fskernel starts after cfe
         * if not, then fskernel starts right after tag */
        if( includecfe )
            fskerneladdr = cfeaddr + cfe_size;
        else
            fskerneladdr = 0;
    else        
        fskerneladdr = cfeaddr + blksize;

    if (stream_rootfs && stream_kernel && stream_dtb_mdata) {
        if (!includecfe) {
            /* CFE is not included in this image */
            total_size = 0;
            imageCrc = CRC32_INIT_VALUE;
            fclose(stream_cfe);
            stream_cfe = NULL;
        }

        while (!feof( stream_rootfs )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_rootfs);
            if (ferror( stream_rootfs)) {
                perror( "bcmImageBuilder: Read error" );
                exit(1);
            }
            imageCrc = getCrc32(buffer, count, imageCrc);
            fsCrc = getCrc32(buffer, count, fsCrc);
            /* Total up actual bytes read */
            total_size += count;
            rootfs_size += count;
        }

        while (!feof( stream_kernel )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_kernel );
            if (ferror( stream_kernel)) {
                perror( "bcmImageBuilder: Read error" );
                exit(1);
            }
            imageCrc = getCrc32(buffer, count, imageCrc);
            kernelCrc = getCrc32(buffer, count, kernelCrc);
            /* Total up actual bytes read */
            total_size += count;
            kernel_size += count;
        }

        while (!feof( stream_dtb_mdata )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_dtb_mdata );
            if (ferror( stream_dtb_mdata)) {
                perror( "bcmImageBuilder: Read error" );
                exit(1);
            }
            imageCrc = getCrc32(buffer, count, imageCrc);
            dtb_mdata_crc = getCrc32(buffer, count, dtb_mdata_crc);
            /* Total up actual bytes read */
            total_size += count;
            dtb_mdata_size += count;
        }
    }

    if (!stream_cfe) {
        cfeaddr = 0;
        cfe_size = 0;
    }

    if (!stream_rootfs || !stream_kernel || !stream_dtb_mdata) {
        fskerneladdr = 0;
        rootfs_size = 0;
        kernel_size = 0;
        dtb_mdata_size = 0;
    }

    // fill the tag structure
    memset(&tag, 0, sizeof(FILE_TAG));

    strcpy(tag.tagVersion, BCM_TAG_VER);
    strncpy(tag.signiture_1, BCM_SIG_1, SIG_LEN -1);
    strncpy(tag.signiture_2, BCM_SIG_2, SIG_LEN_2 -1);
    strncpy(tag.chipId, chipid, CHIP_ID_LEN - 1);
    strncpy(tag.boardId, board, BOARD_ID_LEN -1);
    strncpy(tag.imageVersion, imageversion, IMAGE_VER_LEN -1);
    if (littleendian)
        strcpy(tag.bigEndian, "0");
    else
        strcpy(tag.bigEndian, "1");

    sprintf(tag.totalImageLen,"%d",total_size);

    if( emmcimage )
        sprintf(tag.cfeAddress, "%u", cfeaddr + TAG_LEN);
    else        
        sprintf(tag.cfeAddress, "%u", cfeaddr);

    sprintf(tag.cfeLen,"%d",cfe_size);

    if (!stream_rootfs || !stream_kernel || !stream_dtb_mdata)
    {
        sprintf(tag.rootfsAddress,"%u",(uint32_t ) 0);
        sprintf(tag.rootfsLen,"%d",rootfs_size);
        sprintf(tag.kernelAddress,"%u", 0);
        sprintf(tag.kernelLen,"%d",kernel_size);
        if( emmcimage ) {
            sprintf(tag.mdataAddress,"%u", 0);    
            sprintf(tag.mdataLen,"%d",dtb_mdata_size);
        } else {
            sprintf(tag.dtbAddress,"%u", 0);    
            sprintf(tag.dtbLen,"%d",dtb_mdata_size);
        }
    }
    else
    {
        sprintf(tag.rootfsAddress,"%u",(uint32_t ) (fskerneladdr + TAG_LEN));
        sprintf(tag.rootfsLen,"%d",rootfs_size);
        sprintf(tag.kernelAddress,"%u", fskerneladdr + rootfs_size + TAG_LEN);
        sprintf(tag.kernelLen,"%d",kernel_size);
        if( emmcimage ) {
            sprintf(tag.mdataAddress,"%u", fskerneladdr + rootfs_size + kernel_size + TAG_LEN);
            sprintf(tag.mdataLen,"%d",dtb_mdata_size);
        } else {
            sprintf(tag.dtbAddress,"%u", fskerneladdr + rootfs_size + kernel_size + TAG_LEN);
            sprintf(tag.dtbLen,"%d",dtb_mdata_size);
        }
    }

    imageCrc =  BCM_HOST_TO_TARGET32(imageCrc);
    fsCrc = BCM_HOST_TO_TARGET32(fsCrc);
    kernelCrc = BCM_HOST_TO_TARGET32(kernelCrc);
    dtb_mdata_crc = BCM_HOST_TO_TARGET32(dtb_mdata_crc);
    memcpy(tag.imageValidationToken, (byte*)&imageCrc, CRC_LEN);
    memcpy(tag.imageValidationToken + CRC_LEN, (byte*)&fsCrc, CRC_LEN);
    memcpy(tag.imageValidationToken + (CRC_LEN * 2), (byte*)&kernelCrc, CRC_LEN);
    memcpy(tag.imageValidationToken + (CRC_LEN * 4), (byte*)&dtb_mdata_crc, CRC_LEN);

    // get tag crc
    tagCrc = CRC32_INIT_VALUE;
    tagCrc = getCrc32((byte*)&tag, TAG_LEN-TOKEN_LEN, tagCrc);
    tagCrc = BCM_HOST_TO_TARGET32(tagCrc);
    memcpy(tag.tagValidationToken, (byte*)&tagCrc, CRC_LEN);

#ifdef DEBUG
    printf ("tagVersion %s\n", tag.tagVersion);
    printf ("signiture_1 %s\n", tag.signiture_1);
    printf ("signiture_2 %s\n", tag.signiture_2);
    printf ("chipId %s\n", tag.chipId);
    printf ("boardId %s\n", tag.boardId);
    printf ("bigEndian %s\n", tag.bigEndian);
    printf ("totalImageLen %s\n", tag.totalImageLen);
    printf ("cfeAddress %s, 0x%08X\n", tag.cfeAddress, (unsigned int)cfeaddr);
    printf ("cfeLen %s\n", tag.cfeLen);
    printf ("rootfsAddress %s, 0x%08X\n", tag.rootfsAddress, (unsigned int) fskerneladdr);
    printf ("rootfsLen %s\n", tag.rootfsLen);
    printf ("kernelAddress %s, 0x%08X\n", tag.kernelAddress, (unsigned int) (fskerneladdr + rootfs_size));
    printf ("kernelLen %s\n", tag.kernelLen);
    if( emmcimage ) {
        printf ("mdataAddress %s, 0x%08X\n", tag.dtbAddress, (unsigned int) (fskerneladdr + rootfs_size + kernel_size));
        printf ("mdataLen %s\n", tag.dtbLen);
    } else {
        printf ("dtbAddress %s, 0x%08X\n", tag.dtbAddress, (unsigned int) (fskerneladdr + rootfs_size + kernel_size));
        printf ("dtbLen %s\n", tag.dtbLen);
    }

#endif

    //----------------Start to write image file---------------------------
    //Write tag first
    numWritten = fwrite((byte*)&tag, sizeof(byte), TAG_LEN, stream_output);
    if (numWritten != TAG_LEN) {
        printf("bcmImageBuilder: Failed to write tag: byte written: %d byte\n", numWritten);
        exit(1);
    }

    //Write the cfe if exist
    if (stream_cfe) {
        rewind(stream_cfe);
        while (!feof( stream_cfe )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_cfe );
            if (ferror(stream_cfe)) {
                perror("bcmImageBuilder: Read cfe file error");
                exit(1);
            }
            numWritten = fwrite(buffer, sizeof(byte), count, stream_output);
            if (ferror(stream_output)) {
                perror("bcmImageBuilder: Write image file error");
                exit(1);
            }
        }
        fclose(stream_cfe);
    }

    //Write the rootfs
    if (stream_rootfs) {
        rewind(stream_rootfs);
        while (!feof( stream_rootfs )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_rootfs );
            if (ferror(stream_rootfs)) {
                perror("bcmImageBuilder: Read root file system file error");
                exit(1);
            }
            numWritten = fwrite(buffer, sizeof(byte), count, stream_output);
            if (ferror(stream_output)) {
                perror("bcmImageBuilder: Write image file error");
                exit(1);
            }
        }
        fclose(stream_rootfs);
    }

    //Write the kernel
    if (stream_kernel) {
        rewind(stream_kernel);
        while (!feof( stream_kernel )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_kernel );
            if (ferror(stream_kernel)) {
                perror("bcmImageBuilder: Read kerenl file error");
                exit(1);
            }
            numWritten = fwrite(buffer, sizeof(byte), count, stream_output);
            if (ferror(stream_output)) {
                perror("bcmImageBuilder: Write image file error");
                exit(1);
            }
        }
        fclose(stream_kernel);
    }

    //Write the dtb
    if (stream_dtb_mdata) {
        rewind(stream_dtb_mdata);
        while (!feof( stream_dtb_mdata )) {
            count = fread( buffer, sizeof(byte), BUF_SIZE, stream_dtb_mdata );
            if (ferror(stream_dtb_mdata)) {
                perror("bcmImageBuilder: Read dtb file error");
                exit(1);
            }
            numWritten = fwrite(buffer, sizeof(byte), count, stream_output);
            if (ferror(stream_output)) {
                perror("bcmImageBuilder: Write image file error");
                exit(1);
            }
        }
        fclose(stream_dtb_mdata);
    }


    fclose(stream_output);

    printf("bcmImageBuilder\n");

    printf("\tTarget endianess              : ");
    if (littleendian == 1)
        printf("little\n");
    else
        printf("big\n");

    if (stream_cfe)
        printf("\tCFE image size                : %u\n", cfe_size);

    printf("\tFile tag size                 : %d\n", TAG_LEN);

    if (stream_rootfs)
        printf("\tRoot filesystem image size    : %u\n", rootfs_size);
    if (stream_kernel)
        if( emmcimage ) 
            printf("\tBootfs image size             : %u\n", kernel_size);
        else
            printf("\tKernel image size             : %u\n", kernel_size);
    if (stream_dtb_mdata) {
        if( emmcimage ) 
            printf("\tMetadata image size                : %u\n", dtb_mdata_size);
        else
            printf("\tDtb image size                : %u\n", dtb_mdata_size);

    }

    printf("\tCombined image file size      : %u\n\n", total_size+TAG_LEN);
}


int main (int argc, char **argv)
{
    int optc;
    int h = 0, v = 0, littleendian = 0, includecfe = 0, lose = 0, emmcimage = 0;
    int option_index = 0;
    char cfefile[256], rootfsfile[256], kernelfile[256], dtb_mdata_file[256], outputfile[256];
    char boardid[BOARD_ID_LEN];
    char chipid[CHIP_ID_LEN];
    char imageversion[IMAGE_VER_LEN*2];
    unsigned int blksize = 0;

    progname = argv[0];
    chipid[0] = boardid[0] = cfefile[0] = kernelfile[0] = rootfsfile[0] = dtb_mdata_file[0] = outputfile[0] = '\0';

    while ((optc = getopt_long (argc, argv, "hvbli", longopts, &option_index)) != EOF) {
        switch (optc){
        case 0:
#ifdef DEBUG
            printf ("option %s", longopts[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
#endif
            if (strcmp(longopts[option_index].name, "cfefile") == 0) 
                strcpy(cfefile, optarg);
            else if (strcmp(longopts[option_index].name, "rootfsfile") == 0)
                strcpy(rootfsfile, optarg);
            else if (strcmp(longopts[option_index].name, "kernelfile") == 0) {
                emmcimage = 0;
                strcpy(kernelfile, optarg);
            }
            else if (strcmp(longopts[option_index].name, "dtbfile") == 0) {
                emmcimage = 0;
                strcpy(dtb_mdata_file, optarg);
            }
            else if (strcmp(longopts[option_index].name, "bootfsfile") == 0) {
                emmcimage = 1;
                strcpy(kernelfile, optarg);
            }
            else if (strcmp(longopts[option_index].name, "mdatafile") == 0) {
                emmcimage = 1;
                strcpy(dtb_mdata_file, optarg);
            }
            else if (strcmp(longopts[option_index].name, "output") == 0)
                strcpy(outputfile, optarg);
            else if (strcmp(longopts[option_index].name, "chip") == 0)
                strcpy(chipid, optarg);
            else if (strcmp(longopts[option_index].name, "board") == 0)
                strcpy(boardid, optarg);
            else if (strcmp(longopts[option_index].name, "blocksize") == 0)
                blksize = atoi(optarg) * 1024;
            else if (strcmp(longopts[option_index].name, "image-version") == 0)
                strcpy(imageversion, optarg);
            break;
        case 'v':
            v = 1;
            break;
        case 'h':
            h = 1;
            break;
        case 'b':
            littleendian = 0;
            break;
        case 'l':
            littleendian = 1;
            BCM_SET_TARGET_ENDIANESS(BCM_TARGET_LITTLE_ENDIAN);
            break;
        case 'i':
            includecfe = 1;
            break;
        default:
            lose = 1;
            break;
        }
    }

    if (lose || optind < argc || argc < 2)
    {
        /* Print error message and exit.  */
        fprintf (stderr, usage, progname);
        exit (1);
    }

    if (v) {
        /* Print version number.  */
        fprintf (stderr, "%s\n", version);
        if (! h)
            exit (0);
    }

    if (h) {
        /* Print help info and exit.  */
        fprintf (stderr, "%s\n", version);
        fprintf (stderr, usage, progname);
        fputs ("\n", stderr);
        fputs ("  -h, --help                  Print a summary of the options\n", stderr);
        fputs ("  -v, --version               Print the version number\n", stderr);
        fputs ("  -l, --littleendian          Build little endian image\n", stderr);
        fputs ("  -b, --bigendian             Build big endian image\n", stderr);
        fputs ("  --chip  <chip id>           Chip id\n", stderr);
        fputs ("  --board  <board name>       Board name\n", stderr);
        fputs ("  --blocksize  <block size>   Flash memory block size in KBytes\n", stderr);
        fputs ("  --output <filename>         Output image file name\n", stderr);
        fputs ("  --cfefile    <filename>     CFE imgage name\n", stderr);
        fputs ("  --kernelfile <filename>     Kernel image name\n", stderr);
        fputs ("  --bootfsfile <filename>     eMMC bootfs image name\n", stderr);
        fputs ("  --dtbfile <filename>        Device tree blob file name\n", stderr);
        fputs ("  --rootfsfile <filename>     Root file system image name\n", stderr);
        fputs ("  --mdatafile <filename>      eMMC  metadata file name\n", stderr);
        fputs ("  -i, --include-cfe           Add CFE to kernel and rootfs image\n", stderr);
        fputs ("  -e, --emmc-image            Generate an eMMC compatible image\n", stderr);
        exit (0);
    }


    if (strlen(chipid) == 0 || strlen(boardid) == 0 || strlen(outputfile) == 0
        || strlen(cfefile) == 0)
    {
        fprintf (stderr, "Required input parameters are missing\n\n");
        fprintf (stderr, usage, progname);
        exit (1);
    }
    
    if(!emmcimage)
    {
        if (blksize % 64*1024) 
        {
            fprintf (stderr, "invalid block size %d, must be the multiple of 64KB\n\n", blksize);
            fprintf (stderr, usage, progname);
            exit (1);
        }
    }

    if ((strlen(rootfsfile) == 0 && strlen(kernelfile) != 0) ||
        (strlen(rootfsfile) != 0 && strlen(kernelfile) == 0) )
    {
        fprintf (stderr, "Options --rootfsfile and --kernelfile must be used together\n\n");
        fprintf (stderr, usage, progname);
        exit (1);
    }
    if (strlen(chipid) > (CHIP_ID_LEN - 1))
    {
        fprintf (stderr, "Maximum chip id length is %d characters\n\n", (CHIP_ID_LEN - 1));
        fprintf (stderr, usage, progname);
        exit (1);
    }
    if (strlen(boardid) > 15)
    {
        fprintf (stderr, "Maximum board name length is 15 characters\n\n");
        fprintf (stderr, usage, progname);
        exit (1);
    }
    if (strnlen (imageversion, IMAGE_VER_LEN) > IMAGE_VER_LEN -1)
    {
        imageversion[IMAGE_VER_LEN-1] = '\0';
        imageversion[IMAGE_VER_LEN-2] = '+';
        fprintf (stderr,"\n");
        fprintf (stderr,"WARNING!  bcmImageBuilder truncating version string %s exceeds maximum length\n", imageversion);
        fprintf (stderr,"\n");
    }

    build_image(littleendian, chipid, boardid, blksize, imageversion, 
        outputfile, cfefile, rootfsfile, kernelfile, dtb_mdata_file, includecfe, emmcimage);

    exit(0);
}

