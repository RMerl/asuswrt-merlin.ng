/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define STATEFILE_PRIVATE

#include "core/or/or.h"
#include "lib/encoding/confline.h"
#include "app/config/statefile.h"

#include "test/test.h"

static void
test_statefile_remove_obsolete(void *arg)
{
  (void)arg;
  config_line_t *inp = NULL;
  /* try empty config */
  or_state_remove_obsolete_lines(&inp);
  tt_assert(!inp);

  /* try removing every line */
  config_line_append(&inp, "EntryGuard", "doesn't matter");
  config_line_append(&inp, "HidServRevCounter", "ignore");
  config_line_append(&inp, "hidservrevcounter", "foobar"); // note case
  or_state_remove_obsolete_lines(&inp);
  tt_assert(!inp);

  /* Now try removing a subset of lines. */
  config_line_append(&inp, "EntryGuard", "doesn't matter");
  config_line_append(&inp, "Guard", "in use");
  config_line_append(&inp, "HidServRevCounter", "ignore");
  config_line_append(&inp, "TorVersion", "this test doesn't care");
  or_state_remove_obsolete_lines(&inp);
  tt_assert(inp);
  tt_str_op(inp->key, OP_EQ, "Guard");
  tt_str_op(inp->value, OP_EQ, "in use");
  tt_assert(inp->next);
  tt_str_op(inp->next->key, OP_EQ, "TorVersion");
  tt_str_op(inp->next->value, OP_EQ, "this test doesn't care");
  tt_assert(! inp->next->next);

 done:
  config_free_lines(inp);
}

#define T(name) \
  { #name, test_statefile_##name, 0, NULL, NULL }

struct testcase_t statefile_tests[] = {
  T(remove_obsolete),
  END_OF_TESTCASES
};
