/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

#include "lldpd.h"
#include "trace.h"

static ssize_t
client_handle_none(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	log_info("rpc", "received noop request from client");
	*type = NONE;
	return 0;
}

/* Return the global configuration */
static ssize_t
client_handle_get_configuration(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	ssize_t output_len;
	log_debug("rpc", "client requested configuration");
	output_len = lldpd_config_serialize(&cfg->g_config, output);
	if (output_len <= 0) {
		output_len = 0;
		*type = NONE;
	}
	return output_len;
}

static char*
xstrdup(const char *str)
{
	if (!str) return NULL;
	return strdup(str);
}

/* Change the global configuration */
static ssize_t
client_handle_set_configuration(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	struct lldpd_config *config;

	log_debug("rpc", "client request a change in configuration");
	/* Get the proposed configuration. */
	if (lldpd_config_unserialize(input, input_len, &config) <= 0) {
		*type = NONE;
		return 0;
	}

#define CHANGED(w) (config->w != cfg->g_config.w)
#define CHANGED_STR(w) (!(config->w == cfg->g_config.w ||	\
		(config->w && cfg->g_config.w && !strcmp(config->w, cfg->g_config.w))))

	/* What needs to be done? Transmit delay? */
	if (CHANGED(c_tx_interval) && config->c_tx_interval != 0) {
		if (config->c_tx_interval < 0) {
			log_debug("rpc", "client asked for immediate retransmission");
			levent_send_now(cfg);
		} else {
			log_debug("rpc", "client change transmit interval to %d",
			    config->c_tx_interval);
			cfg->g_config.c_tx_interval = config->c_tx_interval;
			cfg->g_config.c_ttl = cfg->g_config.c_tx_interval *
			    cfg->g_config.c_tx_hold;
		}
	}
	if (CHANGED(c_tx_hold) && config->c_tx_hold > 0) {
		log_debug("rpc", "client change transmit hold to %d",
		    config->c_tx_hold);
		cfg->g_config.c_tx_hold = config->c_tx_hold;
		cfg->g_config.c_ttl = cfg->g_config.c_tx_interval *
		    cfg->g_config.c_tx_hold;
	}
	if (CHANGED(c_lldp_portid_type) &&
	    config->c_lldp_portid_type > LLDP_PORTID_SUBTYPE_UNKNOWN &&
	    config->c_lldp_portid_type <= LLDP_PORTID_SUBTYPE_MAX) {
		log_debug("rpc", "change lldp portid tlv subtype to %d",
		    config->c_lldp_portid_type);
		cfg->g_config.c_lldp_portid_type = config->c_lldp_portid_type;
		levent_update_now(cfg);
	}
	if (CHANGED(c_lldp_agent_type) &&
	    config->c_lldp_agent_type > LLDP_AGENT_TYPE_UNKNOWN &&
	    config->c_lldp_agent_type <= LLDP_AGENT_TYPE_MAX) {
		log_debug("rpc", "change lldp agent type to %d",
		    config->c_lldp_agent_type);
		cfg->g_config.c_lldp_agent_type = config->c_lldp_agent_type;
		levent_update_now(cfg);
	}
	/* Pause/resume */
	if (CHANGED(c_paused)) {
		log_debug("rpc", "client asked to %s lldpd",
		    config->c_paused?"pause":"resume");
		cfg->g_config.c_paused = config->c_paused;
		levent_send_now(cfg);
	}

#ifdef ENABLE_LLDPMED
	if (CHANGED(c_enable_fast_start)) {
		cfg->g_config.c_enable_fast_start = config->c_enable_fast_start;
		log_debug("rpc", "client asked to %s fast start",
		    cfg->g_config.c_enable_fast_start?"enable":"disable");
	}
	if (CHANGED(c_tx_fast_interval) &&
	    config->c_tx_fast_interval > 0) {
		log_debug("rpc", "change fast interval to %d", config->c_tx_fast_interval);
		cfg->g_config.c_tx_fast_interval = config->c_tx_fast_interval;
	}
#endif
	if (CHANGED_STR(c_iface_pattern)) {
		log_debug("rpc", "change interface pattern to %s",
		    config->c_iface_pattern?config->c_iface_pattern:"(NULL)");
		free(cfg->g_config.c_iface_pattern);
		cfg->g_config.c_iface_pattern = xstrdup(config->c_iface_pattern);
		levent_update_now(cfg);
	}
	if (CHANGED_STR(c_mgmt_pattern)) {
		log_debug("rpc", "change management pattern to %s",
		    config->c_mgmt_pattern?config->c_mgmt_pattern:"(NULL)");
		free(cfg->g_config.c_mgmt_pattern);
		cfg->g_config.c_mgmt_pattern = xstrdup(config->c_mgmt_pattern);
		levent_update_now(cfg);
	}
	if (CHANGED_STR(c_description)) {
		log_debug("rpc", "change chassis description to %s",
		    config->c_description?config->c_description:"(NULL)");
		free(cfg->g_config.c_description);
		cfg->g_config.c_description = xstrdup(config->c_description);
		levent_update_now(cfg);
	}
	if (CHANGED_STR(c_platform)) {
		log_debug("rpc", "change platform description to %s",
		    config->c_platform?config->c_platform:"(NULL)");
		free(cfg->g_config.c_platform);
		cfg->g_config.c_platform = xstrdup(config->c_platform);
		levent_update_now(cfg);
	}
	if (CHANGED_STR(c_hostname)) {
		log_debug("rpc", "change system name to %s",
		    config->c_hostname?config->c_hostname:"(NULL)");
		free(cfg->g_config.c_hostname);
		cfg->g_config.c_hostname = xstrdup(config->c_hostname);
		levent_update_now(cfg);
	}
	if (CHANGED(c_set_ifdescr)) {
		log_debug("rpc", "%s setting of interface description based on discovered neighbors",
		    config->c_set_ifdescr?"enable":"disable");
		cfg->g_config.c_set_ifdescr = config->c_set_ifdescr;
		levent_update_now(cfg);
	}
	if (CHANGED(c_promisc)) {
		log_debug("rpc", "%s promiscuous mode on managed interfaces",
		    config->c_promisc?"enable":"disable");
		cfg->g_config.c_promisc = config->c_promisc;
		levent_update_now(cfg);
	}
	if (CHANGED(c_cap_advertise)) {
		log_debug("rpc", "%s chassis capabilities advertisement",
		    config->c_cap_advertise?"enable":"disable");
		cfg->g_config.c_cap_advertise = config->c_cap_advertise;
		levent_update_now(cfg);
	}
	if (CHANGED(c_mgmt_advertise)) {
		log_debug("rpc", "%s management addresses advertisement",
		    config->c_mgmt_advertise?"enable":"disable");
		cfg->g_config.c_mgmt_advertise = config->c_mgmt_advertise;
		levent_update_now(cfg);
	}
	if (CHANGED(c_bond_slave_src_mac_type)) {
		if (config->c_bond_slave_src_mac_type >
		    LLDP_BOND_SLAVE_SRC_MAC_TYPE_UNKNOWN &&
		    config->c_bond_slave_src_mac_type <=
		    LLDP_BOND_SLAVE_SRC_MAC_TYPE_MAX) {
			log_debug("rpc", "change bond src mac type to %d",
			    config->c_bond_slave_src_mac_type);
			cfg->g_config.c_bond_slave_src_mac_type =
			    config->c_bond_slave_src_mac_type;
		} else {
			log_info("rpc", "Invalid bond slave src mac type: %d\n",
			    config->c_bond_slave_src_mac_type);
		}
	}

	lldpd_config_cleanup(config);
	free(config);

	return 0;
}

/* Return the list of interfaces.
   Input:  nothing.
   Output: list of interface names (lldpd_interface_list)
*/
static ssize_t
client_handle_get_interfaces(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	struct lldpd_interface *iff, *iff_next;
	struct lldpd_hardware *hardware;
	ssize_t output_len;

	/* Build the list of interfaces */
	struct lldpd_interface_list ifs;

	log_debug("rpc", "client request the list of interfaces");
	TAILQ_INIT(&ifs);
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if ((iff = (struct lldpd_interface*)malloc(sizeof(
			    struct lldpd_interface))) == NULL)
			fatal("rpc", NULL);
		iff->name = hardware->h_ifname;
		TAILQ_INSERT_TAIL(&ifs, iff, next);
	}

	output_len = lldpd_interface_list_serialize(&ifs, output);
	if (output_len <= 0) {
		output_len = 0;
		*type = NONE;
	}

	/* Free the temporary list */
	for (iff = TAILQ_FIRST(&ifs);
	    iff != NULL;
	    iff = iff_next) {
		iff_next = TAILQ_NEXT(iff, next);
		TAILQ_REMOVE(&ifs, iff, next);
		free(iff);
	}

	return output_len;
}

/* Return the local chassis.
   Input:  nothing.
   Output: local chassis (lldpd_chassis)
*/
static ssize_t
client_handle_get_local_chassis(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	struct lldpd_chassis *chassis = LOCAL_CHASSIS(cfg);
	ssize_t output_len;

	log_debug("rpc", "client request the local chassis");
	output_len = lldpd_chassis_serialize(chassis, output);
	if (output_len <= 0) {
		output_len = 0;
		*type = NONE;
	}

	return output_len;
}

/* Return all available information related to an interface
   Input:  name of the interface (serialized)
   Output: Information about the interface (lldpd_hardware)
*/
static ssize_t
client_handle_get_interface(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	char *name;
	struct lldpd_hardware *hardware;
	void *p;

	/* Get name of the interface */
	if (marshal_unserialize(string, input, input_len, &p) <= 0) {
		*type = NONE;
		return 0;
	}
	name = p;

	/* Search appropriate hardware */
	log_debug("rpc", "client request interface %s", name);
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries)
		if (!strcmp(hardware->h_ifname, name)) {
			ssize_t output_len = lldpd_hardware_serialize(hardware, output);
			free(name);
			if (output_len <= 0) {
				*type = NONE;
				return 0;
			}
			return output_len;
		}

	log_warnx("rpc", "no interface %s found", name);
	free(name);
	*type = NONE;
	return 0;
}

/* Return all available information related to an interface
   Input:  name of the interface (serialized)
   Output: Information about the interface (lldpd_hardware)
*/
static ssize_t
client_handle_get_default_port(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	log_debug("rpc", "client request the default local port");
	ssize_t output_len = lldpd_port_serialize(cfg->g_default_local_port, output);
	if (output_len <= 0) {
		*type = NONE;
		return 0;
	}
	return output_len;
}

static int
_client_handle_set_port(struct lldpd *cfg,
    struct lldpd_port *port, struct lldpd_port_set *set)
{
#ifdef ENABLE_LLDPMED
	struct lldpd_med_loc *loc = NULL;
#endif
	if (set->local_id) {
		log_debug("rpc", "requested change to Port ID");
		free(port->p_id);
		port->p_id = strdup(set->local_id);
		port->p_id_len = strlen(set->local_id);
		port->p_id_subtype = LLDP_PORTID_SUBTYPE_LOCAL;
		port->p_descr_force = 0;
	}
	if (set->local_descr) {
		log_debug("rpc", "requested change to Port Description");
		free(port->p_descr);
		port->p_descr = strdup(set->local_descr);
		port->p_descr_force = 1;
	}
	switch (set->rxtx) {
	case LLDPD_RXTX_TXONLY:
		log_debug("rpc", "requested TX only mode");
		port->p_disable_rx = 1;
		port->p_disable_tx = 0;
		break;
	case LLDPD_RXTX_RXONLY:
		log_debug("rpc", "requested RX only mode");
		port->p_disable_rx = 0;
		port->p_disable_tx = 1;
		break;
	case LLDPD_RXTX_BOTH:
		log_debug("rpc", "requested RX/TX mode");
		port->p_disable_rx = port->p_disable_tx = 0;
		break;
	case LLDPD_RXTX_DISABLED:
		log_debug("rpc", "requested disabled mode");
		port->p_disable_rx = port->p_disable_tx = 1;
		break;
	}
#ifdef ENABLE_LLDPMED
	if (set->med_policy && set->med_policy->type > 0) {
		log_debug("rpc", "requested change to MED policy");
		if (set->med_policy->type > LLDP_MED_APPTYPE_LAST) {
			log_warnx("rpc", "invalid policy provided: %d",
			    set->med_policy->type);
			return -1;
		}
		memcpy(&port->p_med_policy[set->med_policy->type - 1],
		    set->med_policy, sizeof(struct lldpd_med_policy));
		port->p_med_cap_enabled |= LLDP_MED_CAP_POLICY;
	}
	if (set->med_location && set->med_location->format > 0) {
		char *newdata = NULL;
		log_debug("rpc", "requested change to MED location");
		if (set->med_location->format > LLDP_MED_LOCFORMAT_LAST) {
			log_warnx("rpc", "invalid location format provided: %d",
			    set->med_location->format);
			return -1;
		}
		loc = \
		    &port->p_med_location[set->med_location->format - 1];
		free(loc->data);
		memcpy(loc, set->med_location, sizeof(struct lldpd_med_loc));
		if (!loc->data || !(newdata = malloc(loc->data_len))) loc->data_len = 0;
		if (newdata) memcpy(newdata, loc->data, loc->data_len);
		loc->data = newdata;
		port->p_med_cap_enabled |= LLDP_MED_CAP_LOCATION;
	}
	if (set->med_power) {
		log_debug("rpc", "requested change to MED power");
		memcpy(&port->p_med_power, set->med_power,
		    sizeof(struct lldpd_med_power));
		switch (set->med_power->devicetype) {
		case LLDP_MED_POW_TYPE_PD:
			port->p_med_cap_enabled |= LLDP_MED_CAP_MDI_PD;
			port->p_med_cap_enabled &= ~LLDP_MED_CAP_MDI_PSE;
			break;
		case LLDP_MED_POW_TYPE_PSE:
			port->p_med_cap_enabled |= LLDP_MED_CAP_MDI_PSE;
			port->p_med_cap_enabled &= ~LLDP_MED_CAP_MDI_PD;
			break;
		}
	}
#endif
#ifdef ENABLE_DOT3
	if (set->dot3_power) {
		log_debug("rpc", "requested change to Dot3 power");
		memcpy(&port->p_power, set->dot3_power,
		    sizeof(struct lldpd_dot3_power));
	}
#endif
#ifdef ENABLE_CUSTOM
	if (set->custom_list_clear) {
		log_debug("rpc", "requested custom TLVs clear");
		lldpd_custom_list_cleanup(port);
	} else {
		if (set->custom) {
			log_info("rpc", "custom TLV op %s oui %02x:%02x:%02x subtype %x",
			    (set->custom_tlv_op == CUSTOM_TLV_REMOVE)?"remove":
			    (set->custom_tlv_op == CUSTOM_TLV_ADD)?"add":
			    "replace",
			    set->custom->oui[0],
			    set->custom->oui[1],
			    set->custom->oui[2],
			    set->custom->subtype);
			switch (set->custom_tlv_op) {
			case CUSTOM_TLV_REMOVE:
				lldpd_custom_tlv_cleanup(port, set->custom);
				break;
			case CUSTOM_TLV_ADD:
				lldpd_custom_tlv_add(port, set->custom);
				break;
			case CUSTOM_TLV_REPLACE:
			default:
				lldpd_custom_tlv_cleanup(port, set->custom);
				lldpd_custom_tlv_add(port, set->custom);
				break;
			}
		}
	}
#endif
	return 0;
}

/* Set some port related settings (policy, location, power)
   Input: name of the interface, policy/location/power setting to be modified
   Output: nothing
*/
static ssize_t
client_handle_set_port(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	int ret = 0;
	struct lldpd_port_set *set = NULL;
	struct lldpd_hardware *hardware = NULL;

	if (lldpd_port_set_unserialize(input, input_len, &set) <= 0) {
		*type = NONE;
		return 0;
	}
	if (!set->ifname) {
		log_warnx("rpc", "no interface provided");
		goto set_port_finished;
	}

	/* Search the appropriate hardware */
	if (strlen(set->ifname) == 0) {
		log_debug("rpc", "client request change to default port");
		if (_client_handle_set_port(cfg, cfg->g_default_local_port, set) == -1)
			goto set_port_finished;
		ret = 1;
	} else {
		log_debug("rpc", "client request change to port %s", set->ifname);
		TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		    if (!strcmp(hardware->h_ifname, set->ifname)) {
			    struct lldpd_port *port = &hardware->h_lport;
			    if (_client_handle_set_port(cfg, port, set) == -1)
				    goto set_port_finished;
			    ret = 1;
			    break;
		    }
		}
	}

	if (ret == 0)
		log_warn("rpc", "no interface %s found", set->ifname);
	else
	    levent_update_now(cfg);

set_port_finished:
	if (!ret) *type = NONE;
	free(set->ifname);
	free(set->local_id);
	free(set->local_descr);
#ifdef ENABLE_LLDPMED
	free(set->med_policy);
	if (set->med_location) free(set->med_location->data);
	free(set->med_location);
	free(set->med_power);
#endif
#ifdef ENABLE_DOT3
	free(set->dot3_power);
#endif
#ifdef ENABLE_CUSTOM
	if (set->custom) {
		free(set->custom->oui_info);
		free(set->custom);
	}
#endif
	free(set);
	return 0;
}

/* Register subscribtion to neighbor changes */
static ssize_t
client_handle_subscribe(struct lldpd *cfg, enum hmsg_type *type,
    void *input, int input_len, void **output, int *subscribed)
{
	log_debug("rpc", "client subscribe to changes");
	*subscribed = 1;
	return 0;
}

struct client_handle {
	enum hmsg_type type;
	const char *name;
	ssize_t (*handle)(struct lldpd*, enum hmsg_type *,
	    void *, int, void **, int *);
};

static struct client_handle client_handles[] = {
	{ NONE,			"None",              client_handle_none },
	{ GET_CONFIG,		"Get configuration", client_handle_get_configuration },
	{ SET_CONFIG,		"Set configuration", client_handle_set_configuration },
	{ GET_INTERFACES,	"Get interfaces",    client_handle_get_interfaces },
	{ GET_INTERFACE,	"Get interface",     client_handle_get_interface },
	{ GET_DEFAULT_PORT,	"Get default port",  client_handle_get_default_port },
	{ GET_CHASSIS,		"Get local chassis", client_handle_get_local_chassis },
	{ SET_PORT,		"Set port",          client_handle_set_port },
	{ SUBSCRIBE,		"Subscribe",         client_handle_subscribe },
	{ 0,			NULL } };

int
client_handle_client(struct lldpd *cfg,
    ssize_t(*send)(void *, int, void *, size_t),
    void *out,
    enum hmsg_type type, void *buffer, size_t n,
    int *subscribed)
{
	struct client_handle *ch;
	void *answer; ssize_t len, sent;

	log_debug("rpc", "handle client request");
	for (ch = client_handles; ch->handle != NULL; ch++) {
		if (ch->type == type) {
			TRACE(LLDPD_CLIENT_REQUEST(ch->name));
			answer = NULL;
			len  = ch->handle(cfg, &type, buffer, n, &answer,
			    subscribed);
			sent = send(out, type, answer, len);
			free(answer);
			return sent;
		}
	}

	log_warnx("rpc", "unknown message request (%d) received",
	    type);
	return -1;
}
