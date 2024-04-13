/* fuzz target for cli-readconf.c */

#include "fuzz.h"
#include "fuzz-wrapfd.h"
#include "debug.h"
#include "runopts.h"

static void setup_fuzzer(void) {
	fuzz_common_setup();
	/* Set up commandline args */
	char* args[2] = { "dbclient", "far" };
	cli_getopts(2, args);
}

// Needs to be outside so it doesn't get optimised away for the setjmp().
// volatile doesn't seem to work, unsure why.
static FILE *conf_file = NULL;

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	static int once = 0;
	if (!once) {
		setup_fuzzer();
		once = 1;
	}

	if (fuzz_set_input(Data, Size) == DROPBEAR_FAILURE) {
		return 0;
	}

	m_malloc_set_epoch(1);

	if (setjmp(fuzz.jmp) == 0) {

		/* remotehost most be set before config parsing */
		m_free(cli_opts.remotehost);
		cli_opts.remotehost = m_strdup("far");
		/* optional arguments */
		if (buf_getbool(fuzz.input)) {
			m_free(cli_opts.username);
			cli_opts.username = m_strdup("someone");
		}
		if (buf_getbool(fuzz.input)) {
			m_free(cli_opts.remoteport);
			cli_opts.remoteport = m_strdup("999");
		}

		buffer *conf_buf = buf_getstringbuf(fuzz.input);
		if (conf_buf->len > 0)
		{
			conf_file = fmemopen(conf_buf->data, conf_buf->len, "r");
			read_config_file("fuzz", conf_file, &cli_opts);
			fclose(conf_file);
			conf_file = NULL;
		}
		buf_free(conf_buf);

		m_free(cli_opts.remotehost);
		m_free(cli_opts.remoteport);
		m_free(cli_opts.username);

		m_malloc_free_epoch(1, 0);
	} else {
		// Cleanup
		if (conf_file) {
			fclose(conf_file);
			conf_file = NULL;
		}

		m_free(cli_opts.remotehost);
		m_free(cli_opts.remoteport);
		m_free(cli_opts.username);

		m_malloc_free_epoch(1, 1);
		TRACE(("dropbear_exit longjmped"))
		/* dropbear_exit jumped here */
	}

	return 0;
}
