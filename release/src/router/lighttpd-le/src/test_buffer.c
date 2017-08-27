#include "first.h"

#include "buffer.h"

static void run_buffer_path_simplify(buffer *psrc, buffer *pdest, const char *in, size_t in_len, const char *out, size_t out_len) {
	buffer_copy_string_len(psrc, in, in_len);

	buffer_path_simplify(pdest, psrc);

	if (!buffer_is_equal_string(pdest, out, out_len)) {
		fprintf(stderr,
			"%s.%d: buffer_path_simplify('%s') failed: expected '%s', got '%s'\n",
			__FILE__,
			__LINE__,
			in,
			out,
			pdest->ptr ? pdest->ptr : "");
		fflush(stderr);
		abort();
	} else {
		fprintf(stdout,
			"%s.%d: buffer_path_simplify('%s') succeeded: got '%s'\n",
			__FILE__,
			__LINE__,
			in,
			out);

		if (psrc != pdest) buffer_copy_buffer(psrc, pdest);
		buffer_path_simplify(pdest, psrc);

		if (!buffer_is_equal_string(pdest, out, out_len)) {
			fprintf(stderr,
				"%s.%d: buffer_path_simplify('%s') failed - not idempotent: expected '%s', got '%s'\n",
				__FILE__,
				__LINE__,
				out,
				out,
				pdest->ptr ? pdest->ptr : "");
			fflush(stderr);
			abort();
		}
	}
}

static void test_buffer_path_simplify_with(buffer *psrc, buffer *pdest) {
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN(""), CONST_STR_LEN(""));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN(" "), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/"), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("//"), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc"), CONST_STR_LEN("/abc"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc//"), CONST_STR_LEN("/abc/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc/./xyz"), CONST_STR_LEN("/abc/xyz"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc/.//xyz"), CONST_STR_LEN("/abc/xyz"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc/../xyz"), CONST_STR_LEN("/xyz"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/abc/./xyz"), CONST_STR_LEN("/abc/xyz"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/abc//./xyz"), CONST_STR_LEN("/abc/xyz"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/abc/../xyz"), CONST_STR_LEN("/xyz"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc/../xyz/."), CONST_STR_LEN("/xyz/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/abc/../xyz/."), CONST_STR_LEN("/xyz/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("abc/./xyz/.."), CONST_STR_LEN("/abc/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/abc/./xyz/.."), CONST_STR_LEN("/abc/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("./xyz/.."), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN(".//xyz/.."), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/./xyz/.."), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN(".././xyz/.."), CONST_STR_LEN("/"));
	run_buffer_path_simplify(psrc, pdest, CONST_STR_LEN("/.././xyz/.."), CONST_STR_LEN("/"));
}

static void test_buffer_path_simplify() {
	buffer *psrc = buffer_init();
	buffer *pdest = buffer_init();

	/* test with using the same buffer and with using different buffers */
	test_buffer_path_simplify_with(psrc, psrc);
	test_buffer_path_simplify_with(pdest, psrc);

	buffer_free(psrc);
	buffer_free(pdest);
}

int main() {
	test_buffer_path_simplify();

	return 0;
}
