/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
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
/***************************************************************************
 * File Name  : createimg.c
 *
 * Description: This program creates a complete flash image for a BCM963xx
 *              board from an input file that is created by the
 *              bcmImageBuilder utility.
 *
 * Build      : $ gcc -o createimg createimg.c boardparms.c
 *
 * Example    : createimg -b 96362GW -c SI3217X -n 4 -m 02:10:18:18:12:01 -t 0 -p 24
 *              -i bcm96362GW_cfe_fs_kernel -o bcm96362GW_cfe_fs_kernel.img 
 *              OR
 *              -f whole_flash_image.w
 *                 only sets NVRAM_DATA
 *              where:
 *              -b = board id
 *              -c = voice board id
 *              -n = number of mac address
 *              -m = base mac address
 *              -t = main thread number
 *              -p = PSI Size
 *              -i = input file name
 *              -o = output file name
 *
 ***************************************************************************/

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <math.h>
#include "include/bcmTargetEndian.h"
typedef unsigned char byte;
typedef unsigned int UINT32;
#define  BCMTAG_EXE_USE
#include "bcmTag.h"
#include "boardparms.h"
#define MAX_MAC_STR_LEN     19         // mac address string 18+1 in regular format - 02:18:10:11:aa:bb 
#include "board.h"

#define OPT_LEN_IS_VALID(_maxLen) ( strlen(optarg) < (_maxLen) )

static struct option longopts[] = 
{
    { "boardid",        required_argument, 0, 'b'},
    { "voiceboardid",   required_argument, 0, 'c'},
    { "numbermac",      required_argument, 0, 'n'},
    { "macaddr",        required_argument, 0, 'm'},
    { "tp",             optional_argument, 0, 't'},
    { "psisize",        required_argument, 0, 'p'},
    { "logsize",        required_argument, 0, 'l'},
    { "backuppsi",      required_argument, 0, 'k'},
    { "flblksz",        required_argument, 0, 'z'},
    { "auxfsprcnt",     required_argument, 0, 'a'},
    { "gponsn",         required_argument, 0, 's'},
    { "gponpw",         required_argument, 0, 'w'},
    { "ctgponloid",     required_argument, 0, 'd'},
    { "ctgponpw",       required_argument, 0, 'g'},
    { "inputfile",      required_argument, 0, 'i'},
    { "outputfile",     required_argument, 0, 'o'},
    { "wholeflashfile", required_argument, 0, 'f'},
    { "endian",         required_argument, 0, 'e'},
    { "misc1",          required_argument, 0,  1},
    { "misc2",          required_argument, 0,  2},
    { "misc3",          required_argument, 0,  3},
    { "misc4",          required_argument, 0,  4},
    { "help",           no_argument,       0, 'h'},
    { "version",        no_argument,       0, 'v'},
    { 0,                0,                 0,  0 }
};

static char version[] = "Broadcom Flash Image Creator version 0.6";

static char usage[] = 
"Usage:\t%s [-bnmtpswiohve] [--help] [--version]\n"
"\texample:\n"
"\tcreateimg -b 96362GW -n 12 -m 02:10:18:18:12:01 -t 0 "
"-i cfe_fs_kernel -o cfe_fs_kernel.img\n"
"\twhere:\n"
"\t[-b] board id name\n"
"\t[-n] number of mac address\n"
"\t[-m] base mac address\n"
"\t[-t] main thread number\n"
"\t[-p] PSI Size\n"
"\t[-l] System Log Size\n"
"\t[-k] Backup Psi Support\n"
"\t[-z] Flash block size in Kbytes\n"
"\t[-a] Aux file system percent of flash\n"
"\t[-s] GPON Serial Number\n"
"\t[-w] GPON Password\n"
"\t[-i] input image name\n"
"\t[-o] ouput image name\n"
"\t[-f] whole flash image name\n"
"\t[-e] target endianess\n"
"\t[-h] help\n"
"\t[-v] version\n";

static char *progname;

unsigned short partition_info[BCM_MAX_EXTRA_PARTITIONS]={0xffff, 0xffff, 0xffff, 0xffff};

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
// Function Name: parsexdigit
// Description  : parse hex digit
// Parameters   : input char
// Returns      : hex int
****************************************************************************/
static int parsexdigit(char str)
{
    int digit;

    if ((str >= '0') && (str <= '9')) 
        digit = str - '0';
    else if ((str >= 'a') && (str <= 'f')) 
        digit = str - 'a' + 10;
    else if ((str >= 'A') && (str <= 'F')) 
        digit = str - 'A' + 10;
    else 
        return -1;

    return digit;
}

/***************************************************************************
// Function Name: parsehwaddr
// Description  : convert in = "255.255.255.0" to fffffff00
// Parameters   : str = "255.255.255.0" ; 
// Return       : if not -1,  hwaddr = fffffff00
****************************************************************************/
int parsehwaddr(unsigned char *str, unsigned char *hwaddr)
{
    int digit1,digit2;
    int idx = 6;

    if (strlen(str) != MAX_MAC_STR_LEN-2)
        return -1;
    if (*(str+2) != ':' || *(str+5) != ':' || *(str+8) != ':' || *(str+11) != ':' || *(str+14) != ':')
        return -1;
    
    while (*str && (idx > 0)) {
        digit1 = parsexdigit(*str);
        if (digit1 < 0)
            return -1;
        str++;
        if (!*str)
            return -1;

        if (*str == ':') {
            digit2 = digit1;
            digit1 = 0;
        }
        else {
            digit2 = parsexdigit(*str);
            if (digit2 < 0)
                return -1;
            str++;
        }

        *hwaddr++ = (digit1 << 4) | digit2;
        idx--;

        if (*str == ':') 
            str++;
    }
    return 0;
}

/***************************************************************************
 * Function Name: fillInNvramData
 * Description  : Sets the fields of the NVRAM_DATA structure.
 * Returns      : None
 ***************************************************************************/
void fillInNvramData(char *szBoardId, char *szVoiceBoardId, int numOfMac,
    unsigned char *ucaBaseMac, int tpNum, int psiSize,int logSize, int backupPsi,
              int   flashBlkSize, int auxFSPercent,
    unsigned char *gponSerialNbr, unsigned char *gponPasswd,
    PNVRAM_DATA pNvramData)
{
    UINT32 crc = CRC32_INIT_VALUE; 

    memset((char *)pNvramData, 0xFF, sizeof(NVRAM_DATA));
    pNvramData->ulVersion = BCM_HOST_TO_TARGET32(NVRAM_VERSION_NUMBER);
    strncpy(pNvramData->szBoardId, szBoardId, NVRAM_BOARD_ID_STRING_LEN);
    strncpy(pNvramData->szVoiceBoardId, szVoiceBoardId,
        NVRAM_BOARD_ID_STRING_LEN);
    pNvramData->ulNumMacAddrs = BCM_HOST_TO_TARGET32(numOfMac);
    memcpy(pNvramData->ucaBaseMacAddr, ucaBaseMac, NVRAM_MAC_ADDRESS_LEN);
    pNvramData->ulMainTpNum = BCM_HOST_TO_TARGET32(tpNum);
    pNvramData->ulPsiSize = BCM_HOST_TO_TARGET32(psiSize);
    pNvramData->ulSyslogSize = BCM_HOST_TO_TARGET32(logSize);
    pNvramData->backupPsi = (char)backupPsi;
    pNvramData->ucFlashBlkSize = (unsigned char)flashBlkSize;
    pNvramData->ucAuxFSPercent = (unsigned char)auxFSPercent;
    strncpy(pNvramData->gponSerialNumber, gponSerialNbr,
        NVRAM_GPON_SERIAL_NUMBER_LEN);
    strncpy(pNvramData->gponPassword, gponPasswd, NVRAM_GPON_PASSWORD_LEN);
    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, (UINT32) sizeof(NVRAM_DATA), crc);      
    pNvramData->ulCheckSum = BCM_HOST_TO_TARGET32(crc);
} /* fillInNvramData */

void fillInPartiInfo(PNVRAM_DATA pNvramData, int index, int size)
{
    UINT32 crc = CRC32_INIT_VALUE; 

    pNvramData->part_info[index].size = BCM_HOST_TO_TARGET16(size);
    pNvramData->ulCheckSum = 0;
    crc = getCrc32((char *)pNvramData, (UINT32) sizeof(NVRAM_DATA), crc);      
    pNvramData->ulCheckSum = BCM_HOST_TO_TARGET32(crc);
    return;
}

/***************************************************************************
 * Function Name: fillInOutputBuffer
 * Description  : Fills the output buffer with the flash image contents.
 * Returns      : len of output buffer - OK, -1 - failed.
 ***************************************************************************/
char * fillInOutputBuffer(PNVRAM_DATA pNvramData, char *pszIn, int *nLen)
{
    PFILE_TAG pTag = (PFILE_TAG) pszIn;
    char *pszOut = NULL;
    uint32_t ulCfeOffset = 0;
    uint32_t ulNvramOffset = NVRAM_DATA_OFFSET;
    uint32_t ulTagOffset = 0;
    uint32_t ulFsOffset = 0;
    uint32_t ulKernelOffset = 0;
    uint32_t ulDtbOffset = 0;
    uint32_t ulCfeLen = 0;
    uint32_t ulNvramLen = sizeof(NVRAM_DATA);
    uint32_t ulTagLen = 0;
    uint32_t ulFsLen = 0;
    uint32_t ulKernelLen = 0;
    uint32_t ulDtbLen = 0;
    uint32_t spaceLeft = 0;

    if ((ulCfeLen = strtoul(pTag->cfeLen, NULL, 10)) != 0)
        ulCfeOffset = strtoul(pTag->cfeAddress, NULL, 10) - IMAGE_BASE;
    else
        ulCfeOffset = 0;

    ulFsOffset = strtoul(pTag->rootfsAddress, NULL, 10) - IMAGE_BASE;           
    ulFsLen = strtoul(pTag->rootfsLen, NULL, 10);

    ulTagOffset = ulFsOffset - TAG_LEN;           
    ulTagLen = TAG_LEN;

    ulKernelOffset = strtoul(pTag->kernelAddress, NULL, 10) - IMAGE_BASE;
    ulKernelLen = strtoul(pTag->kernelLen, NULL, 10);

    ulDtbOffset = strtoul(pTag->dtbAddress, NULL, 10) - IMAGE_BASE;
    ulDtbLen = strtoul(pTag->dtbLen, NULL, 10);

    printf("\tImage components offsets\n");
    printf("\timage base              : 0x%8.8x\n", 
        IMAGE_BASE);
    printf("\tCFE offset              : 0x%8.8x    -- Length: 0x%x\n", 
        ulCfeOffset, ulCfeLen);
    printf("\tNVRAM offset            : 0x%8.8x    -- Length: 0x%x\n",
        ulNvramOffset + ulCfeOffset, ulNvramLen);
    printf("\tfile tag offset         : 0x%8.8x    -- Length: 0x%x\n", 
        ulTagOffset, ulTagLen);
    printf("\trootfs offset           : 0x%8.8x    -- Length: 0x%x\n", 
        ulFsOffset, ulFsLen);
    printf("\tkernel offset           : 0x%8.8x    -- Length: 0x%x\n",
        ulKernelOffset, ulKernelLen);
    printf("\tDTB offset              : 0x%8.8x    -- Length: 0x%x\n",
        ulDtbOffset, ulDtbLen);
    /*
     * This check is a minimum sanity check.  The actual size of kernel
     * that the board can accept depends on whether scratch pad, 
     * backup psi, persistent syslog is defined and how big they are
     * and how many sectors these things actually take up.  If the
     * image is too big for the board, the cfe and kernel will still
     * reject this image.
     */
    *nLen = ulKernelOffset + ulKernelLen + ulDtbOffset + BCM_TARGET32_TO_HOST(pNvramData->ulPsiSize);

    pszOut = (char *) malloc(*nLen);

    if (!pszOut)
    {
        printf( "Memory allocation error output file, attempted to allocate 0x%x\n\n", *nLen);
        return(0);
    }

    /* Fill in the output buffer with the flash image. */
    memset( (unsigned char *) pszOut, 0xff, *nLen );

    printf( "\tThe size of the entire flash image is %d bytes.\n", *nLen );

    printf( "\tA %d MB or greater flash part is needed.\n", 
        (int)pow(2, ceil(log((double)*nLen/(ONEK*ONEK))/log(2))));
    printf( "\tA %d MB or greater flash part is needed for dual image.\n\n", 
        2*(int)pow(2, ceil(log((double)*nLen/(ONEK*ONEK))/log(2))));

    memcpy( pszOut + ulCfeOffset, pszIn + TAG_LEN, ulCfeLen ); 
    // the tag is before fs now...    
    memcpy( pszOut + ulFsOffset - TAG_LEN, pszIn, TAG_LEN );
    memcpy( pszOut + ulFsOffset, pszIn + TAG_LEN + ulCfeLen, ulFsLen ); 
    memcpy( pszOut + ulKernelOffset, pszIn + TAG_LEN + ulCfeLen + ulFsLen, ulKernelLen ); 
    memcpy( pszOut + ulDtbOffset, pszIn + TAG_LEN + ulCfeLen + ulFsLen + ulKernelLen, ulDtbLen ); 

    // copy nvram data. No default bootline. CFE will create one on the first boot
    memcpy(pszOut + NVRAM_DATA_OFFSET, (char *)pNvramData, sizeof(NVRAM_DATA));

    return pszOut;

} /*fillInOutputBuffer */


/***************************************************************************
 * Function Name: verifyTag
 * Description  : check on the input file by tag verification
 * Returns      : 0 - ok, -1 bad input image file
 ***************************************************************************/
int verifyTag(PFILE_TAG pTag)
{
    UINT32 crc;
    int status = 0;

    // check tag validate token first
    crc = CRC32_INIT_VALUE;
    crc = getCrc32((char*) pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);
    crc = BCM_HOST_TO_TARGET32(crc);

    if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
    {
        printf("Illegal image ! Tag crc failed.\n");
        status = -1;
    }

    return status;
}

/***************************************************************************
 * Function Name: createImage
 * Description  : Creates the flash image file.
 * Returns      : None.
 ***************************************************************************/
void createImage(PNVRAM_DATA pNvramData, char *inputFile, char *outputFile)
{
    FILE *hInput = NULL;
    struct stat StatBuf;
    char *pszIn = NULL, *pszOut = NULL;

    if (stat(inputFile, &StatBuf ) == 0 && (hInput = fopen(inputFile, "rb" )) == NULL)
    {
        printf( "createimg: Error opening input file %s.\n\n", inputFile);
        return;
    }

    /* Allocate buffers to read the entire input file and to write the
     * entire flash.
     */
    pszIn = (char *) malloc(StatBuf.st_size);
    if (!pszIn)
    {
        printf( "Memory allocation error input file, attempted to allocate 0x%x\n\n", (int)StatBuf.st_size);
        fclose( hInput );
        return;
    }

    /* Read the input file into memory. */
    if (fread( pszIn, sizeof(char), StatBuf.st_size, hInput ) == StatBuf.st_size)
    {
        /* Verify that FILE_TAG fields are OK. */
        if( verifyTag((PFILE_TAG)pszIn) == 0)
        {
            FILE *hOutput;
            int nLen;

            if ((pszOut = fillInOutputBuffer(pNvramData, pszIn, &nLen)) != 0)
            {
                /* Open output file. */
                if ((hOutput = fopen(outputFile, "w+" )) != NULL)
                {
                    /* Write the output buffer to the output file. */
                    if (fwrite(pszOut, sizeof(char), nLen, hOutput) == nLen)
                        printf( "\t%s flash image file is\n\tsuccessfully created.\n\n", outputFile);
                    else
                        printf( "createimg: Error writing to output file.\n\n" );
                    fclose( hOutput );
                }
                else
                    printf ("createimg: Error opening output file %s.\n\n", outputFile);
            }
        }
    }

    if( pszIn )
        free( pszIn );

    if( pszOut )
        free( pszOut );
    
    fclose( hInput );

} /* createImage */

/***************************************************************************
 * Function Name: writeNvramData
 * Description  : Writes a filled in NVRAM_DATA structure to a whole flash
 *                image file.
 * Returns      : None.
 ***************************************************************************/
void writeNvramData(PNVRAM_DATA pNvramData, char *wholeFlashFile)
{
    FILE *hFile = NULL;
    struct stat StatBuf;
    unsigned char *pFile = NULL;
    unsigned int *pBuf = NULL;
    WFI_TAG* wfiTag;
    unsigned int nvram_base = 0;

    if( stat(wholeFlashFile, &StatBuf) == 0 &&
        (hFile = fopen(wholeFlashFile, "rb")) == NULL)
    {
        printf("createimg: Error opening whole flash file %s for reading.\n\n",
            wholeFlashFile);
        return;
    }

    /* Allocate buffers to read the entire whole flash file */
    pFile = (char *) malloc(StatBuf.st_size);
    if (!pFile)
    {
        printf("Memory allocation error, 0x%p\n\n",pFile);
        fclose(hFile);
        return;
    }

    /* Read the file into memory. */
    if(fread(pFile, sizeof(char), StatBuf.st_size, hFile) == StatBuf.st_size)
    {
        UINT32 crc, crc_cmp;

        fclose(hFile);
        if((hFile = fopen(wholeFlashFile, "wb")) != NULL)
        {
            /* Verify the CRC before modification. Reference 32 bit CRC value
             * in the file buffer as bytes in case it is not 32 bit aligned.
             */
            crc = getCrc32(pFile,StatBuf.st_size - TOKEN_LEN,CRC32_INIT_VALUE);
            crc = BCM_HOST_TO_TARGET32(crc);
            crc_cmp = (UINT32)(*(UINT32*)(&pFile[StatBuf.st_size - TOKEN_LEN]));
            if( crc == crc_cmp )
            {
	        /* check for 63138 or 63148 if it contain pmc which affect the nvram offset location */
	        wfiTag = (WFI_TAG*)(&pFile[StatBuf.st_size - TOKEN_LEN]);
                if( (BCM_TARGET32_TO_HOST(wfiTag->wfiChipId) == 0x63138) || (BCM_TARGET32_TO_HOST(wfiTag->wfiChipId) == 0x63148) )
		{
		    /* if contains pmc, nvram is 64K+0x580 offset */
  		    if( BCM_TARGET32_TO_HOST(wfiTag->wfiFlags)&WFI_FLAG_HAS_PMC )  
		        nvram_base = 0x10000;
                    else
		    {
		        /*if it has all zero padding in the front, the nvram is at 64K+0x580 offset too*/
		        pBuf = (unsigned int*)pFile; 
                        /* check first 16 bytes */
		        if( *pBuf == 0 && *(pBuf+1) == 0 && *(pBuf+2) == 0 && *(pBuf+2) == 0 )
		            nvram_base = 0x10000;
		    }
		}

                memcpy(pFile + NVRAM_DATA_OFFSET + nvram_base, (char *)pNvramData,
                    sizeof(NVRAM_DATA));

                crc = getCrc32( pFile, StatBuf.st_size - TOKEN_LEN,
                    CRC32_INIT_VALUE );
                crc = BCM_HOST_TO_TARGET32(crc);
                *((UINT32*)(&pFile[StatBuf.st_size - TOKEN_LEN])) = crc;

                rewind(hFile);
                if( fwrite(pFile, 1, StatBuf.st_size,hFile) != StatBuf.st_size)
                    printf( "createimg: Error writing to output file.\n\n" );
            }
            else
                printf("createimg: Invalid whole flash image file\n");

            fclose( hFile );
        }
        else
            printf("createimg: Error opening whole flash file %s for "
                "writing.\n\n", wholeFlashFile);
    }
    else
        printf ("createimg: Error reading file.\n\n");

    if( pFile )
        free( pFile );
} /* writeNvramData */

/*************************************************************
 * Function Name: main
 * Description  : Program entry point that parses command line parameters
 *                and calls a function to create the image.
 * Returns      : 0
 ***************************************************************************/
int main (int argc, char **argv)
{
    int optc;
    int option_index = 0;
    int h = 0, v = 0, lose = 0;
    char inputFile[256], outputFile[256], macString[256], wholeFlashFile[256];
    char szBoardId[BP_BOARD_ID_LEN] = "";
    char szVoiceBoardId[BP_BOARD_ID_LEN] = "";
    int numOfMac = DEFAULT_MAC_NUM, tpNum = DEFAULT_TP_NUM, psiSize = DEFAULT_PSI_SIZE,
        logSize= DEFAULT_LOG_SIZE,
        backupPsi = DEFAUT_BACKUP_PSI, flashBlkSize = DEFAULT_FLASHBLK_SIZE,
        auxFSPercent = DEFAULT_AUXFS_PERCENT;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    unsigned char gponSerialNbr[NVRAM_GPON_SERIAL_NUMBER_LEN];
    unsigned char gponPasswd[NVRAM_GPON_PASSWORD_LEN];
    NVRAM_DATA nvramData;  

    /* Parse command line options. */
    progname = argv[0];
    inputFile[0] = outputFile[0] = macString[0] = wholeFlashFile[0] = gponSerialNbr[0] = gponPasswd[0] = '\0';
  
    while ((optc = getopt_long (argc, argv, "vhn:m:t::p:s:w:d:g:b:i:o:w:e:f:", longopts, &option_index)) != EOF) 
    {
        switch (optc)
        {
            case 'v':
                v = 1;
                break;
            case 'h':
                h = 1;
                break;
            case 'n':
                numOfMac = strtoul(optarg, NULL, 10);
                break;
            case 'm':
                strcpy(macString, optarg);
                break;
            case 't':
                if (optarg)
                    tpNum = strtoul(optarg, NULL, 10);
                break;
            case 'p':
                psiSize = strtoul(optarg, NULL, 10);
                break;
            case 'l':
                logSize = strtoul(optarg, NULL, 10);
                break;
            case 'k':
                backupPsi = strtoul(optarg, NULL, 10);
                break;
            case 'z':
                flashBlkSize = strtoul(optarg, NULL, 10);
                break;
            case 'a':
                auxFSPercent = strtoul(optarg, NULL, 10);
                break;
            case 's':
                if(OPT_LEN_IS_VALID(NVRAM_GPON_SERIAL_NUMBER_LEN)) {
                    strcpy(gponSerialNbr, optarg);
                }
                else {
                    lose = 1;
                }
                break;
            case 'w':
                if(OPT_LEN_IS_VALID(NVRAM_GPON_PASSWORD_LEN)) {
                    strcpy(gponPasswd, optarg);
                }
                else {
                    lose = 1;
                }
                break;
            case 'i':
                strcpy(inputFile, optarg);
                break;
            case 'o':
                strcpy(outputFile, optarg);
                break;
            case 'f':
                strcpy(wholeFlashFile, optarg);
                break;
            case 'b':
                if(OPT_LEN_IS_VALID(BP_BOARD_ID_LEN)) {
                    strcpy(szBoardId, optarg);
                }
                else {
                    lose = 1;
                }
                break;
            case 'c':
                if(OPT_LEN_IS_VALID(BP_BOARD_ID_LEN)) {
                    strcpy(szVoiceBoardId, optarg);
                }
                else {
                    lose = 1;
                }
                break;
            case 'e':
	        if(strncmp(optarg, "little", 6) == 0) {
                    BCM_SET_TARGET_ENDIANESS(BCM_TARGET_LITTLE_ENDIAN);
	        }
	        break;
            case 1:
            case 2:
            case 3:
            case 4:
	        partition_info[optc-1] = atoi(optarg);
	        break;
            case '?':
            default:
                lose = 1;
                break;
         }
    }

    if (v) 
    {
        /* Print version number.  */
        fprintf (stderr, "%s\n", version);
        if (!h)
            exit(0);
    }

    if (h) 
    {
        /* Print help info and exit.  */
        fprintf(stderr, usage, progname);
        exit(0);
    }

    if (lose || optind < argc || argc < 2 || 
        ((inputFile[0] == '\0' || outputFile[0] == '\0') &&
          wholeFlashFile[0] == '\0') ||
        (parsehwaddr(macString, ucaBaseMacAddr) == -1))
    {
        fprintf (stderr, usage, progname);
        exit(1);
    }

    printf("createimg: Creating image with the following inputs:\n" );
    printf("\tBoard id                : %s\n", szBoardId);
    printf("\tVoice board id          : %s\n", szVoiceBoardId);
    printf("\tNumber of Mac Addresses : %d\n", numOfMac);
    printf("\tBase Mac Address:       : %s\n", macString);
    printf("\tMain Thread Number:     : %d\n", tpNum);
    printf("\tPSI Size:               : %d\n", psiSize);
    printf("\tLog Size:               : %d\n", logSize);
    printf("\tBackup PSI:               : %d\n", backupPsi);
    printf("\tFlash Block Size Kbytes : %d\n", flashBlkSize);
    printf("\tAuxillaryFileSys Percent: %d\n", auxFSPercent);

    if(strlen(gponSerialNbr))
    {
        printf("\tGPON Serial Number:     : %s\n", gponSerialNbr);
    }
    if(strlen(gponPasswd))
    {
        printf("\tGPON Password:          : %s\n", gponPasswd);
    }

    printf("\tInput File Name         : %s\n", inputFile);
    printf("\tOutput File Name        : %s\n", outputFile);
    printf("\tTarget Endianess        : %s\n\n", BCM_GET_TARGET_ENDIANESS_STR());

    fillInNvramData(szBoardId, szVoiceBoardId, numOfMac, ucaBaseMacAddr, tpNum,
            psiSize, logSize, backupPsi,
                flashBlkSize, auxFSPercent,
            gponSerialNbr, gponPasswd, &nvramData);

    for (v=0; v < BCM_MAX_EXTRA_PARTITIONS; v++)
    {
        if(partition_info[v] != 0xffff)
            fillInPartiInfo(&nvramData, v, partition_info[v]);
    }

    if( wholeFlashFile[0] == '\0' )
        createImage(&nvramData, inputFile, outputFile);
    else
        writeNvramData(&nvramData, wholeFlashFile);

    return(0);
} /* main */

