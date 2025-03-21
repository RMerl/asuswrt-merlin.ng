#include "includes.h"
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "dbrandom.h"
#include "crypto_desc.h"
#include "ecc.h"
#include "kex.h"

#if DROPBEAR_ECDH
struct kex_ecdh_param *gen_kexecdh_param() {
    struct kex_ecdh_param *param = m_malloc(sizeof(*param));
    const struct dropbear_ecc_curve *curve = ses.newkeys->algo_kex->details;
    if (ecc_make_key_ex(NULL, dropbear_ltc_prng, 
        &param->key, curve->dp) != CRYPT_OK) {
        dropbear_exit("ECC error");
    }
    return param;
}

void free_kexecdh_param(struct kex_ecdh_param *param) {
    ecc_free(&param->key);
    m_free(param);

}
void kexecdh_comb_key(struct kex_ecdh_param *param, buffer *pub_them,
        sign_key *hostkey) {
    const struct dropbear_ecc_curve *curve
        = ses.newkeys->algo_kex->details;
    /* public keys from client and server */
    ecc_key *Q_C, *Q_S, *Q_them;

    Q_them = buf_get_ecc_raw_pubkey(pub_them, curve);
    if (Q_them == NULL) {
        dropbear_exit("ECC error");
    }

    ses.dh_K = dropbear_ecc_shared_secret(Q_them, &param->key);

    /* Create the remainder of the hash buffer, to generate the exchange hash
       See RFC5656 section 4 page 7 */
    if (IS_DROPBEAR_CLIENT) {
        Q_C = &param->key;
        Q_S = Q_them;
    } else {
        Q_C = Q_them;
        Q_S = &param->key;
    } 

    /* K_S, the host key */
    buf_put_pub_key(ses.kexhashbuf, hostkey, ses.newkeys->algo_hostkey);
    /* Q_C, client's ephemeral public key octet string */
    buf_put_ecc_raw_pubkey_string(ses.kexhashbuf, Q_C);
    /* Q_S, server's ephemeral public key octet string */
    buf_put_ecc_raw_pubkey_string(ses.kexhashbuf, Q_S);
    /* K, the shared secret */
    buf_putmpint(ses.kexhashbuf, ses.dh_K);

    ecc_free(Q_them);
    m_free(Q_them);

    /* calculate the hash H to sign */
    finish_kexhashbuf();
}
#endif /* DROPBEAR_ECDH */

