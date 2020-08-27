/* Copyright 2007-2010 Jozsef Kadlecsik (kadlec@netfilter.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef LIBIPSET_SESSION_H
#define LIBIPSET_SESSION_H

#include <stdbool.h>				/* bool */
#include <stdint.h>				/* uintxx_t */
#include <stdio.h>				/* printf */

#include <libipset/linux_ip_set.h>		/* enum ipset_cmd */

/* Report and output buffer sizes */
#define IPSET_ERRORBUFLEN		1024
#define IPSET_OUTBUFLEN			8192

struct ipset_session;
struct ipset_data;

#ifdef __cplusplus
extern "C" {
#endif

extern struct ipset_data *
	ipset_session_data(const struct ipset_session *session);
extern struct ipset_handle *
	ipset_session_handle(const struct ipset_session *session);
extern const struct ipset_type *
	ipset_saved_type(const struct ipset_session *session);
extern void ipset_session_lineno(struct ipset_session *session,
				 uint32_t lineno);
extern void * ipset_session_printf_private(struct ipset_session *session);

enum ipset_err_type {
	IPSET_NO_ERROR,
	IPSET_WARNING,		/* Success code when exit */
	IPSET_NOTICE,		/* Error code and exit in non interactive mode */
	IPSET_ERROR,		/* Error code and exit */
};

extern int ipset_session_report(struct ipset_session *session,
				enum ipset_err_type type,
				const char *fmt, ...);
extern int ipset_session_warning_as_error(struct ipset_session *session);

#define ipset_err(session, fmt, args...) \
	ipset_session_report(session, IPSET_ERROR, fmt , ## args)

#define ipset_warn(session, fmt, args...) \
	ipset_session_report(session, IPSET_WARNING, fmt , ## args)

#define ipset_notice(session, fmt, args...) \
	ipset_session_report(session, IPSET_NOTICE, fmt , ## args)

#define ipset_errptr(session, fmt, args...) ({				\
	ipset_session_report(session, IPSET_ERROR, fmt , ## args);	\
	NULL;								\
})

extern void ipset_session_report_reset(struct ipset_session *session);
extern const char *ipset_session_report_msg(const struct ipset_session *session);
extern enum ipset_err_type ipset_session_report_type(
	const struct ipset_session *session);

#define ipset_session_data_set(session, opt, value)	\
	ipset_data_set(ipset_session_data(session), opt, value)
#define ipset_session_data_get(session, opt)		\
	ipset_data_get(ipset_session_data(session), opt)

/* Environment option flags */
enum ipset_envopt {
	IPSET_ENV_BIT_SORTED	= 0,
	IPSET_ENV_SORTED	= (1 << IPSET_ENV_BIT_SORTED),
	IPSET_ENV_BIT_QUIET	= 1,
	IPSET_ENV_QUIET		= (1 << IPSET_ENV_BIT_QUIET),
	IPSET_ENV_BIT_RESOLVE	= 2,
	IPSET_ENV_RESOLVE	= (1 << IPSET_ENV_BIT_RESOLVE),
	IPSET_ENV_BIT_EXIST	= 3,
	IPSET_ENV_EXIST		= (1 << IPSET_ENV_BIT_EXIST),
	IPSET_ENV_BIT_LIST_SETNAME = 4,
	IPSET_ENV_LIST_SETNAME	= (1 << IPSET_ENV_BIT_LIST_SETNAME),
	IPSET_ENV_BIT_LIST_HEADER = 5,
	IPSET_ENV_LIST_HEADER	= (1 << IPSET_ENV_BIT_LIST_HEADER),
};

extern bool ipset_envopt_test(struct ipset_session *session,
			      enum ipset_envopt env);
extern void ipset_envopt_set(struct ipset_session *session,
			     enum ipset_envopt env);
extern void ipset_envopt_unset(struct ipset_session *session,
			       enum ipset_envopt env);

enum ipset_output_mode {
	IPSET_LIST_NONE,
	IPSET_LIST_PLAIN,
	IPSET_LIST_SAVE,
	IPSET_LIST_XML,
};

extern int ipset_session_output(struct ipset_session *session,
				enum ipset_output_mode mode);

extern int ipset_commit(struct ipset_session *session);
extern int ipset_cmd(struct ipset_session *session, enum ipset_cmd cmd,
		     uint32_t lineno);

typedef int (*ipset_print_outfn)(struct ipset_session *session,
	void *p, const char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));

extern int ipset_session_print_outfn(struct ipset_session *session,
				     ipset_print_outfn outfn,
				     void *p);

enum ipset_io_type {
	IPSET_IO_INPUT,
	IPSET_IO_OUTPUT,
};

extern int ipset_session_io_full(struct ipset_session *session,
		const char *filename, enum ipset_io_type what);
extern int ipset_session_io_normal(struct ipset_session *session,
		const char *filename, enum ipset_io_type what);
extern FILE * ipset_session_io_stream(struct ipset_session *session,
				      enum ipset_io_type what);
extern int ipset_session_io_close(struct ipset_session *session,
				  enum ipset_io_type what);

extern struct ipset_session *ipset_session_init(ipset_print_outfn outfn,
						void *p);
extern int ipset_session_fini(struct ipset_session *session);

extern void ipset_debug_msg(const char *dir, void *buffer, int len);

#ifdef __cplusplus
}
#endif

#endif /* LIBIPSET_SESSION_H */
