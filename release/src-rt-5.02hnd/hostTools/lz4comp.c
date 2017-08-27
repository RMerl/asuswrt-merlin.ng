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
// File Name  : lzcomp.c
//
// Description: Compress file with lz4 codec

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lz4hc.h"


void usage(char *appname)
{
   printf("should run with# %s Inputfile Outputfile\n",appname);
}

int main(int argc, char **argv)
{
   FILE *hInput = NULL, *hOutput = NULL;
   unsigned char *inmem = NULL;
   unsigned char *outmem = NULL;
   struct stat StatBuf = {};
   int rc = 0;

   if (argc < 3) {
      usage(argv[0]);
      goto clean;
   }

   printf("infile %s \noutfile %s \n",argv[1],argv[2]);

   if (stat(argv[1],&StatBuf) != 0) {
      printf( "Error stating uncompressed file %s.\n\n", argv[1]);
      return -1;
   }

   if (StatBuf.st_size == 0) {
      printf( "size of source file %s is zero\n", argv[1]);
      return -1;
   }

   /*open files*/
   if ((hInput = fopen(argv[1], "rb" )) == NULL)
   {
      printf( "Error opening compressed file %s.\n\n", argv[1]);
      goto clean ;
   }
 
   inmem = (unsigned char*)malloc(StatBuf.st_size);
   outmem = (unsigned char*)malloc(StatBuf.st_size);

   if (fread(inmem, sizeof(char), StatBuf.st_size, hInput) != StatBuf.st_size)
   {
     printf( "Error read input file %s.\n\n", argv[1]);
     goto clean;
   }

   if ((hOutput = fopen(argv[2], "w+" )) == NULL)
   {
     printf ("Error opening output file %s.\n\n", argv[2]);
     goto clean;
   }

   rc = LZ4_compressHC((const char*)inmem, (char *)outmem, StatBuf.st_size);
   if (rc == 0) {
      printf ("Error opening output file %s.\n\n", argv[2]);
      goto clean;
   }

   if( fwrite(outmem, sizeof(char), rc, hOutput) != rc)
   {
      printf ("Error writing output file %s,rc=%d.\n\n", argv[2],rc);
   }
   printf("Compressed with ratio of %f%\n",100*((float)rc/(float)StatBuf.st_size));

clean:
   if (inmem) {
      free(inmem);
   }
   if (outmem) {
      free(outmem);
   }

   return 0;
}
