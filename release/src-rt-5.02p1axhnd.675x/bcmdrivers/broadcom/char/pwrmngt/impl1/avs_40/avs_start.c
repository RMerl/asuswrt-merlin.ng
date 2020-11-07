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
 * $brcm_Workfile: avs_start.c $
 * $brcm_Revision: 49 $
 * $brcm_Date: 8/13/12 1:14p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: /AP/bcm3128/c0/src/driver/avs/avs_start.c $
 * 
 * 49   8/13/12 1:14p farshidf
 * SW3128-204: update avs to 0.5.4
 * 
 * Fw_Integration_Devel/34   8/13/12 1:14p farshidf
 * SW3128-204: update avs to 0.5.4
 * 
 * Fw_Integration_Devel/33   8/10/12 5:22p farshidf
 * SW3128-204: update avs to 0.5.4
 * 
 * 46   7/27/12 9:32a enavarro
 * SWSATFE-97: set minimum voltage to 0.77V for BCM4550-B0
 * 
 * 45   7/17/12 5:53p farshidf
 * SW3128-183: fix CableLabs OpenCable ATP 4.1.11 Combined Distortion Test
 *  (Burst Noise)
 * 
 * 44   6/14/12 3:39p enavarro
 * SWSATFE-97: 4550 updates
 * 
 * 43   6/14/12 11:44a enavarro
 * SWSATFE-97: updated 4550 settings
 * 
 * 42   6/6/12 6:11p farshidf
 * SW3128-169: upgrade the AVS algo to version 5.2
 * 
 * 41   6/5/12 4:17p enavarro
 * SWSATFE-97: updates for BCM4550-B0
 * 
 * 40   5/29/12 12:05p farshidf
 * SW3128-167: fix avs detection
 * 
 * 39   5/23/12 10:03a enavarro
 * SWSATFE-97: updated central oscillator thresholds and min voltage for
 *  4550-B0 per Ali Salem
 * 
 * 38   5/11/12 5:19p farshidf
 * SW3128-160: make FW release 4.9
 * 
 * Fw_Integration_Devel/32   7/17/12 5:48p farshidf
 * SW3128-183: fix CableLabs OpenCable ATP 4.1.11 Combined Distortion Test
 *  (Burst Noise)
 * 
 * Fw_Integration_Devel/AP_V5_0_SYS_DEV/2   7/10/12 1:22p farshidf
 * SW3461-244: typo fix
 * 
 * Fw_Integration_Devel/AP_V5_0_SYS_DEV/1   7/10/12 1:12p farshidf
 * SW3461-244: enable sigma for 3461
 * 
 * Fw_Integration_Devel/31   7/10/12 1:10p farshidf
 * SW3461-244: Enable Sigma Calculation for 3461/2/3472
 * 
 * Fw_Integration_Devel/30   6/6/12 6:06p farshidf
 * SW3128-169: upgrade the AVS algo to version 5.2
 * 
 * Fw_Integration_Devel/29   5/29/12 12:02p farshidf
 * SW3128-167: fix the AVS detection for sigma
 * 
 * Fw_Integration_Devel/28   5/11/12 12:39p farshidf
 * SW3128-1: update the new sigma code
 * 
 * Fw_Integration_Devel/27   5/3/12 10:19a farshidf
 * SW3128-1: run avs and sigma before allowing the user to send Hab
 *  command
 * 
 * Fw_Integration_Devel/26   5/2/12 2:39p farshidf
 * SW3128-1: merge to integration
 * 
 * 36   5/2/12 2:16p farshidf
 * SW3128-1: fix compile issue
 * 
 * 35   5/2/12 1:51p farshidf
 * SW3128-1: fix the AVS and sigma calc
 * 
 * 34   4/18/12 11:03a farshidf
 * SW3128-1: upgrade to version 0.5
 * 
 * 33   4/9/12 11:59a farshidf
 * SW3128-125: merge to main
 * 
 * 32   3/23/12 3:15p farshidf
 * SW3128-125: FW version 4.6
 * 
 * 31   3/8/12 12:00a farshidf
 * SW3461-171: merge to main
 * 
 * Fw_Integration_Devel/24   4/9/12 11:49a farshidf
 * SW3128-125: protect divide by zero in AVS
 * 
 * Fw_Integration_Devel/22   3/22/12 5:33p farshidf
 * SW3128-1: reduce printf
 * 
 * Fw_Integration_Devel/21   2/27/12 4:14p farshidf
 * SW3128-1: move the sigma calc out of AVS
 * 
 * Fw_Integration_Devel/18   2/7/12 7:31p farshidf
 * SW3128-1: remove printf
 * 
 * Fw_Integration_Devel/17   2/7/12 10:48a farshidf
 * SW3128-1: print avs vaklue every 7 min
 * 
 * Fw_Integration_Devel/16   1/25/12 12:48p farshidf
 * SW3461-1: change the margin to 75mv
 * 
 * Fw_Integration_Devel/15   1/24/12 6:25p farshidf
 * SW3461-1: merge to integ
 * 
 * 28   1/24/12 6:20p farshidf
 * SW3461-1: do not avs when avs hardware not available
 * 
 * 27   1/20/12 6:52p farshidf
 * SW3461-1: merge to main
 * 
 * Fw_Integration_Devel/14   1/16/12 7:28p farshidf
 * SW3128-1: merge to integ
 * 
 * 26   1/16/12 7:27p farshidf
 * SW3128-1: fix the avs at low voltage
 * 
 * 25   1/11/12 6:08p farshidf
 * SW7425-2146: AVS algorithm needs to limit the upper and lower voltage
 *  points.
 * 
 * 24   1/11/12 4:23p farshidf
 * SW3461-1: remove the debug
 * 
 * 23   1/11/12 4:21p farshidf
 * SW3461-1: fix the avs
 * 
 * Fw_Integration_Devel/11   1/6/12 6:06p farshidf
 * SW3128-1: warning fixes
 * 
 * Fw_Integration_Devel/10   12/20/11 4:32p farshidf
 * SW3128-98: add the sigma code
 * 
 * Fw_Integration_Devel/9   12/16/11 10:35a farshidf
 * SW3461-118: update to version 0.4 of AVS
 * 
 * Fw_Integration_Devel/8   12/7/11 2:37p farshidf
 * SW3461-86: correct the 3461 chip id
 * 
 * Fw_Integration_Devel/7   12/7/11 11:13a farshidf
 * SW3128-86: latest AVS v1.0 to match the new algo
 * 
 * 20   9/15/11 2:57p farshidf
 * SW3461-1: fix the margin algo
 * 
***************************************************************************/
#include "bstd.h"
#include "bkni.h"

#include "bchp_avs_hw_mntr.h"
#include "bchp_avs_rosc_threshold_1.h"
#include "bchp_avs_rosc_threshold_2.h"
#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs_pvt_mntr_config.h"
#include "bchp_avs_ro_registers_1.h"
#include "avs_settings.h"
#if (BCHP_FAMILY==3128) || (BCHP_FAMILY==3461) || (BCHP_FAMILY==3462) || (BCHP_FAMILY==3472)
#include "avs_sigma.h"
#endif
//#include "bchp_leap_ctrl.h"
#define MAJOR_VERSION 0
#define MINOR_VERSION 54


#define AVS_DEBUG 0

static AvsContext_t avs_context[AVS_MAX_CONTEXTS];
static uint32_t num_devices = 1;
extern int AvsLogPeriod;
static int AvsLogTick = 0;

#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
#include "boardparms.h"
#endif


#ifdef BCA_DEBUG_STATS
uint32_t reg_read_count = 0;
uint32_t reg_write_count = 0;
#endif

#if 1
/* These are build options for the new features of the algorithm (they can be disabled individually) */
#define USE_SOFTWARE_TAKEOVER /* use software take over method instaed of using sequencer (averages data) */
#define USE_DAC_LIMITS        /* sets up DAC limits to prevent bad DAC writes */
#define USE_GET_BOARD_DAC     /* get approximate starting DAC value from board voltage */
#define USE_FIND_1V           /* change starting DAC value until voltage reads ~1V */
#undef USE_CHECK_VDDCMON_WARNING /* ensure that final voltage did not cause VTRAP warning to be set */
#endif


#if (BCHP_CHIP==4550) && (BCHP_VER==BCHP_VER_B0)
#define vmin_avs            INT(.770) /*0.77V*/
#define vmax_avs            INT(0.9)  /*0.9V*/
#elif (BCHP_CHIP==6318)
#define vmin_avs            INT(0.90) /*0.90V*/
#define vmax_avs            INT(1.02) /*1.02V*/
#else
#define vmin_avs            INT(0.86) /*0.86V*/
#define vmax_avs            INT(1.02) /*1.02V*/
#endif

#define vtrap_safe_low      INT(0.844)
#define vtrap_safe_high     INT(0.880)

#define vlow_ss             INT(0.94)

#define initial_voltage_offset 64 /* DAC adjustment to perform the initial local fit <--- script says 64, slides say 48 */
#define global_voltage_offset  32 /* DAC adjustment to make the local into a global fit */
#define safe_voltage_offset    64 /* DAC adjustment used to provide a range for a slope calculation that won't produce a voltage that is too low */


/* These allow definitions of oscillators we want to exclude from processing */
#if (BCHP_CHIP==6318)
#define Central_Exclude_Low   0xFFFFFFFCU /* don't exclude oscillators 0 & 1 (include) */
#define Central_Exclude_High  0xFFFFFFFFU
#define CENTRAL_OSC_HVT       0
#define CENTRAL_OSC_SVT       1
#else
#define Central_Exclude_Low   0xFFFFFFFFU
#define Central_Exclude_High  0xFFFFCFFFU /* don't exclude oscillators 44 & 45 (include) */
#define CENTRAL_OSC_HVT       44
#define CENTRAL_OSC_SVT       45
#endif
#define Remote_Exclude_Low    0U
#define Remote_Exclude_High   0U
/* Note: the above doesn't cover "holes" in the list of oscillators because these oscillator values read as zero when not implememented */

/* The number of central oscillators is constant across the different parts */
#define NUMBER_OF_CENTRAL 48

#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif

/* These are the threshold values at 0.9V, SS, 125C condition */

#if (BCHP_CHIP==4550) && (BCHP_VER==BCHP_VER_B0)
/* These new values were supplied on 5/23/12 for BCM4550-B0 only */
#define CENT_FREQ_THRESHOLD_0 INT(9.3369)
#define CENT_FREQ_THRESHOLD_1 INT(7.8520)
#define CENT_FREQ_THRESHOLD_2 INT(14.1601)
#define CENT_FREQ_THRESHOLD_3 INT(6.6772)
#define CENT_FREQ_THRESHOLD_4 INT(5.7181)
#define CENT_FREQ_THRESHOLD_5 INT(9.9292)
#define CENT_FREQ_THRESHOLD_6 INT(9.4352)
#define CENT_FREQ_THRESHOLD_7 INT(8.4552)
#define CENT_FREQ_THRESHOLD_8 INT(14.8042)
#define CENT_FREQ_THRESHOLD_9 INT(6.6798)
#define CENT_FREQ_THRESHOLD_10 INT(6.1818)
#define CENT_FREQ_THRESHOLD_11 INT(10.3479)
#define CENT_FREQ_THRESHOLD_12 INT(12.2939)
#define CENT_FREQ_THRESHOLD_13 INT(10.5406)
#define CENT_FREQ_THRESHOLD_14 INT(18.2659)
#define CENT_FREQ_THRESHOLD_15 INT(8.7556)
#define CENT_FREQ_THRESHOLD_16 INT(7.7057)
#define CENT_FREQ_THRESHOLD_17 INT(12.8317)
#define CENT_FREQ_THRESHOLD_18 INT(4.8104)
#define CENT_FREQ_THRESHOLD_19 INT(4.3770)
#define CENT_FREQ_THRESHOLD_20 INT(8.3934)
#define CENT_FREQ_THRESHOLD_21 INT(2.1784)
#define CENT_FREQ_THRESHOLD_22 INT(2.0060)
#define CENT_FREQ_THRESHOLD_23 INT(3.8176)
#define CENT_FREQ_THRESHOLD_24 INT(5.5570)
#define CENT_FREQ_THRESHOLD_25 INT(4.9436)
#define CENT_FREQ_THRESHOLD_26 INT(10.3330)
#define CENT_FREQ_THRESHOLD_27 INT(2.5950)
#define CENT_FREQ_THRESHOLD_28 INT(2.5209)
#define CENT_FREQ_THRESHOLD_29 INT(4.9313)
#define CENT_FREQ_THRESHOLD_30 INT(2.4658)
#define CENT_FREQ_THRESHOLD_31 INT(3.7134)
#define CENT_FREQ_THRESHOLD_32 INT(3.3466)
#define CENT_FREQ_THRESHOLD_33 INT(3.2017)
#define CENT_FREQ_THRESHOLD_34 INT(4.0224)
#define CENT_FREQ_THRESHOLD_35 INT(4.0014)
#define CENT_FREQ_THRESHOLD_36 INT(2.3975)
#define CENT_FREQ_THRESHOLD_37 INT(2.3793)
#define CENT_FREQ_THRESHOLD_38 INT(3.0485)
#define CENT_FREQ_THRESHOLD_39 INT(3.1807)
#define CENT_FREQ_THRESHOLD_40 INT(3.8462)
#define CENT_FREQ_THRESHOLD_41 INT(3.7271)
#define CENT_FREQ_THRESHOLD_42 INT(4.5406)
#define CENT_FREQ_THRESHOLD_43 INT(4.5412)
#define CENT_FREQ_THRESHOLD_44 INT(1.8779)
#define CENT_FREQ_THRESHOLD_45 INT(2.6914)
#define CENT_FREQ_THRESHOLD_46 INT(1.9960)
#define CENT_FREQ_THRESHOLD_47 INT(2.4202)

#define RMT_FREQ_THRESHOLD_GS INT(2.7005)
#define RMT_FREQ_THRESHOLD_GH INT(1.8653)
#else
/* These new values were supplied on 10/20/11 */
#define CENT_FREQ_THRESHOLD_0  XTAL_ADJUST(11.2564)
#define CENT_FREQ_THRESHOLD_1  XTAL_ADJUST(9.457)
#define CENT_FREQ_THRESHOLD_2  XTAL_ADJUST(16.5571)
#define CENT_FREQ_THRESHOLD_3  XTAL_ADJUST(8.42338)
#define CENT_FREQ_THRESHOLD_4  XTAL_ADJUST(7.0985)
#define CENT_FREQ_THRESHOLD_5  XTAL_ADJUST(12.1348)
#define CENT_FREQ_THRESHOLD_6  XTAL_ADJUST(11.4052)
#define CENT_FREQ_THRESHOLD_7  XTAL_ADJUST(10.239)
#define CENT_FREQ_THRESHOLD_8  XTAL_ADJUST(17.3408)
#define CENT_FREQ_THRESHOLD_9  XTAL_ADJUST(8.51238)
#define CENT_FREQ_THRESHOLD_10 XTAL_ADJUST(7.69997)
#define CENT_FREQ_THRESHOLD_11 XTAL_ADJUST(12.7556)
#define CENT_FREQ_THRESHOLD_12 XTAL_ADJUST(15.1966)
#define CENT_FREQ_THRESHOLD_13 XTAL_ADJUST(12.9541)
#define CENT_FREQ_THRESHOLD_14 XTAL_ADJUST(21.8161)
#define CENT_FREQ_THRESHOLD_15 XTAL_ADJUST(11.2785)
#define CENT_FREQ_THRESHOLD_16 XTAL_ADJUST(9.74897)
#define CENT_FREQ_THRESHOLD_17 XTAL_ADJUST(16.0114)
#define CENT_FREQ_THRESHOLD_18 XTAL_ADJUST(7.21374)
#define CENT_FREQ_THRESHOLD_19 XTAL_ADJUST(6.49054)
#define CENT_FREQ_THRESHOLD_20 XTAL_ADJUST(11.9564)
#define CENT_FREQ_THRESHOLD_21 XTAL_ADJUST(3.63372)
#define CENT_FREQ_THRESHOLD_22 XTAL_ADJUST(3.3447)
#define CENT_FREQ_THRESHOLD_23 XTAL_ADJUST(6.06651)
#define CENT_FREQ_THRESHOLD_24 XTAL_ADJUST(8.02652)
#define CENT_FREQ_THRESHOLD_25 XTAL_ADJUST(7.16555)
#define CENT_FREQ_THRESHOLD_26 XTAL_ADJUST(14.0624)
#define CENT_FREQ_THRESHOLD_27 XTAL_ADJUST(4.15304)
#define CENT_FREQ_THRESHOLD_28 XTAL_ADJUST(3.85656)
#define CENT_FREQ_THRESHOLD_29 XTAL_ADJUST(7.4188)
#define CENT_FREQ_THRESHOLD_30 XTAL_ADJUST(3.01681)
#define CENT_FREQ_THRESHOLD_31 XTAL_ADJUST(4.56524)
#define CENT_FREQ_THRESHOLD_32 XTAL_ADJUST(4.45266)
#define CENT_FREQ_THRESHOLD_33 XTAL_ADJUST(4.25414)
#define CENT_FREQ_THRESHOLD_34 XTAL_ADJUST(5.10062)
#define CENT_FREQ_THRESHOLD_35 XTAL_ADJUST(5.18132)
#define CENT_FREQ_THRESHOLD_36 XTAL_ADJUST(3.2405)
#define CENT_FREQ_THRESHOLD_37 XTAL_ADJUST(3.21587)
#define CENT_FREQ_THRESHOLD_38 XTAL_ADJUST(3.96261)
#define CENT_FREQ_THRESHOLD_39 XTAL_ADJUST(4.1298)
#define CENT_FREQ_THRESHOLD_40 XTAL_ADJUST(4.92304)
#define CENT_FREQ_THRESHOLD_41 XTAL_ADJUST(4.97474)
#define CENT_FREQ_THRESHOLD_42 XTAL_ADJUST(5.50508)
#define CENT_FREQ_THRESHOLD_43 XTAL_ADJUST(5.79556)
#define CENT_FREQ_THRESHOLD_44 XTAL_ADJUST(2.49453)
#define CENT_FREQ_THRESHOLD_45 XTAL_ADJUST(3.48776)
#define CENT_FREQ_THRESHOLD_46 XTAL_ADJUST(2.51983)
#define CENT_FREQ_THRESHOLD_47 XTAL_ADJUST(2.9149)

#define RMT_FREQ_THRESHOLD_GS XTAL_ADJUST(3.446009)
#define RMT_FREQ_THRESHOLD_GH XTAL_ADJUST(2.461833)
#endif

/* all based on 27 MHz clk */
#define ONE_MICRO_SEC       (27)
#define ONE_MILLI_SEC       (27027)
#define ONE_SEC             (27027027)

#define MAX_ITERATIONS 100 /* don't let any loops run forever */

/* Debug method of finding a hang -- save the step in an unused threshold register */
#define Got_To(step) BREG_Write32(0, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0, step);

#define AvsLog(fmt, arg...)   \
  if (AvsLogPeriod && AvsLogPeriod == AvsLogTick) { \
    printk("AVS: " fmt, ##arg); \
  }


void AvsInitializeOscs(AvsContext_t * pAvs)
{
	uint32_t reg;

#define MEAS_TIME_CONTROL 0x7F
#define COUNT_MODE 0  /* 1 is for one-edge, 0 is for 2 edges */

	reg = BREG_Read32_Single(pAvs, BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL);
	reg |= MEAS_TIME_CONTROL;
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL, reg);
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_ROSC_COUNTING_MODE, COUNT_MODE);

	/* always enable all oscillators at start-up */
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0, 0xFFFFFFFF);
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1, 0xFFFF);
	/* hardware issue means we have to clear this after enable */
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0, 0);
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1, 0);

#ifdef USE_SOFTWARE_TAKEOVER
	/* If using software takeover then disable all the threshold enables */
	BREG_Write32(pAvs, BCHP_AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_0, 0);
	BREG_Write32(pAvs, BCHP_AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_0, 0);
	BREG_Write32(pAvs, BCHP_AVS_ROSC_THRESHOLD_1_CEN_ROSC_THRESHOLD1_EN_1, 0);
	BREG_Write32(pAvs, BCHP_AVS_ROSC_THRESHOLD_2_CEN_ROSC_THRESHOLD2_EN_1, 0);
	BREG_Write32(pAvs, BCHP_AVS_ROSC_THRESHOLD_1_RMT_ROSC_THRESHOLD1_EN_0, 0);
	BREG_Write32(pAvs, BCHP_AVS_ROSC_THRESHOLD_2_RMT_ROSC_THRESHOLD2_EN_0, 0);
#endif
}

#define PVT_MON_CTRL 0x7D2683

#if (BCHP_CHIP==6318)
#define AvsSetInitialVoltage AvsSetVoltage
#define AvsClearDACvalues(p) AvsSetVoltage(p, 128)
#else
void AvsSetDac(AvsContext_t * pAvs, uint32_t dac_code)
{
#define DAC_WRITE_ENABLE  BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE_code_program_en_MASK
#define DAC_WRITE_DISABLE 0

	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, DAC_WRITE_ENABLE);
	BKNI_Sleep (2);
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE, dac_code);
	BKNI_Sleep (2);
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, DAC_WRITE_DISABLE);

	BKNI_Sleep (32); /* always give time for sequencer to sequence through all the different available data */
}

void AvsInitialSetDac(AvsContext_t * pAvs, uint32_t dac_code)
{
#define DAC_WRITE_ENABLE  BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE_code_program_en_MASK
#define DAC_WRITE_DISABLE 0

	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, DAC_WRITE_ENABLE);
	BKNI_Delay (2000);
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE, dac_code);;
	BKNI_Delay (2000);
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE, DAC_WRITE_DISABLE);
	BKNI_Delay (32000); /* always give time for sequencer to sequence through all the different available data */
}

/* This is used to undo changes we made to DAC registers when we decide we're NOT going to do AVS processing */
void AvsClearDACvalues(AvsContext_t * pAvs)
{
	/* Set the values back to initial (default) values */
	AvsInitialSetDac(pAvs, 0);
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE, 0);
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE, 0x3FF);
}

/* This is used to program the DAC to get a desired voltage */
void AvsSetInitialVoltage(AvsContext_t * pAvs, uint32_t dac_code)
{
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, PVT_MON_CTRL);
	AvsInitialSetDac(pAvs, dac_code);
#if (BCHP_CHIP==6802)
   pAvs->AvsFlushReading = true;
#endif
}
#endif

#if (BCHP_CHIP==6318)
#include "bchp_avs_asb_registers.h"

void AvsSetVoltage(AvsContext_t * pAvs, uint32_t dac_code)
{
   int32_t target;

   /* Save the dac code because on 6318, it is not possible to read it back */
   pAvs->saved_dac_code = dac_code;

   /* Transform the dac_code to range -128,127, where 32 is the default setting */
   target = 64 - (int)dac_code/4; /* 64 - 128/4 == 32 */
   if (target < -32) {
      target = -32;
   } else if (target > 64) {
      target = 64;
   }

   /* Write target voltage on through ASB */
   BREG_Write32(pAvs, BCHP_AVS_ASB_REGISTERS_ASB_ADDRESS, BCHP_AVS_ASB_REGISTERS_ASB_ADDRESS_vreg_1p0);
   BREG_Write32(pAvs, BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_EN, target & 0xff);
   BREG_Write32(pAvs, BCHP_AVS_ASB_REGISTERS_ASB_COMMAND, BCHP_AVS_ASB_REGISTERS_ASB_COMMAND_start|BCHP_AVS_ASB_REGISTERS_ASB_COMMAND_read_write);
   BKNI_Sleep (32); /* Let the device notice the rising edge of _start bit and let sequencer go through all available data */
   BREG_Write32(pAvs, BCHP_AVS_ASB_REGISTERS_ASB_COMMAND, BCHP_AVS_ASB_REGISTERS_ASB_COMMAND_read_write);

   pAvs->AvsFlushReading = true;
}
#else
/* This is used to program the DAC to get a desired voltage */
void AvsSetVoltage(AvsContext_t * pAvs, uint32_t dac_code)
{
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, PVT_MON_CTRL);
	AvsSetDac(pAvs, dac_code);

#if (BCHP_CHIP==6802)
   pAvs->AvsFlushReading = true;
#endif
}
#endif

#define PVT_TEMPERATURE 0
#define PVT_PROCESS     1
#define PVT_0P99V       2
#define PVT_1P10V_0     3
#define PVT_1P10V_1     4
#define PVT_2p75V       5
#define PVT_3p63V       6
#define PVT_TESTMODE    7

#ifdef USE_SOFTWARE_TAKEOVER
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
static uint32_t sw_takeover_measure(AvsContext_t * pAvs, int sensor_id)
{
	int i,j;
	uint32_t reg, busy=1, code=0, sum=0;

   if (pAvs->AvsFlushReading) {
       pAvs->SwTakeoverVars.index = 0;
       pAvs->SwTakeoverVars.num = AVERAGE_LOOPS;
       pAvs->AvsFlushReading = false;
   } else {
       pAvs->SwTakeoverVars.num = 1;
       if (++(pAvs->SwTakeoverVars.index)==AVERAGE_LOOPS) {
          pAvs->SwTakeoverVars.index = 0;
       }
   }
	for (i=pAvs->SwTakeoverVars.index; i<pAvs->SwTakeoverVars.index+pAvs->SwTakeoverVars.num; i++)
	{
		busy = 1;
		while (busy) {
			/* These steps need to be done one-at-a-time */
			reg = 0;
			reg |= BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK;
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
			BKNI_Sleep (10); /* delay needed after indicating takeover */
			reg |= sensor_id << BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_sensor_idx_SHIFT;
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
			reg |= BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK;
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
			BKNI_Sleep (1); /* delay a bit before checking busy status */

			/* The busy comes on during processing and goes off when done */
			for (j=0; j<MAX_ITERATIONS; j++) 
			{
				busy = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY) & BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY_busy_MASK;
				if (!busy) break;
				BKNI_Sleep (1); /* delay a bit before checking again! */
			}

			/* We sometimes see the busy stuck on for some reason (if its still on, dismiss this data) */
			if (busy) {
				BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, 0);
			}
		}

		/* Note: don't make this a switch statement -- compiler creates initialized data that isn't allowed in this build */
		     if (sensor_id == PVT_1P10V_0)     code = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS);
		else if (sensor_id == PVT_2p75V)       code = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS);

		code &= BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS_data_MASK; /* these all have the same data mask */
		pAvs->SwTakeoverVars.code_array[i] = code;

		reg = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS);
		reg &= ~BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK;
		BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
	}
   
	/* Compute average to dismiss the variance in the reads */
	for (i=0; i<AVERAGE_LOOPS; i++) {
      sum += pAvs->SwTakeoverVars.code_array[i];
	}
   code = sum/AVERAGE_LOOPS;
   if ((sum - code*AVERAGE_LOOPS) >= 5) code++; /*this rounds up the result*/

	reg = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS);
	reg &= ~BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK;
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);

	return code;
}
#else
static uint32_t sw_takeover_measure(AvsContext_t * pAvs, int sensor_id)
{
	int i,j, count=0;
	uint32_t reg, busy=1, code=0, sum=0;
#define AVERAGE_LOOPS 26
	for (i=0; i<AVERAGE_LOOPS; i++)

	{
		/* These steps need to be done one-at-a-time */
		reg = 0;
		reg |= BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK;
		BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
#if (BCHP_CHIP==6318)
		BKNI_Sleep (10); /* delay needed after indicating takeover */
#endif
		reg |= sensor_id << BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_sensor_idx_SHIFT;
		BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
		reg |= BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK;
		BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);

#if (BCHP_CHIP==6318)
		BKNI_Sleep (1); /* delay a bit before checking busy status */
#else
		BKNI_Delay (500); /* delay a bit before checking busy status */
#endif
		/* The busy comes on during processing and goes off when done */
		for (j=0; j<MAX_ITERATIONS; j++) 
		{
			busy = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY) & BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY_busy_MASK;
			if (!busy) break;
			//BKNI_Sleep (1); /* delay a bit before checking again! */
		}

		/* We sometimes see the busy stuck on for some reason (if its still on, dismiss this data) */
		if (busy) {
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, 0);
			continue;
		}
#if (AVERAGE_LOOPS > 10)
		if (i < 10) continue; /* ignore first 10 valid samples (something about setteling?) */
#endif
		/* Note: don't make this a switch statement -- compiler creates initialized data that isn't allowed in this build */
		     if (sensor_id == PVT_1P10V_0)     code = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS);
		else if (sensor_id == PVT_2p75V)       code = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS);
#if (BCHP_CHIP==4550)
      else if (sensor_id == PVT_1P10V_1)     code = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS);   
#endif   
#if 0
		else if (sensor_id == PVT_TEMPERATURE) code = BREG_Read32(0, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS);
		else if (sensor_id == PVT_PROCESS)     code = BREG_Read32(0, BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS);
		else if (sensor_id == PVT_0P99V)       code = BREG_Read32(0, BCHP_AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS);
		else if (sensor_id == PVT_1P10V_1)     code = BREG_Read32(0, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS);
		else if (sensor_id == PVT_3p63V)       code = BREG_Read32(0, BCHP_AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS);
#endif

		code &= BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS_data_MASK; /* these all have the same data mask */

		sum += code;
		count++;

		reg = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS);
		reg &= ~BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK;
		BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
#if 0
		break; /* enable this to just read it once */
#endif
	}

	/* Use an average value to dismiss the variance in the reads */
    if (count) {
        code = sum/count; 
        if ((sum - code*count) >= 5) code++; /*this rounds up the result*/
	}

	reg = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS);
	reg &= ~BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK;
	BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);



	return code;
}
#endif
#endif /*USE_SOFTWARE_TAKEOVER*/


#if defined USE_DAC_LIMITS && defined USE_GET_BOARD_DAC
static void set_dac_code_limits(AvsContext_t * pAvs, uint32_t VFB_pin_voltage)
{
	unsigned range;
	uint32_t code;

    /* don't set any MIN/MAX if less than minimum */
    if (VFB_pin_voltage <= INT(.60)) {
        //AvsDebugPrintString("WARNING: VREF outside design (below min)\n");
        BDBG_MSG(("AVS VREF LOW"));
        return; 
    }
    /* and don't set any MIN/MAX if above the maximum */
    if (VFB_pin_voltage > INT(.90)) {
        //AvsDebugPrintString("WARNING: VREF outside design (above max)\n");
         BDBG_MSG(("AVS VREF HIGH"));
        return;
    }

#define DAC_CODE_MAX_START 800
#define DAC_CODE_MIN_START 180
#define DAC_CODE_INCREMENT 20

	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE, DAC_CODE_MAX_START);

	range = (VFB_pin_voltage - INT(.60))/INT(.05);
	code = DAC_CODE_MIN_START + (range * DAC_CODE_INCREMENT);	
	BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE, code);
}
#endif /*USE_DAC_LIMITS*/


#define COUNTER_MAX     32767U
#define EDGE_COUNT      2U

/* This returns the requested central oscillator threshold value (they each have a threshold) */
static uint32_t AvsReadCentOscThresholds(unsigned osc_num)
{
	/* The reason we spell this out is so that when the threshold values are variables the macro would fail */
	/* Note: don't make this into a switch statement -- compiler can create initialized data which isn't allowed */
#if (BCHP_CHIP==6318)
    if (osc_num ==  0) return (CENT_FREQ_THRESHOLD_44);
    if (osc_num ==  1) return (CENT_FREQ_THRESHOLD_45);
#endif
#if 0
    if (osc_num ==  0) return (CENT_FREQ_THRESHOLD_0);
    if (osc_num ==  1) return (CENT_FREQ_THRESHOLD_1);
    if (osc_num ==  2) return (CENT_FREQ_THRESHOLD_2);
    if (osc_num ==  3) return (CENT_FREQ_THRESHOLD_3);
    if (osc_num ==  4) return (CENT_FREQ_THRESHOLD_4);
    if (osc_num ==  5) return (CENT_FREQ_THRESHOLD_5);
    if (osc_num ==  6) return (CENT_FREQ_THRESHOLD_6);
    if (osc_num ==  7) return (CENT_FREQ_THRESHOLD_7);
    if (osc_num ==  8) return (CENT_FREQ_THRESHOLD_8);
    if (osc_num ==  9) return (CENT_FREQ_THRESHOLD_9);
    if (osc_num == 10) return (CENT_FREQ_THRESHOLD_10);
    if (osc_num == 11) return (CENT_FREQ_THRESHOLD_11);
    if (osc_num == 12) return (CENT_FREQ_THRESHOLD_12);
    if (osc_num == 13) return (CENT_FREQ_THRESHOLD_13);
    if (osc_num == 14) return (CENT_FREQ_THRESHOLD_14);
    if (osc_num == 15) return (CENT_FREQ_THRESHOLD_15);
    if (osc_num == 16) return (CENT_FREQ_THRESHOLD_16);
    if (osc_num == 17) return (CENT_FREQ_THRESHOLD_17);
    if (osc_num == 18) return (CENT_FREQ_THRESHOLD_18);
    if (osc_num == 19) return (CENT_FREQ_THRESHOLD_19);
    if (osc_num == 20) return (CENT_FREQ_THRESHOLD_20);
    if (osc_num == 21) return (CENT_FREQ_THRESHOLD_21);
    if (osc_num == 22) return (CENT_FREQ_THRESHOLD_22);
    if (osc_num == 23) return (CENT_FREQ_THRESHOLD_23);
    if (osc_num == 24) return (CENT_FREQ_THRESHOLD_24);
    if (osc_num == 25) return (CENT_FREQ_THRESHOLD_25);
    if (osc_num == 26) return (CENT_FREQ_THRESHOLD_26);
    if (osc_num == 27) return (CENT_FREQ_THRESHOLD_27);
    if (osc_num == 28) return (CENT_FREQ_THRESHOLD_28);
    if (osc_num == 29) return (CENT_FREQ_THRESHOLD_29);
    if (osc_num == 30) return (CENT_FREQ_THRESHOLD_30);
    if (osc_num == 31) return (CENT_FREQ_THRESHOLD_31);
    if (osc_num == 32) return (CENT_FREQ_THRESHOLD_32);
    if (osc_num == 33) return (CENT_FREQ_THRESHOLD_33);
    if (osc_num == 34) return (CENT_FREQ_THRESHOLD_34);
    if (osc_num == 35) return (CENT_FREQ_THRESHOLD_35);
    if (osc_num == 36) return (CENT_FREQ_THRESHOLD_36);
    if (osc_num == 37) return (CENT_FREQ_THRESHOLD_37);
    if (osc_num == 38) return (CENT_FREQ_THRESHOLD_38);
    if (osc_num == 39) return (CENT_FREQ_THRESHOLD_39);
    if (osc_num == 40) return (CENT_FREQ_THRESHOLD_40);
    if (osc_num == 41) return (CENT_FREQ_THRESHOLD_41);
    if (osc_num == 42) return (CENT_FREQ_THRESHOLD_42);
    if (osc_num == 43) return (CENT_FREQ_THRESHOLD_43);
#endif
	/* The current algorithm only uses these central oscillators */
    if (osc_num == 44) return (CENT_FREQ_THRESHOLD_44);
    if (osc_num == 45) return (CENT_FREQ_THRESHOLD_45);
#if 0
    if (osc_num == 46) return (CENT_FREQ_THRESHOLD_46);
    if (osc_num == 47) return (CENT_FREQ_THRESHOLD_47);
#endif
	return 0;
}

/* This returns the requested remote oscillator threshold value (they share a threshold based on the type) */
static uint32_t AvsReadRmtOscThresholds(unsigned osc_num)
{
	if (osc_num & 1)
		return (RMT_FREQ_THRESHOLD_GH);
	else
		return (RMT_FREQ_THRESHOLD_GS);
}

#ifdef USE_SOFTWARE_TAKEOVER
/* Return the current voltage value generated by the PVTMON */
uint32_t AvsReadVoltage(AvsContext_t * pAvs)
{
	uint32_t voltage_1p1_0, V_1p1_0;

#if (BCHP_CHIP==4550)
	V_1p1_0 = sw_takeover_measure(pAvs, PVT_1P10V_1);
#else
	V_1p1_0 = sw_takeover_measure(pAvs, PVT_1P10V_0);
#endif
	voltage_1p1_0 = (V_1p1_0 * INT(.99) * 8) / (7*1024);


	return voltage_1p1_0;
}
#else
/* Return the current voltage value generated by the PVTMON */
static uint32_t AvsReadVoltage(AvsContext_t * pAvs)
{
	uint32_t voltage_1p1_0, V_1p1_0;

	V_1p1_0 = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS_data_MASK;
	voltage_1p1_0 = (V_1p1_0 * INT(.99) * 8) / (7*1024);

	return voltage_1p1_0;
}
#endif /*USE_SOFTWA */

/* Return the current oscillator value for the specified oscillator */
static  uint32_t AvsReadCentOsc(AvsContext_t * pAvs, unsigned osc_num)
{ 
	return BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0 + (osc_num * 4)) & BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0_data_MASK;
}
static uint32_t AvsReadRmtOsc(AvsContext_t * pAvs, unsigned osc_num)
{ 
	return BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0 + (osc_num * 4)) & BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0_data_MASK;
}

/* Return the current oscillator frequency for the specified oscillator */
static uint32_t AvsReadCentOscFreq(AvsContext_t * pAvs, unsigned osc_num)
{
	uint32_t cent = AvsReadCentOsc(pAvs, osc_num);
    return ((2*AVS_XTAL_FACTOR*cent)*DIVIDER_DEFINE)/(COUNTER_MAX*EDGE_COUNT)*DIVIDER_ADJUSTMENT;
}
 uint32_t AvsReadRmtOscFreq(AvsContext_t * pAvs, unsigned osc_num)
{
	uint32_t rmt = AvsReadRmtOsc(pAvs, osc_num);
    return ((AVS_XTAL_FACTOR*rmt)*DIVIDER_DEFINE)/(COUNTER_MAX*EDGE_COUNT)*DIVIDER_ADJUSTMENT;
}

#ifdef USE_GET_BOARD_DAC
#if (BCHP_CHIP==6318)
uint32_t GetBoardDACvalue(AvsContext_t * pAvs)
{
   uint32_t board_dac_code, V_1p1_0;
#ifdef USE_DAC_LIMITS
   uint32_t VFB_pin_voltage;
#endif

   BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, PVT_MON_CTRL);
#ifdef USE_SOFTWARE_TAKEOVER
   V_1p1_0 = sw_takeover_measure(pAvs, PVT_1P10V_0);
#else
   V_1p1_0 = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS_data_MASK;
#endif
   board_dac_code = 1024 - V_1p1_0 * INT(.99) / INT(1.0); /* v*(.99/1.0) */
#ifdef USE_DAC_LIMITS
   VFB_pin_voltage = V_1p1_0 * INT(.99) / 1024; /* v*(.99/1024) */
   set_dac_code_limits(pAvs, VFB_pin_voltage); /* set over-voltage_protection */
#endif
   return board_dac_code;
}
#else
#define PVT_MON_CTRL_TEST 0x7D2403
/* This uses the PVT monitor to generate a DAC value based on the voltage BEFORE enabling AVS */
uint32_t GetBoardDACvalue(AvsContext_t * pAvs)
{
   uint32_t board_dac_code, V_1p1_0;
#ifdef USE_DAC_LIMITS
   uint32_t VFB_pin_voltage;
#endif

   BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, PVT_MON_CTRL_TEST);
   BKNI_Delay (30000); /* let the PVT monitor cycle through the different samples */
#ifdef USE_SOFTWARE_TAKEOVER
   V_1p1_0 = sw_takeover_measure(pAvs, PVT_2p75V); /* using 2P75V PVTMON's input to collect any PVTMON measurements */
#else
   V_1p1_0 = BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS_data_MASK;
#endif
   board_dac_code = V_1p1_0 * INT(.99) / INT(2.5); /* v*(.99/2.5) */
#ifdef USE_DAC_LIMITS
   VFB_pin_voltage = V_1p1_0 * INT(.99) / 1024; /* v*(.99/1024) */
   set_dac_code_limits(pAvs, VFB_pin_voltage); /* set over-voltage_protection */
#endif

   /* Restore PVTMON to default configuration */
   BREG_Write32(pAvs, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, PVT_MON_CTRL);
   return board_dac_code;
}
#endif
#endif /*USE_GET_BOARD_DAC*/



/* This adjusts the DAC until the voltage reads ~1V */
#ifdef USE_FIND_1V
uint32_t Find1V(AvsContext_t * pAvs, uint32_t board_dac_code, bool sigma_enabled)
{
#if (BCHP_CHIP==4550)
#define TARGET_VOLTAGE 0.9
#else
#define TARGET_VOLTAGE 1.0
#endif
   unsigned i;
   uint32_t voltage=0, cur_dac_code = board_dac_code;
   uint32_t last_voltage=0;
   uint32_t voltage_was=0;
   int32_t step_size;

   for (i=0;i<MAX_ITERATIONS;i++)
   {
      if (!sigma_enabled)
         AvsSetInitialVoltage(pAvs, cur_dac_code);

      voltage = AvsReadVoltage(pAvs);

      /* If I'm in the ball park, then close enough ... */
#define SWING_VALUE (.005)
      if (voltage >= INT(TARGET_VOLTAGE - SWING_VALUE) && voltage <= INT(TARGET_VOLTAGE + SWING_VALUE)) break;
      if (i==0) voltage_was = voltage; /*save the original for display purposes*/

      /* Dynamically calculate a step size based on how far away we are from the target */
      /* note that current voltage may be above 1V so result can be negative */
#if (BCHP_CHIP==6318)
#define MIN_VOLTAGE_STEP_SIZE INT(.00078125) /* regulator target step is 0.003125; step/4 = 0.00078125 */
#else
#define MIN_VOLTAGE_STEP_SIZE INT(.00056) /* calculated based on the minimum resolution of the DAC and the system design */
#endif

      step_size = ((signed)INT(TARGET_VOLTAGE) - (signed)voltage) / MIN_VOLTAGE_STEP_SIZE;

      //if (!step_size) { result='2'; break; } /* this should never happen as the "close enough" test above should have caught this */
      cur_dac_code -= step_size;
      last_voltage = voltage;

      /* Don't let the DAC exceed the limits - this also works for setting less than zero because it wraps */
      /* This can happen if we can still affect the voltage with DAC changes but reach a DAC value of zero before it reaches 1V. */
      /* We don't want to EVER set the DAC value to zero, so give it something above that. */
      /* Note: returning a "small" value also causes the board to reset -- so use original value. */
      if (cur_dac_code > BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_dac_code_MASK)
      {
         cur_dac_code = board_dac_code; /* return this instead */
         break;
      }
   }

   return cur_dac_code;
}

#endif /*USE_FIND_1V*/

#if 0  /* does not apply to Leap chip */ 
#ifdef USE_CHECK_VDDCMON_WARNING
/* The VTRAP status gets set when the voltage goes too low -- use this to clear the status before re-checking */
static void Clear_VTRAP_Warning(void)
{
    uint32_t ctrl;
    ctrl = BREG_Read32(0, BCHP_SUN_TOP_CTRL_VTRAP_CTRL);
    ctrl |= BCHP_SUN_TOP_CTRL_VTRAP_CTRL_vtrap_threshold_warning_status_clear_MASK;
    BREG_Write32(0, BCHP_SUN_TOP_CTRL_VTRAP_CTRL, ctrl);
}
#endif
/* This verifies that we didn't set the voltage too low that the VTRAP warning was set. */
/* We start at a "safe" voltage and lower it until we reach the converged voltage. */
/* We stop alon the way if we see the VTRAP warning set. */
static uint32_t Check_VDDCMON_Warning(uint32_t safe_dac_code, uint32_t cur_dac_code)
{
    int i;
    uint32_t status;

	/* If we don't clear this before checking we could find it set on some parts (i.e. 7231/B0) */
	Clear_VTRAP_Warning();

    status = BREG_Read32(0, BCHP_SUN_TOP_CTRL_VTRAP_STATUS);
    status &= BCHP_SUN_TOP_CTRL_VTRAP_STATUS_vtrap_threshold_warning_status_MASK;
	if (status) 
	{
		Clear_VTRAP_Warning();

    	status = BREG_Read32(0, BCHP_SUN_TOP_CTRL_VTRAP_STATUS);
    	status &= BCHP_SUN_TOP_CTRL_VTRAP_STATUS_vtrap_threshold_warning_status_MASK;
	}

#define VDDCMON_DAC_ADJUSTMENT 16 /* raise and lower the DAC by this amount to clear VTRAP warning */

    /* This lowers the voltage until we reach the converged dac value or the warning happens */
    for (i=0; !status && (safe_dac_code < cur_dac_code) && i<MAX_ITERATIONS; i++)
    {
        safe_dac_code += VDDCMON_DAC_ADJUSTMENT; /* lower the voltage by raising the DAC value */

        AvsSetVoltage(safe_dac_code);
		/* Do we need this?  This is sleeping an additional amount after setting new DAC (that already sleeps) */
        BKNI_Sleep (30);

        status = BREG_Read32(0, BCHP_SUN_TOP_CTRL_VTRAP_STATUS) & BCHP_SUN_TOP_CTRL_VTRAP_STATUS_vtrap_threshold_warning_status_MASK;
    }

    /* If VTRAP status never hit then leave the DAC where it was (i.e. put it back) */
    if (!status)
    {
        AvsSetVoltage(cur_dac_code);
        BKNI_Sleep (30);
        return cur_dac_code;
    }

	/* Back up so that the VTRAP status flag is no longer set */
#if 0
	/* This version backs up one step (it should be gone after this) */
    safe_dac_code -= VDDCMON_DAC_ADJUSTMENT; 
    AvsSetVoltage(safe_dac_code);
    BKNI_Sleep (30);

	Clear_VTRAP_Warning();
#else
    /* This version raises the voltage until the warning status goes away */
    for (i=0;i<MAX_ITERATIONS;i++) 
    {
        status = BREG_Read32(0, BCHP_SUN_TOP_CTRL_VTRAP_STATUS) & BCHP_SUN_TOP_CTRL_VTRAP_STATUS_vtrap_threshold_warning_status_MASK;
        if (!status) break; /* stop when the warning disappears */

        safe_dac_code -= VDDCMON_DAC_ADJUSTMENT; /* raise the voltage by lowering the DAC value */
        AvsSetVoltage(safe_dac_code);

		Clear_VTRAP_Warning();
        BKNI_Sleep (30);
    }
#endif

    return safe_dac_code;
}
#endif /*USE_CHECK_VDDCMON_WARNING*/

/* This is the predictive algorithm.  It generates a predicted voltage... */
static uint32_t AvsPredict(AvsContext_t * pAvs, int pass, uint32_t dac_code_low, uint32_t dac_code_high, int32_t *vlow, int32_t *vhigh)
{
	unsigned i;
	int32_t Vmax, Vconv;
	int32_t first_cent_osc_high[NUMBER_OF_CENTRAL], first_rmt_osc_high[NUMBER_OF_REMOTE];
	int32_t threshold_value, first_osc_value_high, second_osc_value_low;
	int32_t slope, voltage;

	/* Record the oscillator frequency values at the first voltage (low DAC is higher voltage) */
	AvsSetInitialVoltage(pAvs, dac_code_low);
	*vhigh = AvsReadVoltage(pAvs);

	for (i=0; i<NUMBER_OF_CENTRAL; i++) {
		if ((i<32 && (Central_Exclude_Low & (1U<<i))) || (i>=32 && (Central_Exclude_High & (1U<<(i-32)))))
			first_cent_osc_high[i] = 0;
		else
			first_cent_osc_high[i] = AvsReadCentOscFreq(pAvs, i);
	}
	for (i=0; i<NUMBER_OF_REMOTE; i++) {
		if ((i<32 && (Remote_Exclude_Low & (1U<<i))) || (i>=32 && (Remote_Exclude_High & (1U<<(i-32)))))
			first_rmt_osc_high[i] = 0;
		else
			first_rmt_osc_high[i] = AvsReadRmtOscFreq(pAvs, i);
	}
	/* Note: if the oscillator wasn't implemented then reading the oscillator value will return 0 and we'll treat it like excluded */

	/* Switch to the second voltage and record the oscillator values (high DAC is lower voltage) */
	AvsSetInitialVoltage(pAvs, dac_code_high);
	*vlow = AvsReadVoltage(pAvs);


	/* Verify that AVS hardware is enabled on this board! */
	/* If the difference in the two voltages is < 10mV then we don't do AVS on this part! */
	//if (pass == 0)

	if ((*vhigh - *vlow) < INT(.010)) {
		//AvsDebugPrintString("Voltage difference was less than 10mV -- stopping AVS processing!\n");
		 BDBG_MSG(("AVS NOT FOUND"));
		AvsClearDACvalues(pAvs);
		return 0;
	}
	if ((*vhigh - *vlow) == 0) { BDBG_MSG(("AVS/0 4")); return 0;}

	Vmax = 0; /* we want the largest predicted voltage across all the usable oscillators */
	for (i=0; i<NUMBER_OF_CENTRAL; i++)
	{
		if (!first_cent_osc_high[i]) continue; /*if we didn't get a value then this is an unimplemented oscillator*/
		first_osc_value_high = first_cent_osc_high[i];
		second_osc_value_low = AvsReadCentOscFreq(pAvs, i);
		threshold_value = AvsReadCentOscThresholds(i);
		slope = (first_osc_value_high - second_osc_value_low)*S2 / (*vhigh - *vlow);
		if (slope == 0) {BDBG_MSG(("AVS/0 3"));  return 0;}
		voltage = ((threshold_value - second_osc_value_low)*S2)/slope + *vlow;

		if (voltage > Vmax) Vmax = voltage;
	}
	for (i=0; i<NUMBER_OF_REMOTE; i++)
	{
		if (!first_rmt_osc_high[i]) continue; /*if we didn't get a value then this is an unimplemented oscillator*/
		first_osc_value_high = first_rmt_osc_high[i];
		second_osc_value_low = AvsReadRmtOscFreq(pAvs, i);
		threshold_value = AvsReadRmtOscThresholds(i);
		slope = (first_osc_value_high - second_osc_value_low)*S2 / (*vhigh - *vlow);
		if (slope == 0) {BDBG_MSG(("AVS/0 1"));  return 0;}
		voltage = ((threshold_value*S2) - ((second_osc_value_low*S2) - (slope * (*vlow))))/slope;

		if (voltage > Vmax) Vmax = voltage;
	} 
	Vconv = Vmax;

	if (S2>1000) Vmax /= (S2/1000);

	return Vconv;
}

static uint32_t Find_final_avs_voltage(AvsContext_t * pAvs, bool sigma_enabled)
{
	unsigned pass;
	int32_t vlow, vhigh;
	int32_t slope_dac;
	uint32_t dac_code_low, dac_code_high;
	uint32_t board_dac_code, cur_dac_code;
	int32_t vsum, vmax;
	int32_t slope_avs, intercept_avs, vmargin, vconv;
	int32_t avs_dac_code, safe_dac_code=0;

	/* Step 1 from slides: */
#ifdef USE_GET_BOARD_DAC
	board_dac_code = GetBoardDACvalue(pAvs);
#else
	board_dac_code = First_dac_code; /* just start at the old starting point */
#endif
	//V_brd = AvsReadVoltage();
	/* Step 2 from slides: */
#ifdef USE_FIND_1V
	cur_dac_code = Find1V(pAvs, board_dac_code, sigma_enabled);
#else
	cur_dac_code = board_dac_code;
#endif
	//V_brd = AvsReadVoltage();
	/* Step 3 from slides: */
	dac_code_low  = cur_dac_code;
	//V1 = cur_voltage or V_brd

	/* Step 4 from slides: */
	dac_code_high = dac_code_low + initial_voltage_offset;
	//V2 = V_brd - 30mV
	if (((vmax_avs - pAvs->Vmin_Avs) + (pAvs->vmarginL - pAvs->vmarginH)) == 0) { BDBG_MSG(("AVS/0 5")); return 0;}
	/* First pass uses rough values (local fit) and the second uses the more refined (wider range) values (global fit) */
	for (pass=0; pass<2; pass++)
	{  
		vconv = AvsPredict(pAvs, pass, dac_code_low, dac_code_high, &vlow, &vhigh);
		if (!vconv) return 0;
		slope_avs = ((pAvs->vmarginH - pAvs->vmarginL)*S2)/((vmax_avs - pAvs->Vmin_Avs) + (pAvs->vmarginL - pAvs->vmarginH));
		intercept_avs = pAvs->vmarginL - ((slope_avs * (pAvs->Vmin_Avs - pAvs->vmarginL))/S2);
		vmargin = (slope_avs * vconv)/S2 + intercept_avs;
		vsum = vconv + vmargin;

		if (vsum < pAvs->Vmin_Avs)
			vsum = pAvs->Vmin_Avs;
		else if (vsum > vmax_avs)
			vsum = vmax_avs;

		vmax = vsum;

		/* Calculate the slope of the DAC for the different measured voltages */
		/* This will be used to generate the DAC value needed to reach a specific voltage value */
		if ((vlow - vhigh) == 0) { BDBG_MSG(("AVS/0 6")); return 0;}
		slope_dac = ((signed)(dac_code_high - dac_code_low)*S2) / (vlow - vhigh);
		/*BDBG_MSG(("Slope_dac = %d ", slope_dac));*/
/* Since we're now adjusting the voltage until it reads ~1V this shouldn't be necessary anymore (dac_code_low should already be at 1V) */
#if 1
		/* On next pass, find the calculated DAC value at 1V instead of using the measured value */
		dac_code_low = ((slope_dac * INT(1.0)) - (slope_dac * vhigh))/S2 + dac_code_low;
#endif

		avs_dac_code = ((slope_dac * vmax) - (slope_dac * vhigh))/S2 + dac_code_low;

		/* On next pass, adjust DAC high value to get more precise calculation on second pass */
		/* this turns the process from a local fit to a global fit process */
		if (vmax < vlow_ss) 
		{
			dac_code_high = avs_dac_code - global_voltage_offset;
		} 
	}
#define DAC_STEP_SIZE 10
	avs_dac_code -= 2*DAC_STEP_SIZE;
#if 0  /* does not apply to 3128/3461 since there is no VTRAP/security proicessor*/
	if (vmax > vtrap_safe_low && vmax < vtrap_safe_high)
		safe_dac_code = avs_dac_code - safe_voltage_offset;
	else
#endif
		safe_dac_code = avs_dac_code;

	AvsSetInitialVoltage(pAvs, safe_dac_code);

#ifdef USE_CHECK_VDDCMON_WARNING
	vlow = AvsReadVoltage(pAvs);
	avs_dac_code = Check_VDDCMON_Warning(safe_dac_code, avs_dac_code);
#endif

	vlow = AvsReadVoltage(pAvs);
	 if (S2>1000) 
	 {
		 vlow /= (S2/1000);
		 vconv /= (S2/1000);
	 }

	BDBG_MSG(("vlow= %d  vconv = %d", vlow, vconv));
	return vconv;
}

/*************************************************************
*   Adjust the DAC code
**************************************************************/
/* Adjust the DAC (and therefore the voltage) by the specified step value */
static bool AvsAdjustDacCode(AvsContext_t * pAvs, int adjustment_step)
{
	uint32_t cur_val, new_val=0;
	uint32_t voltage_1p1_0;
	bool result = false;
	
#if (BCHP_CHIP==6318)
   cur_val = pAvs->saved_dac_code;
#else
	cur_val= BREG_Read32(pAvs,  BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE); 
#endif
	voltage_1p1_0 = AvsReadVoltage(pAvs);

	/* Make sure we never violate the voltage max and min values */
	/* Note that the adjustment step can be negative to adjust the other way */
	if ((pAvs->Vmin_Avs <= voltage_1p1_0) && (voltage_1p1_0 <= vmax_avs)) 
	{
		
		new_val = cur_val + adjustment_step;
		/*BDBG_MSG(("Setting new DAC value = %d (was %d, step=%d)", new_val, cur_val, adjustment_step));*/
		result = true;
	}
    else
    {
        /* new algorithm says we shouldn't ALLOW the voltage to exceed its limits */
        /* So we know we're outside the safe voltage range -- bring it back */
        if (voltage_1p1_0 > vmax_avs)
            new_val = cur_val + abs(adjustment_step); /* increase DAC to lower voltage */
        else
            new_val = cur_val - abs(adjustment_step); /* decrease DAC to increase voltage */
 
        BDBG_MSG(("Voltage exceeding limit -- correcting (voltage=%d)", voltage_1p1_0));
        /*BDBG_MSG(("Setting new DAC value = %d (was %d, step=%d)", new_val, cur_val, adjustment_step));*/
        result = true;
    }

	if (new_val == 512) new_val += 1; /* apparently using 512 is bad */
	if (new_val == cur_val) return result; /* optimization -- don't write register with same value */

	AvsSetInitialVoltage(pAvs, new_val);
	return result;
}

/*************************************************************
*  AvsReadCentralOscThresholds
**************************************************************/
/* Return the threshold values for the specified central oscillator */
static void AvsReadCentralOscThresholds(AvsContext_t * pAvs, unsigned oscillator, int32_t *reg_min, int32_t *reg_max)
{
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
	if (oscillator & 1) {
		*reg_min = pAvs->lower_svt_threshold/2;
		*reg_max = pAvs->upper_svt_threshold/2;
	} else {
		*reg_min = pAvs->lower_hvt_threshold/2;
		*reg_max = pAvs->upper_hvt_threshold/2;
	}
#else
	*reg_min = BREG_Read32(pAvs, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (oscillator*4));
	*reg_max = BREG_Read32(pAvs, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (oscillator*4));
#endif
}

/*************************************************************
*  AvsReadRemoteOscThresholds
**************************************************************/
/* Return the threshold values for the specified remote oscillator */
static void AvsReadRemoteOscThresholds(AvsContext_t * pAvs, unsigned oscillator, int32_t *reg_min, int32_t *reg_max)
{
	/* The even status values are for the GS thresholds and the odd are for the GH (not documented in the RDB!) */
	/* This might need adjusting by the TPYE of the oscillator... */
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
	if (oscillator & 1) {
		*reg_min = pAvs->lower_hvt_threshold;
		*reg_max = pAvs->upper_hvt_threshold;
	} else {
		*reg_min = pAvs->lower_svt_threshold;
		*reg_max = pAvs->upper_svt_threshold;
	}
#else
	if (oscillator & 1) {
		*reg_min = BREG_Read32(pAvs, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H);
		*reg_max = BREG_Read32(pAvs, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H);
	} else {
		*reg_min = BREG_Read32(pAvs, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S);
		*reg_max = BREG_Read32(pAvs, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S);
	}
#endif
}

/*************************************************************
*   Adjust the Vddc
**************************************************************/
static bool AvsAdjustVddc (AvsContext_t * pAvs)
{
	bool increase=false, found_one=false, failed=false;
	uint32_t i;
	int32_t current_osc_val, lower=0, upper=0;

	for (i=0; i<NUMBER_OF_CENTRAL; i++)
	{
		if ((i<32 && (Central_Exclude_Low & (1U<<i))) || (i>=32 && (Central_Exclude_High & (1U<<(i-32)))))
			continue;
		AvsReadCentralOscThresholds(pAvs, i, &lower, &upper);
		current_osc_val = AvsReadCentOsc(pAvs, i);
#if AVS_DEBUG
 		BDBG_MSG(("%d %08x %08x %08x", i, current_osc_val, lower, upper ));
#endif
   		if (current_osc_val < lower) {
				increase = true;
			} else if (current_osc_val < upper) {
				found_one = true;
		}
	 }

	 for (i=0; i<NUMBER_OF_REMOTE; i++)
	 {
		if ((i<32 && (Remote_Exclude_Low & (1U<<i))) || (i>=32 && (Remote_Exclude_High & (1U<<(i-32)))))
			continue; /*if we didn't get a value then this is an unimplemented oscillator*/

		current_osc_val = AvsReadRmtOsc(pAvs, i);
		AvsReadRemoteOscThresholds(pAvs, i, &lower, &upper);
#if AVS_DEBUG
 		BDBG_MSG(("%d %08x %08x %08x", i, current_osc_val, lower, upper ));
#endif
			if (current_osc_val < lower) {
				increase = true;
			} else if (current_osc_val < upper) {
				found_one = true;
		}
	}
#define DAC_PI_STEP_SIZE 1
	/* If any oscillator was below the lower threshold we need to increase the voltage (by decreasing the DAC) */
	if (increase) {
			failed = AvsAdjustDacCode (pAvs, -(DAC_PI_STEP_SIZE*2));
	}
	/* If no oscillator was lower than the upper threshold then we need to lower the voltage (by increasing the DAC) */
	else if (!found_one) {
		failed = AvsAdjustDacCode (pAvs, DAC_PI_STEP_SIZE);
	}

	return failed;
}

#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
static void AvsLogAll(AvsContext_t * pAvs)
{
	uint32_t i;
	uint32_t reg, busy=1;
	int sensor_id;

	for (sensor_id = PVT_TEMPERATURE; sensor_id <= PVT_TESTMODE; sensor_id++) {
		busy = 1;
		while (busy) {
			/* These steps need to be done one-at-a-time */
			reg = 0;
			reg |= BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK;
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
			BKNI_Sleep (10); /* delay needed after indicating takeover */
			reg |= sensor_id << BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_sensor_idx_SHIFT;
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
			reg |= BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK;
			BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
			BKNI_Sleep (1); /* delay a bit before checking busy status */

			/* The busy comes on during processing and goes off when done */
			for (i=0; i<MAX_ITERATIONS; i++) 
			{
				busy = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY) & BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY_busy_MASK;
				if (!busy) break;
				BKNI_Sleep (1); /* delay a bit before checking again! */
			}

			/* We sometimes see the busy stuck on for some reason (if its still on, dismiss this data) */
			if (busy) {
				BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, 0);
			}
		}

		if (sensor_id == PVT_TEMPERATURE)	AvsLog("Temperature : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS));
		if (sensor_id == PVT_PROCESS)			AvsLog("Process     : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS));
		if (sensor_id == PVT_0P99V)			AvsLog("0.99 Monitor: %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS));
		if (sensor_id == PVT_1P10V_0)			AvsLog("1.10-0      : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS));
		if (sensor_id == PVT_1P10V_1)			AvsLog("1.10-1      : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS));
		if (sensor_id == PVT_2p75V)			AvsLog("2.75        : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS));
		if (sensor_id == PVT_3p63V)			AvsLog("3.63        : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS));
		if (sensor_id == PVT_TESTMODE)		AvsLog("Test Mode   : %08x\n", BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS));

		reg = BREG_Read32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS);
		reg &= ~(BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK | BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK);
		BREG_Write32(pAvs, BCHP_AVS_HW_MNTR_SW_CONTROLS, reg);
	}

	for (i=0; i<NUMBER_OF_CENTRAL; i++)
	{
		if ((i<32 && (Central_Exclude_Low & (1U<<i))) || (i>=32 && (Central_Exclude_High & (1U<<(i-32)))))
			continue;
		AvsLog("CentOsc #%d val: %08x\n", i, BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0 + (i * 4)));
	}

	for (i=0; i<NUMBER_OF_REMOTE; i++)
	{
		if ((i<32 && (Remote_Exclude_Low & (1U<<i))) || (i>=32 && (Remote_Exclude_High & (1U<<(i-32)))))
			continue;
		AvsLog("RmtOsc #%d val: %08x\n", i, BREG_Read32(pAvs, BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0 + (i * 4)));
	}

	return;
}
#endif

/*************************************************************
*  Run the Avs algorithm
**************************************************************/
void AvsStart(AvsContext_t * pAvs, bool sigma_enabled)
{
	int32_t Vpred, Vact;


    BDBG_MSG(("AVS Version is  %d.%d\n", MAJOR_VERSION, MINOR_VERSION));
    
	AvsInitializeOscs(pAvs);
	Vpred = Find_final_avs_voltage(pAvs, sigma_enabled);
	if (!Vpred) 
	{
		pAvs->avs_detect = false;
		return;
	}
	pAvs->predicated_value = Vpred;

	Vact = AvsReadVoltage(pAvs); 
	if (S2>1000) Vact /= (S2/1000); 

	BDBG_MSG(("AVS FFV= %d\n", Vact));

	printk("AVS Predicted Voltage: %d mVolt\n", Vpred);
	printk("AVS Activated Voltage: %d mVolt\n", Vact);

#if 0
	/* Save the last predicted voltage value in a location that can be picked up by the reference code.
	** This is used to identify FF parts (the reference code treats FF parts differenly).
	*/
	BREG_Write32(0, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0, Vpred);
#endif
}

/*************************************************************
*  Update the Low and High Threshold values
**************************************************************/
static void AvsSetNewThresholds(AvsContext_t * pAvs, bool Low_value)
{
	uint32_t i, lowest_hvt, lowest_svt, temp;


	/* 1) Find the lowest threshold for both the central and remote oscillators */	
	lowest_hvt = AvsReadCentOsc(pAvs, CENTRAL_OSC_HVT);
	lowest_svt = AvsReadCentOsc(pAvs, CENTRAL_OSC_SVT);

    lowest_hvt *= 2; /* normalizing the central to the remotes because central are divided by 2 relative to remotes */
    lowest_svt *= 2;

	for (i=0; i<NUMBER_OF_REMOTE; i++)
	{
		if ((i<32 && (Remote_Exclude_Low & (1U<<i))) || (i>=32 && (Remote_Exclude_High & (1U<<(i-32))))) 
			continue; /*if we didn't get a value then this is an unimplemented oscillator*/
		if (i & 1) {
			if ((temp = AvsReadRmtOsc(pAvs, i)) < lowest_hvt) { lowest_hvt = temp; }
		} else {
			if ((temp = AvsReadRmtOsc(pAvs, i)) < lowest_svt) { lowest_svt = temp; }
		}
	}

	if (Low_value)
	{
		/* 2) Set the lower threshold with the smallest value found above. */
		BREG_Write32(pAvs,  (BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (CENTRAL_OSC_HVT*4)), lowest_hvt/2);
		BREG_Write32(pAvs,  (BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0 + (CENTRAL_OSC_SVT*4)), lowest_svt/2);
		BREG_Write32(pAvs,  BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8H, lowest_hvt);
		BREG_Write32(pAvs,  BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_RMT_ROSC_G8S, lowest_svt);
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
      pAvs->lower_hvt_threshold = lowest_hvt;
      pAvs->lower_svt_threshold = lowest_svt;
#endif
	}
	else
	{
		/* 2) Set the upper threshold with the smallest value found above. */
		BREG_Write32(pAvs,  (BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (CENTRAL_OSC_HVT*4)), lowest_hvt/2);
		BREG_Write32(pAvs,  (BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0 + (CENTRAL_OSC_SVT*4)), lowest_svt/2);
		BREG_Write32(pAvs,  BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8H, lowest_hvt);
		BREG_Write32(pAvs,  BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_RMT_ROSC_G8S, lowest_svt);
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
      pAvs->upper_hvt_threshold = lowest_hvt;
      pAvs->upper_svt_threshold = lowest_svt;
#endif
	}
}

/*************************************************************
*  Run  AvsConstantVoltageProcess
**************************************************************/
/* This is a special version for the FF parts.
** This is meant to maintain the voltage at 0.85V regardless of the temperature.
*/
static void AvsConstantVoltageProcess(AvsContext_t * pAvs)
{
	uint32_t dac, saved, voltage;
	uint32_t local_FF_Voltage_Norm;
	voltage = AvsReadVoltage(pAvs);
	if (S2>1000) voltage /= (S2/1000);
#if (BCHP_CHIP==6318)
	saved = dac = pAvs->saved_dac_code;
#else
	saved = dac = BREG_Read32(pAvs,  BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
#endif

#if (BCHP_CHIP==4550)
   #define FF_Voltage_Norm 750 /* 0.750V */
   local_FF_Voltage_Norm = FF_Voltage_Norm;
#else
   local_FF_Voltage_Norm = pAvs->Vmin_Avs/10;
#endif
#define ConstantMargin 4 /* millivolts */
#define DACchange 1 /* amount to change DAC on each pass */

	if (voltage < (local_FF_Voltage_Norm - ConstantMargin)) dac -= DACchange; /* raise the voltage by lowering the DAC */
	if (voltage > (local_FF_Voltage_Norm + ConstantMargin)) dac += DACchange; /* lower the voltage by raising the DAC */

	/* Only make changes when the value has changed */
	if (dac != saved) AvsSetVoltage(pAvs, dac);
}

/*************************************************************
*  Run  AvsResetSequencers
**************************************************************/
/* Reset the PVT Monitor Sequencer.  
** Note: there needs to be a delay before reading all the values for them to become valid. 
** We do that by performing the reset after completing the convergence process, before we get called back in for a second pass.
*/
static void AvsResetSequencers(AvsContext_t * pAvs)
{
	BREG_Write32(pAvs,  BCHP_AVS_HW_MNTR_SEQUENCER_INIT, 1);
	BREG_Write32(pAvs,  BCHP_AVS_HW_MNTR_SEQUENCER_INIT, 0);
}

/*************************************************************
*  Run thw Avs algorithm
**************************************************************/
void AvsUpdate(AvsContext_t * pAvs)
{
	unsigned current_dac;

#if (BCHP_CHIP==4550)
   #define FF_PART						675
#elif (BCHP_CHIP==6802)
   #define FF_PART						680        /* A FF part is identified as having a predicted voltage lower the 0.680V */
#else  
   #define FF_PART						740        /* A FF part is identified as having a predicted voltage lower the 0.740V */
#endif   
#define DAC_OFFSET					16

	pAvs->AvsRunning = true;

	if (pAvs->predicated_value < FF_PART) {
		AvsConstantVoltageProcess(pAvs);
	}
	else
	{

		if (pAvs->first_time)
		{
#if (BCHP_CHIP==6318)
			current_dac = pAvs->saved_dac_code;
#else
			current_dac = BREG_Read32(pAvs,  BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE);
#endif
			AvsSetNewThresholds(pAvs, true);
			/*  Now, lower the DAC (raise the voltage) to choose the upper threshold value */
			current_dac -= DAC_OFFSET;
			AvsSetVoltage(pAvs, current_dac);
			BKNI_Sleep(1000);
			AvsSetNewThresholds(pAvs, false);
			pAvs->first_time = false;

		}

		AvsAdjustVddc(pAvs);
	}

#if 0
	{
		static uint32_t i = 0;

		if ((i++ % 500) == 1)
		{
			int32_t Voltage;
			Voltage = AvsReadVoltage();
			if (S2>1000) Voltage /= (S2/1000); 
			BDBG_MSG(("AVS FV = %d  Temp = %d", Voltage, AvsGetTemperature()));
		}
	}
#endif

	AvsResetSequencers(pAvs);
	pAvs->AvsRunning = false;	
}


/*************************************************************
*   return temperature in units of 1/10 degrees Celsius
**************************************************************/
int AvsGetTemperature(AvsContext_t * pAvs)
{
   uint32_t t;
   
	t = (BREG_Read32(pAvs,  BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS) & BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS_data_MASK);
   return ((4180000 - (int32_t)(5556 * t))/1000);
}


/*************************************************************
*  Initialize the AVS context structure
***********************************************************/
void AvsInit(AvsContext_t * pAvs, uint32_t devId, uint32_t vmin_avs_mv)
{
#if (BCHP_CHIP==6802)
	uint32_t chip_id;
#endif

	pAvs->device_id = devId;
	pAvs->avs_detect = true;
	pAvs->first_time = true;
	pAvs->AvsRunning=false;
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
	pAvs->AvsFlushReading=true;
#endif
	if (vmin_avs_mv) {
		pAvs->Vmin_Avs = vmin_avs_mv * 10;
	}
	else {
		pAvs->Vmin_Avs = vmin_avs;
	}

#if (BCHP_CHIP==6802)
	chip_id = BREG_Read32(pAvs, AVS_CHIP_ID_REGISTER);
	if ((chip_id & 0xFFFEFFFF) == 0x68020020)
	{
		pAvs->vmarginL = INT(.055);
		pAvs->vmarginH = INT(.080);
		pAvs->Vmin_Avs = INT(.840);
	}
	else
	{
		pAvs->vmarginL = INT(.150);
		pAvs->vmarginH = INT(.100);
	}
	//printk("AVS: %X chip: vmarginL=%d vmarginH=%d vmin_avs=%d\n", chip_id, pAvs->vmarginL, pAvs->vmarginH, pAvs->Vmin_Avs);
#else
	pAvs->vmarginL = INT(.075);
	pAvs->vmarginH = INT(.100);
#endif

#ifdef USE_SOFTWARE_TAKEOVER
	pAvs->SwTakeoverVars.index = 0;
	pAvs->SwTakeoverVars.num = AVERAGE_LOOPS;
#endif
}

/*************************************************************
*  Run the Avs at boot time
***********************************************************/
void Avs_BootRun(uint32_t vmin_avs_mv)
{
#ifdef AVS_ENABLED     
    uint32_t devId;
    bool sigma_enable =false;

#if (BCHP_CHIP==6802)
{
	// Find out how many MoCA 6802 devices there are
	BP_MOCA_INFO mocaInfo[BP_MOCA_MAX_NUM];
	int mocaChipNum = BP_MOCA_MAX_NUM;

	BpGetMocaInfo(mocaInfo, &mocaChipNum);
	
	num_devices = (uint32_t) mocaChipNum;
}
#endif
	for (devId = 0; devId < num_devices; devId++)
	{
		AvsInit(&avs_context[devId], devId, vmin_avs_mv);

#if (BCHP_FAMILY==3128) || (BCHP_FAMILY==3461) || (BCHP_FAMILY==3462) || (BCHP_FAMILY==3472)
		uint32_t sigma_res = Sigma_calculation();

		BDBG_MSG(("sigma_results detection %d", sigma_res));
		if (sigma_res == 0)
		{
			avs_context[devId].avs_detect = false;
			return;
		}
	    else
	       sigma_enable = true;
#endif

		AvsStart(&avs_context[devId], sigma_enable);
	}
#endif
}
/*************************************************************
*  Run the Avs Task
***********************************************************/
#if (BCHP_CHIP==6802) || (BCHP_CHIP==6318)
int Avs40_Task(void *data)
{
	uint32_t devId;
	AvsContext_t * pAvs;

	while(1)
	{
		if (kthread_should_stop())
			break;
		BKNI_Sleep(1000);

		if (AvsLogPeriod) {
			AvsLogTick--;
			if (AvsLogTick <= 0) {
				AvsLogTick = AvsLogPeriod;
			}
		}

		for (devId = 0; devId < num_devices; devId++)
		{
			pAvs = &avs_context[devId];

			if ((pAvs->AvsRunning == false) && (pAvs->avs_detect == true))
				AvsUpdate(pAvs);
			if (AvsLogPeriod && AvsLogTick == AvsLogPeriod) {
				AvsLogAll(pAvs);
			}
		}
#ifdef BCA_DEBUG_STATS
      printk("AVS Task: %d reads  %d writes\n", reg_read_count, reg_write_count);
      reg_read_count = 0;
      reg_write_count = 0;
#endif
	}

  	return(0);
}
#else
void Avs40_Task(void *data)
{
	uint32_t devId;
	AvsContext_t * pAvs;

	while(1)
	{
#ifdef AVS_ENABLED
		BKNI_Sleep(1000);
#else
		BKNI_Sleep(5000);
#endif
#ifdef AVS_ENABLED
		for (devId = 0; devId < num_devices; devId++)
		{
			pAvs = &avs_context[devId];

			if ((pAvs->AvsRunning == false) && (pAvs->avs_detect == true))
				AvsUpdate(pAvs);
		}
#endif
	}


}
#endif
