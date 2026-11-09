/*
 * Copyright (C) 2008-2025 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "traffic_selector_list.h"

#include <utils/debug.h>

typedef struct private_traffic_selector_list_t private_traffic_selector_list_t;

/**
 * Private data.
 */
struct private_traffic_selector_list_t {

	/**
	 * Public interface.
	 */
	traffic_selector_list_t public;

	/**
	 * List of managed traffic selectors.
	 */
	linked_list_t *ts;
};

METHOD(traffic_selector_list_t, add, void,
	private_traffic_selector_list_t *this, traffic_selector_t *ts)
{
	this->ts->insert_last(this->ts, ts);
}

METHOD(traffic_selector_list_t, create_enumerator, enumerator_t*,
	private_traffic_selector_list_t *this)
{
	return this->ts->create_enumerator(this->ts);
}

/**
 * Create a copy of the traffic selectors in the given list, while resolving
 * "dynamic" traffic selectors using the given hosts, if any. When not narrowing
 * as initiator, we also replace TS in transport mode.
 */
static linked_list_t *resolve_dynamic_ts(private_traffic_selector_list_t *this,
										 linked_list_t *hosts, bool narrowing,
										 bool force_dynamic)
{
	enumerator_t *e1, *e2;
	traffic_selector_t *ts1, *ts2;
	linked_list_t *result;
	host_t *host;

	if (!hosts || !hosts->get_count(hosts))
	{
		return this->ts->clone_offset(this->ts,
									  offsetof(traffic_selector_t, clone));
	}

	result = linked_list_create();
	e1 = this->ts->create_enumerator(this->ts);
	while (e1->enumerate(e1, &ts1))
	{
		/* set hosts if TS is dynamic or if forced as initiator in
		 * transport mode */
		bool dynamic = ts1->is_dynamic(ts1);
		if (!dynamic && !force_dynamic)
		{
			result->insert_last(result, ts1->clone(ts1));
			continue;
		}
		e2 = hosts->create_enumerator(hosts);
		while (e2->enumerate(e2, &host))
		{
			if (!dynamic && !host->is_anyaddr(host) &&
				!ts1->includes(ts1, host))
			{	/* for transport mode, we skip TS that don't match
				 * specific IPs */
				continue;
			}
			ts2 = ts1->clone(ts1);
			if (dynamic || !host->is_anyaddr(host))
			{	/* don't make regular TS larger than they were */
				ts2->set_address(ts2, host);
			}
			result->insert_last(result, ts2);
		}
		e2->destroy(e2);
	}
	e1->destroy(e1);
	return result;
}

/**
 * Remove duplicate traffic selectors in the given list.
 */
static void remove_duplicate_ts(linked_list_t *list)
{
	enumerator_t *e1, *e2;
	traffic_selector_t *ts1, *ts2;

	e1 = list->create_enumerator(list);
	e2 = list->create_enumerator(list);
	while (e1->enumerate(e1, &ts1))
	{
		while (e2->enumerate(e2, &ts2))
		{
			if (ts1 != ts2)
			{
				if (ts2->is_contained_in(ts2, ts1))
				{
					list->remove_at(list, e2);
					ts2->destroy(ts2);
					list->reset_enumerator(list, e1);
					break;
				}
				if (ts1->is_contained_in(ts1, ts2))
				{
					list->remove_at(list, e1);
					ts1->destroy(ts1);
					break;
				}
			}
		}
		list->reset_enumerator(list, e2);
	}
	e1->destroy(e1);
	e2->destroy(e2);
}

METHOD(traffic_selector_list_t, get, linked_list_t*,
	private_traffic_selector_list_t *this, linked_list_t *hosts,
	bool force_dynamic)
{
	linked_list_t *result;

	result = resolve_dynamic_ts(this, hosts, FALSE, force_dynamic);
	remove_duplicate_ts(result);
	return result;
}

METHOD(traffic_selector_list_t, select_, linked_list_t*,
	private_traffic_selector_list_t *this, linked_list_t *supplied,
	linked_list_t *hosts, bool force_dynamic, bool *narrowed)
{
	enumerator_t *e1, *e2;
	traffic_selector_t *ts1, *ts2, *selected;
	linked_list_t *resolved, *result;

	result = linked_list_create();
	resolved = resolve_dynamic_ts(this, hosts, supplied != NULL, force_dynamic);

	if (!supplied)
	{
		while (resolved->remove_first(resolved, (void**)&ts1) == SUCCESS)
		{
			DBG2(DBG_CFG, " %R", ts1);
			result->insert_last(result, ts1);
		}
	}
	else
	{
		e1 = resolved->create_enumerator(resolved);
		e2 = supplied->create_enumerator(supplied);
		/* enumerate all configured/resolved selectors */
		while (e1->enumerate(e1, &ts1))
		{
			/* enumerate all supplied traffic selectors */
			while (e2->enumerate(e2, &ts2))
			{
				selected = ts1->get_subset(ts1, ts2);
				if (selected)
				{
					DBG2(DBG_CFG, " config: %R, received: %R => match: %R",
						 ts1, ts2, selected);
					result->insert_last(result, selected);
				}
				else
				{
					DBG2(DBG_CFG, " config: %R, received: %R => no match",
						 ts1, ts2);
				}
			}
			supplied->reset_enumerator(supplied, e2);
		}
		e1->destroy(e1);
		e2->destroy(e2);

		if (narrowed)
		{
			*narrowed = FALSE;

			e1 = resolved->create_enumerator(resolved);
			e2 = result->create_enumerator(result);
			while (e1->enumerate(e1, &ts1))
			{
				if (!e2->enumerate(e2, &ts2) || !ts1->equals(ts1, ts2))
				{
					*narrowed = TRUE;
					break;
				}
			}
			e1->destroy(e1);
			e2->destroy(e2);
		}
	}
	resolved->destroy_offset(resolved, offsetof(traffic_selector_t, destroy));
	remove_duplicate_ts(result);
	return result;
}

METHOD(traffic_selector_list_t, equals, bool,
	private_traffic_selector_list_t *this, traffic_selector_list_t *other_pub)
{
	private_traffic_selector_list_t *other = (private_traffic_selector_list_t*)other_pub;
	return this->ts->equals_offset(this->ts, other->ts,
								   offsetof(traffic_selector_t, equals));
}

METHOD(traffic_selector_list_t, destroy, void,
	private_traffic_selector_list_t *this)
{
	this->ts->destroy_offset(this->ts, offsetof(traffic_selector_t, destroy));
	free(this);
}

METHOD(traffic_selector_list_t, clone_, traffic_selector_list_t*,
	private_traffic_selector_list_t *this)
{
	return traffic_selector_list_create_from_list(
		this->ts->clone_offset(this->ts, offsetof(traffic_selector_t, clone)));
}

/*
 * Described in header
 */
traffic_selector_list_t *traffic_selector_list_create_from_list(linked_list_t *list)
{
	private_traffic_selector_list_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.create_enumerator = _create_enumerator,
			.get = _get,
			.select = _select_,
			.equals = _equals,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.ts = list,
	);
	return &this->public;
}

/*
 * Described in header
 */
traffic_selector_list_t *traffic_selector_list_create()
{
	return traffic_selector_list_create_from_list(linked_list_create());
}

/*
 * Described in header
 */
traffic_selector_list_t *traffic_selector_list_create_from_enumerator(
													enumerator_t *enumerator)
{
	traffic_selector_list_t *this = traffic_selector_list_create();
	traffic_selector_t *ts;

	while (enumerator->enumerate(enumerator, &ts))
	{
		add((private_traffic_selector_list_t*)this, ts->clone(ts));
	}
	enumerator->destroy(enumerator);

	return this;
}
