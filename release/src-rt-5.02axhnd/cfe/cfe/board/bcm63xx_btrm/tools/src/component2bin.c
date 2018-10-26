/**********************************************************************
 *  
 *  component2bin.c       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2017
 *  Broadcom Corporation. All rights reserved.
 *
 * <:label-BRCM:2012:NONE:standard
 *
 * :>
 *  
 ********************************************************************* 
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#define ASC2BIN(ch) (isdigit(ch) ? (ch) - '0' : toupper(ch) - 'A' + 10)

/* Look thru the input, look for the beginning of the desired component. Each      */
/* component will be 128 bytes with colons separating them (mixed in with carriage */
/* returns and spaces). Also, some components start with a 00 while others may not.*/
/* Leading zeros are needed in the DER representation of positive integers whose   */
/* most significant nibble is in the range from 8 to F.  Otherwise the leading bit */
/* would cause the integer to be interpreted as negative. Strip it out. The input  */
/* is text and therefore 128*2=256 chars will need to be read in                   */
int main(int argc, char* argv[])
{
    char    comp[16];
    int     len = 0, i = 0, fndCompBgn = 0, compCtr = 0;
    uint8_t ch, buffer[16*1024], data[256];

    /* Check that we weren't given any unexpected arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s < -p | -q | -dmp1 | -dmq1 | -iqmp >", argv[0]);
        return EXIT_FAILURE;
    }

    if (! strcmp(argv[1],"-p"))
    {
        strcpy(comp, "prime1:\n");
	len = 8;
    }
    else 
    {
       if (! strcmp(argv[1],"-q"))
       {
          strcpy(comp, "prime2:\n");
	  len = 8;
       }
       else 
       {
          if (! strcmp(argv[1],"-dmp1"))
          {
             strcpy(comp, "exponent1:\n");
	     len = 11;
          }
          else 
	  {
             if (! strcmp(argv[1],"-dmq1"))
             {
                strcpy(comp, "exponent2:\n");
                len = 11;
             }
             else 
	     { 
                if (! strcmp(argv[1],"-iqmp"))
                {
                   strcpy(comp, "coefficient:\n");
                   len = 13;
                }
                else
                {
                   fprintf(stderr, "Usage: %s < -p | -q | -dmp1 | -dmq1 | -iqmp >", argv[0]);
                   return EXIT_FAILURE;
                }
             }
          }
       }
    }

    while (((ch = getchar()) != EOF) && (compCtr < 256))
    {
       buffer[i] = ch;
       i++;

       if ((fndCompBgn) && (ch != ':') && (ch != '\n') && (ch != ' '))
       {
          /* Some components may start with a 00 while others may not. Leading    */
          /* zeros are needed in the DER representation of positive integers      */
          /* whose most significant nibble is in the range from 8 to F. Otherwise */
          /* the leading bit would cause the integer to be interpreted as negative*/
          /* We will strip this out.                                              */
          if ((compCtr == 1) && (ch == '0') && (data[0] == '0'))
          {
             compCtr = 0;
	  } 
	  else
          {
             data[compCtr] = ch;
             compCtr++;
          }
       }

       if ((fndCompBgn == 0) && (i >= len) && (! memcmp(&buffer[i - len], comp, len)))
          fndCompBgn = 1;
    }

    if (compCtr !=256)
    {
       fprintf(stderr, "The string \n %s was not located in the std input", comp);
       return EXIT_FAILURE;
    }
    else
    {
       /* Convert pairs of ascii hex characters to bytes and write them out */
       for (i = 0; i < sizeof(data); i += 2)
       {
          uint8_t const byte = ASC2BIN(data[i])*16 + ASC2BIN(data[i+1]);
          putchar(byte);
       }
    }

    return EXIT_SUCCESS;
}
