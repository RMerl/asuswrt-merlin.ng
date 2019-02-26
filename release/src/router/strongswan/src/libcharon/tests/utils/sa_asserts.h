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
 * Special assertions against IKE_SAs and CHILD_SAs (e.g. regarding their
 * state).
 *
 * @defgroup sa_asserts sa_asserts
 * @{ @ingroup test_utils_c
 */

#ifndef SA_ASSERTS_H_
#define SA_ASSERTS_H_

#include <inttypes.h>

/**
 * Check that there exists a specific number of IKE_SAs in the manager.
 */
#define assert_ike_sa_count(count) \
({ \
	typeof(count) _count = count; \
	u_int _actual = charon->ike_sa_manager->get_count(charon->ike_sa_manager); \
	test_assert_msg(_count == _actual, "unexpected number of IKE_SAs in " \
					"manager (%d != %d)", _count, _actual); \
})

/**
 * Check that the IKE_SA with the given SPIs and initiator flag is in the
 * manager and return it.  Does not actually keep the SA checked out as
 * that would block cleaning up if asserts against it fail (since we control
 * access to SAs it's also not really necessary).
 */
#define assert_ike_sa_checkout(spi_i, spi_r, initiator) \
({ \
	typeof(spi_i) _spi_i = spi_i; \
	typeof(spi_r) _spi_r = spi_r; \
	typeof(initiator) _init = initiator; \
	ike_sa_id_t *_id = ike_sa_id_create(IKEV2, _spi_i, _spi_r, _init); \
	ike_sa_t *_ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, _id); \
	test_assert_msg(_ike_sa, "IKE_SA with SPIs %.16"PRIx64"_i %.16"PRIx64"_r " \
					"(%d) does not exist", be64toh(_spi_i), be64toh(_spi_r), _init); \
	_id->destroy(_id); \
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, _ike_sa); \
	_ike_sa; \
})

/**
 * Check if the given IKE_SA is in the expected state.
 */
#define assert_ike_sa_state(ike_sa, state) \
({ \
	typeof(ike_sa) _sa = ike_sa; \
	typeof(state) _state = state; \
	test_assert_msg(_state == _sa->get_state(_sa), "%N != %N", \
					ike_sa_state_names, _state, \
					ike_sa_state_names, _sa->get_state(_sa)); \
})

/**
 * Check that there exists a specific number of CHILD_SAs.
 */
#define assert_child_sa_count(ike_sa, count) \
({ \
	typeof(ike_sa) _sa = ike_sa; \
	typeof(count) _count = count; \
	test_assert_msg(_count == _sa->get_child_count(_sa), "unexpected number " \
					"of CHILD_SAs in IKE_SA %s (%d != %d)", #ike_sa, _count, \
					_sa->get_child_count(_sa)); \
})

/**
 * Check if the CHILD_SA with the given SPI is in the expected state, optionally
 * check the state of the outbound SA.
 */
#define assert_child_sa_state(...) VA_ARGS_DISPATCH(assert_child_sa_state, __VA_ARGS__)(__VA_ARGS__)

/**
 * Check if the CHILD_SA with the given SPI is in the expected state.
 */
#define assert_child_sa_state3(ike_sa, spi, state) \
({ \
	typeof(ike_sa) _sa = ike_sa; \
	typeof(spi) _spi = spi; \
	typeof(state) _state = state; \
	child_sa_t *_child = _sa->get_child_sa(_sa, PROTO_ESP, _spi, TRUE) ?: \
						 _sa->get_child_sa(_sa, PROTO_ESP, _spi, FALSE); \
	test_assert_msg(_child, "CHILD_SA with SPI %.8x does not exist", \
					ntohl(_spi)); \
	test_assert_msg(_state == _child->get_state(_child), "%N != %N", \
					child_sa_state_names, _state, \
					child_sa_state_names, _child->get_state(_child)); \
})

/**
 * Check if the outbound SA of a CHILD_SA with the given SPI is in the
 * expected state.
 */
#define assert_child_sa_state4(ike_sa, spi, state, outbound) \
({ \
	typeof(ike_sa) _sa = ike_sa; \
	typeof(spi) _spi = spi; \
	typeof(state) _state = state; \
	typeof(outbound) _outbound = outbound; \
	child_sa_t *_child = _sa->get_child_sa(_sa, PROTO_ESP, _spi, TRUE) ?: \
						 _sa->get_child_sa(_sa, PROTO_ESP, _spi, FALSE); \
	test_assert_msg(_child, "CHILD_SA with SPI %.8x does not exist", \
					ntohl(_spi)); \
	test_assert_msg(_state == _child->get_state(_child), "%N != %N", \
					child_sa_state_names, _state, \
					child_sa_state_names, _child->get_state(_child)); \
	typeof(outbound) _cur_out = _child->get_outbound_state(_child); \
	test_assert_msg(_outbound == _cur_out || _outbound & _cur_out, "%N != %N", \
					child_sa_outbound_state_names, _outbound, \
					child_sa_outbound_state_names, _child->get_outbound_state(_child)); \
})

/**
 * Assert that the CHILD_SA with the given inbound SPI does not exist.
 */
#define assert_child_sa_not_exists(ike_sa, spi) \
({ \
	typeof(ike_sa) _sa = ike_sa; \
	typeof(spi) _spi = spi; \
	child_sa_t *_child = _sa->get_child_sa(_sa, PROTO_ESP, _spi, TRUE) ?: \
						 _sa->get_child_sa(_sa, PROTO_ESP, _spi, FALSE); \
	test_assert_msg(!_child, "CHILD_SA with SPI %.8x exists", ntohl(_spi)); \
})

/**
 * Assert that there is a specific number of tasks in a given queue
 *
 * @param ike_sa		IKE_SA to check
 * @param count			number of expected tasks
 * @param queue			queue to check (task_queue_t)
 */
#define assert_num_tasks(ike_sa, count, queue) \
({ \
	typeof(ike_sa) _sa = ike_sa; \
	typeof(count) _count = count; \
	int _c = 0; task_t *_task; \
	enumerator_t *_enumerator = _sa->create_task_enumerator(_sa, queue); \
	while (_enumerator->enumerate(_enumerator, &_task)) { _c++; } \
	_enumerator->destroy(_enumerator); \
	test_assert_msg(_count == _c, "unexpected number of tasks in " #queue " " \
					"of IKE_SA %s (%d != %d)", #ike_sa, _count, _c); \
})

/**
 * Assert that all task queues of the given IKE_SA are empty
 *
 * @param ike_sa		IKE_SA to check
 */
#define assert_sa_idle(ike_sa) \
({ \
	typeof(ike_sa) _ike_sa = ike_sa; \
	assert_num_tasks(_ike_sa, 0, TASK_QUEUE_QUEUED); \
	assert_num_tasks(_ike_sa, 0, TASK_QUEUE_ACTIVE); \
	assert_num_tasks(_ike_sa, 0, TASK_QUEUE_PASSIVE); \
})

#endif /** SA_ASSERTS_H_ @}*/
