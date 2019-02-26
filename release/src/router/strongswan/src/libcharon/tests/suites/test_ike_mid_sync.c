/*
 * Copyright (C) 2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include <tests/utils/exchange_test_helper.h>
#include <tests/utils/exchange_test_asserts.h>
#include <tests/utils/sa_asserts.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

/**
 * FIXME: Since we don't have the server side yet, this is kind of a hack!!!
 */

/**
 * Add the IKEV2_MESSAGE_ID_SYNC_SUPPORTED notify to the IKE_AUTH response
 */
static bool add_notify(listener_t *listener, ike_sa_t *ike_sa,
					   message_t *message, bool incoming, bool plain)
{
	if (plain && !incoming && message->get_exchange_type(message) == IKE_AUTH &&
		!message->get_request(message))
	{
		message->add_notify(message, FALSE, IKEV2_MESSAGE_ID_SYNC_SUPPORTED,
							chunk_empty);
		return FALSE;
	}
	return TRUE;
}
#define add_notify_to_ike_auth() ({ \
	listener_t _notify_listener = { \
		.message = add_notify, \
	}; \
	exchange_test_helper->add_listener(exchange_test_helper, &_notify_listener); \
})

/**
 * Handle IKEV2_MESSAGE_ID_SYNC notifies
 */
typedef struct {
	listener_t listener;
	struct {
		chunk_t nonce;
		uint32_t send;
		uint32_t recv;
	} init, resp;
} mid_sync_listener_t;

static bool handle_mid(listener_t *listener,
				ike_sa_t *ike_sa, message_t *message, bool incoming, bool plain)
{
	mid_sync_listener_t *this = (mid_sync_listener_t*)listener;

	if (!plain || incoming)
	{
		return TRUE;
	}

	if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		if (streq("resp", ike_sa->get_name(ike_sa)))
		{
			bio_writer_t *writer;
			rng_t *rng;

			rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
			ignore_result(rng->allocate_bytes(rng, 4, &this->init.nonce));
			rng->destroy(rng);
			writer = bio_writer_create(12);
			writer->write_data(writer, this->init.nonce);
			writer->write_uint32(writer, this->init.send);
			writer->write_uint32(writer, this->init.recv);
			message->set_message_id(message, 0);
			message->add_notify(message, FALSE, IKEV2_MESSAGE_ID_SYNC,
								writer->get_buf(writer));
			writer->destroy(writer);
		}
		else
		{
			notify_payload_t *notify;
			bio_reader_t *reader;

			notify = message->get_notify(message, IKEV2_MESSAGE_ID_SYNC);
			reader = bio_reader_create(notify->get_notification_data(notify));
			chunk_clear(&this->resp.nonce);
			reader->read_data(reader, 4, &this->resp.nonce);
			this->resp.nonce = chunk_clone(this->resp.nonce);
			reader->read_uint32(reader, &this->resp.send);
			reader->read_uint32(reader, &this->resp.recv);
			reader->destroy(reader);
		}
	}
	return TRUE;
}

/**
 * Send a MESSAGE_ID_SYNC notify in an INFORMATIONAL.  We reset the state
 * afterwards so this seems as if nothing happened.
 */
static void send_mid_sync(ike_sa_t *sa, uint32_t send, uint32_t recv)
{
	call_ikesa(sa, send_dpd);
	sa->set_message_id(sa, TRUE, send);
	sa->set_message_id(sa, FALSE, recv);
	sa->flush_queue(sa, TASK_QUEUE_QUEUED);
}

/**
 * Send a regular DPD from one IKE_SA to another
 */
static void send_dpd(ike_sa_t *from, ike_sa_t *to)
{
	uint32_t send, recv;

	send = from->get_message_id(from, TRUE);
	recv = to->get_message_id(to, FALSE);
	call_ikesa(from, send_dpd);
	exchange_test_helper->process_message(exchange_test_helper, to, NULL);
	exchange_test_helper->process_message(exchange_test_helper, from, NULL);
	ck_assert_int_eq(send + 1, from->get_message_id(from, TRUE));
	ck_assert_int_eq(recv + 1, to->get_message_id(to, FALSE));
}

/**
 * Send a number of DPDs from on IKE_SA to the other
 */
static void send_dpds(ike_sa_t *from, ike_sa_t *to, int count)
{
	while (count--)
	{
		send_dpd(from, to);
	}
}

static struct {
	int dpds_a, dpds_b;
	uint32_t send, recv;
} data[] = {
	{ 0, 0, 0, 2 },
	{ 0, 0, 1, 3 },
	{ 1, 0, 0, 3 },
	{ 1, 0, 5, 8 },
	{ 0, 1, 1, 2 },
	{ 0, 1, 2, 2 },
	{ 1, 1, 1, 3 },
	{ 1, 1, 2, 4 },
	{ 1, 2, 2, 4 },
};

/**
 * The responder syncs message IDs with the initiator
 */
START_TEST(test_responder)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data[_i].send,
			.recv = data[_i].recv,
		},
	};

	add_notify_to_ike_auth();
	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data[_i].dpds_a);
	send_dpds(b, a, data[_i].dpds_b);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data[_i].send, data[_i].recv);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_chunk_eq(mid.init.nonce, mid.resp.nonce);
	ck_assert_int_eq(data[_i].recv, mid.resp.send);
	ck_assert_int_eq(data[_i].send, mid.resp.recv);
	ck_assert_int_eq(data[_i].recv, a->get_message_id(a, TRUE));
	ck_assert_int_eq(data[_i].send, a->get_message_id(a, FALSE));
	/* this currently won't be handled */
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	charon->bus->remove_listener(charon->bus, &mid.listener);

	send_dpd(a, b);
	send_dpd(b, a);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
	chunk_free(&mid.resp.nonce);
}
END_TEST

/**
 * Make sure a retransmit is handled properly.
 */
START_TEST(test_retransmit)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data[_i].send,
			.recv = data[_i].recv,
		},
	};
	message_t *msg, *retransmit;

	add_notify_to_ike_auth();
	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data[_i].dpds_a);
	send_dpds(b, a, data[_i].dpds_b);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data[_i].send, data[_i].recv);
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	retransmit = message_create_from_packet(msg->get_packet(msg));
	retransmit->parse_header(retransmit);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);
	exchange_test_helper->process_message(exchange_test_helper, a, retransmit);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	charon->bus->remove_listener(charon->bus, &mid.listener);

	send_dpd(a, b);
	send_dpd(b, a);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
	chunk_free(&mid.resp.nonce);
}
END_TEST

/**
 * Make sure a replayed or delayed notify is ignored.
 */
START_TEST(test_replay)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data[_i].send,
			.recv = data[_i].recv,
		},
	};
	message_t *msg, *replay;

	add_notify_to_ike_auth();
	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data[_i].dpds_a);
	send_dpds(b, a, data[_i].dpds_b);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data[_i].send, data[_i].recv);
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	replay = message_create_from_packet(msg->get_packet(msg));
	replay->parse_header(replay);
	exchange_test_helper->process_message(exchange_test_helper, a, msg);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	charon->bus->remove_listener(charon->bus, &mid.listener);

	send_dpd(a, b);
	send_dpd(b, a);

	exchange_test_helper->process_message(exchange_test_helper, a, replay);
	ck_assert(!exchange_test_helper->sender->dequeue(exchange_test_helper->sender));

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
	chunk_free(&mid.resp.nonce);
}
END_TEST

/**
 * Make sure the notify is ignored if the extension is not enabled.
 */
START_TEST(test_disabled)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data[_i].send,
			.recv = data[_i].recv,
		},
	};

	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data[_i].dpds_a);
	send_dpds(b, a, data[_i].dpds_b);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data[_i].dpds_b, UINT_MAX);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	/* we don't expect a response and unchanged MIDs */
	ck_assert(!exchange_test_helper->sender->dequeue(exchange_test_helper->sender));
	ck_assert_int_eq(2 + data[_i].dpds_a, a->get_message_id(a, TRUE));
	ck_assert_int_eq(data[_i].dpds_b, a->get_message_id(a, FALSE));
	charon->bus->remove_listener(charon->bus, &mid.listener);

	send_dpd(a, b);
	send_dpd(b, a);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
	chunk_free(&mid.resp.nonce);
}
END_TEST

static struct {
	int dpds_a, dpds_b;
	uint32_t send, recv;
} data_too_low[] = {
	{ 0, 1, 0, 2 },
	{ 1, 2, 0, 0 },
	{ 1, 2, 1, 3 },
};

/**
 * The responder syncs message IDs with the initiator but uses too low sender
 * MIDs so the initiator ignores the notify.
 */
START_TEST(test_sender_too_low)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data_too_low[_i].send,
			.recv = data_too_low[_i].recv,
		},
	};

	add_notify_to_ike_auth();
	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data_too_low[_i].dpds_a);
	send_dpds(b, a, data_too_low[_i].dpds_b);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data_too_low[_i].dpds_b, UINT_MAX);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	/* we don't expect a response and unchanged MIDs */
	ck_assert(!exchange_test_helper->sender->dequeue(exchange_test_helper->sender));
	ck_assert_int_eq(2 + data_too_low[_i].dpds_a, a->get_message_id(a, TRUE));
	ck_assert_int_eq(data_too_low[_i].dpds_b, a->get_message_id(a, FALSE));
	charon->bus->remove_listener(charon->bus, &mid.listener);

	send_dpd(a, b);
	send_dpd(b, a);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
}
END_TEST

static struct {
	int dpds_a, dpds_b;
	uint32_t send, recv;
	/* reversed so the table below is clearer */
	uint32_t recv_exp, send_exp;
} data_recv_update[] = {
	{ 0, 0, 0, 0, 0, 2 },
	{ 0, 0, 0, 1, 0, 2 },
	{ 0, 0, 1, 1, 1, 2 },
	{ 1, 0, 0, 1, 0, 3 },
	{ 1, 0, 5, 2, 5, 3 },
};

/**
 * The responder syncs message IDs with the initiator but uses too low receiver
 * MID, which is updated by the initiator in the response.
 */
START_TEST(test_recv_update)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data_recv_update[_i].send,
			.recv = data_recv_update[_i].recv,
		},
	};

	add_notify_to_ike_auth();
	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data_recv_update[_i].dpds_a);
	send_dpds(b, a, data_recv_update[_i].dpds_b);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data_recv_update[_i].send, data_recv_update[_i].recv);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_chunk_eq(mid.init.nonce, mid.resp.nonce);
	ck_assert_int_eq(data_recv_update[_i].send_exp, mid.resp.send);
	ck_assert_int_eq(data_recv_update[_i].recv_exp, mid.resp.recv);
	ck_assert_int_eq(data_recv_update[_i].send_exp, a->get_message_id(a, TRUE));
	ck_assert_int_eq(data_recv_update[_i].recv_exp, a->get_message_id(a, FALSE));
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	charon->bus->remove_listener(charon->bus, &mid.listener);
	/* fake the receipt of the notify */
	b->set_message_id(b, TRUE, data_recv_update[_i].recv_exp);
	b->set_message_id(b, FALSE, data_recv_update[_i].send_exp);

	send_dpd(a, b);
	send_dpd(b, a);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
	chunk_free(&mid.resp.nonce);
}
END_TEST

static struct {
	int dpds_a, dpds_b;
	uint32_t send, recv;
	/* reversed so the table below is clearer */
	uint32_t recv_exp, send_exp;
} data_active[] = {
	{ 0, 0, 0, 2, 0, 3 },
	{ 0, 0, 1, 3, 1, 3 },
	{ 1, 0, 0, 3, 0, 4 },
	{ 1, 0, 5, 8, 5, 8 },
	{ 0, 1, 1, 2, 1, 3 },
	{ 0, 1, 2, 2, 2, 2 },
	{ 1, 1, 1, 3, 1, 4 },
	{ 1, 1, 2, 4, 2, 4 },
};

/**
 * The responder syncs message IDs with the initiator that waits for the
 * response for an active task.
 */
START_TEST(test_active)
{
	ike_sa_t *a, *b;
	mid_sync_listener_t mid = {
		.listener = { .message = (void*)handle_mid, },
		.init = {
			.send = data_active[_i].send,
			.recv = data_active[_i].recv,
		},
	};
	message_t *msg;

	add_notify_to_ike_auth();
	exchange_test_helper->establish_sa(exchange_test_helper,
									   &a, &b, NULL);

	send_dpds(a, b, data_active[_i].dpds_a);
	send_dpds(b, a, data_active[_i].dpds_b);

	call_ikesa(a, send_dpd);
	msg = exchange_test_helper->sender->dequeue(exchange_test_helper->sender);
	msg->destroy(msg);

	exchange_test_helper->add_listener(exchange_test_helper, &mid.listener);
	send_mid_sync(b, data_active[_i].recv_exp, data_active[_i].send_exp);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	ck_assert_chunk_eq(mid.init.nonce, mid.resp.nonce);
	ck_assert_int_eq(data_active[_i].send_exp, mid.resp.send);
	ck_assert_int_eq(data_active[_i].recv_exp, mid.resp.recv);
	ck_assert_int_eq(data_active[_i].send_exp, a->get_message_id(a, TRUE));
	ck_assert_int_eq(data_active[_i].recv_exp, a->get_message_id(a, FALSE));
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	charon->bus->remove_listener(charon->bus, &mid.listener);

	/* the active task was queued again */
	call_ikesa(a, initiate, NULL, 0, NULL, NULL);
	exchange_test_helper->process_message(exchange_test_helper, b, NULL);
	exchange_test_helper->process_message(exchange_test_helper, a, NULL);
	send_dpd(b, a);

	call_ikesa(a, destroy);
	call_ikesa(b, destroy);
	chunk_free(&mid.init.nonce);
	chunk_free(&mid.resp.nonce);
}
END_TEST

Suite *ike_mid_sync_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ike MID sync");

	tc = tcase_create("responder");
	tcase_add_loop_test(tc, test_responder, 0, countof(data));
	tcase_add_loop_test(tc, test_retransmit, 0, countof(data));
	tcase_add_loop_test(tc, test_replay, 0, countof(data));
	tcase_add_loop_test(tc, test_disabled, 0, countof(data));
	suite_add_tcase(s, tc);

	tc = tcase_create("sender MID too low");
	tcase_add_loop_test(tc, test_sender_too_low, 0, countof(data_too_low));
	suite_add_tcase(s, tc);

	tc = tcase_create("receiver MID updated");
	tcase_add_loop_test(tc, test_recv_update, 0, countof(data_recv_update));
	suite_add_tcase(s, tc);

	tc = tcase_create("active task");
	tcase_add_loop_test(tc, test_active, 0, countof(data_active));
	suite_add_tcase(s, tc);

	return s;
}
