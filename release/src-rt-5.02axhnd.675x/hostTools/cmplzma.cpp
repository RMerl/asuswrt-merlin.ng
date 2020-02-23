/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
:>
 *
 ************************************************************************/

/***************************************************************************
 * File Name  : cmplzma.c
 *
 * Description: This program compress binary file with lzma compression method
 *              It requires big endian mips32 ELF and the bin files as the input 
 *              for CFE and vmlinux.
 *
 *              For CFE RAM compress:
 *              Command: cmplzma [-t] -c -2 cfe cfe.bin flashimg.S
 *              where cfe is elf, cfe.bin is 
 *              binary file to be compressed and flashimg.S is  Asm data array for
 *              the compressed binary to be linked in as data. 
 *
 *              For vmlinux:
 *              Command: cmplzma [-t] -k -2 vmlinux vmlinux.bin vmlinux.lz
 *              where vmlinux is the elf file, vmliux.bin the binary file
 *              and vmlinux.lz is the compressed output
 *
 * Updates    : 04/08/2003  seanl.  Created.
 *               11/14/2007  xiwang  Use LZMA 4.4.3 utility to compress image, instead of calling a compress function.
 *                                   The LZMA header format is slightly different. The new format has uncompressed size in it.
 *                                   LZMA in old CFE cannot decompress new image, but LZMA in new CFE can decompress both.
 *
 ***************************************************************************/

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <time.h>
#include "include/bcmTargetEndian.h"

// elf structs and defines from CFE

/* p_type */
#define	PT_NULL		0		/* Program header table entry unused */
#define PT_LOAD		1		/* Loadable program segment */
#define PT_DYNAMIC	2		/* Dynamic linking information */
#define PT_INTERP	3		/* Program interpreter */
#define PT_NOTE		4		/* Auxiliary information */
#define PT_SHLIB	5		/* Reserved, unspecified semantics */
#define PT_PHDR		6		/* Entry for header table itself */
#define PT_LOPROC	0x70000000	/* Processor-specific */
#define PT_HIPROC	0x7FFFFFFF	/* Processor-specific */

#define CFE_ERR_NOTELF		-11
#define CFE_ERR_BADCLASS 	-12
#define CFE_ERR_WRONGENDIAN 	-13
#define CFE_ERR_BADELFVERS 	-14
#define CFE_ERR_NOTMIPS 	-15
#define CFE_ERR_BADELFFMT 	-16
#define CFE_ERR_BADADDR 	-17

/* e_indent */
#define EI_MAG0		 0		/* File identification byte 0 index */
#define EI_MAG1		 1		/* File identification byte 1 index */
#define EI_MAG2		 2		/* File identification byte 2 index */
#define EI_MAG3		 3		/* File identification byte 3 index */
#define EI_CLASS	 4		/* File class */

#define ELFCLASSNONE 0		 /* Invalid class */
#define ELFCLASS32	 1		 /* 32-bit objects */
#define ELFCLASS64	 2		 /* 64-bit objects */
#define EI_DATA		 5		/* Data encoding */

#define ELFDATANONE	 0		 /* Invalid data encoding */
#define ELFDATA2LSB	 1		 /* 2's complement, little endian */
#define ELFDATA2MSB	 2		 /* 2's complement, big endian */
#define EI_VERSION	 6		/* File version */
#define EI_PAD		 7		/* Start of padding bytes */

#define ELFMAG0		0x7F	/* Magic number byte 0 */
#define ELFMAG1		'E'		/* Magic number byte 1 */
#define ELFMAG2		'L'		/* Magic number byte 2 */
#define ELFMAG3		'F'		/* Magic number byte 3 */

typedef unsigned short	Elf32_Half;
typedef unsigned int	Elf32_Word;
typedef signed int	Elf32_Sword;
typedef unsigned int	Elf32_Off;
typedef unsigned int	Elf32_Addr;
typedef unsigned char	Elf_Char;

typedef unsigned long long	Elf64_Addr;
typedef unsigned short		Elf64_Half;
typedef signed short		Elf64_SHalf;
typedef unsigned long long	Elf64_Off;
typedef signed int		Elf64_Sword;
typedef unsigned int		Elf64_Word;
typedef unsigned long long	Elf64_Xword;
typedef long long		Elf64_Sxword;

/*
 * ELF File Header 
 */
#define EI_NIDENT	16
typedef struct {
    Elf_Char	e_ident[EI_NIDENT];
    Elf32_Half	e_type;
    Elf32_Half	e_machine;
    Elf32_Word	e_version;
    Elf32_Addr	e_entry;
    Elf32_Off	e_phoff;
    Elf32_Off	e_shoff;
    Elf32_Word	e_flags;
    Elf32_Half	e_ehsize;
    Elf32_Half	e_phentsize;
    Elf32_Half	e_phnum;
    Elf32_Half	e_shentsize;
    Elf32_Half	e_shnum;
    Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_hdr {
    Elf_Char	e_ident[EI_NIDENT];	/* ELF "magic number" */
    Elf64_Half	e_type;
    Elf64_Half	e_machine;
    Elf64_Word	e_version;
    Elf64_Addr	e_entry;		/* Entry point virtual address */
    Elf64_Off	e_phoff;		/* Program header table file offset */
    Elf64_Off	e_shoff;		/* Section header table file offset */
    Elf64_Word	e_flags;
    Elf64_Half	e_ehsize;
    Elf64_Half	e_phentsize;
    Elf64_Half	e_phnum;
    Elf64_Half	e_shentsize;
    Elf64_Half	e_shnum;
    Elf64_Half	e_shstrndx;
} Elf64_Ehdr;


/*
 * Program Header 
 */
typedef struct {
    Elf32_Word	p_type;			/* Identifies program segment type */
    Elf32_Off	p_offset;		/* Segment file offset */
    Elf32_Addr	p_vaddr;		/* Segment virtual address */
    Elf32_Addr	p_paddr;		/* Segment physical address */
    Elf32_Word	p_filesz;		/* Segment size in file */
    Elf32_Word	p_memsz;		/* Segment size in memory */
    Elf32_Word	p_flags;		/* Segment flags */
    Elf32_Word	p_align;		/* Segment alignment, file & memory */
} Elf32_Phdr;

typedef struct elf64_phdr {
    Elf64_Word	p_type;
    Elf64_Word	p_flags;
    Elf64_Off	p_offset;		/* Segment file offset */
    Elf64_Addr	p_vaddr;		/* Segment virtual address */
    Elf64_Addr	p_paddr;		/* Segment physical address */
    Elf64_Xword	p_filesz;		/* Segment size in file */
    Elf64_Xword	p_memsz;		/* Segment size in memory */
    Elf64_Xword	p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;

int getElfInfo(char *elfFile, unsigned int *eEntry, unsigned int *pVaddr);

// Check the elf file validity and extract the program entry and text 
// start address. For our system, all our address is below 4GB/32 bit range
// so always just need to return unsigned int address even for elf64 image
int getElfInfo(char *elfFile, unsigned int *eEntry, unsigned int *pVaddr)
{
    Elf32_Ehdr *ep;
    Elf64_Ehdr *ep64;
    Elf32_Phdr *phtab = 0;
    Elf64_Phdr *phtab64 = 0;
    unsigned char *tab;
    unsigned int nbytes, offset;
    int i, elf64 = 0, numhdr = 0;
    Elf64_Ehdr ehdr;
    FILE *hInput;

    if ((hInput = fopen(elfFile, "rb")) == NULL)
    {
        printf("Error open file: %s\n", elfFile);
        return -1;
    }

    /* read enough bytes for both elf32 and elf64 image */
    if (fread((char *) &ehdr, sizeof(char), sizeof(ehdr), hInput) != sizeof(ehdr))
    {
        printf("Error reading file: %s\n", elfFile);
        return -1;
    }

    ep = (Elf32_Ehdr*)&ehdr;
    ep64 = &ehdr;

    /* check header validity */
    if (ep->e_ident[EI_MAG0] != ELFMAG0 ||
        ep->e_ident[EI_MAG1] != ELFMAG1 ||
	    ep->e_ident[EI_MAG2] != ELFMAG2 ||
	    ep->e_ident[EI_MAG3] != ELFMAG3) 
    {
        printf("Not ELF file\n");
        return CFE_ERR_NOTELF;
    }

    if (ep->e_ident[EI_CLASS] == ELFCLASS32) 
    {
        *eEntry = ep->e_entry;
    }
    else if(ep->e_ident[EI_CLASS] == ELFCLASS64)
    {
        *eEntry = (unsigned int)(ep64->e_entry);
        elf64 =1;
    }
    else
    {
        printf("Not valid elf file\n");
        return  CFE_ERR_BADCLASS;
    }
    
    if(ep->e_ident[EI_DATA] == ELFDATA2LSB)
    {
        BCM_SET_TARGET_ENDIANESS(BCM_TARGET_LITTLE_ENDIAN);
    }
    else if(ep->e_ident[EI_DATA] != ELFDATA2MSB)
    {
        printf("Invalid Endian\n");
        return CFE_ERR_WRONGENDIAN;	/* big endian */
    }

    /* Is there a program header? */
    if ( ((ep->e_phoff == 0 || ep->e_phnum == 0) && elf64 == 0) 
	 || ((ep64->e_phoff == 0 || ep64->e_phnum == 0) && elf64 == 1) )
    {
        printf("No program header? Wrong elf file\n");
        return CFE_ERR_BADELFFMT;
    }

    /* Load program header */
    if( elf64 )
    {
        numhdr = ep64->e_phnum = BCM_HOST_TO_TARGET16(ep64->e_phnum);
        offset = ep64->e_phoff = BCM_HOST_TO_TARGET64(ep64->e_phoff);
        nbytes = ep64->e_phnum * sizeof(Elf64_Phdr);
    }
    else
    {
        numhdr = ep->e_phnum = BCM_HOST_TO_TARGET16(ep->e_phnum);
        offset = ep->e_phoff = BCM_HOST_TO_TARGET32(ep->e_phoff);
        nbytes = ep->e_phnum * sizeof(Elf32_Phdr);
    }
    
    tab = (unsigned char *) malloc(nbytes);
    if (!tab) 
    {
        printf("Failed to malloc memory!\n");
        return -1;
    }

    if (fseek(hInput, offset, SEEK_SET)!= 0)
    {
        free(phtab);
        printf("File seek error\n");
        return -1;
    }
    if (fread((unsigned char *)tab, sizeof(char), nbytes, hInput) != nbytes)
    {
        free(tab);
        printf("File read error\n");
        return -1;
    }

    for (i = 0; i < numhdr; i++)
    {
        unsigned int lowest_offset = ~0;
        Elf32_Phdr *ph = 0;
        Elf64_Phdr *ph64 = 0;
        unsigned int offset, type;

        if( elf64 )
	{
            phtab64 = (Elf64_Phdr*)tab;
            ph64 = &phtab64[i];
            offset = BCM_HOST_TO_TARGET64(ph64->p_offset);
            type = BCM_HOST_TO_TARGET32(ph64->p_type);
	}
	else
	{
            phtab = (Elf32_Phdr*)tab;
            ph = &phtab[i];
            offset = BCM_HOST_TO_TARGET32(ph->p_offset);
            type = BCM_HOST_TO_TARGET32(ph->p_type);
	}
        
	if ((type == PT_LOAD) && (offset < lowest_offset)) 
        {
            lowest_offset = offset;
            if( elf64 )
                *pVaddr = ph64->p_vaddr;      // found the text start address
            else
                *pVaddr = ph->p_vaddr;      // found the text start address
            return 0;
        }
    }
    printf("No text start address found! Wrong elf file ?\n");
    return -1;
}

void usage(char *pName)
{
    printf("Example:\n");
    printf("To compress CFE ram     :  %s -c -2 inputElfFile inputBinFile outputAsmFile\n", pName);
    printf("To compress linux Kernel:  %s -k -2 inputElfFile inputBinFile outputCompressedFile\n\n", pName);
    printf("NOTE: -2 is the default compression level.  Allowable levels are -1 through -3\n");
    printf("where -3 may yield better compression ratio but slower. For faster compression, use -1\n");
    exit(-1);
}

/*************************************************************
 * Function Name: main
 * Description  : Program entry point that parses command line parameters
 *                and calls a function to create the image.
 * Returns      : 0
 ***************************************************************************/
int main (int argc, char **argv)
{
    FILE *hInput = NULL, *hOutput = NULL;
    struct stat StatBuf;
    char inputElfFile[256], inputBinFile[256], lzmaFile[256+5], outputFile[256], segment[16];
    char binpath[256], cmd[1024];
    unsigned char *data;
    char *encryptCmd = NULL;
    bool status;
    uint32_t  outLen, inLen;
    int cmpKernel = 0;
    Elf32_Addr entryPoint;
    Elf32_Addr textAddr;
    unsigned lzma_algo;
    unsigned lzma_dictsize;
    unsigned lzma_fastbytes;
    int lzma_compression_level; 
    int ret;
    char *pgmName = argv[0];
    int i;
    int lz4 = 0;

    for(i=strlen(argv[0])-1; i>=0; i--) {
        if (argv[0][i] == '/') {
            break;
        }
    }
    if (i >= 0) {
        strncpy(binpath, argv[0], i+1);
        binpath[i+1] = 0;
    }
    else {
        strcpy(binpath, "./");
    }

    if (argc == 8 && strcmp(argv[1], "-t") == 0)
    {
        strcpy(segment, ".text");
        argc--;
        argv++;
    }
    else if (argc == 8 && strcmp(argv[1], "-s") == 0)
    {
        strcpy(segment, ".sdata");
        argc--;
        argv++;
    }
    else
        strcpy(segment, ".data");

    if (argc < 6)
        usage(pgmName);

    if (strcmp(argv[1], "-k") == 0) 
        cmpKernel = 1;
    else if (strcmp(argv[1], "-c") != 0)
        usage(pgmName);
    
    if (strcmp(argv[2], "-0") == 0)
        lzma_compression_level = 0;
    else if (strcmp(argv[2], "-1") == 0)
        lzma_compression_level = 1;
    else if (strcmp(argv[2], "-2") == 0)
        lzma_compression_level = 2;
    else if (strcmp(argv[2], "-3") == 0)
        lzma_compression_level = 3;
    else
        usage(pgmName);

    if (strcmp(argv[3], "-lz4") == 0)
    {
       printf("using LZ4-HC compression\n");
       lz4 = 1;
    }
    else if ((strcmp(argv[3], "-lzma") == 0)) {
       printf("using LZMA compression\n");
       lz4 = 0;
    }
    else
       usage(pgmName);

    strcpy(inputElfFile, argv[4]);
    strcpy(inputBinFile, argv[5]);
    strcpy(outputFile, argv[6]);

    if (argc >= 9 && !strcmp(argv[7],"-e")) {
	encryptCmd = (char*)strdup(argv[8]);
        if (!encryptCmd) {
	    return -1; 
        }
    }
    sprintf(lzmaFile, "%s.%s", inputBinFile,lz4 ? "lz4":"lzma");

    if ((ret = getElfInfo(inputElfFile, &entryPoint, &textAddr)) != 0)
        return -1;

    printf("Code text starts: textAddr=0x%08X  Program entry point: 0x%08X,\n",
    	        (unsigned int)(BCM_HOST_TO_TARGET32(textAddr)), (unsigned int)(BCM_HOST_TO_TARGET32(entryPoint)));

    if (stat(inputBinFile, &StatBuf ))
    {
        printf( "Error opening input bin file %s.\n\n", inputBinFile);
        return -1;
    }
    inLen = StatBuf.st_size;

#if 0
    switch (lzma_compression_level)
    {
        case 1 :
            lzma_algo = 1;
            lzma_dictsize = 1 << 20;
            lzma_fastbytes = 64;
            break;
        case 2 :
            lzma_algo = 2;
            lzma_dictsize = 1 << 22;
            lzma_fastbytes = 128;
            break;
        case 3 :
            lzma_algo = 2;
            lzma_dictsize = 1 << 24;
            lzma_fastbytes = 255;
            break;
        default :
            printf("Invalid LZMA compression level.");
    }
#endif

    if (lzma_compression_level != 0)
    {
       // use standard lzma utlity to compress
       if ( lz4 ) 
       {
          sprintf(cmd, "%slz4cmp.out  %s %s", binpath, inputBinFile,lzmaFile);
       }
       else
       {
          sprintf(cmd, "%slzmacmd e %s %s -d22 -lp2 -lc1", binpath, inputBinFile, lzmaFile);
       }
       printf("%s\n", cmd);
       status = system(cmd);
       if (status == -1) 
       {
          /* this should NEVER happen */
          printf("LZMA compression failed.\n");
          return false;
       }
	
       if (encryptCmd) {
          status = system(encryptCmd);
          if (status == -1) {
             /* this should NEVER happen */
              printf("external encrypt failed. %s\n",encryptCmd);
              return false;
          }
       }

       if (stat(lzmaFile, &StatBuf ) == 0 && (hInput = fopen(lzmaFile, "rb" )) == NULL)
       {
          printf( "Error opening compressed file %s.\n\n", lzmaFile);
          return -1;
       }
    }
    else
    {
       if (stat(inputBinFile, &StatBuf ) == 0 && (hInput = fopen(inputBinFile, "rb" )) == NULL)
       {
          printf( "Error opening compressed, encrypted file %s.\n\n", inputBinFile);
          return -1;
       }
    }
    outLen = StatBuf.st_size;

    data = (unsigned char *) malloc(outLen);

    if (!data)
    {
        printf( "Memory allocation errorn\n");
        fclose( hInput );
        return -1;
    }

    if (fread(data, sizeof(char), StatBuf.st_size, hInput) != StatBuf.st_size)
    {
        printf( "Error read input file %s hInput=0x%p.\n\n", inputBinFile, hInput);
        return -1;
    }

    /* Open output file. */
    if ((hOutput = fopen(outputFile, "w+" )) == NULL)
    {
        printf ("Error opening output file %s.\n\n", outputFile);
        return -1;
    }

    if (cmpKernel)
    {
        uint32_t  swapedOutLen;    //little Endian on build host and big Endia on target   
        uint32_t  swapedInLen;
        char      brcmSignature[4] = {'B','R','C','M'};
        uint32_t  brcmSigOut = (*(uint32_t*)brcmSignature);
            
       	swapedOutLen = BCM_HOST_TO_TARGET32(outLen);
        swapedInLen = lz4 ? BCM_HOST_TO_TARGET32(inLen):0;
        if (fwrite(&textAddr, sizeof(Elf32_Addr), 1, hOutput) != 1 || 
            fwrite(&entryPoint, sizeof(Elf32_Addr), 1, hOutput) != 1 || 
            fwrite(&swapedOutLen, sizeof(uint32_t ), 1, hOutput) != 1 ||
            fwrite(brcmSignature, sizeof(brcmSignature ), 1, hOutput) != 1  ||
            fwrite(&swapedInLen, sizeof(uint32_t ), 1, hOutput) != 1  ||
            fwrite(data, sizeof(char), outLen, hOutput) != outLen)
            printf( "Error writing to output file.\n\n" );
    }
    else // write the asm file for CFE
    {
        struct tm *newtime;
        time_t aclock;
        int i;
        unsigned char *curPtr = data, *endPtr = data + outLen;
        unsigned char *entryPtr = (unsigned char*) &entryPoint;

        time( &aclock );                 /* Get time in seconds */
        newtime = localtime( &aclock );  /* Convert time to struct */
        fprintf(hOutput, "/* Convert binary to asm\n   Input file : %s\n   Output file: %s\n   Date       : %s*/\n"
            , inputBinFile, outputFile, asctime(newtime));
        fprintf(hOutput, "%s", "\t.globl _binArrayStart\n");
        fprintf(hOutput, "%s", "\t.globl _binArrayEnd\n");
        fprintf(hOutput, "\t%s\n\n", segment);
        fprintf(hOutput, "%s", "_binArrayStart:");
        // write 4 bytes of entry point first
        fprintf(hOutput, "%s%04o", "\n\t\t.byte ", (unsigned int) *entryPtr++);
        fprintf(hOutput, ",%04o", (unsigned int) *entryPtr++);
        fprintf(hOutput, ",%04o", (unsigned int) *entryPtr++);
        fprintf(hOutput, ",%04o", (unsigned int) *entryPtr);
        i = 4;
        for (; curPtr < endPtr; curPtr++)
        {   
            if (i == 0)
                fprintf(hOutput, "%s%04o", "\n\t\t.byte ", (unsigned int) *curPtr);
            else
                fprintf(hOutput, ",%04o", (unsigned int) *curPtr);
            if (++i == 16)
                i = 0;
        }
        fprintf(hOutput, "%s", "\n_binArrayEnd:\n");
    }

    fclose( hOutput );

    printf("Before compression: %d  After compression (level=%d): %d\n\r", inLen, lzma_compression_level, outLen);
    printf("Percent Compression = %.2f\n\r", (float)((float)(inLen - outLen)/(float)inLen)*(float)100);

    if(data) {
        free(data);
    }

    if (lzma_compression_level != 0)
    {
       sprintf(cmd, "rm %s", lzmaFile);
       status = system(cmd);
    }

    return(0);
}


