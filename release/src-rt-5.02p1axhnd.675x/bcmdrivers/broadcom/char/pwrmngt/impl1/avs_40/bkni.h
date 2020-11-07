/***************************************************************************
 *     (c)2008-2012 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 *
 * Module Description:
 *
 * Revision History:
 *
 * 
 ***************************************************************************/

#ifndef BKNI_H__
#define BKNI_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define BCA_DEBUG_STATS 1

#define AVERAGE_LOOPS 12

typedef struct __AvsContext_t__
{
	uint32_t        device_id;
	bool            AvsRunning;
	bool            first_time;
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
	bool AvsFlushReading;
	uint32_t lower_hvt_threshold;
	uint32_t lower_svt_threshold;
	uint32_t upper_hvt_threshold;
	uint32_t upper_svt_threshold;
#endif
	int32_t Vmin_Avs;
	int32_t  vmarginL;
	int32_t  vmarginH;
	bool avs_detect;
	uint32_t predicated_value;
#if (BCHP_CHIP==6318)
	uint32_t saved_dac_code;
#endif
	struct {
		uint32_t code_array[AVERAGE_LOOPS];
		int index;
		int num;
	} SwTakeoverVars;
} AvsContext_t;


#if defined(CONFIG_BCM_6802_MoCA)
#include "bbsi.h"

#define AVS_BASE 0x10104000

#ifdef BCA_DEBUG_STATS
extern uint32_t reg_read_count;

static inline uint32_t BREG_Read32(void * c, uint32_t x)
{
   reg_read_count++;
   return((uint32_t)kerSysBcmSpiSlaveReadReg32(((AvsContext_t *)c)->device_id, (uint32_t)(x)));
}

#define BREG_Write32(c,x,y)   do { reg_write_count++; kerSysBcmSpiSlaveWriteReg32(((AvsContext_t *)c)->device_id, (uint32_t)(x), (y)); } while(0)
#else
#define BREG_Read32(c,x)   ((uint32_t)kerSysBcmSpiSlaveReadReg32(((AvsContext_t *)c)->device_id, (uint32_t)(x)))
#define BREG_Write32(c,x,y)  kerSysBcmSpiSlaveWriteReg32(((AvsContext_t *)c)->device_id, (uint32_t)(x), (y))
#endif

#define BREG_Read32_Single(c,x)  BREG_Read32(c,x) 

#endif

#define BKNI_Sleep(ms)  msleep(ms)
#define BKNI_Delay(us)  msleep((us-1)/1000 + 1)   // Round up
#define BDBG_MSG(x) //printk x

#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
#define AVS_ENABLED
#endif


#ifdef __cplusplus
}
#endif

#endif

