#include <string.h>

#include <openssl/cms.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#include "testutil.h"

static X509 *cert = NULL;
static EVP_PKEY *privkey = NULL;
static char *ec_recip_in = NULL;

static int test_encrypt_decrypt(void)
{
    int testresult = 0;
    STACK_OF(X509) *certstack = sk_X509_new_null();
    const char *msg = "Hello world";
    BIO *msgbio = BIO_new_mem_buf(msg, strlen(msg));
    BIO *outmsgbio = BIO_new(BIO_s_mem());
    CMS_ContentInfo* content = NULL;
    char buf[80];

    if (!TEST_ptr(certstack) || !TEST_ptr(msgbio) || !TEST_ptr(outmsgbio))
        goto end;

    if (!TEST_int_gt(sk_X509_push(certstack, cert), 0))
        goto end;

    content = CMS_encrypt(certstack, msgbio, EVP_aes_128_cbc(), CMS_TEXT);
    if (!TEST_ptr(content))
        goto end;

    if (!TEST_true(CMS_decrypt(content, privkey, cert, NULL, outmsgbio,
                               CMS_TEXT)))
        goto end;

    /* Check we got the message we first started with */
    if (!TEST_int_eq(BIO_gets(outmsgbio, buf, sizeof(buf)), strlen(msg))
            || !TEST_int_eq(strcmp(buf, msg), 0))
        goto end;

    testresult = 1;
 end:
    sk_X509_free(certstack);
    BIO_free(msgbio);
    BIO_free(outmsgbio);
    CMS_ContentInfo_free(content);

    return testresult;
}

#ifndef OPENSSL_NO_EC

/*
 * Regression test for CVE-2026-63072: an 8-byte out-of-bounds heap write
 * reachable through CMS_decrypt() when a KeyAgreeRecipientInfo names an
 * id-aesNNN-wrap-pad key-wrap OID. CMS sizes the unwrap output buffer from
 * the cipher's length query (inlen - 8), but AES-WRAP-PAD unwrap cleanses
 * inlen bytes of it on every RFC 5649 integrity-failure path.
 *
 * We build a valid ECDH KARI message (which uses non-padded id-aes256-wrap),
 * flip the single OID byte an attacker would flip on the wire to turn it into
 * id-aes256-wrap-pad (key length unchanged), and decrypt with the matching
 * private key. The unwrap must fail its integrity check without writing past
 * the CMS-allocated buffer; CMS_decrypt() must fail cleanly.  Under a
 * memory-checking build (e.g. valgrind) the overflow is flagged directly.
 */
static int test_kari_wrap_pad_unwrap_overflow(void)
{
    /* DER encoding of the id-aes256-wrap OID (2.16.840.1.101.3.4.1.45). */
    static const unsigned char aes256_wrap_oid[] = {
        0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x2d
    };
    int ret = 0;
    X509 *eccert = NULL;
    EVP_PKEY *eckey = NULL;
    BIO *certbio = NULL, *keybio = NULL, *msgbio = NULL, *outbio = NULL;
    STACK_OF(X509) *recips = NULL;
    CMS_ContentInfo *cms = NULL, *cms2 = NULL;
    unsigned char *der = NULL;
    const unsigned char *p;
    int derlen, i, patched = 0;
    const char *msg = "secret content for kari";

    if ((certbio = BIO_new_file(ec_recip_in, "r")) == NULL
        || PEM_read_bio_X509(certbio, &eccert, NULL, NULL) == NULL
        || (keybio = BIO_new_file(ec_recip_in, "r")) == NULL
        || PEM_read_bio_PrivateKey(keybio, &eckey, NULL, NULL) == NULL) {
        goto end;
    }

    if (!TEST_ptr(recips = sk_X509_new_null())
        || !TEST_int_gt(sk_X509_push(recips, eccert), 0))
        goto end;

    /* Build a normal ECDH KARI message; it uses non-padded id-aes256-wrap. */
    if (!TEST_ptr(msgbio = BIO_new_mem_buf(msg, (int)strlen(msg)))
        || !TEST_ptr(cms = CMS_encrypt(recips, msgbio, EVP_aes_256_cbc(),
                         CMS_BINARY)))
        goto end;

    if (!TEST_int_gt(derlen = i2d_CMS_ContentInfo(cms, &der), 0))
        goto end;

    /* Swap id-aes256-wrap -> id-aes256-wrap-pad (0x2d -> 0x30). */
    for (i = 0; i + (int)sizeof(aes256_wrap_oid) <= derlen; i++) {
        if (memcmp(der + i, aes256_wrap_oid, sizeof(aes256_wrap_oid)) == 0) {
            der[i + sizeof(aes256_wrap_oid) - 1] = 0x30;
            patched = 1;
            break;
        }
    }
    if (!TEST_true(patched))
        goto end;

    p = der;
    if (!TEST_ptr(cms2 = d2i_CMS_ContentInfo(NULL, &p, derlen)))
        goto end;

    /*
     * The wrap-pad unwrap fails the AIV check; with the fix it does so without
     * writing past the CMS-allocated buffer.  CMS_decrypt() must fail cleanly.
     */
    if (!TEST_ptr(outbio = BIO_new(BIO_s_mem()))
        || !TEST_false(CMS_decrypt(cms2, eckey, eccert, NULL, outbio, 0)))
        goto end;

    ret = 1;
end:
    ERR_clear_error();
    OPENSSL_free(der);
    sk_X509_free(recips);
    CMS_ContentInfo_free(cms);
    CMS_ContentInfo_free(cms2);
    BIO_free(certbio);
    BIO_free(keybio);
    BIO_free(msgbio);
    BIO_free(outbio);
    X509_free(eccert);
    EVP_PKEY_free(eckey);
    return ret;
}
#endif

int setup_tests(void)
{
    char *certin = NULL, *privkeyin = NULL;
    BIO *certbio = NULL, *privkeybio = NULL;

    if (!TEST_ptr(certin = test_get_argument(0))
            || !TEST_ptr(privkeyin = test_get_argument(1))
            || !TEST_ptr(ec_recip_in = test_get_argument(2)))
        return 0;

    certbio = BIO_new_file(certin, "r");
    if (!TEST_ptr(certbio))
        return 0;
    if (!TEST_true(PEM_read_bio_X509(certbio, &cert, NULL, NULL))) {
        BIO_free(certbio);
        return 0;
    }
    BIO_free(certbio);

    privkeybio = BIO_new_file(privkeyin, "r");
    if (!TEST_ptr(privkeybio)) {
        X509_free(cert);
        cert = NULL;
        return 0;
    }
    if (!TEST_true(PEM_read_bio_PrivateKey(privkeybio, &privkey, NULL, NULL))) {
        BIO_free(privkeybio);
        X509_free(cert);
        cert = NULL;
        return 0;
    }
    BIO_free(privkeybio);

    ADD_TEST(test_encrypt_decrypt);

#ifndef OPENSSL_NO_EC
    ADD_TEST(test_kari_wrap_pad_unwrap_overflow);
#endif
    return 1;
}

void cleanup_tests(void)
{
    X509_free(cert);
    EVP_PKEY_free(privkey);
}
