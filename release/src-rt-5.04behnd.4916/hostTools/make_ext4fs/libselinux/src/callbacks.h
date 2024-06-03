/*
 * This file describes the callbacks passed to selinux_init() and available
 * for use from the library code.  They all have default implementations.
 */
#ifndef _SELINUX_CALLBACKS_H_
#define _SELINUX_CALLBACKS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <selinux/selinux.h>
#include "dso.h"

/* callback pointers */
extern int __attribute__ ((format(printf, 2, 3)))
(*selinux_log) (int type, const char *, ...) hidden;

extern int
(*selinux_audit) (void *, security_class_t, char *, size_t) hidden;

extern int
(*selinux_validate)(security_context_t *ctx) hidden;

extern int
(*selinux_netlink_setenforce) (int enforcing) hidden;

extern int
(*selinux_netlink_policyload) (int seqno) hidden;

#endif				/* _SELINUX_CALLBACKS_H_ */
