/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Core ACPI (Advanced Configuration and Power Interface) support
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __DM_ACPI_H__
#define __DM_ACPI_H__

/* Allow operations to be optional for ACPI */
#if CONFIG_IS_ENABLED(ACPIGEN)
#define ACPI_OPS_PTR(_ptr)	.acpi_ops	= _ptr,
#else
#define ACPI_OPS_PTR(_ptr)
#endif

/* Length of an ACPI name string, excluding null terminator */
#define ACPI_NAME_LEN	4

/* Length of an ACPI name string including nul terminator */
#define ACPI_NAME_MAX	(ACPI_NAME_LEN + 1)

/* Number of nested objects supported */
#define ACPIGEN_LENSTACK_SIZE 10

#if !defined(__ACPI__)

#include <linker_lists.h>

struct nhlt;
struct udevice;

/** enum acpi_dump_option - selects what ACPI information to dump */
enum acpi_dump_option {
	ACPI_DUMP_LIST,		/* Just the list of items */
	ACPI_DUMP_CONTENTS,	/* Include the binary contents also */
};

/**
 * struct acpi_ctx - Context used for writing ACPI tables
 *
 * This contains a few useful pieces of information used when writing
 *
 * @base: Base address of ACPI tables
 * @current: Current address for writing
 * @tab_start: Address of start of the table being written. This is set up
 * before the writer or driver method is called. It must not be changed by the
 * method
 * @rsdp: Pointer to the Root System Description Pointer, typically used when
 *	adding a new table. The RSDP holds pointers to the RSDT and XSDT.
 * @rsdt: Pointer to the Root System Description Table
 * @xsdt: Pointer to the Extended System Description Table
 * @facs: Pointer to the Firmware ACPI Control Structure
 * @dsdt: Pointer to the Differentiated System Description Table
 * @nhlt: Intel Non-High-Definition-Audio Link Table (NHLT) pointer, used to
 *	build up information that audio codecs need to provide in the NHLT ACPI
 *	table
 * @len_stack: Stack of 'length' words to fix up later
 * @ltop: Points to current top of stack (0 = empty)
 */
struct acpi_ctx {
	void *base;
	void *current;
	void *tab_start;
	struct acpi_rsdp *rsdp;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;
	struct acpi_facs *facs;
	struct acpi_table_header *dsdt;
	struct nhlt *nhlt;
	char *len_stack[ACPIGEN_LENSTACK_SIZE];
	int ltop;
};

/**
 * enum acpi_writer_flags_t - flags to use for the ACPI writers
 *
 * ACPIWF_ALIGN64 - align to 64 bytes after writing this one (default is 16)
 */
enum acpi_writer_flags_t {
	ACPIWF_ALIGN64	= 1 << 0,
};

struct acpi_writer;

/**
 * acpi_writer_func() - Function that can write an ACPI table
 *
 * @ctx: ACPI context to use for writing
 * @entry: Linker-list entry for this writer
 * @return 0 if OK, -ve on error
 */
typedef int (*acpi_writer_func)(struct acpi_ctx *ctx,
				const struct acpi_writer *entry);

/**
 * struct acpi_writer - an ACPI table that can be written
 *
 * @name: Name of the writer
 * @table: Table name that is generated (e.g. "DSDT")
 * @h_write: Writer function
 */
struct acpi_writer {
	const char *name;
	const char *table;
	acpi_writer_func h_write;
	int flags;
};

/* Declare a new ACPI-table writer */
#define ACPI_WRITER(_name, _table, _write, _flags)					\
	ll_entry_declare(struct acpi_writer, _name, acpi_writer) = {	\
		.name = #_name,						\
		.table = _table,					\
		.h_write = _write,					\
		.flags = _flags,					\
	}

/* Get a pointer to a given ACPI-table writer */
#define ACPI_WRITER_GET(_name)						\
	ll_entry_get(struct acpi_writer, _name, acpi_writer)

/**
 * struct acpi_ops - ACPI operations supported by driver model
 */
struct acpi_ops {
	/**
	 * get_name() - Obtain the ACPI name of a device
	 *
	 * @dev: Device to check
	 * @out_name: Place to put the name, must hold at least ACPI_NAME_MAX
	 *	bytes
	 * @return 0 if OK, -ENOENT if no name is available, other -ve value on
	 *	other error
	 */
	int (*get_name)(const struct udevice *dev, char *out_name);

	/**
	 * write_tables() - Write out any tables required by this device
	 *
	 * @dev: Device to write
	 * @ctx: ACPI context to use
	 * @return 0 if OK, -ve on error
	 */
	int (*write_tables)(const struct udevice *dev, struct acpi_ctx *ctx);

	/**
	 * fill_ssdt() - Generate SSDT code for a device
	 *
	 * This is called to create the SSDT code. The method should write out
	 * whatever ACPI code is needed by this device. It will end up in the
	 * SSDT table.
	 *
	 * Note that this is called 'fill' because the entire contents of the
	 * SSDT is build by calling this method on all devices.
	 *
	 * @dev: Device to write
	 * @ctx: ACPI context to use
	 * @return 0 if OK, -ve on error
	 */
	int (*fill_ssdt)(const struct udevice *dev, struct acpi_ctx *ctx);

	/**
	 * inject_dsdt() - Generate DSDT code for a device
	 *
	 * This is called to create the DSDT code. The method should write out
	 * whatever ACPI code is needed by this device. It will end up in the
	 * DSDT table.
	 *
	 * Note that this is called 'inject' because the output of calling this
	 * method on all devices is injected into the DSDT, the bulk of which
	 * is written in .asl files for the board.
	 *
	 * @dev: Device to write
	 * @ctx: ACPI context to use
	 * @return 0 if OK, -ve on error
	 */
	int (*inject_dsdt)(const struct udevice *dev, struct acpi_ctx *ctx);

	/**
	 * setup_nhlt() - Set up audio information for this device
	 *
	 * The method can add information to ctx->nhlt if it likes
	 *
	 * @return 0 if OK, -ENODATA if nothing to add, -ve on error
	 */
	int (*setup_nhlt)(const struct udevice *dev, struct acpi_ctx *ctx);
};

#define device_get_acpi_ops(dev)	((dev)->driver->acpi_ops)

/**
 * acpi_get_name() - Obtain the ACPI name of a device
 *
 * @dev: Device to check
 * @out_name: Place to put the name, must hold at least ACPI_NAME_MAX
 *	bytes
 * Return: 0 if OK, -ENOENT if no name is available, other -ve value on
 *	other error
 */
int acpi_get_name(const struct udevice *dev, char *out_name);

/**
 * acpi_copy_name() - Copy an ACPI name to an output buffer
 *
 * This convenience function can be used to return a literal string as a name
 * in functions that implement the get_name() method.
 *
 * For example:
 *
 *	static int mydev_get_name(const struct udevice *dev, char *out_name)
 *	{
 *		return acpi_copy_name(out_name, "WIBB");
 *	}
 *
 * @out_name: Place to put the name
 * @name: Name to copy
 * Return: 0 (always)
 */
int acpi_copy_name(char *out_name, const char *name);

/**
 * acpi_write_dev_tables() - Write ACPI tables required by devices
 *
 * This scans through all devices and tells them to write any tables they want
 * to write.
 *
 * Return: 0 if OK, -ve if any device returned an error
 */
int acpi_write_dev_tables(struct acpi_ctx *ctx);

/**
 * acpi_fill_ssdt() - Generate ACPI tables for SSDT
 *
 * This is called to create the SSDT code for all devices.
 *
 * @ctx: ACPI context to use
 * Return: 0 if OK, -ve on error
 */
int acpi_fill_ssdt(struct acpi_ctx *ctx);

/**
 * acpi_inject_dsdt() - Generate ACPI tables for DSDT
 *
 * This is called to create the DSDT code for all devices.
 *
 * @ctx: ACPI context to use
 * Return: 0 if OK, -ve on error
 */
int acpi_inject_dsdt(struct acpi_ctx *ctx);

/**
 * acpi_setup_nhlt() - Set up audio information
 *
 * This is called to set up the nhlt information for all devices.
 *
 * @ctx: ACPI context to use
 * @nhlt: Pointer to nhlt information to add to
 * Return: 0 if OK, -ve on error
 */
int acpi_setup_nhlt(struct acpi_ctx *ctx, struct nhlt *nhlt);

/**
 * acpi_add_other_item() - Add a new table to the list of ACPI tables
 *
 * This adds an entry of type ACPIT_TYPE_OTHER
 *
 * @ctx: ACPI context
 * @writer: Writer entry that generated the data
 * @type: Table type it refers to
 * @start: The start of the data (the end is obtained from ctx->current)
 * @return 0 if OK, -ENOSPC if too many items, -ENOMEM if out of memory
 */
int acpi_add_other_item(struct acpi_ctx *ctx, const struct acpi_writer *writer,
			void *start);

/**
 * acpi_dump_items() - Dump out the collected ACPI items
 *
 * This lists the ACPI DSDT and SSDT items generated by the various U-Boot
 * drivers.
 *
 * @option: Sets what should be dumpyed
 */
void acpi_dump_items(enum acpi_dump_option option);

/**
 * acpi_get_path() - Get the full ACPI path for a device
 *
 * This checks for any override in the device tree and calls acpi_device_path()
 * if not
 *
 * @dev: Device to check
 * @out_path: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * Return: 0 if OK, -ve on error
 */
int acpi_get_path(const struct udevice *dev, char *out_path, int maxlen);

/**
 * acpi_reset_items() - Reset the list of ACPI items to empty
 *
 * This list keeps track of DSDT and SSDT items that are generated
 * programmatically. The 'acpi items' command shows the list. Use this function
 * to empty the list, before writing new items.
 */
void acpi_reset_items(void);

/**
 * acpi_write_one() - Call a single ACPI writer entry
 *
 * This handles aligning the context afterwards, if the entry flags indicate
 * that.
 *
 * @ctx: ACPI context to use
 * @entry: Entry to call
 * @return 0 if OK, -ENOENT if this writer produced an empty entry, other -ve
 * value on error
 */
int acpi_write_one(struct acpi_ctx *ctx, const struct acpi_writer *entry);

/**
 * acpi_setup_ctx() - Set up a new ACPI context
 *
 * This zeros the context and sets up the base and current pointers, ensuring
 * that they are aligned. Then it writes the acpi_start and acpi_ctx values in
 * global_data
 *
 * @ctx: ACPI context to set up
 * @start: Start address for ACPI table
 */
void acpi_setup_ctx(struct acpi_ctx *ctx, ulong start);

/**
 * acpi_write_one() - Call a single ACPI writer entry
 *
 * This handles aligning the context afterwards, if the entry flags indicate
 * that.
 *
 * @ctx: ACPI context to use
 * @entry: Entry to call
 * @return 0 if OK, -ENOENT if this writer produced an empty entry, other -ve
 * value on error
 */
int acpi_write_one(struct acpi_ctx *ctx, const struct acpi_writer *entry);

#endif /* __ACPI__ */

#endif
