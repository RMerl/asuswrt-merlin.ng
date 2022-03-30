/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifndef __ENV_FLAGS_H__
#define __ENV_FLAGS_H__

enum env_flags_vartype {
	env_flags_vartype_string,
	env_flags_vartype_decimal,
	env_flags_vartype_hex,
	env_flags_vartype_bool,
#ifdef CONFIG_CMD_NET
	env_flags_vartype_ipaddr,
	env_flags_vartype_macaddr,
#endif
	env_flags_vartype_end
};

enum env_flags_varaccess {
	env_flags_varaccess_any,
	env_flags_varaccess_readonly,
	env_flags_varaccess_writeonce,
	env_flags_varaccess_changedefault,
	env_flags_varaccess_end
};

#define ENV_FLAGS_VAR ".flags"
#define ENV_FLAGS_ATTR_MAX_LEN 2
#define ENV_FLAGS_VARTYPE_LOC 0
#define ENV_FLAGS_VARACCESS_LOC 1

#ifndef CONFIG_ENV_FLAGS_LIST_STATIC
#define CONFIG_ENV_FLAGS_LIST_STATIC ""
#endif

#ifdef CONFIG_CMD_NET
#ifdef CONFIG_REGEX
#define ETHADDR_WILDCARD "\\d*"
#else
#define ETHADDR_WILDCARD
#endif
#ifdef CONFIG_ENV_OVERWRITE
#define ETHADDR_FLAGS "eth" ETHADDR_WILDCARD "addr:ma,"
#else
#ifdef CONFIG_OVERWRITE_ETHADDR_ONCE
#define ETHADDR_FLAGS "eth" ETHADDR_WILDCARD "addr:mc,"
#else
#define ETHADDR_FLAGS "eth" ETHADDR_WILDCARD "addr:mo,"
#endif
#endif
#define NET_FLAGS \
	"ipaddr:i," \
	"gatewayip:i," \
	"netmask:i," \
	"serverip:i," \
	"nvlan:d," \
	"vlan:d," \
	"dnsip:i,"
#else
#define ETHADDR_FLAGS
#define NET_FLAGS
#endif

#ifndef CONFIG_ENV_OVERWRITE
#define SERIAL_FLAGS "serial#:so,"
#else
#define SERIAL_FLAGS ""
#endif

#define ENV_FLAGS_LIST_STATIC \
	ETHADDR_FLAGS \
	NET_FLAGS \
	SERIAL_FLAGS \
	CONFIG_ENV_FLAGS_LIST_STATIC

#ifdef CONFIG_CMD_ENV_FLAGS
/*
 * Print the whole list of available type flags.
 */
void env_flags_print_vartypes(void);
/*
 * Print the whole list of available access flags.
 */
void env_flags_print_varaccess(void);
/*
 * Return the name of the type.
 */
const char *env_flags_get_vartype_name(enum env_flags_vartype type);
/*
 * Return the name of the access.
 */
const char *env_flags_get_varaccess_name(enum env_flags_varaccess access);
#endif

/*
 * Parse the flags string from a .flags attribute list into the vartype enum.
 */
enum env_flags_vartype env_flags_parse_vartype(const char *flags);
/*
 * Parse the flags string from a .flags attribute list into the varaccess enum.
 */
enum env_flags_varaccess env_flags_parse_varaccess(const char *flags);
/*
 * Parse the binary flags from a hash table entry into the varaccess enum.
 */
enum env_flags_varaccess env_flags_parse_varaccess_from_binflags(int binflags);

#ifdef CONFIG_CMD_NET
/*
 * Check if a string has the format of an Ethernet MAC address
 */
int eth_validate_ethaddr_str(const char *addr);
#endif

#ifdef USE_HOSTCC
/*
 * Look up the type of a variable directly from the .flags var.
 */
enum env_flags_vartype env_flags_get_type(const char *name);
/*
 * Look up the access of a variable directly from the .flags var.
 */
enum env_flags_varaccess env_flags_get_access(const char *name);
/*
 * Validate the newval for its type to conform with the requirements defined by
 * its flags (directly looked at the .flags var).
 */
int env_flags_validate_type(const char *name, const char *newval);
/*
 * Validate the newval for its access to conform with the requirements defined
 * by its flags (directly looked at the .flags var).
 */
int env_flags_validate_access(const char *name, int check_mask);
/*
 * Validate that the proposed access to variable "name" is valid according to
 * the defined flags for that variable, if any.
 */
int env_flags_validate_varaccess(const char *name, int check_mask);
/*
 * Validate the parameters passed to "env set" for type compliance
 */
int env_flags_validate_env_set_params(char *name, char *const val[], int count);

#else /* !USE_HOSTCC */

#include <search.h>

/*
 * When adding a variable to the environment, initialize the flags for that
 * variable.
 */
void env_flags_init(ENTRY *var_entry);

/*
 * Validate the newval for to conform with the requirements defined by its flags
 */
int env_flags_validate(const ENTRY *item, const char *newval, enum env_op op,
	int flag);

#endif /* USE_HOSTCC */

/*
 * These are the binary flags used in the environment entry->flags variable to
 * decribe properties of veriables in the table
 */
#define ENV_FLAGS_VARTYPE_BIN_MASK			0x00000007
/* The actual variable type values use the enum value (within the mask) */
#define ENV_FLAGS_VARACCESS_PREVENT_DELETE		0x00000008
#define ENV_FLAGS_VARACCESS_PREVENT_CREATE		0x00000010
#define ENV_FLAGS_VARACCESS_PREVENT_OVERWR		0x00000020
#define ENV_FLAGS_VARACCESS_PREVENT_NONDEF_OVERWR	0x00000040
#define ENV_FLAGS_VARACCESS_BIN_MASK			0x00000078

#endif /* __ENV_FLAGS_H__ */
