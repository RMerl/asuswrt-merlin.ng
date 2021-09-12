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

#include <unistd.h>
#include <string.h>

#include "client.h"
#include "../log.h"

static int
_cmd_medlocation(struct lldpctl_conn_t *conn,
    struct cmd_env *env, int format)
{
	lldpctl_atom_t *port;
	const char *name;
	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		lldpctl_atom_t *med_location = NULL, *med_locations = NULL;
		const char *what = NULL;
		int ok = 0;

		med_locations = lldpctl_atom_get(port, lldpctl_k_port_med_locations);
		if (med_locations == NULL) {
			log_warnx("lldpctl", "unable to set LLDP-MED location: support seems unavailable");
			continue; /* Iterator needs to be run until end */
		}

		med_location = lldpctl_atom_iter_value(med_locations,
		    lldpctl_atom_iter_next(med_locations,
			lldpctl_atom_iter(med_locations)));

		switch (format) {
		case LLDP_MED_LOCFORMAT_COORD:
			if ((what = "format", lldpctl_atom_set_int(med_location,
				    lldpctl_k_med_location_format,
				    format)) == NULL ||
			    (what = "latitude", lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_latitude,
				cmdenv_get(env, "latitude"))) == NULL ||
			    (what = "longitude", lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_longitude,
				cmdenv_get(env, "longitude"))) == NULL ||
			    (what = "altitude", lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_altitude,
				cmdenv_get(env, "altitude"))) == NULL ||
			    (what = "altitude unit", lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_altitude_unit,
				cmdenv_get(env, "altitude-unit"))) == NULL ||
			    (what = "datum", lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_geoid,
				cmdenv_get(env, "datum"))) == NULL)
				log_warnx("lldpctl",
				    "unable to set LLDP MED location value for %s on %s. %s.",
				    what, name, lldpctl_last_strerror(conn));
			else ok = 1;
			break;
		case LLDP_MED_LOCFORMAT_CIVIC:
			if ((what = "format", lldpctl_atom_set_int(med_location,
				    lldpctl_k_med_location_format,
				    format)) == NULL ||
			    (what = "country", lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_country,
				cmdenv_get(env, "country"))) == NULL) {
				log_warnx("lldpctl",
				    "unable to set LLDP MED location value for %s on %s. %s.",
				    what, name, lldpctl_last_strerror(conn));
				break;
			}
			ok = 1;
			for (lldpctl_map_t *addr_map =
				 lldpctl_key_get_map(lldpctl_k_med_civicaddress_type);
			     addr_map->string;
			     addr_map++) {
				lldpctl_atom_t *cael, *caels;
				const char *value = cmdenv_get(env, addr_map->string);
				if (!value) continue;

				caels = lldpctl_atom_get(med_location, lldpctl_k_med_location_ca_elements);
				cael = lldpctl_atom_create(caels);

				if (lldpctl_atom_set_str(cael, lldpctl_k_med_civicaddress_type,
					addr_map->string) == NULL ||
				    lldpctl_atom_set_str(cael, lldpctl_k_med_civicaddress_value,
					value) == NULL ||
				    lldpctl_atom_set(med_location,
					lldpctl_k_med_location_ca_elements,
					cael) == NULL) {
						log_warnx("lldpctl",
						    "unable to add a civic address element `%s`. %s",
						    addr_map->string,
						    lldpctl_last_strerror(conn));
						ok = 0;
				}

				lldpctl_atom_dec_ref(cael);
				lldpctl_atom_dec_ref(caels);
				if (!ok) break;
			}
			break;
		case LLDP_MED_LOCFORMAT_ELIN:
			if (lldpctl_atom_set_int(med_location,
				lldpctl_k_med_location_format, format) == NULL ||
			    lldpctl_atom_set_str(med_location,
				lldpctl_k_med_location_elin, cmdenv_get(env, "elin")) == NULL)
				log_warnx("lldpctl", "unable to set LLDP MED location on %s. %s",
				    name, lldpctl_last_strerror(conn));
			else ok = 1;
			break;
		}
		if (ok) {
			if (lldpctl_atom_set(port, lldpctl_k_port_med_locations,
				med_location) == NULL) {
				log_warnx("lldpctl", "unable to set LLDP MED location on %s. %s.",
				    name, lldpctl_last_strerror(conn));
			} else
				log_info("lldpctl", "LLDP-MED location has been set for port %s",
				    name);
		}

		lldpctl_atom_dec_ref(med_location);
		lldpctl_atom_dec_ref(med_locations);
	}
	return 1;
}

static int
cmd_medlocation_coordinate(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set MED location coordinate");
	return _cmd_medlocation(conn, env, LLDP_MED_LOCFORMAT_COORD);
}

static int
cmd_medlocation_address(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set MED location address");
	return _cmd_medlocation(conn, env, LLDP_MED_LOCFORMAT_CIVIC);
}

static int
cmd_medlocation_elin(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set MED location ELIN");
	return _cmd_medlocation(conn, env, LLDP_MED_LOCFORMAT_ELIN);
}

static int
cmd_medpolicy(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set MED policy");
	lldpctl_atom_t *iface;
	while ((iface = cmd_iterate_on_interfaces(conn, env))) {
		const char *name = lldpctl_atom_get_str(iface, lldpctl_k_interface_name);
		lldpctl_atom_t *port = lldpctl_get_port(iface);
		lldpctl_atom_t *med_policy = NULL, *med_policies = NULL;
		const char *what = NULL;

		med_policies = lldpctl_atom_get(port, lldpctl_k_port_med_policies);
		if (med_policies == NULL) {
			log_warnx("lldpctl", "unable to set LLDP-MED policies: support seems unavailable");
			goto end;
		}

		med_policy = lldpctl_atom_iter_value(med_policies,
		    lldpctl_atom_iter_next(med_policies,
			lldpctl_atom_iter(med_policies)));

		if ((what = "application", lldpctl_atom_set_str(med_policy,
			    lldpctl_k_med_policy_type,
			    cmdenv_get(env, "application"))) == NULL ||
		    (what = "unknown flag", lldpctl_atom_set_int(med_policy,
			lldpctl_k_med_policy_unknown,
			cmdenv_get(env, "unknown")?1:0)) == NULL ||
		    (what = "tagged flag", lldpctl_atom_set_int(med_policy,
			lldpctl_k_med_policy_tagged,
			cmdenv_get(env, "tagged")?1:0)) == NULL ||
		    (what = "vlan",
			cmdenv_get(env, "vlan")?
			lldpctl_atom_set_str(med_policy,
			    lldpctl_k_med_policy_vid,
			    cmdenv_get(env, "vlan")):
			lldpctl_atom_set_int(med_policy,
			    lldpctl_k_med_policy_vid, 0)) == NULL ||
		    (what = "priority",
			cmdenv_get(env, "priority")?
			lldpctl_atom_set_str(med_policy,
			    lldpctl_k_med_policy_priority,
			    cmdenv_get(env, "priority")):
			lldpctl_atom_set_int(med_policy,
			    lldpctl_k_med_policy_priority,
			    0)) == NULL ||
		    (what = "dscp",
			cmdenv_get(env, "dscp")?
			lldpctl_atom_set_str(med_policy,
			    lldpctl_k_med_policy_dscp,
			    cmdenv_get(env, "dscp")):
			lldpctl_atom_set_int(med_policy,
			    lldpctl_k_med_policy_dscp,
			    0)) == NULL)
			log_warnx("lldpctl",
			    "unable to set LLDP MED policy value for %s on %s. %s.",
			    what, name, lldpctl_last_strerror(conn));
		else {
			if (lldpctl_atom_set(port, lldpctl_k_port_med_policies,
				med_policy) == NULL) {
				log_warnx("lldpctl", "unable to set LLDP MED policy on %s. %s.",
				    name, lldpctl_last_strerror(conn));
			} else
				log_info("lldpctl", "LLDP-MED policy has been set for port %s",
				    name);
		}

	end:
		lldpctl_atom_dec_ref(med_policy);
		lldpctl_atom_dec_ref(med_policies);
		lldpctl_atom_dec_ref(port);
	}
	return 1;
}

/**
 * Register `configure med location coordinate` commands.
 */
static void
register_commands_medloc_coord(struct cmd_node *configure_medlocation)
{
	/* MED location coordinate (set) */
	struct cmd_node *configure_medloc_coord = commands_new(
		configure_medlocation,
		"coordinate", "MED location coordinate configuration",
		NULL, NULL, NULL);
	commands_new(configure_medloc_coord,
	    NEWLINE, "Configure MED location coordinates",
	    cmd_check_env, cmd_medlocation_coordinate,
	    "latitude,longitude,altitude,altitude-unit,datum");
	commands_new(
		commands_new(
			configure_medloc_coord,
			"latitude", "Specify latitude",
			cmd_check_no_env, NULL, "latitude"),
		NULL, "Latitude as xx.yyyyN or xx.yyyyS",
		NULL, cmd_store_env_value_and_pop2, "latitude");
	commands_new(
		commands_new(
			configure_medloc_coord,
			"longitude", "Specify longitude",
			cmd_check_no_env, NULL, "longitude"),
		NULL, "Longitude as xx.yyyyE or xx.yyyyW",
		NULL, cmd_store_env_value_and_pop2, "longitude");
	struct cmd_node *altitude = commands_new(
		commands_new(
			configure_medloc_coord,
			"altitude", "Specify altitude",
			cmd_check_no_env, NULL, "altitude"),
		NULL, "Altitude",
		NULL, cmd_store_env_value, "altitude");
	commands_new(altitude,
	    "m", "meters",
	    NULL, cmd_store_env_value_and_pop3, "altitude-unit");
	commands_new(altitude,
	    "f", "floors",
	    NULL, cmd_store_env_value_and_pop3, "altitude-unit");

	struct cmd_node *datum = commands_new(configure_medloc_coord,
	    "datum", "Specify datum",
	    cmd_check_no_env, NULL, "datum");
	for (lldpctl_map_t *datum_map =
		 lldpctl_key_get_map(lldpctl_k_med_location_geoid);
	     datum_map->string;
	     datum_map++)
		commands_new(datum, datum_map->string, NULL,
		    NULL, cmd_store_env_value_and_pop2, "datum");
}

/**
 * Register `configure med location address` commands.
 */
static void
register_commands_medloc_addr(struct cmd_node *configure_medlocation)
{
	/* MED location address (set) */
	struct cmd_node *configure_medloc_addr = commands_new(
		configure_medlocation,
		"address", "MED location address configuration",
		NULL, NULL, NULL);
	commands_new(configure_medloc_addr,
	    NEWLINE, "Configure MED location address",
	    cmd_check_env, cmd_medlocation_address,
	    "country");

	/* Country */
	commands_new(
		commands_new(
			configure_medloc_addr,
			"country", "Specify country (mandatory)",
			cmd_check_no_env, NULL, "country"),
		NULL, "Country as a two-letter code",
		NULL, cmd_store_env_value_and_pop2, "country");

	/* Other fields */
	for (lldpctl_map_t *addr_map =
		 lldpctl_key_get_map(lldpctl_k_med_civicaddress_type);
	     addr_map->string;
	     addr_map++) {
		const char *tag = strdup(totag(addr_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			commands_new(
				configure_medloc_addr,
				tag,
				addr_map->string,
				cmd_check_no_env, NULL, addr_map->string),
			NULL, addr_map->string,
			NULL, cmd_store_env_value_and_pop2, addr_map->string);
	}
}

/**
 * Register `configure med location elin` commands.
 */
static void
register_commands_medloc_elin(struct cmd_node *configure_medlocation)
{
	/* MED location elin (set) */
	commands_new(
		commands_new(
			commands_new(
				configure_medlocation,
				"elin", "MED location ELIN configuration",
				NULL, NULL, NULL),
			NULL, "ELIN number",
			NULL, cmd_store_env_value, "elin"),
		NEWLINE, "Set MED location ELIN number",
		NULL, cmd_medlocation_elin, NULL);
}

/**
 * Register `configure med location` commands.
 */
static void
register_commands_medloc(struct cmd_node *configure_med)
{
	struct cmd_node *configure_medlocation = commands_new(
		configure_med,
		"location", "MED location configuration",
		NULL, NULL, NULL);

	register_commands_medloc_coord(configure_medlocation);
	register_commands_medloc_addr(configure_medlocation);
	register_commands_medloc_elin(configure_medlocation);
}

static int
cmd_check_application_but_no(struct cmd_env *env, void *arg)
{
	const char *what = arg;
	if (!cmdenv_get(env, "application")) return 0;
	if (cmdenv_get(env, what)) return 0;
	return 1;
}
static int
cmd_store_app_env_value_and_pop2(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *value)
{
	return cmd_store_something_env_value_and_pop2("application", env, value);
}
static int
cmd_store_prio_env_value_and_pop2(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *value)
{
	return cmd_store_something_env_value_and_pop2("priority", env, value);
}

/**
 * Register `configure med policy` commands.
 */
static void
register_commands_medpol(struct cmd_node *configure_med)
{
	struct cmd_node *configure_medpolicy = commands_new(
		configure_med,
		"policy", "MED policy configuration",
		NULL, NULL, NULL);

	commands_new(
		configure_medpolicy,
		NEWLINE, "Apply new MED policy",
		cmd_check_env, cmd_medpolicy, "application");

	/* Application */
	struct cmd_node *configure_application =
	    commands_new(
		    configure_medpolicy,
		    "application", "MED policy application",
		    cmd_check_no_env, NULL, "application");

	for (lldpctl_map_t *pol_map =
		 lldpctl_key_get_map(lldpctl_k_med_policy_type);
	     pol_map->string;
	     pol_map++) {
		char *tag = strdup(totag(pol_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			configure_application,
			tag,
			pol_map->string,
			NULL, cmd_store_app_env_value_and_pop2, pol_map->string);
	}

	/* Remaining keywords */
	commands_new(
		configure_medpolicy,
		"unknown", "Set unknown flag",
		cmd_check_application_but_no, cmd_store_env_and_pop, "unknown");
	commands_new(
		configure_medpolicy,
		"tagged", "Set tagged flag",
		cmd_check_application_but_no, cmd_store_env_and_pop, "tagged");
	commands_new(
		commands_new(
			configure_medpolicy,
			"vlan", "VLAN advertising",
			cmd_check_application_but_no, NULL, "vlan"),
		NULL, "VLAN ID to advertise",
		NULL, cmd_store_env_value_and_pop2, "vlan");
	commands_new(
		commands_new(
			configure_medpolicy,
			"dscp", "DiffServ advertising",
			cmd_check_application_but_no, NULL, "dscp"),
		NULL, "DSCP value to advertise (between 0 and 63)",
		NULL, cmd_store_env_value_and_pop2, "dscp");
	struct cmd_node *priority =
	    commands_new(
		    configure_medpolicy,
		    "priority", "MED policy priority",
		    cmd_check_application_but_no, NULL, "priority");
	for (lldpctl_map_t *prio_map =
		 lldpctl_key_get_map(lldpctl_k_med_policy_priority);
	     prio_map->string;
	     prio_map++) {
		char *tag = strdup(totag(prio_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			priority,
			tag, prio_map->string,
			NULL, cmd_store_prio_env_value_and_pop2, prio_map->string);
	}
}

static int
cmd_faststart(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "configure fast interval support");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}

	char *action = arg;
	if ((!strcmp(action, "enable") &&
		(lldpctl_atom_set_int(config,
		    lldpctl_k_config_fast_start_enabled, 1) == NULL)) ||
	    (!strcmp(action, "disable") &&
		(lldpctl_atom_set_int(config,
		    lldpctl_k_config_fast_start_enabled, 0) == NULL)) ||
	    (!strcmp(action, "delay") &&
		(lldpctl_atom_set_str(config,
		    lldpctl_k_config_fast_start_interval,
		    cmdenv_get(env, "tx-interval")) == NULL))) {
		log_warnx("lldpctl", "unable to setup fast start. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "configruation for fast start applied");
	lldpctl_atom_dec_ref(config);
	return 1;
}

/**
 * Register "configure med fast-start *"
 */
static void
register_commands_medfast(struct cmd_node *med, struct cmd_node *nomed)
{
	struct cmd_node *configure_fast = commands_new(
		med,
		"fast-start", "Fast start configuration",
		cmd_check_no_env, NULL, "ports");
	struct cmd_node *unconfigure_fast = commands_new(
		nomed,
		"fast-start", "Fast start configuration",
		cmd_check_no_env, NULL, "ports");

	/* Enable */
	commands_new(
		commands_new(
			configure_fast,
			"enable", "Enable fast start",
			NULL, NULL, NULL),
		NEWLINE, "Enable fast start",
		NULL, cmd_faststart, "enable");

	/* Set TX delay */
        commands_new(
		commands_new(
			commands_new(configure_fast,
			    "tx-interval", "Set LLDP fast transmit delay",
			    NULL, NULL, NULL),
			NULL, "LLDP fast transmit delay in seconds",
			NULL, cmd_store_env_value, "tx-interval"),
		NEWLINE, "Set LLDP fast transmit delay",
		NULL, cmd_faststart, "delay");

	/* Disable */
	commands_new(
		commands_new(
			unconfigure_fast,
			NEWLINE, "Disable fast start",
			NULL, cmd_faststart, "disable"),
		NEWLINE, "Disable fast start",
		NULL, cmd_faststart, "disable");
}

/**
 * Register "configure med *"
 */
void
register_commands_configure_med(struct cmd_node *configure, struct cmd_node *unconfigure)
{
	if (lldpctl_key_get_map(
		    lldpctl_k_med_policy_type)[0].string == NULL)
		return;

	struct cmd_node *configure_med = commands_new(
		configure,
		"med", "MED configuration",
		NULL, NULL, NULL);
	struct cmd_node *unconfigure_med = commands_new(
		unconfigure,
		"med", "MED configuration",
		NULL, NULL, NULL);

	register_commands_medloc(configure_med);
	register_commands_medpol(configure_med);
	register_commands_medpow(configure_med);
	register_commands_medfast(configure_med, unconfigure_med);
}
