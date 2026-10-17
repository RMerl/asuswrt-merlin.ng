/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootmeth_h
#define __bootmeth_h

struct blk_desc;
struct bootflow;
struct bootflow_iter;
struct udevice;

/**
 * enum bootmeth_flags - Flags for bootmeths
 *
 * @BOOTMETHF_GLOBAL: bootmeth handles bootdev selection automatically
 * @BOOTMETHF_ANY_PART: bootmeth is willing to check any partition, even if it
 * has no filesystem
 */
enum bootmeth_flags {
	BOOTMETHF_GLOBAL	= BIT(0),
	BOOTMETHF_ANY_PART	= BIT(1),
};

/**
 * struct bootmeth_uc_plat - information the uclass keeps about each bootmeth
 *
 * @desc: A long description of the bootmeth
 * @flags: Flags for this bootmeth (enum bootmeth_flags)
 */
struct bootmeth_uc_plat {
	const char *desc;
	int flags;
};

/** struct bootmeth_ops - Operations for boot methods */
struct bootmeth_ops {
	/**
	 * get_state_desc() - get detailed state information
	 *
	 * Prodecues a textual description of the state of the bootmeth. This
	 * can include newline characters if it extends to multiple lines. It
	 * must be a nul-terminated string.
	 *
	 * This may involve reading state from the system, e.g. some data in
	 * the firmware area.
	 *
	 * @dev:	Bootmethod device to check
	 * @buf:	Buffer to place the info in (terminator must fit)
	 * @maxsize:	Size of buffer
	 * Returns: 0 if OK, -ENOSPC is buffer is too small, other -ve error if
	 * something else went wrong
	 */
	int (*get_state_desc)(struct udevice *dev, char *buf, int maxsize);

	/**
	 * check_supported() - check if a bootmeth supports this bootdev
	 *
	 * This is optional. If not provided, the bootdev is assumed to be
	 * supported
	 *
	 * The bootmeth can check the bootdev (e.g. to make sure it is a
	 * network device) or the partition information. The following fields
	 * in @iter are available:
	 *
	 *   name, dev, state, part
	 *   max_part may be set if part != 0 (i.e. there is a valid partition
	 *      table). Otherwise max_part is 0
	 *   method is available but is the same as @dev
	 *   the partition has not yet been read, nor has the filesystem been
	 *   checked
	 *
	 * It may update only the flags in @iter
	 *
	 * @dev:	Bootmethod device to check against
	 * @iter:	On entry, provides bootdev, hwpart, part
	 * Return: 0 if OK, -ENOTSUPP if this bootdev is not supported
	 */
	int (*check)(struct udevice *dev, struct bootflow_iter *iter);

	/**
	 * read_bootflow() - read a bootflow for a device
	 *
	 * @dev:	Bootmethod device to use
	 * @bflow:	On entry, provides dev, hwpart, part and method.
	 *	Returns updated bootflow if found
	 * Return: 0 if OK, -ve on error
	 */
	int (*read_bootflow)(struct udevice *dev, struct bootflow *bflow);

	/**
	 * set_bootflow() - set the bootflow for a device
	 *
	 * This provides a bootflow file to the bootmeth, to see if it is valid.
	 * If it is, the bootflow is set up accordingly.
	 *
	 * @dev:	Bootmethod device to use
	 * @bflow:	On entry, provides bootdev.
	 *	Returns updated bootflow if found
	 * @buf:	Buffer containing the possible bootflow file
	 * @size:	Size of file
	 * Return: 0 if OK, -ve on error
	 */
	int (*set_bootflow)(struct udevice *dev, struct bootflow *bflow,
			    char *buf, int size);

	/**
	 * read_file() - read a file needed for a bootflow
	 *
	 * Read a file from the same place as the bootflow came from
	 *
	 * @dev:	Bootmethod device to use
	 * @bflow:	Bootflow providing info on where to read from
	 * @file_path:	Path to file (may be absolute or relative)
	 * @addr:	Address to load file
	 * @sizep:	On entry provides the maximum permitted size; on exit
	 *		returns the size of the file
	 * Return: 0 if OK, -ENOSPC if the file is too large for @sizep, other
	 *	-ve value if something else goes wrong
	 */
	int (*read_file)(struct udevice *dev, struct bootflow *bflow,
			 const char *file_path, ulong addr, ulong *sizep);
#if CONFIG_IS_ENABLED(BOOTSTD_FULL)
	/**
	 * readall() - read all files for a bootflow
	 *
	 * @dev:	Bootmethod device to boot
	 * @bflow:	Bootflow to read
	 * Return: 0 if OK, -EIO on I/O error, other -ve on other error
	 */
	int (*read_all)(struct udevice *dev, struct bootflow *bflow);
#endif /* BOOTSTD_FULL */
	/**
	 * boot() - boot a bootflow
	 *
	 * @dev:	Bootmethod device to boot
	 * @bflow:	Bootflow to boot
	 * Return: does not return on success, since it should boot the
	 *	Operating Systemn. Returns -EFAULT if that fails, -ENOTSUPP if
	 *	trying method resulted in finding out that is not actually
	 *	supported for this boot and should not be tried again unless
	 *	something changes, other -ve on other error
	 */
	int (*boot)(struct udevice *dev, struct bootflow *bflow);
};

#define bootmeth_get_ops(dev)  ((struct bootmeth_ops *)(dev)->driver->ops)

/**
 * bootmeth_get_state_desc() - get detailed state information
 *
 * Prodecues a textual description of the state of the bootmeth. This
 * can include newline characters if it extends to multiple lines. It
 * must be a nul-terminated string.
 *
 * This may involve reading state from the system, e.g. some data in
 * the firmware area.
 *
 * @dev:	Bootmethod device to check
 * @buf:	Buffer to place the info in (terminator must fit)
 * @maxsize:	Size of buffer
 * Returns: 0 if OK, -ENOSPC is buffer is too small, other -ve error if
 * something else went wrong
 */
int bootmeth_get_state_desc(struct udevice *dev, char *buf, int maxsize);

/**
 * bootmeth_check() - check if a bootmeth supports this bootflow
 *
 * This is optional. If not provided, the bootdev is assumed to be
 * supported
 *
 * The bootmeth can check the bootdev (e.g. to make sure it is a
 * network device) or the partition information. The following fields
 * in @iter are available:
 *
 *   name, dev, state, part
 *   max_part may be set if part != 0 (i.e. there is a valid partition
 *      table). Otherwise max_part is 0
 *   method is available but is the same as @dev
 *   the partition has not yet been read, nor has the filesystem been
 *   checked
 *
 * It may update only the flags in @iter
 *
 * @dev:	Bootmethod device to check against
 * @iter:	On entry, provides bootdev, hwpart, part
 * Return: 0 if OK, -ENOTSUPP if this bootdev is not supported
 */
int bootmeth_check(struct udevice *dev, struct bootflow_iter *iter);

/**
 * bootmeth_read_bootflow() - set up a bootflow for a device
 *
 * @dev:	Bootmethod device to check
 * @bflow:	On entry, provides dev, hwpart, part and method.
 *	Returns updated bootflow if found
 * Return: 0 if OK, -ve on error
 */
int bootmeth_read_bootflow(struct udevice *dev, struct bootflow *bflow);

/**
 * bootmeth_set_bootflow() - set the bootflow for a device
 *
 * This provides a bootflow file to the bootmeth, to see if it is valid.
 * If it is, the bootflow is set up accordingly.
 *
 * @dev:	Bootmethod device to use
 * @bflow:	On entry, provides bootdev.
 *	Returns updated bootflow if found
 * @buf:	Buffer containing the possible bootflow file (must be allocated
 * by caller to @size + 1 bytes)
 * @size:	Size of file
 * Return: 0 if OK, -ve on error
 */
int bootmeth_set_bootflow(struct udevice *dev, struct bootflow *bflow,
			  char *buf, int size);

/**
 * bootmeth_read_file() - read a file needed for a bootflow
 *
 * Read a file from the same place as the bootflow came from
 *
 * @dev:	Bootmethod device to use
 * @bflow:	Bootflow providing info on where to read from
 * @file_path:	Path to file (may be absolute or relative)
 * @addr:	Address to load file
 * @sizep:	On entry provides the maximum permitted size; on exit
 *		returns the size of the file
 * Return: 0 if OK, -ENOSPC if the file is too large for @sizep, other
 *	-ve value if something else goes wrong
 */
int bootmeth_read_file(struct udevice *dev, struct bootflow *bflow,
		       const char *file_path, ulong addr, ulong *sizep);

/**
 * bootmeth_read_all() - read all bootflow files
 *
 * Some bootmeths delay reading of large files until booting is requested. This
 * causes those files to be read.
 *
 * @dev:	Bootmethod device to use
 * @bflow:	Bootflow to read
 * Return: does not return on success, since it should boot the
 *	Operating Systemn. Returns -EFAULT if that fails, other -ve on
 *	other error
 */
int bootmeth_read_all(struct udevice *dev, struct bootflow *bflow);

/**
 * bootmeth_boot() - boot a bootflow
 *
 * @dev:	Bootmethod device to boot
 * @bflow:	Bootflow to boot
 * Return: does not return on success, since it should boot the
 *	Operating Systemn. Returns -EFAULT if that fails, other -ve on
 *	other error
 */
int bootmeth_boot(struct udevice *dev, struct bootflow *bflow);

/**
 * bootmeth_setup_iter_order() - Set up the ordering of bootmeths to scan
 *
 * This sets up the ordering information in @iter, based on the selected
 * ordering of the bootmethds in bootstd_priv->bootmeth_order. If there is no
 * ordering there, then all bootmethods are added
 *
 * @iter: Iterator to update with the order
 * @include_global: true to add the global bootmeths, in which case they appear
 * first
 * Return: 0 if OK, -ENOENT if no bootdevs, -ENOMEM if out of memory, other -ve
 *	on other error
 */
int bootmeth_setup_iter_order(struct bootflow_iter *iter, bool include_global);

/**
 * bootmeth_set_order() - Set the bootmeth order
 *
 * This selects the ordering to use for bootmeths
 *
 * @order_str: String containing the ordering. This is a comma-separate list of
 * bootmeth-device names, e.g. "extlinux,efi". If empty then a default ordering
 * is used, based on the sequence number of devices (i.e. using aliases)
 * Return: 0 if OK, -ENODEV if an unknown bootmeth is mentioned, -ENOMEM if
 * out of memory, -ENOENT if there are no bootmeth devices
 */
int bootmeth_set_order(const char *order_str);

/**
 * bootmeth_setup_fs() - Set up read to read a file
 *
 * We must redo the setup before each filesystem operation. This function
 * handles that, including setting the filesystem type if a block device is not
 * being used
 *
 * @bflow: Information about file to try
 * @desc: Block descriptor to read from (NULL if not a block device)
 * Return: 0 if OK, -ve on error
 */
int bootmeth_setup_fs(struct bootflow *bflow, struct blk_desc *desc);

/**
 * bootmeth_try_file() - See we can access a given file
 *
 * Check for a file with a given name. If found, the filename is allocated in
 * @bflow
 *
 * Sets the state to BOOTFLOWST_FILE on success. It also calls
 * fs_set_blk_dev_with_part() so that this does not need to be done by the
 * caller before reading the file.
 *
 * @bflow: Information about file to try
 * @desc: Block descriptor to read from (NULL for sandbox host)
 * @prefix: Filename prefix to prepend to @fname (NULL for none)
 * @fname: Filename to read
 * Return: 0 if OK, -ENOMEM if not enough memory to allocate bflow->fname,
 * other -ve value on other error
 */
int bootmeth_try_file(struct bootflow *bflow, struct blk_desc *desc,
		      const char *prefix, const char *fname);

/**
 * bootmeth_alloc_file() - Allocate and read a bootflow file
 *
 * Allocates memory for a bootflow file and reads it in. Sets the state to
 * BOOTFLOWST_READY on success
 *
 * Note that fs_set_blk_dev_with_part() must have been called previously.
 *
 * @bflow: Information about file to read
 * @size_limit: Maximum file size to permit
 * @align: Allocation alignment (1 for unaligned)
 * Return: 0 if OK, -E2BIG if file is too large, -ENOMEM if out of memory,
 *	other -ve on other error
 */
int bootmeth_alloc_file(struct bootflow *bflow, uint size_limit, uint align);

/**
 * bootmeth_alloc_other() - Allocate and read a file for a bootflow
 *
 * This reads an arbitrary file in the same directory as the bootflow,
 * allocating memory for it. The buffer is one byte larger than the file length,
 * so that it can be nul-terminated.
 *
 * @bflow: Information about file to read
 * @fname: Filename to read from (within bootflow->subdir)
 * @bufp: Returns a pointer to the allocated buffer
 * @sizep: Returns the size of the buffer
 * Return: 0 if OK,  -ENOMEM if out of memory, other -ve on other error
 */
int bootmeth_alloc_other(struct bootflow *bflow, const char *fname,
			 void **bufp, uint *sizep);

/**
 * bootmeth_common_read_file() - Common handler for reading a file
 *
 * Reads a named file from the same location as the bootflow file.
 *
 * @dev: bootmeth device to read from
 * @bflow: Bootflow information
 * @file_path: Path to file
 * @addr: Address to load file to
 * @sizep: On entry, the maximum file size to accept, on exit the actual file
 *	size read
 */
int bootmeth_common_read_file(struct udevice *dev, struct bootflow *bflow,
			      const char *file_path, ulong addr, ulong *sizep);

/**
 * bootmeth_get_bootflow() - Get a bootflow from a global bootmeth
 *
 * Check the bootmeth for a bootflow which can be used. In this case the
 * bootmeth handles all bootdev selection, etc.
 *
 * @dev: bootmeth device to read from
 * @bflow: Bootflow information
 * @return 0 on success, -ve if a bootflow could not be found or had an error
 */
int bootmeth_get_bootflow(struct udevice *dev, struct bootflow *bflow);

#endif
