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

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "lldpctl.h"
#include "atom.h"
#include "../log.h"
#include "../marshal.h"
#include "../ctl.h"

lldpctl_conn_t*
lldpctl_atom_get_connection(lldpctl_atom_t *atom)
{
	if (atom)
		return atom->conn;
	return NULL;
}

void
lldpctl_atom_inc_ref(lldpctl_atom_t *atom)
{
	if (atom)
		atom->count++;
}

void
lldpctl_atom_dec_ref(lldpctl_atom_t *atom)
{
	struct atom_buffer *buffer, *buffer_next;
	if (atom && (--atom->count == 0)) {
		if (atom->free)
			atom->free(atom);

		/* Remove special allocated buffers */
		for (buffer = TAILQ_FIRST(&atom->buffers);
		     buffer;
		     buffer = buffer_next) {
			buffer_next = TAILQ_NEXT(buffer, next);
			free(buffer);
		}

		free(atom);
	}
}

lldpctl_atom_t*
lldpctl_atom_get(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->get == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	return atom->get(atom, key);
}

lldpctl_atom_t*
lldpctl_atom_set(lldpctl_atom_t *atom, lldpctl_key_t key,
    lldpctl_atom_t *value)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->set == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	return atom->set(atom, key, value);
}

const char*
lldpctl_atom_get_str(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	char *strresult = NULL;
	const uint8_t *bufresult = NULL;
	long int intresult = -1;
	int n1;
	size_t n2;

	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->get_str != NULL) {
		strresult = (char *)atom->get_str(atom, key);
		if (strresult) return strresult;
		if (lldpctl_last_error(atom->conn) != LLDPCTL_ERR_NOT_EXIST)
			return NULL;
	}

	RESET_ERROR(atom->conn);
	if (atom->get_int != NULL) {
		intresult = atom->get_int(atom, key);
		if (lldpctl_last_error(atom->conn) != LLDPCTL_ERR_NOT_EXIST) {
			strresult = _lldpctl_alloc_in_atom(atom, 21);
			if (!strresult) return NULL;
			n1 = snprintf(strresult, 21, "%ld", intresult);
			if (n1 > -1 && n1 < 21)
				return strresult;
			SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM); /* Not really true... */
			return NULL;
		}
	}

	RESET_ERROR(atom->conn);
	if (atom->get_buffer != NULL) {
		bufresult = atom->get_buffer(atom, key, &n2);
		if (bufresult)
			return _lldpctl_dump_in_atom(atom, bufresult, n2, ' ', 0);
		if (lldpctl_last_error(atom->conn) != LLDPCTL_ERR_NOT_EXIST)
			return NULL;
	}

	SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	return NULL;
}

lldpctl_atom_t*
lldpctl_atom_set_str(lldpctl_atom_t *atom, lldpctl_key_t key,
	const char *value)
{
	lldpctl_atom_t *result = NULL;
	const char *errstr;
	long long converted = 0;
	int isint = 0;
	int bad = 0;

	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->set_str != NULL) {
		result = atom->set_str(atom, key, value);
		if (result) return result;
		if (lldpctl_last_error(atom->conn) != LLDPCTL_ERR_NOT_EXIST &&
		    lldpctl_last_error(atom->conn) != LLDPCTL_ERR_BAD_VALUE)
			return NULL;
		bad = bad || (lldpctl_last_error(atom->conn) == LLDPCTL_ERR_BAD_VALUE);
	}

	if (value) {
		converted = strtonum(value, LLONG_MIN, LLONG_MAX, &errstr);
		isint = (errstr == NULL);
	}

	RESET_ERROR(atom->conn);
	if (atom->set_int != NULL && isint) {
		result = atom->set_int(atom, key, converted);
		if (result) return result;
		if (lldpctl_last_error(atom->conn) != LLDPCTL_ERR_NOT_EXIST &&
		    lldpctl_last_error(atom->conn) != LLDPCTL_ERR_BAD_VALUE)
			return NULL;
		bad = bad || (lldpctl_last_error(atom->conn) == LLDPCTL_ERR_BAD_VALUE);
	}

	RESET_ERROR(atom->conn);
	if (atom->set_buffer != NULL) {
		result = atom->set_buffer(atom, key, (u_int8_t*)value, value?strlen(value):0);
		if (result) return result;
		if (lldpctl_last_error(atom->conn) != LLDPCTL_ERR_NOT_EXIST &&
		    lldpctl_last_error(atom->conn) != LLDPCTL_ERR_BAD_VALUE)
			return NULL;
		bad = bad || (lldpctl_last_error(atom->conn) == LLDPCTL_ERR_BAD_VALUE);
	}

	SET_ERROR(atom->conn, bad?LLDPCTL_ERR_BAD_VALUE:LLDPCTL_ERR_NOT_EXIST);
	return NULL;
}

const u_int8_t*
lldpctl_atom_get_buffer(lldpctl_atom_t *atom, lldpctl_key_t key,
    size_t *length)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->get_buffer == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	return atom->get_buffer(atom, key, length);
}

lldpctl_atom_t*
lldpctl_atom_set_buffer(lldpctl_atom_t *atom, lldpctl_key_t key,
    const u_int8_t* value, size_t length)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->set_buffer == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	return atom->set_buffer(atom, key, value, length);
}

long int
lldpctl_atom_get_int(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	if (atom == NULL) return LLDPCTL_ERR_NOT_EXIST;
	RESET_ERROR(atom->conn);

	if (atom->get_int == NULL)
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	return atom->get_int(atom, key);
}

lldpctl_atom_t*
lldpctl_atom_set_int(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (atom->set_int == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	return atom->set_int(atom, key, value);
}

lldpctl_atom_iter_t*
lldpctl_atom_iter(lldpctl_atom_t *atom)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (!atom->iter) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_CANNOT_ITERATE);
		return NULL;
	}
	return atom->iter(atom);
}

lldpctl_atom_iter_t*
lldpctl_atom_iter_next(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (!atom->next) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_CANNOT_ITERATE);
		return NULL;
	}
	return atom->next(atom, iter);
}

lldpctl_atom_t*
lldpctl_atom_iter_value(lldpctl_atom_t *atom,  lldpctl_atom_iter_t *iter)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (!atom->value) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_CANNOT_ITERATE);
		return NULL;
	}
	return atom->value(atom, iter);
}

lldpctl_atom_t*
lldpctl_atom_create(lldpctl_atom_t *atom)
{
	if (atom == NULL) return NULL;
	RESET_ERROR(atom->conn);

	if (!atom->create) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_CANNOT_CREATE);
		return NULL;
	}
	return atom->create(atom);
}

/**
 * Get somethin with IO.
 *
 * @param conn       The connection to lldpd.
 * @param state_send State to be when "sending"
 * @param state_recv State to be when "receiving"
 * @param state_data Ancillary data for state handling
 * @param type       Type of message to send (and receive)
 * @param to_send    Data to send.
 * @param mi_send    Marshalling info for data to send.
 * @param to_recv    Data to receive.
 * @param mi_recv    Marshalling info for data to recive.
 * @return 0 in case of success, a negative integer in case of failure.
 *
 * The current state must match one of @c CONN_STATE_IDLE, @c state_send or @c
 * state_recv and in the two later cases, the provided @c state_data must match.
 */
int
_lldpctl_do_something(lldpctl_conn_t *conn,
    int state_send, int state_recv, const char *state_data,
    enum hmsg_type type,
    void *to_send, struct marshal_info *mi_send,
    void **to_recv, struct marshal_info *mi_recv)
{
	ssize_t rc;

	if (conn->state == CONN_STATE_WATCHING)
		/* The connection cannot be used anymore. */
		return SET_ERROR(conn, LLDPCTL_ERR_INVALID_STATE);

	if (conn->state == CONN_STATE_IDLE) {
		/* We need to build the message to send, then send
		 * it. */
		if (ctl_msg_send_unserialized(&conn->output_buffer, &conn->output_buffer_len,
			type, to_send, mi_send) != 0)
			return SET_ERROR(conn, LLDPCTL_ERR_SERIALIZATION);
		conn->state = state_send;
		if (state_data)
			strlcpy(conn->state_data, state_data, sizeof(conn->state_data));
		else
			conn->state_data[0] = 0;
	}
	if (conn->state == state_send &&
	    (state_data == NULL || !strncmp(conn->state_data, state_data, sizeof(conn->state_data) - 1))) {
		/* We need to send the currently built message */
		rc = lldpctl_send(conn);
		if (rc < 0)
			return SET_ERROR(conn, rc);
		conn->state = state_recv;
	}
	if (conn->state == state_recv &&
	    (state_data == NULL || !strncmp(conn->state_data, state_data, sizeof(conn->state_data) - 1))) {
		/* We need to receive the answer */
		while ((rc = ctl_msg_recv_unserialized(&conn->input_buffer,
			    &conn->input_buffer_len,
			    type, to_recv, mi_recv)) > 0) {
			/* We need more bytes */
			rc = _lldpctl_needs(conn, rc);
			if (rc < 0)
				return SET_ERROR(conn, rc);
		}
		if (rc < 0)
			return SET_ERROR(conn, LLDPCTL_ERR_SERIALIZATION);
		/* rc == 0 */
		conn->state = CONN_STATE_IDLE;
		conn->state_data[0] = 0;
		return 0;
	} else
		return SET_ERROR(conn, LLDPCTL_ERR_INVALID_STATE);
}


int
lldpctl_watch_callback(lldpctl_conn_t *conn,
    lldpctl_change_callback cb,
    void *data)
{
	int rc;

	RESET_ERROR(conn);

	rc = _lldpctl_do_something(conn,
	    CONN_STATE_SET_WATCH_SEND, CONN_STATE_SET_WATCH_RECV, NULL,
	    SUBSCRIBE, NULL, NULL, NULL, NULL);
	if (rc == 0) {
		conn->watch_cb = cb;
		conn->watch_data = data;
		conn->state = CONN_STATE_WATCHING;
	}
	return rc;
}

int
lldpctl_watch_callback2(lldpctl_conn_t *conn,
    lldpctl_change_callback2 cb,
    void *data)
{
	int rc;

	RESET_ERROR(conn);

	rc = _lldpctl_do_something(conn,
	    CONN_STATE_SET_WATCH_SEND, CONN_STATE_SET_WATCH_RECV, NULL,
	    SUBSCRIBE, NULL, NULL, NULL, NULL);
	if (rc == 0) {
		conn->watch_cb2 = cb;
		conn->watch_data = data;
		conn->state = CONN_STATE_WATCHING;
	}
	return rc;
}

int
lldpctl_watch(lldpctl_conn_t *conn)
{
	int rc = 0;

	RESET_ERROR(conn);

	if (conn->state != CONN_STATE_WATCHING)
		return SET_ERROR(conn, LLDPCTL_ERR_INVALID_STATE);

	conn->watch_triggered = 0;
	while (!conn->watch_triggered) {
		rc = _lldpctl_needs(conn, 1);
		if (rc < 0)
			return SET_ERROR(conn, rc);
	}

	RESET_ERROR(conn);
	return 0;
}

lldpctl_atom_t*
lldpctl_get_configuration(lldpctl_conn_t *conn)
{
	int rc;
	struct lldpd_config *config;
	void *p;

	RESET_ERROR(conn);

	rc = _lldpctl_do_something(conn,
	    CONN_STATE_GET_CONFIG_SEND, CONN_STATE_GET_CONFIG_RECV, NULL,
	    GET_CONFIG,
	    NULL, NULL,
	    &p, &MARSHAL_INFO(lldpd_config));
	if (rc == 0) {
		config = p;
		return _lldpctl_new_atom(conn, atom_config, config);
	}
	return NULL;
}

lldpctl_atom_t*
lldpctl_get_interfaces(lldpctl_conn_t *conn)
{
	struct lldpd_interface_list *ifs;
	void *p;
	int rc;

	RESET_ERROR(conn);

	rc = _lldpctl_do_something(conn,
	    CONN_STATE_GET_INTERFACES_SEND, CONN_STATE_GET_INTERFACES_RECV, NULL,
	    GET_INTERFACES,
	    NULL, NULL,
	    &p, &MARSHAL_INFO(lldpd_interface_list));
	if (rc == 0) {
		ifs = p;
		return _lldpctl_new_atom(conn, atom_interfaces_list, ifs);
	}
	return NULL;
}

lldpctl_atom_t*
lldpctl_get_local_chassis(lldpctl_conn_t *conn)
{
	struct lldpd_chassis *chassis;
	void *p;
	int rc;

	RESET_ERROR(conn);

	rc = _lldpctl_do_something(conn,
	    CONN_STATE_GET_CHASSIS_SEND, CONN_STATE_GET_CHASSIS_RECV, NULL,
	    GET_CHASSIS,
	    NULL, NULL,
	    &p, &MARSHAL_INFO(lldpd_chassis));
	if (rc == 0) {
		chassis = p;
		return _lldpctl_new_atom(conn, atom_chassis, chassis, NULL, 0);
	}
	return NULL;
}

lldpctl_atom_t*
lldpctl_get_port(lldpctl_atom_t *atom)
{
	int rc;
	lldpctl_conn_t *conn = atom->conn;
	struct lldpd_hardware *hardware;
	void *p;
	struct _lldpctl_atom_interface_t *iface =
	    (struct _lldpctl_atom_interface_t *)atom;

	RESET_ERROR(conn);

	if (atom->type != atom_interface) {
		SET_ERROR(conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
		return NULL;
	}
	rc = _lldpctl_do_something(conn,
	    CONN_STATE_GET_PORT_SEND, CONN_STATE_GET_PORT_RECV, iface->name,
	    GET_INTERFACE, (void*)iface->name, &MARSHAL_INFO(string),
	    &p, &MARSHAL_INFO(lldpd_hardware));
	if (rc == 0) {
		hardware = p;
		return _lldpctl_new_atom(conn, atom_port, 1,
		    hardware, &hardware->h_lport, NULL);
	}
	return NULL;
}

lldpctl_atom_t*
lldpctl_get_default_port(lldpctl_conn_t *conn)
{
	struct lldpd_port *port;
	void *p;
	int rc;

	RESET_ERROR(conn);

	rc = _lldpctl_do_something(conn,
	    CONN_STATE_GET_DEFAULT_PORT_SEND, CONN_STATE_GET_DEFAULT_PORT_RECV, "",
	    GET_DEFAULT_PORT, NULL, NULL,
	    &p, &MARSHAL_INFO(lldpd_port));
	if (rc == 0) {
		port = p;
		return _lldpctl_new_atom(conn, atom_port, 1, NULL, port, NULL);
	}
	return NULL;
}

static lldpctl_map_t empty_map[] = {{ 0, NULL }};

static struct atom_map atom_map_list = {
	.next = NULL
};

lldpctl_map_t*
lldpctl_key_get_map(lldpctl_key_t key)
{
	init_atom_map();
	struct atom_map *map;
	for (map = atom_map_list.next; map ; map = map->next) {
		if (map->key == key)
			return map->map;
	}
	return empty_map;
}

void atom_map_register(struct atom_map *map, int prio)
{
	(void)prio;
	struct atom_map* iter = &atom_map_list;

	while (iter->next)
		iter = iter->next;

	iter->next = map;
}

static struct atom_builder atom_builder_list = {
	.nextb = NULL
};

void atom_builder_register(struct atom_builder *builder, int prio)
{
	(void)prio;
	struct atom_builder* iter = &atom_builder_list;

	while (iter->nextb)
		iter = iter->nextb;

	iter->nextb = builder;
}

lldpctl_atom_t*
_lldpctl_new_atom(lldpctl_conn_t *conn, atom_t type, ...)
{
	init_atom_builder();
	struct atom_builder *builder;
	struct lldpctl_atom_t *atom;
	va_list(ap);
	for (builder = atom_builder_list.nextb; builder ; builder = builder->nextb) {
		if (builder->type != type) continue;
		atom = calloc(1, builder->size);
		if (atom == NULL) {
			SET_ERROR(conn, LLDPCTL_ERR_NOMEM);
			return NULL;
		}
		atom->count = 1;
		atom->type  = type;
		atom->conn  = conn;
		TAILQ_INIT(&atom->buffers);
		atom->free  = builder->free;

		atom->iter  = builder->iter;
		atom->next  = builder->next;
		atom->value = builder->value;

		atom->get       = builder->get;
		atom->get_str   = builder->get_str;
		atom->get_buffer= builder->get_buffer;
		atom->get_int   = builder->get_int;

		atom->set       = builder->set;
		atom->set_str   = builder->set_str;
		atom->set_buffer= builder->set_buffer;
		atom->set_int   = builder->set_int;
		atom->create    = builder->create;

		va_start(ap, type);
		if (builder->init && builder->init(atom, ap) == 0) {
			free(atom);
			va_end(ap);
			/* Error to be set in init() */
			return NULL;
		}
		va_end(ap);
		return atom;
	}
	log_warnx("rpc", "unknown atom type: %d", type);
	SET_ERROR(conn, LLDPCTL_ERR_FATAL);
	return NULL;
}

/**
 * Allocate a buffer inside an atom.
 *
 * It will be freed automatically when the atom is released. This buffer cannot
 * be reallocated and should not be freed!
 *
 * @param atom Atom which will be used as a container.
 * @param size Size of the allocated area.
 * @return Pointer to the buffer or @c NULL if allocation fails.
 */
void*
_lldpctl_alloc_in_atom(lldpctl_atom_t *atom, size_t size)
{
	struct atom_buffer *buffer;

	if ((buffer = calloc(1, size + sizeof(struct atom_buffer))) == NULL) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
		return NULL;
	}
	TAILQ_INSERT_TAIL(&atom->buffers, buffer, next);
	return &buffer->data[0];
}

/**
 * Allocate a buffer inside an atom and dump another buffer in it.
 *
 * The dump is done in hexadecimal with the provided separator.
 *
 * @param atom   Atom which will be used as a container.
 * @param input  Buffer we want to dump.
 * @param size   Size of the buffer
 * @param sep    Separator to use.
 * @param max    Maximum number of bytes to dump. Can be 0 if no maximum.
 * @return A string representing the dump of the buffer or @c NULL if error.
 */
const char*
_lldpctl_dump_in_atom(lldpctl_atom_t *atom,
    const uint8_t *input, size_t size,
    char sep, size_t max)
{
	static const char truncation[] = "[...]";
	size_t i, len;
	char *buffer = NULL;

	if (max > 0 && size > max)
		len = max * 3 + sizeof(truncation) + 1;
	else
		len = size * 3 + 1;

	if ((buffer = _lldpctl_alloc_in_atom(atom, len)) == NULL)
		return NULL;

	for (i = 0; (i < size) && (max == 0 || i < max); i++)
		snprintf(buffer + i * 3, 4, "%02x%c", *(u_int8_t*)(input + i), sep);
	if (max > 0 && size > max)
		snprintf(buffer + i * 3, sizeof(truncation) + 1, "%s", truncation);
	else
		*(buffer + i*3 - 1) = 0;
	return buffer;
}
