/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char	vu_char;

#include <config.h>
#include <errno.h>
#include <time.h>
#include <asm-offsets.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/stringify.h>
#include <asm/ptrace.h>
#include <stdarg.h>
#include <stdio.h>
#include <linux/kernel.h>

#include <part.h>
#include <flash.h>
#include <image.h>

#ifdef __LP64__
#define CONFIG_SYS_SUPPORT_64BIT_DATA
#endif

#include <log.h>

typedef void (interrupt_handler_t)(void *);

#include <asm/u-boot.h> /* boot information for Linux kernel */
#include <asm/global_data.h>	/* global data used for startup functions */

#if defined(CONFIG_ENV_IS_EMBEDDED)
#define TOTAL_MALLOC_LEN	CONFIG_SYS_MALLOC_LEN
#elif ( ((CONFIG_ENV_ADDR+CONFIG_ENV_SIZE) < CONFIG_SYS_MONITOR_BASE) || \
	(CONFIG_ENV_ADDR >= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)) ) || \
      defined(CONFIG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CONFIG_SYS_MALLOC_LEN + CONFIG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CONFIG_SYS_MALLOC_LEN
#endif

/* startup functions, used in:
 * common/board_f.c
 * common/init/board_init.c
 * common/board_r.c
 * common/board_info.c
 */
#include <init.h>

/*
 * Function Prototypes
 */
void	hang		(void) __attribute__ ((noreturn));

int	cpu_init(void);

#include <display_options.h>

/* common/main.c */
void	main_loop	(void);
int run_command(const char *cmd, int flag);
int run_command_repeatable(const char *cmd, int flag);

/**
 * Run a list of commands separated by ; or even \0
 *
 * Note that if 'len' is not -1, then the command does not need to be nul
 * terminated, Memory will be allocated for the command in that case.
 *
 * @param cmd	List of commands to run, each separated bu semicolon
 * @param len	Length of commands excluding terminator if known (-1 if not)
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 on success, or != 0 on error.
 */
int run_command_list(const char *cmd, int len, int flag);

int checkflash(void);
int checkdram(void);
extern u8 __dtb_dt_begin[];	/* embedded device tree blob */
extern u8 __dtb_dt_spl_begin[];	/* embedded device tree blob for SPL/TPL */
int mdm_init(void);

/**
 * Show the DRAM size in a board-specific way
 *
 * This is used by boards to display DRAM information in their own way.
 *
 * @param size	Size of DRAM (which should be displayed along with other info)
 */
void board_show_dram(phys_size_t size);

/**
 * Get the uppermost pointer that is valid to access
 *
 * Some systems may not map all of their address space. This function allows
 * boards to indicate what their highest support pointer value is for DRAM
 * access.
 *
 * @param total_size	Size of U-Boot (unused?)
 */
ulong board_get_usable_ram_top(ulong total_size);

/**
 * arch_fixup_fdt() - Write arch-specific information to fdt
 *
 * Defined in arch/$(ARCH)/lib/bootm-fdt.c
 *
 * @blob:	FDT blob to write to
 * @return 0 if ok, or -ve FDT_ERR_... on failure
 */
int arch_fixup_fdt(void *blob);

/* common/flash.c */
void flash_perror (int);

/* common/cmd_source.c */
int	source (ulong addr, const char *fit_uname);

extern ulong load_addr;		/* Default Load Address */
extern ulong save_addr;		/* Default Save Address */
extern ulong save_size;		/* Default Save Size */

/* common/cmd_net.c */
int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/* common/cmd_fat.c */
int do_fat_fsload(cmd_tbl_t *, int, int, char * const []);

/* common/cmd_ext2.c */
int do_ext2load(cmd_tbl_t *, int, int, char * const []);

/* common/cmd_nvedit.c */
int	env_init     (void);
void	env_relocate (void);
int	envmatch     (uchar *, int);

/**
 * env_get() - Look up the value of an environment variable
 *
 * In U-Boot proper this can be called before relocation (which is when the
 * environment is loaded from storage, i.e. GD_FLG_ENV_READY is 0). In that
 * case this function calls env_get_f().
 *
 * @varname:	Variable to look up
 * @return value of variable, or NULL if not found
 */
char *env_get(const char *varname);

/**
 * env_get_f() - Look up the value of an environment variable (early)
 *
 * This function is called from env_get() if the environment has not been
 * loaded yet (GD_FLG_ENV_READY flag is 0). Some environment locations will
 * support reading the value (slowly) and some will not.
 *
 * @varname:	Variable to look up
 * @return value of variable, or NULL if not found
 */
int env_get_f(const char *name, char *buf, unsigned len);

/**
 * env_get_ulong() - Return an environment variable as an integer value
 *
 * Most U-Boot environment variables store hex values. For those which store
 * (e.g.) base-10 integers, this function can be used to read the value.
 *
 * @name:	Variable to look up
 * @base:	Base to use (e.g. 10 for base 10, 2 for binary)
 * @default_val: Default value to return if no value is found
 * @return the value found, or @default_val if none
 */
ulong env_get_ulong(const char *name, int base, ulong default_val);

/**
 * env_get_hex() - Return an environment variable as a hex value
 *
 * Decode an environment as a hex number (it may or may not have a 0x
 * prefix). If the environment variable cannot be found, or does not start
 * with hex digits, the default value is returned.
 *
 * @varname:		Variable to decode
 * @default_val:	Value to return on error
 */
ulong env_get_hex(const char *varname, ulong default_val);

/*
 * Read an environment variable as a boolean
 * Return -1 if variable does not exist (default to true)
 */
int env_get_yesno(const char *var);

/**
 * env_set() - set an environment variable
 *
 * This sets or deletes the value of an environment variable. For setting the
 * value the variable is created if it does not already exist.
 *
 * @varname: Variable to adjust
 * @value: Value to set for the variable, or NULL or "" to delete the variable
 * @return 0 if OK, 1 on error
 */
int env_set(const char *varname, const char *value);

/**
 * env_set_ulong() - set an environment variable to an integer
 *
 * @varname: Variable to adjust
 * @value: Value to set for the variable (will be converted to a string)
 * @return 0 if OK, 1 on error
 */
int env_set_ulong(const char *varname, ulong value);

/**
 * env_set_hex() - set an environment variable to a hex value
 *
 * @varname: Variable to adjust
 * @value: Value to set for the variable (will be converted to a hex string)
 * @return 0 if OK, 1 on error
 */
int env_set_hex(const char *varname, ulong value);

/**
 * env_set_addr - Set an environment variable to an address in hex
 *
 * @varname:	Environment variable to set
 * @addr:	Value to set it to
 * @return 0 if ok, 1 on error
 */
static inline int env_set_addr(const char *varname, const void *addr)
{
	return env_set_hex(varname, (ulong)addr);
}

#ifdef CONFIG_AUTO_COMPLETE
int env_complete(char *var, int maxv, char *cmdv[], int maxsz, char *buf,
		 bool dollar_comp);
#endif
int get_env_id (void);

void	pci_init_board(void);

/* common/exports.c */
void	jumptable_init(void);

/* common/kallsysm.c */
const char *symbol_lookup(unsigned long addr, unsigned long *caddr);

/* common/memsize.c */
long	get_ram_size  (long *, long);
phys_size_t get_effective_memsize(void);

/* $(BOARD)/$(BOARD).c */
void	reset_phy     (void);
void	fdc_hw_init   (void);

/* $(BOARD)/eeprom.c */
#ifdef CONFIG_CMD_EEPROM
void eeprom_init  (int bus);
int  eeprom_read  (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
int  eeprom_write (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
#else
/*
 * Some EEPROM code is depecated because it used the legacy I2C interface. Add
 * some macros here so we don't have to touch every one of those uses
 */
#define eeprom_init(bus)
#define eeprom_read(dev_addr, offset, buffer, cnt) ((void)-ENOSYS)
#define eeprom_write(dev_addr, offset, buffer, cnt) ((void)-ENOSYS)
#endif

#if !defined(CONFIG_ENV_EEPROM_IS_ON_I2C) && defined(CONFIG_SYS_I2C_EEPROM_ADDR)
# define CONFIG_SYS_DEF_EEPROM_ADDR CONFIG_SYS_I2C_EEPROM_ADDR
#endif

/* $(BOARD)/$(BOARD).c */
int board_early_init_f (void);
int board_fix_fdt (void *rw_fdt_blob); /* manipulate the U-Boot fdt before its relocation */
int board_late_init (void);
int board_postclk_init (void); /* after clocks/timebase, before env/serial */
int board_early_init_r (void);

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram(void);
#endif /* CONFIG_SYS_DRAM_TEST */

/* $(CPU)/start.S */
int	icache_status (void);
void	icache_enable (void);
void	icache_disable(void);
int	dcache_status (void);
void	dcache_enable (void);
void	dcache_disable(void);
void	mmu_disable(void);
#if defined(CONFIG_ARM)
void	relocate_code(ulong);
#else
void	relocate_code(ulong, gd_t *, ulong) __attribute__ ((noreturn));
#endif
ulong	get_endaddr   (void);
void	trap_init     (ulong);

/* $(CPU)/cpu.c */
static inline int cpumask_next(int cpu, unsigned int mask)
{
	for (cpu++; !((1 << cpu) & mask); cpu++)
		;

	return cpu;
}

#define for_each_cpu(iter, cpu, num_cpus, mask) \
	for (iter = 0, cpu = cpumask_next(-1, mask); \
		iter < num_cpus; \
		iter++, cpu = cpumask_next(cpu, mask)) \

int	cpu_numcores  (void);
int	cpu_num_dspcores(void);
u32	cpu_mask      (void);
u32	cpu_dsp_mask(void);
int	is_core_valid (unsigned int);

void s_init(void);

int	checkcpu      (void);
int	checkicache   (void);
int	checkdcache   (void);
void	upmconfig     (unsigned int, unsigned int *, unsigned int);
ulong	get_tbclk     (void);
void	reset_misc    (void);
void	reset_cpu     (ulong addr);
void ft_cpu_setup(void *blob, bd_t *bd);
void ft_pci_setup(void *blob, bd_t *bd);

void smp_set_core_boot_addr(unsigned long addr, int corenr);
void smp_kick_all_cpus(void);

/* $(CPU)/serial.c */
int	serial_init   (void);
void	serial_setbrg (void);
void	serial_putc   (const char);
void	serial_putc_raw(const char);
void	serial_puts   (const char *);
int	serial_getc   (void);
int	serial_tstc   (void);

/* $(CPU)/speed.c */
int	get_clocks (void);
ulong	get_bus_freq  (ulong);
int get_serial_clock(void);

/* $(CPU)/interrupts.c */
int	interrupt_init	   (void);
void	timer_interrupt	   (struct pt_regs *);
void	external_interrupt (struct pt_regs *);
void	irq_install_handler(int, interrupt_handler_t *, void *);
void	irq_free_handler   (int);
void	reset_timer	   (void);

/* Return value of monotonic microsecond timer */
unsigned long timer_get_us(void);

void	enable_interrupts  (void);
int	disable_interrupts (void);

/* $(CPU)/.../commproc.c */
int	dpram_init (void);
uint	dpram_base(void);
uint	dpram_base_align(uint align);
uint	dpram_alloc(uint size);
uint	dpram_alloc_align(uint size,uint align);
void	bootcount_store (ulong);
ulong	bootcount_load (void);

/* $(CPU)/.../<eth> */
void mii_init (void);

/* $(CPU)/.../lcd.c */
ulong	lcd_setmem (ulong);

/* $(CPU)/.../video.c */
ulong	video_setmem (ulong);

/* arch/$(ARCH)/lib/cache.c */
void	enable_caches(void);
void	flush_cache   (unsigned long, unsigned long);
void	flush_dcache_all(void);
void	flush_dcache_range(unsigned long start, unsigned long stop);
void	invalidate_dcache_range(unsigned long start, unsigned long stop);
void	invalidate_dcache_all(void);
void	invalidate_icache_all(void);

enum {
	/* Disable caches (else flush caches but leave them active) */
	CBL_DISABLE_CACHES		= 1 << 0,
	CBL_SHOW_BOOTSTAGE_REPORT	= 1 << 1,

	CBL_ALL				= 3,
};

/**
 * Clean up ready for linux
 *
 * @param flags		Flags to control what is done
 */
int cleanup_before_linux_select(int flags);

/* arch/$(ARCH)/lib/ticks.S */
uint64_t get_ticks(void);
void	wait_ticks    (unsigned long);

/* arch/$(ARCH)/lib/time.c */
ulong	usec2ticks    (unsigned long usec);
ulong	ticks2usec    (unsigned long ticks);

/* lib/gunzip.c */
int gzip_parse_header(const unsigned char *src, unsigned long len);
int gunzip(void *, int, unsigned char *, unsigned long *);
int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset);

/**
 * gzwrite progress indicators: defined weak to allow board-specific
 * overrides:
 *
 *	gzwrite_progress_init called on startup
 *	gzwrite_progress called during decompress/write loop
 *	gzwrite_progress_finish called at end of loop to
 *		indicate success (retcode=0) or failure
 */
void gzwrite_progress_init(u64 expected_size);

void gzwrite_progress(int iteration,
		     u64 bytes_written,
		     u64 total_bytes);

void gzwrite_progress_finish(int retcode,
			     u64 totalwritten,
			     u64 totalsize,
			     u32 expected_crc,
			     u32 calculated_crc);

/**
 * decompress and write gzipped image from memory to block device
 *
 * @param	src		compressed image address
 * @param	len		compressed image length in bytes
 * @param	dev		block device descriptor
 * @param	szwritebuf	bytes per write (pad to erase size)
 * @param	startoffs	offset in bytes of first write
 * @param	szexpected	expected uncompressed length
 *				may be zero to use gzip trailer
 *				for files under 4GiB
 */
int gzwrite(unsigned char *src, int len,
	    struct blk_desc *dev,
	    unsigned long szwritebuf,
	    u64 startoffs,
	    u64 szexpected);

/* lib/lz4_wrapper.c */
int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn);

/* lib/qsort.c */
void qsort(void *base, size_t nmemb, size_t size,
	   int(*compar)(const void *, const void *));
int strcmp_compar(const void *, const void *);

/* lib/uuid.c */
#include <uuid.h>

/* lib/vsprintf.c */
#include <vsprintf.h>

/* lib/strmhz.c */
char *	strmhz(char *buf, unsigned long hz);

/* lib/crc32.c */
#include <u-boot/crc.h>

/* lib/rand.c */
#define RAND_MAX -1U
void srand(unsigned int seed);
unsigned int rand(void);
unsigned int rand_r(unsigned int *seedp);

/*
 * STDIO based functions (can always be used)
 */
/* serial stuff */
int	serial_printf (const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2)));

/* lib/gzip.c */
int gzip(void *dst, unsigned long *lenp,
		unsigned char *src, unsigned long srclen);
int zzip(void *dst, unsigned long *lenp, unsigned char *src,
		unsigned long srclen, int stoponerr,
		int (*func)(unsigned long, unsigned long));

/* lib/net_utils.c */
#include <net.h>
static inline struct in_addr env_get_ip(char *var)
{
	return string_to_ip(env_get(var));
}

int	pcmcia_init (void);

#ifdef CONFIG_LED_STATUS
# include <status_led.h>
#endif

#include <bootstage.h>

#ifdef CONFIG_SHOW_ACTIVITY
void show_activity(int arg);
#endif

/* Multicore arch functions */
#ifdef CONFIG_MP
int cpu_status(u32 nr);
int cpu_reset(u32 nr);
int cpu_disable(u32 nr);
int cpu_release(u32 nr, int argc, char * const argv[]);
#endif

#else	/* __ASSEMBLY__ */

#endif	/* __ASSEMBLY__ */

/* Put only stuff here that the assembler can digest */

#ifdef CONFIG_POST
#define CONFIG_HAS_POST
#ifndef CONFIG_POST_ALT_LIST
#define CONFIG_POST_STD_LIST
#endif
#endif

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * check_member() - Check the offset of a structure member
 *
 * @structure:	Name of structure (e.g. global_data)
 * @member:	Name of member (e.g. baudrate)
 * @offset:	Expected offset in bytes
 */
#define check_member(structure, member, offset) _Static_assert( \
	offsetof(struct structure, member) == offset, \
	"`struct " #structure "` offset for `" #member "` is not " #offset)

/* Avoid using CONFIG_EFI_STUB directly as we may boot from other loaders */
#ifdef CONFIG_EFI_STUB
#define ll_boot_init()	false
#else
#define ll_boot_init()	true
#endif

/* Pull in stuff for the build system */
#ifdef DO_DEPS_ONLY
# include <environment.h>
#endif

#endif	/* __COMMON_H_ */
