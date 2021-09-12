/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _CLIENT_H
#define _CLIENT_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "../lib/lldpctl.h"
#include "../lldp-const.h"
#include "../log.h"
#include "../ctl.h"
#include "../compat/compat.h"
#include "writer.h"

#ifdef HAVE_ADDRESS_SANITIZER
# include <sanitizer/lsan_interface.h>
# define SUPPRESS_LEAK(x) __lsan_ignore_object(x)
#else
# define SUPPRESS_LEAK(x)
#endif

/* Readline stuff */
#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else
extern char *readline();
extern char *rl_line_buffer;
extern int rl_point;
extern int rl_insert_text(const char*);
extern void rl_forced_update_display(void);
extern int rl_bind_key(int, int(*f)(int, int));
#  endif
#endif
#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else
extern void add_history ();
#  endif
#endif
#undef NEWLINE

extern const char *ctlname;

/* commands.c */
#define NEWLINE "<CR>"
struct cmd_node;
struct cmd_env;
struct cmd_node *commands_root(void);
struct cmd_node *commands_new(
	struct cmd_node *,
	const char *,
	const char *,
	int(*validate)(struct cmd_env*, void *),
	int(*execute)(struct lldpctl_conn_t*, struct writer*,
	    struct cmd_env*, void *),
	void *);
struct cmd_node* commands_privileged(struct cmd_node *);
struct cmd_node* commands_lock(struct cmd_node *);
struct cmd_node* commands_hidden(struct cmd_node *);
void commands_free(struct cmd_node *);
const char *cmdenv_arg(struct cmd_env*);
const char *cmdenv_get(struct cmd_env*, const char*);
int cmdenv_put(struct cmd_env*, const char*, const char*);
int cmdenv_pop(struct cmd_env*, int);
int commands_execute(struct lldpctl_conn_t *, struct writer *,
    struct cmd_node *, int, const char **, int);
char *commands_complete(struct cmd_node *, int, const char **,
    int, int);
/* helpers */
int cmd_check_no_env(struct cmd_env *, void *);
int cmd_check_env(struct cmd_env *, void *);
int cmd_store_env(struct lldpctl_conn_t *, struct writer *,
    struct cmd_env *, void *);
int cmd_store_env_and_pop(struct lldpctl_conn_t *, struct writer *,
    struct cmd_env *, void *);
int cmd_store_env_value(struct lldpctl_conn_t *, struct writer *,
    struct cmd_env *, void *);
int cmd_store_env_value_and_pop(struct lldpctl_conn_t *, struct writer *,
    struct cmd_env *, void *);
int cmd_store_env_value_and_pop2(struct lldpctl_conn_t *, struct writer *,
    struct cmd_env *, void *);
int cmd_store_env_value_and_pop3(struct lldpctl_conn_t *, struct writer *,
    struct cmd_env *, void *);
int cmd_store_something_env_value_and_pop2(const char *, struct cmd_env *,
    void *);
int cmd_store_something_env_value(const char *, struct cmd_env *,
    void *);
lldpctl_atom_t* cmd_iterate_on_interfaces(struct lldpctl_conn_t *,
    struct cmd_env *);
lldpctl_atom_t* cmd_iterate_on_ports(struct lldpctl_conn_t *,
    struct cmd_env *, const char **);
void cmd_restrict_ports(struct cmd_node *);
void cmd_restrict_protocol(struct cmd_node *);

/* misc.c */
int contains(const char *, const char *);
char*  totag(const char *);

/* display.c */
#define DISPLAY_BRIEF   1
#define DISPLAY_NORMAL  2
#define DISPLAY_DETAILS 3
void display_interfaces(lldpctl_conn_t *, struct writer *,
    struct cmd_env *, int, int);
void display_interface(lldpctl_conn_t *, struct writer *, int,
    lldpctl_atom_t *, lldpctl_atom_t *, int, int);
void display_local_chassis(lldpctl_conn_t *, struct writer *,
    struct cmd_env *, int);
void display_configuration(lldpctl_conn_t *, struct writer *);
void display_interfaces_stats(lldpctl_conn_t *, struct writer *,
    struct cmd_env *);
void display_interface_stats(lldpctl_conn_t *, struct writer *,
    lldpctl_atom_t *);
void display_local_interfaces(lldpctl_conn_t *, struct writer *,
    struct cmd_env *, int, int);



/* show.c */
void register_commands_show(struct cmd_node *);
void register_commands_watch(struct cmd_node *);

/* conf*.c */
void register_commands_configure(struct cmd_node *);
void register_commands_configure_system(struct cmd_node *, struct cmd_node *);
void register_commands_configure_lldp(struct cmd_node *, struct cmd_node *);
void register_commands_configure_med(struct cmd_node *, struct cmd_node *);
void register_commands_configure_dot3(struct cmd_node *);
void register_commands_medpow(struct cmd_node *);
void register_commands_dot3pow(struct cmd_node *);

/* tokenizer.c */
int tokenize_line(const char*, int*, char***);
void tokenize_free(int, char**);

#endif
