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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "ctl.h"
#include "marshal.h"
#include "log.h"
#include "compat/compat.h"

/**
 * Create a new listening Unix socket for control protocol.
 *
 * @param name The name of the Unix socket.
 * @return The socket when successful, -1 otherwise.
 */
int
ctl_create(const char *name)
{
	int s;
	struct sockaddr_un su;
	int rc;

	log_debug("control", "create control socket %s", name);

	if ((s = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;
	if (fcntl(s, F_SETFD, FD_CLOEXEC) == -1) {
		close(s);
		return -1;
	}
	su.sun_family = AF_UNIX;
	strlcpy(su.sun_path, name, sizeof(su.sun_path));
	if (bind(s, (struct sockaddr *)&su, sizeof(struct sockaddr_un)) == -1) {
		rc = errno; close(s); errno = rc;
		return -1;
	}

	log_debug("control", "listen to control socket %s", name);
	if (listen(s, 5) == -1) {
		rc = errno; close(s); errno = rc;
		log_debug("control", "cannot listen to control socket %s", name);
		return -1;
	}
	return s;
}

/**
 * Connect to the control Unix socket.
 *
 * @param name The name of the Unix socket.
 * @return The socket when successful, -1 otherwise.
 */
int
ctl_connect(const char *name)
{
	int s;
	struct sockaddr_un su;
	int rc;

	log_debug("control", "connect to control socket %s", name);

	if ((s = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
		return -1;
	su.sun_family = AF_UNIX;
	strlcpy(su.sun_path, name, sizeof(su.sun_path));
	if (connect(s, (struct sockaddr *)&su, sizeof(struct sockaddr_un)) == -1) {
		rc = errno;
		log_warn("control", "unable to connect to socket %s", name);
		close(s);
		errno = rc; return -1;
	}
	return s;
}

/**
 * Remove the control Unix socket.
 *
 * @param name The name of the Unix socket.
 */
void
ctl_cleanup(const char *name)
{
	log_debug("control", "cleanup control socket");
	if (unlink(name) == -1)
		log_warn("control", "unable to unlink %s", name);
}

/**
 * Serialize and "send" a structure through the control protocol.
 *
 * This function does not really send the message but outputs it to a buffer.
 *
 * @param output_buffer A pointer to a buffer to which the message will be
 *                      appended. Can be @c NULL. In this case, the buffer will
 *                      be allocated.
 * @param[in,out] output_len The length of the provided buffer. Will be updated
 *                           with the new length
 * @param type  The type of message we want to send.
 * @param t     The structure to be serialized and sent.
 * @param mi    The appropriate marshal structure for serialization.
 * @return -1 in case of failure, 0 in case of success.
 *
 * Make sure this function logic matches the server-side one: @c levent_ctl_recv().
 */
int
ctl_msg_send_unserialized(uint8_t **output_buffer, size_t *output_len,
    enum hmsg_type type,
    void *t, struct marshal_info *mi)
{
	ssize_t len = 0, newlen;
	void *buffer = NULL;

	log_debug("control", "send a message through control socket");
	if (t) {
		len = marshal_serialize_(mi, t, &buffer, 0, NULL, 0);
		if (len <= 0) {
			log_warnx("control", "unable to serialize data");
			return -1;
		}
	}

	newlen = len + sizeof(struct hmsg_header);

	if (*output_buffer == NULL) {
		*output_len = 0;
		if ((*output_buffer = malloc(newlen)) == NULL) {
			log_warn("control", "no memory available");
			free(buffer);
			return -1;
		}
	} else {
		void *new = realloc(*output_buffer, *output_len + newlen);
		if (new == NULL) {
			log_warn("control", "no memory available");
			free(buffer);
			return -1;
		}
		*output_buffer = new;
	}

	struct hmsg_header hdr;
	memset(&hdr, 0, sizeof(struct hmsg_header));
	hdr.type = type;
	hdr.len = len;
	memcpy(*output_buffer + *output_len, &hdr, sizeof(struct hmsg_header));
	if (t)
		memcpy(*output_buffer + *output_len + sizeof(struct hmsg_header), buffer, len);
	*output_len += newlen;
	free(buffer);
	return 0;
}

/**
 * "Receive" and unserialize a structure through the control protocol.
 *
 * Like @c ctl_msg_send_unserialized(), this function uses buffer to receive the
 * incoming message.
 *
 * @param[in,out] input_buffer The buffer with the incoming message. Will be
 *                             updated once the message has been unserialized to
 *                             point to the remaining of the message or will be
 *                             freed if all the buffer has been consumed. Can be
 *                             @c NULL.
 * @param[in,out] input_len    The length of the provided buffer. Will be updated
 *                             to the length of remaining data once the message
 *                             has been unserialized.
 * @param expected_type        The expected message type.
 * @param[out] t               Will contain a pointer to the unserialized structure.
 *                             Can be @c NULL if we don't want to store the
 *                             answer.
 * @param mi                   The appropriate marshal structure for unserialization.
 *
 * @return -1 in case of error, 0 in case of success and the number of bytes we
 *         request to complete unserialization.
 *
 * When requesting a notification, the input buffer is left untouched if we
 * don't get one and we fail silently.
 */
size_t
ctl_msg_recv_unserialized(uint8_t **input_buffer, size_t *input_len,
    enum hmsg_type expected_type,
    void **t, struct marshal_info *mi)
{
	struct hmsg_header hdr;
	int rc = -1;

	if (*input_buffer == NULL ||
	    *input_len < sizeof(struct hmsg_header)) {
		/* Not enough data. */
		return sizeof(struct hmsg_header) - *input_len;
	}

	log_debug("control", "receive a message through control socket");
	memcpy(&hdr, *input_buffer, sizeof(struct hmsg_header));
	if (hdr.len > HMSG_MAX_SIZE) {
		log_warnx("control", "message received is too large");
		/* We discard the whole buffer */
		free(*input_buffer);
		*input_buffer = NULL;
		*input_len = 0;
		return -1;
	}
	if (*input_len < sizeof(struct hmsg_header) + hdr.len) {
		/* Not enough data. */
		return sizeof(struct hmsg_header) + hdr.len - *input_len;
	}
	if (hdr.type != expected_type) {
		if (expected_type == NOTIFICATION) return -1;
		log_warnx("control", "incorrect received message type (expected: %d, received: %d)",
		    expected_type, hdr.type);
		goto end;
	}

	if (t && !hdr.len) {
		log_warnx("control", "no payload available in answer");
		goto end;
	}
	if (t) {
		/* We have data to unserialize. */
		if (marshal_unserialize_(mi, *input_buffer + sizeof(struct hmsg_header),
			hdr.len, t, NULL, 0, 0) <= 0) {
			log_warnx("control", "unable to deserialize received data");
			goto end;
		}
	}

	rc = 0;
end:
	/* Discard input buffer */
	*input_len -= sizeof(struct hmsg_header) + hdr.len;
	if (*input_len == 0) {
		free(*input_buffer);
		*input_buffer = NULL;
	} else
		memmove(*input_buffer,
		    *input_buffer + sizeof(struct hmsg_header) + hdr.len,
		    *input_len);
	return rc;
}
