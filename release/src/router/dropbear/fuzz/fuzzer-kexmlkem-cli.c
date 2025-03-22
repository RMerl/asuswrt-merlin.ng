#include "fuzz.h"
#include "session.h"
#include "fuzz-wrapfd.h"
#include "debug.h"
#include "runopts.h"
#include "algo.h"

static struct key_context* keep_newkeys = NULL;

static void setup() __attribute__((constructor));
static void setup() {
	fuzz_common_setup();
	fuzz_cli_setup();

	keep_newkeys = (struct key_context*)m_malloc(sizeof(struct key_context));
	keep_newkeys->algo_kex = fuzz_get_algo(sshkex, "mlkem768x25519-sha256");
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
		return 0;
	}

	m_malloc_set_epoch(1);

	if (setjmp(fuzz.jmp) == 0) {
		/* Arbitrary key to write into a buffer */
		sign_key *hostkey = cli_opts.privkeys->first->item;
		ses.newkeys = keep_newkeys;

		struct kex_pqhybrid_param *param = gen_kexpqhybrid_param();

		buffer * q_s = buf_getstringbuf(fuzz.input);

		ses.kexhashbuf = buf_new(KEXHASHBUF_MAX_INTS);
		kexpqhybrid_comb_key(param, q_s, hostkey);

		free_kexpqhybrid_param(param);

		buf_free(ses.dh_K_bytes);
		buf_free(q_s);

		buf_free(ses.hash);
		buf_free(ses.session_id);
		/* kexhashbuf is freed in kexpqhybrid_comb_key */

		m_malloc_free_epoch(1, 0);
	} else {
		m_malloc_free_epoch(1, 1);
		TRACE(("dropbear_exit longjmped"))
		/* dropbear_exit jumped here */
	}

	return 0;
}
