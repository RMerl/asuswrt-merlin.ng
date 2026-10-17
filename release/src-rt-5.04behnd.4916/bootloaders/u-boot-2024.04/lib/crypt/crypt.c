// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2020 Steffen Jaeckel <jaeckel-floss@eyet-services.de> */

#include <crypt.h>
#include "crypt-port.h"

typedef int (*crypt_fn)(const char *, size_t, const char *, size_t, uint8_t *,
			size_t, void *, size_t);

const unsigned char ascii64[65] =
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void equals_constant_time(const void *a_, const void *b_, size_t len,
				 int *equal)
{
	u8 ret = 0;
	const u8 *a = a_, *b = b_;
	int i;

	for (i = 0; i < len; i++)
		ret |= a[i] ^ b[i];

	ret |= ret >> 4;
	ret |= ret >> 2;
	ret |= ret >> 1;
	ret &= 1;

	*equal = ret ^ 1;
}

int crypt_compare(const char *should, const char *passphrase, int *equal)
{
	u8 output[CRYPT_OUTPUT_SIZE], scratch[ALG_SPECIFIC_SIZE];
	size_t n;
	int err;
	struct {
		const char *prefix;
		crypt_fn crypt;
	} crypt_algos[] = {
#if defined(CONFIG_CRYPT_PW_SHA256)
		{ "$5$", crypt_sha256crypt_rn_wrapped },
#endif
#if defined(CONFIG_CRYPT_PW_SHA512)
		{ "$6$", crypt_sha512crypt_rn_wrapped },
#endif
		{ NULL, NULL }
	};

	*equal = 0;

	for (n = 0; n < ARRAY_SIZE(crypt_algos); ++n) {
		if (!crypt_algos[n].prefix)
			continue;
		if (strncmp(should, crypt_algos[n].prefix, 3) == 0)
			break;
	}

	if (n >= ARRAY_SIZE(crypt_algos))
		return -EINVAL;

	err = crypt_algos[n].crypt(passphrase, strlen(passphrase), should, 0,
				   output, sizeof(output), scratch,
				   sizeof(scratch));
	/* early return on error, nothing really happened inside the crypt() function */
	if (err)
		return err;

	equals_constant_time(should, output, strlen((const char *)output),
			     equal);

	memset(scratch, 0, sizeof(scratch));
	memset(output, 0, sizeof(output));

	return 0;
}
