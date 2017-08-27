#include "first.h"

#include "base64.h"

static void check_all_len_0() {
	buffer *check = buffer_init();
	const char empty[] = "";

	{
		unsigned char* check_res;

		force_assert(0 == li_to_base64_no_padding(NULL, 0, NULL, 0, BASE64_STANDARD));

		buffer_reset(check);
		check_res = buffer_append_base64_decode(check, NULL, 0, BASE64_STANDARD);
		force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
		force_assert(buffer_is_equal_string(check, empty, 0));
	}

	{
		unsigned char* check_res;

		force_assert(0 == li_to_base64_no_padding(NULL, 0, NULL, 0, BASE64_URL));

		buffer_reset(check);
		check_res = buffer_append_base64_decode(check, NULL, 0, BASE64_URL);
		force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
		force_assert(buffer_is_equal_string(check, empty, 0));
	}

	buffer_free(check);
}

static void check_all_len_1() {
	unsigned int c1;
	buffer *check = buffer_init();

	for (c1 = 0; c1 < 256; ++c1) {
		{
			unsigned char in[] = { c1 };
			char out[2] = { 0, 0 };
			unsigned char* check_res;

			force_assert(sizeof(out) == li_to_base64_no_padding(out, sizeof(out), in, sizeof(in), BASE64_STANDARD));

			buffer_reset(check);
			check_res = buffer_append_base64_decode(check, out, sizeof(out), BASE64_STANDARD);
			force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
			force_assert(buffer_is_equal_string(check, (const char*) in, sizeof(in)));
		}

		{
			unsigned char in[] = { c1 };
			char out[2] = { 0, 0 };
			unsigned char* check_res;

			force_assert(sizeof(out) == li_to_base64_no_padding(out, sizeof(out), in, sizeof(in), BASE64_URL));

			buffer_reset(check);
			check_res = buffer_append_base64_decode(check, out, sizeof(out), BASE64_URL);
			force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
			force_assert(buffer_is_equal_string(check, (const char*) in, sizeof(in)));
		}
	}

	buffer_free(check);
}

static void check_all_len_2() {
	unsigned int c1, c2;
	buffer *check = buffer_init();

	for (c1 = 0; c1 < 256; ++c1) for (c2 = 0; c2 < 256; ++c2) {
		{
			unsigned char in[] = { c1, c2 };
			char out[3] = { 0, 0, 0 };
			unsigned char* check_res;

			force_assert(sizeof(out) == li_to_base64_no_padding(out, sizeof(out), in, sizeof(in), BASE64_STANDARD));

			buffer_reset(check);
			check_res = buffer_append_base64_decode(check, out, sizeof(out), BASE64_STANDARD);
			force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
			force_assert(buffer_is_equal_string(check, (const char*) in, sizeof(in)));
		}

		{
			unsigned char in[] = { c1, c2 };
			char out[3] = { 0, 0, 0 };
			unsigned char* check_res;

			force_assert(sizeof(out) == li_to_base64_no_padding(out, sizeof(out), in, sizeof(in), BASE64_URL));

			buffer_reset(check);
			check_res = buffer_append_base64_decode(check, out, sizeof(out), BASE64_URL);
			force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
			force_assert(buffer_is_equal_string(check, (const char*) in, sizeof(in)));
		}
	}

	buffer_free(check);
}

static void check_all_len_3() {
	unsigned int c1, c2, c3;
	buffer *check = buffer_init();

	for (c1 = 0; c1 < 256; ++c1) for (c2 = 0; c2 < 256; ++c2) for (c3 = 0; c3 < 256; ++c3) {
		{
			unsigned char in[] = { c1, c2, c3 };
			char out[4] = { 0, 0, 0, 0 };
			unsigned char* check_res;

			force_assert(sizeof(out) == li_to_base64_no_padding(out, sizeof(out), in, sizeof(in), BASE64_STANDARD));

			buffer_reset(check);
			check_res = buffer_append_base64_decode(check, out, sizeof(out), BASE64_STANDARD);
			force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
			force_assert(buffer_is_equal_string(check, (const char*) in, sizeof(in)));
		}

		{
			unsigned char in[] = { c1, c2, c3 };
			char out[4] = { 0, 0, 0, 0 };
			unsigned char* check_res;

			force_assert(sizeof(out) == li_to_base64_no_padding(out, sizeof(out), in, sizeof(in), BASE64_URL));

			buffer_reset(check);
			check_res = buffer_append_base64_decode(check, out, sizeof(out), BASE64_URL);
			force_assert((check_res != NULL) && (check_res == (unsigned char*) check->ptr));
			force_assert(buffer_is_equal_string(check, (const char*) in, sizeof(in)));
		}
	}

	buffer_free(check);
}

int main() {
	check_all_len_0();
	check_all_len_1();
	check_all_len_2();
	check_all_len_3();

	return 0;
}
