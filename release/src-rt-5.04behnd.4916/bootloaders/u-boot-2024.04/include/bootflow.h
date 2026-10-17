/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootflow_h
#define __bootflow_h

#include <bootdev.h>
#include <dm/ofnode_decl.h>
#include <linux/list.h>

struct bootstd_priv;
struct expo;

enum {
	BOOTFLOW_MAX_USED_DEVS	= 16,
};

/**
 * enum bootflow_state_t - states that a particular bootflow can be in
 *
 * Only bootflows in state BOOTFLOWST_READY can be used to boot.
 *
 * See bootflow_state[] for the names for each of these
 */
enum bootflow_state_t {
	BOOTFLOWST_BASE,	/**< Nothing known yet */
	BOOTFLOWST_MEDIA,	/**< Media exists */
	BOOTFLOWST_PART,	/**< Partition exists */
	BOOTFLOWST_FS,		/**< Filesystem exists */
	BOOTFLOWST_FILE,	/**< Bootflow file exists */
	BOOTFLOWST_READY,	/**< Bootflow file loaded */

	BOOTFLOWST_COUNT
};

/**
 * enum bootflow_flags_t - flags for bootflows
 *
 * @BOOTFLOWF_USE_PRIOR_FDT: Indicates that an FDT was not found by the bootmeth
 *	and it is using the prior-stage FDT, which is the U-Boot control FDT.
 *	This is only possible with the EFI bootmeth (distro-efi) and only when
 *	CONFIG_OF_HAS_PRIOR_STAGE is enabled
 * @BOOTFLOWF_STATIC_BUF: Indicates that @bflow->buf is statically set, rather
 *	than being allocated by malloc().
 * @BOOTFLOWF_USE_BUILTIN_FDT : Indicates that current bootflow uses built-in FDT
 */
enum bootflow_flags_t {
	BOOTFLOWF_USE_PRIOR_FDT	= 1 << 0,
	BOOTFLOWF_STATIC_BUF	= 1 << 1,
	BOOTFLOWF_USE_BUILTIN_FDT	= 1 << 2,
};

/**
 * struct bootflow - information about a bootflow
 *
 * This is connected into two separate linked lists:
 *
 *   bm_sibling - links all bootflows in the same bootdev
 *   glob_sibling - links all bootflows in all bootdevs
 *
 * @bm_node: Points to siblings in the same bootdev
 * @glob_node: Points to siblings in the global list (all bootdev)
 * @dev: Bootdev device which produced this bootflow
 * @blk: Block device which contains this bootflow, NULL if this is a network
 *	device or sandbox 'host' device
 * @part: Partition number (0 for whole device)
 * @fs_type: Filesystem type (FS_TYPE...) if this is fixed by the media, else 0.
 *	For example, the sandbox host-filesystem bootdev sets this to
 *	FS_TYPE_SANDBOX
 * @method: Bootmethod device used to perform the boot and read files
 * @name: Name of bootflow (allocated)
 * @state: Current state (enum bootflow_state_t)
 * @subdir: Subdirectory to fetch files from (with trailing /), or NULL if none
 * @fname: Filename of bootflow file (allocated)
 * @logo: Logo to display for this bootflow (BMP format)
 * @logo_size: Size of the logo in bytes
 * @buf: Bootflow file contents (allocated unless @flags & BOOTFLOWF_STATIC_BUF)
 * @size: Size of bootflow file in bytes
 * @err: Error number received (0 if OK)
 * @os_name: Name of the OS / distro being booted, or NULL if not known
 *	(allocated)
 * @fdt_fname: Filename of FDT file
 * @fdt_size: Size of FDT file
 * @fdt_addr: Address of loaded fdt
 * @flags: Flags for the bootflow (see enum bootflow_flags_t)
 * @cmdline: OS command line, or NULL if not known (allocated)
 * @x86_setup: Pointer to x86 setup block inside @buf, NULL if not present
 * @bootmeth_priv: Private data for the bootmeth
 */
struct bootflow {
	struct list_head bm_node;
	struct list_head glob_node;
	struct udevice *dev;
	struct udevice *blk;
	int part;
	int fs_type;
	struct udevice *method;
	char *name;
	enum bootflow_state_t state;
	char *subdir;
	char *fname;
	void *logo;
	uint logo_size;
	char *buf;
	int size;
	int err;
	char *os_name;
	char *fdt_fname;
	int fdt_size;
	ulong fdt_addr;
	int flags;
	char *cmdline;
	void *x86_setup;
	void *bootmeth_priv;
};

/**
 * enum bootflow_iter_flags_t - flags for the bootflow iterator
 *
 * @BOOTFLOWIF_FIXED: Only used fixed/internal media
 * @BOOTFLOWIF_SHOW: Show each bootdev before scanning it; show each hunter
 * before using it
 * @BOOTFLOWIF_ALL: Return bootflows with errors as well
 * @BOOTFLOWIF_HUNT: Hunt for new bootdevs using the bootdrv hunters
 *
 * Internal flags:
 * @BOOTFLOWIF_SINGLE_DEV: (internal) Just scan one bootdev
 * @BOOTFLOWIF_SKIP_GLOBAL: (internal) Don't scan global bootmeths
 * @BOOTFLOWIF_SINGLE_UCLASS: (internal) Keep scanning through all devices in
 * this uclass (used with things like "mmc")
 * @BOOTFLOWIF_SINGLE_MEDIA: (internal) Scan one media device in the uclass (used
 * with things like "mmc1")
 */
enum bootflow_iter_flags_t {
	BOOTFLOWIF_FIXED		= 1 << 0,
	BOOTFLOWIF_SHOW			= 1 << 1,
	BOOTFLOWIF_ALL			= 1 << 2,
	BOOTFLOWIF_HUNT			= 1 << 3,

	/*
	 * flags used internally by standard boot - do not set these when
	 * calling bootflow_scan_bootdev() etc.
	 */
	BOOTFLOWIF_SINGLE_DEV		= 1 << 16,
	BOOTFLOWIF_SKIP_GLOBAL		= 1 << 17,
	BOOTFLOWIF_SINGLE_UCLASS	= 1 << 18,
	BOOTFLOWIF_SINGLE_MEDIA		= 1 << 19,
};

/**
 * enum bootflow_meth_flags_t - flags controlling which bootmeths are used
 *
 * Used during iteration, e.g. by bootdev_find_by_label(), to determine which
 * bootmeths are used for the current bootdev. The flags reset when the bootdev
 * changes
 *
 * @BOOTFLOW_METHF_DHCP_ONLY: Only use dhcp (scripts and EFI)
 * @BOOTFLOW_METHF_PXE_ONLY: Only use pxe (PXE boot)
 * @BOOTFLOW_METHF_SINGLE_DEV: Scan only a single bootdev (used for labels like
 * "3"). This is used if a sequence number is provided instead of a label
 * @BOOTFLOW_METHF_SINGLE_UCLASS: Scan all bootdevs in this one uclass (used
 * with things like "mmc"). If this is not set, then the bootdev has an integer
 * value in the label (like "mmc2")
 */
enum bootflow_meth_flags_t {
	BOOTFLOW_METHF_DHCP_ONLY	= 1 << 0,
	BOOTFLOW_METHF_PXE_ONLY		= 1 << 1,
	BOOTFLOW_METHF_SINGLE_DEV	= 1 << 2,
	BOOTFLOW_METHF_SINGLE_UCLASS	= 1 << 3,
};

/**
 * struct bootflow_iter - state for iterating through bootflows
 *
 * This starts at with the first bootdev/partition/bootmeth and can be used to
 * iterate through all of them.
 *
 * Iteration starts with the bootdev. The first partition (0, i.e. whole device)
 * is scanned first. For partition 0, it iterates through all the available
 * bootmeths to see which one(s) can provide a bootflow. Then it moves to
 * parition 1 (if there is one) and the process continues. Once all partitions
 * are examined, it moves to the next bootdev.
 *
 * Initially @max_part is 0, meaning that only the whole device (@part=0) can be
 * used. During scanning, if a partition table is found, then @max_part is
 * updated to a larger value, no less than the number of available partitions.
 * This ensures that iteration works through all partitions on the bootdev.
 *
 * @flags: Flags to use (see enum bootflow_iter_flags_t). If
 *	BOOTFLOWIF_GLOBAL_FIRST is enabled then the global bootmeths are being
 *	scanned, otherwise we have moved onto the bootdevs
 * @dev: Current bootdev, NULL if none. This is only ever updated in
 * bootflow_iter_set_dev()
 * @part: Current partition number (0 for whole device)
 * @method: Current bootmeth
 * @max_part: Maximum hardware partition number in @dev, 0 if there is no
 *	partition table
 * @first_bootable: First bootable partition, or 0 if none
 * @err: Error obtained from checking the last iteration. This is used to skip
 *	forward (e.g. to skip the current partition because it is not valid)
 *	-ESHUTDOWN: try next bootdev
 * @num_devs: Number of bootdevs in @dev_used
 * @max_devs: Maximum number of entries in @dev_used
 * @dev_used: List of bootdevs used during iteration
 * @labels: List of labels to scan for bootdevs
 * @cur_label: Current label being processed
 * @num_methods: Number of bootmeth devices in @method_order
 * @cur_method: Current method number, an index into @method_order
 * @first_glob_method: First global method, if any, else -1
 * @cur_prio: Current priority being scanned
 * @method_order: List of bootmeth devices to use, in order. The normal methods
 *	appear first, then the global ones, if any
 * @doing_global: true if we are iterating through the global bootmeths (which
 *	happens before the normal ones)
 * @method_flags: flags controlling which methods should be used for this @dev
 * (enum bootflow_meth_flags_t)
 */
struct bootflow_iter {
	int flags;
	struct udevice *dev;
	int part;
	struct udevice *method;
	int max_part;
	int first_bootable;
	int err;
	int num_devs;
	int max_devs;
	struct udevice *dev_used[BOOTFLOW_MAX_USED_DEVS];
	const char *const *labels;
	int cur_label;
	int num_methods;
	int cur_method;
	int first_glob_method;
	enum bootdev_prio_t cur_prio;
	struct udevice **method_order;
	bool doing_global;
	int method_flags;
};

/**
 * bootflow_init() - Set up a bootflow struct
 *
 * The bootflow is zeroed and set to state BOOTFLOWST_BASE
 *
 * @bflow: Struct to set up
 * @bootdev: Bootdev to use
 * @meth: Bootmeth to use
 */
void bootflow_init(struct bootflow *bflow, struct udevice *bootdev,
		   struct udevice *meth);

/**
 * bootflow_iter_init() - Reset a bootflow iterator
 *
 * This sets everything to the starting point, ready for use.
 *
 * @iter: Place to store private info (inited by this call)
 * @flags: Flags to use (see enum bootflow_iter_flags_t)
 */
void bootflow_iter_init(struct bootflow_iter *iter, int flags);

/**
 * bootflow_iter_uninit() - Free memory used by an interator
 *
 * @iter:	Iterator to free
 */
void bootflow_iter_uninit(struct bootflow_iter *iter);

/**
 * bootflow_iter_drop_bootmeth() - Remove a bootmeth from an iterator
 *
 * Update the iterator so that the bootmeth will not be used again while this
 * iterator is in use
 *
 * @iter: Iterator to update
 * @bmeth: Boot method to remove
 */
int bootflow_iter_drop_bootmeth(struct bootflow_iter *iter,
				const struct udevice *bmeth);

/**
 * bootflow_scan_first() - find the first bootflow for a device or label
 *
 * If @flags includes BOOTFLOWIF_ALL then bootflows with errors are returned too
 *
 * @dev:	Boot device to scan, NULL to work through all of them until it
 *	finds one that can supply a bootflow
 * @label:	Label to control the scan, NULL to work through all devices
 *	until it finds one that can supply a bootflow
 * @iter:	Place to store private info (inited by this call)
 * @flags:	Flags for iterator (enum bootflow_iter_flags_t). Note that if
 *	@dev is NULL, then BOOTFLOWIF_SKIP_GLOBAL is set automatically by this
 *	function
 * @bflow:	Place to put the bootflow if found
 * Return: 0 if found,  -ENODEV if no device, other -ve on other error
 *	(iteration can continue)
 */
int bootflow_scan_first(struct udevice *dev, const char *label,
			struct bootflow_iter *iter, int flags,
			struct bootflow *bflow);

/**
 * bootflow_scan_next() - find the next bootflow
 *
 * This works through the available bootdev devices until it finds one that
 * can supply a bootflow. It then returns that bootflow
 *
 * @iter:	Private info (as set up by bootflow_scan_first())
 * @bflow:	Place to put the bootflow if found
 * Return: 0 if found, -ENODEV if no device, -ESHUTDOWN if no more bootflows,
 *	other -ve on other error (iteration can continue)
 */
int bootflow_scan_next(struct bootflow_iter *iter, struct bootflow *bflow);

/**
 * bootflow_first_glob() - Get the first bootflow from the global list
 *
 * Returns the first bootflow in the global list, no matter what bootflow it is
 * attached to
 *
 * @bflowp: Returns a pointer to the bootflow
 * Return: 0 if found, -ENOENT if there are no bootflows
 */
int bootflow_first_glob(struct bootflow **bflowp);

/**
 * bootflow_next_glob() - Get the next bootflow from the global list
 *
 * Returns the next bootflow in the global list, no matter what bootflow it is
 * attached to
 *
 * @bflowp: On entry, the last bootflow returned , e.g. from
 *	bootflow_first_glob()
 * Return: 0 if found, -ENOENT if there are no more bootflows
 */
int bootflow_next_glob(struct bootflow **bflowp);

/**
 * bootflow_free() - Free memory used by a bootflow
 *
 * This frees fields within @bflow, but not the @bflow pointer itself
 */
void bootflow_free(struct bootflow *bflow);

/**
 * bootflow_boot() - boot a bootflow
 *
 * @bflow: Bootflow to boot
 * Return: -EPROTO if bootflow has not been loaded, -ENOSYS if the bootflow
 *	type is not supported, -EFAULT if the boot returned without an error
 *	when we are expecting it to boot, -ENOTSUPP if trying method resulted in
 *	finding out that is not actually supported for this boot and should not
 *	be tried again unless something changes
 */
int bootflow_boot(struct bootflow *bflow);

/**
 * bootflow_read_all() - Read all bootflow files
 *
 * Some bootmeths delay reading of large files until booting is requested. This
 * causes those files to be read.
 *
 * @bflow: Bootflow to read
 * Return: result of trying to read
 */
int bootflow_read_all(struct bootflow *bflow);

/**
 * bootflow_run_boot() - Try to boot a bootflow
 *
 * @iter: Current iteration (or NULL if none). Used to disable a bootmeth if the
 *	boot returns -ENOTSUPP
 * @bflow: Bootflow to boot
 * Return: result of trying to boot
 */
int bootflow_run_boot(struct bootflow_iter *iter, struct bootflow *bflow);

/**
 * bootflow_state_get_name() - Get the name of a bootflow state
 *
 * @state: State to check
 * Return: name, or "?" if invalid
 */
const char *bootflow_state_get_name(enum bootflow_state_t state);

/**
 * bootflow_remove() - Remove a bootflow and free its memory
 *
 * This updates the linked lists containing the bootflow then frees it.
 *
 * @bflow: Bootflow to remove
 */
void bootflow_remove(struct bootflow *bflow);

/**
 * bootflow_iter_check_blk() - Check that a bootflow uses a block device
 *
 * This checks the bootdev in the bootflow to make sure it uses a block device
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. ethernet)
 */
int bootflow_iter_check_blk(const struct bootflow_iter *iter);

/**
 * bootflow_iter_check_sf() - Check that a bootflow uses SPI FLASH
 *
 * This checks the bootdev in the bootflow to make sure it uses SPI flash
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. ethernet)
 */
int bootflow_iter_check_sf(const struct bootflow_iter *iter);

/**
 * bootflow_iter_check_net() - Check that a bootflow uses a network device
 *
 * This checks the bootdev in the bootflow to make sure it uses a network
 * device
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. MMC)
 */
int bootflow_iter_check_net(const struct bootflow_iter *iter);

/**
 * bootflow_iter_check_system() - Check that a bootflow uses the bootstd device
 *
 * This checks the bootdev in the bootflow to make sure it uses the bootstd
 * device
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. MMC)
 */
int bootflow_iter_check_system(const struct bootflow_iter *iter);

/**
 * bootflow_menu_new() - Create a new bootflow menu
 *
 * @expp: Returns the expo created
 * Returns 0 on success, -ve on error
 */
int bootflow_menu_new(struct expo **expp);

/**
 * bootflow_menu_apply_theme() - Apply a theme to a bootmenu
 *
 * @exp: Expo to update
 * @node: Node containing the theme information
 * Returns 0 on success, -ve on error
 */
int bootflow_menu_apply_theme(struct expo *exp, ofnode node);

/**
 * bootflow_menu_run() - Create and run a menu of available bootflows
 *
 * @std: Bootstd information
 * @text_mode: Uses a text-based menu suitable for a serial port
 * @bflowp: Returns chosen bootflow (set to NULL if nothing is chosen)
 * @return 0 if an option was chosen, -EAGAIN if nothing was chosen, -ve on
 * error
 */
int bootflow_menu_run(struct bootstd_priv *std, bool text_mode,
		      struct bootflow **bflowp);

#define BOOTFLOWCL_EMPTY	((void *)1)

/**
 * cmdline_set_arg() - Update or read an argument in a cmdline string
 *
 * Handles updating a single arg in a cmdline string, returning it in a supplied
 * buffer; also reading an arg from a cmdline string
 *
 * When updating, consecutive spaces are squashed as are spaces at the start and
 * end.
 *
 * @buf: Working buffer to use (initial contents are ignored). Use NULL when
 * reading
 * @maxlen: Length of working buffer. Use 0 when reading
 * @cmdline: Command line to update, in the form:
 *
 *	fred mary= jane=123 john="has spaces"
 *
 * @set_arg: Argument to set or read (may or may not exist)
 * @new_val: Value for the new argument. May not include quotes (") but may
 * include embedded spaces, in which case it will be quoted when added to the
 * command line. Use NULL to delete the argument from @cmdline, BOOTFLOWCL_EMPTY
 * to set it to an empty value (no '=' sign after arg), "" to add an '=' sign
 * but with an empty value. Use NULL when reading.
 * @posp: Ignored when setting an argument; when getting an argument, returns
 * the start position of its value in @cmdline, after the first quote, if any
 *
 * Return:
 * For updating:
 *	length of new buffer (including \0 terminator) on success, -ENOENT if
 *	@new_val is NULL and @set_arg does not exist in @from, -EINVAL if a
 *	quoted arg-value in @from is not terminated with a quote, -EBADF if
 *	@new_val has spaces but does not start and end with quotes (or it has
 *	quotes in the middle of the string), -E2BIG if @maxlen is too small
 * For reading:
 *	length of arg value (excluding quotes), -ENOENT if not found
 */
int cmdline_set_arg(char *buf, int maxlen, const char *cmdline,
		    const char *set_arg, const char *new_val, int *posp);

/**
 * bootflow_cmdline_set_arg() - Set a single argument for a bootflow
 *
 * Update the allocated cmdline and set the bootargs variable
 *
 * @bflow: Bootflow to update
 * @arg: Argument to update (e.g. "console")
 * @val: Value to set (e.g. "ttyS2") or NULL to delete the argument if present,
 * "" to set it to an empty value (e.g. "console=") and BOOTFLOWCL_EMPTY to add
 * it without any value ("initrd")
 * @set_env: true to set the "bootargs" environment variable too
 *
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int bootflow_cmdline_set_arg(struct bootflow *bflow, const char *arg,
			     const char *val, bool set_env);

/**
 * cmdline_get_arg() - Read an argument from a cmdline
 *
 * @cmdline: Command line to read, in the form:
 *
 *	fred mary= jane=123 john="has spaces"
 * @arg: Argument to read (may or may not exist)
 * @posp: Returns position of argument (after any leading quote) if present
 * Return: Length of argument value excluding quotes if found, -ENOENT if not
 * found
 */
int cmdline_get_arg(const char *cmdline, const char *arg, int *posp);

/**
 * bootflow_cmdline_get_arg() - Read an argument from a cmdline
 *
 * @bootflow: Bootflow to read from
 * @arg: Argument to read (may or may not exist)
 * @valp: Returns a pointer to the argument (after any leading quote) if present
 * Return: Length of argument value excluding quotes if found, -ENOENT if not
 * found
 */
int bootflow_cmdline_get_arg(struct bootflow *bflow, const char *arg,
			     const char **val);

/**
 * bootflow_cmdline_auto() - Automatically set a value for a known argument
 *
 * This handles a small number of known arguments, for Linux in particular. It
 * adds suitable kernel parameters automatically, e.g. to enable the console.
 *
 * @bflow: Bootflow to update
 * @arg: Name of argument to set (e.g. "earlycon" or "console")
 * Return: 0 if OK -ve on error
 */
int bootflow_cmdline_auto(struct bootflow *bflow, const char *arg);

#endif
