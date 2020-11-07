/*
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
*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  47622_map.h                                               */
/*   DATE:    01/26/18                                                 */
/*   PURPOSE: Define the proprietary hardware blocks/subblocks for     */
/*            BCM47622                                                  */
/*                                                                     */
/***********************************************************************/
#ifndef __BCM47622_MAP_H
#define __BCM47622_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "47622_common.h"
#include "47622_intr.h"
#include "47622_map_part.h"

/* For the proprietary blocks that needs be mapped in the linux, the base 
   address still define in the map_part.h. The detail block register definition
   must be defined in this file with the C structure below */
#define BROM_BASE                   (PERF_BASE + 0x00000600)  /* bootrom registers */
#define BROM_GEN_BASE               BROM_BASE
#define BROM_SEC_BASE               (PERF_BASE + 0x00000620)  /* bootrom secure registers */
#define BROM_SEC1_BASE              BROM_SEC_BASE

/* For the proprietary blocks does not need be mapped in linux at all */
#define BOOTROM_BASE                0xfff00000
#define BOOTROM_SIZE                0x20000
#define PERF_SRAM_BASE              0xfff80000
#define PERF_SRAM_SIZE              0x8000
#define RDP_SRAM_BASE               0x82600000
#define RDP_SRAM_SIZE               0x80000     /* 512KB */

#define BOOTLUT_SIZE                0x1000

#ifndef __ASSEMBLER__

#if defined(__KERNEL__) && !defined(MODULE)
#error "PRIVATE FILE INCLUDED IN KERNEL"
#endif

typedef struct SotpRegs {
   uint32 sotp_otp_prog_ctrl;            /* 0x00 */  
   uint32 sotp_otp_wdata_0;              /* 0x04 */
   uint32 sotp_otp_wdata_1;              /* 0x08 */
   uint32 sotp_otp_addr;                 /* 0x0c */
   uint32 sotp_otp_ctrl_0;               /* 0x10 */
   uint32 dummy1;                        /* 0x14 */  
   uint32 sotp_otp_status_0;             /* 0x18 */
   uint32 sotp_otp_status_1;             /* 0x1c */
   uint32 sotp_otp_rdata_0;              /* 0x20 */
   uint32 sotp_otp_rdata_1;              /* 0x24 */
   uint32 sotp_chip_states;              /* 0x28 */
   uint32 dummy2;                        /* 0x2c */  
   uint32 sotp_otp_ecccnt;               /* 0x30 */
   uint32 sotp_otp_bad_addr;             /* 0x34 */
   uint32 sotp_otp_wr_lock;              /* 0x38 */
   uint32 sotp_otp_rd_lock;              /* 0x3c */
   uint32 sotp_rom_block_start;          /* 0x40 */
   uint32 sotp_rom_block_end;            /* 0x44 */
   uint32 sotp_samu_cntrl;               /* 0x48 */
   uint32 sotp_chip_cntrl;               /* 0x4c */
   uint32 sotp_sr_state_0;               /* 0x50 */
   uint32 sotp_sr_state_1;               /* 0x54 */
   uint32 sotp_sr_state_2;               /* 0x58 */
   uint32 sotp_sr_state_3;               /* 0x5c */
   uint32 sotp_sr_state_4;               /* 0x60 */
   uint32 sotp_sr_state_5;               /* 0x64 */
   uint32 sotp_sr_state_6;               /* 0x68 */
   uint32 sotp_sr_state_7;               /* 0x6c */
   uint32 sotp_perm;                     /* 0x70 */
   uint32 sotp_sotp_out_0;               /* 0x74 */
   uint32 sotp_sotp_out_1;               /* 0x78 */
   uint32 sotp_sotp_out_2;               /* 0x7c */
   uint32 sotp_sotp_inout;               /* 0x80 */
} SotpRegs;
#define SOTP ((volatile SotpRegs * const) SOTP_BASE)


typedef struct BootBase {
    uint32 general_secbootcfg;
#define BOOTROM_CRC_DONE                (1 << 31)
#define BOOTROM_CRC_FAIL                (1 << 30)
    uint32 general_boot_crc_low;
    uint32 general_boot_crc_high;
} BootBase;

#define BOOTBASE ((volatile BootBase * const) BROM_BASE)

typedef struct BootSec {
    uint32 AccessCtrl;
    uint32 AccessRangeChk[4];
} BootSec;

#define BOOTSECURE ((volatile BootSec * const) BROM_SEC_BASE)

#endif


#ifdef __cplusplus
}
#endif

#endif

