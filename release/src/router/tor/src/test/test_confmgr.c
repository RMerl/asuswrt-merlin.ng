/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/*
 * Tests for confmgt.c's features that support multiple configuration
 * formats and configuration objects.
 */

#define CONFMGT_PRIVATE
#include "orconfig.h"

#include "core/or/or.h"
#include "lib/encoding/confline.h"
#include "lib/confmgt/confmgt.h"
#include "test/test.h"
#include "test/log_test_helpers.h"

/*
 * Set up a few objects: a pasture_cfg is toplevel; it has a llama_cfg and an
 * alpaca_cfg.
 */

typedef struct {
  uint32_t magic;
  char *address;
  int opentopublic;
  config_suite_t *subobjs;
} pasture_cfg_t;

typedef struct {
  char *llamaname;
  int cuteness;
  uint32_t magic;
  int eats_meat; /* deprecated; llamas are never carnivorous. */

  char *description; // derived from other fields.
} llama_cfg_t;

typedef struct {
  uint32_t magic;
  int fuzziness;
  char *alpacaname;
  int n_wings; /* deprecated; alpacas don't have wings. */

  int square_fuzziness; /* Derived from fuzziness. */
} alpaca_cfg_t;

/*
 * Make the above into configuration objects.
 */

static pasture_cfg_t pasture_cfg_t_dummy;
static llama_cfg_t llama_cfg_t_dummy;
static alpaca_cfg_t alpaca_cfg_t_dummy;

#define PV(name, type, dflt) \
  CONFIG_VAR_ETYPE(pasture_cfg_t, #name, type, name, 0, dflt)
#define LV(name, type, dflt) \
  CONFIG_VAR_ETYPE(llama_cfg_t, #name, type, name, 0, dflt)
#define AV(name, type, dflt) \
  CONFIG_VAR_ETYPE(alpaca_cfg_t, #name, type, name, 0, dflt)
static const config_var_t pasture_vars[] = {
  PV(address, STRING, NULL),
  PV(opentopublic, BOOL, "1"),
  END_OF_CONFIG_VARS
};
static const config_var_t llama_vars[] =
{
  LV(llamaname, STRING, NULL),
  LV(eats_meat, BOOL, NULL),
  LV(cuteness, POSINT, "100"),
  END_OF_CONFIG_VARS
};
static const config_var_t alpaca_vars[] =
{
  AV(alpacaname, STRING, NULL),
  AV(fuzziness, POSINT, "50"),
  AV(n_wings, POSINT, "0"),
  END_OF_CONFIG_VARS
};

static config_deprecation_t llama_deprecations[] = {
  { "eats_meat", "Llamas are herbivores." },
  {NULL,NULL}
};

static config_deprecation_t alpaca_deprecations[] = {
  { "n_wings", "Alpacas are quadrupeds." },
  {NULL,NULL}
};

static int clear_llama_cfg_called = 0;
static void
clear_llama_cfg(const config_mgr_t *mgr, void *llamacfg)
{
  (void)mgr;
  llama_cfg_t *lc = llamacfg;
  tor_free(lc->description);
  ++clear_llama_cfg_called;
}

static config_abbrev_t llama_abbrevs[] = {
  { "gracia", "cuteness", 0, 0 },
  { "gentillesse", "cuteness", 0, 0 },
  { NULL, NULL, 0, 0 },
};

static int
legacy_validate_pasture(const void *old_, void *obj, char **msg_out)
{
  const pasture_cfg_t *old = old_;
  pasture_cfg_t *p = obj;

  // llamas can't find their way home if the letters are lowercase.
  if (p->address)
    tor_strupper(p->address);

  if (old && old->address &&
      (!p->address || strcmp(old->address, p->address))) {
    *msg_out = tor_strdup("You can't move a pasture.");
    return -1;
  }

  return 0;
}

static int
validate_llama(const void *obj, char **msg_out)
{
  const llama_cfg_t *llama = obj;
  tor_assert(llama->magic == 0x11aa11);

  if (! llama->llamaname || strlen(llama->llamaname) == 0) {
    *msg_out = tor_strdup("A llama has no name!?");
    return -1;
  }

  if (strspn(llama->llamaname, "0123456789") == strlen(llama->llamaname)) {
    *msg_out = tor_strdup("It is not a number; it is a free llama!");
    return -1;
  }

  return 0;
}

static int
check_transition_alpaca(const void *old_, const void *new_, char **msg_out)
{
  const alpaca_cfg_t *old_alpaca = old_;
  const alpaca_cfg_t *new_alpaca = new_;

  tor_assert(old_alpaca && new_alpaca);
  tor_assert(old_alpaca->magic == 0xa15aca);
  tor_assert(new_alpaca->magic == 0xa15aca);

  if (old_alpaca->fuzziness > new_alpaca->fuzziness) {
    *msg_out = tor_strdup("An alpaca only becomes more fuzzy over time.");
    return -1;
  }

  return 0;
}

static int
post_normalize_llama(void *obj, char **msg_out)
{
  (void)msg_out;
  llama_cfg_t *llama = obj;
  tor_assert(llama->magic == 0x11aa11);
  tor_assert(llama->llamaname); // we have already checked for a NULL name.
  tor_free(llama->description);
  tor_asprintf(&llama->description, "A llama called %s.", llama->llamaname);
  return 0;
}

static int
pre_normalize_alpaca(void *obj, char **msg_out)
{
  (void)msg_out;
  alpaca_cfg_t *alpaca = obj;
  tor_assert(alpaca->magic == 0xa15aca);
  alpaca->square_fuzziness = alpaca->fuzziness * alpaca->fuzziness;
  return 0;
}

static const config_format_t pasture_fmt = {
  sizeof(pasture_cfg_t),
  {
    "pasture_cfg_t",
    8989,
    offsetof(pasture_cfg_t, magic)
  },
  .vars = pasture_vars,
  .has_config_suite = true,
  .config_suite_offset = offsetof(pasture_cfg_t, subobjs),
  .legacy_validate_fn = legacy_validate_pasture,
};

static const config_format_t llama_fmt = {
  sizeof(llama_cfg_t),
  {
    "llama_cfg_t",
    0x11aa11,
    offsetof(llama_cfg_t, magic)
  },
  .vars = llama_vars,
  .deprecations = llama_deprecations,
  .abbrevs = llama_abbrevs,
  .clear_fn = clear_llama_cfg,
  .validate_fn = validate_llama,
  .post_normalize_fn = post_normalize_llama,
};

static const config_format_t alpaca_fmt = {
  sizeof(alpaca_cfg_t),
  {
    "alpaca_cfg_t",
    0xa15aca,
    offsetof(alpaca_cfg_t, magic)
  },
  .vars = alpaca_vars,
  .deprecations = alpaca_deprecations,
  .pre_normalize_fn = pre_normalize_alpaca,
  .check_transition_fn = check_transition_alpaca,
};

#define LLAMA_IDX 0
#define ALPACA_IDX 1

static config_mgr_t *
get_mgr(bool freeze)
{
  config_mgr_t *mgr = config_mgr_new(&pasture_fmt);
  tt_int_op(LLAMA_IDX, OP_EQ, config_mgr_add_format(mgr, &llama_fmt));
  tt_int_op(ALPACA_IDX, OP_EQ, config_mgr_add_format(mgr, &alpaca_fmt));
  if (freeze)
    config_mgr_freeze(mgr);
  return mgr;

 done:
  config_mgr_free(mgr);
  return NULL;
}

static void
test_confmgr_init(void *arg)
{
  (void)arg;
  config_mgr_t *mgr = get_mgr(true);
  smartlist_t *vars = NULL;
  tt_ptr_op(mgr, OP_NE, NULL);

  vars = config_mgr_list_vars(mgr);
  tt_int_op(smartlist_len(vars), OP_EQ, 8); // 8 vars total.

  tt_str_op("cuteness", OP_EQ, config_find_option_name(mgr, "CUTENESS"));
  tt_str_op("cuteness", OP_EQ, config_find_option_name(mgr, "GRACIA"));
  smartlist_free(vars);

  vars = config_mgr_list_deprecated_vars(mgr); // 2 deprecated vars.
  tt_int_op(smartlist_len(vars), OP_EQ, 2);
  tt_assert(smartlist_contains_string(vars, "eats_meat"));
  tt_assert(smartlist_contains_string(vars, "n_wings"));

  tt_str_op("Llamas are herbivores.", OP_EQ,
            config_find_deprecation(mgr, "EATS_MEAT"));
  tt_str_op("Alpacas are quadrupeds.", OP_EQ,
            config_find_deprecation(mgr, "N_WINGS"));

 done:
  smartlist_free(vars);
  config_mgr_free(mgr);
}

static void
test_confmgr_magic(void *args)
{
  (void)args;
  // Every time we build a manager, it is supposed to get a different magic
  // number.  Let's test that.
  config_mgr_t *mgr1 = get_mgr(true);
  config_mgr_t *mgr2 = get_mgr(true);
  config_mgr_t *mgr3 = get_mgr(true);

  pasture_cfg_t *p1 = NULL, *p2 = NULL, *p3 = NULL;

  tt_assert(mgr1);
  tt_assert(mgr2);
  tt_assert(mgr3);

  p1 = config_new(mgr1);
  p2 = config_new(mgr2);
  p3 = config_new(mgr3);

  tt_assert(p1);
  tt_assert(p2);
  tt_assert(p3);

  // By chance, two managers get the same magic with P=2^-32.  Let's
  // make sure that at least two of them are different, so that our
  // odds of a false positive are 1/2^-64.
  tt_assert((p1->magic != p2->magic) || (p2->magic != p3->magic));

 done:
  config_free(mgr1, p1);
  config_free(mgr2, p2);
  config_free(mgr3, p3);

  config_mgr_free(mgr1);
  config_mgr_free(mgr2);
  config_mgr_free(mgr3);
}

static const char *simple_pasture =
  "LLamaname hugo\n"
  "Alpacaname daphne\n"
  "gentillesse 42\n"
  "address 123 Camelid ave\n";

static void
test_confmgr_parse(void *arg)
{
  (void)arg;
  config_mgr_t *mgr = get_mgr(true);
  pasture_cfg_t *p = config_new(mgr);
  config_line_t *lines = NULL;
  char *msg = NULL;

  config_init(mgr, p); // set defaults.

  int r = config_get_lines(simple_pasture, &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  r = config_assign(mgr, p, lines, 0, &msg);
  tt_int_op(r, OP_EQ, 0);

  tt_int_op(p->opentopublic, OP_EQ, 1);
  tt_str_op(p->address, OP_EQ, "123 Camelid ave");

  // We are using this API directly; modules outside confparse will, in the
  // future, not.
  const alpaca_cfg_t *ac = config_mgr_get_obj(mgr, p, ALPACA_IDX);
  const llama_cfg_t *lc = config_mgr_get_obj(mgr, p, LLAMA_IDX);
  tt_str_op(lc->llamaname, OP_EQ, "hugo");
  tt_str_op(ac->alpacaname, OP_EQ, "daphne");
  tt_int_op(lc->cuteness, OP_EQ, 42);
  tt_int_op(ac->fuzziness, OP_EQ, 50);

  // We set the description for the llama here, so that the clear function
  // can clear it.  (Later we can do this in a verification function.)
  clear_llama_cfg_called = 0;
  llama_cfg_t *mut_lc = config_mgr_get_obj_mutable(mgr, p, LLAMA_IDX);
  mut_lc->description = tor_strdup("A llama named Hugo.");
  config_free(mgr, p);
  tt_int_op(clear_llama_cfg_called, OP_EQ, 1);

 done:
  config_free_lines(lines);
  config_free(mgr, p);
  config_mgr_free(mgr);
  tor_free(msg);
}

static void
test_confmgr_dump(void *arg)
{
  (void)arg;
  config_mgr_t *mgr = get_mgr(true);
  pasture_cfg_t *p = config_new(mgr);
  pasture_cfg_t *defaults = config_new(mgr);
  config_line_t *lines = NULL;
  char *msg = NULL;
  char *s = NULL;

  config_init(mgr, p); // set defaults.
  config_init(mgr, defaults); // set defaults.

  int r = config_get_lines(simple_pasture, &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  r = config_assign(mgr, p, lines, 0, &msg);
  tt_int_op(r, OP_EQ, 0);

  s = config_dump(mgr, defaults, p, 1, 0);
  tt_str_op("address 123 Camelid ave\n"
            "alpacaname daphne\n"
            "cuteness 42\n"
            "llamaname hugo\n", OP_EQ, s);

 done:
  config_free_lines(lines);
  config_free(mgr, p);
  config_free(mgr, defaults);
  config_mgr_free(mgr);

  tor_free(msg);
  tor_free(s);
}

static pasture_cfg_t *
parse_and_validate(config_mgr_t *mgr,
                   const char *inp, const pasture_cfg_t *old, char **msg_out)
{
  pasture_cfg_t *p = config_new(mgr);
  pasture_cfg_t *result = NULL;
  config_line_t *lines = NULL;

  config_init(mgr, p); // set defaults.
  int r = config_get_lines(inp, &lines, 0);
  tt_int_op(r, OP_EQ, 0);
  r = config_assign(mgr, p, lines, 0, msg_out);
  tt_int_op(r, OP_EQ, 0);
  tor_free(*msg_out); // sets it to NULL
  r = config_validate(mgr, old, p, msg_out);
  if (r < 0)
    goto done;

  tt_ptr_op(*msg_out, OP_EQ, NULL);
  result = p;
  p = NULL; // prevent free
 done:
  config_free(mgr, p);
  config_free_lines(lines);
  return result;
}

static void
test_confmgr_validate(void *arg)
{
  (void)arg;
  char *msg = NULL;
  config_mgr_t *mgr = get_mgr(true);
  pasture_cfg_t *p_orig, *p=NULL;

  p_orig = parse_and_validate(mgr, "Llamaname Quest\n"
                                   "Address 99 camelid way\n"
                                   "Fuzziness 8\n", NULL, &msg);
  tt_assert(p_orig);

  // Make sure normalization code was run.
  const alpaca_cfg_t *ac0 = config_mgr_get_obj(mgr, p_orig, ALPACA_IDX);
  const llama_cfg_t *lc0 = config_mgr_get_obj(mgr, p_orig, LLAMA_IDX);
  tt_int_op(ac0->fuzziness, OP_EQ, 8);
  tt_int_op(ac0->square_fuzziness, OP_EQ, 64);
  tt_str_op(lc0->description, OP_EQ, "A llama called Quest.");
  tt_str_op(p_orig->address, OP_EQ, "99 CAMELID WAY");

  // try a bad llamaname.
  p = parse_and_validate(mgr, "llamaname 123", p_orig, &msg);
  tt_assert(!p);
  tt_str_op(msg, OP_EQ, "It is not a number; it is a free llama!");
  tor_free(msg);

  // try a llamaname that would crash the post_normalize step, if it ran.
  p = parse_and_validate(mgr, "", p_orig, &msg);
  tt_assert(!p);
  tt_str_op(msg, OP_EQ, "A llama has no name!?");
  tor_free(msg);

  // Verify that a transition to a less fuzzy alpaca fails.
  p = parse_and_validate(mgr, "Llamaname Quest\n"
                              "Address 99 camelid way\n"
                              "Fuzziness 4\n", p_orig, &msg);
  tt_assert(!p);
  tt_str_op(msg, OP_EQ, "An alpaca only becomes more fuzzy over time.");
  tor_free(msg);

  // Try a transition to a more fuzzy alpaca; it should work fine.
  p = parse_and_validate(mgr, "Llamaname Mercutio\n"
                              // the default fuzziness is 50
                              "Address 99 camelid way\n", p_orig, &msg);
  tt_assert(p);
  config_free(mgr, p);

  // Verify that we can't move the pasture.
  p = parse_and_validate(mgr, "Llamaname Montague\n"
                              // the default fuzziness is 50
                              "Address 99 ungulate st\n", p_orig, &msg);
  tt_assert(!p);
  tt_str_op(msg, OP_EQ, "You can't move a pasture.");

 done:
  config_free(mgr, p);
  config_free(mgr, p_orig);
  config_mgr_free(mgr);
  tor_free(msg);
}

#define CONFMGR_TEST(name, flags)                       \
  { #name, test_confmgr_ ## name, flags, NULL, NULL }

struct testcase_t confmgr_tests[] = {
  CONFMGR_TEST(init, 0),
  CONFMGR_TEST(magic, 0),
  CONFMGR_TEST(parse, 0),
  CONFMGR_TEST(dump, 0),
  CONFMGR_TEST(validate, 0),
  END_OF_TESTCASES
};
