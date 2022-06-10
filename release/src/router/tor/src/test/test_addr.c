/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define ADDRESSMAP_PRIVATE
#include "orconfig.h"
#include "core/or/or.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "test/test.h"
#include "feature/client/addressmap.h"
#include "test/log_test_helpers.h"
#include "lib/net/resolve.h"
#include "test/rng_test_helpers.h"
#include "test/resolve_test_helpers.h"

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

static void
test_addr_basic(void *arg)
{
  (void) arg;

  tt_int_op(0,OP_EQ, addr_mask_get_bits(0x0u));
  tt_int_op(32,OP_EQ, addr_mask_get_bits(0xFFFFFFFFu));
  tt_int_op(16,OP_EQ, addr_mask_get_bits(0xFFFF0000u));
  tt_int_op(31,OP_EQ, addr_mask_get_bits(0xFFFFFFFEu));
  tt_int_op(1,OP_EQ, addr_mask_get_bits(0x80000000u));

  /* Test inet_ntop */
  {
    char tmpbuf[TOR_ADDR_BUF_LEN];
    const char *ip = "176.192.208.224";
    struct in_addr in;

    /* good round trip */
    tt_int_op(tor_inet_pton(AF_INET, ip, &in), OP_EQ, 1);
    tt_ptr_op(tor_inet_ntop(AF_INET, &in, tmpbuf, sizeof(tmpbuf)),
              OP_EQ, &tmpbuf);
    tt_str_op(tmpbuf,OP_EQ, ip);

    /* just enough buffer length */
    tt_str_op(tor_inet_ntop(AF_INET, &in, tmpbuf, strlen(ip) + 1), OP_EQ, ip);

    /* too short buffer */
    tt_ptr_op(tor_inet_ntop(AF_INET, &in, tmpbuf, strlen(ip)),OP_EQ, NULL);
  }

 done:
  ;
}

#ifndef COCCI
#define test_op_ip6_(a,op,b,e1,e2)                               \
  STMT_BEGIN                                                     \
  tt_assert_test_fmt_type(a,b,e1" "#op" "e2,struct in6_addr*,    \
    (fast_memcmp(val1_->s6_addr, val2_->s6_addr, 16) op 0),      \
    char *, "%s",                                                \
    { char *cp;                                                  \
      cp = print_ = tor_malloc(64);                              \
      for (int ii_=0;ii_<16;++ii_) {                             \
        tor_snprintf(cp, 3,"%02x", (unsigned)value_->s6_addr[ii_]);     \
        cp += 2;                                                 \
        if (ii_ != 15) *cp++ = ':';                              \
      }                                                          \
    },                                                           \
    { tor_free(print_); },                                       \
    TT_EXIT_TEST_FUNCTION                                        \
  );                                                             \
  STMT_END
#endif /* !defined(COCCI) */

/** Helper: Assert that two strings both decode as IPv6 addresses with
 * tor_inet_pton(), and both decode to the same address. */
#define test_pton6_same(a,b) STMT_BEGIN                 \
     tt_int_op(tor_inet_pton(AF_INET6, a, &a1), OP_EQ, 1); \
     tt_int_op(tor_inet_pton(AF_INET6, b, &a2), OP_EQ, 1); \
     test_op_ip6_(&a1,OP_EQ,&a2,#a,#b);                    \
  STMT_END

/** Helper: Assert that <b>a</b> is recognized as a bad IPv6 address by
 * tor_inet_pton(). */
#define test_pton6_bad(a)                       \
  tt_int_op(0, OP_EQ, tor_inet_pton(AF_INET6, a, &a1))

/** Helper: assert that <b>a</b>, when parsed by tor_inet_pton() and displayed
 * with tor_inet_ntop(), yields <b>b</b>. Also assert that <b>b</b> parses to
 * the same value as <b>a</b>. */
#define test_ntop6_reduces(a,b) STMT_BEGIN                          \
  tt_int_op(tor_inet_pton(AF_INET6, a, &a1), OP_EQ, 1);                \
  tt_str_op(tor_inet_ntop(AF_INET6, &a1, buf, sizeof(buf)), OP_EQ, b); \
  tt_int_op(tor_inet_pton(AF_INET6, b, &a2), OP_EQ, 1);     \
  test_op_ip6_(&a1, OP_EQ, &a2, a, b);                      \
  STMT_END

/** Helper: assert that <b>a</b> parses by tor_inet_pton() into a address that
 * passes tor_addr_is_internal() with <b>for_listening</b>. */
#define test_internal_ip(a,for_listening) STMT_BEGIN           \
    tt_int_op(tor_inet_pton(AF_INET6, a, &t1.addr.in6_addr), OP_EQ, 1); \
    t1.family = AF_INET6;                                      \
    if (!tor_addr_is_internal(&t1, for_listening))             \
      TT_DIE(("%s was not internal", a));                      \
  STMT_END

/** Helper: assert that <b>a</b> parses by tor_inet_pton() into a address that
 * does not pass tor_addr_is_internal() with <b>for_listening</b>. */
#define test_external_ip(a,for_listening) STMT_BEGIN           \
    tt_int_op(tor_inet_pton(AF_INET6, a, &t1.addr.in6_addr), OP_EQ, 1); \
    t1.family = AF_INET6;                                      \
    if (tor_addr_is_internal(&t1, for_listening))              \
      TT_DIE(("%s was internal", a));                      \
  STMT_END

#ifndef COCCI
/** Helper: Assert that <b>a</b> and <b>b</b>, when parsed by
 * tor_inet_pton(), give addresses that compare in the order defined by
 * <b>op</b> with tor_addr_compare(). */
#define test_addr_compare(a, op, b) STMT_BEGIN                    \
    tt_int_op(tor_inet_pton(AF_INET6, a, &t1.addr.in6_addr), OP_EQ, 1); \
    tt_int_op(tor_inet_pton(AF_INET6, b, &t2.addr.in6_addr), OP_EQ, 1); \
    t1.family = t2.family = AF_INET6;                             \
    r = tor_addr_compare(&t1,&t2,CMP_SEMANTIC);                   \
    if (!(r op 0))                                                \
      TT_DIE(("Failed: tor_addr_compare(%s,%s) %s 0", a, b, #op));\
  STMT_END

/** Helper: Assert that <b>a</b> and <b>b</b>, when parsed by
 * tor_inet_pton(), give addresses that compare in the order defined by
 * <b>op</b> with tor_addr_compare_masked() with <b>m</b> masked. */
#define test_addr_compare_masked(a, op, b, m) STMT_BEGIN          \
    tt_int_op(tor_inet_pton(AF_INET6, a, &t1.addr.in6_addr), OP_EQ, 1);    \
    tt_int_op(tor_inet_pton(AF_INET6, b, &t2.addr.in6_addr), OP_EQ, 1);    \
    t1.family = t2.family = AF_INET6;                             \
    r = tor_addr_compare_masked(&t1,&t2,m,CMP_SEMANTIC);          \
    if (!(r op 0))                                                \
      TT_DIE(("Failed: tor_addr_compare_masked(%s,%s,%d) %s 0", \
              a, b, m, #op));                                   \
  STMT_END
#endif /* !defined(COCCI) */

/** Helper: assert that <b>xx</b> is parseable as a masked IPv6 address with
 * ports by tor_parse_mask_addr_ports(), with family <b>f</b>, IP address
 * as 4 32-bit words <b>ip1...ip4</b>, mask bits as <b>mm</b>, and port range
 * as <b>pt1..pt2</b>. */
#define test_addr_mask_ports_parse(xx, f, ip1, ip2, ip3, ip4, mm, pt1, pt2) \
  STMT_BEGIN                                                                \
    tt_int_op(tor_addr_parse_mask_ports(xx, 0, &t1, &mask, &port1, &port2),   \
              OP_EQ, f);                                                   \
    p1=tor_inet_ntop(AF_INET6, &t1.addr.in6_addr, bug, sizeof(bug));        \
    tt_int_op(htonl(ip1), OP_EQ, tor_addr_to_in6_addr32(&t1)[0]);            \
    tt_int_op(htonl(ip2), OP_EQ, tor_addr_to_in6_addr32(&t1)[1]);            \
    tt_int_op(htonl(ip3), OP_EQ, tor_addr_to_in6_addr32(&t1)[2]);            \
    tt_int_op(htonl(ip4), OP_EQ, tor_addr_to_in6_addr32(&t1)[3]);            \
    tt_int_op(mask, OP_EQ, mm);                     \
    tt_uint_op(port1, OP_EQ, pt1);                  \
    tt_uint_op(port2, OP_EQ, pt2);                  \
  STMT_END

/** Run unit tests for IPv6 encoding/decoding/manipulation functions. */
static void
test_addr_ip6_helpers(void *arg)
{
  char buf[TOR_ADDR_BUF_LEN], bug[TOR_ADDR_BUF_LEN];
  char rbuf[REVERSE_LOOKUP_NAME_BUF_LEN];
  struct in6_addr a1, a2;
  tor_addr_t t1, t2;
  int r, i;
  uint16_t port1, port2;
  maskbits_t mask;
  const char *p1;
  struct sockaddr_storage sa_storage;
  struct sockaddr_in *sin;
  struct sockaddr_in6 *sin6;

  /* Test tor_inet_ntop and tor_inet_pton: IPv6 */
  (void)arg;
  {
    const char *ip = "2001::1234";
    const char *ip_ffff = "::ffff:192.168.1.2";

    /* good round trip */
    tt_int_op(tor_inet_pton(AF_INET6, ip, &a1),OP_EQ, 1);
    tt_ptr_op(tor_inet_ntop(AF_INET6, &a1, buf, sizeof(buf)),OP_EQ, &buf);
    tt_str_op(buf,OP_EQ, ip);

    /* good round trip - ::ffff:0:0 style */
    tt_int_op(tor_inet_pton(AF_INET6, ip_ffff, &a2),OP_EQ, 1);
    tt_ptr_op(tor_inet_ntop(AF_INET6, &a2, buf, sizeof(buf)),OP_EQ, &buf);
    tt_str_op(buf,OP_EQ, ip_ffff);

    /* just long enough buffer (remember \0) */
    tt_str_op(tor_inet_ntop(AF_INET6, &a1, buf, strlen(ip)+1),OP_EQ, ip);
    tt_str_op(tor_inet_ntop(AF_INET6, &a2, buf, strlen(ip_ffff)+1),OP_EQ,
               ip_ffff);

    /* too short buffer (remember \0) */
    tt_ptr_op(tor_inet_ntop(AF_INET6, &a1, buf, strlen(ip)),OP_EQ, NULL);
    tt_ptr_op(tor_inet_ntop(AF_INET6, &a2, buf, strlen(ip_ffff)),OP_EQ, NULL);
  }

  /* ==== Converting to and from sockaddr_t. */
  sin = (struct sockaddr_in *)&sa_storage;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(9090);
  sin->sin_addr.s_addr = htonl(0x7f7f0102); /*127.127.1.2*/
  tor_addr_from_sockaddr(&t1, (struct sockaddr *)sin, &port1);
  tt_int_op(tor_addr_family(&t1),OP_EQ, AF_INET);
  tt_int_op(tor_addr_to_ipv4h(&t1),OP_EQ, 0x7f7f0102);
  tt_int_op(port1, OP_EQ, 9090);

  memset(&sa_storage, 0, sizeof(sa_storage));
  tt_int_op(sizeof(struct sockaddr_in),OP_EQ,
          tor_addr_to_sockaddr(&t1, 1234, (struct sockaddr *)&sa_storage,
                               sizeof(sa_storage)));
  tt_int_op(1234,OP_EQ, ntohs(sin->sin_port));
  tt_int_op(0x7f7f0102,OP_EQ, ntohl(sin->sin_addr.s_addr));

  memset(&sa_storage, 0, sizeof(sa_storage));
  sin6 = (struct sockaddr_in6 *)&sa_storage;
  sin6->sin6_family = AF_INET6;
  sin6->sin6_port = htons(7070);
  sin6->sin6_addr.s6_addr[0] = 128;
  tor_addr_from_sockaddr(&t1, (struct sockaddr *)sin6, &port1);
  tt_int_op(tor_addr_family(&t1),OP_EQ, AF_INET6);
  tt_int_op(port1, OP_EQ, 7070);
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 0);
  tt_str_op(p1,OP_EQ, "8000::");

  memset(&sa_storage, 0, sizeof(sa_storage));
  tt_int_op(sizeof(struct sockaddr_in6),OP_EQ,
          tor_addr_to_sockaddr(&t1, 9999, (struct sockaddr *)&sa_storage,
                               sizeof(sa_storage)));
  tt_int_op(AF_INET6,OP_EQ, sin6->sin6_family);
  tt_int_op(9999,OP_EQ, ntohs(sin6->sin6_port));
  tt_int_op(0x80000000,OP_EQ, ntohl(S6_ADDR32(sin6->sin6_addr)[0]));

  /* ==== tor_addr_lookup: static cases.  (Can't test dns without knowing we
   * have a good resolver. */
  tt_int_op(0,OP_EQ, tor_addr_lookup("127.128.129.130", AF_UNSPEC, &t1));
  tt_int_op(AF_INET,OP_EQ, tor_addr_family(&t1));
  tt_int_op(tor_addr_to_ipv4h(&t1),OP_EQ, 0x7f808182);

  tt_int_op(0,OP_EQ, tor_addr_lookup("9000::5", AF_UNSPEC, &t1));
  tt_int_op(AF_INET6,OP_EQ, tor_addr_family(&t1));
  tt_int_op(0x90,OP_EQ, tor_addr_to_in6_addr8(&t1)[0]);
  tt_assert(fast_mem_is_zero((char*)tor_addr_to_in6_addr8(&t1)+1, 14));
  tt_int_op(0x05,OP_EQ, tor_addr_to_in6_addr8(&t1)[15]);

  /* === Test pton: valid af_inet6 */
  /* Simple, valid parsing. */
  r = tor_inet_pton(AF_INET6,
                    "0102:0304:0506:0708:090A:0B0C:0D0E:0F10", &a1);
  tt_int_op(r, OP_EQ, 1);
  for (i=0;i<16;++i) { tt_int_op(i+1,OP_EQ, (int)a1.s6_addr[i]); }
  /* ipv4 ending. */
  test_pton6_same("0102:0304:0506:0708:090A:0B0C:0D0E:0F10",
                  "0102:0304:0506:0708:090A:0B0C:13.14.15.16");
  /* shortened words. */
  test_pton6_same("0001:0099:BEEF:0000:0123:FFFF:0001:0001",
                  "1:99:BEEF:0:0123:FFFF:1:1");
  /* zeros at the beginning */
  test_pton6_same("0000:0000:0000:0000:0009:C0A8:0001:0001",
                  "::9:c0a8:1:1");
  test_pton6_same("0000:0000:0000:0000:0009:C0A8:0001:0001",
                  "::9:c0a8:0.1.0.1");
  /* zeros in the middle. */
  test_pton6_same("fe80:0000:0000:0000:0202:1111:0001:0001",
                  "fe80::202:1111:1:1");
  /* zeros at the end. */
  test_pton6_same("1000:0001:0000:0007:0000:0000:0000:0000",
                  "1000:1:0:7::");

  /* === Test ntop: af_inet6 */
  test_ntop6_reduces("0:0:0:0:0:0:0:0", "::");

  test_ntop6_reduces("0001:0099:BEEF:0006:0123:FFFF:0001:0001",
                     "1:99:beef:6:123:ffff:1:1");

  //test_ntop6_reduces("0:0:0:0:0:0:c0a8:0101", "::192.168.1.1");
  test_ntop6_reduces("0:0:0:0:0:ffff:c0a8:0101", "::ffff:192.168.1.1");
  test_ntop6_reduces("0:0:0:0:0:0:c0a8:0101", "::192.168.1.1");
  test_ntop6_reduces("002:0:0000:0:3::4", "2::3:0:0:4");
  test_ntop6_reduces("0:0::1:0:3", "::1:0:3");
  test_ntop6_reduces("008:0::0", "8::");
  test_ntop6_reduces("0:0:0:0:0:ffff::1", "::ffff:0.0.0.1");
  test_ntop6_reduces("abcd:0:0:0:0:0:7f00::", "abcd::7f00:0");
  test_ntop6_reduces("0000:0000:0000:0000:0009:C0A8:0001:0001",
                     "::9:c0a8:1:1");
  test_ntop6_reduces("fe80:0000:0000:0000:0202:1111:0001:0001",
                     "fe80::202:1111:1:1");
  test_ntop6_reduces("1000:0001:0000:0007:0000:0000:0000:0000",
                     "1000:1:0:7::");

  /* Bad af param */
  tt_int_op(tor_inet_pton(AF_UNSPEC, 0, 0),OP_EQ, -1);

  /* === Test pton: invalid in6. */
  test_pton6_bad("foobar.");
  test_pton6_bad("-1::");
  test_pton6_bad("00001::");
  test_pton6_bad("10000::");
  test_pton6_bad("::10000");
  test_pton6_bad("55555::");
  test_pton6_bad("9:-60::");
  test_pton6_bad("9:+60::");
  test_pton6_bad("9|60::");
  test_pton6_bad("0x60::");
  test_pton6_bad("::0x60");
  test_pton6_bad("9:0x60::");
  test_pton6_bad("1:2:33333:4:0002:3::");
  test_pton6_bad("1:2:3333:4:fish:3::");
  test_pton6_bad("1:2:3:4:5:6:7:8:9");
  test_pton6_bad("1:2:3:4:5:6:7");
  test_pton6_bad("1:2:3:4:5:6:1.2.3.4.5");
  test_pton6_bad("1:2:3:4:5:6:1.2.3");
  test_pton6_bad("::1.2.3");
  test_pton6_bad("::1.2.3.4.5");
  test_pton6_bad("::ffff:0xff.0.0.0");
  test_pton6_bad("::ffff:ff.0.0.0");
  test_pton6_bad("::ffff:256.0.0.0");
  test_pton6_bad("::ffff:-1.0.0.0");
  test_pton6_bad("99");
  test_pton6_bad("");
  test_pton6_bad(".");
  test_pton6_bad(":");
  test_pton6_bad("1::2::3:4");
  test_pton6_bad("a:::b:c");
  test_pton6_bad(":::a:b:c");
  test_pton6_bad("a:b:c:::");
  test_pton6_bad("1.2.3.4");
  test_pton6_bad(":1.2.3.4");
  test_pton6_bad(".2.3.4");
  /* Regression tests for 22789. */
  test_pton6_bad("0xfoo");
  test_pton6_bad("0x88");
  test_pton6_bad("0xyxxy");
  test_pton6_bad("0XFOO");
  test_pton6_bad("0X88");
  test_pton6_bad("0XYXXY");
  test_pton6_bad("0x");
  test_pton6_bad("0X");
  test_pton6_bad("2000::1a00::1000:fc098");

  /* test internal checking */
  test_external_ip("fbff:ffff::2:7", 0);
  test_internal_ip("fc01::2:7", 0);
  test_internal_ip("fc01::02:7", 0);
  test_internal_ip("fc01::002:7", 0);
  test_internal_ip("fc01::0002:7", 0);
  test_internal_ip("fdff:ffff::f:f", 0);
  test_external_ip("fe00::3:f", 0);

  test_external_ip("fe7f:ffff::2:7", 0);
  test_internal_ip("fe80::2:7", 0);
  test_internal_ip("febf:ffff::f:f", 0);

  test_internal_ip("fec0::2:7:7", 0);
  test_internal_ip("feff:ffff::e:7:7", 0);
  test_external_ip("ff00::e:7:7", 0);

  test_internal_ip("::", 0);
  test_internal_ip("::1", 0);
  test_internal_ip("::1", 1);
  test_internal_ip("::", 0);
  test_external_ip("::", 1);
  test_external_ip("::2", 0);
  test_external_ip("2001::", 0);
  test_external_ip("ffff::", 0);

  test_external_ip("::ffff:0.0.0.0", 1);
  test_internal_ip("::ffff:0.0.0.0", 0);
  test_internal_ip("::ffff:0.255.255.255", 0);
  test_external_ip("::ffff:1.0.0.0", 0);

  test_external_ip("::ffff:9.255.255.255", 0);
  test_internal_ip("::ffff:10.0.0.0", 0);
  test_internal_ip("::ffff:10.255.255.255", 0);
  test_external_ip("::ffff:11.0.0.0", 0);

  test_external_ip("::ffff:126.255.255.255", 0);
  test_internal_ip("::ffff:127.0.0.0", 0);
  test_internal_ip("::ffff:127.255.255.255", 0);
  test_external_ip("::ffff:128.0.0.0", 0);

  test_external_ip("::ffff:172.15.255.255", 0);
  test_internal_ip("::ffff:172.16.0.0", 0);
  test_internal_ip("::ffff:172.31.255.255", 0);
  test_external_ip("::ffff:172.32.0.0", 0);

  test_external_ip("::ffff:192.167.255.255", 0);
  test_internal_ip("::ffff:192.168.0.0", 0);
  test_internal_ip("::ffff:192.168.255.255", 0);
  test_external_ip("::ffff:192.169.0.0", 0);

  test_external_ip("::ffff:169.253.255.255", 0);
  test_internal_ip("::ffff:169.254.0.0", 0);
  test_internal_ip("::ffff:169.254.255.255", 0);
  test_external_ip("::ffff:169.255.0.0", 0);

  /* tor_addr_compare(tor_addr_t x2) */
  test_addr_compare("ffff::", OP_EQ, "ffff::0");
  test_addr_compare("0::3:2:1", OP_LT, "0::ffff:0.3.2.1");
  test_addr_compare("0::2:2:1", OP_LT, "0::ffff:0.3.2.1");
  test_addr_compare("0::ffff:0.3.2.1", OP_GT, "0::0:0:0");
  test_addr_compare("0::ffff:5.2.2.1", OP_LT,
                    "::ffff:6.0.0.0"); /* XXXX wrong. */
  tor_addr_parse_mask_ports("[::ffff:2.3.4.5]", 0, &t1, NULL, NULL, NULL);
  tor_addr_parse_mask_ports("2.3.4.5", 0, &t2, NULL, NULL, NULL);
  tt_int_op(tor_addr_compare(&t1, &t2, CMP_SEMANTIC), OP_EQ, 0);
  tor_addr_parse_mask_ports("[::ffff:2.3.4.4]", 0, &t1, NULL, NULL, NULL);
  tor_addr_parse_mask_ports("2.3.4.5", 0, &t2, NULL, NULL, NULL);
  tt_int_op(tor_addr_compare(&t1, &t2, CMP_SEMANTIC), OP_LT, 0);

  /* test compare_masked */
  test_addr_compare_masked("ffff::", OP_EQ, "ffff::0", 128);
  test_addr_compare_masked("ffff::", OP_EQ, "ffff::0", 64);
  test_addr_compare_masked("0::2:2:1", OP_LT, "0::8000:2:1", 81);
  test_addr_compare_masked("0::2:2:1", OP_EQ, "0::8000:2:1", 80);

  /* Test undecorated tor_addr_to_str */
  tt_int_op(AF_INET6,OP_EQ, tor_addr_parse(&t1, "[123:45:6789::5005:11]"));
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 0);
  tt_str_op(p1,OP_EQ, "123:45:6789::5005:11");
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&t1, "18.0.0.1"));
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 0);
  tt_str_op(p1,OP_EQ, "18.0.0.1");

  /* Test decorated tor_addr_to_str */
  tt_int_op(AF_INET6,OP_EQ, tor_addr_parse(&t1, "[123:45:6789::5005:11]"));
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 1);
  tt_str_op(p1,OP_EQ, "[123:45:6789::5005:11]");
  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&t1, "18.0.0.1"));
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 1);
  tt_str_op(p1,OP_EQ, "18.0.0.1");

  /* Test buffer bounds checking of tor_addr_to_str */
  tt_int_op(AF_INET6,OP_EQ, tor_addr_parse(&t1, "::")); /* 2 + \0 */
  tt_ptr_op(tor_addr_to_str(buf, &t1, 2, 0),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 3, 0),OP_EQ, "::");
  tt_ptr_op(tor_addr_to_str(buf, &t1, 4, 1),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 5, 1),OP_EQ, "[::]");

  tt_int_op(AF_INET6,OP_EQ, tor_addr_parse(&t1, "2000::1337")); /* 10 + \0 */
  tt_ptr_op(tor_addr_to_str(buf, &t1, 10, 0),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 11, 0),OP_EQ, "2000::1337");
  tt_ptr_op(tor_addr_to_str(buf, &t1, 12, 1),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 13, 1),OP_EQ, "[2000::1337]");

  tt_int_op(AF_INET,OP_EQ, tor_addr_parse(&t1, "1.2.3.4")); /* 7 + \0 */
  tt_ptr_op(tor_addr_to_str(buf, &t1, 7, 0),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 8, 0),OP_EQ, "1.2.3.4");

  tt_int_op(AF_INET, OP_EQ,
            tor_addr_parse(&t1, "255.255.255.255")); /* 15 + \0 */
  tt_ptr_op(tor_addr_to_str(buf, &t1, 15, 0),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 16, 0),OP_EQ, "255.255.255.255");
  tt_ptr_op(tor_addr_to_str(buf, &t1, 15, 1),OP_EQ, NULL); /* too short buf */
  tt_str_op(tor_addr_to_str(buf, &t1, 16, 1),OP_EQ, "255.255.255.255");

  t1.family = AF_UNSPEC;
  tt_ptr_op(tor_addr_to_str(buf, &t1, sizeof(buf), 0),OP_EQ, NULL);

  /* Test tor_addr_parse_PTR_name */
  i = tor_addr_parse_PTR_name(&t1, "Foobar.baz", AF_UNSPEC, 0);
  tt_int_op(0,OP_EQ, i);
  i = tor_addr_parse_PTR_name(&t1, "Foobar.baz", AF_UNSPEC, 1);
  tt_int_op(0,OP_EQ, i);
  i = tor_addr_parse_PTR_name(&t1, "9999999999999999999999999999.in-addr.arpa",
                              AF_UNSPEC, 1);
  tt_int_op(-1,OP_EQ, i);
  i = tor_addr_parse_PTR_name(&t1, "1.0.168.192.in-addr.arpa",
                                         AF_UNSPEC, 1);
  tt_int_op(1,OP_EQ, i);
  tt_int_op(tor_addr_family(&t1),OP_EQ, AF_INET);
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 1);
  tt_str_op(p1,OP_EQ, "192.168.0.1");
  i = tor_addr_parse_PTR_name(&t1, "192.168.0.99", AF_UNSPEC, 0);
  tt_int_op(0,OP_EQ, i);
  i = tor_addr_parse_PTR_name(&t1, "192.168.0.99", AF_UNSPEC, 1);
  tt_int_op(1,OP_EQ, i);
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 1);
  tt_str_op(p1,OP_EQ, "192.168.0.99");
  memset(&t1, 0, sizeof(t1));
  i = tor_addr_parse_PTR_name(&t1,
                                         "0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f."
                                         "f.e.e.b.1.e.b.e.e.f.f.e.e.e.d.9."
                                         "ip6.ARPA",
                                         AF_UNSPEC, 0);
  tt_int_op(1,OP_EQ, i);
  p1 = tor_addr_to_str(buf, &t1, sizeof(buf), 1);
  tt_str_op(p1,OP_EQ, "[9dee:effe:ebe1:beef:fedc:ba98:7654:3210]");
  /* Failing cases. */
  i = tor_addr_parse_PTR_name(&t1,
                                         "6.7.8.9.a.b.c.d.e.f."
                                         "f.e.e.b.1.e.b.e.e.f.f.e.e.e.d.9."
                                         "ip6.ARPA",
                                         AF_UNSPEC, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1,
                                         "6.7.8.9.a.b.c.d.e.f.a.b.c.d.e.f.0."
                                         "f.e.e.b.1.e.b.e.e.f.f.e.e.e.d.9."
                                         "ip6.ARPA",
                                         AF_UNSPEC, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1,
                                         "6.7.8.9.a.b.c.d.e.f.X.0.0.0.0.9."
                                         "f.e.e.b.1.e.b.e.e.f.f.e.e.e.d.9."
                                         "ip6.ARPA",
                                         AF_UNSPEC, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1, "32.1.1.in-addr.arpa",
                                         AF_UNSPEC, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1, ".in-addr.arpa",
                                         AF_UNSPEC, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1, "1.2.3.4.5.in-addr.arpa",
                                         AF_UNSPEC, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1, "1.2.3.4.5.in-addr.arpa",
                                         AF_INET6, 0);
  tt_int_op(i,OP_EQ, -1);
  i = tor_addr_parse_PTR_name(&t1,
                                         "6.7.8.9.a.b.c.d.e.f.a.b.c.d.e.0."
                                         "f.e.e.b.1.e.b.e.e.f.f.e.e.e.d.9."
                                         "ip6.ARPA",
                                         AF_INET, 0);
  tt_int_op(i,OP_EQ, -1);

  /* === Test tor_addr_to_PTR_name */

  /* Stage IPv4 addr */
  memset(&sa_storage, 0, sizeof(sa_storage));
  sin = (struct sockaddr_in *)&sa_storage;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = htonl(0x7f010203); /* 127.1.2.3 */
  tor_addr_from_sockaddr(&t1, (struct sockaddr *)sin, NULL);

  /* Check IPv4 PTR - too short buffer */
  tt_int_op(tor_addr_to_PTR_name(rbuf, 1, &t1),OP_EQ, -1);
  tt_int_op(tor_addr_to_PTR_name(rbuf,
                               strlen("3.2.1.127.in-addr.arpa") - 1,
                               &t1),OP_EQ, -1);

  /* Check IPv4 PTR - valid addr */
  tt_int_op(tor_addr_to_PTR_name(rbuf, sizeof(rbuf), &t1),OP_EQ,
          strlen("3.2.1.127.in-addr.arpa"));
  tt_str_op(rbuf,OP_EQ, "3.2.1.127.in-addr.arpa");

  /* Invalid addr family */
  t1.family = AF_UNSPEC;
  tt_int_op(tor_addr_to_PTR_name(rbuf, sizeof(rbuf), &t1),OP_EQ, -1);

  /* Stage IPv6 addr */
  memset(&sa_storage, 0, sizeof(sa_storage));
  sin6 = (struct sockaddr_in6 *)&sa_storage;
  sin6->sin6_family = AF_INET6;
  sin6->sin6_addr.s6_addr[0] = 0x80; /* 8000::abcd */
  sin6->sin6_addr.s6_addr[14] = 0xab;
  sin6->sin6_addr.s6_addr[15] = 0xcd;

  tor_addr_from_sockaddr(&t1, (struct sockaddr *)sin6, NULL);

  {
    const char* addr_PTR = "d.c.b.a.0.0.0.0.0.0.0.0.0.0.0.0."
      "0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.ip6.arpa";

    /* Check IPv6 PTR - too short buffer */
    tt_int_op(tor_addr_to_PTR_name(rbuf, 0, &t1),OP_EQ, -1);
    tt_int_op(tor_addr_to_PTR_name(rbuf, strlen(addr_PTR) - 1, &t1),OP_EQ, -1);

    /* Check IPv6 PTR - valid addr */
    tt_int_op(tor_addr_to_PTR_name(rbuf, sizeof(rbuf), &t1),OP_EQ,
            strlen(addr_PTR));
    tt_str_op(rbuf,OP_EQ, addr_PTR);
  }

  /* XXXX turn this into a separate function; it's not all IPv6. */
  /* test tor_addr_parse_mask_ports */
  test_addr_mask_ports_parse("[::f]/17:47-95", AF_INET6,
                             0, 0, 0, 0x0000000f, 17, 47, 95);
  tt_str_op(p1,OP_EQ, "::f");
  //test_addr_parse("[::fefe:4.1.1.7/120]:999-1000");
  //test_addr_parse_check("::fefe:401:107", 120, 999, 1000);
  test_addr_mask_ports_parse("[::ffff:4.1.1.7]/120:443", AF_INET6,
                             0, 0, 0x0000ffff, 0x04010107, 120, 443, 443);
  tt_str_op(p1,OP_EQ, "::ffff:4.1.1.7");
  test_addr_mask_ports_parse("[abcd:2::44a:0]:2-65000", AF_INET6,
                             0xabcd0002, 0, 0, 0x044a0000, 128, 2, 65000);

  tt_str_op(p1,OP_EQ, "abcd:2::44a:0");
  /* Try some long addresses. */
  r=tor_addr_parse_mask_ports("[ffff:1111:1111:1111:1111:1111:1111:1111]",
                              0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, AF_INET6);
  r=tor_addr_parse_mask_ports("[ffff:1111:1111:1111:1111:1111:1111:11111]",
                              0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[ffff:1111:1111:1111:1111:1111:1111:1111:1]",
                              0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports(
         "[ffff:1111:1111:1111:1111:1111:1111:ffff:"
         "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
         "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:"
         "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff]",
         0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  /* Try some failing cases. */
  r=tor_addr_parse_mask_ports("[fefef::]/112", 0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[fefe::/112", 0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[fefe::", 0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[fefe::X]", 0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("efef::/112", 0, &t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[f:f:f:f:f:f:f:f::]",0,&t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[::f:f:f:f:f:f:f:f]",0,&t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[f:f:f:f:f:f:f:f:f]",0,&t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[f:f:f:f:f::]/fred",0,&t1,&mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("[f:f:f:f:f::]/255.255.0.0",
                              0,&t1, NULL, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  /* This one will get rejected because it isn't a pure prefix. */
  r=tor_addr_parse_mask_ports("1.1.2.3/255.255.64.0",0,&t1, &mask,NULL,NULL);
  tt_int_op(r, OP_EQ, -1);
  /* Test for V4-mapped address with mask < 96.  (arguably not valid) */
  r=tor_addr_parse_mask_ports("[::ffff:1.1.2.2/33]",0,&t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("1.1.2.2/33",0,&t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  /* Try extended wildcard addresses with out TAPMP_EXTENDED_STAR*/
  r=tor_addr_parse_mask_ports("*4",0,&t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("*6",0,&t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  tt_int_op(r, OP_EQ, -1);
  /* Try a mask with a wildcard. */
  r=tor_addr_parse_mask_ports("*/16",0,&t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("*4/16",TAPMP_EXTENDED_STAR,
                              &t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("*6/30",TAPMP_EXTENDED_STAR,
                              &t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, -1);
  /* Basic mask tests*/
  r=tor_addr_parse_mask_ports("1.1.2.2/31",0,&t1, &mask, NULL, NULL);
  tt_int_op(r, OP_EQ, AF_INET);
  tt_int_op(mask,OP_EQ,31);
  tt_int_op(tor_addr_family(&t1),OP_EQ,AF_INET);
  tt_int_op(tor_addr_to_ipv4h(&t1),OP_EQ,0x01010202);
  r=tor_addr_parse_mask_ports("3.4.16.032:1-2",0,&t1, &mask, &port1, &port2);
  tt_int_op(r, OP_EQ, -1);
  r=tor_addr_parse_mask_ports("1.1.2.3/255.255.128.0",0,&t1, &mask,NULL,NULL);
  tt_int_op(r, OP_EQ, AF_INET);
  tt_int_op(mask,OP_EQ,17);
  tt_int_op(tor_addr_family(&t1),OP_EQ,AF_INET);
  tt_int_op(tor_addr_to_ipv4h(&t1),OP_EQ,0x01010203);
  r=tor_addr_parse_mask_ports("[efef::]/112",0,&t1, &mask, &port1, &port2);
  tt_int_op(r, OP_EQ, AF_INET6);
  tt_uint_op(port1, OP_EQ, 1);
  tt_uint_op(port2, OP_EQ, 65535);
  /* Try regular wildcard behavior without TAPMP_EXTENDED_STAR */
  r=tor_addr_parse_mask_ports("*:80-443",0,&t1,&mask,&port1,&port2);
  tt_int_op(r,OP_EQ,AF_INET); /* Old users of this always get inet */
  tt_int_op(tor_addr_family(&t1),OP_EQ,AF_INET);
  tt_int_op(tor_addr_to_ipv4h(&t1),OP_EQ,0);
  tt_int_op(mask,OP_EQ,0);
  tt_int_op(port1,OP_EQ,80);
  tt_int_op(port2,OP_EQ,443);
  /* Now try wildcards *with* TAPMP_EXTENDED_STAR */
  r=tor_addr_parse_mask_ports("*:8000-9000",TAPMP_EXTENDED_STAR,
                              &t1,&mask,&port1,&port2);
  tt_int_op(r,OP_EQ,AF_UNSPEC);
  tt_int_op(tor_addr_family(&t1),OP_EQ,AF_UNSPEC);
  tt_int_op(mask,OP_EQ,0);
  tt_int_op(port1,OP_EQ,8000);
  tt_int_op(port2,OP_EQ,9000);
  r=tor_addr_parse_mask_ports("*4:6667",TAPMP_EXTENDED_STAR,
                              &t1,&mask,&port1,&port2);
  tt_int_op(r,OP_EQ,AF_INET);
  tt_int_op(tor_addr_family(&t1),OP_EQ,AF_INET);
  tt_int_op(tor_addr_to_ipv4h(&t1),OP_EQ,0);
  tt_int_op(mask,OP_EQ,0);
  tt_int_op(port1,OP_EQ,6667);
  tt_int_op(port2,OP_EQ,6667);
  r=tor_addr_parse_mask_ports("*6",TAPMP_EXTENDED_STAR,
                              &t1,&mask,&port1,&port2);
  tt_int_op(r,OP_EQ,AF_INET6);
  tt_int_op(tor_addr_family(&t1),OP_EQ,AF_INET6);
  tt_assert(fast_mem_is_zero((const char*)tor_addr_to_in6_addr32(&t1), 16));
  tt_int_op(mask,OP_EQ,0);
  tt_int_op(port1,OP_EQ,1);
  tt_int_op(port2,OP_EQ,65535);

  /* make sure inet address lengths >= max */
  tt_int_op(INET_NTOA_BUF_LEN, OP_GE, sizeof("255.255.255.255"));
  tt_int_op(TOR_ADDR_BUF_LEN, OP_GE,
            sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"));

  tt_assert(sizeof(tor_addr_t) >= sizeof(struct in6_addr));

  /* get interface addresses */
  r = get_interface_address6(LOG_DEBUG, AF_INET, &t1);
  tt_int_op(r, OP_LE, 0); // "it worked or it didn't"
  i = get_interface_address6(LOG_DEBUG, AF_INET6, &t2);
  tt_int_op(i, OP_LE, 0); // "it worked or it didn't"

  TT_BLATHER(("v4 address: %s (family=%d)", fmt_addr(&t1),
              tor_addr_family(&t1)));
  TT_BLATHER(("v6 address: %s (family=%d)", fmt_addr(&t2),
              tor_addr_family(&t2)));

 done:
  ;
}

/* Test that addr_str successfully parses, and:
 *  - the address has family expect_family,
 *  - the fmt_decorated result of tor_addr_to_str() is expect_str.
 */
#define TEST_ADDR_PARSE_FMT(addr_str, expect_family, fmt_decorated, \
                            expect_str) \
  STMT_BEGIN \
    r = tor_addr_parse(&addr, addr_str); \
    tt_int_op(r, OP_EQ, expect_family); \
    sv = tor_addr_to_str(buf, &addr, sizeof(buf), fmt_decorated); \
    tt_str_op(sv, OP_EQ, buf); \
    tt_str_op(buf, OP_EQ, expect_str); \
  STMT_END

/* Test that addr_str fails to parse, and:
 *  - the returned address is null.
 */
#define TEST_ADDR_PARSE_XFAIL(addr_str) \
  STMT_BEGIN \
    r = tor_addr_parse(&addr, addr_str); \
    tt_int_op(r, OP_EQ, -1); \
    tt_assert(tor_addr_is_null(&addr)); \
  STMT_END

/* Test that addr_port_str and default_port successfully parse, and:
 *  - the address has family expect_family,
 *  - the fmt_decorated result of tor_addr_to_str() is expect_str,
 *  - the port is expect_port.
 */
#define TEST_ADDR_PORT_PARSE_FMT(addr_port_str, default_port, expect_family, \
                                 fmt_decorated, expect_str, expect_port) \
  STMT_BEGIN \
    r = tor_addr_port_parse(LOG_DEBUG, addr_port_str, &addr, &port, \
                            default_port); \
    tt_int_op(r, OP_EQ, 0); \
    tt_int_op(tor_addr_family(&addr), OP_EQ, expect_family); \
    sv = tor_addr_to_str(buf, &addr, sizeof(buf), fmt_decorated); \
    tt_str_op(sv, OP_EQ, buf); \
    tt_str_op(buf, OP_EQ, expect_str); \
    tt_int_op(port, OP_EQ, expect_port); \
  STMT_END

/* Test that addr_port_str and default_port fail to parse, and:
 *  - the returned address is null,
 *  - the returned port is 0.
 */
#define TEST_ADDR_PORT_PARSE_XFAIL(addr_port_str, default_port) \
  STMT_BEGIN \
    r = tor_addr_port_parse(LOG_DEBUG, addr_port_str, &addr, &port, \
                            default_port); \
    tt_int_op(r, OP_EQ, -1); \
    tt_assert(tor_addr_is_null(&addr)); \
    tt_int_op(port, OP_EQ, 0); \
  STMT_END

/* Test that addr_str successfully parses as an IPv4 address using
 * tor_lookup_hostname(), and:
 *  - the fmt_addr32() of the result is expect_str.
 */
#define TEST_ADDR_V4_LOOKUP_HOSTNAME(addr_str, expect_str) \
  STMT_BEGIN \
    r = tor_lookup_hostname(addr_str, &addr32h); \
    tt_int_op(r, OP_EQ, 0); \
    tt_str_op(fmt_addr32(addr32h), OP_EQ, expect_str); \
  STMT_END

/* Test that bad_str fails to parse using tor_lookup_hostname(), with a
 * permanent failure, and:
 *  - the returned address is 0.
 */
#define TEST_ADDR_V4_LOOKUP_XFAIL(bad_str) \
  STMT_BEGIN \
    r = tor_lookup_hostname(bad_str, &addr32h); \
    tt_int_op(r, OP_EQ, -1); \
    tt_int_op(addr32h, OP_EQ, 0); \
  STMT_END

/* Test that looking up host_str as an IPv4 address using tor_lookup_hostname()
 * does something sensible:
 *  - the result is -1, 0, or 1.
 *  - if the result is a failure, the returned address is 0.
 * We can't rely on the result of this function, because it depends on the
 * network.
 */
#define TEST_HOST_V4_LOOKUP(host_str) \
  STMT_BEGIN \
    r = tor_lookup_hostname(host_str, &addr32h); \
    tt_int_op(r, OP_GE, -1); \
    tt_int_op(r, OP_LE, 1); \
    if (r != 0) \
      tt_int_op(addr32h, OP_EQ, 0); \
  STMT_END

/* Test that addr_str successfully parses as a require_family IP address using
 * tor_addr_lookup(), and:
 *  - the address has family expect_family,
 *  - the fmt_decorated result of tor_addr_to_str() is expect_str.
 */
#define TEST_ADDR_LOOKUP_FMT(addr_str, require_family, expect_family, \
                             fmt_decorated, expect_str) \
  STMT_BEGIN \
    r = tor_addr_lookup(addr_str, require_family, &addr); \
    tt_int_op(r, OP_EQ, 0); \
    tt_int_op(tor_addr_family(&addr), OP_EQ, expect_family); \
    sv = tor_addr_to_str(buf, &addr, sizeof(buf), fmt_decorated); \
    tt_str_op(sv, OP_EQ, buf); \
    tt_str_op(buf, OP_EQ, expect_str); \
  STMT_END

/* Test that bad_str fails to parse as a require_family IP address using
 * tor_addr_lookup(), with a permanent failure, and:
 *  - the returned address is null.
 */
#define TEST_ADDR_LOOKUP_XFAIL(bad_str, require_family) \
  STMT_BEGIN \
    r = tor_addr_lookup(bad_str, require_family, &addr); \
    tt_int_op(r, OP_EQ, -1); \
    tt_assert(tor_addr_is_null(&addr)); \
  STMT_END

/* Test that looking up host_string as a require_family IP address using
 * tor_addr_lookup(),  does something sensible:
 *  - the result is -1, 0, or 1.
 *  - if the result is a failure, the returned address is null.
 * We can't rely on the result of this function, because it depends on the
 * network.
 */
#define TEST_HOST_LOOKUP(host_str, require_family) \
  STMT_BEGIN \
    r = tor_addr_lookup(host_str, require_family, &addr); \
    tt_int_op(r, OP_GE, -1); \
    tt_int_op(r, OP_LE, 1); \
    if (r != 0) \
      tt_assert(tor_addr_is_null(&addr)); \
  STMT_END

/* Test that addr_port_str successfully parses as an IP address and port
 * using tor_addr_port_lookup(), and:
 *  - the address has family expect_family,
 *  - the fmt_decorated result of tor_addr_to_str() is expect_str,
 *  - the port is expect_port.
 */
#define TEST_ADDR_PORT_LOOKUP_FMT(addr_port_str, expect_family, \
                                  fmt_decorated, expect_str, expect_port) \
  STMT_BEGIN \
    r = tor_addr_port_lookup(addr_port_str, &addr, &port); \
    tt_int_op(r, OP_EQ, 0); \
    tt_int_op(tor_addr_family(&addr), OP_EQ, expect_family); \
    sv = tor_addr_to_str(buf, &addr, sizeof(buf), fmt_decorated); \
    tt_str_op(sv, OP_EQ, buf); \
    tt_str_op(buf, OP_EQ, expect_str); \
    tt_int_op(port, OP_EQ, expect_port); \
  STMT_END

/* Test that bad_str fails to parse as an IP address and port
 * using tor_addr_port_lookup(), and:
 *  - the returned address is null,
 *  - the returned port is 0.
 */
#define TEST_ADDR_PORT_LOOKUP_XFAIL(bad_str) \
  STMT_BEGIN \
    r = tor_addr_port_lookup(bad_str, &addr, &port); \
    tt_int_op(r, OP_EQ, -1); \
    tt_assert(tor_addr_is_null(&addr)); \
    tt_int_op(port, OP_EQ, 0); \
  STMT_END

/* Test that looking up host_port_str as an IP address using
 * tor_addr_port_lookup(),  does something sensible:
 *  - the result is -1 or 0.
 *  - if the result is a failure, the returned address is null, and the
 *    returned port is zero,
 *  - if the result is a success, the returned port is expect_success_port,
 *    and the returned family is AF_INET or AF_INET6.
 * We can't rely on the result of this function, because it depends on the
 * network.
 */
#define TEST_HOST_PORT_LOOKUP(host_port_str, expect_success_port) \
  STMT_BEGIN \
    r = tor_addr_port_lookup(host_port_str, &addr, &port); \
    tt_int_op(r, OP_GE, -1); \
    tt_int_op(r, OP_LE, 0); \
    if (r == -1) { \
      tt_assert(tor_addr_is_null(&addr)); \
      tt_int_op(port, OP_EQ, 0); \
    } else { \
      tt_assert(tor_addr_family(&addr) == AF_INET || \
                tor_addr_family(&addr) == AF_INET6); \
      tt_int_op(port, OP_EQ, expect_success_port); \
    } \
  STMT_END

/* Test that addr_str successfully parses as a canonical IPv4 address.
 * Check for successful parsing using:
 *  - tor_addr_parse(),
 *  - tor_addr_port_parse() with a default port,
 *  - tor_lookup_hostname(),
 *  - tor_addr_lookup() with AF_INET,
 *  - tor_addr_lookup() with AF_UNSPEC,
 *  - tor_addr_port_lookup(), with a zero port.
 * Check for failures using:
 *  - tor_addr_port_parse() without a default port, because there is no port,
 *  - tor_addr_lookup() with AF_INET6,
 *  - tor_addr_port_lookup(), because there is no port.
 */
#define TEST_ADDR_V4_PARSE_CANONICAL(addr_str) \
  STMT_BEGIN \
    TEST_ADDR_PARSE_FMT(addr_str, AF_INET, 0, addr_str); \
    TEST_ADDR_PORT_PARSE_FMT(addr_str, 111, AF_INET, 0, \
                             addr_str, 111); \
    TEST_ADDR_V4_LOOKUP_HOSTNAME(addr_str, addr_str); \
    TEST_ADDR_PORT_LOOKUP_FMT(addr_str, AF_INET, 0, addr_str, 0); \
    TEST_ADDR_LOOKUP_FMT(addr_str, AF_INET, AF_INET, 0, addr_str); \
    TEST_ADDR_LOOKUP_FMT(addr_str, AF_UNSPEC, AF_INET, 0, addr_str); \
    TEST_ADDR_PORT_PARSE_XFAIL(addr_str, -1); \
    TEST_ADDR_LOOKUP_XFAIL(addr_str, AF_INET6); \
  STMT_END

/* Test that addr_str successfully parses as a canonical fmt_decorated
 * IPv6 address.
 * Check for successful parsing using:
 *  - tor_addr_parse(),
 *  - tor_addr_port_parse() with a default port,
 *  - tor_addr_lookup() with AF_INET6,
 *  - tor_addr_lookup() with AF_UNSPEC,
 *  - tor_addr_port_lookup(), with a zero port.
 * Check for failures using:
 *  - tor_addr_port_parse() without a default port, because there is no port,
 *  - tor_lookup_hostname(), because it only supports IPv4,
 *  - tor_addr_lookup() with AF_INET.
 */
#define TEST_ADDR_V6_PARSE_CANONICAL(addr_str, fmt_decorated) \
  STMT_BEGIN \
    TEST_ADDR_PARSE_FMT(addr_str, AF_INET6, fmt_decorated, addr_str); \
    TEST_ADDR_PORT_PARSE_FMT(addr_str, 222, AF_INET6, fmt_decorated, \
                             addr_str, 222); \
    TEST_ADDR_LOOKUP_FMT(addr_str, AF_INET6, AF_INET6, fmt_decorated, \
                         addr_str); \
    TEST_ADDR_LOOKUP_FMT(addr_str, AF_UNSPEC, AF_INET6, fmt_decorated, \
                         addr_str); \
    TEST_ADDR_PORT_LOOKUP_FMT(addr_str, AF_INET6, fmt_decorated, addr_str, \
                              0); \
    TEST_ADDR_PORT_PARSE_XFAIL(addr_str, -1); \
    TEST_ADDR_V4_LOOKUP_XFAIL(addr_str); \
    TEST_ADDR_LOOKUP_XFAIL(addr_str, AF_INET); \
  STMT_END

/* Test that addr_str successfully parses, and the fmt_decorated canonical
 * IPv6 string is expect_str.
 * Check for successful parsing using:
 *  - tor_addr_parse(),
 *  - tor_addr_port_parse() with a default port,
 *  - tor_addr_lookup() with AF_INET6,
 *  - tor_addr_lookup() with AF_UNSPEC,
 *  - tor_addr_port_lookup(), with a zero port.
 * Check for failures using:
 *  - tor_addr_port_parse() without a default port, because there is no port.
 *  - tor_lookup_hostname(), because it only supports IPv4,
 *  - tor_addr_lookup() with AF_INET.
 */
#define TEST_ADDR_V6_PARSE(addr_str, fmt_decorated, expect_str) \
  STMT_BEGIN \
    TEST_ADDR_PARSE_FMT(addr_str, AF_INET6, fmt_decorated, expect_str); \
    TEST_ADDR_PORT_PARSE_FMT(addr_str, 333, AF_INET6, fmt_decorated, \
                             expect_str, 333); \
    TEST_ADDR_LOOKUP_FMT(addr_str, AF_INET6, AF_INET6, fmt_decorated, \
                         expect_str); \
    TEST_ADDR_LOOKUP_FMT(addr_str, AF_UNSPEC, AF_INET6, fmt_decorated, \
                         expect_str); \
    TEST_ADDR_PORT_LOOKUP_FMT(addr_str, AF_INET6, fmt_decorated, expect_str, \
                              0); \
    TEST_ADDR_PORT_PARSE_XFAIL(addr_str, -1); \
    TEST_ADDR_V4_LOOKUP_XFAIL(addr_str); \
    TEST_ADDR_LOOKUP_XFAIL(addr_str, AF_INET); \
  STMT_END

/* Test that addr_port_str successfully parses to the canonical IPv4 address
 * string expect_str, and port expect_port.
 * Check for successful parsing using:
 *  - tor_addr_port_parse() without a default port,
 *  - tor_addr_port_parse() with a default port,
 *  - tor_addr_port_lookup().
 * Check for failures using:
 *  - tor_addr_parse(), because there is a port,
 *  - tor_lookup_hostname(), because there is a port.
 *  - tor_addr_lookup(), regardless of the address family, because there is a
 *    port.
 */
#define TEST_ADDR_V4_PORT_PARSE(addr_port_str, expect_str, expect_port) \
  STMT_BEGIN \
    TEST_ADDR_PORT_PARSE_FMT(addr_port_str,  -1, AF_INET, 0, expect_str, \
                             expect_port); \
    TEST_ADDR_PORT_PARSE_FMT(addr_port_str, 444, AF_INET, 0, expect_str, \
                             expect_port); \
    TEST_ADDR_PORT_LOOKUP_FMT(addr_port_str, AF_INET, 0, expect_str, \
                              expect_port); \
    TEST_ADDR_PARSE_XFAIL(addr_port_str); \
    TEST_ADDR_V4_LOOKUP_XFAIL(addr_port_str); \
    TEST_ADDR_LOOKUP_XFAIL(addr_port_str, AF_INET); \
    TEST_ADDR_LOOKUP_XFAIL(addr_port_str, AF_UNSPEC); \
    TEST_ADDR_LOOKUP_XFAIL(addr_port_str, AF_INET6); \
  STMT_END

/* Test that addr_port_str successfully parses to the canonical undecorated
 * IPv6 address string expect_str, and port expect_port.
 * Check for successful parsing using:
 *  - tor_addr_port_parse() without a default port,
 *  - tor_addr_port_parse() with a default port,
 *  - tor_addr_port_lookup().
 * Check for failures using:
 *  - tor_addr_parse(), because there is a port,
 *  - tor_lookup_hostname(), because there is a port, and because it only
 *    supports IPv4,
 *  - tor_addr_lookup(), regardless of the address family, because there is a
 *    port.
 */
#define TEST_ADDR_V6_PORT_PARSE(addr_port_str, expect_str, expect_port) \
  STMT_BEGIN \
    TEST_ADDR_PORT_PARSE_FMT(addr_port_str,  -1, AF_INET6, 0, expect_str, \
                             expect_port); \
    TEST_ADDR_PORT_PARSE_FMT(addr_port_str, 555, AF_INET6, 0, expect_str, \
                             expect_port); \
    TEST_ADDR_PORT_LOOKUP_FMT(addr_port_str, AF_INET6, 0, expect_str, \
                              expect_port); \
    TEST_ADDR_PARSE_XFAIL(addr_port_str); \
    TEST_ADDR_V4_LOOKUP_XFAIL(addr_port_str); \
    TEST_ADDR_LOOKUP_XFAIL(addr_port_str, AF_INET6); \
    TEST_ADDR_LOOKUP_XFAIL(addr_port_str, AF_UNSPEC); \
    TEST_ADDR_LOOKUP_XFAIL(addr_port_str, AF_INET); \
  STMT_END

/* Test that bad_str fails to parse due to a bad address or port.
 * Check for failures using:
 *  - tor_addr_parse(),
 *  - tor_addr_port_parse() without a default port,
 *  - tor_addr_port_parse() with a default port,
 *  - tor_lookup_hostname(),
 *  - tor_addr_lookup(), regardless of the address family,
 *  - tor_addr_port_lookup().
 */
#define TEST_ADDR_PARSE_XFAIL_MALFORMED(bad_str) \
  STMT_BEGIN \
    TEST_ADDR_PARSE_XFAIL(bad_str); \
    TEST_ADDR_PORT_PARSE_XFAIL(bad_str,  -1); \
    TEST_ADDR_PORT_PARSE_XFAIL(bad_str, 666); \
    TEST_ADDR_V4_LOOKUP_XFAIL(bad_str); \
    TEST_ADDR_LOOKUP_XFAIL(bad_str, AF_UNSPEC); \
    TEST_ADDR_LOOKUP_XFAIL(bad_str, AF_INET); \
    TEST_ADDR_LOOKUP_XFAIL(bad_str, AF_INET6); \
    TEST_ADDR_PORT_LOOKUP_XFAIL(bad_str); \
  STMT_END

/* Test that host_str is treated as a hostname, and not an address.
 * Check for success or failure using the network-dependent functions:
 *  - tor_lookup_hostname(),
 *  - tor_addr_lookup(), regardless of the address family,
 *  - tor_addr_port_lookup(), expecting a zero port.
 * Check for failures using:
 *  - tor_addr_parse(),
 *  - tor_addr_port_parse() without a default port,
 *  - tor_addr_port_parse() with a default port.
 */
#define TEST_HOSTNAME(host_str) \
  STMT_BEGIN \
    TEST_HOST_V4_LOOKUP(host_str); \
    TEST_HOST_LOOKUP(host_str, AF_UNSPEC); \
    TEST_HOST_LOOKUP(host_str, AF_INET); \
    TEST_HOST_LOOKUP(host_str, AF_INET6); \
    TEST_HOST_PORT_LOOKUP(host_str, 0); \
    TEST_ADDR_PARSE_XFAIL(host_str); \
    TEST_ADDR_PORT_PARSE_XFAIL(host_str,  -1); \
    TEST_ADDR_PORT_PARSE_XFAIL(host_str, 777); \
  STMT_END

/* Test that host_port_str is treated as a hostname and port, and not a
 * hostname or an address.
 * Check for success or failure using the network-dependent function:
 *  - tor_addr_port_lookup(), expecting expect_success_port if the lookup is
 *    successful.
 * Check for failures using:
 *  - tor_addr_parse(),
 *  - tor_addr_port_parse() without a default port,
 *  - tor_addr_port_parse() with a default port,
 *  - tor_lookup_hostname(), because it doesn't support ports,
 *  - tor_addr_lookup(), regardless of the address family, because it doesn't
 *    support ports.
 */
#define TEST_HOSTNAME_PORT(host_port_str, expect_success_port) \
  STMT_BEGIN \
    TEST_HOST_PORT_LOOKUP(host_port_str, expect_success_port); \
    TEST_ADDR_PARSE_XFAIL(host_port_str); \
    TEST_ADDR_PORT_PARSE_XFAIL(host_port_str,  -1); \
    TEST_ADDR_PORT_PARSE_XFAIL(host_port_str, 888); \
    TEST_ADDR_V4_LOOKUP_XFAIL(host_port_str); \
    TEST_ADDR_LOOKUP_XFAIL(host_port_str, AF_UNSPEC); \
    TEST_ADDR_LOOKUP_XFAIL(host_port_str, AF_INET); \
    TEST_ADDR_LOOKUP_XFAIL(host_port_str, AF_INET6); \
  STMT_END

static void
test_addr_parse_canonical(void *arg)
{
  int r;
  tor_addr_t addr;
  uint16_t port;
  const char *sv;
  uint32_t addr32h;
  char buf[TOR_ADDR_BUF_LEN];

  (void)arg;

  /* Correct calls. */
  TEST_ADDR_V4_PARSE_CANONICAL("192.0.2.1");
  TEST_ADDR_V4_PARSE_CANONICAL("192.0.2.2");

  TEST_ADDR_V6_PARSE_CANONICAL("[11:22::33:44]", 1);
  TEST_ADDR_V6_PARSE_CANONICAL("[::1]", 1);
  TEST_ADDR_V6_PARSE_CANONICAL("[::]", 1);
  TEST_ADDR_V6_PARSE_CANONICAL("[2::]", 1);
  TEST_ADDR_V6_PARSE_CANONICAL("[11:22:33:44:55:66:77:88]", 1);

  /* Allow IPv6 without square brackets, when there is no port, but only if
   * there is a default port */
  TEST_ADDR_V6_PARSE_CANONICAL("11:22::33:44", 0);
  TEST_ADDR_V6_PARSE_CANONICAL("::1", 0);
  TEST_ADDR_V6_PARSE_CANONICAL("::", 0);
  TEST_ADDR_V6_PARSE_CANONICAL("2::", 0);
  TEST_ADDR_V6_PARSE_CANONICAL("11:22:33:44:55:66:77:88", 0);
 done:
  ;
}

/** Test tor_addr_parse() and tor_addr_port_parse(). */
static void
test_addr_parse(void *arg)
{

  int r;
  tor_addr_t addr;
  uint16_t port;
  const char *sv;
  uint32_t addr32h;
  char buf[TOR_ADDR_BUF_LEN];

  (void)arg;

  mock_hostname_resolver();

  /* IPv6-mapped IPv4 addresses. Tor doesn't really use these. */
  TEST_ADDR_V6_PARSE("11:22:33:44:55:66:1.2.3.4", 0,
                     "11:22:33:44:55:66:102:304");

  TEST_ADDR_V6_PARSE("11:22::33:44:1.2.3.4", 0,
                     "11:22::33:44:102:304");

  /* Ports. */
  TEST_ADDR_V4_PORT_PARSE("192.0.2.1:1234", "192.0.2.1", 1234);
  TEST_ADDR_V6_PORT_PARSE("[::1]:1234", "::1", 1234);

  /* Host names. */
  TEST_HOSTNAME("localhost");
  TEST_HOSTNAME_PORT("localhost:1234", 1234);
  TEST_HOSTNAME_PORT("localhost:0", 0);

  TEST_HOSTNAME("torproject.org");
  TEST_HOSTNAME_PORT("torproject.org:56", 56);

  TEST_HOSTNAME("probably-not-a-valid-dns.name-tld");
  TEST_HOSTNAME_PORT("probably-not-a-valid-dns.name-tld:789", 789);

  /* Malformed addresses. */
  /* Empty string. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("");

  /* Square brackets around IPv4 address. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[192.0.2.1]");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[192.0.2.3]:12345");

  /* Only left square bracket. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[11:22::33:44");

  /* Only right square bracket. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("11:22::33:44]");

  /* Leading colon. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED(":11:22::33:44");

  /* Trailing colon. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("11:22::33:44:");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[::1]:");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("localhost:");

  /* Bad port. */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("192.0.2.2:66666");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[::1]:77777");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("::1:88888");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("localhost:99999");

  TEST_ADDR_PARSE_XFAIL_MALFORMED("192.0.2.2:-1");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[::1]:-2");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("::1:-3");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("localhost:-4");

  TEST_ADDR_PARSE_XFAIL_MALFORMED("192.0.2.2:1 bad");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("192.0.2.2:bad-port");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("[::1]:bad-port-1");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("::1:1-bad-port");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("localhost:1-bad-port");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("localhost:1-bad-port-1");

  /* Bad hostname */
  TEST_ADDR_PARSE_XFAIL_MALFORMED("definitely invalid");
  TEST_ADDR_PARSE_XFAIL_MALFORMED("definitely invalid:22222");

  /* Ambiguous cases */
  /* Too many hex words in IPv4-mapped IPv6 address.
   * But some OS host lookup routines accept it as a hostname, or
   * as an IP address?? (I assume they discard unused characters). */
  TEST_HOSTNAME("11:22:33:44:55:66:77:88:1.2.3.4");

  /* IPv6 address with port and no brackets
   * We reject it, but some OS host lookup routines accept it as an
   * IPv6 address:port ? */
  TEST_HOSTNAME_PORT("11:22::33:44:12345", 12345);
  /* Is it a port, or are there too many hex words?
   * We reject it either way, but some OS host lookup routines accept it as an
   * IPv6 address:port */
  TEST_HOSTNAME_PORT("11:22:33:44:55:66:77:88:99", 99);
  /* But we accept it if it has square brackets. */
  TEST_ADDR_V6_PORT_PARSE("[11:22:33:44:55:66:77:88]:99",
                           "11:22:33:44:55:66:77:88",99);

  /* Bad IPv4 address
   * We reject it, but some OS host lookup routines accept it as an
   * IPv4 address[:port], with a zero last octet */
  TEST_HOSTNAME("192.0.1");
  TEST_HOSTNAME_PORT("192.0.2:1234", 1234);

  /* More bad IPv6 addresses and ports: no brackets
   * We reject it, but some OS host lookup routines accept it as an
   * IPv6 address[:port] */
  TEST_HOSTNAME_PORT("::1:12345", 12345);
  TEST_HOSTNAME_PORT("11:22::33:44:12345", 12345);

  /* And this is an ambiguous case, which is interpreted as an IPv6 address. */
  TEST_ADDR_V6_PARSE_CANONICAL("11:22::88:99", 0);
  /* Use square brackets to resolve the ambiguity */
  TEST_ADDR_V6_PARSE_CANONICAL("[11:22::88:99]", 1);
  TEST_ADDR_V6_PORT_PARSE("[11:22::88]:99",
                           "11:22::88",99);

 done:
  unmock_hostname_resolver();
}

static void
update_difference(int ipv6, uint8_t *d,
                  const tor_addr_t *a, const tor_addr_t *b)
{
  const int n_bytes = ipv6 ? 16 : 4;
  uint8_t a_tmp[4], b_tmp[4];
  const uint8_t *ba, *bb;
  int i;

  if (ipv6) {
    ba = tor_addr_to_in6_addr8(a);
    bb = tor_addr_to_in6_addr8(b);
  } else {
    set_uint32(a_tmp, tor_addr_to_ipv4n(a));
    set_uint32(b_tmp, tor_addr_to_ipv4n(b));
    ba = a_tmp; bb = b_tmp;
  }

  for (i = 0; i < n_bytes; ++i) {
    d[i] |= ba[i] ^ bb[i];
  }
}

static void
test_virtaddrmap(void *data)
{
  /* Let's start with a bunch of random addresses. */
  int ipv6, bits, iter, b;
  virtual_addr_conf_t cfg[2];
  uint8_t bytes[16];

  (void)data;

  tor_addr_parse(&cfg[0].addr, "64.65.0.0");
  tor_addr_parse(&cfg[1].addr, "3491:c0c0::");

  for (ipv6 = 0; ipv6 <= 1; ++ipv6) {
    for (bits = 0; bits < 18; ++bits) {
      tor_addr_t last_a;
      cfg[ipv6].bits = bits;
      memset(bytes, 0, sizeof(bytes));
      tor_addr_copy(&last_a, &cfg[ipv6].addr);
      /* Generate 128 addresses with each addr/bits combination. */
      for (iter = 0; iter < 128; ++iter) {
        tor_addr_t a;

        get_random_virtual_addr(&cfg[ipv6], &a);
        //printf("%s\n", fmt_addr(&a));
        /* Make sure that the first b bits match the configured network */
        tt_int_op(0, OP_EQ, tor_addr_compare_masked(&a, &cfg[ipv6].addr,
                                                 bits, CMP_EXACT));

        /* And track which bits have been different between pairs of
         * addresses */
        update_difference(ipv6, bytes, &last_a, &a);
      }

      /* Now make sure all but the first 'bits' bits of bytes are true */
      for (b = bits+1; b < (ipv6?128:32); ++b) {
        tt_assert(1 & (bytes[b/8] >> (7-(b&7))));
      }
    }
  }

 done:
  ;
}

static void
test_virtaddrmap_persist(void *data)
{
  (void)data;
  const char *a, *b, *c;
  tor_addr_t addr;
  char *ones = NULL;
  const char *canned_data;
  size_t canned_data_len;

  addressmap_init();

  // Try a hostname.
  a = addressmap_register_virtual_address(RESOLVED_TYPE_HOSTNAME,
                                          tor_strdup("foobar.baz"));
  tt_assert(a);
  tt_assert(!strcmpend(a, ".virtual"));

  // mock crypto_rand to repeat the same result twice; make sure we get
  // different outcomes.  (Because even though the odds for receiving the
  // same 80-bit address twice is only 1/2^40, it could still happen for
  // some user -- but running our test through 2^40 iterations isn't
  // reasonable.)
  canned_data = "1234567890" // the first call returns this.
                "1234567890" // the second call returns this.
                "abcdefghij"; // the third call returns this.
  canned_data_len = 30;
  testing_enable_prefilled_rng(canned_data, canned_data_len);

  a = addressmap_register_virtual_address(RESOLVED_TYPE_HOSTNAME,
                                          tor_strdup("quuxit.baz"));
  b = addressmap_register_virtual_address(RESOLVED_TYPE_HOSTNAME,
                                          tor_strdup("nescio.baz"));
  tt_assert(a);
  tt_assert(b);
  tt_str_op(a, OP_EQ, "gezdgnbvgy3tqojq.virtual");
  tt_str_op(b, OP_EQ, "mfrggzdfmztwq2lk.virtual");
  testing_disable_prefilled_rng();

  // Now try something to get us an ipv4 address
  tt_int_op(0,OP_EQ, parse_virtual_addr_network("192.168.0.0/16",
                                                AF_INET, 0, NULL));
  a = addressmap_register_virtual_address(RESOLVED_TYPE_IPV4,
                                          tor_strdup("foobar.baz"));
  tt_assert(a);
  tt_assert(!strcmpstart(a, "192.168."));
  tor_addr_parse(&addr, a);
  tt_int_op(AF_INET, OP_EQ, tor_addr_family(&addr));

  b = addressmap_register_virtual_address(RESOLVED_TYPE_IPV4,
                                          tor_strdup("quuxit.baz"));
  tt_str_op(b, OP_NE, a);
  tt_assert(!strcmpstart(b, "192.168."));

  // Try some canned entropy and verify all the we discard duplicates,
  // addresses that end with 0, and addresses that end with 255.
  canned_data = "\x01\x02\x03\x04" // okay
                "\x01\x02\x03\x04" // duplicate
                "\x03\x04\x00\x00" // bad ending 1
                "\x05\x05\x00\xff" // bad ending 2
                "\x05\x06\x07\xf0"; // okay
  canned_data_len = 20;
  testing_enable_prefilled_rng(canned_data, canned_data_len);

  a = addressmap_register_virtual_address(RESOLVED_TYPE_IPV4,
                                          tor_strdup("wumble.onion"));
  b = addressmap_register_virtual_address(RESOLVED_TYPE_IPV4,
                                          tor_strdup("wumpus.onion"));
  tt_str_op(a, OP_EQ, "192.168.3.4");
  tt_str_op(b, OP_EQ, "192.168.7.240");
  testing_disable_prefilled_rng();

  // Now try IPv6!
  tt_int_op(0,OP_EQ, parse_virtual_addr_network("1010:F000::/20",
                                                AF_INET6, 0, NULL));
  a = addressmap_register_virtual_address(RESOLVED_TYPE_IPV6,
                                          tor_strdup("foobar.baz"));
  tt_assert(a);
  tt_assert(!strcmpstart(a, "[1010:f"));
  tor_addr_parse(&addr, a);
  tt_int_op(AF_INET6, OP_EQ, tor_addr_family(&addr));

  b = addressmap_register_virtual_address(RESOLVED_TYPE_IPV6,
                                          tor_strdup("quuxit.baz"));
  tt_str_op(b, OP_NE, a);
  tt_assert(!strcmpstart(b, "[1010:f"));

  // Try IPv6 with canned entropy, to make sure we detect duplicates.

  canned_data = "acanthopterygian" // okay
                "cinematographist" // okay
                "acanthopterygian" // duplicate
                "acanthopterygian" // duplicate
                "acanthopterygian" // duplicate
                "cinematographist" // duplicate
                "coadministration"; // okay
  canned_data_len = 16 * 7;
  testing_enable_prefilled_rng(canned_data, canned_data_len);

  a = addressmap_register_virtual_address(RESOLVED_TYPE_IPV6,
                                          tor_strdup("wuffle.baz"));
  b = addressmap_register_virtual_address(RESOLVED_TYPE_IPV6,
                                          tor_strdup("gribble.baz"));
  c = addressmap_register_virtual_address(RESOLVED_TYPE_IPV6,
                                      tor_strdup("surprisingly-legible.baz"));
  tt_str_op(a, OP_EQ, "[1010:f16e:7468:6f70:7465:7279:6769:616e]");
  tt_str_op(b, OP_EQ, "[1010:fe65:6d61:746f:6772:6170:6869:7374]");
  tt_str_op(c, OP_EQ, "[1010:f164:6d69:6e69:7374:7261:7469:6f6e]");

  // Try address exhaustion: make sure we can actually fail if we
  // get too many already-existing addresses.
  testing_disable_prefilled_rng();
  canned_data_len = 128*1024;
  canned_data = ones = tor_malloc(canned_data_len);
  memset(ones, 1, canned_data_len);
  testing_enable_prefilled_rng(canned_data, canned_data_len);
  // There is some chance this one will fail if a previous random
  // allocation gave out the address already.
  a = addressmap_register_virtual_address(RESOLVED_TYPE_IPV4,
                                          tor_strdup("might-work.onion"));
  if (a) {
    tt_str_op(a, OP_EQ, "192.168.1.1");
  }
  setup_capture_of_logs(LOG_WARN);
  // This one will definitely fail, since we've set up the RNG to hand
  // out "1" forever.
  b = addressmap_register_virtual_address(RESOLVED_TYPE_IPV4,
                                          tor_strdup("wont-work.onion"));
  tt_assert(b == NULL);
  expect_single_log_msg_containing("Ran out of virtual addresses!");

 done:
  testing_disable_prefilled_rng();
  tor_free(ones);
  addressmap_free_all();
  teardown_capture_of_logs();
}

static void
test_addr_localname(void *arg)
{
  (void)arg;
  tt_assert(tor_addr_hostname_is_local("localhost"));
  tt_assert(tor_addr_hostname_is_local("LOCALHOST"));
  tt_assert(tor_addr_hostname_is_local("LocalHost"));
  tt_assert(tor_addr_hostname_is_local("local"));
  tt_assert(tor_addr_hostname_is_local("LOCAL"));
  tt_assert(tor_addr_hostname_is_local("here.now.local"));
  tt_assert(tor_addr_hostname_is_local("here.now.LOCAL"));

  tt_assert(!tor_addr_hostname_is_local(" localhost"));
  tt_assert(!tor_addr_hostname_is_local("www.torproject.org"));
 done:
  ;
}

static void
test_addr_dup_ip(void *arg)
{
  char *v = NULL;
  (void)arg;
#define CHECK(ip, s) do {                         \
    v = tor_dup_ip(ip);                           \
    tt_str_op(v,OP_EQ,(s));                          \
    tor_free(v);                                  \
  } while (0)

  CHECK(0xffffffff, "255.255.255.255");
  CHECK(0x00000000, "0.0.0.0");
  CHECK(0x7f000001, "127.0.0.1");
  CHECK(0x01020304, "1.2.3.4");

#undef CHECK
 done:
  tor_free(v);
}

static void
test_addr_sockaddr_to_str(void *arg)
{
  char *v = NULL;
  struct sockaddr_in sin;
  struct sockaddr_in6 sin6;
  struct sockaddr_storage ss;
#ifdef HAVE_SYS_UN_H
  struct sockaddr_un s_un;
#endif
#define CHECK(sa, s) do {                                       \
    v = tor_sockaddr_to_str((const struct sockaddr*) &(sa));    \
    tt_str_op(v,OP_EQ,(s));                                        \
    tor_free(v);                                                \
  } while (0)
  (void)arg;

  memset(&ss,0,sizeof(ss));
  ss.ss_family = AF_UNSPEC;
  CHECK(ss, "unspec");

  memset(&sin,0,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(0x7f808001);
  sin.sin_port = htons(1234);
  CHECK(sin, "127.128.128.1:1234");

#ifdef HAVE_SYS_UN_H
  memset(&s_un,0,sizeof(s_un));
  s_un.sun_family = AF_UNIX;
  strlcpy(s_un.sun_path, "/here/is/a/path", sizeof(s_un.sun_path));
  CHECK(s_un, "unix:/here/is/a/path");
#endif /* defined(HAVE_SYS_UN_H) */

  memset(&sin6,0,sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  memcpy(sin6.sin6_addr.s6_addr, "\x20\x00\x00\x00\x00\x00\x00\x00"
                                 "\x00\x1a\x2b\x3c\x4d\x5e\x00\x01", 16);
  sin6.sin6_port = htons(1234);
  CHECK(sin6, "[2000::1a:2b3c:4d5e:1]:1234");

 done:
  tor_free(v);
}

static void
test_addr_is_loopback(void *data)
{
  static const struct loopback_item {
    const char *name;
    int is_loopback;
  } loopback_items[] = {
    { "::1", 1 },
    { "127.0.0.1", 1 },
    { "127.99.100.101", 1 },
    { "128.99.100.101", 0 },
    { "8.8.8.8", 0 },
    { "0.0.0.0", 0 },
    { "::2", 0 },
    { "::", 0 },
    { "::1.0.0.0", 0 },
    { NULL, 0 }
  };

  int i;
  tor_addr_t addr;
  (void)data;

  for (i=0; loopback_items[i].name; ++i) {
    tt_int_op(tor_addr_parse(&addr, loopback_items[i].name), OP_GE, 0);
    tt_int_op(tor_addr_is_loopback(&addr), OP_EQ,
              loopback_items[i].is_loopback);
  }

  tor_addr_make_unspec(&addr);
  tt_int_op(tor_addr_is_loopback(&addr), OP_EQ, 0);

 done:
  ;
}

static void
test_addr_make_null(void *data)
{
  tor_addr_t *addr = tor_malloc(sizeof(*addr));
  tor_addr_t *zeros = tor_malloc_zero(sizeof(*addr));
  char buf[TOR_ADDR_BUF_LEN];
  (void) data;
  /* Ensure that before tor_addr_make_null, addr != 0's */
  memset(addr, 1, sizeof(*addr));
  tt_int_op(fast_memcmp(addr, zeros, sizeof(*addr)), OP_NE, 0);
  /* Test with AF == AF_INET */
  zeros->family = AF_INET;
  tor_addr_make_null(addr, AF_INET);
  tt_int_op(fast_memcmp(addr, zeros, sizeof(*addr)), OP_EQ, 0);
  tt_str_op(tor_addr_to_str(buf, addr, sizeof(buf), 0), OP_EQ, "0.0.0.0");
  /* Test with AF == AF_INET6 */
  memset(addr, 1, sizeof(*addr));
  zeros->family = AF_INET6;
  tor_addr_make_null(addr, AF_INET6);
  tt_int_op(fast_memcmp(addr, zeros, sizeof(*addr)), OP_EQ, 0);
  tt_str_op(tor_addr_to_str(buf, addr, sizeof(buf), 0), OP_EQ, "::");
 done:
  tor_free(addr);
  tor_free(zeros);
}

#define TEST_ADDR_INTERNAL(a, for_listening, rv) STMT_BEGIN \
    tor_addr_t t; \
    tt_int_op(tor_inet_pton(AF_INET, a, &t.addr.in_addr), OP_EQ, 1); \
    t.family = AF_INET; \
    tt_int_op(tor_addr_is_internal(&t, for_listening), OP_EQ, rv); \
  STMT_END;

static void
test_addr_rfc6598(void *arg)
{
  (void)arg;
  TEST_ADDR_INTERNAL("100.64.0.1", 0, 1);
  TEST_ADDR_INTERNAL("100.64.0.1", 1, 0);
 done:
  ;
}

#define TEST_ADDR_ATON(a, rv) STMT_BEGIN \
    struct in_addr addr; \
    tt_int_op(tor_inet_aton(a, &addr), OP_EQ, rv); \
  STMT_END;

static void
test_addr_octal(void *arg)
{
  (void)arg;

  /* Test non-octal IP addresses. */
  TEST_ADDR_ATON("0.1.2.3", 1);
  TEST_ADDR_ATON("1.0.2.3", 1);
  TEST_ADDR_ATON("1.2.3.0", 1);

  /* Test octal IP addresses. */
  TEST_ADDR_ATON("01.1.2.3", 0);
  TEST_ADDR_ATON("1.02.3.4", 0);
  TEST_ADDR_ATON("1.2.3.04", 0);
 done:
  ;
}

#define get_ipv4(test_addr, str, iprv) STMT_BEGIN               \
    test_addr = tor_malloc(sizeof(tor_addr_t));                 \
    test_addr->family = AF_INET;                                \
    iprv = tor_inet_aton(str, &test_addr->addr.in_addr);        \
    tor_assert(iprv);                                           \
  STMT_END;

#define get_ipv6(test_addr, str, iprv) STMT_BEGIN                       \
    test_addr = tor_malloc(sizeof(tor_addr_t));                         \
    test_addr->family = AF_INET6;                                       \
    iprv = tor_inet_pton(AF_INET6, str, &test_addr->addr.in6_addr);     \
    tor_assert(iprv);                                                   \
  STMT_END;

#define get_af_unix(test_addr) STMT_BEGIN                       \
    test_addr = tor_malloc_zero(sizeof(tor_addr_t));            \
    test_addr->family = AF_UNIX;                                \
  STMT_END;

#define get_af_unspec(test_addr) STMT_BEGIN                     \
    test_addr = tor_malloc_zero(sizeof(tor_addr_t));            \
    test_addr->family = AF_UNSPEC;                              \
  STMT_END;

#define TEST_ADDR_VALIDITY(a, lis, rv) STMT_BEGIN               \
    tor_assert(a);                                              \
    tt_int_op(tor_addr_is_valid(a, lis), OP_EQ, rv);            \
  STMT_END;

/* Here we can change the addresses we are testing for. */
#define IP4_TEST_ADDR "123.98.45.1"
#define IP6_TEST_ADDR "2001:0DB8:AC10:FE01::"

static void
test_addr_is_valid(void *arg)
{
  (void)arg;
  tor_addr_t *test_addr;
  int iprv;

  /* Tests for IPv4 addresses. */

  /* Test for null IPv4 address. */
  get_ipv4(test_addr, "0.0.0.0", iprv);
  TEST_ADDR_VALIDITY(test_addr, 0, 0);
  TEST_ADDR_VALIDITY(test_addr, 1, 1);
  tor_free(test_addr);

  /* Test for non-null IPv4 address. */
  get_ipv4(test_addr, IP4_TEST_ADDR, iprv);
  TEST_ADDR_VALIDITY(test_addr, 0, 1);
  TEST_ADDR_VALIDITY(test_addr, 1, 1);
  tor_free(test_addr);

  /* Tests for IPv6 addresses. */

  /* Test for null IPv6 address. */
  get_ipv6(test_addr, "::", iprv);
  TEST_ADDR_VALIDITY(test_addr, 0, 0);
  TEST_ADDR_VALIDITY(test_addr, 1, 1);
  tor_free(test_addr);

  /* Test for non-null IPv6 address. */
  get_ipv6(test_addr, IP6_TEST_ADDR, iprv);
  TEST_ADDR_VALIDITY(test_addr, 0, 1);
  TEST_ADDR_VALIDITY(test_addr, 1, 1);
  tor_free(test_addr);

  /* Test for address of type AF_UNIX. */

  get_af_unix(test_addr);
  TEST_ADDR_VALIDITY(test_addr, 0, 0);
  TEST_ADDR_VALIDITY(test_addr, 1, 0);
  tor_free(test_addr);

  /* Test for address of type AF_UNSPEC. */

  get_af_unspec(test_addr);
  TEST_ADDR_VALIDITY(test_addr, 0, 0);
  TEST_ADDR_VALIDITY(test_addr, 1, 0);

 done:
  tor_free(test_addr);
}

#define TEST_ADDR_IS_NULL(a, rv) STMT_BEGIN                 \
    tor_assert(a);                                          \
    tt_int_op(tor_addr_is_null(a), OP_EQ, rv);              \
  STMT_END;

static void
test_addr_is_null(void *arg)
{
  (void)arg;
  tor_addr_t *test_addr;
  int iprv;

  /* Test for null IPv4. */
  get_ipv4(test_addr, "0.0.0.0", iprv);
  TEST_ADDR_IS_NULL(test_addr, 1);
  tor_free(test_addr);

  /* Test for non-null IPv4. */
  get_ipv4(test_addr, IP4_TEST_ADDR, iprv);
  TEST_ADDR_IS_NULL(test_addr, 0);
  tor_free(test_addr);

  /* Test for null IPv6. */
  get_ipv6(test_addr, "::", iprv);
  TEST_ADDR_IS_NULL(test_addr, 1);
  tor_free(test_addr);

  /* Test for non-null IPv6. */
  get_ipv6(test_addr, IP6_TEST_ADDR, iprv);
  TEST_ADDR_IS_NULL(test_addr, 0);
  tor_free(test_addr);

  /* Test for address family AF_UNIX. */
  get_af_unix(test_addr);
  TEST_ADDR_IS_NULL(test_addr, 1);
  tor_free(test_addr);

  /* Test for address family AF_UNSPEC. */
  get_af_unspec(test_addr);
  TEST_ADDR_IS_NULL(test_addr, 1);

 done:
  tor_free(test_addr);
}

#ifndef COCCI
#define ADDR_LEGACY(name)                                               \
  { #name, test_addr_ ## name , 0, NULL, NULL }
#endif

struct testcase_t addr_tests[] = {
  ADDR_LEGACY(basic),
  ADDR_LEGACY(ip6_helpers),
  ADDR_LEGACY(parse),
  ADDR_LEGACY(parse_canonical),
  { "virtaddr", test_virtaddrmap, 0, NULL, NULL },
  { "virtaddr_persist", test_virtaddrmap_persist, TT_FORK, NULL, NULL },
  { "localname", test_addr_localname, 0, NULL, NULL },
  { "dup_ip", test_addr_dup_ip, 0, NULL, NULL },
  { "sockaddr_to_str", test_addr_sockaddr_to_str, 0, NULL, NULL },
  { "is_loopback", test_addr_is_loopback, 0, NULL, NULL },
  { "make_null", test_addr_make_null, 0, NULL, NULL },
  { "rfc6598", test_addr_rfc6598, 0, NULL, NULL },
  { "octal", test_addr_octal, 0, NULL, NULL },
  { "address_validity", test_addr_is_valid, 0, NULL, NULL },
  { "address_is_null", test_addr_is_null, 0, NULL, NULL },
  END_OF_TESTCASES
};
