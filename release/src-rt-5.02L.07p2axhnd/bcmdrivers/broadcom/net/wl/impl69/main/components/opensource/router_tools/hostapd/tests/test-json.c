/*
 * JSON parser - test program
 * Copyright (c) 2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/os.h"
#include "utils/json.h"
#include "utils/wpa_debug.h"

void run_test(const char *buf, size_t len)
{
	struct json_token *root;
	char *txt;
	size_t buflen = 10000;

	root = json_parse(buf, len);
	if (!root) {
		wpa_printf(MSG_DEBUG, "JSON parsing failed");
		return;
	}

	txt = os_zalloc(buflen);
	if (txt) {
		json_print_tree(root, txt, buflen);
		wpa_printf(MSG_DEBUG, "%s", txt);
		os_free(txt);
	}
	json_free(root);
}

#ifdef TEST_LIBFUZZER
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	run_test((const char *) data, size);
	return 0;
}
#else /* TEST_LIBFUZZER */
int main(int argc, char *argv[])
{
	char *buf;
	size_t len;

	wpa_debug_level = 0;

	if (argc < 2)
		return -1;

	buf = os_readfile(argv[1], &len);
	if (!buf)
		return -1;

	run_test(buf, len);
	os_free(buf);

	return 0;
}
#endif /* TEST_LIBFUZZER */
