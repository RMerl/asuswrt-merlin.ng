/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
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

#include <unistd.h>
#include <string.h>

#include "client.h"
#include "../log.h"

static int
cmd_medpower(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set MED power");
	lldpctl_atom_t *port;
	const char *name;
	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		lldpctl_atom_t *med_power;
		const char *what = NULL;

		med_power = lldpctl_atom_get(port, lldpctl_k_port_med_power);
		if (med_power == NULL) {
			log_warnx("lldpctl", "unable to set LLDP-MED power: support seems unavailable");
			continue; /* Need to finish the loop */
		}

		if ((what = "device type", lldpctl_atom_set_str(med_power,
			    lldpctl_k_med_power_type,
			    cmdenv_get(env, "device-type"))) == NULL ||
		    (what = "power source", lldpctl_atom_set_str(med_power,
			lldpctl_k_med_power_source,
			cmdenv_get(env, "source"))) == NULL ||
		    (what = "power priority", lldpctl_atom_set_str(med_power,
			lldpctl_k_med_power_priority,
			cmdenv_get(env, "priority"))) == NULL ||
		    (what = "power value", lldpctl_atom_set_str(med_power,
			lldpctl_k_med_power_val,
			cmdenv_get(env, "value"))) == NULL)
			log_warnx("lldpctl",
			    "unable to set LLDP MED power value for %s on %s. %s.",
			    what, name, lldpctl_last_strerror(conn));
		else {
			if (lldpctl_atom_set(port, lldpctl_k_port_med_power,
				med_power) == NULL) {
				log_warnx("lldpctl", "unable to set LLDP MED power on %s. %s.",
				    name, lldpctl_last_strerror(conn));
			} else
				log_info("lldpctl", "LLDP-MED power has been set for port %s",
				    name);
		}

		lldpctl_atom_dec_ref(med_power);
	}
	return 1;
}

static int
cmd_store_powerpairs_env_value_and_pop2(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *value)
{
	return cmd_store_something_env_value_and_pop2("powerpairs", env, value);
}
static int
cmd_store_class_env_value_and_pop2(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *value)
{
	return cmd_store_something_env_value_and_pop2("class", env, value);
}
static int
cmd_store_prio_env_value_and_pop2(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *value)
{
	return cmd_store_something_env_value_and_pop2("priority", env, value);
}

static int
cmd_dot3power(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set dot3 power");
	lldpctl_atom_t *port;
	const char *name;
	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		lldpctl_atom_t *dot3_power;
		const char *what = NULL;
		int ok = 1;

		dot3_power = lldpctl_atom_get(port, lldpctl_k_port_dot3_power);
		if (dot3_power == NULL) {
			log_warnx("lldpctl", "unable to set Dot3 power: support seems unavailable");
			continue; /* Need to finish the loop */
		}

		if ((what = "device type", lldpctl_atom_set_str(dot3_power,
			    lldpctl_k_dot3_power_devicetype,
			    cmdenv_get(env, "device-type"))) == NULL ||
		    /* Flags */
		    (what = "supported flag", lldpctl_atom_set_int(dot3_power,
			lldpctl_k_dot3_power_supported,
			cmdenv_get(env, "supported")?1:0)) == NULL ||
		    (what = "enabled flag", lldpctl_atom_set_int(dot3_power,
			lldpctl_k_dot3_power_enabled,
			cmdenv_get(env, "enabled")?1:0)) == NULL ||
		    (what = "paircontrol flag", lldpctl_atom_set_int(dot3_power,
			lldpctl_k_dot3_power_paircontrol,
			cmdenv_get(env, "paircontrol")?1:0)) == NULL ||
		    /* Powerpairs */
		    (what = "power pairs", lldpctl_atom_set_str(dot3_power,
			lldpctl_k_dot3_power_pairs,
			cmdenv_get(env, "powerpairs")?cmdenv_get(env, "powerpairs"):"unknown")) == NULL ||
		    /* Class */
		    (what = "power class", cmdenv_get(env, "class")?
			lldpctl_atom_set_str(dot3_power,
			    lldpctl_k_dot3_power_class,
			    cmdenv_get(env, "class")):
			lldpctl_atom_set_int(dot3_power,
			    lldpctl_k_dot3_power_class, 0)) == NULL ||
		    (what = "802.3at type", lldpctl_atom_set_int(dot3_power,
			lldpctl_k_dot3_power_type, 0)) == NULL) {
			log_warnx("lldpctl",
			    "unable to set LLDP Dot3 power value for %s on %s. %s.",
			    what, name, lldpctl_last_strerror(conn));
			ok = 0;
		} else if (cmdenv_get(env, "typeat")) {
			int typeat = cmdenv_get(env, "typeat")[0] - '0';
			const char *source = cmdenv_get(env, "source");
			if ((what = "802.3at type", lldpctl_atom_set_int(dot3_power,
				    lldpctl_k_dot3_power_type,
				    typeat)) == NULL ||
			    (what = "source", lldpctl_atom_set_int(dot3_power,
				lldpctl_k_dot3_power_source,
				(!strcmp(source, "primary"))?LLDP_DOT3_POWER_SOURCE_PRIMARY:
				(!strcmp(source, "backup"))? LLDP_DOT3_POWER_SOURCE_BACKUP:
				(!strcmp(source, "pse"))?    LLDP_DOT3_POWER_SOURCE_PSE:
				(!strcmp(source, "local"))?  LLDP_DOT3_POWER_SOURCE_LOCAL:
				(!strcmp(source, "both"))?   LLDP_DOT3_POWER_SOURCE_BOTH:
				LLDP_DOT3_POWER_SOURCE_UNKNOWN)) == NULL ||
			    (what = "priority", lldpctl_atom_set_str(dot3_power,
				lldpctl_k_dot3_power_priority,
				cmdenv_get(env, "priority"))) == NULL ||
			    (what = "requested power", lldpctl_atom_set_str(dot3_power,
				lldpctl_k_dot3_power_requested,
				cmdenv_get(env, "requested"))) == NULL ||
			    (what = "allocated power", lldpctl_atom_set_str(dot3_power,
				lldpctl_k_dot3_power_allocated,
				cmdenv_get(env, "allocated"))) == NULL) {
				log_warnx("lldpctl", "unable to set LLDP Dot3 power value for %s on %s. %s.",
				    what, name, lldpctl_last_strerror(conn));
				ok = 0;
			}
		}
		if (ok) {
			if (lldpctl_atom_set(port, lldpctl_k_port_dot3_power,
				dot3_power) == NULL) {
				log_warnx("lldpctl", "unable to set LLDP Dot3 power on %s. %s.",
				    name, lldpctl_last_strerror(conn));
			} else
				log_info("lldpctl", "LLDP Dot3 power has been set for port %s",
				    name);
		}

		lldpctl_atom_dec_ref(dot3_power);
	}
	return 1;
}

static int
cmd_check_type_but_no(struct cmd_env *env, void *arg)
{
	const char *what = arg;
	if (!cmdenv_get(env, "device-type")) return 0;
	if (cmdenv_get(env, what)) return 0;
	return 1;
}
static int
cmd_check_typeat_but_no(struct cmd_env *env, void *arg)
{
	const char *what = arg;
	if (!cmdenv_get(env, "typeat")) return 0;
	if (cmdenv_get(env, what)) return 0;
	return 1;
}
static int
cmd_check_type(struct cmd_env *env, const char *type)
{
	const char *etype = cmdenv_get(env, "device-type");
	if (!etype) return 0;
	return (!strcmp(type, etype));
}
static int
cmd_check_pse(struct cmd_env *env, void *arg)
{
	return cmd_check_type(env, "pse");
}
static int
cmd_check_pd(struct cmd_env *env, void *arg)
{
	return cmd_check_type(env, "pd");
}

static void
register_commands_pow_source(struct cmd_node *source)
{
	commands_new(source,
	    "unknown", "Unknown power source",
	    NULL, cmd_store_env_value_and_pop2, "source");
	commands_new(source,
	    "primary", "Primary power source",
	    cmd_check_pse, cmd_store_env_value_and_pop2, "source");
	commands_new(source,
	    "backup", "Backup power source",
	    cmd_check_pse, cmd_store_env_value_and_pop2, "source");
	commands_new(source,
	    "pse", "Power source is PSE",
	    cmd_check_pd, cmd_store_env_value_and_pop2, "source");
	commands_new(source,
	    "local", "Local power source",
	    cmd_check_pd, cmd_store_env_value_and_pop2, "source");
	commands_new(source,
	    "both", "Both PSE and local source available",
	    cmd_check_pd, cmd_store_env_value_and_pop2, "source");
}

static void
register_commands_pow_priority(struct cmd_node *priority, int key)
{
	for (lldpctl_map_t *prio_map =
		 lldpctl_key_get_map(key);
	     prio_map->string;
	     prio_map++) {
		char *tag = strdup(totag(prio_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			priority,
			tag,
			prio_map->string,
			NULL, cmd_store_prio_env_value_and_pop2, prio_map->string);
	}
}

/**
 * Register `configure med power` commands.
 */
void
register_commands_medpow(struct cmd_node *configure_med)
{
	struct cmd_node *configure_medpower = commands_new(
		configure_med,
		"power", "MED power configuration",
		NULL, NULL, NULL);

	commands_new(
		configure_medpower,
		NEWLINE, "Apply new MED power configuration",
		cmd_check_env, cmd_medpower, "device-type,source,priority,value");

	/* Type: PSE or PD */
	commands_new(
		configure_medpower,
		"pd", "MED power consumer",
		cmd_check_no_env, cmd_store_env_value_and_pop, "device-type");
	commands_new(
		configure_medpower,
		"pse", "MED power provider",
		cmd_check_no_env, cmd_store_env_value_and_pop, "device-type");

	/* Source */
	struct cmd_node *source = commands_new(
		configure_medpower,
		"source", "MED power source",
		cmd_check_type_but_no, NULL, "source");
	register_commands_pow_source(source);

	/* Priority */
	struct cmd_node *priority = commands_new(
		configure_medpower,
		"priority", "MED power priority",
		cmd_check_type_but_no, NULL, "priority");
	register_commands_pow_priority(priority, lldpctl_k_med_power_priority);

	/* Value */
	commands_new(
		commands_new(configure_medpower,
		    "value", "MED power value",
		    cmd_check_type_but_no, NULL, "value"),
		NULL, "MED power value in milliwatts",
		NULL, cmd_store_env_value_and_pop2, "value");
}

static int
cmd_check_env_power(struct cmd_env *env, void *nothing)
{
	/* We need type. If we type is PSE, we need powerpairs. If we have
	 * typeat, we also request source, priority, requested and allocated. */
	if (!cmdenv_get(env, "device-type")) return 0;
	if (!strcmp(cmdenv_get(env, "device-type"), "pse") &&
	    !cmdenv_get(env, "powerpairs")) return 0;
	if (cmdenv_get(env, "typeat")) {
		return (!!cmdenv_get(env, "source") &&
		    !!cmdenv_get(env, "priority") &&
		    !!cmdenv_get(env, "requested") &&
		    !!cmdenv_get(env, "allocated"));
	}
	return 1;
}

/**
 * Register `configure med dot3` commands.
 */
void
register_commands_dot3pow(struct cmd_node *configure_dot3)
{
	struct cmd_node *configure_dot3power = commands_new(
		configure_dot3,
		"power", "Dot3 power configuration",
		NULL, NULL, NULL);

	commands_new(
		configure_dot3power,
		NEWLINE, "Apply new Dot3 power configuration",
		cmd_check_env_power, cmd_dot3power, NULL);

	/* Type: PSE or PD */
	commands_new(
		configure_dot3power,
		"pd", "Dot3 power consumer",
		cmd_check_no_env, cmd_store_env_value_and_pop, "device-type");
	commands_new(
		configure_dot3power,
		"pse", "Dot3 power provider",
		cmd_check_no_env, cmd_store_env_value_and_pop, "device-type");

	/* Flags */
	commands_new(
		configure_dot3power,
		"supported", "MDI power support present",
		cmd_check_type_but_no, cmd_store_env_and_pop, "supported");
	commands_new(
		configure_dot3power,
		"enabled", "MDI power support enabled",
		cmd_check_type_but_no, cmd_store_env_and_pop, "enabled");
	commands_new(
		configure_dot3power,
		"paircontrol", "MDI power pair can be selected",
		cmd_check_type_but_no, cmd_store_env_and_pop, "paircontrol");

	/* Power pairs */
	struct cmd_node *powerpairs = commands_new(
		configure_dot3power,
		"powerpairs", "Which pairs are currently used for power",
		cmd_check_pse, NULL, "powerpairs");
	for (lldpctl_map_t *pp_map =
		 lldpctl_key_get_map(lldpctl_k_dot3_power_pairs);
	     pp_map->string;
	     pp_map++) {
		commands_new(
			powerpairs,
			pp_map->string,
			pp_map->string,
			NULL, cmd_store_powerpairs_env_value_and_pop2, pp_map->string);
	}

	/* Class */
	struct cmd_node *class = commands_new(
		configure_dot3power,
		"class", "Power class",
		cmd_check_type_but_no, NULL, "class");
	for (lldpctl_map_t *class_map =
		 lldpctl_key_get_map(lldpctl_k_dot3_power_class);
	     class_map->string;
	     class_map++) {
		const char *tag = strdup(totag(class_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			class,
			tag,
			class_map->string,
			NULL, cmd_store_class_env_value_and_pop2, class_map->string);
	}

	/* 802.3at type */
	struct cmd_node *typeat = commands_new(
		configure_dot3power,
		"type", "802.3at device type",
		cmd_check_type_but_no, NULL, "typeat");
	commands_new(typeat,
	    "1", "802.3at type 1",
	    NULL, cmd_store_env_value_and_pop2, "typeat");
	commands_new(typeat,
	    "2", "802.3at type 2",
	    NULL, cmd_store_env_value_and_pop2, "typeat");

	/* Source */
	struct cmd_node *source = commands_new(
		configure_dot3power,
		"source", "802.3at dot3 power source (mandatory)",
		cmd_check_typeat_but_no, NULL, "source");
	register_commands_pow_source(source);

	/* Priority */
	struct cmd_node *priority = commands_new(
		configure_dot3power,
		"priority", "802.3at dot3 power priority (mandatory)",
		cmd_check_typeat_but_no, NULL, "priority");
	register_commands_pow_priority(priority, lldpctl_k_dot3_power_priority);

	/* Values */
	commands_new(
		commands_new(configure_dot3power,
		    "requested", "802.3at dot3 power value requested (mandatory)",
		    cmd_check_typeat_but_no, NULL, "requested"),
		NULL, "802.3at power value requested in milliwatts",
		NULL, cmd_store_env_value_and_pop2, "requested");
	commands_new(
		commands_new(configure_dot3power,
		    "allocated", "802.3at dot3 power value allocated (mandatory)",
		    cmd_check_typeat_but_no, NULL, "allocated"),
		NULL, "802.3at power value allocated in milliwatts",
		NULL, cmd_store_env_value_and_pop2, "allocated");
}
