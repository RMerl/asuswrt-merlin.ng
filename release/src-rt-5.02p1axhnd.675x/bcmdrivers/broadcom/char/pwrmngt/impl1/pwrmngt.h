/*
* <:copyright-BRCM:2008:proprietary:standard
* 
*    Copyright (c) 2008 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#define CLK_ALIGNMENT_REG      (unsigned int *) 0xff410040

/* Define in 63[5|6]8_cpu.h */
// 0xff410040
#define KEEPME_MASK     0x00007F00 // bit[14:8]

#if defined (CONFIG_BCM960333)
#define RATIO_ONE_ASYNC     0x0 /* 0b00 */
#define RATIO_ONE_HALF      0x1 /* 0b01 */
#define RATIO_ONE_QUARTER   0x2 /* 0b10 */
#define RATIO_ONE_EIGHTH    0x3 /* 0b11 */

#define MASK_ASCR_BITS 0x3
#define MASK_ASCR_SHFT 23
#define MASK_ASCR (MASK_ASCR_BITS << MASK_ASCR_SHFT)
#else
#define RATIO_ONE_SYNC      0x0 /* 0b000 */
#define RATIO_ONE_ASYNC     0x1 /* 0b001 */
#define RATIO_ONE_HALF      0x3 /* 0b011 */
#define RATIO_ONE_QUARTER   0x5 /* 0b101 */
#define RATIO_ONE_EIGHTH    0x7 /* 0b111 */

#define MASK_ASCR_BITS 0x7
#define MASK_ASCR_SHFT 28
#define MASK_ASCR (MASK_ASCR_BITS << MASK_ASCR_SHFT)
#endif

#if defined(PWRMNGT_TRACE)
#define PWR_TRC(format, args...) printk(KERN_DEBUG "PWRMNGT: " format, ##args)
#else
#define PWR_TRC(format, args...)
#endif

/* handler for opcode */
typedef enum {
   SET,
   GET 
} PWRMNGT_OPCODE;
