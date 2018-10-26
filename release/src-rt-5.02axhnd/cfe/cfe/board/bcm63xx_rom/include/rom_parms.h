/*
    Copyright 2000-2015 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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
 * File Name  : rom_parms.h
 *
 * Description: This file contains defines varios early cfe rom boot parameters 
            mainly to provide run-time choice to re-configure certain cfe functions  
 *
 * Updates    : 09/25/2015  Created.
 ***************************************************************************/

#ifndef _ROM_PARMS_H

#define _ROM_PARMS_H

#ifdef __cplusplus
extern "C" {
#endif

#if (INC_BTRM_BUILD==0)

#include "boardparms.h"
#include "btrm_if.h"
#define MCB_SEL_OVERRIDE                BP_DDR_CONFIG_OVERRIDE
#define MCB_SEL_MASK                    0x7fffffff
#define MCB_SEL_SAFEMODE                0x1 
#define NAND_IMAGESEL_OVERRIDE          (1 << 29)
#define NAND_IMAGESEL_MASK              0xF
#define NAND_IMAGESEL_0                 0
#define NAND_IMAGESEL_1                 1

#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM947189_) || defined(_BCM96846_)
#define RFS_OFFSET  16*1024
#elif defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_)
#define RFS_OFFSET  1024*1024
#else
#define RFS_OFFSET  0
#endif

/*16 32 bit words for future extensions
 Indices are counted (from higher to lower addrss e.g. addr 0x0 == index 16 .. addr 128 == index 0) to base of _ftext for cfe_ram 
*/

#define ROM_PARMS_SIZE  (16+sizeof(Booter1AuthArgs))
#define ROM_PARMS_OFFSET (RFS_OFFSET + ROM_PARMS_SIZE)

#define ROM_PARMS_PTR(type, p, i)    ((type*)(p - (RFS_OFFSET + sizeof(type)*(i+1)))) 
#define ROM_PARMS_SET(type, p, i, v) do { *ROM_PARMS_PTR(type, p, i) = (v); } while(0)
#define ROM_PARMS_GET(type, p, i)    (*ROM_PARMS_PTR(type, p, i))

#define ROM_PARMS_SET_ROOTFS(p,val)      ROM_PARMS_SET(unsigned char, p, 0, val)
#define ROM_PARMS_GET_ROOTFS(p)          ROM_PARMS_GET(unsigned char, p, 0)

#define ROM_PARMS_SET_SEQ_P1(p,val)      ROM_PARMS_SET(unsigned int, p, 1, val)
#define ROM_PARMS_GET_SEQ_P1(p)          ROM_PARMS_GET(unsigned int, p, 1)

#define ROM_PARMS_SET_SEQ_P2(p,val)      ROM_PARMS_SET(unsigned int, p, 2, val)
#define ROM_PARMS_GET_SEQ_P2(p)          ROM_PARMS_GET(unsigned int, p, 2)

#define ROM_PARMS_SET_ROM_PARM(p,val)    ROM_PARMS_SET(unsigned int, p, 3, val)
#define ROM_PARMS_GET_ROM_PARM(p)        ROM_PARMS_GET(unsigned int, p, 3)

#define ROM_PARMS_AUTH_PARM_PTR(p)       ROM_PARMS_PTR(unsigned char, p, ROM_PARMS_SIZE)

#define CHECK_IF_AUTH_TYPE(p)         (void)((*p).manu);(void)((*p).oper);
#define ROM_PARMS_AUTH_PARM_SETM(p, src) do { CHECK_IF_AUTH_TYPE(src); \
                                              memcpy((void *)ROM_PARMS_AUTH_PARM_PTR(p), (void *)src, sizeof(Booter1AuthArgs)); \
                                         } while(0)

#define ROM_PARMS_AUTH_PARM_GETM(p, dst) do { CHECK_IF_AUTH_TYPE(dst);\
                                              memcpy((void*)dst, (void *)ROM_PARMS_AUTH_PARM_PTR(p), sizeof(Booter1AuthArgs));\
                                         } while(0)

#define ROM_PARMS_SET_DEVEL_ARG(p,val)       ROM_PARMS_SET(unsigned char, p, ROM_PARMS_SIZE+sizeof(uint8_t),val)
#define ROM_PARMS_GET_DEVEL_ARG(p)           ROM_PARMS_GET(unsigned char, p, ROM_PARMS_SIZE+sizeof(uint8_t))
extern unsigned char _ftext;
#define CFE_RAM_ROM_PARMS_GET_ROOTFS         ROM_PARMS_GET_ROOTFS(&_ftext)
#define CFE_RAM_ROM_PARMS_GET_SEQ_P1         ROM_PARMS_GET_SEQ_P1(&_ftext)
#define CFE_RAM_ROM_PARMS_GET_SEQ_P2         ROM_PARMS_GET_SEQ_P2(&_ftext)
#define CFE_RAM_ROM_PARMS_GET_ROM_PARM       ROM_PARMS_GET_ROM_PARM(&_ftext)
#define CFE_RAM_ROM_PARMS_AUTH_PARM_GETM(dst)  ROM_PARMS_AUTH_PARM_GETM(&_ftext,dst)
#define CFE_RAM_ROM_PARMS_DEVEL_GET          ROM_PARMS_GET_DEVEL_ARG(&_ftext)

#endif /* !INC_BTRM_BUILD */

#if defined (_BTRM_DEVEL_)
/*12 bits reserved to be used as ASM immediate offset*/
#define ROM_ARG_MASK                    0xffe
/*if used in*/
#define ROM_ARG_DRAM_INIT_BYPASS        0x80
#define ROM_ARG_INVALID                 0x0
#define ROM_ARG_SEC_FLD_DEVEL           0x8
#define ROM_ARG_SEC_MFG_DEVEL           0x4

#ifdef CFG_RAMAPP
#define _ROM_ARG_  CFE_RAM_ROM_PARMS_DEVEL_GET 
#else
#define _ROM_ARG_ (((Booter1Args *)BTRM_INT_MEM_CREDENTIALS_ADDR)->otpDieFallThru)
#endif

#define ROM_ARG_ISSET(v)   (((_ROM_ARG_&ROM_ARG_MASK)&v) == v)

#else

#define ROM_ARG_ISSET(v)   (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BOARDPARMS_H */

