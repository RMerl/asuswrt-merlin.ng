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

#include "client.h"
#include <string.h>
#include <limits.h>

/**
 * Show neighbors.
 *
 * The environment will contain the following keys:
 *  - C{ports} list of ports we want to restrict showing.
 *  - C{hidden} if we should show hidden ports.
 *  - C{summary} if we want to show only a summary
 *  - C{detailed} for a detailed overview
 */
static int
cmd_show_neighbors(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "show neighbors data (%s) %s hidden neighbors",
	    cmdenv_get(env, "summary")?"summary":
	    cmdenv_get(env, "detailed")?"detailed":
	    "normal", cmdenv_get(env, "hidden")?"with":"without");
	if (cmdenv_get(env, "ports"))
		log_debug("lldpctl", "restrict to the following ports: %s",
		    cmdenv_get(env, "ports"));

	display_interfaces(conn, w, env, !!cmdenv_get(env, "hidden"),
	    cmdenv_get(env, "summary")?DISPLAY_BRIEF:
	    cmdenv_get(env, "detailed")?DISPLAY_DETAILS:
	    DISPLAY_NORMAL);

	return 1;
}

/**
 * Show interfaces.
 *
 * The environment will contain the following keys:
 *  - C{ports} list of ports we want to restrict showing.
 *  - C{hidden} if we should show hidden ports.
 *  - C{summary} if we want to show only a summary
 *  - C{detailed} for a detailed overview
 */
static int
cmd_show_interfaces(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "show interfaces data (%s) %s hidden interfaces",
	    cmdenv_get(env, "summary")?"summary":
	    cmdenv_get(env, "detailed")?"detailed":
	    "normal", cmdenv_get(env, "hidden")?"with":"without");
	if (cmdenv_get(env, "ports"))
		log_debug("lldpctl", "restrict to the following ports: %s",
		    cmdenv_get(env, "ports"));

	display_local_interfaces(conn, w, env, !!cmdenv_get(env, "hidden"),
	    cmdenv_get(env, "summary")?DISPLAY_BRIEF:
	    cmdenv_get(env, "detailed")?DISPLAY_DETAILS:
	    DISPLAY_NORMAL);

	return 1;
}

/**
 * Show chassis.
 *
 * The environment will contain the following keys:
 *  - C{summary} if we want to show only a summary
 *  - C{detailed} for a detailed overview
 */
static int
cmd_show_chassis(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "show chassis data (%s)",
	    cmdenv_get(env, "summary")?"summary":
	    cmdenv_get(env, "detailed")?"detailed":
	    "normal");

	display_local_chassis(conn, w, env,
	    cmdenv_get(env, "summary")?DISPLAY_BRIEF:
	    cmdenv_get(env, "detailed")?DISPLAY_DETAILS:
	    DISPLAY_NORMAL);

	return 1;
}

/**
 * Show stats.
 *
 * The environment will contain the following keys:
 *  - C{ports} list of ports we want to restrict showing.
 *  - C{summary} summary of stats
 */
static int
cmd_show_interface_stats(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "show stats data");
	if (cmdenv_get(env, "ports"))
		log_debug("lldpctl", "restrict to the following ports: %s",
		    cmdenv_get(env, "ports"));
	if (cmdenv_get(env, "summary"))
		log_debug("lldpctl", "show summary of stats across ports");

	display_interfaces_stats(conn, w, env);

	return 1;
}

static int
cmd_check_no_detailed_nor_summary(struct cmd_env *env, void *arg)
{
	if (cmdenv_get(env, "detailed")) return 0;
	if (cmdenv_get(env, "summary")) return 0;
	return 1;
}

/**
 * Show running configuration.
 */
static int
cmd_show_configuration(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "show running configuration");
	display_configuration(conn, w);
	return 1;
}

struct watcharg {
	struct cmd_env *env;
	struct writer *w;
	size_t nb;
};

/**
 * Callback for the next function to display a new neighbor.
 */
static void
watchcb(lldpctl_change_t type,
    lldpctl_atom_t *interface,
    lldpctl_atom_t *neighbor,
    void *data)
{
	struct watcharg *wa = data;
	struct cmd_env *env = wa->env;
	struct writer *w = wa->w;
	const char *interfaces = cmdenv_get(env, "ports");
	const char *proto_str;
	int protocol = LLDPD_MODE_MAX;

	if (interfaces && !contains(interfaces, lldpctl_atom_get_str(interface,
		    lldpctl_k_interface_name)))
		return;

	/* user might have specified protocol to filter display results */
	proto_str = cmdenv_get(env, "protocol");

	if (proto_str) {
		log_debug("display", "filter protocol: %s ", proto_str);

		protocol = 0;	/* unsupported */
		for (lldpctl_map_t *protocol_map =
			 lldpctl_key_get_map(lldpctl_k_port_protocol);
		     protocol_map->string;
		     protocol_map++) {
			if (!strcasecmp(proto_str, protocol_map->string)) {
				protocol = protocol_map->value;
				break;
			}
		}
	}

	switch (type) {
	case lldpctl_c_deleted:
		tag_start(w, "lldp-deleted", "LLDP neighbor deleted");
		break;
	case lldpctl_c_updated:
		tag_start(w, "lldp-updated", "LLDP neighbor updated");
		break;
	case lldpctl_c_added:
		tag_start(w, "lldp-added", "LLDP neighbor added");
		break;
	default: return;
	}
	display_interface(NULL, w, 1, interface, neighbor,
	    cmdenv_get(env, "summary")?DISPLAY_BRIEF:
	    cmdenv_get(env, "detailed")?DISPLAY_DETAILS:
	    DISPLAY_NORMAL, protocol);
	tag_end(w);
	wa->nb++;
}

/**
 * Watch for neighbor changes.
 */
static int
cmd_watch_neighbors(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	struct watcharg wa = {
		.env = env,
		.w = w,
		.nb = 0
	};
	const char *limit_str = cmdenv_get(env, "limit");
	size_t limit = 0;

	if (limit_str) {
		const char *errstr;
		limit = strtonum(limit_str, 1, LLONG_MAX, &errstr);
		if (errstr != NULL) {
			log_warnx("lldpctl", "specified limit (%s) is %s and ignored",
			    limit_str, errstr);
		}
	}

	log_debug("lldpctl", "watch for neighbor changes");
	if (lldpctl_watch_callback2(conn, watchcb, &wa) < 0) {
		log_warnx("lldpctl", "unable to watch for neighbors. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	while (1) {
		if (lldpctl_watch(conn) < 0) {
			log_warnx("lldpctl", "unable to watch for neighbors. %s",
			    lldpctl_last_strerror(conn));
			return 0;
		}
		if (limit > 0 && wa.nb >= limit)
			return 1;
	}
	return 0;
}

/**
 * Register common subcommands for `watch` and `show neighbors` and `show chassis'
 */
void
register_common_commands(struct cmd_node *root, int neighbor)
{
	/* With more details */
	commands_new(root,
	    "details",
	    "With more details",
	    cmd_check_no_detailed_nor_summary, cmd_store_env_and_pop, "detailed");

	/* With less details */
	commands_new(root,
	    "summary",
	    "With less details",
	    cmd_check_no_detailed_nor_summary, cmd_store_env_and_pop, "summary");

	if (!neighbor) return;

	/* With hidden neighbors */
	commands_new(root,
	    "hidden",
	    "Include hidden neighbors",
	    cmd_check_no_env, cmd_store_env_and_pop, "hidden");

	/* Some specific port */
	cmd_restrict_ports(root);

	/* Specific protocol */
	cmd_restrict_protocol(root);
}

/**
 * Register sub command summary
 */
void
register_summary_command(struct cmd_node *root)
{
	commands_new(root,
			"summary",
			"With less details",
			cmd_check_no_detailed_nor_summary, cmd_store_env_and_pop, "summary");
}

/**
 * Register subcommands to `show`
 *
 * @param root Root node
 */
void
register_commands_show(struct cmd_node *root)
{
	struct cmd_node *show = commands_new(
		root,
		"show",
		"Show running system information",
		NULL, NULL, NULL);
	struct cmd_node *neighbors = commands_new(
		show,
		"neighbors",
		"Show neighbors data",
		NULL, NULL, NULL);

	struct cmd_node *interfaces = commands_new(
		show,
		"interfaces",
		"Show interfaces data",
		NULL, NULL, NULL);

	struct cmd_node *chassis = commands_new(
		show,
		"chassis",
		"Show local chassis data",
		NULL, NULL, NULL);

	struct cmd_node *stats = commands_new(
		show,
		"statistics",
		"Show statistics",
		NULL, NULL, NULL);

	/* Neighbors data */
	commands_new(neighbors,
	    NEWLINE,
	    "Show neighbors data",
	    NULL, cmd_show_neighbors, NULL);

	register_common_commands(neighbors, 1);

	/* Interfaces data */
	commands_new(interfaces,
	    NEWLINE,
	    "Show interfaces data",
	    NULL, cmd_show_interfaces, NULL);

	cmd_restrict_ports(interfaces);
	register_common_commands(interfaces, 0);

	/* Chassis data */
	commands_new(chassis,
	    NEWLINE,
	    "Show local chassis data",
	    NULL, cmd_show_chassis, NULL);

	register_common_commands(chassis, 0);

	/* Stats data */
	commands_new(stats,
	    NEWLINE,
	    "Show stats data",
	    NULL, cmd_show_interface_stats, NULL);

	cmd_restrict_ports(stats);
	register_summary_command(stats);

	/* Register "show configuration" and "show running-configuration" */
	commands_new(
		commands_new(show,
		    "configuration",
		    "Show running configuration",
		    NULL, NULL, NULL),
		NEWLINE,
		"Show running configuration",
		NULL, cmd_show_configuration, NULL);
	commands_new(
		commands_new(show,
		    "running-configuration",
		    "Show running configuration",
		    NULL, NULL, NULL),
		NEWLINE,
		"Show running configuration",
		NULL, cmd_show_configuration, NULL);
}

/**
 * Register subcommands to `watch`
 *
 * @param root Root node
 */
void
register_commands_watch(struct cmd_node *root)
{
	struct cmd_node *watch = commands_new(
		root,
		"watch",
		"Monitor neighbor changes",
		NULL, NULL, NULL);

	commands_new(watch,
	    NEWLINE,
	    "Monitor neighbors change",
	    NULL, cmd_watch_neighbors, NULL);

	commands_new(
		commands_new(watch,
		    "limit",
		    "Don't show more than X events",
		    cmd_check_no_env, NULL, "limit"),
		NULL,
		"Stop after getting X events",
		NULL, cmd_store_env_value_and_pop2, "limit");

	register_common_commands(watch, 1);
}
