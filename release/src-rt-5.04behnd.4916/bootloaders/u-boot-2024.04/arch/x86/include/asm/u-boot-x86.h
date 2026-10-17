/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 */

#ifndef _U_BOOT_I386_H_
#define _U_BOOT_I386_H_	1

struct global_data;

extern char gdt_rom[];

/* cpu/.../cpu.c */
int arch_cpu_init(void);

/**
 * x86_cpu_init_f() - Set up basic features of the x86 CPU
 *
 * 0 on success, -ve on error
 */
int x86_cpu_init_f(void);

/**
 * x86_cpu_reinit_f() - Set up the CPU a second time
 *
 * Once cpu_init_f() has been called (e.g. in SPL) we should not call it
 * again (e.g. in U-Boot proper) since it sets up the state from scratch.
 * Call this function in later phases of U-Boot instead. It reads the CPU
 * identify so that CPU functions can be used correctly, but does not change
 * anything.
 *
 * Return: 0 (indicating success, to mimic cpu_init_f())
 */
int x86_cpu_reinit_f(void);

/**
 * x86_cpu_init_tpl() - Do the minimum possible CPU init
 *
 * This just sets up the CPU features and figured out the identity
 *
 * Return: 0 (indicating success, to mimic cpu_init_f())
 */
int x86_cpu_init_tpl(void);

/**
 * cpu_reinit_fpu() - Reinit the FPU if something is wrong with it
 *
 * The FSP-M code can leave registers in use in the FPU. This functions reinits
 * it so that the FPU can be used safely
 */
void cpu_reinit_fpu(void);

int cpu_init_f(void);
void setup_gdt(struct global_data *id, u64 *gdt_addr);
/*
 * Setup FSP execution environment GDT to use the one we used in
 * arch/x86/cpu/start16.S and reload the segment registers.
 */
void setup_fsp_gdt(void);
int init_cache(void);
int cleanup_before_linux(void);

/* cpu/.../timer.c */
void timer_isr(void *);
typedef void (timer_fnc_t) (void);
int register_timer_isr (timer_fnc_t *isr_func);
unsigned long get_tbclk_mhz(void);
void timer_set_base(uint64_t base);
int i8254_init(void);

/* cpu/.../interrupts.c */
int cpu_init_interrupts(void);

int cleanup_before_linux(void);
int x86_cleanup_before_linux(void);
void x86_enable_caches(void);
void x86_disable_caches(void);
int x86_init_cache(void);
phys_addr_t board_get_usable_ram_top(phys_size_t total_size);
int default_print_cpuinfo(void);

/* Set up a UART which can be used with printch(), printhex8(), etc. */
int setup_internal_uart(int enable);

void isa_unmap_rom(u32 addr);
u32 isa_map_rom(u32 bus_addr, int size);

/* arch/x86/lib/... */
int video_bios_init(void);

/* arch/x86/lib/fsp1,2/... */

/**
 * fsp_save_s3_stack() - save stack address to CMOS for next S3 boot
 *
 * At the end of pre-relocation phase, save the new stack address
 * to CMOS and use it as the stack on next S3 boot for fsp_init()
 * continuation function.
 *
 * @return:	0 if OK, -ve on error
 */
int fsp_save_s3_stack(void);

/**
 * board_init_f_r_trampoline() - jump to relocated address with new stack
 *
 * @sp: New stack pointer to use
 */
void __noreturn board_init_f_r_trampoline(ulong sp);

/**
 * board_init_f_r() - jump to relocated U-Boot
 *
 * This is used to jump from pre-relocation to post-relocation U-Boot. It
 * enables the cache and jump to the new location.
 */
void __noreturn board_init_f_r(void);

/*
 * board_init_f_r_trampoline64() - jump to relocated address with new stack
 *
 * This is the 64-bit version
 *
 * @new_gd: New global_data pointer to use
 * @sp: New stack pointer to pass on to board_init_r()
 */
void __noreturn board_init_f_r_trampoline64(struct global_data *new_gd,
					    ulong sp);

int arch_misc_init(void);

/* Read the time stamp counter */
static inline notrace uint64_t rdtsc(void)
{
	uint32_t high, low;
	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	return (((uint64_t)high) << 32) | low;
}

/* board/... */
void timer_set_tsc_base(uint64_t new_base);
uint64_t timer_get_tsc(void);

void quick_ram_check(void);

#define PCI_VGA_RAM_IMAGE_START		0xc0000

#endif	/* _U_BOOT_I386_H_ */
