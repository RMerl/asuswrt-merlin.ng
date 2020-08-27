/* Copyright 2007-2010 Jozsef Kadlecsik (kadlec@netfilter.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef LIBIPSET_IPSET_H
#define LIBIPSET_IPSET_H

#include <stdbool.h>				/* bool */
#include <libipset/linux_ip_set.h>		/* enum ipset_cmd */
#include <libipset/session.h>			/* ipset_session_* */
#include <libipset/types.h>			/* ipset_load_types */

#define IPSET_CMD_ALIASES	3

/* Commands in userspace */
struct ipset_commands {
	enum ipset_cmd cmd;
	int has_arg;
	const char *name[IPSET_CMD_ALIASES];
	const char *help;
};

#ifdef __cplusplus
extern "C" {
#endif

extern const struct ipset_commands ipset_commands[];

struct ipset_session;
struct ipset_data;
struct ipset;


/* Environment options */
struct ipset_envopts {
	int flag;
	int has_arg;
	const char *name[2];
	const char *help;
	int (*parse)(struct ipset *ipset, int flag, const char *str);
	int (*print)(char *buf, unsigned int len,
		     const struct ipset_data *data, int flag, uint8_t env);
};

extern const struct ipset_envopts ipset_envopts[];

extern bool ipset_match_cmd(const char *arg, const char * const name[]);
extern bool ipset_match_option(const char *arg, const char * const name[]);
extern bool ipset_match_envopt(const char *arg, const char * const name[]);
extern void ipset_port_usage(void);
extern int ipset_parse_filename(struct ipset *ipset, int opt, const char *str);
extern int ipset_parse_output(struct ipset *ipset,
			      int opt, const char *str);
extern int ipset_envopt_parse(struct ipset *ipset,
			      int env, const char *str);

enum ipset_exittype {
	IPSET_NO_PROBLEM = 0,
	IPSET_OTHER_PROBLEM,
	IPSET_PARAMETER_PROBLEM,
	IPSET_VERSION_PROBLEM,
	IPSET_SESSION_PROBLEM,
};

typedef int (*ipset_custom_errorfn)(struct ipset *ipset, void *p,
	int status, const char *msg, ...)
	__attribute__ ((format (printf, 4, 5)));
typedef int (*ipset_standard_errorfn)(struct ipset *ipset, void *p);

extern struct ipset_session * ipset_session(struct ipset *ipset);
extern bool ipset_is_interactive(struct ipset *ipset);
extern int ipset_custom_printf(struct ipset *ipset,
	ipset_custom_errorfn custom_error,
	ipset_standard_errorfn standard_error,
	ipset_print_outfn outfn,
	void *p);

extern int ipset_parse_argv(struct ipset *ipset, int argc, char *argv[]);
extern int ipset_parse_line(struct ipset *ipset, char *line);
extern int ipset_parse_stream(struct ipset *ipset, FILE *f);
extern struct ipset * ipset_init(void);
extern int ipset_fini(struct ipset *ipset);

#ifdef __cplusplus
}
#endif

#endif /* LIBIPSET_IPSET_H */
