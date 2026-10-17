/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __ASM_GENERIC_GBL_DATA_H
#define __ASM_GENERIC_GBL_DATA_H
/*
 * The following data structure is placed in some memory which is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 *
 * Keep it *SMALL* and remember to set GENERATED_GBL_DATA_SIZE > sizeof(gd_t)
 *
 * Each architecture has its own private fields. For now all are private
 */

#ifndef __ASSEMBLY__
#include <cyclic.h>
#include <event_internal.h>
#include <fdtdec.h>
#include <membuff.h>
#include <linux/list.h>
#include <linux/build_bug.h>
#include <asm-offsets.h>


struct acpi_ctx;
struct driver_rt;

typedef struct global_data gd_t;

/**
 * struct global_data - global data structure
 */
struct global_data {
	/**
	 * @bd: board information
	 */
	struct bd_info *bd;
	/**
	 * @flags: global data flags
	 *
	 * See &enum gd_flags
	 */
	unsigned long flags;
	/**
	 * @baudrate: baud rate of the serial interface
	 */
	unsigned int baudrate;
	/**
	 * @cpu_clk: CPU clock rate in Hz
	 */
	unsigned long cpu_clk;
	/**
	 * @bus_clk: platform clock rate in Hz
	 */
	unsigned long bus_clk;
	/**
	 * @pci_clk: PCI clock rate in Hz
	 */
	/* We cannot bracket this with CONFIG_PCI due to mpc5xxx */
	unsigned long pci_clk;
	/**
	 * @mem_clk: memory clock rate in Hz
	 */
	unsigned long mem_clk;
#if CONFIG_IS_ENABLED(VIDEO)
	/**
	 * @fb_base: base address of frame buffer memory
	 */
	unsigned long fb_base;
#endif
#if defined(CONFIG_POST)
	/**
	 * @post_log_word: active POST tests
	 *
	 * @post_log_word is a bit mask defining which POST tests are recorded
	 * (see constants POST_*).
	 */
	unsigned long post_log_word;
	/**
	 * @post_log_res: POST results
	 *
	 * @post_log_res is a bit mask with the POST results. A bit with value 1
	 * indicates successful execution.
	 */
	unsigned long post_log_res;
	/**
	 * @post_init_f_time: time in ms when post_init_f() started
	 */
	unsigned long post_init_f_time;
#endif
#ifdef CONFIG_BOARD_TYPES
	/**
	 * @board_type: board type
	 *
	 * If a U-Boot configuration supports multiple board types, the actual
	 * board type may be stored in this field.
	 */
	unsigned long board_type;
#endif
	/**
	 * @have_console: console is available
	 *
	 * A value of 1 indicates that serial_init() was called and a console
	 * is available.
	 * A value of 0 indicates that console input and output drivers shall
	 * not be called.
	 */
	unsigned long have_console;
#if CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	/**
	 * @precon_buf_idx: pre-console buffer index
	 *
	 * @precon_buf_idx indicates the current position of the
	 * buffer used to collect output before the console becomes
	 * available. When negative, the pre-console buffer is
	 * temporarily disabled (used when the pre-console buffer is
	 * being written out, to prevent adding its contents to
	 * itself).
	 */
	long precon_buf_idx;
#endif
	/**
	 * @env_addr: address of environment structure
	 *
	 * @env_addr contains the address of the structure holding the
	 * environment variables.
	 */
	unsigned long env_addr;
	/**
	 * @env_valid: environment is valid
	 *
	 * See &enum env_valid
	 */
	unsigned long env_valid;
	/**
	 * @env_has_init: bit mask indicating environment locations
	 *
	 * &enum env_location defines which bit relates to which location
	 */
	unsigned long env_has_init;
	/**
	 * @env_load_prio: priority of the loaded environment
	 */
	int env_load_prio;
	/**
	 * @ram_base: base address of RAM used by U-Boot
	 */
	unsigned long ram_base;
	/**
	 * @ram_top: top address of RAM used by U-Boot
	 */
	phys_addr_t ram_top;
	/**
	 * @relocaddr: start address of U-Boot in RAM
	 *
	 * After relocation this field indicates the address to which U-Boot
	 * has been relocated. It can be displayed using the bdinfo command.
	 * Its value is needed to display the source code when debugging with
	 * GDB using the 'add-symbol-file u-boot <relocaddr>' command.
	 */
	unsigned long relocaddr;
	/**
	 * @ram_size: RAM size in bytes
	 */
	phys_size_t ram_size;
	/**
	 * @mon_len: monitor length in bytes
	 */
	unsigned long mon_len;
	/**
	 * @irq_sp: IRQ stack pointer
	 */
	unsigned long irq_sp;
	/**
	 * @start_addr_sp: initial stack pointer address
	 */
	unsigned long start_addr_sp;
	/**
	 * @reloc_off: relocation offset
	 */
	unsigned long reloc_off;
	/**
	 * @new_gd: pointer to relocated global data
	 */
	struct global_data *new_gd;

#ifdef CONFIG_DM
	/**
	 * @dm_root: root instance for Driver Model
	 */
	struct udevice *dm_root;
	/**
	 * @dm_root_f: pre-relocation root instance
	 */
	struct udevice *dm_root_f;
	/**
	 * @uclass_root_s:
	 * head of core tree when uclasses are not in read-only memory.
	 *
	 * When uclasses are in read-only memory, @uclass_root_s is not used and
	 * @uclass_root points to the root node generated by dtoc.
	 */
	struct list_head uclass_root_s;
	/**
	 * @uclass_root:
	 * pointer to head of core tree, if uclasses are in read-only memory and
	 * cannot be adjusted to use @uclass_root as a list head.
	 *
	 * When not in read-only memory, @uclass_root_s is used to hold the
	 * uclass root, and @uclass_root points to the address of
	 * @uclass_root_s.
	 */
	struct list_head *uclass_root;
# if CONFIG_IS_ENABLED(OF_PLATDATA_DRIVER_RT)
	/** @dm_driver_rt: Dynamic info about the driver */
	struct driver_rt *dm_driver_rt;
# endif
#if CONFIG_IS_ENABLED(OF_PLATDATA_RT)
	/** @dm_udevice_rt: Dynamic info about the udevice */
	struct udevice_rt *dm_udevice_rt;
	/**
	 * @dm_priv_base: Base address of the priv/plat region used when
	 * udevices and uclasses are in read-only memory. This is NULL if not
	 * used
	 */
	void *dm_priv_base;
# endif
#endif
#ifdef CONFIG_TIMER
	/**
	 * @timer: timer instance for Driver Model
	 */
	struct udevice *timer;
#endif
	/**
	 * @fdt_blob: U-Boot's own device tree, NULL if none
	 */
	const void *fdt_blob;
	/**
	 * @new_fdt: relocated device tree
	 */
	void *new_fdt;
	/**
	 * @fdt_size: space reserved for relocated device space
	 */
	unsigned long fdt_size;
	/**
	 * @fdt_src: Source of FDT
	 */
	enum fdt_source_t fdt_src;
#if CONFIG_IS_ENABLED(OF_LIVE)
	/**
	 * @of_root: root node of the live tree
	 */
	struct device_node *of_root;
#endif

#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
	/**
	 * @multi_dtb_fit: pointer to uncompressed multi-dtb FIT image
	 */
	const void *multi_dtb_fit;
#endif
	/**
	 * @jt: jump table
	 *
	 * The jump table contains pointers to exported functions. A pointer to
	 * the jump table is passed to standalone applications.
	 */
	struct jt_funcs *jt;
	/**
	 * @env_buf: buffer for env_get() before reloc
	 */
	char env_buf[32];
#ifdef CONFIG_TRACE
	/**
	 * @trace_buff: trace buffer
	 *
	 * When tracing function in U-Boot this field points to the buffer
	 * recording the function calls.
	 */
	void *trace_buff;
#endif
#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	/**
	 * @cur_i2c_bus: currently used I2C bus
	 */
	int cur_i2c_bus;
#endif
	/**
	 * @timebase_h: high 32 bits of timer
	 */
	unsigned int timebase_h;
	/**
	 * @timebase_l: low 32 bits of timer
	 */
	unsigned int timebase_l;
	/**
	 * @malloc_start: start of malloc() region
	 */
#if CONFIG_IS_ENABLED(CMD_BDINFO_EXTRA)
	unsigned long malloc_start;
#endif
#if CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/**
	 * @malloc_base: base address of early malloc()
	 */
	unsigned long malloc_base;
	/**
	 * @malloc_limit: limit address of early malloc()
	 */
	unsigned long malloc_limit;
	/**
	 * @malloc_ptr: current address of early malloc()
	 */
	unsigned long malloc_ptr;
#endif
#ifdef CONFIG_PCI
	/**
	 * @hose: PCI hose for early use
	 */
	struct pci_controller *hose;
	/**
	 * @pci_ram_top: top of region accessible to PCI
	 */
	phys_addr_t pci_ram_top;
#endif
#ifdef CONFIG_PCI_BOOTDELAY
	/**
	 * @pcidelay_done: delay time before scanning of PIC hose expired
	 *
	 * If CONFIG_PCI_BOOTDELAY=y, pci_hose_scan() waits for the number of
	 * milliseconds defined by environment variable pcidelay before
	 * scanning. Once this delay has expired the flag @pcidelay_done
	 * is set to 1.
	 */
	int pcidelay_done;
#endif
	/**
	 * @cur_serial_dev: current serial device
	 */
	struct udevice *cur_serial_dev;
	/**
	 * @arch: architecture-specific data
	 */
	struct arch_global_data arch;
#ifdef CONFIG_CONSOLE_RECORD
	/**
	 * @console_out: output buffer for console recording
	 *
	 * This buffer is used to collect output during console recording.
	 */
	struct membuff console_out;
	/**
	 * @console_in: input buffer for console recording
	 *
	 * If console recording is activated, this buffer can be used to
	 * emulate input.
	 */
	struct membuff console_in;
#endif
#if CONFIG_IS_ENABLED(VIDEO)
	/**
	 * @video_top: top of video frame buffer area
	 */
	ulong video_top;
	/**
	 * @video_bottom: bottom of video frame buffer area
	 */
	ulong video_bottom;
#endif
#ifdef CONFIG_BOOTSTAGE
	/**
	 * @bootstage: boot stage information
	 */
	struct bootstage_data *bootstage;
	/**
	 * @new_bootstage: relocated boot stage information
	 */
	struct bootstage_data *new_bootstage;
#endif
#ifdef CONFIG_LOG
	/**
	 * @log_drop_count: number of dropped log messages
	 *
	 * This counter is incremented for each log message which can not
	 * be processed because logging is not yet available as signaled by
	 * flag %GD_FLG_LOG_READY in @flags.
	 */
	int log_drop_count;
	/**
	 * @default_log_level: default logging level
	 *
	 * For logging devices without filters @default_log_level defines the
	 * logging level, cf. &enum log_level_t.
	 */
	int default_log_level;
	/**
	 * @log_head: list of logging devices
	 */
	struct list_head log_head;
	/**
	 * @log_fmt: bit mask for logging format
	 *
	 * The @log_fmt bit mask selects the fields to be shown in log messages.
	 * &enum log_fmt defines the bits of the bit mask.
	 */
	int log_fmt;

	/**
	 * @processing_msg: a log message is being processed
	 *
	 * This flag is used to suppress the creation of additional messages
	 * while another message is being processed.
	 */
	bool processing_msg;
	/**
	 * @logc_prev: logging category of previous message
	 *
	 * This value is used as logging category for continuation messages.
	 */
	int logc_prev;
	/**
	 * @logl_prev: logging level of the previous message
	 *
	 * This value is used as logging level for continuation messages.
	 */
	int logl_prev;
	/**
	 * @log_cont: Previous log line did not finished wtih \n
	 *
	 * This allows for chained log messages on the same line
	 */
	bool log_cont;
#endif
#if CONFIG_IS_ENABLED(BLOBLIST)
	/**
	 * @bloblist: blob list information
	 */
	struct bloblist_hdr *bloblist;
	/**
	 * @new_bloblist: relocated blob list information
	 */
	struct bloblist_hdr *new_bloblist;
#endif
#if CONFIG_IS_ENABLED(HANDOFF)
	/**
	 * @spl_handoff: SPL hand-off information
	 */
	struct spl_handoff *spl_handoff;
#endif
#if defined(CONFIG_TRANSLATION_OFFSET)
	/**
	 * @translation_offset: optional translation offset
	 *
	 * See CONFIG_TRANSLATION_OFFSET.
	 */
	fdt_addr_t translation_offset;
#endif
#ifdef CONFIG_ACPI
	/**
	 * @acpi_ctx: ACPI context pointer
	 */
	struct acpi_ctx *acpi_ctx;
	/**
	 * @acpi_start: Start address of ACPI tables
	 */
	ulong acpi_start;
#endif
#if CONFIG_IS_ENABLED(GENERATE_SMBIOS_TABLE)
	/**
	 * @smbios_version: Points to SMBIOS type 0 version
	 */
	char *smbios_version;
#endif
#if CONFIG_IS_ENABLED(EVENT)
	/**
	 * @event_state: Points to the current state of events
	 */
	struct event_state event_state;
#endif
#ifdef CONFIG_CYCLIC
	/**
	 * @cyclic_list: list of registered cyclic functions
	 */
	struct hlist_head cyclic_list;
#endif
	/**
	 * @dmtag_list: List of DM tags
	 */
	struct list_head dmtag_list;
};

//char checker(int lobal_data);
//char coi[sizeof(struct global_data)]={checker(&coi)}; 

#ifndef DO_DEPS_ONLY
static_assert(sizeof(struct global_data) == GD_SIZE);
#endif

/**
 * gd_board_type() - retrieve board type
 *
 * Return: global board type
 */
#ifdef CONFIG_BOARD_TYPES
#define gd_board_type()		gd->board_type
#else
#define gd_board_type()		0
#endif

/* These macros help avoid #ifdefs in the code */
#if CONFIG_IS_ENABLED(OF_LIVE)
#define gd_of_root()		gd->of_root
#define gd_of_root_ptr()	&gd->of_root
#define gd_set_of_root(_root)	gd->of_root = (_root)
#else
#define gd_of_root()		NULL
#define gd_of_root_ptr()	NULL
#define gd_set_of_root(_root)
#endif

#if CONFIG_IS_ENABLED(OF_PLATDATA_DRIVER_RT)
#define gd_set_dm_driver_rt(dyn)	gd->dm_driver_rt = dyn
#define gd_dm_driver_rt()		gd->dm_driver_rt
#else
#define gd_set_dm_driver_rt(dyn)
#define gd_dm_driver_rt()		NULL
#endif

#if CONFIG_IS_ENABLED(OF_PLATDATA_RT)
#define gd_set_dm_udevice_rt(dyn)	gd->dm_udevice_rt = dyn
#define gd_dm_udevice_rt()		gd->dm_udevice_rt
#define gd_set_dm_priv_base(dyn)	gd->dm_priv_base = dyn
#define gd_dm_priv_base()		gd->dm_priv_base
#else
#define gd_set_dm_udevice_rt(dyn)
#define gd_dm_udevice_rt()		NULL
#define gd_set_dm_priv_base(dyn)
#define gd_dm_priv_base()		NULL
#endif

#ifdef CONFIG_ACPI
#define gd_acpi_ctx()		gd->acpi_ctx
#define gd_acpi_start()		gd->acpi_start
#define gd_set_acpi_start(addr)	gd->acpi_start = addr
#else
#define gd_acpi_ctx()		NULL
#define gd_acpi_start()		0UL
#define gd_set_acpi_start(addr)
#endif

#ifdef CONFIG_SMBIOS
#define gd_smbios_start()	gd->arch.smbios_start
#define gd_set_smbios_start(addr)	gd->arch.smbios_start = addr
#else
#define gd_smbios_start()	0UL
#define gd_set_smbios_start(addr)
#endif

#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
#define gd_multi_dtb_fit()	gd->multi_dtb_fit
#define gd_set_multi_dtb_fit(_dtb)	gd->multi_dtb_fit = _dtb
#else
#define gd_multi_dtb_fit()	NULL
#define gd_set_multi_dtb_fit(_dtb)
#endif

#if CONFIG_IS_ENABLED(EVENT_DYNAMIC)
#define gd_event_state()	((struct event_state *)&gd->event_state)
#else
#define gd_event_state()	NULL
#endif

#if CONFIG_IS_ENABLED(CMD_BDINFO_EXTRA)
#define gd_malloc_start()		gd->malloc_start
#define gd_set_malloc_start(_val)	gd->malloc_start = (_val)
#else
#define gd_malloc_start()	0
#define gd_set_malloc_start(val)
#endif

#if CONFIG_IS_ENABLED(PCI)
#define gd_set_pci_ram_top(val)	gd->pci_ram_top = val
#else
#define gd_set_pci_ram_top(val)
#endif

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
#define gd_malloc_ptr()		gd->malloc_ptr
#else
#define gd_malloc_ptr()		0L
#endif

/**
 * enum gd_flags - global data flags
 *
 * See field flags of &struct global_data.
 */
enum gd_flags {
	/**
	 * @GD_FLG_RELOC: code was relocated to RAM
	 */
	GD_FLG_RELOC = 0x00001,
	/**
	 * @GD_FLG_DEVINIT: devices have been initialized
	 */
	GD_FLG_DEVINIT = 0x00002,
	/**
	 * @GD_FLG_SILENT: silent mode
	 */
	GD_FLG_SILENT = 0x00004,
	/**
	 * @GD_FLG_POSTFAIL: critical POST test failed
	 */
	GD_FLG_POSTFAIL = 0x00008,
	/**
	 * @GD_FLG_POSTSTOP: POST sequence aborted
	 */
	GD_FLG_POSTSTOP = 0x00010,
	/**
	 * @GD_FLG_LOGINIT: log Buffer has been initialized
	 */
	GD_FLG_LOGINIT = 0x00020,
	/**
	 * @GD_FLG_DISABLE_CONSOLE: disable console (in & out)
	 */
	GD_FLG_DISABLE_CONSOLE = 0x00040,
	/**
	 * @GD_FLG_ENV_READY: environment imported into hash table
	 */
	GD_FLG_ENV_READY = 0x00080,
	/**
	 * @GD_FLG_SERIAL_READY: pre-relocation serial console ready
	 */
	GD_FLG_SERIAL_READY = 0x00100,
	/**
	 * @GD_FLG_FULL_MALLOC_INIT: full malloc() is ready
	 */
	GD_FLG_FULL_MALLOC_INIT = 0x00200,
	/**
	 * @GD_FLG_SPL_INIT: spl_init() has been called
	 */
	GD_FLG_SPL_INIT = 0x00400,
	/**
	 * @GD_FLG_SKIP_RELOC: don't relocate
	 */
	GD_FLG_SKIP_RELOC = 0x00800,
	/**
	 * @GD_FLG_RECORD: record console
	 */
	GD_FLG_RECORD = 0x01000,
	/**
	 * @GD_FLG_RECORD_OVF: record console overflow
	 */
	GD_FLG_RECORD_OVF = 0x02000,
	/**
	 * @GD_FLG_ENV_DEFAULT: default variable flag
	 */
	GD_FLG_ENV_DEFAULT = 0x04000,
	/**
	 * @GD_FLG_SPL_EARLY_INIT: early SPL initialization is done
	 */
	GD_FLG_SPL_EARLY_INIT = 0x08000,
	/**
	 * @GD_FLG_LOG_READY: log system is ready for use
	 */
	GD_FLG_LOG_READY = 0x10000,
	/**
	 * @GD_FLG_CYCLIC_RUNNING: cyclic_run is in progress
	 */
	GD_FLG_CYCLIC_RUNNING = 0x20000,
	/**
	 * @GD_FLG_SKIP_LL_INIT: don't perform low-level initialization
	 */
	GD_FLG_SKIP_LL_INIT = 0x40000,
	/**
	 * @GD_FLG_SMP_READY: SMP initialization is complete
	 */
	GD_FLG_SMP_READY = 0x80000,
	/**
	 * @GD_FLG_FDT_CHANGED: Device tree change has been detected by tests
	 */
	GD_FLG_FDT_CHANGED = 0x100000,
	/**
	 * @GD_FLG_OF_TAG_MIGRATE: Device tree has old u-boot,dm- tags
	 */
	GD_FLG_OF_TAG_MIGRATE = 0x200000,
	/**
	 * @GD_FLG_DM_DEAD: Driver model is not accessible. This can be set when
	 * the memory used to holds its tables has been mapped out.
	 */
	GD_FLG_DM_DEAD = 0x400000,
	/**
	 * @GD_FLG_BLOBLIST_READY: bloblist is ready for use
	 */
	GD_FLG_BLOBLIST_READY = 0x800000,
	/**
	 * @GD_FLG_HUSH_OLD_PARSER: Use hush old parser.
	 */
	GD_FLG_HUSH_OLD_PARSER = 0x1000000,
	/**
	 * @GD_FLG_HUSH_MODERN_PARSER: Use hush 2021 parser.
	 */
	GD_FLG_HUSH_MODERN_PARSER = 0x2000000,
};

#endif /* __ASSEMBLY__ */

#endif /* __ASM_GENERIC_GBL_DATA_H */
