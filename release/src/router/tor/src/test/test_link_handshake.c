/* Copyright (c) 2014-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define CHANNELTLS_PRIVATE
#define CONNECTION_PRIVATE
#define CHANNEL_OBJECT_PRIVATE
#define TORTLS_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_or.h"
#include "core/or/channeltls.h"
#include "trunnel/link_handshake.h"
#include "feature/relay/router.h"
#include "feature/relay/routerkeys.h"
#include "core/or/scheduler.h"
#include "feature/nodelist/torcert.h"
#include "feature/relay/relay_handshake.h"

#include "core/or/or_connection_st.h"
#include "core/or/or_handshake_certs_st.h"
#include "core/or/or_handshake_state_st.h"
#include "core/or/var_cell_st.h"

#define TOR_X509_PRIVATE
#include "lib/tls/tortls.h"
#include "lib/tls/x509.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

static var_cell_t *mock_got_var_cell = NULL;

static void
mock_write_var_cell(const var_cell_t *vc, or_connection_t *conn)
{
  (void)conn;

  var_cell_t *newcell = var_cell_new(vc->payload_len);
  memcpy(newcell, vc, sizeof(var_cell_t));
  memcpy(newcell->payload, vc->payload, vc->payload_len);

  mock_got_var_cell = newcell;
}
static int
mock_tls_cert_matches_key(const tor_tls_t *tls, const tor_x509_cert_t *cert)
{
  (void) tls;
  (void) cert; // XXXX look at this.
  return 1;
}
static tor_tls_t *mock_peer_cert_expect_tortls = NULL;
static tor_x509_cert_t *mock_peer_cert = NULL;
static tor_x509_cert_t *
mock_get_peer_cert(tor_tls_t *tls)
{
  if (mock_peer_cert_expect_tortls &&
      mock_peer_cert_expect_tortls != tls)
    return NULL;
  return tor_x509_cert_dup(mock_peer_cert);
}

static int mock_send_netinfo_called = 0;
static int
mock_send_netinfo(or_connection_t *conn)
{
  (void) conn;
  ++mock_send_netinfo_called;// XXX check_this
  return 0;
}

static int mock_close_called = 0;
static void
mock_close_for_err(or_connection_t *orconn, int flush)
{
  (void)orconn;
  (void)flush;
  ++mock_close_called;
}

static int mock_send_authenticate_called = 0;
static int mock_send_authenticate_called_with_type = 0;
static int
mock_send_authenticate(or_connection_t *conn, int type)
{
  (void) conn;
  mock_send_authenticate_called_with_type = type;
  ++mock_send_authenticate_called;// XXX check_this
  return 0;
}
static int
mock_export_key_material(tor_tls_t *tls, uint8_t *secrets_out,
                         const uint8_t *context,
                         size_t context_len,
                         const char *label)
{
  (void) tls;
  (void)secrets_out;
  (void)context;
  (void)context_len;
  (void)label;
  memcpy(secrets_out, "int getRandomNumber(){return 4;}", 32);
  return 0;
}

static tor_x509_cert_t *mock_own_cert = NULL;
static tor_x509_cert_t *
mock_get_own_cert(tor_tls_t *tls)
{
  (void)tls;
  return tor_x509_cert_dup(mock_own_cert);
}

/* Test good certs cells */
static void
test_link_handshake_certs_ok(void *arg)
{
  or_connection_t *c1 = or_connection_new(CONN_TYPE_OR, AF_INET);
  or_connection_t *c2 = or_connection_new(CONN_TYPE_OR, AF_INET);
  var_cell_t *cell1 = NULL, *cell2 = NULL;
  certs_cell_t *cc1 = NULL, *cc2 = NULL;
  channel_tls_t *chan1 = NULL, *chan2 = NULL;
  crypto_pk_t *key1 = NULL, *key2 = NULL;
  const int with_ed = !strcmp((const char *)arg, "Ed25519");

  tor_addr_from_ipv4h(&c1->base_.addr, 0x7f000001);
  tor_addr_from_ipv4h(&c2->base_.addr, 0x7f000001);

  scheduler_init();

  MOCK(tor_tls_cert_matches_key, mock_tls_cert_matches_key);
  MOCK(connection_or_write_var_cell_to_buf, mock_write_var_cell);
  MOCK(connection_or_send_netinfo, mock_send_netinfo);
  MOCK(tor_tls_get_peer_cert, mock_get_peer_cert);
  MOCK(tor_tls_get_own_cert, mock_get_own_cert);

  key1 = pk_generate(2);
  key2 = pk_generate(3);

  /* We need to make sure that our TLS certificates are set up before we can
   * actually generate a CERTS cell.
   */
  tt_int_op(tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                                 key1, key2, 86400), OP_EQ, 0);

  if (with_ed) {
    /* If we're making a CERTS cell for an ed handshake, let's make sure we
     * have some Ed25519 certificates and keys. */
    init_mock_ed_keys(key2);
  } else {
    certs_cell_ed25519_disabled_for_testing = 1;
  }

  /* c1 has started_here == 1 */
  {
    const tor_x509_cert_t *link_cert = NULL;
    tt_assert(!tor_tls_get_my_certs(1, &link_cert, NULL));
    mock_own_cert = tor_x509_cert_dup(link_cert);
  }

  c1->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  c1->link_proto = 3;
  tt_int_op(connection_init_or_handshake_state(c1, 1), OP_EQ, 0);

  /* c2 has started_here == 0 */
  c2->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  c2->link_proto = 3;
  tt_int_op(connection_init_or_handshake_state(c2, 0), OP_EQ, 0);

  tt_int_op(0, OP_EQ, connection_or_send_certs_cell(c1));
  tt_assert(mock_got_var_cell);
  cell1 = mock_got_var_cell;

  tt_int_op(0, OP_EQ, connection_or_send_certs_cell(c2));
  tt_assert(mock_got_var_cell);
  cell2 = mock_got_var_cell;

  tt_int_op(cell1->command, OP_EQ, CELL_CERTS);
  tt_int_op(cell1->payload_len, OP_GT, 1);

  tt_int_op(cell2->command, OP_EQ, CELL_CERTS);
  tt_int_op(cell2->payload_len, OP_GT, 1);

  tt_int_op(cell1->payload_len, OP_EQ,
            certs_cell_parse(&cc1, cell1->payload, cell1->payload_len));
  tt_int_op(cell2->payload_len, OP_EQ,
            certs_cell_parse(&cc2, cell2->payload, cell2->payload_len));

  if (with_ed) {
    tt_int_op(5, OP_EQ, cc1->n_certs);
    tt_int_op(5, OP_EQ, cc2->n_certs);
  } else {
    tt_int_op(2, OP_EQ, cc1->n_certs);
    tt_int_op(2, OP_EQ, cc2->n_certs);
  }

  tt_int_op(certs_cell_get_certs(cc1, 0)->cert_type, OP_EQ,
            CERTTYPE_RSA1024_ID_AUTH);
  tt_int_op(certs_cell_get_certs(cc1, 1)->cert_type, OP_EQ,
            CERTTYPE_RSA1024_ID_ID);

  tt_int_op(certs_cell_get_certs(cc2, 0)->cert_type, OP_EQ,
            CERTTYPE_RSA1024_ID_LINK);
  tt_int_op(certs_cell_get_certs(cc2, 1)->cert_type, OP_EQ,
            CERTTYPE_RSA1024_ID_ID);

  if (with_ed) {
    tt_int_op(certs_cell_get_certs(cc1, 2)->cert_type, OP_EQ,
              CERTTYPE_ED_ID_SIGN);
    tt_int_op(certs_cell_get_certs(cc1, 3)->cert_type, OP_EQ,
              CERTTYPE_ED_SIGN_AUTH);
    tt_int_op(certs_cell_get_certs(cc1, 4)->cert_type, OP_EQ,
              CERTTYPE_RSA1024_ID_EDID);

    tt_int_op(certs_cell_get_certs(cc2, 2)->cert_type, OP_EQ,
              CERTTYPE_ED_ID_SIGN);
    tt_int_op(certs_cell_get_certs(cc2, 3)->cert_type, OP_EQ,
              CERTTYPE_ED_SIGN_LINK);
    tt_int_op(certs_cell_get_certs(cc2, 4)->cert_type, OP_EQ,
              CERTTYPE_RSA1024_ID_EDID);
  }

  chan1 = tor_malloc_zero(sizeof(*chan1));
  channel_tls_common_init(chan1);
  c1->chan = chan1;
  chan1->conn = c1;
  c1->base_.address = tor_strdup("C1");
  c1->tls = tor_tls_new(-1, 0);
  c1->link_proto = 4;
  c1->base_.conn_array_index = -1;
  crypto_pk_get_digest(key2, c1->identity_digest);

  if (with_ed) {
    const tor_x509_cert_t *linkc, *idc;
    tor_tls_get_my_certs(1, &linkc, &idc);
    mock_peer_cert_expect_tortls = c1->tls; /* We should see this tls... */
    mock_peer_cert = tor_x509_cert_dup(linkc); /* and when we do, the peer's
                                                *  cert is this... */
  }
  channel_tls_process_certs_cell(cell2, chan1);
  mock_peer_cert_expect_tortls = NULL;
  tor_x509_cert_free(mock_peer_cert);
  mock_peer_cert = NULL;

  tor_assert(c1->handshake_state->authenticated);

  tt_assert(c1->handshake_state->received_certs_cell);
  tt_ptr_op(c1->handshake_state->certs->auth_cert, OP_EQ, NULL);
  tt_ptr_op(c1->handshake_state->certs->ed_sign_auth, OP_EQ, NULL);
  tt_assert(c1->handshake_state->certs->id_cert);
  if (with_ed) {
    tt_assert(c1->handshake_state->certs->ed_sign_link);
    tt_assert(c1->handshake_state->certs->ed_rsa_crosscert);
    tt_assert(c1->handshake_state->certs->ed_id_sign);
    tt_assert(c1->handshake_state->authenticated_rsa);
    tt_assert(c1->handshake_state->authenticated_ed25519);
  } else {
    tt_ptr_op(c1->handshake_state->certs->ed_sign_link, OP_EQ, NULL);
    tt_ptr_op(c1->handshake_state->certs->ed_rsa_crosscert, OP_EQ, NULL);
    tt_ptr_op(c1->handshake_state->certs->ed_id_sign, OP_EQ, NULL);
    tt_assert(c1->handshake_state->authenticated_rsa);
    tt_assert(! c1->handshake_state->authenticated_ed25519);
  }
  tt_assert(! fast_mem_is_zero(
                (char*)c1->handshake_state->authenticated_rsa_peer_id, 20));

  chan2 = tor_malloc_zero(sizeof(*chan2));
  channel_tls_common_init(chan2);
  c2->chan = chan2;
  chan2->conn = c2;
  c2->base_.address = tor_strdup("C2");
  c2->tls = tor_tls_new(-1, 1);
  c2->link_proto = 4;
  c2->base_.conn_array_index = -1;
  crypto_pk_get_digest(key1, c2->identity_digest);

  channel_tls_process_certs_cell(cell1, chan2);

  tt_assert(c2->handshake_state->received_certs_cell);
  if (with_ed) {
    tt_assert(c2->handshake_state->certs->ed_sign_auth);
    tt_assert(c2->handshake_state->certs->ed_rsa_crosscert);
    tt_assert(c2->handshake_state->certs->ed_id_sign);
  } else {
    tt_assert(c2->handshake_state->certs->auth_cert);
    tt_ptr_op(c2->handshake_state->certs->ed_sign_auth, OP_EQ, NULL);
    tt_ptr_op(c2->handshake_state->certs->ed_rsa_crosscert, OP_EQ, NULL);
    tt_ptr_op(c2->handshake_state->certs->ed_id_sign, OP_EQ, NULL);
  }
  tt_assert(c2->handshake_state->certs->id_cert);
  tt_assert(fast_mem_is_zero(
              (char*)c2->handshake_state->authenticated_rsa_peer_id, 20));
  /* no authentication has happened yet, since we haen't gotten an AUTH cell.
   */
  tt_assert(! c2->handshake_state->authenticated);
  tt_assert(! c2->handshake_state->authenticated_rsa);
  tt_assert(! c2->handshake_state->authenticated_ed25519);

 done:
  UNMOCK(tor_tls_cert_matches_key);
  UNMOCK(connection_or_write_var_cell_to_buf);
  UNMOCK(connection_or_send_netinfo);
  UNMOCK(tor_tls_get_peer_cert);
  UNMOCK(tor_tls_get_own_cert);
  tor_x509_cert_free(mock_own_cert);
  tor_x509_cert_free(mock_peer_cert);
  mock_own_cert = mock_peer_cert = NULL;
  memset(c1->identity_digest, 0, sizeof(c1->identity_digest));
  memset(c2->identity_digest, 0, sizeof(c2->identity_digest));
  connection_free_minimal(TO_CONN(c1));
  connection_free_minimal(TO_CONN(c2));
  tor_free(cell1);
  tor_free(cell2);
  certs_cell_free(cc1);
  certs_cell_free(cc2);
  if (chan1)
    circuitmux_free(chan1->base_.cmux);
  tor_free(chan1);
  if (chan2)
    circuitmux_free(chan2->base_.cmux);
  tor_free(chan2);
  crypto_pk_free(key1);
  crypto_pk_free(key2);
}

typedef struct certs_data_t {
  int is_ed;
  int is_link_cert;
  or_connection_t *c;
  channel_tls_t *chan;
  certs_cell_t *ccell;
  var_cell_t *cell;
  crypto_pk_t *key1, *key2;
} certs_data_t;

static int
recv_certs_cleanup(const struct testcase_t *test, void *obj)
{
  (void)test;
  certs_data_t *d = obj;
  UNMOCK(tor_tls_cert_matches_key);
  UNMOCK(connection_or_send_netinfo);
  UNMOCK(connection_or_close_for_error);
  UNMOCK(tor_tls_get_peer_cert);
  UNMOCK(tor_tls_get_own_cert);

  if (d) {
    tor_free(d->cell);
    certs_cell_free(d->ccell);
    connection_or_clear_identity(d->c);
    connection_free_minimal(TO_CONN(d->c));
    circuitmux_free(d->chan->base_.cmux);
    tor_free(d->chan);
    crypto_pk_free(d->key1);
    crypto_pk_free(d->key2);
    tor_free(d);
  }
  routerkeys_free_all();
  return 1;
}

static void *
recv_certs_setup(const struct testcase_t *test)
{
  (void)test;
  certs_data_t *d = tor_malloc_zero(sizeof(*d));
  certs_cell_cert_t *ccc1 = NULL;
  certs_cell_cert_t *ccc2 = NULL;
  ssize_t n;
  int is_ed = d->is_ed = !strcmpstart(test->setup_data, "Ed25519");
  int is_rsa = !strcmpstart(test->setup_data, "RSA");
  int is_link = d->is_link_cert = !strcmpend(test->setup_data, "-Link");
  int is_auth = !strcmpend(test->setup_data, "-Auth");
  tor_assert(is_ed != is_rsa);
  tor_assert(is_link != is_auth);

  d->c = or_connection_new(CONN_TYPE_OR, AF_INET);
  d->chan = tor_malloc_zero(sizeof(*d->chan));
  d->c->chan = d->chan;
  d->c->base_.address = tor_strdup("HaveAnAddress");
  tor_addr_from_ipv4h(&d->c->base_.addr, 0x801f0127);
  d->c->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  d->chan->conn = d->c;
  tt_int_op(connection_init_or_handshake_state(d->c, 1), OP_EQ, 0);
  d->c->link_proto = 4;

  d->key1 = pk_generate(2);
  d->key2 = pk_generate(3);

  tt_int_op(tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                                 d->key1, d->key2, 86400), OP_EQ, 0);
  if (is_ed) {
    init_mock_ed_keys(d->key2);
  } else {
    routerkeys_free_all();
  }

  d->ccell = certs_cell_new();
  ccc1 = certs_cell_cert_new();
  certs_cell_add_certs(d->ccell, ccc1);
  ccc2 = certs_cell_cert_new();
  certs_cell_add_certs(d->ccell, ccc2);
  d->ccell->n_certs = 2;
  ccc1->cert_type = is_link ? 1 : 3;
  ccc2->cert_type = 2;

  const tor_x509_cert_t *a,*b;
  const uint8_t *enca, *encb;
  size_t lena, lenb;
  tor_tls_get_my_certs(is_link ? 1 : 0, &a, &b);
  tor_x509_cert_get_der(a, &enca, &lena);
  tor_x509_cert_get_der(b, &encb, &lenb);
  certs_cell_cert_setlen_body(ccc1, lena);
  ccc1->cert_len = lena;
  certs_cell_cert_setlen_body(ccc2, lenb);
  ccc2->cert_len = lenb;

  memcpy(certs_cell_cert_getarray_body(ccc1), enca, lena);
  memcpy(certs_cell_cert_getarray_body(ccc2), encb, lenb);

  if (is_ed) {
    certs_cell_cert_t *ccc3 = NULL; /* Id->Sign */
    certs_cell_cert_t *ccc4 = NULL; /* Sign->Link or Sign->Auth. */
    certs_cell_cert_t *ccc5 = NULL; /* RSAId->Ed Id. */
    const tor_cert_t *id_sign = get_master_signing_key_cert();
    const tor_cert_t *secondary =
      is_link ? get_current_link_cert_cert() : get_current_auth_key_cert();
    const uint8_t *cc = NULL;
    size_t cc_sz;
    get_master_rsa_crosscert(&cc, &cc_sz);

    ccc3 = certs_cell_cert_new();
    ccc4 = certs_cell_cert_new();
    ccc5 = certs_cell_cert_new();
    certs_cell_add_certs(d->ccell, ccc3);
    certs_cell_add_certs(d->ccell, ccc4);
    certs_cell_add_certs(d->ccell, ccc5);
    ccc3->cert_len = id_sign->encoded_len;
    ccc4->cert_len = secondary->encoded_len;
    ccc5->cert_len = cc_sz;
    certs_cell_cert_setlen_body(ccc3, ccc3->cert_len);
    certs_cell_cert_setlen_body(ccc4, ccc4->cert_len);
    certs_cell_cert_setlen_body(ccc5, ccc5->cert_len);
    memcpy(certs_cell_cert_getarray_body(ccc3), id_sign->encoded,
           ccc3->cert_len);
    memcpy(certs_cell_cert_getarray_body(ccc4), secondary->encoded,
           ccc4->cert_len);
    memcpy(certs_cell_cert_getarray_body(ccc5), cc, ccc5->cert_len);
    ccc3->cert_type = 4;
    ccc4->cert_type = is_link ? 5 : 6;
    ccc5->cert_type = 7;

    d->ccell->n_certs = 5;
  }

  d->cell = var_cell_new(4096);
  d->cell->command = CELL_CERTS;

  n = certs_cell_encode(d->cell->payload, 4096, d->ccell);
  tt_int_op(n, OP_GT, 0);
  d->cell->payload_len = n;

  MOCK(tor_tls_cert_matches_key, mock_tls_cert_matches_key);
  MOCK(connection_or_send_netinfo, mock_send_netinfo);
  MOCK(connection_or_close_for_error, mock_close_for_err);
  MOCK(tor_tls_get_peer_cert, mock_get_peer_cert);

  if (is_link) {
    /* Say that this is the peer's certificate */
    mock_peer_cert = tor_x509_cert_dup(a);
  }

  tt_int_op(0, OP_EQ, d->c->handshake_state->received_certs_cell);
  tt_int_op(0, OP_EQ, mock_send_authenticate_called);
  tt_int_op(0, OP_EQ, mock_send_netinfo_called);

  return d;
 done:
  recv_certs_cleanup(test, d);
  return NULL;
}

static struct testcase_setup_t setup_recv_certs = {
  .setup_fn = recv_certs_setup,
  .cleanup_fn = recv_certs_cleanup
};

static void
test_link_handshake_recv_certs_ok(void *arg)
{
  certs_data_t *d = arg;
  channel_tls_process_certs_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(d->c->handshake_state->authenticated, OP_EQ, 1);
  tt_int_op(d->c->handshake_state->authenticated_rsa, OP_EQ, 1);
  tt_int_op(d->c->handshake_state->received_certs_cell, OP_EQ, 1);
  tt_ptr_op(d->c->handshake_state->certs->id_cert, OP_NE, NULL);
  tt_ptr_op(d->c->handshake_state->certs->auth_cert, OP_EQ, NULL);

  if (d->is_ed) {
    tt_ptr_op(d->c->handshake_state->certs->ed_id_sign, OP_NE, NULL);
    tt_ptr_op(d->c->handshake_state->certs->ed_sign_link, OP_NE, NULL);
    tt_ptr_op(d->c->handshake_state->certs->ed_sign_auth, OP_EQ, NULL);
    tt_ptr_op(d->c->handshake_state->certs->ed_rsa_crosscert, OP_NE, NULL);
    tt_int_op(d->c->handshake_state->authenticated_ed25519, OP_EQ, 1);
  } else {
    tt_ptr_op(d->c->handshake_state->certs->ed_id_sign, OP_EQ, NULL);
    tt_ptr_op(d->c->handshake_state->certs->ed_sign_link, OP_EQ, NULL);
    tt_ptr_op(d->c->handshake_state->certs->ed_sign_auth, OP_EQ, NULL);
    tt_ptr_op(d->c->handshake_state->certs->ed_rsa_crosscert, OP_EQ, NULL);
    tt_int_op(d->c->handshake_state->authenticated_ed25519, OP_EQ, 0);
  }

 done:
  ;
}

static void
test_link_handshake_recv_certs_ok_server(void *arg)
{
  certs_data_t *d = arg;
  d->c->handshake_state->started_here = 0;
  d->c->handshake_state->certs->started_here = 0;
  channel_tls_process_certs_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(d->c->handshake_state->authenticated, OP_EQ, 0);
  tt_int_op(d->c->handshake_state->received_certs_cell, OP_EQ, 1);
  tt_ptr_op(d->c->handshake_state->certs->id_cert, OP_NE, NULL);
  tt_ptr_op(d->c->handshake_state->certs->link_cert, OP_EQ, NULL);
  if (d->is_ed) {
    tt_ptr_op(d->c->handshake_state->certs->ed_sign_auth, OP_NE, NULL);
    tt_ptr_op(d->c->handshake_state->certs->auth_cert, OP_EQ, NULL);
  } else {
    tt_ptr_op(d->c->handshake_state->certs->ed_sign_auth, OP_EQ, NULL);
    tt_ptr_op(d->c->handshake_state->certs->auth_cert, OP_NE, NULL);
  }

 done:
  ;
}

#define CERTS_FAIL(name, code)                          \
  static void                                                           \
  test_link_handshake_recv_certs_ ## name(void *arg)                    \
  {                                                                     \
    certs_data_t *d = arg;                                              \
    const char *require_failure_message = NULL;                         \
    setup_capture_of_logs(LOG_INFO);                                    \
    { code ; }                                                          \
    channel_tls_process_certs_cell(d->cell, d->chan);                   \
    tt_int_op(1, OP_EQ, mock_close_called);                                \
    tt_int_op(0, OP_EQ, mock_send_authenticate_called);                    \
    tt_int_op(0, OP_EQ, mock_send_netinfo_called);                         \
    tt_int_op(0, OP_EQ, d->c->handshake_state->authenticated_rsa);         \
    tt_int_op(0, OP_EQ, d->c->handshake_state->authenticated_ed25519);     \
    if (require_failure_message) {                                      \
      expect_log_msg_containing(require_failure_message);               \
    }                                                                   \
  done:                                                                 \
    teardown_capture_of_logs();                               \
  }

CERTS_FAIL(badstate,
           require_failure_message = "We're not doing a v3 handshake!";
           d->c->base_.state = OR_CONN_STATE_CONNECTING;)
CERTS_FAIL(badproto,
           require_failure_message = "not using link protocol >= 3";
           d->c->link_proto = 2)
CERTS_FAIL(duplicate,
           require_failure_message = "We already got one";
           d->c->handshake_state->received_certs_cell = 1)
CERTS_FAIL(already_authenticated,
           require_failure_message = "We're already authenticated!";
           d->c->handshake_state->authenticated = 1)
CERTS_FAIL(empty,
           require_failure_message = "It had no body";
           d->cell->payload_len = 0)
CERTS_FAIL(bad_circid,
           require_failure_message = "It had a nonzero circuit ID";
           d->cell->circ_id = 1)
CERTS_FAIL(truncated_1,
           require_failure_message = "It couldn't be parsed";
           d->cell->payload[0] = 5)
CERTS_FAIL(truncated_2,
           {
             require_failure_message = "It couldn't be parsed";
             d->cell->payload_len = 4;
             memcpy(d->cell->payload, "\x01\x01\x00\x05", 4);
           })
CERTS_FAIL(truncated_3,
           {
             require_failure_message = "It couldn't be parsed";
             d->cell->payload_len = 7;
             memcpy(d->cell->payload, "\x01\x01\x00\x05""abc", 7);
           })
CERTS_FAIL(truncated_4, /* ed25519 */
           {
             require_failure_message = "It couldn't be parsed";
             d->cell->payload_len -= 10;
           })
CERTS_FAIL(truncated_5, /* ed25519 */
           {
             require_failure_message = "It couldn't be parsed";
             d->cell->payload_len -= 100;
           })

#define REENCODE() do {                                                 \
    const char *msg = certs_cell_check(d->ccell);                       \
    if (msg) puts(msg);                                                 \
    ssize_t n = certs_cell_encode(d->cell->payload, 4096, d->ccell);    \
    tt_int_op(n, OP_GT, 0);                                                 \
    d->cell->payload_len = n;                                           \
  } while (0)

CERTS_FAIL(truncated_6, /* ed25519 */
   {
     /* truncate the link certificate */
     require_failure_message = "undecodable Ed certificate";
     certs_cell_cert_setlen_body(certs_cell_get_certs(d->ccell, 3), 7);
     certs_cell_get_certs(d->ccell, 3)->cert_len = 7;
     REENCODE();
   })
CERTS_FAIL(truncated_7, /* ed25519 */
   {
     /* truncate the crosscert */
     require_failure_message = "Unparseable or overlong crosscert";
     certs_cell_cert_setlen_body(certs_cell_get_certs(d->ccell, 4), 7);
     certs_cell_get_certs(d->ccell, 4)->cert_len = 7;
     REENCODE();
   })
CERTS_FAIL(not_x509,
  {
    require_failure_message = "Received undecodable certificate";
    certs_cell_cert_setlen_body(certs_cell_get_certs(d->ccell, 0), 3);
    certs_cell_get_certs(d->ccell, 0)->cert_len = 3;
    REENCODE();
  })
CERTS_FAIL(both_link,
  {
    require_failure_message = "Duplicate x509 certificate";
    certs_cell_get_certs(d->ccell, 0)->cert_type = 1;
    certs_cell_get_certs(d->ccell, 1)->cert_type = 1;
    REENCODE();
  })
CERTS_FAIL(both_id_rsa,
  {
    require_failure_message = "Duplicate x509 certificate";
    certs_cell_get_certs(d->ccell, 0)->cert_type = 2;
    certs_cell_get_certs(d->ccell, 1)->cert_type = 2;
    REENCODE();
  })
CERTS_FAIL(both_auth,
  {
    require_failure_message = "Duplicate x509 certificate";
    certs_cell_get_certs(d->ccell, 0)->cert_type = 3;
    certs_cell_get_certs(d->ccell, 1)->cert_type = 3;
    REENCODE();
  })
CERTS_FAIL(duplicate_id, /* ed25519 */
  {
    require_failure_message = "Duplicate Ed25519 certificate";
    certs_cell_get_certs(d->ccell, 2)->cert_type = 4;
    certs_cell_get_certs(d->ccell, 3)->cert_type = 4;
    REENCODE();
  })
CERTS_FAIL(duplicate_link, /* ed25519 */
  {
    require_failure_message = "Duplicate Ed25519 certificate";
    certs_cell_get_certs(d->ccell, 2)->cert_type = 5;
    certs_cell_get_certs(d->ccell, 3)->cert_type = 5;
    REENCODE();
  })
CERTS_FAIL(duplicate_crosscert, /* ed25519 */
  {
    require_failure_message = "Duplicate RSA->Ed25519 crosscert";
    certs_cell_get_certs(d->ccell, 2)->cert_type = 7;
    certs_cell_get_certs(d->ccell, 3)->cert_type = 7;
    REENCODE();
  })
static void
test_link_handshake_recv_certs_missing_id(void *arg) /* ed25519 */
{
  certs_data_t *d = arg;
  tt_int_op(certs_cell_getlen_certs(d->ccell), OP_EQ, 5);
  certs_cell_set_certs(d->ccell, 2, certs_cell_get_certs(d->ccell, 4));
  certs_cell_set0_certs(d->ccell, 4, NULL); /* prevent free */
  certs_cell_setlen_certs(d->ccell, 4);
  d->ccell->n_certs = 4;
  REENCODE();

  /* This handshake succeeds, but since we have no ID cert, we will
   * just do the RSA handshake. */
  channel_tls_process_certs_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(0, OP_EQ, d->c->handshake_state->authenticated_ed25519);
  tt_int_op(1, OP_EQ, d->c->handshake_state->authenticated_rsa);
 done:
  ;
}
CERTS_FAIL(missing_signing_key, /* ed25519 */
  {
    require_failure_message = "No Ed25519 signing key";
    tt_int_op(certs_cell_getlen_certs(d->ccell), OP_EQ, 5);
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 2);
    tt_int_op(cert->cert_type, OP_EQ, CERTTYPE_ED_ID_SIGN);
    /* replace this with a valid master->signing cert, but with no
     * signing key. */
    const ed25519_keypair_t *mk = get_master_identity_keypair();
    const ed25519_keypair_t *sk = get_master_signing_keypair();
    tor_cert_t *bad_cert = tor_cert_create_ed25519(mk, CERT_TYPE_ID_SIGNING,
                                           &sk->pubkey, time(NULL), 86400,
                                           0 /* don't include signer */);
    certs_cell_cert_setlen_body(cert, bad_cert->encoded_len);
    memcpy(certs_cell_cert_getarray_body(cert),
           bad_cert->encoded, bad_cert->encoded_len);
    cert->cert_len = bad_cert->encoded_len;
    tor_cert_free(bad_cert);
    REENCODE();
  })
CERTS_FAIL(missing_link, /* ed25519 */
  {
    require_failure_message = "No Ed25519 link key";
    tt_int_op(certs_cell_getlen_certs(d->ccell), OP_EQ, 5);
    certs_cell_set_certs(d->ccell, 3, certs_cell_get_certs(d->ccell, 4));
    certs_cell_set0_certs(d->ccell, 4, NULL); /* prevent free */
    certs_cell_setlen_certs(d->ccell, 4);
    d->ccell->n_certs = 4;
    REENCODE();
  })
CERTS_FAIL(missing_auth, /* ed25519 */
  {
    d->c->handshake_state->started_here = 0;
    d->c->handshake_state->certs->started_here = 0;
    require_failure_message = "No Ed25519 link authentication key";
    tt_int_op(certs_cell_getlen_certs(d->ccell), OP_EQ, 5);
    certs_cell_set_certs(d->ccell, 3, certs_cell_get_certs(d->ccell, 4));
    certs_cell_set0_certs(d->ccell, 4, NULL); /* prevent free */
    certs_cell_setlen_certs(d->ccell, 4);
    d->ccell->n_certs = 4;
    REENCODE();
  })
CERTS_FAIL(missing_crosscert, /* ed25519 */
  {
    require_failure_message = "Missing RSA->Ed25519 crosscert";
    tt_int_op(certs_cell_getlen_certs(d->ccell), OP_EQ, 5);
    certs_cell_setlen_certs(d->ccell, 4);
    d->ccell->n_certs = 4;
    REENCODE();
  })
CERTS_FAIL(missing_rsa_id, /* ed25519 */
  {
    require_failure_message = "Missing legacy RSA ID cert";
    tt_int_op(certs_cell_getlen_certs(d->ccell), OP_EQ, 5);
    certs_cell_set_certs(d->ccell, 1, certs_cell_get_certs(d->ccell, 4));
    certs_cell_set0_certs(d->ccell, 4, NULL); /* prevent free */
    certs_cell_setlen_certs(d->ccell, 4);
    d->ccell->n_certs = 4;
    REENCODE();
  })
CERTS_FAIL(link_mismatch, /* ed25519 */
  {
    require_failure_message = "Link certificate does not match "
      "TLS certificate";
    const tor_x509_cert_t *idc;
    tor_tls_get_my_certs(1, NULL, &idc);
    tor_x509_cert_free(mock_peer_cert);
    /* Pretend that the peer cert was something else. */
    mock_peer_cert = tor_x509_cert_dup(idc);
    /* No reencode needed. */
  })
CERTS_FAIL(bad_ed_sig, /* ed25519 */
  {
    require_failure_message = "At least one Ed25519 certificate was "
      "badly signed";
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 3);
    uint8_t *body = certs_cell_cert_getarray_body(cert);
    ssize_t body_len = certs_cell_cert_getlen_body(cert);
    /* Frob a byte in the signature */
    body[body_len - 13] ^= 7;
    REENCODE();
  })
CERTS_FAIL(bad_crosscert, /*ed25519*/
  {
    require_failure_message = "Invalid RSA->Ed25519 crosscert";
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 4);
    uint8_t *body = certs_cell_cert_getarray_body(cert);
    ssize_t body_len = certs_cell_cert_getlen_body(cert);
    /* Frob a byte in the signature */
    body[body_len - 13] ^= 7;
    REENCODE();
  })
CERTS_FAIL(bad_rsa_id_cert, /*ed25519*/
  {
    require_failure_message = "legacy RSA ID certificate was not valid";
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 1);
    uint8_t *body;
    /* Frob a byte in the signature, after making a new cert. (NSS won't let
     * us just frob the old cert, since it will see that the issuer & serial
     * number are the same, which will make it fail at an earlier stage than
     * signature verification.) */
    const tor_x509_cert_t *idc;
    tor_x509_cert_t *newc;
    tor_tls_get_my_certs(1, NULL, &idc);
    time_t new_end = time(NULL) + 86400 * 10;
    newc = tor_x509_cert_replace_expiration(idc, new_end, d->key2);
    const uint8_t *encoded;
    size_t encoded_len;
    tor_x509_cert_get_der(newc, &encoded, &encoded_len);
    certs_cell_cert_setlen_body(cert, encoded_len);
    certs_cell_cert_set_cert_len(cert, encoded_len);
    body = certs_cell_cert_getarray_body(cert);
    memcpy(body, encoded, encoded_len);
    body[encoded_len - 13] ^= 7;
    REENCODE();
    tor_x509_cert_free(newc);
  })
CERTS_FAIL(expired_rsa_id, /* both */
  {
    require_failure_message = "Certificate already expired";
    /* we're going to replace the identity cert with an expired one. */
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 1);
    const tor_x509_cert_t *idc;
    tor_tls_get_my_certs(1, NULL, &idc);
    tor_x509_cert_t *newc;
    time_t new_end = time(NULL) - 86400 * 10;
    newc = tor_x509_cert_replace_expiration(idc, new_end, d->key2);
    const uint8_t *encoded;
    size_t encoded_len;
    tor_x509_cert_get_der(newc, &encoded, &encoded_len);
    certs_cell_cert_setlen_body(cert, encoded_len);
    certs_cell_cert_set_cert_len(cert, encoded_len);
    memcpy(certs_cell_cert_getarray_body(cert), encoded, encoded_len);
    REENCODE();
    tor_x509_cert_free(newc);
  })
CERTS_FAIL(expired_ed_id, /* ed25519 */
  {
    /* we're going to replace the Ed Id->sign cert with an expired one. */
    require_failure_message = "At least one certificate expired";
    /* We don't need to re-sign, since we check for expiration first. */
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 2);
    uint8_t *body = certs_cell_cert_getarray_body(cert);
    /* The expiration field is bytes [2..5].  It is in HOURS since the
     * epoch. */
    set_uint32(body+2, htonl(24)); /* Back to jan 2, 1970. */
    REENCODE();
  })
CERTS_FAIL(expired_ed_link, /* ed25519 */
  {
    /* we're going to replace the Ed Sign->link cert with an expired one. */
    require_failure_message = "At least one certificate expired";
    /* We don't need to re-sign, since we check for expiration first. */
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 3);
    uint8_t *body = certs_cell_cert_getarray_body(cert);
    /* The expiration field is bytes [2..5].  It is in HOURS since the
     * epoch. */
    set_uint32(body+2, htonl(24)); /* Back to jan 2, 1970. */
    REENCODE();
  })
CERTS_FAIL(expired_crosscert, /* ed25519 */
  {
    /* we're going to replace the Ed Sign->link cert with an expired one. */
    require_failure_message = "Crosscert is expired";
    /* We don't need to re-sign, since we check for expiration first. */
    certs_cell_cert_t *cert = certs_cell_get_certs(d->ccell, 4);
    uint8_t *body = certs_cell_cert_getarray_body(cert);
    /* The expiration field is bytes [32..35]. once again, HOURS. */
    set_uint32(body+32, htonl(24)); /* Back to jan 2, 1970. */
    REENCODE();
  })

CERTS_FAIL(wrong_labels_1,
  {
    require_failure_message = "The link certificate was not valid";
    certs_cell_get_certs(d->ccell, 0)->cert_type = 2;
    certs_cell_get_certs(d->ccell, 1)->cert_type = 1;
    REENCODE();
  })
CERTS_FAIL(wrong_labels_2,
  {
    const tor_x509_cert_t *a;
    const tor_x509_cert_t *b;
    const uint8_t *enca;
    size_t lena;
    require_failure_message = "The link certificate was not valid";
    tor_tls_get_my_certs(1, &a, &b);
    tor_x509_cert_get_der(a, &enca, &lena);
    certs_cell_cert_setlen_body(certs_cell_get_certs(d->ccell, 1), lena);
    memcpy(certs_cell_cert_getarray_body(certs_cell_get_certs(d->ccell, 1)),
           enca, lena);
    certs_cell_get_certs(d->ccell, 1)->cert_len = lena;
    REENCODE();
  })
CERTS_FAIL(wrong_labels_3,
           {
             require_failure_message =
               "The certs we wanted (ID, Link) were missing";
             certs_cell_get_certs(d->ccell, 0)->cert_type = 2;
             certs_cell_get_certs(d->ccell, 1)->cert_type = 3;
             REENCODE();
           })
CERTS_FAIL(server_missing_certs,
           {
             require_failure_message =
               "The certs we wanted (ID, Auth) were missing";
             d->c->handshake_state->started_here = 0;
             d->c->handshake_state->certs->started_here = 0;

           })
CERTS_FAIL(server_wrong_labels_1,
           {
             require_failure_message =
               "The authentication certificate was not valid";
             d->c->handshake_state->started_here = 0;
             d->c->handshake_state->certs->started_here = 0;
             certs_cell_get_certs(d->ccell, 0)->cert_type = 2;
             certs_cell_get_certs(d->ccell, 1)->cert_type = 3;
             REENCODE();
           })

static void
test_link_handshake_send_authchallenge(void *arg)
{
  (void)arg;

  or_connection_t *c1 = or_connection_new(CONN_TYPE_OR, AF_INET);
  var_cell_t *cell1=NULL, *cell2=NULL;

  crypto_pk_t *rsa0 = pk_generate(0), *rsa1 = pk_generate(1);
  tt_int_op(tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                                 rsa0, rsa1, 86400), OP_EQ, 0);
  init_mock_ed_keys(rsa0);

  MOCK(connection_or_write_var_cell_to_buf, mock_write_var_cell);

  tt_int_op(connection_init_or_handshake_state(c1, 0), OP_EQ, 0);
  c1->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  tt_ptr_op(mock_got_var_cell, OP_EQ, NULL);
  tt_int_op(0, OP_EQ, connection_or_send_auth_challenge_cell(c1));
  cell1 = mock_got_var_cell;
  tt_int_op(0, OP_EQ, connection_or_send_auth_challenge_cell(c1));
  cell2 = mock_got_var_cell;
#ifdef HAVE_WORKING_TOR_TLS_GET_TLSSECRETS
  tt_int_op(38, OP_EQ, cell1->payload_len);
  tt_int_op(38, OP_EQ, cell2->payload_len);
#else
  tt_int_op(36, OP_EQ, cell1->payload_len);
  tt_int_op(36, OP_EQ, cell2->payload_len);
#endif /* defined(HAVE_WORKING_TOR_TLS_GET_TLSSECRETS) */
  tt_int_op(0, OP_EQ, cell1->circ_id);
  tt_int_op(0, OP_EQ, cell2->circ_id);
  tt_int_op(CELL_AUTH_CHALLENGE, OP_EQ, cell1->command);
  tt_int_op(CELL_AUTH_CHALLENGE, OP_EQ, cell2->command);

#ifdef HAVE_WORKING_TOR_TLS_GET_TLSSECRETS
  tt_mem_op("\x00\x02\x00\x01\x00\x03", OP_EQ, cell1->payload + 32, 6);
  tt_mem_op("\x00\x02\x00\x01\x00\x03", OP_EQ, cell2->payload + 32, 6);
#else
  tt_mem_op("\x00\x01\x00\x03", OP_EQ, cell1->payload + 32, 4);
  tt_mem_op("\x00\x01\x00\x03", OP_EQ, cell2->payload + 32, 4);
#endif /* defined(HAVE_WORKING_TOR_TLS_GET_TLSSECRETS) */
  tt_mem_op(cell1->payload, OP_NE, cell2->payload, 32);

 done:
  UNMOCK(connection_or_write_var_cell_to_buf);
  connection_free_minimal(TO_CONN(c1));
  tor_free(cell1);
  tor_free(cell2);
  crypto_pk_free(rsa0);
  crypto_pk_free(rsa1);
}

typedef struct authchallenge_data_t {
  or_connection_t *c;
  channel_tls_t *chan;
  var_cell_t *cell;
} authchallenge_data_t;

static int
recv_authchallenge_cleanup(const struct testcase_t *test, void *obj)
{
  (void)test;
  authchallenge_data_t *d = obj;

  UNMOCK(connection_or_send_netinfo);
  UNMOCK(connection_or_close_for_error);
  UNMOCK(connection_or_send_authenticate_cell);

  if (d) {
    tor_free(d->cell);
    connection_free_minimal(TO_CONN(d->c));
    circuitmux_free(d->chan->base_.cmux);
    tor_free(d->chan);
    tor_free(d);
  }
  return 1;
}

static void *
recv_authchallenge_setup(const struct testcase_t *test)
{
  (void)test;

  testing__connection_or_pretend_TLSSECRET_is_supported = 1;
  authchallenge_data_t *d = tor_malloc_zero(sizeof(*d));
  d->c = or_connection_new(CONN_TYPE_OR, AF_INET);
  d->chan = tor_malloc_zero(sizeof(*d->chan));
  d->c->chan = d->chan;
  d->c->base_.address = tor_strdup("HaveAnAddress");
  d->c->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  d->chan->conn = d->c;
  tt_int_op(connection_init_or_handshake_state(d->c, 1), OP_EQ, 0);
  d->c->link_proto = 4;
  d->c->handshake_state->received_certs_cell = 1;
  d->cell = var_cell_new(128);
  d->cell->payload_len = 38;
  d->cell->payload[33] = 2; /* 2 methods */
  d->cell->payload[35] = 7; /* This one isn't real */
  d->cell->payload[37] = 1; /* This is the old RSA one. */
  d->cell->command = CELL_AUTH_CHALLENGE;

  get_options_mutable()->ORPort_set = 1;

  MOCK(connection_or_close_for_error, mock_close_for_err);
  MOCK(connection_or_send_netinfo, mock_send_netinfo);
  MOCK(connection_or_send_authenticate_cell, mock_send_authenticate);
  tt_int_op(0, OP_EQ, d->c->handshake_state->received_auth_challenge);
  tt_int_op(0, OP_EQ, mock_send_authenticate_called);
  tt_int_op(0, OP_EQ, mock_send_netinfo_called);

  return d;
 done:
  recv_authchallenge_cleanup(test, d);
  return NULL;
}

static struct testcase_setup_t setup_recv_authchallenge = {
  .setup_fn = recv_authchallenge_setup,
  .cleanup_fn = recv_authchallenge_cleanup
};

static void
test_link_handshake_recv_authchallenge_ok(void *arg)
{
  authchallenge_data_t *d = arg;

  channel_tls_process_auth_challenge_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(1, OP_EQ, d->c->handshake_state->received_auth_challenge);
  tt_int_op(1, OP_EQ, mock_send_authenticate_called);
  tt_int_op(1, OP_EQ, mock_send_netinfo_called);
  tt_int_op(1, OP_EQ, mock_send_authenticate_called_with_type); /* RSA */
 done:
  ;
}

static void
test_link_handshake_recv_authchallenge_ok_ed25519(void *arg)
{
  authchallenge_data_t *d = arg;

  /* Add the ed25519 authentication mechanism here. */
  d->cell->payload[33] = 3; /* 3 types are supported now. */
  d->cell->payload[39] = 3;
  d->cell->payload_len += 2;
  channel_tls_process_auth_challenge_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(1, OP_EQ, d->c->handshake_state->received_auth_challenge);
  tt_int_op(1, OP_EQ, mock_send_authenticate_called);
  tt_int_op(1, OP_EQ, mock_send_netinfo_called);
  tt_int_op(3, OP_EQ, mock_send_authenticate_called_with_type); /* Ed25519 */
 done:
  ;
}

static void
test_link_handshake_recv_authchallenge_ok_noserver(void *arg)
{
  authchallenge_data_t *d = arg;
  get_options_mutable()->ORPort_set = 0;

  channel_tls_process_auth_challenge_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(1, OP_EQ, d->c->handshake_state->received_auth_challenge);
  tt_int_op(0, OP_EQ, mock_send_authenticate_called);
  tt_int_op(0, OP_EQ, mock_send_netinfo_called);
 done:
  ;
}

static void
test_link_handshake_recv_authchallenge_ok_unrecognized(void *arg)
{
  authchallenge_data_t *d = arg;
  d->cell->payload[37] = 99;

  channel_tls_process_auth_challenge_cell(d->cell, d->chan);
  tt_int_op(0, OP_EQ, mock_close_called);
  tt_int_op(1, OP_EQ, d->c->handshake_state->received_auth_challenge);
  tt_int_op(0, OP_EQ, mock_send_authenticate_called);
  tt_int_op(1, OP_EQ, mock_send_netinfo_called);
 done:
  ;
}

#define AUTHCHALLENGE_FAIL(name, code)                          \
  static void                                                           \
  test_link_handshake_recv_authchallenge_ ## name(void *arg)            \
  {                                                                     \
    authchallenge_data_t *d = arg;                                      \
    const char *require_failure_message = NULL;                         \
    setup_capture_of_logs(LOG_INFO);                                    \
    { code ; }                                                          \
    channel_tls_process_auth_challenge_cell(d->cell, d->chan);          \
    tt_int_op(1, OP_EQ, mock_close_called);                                \
    tt_int_op(0, OP_EQ, mock_send_authenticate_called);                    \
    tt_int_op(0, OP_EQ, mock_send_netinfo_called);                         \
    if (require_failure_message) {                                      \
      expect_log_msg_containing(require_failure_message);               \
    }                                                                   \
  done:                                                                 \
    teardown_capture_of_logs();                               \
  }

AUTHCHALLENGE_FAIL(badstate,
                   require_failure_message = "We're not currently doing a "
                     "v3 handshake";
                   d->c->base_.state = OR_CONN_STATE_CONNECTING)
AUTHCHALLENGE_FAIL(badproto,
                   require_failure_message = "not using link protocol >= 3";
                   d->c->link_proto = 2)
AUTHCHALLENGE_FAIL(as_server,
                   require_failure_message = "We didn't originate this "
                     "connection";
                   d->c->handshake_state->started_here = 0;
                   d->c->handshake_state->certs->started_here = 0;)
AUTHCHALLENGE_FAIL(duplicate,
                   require_failure_message = "We already received one";
                   d->c->handshake_state->received_auth_challenge = 1)
AUTHCHALLENGE_FAIL(nocerts,
                   require_failure_message = "We haven't gotten a CERTS "
                     "cell yet";
                   d->c->handshake_state->received_certs_cell = 0)
AUTHCHALLENGE_FAIL(tooshort,
                   require_failure_message = "It was not well-formed";
                   d->cell->payload_len = 33)
AUTHCHALLENGE_FAIL(truncated,
                   require_failure_message = "It was not well-formed";
                   d->cell->payload_len = 34)
AUTHCHALLENGE_FAIL(nonzero_circid,
                   require_failure_message = "It had a nonzero circuit ID";
                   d->cell->circ_id = 1337)

static int
mock_get_tlssecrets(tor_tls_t *tls, uint8_t *secrets_out)
{
  (void)tls;
  memcpy(secrets_out, "int getRandomNumber(){return 4;}", 32);
  return 0;
}

static void
mock_set_circid_type(channel_t *chan,
                     crypto_pk_t *identity_rcvd,
                     int consider_identity)
{
  (void) chan;
  (void) identity_rcvd;
  (void) consider_identity;
}

typedef struct authenticate_data_t {
  int is_ed;
  or_connection_t *c1, *c2;
  channel_tls_t *chan2;
  var_cell_t *cell;
  crypto_pk_t *key1, *key2;
} authenticate_data_t;

static int
authenticate_data_cleanup(const struct testcase_t *test, void *arg)
{
  (void) test;
  UNMOCK(connection_or_write_var_cell_to_buf);
  UNMOCK(tor_tls_get_peer_cert);
  UNMOCK(tor_tls_get_own_cert);
  UNMOCK(tor_tls_get_tlssecrets);
  UNMOCK(connection_or_close_for_error);
  UNMOCK(channel_set_circid_type);
  UNMOCK(tor_tls_export_key_material);
  authenticate_data_t *d = arg;
  if (d) {
    tor_free(d->cell);
    connection_or_clear_identity(d->c1);
    connection_or_clear_identity(d->c2);
    connection_free_minimal(TO_CONN(d->c1));
    connection_free_minimal(TO_CONN(d->c2));
    circuitmux_free(d->chan2->base_.cmux);
    tor_free(d->chan2);
    crypto_pk_free(d->key1);
    crypto_pk_free(d->key2);
    tor_free(d);
  }
  tor_x509_cert_free(mock_peer_cert);
  tor_x509_cert_free(mock_own_cert);
  mock_peer_cert = NULL;
  mock_own_cert = NULL;

  return 1;
}

static void *
authenticate_data_setup(const struct testcase_t *test)
{
  authenticate_data_t *d = tor_malloc_zero(sizeof(*d));
  int is_ed = d->is_ed = (test->setup_data == (void*)3);

  testing__connection_or_pretend_TLSSECRET_is_supported = 1;

  scheduler_init();

  MOCK(connection_or_write_var_cell_to_buf, mock_write_var_cell);
  MOCK(tor_tls_get_peer_cert, mock_get_peer_cert);
  MOCK(tor_tls_get_own_cert, mock_get_own_cert);
  MOCK(tor_tls_get_tlssecrets, mock_get_tlssecrets);
  MOCK(connection_or_close_for_error, mock_close_for_err);
  MOCK(channel_set_circid_type, mock_set_circid_type);
  MOCK(tor_tls_export_key_material, mock_export_key_material);
  d->c1 = or_connection_new(CONN_TYPE_OR, AF_INET);
  d->c2 = or_connection_new(CONN_TYPE_OR, AF_INET);
  tor_addr_from_ipv4h(&d->c1->base_.addr, 0x01020304);
  tor_addr_from_ipv4h(&d->c2->base_.addr, 0x05060708);

  d->key1 = pk_generate(2);
  d->key2 = pk_generate(3);
  tt_int_op(tor_tls_context_init(TOR_TLS_CTX_IS_PUBLIC_SERVER,
                                 d->key1, d->key2, 86400), OP_EQ, 0);

  init_mock_ed_keys(d->key2);

  d->c1->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  d->c1->link_proto = 3;
  tt_int_op(connection_init_or_handshake_state(d->c1, 1), OP_EQ, 0);

  d->c2->base_.state = OR_CONN_STATE_OR_HANDSHAKING_V3;
  d->c2->link_proto = 3;
  tt_int_op(connection_init_or_handshake_state(d->c2, 0), OP_EQ, 0);
  var_cell_t *cell = var_cell_new(16);
  cell->command = CELL_CERTS;
  or_handshake_state_record_var_cell(d->c1, d->c1->handshake_state, cell, 1);
  or_handshake_state_record_var_cell(d->c2, d->c2->handshake_state, cell, 0);
  memset(cell->payload, 0xf0, 16);
  or_handshake_state_record_var_cell(d->c1, d->c1->handshake_state, cell, 0);
  or_handshake_state_record_var_cell(d->c2, d->c2->handshake_state, cell, 1);
  tor_free(cell);

  d->chan2 = tor_malloc_zero(sizeof(*d->chan2));
  channel_tls_common_init(d->chan2);
  d->c2->chan = d->chan2;
  d->chan2->conn = d->c2;
  d->c2->base_.address = tor_strdup("C2");
  d->c2->tls = tor_tls_new(-1, 1);
  d->c2->handshake_state->received_certs_cell = 1;

  const tor_x509_cert_t *id_cert=NULL, *link_cert=NULL, *auth_cert=NULL;
  tt_assert(! tor_tls_get_my_certs(1, &link_cert, &id_cert));

  const uint8_t *der;
  size_t sz;
  tor_x509_cert_get_der(id_cert, &der, &sz);
  d->c1->handshake_state->certs->id_cert = tor_x509_cert_decode(der, sz);
  d->c2->handshake_state->certs->id_cert = tor_x509_cert_decode(der, sz);

  if (is_ed) {
    d->c1->handshake_state->certs->ed_id_sign =
      tor_cert_dup(get_master_signing_key_cert());
    d->c2->handshake_state->certs->ed_id_sign =
      tor_cert_dup(get_master_signing_key_cert());
    d->c2->handshake_state->certs->ed_sign_auth =
      tor_cert_dup(get_current_auth_key_cert());
  } else {
    tt_assert(! tor_tls_get_my_certs(0, &auth_cert, &id_cert));
    tor_x509_cert_get_der(auth_cert, &der, &sz);
    d->c2->handshake_state->certs->auth_cert = tor_x509_cert_decode(der, sz);
  }

  tor_x509_cert_get_der(link_cert, &der, &sz);
  mock_peer_cert = tor_x509_cert_decode(der, sz);
  tt_assert(mock_peer_cert);

  mock_own_cert = tor_x509_cert_decode(der, sz);
  tt_assert(mock_own_cert);

  /* Make an authenticate cell ... */
  int authtype;
  if (is_ed)
    authtype = AUTHTYPE_ED25519_SHA256_RFC5705;
  else
    authtype = AUTHTYPE_RSA_SHA256_TLSSECRET;
  tt_int_op(0, OP_EQ, connection_or_send_authenticate_cell(d->c1, authtype));

  tt_assert(mock_got_var_cell);
  d->cell = mock_got_var_cell;
  mock_got_var_cell = NULL;

  return d;
 done:
  authenticate_data_cleanup(test, d);
  return NULL;
}

static struct testcase_setup_t setup_authenticate = {
  .setup_fn = authenticate_data_setup,
  .cleanup_fn = authenticate_data_cleanup
};

static void
test_link_handshake_auth_cell(void *arg)
{
  authenticate_data_t *d = arg;
  auth1_t *auth1 = NULL;
  crypto_pk_t *auth_pubkey = NULL;

  /* Is the cell well-formed on the outer layer? */
  tt_int_op(d->cell->command, OP_EQ, CELL_AUTHENTICATE);
  tt_int_op(d->cell->payload[0], OP_EQ, 0);
  if (d->is_ed)
    tt_int_op(d->cell->payload[1], OP_EQ, 3);
  else
    tt_int_op(d->cell->payload[1], OP_EQ, 1);
  tt_int_op(ntohs(get_uint16(d->cell->payload + 2)), OP_EQ,
            d->cell->payload_len - 4);

  /* Check it out for plausibility... */
  auth_ctx_t ctx;
  ctx.is_ed = d->is_ed;
  tt_int_op(d->cell->payload_len-4, OP_EQ, auth1_parse(&auth1,
                                             d->cell->payload+4,
                                             d->cell->payload_len - 4, &ctx));
  tt_assert(auth1);

  if (d->is_ed) {
    tt_mem_op(auth1->type, OP_EQ, "AUTH0003", 8);
  } else {
    tt_mem_op(auth1->type, OP_EQ, "AUTH0001", 8);
  }
  tt_mem_op(auth1->tlssecrets, OP_EQ, "int getRandomNumber(){return 4;}", 32);

  /* Is the signature okay? */
  const uint8_t *start = d->cell->payload+4, *end = auth1->end_of_signed;
  if (d->is_ed) {
    ed25519_signature_t sig;
    tt_int_op(auth1_getlen_sig(auth1), OP_EQ, ED25519_SIG_LEN);
    memcpy(&sig.sig, auth1_getarray_sig(auth1), ED25519_SIG_LEN);
    tt_assert(!ed25519_checksig(&sig, start, end-start,
                                &get_current_auth_keypair()->pubkey));
  } else {
    uint8_t sig[128];
    uint8_t digest[32];
    tt_int_op(auth1_getlen_sig(auth1), OP_GT, 120);
    auth_pubkey = tor_tls_cert_get_key(
                                d->c2->handshake_state->certs->auth_cert);
    int n = crypto_pk_public_checksig(
              auth_pubkey,
              (char*)sig, sizeof(sig), (char*)auth1_getarray_sig(auth1),
              auth1_getlen_sig(auth1));
    tt_int_op(n, OP_EQ, 32);
    crypto_digest256((char*)digest,
                     (const char*)start, end-start, DIGEST_SHA256);
    tt_mem_op(sig, OP_EQ, digest, 32);
  }

  /* Then feed it to c2. */
  tt_int_op(d->c2->handshake_state->authenticated, OP_EQ, 0);
  channel_tls_process_authenticate_cell(d->cell, d->chan2);
  tt_int_op(mock_close_called, OP_EQ, 0);
  tt_int_op(d->c2->handshake_state->authenticated, OP_EQ, 1);
  if (d->is_ed) {
    tt_int_op(d->c2->handshake_state->authenticated_ed25519, OP_EQ, 1);
    tt_int_op(d->c2->handshake_state->authenticated_rsa, OP_EQ, 1);
  } else {
    tt_int_op(d->c2->handshake_state->authenticated_ed25519, OP_EQ, 0);
    tt_int_op(d->c2->handshake_state->authenticated_rsa, OP_EQ, 1);
  }

 done:
  auth1_free(auth1);
  crypto_pk_free(auth_pubkey);
}

#define AUTHENTICATE_FAIL(name, code)                           \
  static void                                                   \
  test_link_handshake_auth_ ## name(void *arg)                  \
  {                                                             \
    authenticate_data_t *d = arg;                               \
    const char *require_failure_message = NULL;                 \
    setup_capture_of_logs(LOG_INFO);                            \
    { code ; }                                                  \
    tt_int_op(d->c2->handshake_state->authenticated, OP_EQ, 0);    \
    channel_tls_process_authenticate_cell(d->cell, d->chan2);   \
    tt_int_op(mock_close_called, OP_EQ, 1);                        \
    tt_int_op(d->c2->handshake_state->authenticated, OP_EQ, 0);    \
    if (require_failure_message) {                              \
      expect_log_msg_containing(require_failure_message);       \
    }                                                           \
  done:                                                         \
    teardown_capture_of_logs();                       \
  }

AUTHENTICATE_FAIL(badstate,
                  require_failure_message = "We're not doing a v3 handshake";
                  d->c2->base_.state = OR_CONN_STATE_CONNECTING)
AUTHENTICATE_FAIL(badproto,
                  require_failure_message = "not using link protocol >= 3";
                  d->c2->link_proto = 2)
AUTHENTICATE_FAIL(atclient,
                  require_failure_message = "We originated this connection";
                  d->c2->handshake_state->started_here = 1;
                  d->c2->handshake_state->certs->started_here = 1;)
AUTHENTICATE_FAIL(duplicate,
                  require_failure_message = "We already got one";
                  d->c2->handshake_state->received_authenticate = 1)
static void
test_link_handshake_auth_already_authenticated(void *arg)
{
  authenticate_data_t *d = arg;
  setup_capture_of_logs(LOG_INFO);
  d->c2->handshake_state->authenticated = 1;
  channel_tls_process_authenticate_cell(d->cell, d->chan2);
  tt_int_op(mock_close_called, OP_EQ, 1);
  tt_int_op(d->c2->handshake_state->authenticated, OP_EQ, 1);
  expect_log_msg_containing("The peer is already authenticated");
 done:
  teardown_capture_of_logs();
}

AUTHENTICATE_FAIL(nocerts,
                  require_failure_message = "We never got a certs cell";
                  d->c2->handshake_state->received_certs_cell = 0)
AUTHENTICATE_FAIL(noidcert,
                  require_failure_message = "We never got an identity "
                    "certificate";
                  tor_x509_cert_free(d->c2->handshake_state->certs->id_cert);
                  d->c2->handshake_state->certs->id_cert = NULL)
AUTHENTICATE_FAIL(noauthcert,
                  require_failure_message = "We never got an RSA "
                    "authentication certificate";
                  tor_x509_cert_free(d->c2->handshake_state->certs->auth_cert);
                  d->c2->handshake_state->certs->auth_cert = NULL)
AUTHENTICATE_FAIL(tooshort,
                  require_failure_message = "Cell was way too short";
                  d->cell->payload_len = 3)
AUTHENTICATE_FAIL(badtype,
                  require_failure_message = "Authenticator type was not "
                    "recognized";
                  d->cell->payload[0] = 0xff)
AUTHENTICATE_FAIL(truncated_1,
                  require_failure_message = "Authenticator was truncated";
                  d->cell->payload[2]++)
AUTHENTICATE_FAIL(truncated_2,
                  require_failure_message = "Authenticator was truncated";
                  d->cell->payload[3]++)
AUTHENTICATE_FAIL(tooshort_1,
                  require_failure_message = "Authenticator was too short";
                  tt_int_op(d->cell->payload_len, OP_GE, 260);
                  d->cell->payload[2] -= 1;
                  d->cell->payload_len -= 256;)
AUTHENTICATE_FAIL(badcontent,
                  require_failure_message = "Some field in the AUTHENTICATE "
                    "cell body was not as expected";
                  d->cell->payload[10] ^= 0xff)
AUTHENTICATE_FAIL(badsig_1,
                  if (d->is_ed)
                    require_failure_message = "Ed25519 signature wasn't valid";
                  else
                    require_failure_message = "RSA signature wasn't valid";
                  d->cell->payload[d->cell->payload_len - 5] ^= 0xff)
AUTHENTICATE_FAIL(missing_ed_id,
                {
                  tor_cert_free(d->c2->handshake_state->certs->ed_id_sign);
                  d->c2->handshake_state->certs->ed_id_sign = NULL;
                  require_failure_message = "Ed authenticate without Ed ID "
                    "cert from peer";
                })
AUTHENTICATE_FAIL(missing_ed_auth,
                {
                  tor_cert_free(d->c2->handshake_state->certs->ed_sign_auth);
                  d->c2->handshake_state->certs->ed_sign_auth = NULL;
                  require_failure_message = "We never got an Ed25519 "
                    "authentication certificate";
                })

#ifndef COCCI
#define TEST_RSA(name, flags)                                           \
  { #name , test_link_handshake_ ## name, (flags),                      \
      &passthrough_setup, (void*)"RSA" }

#define TEST_ED(name, flags)                                            \
  { #name "_ed25519" , test_link_handshake_ ## name, (flags),           \
      &passthrough_setup, (void*)"Ed25519" }

#define TEST_RCV_AUTHCHALLENGE(name)                            \
  { "recv_authchallenge/" #name ,                               \
    test_link_handshake_recv_authchallenge_ ## name, TT_FORK,   \
      &setup_recv_authchallenge, NULL }

#define TEST_RCV_CERTS(name)                                    \
  { "recv_certs/" #name ,                                       \
      test_link_handshake_recv_certs_ ## name, TT_FORK,         \
      &setup_recv_certs, (void*)"RSA-Link" }

#define TEST_RCV_CERTS_RSA(name,type)                           \
  { "recv_certs/" #name ,                                       \
      test_link_handshake_recv_certs_ ## name, TT_FORK,         \
      &setup_recv_certs, (void*)type }

#define TEST_RCV_CERTS_ED(name, type)                           \
  { "recv_certs/" #name "_ed25519",                             \
      test_link_handshake_recv_certs_ ## name, TT_FORK,         \
      &setup_recv_certs, (void*)type }

#define TEST_AUTHENTICATE(name)                                         \
  { "authenticate/" #name , test_link_handshake_auth_ ## name, TT_FORK, \
      &setup_authenticate, NULL }

#define TEST_AUTHENTICATE_ED(name)                                      \
  { "authenticate/" #name "_ed25519" , test_link_handshake_auth_ ## name, \
      TT_FORK, &setup_authenticate, (void*)3 }
#endif /* !defined(COCCI) */

struct testcase_t link_handshake_tests[] = {
  TEST_RSA(certs_ok, TT_FORK),
  TEST_ED(certs_ok, TT_FORK),

  TEST_RCV_CERTS(ok),
  TEST_RCV_CERTS_ED(ok, "Ed25519-Link"),
  TEST_RCV_CERTS_RSA(ok_server, "RSA-Auth"),
  TEST_RCV_CERTS_ED(ok_server, "Ed25519-Auth"),
  TEST_RCV_CERTS(badstate),
  TEST_RCV_CERTS(badproto),
  TEST_RCV_CERTS(duplicate),
  TEST_RCV_CERTS(already_authenticated),
  TEST_RCV_CERTS(empty),
  TEST_RCV_CERTS(bad_circid),
  TEST_RCV_CERTS(truncated_1),
  TEST_RCV_CERTS(truncated_2),
  TEST_RCV_CERTS(truncated_3),
  TEST_RCV_CERTS_ED(truncated_4, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(truncated_5, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(truncated_6, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(truncated_7, "Ed25519-Link"),
  TEST_RCV_CERTS(not_x509),
  TEST_RCV_CERTS(both_link),
  TEST_RCV_CERTS(both_id_rsa),
  TEST_RCV_CERTS(both_auth),
  TEST_RCV_CERTS_ED(duplicate_id, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(duplicate_link, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(duplicate_crosscert, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(missing_crosscert, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(missing_id, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(missing_signing_key, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(missing_link, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(missing_auth, "Ed25519-Auth"),
  TEST_RCV_CERTS_ED(missing_rsa_id, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(link_mismatch, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(bad_ed_sig, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(bad_rsa_id_cert, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(bad_crosscert, "Ed25519-Link"),
  TEST_RCV_CERTS_RSA(expired_rsa_id, "RSA-Link"),
  TEST_RCV_CERTS_ED(expired_rsa_id, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(expired_ed_id, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(expired_ed_link, "Ed25519-Link"),
  TEST_RCV_CERTS_ED(expired_crosscert, "Ed25519-Link"),
  TEST_RCV_CERTS(wrong_labels_1),
  TEST_RCV_CERTS(wrong_labels_2),
  TEST_RCV_CERTS(wrong_labels_3),
  TEST_RCV_CERTS(server_missing_certs),
  TEST_RCV_CERTS(server_wrong_labels_1),

  TEST_RSA(send_authchallenge, TT_FORK),
  TEST_RCV_AUTHCHALLENGE(ok),
  TEST_RCV_AUTHCHALLENGE(ok_ed25519),
  TEST_RCV_AUTHCHALLENGE(ok_noserver),
  TEST_RCV_AUTHCHALLENGE(ok_unrecognized),
  TEST_RCV_AUTHCHALLENGE(badstate),
  TEST_RCV_AUTHCHALLENGE(badproto),
  TEST_RCV_AUTHCHALLENGE(as_server),
  TEST_RCV_AUTHCHALLENGE(duplicate),
  TEST_RCV_AUTHCHALLENGE(nocerts),
  TEST_RCV_AUTHCHALLENGE(tooshort),
  TEST_RCV_AUTHCHALLENGE(truncated),
  TEST_RCV_AUTHCHALLENGE(nonzero_circid),

  TEST_AUTHENTICATE(cell),
  TEST_AUTHENTICATE_ED(cell),
  TEST_AUTHENTICATE(badstate),
  TEST_AUTHENTICATE(badproto),
  TEST_AUTHENTICATE(atclient),
  TEST_AUTHENTICATE(duplicate),
  TEST_AUTHENTICATE(already_authenticated),
  TEST_AUTHENTICATE(nocerts),
  TEST_AUTHENTICATE(noidcert),
  TEST_AUTHENTICATE(noauthcert),
  TEST_AUTHENTICATE(tooshort),
  TEST_AUTHENTICATE(badtype),
  TEST_AUTHENTICATE(truncated_1),
  TEST_AUTHENTICATE(truncated_2),
  TEST_AUTHENTICATE(tooshort_1),
  TEST_AUTHENTICATE(badcontent),
  TEST_AUTHENTICATE(badsig_1),
  TEST_AUTHENTICATE_ED(badsig_1),
  TEST_AUTHENTICATE_ED(missing_ed_id),
  TEST_AUTHENTICATE_ED(missing_ed_auth),
  //TEST_AUTHENTICATE(),

  END_OF_TESTCASES
};
