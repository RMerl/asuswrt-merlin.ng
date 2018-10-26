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
#ifndef __BCM_BTRM_COMMON_H
#define __BCM_BTRM_COMMON_H

#if defined(__ASSEMBLER__)
#define SETLEDS2(a,b,c,d)			          \
	ldr R_ARG, =(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;  \
	bl board_setleds
#endif
#define _UI32(c1,c2,c3,c4)  (((uint32_t)(uint8_t)(c1)<<24)|((uint32_t)(uint8_t)(c2)<<16)|((uint32_t)(uint8_t)(c3)<<8)|((uint32_t)(uint8_t)(c4)))
#define BTRM_PRINT(c1,c2,c3,c4) board_setleds(_UI32(c1,c2,c3,c4))

#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM96838_) || defined(_BCM963381_) || defined(_BCM96848_) || defined(_BCM63268_)
/* Gen2/Gen1 Bootrom */
#include "bcm_btrm_gen2_common.h"
#else
/* Gen3 Bootrom */
#include "bcm_btrm_gen3_common.h"
#endif 

#endif
