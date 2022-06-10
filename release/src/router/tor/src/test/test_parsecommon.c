/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "test/test.h"
#include "lib/memarea/memarea.h"
#include "lib/encoding/binascii.h"
#include "feature/dirparse/parsecommon.h"
#include "test/log_test_helpers.h"

static void
test_parsecommon_tokenize_string_null(void *arg)
{

  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  const char *str_with_null = "a\0bccccccccc";

  int retval =
  tokenize_string(area, str_with_null,
                  str_with_null + 3,
                  tokens, NULL, 0);

  tt_int_op(retval, OP_EQ, -1);

 done:
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_tokenize_string_multiple_lines(void *arg)
{
  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  token_rule_t table[] = {
          T01("uptime", K_UPTIME, GE(1), NO_OBJ),
          T01("hibernating", K_HIBERNATING, GE(1), NO_OBJ),
          T1( "published", K_PUBLISHED, CONCAT_ARGS, NO_OBJ),
          END_OF_TABLE,
  };

  char *str = tor_strdup(
          "hibernating 0\nuptime 1024\n"
          "published 2018-10-15 10:00:00\n");

  int retval =
  tokenize_string(area, str, NULL,
                  tokens, table, 0);

  tt_int_op(smartlist_len(tokens), OP_EQ, 3);
  directory_token_t *token = smartlist_get(tokens, 0);

  tt_int_op(token->tp, OP_EQ, K_HIBERNATING);

  token = smartlist_get(tokens, 1);

  tt_int_op(token->tp, OP_EQ, K_UPTIME);

  token = smartlist_get(tokens, 2);

  tt_int_op(token->tp, OP_EQ, K_PUBLISHED);

  tt_int_op(retval, OP_EQ, 0);

 done:
  tor_free(str);
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_tokenize_string_min_cnt(void *arg)
{
  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  token_rule_t table[] = {
          T01("uptime", K_UPTIME, EQ(2), NO_OBJ),
          T01("hibernating", K_HIBERNATING, GE(1), NO_OBJ),
          END_OF_TABLE,
  };

  // Missing "uptime"
  char *str = tor_strdup("uptime 1024\nhibernating 0\n");

  int retval =
  tokenize_string(area, str, NULL,
                  tokens, table, 0);

  tt_int_op(retval, OP_EQ, -1);

 done:
  tor_free(str);
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_tokenize_string_max_cnt(void *arg)
{
  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  token_rule_t table[] = {
          T01("uptime", K_UPTIME, EQ(1), NO_OBJ),
          T01("hibernating", K_HIBERNATING, GE(1), NO_OBJ),
          END_OF_TABLE,
  };

  // "uptime" expected once, but occurs twice in input.
  char *str = tor_strdup(
                  "uptime 1024\nuptime 2048\nhibernating 0\n");

  int retval =
  tokenize_string(area, str, NULL,
                  tokens, table, 0);

  tt_int_op(retval, OP_EQ, -1);

 done:
  tor_free(str);
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_tokenize_string_at_start(void *arg)
{
  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  token_rule_t table[] = {
          T1_START("client-name", C_CLIENT_NAME, CONCAT_ARGS, NO_OBJ),
          T01("uptime", K_UPTIME, EQ(1), NO_OBJ),
          END_OF_TABLE,
  };

  // "client-name" is not the first line.
  char *str = tor_strdup(
                  "uptime 1024\nclient-name Alice\n");

  int retval =
  tokenize_string(area, str, NULL, tokens, table, 0);

  tt_int_op(retval, OP_EQ, -1);

 done:
  tor_free(str);
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_tokenize_string_at_end(void *arg)
{
  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  token_rule_t table[] = {
          T1_END("client-name", C_CLIENT_NAME, CONCAT_ARGS, NO_OBJ),
          T01("uptime", K_UPTIME, EQ(1), NO_OBJ),
          END_OF_TABLE,
  };

  // "client-name" is not the last line.
  char *str = tor_strdup(
                  "client-name Alice\nuptime 1024\n");

  int retval =
  tokenize_string(area, str, NULL, tokens, table, 0);

  tt_int_op(retval, OP_EQ, -1);

 done:
  tor_free(str);
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_tokenize_string_no_annotations(void *arg)
{
  memarea_t *area = memarea_new();
  smartlist_t *tokens = smartlist_new();

  (void)arg;

  token_rule_t table[] = {
          A01("@last-listed", A_LAST_LISTED, CONCAT_ARGS, NO_OBJ),
          END_OF_TABLE,
  };

  char *str = tor_strdup("@last-listed 2018-09-21 15:30:03\n");

  int retval =
  tokenize_string(area, str, NULL, tokens, table, 0);

  tt_int_op(retval, OP_EQ, -1);

 done:
  tor_free(str);
  memarea_drop_all(area);
  smartlist_free(tokens);
  return;
}

static void
test_parsecommon_get_next_token_success(void *arg)
{
  memarea_t *area = memarea_new();
  const char *str = "uptime 1024";
  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t table = T01("uptime", K_UPTIME, GE(1), NO_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &table);

  tt_int_op(token->tp, OP_EQ, K_UPTIME);
  tt_int_op(token->n_args, OP_EQ, 1);
  tt_str_op(*(token->args), OP_EQ, "1024");
  tt_assert(!token->object_type);
  tt_int_op(token->object_size, OP_EQ, 0);
  tt_assert(!token->object_body);

  tt_ptr_op(*s, OP_EQ, end);

 done:
  memarea_drop_all(area);
  return;
}

static void
test_parsecommon_get_next_token_concat_args(void *arg)
{
  memarea_t *area = memarea_new();
  const char *str = "proto A=1 B=2";
  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t rule = T01("proto", K_PROTO, CONCAT_ARGS, NO_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &rule);

  tt_int_op(token->tp, OP_EQ, K_PROTO);
  tt_int_op(token->n_args, OP_EQ, 1);
  tt_str_op(*(token->args), OP_EQ, "A=1 B=2");

 done:
  memarea_drop_all(area);
}

static void
test_parsecommon_get_next_token_parse_keys(void *arg)
{
  (void)arg;

  memarea_t *area = memarea_new();
  const char *str =
    "onion-key\n"
    "-----BEGIN RSA PUBLIC KEY-----\n"
    "MIGJAoGBAMDdIya33BfNlHOkzoTKSTT8EjD64waMfUr372syVHiFjHhObwKwGA5u\n"
    "sHaMIe9r+Ij/4C1dKyuXkcz3DOl6gWNhTD7dZ89I+Okoh1jWe30jxCiAcywC22p5\n"
    "XLhrDkX1A63Z7XCH9ltwU2WMqWsVM98N2GR6MTujP7wtqdLExYN1AgMBAAE=\n"
    "-----END RSA PUBLIC KEY-----\n";

  const char *end = str + strlen(str);
  const char **s = (const char **)&str;
  directory_token_t *token = NULL;
  directory_token_t *token2 = NULL;

  token_rule_t rule = T1("onion-key", R_IPO_ONION_KEY, NO_ARGS, NEED_KEY_1024);

  token = get_next_token(area, s, end, &rule);
  tt_assert(token);

  tt_int_op(token->tp, OP_EQ, R_IPO_ONION_KEY);
  tt_int_op(token->n_args, OP_EQ, 0);
  tt_str_op(token->object_type, OP_EQ, "RSA PUBLIC KEY");
  tt_int_op(token->object_size, OP_EQ, 140);
  tt_assert(token->object_body);
  tt_assert(token->key);
  tt_assert(!token->error);

  const char *str2 =
    "client-key\n"
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXAIBAAKBgQCwS810a2auH2PQchOBz9smNgjlDu31aq0IYlUohSYbhcv5AJ+d\n"
    "DY0nfZWzS+mZPwzL3UiEnTt6PVv7AgoZ5V9ZJWJTKIURjJpkK0mstfJKHKIZhf84\n"
    "pmFfRej9GQViB6NLtp1obOXJgJixSlMfw9doDI4NoAnEISCyH/tD77Qs2wIDAQAB\n"
    "AoGAbDg8CKkdQOnX9c7xFpCnsE8fKqz9eddgHHNwXw1NFTwOt+2gDWKSMZmv2X5S\n"
    "CVZg3owZxf5W0nT0D6Ny2+6nliak7foYAvkD0BsCiBhgftwC0zAo6k5rIbUKB3PJ\n"
    "QLFXgpJhqWuXkODyt/hS/GTernR437WVSEGp1bnALqiFabECQQDaqHOxzoWY/nvH\n"
    "KrfUi8EhqCnqERlRHwrW0MQZ1RPvF16OPPma+xa+ht/amfh3vYN5tZY82Zm43gGl\n"
    "XWL5cZhNAkEAzmdSootYVnqLLLRMfHKXnO1XbaEcA/08MDNKGlSclBJixFenE8jX\n"
    "iQsUbHwMJuGONvzWpRGPBP2f8xBd28ZtxwJARY+LZshtpfNniz/ixYJESaHG28je\n"
    "xfjbKOW3TQSFV+2WTifFvHEeljQwKMoMyoMGvYRwLCGJjs9JtMLVxsdFjQJBAKwD\n"
    "3BBvBQ39TuPQ1zWX4tb7zjMlY83HTFP3Sriq71tP/1QWoL2SUl56B2lp8E6vB/C3\n"
    "wsMK4SCNprHRYAd7VZ0CQDKn6Zhd11P94PLs0msybFEh1VXr6CEW/BrxBgbL4ls6\n"
    "dbX5XO0z4Ra8gYXgObgimhyMDYO98Idt5+Z3HIdyrSc=\n"
    "-----END RSA PRIVATE KEY-----\n";

  const char *end2 = str2 + strlen(str2);
  const char **s2 = (const char **)&str2;

  token_rule_t rule2 = T01("client-key", C_CLIENT_KEY, NO_ARGS, OBJ_OK);
  token2 = get_next_token(area, s2, end2, &rule2);
  tt_assert(token2);
  tt_int_op(token2->tp, OP_EQ, C_CLIENT_KEY);
  tt_int_op(token2->n_args, OP_EQ, 0);
  tt_str_op(token2->object_type, OP_EQ, "RSA PRIVATE KEY");
  tt_int_op(token2->object_size, OP_EQ, 608);
  tt_assert(token2->object_body);
  tt_assert(token2->key == NULL);
  tt_assert(!token->error);

 done:
  if (token) token_clear(token);
  if (token2) token_clear(token2);
  memarea_drop_all(area);
}

static void
test_parsecommon_get_next_token_object(void *arg)
{
  memarea_t *area = memarea_new();

  const char *str =
    "directory-signature 0232AF901C31A04EE9848595AF9BB7620D4C5B2E "
    "CD1FD971855430880D3C31E0331C5C55800C2F79\n"
    "-----BEGIN SIGNATURE-----\n"
    "dLTbc1Lad/OWKBJhA/dERzDHumswTAzBFAWAz2vnQhLsebs1SOm0W/vceEsiEkiF\n"
    "A+JJSzIyfywJc6Mnk7aKMEIFjOO/MaxuAp4zv+q+JonJkF0ExjMqvKR0D6pSFmfN\n"
    "cnemnxGHxNuPDnKl0imbWKmWDsHtwgi4zWeTq3MekfMOXKi6gIh+bDFzCs9/Vquh\n"
    "uNKJI1jW/A2DEKeaSAODEv9VoCsYSvbVVEuHCBWjeNAurd5aL26BrAolW6m7pkD6\n"
    "I+cQ8dQG6Wa/Zt6gLXtBbOP2o/iDI7ahDP9diNkBI/rm4nfp9j4piTwsqpi7xz9J\n"
    "Ua9DEZB9KbJHVX1rGShrLA==\n"
    "-----END SIGNATURE-----\n";

  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t rule = T("directory-signature", K_DIRECTORY_SIGNATURE,
                        GE(2), NEED_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &rule);

  tt_int_op(token->tp, OP_EQ, K_DIRECTORY_SIGNATURE);
  tt_int_op(token->n_args, OP_EQ, 2);
  tt_str_op(token->args[0], OP_EQ,
                  "0232AF901C31A04EE9848595AF9BB7620D4C5B2E");
  tt_str_op(token->args[1], OP_EQ,
                  "CD1FD971855430880D3C31E0331C5C55800C2F79");

  tt_assert(!token->error);

  char decoded[256];
  const char *signature =
    "dLTbc1Lad/OWKBJhA/dERzDHumswTAzBFAWAz2vnQhLsebs1SOm0W/vceEsiEkiF\n"
    "A+JJSzIyfywJc6Mnk7aKMEIFjOO/MaxuAp4zv+q+JonJkF0ExjMqvKR0D6pSFmfN\n"
    "cnemnxGHxNuPDnKl0imbWKmWDsHtwgi4zWeTq3MekfMOXKi6gIh+bDFzCs9/Vquh\n"
    "uNKJI1jW/A2DEKeaSAODEv9VoCsYSvbVVEuHCBWjeNAurd5aL26BrAolW6m7pkD6\n"
    "I+cQ8dQG6Wa/Zt6gLXtBbOP2o/iDI7ahDP9diNkBI/rm4nfp9j4piTwsqpi7xz9J\n"
    "Ua9DEZB9KbJHVX1rGShrLA==\n";
  tt_assert(signature);
  size_t signature_len = strlen(signature);
  base64_decode(decoded, sizeof(decoded), signature, signature_len);

  tt_str_op(token->object_type, OP_EQ, "SIGNATURE");
  tt_int_op(token->object_size, OP_EQ, 256);
  tt_mem_op(token->object_body, OP_EQ, decoded, 256);

  tt_assert(!token->key);

 done:
  memarea_drop_all(area);
}

static void
test_parsecommon_get_next_token_err_too_many_args(void *arg)
{
  memarea_t *area = memarea_new();
  const char *str = "uptime 1024 1024 1024";
  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t table = T01("uptime", K_UPTIME, EQ(1), NO_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &table);

  tt_int_op(token->tp, OP_EQ, ERR_);
  tt_str_op(token->error, OP_EQ, "Too many arguments to uptime");

 done:
  memarea_drop_all(area);
  return;
}

static void
test_parsecommon_get_next_token_err_too_few_args(void *arg)
{
  memarea_t *area = memarea_new();
  const char *str = "uptime";
  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t table = T01("uptime", K_UPTIME, EQ(1), NO_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &table);

  tt_int_op(token->tp, OP_EQ, ERR_);
  tt_str_op(token->error, OP_EQ, "Too few arguments to uptime");

 done:
  memarea_drop_all(area);
  return;
}

static void
test_parsecommon_get_next_token_err_obj_missing_endline(void *arg)
{
  memarea_t *area = memarea_new();

  const char *str =
    "directory-signature 0232AF901C31A04EE9848595AF9BB7620D4C5B2E "
    "CD1FD971855430880D3C31E0331C5C55800C2F79\n"
    "-----BEGIN SIGNATURE-----\n"
    "dLTbc1Lad/OWKBJhA/dERzDHumswTAzBFAWAz2vnQhLsebs1SOm0W/vceEsiEkiF\n"
    "A+JJSzIyfywJc6Mnk7aKMEIFjOO/MaxuAp4zv+q+JonJkF0ExjMqvKR0D6pSFmfN\n"
    "cnemnxGHxNuPDnKl0imbWKmWDsHtwgi4zWeTq3MekfMOXKi6gIh+bDFzCs9/Vquh\n"
    "uNKJI1jW/A2DEKeaSAODEv9VoCsYSvbVVEuHCBWjeNAurd5aL26BrAolW6m7pkD6\n"
    "I+cQ8dQG6Wa/Zt6gLXtBbOP2o/iDI7ahDP9diNkBI/rm4nfp9j4piTwsqpi7xz9J\n"
    "Ua9DEZB9KbJHVX1rGShrLA==\n";

  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t rule = T("directory-signature", K_DIRECTORY_SIGNATURE,
                        GE(2), NEED_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &rule);

  tt_int_op(token->tp, OP_EQ, ERR_);
  tt_str_op(token->error, OP_EQ, "Malformed object: missing object end line");

 done:
  memarea_drop_all(area);
  return;
}

static void
test_parsecommon_get_next_token_err_bad_beginline(void *arg)
{
  memarea_t *area = memarea_new();

  const char *str =
    "directory-signature 0232AF901C31A04EE9848595AF9BB7620D4C5B2E "
    "CD1FD971855430880D3C31E0331C5C55800C2F79\n"
    "-----BEGIN SIGNATURE-Z---\n"
    "dLTbc1Lad/OWKBJhA/dERzDHumswTAzBFAWAz2vnQhLsebs1SOm0W/vceEsiEkiF\n"
    "A+JJSzIyfywJc6Mnk7aKMEIFjOO/MaxuAp4zv+q+JonJkF0ExjMqvKR0D6pSFmfN\n"
    "cnemnxGHxNuPDnKl0imbWKmWDsHtwgi4zWeTq3MekfMOXKi6gIh+bDFzCs9/Vquh\n"
    "uNKJI1jW/A2DEKeaSAODEv9VoCsYSvbVVEuHCBWjeNAurd5aL26BrAolW6m7pkD6\n"
    "I+cQ8dQG6Wa/Zt6gLXtBbOP2o/iDI7ahDP9diNkBI/rm4nfp9j4piTwsqpi7xz9J\n"
    "Ua9DEZB9KbJHVX1rGShrLA==\n"
    "-----END SIGNATURE-----\n";

  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t rule = T("directory-signature", K_DIRECTORY_SIGNATURE,
                        GE(2), NEED_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &rule);

  tt_int_op(token->tp, OP_EQ, ERR_);
  tt_str_op(token->error, OP_EQ, "Malformed object: bad begin line");

 done:
  memarea_drop_all(area);
  return;
}

static void
test_parsecommon_get_next_token_err_tag_mismatch(void *arg)
{
  memarea_t *area = memarea_new();

  const char *str =
    "directory-signature 0232AF901C31A04EE9848595AF9BB7620D4C5B2E "
    "CD1FD971855430880D3C31E0331C5C55800C2F79\n"
    "-----BEGIN SIGNATURE-----\n"
    "dLTbc1Lad/OWKBJhA/dERzDHumswTAzBFAWAz2vnQhLsebs1SOm0W/vceEsiEkiF\n"
    "A+JJSzIyfywJc6Mnk7aKMEIFjOO/MaxuAp4zv+q+JonJkF0ExjMqvKR0D6pSFmfN\n"
    "cnemnxGHxNuPDnKl0imbWKmWDsHtwgi4zWeTq3MekfMOXKi6gIh+bDFzCs9/Vquh\n"
    "uNKJI1jW/A2DEKeaSAODEv9VoCsYSvbVVEuHCBWjeNAurd5aL26BrAolW6m7pkD6\n"
    "I+cQ8dQG6Wa/Zt6gLXtBbOP2o/iDI7ahDP9diNkBI/rm4nfp9j4piTwsqpi7xz9J\n"
    "Ua9DEZB9KbJHVX1rGShrLA==\n"
    "-----END SOMETHINGELSE-----\n";

  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t rule = T("directory-signature", K_DIRECTORY_SIGNATURE,
                        GE(2), NEED_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &rule);

  tt_int_op(token->tp, OP_EQ, ERR_);
  tt_str_op(token->error, OP_EQ,
            "Malformed object: mismatched end tag SIGNATURE");

 done:
  memarea_drop_all(area);
  return;
}

static void
test_parsecommon_get_next_token_err_bad_base64(void *arg)
{
  memarea_t *area = memarea_new();

  const char *str =
    "directory-signature 0232AF901C31A04EE9848595AF9BB7620D4C5B2E "
    "CD1FD971855430880D3C31E0331C5C55800C2F79\n"
    "-----BEGIN SIGNATURE-----\n"
    "%%@%%%%%%%!!!'\n"
    "-----END SIGNATURE-----\n";

  const char *end = str + strlen(str);
  const char **s = &str;
  token_rule_t rule = T("directory-signature", K_DIRECTORY_SIGNATURE,
                        GE(2), NEED_OBJ);
  (void)arg;

  directory_token_t *token = get_next_token(area, s, end, &rule);

  tt_int_op(token->tp, OP_EQ, ERR_);
  tt_str_op(token->error, OP_EQ, "Malformed object: bad base64-encoded data");

 done:
  memarea_drop_all(area);
  return;
}

#define PARSECOMMON_TEST(name) \
  { #name, test_parsecommon_ ## name, 0, NULL, NULL }

struct testcase_t parsecommon_tests[] = {
  PARSECOMMON_TEST(tokenize_string_null),
  PARSECOMMON_TEST(tokenize_string_multiple_lines),
  PARSECOMMON_TEST(tokenize_string_min_cnt),
  PARSECOMMON_TEST(tokenize_string_max_cnt),
  PARSECOMMON_TEST(tokenize_string_at_start),
  PARSECOMMON_TEST(tokenize_string_at_end),
  PARSECOMMON_TEST(tokenize_string_no_annotations),
  PARSECOMMON_TEST(get_next_token_success),
  PARSECOMMON_TEST(get_next_token_concat_args),
  PARSECOMMON_TEST(get_next_token_parse_keys),
  PARSECOMMON_TEST(get_next_token_object),
  PARSECOMMON_TEST(get_next_token_err_too_many_args),
  PARSECOMMON_TEST(get_next_token_err_too_few_args),
  PARSECOMMON_TEST(get_next_token_err_obj_missing_endline),
  PARSECOMMON_TEST(get_next_token_err_bad_beginline),
  PARSECOMMON_TEST(get_next_token_err_tag_mismatch),
  PARSECOMMON_TEST(get_next_token_err_bad_base64),
  END_OF_TESTCASES
};
