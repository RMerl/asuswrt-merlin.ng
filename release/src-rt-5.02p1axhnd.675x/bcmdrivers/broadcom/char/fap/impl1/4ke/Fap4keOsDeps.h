/************************************************************
 *
 * <:copyright-BRCM:2012:DUAL/GPL:standard
 * 
 *    Copyright (c) 2012 Broadcom 
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
 * :>
 ************************************************************/

/* This file contains mirrors of Linux specific data types and structures */

#ifndef _FAP4KEOSDEPS_H_
#define _FAP4KEOSDEPS_H_

/* we're including one file from the kernel: the version file
   I'm purposely convoluting the include path though so that
   other people are not tempted to include other kernel files. */

/* This does mean that we have to build the kernel before
   we build this though... TBD: add makefile dependency */

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long s64;
typedef unsigned long u64;

#ifndef BUG
#define BUG(...)          do { } while(0)
#endif /* BUG */
void panic(char *s);

struct net_device {
  unsigned int dev;
};

#define IN
#define OUT

#define true 1
#define false 0

#define virt_to_page(val1) 0
#define SetPageReserved(val1)

#define PAGE_SIZE 1
#define PAGE_MASK 0

#define read_c0_count()     0

#define min_t(type, x, y) ({  \
  type __min1 = (x);  \
  type __min2 = (y);  \
  __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({  \
  type __max1 = (x);  \
  type __max2 = (y);  \
  __max1 > __max2 ? __max1: __max2; })

/* WARNING - TBD: look into this... this looks like it could be a problem!!!
   guessing at addresses for now...

   it is used in BCM_PKTDMA_PBUF_FROM_BD, in enetRecvTask.
   */
#define phys_to_virt(address)   ((unsigned long)address - 0x80000000UL + 0)

/* NOTE: these have been modified to match the kernel definitions of the 
   macros.  They no longer match the definitions in bcm_OS_deps.h */
#define CPHYSADDR(val1) ((unsigned long)(val1) & 0x1fffffff)
#define KSEG0ADDR(a)    (CPHYSADDR(a) | 0x80000000)
#define KSEG1ADDR(a)    (CPHYSADDR(a) | 0xa0000000)

#endif /* _FAP4KEOSDEPS_H_ */
