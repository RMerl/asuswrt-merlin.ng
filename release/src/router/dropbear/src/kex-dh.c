#include "includes.h"
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "bignum.h"
#include "dbrandom.h"
#include "crypto_desc.h"
#include "dh_groups.h"
#include "kex.h"

#if DROPBEAR_NORMAL_DH
static void load_dh_p(mp_int * dh_p)
{
    bytes_to_mp(dh_p, ses.newkeys->algo_kex->dh_p_bytes, 
        ses.newkeys->algo_kex->dh_p_len);
}

/* Initialises and generate one side of the diffie-hellman key exchange values.
 * See the transport rfc 4253 section 8 for details */
/* dh_pub and dh_priv MUST be already initialised */
struct kex_dh_param *gen_kexdh_param() {
    struct kex_dh_param *param = NULL;

    DEF_MP_INT(dh_p);
    DEF_MP_INT(dh_q);
    DEF_MP_INT(dh_g);

    TRACE(("enter gen_kexdh_vals"))

    param = m_malloc(sizeof(*param));
    m_mp_init_multi(&param->pub, &param->priv, &dh_g, &dh_p, &dh_q, NULL);

    /* read the prime and generator*/
    load_dh_p(&dh_p);
    
    mp_set_ul(&dh_g, DH_G_VAL);

    /* calculate q = (p-1)/2 */
    /* dh_priv is just a temp var here */
    if (mp_sub_d(&dh_p, 1, &param->priv) != MP_OKAY) { 
        dropbear_exit("Diffie-Hellman error");
    }
    if (mp_div_2(&param->priv, &dh_q) != MP_OKAY) {
        dropbear_exit("Diffie-Hellman error");
    }

    /* Generate a private portion 0 < dh_priv < dh_q */
    gen_random_mpint(&dh_q, &param->priv);

    /* f = g^y mod p */
    if (mp_exptmod(&dh_g, &param->priv, &dh_p, &param->pub) != MP_OKAY) {
        dropbear_exit("Diffie-Hellman error");
    }
    mp_clear_multi(&dh_g, &dh_p, &dh_q, NULL);
    return param;
}

void free_kexdh_param(struct kex_dh_param *param)
{
    mp_clear_multi(&param->pub, &param->priv, NULL);
    m_free(param);
}

/* This function is fairly common between client/server, with some substitution
 * of dh_e/dh_f etc. Hence these arguments:
 * dh_pub_us is 'e' for the client, 'f' for the server. dh_pub_them is 
 * vice-versa. dh_priv is the x/y value corresponding to dh_pub_us */
void kexdh_comb_key(struct kex_dh_param *param, mp_int *dh_pub_them,
        sign_key *hostkey) {

    DEF_MP_INT(dh_p);
    DEF_MP_INT(dh_p_min1);
    mp_int *dh_e = NULL, *dh_f = NULL;

    m_mp_init_multi(&dh_p, &dh_p_min1, NULL);
    load_dh_p(&dh_p);

    if (mp_sub_d(&dh_p, 1, &dh_p_min1) != MP_OKAY) { 
        dropbear_exit("Diffie-Hellman error");
    }

    /* Check that dh_pub_them (dh_e or dh_f) is in the range [2, p-2] */
    if (mp_cmp(dh_pub_them, &dh_p_min1) != MP_LT 
            || mp_cmp_d(dh_pub_them, 1) != MP_GT) {
        dropbear_exit("Diffie-Hellman error");
    }
    
    /* K = e^y mod p = f^x mod p */
    m_mp_alloc_init_multi(&ses.dh_K, NULL);
    if (mp_exptmod(dh_pub_them, &param->priv, &dh_p, ses.dh_K) != MP_OKAY) {
        dropbear_exit("Diffie-Hellman error");
    }

    /* clear no longer needed vars */
    mp_clear_multi(&dh_p, &dh_p_min1, NULL);

    /* From here on, the code needs to work with the _same_ vars on each side,
     * not vice-versaing for client/server */
    if (IS_DROPBEAR_CLIENT) {
        dh_e = &param->pub;
        dh_f = dh_pub_them;
    } else {
        dh_e = dh_pub_them;
        dh_f = &param->pub;
    } 

    /* Create the remainder of the hash buffer, to generate the exchange hash */
    /* K_S, the host key */
    buf_put_pub_key(ses.kexhashbuf, hostkey, ses.newkeys->algo_hostkey);
    /* e, exchange value sent by the client */
    buf_putmpint(ses.kexhashbuf, dh_e);
    /* f, exchange value sent by the server */
    buf_putmpint(ses.kexhashbuf, dh_f);
    /* K, the shared secret */
    buf_putmpint(ses.kexhashbuf, ses.dh_K);

    /* calculate the hash H to sign */
    finish_kexhashbuf();
}
#endif
