/*
 * Testing tool for X.509v3 routines
 * Copyright (c) 2006-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "tls/x509v3.h"

#ifdef TEST_LIBFUZZER
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct x509_certificate *cert;

	cert = x509_certificate_parse(data, size);
	x509_certificate_free(cert);
	return 0;
}
#else /* TEST_LIBFUZZER */
int main(int argc, char *argv[])
{
	FILE *f;
	u8 buf[3000];
	size_t len;
	struct x509_certificate *cert;

	wpa_debug_level = 0;

	f = fopen(argv[1], "rb");
	if (f == NULL)
		return -1;
	len = fread(buf, 1, sizeof(buf), f);
	fclose(f);

	cert = x509_certificate_parse(buf, len);
	if (cert == NULL)
		printf("Failed to parse X.509 certificate\n");
	x509_certificate_free(cert);

	return 0;
}
#endif /* TEST_LIBFUZZER */
