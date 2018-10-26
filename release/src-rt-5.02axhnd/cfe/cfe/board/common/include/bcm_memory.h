/*
    Copyright 2000-2016 Broadcom Corporation

    <:label-BRCM:2016:DUAL/GPL:standard
    
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
*/                       

/**************************************************************************
 * File Name  : bcm_memory.h
 *
 * Description: Common memory mappings macros and related primitives 
 * General memory layout 
              
 *
 * Updates    : 01/19/2016  Created.
 ***************************************************************************/

#ifndef _BCM_MEMORY_H

#define _BCM_MEMORY_H

#include "bcm_map_part.h"
#include "lib_malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SZ_256 ((unsigned long)0x1<<8)
#define SZ_512  (SZ_256<<1)
#define SZ_1K   (SZ_512<<1)
#define SZ_4K   (SZ_1K<<2)
#define SZ_8K   (SZ_4K<<1)
#define SZ_16K  (SZ_8K<<1)
#define SZ_32K  (SZ_16K<<1)
#define SZ_64K  (SZ_32K<<1)
#define SZ_128K (SZ_64K<<1)
#define SZ_256K (SZ_128K<<1)
#define SZ_512K (SZ_256K<<1)
#define SZ_1M   (SZ_512K<<1)
#define SZ_2M   (SZ_1M << 1)
#define SZ_8M   (SZ_1M << 3)
#define SZ_16M  (SZ_1M << 4)
#define SZ_32M  (SZ_16M << 1)
#define SZ_64M  (SZ_32M << 1)
#define SZ_128M (SZ_64M << 1)
#define SZ_256M (SZ_128M << 1)
#define SZ_512M (SZ_256M << 1)
#define SZ_1G   (SZ_512M << 1)

/*Handy align macros*/
#define ALIGN_MASK(N) (N-0x1)
#define ALIGN_MASK_COMPL(N) (~ALIGN_MASK(N))
/* this will round to floor with respect to an align number*/
#define ALIGN_FLR(AN,N) ((N)&ALIGN_MASK_COMPL(AN))
/* this will round to ceiling with respect to an align number*/
#define ALIGN(AN,N) (((N) + ALIGN_MASK(AN))&ALIGN_MASK_COMPL(AN))

/* For system with more than 16MB or more, use mem_topofmem as the starting point of image buffer.
   For system with only 8MB memory(6318), keep the old address - phy address 0x0000-0000
   mem_topofmem = end_of_cfe_ram_code_data_bbs+stack+heap
*/

/* Memory map of free memory areas for cfe usage */   
/* mem_topofmem is either:
1. Physical address mapped 1-1 to virtual address by MMU (ARM)
2. Virtual Address which is unmapped by MIPS with or without TLB(mmu) enabled 
   For the sake of brevity all CPU viewed addresses should be thought as virtual; 
   device view bus, address are physical translated from Virtual map as appropriate 
*/
    
#define MEMORY_AREA_HEAP_ADDR _VA(mem_heapstart)
#define MEMORY_AREA_HEAP_SIZE (_VA(mem_topofmem) - _VA(mem_heapstart))  

#if defined(CFG_RAMAPP)
extern unsigned long cfe_get_sdram_size(void);
extern unsigned char _ftext;
#define MEMORY_AREA_RSRV_ADDR _VA(PHYS_DRAM_BASE)
#define MEMORY_AREA_RSRV_ADDR_SIZE (mem_heapstart - _VA(PHYS_DRAM_BASE))

#define MEMORY_AREA_FREE_MEM (cfe_get_sdram_size() > SZ_8M ? _VA(mem_topofmem) : PHYS_DRAM_BASE)
#define MEMORY_AREA_FREE_MEM_SIZE (_VA(PHYS_DRAM_BASE) + cfe_get_sdram_size() - _VA(mem_topofmem)) 

#define BOARD_MAX_BOOT_IMAGE_SIZE (mem_bottomofmem - _VA(PHYS_DRAM_BASE))

#define MEMORY_BOOT_AREA_SIZE ((&_ftext - MEMORY_AREA_RSRV_ADDR) - ROM_PARMS_OFFSET) 
#define MEMORY_BOOT_AREA_OFFSET 0x8000

#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_) || \
    defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM963158_)|| \
    defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM96846_) ||\
    defined(_BCM96856_)
#define MEMORY_BOOT_AREA_ADDR ((MEMORY_AREA_RSRV_ADDR+MEMORY_BOOT_AREA_OFFSET) -  (otp_is_boot_secure()?sizeof(Booter1AuthArgs):0)) 
#else
#define MEMORY_BOOT_AREA_ADDR (MEMORY_AREA_RSRV_ADDR+MEMORY_BOOT_AREA_OFFSET) 
#endif

#endif

#ifdef __cplusplus
}
#endif
#endif /* _BCM_MEMORY_H */

