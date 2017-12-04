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
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "lldpctl.h"
#include "atom.h"
#include "../compat/compat.h"
#include "../ctl.h"
#include "../log.h"

const char*
lldpctl_get_default_transport(void)
{
	return LLDPD_CTL_SOCKET;
}

/* Connect to the remote end */
static int
sync_connect(lldpctl_conn_t *lldpctl)
{
	return ctl_connect(lldpctl->ctlname);
}

/* Synchronously send data to remote end. */
static ssize_t
sync_send(lldpctl_conn_t *lldpctl,
    const uint8_t *data, size_t length, void *user_data)
{
	struct lldpctl_conn_sync_t *conn = user_data;
	ssize_t nb;

	if (conn->fd == -1 &&
	    ((conn->fd = sync_connect(lldpctl)) == -1)) {
		return LLDPCTL_ERR_CANNOT_CONNECT;
	}

	while ((nb = write(conn->fd, data, length)) == -1) {
		if (errno == EAGAIN || errno == EINTR) continue;
		return LLDPCTL_ERR_CALLBACK_FAILURE;
	}
	return nb;
}

/* Statically receive data from remote end. */
static ssize_t
sync_recv(lldpctl_conn_t *lldpctl,
    const uint8_t *data, size_t length, void *user_data)
{
	struct lldpctl_conn_sync_t *conn = user_data;
	ssize_t nb;
	size_t remain, offset = 0;

	if (conn->fd == -1 &&
	    ((conn->fd = sync_connect(lldpctl)) == -1)) {
		lldpctl->error = LLDPCTL_ERR_CANNOT_CONNECT;
		return LLDPCTL_ERR_CANNOT_CONNECT;
	}

	remain = length;
	do {
		if ((nb = read(conn->fd, (unsigned char*)data + offset, remain)) == -1) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			return LLDPCTL_ERR_CALLBACK_FAILURE;
		}
		remain -= nb;
		offset += nb;
	} while (remain > 0 && nb != 0);
	return offset;
}

lldpctl_conn_t*
lldpctl_new(lldpctl_send_callback send, lldpctl_recv_callback recv, void *user_data)
{
	return lldpctl_new_name(lldpctl_get_default_transport(), send, recv, user_data);
}

lldpctl_conn_t*
lldpctl_new_name(const char *ctlname, lldpctl_send_callback send, lldpctl_recv_callback recv, void *user_data)
{
	lldpctl_conn_t *conn = NULL;
	struct lldpctl_conn_sync_t *data = NULL;

	/* Both callbacks are mandatory or should be NULL. */
	if (send && !recv) return NULL;
	if (recv && !send) return NULL;

	if ((conn = calloc(1, sizeof(lldpctl_conn_t))) == NULL)
		return NULL;

	conn->ctlname = strdup(ctlname);
	if (conn->ctlname == NULL) {
		free(conn);
		return NULL;
	}
	if (!send && !recv) {
		if ((data = malloc(sizeof(struct lldpctl_conn_sync_t))) == NULL) {
			free(conn);
			return NULL;
		}
		data->fd = -1;
		conn->send = sync_send;
		conn->recv = sync_recv;
		conn->user_data = data;
	} else {
		conn->send = send;
		conn->recv = recv;
		conn->user_data = user_data;
	}

	return conn;
}

int
lldpctl_release(lldpctl_conn_t *conn)
{
	if (conn == NULL) return 0;
	free(conn->ctlname);
	if (conn->send == sync_send) {
		struct lldpctl_conn_sync_t *data = conn->user_data;
		if (data->fd != -1) close(data->fd);
		free(conn->user_data);
	}
	free(conn->input_buffer);
	free(conn->output_buffer);
	free(conn);
	return 0;
}

/**
 * Request some bytes if they are not already here.
 *
 * @param conn   The connection to lldpd.
 * @param length The number of requested bytes.
 * @return A negative integer if we can't have the bytes or the number of bytes we got.
 */
ssize_t
_lldpctl_needs(lldpctl_conn_t *conn, size_t length)
{
	uint8_t *buffer = NULL;
	ssize_t  rc;

	if ((buffer = malloc(length)) == NULL)
		return SET_ERROR(conn, LLDPCTL_ERR_NOMEM);
	rc = conn->recv(conn, buffer, length, conn->user_data);
	if (rc < 0) {
		free(buffer);
		return SET_ERROR(conn, rc);
	}
	if (rc == 0) {
		free(buffer);
		return SET_ERROR(conn, LLDPCTL_ERR_EOF);
	}
	rc = lldpctl_recv(conn, buffer, rc);
	free(buffer);
	if (rc < 0)
		return SET_ERROR(conn, rc);
	RESET_ERROR(conn);
	return rc;
}

static int
check_for_notification(lldpctl_conn_t *conn)
{
	struct lldpd_neighbor_change *change;
	void *p;
	int rc;
	lldpctl_change_t type;
	lldpctl_atom_t *interface = NULL, *neighbor = NULL;
	rc = ctl_msg_recv_unserialized(&conn->input_buffer,
	    &conn->input_buffer_len,
	    NOTIFICATION,
	    &p,
	    &MARSHAL_INFO(lldpd_neighbor_change));
	if (rc != 0) return rc;
	change = p;

	/* We have a notification, call the callback */
	if (conn->watch_cb) {
		switch (change->state) {
		case NEIGHBOR_CHANGE_DELETED: type = lldpctl_c_deleted; break;
		case NEIGHBOR_CHANGE_ADDED: type = lldpctl_c_added; break;
		case NEIGHBOR_CHANGE_UPDATED: type = lldpctl_c_updated; break;
		default:
			log_warnx("control", "unknown notification type (%d)",
			    change->state);
			goto end;
		}
		interface = _lldpctl_new_atom(conn, atom_interface,
		    change->ifname);
		if (interface == NULL) goto end;
		neighbor = _lldpctl_new_atom(conn, atom_port, 0,
		    NULL, change->neighbor, NULL);
		if (neighbor == NULL) goto end;
		conn->watch_cb(conn, type, interface, neighbor, conn->watch_data);
		conn->watch_triggered = 1;
		goto end;
	}

end:
	if (interface) lldpctl_atom_dec_ref(interface);
	if (neighbor) lldpctl_atom_dec_ref(neighbor);
	else {
		lldpd_chassis_cleanup(change->neighbor->p_chassis, 1);
		lldpd_port_cleanup(change->neighbor, 1);
		free(change->neighbor);
	}
	free(change->ifname);
	free(change);

	/* Indicate if more data remains in the buffer for processing */
	return (rc);
}

ssize_t
lldpctl_recv(lldpctl_conn_t *conn, const uint8_t *data, size_t length)
{

	RESET_ERROR(conn);

	if (length == 0) return 0;

	/* Received data should be appended to the input buffer. */
	if (conn->input_buffer == NULL) {
		conn->input_buffer_len = 0;
		if ((conn->input_buffer = malloc(length)) == NULL)
			return SET_ERROR(conn, LLDPCTL_ERR_NOMEM);
	} else {
		uint8_t *new = realloc(conn->input_buffer, conn->input_buffer_len + length);
		if (new == NULL)
			return SET_ERROR(conn, LLDPCTL_ERR_NOMEM);
		conn->input_buffer = new;
	}
	memcpy(conn->input_buffer + conn->input_buffer_len, data, length);
	conn->input_buffer_len += length;

	/* Is it a notification? */
	check_for_notification(conn);

	RESET_ERROR(conn);

	return conn->input_buffer_len;
}

int lldpctl_process_conn_buffer(lldpctl_conn_t *conn)
{
	int rc;

	rc = check_for_notification(conn);

	RESET_ERROR(conn);

	return rc;
}

ssize_t
lldpctl_send(lldpctl_conn_t *conn)
{
	/* Send waiting data. */
	ssize_t rc;

	RESET_ERROR(conn);

	if (!conn->output_buffer) return 0;
	rc = conn->send(conn,
	    conn->output_buffer, conn->output_buffer_len,
	    conn->user_data);
	if (rc < 0) return SET_ERROR(conn, rc);

	/* "Shrink" the output buffer. */
	if (rc == conn->output_buffer_len) {
		free(conn->output_buffer);
		conn->output_buffer = NULL;
		conn->output_buffer_len = 0;
		RESET_ERROR(conn);
		return rc;
	}
	conn->output_buffer_len -= rc;
	memmove(conn->output_buffer, conn->output_buffer + rc, conn->output_buffer_len);
	/* We don't shrink the buffer. It will be either freed or shrinked later */
	RESET_ERROR(conn);
	return rc;
}
