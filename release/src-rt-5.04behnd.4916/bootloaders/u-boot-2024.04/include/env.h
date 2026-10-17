/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common environment functions and definitions
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __ENV_H
#define __ENV_H

#include <compiler.h>
#include <stdbool.h>
#include <linux/types.h>

struct environment_s;

/* Value for environment validity */
enum env_valid {
	ENV_INVALID,	/* No valid environment */
	ENV_VALID,	/* First or only environment is valid */
	ENV_REDUND,	/* Redundant environment is valid */
};

/** enum env_op - environment callback operation */
enum env_op {
	env_op_create,
	env_op_delete,
	env_op_overwrite,
};

/** struct env_clbk_tbl - declares a new callback */
struct env_clbk_tbl {
	const char *name;		/* Callback name */
	int (*callback)(const char *name, const char *value, enum env_op op,
			int flags);
};

/*
 * Define a callback that can be associated with variables.
 * when associated through the ".callbacks" environment variable, the callback
 * will be executed any time the variable is inserted, overwritten, or deleted.
 *
 * For SPL these are silently dropped to reduce code size, since environment
 * callbacks are not supported with SPL.
 */
#ifdef CONFIG_SPL_BUILD
#define U_BOOT_ENV_CALLBACK(name, callback) \
	static inline __maybe_unused void _u_boot_env_noop_##name(void) \
	{ \
		(void)callback; \
	}
#else
#define U_BOOT_ENV_CALLBACK(name, callback) \
	ll_entry_declare(struct env_clbk_tbl, name, env_clbk) = \
	{#name, callback}
#endif

/** enum env_redund_flags - Flags for the redundand_environment */
enum env_redund_flags {
	ENV_REDUND_OBSOLETE = 0,
	ENV_REDUND_ACTIVE = 1,
};

/**
 * env_get_id() - Gets a sequence number for the environment
 *
 * This value increments every time the environment changes, so can be used an
 * an indication of this
 *
 * Return: environment ID
 */
int env_get_id(void);

/**
 * env_inc_id() - Increase the sequence number for the environment
 *
 * Increment the value that is used by env_get_id() to inform callers
 * if the environment has changed since they last checked.
 */
void env_inc_id(void);

/**
 * env_init() - Set up the pre-relocation environment
 *
 * This locates the environment or uses the default if nothing is available.
 * This must be called before env_get() will work.
 *
 * Return: 0 if OK, -ENODEV if no environment drivers are enabled
 */
int env_init(void);

/**
 * env_relocate() - Set up the post-relocation environment
 *
 * This loads the environment into RAM so that it can be modified. This is
 * called after relocation, before the environment is used
 */
void env_relocate(void);

/**
 * env_get() - Look up the value of an environment variable
 *
 * In U-Boot proper this can be called before relocation (which is when the
 * environment is loaded from storage, i.e. GD_FLG_ENV_READY is 0). In that
 * case this function calls env_get_f().
 *
 * @varname:	Variable to look up
 * Return: value of variable, or NULL if not found
 */
char *env_get(const char *varname);

/*
 * Like env_get, but prints an error if envvar isn't defined in the
 * environment.  It always returns what env_get does, so it can be used in
 * place of env_get without changing error handling otherwise.
 *
 * @varname:	Variable to look up
 * Return: value of variable, or NULL if not found
 */
char *from_env(const char *envvar);

/**
 * env_get_f() - Look up the value of an environment variable (early)
 *
 * This function is called from env_get() if the environment has not been
 * loaded yet (GD_FLG_ENV_READY flag is 0). Some environment locations will
 * support reading the value (slowly) and some will not.
 *
 * @varname:	Variable to look up
 * Return: actual length of the variable value excluding the terminating
 *	NULL-byte, or -1 if the variable is not found
 */
int env_get_f(const char *name, char *buf, unsigned int len);

/**
 * env_get_yesno() - Read an environment variable as a boolean
 *
 * Return: 1 if yes/true (Y/y/T/t), -1 if variable does not exist (i.e. default
 *	to true), 0 if otherwise
 */
int env_get_yesno(const char *var);

/**
 * env_get_autostart() - Check if autostart is enabled
 *
 * Return: true if the "autostart" env var exists and is set to "yes"
 */
bool env_get_autostart(void);

/**
 * env_set() - set an environment variable
 *
 * This sets or deletes the value of an environment variable. For setting the
 * value the variable is created if it does not already exist.
 *
 * @varname: Variable to adjust
 * @value: Value to set for the variable, or NULL or "" to delete the variable
 * Return: 0 if OK, 1 on error
 */
int env_set(const char *varname, const char *value);

/**
 * env_get_ulong() - Return an environment variable as an integer value
 *
 * Most U-Boot environment variables store hex values. For those which store
 * (e.g.) base-10 integers, this function can be used to read the value.
 *
 * @name:	Variable to look up
 * @base:	Base to use (e.g. 10 for base 10, 2 for binary)
 * @default_val: Default value to return if no value is found
 * Return: the value found, or @default_val if none
 */
ulong env_get_ulong(const char *name, int base, ulong default_val);

/**
 * env_set_ulong() - set an environment variable to an integer
 *
 * @varname: Variable to adjust
 * @value: Value to set for the variable (will be converted to a string)
 * Return: 0 if OK, 1 on error
 */
int env_set_ulong(const char *varname, ulong value);

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

/**
 * env_set_hex() - set an environment variable to a hex value
 *
 * @varname: Variable to adjust
 * @value: Value to set for the variable (will be converted to a hex string)
 * Return: 0 if OK, 1 on error
 */
int env_set_hex(const char *varname, ulong value);

/**
 * env_set_addr - Set an environment variable to an address in hex
 *
 * @varname:	Environment variable to set
 * @addr:	Value to set it to
 * Return: 0 if ok, 1 on error
 */
static inline int env_set_addr(const char *varname, const void *addr)
{
	return env_set_hex(varname, (ulong)addr);
}

/**
 * env_complete() - return an auto-complete for environment variables
 *
 * @var: partial name to auto-complete
 * @maxv: Maximum number of matches to return
 * @cmdv: Returns a list of possible matches
 * @maxsz: Size of buffer to use for matches
 * @buf: Buffer to use for matches
 * @dollar_comp: non-zero to wrap each match in ${...}
 * Return: number of matches found (in @cmdv)
 */
int env_complete(char *var, int maxv, char *cmdv[], int maxsz, char *buf,
		 bool dollar_comp);

/**
 * eth_env_get_enetaddr() - Get an ethernet address from the environmnet
 *
 * @name: Environment variable to get (e.g. "ethaddr")
 * @enetaddr: Place to put MAC address (6 bytes)
 * Return: 1 if OK, 0 on error
 */
int eth_env_get_enetaddr(const char *name, uint8_t *enetaddr);

/**
 * eth_env_set_enetaddr() - Set an ethernet address in the environmnet
 *
 * @name: Environment variable to set (e.g. "ethaddr")
 * @enetaddr: Pointer to MAC address to put into the variable (6 bytes)
 * Return: 0 if OK, non-zero otherwise
 */
int eth_env_set_enetaddr(const char *name, const uint8_t *enetaddr);

/**
 * env_set_default_vars() - reset variables to their default value
 *
 * This resets individual variables to their value in the default environment
 *
 * @nvars: Number of variables to set/reset
 * @vars: List of variables to set/reset
 * @flags: Flags controlling matching (H_... - see search.h)
 */
int env_set_default_vars(int nvars, char *const vars[], int flags);

/**
 * env_load() - Load the environment from storage
 *
 * Return: 0 if OK, -ve on error
 */
int env_load(void);

/**
 * env_reload() - Re-Load the environment from current storage
 *
 * Return: 0 if OK, -ve on error
 */
int env_reload(void);

/**
 * env_save() - Save the environment to storage
 *
 * Return: 0 if OK, -ve on error
 */
int env_save(void);

/**
 * env_erase() - Erase the environment on storage
 *
 * Return: 0 if OK, -ve on error
 */
int env_erase(void);

/**
 * env_select() - Select the environment storage
 *
 * Return: 0 if OK, -ve on error
 */
int env_select(const char *name);

/**
 * env_import() - Import from a binary representation into hash table
 *
 * This imports the environment from a buffer. The format for each variable is
 * var=value\0 with a double \0 at the end of the buffer.
 *
 * @buf: Buffer containing the environment (struct environemnt_s *)
 * @check: non-zero to check the CRC at the start of the environment, 0 to
 *	ignore it
 * @flags: Flags controlling matching (H_... - see search.h)
 * Return: 0 if imported successfully, -ENOMSG if the CRC was bad, -EIO if
 *	something else went wrong
 */
int env_import(const char *buf, int check, int flags);

/**
 * env_export() - Export the environment to a buffer
 *
 * Export from hash table into binary representation
 *
 * @env_out: Buffer to contain the environment (must be large enough!)
 * Return: 0 if OK, 1 on error
 */
int env_export(struct environment_s *env_out);

/**
 * env_check_redund() - check the two redundant environments
 *   and find out, which is the valid one.
 *
 * @buf1: First environment (struct environemnt_s *)
 * @buf1_read_fail: 0 if buf1 is valid, non-zero if invalid
 * @buf2: Second environment (struct environemnt_s *)
 * @buf2_read_fail: 0 if buf2 is valid, non-zero if invalid
 * Return: 0 if OK,
 *	-EIO if no environment is valid,
 *	-ENOMSG if the CRC was bad
 */

int env_check_redund(const char *buf1, int buf1_read_fail,
		     const char *buf2, int buf2_read_fail);

/**
 * env_import_redund() - Select and import one of two redundant environments
 *
 * @buf1: First environment (struct environemnt_s *)
 * @buf1_read_fail: 0 if buf1 is valid, non-zero if invalid
 * @buf2: Second environment (struct environemnt_s *)
 * @buf2_read_fail: 0 if buf2 is valid, non-zero if invalid
 * @flags: Flags controlling matching (H_... - see search.h)
 * Return: 0 if OK, -EIO if no environment is valid, -ENOMSG if the CRC was bad
 */
int env_import_redund(const char *buf1, int buf1_read_fail,
		      const char *buf2, int buf2_read_fail,
		      int flags);

/**
 * env_get_default() - Look up a variable from the default environment
 *
 * @name: Variable to look up
 * Return: value if found, NULL if not found in default environment
 */
char *env_get_default(const char *name);

/**
 * env_get_default_into() - Look up a variable from the default environment and
 * copy its value in buf.
 *
 * @name: Variable to look up
 * Return: actual length of the variable value excluding the terminating
 *	NULL-byte, or -1 if the variable is not found
 */
int env_get_default_into(const char *name, char *buf, unsigned int len);

/* [re]set to the default environment */
void env_set_default(const char *s, int flags);

/**
 * env_import_fdt() - Import environment values from device tree blob
 *
 * This uses the value of the environment variable "env_fdt_path" as a
 * path to an fdt node, whose property/value pairs are added to the
 * environment.
 */
#ifdef CONFIG_ENV_IMPORT_FDT
void env_import_fdt(void);
#else
static inline void env_import_fdt(void) {}
#endif

#endif
