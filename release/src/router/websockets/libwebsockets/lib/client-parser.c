/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2014 Andy Green <andy@warmcat.com>
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

#include "private-libwebsockets.h"

int lws_client_rx_sm(struct lws *wsi, unsigned char c)
{
	int callback_action = LWS_CALLBACK_CLIENT_RECEIVE;
	struct lws_tokens eff_buf;
	int handled, m;

	switch (wsi->lws_rx_parse_state) {
	case LWS_RXPS_NEW:

		switch (wsi->ietf_spec_revision) {

		case 13:
			wsi->u.ws.opcode = c & 0xf;
			wsi->u.ws.rsv = (c & 0x70);
			wsi->u.ws.final = !!((c >> 7) & 1);
			switch (wsi->u.ws.opcode) {
			case LWSWSOPC_TEXT_FRAME:
			case LWSWSOPC_BINARY_FRAME:
				wsi->u.ws.frame_is_binary = wsi->u.ws.opcode ==
						 LWSWSOPC_BINARY_FRAME;
				break;
			}
			wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN;
			break;

		default:
			lwsl_err("unknown spec version %02d\n",
						       wsi->ietf_spec_revision);
			break;
		}
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN:

		wsi->u.ws.this_frame_masked = !!(c & 0x80);

		switch (c & 0x7f) {
		case 126:
			/* control frames are not allowed to have big lengths */
			if (wsi->u.ws.opcode & 8)
				goto illegal_ctl_length;
			wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN16_2;
			break;
		case 127:
			/* control frames are not allowed to have big lengths */
			if (wsi->u.ws.opcode & 8)
				goto illegal_ctl_length;
			wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_8;
			break;
		default:
			wsi->u.ws.rx_packet_length = c;
			if (wsi->u.ws.this_frame_masked)
				wsi->lws_rx_parse_state =
						LWS_RXPS_07_COLLECT_FRAME_KEY_1;
			else {
				if (c)
					wsi->lws_rx_parse_state =
					LWS_RXPS_PAYLOAD_UNTIL_LENGTH_EXHAUSTED;
				else {
					wsi->lws_rx_parse_state = LWS_RXPS_NEW;
					goto spill;
				}
			}
			break;
		}
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN16_2:
		wsi->u.ws.rx_packet_length = c << 8;
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN16_1;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN16_1:
		wsi->u.ws.rx_packet_length |= c;
		if (wsi->u.ws.this_frame_masked)
			wsi->lws_rx_parse_state =
					LWS_RXPS_07_COLLECT_FRAME_KEY_1;
		else {
			if (wsi->u.ws.rx_packet_length)
				wsi->lws_rx_parse_state =
					LWS_RXPS_PAYLOAD_UNTIL_LENGTH_EXHAUSTED;
			else {
				wsi->lws_rx_parse_state = LWS_RXPS_NEW;
				goto spill;
			}
		}
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_8:
		if (c & 0x80) {
			lwsl_warn("b63 of length must be zero\n");
			/* kill the connection */
			return -1;
		}
#if defined __LP64__
		wsi->u.ws.rx_packet_length = ((size_t)c) << 56;
#else
		wsi->u.ws.rx_packet_length = 0;
#endif
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_7;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_7:
#if defined __LP64__
		wsi->u.ws.rx_packet_length |= ((size_t)c) << 48;
#endif
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_6;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_6:
#if defined __LP64__
		wsi->u.ws.rx_packet_length |= ((size_t)c) << 40;
#endif
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_5;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_5:
#if defined __LP64__
		wsi->u.ws.rx_packet_length |= ((size_t)c) << 32;
#endif
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_4;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_4:
		wsi->u.ws.rx_packet_length |= ((size_t)c) << 24;
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_3;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_3:
		wsi->u.ws.rx_packet_length |= ((size_t)c) << 16;
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_2;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_2:
		wsi->u.ws.rx_packet_length |= ((size_t)c) << 8;
		wsi->lws_rx_parse_state = LWS_RXPS_04_FRAME_HDR_LEN64_1;
		break;

	case LWS_RXPS_04_FRAME_HDR_LEN64_1:
		wsi->u.ws.rx_packet_length |= (size_t)c;
		if (wsi->u.ws.this_frame_masked)
			wsi->lws_rx_parse_state =
					LWS_RXPS_07_COLLECT_FRAME_KEY_1;
		else {
			if (wsi->u.ws.rx_packet_length)
				wsi->lws_rx_parse_state =
					LWS_RXPS_PAYLOAD_UNTIL_LENGTH_EXHAUSTED;
			else {
				wsi->lws_rx_parse_state = LWS_RXPS_NEW;
				goto spill;
			}
		}
		break;

	case LWS_RXPS_07_COLLECT_FRAME_KEY_1:
		wsi->u.ws.mask_nonce[0] = c;
		if (c)
			wsi->u.ws.all_zero_nonce = 0;
		wsi->lws_rx_parse_state = LWS_RXPS_07_COLLECT_FRAME_KEY_2;
		break;

	case LWS_RXPS_07_COLLECT_FRAME_KEY_2:
		wsi->u.ws.mask_nonce[1] = c;
		if (c)
			wsi->u.ws.all_zero_nonce = 0;
		wsi->lws_rx_parse_state = LWS_RXPS_07_COLLECT_FRAME_KEY_3;
		break;

	case LWS_RXPS_07_COLLECT_FRAME_KEY_3:
		wsi->u.ws.mask_nonce[2] = c;
		if (c)
			wsi->u.ws.all_zero_nonce = 0;
		wsi->lws_rx_parse_state = LWS_RXPS_07_COLLECT_FRAME_KEY_4;
		break;

	case LWS_RXPS_07_COLLECT_FRAME_KEY_4:
		wsi->u.ws.mask_nonce[3] = c;
		if (c)
			wsi->u.ws.all_zero_nonce = 0;

		if (wsi->u.ws.rx_packet_length)
			wsi->lws_rx_parse_state =
					LWS_RXPS_PAYLOAD_UNTIL_LENGTH_EXHAUSTED;
		else {
			wsi->lws_rx_parse_state = LWS_RXPS_NEW;
			goto spill;
		}
		break;

	case LWS_RXPS_PAYLOAD_UNTIL_LENGTH_EXHAUSTED:

		if (!wsi->u.ws.rx_user_buffer) {
			lwsl_err("NULL client rx_user_buffer\n");
			return 1;
		}

		if ((!wsi->u.ws.this_frame_masked) || wsi->u.ws.all_zero_nonce)
			wsi->u.ws.rx_user_buffer[LWS_SEND_BUFFER_PRE_PADDING +
			       (wsi->u.ws.rx_user_buffer_head++)] = c;
		else
			wsi->u.ws.rx_user_buffer[LWS_SEND_BUFFER_PRE_PADDING +
			       (wsi->u.ws.rx_user_buffer_head++)] =
			c ^ wsi->u.ws.mask_nonce[
					    (wsi->u.ws.frame_mask_index++) & 3];

		if (--wsi->u.ws.rx_packet_length == 0) {
			/* spill because we have the whole frame */
			wsi->lws_rx_parse_state = LWS_RXPS_NEW;
			goto spill;
		}

		/*
		 * if there's no protocol max frame size given, we are
		 * supposed to default to LWS_MAX_SOCKET_IO_BUF
		 */

		if (!wsi->protocol->rx_buffer_size &&
			 		wsi->u.ws.rx_user_buffer_head !=
			 				  LWS_MAX_SOCKET_IO_BUF)
			break;
		else
			if (wsi->protocol->rx_buffer_size &&
					wsi->u.ws.rx_user_buffer_head !=
						  wsi->protocol->rx_buffer_size)
			break;

		/* spill because we filled our rx buffer */
spill:

		handled = 0;

		/*
		 * is this frame a control packet we should take care of at this
		 * layer?  If so service it and hide it from the user callback
		 */

		switch (wsi->u.ws.opcode) {
		case LWSWSOPC_CLOSE:
			/* is this an acknowledgement of our close? */
			if (wsi->state == LWSS_AWAITING_CLOSE_ACK) {
				/*
				 * fine he has told us he is closing too, let's
				 * finish our close
				 */
				lwsl_parser("seen server's close ack\n");
				return -1;
			}
			lwsl_parser("client sees server close len = %d\n",
						 wsi->u.ws.rx_user_buffer_head);
			/*
			 * parrot the close packet payload back
			 * we do not care about how it went, we are closing
			 * immediately afterwards
			 */
			lws_write(wsi, (unsigned char *)
			   &wsi->u.ws.rx_user_buffer[
				LWS_SEND_BUFFER_PRE_PADDING],
				wsi->u.ws.rx_user_buffer_head, LWS_WRITE_CLOSE);
			wsi->state = LWSS_RETURNED_CLOSE_ALREADY;
			/* close the connection */
			return -1;

		case LWSWSOPC_PING:
			lwsl_info("received %d byte ping, sending pong\n",
				  wsi->u.ws.rx_user_buffer_head);

			if (wsi->u.ws.ping_pending_flag) {
				/*
				 * there is already a pending ping payload
				 * we should just log and drop
				 */
				lwsl_parser("DROP PING since one pending\n");
				goto ping_drop;
			}

			/* control packets can only be < 128 bytes long */
			if (wsi->u.ws.rx_user_buffer_head > 128 - 4) {
				lwsl_parser("DROP PING payload too large\n");
				goto ping_drop;
			}

			/* if existing buffer is too small, drop it */
			if (wsi->u.ws.ping_payload_buf &&
			    wsi->u.ws.ping_payload_alloc <
			    wsi->u.ws.rx_user_buffer_head)
				lws_free_set_NULL(wsi->u.ws.ping_payload_buf);

			/* if no buffer, allocate it */
			if (!wsi->u.ws.ping_payload_buf) {
				wsi->u.ws.ping_payload_buf =
					lws_malloc(wsi->u.ws.rx_user_buffer_head +
						   LWS_SEND_BUFFER_PRE_PADDING);
				wsi->u.ws.ping_payload_alloc =
					wsi->u.ws.rx_user_buffer_head;
			}

			/* stash the pong payload */
			memcpy(wsi->u.ws.ping_payload_buf + LWS_SEND_BUFFER_PRE_PADDING,
			       &wsi->u.ws.rx_user_buffer[LWS_SEND_BUFFER_PRE_PADDING],
				wsi->u.ws.rx_user_buffer_head);

			wsi->u.ws.ping_payload_len = wsi->u.ws.rx_user_buffer_head;
			wsi->u.ws.ping_pending_flag = 1;

			/* get it sent as soon as possible */
			lws_callback_on_writable(wsi);
ping_drop:
			wsi->u.ws.rx_user_buffer_head = 0;
			handled = 1;
			break;

		case LWSWSOPC_PONG:
			lwsl_info("client receied pong\n");
			lwsl_hexdump(&wsi->u.ws.rx_user_buffer[LWS_SEND_BUFFER_PRE_PADDING],
				     wsi->u.ws.rx_user_buffer_head);

			/* issue it */
			callback_action = LWS_CALLBACK_CLIENT_RECEIVE_PONG;
			break;

		case LWSWSOPC_CONTINUATION:
		case LWSWSOPC_TEXT_FRAME:
		case LWSWSOPC_BINARY_FRAME:
			break;

		default:

			lwsl_parser("Reserved opc 0x%2X\n", wsi->u.ws.opcode);

			/*
			 * It's something special we can't understand here.
			 * Pass the payload up to the extension's parsing
			 * state machine.
			 */

			eff_buf.token = &wsi->u.ws.rx_user_buffer[
						   LWS_SEND_BUFFER_PRE_PADDING];
			eff_buf.token_len = wsi->u.ws.rx_user_buffer_head;

			if (lws_ext_cb_wsi_active_exts(wsi,
				LWS_EXT_CALLBACK_EXTENDED_PAYLOAD_RX,
					&eff_buf, 0) <= 0) { /* not handle or fail */

				lwsl_ext("Unhandled ext opc 0x%x\n",
					 wsi->u.ws.opcode);
				wsi->u.ws.rx_user_buffer_head = 0;

				return 0;
			}
			handled = 1;
			break;
		}

		/*
		 * No it's real payload, pass it up to the user callback.
		 * It's nicely buffered with the pre-padding taken care of
		 * so it can be sent straight out again using lws_write
		 */
		if (handled)
			goto already_done;

		eff_buf.token = &wsi->u.ws.rx_user_buffer[
						LWS_SEND_BUFFER_PRE_PADDING];
		eff_buf.token_len = wsi->u.ws.rx_user_buffer_head;

		if (lws_ext_cb_wsi_active_exts(wsi,
						     LWS_EXT_CALLBACK_PAYLOAD_RX,
						     &eff_buf, 0) < 0) /* fail */
			return -1;

		if (eff_buf.token_len <= 0 &&
		    callback_action != LWS_CALLBACK_CLIENT_RECEIVE_PONG)
			goto already_done;

		eff_buf.token[eff_buf.token_len] = '\0';

		if (!wsi->protocol->callback)
			goto already_done;

		if (callback_action == LWS_CALLBACK_CLIENT_RECEIVE_PONG)
			lwsl_info("Client doing pong callback\n");

		m = wsi->protocol->callback(wsi,
			(enum lws_callback_reasons)callback_action,
			wsi->user_space, eff_buf.token, eff_buf.token_len);

		/* if user code wants to close, let caller know */
		if (m)
			return 1;

already_done:
		wsi->u.ws.rx_user_buffer_head = 0;
		break;
	default:
		lwsl_err("client rx illegal state\n");
		return 1;
	}

	return 0;

illegal_ctl_length:

	lwsl_warn("Control frame asking for extended length is illegal\n");
	/* kill the connection */
	return -1;
}


