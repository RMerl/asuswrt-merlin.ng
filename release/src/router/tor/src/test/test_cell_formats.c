/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define CONNECTION_EDGE_PRIVATE
#define RELAY_PRIVATE
#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/connection_edge.h"
#include "core/or/connection_or.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/or/onion.h"
#include "core/crypto/onion_fast.h"
#include "core/crypto/onion_ntor.h"
#include "core/or/relay.h"
#include "core/or/relay_msg.h"

#include "core/or/cell_st.h"
#include "core/or/cell_queue_st.h"
#include "core/or/relay_msg_st.h"
#include "core/or/var_cell_st.h"

#include "test/test.h"

#include <stdlib.h>
#include <string.h>

static void
test_cfmt_relay_header(void *arg)
{
  relay_header_t rh;
  NONSTRING const uint8_t hdr_1[RELAY_HEADER_SIZE_V0] =
    "\x03" "\x00\x00" "\x21\x22" "ABCD" "\x01\x03";
  uint8_t hdr_out[RELAY_HEADER_SIZE_V0];
  (void)arg;

  tt_int_op(sizeof(hdr_1), OP_EQ, RELAY_HEADER_SIZE_V0);
  relay_header_unpack(&rh, hdr_1);
  tt_int_op(rh.command, OP_EQ, 3);
  tt_int_op(rh.recognized, OP_EQ, 0);
  tt_int_op(rh.stream_id, OP_EQ, 0x2122);
  tt_mem_op(rh.integrity, OP_EQ, "ABCD", 4);
  tt_int_op(rh.length, OP_EQ, 0x103);

  relay_header_pack(hdr_out, &rh);
  tt_mem_op(hdr_out, OP_EQ, hdr_1, RELAY_HEADER_SIZE_V0);

 done:
  ;
}

static void
make_relay_msg(relay_msg_t *out, uint8_t command,
               const void *body, size_t bodylen)
{
  memset(out, 0, sizeof(*out));
  out->command = command;
  out->body = (uint8_t *)body;
  out->length = bodylen;
  out->stream_id = 5;
}

static void
test_cfmt_begin_cells(void *arg)
{
  relay_msg_t msg;
  begin_cell_t bcell;
  uint8_t end_reason;
  (void)arg;

  /* Try begindir. */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN_DIR, "", 0);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_ptr_op(NULL, OP_EQ, bcell.address);
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(0, OP_EQ, bcell.port);
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_int_op(1, OP_EQ, bcell.is_begindir);

  /* A Begindir with extra stuff. */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN_DIR, "12345", 5);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_ptr_op(NULL, OP_EQ, bcell.address);
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(0, OP_EQ, bcell.port);
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_int_op(1, OP_EQ, bcell.is_begindir);

  /* A short but valid begin cell */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b:9", 6);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_str_op("a.b", OP_EQ, bcell.address);
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(9, OP_EQ, bcell.port);
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_int_op(0, OP_EQ, bcell.is_begindir);
  tor_free(bcell.address);

  /* A significantly loner begin cell */
  memset(&bcell, 0x7f, sizeof(bcell));
  {
  const char c[] = "here-is-a-nice-long.hostname.com:65535";
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, c, strlen(c)+1);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_str_op("here-is-a-nice-long.hostname.com", OP_EQ, bcell.address);
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(65535, OP_EQ, bcell.port);
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_int_op(0, OP_EQ, bcell.is_begindir);
  tor_free(bcell.address);
  }

  /* An IPv4 begin cell. */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "18.9.22.169:80", 15);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_str_op("18.9.22.169", OP_EQ, bcell.address);
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(80, OP_EQ, bcell.port);
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_int_op(0, OP_EQ, bcell.is_begindir);
  tor_free(bcell.address);

  /* An IPv6 begin cell. Let's make sure we handle colons*/
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN,
                  "[2620::6b0:b:1a1a:0:26e5:480e]:80", 34);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_str_op("[2620::6b0:b:1a1a:0:26e5:480e]", OP_EQ, bcell.address);
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(80, OP_EQ, bcell.port);
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_int_op(0, OP_EQ, bcell.is_begindir);
  tor_free(bcell.address);

  /* a begin cell with extra junk but not enough for flags. */
  memset(&bcell, 0x7f, sizeof(bcell));
  {
    const char c[] = "another.example.com:80\x00\x01\x02";
    make_relay_msg(&msg, RELAY_COMMAND_BEGIN, c, sizeof(c)-1);
    tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
    tt_str_op("another.example.com", OP_EQ, bcell.address);
    tt_int_op(0, OP_EQ, bcell.flags);
    tt_int_op(80, OP_EQ, bcell.port);
    tt_int_op(5, OP_EQ, bcell.stream_id);
    tt_int_op(0, OP_EQ, bcell.is_begindir);
    tor_free(bcell.address);
  }

  /* a begin cell with flags. */
  memset(&bcell, 0x7f, sizeof(bcell));
  {
    const char c[] = "another.example.com:443\x00\x01\x02\x03\x04";
    make_relay_msg(&msg, RELAY_COMMAND_BEGIN, c, sizeof(c)-1);
    tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
    tt_str_op("another.example.com", OP_EQ, bcell.address);
    tt_int_op(0x1020304, OP_EQ, bcell.flags);
    tt_int_op(443, OP_EQ, bcell.port);
    tt_int_op(5, OP_EQ, bcell.stream_id);
    tt_int_op(0, OP_EQ, bcell.is_begindir);
    tor_free(bcell.address);
  }

  /* a begin cell with flags and even more cruft after that. */
  memset(&bcell, 0x7f, sizeof(bcell));
  {
    const char c[] = "a-further.example.com:22\x00\xee\xaa\x00\xffHi mom";
    make_relay_msg(&msg, RELAY_COMMAND_BEGIN, c, sizeof(c)-1);
    tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
    tt_str_op("a-further.example.com", OP_EQ, bcell.address);
    tt_int_op(0xeeaa00ff, OP_EQ, bcell.flags);
    tt_int_op(22, OP_EQ, bcell.port);
    tt_int_op(5, OP_EQ, bcell.stream_id);
    tt_int_op(0, OP_EQ, bcell.is_begindir);
    tor_free(bcell.address);
  }

#if 0
  // Note: This is now checked at when we decode the relay message.
  /* bad begin cell: impossible length. */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b:80", 7);
  msg.length = 510;
  tt_int_op(-2, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
#endif

  /* Bad begin cell: no body. */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "", 0);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));

  /* bad begin cell: no body. */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "", 0);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));

  /* bad begin cell: no colon */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b", 4);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));

  /* bad begin cell: no ports */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b:", 5);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));

  /* bad begin cell: bad port */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b:xyz", 8);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b:100000", 11);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));

  /* bad begin cell: no nul */
  memset(&bcell, 0x7f, sizeof(bcell));
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, "a.b:80", 6);
  tt_int_op(-1, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));

  /* bad begin cell: this makes sure we never read out of bound (#41245).
   *
   * We set 10 bytes in the payload (address + 3 bytes of flags). Omitting the
   * last flag byte triggers an off-by-one: the buggy condition (>=) would read
   * out of ound. The fixed condition (>) does not set flags at all. */
  memset(&bcell, 0x7f, sizeof(bcell));
  const char payload[] = "a.b:80\x00\x42\x43\x44";
  make_relay_msg(&msg, RELAY_COMMAND_BEGIN, payload, sizeof(payload) - 1);
  tt_int_op(0, OP_EQ, begin_cell_parse(&msg, &bcell, &end_reason));
  tt_int_op(5, OP_EQ, bcell.stream_id);
  tt_str_op("a.b", OP_EQ, bcell.address);
  tt_int_op(80, OP_EQ, bcell.port);
  /* With the bug it would be 0x424344<junk> */
  tt_int_op(0, OP_EQ, bcell.flags);
  tt_int_op(0, OP_EQ, bcell.is_begindir);
  tor_free(bcell.address);

 done:
  tor_free(bcell.address);
}

static void
test_cfmt_connected_cells(void *arg)
{
  tor_addr_t addr;
  int ttl, r;
  char *mem_op_hex_tmp = NULL;
  relay_msg_t msg;
  uint8_t buf[512];
  (void)arg;

  /* Let's try an oldschool one with nothing in it. */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED, "", 0);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_UNSPEC);
  tt_int_op(ttl, OP_EQ, -1);

  /* A slightly less oldschool one: only an IPv4 address */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED, "\x20\x30\x40\x50", 4);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET);
  tt_str_op(fmt_addr(&addr), OP_EQ, "32.48.64.80");
  tt_int_op(ttl, OP_EQ, -1);

  /* Bogus but understandable: truncated TTL */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED, "\x11\x12\x13\x14\x15", 5);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET);
  tt_str_op(fmt_addr(&addr), OP_EQ, "17.18.19.20");
  tt_int_op(ttl, OP_EQ, -1);

  /* Regular IPv4 one: address and TTL */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x02\x03\x04\x05\x00\x00\x0e\x10", 8);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET);
  tt_str_op(fmt_addr(&addr), OP_EQ, "2.3.4.5");
  tt_int_op(ttl, OP_EQ, 3600);

  /* IPv4 with too-big TTL */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x02\x03\x04\x05\xf0\x00\x00\x00", 8);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET);
  tt_str_op(fmt_addr(&addr), OP_EQ, "2.3.4.5");
  tt_int_op(ttl, OP_EQ, -1);

  /* IPv6 (ttl is mandatory) */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x00\x00\x00\x00\x06"
                  "\x26\x07\xf8\xb0\x40\x0c\x0c\x02"
                  "\x00\x00\x00\x00\x00\x00\x00\x68"
                  "\x00\x00\x02\x58", 25);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET6);
  tt_str_op(fmt_addr(&addr), OP_EQ, "2607:f8b0:400c:c02::68");
  tt_int_op(ttl, OP_EQ, 600);

  /* IPv6 (ttl too big) */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x00\x00\x00\x00\x06"
                  "\x26\x07\xf8\xb0\x40\x0c\x0c\x02"
                  "\x00\x00\x00\x00\x00\x00\x00\x68"
                  "\x90\x00\x02\x58", 25);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET6);
  tt_str_op(fmt_addr(&addr), OP_EQ, "2607:f8b0:400c:c02::68");
  tt_int_op(ttl, OP_EQ, -1);

  /* Bogus size: 3. */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x00\x01\x02", 3);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, -1);

  /* Bogus family: 7. */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x00\x00\x00\x00\x07"
                  "\x26\x07\xf8\xb0\x40\x0c\x0c\x02"
                  "\x00\x00\x00\x00\x00\x00\x00\x68"
                  "\x90\x00\x02\x58", 25);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, -1);

  /* Truncated IPv6. */
  make_relay_msg(&msg, RELAY_COMMAND_CONNECTED,
                  "\x00\x00\x00\x00\x06"
                  "\x26\x07\xf8\xb0\x40\x0c\x0c\x02"
                  "\x00\x00\x00\x00\x00\x00\x00\x68"
                  "\x00\x00\x02", 24);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, -1);

  /* Now make sure we can generate connected cells correctly. */
  /* Try an IPv4 address */
  tor_addr_parse(&addr, "30.40.50.60");
  msg.body = buf;
  msg.length = connected_cell_format_payload(buf, &addr, 1024);
  tt_int_op(msg.length, OP_EQ, 8);
  test_memeq_hex(msg.body, "1e28323c" "00000400");

  /* Try parsing it. */
  tor_addr_make_unspec(&addr);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET);
  tt_str_op(fmt_addr(&addr), OP_EQ, "30.40.50.60");
  tt_int_op(ttl, OP_EQ, 1024);

  /* Try an IPv6 address */
  tor_addr_parse(&addr, "2620::6b0:b:1a1a:0:26e5:480e");
  msg.length = connected_cell_format_payload(buf, &addr, 3600);
  tt_int_op(msg.length, OP_EQ, 25);
  test_memeq_hex(msg.body,
                 "00000000" "06"
                 "2620000006b0000b1a1a000026e5480e" "00000e10");

  /* Try parsing it. */
  tor_addr_make_unspec(&addr);
  r = connected_cell_parse(&msg, &addr, &ttl);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(tor_addr_family(&addr), OP_EQ, AF_INET6);
  tt_str_op(fmt_addr(&addr), OP_EQ, "2620:0:6b0:b:1a1a:0:26e5:480e");
  tt_int_op(ttl, OP_EQ, 3600);

 done:
  tor_free(mem_op_hex_tmp);
}

static void
test_cfmt_create_cells(void *arg)
{
  uint8_t b[MAX_ONIONSKIN_CHALLENGE_LEN];
  create_cell_t cc;
  cell_t cell;
  cell_t cell2;

  (void)arg;

  /* === Let's try parsing some good cells! */

  /* A valid create_fast cell. */
  memset(&cell, 0, sizeof(cell));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, CREATE_FAST_LEN);
  cell.command = CELL_CREATE_FAST;
  memcpy(cell.payload, b, CREATE_FAST_LEN);
  tt_int_op(0, OP_EQ, create_cell_parse(&cc, &cell));
  tt_int_op(CELL_CREATE_FAST, OP_EQ, cc.cell_type);
  tt_int_op(ONION_HANDSHAKE_TYPE_FAST, OP_EQ, cc.handshake_type);
  tt_int_op(CREATE_FAST_LEN, OP_EQ, cc.handshake_len);
  tt_mem_op(cc.onionskin,OP_EQ, b, CREATE_FAST_LEN + 10);
  tt_int_op(0, OP_EQ, create_cell_format(&cell2, &cc));
  tt_int_op(cell.command, OP_EQ, cell2.command);
  tt_mem_op(cell.payload,OP_EQ, cell2.payload, CELL_PAYLOAD_SIZE);

  /* A valid create2 cell with an ntor payload */
  memset(&cell, 0, sizeof(cell));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, NTOR_ONIONSKIN_LEN);
  cell.command = CELL_CREATE2;
  memcpy(cell.payload, "\x00\x02\x00\x54", 4); /* ntor, 84 bytes long */
  memcpy(cell.payload+4, b, NTOR_ONIONSKIN_LEN);
  tt_int_op(0, OP_EQ, create_cell_parse(&cc, &cell));
  tt_int_op(CELL_CREATE2, OP_EQ, cc.cell_type);
  tt_int_op(ONION_HANDSHAKE_TYPE_NTOR, OP_EQ, cc.handshake_type);
  tt_int_op(NTOR_ONIONSKIN_LEN, OP_EQ, cc.handshake_len);
  tt_mem_op(cc.onionskin,OP_EQ, b, NTOR_ONIONSKIN_LEN + 10);
  tt_int_op(0, OP_EQ, create_cell_format(&cell2, &cc));
  tt_int_op(cell.command, OP_EQ, cell2.command);
  tt_mem_op(cell.payload,OP_EQ, cell2.payload, CELL_PAYLOAD_SIZE);

  /* == Okay, now let's try to parse some impossible stuff. */

  /* It has to be some kind of a create cell! */
  cell.command = CELL_CREATED;
  tt_int_op(-1, OP_EQ, create_cell_parse(&cc, &cell));

  /* You can't actually make an unparseable CREATE or CREATE_FAST cell. */

  /* Try some CREATE2 cells.  First with a bad type. */
  cell.command = CELL_CREATE2;
  memcpy(cell.payload, "\x00\x50\x00\x99", 4); /* Type 0x50???? */
  tt_int_op(-1, OP_EQ, create_cell_parse(&cc, &cell));
  /* Now a good type with an incorrect length. */
  memcpy(cell.payload, "\x00\x00\x00\xBC", 4); /* TAP, 187 bytes.*/
  tt_int_op(-1, OP_EQ, create_cell_parse(&cc, &cell));
  /* Now a good type with a ridiculous length. */
  memcpy(cell.payload, "\x00\x00\x02\x00", 4); /* TAP, 512 bytes.*/
  tt_int_op(-1, OP_EQ, create_cell_parse(&cc, &cell));

  /* == Time to try formatting bad cells.  The important thing is that
     we reject big lengths, so just check that for now. */
  cc.handshake_len = 512;
  tt_int_op(-1, OP_EQ, create_cell_format(&cell2, &cc));

  /* == Try formatting a create2 cell we don't understand. XXXX */

 done:
  ;
}

static void
test_cfmt_created_cells(void *arg)
{
  uint8_t b[512];
  created_cell_t cc;
  cell_t cell;
  cell_t cell2;

  (void)arg;

  /* A good CREATED_FAST cell */
  memset(&cell, 0, sizeof(cell));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, CREATED_FAST_LEN);
  cell.command = CELL_CREATED_FAST;
  memcpy(cell.payload, b, CREATED_FAST_LEN);
  tt_int_op(0, OP_EQ, created_cell_parse(&cc, &cell));
  tt_int_op(CELL_CREATED_FAST, OP_EQ, cc.cell_type);
  tt_int_op(CREATED_FAST_LEN, OP_EQ, cc.handshake_len);
  tt_mem_op(cc.reply,OP_EQ, b, CREATED_FAST_LEN + 10);
  tt_int_op(0, OP_EQ, created_cell_format(&cell2, &cc));
  tt_int_op(cell.command, OP_EQ, cell2.command);
  tt_mem_op(cell.payload,OP_EQ, cell2.payload, CELL_PAYLOAD_SIZE);

  /* A good CREATED2 cell with short reply */
  memset(&cell, 0, sizeof(cell));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, 64);
  cell.command = CELL_CREATED2;
  memcpy(cell.payload, "\x00\x40", 2);
  memcpy(cell.payload+2, b, 64);
  tt_int_op(0, OP_EQ, created_cell_parse(&cc, &cell));
  tt_int_op(CELL_CREATED2, OP_EQ, cc.cell_type);
  tt_int_op(64, OP_EQ, cc.handshake_len);
  tt_mem_op(cc.reply,OP_EQ, b, 80);
  tt_int_op(0, OP_EQ, created_cell_format(&cell2, &cc));
  tt_int_op(cell.command, OP_EQ, cell2.command);
  tt_mem_op(cell.payload,OP_EQ, cell2.payload, CELL_PAYLOAD_SIZE);

  /* A good CREATED2 cell with maximal reply */
  memset(&cell, 0, sizeof(cell));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, 496);
  cell.command = CELL_CREATED2;
  memcpy(cell.payload, "\x01\xF0", 2);
  memcpy(cell.payload+2, b, 496);
  tt_int_op(0, OP_EQ, created_cell_parse(&cc, &cell));
  tt_int_op(CELL_CREATED2, OP_EQ, cc.cell_type);
  tt_int_op(496, OP_EQ, cc.handshake_len);
  tt_mem_op(cc.reply,OP_EQ, b, 496);
  tt_int_op(0, OP_EQ, created_cell_format(&cell2, &cc));
  tt_int_op(cell.command, OP_EQ, cell2.command);
  tt_mem_op(cell.payload,OP_EQ, cell2.payload, CELL_PAYLOAD_SIZE);

  /* Bogus CREATED2 cell: too long! */
  memset(&cell, 0, sizeof(cell));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, 496);
  cell.command = CELL_CREATED2;
  memcpy(cell.payload, "\x02\xFF", 2);
  tt_int_op(-1, OP_EQ, created_cell_parse(&cc, &cell));

  /* Unformattable CREATED2 cell: too long! */
  cc.handshake_len = 508;
  tt_int_op(-1, OP_EQ, created_cell_format(&cell2, &cc));

 done:
  ;
}

static void
test_cfmt_extend_cells(void *arg)
{
  cell_t cell;
  uint8_t b[512];
  extend_cell_t ec;
  create_cell_t *cc = &ec.create_cell;
  uint8_t p[RELAY_PAYLOAD_SIZE];
  uint8_t p2[RELAY_PAYLOAD_SIZE];
  uint8_t p2_cmd;
  uint16_t p2_len;
  char *mem_op_hex_tmp = NULL;

  (void) arg;

  /* Now let's do a minimal ntor EXTEND2 cell. */
  memset(&ec, 0xff, sizeof(ec));
  memset(p, 0, sizeof(p));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, NTOR_ONIONSKIN_LEN);
  /* 2 items; one 18.244.0.1:61681 */
  memcpy(p, "\x02\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  /* The other is a digest. */
  memcpy(p+9, "\x02\x14" "anarchoindividualist", 22);
  /* Prep for the handshake: type and length */
  memcpy(p+31, "\x00\x02\x00\x54", 4);
  memcpy(p+35, b, NTOR_ONIONSKIN_LEN);
  tt_int_op(0, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                     p, 35+NTOR_ONIONSKIN_LEN));
  tt_int_op(RELAY_COMMAND_EXTEND2, OP_EQ, ec.cell_type);
  tt_str_op("18.244.0.1", OP_EQ, fmt_addr(&ec.orport_ipv4.addr));
  tt_int_op(61681, OP_EQ, ec.orport_ipv4.port);
  tt_int_op(AF_UNSPEC, OP_EQ, tor_addr_family(&ec.orport_ipv6.addr));
  tt_mem_op(ec.node_id,OP_EQ, "anarchoindividualist", 20);
  tt_int_op(cc->cell_type, OP_EQ, CELL_CREATE2);
  tt_int_op(cc->handshake_type, OP_EQ, ONION_HANDSHAKE_TYPE_NTOR);
  tt_int_op(cc->handshake_len, OP_EQ, NTOR_ONIONSKIN_LEN);
  tt_mem_op(cc->onionskin,OP_EQ, b, NTOR_ONIONSKIN_LEN+20);
  tt_int_op(0, OP_EQ, extend_cell_format(&p2_cmd, &p2_len, p2, &ec));
  tt_int_op(p2_cmd, OP_EQ, RELAY_COMMAND_EXTEND2);
  tt_int_op(p2_len, OP_EQ, 35+NTOR_ONIONSKIN_LEN);
  tt_mem_op(p2,OP_EQ, p, RELAY_PAYLOAD_SIZE);

  /* Now let's do a fanciful EXTEND2 cell. */
  memset(&ec, 0xff, sizeof(ec));
  memset(p, 0, sizeof(p));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, 99);
  /* 4 items; one 18 244 0 1 61681 */
  memcpy(p, "\x04\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  /* One is a digest. */
  memcpy(p+9, "\x02\x14" "anthropomorphization", 22);
  /* One is an ipv6 address */
  memcpy(p+31, "\x01\x12\x20\x02\x00\x00\x00\x00\x00\x00"
               "\x00\x00\x00\x00\x00\xf0\xc5\x1e\x11\x12", 20);
  /* One is the Konami code. */
  memcpy(p+51, "\xf0\x20upupdowndownleftrightleftrightba", 34);
  /* Prep for the handshake: weird type and length */
  memcpy(p+85, "\x01\x05\x00\x63", 4);
  memcpy(p+89, b, 99);
  tt_int_op(0, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2, p, 89+99));
  tt_int_op(RELAY_COMMAND_EXTEND2, OP_EQ, ec.cell_type);
  tt_str_op("18.244.0.1", OP_EQ, fmt_addr(&ec.orport_ipv4.addr));
  tt_int_op(61681, OP_EQ, ec.orport_ipv4.port);
  tt_str_op("2002::f0:c51e", OP_EQ, fmt_addr(&ec.orport_ipv6.addr));
  tt_int_op(4370, OP_EQ, ec.orport_ipv6.port);
  tt_assert(ed25519_public_key_is_zero(&ec.ed_pubkey));
  tt_mem_op(ec.node_id,OP_EQ, "anthropomorphization", 20);
  tt_int_op(cc->cell_type, OP_EQ, CELL_CREATE2);
  tt_int_op(cc->handshake_type, OP_EQ, 0x105);
  tt_int_op(cc->handshake_len, OP_EQ, 99);
  tt_mem_op(cc->onionskin,OP_EQ, b, 99+20);
  tt_int_op(0, OP_EQ, extend_cell_format(&p2_cmd, &p2_len, p2, &ec));
  tt_int_op(p2_cmd, OP_EQ, RELAY_COMMAND_EXTEND2);
  /* We'll generate it minus the konami code */
  tt_int_op(p2_len, OP_EQ, 89+99-34);
  test_memeq_hex(p2,
                 /* Three items */
                 "03"
                 /* IPv4 address */
                 "0006" "12F40001" "F0F1"
                 /* The next is an RSA digest: anthropomorphization */
                 "0214" "616e7468726f706f6d6f727068697a6174696f6e"
                 /*IPv6 address */
                 "0112" "20020000000000000000000000f0c51e" "1112"
                 /* Now the handshake prologue */
                 "01050063");
  tt_mem_op(p2+1+8+22+20+4, OP_EQ, b, 99+20);
  tt_int_op(0, OP_EQ, create_cell_format_relayed(&cell, cc));

  /* Now let's add an ed25519 key to that extend2 cell. */
  memcpy(ec.ed_pubkey.pubkey,
         "brownshoesdontmakeit/brownshoesd", 32);

  /* As before, since we aren't extending by ed25519. */
  get_options_mutable()->ExtendByEd25519ID = 0;
  tt_int_op(0, OP_EQ, extend_cell_format(&p2_cmd, &p2_len, p2, &ec));
  tt_int_op(p2_len, OP_EQ, 89+99-34);
  test_memeq_hex(p2,
                 "03"
                 "000612F40001F0F1"
                 "0214616e7468726f706f6d6f727068697a6174696f6e"
                 "011220020000000000000000000000f0c51e1112"
                 "01050063");

  /* Now try with the ed25519 ID. */
  get_options_mutable()->ExtendByEd25519ID = 1;
  tt_int_op(0, OP_EQ, extend_cell_format(&p2_cmd, &p2_len, p2, &ec));
  tt_int_op(p2_len, OP_EQ, 89+99);
  test_memeq_hex(p2,
                 /* Four items */
                 "04"
                 /* IPv4 address */
                 "0006" "12F40001" "F0F1"
                 /* The next is an RSA digest: anthropomorphization */
                 "0214616e7468726f706f6d6f727068697a6174696f6e"
                 /* Then an ed public key: brownshoesdontmakeit/brownshoesd */
                 "0320" "62726f776e73686f6573646f6e746d616b656"
                        "9742f62726f776e73686f657364"
                 /*IPv6 address */
                 "0112" "20020000000000000000000000f0c51e" "1112"
                 /* Now the handshake prologue */
                 "01050063");
  /* Can we parse that? Did the key come through right? */
  memset(&ec, 0, sizeof(ec));
  tt_int_op(0, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                        p2, p2_len));
  tt_mem_op("brownshoesdontmakeit/brownshoesd", OP_EQ,
            ec.ed_pubkey.pubkey, 32);

  /* Now try IPv6 without IPv4 */
  memset(p, 0, sizeof(p));
  memcpy(p, "\x02", 1);
  memcpy(p+1, "\x02\x14" "anthropomorphization", 22);
  memcpy(p+23, "\x01\x12" "xxxxxxxxxxxxxxxxYY", 20);
  memcpy(p+43, "\xff\xff\x00\x20", 4);
  tt_int_op(0, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                         p, sizeof(p)));
  tt_int_op(RELAY_COMMAND_EXTEND2, OP_EQ, ec.cell_type);
  tt_assert(fast_mem_is_zero((const char *)&ec.orport_ipv4.addr,
                             sizeof(tor_addr_t)));
  tt_int_op(0, OP_EQ, ec.orport_ipv4.port);
  tt_str_op("7878:7878:7878:7878:7878:7878:7878:7878",
            OP_EQ, fmt_addr(&ec.orport_ipv6.addr));
  tt_int_op(22873, OP_EQ, ec.orport_ipv6.port);
  tt_assert(ed25519_public_key_is_zero(&ec.ed_pubkey));
  tt_mem_op(ec.node_id,OP_EQ, "anthropomorphization", 20);
  tt_int_op(cc->cell_type, OP_EQ, CELL_CREATE2);
  tt_int_op(cc->handshake_type, OP_EQ, 0xffff);
  tt_int_op(cc->handshake_len, OP_EQ, 32);
  tt_int_op(0, OP_EQ, extend_cell_format(&p2_cmd, &p2_len, p2, &ec));
  tt_int_op(p2_cmd, OP_EQ, RELAY_COMMAND_EXTEND2);
  tt_int_op(p2_len, OP_EQ, 47+32);
  test_memeq_hex(p2,
                 /* Two items */
                 "02"
                 /* The next is an RSA digest: anthropomorphization */
                 "0214" "616e7468726f706f6d6f727068697a6174696f6e"
                 /*IPv6 address */
                 "0112" "78787878787878787878787878787878" "5959"
                 /* Now the handshake prologue */
                 "ffff0020");
  tt_int_op(0, OP_EQ, create_cell_format_relayed(&cell, cc));

  /* == Now try parsing some junk */

  /* Try a too-long handshake */
  memset(p, 0, sizeof(p));
  memcpy(p, "\x02\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  memcpy(p+9, "\x02\x14" "anarchoindividualist", 22);
  memcpy(p+31, "\xff\xff\x01\xd0", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));

  /* Try two identities. */
  memset(p, 0, sizeof(p));
  memcpy(p, "\x03\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  memcpy(p+9, "\x02\x14" "anarchoindividualist", 22);
  memcpy(p+31, "\x02\x14" "autodepolymerization", 22);
  memcpy(p+53, "\xff\xff\x00\x10", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));

  /* No identities. */
  memset(p, 0, sizeof(p));
  memcpy(p, "\x01\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  memcpy(p+53, "\xff\xff\x00\x10", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));

  /* Try a bad IPv4 address (too long, too short)*/
  memset(p, 0, sizeof(p));
  memcpy(p, "\x02\x00\x07\x12\xf4\x00\x01\xf0\xf1\xff", 10);
  memcpy(p+10, "\x02\x14" "anarchoindividualist", 22);
  memcpy(p+32, "\xff\xff\x00\x10", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));
  memset(p, 0, sizeof(p));
  memcpy(p, "\x02\x00\x05\x12\xf4\x00\x01\xf0", 8);
  memcpy(p+8, "\x02\x14" "anarchoindividualist", 22);
  memcpy(p+30, "\xff\xff\x00\x10", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));

  /* IPv6 address (too long, too short, no IPv4)*/
  memset(p, 0, sizeof(p));
  memcpy(p, "\x03\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  memcpy(p+9, "\x02\x14" "anarchoindividualist", 22);
  memcpy(p+31, "\x01\x13" "xxxxxxxxxxxxxxxxYYZ", 19);
  memcpy(p+50, "\xff\xff\x00\x20", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));
  memset(p, 0, sizeof(p));
  memcpy(p, "\x03\x00\x06\x12\xf4\x00\x01\xf0\xf1", 9);
  memcpy(p+9, "\x02\x14" "anarchoindividualist", 22);
  memcpy(p+31, "\x01\x11" "xxxxxxxxxxxxxxxxY", 17);
  memcpy(p+48, "\xff\xff\x00\x20", 4);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));

  /* Running out of space in specifiers  */
  memset(p,0,sizeof(p));
  memcpy(p, "\x05\x0a\xff", 3);
  memcpy(p+3+255, "\x0a\xff", 2);
  tt_int_op(-1, OP_EQ, extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2,
                                      p, sizeof(p)));

  /* Fuzz, because why not. */
  memset(&ec, 0xff, sizeof(ec));
  {
    int i;
    memset(p, 0, sizeof(p));
    for (i = 0; i < 10000; ++i) {
      int n = crypto_rand_int(sizeof(p));
      crypto_rand((char *)p, n);
      extend_cell_parse(&ec, RELAY_COMMAND_EXTEND2, p, n);
    }
  }

 done:
  tor_free(mem_op_hex_tmp);
}

static void
test_cfmt_extended_cells(void *arg)
{
  uint8_t b[512];
  extended_cell_t ec;
  created_cell_t *cc = &ec.created_cell;
  uint8_t p[RELAY_PAYLOAD_SIZE];
  uint8_t p2[RELAY_PAYLOAD_SIZE];
  uint8_t p2_cmd;
  uint16_t p2_len;
  char *mem_op_hex_tmp = NULL;

  (void) arg;

  /* Try an EXTENDED2 cell */
  memset(&ec, 0xff, sizeof(ec));
  memset(p, 0, sizeof(p));
  memset(b, 0, sizeof(b));
  crypto_rand((char*)b, 42);
  memcpy(p,"\x00\x2a",2);
  memcpy(p+2,b,42);
  tt_int_op(0, OP_EQ,
            extended_cell_parse(&ec, RELAY_COMMAND_EXTENDED2, p, 2+42));
  tt_int_op(RELAY_COMMAND_EXTENDED2, OP_EQ, ec.cell_type);
  tt_int_op(cc->cell_type, OP_EQ, CELL_CREATED2);
  tt_int_op(cc->handshake_len, OP_EQ, 42);
  tt_mem_op(cc->reply,OP_EQ, b, 42+10);
  tt_int_op(0, OP_EQ, extended_cell_format(&p2_cmd, &p2_len, p2, &ec));
  tt_int_op(RELAY_COMMAND_EXTENDED2, OP_EQ, p2_cmd);
  tt_int_op(2+42, OP_EQ, p2_len);
  tt_mem_op(p2,OP_EQ, p, sizeof(p2));

  /* Try an almost-too-long EXTENDED2 cell */
  memcpy(p, "\x01\xf0", 2);
  tt_int_op(0, OP_EQ,
            extended_cell_parse(&ec, RELAY_COMMAND_EXTENDED2, p, sizeof(p)));

  /* Now try a too-long extended2 cell. That's the only misparse I can think
   * of. */
  memcpy(p, "\x01\xf1", 2);
  tt_int_op(-1, OP_EQ,
            extended_cell_parse(&ec, RELAY_COMMAND_EXTENDED2, p, sizeof(p)));

 done:
  tor_free(mem_op_hex_tmp);
}

static void
test_cfmt_resolved_cells(void *arg)
{
  smartlist_t *addrs = smartlist_new();
  int r, errcode;
  address_ttl_t *a;
  relay_msg_t msg;
  uint8_t buf[500];

  (void)arg;
#define CLEAR_CELL() do {                       \
    memset(&msg, 0, sizeof(msg));               \
    memset(&buf, 0, sizeof(buf));               \
  } while (0)
#define CLEAR_ADDRS() do {                              \
    SMARTLIST_FOREACH(addrs, address_ttl_t *, aa_,      \
                      address_ttl_free(aa_); );         \
    smartlist_clear(addrs);                             \
  } while (0)
#define SET_CELL(s) do {                                                \
    CLEAR_CELL();                                                       \
    memcpy(buf, (s), sizeof((s))-1);                                    \
    msg.length = sizeof((s))-1;                                         \
    msg.body = buf;                                                     \
    msg.command = RELAY_COMMAND_RESOLVED;                               \
    errcode = -1;                                                       \
  } while (0)

  /* The cell format is one or more answers; each of the form
   *  type [1 byte---0:hostname, 4:ipv4, 6:ipv6, f0:err-transient, f1:err]
   *  length [1 byte]
   *  body [length bytes]
   *  ttl  [4 bytes]
   */

  /* Let's try an empty cell */
  SET_CELL("");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  CLEAR_ADDRS(); /* redundant but let's be consistent */

  /* Cell with one ipv4 addr */
  SET_CELL("\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00");
  tt_int_op(msg.length, OP_EQ, 10);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 1);
  a = smartlist_get(addrs, 0);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "127.0.2.10");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 256);
  CLEAR_ADDRS();

  /* Cell with one ipv6 addr */
  SET_CELL("\x06\x10"
           "\x20\x02\x90\x90\x00\x00\x00\x00"
           "\x00\x00\x00\x00\xf0\xf0\xab\xcd"
           "\x02\00\x00\x01");
  tt_int_op(msg.length, OP_EQ, 22);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 1);
  a = smartlist_get(addrs, 0);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "2002:9090::f0f0:abcd");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 0x2000001);
  CLEAR_ADDRS();

  /* Cell with one hostname */
  SET_CELL("\x00\x11"
           "motherbrain.zebes"
           "\x00\00\x00\x00");
  tt_int_op(msg.length, OP_EQ, 23);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 1);
  a = smartlist_get(addrs, 0);
  tt_assert(tor_addr_is_null(&a->addr));
  tt_str_op(a->hostname, OP_EQ, "motherbrain.zebes");
  tt_int_op(a->ttl, OP_EQ, 0);
  CLEAR_ADDRS();

#define LONG_NAME \
  "this-hostname-has-255-characters.in-order-to-test-whether-very-long.ho" \
  "stnames-are-accepted.i-am-putting-it-in-a-macro-because-although.this-" \
  "function-is-already-very-full.of-copy-and-pasted-stuff.having-this-app" \
  "ear-more-than-once-would-bother-me-somehow.is"

  tt_int_op(strlen(LONG_NAME), OP_EQ, 255);
  SET_CELL("\x00\xff"
           LONG_NAME
           "\x00\01\x00\x00");
  tt_int_op(msg.length, OP_EQ, 261);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 1);
  a = smartlist_get(addrs, 0);
  tt_assert(tor_addr_is_null(&a->addr));
  tt_str_op(a->hostname, OP_EQ, LONG_NAME);
  tt_int_op(a->ttl, OP_EQ, 65536);
  CLEAR_ADDRS();

  /* Cells with an error */
  SET_CELL("\xf0\x2b"
           "I'm sorry, Dave. I'm afraid I can't do that"
           "\x00\x11\x22\x33");
  tt_int_op(msg.length, OP_EQ, 49);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, RESOLVED_TYPE_ERROR_TRANSIENT);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  CLEAR_ADDRS();

  SET_CELL("\xf1\x40"
           "This hostname is too important for me to allow you to resolve it"
           "\x00\x00\x00\x00");
  tt_int_op(msg.length, OP_EQ, 70);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, RESOLVED_TYPE_ERROR);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  CLEAR_ADDRS();

  /* Cell with an unrecognized type */
  SET_CELL("\xee\x16"
           "fault in the AE35 unit"
           "\x09\x09\x01\x01");
  tt_int_op(msg.length, OP_EQ, 28);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  CLEAR_ADDRS();

  /* Cell with one of each */
  SET_CELL(/* unrecognized: */
           "\xee\x16"
           "fault in the AE35 unit"
           "\x09\x09\x01\x01"
           /* error: */
           "\xf0\x2b"
           "I'm sorry, Dave. I'm afraid I can't do that"
           "\x00\x11\x22\x33"
           /* IPv6: */
           "\x06\x10"
           "\x20\x02\x90\x90\x00\x00\x00\x00"
           "\x00\x00\x00\x00\xf0\xf0\xab\xcd"
           "\x02\00\x00\x01"
           /* IPv4: */
           "\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00"
           /* Hostname: */
           "\x00\x11"
           "motherbrain.zebes"
           "\x00\00\x00\x00"
           );
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0); /* no error reported; we got answers */
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 3);
  a = smartlist_get(addrs, 0);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "2002:9090::f0f0:abcd");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 0x2000001);
  a = smartlist_get(addrs, 1);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "127.0.2.10");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 256);
  a = smartlist_get(addrs, 2);
  tt_assert(tor_addr_is_null(&a->addr));
  tt_str_op(a->hostname, OP_EQ, "motherbrain.zebes");
  tt_int_op(a->ttl, OP_EQ, 0);
  CLEAR_ADDRS();

  /* Cell with several of similar type */
  SET_CELL(/* IPv4 */
           "\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00"
           "\x04\x04" "\x08\x08\x08\x08" "\x00\00\x01\x05"
           "\x04\x04" "\x7f\xb0\x02\xb0" "\x00\01\xff\xff"
           /* IPv6 */
           "\x06\x10"
           "\x20\x02\x90\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\xca\xfe\xf0\x0d"
           "\x00\00\x00\x01"
           "\x06\x10"
           "\x20\x02\x90\x01\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\xfa\xca\xde"
           "\x00\00\x00\x03");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 5);
  a = smartlist_get(addrs, 0);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "127.0.2.10");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 256);
  a = smartlist_get(addrs, 1);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "8.8.8.8");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 261);
  a = smartlist_get(addrs, 2);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "127.176.2.176");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 131071);
  a = smartlist_get(addrs, 3);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "2002:9000::cafe:f00d");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 1);
  a = smartlist_get(addrs, 4);
  tt_str_op(fmt_addr(&a->addr), OP_EQ, "2002:9001::fa:cade");
  tt_ptr_op(a->hostname, OP_EQ, NULL);
  tt_int_op(a->ttl, OP_EQ, 3);
  CLEAR_ADDRS();

  /* Full cell */
#define LONG_NAME2 \
  "this-name-has-231-characters.so-that-it-plus-LONG_NAME-can-completely-" \
  "fill-up-the-payload-of-a-cell.its-important-to-check-for-the-full-thin" \
  "g-case.to-avoid-off-by-one-errors.where-full-things-are-misreported-as" \
  ".overflowing-by-one.z"

  tt_int_op(strlen(LONG_NAME2), OP_EQ, 231);
  SET_CELL("\x00\xff"
           LONG_NAME
           "\x00\01\x00\x00"
           "\x00\xe7"
           LONG_NAME2
           "\x00\01\x00\x00");
  tt_int_op(msg.length, OP_EQ, RELAY_PAYLOAD_SIZE);
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(addrs), OP_EQ, 2);
  a = smartlist_get(addrs, 0);
  tt_str_op(a->hostname, OP_EQ, LONG_NAME);
  a = smartlist_get(addrs, 1);
  tt_str_op(a->hostname, OP_EQ, LONG_NAME2);
  CLEAR_ADDRS();

  /* BAD CELLS */

  /* Invalid length on an IPv4 */
  SET_CELL("\x04\x03zzz1234");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  SET_CELL("\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00"
           "\x04\x05zzzzz1234");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

  /* Invalid length on an IPv6 */
  SET_CELL("\x06\x03zzz1234");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  SET_CELL("\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00"
           "\x06\x17wwwwwwwwwwwwwwwww1234");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  SET_CELL("\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00"
           "\x06\x10xxxx");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

  /* Empty hostname */
  SET_CELL("\x00\x00xxxx");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

#if 0
  //No longer possible with relay message encoding.
  /* rh.length out of range */
  CLEAR_CELL();
  rh.length = 499;
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(errcode, OP_EQ, 0);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
#endif

  /* Item length extends beyond rh.length */
  CLEAR_CELL();
  SET_CELL("\x00\xff"
           LONG_NAME
           "\x00\01\x00\x00");
  msg.length -= 1;
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);
  msg.length -= 5;
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

  SET_CELL("\x04\x04" "\x7f\x00\x02\x0a" "\x00\00\x01\x00");
  msg.length -= 1;
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

  SET_CELL("\xee\x10"
           "\x20\x02\x90\x01\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\xfa\xca\xde"
           "\x00\00\x00\x03");
  msg.length -= 1;
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

  /* Truncated item after first character */
  SET_CELL("\x04");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

  SET_CELL("\xee");
  r = resolved_cell_parse(&msg, addrs, &errcode);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(smartlist_len(addrs), OP_EQ, 0);

 done:
  CLEAR_ADDRS();
  CLEAR_CELL();
  smartlist_free(addrs);
#undef CLEAR_ADDRS
#undef CLEAR_CELL
}

static void
test_cfmt_is_destroy(void *arg)
{
  cell_t cell;
  packed_cell_t packed;
  circid_t circid = 0;
  channel_t *chan;
  (void)arg;

  chan = tor_malloc_zero(sizeof(channel_t));

  memset(&cell, 0xff, sizeof(cell));
  cell.circ_id = 3003;
  cell.command = CELL_RELAY;

  cell_pack(&packed, &cell, 0);
  chan->wide_circ_ids = 0;
  tt_assert(! packed_cell_is_destroy(chan, &packed, &circid));
  tt_int_op(circid, OP_EQ, 0);

  cell_pack(&packed, &cell, 1);
  chan->wide_circ_ids = 1;
  tt_assert(! packed_cell_is_destroy(chan, &packed, &circid));
  tt_int_op(circid, OP_EQ, 0);

  cell.command = CELL_DESTROY;

  cell_pack(&packed, &cell, 0);
  chan->wide_circ_ids = 0;
  tt_assert(packed_cell_is_destroy(chan, &packed, &circid));
  tt_int_op(circid, OP_EQ, 3003);

  circid = 0;
  cell_pack(&packed, &cell, 1);
  chan->wide_circ_ids = 1;
  tt_assert(packed_cell_is_destroy(chan, &packed, &circid));

 done:
  tor_free(chan);
}

static void
test_cfmt_relay_msg_encoding_simple(void *arg)
{
  (void)arg;
  relay_msg_t *msg1 = NULL;
  cell_t cell;
  char *mem_op_hex_tmp = NULL;
  int r;
  uint8_t body[100];

  /* Simple message: Data, fits easily in cell. */
  msg1 = tor_malloc_zero(sizeof(relay_msg_t));
  msg1->command = RELAY_COMMAND_DATA;
  msg1->stream_id = 0x250;
  msg1->length = 11;
  msg1->body = body;
  strlcpy((char*)body, "hello world", sizeof(body));

  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V0, msg1, &cell);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(cell.command, OP_EQ, CELL_RELAY);
  tt_int_op(cell.circ_id, OP_EQ, 0);
  // command, recognized, streamid, digest, len, payload, zero-padding.
  test_memeq_hex(cell.payload,
                 "02" "0000" "0250" "00000000" "000B"
                 "68656c6c6f20776f726c64" "00000000");
  // random padding
  size_t used = RELAY_HEADER_SIZE_V0 + 11 + 4;
  tt_assert(!fast_mem_is_zero((char*)cell.payload + used,
                              CELL_PAYLOAD_SIZE - used));

  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V1, msg1, &cell);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(cell.command, OP_EQ, CELL_RELAY);
  tt_int_op(cell.circ_id, OP_EQ, 0);
  // tag, command, len, optional streamid, payload, zero-padding
  test_memeq_hex(cell.payload,
                 "00000000000000000000000000000000"
                 "02" "000B" "0250"
                 "68656c6c6f20776f726c64" "00000000");
  // random padding.
  used = RELAY_HEADER_SIZE_V1_WITH_STREAM_ID + 11 + 4;
  tt_assert(!fast_mem_is_zero((char*)cell.payload + used,
                              CELL_PAYLOAD_SIZE - used));

  /* Message without stream ID: SENDME, fits easily in cell. */
  relay_msg_clear(msg1);
  msg1->command = RELAY_COMMAND_SENDME;
  msg1->stream_id = 0;
  msg1->length = 20;
  msg1->body = body;
  strlcpy((char *)body, "hello i am a tag....", sizeof(body));

  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V0, msg1, &cell);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(cell.command, OP_EQ, CELL_RELAY);
  tt_int_op(cell.circ_id, OP_EQ, 0);
  // command, recognized, streamid, digest, len, payload, zero-padding.
  test_memeq_hex(cell.payload,
                 "05" "0000" "0000" "00000000" "0014"
                 "68656c6c6f206920616d2061207461672e2e2e2e" "00000000");
  // random padding
  used = RELAY_HEADER_SIZE_V0 + 20 + 4;
  tt_assert(!fast_mem_is_zero((char*)cell.payload + used,
                              CELL_PAYLOAD_SIZE - used));

  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V1, msg1, &cell);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(cell.command, OP_EQ, CELL_RELAY);
  tt_int_op(cell.circ_id, OP_EQ, 0);
  // tag, command, len, optional streamid, payload, zero-padding
  test_memeq_hex(cell.payload,
                 "00000000000000000000000000000000"
                 "05" "0014"
                 "68656c6c6f206920616d2061207461672e2e2e2e" "00000000");
  // random padding.
  used = RELAY_HEADER_SIZE_V1_NO_STREAM_ID + 20 + 4;
  tt_assert(!fast_mem_is_zero((char*)cell.payload + used,
                              CELL_PAYLOAD_SIZE - used));

 done:
  relay_msg_free(msg1);
  tor_free(mem_op_hex_tmp);
}

/** Helper for test_cfmt_relay_cell_padding.
 * Requires that that the body of 'msg' ends with 'pre_padding_byte',
 * and that  when encoded, the zero-padding (if any) will appear at
 * offset 'zeros_begin_at' in the message.
 */
static void
msg_encoder_padding_test(const relay_msg_t *msg,
                         relay_cell_fmt_t fmt,
                         uint8_t pre_padding_byte,
                         size_t zeros_begin_at)
{
  cell_t cell;
  int n = 16, i;
  /* We set this to 0 as soon as we find that the first byte of
   * random padding has been set. */
  bool padded_first = false;
  /* We set this to true as soon as we find that the last byte of
   * random padding has been set */
  bool padded_last = false;

  tt_int_op(zeros_begin_at, OP_LE, CELL_PAYLOAD_SIZE);

  size_t expect_n_zeros = MIN(4, CELL_PAYLOAD_SIZE - zeros_begin_at);
  ssize_t first_random_at = -1;
  if (CELL_PAYLOAD_SIZE - zeros_begin_at > 4) {
    first_random_at = CELL_PAYLOAD_SIZE - zeros_begin_at + 4;
  }

  for (i = 0; i < n; ++i) {
    memset(&cell, 0, sizeof(cell));
    tt_int_op(0, OP_EQ,
              relay_msg_encode_cell(fmt, msg, &cell));

    const uint8_t *body = cell.payload;
    tt_int_op(body[zeros_begin_at - 1], OP_EQ, pre_padding_byte);

    if (expect_n_zeros) {
      tt_assert(fast_mem_is_zero((char*)body + zeros_begin_at,
                                 expect_n_zeros));
    }
    if (first_random_at >= 0) {
      if (body[first_random_at])
        padded_first = true;
      if (body[CELL_PAYLOAD_SIZE-1])
        padded_last = true;
    }
  }

  if (first_random_at >= 0)  {
    tt_assert(padded_first);
    tt_assert(padded_last);
  }

 done:
  ;
}

static void
test_cfmt_relay_cell_padding(void *arg)
{
  (void)arg;
  relay_msg_t *msg1 = NULL;
  uint8_t buf[500]; // Longer than it needs to be.
  memset(buf, 0xff, sizeof(buf));

  /* Simple message; we'll adjust the length and encode it. */
  msg1 = tor_malloc_zero(sizeof(relay_msg_t));
  msg1->command = RELAY_COMMAND_DATA;
  msg1->stream_id = 0x250;
  msg1->body = buf;

  // Empty message
  msg1->length = 0;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V0, 0x00,
                           RELAY_HEADER_SIZE_V0);
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0x50,
                            RELAY_HEADER_SIZE_V1_WITH_STREAM_ID);

  // Short message
  msg1->length = 10;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V0, 0xff,
                            RELAY_HEADER_SIZE_V0 + 10);
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                            RELAY_HEADER_SIZE_V1_WITH_STREAM_ID + 10);

  // Message where zeros extend exactly up to the end of the cell.
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V0 - 4;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V0, 0xff,
                           RELAY_HEADER_SIZE_V0 + msg1->length);
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_WITH_STREAM_ID - 4;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                           RELAY_HEADER_SIZE_V1_WITH_STREAM_ID + msg1->length);

  // Message where zeros would intersect with the end of the cell.
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V0 - 3;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V0, 0xff,
                           RELAY_HEADER_SIZE_V0 + msg1->length);
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_WITH_STREAM_ID - 3;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                           RELAY_HEADER_SIZE_V1_WITH_STREAM_ID + msg1->length);

  // Message with no room for zeros
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V0;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V0, 0xff,
                           RELAY_HEADER_SIZE_V0 + msg1->length);
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_WITH_STREAM_ID;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                           RELAY_HEADER_SIZE_V1_WITH_STREAM_ID + msg1->length);

  ///////////////
  // V1 cases with no stream ID.
  msg1->stream_id = 0;
  msg1->command = RELAY_COMMAND_EXTENDED;

  msg1->length = 0;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0x00,
                            RELAY_HEADER_SIZE_V1_NO_STREAM_ID);
  msg1->length = 10;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                            RELAY_HEADER_SIZE_V1_NO_STREAM_ID + 10);
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_NO_STREAM_ID - 4;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                           RELAY_HEADER_SIZE_V1_NO_STREAM_ID + msg1->length);
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_NO_STREAM_ID - 3;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                           RELAY_HEADER_SIZE_V1_NO_STREAM_ID + msg1->length);
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_NO_STREAM_ID;
  msg_encoder_padding_test(msg1, RELAY_CELL_FORMAT_V1, 0xff,
                           RELAY_HEADER_SIZE_V1_NO_STREAM_ID + msg1->length);

  relay_msg_free(msg1);
}

static void
test_cfmt_relay_msg_encoding_error(void *arg)
{
  (void)arg;
#ifdef ALL_BUGS_ARE_FATAL
  // This test triggers many nonfatal assertions.
  tt_skip();
 done:
  ;
#else
  relay_msg_t *msg1 = NULL;
  int r;
  cell_t cell;
  uint8_t buf[500]; // Longer than it needs to be.
  memset(buf, 0xff, sizeof(buf));

  msg1 = tor_malloc_zero(sizeof(relay_msg_t));
  msg1->command = RELAY_COMMAND_DATA;
  msg1->stream_id = 0x250;
  msg1->body = buf;

  tor_capture_bugs_(5);
  // Too long for v0.
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V0 + 1;
  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V0, msg1, &cell);
  tt_int_op(r, OP_EQ, -1);

  // Too long for v1, with stream ID.
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_WITH_STREAM_ID + 1;
  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V1, msg1, &cell);
  tt_int_op(r, OP_EQ, -1);

  // Too long for v1 with no stream ID.
  msg1->command = RELAY_COMMAND_EXTENDED;
  msg1->stream_id = 0;
  msg1->length = CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_NO_STREAM_ID + 1;
  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V1, msg1, &cell);
  tt_int_op(r, OP_EQ, -1);

  // Invalid (present) stream ID for V1.
  msg1->stream_id = 10;
  msg1->length = 20;
  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V1, msg1, &cell);
  tt_int_op(r, OP_EQ, -1);

  // Invalid (absent) stream ID for V1.
  msg1->stream_id = 0;
  msg1->command = RELAY_COMMAND_DATA;
  r = relay_msg_encode_cell(RELAY_CELL_FORMAT_V1, msg1, &cell);
  tt_int_op(r, OP_EQ, -1);

 done:
  tor_end_capture_bugs_();
  relay_msg_free(msg1);
#endif
}

static void
test_cfmt_relay_msg_decoding_simple(void *arg)
{
  (void) arg;
  cell_t cell;
  relay_msg_t *msg1 = NULL;
  const char *s;

  memset(&cell, 0, sizeof(cell));
  cell.command = CELL_RELAY;

  // V0 decoding, short message.
  s = "02" "0000" "0250" "00000000" "000B"
      "68656c6c6f20776f726c64" "00000000";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));
  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V0, &cell);
  tt_assert(msg1);

  tt_int_op(msg1->command, OP_EQ, RELAY_COMMAND_DATA);
  tt_int_op(msg1->stream_id, OP_EQ, 0x250);
  tt_int_op(msg1->length, OP_EQ, 11);
  tt_mem_op(msg1->body, OP_EQ, "hello world", 11);
  relay_msg_free(msg1);

  // V0 decoding, message up to length of cell.
  memset(cell.payload, 0, sizeof(cell.payload));
  s = "02" "0000" "0250" "00000000" "01F2";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));
  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V0, &cell);
  tt_assert(msg1);

  tt_int_op(msg1->command, OP_EQ, RELAY_COMMAND_DATA);
  tt_int_op(msg1->stream_id, OP_EQ, 0x250);
  tt_int_op(msg1->length, OP_EQ, 498);
  tt_assert(fast_mem_is_zero((char*)msg1->body, 498));
  relay_msg_free(msg1);

  // V1 decoding, short message, no stream ID.
  s = "00000000000000000000000000000000"
      "05" "0014"
      "68656c6c6f206920616d2061207461672e2e2e2e" "00000000";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));

  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_assert(msg1);
  tt_int_op(msg1->command, OP_EQ, RELAY_COMMAND_SENDME);
  tt_int_op(msg1->stream_id, OP_EQ, 0);
  tt_int_op(msg1->length, OP_EQ, 20);
  tt_mem_op(msg1->body, OP_EQ, "hello i am a tag....", 20);
  relay_msg_free(msg1);

  // V1 decoding, up to length of cell, no stream ID.
  memset(cell.payload, 0, sizeof(cell.payload));
  s = "00000000000000000000000000000000"
      "05" "01EA";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));

  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_assert(msg1);
  tt_int_op(msg1->command, OP_EQ, RELAY_COMMAND_SENDME);
  tt_int_op(msg1->stream_id, OP_EQ, 0);
  tt_int_op(msg1->length, OP_EQ, 490);
  tt_assert(fast_mem_is_zero((char*)msg1->body, 490));
  relay_msg_free(msg1);

  // V1 decoding, short message, with stream ID.
  s = "00000000000000000000000000000000"
      "02" "000B" "0250"
      "68656c6c6f20776f726c64" "00000000";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));

  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_assert(msg1);
  tt_int_op(msg1->command, OP_EQ, RELAY_COMMAND_DATA);
  tt_int_op(msg1->stream_id, OP_EQ, 0x250);
  tt_int_op(msg1->length, OP_EQ, 11);
  tt_mem_op(msg1->body, OP_EQ, "hello world", 11);
  relay_msg_free(msg1);

  // V1 decoding, up to length of cell, with stream ID.
  memset(cell.payload, 0, sizeof(cell.payload));
  s = "00000000000000000000000000000000"
      "02" "01E8" "0250";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));

  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_assert(msg1);
  tt_int_op(msg1->command, OP_EQ, RELAY_COMMAND_DATA);
  tt_int_op(msg1->stream_id, OP_EQ, 0x250);
  tt_int_op(msg1->length, OP_EQ, 488);
  tt_assert(fast_mem_is_zero((char*)msg1->body, 488));
  relay_msg_free(msg1);

 done:
  relay_msg_free(msg1);
}

static void
test_cfmt_relay_msg_decoding_error(void *arg)
{
  (void) arg;
  relay_msg_t *msg1 = NULL;
  cell_t cell;
  const char *s;
  memset(&cell, 0, sizeof(cell));

  // V0, too long.
  cell.command = CELL_RELAY;
  s = "02" "0000" "0250" "00000000" "01F3";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));
  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V0, &cell);
  tt_ptr_op(msg1, OP_EQ, NULL);

  // V1, command unrecognized.
  s = "00000000000000000000000000000000"
      "F0" "000C" "0250";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));
  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_ptr_op(msg1, OP_EQ, NULL);

  // V1, too long (with stream ID)
  s = "00000000000000000000000000000000"
      "02" "01E9" "0250";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));
  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_ptr_op(msg1, OP_EQ, NULL);

  // V1, too long (without stream ID)
  s = "00000000000000000000000000000000"
      "05" "01EB";
  base16_decode((char*)cell.payload, sizeof(cell.payload), s, strlen(s));
  msg1 = relay_msg_decode_cell(RELAY_CELL_FORMAT_V1, &cell);
  tt_ptr_op(msg1, OP_EQ, NULL);

 done:
  relay_msg_free(msg1);
}

#define TEST(name, flags)                                               \
  { #name, test_cfmt_ ## name, flags, 0, NULL }

struct testcase_t cell_format_tests[] = {
  TEST(relay_header, 0),
  TEST(begin_cells, 0),
  TEST(connected_cells, 0),
  TEST(create_cells, 0),
  TEST(created_cells, 0),
  TEST(extend_cells, TT_FORK),
  TEST(extended_cells, 0),
  TEST(resolved_cells, 0),
  TEST(is_destroy, 0),
  TEST(relay_msg_encoding_simple, 0),
  TEST(relay_cell_padding, 0),
  TEST(relay_msg_encoding_error, 0),
  TEST(relay_msg_decoding_simple, 0),
  TEST(relay_msg_decoding_error, 0),
  END_OF_TESTCASES
};
