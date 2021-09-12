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
#include <limits.h>

#include "client.h"
#include "../log.h"

static int
cmd_txdelay(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	const char *interval;
	char interval_ms[8]; /* less than 2.5 hours */
	lldpctl_key_t key;
	int arglen;

	log_debug("lldpctl", "set transmit delay");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	interval = cmdenv_get(env, "tx-interval");
	key = lldpctl_k_config_tx_interval;
	/* interval is either <number> for seconds or <number>ms for milliseconds */
	if (interval) {
		arglen = strlen(interval);
		/* room for "ms" in interval, room for interval in interval_ms */
		if (arglen >= 2 && arglen-2 < sizeof(interval_ms) &&
				strcmp("ms", interval+arglen-2) == 0) {
			/* remove "ms" suffix */
			memcpy(interval_ms, interval, arglen-2);
			interval_ms[arglen-2] = '\0';
			/* substitute key and value */
			key = lldpctl_k_config_tx_interval_ms;
			interval = interval_ms;
		}
	}
	if (lldpctl_atom_set_str(config, key, interval) == NULL) {
		log_warnx("lldpctl", "unable to set transmit delay. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "transmit delay set to new value");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_txhold(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "set transmit hold");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_str(config,
		lldpctl_k_config_tx_hold, cmdenv_get(env, "tx-hold")) == NULL) {
		log_warnx("lldpctl", "unable to set transmit hold. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "transmit hold set to new value %s", cmdenv_get(env, "tx-hold"));
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_status(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *port;
	const char *name;
	const char *status = cmdenv_get(env, "status");

	log_debug("lldpctl", "lldp administrative port status set to '%s'", status);

	if (!status || !strlen(status)) {
		log_warnx("lldpctl", "no status specified");
		return 0;
	}

	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		if (lldpctl_atom_set_str(port, lldpctl_k_port_status, status) == NULL) {
			log_warnx("lldpctl", "unable to set LLDP status for %s."
			    " %s", name, lldpctl_last_strerror(conn));
		}
	}

	return 1;
}

static int
cmd_agent_type(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	const char *str = arg;
	int value = -1;

	log_debug("lldpctl", "set agent type to '%s'", str);

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl",
			  "unable to get configuration from lldpd. %s",
			  lldpctl_last_strerror(conn));
		return 0;
	}

	for (lldpctl_map_t *b_map =
		     lldpctl_key_get_map(lldpctl_k_config_lldp_agent_type);
	     b_map->string; b_map++) {
		if (!strcmp(b_map->string, str)) {
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
				 lldpctl_k_config_lldp_agent_type, value) == NULL) {
		log_warnx("lldpctl", "unable to set LLDP agent type."
			  " %s", lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}

	log_info("lldpctl", "agent type set to new value : %s", str);
	lldpctl_atom_dec_ref(config);

	return 1;
}

static int
cmd_portid_type_local(struct lldpctl_conn_t *conn, struct writer *w,
		struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *port;
	const char *name;
	const char *id = cmdenv_get(env, "port-id");
	const char *descr = cmdenv_get(env, "port-descr");

	log_debug("lldpctl", "lldp PortID TLV Subtype Local port-id '%s' port-descr '%s'", id, descr);

	if (!id || !strlen(id)) {
		log_warnx("lldpctl", "no id specified");
		return 0;
	}

	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		if (lldpctl_atom_set_str(port, lldpctl_k_port_id, id) == NULL) {
			log_warnx("lldpctl", "unable to set LLDP PortID for %s."
			    " %s", name, lldpctl_last_strerror(conn));
		}
		if (descr && lldpctl_atom_set_str(port, lldpctl_k_port_descr, descr) == NULL) {
			log_warnx("lldpctl", "unable to set LLDP Port Description for %s."
			    " %s", name, lldpctl_last_strerror(conn));
		}
	}

	return 1;
}

static int
cmd_port_descr(struct lldpctl_conn_t *conn, struct writer *w,
		struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *port;
	const char *name;
	const char *descr = cmdenv_get(env, "port-descr");

	log_debug("lldpctl", "lldp port-descr '%s'", descr);

	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		if (descr && lldpctl_atom_set_str(port, lldpctl_k_port_descr, descr) == NULL) {
			log_warnx("lldpctl", "unable to set LLDP Port Description for %s."
			    " %s", name, lldpctl_last_strerror(conn));
		}
	}

	return 1;
}

static int
cmd_portid_type(struct lldpctl_conn_t *conn, struct writer *w,
		struct cmd_env *env, void *arg)
{
	char *value_str;
	int value = -1;

	log_debug("lldpctl", "lldp PortID TLV Subtype");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl",
			  "unable to get configuration from lldpd. %s",
			  lldpctl_last_strerror(conn));
		return 0;
	}

	value_str = arg;
	for (lldpctl_map_t *b_map =
		     lldpctl_key_get_map(lldpctl_k_config_lldp_portid_type);
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
				 lldpctl_k_config_lldp_portid_type, value) == NULL) {
		log_warnx("lldpctl", "unable to set LLDP PortID type."
			  " %s", lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}

	log_info("lldpctl", "LLDP PortID TLV type set to new value : %s", value_str);
	lldpctl_atom_dec_ref(config);

	return 1;
}

static int
cmd_chassis_cap_advertise(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "lldp capabilities-advertisements %s", arg?"enable":"disable");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_chassis_cap_advertise,
		arg?1:0) == NULL) {
		log_warnx("lldpctl", "unable to %s chassis capabilities advertisement: %s",
		    arg?"enable":"disable",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "chassis capabilities advertisement %s",
	    arg?"enabled":"disabled");
	lldpctl_atom_dec_ref(config);
	return 1;
}

/* FIXME: see about compressing this with other functions */
static int
cmd_chassis_mgmt_advertise(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_debug("lldpctl", "lldp management-addresses-advertisements %s", arg?"enable":"disable");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_chassis_mgmt_advertise,
		arg?1:0) == NULL) {
		log_warnx("lldpctl", "unable to %s management addresses advertisement: %s",
		    arg?"enable":"disable",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "management addresses advertisement %s",
	    arg?"enabled":"disabled");
	lldpctl_atom_dec_ref(config);
	return 1;
}

static int
cmd_vlan_tx(struct lldpctl_conn_t *conn, struct writer *w,
		struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *port;
	const char *name;
	const char *vlan_id = cmdenv_get(env, "vlan-tx-id");
	const char *vlan_prio = cmdenv_get(env, "vlan-tx-prio");
	const char *vlan_dei = cmdenv_get(env, "vlan-tx-dei");

	/* Default values are used to disable VLAN */
	int vlan_id_int = -1;
	int vlan_prio_int = -1;
	int vlan_dei_int = -1;
	int vlan_tag = -1;

	if (!arg)
		log_debug("lldpctl", "lldp disable VLAN tagging of transmitted LLDP frames");
	else
		log_debug("lldpctl", "lldp enable VLAN tagging of transmitted LLDP frames with VLAN ID: '%s', Priority: '%s', DEI: '%s'",
				  vlan_id, vlan_prio, vlan_dei);

	if (arg) {
		if (!vlan_id || !strlen(vlan_id)) {
			log_warnx("lldpctl", "no VLAN id for TX specified");
			return 0;
		} else {
			const char *errstr;
			vlan_id_int = strtonum(vlan_id, 0, 4094, &errstr);
			if (errstr != NULL) {
				log_warnx("lldpctl", "invalid VLAN ID for TX `%s': %s",
					vlan_id, errstr);
				return 0;
			}
		}

		if (!vlan_prio || !strlen(vlan_prio)) {
			log_warnx("lldpctl", "no VLAN priority for TX specified, using default (0)");
			/* Use default priority */
			vlan_prio_int = 0;
		} else {
			const char *errstr;
			vlan_prio_int = strtonum(vlan_prio, 0, 7, &errstr);
			if (errstr != NULL) {
				log_warnx("lldpctl", "invalid VLAN piority `%s': %s",
					vlan_prio, errstr);
				return 0;
			}
		}

		if (!vlan_dei || !strlen(vlan_dei)) {
			log_warnx("lldpctl", "no VLAN Drop eligible indicator (DEI) for TX specified, using default (0)");
			/* Use default priority */
			vlan_dei_int = 0;
		} else {
			const char *errstr;
			vlan_dei_int = strtonum(vlan_dei, 0, 1, &errstr);
			if (errstr != NULL) {
				log_warnx("lldpctl", "invalid VLAN Drop eligible indicator (DEI) `%s': %s",
					vlan_dei, errstr);
				return 0;
			}
		}
		/* Priority(3bits) | DEI(1bit) | VID(12bits) */
		vlan_tag = ((vlan_prio_int & 0x7) << 13) |
			   ((vlan_dei_int & 0x1) << 12) |
			   (vlan_id_int & 0xfff);
	}

	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		if (lldpctl_atom_set_int(port, lldpctl_k_port_vlan_tx, vlan_tag) == NULL) {
			log_warnx("lldpctl", "unable to set VLAN TX config on %s."
			    " %s", name, lldpctl_last_strerror(conn));
		}
	}

	return 1;
}

#ifdef ENABLE_CUSTOM
static int
cmd_custom_tlv_set(struct lldpctl_conn_t *conn, struct writer *w,
        struct cmd_env *env, void *arg)
{
	lldpctl_atom_t *port;
	const char *s;
	const char *name;
	uint8_t oui[LLDP_TLV_ORG_OUI_LEN];
	uint8_t oui_info[LLDP_TLV_ORG_OUI_INFO_MAXLEN];
	int oui_info_len = 0;
	uint16_t subtype = 0;
	char *op = "add";

	if (!arg || !strcmp(arg, "remove"))
		op = "remove";

	log_debug("lldpctl", "lldp custom-tlv(s) %s", op);

	if (!arg)
		goto set;

	s = cmdenv_get(env, "oui");
	if (!s || (
	    sscanf(s, "%02hhx,%02hhx,%02hhx", &oui[0], &oui[1], &oui[2]) != 3 &&
	    sscanf(s, "%02hhX,%02hhX,%02hhX", &oui[0], &oui[1], &oui[2]) != 3) ) {
		log_warnx("lldpctl", "invalid OUI value '%s'", s);
		return 0;
	}

	s = cmdenv_get(env, "subtype");
	if (!s) {
		log_warnx("lldpctl", "no subtype specified");
		return 0;
	} else {
		const char *errstr;
		subtype = strtonum(s, 0, 255, &errstr);
		if (errstr != NULL) {
			log_warnx("lldpctl", "invalid subtype value `%s': %s",
			    s, errstr);
			return 0;
		}
	}

	s = cmdenv_get(env, "oui-info");
	/* This info is optional */
	if (s) {
		const char delim[] = ",";
		char *s_copy = strdup(s);
		char *token = strtok(s_copy, delim);
		while (token != NULL) {
			if (sscanf(token, "%02hhx", &oui_info[oui_info_len]) == 1 ||
			    sscanf(token, "%02hhX", &oui_info[oui_info_len]) == 1)
				oui_info_len++;
			if (oui_info_len >= sizeof(oui_info))
				break;
			token = strtok(NULL, delim);
		}
		free(s_copy);
	}

	s = cmdenv_get(env, "replace");
	/* This info is optional */
	if (s) op = "replace";

set:
	while ((port = cmd_iterate_on_ports(conn, env, &name))) {
		lldpctl_atom_t *custom_tlvs;
		if (!arg) {
			lldpctl_atom_set(port, lldpctl_k_custom_tlvs_clear, NULL);
		} else if (!(custom_tlvs = lldpctl_atom_get(port, lldpctl_k_custom_tlvs))) {
			log_warnx("lldpctl", "unable to get custom TLVs for port %s", name);
		} else {
			lldpctl_atom_t *tlv = lldpctl_atom_create(custom_tlvs);
			if (!tlv) {
				log_warnx("lldpctl", "unable to create new custom TLV for port %s",
				    name);
			} else {
				/* Configure custom TLV */
				lldpctl_atom_set_buffer(tlv, lldpctl_k_custom_tlv_oui, oui, sizeof(oui));
				lldpctl_atom_set_int(tlv, lldpctl_k_custom_tlv_oui_subtype, subtype);
				lldpctl_atom_set_buffer(tlv, lldpctl_k_custom_tlv_oui_info_string, oui_info, oui_info_len);
				lldpctl_atom_set_str(tlv, lldpctl_k_custom_tlv_op, op);

				/* Assign it to port */
				lldpctl_atom_set(port, lldpctl_k_custom_tlv, tlv);

				lldpctl_atom_dec_ref(tlv);
			}
			lldpctl_atom_dec_ref(custom_tlvs);
		}
	}

	return 1;
}

static int
cmd_check_no_add_env(struct cmd_env *env, void *arg)
{
	const char *what = arg;
	if (cmdenv_get(env, "add")) return 0;
	if (cmdenv_get(env, what)) return 0;
	return 1;
}

static int
cmd_check_no_replace_env(struct cmd_env *env, void *arg)
{
	const char *what = arg;
	if (cmdenv_get(env, "replace")) return 0;
	if (cmdenv_get(env, what)) return 0;
	return 1;
}

void
register_commands_configure_lldp_custom_tlvs(struct cmd_node *configure_lldp,
					     struct cmd_node *unconfigure_lldp)
{
	struct cmd_node *configure_custom_tlvs;
	struct cmd_node *unconfigure_custom_tlvs;
	struct cmd_node *configure_custom_tlvs_basic;
	struct cmd_node *unconfigure_custom_tlvs_basic;

	configure_custom_tlvs =
		commands_new(configure_lldp,
			    "custom-tlv",
			    "Add custom TLV(s) to be broadcast on ports",
			    NULL, NULL, NULL);

	unconfigure_custom_tlvs =
		commands_new(unconfigure_lldp,
			    "custom-tlv",
			    "Remove ALL custom TLV(s)",
			    NULL, NULL, NULL);

	commands_new(unconfigure_custom_tlvs,
		NEWLINE, "Remove ALL custom TLV",
		NULL, cmd_custom_tlv_set, NULL);

	commands_new(configure_custom_tlvs,
			"add", "Add custom TLV",
			cmd_check_no_replace_env, cmd_store_env_and_pop, "add");
	commands_new(configure_custom_tlvs,
			"replace", "Replace custom TLV",
			cmd_check_no_add_env, cmd_store_env_and_pop, "replace");

	/* Basic form: 'configure lldp custom-tlv oui 11,22,33 subtype 44' */
	configure_custom_tlvs_basic = 
		commands_new(
			commands_new(
				commands_new(
					commands_new(configure_custom_tlvs,
						"oui", "Organizationally Unique Identifier",
						NULL, NULL, NULL),
					NULL, "Organizationally Unique Identifier",
					NULL, cmd_store_env_value, "oui"),
				"subtype", "Organizationally Defined Subtype",
				NULL, NULL, NULL),
			NULL, "Organizationally Defined Subtype",
			NULL, cmd_store_env_value, "subtype");

	commands_new(configure_custom_tlvs_basic,
		NEWLINE, "Add custom TLV(s) to be broadcast on ports",
		NULL, cmd_custom_tlv_set, "enable");

	/* Basic form: 'unconfigure lldp custom-tlv oui 11,22,33 subtype 44' */
	unconfigure_custom_tlvs_basic =
		commands_new(
			commands_new(
				commands_new(
					commands_new(unconfigure_custom_tlvs,
						"oui", "Organizationally Unique Identifier",
						NULL, NULL, NULL),
					NULL, "Organizationally Unique Identifier",
					NULL, cmd_store_env_value, "oui"),
				"subtype", "Organizationally Defined Subtype",
				NULL, NULL, NULL),
			NULL, "Organizationally Defined Subtype",
			NULL, cmd_store_env_value, "subtype");

	commands_new(unconfigure_custom_tlvs_basic,
		NEWLINE, "Remove specific custom TLV",
		NULL, cmd_custom_tlv_set, "remove");

	/* Extended form: 'configure custom-tlv lldp oui 11,22,33 subtype 44 oui-info 55,66,77,...' */
	commands_new(
		commands_new(
			commands_new(configure_custom_tlvs_basic,
				"oui-info", "Organizationally Unique Identifier",
				NULL, NULL, NULL),
			NULL, "OUI Info String", 
			NULL, cmd_store_env_value, "oui-info"),
		NEWLINE, "Add custom TLV(s) to be broadcast on ports",
		NULL, cmd_custom_tlv_set, "enable");
}
#endif /* ENABLE_CUSTOM */

static int
cmd_store_status_env_value(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *value)
{
	return cmd_store_something_env_value("status", env, value);
}

/**
 * Register `configure lldp` commands.
 *
 * Those are the commands that are related to the LLDP protocol but not
 * Dot1/Dot3/MED. Commands not related to LLDP should go in system instead.
 */
void
register_commands_configure_lldp(struct cmd_node *configure,
    struct cmd_node *unconfigure)
{
	struct cmd_node *configure_lldp = commands_new(
		configure,
		"lldp", "LLDP configuration",
		NULL, NULL, NULL);
	struct cmd_node *unconfigure_lldp = commands_new(
		unconfigure,
		"lldp", "LLDP configuration",
		NULL, NULL, NULL);

        commands_new(
		commands_new(
			commands_new(configure_lldp,
			    "tx-interval", "Set LLDP transmit delay",
			    cmd_check_no_env, NULL, "ports"),
			NULL, "LLDP transmit <delay> in seconds or <delay>ms in milliseconds",
			NULL, cmd_store_env_value, "tx-interval"),
		NEWLINE, "Set LLDP transmit delay",
		NULL, cmd_txdelay, NULL);

        commands_new(
		commands_new(
			commands_new(configure_lldp,
			    "tx-hold", "Set LLDP transmit hold",
			    cmd_check_no_env, NULL, "ports"),
			NULL, "LLDP transmit hold in seconds",
			NULL, cmd_store_env_value, "tx-hold"),
		NEWLINE, "Set LLDP transmit hold",
		NULL, cmd_txhold, NULL);

	struct cmd_node *status = commands_new(configure_lldp,
	    "status", "Set administrative status",
	    NULL, NULL, NULL);

	for (lldpctl_map_t *status_map =
		 lldpctl_key_get_map(lldpctl_k_port_status);
	     status_map->string;
	     status_map++) {
		const char *tag = strdup(totag(status_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			commands_new(status,
			    tag,
			    status_map->string,
			    NULL, cmd_store_status_env_value, status_map->string),
			NEWLINE, "Set port administrative status",
			NULL, cmd_status, NULL);
	}

	/* Configure the various agent type we can configure. */
	struct cmd_node *configure_lldp_agent_type = commands_new(
		configure_lldp,
		"agent-type",
		"LLDP agent type",
		NULL, NULL, NULL);
	for (lldpctl_map_t *b_map =
		 lldpctl_key_get_map(lldpctl_k_config_lldp_agent_type);
	     b_map->string; b_map++) {
		const char *tag = strdup(totag(b_map->string));
		SUPPRESS_LEAK(tag);
		commands_new(
			commands_new(configure_lldp_agent_type,
			    tag,
			    b_map->string,
			    NULL, NULL, NULL),
			NEWLINE, "Set LLDP agent type",
			NULL, cmd_agent_type, b_map->string);
	}

	/* Now handle the various portid subtypes we can configure. */
	struct cmd_node *configure_lldp_portid_type = commands_new(
		configure_lldp,
		"portidsubtype", "LLDP PortID TLV Subtype",
		NULL, NULL, NULL);

	for (lldpctl_map_t *b_map =
		 lldpctl_key_get_map(lldpctl_k_config_lldp_portid_type);
	     b_map->string; b_map++) {
		if (!strcmp(b_map->string, "ifname")) {
			commands_new(
				commands_new(configure_lldp_portid_type,
				    b_map->string, "Interface Name",
				    cmd_check_no_env, NULL, "ports"),
				NEWLINE, NULL,
				NULL, cmd_portid_type,
				b_map->string);
		} else if (!strcmp(b_map->string, "local")) {
			struct cmd_node *port_id = commands_new(
				commands_new(configure_lldp_portid_type,
					     b_map->string, "Local",
					     NULL, NULL, NULL),
				NULL, "Port ID",
				NULL, cmd_store_env_value, "port-id");
			commands_new(port_id,
				NEWLINE, "Set local port ID",
				NULL, cmd_portid_type_local,
				b_map->string);
			commands_new(
				commands_new(
					commands_new(port_id,
					    "description",
					    "Also set port description",
					    NULL, NULL, NULL),
					NULL, "Port description",
					NULL, cmd_store_env_value, "port-descr"),
				NEWLINE, "Set local port ID and description",
				NULL, cmd_portid_type_local, NULL);
		} else if (!strcmp(b_map->string, "macaddress")) {
			commands_new(
				commands_new(configure_lldp_portid_type,
				    b_map->string, "MAC Address",
				    cmd_check_no_env, NULL, "ports"),
				NEWLINE, NULL,
				NULL, cmd_portid_type,
				b_map->string);
		}
	}

	commands_new(
		commands_new(
			commands_new(configure_lldp,
			    "portdescription",
			    "Port Description",
			    NULL, NULL, NULL),
			NULL, "Port description",
			NULL, cmd_store_env_value, "port-descr"),
		NEWLINE, "Set port description",
		NULL, cmd_port_descr, NULL);

	commands_new(
		commands_new(configure_lldp,
		    "capabilities-advertisements",
		    "Enable chassis capabilities advertisement",
		    cmd_check_no_env, NULL, "ports"),
		NEWLINE, "Enable chassis capabilities advertisement",
		NULL, cmd_chassis_cap_advertise, "enable");
	commands_new(
		commands_new(unconfigure_lldp,
		    "capabilities-advertisements",
		    "Don't enable chassis capabilities advertisement",
		    NULL, NULL, NULL),
		NEWLINE, "Don't enable chassis capabilities advertisement",
		NULL, cmd_chassis_cap_advertise, NULL);

	commands_new(
		commands_new(configure_lldp,
		    "management-addresses-advertisements",
		    "Enable management addresses advertisement",
		    NULL, NULL, NULL),
		NEWLINE, "Enable management addresses advertisement",
		NULL, cmd_chassis_mgmt_advertise, "enable");
	commands_new(
		commands_new(unconfigure_lldp,
		    "management-addresses-advertisements",
		    "Don't enable management addresses advertisement",
		    NULL, NULL, NULL),
		NEWLINE, "Don't enable management addresses advertisement",
		NULL, cmd_chassis_mgmt_advertise, NULL);

	struct cmd_node *vlan_tx = commands_new(
		commands_new(configure_lldp,
		    "vlan-tx",
		    "Send LLDP frames with a VLAN tag",
		    NULL, NULL, NULL),
		NULL, "VLAN ID (0-4094)",
		NULL, cmd_store_env_value, "vlan-tx-id");

	struct cmd_node *vlan_tx_prio = commands_new(
		commands_new(vlan_tx,
			"priority",
			"Also set a priority in a VLAN tag (default 0)",
			NULL, NULL, NULL),
		NULL, "Priority to be included in a VLAN tag (0-7)",
		NULL, cmd_store_env_value, "vlan-tx-prio");

	commands_new(vlan_tx,
		NEWLINE, "Enable VLAN tagging of transmitted LLDP frames",
		NULL, cmd_vlan_tx,
		"enable");

	commands_new(
		vlan_tx_prio,
		NEWLINE, "Set VLAN ID and priority for transmitted frames",
		NULL, cmd_vlan_tx, "enable");

	commands_new(
		commands_new(
			commands_new(vlan_tx_prio,
				"dei",
				"Also set a Drop eligible indicator (DEI) in a VLAN tag (default 0)",
				NULL, NULL, NULL),
			NULL, "Drop eligible indicator (DEI) in a VLAN tag (0-don't drop; 1-drop)",
			NULL, cmd_store_env_value, "vlan-tx-dei"),
		NEWLINE, "Set VLAN ID, priority and DEI for transmitted frames",
		NULL, cmd_vlan_tx, "enable");

	commands_new(
		commands_new(unconfigure_lldp,
		    "vlan-tx",
		    "Send LLDP frames without VLAN tag",
		    NULL, NULL, NULL),
		NEWLINE, "Disable VLAN tagging of transmitted LLDP frames",
		NULL, cmd_vlan_tx, NULL);


#ifdef ENABLE_CUSTOM
	register_commands_configure_lldp_custom_tlvs(configure_lldp, unconfigure_lldp);
#endif
}
