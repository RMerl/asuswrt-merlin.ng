#include "fuzz.h"
#include "session.h"
#include "fuzz-wrapfd.h"
#include "debug.h"
#include "dss.h"
#include "ed25519.h"

static void setup_fuzzer(void) {
	fuzz_common_setup();
}

static buffer *verifydata;

/* Tests reading a public key and verifying a signature */
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	static int once = 0;
	if (!once) {
		setup_fuzzer();
		verifydata = buf_new(30);
		buf_putstring(verifydata, "x", 1);
		once = 1;
	}

	if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
		return 0;
	}

	m_malloc_set_epoch(1);

	if (setjmp(fuzz.jmp) == 0) {
		sign_key *key = new_sign_key();
		enum signkey_type keytype = DROPBEAR_SIGNKEY_ANY;
		if (buf_get_pub_key(fuzz.input, key, &keytype) == DROPBEAR_SUCCESS) {
			enum signature_type sigtype;
			if (keytype == DROPBEAR_SIGNKEY_RSA) {
				/* Flip a coin to decide rsa signature type */
				int flag = buf_getbyte(fuzz.input);
				if (flag & 0x01) {
					sigtype = DROPBEAR_SIGNATURE_RSA_SHA256;
				} else {
					sigtype = DROPBEAR_SIGNATURE_RSA_SHA1;
				}
			} else {
				sigtype = signature_type_from_signkey(keytype);
			}
			if (buf_verify(fuzz.input, key, sigtype, verifydata) == DROPBEAR_SUCCESS) {
				/* The fuzzer is capable of generating keys with a signature to match.
				We don't want false positives if the key is bogus, since a client/server 
				wouldn't be trusting a bogus key anyway */
				int boguskey = 0;

				if (keytype == DROPBEAR_SIGNKEY_DSS) {
					/* So far have seen dss keys with bad p/q/g domain parameters */
					int pprime, qprime, trials;
					trials = mp_prime_rabin_miller_trials(mp_count_bits(key->dsskey->p));
					assert(mp_prime_is_prime(key->dsskey->p, trials, &pprime) == MP_OKAY);
					trials = mp_prime_rabin_miller_trials(mp_count_bits(key->dsskey->q));
					assert(mp_prime_is_prime(key->dsskey->q, trials, &qprime) == MP_OKAY);
					boguskey = !(pprime && qprime);
					/* Could also check g**q mod p == 1 */
				}

				if (keytype == DROPBEAR_SIGNKEY_SK_ED25519 || keytype == DROPBEAR_SIGNKEY_ED25519) {
					dropbear_ed25519_key **eck = (dropbear_ed25519_key**)signkey_key_ptr(key, keytype);
					if (eck && *eck) {
						int i;
						/* we've seen all-zero keys validate */
						boguskey = 1;
						for (i = 0; i < CURVE25519_LEN; i++) {
							if ((*eck)->priv[i] != 0x00 || (*eck)->pub[i] != 0x00) {
								boguskey = 0;
							}
						}

					}
				}

				if (!boguskey) {
					printf("Random key/signature managed to verify!\n");
					abort();
				}


			}
		}
		sign_key_free(key);
		m_malloc_free_epoch(1, 0);
	} else {
		m_malloc_free_epoch(1, 1);
		TRACE(("dropbear_exit longjmped"))
		/* dropbear_exit jumped here */
	}

	return 0;
}
