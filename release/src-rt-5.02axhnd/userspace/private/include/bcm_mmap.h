/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 ************************************************************************/

/************************************************************************
 * Allows mapping physical memory, including chip registers, to userspace
 * Sample usage:
 * 
 * #include "bcm_mmap.h"
 * #include "bcm_map.h"
 * 
 * int main(int argc, char **argv)
 * {
 *    BCM_MMAP_INFO bcm_mmap_info;
 * 
 *    bcm_mmap_regs(bcm_mmap_info);
 *    printf("PERF->RevID %08lX\n", PERF->RevID);
 *    bcm_munmap_regs(bcm_mmap_info);
 * }
 ************************************************************************/
#ifndef _BCM_MMAP_H_
#define _BCM_MMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include "bcmtypes.h"

#if !defined(BCM_MMAP_INFO_BASE2)
#define BCM_MMAP_INFO_BASE2 0
#define BCM_MMAP_INFO_SIZE2 0
#endif

typedef struct
{
    int mem_fd;
    char *mmap_addr;
    unsigned long addr;
    unsigned int size;
    unsigned int offset;
    char *mmap_addr2;
    unsigned long addr2;
    unsigned int size2;
    unsigned int offset2;
} BCM_MMAP_INFO;

/***************************************************************************
 * Function Name: bcm_mmap_regs
 * Description  : Maps registers in kernel memory to user space.
 * Parameters   : BCM_MMAP_INFO mi
 ***************************************************************************/
#define bcm_mmap_regs(mi) bcm_mmap(BCM_MMAP_INFO_BASE, BCM_MMAP_INFO_SIZE,  \
                             BCM_MMAP_INFO_BASE2, BCM_MMAP_INFO_SIZE2, mi)

/***************************************************************************
 * Function Name: bcm_munmap_regs
 * Description  : Unmaps registers in kernel memory from user space.
 * Parameters   : BCM_MMAP_INFO mi
 ***************************************************************************/
#define bcm_munmap_regs(mi) bcm_munmap(mi)

/***************************************************************************
 * Function Name: bcm_mmap
 * Description  : Maps kernel memory to user space.
 * Parameters   : unsigned long addr, int size, BCM_MMAP_INFO mi
 ***************************************************************************/
#define bcm_mmap(inaddr, insize, inaddr2, insize2, mi)                      \
do {                                                                        \
    unsigned int pagesize = getpagesize();                                  \
    unsigned long addr = inaddr;                                            \
    unsigned long addr2 = inaddr2;                                          \
                                                                            \
    memset(&mi, 0, sizeof(BCM_MMAP_INFO));                                  \
    mi.mem_fd = open("/dev/mem", O_RDWR | O_SYNC);                          \
    if (mi.mem_fd < 0)                                                      \
    {                                                                       \
        fprintf(stderr, "Device open on /dev/mem failed: %d\n", errno);     \
        break;                                                              \
    }                                                                       \
                                                                            \
    /* handle page alignment */                                             \
    mi.addr = addr & ~(pagesize - 1);                                       \
    mi.offset = addr & (pagesize - 1);                                      \
    mi.size = (insize + mi.offset + pagesize-1) & ~(pagesize-1);            \
                                                                            \
    mi.mmap_addr = (char *)mmap(0, mi.size, PROT_READ | PROT_WRITE,         \
        MAP_SHARED, mi.mem_fd, mi.addr);                                    \
                                                                            \
    if ((int)mi.mmap_addr == -1)                                            \
    {                                                                       \
        fprintf(stderr, "mmap failed\n");                                   \
        close(mi.mem_fd);                                                   \
        break;                                                              \
    }                                                                       \
                                                                            \
    mi.mmap_addr += mi.offset;                                              \
                                                                            \
    /* Repeat this for a second window, if specified */                     \
    if (insize2)                                                            \
    {                                                                       \
        /* handle page alignment */                                         \
        mi.addr2 = addr2 & ~(pagesize - 1);                                 \
        mi.offset2 = addr2 & (pagesize - 1);                                \
        mi.size2 = (insize2 + mi.offset2 + pagesize-1) & ~(pagesize-1);     \
                                                                            \
        mi.mmap_addr2 = (char *)mmap(0, mi.size2, PROT_READ | PROT_WRITE,   \
            MAP_SHARED, mi.mem_fd, mi.addr2);                               \
                                                                            \
        if ((int)mi.mmap_addr2 == -1)                                       \
        {                                                                   \
            fprintf(stderr, "mmap failed\n");                               \
            close(mi.mem_fd);                                               \
            break;                                                          \
        }                                                                   \
                                                                            \
        mi.mmap_addr2 += mi.offset2;                                        \
    }                                                                       \
} while(0);

/***************************************************************************
 * Function Name: bcm_munmap
 * Description  : Unmaps kernel memory from user space.
 * Parameters   : BCM_MMAP_INFO mi
 ***************************************************************************/
#define bcm_munmap(mi)                                                      \
{                                                                           \
    mi.mmap_addr -= mi.offset;                                              \
    if (munmap(mi.mmap_addr, mi.size) == -1 ||                              \
        close(mi.mem_fd) == -1)                                             \
    {                                                                       \
        fprintf(stderr, "munmap failed\n");                                 \
    }                                                                       \
}

#endif
