/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

#ifndef BPCM_63138_H
#define BPCM_63138_H

/*check if the compiler is of C++*/
#ifdef __cplusplus
extern "C" {
#endif

    extern uint32_t readA9mpPTCounter (void);    
    extern void writeA9mpPTCounter (uint32_t val);
    extern void writeA9mpPTIntStatus (uint32_t val);
    extern uint32_t readA9mpPTControl (void);    
    extern void writeA9mpPTControl (uint32_t val);
    extern void writeA9mpPTLoad (uint32_t val);
    extern uint32_t readA9mpPTLoad (void);
    extern void readA9mpGTCounter (uint32_t * low, uint32_t * high);
    extern void writeA9mpGTCounter (uint32_t low, uint32_t high);
    extern void writeA9mpGTControl (uint32_t val);
    extern uint32_t readA9mpGTControl (void);
    extern void readA9mpGTCVal (uint32_t * low, uint32_t * high);
    extern void writeA9mpGTCVal (uint32_t low, uint32_t high);
    extern void writeA9mpGTIntStatus (uint32_t val);      
    extern uint32_t get_arm_core_clk (void); 
    extern uint32_t divA9mp (uint32_t num, uint32_t den);
    extern void resetSoC (uint32_t resetReg);
    extern void DCache_Invalidate_All (void);
#ifdef CONFIG_L2C
    extern void enable_L2C_310 (void);
    extern void sync_l2_cache (void);
    extern void SoC_63138_cacheLineCleanInvalidate (uint32_t vaddr, uint32_t paddr);
#endif
    extern void SCU_Invalidate_Enable (void);
    extern void wait_SCU_Enable(void);    
    extern void config_ARCHes (uint32_t base_address);
    extern uint32_t config_PMCR (uint32_t pmcr_val, uint32_t pmccntr_val);
    extern uint32_t read_PMCCNTR (void);
    extern void invalidate_DL1C_Line_A9 (uint32_t virt_address);
    extern void clean_DL1C_Line_A9 (uint32_t virt_address);
    extern uint32_t read_PAR (uint32_t virt_address);
    // debug helper
    extern void break_here (void);

/*check if the compiler is of C++ */
#ifdef __cplusplus
}
#endif


#endif // BPCM_63138_H
