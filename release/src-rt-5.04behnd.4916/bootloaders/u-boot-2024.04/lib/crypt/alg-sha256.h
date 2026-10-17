/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2020 Steffen Jaeckel <jaeckel-floss@eyet-services.de> */

#include "u-boot/sha256.h"

#define INCLUDE_sha256crypt 1

#define SHA256_CTX sha256_context
#define SHA256_Init sha256_starts
#define SHA256_Update(c, i, l) sha256_update(c, (const void *)i, l)
#define SHA256_Final(b, c) sha256_finish(c, b)
