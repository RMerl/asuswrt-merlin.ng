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

#include "mesh/mesh-defs.h"

#include "mesh/net-keys.h"
#include "mesh/net.h"
#include "mesh/model.h"
#include "mesh/util.h"

#include "mesh/friend.h"

#define MAX_FRND_GROUPS		20
#define FRND_RELAY_WINDOW	250		/* 250 ms */
#define FRND_CACHE_SIZE		16
#define FRND_SUB_LIST_SIZE	8

#define RESPONSE_DELAY		(100 - 12)	/*  100  ms - 12ms hw delay */
#define MIN_RESP_DELAY		10		/*   10  ms */
#define MAX_RESP_DELAY		255		/*  255  ms */

/* Absolute maximum time to wait for LPN to choose us. */
#define RESPONSE_POLL_DELAY	1300		/* 1.300  s */
static uint8_t frnd_relay_window = FRND_RELAY_WINDOW;
static uint8_t frnd_cache_size = FRND_CACHE_SIZE;
static uint8_t frnd_sublist_size = FRND_SUB_LIST_SIZE;
static uint16_t counter;
static struct l_queue *retired_lpns;

static void response_timeout(struct l_timeout *timeout, void *user_data)
{
	struct mesh_friend *neg = user_data;
	struct l_queue *negotiations = mesh_net_get_negotiations(neg->net);

	/* LPN did not choose us */
	l_debug("Did not win negotiation for %4.4x", neg->lp_addr);

	net_key_unref(neg->net_key_cur);
	net_key_unref(neg->net_key_upd);
	l_queue_remove(negotiations, neg);
	l_timeout_remove(timeout);
	l_free(neg);
}

static void response_delay(struct l_timeout *timeout, void *user_data)
{
	struct mesh_friend *neg = user_data;
	uint16_t net_idx = neg->net_idx;
	uint32_t key_id, seq;
	uint8_t msg[8];
	uint16_t n = 0;
	bool res;

	l_timeout_remove(timeout);

	/* Create key Set for this offer */
	res = mesh_net_get_key(neg->net, false, net_idx, &key_id);
	if (!res)
		goto cleanup;

	neg->net_key_cur = net_key_frnd_add(key_id, neg->lp_addr,
						mesh_net_get_address(neg->net),
						neg->lp_cnt, counter);
	if (!neg->net_key_cur)
		goto cleanup;

	neg->fn_cnt = counter++;

	msg[n++] = NET_OP_FRND_OFFER;
	msg[n++] = frnd_relay_window;
	msg[n++] = frnd_cache_size;
	msg[n++] = frnd_sublist_size;
	msg[n++] = neg->u.negotiate.rssi;
	l_put_be16(neg->fn_cnt, msg + n);
	n += 2;
	seq = mesh_net_next_seq_num(neg->net);
	print_packet("Tx-NET_OP_FRND_OFFER", msg, n);
	mesh_net_transport_send(neg->net, key_id, 0,
			mesh_net_get_iv_index(neg->net), 0,
			seq, 0, neg->lp_addr,
			msg, n);

	/* Offer expires in 1.3 seconds, which is the max time for LPN to
	 * receive all offers, 1 second to make decision, and a little extra
	 */
	neg->timeout = l_timeout_create_ms(1000 + MAX_RESP_DELAY,
						response_timeout, neg, NULL);

	return;

cleanup:
	net_key_unref(neg->net_key_cur);
	net_key_unref(neg->net_key_upd);
	l_queue_remove(mesh_net_get_negotiations(neg->net), neg);
	l_free(neg);
}

static uint8_t cache_size(uint8_t power)
{
	return 1 << power;
}

static bool match_by_lpn(const void *a, const void *b)
{
	const struct mesh_friend *neg = a;
	uint16_t lpn = L_PTR_TO_UINT(b);

	return neg->lp_addr == lpn;
}

/* Scaling factors in 1/10 ms */
static const int32_t scaling[] = {
	10,
	15,
	20,
	15,
};

void friend_request(struct mesh_net *net, uint16_t net_idx, uint16_t src,
		uint8_t minReq, uint8_t delay, uint32_t timeout,
		uint16_t prev, uint8_t num_ele, uint16_t cntr,
		int8_t rssi)
{
	struct l_queue *negotiations = mesh_net_get_negotiations(net);
	struct mesh_friend *neg;
	uint8_t rssiScale = (minReq >> 5) & 3;
	uint8_t winScale = (minReq >> 3) & 3;
	uint8_t minCache = (minReq >> 0) & 7;
	int32_t rsp_delay;

	l_debug("RSSI of Request: %d dbm", rssi);
	l_debug("Delay: %d ms", delay);
	l_debug("Poll Timeout of Request: %d ms", timeout * 100);
	l_debug("Previous Friend: %4.4x", prev);
	l_debug("Num Elem: %2.2x", num_ele);
	l_debug("Cache Requested: %d", cache_size(minCache));
	l_debug("Cache to offer: %d", frnd_cache_size);

	/* Determine our own suitability before
	 * deciding to participate in negotiation
	 */
	if (minCache == 0 || num_ele == 0)
		return;

	if (delay < 0x0A)
		return;

	if (timeout < 0x00000A || timeout > 0x34BBFF)
		return;

	if (cache_size(minCache) > frnd_cache_size)
		return;

	/* TODO: Check RSSI, and then start Negotiation if appropriate */

	/* We are participating in this Negotiation */
	neg = l_new(struct mesh_friend, 1);
	l_queue_push_head(negotiations, neg);

	neg->net = net;
	neg->lp_addr = src;
	neg->lp_cnt = cntr;
	neg->u.negotiate.rssi = rssi;
	neg->receive_delay = delay;
	neg->poll_timeout = timeout;
	neg->old_friend = prev;
	neg->ele_cnt = num_ele;
	neg->net_idx = net_idx;

	/* RSSI (Negative Factor, larger values == less time)
	 * Scaling factor 0-3 == multiplier of 1.0 - 2.5
	 * Minimum factor of 1. Bit 1 adds additional factor
	 * of 1, bit zero and additional 0.5
	 */
	rsp_delay = -(rssi * scaling[rssiScale]);
	l_debug("RSSI Factor: %d ms", rsp_delay / 10);

	/* Relay Window (Positive Factor, larger values == more time)
	 * Scaling factor 0-3 == multiplier of 1.0 - 2.5
	 * Minimum factor of 1. Bit 1 adds additional factor
	 * of 1, bit zero and additional 0.5
	 */
	rsp_delay += frnd_relay_window * scaling[winScale];
	l_debug("Win Size Factor: %d ms",
			(frnd_relay_window * scaling[winScale]) / 10);

	/* Normalize to ms */
	rsp_delay /= 10;

	/* Range limits are 10-255 ms */
	if (rsp_delay < MIN_RESP_DELAY)
		rsp_delay = MIN_RESP_DELAY;
	else if (rsp_delay > MAX_RESP_DELAY)
		rsp_delay = MAX_RESP_DELAY;

	l_debug("Total Response Delay: %d ms", rsp_delay);

	/* Add in 100ms delay before start of "Offer Period" */
	rsp_delay += RESPONSE_DELAY;

	neg->timeout = l_timeout_create_ms(rsp_delay,
						response_delay, neg, NULL);
}

void friend_clear_confirm(struct mesh_net *net, uint16_t src,
					uint16_t lpn, uint16_t lpnCounter)
{
	struct l_queue *negotiations = mesh_net_get_negotiations(net);
	struct mesh_friend *neg = l_queue_remove_if(negotiations,
					match_by_lpn, L_UINT_TO_PTR(lpn));

	l_debug("Friend Clear confirmed %4.4x (cnt %4.4x)", lpn, lpnCounter);

	if (!neg)
		return;

	l_timeout_remove(neg->timeout);
	l_queue_remove(negotiations, neg);
	l_free(neg);
}

static void friend_poll_timeout(struct l_timeout *timeout, void *user_data)
{
	struct mesh_friend *frnd = user_data;

	if (mesh_friend_clear(frnd->net, frnd))
		l_debug("Friend Poll Timeout %4.4x", frnd->lp_addr);

	l_timeout_remove(frnd->timeout);
	frnd->timeout = NULL;

	/* Friend may be in either Network or Retired list, so try both */
	l_queue_remove(retired_lpns, frnd);
	mesh_friend_free(frnd);
}

void friend_clear(struct mesh_net *net, uint16_t src, uint16_t lpn,
				uint16_t lpnCounter, struct mesh_friend *frnd)
{
	struct l_queue *negotiations = mesh_net_get_negotiations(net);
	uint8_t msg[5] = { NET_OP_FRND_CLEAR_CONFIRM };
	bool removed = false;
	uint16_t lpnDelta;

	if (frnd) {
		lpnDelta = lpnCounter - frnd->lp_cnt;

		/* Ignore old Friend Clear commands */
		if (lpnDelta > 0x100)
			return;

		/* Move friend from Network list to Retired list */
		removed = mesh_friend_clear(net, frnd);
		if (removed) {
			struct mesh_friend *neg, *old;

			neg = l_queue_remove_if(negotiations, match_by_lpn,
							L_UINT_TO_PTR(lpn));

			/* Cancel any negotiations or clears */
			if (neg) {
				l_timeout_remove(neg->timeout);
				l_free(neg);
			}

			/* Find any duplicates */
			old = l_queue_find(retired_lpns, match_by_lpn,
							L_UINT_TO_PTR(lpn));

			/* Force time-out of old friendship */
			if (old)
				friend_poll_timeout(old->timeout, old);

			if (!retired_lpns)
				retired_lpns = l_queue_new();

			/* Retire this LPN (keeps timeout running) */
			l_queue_push_tail(retired_lpns, frnd);
		}
	} else {
		frnd = l_queue_find(retired_lpns, match_by_lpn,
							L_UINT_TO_PTR(lpn));
		if (!frnd)
			return;

		lpnDelta = lpnCounter - frnd->lp_cnt;

		/* Ignore old Friend Clear commands */
		if (!lpnDelta || (lpnDelta > 0x100))
			return;
	}

	l_debug("Friend Cleared %4.4x (%4.4x)", lpn, lpnCounter);

	l_put_be16(lpn, msg + 1);
	l_put_be16(lpnCounter, msg + 3);
	mesh_net_transport_send(net, 0, 0,
			mesh_net_get_iv_index(net), DEFAULT_TTL,
			0, 0, src,
			msg, sizeof(msg));
}

static void clear_retry(struct l_timeout *timeout, void *user_data)
{
	struct mesh_friend *neg = user_data;
	struct l_queue *negotiations = mesh_net_get_negotiations(neg->net);
	uint8_t msg[5] = { NET_OP_FRND_CLEAR };
	uint32_t secs = 1 << neg->receive_delay;


	l_put_be16(neg->lp_addr, msg + 1);
	l_put_be16(neg->lp_cnt, msg + 3);
	mesh_net_transport_send(neg->net, 0, 0,
			mesh_net_get_iv_index(neg->net), DEFAULT_TTL,
			0, 0, neg->old_friend,
			msg, sizeof(msg));

	if (secs && ((secs << 1) < neg->poll_timeout/10)) {
		neg->receive_delay++;
		l_debug("Try FRND_CLR again in %d seconds (total timeout %d)",
						secs, neg->poll_timeout/10);
		l_timeout_modify(neg->timeout, secs);
	} else {
		l_debug("FRND_CLR timed out %d", secs);
		l_timeout_remove(timeout);
		l_queue_remove(negotiations, neg);
		l_free(neg);
	}
}

static void friend_delay_rsp(struct l_timeout *timeout, void *user_data)
{
	struct mesh_friend *frnd = user_data;
	struct mesh_friend_msg *pkt = frnd->pkt;
	struct mesh_net *net = frnd->net;
	uint32_t net_seq, iv_index;
	uint8_t upd[7] = { NET_OP_FRND_UPDATE };

	l_timeout_remove(timeout);

	if (pkt == NULL)
		goto update;

	if (pkt->ctl) {
		/* Make sure we don't change the bit-sense of MD,
		 * once it has been set because that would cause
		 * a "Dirty Nonce" security violation
		 */
		if (((pkt->u.one[0].hdr >> OPCODE_HDR_SHIFT) & OPCODE_MASK) ==
						NET_OP_SEG_ACKNOWLEDGE) {
			bool rly = !!((pkt->u.one[0].hdr >> RELAY_HDR_SHIFT) &
									true);
			uint16_t seqZero = pkt->u.one[0].hdr >>
							SEQ_ZERO_HDR_SHIFT;

			seqZero &= SEQ_ZERO_MASK;

			l_debug("Fwd ACK pkt %6.6x-%8.8x",
					pkt->u.one[0].seq,
					pkt->iv_index);

			pkt->u.one[0].sent = true;
			mesh_net_ack_send(net, frnd->net_key_cur,
					pkt->iv_index, pkt->ttl,
					pkt->u.one[0].seq, pkt->src, pkt->dst,
					rly, seqZero,
					l_get_be32(pkt->u.one[0].data));


		} else {
			l_debug("Fwd CTL pkt %6.6x-%8.8x",
					pkt->u.one[0].seq,
					pkt->iv_index);

			print_packet("Frnd-CTL",
					pkt->u.one[0].data, pkt->last_len);

			pkt->u.one[0].sent = true;
			mesh_net_transport_send(net, frnd->net_key_cur, 0,
					pkt->iv_index, pkt->ttl,
					pkt->u.one[0].seq, pkt->src, pkt->dst,
					pkt->u.one[0].data, pkt->last_len);
		}
	} else {
		/* If segments after this one, then More Data must be TRUE */
		uint8_t len;

		if (pkt->cnt_out < pkt->cnt_in)
			len = sizeof(pkt->u.s12[0].data);
		else
			len = pkt->last_len;

		l_debug("Fwd FRND pkt %6.6x",
				pkt->u.s12[pkt->cnt_out].seq);

		print_packet("Frnd-Msg", pkt->u.s12[pkt->cnt_out].data, len);

		pkt->u.s12[pkt->cnt_out].sent = true;
		mesh_net_send_seg(net, frnd->net_key_cur,
				pkt->iv_index,
				pkt->ttl,
				pkt->u.s12[pkt->cnt_out].seq,
				pkt->src, pkt->dst,
				pkt->u.s12[pkt->cnt_out].hdr,
				pkt->u.s12[pkt->cnt_out].data, len);
	}

	return;

update:
	/* No More Data -- send Update message with md = false */
	net_seq = mesh_net_get_seq_num(net);
	l_debug("Fwd FRND UPDATE %6.6x with MD == 0", net_seq);

	frnd->u.active.last = frnd->u.active.seq;
	mesh_net_get_snb_state(net, upd + 1, &iv_index);
	l_put_be32(iv_index, upd + 2);
	upd[6] = false; /* Queue is Empty */
	print_packet("Update", upd, sizeof(upd));
	mesh_net_transport_send(net, frnd->net_key_cur, 0,
			mesh_net_get_iv_index(net), 0,
			net_seq, 0, frnd->lp_addr,
			upd, sizeof(upd));
	mesh_net_next_seq_num(net);
}


void friend_poll(struct mesh_net *net, uint16_t src, bool seq,
					struct mesh_friend *frnd)
{
	struct l_queue *negotiations = mesh_net_get_negotiations(net);
	struct mesh_friend *neg;
	struct mesh_friend_msg *pkt;
	bool md;

	l_debug("POLL-RXED");
	neg = l_queue_find(negotiations, match_by_lpn, L_UINT_TO_PTR(src));

	if (neg && !neg->u.negotiate.clearing) {
		uint8_t msg[5] = { NET_OP_FRND_CLEAR };

		l_debug("Won negotiation for %4.4x", neg->lp_addr);

		/* This call will clean-up and replace if already friends */
		frnd = mesh_friend_new(net, src, neg->ele_cnt,
						neg->receive_delay,
						neg->frw,
						neg->poll_timeout,
						neg->fn_cnt, neg->lp_cnt);

		frnd->timeout = l_timeout_create_ms(
					frnd->poll_timeout * 100,
					friend_poll_timeout, frnd, NULL);

		l_timeout_remove(neg->timeout);
		net_key_unref(neg->net_key_cur);
		net_key_unref(neg->net_key_upd);
		neg->net_key_upd = neg->net_key_cur = 0;

		if (neg->old_friend == 0 ||
				neg->old_friend == mesh_net_get_address(net)) {
			l_queue_remove(negotiations, neg);
			l_free(neg);
		} else {
			neg->u.negotiate.clearing = true;
			l_put_be16(neg->lp_addr, msg + 1);
			l_put_be16(neg->lp_cnt, msg + 3);
			mesh_net_transport_send(net, 0, 0,
					mesh_net_get_iv_index(net), DEFAULT_TTL,
					0, 0, neg->old_friend,
					msg, sizeof(msg));

			/* Reuse receive_delay as a shift counter to
			 * time-out FRIEND_CLEAR
			 */
			neg->receive_delay = 1;
			neg->timeout = l_timeout_create(1, clear_retry,
								neg, NULL);
		}
	}

	if (!frnd)
		return;

	/* Reset Poll Timeout */
	l_timeout_modify_ms(frnd->timeout, frnd->poll_timeout * 100);

	if (!l_queue_length(frnd->pkt_cache))
		goto update;

	if (frnd->u.active.seq != frnd->u.active.last &&
						frnd->u.active.seq != seq) {
		pkt = l_queue_peek_head(frnd->pkt_cache);
		if (pkt->cnt_out < pkt->cnt_in) {
			pkt->cnt_out++;
		} else {
			pkt = l_queue_pop_head(frnd->pkt_cache);
			l_free(pkt);
		}
	}

	pkt = l_queue_peek_head(frnd->pkt_cache);

	if (!pkt)
		goto update;

	frnd->u.active.seq = seq;
	frnd->u.active.last = !seq;
	md = !!(l_queue_length(frnd->pkt_cache) > 1);

	if (pkt->ctl) {
		/* Make sure we don't change the bit-sense of MD,
		 * once it has been set because that would cause
		 * a "Dirty Nonce" security violation
		 */
		if (!(pkt->u.one[0].sent))
			pkt->u.one[0].md = md;
	} else {
		/* If segments after this one, then More Data must be TRUE */
		if (pkt->cnt_out < pkt->cnt_in)
			md = true;

		/* Make sure we don't change the bit-sense of MD, once
		 * it has been set because that would cause a
		 * "Dirty Nonce" security violation
		 */
		if (!(pkt->u.s12[pkt->cnt_out].sent))
			pkt->u.s12[pkt->cnt_out].md = md;
	}
	frnd->pkt = pkt;
	l_timeout_create_ms(frnd->frd, friend_delay_rsp, frnd, NULL);

	return;

update:
	frnd->pkt = NULL;
	l_timeout_create_ms(frnd->frd, friend_delay_rsp, frnd, NULL);
}

void friend_sub_add(struct mesh_net *net, struct mesh_friend *frnd,
					const uint8_t *pkt, uint8_t len)
{
	uint16_t *new_list;
	uint32_t net_seq;
	uint8_t plen = len;
	uint8_t msg[] = { NET_OP_PROXY_SUB_CONFIRM, 0 };

	if (!frnd || MAX_FRND_GROUPS < frnd->u.active.grp_cnt + (len/2))
		return;

	msg[1] = *pkt++;
	plen--;

	/* Sanity Check Values, abort if any illegal */
	while (plen >= 2) {
		plen -= 2;
		if (l_get_be16(pkt + plen) < 0x8000)
			return;
	}

	new_list = l_malloc(frnd->u.active.grp_cnt * sizeof(uint16_t) + len);
	if (frnd->u.active.grp_list)
		memcpy(new_list, frnd->u.active.grp_list,
				frnd->u.active.grp_cnt * sizeof(uint16_t));

	while (len >= 2) {
		new_list[frnd->u.active.grp_cnt++] = l_get_be16(pkt);
		pkt += 2;
		len -= 2;
	}

	l_free(frnd->u.active.grp_list);
	frnd->u.active.grp_list = new_list;

	print_packet("Tx-NET_OP_PROXY_SUB_CONFIRM", msg, sizeof(msg));
	net_seq = mesh_net_get_seq_num(net);
	mesh_net_transport_send(net, frnd->net_key_cur, 0,
			mesh_net_get_iv_index(net), 0,
			net_seq, 0, frnd->lp_addr,
			msg, sizeof(msg));
	mesh_net_next_seq_num(net);
}

void friend_sub_del(struct mesh_net *net, struct mesh_friend *frnd,
					const uint8_t *pkt, uint8_t len)
{
	uint32_t net_seq;
	uint8_t msg[] = { NET_OP_PROXY_SUB_CONFIRM, 0 };
	int i;

	if (!frnd)
		return;

	msg[1] = *pkt++;
	len--;

	while (len >= 2) {
		uint16_t grp = l_get_be16(pkt);

		for (i = frnd->u.active.grp_cnt - 1; i >= 0; i--) {
			if (frnd->u.active.grp_list[i] == grp) {
				frnd->u.active.grp_cnt--;
				memcpy(&frnd->u.active.grp_list[i],
					&frnd->u.active.grp_list[i + 1],
					(frnd->u.active.grp_cnt - i) * 2);
				break;
			}
		}
		len -= 2;
		pkt += 2;
	}

	print_packet("Tx-NET_OP_PROXY_SUB_CONFIRM", msg, sizeof(msg));
	net_seq = mesh_net_get_seq_num(net);
	mesh_net_transport_send(net, frnd->net_key_cur, 0,
			mesh_net_get_iv_index(net), 0,
			net_seq, 0, frnd->lp_addr,
			msg, sizeof(msg));
	mesh_net_next_seq_num(net);
}
