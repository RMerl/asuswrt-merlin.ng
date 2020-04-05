#include "fuzz.h"
#include "session.h"
#include "fuzz-wrapfd.h"
#include "debug.h"
#include "runopts.h"
#include "algo.h"
#include "bignum.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	static int once = 0;
	static struct key_context* keep_newkeys = NULL;
	/* number of generated parameters is limited by the timeout for the first run.
	   TODO move this to the libfuzzer initialiser function instead if the timeout
	   doesn't apply there */
	#define NUM_PARAMS 20
	static struct kex_curve25519_param *curve25519_params[NUM_PARAMS];

	if (!once) {
		fuzz_common_setup();
		fuzz_svr_setup();

		keep_newkeys = (struct key_context*)m_malloc(sizeof(struct key_context));
		keep_newkeys->algo_kex = fuzz_get_algo(sshkex, "curve25519-sha256");
		keep_newkeys->algo_hostkey = DROPBEAR_SIGNKEY_ED25519;
		ses.newkeys = keep_newkeys;

		/* Pre-generate parameters */
		int i;
		for (i = 0; i < NUM_PARAMS; i++) {
			curve25519_params[i] = gen_kexcurve25519_param();
		}

		once = 1;
	}

	if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
		return 0;
	}

	m_malloc_set_epoch(1);

	if (setjmp(fuzz.jmp) == 0) {
		/* Based on recv_msg_kexdh_init()/send_msg_kexdh_reply() 
		with DROPBEAR_KEX_CURVE25519 */
		ses.newkeys = keep_newkeys;

		/* Choose from the collection of curve25519 params */
		unsigned int e = buf_getint(fuzz.input);
		struct kex_curve25519_param *curve25519_param = curve25519_params[e % NUM_PARAMS];

		buffer * ecdh_qs = buf_getstringbuf(fuzz.input);

		ses.kexhashbuf = buf_new(KEXHASHBUF_MAX_INTS);
		kexcurve25519_comb_key(curve25519_param, ecdh_qs, svr_opts.hostkey);

		mp_clear(ses.dh_K);
		m_free(ses.dh_K);
		buf_free(ecdh_qs);

		buf_free(ses.hash);
		buf_free(ses.session_id);
		/* kexhashbuf is freed in kexdh_comb_key */

		m_malloc_free_epoch(1, 0);
	} else {
		m_malloc_free_epoch(1, 1);
		TRACE(("dropbear_exit longjmped"))
		/* dropbear_exit jumped here */
	}

	return 0;
}
