/*
 * libwebsockets-test-server - libwebsockets test implementation
 *
 * Copyright (C) 2010-2015 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
#include "test-server.h"

/* lws-mirror_protocol */

#define MAX_MESSAGE_QUEUE 32

struct a_message {
	void *payload;
	size_t len;
};

static struct a_message ringbuffer[MAX_MESSAGE_QUEUE];
static int ringbuffer_head;

int
callback_lws_mirror(struct lws *wsi, enum lws_callback_reasons reason,
		    void *user, void *in, size_t len)
{
	struct per_session_data__lws_mirror *pss =
			(struct per_session_data__lws_mirror *)user;
	int n;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		lwsl_info("%s: LWS_CALLBACK_ESTABLISHED\n", __func__);
		pss->ringbuffer_tail = ringbuffer_head;
		pss->wsi = wsi;
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		lwsl_notice("%s: mirror protocol cleaning up\n", __func__);
		for (n = 0; n < sizeof ringbuffer / sizeof ringbuffer[0]; n++)
			if (ringbuffer[n].payload)
				free(ringbuffer[n].payload);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (close_testing)
			break;
		while (pss->ringbuffer_tail != ringbuffer_head) {

			n = lws_write(wsi, (unsigned char *)
				   ringbuffer[pss->ringbuffer_tail].payload +
				   LWS_SEND_BUFFER_PRE_PADDING,
				   ringbuffer[pss->ringbuffer_tail].len,
								LWS_WRITE_TEXT);
			if (n < 0) {
				lwsl_err("ERROR %d writing to mirror socket\n", n);
				return -1;
			}
			if (n < (int)ringbuffer[pss->ringbuffer_tail].len)
				lwsl_err("mirror partial write %d vs %d\n",
				       n, ringbuffer[pss->ringbuffer_tail].len);

			if (pss->ringbuffer_tail == (MAX_MESSAGE_QUEUE - 1))
				pss->ringbuffer_tail = 0;
			else
				pss->ringbuffer_tail++;

			if (((ringbuffer_head - pss->ringbuffer_tail) &
				  (MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 15))
				lws_rx_flow_allow_all_protocol(lws_get_context(wsi),
					       lws_get_protocol(wsi));

			if (lws_partial_buffered(wsi) || lws_send_pipe_choked(wsi)) {
				lws_callback_on_writable(wsi);
				break;
			}
		}
		break;

	case LWS_CALLBACK_RECEIVE:
		if (((ringbuffer_head - pss->ringbuffer_tail) &
		    (MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 1)) {
			lwsl_err("dropping!\n");
			goto choke;
		}

		if (ringbuffer[ringbuffer_head].payload)
			free(ringbuffer[ringbuffer_head].payload);

		ringbuffer[ringbuffer_head].payload =
				malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
						  LWS_SEND_BUFFER_POST_PADDING);
		ringbuffer[ringbuffer_head].len = len;
		memcpy((char *)ringbuffer[ringbuffer_head].payload +
					  LWS_SEND_BUFFER_PRE_PADDING, in, len);
		if (ringbuffer_head == (MAX_MESSAGE_QUEUE - 1))
			ringbuffer_head = 0;
		else
			ringbuffer_head++;

		if (((ringbuffer_head - pss->ringbuffer_tail) &
		    (MAX_MESSAGE_QUEUE - 1)) != (MAX_MESSAGE_QUEUE - 2))
			goto done;

choke:
		lwsl_debug("LWS_CALLBACK_RECEIVE: throttling %p\n", wsi);
		lws_rx_flow_control(wsi, 0);

done:
		lws_callback_on_writable_all_protocol(lws_get_context(wsi),
					       lws_get_protocol(wsi));
		break;

	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info(wsi);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}
