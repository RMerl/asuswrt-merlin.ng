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

#include <stddef.h>

#define __set_errno(val) do { errno = val; } while (0)

enum env_op {
	env_op_create,
	env_op_delete,
	env_op_overwrite,
};

/* Action which shall be performed in the call to hsearch.  */
typedef enum {
	FIND,
	ENTER
} ACTION;

typedef struct entry {
	const char *key;
	char *data;
	int (*callback)(const char *name, const char *value, enum env_op op,
		int flags);
	int flags;
} ENTRY;

/* Opaque type for internal use.  */
struct _ENTRY;

/*
 * Family of hash table handling functions.  The functions also
 * have reentrant counterparts ending with _r.  The non-reentrant
 * functions all work on a single internal hash table.
 */

/* Data type for reentrant functions.  */
struct hsearch_data {
	struct _ENTRY *table;
	unsigned int size;
	unsigned int filled;
/*
 * Callback function which will check whether the given change for variable
 * "__item" to "newval" may be applied or not, and possibly apply such change.
 * When (flag & H_FORCE) is set, it shall not print out any error message and
 * shall force overwriting of write-once variables.
 * Must return 0 for approval, 1 for denial.
 */
	int (*change_ok)(const ENTRY *__item, const char *newval, enum env_op,
		int flag);
};

/* Create a new hash table which will contain at most "__nel" elements.  */
extern int hcreate_r(size_t __nel, struct hsearch_data *__htab);

/* Destroy current internal hash table.  */
extern void hdestroy_r(struct hsearch_data *__htab);

/*
 * Search for entry matching __item.key in internal hash table.  If
 * ACTION is `FIND' return found entry or signal error by returning
 * NULL.  If ACTION is `ENTER' replace existing data (if any) with
 * __item.data.
 * */
extern int hsearch_r(ENTRY __item, ACTION __action, ENTRY ** __retval,
		     struct hsearch_data *__htab, int __flag);

/*
 * Search for an entry matching "__match".  Otherwise, Same semantics
 * as hsearch_r().
 */
extern int hmatch_r(const char *__match, int __last_idx, ENTRY ** __retval,
		    struct hsearch_data *__htab);

/* Search and delete entry matching "__key" in internal hash table. */
extern int hdelete_r(const char *__key, struct hsearch_data *__htab,
		     int __flag);

extern ssize_t hexport_r(struct hsearch_data *__htab,
		     const char __sep, int __flag, char **__resp, size_t __size,
		     int argc, char * const argv[]);

/*
 * nvars: length of vars array
 * vars: array of strings (variable names) to import (nvars == 0 means all)
 */
extern int himport_r(struct hsearch_data *__htab,
		     const char *__env, size_t __size, const char __sep,
		     int __flag, int __crlf_is_lf, int nvars,
		     char * const vars[]);

/* Walk the whole table calling the callback on each element */
extern int hwalk_r(struct hsearch_data *__htab, int (*callback)(ENTRY *));

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

#endif /* _SEARCH_H_ */
