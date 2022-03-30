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
#if defined(SPD_opteed)
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include <arch.h>

static volatile uint32_t optee_smc_entry = 0;

static uint32_t monitor_stack_pointer [PLATFORM_MAX_CPUS_PER_CLUSTER];
static uint32_t optee_params_register [PLATFORM_MAX_CPUS_PER_CLUSTER][8];

extern void save_atf_sysreg(void);
extern void restore_atf_sysreg(void);

__attribute__((naked)) void opteed_jump(uint32_t jump, uint32_t *monitor_stack, uint32_t *params)
{
   /* Save all GP registers */
   asm("push {r0-r12, r14}");

   /* Find OPTEE's entry address and place to save ATF's monitor stack*/
   asm volatile ("mov r10, %0" "\n\t"
                 "mov r11, %1" "\n\t"
                 "mov r12, %2" "\n\t"
                 : : "r" (params), "r" (monitor_stack), "r" (jump));

   /* Save ATF's monitor's stack in cache memory */
   asm("str sp, [r11]");
   save_atf_sysreg();

   /* Load OPTEE's parameters */
   asm("ldm r10, {r0-r7}");

   /* Turn off MMU and Cache needed to initialize OPTEE */
   asm("mrc p15, 0, r8, c1, c0, 0");
   asm("bic r8, r8, #0x5"); // (SCTLR_M_BIT | SCTLR_C_BIT) = 0x5
   asm("mcr p15, 0, r8, c1, c0, 0");

   /* Save ATF's monitor's stack in non-cache memory */
   asm("str sp, [r11]");
   save_atf_sysreg();

   /* Jump to OPTEE */
   asm("cps #0x13"); // switch to secure MODE_SVC mode (0x13)
   asm("blx r12");
   asm("nop");
   /* Returned from OPTEE with OPTEE's smc entry point in r12. ** DO NOT corrupt r12 ** */

   /* Load ATF's monitor stack from non-cached memory */
   asm("ldr r9,=monitor_stack_pointer");
   /* Point to the monitor stack pointer for this CPU */
   asm("mrc p15, 0, r10, c0, c0, 5");
   asm("and r10, r10, #0xff");
   asm("lsl r10, r10, #2");
   asm("add r9, r10, r9");
   asm("ldr sp, [r9]");

   /* Restore ATF's system registers */
   restore_atf_sysreg();

   /* Turn ON MMU */
   asm("mrc p15, 0, r8, c1, c0, 0");
   asm("ldr r9,=0x5"); // SCTLR_M_BIT | SCTLR_C_BIT
   asm("orr r8, r8, r9");
   asm("mcr p15, 0, r8, c1, c0, 0");
   asm("nop");
   asm("nop");
   /* Save optee's SMC entry in cached memory */
   asm("ldr r8,=optee_smc_entry");
   asm("str r12,[r8]");
   /* Save the OPTEE's return registers r0-r7  in cached memory  */
   asm("ldr r8,=optee_params_register");
   asm("lsl r10, r10, #3");
   asm("add r8, r10, r8");
   asm("stm r8, {r0-r7}");
   /* Restore all GP registers */
   asm("pop {r0-r12, r14}");
   asm("bx lr");
}

/*******************************************************************************
 * OPTEE Dispatcher setup. The OPTEED finds out the OPTEE entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into OPTEE for its initialization.
 ******************************************************************************/
int32_t opteed_setup(void)
{
   entry_point_info_t *optee_ep_info;
   uint32_t cpu_id = plat_my_core_pos();
   uint32_t *monitor_stack = &monitor_stack_pointer[cpu_id];
   uint32_t *optee_params = &optee_params_register[cpu_id][0];
   /*
    * Get information about the Secure Payload (BL32) image. Its
    * absence is a critical failure.  TODO: Add support to
    * conditionally include the SPD service
    */

   optee_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
   if (!optee_ep_info) {
      WARN("No OPTEE provided by BL2 boot loader, Booting device"
           " without OPTEE initialization. SMC`s destined for OPTEE"
           " will return SMC_UNK\n");
      return 1;
   }

   /*
    * If there's no valid entry point for SP, we return a non-zero value
    * signalling failure initializing the service. We bail out without
    * registering any handlers
    */
   if (!optee_ep_info->pc)
     return 1;

   optee_params_register[cpu_id][2] = optee_ep_info->args.arg0;
   opteed_jump(optee_ep_info->pc, monitor_stack, optee_params);

   return 0;
}



/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range from the non-secure state as defined in the SMC Calling Convention
 * Document. It is also responsible for communicating with the Secure
 * payload to delegate work and return results back to the non-secure
 * state. Lastly it will also return any information that OPTEE needs to do
 * the work assigned to it.
 ******************************************************************************/
 uintptr_t opteed_smc_handler(uint32_t smc_fid,
                              u_register_t x1,
                              u_register_t x2,
                              u_register_t x3,
                              u_register_t x4,
                              void *cookie,
                              void *handle,
                              u_register_t flags)
{
   uint32_t cpu_id = plat_my_core_pos();
   uint32_t *monitor_stack = &monitor_stack_pointer[cpu_id];
   uint32_t *optee_params = &optee_params_register[cpu_id][0];
   cookie = cookie;
   handle = handle;
   flags = flags;

   /* Extract r0-r7 from saved context and pass to OPTEE */
   optee_params[0] = ((smc_ctx_t *)handle)->r0; // smc_fid;
   optee_params[1] = ((smc_ctx_t *)handle)->r1; // x1;
   optee_params[2] = ((smc_ctx_t *)handle)->r2; // x2;
   optee_params[3] = ((smc_ctx_t *)handle)->r3; // x3;
   optee_params[4] = ((smc_ctx_t *)handle)->r4; // x4;
   optee_params[5] = ((smc_ctx_t *)handle)->r5;
   optee_params[6] = ((smc_ctx_t *)handle)->r6;
   optee_params[7] = ((smc_ctx_t *)handle)->r7;

   assert(optee_params[0] == smc_fid);

   opteed_jump(optee_smc_entry, monitor_stack, optee_params);

   /* Return the response from OPTEE */
   SMC_RET4(handle, optee_params[0], optee_params[1], optee_params[2], optee_params[3]);
}

/* Define an OPTEED runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
               opteed_fast,

               OEN_TOS_START,
               OEN_TOS_END,
               SMC_TYPE_FAST,
               opteed_setup,
               opteed_smc_handler
               );

/* Define an OPTEED runtime service descriptor for yielding SMC calls */
DECLARE_RT_SVC(
               opteed_std,

               OEN_TOS_START,
               OEN_TOS_END,
               SMC_TYPE_YIELD,
               NULL,
               opteed_smc_handler
);
#endif
