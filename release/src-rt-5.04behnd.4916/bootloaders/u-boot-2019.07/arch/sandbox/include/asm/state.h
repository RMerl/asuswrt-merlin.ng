/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 */

#ifndef __SANDBOX_STATE_H
#define __SANDBOX_STATE_H

#include <config.h>
#include <sysreset.h>
#include <stdbool.h>
#include <linux/list.h>
#include <linux/stringify.h>

/**
 * Selects the behavior of the serial terminal.
 *
 * If Ctrl-C is processed by U-Boot, then the only way to quit sandbox is with
 * the 'reset' command, or equivalent.
 *
 * If the terminal is cooked, then Ctrl-C will terminate U-Boot, and the
 * command line will not be quite such a faithful emulation.
 *
 * Options are:
 *
 *	raw-with-sigs		- Raw, but allow signals (Ctrl-C will quit)
 *	raw			- Terminal is always raw
 *	cooked			- Terminal is always cooked
 */
enum state_terminal_raw {
	STATE_TERM_RAW_WITH_SIGS,	/* Default */
	STATE_TERM_RAW,
	STATE_TERM_COOKED,

	STATE_TERM_COUNT,
};

struct sandbox_spi_info {
	struct udevice *emul;
};

struct sandbox_wdt_info {
	unsigned long long counter;
	uint reset_count;
	bool running;
};

/**
 * struct sandbox_mapmem_entry - maps pointers to/from U-Boot addresses
 *
 * When map_to_sysmem() is called with an address outside sandbox's emulated
 * RAM, a record is created with a tag that can be used to reference that
 * pointer. When map_sysmem() is called later with that tag, the pointer will
 * be returned, just as it would for a normal sandbox address.
 *
 * @tag: Address tag (a value which U-Boot uses to refer to the address)
 * @ptr: Associated pointer for that tag
 */
struct sandbox_mapmem_entry {
	ulong tag;
	void *ptr;
	struct list_head sibling_node;
};

/* The complete state of the test system */
struct sandbox_state {
	const char *cmd;		/* Command to execute */
	bool interactive;		/* Enable cmdline after execute */
	bool run_distro_boot;		/* Automatically run distro bootcommands */
	const char *fdt_fname;		/* Filename of FDT binary */
	const char *parse_err;		/* Error to report from parsing */
	int argc;			/* Program arguments */
	char **argv;			/* Command line arguments */
	const char *jumped_fname;	/* Jumped from previous U_Boot */
	uint8_t *ram_buf;		/* Emulated RAM buffer */
	unsigned int ram_size;		/* Size of RAM buffer */
	const char *ram_buf_fname;	/* Filename to use for RAM buffer */
	bool ram_buf_rm;		/* Remove RAM buffer file after read */
	bool write_ram_buf;		/* Write RAM buffer on exit */
	const char *state_fname;	/* File containing sandbox state */
	void *state_fdt;		/* Holds saved state for sandbox */
	bool read_state;		/* Read sandbox state on startup */
	bool write_state;		/* Write sandbox state on exit */
	bool ignore_missing_state_on_read;	/* No error if state missing */
	bool show_lcd;			/* Show LCD on start-up */
	enum sysreset_t last_sysreset;	/* Last system reset type */
	bool sysreset_allowed[SYSRESET_COUNT];	/* Allowed system reset types */
	enum state_terminal_raw term_raw;	/* Terminal raw/cooked */
	bool skip_delays;		/* Ignore any time delays (for test) */
	bool show_test_output;		/* Don't suppress stdout in tests */
	int default_log_level;		/* Default log level for sandbox */
	bool show_of_platdata;		/* Show of-platdata in SPL */
	bool ram_buf_read;		/* true if we read the RAM buffer */

	/* Pointer to information for each SPI bus/cs */
	struct sandbox_spi_info spi[CONFIG_SANDBOX_SPI_MAX_BUS]
					[CONFIG_SANDBOX_SPI_MAX_CS];

	/* Information about Watchdog */
	struct sandbox_wdt_info wdt;

	ulong next_tag;			/* Next address tag to allocate */
	struct list_head mapmem_head;	/* struct sandbox_mapmem_entry */
	bool hwspinlock;		/* Hardware Spinlock status */

	/*
	 * This struct is getting large.
	 *
	 * Consider putting test data in driver-private structs, like
	 * sandbox_pch.c.
	 *
	 * If you add new members, please put them above this comment.
	 */
};

/* Minimum space we guarantee in the state FDT when calling read/write*/
#define SANDBOX_STATE_MIN_SPACE		0x1000

/**
 * struct sandbox_state_io - methods to saved/restore sandbox state
 * @name: Name of of the device tree node, also the name of the variable
 *	holding this data so it should be an identifier (use underscore
 *	instead of minus)
 * @compat: Compatible string for the node containing this state
 *
 * @read: Function to read state from FDT
 *	If data is available, then blob and node will provide access to it. If
 *	not (blob == NULL and node == -1) this function should set up an empty
 *	data set for start-of-day.
 *	@param blob: Pointer to device tree blob, or NULL if no data to read
 *	@param node: Node offset to read from
 *	@return 0 if OK, -ve on error
 *
 * @write: Function to write state to FDT
 *	The caller will ensure that there is a node ready for the state. The
 *	node may already contain the old state, in which case it should be
 *	overridden. There is guaranteed to be SANDBOX_STATE_MIN_SPACE bytes
 *	of free space, so error checking is not required for fdt_setprop...()
 *	calls which add up to less than this much space.
 *
 *	For adding larger properties, use state_setprop().
 *
 * @param blob: Device tree blob holding state
 * @param node: Node to write our state into
 *
 * Note that it is possible to save data as large blobs or as individual
 * hierarchical properties. However, unless you intend to keep state files
 * around for a long time and be able to run an old state file on a new
 * sandbox, it might not be worth using individual properties for everything.
 * This is certainly supported, it is just a matter of the effort you wish
 * to put into the state read/write feature.
 */
struct sandbox_state_io {
	const char *name;
	const char *compat;
	int (*write)(void *blob, int node);
	int (*read)(const void *blob, int node);
};

/**
 * SANDBOX_STATE_IO - Declare sandbox state to read/write
 *
 * Sandbox permits saving state from one run and restoring it in another. This
 * allows the test system to retain state between runs and thus better
 * emulate a real system. Examples of state that might be useful to save are
 * the emulated GPIOs pin settings, flash memory contents and TPM private
 * data. U-Boot memory contents is dealth with separately since it is large
 * and it is not normally useful to save it (since a normal system does not
 * preserve DRAM between runs). See the '-m' option for this.
 *
 * See struct sandbox_state_io above for member documentation.
 */
#define SANDBOX_STATE_IO(_name, _compat, _read, _write) \
	ll_entry_declare(struct sandbox_state_io, _name, state_io) = { \
		.name = __stringify(_name), \
		.read = _read, \
		.write = _write, \
		.compat = _compat, \
	}

/**
 * Gets a pointer to the current state.
 *
 * @return pointer to state
 */
struct sandbox_state *state_get_current(void);

/**
 * Read the sandbox state from the supplied device tree file
 *
 * This calls all registered state handlers to read in the sandbox state
 * from a previous test run.
 *
 * @param state		Sandbox state to update
 * @param fname		Filename of device tree file to read from
 * @return 0 if OK, -ve on error
 */
int sandbox_read_state(struct sandbox_state *state, const char *fname);

/**
 * Write the sandbox state to the supplied device tree file
 *
 * This calls all registered state handlers to write out the sandbox state
 * so that it can be preserved for a future test run.
 *
 * If the file exists it is overwritten.
 *
 * @param state		Sandbox state to update
 * @param fname		Filename of device tree file to write to
 * @return 0 if OK, -ve on error
 */
int sandbox_write_state(struct sandbox_state *state, const char *fname);

/**
 * Add a property to a sandbox state node
 *
 * This is equivalent to fdt_setprop except that it automatically enlarges
 * the device tree if necessary. That means it is safe to write any amount
 * of data here.
 *
 * This function can only be called from within struct sandbox_state_io's
 * ->write method, i.e. within state I/O drivers.
 *
 * @param node		Device tree node to write to
 * @param prop_name	Property to write
 * @param data		Data to write into property
 * @param size		Size of data to write into property
 */
int state_setprop(int node, const char *prop_name, const void *data, int size);

/**
 * Control skipping of time delays
 *
 * Some tests have unnecessay time delays (e.g. USB). Allow these to be
 * skipped to speed up testing
 *
 * @param skip_delays	true to skip delays from now on, false to honour delay
 *			requests
 */
void state_set_skip_delays(bool skip_delays);

/**
 * See if delays should be skipped
 *
 * @return true if delays should be skipped, false if they should be honoured
 */
bool state_get_skip_delays(void);

/**
 * state_reset_for_test() - Reset ready to re-run tests
 *
 * This clears out any test state ready for another test run.
 */
void state_reset_for_test(struct sandbox_state *state);

/**
 * state_show() - Show information about the sandbox state
 *
 * @param state		Sandbox state to show
 */
void state_show(struct sandbox_state *state);

/**
 * Initialize the test system state
 */
int state_init(void);

/**
 * Uninitialize the test system state, writing out state if configured to
 * do so.
 *
 * @return 0 if OK, -ve on error
 */
int state_uninit(void);

#endif
