/******************************************************************************
 *                Copyright (c) 2010 Broadcom Corporation                     *
 *                                                                            *
 *      This material is the confidential trade secret and proprietary        *
 *      information of Broadcom Corporation. It may not be reproduced,        *
 *      used, sold or transferred to any third party without the prior        *
 *      written consent of Broadcom Corporation. All rights reserved.         *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    uint8_t buffer[18];
    uint8_t buf[18];
    uint64_t n;
    int i,j,k;

    buf[0] = '0';
    buf[1] = '0';
    buf[2] = '0';
    buf[3] = '0';
    buf[4] = '0';
    buf[5] = '0';
    buf[6] = '0';
    buf[7] = '0';
    buf[8] = '0';
    buf[9] = '0';
    buf[10] = '0';
    buf[11] = '0';
    buf[12] = '0';
    buf[13] = '0';
    buf[14] = '0';
    buf[15] = '0';
    buf[16] = '0';
    buf[17] = '\0';



    for (j = 0; j < sizeof(buf); ++j)
    {
        int const ch = getchar();
        if (ch == EOF)
        {
           buf[j] = '\0';
	   break;
        }
        buf[j] = ch;
    }




    for (k = 0; k < (18 - j); k++)
       buffer[k]='0'; 




    j = 0;
    for (i = k; i < sizeof(buffer); ++i)
    {
       int const ch = buf[j];
       j++;
       buffer[i] = ch;
    }




    buffer[i] = '\0';
    buffer[17] = '\0';

    sscanf(buffer, "%d", &n);

    // if arm, no htonl is performed 
    // if mips, do the htonl
    if ((argc > 0) && (! strcmp(argv[1],"mips")))
       n = htonl(n);

    FILE *fd = fopen("./size.tmp","wb");
    fwrite(&n, sizeof(n), 1,fd);
    fclose(fd);

    return EXIT_SUCCESS;
}
