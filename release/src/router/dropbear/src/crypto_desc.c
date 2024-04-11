#include "includes.h"
#include "dbutil.h"
#include "crypto_desc.h"
#include "ltc_prng.h"
#include "ecc.h"
#include "dbrandom.h"

#if DROPBEAR_LTC_PRNG
	int dropbear_ltc_prng = -1;
#endif

/* Wrapper for libtommath */
static mp_err dropbear_rand_source(void* out, size_t size) {
	genrandom((unsigned char*)out, (unsigned int)size);
	return MP_OKAY;
}


/* Register the compiled in ciphers.
 * This should be run before using any of the ciphers/hashes */
void crypto_init() {

	const struct ltc_cipher_descriptor *regciphers[] = {
#if DROPBEAR_AES
		&aes_desc,
#endif
#if DROPBEAR_3DES
		&des3_desc,
#endif
		NULL
	};

	const struct ltc_hash_descriptor *reghashes[] = {
#if DROPBEAR_SHA1_HMAC
		&sha1_desc,
#endif
#if DROPBEAR_SHA256
		&sha256_desc,
#endif
#if DROPBEAR_SHA384
		&sha384_desc,
#endif
#if DROPBEAR_SHA512
		&sha512_desc,
#endif
		NULL
	};
	int i;

	for (i = 0; regciphers[i] != NULL; i++) {
		if (register_cipher(regciphers[i]) == -1) {
			dropbear_exit("Error registering crypto");
		}
	}

	for (i = 0; reghashes[i] != NULL; i++) {
		if (register_hash(reghashes[i]) == -1) {
			dropbear_exit("Error registering crypto");
		}
	}

#if DROPBEAR_LTC_PRNG
	dropbear_ltc_prng = register_prng(&dropbear_prng_desc);
	if (dropbear_ltc_prng == -1) {
		dropbear_exit("Error registering crypto");
	}
#endif

	mp_rand_source(dropbear_rand_source);

#if DROPBEAR_ECC
	ltc_mp = ltm_desc;
	dropbear_ecc_fill_dp();
#endif
}

