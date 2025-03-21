#include "includes.h"
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "bignum.h"
#include "dbrandom.h"
#include "crypto_desc.h"
#include "curve25519.h"
#include "kex.h"

/* PQ hybrids also use curve25519 internally */
#if DROPBEAR_CURVE25519_DEP

struct kex_curve25519_param *gen_kexcurve25519_param() {
    /* Per http://cr.yp.to/ecdh.html */
    struct kex_curve25519_param *param = m_malloc(sizeof(*param));
    const unsigned char basepoint[32] = {9};

    genrandom(param->priv, CURVE25519_LEN);
    dropbear_curve25519_scalarmult(param->pub, param->priv, basepoint);

    return param;
}

void free_kexcurve25519_param(struct kex_curve25519_param *param) {
    m_burn(param->priv, CURVE25519_LEN);
    m_free(param);
}

/* out must be CURVE25519_LEN */
void kexcurve25519_derive(const struct kex_curve25519_param *param, const buffer *buf_pub_them,
    unsigned char *out) {
    char zeroes[CURVE25519_LEN] = {0};
    if (buf_pub_them->len != CURVE25519_LEN)
    {
        dropbear_exit("Bad curve25519");
    }

    dropbear_curve25519_scalarmult(out, param->priv, buf_pub_them->data);

    if (constant_time_memcmp(zeroes, out, CURVE25519_LEN) == 0) {
        dropbear_exit("Bad curve25519");
    }
}

#endif /* DROPBEAR_CURVE25519_DEP */

#if DROPBEAR_CURVE25519

/* Only required for x25519 directly */
void kexcurve25519_comb_key(const struct kex_curve25519_param *param, const buffer *buf_pub_them,
    sign_key *hostkey) {
    unsigned char out[CURVE25519_LEN];
    const unsigned char* Q_C = NULL;
    const unsigned char* Q_S = NULL;

    kexcurve25519_derive(param, buf_pub_them, out);

    m_mp_alloc_init_multi(&ses.dh_K, NULL);
    bytes_to_mp(ses.dh_K, out, CURVE25519_LEN);
    m_burn(out, sizeof(out));

    /* Create the remainder of the hash buffer, to generate the exchange hash.
       See RFC5656 section 4 page 7 */
    if (IS_DROPBEAR_CLIENT) {
        Q_C = param->pub;
        Q_S = buf_pub_them->data;
    } else {
        Q_S = param->pub;
        Q_C = buf_pub_them->data;
    }

    /* K_S, the host key */
    buf_put_pub_key(ses.kexhashbuf, hostkey, ses.newkeys->algo_hostkey);
    /* Q_C, client's ephemeral public key octet string */
    buf_putstring(ses.kexhashbuf, (const char*)Q_C, CURVE25519_LEN);
    /* Q_S, server's ephemeral public key octet string */
    buf_putstring(ses.kexhashbuf, (const char*)Q_S, CURVE25519_LEN);
    /* K, the shared secret */
    buf_putmpint(ses.kexhashbuf, ses.dh_K);

    /* calculate the hash H to sign */
    finish_kexhashbuf();
}

#endif /* DROPBEAR_CURVE25519 */
