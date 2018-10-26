/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  BSP Configuration file			File: bsp_config.h
    *
    *  This module contains global parameters and conditional
    *  compilation settings for building CFE.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *
    *********************************************************************
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */
#include "bcm_cpu.h"

#if defined(CP0_CMT_TPID)
#define CFG_CMT         1
#else
#define CFG_CMT         0
#endif

#define CFG_INIT_L1             1      /* initialize the L1 cache */
#define CFG_INIT_L2             0      /* there is no L2 cache */

#define CFG_INIT_DRAM           1      /* initialize DRAM controller */
#define CFG_DRAM_SIZE           xxx    /* size of DRAM if you don't initialize */
                                       /* NOTE : Size is in kilobytes. */

#define CFG_NETWORK             1      /* define to include network support */

#define CFG_FATFS               0
#define CFG_UI                  1      /* Define to enable user interface */

#define CFG_MULTI_CPUS          0      /* no multi-cpu support */

#if defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM96858_) \
|| defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM94908_) || defined(_BCM963158_)
#define CFG_HEAP_SIZE           (30*1024)   /* heap size in kilobytes */
#else
#define CFG_HEAP_SIZE           1024   /* heap size in kilobytes */
#endif

/* some cfe rom run in LMEM and only has 512KB space.  Heap for cfe
rom is used for LZMA decompression scratch pad(NOR boot) or NAND block
buf(NAND boot). 256KB is big enough for now. */
#if defined (_BCM963381_) || defined (_BCM963138_) || defined (_BCM963148_)
#if defined (INC_DDR_DIAGS)
/* DDR diags require much more code and data size. Only for SPI boot for now */
#define CFG_ROM_HEAP_SIZE       128
#else
#define CFG_ROM_HEAP_SIZE       256    
#endif
#elif defined (_BCM94908_) 
#if defined (INC_DDR_DIAGS)
#define CFG_ROM_HEAP_SIZE       16
#else
#define CFG_ROM_HEAP_SIZE       64
#endif
#elif defined (_BCM96858_) || defined (_BCM963158_) || defined (_BCM96856_)
#define CFG_ROM_HEAP_SIZE       64
#else
#define CFG_ROM_HEAP_SIZE       CFG_HEAP_SIZE 
#endif

#define CFG_STACK_WARN_TRESHOLD 4      /* number of address width words required beform the bottom of stack 
                                          after which warning may be issued */
#if defined(_BCM96858_) || defined(_BCM96846_) || defined (_BCM96856_)
#define CFG_STACK_SIZE          (24*1024)   /* stack size (bytes, rounded up to K) */
#elif defined(_BCM94908_) && defined(INC_DDR_DIAGS)
#define CFG_STACK_SIZE          (44*1024)   /* stack size (bytes, rounded up to K) */
#elif defined(_BCM963158_) && defined(INC_DDR_DIAGS)
#define CFG_STACK_SIZE          (128*1024)   /* stack size (bytes, rounded up to K) */
#elif (defined(_BCM963138_) || defined(_BCM963148_)) && defined(INC_DDR_DIAGS)
#define CFG_STACK_SIZE          (128*1024)   /* stack size (bytes, rounded up to K) */
#else
#define CFG_STACK_SIZE          8192   /* stack size (bytes, rounded up to K) */
#endif

#ifndef IKOS_FAST_UART
#define CFG_SERIAL_BAUD_RATE	115200	/* normal console speed */
#else
#define CFG_SERIAL_BAUD_RATE	1562500 /* maximum speed */
#endif
#define CFG_VENDOR_EXTENSIONS   0
#define CFG_MINIMAL_SIZE        1

#if defined(CONFIG_ARM64)
#define CFG_DTB_ADDRESS         0x00f00000
#define CFG_DTB_MAX_SIZE        0x00010000
#define CFG_PEN_ADDRESS         0x00001000
/* RAM based bootup address and size
   0x00000000:0x00010000 to boot secondary ARM CPU
   0x00010000:0x00020000 to boot PMC core

   NOTE: Please make sure Linux device tree reserves
         same amount of memory using memreserve node.
         Otherwise Linux can overwrite this space.
*/
#define CFG_BOOT_PMC_SIZE       0x10000
#define CFG_BOOT_PMC_ADDR       0x10000
#define CFG_BOOT_AREA_SIZE      0x10000
#define CFG_BOOT_AREA_ADDR      0

#if CFG_BOOT_AREA_ADDR + CFG_BOOT_AREA_SIZE > CFG_BOOT_PMC_ADDR && \
 CFG_BOOT_AREA_ADDR + CFG_BOOT_AREA_SIZE < CFG_BOOT_PMC_ADDR + CFG_BOOT_PMC_SIZE
#error secondary cpu bootstrap and pmc firmware areas overlap
#endif
#elif defined(CONFIG_ARM)
/*  0x00001000 - 0x00008000 secondary boot memory region
    0x00008000 - 0x00010000 DTB
    0x00010000...           linux kernel, entry point at 0x00018000   */
#define CFG_BOOT_AREA_ADDR      0x1000
#define CFG_BOOT_AREA_SIZE      (0x8000 - CFG_BOOT_AREA_ADDR)
#define CFG_DTB_ADDRESS         0x8000
#define CFG_DTB_MAX_SIZE        0x8000

#define ARM_ATAG_LOC            CFG_DTB_ADDRESS
#else
#define CFG_DTB_ADDRESS        ((unsigned char *) (0x80010000-0x8000))
#define CFG_DTB_MAX_SIZE        0x0003000
#endif

/*
 * These parameters control the flash driver's sector buffer.  
 * If you write environment variables or make small changes to
 * flash sectors from user applications, you
 * need to have the heap big enough to store a temporary sector
 * for merging in small changes to flash sectors, so you
 * should set CFG_FLASH_ALLOC_SECTOR_BUFFER in that case.
 * Otherwise, you can provide an address in unallocated memory
 * of where to place the sector buffer.
 */

#define CFG_FLASH_ALLOC_SECTOR_BUFFER 0	/* '1' to allocate sector buffer from the heap */
#define CFG_FLASH_SECTOR_BUFFER_ADDR  (1*1024*1024-128*1024) /* 1MB - 128K */
#define CFG_FLASH_SECTOR_BUFFER_SIZE  (128*1024)

/*
 * The flash staging buffer is where we store a flash image before we write
 * it to the flash.  It's too big for the heap.
 */

#define CFG_FLASH_STAGING_BUFFER_ADDR (1*1024*1024)
#define CFG_FLASH_STAGING_BUFFER_SIZE (1*1024*1024)
