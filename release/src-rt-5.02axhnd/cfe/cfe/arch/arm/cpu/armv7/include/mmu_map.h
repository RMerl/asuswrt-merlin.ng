/*  *********************************************************************
    *
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
       All Rights Reserved
    
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
    ********************************************************************* */
#ifndef _BTRM_MMU_MAP_H_
#define _BTRM_MMU_MAP_H_
#include <arm.h>
#include <armmacros.h>
#include <cpu_config.h>

#ifdef __ASSEMBLER__

#if defined(BTRM_USE_PAGEMAP)
#define armv7_mmu_map board_mmu_map_pg
#else 
#define armv7_mmu_map board_mmu_map_scn
#endif
#define armv7_mmu_update board_mmu_update_scn

#define L1TABLE_SIZE	(4096*4) 
#define L2TABLE_SIZE	(256*4)
#define L2PAGE_SIZE	4096

/* Megabyte aligned address map (section of 1 MB) 
*     _L1TBL_ -  table phys address 
*     _L1TBL_LEN_ - length of the table in  words (4byte)
*     _VA_ - start virtual address; MB aligned
*     _RANGE_ - a virtual/physical range to be mapped (in section units aka mb) 
*     _ATTR_  - since it is a MB section - section attributes are applied
*     _PA_ - corresponding to VA MB, aligned, mapped physical memory address 
*     Note: r1-r8 are trashed
*/
#define MMU_MAP_SCN(_L1TBL_, _L1TBL_LEN_, _VA_, _RANGE_, _ATTR_, _PA_)	\
		ldr	r0, =(_L1TBL_);						\
		ldr	r1, =(_VA_);						\
 		ldr	r2, =(_RANGE_);						\
        	ldr	r3, =(_ATTR_);						\
		ldr	r4, =(_PA_);						\
		ldr	r6, =(_L1TBL_LEN_);					\
		bl	mmu_set_scn
/* As above using register in place of _RANGE_*/
#define _MMU_MAP_SCN(_L1TBL_, _L1TBL_LEN_, _VA_, _RANGE_, _ATTR_, _PA_)	\
		ldr	r0, =(_L1TBL_);						\
		ldr	r1, =(_VA_);						\
 		mov	r2, _RANGE_;						\
        	ldr	r3, =(_ATTR_);						\
		ldr	r4, =(_PA_);						\
		ldr	r6, =(_L1TBL_LEN_);					\
		bl	mmu_set_scn

/* VA to PA page addr calculation (4K small pages) 
*    L2TBL_PA = L1TBL_PA[31:12] + VA[31:20]*4 
*     PAGE_PA = L2TBL_PA[31:10] + VA[19:12] ]
*/

#define MMU_INIT_TABLE(_TBL_,_TBL_SZ_)	\
	ldr	r0,=(_TBL_);			\
	mov	r1,#(_TBL_SZ_);			\
	bl	mmu_init_table
/* 
*	r0 - table paddr
*	r1 - table size (bytes)
*	min size 16 bytes	
*/
#define MMU_SET_TTBR(_L1TBL_)	\
	ldr	r0,=(_L1TBL_);		\
	bl	mmu_set_ttbr


#if defined(BTRM_USE_PAGEMAP)

/* Page (4K) aligned address map
*     Tables addresses set to r0/r1
*     _L1TBL_ table phys address
*     _L2TBL_ (page) table phys address
*     _L2TBL_LEN_ (page) table length in words (4byte) 
*     paramaters:
*     _VA_ - start virtual address; page aligned 
*     _RANGE_ - range of virtual/physical address in pages to be mapped 
*     _PG_ATTR_  - page attributes
*     _PG_PA_ - corresponding page aligned physical address 
*     _L2TBL_ATTR_ - L2TBL attributes 
*     Note: r0-r8 are trashed
*/
#define MMU_MAP_PG(_L1TBL_,_L2TBL_,_L2TBL_LEN_,_VA_, _RANGE_, _PG_ATTR_, _PG_PA_, _L2TBL_ATTR_)	\
		ldr	r0, =(_L1TBL_);									\
		ldr	r1, =(_L2TBL_);									\
		ldr	r2, =(_VA_);									\
 		mov	r3, #(_RANGE_);									\
        	ldr	r4, =(_PG_ATTR_);								\
        	ldr	r5, =(_PG_PA_);									\
        	ldr	r6, =(_L2TBL_ATTR_);								\
		ldr	r8, =(_L2TBL_LEN_);								\
		bl	mmu_set_page
/*
*  Page attributes
* 
*  Small Page (4K) Descriptor
*  | PA[31:12]|nG[11]|S[10]|AP(2)[9]|TEX(2:0)[8:6]|AP(1:0)[5:4]|C[3]]|B[2]|1|0|
*/

#define SMALL_PAGE_ATTR(ATR_NG,ATR_S,ATR_AP,ATR_TEX,ATR_C, ATR_B)   ( ((ATR_NG&0x1)<<11) |	\
									((ATR_S&0x1)<<10)|	\
									(((ATR_AP&0x7)>>2)<<9)|	\
									((ATR_TEX&0x7)<<6) |	\
									((ATR_AP&0x3)<<4) |	\
									((ATR_C&0x1)<<3) |	\
									((ATR_B&0x1)<<2) |	\
									0x2 )

/* nG=0 S=0 AP=0x3 TEX=5 C=0 B=1  */
#define PAGE_OUTER_WBWA_INNER_WBWA	\
	SMALL_PAGE_ATTR(0,0,0x3,0x5,0x0,0x1)

/* Page Table attributes; used in L1 table entries*/
#define TABLE_PAGE_ATTR(DOM,NS,PXN)	\
	(((DOM&0xf)<<5)|((NS&0x1)<<3)|((PXN&0x1)<<2)|0x1)
/*DOM=0x0 NS=0 PXN=0*/
#define TABLE_PAGE_SEC_DOM0_UNPRIV	\
	TABLE_PAGE_ATTR(0x0,0x0,0x0)

/*DOM=0x0 NS=1 PXN=0*/
#define TABLE_PAGE_NS_DOM0_UNPRIV	\
	TABLE_PAGE_ATTR(0x0,0x1,0x0)
#endif /*__PAGE_MAP__*/

/*  MMU and TT (Translation Tables) definitions 

   WBWA == Write-Back, Write-Allocate
   WBNWA == Write-Back, No Write-Allocate
   WTNWA == Write-Through, No Write-Allocate
   NC == Non-Cacheable
   SO == Strongly-Ordered
   SD == Sharable-Device
   NSD == Non-Sharable-Device
*/
  
#define DESC_DOMAIN(x)          ((x << 5) & 0x000001E0)

// section descriptor definitions
#define SECTION_AP              0xc00
#define SECTION_XN              0x10
#define SECTION_PXN             0x1
#if defined(_BCM963138_)
/* A9 does not support PXN */
#define SECTION_XN_ALL          (SECTION_XN)
#else
#define SECTION_XN_ALL          (SECTION_XN|SECTION_PXN)
#endif
#define SECTION_SHAREABLE       (1 << 16)
#define SECTION_SUPER_DESC      (1 << 18)
#define SECTION_DESC_NS         (1 << 19) 
// TEX[2] = 1
#define SECTION_OUTER_NC_INNER_WBWA         0x00004006
#define SECTION_OUTER_WBNWA_INNER_WBWA      0x00007006
#define SECTION_OUTER_WTNWA_INNER_WBWA      0x00006006
#define SECTION_OUTER_WBWA_INNER_NC         0x00005002
// TEX[2] = 0, OUTER & INNER are same all the time
#define SECTION_OUTER_WBWA_INNER_WBWA       0x0000100E
#define SECTION_OUTER_NSD_INNER_NSD         0x00002002
#define SECTION_OUTER_NC_INNER_NC           0x00001002
#define SECTION_OUTER_WTNWA_INNER_WTNWA     0x0000000A
#define SECTION_OUTER_WBNWA_INNER_WBNWA     0x0000000E
#define SECTION_OUTER_SO_INNER_SO           0x00000002
#define SECTION_OUTER_SD_INNER_SD           0x00000006

// definition for common section attribute 
#define SECTION_ATTR_INVALID       0x0  
#define SECTION_ATTR_CACHED_MEM    \
	(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0))
#define SECTION_ATTR_NONCACHED_MEM \
	(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
#define SECTION_ATTR_DEVICE        \
	(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|SECTION_XN_ALL|DESC_DOMAIN(0))
#define SECTION_ATTR_DEVICE_EXEC   \
	(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))

#endif /*__ASSEMBLER__*/

#endif /*_BTRM_MMU_MAP_H_*/
