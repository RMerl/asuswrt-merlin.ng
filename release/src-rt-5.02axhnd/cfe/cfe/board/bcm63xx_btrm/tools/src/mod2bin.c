/**********************************************************************
 *  
 *  mod2bin.c       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
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

/** Extract the public key modulus from the output of the OpenSSL rsa command
 * 
 * When -modulus is specified as an argument to the OpenSSL rsa command, the
 * modulus of the key pair is output as a line in the format
 * 
 * Modulus=ab4d83....f4e3
 *
 * 
 * This filter reads this line from standard input and writes the binary 
 * equivalent of the modulus bytes to standard output. The command will return
 * EXIT_FAILURE if
 * 
 * a) The modulus does not specify 256 bytes (that is, it is not a 2048 bit key)
 * b) The line did not start with the expected 'Modulus='
 * c) Unexpected arguments were specified on the command line
 * 
 * @param argc  the number of command line arguments
 * @param argv  the command line arguments
 * @return      EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char* argv[])
{
    /* Check that we weren't given any unexpected arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s <modulus-text >modulus-binary\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Read in the number of bytes we were expecting - 8 for the 'Modulus=' text
     * and 512 for 256 pairs of hex digits
     */
    uint8_t buffer[8+(256*2)];
    int i;
    for (i = 0; i < sizeof(buffer); ++i)
    {
        int const ch = getchar();
        if (ch == EOF)
        {
            fprintf(stderr, "Unexpected end of modulus after only %d characters\n", i);
            return EXIT_FAILURE;
        }
        buffer[i] = ch;
    }

    /* Make sure there are no more bytes on the line (other than a new-line) */
    if (getchar() != '\n' || getchar() != EOF)
    {
        fprintf(stderr, "Unexpected text after end of modulus\n");
        return EXIT_FAILURE;
    }

    /* Make sure the line starts as expected */
    if (memcmp(buffer, "Modulus=", 8) != 0)
    {
        buffer[8] = 0;
        fprintf(stderr, "Buffer expected to start with 'Modulus=' but '%s' found\n", buffer);
        return EXIT_FAILURE;
    }
   
    /* Convert pairs of ascii hex characters to bytes and write them out */
    for (i = 8; i < sizeof(buffer); i += 2)
    {
        uint8_t const byte = ASC2BIN(buffer[i])*16 + ASC2BIN(buffer[i+1]);
        putchar(byte);
    }

    return EXIT_SUCCESS;
}
