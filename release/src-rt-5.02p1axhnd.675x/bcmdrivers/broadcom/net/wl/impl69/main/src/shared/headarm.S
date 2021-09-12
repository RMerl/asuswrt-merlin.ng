/*
 * RTE ARM run-time initialization.
 *
 * This file assumes that we are in RAM, mapped at address 0,
 * so it is for images that get downloaded and run via a write
 * to corecontrol to turn off the remapzero bit.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: headarm.S 596126 2015-10-29 19:53:48Z $
 */

#include <arminc.h>

#ifdef	CONFIG_XIP
#error	Use bootarm.S for XIP
#endif // endif

#include "startarm.S"

	.text

	FUNC(inflrom)

	/* We don't do flash/ROM */
	TRACE(200)
	b	.

	END(inflrom)
