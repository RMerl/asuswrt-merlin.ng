/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

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
