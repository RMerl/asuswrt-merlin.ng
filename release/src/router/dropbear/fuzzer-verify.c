#include "fuzz.h"
#include "session.h"
#include "fuzz-wrapfd.h"
#include "debug.h"

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
		enum signkey_type type = DROPBEAR_SIGNKEY_ANY;
		if (buf_get_pub_key(fuzz.input, key, &type) == DROPBEAR_SUCCESS) {
			if (buf_verify(fuzz.input, key, verifydata) == DROPBEAR_SUCCESS) {
				/* The fuzzer is capable of generating keys with a signature to match.
				We don't want false positives if the key is bogus, since a client/server 
				wouldn't be trusting a bogus key anyway */
				int boguskey = 0;

				if (type == DROPBEAR_SIGNKEY_DSS) {
					/* So far have seen dss keys with bad p/q/g domain parameters */
					int pprime, qprime;
				    assert(mp_prime_is_prime(key->dsskey->p, 5, &pprime) == MP_OKAY);
				    assert(mp_prime_is_prime(key->dsskey->q, 18, &qprime) == MP_OKAY);
				    boguskey = !(pprime && qprime);
				    /* Could also check g**q mod p == 1 */
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
