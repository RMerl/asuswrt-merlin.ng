/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_proto_misc.c
 * \brief Test our smaller buffer-based protocol functions
 */

#include "core/or/or.h"
#include "test/test.h"
#include "lib/buf/buffers.h"
#include "core/or/connection_or.h"
#include "feature/relay/ext_orport.h"
#include "core/proto/proto_cell.h"
#include "core/proto/proto_control0.h"
#include "core/proto/proto_ext_or.h"

#include "core/or/var_cell_st.h"

static void
test_proto_var_cell(void *arg)
{
  (void)arg;
  char *mem_op_hex_tmp = NULL;
  char tmp[1024];
  buf_t *buf = NULL;
  var_cell_t *cell = NULL;

  buf = buf_new();
  memset(tmp, 0xf0, sizeof(tmp));

  /* Short little commands will make us say "no cell yet." */
  tt_int_op(0, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 4));
  tt_ptr_op(cell, OP_EQ, NULL);
  buf_add(buf, "\x01\x02\x02\0x2", 4);
  tt_int_op(0, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 4));
  /* An incomplete fixed-length cell makes us say "no cell yet". */
  buf_add(buf, "\x03", 1);
  tt_int_op(0, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 4));
  /* A complete fixed length-cell makes us say "not a variable-length cell" */
  buf_add(buf, tmp, 509);
  tt_int_op(0, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 4));
  buf_clear(buf);

  /* An incomplete versions cell is a variable-length cell that isn't ready
   * yet. */
  buf_add(buf,
          "\x01\x02\x03\x04" /* circid */
          "\x07" /* VERSIONS */
          "\x00\x04" /* 4 bytes long */
          "\x00" /* incomplete */, 8);
  tt_int_op(1, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 4));
  tt_ptr_op(cell, OP_EQ, NULL);
  /* Complete it, and it's a variable-length cell. Leave a byte on the end for
   * fun. */
  buf_add(buf, "\x09\x00\x25\ff", 4);
  tt_int_op(1, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 4));
  tt_ptr_op(cell, OP_NE, NULL);
  tt_int_op(cell->command, OP_EQ, CELL_VERSIONS);
  tt_uint_op(cell->circ_id, OP_EQ, 0x01020304);
  tt_int_op(cell->payload_len, OP_EQ, 4);
  test_mem_op_hex(cell->payload, OP_EQ, "00090025");
  var_cell_free(cell);
  cell = NULL;
  tt_int_op(buf_datalen(buf), OP_EQ, 1);
  buf_clear(buf);

  /* In link protocol 3 and earlier, circid fields were two bytes long. Let's
   * ensure that gets handled correctly. */
  buf_add(buf,
          "\x23\x45\x81\x00\x06" /* command 81; 6 bytes long */
          "coraje", 11);
  tt_int_op(1, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 3));
  tt_ptr_op(cell, OP_NE, NULL);
  tt_int_op(cell->command, OP_EQ, 129);
  tt_uint_op(cell->circ_id, OP_EQ, 0x2345);
  tt_int_op(cell->payload_len, OP_EQ, 6);
  tt_mem_op(cell->payload, OP_EQ, "coraje", 6);
  var_cell_free(cell);
  cell = NULL;
  tt_int_op(buf_datalen(buf), OP_EQ, 0);

  /* In link protocol 2, only VERSIONS cells counted as variable-length */
  buf_add(buf,
          "\x23\x45\x81\x00\x06"
          "coraje", 11); /* As above */
  tt_int_op(0, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 2));
  buf_clear(buf);
  buf_add(buf,
          "\x23\x45\x07\x00\x06"
          "futuro", 11);
  tt_int_op(1, OP_EQ, fetch_var_cell_from_buf(buf, &cell, 2));
  tt_ptr_op(cell, OP_NE, NULL);
  tt_int_op(cell->command, OP_EQ, 7);
  tt_uint_op(cell->circ_id, OP_EQ, 0x2345);
  tt_int_op(cell->payload_len, OP_EQ, 6);
  tt_mem_op(cell->payload, OP_EQ, "futuro", 6);
  var_cell_free(cell);
  cell = NULL;

 done:
  buf_free(buf);
  var_cell_free(cell);
  tor_free(mem_op_hex_tmp);
}

static void
test_proto_control0(void *arg)
{
  (void)arg;
  buf_t *buf = buf_new();

  /* The only remaining function for the v0 control protocol is the function
     that detects whether the user has stumbled across an old controller
     that's using it.  The format was:
        u16 length;
        u16 command;
        u8 body[length];
  */

  /* Empty buffer -- nothing to do. */
  tt_int_op(0, OP_EQ, peek_buf_has_control0_command(buf));
  /* 3 chars in buf -- can't tell */
  buf_add(buf, "AUT", 3);
  tt_int_op(0, OP_EQ, peek_buf_has_control0_command(buf));
  /* command in buf -- easy to tell */
  buf_add(buf, "HENTICATE ", 10);
  tt_int_op(0, OP_EQ, peek_buf_has_control0_command(buf));

  /* Control0 command header in buf: make sure we detect it. */
  buf_clear(buf);
  buf_add(buf, "\x09\x05" "\x00\x05" "blah", 8);
  tt_int_op(1, OP_EQ, peek_buf_has_control0_command(buf));

 done:
  buf_free(buf);
}

static void
test_proto_ext_or_cmd(void *arg)
{
  ext_or_cmd_t *cmd = NULL;
  buf_t *buf = buf_new();
  char *tmp = NULL;
  (void) arg;

  /* Empty -- should give "not there. */
  tt_int_op(0, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_EQ, cmd);

  /* Three bytes: shouldn't work. */
  buf_add(buf, "\x00\x20\x00", 3);
  tt_int_op(0, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_EQ, cmd);
  tt_int_op(3, OP_EQ, buf_datalen(buf));

  /* 0020 0000: That's a nil command. It should work. */
  buf_add(buf, "\x00", 1);
  tt_int_op(1, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_NE, cmd);
  tt_int_op(0x20, OP_EQ, cmd->cmd);
  tt_int_op(0, OP_EQ, cmd->len);
  tt_int_op(0, OP_EQ, buf_datalen(buf));
  ext_or_cmd_free(cmd);
  cmd = NULL;

  /* Now try a length-6 command with one byte missing. */
  buf_add(buf, "\x10\x21\x00\x06""abcde", 9);
  tt_int_op(0, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_EQ, cmd);
  buf_add(buf, "f", 1);
  tt_int_op(1, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_NE, cmd);
  tt_int_op(0x1021, OP_EQ, cmd->cmd);
  tt_int_op(6, OP_EQ, cmd->len);
  tt_mem_op("abcdef", OP_EQ, cmd->body, 6);
  tt_int_op(0, OP_EQ, buf_datalen(buf));
  ext_or_cmd_free(cmd);
  cmd = NULL;

  /* Now try a length-10 command with 4 extra bytes. */
  buf_add(buf, "\xff\xff\x00\x0aloremipsum\x10\x00\xff\xff", 18);
  tt_int_op(1, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_NE, cmd);
  tt_int_op(0xffff, OP_EQ, cmd->cmd);
  tt_int_op(10, OP_EQ, cmd->len);
  tt_mem_op("loremipsum", OP_EQ, cmd->body, 10);
  tt_int_op(4, OP_EQ, buf_datalen(buf));
  ext_or_cmd_free(cmd);
  cmd = NULL;

  /* Finally, let's try a maximum-length command. We already have the header
   * waiting. */
  tt_int_op(0, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tmp = tor_malloc_zero(65535);
  buf_add(buf, tmp, 65535);
  tt_int_op(1, OP_EQ, fetch_ext_or_command_from_buf(buf, &cmd));
  tt_ptr_op(NULL, OP_NE, cmd);
  tt_int_op(0x1000, OP_EQ, cmd->cmd);
  tt_int_op(0xffff, OP_EQ, cmd->len);
  tt_mem_op(tmp, OP_EQ, cmd->body, 65535);
  tt_int_op(0, OP_EQ, buf_datalen(buf));
  ext_or_cmd_free(cmd);
  cmd = NULL;

 done:
  ext_or_cmd_free(cmd);
  buf_free(buf);
  tor_free(tmp);
}

static void
test_proto_line(void *arg)
{
  (void)arg;
  char tmp[60];
  buf_t *buf = buf_new();
#define S(str) str, sizeof(str)-1
  const struct {
    const char *input;
    size_t input_len;
    size_t line_len;
    const char *output;
    int returnval;
  } cases[] = {
    { S("Hello world"), 0, NULL, 0 },
    { S("Hello world\n"), 12, "Hello world\n", 1 },
    { S("Hello world\nMore"), 12, "Hello world\n", 1 },
    { S("\n oh hello world\nMore"), 1, "\n", 1 },
    { S("Hello worpd\n\nMore"), 12, "Hello worpd\n", 1 },
    { S("------------------------------------------------------------\n"), 0,
      NULL, -1 },
  };
  unsigned i;
  for (i = 0; i < ARRAY_LENGTH(cases); ++i) {
    buf_add(buf, cases[i].input, cases[i].input_len);
    memset(tmp, 0xfe, sizeof(tmp));
    size_t sz = sizeof(tmp);
    int rv = buf_get_line(buf, tmp, &sz);
    tt_int_op(rv, OP_EQ, cases[i].returnval);
    if (rv == 1) {
      tt_int_op(sz, OP_LT, sizeof(tmp));
      tt_mem_op(cases[i].output, OP_EQ, tmp, sz+1);
      tt_int_op(buf_datalen(buf), OP_EQ, cases[i].input_len - strlen(tmp));
      tt_int_op(sz, OP_EQ, cases[i].line_len);
    } else {
      tt_int_op(buf_datalen(buf), OP_EQ, cases[i].input_len);
      // tt_int_op(sz, OP_EQ, sizeof(tmp));
    }
    buf_clear(buf);
  }

 done:
  buf_free(buf);
}

struct testcase_t proto_misc_tests[] = {
  { "var_cell", test_proto_var_cell, 0, NULL, NULL },
  { "control0", test_proto_control0, 0, NULL, NULL },
  { "ext_or_cmd", test_proto_ext_or_cmd, TT_FORK, NULL, NULL },
  { "line", test_proto_line, 0, NULL, NULL },

  END_OF_TESTCASES
};

