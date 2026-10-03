/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ssltestlib.h"
#include "testutil.h"

#ifndef OPENSSL_NO_DTLS1_2_METHOD
static ossl_inline void ossl_sleep(unsigned int millis)
{
# ifdef OPENSSL_SYS_VXWORKS
    struct timespec ts;
    ts.tv_sec = (long int)(millis / 1000);
    ts.tv_nsec = (long int)(millis % 1000) * 1000000ul;
    nanosleep(&ts, NULL);
# else
    usleep(millis * 1000);
# endif
}
#endif

static char *cert = NULL;
static char *privkey = NULL;
static unsigned int timer_cb_count;

#define NUM_TESTS   2


#define DUMMY_CERT_STATUS_LEN  12

static unsigned char certstatus[] = {
    SSL3_RT_HANDSHAKE, /* Content type */
    0xfe, 0xfd, /* Record version */
    0, 1, /* Epoch */
    0, 0, 0, 0, 0, 0x0f, /* Record sequence number */
    0, DTLS1_HM_HEADER_LENGTH + DUMMY_CERT_STATUS_LEN - 2,
    SSL3_MT_CERTIFICATE_STATUS, /* Cert Status handshake message type */
    0, 0, DUMMY_CERT_STATUS_LEN, /* Message len */
    0, 5, /* Message sequence */
    0, 0, 0, /* Fragment offset */
    0, 0, DUMMY_CERT_STATUS_LEN - 2, /* Fragment len */
    0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80 /* Dummy data */
};

#define RECORD_SEQUENCE 10

static unsigned int timer_cb(SSL *s, unsigned int timer_us)
{
    ++timer_cb_count;

    if (timer_us == 0)
        return 50000;
    else
        return 2 * timer_us;
}

static int test_dtls_unprocessed(int testidx)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *serverssl1 = NULL, *clientssl1 = NULL;
    BIO *c_to_s_fbio, *c_to_s_mempacket;
    int testresult = 0;

    timer_cb_count = 0;

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
                                       DTLS_client_method(),
                                       DTLS1_VERSION, DTLS_MAX_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, "AES128-SHA")))
        goto end;

    c_to_s_fbio = BIO_new(bio_f_tls_dump_filter());
    if (!TEST_ptr(c_to_s_fbio))
        goto end;

    /* BIO is freed by create_ssl_connection on error */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl1, &clientssl1,
                                      NULL, c_to_s_fbio)))
        goto end;

    DTLS_set_timer_cb(clientssl1, timer_cb);

    if (testidx == 1)
        certstatus[RECORD_SEQUENCE] = 0xff;

    /*
     * Inject a dummy record from the next epoch. In test 0, this should never
     * get used because the message sequence number is too big. In test 1 we set
     * the record sequence number to be way off in the future.
     */
    c_to_s_mempacket = SSL_get_wbio(clientssl1);
    c_to_s_mempacket = BIO_next(c_to_s_mempacket);
    mempacket_test_inject(c_to_s_mempacket, (char *)certstatus,
                          sizeof(certstatus), 1, INJECT_PACKET_IGNORE_REC_SEQ);

    /*
     * Create the connection. We use "create_bare_ssl_connection" here so that
     * we can force the connection to not do "SSL_read" once partly connected.
     * We don't want to accidentally read the dummy records we injected because
     * they will fail to decrypt.
     */
    if (!TEST_true(create_bare_ssl_connection(serverssl1, clientssl1,
                                              SSL_ERROR_NONE, 0)))
        goto end;

    if (timer_cb_count == 0) {
        printf("timer_callback was not called.\n");
        goto end;
    }

    testresult = 1;
 end:
    SSL_free(serverssl1);
    SSL_free(clientssl1);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#define CLI_TO_SRV_EPOCH_0_RECS 3
#define CLI_TO_SRV_EPOCH_1_RECS 1
#if !defined(OPENSSL_NO_EC) || !defined(OPENSSL_NO_DH)
# define SRV_TO_CLI_EPOCH_0_RECS 10
#else
/*
 * In this case we have no ServerKeyExchange message, because we don't have
 * ECDHE or DHE. When it is present it gets fragmented into 3 records in this
 * test.
 */
# define SRV_TO_CLI_EPOCH_0_RECS 9
#endif
#define SRV_TO_CLI_EPOCH_1_RECS 1
#define TOTAL_FULL_HAND_RECORDS \
            (CLI_TO_SRV_EPOCH_0_RECS + CLI_TO_SRV_EPOCH_1_RECS + \
             SRV_TO_CLI_EPOCH_0_RECS + SRV_TO_CLI_EPOCH_1_RECS)

#define CLI_TO_SRV_RESUME_EPOCH_0_RECS 3
#define CLI_TO_SRV_RESUME_EPOCH_1_RECS 1
#define SRV_TO_CLI_RESUME_EPOCH_0_RECS 2
#define SRV_TO_CLI_RESUME_EPOCH_1_RECS 1
#define TOTAL_RESUME_HAND_RECORDS \
            (CLI_TO_SRV_RESUME_EPOCH_0_RECS + CLI_TO_SRV_RESUME_EPOCH_1_RECS + \
             SRV_TO_CLI_RESUME_EPOCH_0_RECS + SRV_TO_CLI_RESUME_EPOCH_1_RECS)

#define TOTAL_RECORDS (TOTAL_FULL_HAND_RECORDS + TOTAL_RESUME_HAND_RECORDS)

static int test_dtls_drop_records(int idx)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *serverssl = NULL, *clientssl = NULL;
    BIO *c_to_s_fbio, *mempackbio;
    int testresult = 0;
    int epoch = 0;
    SSL_SESSION *sess = NULL;
    int cli_to_srv_epoch0, cli_to_srv_epoch1, srv_to_cli_epoch0;

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
                                       DTLS_client_method(),
                                       DTLS1_VERSION, DTLS_MAX_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

    if (idx >= TOTAL_FULL_HAND_RECORDS) {
        /* We're going to do a resumption handshake. Get a session first. */
        if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                          NULL, NULL))
                || !TEST_true(create_ssl_connection(serverssl, clientssl,
                              SSL_ERROR_NONE))
                || !TEST_ptr(sess = SSL_get1_session(clientssl)))
            goto end;

        SSL_shutdown(clientssl);
        SSL_shutdown(serverssl);
        SSL_free(serverssl);
        SSL_free(clientssl);
        serverssl = clientssl = NULL;

        cli_to_srv_epoch0 = CLI_TO_SRV_RESUME_EPOCH_0_RECS;
        cli_to_srv_epoch1 = CLI_TO_SRV_RESUME_EPOCH_1_RECS;
        srv_to_cli_epoch0 = SRV_TO_CLI_RESUME_EPOCH_0_RECS;
        idx -= TOTAL_FULL_HAND_RECORDS;
    } else {
        cli_to_srv_epoch0 = CLI_TO_SRV_EPOCH_0_RECS;
        cli_to_srv_epoch1 = CLI_TO_SRV_EPOCH_1_RECS;
        srv_to_cli_epoch0 = SRV_TO_CLI_EPOCH_0_RECS;
    }

    c_to_s_fbio = BIO_new(bio_f_tls_dump_filter());
    if (!TEST_ptr(c_to_s_fbio))
        goto end;

    /* BIO is freed by create_ssl_connection on error */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, c_to_s_fbio)))
        goto end;

    if (sess != NULL) {
        if (!TEST_true(SSL_set_session(clientssl, sess)))
            goto end;
    }

    DTLS_set_timer_cb(clientssl, timer_cb);
    DTLS_set_timer_cb(serverssl, timer_cb);

    /* Work out which record to drop based on the test number */
    if (idx >= cli_to_srv_epoch0 + cli_to_srv_epoch1) {
        mempackbio = SSL_get_wbio(serverssl);
        idx -= cli_to_srv_epoch0 + cli_to_srv_epoch1;
        if (idx >= srv_to_cli_epoch0) {
            epoch = 1;
            idx -= srv_to_cli_epoch0;
        }
    } else {
        mempackbio = SSL_get_wbio(clientssl);
        if (idx >= cli_to_srv_epoch0) {
            epoch = 1;
            idx -= cli_to_srv_epoch0;
        }
         mempackbio = BIO_next(mempackbio);
    }
    BIO_ctrl(mempackbio, MEMPACKET_CTRL_SET_DROP_EPOCH, epoch, NULL);
    BIO_ctrl(mempackbio, MEMPACKET_CTRL_SET_DROP_REC, idx, NULL);

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (sess != NULL && !TEST_true(SSL_session_reused(clientssl)))
        goto end;

    /* If the test did what we planned then it should have dropped a record */
    if (!TEST_int_eq((int)BIO_ctrl(mempackbio, MEMPACKET_CTRL_GET_DROP_REC, 0,
                                   NULL), -1))
        goto end;

    testresult = 1;
 end:
    SSL_SESSION_free(sess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static const char dummy_cookie[] = "0123456";

static int generate_cookie_cb(SSL *ssl, unsigned char *cookie,
                              unsigned int *cookie_len)
{
    memcpy(cookie, dummy_cookie, sizeof(dummy_cookie));
    *cookie_len = sizeof(dummy_cookie);
    return 1;
}

static int verify_cookie_cb(SSL *ssl, const unsigned char *cookie,
                            unsigned int cookie_len)
{
    return TEST_mem_eq(cookie, cookie_len, dummy_cookie, sizeof(dummy_cookie));
}

static int test_cookie(void)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *serverssl = NULL, *clientssl = NULL;
    int testresult = 0;

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
                                       DTLS_client_method(),
                                       DTLS1_VERSION, DTLS_MAX_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

    SSL_CTX_set_options(sctx, SSL_OP_COOKIE_EXCHANGE);
    SSL_CTX_set_cookie_generate_cb(sctx, generate_cookie_cb);
    SSL_CTX_set_cookie_verify_cb(sctx, verify_cookie_cb);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    testresult = 1;
 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static int test_dtls_duplicate_records(void)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *serverssl = NULL, *clientssl = NULL;
    int testresult = 0;

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
                                       DTLS_client_method(),
                                       DTLS1_VERSION, DTLS_MAX_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    DTLS_set_timer_cb(clientssl, timer_cb);
    DTLS_set_timer_cb(serverssl, timer_cb);

    BIO_ctrl(SSL_get_wbio(clientssl), MEMPACKET_CTRL_SET_DUPLICATE_REC, 1, NULL);
    BIO_ctrl(SSL_get_wbio(serverssl), MEMPACKET_CTRL_SET_DUPLICATE_REC, 1, NULL);

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    testresult = 1;
 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test that swapping an app data record so that it is received before the
 * Finished message still works.
 */
static int test_swap_app_data(void)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *sssl = NULL, *cssl = NULL;
    int testresult = 0;
    BIO *bio;
    char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    char buf[10];

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
                                       DTLS_client_method(),
                                       DTLS1_VERSION, 0,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

#ifndef OPENSSL_NO_DTLS1_2
    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, "AES128-SHA")))
        goto end;
#else
    /* Default sigalgs are SHA1 based in <DTLS1.2 which is in security level 0 */
    if (!TEST_true(SSL_CTX_set_cipher_list(sctx, "AES128-SHA:@SECLEVEL=0"))
            || !TEST_true(SSL_CTX_set_cipher_list(cctx,
                                                  "AES128-SHA:@SECLEVEL=0")))
        goto end;
#endif

    if (!TEST_true(create_ssl_objects(sctx, cctx, &sssl, &cssl,
                                      NULL, NULL)))
        goto end;

    /* Send flight 1: ClientHello */
    if (!TEST_int_le(SSL_connect(cssl), 0))
        goto end;

    /* Recv flight 1, send flight 2: ServerHello, Certificate, ServerHelloDone */
    if (!TEST_int_le(SSL_accept(sssl), 0))
        goto end;

    /* Recv flight 2, send flight 3: ClientKeyExchange, CCS, Finished */
    if (!TEST_int_le(SSL_connect(cssl), 0))
        goto end;

    /* Recv flight 3, send flight 4: datagram 1(NST, CCS) datagram 2(Finished) */
    if (!TEST_int_gt(SSL_accept(sssl), 0))
        goto end;

    /* Send flight 5: app data */
    if (!TEST_int_eq(SSL_write(sssl, msg, sizeof(msg)), (int)sizeof(msg)))
        goto end;

    bio = SSL_get_wbio(sssl);
    if (!TEST_ptr(bio)
            || !TEST_true(mempacket_swap_recent(bio)))
        goto end;

    /*
     * Recv flight 4 (datagram 1): NST, CCS, + flight 5: app data
     *      + flight 4 (datagram 2): Finished
     */
    if (!TEST_int_gt(SSL_connect(cssl), 0))
        goto end;

    /* The app data should be buffered already */
    if (!TEST_int_eq(SSL_pending(cssl), (int)sizeof(msg))
            || !TEST_true(SSL_has_pending(cssl)))
        goto end;

    /*
     * Recv flight 5 (app data)
     * We already buffered this so it should be available.
     */
    if (!TEST_int_eq(SSL_read(cssl, buf, sizeof(buf)), (int)sizeof(msg)))
        goto end;

    testresult = 1;
 end:
    SSL_free(cssl);
    SSL_free(sssl);
    SSL_CTX_free(cctx);
    SSL_CTX_free(sctx);
    return testresult;
}

#ifndef OPENSSL_NO_DTLS1_2_METHOD

#define NUM_RETRANSMITS 3

typedef struct {
    BIO *bio;
    int allowed;
    int write_calls;
} frag_bio;

static int frag_write(BIO *bio, const char *buf, size_t len, size_t *written)
{
    frag_bio *f = BIO_get_data(bio);

    BIO_clear_retry_flags(bio);

    f->write_calls++;

    if (f->allowed <= 0) {
        BIO_set_retry_write(bio);
        *written = 0;
        return 0;
    }
    f->allowed--;

    if (!BIO_write_ex(f->bio, buf, len, written)) {
        return 0;
    }

    return 1;
}

static int frag_read(BIO *bio, char *buf, size_t buf_len, size_t *readbytes)
{
    frag_bio *f = BIO_get_data(bio);
    return BIO_read_ex(f->bio, buf, buf_len, readbytes);
}

static long frag_ctrl(BIO *bio, int cmd, long num, void *ptr)
{
    frag_bio *f = BIO_get_data(bio);
    return BIO_ctrl(f->bio, cmd, num, ptr);
}

static int frag_puts(BIO *bio, const char *str)
{
    size_t written;
    return frag_write(bio, str, strlen(str), &written) ? (int)written : -1;
}

static int frag_create(BIO *bio)
{
    frag_bio *f = OPENSSL_zalloc(sizeof(*f));
    if (f == NULL)
        return 0;
    BIO_set_data(bio, f);
    BIO_set_init(bio, 1);
    return 1;
}

static int frag_destroy(BIO *bio)
{
    frag_bio *f = BIO_get_data(bio);
    if (f == NULL)
        return 1;

    BIO_free(f->bio);
    OPENSSL_free(f);
    BIO_set_data(bio, NULL);
    BIO_set_init(bio, 0);
    return 1;
}

static BIO_METHOD *frag_method(void)
{
    static BIO_METHOD *m = NULL;
    if (m == NULL) {
        m = BIO_meth_new(BIO_TYPE_SOURCE_SINK | BIO_TYPE_FILTER, "fragment-limited dgram");
        BIO_meth_set_write_ex(m, frag_write);
        BIO_meth_set_read_ex(m, frag_read);
        BIO_meth_set_ctrl(m, frag_ctrl);
        BIO_meth_set_puts(m, frag_puts);
        BIO_meth_set_create(m, frag_create);
        BIO_meth_set_destroy(m, frag_destroy);
    }
    return m;
}

static BIO *frag_new(BIO *bio, int allowed)
{
    BIO *b = BIO_new(frag_method());
    frag_bio *f;
    if (b == NULL) {
        BIO_free(bio);
        return NULL;
    }
    f = BIO_get_data(b);
    f->bio = bio;
    f->allowed = allowed;
    return b;
}

/* CVE-2026-84782: verify retransmit timer does not corrupt a parked write */
static int test_dtls_client_retransmit(void)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *serverssl = NULL, *clientssl = NULL;
    int testresult = 0;
    int ret, err, i;
    struct timeval tv;
    static unsigned char alpn[750];
    size_t j, used = 0;
    BIO *c_to_s_bio = NULL;
    BIO *frag_wbio;
    frag_bio *fb;
    int write_calls_before;

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
            DTLS_client_method(),
            DTLS1_2_VERSION, DTLS1_2_VERSION,
            &sctx, &cctx, cert, privkey)))
        return 0;

    SSL_CTX_set_verify(cctx, SSL_VERIFY_NONE, NULL);

    for (j = 0; j < 3; j++) {
        char name[250];
        int n = snprintf(name, sizeof(name),
            "proto-%04u-%s", (unsigned int)j,
            "padpadpadpadpadpadpadpadpadpadpadpadpadpadpad"
            "padpadpadpadpadpadpadpadpadpadpadpadpadpadpad"
            "padpadpadpadpadpadpadpadpadpadpadpadpadpadpad"
            "padpadpadpadpadpadpadpadpadpadpadpadpadpadpad"
            "padpadpadpadpadpadpadpadpadpadpadpadpadpad");

        if (!TEST_int_ge(n, 0) || !TEST_size_t_lt((size_t)n, sizeof(name)))
            goto end;
        if (!TEST_size_t_le(used + 1 + (size_t)n, sizeof(alpn)))
            goto end;

        alpn[used++] = (unsigned char)n;
        memcpy(alpn + used, name, (size_t)n);
        used += (size_t)n;
    }
    if (!TEST_false(SSL_CTX_set_alpn_protos(cctx, alpn, (unsigned int)used)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
            NULL, NULL)))
        goto end;

    SSL_set_options(clientssl, SSL_OP_NO_QUERY_MTU);
    if (!TEST_true(SSL_set_mtu(clientssl, 256)))
        goto end;

    c_to_s_bio = SSL_get_wbio(clientssl);

    if (!TEST_ptr(c_to_s_bio) || !TEST_true(BIO_up_ref(c_to_s_bio)))
        goto end;

    frag_wbio = frag_new(c_to_s_bio, 1);
    if (!TEST_ptr(frag_wbio)) {
        BIO_free(c_to_s_bio);
        goto end;
    }
    fb = BIO_get_data(frag_wbio);

    SSL_set0_wbio(clientssl, frag_wbio);

    DTLS_set_timer_cb(clientssl, timer_cb);

    ret = SSL_connect(clientssl);
    if (!TEST_int_le(ret, 0)
        || !TEST_int_eq(SSL_get_error(clientssl, ret), SSL_ERROR_WANT_WRITE))
        goto end;

    fb->allowed = 100;

    for (i = 0; i < NUM_RETRANSMITS; i++) {
        write_calls_before = fb->write_calls;

        if (!TEST_int_gt((int)DTLSv1_get_timeout(clientssl, &tv), 0))
            goto end;

        ossl_sleep((unsigned int)(tv.tv_sec * 1000 + tv.tv_usec / 1000) + 10);

        if (!TEST_int_ge((int)DTLSv1_handle_timeout(clientssl), 0))
            goto end;

        if (!TEST_int_eq(fb->write_calls, write_calls_before))
            goto end;
    }

    ret = SSL_connect(clientssl);
    err = SSL_get_error(clientssl, ret);

    if (!TEST_false(err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL))
        goto end;

    testresult = 1;
end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static int test_dtls_server_retransmit(void)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *serverssl = NULL, *clientssl = NULL;
    int testresult = 0;
    int ret, err, i;
    struct timeval tv;
    BIO *s_to_c_bio = NULL;
    BIO *frag_wbio;
    frag_bio *fb;
    int write_calls_before;

    if (!TEST_true(create_ssl_ctx_pair(DTLS_server_method(),
            DTLS_client_method(),
            DTLS1_2_VERSION, DTLS1_2_VERSION,
            &sctx, &cctx, cert, privkey)))
        return 0;

    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, "AES128-SHA")))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
            NULL, NULL)))
        goto end;

    SSL_set_options(serverssl, SSL_OP_NO_QUERY_MTU);
    if (!TEST_true(SSL_set_mtu(serverssl, 256)))
        goto end;

    s_to_c_bio = SSL_get_wbio(serverssl);

    if (!TEST_ptr(s_to_c_bio) || !TEST_true(BIO_up_ref(s_to_c_bio)))
        goto end;

    frag_wbio = frag_new(s_to_c_bio, 2);
    if (!TEST_ptr(frag_wbio)) {
        BIO_free(s_to_c_bio);
        goto end;
    }
    fb = BIO_get_data(frag_wbio);

    SSL_set0_wbio(serverssl, frag_wbio);

    DTLS_set_timer_cb(serverssl, timer_cb);

    if (!TEST_int_le(SSL_connect(clientssl), 0))
        goto end;

    ret = SSL_accept(serverssl);
    if (!TEST_int_le(ret, 0)
        || !TEST_int_eq(SSL_get_error(serverssl, ret), SSL_ERROR_WANT_WRITE))
        goto end;

    fb->allowed = 100;

    for (i = 0; i < NUM_RETRANSMITS; i++) {
        write_calls_before = fb->write_calls;

        if (!TEST_int_gt((int)DTLSv1_get_timeout(serverssl, &tv), 0))
            goto end;

        ossl_sleep((unsigned int)(tv.tv_sec * 1000 + tv.tv_usec / 1000) + 10);

        if (!TEST_int_ge((int)DTLSv1_handle_timeout(serverssl), 0))
            goto end;

        if (!TEST_int_eq(fb->write_calls, write_calls_before))
            goto end;
    }

    ret = SSL_accept(serverssl);
    err = SSL_get_error(serverssl, ret);

    if (!TEST_false(err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL))
        goto end;

    testresult = 1;
end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#endif

int setup_tests(void)
{
    if (!TEST_ptr(cert = test_get_argument(0))
            || !TEST_ptr(privkey = test_get_argument(1)))
        return 0;

    ADD_ALL_TESTS(test_dtls_unprocessed, NUM_TESTS);
    ADD_ALL_TESTS(test_dtls_drop_records, TOTAL_RECORDS);
    ADD_TEST(test_cookie);
    ADD_TEST(test_dtls_duplicate_records);
    ADD_TEST(test_swap_app_data);
#ifndef OPENSSL_NO_DTLS1_2_METHOD
    ADD_TEST(test_dtls_client_retransmit);
    ADD_TEST(test_dtls_server_retransmit);
#endif

    return 1;
}

void cleanup_tests(void)
{
    bio_f_tls_dump_filter_free();
    bio_s_mempacket_test_free();
}
