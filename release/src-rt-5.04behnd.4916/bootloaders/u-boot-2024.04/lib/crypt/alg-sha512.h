/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2020 Steffen Jaeckel <jaeckel-floss@eyet-services.de> */

#include "u-boot/sha512.h"

#define INCLUDE_sha512crypt 1

#define SHA512_CTX sha512_context
#define SHA512_Init sha512_starts
#define SHA512_Update(c, i, l) sha512_update(c, (const void *)i, l)
#define SHA512_Final(b, c) sha512_finish(c, b)
