/* SPDX-License-Identifier: GPL-2.0 */

#ifndef XENBUS_H__
#define XENBUS_H__

#include <xen/interface/xen.h>
#include <xen/interface/io/xenbus.h>

typedef unsigned long xenbus_transaction_t;
#define XBT_NIL ((xenbus_transaction_t)0)

extern u32 xenbus_evtchn;

/* Initialize the XenBus system. */
void init_xenbus(void);
/* Finalize the XenBus system. */
void fini_xenbus(void);

/**
 * xenbus_read() - Read the value associated with a path.
 *
 * Returns a malloc'd error string on failure and sets *value to NULL.
 * On success, *value is set to a malloc'd copy of the value.
 */
char *xenbus_read(xenbus_transaction_t xbt, const char *path, char **value);

char *xenbus_wait_for_state_change(const char *path, XenbusState *state);
char *xenbus_switch_state(xenbus_transaction_t xbt, const char *path,
			  XenbusState state);

/**
 * xenbus_write() - Associates a value with a path.
 *
 * Returns a malloc'd error string on failure.
 */
char *xenbus_write(xenbus_transaction_t xbt, const char *path,
		   const char *value);

/**
 * xenbus_rm() - Removes the value associated with a path.
 *
 * Returns a malloc'd error string on failure.
 */
char *xenbus_rm(xenbus_transaction_t xbt, const char *path);

/**
 * xenbus_ls() - List the contents of a directory.
 *
 * Returns a malloc'd error string on failure and sets *contents to NULL.
 * On success, *contents is set to a malloc'd array of pointers to malloc'd
 * strings. The array is NULL terminated. May block.
 */
char *xenbus_ls(xenbus_transaction_t xbt, const char *prefix, char ***contents);

/**
 * xenbus_get_perms() - Reads permissions associated with a path.
 *
 * Returns a malloc'd error string on failure and sets *value to NULL.
 * On success, *value is set to a malloc'd copy of the value.
 */
char *xenbus_get_perms(xenbus_transaction_t xbt, const char *path, char **value);

/**
 * xenbus_set_perms() - Sets the permissions associated with a path.
 *
 * Returns a malloc'd error string on failure.
 */
char *xenbus_set_perms(xenbus_transaction_t xbt, const char *path, domid_t dom,
		       char perm);

/**
 * xenbus_transaction_start() - Start a xenbus transaction.
 *
 * Returns the transaction in xbt on success or a malloc'd error string
 * otherwise.
 */
char *xenbus_transaction_start(xenbus_transaction_t *xbt);

/**
 * xenbus_transaction_end() - End a xenbus transaction.
 *
 * Returns a malloc'd error string if it fails. Abort says whether the
 * transaction should be aborted.
 * Returns 1 in *retry if the transaction should be retried.
 */
char *xenbus_transaction_end(xenbus_transaction_t xbt, int abort,
			     int *retry);

/**
 * xenbus_read_integer() - Read path and parse it as an integer.
 *
 * Returns -1 on error.
 */
int xenbus_read_integer(const char *path);

/**
 * xenbus_read_uuid() - Read path and parse it as 16 byte uuid.
 *
 * Returns 1 if read and parsing were successful, 0 if not
 */
int xenbus_read_uuid(const char *path, unsigned char uuid[16]);

/**
 * xenbus_printf() - Contraction of snprintf and xenbus_write(path/node).
 */
char *xenbus_printf(xenbus_transaction_t xbt,
		    const char *node, const char *path,
		    const char *fmt, ...)
	__attribute__((__format__(printf, 4, 5)));

/**
 * xenbus_get_self_id() - Utility function to figure out our domain id
 */
domid_t xenbus_get_self_id(void);

#endif /* XENBUS_H__ */
