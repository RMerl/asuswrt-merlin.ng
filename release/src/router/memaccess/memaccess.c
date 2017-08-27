/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
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
*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

#define swap2bytes(a)           ( ( ( (a) & 0xFF ) << 8 ) | ( ( (a) & 0xFF00 ) >> 8 ) )
#define swap4bytes(a)           ( ( ( (a) & 0xFF ) << 24 ) | ( ( (a) & 0xFF00 ) << 8 ) | ( ( (a) & 0xFF0000 ) >> 8 ) | ( ( (a) & 0xFF000000 ) >> 24 ) )
#define condswap2(sw,a)  ( (sw) ? swap2bytes((a)) : (a) )
#define condswap4(sw,a)  ( (sw) ? swap4bytes((a)) : (a) )


// format-rule : uncrustify bcm_minimal_i4


#define NUM_BYTES_PER_LINE 16
#define MAX_DATA_ALLOWED 64

typedef unsigned int word;
typedef unsigned short halfword;
typedef unsigned char byte;

typedef int bool;
#define true 1
#define false 0


// Globals
long pageSize;

void printUsage(char *cmd)
{
    printf("\n*** Usage:\n"
        " dw/dh/db <physical address in hex> <number> \n"
        " dw/dh/db <-k> <virtual address in hex> <number> \n"
        " sw/sh/sb <physical address in hex> <data value1> <data value2> ..<data valueN> \n"
        " sw/sh/sb <-k> <virtual address in hex> <data value1> <data value2> ..<data valueN> \n"
        " fw/fh/fb  <physical address in hex> <data value> <length> \n"
        " fw/fh/fb <-k> <virtual address in hex> <data value> <length> \n"
        "  -s (currently works with physical addresses for d*/s*/f* commands\n"
        "and virtual addresses for s*/f* commands)\n"
        "\n");
}

void exitWithUsage(char *cmd)
{
    printUsage(cmd);
    exit(0);
}



int main(int argc, char *argv[])
{

    int opt,num,swap,kernelspace;
    char mode = 0;
    BOARD_MEMACCESS_IOCTL_PARMS parms;
    int i;
    unsigned long long int addr = 0;
    unsigned long data;
    int fd;
    char **endptr = NULL;
    char *cmd;
    char *buf;
    int siz = 0;
    int long_index = 0;

    opt = 0;
    num = 1;
    swap = 0;
    kernelspace = 0;

    static struct option long_options[] = {

        { "kernel", no_argument , 0, 'k' },
        { "swap", no_argument   , 0, 's' },
        {0, 0, 0, 0 }

    };

    cmd = strrchr(argv[0], '/');
    if(cmd == NULL)
    {
        cmd = argv[0];
    }
    else
    {
        cmd++;
    }


    if(argc < 2)
    {
        exitWithUsage(cmd);
    }


    mode = cmd[0];
    if (cmd[1] == 'w') {
        siz = 4;
    } else if (cmd[1] == 'h') {
        siz = 2;
    } else if (cmd[1] == 'b') {
        siz = 1;
    } else {
        exitWithUsage(cmd);
    }

    while ((opt = getopt_long(argc, argv, "sk", long_options, &long_index)) != -1)
    {

        switch (opt) {

        case 'k' :
            kernelspace = 1;
            break;
        case 's' :
            swap = 1;
            break;
        default :
            printf("Error: Invalid switch\n");
            exitWithUsage(cmd);
        }
    }

    if(mode  == 0)
    {
        printf ("Error: Invalid operation. \n");
        exitWithUsage(cmd);
    }

    addr = strtoull(argv[optind], endptr, 16);
    if(addr == 0)
    {
        printf("Error: Address %s is invalid\n", argv[optind]);
        exitWithUsage(cmd);
    }
    optind++;

#if defined (__mips__)
    if ( (kernelspace == 1) && (((addr & 0xF0000000) != (0xB0000000)) && ((addr & 0xF0000000) != (0x80000000))) )
    {
        printf("when using -k (kernel) switch, address should start with 0xb or 0x8\n");
        exitWithUsage(cmd);
    }

    if ( (kernelspace == 0 ) && (((addr & 0xF0000000) == (0xB0000000)) || ((addr & 0xF0000000) == (0x80000000))) )

    {
        printf("when using address with 0xb or 0x8, kernel switch (-k) is mandatory\n");
        exitWithUsage(cmd);
    }
#endif


    if(mode == 'd')
    {
        if(optind < argc)
        {
            num = atoi(argv[optind]);
        }
    } else {
        num = argc - optind ;
    }
    buf = malloc(num * siz);

    if(mode == 's')
    {
        if(optind == argc)
        {
            printf("Error: Missing data to write\n");
            exitWithUsage(cmd);
        }

        i = 0;
        while(optind < argc)
        {
            data = strtoul(argv[optind++], NULL, 16);
            if (siz == 1) {
                *(unsigned char *)(&buf[i]) = data;
            }
            else if (siz == 2) {
                *(unsigned short *)(&buf[i*2]) = data;
            }
            else if (siz == 4) {
                *(unsigned long *)(&buf[i*4]) = data;
            }
            i++;

        }
    }


    if(mode == 'f')
    {
        if(optind + 2 != argc)
        {
            printf("Error: wrong number of args\n");
            exitWithUsage(cmd);
        }

        data = strtoul(argv[optind++], NULL, 16);
        if (siz == 1) {
            *(unsigned char *)buf = data;
        }
        else if (siz == 2) {
            *(unsigned short *)buf = data;
        }
        else if (siz == 4) {
            *(unsigned long *)buf = data;
        }
        num  = strtoul(argv[optind++], NULL, 16);

    }




    if ((fd = open (BOARD_DEVICE_NAME, O_RDWR)) < 0)
    {
        printf ("Can't open /dev/brcmboard ");
    }

    parms.address = addr;
    parms.size = siz;
    if (kernelspace == 0) {
        parms.space = BOARD_MEMACCESS_IOCTL_SPACE_REG;
    } else {
        parms.space = BOARD_MEMACCESS_IOCTL_SPACE_KERN;
    }
    parms.count = num;
    parms.buf = buf;
    if  (mode == 's') {
        parms.op = BOARD_MEMACCESS_IOCTL_OP_WRITE;
    } else if  (mode == 'd') {
        parms.op = BOARD_MEMACCESS_IOCTL_OP_READ;
    } else if  (mode == 'f') {
        parms.op = BOARD_MEMACCESS_IOCTL_OP_FILL;
    }
    ioctl(fd, BOARD_IOCTL_MEM_ACCESS, &parms);

    if  (mode == 'd') {
        for (i = 0 ; i < num ; i++) {
            if (((i * siz) % NUM_BYTES_PER_LINE) == 0) {
                printf("\n0x%X : ", (int)addr + i * siz);
            }
            switch (siz) {
            case 1 :
                printf("%02x ",*(unsigned char *)(&buf[i]));
                break;
            case 2 :
                printf("%04x ",condswap2(swap,*(unsigned short *)(&buf[i*2])));
                break;
            case 4 :
                printf("%08x ",condswap4(swap,*(unsigned int *)(&buf[i*4])));
                break;
            }
        }
        printf("\n");
    }

    free(buf);
    close(fd);

    return 0;
}


