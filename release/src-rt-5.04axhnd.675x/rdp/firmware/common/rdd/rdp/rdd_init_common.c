/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
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
#include "rdd.h"
#include "rdd_init_common.h"
#include "rdd_proj_init.h"
#include "rdp_mm.h"

DEFINE_BDMF_FASTLOCK(int_lock_irq);

int rdd_init(void)
{
#ifdef FIRMWARE_INIT
    if (rdd_sim_alloc_segments())
        return -1;
#endif

    /* reset SRAM program memory of both Runners */
    rdp_mm_setl((RUNNER_INST_MAIN *)DEVICE_ADDRESS(RUNNER_INST_MAIN_0_OFFSET), 0, sizeof(RUNNER_INST_MAIN));
    rdp_mm_setl((RUNNER_INST_MAIN *)DEVICE_ADDRESS(RUNNER_INST_MAIN_1_OFFSET), 0, sizeof(RUNNER_INST_MAIN));
    rdp_mm_setl((RUNNER_INST_PICO *)DEVICE_ADDRESS(RUNNER_INST_PICO_0_OFFSET), 0, sizeof(RUNNER_INST_PICO));
    rdp_mm_setl((RUNNER_INST_PICO *)DEVICE_ADDRESS(RUNNER_INST_PICO_1_OFFSET), 0, sizeof(RUNNER_INST_PICO));

    /* reset SRAM common data memory of both Runners */
    rdp_mm_setl((RUNNER_COMMON *)DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET), 0, sizeof(RUNNER_COMMON));
    rdp_mm_setl((RUNNER_COMMON *)DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET), 0, sizeof(RUNNER_COMMON));

    /* reset SRAM private data memory of both Runners */
    rdp_mm_setl((RUNNER_PRIVATE *)DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET), 0, sizeof(RUNNER_PRIVATE));
    rdp_mm_setl((RUNNER_PRIVATE *)DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET), 0, sizeof(RUNNER_PRIVATE));

    /* reset SRAM context memory of both Runners */
    rdp_mm_setl_context((RUNNER_CNTXT_MAIN *)DEVICE_ADDRESS(RUNNER_CNTXT_MAIN_0_OFFSET), 0, sizeof(RUNNER_CNTXT_MAIN));
    rdp_mm_setl_context((RUNNER_CNTXT_MAIN *)DEVICE_ADDRESS(RUNNER_CNTXT_MAIN_1_OFFSET), 0, sizeof(RUNNER_CNTXT_MAIN));
    rdp_mm_setl_context((RUNNER_CNTXT_PICO *)DEVICE_ADDRESS(RUNNER_CNTXT_PICO_0_OFFSET), 0, sizeof(RUNNER_CNTXT_PICO));
    rdp_mm_setl_context((RUNNER_CNTXT_PICO *)DEVICE_ADDRESS(RUNNER_CNTXT_PICO_1_OFFSET), 0, sizeof(RUNNER_CNTXT_PICO));

    /* reset SRAM prediction memory of both Runners */
    rdp_mm_setl((RUNNER_PRED_MAIN *)DEVICE_ADDRESS(RUNNER_PRED_MAIN_0_OFFSET), 0, sizeof(RUNNER_PRED_MAIN) * 2);
    rdp_mm_setl((RUNNER_PRED_MAIN *)DEVICE_ADDRESS(RUNNER_PRED_MAIN_1_OFFSET), 0, sizeof(RUNNER_PRED_MAIN) * 2);
    rdp_mm_setl((RUNNER_PRED_PICO *)DEVICE_ADDRESS(RUNNER_PRED_PICO_0_OFFSET), 0, sizeof(RUNNER_PRED_PICO) * 2);
    rdp_mm_setl((RUNNER_PRED_PICO *)DEVICE_ADDRESS(RUNNER_PRED_PICO_1_OFFSET), 0, sizeof(RUNNER_PRED_PICO) * 2);

    return 0;
}

void rdd_load_microcode(uint8_t *runner_a_microcode, uint8_t *runner_b_microcode, uint8_t *runner_c_microcode,
    uint8_t *runner_d_microcode)
{
    MWRITE_BLK_32(DEVICE_ADDRESS(RUNNER_INST_MAIN_0_OFFSET), runner_a_microcode, sizeof(RUNNER_INST_MAIN));
    MWRITE_BLK_32(DEVICE_ADDRESS(RUNNER_INST_MAIN_1_OFFSET), runner_b_microcode, sizeof(RUNNER_INST_MAIN));
    MWRITE_BLK_32(DEVICE_ADDRESS(RUNNER_INST_PICO_0_OFFSET), runner_c_microcode, sizeof(RUNNER_INST_PICO));
    MWRITE_BLK_32(DEVICE_ADDRESS(RUNNER_INST_PICO_1_OFFSET), runner_d_microcode, sizeof(RUNNER_INST_PICO));
}

void memcpyl_prediction(void *__to, void *__from, unsigned int __n)
{
    uint8_t *src = __from;
    uint8_t *dst = __to;
    int i;

    for (i = 0; i < (__n / 2); i++, src += 2, dst += 4)
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
        *(volatile unsigned int *)dst = swap4bytes((unsigned int)(*(volatile unsigned short *)src));
#else
        *(volatile unsigned int *)dst = (unsigned int)(*(volatile unsigned short *)src);
#endif
}

void rdd_load_prediction(uint8_t *runner_a_predict, uint8_t *runner_b_predict, uint8_t *runner_c_predict,
    uint8_t *runner_d_predict)
{
    memcpyl_prediction((void *)DEVICE_ADDRESS(RUNNER_PRED_MAIN_0_OFFSET), runner_a_predict, sizeof(RUNNER_PRED_MAIN));
    memcpyl_prediction((void *)DEVICE_ADDRESS(RUNNER_PRED_MAIN_1_OFFSET), runner_b_predict, sizeof(RUNNER_PRED_MAIN));
    memcpyl_prediction((void *)DEVICE_ADDRESS(RUNNER_PRED_PICO_0_OFFSET), runner_c_predict, sizeof(RUNNER_PRED_PICO));
    memcpyl_prediction((void *)DEVICE_ADDRESS(RUNNER_PRED_PICO_1_OFFSET), runner_d_predict, sizeof(RUNNER_PRED_PICO));
}

static void _rdd_runner_enable(bdmf_boolean enable)
{
    RUNNER_REGS_CFG_GLOBAL_CTRL runner_global_ctrl_reg;

    /* enable Runner A through the global control register */
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_READ(runner_global_ctrl_reg);
    runner_global_ctrl_reg.main_en = enable;
    runner_global_ctrl_reg.pico_en = enable;
    runner_global_ctrl_reg.main_cntxt_reb_en = enable;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE(runner_global_ctrl_reg);

    /* enable Runner B through the global control register */
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ(runner_global_ctrl_reg);
    runner_global_ctrl_reg.main_en = enable;
    runner_global_ctrl_reg.pico_en = enable;
    runner_global_ctrl_reg.main_cntxt_reb_en = enable;
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_WRITE(runner_global_ctrl_reg);
}

void rdd_runner_enable(void)
{
    _rdd_runner_enable(1);
}

void rdd_runner_disable(void)
{
    _rdd_runner_enable(0);
}

void rdd_runner_frequency_set(uint16_t freq)
{
    RUNNER_REGS_CFG_GLOBAL_CTRL runner_global_ctrl_reg;

    /* set the frequency of the Runner through the global control register */
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_READ(runner_global_ctrl_reg);
    runner_global_ctrl_reg.micro_sec_val = freq - 1;
    RUNNER_REGS_0_CFG_GLOBAL_CTRL_WRITE(runner_global_ctrl_reg);

    RUNNER_REGS_1_CFG_GLOBAL_CTRL_READ(runner_global_ctrl_reg);
    runner_global_ctrl_reg.micro_sec_val = freq - 1;
    RUNNER_REGS_1_CFG_GLOBAL_CTRL_WRITE(runner_global_ctrl_reg);
}

void rdd_scheduler_init(void)
{
#if defined(FIRMWARE_INIT)
    uint32_t runner_scheduler_cfg_register;

    /* main Runner A - class C (Oren) */
    runner_scheduler_cfg_register = (RUNNER_REGS_CFG_MAIN_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_B_USE_CLASS_B_VALUE << 5) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_A_DONT_USE_CLASS_A_VALUE << 4) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_31_24_RR_VALUE << 3) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_23_16_RR_VALUE << 2) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_15_8_RR_VALUE << 1) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0);

    RUNNER_REGS_0_CFG_MAIN_SCH_CFG_WRITE(runner_scheduler_cfg_register);

    /* main Runner B - class C (Oren) */
    runner_scheduler_cfg_register = (RUNNER_REGS_CFG_MAIN_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_B_USE_CLASS_B_VALUE << 5) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_A_DONT_USE_CLASS_A_VALUE << 4) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_31_24_RR_VALUE << 3) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_23_16_RR_VALUE << 2) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_15_8_RR_VALUE << 1) |
                                    (RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0);

    RUNNER_REGS_1_CFG_MAIN_SCH_CFG_WRITE(runner_scheduler_cfg_register);

    /* pico Runner A - class A */
    runner_scheduler_cfg_register = (RUNNER_REGS_CFG_PICO_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_A_USE_CLASS_A_VALUE << 4) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_15_8_RR_VALUE << 1) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0);

    RUNNER_REGS_0_CFG_PICO_SCH_CFG_WRITE(runner_scheduler_cfg_register);

    /* pico Runner B - class A */
    runner_scheduler_cfg_register = (RUNNER_REGS_CFG_PICO_SCH_CFG_ARB_CLASS_USE_RR_VALUE << 6) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE << 5) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_A_USE_CLASS_A_VALUE << 4) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_15_8_RR_VALUE << 1) |
                                    (RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_STRICT_VALUE << 0);

    RUNNER_REGS_1_CFG_PICO_SCH_CFG_WRITE(runner_scheduler_cfg_register);
#else
    RUNNER_REGS_CFG_MAIN_SCH_CFG runner_cfg_main_sched;
    RUNNER_REGS_CFG_PICO_SCH_CFG runner_cfg_pico_sched;

    /* main Runner A - class C (Oren) */
    runner_cfg_main_sched.arb_class = RUNNER_REGS_CFG_MAIN_SCH_CFG_ARB_CLASS_USE_RR_VALUE;
    runner_cfg_main_sched.use_class_b = RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_B_USE_CLASS_B_VALUE;
    runner_cfg_main_sched.use_class_a = RUNNER_REGS_CFG_MAIN_SCH_CFG_USE_CLASS_A_DONT_USE_CLASS_A_VALUE;
    runner_cfg_main_sched.class_7_0 = RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_7_0_STRICT_VALUE;
    runner_cfg_main_sched.class_15_8 = RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_15_8_RR_VALUE;
    runner_cfg_main_sched.class_23_16 = RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_23_16_RR_VALUE;
    runner_cfg_main_sched.class_31_24 = RUNNER_REGS_CFG_MAIN_SCH_CFG_CLASS_31_24_RR_VALUE;

    RUNNER_REGS_0_CFG_MAIN_SCH_CFG_WRITE(runner_cfg_main_sched);
    RUNNER_REGS_1_CFG_MAIN_SCH_CFG_WRITE(runner_cfg_main_sched);

    /* pico Runner A - class A */

    runner_cfg_pico_sched.arb_class = RUNNER_REGS_CFG_PICO_SCH_CFG_ARB_CLASS_USE_RR_VALUE;
    runner_cfg_pico_sched.use_class_b = RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_B_DONT_USE_CLASS_B_VALUE;
    runner_cfg_pico_sched.use_class_a = RUNNER_REGS_CFG_PICO_SCH_CFG_USE_CLASS_A_USE_CLASS_A_VALUE;
#if defined(WL4908)
    runner_cfg_pico_sched.class_7_0 = RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_RR_VALUE;
#else
    runner_cfg_pico_sched.class_7_0 = RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_7_0_STRICT_VALUE;
#endif
    runner_cfg_pico_sched.class_15_8 = RUNNER_REGS_CFG_PICO_SCH_CFG_CLASS_15_8_RR_VALUE;

    RUNNER_REGS_0_CFG_PICO_SCH_CFG_WRITE(runner_cfg_pico_sched);
    RUNNER_REGS_1_CFG_PICO_SCH_CFG_WRITE(runner_cfg_pico_sched);
#endif
}

static void rdd_local_regs_main_a_common_init(void *data)
{
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
#ifdef DS_DYNAMIC_DISPATCH
    /* WAN dispatch: thread 10 */
    (*local_regs)[WAN_RX_DISPATCH_THREAD_NUMBER][R16] = ADDRESS_OF(runner_a, rx_dispatch_wakeup_request) << 16;
    (*local_regs)[WAN_RX_DISPATCH_THREAD_NUMBER][R8] = WAN_RX_NORMAL_DESCRIPTORS_ADDRESS;
    (*local_regs)[WAN_RX_DISPATCH_THREAD_NUMBER][R9] = DS_RX_DISPATCH_PROCESSING_TASKS_VECTOR_ADDRESS;
    (*local_regs)[WAN_RX_DISPATCH_THREAD_NUMBER][R10] = DS_INGRESS_HANDLER_BUFFER_ADDRESS;
    (*local_regs)[WAN_RX_DISPATCH_THREAD_NUMBER][R11] = BBH_PERIPHERAL_WAN_RX;
    (*local_regs)[WAN_RX_DISPATCH_THREAD_NUMBER][R12] = DS_RX_DISPATCH_PROCESSING_TASKS_WAKEUP_REQUESTS_TABLE_ADDRESS;
#endif
#endif
}

static void rdd_local_regs_main_b_common_init(void *data)
{
}

static void rdd_local_regs_pico_a_common_init(void *data)
{
}

static void rdd_local_regs_pico_b_common_init(void *data)
{
}

void rdd_local_registers_init(void)
{
    RUNNER_CNTXT_MAIN *sram_main_context;
    RUNNER_CNTXT_PICO *sram_pico_context;
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][32];

    /********** Fast Runner A **********/
    sram_main_context = (RUNNER_CNTXT_MAIN *)DEVICE_ADDRESS(RUNNER_CNTXT_MAIN_0_OFFSET);

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_main_context, sizeof(RUNNER_CNTXT_MAIN));

    rdd_local_regs_main_a_common_init(&local_regs);
    rdd_local_regs_main_a_proj_init(&local_regs);

#if defined(FIRMWARE_INIT) || defined(FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_main_context, local_regs, sizeof(RUNNER_CNTXT_MAIN));
#else
    rdp_mm_cpyl_context(sram_main_context, local_regs, sizeof(RUNNER_CNTXT_MAIN));
#endif

    /********** Fast Runner B **********/
    sram_main_context = (RUNNER_CNTXT_MAIN *)DEVICE_ADDRESS(RUNNER_CNTXT_MAIN_1_OFFSET);

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_main_context, sizeof(RUNNER_CNTXT_MAIN));

    rdd_local_regs_main_b_common_init(&local_regs);
    rdd_local_regs_main_b_proj_init(&local_regs);

#if defined(FIRMWARE_INIT) || defined(FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_main_context, local_regs, sizeof(RUNNER_CNTXT_MAIN));
#else
    rdp_mm_cpyl_context(sram_main_context, local_regs, sizeof(RUNNER_CNTXT_MAIN));
#endif

    /********** Pico Runner A **********/

    sram_pico_context = (RUNNER_CNTXT_PICO *)DEVICE_ADDRESS(RUNNER_CNTXT_PICO_0_OFFSET);

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_pico_context, sizeof(RUNNER_CNTXT_PICO));

    rdd_local_regs_pico_a_common_init(&local_regs);
    rdd_local_regs_pico_a_proj_init(&local_regs);

#if defined(FIRMWARE_INIT) || defined(FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_pico_context, local_regs, sizeof(RUNNER_CNTXT_PICO));
#else
    rdp_mm_cpyl_context(sram_pico_context, local_regs, sizeof(RUNNER_CNTXT_PICO));
#endif

    /********** Pico Runner B **********/

    sram_pico_context = (RUNNER_CNTXT_PICO *)DEVICE_ADDRESS(RUNNER_CNTXT_PICO_1_OFFSET);

    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_pico_context, sizeof(RUNNER_CNTXT_PICO));

    rdd_local_regs_pico_b_common_init(&local_regs);
    rdd_local_regs_pico_b_proj_init(&local_regs);

#if defined(FIRMWARE_INIT) || defined(FSSIM)
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_pico_context, local_regs, sizeof(RUNNER_CNTXT_PICO));
#else
    rdp_mm_cpyl_context(sram_pico_context, local_regs, sizeof(RUNNER_CNTXT_PICO));
#endif
}

void rdd_pm_counters_init(void)
{
    RUNNER_REGS_CFG_CNTR_CFG  runner_counter_cfg_register;

    runner_counter_cfg_register.base_address = (PM_COUNTERS_ADDRESS >> 3);

    RUNNER_REGS_0_CFG_CNTR_CFG_WRITE(runner_counter_cfg_register);
    RUNNER_REGS_1_CFG_CNTR_CFG_WRITE(runner_counter_cfg_register);
}
