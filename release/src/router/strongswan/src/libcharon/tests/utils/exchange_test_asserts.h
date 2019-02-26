/*
 * Copyright (C) 2016-2017 Tobias Brunner
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

/**
 * Special assertions using listener_t etc.
 *
 * @defgroup exchange_test_asserts exchange_test_asserts
 * @{ @ingroup test_utils_c
 */

#ifndef EXCHANGE_TEST_ASSERTS_H_
#define EXCHANGE_TEST_ASSERTS_H_

#include <bus/listeners/listener.h>

typedef struct listener_hook_assert_t listener_hook_assert_t;
typedef struct listener_message_assert_t listener_message_assert_t;
typedef struct listener_message_rule_t listener_message_rule_t;
typedef struct ipsec_sas_assert_t ipsec_sas_assert_t;

struct listener_hook_assert_t {

	/**
	 * Implemented interface
	 */
	listener_t listener;

	/**
	 * Original source file
	 */
	const char *file;

	/**
	 * Source line
	 */
	int line;

	/**
	 * Name of the hook
	 */
	const char *name;

	/**
	 * Expected number of calls (-1 to ignore)
	 */
	int expected;

	/**
	 * Number of times the hook was called
	 */
	int count;

	/**
	 * Expected updown result
	 */
	bool up;

	/**
	 * Initiator/Inbound SPIs to expect in rekey event
	 */
	uint64_t spi_old, spi_new;
};

/**
 * Basic callback for methods on listener_t, counting the number of calls.
 */
bool exchange_test_asserts_hook(listener_t *this);

/**
 * Implementation of listener_t::ike_updown.
 */
bool exchange_test_asserts_ike_updown(listener_t *this, ike_sa_t *ike_sa,
									  bool up);

/**
 * Implementation of listener_t::child_updown.
 */
bool exchange_test_asserts_child_updown(listener_t *this, ike_sa_t *ike_sa,
										child_sa_t *child_sa, bool up);

/**
 * Implementation of listener_t::ike_rekey.
 */
bool exchange_test_asserts_ike_rekey(listener_t *this, ike_sa_t *old,
									 ike_sa_t *new);

/**
 * Implementation of listener_t::child_rekey.
 */
bool exchange_test_asserts_child_rekey(listener_t *this, ike_sa_t *ike_sa,
									   child_sa_t *old, child_sa_t *new);

/**
 * Check if a statement evaluates to TRUE, use original source file and line
 * in the error message if not.
 *
 * @param x			statement to evaluate
 * @param l			listener providing original source file and line
 * @param fmt		printf format string
 * @param ...		arguments for fmt
 */
#define assert_listener_msg(x, l, fmt, ...) ({ \
	test_fail_if_worker_failed(); \
	if (!(x)) \
	{ \
		test_fail_msg((l)->file, (l)->line, "%s: " fmt, #x, ##__VA_ARGS__); \
	} \
})

/**
 * Initialize an assertion that enforces that the given hook was called.
 * Must be matched by a call to assert_hook().
 *
 * @param name		name of the hook
 */
#define assert_hook_called(name) \
	_assert_hook_init(name, exchange_test_asserts_hook, .expected = 1)

/**
 * Initialize an assertion that enforces that the given hook was not called.
 * Must be matched by a call to assert_hook().
 *
 * @param name		name of the hook
 */
#define assert_hook_not_called(name) \
	_assert_hook_init(name, exchange_test_asserts_hook, .expected = 0)

/**
 * Initialize an assertion that enforces that the given updown hook was called
 * with the expected result.
 * Must be matched by a call to assert_hook().
 *
 * @param name		name of the hook
 * @param e			whether to expect up in the hook to be TRUE or not
 */
#define assert_hook_updown(name, e) \
	_assert_hook_init(name, \
		streq(#name, "ike_updown") ? (void*)exchange_test_asserts_ike_updown \
								   : (void*)exchange_test_asserts_child_updown, \
		.expected = 1, \
		.up = e, \
	)

/**
 * Initialize an assertion that enforces that the given rekey hook was called
 * with the SAs with the matching initiator/inbound SPIs.
 * Must be matched by a call to assert_hook().
 *
 * @param name		name of the hook
 * @param old		SPI of the old SA
 * @param new		SPI of the new SA
 */
#define assert_hook_rekey(name, old, new) \
	_assert_hook_init(name, \
		streq(#name, "ike_rekey") ? (void*)exchange_test_asserts_ike_rekey \
								   : (void*)exchange_test_asserts_child_rekey, \
		.expected = 1, \
		.spi_old = old, \
		.spi_new = new, \
	)

/**
 * Initialize assertions against invocations of listener_t hooks.  Each call
 * must be matched by a call to assert_hook().
 */
#define _assert_hook_init(n, callback, ...) \
do { \
	listener_hook_assert_t _hook_listener = { \
		.listener = { .n = (void*)callback, }, \
		.file = __FILE__, \
		.line = __LINE__, \
		.name = #n, \
		##__VA_ARGS__ \
	}; \
	exchange_test_helper->add_listener(exchange_test_helper, &_hook_listener.listener)

/**
 * Enforce the most recently initialized hook assertion.
 */
#define assert_hook() \
	charon->bus->remove_listener(charon->bus, &_hook_listener.listener); \
	if (_hook_listener.expected > 0) { \
		if (_hook_listener.count > 0) { \
			assert_listener_msg(_hook_listener.expected == _hook_listener.count, \
								&_hook_listener, "hook '%s' was called %d times " \
								"instead of %d", _hook_listener.name, \
								_hook_listener.count, _hook_listener.expected); \
		} else { \
			assert_listener_msg(_hook_listener.count, &_hook_listener, \
				"hook '%s' was not called (expected %d)", _hook_listener.name, \
				_hook_listener.expected); \
		} \
	} else if (_hook_listener.expected == 0) { \
		assert_listener_msg(_hook_listener.count == 0, &_hook_listener, \
				"hook '%s' was called unexpectedly", _hook_listener.name); \
	} \
} while(FALSE)

/**
 * Rules regarding payloads/notifies to expect/not expect in a message
 */
struct listener_message_rule_t {

	/**
	 * Whether the payload/notify is expected in the message, FALSE to fail if
	 * it is found
	 */
	bool expected;

	/**
	 * Payload type to expect/not expect
	 */
	payload_type_t payload;

	/**
	 * Notify type to expect/not expect (paylod type does not have to be
	 * specified)
	 */
	notify_type_t notify;
};

/**
 * Data used to check plaintext messages via listener_t
 */
struct listener_message_assert_t {

	/**
	 * Implemented interface
	 */
	listener_t listener;

	/**
	 * Original source file
	 */
	const char *file;

	/**
	 * Source line
	 */
	int line;

	/**
	 * Whether to check the next inbound or outbound message
	 */
	bool incoming;

	/**
	 * Payload count to expect (-1 to ignore the count)
	 */
	int count;

	/**
	 * Payloads to expect or not expect in a message
	 */
	listener_message_rule_t *rules;

	/**
	 * Number of rules
	 */
	int num_rules;
};

/**
 * Implementation of listener_t::message collecting data and asserting
 * certain things.
 */
bool exchange_test_asserts_message(listener_t *this, ike_sa_t *ike_sa,
							message_t *message, bool incoming, bool plain);

/**
 * Assert that the next in- or outbound plaintext message is empty.
 *
 * @param dir			IN or OUT to check the next in- or outbound message
 */
#define assert_message_empty(dir) \
				_assert_payload(#dir, 0)

/**
 * Assert that the next in- or outbound plaintext message contains exactly
 * one payload of the given type.
 *
 * @param dir			IN or OUT to check the next in- or outbound message
 * @param expected		expected payload type
 */
#define assert_single_payload(dir, expected) \
				_assert_payload(#dir, 1, { TRUE, expected, 0 })

/**
 * Assert that the next in- or outbound plaintext message contains exactly
 * one notify of the given type.
 *
 * @param dir			IN or OUT to check the next in- or outbound message
 * @param expected		expected notify type
 */
#define assert_single_notify(dir, expected) \
				_assert_payload(#dir, 1, { TRUE, 0, expected })

/**
 * Assert that the next in- or outbound plaintext message contains a notify
 * of the given type.
 *
 * @param dir			IN or OUT to check the next in- or outbound message
 * @param expected		expected notify type
 */
#define assert_notify(dir, expected) \
				_assert_payload(#dir, -1, { TRUE, 0, expected })

/**
 * Assert that the next in- or outbound plaintext message does not contain a
 * notify of the given type.
 *
 * @param dir			IN or OUT to check the next in- or outbound message
 * @param unexpected	not expected notify type
 */
#define assert_no_notify(dir, unexpected) \
				_assert_payload(#dir, -1, { FALSE, 0, unexpected })

#define _assert_payload(dir, c, ...) ({ \
	listener_message_rule_t _rules[] = { __VA_ARGS__ }; \
	listener_message_assert_t _listener = { \
		.listener = { .message = exchange_test_asserts_message, }, \
		.file = __FILE__, \
		.line = __LINE__, \
		.incoming = streq(dir, "IN") ? TRUE : FALSE, \
		.count = c, \
		.rules = _rules, \
		.num_rules = countof(_rules), \
	}; \
	exchange_test_helper->add_listener(exchange_test_helper, &_listener.listener); \
})

/**
 * Data used to check IPsec SAs
 */
struct ipsec_sas_assert_t {

	/**
	 * Original source file
	 */
	const char *file;

	/**
	 * Source line
	 */
	int line;

	/**
	 * IKE_SA that installed the IPsec SAs
	 */
	ike_sa_t *ike_sa;

	/**
	 * SPIs to check
	 */
	uint32_t *spis;

	/**
	 * Number of SPIs for IPsec SAs to check
	 */
	int count;
};

/**
 * Assert that all given IPsec SAs (and only these) are installed for the given
 * IKE_SA.
 */
void exchange_test_asserts_ipsec_sas(ipsec_sas_assert_t *sas);

/**
 * Assert that the IPsec SAs with the given SPIs (and none other) are currently
 * installed by the given IKE_SA.
 *
 * @param sa		IKE_SA
 * @param ...		list of SPIs
 */
#define assert_ipsec_sas_installed(sa, ...) ({ \
	uint32_t _spis[] = { __VA_ARGS__ }; \
	ipsec_sas_assert_t _sas_assert = { \
		.file = __FILE__, \
		.line = __LINE__, \
		.ike_sa = sa, \
		.spis = _spis, \
		.count = countof(_spis), \
	}; \
	exchange_test_asserts_ipsec_sas(&_sas_assert); \
})

#endif /** EXCHANGE_TEST_ASSERTS_H_ @}*/
