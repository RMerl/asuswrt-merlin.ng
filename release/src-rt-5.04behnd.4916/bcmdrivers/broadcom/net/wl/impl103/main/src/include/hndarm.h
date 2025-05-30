/*
 * HND SiliconBackplane ARM core software interface.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndarm.h 831253 2023-10-12 14:01:10Z $
 */

#ifndef _hndarm_h_
#define _hndarm_h_

#include <typedefs.h>
#include <siutils.h>
#include <sbhndarm.h>

extern void *hndarm_armr;
extern uint32 hndarm_rev;

/* return pointer to arm core register space */
extern volatile void *si_arm_init(si_t *sih);

extern void enable_arm_irq(void);
extern void enable_arm_dab(void);
extern void disable_arm_irq(void);
extern void enable_arm_fiq(void);
extern void disable_arm_fiq(void);
extern void enable_nvic_ints(uint32 which);
extern void disable_nvic_ints(uint32 which);

extern uint32 get_arm_cyclecount(void);
extern void set_arm_cyclecount(uint32 ticks);

#if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__)
extern void cpu_flush_dcache_range(void *va, uint size);
extern void cpu_flush_cache_all(void);
extern void cpu_inv_dcache_range(void *va, uint size);
extern void cpu_inv_cache_all(void);
#ifdef MMU_STACK_PROT
extern void mmu_stack_prot_enable(uint fiq_stack_end, uint abrt_stack_end, uint irq_stack_end,
	uint def_stack_diff);
#endif /* MMU_STACK_PROT */
#else
#define cpu_flush_dcache_range(va, size)
#define cpu_flush_cache_all(_param)
#define cpu_inv_dcache_range(va, size)
#define cpu_inv_cache_all(_param)
#ifdef MMU_STACK_PROT
#define mmu_stack_prot_enable(fiq_stack_end, abrt_stack_end, irq_stack_end, def_stack_diff)
#endif /* MMU_STACK_PROT */
#endif /* __ARM_ARCH_7A__ || __ARM_ARCH_8A__ */

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__)
extern uint32 get_arm_perfcount_enable(void);
extern void set_arm_perfcount_enable(uint32 which);
extern uint32 set_arm_perfcount_disable(void);

extern uint32 get_arm_perfcount_sel(void);
extern void set_arm_perfcount_sel(uint32 which);

extern uint32 get_arm_perfcount_event(void);
extern void set_arm_perfcount_event(uint32 which);

extern uint32 get_arm_perfcount(void);
extern void set_arm_perfcount(uint32 which);

extern void enable_arm_cyclecount(void);
extern void disable_arm_cyclecount(void);

extern uint32 get_arm_data_fault_status(void);
extern uint32 get_arm_data_fault_address(void);

extern uint32 get_arm_instruction_fault_status(void);
extern uint32 get_arm_instruction_fault_address(void);
#endif	/* __ARM_ARCH_7R__ || __ARM_ARCH_7A__ || __ARM_ARCH_8A__ */

extern uint32 get_arm_inttimer(void);
extern void set_arm_inttimer(uint32 ticks);

extern uint32 get_arm_intmask(void);
extern void set_arm_intmask(uint32 ticks);

extern uint32 get_arm_intstatus(void);
extern void set_arm_intstatus(uint32 ticks);

extern uint32 get_arm_firqmask(void);
extern void set_arm_firqmask(uint32 ticks);

extern uint32 get_arm_firqstatus(void);
extern void set_arm_firqstatus(uint32 ticks);

extern void arm_wfi(si_t *sih);
extern void arm_jumpto(void *addr);

extern void traptest(void);

#if defined(__ARM_ARCH_7R__)
#if defined(MPU_RAM_PROTECT_ENABLED)
#ifndef MAX_MPU_REGION
#define MAX_MPU_REGION						8
#endif /* MAX_MPU_REGION */

#define UPPER_RW_RAM_CODE_MPU_REGION		1
#define LOWER_RO_ROM_CODE_MPU_REGION		2

#define LOWER_RO_RAM_CODE_MPU_REGION		5

#define UPPEREND_RW_RAM_CODE_MPU_REGION	(MAX_MPU_REGION - 1)
#define UPPEREND_RO_ROM_CODE_MPU_REGION	(UPPEREND_RW_RAM_CODE_MPU_REGION - 1)

#define	ROM_CODE_MPU_END_ASSEMBLY	0x140000
#define	RAM_CODE_MPU_END_ASSEMBLY	0x800000

#if defined(RAMBASE)
#if MEMBASE < ROM_CODE_MPU_END_ASSEMBLY
#error "RAMBASE is lower than 0x140000. Not allowed"
#endif
#if (MEMBASE + RAMSIZE)  > RAM_CODE_MPU_END_ASSEMBLY
#error "MEMBASE + RAMSIZE is greater than 0x300000. Not allowed"
#endif
#endif /* RAMBASE */

extern int cr4_mpu_set_region(uint32 region, uint32 base_address, uint32 size_index,
	uint32 control, uint32 subregion);
extern int cr4_calculate_mpu_region(uint32 start, uint32 end, uint32 *p_align_start,
	uint32 *pindex);
extern void disable_mpu_protection(bool disable);
void mpu_protect_code_area(void);
void mpu_protect_best_fit(uint32 mpu_region_start, uint32 mpu_region_end,
	uint32 start_addess, uint32 end_address);
extern void
cr4_mpu_get_assembly_region_addresses(uint32 *rom_mpu_end, uint32 *ram_mpu_end);
extern void dump_mpu_regions(void);
#endif  /* MPU_RAM_PROTECT_ENABLED */
#endif	/* __ARM_ARCH_7R__ */

extern uint32 si_arm_sflags(si_t *sih);
extern uint32 si_arm_disable_deepsleep(si_t *sih, bool disable);
extern bool si_arm_deepsleep_disabled(si_t *sih);

#ifdef BCMOVLHW
#define	BCMOVLHW_ENAB(sih)		TRUE

extern int si_arm_ovl_remap(si_t *sih, void *virt, void *phys, uint size);
extern int si_arm_ovl_reset(si_t *sih);
extern bool si_arm_ovl_vaildaddr(si_t *sih, void *virt);
extern bool si_arm_ovl_isenab(si_t *sih);
extern bool si_arm_ovl_int(si_t *sih, uint32 pc);
#else
#define	BCMOVLHW_ENAB(sih)		FALSE

#define si_arm_ovl_remap(a, b, c, d)	do {} while (0)
#define si_arm_ovl_reset(a)		do {} while (0)
#define si_arm_ovl_int(a, b)		FALSE
#endif

int32 si_arm_clockratio(si_t *sih, uint8 div);

#if defined(BCM_EXCVTBL_RELOC)
void arm_reloc_excvtbl(void);
#endif /* BCM_EXCVTBL_RELOC */

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)
void hnd_arm_inttimer_init(si_t *sih);
void hnd_arm_inttimer_set_timer(si_t *sih, uint32 ticks);
void hnd_arm_inttimer_ack_timer(si_t *sih);
#endif /* __ARM_ARCH_7R__ || __ARM_ARCH_7M__ */

#ifdef DONGLEBUILD
void __stack_chk_guard_setup(void);
void __stack_chk_fail(void);

#ifdef BCMDBG_WP
#define WP_IDX_MAX			4
#define WP_TRIGGER_RESET_CYCLE		2000000000

extern bool reset_debug_hw;
uint32 hnd_arm_watchpoint(uint wp, uint8 *m, uint32 sz);
void dump_arm_watchpoint(void);
void hnd_arm_watchpoint_reset_all(void);
void hnd_cons_arm_wp(void *arg, int argc, char *argv[]);
#endif /* BCMDBG_WP */
#endif /* DONGLEBUILD */

extern uint32 hnd_arm_c0count_ms(void);
extern uint32 hnd_arm_c0count_us(void);
extern void hnd_update_c0count(uint32 mhz);

#endif /* _hndarm_h_ */
