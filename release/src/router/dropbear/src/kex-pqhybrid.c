#include "includes.h"
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "bignum.h"
#include "dbrandom.h"
#include "crypto_desc.h"
#include "curve25519.h"
#include "kex.h"

#if DROPBEAR_PQHYBRID

struct kex_pqhybrid_param *gen_kexpqhybrid_param() {
    struct kex_pqhybrid_param *param = m_malloc(sizeof(*param));
    const struct dropbear_kem_desc *kem = ses.newkeys->algo_kex->details;

    param->curve25519 = gen_kexcurve25519_param();

    if (IS_DROPBEAR_CLIENT) {
        param->kem_cli_secret = buf_new(kem->secret_len);
        param->concat_public = buf_new(kem->public_len + CURVE25519_LEN);
        kem->kem_gen(
            buf_getwriteptr(param->concat_public, kem->public_len),
            buf_getwriteptr(param->kem_cli_secret, kem->secret_len));
        buf_incrwritepos(param->concat_public, kem->public_len);
        buf_incrwritepos(param->kem_cli_secret, kem->secret_len);
        buf_setpos(param->kem_cli_secret, 0);
        /* Append the curve25519 parameter */
        buf_putbytes(param->concat_public, param->curve25519->pub, CURVE25519_LEN);
    }

    return param;
}

void free_kexpqhybrid_param(struct kex_pqhybrid_param *param) {
    free_kexcurve25519_param(param->curve25519);
    if (param->kem_cli_secret) {
        buf_burn_free(param->kem_cli_secret);
        param->kem_cli_secret = NULL;
    }
    buf_free(param->concat_public);
    m_free(param);
}

void kexpqhybrid_comb_key(struct kex_pqhybrid_param *param,
    buffer *buf_pub, sign_key *hostkey) {

    const struct dropbear_kem_desc *kem = ses.newkeys->algo_kex->details;
    const struct ltc_hash_descriptor *hash_desc
        = ses.newkeys->algo_kex->hash_desc;

    /* Either public key (from client) or ciphertext (from server) */
    unsigned char *remote_pub_kem = NULL;
    buffer *pub_25519 = NULL;
    buffer *k_out = NULL;
    unsigned int remote_len;
    hash_state hs;
    const buffer * Q_C = NULL;
    const buffer * Q_S = NULL;

    /* Extract input parts from the remote peer */
    if (IS_DROPBEAR_CLIENT) {
        /* S_REPLY = S_CT2 || S_PK1 */
        remote_len = kem->ciphertext_len;
    } else {
        /* C_INIT = C_PK2 || C_PK1 */
        remote_len = kem->public_len;
    }
    remote_pub_kem = buf_getptr(buf_pub, remote_len);
    buf_incrpos(buf_pub, remote_len);
    pub_25519 = buf_getptrcopy(buf_pub, CURVE25519_LEN);
    buf_incrpos(buf_pub, CURVE25519_LEN);
    /* Check all is consumed */
    if (buf_pub->pos != buf_pub->len) {
        dropbear_exit("Bad sntrup");
    }

    /* k_out = K_PQ || K_CL */
    k_out = buf_new(kem->output_len + CURVE25519_LEN);

    /* Derive pq kem part (K_PQ) */
    if (IS_DROPBEAR_CLIENT) {
        kem->kem_dec(
            buf_getwriteptr(k_out, kem->output_len),
            remote_pub_kem,
            buf_getptr(param->kem_cli_secret, kem->secret_len));
        buf_burn_free(param->kem_cli_secret);
        param->kem_cli_secret = NULL;
    } else {
        /* Server returns ciphertext */
        assert(param->concat_public == NULL);
        param->concat_public = buf_new(kem->ciphertext_len + CURVE25519_LEN);
        kem->kem_enc(
            buf_getwriteptr(param->concat_public, kem->ciphertext_len),
            buf_getwriteptr(k_out, kem->output_len),
            remote_pub_kem);
        buf_incrwritepos(param->concat_public, kem->ciphertext_len);
        /* Append the curve25519 parameter */
        buf_putbytes(param->concat_public, param->curve25519->pub, CURVE25519_LEN);
    }
    buf_incrwritepos(k_out, kem->output_len);

    /* Derive ec part (K_CL) */
    kexcurve25519_derive(param->curve25519, pub_25519,
        buf_getwriteptr(k_out, CURVE25519_LEN));
    buf_incrwritepos(k_out, CURVE25519_LEN);

    /* dh_K_bytes = HASH(k_out)
       dh_K_bytes is a SSH string with length prefix, since
       that is what needs to be hashed in gen_new_keys() */
    ses.dh_K_bytes = buf_new(4 + hash_desc->hashsize);
    buf_putint(ses.dh_K_bytes, hash_desc->hashsize);
    hash_desc->init(&hs);
    hash_desc->process(&hs, k_out->data, k_out->len);
    hash_desc->done(&hs, buf_getwriteptr(ses.dh_K_bytes, hash_desc->hashsize));
    m_burn(&hs, sizeof(hash_state));
    buf_incrwritepos(ses.dh_K_bytes, hash_desc->hashsize);

    /* Create the remainder of the hash buffer */
    if (IS_DROPBEAR_CLIENT) {
        Q_C = param->concat_public;
        Q_S = buf_pub;
    } else {
        Q_S = param->concat_public;
        Q_C = buf_pub;
    }

    /* K_S, the host key */
    buf_put_pub_key(ses.kexhashbuf, hostkey, ses.newkeys->algo_hostkey);
    buf_putbufstring(ses.kexhashbuf, Q_C);
    buf_putbufstring(ses.kexhashbuf, Q_S);
    /* K, the shared secret */
    buf_putbytes(ses.kexhashbuf, ses.dh_K_bytes->data, ses.dh_K_bytes->len);

    /* calculate the hash H to sign */
    finish_kexhashbuf();

    buf_burn_free(k_out);
    buf_free(pub_25519);
}

#endif /* DROPBEAR_PQHYBRID */
