// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ell/ell.h>

#include "mesh/mesh.h"
#include "mesh/error.h"
#include "mesh/dbus.h"
#include "mesh/agent.h"

typedef enum {
	MESH_AGENT_REQUEST_BLINK,
	MESH_AGENT_REQUEST_BEEP,
	MESH_AGENT_REQUEST_VIBRATE,
	MESH_AGENT_REQUEST_OUT_NUMERIC,
	MESH_AGENT_REQUEST_OUT_ALPHA,
	MESH_AGENT_REQUEST_PUSH,
	MESH_AGENT_REQUEST_TWIST,
	MESH_AGENT_REQUEST_IN_NUMERIC,
	MESH_AGENT_REQUEST_IN_ALPHA,
	MESH_AGENT_REQUEST_STATIC_OOB,
	MESH_AGENT_REQUEST_PRIVATE_KEY,
	MESH_AGENT_REQUEST_PUBLIC_KEY,
	MESH_AGENT_REQUEST_CAPABILITIES,
} agent_request_type_t;

struct agent_request {
	agent_request_type_t type;
	struct l_dbus_message *msg;
	void *cb;
	void *user_data;
};

struct mesh_agent {
	char *path;
	char *owner;
	struct mesh_agent_prov_caps caps;
	struct agent_request *req;
};

struct prov_action {
	const char *action;
	uint16_t output;
	uint16_t input;
	uint8_t size;
};

struct oob_info {
	const char *oob;
	uint16_t mask;
};

static struct prov_action cap_table[] = {
	{"blink", 0x0001, 0x0000, 1},
	{"beep", 0x0002, 0x0000, 1},
	{"vibrate", 0x0004, 0x0000, 1},
	{"out-numeric", 0x0008, 0x0000, 8},
	{"out-alpha", 0x0010, 0x0000, 8},
	{"push", 0x0000, 0x0001, 1},
	{"twist", 0x0000, 0x0002, 1},
	{"in-numeric", 0x0000, 0x0004, 8},
	{"in-alpha", 0x0000, 0x0008, 8}
};

static struct oob_info oob_table[] = {
	{"other", 0x0001},
	{"uri", 0x0002},
	{"machine-code-2d", 0x0004},
	{"barcode", 0x0008},
	{"nfc", 0x0010},
	{"number", 0x0020},
	{"string", 0x0040},
	{"on-box", 0x0800},
	{"in-box", 0x1000},
	{"on-paper", 0x2000},
	{"in-manual", 0x4000},
	{"on-device", 0x8000}
};

static struct l_queue *agents;

static bool simple_match(const void *a, const void *b)
{
	return a == b;
}

static bool parse_prov_caps(struct mesh_agent_prov_caps *caps,
				struct l_dbus_message_iter *property)
{
	struct l_dbus_message_iter iter_caps;
	const char *str;
	uint32_t i;

	if (!l_dbus_message_iter_get_variant(property, "as", &iter_caps))
		return false;

	while (l_dbus_message_iter_next_entry(&iter_caps, &str)) {
		for (i = 0; i < L_ARRAY_SIZE(cap_table); i++) {
			if (strcmp(str, cap_table[i].action))
				continue;

			caps->output_action |= cap_table[i].output;
			if (cap_table[i].output &&
					caps->output_size < cap_table[i].size)
				caps->output_size = cap_table[i].size;

			caps->input_action |= cap_table[i].input;
			if (cap_table[i].input &&
					caps->input_size < cap_table[i].size)
				caps->input_size = cap_table[i].size;

			break;
		}

		if (!strcmp(str, "public-oob"))
			caps->pub_type = 1;
		else if (!strcmp(str, "static-oob"))
			caps->static_type = 1;
	}

	return true;
}

static bool parse_oob_info(struct mesh_agent_prov_caps *caps,
				struct l_dbus_message_iter *property)
{
	struct l_dbus_message_iter iter_oob;
	uint32_t i;
	const char *str;

	if (!l_dbus_message_iter_get_variant(property, "as", &iter_oob))
		return false;

	while (l_dbus_message_iter_next_entry(&iter_oob, &str)) {
		for (i = 0; i < L_ARRAY_SIZE(oob_table); i++) {
			if (strcmp(str, oob_table[i].oob))
				continue;
			caps->oob_info |= oob_table[i].mask;
		}
	}

	return true;
}

static bool parse_properties(struct mesh_agent *agent,
					struct l_dbus_message_iter *properties)
{
	const char *key, *uri_string;
	struct l_dbus_message_iter variant;

	memset(&agent->caps, 0, sizeof(agent->caps));

	while (l_dbus_message_iter_next_entry(properties, &key, &variant)) {
		if (!strcmp(key, "Capabilities")) {
			if (!parse_prov_caps(&agent->caps, &variant))
				return false;
		} else if (!strcmp(key, "URI")) {
			if (!l_dbus_message_iter_get_variant(&variant, "s",
								&uri_string))
				return false;
			/* TODO: compute hash */
		} else if (!strcmp(key, "OutOfBandInfo")) {
			if (!parse_oob_info(&agent->caps, &variant))
				return false;
		}
	}

	return true;
}

static void agent_free(void *agent_data)
{
	struct mesh_agent *agent = agent_data;
	int err;
	mesh_agent_cb_t simple_cb;
	mesh_agent_key_cb_t key_cb;
	mesh_agent_number_cb_t number_cb;

	err = MESH_ERROR_DOES_NOT_EXIST;

	if (agent->req && agent->req->cb) {
		struct agent_request *req = agent->req;

		switch (req->type) {
		case MESH_AGENT_REQUEST_PUSH:
		case MESH_AGENT_REQUEST_TWIST:
		case MESH_AGENT_REQUEST_IN_NUMERIC:
			number_cb = req->cb;
			number_cb(req->user_data, err, 0);
			break;
		case MESH_AGENT_REQUEST_IN_ALPHA:
		case MESH_AGENT_REQUEST_STATIC_OOB:
		case MESH_AGENT_REQUEST_PRIVATE_KEY:
		case MESH_AGENT_REQUEST_PUBLIC_KEY:
			key_cb = req->cb;
			key_cb(req->user_data, err, NULL, 0);
			break;
		case MESH_AGENT_REQUEST_BLINK:
		case MESH_AGENT_REQUEST_BEEP:
		case MESH_AGENT_REQUEST_VIBRATE:
		case MESH_AGENT_REQUEST_OUT_NUMERIC:
		case MESH_AGENT_REQUEST_OUT_ALPHA:
		case MESH_AGENT_REQUEST_CAPABILITIES:
			simple_cb = agent->req->cb;
			simple_cb(req->user_data, err);
		default:
			break;
		}

		l_dbus_message_unref(req->msg);
		l_free(req);
	}

	l_free(agent->path);
	l_free(agent->owner);
	l_free(agent);
}

void mesh_agent_remove(struct mesh_agent *agent)
{
	if (!agent)
		return;

	if (l_queue_remove(agents, agent))
		agent_free(agent);
}

void mesh_agent_cleanup(void)
{
	if (!agents)
		return;

	l_queue_destroy(agents, agent_free);
	agents = NULL;

}

void mesh_agent_init(void)
{
	if (!agents)
		agents = l_queue_new();
}

struct mesh_agent *mesh_agent_create(const char *path, const char *owner,
					struct l_dbus_message_iter *properties)
{
	struct mesh_agent *agent;

	agent = l_new(struct mesh_agent, 1);
	agent->owner = l_strdup(owner);
	agent->path = l_strdup(path);

	if (!parse_properties(agent, properties)) {
		l_free(agent);
		return NULL;
	}

	l_queue_push_tail(agents, agent);

	return agent;
}

struct mesh_agent_prov_caps *mesh_agent_get_caps(struct mesh_agent *agent)
{
	if (!agent || !l_queue_find(agents, simple_match, agent))
		return NULL;

	return &agent->caps;
}

static struct agent_request *create_request(agent_request_type_t type,
						void *cb, void *data)
{
	struct agent_request *req;

	req = l_new(struct agent_request, 1);

	req->type = type;
	req->cb = cb;
	req->user_data = data;

	return req;
}

static int get_reply_error(struct l_dbus_message *reply)
{
	const char *name, *desc;

	if (l_dbus_message_is_error(reply)) {

		l_dbus_message_get_error(reply, &name, &desc);
		l_error("Agent failed (%s), %s", name, desc);
		return MESH_ERROR_FAILED;
	}

	return MESH_ERROR_NONE;
}

static void properties_reply(struct l_dbus_message *reply, void *user_data)
{
	struct mesh_agent *agent = user_data;
	struct agent_request *req;
	mesh_agent_cb_t cb;
	struct l_dbus_message_iter properties;
	int err;

	if (!l_queue_find(agents, simple_match, agent) || !agent->req)
		return;

	req = agent->req;

	err = get_reply_error(reply);

	if (err != MESH_ERROR_NONE)
		goto done;

	if (!l_dbus_message_get_arguments(reply, "a{sv}", &properties)) {
		err = MESH_ERROR_FAILED;
		goto done;
	}

	if (!parse_properties(agent, &properties))
		err = MESH_ERROR_FAILED;

done:
	if (req->cb) {
		cb = req->cb;
		cb(req->user_data, err);
	}

	l_dbus_message_unref(req->msg);
	l_free(req);
	agent->req = NULL;
}

void mesh_agent_refresh(struct mesh_agent *agent, mesh_agent_cb_t cb,
							void *user_data)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	agent->req = create_request(MESH_AGENT_REQUEST_CAPABILITIES, (void *)cb,
								user_data);

	msg = l_dbus_message_new_method_call(dbus, agent->owner, agent->path,
						L_DBUS_INTERFACE_PROPERTIES,
						"GetAll");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 's',
						MESH_PROVISION_AGENT_INTERFACE);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(dbus_get_bus(), msg, properties_reply, agent,
									NULL);

	agent->req->msg = l_dbus_message_ref(msg);
}


static void simple_reply(struct l_dbus_message *reply, void *user_data)
{
	struct mesh_agent *agent = user_data;
	struct agent_request *req;
	mesh_agent_cb_t cb;
	int err;

	if (!l_queue_find(agents, simple_match, agent) || !agent->req)
		return;

	req = agent->req;

	err = get_reply_error(reply);

	l_dbus_message_unref(req->msg);

	if (req->cb) {
		cb = req->cb;
		cb(req->user_data, err);
	}

	l_free(req);
	agent->req = NULL;
}

static void numeric_reply(struct l_dbus_message *reply, void *user_data)
{
	struct mesh_agent *agent = user_data;
	struct agent_request *req;
	mesh_agent_number_cb_t cb;
	uint32_t count;
	int err;

	if (!l_queue_find(agents, simple_match, agent) || !agent->req)
		return;

	req = agent->req;

	err = get_reply_error(reply);

	count = 0;

	if (err == MESH_ERROR_NONE) {
		if (!l_dbus_message_get_arguments(reply, "u", &count)) {
			l_error("Failed to retrieve numeric input");
			err = MESH_ERROR_FAILED;
		}
	}

	l_dbus_message_unref(req->msg);

	if (req->cb) {
		cb = req->cb;
		cb(req->user_data, err, count);
	}

	l_free(req);
	agent->req = NULL;
}

static void key_reply(struct l_dbus_message *reply, void *user_data)
{
	struct mesh_agent *agent = user_data;
	struct agent_request *req;
	mesh_agent_key_cb_t cb;
	struct l_dbus_message_iter iter_array;
	uint32_t n = 0, expected_len = 0;
	uint8_t *buf = NULL;
	int err;

	if (!l_queue_find(agents, simple_match, agent) || !agent->req)
		return;

	req = agent->req;

	err = get_reply_error(reply);

	if (err != MESH_ERROR_NONE)
		goto done;

	if (!l_dbus_message_get_arguments(reply, "ay", &iter_array)) {
		l_error("Failed to retrieve key input");
		err = MESH_ERROR_FAILED;
		goto done;
	}

	if (!l_dbus_message_iter_get_fixed_array(&iter_array, &buf, &n)) {
		l_error("Failed to retrieve key input");
		err = MESH_ERROR_FAILED;
		goto done;
	}

	if (req->type == MESH_AGENT_REQUEST_PRIVATE_KEY)
		expected_len = 32;
	else if (req->type == MESH_AGENT_REQUEST_PUBLIC_KEY)
		expected_len = 64;
	else
		expected_len = 16;

	if (n != expected_len) {
		l_error("Bad response length: %u (need %u)", n, expected_len);
		err = MESH_ERROR_FAILED;
		n = 0;
	}

done:
	if (req->cb) {
		cb = req->cb;
		cb(req->user_data, err, buf, n);
	}

	l_dbus_message_unref(req->msg);

	l_free(req);
	agent->req = NULL;
}

static int output_request(struct mesh_agent *agent, const char *action,
					agent_request_type_t type, uint32_t cnt,
					void *cb, void *user_data)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	if (!l_queue_find(agents, simple_match, agent))
		return MESH_ERROR_DOES_NOT_EXIST;

	if (agent->req)
		return MESH_ERROR_BUSY;

	agent->req = create_request(type, cb, user_data);
	msg = l_dbus_message_new_method_call(dbus, agent->owner, agent->path,
						MESH_PROVISION_AGENT_INTERFACE,
						"DisplayNumeric");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 's', action);
	l_dbus_message_builder_append_basic(builder, 'u', &cnt);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_debug("Send DisplayNumeric request to %s %s",
						agent->owner, agent->path);

	l_dbus_send_with_reply(dbus_get_bus(), msg, simple_reply, agent,
									NULL);

	agent->req->msg = l_dbus_message_ref(msg);

	return MESH_ERROR_NONE;
}

static int prompt_input(struct mesh_agent *agent, const char *action,
					agent_request_type_t type, bool numeric,
					void *cb, void *user_data)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;
	const char *method_name;
	l_dbus_message_func_t reply_cb;

	if (!l_queue_find(agents, simple_match, agent))
		return MESH_ERROR_DOES_NOT_EXIST;

	if (agent->req)
		return MESH_ERROR_BUSY;

	agent->req = create_request(type, cb, user_data);

	method_name = numeric ? "PromptNumeric" : "PromptStatic";

	msg = l_dbus_message_new_method_call(dbus, agent->owner,
						agent->path,
						MESH_PROVISION_AGENT_INTERFACE,
						method_name);

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 's', action);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_debug("Send \"%s\" input request to %s %s", action,
						agent->owner, agent->path);

	reply_cb = numeric ? numeric_reply : key_reply;

	l_dbus_send_with_reply(dbus_get_bus(), msg, reply_cb, agent, NULL);

	agent->req->msg = l_dbus_message_ref(msg);

	return MESH_ERROR_NONE;
}

static int request_key(struct mesh_agent *agent,
					agent_request_type_t type,
					void *cb, void *user_data)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message *msg;
	const char *method_name;

	if (!l_queue_find(agents, simple_match, agent))
		return MESH_ERROR_DOES_NOT_EXIST;

	if (agent->req)
		return MESH_ERROR_BUSY;

	agent->req = create_request(type, cb, user_data);

	method_name = (type == MESH_AGENT_REQUEST_PRIVATE_KEY) ?
						"PrivateKey" : "PublicKey";

	msg = l_dbus_message_new_method_call(dbus, agent->owner,
						agent->path,
						MESH_PROVISION_AGENT_INTERFACE,
						method_name);

	l_dbus_message_set_arguments(msg, "");

	l_debug("Send key request to %s %s", agent->owner, agent->path);

	l_dbus_send_with_reply(dbus_get_bus(), msg, key_reply, agent, NULL);

	agent->req->msg = l_dbus_message_ref(msg);

	return MESH_ERROR_NONE;
}

int mesh_agent_display_string(struct mesh_agent *agent, const char *str,
				mesh_agent_cb_t cb, void *user_data)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message *msg;
	struct l_dbus_message_builder *builder;

	if (!l_queue_find(agents, simple_match, agent))
		return MESH_ERROR_DOES_NOT_EXIST;

	if (agent->req)
		return MESH_ERROR_BUSY;

	agent->req = create_request(MESH_AGENT_REQUEST_OUT_ALPHA,
								cb, user_data);
	msg = l_dbus_message_new_method_call(dbus, agent->owner, agent->path,
						MESH_PROVISION_AGENT_INTERFACE,
						"DisplayString");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 's', str);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_debug("Send DisplayString request to %s %s",
						agent->owner, agent->path);

	l_dbus_send_with_reply(dbus_get_bus(), msg, simple_reply, agent,
									NULL);

	agent->req->msg = l_dbus_message_ref(msg);

	return MESH_ERROR_NONE;

}

int mesh_agent_display_number(struct mesh_agent *agent, bool initiator,
					uint8_t action, uint32_t count,
					mesh_agent_cb_t cb, void *user_data)
{
	const char *str_type;
	agent_request_type_t type;

	type = action;

	if (initiator)
		type = action + MESH_AGENT_REQUEST_PUSH;

	if (type >= L_ARRAY_SIZE(cap_table))
		return MESH_ERROR_INVALID_ARGS;

	str_type = cap_table[type].action;

	return output_request(agent, str_type, type, count, cb, user_data);
}

int mesh_agent_prompt_number(struct mesh_agent *agent, bool initiator,
						uint8_t action,
						mesh_agent_number_cb_t cb,
						void *user_data)
{
	const char *str_type;
	agent_request_type_t type;

	type = action;

	if (!initiator)
		type = action + MESH_AGENT_REQUEST_PUSH;

	if (type >= L_ARRAY_SIZE(cap_table))
		return MESH_ERROR_INVALID_ARGS;

	str_type = cap_table[type].action;

	return prompt_input(agent, str_type, type, true, cb, user_data);
}

int mesh_agent_prompt_alpha(struct mesh_agent *agent, bool initiator,
					mesh_agent_key_cb_t cb, void *user_data)
{
	if (initiator)
		return prompt_input(agent,
				cap_table[MESH_AGENT_REQUEST_OUT_ALPHA].action,
				MESH_AGENT_REQUEST_OUT_ALPHA, false, cb,
				user_data);
	else
		return prompt_input(agent,
				cap_table[MESH_AGENT_REQUEST_IN_ALPHA].action,
				MESH_AGENT_REQUEST_IN_ALPHA, false, cb,
				user_data);
}

int mesh_agent_request_static(struct mesh_agent *agent, mesh_agent_key_cb_t cb,
								void *user_data)
{
	return prompt_input(agent, "static-oob", MESH_AGENT_REQUEST_STATIC_OOB,
							false, cb, user_data);
}

int mesh_agent_request_private_key(struct mesh_agent *agent,
				mesh_agent_key_cb_t cb, void *user_data)
{
	return request_key(agent, MESH_AGENT_REQUEST_PRIVATE_KEY, cb,
								user_data);

}

int mesh_agent_request_public_key(struct mesh_agent *agent,
				mesh_agent_key_cb_t cb, void *user_data)
{
	return request_key(agent, MESH_AGENT_REQUEST_PUBLIC_KEY, cb,
								user_data);
}

void mesh_agent_cancel(struct mesh_agent *agent)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message *msg;

	if (!l_queue_find(agents, simple_match, agent))
		return;

	msg = l_dbus_message_new_method_call(dbus, agent->owner, agent->path,
						MESH_PROVISION_AGENT_INTERFACE,
						"Cancel");

	l_dbus_message_set_arguments(msg, "");

	l_dbus_send(dbus, msg);
}
