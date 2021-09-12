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
#include <sys/utsname.h>

#include "client.h"
#include "../log.h"

static int
cmd_iface_pattern(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set iface pattern");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}

	const char *value = cmdenv_get(env, "iface-pattern");
	if (lldpctl_atom_set_str(config,
		lldpctl_k_config_iface_pattern,
		value) == NULL) {
		log_warnx("lldpctl", "unable to set iface-pattern. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "iface-pattern set to new value %s",
	    value?value:"(none)");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_perm_iface_pattern(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set permanent iface pattern");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}

	const char *value = cmdenv_get(env, "iface-pattern");
	if (lldpctl_atom_set_str(config,
		lldpctl_k_config_perm_iface_pattern,
		value) == NULL) {
		log_warnx("lldpctl", "unable to set permanent iface pattern. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "permanent iface pattern set to new value %s",
	    value?value:"(none)");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_iface_promisc(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_iface_promisc,
		arg?1:0) == NULL) {
		log_warnx("lldpctl", "unable to %s promiscuous mode: %s",
		    arg?"enable":"disable",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "interface promiscuous mode %s",
	    arg?"enabled":"disabled");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_system_description(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	int platform = 0;
	const char *what = arg;
	const char *value;
	if (!strcmp(what, "system")) {
		value = cmdenv_get(env, "description");
	} else {
		value = cmdenv_get(env, "platform");
		platform = 1;
	}
	log_debug("lldpctl", "set %s description", what);
	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_str(config,
		platform?lldpctl_k_config_platform:lldpctl_k_config_description,
		value) == NULL) {
		log_warnx("lldpctl", "unable to set description. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "description set to new value %s",
	    value?value:"(none)");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_system_chassisid(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	const char *value;
	value = cmdenv_get(env, "description");
	log_debug("lldpctl", "set chassis ID");
	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_str(config,
	    lldpctl_k_config_cid_string,
	    value) == NULL) {
		log_warnx("lldpctl", "unable to set chassis ID. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "chassis ID set to new value %s",
	    value?value:"(none)");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_management(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set management pattern");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}

	const char *value = cmdenv_get(env, "management-pattern");
	if (lldpctl_atom_set_str(config,
		lldpctl_k_config_mgmt_pattern, value) == NULL) {
		log_warnx("lldpctl", "unable to set management pattern. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "management pattern set to new value %s",
	    value?value:"(none)");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_hostname(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	struct utsname un;
	log_debug("lldpctl", "set system name");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}

	const char *value = cmdenv_get(env, "hostname");
	if (value && strlen(value) == 1 && value[0] == '.') {
		if (uname(&un) < 0) {
			log_warn("lldpctl", "cannot get node name");
			lldpctl_atom_dec_ref(config);
			return 0;
		}
		value = un.nodename;
	}
	if (lldpctl_atom_set_str(config,
		lldpctl_k_config_hostname, value) == NULL) {
		log_warnx("lldpctl", "unable to set system name. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "system name set to new value %s",
	    value?value:"(none)");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_update_descriptions(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_ifdescr_update,
		arg?1:0) == NULL) {
		log_warnx("lldpctl", "unable to %s interface description update: %s",
		    arg?"enable":"disable",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "interface description update %s",
	    arg?"enabled":"disabled");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_bondslave_srcmac_type(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	char *value_str;
	int value = -1;

	log_debug("lldpctl", "bond slave src mac");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl",
			"unable to get configuration from lldpd. %s",
			lldpctl_last_strerror(conn));
		return 0;
	}

	value_str = arg;
	for (lldpctl_map_t *b_map =
		lldpctl_key_get_map(lldpctl_k_config_bond_slave_src_mac_type);
		b_map->string; b_map++) {
		if (!strcmp(b_map->string, value_str)) {
			value = b_map->value;
			break;
		}
	}

	if (value == -1) {
		log_warnx("lldpctl", "invalid value");
		lldpctl_atom_dec_ref(config);
		return 0;
	}

	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_bond_slave_src_mac_type, value) == NULL) {
		log_warnx("lldpctl", "unable to set bond slave src mac type."
			" %s", lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}

	log_info("lldpctl", "bond slave src mac set to new value: %s",
	    value_str);
	lldpctl_atom_dec_ref(config);

	return 1;
}

static int
cmd_maxneighs(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set maximum neighbors");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_str(config,
		lldpctl_k_config_max_neighbors, cmdenv_get(env, "max-neighbors")) == NULL) {
		log_warnx("lldpctl", "unable to set maximum of neighbors. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "maximum neighbors set to new value %s", cmdenv_get(env, "max-neighbors"));
	lldpctl_atom_dec_ref(config);
	return 1;
}

/**
 * Register `configure system bond-slave-src-mac-type`
 */
static void
register_commands_srcmac_type(struct cmd_node *configure)
{
	struct cmd_node *bond_slave_src_mac_type =
		commands_new(configure,
			"bond-slave-src-mac-type",
			"Set LLDP bond slave source MAC type",
			NULL, NULL, NULL);

	for (lldpctl_map_t *b_map =
		lldpctl_key_get_map(lldpctl_k_config_bond_slave_src_mac_type);
		b_map->string; b_map++) {
		if (!strcmp(b_map->string, "real")) {
			commands_new(
				commands_new(bond_slave_src_mac_type,
					b_map->string, "Real mac",
					NULL, NULL, NULL),
					NEWLINE, NULL,
					NULL, cmd_bondslave_srcmac_type,
					b_map->string);
		} else if (!strcmp(b_map->string, "zero")) {
			commands_new(
				commands_new(bond_slave_src_mac_type,
					b_map->string, "All zero mac",
					NULL, NULL, NULL),
					NEWLINE, NULL,
					NULL, cmd_bondslave_srcmac_type,
					b_map->string);
		} else if (!strcmp(b_map->string, "fixed")) {
			commands_new(
				commands_new(bond_slave_src_mac_type,
					b_map->string, "Fixed value (3Com card)",
					NULL, NULL, NULL),
					NEWLINE, NULL,
					NULL, cmd_bondslave_srcmac_type,
					b_map->string);
		} else if (!strcmp(b_map->string, "local")) {
			commands_new(
				commands_new(bond_slave_src_mac_type,
					b_map->string, "Real Mac with locally "
					"administered bit set",
					NULL, NULL, NULL),
					NEWLINE, NULL,
					NULL, cmd_bondslave_srcmac_type,
					b_map->string);
		}
	}
}

/**
 * Register `configure system` commands.
 *
 * Those are the commands to configure protocol-independant stuff.
 */
void
register_commands_configure_system(struct cmd_node *configure,
    struct cmd_node *unconfigure)
{
	struct cmd_node *configure_system = commands_new(
		configure,
		"system", "System configuration",
		cmd_check_no_env, NULL, "ports");
	struct cmd_node *unconfigure_system = commands_new(
		unconfigure,
		"system", "System configuration",
		cmd_check_no_env, NULL, "ports");
	struct cmd_node *configure_interface = commands_new(
		configure_system,
		"interface", "Interface related items",
		NULL, NULL, NULL);
	struct cmd_node *unconfigure_interface = commands_new(
		unconfigure_system,
		"interface", "Interface related items",
		NULL, NULL, NULL);

	commands_new(
		commands_new(
			commands_new(configure_system,
			    "description", "Override chassis description",
			    NULL, NULL, NULL),
			NULL, "Chassis description",
			NULL, cmd_store_env_value, "description"),
		NEWLINE, "Override chassis description",
		NULL, cmd_system_description, "system");
	commands_new(
		commands_new(unconfigure_system,
		    "description", "Don't override chassis description",
		    NULL, NULL, NULL),
		NEWLINE, "Don't override chassis description",
		NULL, cmd_system_description, "system");

	commands_new(
		commands_new(
			commands_new(configure_system,
			    "chassisid", "Override chassis ID",
			    NULL, NULL, NULL),
			NULL, "Chassis ID",
			NULL, cmd_store_env_value, "description"),
		NEWLINE, "Override chassis ID",
		NULL, cmd_system_chassisid, "system");
	commands_new(
		commands_new(unconfigure_system,
		    "chassisid", "Don't override chassis ID",
		    NULL, NULL, NULL),
		NEWLINE, "Don't override chassis ID",
		NULL, cmd_system_chassisid, "system");

	commands_new(
		commands_new(
			commands_new(configure_system,
			    "platform", "Override platform description",
			    NULL, NULL, NULL),
			NULL, "Platform description (CDP)",
			NULL, cmd_store_env_value, "platform"),
		NEWLINE, "Override platform description",
		NULL, cmd_system_description, "platform");
	commands_new(
		commands_new(unconfigure_system,
		    "platform", "Don't override platform description",
		    NULL, NULL, NULL),
		NEWLINE, "Don't override platform description",
		NULL, cmd_system_description, "platform");

	commands_new(
		commands_new(
			commands_new(configure_system,
			    "hostname", "Override system name",
			    NULL, NULL, NULL),
			NULL, "System name",
			NULL, cmd_store_env_value, "hostname"),
		NEWLINE, "Override system name",
		NULL, cmd_hostname, NULL);
	commands_new(
		commands_new(unconfigure_system,
		    "hostname", "Don't override system name",
		    NULL, NULL, NULL),
		NEWLINE, "Don't override system name",
		NULL, cmd_hostname, NULL);

        commands_new(
		commands_new(
			commands_new(configure_system,
			    "max-neighbors", "Set maximum number of neighbors per port",
			    cmd_check_no_env, NULL, "ports"),
			NULL, "Maximum number of neighbors",
			NULL, cmd_store_env_value, "max-neighbors"),
		NEWLINE, "Set maximum number of neighbors per port",
		NULL, cmd_maxneighs, NULL);

	commands_new(
		commands_new(
			commands_new(
				commands_new(
					commands_new(configure_system,
					    "ip", "IP related options",
					    NULL, NULL, NULL),
					"management", "IP management related options",
					NULL, NULL, NULL),
				"pattern", "Set IP management pattern",
				NULL, NULL, NULL),
			NULL, "IP management pattern (comma-separated list of wildcards)",
			NULL, cmd_store_env_value, "management-pattern"),
		NEWLINE, "Set IP management pattern",
		NULL, cmd_management, NULL);
	commands_new(
		commands_new(
			commands_new(
				commands_new(unconfigure_system,
				    "ip", "IP related options",
				    NULL, NULL, NULL),
				"management", "IP management related options",
				NULL, NULL, NULL),
			"pattern", "Delete any IP management pattern",
			NULL, NULL, NULL),
		NEWLINE, "Delete any IP management pattern",
		NULL, cmd_management, NULL);

        commands_new(
		commands_new(
			commands_new(configure_interface,
			    "pattern", "Set active interface pattern",
			    NULL, NULL, NULL),
			NULL, "Interface pattern (comma-separated list of wildcards)",
			NULL, cmd_store_env_value, "iface-pattern"),
		NEWLINE, "Set active interface pattern",
		NULL, cmd_iface_pattern, NULL);
        commands_new(
		commands_new(unconfigure_interface,
		    "pattern", "Delete any interface pattern",
		    NULL, NULL, NULL),
		NEWLINE, "Clear interface pattern",
		NULL, cmd_iface_pattern, NULL);

        commands_new(
		commands_new(
			commands_new(configure_interface,
			    "permanent", "Set permanent interface pattern",
			    NULL, NULL, NULL),
			NULL, "Permanent interface pattern (comma-separated list of wildcards)",
			NULL, cmd_store_env_value, "iface-pattern"),
		NEWLINE, "Set permanent interface pattern",
		NULL, cmd_perm_iface_pattern, NULL);
        commands_new(
		commands_new(unconfigure_interface,
		    "permanent", "Clear permanent interface pattern",
		    NULL, NULL, NULL),
		NEWLINE, "Delete any interface pattern",
		NULL, cmd_perm_iface_pattern, NULL);

	commands_new(
		commands_new(configure_interface,
		    "description", "Update interface descriptions with neighbor name",
		    NULL, NULL, NULL),
		NEWLINE, "Update interface descriptions with neighbor name",
		NULL, cmd_update_descriptions, "enable");
	commands_new(
		commands_new(unconfigure_interface,
		    "description", "Don't update interface descriptions with neighbor name",
		    NULL, NULL, NULL),
		NEWLINE, "Don't update interface descriptions with neighbor name",
		NULL, cmd_update_descriptions, NULL);

	commands_new(
		commands_new(configure_interface,
		    "promiscuous", "Enable promiscuous mode on managed interfaces",
		    NULL, NULL, NULL),
		NEWLINE, "Enable promiscuous mode on managed interfaces",
		NULL, cmd_iface_promisc, "enable");
	commands_new(
		commands_new(unconfigure_interface,
		    "promiscuous", "Don't enable promiscuous mode on managed interfaces",
		    NULL, NULL, NULL),
		NEWLINE, "Don't enable promiscuous mode on managed interfaces",
		NULL, cmd_iface_promisc, NULL);

	register_commands_srcmac_type(configure_system);
}

