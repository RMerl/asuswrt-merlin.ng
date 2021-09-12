// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

struct def {
	const char *name;
	unsigned long offset;
	unsigned long indirection_offset;
};
extern const struct def defs[];

#ifdef __KERNEL__
#include "../drivers/net/wireguard/noise.h"

const struct def defs[] = {
	{ "LOCAL_STATIC_PRIVATE_KEY", offsetof(struct noise_static_identity, static_private), offsetof(struct noise_handshake, static_identity) },
	{ "LOCAL_EPHEMERAL_PRIVATE_KEY", offsetof(struct noise_handshake, ephemeral_private), -1 },
	{ "REMOTE_STATIC_PUBLIC_KEY", offsetof(struct noise_handshake, remote_static), -1 },
	{ "PRESHARED_KEY", offsetof(struct noise_handshake, preshared_key), -1 },
	{ NULL, 0 }
};
#else
#include <stdio.h>
int main(int argc, char *argv[])
{
	puts("declare -A OFFSETS=(");
	for (const struct def *def = defs; def->name; ++def) {
		printf("\t[%s]=%ld", def->name, def->offset);
		if (def->indirection_offset != -1)
			printf(",%ld", def->indirection_offset);
		putchar('\n');
	}
	puts(")");
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	puts("ENDIAN=big");
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	puts("ENDIAN=little");
#else
#error "Unsupported endianness"
#endif
	return 0;
}
#endif
