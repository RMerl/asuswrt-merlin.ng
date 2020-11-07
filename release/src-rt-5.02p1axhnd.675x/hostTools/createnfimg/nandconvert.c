/*
******************************************************************************
* Copyright 2018 Broadcom Inc.
******************************************************************************
<:label-BRCM:2018:proprietary:standard

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
*
* Description:
*   This program converts a serial NAND image to allow it to be booted with an
*   older bootrom. Mainly this is to allow a 4k page size image and serial NAND
*   to work properly. The boot rom is expecting 2k size pages so the CFEROM needs
*   to be in the 2k accessible part of a page. In addition once CFEROM starts
*   it will detect a 4k page size serial NAND so NVRAM, CFERAM and Linux need to remain
*   as 4k pages. In addition the CRC needs to be recalculated for this new image.
*
* Build:
*   gcc nandconvert.c -o nandconvert
*
* Usage examples:
*   nandconvert -b [bootrom page size] -n [image/device page size] -i [in file] -o [out file] -l [little endian]
*   nandconvert -b 2048 -n 4096 -i bcm963381GW_nand_cfeonly.256.w -o bcm963381GW_nand_cfeonly_new.256.w
*   nandconvert -i bcm963381GW_nand_cfeonly.256.w -o bcm963381GW_nand_cfeonly_new.256.w
*
******************************************************************************
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>


/* Variables that start with 'p_' are program parameters. */
int  p_bootrom = 2048; // boot rom page size
int  p_nand = 4096; // device/file page size
int  p_littlendian = 1; // little endian
char *p_outfile_name = 0;
const char *p_infile_name = 0;
char *pgm_name;


void usage()
{
   printf("\nUSAGE:\n");
   printf("  %s <options>\n", pgm_name);
   printf("\nDESCRIPTION:\n");
   printf("  %s takes a 4K page serial NAND input file and converts it to be compatible with a boot rom that addresses serial NAND using 2K bytes/page\n", pgm_name);
   printf("\nOPTIONS:\n");
   printf("    -i <input file name>\n");
   printf("    -o <output file name>\n");
   printf("    -b <bootrom page size> -- page size boot rom expects, default = %d.\n", p_bootrom);
   printf("    -n <target page size> -- serial NAND page size, default = %d.\n", p_nand);
   printf("    -l <little endian> -- endianess of CRC, 0 for MIPS (63381/6838), 1 for ARM (63138/6848/6858 and newer chips.) default = %d.\n", p_littlendian);
   printf("\n");
   exit(1);
}


unsigned int getCrc32(char *pdata, unsigned int size, unsigned int crc)
{
   static unsigned int local_crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

   while (size-- > 0)
      crc = (crc >> 8) ^ local_crc32_table[(crc ^ *pdata++) & 0xff];

   return crc;
}


int main(int argc, char **argv)
{
   int       page = 0;   
   int       optchar;

   FILE      *infile;
   FILE      *outfile;
   char      *pch;
   int       bytes_read;

   pgm_name = strrchr(argv[0], '/');
   pgm_name = pgm_name ? pgm_name + 1 : argv[0];

   if (argc < 2) usage();

   /* Parse options */
   opterr = 0;
   while (-1 != (optchar = getopt(argc, argv, "b:B:i:I:l:L:n:N:o:O")))
   {
      switch (optchar)
      {
         case 'b':
         case 'B':
            p_bootrom = atoi(optarg);
            break;

         case 'i':
         case 'I':
            p_infile_name = optarg;
            break;

         case 'l':
         case 'L':
            p_littlendian = atoi(optarg);
            break;

         case 'n':
         case 'N':
            p_nand = atoi(optarg);
            break;

         case 'o':
         case 'O':
            p_outfile_name = optarg;
            break;

         default:
            usage();
      }

   }

   if (p_bootrom != 2048)
   {
      fprintf(stderr, "%s: For now can only handle 2048 target page size.\n", pgm_name);
      exit(1);
   }

   if ((p_nand % 2048) != 0)
   {
      fprintf(stderr, "%s: Native page size must be a multiple of target page size.\n", pgm_name);
      exit(1);
   }

   if (! p_infile_name)
   {
      fprintf(stderr, "%s: No infile name given!\n", pgm_name);
      exit(1);
   }

   if (! p_outfile_name)
   {
      fprintf(stderr, "%s: No outfile name given!\n", pgm_name);
      exit(1);
   }

   infile  = fopen(p_infile_name, "rb");
   if (infile == NULL) 
   {
      fprintf(stderr, "%s: Could not open input file '%s'.\n", pgm_name, p_infile_name);
      exit(1);
   }

   if (! p_outfile_name)
   { /* Open output out file (name = <input_file>.out) */
      int i;

      i = strlen(p_infile_name);
      p_outfile_name = (char *)malloc(i + 16);
      strcpy(p_outfile_name, p_infile_name);
      if (pch = strchr(p_outfile_name, '.')) {
         *pch = 0;
    
      strcat(p_outfile_name, ".out");
      }
   }

   outfile = fopen(p_outfile_name, "wb");
   if (outfile == NULL) 
   {
      fprintf(stderr, "%s: Could not open output file '%s' for writing.\n", pgm_name, p_outfile_name);
      fclose (infile);
      exit(1);
   }

   unsigned char data[p_nand];
   int boot_block = 1;
   int done = 0;
   int pages = 0;
   int position = 0;
   unsigned int crc = -1;

   while (!done)
   {
printf("boot_block %d, position 0x%x\n", boot_block, position);
      if (!boot_block)
      { // write CFERAM, Linux blocks normally
         fseek(infile, position, SEEK_SET);
         bytes_read = fread(data, 1, p_nand, infile);

         if (bytes_read != p_nand)
         { // we are at the end of the file
            if (bytes_read) // write trailer
            { // might need to fix for endianess
               if (p_littlendian)
               {
                  data[3] = crc >> 24 & 0xFF;
                  data[2] = crc >> 16 & 0xFF;
                  data[1] = crc >> 8  & 0xFF;
                  data[0] = crc       & 0xFF;
               }
               else
               {
                  data[0] = crc >> 24 & 0xFF;
                  data[1] = crc >> 16 & 0xFF;
                  data[2] = crc >> 8  & 0xFF;
                  data[3] = crc       & 0xFF;
               }
               printf("crc 0x%8x\n", crc);
            }
            done = 1;
         }
         else
            position += p_nand;
      }
      else
      { // write modified CFEROM
         fseek(infile, position * (p_nand / p_bootrom), SEEK_SET);
         bytes_read = fread(data, 1, 3, infile);

         if ( ((data[0] == 0x19) && (data[1] == 0x85)) || ((data[1] == 0x19) && (data[0] == 0x85)) || ((data[0] == 'U') && (data[1] == 'B') && (data[2] == 'I')) )
         { // we've reached the end of CFEROM
            position = position * (p_nand / p_bootrom);
            boot_block = 0;
            continue;
         }

         fseek(infile, position, SEEK_SET);
         if (position)
         {
            memset(data + p_bootrom, 0xFF, p_nand - p_bootrom);
            bytes_read = fread(data, 1, p_bootrom, infile);
            bytes_read = p_nand;
         }
         else
            bytes_read = fread(data, 1, p_nand, infile);

         position += p_bootrom;
      }

      fwrite(data, bytes_read, 1, outfile);
      crc = getCrc32(data, bytes_read, crc);
   }
      
EXIT:
    fclose(infile);
    fclose(outfile);    
} /* end of main */

