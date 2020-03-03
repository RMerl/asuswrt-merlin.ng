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

#include "armv8.h"
#include "armmacros.h"

#define MEMC_HDP_MODE    0

/* DDR INIT test code */
/*
	op - operation type
		0 - read, 32 bit read operation
		1 - write, 32 bit write operation
		2 - poll read, read until the value read mactches read_value&mask
*/
#define reg_table_entry(op, addr, mask, value)	\
	.word	op;				\
	.word	addr;				\
	.word	mask;				\
	.word	value;				\
	
	.align 3
reg_table:
#if defined(_BCM94908_)	
#if MEMC_HDP_MODE
	reg_table_entry(0, 0x80018230, 0, 0x00000001 )
	reg_table_entry(1, 0x8001820C, 0, 0x8000b600 )
	reg_table_entry(1, 0x80018200, 0, 0x00000305 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000c1 )
	reg_table_entry(1, 0x8001A044, 0, 0x06005540 )
	reg_table_entry(1, 0x8001A048, 0, 0x1c060a0a )
	reg_table_entry(1, 0x8001A04C, 0, 0x060c080a )
	reg_table_entry(1, 0x8001A050, 0, 0x00046118 )
	reg_table_entry(1, 0x8001A054, 0, 0x00000000 )
	reg_table_entry(0, 0x8001A008, 0, 0x00810013 )
	reg_table_entry(1, 0x8001A008, 0, 0x00810012 )
	reg_table_entry(1, 0x8001A018, 0, 0x002030bf )
	reg_table_entry(0, 0x8001A010, 0, 0x18000000 )
	reg_table_entry(1, 0x8001A010, 0, 0x9c000000 )
	reg_table_entry(1, 0x8001A00C, 0, 0x000000d0 )
	reg_table_entry(0, 0x8001A008, 0, 0x00810012 )
	reg_table_entry(1, 0x8001A008, 0, 0x00810000 )
	reg_table_entry(2, 0x8001A004, 1, 0x00000001 )
	reg_table_entry(0, 0x8001A008, 0, 0x00810000 )
	reg_table_entry(1, 0x8001A008, 0, 0x00810000 )
	reg_table_entry(1, 0x8001A024, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A020, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A020, 0, 0x00000000 )
	reg_table_entry(0, 0x8001A1D0, 0, 0x00000009 )
	reg_table_entry(1, 0x8001A1D0, 0, 0x00000008 )
	reg_table_entry(1, 0x8001A19C, 0, 0x00000820 )
	reg_table_entry(0, 0x8001A1DC, 0, 0x000000c3 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e3 )
	reg_table_entry(0, 0x8001A1D0, 0, 0x00000008 )
	reg_table_entry(1, 0x8001A1D0, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A1D0, 0, 0x00000004 )
	reg_table_entry(1, 0x8001A058, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A058, 0, 0x00000059 )
	reg_table_entry(2, 0x8001A05C, 1, 0x00000001 )
	reg_table_entry(0, 0x8001A060, 0, 0x00090037 )
	reg_table_entry(0, 0x8001A05C, 0, 0x00005917 )
	reg_table_entry(0, 0x8001A058, 0, 0x00000058 )
	reg_table_entry(1, 0x8001A058, 0, 0x00000000 )
	reg_table_entry(0, 0x8001A0EC, 0, 0x00000016 )
	reg_table_entry(1, 0x8001A0EC, 0, 0x00010018 )
	reg_table_entry(0, 0x8001A0E0, 0, 0x00000016 )
	reg_table_entry(1, 0x8001A0E0, 0, 0x00010018 )
	reg_table_entry(1, 0x8001A038, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A03C, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A1F8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4B0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6B0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4B4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6B4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4B8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6B8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8B0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AAB0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8B4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AAB4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8B8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AAB8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4BC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6BC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8BC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AABC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4C0, 0, 0x000003ff )
	reg_table_entry(1, 0x8001A6C0, 0, 0x000003ff )
	reg_table_entry(1, 0x8001A8C0, 0, 0x000003ff )
	reg_table_entry(1, 0x8001AAC0, 0, 0x000003ff )
	reg_table_entry(0, 0x8001A040, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A040, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A1E0, 0, 0x18001800 )
	reg_table_entry(1, 0x8001A4CC, 0, 0x00000238 )
	reg_table_entry(1, 0x8001A6CC, 0, 0x00000238 )
	reg_table_entry(0, 0x8001A4C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A4C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A6C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A8C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001AAC4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A4C8, 0, 0x001241c2 )
	reg_table_entry(1, 0x8001A6C8, 0, 0x001241c2 )
	reg_table_entry(1, 0x8001A8C8, 0, 0x001241c2 )
	reg_table_entry(1, 0x8001AAC8, 0, 0x001241c2 )
	reg_table_entry(0, 0x8001A4AC, 0, 0x000f000a )
	reg_table_entry(1, 0x8001A4AC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001A6AC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001A8AC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001AAAC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000c1 )
	reg_table_entry(2, 0x8001A1DC, 2, 0x00000002 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e1 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e9 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00820218 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00830000 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00810006 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00801d60 )
	reg_table_entry(1, 0x8001A118, 0, 0x00300400 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e8 )
	reg_table_entry(0, 0x8001A1DC, 0, 0x000000e8 )
	reg_table_entry(0, 0x8001A138, 0, 0x00011d60 )
	reg_table_entry(0, 0x8001A13C, 0, 0x00010006 )
	reg_table_entry(0, 0x8001A140, 0, 0x00010218 )
	reg_table_entry(0, 0x8001A144, 0, 0x00010000 )
	reg_table_entry(0, 0x80018004, 0, 0x01100044 )
	reg_table_entry(1, 0x80018004, 0, 0x01100044 )
	reg_table_entry(0, 0x80018004, 0, 0x01100044 )
	reg_table_entry(1, 0x80018004, 0, 0x01100244 )
	reg_table_entry(0, 0x80018004, 0, 0x01100244 )
	reg_table_entry(1, 0x80018004, 0, 0x01100644 )
	reg_table_entry(0, 0x80018234, 0, 0x00000101 )
	reg_table_entry(1, 0x80018234, 0, 0x00001101 )
	reg_table_entry(1, 0x8001820C, 0, 0x0000b600 )
	reg_table_entry(1, 0x80018110, 0, 0x11100f0e )
	reg_table_entry(1, 0x80018114, 0, 0x15141312 )
	reg_table_entry(1, 0x80018118, 0, 0x19181716 )
	reg_table_entry(1, 0x8001811C, 0, 0x1d1c1b1a )
	reg_table_entry(1, 0x80018124, 0, 0x04000000 )
	reg_table_entry(1, 0x80018128, 0, 0x08070605 )
	reg_table_entry(1, 0x8001812C, 0, 0x00000a09 )
	reg_table_entry(1, 0x80018134, 0, 0x000d0c0b )
	reg_table_entry(1, 0x80018214, 0, 0x080a0a0a )
	reg_table_entry(1, 0x80018218, 0, 0x04040606 )
	reg_table_entry(1, 0x8001821C, 0, 0x20000626 )
	reg_table_entry(1, 0x80018220, 0, 0x06060c0c )
	reg_table_entry(1, 0x80018224, 0, 0x0a000118 )
	reg_table_entry(0, 0x80018004, 0, 0x01100644 )
	reg_table_entry(1, 0x80018004, 0, 0x811007aa )
	reg_table_entry(0, 0x80018004, 0, 0x811007aa )
#else
	reg_table_entry(0, 0x80018230, 0, 0x00000001 )
	reg_table_entry(1, 0x8001820C, 0, 0x8000b600 )
	reg_table_entry(1, 0x80018200, 0, 0x00000305 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000c1 )
	reg_table_entry(1, 0x8001A044, 0, 0x00005540 )	
	reg_table_entry(1, 0x8001A048, 0, 0x1c060a0a )
	reg_table_entry(1, 0x8001A04C, 0, 0x060c080a )
	reg_table_entry(1, 0x8001A050, 0, 0x00046118 )
	reg_table_entry(1, 0x8001A054, 0, 0x00000000 )
	reg_table_entry(0, 0x8001A008, 0, 0x00810013 )
	reg_table_entry(1, 0x8001A008, 0, 0x00810012 )
	reg_table_entry(1, 0x8001A018, 0, 0x002030bf )
	reg_table_entry(0, 0x8001A010, 0, 0x18000000 )
	reg_table_entry(1, 0x8001A010, 0, 0x9c000000 )
	reg_table_entry(1, 0x8001A00C, 0, 0x000000d0 )
	reg_table_entry(0, 0x8001A008, 0, 0x00810012 )
	reg_table_entry(1, 0x8001A008, 0, 0x00810000 )
	reg_table_entry(2, 0x8001A004, 1, 0x00000001 )
	reg_table_entry(0, 0x8001A008, 0, 0x00810000 )
	reg_table_entry(1, 0x8001A008, 0, 0x00810000 )
	reg_table_entry(1, 0x8001A024, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A020, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A020, 0, 0x00000000 )
	reg_table_entry(0, 0x8001A1D0, 0, 0x00000009 )
	reg_table_entry(1, 0x8001A1D0, 0, 0x00000008 )
	reg_table_entry(1, 0x8001A19C, 0, 0x00000820 )
	reg_table_entry(0, 0x8001A1DC, 0, 0x000000c3 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e3 )
	reg_table_entry(0, 0x8001A1D0, 0, 0x00000008 )
	reg_table_entry(1, 0x8001A1D0, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A1D0, 0, 0x00000004 )
	reg_table_entry(1, 0x8001A058, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A058, 0, 0x00000059 )
	reg_table_entry(2, 0x8001A05C, 1, 0x00000001 )
	reg_table_entry(0, 0x8001A060, 0, 0x00090037 )
	reg_table_entry(0, 0x8001A05C, 0, 0x00005917 )
	reg_table_entry(0, 0x8001A058, 0, 0x00000058 )
	reg_table_entry(1, 0x8001A058, 0, 0x00000000 )
	reg_table_entry(0, 0x8001A0EC, 0, 0x00000016 )
	reg_table_entry(1, 0x8001A0EC, 0, 0x00010018 )
	reg_table_entry(0, 0x8001A0E0, 0, 0x00000016 )
	reg_table_entry(1, 0x8001A0E0, 0, 0x00010018 )
	reg_table_entry(1, 0x8001A038, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A03C, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A1F8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4B0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6B0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4B4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6B4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4B8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6B8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8B0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AAB0, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8B4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AAB4, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8B8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AAB8, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4BC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A6BC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A8BC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001AABC, 0, 0x00031bff )
	reg_table_entry(1, 0x8001A4C0, 0, 0x000003ff )
	reg_table_entry(1, 0x8001A6C0, 0, 0x000003ff )
	reg_table_entry(1, 0x8001A8C0, 0, 0x000003ff )
	reg_table_entry(1, 0x8001AAC0, 0, 0x000003ff )
	reg_table_entry(0, 0x8001A040, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A040, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A1E0, 0, 0x18001800 )
	reg_table_entry(1, 0x8001A4CC, 0, 0x00000238 )
	reg_table_entry(1, 0x8001A6CC, 0, 0x00000238 )
	reg_table_entry(0, 0x8001A4C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A4C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A6C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A8C4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001AAC4, 0, 0x00020000 )
	reg_table_entry(1, 0x8001A4C8, 0, 0x001241c2 )
	reg_table_entry(1, 0x8001A6C8, 0, 0x001241c2 )
	reg_table_entry(1, 0x8001A8C8, 0, 0x001241c2 )
	reg_table_entry(1, 0x8001AAC8, 0, 0x001241c2 )
	reg_table_entry(0, 0x8001A4AC, 0, 0x000f000a )
	reg_table_entry(1, 0x8001A4AC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001A6AC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001A8AC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001AAAC, 0, 0x0000000a )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000c1 )
	reg_table_entry(2, 0x8001A1DC, 2, 0x00000002 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e1 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e9 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00820218 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00830000 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00810006 )
	reg_table_entry(1, 0x8001A11C, 0, 0x00000000 )
	reg_table_entry(1, 0x8001A118, 0, 0x00801d60 )
	reg_table_entry(1, 0x8001A118, 0, 0x00300400 )
	reg_table_entry(1, 0x8001A1DC, 0, 0x000000e8 )
	reg_table_entry(0, 0x8001A1DC, 0, 0x000000e8 )
	reg_table_entry(0, 0x8001A138, 0, 0x00011d60 )
	reg_table_entry(0, 0x8001A13C, 0, 0x00010006 )
	reg_table_entry(0, 0x8001A140, 0, 0x00010218 )
	reg_table_entry(0, 0x8001A144, 0, 0x00010000 )
	reg_table_entry(0, 0x80018004, 0, 0x01100044 )
	reg_table_entry(1, 0x80018004, 0, 0x01100044 )
	reg_table_entry(0, 0x80018004, 0, 0x01100044 )
	reg_table_entry(1, 0x80018004, 0, 0x01100244 )
	reg_table_entry(0, 0x80018004, 0, 0x01100244 )
	reg_table_entry(1, 0x80018004, 0, 0x01100644 )
	reg_table_entry(0, 0x80018234, 0, 0x00000101 )
	reg_table_entry(1, 0x80018234, 0, 0x00000101 )
	reg_table_entry(1, 0x8001820C, 0, 0x0000b600 )
	reg_table_entry(1, 0x80018110, 0, 0x1211100f )
	reg_table_entry(1, 0x80018114, 0, 0x16151413 )
	reg_table_entry(1, 0x80018118, 0, 0x1a191817 )
	reg_table_entry(1, 0x8001811C, 0, 0x1e1d1c1b )
	reg_table_entry(1, 0x80018124, 0, 0x05000000 )
	reg_table_entry(1, 0x80018128, 0, 0x09080706 )
	reg_table_entry(1, 0x8001812C, 0, 0x00000b0a )
	reg_table_entry(1, 0x80018134, 0, 0x000e0d0c )
	reg_table_entry(1, 0x80018214, 0, 0x080a0a0a )
	reg_table_entry(1, 0x80018218, 0, 0x04040606 )
	reg_table_entry(1, 0x8001821C, 0, 0x20000626 )
	reg_table_entry(1, 0x80018220, 0, 0x06060c0c )
	reg_table_entry(1, 0x80018224, 0, 0x0a000118 )
	reg_table_entry(0, 0x80018004, 0, 0x01100644 )
	reg_table_entry(1, 0x80018004, 0, 0x811007bb )
	reg_table_entry(0, 0x80018004, 0, 0x811007bb )
#endif
#endif
#if defined(_BCM963158_)
	reg_table_entry(2, 0x80180230, 1, 0x00000001)
	reg_table_entry(1, 0x8018020c, 0, 0x8000f000)
	reg_table_entry(1, 0x80180200, 0, 0x00000305)
	reg_table_entry(1, 0x801a01a4, 0, 0x00000001)
	reg_table_entry(1, 0x801a005c, 0, 0x00000000)
	reg_table_entry(1, 0x801a0060, 0, 0x00000a0e)
	reg_table_entry(1, 0x801a0064, 0, 0x00000000)
	reg_table_entry(0, 0x801a0010, 0, 0x55010013)
	reg_table_entry(1, 0x801a0010, 0, 0x55010012)
	reg_table_entry(1, 0x801a0020, 0, 0x0010102a)
	reg_table_entry(1, 0x801a0024, 0, 0x00099999)
	reg_table_entry(0, 0x801a0018, 0, 0x18000000)
	reg_table_entry(1, 0x801a0018, 0, 0x1c000000)
	reg_table_entry(1, 0x801a0014, 0, 0x00000220)
	reg_table_entry(0, 0x801a0010, 0, 0x55010012)
	reg_table_entry(1, 0x801a0010, 0, 0x55010000)
	reg_table_entry(2, 0x801a000c, 1, 0x00000001)
	reg_table_entry(0, 0x801a0010, 0, 0x55010000)
	reg_table_entry(1, 0x801a0010, 0, 0x55010000)
	reg_table_entry(1, 0x801a002c, 0, 0x00000000)
	reg_table_entry(1, 0x801a0028, 0, 0x00000000)
	reg_table_entry(1, 0x801a0028, 0, 0x00000000)
	reg_table_entry(0, 0x801a0198, 0, 0x00000009)
	reg_table_entry(1, 0x801a0198, 0, 0x00000008)
	reg_table_entry(1, 0x801a0194, 0, 0x00000820)
	reg_table_entry(0, 0x801a0204, 0, 0x07f8f0f0)
	reg_table_entry(1, 0x801a0204, 0, 0x27f8f0f0)
	reg_table_entry(0, 0x801a0198, 0, 0x00000008)
	reg_table_entry(1, 0x801a0198, 0, 0x00000000)
	reg_table_entry(1, 0x801a0198, 0, 0x00000004)
	reg_table_entry(1, 0x801a0068, 0, 0x00000000)
	reg_table_entry(1, 0x801a0068, 0, 0x00000059)
	reg_table_entry(2, 0x801a006c, 1, 0x00000001)
	reg_table_entry(0, 0x801a006c, 0, 0x00008617)
	reg_table_entry(0, 0x801a0070, 0, 0x000cd047)
	reg_table_entry(0, 0x801a006c, 0, 0x00008617)
	reg_table_entry(0, 0x801a0068, 0, 0x00000058)
	reg_table_entry(1, 0x801a0068, 0, 0x00000000)
	reg_table_entry(0, 0x801a0100, 0, 0x00000021)
	reg_table_entry(1, 0x801a0100, 0, 0x00010023)
	reg_table_entry(0, 0x801a00f0, 0, 0x00000021)
	reg_table_entry(1, 0x801a00f0, 0, 0x00010023)
	reg_table_entry(1, 0x801a018c, 0, 0x00000000)
	reg_table_entry(1, 0x801a0190, 0, 0x00000000)
	reg_table_entry(1, 0x801a0184, 0, 0x00000000)
	reg_table_entry(1, 0x801a0040, 0, 0x00031bff)
	reg_table_entry(1, 0x801a0044, 0, 0x00031bff)
	reg_table_entry(1, 0x801a01c4, 0, 0x00031bff)
	reg_table_entry(1, 0x801a04b0, 0, 0x00031bff)
	reg_table_entry(1, 0x801a04b4, 0, 0x00031bff)
	reg_table_entry(1, 0x801a04b8, 0, 0x00031bff)
	reg_table_entry(1, 0x801a06b0, 0, 0x00031bff)
	reg_table_entry(1, 0x801a06b4, 0, 0x00031bff)
	reg_table_entry(1, 0x801a06b8, 0, 0x00031bff)
	reg_table_entry(1, 0x801a08b0, 0, 0x00031bff)
	reg_table_entry(1, 0x801a08b4, 0, 0x00031bff)
	reg_table_entry(1, 0x801a08b8, 0, 0x00031bff)
	reg_table_entry(1, 0x801a0ab0, 0, 0x00031bff)
	reg_table_entry(1, 0x801a0ab4, 0, 0x00031bff)
	reg_table_entry(1, 0x801a0ab8, 0, 0x00031bff)
	reg_table_entry(1, 0x801a04bc, 0, 0x000003ff)
	reg_table_entry(1, 0x801a06bc, 0, 0x000003ff)
	reg_table_entry(1, 0x801a08bc, 0, 0x000003ff)
	reg_table_entry(1, 0x801a0abc, 0, 0x000003ff)
	reg_table_entry(0, 0x801a004c, 0, 0x00000600)
	reg_table_entry(1, 0x801a004c, 0, 0x000006ec)
	reg_table_entry(1, 0x801a0050, 0, 0x000006ec)
	reg_table_entry(0, 0x801a01b0, 0, 0x00180000)
	reg_table_entry(1, 0x801a01b0, 0, 0x001800e9)
	reg_table_entry(1, 0x801a01a8, 0, 0x00001800)
	reg_table_entry(1, 0x801a04d4, 0, 0x0000023c)
	reg_table_entry(1, 0x801a06d4, 0, 0x0000023c)
	reg_table_entry(1, 0x801a08d4, 0, 0x0000023c)
	reg_table_entry(1, 0x801a0ad4, 0, 0x0000023c)
	reg_table_entry(1, 0x801a0200, 0, 0x00000000)
	reg_table_entry(1, 0x801a04a8, 0, 0x00000000)
	reg_table_entry(1, 0x801a06a8, 0, 0x00000000)
	reg_table_entry(1, 0x801a08a8, 0, 0x00000000)
	reg_table_entry(1, 0x801a0aa8, 0, 0x00000000)
	reg_table_entry(0, 0x801a04c0, 0, 0x00020000)
	reg_table_entry(1, 0x801a04c0, 0, 0x000201d1)
	reg_table_entry(1, 0x801a06c0, 0, 0x000201d1)
	reg_table_entry(1, 0x801a08c0, 0, 0x000201d1)
	reg_table_entry(1, 0x801a0ac0, 0, 0x000201d1)
	reg_table_entry(1, 0x801a04d0, 0, 0x001241c2)
	reg_table_entry(1, 0x801a06d0, 0, 0x001241c2)
	reg_table_entry(1, 0x801a08d0, 0, 0x001241c2)
	reg_table_entry(1, 0x801a0ad0, 0, 0x001241c2)
	reg_table_entry(0, 0x801a04ac, 0, 0x003f000a)
	reg_table_entry(1, 0x801a04ac, 0, 0x0000000a)
	reg_table_entry(1, 0x801a06ac, 0, 0x0000000a)
	reg_table_entry(1, 0x801a08ac, 0, 0x0000000a)
	reg_table_entry(1, 0x801a0aac, 0, 0x0000000a)
	reg_table_entry(1, 0x801a01a4, 0, 0x00000001)
	reg_table_entry(2, 0x801a01a4, 2, 0x00000002)
	reg_table_entry(0, 0x801a0204, 0, 0x27f8f0f0)
	reg_table_entry(1, 0x801a0204, 0, 0x27f8f0f0)
	reg_table_entry(0, 0x801a0204, 0, 0x27f8f0f0)
	reg_table_entry(1, 0x801a0204, 0, 0x2ff8f0f0)
	reg_table_entry(1, 0x801a0128, 0, 0x00820228)
	reg_table_entry(1, 0x801a0128, 0, 0x00830000)
	reg_table_entry(1, 0x801a0128, 0, 0x00810006)
	reg_table_entry(1, 0x801a0128, 0, 0x00801124)
	reg_table_entry(1, 0x801a0128, 0, 0x00300400)
	reg_table_entry(1, 0x801a01a4, 0, 0x00000000)
	reg_table_entry(2, 0x801a01a4, 1, 0x00000000)
	reg_table_entry(0, 0x801a0148, 0, 0x00011124)
	reg_table_entry(0, 0x801a014c, 0, 0x00010006)
	reg_table_entry(0, 0x801a0150, 0, 0x00010228)
	reg_table_entry(0, 0x801a0154, 0, 0x00010000)
	reg_table_entry(0, 0x80180004, 0, 0x0110001f)
	reg_table_entry(1, 0x80180004, 0, 0x0110061f)
	reg_table_entry(0, 0x80180234, 0, 0x00000101)
	reg_table_entry(1, 0x80180234, 0, 0x00000101)
	reg_table_entry(1, 0x8018020c, 0, 0x0000f000)
	reg_table_entry(1, 0x80180110, 0, 0x13121110)
	reg_table_entry(1, 0x80180114, 0, 0x17161514)
	reg_table_entry(1, 0x80180118, 0, 0x1b1a1918)
	reg_table_entry(1, 0x8018011c, 0, 0x1f1e1d1c)
	reg_table_entry(1, 0x80180124, 0, 0x05000000)
	reg_table_entry(1, 0x80180128, 0, 0x09080706)
	reg_table_entry(1, 0x8018012c, 0, 0x0c000b0a)
	reg_table_entry(1, 0x80180134, 0, 0x000f0e0d)
	reg_table_entry(1, 0x80180100, 0, 0x00000000)
	reg_table_entry(1, 0x80180214, 0, 0x0a0e0e0e)
	reg_table_entry(1, 0x80180218, 0, 0x04040606)
	reg_table_entry(1, 0x8018021c, 0, 0x1b000832)
	reg_table_entry(1, 0x80180220, 0, 0x08080010)
	reg_table_entry(1, 0x80180224, 0, 0x0a000177)
	reg_table_entry(0, 0x80180004, 0, 0x0110061f)
	reg_table_entry(1, 0x80180004, 0, 0x8110071f)
	reg_table_entry(0, 0x80180004, 0, 0x8110071f)
	reg_table_entry(1, 0x80180010, 0, 0x0000000c) /* in MB, 4GB total size */
#endif
reg_table_end:



test_table:
	.word	0x0
	.word	0xffc0
	.word	0x3f0ffc0		
test_table_end: /* even number of total entries */


FUNC(ddr_init)
#if defined(_BCM94908_) || defined(_BCM963158_)
	mov	x14, lr
	
	LOADREL(x8, =reg_table)
	LOADREL(x9, =reg_table_end)
	mov	x0, xzr
	mov	x1, xzr
	mov	x2, xzr
	mov	x3, xzr
reg_loop:
	ldr	w0, [x8], #4
	ldr	w1, [x8], #4
	ldr	w2, [x8], #4
	ldr	w3, [x8], #4

	cmp	w0, #0
	bne	1f
	ldr	w4, [x1]
	b	next_reg
1:
	cmp	w0, #1
	bne	1f
	str	w3, [x1]
	b	next_reg
1:
	cmp	w0, #2
	bne	next_reg

	/* try up to 60 times... */
	mov	w6, #60
read_again:
	sub	w6, w6, #1
	cbnz	w6, read_reg
	SETLEDS('T','O','E','R')
	b	next_reg
read_reg:	
	ldr	w4, [x1]
	and	w4, w4, w2
	cmp	w4, w3
	beq	next_reg
	/* add some delay and read again */
	mov	w5, #100
delay:	
	sub	w5, w5, #1
	cbnz	w5, delay
	
	b	read_again
	
next_reg:	
	cmp	x8, x9
	bne	reg_loop

	/* make sure configure register write are really carried over to target block */
	dsb	sy
	isb
	
	/* simple memory test */
	LOADREL(x8, =test_table)
	LOADREL(x9, =test_table_end)
	mov	x0, xzr
	mov	x4, xzr
test_loop:
	ldr	w0, [x8], #4

	mov	w1, #128
next_addr:
	mvn	w3, w1
	str	w3, [x0]
	ldr	w2, [x0], #4
	cmp	w2, w3
	bne	test_error
	sub	w1, w1, #1
	cbnz	w1, next_addr
	
	cmp	x8, x9
	beq	test_ok
	add	x4, x4, #1
	b	test_loop
	
test_error:
	ldr	x1, =0x44524530   /* DRE0 */
	add	x0, x4, x1
	CALLINIT(=board_setleds)

	mov	lr, x14
	ret
test_ok:
	SETLEDS('P','A','S','S')
	mov	lr, x14

#elif defined(_BCM96858_)
	ldr	x0, =0x80180004
#if defined(CONFIG_BRCM_IKOS)
	ldr	w1, =0x81100745
#else
	ldr w1, =0x81100747
#endif
	str	w1, [x0]
#endif
	ret
END(ddr_init)
