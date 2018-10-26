/*  *********************************************************************
    *
    <:copyright-BRCM:2015:proprietary:standard
    
       Copyright (c) 2015 Broadcom 
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

#define CPUCFG_CPUINIT		bcmcore_cpuinit
#define CPUCFG_ALTCPU_START1	bcmcore_null
#define CPUCFG_ALTCPU_START2	bcmcore_null
#define CPUCFG_ALTCPU_RESET	bcmcore_null
#define CPUCFG_CPURESTART	bcmcore_cpurestart
#define CPUCFG_DRAMINIT		board_draminit		/* no dram on CPU */
#define CPUCFG_CACHEOPS		bcmcore_cacheops
#define CPUCFG_ARENAINIT	armv8_arena_init
#define CPUCFG_PAGETBLINIT	armv8_pagetable_init
#define CPUCFG_TLBHANDLER	bcmcore_tlbhandler
#define CPUCFG_DIAG_TEST1	bcmcore_null
#define CPUCFG_DIAG_TEST2	bcmcore_null

#define CPUCFG_SCU_ON		armv8_scu_on
#define CPUCFG_L1CACHE_ON	armv8_l1cache_on
#define CPUCFG_L2CACHE_ON	l2cache_on

/*
 * The ARM ticks CP15 every other cycle.
 */

#define CPUCFG_CYCLESPERCPUTICK	1

/*
 * Hazard macro
 */

#define HAZARD nop ; nop ; nop ; nop ; nop ; nop ; nop

#if !defined(_BCM963158_)
#define MEMC_SRAM
#endif

#if defined(CFG_RAMAPP)
#define CPUCFG_MMU_TABLE_BASE	0x00f80000
#define CPUCFG_MMU_TABLE_SIZE	0x80000
#else
#if !defined (MEMC_SRAM) && defined(_BCM963158_)
#define CPUCFG_MMU_TABLE_BASE	0x808a0000
#define CPUCFG_MMU_TABLE_SIZE	0x10000
#else  
#define CPUCFG_MMU_TABLE_BASE	0x7fff0000
#define CPUCFG_MMU_TABLE_SIZE	0x10000
#endif

#endif

/* 
 * Definition for the number of VA and PA address bit. 
 *
 * ARMv8 supports up to 48 bit VA. We only support up to 38 bit VA(MMU 
 * walking starting at level 1). This is good enough. 
 *
 * Number PA bit depends on the SoC.For SoC support 4GB 
 * memory(2GB max) and peripheral space, 32 bit is used. For newer
 * SoC that with large memory 4GB and up, additional physical bits
 * are needed. ARMv8 only support certain bits like 32, 36, 40 so
 * we round to the next available bits. 
 *
 * We use more VA bits than PA bits so the additional upper bits 
 * can be used for specifying VA address properties such as 
 * cached/noncached/ro/rw for memory
 */

#if !defined(_BCM963158_)
#define CPUCFG_NUM_VA_BITS	38
#define CPUCFG_NUM_PA_BITS	32
#else
#define CPUCFG_NUM_VA_BITS	38
/* 33 bits PA is needed to support 4GB DDR + Periph for 63158, round to
 * the next supported ARM required PA bits */
#define CPUCFG_NUM_PA_BITS	36
#endif

/* highest VA bit for cache/noncache property */
#define CPUCFG_VA_NONCACHE_BIT   (CPUCFG_NUM_VA_BITS-1)
