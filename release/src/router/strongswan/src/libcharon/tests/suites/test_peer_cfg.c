/*
 * Copyright (C) 2018 Tobias Brunner
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

#include <config/peer_cfg.h>
#include <config/child_cfg.h>

/**
 * Create a simple IKE config
 */
static ike_cfg_t *create_ike_cfg()
{
	return ike_cfg_create(IKEV2, TRUE, FALSE, "127.0.0.1", 500,
						  "127.0.0.1", 500, FRAGMENTATION_NO, 0);
}

/**
 * Create a simple peer config
 */
static peer_cfg_t *create_peer_cfg()
{
	peer_cfg_create_t peer = {};

	return peer_cfg_create("peer", create_ike_cfg(), &peer);
}

static peer_cfg_t *peer_a, *peer_b;

START_SETUP(setup_replace)
{
	peer_a = create_peer_cfg();
	peer_b = create_peer_cfg();
}
END_SETUP

START_TEARDOWN(teardown_replace)
{
	peer_a->destroy(peer_a);
	peer_b->destroy(peer_b);
}
END_TEARDOWN

/**
 * Check if the changes are correctly reported
 * All given objects are destroyed
 */
static void test_replace(enumerator_t *changes, linked_list_t *rem,
						 linked_list_t *add)
{
	child_cfg_t *child;
	bool added;

	while (changes->enumerate(changes, &child, &added))
	{
		if (added)
		{
			ck_assert_msg(add->remove(add, child, NULL) == 1, "child config "
						  "was unexpectedly added");
		}
		else
		{
			ck_assert_msg(rem->remove(rem, child, NULL) == 1, "child config "
						  "was unexpectedly removed");
		}
	}
	changes->destroy(changes);
	ck_assert_msg(!rem->get_count(rem), "expected child config was not removed");
	ck_assert_msg(!add->get_count(add), "expected child config was not added");
	rem->destroy(rem);
	add->destroy(add);
}

/**
 * Check if the given child configs are contained in the peer config
 * The list is destroyed
 */
static void test_child_cfgs(peer_cfg_t *peer, linked_list_t *children)
{
	enumerator_t *enumerator;
	child_cfg_t *child;

	enumerator = peer->create_child_cfg_enumerator(peer);
	while (enumerator->enumerate(enumerator, &child))
	{
		ck_assert_msg(children->remove(children, child, NULL) == 1, "child "
					  "config was unexpectedly contained in peer config");
	}
	enumerator->destroy(enumerator);
	ck_assert_msg(!children->get_count(children), "expected child config was "
				  "not contained in peer config");
	children->destroy(children);
}

START_TEST(replace_child_cfgs_empty)
{
	child_cfg_create_t cfg = {};
	child_cfg_t *child;

	child = child_cfg_create("c", &cfg);
	peer_b->add_child_cfg(peer_b, child->get_ref(child));

	test_replace(peer_a->replace_child_cfgs(peer_a, peer_b),
				 linked_list_create(),
				 linked_list_create_with_items(child, NULL));
	test_child_cfgs(peer_a,
					linked_list_create_with_items(child, NULL));

	child->destroy(child);
}
END_TEST

START_TEST(replace_child_cfgs_same)
{
	child_cfg_create_t cfg = {};
	child_cfg_t *child;

	child = child_cfg_create("c", &cfg);
	peer_a->add_child_cfg(peer_a, child->get_ref(child));
	peer_b->add_child_cfg(peer_b, child->get_ref(child));

	test_replace(peer_a->replace_child_cfgs(peer_a, peer_b),
				 linked_list_create(),
				 linked_list_create());
	test_child_cfgs(peer_a,
					linked_list_create_with_items(child, NULL));

	child->destroy(child);
}
END_TEST

START_TEST(replace_child_cfgs_same_replace)
{
	child_cfg_create_t cfg = {};
	child_cfg_t *c1, *c2;

	c1 = child_cfg_create("c1", &cfg);
	peer_a->add_child_cfg(peer_a, c1->get_ref(c1));
	c2 = child_cfg_create("c2", &cfg);
	peer_b->add_child_cfg(peer_b, c2->get_ref(c2));

	test_replace(peer_a->replace_child_cfgs(peer_a, peer_b),
				 linked_list_create(),
				 linked_list_create());
	test_child_cfgs(peer_a,
					linked_list_create_with_items(c2, NULL));

	c1->destroy(c1);
	c2->destroy(c2);
}
END_TEST

START_TEST(replace_child_cfgs_clear)
{
	child_cfg_create_t cfg = {};
	child_cfg_t *child;

	child = child_cfg_create("c", &cfg);
	peer_a->add_child_cfg(peer_a, child->get_ref(child));

	test_replace(peer_a->replace_child_cfgs(peer_a, peer_b),
				 linked_list_create_with_items(child, NULL),
				 linked_list_create());
	test_child_cfgs(peer_a,
					linked_list_create());

	child->destroy(child);
}
END_TEST

START_TEST(replace_child_cfgs_mixed)
{
	child_cfg_create_t cfg1 = {}, cfg2 = { .mode = MODE_TUNNEL, };
	child_cfg_create_t cfg3 = { .mode = MODE_TRANSPORT};
	child_cfg_t *c1, *c2, *c3, *c4;

	c1 = child_cfg_create("c1", &cfg1);
	peer_a->add_child_cfg(peer_a, c1->get_ref(c1));
	c2 = child_cfg_create("c2", &cfg2);
	peer_a->add_child_cfg(peer_a, c2->get_ref(c2));

	c3 = child_cfg_create("c3", &cfg3);
	peer_b->add_child_cfg(peer_b, c3->get_ref(c3));
	c4 = child_cfg_create("c4", &cfg2);
	peer_b->add_child_cfg(peer_b, c4->get_ref(c4));

	test_replace(peer_a->replace_child_cfgs(peer_a, peer_b),
				 linked_list_create_with_items(c1, NULL),
				 linked_list_create_with_items(c3, NULL));
	test_child_cfgs(peer_a,
					linked_list_create_with_items(c3, c4, NULL));

	c1->destroy(c1);
	c2->destroy(c2);
	c3->destroy(c3);
	c4->destroy(c4);
}
END_TEST

Suite *peer_cfg_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("peer_cfg");

	tc = tcase_create("replace_child_cfgs");
	tcase_add_checked_fixture(tc, setup_replace, teardown_replace);
	tcase_add_test(tc, replace_child_cfgs_empty);
	tcase_add_test(tc, replace_child_cfgs_same);
	tcase_add_test(tc, replace_child_cfgs_same_replace);
	tcase_add_test(tc, replace_child_cfgs_clear);
	tcase_add_test(tc, replace_child_cfgs_mixed);
	suite_add_tcase(s, tc);

	return s;
}
