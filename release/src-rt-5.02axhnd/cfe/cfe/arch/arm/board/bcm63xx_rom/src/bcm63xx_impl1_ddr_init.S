/*
   <:copyright-BRCM:2013:DUAL/GPL:standard
   
      Copyright (c) 2013 Broadcom 
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

#include "arm.h"
#include "armmacros.h"
#include "bcm_map.h"

#define SPIN_CYCLES                 \
    mov r1, #0;                     \
1:  cmp r0, r1;                     \
beq 2f;                             \
    add r1, r1, #1;                 \
    b 1b;                           \
2:  nop

	
#define USE_PRE_COMPUTED_CONFIG_1
	
#ifdef USE_PRE_COMPUTED_CONFIG_1
// we use this hard-coded constant, derived from the MemcSupport::mc_cfg_init() code.
#define MEMC_GLB_GCFG_VAL_first 0x01200044
#define PHY_CONTROL_REGS_DRAM_CONFIG_VAL 0x1510
#define PHY_CONTROL_REGS_DRAM_TIMING1_VAL 0x1c060a0a
#define PHY_CONTROL_REGS_DRAM_TIMING2_VAL 0x60c080a
#define PHY_CONTROL_REGS_DRAM_TIMING3_VAL 0x46058
#define PHY_CONTROL_REGS_DRAM_TIMING4_VAL 0x0
#define PHY_CONTROL_REGS_PLL_CONFIG_VAL_first 0x39d0012
#define PHY_CONTROL_REGS_PLL_DIVIDERS_VAL 0x2030bf
#define PHY_CONTROL_REGS_PLL_CONTROL2_VAL 0x94000000
#define PHY_CONTROL_REGS_PLL_CONFIG_VAL_second 0x39d0010
#define PHY_CONTROL_REGS_PLL_CONFIG_VAL_third 0x39d0000
#define MEMC_GLB_GCFG_VAL_second 0x1200244
#define PHY_CONTROL_REGS_VREF_DAC_CONTROL_VAL 0x820
#define PHY_CONTROL_REGS_VDL_CALIBRATE_VAL_first 0x0
#define PHY_CONTROL_REGS_VDL_CALIBRATE_VAL_second 0x19
#define PHY_CONTROL_REGS_VDL_CALIBRATE_VAL_third 0x0
#define PHY_BYTE_LANE_0_RD_EN_DLY_CYC_VAL 0x10011
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0_VAL 0x10024
#define PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1_VAL 0x10024
#define PHY_BYTE_LANE_1_RD_EN_DLY_CYC_VAL 0x10011
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0_VAL 0x10024
#define PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1_VAL 0x10024
#define PHY_CONTROL_REGS_DRIVE_PAD_CTL_VAL 0x31bff
#define PHY_BYTE_LANE_0_DRIVE_PAD_CTL_VAL 0x31bff
#define PHY_BYTE_LANE_1_DRIVE_PAD_CTL_VAL 0x31bff
#define PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL_VAL 0xfffff
#define PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL_VAL 0xfffff
#define PHY_CONTROL_REGS_STATIC_PAD_CTL_VAL 0x1000000
#define PHY_BYTE_LANE_0_WR_PREAMBLE_MODE_VAL 0xe02
#define PHY_BYTE_LANE_1_WR_PREAMBLE_MODE_VAL 0xe02
#define PHY_BYTE_LANE_0_IDLE_PAD_CONTROL_VAL 0xa
#define PHY_BYTE_LANE_1_IDLE_PAD_CONTROL_VAL 0xa
#define PHY_CONTROL_REGS_DFI_CNTRL_VAL_first 0x61
#define PHY_CONTROL_REGS_DFI_CNTRL_VAL_second 0x71
#define PHY_CONTROL_REGS_DFI_CNTRL_VAL_third 0x79
#define PHY_CONTROL_REGS_COMMAND_AUX_REG1VAL 0x0
#define PHY_CONTROL_REGS_COMMAND_REG1_MR2_VAL 0x20218
#define PHY_CONTROL_REGS_COMMAND_REG1_MR3_VAL 0x30000
#define PHY_CONTROL_REGS_COMMAND_REG1_MR1_VAL 0x10006
#define PHY_CONTROL_REGS_COMMAND_REG1_MR0_VAL 0x1d60
#define PHY_CONTROL_REGS_COMMAND_REG1_VAL_cal 0x300400
#define PHY_CONTROL_REGS_DFI_CNTRL_VAL 0x78
#define MEMC_GLB_CFG_VAL 0x0
#define MEMC_GLB_SP_SEL_VAL 0x00000000
#define MEMC_GLB_SP_PRI_0_VAL 0x3020100
#define MEMC_GLB_SP_PRI_1_VAL 0x7060504
#define MEMC_GLB_SP_PRI_2_VAL 0xb0a0908
#define MEMC_GLB_SP_PRI_3_VAL 0xf0e0d0c
#define MEMC_GLB_SP_PRI_4_VAL 0x13121110
#define MEMC_GLB_SP_PRI_5_VAL 0x1514
#define MEMC_GLB_RR_QUANTUM0_VAL 0x10001  
#define MEMC_GLB_RR_QUANTUM1_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM2_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM3_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM4_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM5_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM6_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM7_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM8_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM9_VAL 0x10001
#define MEMC_GLB_RR_QUANTUM10_VAL 0x1
#define MEMC_UBUSIF_0_CFG_VAL 0x0
#define MEMC_UBUSIF_0_REP_ARB_MODE_VAL 0x0
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_0_VAL 0x0
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_1_VAL 0x0
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_2_VAL 0x0
#define MEMC_UBUSIF_0_SRC_QUEUE_CTRL_3_VAL 0x0
#define MEMC_UBUSIF_1_CFG_VAL 0x0
#define MEMC_UBUSIF_1_REP_ARB_MODE_VAL 0x0
#define MEMC_AXIRIF_0_REP_ARB_MODE_VAL 0x0
#define MEMC_AXIWIF_0_CFG_VAL 0x0
#define MEMC_AXIWIF_0_REP_ARB_MODE_VAL 0x0
#define MEMC_CHN_TIM_DCMD_VAL 0x00000305
#define MEMC_CHN_TIM_CLKS_VAL_first 0x1c000
// we use this hard-coded constant, derived from the MemcSupport::load_csr_map_x16() code.
#define MEMC_CHN_CFG_BNK10_VAL 0x00080706
#define MEMC_CHN_CFG_COL00_0_VAL 0x04000000
#define MEMC_CHN_CFG_COL00_1_VAL 0x0b0a0905
#define MEMC_CHN_CFG_COL01_0_VAL 0x00000d0c
#define MEMC_CHN_CFG_COL01_1_VAL 0x00000000
#define MEMC_CHN_CFG_ROW00_0_VAL 0x11100f0e
#define MEMC_CHN_CFG_ROW00_1_VAL 0x15141312
#define MEMC_CHN_CFG_ROW01_0_VAL 0x19181716
#define MEMC_CHN_CFG_ROW01_1_VAL 0x0000001a
#define MEMC_CHN_TIM_TIM1_0_VAL 0x6a8caa
#define MEMC_CHN_TIM_TIM1_1_VAL 0x1a582026
#define MEMC_CHN_TIM_TIM2_VAL 0x6060
#define MEMC_CHN_TIM_DRAM_CFG_VAL 0x101
#define MEMC_CHN_TIM_CLKS_VAL_second 0xc000
#define MEMC_GLB_GCFG_VAL_third (0x1200344 | MEMC_GLB_GCFG_MEM_INIT_DONE | MEMC_GLB_GCFG_GCFG_DRAM_EN)
	
#endif
	
FUNC(ddr_init)
#ifdef CONFIG_BRCM_IKOS
    cmp r0, #0
    bne 1f
	
    /* Bench configures memc, wait for done */
poll_memc_init_done_loop:	
    ldr   r1, =MEMC_BASE
    ldr   r0, [r1, #MEMC_GLB_GCFG]
    tst   r0, #MEMC_GLB_GCFG_MEM_INIT_DONE  
    beq   poll_memc_init_done_loop

    ldr   r0, [r1, #MEMC_GLB_GCFG]
    orr   r0, r0, #MEMC_GLB_GCFG_GCFG_DRAM_EN
    str   r0, [r1, #MEMC_GLB_GCFG]

    mov   pc, lr
#endif

1:
    // save the return address
    mov r8, lr

#ifdef USE_PRE_COMPUTED_CONFIG_1

    // load the base addresses
    ldr r9, =MEMC_BASE
    ldr r10, =MEMC_BASE_OFF_4K
    
    // MEMC_GLB_GCFG
    ldr r0, =MEMC_GLB_GCFG_VAL_first
    str r0, [r9, #MEMC_GLB_GCFG]
    
    // mov r0, #200 // #1000
    // bl spin_Cycles  
    // Poll the PHY Status Register
    // loop while power down, or resets(SW/HW) up...
    SETLEDS('P','H','Y','S')
    mov r0, #1 // #200
poll_phy_status:
    SPIN_CYCLES
    ldr r1, [r9, #MEMC_CHN_TIM_PHY_ST]
    tst r1, #MEMC_CHN_TIM_PHY_ST_PHY_PWR_UP
    beq poll_phy_status
    tst r1, #MEMC_CHN_TIM_PHY_ST_SW_RESET
    bne poll_phy_status
    tst r1, #MEMC_CHN_TIM_PHY_ST_HW_RESET
    bne poll_phy_status
    
    ldr r0, =PHY_CONTROL_REGS_DRAM_CONFIG_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DRAM_CONFIG_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_DRAM_TIMING1_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DRAM_TIMING1_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_DRAM_TIMING2_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DRAM_TIMING2_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_DRAM_TIMING3_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DRAM_TIMING3_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_DRAM_TIMING4_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DRAM_TIMING4_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_PLL_CONFIG_VAL_first
    str r0, [r10, #PHY_CONTROL_REGS_PLL_CONFIG_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_PLL_DIVIDERS_VAL
    str r0, [r10, #PHY_CONTROL_REGS_PLL_DIVIDERS_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_PLL_CONTROL2_VAL
    str r0, [r10, #PHY_CONTROL_REGS_PLL_CONTROL2_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_PLL_CONFIG_VAL_second
    str r0, [r10, #PHY_CONTROL_REGS_PLL_CONFIG_OFF_4K]
	
    //mov r0, #20 // #1000
    //SPIN_CYCLES  
    //DDR PHY PLL polling for lock...
    mov r0, #1 // #1000
wait_for_pll_lock:	
    SPIN_CYCLES
    ldr r1, [r10, #PHY_CONTROL_REGS_PLL_STATUS_OFF_4K]
    tst r1, #PHY_CONTROL_REGS_PLL_STATUS_LOCK
    beq wait_for_pll_lock
    
    
    ldr r0, =PHY_CONTROL_REGS_PLL_CONFIG_VAL_third
    str r0, [r10, #PHY_CONTROL_REGS_PLL_CONFIG_OFF_4K]
	
    ldr r0, =MEMC_GLB_GCFG_VAL_second
    str r0, [r9, #MEMC_GLB_GCFG]
	
    ldr r0, =PHY_CONTROL_REGS_VREF_DAC_CONTROL_VAL
    str r0, [r10, #PHY_CONTROL_REGS_VREF_DAC_CONTROL_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_VDL_CALIBRATE_VAL_first
    str r0, [r10, #PHY_CONTROL_REGS_VDL_CALIBRATE_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_VDL_CALIBRATE_VAL_second
    str r0, [r10, #PHY_CONTROL_REGS_VDL_CALIBRATE_OFF_4K]

    SETLEDS('P','L','L','E')
	
    //mov r0, #20 // #1000
    //SPIN_CYCLES      
    // DDR PHY calibration polling for lock...
    mov r0, #1 // #1000
wait_for_calibration_lock:	
    SPIN_CYCLES 
    ldr r1, [r10, #PHY_CONTROL_REGS_VDL_CALIB_STATUS1_OFF_4K]
    tst r1, #PHY_CONTROL_REGS_VDL_CALIB_STATUS1_IDLE
    beq wait_for_calibration_lock
	
    SETLEDS('C','A','L','E')
    
    ldr r0, =PHY_CONTROL_REGS_VDL_CALIBRATE_VAL_third
    str r0, [r10, #PHY_CONTROL_REGS_VDL_CALIBRATE_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_RD_EN_DLY_CYC_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_RD_EN_DLY_CYC_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS0_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_VDL_CONTROL_RD_EN_CS1_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_RD_EN_DLY_CYC_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_RD_EN_DLY_CYC_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS0_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_VDL_CONTROL_RD_EN_CS1_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_DRIVE_PAD_CTL_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DRIVE_PAD_CTL_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_DRIVE_PAD_CTL_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_DRIVE_PAD_CTL_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_DRIVE_PAD_CTL_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_DRIVE_PAD_CTL_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_RD_EN_DRIVE_PAD_CTL_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_RD_EN_DRIVE_PAD_CTL_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_STATIC_PAD_CTL_VAL
    str r0, [r10, #PHY_CONTROL_REGS_STATIC_PAD_CTL_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_WR_PREAMBLE_MODE_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_WR_PREAMBLE_MODE_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_WR_PREAMBLE_MODE_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_WR_PREAMBLE_MODE_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_0_IDLE_PAD_CONTROL_VAL
    str r0, [r10, #PHY_BYTE_LANE_0_IDLE_PAD_CONTROL_OFF_4K]
    
    ldr r0, =PHY_BYTE_LANE_1_IDLE_PAD_CONTROL_VAL
    str r0, [r10, #PHY_BYTE_LANE_1_IDLE_PAD_CONTROL_OFF_4K]
    
    mov r0, #10 // #400
    SPIN_CYCLES 
    
    ldr r0, =PHY_CONTROL_REGS_DFI_CNTRL_VAL_first
    str r0, [r10, #PHY_CONTROL_REGS_DFI_CNTRL_OFF_4K]
    
    // wait 10us
    mov r0, #20 //#10000
    SPIN_CYCLES
    
    
    ldr r0, =PHY_CONTROL_REGS_DFI_CNTRL_VAL_second
    str r0, [r10, #PHY_CONTROL_REGS_DFI_CNTRL_OFF_4K]
    
    // wait 1us
    mov r0, #10 // #1000
    SPIN_CYCLES
    
    
    ldr r0, =PHY_CONTROL_REGS_DFI_CNTRL_VAL_third
    str r0, [r10, #PHY_CONTROL_REGS_DFI_CNTRL_OFF_4K]
    
    // wait 1us
    mov r0, #10 // #1000
    SPIN_CYCLES 
    
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_AUX_REG1VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_AUX_REG1_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_REG1_MR2_VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_REG1_OFF_4K]
    
    // wait 200ns
    mov r0, #10 // #200
    SPIN_CYCLES 
    
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_AUX_REG1VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_AUX_REG1_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_REG1_MR3_VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_REG1_OFF_4K]
    
    // wait 200ns
    mov r0, #10 // #200
    SPIN_CYCLES
    
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_AUX_REG1VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_AUX_REG1_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_REG1_MR1_VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_REG1_OFF_4K]
    
    // wait 200ns
    mov r0, #10 // #200
    SPIN_CYCLES 
    
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_AUX_REG1VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_AUX_REG1_OFF_4K]
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_REG1_MR0_VAL
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_REG1_OFF_4K]
    
    // wait 200ns
    mov r0, #10 // #200
    SPIN_CYCLES
    
    
    ldr r0, =PHY_CONTROL_REGS_COMMAND_REG1_VAL_cal
    str r0, [r10, #PHY_CONTROL_REGS_COMMAND_REG1_OFF_4K]
    
    // wait 1500ns
    mov r0, #10 // #1500
    SPIN_CYCLES
    
    
    ldr r0, =PHY_CONTROL_REGS_DFI_CNTRL_VAL
    str r0, [r10, #PHY_CONTROL_REGS_DFI_CNTRL_OFF_4K]

    // wait 5000ns
    mov r0, #10 // #5000   
    SPIN_CYCLES 

    SETLEDS('P','H','Y','E')
	
    ldr r0, =MEMC_GLB_CFG_VAL
    str r0, [r9, #MEMC_GLB_CFG]
    
    ldr r0, =MEMC_GLB_SP_SEL_VAL
    str r0, [r9, #MEMC_GLB_SP_SEL]
    
    ldr r0, =MEMC_GLB_SP_PRI_0_VAL
    str r0, [r9, #MEMC_GLB_SP_PRI_0]
    
    ldr r0, =MEMC_GLB_SP_PRI_1_VAL
    str r0, [r9, #MEMC_GLB_SP_PRI_1]
    
    ldr r0, =MEMC_GLB_SP_PRI_2_VAL
    str r0, [r9, #MEMC_GLB_SP_PRI_2]
    
    ldr r0, =MEMC_GLB_SP_PRI_3_VAL
    str r0, [r9, #MEMC_GLB_SP_PRI_3]
    
    ldr r0, =MEMC_GLB_SP_PRI_4_VAL
    str r0, [r9, #MEMC_GLB_SP_PRI_4]
    
    ldr r0, =MEMC_GLB_SP_PRI_5_VAL
    str r0, [r9, #MEMC_GLB_SP_PRI_5]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM0_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM0]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM1_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM1]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM2_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM2]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM3_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM3]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM4_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM4]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM5_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM5]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM6_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM6]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM7_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM7]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM8_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM8]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM9_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM9]
    
    ldr r0, =MEMC_GLB_RR_QUANTUM10_VAL
    str r0, [r9, #MEMC_GLB_RR_QUANTUM10]
    
    ldr r0, =MEMC_UBUSIF_0_CFG_VAL
    str r0, [r9, #MEMC_UBUSIF_0_CFG]
    
    ldr r0, =MEMC_UBUSIF_0_REP_ARB_MODE_VAL
    str r0, [r9, #MEMC_UBUSIF_0_REP_ARB_MODE]
    
    ldr r0, =MEMC_UBUSIF_0_SRC_QUEUE_CTRL_0_VAL
    str r0, [r9, #MEMC_UBUSIF_0_SRC_QUEUE_CTRL_0]
    
    ldr r0, =MEMC_UBUSIF_0_SRC_QUEUE_CTRL_1_VAL
    str r0, [r9, #MEMC_UBUSIF_0_SRC_QUEUE_CTRL_1]
    
    ldr r0, =MEMC_UBUSIF_0_SRC_QUEUE_CTRL_2_VAL
    str r0, [r9, #MEMC_UBUSIF_0_SRC_QUEUE_CTRL_2]
    
    ldr r0, =MEMC_UBUSIF_0_SRC_QUEUE_CTRL_3_VAL
    str r0, [r9, #MEMC_UBUSIF_0_SRC_QUEUE_CTRL_3]
    
    ldr r0, =MEMC_UBUSIF_1_CFG_VAL
    str r0, [r9, #MEMC_UBUSIF_1_CFG]
    
    ldr r0, =MEMC_UBUSIF_1_REP_ARB_MODE_VAL
    str r0, [r9, #MEMC_UBUSIF_1_REP_ARB_MODE]
    
    ldr r0, =MEMC_AXIRIF_0_REP_ARB_MODE_VAL
    str r0, [r9, #MEMC_AXIRIF_0_REP_ARB_MODE]
    
    ldr r0, =MEMC_AXIWIF_0_CFG_VAL
    str r0, [r9, #MEMC_AXIWIF_0_CFG]
    
    ldr r0, =MEMC_AXIWIF_0_REP_ARB_MODE_VAL
    str r0, [r9, #MEMC_AXIWIF_0_REP_ARB_MODE]
    
    ldr r0, =MEMC_CHN_TIM_DCMD_VAL
    str r0, [r9, #MEMC_CHN_TIM_DCMD]
    
    ldr r0, =MEMC_CHN_TIM_CLKS_VAL_first
    str r0, [r9, #MEMC_CHN_TIM_CLKS]
    
    mov r0, #10 // #2000
    SPIN_CYCLES
    
    
    ldr r0, =MEMC_CHN_CFG_BNK10_VAL
    str r0, [r9, #MEMC_CHN_CFG_BNK10]
    
    ldr r0, =MEMC_CHN_CFG_COL00_0_VAL
    str r0, [r9, #MEMC_CHN_CFG_COL00_0]
    
    ldr r0, =MEMC_CHN_CFG_COL00_1_VAL
    str r0, [r9, #MEMC_CHN_CFG_COL00_1]
    
    ldr r0, =MEMC_CHN_CFG_COL01_0_VAL
    str r0, [r9, #MEMC_CHN_CFG_COL01_0]
    
    ldr r0, =MEMC_CHN_CFG_COL01_1_VAL
    str r0, [r9, #MEMC_CHN_CFG_COL01_1]
    
    ldr r0, =MEMC_CHN_CFG_ROW00_0_VAL
    str r0, [r9, #MEMC_CHN_CFG_ROW00_0]
    
    ldr r0, =MEMC_CHN_CFG_ROW00_1_VAL
    str r0, [r9, #MEMC_CHN_CFG_ROW00_1]
    
    ldr r0, =MEMC_CHN_CFG_ROW01_0_VAL
    str r0, [r9, #MEMC_CHN_CFG_ROW01_0]
    
    ldr r0, =MEMC_CHN_CFG_ROW01_1_VAL
    str r0, [r9, #MEMC_CHN_CFG_ROW01_1]
    
    ldr r0, =MEMC_CHN_TIM_TIM1_0_VAL
    str r0, [r9, #MEMC_CHN_TIM_TIM1_0]
    
    ldr r0, =MEMC_CHN_TIM_TIM1_1_VAL
    str r0, [r9, #MEMC_CHN_TIM_TIM1_1]
    
    ldr r0, =MEMC_CHN_TIM_TIM2_VAL
    str r0, [r9, #MEMC_CHN_TIM_TIM2]
    
    ldr r0, =MEMC_CHN_TIM_DRAM_CFG_VAL
    str r0, [r9, #MEMC_CHN_TIM_DRAM_CFG]
    
    ldr r0, =MEMC_CHN_TIM_CLKS_VAL_second
    str r0, [r9, #MEMC_CHN_TIM_CLKS]

    // wait 2000ns
    mov r0, #10 // #2000
    SPIN_CYCLES 

    ldr r0, =MEMC_GLB_GCFG_VAL_third
    str r0, [r9, #MEMC_GLB_GCFG]
    
    // wait 2000ns
    mov r0, #10 // #2000
    SPIN_CYCLES

    SETLEDS('D','R','M','E')
#endif

    mov	pc, r8

END(ddr_init)

