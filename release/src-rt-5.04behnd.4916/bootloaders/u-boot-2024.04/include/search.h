/* SPDX-License-Identifier: LGPL-2.1+ */
/*
 * Declarations for System V style searching functions.
 * Copyright (C) 1995-1999, 2000 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 */

/*
 * Based on code from uClibc-0.9.30.3
 * Extensions for use within U-Boot
 * Copyright (C) 2010-2013 Wolfgang Denk <wd@denx.de>
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <env.h>
#include <stddef.h>

#define set_errno(val) do { errno = val; } while (0)

/* enum env_action: action which shall be performed in the call to hsearch */
enum env_action {
	ENV_FIND,
	ENV_ENTER,
};

/** struct env_entry - An entry in the environment hashtable */
struct env_entry {
	const char *key;
	char *data;
#ifndef CONFIG_SPL_BUILD
	int (*callback)(const char *name, const char *value, enum env_op op,
		int flags);
#endif
	int flags;
};

/*
 * Family of hash table handling functions.  The functions also
 * have reentrant counterparts ending with _r.  The non-reentrant
 * functions all work on a single internal hash table.
 */

/* Data type for reentrant functions.  */
struct hsearch_data {
	struct env_entry_node *table;
	unsigned int size;
	unsigned int filled;
/*
 * Callback function which will check whether the given change for variable
 * "item" to "newval" may be applied or not, and possibly apply such change.
 * When (flag & H_FORCE) is set, it shall not print out any error message and
 * shall force overwriting of write-once variables.
 * Must return 0 for approval, 1 for denial.
 */
	int (*change_ok)(const struct env_entry *item, const char *newval,
			 enum env_op, int flag);
};

/* Create a new hash table which will contain at most "nel" elements.  */
int hcreate_r(size_t nel, struct hsearch_data *htab);

/* Destroy current internal hash table.  */
void hdestroy_r(struct hsearch_data *htab);

/*
 * Search for entry matching item.key in internal hash table.  If
 * action is `ENV_FIND' return found entry or signal error by returning
 * NULL.  If action is `ENV_ENTER' replace existing data (if any) with
 * item.data.
 * */
int hsearch_r(struct env_entry item, enum env_action action,
	      struct env_entry **retval, struct hsearch_data *htab, int flag);

/*
 * Search for an entry matching "match".  Otherwise, Same semantics
 * as hsearch_r().
 */
int hmatch_r(const char *match, int last_idx, struct env_entry **retval,
	     struct hsearch_data *htab);

/**
 * hdelete_r() - Search and delete entry in internal hash table
 *
 * @key: Name of entry to delete
 * @htab: Hash table
 * @flag: Flags to use (H_...)
 * Return: 0 on success, -ENOENT if not found, -EPERM if the hash table callback
 *	rejected changing the variable, -EINVAL if the hash table refused to
 *	delete the variable
 */
int hdelete_r(const char *key, struct hsearch_data *htab, int flag);

ssize_t hexport_r(struct hsearch_data *htab, const char sep, int flag,
		  char **resp, size_t size, int argc, char *const argv[]);

/*
 * nvars: length of vars array
 * vars: array of strings (variable names) to import (nvars == 0 means all)
 */
int himport_r(struct hsearch_data *htab, const char *env, size_t size,
	      const char sep, int flag, int crlf_is_lf, int nvars,
	      char * const vars[]);

/* Walk the whole table calling the callback on each element */
int hwalk_r(struct hsearch_data *htab,
	    int (*callback)(struct env_entry *entry));

/* Flags for himport_r(), hexport_r(), hdelete_r(), and hsearch_r() */
#define H_NOCLEAR	(1 << 0) /* do not clear hash table before importing */
#define H_FORCE		(1 << 1) /* overwrite read-only/write-once variables */
#define H_INTERACTIVE	(1 << 2) /* indicate that an import is user directed */
#define H_HIDE_DOT	(1 << 3) /* don't print env vars that begin with '.' */
#define H_MATCH_KEY	(1 << 4) /* search/grep key  = variable names	     */
#define H_MATCH_DATA	(1 << 5) /* search/grep data = variable values	     */
#define H_MATCH_BOTH	(H_MATCH_KEY | H_MATCH_DATA) /* search/grep both     */
#define H_MATCH_IDENT	(1 << 6) /* search for indentical strings	     */
#define H_MATCH_SUBSTR	(1 << 7) /* search for substring matches	     */
#define H_MATCH_REGEX	(1 << 8) /* search for regular expression matches    */
#define H_MATCH_METHOD	(H_MATCH_IDENT | H_MATCH_SUBSTR | H_MATCH_REGEX)
#define H_PROGRAMMATIC	(1 << 9) /* indicate that an import is from env_set() */
#define H_ORIGIN_FLAGS	(H_INTERACTIVE | H_PROGRAMMATIC)
#define H_DEFAULT	(1 << 10) /* indicate that an import is default env */
#define H_EXTERNAL	(1 << 11) /* indicate that an import is external env */

#endif /* _SEARCH_H_ */
