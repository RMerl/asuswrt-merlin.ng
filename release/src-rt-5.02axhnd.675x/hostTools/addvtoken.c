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
// File Name  : addvtoken.c
//
// Description: Add validation token - 20 bytes, to the firmware image file to 
//              be uploaded by CFE 'w' command. For now, just 4 byte crc in 
//              network byte order************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include "include/bcmTargetEndian.h"
#include "version.h"

typedef unsigned char byte;
typedef unsigned int UINT32;
#define BUF_SIZE        100 * 1024

#define  BCMTAG_EXE_USE
#include "bcmTag.h"

byte buffer[BUF_SIZE];
byte vToken[TOKEN_LEN];

#define MAX_TAIL_LEN 64
#define MAX_PID_LEN 20

typedef struct {
	uint8_t major;
	uint8_t minor;
} version_t;

struct tail_t {
	version_t kernel;	/* Kernel version */
	version_t fs;		/* Filsystem version */
	uint16_t  sn;
	uint16_t  en;
	char pid[MAX_PID_LEN];
	uint32_t en2;
	uint8_t pad2[31];
	uint8_t flag;
} tail;

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


int main(int argc, char **argv)
{
    FILE *stream_in, *stream_out;
    int total = 0, count, numWritten = 0, i;
    UINT32 imageCrc;
    PWFI_TAG pWt = (PWFI_TAG) vToken;
    char *p;
    uint32_t v1, v2;
    long filelen = 0;

    memset(vToken, 0, TOKEN_LEN);

    memset(&tail, 0, MAX_TAIL_LEN);
    strncpy(&tail.pid[0], RT_BUILD_NAME, MAX_PID_LEN);

    sscanf(RT_MAJOR_VERSION, "%d.%d", &v1, &v2);
    tail.kernel.major = (uint8_t)v1;
    tail.kernel.minor = (uint8_t)v2;
    sscanf(RT_MINOR_VERSION, "%d.%d", &v1, &v2);
    tail.fs.major = (uint8_t)v1;
    tail.fs.minor = (uint8_t)v2;
    sscanf(RT_SERIALNO, "%d", &v1);
    tail.sn = (uint16_t)v1;
    sscanf(RT_EXTENDNO, "%d-%*s", &v1);
    tail.en = (uint16_t)v1;
    tail.en2 = (uint32_t)v1;
    tail.flag = 1;

    /* check if it is little endian system first */
    if( argc > 4 )
    {
        for( i = 1; i < argc; )
        {
            if( !strcmp(&argv[i][0], "--endian") )
            {
                if(!strcmp(&argv[i+1][0], "little") )
		    BCM_SET_TARGET_ENDIANESS(BCM_TARGET_LITTLE_ENDIAN);
       	        break;
            }
            i += 2;
        }
    }

    while( argc > 1 && argv[1][0] == '-' && argv[1][1] == '-' )
    {
        if( !strcmp(&argv[1][2], "chip") )
        {
            pWt->wfiChipId = BCM_HOST_TO_TARGET32(strtoul(argv[2], NULL, 16));
            pWt->wfiVersion = BCM_HOST_TO_TARGET32(WFI_VERSION);
        }
        else if( !strcmp(&argv[1][2], "flashtype") )
        {
            if( !strcmp(argv[2], "NOR") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NOR_FLASH);
            else if( !strcmp(argv[2], "NAND16") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NAND16_FLASH);
            else if( !strcmp(argv[2], "NAND128") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NAND128_FLASH);
            else if( !strcmp(argv[2], "NAND256") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NAND256_FLASH);
            else if( !strcmp(argv[2], "NAND512") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NAND512_FLASH);
            else if( !strcmp(argv[2], "NAND1024") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NAND1024_FLASH);
            else if( !strcmp(argv[2], "NAND2048") )
                pWt->wfiFlashType = BCM_HOST_TO_TARGET32(WFI_NAND2048_FLASH);
            else
            {
                printf("invalid flash type '%s'\n", argv[2]);
                exit(1);
            }
            pWt->wfiVersion = BCM_HOST_TO_TARGET32(WFI_VERSION);
        }
        else if( !strcmp(&argv[1][2], "ddrtype") )
        {
            if( !strcmp(argv[2], "DDR3") )
                pWt->wfiFlags |= BCM_HOST_TO_TARGET32(WFI_FLAG_DDR_TYPE_DDR3);
            else if( !strcmp(argv[2], "DDR4") )
                pWt->wfiFlags |= BCM_HOST_TO_TARGET32(WFI_FLAG_DDR_TYPE_DDR4);
            else
            {
                printf("invalid ddr type '%s'\n", argv[2]);
                exit(1);
            }
        }
        else if ( !strcmp(&argv[1][2], "pmc"))
        {
	    if( atoi(argv[2]) )
	    {
    	        pWt->wfiFlags |= BCM_HOST_TO_TARGET32(WFI_FLAG_HAS_PMC);
	    }
        }
        else if ( !strcmp(&argv[1][2], "btrm"))
        {
	    if( atoi(argv[2]) )
	    {
    	        pWt->wfiFlags |= BCM_HOST_TO_TARGET32(WFI_FLAG_SUPPORTS_BTRM);
	    }
        }
        else if( strcmp(&argv[1][2], "endian") )
        {
            printf("invalid option '%s'\n", argv[1]);
            exit(1);
        }
       
        argc -= 2;
        argv += 2;
    }

    if( pWt->wfiVersion == BCM_HOST_TO_TARGET32(WFI_VERSION) &&
        (pWt->wfiChipId == 0 || pWt->wfiFlashType == 0) )
    {
        printf("options --chip and --flashtype must both be specified\n");
        exit(1);
    }

    if (argc != 3)
    {
        printf("Usage:\n");
        printf("addvtoken  input-file tag-output-file\n");
        exit(1);
    }
    if( (stream_in  = fopen(argv[1], "rb")) == NULL)
    {
	     printf("failed on open input file: %s\n", argv[1]);
		 exit(1);
    }
    else printf("input file: %s\n", argv[1]);
    if( (stream_out = fopen(argv[2], "wb+")) == NULL)
    {
	     printf("failed on open output file: %s\n", argv[2]);
	     exit(1);
    }

    total = count = 0;
    imageCrc = CRC32_INIT_VALUE;

    fseek(stream_in, 0, SEEK_END);
    filelen = ftell(stream_in);
    fseek(stream_in, 0, SEEK_SET);

    while( !feof( stream_in ) )
    {
        count = fread( buffer, sizeof( char ), BUF_SIZE, stream_in );
        if( ferror( stream_in ) )
        {
            perror( "Read error" );
            exit(1);
        }

	if ((count && (total + count) == filelen) || (count < BUF_SIZE && count > MAX_TAIL_LEN)) {
		p = (char *) buffer + count - MAX_TAIL_LEN;
		memcpy(p, &tail, MAX_TAIL_LEN);
	}

        imageCrc = getCrc32(buffer, count, imageCrc);
        numWritten = fwrite(buffer, sizeof(char), count, stream_out);
        if (ferror(stream_out)) 
        {
            perror("addcrc: Write image file error");
            exit(1);
        }
        total += count;
    }
    
    // *** assume it is always in network byte order (big endia)
    imageCrc = BCM_HOST_TO_TARGET32(imageCrc);
    memcpy(&pWt->wfiCrc, (byte*)&imageCrc, CRC_LEN);

    // write the crc to the end of the output file
    numWritten = fwrite(vToken, sizeof(char), TOKEN_LEN, stream_out);
    if (ferror(stream_out)) 
    {
        perror("addcrc: Write image file error");
        exit(1);
    }

    fclose(stream_in);
    fclose(stream_out);

    printf( "addvtoken: %s Output file size = %d with image crc = 0x%x\n", BCM_GET_TARGET_ENDIANESS_STR() , total+TOKEN_LEN, imageCrc);

    exit(0);
}
