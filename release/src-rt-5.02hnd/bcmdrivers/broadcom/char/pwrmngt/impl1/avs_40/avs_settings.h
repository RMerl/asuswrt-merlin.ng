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
 * $brcm_Workfile: avs_settings.h $
 * $brcm_Revision: 37 $
 * $brcm_Date: 8/10/12 5:24p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: /AP/bcm3128/c0/src/driver/avs/avs_settings.h $
 * 
 * 37   8/10/12 5:24p farshidf
 * SW3128-204: update avs to 0.5.4
 * 
 * Fw_Integration_Devel/19   8/10/12 5:21p farshidf
 * SW3128-204: update avs to 0.5.4
 * 
 * 36   6/27/12 11:47a enavarro
 * SWSATFE-174: added support for BCM4538
 * 
 * 35   6/26/12 6:34p mpovich
 * SW3472-3: Merge initial 3472 a0 baseline.
 * 
 * Fw_Integration_Devel/18   6/26/12 5:43p mpovich
 * SW3472-3: Merge to 3472 a0 initial baseline to integ. branch.
 * 
 * Fw_Integration_Devel/AP_V5_0_SYS_DEV/5   6/25/12 3:59p mpovich
 * SW3472-3: Set initial AVS REMOTE definition for 3472.
 * 
 * 34   6/14/12 3:42p farshidf
 * SW3461-227: Make the 3461/3462 5.0 release
 * 
 * Fw_Integration_Devel/17   6/14/12 3:18p farshidf
 * SW3461-227: Make the 3461/3462 5.0 release
 * 
 * Fw_Integration_Devel/AP_V5_0_SYS_DEV/4   6/7/12 12:35p farshidf
 * SW3461-1: merge to dev 5 branch
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/3   6/6/12 6:07p farshidf
 * SW3128-169: upgrade the AVS algo to version 5.2
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/2   5/11/12 4:45p farshidf
 * SW3128-160 : Make the FW 4.9 release
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/3   6/6/12 6:07p farshidf
 * SW3128-169: upgrade the AVS algo to version 5.2
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/2   5/11/12 4:45p farshidf
 * SW3128-160 : Make the FW 4.9 release
 * 
 * Fw_Integration_Devel/14   5/11/12 12:39p farshidf
 * SW3128-1: update the new sigma code
 * 
 * Fw_Integration_Devel/13   5/3/12 10:18a farshidf
 * SW3128-1: run avs and sigma before allowing the user to send Hab
 *  command
 * 
 * Fw_Integration_Devel/12   5/2/12 1:54p farshidf
 * SW3128-1: merge to integ
 * 
 * 29   5/2/12 1:51p farshidf
 * SW3128-1: fix the AVS and sigma calc
 * 
 * 28   4/19/12 4:01p enavarro
 * SWSATFE-97: set NUMBER_OF_REMOTE for BCM4550-B0
 * 
 * 27   3/30/12 11:15a farshidf
 * SW3128-139: Fw version 4.1
 * 
 * Fw_Integration_Devel/11   3/29/12 6:34p farshidf
 * SW3128-139: remove warning
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/1   3/29/12 6:19p farshidf
 * SW3128-139: remove warning
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/SW3462-6/2   2/29/12 5:33p farshidf
 * SW3461-165: fix avs for 3462
 * 
 * Fw_Integration_Devel/AP_V4_0_SYS_DEV/SW3462-6/1   2/28/12 5:22p mpovich
 * SW3462-6: Rebase with SW3462-3 dev. branch.
 * 
 * Fw_Integration_Devel/AP_V3_0_SYS_DEV/SW3462-3/1   2/27/12 5:01p mpovich
 * SW3462-3: 3462 initial baseline development and initial HAB support.
 * 
 * Fw_Integration_Devel/AP_V3_0_SYS_DEV/SW3462-2/1   1/23/12 5:11p mpovich
 * SW3462-2: Support for 3462.
 * 
 * Fw_Integration_Devel/10   12/20/11 4:31p farshidf
 * SW3128-98: add the sigma code
 * 
***************************************************************************/

#ifndef AVS_START_H__
#define AVS_START_H__

#ifdef __cplusplus
extern "C" {
#endif

void AvsInitializeOscs(AvsContext_t * pAvs);
void AvsSetVoltage(AvsContext_t * pAvs, uint32_t dac_code);
uint32_t AvsReadRmtOscFreq(AvsContext_t * pAvs, unsigned osc_num);
int AvsGetTemperature(AvsContext_t * pAvs);
uint32_t AvsReadVoltage(AvsContext_t * pAvs);
uint32_t AvsReadPvt(AvsContext_t * pAvs);
void AvsStart(AvsContext_t * pAvs, bool sigma_enabled);
void AvsUpdate(AvsContext_t * pAvs);
void AvsSetInitialVoltage(AvsContext_t * pAvs, uint32_t dac_code);
void Avs_BootRun(uint32_t vmin_avs_mv);
void AvsClearDACvalues(AvsContext_t * pAvs);
uint32_t Find1V(AvsContext_t * pAvs, uint32_t board_dac_code, bool sigma_enabled);
inline uint32_t GetBoardDACvalue(AvsContext_t * pAvs);

/* The number of remote oscillators is different from part to part */
#if BCHP_CHIP==7422
 #define NUMBER_OF_REMOTE 32
#elif BCHP_CHIP==7425
 #define NUMBER_OF_REMOTE 32
#elif BCHP_CHIP==7344
 #define NUMBER_OF_REMOTE 32
#elif (BCHP_CHIP==7346) || (BCHP_CHIP==4528)
 #define NUMBER_OF_REMOTE 38
#elif BCHP_CHIP==7231
 #define NUMBER_OF_REMOTE 28
#elif BCHP_CHIP==7358
 #define NUMBER_OF_REMOTE 32 /* RDB shows 38 but really only 32 working */
#elif BCHP_CHIP==7552
 #define NUMBER_OF_REMOTE 38
#elif (BCHP_FAMILY==3128)
#define NUMBER_OF_REMOTE 8
#elif ((BCHP_FAMILY==3461) || (BCHP_FAMILY==3462) || (BCHP_FAMILY==3472))
#define NUMBER_OF_REMOTE 22
#elif (BCHP_CHIP==4538)
#define NUMBER_OF_REMOTE 50
#elif (BCHP_CHIP==4550)
#if BCHP_VER==BCHP_VER_A0     
   #define NUMBER_OF_REMOTE 20
#else
   #define NUMBER_OF_REMOTE 30
#endif
#elif (BCHP_CHIP==4517)
#define NUMBER_OF_REMOTE 14
#elif (BCHP_CHIP==6802)
#define NUMBER_OF_REMOTE 20
#elif (BCHP_CHIP==6318)
#define NUMBER_OF_REMOTE 16
#endif

#if (BCHP_CHIP==6802)
#define AVS_XTAL_FREQUENCY_MHZ  50
#define AVS_CHIP_ID_REGISTER    0x10404004
#else
#define AVS_XTAL_FREQUENCY_MHZ  54
#define AVS_CHIP_ID_REGISTER    0x0
#endif

#define AVS_XTAL_FACTOR         (AVS_XTAL_FREQUENCY_MHZ * 2)

#define SCALING_FACTOR 10000 /* make this larger to get more precision in the numbers (but be careful of overflow) */
#define S1 SCALING_FACTOR
#define S2 SCALING_FACTOR
#define DIVIDER_DEFINE     1000 
#define DIVIDER_ADJUSTMENT 10   

#define INT(x) ((int32_t)((x)*S1)) /* this creates an integer from a float with the precision defined above */

#define XTAL_ADJUST(x) (INT(((x) * 54) / AVS_XTAL_FREQUENCY_MHZ))

#define AVS_MAX_CONTEXTS  2

#ifdef __cplusplus
}
#endif

#endif

