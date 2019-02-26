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

#include <inttypes.h>

#include <test_suite.h>

#include "exchange_test_asserts.h"
#include "mock_ipsec.h"

/*
 * Described in header
 */
bool exchange_test_asserts_hook(listener_t *listener)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;

	this->count++;
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_ike_updown(listener_t *listener, ike_sa_t *ike_sa,
									  bool up)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;

	this->count++;
	assert_listener_msg(this->up == up, this, "IKE_SA not '%s'",
						this->up ? "up" : "down");
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_child_updown(listener_t *listener, ike_sa_t *ike_sa,
										child_sa_t *child_sa, bool up)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;

	this->count++;
	assert_listener_msg(this->up == up, this, "CHILD_SA not '%s'",
						this->up ? "up" : "down");
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_ike_rekey(listener_t *listener, ike_sa_t *old,
									 ike_sa_t *new)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;
	ike_sa_id_t *id;
	uint64_t spi;

	this->count++;
	id = old->get_id(old);
	spi = id->get_initiator_spi(id);
	assert_listener_msg(this->spi_old == spi, this, "unexpected old IKE_SA "
						"%.16"PRIx64"_i instead of %.16"PRIx64"_i",
						be64toh(spi), be64toh(this->spi_old));
	id = new->get_id(new);
	spi = id->get_initiator_spi(id);
	assert_listener_msg(this->spi_new == spi, this, "unexpected new IKE_SA "
						"%.16"PRIx64"_i instead of %.16"PRIx64"_i",
						be64toh(spi), be64toh(this->spi_new));
	return TRUE;
}

/*
 * Described in header
 */
bool exchange_test_asserts_child_rekey(listener_t *listener, ike_sa_t *ike_sa,
									   child_sa_t *old, child_sa_t *new)
{
	listener_hook_assert_t *this = (listener_hook_assert_t*)listener;
	uint32_t spi, expected;

	this->count++;
	spi = old->get_spi(old, TRUE);
	expected = this->spi_old;
	assert_listener_msg(expected == spi, this, "unexpected old CHILD_SA %.8x "
						"instead of %.8x", spi, expected);
	spi = new->get_spi(new, TRUE);
	expected = this->spi_new;
	assert_listener_msg(expected == spi, this, "unexpected new CHILD_SA %.8x "
						"instead of %.8x", spi, expected);
	return TRUE;
}

/**
 * Assert a given message rule
 */
static void assert_message_rule(listener_message_assert_t *this, message_t *msg,
								listener_message_rule_t *rule)
{
	if (rule->expected)
	{
		if (rule->payload)
		{
			assert_listener_msg(msg->get_payload(msg, rule->payload),
								this, "expected payload (%N) not found",
								payload_type_names, rule->payload);

		}
		if (rule->notify)
		{
			assert_listener_msg(msg->get_notify(msg, rule->notify),
								this, "expected notify payload (%N) not found",
								notify_type_names, rule->notify);
		}
	}
	else
	{
		if (rule->payload)
		{
			assert_listener_msg(!msg->get_payload(msg, rule->payload),
								this, "unexpected payload (%N) found",
								payload_type_names, rule->payload);

		}
		if (rule->notify)
		{
			assert_listener_msg(!msg->get_notify(msg, rule->notify),
								this, "unexpected notify payload (%N) found",
								notify_type_names, rule->notify);
		}
	}
}

/*
 * Described in header
 */
bool exchange_test_asserts_message(listener_t *listener, ike_sa_t *ike_sa,
								message_t *message, bool incoming, bool plain)
{
	listener_message_assert_t *this = (listener_message_assert_t*)listener;

	if (plain && this->incoming == incoming)
	{
		if (this->count >= 0)
		{
			enumerator_t *enumerator;
			int count = 0;
			enumerator = message->create_payload_enumerator(message);
			while (enumerator->enumerate(enumerator, NULL))
			{
				count++;
			}
			enumerator->destroy(enumerator);
			assert_listener_msg(this->count == count, this, "unexpected payload "
								"count in message (%d != %d)", this->count,
								count);
		}
		if (this->num_rules)
		{
			int i;

			for (i = 0; i < this->num_rules; i++)
			{
				assert_message_rule(this, message, &this->rules[i]);
			}
		}
		return FALSE;
	}
	return TRUE;
}

/**
 * Compare two SPIs
 */
static int spis_cmp(const void *a, const void *b)
{
	return *(const uint32_t*)a - *(const uint32_t*)b;
}

/**
 * Compare two SPIs to sort them
 */
static int spis_sort(const void *a, const void *b, void *data)
{
	return spis_cmp(a, b);
}


/*
 * Described in header
 */
void exchange_test_asserts_ipsec_sas(ipsec_sas_assert_t *sas)
{
	enumerator_t *enumerator;
	array_t *spis;
	ike_sa_t *ike_sa;
	uint32_t spi;
	int i;

	spis = array_create(sizeof(uint32_t), 0);
	for (i = 0; i < sas->count; i++)
	{
		array_insert(spis, ARRAY_TAIL, &sas->spis[i]);
	}
	array_sort(spis, spis_sort, NULL);

	enumerator = mock_ipsec_create_sa_enumerator();
	while (enumerator->enumerate(enumerator, &ike_sa, &spi))
	{
		if (ike_sa == sas->ike_sa)
		{
			i = array_bsearch(spis, &spi, spis_cmp, NULL);
			assert_listener_msg(i != -1, sas, "unexpected IPsec SA %.8x", spi);
			array_remove(spis, i, NULL);
		}
	}
	enumerator->destroy(enumerator);
	for (i = 0; i < array_count(spis); i++)
	{
		array_get(spis, i, &spi);
		assert_listener_msg(!spi, sas, "expected IPsec SA %.8x not found", spi);
	}
	array_destroy(spis);
}
