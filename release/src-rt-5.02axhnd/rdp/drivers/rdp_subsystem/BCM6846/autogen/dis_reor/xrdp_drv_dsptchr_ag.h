/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _XRDP_DRV_DSPTCHR_AG_H_
#define _XRDP_DRV_DSPTCHR_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* frst_lvl: FIRST_LEVEL - First Level congestion threshold.                                      */
/* scnd_lvl: SECOND_LEVEL - Second Level congestion threshold.                                    */
/* hyst_thrs: HYST_THRESHOLD - Hystersis value in which to stop congestion indication. once reach */
/*            in a congestion level only after crossing the (threshold_level - HYST_TRSH) will th */
/*            e congestion indication be removed                                                  */
/**************************************************************************************************/
typedef struct
{
    uint16_t frst_lvl;
    uint16_t scnd_lvl;
    uint8_t hyst_thrs;
} dsptchr_cngs_params;


/**************************************************************************************************/
/* frst_lvl: FIRST_LEVEL - First Level congestion threshold.                                      */
/* scnd_lvl: SECOND_LEVEL - Second Level congestion threshold.                                    */
/* hyst_thrs: HYST_THRESHOLD - Hystersis value in which to stop congestion indication. once reach */
/*            in a congestion level only after crossing the (threshold_level - HYST_TRSH) will th */
/*            e congestion indication be removed                                                  */
/**************************************************************************************************/
typedef struct
{
    uint16_t frst_lvl;
    uint16_t scnd_lvl;
    uint8_t hyst_thrs;
} dsptchr_glbl_cngs_params;


/**************************************************************************************************/
/* cmn_pool_lmt: POOL_LMT - MAX number of buffers allowed in the pool                             */
/* grnted_pool_lmt: POOL_LMT - MAX number of buffers allowed in the pool                          */
/* mcast_pool_lmt: POOL_LMT - MAX number of buffers allowed in the pool                           */
/* rnr_pool_lmt: POOL_LMT - MAX number of buffers allowed in the pool                             */
/* cmn_pool_size: POOL_SIZE - Number of buffers currently in the pool                             */
/* grnted_pool_size: POOL_SIZE - Number of buffers currently in the pool                          */
/* mcast_pool_size: POOL_SIZE - Number of buffers currently in the pool                           */
/* rnr_pool_SIZE: POOL_SIZE - Number of buffers currently in the pool                             */
/* processing_pool_size: POOL_SIZE - Number of buffers currently in the pool                      */
/**************************************************************************************************/
typedef struct
{
    uint16_t cmn_pool_lmt;
    uint16_t grnted_pool_lmt;
    uint16_t mcast_pool_lmt;
    uint16_t rnr_pool_lmt;
    uint16_t cmn_pool_size;
    uint16_t grnted_pool_size;
    uint16_t mcast_pool_size;
    uint16_t rnr_pool_size;
    uint16_t processing_pool_size;
} dsptchr_pools_limits;


/**************************************************************************************************/
/* head: HEAD - Pointer to the first BD in the link list of this queue.                           */
/* tail: TAIL - Pointer to the last BD in the linked list of this queue.                          */
/* minbuf: MINBUF - Low threshold Interrupt. When number of bytes reach this level, then an inter */
/*         rupt is generated to the Host.                                                         */
/* bfin: BFIN - 32 bit wrap around counter. Counts number of entries that entered this queue sinc */
/*       e start of queue activity.                                                               */
/* count: COUNT - 32 bit wrap around counter. Counts number of entries that left this queue since */
/*         start of queue activity.                                                               */
/**************************************************************************************************/
typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint32_t minbuf;
    uint32_t bfin;
    uint32_t count;
} dsptchr_fll_entry;


/**************************************************************************************************/
/* base_add: BASE_ADDRESS - Base address within each RNR                                          */
/* offset_add: OFFSET_ADDRESS - OFFSET address, in conjunction with base address for each task th */
/*             ere will be a different address to where to send the PDADD = BASE_ADD + (OFFSET_AD */
/*             D x TASK)PD size is 128bits                                                        */
/**************************************************************************************************/
typedef struct
{
    uint16_t base_add;
    uint16_t offset_add;
} dsptchr_rnr_dsptch_addr;


/**************************************************************************************************/
/* disp_enable: ENABLE - Enable dispatcher reorder block                                          */
/* rdy: READY - Dispatcher reorder block is RDY                                                   */
/* reordr_par_mod: REORDER_SM_PARALLEL_MODE_ - Enables parallel operation of Re-Order scheduler t */
/*                 o Re-Order SM.Reduces Re-Order cycle from 16 clocks to 7.                      */
/* per_q_egrs_congst_en: EGRESS_PER_Q_CONGESTION_ENALBE - Enable per Q Egress congestion monitori */
/*                       ng                                                                       */
/* dsptch_sm_enh_mod: DISPATCHER_SM_PERFORMANCE_ENH_MODE_ - Enables Enhanced performance mode of  */
/*                    Dispatcher Load balancing and Dispatcher SM.This allows Disptach of PD to R */
/*                    NR instead of every 14 clocks, every 11 clocks.                             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean disp_enable;
    bdmf_boolean rdy;
    bdmf_boolean reordr_par_mod;
    bdmf_boolean per_q_egrs_congst_en;
    bdmf_boolean dsptch_sm_enh_mod;
} dsptchr_reorder_cfg_dsptchr_reordr_cfg;


/**************************************************************************************************/
/* bypass_clk_gate: BYPASS_CLOCK_GATE - If set to 1b1 will disable the clock gate logic such to a */
/*                  lways enable the clock                                                        */
/* timer_val: TIMER_VALUE - For how long should the clock stay active once all conditions for clo */
/*            ck disable are met.                                                                 */
/* keep_alive_en: KEEP_ALIVE_ENABLE - Enables the keep alive logic which will periodically enable */
/*                 the clock to assure that no deadlock of clock being removed completely will oc */
/*                cur                                                                             */
/* keep_alive_intrvl: KEEP_ALIVE_INTERVAL - If the KEEP alive option is enabled the field will de */
/*                    termine for how many cycles should the clock be active                      */
/* keep_alive_cyc: KEEP_ALIVE_CYCLE - If the KEEP alive option is enabled this field will determi */
/*                 ne for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTE */
/*                 RVAL)So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} dsptchr_reorder_cfg_clk_gate_cntrl;


/**************************************************************************************************/
/* glbl_congstn: GLOBAL_CONGESTION - Global congestion levels (according to FLL buffer availabili */
/*               ty)                                                                              */
/* glbl_egrs_congstn: GLOBAL_EGRESS_CONGESTION - Global Egress congestion levels                  */
/* sbpm_congstn: SBPM_CONGESTION - SBPM congestion levels according to SPBM messages              */
/* glbl_congstn_stcky: GLOBAL_CONGESTION_STICKY - Global congestion levels (according to FLL buff */
/*                     er availability)Sticky Value                                               */
/* glbl_egrs_congstn_stcky: GLOBAL_EGRESS_CONGESTION_STICKY - Global Egress congestion levelsStic */
/*                          ky value                                                              */
/* sbpm_congstn_stcky: SBPM_CONGESTION_STICKY - SBPM congestion levels according to SPBM messages */
/*                     Sticky value                                                               */
/**************************************************************************************************/
typedef struct
{
    uint8_t glbl_congstn;
    uint8_t glbl_egrs_congstn;
    uint8_t sbpm_congstn;
    uint8_t glbl_congstn_stcky;
    uint8_t glbl_egrs_congstn_stcky;
    uint8_t sbpm_congstn_stcky;
} dsptchr_congestion_congstn_status;


/**************************************************************************************************/
/* mask: MASK - MASK                                                                              */
/**************************************************************************************************/
typedef struct
{
    uint32_t task_mask[8];
} dsptchr_mask_msk_tsk_255_0;


/**************************************************************************************************/
/* q0: QEUEU_0 - wakeup request pending                                                           */
/* q1: QEUEU_1 - wakeup request pending                                                           */
/* q2: QEUEU_2 - wakeup request pending                                                           */
/* q3: QEUEU_3 - wakeup request pending                                                           */
/* q4: QEUEU_4 - wakeup request pending                                                           */
/* q5: QEUEU_5 - wakeup request pending                                                           */
/* q6: QEUEU_6 - wakeup request pending                                                           */
/* q7: QEUEU_7 - wakeup request pending                                                           */
/* q8: QEUEU_8 - wakeup request pending                                                           */
/* q9: QEUEU_9 - wakeup request pending                                                           */
/* q10: QEUEU_10 - wakeup request pending                                                         */
/* q11: QEUEU_11 - wakeup request pending                                                         */
/* q12: QEUEU_12 - wakeup request pending                                                         */
/* q13: QEUEU_13 - wakeup request pending                                                         */
/* q14: QEUEU_14 - wakeup request pending                                                         */
/* q15: QEUEU_15 - wakeup request pending                                                         */
/* q16: QEUEU_16 - wakeup request pending                                                         */
/* q17: QEUEU_17 - wakeup request pending                                                         */
/* q18: QEUEU_18 - wakeup request pending                                                         */
/* q19: QEUEU_19 - wakeup request pending                                                         */
/* q20: QEUEU_20 - wakeup request pending                                                         */
/* q21: QEUEU_21 - wakeup request pending                                                         */
/* q22: QEUEU_22 - wakeup request pending                                                         */
/* q23: QEUEU_23 - wakeup request pending                                                         */
/* q24: QEUEU_24 - wakeup request pending                                                         */
/* q25: QEUEU_25 - wakeup request pending                                                         */
/* q26: QEUEU_26 - wakeup request pending                                                         */
/* q27: QEUEU_27 - wakeup request pending                                                         */
/* q28: QEUEU_28 - wakeup request pending                                                         */
/* q29: QEUEU_29 - wakeup request pending                                                         */
/* q30: QEUEU_30 - wakeup request pending                                                         */
/* q31: QEUEU_31 - wakeup request pending                                                         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean q0;
    bdmf_boolean q1;
    bdmf_boolean q2;
    bdmf_boolean q3;
    bdmf_boolean q4;
    bdmf_boolean q5;
    bdmf_boolean q6;
    bdmf_boolean q7;
    bdmf_boolean q8;
    bdmf_boolean q9;
    bdmf_boolean q10;
    bdmf_boolean q11;
    bdmf_boolean q12;
    bdmf_boolean q13;
    bdmf_boolean q14;
    bdmf_boolean q15;
    bdmf_boolean q16;
    bdmf_boolean q17;
    bdmf_boolean q18;
    bdmf_boolean q19;
    bdmf_boolean q20;
    bdmf_boolean q21;
    bdmf_boolean q22;
    bdmf_boolean q23;
    bdmf_boolean q24;
    bdmf_boolean q25;
    bdmf_boolean q26;
    bdmf_boolean q27;
    bdmf_boolean q28;
    bdmf_boolean q29;
    bdmf_boolean q30;
    bdmf_boolean q31;
} dsptchr_wakeup_control_wkup_req;


/**************************************************************************************************/
/* q0: QEUEU_0 - Valid Credits                                                                    */
/* q1: QEUEU_1 - Valid Credits.                                                                   */
/* q2: QEUEU_2 - Valid Credits                                                                    */
/* q3: QEUEU_3 - Valid Credits                                                                    */
/* q4: QEUEU_4 - Valid Credits                                                                    */
/* q5: QEUEU_5 - Valid Credits                                                                    */
/* q6: QEUEU_6 - Valid Credits                                                                    */
/* q7: QEUEU_7 - Valid Credits                                                                    */
/* q8: QEUEU_8 - Valid Credits                                                                    */
/* q9: QEUEU_9 - Valid Credits                                                                    */
/* q10: QEUEU_10 - Valid Credits                                                                  */
/* q11: QEUEU_11 - Valid Credits                                                                  */
/* q12: QEUEU_12 - Valid Credits                                                                  */
/* q13: QEUEU_13 - Valid Credits                                                                  */
/* q14: QEUEU_14 - Valid Credits                                                                  */
/* q15: QEUEU_15 - Valid Credits                                                                  */
/* q16: QEUEU_16 - Valid Credits                                                                  */
/* q17: QEUEU_17 - Valid Credits                                                                  */
/* q18: QEUEU_18 - Valid Credits                                                                  */
/* q19: QEUEU_19 - Valid Credits                                                                  */
/* q20: QEUEU_20 - Valid Credits                                                                  */
/* q21: QEUEU_21 - Valid Credits                                                                  */
/* q22: QEUEU_22 - Valid Credits                                                                  */
/* q23: QEUEU_23 - Valid Credits                                                                  */
/* q24: QEUEU_24 - Valid Credits                                                                  */
/* q25: QEUEU_25 - Valid Credits                                                                  */
/* q26: QEUEU_26 - Valid Credits                                                                  */
/* q27: QEUEU_27 - Valid Credits                                                                  */
/* q28: QEUEU_28 - Valid Credits                                                                  */
/* q29: QEUEU_29 - Valid Credits                                                                  */
/* q30: QEUEU_30 - Valid Credits                                                                  */
/* q31: QEUEU_31 - Valid Credits                                                                  */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean q0;
    bdmf_boolean q1;
    bdmf_boolean q2;
    bdmf_boolean q3;
    bdmf_boolean q4;
    bdmf_boolean q5;
    bdmf_boolean q6;
    bdmf_boolean q7;
    bdmf_boolean q8;
    bdmf_boolean q9;
    bdmf_boolean q10;
    bdmf_boolean q11;
    bdmf_boolean q12;
    bdmf_boolean q13;
    bdmf_boolean q14;
    bdmf_boolean q15;
    bdmf_boolean q16;
    bdmf_boolean q17;
    bdmf_boolean q18;
    bdmf_boolean q19;
    bdmf_boolean q20;
    bdmf_boolean q21;
    bdmf_boolean q22;
    bdmf_boolean q23;
    bdmf_boolean q24;
    bdmf_boolean q25;
    bdmf_boolean q26;
    bdmf_boolean q27;
    bdmf_boolean q28;
    bdmf_boolean q29;
    bdmf_boolean q30;
    bdmf_boolean q31;
} dsptchr_disptch_scheduling_vld_crdt;


/**************************************************************************************************/
/* tsk0: TSK0_TO_RG_MAP - Can be Task 0/8/16...                                                   */
/* tsk1: TSK1_TO_RG_MAP - Can be Task 1/9/17...                                                   */
/* tsk2: TSK2_TO_RG_MAP - Can be Task 2/10/18...                                                  */
/* tsk3: TSK3_TO_RG_MAP - Can be Task 3/11/19...                                                  */
/* tsk4: TSK4_TO_RG_MAP - Can be Task 4/12/20...                                                  */
/* tsk5: TSK5_TO_RG_MAP - Can be Task 5/13/21...                                                  */
/* tsk6: TSK6_TO_RG_MAP - Can be Task 6/14/22...                                                  */
/* tsk7: TSK7_TO_RG_MAP - Can be Task 7/15/23...                                                  */
/**************************************************************************************************/
typedef struct
{
    uint8_t tsk0;
    uint8_t tsk1;
    uint8_t tsk2;
    uint8_t tsk3;
    uint8_t tsk4;
    uint8_t tsk5;
    uint8_t tsk6;
    uint8_t tsk7;
} dsptchr_load_balancing_tsk_to_rg_mapping;


/**************************************************************************************************/
/* fll_return_buf: BUF_RETURNED_TO_FLL - Buffer returned to Fll                                   */
/* fll_cnt_drp: FLL_COUNTED_DROP - Drop PD counted                                                */
/* unknwn_msg: UNKNOWN_MESSAGE - Unknown message entered the dispatcher                           */
/* fll_overflow: FLL_OVERFLOW - Number of buffers returned to FLL exceeds the pre-defined allocat */
/*               ed buffer amount (due to linked list bug)                                        */
/* fll_neg: FLL_NEGATIVE_AMOUNT_OF_BUF - Number of buffers returned to FLL decreased under zero a */
/*          nd reached a negative amount (due to linked list bug)                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean fll_return_buf;
    bdmf_boolean fll_cnt_drp;
    bdmf_boolean unknwn_msg;
    bdmf_boolean fll_overflow;
    bdmf_boolean fll_neg;
} dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr;


/**************************************************************************************************/
/* qdest0_int: QDEST0_INT - New Entry added to Destination queue 0                                */
/* qdest1_int: QDEST1_INT - New Entry added to Destination queue 1                                */
/* qdest2_int: QDEST2_INT - New Entry added to Destination queue 2                                */
/* qdest3_int: QDEST3_INT - New Entry added to Destination queue 3                                */
/* qdest4_int: QDEST4_INT - New Entry added to Destination queue 4                                */
/* qdest5_int: QDEST5_INT - New Entry added to Destination queue 5                                */
/* qdest6_int: QDEST6_INT - New Entry added to Destination queue 6                                */
/* qdest7_int: QDEST7_INT - New Entry added to Destination queue 7                                */
/* qdest8_int: QDEST8_INT - New Entry added to Destination queue 8                                */
/* qdest9_int: QDEST9_INT - New Entry added to Destination queue 9                                */
/* qdest10_int: QDEST10_INT - New Entry added to Destination queue 10                             */
/* qdest11_int: QDEST11_INT - New Entry added to Destination queue 11                             */
/* qdest12_int: QDEST12_INT - New Entry added to Destination queue 12                             */
/* qdest13_int: QDEST13_INT - New Entry added to Destination queue 13                             */
/* qdest14_int: QDEST14_INT - New Entry added to Destination queue 14                             */
/* qdest15_int: QDEST15_INT - New Entry added to Destination queue 15                             */
/* qdest16_int: QDEST16_INT - New Entry added to Destination queue 16                             */
/* qdest17_int: QDEST17_INT - New Entry added to Destination queue 17                             */
/* qdest18_int: QDEST18_INT - New Entry added to Destination queue 18                             */
/* qdest19_int: QDEST19_INT - New Entry added to Destination queue 19                             */
/* qdest20_int: QDEST20_INT - New Entry added to Destination queue 20                             */
/* qdest21_int: QDEST21_INT - New Entry added to Destination queue 21                             */
/* qdest22_int: QDEST22_INT - New Entry added to Destination queue 22                             */
/* qdest23_int: QDEST23_INT - New Entry added to Destination queue 23                             */
/* qdest24_int: QDEST24_INT - New Entry added to Destination queue 24                             */
/* qdest25_int: QDEST25_INT - New Entry added to Destination queue 25                             */
/* qdest26_int: QDEST26_INT - New Entry added to Destination queue 26                             */
/* qdest27_int: QDEST27_INT - New Entry added to Destination queue 27                             */
/* qdest28_int: QDEST28_INT - New Entry added to Destination queue 28                             */
/* qdest29_int: QDEST29_INT - New Entry added to Destination queue 29                             */
/* qdest30_int: QDEST30_INT - New Entry added to Destination queue 30                             */
/* qdest31_int: QDEST31_INT - New Entry added to Destination queue 31                             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean qdest0_int;
    bdmf_boolean qdest1_int;
    bdmf_boolean qdest2_int;
    bdmf_boolean qdest3_int;
    bdmf_boolean qdest4_int;
    bdmf_boolean qdest5_int;
    bdmf_boolean qdest6_int;
    bdmf_boolean qdest7_int;
    bdmf_boolean qdest8_int;
    bdmf_boolean qdest9_int;
    bdmf_boolean qdest10_int;
    bdmf_boolean qdest11_int;
    bdmf_boolean qdest12_int;
    bdmf_boolean qdest13_int;
    bdmf_boolean qdest14_int;
    bdmf_boolean qdest15_int;
    bdmf_boolean qdest16_int;
    bdmf_boolean qdest17_int;
    bdmf_boolean qdest18_int;
    bdmf_boolean qdest19_int;
    bdmf_boolean qdest20_int;
    bdmf_boolean qdest21_int;
    bdmf_boolean qdest22_int;
    bdmf_boolean qdest23_int;
    bdmf_boolean qdest24_int;
    bdmf_boolean qdest25_int;
    bdmf_boolean qdest26_int;
    bdmf_boolean qdest27_int;
    bdmf_boolean qdest28_int;
    bdmf_boolean qdest29_int;
    bdmf_boolean qdest30_int;
    bdmf_boolean qdest31_int;
} dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr;


/**************************************************************************************************/
/* tsk_cnt_rnr_0: TASK_COUNT_RNR_0 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_1: TASK_COUNT_RNR_1 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_2: TASK_COUNT_RNR_2 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_3: TASK_COUNT_RNR_3 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_4: TASK_COUNT_RNR_4 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_5: TASK_COUNT_RNR_5 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_6: TASK_COUNT_RNR_6 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_7: TASK_COUNT_RNR_7 - Counter the amount of active tasks                           */
/**************************************************************************************************/
typedef struct
{
    uint8_t tsk_cnt_rnr_0;
    uint8_t tsk_cnt_rnr_1;
    uint8_t tsk_cnt_rnr_2;
    uint8_t tsk_cnt_rnr_3;
    uint8_t tsk_cnt_rnr_4;
    uint8_t tsk_cnt_rnr_5;
    uint8_t tsk_cnt_rnr_6;
    uint8_t tsk_cnt_rnr_7;
} dsptchr_debug_glbl_tsk_cnt_0_7;


/**************************************************************************************************/
/* tsk_cnt_rnr_8: TASK_COUNT_RNR_8 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_9: TASK_COUNT_RNR_9 - Counter the amount of active tasks                           */
/* tsk_cnt_rnr_10: TASK_COUNT_RNR_10 - Counter the amount of active tasks                         */
/* tsk_cnt_rnr_11: TASK_COUNT_RNR_11 - Counter the amount of active tasks                         */
/* tsk_cnt_rnr_12: TASK_COUNT_RNR_12 - Counter the amount of active tasks                         */
/* tsk_cnt_rnr_13: TASK_COUNT_RNR_13 - Counter the amount of active tasks                         */
/* tsk_cnt_rnr_14: TASK_COUNT_RNR_14 - Counter the amount of active tasks                         */
/* tsk_cnt_rnr_15: TASK_COUNT_RNR_15 - Counter the amount of active tasks                         */
/**************************************************************************************************/
typedef struct
{
    uint8_t tsk_cnt_rnr_8;
    uint8_t tsk_cnt_rnr_9;
    uint8_t tsk_cnt_rnr_10;
    uint8_t tsk_cnt_rnr_11;
    uint8_t tsk_cnt_rnr_12;
    uint8_t tsk_cnt_rnr_13;
    uint8_t tsk_cnt_rnr_14;
    uint8_t tsk_cnt_rnr_15;
} dsptchr_debug_glbl_tsk_cnt_8_15;


/**************************************************************************************************/
/* bfout: BFOUT - 32 bit wrap around counter. Counts number of packets that left this queue since */
/*         start of queue activity.                                                               */
/**************************************************************************************************/
typedef struct
{
    uint32_t qdes_bfout[32];
} dsptchr_qdes_bfout;


/**************************************************************************************************/
/* bufin: BUFIN - 32 bit wrap around counter. Counts number of packets that entered this queue si */
/*        nce start of queue activity.                                                            */
/**************************************************************************************************/
typedef struct
{
    uint32_t qdes_bfin[32];
} dsptchr_qdes_bufin;


/**************************************************************************************************/
/* fbdnull: FBDNull - If this bit is set then the first BD attached to this Q is a null BD. In th */
/*          is case, its Data Pointer field is not valid, but its Next BD pointer field is valid. */
/*           When it is set, the NullBD field for this queue is not valid.                        */
/**************************************************************************************************/
typedef struct
{
    uint32_t qdes_fbdnull[32];
} dsptchr_qdes_fbdnull;


/**************************************************************************************************/
/* nullbd: NullBD - 32 bits index of a Null BD that belongs to this queue. Both the data buffer p */
/*         ointer and the next BD field are non valid. The pointer defines a memory allocation fo */
/*         r a BD that might be used or not.                                                      */
/**************************************************************************************************/
typedef struct
{
    uint32_t qdes_nullbd[32];
} dsptchr_qdes_nullbd;


/**************************************************************************************************/
/* bufavail: BUFAVAIL - number of entries available in queue.bufin - bfout                        */
/**************************************************************************************************/
typedef struct
{
    uint32_t qdes_bufavail[32];
} dsptchr_qdes_bufavail;


/**************************************************************************************************/
/* data: Data - Data Buffer entry                                                                 */
/**************************************************************************************************/
typedef struct
{
    uint32_t reorder_ram_data[2];
} dsptchr_pdram_data;

bdmf_error_t ag_drv_dsptchr_cngs_params_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_cngs_params_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_glbl_cngs_params_set(const dsptchr_glbl_cngs_params *glbl_cngs_params);
bdmf_error_t ag_drv_dsptchr_glbl_cngs_params_get(dsptchr_glbl_cngs_params *glbl_cngs_params);
bdmf_error_t ag_drv_dsptchr_q_size_params_set(uint8_t q_idx, uint16_t cmn_cnt);
bdmf_error_t ag_drv_dsptchr_q_size_params_get(uint8_t q_idx, uint16_t *cmn_cnt);
bdmf_error_t ag_drv_dsptchr_credit_cnt_set(uint8_t q_idx, uint16_t credit_cnt);
bdmf_error_t ag_drv_dsptchr_credit_cnt_get(uint8_t q_idx, uint16_t *credit_cnt);
bdmf_error_t ag_drv_dsptchr_q_limits_params_set(uint8_t q_idx, uint16_t cmn_max, uint16_t gurntd_max);
bdmf_error_t ag_drv_dsptchr_q_limits_params_get(uint8_t q_idx, uint16_t *cmn_max, uint16_t *gurntd_max);
bdmf_error_t ag_drv_dsptchr_ingress_coherency_params_set(uint8_t q_idx, bdmf_boolean chrncy_en, uint16_t chrncy_cnt);
bdmf_error_t ag_drv_dsptchr_ingress_coherency_params_get(uint8_t q_idx, bdmf_boolean *chrncy_en, uint16_t *chrncy_cnt);
bdmf_error_t ag_drv_dsptchr_pools_limits_set(const dsptchr_pools_limits *pools_limits);
bdmf_error_t ag_drv_dsptchr_pools_limits_get(dsptchr_pools_limits *pools_limits);
bdmf_error_t ag_drv_dsptchr_fll_entry_set(const dsptchr_fll_entry *fll_entry);
bdmf_error_t ag_drv_dsptchr_fll_entry_get(dsptchr_fll_entry *fll_entry);
bdmf_error_t ag_drv_dsptchr_rnr_dsptch_addr_set(uint8_t rnr_idx, const dsptchr_rnr_dsptch_addr *rnr_dsptch_addr);
bdmf_error_t ag_drv_dsptchr_rnr_dsptch_addr_get(uint8_t rnr_idx, dsptchr_rnr_dsptch_addr *rnr_dsptch_addr);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(const dsptchr_reorder_cfg_dsptchr_reordr_cfg *reorder_cfg_dsptchr_reordr_cfg);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_get(dsptchr_reorder_cfg_dsptchr_reordr_cfg *reorder_cfg_dsptchr_reordr_cfg);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_vq_en_set(uint32_t en);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_vq_en_get(uint32_t *en);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_bb_cfg_set(uint8_t src_id, uint8_t dst_id_ovride, uint16_t route_ovride, bdmf_boolean ovride_en);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_bb_cfg_get(uint8_t *src_id, uint8_t *dst_id_ovride, uint16_t *route_ovride, bdmf_boolean *ovride_en);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(const dsptchr_reorder_cfg_clk_gate_cntrl *reorder_cfg_clk_gate_cntrl);
bdmf_error_t ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get(dsptchr_reorder_cfg_clk_gate_cntrl *reorder_cfg_clk_gate_cntrl);
bdmf_error_t ag_drv_dsptchr_congestion_egrs_congstn_set(uint8_t viq_idx, const dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_congestion_egrs_congstn_get(uint8_t viq_idx, dsptchr_cngs_params *cngs_params);
bdmf_error_t ag_drv_dsptchr_congestion_total_egrs_congstn_set(const dsptchr_glbl_cngs_params *glbl_cngs_params);
bdmf_error_t ag_drv_dsptchr_congestion_total_egrs_congstn_get(dsptchr_glbl_cngs_params *glbl_cngs_params);
bdmf_error_t ag_drv_dsptchr_congestion_congstn_status_get(dsptchr_congestion_congstn_status *congestion_congstn_status);
bdmf_error_t ag_drv_dsptchr_congestion_per_q_ingrs_congstn_low_get(uint32_t *congstn_state);
bdmf_error_t ag_drv_dsptchr_congestion_per_q_ingrs_congstn_high_get(uint32_t *congstn_state);
bdmf_error_t ag_drv_dsptchr_congestion_per_q_egrs_congstn_low_get(uint32_t *congstn_state);
bdmf_error_t ag_drv_dsptchr_congestion_per_q_egrs_congstn_high_get(uint32_t *congstn_state);
bdmf_error_t ag_drv_dsptchr_queue_mapping_crdt_cfg_set(uint8_t q_idx, uint8_t bb_id, uint16_t trgt_add);
bdmf_error_t ag_drv_dsptchr_queue_mapping_crdt_cfg_get(uint8_t q_idx, uint8_t *bb_id, uint16_t *trgt_add);
bdmf_error_t ag_drv_dsptchr_queue_mapping_q_dest_set(uint8_t q_idx, bdmf_boolean is_dest_disp);
bdmf_error_t ag_drv_dsptchr_queue_mapping_q_dest_get(uint8_t q_idx, bdmf_boolean *is_dest_disp);
bdmf_error_t ag_drv_dsptchr_mask_msk_tsk_255_0_set(uint8_t group_idx, const dsptchr_mask_msk_tsk_255_0 *mask_msk_tsk_255_0);
bdmf_error_t ag_drv_dsptchr_mask_msk_tsk_255_0_get(uint8_t group_idx, dsptchr_mask_msk_tsk_255_0 *mask_msk_tsk_255_0);
bdmf_error_t ag_drv_dsptchr_mask_msk_q_set(uint8_t group_idx, uint32_t mask);
bdmf_error_t ag_drv_dsptchr_mask_msk_q_get(uint8_t group_idx, uint32_t *mask);
bdmf_error_t ag_drv_dsptchr_mask_dly_q_set(uint8_t q_idx, bdmf_boolean set_delay);
bdmf_error_t ag_drv_dsptchr_mask_dly_q_get(uint8_t q_idx, bdmf_boolean *set_delay);
bdmf_error_t ag_drv_dsptchr_mask_non_dly_q_set(uint8_t q_idx, bdmf_boolean set_non_delay);
bdmf_error_t ag_drv_dsptchr_mask_non_dly_q_get(uint8_t q_idx, bdmf_boolean *set_non_delay);
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_set(uint8_t dly_crdt);
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_dly_qm_crdt_get(uint8_t *dly_crdt);
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_set(uint8_t non_dly_crdt);
bdmf_error_t ag_drv_dsptchr_egrs_queues_egrs_non_dly_qm_crdt_get(uint8_t *non_dly_crdt);
bdmf_error_t ag_drv_dsptchr_egrs_queues_total_q_egrs_size_set(uint16_t total_egrs_size);
bdmf_error_t ag_drv_dsptchr_egrs_queues_total_q_egrs_size_get(uint16_t *total_egrs_size);
bdmf_error_t ag_drv_dsptchr_egrs_queues_per_q_egrs_size_get(uint16_t q_idx, uint16_t *q_egrs_size);
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_req_set(const dsptchr_wakeup_control_wkup_req *wakeup_control_wkup_req);
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_req_get(dsptchr_wakeup_control_wkup_req *wakeup_control_wkup_req);
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_thrshld_set(uint16_t wkup_thrshld);
bdmf_error_t ag_drv_dsptchr_wakeup_control_wkup_thrshld_get(uint16_t *wkup_thrshld);
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_dwrr_info_set(uint8_t dwrr_q_idx, uint32_t q_crdt, bdmf_boolean ngtv, uint16_t quntum);
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_dwrr_info_get(uint8_t dwrr_q_idx, uint32_t *q_crdt, bdmf_boolean *ngtv, uint16_t *quntum);
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_vld_crdt_set(const dsptchr_disptch_scheduling_vld_crdt *disptch_scheduling_vld_crdt);
bdmf_error_t ag_drv_dsptchr_disptch_scheduling_vld_crdt_get(dsptchr_disptch_scheduling_vld_crdt *disptch_scheduling_vld_crdt);
bdmf_error_t ag_drv_dsptchr_load_balancing_lb_cfg_set(bdmf_boolean lb_mode, uint8_t sp_thrshld);
bdmf_error_t ag_drv_dsptchr_load_balancing_lb_cfg_get(bdmf_boolean *lb_mode, uint8_t *sp_thrshld);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_0_1_set(uint16_t rnr0, uint16_t rnr1);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_0_1_get(uint16_t *rnr0, uint16_t *rnr1);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_2_3_set(uint16_t rnr2, uint16_t rnr3);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_2_3_get(uint16_t *rnr2, uint16_t *rnr3);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_4_5_set(uint16_t rnr4, uint16_t rnr5);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_4_5_get(uint16_t *rnr4, uint16_t *rnr5);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_6_7_set(uint16_t rnr6, uint16_t rnr7);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_6_7_get(uint16_t *rnr6, uint16_t *rnr7);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_8_9_set(uint16_t rnr8, uint16_t rnr9);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_8_9_get(uint16_t *rnr8, uint16_t *rnr9);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_10_11_set(uint16_t rnr10, uint16_t rnr11);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_10_11_get(uint16_t *rnr10, uint16_t *rnr11);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_12_13_set(uint16_t rnr12, uint16_t rnr13);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_12_13_get(uint16_t *rnr12, uint16_t *rnr13);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_14_15_set(uint16_t rnr14, uint16_t rnr15);
bdmf_error_t ag_drv_dsptchr_load_balancing_free_task_14_15_get(uint16_t *rnr14, uint16_t *rnr15);
bdmf_error_t ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_set(uint8_t task_to_rg_mapping, const dsptchr_load_balancing_tsk_to_rg_mapping *load_balancing_tsk_to_rg_mapping);
bdmf_error_t ag_drv_dsptchr_load_balancing_tsk_to_rg_mapping_get(uint8_t task_to_rg_mapping, dsptchr_load_balancing_tsk_to_rg_mapping *load_balancing_tsk_to_rg_mapping);
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_set(uint8_t tsk_cnt_rg_0, uint8_t tsk_cnt_rg_1, uint8_t tsk_cnt_rg_2, uint8_t tsk_cnt_rg_3);
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_0_3_get(uint8_t *tsk_cnt_rg_0, uint8_t *tsk_cnt_rg_1, uint8_t *tsk_cnt_rg_2, uint8_t *tsk_cnt_rg_3);
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_set(uint8_t tsk_cnt_rg_4, uint8_t tsk_cnt_rg_5, uint8_t tsk_cnt_rg_6, uint8_t tsk_cnt_rg_7);
bdmf_error_t ag_drv_dsptchr_load_balancing_rg_avlabl_tsk_4_7_get(uint8_t *tsk_cnt_rg_4, uint8_t *tsk_cnt_rg_5, uint8_t *tsk_cnt_rg_6, uint8_t *tsk_cnt_rg_7);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_set(const dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr *dsptcher_reordr_top_intr_ctrl_0_isr);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr_get(dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr *dsptcher_reordr_top_intr_ctrl_0_isr);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_set(uint32_t iem);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_set(uint32_t ist);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr_get(uint32_t *ist);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_set(const dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr *dsptcher_reordr_top_intr_ctrl_1_isr);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr_get(dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr *dsptcher_reordr_top_intr_ctrl_1_isr);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_set(uint32_t iem);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_set(uint32_t ist);
bdmf_error_t ag_drv_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr_get(uint32_t *ist);
bdmf_error_t ag_drv_dsptchr_debug_dbg_bypss_cntrl_set(bdmf_boolean en_byp, uint8_t bbid_non_dly, uint8_t bbid_dly);
bdmf_error_t ag_drv_dsptchr_debug_dbg_bypss_cntrl_get(bdmf_boolean *en_byp, uint8_t *bbid_non_dly, uint8_t *bbid_dly);
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_set(const dsptchr_debug_glbl_tsk_cnt_0_7 *debug_glbl_tsk_cnt_0_7);
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_0_7_get(dsptchr_debug_glbl_tsk_cnt_0_7 *debug_glbl_tsk_cnt_0_7);
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_set(const dsptchr_debug_glbl_tsk_cnt_8_15 *debug_glbl_tsk_cnt_8_15);
bdmf_error_t ag_drv_dsptchr_debug_glbl_tsk_cnt_8_15_get(dsptchr_debug_glbl_tsk_cnt_8_15 *debug_glbl_tsk_cnt_8_15);
bdmf_error_t ag_drv_dsptchr_debug_dbg_bus_cntrl_set(uint8_t dbg_sel);
bdmf_error_t ag_drv_dsptchr_debug_dbg_bus_cntrl_get(uint8_t *dbg_sel);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_0_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_1_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_2_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_3_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_4_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_5_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_6_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_7_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_8_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_9_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_10_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_11_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_12_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_13_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_14_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_15_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_16_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_17_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_18_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_19_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_20_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_21_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_22_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_dbg_vec_23_get(uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_set(uint8_t dbg_mode, bdmf_boolean en_cntrs, bdmf_boolean clr_cntrs, uint8_t dbg_rnr_sel);
bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_sttstcs_ctrl_get(uint8_t *dbg_mode, bdmf_boolean *en_cntrs, bdmf_boolean *clr_cntrs, uint8_t *dbg_rnr_sel);
bdmf_error_t ag_drv_dsptchr_debug_statistics_dbg_cnt_get(uint8_t index, uint32_t *dbg_vec_val);
bdmf_error_t ag_drv_dsptchr_qdes_head_set(uint8_t q_idx, uint32_t head);
bdmf_error_t ag_drv_dsptchr_qdes_head_get(uint8_t q_idx, uint32_t *head);
bdmf_error_t ag_drv_dsptchr_qdes_bfout_set(uint8_t zero, const dsptchr_qdes_bfout *qdes_bfout);
bdmf_error_t ag_drv_dsptchr_qdes_bfout_get(uint8_t zero, dsptchr_qdes_bfout *qdes_bfout);
bdmf_error_t ag_drv_dsptchr_qdes_bufin_set(uint8_t zero, const dsptchr_qdes_bufin *qdes_bufin);
bdmf_error_t ag_drv_dsptchr_qdes_bufin_get(uint8_t zero, dsptchr_qdes_bufin *qdes_bufin);
bdmf_error_t ag_drv_dsptchr_qdes_tail_set(uint8_t q_idx, uint32_t tail);
bdmf_error_t ag_drv_dsptchr_qdes_tail_get(uint8_t q_idx, uint32_t *tail);
bdmf_error_t ag_drv_dsptchr_qdes_fbdnull_set(uint8_t zero, const dsptchr_qdes_fbdnull *qdes_fbdnull);
bdmf_error_t ag_drv_dsptchr_qdes_fbdnull_get(uint8_t zero, dsptchr_qdes_fbdnull *qdes_fbdnull);
bdmf_error_t ag_drv_dsptchr_qdes_nullbd_set(uint8_t zero, const dsptchr_qdes_nullbd *qdes_nullbd);
bdmf_error_t ag_drv_dsptchr_qdes_nullbd_get(uint8_t zero, dsptchr_qdes_nullbd *qdes_nullbd);
bdmf_error_t ag_drv_dsptchr_qdes_bufavail_get(uint8_t zero, dsptchr_qdes_bufavail *qdes_bufavail);
bdmf_error_t ag_drv_dsptchr_qdes_reg_q_head_set(uint8_t q_head_idx, uint16_t head);
bdmf_error_t ag_drv_dsptchr_qdes_reg_q_head_get(uint8_t q_head_idx, uint16_t *head);
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_head_vld_set(uint32_t viq_head_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_head_vld_get(uint32_t *viq_head_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_set(uint32_t chrncy_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_viq_chrncy_vld_get(uint32_t *chrncy_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_veq_head_vld_set(uint32_t viq_head_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_veq_head_vld_get(uint32_t *viq_head_vld);
bdmf_error_t ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_set(bdmf_boolean use_buf_avl, bdmf_boolean dec_bufout_when_mltcst);
bdmf_error_t ag_drv_dsptchr_qdes_reg_qdes_buf_avl_cntrl_get(bdmf_boolean *use_buf_avl, bdmf_boolean *dec_bufout_when_mltcst);
bdmf_error_t ag_drv_dsptchr_flldes_flldrop_set(uint32_t drpcnt);
bdmf_error_t ag_drv_dsptchr_flldes_flldrop_get(uint32_t *drpcnt);
bdmf_error_t ag_drv_dsptchr_flldes_bufavail_get(uint32_t *bufavail);
bdmf_error_t ag_drv_dsptchr_flldes_freemin_get(uint32_t *freemin);
bdmf_error_t ag_drv_dsptchr_bdram_data_set(uint16_t temp_index, uint16_t data);
bdmf_error_t ag_drv_dsptchr_bdram_data_get(uint16_t temp_index, uint16_t *data);
bdmf_error_t ag_drv_dsptchr_pdram_data_set(uint16_t temp_index, const dsptchr_pdram_data *pdram_data);
bdmf_error_t ag_drv_dsptchr_pdram_data_get(uint16_t temp_index, dsptchr_pdram_data *pdram_data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dsptchr_cngs_params,
    cli_dsptchr_glbl_cngs_params,
    cli_dsptchr_q_size_params,
    cli_dsptchr_credit_cnt,
    cli_dsptchr_q_limits_params,
    cli_dsptchr_ingress_coherency_params,
    cli_dsptchr_pools_limits,
    cli_dsptchr_fll_entry,
    cli_dsptchr_rnr_dsptch_addr,
    cli_dsptchr_reorder_cfg_dsptchr_reordr_cfg,
    cli_dsptchr_reorder_cfg_vq_en,
    cli_dsptchr_reorder_cfg_bb_cfg,
    cli_dsptchr_reorder_cfg_clk_gate_cntrl,
    cli_dsptchr_congestion_egrs_congstn,
    cli_dsptchr_congestion_total_egrs_congstn,
    cli_dsptchr_congestion_congstn_status,
    cli_dsptchr_congestion_per_q_ingrs_congstn_low,
    cli_dsptchr_congestion_per_q_ingrs_congstn_high,
    cli_dsptchr_congestion_per_q_egrs_congstn_low,
    cli_dsptchr_congestion_per_q_egrs_congstn_high,
    cli_dsptchr_queue_mapping_crdt_cfg,
    cli_dsptchr_queue_mapping_q_dest,
    cli_dsptchr_mask_msk_tsk_255_0,
    cli_dsptchr_mask_msk_q,
    cli_dsptchr_mask_dly_q,
    cli_dsptchr_mask_non_dly_q,
    cli_dsptchr_egrs_queues_egrs_dly_qm_crdt,
    cli_dsptchr_egrs_queues_egrs_non_dly_qm_crdt,
    cli_dsptchr_egrs_queues_total_q_egrs_size,
    cli_dsptchr_egrs_queues_per_q_egrs_size,
    cli_dsptchr_wakeup_control_wkup_req,
    cli_dsptchr_wakeup_control_wkup_thrshld,
    cli_dsptchr_disptch_scheduling_dwrr_info,
    cli_dsptchr_disptch_scheduling_vld_crdt,
    cli_dsptchr_load_balancing_lb_cfg,
    cli_dsptchr_load_balancing_free_task_0_1,
    cli_dsptchr_load_balancing_free_task_2_3,
    cli_dsptchr_load_balancing_free_task_4_5,
    cli_dsptchr_load_balancing_free_task_6_7,
    cli_dsptchr_load_balancing_free_task_8_9,
    cli_dsptchr_load_balancing_free_task_10_11,
    cli_dsptchr_load_balancing_free_task_12_13,
    cli_dsptchr_load_balancing_free_task_14_15,
    cli_dsptchr_load_balancing_tsk_to_rg_mapping,
    cli_dsptchr_load_balancing_rg_avlabl_tsk_0_3,
    cli_dsptchr_load_balancing_rg_avlabl_tsk_4_7,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ism,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_ier,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_0_itr,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_isr,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ism,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_ier,
    cli_dsptchr_dsptcher_reordr_top_intr_ctrl_1_itr,
    cli_dsptchr_debug_dbg_bypss_cntrl,
    cli_dsptchr_debug_glbl_tsk_cnt_0_7,
    cli_dsptchr_debug_glbl_tsk_cnt_8_15,
    cli_dsptchr_debug_dbg_bus_cntrl,
    cli_dsptchr_debug_dbg_vec_0,
    cli_dsptchr_debug_dbg_vec_1,
    cli_dsptchr_debug_dbg_vec_2,
    cli_dsptchr_debug_dbg_vec_3,
    cli_dsptchr_debug_dbg_vec_4,
    cli_dsptchr_debug_dbg_vec_5,
    cli_dsptchr_debug_dbg_vec_6,
    cli_dsptchr_debug_dbg_vec_7,
    cli_dsptchr_debug_dbg_vec_8,
    cli_dsptchr_debug_dbg_vec_9,
    cli_dsptchr_debug_dbg_vec_10,
    cli_dsptchr_debug_dbg_vec_11,
    cli_dsptchr_debug_dbg_vec_12,
    cli_dsptchr_debug_dbg_vec_13,
    cli_dsptchr_debug_dbg_vec_14,
    cli_dsptchr_debug_dbg_vec_15,
    cli_dsptchr_debug_dbg_vec_16,
    cli_dsptchr_debug_dbg_vec_17,
    cli_dsptchr_debug_dbg_vec_18,
    cli_dsptchr_debug_dbg_vec_19,
    cli_dsptchr_debug_dbg_vec_20,
    cli_dsptchr_debug_dbg_vec_21,
    cli_dsptchr_debug_dbg_vec_22,
    cli_dsptchr_debug_dbg_vec_23,
    cli_dsptchr_debug_statistics_dbg_sttstcs_ctrl,
    cli_dsptchr_debug_statistics_dbg_cnt,
    cli_dsptchr_qdes_head,
    cli_dsptchr_qdes_bfout,
    cli_dsptchr_qdes_bufin,
    cli_dsptchr_qdes_tail,
    cli_dsptchr_qdes_fbdnull,
    cli_dsptchr_qdes_nullbd,
    cli_dsptchr_qdes_bufavail,
    cli_dsptchr_qdes_reg_q_head,
    cli_dsptchr_qdes_reg_viq_head_vld,
    cli_dsptchr_qdes_reg_viq_chrncy_vld,
    cli_dsptchr_qdes_reg_veq_head_vld,
    cli_dsptchr_qdes_reg_qdes_buf_avl_cntrl,
    cli_dsptchr_flldes_flldrop,
    cli_dsptchr_flldes_bufavail,
    cli_dsptchr_flldes_freemin,
    cli_dsptchr_bdram_data,
    cli_dsptchr_pdram_data,
};

int bcm_dsptchr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dsptchr_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

