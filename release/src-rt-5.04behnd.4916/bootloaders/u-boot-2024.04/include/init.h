/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copy the startup prototype, previously defined in common.h
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#ifndef __INIT_H_
#define __INIT_H_	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

#include <linux/types.h>

/*
 * In case of the EFI app the UEFI firmware provides the low-level
 * initialisation.
 */
#ifdef CONFIG_EFI
#define ll_boot_init()	false
#else
#include <asm/global_data.h>

#define ll_boot_init()	(!(gd->flags & GD_FLG_SKIP_LL_INIT))
#endif

/*
 * Function Prototypes
 */

/* common/board_f.c */
void board_init_f(ulong dummy);

/**
 * arch_cpu_init() - basic cpu-dependent setup for an architecture
 *
 * This is called after early malloc is available. It should handle any
 * CPU- or SoC- specific init needed to continue the init sequence. See
 * board_f.c for where it is called. If this is not provided, a default
 * version (which does nothing) will be used.
 *
 * Return: 0 on success, otherwise error
 */
int arch_cpu_init(void);

/**
 * mach_cpu_init() - SoC/machine dependent CPU setup
 *
 * This is called after arch_cpu_init(). It should handle any
 * SoC or machine specific init needed to continue the init sequence. See
 * board_f.c for where it is called. If this is not provided, a default
 * version (which does nothing) will be used.
 *
 * Return: 0 on success, otherwise error
 */
int mach_cpu_init(void);

/**
 * arch_fsp_init() - perform post-relocation firmware support package init
 *
 * Where U-Boot relies on binary blobs to handle part of the system init, this
 * function can be used to set up the blobs. This is used on some Intel
 * platforms.
 *
 * Return: 0
 */
int arch_fsp_init_r(void);

int dram_init(void);

/**
 * dram_init_banksize() - Set up DRAM bank sizes
 *
 * This can be implemented by boards to set up the DRAM bank information in
 * gd->bd->bi_dram(). It is called just before relocation, after dram_init()
 * is called.
 *
 * If this is not provided, a default implementation will try to set up a
 * single bank. It will do this if CONFIG_NR_DRAM_BANKS and
 * CFG_SYS_SDRAM_BASE are set. The bank will have a start address of
 * CFG_SYS_SDRAM_BASE and the size will be determined by a call to
 * get_effective_memsize().
 *
 * Return: 0 if OK, -ve on error
 */
int dram_init_banksize(void);

long get_ram_size(long *base, long size);
phys_size_t get_effective_memsize(void);

int testdram(void);

/**
 * arch_setup_dest_addr() - Fix up initial reloc address
 *
 * This is called in generic board init sequence in common/board_f.c at the end
 * of the setup_dest_addr() initcall. Each architecture could provide this
 * function to make adjustments to the initial reloc address.
 *
 * If an implementation is not provided, it will just be a nop stub.
 *
 * Return: 0 if OK
 */
int arch_setup_dest_addr(void);

/**
 * arch_reserve_stacks() - Reserve all necessary stacks
 *
 * This is used in generic board init sequence in common/board_f.c. Each
 * architecture could provide this function to tailor the required stacks.
 *
 * On entry gd->start_addr_sp is pointing to the suggested top of the stack.
 * The callee ensures gd->start_add_sp is 16-byte aligned, so architectures
 * require only this can leave it untouched.
 *
 * On exit gd->start_addr_sp and gd->irq_sp should be set to the respective
 * positions of the stack. The stack pointer(s) will be set to this later.
 * gd->irq_sp is only required, if the architecture needs it.
 *
 * Return: 0 if no error
 */
int arch_reserve_stacks(void);

/**
 * arch_reserve_mmu() - Reserve memory for MMU TLB table
 *
 * Architecture-specific routine for reserving memory for the MMU TLB table.
 * This is used in generic board init sequence in common/board_f.c.
 *
 * If an implementation is not provided, it will just be a nop stub.
 *
 * Return: 0 if OK
 */
int arch_reserve_mmu(void);

/**
 * arch_setup_bdinfo() - Architecture dependent boardinfo setup
 *
 * Architecture-specific routine for populating various boardinfo fields of
 * gd->bd. It is called during the generic board init sequence.
 *
 * If an implementation is not provided, it will just be a nop stub.
 *
 * Return: 0 if OK
 */
int arch_setup_bdinfo(void);

/**
 * setup_bdinfo() - Generic boardinfo setup
 *
 * Routine for populating various generic boardinfo fields of
 * gd->bd. It is called during the generic board init sequence.
 *
 * Return: 0 if OK
 */
int setup_bdinfo(void);

#if defined(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
defined(CONFIG_SAVE_PREV_BL_FDT_ADDR)
/**
 * save_prev_bl_data - Save prev bl data in env vars.
 *
 * When u-boot is chain-loaded, save previous bootloader data,
 * like initramfs address to environment variables.
 *
 * Return: 0 if ok; -ENODATA on error
 */
int save_prev_bl_data(void);
#endif

/**
 * cpu_secondary_init_r() - CPU-specific secondary initialization
 *
 * After non-volatile devices, environment and cpu code are setup, have
 * another round to deal with any initialization that might require
 * full access to the environment or loading of some image (firmware)
 * from a non-volatile device.
 *
 * It is called during the generic post-relocation init sequence.
 *
 * Return: 0 if OK
 */
int cpu_secondary_init_r(void);

/**
 * pci_ep_init() - Initialize pci endpoint devices
 *
 * It is called during the generic post-relocation init sequence.
 *
 * Return: 0 if OK
 */
int pci_ep_init(void);

/**
 * pci_init() - Enumerate pci devices
 *
 * It is called during the generic post-relocation init sequence to enumerate
 * pci buses. This is needed, for instance, in the case of DM PCI-based
 * Ethernet devices, which will not be detected without having the enumeration
 * performed earlier.
 *
 * Return: 0 if OK
 */
int pci_init(void);

/**
 * init_cache_f_r() - Turn on the cache in preparation for relocation
 *
 * Return: 0 if OK, -ve on error
 */
int init_cache_f_r(void);

#if !CONFIG_IS_ENABLED(CPU)
/**
 * print_cpuinfo() - Display information about the CPU
 *
 * Return: 0 if OK, -ve on error
 */
int print_cpuinfo(void);
#endif
int timer_init(void);

#if defined(CONFIG_DTB_RESELECT)
int embedded_dtb_select(void);
#endif

/* common/init/board_init.c */
extern ulong monitor_flash_len;

/**
 * ulong board_init_f_alloc_reserve - allocate reserved area
 * @top: top of the reserve area, growing down.
 *
 * This function is called by each architecture very early in the start-up
 * code to allow the C runtime to reserve space on the stack for writable
 * 'globals' such as GD and the malloc arena.
 *
 * Return: bottom of reserved area
 */
ulong board_init_f_alloc_reserve(ulong top);

/**
 * board_init_f_init_reserve - initialize the reserved area(s)
 * @base:	top from which reservation was done
 *
 * This function is called once the C runtime has allocated the reserved
 * area on the stack. It must initialize the GD at the base of that area.
 */
void board_init_f_init_reserve(ulong base);

struct global_data;

/**
 * arch_setup_gd() - Set up the global_data pointer
 * @gd_ptr: Pointer to global data
 *
 * This pointer is special in some architectures and cannot easily be assigned
 * to. For example on x86 it is implemented by adding a specific record to its
 * Global Descriptor Table! So we we provide a function to carry out this task.
 * For most architectures this can simply be:
 *
 *    gd = gd_ptr;
 */
void arch_setup_gd(struct global_data *gd_ptr);

/* common/board_r.c */
void board_init_r(struct global_data *id, ulong dest_addr)
	__attribute__ ((noreturn));

int cpu_init_r(void);
int mac_read_from_eeprom(void);

/**
 *  serial_read_from_eeprom - read the serial number from EEPROM
 *
 *  This function reads the serial number from the EEPROM and sets the
 *  appropriate environment variable.
 *
 *  The environment variable is only set if it has not been set
 *  already. This ensures that any user-saved variables are never
 *  overwritten.
 *
 *  This function must be called after relocation.
 */
int serial_read_from_eeprom(int devnum);
int set_cpu_clk_info(void);
int update_flash_size(int flash_size);
int arch_early_init_r(void);
int misc_init_r(void);

/* common/board_info.c */
int checkboard(void);

/**
 * show_board_info() - Show board information
 *
 * Check sysinfo for board information. Failing that if the root node of the DTB
 * has a "model" property, show it.
 *
 * Then call checkboard().
 *
 * Return 0 if OK, -ve on error
 */
int show_board_info(void);

/**
 * board_get_usable_ram_top() - get uppermost address for U-Boot relocation
 *
 * Some systems have reserved memory areas in high memory. By implementing this
 * function boards can indicate the highest address value to be used when
 * relocating U-Boot. The returned address is exclusive (i.e. 1 byte above the
 * last usable address).
 *
 * Due to overflow on systems with 32bit phys_addr_t a value 0 is used instead
 * of 4GiB.
 *
 * @total_size:	monitor length in bytes (size of U-Boot code)
 * Return:	uppermost address for U-Boot relocation
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size);

int board_early_init_f(void);

/* manipulate the U-Boot fdt before its relocation */
int board_fix_fdt(void *rw_fdt_blob);
int board_late_init(void);
int board_postclk_init(void); /* after clocks/timebase, before env/serial */
int board_early_init_r(void);

/**
 * arch_initr_trap() - Init traps
 *
 * Arch specific routine for initializing traps. It is called during the
 * generic board init sequence, after relocation.
 *
 * Return: 0 if OK
 */
int arch_initr_trap(void);

/**
 * init_addr_map()
 *
 * Initialize non-identity virtual-physical memory mappings for 32bit CPUs.
 * It is called during the generic board init sequence, after relocation.
 *
 * Return: 0 if OK
 */
int init_addr_map(void);

/**
 * main_loop() - Enter the main loop of U-Boot
 *
 * This normally runs the command line.
 */
void main_loop(void);

#if defined(CONFIG_ARM)
void relocate_code(ulong addr_moni);
#else
void relocate_code(ulong start_addr_sp, struct global_data *new_gd,
		   ulong relocaddr)
	__attribute__ ((noreturn));
#endif

/* Print a numeric value (for use in arch_print_bdinfo()) */
void bdinfo_print_num_l(const char *name, ulong value);
void bdinfo_print_num_ll(const char *name, unsigned long long value);

/* Print a string value (for use in arch_print_bdinfo()) */
void bdinfo_print_str(const char *name, const char *str);

/* Print a clock speed in MHz */
void bdinfo_print_mhz(const char *name, unsigned long hz);

/**
 * bdinfo_print_size - print size variables in bdinfo format
 * @name:	string to print before the size
 * @size:	size to print
 *
 * Helper function for displaying size variables as properly formatted bdinfo
 * entries. The size is printed as "xxx Bytes", "xxx KiB", "xxx MiB",
 * "xxx GiB", etc. as needed;
 *
 * For use in arch_print_bdinfo().
 */
void bdinfo_print_size(const char *name, uint64_t size);

/* Show arch-specific information for the 'bd' command */
void arch_print_bdinfo(void);

int do_bdinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

#endif	/* __ASSEMBLY__ */
/* Put only stuff here that the assembler can digest */

#endif	/* __INIT_H_ */
