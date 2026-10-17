/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __PXE_UTILS_H
#define __PXE_UTILS_H

#include <linux/list.h>

/*
 * A note on the pxe file parser.
 *
 * We're parsing files that use syslinux grammar, which has a few quirks.
 * String literals must be recognized based on context - there is no
 * quoting or escaping support. There's also nothing to explicitly indicate
 * when a label section completes. We deal with that by ending a label
 * section whenever we see a line that doesn't include.
 *
 * As with the syslinux family, this same file format could be reused in the
 * future for non pxe purposes. The only action it takes during parsing that
 * would throw this off is handling of include files. It assumes we're using
 * pxe, and does a tftp download of a file listed as an include file in the
 * middle of the parsing operation. That could be handled by refactoring it to
 * take a 'include file getter' function.
 */

/*
 * Describes a single label given in a pxe file.
 *
 * Create these with the 'label_create' function given below.
 *
 * name - the name of the menu as given on the 'menu label' line.
 * kernel_label - the kernel label, including FIT config if present.
 * kernel - the path to the kernel file to use for this label.
 * append - kernel command line to use when booting this label
 * initrd - path to the initrd to use for this label.
 * attempted - 0 if we haven't tried to boot this label, 1 if we have.
 * localboot - 1 if this label specified 'localboot', 0 otherwise.
 * kaslrseed - 1 if generate kaslrseed from hw_rng
 * list - lets these form a list, which a pxe_menu struct will hold.
 */
struct pxe_label {
	char num[4];
	char *name;
	char *menu;
	char *kernel_label;
	char *kernel;
	char *config;
	char *append;
	char *initrd;
	char *fdt;
	char *fdtdir;
	char *fdtoverlays;
	int ipappend;
	int attempted;
	int localboot;
	int localboot_val;
	int kaslrseed;
	struct list_head list;
};

/*
 * Describes a pxe menu as given via pxe files.
 *
 * title - the name of the menu as given by a 'menu title' line.
 * default_label - the name of the default label, if any.
 * bmp - the bmp file name which is displayed in background
 * timeout - time in tenths of a second to wait for a user key-press before
 *           booting the default label.
 * prompt - if 0, don't prompt for a choice unless the timeout period is
 *          interrupted.  If 1, always prompt for a choice regardless of
 *          timeout.
 * labels - a list of labels defined for the menu.
 */
struct pxe_menu {
	char *title;
	char *default_label;
	char *bmp;
	int timeout;
	int prompt;
	struct list_head labels;
};

struct pxe_context;
typedef int (*pxe_getfile_func)(struct pxe_context *ctx, const char *file_path,
				char *file_addr, ulong *filesizep);

/**
 * struct pxe_context - context information for PXE parsing
 *
 * @cmdtp: Pointer to command table to use when calling other commands
 * @getfile: Function called by PXE to read a file
 * @userdata: Data the caller requires for @getfile
 * @allow_abs_path: true to allow absolute paths
 * @bootdir: Directory that files are loaded from ("" if no directory). This is
 *	allocated
 * @pxe_file_size: Size of the PXE file
 * @use_ipv6: TRUE : use IPv6 addressing, FALSE : use IPv4 addressing
 */
struct pxe_context {
	struct cmd_tbl *cmdtp;
	/**
	 * getfile() - read a file
	 *
	 * @ctx: PXE context
	 * @file_path: Path to the file
	 * @file_addr: String containing the hex address to put the file in
	 *	memory
	 * @filesizep: Returns the file size in bytes
	 * Return 0 if OK, -ve on error
	 */
	pxe_getfile_func getfile;

	void *userdata;
	bool allow_abs_path;
	char *bootdir;
	ulong pxe_file_size;
	bool use_ipv6;
};

/**
 * destroy_pxe_menu() - Destroy an allocated pxe structure
 *
 * Free the memory used by a pxe_menu and its labels
 *
 * @cfg: Config to destroy, previous returned from parse_pxefile()
 */
void destroy_pxe_menu(struct pxe_menu *cfg);

/**
 * get_pxe_file() - Read a file
 *
 * Retrieve the file at 'file_path' to the locate given by 'file_addr'. If
 * 'bootfile' was specified in the environment, the path to bootfile will be
 * prepended to 'file_path' and the resulting path will be used.
 *
 * @ctx: PXE context
 * @file_path: Path to file
 * @file_addr: Address to place file
 * Returns 1 on success, or < 0 for error
 */
int get_pxe_file(struct pxe_context *ctx, const char *file_path,
		 ulong file_addr);

/**
 * get_pxelinux_path() - Read a file from the same place as pxelinux.cfg
 *
 * Retrieves a file in the 'pxelinux.cfg' folder. Since this uses get_pxe_file()
 * to do the hard work, the location of the 'pxelinux.cfg' folder is generated
 * from the bootfile path, as described in get_pxe_file().
 *
 * @ctx: PXE context
 * @file: Relative path to file
 * @pxefile_addr_r: Address to load file
 * Returns 1 on success or < 0 on error.
 */
int get_pxelinux_path(struct pxe_context *ctx, const char *file,
		      ulong pxefile_addr_r);

/**
 * handle_pxe_menu() - Boot the system as prescribed by a pxe_menu.
 *
 * Use the menu system to either get the user's choice or the default, based
 * on config or user input.  If there is no default or user's choice,
 * attempted to boot labels in the order they were given in pxe files.
 * If the default or user's choice fails to boot, attempt to boot other
 * labels in the order they were given in pxe files.
 *
 * If this function returns, there weren't any labels that successfully
 * booted, or the user interrupted the menu selection via ctrl+c.
 *
 * @ctx: PXE context
 * @cfg: PXE menu
 */
void handle_pxe_menu(struct pxe_context *ctx, struct pxe_menu *cfg);

/**
 * parse_pxefile() - Parsing a pxe file
 *
 * This is only used for the top-level file.
 *
 * @ctx: PXE context (provided by the caller)
 * Returns NULL if there is an error, otherwise, returns a pointer to a
 * pxe_menu struct populated with the results of parsing the pxe file (and any
 * files it includes). The resulting pxe_menu struct can be free()'d by using
 * the destroy_pxe_menu() function.
 */
struct pxe_menu *parse_pxefile(struct pxe_context *ctx, ulong menucfg);

/**
 * format_mac_pxe() - Convert a MAC address to PXE format
 *
 * Convert an ethaddr from the environment to the format used by pxelinux
 * filenames based on mac addresses. Convert's ':' to '-', and adds "01-" to
 * the beginning of the ethernet address to indicate a hardware type of
 * Ethernet. Also converts uppercase hex characters into lowercase, to match
 * pxelinux's behavior.
 *
 * @outbuf: Buffer to hold the output (must hold 22 bytes)
 * @outbuf_len: Length of buffer
 * Returns 1 for success, -ENOENT if 'ethaddr' is undefined in the
 * environment, or some other value < 0 on error.
 */
int format_mac_pxe(char *outbuf, size_t outbuf_len);

/**
 * pxe_setup_ctx() - Setup a new PXE context
 *
 * @ctx: Context to set up
 * @cmdtp: Command table entry which started this action
 * @getfile: Function to call to read a file
 * @userdata: Data the caller requires for @getfile - stored in ctx->userdata
 * @allow_abs_path: true to allow absolute paths
 * @bootfile: Bootfile whose directory loaded files are relative to, NULL if
 *	none
 * @use_ipv6: TRUE : use IPv6 addressing
 *            FALSE : use IPv4 addressing
 * Return: 0 if OK, -ENOMEM if out of memory, -E2BIG if bootfile is larger than
 *	MAX_TFTP_PATH_LEN bytes
 */
int pxe_setup_ctx(struct pxe_context *ctx, struct cmd_tbl *cmdtp,
		  pxe_getfile_func getfile, void *userdata,
		  bool allow_abs_path, const char *bootfile, bool use_ipv6);

/**
 * pxe_destroy_ctx() - Destroy a PXE context
 *
 * @ctx: Context to destroy
 */
void pxe_destroy_ctx(struct pxe_context *ctx);

/**
 * pxe_process() - Process a PXE file through to boot
 *
 * @ctx: PXE context created with pxe_setup_ctx()
 * @pxefile_addr_r: Address to load file
 * @prompt: Force a prompt for the user
 */
int pxe_process(struct pxe_context *ctx, ulong pxefile_addr_r, bool prompt);

/**
 * pxe_get_file_size() - Read the value of the 'filesize' environment variable
 *
 * @sizep: Place to put the value
 * Return: 0 if OK, -ENOENT if no such variable, -EINVAL if format is invalid
 */
int pxe_get_file_size(ulong *sizep);

/**
 * pxe_get() - Get the PXE file from the server
 *
 * This tries various filenames to obtain a PXE file
 *
 * @pxefile_addr_r: Address to put file
 * @bootdirp: Returns the boot filename, or NULL if none. This is the 'bootfile'
 *	option provided by the DHCP server. If none, returns NULL. For example,
 *	"rpi/info", which indicates that all files should be fetched from the
 *	"rpi/" subdirectory
 * @sizep: Size of the PXE file (not bootfile)
 * @use_ipv6: TRUE : use IPv6 addressing
 *            FALSE : use IPv4 addressing
 */
int pxe_get(ulong pxefile_addr_r, char **bootdirp, ulong *sizep, bool use_ipv6);

#endif /* __PXE_UTILS_H */
