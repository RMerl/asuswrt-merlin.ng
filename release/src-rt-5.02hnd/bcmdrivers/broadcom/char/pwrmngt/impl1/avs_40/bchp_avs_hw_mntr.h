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

#ifndef BCHP_AVS_HW_MNTR_H__
#define BCHP_AVS_HW_MNTR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BCHP_AVS_HW_MNTR_SW_CONTROLS                      (AVS_BASE + 0x00000000) // Software control command registers for AVS
#define BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_takeover_MASK       (1 << 0)
#define BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_do_measure_MASK     (1 << 1)
#define BCHP_AVS_HW_MNTR_SW_CONTROLS_sw_sensor_idx_SHIFT    8

#define BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY         (AVS_BASE + 0x00000004) // Indicate measurement unit is busy and SW should not de-assert sw_takeover while this is asserted
#define BCHP_AVS_HW_MNTR_SW_MEASUREMENT_UNIT_BUSY_busy_MASK (1 << 0)

#define BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_PVT_MNTR       (AVS_BASE + 0x00000008) // Software to reset the pvt monitors measurements' valid bits in RO registers
#define BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_0     (AVS_BASE + 0x0000000c) // Software to reset the central roscs measurements' valid bits in RO registers
#define BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_CEN_ROSC_1     (AVS_BASE + 0x00000010) // Software to reset the central roscs measurements' valid bits in RO registers
#define BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_RMT_ROSC_0     (AVS_BASE + 0x00000014) // Software to reset the remote roscs measurements' valid bits in RO registers
#define BCHP_AVS_HW_MNTR_MEASUREMENTS_INIT_POW_WDOG       (AVS_BASE + 0x0000001c) // Software to reset the power watchdog measurment's valid bits in RO registers
#define BCHP_AVS_HW_MNTR_SEQUENCER_INIT                   (AVS_BASE + 0x00000020) // Initialize the sensor sequencer
#define BCHP_AVS_HW_MNTR_SEQUENCER_MASK_PVT_MNTR          (AVS_BASE + 0x00000024) // Indicate which PVT Monitor measurements should be masked(skipped) in the measurement sequence
#define BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_0        (AVS_BASE + 0x00000028) // Indicate which central ring oscillators should be masked(skipped) in the measurement sequence
#define BCHP_AVS_HW_MNTR_SEQUENCER_MASK_CEN_ROSC_1        (AVS_BASE + 0x0000002c) // Indicate which central ring oscillators should be masked(skipped) in the measurement sequence
#define BCHP_AVS_HW_MNTR_SEQUENCER_MASK_RMT_ROSC_0        (AVS_BASE + 0x00000030) // Indicate which remote ring oscillators should be masked(skipped) in the measurement sequence
#define BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_PVT_MNTR          (AVS_BASE + 0x00000038) // Enabling/Disabling PVT monitor
#define BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_0        (AVS_BASE + 0x0000003c) // Enabling/Disabling of central ring oscillators
#define BCHP_AVS_HW_MNTR_ENABLE_DEFAULT_CEN_ROSC_1        (AVS_BASE + 0x00000040) // Enabling/Disabling of central ring oscillators
#define BCHP_AVS_HW_MNTR_ROSC_MEASUREMENT_TIME_CONTROL    (AVS_BASE + 0x00000044) // Control the time taken for a rosc/pwd measurement
#define BCHP_AVS_HW_MNTR_ROSC_COUNTING_MODE               (AVS_BASE + 0x00000048) // Control the counting event for rosc signal counter
#define BCHP_AVS_HW_MNTR_INTERRUPT_POW_WDOG_EN            (AVS_BASE + 0x0000004c) // Software to program a mask(1 bit per PWD) to indicate which PWD signals may trigger an interrupt
#define BCHP_AVS_HW_MNTR_INTERRUPT_SW_MEASUREMENT_DONE_EN (AVS_BASE + 0x00000050) // Enable to trigger an interrupt when the measurement is done
#define BCHP_AVS_HW_MNTR_LAST_MEASURED_SENSOR             (AVS_BASE + 0x00000054) // The last sensor that has been measured
#define BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS              (AVS_BASE + 0x00000058) // Indicate AVS interrupts status whether they are triggered or not
#define BCHP_AVS_HW_MNTR_AVS_INTERRUPT_FLAGS_CLEAR        (AVS_BASE + 0x0000005c) // Software to clear the respective AVS interrupt flags
#define BCHP_AVS_HW_MNTR_REMOTE_SENSOR_TYPE               (AVS_BASE + 0x00000060) // Identify the technology type (8T or 10T) of ring oscillators used in AVS remote sensors
#define BCHP_AVS_HW_MNTR_AVS_REGISTERS_LOCKS              (AVS_BASE + 0x00000064) // To lock read/write accesses to AVS sensors and regulators registers for security purpose
#define BCHP_AVS_HW_MNTR_TEMPERATURE_RESET_ENABLE         (AVS_BASE + 0x00000068) // Software to enable chip's temperature monitoring and temperature reset
#define BCHP_AVS_HW_MNTR_TEMPERATURE_THRESHOLD            (AVS_BASE + 0x0000006c) // Threshold value for chip's temperature monitoring
#define BCHP_AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_0          (AVS_BASE + 0x00000070) // Set the output value of ring oscillators when not enabled.
#define BCHP_AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_1          (AVS_BASE + 0x00000074) // Set the output value of ring oscillators when not enabled.

#ifdef __cplusplus
}
#endif

#endif

